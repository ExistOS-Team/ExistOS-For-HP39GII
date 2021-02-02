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

#include "exception.h"
#include "memory_map.h"
#include "mmu.h"
#include "uart_debug.h"

#include "ServiceSwap.h"

#include "pageman.h"
#include "ProcessesMan.h"
#include "startup_info.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>
unsigned int faultAddress;
unsigned int insAddress;
unsigned int FSR;

extern void flush_tlb();

unsigned char exception_handler_table[4096]
    __attribute__((section(".exception_handler_table"))) __attribute__((aligned(4096)));

volatile unsigned int exception_handler_map_table[256]
    __attribute__((section(".exception_handler_map_table"))) __attribute__((aligned(0x400)));

void *swi_jump_table[255];

void __handler_und(void) __attribute__((naked));
void __handler_und() {
    asm volatile("mov r0,lr");
    asm volatile("str r0,%0"
                 : "=m"(insAddress)); //取出异常指令的地址

    uartdbg_printf("undefined abortion at: %x\n", insAddress);
    int i =0;
    int *addr = insAddress - 8;
    for(int i = 0; i< 16; i++){
        uartdbg_printf("%x :  %x\n",addr,*addr);
        addr++;
    }
    switch_mode(SVC_MODE);
    uartdbg_print_regs();
    switch_mode(SYS_MODE);
    uartdbg_print_regs();

    //dump_vm_spaces();
    while (1)
        ;
}


volatile void __handler_pabort(void) __attribute__((naked));
volatile void __handler_pabort() {

    asm volatile("ldr sp,=ABT_STACK_ADDR");

    asm volatile("subs lr,lr,#4");
    asm volatile("stmfd sp!, {r0-r12, lr}");

    //asm volatile("mrs r1, cpsr_all");
    //asm volatile("orr r1, r1, #0xc0");
    //asm volatile("msr cpsr_all, r1");

    asm volatile("mov r0,lr");
    asm volatile("str r0,%0"
                 : "=m"(insAddress)); //取出异常指令的地址

    unsigned int error = 0;
    vTaskSuspendAll();


    //uartdbg_printf("PA insAddress:%x fsr:%x\n",insAddress,FSR);
    //uartdbg_print_regs();

    //asm volatile("mrs r1, cpsr_all");
    //asm volatile("bic r1, r1, #0xc0");
    //asm volatile("msr cpsr_all, r1");

    

    PID_t current_PID;
    current_PID = get_current_running_task_pid();

    //uartdbg_print_regs();

    if(current_PID == -1){
        //uartdbg_printf("no pid:%d\n",current_PID);
        error = 1;
    }else{
        //uartdbg_printf("test:%d\n",current_PID);
        error = data_access_fault_isr( current_PID ,
                    (unsigned int *)insAddress,
                    (unsigned int *)insAddress,
                    FSR);
    }
    if (error) {
        printf("The instruction at 0x%04X referenced\n memory at 0x%04X. \n", insAddress, insAddress);
        printf("The memory could not be read.\n");
        printf("<< %s >> killed.\n", pcTaskGetName(NULL));
        xTaskResumeAll();
        vTaskDelete(NULL);
    }    
    
    xTaskResumeAll();
    asm volatile("ldmia sp!, {r0-r12, pc}^");

}

void fault() {

    for (;;) {
    }
}
extern unsigned int fault_count;
unsigned int fault_sp;
volatile void __handler_dabort(void) __attribute__((naked));
volatile void __handler_dabort() {

    asm volatile("ldr sp,=ABT_STACK_ADDR");

    asm volatile("subs lr,lr,#8");
    asm volatile("stmfd sp!, {r0-r12, lr}");
/*
    asm volatile("mrs r1, cpsr_all");
    asm volatile("orr r1, r1, #0xc0");
    asm volatile("msr cpsr_all, r1");
*/
    //asm volatile ("mrs r0,spsr");
    //asm volatile ("stmfd sp!, {r0}");

    asm volatile("mov r0,lr");
    asm volatile("str r0,%0"
                 : "=m"(insAddress)); //取出异常指令的地址
    asm volatile("mrc p15, 0, r0, c6, c0, 0");
    asm volatile("str r0,%0"
                 : "=m"(faultAddress));        //取出异常指令访问的地址
    asm volatile("mrc p15, 0, r0, c5, c0, 0"); // D
    asm volatile("str r0,%0"
                 : "=m"(FSR));

    //uartdbg_printf("DA faultAddress:%x insAddress:%x fsr:%x\n",insAddress,faultAddress,FSR);
    //uartdbg_printf("Page fault at 0x%x referenced memory at 0x%x, FSR:%x \n",insAddress,faultAddress,FSR);
   
   /* switch_mode(SVC_MODE);

    switch_mode(SYS_MODE);    
    asm volatile("mov r0,sp");
    asm volatile("str r0,%0" : "=o"(fault_sp));
    switch_mode(SVC_MODE);
*/

    //asm volatile("ldr sp,=SVC_STACK_ADDR");

    unsigned int error = 0;
    vTaskSuspendAll();
    

    
    //printf("Page fault at 0x%04X referenced memory at 0x%04X, FSR:%08x \n",insAddress,faultAddress,FSR);
    //uartdbg_printf("Page fault at 0x%x referenced memory at 0x%x, FSR:%x \n",insAddress,faultAddress,FSR);

    //vTaskDelete(NULL);
/*
    error = pageFaultISR(
        0,
        faultAddress,
        insAddress,
        FSR);
*/
    PID_t current_PID;
    current_PID = get_current_running_task_pid();
    if(current_PID == -1){
        error = 1;
    }else{
        error = data_access_fault_isr( current_PID ,
                    (unsigned int *)faultAddress,
                    (unsigned int *)insAddress,
                    FSR);
    }

    fault_count = 0;
    
    //uartdbg_print_regs();
    //uartdbg_printf("error:%d\n",error);
    //xQueueSend(Q_MEM_Exception,&e,0);
    //xQueueSendFromISR(Q_MEM_Exception,&e,0);
    //switch_mode(ABT_MODE);
    //printf("page fault!!!\n");
/*
    asm volatile("mrs r1, cpsr_all");
    asm volatile("bic r1, r1, #0xc0");
    asm volatile("msr cpsr_all, r1");
*/

    if (error) {
        printf("The instruction at 0x%X referenced\n memory at 0x%X. \n", insAddress, faultAddress);
        printf("The memory could not be read.\n");
        printf("<< %s >> killed.\n", pcTaskGetName(NULL));
        //uartdbg_print_regs();
        xTaskResumeAll();
        //switch_mode(ABT_MODE);
        vTaskDelete(NULL);
    }

    //switch_mode(ABT_MODE);
    //uartdbg_print_regs();
    xTaskResumeAll();
    asm volatile("ldmia sp!, {r0-r12, pc}^");
}

void src_c_swi_handler(unsigned int arg0, unsigned int arg1, unsigned arg2, unsigned int swiImmed);

volatile void __handler_swi(void) __attribute__((naked));
volatile void __handler_swi(void) {
/*
    asm volatile("stmfd sp!,{r0-r3, lr}");

    asm volatile("ldr r3, [lr, #-4]");

    asm volatile("bic r3, r3, #0xff000000");

    asm volatile("bl src_c_swi_handler");

    asm volatile("ldmia sp!,{r0-r3, lr}");
*/
    asm volatile("ldr sp,=SVC_STACK_ADDR");
    asm volatile("add lr,lr,#4");
    portSAVE_CONTEXT_ASM;
    asm volatile("ldr sp,=SVC_STACK_ADDR");

    asm volatile("bl vTaskSwitchContext");

    portRESTORE_CONTEXT_ASM;
}

void install_swi_service(unsigned int swi_num, void *service) {
    swi_jump_table[swi_num] = service;
}

void exception_install(exception_type type, unsigned int *exception_handler_addr) {
    unsigned int *exception_table_base = (unsigned int *)EXCEPTION_VECTOR_TABLE_BASE_ADDR;
    //resule = FFFFFE+(jmp_addr/4)-(offset/4)	现场编译跳转指令 // B xx
    //exception_table_base[type] = 0xEA000000 | ((0xFFFFFE + (((unsigned int)(exception_handler_addr))/4)-(((unsigned int)&exception_table_base[type])/4))&0x00FFFFFF);

    exception_table_base[type] = 0xE59FF018; //ldr pc,[pc,#0x18];
    exception_table_base[type + (0x20 / 4)] = (unsigned int)exception_handler_addr;

    //flush_tlb();
}

extern void __handler_swi_asm(void);
extern unsigned int *tlb_base;

void exception_init() {

    for (int i = 0; i < 4096; i++) {
        exception_handler_table[i] = 0;
    }
    for (int i = 0; i < 256; i++) {
        exception_handler_map_table[i] = 0;
    }

    BF_WRn(DIGCTL_MPTEn_LOC, 6, LOC, EXCEPTION_VECTOR_TABLE_BASE_ADDR >> 20);

    MMU_MAP_COARSE_RAM((unsigned int)VIR_TO_PHY_ADDR((uint8_t *)&exception_handler_map_table), EXCEPTION_VECTOR_TABLE_BASE_ADDR);

    MMU_MAP_SMALL_PAGE_NONCACHED((unsigned int)VIR_TO_PHY_ADDR((uint8_t *)&exception_handler_table), EXCEPTION_VECTOR_TABLE_BASE_ADDR);

    flush_tlb();

    unsigned *exception_handler_addr = (unsigned int *)EXCEPTION_VECTOR_TABLE_BASE_ADDR;
    for (int i = 0; i < 0x1C; i++) {
        exception_handler_addr[i] = 0; //清空异常向量表
    }
    exception_install(EXCEPTION_UND, (unsigned int *)&__handler_und);
    exception_install(EXCEPTION_SWI, (unsigned int *)&__handler_swi);
    exception_install(EXCEPTION_PABORT, (unsigned int *)&__handler_pabort);
    exception_install(EXCEPTION_DABORT, (unsigned int *)&__handler_dabort);

    asm volatile("mrc p15, 0, r0, c1, c0, 0");
    //asm volatile ("bic r0,r0,#0x2000"); 				//设置使用低端向量表
    asm volatile("orr r0,r0,#0x2000"); //设置使用高端向量表
    asm volatile("mcr p15, 0, r0, c1, c0, 0");
}
