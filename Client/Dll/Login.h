#pragma once
#include "IOCPClient.h"
#include <Vfw.h>
#include <winternl.h>
#include <ntstatus.h>
#pragma comment(lib,"Vfw32.lib")


// #pragma pack(1)  �������Լ����Լ���Ƶģ�û��Ҫ���İ�
typedef struct _LOGIN_INFO
{
	BYTE			 IsToken;
	OSVERSIONINFOEX  OsVersionInfoEx;
	TCHAR			 ProcessorNameStringData[MAX_PATH];
	IN_ADDR			 ClientAddrData;
	CHAR			 HostNameData[MAX_PATH];
	BOOL			 IsWebCameraExist;
	DWORD			 WebSpeed;
}LOGIN_INFO, *PLOGIN_INFO;


INT      SendLoginInfo(CIOCPClient* IOCPClient, DWORD WebSpeed);

NTSTATUS ProcessorNameString(LPTSTR ProcessorNameString, ULONG* ProcessorNameStringLength);

BOOL     IsWebCamera();

