#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* System serive includes. */
#include "ServiceDebug.h"

/* Library includes. */
#include "display.h"
#include "irq.h"
#include "keyboard.h"
#include "regsuartdbg.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

//KeyMessage *CurrentKeyMessage;

void vServiceKeyboard(void *pvParameters) {

    //	DebugQueue = xQueueCreate(64, sizeof(DebugMessage *));

    for (;;) {
        vTaskDelay(50);
        key_scan();
        //xQueueReceive( DebugQueue, &( CurrentDebugMessage ), ( TickType_t ) portMAX_DELAY );
    }
}