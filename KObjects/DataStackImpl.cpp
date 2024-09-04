#include <ntddk.h>
#include "DataStack.h"

NTSTATUS DsPushDataStack(DataStack* ds, PVOID Item, ULONG ItemSize) {
	KdPrint(("DsPushDataStack 0x%p 0x%p 0x%X\n", ds, Item, ItemSize));

	auto buffer = (DataBlock*)ExAllocatePool2(POOL_FLAG_PAGED | POOL_FLAG_UNINITIALIZED, 
		ItemSize + sizeof(DataBlock), DataStackTag);
	if (buffer == nullptr)
		return STATUS_INSUFFICIENT_RESOURCES;

	auto status = STATUS_SUCCESS;
	if (ExGetPreviousMode() != KernelMode) {
		__try {
			ProbeForRead(Item, ItemSize, 1);
			memcpy(buffer->Data, Item, ItemSize);
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			ExFreePool(buffer);
			return GetExceptionCode();
		}
	}
	else {
		memcpy(buffer->Data, Item, ItemSize);
	}
	buffer->Size = ItemSize;

	ExAcquireFastMutex(&ds->Lock);
	do {
		if (ds->MaxItemCount == ds->Count) {
			status = STATUS_NO_MORE_ENTRIES;
			break;
		}

		if (ds->MaxItemSize && ItemSize > ds->MaxItemSize) {
			status = STATUS_NOT_CAPABLE;
			break;
		}

		if (ds->MaxSize && ds->Size + ItemSize > ds->MaxSize) {
			status = STATUS_NOT_CAPABLE;
			break;
		}
	} while (false);

	if (NT_SUCCESS(status)) {
		InsertTailList(&ds->Head, &buffer->Link);
		ds->Count++;
		ds->Size += ItemSize;
	}
	ExReleaseFastMutex(&ds->Lock);

	if (!NT_SUCCESS(status))
		ExFreePool(buffer);

	return status;
}

NTSTATUS DsPopDataStack(DataStack* ds, PVOID buffer, ULONG inputSize, ULONG* itemSize) {
	KdPrint(("DsPopDataStack 0x%p 0x%p 0x%X 0x%X\n", ds, buffer, inputSize, *itemSize));

	ExAcquireFastMutex(&ds->Lock);
	__try {
		if (inputSize == 0) {
			//
			// return size of next item
			//			
			__try {
				if (ds->Count == 0) {
					//
					// stack empty
					//
					*itemSize = 0;
				}
				else {
					auto top = CONTAINING_RECORD(ds->Head.Blink, DataBlock, Link);
					*itemSize = top->Size;
				}
				return STATUS_SUCCESS;
			}
			__except (EXCEPTION_EXECUTE_HANDLER) {
				return GetExceptionCode();
			}
		}

		if (ds->Count == 0) {
			__try {
				*itemSize = 0;
			}
			__except (EXCEPTION_EXECUTE_HANDLER) {
				return GetExceptionCode();
			}
			return STATUS_PIPE_EMPTY;
		}

		//
		// remove item
		//
		auto link = RemoveTailList(&ds->Head);
		NT_ASSERT(link != &ds->Head);
	
		auto item = CONTAINING_RECORD(link, DataBlock, Link);
		__try {
			*itemSize = item->Size;
			if (inputSize < item->Size) {
				//
				// buffer too small
				// reinsert item
				//
				InsertTailList(&ds->Head, link);
				return STATUS_BUFFER_TOO_SMALL;
			}
			else {
				memcpy(buffer, item->Data, item->Size);
				ds->Count--;
				ds->Size -= item->Size;
				ExFreePool(item);
				return STATUS_SUCCESS;
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			return GetExceptionCode();
		}
	}
	__finally {
		ExReleaseFastMutex(&ds->Lock);
	}
}

NTSTATUS DsClearDataStack(DataStack* ds) {
	KdPrint(("DsClearDataStack 0x%p\n", ds));

	ExAcquireFastMutex(&ds->Lock);
	LIST_ENTRY* link;

	while ((link = RemoveHeadList(&ds->Head)) != &ds->Head) {
		auto item = CONTAINING_RECORD(link, DataBlock, Link);
		ExFreePool(item);
	}
	ds->Count = 0;
	ds->Size = 0;
	ExReleaseFastMutex(&ds->Lock);

	return STATUS_SUCCESS;
}
