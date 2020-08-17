#pragma once

#include "hw_irq.h"

#define IRQ_N(n) (n + HW_IRQ_TIMER0)

/*Call this function to initialize.*/
void timer_init();

/*Call this function to set the fixed count and the callback function of a timer.
    n: 0~3. Specify the timer.
    count: 0~65535. It decreases by 1 every 1/32 ms.
    callback: The address of the callback function.

    return value: If the timer is successfully set(1) or not(0).*/
char timer_set(char n, unsigned short count, unsigned int *callback);

/*Call this function to start a timer.
    n: 0~3. Specify the timer.*/
void timer_start(char n);

/*Call this function to stop a timer. This function will also clear the settings set by timer_set.
    n: 0~3. Specify the timer.*/
void timer_stop(char n);


/* Some notes from the manual: 

p595: Recall that the APBX typically runs at a divided clock rate from the 24-MHz crystal clock (6 MHz). 

P597: Selecting the ALWAYS tick causes the timer to decrement continuously at the rate established by the pre-scaled APBX clock.

P603: This bit must be cleared to 0 to enable operation of any timer or the rotary decoder. When set to 1, it forces a block-level reset and gates off the clocks to the block.

*/