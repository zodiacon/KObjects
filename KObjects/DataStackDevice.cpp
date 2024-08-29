#include <ntifs.h>
#include "DataStackNativeAPI.h"
#include "DataStackConv.h"

NTSTATUS OnDeviceControl(PDEVICE_OBJECT, PIRP Irp) {
	auto stack = IoGetCurrentIrpStackLocation(Irp);
	auto& dic = stack->Parameters.DeviceIoControl;
	auto len = 0U;
	auto status = STATUS_INVALID_DEVICE_REQUEST;

	switch (dic.IoControlCode) {
		case IOCTL_DATASTACK_CREATE:
		{
			auto data = (DataStackCreate*)Irp->AssociatedIrp.SystemBuffer;
			if (dic.InputBufferLength < sizeof(*data)) {
				Irp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
				break;
			}
			HANDLE hDataStack;
			status = NtCreateDataStack(&hDataStack, data->ObjectAttributes, data->MaxItemSize, data->MaxItemCount, data->MaxSize);
			if (NT_SUCCESS(status)) {
				len = IoIs32bitProcess(Irp) ? sizeof(ULONG) : sizeof(HANDLE);
				memcpy(data, &hDataStack, len);
			}
			break;
		}

		case IOCTL_DATASTACK_OPEN:
		{
			auto data = (DataStackOpen*)Irp->AssociatedIrp.SystemBuffer;
			if (dic.InputBufferLength < sizeof(*data)) {
				Irp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
				break;
			}
			HANDLE hDataStack;
			status = NtOpenDataStack(&hDataStack, data->DesiredAccess, data->ObjectAttributes);
			if (NT_SUCCESS(status)) {
				len = IoIs32bitProcess(Irp) ? sizeof(ULONG) : sizeof(HANDLE);
				memcpy(data, &hDataStack, len);
			}
			break;
		}
	}

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = len;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}
