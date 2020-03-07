// NewFolderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Server.h"
#include "NewFolderDlg.h"
#include "afxdialogex.h"


// CNewFolderDlg dialog

IMPLEMENT_DYNAMIC(CFileNewFolderDlg, CDialogEx)

CFileNewFolderDlg::CFileNewFolderDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_FILE_NEW_FOLDER, pParent)
	, _M_CEdit_String(_T(""))
{

}

CFileNewFolderDlg::~CFileNewFolderDlg()
{
}

void CFileNewFolderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT_FILE_NEW_STRING, _M_CEdit_String);
}


BEGIN_MESSAGE_MAP(CFileNewFolderDlg, CDialogEx)
END_MESSAGE_MAP()


// CNewFolderDlg message handlers
