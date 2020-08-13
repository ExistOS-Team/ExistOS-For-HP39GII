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
 
#include <regsdigctl.h>
#include <mmu.h>



void DFLTP_init(){
	unsigned int *mmu_base=(unsigned int *)FIRST_LEVEL_PAGE_TABLE_FOR_KERNEL;	//一级页表基地址
	
	for(unsigned char *ptr = (unsigned char*)(SECOUND_LEVEL_PAGE_TABLE_FOR_KERNEL);ptr < (unsigned char *)(SECOUND_LEVEL_PAGE_TABLE_FOR_KERNEL + 0x800);ptr++) {	//清空二级页表
		*ptr = 0;
	}
	
	BF_WRn(DIGCTL_MPTEn_LOC,0,LOC,0x0);											//SoC内部自带的硬件一级页表
	BF_WRn(DIGCTL_MPTEn_LOC,1,LOC,KERNEL_ADDR_BASE >> 20);	
	BF_WRn(DIGCTL_MPTEn_LOC,2,LOC,0x2);
	BF_WRn(DIGCTL_MPTEn_LOC,3,LOC,0x3);
	BF_WRn(DIGCTL_MPTEn_LOC,4,LOC,0x4);
	BF_WRn(DIGCTL_MPTEn_LOC,5,LOC,0x5);
	BF_WRn(DIGCTL_MPTEn_LOC,6,LOC,0x6);
	BF_WRn(DIGCTL_MPTEn_LOC,7,LOC,0x7);
	
	MMU_MAP_SECTION_DEV(0x00000000,0x00000000);										//内存开头1MB映射
	MMU_MAP_COARSE_RAM(SECOUND_LEVEL_PAGE_TABLE_FOR_KERNEL,KERNEL_ADDR_BASE);		//0x90000000处的内核内存
	
	for(unsigned int i = 0;i<384*1024;i+=4*1024)									//填写二级页表
		MMU_MAP_PAGE_IN_MEMORY(0x00020000 + i,KERNEL_ADDR_BASE + i);				//384个页，每个页4KB
} 

//设置栈指针
void set_stack(unsigned int *newstackptr)
{
	asm volatile ("mov sp,r0");
	asm volatile ("bx lr");
}


//设置运行模式
void switch_mode(int mode)
{
	asm volatile ("and r0,r0,#0x1f");
	asm volatile ("mrs r1,cpsr_all");
	asm volatile ("bic r1,r1,#0x1f");
	asm volatile ("orr r1,r1,r0");
	asm volatile ("mov r0,lr");         // GET THE RETURN ADDRESS **BEFORE** MODE CHANGE
	asm volatile ("msr cpsr_all,r1");
	asm volatile ("bx r0");
}

//获取运行模式
unsigned int get_mode()
{
	register unsigned int cpsr;

	asm volatile ("mrs %0,cpsr_all" : "=r" (cpsr) );

	return cpsr&0x1f;
}

static void __disable_mmu(){
	
	asm volatile ("mrc p15, 0, r0, c1, c0, 0");
	asm volatile ("bic r0,r0,#1"); 				// disable MMU
	asm volatile ("bic r0,r0,#1000"); 			// disable INSTRUCTION CACHE
	asm volatile ("bic r0,r0,#4"); 				// disable DATA CACHE
	asm volatile ("mcr p15, 0, r0, c1, c0, 0");
	
	asm volatile ("mov r0,r0"); 	
	asm volatile ("mov r0,r0");	
	
}

void __enable_mmu()
{

	asm volatile ("mov r0,#0x80000000");
	asm volatile ("add r0,r0,#0xC0000");
	asm volatile ("mcr p15,0,r0,c2,c0,0");      // WRITE MMU BASE REGISTER, ALL CACHES SHOULD'VE BEEN CLEARED BEFORE

	asm volatile ("mvn r0,#0");
	asm volatile ("mcr p15,0,r0,c3,c0,0");      // SET R/W ACCESS PERMISSIONS FOR ALL DOMAINS
	
	asm volatile ("mrc p15, 0, r0, c1, c0, 0");
	asm volatile ("orr r0,r0,#1"); 				// Enable MMU
	
	asm volatile ("orr r0,r0,#5");              // ENABLE MMU AND DATA CACHES
	asm volatile ("orr r0,r0,#0x1000");         // ENABLE INSTRUCTION CACHE

	asm volatile ("mcr p15, 0, r0, c1, c0, 0");

	asm volatile ("mov r0,r0");                 // NOP INSTRUCTIONS THAT ARE FETCHED FROM PHYSICAL ADDRESS
	asm volatile ("mov r0,r0");


}


static void __flush_Dcache(void)
{
	register unsigned int counter asm("r2");
	register unsigned int cacheaddr asm("r3");

	counter=0;
	while(counter<512)
		{
			cacheaddr=((counter>>1)&0xe0) | ((counter&63)<<26);
			// CLEAN AND INVALIDATE ENTRY USING INDEX
			asm volatile ("mcr p15, 0, %0, c7, c14, 2" : : "r" (cacheaddr));
			++counter;
		}

}

static void __flush_Icache(void)
{
	// CLEAN AND INVALIDATE ENTRY USING INDEX
	register unsigned int value;
	value=0;
	asm volatile ("mcr p15, 0, %0, c7, c5, 0" : : "r" (value));
}

static void __flush_TLB(void)
{
	// CLEAN AND INVALIDATE ENTRY USING INDEX
	register unsigned int value;
	value=0;
	asm volatile ("mcr p15, 0, %0, c8, c7, 0" : : "r" (value));

}

void enable_mmu()
{

	__flush_Dcache();
	__flush_Icache();
	__flush_TLB();

	__enable_mmu();

}


void stack_init(){
		
    switch_mode(ABT_MODE);
    set_stack((unsigned int *)0x0007FD00);
    
	switch_mode(UND_MODE);
    set_stack((unsigned int *)0x0007FB00);
    
	switch_mode(FIQ_MODE);
    set_stack((unsigned int *)0x0007F900);
	
    switch_mode(IRQ_MODE);
    set_stack((unsigned int *)0x0007F500);
    
	switch_mode(SYS_MODE);
	set_stack((unsigned int *)0x0007F700);
	
	switch_mode(SVC_MODE);
    asm volatile ("nop");       
    
}


void disable_mmu(){
	__flush_Dcache();
	__flush_Icache();
	__flush_TLB();
	__disable_mmu();
}

