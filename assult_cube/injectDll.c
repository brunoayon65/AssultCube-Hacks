
#include <Windows.h>
#include "stdio.h"
#include <stdlib.h>
#include "proc.h"
#include "return_codes.h"

// On success the program should open a message box on the remote processes.
PCSTR PATH_TO_DLL = "C:\\Users\\User\\source\\repos\\assult_cube\\Release\\assult_cube.dll";
#define MAX_PATH_SIZE (1000)

LPCWSTR WINDOW_NAME_OF_PROGRAM = L"AssaultCube";

/*
* purpose: inject the dll into game process.
* return: return the result of the function (success, error ....)
*/
int main()
{
	return_codes_t result = RC__UNINITIALIZED;
	DWORD process_id = get_process_id(WINDOW_NAME_OF_PROGRAM);
	//Get LoadLibrary function address –
	// the address doesn't change at remote process
	HMODULE kerenel_dll_handle = GetModuleHandleA("kernel32.dll");
	if (kerenel_dll_handle == NULL)
	{
		printf("error opening kerenk32.dll");
		return (int)RC__GET_MODULE_fAILED;
	}
	PVOID LoadLibary_address = (PVOID)GetProcAddress(kerenel_dll_handle, "LoadLibraryA");
	// Open remote process
	HANDLE proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_id);
	if (proc == NULL)
	{
		perror("error opening the process: ");
		return (int)RC__OPEN_PROCESS_FAILED;
	}
	// Get a pointer to memory location in remote process,
	// big enough to store DLL path
	PVOID memory_address = (PVOID)VirtualAllocEx(proc, NULL, sizeof(char) * strlen(PATH_TO_DLL), MEM_COMMIT, PAGE_READWRITE);
	if (NULL == memory_address) {
		perror("error allocate memory in remote process: ");
		return (int) RC__ALLOCATE_MEMORY_IN_REMOTE_PROCESS_FAILED;
	}
	// Write DLL name to remote process memory
	if (!WriteProcessMemory(proc, memory_address, PATH_TO_DLL, sizeof(char) * strlen(PATH_TO_DLL), NULL)) {
		perror("error write dll path in remote process memory: ");
		return RC__WRITE_IN_REMOTE_PROCESS_MEMORY_FAILED;
	}
	// Open remote thread, while executing LoadLibrary
	// with parameter DLL name, will trigger DLLMain
	HANDLE remote_process_handle = CreateRemoteThread(proc, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibary_address, memory_address, 0, NULL);
	if (NULL == remote_process_handle) {
		perror("error creating new thread in remote process: ");
		return (int)RC__CREATE_THREAD_FAILED;
	}
	printf("inject worked and finished successfully");
	WaitForSingleObject(remote_process_handle, INFINITE);
	CloseHandle(remote_process_handle);
	return 0;
}

