

#ifndef SERVICE_USBDEVICE_H
#define SERVICE_USBDEVICE_H

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

QueueHandle_t CDCCmdLineQueue;

void vServiceUSBCDC( void *pvParameters );
void vServiceUSBDevice( void *pvParameters );

void cdc_printf(const char * fmt, ...);

#endif

