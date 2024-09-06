#include "pch.h"
#include "DataStackAPI.h"
#include "..\KObjects\DataStackNativeAPI.h"

#define DIRECTORY_QUERY 0x0001
#define DIRECTORY_TRAVERSE 0x0002
#define DIRECTORY_CREATE_OBJECT 0x0004
#define DIRECTORY_CREATE_SUBDIRECTORY 0x0008
#define DIRECTORY_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | 0xf)

extern "C" NTSTATUS NTAPI NtOpenDirectoryObject(
	_Out_ PHANDLE DirectoryHandle,
	_In_ ACCESS_MASK DesiredAccess,
	_In_ POBJECT_ATTRIBUTES ObjectAttributes);

HANDLE GetUserDirectoryRoot() {
	static HANDLE hDir;
	if (hDir)
		return hDir;

	DWORD session = 0;
	ProcessIdToSessionId(GetCurrentProcessId(), &session);

	UNICODE_STRING name;
	WCHAR path[256];
	if (session == 0)
		RtlInitUnicodeString(&name, L"\\BaseNamedObjects");
	else {
		wsprintfW(path, L"\\Sessions\\%u\\BaseNamedObjects", session);
		RtlInitUnicodeString(&name, path);
	}
	OBJECT_ATTRIBUTES dirAttr;
	InitializeObjectAttributes(&dirAttr, &name, OBJ_CASE_INSENSITIVE, nullptr, nullptr);
	NtOpenDirectoryObject(&hDir, DIRECTORY_QUERY, &dirAttr);
	return hDir;
}

HANDLE WINAPI CreateDataStack(_In_opt_ SECURITY_ATTRIBUTES* sa, _In_ ULONG maxItemSize, _In_ ULONG maxItemCount, _In_ ULONG_PTR maxSize, _In_opt_ PCWSTR name) {
	if (BOOL wow; IsWow64Process(GetCurrentProcess(), &wow) && wow) {
		SetLastError(ERROR_NOT_SUPPORTED);
		return nullptr;
	}

	UNICODE_STRING uname{};
	if (name && *name) {
		RtlInitUnicodeString(&uname, name);
	}
	OBJECT_ATTRIBUTES attr;
	InitializeObjectAttributes(&attr,
		uname.Length ? &uname : nullptr,
		(sa && sa->bInheritHandle ? OBJ_INHERIT : 0) | (uname.Length ? (OBJ_OPENIF | OBJ_CASE_INSENSITIVE) : 0),
		uname.Length ? GetUserDirectoryRoot() : nullptr,
		sa ? sa->lpSecurityDescriptor : nullptr);

	HANDLE hDataStack;
	auto status = NtCreateDataStack(&hDataStack, &attr, maxItemSize, maxItemCount, maxSize);
	if (NT_SUCCESS(status)) {
		const NTSTATUS STATUS_OBJECT_NAME_EXISTS = 0x40000000;

		if (status == STATUS_OBJECT_NAME_EXISTS) {
			SetLastError(ERROR_ALREADY_EXISTS);
		}
		else {
			SetLastError(0);
		}
		return hDataStack;
	}

	SetLastError(RtlNtStatusToDosError(status));
	return nullptr;
}

HANDLE WINAPI OpenDataStack(_In_ ACCESS_MASK desiredAccess, _In_ BOOL inheritHandle, _In_ PCWSTR name) {
	if (BOOL wow; IsWow64Process(GetCurrentProcess(), &wow) && wow) {
		SetLastError(ERROR_NOT_SUPPORTED);
		return nullptr;
	}

	if (name == nullptr || *name == 0) {
		SetLastError(ERROR_INVALID_NAME);
		return nullptr;
	}

	UNICODE_STRING uname;
	RtlInitUnicodeString(&uname, name);
	OBJECT_ATTRIBUTES attr;
	InitializeObjectAttributes(&attr,
		&uname,
		OBJ_CASE_INSENSITIVE | (inheritHandle ? OBJ_INHERIT : 0),
		GetUserDirectoryRoot(),
		nullptr);
	HANDLE hDataStack;
	auto status = NtOpenDataStack(&hDataStack, desiredAccess, &attr);
	if (NT_SUCCESS(status))
		return hDataStack;

	SetLastError(RtlNtStatusToDosError(status));
	return nullptr;
}

_Use_decl_annotations_
BOOL WINAPI PushDataStack(HANDLE hDataStack, const PVOID buffer, DWORD size) {
	auto status = NtPushDataStack(hDataStack, buffer, size);
	if (!NT_SUCCESS(status))
		SetLastError(RtlNtStatusToDosError(status));

	return NT_SUCCESS(status);
}

_Use_decl_annotations_
BOOL WINAPI PopDataStack(HANDLE hDataStack, PVOID buffer, DWORD* size) {
	auto status = NtPopDataStack(hDataStack, buffer, size);
	if (!NT_SUCCESS(status))
		SetLastError(RtlNtStatusToDosError(status));

	return NT_SUCCESS(status);
}

_Use_decl_annotations_
BOOL WINAPI ClearDataStack(HANDLE hDataStack) {
	auto status = NtClearDataStack(hDataStack);
	if (!NT_SUCCESS(status))
		SetLastError(RtlNtStatusToDosError(status));

	return NT_SUCCESS(status);
}

_Use_decl_annotations_
BOOL WINAPI GetDataStackSize(HANDLE hDataStack, ULONG_PTR* pSize) {
	auto status = NtQueryInformationDataStack(hDataStack, DataStackTotalSize, pSize, sizeof(ULONG_PTR), nullptr);
	if (!NT_SUCCESS(status))
		SetLastError(RtlNtStatusToDosError(status));

	return NT_SUCCESS(status);
}

_Use_decl_annotations_
BOOL WINAPI GetDataStackItemCount(HANDLE hDataStack, ULONG* pCount) {
	auto status = NtQueryInformationDataStack(hDataStack, DataStackItemCount, pCount, sizeof(ULONG), nullptr);
	if (!NT_SUCCESS(status))
		SetLastError(RtlNtStatusToDosError(status));

	return NT_SUCCESS(status);
}

_Use_decl_annotations_
BOOL WINAPI GetDataStackConfig(HANDLE hDataStack, DATA_STACK_CONFIG* pConfig) {
	auto status = NtQueryInformationDataStack(hDataStack, DataStackConfiguration, pConfig, sizeof(DATA_STACK_CONFIG), nullptr);
	if (!NT_SUCCESS(status))
		SetLastError(RtlNtStatusToDosError(status));

	return NT_SUCCESS(status);
}
