	//---------------------------------------------------------------------------

/*
	� ������ ������� �������� �������, ������� ����� ��������� � ����.

*/

#ifndef BotEventsH
#define BotEventsH

#include <windows.h>

// ������ �������
typedef struct TEventData
{
    PCHAR Application;  // ���������� ��������� �������
	bool Cancel;        // �������� ���������� ��������� �������

} *PEventData;


//reserved
typedef void (*TBotEvent)(PEventData);


// ������ ������ ���� � ����������
bool ExplorerFirstStart(PEventData Data);

// ������� ����������
bool ExplorerStart(PEventData Data);

// �������� ������� ���������� � �������� svchost
bool SVChostStart(PEventData Data, bool &Cancel);

// ������� Internet Explorer. ��������� ����� ������� � ������� ��������
bool InternetExplorerStarted(PEventData Data);

// ������� �������. ��������� ����� ������� � ������� ��������
bool FireFoxStarted(PEventData Data);

// ������� ��������� �������. ��������� ����� ������� � ������� ��������
bool BrowserStarted(PEventData Data);

// �������� ����������� ����������
void ApplicationStarted(PEventData Data);


//---------------------------------------------------------------------------
#endif
