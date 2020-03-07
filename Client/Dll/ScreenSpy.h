#pragma once

#include <Windows.h>

class CScreenSpy
{
private:
	enum { ALGORITHM_DIFF = 1 };

public:
	CScreenSpy();
	CScreenSpy(ULONG BitmapCount);
	virtual ~CScreenSpy();

public:
	ULONG GetBitmapInfoSize();
	LPBITMAPINFO OnInitBitmapInfo(ULONG BitmapCount, ULONG FullWidth, ULONG FullHeight);
	LPBITMAPINFO GetBitmapInfoData();
	LPVOID GetFirstScreenData();
	ULONG  GetFirstScreenDataSize();
	LPVOID GetNextScreenData(ULONG *ScreenDataSize);

	VOID WriteScreenData(LPBYTE BufferData,ULONG BufferLength);
	VOID ScanScreen(HDC hDstDC,HDC hSrcDC, ULONG Width, ULONG Height);
	ULONG CompareBitmap(LPBYTE NextScreenData, LPBYTE FirstScreenData,
						LPBYTE BufferData,     ULONG  ScreenDataSize);

protected:
	BYTE	_M_Algorithm;

	ULONG	_M_BitmapCount;
	INT		_M_FullWidth;
	INT		_M_FullHeight;

	LPBITMAPINFO	_M_BitmapInfo;

	HWND	_M_hDesktopWnd;
	HDC		_M_hDesktopDC;
	HDC		_M_hDesktopMemoryDC;
	
	HBITMAP			_M_hBitmap;
	LPVOID			_M_BitmapData;
	
	HBITMAP 	    _M_hDiffBitmap;
	LPVOID			_M_DiffBitmapData;
	HDC             _M_hDiffMemoryDC;

	DWORD      _M_offset;
	BYTE       *_M_BufferData;
};

