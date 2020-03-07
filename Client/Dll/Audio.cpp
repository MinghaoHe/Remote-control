#include "stdafx.h"
#include "Audio.h"


CAudio::CAudio()
{
	_M_IsWavInUsed  = FALSE;
	_M_IsWavOutUsed = FALSE;
	_M_WavInIndex  = 0;
	_M_WavOutIndex = 0;

	_M_hStartRecord = CreateEvent(NULL, TRUE, FALSE, NULL);
	_M_hEventWavIn  = CreateEvent(NULL, TRUE, FALSE, NULL);

	ZeroMemory(&_M_GSMWavefmt, sizeof(GSM610WAVEFORMAT));
	
	// 初始化结构体
	_M_GSMWavefmt.wfx.wFormatTag = WAVE_FORMAT_GSM610;
	_M_GSMWavefmt.wfx.cbSize     = 2;
	_M_GSMWavefmt.wfx.nChannels  = 1;
	_M_GSMWavefmt.wfx.nSamplesPerSec  = 8000;
	_M_GSMWavefmt.wfx.nAvgBytesPerSec = 1625;
	_M_GSMWavefmt.wfx.nBlockAlign     = 65;
	_M_GSMWavefmt.wfx.wBitsPerSample  = 0;
	_M_GSMWavefmt.wSamplesPerBlock    = 320;

	_M_BufferLength = 1000;
	int i = 0;
	for (i = 0; i < 2; i++)
	{
		_M_InAudioData[i]   = new BYTE[_M_BufferLength];
		_M_InAudioHeader[i] = new WAVEHDR;

		_M_OutAudioData[i]   = new BYTE[_M_BufferLength];
		_M_OutAudioHeader[i] = new WAVEHDR;
	}
	return;
}

CAudio::~CAudio()
{
	int i;
	if (_M_IsWavInUsed)
	{
		waveInStop(_M_hWavIn);
		waveInReset(_M_hWavIn);

		for (i = 0; i < 2; i++)
		{
			waveInUnprepareHeader(
							_M_hWavIn, 
							_M_InAudioHeader[i], 
							sizeof(WAVEHDR)
						);
		}
		waveInClose(_M_hWavIn);

		TerminateThread(_M_hWavInThread, -1);
	}


	for (i = 0; i < 2; i++)
	{
		
		delete[] _M_InAudioData[i];
		_M_InAudioData[i] = NULL;
		delete _M_InAudioHeader[i];
		_M_InAudioHeader[i] = NULL;
	}

	

	CloseHandle(_M_hEventWavIn);
	CloseHandle(_M_hStartRecord);
	if (_M_hWavInThread != NULL)
	{
		CloseHandle(_M_hWavInThread);
		_M_hWavInThread = NULL;
	}
	if (_M_IsWavOutUsed)
	{
		waveOutReset(_M_hWavOut);

		for (i = 0; i < 2; i++)
		{
			waveOutUnprepareHeader(
								_M_hWavOut,
								_M_OutAudioHeader[i],
								sizeof(WAVEHDR)
							);
		}
		waveOutClose(_M_hWavOut);
	}

	for (i = 0; i < 2; i++)
	{
		delete[] _M_OutAudioData[i];
		_M_OutAudioData[i] = NULL;
		delete _M_OutAudioHeader[i];
		_M_OutAudioHeader[i] = NULL;
	}

	return;
}


LPBYTE CAudio::GetRecordData(LPDWORD BufferLength)
{
	if (_M_IsWavInUsed == FALSE && InitWavIn() == FALSE)
		return NULL;

	if (BufferLength == NULL)
		return NULL;

	SetEvent(_M_hStartRecord);

	WaitForSingleObject(_M_hEventWavIn, INFINITE);
	*BufferLength = _M_BufferLength;

	
	return _M_InAudioData[_M_WavInIndex];
}

BOOL   CAudio::InitWavIn()
{
	MMRESULT MMRes    = 0;
	DWORD    ThreadID = 0;

	_M_hWavInThread = CreateThread(
								NULL,
								0,
								(LPTHREAD_START_ROUTINE)_WavInProc,
								(LPVOID)this,
								CREATE_SUSPENDED,
								&ThreadID
							);
	MMRes = waveInOpen(
					&_M_hWavIn,
					(UINT)WAVE_MAPPER,
					&(_M_GSMWavefmt.wfx),
					(DWORD_PTR)ThreadID,
					(DWORD_PTR)0,
					CALLBACK_THREAD
				);

	if (MMRes != MMSYSERR_NOERROR)
		return FALSE;

	for (int i = 0; i < 2; i++)
	{
		_M_InAudioHeader[i]->lpData = (LPSTR)_M_InAudioData[i];
		_M_InAudioHeader[i]->dwBufferLength = _M_BufferLength;
		_M_InAudioHeader[i]->dwFlags = 0;
		_M_InAudioHeader[i]->dwLoops = 0;
		waveInPrepareHeader(_M_hWavIn, _M_InAudioHeader[i], sizeof(WAVEHDR));
	}

	waveInAddBuffer(_M_hWavIn, _M_InAudioHeader[_M_WavInIndex], sizeof(WAVEHDR));

	ResumeThread(_M_hWavInThread);
	waveInStart(_M_hWavIn);

	_M_IsWavInUsed = TRUE;

	return TRUE;
}

BOOL   CAudio::InitWavOut()
{
	if (!waveOutGetNumDevs())
		return FALSE;

	MMRESULT MMRes = 0;
	int i;

	for (i = 0; i < 2; i++)
		ZeroMemory(_M_OutAudioData[i], _M_BufferLength);

	MMRes = waveOutOpen(
					&_M_hWavOut,
					(UINT)WAVE_MAPPER,
					&(_M_GSMWavefmt.wfx),
					(DWORD_PTR)0,
					(DWORD_PTR)0,
					CALLBACK_NULL
				);

	if (MMRes != MMSYSERR_NOERROR)
		return FALSE;

	for (int i = 0; i < 2; i++)
	{
		_M_OutAudioHeader[i]->lpData = (LPSTR)_M_OutAudioData[i];
		_M_OutAudioHeader[i]->dwBufferLength = _M_BufferLength;
		_M_OutAudioHeader[i]->dwFlags = 0;
		_M_OutAudioHeader[i]->dwLoops = 0;
		waveOutPrepareHeader(_M_hWavOut, _M_OutAudioHeader[i], sizeof(WAVEHDR));
	}


	_M_IsWavOutUsed = TRUE;

	return TRUE;
}

BOOL   CAudio::PlayBuffer(LPBYTE BufferData, ULONG BufferLength)
{
	if (!_M_IsWavOutUsed && !InitWavOut())
		return FALSE;

	for (ULONG i = 0; i < BufferLength; i += _M_BufferLength)
	{
		memcpy(_M_OutAudioData[_M_WavOutIndex], BufferData, _M_BufferLength);
		waveOutWrite(_M_hWavOut, _M_OutAudioHeader[_M_WavOutIndex], sizeof(WAVEHDR));
		_M_WavOutIndex = 1 - _M_WavOutIndex;
	}
	return TRUE;
}

DWORD WINAPI CAudio::_WavInProc(LPVOID lParam)
{
	CAudio *This = (CAudio*)lParam;
	
	MSG Msg;

	while (GetMessage(&Msg, NULL, 0, 0))
	{
		if (Msg.message == MM_WIM_DATA)
		{
			SetEvent(This->_M_hEventWavIn);
			WaitForSingleObject(This->_M_hStartRecord, INFINITE);
			ResetEvent(This->_M_hEventWavIn);			
			Sleep(1);
			This->_M_WavInIndex = 1 - This->_M_WavInIndex;

			MMRESULT MMRes = waveInAddBuffer(
										This->_M_hWavIn,
										This->_M_InAudioHeader[This->_M_WavInIndex],
										sizeof(WAVEHDR)
									);
			if (MMRes != MMSYSERR_NOERROR)
				return -1;
		}

		if (Msg.message == MM_WIM_CLOSE)
			break;

		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	return 0;
}
