#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "fxlib.h"

#define DEBUG 0

void debug_init(void);
void debug(const char *format, ...);
void debug_quit(void);
