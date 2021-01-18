#pragma once

#include "hw_irq.h"
#ifdef __cplusplus
extern "C" {
#endif
#define IRQ_N(n) (n + HW_IRQ_TIMER0)

/* Warning: Arguments out of range could cause unexpected results. */

/*Initialize.*/
void timer_init();

/*Set some properties of a timer before you start it.
    n: 0~3. Specify the timer.
    isRepeat: 0 or 1. Whether the timewr will auto reload.
    accuracy: Determine the length of a tick.
        0x0: Never tick.
        0x8: 1/32 ms.
        0x9: 1/16 ms.
        0xA: 1/8 ms.
        0xB: 1/4 ms.
        0xC: 1/24 us.
        0xD: 1/12 us.
        0xE: 1/6 us.
        0xF: 1/3 us.
    callback: The address of the callback function.
    return value: If the timer is successfully set(1) or not(0).*/
char timer_set(char n, char isRepeat, char accuracy, unsigned int *callback);

/*Start a timer.
    n: 0~3. Specify the timer.
    count: 0~65535. It decreases by 1 every 1/32 ms. Note that count should be set to real count - 1 for a repeating timer.*/
void timer_start(char n, unsigned short count);

/*Stop a timer. This function will clear the time count, so call timer_get after this is useless.
    n: 0~3. Specify the timer.*/
void timer_stop(char n);

/*This function must be called in the callback function of a repeating timer to make it work correctly. Important.
    n: 0~3. Specify the timer.*/
void timer_reset(char n);

/*Get the rest count of a timer.
    n: 0~3. Specify the timer.
    return value: The rest count of a timer.*/
unsigned short timer_get(char n);

/* Some notes from the manual: 

p595: Recall that the APBX typically runs at a divided clock rate from the 24-MHz crystal clock (6 MHz). 

P597: Selecting the ALWAYS tick causes the timer to decrement continuously at the rate established by the pre-scaled APBX clock.

P603: This bit must be cleared to 0 to enable operation of any timer or the rotary decoder. When set to 1, it forces a block-level reset and gates off the clocks to the block.

*/
#ifdef __cplusplus
}
#endif