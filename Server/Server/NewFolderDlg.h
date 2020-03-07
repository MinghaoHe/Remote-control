#pragma once


// CNewFolderDlg dialog

class CFileNewFolderDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CFileNewFolderDlg)

public:
	CFileNewFolderDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CFileNewFolderDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_FILE_NEW_FOLDER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	CString _M_CEdit_String;
};
