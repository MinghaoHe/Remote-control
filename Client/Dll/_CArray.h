#pragma once
#include <windows.h>
#include <cmath>

class _declspec(dllexport) _CArray
{
public:
	_CArray();
	virtual ~_CArray();

public:
	BOOL       WriteArray(PUINT8 BufferData, ULONG_PTR BufferLength);
	ULONG_PTR  ReAllocateArray(ULONG_PTR BufferLength);
	ULONG_PTR  GetArrayLength();
	ULONG_PTR  GetArrayMaxLength();
	PUINT8     GetArray(ULONG_PTR Pos = 0);
	VOID	   ClearArray();
	ULONG_PTR  DeAllocateArray(ULONG_PTR BufferLength);
	ULONG_PTR  ReadArray(PUINT8 BufferData, ULONG_PTR BufferLength);
	ULONG_PTR  RemoveCompletedArray(ULONG_PTR BufferLength);



private:
	PUINT8		_M_BufferData;
	PUINT8		_M_CheckPtr;
	ULONG_PTR	_M_MaxLength;
	CRITICAL_SECTION _M_cs;
};

