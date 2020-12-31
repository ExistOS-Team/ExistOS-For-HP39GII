#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

/* System serive includes. */
#include "ServiceDebug.h"

/* Library includes. */
#include "regsuartdbg.h"
#include "irq.h"
#include "display.h"
#include "keyboard.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"


//KeyMessage *CurrentKeyMessage;

void vServiceKeyboard( void *pvParameters )
{
	
//	DebugQueue = xQueueCreate(64, sizeof(DebugMessage *));
	
	
	for(;;){
		vTaskDelay(10);
		key_scan();
		//xQueueReceive( DebugQueue, &( CurrentDebugMessage ), ( TickType_t ) portMAX_DELAY );
	}
	
	
}