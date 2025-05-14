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

char *file_to_str(const char *);

char *generate_http_response(const char *, const char *);

char *handle_home();
char *handle_help();
char *handle_page_not_found();
char *handle_styles();

#endif
