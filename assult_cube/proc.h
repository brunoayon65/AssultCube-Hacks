#pragma once
#include <Windows.h>

DWORD_PTR get_process_base_address(HANDLE);
DWORD get_process_id(LPCWSTR);