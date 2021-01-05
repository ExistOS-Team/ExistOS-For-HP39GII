

#ifndef SERVICE_RAW_FLASH_H
#define SERVICE_RAW_FLASH_H

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

typedef enum {
	NO_ERROR,
	DEVICE_BUSY,
	TIMEOUT,
	OPERATION_FAIL,
	BAD
}rawFlashStatus;

typedef enum {
	NONE,
	WRTIE,
	READ,
	CHECK
}OperationType;


typedef struct OperationQueueInfo{
	unsigned char OperationType;
	unsigned int whichPage;
	void *flashDataInBuffer;
	void *flashMetaInBuffer;
	void *flashDataOutBuffer;
	void *flashMetaOutBuffer;
}OperationQueue;

QueueHandle_t flashOperationQueue;



extern volatile unsigned char __DMA_NAND_AUXILIARY_BUFFER[65];

unsigned int isNANDinited();

BaseType_t xWriteFlashPages( unsigned int start_page, unsigned int pages, void *data_buffer, void *meta_buffer, unsigned int timeout_ms ) ;
BaseType_t xReadFlashPages( unsigned int start_page, unsigned int pages, void *buffer, unsigned int timeout_ms );
BaseType_t xGetFlashStatus( void );
BaseType_t xEraseFlashBlocks( unsigned int start_block, unsigned int blocks, unsigned int timeout_ms ) ;

void vServiceRawFlash( void *pvParameters );

#endif

