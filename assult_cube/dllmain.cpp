// dllmain.cpp : Defines the entry point for the DLL application.
#include <Windows.h>
#include "stdio.h"
#include "proc.h"
#include "mem.h"
#include <stdlib.h>

LPCSTR MESSAGEBOX_TEXT = "Error openning thread";
LPCWSTR WINDOW_NAME = L"AssaultCube";
// bytes to inject:
// 0F 85 46 01 00 00
#define INJECT_ADDRESS 0x096A1
#define BYTES_COUNT 6


VOID mainHack()
{
    PVOID old_bytes = malloc(BYTES_COUNT);
    BOOL is_hacked = FALSE;
    
    int proc_id = get_process_id(WINDOW_NAME);
    HANDLE process_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, proc_id);
    DWORD module_base_address = get_process_base_address(process_handle);
    PVOID inject_address = (PVOID)(module_base_address + INJECT_ADDRESS);
    
    DWORD process_status = 1;
    while (GetExitCodeProcess(process_handle, &process_status) && process_status == STILL_ACTIVE)
    {
        if (GetAsyncKeyState(VK_RBUTTON) & 1)
        {
            if (!is_hacked)
            {
                replace_code_with_nop(process_handle, inject_address, old_bytes, BYTES_COUNT);
            }
            else
            {
                patch_bytes(process_handle, inject_address, old_bytes, BYTES_COUNT);
            }
            is_hacked = !is_hacked;
        }
    }
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        HANDLE hThread = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)mainHack, NULL, 0, nullptr);
        if (hThread == NULL)
        {
            // Error
            MessageBoxA(NULL, MESSAGEBOX_TEXT, NULL, MB_OK);
        }
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

