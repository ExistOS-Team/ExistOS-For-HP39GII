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


//DebugMessage *CurrentDebugMessage;
/*
static void usb_drv_reset(void)
{
	REG_USBCMD &= ~USBCMD_RUN;
	//REG_PORTSC1 = (REG_PORTSC1 & ~PORTSCX_PHY_TYPE_SEL) | USB_PORTSCX_PHY_TYPE;
    vTaskDelay(500);	//等待一段时间使主机释放该USB设备
	REG_USBCMD |= USBCMD_CTRL_RESET;
    while (REG_USBCMD & USBCMD_CTRL_RESET);
	
}
*/
void vServiceUSBDevice( void *pvParameters )
{
	
	//DebugQueue = xQueueCreate(64, sizeof(DebugMessage *));
	
	
	//usb_drv_reset();
	
	tusb_init();
	
	
	for(;;){
		 tud_task();
		//xQueueReceive( DebugQueue, &( CurrentDebugMessage ), ( TickType_t ) portMAX_DELAY );
	}
	
	
}