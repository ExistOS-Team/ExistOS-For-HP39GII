
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "sys_llapi.h"
#include "llapi_code.h"


volatile void IRQ_ISR(uint32_t IRQNum, uint32_t par1, uint32_t par2, uint32_t par3) __attribute__((naked));
volatile void IRQ_ISR(uint32_t IRQNum, uint32_t par1, uint32_t par2, uint32_t par3)
{
    printf("IRQNum:%d, par1:%d, par2:%d, par3:%d\n",IRQNum, par1, par2, par3);
    asm volatile("mov r0,r0");
    asm volatile("mov r0,r0");
    asm volatile("mov r0,r0");
    asm volatile("mov r0,r0");
    asm volatile("mov r0,r0");
    asm volatile("mov r0,r0");
    asm volatile("mov r0,r0");
    asm volatile("mov r0,r0");

    while(1);

}



