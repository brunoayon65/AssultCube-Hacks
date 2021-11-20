#pragma once
#include <Windows.h>
#include "game_structs.h"

VOID aimbot(PVOID);
player_t* find_closest_target(player_t** , player_t* , DWORD );