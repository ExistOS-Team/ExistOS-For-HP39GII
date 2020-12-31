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
#include <stdio.h>

void *swi_jump_table[255];

void __handler_und(void) __attribute__((naked));
void __handler_und(){
	
	printf("Undifined abortion\n");
	while(1);
}


void __handler_pabort(void) __attribute__((naked));
void __handler_pabort(){
	
	uartdbg_print_regs();
	uartdbg_printf("Prefetch abortion\n");
	while(1);
}


unsigned int faultAddress;
unsigned int insAddress;

void __handler_dabort(void) __attribute__((naked));
void __handler_dabort(){
	

	
	asm volatile ("subs lr,lr,#8");		//LR-8为异常发生时的地址，如果是缺页处理完后应返回该地址
	asm volatile ("stmfd sp!, {r0-r12, lr}");
    asm volatile ("mrs r0,cpsr");
    asm volatile ("orr r0,r0,#0xc0");   //禁止所有中断
    asm volatile ("msr cpsr,r0");
	
	asm volatile ("mov r0,lr");
	asm volatile ("str r0,%0":"=m"(insAddress));	//取出异常指令的地址
    asm volatile ("mrc p15, 0, r0, c6, c0, 0");
	asm volatile ("str r0,%0":"=m"(faultAddress));	//取出异常指令访问的地址
	
	printf("====Data Abort.==== \n");
	printf("The instruction at 0x%04X referenced\n memory at 0x%04X. \n",insAddress,faultAddress);
	printf("The memory could not be read.\n");
	printf("System halt.\n");
	while(1);
	
	
	asm volatile ("ldmia sp!, {r0-r12, pc}^");
	//uartdbg_print_regs();
	//uartdbg_printf("Data abort\n");
	
	//while(1);
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
	unsigned int *exception_table_base = (unsigned int *)0x00000000;
	//resule = FFFFFE+(jmp_addr/4)-(offset/4)	现场编译跳转指令 // B xx
	exception_table_base[type] = 0xEA000000 | ((0xFFFFFE + (((unsigned int)(exception_handler_addr))/4)-(((unsigned int)&exception_table_base[type])/4))&0x00FFFFFF);
}

extern void __handler_swi_asm(void);

void exception_init(){

    unsigned *exception_handler_addr = (unsigned int *)EXCEPTION_VECTOR_TABLE_BASE_ADDR;	
	for(int i=0; i<0x1C; i++) {
		exception_handler_addr[i] = 0;					//清空异常向量表
	}
	exception_install(EXCEPTION_UND,(unsigned int *)&__handler_und);
	exception_install(EXCEPTION_SWI,(unsigned int *)&__handler_swi);
	exception_install(EXCEPTION_PABORT,(unsigned int *)&__handler_pabort);
	exception_install(EXCEPTION_DABORT,(unsigned int *)&__handler_dabort);
	
	asm volatile ("mrc p15, 0, r0, c1, c0, 0");
	asm volatile ("bic r0,r0,#0x2000"); 				//设置使用低端向量表
	asm volatile ("mcr p15, 0, r0, c1, c0, 0");
}


