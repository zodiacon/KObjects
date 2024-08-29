#pragma once

#define DATA_STACK_QUERY	0x1
#define DATA_STACK_PUSH		0x2
#define DATA_STACK_POP		0x4
#define DATA_STACK_CLEAR	0x8

#define DATA_STACK_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | DATA_STACK_QUERY | DATA_STACK_PUSH | DATA_STACK_POP | DATA_STACK_CLEAR)

typedef enum _DataStackInformationClass {
	DataStackItemCount,
	DataStackTotalSize,
	DataStackInitState,
} DataStackInformationClass;

extern "C" {
	NTSTATUS NTAPI NtCreateDataStack(
		_Out_ PHANDLE DataStackHandle,
		_In_opt_ POBJECT_ATTRIBUTES DataStackAttributes,
		_In_ ULONG MaxItemSize,
		_In_ ULONG MaxItemCount,
		ULONG_PTR MaxSize);
	NTSTATUS NTAPI NtOpenDataStack(
		_Out_ PHANDLE DataStackHandle,
		_In_ ACCESS_MASK DesiredAccess,
		_In_ POBJECT_ATTRIBUTES DataStackAttributes);
	NTSTATUS NTAPI NtQueryInformationDataStack(
		_In_ HANDLE DataStackHandle,
		_In_ DataStackInformationClass InformationClass,
		_Out_ PVOID Buffer,
		_In_ ULONG BufferSize,
		_Out_opt_ PULONG ReturnLength);
	NTSTATUS NTAPI NtPushDataStack(
		_In_ HANDLE DataStackHandle,
		_In_ PVOID Item,
		_In_ ULONG ItemSize);
	NTSTATUS NTAPI NtPopDataStack(
		_In_ HANDLE DataStackHandle,
		_Out_ PVOID Buffer,
		_Inout_ PULONG BufferSize);
	NTSTATUS NTAPI NtClearDataStack(_In_ HANDLE DataStackHandle);

	NTSTATUS NTAPI NtPeekDataStack(_In_ HANDLE DataStackHandle, _Out_ PULONG ItemSize);
}

#define DEVICE_DATASTACK 0x8009
#define IOCTL_DATASTACK_CREATE CTL_CODE(DEVICE_DATASTACK, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DATASTACK_OPEN CTL_CODE(DEVICE_DATASTACK, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DATASTACK_PUSH CTL_CODE(DEVICE_DATASTACK, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DATASTACK_POP CTL_CODE(DEVICE_DATASTACK, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DATASTACK_CLEAR CTL_CODE(DEVICE_DATASTACK, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS)

