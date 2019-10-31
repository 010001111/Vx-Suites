/*
  Dokan : user-mode file system library for Windows

  Copyright (C) 2008 Hiroki Asakawa info@dokan-dev.net

  http://dokan-dev.net/en

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free
Software Foundation; either version 3 of the License, or (at your option) any
later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program. If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef _DOKANI_H_
#define _DOKANI_H_

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "public.h"
#include "dokan.h"
#include "dokanc.h"
#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif



typedef struct _DOKAN_INSTANCE
{
	// to ensure that unmount dispatch is called at once
	CRITICAL_SECTION	CriticalSection;

	// store CurrentDeviceName
	// (when there are many mounts, each mount use 
	// other DeviceName)
	WCHAR	DeviceName[MAX_PATH];
	ULONG	DeviceNumber;

	PDOKAN_OPTIONS		DokanOptions;
	PDOKAN_OPERATIONS	DokanOperations;

	LIST_ENTRY	ListEntry;

} DOKAN_INSTANCE, *PDOKAN_INSTANCE;


typedef struct _DOKAN_OPEN_INFO {

	BOOL	IsDirectory;
	ULONG64	Context;
	ULONG64	UserContext;
	ULONG	EventId;
	PLIST_ENTRY	DirListHead;
} DOKAN_OPEN_INFO, *PDOKAN_OPEN_INFO;


ULONG
DokanStart(
	WCHAR	DriveLetter);

BOOL
SendToDevice(
	PWCHAR	DeviceName,
	DWORD	IoControlCode,
	PVOID	InputBuffer,
	ULONG	InputLength,
	PVOID	OutputBuffer,
	ULONG	OutputLength,
	PULONG	ReturnedLength);


DWORD __stdcall
DokanLoop(
	PVOID Param);


BOOL
DokanMount(
	ULONG	DeviceNumber,
	WCHAR	DriveLetter);

BOOL
DokanSendIoControl(
	WCHAR	DriveLetter,
	DWORD	IoControlCode);


VOID
SendEventInformation(
	HANDLE				Handle,
	PEVENT_INFORMATION	EventInfo,
	ULONG				EventLength);


PEVENT_INFORMATION
DispatchCommon(
	PEVENT_CONTEXT		EventContext,
	ULONG				SizeOfEventInfo,
	PDOKAN_INSTANCE		DokanInstance,
	PDOKAN_FILE_INFO	DokanFileInfo);


VOID
DispatchDirectoryInformation(
	HANDLE				Handle,
	PEVENT_CONTEXT		EventContext,
	PDOKAN_INSTANCE		DokanInstance);


VOID
DispatchQueryInformation(
 	HANDLE				Handle,
	PEVENT_CONTEXT		EventContext,
	PDOKAN_INSTANCE		DokanInstance);


VOID
DispatchQueryVolumeInformation(
 	HANDLE				Handle,
	PEVENT_CONTEXT		EventContext,
	PDOKAN_INSTANCE		DokanInstance);


VOID
DispatchSetInformation(
 	HANDLE				Handle,
	PEVENT_CONTEXT		EventContext,
	PDOKAN_INSTANCE		DokanInstance);


VOID
DispatchRead(
 	HANDLE				Handle,
	PEVENT_CONTEXT		EventContext,
	PDOKAN_INSTANCE		DokanInstance);


VOID
DispatchWrite(
 	HANDLE				Handle,
	PEVENT_CONTEXT		EventContext,
	PDOKAN_INSTANCE		DokanInstance);


VOID
DispatchCreate(
	HANDLE				Handle,
	PEVENT_CONTEXT		EventContext,
	PDOKAN_INSTANCE		DokanInstance);


VOID
DispatchClose(
  	HANDLE				Handle,
	PEVENT_CONTEXT		EventContext,
	PDOKAN_INSTANCE		DokanInstance);


VOID
DispatchCleanup(
  	HANDLE				Handle,
	PEVENT_CONTEXT		EventContext,
	PDOKAN_INSTANCE		DokanInstance);


VOID
DispatchFlush(
  	HANDLE				Handle,
	PEVENT_CONTEXT		EventContext,
	PDOKAN_INSTANCE		DokanInstance);


VOID
DispatchUnmount(
  	HANDLE				Handle,
	PEVENT_CONTEXT		EventContext,
	PDOKAN_INSTANCE		DokanInstance);


VOID
DispatchLock(
	HANDLE				Handle,
	PEVENT_CONTEXT		EventContext,
	PDOKAN_INSTANCE		DokanInstance);


BOOLEAN
InstallDriver(
	SC_HANDLE  SchSCManager,
	LPCWSTR    DriverName,
	LPCWSTR    ServiceExe);


BOOLEAN
RemoveDriver(
    SC_HANDLE  SchSCManager,
    LPCWSTR    DriverName);


BOOLEAN
StartDriver(
    SC_HANDLE  SchSCManager,
    LPCWSTR    DriverName);


BOOLEAN
StopDriver(
    SC_HANDLE  SchSCManager,
    LPCWSTR    DriverName);


BOOLEAN
ManageDriver(
	LPCWSTR  DriverName,
    LPCWSTR  ServiceName,
    USHORT   Function);


BOOL
SendReleaseIRP(
	WCHAR DriveLetter);


BOOL
SendReleaseIRP2(
	ULONG DeviceNumber);


VOID
CheckFileName(
	LPWSTR	FileName);

VOID
ClearFindData(
  PLIST_ENTRY	ListHead);

DWORD WINAPI
DokanKeepAlive(
	PVOID	Param);


ULONG
GetNTStatus(DWORD ErrorCode);

#ifdef __cplusplus
}
#endif


#endif
