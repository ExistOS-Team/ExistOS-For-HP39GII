
#include <string.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"

#include "board_up.h"
#include "display_up.h"

#include "llapi.h"
#include "llapi_code.h"

#include "../debug.h"

#include "mmu.h"
#include "vmMgr.h"

#include "tusb.h"


#define TASK_STATUS_BLOCK       0
#define TASK_STATUS_SUSPEND     1
#define TASK_STATUS_RUN         2

typedef struct LLAPI_TimerInfo_t
{
    TaskHandle_t forTask;
    struct LLAPI_TimerInfo_t *self;
}LLAPI_TimerInfo_t;


typedef struct LL_TaskInfo_t
{
    struct LL_TaskInfo_t *next;
    uint32_t *taskContext;
    int64_t blockTimeUs;
    uint8_t status;


}LL_TaskInfo_t;

LL_TaskInfo_t *LL_Task = NULL;
LL_TaskInfo_t *CurRunningTask;

int totalTask = 1;

bool LL_SchedulerInCritical = false;

 uint32_t IRQSavedContext[16];
static uint32_t SWISavedContext[16];
static uint32_t SPSR_IRQ, SPSR_SWI;

static uint32_t LL_IRQVector;
static uint32_t LL_IRQStack;
volatile uint32_t LL_ConfigAddr = 0;


volatile uint32_t LL_SYSTimerPeriod;
volatile bool LL_TimerEnable = false;


volatile bool LL_EnableIRQ = false;
volatile uint32_t LLInException = 0;


volatile uint32_t upSysInFault = 0;


TaskHandle_t hLLAPIBlockTask;
SemaphoreHandle_t LL_upSysContextMutex;


uint32_t INSWI = 0;
uint32_t savedBy = 0;
TaskHandle_t upSystem;

extern volatile uint32_t swapping;



#define upSYSContext    ((uint32_t *)(((((uint32_t *)upSystem)[1])) - 16 * 4))

bool isExceptionRet()
{
    uint32_t ret = ((uint32_t *)LL_ConfigAddr)[20];
    //INFO("ret:%d\n", ret);
    return (ret == 0);
}

void LL_TrapInException(uint32_t exceptionType, uint32_t IRQNum, uint32_t par1, uint32_t par2, uint32_t par3)
{

    
    ((uint32_t *)LL_ConfigAddr)[20] = 1;

    memcpy((uint32_t *)LL_ConfigAddr, upSYSContext, sizeof(IRQSavedContext));
    ((uint32_t *)LL_ConfigAddr)[15] -= 4;

    for(int i = 0; i<16;i++){
        LLAPI_INFO("save[%d]:%08x\n",i, ((uint32_t *)LL_ConfigAddr)[i]);
    }

    upSYSContext[0] = IRQNum;

    upSYSContext[13] = LL_IRQStack;
    upSYSContext[15] = LL_IRQVector + 0; 



}



void LL_SysTimerCallBack(TimerHandle_t timerx)
{
    //INFO("SysTick A\n");
    if((isExceptionRet()) && (LL_EnableIRQ) && (LL_TimerEnable) && (   (swapping == 0)  ) /* && ( INSWI == 0)*/)
    {
        
        LL_TrapInException(EXCEPTION_IRQ, LL_IRQ_TIMER, 0, 0, 0);
        INFO("SysTick\n");
    }
    
}




void vLLAPI_TaskPostKey(void *p)
{
    LLAPI_KBD_t key;

    for(;;)
    {
        while (xQueueReceive(LLAPI_KBDQueue, &key, portMAX_DELAY) == pdTRUE){
            if(LL_ConfigAddr != 0){
                while(((char *)LL_ConfigAddr)[100] != 0xFE)
                {
                    vTaskDelay(pdMS_TO_TICKS(40));
                }
                ((char *)LL_ConfigAddr)[100] = key.key;
                ((char *)LL_ConfigAddr)[101] = key.press;
            }
        }
    }
}

TimerHandle_t sysTickTimer;


void LLAPI_init(TaskHandle_t upSys)
{
    upSystem = upSys;

    LL_IRQVector = 0xF0F00000;
    LL_IRQStack  = 0xF0FF0000;

    LLAPI_Queue = xQueueCreate(32, sizeof(LLAPI_CallInfo_t));
    LLAPI_KBDQueue = xQueueCreate(32, sizeof(LLAPI_KBD_t));

    LL_SYSTimerPeriod = portMAX_DELAY;
   
    xTaskCreate(vLLAPI_TaskPostKey, "LLAPI Post KBD", configMINIMAL_STACK_SIZE, NULL, 4, NULL);

    //vTaskSuspend(hLLAPIBlockTask);
    sysTickTimer = xTimerCreate("systick", LL_SYSTimerPeriod, pdTRUE, NULL, LL_SysTimerCallBack);
}

LLAPI_CallInfo_t currentCall;

volatile bool nextTrapInSwi = false;

void TrapHasIn()
{
    vTaskSuspend(hLLAPIBlockTask);

}

void LL_IDLETask()
{
    
    for(;;)
    {
        taskYIELD();
    }
}

char *str_e[] = {"SWI", "IRQ", "PAB", "DAB", "UND"};

void LL_Scheduler_(uint32_t exception, uint32_t *SYSContext)
{
    static bool reent = false;
    static int lastExp = 0;
    static uint32_t tickTimeUs = 0;

    if(reent){
        INFO("NO REENT! excep:%s, lastExp:%s, [%s]\n", str_e[exception], str_e[lastExp], pcTaskGetName(NULL));
        while(1);
    }
    reent = true;

    lastExp = exception;

    if(LL_Task == NULL){
        LL_Task = pvPortMalloc(sizeof(LL_TaskInfo_t));

        configASSERT(LL_Task);
        LL_Task->taskContext = pvPortMalloc(sizeof(uint32_t) * 16);
        configASSERT(LL_Task->taskContext);
        LL_Task->taskContext[13] = (uint32_t) pvPortMalloc(4 * 200);
        configASSERT(LL_Task->taskContext[13]);
        LL_Task->taskContext[15] = (uint32_t) LL_IDLETask + 4;
        LL_Task->blockTimeUs = 0;
        LL_Task->status = TASK_STATUS_RUN;

        LL_Task->next = pvPortMalloc(sizeof(LL_TaskInfo_t));
        configASSERT(LL_Task);

        LL_Task->next->taskContext = pvPortMalloc(sizeof(uint32_t) * 16);
        LL_Task->next->blockTimeUs = 0;
        LL_Task->next->status = TASK_STATUS_RUN;
        LL_Task->next->next = NULL;
        CurRunningTask = LL_Task->next;
    }

    configASSERT(CurRunningTask);

    memcpy(CurRunningTask->taskContext, SYSContext, sizeof(uint32_t) * 16);

    switch (exception)
    {

        case L_SWI:
            {
                uint32_t SWINum = *((uint32_t *)(CurRunningTask->taskContext[15] - 8)) & 0x00FFFFFF;
                //INFO("SWI NUM:%x\n",SWINum);
                if((SWINum >= LL_SWI_BASE) && (SWINum < LL_SWI_BASE + LL_SWI_NUM))
                {

                    switch (SWINum)
                    {
                        case LL_SWI_ENTER_CRITICAL:
                            LL_SchedulerInCritical = true;
                        break;

                        case LL_SWI_EXIT_CRITICAL:
                            LL_SchedulerInCritical = false;
                        break;

                        case LL_SWI_TASK_SLEEP_US:
                            
                            {
                                uint32_t *p = (uint32_t *)&(CurRunningTask->blockTimeUs);
                                p[0] = CurRunningTask->taskContext[0];
                                p[1] = CurRunningTask->taskContext[1];
                                CurRunningTask->status = TASK_STATUS_BLOCK;
                            }                           
                            
                        break;
                    
                    default:
                        {
                            LLAPI_CallInfo_t LLSWI;
                            BaseType_t SwitchContext;
                            LLSWI.para0 = CurRunningTask->taskContext[0];
                            LLSWI.para1 = CurRunningTask->taskContext[1];
                            LLSWI.para2 = CurRunningTask->taskContext[2];
                            LLSWI.para3 = CurRunningTask->taskContext[3];
                            LLSWI.pRet = (uint32_t *)&CurRunningTask->taskContext[0]; //R0
                            LLSWI.task = xTaskGetCurrentTaskHandle();
                            LLSWI.SWINum = SWINum;
                            if( xQueueSendFromISR(LLAPI_Queue, &LLSWI, &SwitchContext) == pdFALSE)
                            {
                                printf("ERROR QUEUE FULL!\n");
                            }
                        }
                        break;
                    }

                }


            }
        break;

        
    
    default:
        break;
    }
    
    if(tickTimeUs){
        uint32_t passTimeUs = (portBoardGetTime_us()) - tickTimeUs;
        LL_TaskInfo_t *chain;
        chain = LL_Task->next;
        while(chain){

            if(
                (chain->status != TASK_STATUS_SUSPEND) && (chain->blockTimeUs > 0LL)
            )
                {
                chain->blockTimeUs -= passTimeUs;
                if(chain->blockTimeUs <= 0LL){
                    chain->blockTimeUs = 0LL;
                    chain->status = TASK_STATUS_RUN;
                    //INFO("TIMEOUT\n");
                }
            }
            chain = chain -> next;
        }
    }
    tickTimeUs = portBoardGetTime_us();


    if((LL_SchedulerInCritical == false) && (

        (exception == L_IRQ) || (exception == L_DAB)

        ))
    {

search:

        while (CurRunningTask->next)
        {
            CurRunningTask = CurRunningTask->next;
            if(CurRunningTask->status == TASK_STATUS_RUN){
                goto found;
            }
        }
        
        CurRunningTask = LL_Task;

        while (CurRunningTask->next)
        {
            CurRunningTask = CurRunningTask->next;
            if(CurRunningTask->status == TASK_STATUS_RUN){
                goto found;
            }
        }

        CurRunningTask = LL_Task;

    }
    
found:

        if(CurRunningTask ->status != TASK_STATUS_RUN){
            goto search;
        }

    configASSERT(CurRunningTask->status == TASK_STATUS_RUN);
    configASSERT(CurRunningTask);
    configASSERT(CurRunningTask->taskContext);


    memcpy(SYSContext, CurRunningTask->taskContext, sizeof(uint32_t) * 16);


    //INFO("E:%d, con:%08x\n",exception, SYSContext);

    reent = false;
}

void LLAPI_Task()
{

    for(;;)
    {
        while(xQueueReceive(LLAPI_Queue, &currentCall, portMAX_DELAY) == pdTRUE){
            INSWI = 1;
            vTaskSuspend(upSystem);
            switch (currentCall.SWINum)
            {
            case LL_SWI_SET_IRQ_STACK:
                LL_IRQStack = currentCall.para0;
                
                INFO("SET LL_IRQStack:%08x\n",LL_IRQStack);
                vTaskResume(upSystem);
                break;

            case LL_SWI_SET_IRQ_VECTOR:
                LL_IRQVector = currentCall.para0;

                INFO("SET LL_IRQVector:%08x\n",LL_IRQVector);
                vTaskResume(upSystem);
                break;

            case LL_SWI_ENABLE_IRQ:
                LL_EnableIRQ = currentCall.para0;
                vTaskResume(upSystem);
                break;

            case LL_SWI_LOAD_CONTEXT:
                memcpy(upSYSContext, (uint32_t *)currentCall.para0, sizeof(IRQSavedContext));
                LLAPI_INFO("LOAD\n");
                for(int i =0;i<16;i++){
                    LLAPI_INFO("LOAD[%d]:%08x\n",i, upSYSContext[i]);
                }
                vTaskResume(upSystem);
                break;
            
            case LL_SWI_SET_CONFIG_ADDR:
            {
                int ret;
                LL_ConfigAddr = currentCall.para0;
                
                ret = vmMgr_getMountPhyAddressAndLock(LL_ConfigAddr, PERM_R | PERM_W);
                INFO("LL_SWI_SET_CONFIG_ADDR:%08x, ret:%08x\n", LL_ConfigAddr, ret);
                vTaskResume(upSystem);
            }
                break;

                
            case LL_SWI_GET_TIME_US:
                *currentCall.pRet = portBoardGetTime_us();
                vTaskResume(upSystem);
                break;

            case LL_SWI_ENABLE_TIMER:
                LL_TimerEnable = currentCall.para0;
                LL_SYSTimerPeriod = pdMS_TO_TICKS(currentCall.para1);
                //LL_SYSTimerPeriod = pdMS_TO_TICKS(500);

                if(LL_TimerEnable){
                    xTimerChangePeriod(sysTickTimer, LL_SYSTimerPeriod, 0);
                    xTimerStart(sysTickTimer, 0);
                }else{
                    xTimerStop(sysTickTimer, 0);
                }
                
                vTaskResume(upSystem);
                break;

            case LL_SWI_YEILD:
                //INFO("POST YEILD\n");
                //if(isExceptionRet())
                //   nextTrapInSwi = true;

                LL_TrapInException(EXCEPTION_SWI, LL_IRQ_YEILD, 0, 0, 0);

                vTaskResume(upSystem);
                break;

            case LL_SWI_WRITE_STRING2:
                {
                    if(currentCall.para1 < 512)
                    for(int i = 0 ; i< currentCall.para1; i++){
                        putchar(((char *)currentCall.para0)[i]);
                    }
                }
                vTaskResume(upSystem);
                break;
            
            case LL_SWI_DISPLAY_FLUSH:
                {
                    DispFlushInfo_t *info;
                    uint32_t vramPhyAddr;
                    uint32_t StartVramPhyAddr;
                    long bufSize;
                    int needPage;
                    info = (DispFlushInfo_t *)currentCall.para0;

                    //INFO("draw:%d %d %d %d, addr:%08x\n",info->x0, info->y0, info->x1, info->y1, info->vram);
                    bufSize = (info->y1 - info->y0) * (info->x1 - info->x0);
                    if((bufSize <= 0) || (bufSize > 32*1024)){
                        vTaskResume(upSystem);
                        break;
                    }
                    needPage = (bufSize / PAGE_SIZE) + 1;

                    StartVramPhyAddr = vmMgr_getMountPhyAddressAndLock((uint32_t)info->vram, PERM_R);

                    for(int i = 1; i < needPage; i++)
                    {
                        vramPhyAddr = vmMgr_getMountPhyAddressAndLock((uint32_t)info->vram + (PAGE_SIZE * i), PERM_R);
                        if(vramPhyAddr > 2){
                            mmu_clean_invalidated_dcache((uint32_t)info->vram + (PAGE_SIZE * i), PAGE_SIZE);
                        }
                    }

                    DisplayFlushArea(info->x0, info->y0, info->x1, info->y1, (uint8_t *)StartVramPhyAddr);

                    for(int i = 0; i < needPage; i++)
                    {
                        vmMgr_unlockMap(((uint32_t)info->vram) + (i * PAGE_SIZE));
                    }

                }
                vTaskResume(upSystem);
                break;

            case LL_SWI_DISPLAY_CLEAR:
                {
                    DisplayClean();
                }
                vTaskResume(upSystem);
                break;

            case LL_SWI_DISPLAY_PUTSTR:
                {
                    DispPutStrInfo_t *info = (DispPutStrInfo_t *)currentCall.para0;
                    DisplayPutStr(info->x0, info->y0, info->string, info->fg, info->bg, info->fontsize);
                }
                vTaskResume(upSystem);
                break;

            case LL_SWI_DISPLAY_PUT_BOX:
                {
                    DispPutBoxInfo_t *info = (DispPutBoxInfo_t *)currentCall.para0;
                    if(info->fill){
                        DisplayFillBox(info->x0, info->y0, info->x1, info->y1, info->color);
                    }else{
                        DisplayBox(info->x0, info->y0, info->x1, info->y1, info->color);
                    }
                }
                vTaskResume(upSystem);
                break;

            case LL_SWI_DISPLAY_HLINE:
                {
                    DisplayHLine(currentCall.para1, currentCall.para2, currentCall.para0, currentCall.para3);
                }
                vTaskResume(upSystem);
                break;

            case LL_SWI_DISPLAY_VLINE:
                {
                    DisplayVLine(currentCall.para1, currentCall.para2, currentCall.para0, currentCall.para3);
                }
                vTaskResume(upSystem);
                break;

            case LL_SWI_DISPLAY_SETINDICATE:
                {
                    DisplaySetIndicate(currentCall.para0, currentCall.para1);
                }
                vTaskResume(upSystem);
                break;


            case LL_SWI_DISPLAY_SEND_SCREEN:
                    { 
                       uint8_t *buf = pvPortMalloc(1024);
                       bool fin = false;
                       if(buf == NULL){
                         break;
                       }
                       if(tud_cdc_available()){
                            for(int i = 0; i < 128; i+=4){
                                DisplayReadArea(0, i, 255, (i + 3), buf, &fin);
                                while(!fin);


                                for(int i = 0; i < 1024; i++){
                                  tud_cdc_n_write_char(0, ((char *)buf)[i]);    
                                }
                                tud_cdc_write_flush();
                            }

                       }
                       vPortFree(buf);
                    }
                    vTaskResume(upSystem);
                break;



            case LL_SWI_TASK_CREATE:
                {
                    LL_TaskInfo_t *chain = LL_Task;
                    configASSERT(chain);

                    uint32_t *con = pvPortMalloc(16 * sizeof(uint32_t));
                    if(con == NULL){
                        *currentCall.pRet = 0;
                        vTaskResume(upSystem);
                        break;
                    }
                    //memset(con, 0xEF, 16 * sizeof(uint32_t));
                    uint32_t t = 0x01010101;
                    for(int i = 0; i < 16; i++)
                    {
                        con[i] = t;
                        t+=0x01010101;
                    }

                    while (chain->next)
                    {
                        chain = chain->next;
                    }

                    LL_TaskInfo_t *newTask = pvPortMalloc(sizeof(LL_TaskInfo_t));
                    if(newTask == NULL){
                        *currentCall.pRet = 0;
                        vTaskResume(upSystem);
                        break;
                    }

                    newTask->taskContext = con;
                    newTask->next = NULL;
                    newTask->status = TASK_STATUS_RUN;
                    newTask->blockTimeUs = 0;

                    con[13] = currentCall.para0;

                    con[15] = currentCall.para1 + 4;


                    INFO("Creat Task:%08x, %08x\n", con[13], con[15]);
                    
                    *currentCall.pRet = (uint32_t)newTask;

                    chain->next = newTask;
                    vTaskResume(upSystem);
                    break;
                }

            default:
                break;
            }

            



            

            INSWI = 0;
        }
        
    }
}







