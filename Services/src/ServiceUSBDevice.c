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
static void usbd_update_desc();

void vUsbDeviceTask() {
    tusb_init();
    for (;;) {
        tud_task();
    }
}

void vServiceUSBDevice(void *pvParameters) {
    //CDCCmdLineQueue = xQueueCreate(32, 64);
    vTaskDelay(600);

    usbd_set_itf(ITF_NUM_CDC);
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

#define EPNUM_CDC_NOTIF 0x81
#define EPNUM_CDC_OUT 0x02
#define EPNUM_CDC_IN 0x82

#define EPNUM_MSC_OUT 0x03
#define EPNUM_MSC_IN 0x83

#define EPNUM_HID 0x81

uint8_t* usbd_desc = NULL;

static void usbd_update_desc() {
  int itf_num = 0;
  int itf_desc_num = sizeof(usbd_enabled_itfs) / sizeof(usbd_enabled_itfs[0]);
  int config_len = TUD_CONFIG_DESC_LEN;
  size_t desc_size = 0;
  bool usb_hs = (tud_speed_get() == TUSB_SPEED_HIGH)? true:false;
  uint8_t itf_idx_cdc,itf_idx_msc,itf_idx_hid;
  for (int i = 0; i < itf_desc_num; ++i) {
    switch (usbd_enabled_itfs[i]) {
    case ITF_NUM_CDC:
      itf_idx_cdc = i;
      break;
    case ITF_NUM_MSC:
      itf_idx_msc = i;
      break;
    case ITF_NUM_HID:
      itf_idx_hid = i;
      break;
    default:
      break;
    }
  }

  uint8_t cdc_desc[] = {TUD_CDC_DESCRIPTOR(itf_idx_cdc, 4, EPNUM_CDC_NOTIF, 8, EPNUM_CDC_OUT, EPNUM_CDC_IN, usb_hs? 512:64)};
  uint8_t msc_desc[] = {TUD_MSC_DESCRIPTOR(itf_idx_msc, 5, EPNUM_MSC_OUT, EPNUM_MSC_IN, usb_hs? 512:64)};
  uint8_t hid_desc[] = {TUD_HID_DESCRIPTOR(itf_idx_hid, 6, HID_PROTOCOL_NONE,144, EPNUM_HID, CFG_TUD_HID_EP_BUFSIZE, 5)}; //注意写死的144

  for (size_t i = 0; i < itf_desc_num; i++) {
    switch (usbd_enabled_itfs[i]) {
    case ITF_NUM_CDC:
      itf_num += 2;
      desc_size += sizeof(cdc_desc);
      config_len += TUD_CDC_DESC_LEN;
      break;
    case ITF_NUM_MSC:
      itf_num++;
      desc_size += sizeof(msc_desc);
      config_len += TUD_MSC_DESC_LEN;
      break;
    case ITF_NUM_HID:
      itf_num++;
      desc_size += sizeof(hid_desc);
      config_len += TUD_HID_DESC_LEN;
      break;
    default:
      break;
    }
  }
  uint8_t config_desc[] = {TUD_CONFIG_DESCRIPTOR(1, itf_num, 0, config_len, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100)};
  desc_size += sizeof(config_desc);
  
  if(usbd_desc != NULL){
      free(usbd_desc);
  }

  uint8_t* desc = (uint8_t*)malloc(desc_size);  //GCC YES
  size_t desc_ptr = 0;
  memcpy(desc,config_desc,sizeof(config_desc));
  desc_ptr += sizeof(config_desc);
  for (int i = 0; i < itf_desc_num; ++i){
     switch (usbd_enabled_itfs[i]) {
    case ITF_NUM_CDC:
      memcpy(&desc[desc_ptr], cdc_desc, sizeof(cdc_desc));
      desc_ptr += sizeof(cdc_desc);
      break;
    case ITF_NUM_MSC:
      memcpy(&desc[desc_ptr], msc_desc, sizeof(msc_desc));
      desc_ptr += sizeof(msc_desc);
      break;
    case ITF_NUM_HID:
      memcpy(&desc[desc_ptr], hid_desc, sizeof(hid_desc));
      desc_ptr += sizeof(hid_desc);
      break;
    default:
      break;
    }
  }
  usbd_desc = desc;
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
    usbd_update_desc();
    usb_reload();
    return 0;
}