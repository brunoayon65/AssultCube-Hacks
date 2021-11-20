#include <Windows.h>
#include <stdlib.h>
#include "return_codes.h"
#include "proc.h"

#define NOP_OPCODE 0x90

return_codes_t patch_bytes(HANDLE hProc, PVOID patchAddress, PVOID dataAddress, SIZE_T size)
{
    // The function is patching bytes
    // before the patch the function make sure that the page have the right permissions.

    DWORD old_protect;
    if (!VirtualProtectEx(hProc, patchAddress, size, PAGE_EXECUTE_READWRITE, &old_protect)) // Return 0 on fail
    {
        print_error(RC__CHANGE_MEMORY_PERMISSIONS_FAILED);
        return RC__CHANGE_MEMORY_PERMISSIONS_FAILED;
    }
    if (!WriteProcessMemory(hProc, patchAddress, dataAddress, size, NULL)) // Return 0 on fail
    {
        print_error(RC__WRITE_IN_REMOTE_PROCESS_MEMORY_FAILED);
        return RC__WRITE_IN_REMOTE_PROCESS_MEMORY_FAILED;
    }
    if (!VirtualProtectEx(hProc, patchAddress, size, old_protect, &old_protect)) // Return 0 on fail
    {
        print_error(RC__CHANGE_MEMORY_PERMISSIONS_FAILED);
        return RC__CHANGE_MEMORY_PERMISSIONS_FAILED;
    }
    return RC__SUCCESS;
}

BOOL replace_code_with_nop(HANDLE hProc, PVOID patchAddress, PVOID bufferStoreData, SIZE_T size)
{
    // nop is the instrucion in asm that does nothing.
    BOOL result = TRUE;
    if (!ReadProcessMemory(hProc, patchAddress, bufferStoreData, size, NULL))
        return FALSE;
    PCHAR nop_arr = (PCHAR)malloc(size);
    for (int i = 0; i < size; i++)
    {
        *(nop_arr+i) = NOP_OPCODE;
    }
    if (!patch_bytes(hProc, patchAddress, (PVOID)nop_arr, size))
        result = FALSE;
    free(nop_arr);
    return result;
}