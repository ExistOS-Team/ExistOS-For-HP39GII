#include "rtc.h"
#include "regsrtc.h"

inline void rtc_init(){
    BW_RTC_CTRL_CLKGATE(0);
    BW_RTC_PERSISTENT0_XTAL24MHZ_PWRUP(1);
}

inline void rtc_ms_set(unsigned int count){
    HW_RTC_MILLISECONDS_WR(count);
}

inline unsigned int rtc_ms_get(){
    return HW_RTC_MILLISECONDS_RD();
}

inline void rtc_ms_reset(){
    HW_RTC_MILLISECONDS_CLR(BM_RTC_MILLISECONDS_COUNT);
}

inline char rtc_sec_set(unsigned int count){
    if (HW_RTC_STAT_RD() & 0x8000) {
        return 0;
    }else{
        HW_RTC_SECONDS_WR(count);
        return 1;
    }
}

inline unsigned int rtc_sec_get(){
    return HW_RTC_SECONDS_RD();
}

inline char rtc_persistent_set(char n, unsigned int general){
    if (HW_RTC_STAT_RD() & (0x1 << n + BP_RTC_STAT_NEW_REGS)) {
        return 0;
    }else{
        HW_RTC_PERSISTENTn_WR(n, general);
        return 1;     
    }
}

inline unsigned int rtc_persistent_get(char n){
    return HW_RTC_PERSISTENTn_RD(n);
}