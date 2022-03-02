
#include <stdint.h>
#include <stdio.h>

#include "sys_llapi.h"
#include "llapi_code.h"

int coremain(void);


uint32_t IRQ_Context[16];
void IRQ_ISR(uint32_t IRQNum, uint32_t par1, uint32_t par2, uint32_t par3)
{
    ll_irq_get_context(IRQ_Context);

    //printf("IRQNum:%d, par1:%d, par2:%d, par3:%d\n",IRQNum, par1, par2, par3);
    switch (IRQNum)
    {
    case LL_IRQ_KEYBOARD:
        printf("Key:%d, Press:%d\n", par1, par2);
        break;
    case LL_IRQ_TIMER:
        printf("TimerTick\n");
        break;
    default:
        break;
    }

    ll_irq_set_context(IRQ_Context);


    ll_irq_restore_context();

}

void main()
{

    ll_irq_set_Stack((uint32_t *)0x004A0000);
    ll_irq_set_Vector(IRQ_ISR);
    ll_irq_enable(true);

    ll_systick_set_period(1000);
    ll_systick_enable(true);
/*
    printf("Core Mark Testing..\n");
    coremain();
    printf("Core Mark Test Finish\n");
*/
    for(;;){
        ll_delay(1000);
    }

}