#ifndef TEST_HASHMAP_C
#define TEST_HASHMAP_C

#include "hashmap.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#define NUM_CHARS (26)
#define NUM_ITEMS (NUM_CHARS * 2)

int run_test_hashmap(void) {
    printf("Testing hashmap...\n");

    hashmap_t *map = hashmap_init(16);
    hashmap_free(map);
    printf("\tinit OK\n");

    map = hashmap_init(1);
    void *a, *b, *c, *d;
    const char *k1 = "aa", *k2 = "ab", *k3 = "ac", *k4 = "ad";

    hashmap_insert(map, k1, strlen(k1), a);
    hashmap_insert(map, k2, strlen(k2), b);
    hashmap_insert(map, k3, strlen(k3), c);
    hashmap_insert(map, k4, strlen(k4), d);
    assert( map->length == 4 );
    printf("\tinsertion OK\n");

    const char *val = hashmap_find(map, k3, strlen(k3));
    assert( val == c );
    printf("\tfind OK\n");

    hashmap_insert(map, k3, strlen(k3), d);
    val = hashmap_find(map, k3, strlen(k3));
    assert( val == d );
    printf("\tvalue change OK\n");

    hashmap_delete(map, k3, strlen(k3));
    assert( map->length == 3 );
    assert( hashmap_find(map, k3, strlen(k3)) == NULL );
    assert( hashmap_delete(map, k3, strlen(k3)) == NULL );
    printf("\tdeletion OK\n");

    // Collision handling and resizing. 
    // Generate lots of insertions to cause collisions and force resizing.
    hashmap_free(map);
    map = hashmap_init(1);
    size_t idx = 0;
    char key[] = "aa";
    assert( strlen(key) == NUM_ITEMS / NUM_CHARS );
    char *keys[NUM_ITEMS];
    size_t *vals[NUM_ITEMS];
    for (size_t i = 0; i < NUM_ITEMS; ++i, ++key[idx]) {
        if (((i + 1) % NUM_CHARS == 0))
            idx++;

        keys[i] = malloc(3);
        strncpy(keys[i], key, 3);

        vals[i] = malloc(sizeof(size_t));
        *vals[i] = i;
        hashmap_insert(map, keys[i], strlen(keys[i]), vals[i]);

        assert( hashmap_find(map, key, strlen(key)) == vals[i] );
    }
    assert( map->length == NUM_ITEMS );
    for (size_t i = 0; i < map->length; ++i) {
        free(keys[i]);
        free(vals[i]);
    }
    hashmap_free(map);
    printf("\tcollisions and resizing OK\n");

    printf("test_hashmap OK\n");
    return EXIT_SUCCESS;
}

#endif
