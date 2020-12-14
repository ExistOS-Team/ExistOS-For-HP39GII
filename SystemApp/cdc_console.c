#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "init.h"

#include "ServiceUSBDevice.h"

#include "FreeRTOS_CLI.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

uint8_t buf[64];
uint8_t console_output_buffer[1024];

BaseType_t cdc_tasklist_cb(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString){
	
	vTaskList(pcWriteBuffer);
	cdc_printf("=======================================================\r\n");
    cdc_printf("任务名                 任务状态   优先级   剩余栈   任务序号\n");
	cdc_printf("%s\n",pcWriteBuffer);
	cdc_printf("任务名                运行计数           CPU使用率\n");
	vTaskGetRunTimeStats(pcWriteBuffer);
    cdc_printf("%s", pcWriteBuffer);
	cdc_printf("任务状态:  X-运行  R-就绪  B-阻塞  S-挂起  D-删除\n");
	cdc_printf("内存剩余:   %d Bytes\n",(unsigned int)xPortGetFreeHeapSize());
	cdc_printf("Task mode: %x\n",get_mode());
	*pcWriteBuffer = 0;
	return pdFALSE;
}



static const CLI_Command_Definition_t cmd_tasklist = {
	"tasklist",
	"\r\ntasklist - Displays a table showing the state of each task.\r\n",
	cdc_tasklist_cb,
	0
};

void vCDC_Console(){
	
	vTaskDelay(500);
	
	FreeRTOS_CLIRegisterCommand(&cmd_tasklist);
	
	
	for(;;){
		if(xQueueReceive( CDCCmdLineQueue, (&buf), ( TickType_t ) portMAX_DELAY ) == pdTRUE ){
			FreeRTOS_CLIProcessCommand(buf,console_output_buffer,sizeof(console_output_buffer));
			cdc_printf("%s",console_output_buffer);
			memset(buf,0,sizeof(buf));
		}
	}
}