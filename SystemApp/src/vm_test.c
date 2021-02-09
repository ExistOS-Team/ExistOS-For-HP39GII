
#include "pageman.h"
#include <stdio.h>
#include <malloc.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "regsdigctl.h"
#include "uart_debug.h"

#include "ProcessesMan.h"
#include "swi_system_call.h"

#include <assert.h>

char testx[]="qweasdzxcqweasdzxcq2";


void vAssertCalled(){
    uartdbg_dump_regs();
    uartdbg_print_dumpregs();
    return;
    //while(1);
}

extern volatile uint32_t *pxCurrentTCB;
void vFaultTask(void *pvParameters) {

    int ret;
    ret = create_process("test","/test.elf");
    printf("create_process:%08x\n",ret); 

    vTaskDelete(NULL);
    for (;;) {
        vTaskDelay(1000);
    }
}

void vm_test(){

    xTaskCreate(vFaultTask,"Falut Task", configMINIMAL_STACK_SIZE*3, NULL, 2, NULL);
    vTaskDelay(3000);
    //dump_vm_spaces();
    malloc_stats();
    for (;;) {
        vTaskDelay(1000);
    }
}


