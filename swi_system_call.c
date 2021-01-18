

#include "exception.h"
#include "mmu.h"
#include "uart_debug.h"
#include <stdio.h>

void src_c_swi_handler(unsigned int *arg0, unsigned int *arg1, unsigned int *arg2, unsigned int swiImmed) {
    /*
	if(swiImmed != 0)
		printf("SWI: %d, arg0:%d, arg1:%d, arg2:%d\n",swiImmed,arg0,arg1,arg2);
	
	*/
    switch (swiImmed) {
    case 1000:
        *arg2 = 55;
        printf("arg0 :%d\n", (*arg0));

    case 1001:
        switch_mode(ABT_MODE);
        uartdbg_print_regs();

        switch_mode(UND_MODE);
        uartdbg_print_regs();

        switch_mode(FIQ_MODE);
        uartdbg_print_regs();

        switch_mode(IRQ_MODE);
        uartdbg_print_regs();

        switch_mode(SYS_MODE);
        uartdbg_print_regs();

        switch_mode(SVC_MODE);
        asm volatile("nop");

        uartdbg_print_regs();
    }
}