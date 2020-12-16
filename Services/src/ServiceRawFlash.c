#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

/* System serive includes. */
#include "ServiceRawFlash.h"

/* Library includes. */
#include "regsuartdbg.h"
#include "irq.h"
#include "raw_flash.h"


/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

extern unsigned int dma_error;


SemaphoreHandle_t flashLock;

extern volatile unsigned int dmaOperationCompleted;
extern volatile unsigned int eccOperationCompleted;
extern unsigned char address_page_data[5];

							   
BaseType_t xReadFlashPages( unsigned int start_page, unsigned int pages, void *buffer, unsigned int timeout_ms ) {
	unsigned int count = 0;
	unsigned int startTick = xTaskGetTickCount();
	if(xSemaphoreTake(flashLock, timeout_ms) == pdFALSE){
		return DEVICE_BUSY;
	}
	
	while(count < pages){
		startTick = xTaskGetTickCount();
		while( !dmaOperationCompleted ){
		if( xTaskGetTickCount() - startTick > timeout_ms){
				xSemaphoreGive(flashLock);
				return TIMEOUT;
		}
	}
		
		startTick = xTaskGetTickCount();
		taskENTER_CRITICAL();
		set_page_address_data(start_page + count);
		GPMI_read_block_with_ecc8(NAND_CMD_READ0,NAND_CMD_READSTART,address_page_data,buffer,4);
		taskEXIT_CRITICAL();
		count++;
		while( !dmaOperationCompleted || !eccOperationCompleted ){
			if( xTaskGetTickCount() - startTick > timeout_ms){
				xSemaphoreGive(flashLock);
				return TIMEOUT;
			}
		}
 
		
	}
	
	xSemaphoreGive(flashLock);
	return NO_ERROR;
}

unsigned char NMETA[19] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

BaseType_t xWriteFlashPages( unsigned int start_page, unsigned int pages, void *data_buffer, void *meta_buffer, unsigned int timeout_ms ) {
	unsigned int count = 0;
	unsigned int startTick = xTaskGetTickCount();
	if(xSemaphoreTake(flashLock, timeout_ms) == pdFALSE){
		return DEVICE_BUSY;
	}
	
	if(meta_buffer = NULL){
		meta_buffer = NMETA;
		
	}
	
	while(count < pages){
		startTick = xTaskGetTickCount();
		while( !dmaOperationCompleted ){
		if( xTaskGetTickCount() - startTick > timeout_ms){
				xSemaphoreGive(flashLock);
				return TIMEOUT;
		}
	}
		startTick = xTaskGetTickCount();
		taskENTER_CRITICAL();
		GPMI_write_block_with_ecc8(NAND_CMD_SEQIN,NAND_CMD_PAGEPROG,NAND_CMD_STATUS,
								start_page + count ,data_buffer,meta_buffer);
		taskEXIT_CRITICAL();
		count++;
		data_buffer += 2048;
		meta_buffer += 19;
		
		while( !dmaOperationCompleted ){
			if( xTaskGetTickCount() - startTick > timeout_ms){
				xSemaphoreGive(flashLock);
				return TIMEOUT;
			}
		}
		eccOperationCompleted = 1;
		
		if(dma_error){
			xSemaphoreGive(flashLock);
			return OPERATION_FAIL;
		}
		
	}

	xSemaphoreGive(flashLock);
	return NO_ERROR;
}

BaseType_t xEraseFlashBlocks( unsigned int start_block, unsigned int blocks, unsigned int timeout_ms ) {
	unsigned int count = 0;
	unsigned int startTick = xTaskGetTickCount();
	if(xSemaphoreTake(flashLock, timeout_ms) == pdFALSE){
		return DEVICE_BUSY;
	}
	
	while(count < blocks){
		startTick = xTaskGetTickCount();
		while( !dmaOperationCompleted ){
				if( xTaskGetTickCount() - startTick > timeout_ms){
						xSemaphoreGive(flashLock);
						return TIMEOUT;
				}
			}
		startTick = xTaskGetTickCount();
		taskENTER_CRITICAL();
		GPMI_erase_block_cmd(NAND_CMD_ERASE1, NAND_CMD_ERASE2, NAND_CMD_STATUS, start_block + count);
		taskEXIT_CRITICAL();
		vTaskDelay(3);
		count++;
		
		while( !dmaOperationCompleted ){
			if( xTaskGetTickCount() - startTick > timeout_ms){
				xSemaphoreGive(flashLock);
				return TIMEOUT;
			}
		}
		eccOperationCompleted = 1;
		
		if(dma_error){
			xSemaphoreGive(flashLock);
			return OPERATION_FAIL;
		}
		
	}

	xSemaphoreGive(flashLock);
	return NO_ERROR;
}


unsigned int pageInBlock(unsigned int page){
	return (page / 64);
}

unsigned int blockStartPage(unsigned int block){
	return (block * 64);
}

BaseType_t xGetFlashStatus(){
	if(dmaOperationCompleted && eccOperationCompleted){
		return NO_ERROR;
	}else{
		return DEVICE_BUSY;
	}
}


OperationQueue  CurrentOperation;

void vServiceRawFlash( void *pvParameters ){
	
	flashLock = xSemaphoreCreateMutex();
	flashOperationQueue = xQueueCreate(256, sizeof(OperationQueue));
	
	NAND_init();

	//vTaskSuspend(NULL);
	for(;;){
		if(xQueueReceive( flashOperationQueue, (&CurrentOperation), ( TickType_t ) portMAX_DELAY ) == pdTRUE ){
			switch(CurrentOperation.OperationType){
				case WRTIE:
					xWriteFlashPages( CurrentOperation.whichPage, 1, 
											CurrentOperation.flashDataInBuffer, 
											CurrentOperation.flashMetaInBuffer, 
											2000 ) ;
				break;
				
				
			}
			
		
			
		}
		
	}
	
	
}