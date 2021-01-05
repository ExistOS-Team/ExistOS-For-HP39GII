#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

/* System serive includes. */
#include "ServiceDebug.h"

/* Library includes. */
#include "regsuartdbg.h"
#include "irq.h"
#include "display.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"


DebugMessage *CurrentDebugMessage;

void vServiceDebug( void *pvParameters )
{
	
	//DebugQueue = xQueueCreate(64, sizeof(DebugMessage *));
	
	vTaskDelete(NULL);
	
	for(;;){
		xQueueReceive( DebugQueue, &( CurrentDebugMessage ), ( TickType_t ) portMAX_DELAY );
	}
	
	
}