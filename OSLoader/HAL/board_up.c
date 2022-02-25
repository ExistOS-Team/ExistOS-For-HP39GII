

#include "clkctrl_up.h"
#include "uart_up.h"
#include "interrupt_up.h"

#include "board_up.h"
#include "mtd_up.h"
#include "display_up.h"
#include "keyboard_up.h"

#include "../debug.h"

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


}