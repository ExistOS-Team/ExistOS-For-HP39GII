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

#include "eabi_swi_system_call.h"

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
    int *addr = (((unsigned int *)insAddress) - 2);
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
volatile uint32_t swi_n;
extern volatile uint32_t *pxCurrentTCB;
extern unsigned int fault_count;
volatile void __handler_pabort(void) __attribute__((naked));
volatile void __handler_pabort() {

    //asm volatile("ldr sp,=ABT_STACK_ADDR");
    //asm volatile("ldr sp,=ABT_STACK_ADDR");
    //portSAVE_CONTEXT_ASM;
    
    asm volatile("sub lr,lr,#4");
    
    asm volatile("stmfd sp!, {r0-r12, lr}");

    
    //asm volatile("mrs r1, cpsr_all");
    //asm volatile("orr r1, r1, #0xc0");
    //asm volatile("msr cpsr_all, r1");

    asm volatile("mov r0,lr");
    //asm volatile("sub r0,#4");
    asm volatile("str r0,%0"
                 : "=m"(insAddress)); //取出异常指令的地址

    unsigned int error = 0;
    //vTaskSuspendAll();
    //vTaskSuspendAll();
    
    

    //uartdbg_printf("PA insAddress:%x \n",insAddress);
    //uartdbg_print_regs();

    //asm volatile("mrs r1, cpsr_all");
    //asm volatile("bic r1, r1, #0xc0");
    //asm volatile("msr cpsr_all, r1");

    

    PID_t current_PID;
    current_PID = get_current_running_task_pid();

    //uartdbg_print_regs();

    if(current_PID == -1){
        uartdbg_printf("no pid:%d\n",current_PID);
        error = 1;
    }else{
        //uartdbg_printf("test:%d\n",current_PID);
        
        error = data_access_fault_isr( current_PID ,
                    (unsigned int *)insAddress,
                    (unsigned int *)insAddress,
                    FSR);
    }

    fault_count = 0;

    if (error) {

        //extern volatile uint32_t *pxCurrentTCB;
        //uartdbg_printf("swi contx:%x\n",pxCurrentTCB); 
        /*
        for(int i=0;i<128;i++){
            printf("pxCurrentTCB[%d,%08x]:%08x\n",i,(int)pxCurrentTCB + i,pxCurrentTCB[i]);
        }
        */
       //dump_vm_spaces();
       printf("Bad instruction at:%08X\n",insAddress);
       //printf("Bad instruction:%08X\n",*((int *)insAddress));

        //printf("The instruction at 0x%04X referenced\n memory at 0x%04X. \n", insAddress, insAddress);
        //printf("The memory could not be read.\n");
        printf("<< %s >> killed.\n", pcTaskGetName(NULL));
        //
        
        vTaskDelete(NULL);
        //vTaskSwitchContext();
    }else{
        //printf("FAULT INS:%08x\n",*((int *)insAddress));
    }
    
    //xTaskResumeAll();
    //portRESTORE_CONTEXT_ASM;
    asm volatile("ldmia sp!, {r0-r12, pc}^");

}
 

unsigned int fault_sp;
volatile void __handler_dabort(void) __attribute__((naked));
volatile void __handler_dabort() {

    //asm volatile("ldr sp,=ABT_STACK_ADDR");
    //asm volatile("ldr sp,=ABT_STACK_ADDR");

    //

    //asm volatile("sub lr,lr,#4");
    //portSAVE_CONTEXT_ASM;


    asm volatile("sub lr,lr,#8");
    
    asm volatile("stmfd sp!, {r0-r12, lr}");
    
/*
    asm volatile("mrs r1, cpsr_all");
    asm volatile("orr r1, r1, #0xc0");
    asm volatile("msr cpsr_all, r1");
*/
    //asm volatile ("mrs r0,spsr");
    //asm volatile ("stmfd sp!, {r0}");

    asm volatile("mov r0,lr");
    //asm volatile("sub r0,r0,#4");
    asm volatile("str r0,%0"
                 : "=m"(insAddress)); //取出异常指令的地址
    asm volatile("mrc p15, 0, r0, c6, c0, 0");
    asm volatile("str r0,%0"
                 : "=m"(faultAddress));        //取出异常指令访问的地址
    asm volatile("mrc p15, 0, r0, c5, c0, 0"); // D
    asm volatile("str r0,%0"
                 : "=m"(FSR));
    //vTaskSuspendAll();
    //uartdbg_printf("DA faultAddress:%x insAddress:%x fsr:%x\n",faultAddress,insAddress,FSR);
    //uartdbg_printf("Page fault at 0x%x referenced memory at 0x%x, FSR:%x \n",insAddress,faultAddress,FSR);
        extern volatile uint32_t *pxCurrentTCB;
       // uartdbg_printf("swi contx:%x\n",pxCurrentTCB);   
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
    int current_PID;
    current_PID = get_current_running_task_pid();
    //printf("fault current_PID:%d\n",current_PID);
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

                
        //uartdbg_printf("swi contx:%x\n",pxCurrentTCB); 
        /*
        for(int i=0;i<128;i++){
            printf("pxCurrentTCB[%d,%08x]:%08x\n",i,(int)pxCurrentTCB + i,pxCurrentTCB[i]);
        }*/
        //dump_vm_spaces();
        printf("The instruction at 0x%08X referenced memory at 0x%08X, ", insAddress, faultAddress);
        printf("FSR:%08X\n",FSR);
        printf("The memory could not be read.\n");
        printf("<< %s >> killed.\n", pcTaskGetName(NULL));
        //uartdbg_print_regs();
        xTaskResumeAll();
        //switch_mode(ABT_MODE);
        vTaskDelete(NULL);
        //while(1);
        //vTaskSwitchContext();
    }

    //switch_mode(ABT_MODE);
    //uartdbg_print_regs();
    xTaskResumeAll();
    //portRESTORE_CONTEXT_ASM;
    asm volatile("ldmia sp!, {r0-r12, pc}^");
}


//volatile uint32_t tmp_stack[200];
//volatile uint32_t *tmp_stack_ptr = &tmp_stack[180];

//volatile uint32_t *swi_contx_ptr = &swi_contx_stack[0];

volatile void swi_eabi_handler(register uint32_t regs){
     __asm__ volatile ("push {r1-r12,lr}");
    struct regs{
        unsigned int r0;
        unsigned int r1;
        unsigned int r2;
        unsigned int r3;
        unsigned int r4;
        unsigned int r5;
        unsigned int r7;
    } *rc_regs;
    rc_regs = (struct regs *)regs;
   
    rc_regs->r0 = src_c_swi_handler(rc_regs->r0,rc_regs->r1,rc_regs->r2,rc_regs->r3,rc_regs->r4,rc_regs->r5,rc_regs->r7);
    asm volatile("ldr r7,=pxCurrentTCB");
    asm volatile("ldr r7,[r7]");
    asm volatile("ldr r7,[r7, #520]");
    asm volatile("msr cpsr,r7");
    __asm__ volatile ("pop {r1-r12,lr}");
    
}
 
volatile void __handler_swi(void) __attribute__((naked));
volatile void __handler_swi(void) {
   asm volatile("add lr,lr,#4");
   portSAVE_CONTEXT_ASM;
   asm volatile("ldr sp,=SVC_STACK_ADDR");
    __asm__ volatile ("":::"memory");
    volatile unsigned int *current_regs = ((unsigned int *)pxCurrentTCB[1]) - 16;
    int swi_num = ((*(((unsigned int *) *((unsigned int *)pxCurrentTCB[1]-1)) - 2)) & 0x00FFFFFF);
    switch (  swi_num  )
    {

    case 1:
        vTaskSwitchContext();
        break;
    case 0:
        uartdbg_printf("SYSCALL\n");
        current_regs[15] = ((unsigned int)swi_eabi_handler) + 4;
        pxCurrentTCB[130] = current_regs[-1];
        current_regs[-1] &= 0xFFFFFFE0;
        current_regs[-1] |= 0x1f;
        break;

    default:

        uartdbg_printf("SYSCALL %x\n",swi_num);
        break;
    }
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

    MMU_MAP_SMALL_PAGE_NONCACHED_PRIVILEGED((unsigned int)VIR_TO_PHY_ADDR((uint8_t *)&exception_handler_table), EXCEPTION_VECTOR_TABLE_BASE_ADDR);

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
