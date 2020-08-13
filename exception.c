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
 
#include "uart_debug.h"
#include "exception.h"


void __handler_und(void) __attribute__((naked));
void __handler_und(){
	
	uartdbg_print_regs();
	uartdbg_printf("Undifined abortion\n");
	while(1);
}


void __handler_pabort(void) __attribute__((naked));
void __handler_pabort(){
	
	uartdbg_print_regs();
	uartdbg_printf("Prefetch abortion\n");
	while(1);
}

void __handler_swi(void) __attribute__((naked));
void __handler_swi(){
	
	uartdbg_print_regs();
	uartdbg_printf("Software interrupt.\n");	
	while(1);
}

void __handler_dabort(void) __attribute__((naked));
void __handler_dabort(){
	asm volatile ("stmfd sp!, {r0-r12, r14}");
    asm volatile ("mrs r0,cpsr");
    asm volatile ("orr r0,r0,#0xc0");   // DISABLE ALL FIQ AND IRQ INTERRUPTS
    asm volatile ("msr cpsr,r0");
	
	
	uartdbg_print_regs();
	uartdbg_printf("Data abort\n");
	
	while(1);
}


void exception_install(exception_type type, unsigned int *exception_handler_addr){
	unsigned int *exception_table_base = (unsigned int *)0x00000000;
	//resule = FFFFFE+(jmp_addr/4)-(offset/4)	现场编译跳转指令（（
	exception_table_base[type] = 0xEA000000 | ((0xFFFFFE + (((unsigned int)(exception_handler_addr))/4)-(((unsigned int)&exception_table_base[type])/4))&0x00FFFFFF);
}

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


