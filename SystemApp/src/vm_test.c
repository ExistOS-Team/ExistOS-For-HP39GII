
#include "pageman.h"
#include <stdio.h>
#include <malloc.h>
#include <fcntl.h>

void vFaultTask(void *pvParameters) {

    create_vm_space(xTaskGetHandle(pcTaskGetName(NULL)));
    create_new_task_space_zone(xTaskGetHandle(pcTaskGetName(NULL)),NULL,"BSS",ZONE_ATTR_R | ZONE_ATTR_W,0x1000,200 * 1024 * 1024);
     

    unsigned int *test_p = 0x1000;

    for(int i = 0; i<189; i++){
        *test_p = 189 - i;
        //printf("WR:%d\n",i);
        //vTaskDelay(10);
        test_p += ((1024 * 1024)/4);
    }

    test_p = 0x1000;
    for(int i = 0; i<189; i++){
        printf("read back:%08x\n",*test_p);
        test_p += ((1024 * 1024)/4);
    }    
   

    for (;;) {
        vTaskDelay(1000);
    }
}

void vm_test(){

    //vTaskDelay(8000);
    //xTaskCreate(vFaultTask,"Falut Task", configMINIMAL_STACK_SIZE, NULL, 3, NULL);

    //FILE *test_file;
    //test_file = fopen("/CONFIG", "wr+");
    int fd;
    fd = open("/pagefile",O_RDWR | O_CREAT);
    

    
    vTaskDelay(10000);
    
    dump_vm_spaces();
    malloc_stats();
    for (;;) {
        vTaskDelay(1000);
    }
}


