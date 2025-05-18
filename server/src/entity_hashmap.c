#include "entity_hashmap.h"

#include <assert.h>
#include <stdlib.h>

entity_hashmap_t *init_entity_hashmap() {
  entity_hashmap_t *ehm = malloc(sizeof(entity_hashmap_t));
  assert(ehm != NULL);
  ehm->elements = calloc(HASH_ARR_LEN, sizeof(hash_element_t *));
  assert(ehm->elements != NULL);
  return ehm;
}

void ehm_add(entity_hashmap_t *ehm, unsigned short id, void *entity) {
  assert(ehm != NULL && entity != NULL);

  hash_element_t *addr = *(ehm->elements + id % HASH_ARR_LEN);
  // Case 1: No data in the cell
  if (addr->entity == NULL) {
    addr->entity = entity;
    addr->id = id;
    return;
  }

  // Case 2: New cell must be added
  hash_element_t *new_element = malloc(sizeof(hash_element_t));
  assert(new_element != NULL);
  new_element->entity = entity;
  new_element->id = id;
  new_element->next = NULL;
  
  while (addr->next != NULL) {
    addr = addr->next;
  }
  addr->next = new_element;
}

/* Returns NULL if the given id is not found!!! */

void *ehm_get(entity_hashmap_t *ehm, unsigned short id) {
  assert(ehm != NULL);

  hash_element_t *addr = *(ehm->elements + id % HASH_ARR_LEN);

  while (addr != NULL && addr->id != id) {
    addr = addr->next;
  }

  return addr;
}

/* Returns NULL if the given id is not found!!! */

void *ehm_remove(entity_hashmap_t *ehm, unsigned short id) {
  int index = id % HASH_ARR_LEN;

  hash_element_t *addr = ehm->elements[index];
  // Case 1: The first element of the linked list matches
  if (addr->id == id) {
    ehm->elements[index] = addr->next;
    addr->next = NULL;
    void *entity = addr->entity;
    free(addr);
    addr = NULL;
    return entity;
  }

  // Case 2: Another element matches
  // TODO

  return NULL;
}

/* Does not free the data associated with the hashmap!!! */

void ehm_free(entity_hashmap_t *);

