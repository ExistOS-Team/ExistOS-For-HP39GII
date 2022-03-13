#include "clkctrl_up.h"
#include "regsclkctrl.h"
#include "regsdigctl.h"
#include "board_up.h"

#include "regspower.h"

#define PLL_FREQ_HZ    480000000


static void PLLEnable(bool enable)
{
    BF_SETV(CLKCTRL_PLLCTRL0, POWER, enable);
    portDelayus(20);
}

static void setHCLKDivider(uint32_t div)
{
    uint32_t val = BF_RD(CLKCTRL_HBUS, DIV);
    if(!div){
        return;
    }
    BF_SETV(CLKCTRL_HBUS, DIV, div | 1);
    if(!(val & 1)){
        BF_CLRV(CLKCTRL_HBUS, DIV, 1);
    }
}

static void setCPUDivider(uint32_t div)
{
    uint32_t val = BF_RD(CLKCTRL_CPU, DIV_CPU);
    if(!div){
        return;
    }
    BF_SETV(CLKCTRL_CPU, DIV_CPU, div | 1);
    if(!(val & 1)){
        BF_CLRV(CLKCTRL_CPU, DIV_CPU, 1);
    }
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
    BF_SETV(POWER_VDDDCTRL,TRG,26); // Set voltage = 1.45 V
    PLLEnable(true);


    setCPUDivider(2);
    setHCLKDivider(2);

    setCPU_HFreqDomain(true);
    enableUSBClock(true);
    
}