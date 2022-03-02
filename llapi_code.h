#ifndef __LLAPI_CODE_H__
#define __LLAPI_CODE_H__

#define LL_SWI_BASE                     (0xEE00)
#define LL_SWI_NUM                      (255)


#define LL_SWI_DELAY_MS                (LL_SWI_BASE + 1)    //NO USE
#define LL_SWI_PUT_CH                  (LL_SWI_BASE + 2)
#define LL_SWI_WRITE_STRING1           (LL_SWI_BASE + 3)
#define LL_SWI_WRITE_STRING2           (LL_SWI_BASE + 4)
#define LL_SWI_GET_TIME_US             (LL_SWI_BASE + 5)

#define LL_SWI_ENABLE_IRQ              (LL_SWI_BASE + 6)
#define LL_SWI_DISABLE_IRQ             (LL_SWI_BASE + 7)
#define LL_SWI_SET_IRQ_VECTOR          (LL_SWI_BASE + 8)
#define LL_SWI_SET_IRQ_STACK           (LL_SWI_BASE + 9)


#define LL_SWI_IRQ_GET_CONTEXT         (LL_SWI_BASE + 10)
#define LL_SWI_IRQ_SET_CONTEXT         (LL_SWI_BASE + 11)
#define LL_SWI_IRQ_RESTORE_CONTEXT     (LL_SWI_BASE + 12)

#define LL_SWI_SYSTICK_SET_PERIOD      (LL_SWI_BASE + 13)
#define LL_SWI_SYSTICK_ENABLE          (LL_SWI_BASE + 14)


#define LL_IRQ_KEYBOARD                (0)
#define LL_IRQ_TIMER                   (1)

#endif

