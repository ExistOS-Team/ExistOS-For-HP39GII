#ifndef __TIMER_H__
#define __TIMER_H__

#include <stdbool.h>
#include "interrupt_up.h"



int portGetTimerNum(void);
bool portSetTimerPeriod(int timer, unsigned int us);
bool portEnableTimerIRQ(int timer, bool enable);
bool portEnableTimer(int timer, bool enable);
void portTimerInit(void);

void portAckTimerIRQ(void);
int portGetTimer(void);

bool up_TimerSetup(void);
void up_TimerIRQ(IRQNumber IRQNum);
void up_TimerTick(void);

#endif