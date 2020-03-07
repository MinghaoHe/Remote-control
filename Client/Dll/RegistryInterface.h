#pragma once
#include <Windows.h>
#include <cstdlib>
#include <tchar.h>

#pragma warning (disable:4996)

class CRegistryInterface
{
private:
	enum
	{
		MHKEY_CLASSES_ROOT , MHKEY_CURRENT_USER,
		MHKEY_LOCAL_MACHINE, MHKEY_USERS,
		MHKEY_CURRENT_CONFIG
	};
	enum
	{
		MREG_SZ, MREG_DWORD,
		MREG_BINARY, MREG_EXPAND_SZ
	};
	struct REGMSG
	{
		int   count;
		DWORD size;
		DWORD valsize;
	};
public:
	CRegistryInterface(BYTE IsToken);
	virtual ~CRegistryInterface();

public:
	VOID   SetPath(TCHAR* FileFullPath);
	TCHAR* FindPath();
	TCHAR* FindKey();

protected:
	HKEY  _M_hKey;
	TCHAR _M_KeyPath[MAX_PATH];

};

