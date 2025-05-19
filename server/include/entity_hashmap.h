#ifndef ENTITY_HASHMAP_H
#define ENTITY_HASHMAP_H

#define HASH_ARR_LEN (32)

typedef struct hash_element {
  unsigned short id;
  void *entity;
  struct hash_element *next;
} hash_element_t;

typedef struct entity_hashmap {
  hash_element_t **elements;
} entity_hashmap_t;

entity_hashmap_t *init_entity_hashmap();

entity_hashmap_t *init_entity_hashmap();

void ehm_add(entity_hashmap_t *, unsigned short, void *);

void *ehm_get(entity_hashmap_t *, unsigned short);

void *ehm_remove(entity_hashmap_t *, unsigned short);

void ehm_free(entity_hashmap_t *);

#endif
