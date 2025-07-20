#ifndef SNAKE_H
#define SNAKE_H

#include "snake_t.h"

int main(int argc, char** argv);

static void show_welcome();
static void init_game();
static void start_game();
static void init_map();
static void init_snake();

static void create_food();
static void free_food();

static void move_forward(int direction);
static void add_head(int row, int col);
static void remove_tail();

static bool is_hit_self();
static bool is_hit_wall();
static bool is_hit_food();

#endif // SNAKE_H

#ifdef SNAKE_T_H
#endif

