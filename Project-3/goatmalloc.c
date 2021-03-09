#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include "goatmalloc.h"

// global variables
void *_arena_start = NULL;
size_t mapped_bytes_size=0;
node_t *freeList = NULL;
int statusno=0;

// function prototypes
void add_block(size_t size);
node_t *getFreeBlock();
node_t *split_block(node_t *block, size_t size);
void coalesce(node_t **block);

/* --------------------------------------------------------------------------*/
/**
 * @Brief initialize a block of memory using mmap
 *
 * @Param size how big the arena size should be
 *
 * @Returns number of bytes mapped
 */
/* --------------------------------------------------------------------------*/
int init(size_t size) {

  puts("Initializing arena:");
  mapped_bytes_size=size; // initialized mapped bytes to initial value

  if(size>MAX_ARENA_SIZE) { // check to see if requested size is valid
    printf("...requested size %lu bytes\n", size);
    puts("...error: requested size larger than MAX_ARENA_SIZE (2147483647)");
    return ERR_BAD_ARGUMENTS;
  }

  printf("...requested size %lu bytes\n", size);
  puts("...adjusting size with page boundaries");

  int mult = (int)ceil((float)size / 4096); // find correct size of arena
  size_t mapped_bytes_size = getpagesize() * mult;

  printf("...adjusted size is %lu bytes\n", mapped_bytes_size);
  puts("...mapping arena with mmap()");

  int fd=open("/dev/zero", O_RDWR);
  _arena_start = mmap(NULL, mapped_bytes_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  freeList = _arena_start;
  add_block(mapped_bytes_size);

  if(_arena_start == MAP_FAILED) {
    perror("ERROR:");
  }

  printf("...arena start at %p\n", _arena_start);
  printf("...arena ends at %p\n", _arena_start + mapped_bytes_size);

  puts("...initializing header for initial free chunk");
  printf("...header size is %lu bytes\n", sizeof(node_t));

  return mapped_bytes_size;

}

/* --------------------------------------------------------------------------*/
/**
 * @Brief free arena
 *
 * @Returns number of bytes freed
 */
/* --------------------------------------------------------------------------*/
int destroy() {

  puts("Destroying Arena:");
  puts("...unmapping arena with munmap()");

  if(mapped_bytes_size>MAX_ARENA_SIZE) {
    puts("...error: cannot destroy uninitialized arena. Setting error status");
    return ERR_UNINITIALIZED;
  }


  int deallocate = munmap(_arena_start, sizeof(mapped_bytes_size));

  mapped_bytes_size=0;
  freeList=NULL;

  return deallocate;

}

/* --------------------------------------------------------------------------*/
/**
 * @Brief Malloc(); Allocated size bytes in arena
 *
 * @Param size bytes to allocate
 *
 * @Returns pointer to block
 */
/* --------------------------------------------------------------------------*/
void *walloc(size_t size) {

  if(mapped_bytes_size>MAX_ARENA_SIZE || mapped_bytes_size==0) {

    puts("...error: cannot destroy uninitialized arena. Setting error status");
    statusno=ERR_UNINITIALIZED;
    return NULL;
  
  } 

  node_t *block = getFreeBlock();
  puts("Allocating memory:");
  printf("...looking for free chunk of >= %lu\n", size);
  
  if(block!=NULL && size<mapped_bytes_size) { // allocate if there is a free block and size isn't greater than arena
    block->is_free = 0;
    printf("...found free chunk of %lu bytes with header at %p\n", block->size, block);
    printf("...free chunk->fwd currently points to %p\n", block->fwd);
    printf("...free chunk->bwd currently points to %p\n", block->bwd);
    puts("...checking if splitting is required");

    if(size<block->size - sizeof(node_t)) { // check to see if it needs to be split
      block = split_block(block, size);

      if(block==NULL) {
        puts("...splitting not possible");
        return NULL;
      }

      puts("...splitting free chunk");

    } else {
      puts("...splitting not required");
    }

    printf("...updating chunk header at %p\n", block);
    puts("...being careful with my pointer arithmetic and void pointer casting");
    // make it not free
    printf("...allocation starts at %p\n", (void*)block + sizeof(node_t));  

  } else {

    puts("...no such free chunk exists");
    puts("...setting error code");
    
    if(size>=mapped_bytes_size) { // set statusno if request is greater than arena size
      statusno=ERR_OUT_OF_MEMORY;
    } 
      return NULL;
  }

  return (void*)block + sizeof(node_t);

}

/* --------------------------------------------------------------------------*/
/**
 * @Brief free memory
 *
 * @Param ptr pointer to memory
 */
/* --------------------------------------------------------------------------*/
void wfree(void *ptr) {

  puts("Freeing allocated memory:");

  printf("...supplied pointer %p\n", ptr);
  puts("...being careful with my pointer arthimetic and void pointer casting");
  node_t *header = (node_t*)(ptr - sizeof(node_t)); // manipulate pointer to access header 

  printf("...accessing chunk header at %p\n", header);
  header->is_free = 1;
  printf("...chunk of size %lu\n", header->size);
  puts("...checking if coalescing is needed");

  coalesce(&header); // check to see if blocks of memory can be coalesced
}

/* --------------------------------------------------------------------------*/
/**
 * @Brief helper function to add chunk/block to chunk list
 *
 * @Param size number of bytes for the block
 */
/* --------------------------------------------------------------------------*/
void add_block(size_t size) {

  node_t *head = freeList;

  if(head->fwd==NULL && head->bwd==NULL) { // chunk list has no blocks of memory
    head->size = size - sizeof(node_t);
    head->is_free = 1;
    head->bwd = NULL;
    head->fwd = NULL;
    return;
  } 

  // add to populated chunk list
  node_t *cur = head;
  while(cur->fwd!=NULL) {
    cur = cur->fwd;
  }

  node_t *prev = cur;
  cur->fwd = (node_t*)((void*)cur + cur->size + sizeof(node_t));
  cur = cur->fwd;
  cur->size = size;
  cur->bwd = prev;
  cur->is_free = 1;
  cur->fwd = NULL;
  
}

/* --------------------------------------------------------------------------*/
/**
 * @Brief split a block of memory if possible
 *
 * @Param block pointer to block
 * @Param size number of bytes to cut from main block
 *
 * @Returns pointer to block
 */
/* --------------------------------------------------------------------------*/
node_t *split_block(node_t *block, size_t size) {

  node_t *head = block;
  node_t *cur = block;
  size_t adjSize = cur->size - size;
  cur->size = size;

  if(adjSize<=1) { // only if new block is size 1 byte or 0; cannot support header
    return NULL;
  }

  if(cur->fwd==NULL) { // check to see if you are at the end of the chunk list
    cur->fwd = (node_t*)((void*)block + size + sizeof(node_t));
    cur = cur->fwd;
    cur->fwd = NULL;
  } else {
    node_t *next = cur->fwd;
    cur->fwd = (node_t*)((void*)block + size + sizeof(node_t));
    cur = cur->fwd;
    cur->fwd = next;
    next->bwd = cur;

  }

  cur->size = adjSize - sizeof(node_t);
  cur->bwd = head;
  cur->is_free = 1;
  return head;
}

/* --------------------------------------------------------------------------*/
/**
 * @Brief find a free block in the chunk list
 *
 * @Returns pointer to free block
 */
/* --------------------------------------------------------------------------*/
node_t *getFreeBlock() {
  node_t *cur = freeList;

  size_t occupiedSpace = 0;

  while(cur!=NULL) { // go through the chunk list
    if(cur->is_free==1) { // found free block
      cur->fwd = NULL;
      return cur;
    } else {
      occupiedSpace+=cur->size;
      
      if(occupiedSpace>=mapped_bytes_size) { // just to keep track of the number of bytes seen
        break;
      }
    }

    cur = cur->fwd;
  }

  return NULL;
}

/* --------------------------------------------------------------------------*/
/**
 * @Brief combine free blocks of memory
 *
 * @Param block initial block of free memory
 */
/* --------------------------------------------------------------------------*/
void coalesce(node_t **block) {
  node_t *head = *block;
  node_t *cur = head;
  node_t *next = head->fwd;

  bool prevBlock = false, nextBlock = false; // variables to check if there were any blocks of free memory on the left or right
  int numFreeBlocks = 0;
  size_t size = head->size;

  if(head->bwd!=NULL) { // combine adjacent blocks on the left
    node_t *curRef = head->bwd;
    while(curRef!=NULL) {
      if(curRef->is_free==0) {
        break;
      }

      numFreeBlocks++;

      if(curRef->bwd==NULL) {
        cur = curRef;
      }

      size += curRef->size;
      curRef = curRef->bwd;

    }
    prevBlock = true;

  }

  if(head->fwd!=NULL) { // combine adjacent blocks on the right
    node_t *curRef = head->fwd;
    while(curRef!=NULL) {
      if(curRef->is_free==0) {
        break;
      }
      numFreeBlocks++;

      size += curRef->size; 
      curRef = curRef->fwd;
    }
    next = curRef;
    nextBlock = true;
  }

  if(numFreeBlocks>0) { // if any blocks were freed then print out message and fill information for header
    cur->size = getpagesize() - sizeof(node_t);
    cur->is_free = 1;
    cur->fwd = next;

  }

  if(prevBlock&&!nextBlock) { // case 2
    puts("...col. case 2: previous and current chunks free.");
  } else if(!prevBlock && nextBlock) { // case 3
    puts("...col. case 3: current and next chunks free.");
  } else if(prevBlock && nextBlock) { // case 1
    puts("...col. case 1: previous, current, and next chunks all free.");
  } else {
    puts("...coalescing not needed.");  
  }
}

