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

#include <stdio.h>
#include <malloc.h>
#include "portmacro.h"
#include "mmu.h"

#include "GenQTest.h"
#include "BlockQ.h"

#include "init.h"
#include "uart_debug.h"

/* System serive includes. */
#include "ServiceManger.h"
#include "ServiceDebug.h"

/* Library includes. */
#include "regsuartdbg.h"
#include "regsapbh.h"
#include "irq.h"
#include "tusb.h"
#include "rtc.h"
/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

static void prvSetupHardware(void);

volatile int a = 1, b = 1, c = 1;

unsigned char pcWriteBuffer[2048];
extern volatile unsigned int dmaOperationCompleted;
extern volatile unsigned int eccOperationCompleted;

void printTaskList(){
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
	
}

void vTask1( void *pvParameters ){
	
	for(;;){
		//static char c = '1';
		//vTaskDelay(1);
		//printf("1");
		//a++;
		vTaskDelay(1000);
		//printf("c\n");
	}
	
}

void vTask2( void *pvParameters ){
	
	for(;;){
		
		//b++;
		vTaskDelay(1000);
		//printf("b\n");
	}
	
}




void vTask3( void *pvParameters ){
	
	
	for(;;){
		//uartdbg_print_regs();
		//switch_mode(USER_MODE);
		vTaskDelay(5000);/*
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




/* Create all the demo application tasks, then start the scheduler. */
int main( void )
{
	/* Perform any hardware setup necessary. */
  	prvSetupHardware();

	/* Create the tasks defined within this file. */
	//xTaskCreate( vTask1, "test task1", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
	//xTaskCreate( vTask2, "test task2", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
	
	xTaskCreate( vTask3, "Task Manager", configMINIMAL_STACK_SIZE, NULL, 4, NULL );
	xTaskCreate( vServiceManger, "Service Host", configMINIMAL_STACK_SIZE, NULL, 4, NULL );
	xTaskCreate( vInit, "init", configMINIMAL_STACK_SIZE*4, NULL, 3, NULL );
	
	//vStartBlockingQueueTasks(1);
	//vStartGenericQueueTasks(1);
	
	printf("pdMS_TO_TICKS(500)=%d\n",pdMS_TO_TICKS(500));
	
	vTaskStartScheduler();
	/* Execution will only reach here if there was insufficient heap to
	start the scheduler. */
	return 0;
}
/*-----------------------------------------------------------*/



static void prvSetupHardware(void)
{
	BF_CS2(APBH_CTRL0, SFTRST, 0, CLKGATE, 0);			//启动APHB桥的DMA
	
	
	
	
	enable_interrupts();					//打开中断
}
/*-----------------------------------------------------------*/

