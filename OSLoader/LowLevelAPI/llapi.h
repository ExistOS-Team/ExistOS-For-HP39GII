#ifndef __LLAPI_H__
#define __LLAPI_H__

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"

#define L_SWI       0
#define L_IRQ       1
#define L_PAB       2
#define L_DAB       3
#define L_UND       4



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
    uint32_t sp; 
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

extern QueueHandle_t LLAPI_Queue;
extern QueueHandle_t LLIRQ_Queue;
extern QueueHandle_t LLAPI_KBDQueue;

void LLAPI_init(TaskHandle_t upSys);
void LLAPI_Task(void);
void LLIRQ_task(void *pvParameters);
void LLIO_ScanTask(void *pvParameters);

void LLIO_NotifySerialRxAvailable();
void LLIO_NotifySerialTxAvailable();

void LLIRQ_ClearIRQs(void);
void LLAPI_ClearAPIs();
bool LLIRQ_enable(bool enable);
void LL_Scheduler_(uint32_t exception, uint32_t *SYSContext);

void LL_CheckIRQAndTrap();


void LLIRQ_PostIRQ(uint32_t IRQNum, uint32_t par1, uint32_t par2, uint32_t par3);

#endif

