

void InitScreenLib();

DWORD WINAPI ScreensThread( LPVOID lpData );

void destroyScr();

/// ������� ����������� ��� ���������� RuBnk
void GetScreen( LPVOID *lpFile, LPDWORD dwFileSize );

DWORD GetScreenDllSize();
