#pragma once

#include <WinSock2.h>
#include <mstcpip.h>
#include "_CriticalSection.h"
#include "_CArray.h"

#include "zconf.h"
#include "zlib.h"


#pragma comment(lib,"WS2_32.lib")




enum {
	PACKET_FLAG_LENGTH   = 5,
	PACKET_HEADER_LENGTH = 13
};

enum { PACKET_LENGTH = 0x2000 };

enum { MAX_RECV_BUFFER = 0x2000 };

enum { MAX_SEND_BUFFER = 0x2000 };

typedef struct _CONTEXT_OBJECT_
{

	SOCKET ClientSocket;
	WSABUF WsaRecvBuff;
	WSABUF WsaSendBuff;
	_CArray _M_InCompressedBufferData;
	_CArray _M_InDeCompressedBufferData;
	_CArray _M_OutCompressedBufferData;
	char _M_BufferData[PACKET_LENGTH];

	void InitMember()
	{
		ClientSocket = INVALID_SOCKET;
		memset(_M_BufferData, 0, sizeof(char)*PACKET_LENGTH);
		memset(&WsaRecvBuff,  0, sizeof(WSABUF));
		memset(&WsaSendBuff,  0, sizeof(WSABUF));
		DlgId = 0;
		DlgHandle = NULL;
	}

	DWORD  DlgId;
	HANDLE DlgHandle;
}CONTEXT_OBJECT, *PCONTEXT_OBJECT;



enum PACKET_TYPE { IOINIT, IORECV, IOSEND, IOIDLE };

class OVERLAPPEDEX
{
public:
	
	OVERLAPPED  _M_ol;			// ��ɶ˿�
	PACKET_TYPE _M_PackType;	// �����������

	OVERLAPPEDEX(PACKET_TYPE PackType)
	{
		ZeroMemory(this, sizeof(OVERLAPPEDEX));
		_M_PackType = PackType;
	}
};

class CIOCPServer
{
private:
	typedef CList<PCONTEXT_OBJECT> ContextObjectList;
	typedef void (CALLBACK *lpfn_WindowNotifyProc)(PCONTEXT_OBJECT ContextObject);

public:
	CIOCPServer();
	~CIOCPServer();

public:
	BOOL StartServer(USHORT nPort, INT Backlog, lpfn_WindowNotifyProc WindowNotifyProc);
	BOOL InitializeIOCP(VOID);

	BOOL HandleIO(PACKET_TYPE PacketType, PCONTEXT_OBJECT ContextObject, DWORD TransferDataLength);

	BOOL OnClientInitialize(PCONTEXT_OBJECT ContextObject, DWORD TransferDataLength);
	BOOL OnClientReceive   (PCONTEXT_OBJECT ContextObject, DWORD TransferDataLength);
	BOOL OnClientSend      (PCONTEXT_OBJECT ContextObject, DWORD TransferDataLength);
	VOID OnClientPreSend   (CONTEXT_OBJECT* ContextObject, PBYTE BufferData, ULONG BufferLength);
	
	VOID OnAccept();
	VOID PostRecv(CONTEXT_OBJECT* ContectObj);

	PCONTEXT_OBJECT AllocateContextObj();
	VOID RemoveStaleContext   (CONTEXT_OBJECT* ContextObj);
	VOID MoveContextToFreePool(CONTEXT_OBJECT* ContextObj);

public:
	static DWORD WINAPI _ListenThreadProc(LPVOID lpParam);
	static DWORD WINAPI _WorkerThreadProc(LPVOID lpParam);
	
private:
	SOCKET   _M_sListen;					// �����׽���

	WSAEVENT _M_hListenEvent;				// �����¼�
	WSAEVENT _M_hKillEvent;					// ֪ͨ�����߳̽���

	LONG     _M_KeepLiveTime;
	BOOL     _M_TimeToKill;

	HANDLE   _M_hListenThread;				// �����߳�
	HANDLE   _M_hCompletionPort;			// ��ɶ˿�

	// IOCPServer ��û�������õ�Windows�̳߳ػ���( QueueUserWorkItem() )
	// ������¼�����̵߳���Ϣ�����ƹ����߳���
	DWORD _M_ThreadPoolMin;
	DWORD _M_ThreadPoolMax;

	DWORD _M_WorkThreadCount;				// ���ڹ������߳���
	ULONG _M_CurrentThreadCount;
	ULONG _M_BusyThreadCount;

	CHAR  _M_PacketFlagData[PACKET_FLAG_LENGTH];	// ���ݰ�ͷ

	CRITICAL_SECTION  _M_CriticalSection;
	PCONTEXT_OBJECT   _M_ContextFreeList;
	ContextObjectList _M_ContextFreePoolList;
	ContextObjectList _M_ContextClientList;
	
	lpfn_WindowNotifyProc _M_WindowNotifyProc;	// ����֪ͨ����

};


