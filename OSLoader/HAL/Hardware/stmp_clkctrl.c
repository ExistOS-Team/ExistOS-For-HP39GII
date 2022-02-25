#include "clkctrl_up.h"
#include "regsclkctrl.h"
#include "regsdigctl.h"
#include "board_up.h"
#define PLL_FREQ_HZ    480000000


static void PLLEnable(bool enable)
{
    BF_SETV(CLKCTRL_PLLCTRL0, POWER, enable);
    portDelayus(20);
}

static void setHCLKDivider(uint32_t div)
{
    if(!div){
        return;
    }
    BF_SETV(CLKCTRL_HBUS, DIV, div);
}

static void setCPUDivider(uint32_t div)
{
    if(!div){
        return;
    }
    BF_SETV(CLKCTRL_CPU, DIV_CPU, div);
}

static void setCPU_HFreqDomain(bool enable)
{
    if(enable){
        BF_CLR(CLKCTRL_FRAC, CLKGATECPU);
        BF_CLR(CLKCTRL_CLKSEQ, BYPASS_CPU);
    }else{
        BF_SET(CLKCTRL_CLKSEQ, BYPASS_CPU);
    }
}

static void enableUSBClock(bool enable)
{
    if(enable){
        BF_SET(CLKCTRL_PLLCTRL0, EN_USB_CLKS);
    }else{
        BF_CLR(CLKCTRL_PLLCTRL0, EN_USB_CLKS);
    }
}

uint32_t portGetCoreFreqHz()
{
    
}

void portCLKCtrlInit(void)
{
    PLLEnable(true);
    setCPUDivider(8);
    setHCLKDivider(1);
    setCPU_HFreqDomain(true);
    enableUSBClock(true);
    
}