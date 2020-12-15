#include <stdint.h>

/* System serive includes. */
#include "ServiceManger.h"
#include "ServiceGraphic.h"
#include "ServiceDebug.h"
#include "ServiceUSBDevice.h"
#include "ServiceRawFlash.h"
#include "ServiceSTMPPartition.h"


/* Library includes. */
#include "regsuartdbg.h"
#include "irq.h"
#include "display.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

GraphicMessage CurrentGraphicMessage;


void vServiceManger( void *pvParameters )
{
	
	xTaskCreate( vServiceGraphic, "Graphic Service", configMINIMAL_STACK_SIZE, NULL, 4, NULL );
	xTaskCreate( vServiceRawFlash, "RawFlash Service", configMINIMAL_STACK_SIZE, NULL, 4, NULL );
	xTaskCreate( vServiceDebug, "Debug Service", configMINIMAL_STACK_SIZE, NULL, 4, NULL );
	xTaskCreate( vServiceUSBDevice, "USB Device Service", configMINIMAL_STACK_SIZE, NULL, 4, NULL );
	xTaskCreate( vServiceUSBCDC, "USB CDC Service", configMINIMAL_STACK_SIZE, NULL, 2, NULL );
	xTaskCreate( vSTMPPartition, "STMP Partition Svc", configMINIMAL_STACK_SIZE, NULL, 2, NULL );
	
	vTaskSuspend(NULL);
	for(;;){
		
		
		vTaskDelay(portMAX_DELAY);
	}
	
	
}