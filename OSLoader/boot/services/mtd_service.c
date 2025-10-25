#include "services.h"
#include "mtd_up.h"

void vMTDSvc(void *pvParameters)
{
    MTD_DeviceInit();
    for(;;)
        MTD_Task();
}