#include "stdafx.h"

#include "IOCPClient.h"



CIOCPClient::CIOCPClient()
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	_M_IsRecv = TRUE;

	_M_sClient = INVALID_SOCKET;
	_M_hWorkThread = NULL;

	_M_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	memcpy(_M_PacketHeaderFlag, "Shine", PACKET_FLAG_LENGTH);

}

CIOCPClient::~CIOCPClient()
{
	if (_M_sClient != INVALID_SOCKET)
	{
		closesocket(_M_sClient);
		_M_sClient;
	}
	if (_M_hWorkThread != NULL)
	{
		CloseHandle(_M_hWorkThread);
		_M_hWorkThread = NULL;
	}
	if (_M_hEvent != NULL)
	{
		CloseHandle(_M_hEvent);
		_M_hEvent = NULL;
	}

	WSACleanup();

}


int   CIOCPClient::OnServerSending(char* BufferData, ULONG BufferLength)
{
	_M_OutCompressedBufferData.ClearArray();
	if (BufferLength > 0)
	{
		ULONG  CompressedLength = (ULONG)((double)BufferLength*1.001 + 12);
		LPBYTE CompressedData = new BYTE[CompressedLength];
		if (CompressedData == NULL)
		{
			return 0;
		}

		int rVal = compress(CompressedData, &CompressedLength, (BYTE*)BufferData, BufferLength);

		if (rVal = Z_OK)
		{
			delete[] CompressedData;
			CompressedData = NULL;
			return FALSE;
		}

		ULONG PackTotalLength = CompressedLength + PACKET_HEADER_LENGTH;
		// [Shine]
		_M_OutCompressedBufferData.WriteArray((PBYTE)_M_PacketHeaderFlag, sizeof(_M_PacketHeaderFlag));
		// [Shine][PacketSize]
		_M_OutCompressedBufferData.WriteArray((PBYTE)&PackTotalLength, sizeof(ULONG));
		// [Shine][PacketSize][BufferLength]
		_M_OutCompressedBufferData.WriteArray((PBYTE)&BufferLength, sizeof(ULONG));
		// [Shine][PacketSize][bufferLength][..... Ñ¹ËõµÄÊý¾Ý]
		_M_OutCompressedBufferData.WriteArray(CompressedData, CompressedLength);

		delete[] CompressedData;
		CompressedData = NULL;

	}

	return SendwithSplit(
					(char*)_M_OutCompressedBufferData.GetArray(),
					_M_OutCompressedBufferData.GetArrayLength(),
					MAX_SEND_BUFFER
				);
}

VOID  CIOCPClient::OnServerRecv (CHAR* BufferData, ULONG BufferLength)
{
	try
	{
		if (BufferLength == 0)
		{
			Disconnect();
			return;
		}
		_M_InCompressedBufferData.WriteArray((LPBYTE)BufferData, BufferLength);
		while (_M_InCompressedBufferData.GetArrayLength() > PACKET_HEADER_LENGTH)
		{
			char v10[PACKET_FLAG_LENGTH] = { 0 };
			CopyMemory(v10, _M_InCompressedBufferData.GetArray(), PACKET_FLAG_LENGTH);
			if (memcmp(_M_PacketHeaderFlag, v10, PACKET_FLAG_LENGTH != 0))
			{
				throw "Bad Buffer";
			}
			ULONG PackTotalLength = 0;
			CopyMemory(&PackTotalLength, _M_InCompressedBufferData.GetArray(PACKET_FLAG_LENGTH), sizeof(ULONG));

			if (PackTotalLength &&
				(_M_InCompressedBufferData.GetArrayLength()) >= PackTotalLength)
			{
				_M_InCompressedBufferData.ReadArray((PBYTE)v10, PACKET_FLAG_LENGTH);
				_M_InCompressedBufferData.ReadArray((PBYTE)&PackTotalLength, sizeof(ULONG));

				ULONG DeCompressedLength = 0;

				_M_InCompressedBufferData.ReadArray((PBYTE)&DeCompressedLength, sizeof(ULONG));

				ULONG CompressedLength = PackTotalLength - PACKET_HEADER_LENGTH;
				PBYTE CompressedData = new BYTE[CompressedLength];
				PBYTE DeCompressedData = new BYTE[DeCompressedLength];
				if (CompressedData == NULL || DeCompressedData == NULL)
				{
					throw "Bad Alloc";
				}
				_M_InCompressedBufferData.ReadArray(CompressedData, CompressedLength);
				int rVal = uncompress(DeCompressedData, &DeCompressedLength, CompressedData, CompressedLength);

				if (rVal == Z_OK)
				{
					_M_InCompressedBufferData.ClearArray();
					_M_InDeCompressedBufferData.ClearArray();
					_M_InDeCompressedBufferData.WriteArray(DeCompressedData, DeCompressedLength);

					delete[] CompressedData;
					delete[] DeCompressedData;

					_M_ManagerOB->PacketHandleIO((PBYTE)_M_InDeCompressedBufferData.GetArray(0),
						_M_InDeCompressedBufferData.GetArrayLength());
				}
				else
				{
					delete[] CompressedData;
					delete[] DeCompressedData;
					throw "Bad Buffer";
				}
			}
			else
				break;

		}

	}
	catch (...)
	{
		_M_InCompressedBufferData.ClearArray();
		_M_InDeCompressedBufferData.ClearArray();
	}
}

BOOL  CIOCPClient::SendwithSplit(CHAR* BufferData, ULONG BufferLength, ULONG SplitLength)
{
	int rLen = 0;
	const char* Travel = (char*)BufferData;

	ULONG i, j;

	ULONG Sended    = 0;
	ULONG SendRetry = 15;

	for (i = BufferLength; i >= SplitLength; i -= SplitLength)
	{
		for (j = 0; j < SendRetry; j++)
		{
			rLen = send(_M_sClient, Travel, SplitLength, 0);
			if (rLen > 0)
			{
				break;
			}
		}
		if (i == SendRetry)
		{
			return FALSE;
		}
		Sended += SplitLength;
		Travel += rLen;
		Sleep(15);
	}

	if (i > 0)
	{
		for (ULONG j = 0; j < SendRetry; j++)
		{
			rLen = send(_M_sClient, (char*)Travel, i, 0);

			Sleep(15);
			if (rLen > 0)
			{
				break;
			}
		}
		if (j == SendRetry)
		{
			return FALSE;
		}
		Sended += rLen;
	}
	if (Sended == BufferLength)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL  CIOCPClient::ConnectServer(CHAR* ServerIP, USHORT ServerPort)
{
	_M_sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_M_sClient == SOCKET_ERROR) {
		return FALSE;
	}

	sockaddr_in ServerAddress;
	ServerAddress.sin_family = AF_INET;
	ServerAddress.sin_port = htons(ServerPort);
	ServerAddress.sin_addr.S_un.S_addr = inet_addr(ServerIP);
	if (connect(_M_sClient, (sockaddr*)&ServerAddress, sizeof(sockaddr_in)) == SOCKET_ERROR)
	{
		if (_M_sClient != INVALID_SOCKET) {
			closesocket(_M_sClient);
			_M_sClient = INVALID_SOCKET;
		}
		return FALSE;
	}
	_M_hWorkThread =
		(HANDLE)CreateThread(
					NULL,
					0,
					(LPTHREAD_START_ROUTINE)_WorkerThreadProc,
					(LPVOID)this,
					0,
					NULL
				);

	return TRUE;
}

VOID  CIOCPClient::Disconnect()
{
	CancelIo((HANDLE)_M_sClient);
	InterlockedExchange((LPLONG)&_M_IsRecv, FALSE);
	closesocket(_M_sClient);
	SetEvent(_M_hEvent);
	_M_sClient = INVALID_SOCKET;
	return;
}

DWORD CIOCPClient::_WorkerThreadProc(LPVOID lParam)
{
	CIOCPClient *This = (CIOCPClient*)lParam;

	fd_set fdOld;
	fd_set fdNew;
	FD_ZERO(&fdOld);
	FD_ZERO(&fdNew);
	CHAR BufferData[MAX_RECV_BUFFER] = { 0 };

	FD_SET(This->_M_sClient, &fdOld);

	while (This->_M_IsRecv)
	{
		fdNew = fdOld;
		int rVal = select(NULL, &fdNew, NULL, NULL, NULL);

		if (rVal == SOCKET_ERROR)
		{
			This->Disconnect();
			printf("Server Close\r\n");
			break;
		}
		if (rVal > 0)
		{
			memset(BufferData, 0, sizeof(BufferData));
			int BufferLength = recv(This->_M_sClient, BufferData, sizeof(BufferData), 0);
			if (BufferLength > 0)
			{
				This->OnServerRecv((CHAR*)BufferData, BufferLength);

			}
			else
			{
				printf("WorkThreadProc: be closed\r\n");
				This->Disconnect();
				break;
			}
		}
	}
	return 0;
}




