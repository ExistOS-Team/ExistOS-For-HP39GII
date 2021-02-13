
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

#include <assert.h>
#include <sys/times.h>


void vAssertCalled(){
    uartdbg_dump_regs();
    uartdbg_print_dumpregs();
    return;
    //while(1);
}

void load_linux(){
    int pid = 2;
    int fd;
    int status;
    FILE *fp = NULL;
    fp = fopen("/xipimage","rb");
    if(fp == NULL){
        printf("Linux image not load.\n");
        return;
    }
    fd = fileno(fp);
    set_current_running_pid(pid);
    create_vm_space(pid);
    status = create_new_task_space_zone(pid,fd,0,"LINUXIMG"
                            ,ZONE_ATTR_R | ZONE_ATTR_W | ZONE_ATTR_X,
                            0xB0000000, 0xBF000000);
    printf("Load img 1:%d\n",status);

    status = create_new_task_space_zone(pid,
                            get_page_file_fd(),0,
                            "VMLINUX"
                            ,ZONE_ATTR_R | ZONE_ATTR_W | ZONE_ATTR_X,
                            0xA0000000, 0xA0040000);

    status = create_new_task_space_zone(pid,
                            get_page_file_fd(),0,
                            "VMIRQ"
                            ,ZONE_ATTR_R | ZONE_ATTR_W | ZONE_ATTR_X,
                            0, 4096);

    printf("Load img 2:%d\n",status);
    
    printf("RD img 2:%08x\n",*((unsigned int *)0xB0000000));
    void (*kernel)(uint32_t reserved, uint32_t mach, uint32_t dt) = (void (*)(uint32_t, uint32_t, uint32_t))(0xB0000000);
    kernel(0, 193, 0x001FC000);
    //dump_vm_spaces();
    //dump_vfs_descriptor_chains();
    //unsigned int entry_point = 0xB0000000;
    //(*((void (*)())(entry_point)))();

}


void vFaultTask(void *pvParameters) {
    
    int ret;
    vTaskDelay(3000);
    printf("start run elf\n");
    //core_main(0,0);
    //load_linux();
    ret = create_process("test","/test.elf");
    //ret = create_process("test","/vmlinux");
    //printf("create_process:%08x\n",ret); 
    
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


