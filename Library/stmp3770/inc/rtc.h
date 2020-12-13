#pragma once

#include "regsrtc.h"
#ifdef __cplusplus 
extern "C" { 
#endif

#define HW_RTC_PERSISTENTn_ADDR(n)      (REGS_RTC_BASE + 0x00000060 + n * 0x10)
#define HW_RTC_PERSISTENTn_SET_ADDR(n)  (REGS_RTC_BASE + 0x00000064 + n * 0x10)
#define HW_RTC_PERSISTENTn_CLR_ADDR(n)  (REGS_RTC_BASE + 0x00000068 + n * 0x10)
#define HW_RTC_PERSISTENTn_TOG_ADDR(n)  (REGS_RTC_BASE + 0x0000006C + n * 0x10)

#ifndef __LANGUAGE_ASM__
#define HW_RTC_PERSISTENTn(n)           (*(volatile hw_rtc_persistent2_t *) HW_RTC_PERSISTENTn_ADDR(n))
#define HW_RTC_PERSISTENTn_RD(n)        (HW_RTC_PERSISTENTn(n).U)
#define HW_RTC_PERSISTENTn_WR(n,v)      (HW_RTC_PERSISTENTn(n).U = (v))
#define HW_RTC_PERSISTENTn_SET(n,v)     ((*(volatile reg32_t *) HW_RTC_PERSISTENTn_SET_ADDR(n)) = (v))
#define HW_RTC_PERSISTENTn_CLR(n,v)     ((*(volatile reg32_t *) HW_RTC_PERSISTENTn_CLR_ADDR(n)) = (v))
#define HW_RTC_PERSISTENTn_TOG(n,v)     ((*(volatile reg32_t *) HW_RTC_PERSISTENTn_TOG_ADDR(n)) = (v))
#endif

/* Warning: Arguments out of range could cause unexpected results. */

/*Initialize the second counter. Theoretically the millisecond counter and the persistent registers need not initialization.
    return value: If it is successfully initialized(1) or not(0).*/
char rtc_init();

/*Set the current count of the millisecond counter.
    count: The value to set.*/
void rtc_ms_set(unsigned int count);

/*Get the value of the millisecond counter.
    return value: The current count of the millisecond counter.*/
unsigned int rtc_ms_get();

/*Reset the current count of the millisecond counter to 0.*/
void rtc_ms_reset();

/*Set the current count of the second counter.
    count: The value to set.
    return value: If the counter is successfully set(1) or not(0).*/
char rtc_sec_set(unsigned int count);

/*Get the value of the second counter.
    return value: The current count of the second counter.*/
unsigned int rtc_sec_get();

/*Set the value of a persistent register.
    n: 2~5. Specify the register.
    general: The value put into the register.
    return value: If the register is successfully set(1) or not(0).*/
char rtc_persistent_set(char n, unsigned int general);

/*Get the value of a persistent register.
    n: 2~5. Specify the register. 
    return value: The value of a persistent register.*/
unsigned int rtc_persistent_get(char n);
#ifdef __cplusplus 
}
#endif
