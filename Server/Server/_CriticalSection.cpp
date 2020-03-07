#include "stdafx.h"
#include "_CriticalSection.h"

_CLock::_CLock(CRITICAL_SECTION& CriticalSection)
{
	_M_CriticalSection = &CriticalSection;
	Lock();
}

_CLock::~_CLock()
{
	Unlock();
}


void _CLock::Lock()
{
	EnterCriticalSection(_M_CriticalSection);
}

void _CLock::Unlock()
{
	LeaveCriticalSection(_M_CriticalSection);
}