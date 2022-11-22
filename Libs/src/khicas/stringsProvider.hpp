#ifndef __STRINGSPROVIDER_H
#define __STRINGSPROVIDER_H

#include <fxcg/display.h>
#include <fxcg/file.h>
#include <fxcg/keyboard.h>
#include <fxcg/system.h>
#include <fxcg/misc.h>
#include <fxcg/app.h>
#include <fxcg/serial.h>
#include <fxcg/rtc.h>
#include <fxcg/heap.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


char tolower(char c);
char toupper(char c);
int strncasecmp_duplicate(const char *s1, const char *s2, size_t n);
char *strcasestr_duplicate(const char *s, const char *find);
unsigned char *toksplit(unsigned char *src, char tokchar, unsigned char *token, int lgh);
int EndsIWith(const char *str, const char *suffix);
void* memmem(char* haystack, int hlen, char* needle, int nlen, int matchCase=1);
void stringToMini(char* dest, char* orig);

#endif
