/*

Copyright (c) 2007, 2008 Hiroki Asakawa info@dokan-dev.net

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "dokan.h"
#include "fileinfo.h"

BOOL g_UseStdErr;
BOOL g_DebugMode;

static void DbgPrint(LPCWSTR format, ...)
{
	if (g_DebugMode) {
		WCHAR buffer[512];
		va_list argp;
		va_start(argp, format);
		vswprintf_s(buffer, sizeof(buffer)/sizeof(WCHAR), format, argp);
		va_end(argp);
		if (g_UseStdErr) {
			fwprintf(stderr, buffer);
		} else {
			OutputDebugStringW(buffer);
		}
	}
}

static WCHAR RootDirectory[MAX_PATH] = L"C:";

static void
GetFilePath(
	PWCHAR	filePath,
	LPCWSTR FileName)
{
	RtlZeroMemory(filePath, MAX_PATH);
	wcsncpy(filePath, RootDirectory, wcslen(RootDirectory));
	wcsncat(filePath, FileName, wcslen(FileName));
}


#define MirrorCheckFlag(val, flag) if (val&flag) { DbgPrint(L"\t" L#flag L"\n"); }

static int
MirrorCreateFile(
	LPCWSTR					FileName,
	DWORD					AccessMode,
	DWORD					ShareMode,
	DWORD					CreationDisposition,
	DWORD					FlagsAndAttributes,
	PDOKAN_FILE_INFO		DokanFileInfo)
{
	WCHAR filePath[MAX_PATH];
	HANDLE handle;

	GetFilePath(filePath, FileName);

	DbgPrint(L"CreateFile : %s\n", filePath);
	
	if (CreationDisposition == CREATE_NEW)
		DbgPrint(L"\tCREATE_NEW\n");
	if (CreationDisposition == OPEN_ALWAYS)
		DbgPrint(L"\tOPEN_ALWAYS\n");
	if (CreationDisposition == CREATE_ALWAYS)
		DbgPrint(L"\tCREATE_ALWAYS\n");
	if (CreationDisposition == OPEN_EXISTING)
		DbgPrint(L"\tOPEN_EXISTING\n");
	if (CreationDisposition == TRUNCATE_EXISTING)
		DbgPrint(L"\tTRUNCATE_EXISTING\n");

	/*
	if (ShareMode == 0 && AccessMode & FILE_WRITE_DATA)
		ShareMode = FILE_SHARE_WRITE;
	else if (ShareMode == 0)
		ShareMode = FILE_SHARE_READ;
	*/

	DbgPrint(L"\tShareMode = 0x%x\n", ShareMode);

	MirrorCheckFlag(ShareMode, FILE_SHARE_READ);
	MirrorCheckFlag(ShareMode, FILE_SHARE_WRITE);
	MirrorCheckFlag(ShareMode, FILE_SHARE_DELETE);

	DbgPrint(L"\tAccessMode = 0x%x\n", AccessMode);

	MirrorCheckFlag(AccessMode, GENERIC_READ);
	MirrorCheckFlag(AccessMode, GENERIC_WRITE);
	MirrorCheckFlag(AccessMode, GENERIC_EXECUTE);
	
	MirrorCheckFlag(AccessMode, DELETE);
	MirrorCheckFlag(AccessMode, FILE_READ_DATA);
	MirrorCheckFlag(AccessMode, FILE_READ_ATTRIBUTES);
	MirrorCheckFlag(AccessMode, FILE_READ_EA);
	MirrorCheckFlag(AccessMode, READ_CONTROL);
	MirrorCheckFlag(AccessMode, FILE_WRITE_DATA);
	MirrorCheckFlag(AccessMode, FILE_WRITE_ATTRIBUTES);
	MirrorCheckFlag(AccessMode, FILE_WRITE_EA);
	MirrorCheckFlag(AccessMode, FILE_APPEND_DATA);
	MirrorCheckFlag(AccessMode, WRITE_DAC);
	MirrorCheckFlag(AccessMode, WRITE_OWNER);
	MirrorCheckFlag(AccessMode, SYNCHRONIZE);
	MirrorCheckFlag(AccessMode, FILE_EXECUTE);
	MirrorCheckFlag(AccessMode, STANDARD_RIGHTS_READ);
	MirrorCheckFlag(AccessMode, STANDARD_RIGHTS_WRITE);
	MirrorCheckFlag(AccessMode, STANDARD_RIGHTS_EXECUTE);


	// when filePath is a directory, flags is changed to the file be opened
	if (GetFileAttributes(filePath) & FILE_ATTRIBUTE_DIRECTORY) {
		FlagsAndAttributes |= FILE_FLAG_BACKUP_SEMANTICS;
		//AccessMode = 0;
	}

	DbgPrint(L"\tFlagsAndAttributes = 0x%x\n", FlagsAndAttributes);

	MirrorCheckFlag(FlagsAndAttributes, FILE_ATTRIBUTE_ARCHIVE);
	MirrorCheckFlag(FlagsAndAttributes, FILE_ATTRIBUTE_ENCRYPTED);
	MirrorCheckFlag(FlagsAndAttributes, FILE_ATTRIBUTE_HIDDEN);
	MirrorCheckFlag(FlagsAndAttributes, FILE_ATTRIBUTE_NORMAL);
	MirrorCheckFlag(FlagsAndAttributes, FILE_ATTRIBUTE_NOT_CONTENT_INDEXED);
	MirrorCheckFlag(FlagsAndAttributes, FILE_ATTRIBUTE_OFFLINE);
	MirrorCheckFlag(FlagsAndAttributes, FILE_ATTRIBUTE_READONLY);
	MirrorCheckFlag(FlagsAndAttributes, FILE_ATTRIBUTE_SYSTEM);
	MirrorCheckFlag(FlagsAndAttributes, FILE_ATTRIBUTE_TEMPORARY);
	MirrorCheckFlag(FlagsAndAttributes, FILE_FLAG_WRITE_THROUGH);
	MirrorCheckFlag(FlagsAndAttributes, FILE_FLAG_OVERLAPPED);
	MirrorCheckFlag(FlagsAndAttributes, FILE_FLAG_NO_BUFFERING);
	MirrorCheckFlag(FlagsAndAttributes, FILE_FLAG_RANDOM_ACCESS);
	MirrorCheckFlag(FlagsAndAttributes, FILE_FLAG_SEQUENTIAL_SCAN);
	MirrorCheckFlag(FlagsAndAttributes, FILE_FLAG_DELETE_ON_CLOSE);
	MirrorCheckFlag(FlagsAndAttributes, FILE_FLAG_BACKUP_SEMANTICS);
	MirrorCheckFlag(FlagsAndAttributes, FILE_FLAG_POSIX_SEMANTICS);
	MirrorCheckFlag(FlagsAndAttributes, FILE_FLAG_OPEN_REPARSE_POINT);
	MirrorCheckFlag(FlagsAndAttributes, FILE_FLAG_OPEN_NO_RECALL);
	MirrorCheckFlag(FlagsAndAttributes, SECURITY_ANONYMOUS);
	MirrorCheckFlag(FlagsAndAttributes, SECURITY_IDENTIFICATION);
	MirrorCheckFlag(FlagsAndAttributes, SECURITY_IMPERSONATION);
	MirrorCheckFlag(FlagsAndAttributes, SECURITY_DELEGATION);
	MirrorCheckFlag(FlagsAndAttributes, SECURITY_CONTEXT_TRACKING);
	MirrorCheckFlag(FlagsAndAttributes, SECURITY_EFFECTIVE_ONLY);
	MirrorCheckFlag(FlagsAndAttributes, SECURITY_SQOS_PRESENT);

	handle = CreateFile(
		filePath,
		AccessMode,//GENERIC_READ|GENERIC_WRITE|GENERIC_EXECUTE,
		ShareMode,
		NULL, // security attribute
		CreationDisposition,
		FlagsAndAttributes,// |FILE_FLAG_NO_BUFFERING,
		NULL); // template file handle

	if (handle == INVALID_HANDLE_VALUE) {
		DWORD error = GetLastError();
		DbgPrint(L"\terror code = %d\n\n", error);
		return error * -1; // error codes are negated value of Windows System Error codes
	}

	DbgPrint(L"\n");

	// save the file handle in Context
	DokanFileInfo->Context = (ULONG64)handle;
	return 0;
}


static int
MirrorCreateDirectory(
	LPCWSTR					FileName,
	PDOKAN_FILE_INFO		DokanFileInfo)
{
	WCHAR filePath[MAX_PATH];
	GetFilePath(filePath, FileName);

	DbgPrint(L"CreateDirectory : %s\n", filePath);
	if (!CreateDirectory(filePath, NULL)) {
		DWORD error = GetLastError();
		DbgPrint(L"\terror code = %d\n\n", error);
		return error * -1; // error codes are negated value of Windows System Error codes
	}
	return 0;
}


static int
MirrorOpenDirectory(
	LPCWSTR					FileName,
	PDOKAN_FILE_INFO		DokanFileInfo)
{
	WCHAR filePath[MAX_PATH];
	HANDLE handle;

	GetFilePath(filePath, FileName);

	DbgPrint(L"OpenDirectory : %s\n", filePath);

	handle = CreateFile(
		filePath,
		0,
		FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS,
		NULL);

	if (handle == INVALID_HANDLE_VALUE) {
		DWORD error = GetLastError();
		DbgPrint(L"\terror code = %d\n\n", error);
		return error * -1;
	}

	DbgPrint(L"\n");

	DokanFileInfo->Context = (ULONG64)handle;

	return 0;
}


static int
MirrorCloseFile(
	LPCWSTR					FileName,
	PDOKAN_FILE_INFO		DokanFileInfo)
{
	WCHAR filePath[MAX_PATH];
	GetFilePath(filePath, FileName);

	if (DokanFileInfo->Context) {
		DbgPrint(L"CloseFile: %s\n", filePath);
		DbgPrint(L"\terror : not cleanuped file\n\n");
		CloseHandle((HANDLE)DokanFileInfo->Context);
		DokanFileInfo->Context = 0;
	} else {
		//DbgPrint(L"Close: %s\n\tinvalid handle\n\n", filePath);
		DbgPrint(L"Close: %s\n\n", filePath);
		return 0;
	}

	//DbgPrint(L"\n");
	return 0;
}


static int
MirrorCleanup(
	LPCWSTR					FileName,
	PDOKAN_FILE_INFO		DokanFileInfo)
{
	WCHAR filePath[MAX_PATH];
	GetFilePath(filePath, FileName);

	if (DokanFileInfo->Context) {
		DbgPrint(L"Cleanup: %s\n\n", filePath);
		CloseHandle((HANDLE)DokanFileInfo->Context);
		DokanFileInfo->Context = 0;

		if (DokanFileInfo->DeleteOnClose) {
			if (DokanFileInfo->IsDirectory) {
				DbgPrint(L"  DeleteDirectory ");
				if (!RemoveDirectory(filePath)) {
					DbgPrint(L"error code = %d\n\n", GetLastError());
				} else {
					DbgPrint(L"success\n\n");
				}
			} else {
				DbgPrint(L"  DeleteFile ");
				if (DeleteFile(filePath) == 0) {
					DbgPrint(L" error code = %d\n\n", GetLastError());
				} else {
					DbgPrint(L"success\n\n");
				}
			}
		}


	} else {
		DbgPrint(L"Cleanup: %s\n\tinvalid handle\n\n", filePath);
		return -1;
	}

	return 0;
}


static int
MirrorReadFile(
	LPCWSTR				FileName,
	LPVOID				Buffer,
	DWORD				BufferLength,
	LPDWORD				ReadLength,
	LONGLONG			Offset,
	PDOKAN_FILE_INFO	DokanFileInfo)
{
	WCHAR	filePath[MAX_PATH];
	HANDLE	handle = (HANDLE)DokanFileInfo->Context;
	ULONG	offset = (ULONG)Offset;
	BOOL	opened = FALSE;

	GetFilePath(filePath, FileName);

	DbgPrint(L"ReadFile : %s\n", filePath);

	if (!handle || handle == INVALID_HANDLE_VALUE) {
		DbgPrint(L"\tinvalid handle, cleanuped?\n");
		handle = CreateFile(
			filePath,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			0,
			NULL);
		if (handle == INVALID_HANDLE_VALUE) {
			DbgPrint(L"\tCreateFile error : %d\n\n", GetLastError());
			return -1;
		}
		opened = TRUE;
	}
	
	if (SetFilePointer(handle, offset, NULL, FILE_BEGIN) == 0xFFFFFFFF) {
		DbgPrint(L"\tseek error, offset = %d\n\n", offset);
		if (opened)
			CloseHandle(handle);
		return -1;
	}

		
	if (!ReadFile(handle, Buffer, BufferLength, ReadLength,NULL)) {
		DbgPrint(L"\tread error = %u, buffer length = %d, read length = %d\n\n",
			GetLastError(), BufferLength, *ReadLength);
		if (opened)
			CloseHandle(handle);
		return -1;

	} else {
		DbgPrint(L"\tread %d, offset %d\n\n", *ReadLength, offset);
	}

	if (opened)
		CloseHandle(handle);

	return 0;
}


static int
MirrorWriteFile(
	LPCWSTR		FileName,
	LPCVOID		Buffer,
	DWORD		NumberOfBytesToWrite,
	LPDWORD		NumberOfBytesWritten,
	LONGLONG			Offset,
	PDOKAN_FILE_INFO	DokanFileInfo)
{
	WCHAR	filePath[MAX_PATH];
	HANDLE	handle = (HANDLE)DokanFileInfo->Context;
	ULONG	offset = (ULONG)Offset;
	BOOL	opened = FALSE;

	GetFilePath(filePath, FileName);

	DbgPrint(L"WriteFile : %s, offset %I64d, length %d\n", filePath, Offset, NumberOfBytesToWrite);
	//DbgPrint(L"----\n%s\n----\n\n", Buffer);

	//DbgPrint(L"press any key?");
	//getchar();


	// reopen the file
	if (!handle || handle == INVALID_HANDLE_VALUE) {
		DbgPrint(L"\tinvalid handle, cleanuped?\n");
		handle = CreateFile(
			filePath,
			GENERIC_WRITE,
			FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			0,
			NULL);
		if (handle == INVALID_HANDLE_VALUE) {
			DbgPrint(L"\tCreateFile error : %d\n\n", GetLastError());
			return -1;
		}
		opened = TRUE;
		
		return -1;
	}

	if (SetFilePointer(handle, offset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
		DbgPrint(L"\tseek error, offset = %d, error = %d\n", offset, GetLastError());
		return -1;
	}

		
	if (!WriteFile(handle, Buffer, NumberOfBytesToWrite, NumberOfBytesWritten, NULL)) {
		DbgPrint(L"\twrite error = %u, buffer length = %d, write length = %d\n",
			GetLastError(), NumberOfBytesToWrite, *NumberOfBytesWritten);
		return -1;

	} else {
		DbgPrint(L"\twrite %d, offset %d\n\n", *NumberOfBytesWritten, offset);
	}

	// close the file when it is reopened
	if (opened)
		CloseHandle(handle);

	return 0;
}


static int
MirrorFlushFileBuffers(
	LPCWSTR		FileName,
	PDOKAN_FILE_INFO	DokanFileInfo)
{
	WCHAR	filePath[MAX_PATH];
	HANDLE	handle = (HANDLE)DokanFileInfo->Context;

	GetFilePath(filePath, FileName);

	DbgPrint(L"FlushFileBuffers : %s\n", filePath);

	if (!handle || handle == INVALID_HANDLE_VALUE) {
		DbgPrint(L"\tinvalid handle\n\n");
		return 0;
	}

	if (FlushFileBuffers(handle)) {
		return 0;
	} else {
		DbgPrint(L"\tflush error code = %d\n", GetLastError());
		return -1;
	}

}


static int
MirrorGetFileInformation(
	LPCWSTR							FileName,
	LPBY_HANDLE_FILE_INFORMATION	HandleFileInformation,
	PDOKAN_FILE_INFO				DokanFileInfo)
{
	WCHAR	filePath[MAX_PATH];
	HANDLE	handle = (HANDLE)DokanFileInfo->Context;
	BOOL	opened = FALSE;

	GetFilePath(filePath, FileName);

	DbgPrint(L"GetFileInfo : %s\n", filePath);

	if (!handle || handle == INVALID_HANDLE_VALUE) {
		DbgPrint(L"\tinvalid handle\n\n");

		// If CreateDirectory returned FILE_ALREADY_EXISTS and 
		// it is called with FILE_OPEN_IF, that handle must be opened.
		handle = CreateFile(filePath, 0, FILE_SHARE_READ, NULL, OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS, NULL);
		if (handle == INVALID_HANDLE_VALUE)
			return -1;
		opened = TRUE;
	}

	if (!GetFileInformationByHandle(handle,HandleFileInformation)) {
		DbgPrint(L"\terror code = %d\n", GetLastError());

		// FileName is a root directory
		// in this case, FindFirstFile can't get directory information
		if (wcslen(FileName) == 1) {
			HandleFileInformation->dwFileAttributes = GetFileAttributes(filePath);

		} else {
			WIN32_FIND_DATAW find;
			ZeroMemory(&find, sizeof(WIN32_FIND_DATAW));
			handle = FindFirstFile(filePath, &find);
			if (handle == INVALID_HANDLE_VALUE) {
				DbgPrint(L"\tFindFirstFile error code = %d\n\n", GetLastError());
				return -1;
			}
			HandleFileInformation->dwFileAttributes = find.dwFileAttributes;
			HandleFileInformation->ftCreationTime = find.ftCreationTime;
			HandleFileInformation->ftLastAccessTime = find.ftLastAccessTime;
			HandleFileInformation->ftLastWriteTime = find.ftLastWriteTime;
			HandleFileInformation->nFileSizeHigh = find.nFileSizeHigh;
			HandleFileInformation->nFileSizeLow = find.nFileSizeLow;
			DbgPrint(L"\tFindFiles OK\n");
			CloseHandle(handle);
		}
	}

	DbgPrint(L"\n");

	if (opened)
		CloseHandle(handle);

	return 0;
}


static int
MirrorFindFiles(
	LPCWSTR				FileName,
	PFillFindData		FillFindData, // function pointer
	PDOKAN_FILE_INFO	DokanFileInfo)
{
	WCHAR				filePath[MAX_PATH];
	HANDLE				hFind;
	WIN32_FIND_DATAW	findData;
	DWORD				error;
	PWCHAR				yenStar = L"\\*";
	int count = 0;

	GetFilePath(filePath, FileName);

	wcscat(filePath, yenStar);
	DbgPrint(L"FindFiles :%s\n", filePath);

	hFind = FindFirstFile(filePath, &findData);

	if (hFind == INVALID_HANDLE_VALUE) {
		DbgPrint(L"\tinvalid file handle. Error is %u\n\n", GetLastError());
		return -1;
	}

	FillFindData(&findData, DokanFileInfo);
	count++;

	while (FindNextFile(hFind, &findData) != 0) {
 		FillFindData(&findData, DokanFileInfo);
		count++;
	}
	
	error = GetLastError();
	FindClose(hFind);

	if (error != ERROR_NO_MORE_FILES) {
		DbgPrint(L"\tFindNextFile error. Error is %u\n\n", error);
		return -1;
	}

	DbgPrint(L"\tFindFiles return %d entries in %s\n\n", count, filePath);

	return 0;
}


static int
MirrorDeleteFile(
	LPCWSTR				FileName,
	PDOKAN_FILE_INFO	DokanFileInfo)
{
	WCHAR	filePath[MAX_PATH];
	HANDLE	handle = (HANDLE)DokanFileInfo->Context;

	GetFilePath(filePath, FileName);

	DbgPrint(L"DeleteFile %s\n", filePath);

	return 0;
}


static int
MirrorDeleteDirectory(
	LPCWSTR				FileName,
	PDOKAN_FILE_INFO	DokanFileInfo)
{
	WCHAR	filePath[MAX_PATH];
	HANDLE	handle = (HANDLE)DokanFileInfo->Context;
	HANDLE	hFind;
	WIN32_FIND_DATAW	findData;

	GetFilePath(filePath, FileName);

	DbgPrint(L"DeleteDirectory %s\n", filePath);

	hFind = FindFirstFile(filePath, &findData);
	if (hFind == INVALID_HANDLE_VALUE) {
		if (GetLastError() == ERROR_NO_MORE_FILES) {
			return 0;
		} else {
			DbgPrint(L"\tinvalid file handle. Error is %u\n\n", GetLastError());
			return -1;
		}
	} else {
		return -(int)STATUS_DIRECTORY_NOT_EMPTY;
	}
	
	return 0;
}


static int
MirrorMoveFile(
	LPCWSTR				FileName, // existing file name
	LPCWSTR				NewFileName,
	BOOL				ReplaceIfExisting,
	PDOKAN_FILE_INFO	DokanFileInfo)
{
	WCHAR			filePath[MAX_PATH];
	WCHAR			newFilePath[MAX_PATH];
	BOOL			status;

	GetFilePath(filePath, FileName);
	GetFilePath(newFilePath, NewFileName);

	DbgPrint(L"MoveFile %s -> %s\n\n", filePath, newFilePath);

	if (DokanFileInfo->Context) {
		// should close? or rename at closing?
		CloseHandle((HANDLE)DokanFileInfo->Context);
		DokanFileInfo->Context = 0;
	}

	if (ReplaceIfExisting)
		status = MoveFileEx(filePath, newFilePath, MOVEFILE_REPLACE_EXISTING);
	else
		status = MoveFile(filePath, newFilePath);

	if (status == FALSE) {
		DWORD error = GetLastError();
		DbgPrint(L"\tMoveFile failed status = %d, code = %d\n", status, error);
		return -(int)error;
	} else {
		return 0;
	}
}


static int
MirrorLockFile(
	LPCWSTR				FileName,
	LONGLONG			ByteOffset,
	LONGLONG			Length,
	PDOKAN_FILE_INFO	DokanFileInfo)
{
	WCHAR	filePath[MAX_PATH];
	HANDLE	handle;
	LARGE_INTEGER offset;
	LARGE_INTEGER length;

	GetFilePath(filePath, FileName);

	DbgPrint(L"LockFile %s\n", filePath);

	handle = (HANDLE)DokanFileInfo->Context;
	if (!handle || handle == INVALID_HANDLE_VALUE) {
		DbgPrint(L"\tinvalid handle\n\n");
		return -1;
	}

	length.QuadPart = Length;
	offset.QuadPart = ByteOffset;

	if (LockFile(handle, offset.HighPart, offset.LowPart, length.HighPart, length.LowPart)) {
		DbgPrint(L"\tsuccess\n\n");
		return 0;
	} else {
		DbgPrint(L"\tfail\n\n");
		return -1;
	}
}


static int
MirrorSetEndOfFile(
	LPCWSTR				FileName,
	LONGLONG			ByteOffset,
	PDOKAN_FILE_INFO	DokanFileInfo)
{
	WCHAR			filePath[MAX_PATH];
	HANDLE			handle;
	LARGE_INTEGER	offset;

	GetFilePath(filePath, FileName);

	DbgPrint(L"SetEndOfFile %s, %I64d\n", filePath, ByteOffset);

	handle = (HANDLE)DokanFileInfo->Context;
	if (!handle || handle == INVALID_HANDLE_VALUE) {
		DbgPrint(L"\tinvalid handle\n\n");
		return -1;
	}

	offset.QuadPart = ByteOffset;
	if (!SetFilePointerEx(handle, offset, NULL, FILE_BEGIN)) {
		DbgPrint(L"\tSetFilePointer error: %d, offset = %I64d\n\n", GetLastError(), ByteOffset);
		return GetLastError() * -1;
	}

	if (!SetEndOfFile(handle)) {
		DWORD error = GetLastError();
		DbgPrint(L"\terror code = %d\n\n", error);
		return error * -1;
	}

	return 0;
}


static int
MirrorSetFileAttributes(
	LPCWSTR				FileName,
	DWORD				FileAttributes,
	PDOKAN_FILE_INFO	DokanFileInfo)
{
	WCHAR	filePath[MAX_PATH];
	
	GetFilePath(filePath, FileName);

	DbgPrint(L"SetFileAttributes %s\n", filePath);

	if (!SetFileAttributes(filePath, FileAttributes)) {
		DWORD error = GetLastError();
		DbgPrint(L"\terror code = %d\n\n", error);
		return error * -1;
	}

	DbgPrint(L"\n");
	return 0;
}


static int
MirrorSetFileTime(
	LPCWSTR				FileName,
	CONST FILETIME*		CreationTime,
	CONST FILETIME*		LastAccessTime,
	CONST FILETIME*		LastWriteTime,
	PDOKAN_FILE_INFO	DokanFileInfo)
{
	WCHAR	filePath[MAX_PATH];
	HANDLE	handle;

	GetFilePath(filePath, FileName);

	DbgPrint(L"SetFileTime %s\n", filePath);

	handle = (HANDLE)DokanFileInfo->Context;

	if (!handle || handle == INVALID_HANDLE_VALUE) {
		DbgPrint(L"\tinvalid handle\n\n");
		return -1;
	}

	if (!SetFileTime(handle, CreationTime, LastAccessTime, LastWriteTime)) {
		DWORD error = GetLastError();
		DbgPrint(L"\terror code = %d\n\n", error);
		return error * -1;
	}

	DbgPrint(L"\n");
	return 0;
}



static int
MirrorUnlockFile(
	LPCWSTR				FileName,
	LONGLONG			ByteOffset,
	LONGLONG			Length,
	PDOKAN_FILE_INFO	DokanFileInfo)
{
	WCHAR	filePath[MAX_PATH];
	HANDLE	handle;
	LARGE_INTEGER	length;
	LARGE_INTEGER	offset;

	GetFilePath(filePath, FileName);

	DbgPrint(L"UnlockFile %s\n", filePath);

	handle = (HANDLE)DokanFileInfo->Context;
	if (!handle || handle == INVALID_HANDLE_VALUE) {
		DbgPrint(L"\tinvalid handle\n\n");
		return -1;
	}

	length.QuadPart = Length;
	offset.QuadPart = ByteOffset;

	if (UnlockFile(handle, offset.HighPart, offset.LowPart, length.HighPart, length.LowPart)) {
		DbgPrint(L"\tsuccess\n\n");
		return 0;
	} else {
		DbgPrint(L"\tfail\n\n");
		return -1;
	}
}


static int
MirrorUnmount(
	PDOKAN_FILE_INFO	DokanFileInfo)
{
	DbgPrint(L"Unmount\n");
	return 0;
}


static DOKAN_OPERATIONS
dokanOperations = {
	MirrorCreateFile,
	MirrorOpenDirectory,
	MirrorCreateDirectory,
	MirrorCleanup,
	MirrorCloseFile,
	MirrorReadFile,
	MirrorWriteFile,
	MirrorFlushFileBuffers,
	MirrorGetFileInformation,
	MirrorFindFiles,
	NULL, // FindFilesWithPattern
	MirrorSetFileAttributes,
	MirrorSetFileTime,
	MirrorDeleteFile,
	MirrorDeleteDirectory,
	MirrorMoveFile,
	MirrorSetEndOfFile,
	MirrorLockFile,
	MirrorUnlockFile,
	NULL, // GetDiskFreeSpace
	NULL, // GetVolumeInformation
	MirrorUnmount // Unmount
};



int __cdecl
main(ULONG argc, PCHAR argv[])
{
	int status;
	ULONG command;
	PDOKAN_OPTIONS dokanOptions = (PDOKAN_OPTIONS)malloc(sizeof(DOKAN_OPTIONS));

	if (argc < 5) {
		fprintf(stderr, "mirror.exe\n"
			"  /r RootDirectory (ex. /r c:\\test)\n"
			"  /l DriveLetter (ex. /l m)\n"
			"  /t ThreadCount (ex. /t 5)\n"
			"  /d (enable debug output)\n"
			"  /s (use stderr for output)");
		return -1;
	}

	g_DebugMode = FALSE;
	g_UseStdErr = FALSE;

	ZeroMemory(dokanOptions, sizeof(DOKAN_OPTIONS));
	dokanOptions->ThreadCount = 0; // use default

	for (command = 1; command < argc; command++) {
		switch (tolower(argv[command][1])) {
		case 'r':
			command++;
			mbstowcs(RootDirectory, argv[command], strlen(argv[command]));
			DbgPrint(L"RootDirectory: %ls\n", RootDirectory);
			break;
		case 'l':
			command++;
			dokanOptions->DriveLetter = argv[command][0];
			break;
		case 't':
			command++;
			dokanOptions->ThreadCount = (USHORT)atoi(argv[command]);
			break;
		case 'd':
			g_DebugMode = TRUE;
			break;
		case 's':
			g_UseStdErr = TRUE;
			break;
		default:
			fprintf(stderr, "unknown command: %s\n", argv[command]);
			return -1;
		}
	}

	dokanOptions->DebugMode = (UCHAR)g_DebugMode;
	dokanOptions->UseStdErr = (UCHAR)g_UseStdErr;
	dokanOptions->UseKeepAlive = 1;

	status = DokanMain(dokanOptions, &dokanOperations);
	switch (status) {
		case DOKAN_SUCCESS:
			fprintf(stderr, "Success\n");
			break;
		case DOKAN_ERROR:
			fprintf(stderr, "Error\n");
			break;
		case DOKAN_DRIVE_LETTER_ERROR:
			fprintf(stderr, "Bad Drive letter\n");
			break;
		case DOKAN_DRIVER_INSTALL_ERROR:
			fprintf(stderr, "Can't install driver\n");
			break;
		case DOKAN_START_ERROR:
			fprintf(stderr, "Driver something wrong\n");
			break;
		case DOKAN_MOUNT_ERROR:
			fprintf(stderr, "Can't assign a drive letter\n");
			break;
		default:
			fprintf(stderr, "Unknown error: %d\n", status);
			break;
	}

	return 0;
}

