#pragma once


#define DLL_TABLE_ADD(x)   dll_table_add(#x, (void *)x)

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>


int dll_table_add(const char *fname, void *func);
void *dll_table_find(char *fname);

