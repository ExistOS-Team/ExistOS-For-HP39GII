

#include "SystemConfig.h"
//#include "ff.h"
#include "FTL_up.h"
#include "display_up.h"
#include "mtd_up.h"

#include "../debug.h"

#include "llapi.h"
#include "llapi_code.h"
#include "mapList.h"
//#include "mem_malloc.h"
#include "mmu.h"
#include "queue.h"
#include "vmMgr.h"

#include "minilzo.h"
#include "quicklz.h"
#include "tlsf/tlsf.h"

typedef struct CachePageInfo_t {
    struct CachePageInfo_t *prev;
    struct CachePageInfo_t *next;
    uint32_t mapToVirtAddr;
    uint32_t PageOnPhyAddr;
    uint32_t onPart;
    uint32_t onSector;
    uint32_t sectorOffset;
    bool dirty;
    bool lock;
} CachePageInfo_t;




static MapList_t *maplist;


static inline void mapListInit()
{
    maplist = pvPortMalloc(sizeof(MapList_t));
    memset(maplist, 0, sizeof(MapList_t));
}


static inline uint32_t mapList_AddPartitionMap(int part, uint32_t perm, uint32_t VMemStartAddr, uint32_t PartStartSector, uint32_t memSize)
{
    MapList_t *tmp = pvPortMalloc(sizeof(MapList_t));
    MapList_t *chain = maplist;
    if(tmp == NULL){
        return 1;
    }

    tmp->next = NULL;
    tmp->memSize = memSize;
    tmp->part = part;
    tmp->PartStartSector = PartStartSector;
    tmp->perm = perm;
    tmp->VMemStartAddr = VMemStartAddr;

    while(chain->next != NULL){
        chain = chain->next;
    }
    chain->next = tmp;
    return 0;
}

static inline MapList_t *mapList_findVirtAddrInWhichMap(uint32_t Addr)
{
    MapList_t *chain = maplist;

    if(  (Addr >= chain->VMemStartAddr) && (Addr < (chain->VMemStartAddr + chain->memSize))  ){
         return chain;
    }
    chain = chain->next;

    while(chain != NULL){
        if(  (Addr >= chain->VMemStartAddr) && (Addr < (chain->VMemStartAddr + chain->memSize))  ){
         return chain;
        }
        chain = chain->next;
    }
    
    return NULL;
}



#if SEPARATE_VMM_CACHE
uint8_t CachePageTotal[PAGE_SIZE * (NUM_CACHEPAGE_VROM + NUM_CACHEPAGE_VRAM)] __attribute__((aligned(PAGE_SIZE)));
uint8_t *CachePageVROM = &CachePageTotal[0];
uint8_t *CachePageVRAM = &CachePageTotal[PAGE_SIZE * NUM_CACHEPAGE_VROM];
uint8_t ZRAM[ZRAM_SIZE];

uint32_t ZRAMAddress_Tab[ZRAM_COMPRESSED_SIZE / 1024];
static CachePageInfo_t CachePageInfoVROM[NUM_CACHEPAGE_VROM];
static CachePageInfo_t CachePageInfoVRAM[NUM_CACHEPAGE_VRAM];

static volatile CachePageInfo_t *CachePageVROMCur;
static volatile CachePageInfo_t *CachePageVROMHead;
static volatile CachePageInfo_t *CachePageVROMTail;

static volatile CachePageInfo_t *CachePageVRAMCur;
static volatile CachePageInfo_t *CachePageVRAMHead;
static volatile CachePageInfo_t *CachePageVRAMTail;

#define CACHEVROM_PAGEn_BASE(n) (uint32_t)(&CachePageVROM[n * PAGE_SIZE])
#define CACHEVRAM_PAGEn_BASE(n) (uint32_t)(&CachePageVRAM[n * PAGE_SIZE])

bool mem_swap_enable = 0;

#else
uint8_t CachePage[PAGE_SIZE * NUM_CACHEPAGE] __attribute__((aligned(PAGE_SIZE)));
static CachePageInfo_t CachePageInfo[NUM_CACHEPAGE];
static volatile CachePageInfo_t *CachePageCur;
static volatile CachePageInfo_t *CachePageHead;
static volatile CachePageInfo_t *CachePageTail;
#define CACHE_PAGEn_BASE(n) (uint32_t)(&CachePage[n * PAGE_SIZE])
#endif

#if (VMRAM_USE_FTL == 0)
uint8_t vm_ram_none_ftl[VM_RAM_SIZE_NONE_FTL] __attribute__((aligned(PAGE_SIZE)));
#endif

static bool vmMgrInit = false;

// extern SemaphoreHandle_t LL_upSysContextMutex;
extern TaskHandle_t upSystem;
extern bool upSystemInException;
uint32_t g_page_vram_fault_cnt = 0;
uint32_t g_page_vrom_fault_cnt = 0;

extern bool g_vm_in_pagefault;

//tlsf_t tlsf_pool;

uint8_t *VMMGR_GetCacheAddress() {
#if SEPARATE_VMM_CACHE
    return CachePageTotal;
#else
    return CachePage;
#endif
}
/*
uint8_t calc_chksum(uint8_t *buf, uint32_t size) {
    uint8_t result = 0;
    uint32_t i = 0;
    while (size--) {
        result ^= buf[i++];
    }
    return result;
}
*/
QueueHandle_t PageFaultQueue;

void VM_Unconscious(TaskHandle_t task, char *res, uint32_t address);

static void taskAccessFaultAddr(pageFaultInfo_t *info, char *res) {
    // VM_ERR("Fault Task:%s\n", pcTaskGetTaskName(info->FaultTask));
    VM_ERR("Access Invalid Address:%08lx\n", info->FaultMemAddr);
    VM_ERR("%s\n", res);

    uint32_t *regs = (uint32_t *)info->FaultTask;
    regs = (uint32_t *)regs[1];

    regs -= 16;
    for (int i = 0; i < 16; i++) {
        VM_ERR("SAVED REGS[%d]:%08lx\n", i, regs[i]);
    }
    switch (info->FSR) {
    case FSR_UNKNOWN:
        VM_Unconscious(info->FaultTask, "[UNK]", info->FaultMemAddr);
        break;
    case FSR_DATA_UNALIGN:
        VM_Unconscious(info->FaultTask, "[ACCESS UNALIGN]", info->FaultMemAddr);
        break;
    case FSR_INST_FETCH:
        VM_Unconscious(info->FaultTask, "[INS FETCH]", info->FaultMemAddr);
        break;
    case FSR_DATA_WR_RDONLY:
        VM_Unconscious(info->FaultTask, "[WR RO]", info->FaultMemAddr);
        break;
    case FSR_DATA_ACCESS_UNMAP_DAB:
    case FSR_DATA_ACCESS_UNMAP_PAB:
        VM_Unconscious(info->FaultTask, "[MEMORY UNMAP]", info->FaultMemAddr);
        break;
    case FSR_ZRAM_OOM:
        VM_Unconscious(info->FaultTask, "[ZRAM OOM]", info->FaultMemAddr);
        break;
    case FSR_SWAP_NOTENABLE:
        VM_Unconscious(info->FaultTask, "[SWAP NOT ENABLE]", info->FaultMemAddr);
        break;    
    default:
        VM_Unconscious(info->FaultTask, "[???]", info->FaultMemAddr);
        break;
    }
    // cdmp_dump_layout();
}

extern volatile uint32_t swapping;

inline static bool vmMgr_CheckAddrVaild(uint32_t addr) {
    MapList_t *L;
    L = mapList_findVirtAddrInWhichMap(addr);

    if (L == NULL) {
        return false;
    }

    if ((addr - L->VMemStartAddr) >= L->memSize) {
        return false;
    }

    return true;
}

#if SEPARATE_VMM_CACHE

static uint32_t zram_free, zram_used, zram_total;

/*
static void zram_tlsf_walker(void* ptr, size_t size, int used, void* user)
{
    if(!used)
    {
        zram_free += size;
    }else{
        zram_used += size;
    }
}
*/

void zram_info(uint32_t *free, uint32_t *total)
{
    zram_used = get_used_size(ZRAM);
    //zram_free = get_max_size(ZRAM) - zram_used;
 //   tlsf_walk_pool(tlsf_get_pool(tlsf_pool), zram_tlsf_walker, NULL);
    
    zram_total = ZRAM_SIZE;
    *free = ZRAM_SIZE - zram_used;
    *total = zram_total;

}

static inline void get_vrom_page_and_move_to_tail() {

    CachePageVROMCur = CachePageVROMHead;

    CachePageVROMHead = CachePageVROMHead->next;
    CachePageVROMHead->prev = NULL;

    CachePageVROMTail->next = (CachePageInfo_t *)CachePageVROMCur;
    CachePageVROMCur->prev = (CachePageInfo_t *)CachePageVROMTail;
    CachePageVROMCur->next = NULL;

    CachePageVROMTail = CachePageVROMCur;
}
static inline void get_vram_page_and_move_to_tail() {

    CachePageVRAMCur = CachePageVRAMHead;

    CachePageVRAMHead = CachePageVRAMHead->next;
    CachePageVRAMHead->prev = NULL;

    CachePageVRAMTail->next = (CachePageInfo_t *)CachePageVRAMCur;
    CachePageVRAMCur->prev = (CachePageInfo_t *)CachePageVRAMTail;
    CachePageVRAMCur->next = NULL;

    CachePageVRAMTail = CachePageVRAMCur;
}

static inline void move_vrom_cache_page_to_tail(CachePageInfo_t *item) {
    if ((!item->next) && (item->prev)) {
        return;
    }
    if ((item->prev) && (item->next)) {
        item->prev->next = item->next;
        item->next->prev = item->prev;
        item->prev = (CachePageInfo_t *)CachePageVROMTail;
        CachePageVROMTail->next = item;
        CachePageVROMTail = item;
        return;
    }
    if ((item->next) && (!item->prev)) {
        get_vrom_page_and_move_to_tail();
    }
}

static inline void move_vram_cache_page_to_tail(CachePageInfo_t *item) {
    if ((!item->next) && (item->prev)) {
        return;
    }
    if ((item->prev) && (item->next)) {
        item->prev->next = item->next;
        item->next->prev = item->prev;
        item->prev = (CachePageInfo_t *)CachePageVRAMTail;
        CachePageVRAMTail->next = item;
        CachePageVRAMTail = item;
        return;
    }
    if ((item->next) && (!item->prev)) {
        get_vram_page_and_move_to_tail();
    }
}

static inline CachePageInfo_t *search_vrom_cache_page_by_vaddr(uint32_t vaddr) {
    CachePageInfo_t *tmp = (CachePageInfo_t *)CachePageVROMTail;
    do {
        if (tmp->mapToVirtAddr == vaddr) {
            return tmp;
        }
        tmp = tmp->prev;
    } while (tmp);
    return NULL;
}

static inline CachePageInfo_t *search_vram_cache_page_by_vaddr(uint32_t vaddr) {
    CachePageInfo_t *tmp = (CachePageInfo_t *)CachePageVRAMTail;
    do {
        if (tmp->mapToVirtAddr == vaddr) {
            return tmp;
        }
        tmp = tmp->prev;
    } while (tmp);
    return NULL;
}

// char compress_buffer[PAGE_SIZE + 42];
uint32_t compress_buffer[2 * PAGE_SIZE / (sizeof(uint32_t)) + 12];
// static qlz_state_compress comp_state;
// static qlz_state_decompress decomp_state;
uint32_t comp_wrkbuffer[2048 / 4];

extern uint32_t g_mem_comp_rate[16];
extern uint32_t g_mem_comp_rate_ptr;
static inline int save_cache_page(CachePageInfo_t *cache_page) {
    // sizeof(comp_state);
    // sizeof(decomp_state);
    int ret = 0;
    uint32_t sz;
    if (
        (cache_page->dirty) &&
        (cache_page->onPart == MAP_PART_FTL)) {
        mmu_clean_invalidated_dcache(cache_page->mapToVirtAddr, PAGE_SIZE);
        mmu_drain_buffer();
        // printf("sector:%d,%d\n", cache_page->onSector, cache_page->sectorOffset);
        // printf("ind:%d\n", (cache_page->onSector * 2048 + cache_page->sectorOffset) / PAGE_SIZE);
        uint32_t ind = (cache_page->onSector * 2048 + cache_page->sectorOffset) / PAGE_SIZE;
        if (ind < (ZRAM_COMPRESSED_SIZE / PAGE_SIZE)) {
            //
            // memcpy(mem_buffer(ZRAMAddress_Tab[ind]), (void *)cache_page->PageOnPhyAddr, PAGE_SIZE);
            // memset(workbuf_comp, 0, sizeof(workbuf_comp));
#if MEM_COMPRESSION_ALGORITHM == QUICKLZ
            sz = qlz_compress((void *)cache_page->PageOnPhyAddr, compress_buffer, PAGE_SIZE, (qlz_state_compress *)&comp_state);
            g_mem_comp_rate[g_mem_comp_rate_ptr++] = sz * 100 / PAGE_SIZE;
            if (g_mem_comp_rate_ptr >= 16) {
                g_mem_comp_rate_ptr = 0;
            }
            cdmp_free(ZRAMAddress_Tab[ind]);
            ZRAMAddress_Tab[ind] = cdmp_alloc(sz);
            cdmp_wrtie(ZRAMAddress_Tab[ind], 0, sz, (void *)compress_buffer);
#endif
#if MEM_COMPRESSION_ALGORITHM == MINILZO
            int ret = lzo1x_1_compress((void *)cache_page->PageOnPhyAddr, PAGE_SIZE, (char *)compress_buffer, &sz, comp_wrkbuffer);
            if (ret == LZO_E_OK) {
                lzo_sizeof_dict_t;
                // printf("comp:%d>%d\n", PAGE_SIZE, sz);
            } else {
                printf("COMPERR:%d\n", ret);
                while (1)
                    ;
            }
            g_mem_comp_rate[g_mem_comp_rate_ptr++] = sz * 100 / PAGE_SIZE;
            if (g_mem_comp_rate_ptr >= 16) {
                g_mem_comp_rate_ptr = 0;
            }

            ZRAMAddress_Tab[ind] = (uint32_t)tlsf_realloc((void *)ZRAMAddress_Tab[ind], sz);
            if(ZRAMAddress_Tab[ind])
            {
                memcpy((void *)ZRAMAddress_Tab[ind], (void *)compress_buffer, sz );
            }else{
                printf("ZRAM OOM!\n");
                return -2;
            }
            
            //cdmp_free(ZRAMAddress_Tab[ind]);
            //ZRAMAddress_Tab[ind] = cdmp_alloc(sz);
            //cdmp_wrtie(ZRAMAddress_Tab[ind], 0, sz, (void *)compress_buffer);
#endif
#if MEM_COMPRESSION_ALGORITHM == 0
            g_mem_comp_rate[g_mem_comp_rate_ptr++] = PAGE_SIZE * 100 / PAGE_SIZE;
            if (g_mem_comp_rate_ptr >= 16) {
                g_mem_comp_rate_ptr = 0;
            }
            ZRAMAddress_Tab[ind] = (uint32_t)tlsf_realloc(tlsf_pool, (void *)ZRAMAddress_Tab[ind], PAGE_SIZE);
            if(ZRAMAddress_Tab[ind])
            {
                memcpy((void *)ZRAMAddress_Tab[ind], (void *)compress_buffer, sz );
            }else{
                printf("ZRAM OOM!\n");
                return -2;
            }
            //cdmp_free(ZRAMAddress_Tab[ind]);
            //ZRAMAddress_Tab[ind] = cdmp_alloc(PAGE_SIZE);
            //cdmp_wrtie(ZRAMAddress_Tab[ind], 0, sz, (void *)cache_page->PageOnPhyAddr);
#endif

            // printf("page comp:%d -> %ld\n", PAGE_SIZE, sz);

            // cdmp_wrtie(ZRAMAddress_Tab[ind], 0, PAGE_SIZE, (void *)cache_page->PageOnPhyAddr);
        } else {
            // printf("TO SWAP AREA\n");
            if (mem_swap_enable) {
                FTL_ReadSector(cache_page->onSector, 1, (uint8_t *)compress_buffer);
                memcpy((void *)((uint32_t)compress_buffer + cache_page->sectorOffset), (uint8_t *)cache_page->PageOnPhyAddr, PAGE_SIZE);
                FTL_WriteSector(cache_page->onSector, 1, (uint8_t *)compress_buffer);

            } else {
                printf("SWAP IS NOT ENABLE!!\n");
                return -3;
            }
        }
        /*
        if(cache_page->mapToVirtAddr - VM_RAM_BASE < ZRAM_SIZE)
        {
            uint32_t ind = (cache_page->mapToVirtAddr - VM_RAM_BASE) / PAGE_SIZE;

            //printf("Save IND:%d\n", (cache_page->mapToVirtAddr - VM_RAM_BASE) / PAGE_SIZE );
            ZRAMAddress_Tab[ind] = cdmp_alloc(PAGE_SIZE);
            cdmp_wrtie(ZRAMAddress_Tab[ind], 0, PAGE_SIZE, (void *)cache_page->PageOnPhyAddr);
            chksum[ind] = calc_chksum((uint8_t *)cache_page->PageOnPhyAddr, PAGE_SIZE);

        }else{
            printf("TO FTL1\n");
        }
        */

        /*
                FTL_ReadSector(cache_page->onSector, 1, page_save_wr_buf);
                memcpy(&page_save_wr_buf[CachePageCur->sectorOffset], (uint8_t *)cache_page->PageOnPhyAddr, PAGE_SIZE);
                ret = FTL_WriteSector(cache_page->onSector, 1, (uint8_t *)page_save_wr_buf);
                if (pagebuf_last_rd == cache_page->onSector) {
                    pagebuf_last_rd = 0xFFFFFFFF;
                }
                */

        cache_page->dirty = false;
    }
    return ret;
}

#else

static inline __attribute__((target("thumb"))) void get_page_and_move_to_tail() {

    CachePageCur = CachePageHead;

    CachePageHead = CachePageHead->next;
    CachePageHead->prev = NULL;

    CachePageTail->next = (CachePageInfo_t *)CachePageCur;
    CachePageCur->prev = (CachePageInfo_t *)CachePageTail;
    CachePageCur->next = NULL;

    CachePageTail = CachePageCur;
}

static inline __attribute__((target("thumb"))) void move_cache_page_to_tail(CachePageInfo_t *item) {
    if ((!item->next) && (item->prev)) {
        return;
    }
    if ((item->prev) && (item->next)) {
        item->prev->next = item->next;
        item->next->prev = item->prev;
        item->prev = (CachePageInfo_t *)CachePageTail;
        CachePageTail->next = item;
        CachePageTail = item;
        return;
    }
    if ((item->next) && (!item->prev)) {
        get_page_and_move_to_tail();
    }
}

static inline __attribute__((target("thumb"))) CachePageInfo_t *search_cache_page_by_vaddr(uint32_t vaddr) {
    CachePageInfo_t *tmp = (CachePageInfo_t *)CachePageTail;
    do {
        if (tmp->mapToVirtAddr == vaddr) {
            return tmp;
        }
        tmp = tmp->prev;
    } while (tmp);
    return NULL;
}

#if USE_TINY_PAGE
static uint8_t page_save_wr_buf[2048];
static uint8_t page_save_rd_buf[2048];
static uint32_t pagebuf_last_rd = 0xFFFFFFFF;
#endif

static inline int save_cache_page(CachePageInfo_t *cache_page) {
    int ret = 0;
    if (
        (cache_page->dirty) &&
        (cache_page->onPart == MAP_PART_FTL)) {
        mmu_clean_invalidated_dcache(cache_page->mapToVirtAddr, PAGE_SIZE);
        mmu_drain_buffer();
#if USE_TINY_PAGE
        FTL_ReadSector(cache_page->onSector, 1, page_save_wr_buf);
        memcpy(&page_save_wr_buf[CachePageCur->sectorOffset], (uint8_t *)cache_page->PageOnPhyAddr, PAGE_SIZE);
        ret = FTL_WriteSector(cache_page->onSector, 1, (uint8_t *)page_save_wr_buf);
        if (pagebuf_last_rd == cache_page->onSector) {
            pagebuf_last_rd = 0xFFFFFFFF;
        }
#else
        ret = FTL_WriteSector(cache_page->onSector, 2, (uint8_t *)cache_page->PageOnPhyAddr);
#endif
        cache_page->dirty = false;
    }
    return ret;
}

#endif

bool inline __attribute__((target("thumb"))) vmMgr_checkAddressValid(uint32_t address, uint32_t perm) {
    if (address < MEMORY_SIZE) {
        return true;
    }
#if (VMRAM_USE_FTL == 0)
    if ((address >= VM_RAM_BASE) && (address <= VM_RAM_BASE + VM_RAM_SIZE_NONE_FTL)) {
        return true;
    }

#endif
    MapList_t *ret = mapList_findVirtAddrInWhichMap(address);
    if (ret) {
        if (ret->perm & perm) {
            return true;
        }
    }
    return false;
}

// volatile uint8_t m_test_read;
/*
uint32_t vmMgr_LoadPageGetPAddr(uint32_t vaddr) {
    if (!vmMgr_checkAddressValid(vaddr, PERM_R)) {
        return 0;
    }
    // m_test_read = *((uint8_t *)vaddr);
    // m_test_read--;

    for (int i = 0; i < NUM_CACHEPAGE; i++) {
#if USE_TINY_PAGE
        if (((CachePageInfo[i].mapToVirtAddr & 0xFFFFFC00) == (vaddr & 0xFFFFFC00))) {
            mmu_clean_invalidated_dcache(vaddr & 0xFFFFFC00, PAGE_SIZE);
            return (CachePageInfo[i].PageOnPhyAddr + (vaddr & (PAGE_SIZE - 1)));
        }
#else
        if (((CachePageInfo[i].mapToVirtAddr & 0xFFFFF000) == (vaddr & 0xFFFFF000))) {
            mmu_clean_invalidated_dcache(vaddr & 0xFFFFF000, PAGE_SIZE);
            return (CachePageInfo[i].PageOnPhyAddr + (vaddr & (PAGE_SIZE - 1)));
        }
#endif
    }

    return 0;
}
*/
#if SEPARATE_VMM_CACHE

void vmMgr_ReleaseAllPage() {
    memset(CachePageTotal, 0, sizeof(CachePageTotal));
    memset(CachePageInfoVRAM, 0, sizeof(CachePageInfoVRAM));
    memset(CachePageInfoVROM, 0, sizeof(CachePageInfoVROM));
    for (int i = 0; i < NUM_CACHEPAGE_VROM; i++) {
        CachePageInfoVROM[i].dirty = false;
        CachePageInfoVROM[i].onPart = -1;
        CachePageInfoVROM[i].onSector = 0;
        CachePageInfoVROM[i].sectorOffset = 0;
        CachePageInfoVROM[i].PageOnPhyAddr = CACHEVROM_PAGEn_BASE(i);
        CachePageInfoVROM[i].mapToVirtAddr = 0;
        CachePageInfoVROM[i].lock = false;
        if (i > 0) {
            CachePageInfoVROM[i - 1].next = &CachePageInfoVROM[i];
            CachePageInfoVROM[i].prev = &CachePageInfoVROM[i - 1];
        }
    }

    CachePageVROMCur = CachePageVROMHead = &CachePageInfoVROM[0];
    CachePageVROMTail = &CachePageInfoVROM[NUM_CACHEPAGE_VROM - 1];

    for (int i = 0; i < NUM_CACHEPAGE_VRAM; i++) {
        CachePageInfoVRAM[i].dirty = false;
        CachePageInfoVRAM[i].onPart = -1;
        CachePageInfoVRAM[i].onSector = 0;
        CachePageInfoVRAM[i].sectorOffset = 0;
        CachePageInfoVRAM[i].PageOnPhyAddr = CACHEVRAM_PAGEn_BASE(i);
        CachePageInfoVRAM[i].mapToVirtAddr = 0;
        CachePageInfoVRAM[i].lock = false;
        if (i > 0) {
            CachePageInfoVRAM[i - 1].next = &CachePageInfoVRAM[i];
            CachePageInfoVRAM[i].prev = &CachePageInfoVRAM[i - 1];
        }
    }

    CachePageVRAMCur = CachePageVRAMHead = &CachePageInfoVRAM[0];
    CachePageVRAMTail = &CachePageInfoVRAM[NUM_CACHEPAGE_VRAM - 1];

    mmu_drain_buffer();

    for (uint32_t i = VM_ROM_BASE; i < VM_ROM_BASE + VM_ROM_SIZE; i += PAGE_SIZE) {
        mmu_unmap_page(i);
        mmu_clean_invalidated_dcache(i, PAGE_SIZE);
    }
    for (uint32_t i = VM_RAM_BASE; i < VM_RAM_BASE + VM_RAM_SIZE; i += PAGE_SIZE) {
        mmu_unmap_page(i);
        mmu_clean_dcache(i, PAGE_SIZE);
        mmu_clean_invalidated_dcache(i, PAGE_SIZE);
    }

    mmu_invalidate_tlb();
    mmu_invalidate_icache();
    mmu_invalidate_dcache_all();
}

#else

void vmMgr_ReleaseAllPage() {
    memset(CachePage, 0, sizeof(CachePage));
    memset(CachePageInfo, 0, sizeof(CachePageInfo));
    for (int i = 0; i < NUM_CACHEPAGE; i++) {
        CachePageInfo[i].dirty = false;
        CachePageInfo[i].onPart = -1;
        CachePageInfo[i].onSector = 0;
        CachePageInfo[i].sectorOffset = 0;
        CachePageInfo[i].PageOnPhyAddr = CACHE_PAGEn_BASE(i);
        CachePageInfo[i].mapToVirtAddr = 0;
        CachePageInfo[i].lock = false;
        if (i > 0) {
            CachePageInfo[i - 1].next = &CachePageInfo[i];
            CachePageInfo[i].prev = &CachePageInfo[i - 1];
        }
    }

    CachePageCur = CachePageHead = &CachePageInfo[0];
    CachePageTail = &CachePageInfo[NUM_CACHEPAGE - 1];

    mmu_drain_buffer();

    for (uint32_t i = VM_ROM_BASE; i < VM_ROM_BASE + VM_ROM_SIZE; i += PAGE_SIZE) {
        mmu_unmap_page(i);
        mmu_clean_invalidated_dcache(i, PAGE_SIZE);
    }
    for (uint32_t i = VM_RAM_BASE; i < VM_RAM_BASE + VM_RAM_SIZE; i += PAGE_SIZE) {
        mmu_unmap_page(i);
        mmu_clean_dcache(i, PAGE_SIZE);
        mmu_clean_invalidated_dcache(i, PAGE_SIZE);
    }

    mmu_invalidate_tlb();
    mmu_invalidate_icache();
    mmu_invalidate_dcache_all();
}

#endif

void __attribute__((optimize("-Os"))) vmMgr_task() {
    pageFaultInfo_t currentFault;
    MapList_t *mapinfo;

    for (;;) {
        while (xQueueReceive(PageFaultQueue, &currentFault, portMAX_DELAY) == pdTRUE) {
            vTaskSuspend(currentFault.FaultTask);

            VM_INFO("PAGE FAULT TASK [%s]. access %08x, FSR:%08x\n",
                    pcTaskGetName(currentFault.FaultTask), currentFault.FaultMemAddr, currentFault.FSR);

#if USE_HARDWARE_DFLPT
            if (reload_DFLPT_seg(currentFault.FaultMemAddr >> 20) == 2) {
                vTaskResume(currentFault.FaultTask);
                continue;
            }
#endif

            if (vmMgr_CheckAddrVaild(currentFault.FaultMemAddr) == false) {
                taskAccessFaultAddr(&currentFault, "Area is not mapped.");
                if (currentFault.FSR == FSR_DATA_ACCESS_UNMAP_PAB) {
                    printf("PAB\n");
                }
                if (currentFault.FSR == FSR_DATA_ACCESS_UNMAP_DAB) {
                    printf("DAB\n");
                }
                continue;
            }

            switch (currentFault.FSR) {
            case FSR_DATA_ACCESS_UNMAP_PAB:
            case FSR_DATA_ACCESS_UNMAP_DAB: {
                VM_INFO("Catch UnMap Mem:%s,%08x\n", pcTaskGetName(currentFault.FaultTask), currentFault.FaultMemAddr);
                mapinfo = mapList_findVirtAddrInWhichMap(currentFault.FaultMemAddr);
                if (mapinfo) {
#if SEPARATE_VMM_CACHE
                    switch (mapinfo->part) {
                    case MAP_PART_RAWFLASH:
                        g_page_vrom_fault_cnt++;
                        get_vrom_page_and_move_to_tail();
                        if (CachePageVROMCur->mapToVirtAddr) {
                            mmu_unmap_page(CachePageVROMCur->mapToVirtAddr);
                        }
                        CachePageVROMCur->mapToVirtAddr = currentFault.FaultMemAddr & ~(PAGE_SIZE - 1);
                        CachePageVROMCur->onPart = mapinfo->part;
                        CachePageVROMCur->onSector = mapinfo->PartStartSector + ((currentFault.FaultMemAddr - mapinfo->VMemStartAddr) & 0xFFFFFC00) / 2048;
                        CachePageVROMCur->sectorOffset = (currentFault.FaultMemAddr / 1024) % 2 ? 1024 : 0;
                        CachePageVROMCur->dirty = false;
                        MTD_ReadPhyPage(CachePageVROMCur->onSector, CachePageVROMCur->sectorOffset, PAGE_SIZE, (uint8_t *)CachePageVROMCur->PageOnPhyAddr);

                        mmu_map_page(
                            CachePageVROMCur->mapToVirtAddr,
                            CachePageVROMCur->PageOnPhyAddr,
                            AP_READONLY,
                            VM_CACHE_ENABLE,
                            VM_BUFFER_ENABLE);

                        if (currentFault.FSR == FSR_DATA_ACCESS_UNMAP_DAB)
                            mmu_clean_invalidated_dcache(CachePageVROMCur->mapToVirtAddr, PAGE_SIZE);
                        if (currentFault.FSR == FSR_DATA_ACCESS_UNMAP_PAB)
                            mmu_invalidate_icache();
                        mmu_invalidate_tlb();
                        // LL_CheckIRQAndTrap();
                        vTaskResume(currentFault.FaultTask);
                        g_vm_in_pagefault = false;
                        break;

                    case MAP_PART_FTL: {
                        g_page_vram_fault_cnt++;
                        get_vram_page_and_move_to_tail();
                        int ret = save_cache_page((CachePageInfo_t *)CachePageVRAMCur);
                        if(ret == -2)
                        {
                            currentFault.FSR = FSR_ZRAM_OOM;
                            taskAccessFaultAddr(&currentFault, "ZRAM OUT OF MEMORY");
                            break;
                        }else if(ret == -3)
                        {
                            currentFault.FSR = FSR_SWAP_NOTENABLE;
                            taskAccessFaultAddr(&currentFault, "SWAP NOT ENABLE");
                            break;
                        }
                        if (CachePageVRAMCur->mapToVirtAddr) {
                            mmu_unmap_page(CachePageVRAMCur->mapToVirtAddr);
                        }
                        CachePageVRAMCur->mapToVirtAddr = currentFault.FaultMemAddr & ~(PAGE_SIZE - 1);
                        CachePageVRAMCur->onPart = mapinfo->part;
                        CachePageVRAMCur->onSector = mapinfo->PartStartSector + ((currentFault.FaultMemAddr - mapinfo->VMemStartAddr) & 0xFFFFFC00) / 2048;
                        CachePageVRAMCur->sectorOffset = (currentFault.FaultMemAddr / 1024) % 2 ? 1024 : 0;
                        CachePageVRAMCur->dirty = false;
                        uint32_t zram_ind = (CachePageVRAMCur->onSector * 2048 + CachePageVRAMCur->sectorOffset) / PAGE_SIZE;
                        if ((zram_ind < (ZRAM_COMPRESSED_SIZE / PAGE_SIZE))) {
                            if (ZRAMAddress_Tab[zram_ind]) {
// memset((void *)CachePageVRAMCur->PageOnPhyAddr, 0, PAGE_SIZE);
// printf("free:%d\n", zram_ind);
// cdmp_read(ZRAMAddress_Tab[zram_ind], 0, PAGE_SIZE, (void *)CachePageVRAMCur->PageOnPhyAddr);
// cdmp_read(ZRAMAddress_Tab[zram_ind], 0, PAGE_SIZE, (void *)compress_buffer);
// qlz_decompress(compress_buffer, (void *)CachePageVRAMCur->PageOnPhyAddr, &decomp_state);
// memset(workbuf_comp, 0xFF, sizeof(workbuf_comp));
#if MEM_COMPRESSION_ALGORITHM == QUICKLZ
                                qlz_decompress(cdmp_get_memblock(ZRAMAddress_Tab[zram_ind]), (void *)CachePageVRAMCur->PageOnPhyAddr, (qlz_state_decompress *)&decomp_state);
#endif
#if MEM_COMPRESSION_ALGORITHM == MINILZO
                                uint32_t sz;
                                //int ret = lzo1x_decompress(cdmp_get_memblock(ZRAMAddress_Tab[zram_ind]),
                                //                           cdmp_memblock_size(ZRAMAddress_Tab[zram_ind]),
                                //                           (void *)CachePageVRAMCur->PageOnPhyAddr, &sz, NULL);
                                int ret = lzo1x_decompress(((void *)ZRAMAddress_Tab[zram_ind]),
                                                           PAGE_SIZE,
                                                           (void *)CachePageVRAMCur->PageOnPhyAddr, &sz, NULL);
                                if ((ret == LZO_E_OK) || (ret == LZO_E_INPUT_NOT_CONSUMED) || (ret == LZO_E_INPUT_OVERRUN)) {

                                } else {
                                    printf("DECOMP ERR:%d\n", ret);
                                    while (1)
                                        ;
                                }
#endif
#if MEM_COMPRESSION_ALGORITHM == NONE
                                cdmp_read(ZRAMAddress_Tab[zram_ind], 0, PAGE_SIZE, (void *)CachePageVRAMCur->PageOnPhyAddr);
#endif

                                // cdmp_free(ZRAMAddress_Tab[zram_ind]);

                                // memcpy((void *)CachePageVRAMCur->PageOnPhyAddr, mem_buffer(ZRAMAddress_Tab[zram_ind]), PAGE_SIZE);
                                // mem_free(ZRAMAddress_Tab[zram_ind]);
                                // ZRAMAddress_Tab[zram_ind] = 0;
                            } else {
                                // printf("new page:%ld\n", zram_ind);
                            }
                        } else {
                            if (mem_swap_enable) {
                                FTL_ReadSector(CachePageVRAMCur->onSector, 1, (uint8_t *)compress_buffer);
                                memcpy((uint8_t *)CachePageVRAMCur->PageOnPhyAddr, (void *)((uint32_t)compress_buffer + CachePageVRAMCur->sectorOffset), PAGE_SIZE);

                            } else {
                                printf("FROM SWAP AREA!!\n");
                            }
                        }

                        mmu_map_page(
                            CachePageVRAMCur->mapToVirtAddr,
                            CachePageVRAMCur->PageOnPhyAddr,
                            AP_READONLY,
                            VM_CACHE_ENABLE,
                            VM_BUFFER_ENABLE);

                        if (currentFault.FSR == FSR_DATA_ACCESS_UNMAP_DAB)
                            mmu_clean_invalidated_dcache(CachePageVRAMCur->mapToVirtAddr, PAGE_SIZE);
                        if (currentFault.FSR == FSR_DATA_ACCESS_UNMAP_PAB)
                            mmu_invalidate_icache();
                        mmu_invalidate_tlb();
                        // LL_CheckIRQAndTrap();
                        vTaskResume(currentFault.FaultTask);
                        g_vm_in_pagefault = false;
                        break;

                    } break;
                    default:
                        printf("VMMGR: Unknown map!\n");
                        break;
                    }
                    break;

#else
                    get_page_and_move_to_tail();
                    save_cache_page((CachePageInfo_t *)CachePageCur);
                    if (CachePageCur->mapToVirtAddr) {
                        mmu_unmap_page(CachePageCur->mapToVirtAddr);
                    }
#if USE_TINY_PAGE
                    CachePageCur->mapToVirtAddr = currentFault.FaultMemAddr & ~(PAGE_SIZE - 1);
                    CachePageCur->onPart = mapinfo->part;
                    CachePageCur->onSector = mapinfo->PartStartSector + ((currentFault.FaultMemAddr - mapinfo->VMemStartAddr) & 0xFFFFFC00) / 2048;
                    CachePageCur->sectorOffset = (currentFault.FaultMemAddr / 1024) % 2 ? 1024 : 0;
                    CachePageCur->dirty = false;
#else
                    CachePageCur->mapToVirtAddr = currentFault.FaultMemAddr & 0xFFFFF000;
                    CachePageCur->onPart = mapinfo->part;
                    CachePageCur->onSector = mapinfo->PartStartSector + ((currentFault.FaultMemAddr - mapinfo->VMemStartAddr) & 0xFFFFF000) / 2048;
                    CachePageCur->dirty = false;
#endif
                    int ret = 0;

                    switch (mapinfo->part) {
                    case MAP_PART_RAWFLASH:
                        g_page_vrom_fault_cnt++;
#if USE_TINY_PAGE
                        ret = MTD_ReadPhyPage(CachePageCur->onSector, CachePageCur->sectorOffset, PAGE_SIZE, (uint8_t *)CachePageCur->PageOnPhyAddr);
#else
                        ret = MTD_ReadPhyPage(CachePageCur->onSector, 0, 2048, (uint8_t *)CachePageCur->PageOnPhyAddr);
                        ret = MTD_ReadPhyPage(CachePageCur->onSector + 1, 0, 2048, (uint8_t *)(CachePageCur->PageOnPhyAddr + 2048));
#endif
                        // INFO("rom:%d\n", CachePageCur->onSector );
                        break;
                    case MAP_PART_FTL:
                        g_page_vram_fault_cnt++;
#if USE_TINY_PAGE
                        if (pagebuf_last_rd != CachePageCur->onSector) {
                            ret = FTL_ReadSector(CachePageCur->onSector, 1, (uint8_t *)page_save_rd_buf);
                        }

                        {
                            memcpy((uint8_t *)CachePageCur->PageOnPhyAddr, &page_save_rd_buf[CachePageCur->sectorOffset], PAGE_SIZE);
                            pagebuf_last_rd = CachePageCur->onSector;
                        }

#else
                        ret = FTL_ReadSector(CachePageCur->onSector, 2, (uint8_t *)CachePageCur->PageOnPhyAddr);
#endif
                        // INFO("ram:%d\n", CachePageCur->onSector );
                        break;

                    case MAP_PART_SYS: {
                        void vm_set_irq_num(uint32_t IRQNum, uint32_t r1, uint32_t r2, uint32_t r3);
                        void vm_jump_irq();
                        void vm_save_context();

                        vm_save_context();
                        vm_jump_irq();
                        vm_set_irq_num(LL_IRQ_MMU, CachePageCur->mapToVirtAddr, 0, 0);
                    }

                    break;
                    default:
                        ret = -1;
                        break;
                    }

                    if (ret >= 0) {

                        mmu_map_page(
                            CachePageCur->mapToVirtAddr,
                            CachePageCur->PageOnPhyAddr,
                            AP_READONLY,
                            VM_CACHE_ENABLE,
                            VM_BUFFER_ENABLE);

                        if (currentFault.FSR == FSR_DATA_ACCESS_UNMAP_DAB)
                            mmu_clean_invalidated_dcache(CachePageCur->mapToVirtAddr, PAGE_SIZE);
                        if (currentFault.FSR == FSR_DATA_ACCESS_UNMAP_PAB)
                            mmu_invalidate_icache();
                        mmu_invalidate_tlb();

                        // LL_CheckIRQAndTrap();
                        vTaskResume(currentFault.FaultTask);
                        g_vm_in_pagefault = false;
                        break;
                    }
                    taskAccessFaultAddr(&currentFault, "Remap Failed.");
                    break;
#endif
                }
                taskAccessFaultAddr(&currentFault, "Unknown Map.");
                break;
            }

            case FSR_DATA_WR_RDONLY: {
                VM_INFO("Catch WR RO Mem:%s,%08x\n", pcTaskGetName(currentFault.FaultTask), currentFault.FaultMemAddr);
                mapinfo = mapList_findVirtAddrInWhichMap(currentFault.FaultMemAddr);
                if (mapinfo) {
                    if (mapinfo->perm & PERM_W) {
#if SEPARATE_VMM_CACHE
                        CachePageInfo_t *gotCache = search_vram_cache_page_by_vaddr(currentFault.FaultMemAddr & 0xFFFFFC00);
                        if (gotCache) {
                            // INFO("Get:%08x\n", gotCache->mapToVirtAddr);
                            gotCache->dirty = true;
                            move_vram_cache_page_to_tail(gotCache);
                            mmu_unmap_page(gotCache->mapToVirtAddr);
                            mmu_map_page(gotCache->mapToVirtAddr,
                                         gotCache->PageOnPhyAddr,
                                         AP_SYSRW_USRRW, VM_CACHE_ENABLE, VM_BUFFER_ENABLE);

                            mmu_clean_invalidated_dcache(gotCache->mapToVirtAddr, PAGE_SIZE);

                            mmu_invalidate_tlb();

                            // LL_CheckIRQAndTrap();
                            g_page_vram_fault_cnt++;
                            vTaskResume(currentFault.FaultTask);
                            g_vm_in_pagefault = false;
                        } else {
                            printf("search_vram_cache_page_by_vaddr failed\n");
                        }
#else

#if USE_TINY_PAGE
                        CachePageInfo_t *gotCache = search_cache_page_by_vaddr(currentFault.FaultMemAddr & 0xFFFFFC00);
#else
                        CachePageInfo_t *gotCache = search_cache_page_by_vaddr(currentFault.FaultMemAddr & 0xFFFFF000);
#endif
                        if (gotCache) {
                            // INFO("Get:%08x\n", gotCache->mapToVirtAddr);
                            gotCache->dirty = true;
                            move_cache_page_to_tail(gotCache);
                            // mmu_unmap_page(gotCache->mapToVirtAddr);
                            mmu_map_page(gotCache->mapToVirtAddr,
                                         gotCache->PageOnPhyAddr,
                                         AP_SYSRW_USRRW, VM_CACHE_ENABLE, VM_BUFFER_ENABLE);

                            mmu_clean_invalidated_dcache(gotCache->mapToVirtAddr, PAGE_SIZE);

                            mmu_invalidate_tlb();

                            // LL_CheckIRQAndTrap();
                            g_page_vram_fault_cnt++;
                            vTaskResume(currentFault.FaultTask);
                            g_vm_in_pagefault = false;
                        }
#endif
                    } else {
                        taskAccessFaultAddr(&currentFault, "Memory can not be written.");
                        // INFO("Read Only Page!\n");
                    }
                }
                // mmu_dumpMapInfo();
                // vTaskResume(currentFault.FaultTask);
                // vTaskSuspend(vmBlockTask);

            } break;
            default:
                VM_ERR("Unknown Task Fault Reason.\n");
                break;
            }
            swapping = 0;
        }
    }
}

void vmMgr_init() {
    PageFaultQueue = xQueueCreate(32, sizeof(pageFaultInfo_t));
#if SEPARATE_VMM_CACHE
    //cdmp_mem_init(ZRAM, sizeof(ZRAM));
    //tlsf_pool = tlsf_create_with_pool(ZRAM, sizeof(ZRAM));
    init_memory_pool(sizeof(ZRAM), ZRAM);
    memset(ZRAMAddress_Tab, 0, sizeof(ZRAMAddress_Tab));
#endif
    mapListInit();
    mmu_init();

    vmMgr_ReleaseAllPage();

    // DisplayPutStr(0, 16 * 1, "Waiting for Flash GC...", 0, 255, 16);
    DisplayFillBox(48, 80, 208, 96, 200);
    DisplayFillBox(50, 82, 206, 94, 255);
    // DisplayFillBox(32, 32, 224, 64, 128);
    // DisplayPutStr(60, 42, "Wait for Flash GC ", 255, 128, 16);

    DisplayFillBox(48, 80, 208, 96, 200);
    DisplayFillBox(50, 82, 206, 94, 255);

    // DisplayFillBox(52, 84, 90, 92, 16);
    for (int i = 54; i <= 90; ++i)
        DisplayFillBox(i - 2, 84, i, 92, 72);
    for (int i = 0; i < FLASH_FTL_DATA_SECTOR; i++) {
        // FTL_TrimSector(i);
    }

    mapList_AddPartitionMap(MAP_PART_RAWFLASH, PERM_R, VM_ROM_BASE, FLASH_SYSTEM_BLOCK * 64, VM_ROM_SIZE);

#if (VMRAM_USE_FTL == 1)
    mapList_AddPartitionMap(MAP_PART_FTL, PERM_R | PERM_W, VM_RAM_BASE, 0, VM_RAM_SIZE);
#else
    uint32_t paddr = (uint32_t)vm_ram_none_ftl;
    for (uint32_t vaddr = VM_RAM_BASE; vaddr < (VM_RAM_BASE + VM_RAM_SIZE_NONE_FTL); vaddr += PAGE_SIZE) {
        mmu_map_page(vaddr, paddr, AP_SYSRW_USRRW, VM_CACHE_ENABLE, VM_BUFFER_ENABLE);
        paddr += PAGE_SIZE;
    }
    mmu_invalidate_tlb();
    //

#endif

    // mapList_AddPartitionMap(MAP_PART_SYS, PERM_R | PERM_W, VM_SYS_ROM_BASE, 0, VM_SYS_ROM_SIZE);

    // xTaskCreate(vVmBlockTask, "VM Swap IO Waiting", 12, NULL, 6, &vmBlockTask);
    // vTaskSuspend(vmBlockTask);
    /*
        for (int i = 0; i < NUM_CACHEPAGE_VROM; i++) {
            INFO("CACHEVROM_PAGEn_BASE:%d,%p\n", i, CACHEVROM_PAGEn_BASE(i));
        }

        for (int i = 0; i < NUM_CACHEPAGE_VRAM; i++) {
            INFO("CACHEVRAM_PAGEn_BASE:%d,%p\n", i, CACHEVRAM_PAGEn_BASE(i));
        }
        */

    INFO("Virtual Memory Enable.\n");
    vmMgrInit = true;
}

bool vmMgrInited() {
    return vmMgrInit;
}
