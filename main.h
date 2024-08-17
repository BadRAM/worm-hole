#ifndef MAIN_H
#define MAIN_H

#include <stdbool.h>


// World Tile ID Definitions

// Floor Tiles
#define FLOOR_TILE_BITMASK 0b11
#define EMPTY_TILE 0
#define BARE_STONE 1
#define MYCELIUM   2
#define WEEDS      3

// Objects
#define OBJECT_BITMASK      0xFC
#define OBJECT_TYPE_BITMASK 0xF0 // Some objects are grouped into types which share the left nibble of their ID

#define PLAYER 0x10

#define TYPE_FRUIT 0x40
#define MUSHROOM   0x40
#define FLOWER     0x44

#define DUNGBALL               0x5C
#define DUNGBALL_ROLLING_UP    0x60
#define DUNGBALL_ROLLING_DOWN  0x64
#define DUNGBALL_ROLLING_LEFT  0x68
#define DUNGBALL_ROLLING_RIGHT 0x6C

#define SLUG_BABY 0x90
#define SLUG_EGGS 0x94
#define SLIME     0x98

#define SLUG_HEAD       0xA0
#define SLUG_HEAD_UP    0xA0
#define SLUG_HEAD_DOWN  0xA4
#define SLUG_HEAD_LEFT  0xA8
#define SLUG_HEAD_RIGHT 0xAC

#define SLUG_TAIL       0xB0
#define SLUG_TAIL_UP    0xB0
#define SLUG_TAIL_DOWN  0xB4
#define SLUG_TAIL_LEFT  0xB8
#define SLUG_TAIL_RIGHT 0xBC

// NOFLOOR objects
#define NOFLOOR_FLAG 0xC0

#define WORM_TAIL     0xC0
#define WORM_TAIL_FAT 0xC4
#define WORM_HEAD     0xD0

// Direction bits for NOFLOOR, DIRECTIONAL type tiles (mostly worm segments)
#define DIRECTION_BITMASK 0b11
#define UP    0
#define DOWN  1
#define LEFT  2
#define RIGHT 3

// Wall Tiles
#define WALL_DIRT    0xFC
#define WALL_STONE   0xFD
#define WALL_ROOT    0xFE
#define WALL_CRACKED 0xFF

// POPFLAGs, used to check if a certain population is zero
#define MYCELIUM_POPFLAG 0b00000001
#define WEEDS_POPFLAG    0b00000010
#define SLUG_POPFLAG     0b00000100

// World size definitions
#define WORLD_WIDTH 64
#define WORLD_HEIGHT 30
#define RANDOM_TICKS WORLD_WIDTH * WORLD_HEIGHT / 16

struct xy
{
    int x, y;
};

// Prototypes
void init();
bool loop();
void update_cell(int x, int y);
void random_tick(int x, int y);
void repopulate(int tile);
int slug_brain(int x, int y);
void slug_baby_move(int x, int y);
void slug_move(int x, int y);
void worm_head(int x, int y);
void render();

#endif