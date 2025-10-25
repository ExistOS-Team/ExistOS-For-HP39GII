#include "services.h"
#include "FTL_up.h"

extern uint32_t g_FTL_status;

void vFTLSvc(void *pvParameters)
{
    g_FTL_status = FTL_init();
    for(;;)
        FTL_task();
}