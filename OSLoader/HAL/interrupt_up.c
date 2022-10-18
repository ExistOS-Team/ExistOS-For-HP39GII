

#include "interrupt_up.h"
#include "../debug.h"
#include "timer_up.h"

#include "hw_irq.h"
#include "regsicoll.h"

#include "timer_up.h"
#include "regstimrot.h"

#include "FreeRTOS.h"
#include "task.h"

IRQNumber CurrentIRQNumber;

IRQList IRQLists[MAX_IRQ_NUM];

/*
void up_register_ISR(IRQNumber ISRNum, IRQTypes IRQType, IRQCallback cb)
{
    IRQLists[ISRNum].IRQType = IRQType;
    IRQLists[ISRNum].cb = cb;
}
*/

void usb_dcd_isr(void);

void portMTD_ISR();
void portMTD_DMA_ISR();

bool portMTD_ECC_ISR();
void portDISP_ISR();
void port_LRADC_IRQ(uint32_t ch);
void portPowerIRQ(uint32_t nirq);
void portDAC_IRQ(uint32_t IRQn);

void up_isr(void) {
/*
    portIRQDecode(&CurrentIRQNumber, &IRQType, &IRQInfo);

    switch (IRQType) {
    case IRQType_Timer:
        portAckTimerIRQ();
        break;
    case IRQType_USBCtrl:
        usb_dcd_isr();
        break;
    case IRQType_MTD:
        portMTD_ISR();
        break;
    case IRQType_MTD_DMA:
        portMTD_DMA_ISR();
        break;
    case IRQType_MTD_ECC:
        portMTD_ECC_ISR();
        break;
    case IRQType_DISP:
        portDISP_ISR();
        break;
    case IRQType_LRADC:
        port_LRADC_IRQ(IRQInfo);
        break;
    case IRQType_PWR:
        portPowerIRQ(IRQInfo);
        break;
#ifdef ENABLE_AUIDIOOUT
    case IRQType_DAC:
        portDAC_IRQ(IRQInfo);
        break;
#endif
    default:
        PANIC("Unknown IRQ:%d,%d\n", CurrentIRQNumber, IRQInfo);
        break;
    }*/

    CurrentIRQNumber = BF_RD(ICOLL_VECTOR, IRQVECTOR)  ;
    switch (CurrentIRQNumber)
    {
    case HW_IRQ_TIMER0:
        BF_CLRn(TIMROT_TIMCTRLn, 0, IRQ);
        //up_TimerTick();
        if( xTaskIncrementTick() != pdFALSE )
	    {	
	    	vTaskSwitchContext();
	    }
        //portAckTimerIRQ();
        break;/*
    case HW_IRQ_TIMER1:
        portAckTimerIRQ();
        break;    */
    case HW_IRQ_USB_CTRL:
        usb_dcd_isr();
        break; 
    case HW_IRQ_GPMI:
        portMTD_ISR();
        break;
    case HW_IRQ_GPMI_DMA:
        portMTD_DMA_ISR();
        break;
    case HW_IRQ_ECC8_IRQ:
        portMTD_ECC_ISR();
        break;
    case HW_IRQ_LCDIF_DMA:
    case HW_IRQ_LCDIF_ERROR:
        portDISP_ISR();
        break;
        
    case HW_IRQ_LRADC_CH0:
        port_LRADC_IRQ(0);
        break;
    case HW_IRQ_LRADC_CH1:
        port_LRADC_IRQ(1);
        break;
    case HW_IRQ_LRADC_CH2:
        port_LRADC_IRQ(2);
        break;
    case HW_IRQ_LRADC_CH3:
        port_LRADC_IRQ(3);
        break;
    case HW_IRQ_LRADC_CH4:
        port_LRADC_IRQ(4);
        break;
    case HW_IRQ_LRADC_CH5:
        port_LRADC_IRQ(5);
        break;
    case HW_IRQ_LRADC_CH6:
        port_LRADC_IRQ(6);
        break;
    case HW_IRQ_LRADC_CH7:
        port_LRADC_IRQ(7);
        break;    
    case HW_IRQ_VDD5V:
    case HW_IRQ_VDD5V_DROOP:
    case HW_IRQ_VDD18_BRNOUT:
    case HW_IRQ_VDDD_BRNOUT:
    case HW_IRQ_VDDIO_BRNOUT:
    case HW_IRQ_BATT_BRNOUT:
    case 63:
        portPowerIRQ(CurrentIRQNumber);
        break;
#ifdef ENABLE_AUIDIOOUT
    case HW_IRQ_DAC_DMA:
    case HW_IRQ_DAC_ERROR:
        portDAC_IRQ(CurrentIRQNumber);
        break;
#endif
    default:
        INFO("ERR IRQNum:%d\n", CurrentIRQNumber);
    }

    portAckIRQ(CurrentIRQNumber);
}

void IRQInit() {
    portIRQCtrlInit();
}
