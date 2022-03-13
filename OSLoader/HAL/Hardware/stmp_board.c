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

uint32_t nsToCycles(uint32_t nstime, uint32_t period, uint32_t min) 
{
    uint32_t k;
    k = (nstime + period - 1) / period;
    return (k > min) ? k : min;
}

uint32_t boardTick = 0;

uint32_t portBoardGetTime_us()
{
    return HW_DIGCTL_MICROSECONDS_RD() - boardTick;
}

void portBoardResetTick()
{
    boardTick = HW_DIGCTL_MICROSECONDS_RD();
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

void portBoardPowerOff()
{
    
    BF_WR(POWER_RESET, UNLOCK, 0x3E77);
    BF_WR(POWER_RESET, PWD, 1);

}

void portBoardReset()
{
    BF_WR(CLKCTRL_RESET, CHIP, 1);
}

void portBoardInit()
{
    USBPHYInit();
    AHBH_DMAInit();
    AHBX_DMAInit();
    GPMI_Init();
    HardECC8_Init();
    LCDIF_Init();

}
