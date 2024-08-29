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
