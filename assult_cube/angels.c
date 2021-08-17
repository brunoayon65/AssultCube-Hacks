#include "math.h"
#include "game_structs.h"

#define PI 3.142857

float radian_to_degree(float);
float degree_to_radian(float);

float get_yaw_angel(player_t* user_player, player_t* enemy_player)
{
	// Yaw angel is left right angel
	// Yaw value is between 0 to 360
	float x_dif = enemy_player->x_value - user_player->x_value;
	float y_dif = enemy_player->y_value - user_player->y_value;

	return radian_to_degree(-atan2f(x_dif, y_dif)) + 180;
}

float get_pitch_angel(player_t* user_player, player_t* enemy_player)
{
	// pitch angel is up and down 
	// pitch value is between -90 to 90
	float x_dif = enemy_player->x_value - user_player->x_value;
	float y_dif = enemy_player->y_value - user_player->y_value;
	float ground_distance =(x_dif * x_dif) + (y_dif * y_dif); // distance form.
	ground_distance = sqrtf(ground_distance);
	
	float z_dif = enemy_player->z_value - user_player->z_value;
	float pitch = radian_to_degree(atan2f(z_dif, ground_distance));
	return pitch;
}

float get_distance(player_t* user_player, player_t* enemy_player)
{
	float x_dif = enemy_player->x_value - user_player->x_value;
	float y_dif = enemy_player->y_value - user_player->y_value;
	float z_dif = enemy_player->z_value - user_player->z_value;
	return sqrtf((x_dif * x_dif) + (y_dif * y_dif) + (z_dif * z_dif));
}

float radian_to_degree(float angel)
{
	return angel*(180.0F/ PI);
}

float degree_to_radian(float angel)
{
	return angel * (PI/180.0F);
}