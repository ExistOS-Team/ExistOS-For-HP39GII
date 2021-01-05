#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>

#include "init.h"
#include "cdc_console.h"
#include "keyboard.h"
#include "ff.h"
#include "rtc.h"
#include "startup_info.h"
#include "memory_map.h"
#include "mmu.h"
#include "regsdigctl.h"

#include "ServiceGraphic.h"
#include "ServiceSTMPPartition.h"
#include "ServiceFlashMap.h"
#include "ServiceRawFlash.h"
#include "ServiceUSBDevice.h"
#include "ServiceFatFs.h"
#include "ServiceSwap.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "uart_debug.h"
#include "map.h"

extern int heapBytesRemaining;

unsigned char *__pageBuffer;
unsigned char *__l2_pgbuff_tab;

FIL PageFileHandle;

FIL* getPageFileHandle(){
	return &PageFileHandle;
}

unsigned int swap_file_inited = 0;

unsigned char *l2_pgbuff_tab;

unsigned int buffer_page_index = 0;
unsigned int buffer_section_index = 0;

unsigned int fault_addr_in_page;
unsigned int page_save_size;

unsigned char *pageBuffer;



//page_buffer_info page_buffer_info;
unsigned int *page_buffer_info;

volatile int last_save_Bank1_page_addr = -1;


extern unsigned int *tlb_base;

volatile int current_buffer_page_addr = -1;

//unsigned int *mmu_base=(unsigned int *)FIRST_LEVEL_PAGE_TABLE_BASE;	//一级页表基地址

	FRESULT pg_fr;
	UINT pg_br;
	
unsigned int pageFaultISR(TaskHandle_t ExceptionTaskHandle, unsigned int accessFaultAddress, unsigned int insFaultAddress, unsigned int FSR){
	
			fault_addr_in_page = (accessFaultAddress - KHEAP_MEMORY_VIR_START) / PAGE_SIZE;
			//printf("FAULT ACCESS VM PAGE:%08x\n",fault_addr_in_page);	
			//while(1);
			
			
			if(swap_file_inited){
				printf("<%d %d %d>\n",fault_addr_in_page
					,( (PHY_KHEAP_MEMORY_PAGES - 1) ),
					( PHY_KHEAP_MEMORY_PAGES + (swapSizeMB * PAGES_PER_MIB)));
					
					
				if(( fault_addr_in_page > (PHY_KHEAP_MEMORY_PAGES - 1) )&& 
						(fault_addr_in_page < PHY_KHEAP_MEMORY_PAGES + (swapSizeMB * PAGES_PER_MIB))
						){
							
							//unsigned int want_to_access_section = accessFaultAddress >> 20;
							/*
							for(int i = 0;i<VM_ROUND_ROBIN_SECTIONS; i++){
								//if(){
									
								//}
							}
							*/
							
							
							if(current_buffer_page_addr != -1){
								f_lseek(&PageFileHandle, current_buffer_page_addr);
								pg_fr = f_write (&PageFileHandle, pageBuffer,PAGE_SIZE ,&pg_br );
							}
							
							current_buffer_page_addr = (fault_addr_in_page - PHY_KHEAP_MEMORY_PAGES) * PAGE_SIZE;
							
							f_lseek(&PageFileHandle, current_buffer_page_addr);
							pg_fr = f_read(&PageFileHandle, pageBuffer, PAGE_SIZE, &pg_br);
							
							
							
							if(		(accessFaultAddress >> 20 != 1) 
								&&  (accessFaultAddress >> 20 != 0)
								&&  (accessFaultAddress >> 20 != BF_RDn(DIGCTL_MPTEn_LOC,0,LOC))
								){
									
									BF_WRn(DIGCTL_MPTEn_LOC,0,LOC,accessFaultAddress >> 20);
									MMU_MAP_COARSE_RAM((unsigned int) VIR_TO_PHY_ADDR((uint8_t *)l2_pgbuff_tab) , accessFaultAddress & 0xFFF00000);
									memset(l2_pgbuff_tab,0,PAGE_TABLE_SIZE);
							}
							
							if(last_save_Bank1_page_addr != -1){
										MMU_MAP_SMALL_PAGE_UNMAP(last_save_Bank1_page_addr);
										last_save_Bank1_page_addr = -1;
							}
							if(accessFaultAddress >> 20 == 1){
								last_save_Bank1_page_addr = accessFaultAddress;
							}
							
							
							
							MMU_MAP_SMALL_PAGE_CACHED((unsigned int) VIR_TO_PHY_ADDR((uint8_t *)(pageBuffer)),accessFaultAddress);	
							
							
						
							asm volatile ("mov	r0, #0");
							asm volatile ("mcr p15, 0, r0, c8, c7, 0");
					
							
							return 0;
					//vTaskSuspendAll();
				}
			}
	//printf("");
	
	return 1;
}



void vServiceException( void *pvParameters ){
	
	Q_MEM_Exception = xQueueCreate(128,sizeof(MEM_Exception));
	MEM_Exception e;

	for(;;){
		if(xQueueReceive( Q_MEM_Exception, (&e), ( TickType_t ) portMAX_DELAY ) == pdTRUE ){

		}
				
		//vTaskDelay(100);
	}
}



void vServiceSwap( void *pvParameters ){
	
	xTaskCreate( vServiceException, "MEM Exception SVC", configMINIMAL_STACK_SIZE, NULL, 4, NULL );
	
	swap_file_inited = 0;
	if(swapSizeMB == 0){
		vTaskDelete(NULL);
	}
	
	FRESULT fr;
	
	fr = f_open(&PageFileHandle,"Pagefile",FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
	if(fr != 0){
		printf("Create pagefile fail, %d\n",fr);
		
		vTaskDelete(NULL);
	}
	
	fr = f_expand(&PageFileHandle,swapSizeMB * 1024 * 1024, 1);
	
	f_sync(&PageFileHandle);
	flashSyncNow();
	
	
	if(fr != 0){
		printf("Allocates %d MB pagefile fail, %d\n",swapSizeMB,fr);
		
		vTaskDelete(NULL);
	}
	
	__pageBuffer = (unsigned char *)((unsigned char *)
	
		pvPortMalloc( 
		
		sizeof(int) * 1
		
		+ (1 + 1) * PAGE_SIZE )
			
	);
	
	__pageBuffer = (unsigned char *)(__pageBuffer);
	
	
	
	page_buffer_info  = __pageBuffer;
	memset(page_buffer_info,0,sizeof(page_buffer_info) * BUFFER_PAGES);
	
	 
	
	l2_pgbuff_tab = __l2_pgbuff_tab = malloc(PAGE_TABLE_SIZE * 2);//malloc((VM_SECTION_NUM + 1) * PAGE_TABLE_SIZE); 
	
	while((unsigned int)(l2_pgbuff_tab++) % PAGE_TABLE_SIZE != 0);
	l2_pgbuff_tab--;
	l2_pgbuff_tab = (unsigned char *)(l2_pgbuff_tab);
	
	
	memset(l2_pgbuff_tab,0,PAGE_TABLE_SIZE);
	 
	pageBuffer = __pageBuffer + sizeof(int) * BUFFER_PAGES;
	
	while((unsigned int)(pageBuffer++) % PAGE_SIZE != 0);
	pageBuffer--;
	
	printf("page buffer addr:%08x , %08x\n",pageBuffer,l2_pgbuff_tab);
	
	swap_file_inited = 1;
	vTaskDelete(NULL);
}