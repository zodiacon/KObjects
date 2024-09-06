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

NTSTATUS NTAPI NtPushDataStack(_In_ HANDLE DataStackHandle, _In_ const PVOID Item, _In_ ULONG ItemSize) {
	DataStackPush data;
	data.DataStackHandle = DataStackHandle;
	data.Buffer = Item;
	data.Size = ItemSize;

	IO_STATUS_BLOCK ioStatus;
	return NtDeviceIoControlFile(g_hDevice, nullptr, nullptr, nullptr, &ioStatus,
		IOCTL_DATASTACK_PUSH, &data, sizeof(data), nullptr, 0);
}

NTSTATUS NTAPI NtPopDataStack(_In_ HANDLE DataStackHandle, _In_ PVOID Buffer, _Inout_ PULONG ItemSize) {
	DataStackPop data;
	data.DataStackHandle = DataStackHandle;
	data.Buffer = Buffer;
	data.Size = ItemSize;

	IO_STATUS_BLOCK ioStatus;
	return NtDeviceIoControlFile(g_hDevice, nullptr, nullptr, nullptr, &ioStatus,
		IOCTL_DATASTACK_POP, &data, sizeof(data), nullptr, 0);
}

NTSTATUS NTAPI NtClearDataStack(_In_ HANDLE DataStackHandle) {
	IO_STATUS_BLOCK ioStatus;
	return NtDeviceIoControlFile(g_hDevice, nullptr, nullptr, nullptr, &ioStatus,
		IOCTL_DATASTACK_CLEAR, &DataStackHandle, sizeof(HANDLE), nullptr, 0);
}

NTSTATUS NTAPI NtQueryInformationDataStack(_In_ HANDLE DataStackHandle, _In_ DataStackInformationClass InformationClass,
	_Out_ PVOID Buffer, _In_ ULONG BufferSize, _Out_opt_ PULONG ReturnLength) {
	DataStackQuery data;
	data.DataStackHandle = DataStackHandle;
	data.Buffer = Buffer;
	data.InfoClass = InformationClass;
	data.BufferSize = BufferSize;
	data.ReturnLength = ReturnLength;

	IO_STATUS_BLOCK ioStatus;
	return NtDeviceIoControlFile(g_hDevice, nullptr, nullptr, nullptr, &ioStatus,
		IOCTL_DATASTACK_QUERY, &data, sizeof(data), nullptr, 0);
}
