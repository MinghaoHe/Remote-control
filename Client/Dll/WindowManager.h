#pragma once
#include "Manager.h"
#include "IOCPClient.h"

class CWindowManager :
	public CManager
{
public:
	CWindowManager(CIOCPClient* IOCPClient);
	virtual ~CWindowManager();

public:
	void  PacketHandleIO(PBYTE BufferData, ULONG_PTR BufferLength);
	BOOL  SendClientWindowList();
	LPBYTE GetWindowList();

	static BOOL CALLBACK _EnumWindowProc(HWND hWnd, LPARAM lParam);

};

