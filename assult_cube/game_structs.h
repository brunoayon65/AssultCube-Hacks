#pragma once


struct player_struct;
struct weapon_struct;

typedef unsigned char _BYTE;

typedef struct player_struct player_t;
typedef struct weapon_struct weapon_t;
typedef struct threeD_cords TD_t;
typedef struct weapon_data weapon_data_t;

struct player_struct
{
    char padding[4];
    float head_x_value;
    float head_y_value;
    float head_z_value;
    char padding2[36];
    float x_value;
    float y_value;
    float z_value;
    float yaw_angel;
    float pitch_angel;
    _BYTE gap48[176];
    int health;
    int shield;
    _BYTE gap100[292];
    char is_shooting;
    char name[10];
    _BYTE gap22F[253];
    int team;
    _BYTE gap330[68];
    weapon_t* pointer_to_current_weapon;
    _BYTE gap378[73];
    char field_3D;
};


struct weapon_struct
{
    _BYTE gap0[4];
    int type;
    player_t* owner;
    weapon_data_t* data;
    int* mag_ammo;
    int* ammo;
};

struct weapon_data
{
    _BYTE gap0[268];
    int damage;
};

struct threeD_cords
{
    float x;
    float y;
    float z;
};