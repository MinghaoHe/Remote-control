#include "stdafx.h"
#include "Manager.h"
#include "InstanceMessage.h"
#include "IOCPClient.h"



CManager::CManager(CIOCPClient *IOCPClient)
{
	_M_IOCPClient = IOCPClient;
	IOCPClient->SetManagerObject(this);
	_M_hEventDlgOpen = CreateEvent(NULL, TRUE, FALSE, NULL);
}

CManager::~CManager()
{
	// do nothing
}


VOID CManager::NotifyDialogIsOpen()
{
	SetEvent(_M_hEventDlgOpen);

	return;
}

VOID CManager::WaitForServerDialogOpen()
{
	
	WaitForSingleObject(_M_hEventDlgOpen, INFINITE);
	// Sleep 是必须的, 因为远程窗口从InitDialog中发送COMMAND_NEXT到显示还要一段时间
	
	Sleep(150);

	return;
}

BOOL CManager::EnableSeDebugPrivilege(const TCHAR* DebugValue, BOOL IsEnable)
{
	HANDLE hProcess = GetCurrentProcess();
	HANDLE hToken = NULL;
	TOKEN_PRIVILEGES TokenPrivileges;
	ZeroMemory(&TokenPrivileges, sizeof(TokenPrivileges));
	if (!OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		return FALSE;
	}

	LUID Luid;
	ZeroMemory(&Luid, sizeof(Luid));
	if (!LookupPrivilegeValue(NULL, DebugValue, &Luid))
	{
		CloseHandle(hToken);
		hToken = NULL;
		return FALSE;
	}

	TokenPrivileges.PrivilegeCount = 1;
	TokenPrivileges.Privileges[0].Attributes = IsEnable == TRUE ? SE_PRIVILEGE_ENABLED : 0;
	TokenPrivileges.Privileges[0].Luid = Luid;

	if (!AdjustTokenPrivileges(hToken, FALSE, &TokenPrivileges,
				sizeof(TOKEN_PRIVILEGES), NULL, NULL))
	{
		CloseHandle(hToken);
		hToken = NULL;
		return FALSE;
	}
	CloseHandle(hToken);
	hToken = NULL;

	return TRUE;
}




