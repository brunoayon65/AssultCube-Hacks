#include <Windows.h>
#include <stdlib.h>
#include "proc.h"

#define NOP_OPCODE 0x90

BOOL patch_bytes(HANDLE hProc, PVOID patchAddress, PVOID dataAddress, SIZE_T size)
{
    // The function is patching bytes
    // before the patch the function make sure that the page have the right presmitions.

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
        *(nop_arr+i) = NOP_OPCODE;
    }
    if (!patch_bytes(hProc, patchAddress, (PVOID)nop_arr, size))
        return FALSE;
    return TRUE;
}