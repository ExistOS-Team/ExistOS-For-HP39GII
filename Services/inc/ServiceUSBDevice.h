

#ifndef SERVICE_USBDEVICE_H
#define SERVICE_USBDEVICE_H

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"



/*
typedef enum {
	DEBUG_MSG_TYPE_NOP,
	DEBUG_MSG_TYPE_TO_UART,
	DEBUG_MSG_TYPE_TO_CONSOLE
}DebugMsgType;

typedef struct DebugMessage{
	unsigned int DebugMsgType;
	unsigned char *text;
}DebugMessage;

QueueHandle_t DebugQueue;
*/

void vServiceUSBDevice( void *pvParameters );

#endif

