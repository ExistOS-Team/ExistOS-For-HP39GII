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
#define USBD_STACK_SIZE (12 * configMINIMAL_STACK_SIZE)
#else
#define USBD_STACK_SIZE (12 * configMINIMAL_STACK_SIZE / 2)
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
    vTaskDelay(600);
    
    xTaskCreateStatic(vUsbDeviceTask, "usbd", USBD_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, usb_device_stack, &usb_device_taskdef);

    vTaskDelete(NULL);
    for (;;) {
        vTaskSuspend(NULL);
    }
}

int usbd_enabled_itfs[5]= {0,0,0,0,0};

TaskHandle_t usbd_cdc_task_handle;

static void usbd_cdc_service_stop(){
    vTaskDelete(usbd_cdc_task_handle);
}

static void usbd_cdc_service_start(){
     xTaskCreate(vCDC_Console, "CDC Console", configMINIMAL_STACK_SIZE, NULL, 3, &usbd_cdc_task_handle);
}

void usb_reload() {
  tud_disconnect();
  vTaskDelay(1000/portTICK_RATE_MS);
  tud_connect();
}

int usbd_set_itf(int itf_num){
    if (usbd_enabled_itfs[0] == itf_num) return 0;
    if (usbd_enabled_itfs[0] == ITF_NUM_CDC){
        usbd_cdc_service_stop();
    }
    if (itf_num == ITF_NUM_CDC){
        usbd_cdc_service_start();
    }
    usbd_enabled_itfs[0] = itf_num;
    usb_reload();
    return 0;
}