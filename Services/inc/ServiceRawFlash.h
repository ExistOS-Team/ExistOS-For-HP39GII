

#ifndef SERVICE_RAW_FLASH_H
#define SERVICE_RAW_FLASH_H

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

typedef enum {
	NO_ERROR,
	DEVICE_BUSY,
	READ_TIMEOUT
}rawFlashStatus;

extern unsigned int __DMA_NAND_PALLOAD_BUFFER;
extern unsigned int __DMA_NAND_AUXILIARY_BUFFER;

BaseType_t xReadFlashPages( unsigned int start_page, unsigned int pages, unsigned int *buffer, unsigned int timeout_ms );

void vServiceRawFlash( void *pvParameters );

#endif

