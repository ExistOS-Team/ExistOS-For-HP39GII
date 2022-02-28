#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdarg.h>
#include <stdio.h>

#define PANNIC(...)  do{printf(__VA_ARGS__);while(1);}while(0)


#define INFO(...) do{printf(__VA_ARGS__);}while(0)



//#define MTD_INFO(...) do{printf(__VA_ARGS__);}while(0)
#define MTD_INFO(...) 
#define MTD_WARN(...) do{printf(__VA_ARGS__);}while(0)

#define FTL_INFO(...) 
#define FTL_WARN(...) do{printf(__VA_ARGS__);}while(0)

#define VM_BKPT()  __asm volatile (".int 0xF7BBBBBB")


#define VM_INFO(...) do{printf(__VA_ARGS__);}while(0)
//#define VM_INFO(...) 
#define VM_ERR(...) do{printf(__VA_ARGS__);}while(0)



void _randSetSeed(uint32_t seed);
uint32_t _rand(void);

#endif