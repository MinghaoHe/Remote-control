#pragma once
#include "Manager.h"
#include <vector>



class CProcessManager :
	public CManager
{
private:
	typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

	typedef struct _PROCESS_INFO
	{
		DWORD  ProcessID;   // (HANDLE)
		TCHAR  ProcessImageName[MAX_PATH];
		TCHAR  ProcessFullPath[MAX_PATH];
		TCHAR  IsWow64[20];

	}PROCESS_INFO, *PPROCESS_INFO;

public:
	CProcessManager(CIOCPClient *ClientOB);
	virtual ~CProcessManager();

public:
	void PacketHandleIO(PBYTE BufferData, ULONG_PTR BufferLength);
	BOOL SendClientProcessList();
	int  EnumProcessList(std::vector<PROCESS_INFO>& ProcInfoVector);
	BOOL IsWow64Process_(HANDLE hProcess, BOOL *IsResult);
	BOOL IsValidWritePoint_(LPVOID VirtualAddress);

protected:
	LPFN_ISWOW64PROCESS _M_IsWow64Process;
	
};

