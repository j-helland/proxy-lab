/**
 * @author Jonathan Helland
 * 
 * A hash table implementation using Robin Hood hashing to resolve collisions
 * efficiently while trying to maintain locality of previously inserted elements.
 *
 * Inspired by the following Robin Hood Hashmap library:
 * https://github.com/rmind/rhashmap
 *
 * The hashing function used is adapted from the djb2 hash function: 
 * http://www.cse.yorku.ca/~oz/hash.html.
 */
#include "hashmap.h"

#include <limits.h> // UINT_MAX
#include <stdbool.h>
#include <stdint.h> // uintptr_t
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // memcmp

#define HASHMAP_MAX (UINT_MAX)
#define HASHMAP_MAX_GROWTH_STEP (1024U * 1024)

/**
 * @brief Compute approximately 85% of the input value.
 *
 * Used for upsizing the hash table -- when more than 85% of the memory is in
 * use, we increase size.
 */
static inline size_t approx_85_percent(size_t x) {
    return (x * 870) >> 10;
}

/**
 * @brief Compute approximately 40% of the input value.
 *
 * Used for downsizing the hash table -- when less than 40% of the memory is in
 * use, we reduce size.
 */
static inline size_t approx_40_percent(size_t x) {
    return (x * 409) >> 10;
}

/**
 * @brief Compute the min of two nonnegative integers.
 */
static inline size_t min(size_t a, size_t b) {
    return (a > b) ? b : a;
}

/**
 * @brief Compute the max of two nonnegative integers.
 */
static inline size_t max(size_t a, size_t b) {
    return (a > b) ? a : b;
}

/**
 * @brief Compute the hash of an input bytestring.
 *
 * Based on the djb2 hash function:
 * http://www.cse.yorku.ca/~oz/hash.html.
 *
 * All sorts of magic numbers abound -- welcome to hashing.
 *
 * @param  key     Pointer to the bytestring to hash.
 * @param  keylen  Number of bytes that should be hashed.
 * @return Hashed size_t value that can be reduced to a bin index.
 */
size_t get_hash(const void *key, size_t keylen) {
    size_t hash = 5381;
    const char *str = (char *)key;
    for (size_t i = 0; i < keylen; ++i)
        hash = ((hash << 5) + hash) + (int)(*str++); // hashed * 33 + key byte
    return hash;
}

/**
 * @brief Lookup a table entry.
 *
 * @param  map     Pointer to the hashmap to search.
 * @param  key     Pointer to the bytestring to hash into an index.
 * @param  keylen  Number of bytes in the key to hash.
 * @return Pointer to the table entry if it exists. NULL otherwise.
 */
void *hashmap_find(hashmap_t *map, const void *key, size_t keylen) {
    const size_t hash = get_hash(key, keylen);
    bin_t *bin;

    for (size_t n = 0, i = hash % map->size;; ++n, i = (i + 1) % map->size) {
        bin = &map->bins[i];
        if (bin->hash == hash && bin->keylen == keylen &&
            memcmp(bin->key, key, keylen) == 0)
            return bin->value;

        if (bin->key == NULL || n > bin->psl)
            return NULL;
    }
}

/**
 * @brief Add a new table entry. Uses the Robin Hood displacement policy to
 * resolve collisions. That is, entries with small PSLs tend to be displaced in
 * favor of large PSL entries.
 *
 * Trying to insert a duplicate will result in simply updating the existing
 * value.
 *
 * @warning Memory is not copied, so freeing after insertion can be dangerous.
 *
 * @param  map     Pointer to hashmap to insert into.
 * @param  key     Pointer to bytestring to hash into an index.
 * @param  keylen  Number of bytes of key to hash.
 * @param  value   Value that will be associated with the key.
 *
 * @return Pointer to the newly inserted value (copied).
 */
static void *hashmap_insert_no_resize(hashmap_t *map, const void *key,
                                      size_t keylen, const void *value) {
    const size_t hash = get_hash(key, keylen);
    bin_t *bin, entry;

    entry.key = (void *)(uintptr_t)key;
    entry.hash = hash;
    entry.keylen = keylen;
    entry.value = (void *)(uintptr_t)value;
    entry.psl = 0;

    // Handle collisions.
    // If the PSL (probe sequence length) of the element to insert is greater
    // than the PSL of the element in the bin, then swap them and continue.
    for (size_t i = hash % map->size;; i = (i + 1) % map->size) {
        bin = &map->bins[i];

        // The bin contains a key.
        if (bin->key) {
            // Check if the bin is a duplicate of the one we're trying to
            // insert. If so, just set and return its value.
            if (bin->hash == hash && bin->keylen == keylen &&
                memcmp(bin->key, key, keylen) == 0) {
                bin->value = (void *)value;
                return bin->value;
            }

            // Handle relatively rich bins. Rich bins are those with a small PSL
            // (probe sequence length).
            if (entry.psl > bin->psl) {
                // Swap the rich bin with this bin.
                bin_t tmp = entry;
                entry = *bin;
                *bin = tmp;
            }
            entry.psl++;

        } else
            // When the bin is empty, we can insert something, so break out of
            // collision handling.
            break;
    }

    // Insertion step. We've located a valid bin for insertion.
    *bin = entry;
    map->length++;

    return (void *)value;
}

/**
 * @brief  Resize the hashtable to a new number of bytes.
 */
static int hashmap_resize(hashmap_t *map, size_t size) {
    size_t size_old = map->size;
    bin_t *bins_old = map->bins;
    bin_t *bins = NULL;

    if (size > HASHMAP_MAX)
        return -1;
    else if ((bins = calloc(size, sizeof(bin_t))) == NULL)
        return -1;

    map->bins = bins;
    map->size = size;
    map->length = 0;

    // Need to preserve PSLs, so re-insert entries.
    for (size_t i = 0; i < size_old; ++i) {
        const bin_t *bin = &bins_old[i];

        if (bin->key == NULL)
            continue;

        hashmap_insert_no_resize(map, bin->key, bin->keylen, bin->value);
    }

    if (bins_old)
        free(bins_old);

    return 0;
}

/**
 * @brief Add a new table entry. Uses the Robin Hood displacement policy to
 * resolve collisions. That is, entries with small PSLs tend to be displaced in
 * favor of large PSL entries.
 *
 * Trying to insert a duplicate will result in simply updating the existing
 * value.
 *
 * The table will automatically increase in size if the new entry causes at
 * least 85% of the table size to be consumed.
 *
 * @warning Memory is not copied, so freeing after insertion can be dangerous.
 *
 * @param  map     Pointer to hashmap to insert into.
 * @param  key     Pointer to bytestring to hash into an index.
 * @param  keylen  Number of bytes of key to hash.
 * @param  value   Value that will be associated with the key.
 *
 * @return Pointer to the newly inserted value (copied).
 */
void *hashmap_insert(hashmap_t *map, const void *key, size_t keylen,
                     const void *value) {
    const size_t threshold = approx_85_percent(map->size);
    if (map->length > threshold) {
        const size_t grow_limit = map->size + HASHMAP_MAX_GROWTH_STEP;
        const size_t size = min(map->size << 1, grow_limit);
        if (hashmap_resize(map, size) != 0)
            return NULL;
    }
    return hashmap_insert_no_resize(map, key, keylen, value);
}

/**
 * @brief Remove an entry from the table.
 *
 * The table will automatically decrease in size if removing this entry causes
 * at most 40% of the table size to be consumed.
 *
 * @warning Does not free any memory, you must do this yourself.
 */
void *hashmap_delete(hashmap_t *map, const void *key, size_t keylen) {
    const size_t threshold = approx_40_percent(map->size);
    const size_t hash = get_hash(key, keylen);
    bin_t *bin;
    void *value;

    size_t i = hash % map->size;
    for (size_t n = 0;; ++n, i = (i + 1) % map->size) {
        bin = &map->bins[i];
        if (bin->key == NULL || n > bin->psl)
            return NULL;

        // Bin has been found.
        if (bin->hash == hash && bin->keylen == keylen &&
            memcmp(bin->key, key, keylen) == 0)
            break;
    }

    // Remove the located bin.
    value = bin->value;
    map->length--;

    // Maintain probe sequence using backwards shifting method.
    while (1) {
        bin_t *nbin;
        bin->key = NULL;
        bin->keylen = 0;

        i = (i + 1) % map->size;
        nbin = &map->bins[i];

        // Halt if we find an empty bin or hit a key that's in it's original
        // position.
        if (nbin->key == NULL || nbin->psl == 0)
            break;

        nbin->psl--;
        *bin = *nbin;
        bin = nbin;
    }

    if (map->length > map->minsize && map->length < threshold) {
        size_t size = max(map->size >> 1, map->minsize);
        hashmap_resize(map, size);
    }
    return value;
}

/**
 * @brief Allocate memory for the hash table according to the desired size.
 *
 * This hash table must be freed by calling hashmap_free.
 */
hashmap_t *hashmap_init(size_t size) {
    hashmap_t *map = calloc(1, sizeof(hashmap_t));
    if (map == NULL)
        return NULL;

    map->minsize = max(size, 1);
    if (hashmap_resize(map, map->minsize) != 0) {
        free(map);
        return NULL;
    }

    return map;
}

/**
 * @brief Free the hash table's memory.
 *
 * @warning This does not free the memory of the entries themselves, you must do
 *          this yourself.
 */
void hashmap_free(hashmap_t *map) {
    free(map->bins);
    free(map);
}
