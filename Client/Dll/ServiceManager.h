#pragma once
#include "Manager.h"


class CServiceManager :
	public CManager
{
public:
	CServiceManager(CIOCPClient *IOCPClient);
	virtual ~CServiceManager();

public:
	VOID  SendClientServiceList();
	BYTE* GetClientServiceList();
	void  PacketHandleIO(PBYTE BufferData, ULONG_PTR BufferLength);
	void  ConfigClientService(PBYTE BufferData, ULONG BufferLength);

protected:
	SC_HANDLE _M_hServiceManager;
};

