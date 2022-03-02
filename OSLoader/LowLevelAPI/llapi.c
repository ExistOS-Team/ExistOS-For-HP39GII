
#include <string.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "timers.h"

#include "board_up.h"

#include "llapi.h"
#include "llapi_code.h"

#include "../debug.h"

typedef struct LLAPI_TimerInfo_t
{
    TaskHandle_t forTask;
    struct LLAPI_TimerInfo_t *self;
}LLAPI_TimerInfo_t;

static bool LLEnableIRQ;
static bool LLInIRQ;

static TaskHandle_t upSystem;

static volatile uint32_t savedContext[16];

static uint32_t LL_IRQVector;
static uint32_t LL_IRQStack;

static xTimerHandle LL_SYSTimer;
static uint32_t LL_SYSTimerPeriod;


static void LL_TrapInIRQ(uint32_t IRQNum, uint32_t par1, uint32_t par2, uint32_t par3)
{
    uint32_t *r;
    vTaskSuspend(upSystem);
    portYIELD();
    LLInIRQ = true;
    r = (uint32_t *)(((uint32_t *)upSystem)[1]) - 16;
    memcpy(savedContext, r, sizeof(savedContext));


    r[0] = IRQNum;
    r[1] = par1;
    r[2] = par2;
    r[3] = par3;


    r[13] = LL_IRQStack;
    r[15] = LL_IRQVector + 4; // against   "SUBS  PC,R14,#4"

    vTaskResume(upSystem);
}

static void LL_IRQReturn()
{
    uint32_t *r;
    vTaskSuspend(upSystem);
    portYIELD();
    r = (uint32_t *)(((uint32_t *)upSystem)[1]) - 16;

    LL_IRQStack = r[13];

    memcpy(r, savedContext, sizeof(savedContext));

    vTaskResume(upSystem);
    LLInIRQ = false;

}





void LLAPI_TimerCallBack(TimerHandle_t xTimer)
{
    LLAPI_TimerInfo_t *lltmr = pvTimerGetTimerID(xTimer);
    TaskHandle_t task = lltmr->forTask;
    vPortFree(lltmr->self);
    if(!LLInIRQ)
        vTaskResume(task);
    xTimerDelete(xTimer, 0);
}



void LLIRQ_task(void *pvParameters)
{
    LLIRQ_Info_t curIRQ;
    

    for(;;)
    {

        while(xQueueReceive(LLIRQ_Queue, &curIRQ, portMAX_DELAY) == pdTRUE){
            if(LLEnableIRQ){
                if(LLInIRQ == false){
                    //LLAPI_INFO("LL_TrapInIRQ\n");
                    LL_TrapInIRQ(curIRQ.IRQNum, curIRQ.r1, curIRQ.r2, curIRQ.r3);
                }
            }
        }

    }
}

void LLIRQ_PostIRQ(uint32_t IRQNum, uint32_t par1, uint32_t par2, uint32_t par3)
{
    LLIRQ_Info_t n;
    //LLAPI_INFO("LLIRQ_PostIRQ\n");
    n.IRQNum = IRQNum;
    n.r1 = par1;
    n.r2 = par2;
    n.r3 = par3;
    xQueueSend(LLIRQ_Queue, &n, pdMS_TO_TICKS(1));
}

void LL_SysTimerCallBack(xTimerHandle timer)
{
    LLIRQ_PostIRQ(LL_IRQ_TIMER, 0, 0, 0);
}

void LLAPI_init(TaskHandle_t upSys)
{
    upSystem = upSys;

    LLEnableIRQ = false;
    LLInIRQ = false;

    LL_SYSTimerPeriod = portMAX_DELAY;

    LL_IRQVector = 0xF0F00000;
    LL_IRQStack  = 0xF0FF0000;
    

    LLAPI_Queue = xQueueCreate(32, sizeof(LLAPI_CallInfo_t));
    LLIRQ_Queue = xQueueCreate(32, sizeof(LLIRQ_Info_t));

    LL_SYSTimer = xTimerCreate("LLSYSTimer", LL_SYSTimerPeriod, pdTRUE, NULL, LL_SysTimerCallBack);
    
}

void LLAPI_Task()
{
    LLAPI_CallInfo_t currentCall;
    LLAPI_TimerInfo_t *lltmr;
    TimerHandle_t xTimer;
    for(;;)
    {
        while(xQueueReceive(LLAPI_Queue, &currentCall, portMAX_DELAY) == pdTRUE){
            

/*
            LLAPI_INFO("task:[%s] SWI NUM:%06x, r0:%08x, r1:%08x, r2:%08x, r3:%08x\n",
                pcTaskGetName(currentCall.task), currentCall.SWINum, currentCall.para0,
                currentCall.para1, currentCall.para2, currentCall.para3);
*/
            switch (currentCall.SWINum)
            {
            case LL_SWI_DELAY_MS:
                vTaskSuspend(currentCall.task);
                lltmr = pvPortMalloc(sizeof(LLAPI_TimerInfo_t));
                lltmr->forTask = currentCall.task;
                lltmr->self = lltmr;
                xTimer = xTimerCreate("SWI_DELAY_MS", pdMS_TO_TICKS(currentCall.para0), 0, lltmr, LLAPI_TimerCallBack);
                xTimerStart(xTimer, 0);
                break;
            
            case LL_SWI_WRITE_STRING1:
                vTaskSuspend(currentCall.task);
                *currentCall.pRet = strlen((char *)currentCall.para0);
                printf("%s", (char *)currentCall.para0);
                vTaskResume(currentCall.task);
                break;

            case LL_SWI_WRITE_STRING2:
                vTaskSuspend(currentCall.task);
                for(int i=0;i<currentCall.para1;i++){
                    putchar(((char *)currentCall.para0)[i]);
                }
                vTaskResume(currentCall.task);
                break;

            
            case LL_SWI_PUT_CH:
                vTaskSuspend(currentCall.task);
                *currentCall.pRet = 1;
                printf("%c", currentCall.para0);
                vTaskResume(currentCall.task);
                break;
            
            case LL_SWI_GET_TIME_US:
                vTaskSuspend(currentCall.task);
                *currentCall.pRet = portBoardGetTime_us();
                vTaskResume(currentCall.task);
                break;

            case LL_SWI_ENABLE_IRQ:
                vTaskSuspend(currentCall.task);
                //LLAPI_INFO("IRQ ENABLE\n");
                LLEnableIRQ = true;
                vTaskResume(currentCall.task);
                break;


            case LL_SWI_DISABLE_IRQ:
                vTaskSuspend(currentCall.task);
                //LLAPI_INFO("IRQ DISABLE\n");
                LLEnableIRQ = false;
                vTaskResume(currentCall.task);
                break;

            case LL_SWI_SET_IRQ_VECTOR:
                vTaskSuspend(currentCall.task);
                LL_IRQVector = *currentCall.pRet;
                LLAPI_INFO("SET IRQ VECTOR:%08x\n", LL_IRQVector);
                vTaskResume(currentCall.task);
                break;

            case LL_SWI_IRQ_RESTORE_CONTEXT:
                vTaskSuspend(currentCall.task);
                LLAPI_INFO("IRQ RESTORE\n");
                LL_IRQReturn();
                vTaskResume(currentCall.task);
                break;

            case LL_SWI_IRQ_GET_CONTEXT:
                vTaskSuspend(currentCall.task);
                memcpy((uint32_t *)(*currentCall.pRet), savedContext, sizeof(savedContext));
                vTaskResume(currentCall.task);
                break;

            case LL_SWI_IRQ_SET_CONTEXT:
                vTaskSuspend(currentCall.task);
                memcpy(savedContext, (uint32_t *)(*currentCall.pRet), sizeof(savedContext));
                vTaskResume(currentCall.task);
                break;

            case LL_SWI_SET_IRQ_STACK:
                vTaskSuspend(currentCall.task);
                LL_IRQStack = *currentCall.pRet;
                LLAPI_INFO("SET IRQ STACK:%08x\n", LL_IRQStack);
                vTaskResume(currentCall.task);
                break;

            case LL_SWI_SYSTICK_ENABLE:
                vTaskSuspend(currentCall.task);
                if(currentCall.para0){
                    xTimerStart(LL_SYSTimer, 0);
                }else{
                    xTimerStop(LL_SYSTimer, 0);
                }
                vTaskResume(currentCall.task);
                break;
            case LL_SWI_SYSTICK_SET_PERIOD:
                vTaskSuspend(currentCall.task);
                LL_SYSTimerPeriod = pdMS_TO_TICKS(currentCall.para0);
                if(LL_SYSTimerPeriod == 0){
                    xTimerStop(LL_SYSTimer, 0);
                }else{
                    xTimerChangePeriod(LL_SYSTimer, LL_SYSTimerPeriod, 0);
                }
                vTaskResume(currentCall.task);
                break;

            default:
                break;
            }
        }
        
    }
}







