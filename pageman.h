
#ifndef MMEMOTY_H
#define MMEMOTY_H

#include "FreeRTOS.h"
#include "task.h"

#include "ff.h"

#define MAX_L1_PTE_IN_USE   4
#define L2_TABLE_NUMBER     8

#define ZONE_ATTR_R     0b0001
#define ZONE_ATTR_W     0b0010
#define ZONE_ATTR_X     0b0100

typedef struct{
    unsigned int virt_seg;
    unsigned int *phy_addr;
    unsigned int LRU_count;
}L1_PTE_info;

typedef struct{
    unsigned int virt_seg_inner_page_offset;
    unsigned int *map_phy_addr;
}L2_PET_info;

typedef struct{
    unsigned int *table_page_phy_addr;
    unsigned int map_for_virt_seg;
    unsigned int LRU_count;
}L2_PAGE_TABLE_info;

typedef struct{
    unsigned int *map_on_virt_addr;
    unsigned int *page_phy_addr;
    unsigned int LRU_count;
    TaskHandle_t belong_task;
    int map_file_fd;
    unsigned int page_attr;
    unsigned int offset_page_in_file;
    unsigned int is_dirty;
}page_info;

typedef struct vmfile_map_info{
    struct vmfile_map_info *next_vmf_map_info;
    unsigned int virt_map_addr;
    unsigned int page_in_vm_file_page_offset;
}vmfile_map_info;

typedef struct zone_info{
    struct zone_info *next_info;
    unsigned char zone_attr;
    char zone_name[9];
    unsigned int* zone_start_addr;
    unsigned int* zone_stop_addr;
    int map_file_fd;
    unsigned int file_inner_page_start;
    vmfile_map_info *first_map_info;
}zone_info;

typedef struct vm_space{
    struct vm_space *next_vm_space;
    TaskHandle_t task_handle;
    zone_info* first_zone_info;
}vm_space;

int get_page_file_fd();

int pageman_init(unsigned int cache_page_number,unsigned int vm_file_size_m);
vm_space *create_vm_space(TaskHandle_t task_handle);
vm_space *get_task_vm_space(TaskHandle_t task_handle);
int create_new_task_space_zone(TaskHandle_t task_handle,int file, unsigned int file_inner_page_start, 
        char *zone_name, unsigned char zone_attr, unsigned int *start_addr, unsigned int *end_addr);
void dump_vm_spaces();

int data_access_fault_isr(TaskHandle_t task_handle, unsigned int *access_fault_addr, unsigned int *fault_ins_addr);

void sync_all_write_back_cache_page();

#endif 