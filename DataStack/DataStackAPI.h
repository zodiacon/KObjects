#pragma once

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
}
