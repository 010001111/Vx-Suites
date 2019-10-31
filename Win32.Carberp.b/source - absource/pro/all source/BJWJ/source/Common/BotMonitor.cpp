//---------------------------------------------------------------------------


#include "Modules.h"

// ��� ��������� ������ � ������ ���� � �������
// �������� BotMonitor.h

#ifdef BOTMONITOR
//---------------------------------------------------------------------


#include "BotMonitor.h"
#include "Pipes.h"

#pragma hdrstop
//---------------------------------------------------------------------------

bool BotMonitor::StartServer()
{
	// ��������� ��������� ����� ��������, ������� ����� �����������
	// � �������� �����������.
	PProcessPipe Monitor = PIPE::CreateProcessPipe((PCHAR)ServerName, true);
   	return Monitor != NULL;
}
//---------------------------------------------------------------------------

bool BotMonitor::SendMessage(PCHAR Message, PCHAR Data, DWORD DataSize)
{
	//  ������� ���������� ��������� ������� ��������
	return PIPE::SendMessage((PCHAR)ClientName, Message, Data, DataSize, NULL);
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
#endif
