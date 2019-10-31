#pragma once
#include "Redirector.h"

class CmdRedirector : public Redirector
{
public:
	CmdRedirector();
	~CmdRedirector();

	BOOL Start();
	void Stop();

private:
	// Redirector�ӿڵ�ʵ��
	virtual void OnChildStarted( LPCSTR lpszCmdLine );
	virtual void OnChildStdErrWrite( LPCSTR lpszOutput );
	virtual void OnChildStdOutWrite( LPCSTR lpszOutput );
	virtual void OnChildTerminate();

	void SendOutput(LPCSTR output);
};
