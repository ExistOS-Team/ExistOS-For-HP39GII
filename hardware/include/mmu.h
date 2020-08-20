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
 #pragma once
#ifndef _MMU_H
#define _MMU_H

#define MMU_SECTION_RAM(a) (((a)&0xfff00000)| 0xc1e)	
#define MMU_SECTION_ROM(a) (((a)&0xfff00000)| 0x01e)	
#define MMU_SECTION_DEV(a) (((a)&0xfff00000)| 0xc12)	
#define MMU_PAGES_RAM(a)   (((a)&0xfffffc00)| 0x011)	

#define MMU_PAGE(a)			      (((a)&0xfffff000)|0xffe)	
#define MMU_PAGE_NOT_IN_MEMORY(a) (((a)&0xfffff000)|0x00e)	

#define MMU_LEVEL1_INDEX(virt) (((virt)>>20)&0xfff)
#define MMU_LEVEL2_INDEX(virt) (((virt)>>12)&0xff)


#define MMU_MAP_SECTION_ROM(phys,virt) (mmu_base[MMU_LEVEL1_INDEX(virt)]=MMU_SECTION_ROM(phys))//映射一个1MB的内存段（带Chche和Buffer
#define MMU_MAP_SECTION_RAM(phys,virt) (mmu_base[MMU_LEVEL1_INDEX(virt)]=MMU_SECTION_RAM(phys))//映射一个1MB的ROM段 （带Chche和Buffer，只读，写入会产生异常
#define MMU_MAP_SECTION_DEV(phys,virt) (mmu_base[MMU_LEVEL1_INDEX(virt)]=MMU_SECTION_DEV(phys))//映射一个1MB的设备内存段（无ChChe无Buffer 
#define MMU_MAP_COARSE_RAM(phys,virt)  (mmu_base[MMU_LEVEL1_INDEX(virt)]=MMU_PAGES_RAM(phys))  //二级页表的映射

#define MMU_MAP_PAGE_IN_MEMORY(phys,virt) 	  ( ( (unsigned int *)(mmu_base[MMU_LEVEL1_INDEX(virt)]&0xfffffc00))[MMU_LEVEL2_INDEX(virt)]=MMU_PAGE(phys))	//用于一个4K页的映射，页面且在内存中
#define MMU_MAP_PAGE_NOT_IN_MEMORY(phys,virt) ( ( (unsigned int *)(mmu_base[MMU_LEVEL1_INDEX(virt)]&0xfffffc00))\
												[MMU_LEVEL2_INDEX(virt)]=MMU_PAGE_NOT_IN_MEMORY(phys) 
												//用于一个4K页的映射，页面且不在内存中 )


#define USER_MODE 0x10
#define FIQ_MODE  0x11
#define IRQ_MODE  0x12
#define SVC_MODE  0x13
#define ABT_MODE  0x17
#define UND_MODE  0x1b
#define SYS_MODE  0x1f

#define FIRST_LEVEL_PAGE_TABLE_BASE		0x800C0000	//一级页表基址
#define SECOUND_LEVEL_PAGE_TABLE_FOR_KERNEL		0x00000400	//存放用于内核的二级页表

#define KERNEL_ADDR_BASE						0x90000000	//内核的内存基址

#ifdef __cplusplus 
extern "C" { 
#endif
//一级页表初始化
void DFLTP_init(void);
//栈初始化
void stack_init(void);
//模式切换
void switch_mode(int mode) __attribute__ ((naked));
//获取模式
unsigned int get_mode(void);

//开/关 MMU
void disable_mmu(void);
void enable_mmu(void);

//设置当前模式下的栈地址（即R13寄存器）
void set_stack(unsigned int *) __attribute__ ((naked));
#ifdef __cplusplus 
} 
#endif
#endif

