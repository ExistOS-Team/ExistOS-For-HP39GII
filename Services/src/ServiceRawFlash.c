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

SemaphoreHandle_t flashLock, readPageLock;

extern volatile unsigned int dmaOperationCompleted;
extern volatile unsigned int eccOperationCompleted;
extern unsigned char address_page_data[5];
void set_page_address_data(unsigned int pageNumber);
void GPMI_read_block_with_ecc8(unsigned char set_read_command,unsigned char start_read_command,
                               unsigned char *address_data, unsigned int *buffer, unsigned int address_data_size_bytes );
							   
BaseType_t xReadFlashPages( unsigned int start_page, unsigned int pages, unsigned int *buffer, unsigned int timeout_ms ) {
	unsigned int count = 0;
	unsigned int startTick = xTaskGetTickCount();
	if(xSemaphoreTake(flashLock, timeout_ms) == pdFALSE){
		return DEVICE_BUSY;
	}
	
	while(count < pages){
		set_page_address_data(start_page + count);
		GPMI_read_block_with_ecc8(NAND_CMD_READ0,NAND_CMD_READSTART,address_page_data,buffer,4);
		count++;
		while( !dmaOperationCompleted || !eccOperationCompleted ){
			if( xTaskGetTickCount() - startTick > timeout_ms){
				xSemaphoreGive(flashLock);
				return READ_TIMEOUT;
			}
		}
 
		
	}
	
	xSemaphoreGive(flashLock);
	return NO_ERROR;
}




void vServiceRawFlash( void *pvParameters ){
	
	flashLock = xSemaphoreCreateMutex();
	readPageLock = xSemaphoreCreateMutex();
	
	NAND_init();
	
	
	
	
	vTaskSuspend(NULL);
	for(;;){
		vTaskDelay(portMAX_DELAY);
	}
	
	
}