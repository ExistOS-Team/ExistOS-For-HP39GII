

#include <stdio.h>
#include <stdbool.h>

#include "SystemConfig.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "tusb.h"
#include "usbd.h"

#include "board_up.h"
#include "mtd_up.h"
#include "FTL_up.h"
#include "display_up.h"
#include "keyboard_up.h"
#include "vmMgr.h"
#include "llapi.h"
#include "llapi_code.h"

#include "../debug.h"

extern uint32_t g_FTL_status;
extern TaskHandle_t pSysTask;

bool g_vm_inited = false;

void vTaskTinyUSB(void *pvParameters)
{
  tusb_init();
  for(;;)
    tud_task();
}

void vMTDSvc(void *pvParameters)
{
  MTD_DeviceInit();
  for(;;)
    MTD_Task();
}

void vFTLSvc(void *pvParameters)
{
  g_FTL_status = FTL_init();
  for(;;)
    FTL_task();
}

void vKeysSvc(void *pvParameters)
{
  key_svcInit();
  for(;;)
  {
    key_task();
  }
  
}

void vVMMgrSvc(void *pvParameters)
{
  //vTaskDelay(pdMS_TO_TICKS(10));
  vmMgr_init();
  g_vm_inited = true;
  for(;;){
    vmMgr_task();
  }
}

void vLLAPISvc(void *pvParameters)
{
  LLAPI_init(pSysTask);
  for(;;){
    LLAPI_Task();
  }
}



void vDispSvc(void *pvParameters)
{
  DisplayInit();
  //DisplayPutStr(0, 16 * 0, "System Booting...", 0, 255, 16);
  DisplayFillBox(32, 32, 224, 64, 128);
  DisplayPutStr(72, 42, "Init display" ,255 ,128, 16);

  DisplayFillBox(48, 80, 208, 96, 200);
  DisplayFillBox(50, 82, 206, 94, 255);
  
  DisplaySetIndicate(0, 0);
  for(;;){
    DisplayTask();
  }
}
