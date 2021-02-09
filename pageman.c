#include <regsdigctl.h>

#include <stdio.h>


#include "vfsman.h"
#include "pageman.h"
#include "memory_map.h"
#include "FreeRTOS.h"
#include "task.h"

#include "uart_debug.h"

#include "mmu.h"
#include "ServiceFlashMap.h"

#include <fcntl.h>
#include <unistd.h>

#include <malloc.h>



#define DIFF_PHY_VS_VIRT    (RAM_START_VIRT_ADDR)

L1_PTE_info L1_PTE[MAX_L1_PTE_IN_USE];
page_info *pages_info;
L2_PAGE_TABLE_info *l2_table_info;

int MAX_CACHE_PAGES;

int vm_page_file_fd = -1;


unsigned char *vm_file_bitmap;
vm_space *vm_space_info = NULL;
unsigned int vm_file_size = 0;

//FIL vm_page_file_handle;

int get_page_file_fd(){
    return vm_page_file_fd;
}

void reload_L1_pte()
{
    volatile unsigned int *l1_tab_entry = (unsigned int *)0x800C0000;
    for(int i=0; i<MAX_L1_PTE_IN_USE; i++){
        BF_CS1n(DIGCTL_MPTEn_LOC, i, LOC, L1_PTE[i].virt_seg);
        l1_tab_entry[ L1_PTE[i].virt_seg ] = ((unsigned int)L1_PTE[i].phy_addr & 0xFFFFFC00) | 0x11;
    }
    flush_tlb();
}

void invalid_all_L1_PTE(){
    volatile unsigned int *l1_tab_entry = (unsigned int *)0x800C0000;
    for(int i=0; i<MAX_L1_PTE_IN_USE; i++){
        BF_CS1n(DIGCTL_MPTEn_LOC, i, LOC, i);
        L1_PTE[i].virt_seg = i;
        l1_tab_entry[ i ] = 0;
    }
    flush_tlb();    
}

void set_l2_table_PTE(unsigned int which_L2_table,L2_PET_info *pet_info, unsigned int enable_W){
    volatile unsigned int *l2_tab_base;
    unsigned int index;
    unsigned int APLIST = 0b11111111;
    
    l2_tab_base = l2_table_info[which_L2_table].table_page_phy_addr;
    l2_tab_base += DIFF_PHY_VS_VIRT/4;

    index = pet_info->virt_seg_inner_page_offset;
    l2_tab_base[index] = ((unsigned int)pet_info->map_phy_addr) & 0xFFFFF000;
    l2_tab_base[index] |= 0b1110;

    if(enable_W){
        l2_tab_base[index] |= (APLIST << 4);
    }
    
    //l2_tab_base[index] |= 0b111111111110;
}

void set_l1_table_PTE(int which_L1_PTE, unsigned int which_L2_table){
    volatile unsigned int *l2_tab_base;
    volatile unsigned int *l1_tab_entry = (unsigned int *)0x800C0000;
    int domain = 1;
    
    l2_tab_base = l2_table_info[which_L2_table].table_page_phy_addr;
    /*
    printf("\nwhich_L1_PTE:%d, which_L2_table:%d, (unsigned int)l2_tab_base >> 20:%08x, L1_PTE[which_L1_PTE].virt_seg:%08x\n",
        which_L1_PTE,
        which_L2_table,
        (unsigned int)l2_tab_base >> 20, 
        L1_PTE[which_L1_PTE].virt_seg);*/
    //HW_DIGCTL_MPTEn_LOC(which_L1_PTE).B.LOC = (unsigned int)l2_tab_base >> 20;
    //dump_MPTE();
    BF_CS1n(DIGCTL_MPTEn_LOC, which_L1_PTE, LOC, L1_PTE[which_L1_PTE].virt_seg);

    //l1_tab_entry[ L1_PTE[which_L1_PTE].virt_seg ] = 0;

    

    l1_tab_entry[ L1_PTE[which_L1_PTE].virt_seg ]   =  (unsigned int)l2_table_info[which_L2_table].table_page_phy_addr & 0xFFFFFC00;

    

    l1_tab_entry[ L1_PTE[which_L1_PTE].virt_seg ] |= (domain << 5);
    
    l1_tab_entry[ L1_PTE[which_L1_PTE].virt_seg ] |= 0x01;
    //printf("RB L1_PTE:%x\n",l1_tab_entry[ L1_PTE[which_L1_PTE].virt_seg ]);
    //dump_MPTE();
    //dump_page_tables();
    

}

int pageman_init(unsigned int cache_page_number,unsigned int vm_file_size_m)
{

    vm_file_bitmap = (unsigned char *)pvPortMalloc(vm_file_size_m * 32);
    if(vm_file_bitmap == NULL){
        return -1;
    }

    pages_info = pvPortMalloc(sizeof(page_info)*cache_page_number);
    if(pages_info == NULL){
        return -1;
    }
    
    l2_table_info = pvPortMalloc(sizeof(L2_PAGE_TABLE_info)*L2_TABLE_NUMBER);
    if(l2_table_info == NULL){
        return -1;
    }


    MAX_CACHE_PAGES = cache_page_number;
    memset(vm_file_bitmap,0,vm_file_size_m * 32);
    vm_file_size = vm_file_size_m;
/*
    FRESULT fr = f_open(&vm_page_file_handle, "Pagefile", FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
    if (fr != 0) {
        printf("Create pagefile failed, %d\r\n", fr);
        //vPortFree(page_cache_start_addr);
        vPortFree(vm_file_bitmap);
        return -2;
    }
    fr = f_expand(&vm_page_file_handle, vm_file_size * 1024 * 1024, 1);
    if (fr != 0) {
        printf("Allocates %d MB pagefile failed, %d\r\n", vm_file_size, fr);
        //vPortFree(page_cache_start_addr);
        vPortFree(vm_file_bitmap);
        return -2;
    }
 */
    vm_page_file_fd = open("/PAGEFILE",O_CREAT | O_RDWR | O_TRUNC);
    if(vm_page_file_fd < 0){
        vPortFree(vm_file_bitmap);
        vPortFree(l2_table_info);
        vPortFree(pages_info);
        return -2;
    }

    /*
    if(vfs_lseek(vm_page_file_fd, vm_file_size * 1024 * 1024, SEEK_SET) < 0){
        printf("Allocates %d MB pagefile failed. \r\n", vm_file_size);
        vPortFree(vm_file_bitmap);
        vPortFree(l2_table_info);
        vPortFree(pages_info);
        close(vm_page_file_fd);
        return -3;
    }
    */

    //f_sync(&vm_page_file_handle);
    
    
    
    flashSyncNow();

    for(int i=0; i<MAX_L1_PTE_IN_USE; i++){
        L1_PTE[i].LRU_count = 9999;
        L1_PTE[i].phy_addr = (unsigned int *)0x100000;
        L1_PTE[i].virt_seg = 4000 + i;
    }

    for(int i=0; i<L2_TABLE_NUMBER; i++){
        //l2_table_info[i].in_use = 0;
        l2_table_info[i].LRU_count = 9999;
        //l2_table_info[i].table_page_phy_addr = l2_table_start_addr + i * PAGE_TABLE_SIZE / 4;
        l2_table_info[i].table_page_phy_addr = (unsigned int *)((unsigned int)memalign(PAGE_TABLE_SIZE,PAGE_TABLE_SIZE) - DIFF_PHY_VS_VIRT);
        if(l2_table_info[i].table_page_phy_addr == NULL){
            return -1;
        }
        l2_table_info[i].map_for_virt_seg = UNUSE;
        printf("L2_PT_ADDR:%08x\n",l2_table_info[i].table_page_phy_addr );
    }

    for(int i = 0 ;i<MAX_CACHE_PAGES; i++){
        //pages_info[i].page_phy_addr = page_cache_start_addr + i * PAGE_SIZE / 4;
        pages_info[i].page_phy_addr = (unsigned int *)((unsigned int)memalign(PAGE_SIZE,PAGE_SIZE) - DIFF_PHY_VS_VIRT);
        if(pages_info[i].page_phy_addr == NULL){
            return -1;
        }
        pages_info[i].LRU_count = 9999;
        pages_info[i].belong_task = 0;
        pages_info[i].map_file_fd = -1;
        pages_info[i].file_offset_addr = 0;
        pages_info[i].map_on_virt_addr = UNUSE;
        pages_info[i].is_dirty = 0;
        pages_info[i].page_attr = 0;
        printf("PAGE_CACHE_ADDR:%08x\n",pages_info[i].page_phy_addr );
    }

    mmu_set_RS(0b10);   //当AP = 00只读，AP =11 读写
    mmu_set_domain_control_bit(0, 0b11);    //domain 0内的所有页任意访问
    mmu_set_domain_control_bit(1, 0b01);    //domain 1内的所有页都需进行权限检查

    void dump_MPTE();
    dump_MPTE();

    invalid_all_L1_PTE();

    return 0;

}

void dump_vm_page_file_bitmap(){
    unsigned char c;
    printf("---------------dump_vm_page_file_bitmap------------------\n");
    for(int i = 0; i < (vm_file_size * 1048576 / PAGE_SIZE / (sizeof(unsigned char)*8)); i++){
        c = vm_file_bitmap[i];
        printf("%02X ",c);
    }
    printf("\n\n\n");
}

int vm_page_alloc(){
    unsigned char c;
    for(int i = 0; i < (vm_file_size * 1048576 / PAGE_SIZE / (sizeof(unsigned char)*8)); i++){
        c = vm_file_bitmap[i];
        for(int j = 0; j < (sizeof(unsigned char)*8) ;j++){	
			if((c & 1) == 0){
                vm_file_bitmap[i] = vm_file_bitmap[i] | (1 << j);
                return ((i * ((sizeof(unsigned char)*8)) + j));
            }
            c = c >> 1;
		}
    }
	return -1;
}

void vm_page_free(unsigned int vm_page_order){
    unsigned char c = vm_file_bitmap[  (vm_page_order )/(sizeof(unsigned char)*8)  ];
    c = c & ~(1 <<  (vm_page_order )%(sizeof(unsigned char)*8) );
    vm_file_bitmap[  (vm_page_order)/(sizeof(unsigned char)*8)  ] = c;
}

int get_max_lru_count_L1_PTE(){
    int max = 0;
    int max_item = 0;
    for(int i=0; i<MAX_L1_PTE_IN_USE; i++){
        if(L1_PTE[i].LRU_count > max){
            max = L1_PTE[i].LRU_count;
            max_item = i;
        }
    }
    return max_item;
}

vm_space *create_vm_space(PID_t task_handle){
    vm_space* current_vm_space_info = vm_space_info;
    if(vm_space_info == NULL){
        vm_space_info = (vm_space *)pvPortMalloc(sizeof(vm_space));
        if(vm_space_info == NULL){
            return NULL;
        }
        vm_space_info->next_vm_space = NULL;
        vm_space_info->task_handle = task_handle;
        vm_space_info->first_zone_info = NULL;
        return vm_space_info;
    }
    
    while(current_vm_space_info->next_vm_space != NULL){
        if(current_vm_space_info->task_handle == task_handle){
            return NULL;
        }
        current_vm_space_info = current_vm_space_info->next_vm_space;
    }

    current_vm_space_info->next_vm_space = (vm_space *)pvPortMalloc(sizeof(vm_space));
    if(current_vm_space_info->next_vm_space == NULL)
        return NULL;
    current_vm_space_info = current_vm_space_info->next_vm_space;
    current_vm_space_info->task_handle = task_handle;
    current_vm_space_info->first_zone_info=NULL;
    current_vm_space_info->next_vm_space=NULL;
    return current_vm_space_info;

}

vm_space *get_task_vm_space(PID_t task_handle)
{
    vm_space* current_vm_space_info = vm_space_info;
    if(current_vm_space_info == NULL){
        return NULL;
    }
    do{
        if(current_vm_space_info->task_handle == task_handle){
            return current_vm_space_info;
        }
        current_vm_space_info=current_vm_space_info->next_vm_space;
    }while(current_vm_space_info != NULL);
    return NULL;
}

void dump_vm_spaces(){
    void dump_page_tables();
    dump_page_tables();

    vm_space* current_vm_space_info = vm_space_info;
    int i=0;
    if(current_vm_space_info == NULL)return;
    printf("-------------------------------------------\n");
    do{
        printf("vm space:\t%d# task handle:\t%08X\n",i, current_vm_space_info->task_handle);
        zone_info *current_zone_info = current_vm_space_info->first_zone_info;
        if(current_zone_info != NULL){
            do{
                printf("\tzone name:%s\n",current_zone_info->zone_name);
                printf("\tzone attr:%02X\n",current_zone_info->zone_attr);
                if(current_zone_info->map_file_fd == vm_page_file_fd){
                    printf("\tzone map file:PAGEFILE\n");
                }else{
                    printf("\tzone map file:%08X\n",current_zone_info->map_file_fd);
                }
                printf("\tzone map addr start:%08X\n",current_zone_info->zone_start_addr);
                printf("\tzone map addr stop:%08X\n",current_zone_info->zone_stop_addr);
                //if(current_zone_info->map_file_fd == vm_page_file_fd){
                    vmfile_map_info *current_vmf_map_info = current_zone_info->first_map_info;
                    if(current_vmf_map_info != NULL){
                        int j=0;
                        do{
                            printf("\t\tmap index: #%d\n",j);
                            printf("\t\tmap_page_in_file_offset_addr:%08X\n",current_vmf_map_info->page_in_file_offset_addr);
                            printf("\t\tvmf_map_addr:%08X\n\n",current_vmf_map_info->virt_map_addr);
                            j++;
                            current_vmf_map_info = current_vmf_map_info->next_vmf_map_info;
                        }while(current_vmf_map_info != NULL);
                    }
                //}
                printf("-------------------------------------------\n");
                current_zone_info = current_zone_info->next_info;
            }while(current_zone_info != NULL);           
        }
        current_vm_space_info = current_vm_space_info->next_vm_space;
    }while(current_vm_space_info != NULL);

}

int remove_task_space_zone(PID_t pid, char *zone_name){
    vm_space *current_vm_space_info;
    zone_info *current_zone_info;
    zone_info *prev_zone_info;
    current_vm_space_info = get_task_vm_space(pid);
    if(current_vm_space_info == NULL){
        return -1;
    }
    if(current_vm_space_info->first_zone_info == NULL){
        return -1;
    }
    current_zone_info = current_vm_space_info->first_zone_info;
    if(strcmp(current_zone_info->zone_name , zone_name) == 0){
        current_vm_space_info->first_zone_info = current_zone_info->next_info;
        vPortFree(current_zone_info);
        return 0;
    }
    do{
        if(strcmp(current_zone_info->zone_name , zone_name) == 0){
            prev_zone_info->next_info = current_zone_info->next_info;
            vPortFree(current_zone_info);
            return 0;
        }
        prev_zone_info = current_zone_info;
        current_zone_info = current_zone_info->next_info;
    }while(current_zone_info != NULL);
    return -1;
}

int create_new_task_space_zone(PID_t task_handle,int file, unsigned int file_offset_addr, 
        char *zone_name, unsigned char zone_attr, unsigned int start_addr, unsigned int end_addr)
{
    vm_space *current_vm_space_info;
    zone_info *current_zone_info;
    current_vm_space_info = get_task_vm_space(task_handle);
    if(current_vm_space_info == NULL){
        return -1;
    }
    if(current_vm_space_info->first_zone_info == NULL){
        current_vm_space_info->first_zone_info = (zone_info *)pvPortMalloc(sizeof(zone_info));
        if(current_vm_space_info->first_zone_info == NULL)
        {
            return -1;
        }
        current_vm_space_info->first_zone_info->next_info = NULL;
        current_vm_space_info->first_zone_info->zone_attr = zone_attr;
        current_vm_space_info->first_zone_info->zone_start_addr = start_addr;
        current_vm_space_info->first_zone_info->zone_stop_addr = end_addr;
        memcpy(current_vm_space_info->first_zone_info->zone_name,zone_name,9);
        current_vm_space_info->first_zone_info->zone_name[8] = '\0';
        current_vm_space_info->first_zone_info->map_file_fd = file;
        current_vm_space_info->first_zone_info->file_offset_addr = file_offset_addr;
        current_vm_space_info->first_zone_info->first_map_info = NULL;

        return 0;
    }


    current_zone_info = current_vm_space_info->first_zone_info;
    int overlap;
    while(current_zone_info->next_info != NULL){
        overlap = 0;
        if(!((start_addr > current_zone_info->zone_start_addr)  &&  (end_addr > current_zone_info->zone_stop_addr))  ){
            overlap++;
            //printf("overlap 1\n");
        }
        if(!((start_addr < current_zone_info->zone_start_addr)  &&  (end_addr < current_zone_info->zone_stop_addr))  ){
            overlap++;
            //printf("overlap 2\n");
        }
        if(overlap >= 2){
            overlap = 0;
            return -2;
        }
        if(strcmp(zone_name,(current_zone_info->zone_name)) == 0){

            return -3;
        }
        current_zone_info = current_zone_info->next_info;
    }
    current_zone_info->next_info = (zone_info *)pvPortMalloc(sizeof(zone_info));
    if(current_zone_info->next_info == NULL){
        return -1;
    }
    current_zone_info = current_zone_info->next_info;
    current_zone_info->next_info = NULL;
    current_zone_info->zone_attr = zone_attr;
    current_zone_info->zone_start_addr = start_addr;
    current_zone_info->zone_stop_addr = end_addr;
    memcpy(current_zone_info->zone_name,zone_name,9);
    current_zone_info->zone_name[8] = '\0';
    current_zone_info->map_file_fd = file;
    current_zone_info->file_offset_addr = file_offset_addr;
    current_zone_info->first_map_info = NULL;
    return 0;
}

void ummap_cache_page(unsigned int which_page){
    unsigned int cache_page_virt_addr = (unsigned int)pages_info[which_page].map_on_virt_addr;
    unsigned int cache_page_phy_addr = (unsigned int)pages_info[which_page].page_phy_addr;
    volatile unsigned int *l1_tab_entry = (unsigned int *)0x800C0000;
    volatile unsigned int *l2_tab_entry;
    
    pages_info[which_page].map_on_virt_addr = UNUSE;

    if((l1_tab_entry[ cache_page_virt_addr >> 20] & 0x3) == 0b01){
        l2_tab_entry = (unsigned int *)(l1_tab_entry[ cache_page_virt_addr >> 20] & 0xFFFFFC00);
        l2_tab_entry += (DIFF_PHY_VS_VIRT / 4);
        if(((unsigned int)l2_tab_entry) >= KHEAP_START_VIRT_ADDR && ((unsigned int)l2_tab_entry) <= (KHEAP_START_VIRT_ADDR + TOTAL_PHY_MEMORY - KHEAP_MAP_PHY_START)){
            //printf("!!!L2:%08x  %08x\n",l2_tab_entry[ (cache_page_virt_addr>>12) & 0xFF] , cache_page_phy_addr);
            if( (l2_tab_entry[ (cache_page_virt_addr>>12) & 0xFF] & 0xFFFFF000 ) == (cache_page_phy_addr & 0xFFFFF000)){
                l2_tab_entry[ (cache_page_virt_addr>>12) & 0xFF] = 0;
            }
        }
    }
}

void sync_all_write_back_cache_page(){
    int fr = 0;
    for(int i = 0; i < MAX_CACHE_PAGES; i++){
        if( (pages_info[i].map_file_fd != vm_page_file_fd) && (pages_info[i].map_file_fd > 0))
        {
            if(pages_info[i].map_file_fd == vm_page_file_fd){
                
            }

           if((pages_info[i].page_attr & ZONE_ATTR_W)){
              if(pages_info[i].is_dirty == 1){
                    fr = vfs_lseek(pages_info[i].map_file_fd, pages_info[i].file_offset_addr, SEEK_SET);
                    fr = write(pages_info[i].map_file_fd, pages_info[i].page_phy_addr + DIFF_PHY_VS_VIRT/4, PAGE_SIZE);
                    pages_info[i].is_dirty = 0;
                }
            }       
        }
        fsync(pages_info[i].map_file_fd);
    }

}



int swap_out_cache_page(unsigned int which_page){
    int fr = 0;
    if(pages_info[which_page].belong_task == 0){
        return 0;
    }

    //printf("swap out:%d\n",which_page);
    //printf("pages map_on_virt_addr:%08X\n",pages_info[which_page].map_on_virt_addr);


    if(pages_info[which_page].map_file_fd == vm_page_file_fd){
        if(pages_info[which_page].is_dirty == 1){
            fr = vfs_lseek(vm_page_file_fd, pages_info[which_page].file_offset_addr, SEEK_SET);
            // printf("vfs_lseek:%d, file_offset_addr:%08x\n",fr, pages_info[which_page].file_offset_addr);
            fr = vfs_write(vm_page_file_fd, pages_info[which_page].page_phy_addr + DIFF_PHY_VS_VIRT/4, PAGE_SIZE);
            //printf("vfs_write:%d\n",fr);
            //fr = f_vfs_lseek(&vm_page_file_handle, pages_info[which_page].offset_page_in_file * PAGE_SIZE);
            //fr = f_write(&vm_page_file_handle, pages_info[which_page].page_phy_addr + DIFF_PHY_VS_VIRT/4,PAGE_SIZE,&br);
            pages_info[which_page].is_dirty = 0;
        }
    }else{
        if((pages_info[which_page].page_attr & ZONE_ATTR_W)){
            if(pages_info[which_page].page_attr & ZONE_ATTR_WRITEBACK){
                if(pages_info[which_page].is_dirty == 1){
                    //printf("swap out:%d\n",pages_info[which_page].map_file_fd);
                    fr = vfs_lseek( pages_info[which_page].map_file_fd,
                                    pages_info[which_page].file_offset_addr,
                                    SEEK_SET);

                    fr = vfs_write( pages_info[which_page].map_file_fd,
                                    pages_info[which_page].page_phy_addr + DIFF_PHY_VS_VIRT/4,
                                    PAGE_SIZE);

                    pages_info[which_page].is_dirty = 0;
                }
            }else{
                //NOT WRITE BACK
                //printf("NOT WRITE BACK TO RO FILE.\n");

            }
        }       
    }


    finish:
    pages_info[which_page].is_dirty = 0;

    pages_info[which_page].belong_task = 0;
    ummap_cache_page(which_page);
    return fr;
}

int swap_in_cache_page( unsigned int which_page,
                        int file,
                        unsigned int zone_start_addr_in_file,
                        unsigned int offset_in_file,
                        unsigned int fault_file_page_idx,
                        int page_diff,
                        unsigned int virt_addr_to_load){
    int fr = 0;
    //printf("test 1\n");
    /*
    if(file == vm_page_file_fd){
        
        //fr = f_vfs_lseek(&vm_page_file_handle, page_offset_in_file * PAGE_SIZE);
        //fr = f_read(&vm_page_file_handle, (unsigned int *)(((unsigned int)pages_info[which_page].page_phy_addr) + DIFF_PHY_VS_VIRT), PAGE_SIZE, &br);
        //printf("readin addr:%08x\n", (((unsigned int)pages_info[which_page].page_phy_addr) + DIFF_PHY_VS_VIRT));

    }else{
        fr = f_vfs_lseek(file_handle, page_offset_in_file * PAGE_SIZE);
        fr = f_read(file_handle, (unsigned int *)(((unsigned int)pages_info[which_page].page_phy_addr) + DIFF_PHY_VS_VIRT), PAGE_SIZE, &br); 
    }
    */
    memset((unsigned int *)(((unsigned int)pages_info[which_page].page_phy_addr) + DIFF_PHY_VS_VIRT),0,PAGE_SIZE);
    if(file == vm_page_file_fd){
        fr = vfs_lseek(file, offset_in_file, SEEK_SET);
       // printf("swap in: seek:%d, offset_in_file:%08x",fr,offset_in_file);
        fr = vfs_read(file, (unsigned int *)(((unsigned int)pages_info[which_page].page_phy_addr) + DIFF_PHY_VS_VIRT), PAGE_SIZE);
        //printf(", swap in addr:%08x, read:%d\n", (unsigned int *)(((unsigned int)pages_info[which_page].page_phy_addr) + DIFF_PHY_VS_VIRT),fr);
        //printf("read in map addr:%08x\n",virt_addr_to_load);
    }else{

       // if((pages_info[which_page].page_attr & ZONE_ATTR_WRITEBACK)){
       //     printf("swap in file:%d\n",file);
        //    printf("page_attr:%d\n",pages_info[which_page].page_attr);
        //printf("swap in file:%d offset_in_file:%08x\n",file,offset_in_file);
        /*
        printf("offset_in_file:0x%X\n",offset_in_file);
        printf("page_diff:0x%x\n",page_diff);
        printf("virt_addr_to_load:0x%x\n",virt_addr_to_load);
        printf("zone_start_addr_in_file:0x%x\n",zone_start_addr_in_file);
        printf("fault_file_page_idx:0x%x\n",fault_file_page_idx);
        */
        if(fault_file_page_idx == 0){
            fr = vfs_lseek(file, zone_start_addr_in_file, SEEK_SET);
            fr = vfs_read(file, 
                        (unsigned int *)(
                            ((unsigned int)pages_info[which_page].page_phy_addr) + page_diff + DIFF_PHY_VS_VIRT
                        ),
                         PAGE_SIZE - page_diff);

        }else{
            fr = vfs_lseek(file, (0xFFF - page_diff) * 1 + zone_start_addr_in_file + 1 + 0x1000 * (fault_file_page_idx - 1), SEEK_SET);
            fr = vfs_read(file, 
                        (unsigned int *)(
                            ((unsigned int)pages_info[which_page].page_phy_addr) + DIFF_PHY_VS_VIRT
                        ),
                         PAGE_SIZE);


        }
/*
        if(((long)offset_in_file - (long)page_diff) < 0){
            fr = vfs_lseek(file, zone_start_addr_in_file, SEEK_SET);
            printf("FR1:%d\n",fr);
            fr = vfs_read(file, 
                        (unsigned int *)(
                            ((unsigned int)pages_info[which_page].page_phy_addr) + page_diff + DIFF_PHY_VS_VIRT
                        ),
                         PAGE_SIZE - page_diff);
            printf("FR2:%d\n",fr);             
        }else{
            fr = vfs_lseek(file, zone_start_addr_in_file + (offset_in_file & 0xFFFFF000), SEEK_SET);
            fr = vfs_read(file, 
                        (unsigned int *)(
                            ((unsigned int)pages_info[which_page].page_phy_addr) + DIFF_PHY_VS_VIRT
                        ),
                         PAGE_SIZE);
        }*/


/*
            fr = vfs_lseek(file, offset_in_file + page_diff, SEEK_SET);
            fr = vfs_read(file, 
                        (unsigned int *)(
                            ((unsigned int)pages_info[which_page].page_phy_addr) + page_diff + DIFF_PHY_VS_VIRT
                        ),
                         PAGE_SIZE - page_diff);

            if(offset_in_file - page_diff > 0){

            }

            fr = vfs_lseek(file, offset_in_file - page_diff, SEEK_SET);
            fr = vfs_read(file, 
                        (unsigned int *)(
                            ((unsigned int)pages_info[which_page].page_phy_addr) + DIFF_PHY_VS_VIRT
                        ),
                        page_diff); 
*/
                         /*
            if( (vfs_tell(file) - page_diff) > 0){
                fr = vfs_lseek(file, offset_in_file - page_diff, SEEK_SET);
                fr = vfs_read(file, 
                        (unsigned int *)(
                            ((unsigned int)pages_info[which_page].page_phy_addr) + DIFF_PHY_VS_VIRT
                        ),
                        page_diff);
                }else{
                    memset(
                     (unsigned int *)(
                            ((unsigned int)pages_info[which_page].page_phy_addr) + DIFF_PHY_VS_VIRT
                        ),
                        0,
                        page_diff
                );
            }*/

        
       // }else if((pages_info[which_page].page_attr & ZONE_ATTR_W)){
           

       // }
    }

    finish:


    //printf("test 2\n");
    pages_info[which_page].is_dirty = 0;
    pages_info[which_page].map_on_virt_addr = (unsigned int *)virt_addr_to_load;
    return fr;
}

int get_optimizing_L1_entry(unsigned int* addr, unsigned int* current_fault_ins_addr){
    int max_lru = 0;
    int max_lru_item = 0;

    for(int i = 0; i<MAX_L1_PTE_IN_USE; i++){
        if(L1_PTE[i].virt_seg == ((unsigned int)addr >> 20)){
            return i;
        }
    }

    for(int i = 0; i<MAX_L1_PTE_IN_USE; i++){
        if(L1_PTE[i].virt_seg != ((unsigned int)current_fault_ins_addr >> 20 ) ){
            max_lru_item = i;
            break;
        }
    }

    for(int i = 0; i<MAX_L1_PTE_IN_USE; i++){
        if((L1_PTE[i].LRU_count > max_lru) && (L1_PTE[i].virt_seg != ((unsigned int)current_fault_ins_addr >> 20 ) )){
            max_lru = L1_PTE[i].LRU_count;
            max_lru_item = i;
        }
    }


    for(int i = 0; i<MAX_L1_PTE_IN_USE; i++){
        L1_PTE[i].LRU_count++;
    }
    L1_PTE[max_lru_item].LRU_count = 0;
    return max_lru_item;
}

int get_optimizing_L2_Table(unsigned int * current_fault_addr ,unsigned int* current_fault_ins_addr){
    int max_lru = 0;
    int max_lru_item = 0;
    //printf("current_fault_addr:%08x\n",current_fault_addr);
    for(int i = 0; i < L2_TABLE_NUMBER; i++)
    {
        if((unsigned int)l2_table_info[i].map_for_virt_seg == ((unsigned int)current_fault_addr) >> 20){
            return i;
        }
    }

    for(int i = 0; i < L2_TABLE_NUMBER; i++){
        if((unsigned int)l2_table_info[i].map_for_virt_seg  != (((unsigned int)current_fault_ins_addr) >> 20)){
            max_lru_item = i;
        }
    }

    for(int i = 0; i< L2_TABLE_NUMBER; i++){
        if((l2_table_info[i].LRU_count > max_lru) && ((unsigned int)l2_table_info[i].map_for_virt_seg != ((unsigned int)current_fault_ins_addr >> 20 ) )){
            max_lru = l2_table_info[i].LRU_count;
            max_lru_item = i;
        }
    }
    
    for(int i = 0; i<L2_TABLE_NUMBER; i++){
        l2_table_info[i].LRU_count++;
    }
    l2_table_info[max_lru_item].LRU_count = 0;   

    memset( (unsigned int *)((unsigned int)l2_table_info[max_lru_item].table_page_phy_addr + DIFF_PHY_VS_VIRT ), 0 ,PAGE_TABLE_SIZE);
    return max_lru_item;


}

int get_optimizing_new_cache_page(unsigned int* current_fault_ins_addr){
    int max_lru = 0;
    int max_lru_item = 0;
    int status = 0;

    for(int i = 0 ;i<MAX_CACHE_PAGES; i++){
        if(pages_info[i].belong_task == 0){
            //printf("belong_task = 0,page #%d\n",i);
            pages_info[i].LRU_count = 0;
            return i;
        }
    }



    for(int i = 0; i<MAX_CACHE_PAGES; i++){
        if(pages_info[i].map_on_virt_addr != current_fault_ins_addr){
            max_lru_item = i;
            break;
        }
    }

    for(int i = 0; i<MAX_CACHE_PAGES; i++){
        if((pages_info[i].LRU_count > max_lru) && (pages_info[i].map_on_virt_addr != current_fault_ins_addr)){
            max_lru = pages_info[i].LRU_count;
            max_lru_item = i;
        }
    }


    for(int i=0; i<MAX_CACHE_PAGES; i++){
        pages_info[i].LRU_count++;
    }
    pages_info[max_lru_item].LRU_count = 0;
    
    status = swap_out_cache_page(max_lru_item);
    //printf("swap_out_cache_page:%d\n",status);
    return max_lru_item;
}

void dump_MPTE(){
    volatile unsigned int *l1_tab_entry = (unsigned int *)0x800C0000;
    printf("-------------dump MPTE table----------------\n");
    for(int i = 0; i<8;i++){
        printf("#%d MPET_LOC: %08x ", i, BF_RDn(DIGCTL_MPTEn_LOC, i, LOC));
        printf("value: %08x \n", l1_tab_entry[BF_RDn(DIGCTL_MPTEn_LOC, i, LOC)]);
    }
    printf("--------------------------------------------\n");
}

void dump_page_tables(){
    dump_MPTE();

    printf("-------------dump page table----------------\n");
    printf("L1 PTE:\n");
    for(int i=0; i< MAX_L1_PTE_IN_USE; i++){
        printf("\t#%d: virt addr:%08x, phy addr:%08x, LRU_COUNT:%d\n",i,L1_PTE[i].virt_seg,L1_PTE[i].phy_addr,L1_PTE[i].LRU_count);
    }
    printf("L2 PT INFO:\n");
    for(int i=0; i< L2_TABLE_NUMBER; i++){
        printf("\t#%d: virt seg:%08x, tab phy addr:%08x, LRU_COUNT:%d\n",i,l2_table_info[i].map_for_virt_seg ,l2_table_info[i].table_page_phy_addr,l2_table_info[i].LRU_count);
    }
    printf("CACHE PAGE INFO:\n");
    for(int i=0; i< MAX_CACHE_PAGES; i++){
        printf("\t#%d\n",i);
        printf("\t belong_task:%08x\n",pages_info[i].belong_task);
        printf("\t map file fd:%08x\n",pages_info[i].map_file_fd);
        printf("\t is_dirty:%d\n",pages_info[i].is_dirty);
        printf("\t LRU_count:%d\n",pages_info[i].LRU_count);
        printf("\t map_on_virt_addr:%08x\n",pages_info[i].map_on_virt_addr);
        printf("\t offset_in_file:%x\n",pages_info[i].file_offset_addr);
        printf("\t page_attr:%08x\n",pages_info[i].page_attr);
        printf("\t page_phy_addr:%08x\n\n",pages_info[i].page_phy_addr);
        
    }

    volatile unsigned int *l1_tab_entry = (unsigned int *)0x800C0000;
    volatile unsigned int *l2_tab_entry;
    printf("-------------dump page content----------------\n");
    for(int i=0; i<4096; i++){
        if((l1_tab_entry[i] & 0x3) != 0){
            
            printf("L1 PTE at:#%d, value = %08x,",i,l1_tab_entry[i]);
            printf(" virt addr:%08X",i << 20);
            if(l1_tab_entry[i] & 0b01){
                l2_tab_entry = (unsigned int *)((unsigned int)l1_tab_entry[i] & 0xFFFFFC00);
                printf(" -> phy addr:%08X (L2 Entry)\n",l2_tab_entry);
                l2_tab_entry += DIFF_PHY_VS_VIRT/4;

                for(int j = 0; j < 256; j++){
                    if(l2_tab_entry[j] & 0b11){
                        printf("\tL2 PTE at:#%d , value=%08x\n",j,l2_tab_entry[j]);
                        printf("\t\t virt addr:%08x -> phy addr:%08x\n" ,(i << 20) + j*4096,(l2_tab_entry[j] & 0xFFFFF000));

                    }
                }




            }else if(l1_tab_entry[i] & 0b10){
                printf(" -> phy addr:%08X (Seg)\n",l1_tab_entry[i] & 0xFFF00000);
            }
            

        }
    }
    

    dump_vm_page_file_bitmap();

    printf("--------------------------------------------\n");
}


int remap_cache_page_as_writeable(unsigned int which_page){
    int got_L1_entry;
    int got_L2_PT;
    L2_PET_info L2_PTE;
    unsigned int cache_page_virt_addr_base;

    if((pages_info[which_page].belong_task == NULL )&& (pages_info[which_page].map_on_virt_addr == UNUSE)){
        return 1;
    }
    got_L1_entry = get_optimizing_L1_entry(pages_info[which_page].map_on_virt_addr, NULL);
    got_L2_PT = get_optimizing_L2_Table(pages_info[which_page].map_on_virt_addr, NULL);

    L1_PTE[got_L1_entry].phy_addr = l2_table_info[got_L2_PT].table_page_phy_addr;
    L1_PTE[got_L1_entry].virt_seg = (unsigned int)pages_info[which_page].map_on_virt_addr >> 20;

    l2_table_info[got_L2_PT].map_for_virt_seg = ((unsigned int)pages_info[which_page].map_on_virt_addr >> 20);
   
    cache_page_virt_addr_base = (unsigned int)pages_info[which_page].map_on_virt_addr;
    cache_page_virt_addr_base = cache_page_virt_addr_base & 0x000FF000;
    cache_page_virt_addr_base = cache_page_virt_addr_base >> 12;

    L2_PTE.virt_seg_inner_page_offset = cache_page_virt_addr_base;
    L2_PTE.map_phy_addr = pages_info[which_page].page_phy_addr;

    set_l2_table_PTE(got_L2_PT, &L2_PTE, 1);
    set_l1_table_PTE(got_L1_entry, got_L2_PT);

    flush_cache();
    flush_tlb();

    return 0;
}


int load_cache_page_to_map_vm_space(unsigned int which_page, unsigned int *current_fault_ins_addr){
    int got_L1_entry;
    int got_L2_PT;
    L2_PET_info L2_PTE;
    unsigned int cache_page_virt_addr_base;

    if((pages_info[which_page].belong_task == NULL )&& (pages_info[which_page].map_on_virt_addr == UNUSE)){
        return 1;
    }
    got_L1_entry = get_optimizing_L1_entry(pages_info[which_page].map_on_virt_addr, current_fault_ins_addr);
    got_L2_PT = get_optimizing_L2_Table(pages_info[which_page].map_on_virt_addr, current_fault_ins_addr);

    

    L1_PTE[got_L1_entry].phy_addr = l2_table_info[got_L2_PT].table_page_phy_addr;
    L1_PTE[got_L1_entry].virt_seg = (unsigned int)pages_info[which_page].map_on_virt_addr >> 20;
    

    l2_table_info[got_L2_PT].map_for_virt_seg = ((unsigned int)pages_info[which_page].map_on_virt_addr >> 20);
   
    cache_page_virt_addr_base = (unsigned int)pages_info[which_page].map_on_virt_addr;
    cache_page_virt_addr_base = cache_page_virt_addr_base & 0x000FF000;
    cache_page_virt_addr_base = cache_page_virt_addr_base >> 12;

    //

    L2_PTE.virt_seg_inner_page_offset = cache_page_virt_addr_base;
    L2_PTE.map_phy_addr = pages_info[which_page].page_phy_addr;

    
/*
    printf("l2_table_info[got_L2_PT].map_for_virt_seg:%08x\n",l2_table_info[got_L2_PT].map_for_virt_seg);
    printf("cache_page_virt_addr_base:%08x\n",cache_page_virt_addr_base);
    printf("got_L1_entry:%d, got_L2_PT:%d\n",got_L1_entry,got_L2_PT);
    printf("reload which_page:%d\n",which_page);
    printf("L1_PTE[got_L1_entry].virt_seg:%d\n",L1_PTE[got_L1_entry].virt_seg);
*/
    set_l2_table_PTE(got_L2_PT, &L2_PTE, 0);
    set_l1_table_PTE(got_L1_entry, got_L2_PT);

    
    //uartdbg_print_regs();

    flush_cache();

    //uartdbg_print_regs();

    flush_tlb();

    //printf("finish load.\n");
    //dump_page_tables();

    return 0;

}

unsigned int fault_count = 0;
int data_access_fault_isr(PID_t task_handle, unsigned int *access_fault_addr, unsigned int *fault_ins_addr, unsigned int FSR)
{
    vm_space *current_vm_space_info;
    zone_info *current_zone_info;
    int status;

    

    current_vm_space_info = get_task_vm_space(task_handle);
    if(current_vm_space_info == NULL){
        return 1;
    }
    current_zone_info = current_vm_space_info -> first_zone_info;
    int page_order;
    if(current_zone_info == NULL){
        return 1;
    }
    fault_count++;

    if(fault_count > 10){
        return 1;
    }
    
    //uartdbg_printf("Fault Process: %x, Data Address: %x, ins address: %x fault_count:%d\n",task_handle, access_fault_addr,fault_ins_addr,fault_count);
    //uartdbg_printf("FSR:%x \n",FSR);

    do{
        if( (access_fault_addr >= current_zone_info->zone_start_addr) && (access_fault_addr <= current_zone_info->zone_stop_addr)){

            
            if(((FSR & 0xF) == 0b1111) || ((FSR & 0xF) == 0b1101)){
               //uartdbg_printf("WR fault FSR:%x, W addr:%x\n",FSR,access_fault_addr);
                //printf("FSR:%08x\n",FSR);

                for(int i=0; i < MAX_CACHE_PAGES; i++){
                    if((unsigned int)pages_info[i].map_on_virt_addr == ((unsigned int)access_fault_addr & 0xFFFFF000)){
                        if( (pages_info[i].page_attr & ZONE_ATTR_W) ){
                            
                            if( !(pages_info[i].page_attr & ZONE_ATTR_WRITEBACK) && (pages_info[i].map_file_fd != vm_page_file_fd) ){
                                vmfile_map_info *current_map_info;
                                vmfile_map_info *prev_map_info;
                                current_map_info = current_zone_info ->first_map_info;


                                if(current_map_info == NULL){
                                    current_zone_info ->first_map_info = pvPortMalloc(sizeof(vmfile_map_info));
                                    if(current_zone_info ->first_map_info == NULL){
                                        printf("memory overflow:1\n");
                                        return 1;
                                    }
                                    current_map_info = current_zone_info ->first_map_info;
                                    current_map_info->virt_map_addr = ((unsigned int)access_fault_addr & 0xFFFFF000);
                                    current_map_info->page_in_file_offset_addr = vm_page_alloc() * PAGE_SIZE;
                                    current_map_info->next_vmf_map_info = NULL;
                                    pages_info[i].map_file_fd = vm_page_file_fd;
                                    pages_info[i].file_offset_addr = current_map_info->page_in_file_offset_addr;
                                    pages_info[i].is_dirty = 1;
                                    remap_cache_page_as_writeable(i);
                                    return 0;
                                }else{
                                        do{
                                            if(current_map_info->virt_map_addr == ((unsigned int)access_fault_addr & 0xFFFFF000)){
                                                pages_info[i].is_dirty = 1;
                                                return 0;
                                            }
                                            prev_map_info = current_map_info;
                                            current_map_info = current_map_info->next_vmf_map_info;
                                        }while(current_map_info != NULL);

                                        prev_map_info -> next_vmf_map_info = pvPortMalloc(sizeof(vmfile_map_info));
                                        if(prev_map_info -> next_vmf_map_info == NULL){
                                            printf("memory overflow:1\n");
                                            return 1;
                                        }
                                        current_map_info = prev_map_info -> next_vmf_map_info;
                                        current_map_info->next_vmf_map_info = NULL;
                                        current_map_info->page_in_file_offset_addr = vm_page_alloc() * PAGE_SIZE;
                                        current_map_info->virt_map_addr = ((unsigned int)access_fault_addr & 0xFFFFF000);
                                        pages_info[i].map_file_fd = vm_page_file_fd;
                                        pages_info[i].file_offset_addr = current_map_info->page_in_file_offset_addr;
                                        pages_info[i].is_dirty = 1;
                                        remap_cache_page_as_writeable(i);
                                        return 0;
                                }

                                
                            }





                            pages_info[i].is_dirty = 1;
                            remap_cache_page_as_writeable(i);
                            return 0;
                        }

                            pages_info[i].is_dirty = 1;
                            remap_cache_page_as_writeable(i);
                            return 0;

                    }
                }

            }
    
            
            //printf("Fault Process 1: %08x, Address: %08x\n",task_handle, access_fault_addr);
            if(current_zone_info -> map_file_fd == vm_page_file_fd){

                vmfile_map_info *current_map_info;
                vmfile_map_info *prev_map_info;
                current_map_info = current_zone_info ->first_map_info;
                if(current_map_info != NULL){
                     //current_map_info NOT EQU NULL
                    int is_found_fault_page = 0;
                    do{
                        if((current_map_info->virt_map_addr) == ((unsigned int)access_fault_addr & 0xFFFFF000 )){
                            //printf("Found in vm page file\n");
                            page_order = get_optimizing_new_cache_page(fault_ins_addr);
                            status = swap_in_cache_page(page_order, 
                                                        vm_page_file_fd,
                                                        0,
                                                        current_map_info->page_in_file_offset_addr,
                                                        0,
                                                        0,
                                                        (unsigned int)access_fault_addr & 0xFFFFF000);

                            if(status < 0){
                                printf("load page file fault!\n");
                                return 1;
                            }
                            pages_info[page_order].belong_task = task_handle;
                            pages_info[page_order].map_file_fd = vm_page_file_fd;
                            pages_info[page_order].file_offset_addr = current_map_info->page_in_file_offset_addr;
                            pages_info[page_order].page_attr = current_zone_info->zone_attr;
                            is_found_fault_page = 1;
                            //printf("is_found_fault_page:%d\n",is_found_fault_page);
                            //finish setting, reload all cache pages.
                            load_cache_page_to_map_vm_space(page_order, fault_ins_addr);
                            return 0;

                            break;
                        }
                        prev_map_info = current_map_info;
                        current_map_info = current_map_info->next_vmf_map_info;
                    }while(current_map_info != NULL);

                    if(is_found_fault_page == 0){
                        //uartdbg_printf("NOT Found loaded page.\n");
                        //malloc_stats();
                        prev_map_info->next_vmf_map_info = pvPortMalloc(sizeof(vmfile_map_info));
                        if(prev_map_info->next_vmf_map_info == NULL){
                            malloc_stats();
                            printf("memory overflow!\n");
                            return 1;
                        }
                        current_map_info = prev_map_info->next_vmf_map_info;
                        current_map_info->next_vmf_map_info = NULL;
                        current_map_info->page_in_file_offset_addr = vm_page_alloc() * PAGE_SIZE;
                        if( current_map_info->page_in_file_offset_addr == -1){
                            printf("Fault return memory overflow 1: %08x\n",task_handle);
                            return 1;   //memory overflow
                        }
                        current_map_info->virt_map_addr = (unsigned int)access_fault_addr & 0xFFFFF000;
                        page_order = get_optimizing_new_cache_page(fault_ins_addr);
                        status = swap_in_cache_page(page_order, 
                                                    vm_page_file_fd,
                                                    0,
                                                    current_map_info->page_in_file_offset_addr, 
                                                    0,
                                                    0,
                                                    (unsigned int)access_fault_addr & 0xFFFFF000);

                        if(status < 0){
                            printf("load page file fault!\n");
                            return 1;
                        }
                        //pages_info[page_order].map_on_virt_addr
                        pages_info[page_order].belong_task = task_handle;
                        pages_info[page_order].map_file_fd = vm_page_file_fd;
                        pages_info[page_order].file_offset_addr = current_map_info->page_in_file_offset_addr;
                        pages_info[page_order].page_attr = current_zone_info->zone_attr;
                        //finish setting, reload all cache pages.
                        load_cache_page_to_map_vm_space(page_order, fault_ins_addr);
                        //printf("Fault return 1: %08x\n",task_handle);
                        return 0;

                    }
                    //END current_map_info NOT EQU NULL
                }else{
                    //START current_map_info EQU NULL
                    //uartdbg_printf("NOT Found loaded page 2.\n");
                    
                    current_zone_info ->first_map_info = pvPortMalloc(sizeof(vmfile_map_info));
                    //uartdbg_printf("malloc end.\n");

                    if(current_zone_info ->first_map_info == NULL){
                        printf("Fault return malloc fail 2: %08x\n",task_handle);
                        return 1;
                    }
                    current_map_info = current_zone_info ->first_map_info;
                    current_map_info->next_vmf_map_info = NULL;
                    current_map_info->page_in_file_offset_addr = vm_page_alloc() * PAGE_SIZE;
                    //uartdbg_printf("vm_page_alloc end.\n");
                    if( current_map_info->page_in_file_offset_addr < 0){
                            printf("Fault return memory overflow 2: %08x\n",task_handle);
                            return 1;   //memory overflow
                    }
                    current_map_info->virt_map_addr = (unsigned int)access_fault_addr & 0xFFFFF000;
                    page_order = get_optimizing_new_cache_page(fault_ins_addr);
                    //uartdbg_printf("get_optimizing_new_cache_page end.\n");
                    //printf("n get_optimizing_new_cache_page:%d \n",page_order);
                    status = swap_in_cache_page(page_order, 
                                                vm_page_file_fd,
                                                0,
                                                current_map_info->page_in_file_offset_addr, 
                                                0,
                                                0,
                                                (unsigned int)access_fault_addr & 0xFFFFF000);
                    if(status < 0){
                        printf("load page file fault!\n");
                        return 1;
                    }
                    pages_info[page_order].belong_task = task_handle;
                    pages_info[page_order].map_file_fd = vm_page_file_fd;
                    pages_info[page_order].file_offset_addr = current_map_info->page_in_file_offset_addr;
                    pages_info[page_order].page_attr = current_zone_info->zone_attr;
                    
                    //finish setting, reload all cache pages.
                    //uartdbg_printf("preload .\n");
                    load_cache_page_to_map_vm_space(page_order, fault_ins_addr);
                    //printf("Fault return 2: %08x\n",task_handle);

                    return 0;
                    //END current_map_info EQU NULL
                }

            }else if(current_zone_info -> map_file_fd > STDERR_FILENO){
                //else ZONE MAP TO A FILE
                //printf("fault area map in file.\n");
                //uartdbg_printf("fault addr:%x\n",access_fault_addr);
                vmfile_map_info *current_map_info;
                current_map_info = current_zone_info ->first_map_info;
                if(current_map_info != NULL){
                    do{
                        if((current_map_info->virt_map_addr) == ((unsigned int)access_fault_addr & 0xFFFFF000 )){
                            //printf("found file map.\n");
                            page_order = get_optimizing_new_cache_page(fault_ins_addr);
                            status = swap_in_cache_page(page_order, 
                                                        vm_page_file_fd, 
                                                        0,
                                                        current_map_info->page_in_file_offset_addr, 
                                                        0,
                                                        0,
                                                        (unsigned int)access_fault_addr & 0xFFFFF000);
                            if(status < 0){
                                printf("load page file fault!\n");
                                return 1;
                            }
                            pages_info[page_order].belong_task = task_handle;
                            pages_info[page_order].map_file_fd = vm_page_file_fd;
                            pages_info[page_order].file_offset_addr = current_map_info->page_in_file_offset_addr;
                            pages_info[page_order].page_attr = current_zone_info->zone_attr;
                            
                            //finish setting, reload all cache pages.
                            load_cache_page_to_map_vm_space(page_order, fault_ins_addr);
                            return 0; 

                        }
                        current_map_info = current_map_info->next_vmf_map_info;
                    }while(current_map_info != NULL);
                }
                

                unsigned int fault_in_file_addr;
                page_order = get_optimizing_new_cache_page(fault_ins_addr);

                
                //printf("page diff:%d\n",(unsigned int)current_zone_info->zone_start_addr & 0xFFF);
/*
                if( ((unsigned int)current_zone_info->zone_start_addr & 0xFFF) == 0){
                    fault_in_file_addr = ((unsigned int)access_fault_addr - (unsigned int)current_zone_info->zone_start_addr) & 0xFFFFF000;
                }else{
                    fault_in_file_addr = ((unsigned int)access_fault_addr - (unsigned int)current_zone_info->zone_start_addr);
                }
*/
                fault_in_file_addr =   ((unsigned int)access_fault_addr - (unsigned int)current_zone_info->zone_start_addr);

                int page_diff = (unsigned int)current_zone_info->zone_start_addr & 0xFFF;

                status = swap_in_cache_page(page_order, 
                                            current_zone_info->map_file_fd, 
                                            (unsigned int)current_zone_info->file_offset_addr,
                                            fault_in_file_addr,
                                            ((unsigned int)access_fault_addr >> 12) - ((unsigned int)current_zone_info->zone_start_addr >> 12),
                                            page_diff,
                                            (unsigned int)access_fault_addr & 0xFFFFF000);
                
                //printf("swap_in_cache_page:%d\n",status);
                if(status < 0)
                {
                    printf("load page file fault!\n");
                    return 1;
                }
                pages_info[page_order].belong_task = task_handle;
                pages_info[page_order].map_file_fd = current_zone_info -> map_file_fd;
                pages_info[page_order].file_offset_addr = current_zone_info->file_offset_addr  +  fault_in_file_addr;


                pages_info[page_order].page_attr = current_zone_info->zone_attr;
                //printf("preload.\n");
                load_cache_page_to_map_vm_space(page_order, fault_ins_addr);
                //printf("load.\n");
                return 0;



            }



        }
        current_zone_info = current_zone_info->next_info;
    }while(current_zone_info != NULL);



    return 1;
}


