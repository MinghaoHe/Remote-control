// FileTransferModeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Server.h"
#include "FileTransferModeDlg.h"
#include "afxdialogex.h"


// CFileTransferModeDlg dialog

IMPLEMENT_DYNAMIC(CFileTransferModeDlg, CDialogEx)

CFileTransferModeDlg::CFileTransferModeDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_FILE_TRANSFER_MODE, pParent)
	, _M_FileFullPath(_T(""))
{

}

CFileTransferModeDlg::~CFileTransferModeDlg()
{
}

void CFileTransferModeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_FILE_TRANSFER_MODE, _M_FileFullPath);
}


BEGIN_MESSAGE_MAP(CFileTransferModeDlg, CDialogEx)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_BUTTON_FILE_TRANSFER_MODE_OVERWRITE, 
								 IDC_BUTTON_FILE_TRANSFER_MODE_JUMP_ALL , OnEndDialog)


END_MESSAGE_MAP()


// CFileTransferModeDlg message handlers
void CFileTransferModeDlg::OnEndDialog(UINT id)
{
	EndDialog(id);
}

BOOL CFileTransferModeDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CString Tips;
	Tips.Format(_T("Fail...\"%s\""), _M_FileFullPath);
	for (int i = 0; i < Tips.GetLength(); i += 120)
	{
		Tips.Insert(i, _T("\n"));
		i++;
	}
	SetDlgItemText(IDC_STATIC_FILE_TRANSFER_MODE, Tips);


	return TRUE;
}


