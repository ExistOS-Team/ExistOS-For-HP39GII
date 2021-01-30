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

#include "console.h"
#include "display.h"
#include "exception.h"
#include "hw_irq.h"
#include "irq.h"
#include "keyboard.h"
#include "memory.h"
#include "memory_map.h"
#include "mmu.h"
#include "regsapbh.h"
#include "regsclkctrl.h"
#include "regsdigctl.h"
#include "regsicoll.h"
#include "regslcdif.h"
#include "regspinctrl.h"
#include "regsuartdbg.h"
#include "rtc.h"
#include "uart_debug.h"
#include "utils.h"
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

extern unsigned char key_matrix[5][11];

extern int main();
extern void fs_test_main();

extern unsigned int SVC_STACK_ADDR;

void set_stack(unsigned int *newstackptr) __attribute__((naked));

void _boot();

volatile unsigned int HEAP_MEMORY_COARSE_TABLE[256] __attribute__((aligned(0x400))) __attribute__((section(".kernel_heap_memory")));

//at 0x00000000
volatile void _kernel_init() __attribute__((section(".init"))) __attribute__((naked));
volatile void _kernel_init() {

    volatile unsigned int *tlb_base = (unsigned int *)0x800C0000;

    BF_CS1n(DIGCTL_MPTEn_LOC, 0, LOC, 4000);
    BF_CS1n(DIGCTL_MPTEn_LOC, 1, LOC, 4001);
    BF_CS1n(DIGCTL_MPTEn_LOC, 2, LOC, 4002);
    BF_CS1n(DIGCTL_MPTEn_LOC, 3, LOC, 4003);
    BF_CS1n(DIGCTL_MPTEn_LOC, 4, LOC, 4004); //将暂时用不到的一级页表项移动到页表尾部

    BF_CS1n(DIGCTL_MPTEn_LOC, 5, LOC, 0x0);                          //用于暂时物理内存映射到高处  
    BF_CS1n(DIGCTL_MPTEn_LOC, 6, LOC, 4005);                         //用于把异常向量表映射到虚拟地址空间尾部
    BF_CS1n(DIGCTL_MPTEn_LOC, 7, LOC, RAM_START_VIRT_ADDR >> 20);    //用于映射SRAM空间的一级页表项

    MMU_MAP_SECTION_DEV(0x00000000, 0x00000000);          //映射物理地址0x00000000 到虚拟地址 0x00000000
    MMU_MAP_SECTION_DEV(0x00000000, RAM_START_VIRT_ADDR); //映射物理地址0x00000000 到虚拟地址 RAM_START_VIRT_ADDR   (高端位置)

    //MMU_MAP_COARSE_RAM(((unsigned int)&HEAP_MEMORY_COARSE_TABLE), KHEAP_MEMORY_VIR_START); //映射虚拟地址中内核堆内存区域到二级页表上

    //for (unsigned int i = 0; i < 256 * 1024; i += 4 * 1024)
    //    MMU_MAP_SMALL_PAGE_CACHED(KHEAP_MAP_PHY_START + i, KHEAP_MEMORY_VIR_START + i); //填写二级页表，确定虚拟内存地址中内核堆内存区所映射到的物理内存位置

    register unsigned int value;
    value = 0;
    asm volatile("mcr p15, 0, %0, c7, c5, 0"
                 :
                 : "r"(value)); //清除I Cache内容
    register unsigned int counter asm("r2");
    register unsigned int cacheaddr asm("r3");

    counter = 0;
    while (counter < 512) {
        cacheaddr = ((counter >> 1) & 0xe0) | ((counter & 63) << 26);
        // CLEAN AND INVALIDATE ENTRY USING INDEX
        asm volatile("mcr p15, 0, %0, c7, c14, 2"
                     :
                     : "r"(cacheaddr)); //清除D Cache
        ++counter;
    }

    value = 0;
    asm volatile("mcr p15, 0, %0, c8, c7, 0"
                 :
                 : "r"(value));

    asm volatile("ldr r0,=0x800C0000");
    asm volatile("mcr p15,0,r0,c2,c0,0"); // WRITE MMU BASE REGISTER, ALL CACHES SHOULD'VE BEEN CLEARED BEFORE

    asm volatile("mvn r0,#0");
    asm volatile("mcr p15,0,r0,c3,c0,0"); // SET R/W ACCESS PERMISSIONS FOR ALL DOMAINS

    asm volatile("mrc p15, 0, r0, c1, c0, 0");
    asm volatile("orr r0,r0,#1"); // Enable MMU

    asm volatile("orr r0,r0,#5");      // ENABLE MMU AND DATA CACHES
    asm volatile("orr r0,r0,#0x1000"); // ENABLE INSTRUCTION CACHE

    asm volatile("mcr p15, 0, r0, c1, c0, 0");

    asm volatile("mov r0,r0"); // NOP INSTRUCTIONS THAT ARE FETCHED FROM PHYSICAL ADDRESS
    asm volatile("mov r0,r0");

    asm volatile("b _boot");
}


//unsigned int save_1;
//at 0xC0000000
void _boot() {
    volatile unsigned int *tlb_base = (unsigned int *)0x800C0000;
    HW_UARTDBGDR_WR('d');
    disable_interrupts(); //关闭所有中断

    stack_init();               //栈初始化（设定异常、系统、中断等模式下的堆栈
    switch_mode(SVC_MODE);      //切换到系统管理模式
    set_stack(&SVC_STACK_ADDR); //设置系统管理模式下的栈地址
    exception_init();           //初始化异常向量
    irq_init();                 //初始化中断

    keyboard_init(); //键盘初始化
    uartdbg_printf("next boot 1.\n");
    uartdbg_printf("test 1.\n");
    printf("Starting Kernel...\n");
/*
    save_1 = 0x23231234;
    asm volatile("ldr r0,%0" : "=m"(save_1));
    asm volatile("ldr r0,[r0]");
    asm volatile("str pc,%0" : "=m"(save_1));
    printf("save:%08x\n",save_1);
*/
    MMU_UNMAP_SECTION_VIRT_RAM(0); //过河拆桥
    BF_WRn(DIGCTL_MPTEn_LOC, 5, LOC, 4005);
    flush_tlb();
    main(); //进入内核

    printf("System halt.\n");
    while (1)
        ;
}
