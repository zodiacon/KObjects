#pragma once

struct DataStack {
	LIST_ENTRY Head;
	FAST_MUTEX Lock;
	ULONG Count;
	ULONG MaxItemCount;
	ULONG_PTR Size;
	ULONG MaxItemSize;
	ULONG_PTR MaxSize;
};

struct DataBlock {
	LIST_ENTRY Link;
	UCHAR Data[1];
};

NTSTATUS DsCreateDataStackObjectType();

