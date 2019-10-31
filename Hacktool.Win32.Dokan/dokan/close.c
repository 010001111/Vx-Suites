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


#include "dokani.h"
#include "fileinfo.h"


VOID
DispatchClose(
	HANDLE				Handle,
	PEVENT_CONTEXT		EventContext,
	PDOKAN_INSTANCE		DokanInstance)
{
	PEVENT_INFORMATION		eventInfo;
	DOKAN_FILE_INFO			fileInfo;	
	PDOKAN_OPEN_INFO		openInfo;
	ULONG					sizeOfEventInfo = sizeof(EVENT_INFORMATION);

	CheckFileName(EventContext->Close.FileName);

	eventInfo = DispatchCommon(EventContext, sizeOfEventInfo, DokanInstance, &fileInfo);

	openInfo = (PDOKAN_OPEN_INFO)EventContext->Context;

	eventInfo->Status = STATUS_SUCCESS; // return success at any case

	DbgPrint("###Close %04d\n", openInfo->EventId);

	if (DokanInstance->DokanOperations->CloseFile) {
		// ignore return value
		DokanInstance->DokanOperations->CloseFile(
			EventContext->Close.FileName, &fileInfo);
	}

	// do not send it to the driver
	//SendEventInformation(Handle, eventInfo, length);

	free(eventInfo);

	if (openInfo->DirListHead != NULL) {
		ClearFindData(openInfo->DirListHead);
		free(openInfo->DirListHead);
		openInfo->DirListHead = NULL;
	}

	free(openInfo);
	
	return;
}

