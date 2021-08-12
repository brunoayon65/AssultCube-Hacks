#include <Windows.h>

BOOL patch_bytes(HANDLE , PVOID , PVOID , SIZE_T );
BOOL replace_code_with_nop(HANDLE, PVOID, PVOID, SIZE_T);
