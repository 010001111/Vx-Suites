#include <windows.h>
#include <winsock.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "main.h"
extern HWND hwnd;
extern SOCKET clones[42];
SOCKADDR_IN sockit;
LPHOSTENT lpHostEntry;
void clone_parse(char *ins, SOCKET sockkz);

char buffpong[100];
int clonecallback(SOCKET Csock, LPARAM lParam)
{
	char buffq[1000];
	char *ptr;
    int i;
	char tmpest[300];
	char randstg[20];
	memset(tmpest, 0, 300);
	memset(randstg,0,20);
	switch(lParam)
	{
	case FD_READ:
		memset(buffq,0,1000);
		recv(Csock, buffq,1000,0);
		clone_parse(buffq,Csock);
		ptr = strstr(buffq,"\n");    //looking for cr
		while (ptr != 0)
		{
			clone_parse(ptr,Csock);
			ptr = ptr +1;
			clone_parse(ptr,Csock);
			ptr = strstr(ptr,"\r\n");
		}
		break;
	case FD_CLOSE:
		closesocket(Csock);
		for (i = 0; i <= 40; i++ )
		{
			if (clones[i] == Csock)
			{
				clones[i] = 0;
			}
		}

		closesocket(Csock);
		break;
	case FD_CONNECT:
		if(!HIWORD(lParam))
		{
			for (i = 0; i < 10; i++ )
			{
				char randchr[1];
				sprintf(randchr,"%c",(((rand()+26) %26) + 64));
				lstrcat(randstg,randchr);
				//strcat((char *) (((rand()+26) %26) + 64),&randstg);
			}

			sprintf(tmpest,"USER %s 127.0.0.1 127.0.0.1 %s\r\nNICK %s\r\n",randstg,randstg,randstg);
			send(Csock,tmpest,lstrlen(tmpest),0);
		}
		else
		{
			closesocket(Csock);
			for (i = 0; i <= 40; i++ )
			{
				if (clones[i] == Csock)
				{
					clones[i] = 0;
				}
			}
		}


		break;
	}

	return 1;
}

void clone_parse(char *ins, SOCKET sockkz)
{
	int Qc;
	if(memcmp(ins,"PING",4) == 0)
	{
		memset(buffpong,0,100);
		ins = ins + 5;
//		lstrcat(buffpong,"PONG ");
//		lstrcat(buffpong,ins);
		sprintf(buffpong,"PONG %s",ins);
   		send(sockkz,buffpong,lstrlen(buffpong),0);
	}
}

void clonesdie()     //kills the clones
{
int i;

		for (i = 0; i <= 40; i++ )
		{
			if (clones[i] != 0)
			{
				closesocket(clones[i]);
				clones[i] = 0;
			}
		}
}

void clonesraw( char *lpcommand) // sends the text raw to the server
{
int i;
		for (i = 0; i <= 40; i++ )
		{
			if (clones[i] != 0)
			{
				send(clones[i],lpcommand,lstrlen(lpcommand),0);
			}
		}
}


int clonesload(char *c_server, int c_port, int c_amount)
{
	int clonesloaded = 0;
    int tcount;

	for (tcount = 0; tcount <= 40; tcount++ )  //scan for an open place to put the socket
	{
		if (clones[tcount] == 0)
		{
			sockit.sin_addr.s_addr = inet_addr(c_server);
			if (sockit.sin_addr.s_addr == INADDR_NONE)
			{
				// The server name its not an ip, Resove it //
				lpHostEntry = gethostbyname( c_server );
				if (lpHostEntry != 0)
				{
					sockit.sin_addr.s_addr = *(int long *)lpHostEntry->h_addr_list[0];
				}
				else
			    {
					return 5;    //non resovable
				}
			}
			if (clonesloaded < c_amount)
			{
				clonesloaded++;
				clones[tcount] = socket(PF_INET,SOCK_STREAM,0);   //save socket
				WSAAsyncSelect(clones[tcount],hwnd, 5807, FD_CONNECT | FD_CLOSE | FD_READ);
				sockit.sin_port = htons(c_port);
				sockit.sin_family = AF_INET;
				connect(clones[tcount],&sockit,sizeof(sockit));
			}
			else
			if (clonesloaded >= c_amount)
			{
				return 88; //all the clones are probably loaded good
			}
		}
	}
return 89; //no spaceleft
}


