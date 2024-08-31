#include "pch.h"

HANDLE g_hDevice = INVALID_HANDLE_VALUE;

bool OpenDevice() {
	UNICODE_STRING devName;
	RtlInitUnicodeString(&devName, L"\\Device\\KDataStack");
	OBJECT_ATTRIBUTES devAttr;
	InitializeObjectAttributes(&devAttr, &devName, 0, nullptr, nullptr);
	IO_STATUS_BLOCK ioStatus;
	return NT_SUCCESS(NtOpenFile(&g_hDevice, GENERIC_READ | GENERIC_WRITE, &devAttr, &ioStatus, 0, 0));
}

void CloseDevice() {
	if (g_hDevice != INVALID_HANDLE_VALUE) {
		CloseHandle(g_hDevice);
		g_hDevice = INVALID_HANDLE_VALUE;
	}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
	switch (reason) {
		case DLL_PROCESS_ATTACH:
			DisableThreadLibraryCalls(hModule);
			return OpenDevice();

		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			CloseDevice();
			break;
	}
	return TRUE;
}

