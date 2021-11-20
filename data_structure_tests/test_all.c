#ifndef TEST_ALL_C
#define TEST_ALL_C

#include "test_list.c"
#include "test_hashmap.c"
#include "test_cache.c"

#include <assert.h>
#include <stdio.h>

int main(void) {
    assert(run_test_cache() == EXIT_SUCCESS);
    printf("\n");

    assert( run_test_hashmap() == EXIT_SUCCESS);
    printf("\n");

    assert( run_test_list() == EXIT_SUCCESS );
    printf("\n");
}

#endif
