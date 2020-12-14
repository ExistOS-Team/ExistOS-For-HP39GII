#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "init.h"

#include "ServiceUSBDevice.h"
#include "ServiceRawFlash.h"

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

BaseType_t dreadpage_cb(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString){
	const char *pcParameter;
	BaseType_t xParameterStringLength, xStatus;
	uint8_t cNum[16];
	uint32_t paraNum = 0;
	
	pcParameter = FreeRTOS_CLIGetParameter
						(
							pcCommandString,
							1,		
							&xParameterStringLength	
						);
	memcpy(cNum,pcParameter,xParameterStringLength);
	paraNum = atoi(cNum);
	xStatus = xReadFlashPages( paraNum, 1,(unsigned int *)(&__DMA_NAND_PALLOAD_BUFFER), 1 );
	if(xStatus == NO_ERROR){
		
		cdc_printf("Data Area (2 KBytes):");
		for(int i = 0; i < 2048; i++){
			if(i % 16 == 0){
				cdc_printf("\n%08X: ", i);
			}
			if(i % 8 == 0){
				cdc_printf(" ");
			}
			
			cdc_printf("%02X ",*((unsigned char *)(&__DMA_NAND_PALLOAD_BUFFER + i)));
		}
		cdc_printf("\n");
		
		cdc_printf("Spare Area 19 Bytes:\n");
		for(int i = 0; i < 19; i++){
			if(i % 16 == 0){
				cdc_printf("\n%08X: ", i);
			}
			if(i % 8 == 0){
				cdc_printf(" ");
			}
			
			cdc_printf("%02X ",*((unsigned char *)(&__DMA_NAND_AUXILIARY_BUFFER + i)));
		}
		cdc_printf("\n");
		
		
	}else{
		cdc_printf("Read ERROR.\n");
	}
	*pcWriteBuffer = 0;
	return pdFALSE;
}

BaseType_t drbtest_cb(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString){
	const char *pcParameter;
	BaseType_t xParameterStringLength, xStatus;
	uint8_t cNum[16];
	uint32_t paraNum = 0;
	
	pcParameter = FreeRTOS_CLIGetParameter
						(
							pcCommandString,
							1,		
							&xParameterStringLength	
						);
	memcpy(cNum,pcParameter,xParameterStringLength);
	paraNum = atoi(cNum);
	
	unsigned int testSizeKB = paraNum;
	unsigned int *buffer = pvPortMalloc(testSizeKB*1024);
	unsigned int currentTick = xTaskGetTickCount();
	xStatus = xReadFlashPages(0,testSizeKB/2,buffer,2000);
	if(xStatus == NO_ERROR){
		cdc_printf("%d KB read, time %d ms, speed %.1f MB/s \n",testSizeKB, (xTaskGetTickCount() - currentTick),
															(((double)testSizeKB/(xTaskGetTickCount() - currentTick))*1000.0)/1024.0 );
	}else if (xStatus == READ_TIMEOUT){
		cdc_printf("READ_TIMEOUT. time %d ms.\n",  (xTaskGetTickCount() - currentTick));
	}else if (xStatus == DEVICE_BUSY){
		cdc_printf("DEVICE_BUSY. time %d ms.\n",  (xTaskGetTickCount() - currentTick));
	}
	vPortFree(buffer);
	
	*pcWriteBuffer = 0;
	return pdFALSE;
}

BaseType_t chkdsk_cb(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString){
	BaseType_t xStatus;
	unsigned int badblocks = 0;
	unsigned int p = 0,i;
	unsigned int tick = xTaskGetTickCount();
	cdc_printf("Scanning bad block ... \n");
	for(i = 0; i < 65535; i++){
		xStatus = xReadFlashPages( i, 1,(unsigned int *)(&__DMA_NAND_PALLOAD_BUFFER), 100 );
		if( *((unsigned char *)(&__DMA_NAND_AUXILIARY_BUFFER)) != 0xFF ){
			cdc_printf("Found 1 bad block at page %d \n", i);
			badblocks++;
		}
		if( i % 6553 == 0){
			cdc_printf("%d %%, Speed %.2f MB/s ...\n",p*10, (xTaskGetTickCount() - tick)/6553.0 * 2 );
			p++;
			tick = xTaskGetTickCount();
		}
	}

	cdc_printf("Total page: %d, total bad page: %d\n", i+1, badblocks);
	
	return pdFALSE;
}

static const CLI_Command_Definition_t cmd_chkdsk = {
	"chkdsk",
	"\r\nchkdsk - Flash bad block scan.\r\n ",
	chkdsk_cb,
	0
};

static const CLI_Command_Definition_t cmd_drbtest = {
	"drbtest",
	"\r\ndrbtest - Flash read back speed test.\r\n    <param1> test size in KByte(s).",
	drbtest_cb,
	1
};

static const CLI_Command_Definition_t cmd_tasklist = {
	"tasklist",
	"\r\ntasklist - Displays a table showing the state of each task.\r\n",
	cdc_tasklist_cb,
	0
};

static const CLI_Command_Definition_t cmd_dreadpage = {
	"dreadpage",
	"\r\ndreadpage - Read a page of flash.\r\n    <param1> which page.\r\n",
	dreadpage_cb,
	1
};


void vCDC_Console(){
	
	vTaskDelay(500);
	
	FreeRTOS_CLIRegisterCommand(&cmd_tasklist);
	FreeRTOS_CLIRegisterCommand(&cmd_dreadpage);
	FreeRTOS_CLIRegisterCommand(&cmd_drbtest);
	FreeRTOS_CLIRegisterCommand(&cmd_chkdsk);
	
	
	for(;;){
		if(xQueueReceive( CDCCmdLineQueue, (&buf), ( TickType_t ) portMAX_DELAY ) == pdTRUE ){
			FreeRTOS_CLIProcessCommand(buf,console_output_buffer,sizeof(console_output_buffer));
			cdc_printf("%s",console_output_buffer);
			memset(buf,0,sizeof(buf));
		}
	}
}