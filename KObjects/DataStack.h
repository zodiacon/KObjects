#pragma once

const ULONG DataStackTag = 'ktsD';

struct DataStack {
	KEVENT Event;
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
	ULONG Size;
	UCHAR Data[1];
};

NTSTATUS DsCreateDataStackObjectType();
void OnDataStackDelete(_In_ PVOID Object);

NTSTATUS DsPushDataStack(DataStack* ds, PVOID Item, ULONG ItemSize);
NTSTATUS DsPopDataStack(DataStack* ds, PVOID Item, ULONG inputSize, ULONG* ItemSize);
NTSTATUS DsClearDataStack(DataStack* ds);

