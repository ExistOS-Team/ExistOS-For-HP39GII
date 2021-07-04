

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

extern uint8_t* usbd_desc;

enum {
  ITF_NUM_CDC = 1,
  ITF_NUM_MSC,
  ITF_NUM_HID
};

int usbd_set_itf(int itf_num);

#endif
