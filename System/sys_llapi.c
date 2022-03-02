

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "llapi_code.h"
#include "sys_llapi.h"


static uint32_t inline syscall_1(uint32_t nr, uint32_t p1)
{
    register uint32_t r0 asm("r0") = p1;
    __asm volatile("svc %[nr]\n"
        : "=r" (r0)
        : [nr] "i" (nr), "r" (r0)
        : "memory", "r1", "r2", "r3", "r12", "lr");
    return (uint32_t) r0;
}

static uint32_t inline syscall_2(uint32_t nr, uint32_t p1, uint32_t p2)
{
    register uint32_t r0 asm("r0") = p1;
    register uint32_t r1 asm("r1") = p2;
    __asm volatile("svc %[nr]\n"
        : "=r" (r0)
        : [nr] "i" (nr), "r" (r0), "r" (r1)
        : "memory", "r2", "r3", "r12", "lr");
    return (uint32_t) r0;
}

static uint32_t inline syscall_4(uint32_t nr, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4)
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
    return syscall_1(LL_SWI_WRITE_STRING1, (uint32_t)s);
}

uint32_t ll_putStr2(char *s, uint32_t len)
{
    return syscall_2(LL_SWI_WRITE_STRING2, (uint32_t)s, len);
}

uint32_t ll_putChr(char c)
{
    return syscall_1(LL_SWI_PUT_CH, c);
}

uint32_t ll_gettime_us()
{
    return syscall_1(LL_SWI_GET_TIME_US, 0);
}

void ll_irq_set_Vector(void *p)
{
    syscall_1(LL_SWI_SET_IRQ_VECTOR, (uint32_t)p);
}

void ll_irq_set_Stack(void *p)
{
    syscall_1(LL_SWI_SET_IRQ_STACK, (uint32_t)p);
}

void ll_irq_enable(bool enable)
{
    if(enable){
        syscall_1(LL_SWI_ENABLE_IRQ, 0);
    }else{
        syscall_1(LL_SWI_DISABLE_IRQ, 0);
    }
}

void ll_irq_get_context(uint32_t *save)
{
    syscall_1(LL_SWI_IRQ_GET_CONTEXT, (uint32_t)save);
}

void ll_irq_set_context(uint32_t *load)
{
    syscall_1(LL_SWI_IRQ_SET_CONTEXT, (uint32_t)load);
}


void ll_irq_restore_context() __attribute__((naked));
void ll_irq_restore_context()
{
    syscall_1(LL_SWI_IRQ_RESTORE_CONTEXT, 0);
}

void ll_systick_set_period(uint32_t periodMs)
{
    syscall_1(LL_SWI_SYSTICK_SET_PERIOD, periodMs);
}

void ll_systick_enable(bool enable)
{
    syscall_1(LL_SWI_ENABLE_IRQ, enable);
}

