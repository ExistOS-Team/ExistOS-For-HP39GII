

#include "interrupt_up.h"
#include "timer_up.h"
#include "../debug.h"

#include "hw_irq.h"

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
void up_isr( void )
{
    IRQTypes IRQType;
    IRQInfo IRQInfo;


    portIRQDecode(&CurrentIRQNumber, &IRQType, &IRQInfo);   

    switch (IRQType)
    {
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
    default:
        PANIC("Unknown IRQ:%d,%d\n", CurrentIRQNumber, IRQInfo);
        break;
    }

    portAckIRQ(CurrentIRQNumber);
}

void IRQInit()
{
    portIRQCtrlInit();

}

