

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "llapi_code.h"
#include "sys_llapi.h"


#undef DECDEF_LLSWI
#define DECDEF_LLSWI(ret, name, pars, SWINum)                              \
    ret NAKED name pars                                                 \
    {                                                                   \
        __asm volatile("swi %0" :: "i"(SWINum));                        \
        __asm volatile("bx lr");                                        \
    }

#ifdef __cplusplus
    extern "C" {
#endif
//    Return Type       Function Name       Parameters                               LL SWI Number
DECDEF_LLSWI(void,         ll_put_str,            (char *s)                               ,LL_SWI_WRITE_STRING1           );
DECDEF_LLSWI(void,         ll_put_str2,           (char *s, uint32_t len)                 ,LL_SWI_WRITE_STRING2           );
DECDEF_LLSWI(void,         ll_put_ch,             (char c)                                ,LL_SWI_PUT_CH                  );
DECDEF_LLSWI(uint32_t,     ll_get_time_us,        (void)                                  ,LL_FAST_SWI_GET_TIME_US        );
DECDEF_LLSWI(uint32_t,     ll_get_time_ms,        (void)                                  ,LL_FAST_SWI_GET_TIME_MS        );
DECDEF_LLSWI(void,         ll_vm_sleep_ms,        (uint32_t ms)                           ,LL_FAST_SWI_VM_SLEEP_MS        );
DECDEF_LLSWI(uint32_t,     ll_vm_check_key,       (void)                                  ,LL_FAST_SWI_CHECK_KEY          );
DECDEF_LLSWI(void,         ll_set_keyboard,       (bool enable_report)                    ,LL_SWI_SET_KEY_REPORT          );
DECDEF_LLSWI(void,         ll_set_serial,         (bool enable)                           ,LL_SWI_SET_SERIALPORT          );
DECDEF_LLSWI(void,         ll_set_timer,          (bool enbale, uint32_t period_ms)       ,LL_SWI_ENABLE_TIMER            );
DECDEF_LLSWI(void,         ll_set_irq_vector,     (uint32_t addr)                         ,LL_SWI_SET_IRQ_VECTOR          );
DECDEF_LLSWI(void,         ll_set_irq_stack,      (uint32_t addr)                         ,LL_SWI_SET_IRQ_STACK           );
DECDEF_LLSWI(void,         ll_set_svc_vector,     (uint32_t addr)                         ,LL_SWI_SET_SVC_VECTOR          );
DECDEF_LLSWI(void,         ll_set_svc_stack,      (uint32_t addr)                         ,LL_SWI_SET_SVC_STACK           );
DECDEF_LLSWI(void,         ll_set_context,        (uint32_t addr, bool en_IRQ)            ,LL_SWI_SET_CONTEXT             );
DECDEF_LLSWI(void,         ll_restore_context,    (uint32_t addr, bool en_IRQ)            ,LL_SWI_RESTORE_CONTEXT         );
DECDEF_LLSWI(void,         ll_get_context,        (uint32_t addr)                         ,LL_SWI_GET_CONTEXT             );
DECDEF_LLSWI(void,         ll_enable_irq,         (bool enable)                           ,LL_SWI_ENABLE_IRQ              );
DECDEF_LLSWI(void,         ll_disp_put_area,      (uint8_t *vbuffer,
                                                   uint32_t x0, uint32_t y0,
                                                   uint32_t x1, uint32_t y1)              ,LL_SWI_DISPLAY_FLUSH           );

DECDEF_LLSWI(void,         ll_disp_set_indicator, (int indicateBit, int BatInt)           ,LL_SWI_DISPLAY_SET_INDICATION  );

DECDEF_LLSWI(uint32_t,     ll_serial_getch,       (void)                                  ,LL_SWI_SERIAL_GETCH             );
DECDEF_LLSWI(uint32_t,     ll_serial_rx_count,    (void)                                  ,LL_SWI_SERIAL_RX_COUNT          );
DECDEF_LLSWI(uint32_t,     ll_get_tmp_storage_val,(uint32_t index)                        ,LL_FAST_SWI_GET_STVAL          );
DECDEF_LLSWI(void,         ll_set_tmp_storage_val,(uint32_t index, uint32_t val)          ,LL_FAST_SWI_SET_STVAL          );

DECDEF_LLSWI(void,         ll_set_clkctrl_div,    (uint32_t cpu_div, uint32_t cpu_frac,
                                                             uint32_t hclk_frac)          ,LL_SWI_CLKCTL_SET_DIV          );
DECDEF_LLSWI(void,         ll_get_clkctrl_div,    (uint32_t *save)                        ,LL_SWI_CLKCTL_GET_DIV          );
DECDEF_LLSWI(int,          ll_flash_page_read,    (uint32_t start_page, uint32_t pages,
                                                   uint8_t *buffer)                       ,LL_SWI_FLASH_PAGE_READ          );
DECDEF_LLSWI(int,          ll_flash_page_write,   (uint32_t start_page, uint32_t pages,
                                                   uint8_t *buffer)                       ,LL_SWI_FLASH_PAGE_WRITE         );
DECDEF_LLSWI(void,         ll_flash_page_trim,    (uint32_t page)                         ,LL_SWI_FLASH_PAGE_TRIM          );
DECDEF_LLSWI(void,         ll_flash_sync,         (void)                                  ,LL_SWI_FLASH_SYNC               );
DECDEF_LLSWI(uint32_t,     ll_flash_get_pages,    (void)                                  ,LL_SWI_FLASH_PAGE_NUM           );
DECDEF_LLSWI(uint32_t,     ll_flash_get_page_size,(void)                                  ,LL_SWI_FLASH_PAGE_SIZE_B        );

DECDEF_LLSWI(uint32_t,     ll_power_off,          (void)                                  ,LL_SWI_PWR_POWEROFF             );
DECDEF_LLSWI(uint32_t,     ll_get_bat_voltage,    (void)                                  ,LL_FAST_SWI_PWR_VOLTAGE             );
DECDEF_LLSWI(uint32_t,     ll_get_pwrspeed  ,    (void)                                  ,LL_SWI_PWR_SPEED             );
DECDEF_LLSWI(uint32_t,     ll_get_core_temp,    (void)                                  ,LL_FAST_SWI_CORE_TEMP             );
DECDEF_LLSWI(uint32_t,     ll_get_cur_freq,    (void)                                     ,LL_FAST_SWI_CORE_CUR_FREQ             );
DECDEF_LLSWI(uint32_t,     ll_get_charge_status,    (void)                                     ,LL_FAST_SWI_GET_CHARGE_STATUS             );
DECDEF_LLSWI(uint32_t,     ll_charge_enable,    (bool enable)                               ,LL_SWI_CHARGE_ENABLE             );
DECDEF_LLSWI(uint32_t,     ll_cpu_slowdown_enable,    (bool enable)                               ,LL_SWI_SLOW_DOWN_ENABLE             );
DECDEF_LLSWI(uint32_t,     ll_cpu_slowdown_min_frac,    (uint32_t val)                               ,LL_SWI_SLOW_DOWN_MINFRAC            );
DECDEF_LLSWI(uint32_t,     ll_rtc_get_sec,    (void)                                         ,LL_FAST_SWI_RTC_GET_SEC            );
DECDEF_LLSWI(void,         ll_rtc_set_sec,    (uint32_t val)                                 ,LL_FAST_SWI_RTC_SET_SEC            );

DECDEF_LLSWI(uint32_t,     ll_pcm_buffer_idle,              (void)                                   ,LL_FAST_SWI_PCM_BUFFER_IS_IDLE            );
DECDEF_LLSWI(void,         ll_pcm_buffer_load,              (uint32_t addr)                          ,LL_FAST_SWI_PCM_BUFFER_PLAY               );

DECDEF_LLSWI(uint32_t,     ll_system_idle,      (void)                                    ,LL_FAST_SWI_SYSTEM_IDLE             );



#ifdef __cplusplus          
    }          
#endif

