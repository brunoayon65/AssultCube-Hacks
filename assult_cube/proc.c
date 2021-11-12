#include <Windows.h>
#include <processthreadsapi.h>
#include <Psapi.h>
#include "proc.h"


DWORD get_process_id(LPCWSTR WINDOW_NAME)
{
    // Return processs id base on his window name.
    // It won't work if we would open the program twice in the same time.
    DWORD process_id = 0;
    HWND window_handle = FindWindow(0, WINDOW_NAME);
    if (window_handle == NULL)
    {
        return 0;
    }
    GetWindowThreadProcessId(window_handle, &process_id);
    return process_id;
}