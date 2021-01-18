

#ifndef SERVICE_USBDEVICE_H
#define SERVICE_USBDEVICE_H

/* Kernel includes. */
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

QueueHandle_t CDCCmdLineQueue;

void vServiceUSBCDC(void *pvParameters);
void vServiceUSBDevice(void *pvParameters);

void cdc_printf(const char *fmt, ...);
void cdc_putchar(const char c);

#endif
