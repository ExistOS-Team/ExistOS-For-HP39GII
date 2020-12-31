#include "clkgen.h"
#include "regsclkctrl.h"
#include "rtc.h"
#include "regs.h"
#include "regslcdif.h"

#include <stdio.h>

#include "utils.h"

inline void PLL_enable(reg8_t isAble){
    BF_SETV(CLKCTRL_PLLCTRL0, POWER, isAble);
	delay_us(10);
}

inline reg8_t CPUCLK_set_div(reg8_t isFracEnabled, reg16_t divider){
    if (HW_CLKCTRL_CPU_RD() & BM_CLKCTRL_CPU_BUSY_REF_CPU) {
        return 0;
    }else{
        BF_CS1(CLKCTRL_CPU, DIV_CPU_FRAC_EN, isFracEnabled);
        BF_SETV(CLKCTRL_CPU, DIV_CPU, divider);
		if(!(divider & 1))
			BF_CLRV(CLKCTRL_CPU, DIV_CPU, 1);
        return 1;
    }
}

inline void CPUCLK_set_bypass(reg8_t bypass){
    BF_CS1(CLKCTRL_CLKSEQ,BYPASS_CPU, bypass);
}

inline void CPUCLK_set_gating(reg8_t gating){
    BF_CS1(CLKCTRL_FRAC, CLKGATECPU, gating);
}

inline reg8_t HCLK_set_div(reg8_t isFracEnabled, reg16_t divider){
    if (HW_CLKCTRL_HBUS_RD() & BM_CLKCTRL_HBUS_BUSY) {
        return 0;
    }else{
        BF_CS1(CLKCTRL_HBUS, DIV_FRAC_EN, isFracEnabled);

        BF_SETV(CLKCTRL_HBUS, DIV, divider);
		if(!(divider & 1))
			BF_CLRV(CLKCTRL_HBUS, DIV, 1);
		
		
        return 1;
    }
}

inline void HCLK_set_autoslow(reg8_t autoslow){
    if (autoslow) {
        BF_SETV(CLKCTRL_HBUS, APBHDMA_AS_ENABLE, 1);
        BF_SETV(CLKCTRL_HBUS, AUTO_SLOW_MODE, 1);
        BF_SETV(CLKCTRL_HBUS, SLOW_DIV, 4);
    }else{
        BF_SETV(CLKCTRL_HBUS, APBHDMA_AS_ENABLE, 0);
        BF_SETV(CLKCTRL_HBUS, AUTO_SLOW_MODE, 0);
        BF_SETV(CLKCTRL_HBUS, SLOW_DIV, 0);
    }
}

inline reg8_t overclock(reg8_t isFracEnabled, reg16_t divider, reg8_t isHbusFracEnabled, reg16_t hbusDivider, reg8_t isAutoSlow){
    if (CPUCLK_set_div(isFracEnabled, divider) && HCLK_set_div(isHbusFracEnabled, hbusDivider)) {
        if (isAutoSlow) {
            BF_SETV(CLKCTRL_HBUS, APBHDMA_AS_ENABLE, 1);
            BF_SETV(CLKCTRL_HBUS, AUTO_SLOW_MODE, 1);
            BF_SETV(CLKCTRL_HBUS, SLOW_DIV, 4);
        }
        CPUCLK_set_gating(0);
        PLL_enable(1);
        delay_us(10);
        CPUCLK_set_bypass(0);
        return 1;
    }else{
        return 0;
    }
}
