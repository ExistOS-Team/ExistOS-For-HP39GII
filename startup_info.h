#ifndef STARTUP_INFO_H
#define STARTUP_INFO_H
#include <string.h>

unsigned int swapSizeMB;

void *malloc_PF( size_t xWantedSize );

void free_PF( void* xWantedSize );

#endif
