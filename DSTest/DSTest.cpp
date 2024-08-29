// DSTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <stdio.h>
#include "..\DataStack\DataStackAPI.h"

int main() {
	HANDLE hDataStack = CreateDataStack(nullptr, 0, 100, 10 << 20, nullptr);
	if (!hDataStack) {
		printf("Failed to create data stack (%u)\n", GetLastError());
		return 1;
	}

	CloseHandle(hDataStack);

	return 0;
}

