
#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>
#include <stdio.h>

#include "elf_user.h"

#include "ProcessesMan.h"
#include "pageman.h"
#include "memory_map.h"
#include "mmu.h"

#include "uart_debug.h"

processes_info *processes_table;
PID_t pid_count = 10;
PID_t current_running_pid = -1;

PID_t get_current_running_task_pid(){
    return current_running_pid;
}

void set_current_running_pid(PID_t pid){
    current_running_pid = pid;
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
                status = create_new_task_space_zone(pid,fd,elf_getProgramHeaderOffset(&elf_file,i) / PAGE_SIZE ,
                            name_buffer,
                            ZONE_ATTR_R | ZONE_ATTR_W,
                            elf_getProgramHeaderVaddr(&elf_file,i),
                            elf_getProgramHeaderVaddr(&elf_file,i) + elf_getProgramHeaderMemorySize(&elf_file,i));
            }
            if( (current_flag & PF_R) && (current_flag & PF_X)){
                printf("Load Seg #%d = TEXT\n",i);
                sprintf(name_buffer,"TEXT%d",TEXT_count);
                TEXT_count++;
                status = create_new_task_space_zone(pid,fd,elf_getProgramHeaderOffset(&elf_file,i) / PAGE_SIZE ,
                            name_buffer,
                            ZONE_ATTR_R | ZONE_ATTR_X,
                            elf_getProgramHeaderVaddr(&elf_file,i),
                            elf_getProgramHeaderVaddr(&elf_file,i) + elf_getProgramHeaderMemorySize(&elf_file,i));
            }
            if( (current_flag & PF_R) && !(current_flag & PF_X) && !(current_flag & PF_W)){
                printf("Load Seg #%d = RODATA\n",i);
                sprintf(name_buffer,"RODATA%d",RODATA_count);
                RODATA_count++;
                status = create_new_task_space_zone(pid,fd,elf_getProgramHeaderOffset(&elf_file,i) / PAGE_SIZE ,
                            name_buffer,
                            ZONE_ATTR_R,
                            elf_getProgramHeaderVaddr(&elf_file,i),
                            elf_getProgramHeaderVaddr(&elf_file,i) + elf_getProgramHeaderMemorySize(&elf_file,i));
            }

            printf("load status:%d\n",status);
        }
    }

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


void task_entry_point(unsigned int *entry_point){
    
    (*((int (*)())(entry_point)))();
    while(1);
}

int create_process(char *process_name, char *image_path){
    vTaskSuspendAll();
    int status;
    processes_info *current_process_info;

    if(processes_table == NULL){
        processes_table = pvPortMalloc(sizeof(processes_info));
        if(processes_table == NULL){
            return -1;
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
            return -1;
    }
    current_process_info = current_process_info->next_info;

    setting:
    current_process_info->PID = pid_count;
    current_process_info->next_info = NULL;
    current_process_info->first_thread_info = NULL;
    memcpy(current_process_info->process_name, process_name, configMAX_TASK_NAME_LEN);
    current_process_info->process_name[configMAX_TASK_NAME_LEN-1] = '\0';

    if(create_vm_space(pid_count) == NULL){
        return -1;
    }
    unsigned int entry_point;
    set_current_running_pid(pid_count);
    status = vm_load_exec_elf(pid_count, image_path, &entry_point);
    if(status < 0){
        return status;
    }
    
    //uartdbg_print_regs();
    /*
    for(char *i=0x80000; i<0x8F000;i++){
        *i = (char)i & 0xFF;
    }

    for(char *i=0x80000; i<0x8F000;i++){
        printf("%02X ",*i);
    }
*/
    
    //memset(((int *)0x7FF00),0,0x200);
    //volatile register unsigned int r0 asm("r0"); 
    //asm volatile("ldr r1,%0" : "=m"(entry_point) );
    //asm volatile("ldr r0,[r1]");
    //printf("r0:%08x\n",r0);

    //disable_interrupts();

   // asm volatile("push {pc}");
    //asm volatile("mov pc,r0");
    //(*((int (*)())(entry_point)))();

    *((unsigned int *)0xC0072200) = (unsigned int *)&uartdbg_print_regs;
    //pvPortMalloc(sizeof(0x200 * 4))
    printf("entry_point:%08x\n",entry_point);
    //xTaskCreateStatic((TaskFunction_t)entry_point, process_name, 0x200, NULL, 0,0x80000,(StaticTask_t *)pvPortMalloc(sizeof(StaticTask_t)) );
    
    
    //memset(0x80000,0,0x400);

    xTaskCreateStatic((TaskFunction_t)task_entry_point, process_name, PAGE_SIZE, entry_point, 0,(StackType_t *) 0x80000,pvPortMalloc(sizeof(StaticTask_t )));
    
    //uartdbg_print_regs();
    

    pid_count++;
    xTaskResumeAll();
    //dump_vm_spaces();
}