#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "tusb.h"

/* System serive includes. */
#include "ServiceUSBDevice.h"

/* Library includes. */
#include "regsuartdbg.h"
#include "irq.h"
#include "display.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"


void vServiceUSBCDC( void *pvParameters ){
	
	
	for(;;) {
		if ( tud_cdc_available() )
		{
			uint8_t buf[64];
	
			// read and echo back
			uint32_t count = tud_cdc_read(buf, sizeof(buf));

			for(uint32_t i=0; i<count; i++)
			{
				tud_cdc_write_char(buf[i]);
				if ( buf[i] == '\r' ) tud_cdc_write_char('\n');
			}

			tud_cdc_write_flush();
		}
		
		vTaskDelay(10);
	}
	
}



void vServiceUSBDevice( void *pvParameters )
{
	
	tusb_init();
	
	//xTaskCreate( vServiceUSBCDC, "USB CDC Service", configMINIMAL_STACK_SIZE, NULL, 3, NULL );
	
	for(;;){
		 tud_task();

	}
	
	
}