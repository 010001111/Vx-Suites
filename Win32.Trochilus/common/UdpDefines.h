#pragma once
#include <map>

#define UDP_FLAG 0xAFAFBFBA


//TCP��ͷ
typedef struct
{
	DWORD flag;
	int nSize;
}UDP_HEADER,*PUDPHEADER;
