

#include "interrupt_up.h"
#include "timer_up.h"
#include "../debug.h"


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


bool up_isr( void )
{
    bool YIELD = false;
    bool IRQvaild;
    IRQTypes IRQType;
    IRQInfo IRQInfo;
    IRQvaild = portIRQDecode(&CurrentIRQNumber, &IRQType, &IRQInfo);


    switch (IRQType)
    {
    case IRQType_Timer:
        portAckTimerIRQ(CurrentIRQNumber, IRQInfo);
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
        YIELD = portMTD_ECC_ISR();
        break;
    case IRQType_DISP:
        portDISP_ISR();
        break;

    default:
        PANNIC("Unknown IRQ:%d,%d\n", CurrentIRQNumber, IRQInfo);
        break;
    }

    

    portAckIRQ(CurrentIRQNumber);

    return YIELD;
}

void IRQInit()
{
    portIRQCtrlInit();

}

