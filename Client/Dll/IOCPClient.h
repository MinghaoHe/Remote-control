#pragma once

#include <WinSock2.h>
#include <cstdio>
#include "_CArray.h"
#include "zconf.h"
#include "zlib.h"
#include "Manager.h"


#pragma comment(lib,"WS2_32.lib")

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#pragma warning(disable : 4996)

enum {
	PACKET_FLAG_LENGTH = 5,
	PACKET_HEADER_LENGTH = 13
};

enum { PACKET_LENGTH = 0x2000 };

enum { MAX_RECV_BUFFER = 0x2000 };

enum { MAX_SEND_BUFFER = 0x2000 };



class CManager;

class CIOCPClient
{
public:
	CIOCPClient();
	virtual ~CIOCPClient();

public:
	BOOL ConnectServer(CHAR* ServerIP,USHORT ServerPort);
	VOID Disconnect();
	int  OnServerSending(char* bufferData, ULONG BufferLength);
	BOOL SendwithSplit(CHAR* BufferData, ULONG BufferLength, ULONG SplitLength);
	VOID OnServerRecv(CHAR* BufferData, ULONG BufferLength);
	VOID SetManagerObject(CManager *Manager)
	{
		_M_ManagerOB = Manager;
	}
	VOID WaitForEvent()
	{
		WaitForSingleObject(_M_hEvent, INFINITE);
	}

	static DWORD WINAPI _WorkerThreadProc(LPVOID lParam);

public:
	WSAEVENT _M_hEvent;
	SOCKET   _M_sClient;

protected:
	BOOL     _M_IsRecv;
	HANDLE   _M_hWorkThread;

	// 向子功能模块发送消息( 多态 PacketIO() )
	CManager *_M_ManagerOB;
	
	_CArray _M_OutCompressedBufferData;
	_CArray _M_InCompressedBufferData;
	_CArray _M_InDeCompressedBufferData;
	TCHAR   _M_PacketHeaderFlag[PACKET_FLAG_LENGTH];
	
};

