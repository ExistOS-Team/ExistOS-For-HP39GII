// #define MEMORY_MIN 0x20000
// #define MEMORY_MAX 0x70000
// #define MEMORY_BLOCK_INFO_SIZE 2+8+4

// typedef struct _MemoryBlockInfo{
//     unsigned short isAvailable;
//     unsigned long size;
//     struct _MemoryBlockInfo *pointer; // If this is the first MemoryBlockInfo or isAvailable=1, pointer points to the next availavle MemoryBlokck; else, it points to the previous block.
// } MemoryBlockInfo;

#include "memory.h"

void malloc_init(){
    MemoryBlockInfo * p;
    p = MEMORY_MIN;
    p->isAvailable = 1;
    p->size = MEMORY_MAX - MEMORY_MIN - MEMORY_BLOCK_INFO_SIZE;
    p->pointer = 0x0;
}

void *malloc(unsigned long size){
    MemoryBlockInfo *previous = 0x0, // The prevoius available block
        *p = MEMORY_MIN, // The present available block
        *rest, // The available block newly created in the rest space of p
        *next; // The next available block
    long restSize;

    // We call the first block and all the available blocks "the list".
    while (p->size < size && p->isAvailable == 1) { // Find the first available block in the list. "p->isAvailable == 1" will be evaluated no more than twice.
        previous = p;
        p = p->pointer;
        if (p == 0x0) { // No block available
            return 0x0;
        }
    }

    restSize = p->size - size - MEMORY_BLOCK_INFO_SIZE;

    if (restSize > 0) { // If we can create another block in the rest space
        rest = p + MEMORY_BLOCK_INFO_SIZE + size;
        rest->isAvailable = 1;
        rest->size = restSize;
        rest->pointer = p->pointer; // Points to its next available block
        next = rest + MEMORY_BLOCK_INFO_SIZE + restSize; //p + MEMORY_BLOCK_INFO_SIZE + p->size;
        next->pointer = rest; // Points to its prevoius block
        previous->pointer = rest; // Points to "rest"
    }else{
        previous->pointer = p->pointer; // Points to its next available block
    }

    p->isAvailable = 0;
    p->size = size;
    p->pointer = p == MEMORY_MIN ? rest : previous; // Points to its next available block (being the first block) or its prevoius block （beinga normal unavailable block)

    return p + MEMORY_BLOCK_INFO_SIZE;
}

void free(void *ptr){
    MemoryBlockInfo *p = ptr, // The prevoius block
        *head, // The head of the new available block (included)
        *tail, // The tail of the new available block (not included)
        *next; // The next block of p
    p -= MEMORY_BLOCK_INFO_SIZE;
    if (p >= MEMORY_MIN && p < MEMORY_MAX && !p->isAvailable) {
        head = p != MEMORY_MIN && p->pointer->isAvailable ? p->pointer : p;
        next = p + MEMORY_BLOCK_INFO_SIZE + p->size;
        tail = next < MEMORY_MAX && next->isAvailable ? next + MEMORY_BLOCK_INFO_SIZE + next->size : next;
        head->isAvailable = 1;
        head->size = tail - head - MEMORY_BLOCK_INFO_SIZE;
        p = MEMORY_MIN;
        while (p->pointer < tail && p->pointer != 0x0) {
            p = p->pointer;
        }
        head->pointer = p->pointer;
        p->pointer = head;
    }else{
        // Throw an error
    }
}