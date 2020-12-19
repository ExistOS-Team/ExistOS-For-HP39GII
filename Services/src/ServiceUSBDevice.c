#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

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

#if CFG_TUSB_DEBUG
  #define USBD_STACK_SIZE     (3*configMINIMAL_STACK_SIZE)
#else
  #define USBD_STACK_SIZE     (3*configMINIMAL_STACK_SIZE/2)
#endif

StackType_t  usb_device_stack[USBD_STACK_SIZE];
StaticTask_t usb_device_taskdef;


unsigned char buffer[512];

void cdc_printf(const char * fmt, ...)
{
	int i = 0;
	va_list args;
	tud_cdc_write_clear();
	va_start(args,fmt);
	vsprintf(buffer, fmt, args);
	while(!tud_cdc_write_available()){
		vTaskDelay(1);
	}
	while(buffer[i]){
		tud_cdc_write_char(buffer[i++]);
	}
	tud_cdc_write_flush();
	va_end(args);
	memset(buffer,0,512);
}



uint8_t buf[64];
void vServiceUSBCDC( void *pvParameters ){
	
	for(;;) {
		
		if ( tud_cdc_available() )
		{	
			uint32_t count = tud_cdc_read(buf, sizeof(buf));
			xQueueSend(CDCCmdLineQueue, buf , ( TickType_t ) 0 );
			memset(buf,0,sizeof(buf));
			tud_cdc_read_flush();
		}
		vTaskDelay(10);
	}
	
}


void vUsbDeviceTask(){
	
	tusb_init();
	for(;;) {
		tud_task();
	}
}

void vServiceUSBDevice( void *pvParameters )
{
	CDCCmdLineQueue =  xQueueCreate(32, 64);
	vTaskDelay(1000);
	
	xTaskCreateStatic( vUsbDeviceTask, "usbd", USBD_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, usb_device_stack, &usb_device_taskdef);
	
	
	
	
	for(;;) {
		vTaskSuspend(NULL);
	}
	
	
}