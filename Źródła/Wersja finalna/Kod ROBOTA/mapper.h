#ifndef _MOVEMENT_H_
#define _MOVEMENT_H_

#include "MKL46Z4.h"
void mapper_init(void);
void add_wall(void);
void change_direction_after_turn_left(void);
void change_direction_after_turn_right(void);
void change_direction_after_turn_around(void);
void print_map(void);
#endif

