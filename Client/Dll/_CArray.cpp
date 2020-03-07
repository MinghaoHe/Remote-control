#include "stdafx.h"
#include "_CArray.h"

#define U_PAGE_ALIGNMENT	3
#define F_PAGE_ALIGNMENT	3.0

_CArray::_CArray()
{
	_M_MaxLength  = 0;
	_M_CheckPtr   = NULL;
	_M_BufferData = NULL;
	InitializeCriticalSection(&_M_cs);
}


_CArray::~_CArray()
{
	if (_M_BufferData)
	{
		VirtualFree(_M_BufferData, 0, MEM_RELEASE);
		_M_BufferData = NULL;
	}
	DeleteCriticalSection(&_M_cs);
	_M_CheckPtr = NULL;
	_M_MaxLength = 0;
}

BOOL _CArray::WriteArray(PUINT8 BufferData, ULONG_PTR BufferLength)
{
	EnterCriticalSection(&_M_cs);
	if (ReAllocateArray(BufferLength + GetArrayMaxLength()) == (ULONG_PTR)-1)
	{
		LeaveCriticalSection(&_M_cs);
		return FALSE;
	}

	CopyMemory(_M_CheckPtr, BufferData, BufferLength);

	_M_CheckPtr += BufferLength;
	LeaveCriticalSection(&_M_cs);

	return TRUE;
}
ULONG_PTR _CArray::ReAllocateArray(ULONG_PTR BufferLength)
{
	if (BufferLength < GetArrayMaxLength())
		return 0;
	ULONG_PTR v7 = (ULONG_PTR)ceil(BufferLength / F_PAGE_ALIGNMENT) * U_PAGE_ALIGNMENT;
	PUINT8 v5 = (PUINT8)VirtualAlloc(NULL, v7, MEM_COMMIT, PAGE_READWRITE);
	if (v5 == NULL)
	{
		return -1;
	}
	ULONG_PTR v3 = GetArrayLength();
	CopyMemory(v5, _M_BufferData, v3);
	if (_M_BufferData)
	{
		VirtualFree(_M_BufferData, 0, MEM_RELEASE);
	}
	_M_BufferData = v5;
	_M_CheckPtr = _M_BufferData + v3;
	_M_MaxLength = v7;

	return _M_MaxLength;
}

ULONG_PTR _CArray::GetArrayLength()
{
	if (_M_BufferData == NULL)
		return 0;
	return (ULONG_PTR)_M_CheckPtr - (ULONG_PTR)_M_BufferData;
}
ULONG_PTR _CArray::GetArrayMaxLength()
{
	return _M_MaxLength;
}
PUINT8 _CArray::GetArray(ULONG_PTR Pos)
{
	if (_M_BufferData == NULL)
	{
		return NULL;
	}
	if (Pos >= GetArrayLength())
	{
		return NULL;
	}
	return _M_BufferData + Pos;
}
VOID _CArray::ClearArray()
{
	EnterCriticalSection(&_M_cs);
	_M_CheckPtr = _M_BufferData;
	DeAllocateArray(1024);
	LeaveCriticalSection(&_M_cs);
}
ULONG_PTR _CArray::DeAllocateArray(ULONG_PTR BufferLength)
{
	if (BufferLength < GetArrayLength())
	{
		return 0;
	}
	ULONG_PTR v7 = (ULONG_PTR)ceil(BufferLength / F_PAGE_ALIGNMENT)*U_PAGE_ALIGNMENT;

	if (GetArrayMaxLength() <= v7)
	{
		return 0;
	}

	PUINT8 v5 = (PUINT8)VirtualAlloc(NULL, v7, MEM_COMMIT, PAGE_READWRITE);

	ULONG_PTR v3 = GetArrayLength();
	CopyMemory(v5, _M_BufferData, v3);
	VirtualFree(_M_BufferData, 0, MEM_RELEASE);

	_M_BufferData = v5;
	_M_CheckPtr = _M_BufferData + v3;
	_M_MaxLength = v7;
	return _M_MaxLength;
}
ULONG_PTR _CArray::ReadArray(PUINT8 BufferData, ULONG_PTR BufferLength)
{
	EnterCriticalSection(&_M_cs);
	if (BufferLength > GetArrayMaxLength())
	{
		LeaveCriticalSection(&_M_cs);
		return 0;
	}

	if (BufferLength > GetArrayLength())
	{
		BufferLength = GetArrayLength();
	}

	if (BufferLength)
	{
		CopyMemory(BufferData, _M_BufferData, BufferLength);
		MoveMemory(_M_BufferData, _M_BufferData + BufferLength, GetArrayMaxLength() - BufferLength);
		_M_CheckPtr -= BufferLength;
	}
	DeAllocateArray(GetArrayLength());
	LeaveCriticalSection(&_M_cs);
	return BufferLength;

}
ULONG_PTR _CArray::RemoveCompletedArray(ULONG_PTR BufferLength)
{
	EnterCriticalSection(&_M_cs);
	if (BufferLength > GetArrayMaxLength())
	{
		LeaveCriticalSection(&_M_cs);
		return 0;
	}

	if (BufferLength > GetArrayLength())
	{
		BufferLength = GetArrayLength();
	}

	if (BufferLength)
	{
		MoveMemory(_M_BufferData, _M_BufferData + BufferLength, GetArrayMaxLength() - BufferLength);
		_M_CheckPtr -= BufferLength;
	}
	DeAllocateArray(GetArrayLength());
	LeaveCriticalSection(&_M_cs);
	return BufferLength;
}