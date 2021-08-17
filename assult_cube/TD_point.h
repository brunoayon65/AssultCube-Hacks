#pragma once
typedef struct threeD_cords TD_t;

void switch_points(TD_t*, TD_t*);
void add(TD_t*, TD_t*, TD_t*);
void sub(TD_t*, TD_t*, TD_t*);
float get_distance(TD_t*, TD_t*);

struct threeD_cords
{
    float x;
    float y;
    float z;
};
