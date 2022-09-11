#include "board_up.h"
#include "clkctrl_up.h"
#include "regsclkctrl.h"
#include "regsdigctl.h"

#include "regspower.h"

#include "../debug.h"

#include "SystemConfig.h"

#define PLL_FREQ_HZ (480000000UL)

bool g_slowdown_enable = true;
static uint8_t min_cpu_frac_sd = CPU_DIVIDE_IDLE_INTIAL;

static void PLLEnable(bool enable) {
    BF_SETV(CLKCTRL_PLLCTRL0, POWER, enable);
    portDelayus(20);
}

void setHCLKDivider(uint32_t div) 
{
    if (!div) {
        return;
    }
    while(BF_RD(CLKCTRL_HBUS, BUSY));
    BF_SETV(CLKCTRL_HBUS, DIV, div);
    while(BF_RD(CLKCTRL_HBUS, BUSY));
    BF_CLRV(CLKCTRL_HBUS, DIV, BF_RD(CLKCTRL_HBUS, DIV) ^ div);

}

void setSlowDownMinCpuFrac(uint8_t frac)
{
    if(frac > 14 || frac < 2)
    {
        return;
    }
    min_cpu_frac_sd = frac;
}

void enterSlowDown()
{
    if(g_slowdown_enable)
    {
        setCPUDivider(min_cpu_frac_sd);
    }
}

void exitSlowDown()
{
    if(g_slowdown_enable)
    {
        setCPUDivider(CPU_DIVIDE_NORMAL);
    }
    
}


void slowDownEnable(bool enable)
{
    g_slowdown_enable = enable;
    if(!g_slowdown_enable)
    {
        setCPUDivider(CPU_DIVIDE_NORMAL);
    }
}



void setCPUDivider(uint32_t div) 
{
    //uint32_t val = BF_RD(CLKCTRL_CPU, DIV_CPU);
    //INFO("CPU old Div:%lu\n", val);
    if (!div) {
        return;
    }
    //while (BF_RD(CLKCTRL_CPU, BUSY_REF_CPU));
    BF_SETV(CLKCTRL_CPU, DIV_CPU, div);
    //while (BF_RD(CLKCTRL_CPU, BUSY_REF_CPU));
    BF_CLRV(CLKCTRL_CPU, DIV_CPU, BF_RD(CLKCTRL_CPU, DIV_CPU) ^ div);

    //INFO("CPU new Div:%d\n", BF_RD(CLKCTRL_CPU, DIV_CPU));
}

void setCPUFracDivider(uint32_t div) {

    if (!div) {
        return;
    }
    bool bypass;
    bypass = BF_RD(CLKCTRL_CLKSEQ, BYPASS_CPU);
    BF_SET(CLKCTRL_CLKSEQ, BYPASS_CPU);
    BF_WR(CLKCTRL_FRAC, CPUFRAC, div);
    if(!bypass){
        BF_CLR(CLKCTRL_CLKSEQ, BYPASS_CPU);
    }
}

static void setCPU_HFreqDomain(bool enable) {
    if (enable) {
        
        BF_CLR(CLKCTRL_CLKSEQ, BYPASS_CPU);
    } else {
        BF_SET(CLKCTRL_CLKSEQ, BYPASS_CPU);
    }
}

static void enableUSBClock(bool enable) {
    if (enable) {
        BF_SET(CLKCTRL_PLLCTRL0, EN_USB_CLKS);
    } else {
        BF_CLR(CLKCTRL_PLLCTRL0, EN_USB_CLKS);
    }
}

void volatile portCLKCtrlInit(void) {
    //BF_SETV(POWER_VDDDCTRL, TRG, 26); // Set voltage = 1.45 V
    //BF_SETV(POWER_VDDACTRL, TRG, 18); // Set voltage = 1.95 V  val = (TAG_v - 1.5v)/0.025v


    PLLEnable(true);
    
    BF_CLR(CLKCTRL_FRAC, CLKGATECPU);
    

    setCPUDivider(5);
    setHCLKDivider(4);
    
    setCPU_HFreqDomain(true);

    setHCLKDivider(2);
    setCPUFracDivider(22);
    
    enableUSBClock(true);
}

void portGetCoreFreqDIV(uint32_t *CPU_DIV, uint32_t *CPU_Frac, uint32_t *HCLK_DIV)
{
    *CPU_DIV = BF_RD(CLKCTRL_CPU, DIV_CPU);
    *CPU_Frac = BF_RD(CLKCTRL_FRAC, CPUFRAC);
    *HCLK_DIV = BF_RD(CLKCTRL_HBUS, DIV);
}
