/**
 * @author Jonathan Helland

 * A hash table implementation using Robin Hood hashing to resolve collisions
 * efficiently while trying to maintain locality of previously inserted elements.
 *
 * Inspired by the following Robin Hood Hashmap library:
 * https://github.com/rmind/rhashmap
 *
 * The hashing function used is adapted from the djb2 hash function: 
 * http://www.cse.yorku.ca/~oz/hash.html.
 */
#ifndef HASHMAP_H
#define HASHMAP_H

#include <stddef.h>

/**
 * @param  key     Key that will be mapped to by the hash function.
 * @param  value   Value associated with the key.
 * @param  hash    Full hash of the key (i.e. non-modulo the size).
 * @param  psl     Probe sequence length. Describes how far this bin is
 *                 displaced from its original key mapping. Used for
 *                 Robin Hood displacement policy.
 * @param  keylen  Number of bytes for the key.
 */
typedef struct Bin {
    void *key, *value;
    size_t hash;
    size_t psl;
    size_t keylen;
} bin_t;

/**
 * @param  size     The current number of bytes in the hash table.
 * @param  minsize  The smallest number of entries allowable. This is used for
 *                  resizing.
 * @param  length   The number of entries in the hash table.
 * @param  bins     Pointer to the hash table bins of type bin_t.
 */
typedef struct Hashmap {
    size_t size, minsize;
    size_t length;
    bin_t *bins;
} hashmap_t;

/**
 * Compute the hash of a given bytestring.
 */
size_t get_hash(const void *key, size_t keylen);

/**
 * Lookup the key. If it exists, return a pointer to the table entry, otherwise
 * return NULL.
 */
void *hashmap_find(hashmap_t *map, const void *key, size_t keylen);

/**
 * Insert a new table entry. If this key already exists, update the value.
 * The key and value are copied, meaning they can be freed after insertion.
 */
void *hashmap_insert(hashmap_t *map, const void *key, size_t keylen,
                     const void *value);

/**
 * Remove a table entry, freeing its memory.
 */
void *hashmap_delete(hashmap_t *map, const void *key, size_t keylen);

/**
 * Initialize memory needed for the hashmap.
 */
hashmap_t *hashmap_init(size_t size);

/**
 * Free all memory associated with the hash table.
 */
void hashmap_free(hashmap_t *map);

#endif
