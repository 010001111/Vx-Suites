
// Generate_bin.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CGenerate_binApp:
// �йش����ʵ�֣������ Generate_bin.cpp
//

class CGeneratorApp : public CWinApp
{
public:
	CGeneratorApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CGeneratorApp theApp;