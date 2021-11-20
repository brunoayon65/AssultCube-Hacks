#include <Windows.h>
#include "return_codes.h"

return_codes_t patch_bytes(HANDLE , PVOID , PVOID , SIZE_T );
return_codes_t replace_code_with_nop(HANDLE, PVOID, PVOID, SIZE_T);
