#include <stdlib.h>
#include <stdio.h>

// See the C++ standard, section 3.7.4 for those definitions

void * operator new(size_t size) { 
  return malloc(size);
  //printf("malloc:%d\n", size);
}

void operator delete(void * ptr) noexcept {
  free(ptr);
}

void * operator new[](size_t size) {
    return ::operator new(size);
}

void operator delete[](void * ptr) noexcept {
    ::operator delete(ptr);
}

