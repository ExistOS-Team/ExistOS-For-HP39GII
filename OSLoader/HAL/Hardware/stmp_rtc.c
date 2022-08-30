


#include "regsrtc.h"

#include "rtc_up.h"


void portRTC_init()
{



}

uint32_t rtc_get_seconds()
{
    return HW_RTC_SECONDS.B.COUNT;
}

void rtc_set_seconds(uint32_t s)
{
    HW_RTC_SECONDS_WR(s);
}