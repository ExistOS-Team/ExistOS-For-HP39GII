

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>

#include "keyboard.h"
#include "llapi_code.h"
#include "sys_llapi.h"

#include "SysConf.h"

#include "FreeRTOS.h"
#include "task.h"

//#include "ff.h"


#include "debug.h"

#include "SystemFs.h"
#include "SystemUI.h"

#include "VROMLoader.h"

#include "Fatfs/ff.h"
//#include "mpy_port.h"

uint32_t AvailableMemorySize = BASIC_RAM_SIZE;

volatile unsigned long ulHighFrequencyTimerTicks;

char pcWriteBuffer[4096];
void printTaskList() {
    vTaskList((char *)&pcWriteBuffer);
    printf("=============SYSTEM STATUS=================\r\n");
    printf("Task Name         Task Status   Priority   Stack   ID\n");
    printf("%s\n", pcWriteBuffer);
    printf("Task Name                Running Count         CPU %%\n");
    vTaskGetRunTimeStats((char *)&pcWriteBuffer);
    printf("%s\n", pcWriteBuffer);
    printf("Status:  X-Running  R-Ready  B-Block  S-Suspend  D-Delete\n");
//    printf("Free memory:   %d Bytes\n", (unsigned int)xPortGetFreeHeapSize());
}

void vTask1(void *par1) {
    while (1) {
        printTaskList();
        vTaskDelay(pdMS_TO_TICKS(5678));
    }
}

void softDelayMs(uint32_t ms) {
    uint32_t cur = ll_get_time_ms();
    while ((ll_get_time_ms() - cur) < ms) {
        ;
    }
}


void delay_ms(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

void vTask2(void *par1) {
    uint32_t ticks = 0;
    while (1) {
        printf("SYS Run Time: %d s\n", ticks);
        ticks++;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void vApplicationIdleHook(void) {
    ll_system_idle();
}

void exp_exec(void *par);
static bool time_lable_refresh = true;

#define EMU_DATA_PORT ((volatile uint8_t *)0x20000000)
extern bool g_system_in_emulator;

void check_emulator_status() {

    if (g_system_in_emulator) {
        if (EMU_DATA_PORT[0]) {
            FIL savef;
            FRESULT fr;
            UINT br;
            char *fname = (char *)&EMU_DATA_PORT[10];
            uint32_t fsz = ((uint32_t *)(&EMU_DATA_PORT[4]))[0];
            printf("File send command detected.\n");
            printf("Receive file:%s\n", fname);
            printf("file size:%d\n", fsz);

            fname--;
            *fname = '/';
            fr = f_open(&savef, fname, FA_CREATE_ALWAYS | FA_WRITE);
            if (fr) {
                printf("Failed to create file:%s\n", fname);
            } else {
                f_write(&savef, (const void *)&EMU_DATA_PORT[200], fsz, &br);
                printf("File wrote to:%s, wsz:%d\n", fname, br);
                f_close(&savef);

                char *testname = fname + 1;
                while (*testname) {
                    if (
                        (testname[0] == '.') &&
                        (testname[1] == 'e') &&
                        (testname[2] == 'x') &&
                        (testname[3] == 'p') &&
                        (testname[4] == 0)) {
                        //xTaskCreate(exp_exec, fname + 1, configMINIMAL_STACK_SIZE, fname, configMAX_PRIORITIES - 3, NULL);
                    }
                    testname++;
                }
            }

            EMU_DATA_PORT[0] = 0;
        }
    }
}


static DIR dp;
static FILINFO finfo;


void khicasTask(void *_) {

    SystemUISuspend();

    void testcpp();
    testcpp();


    SystemUIResume();
    
    vTaskDelete(NULL);
}


void main_thread() {

    // printf("R13:%08x\n", get_stack());

    void SystemUIInit();
    SystemUIInit();
    SystemFSInit();

    for (;;) {

        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

void main() {

    // SYS STACK      0x023FA000
    // IRQ STACK      0x023FFFF0
    void IRQ_ISR();
    void SWI_ISR();
    ll_set_irq_stack(IRQ_STACK_ADDR);
    ll_set_irq_vector(((uint32_t)IRQ_ISR) + 4);
    ll_set_svc_stack(SWI_STACK_ADDR);
    ll_set_svc_vector(((uint32_t)SWI_ISR) + 4);
    ll_enable_irq(false);
    // ll_set_keyboard(true);
    

    // memset(&__HEAP_START[0], 0xFF, 384 * 1024);

    VROMLoader_Initialize();

    printf("System Booting...\n");

    
    //xTaskCreate(vTask1, "print task", 100, NULL, configMAX_PRIORITIES - 3, NULL);



    uint32_t free, total, total_comp;
    //AvailableMemorySize
    total_comp = ll_mem_phy_info(&free, &total);
    if(total_comp > AvailableMemorySize)
    {
        AvailableMemorySize = total_comp;
    }

    xTaskCreate(main_thread, "System", 2048, NULL, configMAX_PRIORITIES - 3, NULL);



    vTaskStartScheduler();

    for (;;) {
        *((double *)0x45678901) = 114514.1919810f;
        void symtab_def();
        symtab_def();
    }
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    PANIC("StackOverflowHook:%s\n", pcTaskName);
}

void vAssertCalled(char *file, int line) {
    PANIC("ASSERTION FAILED AT %s:%d\n", file, line);
}

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize) {
    *ppxTimerTaskTCBBuffer = (StaticTask_t *)pvPortMalloc(sizeof(StaticTask_t));
    *ppxTimerTaskStackBuffer = (StackType_t *)pvPortMalloc(configMINIMAL_STACK_SIZE * 4);
    *pulTimerTaskStackSize = configMINIMAL_STACK_SIZE;
}

void vApplicationMallocFailedHook() {
    PANIC("ASSERT: Out of Memory.\n");
}
