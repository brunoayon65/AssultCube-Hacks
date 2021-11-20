#include <Windows.h>
#include "game_structs.h"
#include "aimbot.h"

#define GET_PLAYER_ON_TARGET_OFFSET  0x607C0
#define PLAYER_POINTER_RELATIVE_ADDRESS 0x10F4F4
#define OTHER_PLAYERS_ARRAY_RELATIVE_ADDRESS 0x10F4F8
#define NUMBER_OF_PLAYERS_RELATIVE_ADDRESS 0x10F500

/*
* purpose: find closet enemy player to the user player.
* return: a pointer to player struct of the closest player.
*/
player_t* find_closest_target(player_t** other_players, player_t* user_player, DWORD number_of_players)
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
