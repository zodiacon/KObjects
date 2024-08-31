// DSTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <stdio.h>
#include "..\DataStack\DataStackAPI.h"

int main() {
	HANDLE hDataStack = CreateDataStack(nullptr, 0, 100, 10 << 20, L"MyDataStack");
	if (!hDataStack) {
		printf("Failed to create data stack (%u)\n", GetLastError());
		return 1;
	}

	printf("Handle created: 0x%p\n", hDataStack);

	auto hOpen = OpenDataStack(GENERIC_READ, FALSE, L"MyDataStack");
	if (!hOpen) {
		printf("Failed to open data stack (%u)\n", GetLastError());
		return 1;
	}

	CloseHandle(hDataStack);
	CloseHandle(hOpen);
	return 0;
}

