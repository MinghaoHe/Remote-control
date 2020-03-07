#pragma once
#include "IOCPServer.h"
#include "CTrueColorToolBar.h"


// CFileManagerDlg dialog

class CFileManagerDlg : public CDialogEx
{

	DECLARE_DYNAMIC(CFileManagerDlg)

public:
	CFileManagerDlg(CWnd* pParent = nullptr, CIOCPServer* IOCPServer = NULL, PCONTEXT_OBJECT = NULL);   // standard constructor
	virtual ~CFileManagerDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_FILE_MANAGER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private :
	typedef CList<CString, CString&> JobTemplate;

	typedef struct _FILE_SIZE
	{
		DWORD FileSizeHigh;
		DWORD FileSizeLow;
	}FILE_SIZE;

	enum
	{
		TRANSFER_MODE_NORMAL,
		TRANSFER_MODE_ADDITION,
		TRANSFER_MODE_ADDITION_ALL,
		TRANSFER_MODE_OVERWRITE,
		TRANSFER_MODE_OVERWRITE_ALL,
		TRANSFER_MODE_JUMP,
		TRANSFER_MODE_JUMP_ALL,
		TRANSFER_MODE_CANCEL
	};

public:
	void OnRecvComplete(void);
	VOID FixedServerVolumeList();
	VOID FixedClientVolumeList();

	VOID FixedServerFileList(CString Directory = _T(""));
	VOID FixedClientFileList(BYTE *BufferData, ULONG BufferLength);

	VOID GetClientFileList(CString Directory   = _T(""));
	INT	 GetIconIndex(LPCTSTR VolumeName, DWORD FileAttributes);
	CString GetParentDirectory(CString FileFullPath);
	BOOL DeleteDirectory(TCHAR* DirFullPath);
	BOOL MakeSureDirPathExists(TCHAR* DirFullPath);
	VOID DropItemOnList();
	VOID OnCopyServerToClient();
	// VOID OnCopyClientToServer();
	BOOL FixedServerToClientDir(TCHAR* DirFullPath);

	BOOL SendToClientJob();				// 将传送的任务发送给客户端
	VOID SendFileDataServerToClient();

	VOID SendTransferMode();
	VOID EndCopyServerFileToClient();
	void ShowProgress();

	void ViewFileSmall();
	void ViewFileList();
	void ViewFileDetail();
	

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();

	afx_msg void OnNMDblclkListFileServer(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnFileServerBack();
	afx_msg void OnFileServerDelete();
	afx_msg void OnFileServerNew();
	afx_msg void OnFileServerStop();
	afx_msg void OnFileServerView();

	afx_msg void OnNMDblclkListFileClient(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnFileClientBack();
	afx_msg void OnFileClientDelete();
	afx_msg void OnFileClientNew();
	afx_msg void OnFileClientStop();
	afx_msg void OnFileClientView();
	
	afx_msg void OnLvnBegindragListFileServer(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

protected:
	CIOCPServer*    _M_IOCPServer;
	PCONTEXT_OBJECT _M_ContextObject;
	HICON			_M_IconHwnd;

	BYTE		_M_ClientInfo[0x1000];
	BYTE		_M_ServerInfo[0x1000];

	CImageList* _M_CImageList_Large;
	CImageList* _M_CImageList_Small;

	BOOL _M_IsDrag;
	BOOL _M_IsStop;

	CTrueColorToolBar _M_ToolBar_File_Server;
	CTrueColorToolBar _M_ToolBar_File_Client;

	CStatusBar      _M_StatusBar;
	CProgressCtrl*  _M_ProgressCtrl;

	CString _M_ServerFileFullPath;
	CString _M_ClientFileFullPath;

	CComboBox _M_CComboBox_File_Server;
	CComboBox _M_CComboBox_File_Client;

	CString _M_SrcFileFullPath;
	CString _M_DstFileFullPath;
	__int64 _M_TransferFileSize;

	DWORD   _M_TransferMode;
	__int64 _M_Counter;

	CListCtrl _M_CListCtrl_File_Server;
	CListCtrl _M_CListCtrl_File_Client;
	CStatic   _M_CStatic_File_ServerPos;
	CStatic   _M_CStatic_File_ClientPos;
	HCURSOR   _M_hCursorWnd;
	CListCtrl *_M_DragControlList;
	CListCtrl *_M_DropControlList;

	JobTemplate _M_Client_Upload_Job;
	JobTemplate _M_Server_Upload_Job;
	
	INT  ViewIndex;
};
