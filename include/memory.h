#ifndef _MEMORY_H
#define _MEMORY_H

#define MEMORY_MIN 0x20000
#define MEMORY_MAX 0x70000
#define MEMORY_BLOCK_INFO_SIZE 2+8+4

typedef struct _MemoryBlockInfo{
    unsigned short isAvailable;
    unsigned long size;
    struct _MemoryBlockInfo *pointer; // If this is the first MemoryBlockInfo or isAvailable=1, pointer points to the next availavle MemoryBlokck; else, it points to the previous block.
} MemoryBlockInfo;

void malloc_init();

void *malloc(unsigned long size);

void free(void *printer);

#endif