#include <ntifs.h>
#include "DataStack.h"
#include "DataStackNativeAPI.h"

POBJECT_TYPE g_DataStackType;

typedef struct _OBJECT_DUMP_CONTROL {
	PVOID Stream;
	ULONG Detail;
} OB_DUMP_CONTROL, * POB_DUMP_CONTROL;

typedef VOID(*OB_DUMP_METHOD)(
	_In_ PVOID Object,
	_In_opt_ POB_DUMP_CONTROL Control);

typedef enum _OB_OPEN_REASON {
	ObCreateHandle,
	ObOpenHandle,
	ObDuplicateHandle,
	ObInheritHandle,
	ObMaxOpenReason
} OB_OPEN_REASON;

typedef NTSTATUS(*OB_OPEN_METHOD)(
	_In_ OB_OPEN_REASON OpenReason,
	_In_ KPROCESSOR_MODE AccessMode,
	_In_opt_ PEPROCESS Process,
	_In_ PVOID Object,
	_Inout_ PACCESS_MASK GrantedAccess,
	_In_ ULONG HandleCount
	);

typedef BOOLEAN(*OB_OKAYTOCLOSE_METHOD)(
	_In_opt_ PEPROCESS Process,
	_In_ PVOID Object,
	_In_ HANDLE Handle,
	_In_ KPROCESSOR_MODE PreviousMode
	);

typedef VOID(*OB_CLOSE_METHOD)(
	_In_opt_ PEPROCESS Process,
	_In_ PVOID Object,
	_In_ ULONG_PTR ProcessHandleCount,
	_In_ ULONG_PTR SystemHandleCount
	);

typedef VOID(*OB_DELETE_METHOD)(
	_In_  PVOID   Object
	);

typedef NTSTATUS(*OB_SECURITY_METHOD)(
	_In_ PVOID Object,
	_In_ SECURITY_OPERATION_CODE OperationCode,
	_In_ PSECURITY_INFORMATION SecurityInformation,
	_Inout_ PSECURITY_DESCRIPTOR SecurityDescriptor,
	_Inout_ PULONG CapturedLength,
	_Inout_ PSECURITY_DESCRIPTOR* ObjectsSecurityDescriptor,
	_In_ POOL_TYPE PoolType,
	_In_ PGENERIC_MAPPING GenericMapping,
	_In_ KPROCESSOR_MODE Mode
	);

typedef NTSTATUS(*OB_QUERYNAME_METHOD)(
	_In_ PVOID Object,
	_In_ BOOLEAN HasObjectName,
	_Out_writes_bytes_opt_(Length) POBJECT_NAME_INFORMATION ObjectNameInfo,
	_In_ ULONG Length,
	_Out_ PULONG ReturnLength,
	_In_ KPROCESSOR_MODE Mode
	);

typedef NTSTATUS(*OB_PARSE_METHOD)(
	_In_ PVOID ParseObject,
	_In_ PVOID ObjectType,
	_Inout_ PACCESS_STATE AccessState,
	_In_ KPROCESSOR_MODE AccessMode,
	_In_ ULONG Attributes,
	_Inout_ PUNICODE_STRING CompleteName,
	_Inout_ PUNICODE_STRING RemainingName,
	_Inout_opt_ PVOID Context,
	_In_opt_ PSECURITY_QUALITY_OF_SERVICE SecurityQos,
	_Out_ PVOID* Object);

typedef struct _OBJECT_TYPE_INITIALIZER {
	USHORT Length;
	union {
		USHORT Flags;
		struct {
			UCHAR CaseInsensitive : 1;
			UCHAR UnnamedObjectsOnly : 1;
			UCHAR UseDefaultObject : 1;
			UCHAR SecurityRequired : 1;
			UCHAR MaintainHandleCount : 1;
			UCHAR MaintainTypeList : 1;
			UCHAR SupportsObjectCallbacks : 1;
			UCHAR CacheAligned : 1;
			UCHAR UseExtendedParameters : 1;
			UCHAR _Reserved : 7;
		};
	};

	ULONG ObjectTypeCode;
	ULONG InvalidAttributes;
	GENERIC_MAPPING GenericMapping;
	ULONG ValidAccessMask;
	ULONG RetainAccess;
	POOL_TYPE PoolType;
	ULONG DefaultPagedPoolCharge;
	ULONG DefaultNonPagedPoolCharge;
	OB_DUMP_METHOD DumpProcedure;
	OB_OPEN_METHOD OpenProcedure;
	OB_CLOSE_METHOD CloseProcedure;
	OB_DELETE_METHOD DeleteProcedure;
	OB_PARSE_METHOD ParseProcedure;
	OB_SECURITY_METHOD SecurityProcedure;
	OB_QUERYNAME_METHOD QueryNameProcedure;
	OB_OKAYTOCLOSE_METHOD OkayToCloseProcedure;
	ULONG WaitObjectFlagMask;
	USHORT WaitObjectFlagOffset;
	USHORT WaitObjectPointerOffset;
} OBJECT_TYPE_INITIALIZER, * POBJECT_TYPE_INITIALIZER;

extern "C" {
	NTSTATUS NTAPI ObCreateObjectType(
		_In_ PUNICODE_STRING TypeName,
		_In_ POBJECT_TYPE_INITIALIZER ObjectTypeInitializer,
		_In_opt_ PSECURITY_DESCRIPTOR sd,
		_Deref_out_ POBJECT_TYPE* ObjectType);

	NTSTATUS NTAPI ObCreateObject(
		_In_ KPROCESSOR_MODE ProbeMode,
		_In_ POBJECT_TYPE ObjectType,
		_In_opt_ POBJECT_ATTRIBUTES ObjectAttributes,
		_In_ KPROCESSOR_MODE OwnershipMode,
		_Inout_opt_ PVOID ParseContext,
		_In_ ULONG ObjectBodySize,
		_In_ ULONG PagedPoolCharge,
		_In_ ULONG NonPagedPoolCharge,
		_Deref_out_ PVOID* Object);

	NTSTATUS NTAPI ObOpenObjectByName(
		_In_ POBJECT_ATTRIBUTES ObjectAttributes,
		_In_ POBJECT_TYPE ObjectType,
		_In_ KPROCESSOR_MODE AccessMode,
		_Inout_opt_ PACCESS_STATE AccessState,
		_In_opt_ ACCESS_MASK DesiredAccess,
		_Inout_opt_ PVOID ParseContext,
		_Out_ PHANDLE Handle);

	NTSTATUS NTAPI ZwGetNextProcess(
		_In_opt_ HANDLE ProcessHandle,
		_In_ ACCESS_MASK DesiredAccess,
		_In_ ULONG HandleAttributes,
		_In_ ULONG Flags,
		_Out_ PHANDLE NewProcessHandle);
}

PEPROCESS GetUserProcess() {
	HANDLE hProcess = nullptr, hNewProcess;
	for (int i = 0; i < 6; i++) {
		auto status = ZwGetNextProcess(hProcess, GENERIC_READ, OBJ_KERNEL_HANDLE, 0, &hNewProcess);
		if (hProcess)
			NtClose(hProcess);

		if (!NT_SUCCESS(status))
			break;

		hProcess = hNewProcess;
	}
	PEPROCESS process = nullptr;
	if (hProcess) {
		KdPrint(("Using handle 0x%p for process\n", hProcess));
		ObReferenceObjectByHandle(hProcess, GENERIC_READ, *PsProcessType, KernelMode, (PVOID*)&process, nullptr);
		ZwClose(hProcess);
	}
	KdPrint(("Process object from handle 0x%p (PID: 0x%X)\n", process, 
		process ? HandleToULong(PsGetProcessId(process)) : 0));
	return process;
}

NTSTATUS DsCreateDataStackObjectType() {
	UNICODE_STRING typeName = RTL_CONSTANT_STRING(L"DataStack");
	OBJECT_TYPE_INITIALIZER init{ sizeof(init) };
	init.DefaultNonPagedPoolCharge = sizeof(DataStack);
	init.PoolType = NonPagedPoolNx;
	init.ValidAccessMask = DATA_STACK_ALL_ACCESS;
	init.DeleteProcedure = OnDataStackDelete;
	init.SecurityRequired = TRUE;

	GENERIC_MAPPING mapping{
		STANDARD_RIGHTS_READ | DATA_STACK_QUERY,
		STANDARD_RIGHTS_WRITE | DATA_STACK_PUSH | DATA_STACK_POP | DATA_STACK_CLEAR,
		STANDARD_RIGHTS_EXECUTE | SYNCHRONIZE,
		DATA_STACK_ALL_ACCESS
	};
	init.GenericMapping = mapping;

	auto status = ObCreateObjectType(&typeName, &init, nullptr, &g_DataStackType);

	if (status == STATUS_OBJECT_NAME_COLLISION) {
		KdPrint(("DataStack Object type already exists\n"));
	}
	return status;
}

void DsInitializeDataStack(DataStack* DataStack, ULONG MaxItemSize, ULONG MaxItemCount, ULONG_PTR MaxSize) {
	InitializeListHead(&DataStack->Head);
	ExInitializeFastMutex(&DataStack->Lock);
	KeInitializeEvent(&DataStack->Event, NotificationEvent, FALSE);
	DataStack->Count = 0;
	DataStack->MaxItemCount = MaxItemCount;
	DataStack->Size = 0;
	DataStack->MaxItemSize = MaxItemSize;
	DataStack->MaxSize = MaxSize;
}

NTSTATUS NTAPI NtCreateDataStack(_Out_ PHANDLE DataStackHandle, _In_opt_ POBJECT_ATTRIBUTES DataStackAttributes, _In_ ULONG MaxItemSize, _In_ ULONG MaxItemCount, ULONG_PTR MaxSize) {
	auto mode = ExGetPreviousMode();

	extern POBJECT_TYPE g_DataStackType;
	//
	// sanity check
	//
	if (g_DataStackType == nullptr)
		return STATUS_NOT_FOUND;

	DataStack* ds;
	auto status = ObCreateObject(mode, g_DataStackType, DataStackAttributes, mode,
		nullptr, sizeof(DataStack), 0, 0, (PVOID*)&ds);
	if (!NT_SUCCESS(status)) {
		KdPrint(("Error in ObCreateObject (0x%X)\n", status));
		return status;
	}

	DsInitializeDataStack(ds, MaxItemSize, MaxItemCount, MaxSize);
	HANDLE hDataStack;
	status = ObInsertObject(ds, nullptr, DATA_STACK_ALL_ACCESS, 0, nullptr, &hDataStack);
	if (NT_SUCCESS(status)) {
		if (mode != KernelMode) {
			__try {
				*DataStackHandle = hDataStack;
			}
			__except (EXCEPTION_EXECUTE_HANDLER) {

			}
		}
		else {
			*DataStackHandle = hDataStack;
		}
	}
	else {
		KdPrint(("Error in ObInsertObject (0x%X)\n", status));
	}
	return status;
}

NTSTATUS NTAPI NtOpenDataStack(_Out_ PHANDLE DataStackHandle, _In_ ACCESS_MASK DesiredAccess, _In_ POBJECT_ATTRIBUTES DataStackAttributes) {
	return ObOpenObjectByName(DataStackAttributes, g_DataStackType, ExGetPreviousMode(),
		nullptr, DesiredAccess, nullptr, DataStackHandle);
}

_Use_decl_annotations_
NTSTATUS NTAPI NtPushDataStack(HANDLE DataStackHandle, const PVOID Item, ULONG ItemSize) {
	if (ItemSize == 0)
		return STATUS_INVALID_PARAMETER_3;

	if (!ARGUMENT_PRESENT(Item))
		return STATUS_INVALID_PARAMETER_2;

	DataStack* ds;
	auto status = ObReferenceObjectByHandleWithTag(DataStackHandle, DATA_STACK_PUSH, g_DataStackType,
		ExGetPreviousMode(), DataStackTag, (PVOID*)&ds, nullptr);
	if (!NT_SUCCESS(status))
		return status;

	status = DsPushDataStack(ds, Item, ItemSize);
	ObDereferenceObjectWithTag(ds, DataStackTag);

	return status;
}

_Use_decl_annotations_
NTSTATUS NTAPI NtPopDataStack(HANDLE DataStackHandle, PVOID Buffer, PULONG BufferSize) {
	if (!ARGUMENT_PRESENT(BufferSize))
		return STATUS_INVALID_PARAMETER_3;

	ULONG size;
	if (ExGetPreviousMode() != KernelMode) {
		__try {
			ProbeForRead(BufferSize, sizeof(ULONG), 1);
			size = *BufferSize;
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			return GetExceptionCode();
		}
	}
	else {
		size = *BufferSize;
	}

	if (!ARGUMENT_PRESENT(Buffer) && size != 0)
		return STATUS_INVALID_PARAMETER_2;

	DataStack* ds;
	auto status = ObReferenceObjectByHandleWithTag(DataStackHandle, DATA_STACK_POP, g_DataStackType,
		ExGetPreviousMode(), DataStackTag, (PVOID*)&ds, nullptr);
	if (!NT_SUCCESS(status))
		return status;

	status = DsPopDataStack(ds, Buffer, size, BufferSize);
	ObDereferenceObjectWithTag(ds, DataStackTag);
	return status;
}

NTSTATUS NTAPI NtClearDataStack(HANDLE DataStackHandle) {
	DataStack* ds;
	auto status = ObReferenceObjectByHandleWithTag(DataStackHandle, DATA_STACK_CLEAR, g_DataStackType,
		ExGetPreviousMode(), DataStackTag, (PVOID*)&ds, nullptr);
	if (!NT_SUCCESS(status))
		return status;

	status = DsClearDataStack(ds);
	ObDereferenceObjectWithTag(ds, DataStackTag);

	return status;
}

