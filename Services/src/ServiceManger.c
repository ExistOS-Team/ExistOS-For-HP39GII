#include <stdint.h>

/* System serive includes. */
#include "ServiceDebug.h"
#include "ServiceFatFs.h"
#include "ServiceFlashMap.h"
#include "ServiceGraphic.h"
#include "ServiceKeyboard.h"
#include "ServiceManger.h"
#include "ServiceRawFlash.h"
#include "ServiceSTMPPartition.h"
#include "ServiceUSBDevice.h"

/* Library includes. */
#include "display.h"
#include "irq.h"
#include "regsuartdbg.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

GraphicMessage CurrentGraphicMessage;

void vServiceManger(void *pvParameters) {

    xTaskCreate(vServiceGraphic, "Graphic Service", configMINIMAL_STACK_SIZE, NULL, 4, NULL);
    xTaskCreate(vServiceRawFlash, "RawFlash Service", configMINIMAL_STACK_SIZE, NULL, 4, NULL);
    xTaskCreate(vSTMPPartition, "STMP Partition Svc", configMINIMAL_STACK_SIZE, NULL, 4, NULL);
    xTaskCreate(vServiceFlashMap, "Flash Map Svc", configMINIMAL_STACK_SIZE, NULL, 4, NULL);
    xTaskCreate(vServiceDebug, "Debug Service", configMINIMAL_STACK_SIZE, NULL, 4, NULL);
    xTaskCreate(vServiceFatfs, "FATFS Service", configMINIMAL_STACK_SIZE, NULL, 4, NULL);
    xTaskCreate(vServiceKeyboard, "KeyBoard Service", configMINIMAL_STACK_SIZE, NULL, 4, NULL);

    vTaskDelete(NULL);

    vTaskSuspend(NULL);
    for (;;) {

        vTaskDelay(portMAX_DELAY);
    }
}