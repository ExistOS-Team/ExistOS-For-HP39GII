
#include <stdint.h>
#include <stdio.h>

#include "sys_llapi.h"
#include "llapi_code.h"

int coremain(void);

void testcpp();

char ctrlC = 0;

uint32_t IRQ_Context[16];
volatile void IRQ_ISR(uint32_t IRQNum, uint32_t par1, uint32_t par2, uint32_t par3) __attribute__((naked));
volatile void IRQ_ISR(uint32_t IRQNum, uint32_t par1, uint32_t par2, uint32_t par3)
{
    //printf("IRQNum:%d, par1:%d, par2:%d, par3:%d\n",IRQNum, par1, par2, par3);
    switch (IRQNum)
    {
    case LL_IRQ_KEYBOARD:
        printf("Key:%d, Press:%d\n", par1, par2);
        //ll_putStr("KEY IRQ\n");
        if((par1 == 80) && (par2 == 1))
        {
            ctrlC = 1;
        }
        break;
    case LL_IRQ_TIMER:
        printf("TimerTick\n");
        break;
    default:
        break;
    }
    ll_irq_restore_context();

}

void delayms(uint32_t ms)
{
    uint32_t startTime = ll_gettime_us();
    while((ll_gettime_us() - startTime) < (ms * 1000))
    {
        ;
    }

}

int system_checkCtrlC()
{
    if(ctrlC)
    {
        ctrlC = 0;
        return 1;
    }
    return 0;
}


void main()
{

    ll_irq_set_Stack((uint32_t *)0x02400000);
    
    ll_irq_set_Vector(IRQ_ISR);
    ll_irq_enable(true);

//    ll_systick_set_period(500);
//    ll_systick_enable(true);

/*
    printf("Core Mark Testing..\n");
    coremain();
    printf("Core Mark Test Finish\n");
*/
    printf("Start Test....\n");
    ll_putStr("ll_putStr Start Test....\n");
    for(;;){
        ll_putStr("LOOP....1\n");
        testcpp();
        delayms(1000);
        //coremain();
        delayms(1000);
        ll_putStr("LOOP....2\n");
    }

}