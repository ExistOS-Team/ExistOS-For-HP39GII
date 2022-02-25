#include "regs.h"
#include "regsuartdbg.h"

#include "uart_up.h"




unsigned int is_uartdbg_busy(void){
	return BF_RD(UARTDBGFR, BUSY);
}

void uartdbg_putc(unsigned char c)
{
	while(is_uartdbg_busy());
	BF_WR(UARTDBGDR, DATA, c);
}



void uart_putc(unsigned char c)
{
	uartdbg_putc(c);
}