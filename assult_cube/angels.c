#include "math.h"
#include "game_structs.h"
#include "angels.h"
#include "TD_point.h"

#define PI 3.142857

float radian_to_degree(float);
float degree_to_radian(float);

float get_yaw_angel(player_t* user_player, player_t* enemy_player)
{
	// Yaw angel is left right angel
	// Yaw value is between 0 to 360
	float x_dif = enemy_player->cords.x - user_player->cords.x;
	float y_dif = (-enemy_player->cords.y) - (-user_player->cords.y);
	
	if(x_dif > 0)
		return radian_to_degree(atan2f(x_dif, y_dif));

	return 360 + radian_to_degree(atan2f(x_dif, y_dif));
}


float get_pitch_angel(player_t* user_player, player_t* enemy_player)
{
	// pitch angel is up and down 
	// pitch value is between -90 to 90
	float distance = get_distance(&(user_player->cords), &(enemy_player->cords));
	float z_dif = enemy_player->head_cords.z - user_player->head_cords.z;
	float pitch = radian_to_degree(asinf(z_dif/distance));
	return pitch;
}

float radian_to_degree(float angel)
{
	return angel*(180.0F/ PI);
}

float degree_to_radian(float angel)
{
	return angel * (PI/180.0F);
}