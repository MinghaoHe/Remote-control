#pragma once
#include "IOCPServer.h"

// CInstantMessageDlg dialog

class CInstantMessageDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CInstantMessageDlg)

public:
	CInstantMessageDlg(CWnd* pParent = nullptr, CIOCPServer* ServerOB = NULL,PCONTEXT_OBJECT = NULL);   // standard constructor
	virtual ~CInstantMessageDlg();




// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_INSTANCE_MESSAGE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	void OnClientSending();

public:
	afx_msg void OnClose();
    virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	
protected:
	CIOCPServer*    _M_IOCPServer;
	PCONTEXT_OBJECT _M_ContextObject;
	HICON			_M_IconHwnd;
public:	
	CEdit _M_CEdit_Instant_Message;

};
