
// Trochilus.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CTrochilusApp:
// �йش����ʵ�֣������ Trochilus.cpp
//

class CTrochilusApp : public CWinApp
{
public:
	CTrochilusApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CTrochilusApp theApp;