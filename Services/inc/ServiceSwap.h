

#ifndef SERVICE_SWAP_H
#define SERVICE_SWAP_H

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "ff.h"

QueueHandle_t Q_MEM_Exception;

typedef struct MEM_Exception{
	TaskHandle_t ExceptionTaskHandle;
	unsigned int accessFaultAddress;
	unsigned int insFaultAddress;
	unsigned int FSR;
	
}MEM_Exception;


unsigned int pageFaultISR(TaskHandle_t ExceptionTaskHandle, unsigned int accessFaultAddress, unsigned int insFaultAddress, unsigned int FSR);

void vServiceSwap( void *pvParameters );
FIL* getPageFileHandle();


#endif

