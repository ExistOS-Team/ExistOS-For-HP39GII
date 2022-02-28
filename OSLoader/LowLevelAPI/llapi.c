
#include <string.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "timers.h"

#include "llapi.h"
#include "llapi_code.h"

#include "../debug.h"

typedef struct LLAPI_TimerInfo_t
{
    TaskHandle_t forTask;
    struct LLAPI_TimerInfo_t *self;
}LLAPI_TimerInfo_t;




void LLAPI_init()
{
   LLAPI_Queue = xQueueCreate(32, sizeof(LLAPI_CallInfo_t));
}

void LLAPI_TimerCallBack(TimerHandle_t xTimer)
{
    LLAPI_TimerInfo_t *lltmr = pvTimerGetTimerID(xTimer);
    TaskHandle_t task = lltmr->forTask;
    vPortFree(lltmr->self);
    vTaskResume(task);
    xTimerDelete(xTimer, 0);
}

void LLAPI_Task()
{
    LLAPI_CallInfo_t currentCall;
    LLAPI_TimerInfo_t *lltmr;
    TimerHandle_t xTimer;
    for(;;)
    {
        while(xQueueReceive(LLAPI_Queue, &currentCall, portMAX_DELAY) == pdTRUE){
            vTaskSuspend(currentCall.task);

/*
            INFO("task:[%s] SWI NUM:%06x, r0:%08x, r1:%08x, r2:%08x, r3:%08x\n",
                pcTaskGetName(currentCall.task), currentCall.SWINum, currentCall.para0,
                currentCall.para1, currentCall.para2, currentCall.para3);
*/
            switch (currentCall.SWINum)
            {
            case LL_SWI_DELAY_MS:
                lltmr = pvPortMalloc(sizeof(LLAPI_TimerInfo_t));
                lltmr->forTask = currentCall.task;
                lltmr->self = lltmr;
                xTimer = xTimerCreate("SWI_DELAY_MS", pdMS_TO_TICKS(currentCall.para0), 0, lltmr, LLAPI_TimerCallBack);
                xTimerStart(xTimer, 0);
                break;
            
            case LL_SWI_WRITE_STRING:
                *currentCall.pRet = strlen((char *)currentCall.para0);
                printf("%s", (char *)currentCall.para0);
                vTaskResume(currentCall.task);
                break;
            
            case LL_SWI_PUT_CH:
                *currentCall.pRet = 1;
                printf("%c", currentCall.para0);
                vTaskResume(currentCall.task);
                break;
            


            default:
                break;
            }
        }
        
        {
            vTaskDelay(pdMS_TO_TICKS(100));
        }

    }
}







