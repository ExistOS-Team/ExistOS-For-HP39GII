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

#define MMU_SECTION_RAM(a) (((a)&0xfff00000)| 0xc1e)	//1MB PAGE
#define MMU_SECTION_ROM(a) (((a)&0xfff00000)| 0x01e)	//1MB PAGE
#define MMU_SECTION_DEV(a) (((a)&0xfff00000)| 0xc12)	//1MB PAGE

#define MMU_PAGES_RAM(a)   (((a)&0xfffffc00)| 0x011)	//0 - 3FF (0x400 * N)

#define MMU_PAGE(a)			      (((a)&0xfffff000)|0xffe)	//4K
#define MMU_PAGE_NOT_IN_MEMORY(a) (((a)&0xfffff000)|0x00e)	//4K

#define MMU_LEVEL1_INDEX(virt) (((virt)>>20)&0xfff)
#define MMU_LEVEL2_INDEX(virt) (((virt)>>12)&0xff)

#define MMU_MAP_SECTION_ROM(phys,virt) (mmu_base[MMU_LEVEL1_INDEX(virt)]=MMU_SECTION_ROM(phys))
#define MMU_MAP_SECTION_RAM(phys,virt) (mmu_base[MMU_LEVEL1_INDEX(virt)]=MMU_SECTION_RAM(phys))
#define MMU_MAP_SECTION_DEV(phys,virt) (mmu_base[MMU_LEVEL1_INDEX(virt)]=MMU_SECTION_DEV(phys))
#define MMU_MAP_COARSE_RAM(phys,virt)  (mmu_base[MMU_LEVEL1_INDEX(virt)]=MMU_PAGES_RAM(phys))

#define MMU_MAP_PAGE_IN_MEMORY(phys,virt) 	  ( ( (unsigned int *)(mmu_base[MMU_LEVEL1_INDEX(virt)]&0xfffffc00))[MMU_LEVEL2_INDEX(virt)]=MMU_PAGE(phys))	//4K PAGE
#define MMU_MAP_PAGE_NOT_IN_MEMORY(phys,virt) ( ( (unsigned int *)(mmu_base[MMU_LEVEL1_INDEX(virt)]&0xfffffc00))[MMU_LEVEL2_INDEX(virt)]=MMU_PAGE_NOT_IN_MEMORY(phys))	//4K PAGE

#define SECOUNDRY_PAGE_TABLE_FOR_KERNEL		0x00000400

void DFLTP_init(){
	unsigned int *mmu_base=(unsigned int *)0x800C0000;		//一级页表基地址
	
	for(unsigned char *ptr = (unsigned char*)(SECOUNDRY_PAGE_TABLE_FOR_KERNEL);ptr < (unsigned char *)(SECOUNDRY_PAGE_TABLE_FOR_KERNEL + 0x800);ptr++)
		*ptr = 0;
	
	
	BF_WRn(DIGCTL_MPTEn_LOC,0,LOC,0x0);					//内存开头1MB映射
	BF_WRn(DIGCTL_MPTEn_LOC,1,LOC,0x90000000 >> 20);	//0x90000000处
	
	BF_WRn(DIGCTL_MPTEn_LOC,2,LOC,0x2);
	BF_WRn(DIGCTL_MPTEn_LOC,3,LOC,0x3);
	BF_WRn(DIGCTL_MPTEn_LOC,4,LOC,0x4);
	BF_WRn(DIGCTL_MPTEn_LOC,5,LOC,0x5);
	BF_WRn(DIGCTL_MPTEn_LOC,6,LOC,0x6);
	BF_WRn(DIGCTL_MPTEn_LOC,7,LOC,0x7);
	
	MMU_MAP_SECTION_DEV(0x00000000,0x00000000);
	MMU_MAP_COARSE_RAM(SECOUNDRY_PAGE_TABLE_FOR_KERNEL,0x90000000);
	
	for(unsigned int i = 0;i<384*1024;i+=4*1024)
		MMU_MAP_PAGE_IN_MEMORY(0x00020000 + i,0x90000000 + i);	//384个页，每个页4KB
} 

void switch_mode(int mode) __attribute__ ((naked));
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

void enable_interrupts()
{
	asm volatile ("mrs r1,cpsr_all");
	asm volatile ("bic r1,r1,#0xc0");
	asm volatile ("msr cpsr_all,r1");
}


void disable_interrupts()
{
	asm volatile ("mrs r1,cpsr_all");
	asm volatile ("orr r1,r1,#0xc0");
	asm volatile ("msr cpsr_all,r1");
}

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

void mmu_flush_DIT(){
	__flush_Dcache();
	__flush_Icache();
	__flush_TLB();
}

void enable_mmu()
{

	// WE MUST BE IN SUPERVISOR MODE ALREADY

	__flush_Dcache();
	__flush_Icache();
	__flush_TLB();

	__enable_mmu();

}

void disable_mmu(){
	__flush_Dcache();
	__flush_Icache();
	__flush_TLB();
	
	__disable_mmu();
	
}

