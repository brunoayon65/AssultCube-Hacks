#include <Windows.h>
#include <processthreadsapi.h>
#include <Psapi.h>
#include "proc.h"


DWORD get_process_id(LPCWSTR WINDOW_NAME)
{
    DWORD process_id = 0;
    HWND window_handle = FindWindow(0, WINDOW_NAME);
    if (window_handle == NULL)
    {
        return 0;
    }
    GetWindowThreadProcessId(window_handle, &process_id);
    return process_id;
}

DWORD_PTR get_process_base_address(HANDLE processHandle)
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