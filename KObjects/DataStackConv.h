#pragma once

struct DataStackCreate {
	POBJECT_ATTRIBUTES ObjectAttributes;
	ULONG MaxItemSize;
	ULONG MaxItemCount;
	ULONG_PTR MaxSize;
};

struct DataStackOpen {
	ACCESS_MASK DesiredAccess;
	POBJECT_ATTRIBUTES ObjectAttributes;
};

struct DataStackClear {
	HANDLE DataStackHandle;
};

struct DataStackPush {
	HANDLE DataStackHandle;
	PVOID Buffer;
	ULONG Size;
};

struct DataStackPop {
	HANDLE DataStackHandle;
	PVOID Buffer;
	PULONG Size;
};
