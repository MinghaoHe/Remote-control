// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "dll.h"
#include "Login.h"
#include "IOCPClient.h"
#include "Manager.h"
#include "KernelManager.h"
#include <tchar.h>
#include <cstdio>

CHAR      __ServerIP[MAX_PATH] = { 0 };
USHORT    __ServerPort = 0;
HINSTANCE __hInstance  = NULL;


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD   ul_reason_for_call,
                       LPVOID  lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		// 代表当前exe(加载当前Dll模块)
		__hInstance = (HINSTANCE)hModule;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


void ClientRun(CHAR* ServerIP, USHORT ServerPort)
{
	memcpy(__ServerIP, ServerIP, strlen(ServerIP));
	__ServerPort = ServerPort;
	
	HANDLE hThread = CreateThread(
							NULL,
							0,
							(LPTHREAD_START_ROUTINE)_WorkerThreadProc,
							NULL,
							0,
							NULL
						);

	WaitForSingleObject(hThread, INFINITE);
	_tprintf(_T("\nClient disconnects\n"));
	
	if (hThread != NULL)
	{
		CloseHandle(hThread);
	}
}

DWORD WINAPI _WorkerThreadProc(LPVOID lParam)
{
	CIOCPClient IOCPClient;
	BOOL IsOk = FALSE;
	while (TRUE)
	{
		if (IsOk == TRUE)
		{
			break;
		}
		DWORD TickCount = GetTickCount();
		if (!IOCPClient.ConnectServer(__ServerIP, __ServerPort))
		{
			continue;
		}

		SendLoginInfo(&IOCPClient, GetTickCount() - TickCount);
		
		CKernelManager KernelManagerOB(&IOCPClient);


		int rVal = 0;

		do
		{
			rVal = WaitForSingleObject(IOCPClient._M_hEvent, 100);
			rVal = rVal - WAIT_OBJECT_0;

			IsOk = TRUE;
		} while (rVal != 0);
		IsOk = TRUE;
	}
	return 0;
}