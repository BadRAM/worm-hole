#ifndef WORLD_H
#define WORLD_H

#include <stdbool.h>


bool bounds_check(int x, int y);
int wget(int x, int y);
int wget_n(int x, int y);
int wget_floor(int x, int y);
int wget_n_floor(int x, int y);
int wget_dir(int x, int y);
int wget_n_dir(int x, int y);
int wget_obj(int x, int y);
int wget_n_obj(int x, int y);
int wget_type(int x, int y);
int wget_n_type(int x, int y);
bool wget_is_nofloor(int x, int y);
bool wget_n_is_nofloor(int x, int y);
void wset(int x, int y, int id);
void wset_floor(int x, int y, int floor_id);
void wset_obj(int x, int y, int object_id);
void wset_dir(int x, int y, int dir_id);
void buffer_world();
int char_to_dir(char c);
struct xy dir_to_offset(struct xy a, int dir);
struct xy worm_next_tile(int x, int y);
struct xy worm_prev_tile(int x, int y);
bool tile_empty(int x, int y);
int cell_neighbors(int x, int y, int floor_type);



#endif