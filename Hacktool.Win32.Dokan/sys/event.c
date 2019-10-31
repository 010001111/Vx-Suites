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


/*
 Control Flow

 loop {
	=> EventIRP 
	    When IRP for Event from user-mode dokan library comes,
		 put it in EventQueue                   # DokanRegisterPendingIrpForEvent
	<= STATUS_PENDING (EventIRP)

	=> QueryDirectoryIRP                        # from individual dispatch routine
		put it in IrpQueue                      # DokanRegisterPendingIrp
		by use one of EventQueue
		 inform it to user-mode that QueryDirectory is requested
		                                        # DokanEventNotification
		<= Complete EventIRP
	<= STATUS_PENDING (QueryDirectoryIRP)

	=> EventIRP with QueryDirectoryInfo         # DokanCompleteIrp
		When user-mode file system returns EventInfo for QueryDirectory
		  search corresponding IRP from IrpQueue
		  copy QueryDirectoryInfo to that IRP
		<= Complete QueryDirectoryIRP
	<= STATUS_SUCCESS (EventIRP with QueryDirectoryInfo)
 }
    

  * Caution *
	use lock while Queue operations
	register CancelRoutine
	needs to regiter CleanupRoutine

*/



VOID
DokanIrpCancelRoutine(
    __in PDEVICE_OBJECT   DeviceObject,
    __in PIRP             Irp
    )
{
	PDokanVCB			vcb;
    PDEVICE_EXTENSION   deviceExtension;
    KIRQL               oldIrql;
    PIRP_ENTRY			irpEntry;
	ULONG				serialNumber;
	PIO_STACK_LOCATION	irpSp;

    DDbgPrint("==> DokanIrpCancelRoutine\n");

	vcb = DokanGetVcb(DeviceObject);
	deviceExtension = DokanGetDeviceExtension(DeviceObject);

    // Release the cancel spinlock
    IoReleaseCancelSpinLock(Irp->CancelIrql);

    irpEntry = Irp->Tail.Overlay.DriverContext[DRIVER_CONTEXT_IRP_ENTRY];
    
	if (irpEntry != NULL) {
		PKSPIN_LOCK	lock = &irpEntry->IrpList->ListLock;

		// Acquire the queue spinlock
		ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
		KeAcquireSpinLock(lock, &oldIrql);

		irpSp = IoGetCurrentIrpStackLocation(Irp);
		ASSERT(irpSp != NULL);

		serialNumber = irpEntry->SerialNumber;

		RemoveEntryList(&irpEntry->ListEntry);

		// If Write is canceld before completion and buffer that saves writing
		// content is not freed, free it here
		if (irpSp->MajorFunction == IRP_MJ_WRITE) {
			PVOID eventContext = Irp->Tail.Overlay.DriverContext[DRIVER_CONTEXT_EVENT];
			if (eventContext != NULL) {
				DokanFreeEventContext(eventContext);
			}
			Irp->Tail.Overlay.DriverContext[DRIVER_CONTEXT_EVENT] = NULL;
		}


		if (IsListEmpty(&irpEntry->IrpList->ListHead)) {
			//DDbgPrint("    list is empty ClearEvent\n");
			KeClearEvent(&irpEntry->IrpList->NotEmpty);
		}

		irpEntry->Irp = NULL;

		if (irpEntry->CancelRoutineFreeMemory == FALSE) {
			InitializeListHead(&irpEntry->ListEntry);
		} else {
			ExFreePool(irpEntry);
			irpEntry = NULL;
		}

		Irp->Tail.Overlay.DriverContext[DRIVER_CONTEXT_IRP_ENTRY] = NULL; 

		KeReleaseSpinLock(lock, oldIrql);
	}

	DDbgPrint("   canceled IRP #%X\n", serialNumber);
    Irp->IoStatus.Status = STATUS_CANCELLED;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

	DDbgPrint("<== DokanIrpCancelRoutine\n");
    return;

}


NTSTATUS
RegisterPendingIrpMain(
    __in PDEVICE_OBJECT DeviceObject,
    __in PIRP			Irp,
	__in ULONG			SerialNumber,
	__in PIRP_LIST		IrpList,
	__in ULONG			CheckMount
    )
{
	PDokanVCB			vcb;
    PDEVICE_EXTENSION   deviceExtension;
 	PIRP_ENTRY			irpEntry;
    PIO_STACK_LOCATION	irpSp;
    KIRQL				oldIrql;
 
	//DDbgPrint("==> DokanRegisterPendingIrpMain\n");

	vcb = DokanGetVcb(DeviceObject);
	deviceExtension = DokanGetDeviceExtension(DeviceObject);

	//ExAcquireResourceSharedLite(&deviceExtension->Resource, TRUE);
	if (CheckMount && !deviceExtension->Mounted) {
		DDbgPrint(" device is not mounted\n");
		//ExReleaseResourceLite(&deviceExtension->Resource);
		return STATUS_INSUFFICIENT_RESOURCES;
	}
	//ExReleaseResourceLite(&deviceExtension->Resource);

    irpSp = IoGetCurrentIrpStackLocation(Irp);
 
    // Allocate a record and save all the event context.
    irpEntry = ExAllocatePool(sizeof(IRP_ENTRY));

    if (NULL == irpEntry) {
        return  STATUS_INSUFFICIENT_RESOURCES;
    }

	RtlZeroMemory(irpEntry, sizeof(IRP_ENTRY));

    InitializeListHead(&irpEntry->ListEntry);

	irpEntry->SerialNumber		= SerialNumber;
    irpEntry->FileObject		= irpSp->FileObject;
    irpEntry->DeviceExtension	= deviceExtension;
    irpEntry->Irp				= Irp;
	irpEntry->IrpSp				= irpSp;
	irpEntry->IrpList			= IrpList;

	KeQueryTickCount(&irpEntry->TickCount);

	//DDbgPrint("  Lock IrpList.ListLock\n");
	ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    KeAcquireSpinLock(&IrpList->ListLock, &oldIrql);

    IoSetCancelRoutine(Irp, DokanIrpCancelRoutine);

    if (Irp->Cancel) {
        if (IoSetCancelRoutine(Irp, NULL) != NULL) {
			//DDbgPrint("  Release IrpList.ListLock %d\n", __LINE__);
            KeReleaseSpinLock(&IrpList->ListLock, oldIrql);

            ExFreePool(irpEntry);

            return STATUS_CANCELLED;
        }
	}

    IoMarkIrpPending(Irp);

    InsertTailList(&IrpList->ListHead, &irpEntry->ListEntry);

    irpEntry->CancelRoutineFreeMemory = FALSE;

	// save the pointer in order to be accessed by cancel routine
	Irp->Tail.Overlay.DriverContext[DRIVER_CONTEXT_IRP_ENTRY] =  irpEntry;


	KeSetEvent(&IrpList->NotEmpty, IO_NO_INCREMENT, FALSE);

	//DDbgPrint("  Release IrpList.ListLock\n");
    KeReleaseSpinLock(&IrpList->ListLock, oldIrql);

	//DDbgPrint("<== DokanRegisterPendingIrpMain\n");
    return STATUS_PENDING;;

}


NTSTATUS
DokanRegisterPendingIrp(
    __in PDEVICE_OBJECT DeviceObject,
    __in PIRP			Irp,
	__in PEVENT_CONTEXT	EventContext
    )
{
	PDEVICE_EXTENSION deviceExtension = DokanGetDeviceExtension(DeviceObject);

	NTSTATUS status = RegisterPendingIrpMain(
		DeviceObject,
		Irp,
		EventContext->SerialNumber,
		&deviceExtension->PendingIrp,
		TRUE);

	if (status == STATUS_PENDING) {
		DokanEventNotification(&deviceExtension->NotifyEvent, EventContext);
	} else {
		DokanFreeEventContext(EventContext);
	}
	return status;
}



NTSTATUS
DokanRegisterPendingIrpForEvent(
    __in PDEVICE_OBJECT DeviceObject,
    __in PIRP			Irp
    )
{
	PDEVICE_EXTENSION deviceExtension = DokanGetDeviceExtension(DeviceObject);

	//DDbgPrint("DokanRegisterPendingIrpForEvent\n");

	return RegisterPendingIrpMain(
		DeviceObject,
		Irp,
		0, // SerialNumber
		&deviceExtension->PendingEvent,
		TRUE);
}



NTSTATUS
DokanRegisterPendingIrpForService(
	__in PDEVICE_OBJECT	DeviceObject,
	__in PIRP			Irp
	)
{
	PDEVICE_EXTENSION deviceExtension = DokanGetDeviceExtension(DeviceObject);

	DDbgPrint("DokanRegisterPendingIrpForService\n");

	return RegisterPendingIrpMain(
		DeviceObject,
		Irp,
		0, // SerialNumber
		&deviceExtension->Global->PendingService,
		FALSE);
}


// When user-mode file system application returns EventInformation,
// search corresponding pending IRP and complete it
NTSTATUS
DokanCompleteIrp(
    __in PDEVICE_OBJECT DeviceObject,
    __in PIRP Irp
	)
{
	KIRQL				oldIrql;
    PLIST_ENTRY			thisEntry, nextEntry, listHead;
	PIRP_ENTRY			irpEntry;
	PDokanVCB			vcb;
    PDEVICE_EXTENSION   deviceExtension;
	PEVENT_INFORMATION	eventInfo;

	eventInfo		= (PEVENT_INFORMATION)Irp->AssociatedIrp.SystemBuffer;
	ASSERT(eventInfo != NULL);
	
	//DDbgPrint("==> DokanCompleteIrp [EventInfo #%X]\n", eventInfo->SerialNumber);

	vcb = DokanGetVcb(DeviceObject);
	deviceExtension = DokanGetDeviceExtension(DeviceObject);

	//DDbgPrint("      Lock IrpList.ListLock\n");
	ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
	KeAcquireSpinLock(&deviceExtension->PendingIrp.ListLock, &oldIrql);

	// search corresponding IRP through pending IRP list
	listHead = &deviceExtension->PendingIrp.ListHead;

    for (thisEntry = listHead->Flink; thisEntry != listHead; thisEntry = nextEntry) {

		PIRP				irp;
		PIO_STACK_LOCATION	irpSp;

        nextEntry = thisEntry->Flink;

        irpEntry = CONTAINING_RECORD(thisEntry, IRP_ENTRY, ListEntry);

		// check whether this is corresponding IRP

        //DDbgPrint("SerialNumber irpEntry %X eventInfo %X\n", irpEntry->SerialNumber, eventInfo->SerialNumber);

		// this irpEntry must be freed in this if statement
		if (irpEntry->SerialNumber != eventInfo->SerialNumber)  {
			continue;
		}

		RemoveEntryList(thisEntry);

		irp = irpEntry->Irp;
	
		if (irp == NULL) {
			// this IRP is already canceled
			ASSERT(irpEntry->CancelRoutineFreeMemory == FALSE);
			ExFreePool(irpEntry);
			irpEntry = NULL;
			break;
		}

		if (IoSetCancelRoutine(irp, NULL) == NULL) {
			// Cancel routine will run as soon as we release the lock
			InitializeListHead(&irpEntry->ListEntry);
			irpEntry->CancelRoutineFreeMemory = TRUE;
			break;
		}

		// IRP is not canceled yet
		irpSp = irpEntry->IrpSp;	
		
		ASSERT(irpSp != NULL);
					
		// IrpEntry is saved here for CancelRoutine
		// Clear it to prevent to be completed by CancelRoutine twice
		irp->Tail.Overlay.DriverContext[DRIVER_CONTEXT_IRP_ENTRY] = NULL;
		KeReleaseSpinLock(&deviceExtension->PendingIrp.ListLock, oldIrql);

		switch (irpSp->MajorFunction) {
		case IRP_MJ_DIRECTORY_CONTROL:
			DokanCompleteDirectoryControl(irpEntry, eventInfo);
			break;
		case IRP_MJ_READ:
			DokanCompleteRead(irpEntry, eventInfo);
			break;
		case IRP_MJ_WRITE:
			DokanCompleteWrite(irpEntry, eventInfo);
			break;
		case IRP_MJ_QUERY_INFORMATION:
			DokanCompleteQueryInformation(irpEntry, eventInfo);
			break;
		case IRP_MJ_QUERY_VOLUME_INFORMATION:
			DokanCompleteQueryVolumeInformation(irpEntry, eventInfo);
			break;
		case IRP_MJ_CREATE:
			DokanCompleteCreate(irpEntry, eventInfo);
			break;
		case IRP_MJ_CLEANUP:
			DokanCompleteCleanup(irpEntry, eventInfo);
			break;
		case IRP_MJ_LOCK_CONTROL:
			DokanCompleteLock(irpEntry, eventInfo);
			break;
		case IRP_MJ_SET_INFORMATION:
			DokanCompleteSetInformation(irpEntry, eventInfo);
			break;
		case IRP_MJ_FLUSH_BUFFERS:
			DokanCompleteFlush(irpEntry, eventInfo);
			break;
		default:
			DDbgPrint("Unknown IRP %d\n", irpSp->MajorFunction);
			// TODO: in this case, should complete this IRP
			break;
		}		

		ExFreePool(irpEntry);
		irpEntry = NULL;

		return STATUS_SUCCESS;
	}

	KeReleaseSpinLock(&deviceExtension->PendingIrp.ListLock, oldIrql);

    //DDbgPrint("<== AACompleteIrp [EventInfo #%X]\n", eventInfo->SerialNumber);

	// TODO: should return error
    return STATUS_SUCCESS;
}


 
// start event dispatching
NTSTATUS
DokanEventStart(
    __in PDEVICE_OBJECT DeviceObject,
    __in PIRP Irp
   )
{
	PDokanVCB			vcb;
    PDEVICE_EXTENSION   deviceExtension;
	ULONG				bufferLen;
	ULONG				inBufferLen;
	PVOID				buffer;
	PIO_STACK_LOCATION	irpSp;
	PEVENT_START		eventStart;
	WCHAR				driveLetter;

	DDbgPrint("==> DokanEventStart\n");

	vcb = DokanGetVcb(DeviceObject);
	deviceExtension = DokanGetDeviceExtension(DeviceObject);

	irpSp		= IoGetCurrentIrpStackLocation(Irp);

	bufferLen = irpSp->Parameters.DeviceIoControl.OutputBufferLength;		
	inBufferLen = irpSp->Parameters.DeviceIoControl.InputBufferLength;
	eventStart = (PEVENT_START)Irp->AssociatedIrp.SystemBuffer;

	if (bufferLen < sizeof(EVENT_START))
		return STATUS_INSUFFICIENT_RESOURCES;
	if (inBufferLen < sizeof(WCHAR))
		return STATUS_INSUFFICIENT_RESOURCES;

	
	driveLetter = *(WCHAR*)Irp->AssociatedIrp.SystemBuffer;

	eventStart->Version = DOKAN_VERSION;
	eventStart->DeviceNumber = deviceExtension->Number;

	KeEnterCriticalRegion();
	ExAcquireResourceExclusiveLite(&deviceExtension->Resource, TRUE);

	if (deviceExtension->Mounted) {
		DDbgPrint("  DOKAN_USED\n");
		eventStart->Status = DOKAN_USED;
	} else {
		DDbgPrint("  DOKAN_MOUNTED\n");
		eventStart->Status = DOKAN_MOUNTED;
		deviceExtension->Mounted = driveLetter;
		KeQueryTickCount(&deviceExtension->TickCount);
		InterlockedIncrement(&deviceExtension->MountId);
		DDbgPrint("  MountId:%d\n", deviceExtension->MountId);

		deviceExtension->UseAltStream = 0;
		deviceExtension->UseKeepAlive = 0;
		DokanStartEventNotificationThread(deviceExtension);
		DokanStartCheckThread(deviceExtension);
	}

	ExReleaseResourceLite(&deviceExtension->Resource);
	KeLeaveCriticalRegion();

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = sizeof(EVENT_START);

	DDbgPrint("<== DokanEventStart\n");

	return Irp->IoStatus.Status;
}




// user assinged bigger buffer that is enough to return WriteEventContext
NTSTATUS
DokanEventWrite(
    __in PDEVICE_OBJECT DeviceObject,
    __in PIRP Irp
	)
{
	KIRQL				oldIrql;
    PLIST_ENTRY			thisEntry, nextEntry, listHead;
	PIRP_ENTRY			irpEntry;
	PDokanVCB			vcb;
    PDEVICE_EXTENSION   deviceExtension;
	PEVENT_INFORMATION	eventInfo;
	PIRP				writeIrp;

	eventInfo		= (PEVENT_INFORMATION)Irp->AssociatedIrp.SystemBuffer;
	ASSERT(eventInfo != NULL);
	
	DDbgPrint("==> DokanEventWrite [EventInfo #%X]\n", eventInfo->SerialNumber);

	vcb = DokanGetVcb(DeviceObject);
	deviceExtension = DokanGetDeviceExtension(DeviceObject);

	ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
	KeAcquireSpinLock(&deviceExtension->PendingIrp.ListLock, &oldIrql);

	// search corresponding write IRP through pending IRP list
	listHead = &deviceExtension->PendingIrp.ListHead;

    for (thisEntry = listHead->Flink; thisEntry != listHead; thisEntry = nextEntry) {

		PIO_STACK_LOCATION writeIrpSp, eventIrpSp;
		PEVENT_CONTEXT	eventContext;
		ULONG			info = 0;
		NTSTATUS		status;

		nextEntry = thisEntry->Flink;
        irpEntry = CONTAINING_RECORD(thisEntry, IRP_ENTRY, ListEntry);

		// check whehter this is corresponding IRP

        //DDbgPrint("SerialNumber irpEntry %X eventInfo %X\n", irpEntry->SerialNumber, eventInfo->SerialNumber);

		if (irpEntry->SerialNumber != eventInfo->SerialNumber)  {
			continue;
		}

		// do NOT free irpEntry here
		writeIrp = irpEntry->Irp;
		if (writeIrp == NULL) {
			// this IRP has already been canceled
			ASSERT(irpEntry->CancelRoutineFreeMemory == FALSE);
			ExFreePool(irpEntry);
			continue;
		}

		if (IoSetCancelRoutine(writeIrp, DokanIrpCancelRoutine) == NULL) {
		//if (IoSetCancelRoutine(writeIrp, NULL) != NULL) {
			// Cancel routine will run as soon as we release the lock
			InitializeListHead(&irpEntry->ListEntry);
			irpEntry->CancelRoutineFreeMemory = TRUE;
			continue;
		}

		writeIrpSp = irpEntry->IrpSp;
		eventIrpSp = IoGetCurrentIrpStackLocation(Irp);
			
		ASSERT(writeIrpSp != NULL);
		ASSERT(eventIrpSp != NULL);

		eventContext = (PEVENT_CONTEXT)writeIrp->Tail.Overlay.DriverContext[DRIVER_CONTEXT_EVENT];
		ASSERT(eventContext != NULL);
				
		// short of buffer length
		if (eventIrpSp->Parameters.DeviceIoControl.OutputBufferLength
			< eventContext->Length) {		
			DDbgPrint("  EventWrite: STATUS_INSUFFICIENT_RESOURCE\n");
			status =  STATUS_INSUFFICIENT_RESOURCES;
		} else {
			PVOID buffer;
			//DDbgPrint("  EventWrite CopyMemory\n");
			//DDbgPrint("  EventLength %d, BufLength %d\n", eventContext->Length,
			//			eventIrpSp->Parameters.DeviceIoControl.OutputBufferLength);
			if (Irp->MdlAddress)
				buffer = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority);
			else
				buffer = Irp->AssociatedIrp.SystemBuffer;
					
			ASSERT(buffer != NULL);
			RtlCopyMemory(buffer, eventContext, eventContext->Length);
						
			info = eventContext->Length;
			status = STATUS_SUCCESS;
		}

		DokanFreeEventContext(eventContext);
		writeIrp->Tail.Overlay.DriverContext[DRIVER_CONTEXT_EVENT] = 0;

		KeReleaseSpinLock(&deviceExtension->PendingIrp.ListLock, oldIrql);

		Irp->IoStatus.Status = status;
		Irp->IoStatus.Information = info;

		// this IRP will be completed by caller function
		return Irp->IoStatus.Status;
	}

	KeReleaseSpinLock(&deviceExtension->PendingIrp.ListLock, oldIrql);

   return STATUS_SUCCESS;
}



