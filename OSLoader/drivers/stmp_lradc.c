
#include "stdint.h"
#include "stdbool.h"

#include "hw_irq.h"
#include "regslradc.h"
#include "../debug.h"

#include "interrupt_up.h"

#include "board_up.h"



void portLRADC_init()
{
    
    HW_LRADC_CONVERSION.B.AUTOMATIC = 1;

    INFO("LRADC_STATUS:%08x\n", HW_LRADC_STATUS_RD());
    

    HW_LRADC_CTRL4.B.LRADC5SELECT = BV_LRADC_CTRL4_LRADC7SELECT__CHANNEL15; //VDD5V

    HW_LRADC_CTRL2.B.TEMPSENSE_PWD = 0;

    HW_LRADC_CTRL4.B.LRADC4SELECT = BV_LRADC_CTRL4_LRADC7SELECT__CHANNEL9;
    HW_LRADC_CTRL4.B.LRADC3SELECT = BV_LRADC_CTRL4_LRADC7SELECT__CHANNEL8;
    HW_LRADC_CTRL4.B.LRADC2SELECT = BV_LRADC_CTRL4_LRADC7SELECT__CHANNEL14;

}

void portLRADCEnable(bool enable ,uint32_t ch)
{
    INFO("Enable LRADC:%lu,%d\n",ch, enable);

    portEnableIRQ(HW_IRQ_LRADC_CH0 + ch, enable);
    
    if(enable)
    {
        HW_LRADC_CTRL1_SET( ((1) << 16) << ch);  
    }else{
        HW_LRADC_CTRL1_CLR( ((1) << 16) << ch);  
    }

}

uint32_t portLRADCConvCh(uint32_t ch, uint32_t samples)
{
    if(!samples)
    {
        return 1;
    }
    uint32_t n = samples;
    uint32_t acc_val = 0;

    do{
        BF_CLRn(LRADC_CHn, ch, VALUE);
        BF_CLRn(LRADC_CHn, ch, TOGGLE);
        BF_WRn(LRADC_CHn, ch,  ACCUMULATE, 1);
        BF_CS1n(LRADC_CHn, ch, NUM_SAMPLES, 1);
        BF_WR(LRADC_CTRL0, SCHEDULE, (1) << ch);
        while(BF_RDn(LRADC_CHn, ch, TOGGLE) == 0);
        acc_val += BF_RDn(LRADC_CHn, ch, VALUE);
        n--;
    }while(n > 0);
    
    return acc_val / samples;

}

void port_LRADC_IRQ(uint32_t ch)
{

    INFO("\n\nLRADC IRQ:%ld, val:%d\n", ch, BF_RDn(LRADC_CHn, ch, VALUE));
    HW_LRADC_CTRL1_CLR(1 << ch);  





}


