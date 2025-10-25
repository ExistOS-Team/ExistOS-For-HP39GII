#include "services.h"
#include "tusb.h"

void vTaskTinyUSB(void *pvParameters)
{
    tusb_init();
    for(;;)
        tud_task();
}