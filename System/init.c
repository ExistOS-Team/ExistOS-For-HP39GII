
#include "SysConf.h"

#include <sys/types.h>
#include <stdlib.h>
extern unsigned int _sbss;
extern unsigned int _ebss;

extern void (*__preinit_array_start[])(void) __attribute__((weak));
extern void (*__preinit_array_end[])(void) __attribute__((weak));
extern void (*__init_array_start[])(void) __attribute__((weak));
extern void (*__init_array_end[])(void) __attribute__((weak));

void main();

extern unsigned int __init_data;
extern unsigned int __data_start;
extern unsigned int __data_end;

void volatile set_r13(uint32_t r13) __attribute__((naked));
void volatile set_r13(uint32_t r13)
{
    __asm volatile("mov r13,r0");
}

void volatile _init() __attribute__((section(".init"))) __attribute__((naked));
void volatile _init() {

    __asm volatile(".word 0xEF5AE0EF");
    __asm volatile(".word 0xFECDAFDE");

    set_r13(NORMAL_STACK_ADDR);

    for (char *i = (char *)&_sbss; i < (char *)&_ebss; i++) {
        *i = 0; // clear bss
    }

    uint32_t *pui32Src, *pui32Dest;
    pui32Src = (uint32_t *)&__init_data;
    for (pui32Dest =  (uint32_t *)&__data_start; pui32Dest <  (uint32_t *)&__data_end;) {
        *pui32Dest++ = *pui32Src++;
    }



    //__libc_init_array();

    typedef void (*pfunc)();
    extern pfunc __ctors_start__[];
    extern pfunc __ctors_end__[];
    pfunc *p;
    for (p = __ctors_start__; p < __ctors_end__; p++) {
        (*p)();
    }


    main();

    while (1)
        ;
}

extern void (*__fini_array_start[])(void);
extern void (*__fini_array_end[])(void);

void __libc_fini_array(void) {
    size_t count;
    size_t i;

    count = __fini_array_end - __fini_array_start;
    for (i = count; i > 0; i--)
        __fini_array_start[i - 1]();
}

/*
void __libc_init_array (void)
{
  size_t count;
  size_t i;

  count = __preinit_array_end - __preinit_array_start;
  for (i = 0; i < count; i++)
    __preinit_array_start[i] ();

  count = __init_array_end - __init_array_start;
  for (i = 0; i < count; i++)
    __init_array_start[i] ();
}*/

