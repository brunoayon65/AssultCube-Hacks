#pragma once

#include <Windows.h>

// So other files could see the logger file pointer.
// the definition of the var is in return_codes.c

enum return_codes
{
    RC__UNINITIALIZED = -1,
    RC__SUCCESS = 0,
    RC__CREATE_THREAD_FAILED,
    RC__CREATE_PROCESS_FAILED,
    RC__MALOC_FAILED,
    RC__CANCEL_RECOIL_HACK_FAILED,
    RC__ALLOCATE_MEMORY_IN_REMOTE_PROCESS_FAILED,
    RC__PATCH_BYTES_FAILED,
    RC__CHANGE_MEMORY_PERMISSIONS_FAILED,
    RC__WRITE_IN_REMOTE_PROCESS_MEMORY_FAILED,
    RC__READ_IN_REMOTE_PROCESS_MEMORY_FAILED,
    RC__GET_MODULE_fAILED,
    RC__OPEN_PROCESS_FAILED,
    RC__TERMINATE_THREAD_FAILED,
    RC__OPEN_LOGGER_FILE_FAILED
};

enum log_event
{
    LE__WARNING,
    LE__ERROR,
    LE__INFO,
    LE__DEBUG
};

typedef enum log_event log_event_t;
typedef enum return_codes return_codes_t;

VOID print_error(return_codes_t );
VOID log_message(char const*, log_event_t );