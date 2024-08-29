#include <ntifs.h>
#include "DataStack.h"

DRIVER_DISPATCH OnDeviceControl;

void OnUnload(PDRIVER_OBJECT DriverObject) {
	IoDeleteDevice(DriverObject->DeviceObject);
	extern POBJECT_TYPE g_DataStackType;
	if (g_DataStackType) {
		//
		// this causes a BSOD as the kernel does not support object types being destroyed
		//
		/*
		HANDLE hType;
		auto status = ObOpenObjectByPointer(g_DataStackType, OBJ_KERNEL_HANDLE, nullptr, 0, nullptr, KernelMode, &hType);
		if (NT_SUCCESS(status)) {
			status = ZwMakeTemporaryObject(hType);
			ZwClose(hType);
		}
		*/
		ObDereferenceObject(g_DataStackType);
	}
}

extern "C" NTSTATUS
DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
	UNREFERENCED_PARAMETER(RegistryPath);

	UNICODE_STRING devName = RTL_CONSTANT_STRING(L"KDataStack");
	PDEVICE_OBJECT devObj;
	auto status = IoCreateDevice(DriverObject, 0, &devName, FILE_DEVICE_UNKNOWN, 0, FALSE, &devObj);
	if (!NT_SUCCESS(status))
		return status;

	status = DsCreateDataStackObjectType();
	if (!NT_SUCCESS(status)) {
		IoDeleteDevice(devObj);
		return status;
	}

	DriverObject->DriverUnload = OnUnload;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = DriverObject->MajorFunction[IRP_MJ_CLOSE] =
		[](PDEVICE_OBJECT, PIRP Irp) -> NTSTATUS {
		Irp->IoStatus.Status = STATUS_SUCCESS;
		IoCompleteRequest(Irp, IO_NO_INCREMENT);
		return STATUS_SUCCESS;
		};

	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = OnDeviceControl;

	return STATUS_SUCCESS;
}
