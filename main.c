/*
 * FreeRTOS Kernel V10.3.1
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */
#include "keyboard.h"
#include "display.h"
#include "mmu.h"
#include "portmacro.h"
#include <malloc.h>
#include <stdio.h>

#include <math.h>

#include "BlockQ.h"
#include "GenQTest.h"

#include "clkgen.h"
#include "init.h"
#include "uart_debug.h"

#include "ServiceSwap.h"
#include "ServiceUSBDevice.h"
#include "cdc_console.h"

/* System serive includes. */
#include "ServiceDebug.h"
#include "ServiceManger.h"

/* Library includes. */
#include "elf_user.h"
#include "irq.h"
#include "memory_map.h"
#include "mmu.h"
#include "regsapbh.h"
#include "regsuartdbg.h"
#include "regspower.h"
#include "rtc.h"
#include "tusb.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

static void prvSetupHardware(void);

unsigned char pcWriteBuffer[2048];
extern volatile unsigned int dmaOperationCompleted;
extern volatile unsigned int eccOperationCompleted;

void printTaskList() {
    vTaskList((char *)&pcWriteBuffer);
    printf("=======================================================\r\n");
    printf("任务名                 任务状态   优先级   剩余栈   任务序号\n");
    printf("%s\n", pcWriteBuffer);
    printf("任务名                运行计数           CPU使用率\n");
    vTaskGetRunTimeStats((char *)&pcWriteBuffer);
    printf("%s", pcWriteBuffer);
    printf("任务状态:  X-运行  R-就绪  B-阻塞  S-挂起  D-删除\n");
    printf("内存剩余:   %d Bytes\n", (unsigned int)xPortGetFreeHeapSize());
    printf("Task mode: %x\n", get_mode());
    printf("dmaOperationCompleted %d eccOperationCompleted %d\n", dmaOperationCompleted, eccOperationCompleted);
}

void vTask1(void *pvParameters) {
    
    for (;;) {
        //static char c = '1';
        /*
        vTaskDelay(1000);
        printf("is_key_down 1:%08x\n",is_key_down(KEY_F1));
        printf("is_key_down 2:%08x\n",is_key_down(KEY_ON));*/

        //a++;
    
        //printf("c\n");
    }
}
 
void vTask2(void *pvParameters) {

    cdc_printf("Succeed!\r\n");

    for (;;) {

        //b++;
        vTaskDelay(1000);
        //cdc_printf("2\n");
        //printf("b\n");
    }
}

volatile int a = 0;


void vTask3(void *pvParameters) {
    unsigned int i = 0;
    for (;;) {
        //call_test();
        uartdbg_print_regs();
        //switch_mode(USER_MODE);
        vTaskDelay(1000);
        /*
		vTaskList((char *)&pcWriteBuffer);
		printf("=======================================================\r\n");
        printf("任务名                 任务状态   优先级   剩余栈   任务序号\n");
		printf("%s\n",pcWriteBuffer);
		printf("任务名                运行计数           CPU使用率\n");
		vTaskGetRunTimeStats((char *)&pcWriteBuffer);
        printf("%s", pcWriteBuffer);
		printf("任务状态:  X-运行  R-就绪  B-阻塞  S-挂起  D-删除\n");
		printf("内存剩余:   %d Bytes\n",(unsigned int)xPortGetFreeHeapSize());
		printf("Task mode: %x\n",get_mode());
		printf("dmaOperationCompleted %d eccOperationCompleted %d\n",dmaOperationCompleted,eccOperationCompleted);
		*/
    }
}
extern volatile uint32_t saved_svc_sp;
/* Create all the demo application tasks, then start the scheduler. */
int main(void) {
    /* Perform any hardware setup necessary. */
    
    prvSetupHardware();
    
    /* Create the tasks defined within this file. */
    //xTaskCreate( vTask1, "test task1", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
    //xTaskCreate(vTask2, "test task2", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    //xTaskCreate(vTask3, "Task Manager", configMINIMAL_STACK_SIZE, NULL, 4, NULL);

    xTaskCreate(vServiceManger, "Service Host", configMINIMAL_STACK_SIZE, NULL, 4, NULL);
    xTaskCreate(vInit, "init", configMINIMAL_STACK_SIZE * 2, NULL, 3, NULL);

    //vStartBlockingQueueTasks(1);
    //vStartGenericQueueTasks(1); 
    printf("pdMS_TO_TICKS(500)=%d\n", pdMS_TO_TICKS(500));
    
    uartdbg_print_regs();

    vTaskStartScheduler();

    printf("kernel start fail.\n");
    
    /* Execution will only reach here if there was insufficient heap to
	start the scheduler. */
    return 0;
}
/*-----------------------------------------------------------*/

static void prvSetupHardware(void) {

    
    BF_SETV(POWER_VDDDCTRL,TRG,26); // Set voltage = 1.45 V

    PLL_enable(1);
    
    HCLK_set_div(0, 4);   //120 MHz
    CPUCLK_set_div(0, 1); //480 MHz
    CPUCLK_set_gating(0);
    CPUCLK_set_bypass(0);

    
	BF_CS1(CLKCTRL_HBUS, SLOW_DIV, 1);	
	BF_CS1(CLKCTRL_HBUS, APBHDMA_AS_ENABLE, 1);
	BF_CS1(CLKCTRL_HBUS, APBXDMA_AS_ENABLE, 1);
	//BF_CS1(CLKCTRL_HBUS, CPU_DATA_AS_ENABLE, 1);
	//BF_CS1(CLKCTRL_HBUS, CPU_INSTR_AS_ENABLE, 1);
	BF_CS1(CLKCTRL_HBUS, TRAFFIC_JAM_AS_ENABLE, 1);
	BF_CS1(CLKCTRL_HBUS, TRAFFIC_AS_ENABLE, 1);
    BF_CS1(CLKCTRL_HBUS, AUTO_SLOW_MODE, 1);
   
    
    //BF_CS1(CLKCTRL_PIX, DIV, 8);	//修正LCD控制器频率

    //BF_CS1(CLKCTRL_CLKSEQ, BYPASS_PIX, 0);	//修正LCD控制器频率

    //BF_SETV(CLKCTRL_GPMI,DIV,4);

    BF_CS1(CLKCTRL_FRAC, CLKGATEIO, 0);
    BF_CLR(CLKCTRL_CLKSEQ, BYPASS_GPMI);

    BF_CS1(CLKCTRL_FRAC, CLKGATEPIX, 0);
    BF_CLR(CLKCTRL_CLKSEQ, BYPASS_PIX);

    BF_SET(CLKCTRL_PIX, CLKGATE);
    BF_CS1(CLKCTRL_PIX, DIV, 20); //480 / 20 = 24MHz

    BF_CLR(CLKCTRL_PIX, CLKGATE);
    BF_CS2(APBH_CTRL0, SFTRST, 0, CLKGATE, 0); //启动APBH DMA

    enable_interrupts(); //打开中断

    printf("(CLKCTRL_CPU,DIV_CPU), %08x\n", BF_RD(CLKCTRL_CPU, DIV_CPU));
    printf("(CLKCTRL_HBUS,DIV), %08x\n", BF_RD(CLKCTRL_HBUS, DIV));
    printf("VDDD voltage:%.2f V\n", 0.8 + (BF_RD(POWER_VDDDCTRL,TRG)*0.025));

    LCD_init();
    keyboard_init(); //键盘初始化
}
/*-----------------------------------------------------------*/
