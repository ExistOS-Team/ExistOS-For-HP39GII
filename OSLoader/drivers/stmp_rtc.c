


#include "regsrtc.h"

#include "rtc_up.h"


void portRTC_init()
{
    while(HW_RTC_STAT.B.NEW_REGS);

    HW_RTC_PERSISTENT0.B.XTAL32KHZ_PWRUP = 1;

    while(HW_RTC_STAT.B.NEW_REGS);

    HW_RTC_PERSISTENT0.B.CLOCKSOURCE = 1;

    while(HW_RTC_STAT.B.NEW_REGS);

    HW_RTC_PERSISTENT0.B.AUTO_RESTART = 0;

    while(HW_RTC_STAT.B.NEW_REGS);

    HW_RTC_PERSISTENT0.B.DISABLE_PSWITCH = 1;

    while(HW_RTC_STAT.B.NEW_REGS);



}

uint32_t rtc_get_seconds()
{
    return HW_RTC_SECONDS.B.COUNT;
}

void rtc_set_seconds(uint32_t s)
{
    
    while(HW_RTC_STAT.B.NEW_REGS);
    HW_RTC_SECONDS_WR(s);
    while(HW_RTC_STAT.B.NEW_REGS);
}