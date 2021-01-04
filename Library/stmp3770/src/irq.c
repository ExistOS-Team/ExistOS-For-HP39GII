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
 #include "regsuartdbg.h"
#include "regsicoll.h"
#include "exception.h"
#include "uart_debug.h"
#include "irq.h"

unsigned int *irq_vector_table_base = (unsigned int *)IRQ_VECTOR_TABLE_BASE_ADDR;	//中断向量表基址
volatile unsigned int current_irq_number;		//当前发生中断的中断号

void irq_dummy(void){					//没有注册但是发生的中断会跳转到这里执行
	uartdbg_printf("invalid IRQ %x\n", HW_ICOLL_STAT.B.VECTOR_NUMBER);
	for(unsigned int i=0;i<64;i++)
		uartdbg_printf("IRQ%x %x\n", i, *((unsigned int *)HW_ICOLL_PRIORITYn_ADDR((i/4))));
	while(1);
}

void __fiq_service() __attribute__ ((naked));
void __fiq_service() {					//FIQ中断暂不支持
	
	uartdbg_printf("unsupposed FIQ.\n");
	uartdbg_print_regs();
	while(1);
	
}



unsigned char irq_get_irq_priority(unsigned int irq_n) 
{
	return (*((volatile unsigned int *)HW_ICOLL_PRIORITYn_ADDR(irq_n/4)) >> (8*(irq_n % 4))) & 0x03;
}

void enable_interrupts()  __attribute__ ((naked));
void enable_interrupts() 
{				
	//asm volatile ("push {r1}");
	asm volatile ("mrs r1, cpsr_all");
	asm volatile ("bic r1, r1, #0xc0");
	asm volatile ("msr cpsr_all, r1");
	//asm volatile ("pop {r1}");
}


void disable_interrupts()  __attribute__ ((naked));
void disable_interrupts() 
{				

	//HW_UARTDBGDR_WR('f');
	
	//while(1);
	//asm volatile ("push {r1}");
	asm volatile ("mrs r1, cpsr_all");
	asm volatile ("orr r1, r1, #0xc0");
	asm volatile ("msr cpsr_all, r1");
	//asm volatile ("pop {r1}");
}


void irq_set_enable(unsigned int irq_n, unsigned int enable) {
	if(irq_n > 63)return;
	unsigned int volatile *baseAddress = (unsigned int *)HW_ICOLL_PRIORITYn_ADDR((irq_n/4));
	if(enable)
		*(baseAddress + 1) = (0x4<<((irq_n%4)*8));
	else
		*(baseAddress + 2) = (0x4<<((irq_n%4)*8));

}

void irq_install_service(unsigned int irq_n, unsigned int *service_program) {
	irq_vector_table_base[irq_n] = (unsigned int )service_program;
}


void __irq_service() __attribute__ ((naked));
void volatile __irq_service() {
	
	//asm volatile ("subs lr, lr, #4");			//计算中断结束后应返回的地址
	//asm volatile ("stmdb sp!, {r0-r12, lr} ");	//保存现场
	
	//
	//disable_interrupts();						//禁止外部中断
    //asm volatile ("mrs r1, cpsr_all");
    //asm volatile ("orr r1, r1, #0x1f");
	//asm volatile ("msr cpsr_all, r1");			//切换到系统管理模式
	
	
	portSAVE_CONTEXT_ASM;

	//调用中断服务函数
	
	//current_irq_number = HW_ICOLL_STAT.B.VECTOR_NUMBER;										//获取当前发生的中断号
	//BF_SETV(ICOLL_LEVELACK, IRQLEVELACK, 1<<(irq_get_irq_priority(current_irq_number)));		//通知中断优先级控制器已经响应当前中断	
	
		
	//irq_set_enable(current_irq_number , 0);														//关闭当前发生的中断，在对应的中断服务程序中再打开	
	//if(current_irq_number < 63)																	//判断是有效的中断
	//uartdbg_printf("irq:%d\n",BF_RD(ICOLL_VECTOR, IRQVECTOR));
		//printf("\n%08x %08x\n",BF_RD(ICOLL_VECTOR, IRQVECTOR) , irq_vector_table_base[HW_ICOLL_STAT.B.VECTOR_NUMBER]);
		
		//while(1);
		//(*(void(*)(void))( *((unsigned int *)BF_RD(ICOLL_VECTOR, IRQVECTOR)) )) ();
		(*(void(*)(void))((unsigned int)irq_vector_table_base[HW_ICOLL_STAT.B.VECTOR_NUMBER])) ();				//调用对应的中断服务程序
	
	
	BF_SETV(ICOLL_VECTOR, IRQVECTOR, BF_RD(ICOLL_VECTOR, IRQVECTOR));							//通知中断控制器已经响应当前中断	
	
	BF_SETV(ICOLL_LEVELACK, IRQLEVELACK, 1<<(
			(*((volatile unsigned int *)HW_ICOLL_PRIORITYn_ADDR(HW_ICOLL_STAT.B.VECTOR_NUMBER/4)) >> (8*(HW_ICOLL_STAT.B.VECTOR_NUMBER % 4))) & 0x03
		));		//通知中断优先级控制器已经响应当前中断	
	
	portRESTORE_CONTEXT_ASM;

	//asm volatile ("mrs r1, cpsr_all");
    //asm volatile ("bic r1, r1, #0xd");
    //asm volatile ("msr cpsr_all, r1");  		//切换回中断模式
	//enable_interrupts();						//允许中断
    
	//asm volatile ("ldmia sp!, {r0-r12, pc}^");  //中断返回，^表示将spsr的值复制到cpsr
}


void irq_init() {		//中断系统的初始化

	for(unsigned int i=0;i<64;i++) {	//禁用所有中断源
		irq_set_enable(i, 0);	
	}

	for(unsigned int i=0;i<64;i++) {	//将所有的中断服务函数指向预置函数
		irq_install_service(i, (unsigned int *)irq_dummy);
	}
	exception_install(EXCEPTION_IRQ, (unsigned int*)__irq_service);
	exception_install(EXCEPTION_FIQ, (unsigned int*)__fiq_service);
	
	
	

	
	
	HW_ICOLL_CTRL_CLR(BM_ICOLL_CTRL_CLKGATE);
	HW_ICOLL_CTRL_SET(BM_ICOLL_CTRL_SFTRST);
	uartdbg_printf("init irq.\n");
	HW_ICOLL_CTRL_CLR(BM_ICOLL_CTRL_CLKGATE);
	while(BF_RD(ICOLL_CTRL,CLKGATE))
		{
			;    //等待重置完成
		}
	//HW_ICOLL_CTRL_CLR(BM_ICOLL_CTRL_SFTRST);
	
	
	HW_ICOLL_CTRL_CLR(BM_ICOLL_CTRL_SFTRST | BM_ICOLL_CTRL_CLKGATE |
                      BM_ICOLL_CTRL_BYPASS_FSM | BM_ICOLL_CTRL_NO_NESTING | BM_ICOLL_CTRL_ARM_RSE_MODE);
	
	HW_ICOLL_CTRL_SET(BM_ICOLL_CTRL_FIQ_FINAL_ENABLE |
                      BM_ICOLL_CTRL_IRQ_FINAL_ENABLE |
                      BM_ICOLL_CTRL_ARM_RSE_MODE |
					  BM_ICOLL_CTRL_NO_NESTING
					  );
	BF_CS1(ICOLL_VBASE, TABLE_ADDRESS, IRQ_VECTOR_TABLE_BASE_ADDR);		//设置中断控制器内部的中断向量表地址
	BW_ICOLL_CTRL_VECTOR_PITCH(BV_ICOLL_CTRL_VECTOR_PITCH__BY16);
	
	uartdbg_printf("irq inited.\n");
}

