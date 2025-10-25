#include "board_up.h"
#include "regsapbh.h"
#include "regsapbx.h"
#include "regsgpmi.h"
#include "regspinctrl.h"
#include "regsecc8.h"
#include "regsusbphy.h"
#include "regsdigctl.h"
#include "regslcdif.h"
#include "regsclkctrl.h"
#include "regspower.h"
#include "regsrtc.h"
#include "regslradc.h"

uint64_t nsToCycles(uint64_t nstime, uint64_t period, uint64_t min) 
{
    uint64_t k;
    k = (nstime + period - 1) / period;
    return (k > min) ? k : min;
}

uint32_t boardTick = 0;

uint32_t portBoardGetTime_us()
{
    return HW_DIGCTL_MICROSECONDS_RD();
}

uint32_t portBoardGetTime_ms()
{
    return HW_RTC_MILLISECONDS_RD();
}

uint32_t portBoardGetTime_s()
{
    return HW_RTC_SECONDS_RD();
}

uint32_t portBoardGetTick()
{
    return portBoardGetTime_ms() - boardTick;
}

void portBoardResetTick()
{
    boardTick = portBoardGetTime_ms();
    //HW_DIGCTL_MICROSECONDS_RD();
    //HW_DIGCTL_MICROSECONDS_CLR(0xFFFFFFFF);
}

void portDelayus(uint32_t us)
{
    uint32_t start, cur;
    start = cur = HW_DIGCTL_MICROSECONDS_RD();
    while (cur < start + us) {
        cur = HW_DIGCTL_MICROSECONDS_RD();
    }
}

void portDelayms(uint32_t ms)
{
    uint32_t start, cur;
    start = cur = HW_RTC_MILLISECONDS_RD();
    while (cur < start + ms) {
        cur = HW_RTC_MILLISECONDS_RD();
    }
}

static void AHBH_DMAInit()
{


    BF_CLR(APBH_CTRL0, SFTRST);
    BF_CLR(APBH_CTRL0, CLKGATE);

    BF_SET(APBH_CTRL0, SFTRST);
    while(BF_RD(APBH_CTRL0, CLKGATE) == 0){
        ;
    }

    BF_CLR(APBH_CTRL0, SFTRST);
    BF_CLR(APBH_CTRL0, CLKGATE);
}

static void AHBX_DMAInit()
{
    BF_CLR(APBX_CTRL0, SFTRST);
    BF_CLR(APBX_CTRL0, CLKGATE);

    BF_SET(APBX_CTRL0, SFTRST);
    while(BF_RD(APBX_CTRL0, CLKGATE) == 0){
        ;
    }

    BF_CLR(APBX_CTRL0, SFTRST);
    BF_CLR(APBX_CTRL0, CLKGATE);
}

static void GPMI_Init()
{
    
    BF_CLR(CLKCTRL_GPMI, CLKGATE);
    
    BF_CLR(GPMI_CTRL0, SFTRST);
    BF_CLR(GPMI_CTRL0, CLKGATE);

    BF_SET(GPMI_CTRL0, SFTRST);
    
    while(BF_RD(GPMI_CTRL0, CLKGATE) == 0){
        ;
    }

    BF_CLR(GPMI_CTRL0, SFTRST);
    BF_CLR(GPMI_CTRL0, CLKGATE);

    BF_CS8(
        PINCTRL_MUXSEL0,
        BANK0_PIN07, 0, //D7
        BANK0_PIN06, 0, //D6
        BANK0_PIN05, 0, //D5
        BANK0_PIN04, 0, //D4
        BANK0_PIN03, 0, //D3
        BANK0_PIN02, 0, //D2
        BANK0_PIN01, 0, //D1
        BANK0_PIN00, 0  //D0
    );

    BF_CS1(
        PINCTRL_MUXSEL4,
        BANK2_PIN15, 1 //GPMI_CE0N
    );

    BF_CS7(
        PINCTRL_MUXSEL1,
        BANK0_PIN25, 0, //RDn
        BANK0_PIN24, 0, //WRn
        BANK0_PIN23, 3, //IRQ
        BANK0_PIN22, 0, //RSTn
        BANK0_PIN19, 0, //RB0

        BANK0_PIN17, 0, //A1
        BANK0_PIN16, 0  //A0
    );

    BF_CS8(
        PINCTRL_DRIVE0,
        BANK0_PIN07_MA, 2,      //12mA
        BANK0_PIN06_MA, 2,      //12mA
        BANK0_PIN05_MA, 2,      //12mA
        BANK0_PIN04_MA, 2,      //12mA
        BANK0_PIN03_MA, 2,      //12mA
        BANK0_PIN02_MA, 2,      //12mA
        BANK0_PIN01_MA, 2,      //12mA
        BANK0_PIN00_MA, 2       //12mA
    );

    BF_CS5(
        PINCTRL_DRIVE2,
        BANK0_PIN23_MA, 2,      //12mA
        BANK0_PIN22_MA, 2,      //12mA
        BANK0_PIN19_MA, 2,      //12mA
        BANK0_PIN17_MA, 2,      //12mA
        BANK0_PIN16_MA, 2       //12mA
    );

    BF_CS2(
        PINCTRL_DRIVE3,
        BANK0_PIN25_MA, 2,      //12mA
        BANK0_PIN24_MA, 2      //12mA
    );

}

static void HardECC8_Init()
{
    BF_CLR(ECC8_CTRL, SFTRST);
    BF_CLR(ECC8_CTRL, CLKGATE);

    BF_SET(ECC8_CTRL, SFTRST);
    while(BF_RD(ECC8_CTRL, CLKGATE) == 0)
    {
        ;
    }

    BF_CLR(ECC8_CTRL, SFTRST);
    BF_CLR(ECC8_CTRL, CLKGATE);

    BF_CLR(ECC8_CTRL, AHBM_SFTRST);
    
}

static void USBPHYInit()
{
    BF_CLR(USBPHY_CTRL, SFTRST);
    BF_CLR(USBPHY_CTRL, CLKGATE);

    BF_SET(USBPHY_CTRL, SFTRST);
    while(BF_RD(USBPHY_CTRL, CLKGATE) == 0)
    {
        ;
    }

    BF_CLR(USBPHY_CTRL, SFTRST);
    BF_CLR(USBPHY_CTRL, CLKGATE);
}

static void LCDIF_Init()
{
    
    BF_SET(CLKCTRL_CLKSEQ, BYPASS_PIX);
    BF_CLR(CLKCTRL_PIX, CLKGATE);

    BF_CLR(LCDIF_CTRL, SFTRST);
    BF_CLR(LCDIF_CTRL, CLKGATE);

    BF_SET(LCDIF_CTRL, SFTRST);
    while(BF_RD(LCDIF_CTRL, CLKGATE) == 0)
    {
        ;
    }

    BF_CLR(LCDIF_CTRL, SFTRST);
    BF_CLR(LCDIF_CTRL, CLKGATE);
}

static void RTC_Init()
{
    BF_CLR(RTC_CTRL, SFTRST);
    BF_CLR(RTC_CTRL, CLKGATE);

    BF_SET(RTC_CTRL, SFTRST);
    while(BF_RD(RTC_CTRL, CLKGATE) == 0)
    {
        ;
    }

    BF_CLR(RTC_CTRL, SFTRST);
    BF_CLR(RTC_CTRL, CLKGATE);

}

static void LRADC_init()
{
    BF_CLR(LRADC_CTRL0, SFTRST);
    BF_CLR(LRADC_CTRL0, CLKGATE);

    BF_SET(LRADC_CTRL0, SFTRST);
    while(BF_RD(LRADC_CTRL0, CLKGATE) == 0)
    {
        ;
    }

    BF_CLR(LRADC_CTRL0, SFTRST);
    BF_CLR(LRADC_CTRL0, CLKGATE);

}


void portBoardReset()
{
    BF_WR(CLKCTRL_RESET, CHIP, 1);
}

uint32_t portGetBatterVoltage_mv()
{
    //portLRADCConvCh(7, 1);
    uint32_t ad_val = BF_RD(POWER_BATTMONITOR, BATT_VAL);
    return ad_val * 8;
}

uint32_t portGetBatteryMode()
{
    return BF_RD(POWER_STS, MODE);
}

uint32_t portGetPWRSpeed()
{
    uint8_t val;
    static uint8_t last_val;

    vTaskEnterCritical();
    HW_POWER_SPEED.B.CTRL = 0;
    portDelayus(1);
    HW_POWER_SPEED.B.CTRL = 1;
    portDelayus(1);
    HW_POWER_SPEED.B.CTRL = 3;
    val = HW_POWER_SPEED.B.STATUS;
    vTaskExitCritical();
    last_val = val;
    return (val + last_val) / 2;
}

void portBoardInit()
{

    
    
    portPowerInit();

    USBPHYInit();
    AHBH_DMAInit();
    AHBX_DMAInit();
    GPMI_Init();
    HardECC8_Init();
    LCDIF_Init();
    RTC_Init();
    LRADC_init();
    portLRADC_init();

    

}
