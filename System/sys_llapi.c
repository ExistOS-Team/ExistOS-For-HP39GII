

#include <stdint.h>
#include <stdio.h>


#include "sys_llapi_code.h"
#include "sys_llapi.h"


uint32_t inline syscall_1(uint32_t nr, uint32_t p1)
{
    register uint32_t r0 asm("r0") = p1;
    __asm volatile("svc %[nr]\n"
        : "=r" (r0)
        : [nr] "i" (nr), "r" (r0)
        : "memory", "r1", "r2", "r3", "r12", "lr");
    return (uint32_t) r0;
}

uint32_t inline syscall_4(uint32_t nr, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4)
{
    register uint32_t r0 asm("r0") = p1;
    register uint32_t r1 asm("r1") = p2;
    register uint32_t r2 asm("r2") = p3;
    register uint32_t r3 asm("r3") = p4;
    __asm volatile("svc %[nr]\n"
        : "=r" (r0)
        : [nr] "i" (nr), "r" (r0), "r" (r1), "r" (r2), "r" (r3)
        : "memory", "r12", "lr");
    return (uint32_t) r0;
}


void ll_delay(uint32_t ms) 
{
    syscall_1(LL_SWI_DELAY_MS, ms);
}

uint32_t ll_putStr(char *s)
{
    syscall_1(LL_SWI_WRITE_STRING, (uint32_t)s);
}

uint32_t ll_putChr(char c)
{
    syscall_1(LL_SWI_PUT_CH, c);
}
