#include "FreeRTOS.h"
#include "task.h"

#include "timer_up.h"
#include "interrupt_up.h"

#define LF_TimerFreq    (configTICK_RATE_HZ)
#define HF_TimerFreq    (configTICK_RATE_HZ * 5)

volatile unsigned long ulHighFrequencyTimerTicks;

int HFTimer = -1;
int LFTimer = -1;



bool up_TimerSetup( void ){

    if(portGetTimerNum() < 2){
        return false;
    }

    portTimerInit();

    HFTimer = portGetHighFrequencyTimer();
    LFTimer = portGetLowFrequencyTimer();
    
    portSetTimerPeriod(LFTimer, 1000000 / (LF_TimerFreq));
    portSetTimerPeriod(HFTimer, 1000000 / (HF_TimerFreq));

    portEnableTimerIRQ(HFTimer, true);
    portEnableTimerIRQ(LFTimer, true);

    portEnableTimer(HFTimer, true);
    portEnableTimer(LFTimer, true);
    
    return true;
}



void up_LowFrequencyTimerTick()
{
    if( xTaskIncrementTick() != pdFALSE )
	{	
		vTaskSwitchContext();
	}
}


void up_HighFrequencyTimerTick()
{
	ulHighFrequencyTimerTicks++;
}
