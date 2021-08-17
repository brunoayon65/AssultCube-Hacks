#include "TD_point.h"
#include "math.h"

void switch_points(TD_t* first, TD_t* second)
{
	TD_t tmp;
	tmp.x = first->x;
	tmp.y = first->y;
	tmp.z = first->z;

	first->x = second->x;
	first->y = second->y;
	first->z = second->z;

	second->x = tmp.x;
	second->y = tmp.y;
	second->z = tmp.z;
}

void sub(TD_t* first, TD_t* second, TD_t* result)
{
	result->x = first->x - second->x;
	result->y = first->y - second->y;
	result->z = first->z - second->z;
}

void add(TD_t* first, TD_t* second, TD_t* result)
{
	result->x = first->x + second->x;
	result->y = first->y + second->y;
	result->z = first->z + second->z;
}


float get_distance(TD_t* first, TD_t* second)
{
	TD_t dif;
	sub(first, second, &dif);
	return sqrtf((dif.x * dif.x) + (dif.y * dif.y) + (dif.z * dif.z));
}