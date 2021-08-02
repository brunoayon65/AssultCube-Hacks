/*
    This program going to change this two lines who suppose to move edx reciol func address
    *****************************************
    the code:
    .text:00463786                 push    eax
    .text:00463787                 lea     ecx, [esp+34h+var_18]
    .text:0046378B                 push    ecx
    .text:0046378C                 mov     ecx, esi
    .text:0046378E                 call    edx
    opcodes:
    50 8D  4C 24 1C 51 8B CE FF D2
    ******************************************
    to do nothing
    opcodes:
    0x90 * 10.
*/

#include <Windows.h>
#include "stdio.h"
#include <Psapi.h>
#include <processthreadsapi.h>

LPCWSTR WINDOW_NAME = L"AssaultCube";
//Char is the only type who is one byte.
CHAR BYTE_TO_INJECT[10] = { 0x90, 0x90, 0x90,0x90,0x90 ,0x90 ,0x90 ,0x90 ,0x90, 0x90 };

#define PLAYER_RELATIVE_ADDRESS 0x10F4F4 
#define FUNC_LENGTH 549 // In bytes.
#define INJECT_ADDRESS 0x63786
// This is a relative address to the process base pointer.
// From there the program read the address of the reciol funciton.

void increaseHealth();
DWORD getProcBaseAdd(HANDLE processHandle);
DWORD patchBytes(HANDLE , PVOID , PVOID , SIZE_T );

int main()
{
    DWORD procId = 0;
    HWND hWnd = FindWindow(0, WINDOW_NAME);
    if (hWnd == NULL)
    {
        printf("error finding game's window");
        return 0;
    }
    GetWindowThreadProcessId(hWnd, &procId);

    // get proc handle
    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procId);
    if (!hProc)
    {
        perror("error opening remote process: ");
        return 0;
    }
    DWORD_PTR baseAddress = getProcBaseAdd(hProc);


    // Find call reciol func
    PVOID injectionAddr = (PVOID)(baseAddress + INJECT_ADDRESS);

    // Change call reciol to do nothing.
    bool isValid = patchBytes(hProc, injectionAddr, &BYTE_TO_INJECT, sizeof(BYTE_TO_INJECT));
    if (!isValid)
    {
        printf("faild patch mov dx byte in remote process memory");
        return 0;
    }

    printf(" Injected successfully in address%p", injectionAddr);
    return 1;
}


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


DWORD_PTR getProcBaseAdd(HANDLE processHandle)
{
    // TODO: undresatnd code.
    DWORD_PTR   baseAddress = 0;
    HMODULE* moduleArray;
    LPBYTE      moduleArrayBytes;
    DWORD       bytesRequired;

    if (EnumProcessModules(processHandle, NULL, 0, &bytesRequired))
    {
        if (bytesRequired)
        {
            moduleArrayBytes = (LPBYTE)LocalAlloc(LPTR, bytesRequired);

            if (moduleArrayBytes)
            {
                unsigned int moduleCount;

                moduleCount = bytesRequired / sizeof(HMODULE);
                moduleArray = (HMODULE*)moduleArrayBytes;

                if (EnumProcessModules(processHandle, moduleArray, bytesRequired, &bytesRequired))
                {
                    baseAddress = (DWORD_PTR)moduleArray[0];
                }

                LocalFree(moduleArrayBytes);
            }
        }
    }
    return baseAddress;
}