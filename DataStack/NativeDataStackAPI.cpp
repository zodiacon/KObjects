#include "pch.h"
#include "..\KObjects\DataStackConv.h"
#include "..\KObjects\DataStackNativeAPI.h"

#pragma comment(lib, "ntdll")

extern HANDLE g_hDevice;

NTSTATUS NTAPI NtCreateDataStack(_Out_ PHANDLE DataStackHandle, _In_opt_ POBJECT_ATTRIBUTES DataStackAttributes, _In_ ULONG MaxItemSize, _In_ ULONG MaxItemCount, ULONG_PTR MaxSize) {
	DataStackCreate data;
	data.MaxItemCount = MaxItemCount;
	data.MaxItemSize = MaxItemSize;
	data.ObjectAttributes = DataStackAttributes;
	data.MaxSize = MaxSize;

	IO_STATUS_BLOCK ioStatus;
	return NtDeviceIoControlFile(g_hDevice, nullptr, nullptr, nullptr, &ioStatus,
		IOCTL_DATASTACK_CREATE, &data, sizeof(data), DataStackHandle, sizeof(HANDLE));
}

NTSTATUS NTAPI NtOpenDataStack(_Out_ PHANDLE DataStackHandle, _In_ ACCESS_MASK DesiredAccess, _In_ POBJECT_ATTRIBUTES DataStackAttributes) {
	DataStackOpen data;
	data.DesiredAccess = DesiredAccess;
	data.ObjectAttributes = DataStackAttributes;

	IO_STATUS_BLOCK ioStatus;
	return NtDeviceIoControlFile(g_hDevice, nullptr, nullptr, nullptr, &ioStatus,
		IOCTL_DATASTACK_OPEN, &data, sizeof(data), DataStackHandle, sizeof(HANDLE));
}
