#ifndef __LLAPI_H__
#define __LLAPI_H__

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"



#define EXCEPTION_IRQ      1
#define EXCEPTION_SWI      2


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


typedef struct LLAPI_KBD_t
{
    uint8_t key;
    uint8_t press;
}LLAPI_KBD_t;

QueueHandle_t LLAPI_Queue;
QueueHandle_t LLAPI_KBDQueue;

void LLAPI_init(TaskHandle_t upSys);
void LLAPI_Task(void);
void LLIRQ_task(void *pvParameters);

bool LL_timerTick();

void LLIRQ_PostIRQ(uint32_t IRQNum, uint32_t par1, uint32_t par2, uint32_t par3);

#endif

