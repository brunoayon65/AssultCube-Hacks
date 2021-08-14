// dllmain.cpp : Defines the entry point for the DLL application.
#include <Windows.h>
#include "stdio.h"
#include "proc.h"
#include "mem.h"
#include <stdlib.h>

typedef unsigned char _BYTE;


struct player_struct
{
    char padding[4];
    int head_x_value;
    int head_y_value;
    int head_z_value;
    char padding2[36];
    int x_value;
    int y_value;
    int z_value;
    int yaw_angel;
    int pitch_angel;
    _BYTE gap48[176];
    int health;
    int shield;
    _BYTE gap100[292];
    char is_shooting;
    char name[10];
    _BYTE gap22F[253];
    int team;
    _BYTE gap330[68];
    struct weapon_struct* pointer_to_current_weapon;
    _BYTE gap378[73];
    char field_3D;
};


struct weapon_struct
{
    _BYTE gap0[4];
    int type;
    struct player_struct* owner;
    int* type_and_data;
    int* mag_ammo;
    int* ammo;
};


typedef struct player_struct player_t;
typedef struct weapon_struct ammo;

// This is a relative address to the process base pointer.
// From there the program read the address of the reciol funciton.
#define PLAYER_POINTER_RELATIVE_ADDRESS 0x10F4F4
#define OTHER_PLAYERS_ARRAY_RELATIVE_ADDRESS 0x10F4F8
#define NUMBER_OF_PLAYERS_RELATIVE_ADDRESS 0x10F500
#define HEALTH_OFFSET 0xF8

//TODO: change to real function length.
#define FUNC_LENGTH 549 // In bytes.
#define INJECT_ADDRESS_NO_RECIOL 0x63781

// inject to map hack:
// jnz bytes:
// 0F 85 46 01 00 00
#define INJECT_ADDRESS_MAP_HACK 0x096A1
#define BYTES_COUNT 6


LPCSTR MESSAGEBOX_TEXT = "Error openning thread";
LPCWSTR WINDOW_NAME = L"AssaultCube";
// For cancel reciol hack.
CHAR BYTE_TO_INJECT = 0xba;

void msg_box(const char *);
void increase_player_health();
VOID mainHack();
BOOL cancel_reciol(HANDLE, DWORD);

VOID mainHack()
{
    //msg_box("to start hack press ok");
    PVOID old_bytes = malloc(BYTES_COUNT);
    BOOL is_hacked = FALSE;
    
    int proc_id = get_process_id(WINDOW_NAME);
    HANDLE process_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, proc_id);
    DWORD process_base_address = get_process_base_address(process_handle);

    player_t* player_pointer = *(player_t**)(process_base_address + PLAYER_POINTER_RELATIVE_ADDRESS);
    DWORD number_of_players = *(DWORD*)(NUMBER_OF_PLAYERS_RELATIVE_ADDRESS + process_base_address);
    player_t**  other_players = process_base_address + OTHER_PLAYERS_ARRAY_RELATIVE_ADDRESS;
    
    if (!cancel_reciol(process_handle, process_base_address))
        msg_box("error canceling reciol");

    PVOID inject_address = (PVOID)(process_base_address + INJECT_ADDRESS_MAP_HACK);
    DWORD process_status = 1;
    player_t* enemy_player;
    DWORD counter = 1;
    while (GetExitCodeProcess(process_handle, &process_status) && process_status == STILL_ACTIVE)
    {
        if (GetAsyncKeyState(VK_NUMPAD9) & 1)
        {

            if (!is_hacked)
            {
                replace_code_with_nop(process_handle, inject_address, old_bytes, BYTES_COUNT);
            }
            else
            {
                patch_bytes(process_handle, inject_address, old_bytes, BYTES_COUNT);
            }
            is_hacked = !is_hacked;
        }
        
        if (GetAsyncKeyState(VK_NUMPAD0) & 1)
        {
            number_of_players = *(DWORD*)(NUMBER_OF_PLAYERS_RELATIVE_ADDRESS + process_base_address);
            if (number_of_players == 0)
                continue;
            do
            {
                enemy_player = *((player_t**)(*other_players)+counter);
                if (enemy_player && enemy_player->team != player_pointer->team)
                    break;
                counter++;
            } while (counter < number_of_players);
            
            if (!enemy_player)
                // didn't find enemy player.
                continue;

            player_pointer->x_value = enemy_player->x_value + 5;
            player_pointer->y_value = enemy_player->y_value;
            player_pointer->z_value = enemy_player->z_value;
            
        }


        if (player_pointer->health < 200)
        {
            player_pointer->health = 234;
        }

        if (player_pointer->shield < 100)
        {
            player_pointer->shield = 122;
        }

        if (*(player_pointer->pointer_to_current_weapon->ammo) < 50)
        {
            *(player_pointer->pointer_to_current_weapon->ammo) = 50;
        }
    }
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)mainHack, NULL, 0, NULL);
        if (hThread == NULL)
        {
            msg_box("Error oppening hack thread");
        }
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


BOOL cancel_reciol(HANDLE process_handle, DWORD process_base_address)
{
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

    // Write function to proc
    // option 1:
    // aloccate memory for the function
    PVOID allocate_memory = (PVOID)VirtualAllocEx(process_handle, NULL, FUNC_LENGTH, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    // option 2:
    // overide function "hit" at address 0x29C20
    //PVOID hitFuncAddr = (PVOID)(baseAddress + 0x29C20);
    
    if (!patch_bytes(process_handle, allocate_memory, (PVOID)increase_player_health, FUNC_LENGTH))
    {
        msg_box("faild patch increaseHealth function in remote process memory");
        return FALSE;
    }

    // Find call reciol func
    PVOID injection_address = (PVOID)(process_base_address + INJECT_ADDRESS_NO_RECIOL);

    // Change call reciol to call inject function
    if (!patch_bytes(process_handle, injection_address, &BYTE_TO_INJECT, sizeof(char)))
    {
        msg_box("faild patch mov dx byte in remote process memory");
        return FALSE;
    }
    //This way injectionAddr increase only by one.
    injection_address = (PCHAR)injection_address + 1;
    // Switch bytes to call mine function
    if (!patch_bytes(process_handle, injection_address, &allocate_memory, sizeof(PVOID)))
    {
        msg_box("faild patch increaseHealth *Address* in remote process memory");
        return FALSE;
    }

    return TRUE;
}

// __declspec(naked) saying that this code will be as he is.
// the compiler won't add any code.
void __declspec(naked) increase_player_health()
{
    // assuming that assultCube game loaded to 0x40000
    _asm
    {
        push ebx

        mov ebx, PLAYER_POINTER_RELATIVE_ADDRESS
        add ebx, 0x400000
        mov ebx, [ebx]
        add ebx, HEALTH_OFFSET
        mov eax, [ebx]
        inc eax
        mov[ebx], eax
        pop ebx
        retn 8
    }
}

void msg_box(const char* txt)
{
    MessageBoxA(NULL, txt, NULL, MB_OK);
}