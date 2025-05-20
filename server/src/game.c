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

  for (int i = 0; i < N_ROWS; i++) {
    for (int j = 0; j < N_COLS; j++) {
      g.grid[i][j].x = i;
      g.grid[i][j].y = j;
    }
  }

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
  assert(game->players[index].ships != NULL);

  int player_id = game->players[index].id;
  int i = 0;
  int start_x = player_num == 1 ? 0 : N_COLS - 1 - game->players[index].n_ships; 
  int start_y = player_num == 1 ? 0 : N_ROWS - 1; 
  for (i = 0; i < n_scouts; i++) {
    game->players[index].ships[i] = (ship_t) { 
      .id = global_id_counter++, .cooldown_left = 0, .player_id = player_id,
      .hp = BASE_HP, .model = SCOUT, .pos = &game->grid[start_x + i][start_y]
    }; 
    game->grid[start_x + i][start_y].ship = &game->players[index].ships[i];
  }
  int start_i = i;
  for (; i < n_strikers + start_i; i++) {
    game->players[index].ships[i] = (ship_t) { 
      .id = global_id_counter++, .cooldown_left = BASE_COOLDOWN, .player_id = player_id,
      .hp = BASE_HP, .model = STRIKE, .pos = &game->grid[start_x + i][start_y]
    }; 
    game->grid[start_x + i][start_y].ship = &game->players[index].ships[i];
  }
  start_i = i;
  for (; i < n_screens + start_i; i++) {
    game->players[index].ships[i] = (ship_t) { 
      .id = global_id_counter++, .cooldown_left = MEDIUM(BASE_COOLDOWN), .player_id = player_id,
      .hp = MEDIUM(BASE_HP), .model = SCREEN, .pos = &game->grid[start_x + i][start_y]
    }; 
    game->grid[start_x + i][start_y].ship = &game->players[index].ships[i];
  }
  start_i = i;
  for (; i < n_capitals + start_i; i++) {
    game->players[index].ships[i] = (ship_t) { 
      .id = global_id_counter++, .cooldown_left = HIGH(BASE_COOLDOWN), .player_id = player_id,
      .hp = HIGH(BASE_HP), .model = CAPITAL, .pos = &game->grid[start_x + i][start_y]
    }; 
    game->grid[start_x + i][start_y].ship = &game->players[index].ships[i];
  }
}

void hit_ship(game_t *game, ship_t *ship) {
  assert(game != NULL && ship != NULL && ship->hp > 0);
  player_t *player = ehm_get(game->ehm, ship->player_id);
  for (int i = 0; i < player->n_ships; i++) {
    if (player->ships[i].id == ship->id) {
      player->ships[i].hp--;
    }
  }

  if (ship->hp == 0) {
    ship_t *removed = ehm_remove(game->ehm, ship->id);
    assert(removed == ship);
    ship->id = SUNK_SHIP_ID;
    ship->pos->ship = NULL;
    ship->pos = NULL;
  }
}

void missle_free(game_t *game, missle_t *missle) {
  grid_t *pos = missle->pos;
  assert(pos->missle == missle);
  pos->missle = NULL;
  missle_node_t *node = game->head;
  while (node != NULL) {
    if (node->missle != missle)
      node = node->next;
  }
  assert(node != NULL);
  if (node->prev != NULL) {
    node->prev->next = node->next;
  }
  if (node->next != NULL) {
    node->next->prev = node->prev;
  }
  node->prev = NULL;
  node->next = NULL;
  node->missle = NULL;
  missle_t *m = ehm_remove(game->ehm, missle->id);
  assert(m == missle);
  free(missle);
  free(node);
  missle = NULL;
  node = NULL;
}

void add_missle(game_t *game, int shooter_id, int target_x, int target_y) {
  assert(game != NULL && shooter_id >= 0);
  assert(target_x >= 0 && target_x < N_ROWS);
  assert(target_y >= 0 && target_y < N_COLS);

  grid_t *spawn_pos = ehm_get(game->ehm, shooter_id);
  int delta_x = target_x - spawn_pos->x > 0 ? 1 : - 1;
  int delta_y = target_y - spawn_pos->y > 0 ? 1 : - 1;
  // Don't do all that computation if there has been a collision
  if (&game->grid[spawn_pos->x + delta_x][spawn_pos->y + delta_y] != NULL) {
    missle_free(game, game->grid[spawn_pos->x + delta_x][spawn_pos->y + delta_y].missle);
    game->grid[spawn_pos->x + delta_x][spawn_pos->y + delta_y].missle = NULL;
    return;
  }

  missle_t *new_missle = malloc(sizeof(missle_t));
  assert(new_missle != NULL);
  
  new_missle->pos = &game->grid[spawn_pos->x + delta_x][spawn_pos->y + delta_y];
  new_missle->id = global_id_counter++;
  new_missle->target = &game->grid[target_x][target_y];
  assert(spawn_pos != NULL);

  game->grid[spawn_pos->x + delta_x][spawn_pos->y + delta_y].missle = new_missle;

  missle_node_t *node = malloc(sizeof(missle_node_t));
  assert(node != NULL);
  node->missle = new_missle;
  node->prev = NULL;
  node->next = game->head;
  if (game->head != NULL) {
    game->head->prev = node;
  }
  game->head = node;
}

/* 
 * Helper function for update()
 * Move orders are in format (without <>):
 * <id>:<new_x>;<new_y>
 * Ship movement takes precedence over missle movement.
 * Missle create orders are in the form (without <>):
 * <shooter_id>-<target_x>;<target_y>
 */

void carry_out_orders(game_t *game, const char *orders) {
  assert(game != NULL && orders != NULL);

  int status = 3;
  // move orders
  while (status == 3) {
    int id = 0, new_x = 0, new_y = 0;
    status = sscanf(orders, "%d:%d;%d", &id, &new_x, &new_y);
    ship_t *ship = ehm_get(game->ehm, id);
    assert(ship != NULL);
    if (game->grid[new_x][new_y].ship != NULL) continue;
    ship->pos->ship = NULL;
    game->grid[new_x][new_y].ship = ship;
  }

  // missle create orders
  status = 3;
  while (status == 3) {
    int target_id = 0, target_x = 0, target_y = 0;
    status = sscanf(orders, "%d-%d;%d", &target_id, &target_x, &target_y);
    add_missle(game, target_id, target_x, target_y);
  }
}

void move_missles(game_t *game) {
  assert(game != NULL);

  missle_node_t *curr = game->head;
  while (curr != NULL) {
    missle_t *missle = curr->missle;

    int curr_x = missle->pos->x;
    int curr_y = missle->pos->y;
    int d_x = missle->target->x - curr_x;
    int d_y = missle->target->y - curr_y;

    // Move
    curr_x = d_x != 0 ? curr_x + MISSLE_MOVE_DIST : curr_x;
    curr_y = d_x != 0 ? curr_y + MISSLE_MOVE_DIST : curr_y;

    // Check collision
    grid_t *new_pos = &game->grid[curr_x][curr_y];
    
    // Missle collision
    if (new_pos->missle == NULL) {
      missle->pos->missle = NULL;
      new_pos->missle = missle;
    } else {
      missle_free(game, missle);
      missle_free(game, new_pos->missle);
      missle = NULL;
      return;
    }

    // Target/ship collision
    if (new_pos == missle->target) {
      if (new_pos->ship != NULL) hit_ship(game, new_pos->ship);
      missle_free(game, missle);
      missle = NULL;
    }

    curr = curr->next;
  }
}

void update(game_t *game, const char *p1_orders, const char *p2_orders) {
  assert(game != NULL && p1_orders != NULL && p2_orders != NULL);
  carry_out_orders(game, p1_orders);
  carry_out_orders(game, p2_orders);

  move_missles(game);
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

/* Assues the game instance is on the stack thus doesn't free it */

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

