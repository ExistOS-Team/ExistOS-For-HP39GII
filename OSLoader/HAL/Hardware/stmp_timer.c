

#include "hw_irq.h"
#include "timer_up.h"
#include <stdint.h>
#include "interrupt_up.h"

#include "regsclkctrl.h"
#include "regstimrot.h"

#include "../debug.h"

static uint32_t timer0ReloadVal;
static uint32_t timer1ReloadVal;

void portTimerInit(void)
{

    BF_CLR(TIMROT_ROTCTRL, SFTRST);
    BF_CLR(TIMROT_ROTCTRL, CLKGATE);
    
    BF_SET(TIMROT_ROTCTRL, SFTRST);
    while(BF_RD(TIMROT_ROTCTRL, CLKGATE) == 0)
        ;
    
    BF_CLR(TIMROT_ROTCTRL, SFTRST);
    BF_CLR(TIMROT_ROTCTRL, CLKGATE);
    
    
    BF_WRn(TIMROT_TIMCTRLn, 0, SELECT, BV_TIMROT_TIMCTRLn_SELECT__32KHZ_XTAL);
    BF_WRn(TIMROT_TIMCTRLn, 1, SELECT, BV_TIMROT_TIMCTRLn_SELECT__32KHZ_XTAL);


}



void portAckTimerIRQ(void)
{
    BF_CLRn(TIMROT_TIMCTRLn, 0, IRQ);
    up_TimerTick();
}

bool portEnableTimerIRQ(int timer, bool enable)
{

        BF_CS1n(TIMROT_TIMCTRLn, 0, IRQ_EN, enable);
        portEnableIRQ(HW_IRQ_TIMER0, enable);
        return true;
}

bool portEnableTimer(int timer, bool enable)
{

        if(enable){
            BF_WRn(TIMROT_TIMCOUNTn, 0, FIXED_COUNT, timer0ReloadVal);          
        }else{
            BF_CS1n(TIMROT_TIMCOUNTn, 0, FIXED_COUNT, 0);
        }
        BF_CS1n(TIMROT_TIMCTRLn, 0, RELOAD, enable);
        BF_CS1n(TIMROT_TIMCTRLn, 0, UPDATE, enable);
        return true;
}

bool portSetTimerPeriod(int timer, unsigned int us)
{


        timer0ReloadVal = us/32;
        return true;
        

}

int portGetTimerNum(void){
    return 1;
}


int portGetTimer(void){
    return HW_IRQ_TIMER0;
}