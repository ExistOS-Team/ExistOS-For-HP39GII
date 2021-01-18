#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* System serive includes. */
#include "ServiceDebug.h"

/* Library includes. */
#include "display.h"
#include "irq.h"
#include "regsuartdbg.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

DebugMessage *CurrentDebugMessage;

void vServiceDebug(void *pvParameters) {

    //DebugQueue = xQueueCreate(64, sizeof(DebugMessage *));

    vTaskDelete(NULL);

    for (;;) {
        xQueueReceive(DebugQueue, &(CurrentDebugMessage), (TickType_t)portMAX_DELAY);
    }
}