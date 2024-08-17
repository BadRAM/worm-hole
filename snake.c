#include <ncurses.h>
#include <stdlib.h>

// World Tile Content Definitions

// Floor Tiles
#define EMPTY_TILE 0
#define BARE_STONE 1
#define MYCELIUM 2
#define WEEDS 3
#define FLOOR_TILE_BITMASK 3

// Objects
#define OBJECT_BITMASK 0xFC
#define OBJECT_CATEGORY_BITMASK 0xF0

#define PLAYER 0x10

#define FRUIT    0x40
#define MUSHROOM 0x40
#define FLOWER   0x44

#define DUNGBALL 0x5C
#define DUNGBALL_ROLLING_UP    0x60
#define DUNGBALL_ROLLING_DOWN  0x64
#define DUNGBALL_ROLLING_LEFT  0x68
#define DUNGBALL_ROLLING_RIGHT 0x6C

#define SLUG_BABY 0x90
#define SLUG_EGGS 0x94
#define SLIME     0x98

#define SLUG_HEAD_UP    0XA0
#define SLUG_HEAD_DOWN  0XA4
#define SLUG_HEAD_LEFT  0XA8
#define SLUG_HEAD_RIGHT 0XAC

#define SLUG_TAIL_UP    0XB0
#define SLUG_TAIL_DOWN  0XB4
#define SLUG_TAIL_LEFT  0XB8
#define SLUG_TAIL_RIGHT 0XBC


// NOFLOOR objects
#define NOFLOOR_FLAG 0xC0

#define WORM_TAIL       0xD0
#define WORM_TAIL_UP    0xD0
#define WORM_TAIL_DOWN  0xD1
#define WORM_TAIL_LEFT  0xD2
#define WORM_TAIL_RIGHT 0xD3

#define WORM_TAIL_FAT       0xD4
#define WORM_TAIL_FAT_UP    0xD4
#define WORM_TAIL_FAT_DOWN  0xD5
#define WORM_TAIL_FAT_LEFT  0xD6
#define WORM_TAIL_FAT_RIGHT 0xD7 

#define WORM_HEAD       0xD8
#define WORM_HEAD_UP    0xD8
#define WORM_HEAD_DOWN  0xD9
#define WORM_HEAD_LEFT  0xDA
#define WORM_HEAD_RIGHT 0xDB

// Direction bits for NOFLOOR, DIRECTIONAL type tiles (mostly worm segments)
#define DIRECTION_BITMASK 0b11
#define UP    0
#define DOWN  1
#define LEFT  2
#define RIGHT 3

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

// Define Variables
char ch; // this variable stores the last input
int world[WORLD_WIDTH][WORLD_HEIGHT] = {0};
int world_new[WORLD_WIDTH][WORLD_HEIGHT] = {0};
int popflags = 0; // population flags, used to check for extinction
int score = 0;

// Prototypes
void init();
bool loop();
void update_cell(int x, int y);
void random_tick(int x, int y);
void repopulate(int tile);
void slug_baby_move(int x, int y);
void worm_head(int x, int y);
int char_to_dir(char c);
struct xy worm_next_tile(int x, int y);
bool tile_empty(int x, int y);
int wget_floor(int x, int y);
int wget_n_floor(int x, int y);
int wget_dir(int x, int y);
int wget_n_dir(int x, int y);
int wget_obj(int x, int y);
int wget_n_obj(int x, int y);
void wset_floor(int x, int y, int floor_id);
void wset_obj(int x, int y, int object_id);
void wset_dir(int x, int y, int dir_id);
void wset_full(int x, int y, int id);
void buffer_world();
int cell_neighbors(int x, int y, int floor_type);
struct xy dir_to_offset(struct xy a, int dir);
bool bounds_check(int x, int y);
void render();

int main()
{   
    init();

    // Game Loop
    while (loop())
    {
        // idk we already did everything we wanted to.
    }

    endwin();           /* End curses mode        */

    return 0;
}

void init()
{
    // Initialize ncurses
    initscr();          /* Start curses mode        */

    if(has_colors() == FALSE)
    {   endwin();
        printf("Your terminal does not support color\n");
        exit(1);
    }

    start_color();
    // init_color(COLOR_GREEN, 50, 200, 50);
    // init_color(COLOR_RED, 200, 50, 150);
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_BLACK, COLOR_WHITE);
    init_pair(3, COLOR_WHITE, COLOR_MAGENTA);
    init_pair(4, COLOR_WHITE, COLOR_GREEN);

    raw();              /* Line buffering disabled  */
    keypad(stdscr, TRUE);       /* We get F1, F2 etc..      */
    noecho();           /* Don't echo() while we do getch */
    erase();
    curs_set(0);

    move(1, WORLD_WIDTH + 1);
    printw("RANDOM_TICKS: ");    
    char str[10];
    sprintf(str, "%d", RANDOM_TICKS);
    printw(str);   


    world_new[2][2]  = WORM_HEAD_RIGHT;
    world_new[3][2]  = WORM_TAIL_RIGHT;
    world_new[4][2]  = WORM_TAIL_RIGHT;
    world_new[5][2]  = WORM_TAIL_RIGHT;
    world_new[6][2]  = WORM_TAIL_RIGHT;


    //world_new[14][4] = MUSHROOM;
    //world_new[15][7] = MYCELIUM;

    buffer_world();
}

bool loop()
{
    // Render
    render();
    
    // Wait for input
    ch = getch();

    move(2, WORLD_WIDTH + 1);
    printw("LAST INPUT: ");
    addch(ch);

    move(3, WORLD_WIDTH + 1);
    printw("POPFLAGS: ");
    char str[8];
    sprintf(str, "%d", popflags);
    printw(str); 

    move(4, WORLD_WIDTH + 1);
    printw("SCORE: ");
    sprintf(str, "%d", score);
    printw(str); 
    
    // Process input
    if (ch == 'q')
    {
        return false;
    }
    

    // Update world
    popflags = 0;
    for (int x = 0; x < WORLD_WIDTH; x++)
    {
        for (int y = 0; y < WORLD_HEIGHT; y++)
        {
            update_cell(x, y);
        }
    }

    for (int i = 0; i < RANDOM_TICKS; ++i)
    {
        random_tick(rand() % WORLD_WIDTH, rand() % WORLD_HEIGHT);
    }

    if ((popflags & MYCELIUM_POPFLAG) == 0) repopulate(MYCELIUM);
    if ((popflags & WEEDS_POPFLAG) == 0) repopulate(WEEDS);
    if ((popflags & SLUG_POPFLAG) == 0) repopulate(SLUG_EGGS);


    buffer_world();

    return true;
}


void update_cell(int x, int y)
{
    switch (world[x][y] & FLOOR_TILE_BITMASK)
    {
        case MYCELIUM:
            popflags |= MYCELIUM_POPFLAG;
            break;
        case WEEDS:
            popflags |= WEEDS_POPFLAG;
            break;
        default: 
            break;
    }
    switch (world[x][y] & OBJECT_BITMASK)
    {
        case EMPTY_TILE:
            break;

        case SLUG_EGGS:
            popflags |= SLUG_POPFLAG;
            break;
        case SLUG_BABY:
            popflags |= SLUG_POPFLAG;
            slug_baby_move(x, y);
            break;

        case WORM_HEAD:
            worm_head(x, y);
            break;
    }
}

void random_tick(int x, int y)
{
    // Early return if worm or other nofloor tile
    if ((world[x][y] & NOFLOOR_FLAG) == NOFLOOR_FLAG) return;

    // Update floor tiles
    switch (world[x][y] & FLOOR_TILE_BITMASK)
    {
        case EMPTY_TILE:
            if (tile_empty(x, y))
            {
                int ne = cell_neighbors(x, y, MYCELIUM);
                if (ne >= 1 && ne <= (rand()%7)+1)
                {
                    world_new[x][y] = (world_new[x][y] & OBJECT_BITMASK) | MYCELIUM;
                }
            }
            break;

        case MYCELIUM:
            int nm = cell_neighbors(x, y, MYCELIUM);
            if (nm == 8 && tile_empty(x,y))
            {
                world_new[x][y] = (world_new[x][y] & FLOOR_TILE_BITMASK) | MUSHROOM;
            }
            break;
        case WEEDS:
            if (world[x][y] == WEEDS && world_new[x][y] == WEEDS && cell_neighbors(x, y, WEEDS) == 8)
            {
                world_new[x][y] = FLOWER | WEEDS;
            }
            // select a random adjacent or diagonal tile
            int rx = (rand() % 3) - 1;
            int ry = (rand() % 3) - 1;
            // abort if it is outside of the world.
            if (bounds_check(x+rx, y+ry) || (world_new[x+rx][y+ry] & NOFLOOR_FLAG) == NOFLOOR_FLAG) break;
            if ((world[x+rx][y+ry] & FLOOR_TILE_BITMASK) != BARE_STONE)
            {
                world_new[x+rx][y+ry] = (world_new[x+rx][y+ry] & OBJECT_BITMASK) | WEEDS;
            }
            break;
    }

    switch (world[x][y] & OBJECT_BITMASK)
    {
        case SLUG_EGGS:
            world_new[x][y] = (world_new[x][y] & FLOOR_TILE_BITMASK) | SLUG_BABY;
    }
}

void repopulate(int tile)
{
    int x = rand() % WORLD_WIDTH;
    int y = rand() % WORLD_HEIGHT;

    if (tile_empty(x, y))
    {
        world_new[x][y] = tile;
    }
}

void slug_baby_move(int x, int y)
{
    if ((world[x][y] & FLOOR_TILE_BITMASK) == WEEDS)
    {
        world_new[x][y] = world_new[x][y] & OBJECT_BITMASK;
        return;
    }
    int dirs[4] = {0};
    struct xy to;
    for (int i = 0; i < 4; ++i)
    {
        to = dir_to_offset((struct xy){.x = x, .y = y}, i);
        if (!tile_empty(to.x, to.y))
        {
            dirs[i] = 0;
            continue;
        }
        dirs[i] = 100;
        dirs[i] += rand() % 10;
        if ((world[to.x][to.y] & FLOOR_TILE_BITMASK) == WEEDS)
        {
            dirs[i] += 20;
        }
        if ((world[to.x][to.y] & OBJECT_BITMASK) == SLIME)
        {
            dirs[i] -= 15;
        }
    }
    int biggest = 0;
    for (int i = 0; i < 4; ++i)
    {
        if (dirs[i] > dirs[biggest])
        {
            biggest = i;
        }
    }
    if (dirs[biggest] > 0)
    {
        to = dir_to_offset((struct xy){.x = x, .y = y}, biggest);
        world_new[to.x][to.y] = world_new[to.x][to.y] & FLOOR_TILE_BITMASK;
        world_new[to.x][to.y] |= world[to.x][to.y] == FLOWER ? SLUG_HEAD_UP : SLUG_BABY;
        world_new[x][y] = (world_new[x][y] & FLOOR_TILE_BITMASK) | SLIME;
    }
}

void worm_head(int x, int y)
{
    struct xy to = {x,y};

    switch (ch)
    {
        case 'w':
            to.y--;
            break;
        case 's':
            to.y++;
            break;
        case 'a':
            to.x--;
            break;
        case 'd':
            to.x++;
            break;
    }

    bool has_moved = false;

    if (tile_empty(to.x, to.y))
    {
        world_new[to.x][to.y] = WORM_HEAD | char_to_dir(ch);
        has_moved = true;
    }

    if (has_moved)
    {
        if ((world[to.x][to.y] & OBJECT_BITMASK) == MUSHROOM)
        {
            score++;
            has_moved = false;
            world_new[x][y] = (world[x][y] & DIRECTION_BITMASK) | WORM_TAIL_FAT;
        } 
        else 
        {
            world_new[x][y] = (world[x][y] & DIRECTION_BITMASK) | WORM_TAIL;
        }
    }

    if (has_moved)
    {
        // define variables to store the position on the worm, and the surrounding segments.
        struct xy prev; // previous tile
        struct xy curr; // current tile
        struct xy next; // next tile
        curr.x = x;
        curr.y = y;
        prev = curr;

        // loop until hitting the end of the snake. will crash if oroboros is created.
        while (true)
        {
            next = worm_next_tile(curr.x, curr.y);

            // If the next tile does not contain a worm tail, We've hit the end!
            if ((world[next.x][next.y] & OBJECT_BITMASK) != WORM_TAIL && 
                (world[next.x][next.y] & OBJECT_BITMASK) != WORM_TAIL_FAT)
            {
                // Set the current tile to be replaced with EMPTY_TILE, or DUNGBALL if it or the previous tile was fat.
                bool dung = ((world[curr.x][curr.y] & OBJECT_BITMASK) == WORM_TAIL_FAT) || ((world[prev.x][prev.y] & OBJECT_BITMASK) == WORM_TAIL_FAT);
                world_new[curr.x][curr.y] = dung ? DUNGBALL : EMPTY_TILE;
                break;
            }
            // Else check if the current tile is fat, and the next tile is not fat.
            else if ((world[curr.x][curr.y] & OBJECT_BITMASK) == WORM_TAIL_FAT && (world[next.x][next.y] & OBJECT_BITMASK) != WORM_TAIL_FAT)
            {
                // Set the current tile to become not fat and the next tile to become fat.
                // world_new[curr.x][curr.y] = (world_new[curr.x][curr.y] & DIRECTION_BITMASK) | WORM_TAIL;
                wset_obj(curr.x, curr.y, WORM_TAIL);
                // world_new[next.x][next.y] = (world_new[next.x][next.y] & DIRECTION_BITMASK) | WORM_TAIL_FAT;
                wset_obj(next.x, next.y, WORM_TAIL_FAT);
            }

            prev = curr;
            curr = next;
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
    switch (/*world[x][y] & DIRECTION_BITMASK ||*/ wget_dir(x, y))
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

// set world_new's full tile object at x, y to id
void wset_full(int x, int y, int id)
{
    world_new[x][y] = id;
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

//Game of life style count of neighbors to cell that contain floor_type
int cell_neighbors(int x, int y, int floor_type)
{
    int r = 0;

    for(int nx = x-1; nx <= x+1; nx++)
    {
        for(int ny = y-1; ny <= y+1; ny++)
        {
            if ((nx == x && ny == y) || 
                bounds_check(nx, ny) || 
                (world[nx][ny] & OBJECT_CATEGORY_BITMASK) == FRUIT || 
                (world[nx][ny] & NOFLOOR_FLAG) == NOFLOOR_FLAG)
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

// check if coords are out of bounds, return true if so.
bool bounds_check(int x, int y)
{
    return x < 0 || y < 0 || x >= WORLD_WIDTH || y >= WORLD_HEIGHT;
}

void render()
{
    for (int x = 0; x < WORLD_WIDTH; x++)
    {
        for (int y = 0; y < WORLD_HEIGHT; y++)
        {
            int col = 1;
            if ((world[x][y] & 0xC0) == 0xC0)
            {
                col = 2;
            }
            else
            {
                col = (world[x][y] & FLOOR_TILE_BITMASK)+1;
            }
            attron(COLOR_PAIR(col));
            move(y, x);
            switch (world[x][y] & OBJECT_BITMASK)
            {
                case EMPTY_TILE:
                    addch('.');
                    break;

                case WORM_TAIL:
                    addch('w');
                    break;

                case WORM_TAIL_FAT:
                    addch('W');
                    break;

                case WORM_HEAD:
                    addch('H');
                    break;

                // case PLAYER:
                case SLIME:
                    addch('_');
                    break;

                case DUNGBALL:
                    addch('O');
                    break;

                case SLUG_EGGS:
                    addch('"');
                    break;

                case SLUG_BABY:
                    addch('s');
                    break;

                case SLUG_HEAD_UP:
                case SLUG_HEAD_DOWN:
                case SLUG_HEAD_LEFT:
                case SLUG_HEAD_RIGHT:
                case SLUG_TAIL_UP:
                case SLUG_TAIL_DOWN:
                case SLUG_TAIL_LEFT:
                case SLUG_TAIL_RIGHT:
                    addch('S');
                    break;

                case MUSHROOM:
                    addch('?');
                    break;

                case FLOWER:
                    addch('*');
                    break;
            }
            attroff(COLOR_PAIR(col));
        }
    }
}