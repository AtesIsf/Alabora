#ifndef GAME_H
#define GAME_H

#include "entity_hashmap.h"

#define N_PLAYERS (2)
#define N_ROWS (100)
#define N_COLS (100)

#define MIN_LAND (300)
#define MAX_LAND (600)

#define MEDIUM(n) ((n) * 2)
#define HIGH(n) ((n) * 4)

#define BASE_VISIBILITY (4)
#define BASE_COOLDOWN (1)
#define BASE_RANGE (8)
#define BASE_HP (1)
#define BASE_SPEED (1)

#define BUDGET (160)

#define SCOUT_PRICE (20)
#define STRIKE_PRICE (10)
#define SCREEN_PRICE (20)
#define CAPITAL_PRICE (40)

extern unsigned int global_id_counter;

typedef unsigned char byte_t;
struct ship;
struct missle;

typedef struct grid {
  struct ship *ship;
  struct missle *missle;
  int depth;
} grid_t;

typedef enum ship_type {
  SCOUT, // High visibility, no attack, low hp, medium speed
  STRIKE, // Short visibility, short range, fast attack, low hp, fast speed
  SCREEN, // Medium visibility, medium range. medium attack, medium hp, medium speed
  CAPITAL // Medium visibility, long range, slow attack, high hp, slow speed
} ship_type_t;

typedef struct missle {
  grid_t *pos;
  grid_t *target;
  unsigned short id;
} missle_t;

typedef struct missle_node {
  missle_t missle;
  struct missle_node *next;
  struct missle_node *prev;
} missle_node_t;

typedef struct ship {
  grid_t *pos;
  ship_type_t model;
  int hp;
  unsigned short id;
  unsigned short cooldown_left;
} ship_t;

typedef struct player {
  ship_t *ships;
  unsigned int id;
  unsigned int n_ships;
} player_t;

typedef struct game {
  player_t players[N_PLAYERS];
  grid_t grid[N_ROWS][N_COLS];
  missle_node_t *head;
  entity_hashmap_t *ehm;
} game_t;

game_t init_game();

void assign_ships(game_t *, int, const char *);

void update(game_t *, const char *, const char *);

void free_game(game_t *);

#endif
