#include "stdafx.h"

#include "IOCPServer.h"

CIOCPServer::CIOCPServer()
{
	// ����WinSock2��
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2),&wsaData))
	{
		return;
	}

	// ��ʼ����ͷ [Shine]
	memcpy(_M_PacketFlagData, "Shine", PACKET_FLAG_LENGTH);

	_M_sListen = INVALID_SOCKET;
	

	_M_hListenEvent  =  WSA_INVALID_EVENT;
	_M_hKillEvent    =  WSA_INVALID_EVENT;

	_M_hCompletionPort =  NULL;

	_M_hListenThread   =  NULL;


	_M_KeepLiveTime = 0;

	_M_ThreadPoolMin = 0;
	_M_ThreadPoolMax = 0;
	_M_WorkThreadCount = 0;

	_M_CurrentThreadCount = 0;
	_M_BusyThreadCount    = 0;

	_M_ContextFreeList = NULL;

	_M_TimeToKill = FALSE;

	InitializeCriticalSection(&_M_CriticalSection);

}

CIOCPServer::~CIOCPServer()
{
	Sleep(0);
	SetEvent(_M_hKillEvent);
	WaitForSingleObject(_M_hListenThread, INFINITE);


	if (_M_sListen != INVALID_SOCKET)
	{
		closesocket(_M_sListen);
		_M_sListen = INVALID_SOCKET;
	}

	if (_M_hListenEvent != WSA_INVALID_EVENT)
	{
		WSACloseEvent(_M_hListenEvent);
		_M_hListenEvent = WSA_INVALID_EVENT;
	}

	if (_M_hCompletionPort != INVALID_HANDLE_VALUE)
	{
		CloseHandle(_M_hCompletionPort);
		_M_hCompletionPort = INVALID_HANDLE_VALUE;
	}

	DeleteCriticalSection(&_M_CriticalSection);

	WSACleanup();

}

BOOL  CIOCPServer::StartServer(USHORT nPort, INT Backlog, lpfn_WindowNotifyProc WindowNotifyProc)
{

#define WSA_START_SERVER_ERROR_CLEAN()		WSACloseEvent(_M_hListenEvent);\
									_M_hListenEvent = WSA_INVALID_EVENT;\
									closesocket(_M_sListen);\
									_M_sListen = INVALID_SOCKET;\
									return FALSE;\

	BOOL rVal = FALSE;

	_M_WindowNotifyProc = WindowNotifyProc;

	_M_hKillEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (_M_hKillEvent == NULL)
	{
		return FALSE;
	}

	// ���������׽���
	_M_sListen = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	if (_M_sListen == INVALID_SOCKET) {
		return FALSE;
	}

	// ���������¼�
	_M_hListenEvent = WSACreateEvent();

	if (_M_hListenEvent == WSA_INVALID_EVENT) {
		closesocket(_M_sListen);
		_M_sListen = INVALID_SOCKET;
		return FALSE;
	}

	// �����¼�������׽��ְ�
	rVal = WSAEventSelect(_M_sListen, _M_hListenEvent, FD_ACCEPT | FD_CLOSE);

	if (rVal == SOCKET_ERROR){
		WSA_START_SERVER_ERROR_CLEAN()
	}

	// ��ʼ������
	SOCKADDR_IN ServerAddr;
	ServerAddr.sin_port   = htons(nPort);
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_addr.s_addr = INADDR_ANY;

	// �����׽�����������
	rVal = bind(_M_sListen, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr));

	if (rVal == SOCKET_ERROR) {
		WSA_START_SERVER_ERROR_CLEAN()
	}

	// ��ʼ����
	rVal = listen(_M_sListen, Backlog);

	if (rVal == SOCKET_ERROR) {
		WSA_START_SERVER_ERROR_CLEAN()
	}

	// ���������߳�
	_M_hListenThread = CreateThread(
								NULL,
								0,
								(LPTHREAD_START_ROUTINE)_ListenThreadProc,
								(VOID*)this,
								0,
								NULL
							);

	if(_M_hListenThread == INVALID_HANDLE_VALUE) {
		WSA_START_SERVER_ERROR_CLEAN()
	}

	// ��ʼ��IOCP
	InitializeIOCP();

#undef WSA_START_SERVER_ERROR_CLEAN

	return TRUE;
}

BOOL  CIOCPServer::InitializeIOCP(VOID)
{
	// ����IO��ɶ˿�
		// ��һ��������INVALID_HANDLE_VALUE(-1)����NULL(0)
	_M_hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	
	if (_M_hCompletionPort == NULL) {
		return FALSE;
	}

	// ѡ����ʵĹ����߳���
	SYSTEM_INFO SystemInfo;
	ZeroMemory(&SystemInfo, sizeof(SYSTEM_INFO));
	GetSystemInfo(&SystemInfo);

		// �����߳�������Ϊ1, ���ΪNumberOfProcessors * 2
	_M_ThreadPoolMin = 1;
	_M_ThreadPoolMax = SystemInfo.dwNumberOfProcessors * 2;

	// _M_ProcessorLowThreadsHold;

	ULONG WorkThreadCount = 2;
	HANDLE hWorkThread = NULL;


	// �������������߳�
	for (ULONG i = 0; i < WorkThreadCount; i++)
	{
		hWorkThread = (HANDLE)CreateThread(
								NULL,
								0,
								(LPTHREAD_START_ROUTINE)_WorkerThreadProc,
								(LPVOID)this,
								0,
								NULL
							);
		if (hWorkThread == NULL) {
			CloseHandle(_M_hCompletionPort);
			return FALSE;
		}
		_M_WorkThreadCount++;
		CloseHandle(hWorkThread);	// ���߳��޺����������ر��߳̾��
									// �ͷ��˶��̲߳������ں˶����̼߳�������
	}
	return TRUE;
}

VOID  CIOCPServer::OnAccept()
{
	int Res = 0;

	SOCKET sClient = INVALID_SOCKET;
	SOCKADDR_IN ClientAddress{ 0 };

	int ClientAddressSize = sizeof(SOCKADDR_IN);

	
	sClient = accept(_M_sListen, (SOCKADDR*)&ClientAddress, &ClientAddressSize);

	if (sClient == SOCKET_ERROR) {
		//�׽��ִ���
		return;
	}

	// Ϊ���û����������Ķ��� ContextObject
	PCONTEXT_OBJECT ContextObject = AllocateContextObj();
	if (ContextObject == NULL) {
		closesocket(sClient);
		sClient = INVALID_SOCKET;
		return;
	}

	ContextObject->ClientSocket = sClient;
	ContextObject->WsaRecvBuff.buf = (char*)ContextObject->_M_BufferData;
	ContextObject->WsaRecvBuff.len = sizeof(ContextObject->_M_BufferData);


	// ������ɶ˿�, �ȴ��¼����� (CompletionKey -- ContextObject)
	HANDLE h = CreateIoCompletionPort(
							(HANDLE)sClient,
							_M_hCompletionPort, 
							(ULONG_PTR)ContextObject, 
							0
						);

	if (h != _M_hCompletionPort)
	{
		// ���������쳣, �˳�
		delete ContextObject;
		ContextObject = NULL;
		if (sClient != INVALID_SOCKET)
		{
			closesocket(sClient);
			sClient = INVALID_SOCKET;
		}
		return;
	}

	// ������ƣ����Ӻ����޲�����ʱ��������Ͽ�����û�������
	_M_KeepLiveTime = 3;
	BOOL optVal = TRUE;

	/*if (setsockopt(ContextObject->ClientSocket, SOL_SOCKET, SO_KEEPALIVE,
		(char*)&optVal, sizeof(BOOL)) != 0)
	{

	}

	tcp_keepalive KeepAlive;

	KeepAlive.onoff = 1;
	KeepAlive.keepalivetime = _M_KeepLiveTime;
	KeepAlive.keepaliveinterval = 1000 * 10;
	WSAIoctl(
		ContextObject->ClientSocket,
		SIO_KEEPALIVE_VALS,
		&KeepAlive,
		sizeof(KeepAlive),
		NULL,
		0,
		(DWORD*)&optVal,
		0,
		NULL
	);*/

	// �����ٽ���
	_CLock CriticalSection(_M_CriticalSection);

	// ��������
	_M_ContextClientList.AddTail(ContextObject);

	OVERLAPPEDEX* OverLappedEx = new OVERLAPPEDEX(IOINIT);

	BOOL rVal = FALSE;

	// ��˿ڷ�������
	rVal = PostQueuedCompletionStatus(
							_M_hCompletionPort,
							0,
							(ULONG_PTR)ContextObject,
							&OverLappedEx->_M_ol
						);


	if (!rVal && WSAGetLastError() != ERROR_IO_PENDING)
	{
		// �ǳ�ʱ�쳣, ɾ��֮
		RemoveStaleContext(ContextObject);
		return;
	}
	PostRecv(ContextObject);
}

VOID  CIOCPServer::PostRecv(CONTEXT_OBJECT* ContextObj)
{
	OVERLAPPEDEX * OverLappedEx = new OVERLAPPEDEX(IORECV);

	DWORD RetLen;
	ULONG Flags = MSG_PARTIAL;

	// �첽Recv����, ���̷���
	int rVal = WSARecv(
					ContextObj->ClientSocket,
					&ContextObj->WsaRecvBuff,
					1,
					&RetLen,
					&Flags,
					&OverLappedEx->_M_ol,
					NULL
				);

	if (rVal == SOCKET_ERROR && WSAGetLastError()!= WSA_IO_PENDING)
	{
		// �ǳ�ʱ����, ɾ��֮
		RemoveStaleContext(ContextObj);
	}
}

BOOL  CIOCPServer::HandleIO(PACKET_TYPE PacketType, PCONTEXT_OBJECT ContextObject, DWORD TransferDataLength)
{
	BOOL rVal = FALSE;
	if (IOINIT == PacketType)
	{
		rVal = OnClientInitialize(ContextObject, TransferDataLength);
	}

	if (IORECV == PacketType)
	{
		rVal = OnClientReceive(ContextObject, TransferDataLength);
	}

	if (IOSEND == PacketType)
	{
		rVal = OnClientSend(ContextObject, TransferDataLength);
	}

	return rVal;
}

BOOL  CIOCPServer::OnClientInitialize(PCONTEXT_OBJECT ContextObject, DWORD TransferDataLength)
{
	// MessageBox(NULL,_T("IOCPServer"), _T("Initialize"), MB_OK);
	return TRUE;
}

BOOL  CIOCPServer::OnClientReceive(PCONTEXT_OBJECT ContextObject, DWORD TransferDataLength)
{
	_CLock Object(_M_CriticalSection);

	try
	{
		if (TransferDataLength == 0)
		{
			MessageBox(NULL, _T("Close Socket"), _T("Close Socket"), MB_OK);
			RemoveStaleContext(ContextObject);
			return FALSE;
		}
		ContextObject->_M_InCompressedBufferData.WriteArray((PBYTE)ContextObject->_M_BufferData, TransferDataLength);
		while (ContextObject->_M_InCompressedBufferData.GetArrayLength() > PACKET_HEADER_LENGTH)
		{
			char v10[PACKET_FLAG_LENGTH] = { 0 };
			CopyMemory(v10, ContextObject->_M_InCompressedBufferData.GetArray(), PACKET_FLAG_LENGTH);
		
			if (memcmp(_M_PacketFlagData, v10, PACKET_FLAG_LENGTH) != 0)
			{
				throw _T("Bad Buffer");
			}

			ULONG PackTotalLength = 0;
			CopyMemory(&PackTotalLength, ContextObject->_M_InCompressedBufferData.GetArray(PACKET_FLAG_LENGTH),
				sizeof(ULONG));
			
			if (PackTotalLength && 
				(ContextObject->_M_InCompressedBufferData.GetArrayLength()) >= PackTotalLength)
			{
				ULONG DeCompressedLength = 0;

				ContextObject->_M_InCompressedBufferData.ReadArray((PBYTE)v10, PACKET_FLAG_LENGTH);

				ContextObject->_M_InCompressedBufferData.ReadArray((PBYTE)&PackTotalLength, sizeof(ULONG));

				ContextObject->_M_InCompressedBufferData.ReadArray((PBYTE)&DeCompressedLength, sizeof(ULONG));

				ULONG CompressedLength = PackTotalLength - PACKET_HEADER_LENGTH;

				PBYTE CompressedData   = new BYTE[CompressedLength];

				PBYTE DeCompressedData = new BYTE[DeCompressedLength];

				if (CompressedData == NULL || DeCompressedData == NULL)
				{
					throw "Bad Allocate";
				}
				ContextObject->_M_InCompressedBufferData.ReadArray(CompressedData, CompressedLength);

				int  rVal = uncompress(DeCompressedData,&DeCompressedLength,CompressedData,CompressedLength);
				
				if (rVal == Z_OK)
				{
					ContextObject->_M_InDeCompressedBufferData.ClearArray();
					ContextObject->_M_InCompressedBufferData.ClearArray();

					ContextObject->_M_InDeCompressedBufferData.WriteArray(DeCompressedData, DeCompressedLength);

					delete[] CompressedData;
					delete[] DeCompressedData;

					_M_WindowNotifyProc(ContextObject);

				}
				else
				{
					delete[] CompressedData;
					delete[] DeCompressedData;
					throw "Bad Buffer";
				}
			}
			else
			{
				break;
			}

		}

		
		PostRecv(ContextObject);

	}
	catch(...)
	{
		ContextObject->_M_InCompressedBufferData.ClearArray();
		ContextObject->_M_InDeCompressedBufferData.ClearArray();

		PostRecv(ContextObject);
	}
	return TRUE;
}

VOID  CIOCPServer::OnClientPreSend(CONTEXT_OBJECT* ContextObject, PBYTE BufferData, ULONG BufferLength)
{
	if (ContextObject == NULL)
	{
		return;
	}
	try
	{
		if (BufferLength > 0)
		{
			unsigned long CompressedLength = (unsigned long)((double)BufferLength*1.001 + 12);
			LPBYTE        CompressedData   = new BYTE[CompressedLength];
			int rVal = compress(CompressedData, &CompressedLength, (PBYTE)BufferData, BufferLength);
			if (rVal != Z_OK)
			{
				delete[] CompressedData;
				return;
			}
			ULONG PackTotalLength = CompressedLength + PACKET_HEADER_LENGTH;

			// �������ݱ�
			ContextObject->_M_OutCompressedBufferData.WriteArray((LPBYTE)_M_PacketFlagData,PACKET_FLAG_LENGTH);
			// [Shine]
			ContextObject->_M_OutCompressedBufferData.WriteArray((LPBYTE)&PackTotalLength, sizeof(ULONG));
			// [Shine][PackTotalLength]
			ContextObject->_M_OutCompressedBufferData.WriteArray((LPBYTE)&BufferLength, sizeof(ULONG));
			// [Shine][PackTotalLength][BufferLength]
			ContextObject->_M_OutCompressedBufferData.WriteArray(CompressedData, CompressedLength);
			// [Shine][PackTotalLength][BufferLength][...ѹ���������]
			delete[] CompressedData;
		}

		OVERLAPPEDEX* OverlappedEx = new OVERLAPPEDEX(IOSEND);
		// ��������Ͷ�ݵ���ɶ˿�
		PostQueuedCompletionStatus(_M_hCompletionPort, 0, (DWORD)ContextObject, &OverlappedEx->_M_ol);

	}
	catch (...)
	{

	}
}

BOOL  CIOCPServer::OnClientSend(PCONTEXT_OBJECT ContextObject, DWORD TransferDataLength)
{
	try
	{
		DWORD Flags = MSG_PARTIAL;
		// ȥ���Ѿ����͵�����
		ContextObject->_M_OutCompressedBufferData.RemoveCompletedArray(TransferDataLength);

		if (ContextObject->_M_OutCompressedBufferData.GetArrayLength() == 0)
		{
			// ���ݷ������
			ContextObject->_M_OutCompressedBufferData.ClearArray();
			return true;
		}
		else
		{
			OVERLAPPEDEX* OverlappedEx = new OVERLAPPEDEX(IOSEND);

			ContextObject->WsaSendBuff.buf = (char*)ContextObject->_M_OutCompressedBufferData.GetArray();
			ContextObject->WsaSendBuff.len = ContextObject->_M_OutCompressedBufferData.GetArrayLength();

			int rVal = WSASend(ContextObject->ClientSocket,
				&ContextObject->WsaSendBuff,
				1,
				&ContextObject->WsaSendBuff.len,
				Flags,
				&OverlappedEx->_M_ol,
				NULL);
			if (rVal == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
			{
				RemoveStaleContext(ContextObject);
			}
		}
	}
	catch (...)
	{

	}
	return FALSE;
}

PCONTEXT_OBJECT CIOCPServer::AllocateContextObj()
{
	PCONTEXT_OBJECT ContextObj = NULL;

	// �����ٽ���
	_CLock Object(_M_CriticalSection);

	if (_M_ContextFreePoolList.IsEmpty() == FALSE)
	{
		// _M_ContextFreePoolList �ڴ�طǿ�
		// ��ȡ���ڴ�ص�һ���ڴ�
		ContextObj = _M_ContextFreePoolList.RemoveHead();
	}
	else
	{
		// �ڴ��Ϊ��
		// ��ϵͳ��������(new)
		ContextObj = new CONTEXT_OBJECT;
	}
	if (ContextObj != NULL)
	{
		// ��ʼ����Ա
		ContextObj->InitMember();
	}
	return ContextObj;
	// �����˳�, �Զ�����~_CLock() �˳��ٽ���
}

VOID  CIOCPServer::RemoveStaleContext(CONTEXT_OBJECT* ContextObj)
{
	_CLock CriticalSection(_M_CriticalSection);
	if (_M_ContextClientList.Find(ContextObj))
	{
		CancelIo((HANDLE)ContextObj->ClientSocket);
		closesocket(ContextObj->ClientSocket);
		ContextObj->ClientSocket = INVALID_SOCKET;
		while (!HasOverlappedIoCompleted((LPOVERLAPPED)ContextObj))
		{
			Sleep(0);
		}
		MoveContextToFreePool(ContextObj);
	}
}

VOID  CIOCPServer::MoveContextToFreePool(CONTEXT_OBJECT* ContextObj)
{
	_CLock CriticalSection(_M_CriticalSection);

	POSITION Postion = _M_ContextFreePoolList.Find(ContextObj);

	if (Postion)
	{
		memset(ContextObj->_M_BufferData, 0, PACKET_LENGTH);

		_M_ContextFreePoolList.AddTail(ContextObj);
		_M_ContextClientList.RemoveAt(Postion);

	}
}

DWORD CIOCPServer::_ListenThreadProc(LPVOID lpParam)
{

	CIOCPServer* This = (CIOCPServer*)lpParam;

	int EventIndex;
	WSANETWORKEVENTS NetWorkEvents;
	DWORD Res;

	while (TRUE)
	{
		// ����Ƿ��������
		EventIndex = WaitForSingleObject(This->_M_hKillEvent, 100);
		EventIndex = EventIndex - WAIT_OBJECT_0;
		if (EventIndex == 0) {
			// _M_hKillEvent �¼�����, �˳�����ѭ��,�˳������߳�
			break;
		}
		Res = WSAWaitForMultipleEvents(1, &This->_M_hListenEvent, FALSE, 100, FALSE);
		if (Res == WSA_WAIT_TIMEOUT) {
			// ��ʱ, ��������ѭ��, �ȴ���������
			continue;
		}

		// �յ���������, �ж��¼�����
		Res = WSAEnumNetworkEvents(
							This->_M_sListen, 
							This->_M_hListenEvent, 
							&NetWorkEvents
						);

		if (Res == SOCKET_ERROR) {
			// �׽��ִ���
			break;
		}
		if (NetWorkEvents.lNetworkEvents & FD_ACCEPT) {
			// FD_ACCEPT
			if (NetWorkEvents.iErrorCode[FD_ACCEPT_BIT] == 0) {
				// �޴���
				This->OnAccept();
			}
			else
				// �����쳣����, �˳�
				break;
		}
		else {
			// �����¼����ţ��¼�����ȴ���ǿͻ��������¼�
			// ���ӳ����쳣
			// �˳�
		}
	}
	return 0;
}

DWORD CIOCPServer::_WorkerThreadProc(LPVOID lpParam)
{
	CIOCPServer* This = (CIOCPServer*)lpParam;

	HANDLE hComletionPort = This->_M_hCompletionPort;
	DWORD  TransferDataLength     = 0;
	LPOVERLAPPED  Overlapped      = NULL;
	OVERLAPPEDEX* OverlappedEx    = NULL;
	PCONTEXT_OBJECT ContextObject = NULL;

	DWORD BusyThread = 0;
	BOOL v1   = FALSE;
	BOOL rVal = FALSE;

	InterlockedIncrement(&This->_M_CurrentThreadCount);
	InterlockedIncrement(&This->_M_BusyThreadCount);

	while (This->_M_TimeToKill == FALSE) {
		InterlockedDecrement(&This->_M_BusyThreadCount);
		rVal = GetQueuedCompletionStatus(
							hComletionPort,
							&TransferDataLength,
							(PULONG_PTR)&ContextObject,
							&Overlapped,
							60000
						);


		DWORD LastError = GetLastError();
		OverlappedEx = CONTAINING_RECORD(Overlapped, OVERLAPPEDEX, _M_ol);

		BusyThread = InterlockedIncrement(&This->_M_BusyThreadCount);

		if (!rVal && LastError != WAIT_TIMEOUT) {
			if (ContextObject&&This->_M_TimeToKill == FALSE
				&& TransferDataLength == 0) {
				// �׽��ֹر�
				This->RemoveStaleContext(ContextObject);
			}
			continue;
		}
		if (!v1)
		{
			if (BusyThread == This->_M_CurrentThreadCount)
			{
				if (BusyThread < This->_M_ThreadPoolMax)
				{
					if (ContextObject != NULL)
					{
						HANDLE hThread = CreateThread(
												NULL,
												0,
												(LPTHREAD_START_ROUTINE)_WorkerThreadProc,
												(void*)This,
												0,
												NULL
											);
						InterlockedIncrement(&This->_M_WorkThreadCount);

						CloseHandle(hThread);
					}
				}
			}

			if (!rVal && LastError == WAIT_TIMEOUT)
			{
				if (ContextObject == NULL)
				{
					if (This->_M_CurrentThreadCount > This->_M_ThreadPoolMin)
						break;

					v1 = TRUE;
				}
			}
		}

		if (!v1)
		{
			if (rVal && OverlappedEx != NULL && ContextObject != NULL)
			{
				try
				{
					This->HandleIO(OverlappedEx->_M_PackType, ContextObject, TransferDataLength);

					ContextObject = NULL;
				}
				catch (...)
				{
				}
			}
		}
		if (OverlappedEx)
		{
			delete OverlappedEx;
			OverlappedEx = NULL;
		}
	}
	
	InterlockedDecrement(&This->_M_WorkThreadCount);
	InterlockedDecrement(&This->_M_CurrentThreadCount);
	InterlockedDecrement(&This->_M_BusyThreadCount);

	return 0;
}
