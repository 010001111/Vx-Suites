
#include "windows.h"

// ������� ������� ������ ��������� � Macromedia Flash Player
void WINAPI DeleteBrowsersCookies();

bool DeleteFFCookies();
bool DeleteSOL();

BOOL Delete_IECookies_Norm(BOOL bDeleteCookies, BOOL bDeleteCookiesIndex);

BOOL Delete_IECookies_Vista();
bool HookCookie();



