#pragma once

#include "regsrtc.h"

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

void rtc_init();

void rtc_ms_set(unsigned int count);

unsigned int rtc_ms_get();

void rtc_ms_reset();

char rtc_sec_set(unsigned int count);

unsigned int rtc_sec_get();

char rtc_persistent_set(char n, unsigned int general);

unsigned int rtc_persistent_get(char n);