
#include "pageman.h"
#include <stdio.h>
#include <malloc.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

char testx[]="qweasdzxcqweasdzxcq2";

void vFaultTask(void *pvParameters) {

    create_vm_space(xTaskGetHandle(pcTaskGetName(NULL)));
    create_new_task_space_zone(xTaskGetHandle(pcTaskGetName(NULL)),get_page_file_fd(),0,"BSS",ZONE_ATTR_R | ZONE_ATTR_W,0x1000, 0x2000);
    
    unsigned char *buf = 0x2000;
    struct stat *fstat_ = 0x2100;
    int fd;
    fd = open("/config",O_RDWR);

    create_new_task_space_zone(xTaskGetHandle(pcTaskGetName(NULL)),fd,0,"DATA",ZONE_ATTR_R | ZONE_ATTR_W,0x4000, 0x4000 + 64 * 1024);
    /*
    char i = 'A';
    for(buf = 0x4000; buf < 0x4000 + 63 * 1024; buf++){
        *buf = i;
        i++;
        if(i > 'Z')i = 'A';

    }*/
    //sync_all_write_back_cache_page();
    

    //fsync(fd);
    //close(fd);
    printf("%s\n",buf);
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
   

    dump_vfs_descriptor_chains();

    unsigned int *test_p = 0x1000;
/*
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
    //xTaskCreate(vFaultTask,"Falut Task", configMINIMAL_STACK_SIZE, NULL, 3, NULL);

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
    vTaskDelay(4000);
    
    dump_vm_spaces();
    malloc_stats();
    for (;;) {
        vTaskDelay(1000);
    }
}


