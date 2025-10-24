#include "services.h"
#include "vmMgr.h"

extern TaskHandle_t pSysTask;
bool g_vm_inited = false;

void vVMMgrSvc(void *pvParameters)
{
    //vTaskDelay(pdMS_TO_TICKS(10));
    vmMgr_init();
    g_vm_inited = true;
    for(;;){
        vmMgr_task();
    }
}