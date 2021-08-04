// This program doesn't work.

/*
    This program going to change this two lines who suppose to move edx reciol func address
    *****************************************
    the code:
    .text:00463781                 mov     edx, [esi]
    .text:00463783                 mov     edx, [edx + 14h]
    opcodes:
    8B 16 8B 52 14
    ******************************************
    to:
    ******************************************
    the code:
    mov edx, increase_player_health_func
    for example  -> mov edx, 0x400000
    opcodes:
    ba 00 00 40 00


*/

#include <Windows.h>
#include "stdio.h"
#include <Psapi.h>
#include <processthreadsapi.h>

LPCWSTR WINDOW_NAME = L"AssaultCube";
//Char is the only type who is one byte.
CHAR BYTE_TO_INJECT = 0xba;

#define PLAYER_RELATIVE_ADDRESS 0x10F4F4
#define HEALTH_OFFSET 0xF8
#define FUNC_LENGTH 549 // In bytes.
#define INJECT_ADDRESS 0x63781

// This is a relative address to the process base pointer.
// From there the program read the address of the reciol funciton.

void increaseHealth();
DWORD getProcBaseAdd(HANDLE processHandle);
DWORD patchBytes(HANDLE, PVOID, PVOID, SIZE_T);

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

    // Change increase health premitions to readable.
    // get proc handle
    DWORD myProcId = GetCurrentProcessId();
    HANDLE hMyProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, myProcId);
    if (!hMyProc)
    {
        perror("Open program process failed: ");
        return 0;
    }
    DWORD oldProtect;

    // The increaseHealth function start with jump to funciton.
    // Find jump to func instruction address.
    PVOID pJumpIncreaseHealth = ((PCHAR)increaseHealth + 1);
    printf("address of jump instruction plus one: %p \naddress of incraseHealth function: %p\n",
        (PVOID)(pJumpIncreaseHealth), *(PVOID *)(pJumpIncreaseHealth));
    // find what is the value of the jump function.
    // The jump value is relative to the jump instruction address.
    DWORD increaseHealthRealStart = DWORD(increaseHealth) + *((DWORD_PTR *)pJumpIncreaseHealth) + 5;
    //ReadProcessMemory(hMyProc, pJumpIncreaseHealth, &increaseHealthRealStart, sizeof(PVOID), 0);
    //printf("jump relative address value is: %p \n", increaseHealthRealStart);

    //// sum the jump instruction address and the jump value togehter.
    //// +4 because we add the bytes of the jump instruction.
    //increaseHealthRealStart = ((PCHAR)increaseHealth) + ((DWORD)pJumpIncreaseHealth) + 5;
    printf("increaseHealth real address is %p \nFirst byte of function is: %hhx\n", increaseHealthRealStart,*(PCHAR)(increaseHealthRealStart));
    // Now we got increaseHealth function address.


    if (!VirtualProtectEx(hMyProc, (PVOID)increaseHealthRealStart, FUNC_LENGTH, PAGE_EXECUTE_READWRITE, &oldProtect))
    {
        printf("error change increase health premitions");
        return 0;
    }
    //printf("%d \n", oldProtect);

    // Write function to proc
    // option 1:
    // aloccate memory for the function
    // PVOID funcAddr = (PVOID)VirtualAllocEx(hProc, NULL, FUNC_LENGTH, MEM_COMMIT, PAGE_READWRITE);
    // option 2:
    // overide function "hit" at address 0x29C20
    PVOID hitFuncAddr = (PVOID)(baseAddress + 0x29C20);
    bool isValid = patchBytes(hProc, hitFuncAddr, (PVOID)increaseHealthRealStart, FUNC_LENGTH);
    if (!isValid)
    {
        printf("faild patch increaseHealth function in remote process memory");
        return 0;
    }

    // Find call reciol func
    PVOID injectionAddr = (PVOID)(baseAddress + INJECT_ADDRESS);

    // Change call reciol to call inject function
    isValid = patchBytes(hProc, injectionAddr, &BYTE_TO_INJECT, sizeof(char));
    if (!isValid)
    {
        printf("faild patch mov dx byte in remote process memory");
        return 0;
    }
    //This way injectionAddr increase only by one.
    injectionAddr = (PCHAR)injectionAddr + 1;
    // Switch bytes to call mine function
    SIZE_T addrSize = sizeof(PVOID);
    isValid = patchBytes(hProc, injectionAddr, &hitFuncAddr, addrSize);
    if (!isValid)
    {
        printf("faild patch increaseHealth *Address* in remote process memory");
        return 0;
    }

    //debug shit
    printf("recoil func address is %p \nInjected command in address: %p \nincreasePlayerHealth address is %p",
        injectionAddr, hitFuncAddr, pJumpIncreaseHealth);
    //increaseHealth();

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

//__declspec(naked) is for nothing to be write other than the function content.
void __declspec(naked) increaseHealth()
{
    _asm
    {
        retn 8
    }
    
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