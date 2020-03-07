#pragma once

#include "stdafx.h"



class CConfigFile
{
public:
	CConfigFile();
	~CConfigFile();

	BOOL InitConfigFile();
	int  GetInt(TCHAR* MainKey, TCHAR* SubKey);
	BOOL SetInt(TCHAR* MainKey, TCHAR* SubKey, int Data);

private:
	TCHAR* _M_FileFullPath;
};