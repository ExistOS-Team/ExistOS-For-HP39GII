
#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>

#include "elf_user.h"

#include "ProcessesMan.h"
#include "pageman.h"
#include "memory_map.h"
#include "mmu.h"

#include "uart_debug.h"
#include "swi_system_call.h"

processes_info *processes_table;
PID_t pid_count = 10;
PID_t current_running_pid = -1;

PID_t get_current_running_task_pid(){
    return current_running_pid;
}

void set_current_running_pid(PID_t current_pid){
    current_running_pid = current_pid;
}

int vm_load_exec_elf(PID_t pid, char * elf_path, unsigned int* enrty_point){
    int status = 0;
    elf_t elf_file;
    FILE *fp = NULL;
    int fd;
    fp = fopen(elf_path,"rb");
    if(fp == NULL){
        return -1;
    }
    fd = fileno(fp);
    fseek(fp, 0, SEEK_END);
    elf_file.elfSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    elf_file.elfClass = ELFCLASS32;
    status = create_new_task_space_zone(pid,fd,0,"ELFIMAGE"
                            ,ZONE_ATTR_R,0x01000000, 0x01000000 + elf_file.elfSize);
    
    elf_file.elfFile = (unsigned int *)0x01000000;
    if(elf_check_magic((char *)elf_file.elfFile) != 0){
        return -1;
    }
    printf("ELF load.\n");
    
    int num_program_headers = elf_getNumProgramHeaders(&elf_file);
    
    int RODATA_count = 0;
    int DATA_count = 0;
    int TEXT_count = 0;
    char *name_buffer = pvPortMalloc(10);
    
    uint32_t current_flag;

    if(num_program_headers == 0)return -1;
    
    for(int i = 0; i < num_program_headers; i++){
        printf("#%d Type:%d  ", i, elf_getProgramHeaderType(&elf_file,i) & 0xF);
        printf("Phy Addr:%08X  ", elf_getProgramHeaderPaddr(&elf_file,i));
        printf("Virt Addr:%08X  ", elf_getProgramHeaderVaddr(&elf_file,i));
        printf("File offset:%08X  ", elf_getProgramHeaderOffset(&elf_file,i));
        printf("Flag:%08X  ", elf_getProgramHeaderFlags(&elf_file,i));
        printf("Length:%08X\n", elf_getProgramHeaderMemorySize(&elf_file,i));

        if( (elf_getProgramHeaderType(&elf_file,i) & PT_LOAD) && !(elf_getProgramHeaderType(&elf_file,i) & (~0xF))){
            current_flag = elf_getProgramHeaderFlags(&elf_file,i);

            if( (current_flag & PF_W) && (current_flag & PF_R) && !(current_flag & PF_X)){
                printf("Load Seg #%d = DATA\n",i);
                sprintf(name_buffer,"DATA%d",DATA_count);
                DATA_count++;
                status = create_new_task_space_zone(pid,fd,elf_getProgramHeaderOffset(&elf_file,i) ,
                            name_buffer,
                            ZONE_ATTR_R | ZONE_ATTR_W,
                            elf_getProgramHeaderVaddr(&elf_file,i),
                            elf_getProgramHeaderVaddr(&elf_file,i) + elf_getProgramHeaderMemorySize(&elf_file,i));
            }
            if( (current_flag & PF_R) && (current_flag & PF_X)){
                printf("Load Seg #%d = TEXT\n",i);
                sprintf(name_buffer,"TEXT%d",TEXT_count);
                TEXT_count++;
                status = create_new_task_space_zone(pid,fd,elf_getProgramHeaderOffset(&elf_file,i) ,
                            name_buffer,
                            ZONE_ATTR_R | ZONE_ATTR_X,
                            elf_getProgramHeaderVaddr(&elf_file,i),
                            elf_getProgramHeaderVaddr(&elf_file,i) + elf_getProgramHeaderMemorySize(&elf_file,i));
            }
            if( (current_flag & PF_R) && !(current_flag & PF_X) && !(current_flag & PF_W)){
                printf("Load Seg #%d = RODATA\n",i);
                sprintf(name_buffer,"RODATA%d",RODATA_count);
                RODATA_count++;
                status = create_new_task_space_zone(pid,fd,elf_getProgramHeaderOffset(&elf_file,i) ,
                            name_buffer,
                            ZONE_ATTR_R,
                            elf_getProgramHeaderVaddr(&elf_file,i),
                            elf_getProgramHeaderVaddr(&elf_file,i) + elf_getProgramHeaderMemorySize(&elf_file,i));
            }

            printf("load status:%d\n",status);
        }
    }
    vPortFree(name_buffer);

    status = create_new_task_space_zone(pid,get_page_file_fd(),0,
                            "STACK",
                            ZONE_ATTR_R | ZONE_ATTR_W,
                            0x70000,
                            0x90000);
    remove_task_space_zone(pid,"ELFIMAGE");

    printf("STACK:%d\n",status);
    //dump_vm_spaces();
    *enrty_point = (unsigned int)elf_getEntryPoint(&elf_file);
    return 0;
    //printf("test rd %08x :%08x\n",p_entyp_point, *p_entyp_point);
    //vTaskDelete(NULL);
/*
    if(p_entyp_point != NULL){
        int return_value;
        return_value = 
        (*((int (*)())(p_entyp_point)))();
        printf("exec return :%d\n",return_value);
    }
*/

}



PID_t get_pid_from_task_handle(TaskHandle_t task_handle)
{
    processes_info *current_process_info;
    threads_info *current_thread_info;
    current_process_info = processes_table;
    if(current_process_info == NULL){
        return -1;
    }
    do{
        if(current_process_info->main_thread_task_handle == task_handle){
            return current_process_info->PID;
        }
        if(current_process_info->first_thread_info != NULL){
            current_thread_info = current_process_info->first_thread_info;
            do{
                if(current_thread_info->task_handle == task_handle){
                    return current_process_info->PID;
                }
                current_thread_info = current_thread_info->next_info;
            }while(current_thread_info != NULL);
        }
        current_process_info = current_process_info->next_info;
    }while(current_process_info != NULL);
    return -1;
}

 


unsigned int U8RAND = 1;
volatile unsigned char
get_u8_rand (unsigned int *seed)
{
  unsigned int next = *seed;
  int result;

  next *= 1103515245;
  next += 12345;
  result = (unsigned int) (next / 65536) % 2048;

  next *= 1103515245;
  next += 12345;
  result <<= 10;
  result ^= (unsigned int) (next / 65536) % 1024;

  next *= 1103515245;
  next += 12345;
  result <<= 10;
  result ^= (unsigned int) (next / 65536) % 1024;

  *seed = next;

  return result & 0xFF;
}




int create_process(char *process_name, char *image_path){
    vTaskSuspendAll();
    int status;
    processes_info *current_process_info;
    processes_info *prev_process_info = NULL;

    if(access(image_path,F_OK) != F_OK){
        xTaskResumeAll();
        return -1;
    }

    if(processes_table == NULL){
        processes_table = pvPortMalloc(sizeof(processes_info));
        if(processes_table == NULL){
            xTaskResumeAll();
            return -ENOMEM;
        }
        current_process_info = processes_table;
        goto setting;
    }

    current_process_info = processes_table;
    while(current_process_info->next_info != NULL){
        current_process_info = current_process_info->next_info;
    }
    current_process_info->next_info = pvPortMalloc(sizeof(processes_info));
    if(current_process_info->next_info == NULL){
            xTaskResumeAll();
            return -ENOMEM;
    }
    prev_process_info = current_process_info;
    current_process_info = current_process_info->next_info;

    setting:
    current_process_info->PID = pid_count;
    current_process_info->next_info = NULL;
    current_process_info->first_thread_info = NULL;
    memcpy(current_process_info->process_name, process_name, configMAX_TASK_NAME_LEN);
    current_process_info->process_name[configMAX_TASK_NAME_LEN-1] = '\0';
    set_current_running_pid(pid_count);
    if(create_vm_space(pid_count) == NULL){
        if(prev_process_info != NULL){
            prev_process_info->next_info = NULL;
        }
        //vPortFree(current_process_info);
        xTaskResumeAll();
        return -ENOMEM;
    }
    unsigned int entry_point;
    
    status = vm_load_exec_elf(pid_count, image_path, &entry_point);
    if(status < 0){
        if(prev_process_info != NULL){
            prev_process_info->next_info = NULL;
        }
        vPortFree(current_process_info);
        xTaskResumeAll();
        return status;
    }
    //*((unsigned int *)0x300000) = 0x1234;

    //printf("task test.\n");
    void task_entry_point(unsigned int *entry_point);
    //printf("entry_point:%08x,%08x\n",entry_point,*((unsigned int *)entry_point));
    //xTaskResumeAll();
    //return 1;
    /*
    int testfile;
    //testfile = open("/test.elf",O_RDWR);
    testfile = open("/testd", O_RDWR);
    unsigned char *pstart = (unsigned char *) 0x6123456;
    if(testfile > 0){
        status = create_new_task_space_zone(get_current_running_task_pid(),
                            testfile,
                            10,
                            "TSTF",
                            ZONE_ATTR_R | ZONE_ATTR_W,
                            (unsigned int )pstart,
                            ((unsigned int )pstart) + 512*1024 );

        printf("mmap:%d\n",status);

        //dump_vm_spaces();
        volatile unsigned char wrnum = 0;
        volatile unsigned char *p = pstart;
        if(status == 0){
            
            printf("prev WR.\n");
            U8RAND = 1;*/
            /*
            for(int i=0;i<10;i++){
                get_u8_rand(&U8RAND);
            }*/
            /*
            for(p = pstart;p < (pstart + 512 * 1024); p++){
                if(((unsigned int)p % 512) == 0){
                    //printf("((unsigned int)p %% 512):%08X, p:%08x\n",((unsigned int)p % 512),p);
                    //*p = get_u8_rand(&U8RAND);
                    *p = wrnum++;
                    ++wrnum;
                }else{
                    ++wrnum;
                    //get_u8_rand(&U8RAND);
                }                
            }*/
/*
            p = pstart;
            //printf("test rd:%02x\n",*((unsigned char *)pstart));
            p+=4;
            *p = 0x00;
            

            __asm__ volatile ("":::"memory");
            
            printf("prev RD\n");
            
            U8RAND = 1; 
            for(int i=0;i<10;i++){
                get_u8_rand(&U8RAND);
            }
            wrnum = 0;
            for(p = pstart; p< (pstart + 512 * 1024); p++){
                volatile unsigned char test_val;
                test_val = (get_u8_rand(&U8RAND))&0xFF;
                //test_val = wrnum++;
                //test_val = p;
                //printf("%02X == %02X \n",test_val,(*p & 0xFF) );
                if(test_val != ((*p)&0xFF) ){
                    printf("check fail at:%x, RB:%02x, TEST:%02x\n",p, (*p)  ,(test_val));
                }
            }
            printf("test finish.\n");


        }
    }*/


    //dump_vm_spaces();
    /**pd = 0x186E0;
    unsigned int *st = pd;
    for(;st<pd + 10;st++){
        printf("%08x:%08x\n",st,*st);
    }*/
   // xTaskCreateStatic((TaskFunction_t) task_entry_point, process_name, PAGE_SIZE, (void *)entry_point, 2,(StackType_t *) 0x80000,pvPortMalloc(sizeof(StaticTask_t )));
    
    
    unsigned int *new_task_tcb = (unsigned int *)xTaskCreateStatic((TaskFunction_t) entry_point,
                                                                    process_name, PAGE_SIZE, 
                                                                    (void *)0, 
                                                                    2,
                                                                    (StackType_t *) 0x80000,
                                                                    pvPortMalloc(sizeof(StaticTask_t)));
    new_task_tcb[2] &= 0xFFFFFFE0;
    new_task_tcb[2] |= 0x10;

/*
    for(int i=0;i<20;i++)
        printf(" %02X",get_u8_rand(&U8RAND));
    printf("\n");*/
    //task_entry_point(entry_point);

    pid_count++;
    xTaskResumeAll();
  
   // dump_vm_spaces();
      return 0;
}

volatile void swi(register unsigned int regs) __attribute__((naked));
volatile void swi(register unsigned int regs)
{

    __asm__ volatile ("":::"memory");
    __asm volatile("swi #0" :: "r"(regs));
}


volatile unsigned int syscall(unsigned int r0, unsigned int r1, unsigned int r2,
	unsigned int r3, unsigned int r4, unsigned int r5, unsigned int r7)
{
    struct regs{
        unsigned int r0;
        unsigned int r1;
        unsigned int r2;
        unsigned int r3;
        unsigned int r4;
        unsigned int r5;
        unsigned int r7;
    } regs;
    __asm__ volatile ("":::"memory");
    regs.r0 = r0;
    regs.r1 = r1;
    regs.r2 = r2;
    regs.r3 = r3;
    regs.r4 = r4;
    regs.r5 = r5;
    regs.r7 = r7;
    register unsigned int par __asm__("r0");
    par = (unsigned int)&regs;
    __asm__ volatile ("":::"memory");
    swi(par);
    return regs.r0;
}



void task_entry_point(unsigned int *___p){
    unsigned int *entry_point = ___p;
    asm volatile("mrs r1,cpsr");
    asm volatile("bic r1,r1,#0x1F");
    asm volatile("orr r1,r1,#0x10");
    asm volatile("msr cpsr,r1");
    //syscall(200,0,0,0,0,0,SWI_NANOSLEEP);
    
    /*

    */
   //printf("dump 0x26510:%08x\n",*((unsigned int*)0x26510));
   //(*((void (*)())(entry_point)))();
    printf("entry_point:%08x : %08x\n",entry_point,*entry_point);
/*
    float i=0;
    while(1){
        i+=0.1;
		printf("hello world %f.\n",(float)i);
        syscall(1,"testwrite\n",10,0,0,0,SWI_WRITE);
        syscall(200,0,0,0,0,0,SWI_NANOSLEEP);
    }
*/
/*
	int k = 0;
	char p[20];
	while(1){
		sprintf(p,"test:%d\n",k);
		k++;
		printf("hello world:%s\n",p);
		syscall(600,0,0,0,0,0,SWI_NANOSLEEP);
	}*/

    (*((void (*)())(entry_point)))();


    /*
    int *p = 0x26000;
    //dump_vm_spaces();
    for(p = 0x26000; p< 0x26400; p++){
        printf("%08x\n",*p);
        if(p % 4 == 0){
            printf("\n");
        }
    }
    printf("----\n",*p);
    p = 0x26000;
    *p = 0x12345678;
 for(p = 0x26000; p< 0x26400; p++){
        printf("%08x\n",*p);
                if(p % 4 == 0){
            printf("\n");
        }
    }
*/
/*
    uint8_t *p = 0x80000;
    for(p<0x84000;p++){
        *p = p;
    }

    for(p=0x80000;p<0x84000;p++){
        printf("%02X ",*p);
    }
*/
    //syscall(4000,0,0,0,0,0,SWI_NANOSLEEP);
    /*
    vTaskDelay(2000);
    srand(1);
    for(volatile unsigned char *p=0x70000;p<0x90000; p++){
        *p = (rand() & 0xFF);
        //*p = p;
    }
    __asm__ volatile ("":::"memory");
    srand(1);
    for(volatile unsigned char *p=0x70000;p<0x90000; p++){
        volatile unsigned char test_val;
        test_val = (rand() & 0xFF);
        //test_val = p;
        if(test_val != (*p & 0xFF)){
            printf("check fail at:%x, RB:%02x, TEST:%02x\n",p, (*p & 0xFF) ,test_val);
        }
    }
    printf("memchk finish.\n");
*/
    vTaskDelete(NULL);
    while(1);
}
