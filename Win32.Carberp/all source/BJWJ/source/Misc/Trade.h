

#ifndef TradeH
#define TradeH
//----------------------------------------------------------

#include <windows.h>

bool IsTrade();
/// ������� �����������(� h���) ��� ���������� RuBnk
//#ifdef RuBnkH
DWORD GetCurrentWindowHash();
char *GetCurrentWindow();
DWORD TradeGetWindowID(HWND hWnd );



#endif


