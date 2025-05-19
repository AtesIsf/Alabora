#include "../include/game.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

unsigned int global_id_counter = 0;

game_t init_game() {
  game_t g = { 0 };
  g.players[0].id = global_id_counter++;
  g.players[1].id = global_id_counter++;

  srand(time(0));

  int n_islands = rand() % (MAX_LAND - MIN_LAND) + MIN_LAND + 1;
  for (int i = 0; i < n_islands; i++) {
    g.grid[rand() % N_ROWS][rand() % N_COLS].depth++;
  }
  for (int i = 0; i < 20; i++)
    for (int j = 0; j < 20; j++)
      g.grid[i][j].depth = 0;
  for (int i = N_ROWS - 20; i < N_ROWS; i++)
    for (int j = N_COLS - 20; j < N_COLS; j++)
      g.grid[i][j].depth = 0;

  g.ehm = init_entity_hashmap();

  return g;
}

/*
 * "ship_str" is expected in the format (without the <>'s):
 * <n_scouts>:<n_strikers>:<n_screens>:<n_capitals>
 */

void assign_ships(game_t *game, int player_num, const char *ship_str) {
  assert(game != NULL && player_num > 0 && player_num <= N_PLAYERS && ship_str != NULL);
  int index = player_num - 1;
  int n_scouts = 0;
  int n_strikers = 0;
  int n_screens = 0;
  int n_capitals = 0;

  int status = sscanf(ship_str, "%d:%d:%d:%d", &n_scouts, &n_strikers, &n_screens, &n_capitals);
  assert(status == 4);

  game->players[index].n_ships = n_scouts + n_strikers + n_screens + n_capitals;
  game->players[index].ships = malloc(game->players[index].n_ships * sizeof(ship_t));

  int i = 0;
  int start_x = player_num == 1 ? 0 : N_COLS - 1 - game->players[index].n_ships; 
  int start_y = player_num == 1 ? 0 : N_ROWS - 1; 
  for (i = 0; i < n_scouts; i++) {
    game->players[index].ships[i] = (ship_t) { 
      .id = global_id_counter++, .cooldown_left = 0,
      .hp = BASE_HP, .model = SCOUT, .pos = &game->grid[start_x + i][start_y]
    }; 
    game->grid[start_x + i][start_y].ship = &game->players[index].ships[i];
  }
  int start_i = i;
  for (; i < n_strikers + start_i; i++) {
    game->players[index].ships[i] = (ship_t) { 
      .id = global_id_counter++, .cooldown_left = BASE_COOLDOWN,
      .hp = BASE_HP, .model = STRIKE, .pos = &game->grid[start_x + i][start_y]
    }; 
    game->grid[start_x + i][start_y].ship = &game->players[index].ships[i];
  }
  start_i = i;
  for (; i < n_screens + start_i; i++) {
    game->players[index].ships[i] = (ship_t) { 
      .id = global_id_counter++, .cooldown_left = MEDIUM(BASE_COOLDOWN),
      .hp = MEDIUM(BASE_HP), .model = SCREEN, .pos = &game->grid[start_x + i][start_y]
    }; 
    game->grid[start_x + i][start_y].ship = &game->players[index].ships[i];
  }
  start_i = i;
  for (; i < n_capitals + start_i; i++) {
    game->players[index].ships[i] = (ship_t) { 
      .id = global_id_counter++, .cooldown_left = HIGH(BASE_COOLDOWN),
      .hp = HIGH(BASE_HP), .model = CAPITAL, .pos = &game->grid[start_x + i][start_y]
    }; 
    game->grid[start_x + i][start_y].ship = &game->players[index].ships[i];
  }
}

/*
 * Move orders are in format (without <>):
 * <id>:<new_x>;<new_y>
 * Ship movement takes precedence over missle movement.
 * Missle create orders are in the form (without <>):
 * <shooter_id>-<target_x>;<target_y>
 */

void update(game_t *game, const char *p1_orders, const char *p2_orders) {
  assert(game != NULL && p1_orders != NULL && p2_orders != NULL);
  int status = 3;
  // P1 move orders
  while (status == 3) {
    int id = 0, new_x = 0, new_y = 0;
    status = sscanf(p1_orders, "%d:%d;%d", &id, &new_x, &new_y);
    ship_t *ship = ehm_get(game->ehm, id);
    assert(ship != NULL);
    if (game->grid[new_x][new_y].ship != NULL) continue;
    ship->pos->ship = NULL;
    game->grid[new_x][new_y].ship = ship;
  }
}

/*
 * Helper to free the missle linked list of a game instance.
 */

void free_game_missles(missle_node_t *node) {
  missle_node_t *next = node->next;
  node->next = NULL;
  node->prev = NULL;
  free(node);
  node = NULL;
  free_game_missles(next);
}

void free_game(game_t *game) {
  for (int i = 0; i < N_PLAYERS; i++) {
    free(game->players[i].ships);
    game->players[i].ships = NULL;
  }
  ehm_free(game->ehm);
  game->ehm = NULL;
  free_game_missles(game->head);
  game->head = NULL;
}

