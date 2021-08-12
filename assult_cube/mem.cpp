#include <Windows.h>
#include <stdlib.h>
#include "proc.h"

BOOL patch_bytes(HANDLE hProc, PVOID patchAddress, PVOID dataAddress, SIZE_T size)
{
    DWORD old_protect;
    if (!VirtualProtectEx(hProc, patchAddress, size, PAGE_EXECUTE_READWRITE, &old_protect)) // Return 0 on fail
        return 0;
    if (!WriteProcessMemory(hProc, patchAddress, dataAddress, size, NULL)) // Return 0 on fail
        return 0;
    if (!VirtualProtectEx(hProc, patchAddress, size, old_protect, &old_protect)) // Return 0 on fail
        return 0;
    return 1;
}

BOOL replace_code_with_nop(HANDLE hProc, PVOID patchAddress, PVOID bufferStoreData, SIZE_T size)
{
    // nop is the instrucion in asm that does nothing.

    if (!ReadProcessMemory(hProc, patchAddress, bufferStoreData, size, NULL))
        return FALSE;
    PCHAR nop_arr = (PCHAR)malloc(size);
    for (int i = 0; i < size; i++)
    {
        // 0x90 is nop opcode.
        *(nop_arr+i) = 0x90;
    }
    if (!patch_bytes(hProc, patchAddress, (PVOID)nop_arr, size))
        return FALSE;
    return TRUE;
}