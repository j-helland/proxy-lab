# Multithreaded, caching web proxy in C for CMU 15213

Our instructions for this assignment were to _only_ use mutexes for thread-safety.
We specifically were not allowed to use more sophisticated concurrency primitives like semaphores, which would have simplified the implementation of my multi-reader single-writer lock much simpler.

This was, in my opinion, the hardest assignment in the course, both in terms of the amount of code that needed to be written and the heinous concurrency debugging.

# Documentation

The cache implements an LRU eviction policy and a Robin Hood hashing scheme. 
I decided to implement Robin Hood hashing because I got excited about its following properties:
- The expected value of the probe sequence length for full tables is O(log n), meaning that collisions are handled gracefully and efficiently.
- Cache friendliness of trying to maintain relative proximity of inserted elements despite collisions.

The proxy itself implements a FIFO read/write queue to handle concurrent requests/responses.

### **Organization**
- [`proxy.c`](./proxy.c) contains the main proxy logic.
This file implements all of the multithreading functionality, as well as the request handling, client/server communication, and signal handling.

- The data structures are as follows:
    - [`list.h`](./list.h) is a doubly linked circular list with head insertion that is used to implement an LRU policy in the cache.
    - [`hashmap.h`](./hashmap.h) is a Round Robin hashmap implementation used for the obvious purposes of caching responses to client requests.
    - [`cache.h`](./cache.h) is the LRU cache implementation leveraging both data structures above.
    - [`data_structure_tests/`](./data_structure_tests) provides a very simple test harness for these three data structures.

- There are two required headers that reference code that does not exist in this repo.
These libraries were provided by the course, so I only include the headers here to give a sense of what functionality they provided.
    1. `http_parser.h` is a library for parsing HTTP requests that was provided by the course.
    2. `csapp.h` is a library providing robust, signal-safe I/O that was also provided by the course. More documentation info can be found [here](http://csapp.cs.cmu.edu/2e/ch10-preview.pdf).

### **The single bug that haunts me**
Unfortunately, I didn't have enough time to root out the final concurrency bug, which occurred in two tests out of a total of 52. 
I no longer have access to the course compute environment, so it is unlikely I will ever fix this bug :(

- The core problem is a segmentation fault that occurs when a cache eviction occurs in the middle of a reader accessing a resource.
- The tests where this bug occurs are:
    1. [`failed_tests/D13-multi-evict1.cmd`](./failed_tests/D13-multi-evict1.cmd) which is a concurrent eviction test. Perlexingly, the bug does not occur for `D14-multi-evict2.cmd`.
    2. [`failed_tests/D17-stress.cmd`](./failed_tests/D17-stress.cmd), which is the final concurrency test.
- I'm fairly certain that the bug exists somewhere in the read/write queue implementation.

