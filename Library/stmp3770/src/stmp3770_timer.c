#include "stmp3770_timer.h"
#include "irq.h"
#include "regstimrot.h"

inline void timer_init() {
    BW_TIMROT_ROTCTRL_SFTRST(0);
    BW_TIMROT_ROTCTRL_CLKGATE(0);
    for (int i = 0; i <= HW_TIMROT_TIMCTRLn_COUNT; i++) { // All 4 timers are available on this device.
        HW_TIMROT_TIMCTRLn_SET(i, BM_TIMROT_TIMCTRL3_UPDATE | BM_TIMROT_TIMCTRLn_IRQ_EN);
    }
}

inline char timer_set(char n, char isRepeat, char accuracy, unsigned int *callback) {
    if (n <= HW_TIMROT_TIMCTRLn_COUNT && !(HW_TIMROT_TIMCOUNTn_RD(n) & BM_TIMROT_TIMCOUNTn_FIXED_COUNT)) {
        BW_TIMROT_TIMCTRLn_RELOAD(n, isRepeat);
        if (accuracy > BV_TIMROT_TIMCTRLn_SELECT__TICK_ALWAYS) {
            BW_TIMROT_TIMCTRLn_SELECT(n, BV_TIMROT_TIMCTRLn_SELECT__TICK_ALWAYS);
            BW_TIMROT_TIMCTRLn_PRESCALE(n, accuracy - BV_TIMROT_TIMCTRLn_SELECT__TICK_ALWAYS);
        } else {
            BW_TIMROT_TIMCTRLn_SELECT(n, accuracy);
            BW_TIMROT_TIMCTRLn_PRESCALE(n, BV_TIMROT_TIMCTRLn_PRESCALE__DIV_BY_8);
        }
        irq_install_service(IRQ_N(n), callback);
        irq_set_enable(IRQ_N(n), 1);
        return 1;
    } else {
        return 0;
    }
}

inline void timer_start(char n, unsigned short count) {
    if (n <= HW_TIMROT_TIMCTRLn_COUNT) {
        BW_TIMROT_TIMCOUNTn_FIXED_COUNT(n, count);
    }
}

inline void timer_stop(char n) {
    if (n <= HW_TIMROT_TIMCTRLn_COUNT) {
        irq_set_enable(IRQ_N(n), 0);
        irq_install_service(IRQ_N(n), 0x0);
        BW_TIMROT_TIMCOUNTn_FIXED_COUNT(n, 0);
    }
}

inline void timer_reset(char n) {
    if (n <= HW_TIMROT_TIMCTRLn_COUNT) {
        BW_TIMROT_TIMCTRLn_IRQ(n, 0);
        //irq_set_enable(IRQ_N(n), 1);
    }
}

inline unsigned short timer_get(char n) {
    if (n <= HW_TIMROT_TIMCTRLn_COUNT) {
        return HW_TIMROT_TIMCOUNTn_RD(n) >> 16;
    } else {
        return 0;
    }
}