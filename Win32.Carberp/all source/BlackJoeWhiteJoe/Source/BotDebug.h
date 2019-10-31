//---------------------------------------------------------------------------

/*
	������ ���������� ������� ��� ������� ����.

	������� ������ ����� �������� ��� ���������� ��������� �������������

	DEBUGBOT

*/

#ifndef DebugUtils
#define DebugUtils

#include <Windows.h>

// ���������������� ������ ��� ������� ���� � ������ ���������
#ifdef DEBUGBOT
bool StartInDebugingMode(bool ShMSG);
#endif

// ������� ���������
//void DebugMessage(PCHAR Msg);


// ����� ��������� ���������� �����
typedef void (WINAPI *TDebugMessageHandlerMethod)(LPVOID Handler, PCHAR Module,
	DWORD Line, PCHAR Section, PCHAR ExtData, PCHAR Str);


// ���������� ���������� ���������� �����
void SetDebugMessageHandler(LPVOID Handler, TDebugMessageHandlerMethod Method);

// ���������� �� ���������� ���������
typedef struct TDebugMessage
{
	PCHAR Module;
	DWORD Line;
	PCHAR Section;
	PCHAR Data;
} *PDebugMessage;

//----------------------------------------------------------------------------
//  Debug - ������ ��� ������ � ����������� �������
//----------------------------------------------------------------------------
namespace Debug
{
	void Message(PCHAR Module, PCHAR Str);
	void MessageEx(PCHAR Module, DWORD Line, PCHAR Section, PCHAR ExtData, PCHAR Str, ...);
}

//---------------------------------------------------------------------------
#endif


