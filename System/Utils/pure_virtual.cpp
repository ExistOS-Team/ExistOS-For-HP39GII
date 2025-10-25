extern "C" {
#include <stdlib.h>
#include <stdio.h>
}

/* C++ code calls __cxa_pure_virtual when a pure-virtual method is called.
 * This is an error case, so we just redirect it to abort. */

extern "C" void __cxa_pure_virtual() {
  printf("Unsupported pure-virtual method.\n");
  while(1);
}
