#pragma once
#include "IOCPServer.h"
#include "Audio.h"
#include <tchar.h>

// CAudioManager dialog

class CAudioManagerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAudioManagerDlg)

public:
	CAudioManagerDlg(CWnd* pParent = nullptr, CIOCPServer* IOCPServer = NULL, PCONTEXT_OBJECT = NULL);
	virtual ~CAudioManagerDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_AUDIO_MANAGER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	void OnRecvComplete(void);

public:
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();

protected:
	CIOCPServer*    _M_IOCPServer;
	PCONTEXT_OBJECT _M_ContextObject;
	HICON			_M_IconHwnd;
	CAudio          _M_Audio;
	HANDLE			_M_hAudioProc;

};
