/* The following provokes a linker invocation problem with gcc 3.0.3
   on AIX 4.3 under "-maix64 -mpowerpc64 -mcpu=630".  The -mcpu=630
   option causes gcc to incorrectly select the 32-bit libgcc.a, not
   the 64-bit one, and consequently it misses out on the __fixunsdfdi
   helper (double -> uint64 conversion).  */
double d;
unsigned long gcc303 () { return d; }

int main () { return 0; }
