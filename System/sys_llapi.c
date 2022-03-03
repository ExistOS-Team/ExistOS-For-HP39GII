

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "llapi_code.h"
#include "sys_llapi.h"


static uint32_t inline syscall_2(uint32_t nr, uint32_t p1, uint32_t p2)
{
    register uint32_t r0 asm("r0") = p1;
    register uint32_t r1 asm("r1") = p2;
    __asm volatile("swi %[nr]\n"
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
    __asm volatile("swi %[nr]\n"
        : "=r" (r0)
        : [nr] "i" (nr), "r" (r0), "r" (r1), "r" (r2), "r" (r3)
        : "memory", "r12", "lr");
    return (uint32_t) r0;
}


volatile void ll_delay(uint32_t ms) 
{
    register uint32_t r0 asm("r0") = ms;
    __asm volatile(
    "svc %[num]" 
        : "=r"(r0)
        : [num] "i"(LL_SWI_DELAY_MS), 
            "r"(r0)
        : "memory", "r1", "r2", "r3", "r12", "lr"
    ); 

}

volatile uint32_t ll_putStr(char *s)
{
    register uint32_t r0 asm("r0") = (uint32_t)s;
    __asm__ volatile(
    "swi %[num]" 
        : "=r"(r0)
        : [num] "i"(LL_SWI_WRITE_STRING1), 
            "r"(r0)
        : "memory", "r1", "r2", "r3", "r12", "lr"
    ); 
    return r0;
}

volatile uint32_t ll_putStr2(char *s, uint32_t len)
{
    register uint32_t r0 asm("r0") = (uint32_t)s;
    register uint32_t r1 asm("r1") = (uint32_t)len;
    __asm__ volatile(
    "swi %[num]" 
        : "=r"(r0)
        : [num] "i"(LL_SWI_WRITE_STRING2), 
            "r"(r0), "r"(r1)
        : "memory", "r2", "r3", "r12", "lr"
    ); 
    return r0;
}

volatile uint32_t ll_putChr(char c)
{
    register uint32_t r0 asm("r0") = (uint32_t)c;
    __asm__ volatile(
    "swi %[num]" 
        : "=r"(r0)
        : [num] "i"(LL_SWI_PUT_CH), 
            "r"(r0)
        : "memory", "r1", "r2", "r3", "r12", "lr"
    ); 
    return r0;
}

volatile uint32_t ll_gettime_us()
{
    register uint32_t r0 asm("r0");
    __asm__ volatile(
    "swi %[num]" 
        : "=r"(r0)
        : [num] "i"(LL_SWI_GET_TIME_US), 
            "r"(r0)
        : "memory", "r1", "r2", "r3", "r12", "lr"
    ); 
    return r0;
}

volatile void ll_irq_set_Vector(void *p)
{
    register uint32_t r0 asm("r0") = (uint32_t)p;
    __asm__ volatile(
    "swi %[num]" 
        : "=r"(r0)
        : [num] "i"(LL_SWI_SET_IRQ_VECTOR), 
            "r"(r0)
        : "memory", "r1", "r2", "r3", "r12", "lr"
    ); 
}

volatile void ll_irq_set_Stack(void *p)
{
    register uint32_t r0 asm("r0") = (uint32_t)p;
    __asm__ volatile(
    "swi %[num]" 
        : "=r"(r0)
        : [num] "i"(LL_SWI_SET_IRQ_STACK), 
            "r"(r0)
        : "memory", "r1", "r2", "r3", "r12", "lr"
    ); 
}

volatile void ll_irq_enable(bool enable)
{

    register uint32_t r0 asm("r0");
    if(enable){
    __asm__ volatile(
        "swi %[num]" 
            : "=r"(r0)
            : [num] "i"(LL_SWI_ENABLE_IRQ), 
                "r"(r0)
            : "memory", "r1", "r2", "r3", "r12", "lr"
        ); 
    }else{
        __asm__ volatile(
        "swi %[num]" 
            : "=r"(r0)
            : [num] "i"(LL_SWI_DISABLE_IRQ), 
                "r"(r0)
            : "memory", "r1", "r2", "r3", "r12", "lr"
        ); 
    }
}

volatile void ll_irq_get_context(uint32_t *save)
{
    register uint32_t r0 asm("r0") = (uint32_t)save;
    __asm__ volatile(
    "swi %[num]" 
        : "=r"(r0)
        : [num] "i"(LL_SWI_IRQ_GET_CONTEXT), 
            "r"(r0)
        : "memory", "r1", "r2", "r3", "r12", "lr"
    ); 
}

volatile void ll_irq_set_context(uint32_t *load)
{
    register uint32_t r0 asm("r0") = (uint32_t)load;
    __asm__ volatile(
    "swi %[num]" 
        : "=r"(r0)
        : [num] "i"(LL_SWI_IRQ_SET_CONTEXT), 
            "r"(r0)
        : "memory", "r1", "r2", "r3", "r12", "lr"
    ); 
}


volatile void ll_irq_restore_context() __attribute__((naked));
volatile void ll_irq_restore_context()
{
    register uint32_t r0 asm("r0");
    __asm__ volatile(
    "swi %[num]" 
        : "=r"(r0)
        : [num] "i"(LL_SWI_IRQ_RESTORE_CONTEXT), 
            "r"(r0)
        : "memory", "r1", "r2", "r3", "r12", "lr"
    ); 
}

volatile void ll_systick_set_period(uint32_t periodMs)
{
    register uint32_t r0 asm("r0") = (uint32_t)periodMs;
    __asm__ volatile(
    "swi %[num]" 
        : "=r"(r0)
        : [num] "i"(LL_SWI_SYSTICK_SET_PERIOD), 
            "r"(r0)
        : "memory", "r1", "r2", "r3", "r12", "lr"
    ); 
}

volatile void ll_systick_enable(bool enable)
{
    register uint32_t r0 asm("r0") = (uint32_t)enable;
    __asm__ volatile(
    "swi %[num]" 
        : "=r"(r0)
        : [num] "i"(LL_SWI_ENABLE_IRQ), 
            "r"(r0)
        : "memory", "r1", "r2", "r3", "r12", "lr"
    ); 
}

