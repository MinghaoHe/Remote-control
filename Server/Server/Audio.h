#pragma once


#include <mmsystem.h>
#include <mmreg.h>
#pragma comment(lib,"Winmm.lib")
class CAudio
{
public:
	CAudio();
	virtual ~CAudio();
public:
	

	BOOL   InitWavIn();
	BOOL   InitWavOut();
	BOOL   PlayBuffer(LPBYTE BufferData, ULONG BufferLength);
	LPBYTE GetRecordData(LPDWORD BufferLength);

	static DWORD WINAPI _WavInProc(LPVOID lParam);

protected:
	GSM610WAVEFORMAT _M_GSMWavefmt;
	ULONG   _M_BufferLength;
	HANDLE  _M_hStartRecord;
	HANDLE  _M_hEventWavIn;

	HWAVEIN  _M_hWavIn;
	BOOL     _M_IsWavInUsed;
	DWORD    _M_WavInIndex;
	LPWAVEHDR _M_InAudioHeader[2];
	LPBYTE    _M_InAudioData[2];
	HANDLE    _M_hWavInThread;

	HWAVEOUT _M_hWavOut;
	BOOL     _M_IsWavOutUsed;
	DWORD    _M_WavOutIndex;
	LPWAVEHDR _M_OutAudioHeader[2];
	LPBYTE    _M_OutAudioData[2];

};

