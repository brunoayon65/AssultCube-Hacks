#include <Windows.h>
#include <processthreadsapi.h>
#include <Psapi.h>
#include "proc.h"


DWORD getProcId(LPCWSTR WINDOW_NAME)
{
    DWORD procId = 0;
    HWND hWnd = FindWindow(0, WINDOW_NAME);
    if (hWnd == NULL)
    {
        return 0;
    }
    GetWindowThreadProcessId(hWnd, &procId);
    return procId;
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