// CreateClientDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Server.h"
#include "CreateClientDlg.h"
#include "afxdialogex.h"



_SERVER_CONNECT_INFO __ServerConnectInfo = { 0x99999999, "192.168.1.1",0 };


// CCreateClientDlg dialog

IMPLEMENT_DYNAMIC(CCreateClientDlg, CDialogEx)

CCreateClientDlg::CCreateClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_CREATE_CLIENT, pParent)
	, _M_CEdit_IP  (_T("Input Link IP"  ))
	, _M_CEdit_Port(_T("Input Link Port"))
{

}

CCreateClientDlg::~CCreateClientDlg()
{
}

void CCreateClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_CREATE_IP, _M_CEdit_IP);
	DDX_Text(pDX, IDC_EDIT_CREATE_PORT, _M_CEdit_Port);
}


int CCreateClientDlg::MemoryFind(const char*BufferData, const char *KeyValue, int BufferLength, int KeyLength)
{
	int i, j;
	if (KeyLength == 0 || BufferLength == 0)
	{
		return -1;
	}

	for (i = 0; i < BufferLength; i++)
	{
		for (j = 0; j < KeyLength; j++)
		{
			if (BufferData[i + j] != KeyValue[j])
			{
				break;
			}
		}
		if (j == KeyLength)
		{
			return i;
		}
	}
	return -1;
}

BEGIN_MESSAGE_MAP(CCreateClientDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CCreateClientDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CCreateClientDlg message handlers


void CCreateClientDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CFile FileOB;

	UpdateData(TRUE);

	USHORT ServerPort = _tstoi(_M_CEdit_Port);
	strcpy(__ServerConnectInfo.ServerIP, _M_CEdit_IP);

	if (ServerPort < 0 || ServerPort>65536)
	{
		__ServerConnectInfo.ServerPort = 2356;
	}
	else
	{
		__ServerConnectInfo.ServerPort = ServerPort;
	}
	

	TCHAR v1[MAX_PATH];
	ZeroMemory(v1, MAX_PATH);

	LONGLONG BufferLength = 0;
	BYTE  *BufferData = NULL;
	CString v3;
	CString ClientFullPath;

	try
	{
		GetModuleFileName(NULL, v1, MAX_PATH);
		v3 = v1;
		int Pos = v3.ReverseFind(_T('\\'));
		v3 = v3.Left(Pos);

		ClientFullPath = v3 + _T("\\Client.exe");
		FileOB.Open(ClientFullPath, CFile::modeRead | CFile::typeBinary);
		
		BufferLength = FileOB.GetLength();
		BufferData = new BYTE[BufferLength];
		ZeroMemory(BufferData, BufferLength);

		FileOB.Read(BufferData, BufferLength);
		
		FileOB.Close();

		int Offset = MemoryFind((char*)BufferData, (char*)&__ServerConnectInfo.CheckFlag,
			BufferLength, sizeof(DWORD));

		memcpy(BufferData + Offset, &__ServerConnectInfo, sizeof(__ServerConnectInfo));
		
		FileOB.Open(ClientFullPath, CFile::typeBinary | CFile::modeCreate | CFile::modeWrite);

		FileOB.Write(BufferData, BufferLength);
		FileOB.Close();

		delete[] BufferData;
		MessageBox(_T("Create Done"));
	}
	catch (CMemoryException *e)
	{
		MessageBox(_T("Bad Alloc"));
	}
	catch (CFileException *e)
	{
		int i = GetLastError();
		MessageBox(_T("File Error"));
	}
	catch (CException *e)
	{
		MessageBox(_T("Unknown Error"));
	}


	
	CDialogEx::OnOK();
}
