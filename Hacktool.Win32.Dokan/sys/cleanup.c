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


#include "dokan.h"


NTSTATUS
DokanDispatchCleanup(
	__in PDEVICE_OBJECT DeviceObject,
	__in PIRP Irp
	)

/*++

Routine Description:

	This device control dispatcher handles Cleanup IRP.

Arguments:

	DeviceObject - Context for the activity.
	Irp 		 - The device control argument block.

Return Value:

	NTSTATUS

--*/
{
	PDokanVCB			vcb;
	PDEVICE_EXTENSION	deviceExtension;
	PIO_STACK_LOCATION	irpSp;
	NTSTATUS			status = STATUS_INVALID_PARAMETER;
	PFILE_OBJECT		fileObject;
	PDokanCCB			ccb = NULL;
	PDokanFCB			fcb = NULL;
	PEVENT_CONTEXT		eventContext;
	ULONG				eventLength;

	PAGED_CODE();

	__try {

		//FsRtlEnterFileSystem();

		DDbgPrint("==> DokanCleanup\n");
	
		irpSp = IoGetCurrentIrpStackLocation(Irp);
		fileObject = irpSp->FileObject;

		vcb = DokanGetVcb(DeviceObject);
		deviceExtension = DokanGetDeviceExtension(DeviceObject);

		DDbgPrint("  ProcessId %lu\n", IoGetRequestorProcessId(Irp));
		DDbgPrint("  FileName:%wZ\n", &fileObject->FileName);

		// Cleanup must be success in any case
		if (fileObject == NULL) {
			DDbgPrint("  fileObject == NULL\n");
			status = STATUS_SUCCESS;
			__leave;
		}

		if (!DokanCheckCCB(deviceExtension, fileObject->FsContext2)) {
			status = STATUS_SUCCESS;
			__leave;
		}

		ccb = fileObject->FsContext2;
		ASSERT(ccb != NULL);

		fcb = ccb->Fcb;
		ASSERT(fcb != NULL);

		eventLength = sizeof(EVENT_CONTEXT) + fcb->FileName.Length;
		eventContext = AllocateEventContext(deviceExtension, Irp, eventLength);

		if (eventContext == NULL) {
			status = STATUS_INSUFFICIENT_RESOURCES;
			__leave;
		}

		eventContext->Context = ccb->UserContext;
		//DDbgPrint("   get Context %X\n", (ULONG)ccb->UserContext);

		if (ccb->Flags & DOKAN_DELETE_ON_CLOSE) {
			eventContext->Cleanup.DeleteOnClose = TRUE;
		}

		// copy the filename to EventContext from ccb
		eventContext->Cleanup.FileNameLength = fcb->FileName.Length;
		RtlCopyMemory(eventContext->Cleanup.FileName, fcb->FileName.Buffer, fcb->FileName.Length);

		// register this IRP to pending IRP list
		status = DokanRegisterPendingIrp(DeviceObject, Irp, eventContext);

	} __finally {

		if (status != STATUS_PENDING) {
			Irp->IoStatus.Status = status;
			Irp->IoStatus.Information = 0;
			IoCompleteRequest(Irp, IO_NO_INCREMENT);
			DokanPrintNTStatus(status);
		}

		DDbgPrint("<== DokanCleanup\n");
	
		//FsRtlExitFileSystem();
	}

	return status;
}



VOID
DokanCompleteCleanup(
	 __in PIRP_ENTRY			IrpEntry,
	 __in PEVENT_INFORMATION	EventInfo
	 )
{
	PIRP				irp;
	PIO_STACK_LOCATION	irpSp;
	NTSTATUS			status   = STATUS_SUCCESS;
	ULONG				info	 = 0;
	PDokanCCB			ccb;
	PDokanFCB			fcb;
	PDokanVCB			vcb;
	PFILE_OBJECT		fileObject;

	//FsRtlEnterFileSystem();

	DDbgPrint("==> DokanCompleteCleanup\n");

	irp   = IrpEntry->Irp;
	irpSp = IrpEntry->IrpSp;

	fileObject = IrpEntry->FileObject;
	ASSERT(fileObject != NULL);

	ccb	= fileObject->FsContext2;
	ASSERT(ccb != NULL);

	ccb->UserContext = EventInfo->Context;
	//DDbgPrint("   set Context %X\n", (ULONG)ccb->UserContext);

	fcb = ccb->Fcb;
	ASSERT(fcb != NULL);

	vcb = fcb->Vcb;

	status = EventInfo->Status;

	if (fcb->Flags & DOKAN_FILE_DIRECTORY)
		FsRtlNotifyCleanup(vcb->NotifySync, &vcb->DirNotifyList, ccb);

	CcFlushCache(&fcb->SectionObjectPointers, NULL, 0, NULL);
	CcPurgeCacheSection(&fcb->SectionObjectPointers, NULL, 0, FALSE);
	CcUninitializeCacheMap(fileObject, NULL, NULL);
	fileObject->Flags |= FO_CLEANUP_COMPLETE;


	irp->IoStatus.Status = status;
	irp->IoStatus.Information = 0;
	IoCompleteRequest(irp, IO_NO_INCREMENT);

	DDbgPrint("<== DokanCompleteCleanup\n");

	//FsRtlExitFileSystem();

}


