#include "stdafx.h"

#include <tchar.h>
#include <shellapi.h>

#include "FileManager.h"



CFileManager::CFileManager(CIOCPClient* IOCPClient)
	: CManager(IOCPClient)
{
	_tprintf(_T("FileManager is under Execution\r\n"));
	SendClientVolumeInfo();
}

CFileManager::~CFileManager()
{
	_tprintf(_T("FileManager Exit\r\n"));
}

void  CFileManager::PacketHandleIO(PBYTE BufferData, ULONG_PTR BufferLength)
{
	switch (BufferData[0])
	{
	case CLIENT_GO_ON:
	{
		NotifyDialogIsOpen();
		break;
	}
	case CLIENT_FILE_MANAGER_FILE_LIST_REQUEST:
	{
		SendClientFileInfo(BufferData + 1);
		break;
	}
	case CLIENT_FILE_MANAGER_FILE_INFO_REQUIRE:
	{
		CreateClientRecvFile(BufferData + 1);
		break;
	}
	case CLIENT_FILE_MANAGER_SET_TRANSFER_MODE:
	{
		SetTransferMode(BufferData + 1);
		break;
	}
	case CLIENT_FILE_MANAGER_FILE_DATA_REPLY:
	{
		WriteClientRecvFile(BufferData + 1, BufferLength - 1);
		break;
	}
	default:
		break;
	}
	return;
}

ULONG CFileManager::SendClientVolumeInfo()
{
	TCHAR VolumeList[0x0500];
	BYTE  BufferData[0x1000];
	TCHAR FileSystem[MAX_PATH];
	
	ZeroMemory(VolumeList, sizeof(VolumeList));
	ZeroMemory(BufferData, sizeof(BufferData));
	ZeroMemory(FileSystem, sizeof(FileSystem));
	
	TCHAR* Travel = NULL;
	BufferData[0] = CLIENT_FILE_MANAGER_REPLY;
	GetLogicalDriveStrings(sizeof(VolumeList), VolumeList);

	Travel = VolumeList;
	unsigned __int64  HardDiskAmount      =  0;
	unsigned __int64  HardDiskFreeSpace   =  0;
	unsigned long     HardDiskAmountMB    =  0;
	unsigned long     HardDiskFreeSpaceMB =  0;

	DWORD offset = 0;
	for (offset  = 1; *Travel != _T('\0'); Travel += _tcslen(Travel) + sizeof(TCHAR)) {
		ZeroMemory(&FileSystem, sizeof(FileSystem));
		GetVolumeInformation(Travel, NULL, 0, NULL, NULL, NULL, FileSystem, MAX_PATH);

		ULONG FileSystemLength = _tcslen(FileSystem) + 1;
		SHFILEINFO sfi;
		
		SHGetFileInfo(
				Travel, 
				FILE_ATTRIBUTE_NORMAL, 
				&sfi,
				sizeof(SHFILEINFO), 
				SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES
			);
		ULONG HardDiskTypeNameLength = _tcslen(sfi.szTypeName) + 1;

		// 计算磁盘大小(A B 驱动盘)
		if (Travel[0] != _T('A') && Travel[0] != _T('B')
			&& GetDiskFreeSpaceEx(Travel, (PULARGE_INTEGER)&HardDiskFreeSpace,
			(PULARGE_INTEGER)&HardDiskAmount, NULL))
		{
			HardDiskAmountMB    = HardDiskAmount      / 1024 / 1024;
			HardDiskFreeSpaceMB = HardDiskFreeSpace   / 1024 / 1024;
		}
		else
		{
			HardDiskAmountMB    = 0;
			HardDiskFreeSpaceMB = 0;
		}

		BufferData[offset] = Travel[0];
		BufferData[offset + 1] = GetDriveType(Travel);

		memcpy(BufferData + offset + 2, &HardDiskAmountMB,    sizeof(ULONG));
		memcpy(BufferData + offset + 6, &HardDiskFreeSpaceMB, sizeof(ULONG));


		memcpy(BufferData + offset + 10, sfi.szTypeName, HardDiskTypeNameLength);
		memcpy(
				BufferData + offset + 10 + HardDiskTypeNameLength,
				FileSystem,
				FileSystemLength
			);

		offset += 10 + HardDiskTypeNameLength + FileSystemLength;
	}

	return _M_IOCPClient->OnServerSending((char*)BufferData, offset);
}

INT   CFileManager::SendClientFileInfo(PBYTE DirPath)
{
	_M_TransferMode = TRANSFER_MODE_NORMAL;

	DWORD	offset = 0;          // 位移指针
	char*   BufferData = NULL;
	ULONG   BufferLength = 1024 * 10; // 先分配10K的缓冲区

	BufferData = (char*)LocalAlloc(LPTR, BufferLength);
	if (BufferData == NULL) {
		return 0;
	}

	//在目录后添上*.*
	TCHAR v1[MAX_PATH];
	ZeroMemory(v1, sizeof(v1));
	_stprintf(v1, _T("%s\\\\*.*"), DirPath);


	HANDLE hFile = NULL;
	WIN32_FIND_DATA	wfd;
	hFile = FindFirstFile(v1, &wfd);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		BYTE Token = CLIENT_FILE_MANAGER_FILE_LIST_REPLY;

		if (BufferData != NULL)
		{
			LocalFree(BufferData);
			BufferData = NULL;
		}
		return _M_IOCPClient->OnServerSending((char*)&Token, 1);

	}

	BufferData[0] = CLIENT_FILE_MANAGER_FILE_LIST_REPLY;
	
	offset = 1;

	do
	{
		// 动态扩展缓冲区
		if (offset > (BufferLength - MAX_PATH * 2))
		{
			BufferLength += MAX_PATH * 2;
			BufferData = (char*)LocalReAlloc(BufferData,
				BufferLength, LMEM_ZEROINIT | LMEM_MOVEABLE);
		}
		TCHAR* FileName = wfd.cFileName;
		if (_tccmp(FileName, _T(".")) == 0 || _tccmp(FileName, _T("..")) == 0)
		{
			continue;
		}


		*(BufferData + offset) = wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;   //1为文件  0为目录
																				   //[CLENT_FILE_MANAGER_FILE_DATA_REPLY][文件属性(1字节)]
		offset++;

		// 文件名 lstrlen(pszFileName) + 1 字节
		ULONG FileNameLength = (_tcslen(FileName)) * sizeof(TCHAR);
		memcpy(BufferData + offset, FileName, FileNameLength);
		offset += FileNameLength;
		*(BufferData + offset) = 0;
		//[CLENT_FILE_MANAGER_FILE_DATA_REPLY][文件属性(1字节)][文件名\0]
		offset++;

		// 文件大小 8 字节,文件大小要分高位低位来存放
		memcpy(BufferData + offset,     &wfd.nFileSizeHigh, sizeof(DWORD));
		memcpy(BufferData + offset + 4, &wfd.nFileSizeLow,  sizeof(DWORD));
		//[CLENT_FILE_MANAGER_FILE_DATA_REPLY][文件属性(1字节)][文件名\0][文件大小(8字节)]
		offset += 8;

		// 最后访问时间 8 字节
		memcpy(BufferData + offset, &wfd.ftLastWriteTime, sizeof(FILETIME));
		//[CLENT_FILE_MANAGER_FILE_DATA_REPLY][文件属性(1字节)][文件名\0][文件大小(8字节)][最后访问时间(8字节)]
		offset += 8;
	} while (FindNextFile(hFile, &wfd));


	ULONG rLen = _M_IOCPClient->OnServerSending(BufferData, offset);
	LocalFree(BufferData);
	FindClose(hFile);
	return rLen;
}

VOID  CFileManager::WriteClientRecvFile(BYTE* BufferData, ULONG BufferLength)
{
	BYTE	*Travel;
	DWORD	NumberOfBytesToWrite = 0;
	DWORD	NumberOfBytesWrite   = 0;
	int		v3 = 9;
	FILE_SIZE* v1;

	Travel = BufferData + 8;

	v1 = (FILE_SIZE *)BufferData;
	LONG	OffsetHigh = v1->FileSizeHigh;
	LONG	OffsetLow  = v1->FileSizeLow;

	NumberOfBytesToWrite = BufferLength - 8;

	HANDLE	FileHandle = CreateFile(
							_M_FileFullPath,
							GENERIC_WRITE,
							FILE_SHARE_WRITE,
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,
							0
						);

	SetFilePointer(FileHandle, OffsetLow, &OffsetHigh, FILE_BEGIN);

	WriteFile(
			FileHandle,
			Travel,
			NumberOfBytesToWrite,
			&NumberOfBytesWrite,
			NULL
		);

	CloseHandle(FileHandle);
	BYTE	IsToken[9];

	IsToken[0] = CLIENT_FILE_MANAGER_FILE_DATA_CONTINUE;
	OffsetLow += NumberOfBytesWrite;
	memcpy(IsToken + 1, &OffsetHigh, sizeof(OffsetHigh));
	memcpy(IsToken + 5, &OffsetLow, sizeof(OffsetLow));
	_M_IOCPClient->OnServerSending((char*)&IsToken, sizeof(IsToken));
}

VOID  CFileManager::CreateClientRecvFile(LPBYTE BufferData)
{
	FILE_SIZE* v1 = (FILE_SIZE*)BufferData;

	ZeroMemory(&_M_FileFullPath, sizeof(_M_FileFullPath));
	_tcscpy(_M_FileFullPath, (TCHAR*)(BufferData + 8));

	_M_TransferFileLength =
		(v1->FileSizeHigh * (MAXDWORD + 1)) + v1->FileSizeLow;

	// 执行多重目录的创建
	MakeSureDirPathExists(_M_FileFullPath);

	WIN32_FIND_DATA wfd;
	ZeroMemory(&wfd, sizeof(wfd));
	
	HANDLE hFile = FindFirstFile(_M_FileFullPath, &wfd);

	if (hFile != INVALID_HANDLE_VALUE
		&& _M_TransferMode != TRANSFER_MODE_OVERWRITE_ALL
		&& _M_TransferMode != TRANSFER_MODE_JUMP_ALL )
	{
		// 选择模式 - 覆盖 - 跳过
		BYTE IsToken;
		IsToken = CLINET_FILE_MANAGER_TRANSFER_MODE_REQUIRE;
		_M_IOCPClient->OnServerSending((char*)&IsToken, sizeof(BYTE));
	}
	else
	{
		// 没有重复文件，请求文件数据
		GetServerFileData();
	}
	FindClose(hFile);

	return;
}

BOOL  CFileManager::MakeSureDirPathExists(TCHAR* DirFullPath)
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

VOID  CFileManager::GetServerFileData()
{
	int	TransferMode;
	switch (_M_TransferMode)
	{
	case TRANSFER_MODE_OVERWRITE_ALL:
	{
		TransferMode = TRANSFER_MODE_OVERWRITE;
		break;
	}
	case TRANSFER_MODE_JUMP_ALL:
	{
		TransferMode = TRANSFER_MODE_JUMP;
		break;
	}
	default:
	{
		TransferMode = _M_TransferMode;
		break;
	}
	}

	WIN32_FIND_DATA wfd;
	ZeroMemory(&wfd, sizeof(wfd));

	HANDLE FileHandle = FindFirstFile(_M_FileFullPath, &wfd);

	BYTE	Token[9];
	DWORD	CreationDisposition = 0; // 文件打开方式 
	ZeroMemory(Token, sizeof(Token));

	Token[0] = CLIENT_FILE_MANAGER_FILE_DATA_CONTINUE;
	if (FileHandle != INVALID_HANDLE_VALUE)
	{
		// 文件句柄不是非法，即存在相同文件
		if (TransferMode == TRANSFER_MODE_OVERWRITE)
		{
			memset(Token + 1, 0, 8);
			CreationDisposition = CREATE_ALWAYS;
		}
		else if (TransferMode == TRANSFER_MODE_JUMP)
		{
			DWORD v2 = -1;  //0000 -1
			memcpy(Token + 5, &v2, 4);
			CreationDisposition = OPEN_EXISTING;
		}
	}
	else
	{
		// 文件无重名现象
		memset(Token + 1, 0, 8);					  
		CreationDisposition = CREATE_ALWAYS;
	}
	FindClose(FileHandle);

	FileHandle =
		CreateFile(
				_M_FileFullPath,
				GENERIC_WRITE,
				FILE_SHARE_WRITE,
				NULL,
				CreationDisposition,
				FILE_ATTRIBUTE_NORMAL,
				0
			);
	// 需要错误处理
	if (FileHandle == INVALID_HANDLE_VALUE)
	{
		_M_TransferFileLength = 0;
		return;
	}
	CloseHandle(FileHandle);

	_M_IOCPClient->OnServerSending((char*)&Token, sizeof(Token));
}

VOID  CFileManager::SetTransferMode(LPBYTE BufferData)
{
	memcpy(&_M_TransferMode, BufferData, sizeof(_M_TransferMode));
	GetServerFileData();
}