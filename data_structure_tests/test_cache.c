#ifndef TEST_CACHE_C
#define TEST_CACHE_C

#include "cache.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CACHE_SIZE (64)
#define BLOCK_SIZE (32)

int run_test_cache(void) {
    printf("Testing cache...\n");

    cache_t *cache = cache_init(16);
    cache_free(cache);
    printf("\tinit OK\n");

    pthread_cond_t cond;
    pthread_mutex_t mut;

    cache = cache_init(16);
    char *mem = malloc(16);
    int res = cache_insert(cache, "abc", 4, mem, 17, &cond, &mut);
    assert(res == -1);
    cache_insert(cache, "abc", 4, mem, 16, &cond, &mut);
    assert(cache->size == 16);
    cache_insert(cache, "cba", 4, mem, 16, &cond, &mut);
    assert(cache->size == 16);
    assert(cache_find(cache, "abc", 4, &mut) == NULL);
    printf("\tinsert OK\n");

    block_t *block = cache_find(cache, "cba", 4, &mut);
    assert(memcmp(block->value, mem, 16) == 0);
    assert(block->refcount == 1);
    cache_release(cache, "cba", 4, &cond);
    assert(block->refcount == 0);
    cache_delete(cache, block);
    assert(cache->size == 0);
    printf("\tdelete OK\n");

    cache_free(cache);
    free(mem);

    cache = cache_init(CACHE_SIZE);
    const size_t str_size = 4;
    const size_t size = CACHE_SIZE / BLOCK_SIZE / str_size;
    char *mem2[size + 10];
    char *keys[size + 10];
    for (size_t i = 0; i < size + 10; ++i) {
        char *key = malloc(2);
        key[0] = 'a' + i;
        key[1] = '\0';
        keys[i] = key;

        mem2[i] = malloc(10);
        strncpy(mem2[i], "aaa", sizeof("aaa"));

        cache_insert(cache, keys[i], strlen(keys[i]) + 1, mem2[i],
                     10, &cond, &mut);
        printf("cache->size %zu\n", cache->size);
        free(key);
        free(mem2[i]);
    }
    cache_free(cache);
    printf("\tmany insertions OK\n");

    printf("test_cache OK\n");
    return EXIT_SUCCESS;
}

#endif
