#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdarg.h>
#include <stdio.h>

volatile uint32_t isIRQAllowed();

void dbg_printf(char *fmt, ...);

#define PANIC(...)  do{printf(__VA_ARGS__);while(1);}while(0)


//#define INFO(...) do{dbg_printf(__VA_ARGS__);}while(0)
#define INFO(...) do{printf(__VA_ARGS__);}while(0)



//#define MTD_INFO(...) do{printf(__VA_ARGS__);}while(0)
#define MTD_INFO_READ(...)  
#define MTD_INFO_WRITE(...) 
#define MTD_INFO_ERASE(...) 
/*
#define MTD_INFO_READ(...)  do{printf(__VA_ARGS__);}while(0)
#define MTD_INFO_WRITE(...) do{printf(__VA_ARGS__);}while(0)
#define MTD_INFO_ERASE(...) do{printf(__VA_ARGS__);}while(0)
*/
#define MTD_INFO(...) 
#define MTD_WARN(...) do{printf(__VA_ARGS__);}while(0)

#define FTL_INFO(...) 
#define FTL_WARN(...) do{printf(__VA_ARGS__);}while(0)



//define VM_INFO(...) do{printf(__VA_ARGS__);}while(0)
#define VM_INFO(...) 
#define FAULT_INFO(...) 
//#define FAULT_INFO(...) do{printf(__VA_ARGS__);}while(0)

#define VM_ERR(...) do{printf(__VA_ARGS__);}while(0)

#define LLAPI_INFO(...)
//#define LLAPI_INFO(...) do{printf(__VA_ARGS__);}while(0)



#endif