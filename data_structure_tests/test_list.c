#ifndef TEST_LIST_C
#define TEST_LIST_C

#include "list.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int run_test_list(void) {
   printf("Testing list...\n");

   list_t *list = list_init(); 
   size_t a = 0, b = 1, c = 2, d = 3;
   node_t *n1, *n2, *n3, *n4;

   n1 = list_insert(list, &a);
   n2 = list_insert(list, &b);
   n3 = list_insert(list, &c);
   n4 = list_insert(list, &d);
   assert(list->length == 4);
   printf("\tinsertion OK\n");

   node_t *n = list_find(list, &a);
   assert( n == n1 );
   printf("\tfind OK\n");

   list_delete(list, n3);
   assert(list->length == 3);
   assert(list_find(list, n3) == NULL);
   bool looped = false;
   for (node_t *n = list->head; !looped && n != NULL;) {
       assert(n != n3);

       n = n->next;
       looped = (n == list->head);
   }
   free(n3);
   printf("\tdeletion OK\n");

   list_free(list);
   printf("test_list OK\n");
   return EXIT_SUCCESS;
}

#endif
