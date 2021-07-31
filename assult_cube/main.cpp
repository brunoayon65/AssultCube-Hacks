
#include <Windows.h>
#include "stdio.h"
#include <Psapi.h>

LPCWSTR WINDOW_NAME = L"AssaultCube";
DWORD NEW_PLAYER_HEALTH = 10000;

#define PLAYER_RELATIVE_ADDRESS 0x10F4F4
#define HEALTH_OFFSET 0xF8

DWORD getModuleHandle(HANDLE processHandle);

int main()
{
    DWORD playerHealth = 0;
    // Get proc Id.
	HWND hWnd = FindWindow(0, WINDOW_NAME);
	if (hWnd == NULL)
	{
		printf("error 1");
		return 0;
	}
	DWORD pId;
	GetWindowThreadProcessId(hWnd, &pId);
    
    // get proc handle
	HANDLE hProc= OpenProcess(PROCESS_ALL_ACCESS, FALSE, pId);
	if (!hProc)
	{
        perror("Open process failed");
		printf("error 2");
		return 0;
	}

    // proc baseaddress after lodaing to the RAM. probably 0x400000.
	DWORD_PTR baseAddress = (DWORD)getModuleHandle(hProc);
    printf("proc start in this address: %d \n", baseAddress);

    // The pointer to player struct is a static address.
    // always be in the same relative place to proc base address
    DWORD_PTR pPlayer = baseAddress + PLAYER_RELATIVE_ADDRESS;
    printf("Player address is: %d \n", pPlayer);

    // Get player health location
    PVOID pHealth = 0;
    int is_valid = ReadProcessMemory(hProc, (LPCVOID)pPlayer, &pHealth, sizeof(DWORD), NULL);
    if (!is_valid)
    {
        printf("error 3");
        return 0;
    }
    // pHealth now got the address of the player.
    pHealth = PVOID(DWORD(pHealth) + HEALTH_OFFSET);

    // Get player health.
    printf("player health address is: %d \n", (DWORD)pHealth);
    is_valid = ReadProcessMemory(hProc, pHealth, &playerHealth, sizeof(int), NULL);
    if (!is_valid)
    {
        printf("error 4");
        return 0;
    }
    printf("Player current health is %d \n", playerHealth);

    //Change player health.
    WriteProcessMemory(hProc, pHealth, &NEW_PLAYER_HEALTH, sizeof(int), NULL);
}

DWORD_PTR getModuleHandle(HANDLE processHandle)
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
