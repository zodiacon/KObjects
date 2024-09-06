#pragma once

#include "DataStackNativeAPI.h"

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

struct DataStackQuery {
	HANDLE DataStackHandle;
	DataStackInformationClass InfoClass;
	PVOID Buffer;
	ULONG BufferSize;
	PULONG ReturnLength;
};
