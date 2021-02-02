
#include "pageman.h"
#include <stdio.h>
#include <malloc.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "regsdigctl.h"


char testx[]="qweasdzxcqweasdzxcq2";



void vFaultTask(void *pvParameters) {
    unsigned char *buf = 0;
    struct stat *fstat_ = 0x2100;
    create_process("test","/test.elf");

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
    for (;;) {
        vTaskDelay(1000);
    }
}

void vm_test(){

    //vTaskDelay(8000);
    xTaskCreate(vFaultTask,"Falut Task", configMINIMAL_STACK_SIZE, NULL, 2, NULL);

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
    
    dump_vm_spaces();
    //dump_vm_spaces();
    //dump_vfs_descriptor_chains();
    malloc_stats();
    for (;;) {
        vTaskDelay(1000);
    }
}


