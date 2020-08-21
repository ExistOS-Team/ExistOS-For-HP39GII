#pragma once

#include "regs.h"

#include "clkgen.h"
#include "regsclkctrl.h"
#include "rtc.h"
#include "regs.h"
#include <stdio.h>

#include "utils.h"
#ifdef __cplusplus 
extern "C" { 
#endif

void PLL_enable(reg8_t isAble);

reg8_t CPUCLK_set_div(reg8_t isFracEnabled, reg16_t divider);

void CPUCLK_set_bypass(reg8_t bypass);

void CPUCLK_set_gating(reg8_t gating);

reg8_t HCLK_set_div(reg8_t isFracEnabled, reg16_t divider);

void CPUCLK_set_frac(reg8_t frac);

void HCLK_set_autoslow(reg8_t autoslow);

reg8_t overclock(reg8_t frac, reg8_t isFracEnabled, reg16_t divider, reg8_t isHbusFracEnabled, reg16_t hbusDivider, reg8_t isAutoSlow);
#ifdef __cplusplus 
};
#endif
