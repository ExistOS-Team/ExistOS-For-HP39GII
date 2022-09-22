#ifndef __INTERRUPT_UP_H__
#define __INTERRUPT_UP_H__

#include <stdbool.h>


#define MAX_IRQ_NUM     63

typedef int IRQNumber;
typedef unsigned int IRQInfo;

typedef enum IRQTypes
{
    IRQType_Timer = 0,
    IRQType_Display,
    IRQType_Pinctrl,
    IRQType_USBCtrl,
    IRQType_MTD_DMA,
    IRQType_MTD_ECC,
    IRQType_MTD,
    IRQType_DISP,
    IRQType_LRADC,
    IRQType_PWR,
    IRQType_DAC

}IRQTypes;


typedef void (*IRQCallback)(IRQNumber IRQNum);

typedef struct
{
    IRQTypes IRQType;
    IRQCallback cb;
}IRQList;


bool portIRQDecode(IRQNumber* IRQNum, IRQTypes *IRQType, IRQInfo *IRQInfo);
void portIRQCtrlInit( void );
void portAckIRQ(IRQNumber IRQNum);
void portEnableIRQ(unsigned int IRQNum, unsigned int enable);


void register_ISR(IRQNumber IRQNum, IRQTypes IRQType, IRQCallback cb);
void up_isr( void );
void IRQInit( void );
void IRQTimerEnable( void );

#endif