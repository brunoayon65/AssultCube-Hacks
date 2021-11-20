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

/*
* replace the current code at address 'patch_address' with code that does nothing.
*/
return_codes_t replace_code_with_nop(HANDLE process_handle, PVOID patch_address, PVOID buffer_store_data, SIZE_T size)
{
    // nop is the instruction in asm that does nothing.
    return_codes_t result = RC__UNINITIALIZED;
    if (!ReadProcessMemory(process_handle, patch_address, buffer_store_data, size, NULL))
        return RC__READ_IN_REMOTE_PROCESS_MEMORY_FAILED;
    PCHAR nop_arr = (PCHAR)malloc(size);
    for (int i = 0; i < size; i++)
    {
        *(nop_arr+i) = NOP_OPCODE;
    }
    result = patch_bytes(process_handle, patch_address, (PVOID)nop_arr, size);
    if(nop_arr != NULL)
        free(nop_arr);
    return result;
}