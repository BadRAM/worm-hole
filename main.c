#include <ncurses.h>
#include <stdlib.h>
#include <stdbool.h>

#include "main.h"
#include "world.h"

// Define Global Variables
char ch; // this variable stores the last input
int popflags = 0; // population flags, used to check for extinction
int score = 0;
int tick = 0;

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

    raw();                  /* Line buffering disabled  */
    keypad(stdscr, TRUE);   /* We get F1, F2 etc..      */
    noecho();               /* Don't echo() while we do getch */
    erase();
    curs_set(0);

    move(1, WORLD_WIDTH + 1);
    printw("RANDOM_TICKS: ");    
    char str[10];
    sprintf(str, "%d", RANDOM_TICKS);
    printw(str);   


    wset(2, 2, WORM_HEAD | RIGHT);
    wset(3, 2, WORM_TAIL | RIGHT);
    wset(4, 2, WORM_TAIL | RIGHT);
    wset(5, 2, WORM_TAIL | RIGHT);
    wset(6, 2, WORM_TAIL | RIGHT);

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

    buffer_world(); // having more than one buffer_world() per update may be an unnecessary slowdown.

    // random tick update
    for (int i = 0; i < RANDOM_TICKS; ++i)
    {
        random_tick(rand() % WORLD_WIDTH, rand() % WORLD_HEIGHT);
    }

    buffer_world();

    if ((popflags & MYCELIUM_POPFLAG) == 0) repopulate(MYCELIUM);
    if ((popflags & WEEDS_POPFLAG) == 0) repopulate(WEEDS);
    if ((popflags & SLUG_POPFLAG) == 0) repopulate(SLUG_EGGS);

    buffer_world();

    tick++;
    return true;
}

void update_cell(int x, int y)
{
    switch (wget_floor(x, y))
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
    switch (wget_obj(x, y))
    {
        case EMPTY_TILE:
            break;

        case SLUG_EGGS:
            popflags |= SLUG_POPFLAG;
            break;
        case SLUG_BABY:
            popflags |= SLUG_POPFLAG;
            if (tick % 2 == 0)
            {
                slug_baby_move(x, y);
            }
            break;
        case SLUG_HEAD_UP:
        case SLUG_HEAD_DOWN:
        case SLUG_HEAD_LEFT:
        case SLUG_HEAD_RIGHT:
            popflags |= SLUG_POPFLAG;
            if (tick % 3 == 0)
            {
                slug_move(x, y);
            }
            break;

        case WORM_HEAD:
            worm_head(x, y);
            break;
    }
}

void random_tick(int x, int y)
{
    // Early return if worm or other nofloor tile
    if ((wget(x, y) & NOFLOOR_FLAG) == NOFLOOR_FLAG) return;

    // Update floor tiles
    switch (wget_floor(x, y))
    {
        case EMPTY_TILE:
            if (tile_empty(x, y))
            {
                int ne = cell_neighbors(x, y, MYCELIUM);
                if (ne >= 1 && ne <= (rand()%7)+1)
                {
                    wset_floor(x, y, MYCELIUM);
                }
            }
            break;

        case MYCELIUM:
            int nm = cell_neighbors(x, y, MYCELIUM);
            if (nm == 8 && tile_empty(x,y))
            {
                wset_obj(x, y, MUSHROOM);
            }
            break;
        case WEEDS:
            // if the tile is empty and all it's neighbors are empty weeds, grow a flower here.
            if (wget(x, y) == WEEDS && wget_n(x, y) == WEEDS && cell_neighbors(x, y, WEEDS) == 8)
            {
                wset_obj(x, y, FLOWER);
            }
            // select a random adjacent or diagonal tile
            int rx = (rand() % 3) - 1;
            int ry = (rand() % 3) - 1;

            // abort if it is outside of the world
            if (bounds_check(x+rx, y+ry) || wget_n_is_nofloor(x+rx, y+ry)) break;
            if (wget_floor(x+rx, y+ry) != BARE_STONE)
            {
                wset_floor(x+rx, y+ry, WEEDS);
            }
            break;
    }

    switch (wget_obj(x, y))
    {
        case SLUG_EGGS:
            wset_obj(x, y, SLUG_BABY);
    }
}

void repopulate(int tile)
{
    int x = rand() % WORLD_WIDTH;
    int y = rand() % WORLD_HEIGHT;

    if (tile_empty(x, y))
    {
        wset(x, y, tile);
    }
}


// slug brain, decides which way to move and returns it as a direction.
// !! RETURNS -1 TO NOT MOVE
int slug_brain(int x, int y)
{
    int dirs[4] = {0};
    int best_dir = 0;
    struct xy to;

    // determine how appealing each direction is
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
        if (wget_floor(to.x, to.y) == WEEDS)
        {
            dirs[i] += 20;
        }
        if (wget_obj(to.x, to.y) == FLOWER)
        {
            dirs[i] += 15;
        }        
        if (wget_obj(to.x, to.y) == SLIME)
        {
            dirs[i] -= 15;
        }

        if (dirs[i] > dirs[best_dir])
        {
            best_dir = i;
        }
    }

    if (dirs[best_dir] > 0)
    {
        return best_dir;
    }

    return -1;
}

void slug_baby_move(int x, int y)
{
    if (wget_floor(x, y) == WEEDS)
    {
        wset_floor(x, y, EMPTY_TILE);
        return;
    }

    int dir = slug_brain(x, y);

    if (dir == -1) return;

    struct xy to = dir_to_offset((struct xy){.x = x, .y = y}, dir);
    if (wget_obj(to.x, to.y) == FLOWER)
    {
        wset_obj(to.x, to.y, SLUG_HEAD | ((dir^1)<<2));
    }
    else
    {
        wset_obj(to.x, to.y, SLUG_BABY);
    }
    wset_obj(x, y, SLIME);
}

void slug_move(int x, int y)
{
    if (wget_floor(x, y) == WEEDS)
    {
        wset_floor(x, y, EMPTY_TILE);
    }

    int dir = slug_brain(x, y);

    if (dir == -1) return;

    // to is used as a general purpose cursor.
    struct xy to = dir_to_offset((struct xy){.x = x, .y = y}, dir);
    wset_obj(to.x, to.y, SLUG_HEAD | ((dir^1)<<2));
    wset(x, y, SLUG_TAIL | (wget_n(x,y) & 0x0F));

    struct xy tail = dir_to_offset((struct xy){.x = x, .y = y}, (wget(x,y) & 0x0C)>>2);
    if (wget_type(tail.x, tail.y) == SLUG_TAIL) // if a third tail segment exists, replace it with slime
    {
        tail = dir_to_offset(tail, (wget(tail.x, tail.y) & 0x0C)>>2);
        wset_obj(tail.x, tail.y, SLIME);
    }
    if (wget_obj(to.x, to.y) == FLOWER) // if slug ate a flower, emit eggs
    {
        wset_obj(tail.x, tail.y, SLUG_EGGS);
    }

    // wset_obj(tail.x, tail.y, wget_obj(to.x, to.y) == FLOWER ? SLUG_EGGS : SLIME);
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
        wset(to.x, to.y, WORM_HEAD | char_to_dir(ch));
        has_moved = true;
    }

    if (has_moved)
    {
        if (wget_obj(to.x, to.y) == MUSHROOM)
        {
            score++;
            has_moved = false;
            wset_obj(x, y, WORM_TAIL_FAT); // Using wset_obj() here to set tail fat without affecting direction.
        } 
        else 
        {
            wset_obj(x, y, WORM_TAIL); // Using wset_obj() here to set tail without affecting direction.
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
            if (wget_obj(next.x, next.y) != WORM_TAIL && 
                (wget_obj(next.x, next.y) != WORM_TAIL_FAT))
            {
                // Set the current tile to be replaced with EMPTY_TILE, or DUNGBALL if it or the previous tile was fat.
                bool dung = (wget_obj(curr.x, curr.y) == WORM_TAIL_FAT) || (wget_obj(prev.x, prev.y) == WORM_TAIL_FAT);
                wset(curr.x, curr.y, dung ? DUNGBALL : EMPTY_TILE);
                break;
            }
            // Else check if the current tile is fat, and the next tile is not fat.
            else if (wget_obj(curr.x, curr.y) == WORM_TAIL_FAT && wget_obj(next.x, next.y) != WORM_TAIL_FAT)
            {
                // Set the current tile to become not fat and the next tile to become fat.
                wset_obj(curr.x, curr.y, WORM_TAIL);
                wset_obj(next.x, next.y, WORM_TAIL_FAT);
            }

            prev = curr;
            curr = next;
        }
    }
}

void render()
{
    for (int x = 0; x < WORLD_WIDTH; x++)
    {
        for (int y = 0; y < WORLD_HEIGHT; y++)
        {
            int col = 1;
            if (wget_is_nofloor(x, y))
            {
                col = 2;
            }
            else
            {
                col = wget_floor(x, y) + 1;
            }
            attron(COLOR_PAIR(col));
            move(y, x);
            switch (wget_obj(x, y))
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