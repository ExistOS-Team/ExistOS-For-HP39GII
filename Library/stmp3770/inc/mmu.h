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
#include <regsdigctl.h>
#include <stdint.h>

#define MMU_SECTION_RAM(a) (((a)&0xfff00000) | 0xc1e)
#define MMU_SECTION_DEV(a) (((a)&0xfff00000) | 0xc12)
#define MMU_COARSE_PAGE(a) (((a)&0xfffffc00) | 0x011)

#define MMU_SMALL_PAGE_CACHED(a) (((a)&0xfffff000) | 0xffe)
#define MMU_SMALL_PAGE_CACHED_RO(a) (((a)&0xfffff000) | 0xaae)
#define MMU_SMALL_PAGE_NONCACHED(a) (((a)&0xfffff000) | 0xff2)
#define MMU_SMALL_PAGE_UNMAP (0)

#define MMU_LEVEL1_INDEX(virt) (((virt) >> 20) & 0xfff)
#define MMU_LEVEL2_INDEX(virt) (((virt) >> 12) & 0xff)

#define MMU_UNMAP_SECTION_VIRT_RAM(virt) (tlb_base[MMU_LEVEL1_INDEX(virt)] = 0)

#define MMU_MAP_SECTION_RAM(phys, virt) (tlb_base[MMU_LEVEL1_INDEX(virt)] = MMU_SECTION_RAM(phys))
#define MMU_MAP_SECTION_DEV(phys, virt) (tlb_base[MMU_LEVEL1_INDEX(virt)] = MMU_SECTION_DEV(phys))
#define MMU_MAP_COARSE_RAM(phys, virt) (tlb_base[MMU_LEVEL1_INDEX(virt)] = MMU_COARSE_PAGE(phys))

#define MMU_MAP_SMALL_PAGE_CACHED(phys, virt) (((unsigned int *)(tlb_base[MMU_LEVEL1_INDEX(virt)] & 0xfffffc00))[MMU_LEVEL2_INDEX(virt)] = MMU_SMALL_PAGE_CACHED(phys))
#define MMU_MAP_SMALL_PAGE_CACHED_WITH_L2(phys, virt, l2virt) (((unsigned int *)(l2virt))[MMU_LEVEL2_INDEX(virt)] = MMU_SMALL_PAGE_CACHED(phys))

#define MMU_MAP_SMALL_PAGE_CACHED_RO_WITH_L2(phys, virt, l2virt) (((unsigned int *)(l2virt))[MMU_LEVEL2_INDEX(virt)] = MMU_SMALL_PAGE_CACHED_RO(phys))

#define MMU_MAP_SMALL_PAGE_NONCACHED(phys, virt) (((unsigned int *)(tlb_base[MMU_LEVEL1_INDEX(virt)] & 0xfffffc00))[MMU_LEVEL2_INDEX(virt)] = MMU_SMALL_PAGE_NONCACHED(phys))
#define MMU_MAP_SMALL_PAGE_NONCACHED_WITH_L2(phys, virt, l2virt) (((unsigned int *)(l2virt))[MMU_LEVEL2_INDEX(virt)] = MMU_SMALL_PAGE_NONCACHED(phys))

#define MMU_MAP_SMALL_PAGE_UNMAP(virt) (((unsigned int *)(tlb_base[MMU_LEVEL1_INDEX(virt)] & 0xfffffc00))[MMU_LEVEL2_INDEX(virt)] = MMU_SMALL_PAGE_UNMAP)
#define MMU_MAP_SMALL_PAGE_UNMAP_WITH_L2(virt, l2virt) (((unsigned int *)(l2virt))[MMU_LEVEL2_INDEX(virt)] = MMU_SMALL_PAGE_UNMAP)

//#define VIR_TO_PHY_ADDR(virt)		((unsigned int *)(((((unsigned int *)(tlb_base[((((unsigned int)(virt))>>20)&0xFFF)] &0xfffffc00 ))[(((unsigned int)(virt))>>12)&0xFF]) & 0xfffff000) | ((unsigned int)(virt) & 0xFFF)))

//((((unsigned int *)(tlb_base[MMU_LEVEL1_INDEX((unsigned int)(virt))]&0xfffffc00))[MMU_LEVEL2_INDEX((unsigned int)(virt))]) & 0xfffff000 |(((unsigned int)(virt))&0xfffff000))

#define USER_MODE 0x10
#define FIQ_MODE 0x11
#define IRQ_MODE 0x12
#define SVC_MODE 0x13
#define ABT_MODE 0x17
#define UND_MODE 0x1b
#define SYS_MODE 0x1f

//#define FIRST_LEVEL_PAGE_TABLE_BASE		0x800C0000	//一级页表基址

#ifdef __cplusplus
extern "C" {
#endif

void mmu_set_RS(unsigned int RS);
void mmu_set_domain_control_bit(unsigned int domain, unsigned int controlBit);

//一级页表初始化
void DFLTP_init(void);
//栈初始化
void stack_init(void);
void stack_relocate();

//模式切换
void switch_mode(int mode) __attribute__((naked));
//获取模式
unsigned int get_mode(void);

//开/关 MMU
void disable_mmu(void);
void enable_mmu(void);

//设置当前模式下的栈地址（即R13寄存器）
void set_stack(unsigned int *) __attribute__((naked));

void flush_cache();
volatile void flush_tlb();


uint8_t *VIR_TO_PHY_ADDR(uint8_t *VIRT_ADDR);

#ifdef __cplusplus
}
#endif
#endif