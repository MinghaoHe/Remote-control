#pragma once
#include <Windows.h>
#include <tchar.h>
#include <cstdio>



struct _SERVER_CONNECT_INFO {

	DWORD    CheckFlag;
	CHAR     ServerIP[20];
	USHORT   ServerPort;
};

typedef void(*lpfn_ClientRun)(char* ServerIP, USHORT ServerPort);

