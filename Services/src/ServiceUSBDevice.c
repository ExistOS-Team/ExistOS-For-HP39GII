#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "tusb.h"

/* System serive includes. */
#include "ServiceUSBDevice.h"

#include "cdc_console.h"

/* Library includes. */
#include "display.h"
#include "irq.h"
#include "regsuartdbg.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#if CFG_TUSB_DEBUG
#define USBD_STACK_SIZE (3 * configMINIMAL_STACK_SIZE)
#else
#define USBD_STACK_SIZE (3 * configMINIMAL_STACK_SIZE / 2)
#endif

StackType_t usb_device_stack[USBD_STACK_SIZE];
StaticTask_t usb_device_taskdef;

void vUsbDeviceTask() {
    tusb_init();
    for (;;) {
        tud_task();
    }
}

void vServiceUSBDevice(void *pvParameters) {
    //CDCCmdLineQueue = xQueueCreate(32, 64);
    vTaskDelay(3000);
    
    xTaskCreateStatic(vUsbDeviceTask, "usbd", USBD_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, usb_device_stack, &usb_device_taskdef);
    xTaskCreate(vCDC_Console, "CDC Console", configMINIMAL_STACK_SIZE, NULL, 3, NULL);
    for (;;) {
        vTaskSuspend(NULL);
    }
}