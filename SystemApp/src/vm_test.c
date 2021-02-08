
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
    //unsigned char *buf = 0;
    //struct stat *fstat_ = 0x2100;
    /*
    set_current_running_pid(1);
    create_vm_space(1);
    create_new_task_space_zone(1,get_page_file_fd(),0,"BSS",ZONE_ATTR_R | ZONE_ATTR_W,0x1000, 0x2000);   
    *((unsigned int*) 0x1000 ) = 0x200;
    vTaskDelay(2000);
    printf("\n\nRDDATA:%08x\n", *((unsigned int*) 0x1000 ) );*/
   // while(1){
        
        //syscall(1,2,3,4,5,6,7);
        //syscall(0,(unsigned int)"Hello World\n",13,0,0,0,SWI_WRITE);

        //vTaskSuspendAll();
        
/*
                asm volatile("mrs r1,cpsr_all");
        asm volatile("bic r1,r1,#0x1f");
        asm volatile("orr r1,r1,#0x17");
        asm volatile("msr cpsr_all,r1");
        uartdbg_printf("A[%x,%x]\n",((unsigned int)pxCurrentTCB), pxCurrentTCB[1]);
        vTaskSuspendAll();
        uartdbg_printf("B[%x,%x]\n",((unsigned int)pxCurrentTCB), pxCurrentTCB[1]);
        xTaskResumeAll();
        uartdbg_printf("C[%x,%x]\n",((unsigned int)pxCurrentTCB), pxCurrentTCB[1]);
                asm volatile("mrs r1,cpsr_all");
        asm volatile("bic r1,r1,#0x1f");
        asm volatile("orr r1,r1,#0x1f");
        asm volatile("msr cpsr_all,r1");*/
        //vTaskDelay(200);
        //int ret = syscall(1,3,5,7,9,2,4);
        //syscall(0,(unsigned int)"Hello World\n",13,0,0,0,SWI_WRITE);
        //printf("call finish:%d\n",ret);
        //__asm volatile ( "swi 0" ::: "memory" );
        //vTaskDelay(500);
        //*((unsigned int*) 0x1000 ) = 0x200;
      // __asm volatile ( "BKPT 16389" ::: "memory" );
        //extern volatile uint32_t *pxCurrentTCB;
        
       // xTaskResumeAll();

        /*
        vTaskDelay(1);
        vTaskSuspendAll();
        __asm volatile("swi #1");
        uartdbg_printf("test\n");
        xTaskResumeAll();*/
        
   // }

   //assert(0);
    //int ret;
    //struct stat;
    //ret = stat("/test.elf",&stat);
    //printf("stat ret:%d\n",ret);
    int ret = create_process("test","/test.elf");
    printf("create_process:%08x",ret);
    //create_vm_space(xTaskGetHandle(pcTaskGetName(NULL)));
    //create_new_task_space_zone(xTaskGetHandle(pcTaskGetName(NULL)),get_page_file_fd(),0,"BSS",ZONE_ATTR_R | ZONE_ATTR_W,0x1000, 0x2000);
   /*
    int fd;
    fd = open("/Q2.TXT",O_RDWR | O_CREAT);
    create_new_task_space_zone(xTaskGetHandle(pcTaskGetName(NULL)),fd,0,"DATA",ZONE_ATTR_R | ZONE_ATTR_W,0x1000, 0x1000 + 128 * 1024);
    
    //write(fd,"asd",4);
    char i = 'a';
    for(buf = 0x1000; buf < 0x1000 + 127 * 1024; buf++){

        *buf = i;
        i++;
        if(i>'z')i='a';
    }
    
    char p;
    for(buf = 0x1000; buf < 0x1000 + 80; buf++){
        p = *buf;
        printf("%c",p);
    }
    printf("\n");
*/
    //dump_vm_spaces();
   



    //close(fd);
    

    // *buf = 0x12;
    //memcpy(buf, testx, sizeof(testx));
    /*
    printf("%09x\n",*buf);
    printf("%c\n",*buf);

    *buf = 0x34;

    printf("%c\n",*buf);
*/
    
    //memcpy_asm(0,testx,sizeof(testx));

    //sync_all_write_back_cache_page();
    
    /*
    FILE *fp;
    fp = fopen("/test2.txt","wb+");
    printf("fileno :%d\n",fileno(fp));
*/
    //vm_load_exec_elf("/test.elf");
    //dump_vm_spaces();

    //fsync(fd);
    //close(fd);
    //printf("%s\n",buf);
    //memcpy(buf,testx,10);
/*
    fd = open("/testf",O_CREAT | O_RDWR);
    lseek(fd,0x1000,SEEK_SET);
    write(fd,buf,10);
    fsync(fd);
    close(fd);
*/
    /*
    fd = open("/config",O_RDWR);
    printf("open res:%d\n",fd);
    lseek(fd,0,SEEK_SET);
    write(fd, testx, sizeof(testx));
    lseek(fd,0,SEEK_SET);
    FILE *fp = fopen("/a.txt", "a+");
    fprintf(fp,"hello world!\n");
    fclose(fp);
    fsync(fd);
    fstat(fd, fstat_);
    printf("/config size:%d\n",fstat_->st_size);
    stat("/a.txt",fstat_);
    access("/PAGEFILE",R_OK);
    printf("/a.txt size:%d\n",fstat_->st_size);
    read(fd, buf, 10);
    buf[9] = 0;
    printf("test read:%s\n",buf);
    mkdir("/testdir",0777);
    */
   
   
    
/*
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
 
*/
    vTaskDelete(NULL);
    for (;;) {
        vTaskDelay(1000);
    }
}

void vm_test(){

    //vTaskDelay(8000);
    xTaskCreate(vFaultTask,"Falut Task", configMINIMAL_STACK_SIZE*3, NULL, 2, NULL);

    //FILE *test_file;
    //test_file = fopen("/CONFIG", "wr+");
    /*
    int fd;
    fd = open("/config",O_RDWR);
    printf("open res:%d\n",fd);
    dump_vfs_descriptor_chains();
    char *tesbuf = pvPortMalloc(10);
    read(fd, tesbuf, 10);
    printf("test read:%s\n",tesbuf);
*/
    //vTaskDelay(6000);
    
    //dump_vm_spaces();
    //dump_vm_spaces();
    //dump_vfs_descriptor_chains();
    malloc_stats();
    for (;;) {
        vTaskDelay(1000);
    }
}


