#pragma once
#include "Manager.h"
class CRegistryManager :
	public CManager
{
public:
	CRegistryManager(CIOCPClient *IOCPClient);
	virtual ~CRegistryManager();

public:
	void  PacketHandleIO(PBYTE BufferData, ULONG_PTR BufferLength);
	VOID  FindRegistryData(BYTE IsToken, TCHAR* FileFullPath);

};

