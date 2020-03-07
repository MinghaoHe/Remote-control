#pragma once


// CFileTransferModeDlg dialog

class CFileTransferModeDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CFileTransferModeDlg)

public:
	CFileTransferModeDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CFileTransferModeDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_FILE_TRANSFER_MODE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnEndDialog(UINT id);

public:
	CString _M_FileFullPath;
};
