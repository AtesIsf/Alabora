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

/* 
 * Returns the entity of the removed element.
 * Returns NULL if the given id is not found!!!
 */

void *ehm_remove(entity_hashmap_t *ehm, unsigned short id) {
  int index = id % HASH_ARR_LEN;

  hash_element_t *curr = ehm->elements[index];
  // Case 1: The first element of the linked list matches
  if (curr->id == id) {
    ehm->elements[index] = curr->next;
    curr->next = NULL;
    void *entity = curr->entity;
    free(curr);
    curr = NULL;
    return entity;
  }

  // Case 2: Another element matches
  void *entity = NULL;
  hash_element_t *prev = NULL;
  while (curr != NULL) {
    if (curr->id == id) {
      prev->next = curr->next;
      curr->next = NULL;
      entity = curr->entity;
      curr->entity = NULL;
      free(curr);
      curr = NULL;
      break;
    }
    prev = curr;
    curr = curr->next;
  }

  return entity;
}

/*
 * Helper function to free the linked lists of entity hashmaps.
 */

void ehm_free_helper(hash_element_t *element) {
  if (element == NULL) return;

  hash_element_t *next = element->next;
  element->next = NULL;
  element->entity = NULL;
  element->id = 0;
  free(element);
  ehm_free_helper(next);
}

/* Does not free the data associated with the hashmap!!! */

void ehm_free(entity_hashmap_t *ehm) {
  assert(ehm != NULL);

  for (int i = 0; i < HASH_ARR_LEN; i++) {
    hash_element_t *curr = ehm->elements[i];
    if (curr == NULL) continue;
    ehm_free_helper(curr);
    curr = NULL;
  }
} 

