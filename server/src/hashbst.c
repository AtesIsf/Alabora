#include "../include/hashbst.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUF_LEN (256)

hashbst_node_t *create_hashbst_node(const char *path, char *(*handler)(void)) {
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
  char *foo = "/foo";
  char *ogr = "/ogr";
  char *bar = "/foo/bar";
  char *ats = "/foo/ats";

  hashbst_node_t *head = create_hashbst_node(foo, handle_foo);
  insert_hashbst_node(&head, ogr, handle_ogr);
  insert_hashbst_node(&head, bar, handle_bar);
  insert_hashbst_node(&head, ats, handle_ats);

  return head;
}

/*
 * Allocates a new node and inserts it to the hashbst. Does not create and insert a new node
 * if the given path matches an already existing path.
 */

void insert_hashbst_node(hashbst_node_t **node, const char *path, char *(*handler)(void)) {
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

char *handle_foo() {
  return "/foo response";
}

char *handle_bar() {
  return "/foo/bar response";
}

char *handle_ogr() {
  return "/ogr response";
}

char *handle_ats() {
  return "/foo/ats response";
}

