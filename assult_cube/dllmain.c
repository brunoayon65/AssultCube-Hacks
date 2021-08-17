// dllmain.cpp : Defines the entry point for the DLL application.
#include <Windows.h>
#include <stdlib.h>
#include "stdio.h"
#include "proc.h"
#include "mem.h"
#include "game_structs.h"
#include "angels.h"    
#include "TD_point.h"

// This is a relative address to the process base pointer.
// From there the program read the address of the reciol funciton.
#define PLAYER_POINTER_RELATIVE_ADDRESS 0x10F4F4
#define OTHER_PLAYERS_ARRAY_RELATIVE_ADDRESS 0x10F4F8
#define NUMBER_OF_PLAYERS_RELATIVE_ADDRESS 0x10F500
#define HEALTH_OFFSET 0xF8
#define GET_PLAYER_ON_TARGET_OFFSET  0x607C0

//TODO: change to real function length.
#define FUNC_LENGTH 549 // In bytes.
#define INJECT_ADDRESS_NO_RECIOL 0x63781
#define WEAPON_NEW_STRENGTH 50

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
VOID main_hack();
BOOL cancel_reciol(HANDLE, DWORD);
player_t* find_closest_target(player_t**, player_t*, DWORD);

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)main_hack, NULL, 0, NULL);
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

VOID main_hack()
{
    //msg_box("to start hack press ok");
    
    BOOL is_map_hacked = FALSE;
    BOOL aimbot_status = FALSE;
    
    int proc_id = get_process_id(WINDOW_NAME);
    HANDLE process_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, proc_id);
    DWORD process_base_address = get_process_base_address(process_handle);

    player_t* player_pointer = *(player_t**)(process_base_address + PLAYER_POINTER_RELATIVE_ADDRESS);
    DWORD number_of_players = *(DWORD*)(NUMBER_OF_PLAYERS_RELATIVE_ADDRESS + process_base_address);
    player_t**  other_players = process_base_address + OTHER_PLAYERS_ARRAY_RELATIVE_ADDRESS;
    
    if (!cancel_reciol(process_handle, process_base_address))
        msg_box("error canceling reciol");

    //in this var will be stored the original code
    //of filtiring the enemy players and not showing them on the map.
    PVOID map_jnz_bytes = malloc(BYTES_COUNT);

    PVOID inject_address = (PVOID)(process_base_address + INJECT_ADDRESS_MAP_HACK);
    DWORD process_status = STILL_ACTIVE;
    player_t* target;

    while (GetExitCodeProcess(process_handle, &process_status) && process_status == STILL_ACTIVE)
    {
        player_t* (* get_player_ot_target)() = process_base_address + GET_PLAYER_ON_TARGET_OFFSET;

        //& 1 because if the user clicked on the key the function return 1 and this is what we are looking for
        if (GetAsyncKeyState(VK_NUMPAD9) & 1)
        {
            //see enemys on map
            if (!is_map_hacked)
            {
                replace_code_with_nop(process_handle, inject_address, map_jnz_bytes, BYTES_COUNT);
            }
            else
            {
                patch_bytes(process_handle, inject_address, map_jnz_bytes, BYTES_COUNT);
            }
            is_map_hacked = !is_map_hacked;
        }
        
        //teleport nearby enemy
        if (GetAsyncKeyState(VK_NUMPAD0) & 1)
        {
            DWORD number_of_players = *(DWORD*)(NUMBER_OF_PLAYERS_RELATIVE_ADDRESS + process_base_address);
            if (number_of_players == 0)
                continue;
            
            target = find_closest_target(other_players, player_pointer, number_of_players);
            if (target == NULL)
                // didn't find enemy player.
                continue;

            switch_points(&(player_pointer->cords), &(target->cords));
        }

        //aimbot
        if (GetAsyncKeyState(VK_NUMPAD8) & 1)
        {            
            aimbot_status = !aimbot_status;
        }

        if (aimbot_status)
        {
            number_of_players = *(DWORD*)(NUMBER_OF_PLAYERS_RELATIVE_ADDRESS + process_base_address);
            if (!number_of_players)
            {
                // if we're the only one in the game aimbot shouldn't work.
                continue;
            }
            target = find_closest_target(other_players, player_pointer, number_of_players);
            if (target == NULL)
                continue;
            player_pointer->yaw_angel = get_yaw_angel(player_pointer, target);
            player_pointer->pitch_angel = get_pitch_angel(player_pointer, target);

            if (target == get_player_ot_target())
            {
                player_pointer->is_shooting = TRUE;
            }
            else
            {
                player_pointer->is_shooting = FALSE;
            }
        }

        //changing values hack
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
            *(player_pointer->pointer_to_current_weapon->ammo) = 80;
        }

        player_pointer->pointer_to_current_weapon->data->damage = WEAPON_NEW_STRENGTH;
    }

    free(map_jnz_bytes);
}

player_t* find_closest_target(player_t** other_players,player_t* user_player, DWORD number_of_players)
{
    player_t* current_player;
    player_t* best_target = NULL;
    float min_distance = 100000;
    float current_distance;
    DWORD counter = 1;
    do
    {
        current_player = *((player_t**)(*other_players) + counter);
        counter++;

        if (!current_player || current_player->team == user_player->team || current_player->health > 100 || current_player->health <= 0)
            continue;

        current_distance = get_distance(&(user_player->cords), &(current_player->cords));

        if ( current_distance<min_distance)
        {
            best_target = current_player;
            min_distance = current_distance;
        }
        
    } while (counter < number_of_players);
    return best_target;
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

    // Find call reciol address
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