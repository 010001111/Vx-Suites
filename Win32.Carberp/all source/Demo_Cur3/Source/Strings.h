#include <windows.h>

#ifndef StringsH
#define StringsH
//-----------------------------------------------------------------------------

int WINAPI UrlEncode( char *pszDestiny, char *pszSource );

void DbgMsg(char *file, int line, char *msg, ...);
void DbgMsgW(WCHAR *file, int line, WCHAR *msg, ...);

int m_atoi( const char *nptr );

void   WINAPI m_lstrcat( char *szBuf, const char *szStr );
void   WINAPI m_lstrcpy( char *szBuf, const char *szStr );
void   WINAPI m_lstrlwr( char *str );

void WINAPI m_lwcscat( WCHAR *szBuf, const WCHAR *szStr );

char * WINAPI m_strstr( const char * _Str, const char * _SubStr );
char * WINAPI m_strstrmask( const char * _Str, const char * _SubStr );
char * WINAPI m_strtok_s( char *String, const char *Control, char **Context );

char * WINAPI ToAnsi( LPCWSTR String );
char * WINAPI ToAnsiEx( LPCWSTR String, DWORD dwSize );

bool   WINAPI m_lstrncpy( char *pString_src, const char *pString_dst, DWORD len );
bool   WINAPI SearchByMask( char *mask, char *name );

int  WINAPI m_istrstr( const char * _Str, const char * _SubStr );
DWORD  WINAPI m_lstrcmp( const char *szStr1, const char *szStr2 );
DWORD  WINAPI m_lstrncmp( const char *szstr1, const char *szstr2, int nlen );
DWORD  WINAPI m_lstrlen( char *szPointer );

DWORD WINAPI m_wcslen( const wchar_t *String );

wchar_t *m_wcsncpy( wchar_t *dest, wchar_t *src, unsigned long n );
wchar_t *m_wcslwr( wchar_t *Str );
bool m_wcsncmp( WCHAR *s1, WCHAR *s2, size_t iMaxLen );


bool WildCmp( char *Buffer, char *Mask, LPDWORD dwStart, LPDWORD dwEnd, LPDWORD dwLen );
bool CompareUrl( char *MaskUrl, char *Url );

WCHAR * AnsiToUnicode( char *AnsiString, DWORD dwStrLen );
void AlertError( LPTSTR lpszFunction ) ;


// ----------------------------------------------------------------------------
//  ����� ������� ��� ������ �� ��������
//
//  ���� ������������ ������ ������� ��� ������ �� ��������, ��? � ���������
//  �������, ����� ������� �������� ��������������. ��������, �������
//  StrLength �����������  ��������� �� �������������� ������� �������
//  ������������ ����� ������ �������� � �� �������� �������, � ��� ������
//  ������ ��� ������ ������� � ��������������
// ----------------------------------------------------------------------------


// ������ � ������ ������ ������ ������� ��� �� ����� �������� StrLen + 1
PCHAR StrAlloc(DWORD StrLen);

//  ������� ����� ������ �� ������ Source (������ ������� ������ Source)
//  ���� CopySize = 0 �� ���������� ��� ������
PCHAR StrNew(PCHAR Source, DWORD CopySize = 0);

// ������� ������ ������ ��������� ��������� �����
PCHAR StrNew(DWORD Count, PCHAR Str...);

// �������� ������ ������ ������
// �����!!! ������� �������� �� �������� ������� ������� �������� StrNew
void StrSetLength(PCHAR &Str, DWORD NewLength);

// ���������� ������.
// �����!!! ������ ������ ���� ������� � ������� ������� StrNew
void StrFree(PCHAR Str);

// ������� ���������� ����� ������.
DWORD StrLength(PCHAR Str);

// ������� ����������� ����� ������ �������� �� � ������� ������������ ����
DWORD StrCalcLength(PCHAR Source);

// ������� ���������� ������ ���� �� ����� ������� ��������� ����
// ������ ������
bool StrIsEmpty(PCHAR Str);

//  ������� StrConcat ���������� ��� � ����� ������.
//  ���� ������ ������ ������ Str1 ������ ������������,
//  �� ������ Str1 ����� ��������������
//  Count ���������� ������� ����� ����� ��������� � Str1
void StrConcat(PCHAR &Str1, PCHAR Str2);
void StrConcat(PCHAR &Str1, DWORD Count, PCHAR Str2...);


//-----------------------------------------------------------------------------
#endif
