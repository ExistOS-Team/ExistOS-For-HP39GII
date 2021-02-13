
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

//#include "uart_debug.h"
//#include "swi_system_call.h"

processes_info *processes_table;
int pid_count = 10;
volatile int current_running_pid = -1;

int get_current_running_task_pid(){
    return current_running_pid;
}

void set_current_running_pid(int current_pid)
{
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
    //printf("ELF load.\n");
    
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
    Elf32_Syminfo a;
    status = create_new_task_space_zone(pid,get_page_file_fd(),0,
                            "DEFSTACK",
                            ZONE_ATTR_R | ZONE_ATTR_W,
                            0x70000,
                            0x80000);
    remove_task_space_zone(pid,"ELFIMAGE");

    printf("STACK:%d\n",status);
    //dump_vm_spaces();
    *enrty_point = (unsigned int)elf_getEntryPoint(&elf_file);
    return 0;

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

processes_info* get_process_info(PID_t pid){
    processes_info *current_process_info;
    current_process_info = processes_table;
    if(current_process_info == NULL)
    {
        return NULL;
    }

    do{
        if(current_process_info->PID == pid){
            return current_process_info;
        }
        current_process_info = current_process_info->next_info;
    }while(current_process_info != NULL);
    return NULL;
}

void dump_all_processes_info(){
    processes_info *current_process_info;
    threads_info *current_thread_info;
    current_process_info = processes_table;
    if(current_process_info == NULL)
    {
        return;
    }
    printf("-------------------\n");
    do{
        printf("PID:#%d\n",current_process_info->PID);
        printf("    Name:%s\n",current_process_info->process_name);
        printf("    PageFaultCount:%d\n",current_process_info->seg_fault_count);
        printf("    MainThread:%08x\n",current_process_info->main_thread_task_handle);
        printf("    SBRK:%d\n",current_process_info->sbrk);
        if(current_process_info->first_thread_info != NULL){
            current_thread_info = current_process_info->first_thread_info;
            do{
                printf("        Thread #%d\n",current_thread_info->tid);
                printf("        Thread Task Handle:%08X\n",current_thread_info->task_handle);
                printf("        Thread Status:%d\n\n",current_thread_info->status);
                current_thread_info = current_thread_info->next_info;
            }while(current_thread_info != NULL);
        }
        current_process_info = current_process_info->next_info;
    }while(current_process_info != NULL);
    printf("-------------------\n");
}

#define BASE_HEAP_ADDR       0x10000000

unsigned int* process_sbrk(int pid, intptr_t increment)
{
    processes_info *current_process_info;
    int status;
    unsigned int prev_incr = 0;
    char heap_zone_name[9];
    current_process_info = get_process_info(pid);
    if(current_process_info == NULL){
        return NULL;
    }
    if( current_process_info -> sbrk == 0){
        status = create_new_task_space_zone(
                            pid,
                            get_page_file_fd(),
                            0,
                            "HEAP00",
                            ZONE_ATTR_R | ZONE_ATTR_W,
                            BASE_HEAP_ADDR,
                            BASE_HEAP_ADDR + ((increment + 0x000FFFFF) & 0xFFF00000));
        if(status != 0){
            return 0;
        }

        prev_incr = BASE_HEAP_ADDR;
        current_process_info -> sbrk = prev_incr;
        current_process_info -> sbrk += increment;
        current_process_info -> last_sbrk_seg_end = BASE_HEAP_ADDR + ((increment + 0x000FFFFF) & 0xFFF00000);
        return (unsigned int *)prev_incr;
    }

    prev_incr = current_process_info -> sbrk;
    current_process_info -> sbrk += increment;

    if(current_process_info -> sbrk < current_process_info -> last_sbrk_seg_end){
        return (unsigned int *)prev_incr;
    }else{
        sprintf(heap_zone_name,"HEAP%02X",(current_process_info -> last_sbrk_seg_end & 0x0FF00000) >> 4*5);
        status = create_new_task_space_zone(
                            pid,
                            get_page_file_fd(),
                            0,
                            heap_zone_name,
                            ZONE_ATTR_R | ZONE_ATTR_W,
                            current_process_info -> last_sbrk_seg_end,
                            current_process_info -> last_sbrk_seg_end + ((increment + 0x000FFFFF) & 0xFFF00000));
        current_process_info -> last_sbrk_seg_end = current_process_info -> last_sbrk_seg_end + ((increment + 0x000FFFFF) & 0xFFF00000);
        if(status != 0){
            return 0;
        }
        return (unsigned int *)prev_incr;                     
    }
    
    return (unsigned int *)prev_incr;
}

TaskHandle_t create_thread(PID_t pid, void *func, unsigned int *stack_addr, char *thread_name ){
    vTaskSuspendAll();
    processes_info *current_process_info;
    threads_info *current_thread_info;

    current_process_info = get_process_info(pid);
    if(current_process_info == NULL){
        xTaskResumeAll();
        return NULL;
    }
    
    unsigned int *new_task_tcb = pvPortMalloc(sizeof(StaticTask_t));
    if(new_task_tcb == NULL){
        xTaskResumeAll();
        return NULL;
    }
    int tid_count = 1;
    current_thread_info = current_process_info->first_thread_info;
    
    if(current_thread_info == NULL){
        current_process_info->first_thread_info = pvPortMalloc(sizeof(threads_info));
        if(current_process_info->first_thread_info == NULL){
            vPortFree(new_task_tcb);
            xTaskResumeAll();
            return NULL;
        }
        current_thread_info = current_process_info->first_thread_info;
        current_thread_info->tid = 0;
    }else{
        while(current_thread_info->next_info != NULL){
            current_thread_info = current_thread_info->next_info;
            tid_count++;
        }
        current_thread_info->next_info = pvPortMalloc(sizeof(threads_info));
        if(current_thread_info->next_info == NULL){
            vPortFree(new_task_tcb);
            xTaskResumeAll();
            return NULL;
        }
        current_thread_info = current_thread_info->next_info;
        current_thread_info->tid = tid_count;
    }
    
    current_thread_info->next_info = NULL;
    current_thread_info->belong_PID = pid;
    current_thread_info->task_handle = (TaskHandle_t)new_task_tcb;
    current_thread_info->status = STATUS_RUNNING;
    xTaskCreateStatic((TaskFunction_t) func,
                                    thread_name, 
                                    PAGE_SIZE, 
                                    (void *)0, 
                                    2,
                                    (StackType_t *)stack_addr,
                                    (StaticTask_t *)new_task_tcb);
    new_task_tcb[2] &= 0xFFFFFFE0;
    new_task_tcb[2] |= 0x10;
    xTaskResumeAll();
    return new_task_tcb;
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
    current_process_info->sbrk = 0;
    current_process_info->last_sbrk_seg_end = 0;
    current_process_info->seg_fault_count = 0;
    current_process_info->next_info = NULL;
    current_process_info->first_thread_info = NULL;
    memcpy(current_process_info->process_name, process_name, configMAX_TASK_NAME_LEN);
    current_process_info->process_name[configMAX_TASK_NAME_LEN-1] = '\0';
    set_current_running_pid(pid_count);
    if(create_vm_space(pid_count) == NULL){
        if(prev_process_info != NULL){
            prev_process_info->next_info = NULL;
        }
        vPortFree(current_process_info);
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

    current_process_info -> main_thread_task_handle = create_thread(pid_count, ( void *) entry_point, 0x80000, "main_thread");
    if( current_process_info -> main_thread_task_handle == NULL )
    {
        if(prev_process_info != NULL){
            prev_process_info->next_info = NULL;
        }
        vPortFree(current_process_info);
        xTaskResumeAll();
        return -ENOMEM;
    }
 
    pid_count++;


    dump_all_processes_info();

    xTaskResumeAll(); 
    return 0;
}


/*
void task_entry_point(unsigned int *___p){
    unsigned int *entry_point = ___p;
    asm volatile("mrs r1,cpsr");
    asm volatile("bic r1,r1,#0x1F");
    asm volatile("orr r1,r1,#0x10");
    asm volatile("msr cpsr,r1");

    printf("entry_point:%08x : %08x\n",entry_point,*entry_point);

    (*((void (*)())(entry_point)))();

    vTaskDelete(NULL);
    while(1);
}
*/