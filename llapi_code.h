#ifndef __LLAPI_CODE_H__
#define __LLAPI_CODE_H__

#include <stdint.h>
#include <stdbool.h>

typedef struct CreateTaskInfo_t
{
    void *entry;
    void *stack;
    char *name;
    uint8_t priority;
    uint32_t stackSize;
    void *passParameter;

}CreateTaskInfo_t;

typedef struct DispFlushInfo_t
{
    uint8_t *vram;
    uint32_t x0;
    uint32_t y0;
    uint32_t x1;
    uint32_t y1;
}DispFlushInfo_t;

typedef struct DispPutStrInfo_t
{
    char *string;
    uint32_t x0;
    uint32_t y0;
    uint8_t fg;
    uint8_t bg;
    uint8_t fontsize;
}DispPutStrInfo_t;

typedef struct DispPutBoxInfo_t
{
    uint32_t x0;
    uint32_t y0;
    uint32_t x1;
    uint32_t y1;
    bool fill;
    uint8_t color;
}DispPutBoxInfo_t;


#define LL_SWI_BASE                     (0xEE00)
#define LL_SWI_NUM                      (255)


#define LL_SWI_DELAY_MS                (LL_SWI_BASE + 1)    //NO USE
#define LL_SWI_PUT_CH                  (LL_SWI_BASE + 2)
#define LL_SWI_WRITE_STRING1           (LL_SWI_BASE + 3)
#define LL_SWI_WRITE_STRING2           (LL_SWI_BASE + 4)
#define LL_SWI_GET_TIME_US             (LL_SWI_BASE + 5)



#define LL_SWI_ENABLE_IRQ              (LL_SWI_BASE + 7)
#define LL_SWI_SET_IRQ_VECTOR          (LL_SWI_BASE + 8)
#define LL_SWI_SET_IRQ_STACK           (LL_SWI_BASE + 9)
#define LL_SWI_LOAD_CONTEXT            (LL_SWI_BASE + 10)
#define LL_SWI_SET_CONFIG_ADDR         (LL_SWI_BASE + 11)



#define LL_SWI_ENABLE_TIMER            (LL_SWI_BASE + 14)
#define LL_SWI_YEILD                   (LL_SWI_BASE + 15)


#define LL_SWI_DISPLAY_PUT_BOX         (LL_SWI_BASE + 17)
#define LL_SWI_DISPLAY_HLINE           (LL_SWI_BASE + 18)
#define LL_SWI_DISPLAY_VLINE           (LL_SWI_BASE + 19)
#define LL_SWI_DISPLAY_PUTSTR          (LL_SWI_BASE + 20)
#define LL_SWI_DISPLAY_CLEAR           (LL_SWI_BASE + 21)
#define LL_SWI_DISPLAY_FLUSH           (LL_SWI_BASE + 22)
#define LL_SWI_DISPLAY_SETINDICATE     (LL_SWI_BASE + 23)
#define LL_SWI_DISPLAY_SEND_SCREEN     (LL_SWI_BASE + 24)





#define LL_IRQ_KEYBOARD                (0)
#define LL_IRQ_TIMER                   (1)
#define LL_IRQ_YEILD                   (2)

#endif

