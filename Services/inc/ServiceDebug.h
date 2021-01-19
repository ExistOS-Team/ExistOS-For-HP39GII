

#ifndef SERVICE_DEBUG_H
#define SERVICE_DEBUG_H

/* Kernel includes. */
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

typedef enum {
    DEBUG_MSG_TYPE_NOP,
    DEBUG_MSG_TYPE_TO_UART,
    DEBUG_MSG_TYPE_TO_CONSOLE
} DebugMsgType;

typedef struct DebugMessage {
    unsigned int DebugMsgType;
    unsigned char *text;
} DebugMessage;

QueueHandle_t DebugQueue;

void vServiceDebug(void *pvParameters);

#endif
