#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "init.h"

#include "ServiceUSBDevice.h"
#include "ServiceRawFlash.h"
#include "ServiceSTMPPartition.h"

#include "raw_flash.h"

#include "FreeRTOS_CLI.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
extern unsigned char ecc_res[4];

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

BaseType_t erase_all_cb(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString){
	const char *pcParameter;
	BaseType_t xParameterStringLength, xStatus;
	uint8_t cNum[16];
	uint32_t paraNum = 0;

	
	for(int i=0; i<1024; i++){
		xStatus = xEraseFlashBlocks(i, 1, 5000);
		vTaskDelay(1);
		if(xStatus == TIMEOUT){
			cdc_printf("Timeout at %d block.\n",  i);
		}else if(xStatus == OPERATION_FAIL){
			cdc_printf("OPERATION_FAIL at %d block.\n",  i);
		}
		if(i % 100 == 0){
			cdc_printf("Erasing %d/1024 ...\n",i);
		}
		
	}
	cdc_printf("Erase finish. %d ...\n",paraNum);
	
	
	return pdFALSE;
	
}

BaseType_t erase_data_cb(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString){
	const char *pcParameter;
	BaseType_t xParameterStringLength, xStatus;
	uint8_t cNum[16];
	uint32_t paraNum = 0;

	
	for(int i=getDataRegonStartBlock(); i<getDataRegonStartBlock() + getDataRegonTotalBlocks(); i++){
		
		xStatus = xEraseFlashBlocks(i, 1, 5000);
		vTaskDelay(1);
		
		if(xStatus == TIMEOUT){
			cdc_printf("Timeout at %d block.\n",  i);
		}else if(xStatus == OPERATION_FAIL){
			cdc_printf("OPERATION_FAIL at %d block.\n",  i);
		}
		if(i % (getDataRegonTotalBlocks() / 10 ) == 0){
			cdc_printf("Erasing %d/%d ...\n",i,getDataRegonTotalBlocks());
		}
		
	}
	cdc_printf("Erase finish. %d ...\n",paraNum);
	
	
	return pdFALSE;
	
}


BaseType_t deraseb_cb(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString){
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

	cdc_printf("Erasing %d ...\n",paraNum);
	unsigned int currentTick = xTaskGetTickCount();
	xStatus = xEraseFlashBlocks(paraNum, 1, 5000);
	if(xStatus == NO_ERROR){
		cdc_printf("Finish. time %d ms.\n",  (xTaskGetTickCount() - currentTick));
	}else if(xStatus == TIMEOUT){
		cdc_printf("TIMEOUT. time %d ms.\n",  (xTaskGetTickCount() - currentTick));
	}else if(xStatus == OPERATION_FAIL){
		cdc_printf("OPERATION_FAIL. time %d ms.\n",  (xTaskGetTickCount() - currentTick));
	}
	
 
	cdc_printf("Erase finish. %d ...\n",paraNum);
	
	
	return pdFALSE;
	
}
 
BaseType_t dwrallonec_cb(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString){
	const char *pcParameter;
	BaseType_t xParameterStringLength, xStatus;
	uint8_t cNum[16];
	uint32_t paraNum = 0;
	unsigned char *buffer, *buffer_meta;
	
	pcParameter = FreeRTOS_CLIGetParameter
						(
							pcCommandString,
							1,		
							&xParameterStringLength	
						);
	memcpy(cNum,pcParameter,xParameterStringLength);
	paraNum = atoi(cNum);
	buffer = pvPortMalloc(2048);
	buffer_meta = pvPortMalloc(19);
	
	cdc_printf("Writing %d ...\n",paraNum);
	
	memset(buffer , 0x5A , 2048);	
	memset(buffer_meta , 0xFF , 19);	
	unsigned int currentTick = xTaskGetTickCount();
	xStatus = xWriteFlashPages( paraNum, 1, buffer, buffer_meta, 5000 );
	if(xStatus == NO_ERROR){
		cdc_printf("Finish. time %d ms.\n",  (xTaskGetTickCount() - currentTick));
	}else if(xStatus == TIMEOUT){
		cdc_printf("TIMEOUT. time %d ms.\n",  (xTaskGetTickCount() - currentTick));
	}else if(xStatus == OPERATION_FAIL){
		cdc_printf("OPERATION_FAIL. time %d ms.\n",  (xTaskGetTickCount() - currentTick));
	}
	
	vPortFree(buffer);
	vPortFree(buffer_meta);
	cdc_printf("Write finish. %d ...\n",paraNum);
	
	
	return pdFALSE;
	
}

BaseType_t dreadpage_cb(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString){
	const char *pcParameter;
	BaseType_t xParameterStringLength, xStatus;
	uint8_t cNum[16];
	uint32_t paraNum = 0;
	unsigned char *buffer;
	
	pcParameter = FreeRTOS_CLIGetParameter
						(
							pcCommandString,
							1,		
							&xParameterStringLength	
						);
	memcpy(cNum,pcParameter,xParameterStringLength);
	paraNum = atoi(cNum);
	buffer = pvPortMalloc(2048);
	memset(buffer , 0 , 2048);
	xStatus = xReadFlashPages( paraNum, 1,buffer, 1000 );
	if(xStatus == NO_ERROR){
		
		cdc_printf("Data Area (2 KBytes):");
		for(unsigned int i = 0; i < 2048; i++){
			if(i % 16 == 0){
				cdc_printf("\n%08X: ", i);
			}
			if(i % 8 == 0){
				cdc_printf(" ");
			}
			
			cdc_printf("%02X ",buffer[i]);
		}
		cdc_printf("\n");
		
		cdc_printf("Spare Area 19 Bytes:");
		for(unsigned int i = 0; i < 19; i++){
			if(i % 16 == 0){
				cdc_printf("\n%08X: ", i);
			}
			if(i % 8 == 0){
				cdc_printf(" ");
			}
			
			cdc_printf("%02X ",__DMA_NAND_AUXILIARY_BUFFER[i]);
		}
		cdc_printf("\n");
		cdc_printf("\nECC status: %02X %02X %02X %02X\n",ecc_res[0], ecc_res[1], ecc_res[2], ecc_res[3]);
		
		
	}else{
		cdc_printf("\nRead ERROR.\n");
	}
	vPortFree(buffer);
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
	}else if (xStatus == TIMEOUT){
		cdc_printf("TIMEOUT. time %d ms.\n",  (xTaskGetTickCount() - currentTick));
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
	unsigned int unstable = 0;
	unsigned int p = 0,i;
	unsigned int tick = 0;
	unsigned char *buffer;
	buffer = pvPortMalloc(2048);
	cdc_printf("Scanning bad block ... \n");
	tick = xTaskGetTickCount();
	for(i = 0; i < 65535; i++){
		xStatus = xReadFlashPages( i, 1,buffer, 100 );
		
		//if( *((unsigned char *)(&__DMA_NAND_AUXILIARY_BUFFER)) != 0xFF ){
		if( ecc_res[0] == 0xE ||
			ecc_res[1] == 0xE ||
			ecc_res[2] == 0xE ||
			ecc_res[3] == 0xE   )
		{
			cdc_printf("page: %d, ecc0=%02x, ecc1=%02x, ecc2=%02x, ecc3=%02x \n",i ,ecc_res[0], ecc_res[1], ecc_res[2], ecc_res[3]);
			cdc_printf("Found 1 bad block at page %d \n", i);
			badblocks++;
		}
		if( (ecc_res[0] > 0 && ecc_res[0] < 9) ||
			(ecc_res[1] > 0 && ecc_res[1] < 9) ||
			(ecc_res[2] > 0 && ecc_res[2] < 9) ||
			(ecc_res[3] > 0 && ecc_res[3] < 9)   )
		{
			cdc_printf("page: %d, ecc0=%02x, ecc1=%02x, ecc2=%02x, ecc3=%02x \n",i ,ecc_res[0], ecc_res[1], ecc_res[2], ecc_res[3]);
			cdc_printf("Found 1 unstable block at page %d \n", i);
			unstable++;
		}		
		
		if( (i+1) % 6553 == 0){
			p++;
			cdc_printf("%d %%, Speed %.2f KB/s ...\n",p*10, 6553.0/(xTaskGetTickCount() - tick) * 2 );
			
			tick = xTaskGetTickCount();
		}
	}

	cdc_printf("Total page: %d, total bad page: %d, unstable: %d \n", i+1, badblocks, unstable);
	vPortFree(buffer);
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

static const CLI_Command_Definition_t cmd_dwrallonec = {
	"dwrallonec",
	"\r\ndwrallonec - Write a page of flash by 0x5A'.\r\n    <param1> which page.\r\n",
	dwrallonec_cb,
	1
};

static const CLI_Command_Definition_t cmd_deraseb = {
	"deraseb",
	"\r\nderaseb - Erase a flash block'.\r\n    <param1> which block.\r\n",
	deraseb_cb,
	1
};

static const CLI_Command_Definition_t cmd_erase_all = {
	"erase_all",
	"\r\nerase_all - Erase all flash blocks'\r\n",
	erase_all_cb,
	0
};

static const CLI_Command_Definition_t cmd_erase_data = {
	"erase_data",
	"\r\nerase_all - Erase all flash blocks'\r\n",
	erase_data_cb,
	0
};



void vCDC_Console(){
	
	vTaskDelay(500);
	
	FreeRTOS_CLIRegisterCommand(&cmd_tasklist);
	FreeRTOS_CLIRegisterCommand(&cmd_dreadpage);
	FreeRTOS_CLIRegisterCommand(&cmd_drbtest);
	FreeRTOS_CLIRegisterCommand(&cmd_chkdsk);
	FreeRTOS_CLIRegisterCommand(&cmd_dwrallonec);
	FreeRTOS_CLIRegisterCommand(&cmd_deraseb);
	FreeRTOS_CLIRegisterCommand(&cmd_erase_all);
	FreeRTOS_CLIRegisterCommand(&cmd_erase_data);
	
	
	
	for(;;){
		if(xQueueReceive( CDCCmdLineQueue, (&buf), ( TickType_t ) portMAX_DELAY ) == pdTRUE ){
			FreeRTOS_CLIProcessCommand(buf,console_output_buffer,sizeof(console_output_buffer));
			cdc_printf("%s",console_output_buffer);
			memset(buf,0,sizeof(buf));
		}
	}
}