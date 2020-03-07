#pragma once

#include "Manager.h"
#include "KernelManager.h"
#include "Common.h"


#define WIN_WIDTH  220
#define WIN_HEIGHT 150


enum 
{
	ID_TIMER_POP_WINDOW = 1,
	ID_TIMER_DELAY_DISPLAY,
	ID_TIMER_CLOSE_WINDOW
};

class CClientInstantManager : public CManager
{
public:
	CClientInstantManager(CIOCPClient* ClientOB);
	virtual ~CClientInstantManager();

public:
	virtual VOID PacketHandleIO(PBYTE BufferData, ULONG_PTR BufferLength);

};

VOID OnInitDialog(HWND DlgHwnd);

VOID OnTimerDialog(HWND DlgHwd);


INT CALLBACK _DialogProc(HWND DlgHwnd, UINT Msg, WPARAM wParam, LPARAM lParam);


