#include "Client.h"


_SERVER_CONNECT_INFO __ServerConnectInfo = { 0x99999999,_T("127.0.0.1"), 2356 };


int _tmain(int argc, LPCTSTR argv[], LPCTSTR envp[])
{
	_tprintf_s(_T("%s\r\n"), __ServerConnectInfo.ServerIP);

	HMODULE hModule = LoadLibrary(_T("Dll.dll"));
	
	if (hModule == NULL)
		return 0;
	

	// get function in Dll module
	lpfn_ClientRun ClientRun =
		(lpfn_ClientRun)GetProcAddress(hModule, "ClientRun");

	if (ClientRun == NULL) {
		FreeLibrary(hModule);
		return 0;
	}
	else {
		ClientRun(__ServerConnectInfo.ServerIP, __ServerConnectInfo.ServerPort);
	}

	_tprintf_s(_T("\r\nInput any key to exit\r\n"));
	_gettchar();
	FreeLibrary(hModule);

	return 0;
}
