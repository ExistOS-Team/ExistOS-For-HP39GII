
#include "FreeRTOS.h"
#include "task.h"

#include "clkctrl_up.h"
#include "uart_up.h"
#include "interrupt_up.h"

#include "board_up.h"
#include "mtd_up.h"
#include "display_up.h"
#include "keyboard_up.h"
#include "rtc_up.h"

#include "../debug.h"

bool driverWaitTrueF(bool (*f)(), TickType_t timeout)
{
    while(((*f)()) == false)
    {
        //vTaskDelay(1);
        portDelayus(1);
        if(timeout != portMAX_DELAY)
        {
            if(timeout >= 1)
            {
                timeout -= 1;
            }else{
                return true;
            }
        }
    }
    return false;
}

bool driverWaitFalseF(bool (*f)(), TickType_t timeout)
{
    while(((*f)()) == true)
    {
        //vTaskDelay(1);
        portDelayus(1);
        if(timeout != portMAX_DELAY)
        {
            if(timeout >= 1)
            {
                timeout -= 1;
            }else{
                return true;
            }
        }
    }
    return false;
}

void boardInit(void)
{
    INFO("portBoardInit\n");
    
    CLKCtrlInit();

    

    portBoardInit();

    uartInit();
    
    IRQInit();
    
    MTD_InterfaceInit();
    
    Display_InterfaceInit();
    
    portKeyboardGPIOInit();

    
    rtc_init();

    
#ifdef ENABLE_AUIDIOOUT
    stmp_audio_init();
#endif
}

 
