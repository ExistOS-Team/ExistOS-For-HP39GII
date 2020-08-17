#include "timer.h"
#include "irq.h"
#include "regstimrot.h"
#include <stdio.h>

inline void timer_init(){
    BW_TIMROT_ROTCTRL_SFTRST(0);
    BW_TIMROT_ROTCTRL_CLKGATE(0);
    for (int i = 0; i <= HW_TIMROT_TIMCTRLn_COUNT; i++) { // All 4 timers are available on this device.
        BW_TIMROT_TIMCTRLn_UPDATE(i, 1);
        BW_TIMROT_TIMCTRLn_IRQ_EN(i, 1);
    }
}

inline char timer_set(char n, unsigned short count, unsigned int *callback){
    if (n <= HW_TIMROT_TIMCTRLn_COUNT && !(HW_TIMROT_TIMCTRLn_RD(n) & BM_TIMROT_TIMCTRLn_SELECT) && count) {
        BW_TIMROT_TIMCOUNTn_FIXED_COUNT(n, count);
        //BW_TIMROT_TIMCTRLn_RELOAD(n, isRepeat);
        irq_install_service(IRQ_N(n), callback);
        irq_set_enable(IRQ_N(n), 1);
        return 1;
    }else{
        return 0;
    }
}

inline void timer_start(char n){
    if (n <= HW_TIMROT_TIMCTRLn_COUNT) {
        BW_TIMROT_TIMCTRLn_SELECT(n, BV_TIMROT_TIMCTRLn_SELECT__32KHZ_XTAL);
    }  
}

inline void timer_stop(char n){
    if (n <= HW_TIMROT_TIMCTRLn_COUNT) {
        irq_set_enable(IRQ_N(n), 0);
        irq_install_service(IRQ_N(n), 0x0);
        BW_TIMROT_TIMCTRLn_SELECT(n, BV_TIMROT_TIMCTRLn_SELECT__NEVER_TICK); 
    }    
}