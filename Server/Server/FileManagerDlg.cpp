// FileManagerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Server.h"

#include "afxdialogex.h"
#include "Common.h"

#include "FileManagerDlg.h"
#include "NewFolderDlg.h"
#include "FileTransferModeDlg.h"





static UINT __Indicators[] = 
{
	ID_SEPARATOR,
	ID_SEPARATOR,
	IDR_STATUSBAR_PROGRESS
};


// CFileManagerDlg dialog

IMPLEMENT_DYNAMIC(CFileManagerDlg, CDialogEx)

CFileManagerDlg::CFileManagerDlg(CWnd* pParent, CIOCPServer* IOCPServer, PCONTEXT_OBJECT ContextObject)
	: CDialogEx(IDD_DIALOG_REMOTE_CONTROL, pParent)
{
	_M_IOCPServer    = IOCPServer;
	_M_ContextObject = ContextObject;
	_M_IconHwnd = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));

	ZeroMemory(_M_ClientInfo, sizeof(_M_ClientInfo));
	ZeroMemory(_M_ServerInfo, sizeof(_M_ServerInfo));

	memcpy(
			_M_ClientInfo,
			ContextObject->_M_InDeCompressedBufferData.GetArray(1),
			ContextObject->_M_InDeCompressedBufferData.GetArrayLength() - 1
		);

	SHFILEINFO sfi;

	HIMAGELIST hImageListWnd;

	// ��ȡϵͳͼ���б�(Tips: ���ܻ�ȡ.htm/.mht/.xml�ļ���ͼ��)
	hImageListWnd = (HIMAGELIST)SHGetFileInfo(
									NULL,
									0,
									&sfi,
									sizeof(SHFILEINFO),
									SHGFI_LARGEICON|SHGFI_SYSICONINDEX
								);
	_M_CImageList_Large = CImageList::FromHandle(hImageListWnd);
	
	hImageListWnd = (HIMAGELIST)SHGetFileInfo(
									NULL,
									0,
									&sfi,
									sizeof(SHFILEINFO),
									SHGFI_SMALLICON | SHGFI_SYSICONINDEX
								);
	_M_CImageList_Small = CImageList::FromHandle(hImageListWnd);

	_M_IsDrag = FALSE;
	_M_IsStop = FALSE;


}

CFileManagerDlg::~CFileManagerDlg()
{
}

void CFileManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_FILE_SERVER, _M_CListCtrl_File_Server);
	DDX_Control(pDX, IDC_LIST_FILE_CLIENT, _M_CListCtrl_File_Client);
	DDX_Control(pDX, IDC_STATIC_FILE_SERVER_POSITION, _M_CStatic_File_ServerPos);
	DDX_Control(pDX, IDC_STATIC_FILE_CLIENT_POSITION, _M_CStatic_File_ClientPos);
	DDX_Control(pDX, IDC_COMBO_FILE_SERVER, _M_CComboBox_File_Server);
	DDX_Control(pDX, IDC_COMBO_FILE_CLIENT, _M_CComboBox_File_Client);
}


void CFileManagerDlg::OnRecvComplete(void)
{
	if (_M_ContextObject == NULL)
	{
		return;
	}
	switch (_M_ContextObject->_M_InDeCompressedBufferData.GetArray()[0])
	{
	case CLIENT_FILE_MANAGER_FILE_LIST_REPLY:
	{
		FixedClientFileList(
			_M_ContextObject->_M_InDeCompressedBufferData.GetArray(),
			_M_ContextObject->_M_InDeCompressedBufferData.GetArrayLength() - 1
		);
		break;
	}
	case CLINET_FILE_MANAGER_TRANSFER_MODE_REQUIRE:
	{
		SendTransferMode();
		break;
	}
	case CLIENT_FILE_MANAGER_FILE_DATA_CONTINUE:
	{
		SendFileDataServerToClient();
		break;
	}
	case CLIENT_FILE_MANAGER_FILE_DATA_REPLY:
	{

		break;
	}
	default:
	{
		break;
	}
	}
	return;
}

VOID CFileManagerDlg::FixedServerVolumeList()
{
	TCHAR VolumeList[0x500];
	ZeroMemory(VolumeList, sizeof(VolumeList));
	TCHAR *Travel = NULL;
	_M_CListCtrl_File_Server.DeleteAllItems();
	while (_M_CListCtrl_File_Server.DeleteColumn(0) != 0);

	_M_CListCtrl_File_Server.InsertColumn(0, _T("Name"), LVCFMT_LEFT, 50);
	_M_CListCtrl_File_Server.InsertColumn(1, _T("Type"), LVCFMT_LEFT, 65);
	_M_CListCtrl_File_Server.InsertColumn(2, _T("System"), LVCFMT_LEFT, 45);
	_M_CListCtrl_File_Server.InsertColumn(3, _T("Capacity"), LVCFMT_LEFT, 90);
	_M_CListCtrl_File_Server.InsertColumn(4, _T("Free space"), LVCFMT_LEFT, 80);

	_M_CListCtrl_File_Server.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	GetLogicalDriveStrings(sizeof(VolumeList), (LPTSTR)VolumeList);
	Travel = VolumeList;

	CHAR	FileSystemType[MAX_PATH];
	unsigned __int64	HardDiskAmount = 0;
	unsigned __int64	HardDiskFreeSpace = 0;
	unsigned long		HardDiskAmountMB = 0;
	unsigned long		HardDiskFreeSpaceMB = 0;

	for (int i = 0; *Travel != _T('\0'); i++, Travel += _tcslen(Travel) + 1)
	{
		ZeroMemory(FileSystemType, sizeof(FileSystemType));

		GetVolumeInformation(
			Travel,
			NULL, 0, NULL, NULL, NULL,
			FileSystemType,
			MAX_PATH
		);
		ULONG	FileSystemLength = _tcslen(FileSystemType) + 1;


		if (GetDiskFreeSpaceEx(Travel, (PULARGE_INTEGER)&HardDiskFreeSpace, (PULARGE_INTEGER)&HardDiskAmount, NULL))
		{
			HardDiskAmountMB = HardDiskAmount / 1024 / 1024;
			HardDiskFreeSpaceMB = HardDiskFreeSpace / 1024 / 1024;
		}
		else
		{
			HardDiskAmountMB = 0;
			HardDiskFreeSpaceMB = 0;
		}


		int	Item = _M_CListCtrl_File_Server.InsertItem(
			i,
			Travel,
			GetIconIndex(Travel, GetFileAttributes(Travel))
		);    //���ϵͳ��ͼ��		

		_M_CListCtrl_File_Server.SetItemData(Item, 1);	//���������1����Ŀ¼��0�����ļ�
		_M_CListCtrl_File_Server.SetItemText(Item, 2, FileSystemType);


		SHFILEINFO	sfi;
		ZeroMemory(&sfi, sizeof(sfi));
		SHGetFileInfo(Travel, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(SHFILEINFO), SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);
		_M_CListCtrl_File_Server.SetItemText(Item, 1, sfi.szTypeName);


		CString	v2;
		v2.Format(_T("%10.1f GB"), (float)HardDiskAmountMB / 1024);
		_M_CListCtrl_File_Server.SetItemText(Item, 3, v2);
		v2.Format(_T("%10.1f GB"), (float)HardDiskFreeSpaceMB / 1024);
		_M_CListCtrl_File_Server.SetItemText(Item, 4, v2);
	}

	return;
}

VOID CFileManagerDlg::FixedClientVolumeList()
{
	_M_CListCtrl_File_Client.DeleteAllItems();
	while (_M_CListCtrl_File_Client.DeleteColumn(0) != 0);

	_M_CListCtrl_File_Client.InsertColumn(0, _T("Name"      ), LVCFMT_LEFT, 50);
	_M_CListCtrl_File_Client.InsertColumn(1, _T("Type"      ), LVCFMT_LEFT, 65);
	_M_CListCtrl_File_Client.InsertColumn(2, _T("System"    ), LVCFMT_LEFT, 45);
	_M_CListCtrl_File_Client.InsertColumn(3, _T("Capacity"  ), LVCFMT_LEFT, 90);
	_M_CListCtrl_File_Client.InsertColumn(4, _T("Free Space"), LVCFMT_LEFT, 80);
	_M_CListCtrl_File_Client.SetExtendedStyle(LVS_EX_FULLROWSELECT);


	TCHAR	*Travel = NULL;
	Travel = (TCHAR*)_M_ClientInfo;   //�Ѿ�ȥ������Ϣͷ��1���ֽ�


	int i = 0;
	ULONG v1 = 0;
	for (i = 0; Travel[i] != _T('\0');)
	{
		if (Travel[i] == _T('A') || Travel[i] == _T('B'))
		{
			v1 = 6;
		}
		else
		{

			switch (Travel[i + 1])
			{
			case DRIVE_REMOVABLE:
				v1 = 2 + 5;
				break;
			case DRIVE_FIXED:
				v1 = 3 + 5;
				break;
			case DRIVE_REMOTE:
				v1 = 4 + 5;
				break;
			case DRIVE_CDROM:
				v1 = 9;
				break;
			default:
				v1 = 0;
				break;
			}
		}

		CString	v3;
		v3.Format(_T("%c:\\"), Travel[i]);	//�̷�
		int	Item = _M_CListCtrl_File_Client.InsertItem(i, v3, v1);
		_M_CListCtrl_File_Client.SetItemData(Item, 1);     //���������1����Ŀ¼��0�����ļ�

		//[�̷�(1)][����������������(1)][HardDiskAmountMB(4)][HardDiskFreeSpaceMB(4)][����������][�ļ�ϵͳ��Ϣ]

		unsigned long		HardDiskAmountMB = 0; // �ܴ�С
		unsigned long		HardDiskFreeMB = 0;   // ʣ��ռ�
		memcpy(&HardDiskAmountMB, Travel + i + 2, 4);
		memcpy(&HardDiskFreeMB, Travel + i + 6, 4);

		CString  v5;
		v5.Format(_T("%10.1f GB"), (float)HardDiskAmountMB / 1024);
		_M_CListCtrl_File_Client.SetItemText(Item, 3, v5);
		v5.Format(_T("%10.1f GB"), (float)HardDiskFreeMB / 1024);
		_M_CListCtrl_File_Client.SetItemText(Item, 4, v5);
		i += 10;   //1 + 1 + 4 + 4

				   //��������
		CString  HardDiskType;
		HardDiskType = Travel + i;
		_M_CListCtrl_File_Client.SetItemText(Item, 1, HardDiskType);
		i += _tcslen(Travel + i) + 1;
		//�ļ�ϵͳ����
		CString  FileSystemType;
		FileSystemType = Travel + i;
		_M_CListCtrl_File_Client.SetItemText(Item, 2, FileSystemType);
		i += _tcslen(Travel + i) + 1;

	}
}

INT	 CFileManagerDlg::GetIconIndex(LPCTSTR VolumeName, DWORD FileAttributes)
{
	SHFILEINFO	sfi;
	if (FileAttributes == INVALID_FILE_ATTRIBUTES)
		FileAttributes = FILE_ATTRIBUTE_NORMAL;
	else
		FileAttributes |= FILE_ATTRIBUTE_NORMAL;

	SHGetFileInfo(
		VolumeName,
		FileAttributes,
		&sfi,
		sizeof(SHFILEINFO),
		SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES
	);

	return sfi.iIcon;
}

VOID CFileManagerDlg::FixedServerFileList(CString Directory)
{
	if (Directory.GetLength() == 0)
	{
		int	iItem = _M_CListCtrl_File_Server.GetSelectionMark();

		if (iItem != -1)
		{
			if (_M_CListCtrl_File_Server.GetItemData(iItem) == 1)
			{
				Directory = _M_CListCtrl_File_Server.GetItemText(iItem, 0);
			}
		}
		else
		{

			//	_M_CListCtrl_File_Server.GetWindowText(_M_ServerFileFullPath);
		}
	}

	if (Directory == _T(".."))
	{
		_M_ServerFileFullPath = GetParentDirectory(_M_ServerFileFullPath);

	}
	// ˢ�µ�ǰ��
	else if (Directory != _T("."))   //��ϵͳ��ÿ��Ŀ¼�ж�����һ��.��..Ŀ¼
	{
		_M_ServerFileFullPath += Directory;
		if (_M_ServerFileFullPath.Right(1) != _T("\\"))
		{
			_M_ServerFileFullPath += _T("\\");
		}
	}

	if (_M_ServerFileFullPath.GetLength() == 0)
	{
		FixedServerVolumeList();
		return;
	}

	//������·�����뵽ComboBox����
	_M_CComboBox_File_Server.InsertString(0, _M_ServerFileFullPath);
	_M_CComboBox_File_Server.SetCurSel(0);

	//ɾ��ControlList�ϵ�����
	_M_CListCtrl_File_Server.DeleteAllItems();
	while (_M_CListCtrl_File_Server.DeleteColumn(0) != 0);

	//���²�������
	_M_CListCtrl_File_Server.InsertColumn(0, _T("Name"), LVCFMT_LEFT, 150);
	_M_CListCtrl_File_Server.InsertColumn(1, _T("Size"), LVCFMT_LEFT, 100);
	_M_CListCtrl_File_Server.InsertColumn(2, _T("Type"), LVCFMT_LEFT, 100);
	_M_CListCtrl_File_Server.InsertColumn(3, _T("Data modified"), LVCFMT_LEFT, 115);



	int			v10 = 0;
	//�Լ��ڿؼ�������һ��..Ŀ¼��˫��������һ�㣬˳��������������
	_M_CListCtrl_File_Server.SetItemData(_M_CListCtrl_File_Server.InsertItem(v10++, _T(".."),
		GetIconIndex(NULL, FILE_ATTRIBUTE_DIRECTORY)), 1);	//����ļ���ͼ��

															//ѭ�����ηֱ����Ŀ¼���ļ�(��ʱ0����Ŀ¼��1�����ļ�)
	for (int i = 0; i < 2; i++)
	{
		CFileFind	FileFind;
		BOOL		IsLoop;
		IsLoop = FileFind.FindFile(_M_ServerFileFullPath + _T("*.*"));   //C:\*.*      //*.*����һ��
		while (IsLoop)
		{
			IsLoop = FileFind.FindNextFile();
			if (FileFind.IsDots())
			{
				continue;// .. Ŀ¼
			}

			BOOL IsInsert = !FileFind.IsDirectory() == i;
			if (!IsInsert)
			{
				//����Ŀ¼
				continue;
			}


			int Item = _M_CListCtrl_File_Server.InsertItem(
				v10++,
				FileFind.GetFileName(),
				GetIconIndex(FileFind.GetFileName(),
					GetFileAttributes(FileFind.GetFilePath())
				)
			);
			//�����������ݣ�Ŀ¼Ϊ1���ļ�Ϊ0
			_M_CListCtrl_File_Server.SetItemData(Item, FileFind.IsDirectory());

			//��������
			SHFILEINFO	sfi;
			SHGetFileInfo(FileFind.GetFileName(), FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(SHFILEINFO),
				SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);
			if (FileFind.IsDirectory())
			{
				_M_CListCtrl_File_Server.SetItemText(Item, 2, _T("folder"));
			}
			else
			{
				_M_CListCtrl_File_Server.SetItemText(Item, 2, sfi.szTypeName);
			}
			//�����ļ���С
			CString v2;
			v2.Format(_T("%10d KB"), FileFind.GetLength() / 1024 + (FileFind.GetLength() % 1024 ? 1 : 0));
			_M_CListCtrl_File_Server.SetItemText(Item, 1, v2);
			//�������һ�η���ʱ��
			CTime Time;
			FileFind.GetLastWriteTime(Time);
			_M_CListCtrl_File_Server.SetItemText(Item, 3, Time.Format(_T("%Y-%m-%d %H:%M")));
		}
	}
	return;
}

VOID CFileManagerDlg::FixedClientFileList(BYTE *BufferData, ULONG BufferLength)
{
	// �ؽ�����
	_M_CListCtrl_File_Client.DeleteAllItems();
	//����ԭ������
	while (_M_CListCtrl_File_Client.DeleteColumn(0) != 0);

	_M_CListCtrl_File_Client.InsertColumn(0, _T("Name"), LVCFMT_LEFT, 150);
	_M_CListCtrl_File_Client.InsertColumn(1, _T("Size"), LVCFMT_LEFT, 100);
	_M_CListCtrl_File_Client.InsertColumn(2, _T("Type"), LVCFMT_LEFT, 100);
	_M_CListCtrl_File_Client.InsertColumn(3, _T("Data modified"), LVCFMT_LEFT, 115);
	//����..Ŀ¼
	int	v10 = 0;
	_M_CListCtrl_File_Client.SetItemData(
		_M_CListCtrl_File_Client.InsertItem(
			v10++,
			_T(".."),
			GetIconIndex(NULL, FILE_ATTRIBUTE_DIRECTORY)),
		1
	);

	if (BufferLength != 0)
	{
		// ������������������ʾ���б���
		for (int i = 0; i < 2; i++)
		{
			//Խ����־
			char *Travel = (char *)(BufferData + 1);

			for (char *v1 = Travel; Travel - v1 < BufferLength - 1;)
			{

				DWORD	 FileSizeHigh = 0; // �ļ����ֽڴ�С
				DWORD	 FileSizeLow = 0;  // �ļ����ֽڴ�С
				TCHAR*	 FileName = NULL;
				int		 Item = 0;
				BOOL	 IsInsert = FALSE;
				FILETIME FileTime;

				ZeroMemory(&FileTime, sizeof(FileTime));

				//[�ļ�����(1�ֽ�)][�ļ���\0][�ļ���С(8�ֽ�)][������ʱ��(8�ֽ�)]
				int	v2 = *Travel ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;

				// iΪ0ʱ,����Ŀ¼,iΪ1�����ļ�
				IsInsert = !(v2 == FILE_ATTRIBUTE_DIRECTORY) == i;

				FileName = ++Travel;
				//[�ļ���\0][�ļ���С(8�ֽ�)][������ʱ��(8�ֽ�)]

				if (IsInsert)
				{
					//�����ļ�������������������
					Item = _M_CListCtrl_File_Client.InsertItem(v10++, FileName, GetIconIndex(FileName, v2));
					_M_CListCtrl_File_Client.SetItemData(Item, v2 == FILE_ATTRIBUTE_DIRECTORY);   //�������ԣ�1ΪĿ¼0Ϊ�ļ�
																													 //����������
					SHFILEINFO	sfi;
					SHGetFileInfo(FileName, FILE_ATTRIBUTE_NORMAL | v2, &sfi, sizeof(SHFILEINFO),
						SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);
					_M_CListCtrl_File_Client.SetItemText(Item, 2, sfi.szTypeName);
				}

				// �õ��ļ���С
				Travel += strlen(FileName) + 1;
				//[�ļ���С(8�ֽ�)][������ʱ��(8�ֽ�)]
				if (IsInsert)
				{
					memcpy(&FileSizeHigh, Travel, 4);
					memcpy(&FileSizeLow, Travel + 4, 4);
					CString v7;
					v7.Format(_T("%10d KB"), (FileSizeHigh * (MAXDWORD + 1)) / 1024 + FileSizeLow / 1024 + (FileSizeLow % 1024 ? 1 : 0));
					_M_CListCtrl_File_Client.SetItemText(Item, 1, v7);
					memcpy(&FileTime, Travel + 8, sizeof(FILETIME));
					//[������ʱ��(8�ֽ�)]
					CTime	Time(FileTime);
					_M_CListCtrl_File_Client.SetItemText(Item, 3, Time.Format(_T("%Y-%m-%d %H:%M")));
				}
				//��һ������
				Travel += 16;
			}
		}
	}

	// �ָ�����
	_M_CListCtrl_File_Client.EnableWindow(TRUE);
}


VOID CFileManagerDlg::GetClientFileList(CString Directory)
{
	if (Directory.GetLength() == 0)
	{
		int Item = _M_CListCtrl_File_Client.GetSelectionMark();
		if (Item != -1)
		{
			if (_M_CListCtrl_File_Client.GetItemData(Item) == 1)
			{
				Directory = _M_CListCtrl_File_Client.GetItemText(Item, 0);
			}
		}
		else
		{

		}
	}
	if (Directory == _T(".."))
	{
		_M_ClientFileFullPath = GetParentDirectory(_M_ClientFileFullPath);
	}
	else if (Directory != ".")
	{
		_M_ClientFileFullPath += Directory;
		if (_M_ClientFileFullPath.Right(1) != _T("\\"))
		{
			_M_ClientFileFullPath += _T("\\");
		}
	}
	if (_M_ClientFileFullPath.GetLength() == 0)
	{
		FixedClientVolumeList();
		return;
	}
	_M_CComboBox_File_Client.InsertString(0, _M_ClientFileFullPath);
	_M_CComboBox_File_Client.SetCurSel(0);

	ULONG	BufferLength = _M_ClientFileFullPath.GetLength() + 2;	//��־ + \0
	BYTE*   BufferData = (BYTE *)new BYTE[BufferLength];

	BufferData[0] = CLIENT_FILE_MANAGER_FILE_LIST_REQUEST;
	memcpy(BufferData + 1, _M_ClientFileFullPath.GetBuffer(0), BufferLength - 1);
	_M_IOCPServer->OnClientPreSend(_M_ContextObject, BufferData, BufferLength);

	delete[] BufferData;
	BufferData = NULL;

	//	m_CComboBox_Dialog_File_Manager_Client_File.InsertString(0, m_Remote_Path);
	//	m_CComboBox_Dialog_File_Manager_Client_File.SetCurSel(0);

	_M_CListCtrl_File_Client.EnableWindow(FALSE);
	_M_ProgressCtrl->SetPos(0);

	return;
}

CString 
	 CFileManagerDlg::GetParentDirectory(CString FileFullPath)
{
	CString v1 = FileFullPath;
	int Index = v1.ReverseFind(_T('\\'));
	if (Index == -1) {
		// �Ҳ��� '//' Ӧ�ǳ������쳣
		return v1;
	}

	CString v3 = v1.Left(Index);
	Index = v3.ReverseFind(_T('\\'));
	if (Index == -1)
	{
		v1 = _T("");
		return v1;
	}

	v1 = v3.Left(Index);

	if (v1.Right(1) != _T("\\"))
		v1 += _T("\\");

	return v1;
}

BOOL CFileManagerDlg::DeleteDirectory(TCHAR* DirFullPath)
{
	WIN32_FIND_DATA	wfd;
	TCHAR	BufferData[MAX_PATH];
	ZeroMemory(&wfd, sizeof(WIN32_FIND_DATA));
	ZeroMemory(BufferData, sizeof(BufferData));

	_stprintf(BufferData, _T("%s\\*.*"), DirFullPath);

	HANDLE hFile = FindFirstFile(BufferData, &wfd);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	do
	{
		if (strcmp(wfd.cFileName, _T(".")) == 0 || strcmp(wfd.cFileName, _T("..")) == 0)
		{
			continue;
		}
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			TCHAR v2[MAX_PATH];
			wsprintf(v2, _T("%s\\%s"), DirFullPath, wfd.cFileName);
			DeleteDirectory(v2);
		}
		else
		{
			TCHAR v2[MAX_PATH];
			_stprintf(v2, _T("%s\\%s"), DirFullPath, wfd.cFileName);
			DeleteFile(v2);
		}
	} while (FindNextFile(hFile, &wfd));

	FindClose(hFile);

	if (!RemoveDirectory(DirFullPath))
	{
		return FALSE;
	}
	return TRUE;
}

BOOL CFileManagerDlg::MakeSureDirPathExists(TCHAR* DirFullPath)
{
	TCHAR* Travel = NULL;
	TCHAR* BufferData = NULL;
	DWORD DiryAttributes;
	__try
	{
		BufferData = (TCHAR*)malloc(sizeof(TCHAR)*(_tcslen(DirFullPath) + 1));

		if (BufferData == NULL)
		{
			return FALSE;
		}

		_tcscpy(BufferData, DirFullPath);

		Travel = BufferData;


		if (*(Travel + 1) == _T(':'))
		{
			Travel++;
			Travel++;
			if (*Travel && (*Travel == _T('\\')))
			{
				Travel++;
			}
		}

		while (*Travel)
		{
			if (*Travel == _T('\\'))
			{
				*Travel = _T('\0');
				DiryAttributes = GetFileAttributes(BufferData);
				if (DiryAttributes == 0xffffffff)
				{
					if (!CreateDirectory(BufferData, NULL))
					{
						if (GetLastError() != ERROR_ALREADY_EXISTS)
						{
							free(BufferData);
							return FALSE;
						}
					}
				}
				else
				{
					if ((DiryAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
					{
						free(BufferData);
						BufferData = NULL;
						return FALSE;
					}
				}
				*Travel = _T('\\');
			}
			Travel = CharNext(Travel);
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		if (BufferData != NULL)
		{
			free(BufferData);
			BufferData = NULL;
		}

		return FALSE;
	}

	if (BufferData != NULL)
	{
		free(BufferData);
		BufferData = NULL;
	}
	return TRUE;
}

VOID CFileManagerDlg::DropItemOnList()
{
	if (_M_DragControlList == _M_DropControlList)
		return;

	// �ж����ݿ�������
	if ((CWnd*)_M_DropControlList == &_M_CListCtrl_File_Server)
	{
		// OnCopyClientToServer();
	}
	else if ((CWnd*)_M_DropControlList == &_M_CListCtrl_File_Client)
	{
		OnCopyServerToClient();
	}
	else
	{
		return;
	}

	return;
}


VOID CFileManagerDlg::OnCopyServerToClient()
{
	_M_Client_Upload_Job.RemoveAll();

	POSITION Pos = _M_CListCtrl_File_Server.GetFirstSelectedItemPosition();
	while (Pos)
	{
		int Item = _M_CListCtrl_File_Server.GetNextSelectedItem(Pos);
		CString FileFullPath = NULL;

		FileFullPath = _M_ServerFileFullPath + _M_CListCtrl_File_Server.GetItemText(Item, 0);
		if (_M_CListCtrl_File_Server.GetItemData(Item)) // ���������ж��ļ�����
		{
			// ��Ŀ¼�� ��'\\'
			FileFullPath += _T("\\");
			FixedServerToClientDir(FileFullPath.GetBuffer(0));
		}
		else
		{
			// ���ļ���CreateFile() �ж��Ƿ�Ϸ�
			HANDLE hFile = CreateFile(
								FileFullPath,
								GENERIC_READ | GENERIC_WRITE,
								FILE_SHARE_READ | FILE_SHARE_WRITE,
								NULL,
								OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL,
								NULL
							);
			if (hFile == INVALID_HANDLE_VALUE) {
				continue;
			}
			_M_Client_Upload_Job.AddTail(FileFullPath);
			CloseHandle(hFile);
		}
	}
	if (_M_Client_Upload_Job.IsEmpty())
	{
		// ��Ŀ¼�����������ݴ���
		::MessageBox(m_hWnd, _T("Empty folder"), _T("Warning"), MB_OK | MB_ICONWARNING);
		return;
	}
	_M_CListCtrl_File_Client.EnableWindow(FALSE);
	_M_CListCtrl_File_Server.EnableWindow(FALSE);

	// ���͵�һ������
	SendToClientJob();
	return;
}

BOOL CFileManagerDlg::FixedServerToClientDir(TCHAR* DirFullPath)
{
	TCHAR BufferData[MAX_PATH];
	TCHAR* Slash = NULL;
	ZeroMemory(BufferData, sizeof(BufferData));


	if (DirFullPath[_tcslen(DirFullPath) - 1] != _T('\\')) {
		Slash = _T("\\");
	}
	else {
		Slash = _T("");
	}
	_stprintf(BufferData, _T("%s%s*.*"), DirFullPath, Slash);

	WIN32_FIND_DATA wfd;
	ZeroMemory(&wfd, sizeof(wfd));
	HANDLE hFile = FindFirstFile(BufferData, &wfd);
	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	do {
		// ���˵�Ŀ¼ '.' �� '..'
		if (wfd.cFileName[0] == _T('.'))
			continue;
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			// ��Ȼ��Ŀ¼������ݹ�
			TCHAR v1[MAX_PATH];
			ZeroMemory(v1, sizeof(v1));
			_stprintf(v1, _T("%s%s%s"), DirFullPath, Slash, wfd.cFileName);
			FixedServerToClientDir(v1);
		}
		else {
			CString FileFullPath;
			FileFullPath.Format(_T("%s%s%s"), DirFullPath, Slash, wfd.cFileName);
			_M_Client_Upload_Job.AddTail(FileFullPath);
		}
	} while (FindNextFile(hFile, &wfd));

	FindClose(hFile);

	return TRUE;
}

BOOL CFileManagerDlg::SendToClientJob()
{
	if (_M_Client_Upload_Job.IsEmpty())
		return FALSE;

	CString DstDirectory = _M_ClientFileFullPath;

	// ȷ��Ŀ��·���ĸ�Ŀ¼
	_M_SrcFileFullPath = _M_Client_Upload_Job.GetHead();

	DWORD FileSizeHigh = 0;
	DWORD FileSizeLow  = 0;

	HANDLE  hFile;
	CString v1 = _M_SrcFileFullPath;

	v1.Replace(_M_ServerFileFullPath, _M_ClientFileFullPath);

	_M_DstFileFullPath = v1;

	hFile = CreateFile(
				_M_SrcFileFullPath.GetBuffer(0),
				GENERIC_READ,
				FILE_SHARE_READ,
				NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				0
			);
	if (hFile == INVALID_HANDLE_VALUE) {
		return FALSE;
	}

	FileSizeLow = GetFileSize(hFile, &FileSizeHigh);

	_M_TransferFileSize = (FileSizeHigh * (MAXDWORD + 1)) + FileSizeLow;

	CloseHandle(hFile);

	ULONG BufferLength = v1.GetLength() + 10;
	BYTE* BufferData = (BYTE*)LocalAlloc(LPTR, BufferLength);
	ZeroMemory(BufferData, BufferLength);

	BufferData[0] = CLIENT_FILE_MANAGER_FILE_INFO_REQUIRE;

	// BufferData
	// [FLAG][FileSizeHigh][FileSizeLow][FileFullPath]

	memcpy(BufferData + 1, &FileSizeHigh, sizeof(DWORD));
	memcpy(BufferData + 5, &FileSizeLow,  sizeof(DWORD));

	memcpy(BufferData + 9, v1.GetBuffer(0), v1.GetLength() + 1);
	// ��������
	_M_IOCPServer->OnClientPreSend(_M_ContextObject, BufferData, BufferLength);

	LocalFree(BufferData);

	// ɾ������ɵ�����
	_M_Client_Upload_Job.RemoveHead();

	return TRUE;
}

VOID CFileManagerDlg::SendTransferMode()
{
	CFileTransferModeDlg	Dialog(this);
	Dialog._M_FileFullPath = _M_DstFileFullPath;
	switch (Dialog.DoModal())
	{
	case IDC_BUTTON_FILE_TRANSFER_MODE_OVERWRITE:
	{
		_M_TransferMode = TRANSFER_MODE_OVERWRITE;
		break;
	}
	case IDC_BUTTON_FILE_TRANSFER_MODE_OVERWRITE_ALL:
	{
		_M_TransferMode = TRANSFER_MODE_OVERWRITE_ALL;
		break;
	}
	case IDC_BUTTON_FILE_TRANSFER_MODE_JUMP:
	{
		_M_TransferMode = TRANSFER_MODE_JUMP;
		break;
	}
	case IDC_BUTTON_FILE_TRANSFER_MODE_JUMP_ALL:
	{
		_M_TransferMode = TRANSFER_MODE_JUMP_ALL;
		break;
	}
	case IDCANCEL:
	{
		_M_TransferMode = TRANSFER_MODE_CANCEL;
		break;
	}
	}

	if (_M_TransferMode == TRANSFER_MODE_CANCEL)
	{
		_M_IsStop = TRUE;
		EndCopyServerFileToClient();
		return;
	}
	BYTE Token[5];
	Token[0] = CLIENT_FILE_MANAGER_SET_TRANSFER_MODE;
	memcpy(Token + 1, &_M_TransferMode, sizeof(_M_TransferMode));

	_M_IOCPServer->OnClientPreSend(_M_ContextObject, (unsigned char *)&Token, sizeof(Token));

	return;
}

VOID CFileManagerDlg::EndCopyServerFileToClient()
{
	_M_Counter = 0;
	_M_TransferFileSize = 0;
	ShowProgress();

	if (_M_Client_Upload_Job.IsEmpty() || _M_IsStop)
	{
		_M_Client_Upload_Job.RemoveAll();
		_M_IsStop = FALSE;

		_M_CListCtrl_File_Client.EnableWindow(TRUE);
		_M_CListCtrl_File_Server.EnableWindow(TRUE);

		_M_TransferMode = TRANSFER_MODE_NORMAL;
		GetClientFileList(_T("."));
	}
	else
	{
		Sleep(5);
		SendToClientJob();
	}
	return;
}

VOID CFileManagerDlg::SendFileDataServerToClient()
{
	auto MAKEINT64 = [](unsigned long l, unsigned long h) {
		return (((unsigned __int64)(l)) | (((unsigned __int64)(h)) << 0x20));
	};

	FILE_SIZE* v1 = (FILE_SIZE *)(_M_ContextObject->_M_InDeCompressedBufferData.GetArray(1));
	LONG	OffsetHigh = v1->FileSizeHigh;
	LONG	OffsetLow  = v1->FileSizeLow;

	//��ʾ�Ѿ�������ɵ�����
	_M_Counter = MAKEINT64(v1->FileSizeLow, v1->FileSizeHigh);

	 //֪ͨ������
	ShowProgress();

	if (_M_Counter == _M_TransferFileSize
		|| _M_IsStop
		|| v1->FileSizeLow == -1)
	{
		//�����¸�����Ĵ����������
		EndCopyServerFileToClient(); 
		return;
	}

	HANDLE	hFile;
	hFile = CreateFile(
				_M_SrcFileFullPath.GetBuffer(0), 
				GENERIC_READ, 
				FILE_SHARE_READ, 
				NULL, 
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL, 
				0
			);

	if (hFile == INVALID_HANDLE_VALUE) {
		return;
	}
	//��λ�ļ���ָ�뵽�ļ���ʼ�������ڶ�ȡ�ļ�����
	SetFilePointer(hFile, OffsetLow, &OffsetHigh, FILE_BEGIN);

	// [Token][OffsetHigh][OffsetLow]
	int		offset = 9; // 1 + 4 + 4  
	DWORD	NumberOfBytesToRead = MAX_SEND_BUFFER - offset;
	DWORD	rLen = 0;
	BYTE	*BufferData = (BYTE *)LocalAlloc(LPTR, MAX_SEND_BUFFER);

	if (BufferData == NULL)
	{
		CloseHandle(hFile);
		return;
	}



	BufferData[0] = CLIENT_FILE_MANAGER_FILE_DATA_REPLY;
	memcpy(BufferData + 1, &OffsetHigh, sizeof(OffsetHigh));
	memcpy(BufferData + 5, &OffsetLow,  sizeof(OffsetLow));

	ReadFile(hFile, BufferData + offset, NumberOfBytesToRead, &rLen, NULL);

	CloseHandle(hFile);

	if (rLen > 0)
	{
		ULONG	BufferLength = rLen + offset;
		_M_IOCPServer->OnClientPreSend(_M_ContextObject, BufferData, BufferLength);
	}
	LocalFree(BufferData);

	return;
}

void CFileManagerDlg::ShowProgress()
{
	if ((int)_M_Counter == -1)
	{
		_M_Counter = _M_TransferFileSize;
	}
	int Progress = (float)(_M_Counter * 100) / _M_TransferFileSize;
	_M_ProgressCtrl->SetPos(Progress);

	if (_M_Counter == _M_TransferFileSize)
	{
		_M_Counter = _M_TransferFileSize = 0;
	}

	return;
}


void CFileManagerDlg::ViewFileSmall()
{

}

void CFileManagerDlg::ViewFileList()
{

}

void CFileManagerDlg::ViewFileDetail()
{

}

BEGIN_MESSAGE_MAP(CFileManagerDlg, CDialogEx)
	ON_WM_CLOSE()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_FILE_SERVER, &CFileManagerDlg::OnNMDblclkListFileServer)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_FILE_CLIENT, &CFileManagerDlg::OnNMDblclkListFileClient)
	ON_COMMAND(IDT_FILE_SERVER_BACK,	  &CFileManagerDlg::OnFileServerBack  )
	ON_COMMAND(IDT_FILE_SERVER_VIEW,      &CFileManagerDlg::OnFileServerView  )
	ON_COMMAND(IDT_FILE_SERVER_DELETE,    &CFileManagerDlg::OnFileServerDelete)
	ON_COMMAND(IDT_FILE_SERVER_NEW,       &CFileManagerDlg::OnFileServerNew   )
	ON_COMMAND(IDT_FILE_SERVER_STOP,      &CFileManagerDlg::OnFileServerStop  )
	ON_COMMAND(IDT_FILE_CLIENT_BACK,      &CFileManagerDlg::OnFileClientBack  )
	ON_COMMAND(IDT_FILE_CLIENT_VIEW,      &CFileManagerDlg::OnFileClientView  )
	ON_COMMAND(IDT_FILE_CLIENT_DELETE,    &CFileManagerDlg::OnFileClientDelete)
	ON_COMMAND(IDT_FILE_CLIENT_NEW,       &CFileManagerDlg::OnFileClientNew   )
	ON_COMMAND(IDT_FILE_CLIENT_STOP,      &CFileManagerDlg::OnFileClientStop  )


	ON_NOTIFY(LVN_BEGINDRAG, IDC_LIST_FILE_SERVER, &CFileManagerDlg::OnLvnBegindragListFileServer)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()


// CFileManagerDlg message handlers


BOOL CFileManagerDlg::OnInitDialog()
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
	v1.Format(_T("\\%s - File Manager"), rVal != INVALID_SOCKET ? inet_ntoa(ClientAddr.sin_addr) : _T("Local Host"));

	SetWindowText(v1);

	
	if (!_M_ToolBar_File_Server.Create(
				this, 
				WS_CHILD | WS_VISIBLE | CBRS_ALIGN_ANY | CBRS_TOOLTIPS | CBRS_FLYBY, 
				IDR_TOOLBAR_SERVER_FILE
			)
		|| !_M_ToolBar_File_Server.LoadToolBar(IDR_TOOLBAR_SERVER_FILE)
		)
	{

		return -1;
	}

	if (!_M_ToolBar_File_Client.Create(
				this, 
				WS_CHILD | WS_VISIBLE | CBRS_ALIGN_ANY | CBRS_TOOLTIPS | CBRS_FLYBY, 
				IDR_TOOLBAR_CLIENT_FILE
			)
		|| !_M_ToolBar_File_Client.LoadToolBar(IDR_TOOLBAR_CLIENT_FILE)
		)
	{

		return -1;
	}

	_M_ToolBar_File_Server.LoadTrueColorToolBar
	(
		24,    
		IDB_BITMAP_TOOLBAR_FILE,
		IDB_BITMAP_TOOLBAR_FILE,
		IDB_BITMAP_TOOLBAR_FILE
	);

	_M_ToolBar_File_Client.LoadTrueColorToolBar
	(
		24,     
		IDB_BITMAP_TOOLBAR_FILE,
		IDB_BITMAP_TOOLBAR_FILE,
		IDB_BITMAP_TOOLBAR_FILE
	);

	// _M_ToolBar_File_Server.AddDropDownButton(this, IDT_FILE_SERVER_VIEW, IDT_FILE_SERVER_VIEW);
	_M_ToolBar_File_Server.SetButtonText(0, _T("Back")  );
	_M_ToolBar_File_Server.SetButtonText(1, _T("View")  );
	_M_ToolBar_File_Server.SetButtonText(2, _T("Delete"));
	_M_ToolBar_File_Server.SetButtonText(3, _T("New")   );
	_M_ToolBar_File_Server.SetButtonText(4, _T("Search"));
	_M_ToolBar_File_Server.SetButtonText(5, _T("Stop")  );

	//m_ToolBar_File_Manager_Client.AddDropDownButton(this, IDT_TOOLBAR_DIALOG_FILE_MANAGER_CLIENT_VIEW, IDT_TOOLBAR_DIALOG_FILE_MANAGER_CLIENT_VIEW);
	_M_ToolBar_File_Client.SetButtonText(0, _T("Back")  );
	_M_ToolBar_File_Client.SetButtonText(1, _T("View")  );
	_M_ToolBar_File_Client.SetButtonText(2, _T("Delete"));
	_M_ToolBar_File_Client.SetButtonText(3, _T("New")   );
	_M_ToolBar_File_Client.SetButtonText(4, _T("Search"));
	_M_ToolBar_File_Client.SetButtonText(5, _T("Stop")  );

	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
	_M_CListCtrl_File_Server.SetImageList(_M_CImageList_Large, LVSIL_NORMAL);
	_M_CListCtrl_File_Server.SetImageList(_M_CImageList_Small, LVSIL_SMALL );
	_M_CListCtrl_File_Client.SetImageList(_M_CImageList_Large, LVSIL_NORMAL);
	_M_CListCtrl_File_Client.SetImageList(_M_CImageList_Small, LVSIL_SMALL );


	RECT	Rect;
	GetClientRect(&Rect);

	CRect v3;
	v3.top    = Rect.bottom - 25;
	v3.left   = 0;
	v3.right  = Rect.right;
	v3.bottom = Rect.bottom;

	if (!_M_StatusBar.Create(this) ||
		!_M_StatusBar.SetIndicators(
				__Indicators,
				sizeof(__Indicators) / sizeof(UINT))
			)
	{
		return -1;
	}
	_M_StatusBar.SetPaneInfo(0, _M_StatusBar.GetItemID(0), SBPS_STRETCH, NULL);
	_M_StatusBar.SetPaneInfo(1, _M_StatusBar.GetItemID(1), SBPS_NORMAL,  120 );
	_M_StatusBar.SetPaneInfo(2, _M_StatusBar.GetItemID(2), SBPS_NORMAL,  50  );
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0); //��ʾ״̬��

	_M_StatusBar.MoveWindow(v3);
	_M_StatusBar.GetItemRect(1, &Rect);

	_M_ProgressCtrl = new CProgressCtrl;
	_M_ProgressCtrl->Create(PBS_SMOOTH | WS_VISIBLE, Rect, &_M_StatusBar, 1);
	_M_ProgressCtrl->SetRange(0, 100);
	_M_ProgressCtrl->SetPos(0);

	RECT	ServerRect;
	_M_CStatic_File_ServerPos.GetWindowRect(&ServerRect);
	_M_CStatic_File_ServerPos.ShowWindow(SW_HIDE);

	//��ʾ������
	_M_ToolBar_File_Server.MoveWindow(&ServerRect);

	RECT	ClientRect;
	_M_CStatic_File_ClientPos.GetWindowRect(&ClientRect);
	_M_CStatic_File_ClientPos.ShowWindow(SW_HIDE);
	//��ʾ������
	_M_ToolBar_File_Client.MoveWindow(&ClientRect);

	FixedServerVolumeList();
	FixedClientVolumeList();
	return TRUE;  			 
}

void CFileManagerDlg::OnClose()
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


void CFileManagerDlg::OnNMDblclkListFileServer(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	if (_M_CListCtrl_File_Server.GetSelectedCount() == 0 ||
		_M_CListCtrl_File_Server.GetItemData(_M_CListCtrl_File_Server.GetSelectionMark()) != 1)
	{
		return;
	}

	FixedServerFileList();
	*pResult = 0;
	return;
}

void CFileManagerDlg::OnFileServerBack()
{
	FixedServerFileList(_T(".."));
}

void CFileManagerDlg::OnFileServerDelete()
{
	CString v1;
	if (_M_CListCtrl_File_Server.GetSelectedCount() > 1)
	{
		v1.Format(_T("Are you sure you want to delete these %d items"), _M_CListCtrl_File_Server.GetSelectedCount());
	}
	else
	{
		int Item = _M_CListCtrl_File_Server.GetSelectionMark();   //.. fff 1  Hello
		if (Item == -1)
		{
			return;
		}
		CString FileName = _M_CListCtrl_File_Server.GetItemText(_M_CListCtrl_File_Server.GetSelectionMark(), 0);

		if (_M_CListCtrl_File_Server.GetItemData(Item) == 1)
		{
			v1.Format(_T("Are you sure you want to delete all these items in folder %s"), FileName);
		}
		else
		{
			v1.Format(_T("Are you sure you want to delete item %s"), FileName);
		}
	}
	if (::MessageBox(m_hWnd, v1, _T(""), MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		return;
	}
	_M_CListCtrl_File_Server.EnableWindow(FALSE);

	POSITION Position = _M_CListCtrl_File_Server.GetFirstSelectedItemPosition();
	while (Position)
	{
		int Item = _M_CListCtrl_File_Server.GetNextSelectedItem(Position);
		CString	FileFullPath = _M_ServerFileFullPath + _M_CListCtrl_File_Server.GetItemText(Item, 0);

		if (_M_CListCtrl_File_Server.GetItemData(Item))
		{
			FileFullPath += _T('\\');
			DeleteDirectory(FileFullPath.GetBuffer());
		}
		else
		{
			DeleteFile(FileFullPath);
		}
	}

	_M_CListCtrl_File_Server.EnableWindow();

	FixedServerFileList(_T("."));
}

void CFileManagerDlg::OnFileServerNew()
{
	if (_M_ServerFileFullPath == _T(""))
		return;

	CFileNewFolderDlg Dlg(this);

	if (Dlg.DoModal() == IDOK && Dlg._M_CEdit_String.GetLength())
	{
		CString v1;
		v1 = _M_ServerFileFullPath + Dlg._M_CEdit_String + _T("\\");
		MakeSureDirPathExists(v1.GetBuffer());
		FixedServerFileList(_T("."));
	}

}

void CFileManagerDlg::OnFileServerStop()
{

}

void CFileManagerDlg::OnFileServerView()
{

}


void CFileManagerDlg::OnNMDblclkListFileClient(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	if (_M_CListCtrl_File_Client.GetSelectedCount() == 0 ||
		_M_CListCtrl_File_Client.GetItemData(_M_CListCtrl_File_Client.GetSelectionMark()) != 1)
	{
		return;
	}

	GetClientFileList();
	*pResult = 0;
	return;
}

void CFileManagerDlg::OnFileClientBack()
{
	GetClientFileList(_T(".."));
}

void CFileManagerDlg::OnFileClientDelete()
{
	MessageBox("Hello", "Hello", MB_OK);
}

void CFileManagerDlg::OnFileClientNew()
{

}

void CFileManagerDlg::OnFileClientStop()
{

}

void CFileManagerDlg::OnFileClientView()
{

}


void CFileManagerDlg::OnLvnBegindragListFileServer(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	
	if (_M_ServerFileFullPath.IsEmpty() || _M_ClientFileFullPath.IsEmpty())
		return;

	if (_M_CListCtrl_File_Server.GetSelectedCount() > 1)
	{
		_M_hCursorWnd = AfxGetApp()->LoadCursor(IDC_CURSOR_MULTI_DRAG);		
	}
	else
	{
		_M_hCursorWnd = AfxGetApp()->LoadCursor(IDC_CURSOR_SINGLE_DRAG);
	}

	_M_IsDrag = TRUE;
	
	
	_M_DragControlList = &_M_CListCtrl_File_Server;
	_M_DropControlList = &_M_CListCtrl_File_Client;

	SetCapture();

	*pResult = 0;
	return;
}

void CFileManagerDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if (_M_IsDrag)
	{
		CPoint Point(point);
		ClientToScreen(&Point);

		CWnd* v1 = WindowFromPoint(Point);

		if (v1->IsKindOf(RUNTIME_CLASS(CListCtrl)))
		{
			SetCursor(_M_hCursorWnd);
			return;
		}
		else
		{
			SetCursor(LoadCursor(NULL, IDC_NO));
		}
	}
	CDialogEx::OnMouseMove(nFlags, point);
}

void CFileManagerDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (_M_IsDrag)
	{
		ReleaseCapture();

		_M_IsDrag = FALSE;
		CPoint Point(point);

		ClientToScreen(&Point);

		CWnd* v1 = WindowFromPoint(Point);

		if (v1->IsKindOf(RUNTIME_CLASS(CListCtrl)))
		{
			_M_DropControlList = (CListCtrl*)v1;
			DropItemOnList();
		}
	}

	CDialogEx::OnLButtonUp(nFlags, point);
	return;
}

