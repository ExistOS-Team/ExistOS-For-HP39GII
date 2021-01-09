/*
 * A simple user interface for this project
 *
 * Copyright 2020 Creep_er
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */
  
#include "portmacro.h"

#include "uart_debug.h"
#include "exception.h"
#include "memory_map.h"
#include "mmu.h"

#include "ServiceSwap.h"

#include "startup_info.h"
#include "ServiceSwap.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>
unsigned int faultAddress;
unsigned int insAddress;
unsigned int FSR;

  

unsigned char exception_handler_table[4096] 
__attribute__ (( section(".exception_handler_table") )) __attribute__ ((aligned(4096)));

volatile unsigned int exception_handler_map_table[256] 
__attribute__ (( section(".exception_handler_map_table") )) __attribute__((aligned(0x400))) ;

 

void *swi_jump_table[255];

void __handler_und(void) __attribute__((naked));
void __handler_und(){
	asm volatile ("mov r0,lr");
	asm volatile ("str r0,%0":"=m"(insAddress));	//取出异常指令的地址
	
	uartdbg_printf("Undifined abortion at: %x\n",insAddress);
	while(1);
}


void __handler_pabort(void) __attribute__((naked));
void __handler_pabort(){
	asm volatile ("mov r0,lr");
	asm volatile ("str r0,%0":"=m"(insAddress));	//取出异常指令的地址

	//uartdbg_print_regs();
	uartdbg_printf("Prefetch abortion at: %x\n",insAddress);
	while(1);
}

void fault(){
	
	for(;;){}
}




//extern unsigned int ABT_STACK_ADDR;



void __handler_dabort(void) __attribute__((naked));
void __handler_dabort(){
	
	//asm volatile ( "stmfd sp!,{r0-r12, lr}");
	//uartdbg_printf("dabort 11\n");
	
	//while(1);
	
	asm volatile ("ldr sp,=ABT_STACK_ADDR");
	//asm volatile ("add sp,sp,#0xC0000000");
	
	
	asm volatile ("subs lr,lr,#8");
	asm volatile ("stmfd sp!, {r0-r12, lr}");
	
	//asm volatile ("mrs r0,spsr");
	//asm volatile ("stmfd sp!, {r0}");
	
	asm volatile ("mov r0,lr");
	asm volatile ("str r0,%0":"=m"(insAddress));	//取出异常指令的地址
    asm volatile ("mrc p15, 0, r0, c6, c0, 0");
	asm volatile ("str r0,%0":"=m"(faultAddress));	//取出异常指令访问的地址
	asm volatile ("mrc p15, 0, r0, c5, c0, 0");	// D
	asm volatile ("str r0,%0":"=m"(FSR));	//取出异常指令的地址
	

	//uartdbg_printf("Page fault at 0x%x referenced memory at 0x%x, FSR:%x \n",insAddress,faultAddress,FSR);
	//switch_mode(SVC_MODE);
	
	unsigned int error = 0;	
	vTaskSuspendAll();
	//printf("Page fault at 0x%04X referenced memory at 0x%04X, FSR:%08x \n",insAddress,faultAddress,FSR);
	//uartdbg_printf("Page fault at 0x%x referenced memory at 0x%x, FSR:%x \n",insAddress,faultAddress,FSR);
	
	//vTaskDelete(NULL);
	
	error = pageFaultISR(
		0,
		faultAddress,
		insAddress,
		FSR
	);
	
	//xQueueSend(Q_MEM_Exception,&e,0);	
	//xQueueSendFromISR(Q_MEM_Exception,&e,0);
	
	//printf("page fault!!!\n");
	
	
	
	if(error){
		printf("The instruction at 0x%04X referenced\n memory at 0x%04X. \n",insAddress,faultAddress);
		printf("The memory could not be read.\n");
		printf("<< %s >> killed.\n",pcTaskGetName(NULL));
		/*
		while(lGetCurrentTaskMallocCount(NULL) > -1){
			printf("Force free %d,%08x\n",lGetCurrentTaskMallocCount(NULL),ulGetCurrentTaskMallocLogTableVal(NULL,lGetCurrentTaskMallocCount(NULL)));
			vPortFree( (unsigned int *) ulGetCurrentTaskMallocLogTableVal(NULL,lGetCurrentTaskMallocCount(NULL)) );
		for(int i=0;i<lGetCurrentTaskMallocCount(NULL);i++){
				printf("malloc %d,%08x\n",i,ulGetCurrentTaskMallocLogTableVal(NULL,i));
			}
		
		}
		*/
		
		
		xTaskResumeAll();
		vTaskDelete(NULL);
	}
	
	xTaskResumeAll();
	//switch_mode(ABT_MODE);
	//vTaskSuspend(NULL);
	
	//asm volatile ( "ldmia sp!,{r0-r12, lr}");
	//asm volatile ("subs pc,lr,#8");
	
	//asm volatile ("ldmia sp!,{r0}");
	//asm volatile ("msr spsr,r0");
	asm volatile ("ldmia sp!, {r0-r12, pc}^");
	//asm volatile ("subs pc,lr,#8");
	
	
/*
	asm volatile ("add lr,lr,#8");
	
	portSAVE_CONTEXT_ASM;

	asm volatile ( "bl vTaskSwitchContext" );

	portRESTORE_CONTEXT_ASM;  
*/


	/*
	asm volatile ("subs lr,lr,#8");		//LR-8为异常发生时的地址，如果是缺页处理完后应返回该地址
	//asm volatile ("stmfd sp!, {r0-r12, lr}");
	asm volatile ("stmfd sp!, {r0-r12, lr}");
	
    asm volatile ("mrs r0,cpsr");
    asm volatile ("orr r0,r0,#0xc0");   //禁止所有中断
    asm volatile ("msr cpsr,r0");
	
	asm volatile ("mov r0,lr");
	asm volatile ("str r0,%0":"=m"(insAddress));	//取出异常指令的地址
    asm volatile ("mrc p15, 0, r0, c6, c0, 0");
	asm volatile ("str r0,%0":"=m"(faultAddress));	//取出异常指令访问的地址
	asm volatile ("mrc p15, 0, r0, c5, c0, 0");	// D
	asm volatile ("str r0,%0":"=m"(FSR));	//取出异常指令的地址
	
*/
	/*
	if(swapSizeMB > 0){
		
		if(( faultAddress > HEAP_MEMORY + (256 * 1024) ) 
			&&( faultAddress < HEAP_MEMORY + (256 * 1024) + (swapSizeMB * 1024 * 1024) ) ){
			
			da_fault_page_ext_stor = (faultAddress - HEAP_MEMORY + 256 * 1024) / 4096;
			
			if(page_buffer_virtual_addr != NULL){
				
				
				
				f_write(getSwapfileHandle(), 
					page_buffer_virtual_addr[buffer_page_index]  ,
					4096,
					&page_save_size
					);
				
			}

		}
		
	}
	*/
	
	//printf("====Data Abort.==== \n");
	//printf("The instruction at 0x%04X referenced\n memory at 0x%04X. \n",insAddress,faultAddress);
	//printf("The memory could not be read.\n");
	//printf("System halt.\n");
	//fault();
	
	/*
	MEM_Exception e;
	e.ExceptionTaskHandle = xTaskGetHandle(pcTaskGetName(NULL));
	e.accessFaultAddress = faultAddress;
	e.insFaultAddress = insAddress;
	e.FSR = FSR;
	
	//xQueueSend(Q_MEM_Exception,&e,0);	
	xQueueSendFromISR(Q_MEM_Exception,&e,0);	
	//vTaskSuspend(NULL);
	
	*/
	
	/*
	asm volatile ("mrs r0, cpsr_all");
	asm volatile ("bic r0, r0, #0xc0");
	asm volatile ("msr cpsr_all, r0");
	
	asm volatile ("ldmia sp!, {r0-r12, pc}^");*/
	
	//asm volatile ("ldmia sp!, {r0-r12, lr}");

	
	//asm volatile ("add lr,lr,#8");
	//portSAVE_CONTEXT_ASM;
	/*
	asm volatile ("stmfd sp!, {lr}");
	
	//vTaskSuspend(NULL);
	
	asm volatile ("ldmia sp!, {lr}");
	asm volatile ("subs lr,lr,#8");
	asm volatile ("mov pc,lr");
	*/
	//asm volatile ("stmfd sp!, {r0-r12, lr}");
	

	
	
	//vTaskDelay(1000);
	
	//printf("\nException 1.\n");
	//vTaskDelay(1000);
	//printf("\nException 2.\n");
	
	
	//asm volatile ("ldmia sp!, {r0-r12, pc}^");
	
	//uartdbg_print_regs();
	//printf("%s\n",pcTaskGetName(NULL));
	
	//asm volatile ( "bl vTaskSwitchContext" );
	
	
	//portRESTORE_CONTEXT_ASM; 


}


void src_c_swi_handler(unsigned int arg0, unsigned int arg1, unsigned arg2, unsigned int swiImmed);

volatile void __handler_swi(void) __attribute__((naked));
volatile void __handler_swi(void)
{
	
	
	
		asm volatile ( "stmfd sp!,{r0-r3, lr}");

		asm volatile ("ldr r3, [lr, #-4]");
		
		asm volatile ("bic r3, r3, #0xff000000");
		
		asm volatile ( "bl src_c_swi_handler" );
		
		asm volatile ( "ldmia sp!,{r0-r3, lr}");
		
		
		asm volatile ("add lr,lr,#4");
		portSAVE_CONTEXT_ASM;

		asm volatile ( "bl vTaskSwitchContext" );



		
		portRESTORE_CONTEXT_ASM;                          		

}




void install_swi_service(unsigned int swi_num, void *service){
	swi_jump_table[swi_num] = service;
}

void exception_install(exception_type type, unsigned int *exception_handler_addr){
	unsigned int *exception_table_base = (unsigned int *)EXCEPTION_VECTOR_TABLE_BASE_ADDR;
	//resule = FFFFFE+(jmp_addr/4)-(offset/4)	现场编译跳转指令 // B xx
	//exception_table_base[type] = 0xEA000000 | ((0xFFFFFE + (((unsigned int)(exception_handler_addr))/4)-(((unsigned int)&exception_table_base[type])/4))&0x00FFFFFF);
	
	exception_table_base[type] = 0xE59FF018;	//ldr pc,[pc,#0x18];
	exception_table_base[type + (0x20 / 4)] = (unsigned int)exception_handler_addr;
	

	//flush_tlb();
}

extern void __handler_swi_asm(void);
extern unsigned int *tlb_base;

void exception_init(){

	for(int i=0; i< 4096;i++){
		exception_handler_table[i] = 0;
	}
	for(int i=0; i< 256;i++){
		exception_handler_map_table[i] = 0;
	}

	BF_WRn(DIGCTL_MPTEn_LOC,4,LOC,EXCEPTION_VECTOR_TABLE_BASE_ADDR >> 20);
	
	MMU_MAP_COARSE_RAM((unsigned int) VIR_TO_PHY_ADDR( (uint8_t*) &exception_handler_map_table), EXCEPTION_VECTOR_TABLE_BASE_ADDR);
	
	MMU_MAP_SMALL_PAGE_NONCACHED((unsigned int)VIR_TO_PHY_ADDR( (uint8_t*) &exception_handler_table),EXCEPTION_VECTOR_TABLE_BASE_ADDR);

	flush_tlb();

    unsigned *exception_handler_addr = (unsigned int *)EXCEPTION_VECTOR_TABLE_BASE_ADDR;	
	for(int i=0; i<0x1C; i++) {
		exception_handler_addr[i] = 0;					//清空异常向量表
	}
	exception_install(EXCEPTION_UND,(unsigned int *)&__handler_und);
	exception_install(EXCEPTION_SWI,(unsigned int *)&__handler_swi);
	exception_install(EXCEPTION_PABORT,(unsigned int *)&__handler_pabort);
	exception_install(EXCEPTION_DABORT,(unsigned int *)&__handler_dabort);
	

	
	
	asm volatile ("mrc p15, 0, r0, c1, c0, 0");
	//asm volatile ("bic r0,r0,#0x2000"); 				//设置使用低端向量表
	asm volatile ("orr r0,r0,#0x2000"); 				//设置使用高端向量表
	asm volatile ("mcr p15, 0, r0, c1, c0, 0");
	
	
}


