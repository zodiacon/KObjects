#include <ntifs.h>
#include "DataStackNativeAPI.h"
#include "DataStackConv.h"

NTSTATUS OnDeviceControl(PDEVICE_OBJECT, PIRP Irp) {
	auto len = 0U;
	auto status = STATUS_INVALID_DEVICE_REQUEST;

	if (IoIs32bitProcess(Irp)) {
		status = STATUS_NOT_IMPLEMENTED;
	}
	else {
		auto stack = IoGetCurrentIrpStackLocation(Irp);
		auto& dic = stack->Parameters.DeviceIoControl;

		switch (dic.IoControlCode) {
			case IOCTL_DATASTACK_CREATE:
			{
				auto data = (DataStackCreate*)Irp->AssociatedIrp.SystemBuffer;
				if (dic.InputBufferLength < sizeof(*data)) {
					status = STATUS_BUFFER_TOO_SMALL;
					break;
				}
				HANDLE hDataStack;
				status = NtCreateDataStack(&hDataStack, data->ObjectAttributes, data->MaxItemSize, data->MaxItemCount, data->MaxSize);
				if (NT_SUCCESS(status)) {
					len = sizeof(HANDLE);
					memcpy(data, &hDataStack, len);
				}
				break;
			}

			case IOCTL_DATASTACK_OPEN:
			{
				auto data = (DataStackOpen*)Irp->AssociatedIrp.SystemBuffer;
				if (dic.InputBufferLength < sizeof(*data)) {
					status = STATUS_BUFFER_TOO_SMALL;
					break;
				}
				HANDLE hDataStack;
				status = NtOpenDataStack(&hDataStack, data->DesiredAccess, data->ObjectAttributes);
				if (NT_SUCCESS(status)) {
					len = sizeof(HANDLE);
					memcpy(data, &hDataStack, len);
				}
				break;
			}

			case IOCTL_DATASTACK_CLEAR:
			{
				auto data = (DataStackClear*)Irp->AssociatedIrp.SystemBuffer;
				if (dic.InputBufferLength < sizeof(*data)) {
					status = STATUS_BUFFER_TOO_SMALL;
					break;
				}
				status = NtClearDataStack(data->DataStackHandle);
				break;
			}

			case IOCTL_DATASTACK_PUSH:
			{
				auto data = (DataStackPush*)Irp->AssociatedIrp.SystemBuffer;
				if (dic.InputBufferLength < sizeof(*data)) {
					status = STATUS_BUFFER_TOO_SMALL;
					break;
				}
				status = NtPushDataStack(data->DataStackHandle, data->Buffer, data->Size);
				break;
			}

			case IOCTL_DATASTACK_POP:
			{
				auto data = (DataStackPop*)Irp->AssociatedIrp.SystemBuffer;
				if (dic.InputBufferLength < sizeof(*data)) {
					status = STATUS_BUFFER_TOO_SMALL;
					break;
				}
				status = NtPopDataStack(data->DataStackHandle, data->Buffer, data->Size);
				break;
			}

			case IOCTL_DATASTACK_QUERY:
			{
				auto data = (DataStackQuery*)Irp->AssociatedIrp.SystemBuffer;
				if (dic.InputBufferLength < sizeof(*data)) {
					status = STATUS_BUFFER_TOO_SMALL;
					break;
				}
				status = NtQueryInformationDataStack(data->DataStackHandle, data->InfoClass, data->Buffer, data->BufferSize, data->ReturnLength);
				break;
			}

		}
	}

	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = len;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}
