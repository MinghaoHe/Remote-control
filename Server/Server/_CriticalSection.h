#pragma once
#include <Windows.h>
class _CLock
{
public:
	_CLock(CRITICAL_SECTION&);
	virtual ~_CLock();

public:
	void Lock();
	void Unlock();

protected:
	CRITICAL_SECTION *_M_CriticalSection;
};