
#include "stdafx.h"
#include "OptionConfigFile.h"



CConfigFile::CConfigFile()
{
	InitConfigFile();
}

CConfigFile::~CConfigFile()
{
}

BOOL CConfigFile::InitConfigFile()
{
	TCHAR* FileFullPath = new TCHAR[MAX_PATH];
	ZeroMemory(FileFullPath, sizeof(FileFullPath));

	::GetModuleFileName(NULL, FileFullPath, MAX_PATH);

	TCHAR* buff = _tcsstr(FileFullPath, _T("."));
	if (buff != NULL)
	{
		*buff = _T('\0');
		_tcscat_s(FileFullPath, MAX_PATH, _T(".ini"));
	}

	HANDLE hFile = CreateFile(
						FileFullPath, 
						GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE,
						NULL, 
						OPEN_ALWAYS, 
						FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_HIDDEN, 
						NULL
					);

	if (hFile == INVALID_HANDLE_VALUE) {
		return FALSE;
	}

	_M_FileFullPath = FileFullPath;

	ULONG HighLength = 0;
	ULONG LowLength  = GetFileSize(hFile, &HighLength);
	if (LowLength > 0 || HighLength > 0)
	{
		CloseHandle(hFile);

		return FALSE;
	}

	CloseHandle(hFile);

	WritePrivateProfileString(_T("Option"), _T("Port"   ), _T("2356"), FileFullPath);
	WritePrivateProfileString(_T("Option"), _T("Backlog"), _T("10"  ), FileFullPath);
	
	return TRUE;
}

int  CConfigFile::GetInt(TCHAR* MainKey, TCHAR* SubKey)
{
	return (::GetPrivateProfileInt(MainKey, SubKey, 0, _M_FileFullPath));
}

BOOL CConfigFile::SetInt(TCHAR* MainKey, TCHAR* SubKey, int Data)
{
	TCHAR buff[20];
	_itot_s(Data, buff, 10);
	return (::WritePrivateProfileString(MainKey, SubKey, buff, _M_FileFullPath));
}