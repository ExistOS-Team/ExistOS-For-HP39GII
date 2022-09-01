
#include "FreeRTOS.h"
#include "task.h"

#include "regsdigctl.h"

#include "SystemConfig.h"
#include "mmu.h"
#include <stdbool.h>

#include "../debug.h"

//#define DFLPT_BASE  0x800C0000

#define L1PTE_NUM       (2049)

uint32_t L1PTE[L1PTE_NUM] __attribute__((aligned(16384)));  //Must 16KB aligned
uint32_t DFLPT_BASE = (uint32_t)L1PTE;

#if USE_TINY_PAGE
    uint32_t L2PTE[TOTAL_VM_SEG * 1024]  __attribute__((aligned(4096)));
    #define SEGn_L2TAB_BASE(x)    ((uint32_t)(&L2PTE[(x) * 1024]))
#else
    uint32_t L2PTE[TOTAL_VM_SEG * 256]  __attribute__((aligned(1024)));
    #define SEGn_L2TAB_BASE(x)    ((uint32_t)(&L2PTE[(x) * 256]))
#endif

#define VADDR_TO_L1SEGn(addr)   ((addr) >> 20)

#define GetPTEMapType(x)  ((x)&0x3)

#define CACHE_LINE_SIZE     32

void mmu_clean_invalidated_cache_index(uint32_t index)
{
    __asm volatile ("mcr p15, 0, %0, c7, c14, 2" :: "r"(index));
}

void mmu_clean_invalidated_dcache(uint32_t buffer, uint32_t size)
{
    register uint32_t ptr;

    ptr = buffer & ~(CACHE_LINE_SIZE - 1);

    while (ptr < buffer + size)
    {
        __asm volatile ( "MCR p15, 0, %0, c7, c14, 1" :: "r"(ptr) );
        ptr += CACHE_LINE_SIZE;
    }
}


void mmu_clean_dcache(uint32_t buffer, uint32_t size)
{
    register uint32_t ptr;

    ptr = buffer & ~(CACHE_LINE_SIZE - 1);

    while (ptr < buffer + size)
    {
        __asm volatile ("MCR p15, 0, %0, c7, c10, 1 " :: "r"(ptr) );
        ptr += CACHE_LINE_SIZE;
    }
}

void mmu_drain_buffer()
{
    register uint32_t reg;
    reg = 0;
    __asm volatile("mcr p15,0,%0,c7,c10,4" :: "r"(reg));


}

void mmu_invalidate_dcache(uint32_t buffer, uint32_t size)
{
    register uint32_t ptr;

    ptr = buffer & ~(CACHE_LINE_SIZE - 1);

    while (ptr < buffer + size)
    {
        __asm volatile ("MCR p15, 0, %0, c7, c6, 1" :: "r"(ptr) );
        ptr += CACHE_LINE_SIZE;
    }
}

void mmu_invalidate_tlb()
{
    register uint32_t value asm("r0");

    value = 0;
    __asm volatile ("mcr p15, 0, %0, c8, c7, 0" :: "r"(value) );

}

void mmu_invalidate_icache()
{
    register uint32_t value asm("r0");

    value = 0;

    __asm volatile ("mcr p15, 0, %0, c7, c5, 0" :: "r"(value) );
}


void mmu_invalidate_dcache_all()
{
    register uint32_t value asm("r0");

    value = 0;

    __asm volatile (" mcr p15, 0, %0, c7, c6, 0 " :: "r"(value) );
}

volatile void mmu_set_rs(uint32_t rs) {                
    register uint32_t c1_r = 0;                  
    asm volatile("mrc p15, 0, %0, c1, c0, 0" ::"r"(c1_r)); 
    c1_r &= 0xFFFFFCFF;
    c1_r |= (rs & 0x3) << 8;
    asm volatile("mcr p15, 0, %0, c1, c0, 0" ::"r"(c1_r));
}

void mmu_SetDomainPermCheck(uint32_t domain, bool check)
{
    register uint32_t c3 asm("r2");
    __asm volatile("mrc p15,0,%0,c3,c0,0" ::"r"(c3)); 


    c3 &= ~(0x3 << (domain * 2));
    if(check)
    {
        c3 |= (0x1 << (domain * 2));
    }else
    {
        c3 |= (0x3 << (domain * 2));
    }

    __asm volatile("mcr p15,0,%0,c3,c0,0" ::"r"(c3)); 
}

void mmu_enable(uint32_t base)
{
    __asm volatile("mcr p15, 0, R0, c2, c0, 0");    // Write L1 Trans-table base addr;
    __asm volatile("PUSH {R1-R3}");

    mmu_invalidate_dcache_all();
    mmu_invalidate_icache();
    mmu_invalidate_tlb();

    __asm volatile("mrc p15, 0, r0, c1, c0, 0");    // Read MMU c1 Register;
    __asm volatile("orr r0,r0,#1");                 // Enable MMU;
    __asm volatile("orr r0,r0,#5");                 // Enable MMU and DATA Cache;
    __asm volatile("orr r0,r0,#0x1000");            // Enable Instruction Cache;
    __asm volatile("mcr p15, 0, r0, c1, c0, 0");    // Write back to c1;

    __asm volatile("mov r0,r0"); 
    __asm volatile("mov r0,r0");
    __asm volatile("POP {R1-R3}");
}

static inline void SetMPTELoc(uint32_t mpte, uint32_t seg)
{
    BF_CS1n(DIGCTL_MPTEn_LOC, mpte, LOC, seg);
}

static inline void SetL1PTE(uint32_t pte, uint32_t interpret, uint32_t targetAddr, uint32_t domain, 
    uint32_t AP, bool cache, bool buffer)
{
    //volatile uint32_t *PTE_LOC = (uint32_t *)DFLPT_BASE;
    volatile uint32_t *PTE_LOC = (uint32_t *)L1PTE;
    PTE_LOC[pte] = 0;
    switch (interpret)
    {
    case L1PTE_INTERPRET_INVALID:
        PTE_LOC[pte] = 0;
        break;
    case L1PTE_INTERPRET_COARSE:
        PTE_LOC[pte] |= L1PTE_INTERPRET_COARSE;
        PTE_LOC[pte] |= targetAddr & 0xFFFFFC00;
        PTE_LOC[pte] |= (domain & 0xF) << 5;
        PTE_LOC[pte] |= 1 << 4;
        break;
    case L1PTE_INTERPRET_SECTION:
        PTE_LOC[pte] |= L1PTE_INTERPRET_SECTION;
        PTE_LOC[pte] |= targetAddr & 0xFFF00000;
        PTE_LOC[pte] |= (AP & 0x3) << 10 ;
        PTE_LOC[pte] |= (domain & 0xF) << 5;
        PTE_LOC[pte] |= 1 << 4;
        if(cache)
            PTE_LOC[pte] |= (1) << 3;
        if(buffer)
            PTE_LOC[pte] |= (1) << 2;
        break;
    case L1PTE_INTERPRET_FINE:
        PTE_LOC[pte] |= L1PTE_INTERPRET_FINE;
        PTE_LOC[pte] |= targetAddr & 0xFFFFF000;
        PTE_LOC[pte] |= (domain & 0xF) << 5;
        PTE_LOC[pte] |= 1 << 4;
        break;
    default:
        INFO("ERROR: Unexpected L1 Interpret:%lu\n", interpret);
        break;
    }

    VM_INFO("WR PTE:%08x\n",PTE_LOC[pte]);
}

void mmu_dumpMapInfo()
{
    uint32_t *L1PTE = (uint32_t *)DFLPT_BASE;
    for(int i = 0; i < L1PTE_NUM; i++)
    {
        if(L1PTE[i] != 0){
            switch (GetPTEMapType(L1PTE[i]))
            {
            case L1PTE_INTERPRET_SECTION:
                INFO("L1PTE:  VirtAddr:%08x ~ %08x  MapTo  %08lx ~ %08lx\n",
                    i*SEG_SIZE,(i+1)*SEG_SIZE - 1, 
                    (L1PTE[i] & 0xFFF00000), (L1PTE[i] & 0xFFF00000) | (SEG_SIZE - 1));
                    
                break;
            case L1PTE_INTERPRET_COARSE:
                INFO("L1PTE:  Seg %d(%08x) Reference L2 Table:%08lx:\n",
                    i, i*SEG_SIZE, (L1PTE[i] & ~0x3FF));

                    uint32_t *L2PTE = ((uint32_t *)(L1PTE[i] & ~0x3FF));
                    for(int j=0; j<256; j++){
                        if(L2PTE[j] != 0){
                            INFO("\tL2PTE:  VirtAddr:%08x ~ %08x MapTo %08lx ~ %08lx, AP:%lx, buffer:%ld, cache:%ld, VAL:%08lx\n",
                                i*SEG_SIZE + j*PAGE_SIZE, i*SEG_SIZE + (j+1)*PAGE_SIZE - 1,
                                (L2PTE[j] & 0xFFFFF000), (L2PTE[j] & 0xFFFFF000) | (PAGE_SIZE - 1),
                                (L2PTE[j] >> 4) & 0xFF,
                                (L2PTE[j] >> 2) & 1,
                                (L2PTE[j] >> 3) & 1,
                                L2PTE[j]
                            );
                        }

                    }
                break;
            default:
                break;
            }
        }
    }
}

void mmu_unmap_page(uint32_t vaddr)
{
    if(vaddr >> 20 == 0)
    {
        INFO("Can not ummap seg 0!\n");
        return;
    }
    uint32_t *L1PTE = (uint32_t *)DFLPT_BASE;
    uint32_t *L2PTE = ((uint32_t *)(L1PTE[  vaddr >> 20 ] & ~0x3FF));

    #if USE_TINY_PAGE
    L2PTE[(vaddr >> 10) & 0x3FF] = 0;
    #else
    L2PTE[(vaddr >> 12) & 0xFF] = 0;
    #endif
}

void mmu_map_page(uint32_t vaddr, uint32_t paddr,
    uint32_t AP, bool cache, bool buffer)
{
    if(vaddr >> 20 == 0)
    {
        INFO("Can not remap seg 0!\n");
        return;
    }
    uint32_t *L1PTE = (uint32_t *)DFLPT_BASE;
    uint32_t *L2PTE = ((uint32_t *)(L1PTE[  vaddr >> 20 ] & ~0x3FF)); // check which seg (0~4095)

    uint32_t val = 0;

#if USE_TINY_PAGE
    val  = (paddr & (~0x3FF));
    val |= (AP & 0x3) << 4;
    val |= ((cache & 1) << 3);
    val |= ((buffer & 1) << 2);
    val |= 3;
    L2PTE[(vaddr >> 10) & 0x3FF] = val;
#else
    val  = (paddr & (~0xFFF));
    val |= (AP & 0x3) << 4;
    val |= (AP & 0x3) << 6;
    val |= (AP & 0x3) << 8;
    val |= (AP & 0x3) << 10;

    val |= ((cache & 1) << 3);
    val |= ((buffer & 1) << 2);
    val |= 2;

    L2PTE[(vaddr >> 12) & 0xFF] = val;

#endif
    //INFO("SET VAL:%08x\n", val);

}

void mmu_init()
{
    memset(L2PTE, 0, sizeof(L2PTE));
    memset(L1PTE, 0, sizeof(L1PTE));

    //SetMPTELoc(0, 0);
    SetL1PTE(0      , L1PTE_INTERPRET_SECTION, 0         , OSLOADER_MEMORY_DOMAIN, AP_SYSRW_USROR, false, false);
    SetL1PTE(0x800  , L1PTE_INTERPRET_SECTION, 0x80000000, HARDWARE_MEMORY_DOMAIN, AP_SYSRW_USRNONE, false, false);
    
    uint32_t j = 0;
    for(int i = 0; i < VM_ROM_NUM_SEG; i++)
    {
        #if USE_TINY_PAGE
        SetL1PTE(VADDR_TO_L1SEGn(VM_ROM_BASE + i * SEG_SIZE) ,
                 L1PTE_INTERPRET_FINE,
                 SEGn_L2TAB_BASE(j++),
                 VM_ROM_DOMAIN, AP_SYSRW_USROR, true, true);
        #else
        SetL1PTE(VADDR_TO_L1SEGn(VM_ROM_BASE + i * SEG_SIZE) ,
                 L1PTE_INTERPRET_COARSE,
                 SEGn_L2TAB_BASE(j++),
                 VM_ROM_DOMAIN, AP_SYSRW_USROR, true, true);
        #endif
    }
    for(int i = 0; i < VM_RAM_NUM_SEG; i++)
    {
        #if USE_TINY_PAGE
        SetL1PTE(VADDR_TO_L1SEGn(VM_RAM_BASE + i * SEG_SIZE) ,
                 L1PTE_INTERPRET_FINE,
                 SEGn_L2TAB_BASE(j++),
                 VM_ROM_DOMAIN, AP_SYSRW_USROR, true, true);
        #else
        SetL1PTE(VADDR_TO_L1SEGn(VM_RAM_BASE + i * SEG_SIZE) ,
                 L1PTE_INTERPRET_COARSE,
                 SEGn_L2TAB_BASE(j++),
                 VM_ROM_DOMAIN, AP_SYSRW_USROR, true, true);
        #endif
    }
    for(int i = 0; i < VM_SYS_ROM_NUM_SEG; i++)
    {
        #if USE_TINY_PAGE
        SetL1PTE(VADDR_TO_L1SEGn(VM_SYS_ROM_BASE + i * SEG_SIZE) ,
                 L1PTE_INTERPRET_FINE,
                 SEGn_L2TAB_BASE(j++),
                 VM_ROM_DOMAIN, AP_SYSRW_USRRW, true, true);
        #else
        SetL1PTE(VADDR_TO_L1SEGn(VM_SYS_ROM_BASE + i * SEG_SIZE) ,
                 L1PTE_INTERPRET_COARSE,
                 SEGn_L2TAB_BASE(j++),
                 VM_ROM_DOMAIN, AP_SYSRW_USROR, true, true);
        #endif
    }

    mmu_SetDomainPermCheck(HARDWARE_MEMORY_DOMAIN, true);
    mmu_SetDomainPermCheck(OSLOADER_MEMORY_DOMAIN, true);
    mmu_SetDomainPermCheck(VM_RAM_DOMAIN, true);
    mmu_SetDomainPermCheck(VM_ROM_DOMAIN, true);

    mmu_set_rs(2);

    //mmu_dumpMapInfo();

    mmu_enable(DFLPT_BASE);


    VM_INFO("L2 PTE Size:%d\n",sizeof(L2PTE));
    //mmu_dumpMapInfo();
}

