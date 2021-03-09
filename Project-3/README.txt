Tae Hyun Je
Project 3 - Memory Allocation
CS3013 - Operating Systems

This project focused on the implementation of memory allocation. 
- init(): This function was used to great an arena for walloc()
- walloc(): This was the malloc() implementation to allocated memory in the arena.
- wfree(): This was the free() implememntation to free allocated memory.
- destroy(): This function released the arena

All main functions used the global variables to find, allocate, free, and verify that allocation or de-allocation was possible. They were used to keep track 
of a chunk list, size of the arena, and reference pointer to the arena.
I created various helper functions to aid the main functions, such as getFreeBlock(), coalesce(), etc. I added Doxygen and comments to explain the code.

