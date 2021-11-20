// dllmain.cpp : Defines the entry point for the DLL application.
#include <Windows.h>
#include <stdlib.h>
#include "stdio.h"
#include "proc.h"
#include "mem.h"
#include "game_structs.h"
#include "angels.h"    
#include "TD_point.h"
#include "return_codes.h"

// all of this address are RVA - relative virtual address to the process base pointer.
#define PLAYER_POINTER_RELATIVE_ADDRESS 0x10F4F4
#define OTHER_PLAYERS_ARRAY_RELATIVE_ADDRESS 0x10F4F8
#define NUMBER_OF_PLAYERS_RELATIVE_ADDRESS 0x10F500

#define GET_PLAYER_ON_TARGET_OFFSET  0x607C0

// Health offset from start of player struct
#define HEALTH_OFFSET 0xF8

// Have to be updated manually.
#define FUNC_LENGTH 29 // In bytes.
#define INJECT_ADDRESS_NO_RECIOL 0x63781

#define NEW_WEAPON_STRENGTH 50
#define NEW_HEALTH_VALUE 234
#define NEW_SHIELD_VALUE 123
#define NEW_AMMO_VALUE 80

// inject to map hack:
// jnz bytes:
// 0F 85 46 01 00 00
#define INJECT_ADDRESS_MAP_HACK 0x096A1
#define BYTES_COUNT 6


LPCWSTR WINDOW_NAME = L"AssaultCube";
// For cancel recoil hack.
CHAR BYTE_TO_INJECT = 0xba;


VOID increase_player_health();
VOID main_hack();
VOID aimbot(PVOID);
PVOID cancel_reciol(HANDLE, DWORD, return_codes_t* );
player_t* find_closest_target(player_t**, player_t*, DWORD);
VOID values_hack(PVOID);
VOID HACK_LOOP(DWORD, HANDLE, player_t*, player_t**);

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
            print_error(RC__CREATE_THREAD_FAILED);
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
    MessageBoxA(NULL, "DLL injected Successfully", "HACK STARTED", MB_OK);

    player_t* player_pointer = NULL;
    player_t** other_players = NULL;
    PVOID p_increase_health_in_game_process = NULL;
    DWORD number_of_players = 0;
    return_codes_t result = RC__UNINITIALIZED;
    DWORD process_base_address = 0;

    DWORD proc_id = get_process_id(WINDOW_NAME);
    HANDLE game_process_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, proc_id);

    if (game_process_handle == NULL)
    {
        print_error(RC__CREATE_PROCESS_FAILED);
    }

    process_base_address = GetModuleHandle(L"ac_client.exe");
    player_pointer = *(player_t**)(process_base_address + PLAYER_POINTER_RELATIVE_ADDRESS);
    number_of_players = *(DWORD*)(NUMBER_OF_PLAYERS_RELATIVE_ADDRESS + process_base_address);
    other_players = process_base_address + OTHER_PLAYERS_ARRAY_RELATIVE_ADDRESS;

    p_increase_health_in_game_process = cancel_reciol(game_process_handle, process_base_address, &result);
    if (result != RC__SUCCESS)
    {
        print_error(RC__CANCEL_RECOIL_HACK_FAILED);
    }

    HANDLE values_hack_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)values_hack, (PVOID)player_pointer, 0, NULL);
    if (values_hack_thread == NULL)
    {
        print_error(RC__CREATE_THREAD_FAILED);
    }
    //Running until Assultcube stops.
    HACK_LOOP(process_base_address, game_process_handle, player_pointer, other_players);
    MessageBoxA(NULL, "FINISH HACK", "MESSAGE", MB_OK);
}

VOID HACK_LOOP(DWORD process_base_address, HANDLE game_process_handle, player_t* player_pointer, player_t** other_players)
{
    DWORD process_status = STILL_ACTIVE;
 
    //Aim bot variables.
    BOOL aimbot_status = FALSE;
    HANDLE aimbot_thread_handle = NULL;
    player_t* (*get_player_ot_target)() = process_base_address + GET_PLAYER_ON_TARGET_OFFSET;
    player_t* target = NULL;

    // Map hack variables.
    BOOL map_hack_status = FALSE;
    PVOID inject_address_map_hack = (PVOID)(process_base_address + INJECT_ADDRESS_MAP_HACK);
    //in this var will be stored the original code
    //of filtering the enemy players and not showing them on the map.
    PVOID map_jnz_bytes = malloc(BYTES_COUNT);
    if (map_jnz_bytes == NULL)
    {
        print_error(RC__MALOC_FAILED);
        return;
    }

    while (GetExitCodeProcess(game_process_handle, &process_status) && process_status == STILL_ACTIVE)
    {

        //& 1 because if the user clicked on the key the function return 1 and this is what we are looking for
        if (GetAsyncKeyState('2') & 1)
        {
            //see enemies on map
            if (!map_hack_status)
            {
                replace_code_with_nop(game_process_handle, inject_address_map_hack, map_jnz_bytes, BYTES_COUNT);
            }
            else
            {
                patch_bytes(game_process_handle, inject_address_map_hack, map_jnz_bytes, BYTES_COUNT);
            }
            map_hack_status = !map_hack_status;
        }

        //teleport nearby enemy
        if (GetAsyncKeyState('0') & 1)
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
        if (GetAsyncKeyState('1') & 1)
        {
            if (!aimbot_status)
            {
                // Turn on aimbot.
                aimbot_thread_handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)aimbot, &process_base_address, 0, NULL);
                if (aimbot_thread_handle == NULL)
                    print_error(RC__CREATE_THREAD_FAILED);
            }
            if (aimbot_status)
            {
                // Turn off aimbot.
                TerminateThread(aimbot_thread_handle, 0);
                player_pointer->is_shooting = FALSE;
            }
            aimbot_status = !aimbot_status;
        }
    }

    if (map_jnz_bytes != NULL)
    {
        free(map_jnz_bytes);
    }
}

VOID aimbot(PVOID process_base_address_pointer)
{
    DWORD process_base_address = *(DWORD *)process_base_address_pointer;
    player_t* (*get_player_ot_target)() = process_base_address + GET_PLAYER_ON_TARGET_OFFSET;
    player_t* player_pointer = *(player_t**)(process_base_address + PLAYER_POINTER_RELATIVE_ADDRESS);
    player_t** other_players = (process_base_address + OTHER_PLAYERS_ARRAY_RELATIVE_ADDRESS);

    player_t* target;
    while (TRUE)
    {
        DWORD number_of_players = *(DWORD*)(process_base_address + NUMBER_OF_PLAYERS_RELATIVE_ADDRESS);
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
}


player_t* find_closest_target(player_t** other_players,player_t* user_player, DWORD number_of_players)
{
    /*
        Return the closest enemy to player.
    */

    player_t* current_player;
    player_t* best_target = NULL;
    float min_distance = INT_MAX;
    float current_distance;
    DWORD counter = 1;
    do
    {
        current_player = *((player_t**)(*other_players) + counter);
        counter++;

        if (!current_player || current_player->team == user_player->team || current_player->health > 100 || current_player->health <= 0)
            continue;

        current_distance = get_distance(&(user_player->cords), &(current_player->cords));

        if (current_distance < min_distance)
        {
            best_target = current_player;
            min_distance = current_distance;
        }
        
    } while (counter < number_of_players);
    return best_target;
}

VOID values_hack(PVOID player)
{
    /*
    
    */

    player_t* player_pointer = (player_t*)player;
    //changing values hack
    while (TRUE)
    {
        if (player_pointer->health < 200)
        {
            player_pointer->health = NEW_HEALTH_VALUE;
        }

        if (player_pointer->shield < 100)
        {
            player_pointer->shield = NEW_SHIELD_VALUE;
        }

        if (*(player_pointer->pointer_to_current_weapon->ammo) < 50)
        {
            *(player_pointer->pointer_to_current_weapon->ammo) = NEW_AMMO_VALUE;
        }
        player_pointer->pointer_to_current_weapon->data->damage = NEW_WEAPON_STRENGTH;
    }
}

PVOID cancel_reciol(HANDLE process_handle, DWORD process_base_address, return_codes_t* result_out)
{
    return_codes_t result = RC__UNINITIALIZED;
    /*
    This program going to change this two lines who suppose to move edx recoil func address
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

    // Write function to remote process.
    // option 1:
    // allocate memory for the function
    PVOID p_increase_health_in_game_process = (PVOID)VirtualAllocEx(process_handle, NULL, FUNC_LENGTH, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (p_increase_health_in_game_process == NULL)
    {
        *result_out = RC__ALLOCATE_MEMORY_IN_REMOTE_PROCESS_FAILED;
        return NULL;
    }
    // option 2:
    // override function "hit" at address 0x29C20
    //PVOID hitFuncAddr = (PVOID)(baseAddress + 0x29C20);
    
    result = patch_bytes(process_handle, p_increase_health_in_game_process, (PVOID)increase_player_health, FUNC_LENGTH);
    if (result != RC__SUCCESS)
    {
        print_error(RC__PATCH_BYTES_FAILED);
        *result_out = result;
        return p_increase_health_in_game_process;
    }

    // Find call recoil address
    PVOID injection_address = (PVOID)(process_base_address + INJECT_ADDRESS_NO_RECIOL);

    // Change call recoil to call inject function
    result = patch_bytes(process_handle, injection_address, &BYTE_TO_INJECT, sizeof(char));
    if (result != RC__SUCCESS)
    {
        print_error(RC__PATCH_BYTES_FAILED);
        *result_out = result;
        return p_increase_health_in_game_process;
    }
    //This way injectionAddr increase only by one.
    injection_address = (PCHAR)injection_address + 1;
    // Switch bytes to call mine function
    result = patch_bytes(process_handle, injection_address, &p_increase_health_in_game_process, sizeof(PVOID));
    if (result != RC__SUCCESS)
    {
        print_error(RC__PATCH_BYTES_FAILED);
        *result_out = result;
        return p_increase_health_in_game_process;
    }

    *result_out = RC__SUCCESS;
    return p_increase_health_in_game_process;
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