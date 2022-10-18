#ifndef __LLAPI_CODE_H__
#define __LLAPI_CODE_H__

#include <stdint.h>
#include <stdbool.h>

#define LL_FAST_SWI_BASE                (0xEF00)
#define LL_SWI_BASE                     (0xEE00)
#define SYS_SWI_BASE                     (0xAC00)
#define LL_SWI_NUM                      (255)

#define LL_FAST_SWI_GET_STVAL          (LL_FAST_SWI_BASE + 0)
#define LL_FAST_SWI_SET_STVAL          (LL_FAST_SWI_BASE + 1)

#define LL_SWI_PUT_CH                  (LL_SWI_BASE + 2)
#define LL_SWI_WRITE_STRING1           (LL_SWI_BASE + 3)
#define LL_SWI_WRITE_STRING2           (LL_SWI_BASE + 4)

#define LL_FAST_SWI_GET_TIME_US        (LL_FAST_SWI_BASE + 5)
#define LL_FAST_SWI_VM_SLEEP_MS        (LL_FAST_SWI_BASE + 6)
#define LL_FAST_SWI_GET_TIME_MS        (LL_FAST_SWI_BASE + 7)



#define LL_SWI_ENABLE_IRQ              (LL_SWI_BASE + 7)
#define LL_SWI_SET_IRQ_VECTOR          (LL_SWI_BASE + 8)
#define LL_SWI_SET_IRQ_STACK           (LL_SWI_BASE + 9)
#define LL_SWI_SET_CONTEXT             (LL_SWI_BASE + 10)
#define LL_SWI_GET_CONTEXT             (LL_SWI_BASE + 11)
#define LL_SWI_RESTORE_CONTEXT         (LL_SWI_BASE + 12)
#define LL_SWI_SET_SVC_VECTOR          (LL_SWI_BASE + 13)
#define LL_SWI_SET_SVC_STACK           (LL_SWI_BASE + 14)
#define LL_SWI_ENABLE_TIMER            (LL_SWI_BASE + 15)


#define LL_SWI_DISPLAY_FLUSH           (LL_SWI_BASE + 21)
#define LL_SWI_DISPLAY_SET_INDICATION  (LL_SWI_BASE + 22)


#define LL_SWI_SET_KEY_REPORT          (LL_SWI_BASE + 30)
#define LL_FAST_SWI_CHECK_KEY          (LL_FAST_SWI_BASE + 31)


#define LL_SWI_PWR_POWEROFF            (LL_SWI_BASE + 40)
#define LL_SWI_PWR_RESET               (LL_SWI_BASE + 41)
#define LL_SWI_PWR_HIBERNATE           (LL_SWI_BASE + 42)
#define LL_FAST_SWI_PWR_VOLTAGE        (LL_FAST_SWI_BASE + 43)
#define LL_FAST_SWI_CORE_TEMP          (LL_FAST_SWI_BASE + 44)
#define LL_SWI_PWR_SPEED               (LL_SWI_BASE + 45)
#define LL_FAST_SWI_RTC_GET_SEC        (LL_FAST_SWI_BASE + 46)
#define LL_FAST_SWI_RTC_SET_SEC        (LL_FAST_SWI_BASE + 47)

#define LL_SWI_SET_SERIALPORT          (LL_SWI_BASE + 50)
#define LL_SWI_SERIAL_GETCH            (LL_SWI_BASE + 51)
#define LL_SWI_SERIAL_RX_COUNT         (LL_SWI_BASE + 52)

#define LL_SWI_CLKCTL_GET_DIV          (LL_SWI_BASE + 60)
#define LL_SWI_CLKCTL_SET_DIV          (LL_SWI_BASE + 61)



#define LL_SWI_FLASH_PAGE_READ         (LL_SWI_BASE + 70)
#define LL_SWI_FLASH_PAGE_WRITE        (LL_SWI_BASE + 71)
#define LL_SWI_FLASH_PAGE_TRIM         (LL_SWI_BASE + 72)
#define LL_SWI_FLASH_SYNC              (LL_SWI_BASE + 73)
#define LL_SWI_FLASH_PAGE_NUM          (LL_SWI_BASE + 74)
#define LL_SWI_FLASH_PAGE_SIZE_B       (LL_SWI_BASE + 75)

#define LL_FAST_SWI_SYSTEM_IDLE             (LL_FAST_SWI_BASE + 80)
#define LL_FAST_SWI_CORE_CUR_FREQ           (LL_FAST_SWI_BASE + 81)

#define LL_FAST_SWI_GET_CHARGE_STATUS     (LL_FAST_SWI_BASE + 82)
#define LL_SWI_CHARGE_ENABLE              (LL_SWI_BASE + 83)
#define LL_SWI_SLOW_DOWN_ENABLE              (LL_SWI_BASE + 84)
#define LL_SWI_SLOW_DOWN_MINFRAC              (LL_SWI_BASE + 85)



#define LL_FAST_SWI_PCM_BUFFER_IS_IDLE        (LL_FAST_SWI_BASE + 90)
#define LL_FAST_SWI_PCM_BUFFER_PLAY           (LL_FAST_SWI_BASE + 91)


#define LL_IRQ_SERIAL                  (0)
#define LL_IRQ_KEYBOARD                (1)
#define LL_IRQ_TIMER                   (2)
#define LL_IRQ_MMU                     (3)



#define SYS_APP_EXIT                            (SYS_SWI_BASE + 1)
#define SYS_APP_SLEEP_MS                        (SYS_SWI_BASE + 2)


#endif

