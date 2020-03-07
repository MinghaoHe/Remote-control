// InstantMessageDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Server.h"
#include "InstantMessageDlg.h"
#include "afxdialogex.h"
#include "Common.h"

// CInstantMessageDlg dialog

IMPLEMENT_DYNAMIC(CInstantMessageDlg, CDialogEx)

CInstantMessageDlg::CInstantMessageDlg(CWnd* pParent, CIOCPServer* ServerOB, PCONTEXT_OBJECT ContextOB)
	: CDialogEx(IDD_DIALOG_INSTANCE_MESSAGE, pParent)
	
{
	_M_IOCPServer = ServerOB;
	_M_ContextObject = ContextOB;
	_M_IconHwnd = LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDR_MAINFRAME));
	
	return;
}

CInstantMessageDlg::~CInstantMessageDlg()
{
}

void CInstantMessageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_EDIT_INSTANT_MESSAGE, _M_CEdit_Instant_Message);
}


void CInstantMessageDlg::OnClientSending()
{
	int BufferLength = _M_CEdit_Instant_Message.GetWindowTextLength();
	if (!BufferLength)
	{
		return;
	}
	CString v1;


	_M_CEdit_Instant_Message.GetWindowText(v1);

	TCHAR *BufferData = (TCHAR*)VirtualAlloc(
		NULL,
		sizeof(TCHAR)*BufferLength,
		MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE
	);
	memset(BufferData, 0, sizeof(char)*BufferLength);

	_stprintf((TCHAR*)BufferData, _T("%s"), v1.GetBuffer(0));


	// Çå¿Õ
	_M_CEdit_Instant_Message.SetWindowText(NULL);

	// ·¢ËÍ
	_M_IOCPServer->OnClientPreSend(_M_ContextObject, (LPBYTE)BufferData, _tcslen(BufferData) * sizeof(TCHAR));

	if (BufferData != NULL)
	{
		VirtualFree(BufferData, BufferLength, MEM_RELEASE);
		BufferData = NULL;
	}

	return;
}

BEGIN_MESSAGE_MAP(CInstantMessageDlg, CDialogEx)

	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CInstantMessageDlg message handlers

void CInstantMessageDlg::OnClose()
{
	
	CancelIo((HANDLE)_M_ContextObject->ClientSocket);
	closesocket(_M_ContextObject->ClientSocket);


	CDialogEx::OnClose();
}

BOOL CInstantMessageDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(_M_IconHwnd, FALSE);
	
	BYTE IsToken = CLIENT_GO_ON;

	_M_IOCPServer->OnClientPreSend(_M_ContextObject, &IsToken, sizeof(BYTE));

	return TRUE;  
}

BOOL CInstantMessageDlg::PreTranslateMessage(MSG* pMsg)
{

	if (pMsg->message  == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE)
		{
			PostMessage(WM_CLOSE);
			return TRUE;
		}
		if (pMsg->wParam == VK_RETURN && pMsg->hwnd == _M_CEdit_Instant_Message.m_hWnd)
		{
			OnClientSending();
			return TRUE;
		}
	}
	
	return CDialogEx::PreTranslateMessage(pMsg);
}

