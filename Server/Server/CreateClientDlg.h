#pragma once



struct _SERVER_CONNECT_INFO
{
	DWORD    CheckFlag;
	CHAR     ServerIP[20];
	USHORT   ServerPort;
};

// CCreateClientDlg dialog

class CCreateClientDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CCreateClientDlg)

public:
	CCreateClientDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CCreateClientDlg();



// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_CREATE_CLIENT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:



public:
	int MemoryFind(const char*BufferData, const char *KeyValue, int BufferLength, int KeyLength);

public:
	afx_msg void OnBnClickedOk();

protected:
	CString _M_CEdit_IP;
	CString _M_CEdit_Port;
	
};
