// DSTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <stdio.h>
#include "..\DataStack\DataStackAPI.h"
#include <string>

bool PushString(HANDLE h, std::string const& text) {
	auto ok = PushDataStack(h, (PVOID)text.c_str(), (ULONG)text.length() + 1);
	if (!ok)
		printf("Error in PushString: %u\n", GetLastError());
	return ok;
}

void PopItems(HANDLE h) {
	BYTE buffer[256];

	auto tick = GetTickCount64();
	while (GetTickCount64() - tick < 10000) {
		DWORD size = sizeof(buffer);
		if (!PopDataStack(h, buffer, &size) && GetLastError() != ERROR_NO_DATA) {
			printf("Error in PopDataStack (%u)\n", GetLastError());
			break;
		}

		if (size) {
			printf("Popped %u bytes: ", size);
			if (size > sizeof(int))
				printf("%s\n", (PCSTR)buffer);
			else
				printf("%d\n", *(int*)buffer);
		}
		Sleep(300);
	}
}

int main() {
	HANDLE hDataStack = CreateDataStack(nullptr, 0, 100, 10 << 20, L"MyDataStack");
	if (!hDataStack) {
		printf("Failed to create data stack (%u)\n", GetLastError());
		return 1;
	}

	printf("Handle created: 0x%p\n", hDataStack);

	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		printf("Opened an existing object... will popup elements\n");
		PopItems(hDataStack);
	}
	else {
		Sleep(5000);

		PushString(hDataStack, "Hello, data stack!");
		PushString(hDataStack, "Pushing another string...");
		for (int i = 1; i <= 10; i++) {
			Sleep(100);
			PushDataStack(hDataStack, &i, sizeof(i));
		}
	}

	CloseHandle(hDataStack);
	return 0;
}

