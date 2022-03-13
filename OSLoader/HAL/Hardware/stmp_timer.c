

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



void portAckTimerIRQ(IRQNumber IRQNum, IRQInfo IRQInfo)
{
    
    switch (IRQNum)
    {
    case HW_IRQ_TIMER0:
        
        BF_CLRn(TIMROT_TIMCTRLn, 0, IRQ);
        up_LowFrequencyTimerTick();
        return;/*
    case HW_IRQ_TIMER1:
        
        BF_CLRn(TIMROT_TIMCTRLn, 1, IRQ);

        return;
*/
    default:
        PANNIC("ERR IRQNum%d\n", IRQNum);
        return;
    }
}

bool portEnableTimerIRQ(int timer, bool enable)
{
    switch (timer)
    {
    case HW_IRQ_TIMER0:
        BF_CS1n(TIMROT_TIMCTRLn, 0, IRQ_EN, enable);
        portEnableIRQ(HW_IRQ_TIMER0, enable);
        return true;
    case HW_IRQ_TIMER1:
        BF_CS1n(TIMROT_TIMCTRLn, 1, IRQ_EN, enable);
        portEnableIRQ(HW_IRQ_TIMER1, enable);
        return true;    
    
    default:
        return false;
    }
}

bool portEnableTimer(int timer, bool enable)
{
    switch (timer)
    {
    case HW_IRQ_TIMER0:
        if(enable){
            BF_WRn(TIMROT_TIMCOUNTn, 0, FIXED_COUNT, timer0ReloadVal);          
        }else{
            BF_CS1n(TIMROT_TIMCOUNTn, 0, FIXED_COUNT, 0);
        }
        BF_CS1n(TIMROT_TIMCTRLn, 0, RELOAD, enable);
        BF_CS1n(TIMROT_TIMCTRLn, 0, UPDATE, enable);
        return true;
    case HW_IRQ_TIMER1:
        if(enable){   
            BF_CS1n(TIMROT_TIMCOUNTn, 1, FIXED_COUNT, timer1ReloadVal);
        }else{
            BF_CS1n(TIMROT_TIMCOUNTn, 1, FIXED_COUNT, 0);
        }
        BF_CS1n(TIMROT_TIMCTRLn, 1, RELOAD, enable);
        BF_CS1n(TIMROT_TIMCTRLn, 1, UPDATE, enable);
        return true;    
    
    default:
        return false;
    }
}

bool portSetTimerPeriod(int timer, unsigned int us)
{

    switch (timer)
    {
    case HW_IRQ_TIMER0:
        timer0ReloadVal = us/32;
        return true;
        
    case HW_IRQ_TIMER1:
        timer1ReloadVal = us/32;
        return true;        
    
    default:
        return false;
    }

}

int portGetTimerNum(void){
    return 2;
}


int portGetLowFrequencyTimer(void){
    return HW_IRQ_TIMER0;
}

int portGetHighFrequencyTimer(void){
    return HW_IRQ_TIMER1;
}