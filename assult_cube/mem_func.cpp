#include <Windows.h>

DWORD patchBytes(HANDLE hProc, PVOID patchAddress, PVOID dataAddress, SIZE_T size)
{
    DWORD oldProtect;
    if (!VirtualProtectEx(hProc, patchAddress, size, PAGE_EXECUTE_READWRITE, &oldProtect)) // Return 0 on fail
        return 0;
    if (!WriteProcessMemory(hProc, patchAddress, dataAddress, size, NULL)) // Return 0 on fail
        return 0;
    if (!VirtualProtectEx(hProc, patchAddress, size, oldProtect, &oldProtect)) // Return 0 on fail
        return 0;
    return 1;
}