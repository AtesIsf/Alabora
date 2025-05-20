#ifndef HASH_BST_H
#define HASH_BST_H

struct http_request;

typedef struct hashbst_node {
  const char *path;
  char *(*handler)(struct http_request *);
  struct hashbst_node *left;
  struct hashbst_node *right;
} hashbst_node_t;

hashbst_node_t *create_hashbst_node(const char *path, char *(*)(struct http_request *));

hashbst_node_t *generate_hashbst();

void insert_hashbst_node(hashbst_node_t **, const char *, char *(*)(struct http_request *));

void *find_hashbst_node(hashbst_node_t *, const char *);

void print_hashbst(hashbst_node_t *);

void free_hashbst(hashbst_node_t **);

char *file_to_str(const char *);

char *generate_http_response(const char *, const char *);

char *handle_home(struct http_request *);
char *handle_help(struct http_request *);
char *handle_page_not_found(struct http_request *);
char *handle_styles(struct http_request *);
char *handle_join(struct http_request *);

#endif
