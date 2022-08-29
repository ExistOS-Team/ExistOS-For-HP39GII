
#ifndef __MMU_H__
#define __MMU_H__

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>


#define L1PTE_INTERPRET_INVALID     (0x0)
#define L1PTE_INTERPRET_COARSE      (0x1)
#define L1PTE_INTERPRET_SECTION     (0x2)
#define L1PTE_INTERPRET_FINE        (0x3)

#define AP_READONLY                 (0)
#define AP_SYSRW_USRNONE            (1)
#define AP_SYSRW_USROR              (2)
#define AP_SYSRW_USRRW              (3)


#define HARDWARE_MEMORY_DOMAIN      0
#define OSLOADER_MEMORY_DOMAIN      1
#define VM_RAM_DOMAIN               2
#define VM_ROM_DOMAIN               3


void mmu_init(void);

void mmu_map_page(
    uint32_t vaddr, 
    uint32_t paddr,
    uint32_t AP, 
    bool cache, 
    bool buffer
    );

void mmu_unmap_page(uint32_t vaddr);

void mmu_clean_invalidated_cache_index(uint32_t index);
void mmu_clean_invalidated_dcache(uint32_t buffer, uint32_t size);
void mmu_clean_dcache(uint32_t buffer, uint32_t size);
void mmu_invalidate_dcache(uint32_t buffer, uint32_t size);
void mmu_invalidate_tlb(void);
void mmu_invalidate_icache(void);
void mmu_invalidate_dcache_all(void);
void mmu_drain_buffer(void);

void mmu_dumpMapInfo(void);


#endif

