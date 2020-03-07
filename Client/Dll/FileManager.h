#pragma once
#include "Manager.h"
class CFileManager :
	public CManager
{
private:
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
	CFileManager(CIOCPClient* IOCPClient);
	virtual ~CFileManager();
public:
	void  PacketHandleIO(PBYTE BufferData, ULONG_PTR BufferLength);
	ULONG SendClientVolumeInfo();
	INT   SendClientFileInfo(PBYTE DirPath);
	VOID  CreateClientRecvFile(LPBYTE BufferData);
	BOOL  MakeSureDirPathExists(TCHAR* DirFullPath);
	VOID  SetTransferMode(LPBYTE BufferData);
	VOID  GetServerFileData();
	VOID  WriteClientRecvFile(BYTE* BufferData, ULONG BufferLength);
protected:
	TCHAR    _M_FileFullPath[MAX_PATH];
	__int64  _M_TransferFileLength;
	DWORD    _M_TransferMode;

};

