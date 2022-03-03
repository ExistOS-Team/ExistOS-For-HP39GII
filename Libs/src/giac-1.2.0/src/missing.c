#include <sys/times.h>
// missing _times function in libc
clock_t _times(struct tms *ptms){
  return 0;
}

