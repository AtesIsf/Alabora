#ifndef HASH_BST_H
#define HASH_BST_H

typedef struct hashbst_node {
  const char *path;
  char *(*handler)(void);
  struct hashbst_node *left;
  struct hashbst_node *right;
} hashbst_node_t;

hashbst_node_t *create_hashbst_node(const char *path, char *(*)(void));

hashbst_node_t *generate_hashbst();

void insert_hashbst_node(hashbst_node_t **, const char *, char *(*)(void));

void *find_hashbst_node(hashbst_node_t *, const char *);

void print_hashbst(hashbst_node_t *);

void free_hashbst(hashbst_node_t **);

char *handle_ats();
char *handle_foo();
char *handle_ogr();
char *handle_bar();

#endif
