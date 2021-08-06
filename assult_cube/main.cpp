
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
#include "proc.h"
#include "mem.h"


LPCWSTR WINDOW_NAME = L"AssaultCube";
//Char is the only type who is one byte.
CHAR BYTE_TO_INJECT = 0xba;

#define PLAYER_RELATIVE_ADDRESS_POINTER 0x10F4F4
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
    DWORD procId = getProcId(WINDOW_NAME);
    if (procId == 0)
    {
        printf("error getting process Id by his window");
        return 0;
    }

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
    PVOID allocMem = (PVOID)VirtualAllocEx(hProc, NULL, FUNC_LENGTH, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    // option 2:
    // overide function "hit" at address 0x29C20
    //PVOID hitFuncAddr = (PVOID)(baseAddress + 0x29C20);
    bool isValid = patchBytes(hProc, allocMem, (PVOID)increaseHealthRealStart, FUNC_LENGTH);
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
    isValid = patchBytes(hProc, injectionAddr, &allocMem, addrSize);
    if (!isValid)
    {
        printf("faild patch increaseHealth *Address* in remote process memory");
        return 0;
    }

    //debug shit
    printf("recoil func address is %p \nInjected incraseHealth in address: %p \nincreasePlayerHealth address on our process is %p",
        injectionAddr, allocMem, pJumpIncreaseHealth);
    //increaseHealth();

    return 1;
}

void __declspec(naked) increaseHealth()
{
   // assuming that assultCube game loaded to 0x40000
    _asm
    {
        push ebx
        
        mov ebx, PLAYER_RELATIVE_ADDRESS_POINTER 
        add ebx, 0x400000
        mov ebx, [ebx]
        add ebx, HEALTH_OFFSET
        mov eax, [ebx]
        inc eax
        mov [ebx], eax
        pop ebx
        retn 8
    }
}

//void DECLSPEC_SAFEBUFFERS increaseHealth()
//{
//    // If you change this function you have to change FUNC_LENGTH
//    // By using "nm -S --size-sort -t d <objfile>" command.
//
//    DWORD playerHealth = 0;
//    // Get proc Id
//    DWORD pId = GetCurrentProcessId();
//    // get proc handle
//    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pId);
//    if (!hProc)
//    {
//        perror("Open process failed");
//        printf("error 2");
//        return;
//    }
//
//    // proc baseaddress after lodaing to the RAM. probably 0x400000.
//    DWORD baseAddress = (DWORD)GetModuleHandle(NULL);
//    printf("proc start in this address: %d \n", baseAddress);
//
//    // The pointer to player struct is a static address.
//    // always be in the same relative place to proc base address
//    DWORD_PTR pPlayer = baseAddress + PLAYER_RELATIVE_ADDRESS;
//    printf("Player address is: %d \n", pPlayer);
//
//    // Get player health location
//    PVOID pHealth = 0;
//    int is_valid = ReadProcessMemory(hProc, (LPCVOID)pPlayer, &pHealth, sizeof(DWORD), NULL);
//    if (!is_valid)
//    {
//        printf("error 3");
//        return;
//    }
//    // pHealth now got the address of the player.
//    pHealth = PVOID(DWORD(pHealth) + HEALTH_OFFSET);
//
//    // Get player health.
//    printf("player health address is: %d \n", (DWORD)pHealth);
//    is_valid = ReadProcessMemory(hProc, pHealth, &playerHealth, sizeof(int), NULL);
//    if (!is_valid)
//    {
//        printf("error 4");
//        return;
//    }
//    printf("Player current health is %d \n", playerHealth);
//
//    DWORD newPlayerHealth = playerHealth + 1;
//    //Change player health.
//    WriteProcessMemory(hProc, pHealth, &newPlayerHealth, sizeof(int), NULL);
//}