TODO 
====

 - Slug predation
 - Slugs eat slowly
 - Worm AI
 - Worm splitting
 - Isopod
 - Player is snail
 - Tile object type groups
 - ???
 - GBA port


PROBLEM:
========
Predation poses an issue, when can

Worm Cave Design Document:
==========================

Snake, but you're the food! Play as a snail whose slime trail attracts hungry worms!

Damage worms by driving them into themselves or eachother. Stronger worms will become two weaker worms when split. The rear half worm will reverse direction, the tail becoming the new head.

The worm cave will begin small, but worms can destroy the wall tiles and allow you more movement.

The cave contains an ecosystem of small creatures and plants which will grow and reproduce over time. The worms are the apex predator of this system, and do not reproduce except by splitting.


Ecosystem elements
==================

|----------------|---------------|----------------------|--------------------|
|     Object     |   Diet/Role   |     Destroyed by     |  Reproduces/Grows  |
|----------------|---------------|----------------------|--------------------|
| Worm           | apex predator | player/itself        | when split         |
| Snail (player) | Fungivore     | Worm                 | No                 |
| Slug           | Herbivore     | Worm                 | When eating flower |
| Isopod         | Detritivore   | Worm                 | When eating dung   |
| Mushroom       | Is Fruit      | Worm, Player, Isopod | From Mycelium      |
| Flower         | Is Fruit      | Worm, Slug           | From Weeds         |
| Mycelium       | Producer      | Worm, Weeds          | Spreads Quickly    |
| Weeds          | Producer      | Worm, Slug           | Spreads Slowly     |
| Dung           | Worm Residue  | ???                  | From Worm          |
|                |               |                      |                    |
|----------------|---------------|----------------------|--------------------|

Dirt Walls
 - Eroded by worm action

Worm
 - Apex predator
 - Can dig through dirt walls

Slug
 - worm's favorite treat
 - leaves a trail of slime
 - eats weeds

Beetle?
Isopod?
Ants?
Centipede?

Isopod


Weeds
 - Eaten by slugs

Mycelium
 - a

Mushroom

Dungball
 - Produced when worms eat
 - Can be dragged behind and sent rolling by snail
 - Rubs out slime trails

Snail (you!)
 - leaves a trail of slime


WORM AI
=======

 1. Eat adjacent food
 2. Follow slime trails
 3. Enter Mycelium Patches
 4. Eat Dirt
 5. Go Forward 80%, turn L/R 10%


WORLD TILE ID LAYOUT
====================

The world consists of a 2d array of byte sized tiles.

The right four bits represent the floor tile, and the left represent entities on top. Worms, Walls, and other entities which destroy/do not have floor under them use the full width of the byte to save definiton space.

Worm tiles use the right two bits to represent the direction to the next tile in the worm, away from the head.

Floor Tiles:
 0 - EMPTY
 1 - BARE_STONE
 2 - MYCELIUM
 3 - WEEDS


Objects:
 00 - EMPTY
 04 - 
 08 - 
 0C - 
 10 - PLAYER
 14 - 
 18 - 
 1C - 
 20 - 
 24 - 
 28 - 
 2C - 
 30 - 
 34 - 
 38 - 
 3C - 
 40 - MUSHROOM
 44 - FLOWER
 48 - MUSHROOM_EATEN
 4C - FLOWER_EATEN
 50 - 
 54 - 
 58 - 
 5C - DUNGBALL
 60 - DUNGBALL_ROLLING_UP
 64 - DUNGBALL_ROLLING_DOWN
 68 - DUNGBALL_ROLLING_LEFT
 6C - DUNGBALL_ROLLING_RIGHT
 70 - 
 74 - 
 78 - 
 7C - 
 80 - ISOPOD
 84 - ISOPOD_BABY
 88 - ISOPOD_PREGNANT_1
 8C - ISOPOD_PREGNANT_2
 90 - SLUG_BABY
 94 - SLUG_EGGS
 98 - SLIME
 9C - 
 A0 - SLUG_HEAD_UP
 A4 - SLUG_HEAD_DOWN
 A8 - SLUG_HEAD_LEFT
 AC - SLUG_HEAD_RIGHT
 B0 - SLUG_TAIL_UP
 B4 - SLUG_TAIL_DOWN
 B8 - SLUG_TAIL_LEFT
 BC - SLUG_TAIL_RIGHT
                            --- EVERYTHING BELOW C0 IS NOFLOOR --- 
 C0 - WORM_TAIL (NOFLOOR, DIRECTIONAL)
 C4 - WORM_TAIL_FAT (NOFLOOR, DIRECTIONAL)
 C8 - 
 CC - 
 D0 - WORM1_HEAD (NOFLOOR, DIRECTIONAL)
 D4 - WORM2_HEAD (NOFLOOR, DIRECTIONAL)
 D8 - WORM3_HEAD (NOFLOOR, DIRECTIONAL)
 DC - WORM4_HEAD (NOFLOOR, DIRECTIONAL)
 E0 - 
 E4 - 
 E8 - 
 EC - 
 F0 - 
 F4 - 
 F8 - 
 FC - WALL (NOFLOOR, FC:WALL_DIRT FD:WALL_STONE FE:ROOT_WALL FF:CRACKED_STONE_WALL)
