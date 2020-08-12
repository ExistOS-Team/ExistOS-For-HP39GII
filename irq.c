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
 
 
#include "regsicoll.h"
#include "exception.h"

unsigned int *irq_table_base = (unsigned int *)0x00000100;
 
void irq_empty(void)
{
	return;
}
 

void __irq_service() __attribute__ ((naked));
void __irq_service()
{
	
	   asm volatile ("sub lr,pc,#4");                     //计算中断处理完毕后的返回地址
       asm volatile ("stmdb sp!,{r0-r12,lr} ");      	  //保存使用到的寄存器                                                                                  
       //asm volatile ("bl EINT_Handle " )   ;            //调用中断服务函数，在interrupt.c中 
	   
       asm volatile ("ldmia sp!,{r0-r12,pc}^");           //中断返回，^表示将spsr的值复制到cpsr
	
	/*
	asm volatile ("stmfd sp!, {r0-r12,lr}");
    asm volatile ("mov r0,sp");
    asm volatile ("mrs r1,cpsr_all");
    asm volatile ("orr r1,r1,#0x1f");
    asm volatile ("msr cpsr_all,r1");  		// SWITCH TO SYSTEM MODE
    asm volatile ("stmfd r0!,{sp,lr}");		// SAVE REGISTERS THAT WERE BANKED
    asm volatile ("stmfd sp!,{ r0 }"); 		// SAVE IRQ STACK PTR
	/*
    (*( (__interrupt__) (irq_table[*HWREG(INT_REGS,0x14)]))) ();
	// CLEAR INTERRUPT PENDING FLAG
	register unsigned int a=1<<(*HWREG(INT_REGS,0x14));
	*HWREG(INT_REGS,0x0)=a;
	*HWREG(INT_REGS,0x10)=a;*/
	/*
    asm volatile ("ldmia sp!, { r0 }"); 	// GET IRQ STACK PTR
    asm volatile ("ldmia r0!, { sp, lr }"); // RESTORE USER STACK AND LR
    asm volatile ("mrs r1,cpsr_all");
    asm volatile ("bic r1,r1,#0xd");
    asm volatile ("msr cpsr_all,r1");  		// SWITCH BACK TO IRQ MODE
    asm volatile ("ldmia sp!, {r0-r12,lr}");    // RESTORE ALL OTHER REGISTERS BACK
	asm volatile ("subs pc,lr,#4");*/
}



inline unsigned int *irq_get_irqnumber_ctrl_priority_n_address(unsigned int irq_n){
	  unsigned int *baseAddress = 
            (unsigned int *)HW_ICOLL_PRIORITYn_ADDR((irq_n/4));
        baseAddress += irq_n%4;
	return baseAddress;
}


void irq_set_irq_n(unsigned int irq_n, unsigned int enable){
	if(irq_n > 63)return;
	unsigned int *baseAddress = irq_get_irqnumber_ctrl_priority_n_address(irq_n);
	if(enable)
		*(baseAddress + 4) = BM_ICOLL_PRIORITYn_ENABLE0;
	else
		*(baseAddress + 8) = BM_ICOLL_PRIORITYn_ENABLE0;
}


void irq_install(){
	
	
	for(unsigned int i=0;i<63;i++)
		irq_set_irq_n(i,0);
	
	for(unsigned int i=0;i<0xFF;i++){
		irq_table_base[i] = (unsigned int)&irq_empty;
	}
	BF_CS1(ICOLL_VBASE,TABLE_ADDRESS,0x00000100);
	
	
	
	HW_ICOLL_CTRL_CLR(BM_ICOLL_CTRL_SFTRST | BM_ICOLL_CTRL_CLKGATE |
                      BM_ICOLL_CTRL_BYPASS_FSM | BM_ICOLL_CTRL_NO_NESTING);
	
	HW_ICOLL_CTRL_SET(//BM_ICOLL_CTRL_FIQ_FINAL_ENABLE |
                      BM_ICOLL_CTRL_IRQ_FINAL_ENABLE |
                      BM_ICOLL_CTRL_ARM_RSE_MODE);
	
}

void irq_init(){
	irq_install();
	exception_install(EXCEPTION_IRQ,(unsigned int*)__irq_service);
	
	
}

