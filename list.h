/**
 * @author Jonathan Helland
 *
 * Simple doubly linked circular list implementation with head insertion.
 * This is handy for enforcing an LRU policy in a cache.
 */
#ifndef LIST_H
#define LIST_H

#include <stddef.h>

/**
 * Basic node in the linked list.
 */
typedef struct Node {
    struct Node *next, *prev;
    void *value;
} node_t;

/**
 * Stores pointer to start of the list plus metadata.
 */
typedef struct List {
    node_t *head;
    size_t length;
} list_t;

/**
 * Create a new node containing the value and insert to the head of the list.
 */
node_t *list_insert(list_t *list, void *value);

/**
 * Remove a node from the list. Does not free iany memory.
 */
node_t *list_delete(list_t *list, node_t *node);

/**
 * Move a node in the list to the head. This is useful for LRU eviction policies
 * in caches.
 */
node_t *list_move_to_head(list_t *list, node_t *node);

/**
 * Find a node in the list by its value.
 */
node_t *list_find(list_t *list, void *value);

/**
 * Initialize memory for the list. You must call list_free when finished using
 * the list.
 */
list_t *list_init(void);

/**
 * Free memory of the list. Does not free the values contained by the nodes, you
 * must do this yourself.
 */
void list_free(list_t *list);

#endif
