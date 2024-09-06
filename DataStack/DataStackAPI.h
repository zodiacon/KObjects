#pragma once

typedef struct _DATA_STACK_CONFIG {
	ULONG MaxItemSize;
	ULONG MaxItemCount;
	ULONG_PTR MaxSize;
} DATA_STACK_CONFIG;

extern "C" {
	HANDLE WINAPI CreateDataStack(
		_In_opt_ SECURITY_ATTRIBUTES* sa,
		_In_ ULONG maxItemSize,
		_In_ ULONG maxItemCount,
		_In_ ULONG_PTR maxSize,
		_In_opt_ PCWSTR name);

	HANDLE WINAPI OpenDataStack(
		_In_ ACCESS_MASK desiredAccess,
		_In_ BOOL inheritHandle,
		_In_ PCWSTR name);

	BOOL WINAPI PushDataStack(_In_ HANDLE hDataStack, _In_ const PVOID buffer, _In_ DWORD size);
	BOOL WINAPI PopDataStack(_In_ HANDLE hDataStack, _Out_ PVOID buffer, _Inout_ DWORD* size);
	BOOL WINAPI ClearDataStack(_In_ HANDLE hDataStack);

	BOOL WINAPI GetDataStackItemCount(_In_ HANDLE hDataStack, _Out_ ULONG* pCount);
	BOOL WINAPI GetDataStackSize(_In_ HANDLE hDataStack, _Out_ ULONG_PTR* pSize);
	BOOL WINAPI GetDataStackConfig(_In_ HANDLE hDataStack, _Out_ DATA_STACK_CONFIG* pConfig);
}
