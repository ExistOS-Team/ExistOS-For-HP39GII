#include "FreeRTOS.h"
#include "task.h"

#include "timer_up.h"
#include "interrupt_up.h"

#define LF_TimerFreq    (configTICK_RATE_HZ)

//volatile unsigned long ulHighFrequencyTimerTicks;

uint32_t g_tick_cnt = 0;

int LFTimer = -1;


bool up_TimerSetup( void ){

    portTimerInit();

    LFTimer = portGetTimer();
    
    portSetTimerPeriod(LFTimer, 1000000 / (LF_TimerFreq));

    portEnableTimerIRQ(LFTimer, true);

    portEnableTimer(LFTimer, true);
    
    return true;
}



void up_TimerTick()
{
    g_tick_cnt++;
    
    if( xTaskIncrementTick() != pdFALSE )
	{	
		vTaskSwitchContext();
	}
}


