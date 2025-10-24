#include "services.h"
#include "keyboard_up.h"

void vKeysSvc(void *pvParameters)
{
    key_svcInit();
    for(;;)
    {
        key_task();
    }
}