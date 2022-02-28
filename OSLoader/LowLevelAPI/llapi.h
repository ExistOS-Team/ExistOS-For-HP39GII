#ifndef __LLAPI_H__
#define __LLAPI_H__

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"

typedef struct LLAPI_CallInfo_t
{
    TaskHandle_t task;
    uint32_t SWINum;
    uint32_t para0;
    uint32_t para1;
    uint32_t para2;
    uint32_t para3;
    uint32_t *pRet;
}LLAPI_CallInfo_t;

QueueHandle_t LLAPI_Queue;

void LLAPI_init(void);
void LLAPI_Task(void);

#endif

