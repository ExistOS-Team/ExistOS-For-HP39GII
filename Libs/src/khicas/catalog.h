#ifndef __CATALOGGUI_H
#define __CATALOGGUI_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

extern int lang; // 0 english, 1 francais

typedef struct {
  char* name;
  char* insert;
  char* desc;
  char * example;
  char * example2;
  int category;
} catalogFunc;

int showCatalog(char* insertText,int preselect=0,int menupos=0);
int doCatalogMenu(char* insertText, char* title, int category,const char * cmdname=0);
extern const char aide_khicas_string[];
extern const char shortcuts_string[];
extern const char apropos_string[];
//const char * unary_function_ptr_name(void * ptr); // in main.cpp

#endif
