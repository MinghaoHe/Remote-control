#pragma once

#include "IOCPClient.h"

#include "Common.h"


class CIOCPClient;

class CManager
{
public:
	CManager(CIOCPClient *IOCPClient);
	virtual ~CManager();

public:
	virtual VOID PacketHandleIO(PBYTE BufferData, ULONG_PTR BufferLength) = 0;

	BOOL EnableSeDebugPrivilege(const TCHAR* PrivilegeName, BOOL IsEnable);
	VOID WaitForServerDialogOpen();
	VOID NotifyDialogIsOpen();
	VOID ShowErrorMessage(TCHAR* Class, TCHAR *Message) {
		MessageBox(NULL, Message, Class, MB_OK);
	}

protected:
	CIOCPClient* _M_IOCPClient;
	HANDLE       _M_hEventDlgOpen;
};










