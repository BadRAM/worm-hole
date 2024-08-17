#include <stdlib.h>
#include <stdbool.h>

#include "main.h"

int world[WORLD_WIDTH][WORLD_HEIGHT] = {0};
int world_new[WORLD_WIDTH][WORLD_HEIGHT] = {0};


// check if coords are out of bounds, return true if so.
bool bounds_check(int x, int y)
{
    return x < 0 || y < 0 || x >= WORLD_WIDTH || y >= WORLD_HEIGHT;
}

int wget(int x, int y)
{
	return world[x][y];
}

int wget_n(int x, int y)
{
	return world_new[x][y];
}

int wget_floor(int x, int y)
{
    return world[x][y] & FLOOR_TILE_BITMASK;
}

int wget_n_floor(int x, int y)
{
    return world_new[x][y] & FLOOR_TILE_BITMASK;
}

int wget_dir(int x, int y)
{
    return world[x][y] & DIRECTION_BITMASK;
}

int wget_n_dir(int x, int y)
{
    return world_new[x][y] & DIRECTION_BITMASK;
}

int wget_obj(int x, int y)
{
    return world[x][y] & OBJECT_BITMASK;
}

int wget_n_obj(int x, int y)
{
    return world_new[x][y] & OBJECT_BITMASK;
}

int wget_type(int x, int y)
{
	return world[x][y] & OBJECT_TYPE_BITMASK;
}

int wget_n_type(int x, int y)
{
	return world_new[x][y] & OBJECT_TYPE_BITMASK;
}

bool wget_is_nofloor(int x, int y)
{
	return (world[x][y] & NOFLOOR_FLAG) == NOFLOOR_FLAG;
}

bool wget_n_is_nofloor(int x, int y)
{
	return (world[x][y] & NOFLOOR_FLAG) == NOFLOOR_FLAG;
}

// set world_new's full tile object at x, y to id
void wset(int x, int y, int id)
{
    world_new[x][y] = id;
}

// set world_new's tile floor at x, y to floor_id
void wset_floor(int x, int y, int floor_id)
{
    world_new[x][y] = (world_new[x][y] & OBJECT_BITMASK) | floor_id;
}

// set world_new's tile object at x, y to object_id
void wset_obj(int x, int y, int object_id)
{
    world_new[x][y] = (world_new[x][y] & FLOOR_TILE_BITMASK) | object_id;
}

void wset_dir(int x, int y, int dir_id)
{
    wset_floor(x, y, dir_id);
}

void buffer_world()
{
    for (int x = 0; x < WORLD_WIDTH; x++)
    {
        for (int y = 0; y < WORLD_HEIGHT; y++)
        {
            world[x][y] = world_new[x][y];
        }
    }
}

int char_to_dir(char c)
{
    switch (c)
    {
        case 'w':
            return DOWN;
        case 's':
            return UP;
        case 'a':
            return RIGHT;
        case 'd':
            return LEFT;
    }
    return 0;
}

struct xy dir_to_offset(struct xy a, int dir)
{
    switch (dir)
    {
        case 0:
            a.y--;
            break;
        case 1:
            a.y++;
            break;
        case 2:
            a.x--;
            break;
        case 3:
            a.x++;
            break;
    }
    return a;
}

// Get the position of the next tile in a worm.
struct xy worm_next_tile(int x, int y)
{
    struct xy pos;
    pos.x = x;
    pos.y = y;
    switch (wget_dir(x, y))
    {
        case 0:
            pos.y = y-1;
            break;
        case 1:
            pos.y = y+1;
            break;
        case 2:
            pos.x = x-1;
            break;
        case 3:
            pos.x = x+1;
            break;
    }
    return pos;
}

// Get the position of the previous tile in a worm. returns input if none can be found.
struct xy worm_prev_tile(int x, int y)
{
    struct xy r = {x,y};
    if (world[x][y-1] & NOFLOOR_FLAG == NOFLOOR_FLAG && world[x][y-1] & DIRECTION_BITMASK == DOWN)
    {
        r.y--;
    }
    else if (world[x][y+1] & NOFLOOR_FLAG == NOFLOOR_FLAG && world[x][y+1] & DIRECTION_BITMASK == UP)
    {
        r.y++;
    }
    else if (world[x-1][y] & NOFLOOR_FLAG == NOFLOOR_FLAG && world[x-1][y] & DIRECTION_BITMASK == RIGHT)
    {
        r.x--;
    }
    else if (world[x+1][y] & NOFLOOR_FLAG == NOFLOOR_FLAG && world[x+1][y] & DIRECTION_BITMASK == LEFT)
    {
        r.x++;
    }
    return r;
}

bool tile_empty(int x, int y)
{
    // block out of bounds
    if (bounds_check(x, y))
    {
        return false;
    }

    int tile = world[x][y] & OBJECT_BITMASK;
    for (int i = 0; i < 2; ++i) // this runs twice.
    {
        if (tile != MUSHROOM && tile != FLOWER && tile != SLIME && tile != 0) 
        {
            return false;
        }
        tile = world_new[x][y] & OBJECT_BITMASK;
    }
    return true;
}

//Game of life style count of neighbors to cell that contain floor_type
int cell_neighbors(int x, int y, int floor_type)
{
    int r = 0;

    for(int nx = x-1; nx <= x+1; nx++)
    {
        for(int ny = y-1; ny <= y+1; ny++)
        {
            if ((nx == x && ny == y) || bounds_check(nx, ny) || wget_type(nx, ny) == TYPE_FRUIT || wget_is_nofloor(nx, ny))
            {
                continue;
            }

            if ((world[nx][ny] & FLOOR_TILE_BITMASK) == floor_type)
            {
                r++;
            }
        }
    }
    return r;
}