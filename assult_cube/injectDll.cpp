// Barak Gonen 2019
// Skeleton code - inject DLL to a running process
// exercise number 8.1
#include <Windows.h>
#include "stdio.h"
#include <stdlib.h>
#include "proc.h"

// On succes the program should open a message box on the remote procces.
// You should open procces explorer and choose proc Id
PCSTR PATH_TO_DLL = "C:\\Users\\User\\source\\repos\\assult_cube\\Release\\AssultCube.dll";
#define MAX_PATH_SIZE (1000)

LPCWSTR WINDOW_NAME_OF_PROGRAM = L"AssaultCube";

int main()
{
	DWORD process_id = get_process_id(WINDOW_NAME_OF_PROGRAM);
	//Get LoadLibrary function address –
	// the address doesn't change at remote process
	HMODULE kerenel_dll_handle = GetModuleHandleA("kernel32.dll");
	if (kerenel_dll_handle == NULL)
	{
		printf("error openning kerenk32.dll");
		return 0;
	}
	PVOID LoadLibary_address = (PVOID)GetProcAddress(kerenel_dll_handle, "LoadLibraryA");
	DWORD err = 0;
	// Open remote process
	HANDLE proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_id);
	if (proc == NULL)
	{
		perror("error openenig the process: ");
		return 0;
	}
	// Get a pointer to memory location in remote process,
	// big enough to store DLL path
	PVOID memory_address = (PVOID)VirtualAllocEx(proc, NULL, sizeof(char) * strlen(PATH_TO_DLL), MEM_COMMIT, PAGE_READWRITE);
	if (NULL == memory_address) {
		perror("error allocate memory in remote proces: ");
		return 0;
	}
	// Write DLL name to remote process memory
	if (!WriteProcessMemory(proc, memory_address, PATH_TO_DLL, sizeof(char) * strlen(PATH_TO_DLL), NULL)) {
		perror("error write dll path in remote process memory: ");
		return 0;
	}
	// Open remote thread, while executing LoadLibrary
	// with parameter DLL name, will trigger DLLMain
	HANDLE remote_process_handle = CreateRemoteThread(proc, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibary_address, memory_address, 0, NULL);
	if (NULL == remote_process_handle) {
		perror("error creating new thread in remote process: ");
		return 0;
	}
	printf("inject worked and finished succesfully");
	WaitForSingleObject(remote_process_handle, INFINITE);
	CloseHandle(remote_process_handle);
	return 0;
}