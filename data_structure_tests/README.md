Very simple test harness for data structures that I implemented for this project.
Each test is just a sequence of increasingly complex sanity checks that cover my use-cases within the proxy itself.

# Test-driven development process
I found it very useful to take a test-driven development approach to implementing my data structures here so that I could verify that each one behaved independently as expected prior to being hooked up to the proxy.
Moreover, the LRU cache is fundamentally relies on the list and hashmap, so it was important to make sure those components were solid first.

This test-driven approach was extrememly necessary once I introduced multithreading into the proxy -- I didn't have to waste time bug-hunting in the core data structure implementations.