#include "../include/hashbst.h"
#include "../include/connection_handler.h"
#include "../include/game.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUF_LEN (256)

hashbst_node_t *create_hashbst_node(const char *path, char *(*handler)(http_request_t *)) {
  assert(path != NULL && handler != NULL);

  hashbst_node_t *node = malloc(sizeof(hashbst_node_t));
  assert(node != NULL);
  node->left = NULL;
  node->right = NULL;
  node->handler = handler;
  node->path = path;

  return node;
}

hashbst_node_t *generate_hashbst() {
  hashbst_node_t *head = create_hashbst_node("/home", handle_home);
  insert_hashbst_node(&head, "/help", handle_help);
  insert_hashbst_node(&head, "/pagenotfound", handle_page_not_found);
  insert_hashbst_node(&head, "/styles.css", handle_styles);
  insert_hashbst_node(&head, "/join", handle_join);
  insert_hashbst_node(&head, "/", handle_home);

  return head;
}

/*
 * Allocates a new node and inserts it to the hashbst. Does not create and insert a new node
 * if the given path matches an already existing path.
 */

void insert_hashbst_node(hashbst_node_t **node, const char *path, char *(*handler)(http_request_t *)) {
  assert(node != NULL && path != NULL && handler != NULL);

  if (*node == NULL) {
    *node = create_hashbst_node(path, handler);
    return;
  }

  int status = strcmp((*node)->path, path);
  if (status == 0) return;
  if (status < 0) insert_hashbst_node(&(*node)->right, path, handler);
  if (status > 0) insert_hashbst_node(&(*node)->left, path, handler);
}

/*
 * Finds a node and returns its handler function. Returns NULL if it isn't found
 * The returned pointer must be cast to char *(*)(void)!!!
 */

void *find_hashbst_node(hashbst_node_t *node, const char *path) {
  assert(node != NULL && path != NULL);

  int status = strcmp(node->path, path);
  if (status == 0) return node->handler;

  if (node->right != NULL && status < 0)
    return find_hashbst_node(node->right, path);

  if (node->left != NULL && status > 0)
    return find_hashbst_node(node->left, path);

  return NULL;
}

/*
 * Prints the given hashbst in order
 */

void print_hashbst(hashbst_node_t *root) {
  assert(root != NULL);

  if (root->left != NULL) print_hashbst(root->left);
  printf("Endpoint: %s\n", root->path);
  if (root->right != NULL) print_hashbst(root->right);
}

void free_hashbst(hashbst_node_t **root) {
  assert(root != NULL && *root != NULL);
  if ((*root)->left != NULL) free_hashbst(&(*root)->left);
  if ((*root)->right != NULL) free_hashbst(&(*root)->right);
  free(*root);
  *root = NULL;
}

/*
 * --------------------
 *  HANDLERS FROM HERE
 * --------------------
 */

/*
 * Reads a file and returns the dynamically allocated string
 * to that file's contents. The return must be freed afterwards.
 */

char *file_to_str(const char *file_name) {
  assert(file_name != NULL);

  FILE *fp = fopen(file_name, "r");
  assert(fp != NULL);

  fseek(fp, 0, SEEK_END);
  unsigned long file_size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  char *response = malloc(file_size + 1);
  assert(response != NULL);
  response[file_size] = '\0';

  fread(response, file_size, 1, fp);

  fclose(fp);
  fp = NULL;
  return response;
}

char *generate_http_response(const char *file_name, const char *file_type) {
  assert(file_name != NULL);

  // "./pages/<file_name>.<file_type>" -> strlen(file_name) + 10 length str
  int size = strlen(file_name) + strlen(file_type) + 10;
  char *str = malloc(size);
  str[size - 1] = '\0';
  assert(str != NULL);
  sprintf(str, "./pages/%s.%s", file_name, file_type);

  char *body = file_to_str(str);
  char *response = wrap_in_http(body, HTTP_OK, file_type);

  free(str);
  str = NULL;
  free(body);
  body = NULL;
  return response;
}

char *handle_home(http_request_t *req) {
  return generate_http_response("home", "html");
}

char *handle_help(http_request_t *req) {
  return generate_http_response("help", "html");
}

char *handle_page_not_found(http_request_t *req) {
  return generate_http_response("page_not_found", "html");
}

char *handle_styles(http_request_t *req) {
  return generate_http_response("styles", "css");
}

/* The request MUST come with the player_id cookie!!! */

char *handle_join(http_request_t *req) {
  if (strncmp(req->method, "POST", 4) != 0)
    return generate_http_response("page_not_found", "html"); 

  char *player_id_str = strstr(req->cookies, "player_id");
  if (player_id_str != NULL) {
    unsigned short id = 0;
    sscanf(player_id_str, "player_id=%hu", &id);
    for (int i = 0; i < N_PLAYERS; i++) 
      if (id == global_player_ids[i])
        return generate_http_response("already_in_game", "html");
  }

  assert(global_player_count <= N_PLAYERS);
  if (global_player_count == N_PLAYERS)
      return generate_http_response("join_fail", "html");

  char id_str[4] = { '\0' };
  sprintf(id_str, "%hd", global_player_ids[global_player_count++]);
  return create_cookie_str("player_id", id_str, 300);
}

