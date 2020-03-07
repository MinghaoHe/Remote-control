#pragma once

#include "IOCPServer.h"
#include <tchar.h>


// CCMDManagerDlg dialog

class CCMDManagerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CCMDManagerDlg)

public:
	CCMDManagerDlg(CWnd* pParent = nullptr, CIOCPServer *IOCPServer = nullptr,CONTEXT_OBJECT* ContextObject = nullptr);
	virtual ~CCMDManagerDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_CMD_MANAGER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:


public:
	void OnRecvComplete(void);
	void ShowCmdData();

public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void   OnClose();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

protected:
	HICON           _M_IconHwnd;
	CIOCPServer    *_M_IOCPServer;
	CONTEXT_OBJECT *_M_ContextObject;
	CEdit           _M_CEdit_Cmd;
	ULONG _M_CurrentPos;
	ULONG _M_ShowDataLength;
};
