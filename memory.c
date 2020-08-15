#include "memory.h"
#include "uart_debug.h"

void malloc_init() {
    MemoryBlockInfo* p;
    p = (MemoryBlockInfo*)MEM_MIN;
    p->isAvailable = 1;
    p->size = MEM_MAX - MEM_MIN - MEM_BLOCK_INFO_SIZE;
    p->pointer = 0x0;
    //printf("Malloc: Initialized. MEM_MIN=0x%x, MEM_MAX=0x%x, MEM_BLOCK_INFO_SIZE=0x%x.\n", MEM_MIN, MEM_MAX, MEM_BLOCK_INFO_SIZE);
    //printf("Malloc: New available block created. Address=0x%x, isAvailable=%d, size=0x%x, pointer=0x%x.\n", p, p->isAvailable, p->size, p->pointer);
}

void* malloc(unsigned int size) {
    //printf("Malloc: Preparing to alloc.\n");
    MemoryBlockInfo* previous = 0x0, // The previous available block
        * p = (MemoryBlockInfo*)MEM_MIN, // The present available block
        * rest, // The available block newly created in the rest space of p
        * next; // The next available block
    unsigned int restSize;

    // We call the first block and all the available blocks "the list".
    while (p->size < size || !p->isAvailable) { // Find the first available block in the list.  "p->isAvailable == 1" will be evaluated no more than twice.
        previous = p;
        p = p->pointer;
        if (p == 0x0) { // No block available
            //printf("Malloc: Block not found. Alloc aborted.\n");
            return 0x0;
        }
    }
    //printf("Malloc: Available block found. Address=0x%x, isAvailable=%d, size=0x%x, pointer=0x%x.\n", p, p->isAvailable, p->size, p->pointer);

    p->pointer = 0x0; // Default
    
    if (p->size > size + MEM_BLOCK_INFO_SIZE) { // If we can create another block in the rest space
        restSize = p->size - size - MEM_BLOCK_INFO_SIZE;
        rest = (MemoryBlockInfo *)((char *)p + MEM_BLOCK_INFO_SIZE + size);
        rest->isAvailable = 1;
        rest->size = restSize;
        rest->pointer = p->pointer; // Points to its next available block

        if (previous == 0x0) { // Or "p == (MemoryBlockInfo*)MEM_MIN"
            p->pointer = rest; // p -> rest -> (p->pointer) -> ...
        }else{
            previous->pointer = rest;  // ... -> previous -> rest -> (p->pointer) -> ...
        }

        next = (MemoryBlockInfo*)((char*)rest + MEM_BLOCK_INFO_SIZE + restSize); //p + MEM_BLOCK_INFO_SIZE + p->size;
        if (next < (MemoryBlockInfo*)(MEM_MAX - MEM_BLOCK_INFO_SIZE)) {
            next->pointer = rest; // Surely next->isAvailable==0. Points to its previous block, rest.
        }

        p->size = size;
        //printf("Malloc: New available block created. Address=0x%x, isAvailable=%d, size=0x%x, pointer=0x%x.\n", rest, rest->isAvailable, rest->size, rest->pointer);
    }
    else {
        // Give the applier size+rest (p->size).
        previous->pointer = p->pointer; // Points to its next available block
        //printf("Malloc: Rest space not enough. No new available block created.\n");
    }

    p->isAvailable = 0;
    //printf("Malloc: Space allocated. Address=0x%x, isAvailable=%d, size=0x%x, pointer=0x%x.\n", p, p->isAvailable, p->size, p->pointer);
    //printf("Malloc: Alloc finished.\n");
    return p + 1; //(char *)p + MEM_BLOCK_INFO_SIZE;
}

void free(void* ptr) {
    //printf("Malloc: Preparing to free.\n");
    MemoryBlockInfo* p = (MemoryBlockInfo*)ptr - 1, // The present block
        * head, // The head of the new available block (included)
        * tail, // The tail of the new available block (not included), also the head of the next block
        * next; // The next block of p

    if (p >= (MemoryBlockInfo*)MEM_MIN && p < (MemoryBlockInfo*)(MEM_MAX - MEM_BLOCK_INFO_SIZE) && !p->isAvailable) {
        head = p->pointer != 0x0 && p->pointer->isAvailable && p != (MemoryBlockInfo*)MEM_MIN ? p->pointer : p;
        next = (MemoryBlockInfo*)((char*)p + MEM_BLOCK_INFO_SIZE + p->size);
        tail = next < (MemoryBlockInfo*)MEM_MAX && next->isAvailable ? (MemoryBlockInfo*)((char*)next + MEM_BLOCK_INFO_SIZE + next->size) : next;
        head->isAvailable = 1;
        head->size = (unsigned int)tail - (unsigned int)head - MEM_BLOCK_INFO_SIZE;
        if (tail < (MemoryBlockInfo*)MEM_MAX) {
            tail->pointer = head; // Surely tail->isAvailable==0. Points to its previous block, head.
        }

        p = (MemoryBlockInfo*)MEM_MIN; // The previous available block
        while (p->pointer < head && p->pointer != 0x0) {
            p = p->pointer;
        }

        next = p->pointer; // The next available block
        while (next < tail && next != 0x0) {
            next = next->pointer;
        }

        p->pointer = head;
        head->pointer = next;
        //printf("Malloc: New available block created. Address=0x%x, isAvailable=%d, size=0x%x, pointer=0x%x.\n", head, head->isAvailable, head->size, head->pointer);
        //printf("Malloc: Free finished.\n");
    }
    else {
        //printf("Malloc: Warning: Trying to free an non-existent block. Address=0x%x, isAvailable=%d, size=0x%x, pointer=0x%x.\n", p, p->isAvailable, p->size, p->pointer);
        //printf("Malloc: Free aborted.\n");
        // Throw an error
        uartdbg_printf("[Malloc]Warning: Freeing aborted.");
    }
}