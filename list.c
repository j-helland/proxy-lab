/**
 * @author Jonathan Helland
 * 
 * Simple doubly linked circular list implementation with head insertion.
 * This is handy for enforcing an LRU policy in a cache.
 */
#include "list.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Insert a node at the head of the list.
 */
static node_t *list_insert_head(list_t *list, node_t *node) {
    if (list->length == 0) {
        list->head = node;
        node->next = node->prev = node;
    } else {
        node_t *head_orig = list->head;
        list->head = node;

        node->prev = head_orig->prev;
        head_orig->prev->next = node;

        node->next = head_orig;
        head_orig->prev = node;
    }

    list->length++;
    return list->head;
}

/**
 * Insert a node at the head of the list.
 */
node_t *list_insert(list_t *list, void *value) {
    node_t *node = malloc(sizeof(node_t));
    node->value = value;
    return list_insert_head(list, node);
}

/**
 * Remove a node from the list. Does not free any memory.
 */
node_t *list_delete(list_t *list, node_t *node) {
    if (list->length == 1) {
        list->head = NULL;
    } else {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }

    list->length--;
    return node;
}

/**
 * Move a node in the list to the head. This is useful for LRU eviction policies
 * in caches.
 */
node_t *list_move_to_head(list_t *list, node_t *node) {
    if (node == list->head)
        return node;

    list_delete(list, node);
    list_insert_head(list, node);
    return node;
}

/**
 * Find a node in the list by its value.
 *
 * @return NULL if the node was not found.
 */
node_t *list_find(list_t *list, void *value) {
    bool looped = false;
    node_t *n = list->head;
    while (!looped && n != NULL) {
        if (n->value == value)
            return n;

        n = n->next;
        looped = (n == list->head);
    }
    return NULL;
}

/**
 * Initialize memory for the list. You must call list_free when finished using
 * the list.
 *
 * @return Pointer to the allocated list.
 */
list_t *list_init(void) {
    list_t *list = calloc(1, sizeof(list_t));
    return list;
}

/**
 * Free memory of the list. Does not free the values contained by the nodes, you
 * must do this yourself.
 */
void list_free(list_t *list) {
    node_t *n = list->head;
    for (bool looped = false; n != NULL && !looped;) {
        node_t *next = n->next;
        free(n);
        n = next;

        looped = (n == list->head);
    }
    free(list);
}
