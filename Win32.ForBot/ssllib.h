#ifndef __SSLLIB_H__
#define __SSLLIB_H__

#ifndef AGOBOT_NO_OPENSSL

class CSSLSocket
{
public:
	CSSLSocket();
	virtual ~CSSLSocket();

	bool Init();
	bool AttachToSocket(int sSocket);
	int GetSocket();

	bool Accept();
	bool Connect();
	void Close();
	int Read(void *pBuf, int iNum);
	int Write(const void *pBuf, int iNum);

	bool m_bConnected;
protected:
	SSL_CTX	*m_psslCtx;
	SSL		*m_pSSL;
	int		 m_sSocket;
};

#else

class CSSLSocket;

#endif // AGOBOT_NO_OPENSSL

#endif // __SSLLIB_H__
