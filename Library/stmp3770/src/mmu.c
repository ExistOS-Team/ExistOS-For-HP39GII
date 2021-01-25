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

#include "mmu.h"
#include "memory_map.h"
volatile unsigned int *tlb_base = (unsigned int *)0x800C0000; //一级页表基地址

volatile void mmu_set_RS(unsigned int RS) {                //设置ARM CP15 C1寄存器的RS位
    register unsigned int c1_r asm("r1");                  //定义变量c1_r为一个寄存器变量，使用r1寄存器
    asm volatile("mrc p15, 0, %0, c1, c0, 0" ::"r"(c1_r)); //CP15协处理器里的c1寄存器
    c1_r &= 0xFFFFFCFF;
    c1_r |= (RS & 0x3) << 8;
    asm volatile("mcr p15, 0, %0, c1, c0, 0" ::"r"(c1_r));
}

volatile void mmu_set_domain_control_bit(unsigned int domain, unsigned int controlBit) {
    register unsigned int c3_r asm("r2");
    asm volatile("mrc p15, 0, %0, c3, c0, 0" ::"r"(c3_r));
    c3_r &= ~((0x3) << (domain * 2));
    c3_r |= ((controlBit & 0x3) << domain * 2);
    asm volatile("mcr p15, 0, %0, c3, c0, 0" ::"r"(c3_r));
}

uint8_t *VIR_TO_PHY_ADDR(uint8_t *VIRT_ADDR) {
    uint8_t *result = VIRT_ADDR;
    unsigned int COARSE_ADDR;

    //printf("search virt %08x\n", VIRT_ADDR);

    switch (tlb_base[((((unsigned int)(VIRT_ADDR)) >> 20) & 0xFFF)] & 3) {
    case 1:
        COARSE_ADDR = tlb_base[((((unsigned int)(VIRT_ADDR)) >> 20) & 0xFFF)] & 0xfffffc00;

        if (COARSE_ADDR < TOTAL_PHY_MEMORY) {

            COARSE_ADDR += RAM_START_VIRT_ADDR;
            //uartdbg_printf("COARSE_ADDR %x\n",COARSE_ADDR);
        }

        if ((((unsigned int *)(COARSE_ADDR))[(((unsigned int)(VIRT_ADDR)) >> 12) & 0xFF]) & 0xfffff000 == 0) {
            return 0x0;
        }

        result = (uint8_t *)((unsigned int *)(((((unsigned int *)(COARSE_ADDR))[(((unsigned int)(VIRT_ADDR)) >> 12) & 0xFF]) & 0xfffff000) | ((unsigned int)(VIRT_ADDR)&0xFFF)));
        break;
    case 2:
        result = (uint8_t *)((tlb_base[((unsigned int)VIRT_ADDR) >> 20] & 0xFFF00000) | ((unsigned int)VIRT_ADDR & 0x000FFFFF));
        //uartdbg_printf("SECTION ADDR %x\n",result);
        break;
    }

    //printf("result %08x\n" , result);

    return result;
}

//设置栈指针
void set_stack(unsigned int *newstackptr) __attribute__((naked));
void set_stack(unsigned int *newstackptr) {
    asm volatile("mov sp,r0");
    asm volatile("bx lr");
}

//设置运行模式
void switch_mode(int mode) __attribute__((naked));
void switch_mode(int mode) {
    asm volatile("and r0,r0,#0x1f");
    asm volatile("mrs r1,cpsr_all");
    asm volatile("bic r1,r1,#0x1f");
    asm volatile("orr r1,r1,r0");
    asm volatile("mov r0,lr"); // GET THE RETURN ADDRESS **BEFORE** MODE CHANGE
    asm volatile("msr cpsr_all,r1");
    asm volatile("bx r0");
}

//获取运行模式
unsigned int get_mode() {
    register unsigned int cpsr;

    asm volatile("mrs %0,cpsr_all"
                 : "=r"(cpsr));

    return cpsr & 0x1f;
}

static void __disable_mmu() {

    asm volatile("mrc p15, 0, r0, c1, c0, 0");
    asm volatile("bic r0,r0,#1");    // disable MMU
    asm volatile("bic r0,r0,#1000"); // disable INSTRUCTION CACHE
    asm volatile("bic r0,r0,#4");    // disable DATA CACHE
    asm volatile("mcr p15, 0, r0, c1, c0, 0");

    asm volatile("mov r0,r0");
    asm volatile("mov r0,r0");
}

void __enable_mmu(unsigned int *base) {
    //r0 = base
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
}

volatile static void __flush_Dcache(void) {
    register unsigned int counter asm("r2");
    register unsigned int cacheaddr asm("r3");

    counter = 0;
    while (counter < 512) {
        cacheaddr = ((counter >> 1) & 0xe0) | ((counter & 63) << 26);
        // CLEAN AND INVALIDATE ENTRY USING INDEX
        asm volatile("mcr p15, 0, %0, c7, c14, 2"
                     :
                     : "r"(cacheaddr));
        ++counter;
    }
}

volatile static void __flush_Icache(void) {
    // CLEAN AND INVALIDATE ENTRY USING INDEX
    register unsigned int value;
    value = 0;
    asm volatile("mcr p15, 0, %0, c7, c5, 0"
                 :
                 : "r"(value));
}

volatile static void __flush_TLB(void) {
    // CLEAN AND INVALIDATE ENTRY USING INDEX
    register unsigned int value;
    value = 0;
    asm volatile("mcr p15, 0, %0, c8, c7, 0"
                 :
                 : "r"(value));
}

void enable_mmu() {

    __flush_Dcache();
    __flush_Icache();
    __flush_TLB();
    //__enable_mmu(FIRST_LEVEL_PAGE_TABLE_BASE);
    __enable_mmu((unsigned int *)tlb_base);
}

extern unsigned int ABT_STACK_ADDR;
extern unsigned int UND_STACK_ADDR;
extern unsigned int FIQ_STACK_ADDR;
extern unsigned int IRQ_STACK_ADDR;
extern unsigned int SYS_STACK_ADDR;
extern unsigned int SVC_STACK_ADDR;

void stack_init() {

    switch_mode(ABT_MODE);
    set_stack(&ABT_STACK_ADDR);

    switch_mode(UND_MODE);
    set_stack(&UND_STACK_ADDR);

    switch_mode(FIQ_MODE);
    set_stack(&FIQ_STACK_ADDR);

    switch_mode(IRQ_MODE);
    set_stack(&IRQ_STACK_ADDR);

    switch_mode(SYS_MODE);
    set_stack(&SYS_STACK_ADDR);

    switch_mode(SVC_MODE);
    asm volatile("nop");
}

void stack_relocate() {
    switch_mode(ABT_MODE);
    set_stack((&ABT_STACK_ADDR) + RAM_START_VIRT_ADDR);

    switch_mode(UND_MODE);
    set_stack((&UND_STACK_ADDR) + RAM_START_VIRT_ADDR);

    switch_mode(FIQ_MODE);
    set_stack((&FIQ_STACK_ADDR) + RAM_START_VIRT_ADDR);

    switch_mode(IRQ_MODE);
    set_stack((&IRQ_STACK_ADDR) + RAM_START_VIRT_ADDR);

    switch_mode(SYS_MODE);
    set_stack((&SYS_STACK_ADDR) + RAM_START_VIRT_ADDR);

    switch_mode(SVC_MODE);
    asm volatile("nop");
}

void disable_mmu() {
    __flush_Dcache();
    __flush_Icache();
    __flush_TLB();
    __disable_mmu();
}

volatile void flush_cache() {
    asm volatile("tci_loop: MRC p15, 0, r15, c7, c14, 3 "); // test clean and invalidate
    asm volatile("BNE tci_loop");
}

volatile void flush_tlb() {
    // CLEAN AND INVALIDATE ENTRY USING INDEX
    register unsigned int value;
    value = 0;
    asm volatile("mcr p15, 0, %0, c8, c7, 0"
                 :
                 : "r"(value));
}
