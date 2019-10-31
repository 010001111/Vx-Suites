/*
 * PROJECT:     ReactOS advapi32
 * LICENSE:     GPL - See COPYING in the top level directory
 * FILE:        dll/win32/advapi32/service/sctrl.c
 * PURPOSE:     Service control manager functions
 * COPYRIGHT:   Copyright 1999 Emanuele Aliberti
 *              Copyright 2007 Ged Murphy <gedmurphy@reactos.org>
 *                             Gregor Brunmar <gregor.brunmar@home.se>
 *
 */


/* INCLUDES ******************************************************************/
#include "stdafx.h"
#include "advapi32.h"
#include "services.h"


/* TYPES *********************************************************************/

typedef struct _SERVICE_THREAD_PARAMSA
{
    LPSERVICE_MAIN_FUNCTIONA lpServiceMain;
    DWORD dwArgCount;
    LPSTR *lpArgVector;
} SERVICE_THREAD_PARAMSA, *PSERVICE_THREAD_PARAMSA;


typedef struct _SERVICE_THREAD_PARAMSW
{
    LPSERVICE_MAIN_FUNCTIONW lpServiceMain;
    DWORD dwArgCount;
    LPWSTR *lpArgVector;
} SERVICE_THREAD_PARAMSW, *PSERVICE_THREAD_PARAMSW;


typedef struct _ACTIVE_SERVICE
{
    SERVICE_STATUS_HANDLE hServiceStatus;
    UNICODE_STRING ServiceName;
    union
    {
        SERVICE_THREAD_PARAMSA A;
        SERVICE_THREAD_PARAMSW W;
    } ThreadParams;
    LPHANDLER_FUNCTION HandlerFunction;
    LPHANDLER_FUNCTION_EX HandlerFunctionEx;
    LPVOID HandlerContext;
    BOOL bUnicode;
    BOOL bOwnProcess;
} ACTIVE_SERVICE, *PACTIVE_SERVICE;


/* GLOBALS *******************************************************************/

static DWORD dwActiveServiceCount = 0;
static PACTIVE_SERVICE lpActiveServices = NULL;
static handle_t hStatusBinding = NULL;


/* FUNCTIONS *****************************************************************/

handle_t __RPC_USER
RPC_SERVICE_STATUS_HANDLE_bind(RPC_SERVICE_STATUS_HANDLE hServiceStatus)
{
    return hStatusBinding;
}


void __RPC_USER
RPC_SERVICE_STATUS_HANDLE_unbind(RPC_SERVICE_STATUS_HANDLE hServiceStatus,
                                 handle_t hBinding)
{
}


static RPC_STATUS
ScCreateStatusBinding(VOID)
{
    RPC_WSTR pszStringBinding;
    RPC_STATUS status;

    TRACE("ScCreateStatusBinding() called\n");

    status = RpcStringBindingComposeW(NULL,
                                      RPC_WSTR("ncacn_np"),
                                      NULL,
                                      RPC_WSTR("\\pipe\\ntsvcs"),
                                      NULL,
                                      &pszStringBinding);
    if (status != RPC_S_OK)
    {
        ERR("RpcStringBindingCompose returned 0x%x\n", status);
        return status;
    }

    /* Set the binding handle that will be used to bind to the server. */
    status = RpcBindingFromStringBindingW(pszStringBinding,
                                          &hStatusBinding);
    if (status != RPC_S_OK)
    {
        ERR("RpcBindingFromStringBinding returned 0x%x\n", status);
    }

    status = RpcStringFreeW(&pszStringBinding);
    if (status != RPC_S_OK)
    {
        ERR("RpcStringFree returned 0x%x\n", status);
    }

    return status;
}


static RPC_STATUS
ScDestroyStatusBinding(VOID)
{
    RPC_STATUS status;

    TRACE("ScDestroyStatusBinding() called\n");

    if (hStatusBinding == NULL)
        return RPC_S_OK;

    status = RpcBindingFree(&hStatusBinding);
    if (status != RPC_S_OK)
    {
        ERR("RpcBindingFree returned 0x%x\n", status);
    }
    else
    {
        hStatusBinding = NULL;
    }

    return status;
}


static PACTIVE_SERVICE
ScLookupServiceByServiceName(LPCWSTR lpServiceName)
{
    DWORD i;

    TRACE("ScLookupServiceByServiceName(%S) called\n", lpServiceName);

    if (lpActiveServices[0].bOwnProcess)
        return &lpActiveServices[0];

    for (i = 0; i < dwActiveServiceCount; i++)
    {
        TRACE("Checking %S\n", lpActiveServices[i].ServiceName.Buffer);
        if (_wcsicmp(lpActiveServices[i].ServiceName.Buffer, lpServiceName) == 0)
        {
            TRACE("Found!\n");
            return &lpActiveServices[i];
        }
    }

    TRACE("No service found!\n");

    SetLastError(ERROR_SERVICE_DOES_NOT_EXIST);

    return NULL;
}


static DWORD WINAPI
ScServiceMainStub(LPVOID Context)
{
    PACTIVE_SERVICE lpService = (PACTIVE_SERVICE)Context;

    TRACE("ScServiceMainStub() called\n");

    /* Call the main service routine and free the arguments vector */
    if (lpService->bUnicode)
    {
        (lpService->ThreadParams.W.lpServiceMain)(lpService->ThreadParams.W.dwArgCount,
                                                  lpService->ThreadParams.W.lpArgVector);

        if (lpService->ThreadParams.W.lpArgVector != NULL)
        {
            HeapFree(GetProcessHeap(),
                     0,
                     lpService->ThreadParams.W.lpArgVector);

            lpService->ThreadParams.W.lpArgVector = NULL;
            lpService->ThreadParams.W.dwArgCount = 0;
        }
    }
    else
    {
        (lpService->ThreadParams.A.lpServiceMain)(lpService->ThreadParams.A.dwArgCount,
                                                  lpService->ThreadParams.A.lpArgVector);

        if (lpService->ThreadParams.A.lpArgVector != NULL)
        {
            HeapFree(GetProcessHeap(),
                     0,
                     lpService->ThreadParams.A.lpArgVector);

            lpService->ThreadParams.A.lpArgVector = NULL;
            lpService->ThreadParams.A.dwArgCount = 0;
        }
    }

    return ERROR_SUCCESS;
}

//
//static DWORD
//ScConnectControlPipe(HANDLE *hPipe)
//{
//    DWORD dwBytesWritten;
//    DWORD dwState;
//    DWORD dwServiceCurrent = 0;
//    NTSTATUS Status;
//    WCHAR NtControlPipeName[MAX_PATH + 1];
//    RTL_QUERY_REGISTRY_TABLE QueryTable[2];
//    DWORD dwProcessId;
//
//    /* Get the service number and create the named pipe */
//    RtlZeroMemory(&QueryTable,
//                  sizeof(QueryTable));
//
//    QueryTable[0].Name = L"";
//    QueryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
//    QueryTable[0].EntryContext = &dwServiceCurrent;
//
//    Status = RtlQueryRegistryValues(RTL_REGISTRY_CONTROL,
//                                    L"ServiceCurrent",
//                                    QueryTable,
//                                    NULL,
//                                    NULL);
//
//    if (!NT_SUCCESS(Status))
//    {
//        ERR("RtlQueryRegistryValues() failed (Status %lx)\n", Status);
//        return RtlNtStatusToDosError(Status);
//    }
//
//    swprintf(NtControlPipeName, L"\\\\.\\pipe\\net\\NtControlPipe%u", dwServiceCurrent);
//
//    if (!WaitNamedPipeW(NtControlPipeName, 15000))
//    {
//        ERR("WaitNamedPipe(%S) failed (Error %lu)\n", NtControlPipeName, GetLastError());
//        return ERROR_FAILED_SERVICE_CONTROLLER_CONNECT;
//    }
//
//    *hPipe = CreateFileW(NtControlPipeName,
//                         GENERIC_READ | GENERIC_WRITE,
//                         0,
//                         NULL,
//                         OPEN_EXISTING,
//                         FILE_ATTRIBUTE_NORMAL,
//                         NULL);
//    if (*hPipe == INVALID_HANDLE_VALUE)
//    {
//        ERR("CreateFileW() failed for pipe %S (Error %lu)\n", NtControlPipeName, GetLastError());
//        return ERROR_FAILED_SERVICE_CONTROLLER_CONNECT;
//    }
//
//    dwState = PIPE_READMODE_MESSAGE;
//    if (!SetNamedPipeHandleState(*hPipe, &dwState, NULL, NULL))
//    {
//        CloseHandle(*hPipe);
//        *hPipe = INVALID_HANDLE_VALUE;
//        return ERROR_FAILED_SERVICE_CONTROLLER_CONNECT;
//    }
//
//    /* Pass the ProcessId to the SCM */
//    dwProcessId = GetCurrentProcessId();
//    WriteFile(*hPipe,
//              &dwProcessId,
//              sizeof(DWORD),
//              &dwBytesWritten,
//              NULL);
//
//    TRACE("Sent Process ID %lu\n", dwProcessId);
//
//    return ERROR_SUCCESS;
//}


//static DWORD
//ScBuildUnicodeArgsVector(PSCM_CONTROL_PACKET ControlPacket,
//                         LPDWORD lpArgCount,
//                         LPWSTR **lpArgVector)
//{
//    LPWSTR *lpVector;
//    LPWSTR *lpArg;
//    DWORD i;
//
//    if (ControlPacket == NULL || lpArgCount == NULL || lpArgVector == NULL)
//        return ERROR_INVALID_PARAMETER;
//
//    *lpArgCount = 0;
//    *lpArgVector = NULL;
//
//    if (ControlPacket->dwArgumentsCount > 0)
//    {
//        lpVector = HeapAlloc(GetProcessHeap(),
//                             HEAP_ZERO_MEMORY,
//                             ControlPacket->dwSize - ControlPacket->dwArgumentsOffset);
//        if (lpVector == NULL)
//            return ERROR_OUTOFMEMORY;
//
//        memcpy(lpVector,
//               ((PBYTE)ControlPacket + ControlPacket->dwArgumentsOffset),
//               ControlPacket->dwSize - ControlPacket->dwArgumentsOffset);
//
//        lpArg = lpVector;
//        for (i = 0; i < ControlPacket->dwArgumentsCount; i++)
//        {
//            *lpArg = (LPWSTR)((ULONG_PTR)lpArg + (ULONG_PTR)*lpArg);
//            lpArg++;
//        }
//
//        *lpArgCount = ControlPacket->dwArgumentsCount;
//        *lpArgVector = lpVector;
//    }
//
//    return ERROR_SUCCESS;
//}
//
//
//static DWORD
//ScBuildAnsiArgsVector(PSCM_CONTROL_PACKET ControlPacket,
//                      LPDWORD lpArgCount,
//                      LPSTR **lpArgVector)
//{
//    LPSTR *lpVector;
//    LPSTR *lpPtr;
//    LPWSTR lpUnicodeString;
//    LPSTR lpAnsiString;
//    DWORD dwVectorSize;
//    DWORD dwUnicodeSize;
//    DWORD dwAnsiSize;
//    DWORD i;
//
//    if (ControlPacket == NULL || lpArgCount == NULL || lpArgVector == NULL)
//        return ERROR_INVALID_PARAMETER;
//
//    *lpArgCount = 0;
//    *lpArgVector = NULL;
//
//    if (ControlPacket->dwArgumentsCount > 0)
//    {
//        dwVectorSize = ControlPacket->dwArgumentsCount * sizeof(LPWSTR);
//
//        lpUnicodeString = (LPWSTR)((PBYTE)ControlPacket +
//                                   ControlPacket->dwArgumentsOffset +
//                                   dwVectorSize);
//        dwUnicodeSize = (ControlPacket->dwSize -
//                         ControlPacket->dwArgumentsOffset -
//                         dwVectorSize) / sizeof(WCHAR);
//
//        dwAnsiSize = WideCharToMultiByte(CP_ACP,
//                                         0,
//                                         lpUnicodeString,
//                                         dwUnicodeSize,
//                                         NULL,
//                                         0,
//                                         NULL,
//                                         NULL);
//
//        lpVector = HeapAlloc(GetProcessHeap(),
//                             HEAP_ZERO_MEMORY,
//                             dwVectorSize + dwAnsiSize);
//        if (lpVector == NULL)
//            return ERROR_OUTOFMEMORY;
//
//        lpPtr = (LPSTR*)lpVector;
//        lpAnsiString = (LPSTR)((ULONG_PTR)lpVector + dwVectorSize);
//
//        WideCharToMultiByte(CP_ACP,
//                            0,
//                            lpUnicodeString,
//                            dwUnicodeSize,
//                            lpAnsiString,
//                            dwAnsiSize,
//                            NULL,
//                            NULL);
//
//        for (i = 0; i < ControlPacket->dwArgumentsCount; i++)
//        {
//            *lpPtr = lpAnsiString;
//
//            lpPtr++;
//            lpAnsiString += (strlen(lpAnsiString) + 1);
//        }
//
//        *lpArgCount = ControlPacket->dwArgumentsCount;
//        *lpArgVector = lpVector;
//    }
//
//    return ERROR_SUCCESS;
//}
//
//
//static DWORD
//ScStartService(PACTIVE_SERVICE lpService,
//               PSCM_CONTROL_PACKET ControlPacket)
//{
//    HANDLE ThreadHandle;
//    DWORD ThreadId;
//    DWORD dwError;
//
//    if (lpService == NULL || ControlPacket == NULL)
//        return ERROR_INVALID_PARAMETER;
//
//    TRACE("ScStartService() called\n");
//    TRACE("Size: %lu\n", ControlPacket->dwSize);
//    TRACE("Service: %S\n", (PWSTR)((PBYTE)ControlPacket + ControlPacket->dwServiceNameOffset));
//
//    /* Set the service status handle */
//    lpService->hServiceStatus = ControlPacket->hServiceStatus;
//
//    /* Build the arguments vector */
//    if (lpService->bUnicode == TRUE)
//    {
//        dwError = ScBuildUnicodeArgsVector(ControlPacket,
//                                           &lpService->ThreadParams.W.dwArgCount,
//                                           &lpService->ThreadParams.W.lpArgVector);
//    }
//    else
//    {
//        dwError = ScBuildAnsiArgsVector(ControlPacket,
//                                        &lpService->ThreadParams.A.dwArgCount,
//                                        &lpService->ThreadParams.A.lpArgVector);
//    }
//
//    if (dwError != ERROR_SUCCESS)
//        return dwError;
//
//    /* Invoke the services entry point and implement the command loop */
//    ThreadHandle = CreateThread(NULL,
//                                0,
//                                ScServiceMainStub,
//                                lpService,
//                                CREATE_SUSPENDED,
//                                &ThreadId);
//    if (ThreadHandle == NULL)
//    {
//        /* Free the arguments vector */
//        if (lpService->bUnicode)
//        {
//            if (lpService->ThreadParams.W.lpArgVector != NULL)
//            {
//                HeapFree(GetProcessHeap(),
//                         0,
//                         lpService->ThreadParams.W.lpArgVector);
//                lpService->ThreadParams.W.lpArgVector = NULL;
//                lpService->ThreadParams.W.dwArgCount = 0;
//            }
//        }
//        else
//        {
//            if (lpService->ThreadParams.A.lpArgVector != NULL)
//            {
//                HeapFree(GetProcessHeap(),
//                         0,
//                         lpService->ThreadParams.A.lpArgVector);
//                lpService->ThreadParams.A.lpArgVector = NULL;
//                lpService->ThreadParams.A.dwArgCount = 0;
//            }
//        }
//
//        return ERROR_SERVICE_NO_THREAD;
//    }
//
//    ResumeThread(ThreadHandle);
//    CloseHandle(ThreadHandle);
//
//    return ERROR_SUCCESS;
//}
//
//
//static DWORD
//ScControlService(PACTIVE_SERVICE lpService,
//                 PSCM_CONTROL_PACKET ControlPacket)
//{
//    if (lpService == NULL || ControlPacket == NULL)
//        return ERROR_INVALID_PARAMETER;
//
//    TRACE("ScControlService() called\n");
//    TRACE("Size: %lu\n", ControlPacket->dwSize);
//    TRACE("Service: %S\n", (PWSTR)((PBYTE)ControlPacket + ControlPacket->dwServiceNameOffset));
//
//    if (lpService->HandlerFunction)
//    {
//        (lpService->HandlerFunction)(ControlPacket->dwControl);
//    }
//    else if (lpService->HandlerFunctionEx)
//    {
//        /* FIXME: send correct params */
//        (lpService->HandlerFunctionEx)(ControlPacket->dwControl, 0, NULL, NULL);
//    }
//
//    TRACE("ScControlService() done\n");
//
//    return ERROR_SUCCESS;
//}
//
//
//static BOOL
//ScServiceDispatcher(HANDLE hPipe,
//                    PSCM_CONTROL_PACKET ControlPacket,
//                    DWORD dwBufferSize)
//{
//    DWORD Count;
//    BOOL bResult;
//    DWORD dwRunningServices = 0;
//    LPWSTR lpServiceName;
//    PACTIVE_SERVICE lpService;
//    SCM_REPLY_PACKET ReplyPacket;
//    DWORD dwError;
//
//    TRACE("ScDispatcherLoop() called\n");
//
//    if (ControlPacket == NULL || dwBufferSize < sizeof(SCM_CONTROL_PACKET))
//        return FALSE;
//
//    while (TRUE)
//    {
//        /* Read command from the control pipe */
//        bResult = ReadFile(hPipe,
//                           ControlPacket,
//                           dwBufferSize,
//                           &Count,
//                           NULL);
//        if (bResult == FALSE)
//        {
//            ERR("Pipe read failed (Error: %lu)\n", GetLastError());
//            return FALSE;
//        }
//
//        lpServiceName = (LPWSTR)((PBYTE)ControlPacket + ControlPacket->dwServiceNameOffset);
//        TRACE("Service: %S\n", lpServiceName);
//
//        if (ControlPacket->dwControl == SERVICE_CONTROL_START_OWN)
//            lpActiveServices[0].bOwnProcess = TRUE;
//
//        lpService = ScLookupServiceByServiceName(lpServiceName);
//        if (lpService != NULL)
//        {
//            /* Execute command */
//            switch (ControlPacket->dwControl)
//            {
//                case SERVICE_CONTROL_START_SHARE:
//                case SERVICE_CONTROL_START_OWN:
//                    TRACE("Start command - recieved SERVICE_CONTROL_START\n");
//                    dwError = ScStartService(lpService, ControlPacket);
//                    if (dwError == ERROR_SUCCESS)
//                        dwRunningServices++;
//                    break;
//
//                case SERVICE_CONTROL_STOP:
//                    TRACE("Stop command - recieved SERVICE_CONTROL_STOP\n");
//                    dwError = ScControlService(lpService, ControlPacket);
//                    if (dwError == ERROR_SUCCESS)
//                        dwRunningServices--;
//                    break;
//
//                default:
//                    TRACE("Command %lu received", ControlPacket->dwControl);
//                    dwError = ScControlService(lpService, ControlPacket);
//                    break;
//            }
//        }
//        else
//        {
//            dwError = ERROR_SERVICE_DOES_NOT_EXIST;
//        }
//
//        ReplyPacket.dwError = dwError;
//
//        /* Send the reply packet */
//        bResult = WriteFile(hPipe,
//                            &ReplyPacket,
//                            sizeof(ReplyPacket),
//                            &Count,
//                            NULL);
//        if (bResult == FALSE)
//        {
//            ERR("Pipe write failed (Error: %lu)\n", GetLastError());
//            return FALSE;
//        }
//
//        if (dwRunningServices == 0)
//            break;
//    }
//
//    return TRUE;
//}
//
//
//
//
//
//
//
//
///**********************************************************************
// *	I_ScSetServiceBitsA
// *
// * Undocumented
// *
// * @implemented
// */
//BOOL WINAPI
//I_ScSetServiceBitsA(SERVICE_STATUS_HANDLE hServiceStatus,
//                    DWORD dwServiceBits,
//                    BOOL bSetBitsOn,
//                    BOOL bUpdateImmediately,
//                    LPSTR lpString)
//{
//    BOOL bResult;
//
//    RpcTryExcept
//    {
//        /* Call to services.exe using RPC */
//        bResult = RI_ScSetServiceBitsA((RPC_SERVICE_STATUS_HANDLE)hServiceStatus,
//                                       dwServiceBits,
//                                       bSetBitsOn,
//                                       bUpdateImmediately,
//                                       lpString);
//    }
//    RpcExcept(EXCEPTION_EXECUTE_HANDLER)
//    {
//        SetLastError(ScmRpcStatusToWinError(RpcExceptionCode()));
//        bResult = FALSE;
//    }
//    RpcEndExcept;
//
//    return bResult;
//}
//
//
///**********************************************************************
// *	I_ScSetServiceBitsW
// *
// * Undocumented
// *
// * @implemented
// */
//BOOL WINAPI
//I_ScSetServiceBitsW(SERVICE_STATUS_HANDLE hServiceStatus,
//                    DWORD dwServiceBits,
//                    BOOL bSetBitsOn,
//                    BOOL bUpdateImmediately,
//                    LPWSTR lpString)
//{
//    BOOL bResult;
//
//    RpcTryExcept
//    {
//        /* Call to services.exe using RPC */
//        bResult = RI_ScSetServiceBitsW((RPC_SERVICE_STATUS_HANDLE)hServiceStatus,
//                                       dwServiceBits,
//                                       bSetBitsOn,
//                                       bUpdateImmediately,
//                                       lpString);
//    }
//    RpcExcept(EXCEPTION_EXECUTE_HANDLER)
//    {
//        SetLastError(ScmRpcStatusToWinError(RpcExceptionCode()));
//        bResult = FALSE;
//    }
//    RpcEndExcept;
//
//    return bResult;
//}
//
//
///**********************************************************************
// *	SetServiceBits
// *
// * @implemented
// */
//BOOL WINAPI
//SetServiceBits(SERVICE_STATUS_HANDLE hServiceStatus,
//               DWORD dwServiceBits,
//               BOOL bSetBitsOn,
//               BOOL bUpdateImmediately)
//{
//    return I_ScSetServiceBitsW(hServiceStatus,
//                               dwServiceBits,
//                               bSetBitsOn,
//                               bUpdateImmediately,
//                               NULL);
//}
//
//
///**********************************************************************
// *	SetServiceStatus
// *
// * @implemented
// */
//BOOL WINAPI
//SetServiceStatus(SERVICE_STATUS_HANDLE hServiceStatus,
//                 LPSERVICE_STATUS lpServiceStatus)
//{
//    DWORD dwError;
//
//    TRACE("SetServiceStatus() called\n");
//    TRACE("hServiceStatus %lu\n", hServiceStatus);
//
//    RpcTryExcept
//    {
//        /* Call to services.exe using RPC */
//        dwError = RSetServiceStatus((RPC_SERVICE_STATUS_HANDLE)hServiceStatus,
//                                    lpServiceStatus);
//    }
//    RpcExcept(EXCEPTION_EXECUTE_HANDLER)
//    {
//        dwError = ScmRpcStatusToWinError(RpcExceptionCode());
//    }
//    RpcEndExcept;
//
//    if (dwError != ERROR_SUCCESS)
//    {
//        ERR("ScmrSetServiceStatus() failed (Error %lu)\n", dwError);
//        SetLastError(dwError);
//        return FALSE;
//    }
//
//    TRACE("SetServiceStatus() done (ret %lu)\n", dwError);
//
//    return TRUE;
//}


/* EOF */
