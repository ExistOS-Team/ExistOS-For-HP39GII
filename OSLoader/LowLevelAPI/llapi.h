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


typedef struct LLIRQ_Info_t
{
    uint32_t IRQNum;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
}LLIRQ_Info_t;

QueueHandle_t LLAPI_Queue;
QueueHandle_t LLIRQ_Queue;

void LLAPI_init(TaskHandle_t upSys);
void LLAPI_Task(void);
void LLIRQ_task(void *pvParameters);

void LLIRQ_PostIRQ(uint32_t IRQNum, uint32_t par1, uint32_t par2, uint32_t par3);

#endif

