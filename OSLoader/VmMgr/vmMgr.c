

#include "SystemConfig.h"
//#include "ff.h"
#include "FTL_up.h"
#include "mtd_up.h"
#include "display_up.h"

#include "../debug.h"

#include "llapi.h"
#include "mapList.h"
#include "mmu.h"
#include "vmMgr.h"

#include "queue.h"

typedef struct CachePageInfo_t {
    struct CachePageInfo_t *prev;
    struct CachePageInfo_t *next;
    uint32_t mapToVirtAddr;
    uint32_t PageOnPhyAddr;
    uint32_t onPart;
    uint32_t onSector;
    bool dirty;
    bool lock;
} CachePageInfo_t;

uint8_t CachePage[PAGE_SIZE * NUM_CACHEPAGE] __attribute__((aligned(PAGE_SIZE)));
static CachePageInfo_t CachePageInfo[NUM_CACHEPAGE];


#if (VMRAM_USE_FTL == 0)
uint8_t vm_ram_none_ftl[VM_RAM_SIZE_NONE_FTL] __attribute__((aligned(PAGE_SIZE)));
#endif

static volatile CachePageInfo_t *CachePageCur;
static volatile CachePageInfo_t *CachePageHead;
static volatile CachePageInfo_t *CachePageTail;

#define CACHE_PAGEn_BASE(n) (uint32_t)(&CachePage[n * PAGE_SIZE])

static bool vmMgrInit = false;


// extern SemaphoreHandle_t LL_upSysContextMutex;
extern TaskHandle_t upSystem;
extern bool upSystemInException;
uint32_t g_page_vram_fault_cnt = 0;
uint32_t g_page_vrom_fault_cnt = 0;

extern bool g_vm_in_pagefault;

uint8_t *VMMGR_GetCacheAddress() {
    return CachePage;
}

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
    switch (info->FSR)
    {
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
    case FSR_DATA_ACCESS_UNMAP:
        VM_Unconscious(info->FaultTask, "[MEMORY UNMAP]", info->FaultMemAddr);
        break;   
    default:
        VM_Unconscious(info->FaultTask, "[???]", info->FaultMemAddr);
        break;
    }
   
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

static void fetch_cache_and_move_to_tail() {

    CachePageCur = CachePageHead;

    CachePageHead = CachePageHead->next;
    CachePageHead->prev = NULL;

    CachePageTail->next = (CachePageInfo_t *)CachePageCur;
    CachePageCur->prev = (CachePageInfo_t *)CachePageTail;
    CachePageCur->next = NULL;

    CachePageTail = CachePageCur;
}

static void move_cache_page_to_tail(CachePageInfo_t *item) {
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
        fetch_cache_and_move_to_tail();
    }
}

static CachePageInfo_t *search_cache_page_by_vaddr(uint32_t vaddr) {
    CachePageInfo_t *tmp = (CachePageInfo_t *)CachePageTail;
    do {
        if (tmp->mapToVirtAddr == vaddr) {
            return tmp;
        }
        tmp = tmp->prev;
    } while (tmp);
    return NULL;
}

static int save_cache_page(CachePageInfo_t *cache_page) {
    int ret = 0;
    if (
        (cache_page->dirty) &&
        (cache_page->onPart == MAP_PART_FTL)) {
        mmu_clean_invalidated_dcache(cache_page->mapToVirtAddr, PAGE_SIZE);
        mmu_drain_buffer();
        ret = FTL_WriteSector(cache_page->onSector, 2, (uint8_t *)cache_page->PageOnPhyAddr);
        cache_page->dirty = false;
    }
    return ret;
}

void vmMgr_task() {
    pageFaultInfo_t currentFault;
    MapList_t *mapinfo;

    for (;;) {
        while (xQueueReceive(PageFaultQueue, &currentFault, portMAX_DELAY) == pdTRUE) {
            vTaskSuspend(currentFault.FaultTask);

            VM_INFO("REC FAULT TASK [%s] DAB. access %08x, FSR:%08x\n",
                    pcTaskGetName(currentFault.FaultTask), currentFault.FaultMemAddr, currentFault.FSR);

            if (vmMgr_CheckAddrVaild(currentFault.FaultMemAddr) == false) {
                taskAccessFaultAddr(&currentFault, "Area is not mapped.");
                continue;
            }

            switch (currentFault.FSR) {
            case FSR_DATA_ACCESS_UNMAP: {
                VM_INFO("Catch UnMap Mem:%s,%08x\n", pcTaskGetName(currentFault.FaultTask), currentFault.FaultMemAddr);
                mapinfo = mapList_findVirtAddrInWhichMap(currentFault.FaultMemAddr);
                if (mapinfo) {
                    fetch_cache_and_move_to_tail();
                    save_cache_page((CachePageInfo_t *)CachePageCur);
                    if (CachePageCur->mapToVirtAddr) {
                        mmu_unmap_page(CachePageCur->mapToVirtAddr);
                    }

                    CachePageCur->mapToVirtAddr = currentFault.FaultMemAddr & 0xFFFFF000;
                    CachePageCur->onPart = mapinfo->part;
                    CachePageCur->onSector = mapinfo->PartStartSector + ((currentFault.FaultMemAddr - mapinfo->VMemStartAddr) & 0xFFFFF000) / 2048;
                    CachePageCur->dirty = false;
                    int ret;

                    switch (mapinfo->part)
                    {
                    case MAP_PART_RAWFLASH:
                        g_page_vrom_fault_cnt++;
                        ret = MTD_ReadPhyPage(CachePageCur->onSector, 0, 2048, (uint8_t *)CachePageCur->PageOnPhyAddr);
                        ret = MTD_ReadPhyPage(CachePageCur->onSector + 1, 0, 2048, (uint8_t *)(CachePageCur->PageOnPhyAddr + 2048));
                        break;
                    case MAP_PART_FTL:
                        g_page_vram_fault_cnt++;
                        ret = FTL_ReadSector(CachePageCur->onSector, 2, (uint8_t *)CachePageCur->PageOnPhyAddr);
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
                        mmu_clean_invalidated_dcache(CachePageCur->mapToVirtAddr, PAGE_SIZE);
                        mmu_invalidate_icache();
                        mmu_invalidate_tlb();

                        // LL_CheckIRQAndTrap();
                        vTaskResume(currentFault.FaultTask);
                        g_vm_in_pagefault = false;
                        break;
                    }
                    taskAccessFaultAddr(&currentFault, "Remap Failed.");
                    break;
                }
                taskAccessFaultAddr(&currentFault, "Unknown Map.");
                break;
            }

            case FSR_DATA_WR_RDONLY: {
                VM_INFO("Catch WR RO Mem:%s,%08x\n", pcTaskGetName(currentFault.FaultTask), currentFault.FaultMemAddr);
                mapinfo = mapList_findVirtAddrInWhichMap(currentFault.FaultMemAddr);
                if (mapinfo) {
                    if (mapinfo->perm & PERM_W) {
                        CachePageInfo_t *gotCache = search_cache_page_by_vaddr(currentFault.FaultMemAddr & 0xFFFFF000);
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

bool vmMgr_checkAddressValid(uint32_t address, uint32_t perm) {
    if (address < MEMORY_SIZE) {
        return true;
    }
    #if (VMRAM_USE_FTL == 0)
        if((address >= VM_RAM_BASE) && (address <= VM_RAM_BASE + VM_RAM_SIZE_NONE_FTL))
        {
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

volatile uint8_t m_test_read;
uint32_t vmMgr_LoadPageGetPAddr(uint32_t vaddr) {
    if (!vmMgr_checkAddressValid(vaddr, PERM_R)) {
        return 0;
    }
    m_test_read = *((uint8_t *)vaddr);
    m_test_read--;

    for (int i = 0; i < NUM_CACHEPAGE; i++) {
        if (((CachePageInfo[i].mapToVirtAddr & 0xFFFFF000) == (vaddr & 0xFFFFF000))) {
            mmu_clean_invalidated_dcache(vaddr & 0xFFFFF000, PAGE_SIZE);
            return (CachePageInfo[i].PageOnPhyAddr + (vaddr & (PAGE_SIZE - 1)));
        }
    }

    return 0;
}

void vmMgr_ReleaseAllPage() {
    memset(CachePage, 0, sizeof(CachePage));
    memset(CachePageInfo, 0, sizeof(CachePageInfo));
    for (int i = 0; i < NUM_CACHEPAGE; i++) {
        CachePageInfo[i].dirty = false;
        CachePageInfo[i].onPart = -1;
        CachePageInfo[i].onSector = 0;
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

void vmMgr_init() {
    PageFaultQueue = xQueueCreate(32, sizeof(pageFaultInfo_t));

    mapListInit(); 
    mmu_init();

    vmMgr_ReleaseAllPage();
    
    DisplayPutStr(0, 16 * 1, "Waiting for Flash GC...", 0, 255, 16);
    for (int i = 0; i < FLASH_FTL_DATA_SECTOR  ; i++) {
        FTL_TrimSector(i);
    }
    
    
    mapList_AddPartitionMap(MAP_PART_RAWFLASH, PERM_R, VM_ROM_BASE, FLASH_SYSTEM_BLOCK * 64, VM_ROM_SIZE);

    #if (VMRAM_USE_FTL == 1)
        mapList_AddPartitionMap(MAP_PART_FTL, PERM_R | PERM_W, VM_RAM_BASE, 0, VM_RAM_SIZE);
    #else
        uint32_t paddr = (uint32_t)vm_ram_none_ftl;
        for(uint32_t vaddr = VM_RAM_BASE; vaddr < (VM_RAM_BASE + VM_RAM_SIZE_NONE_FTL); vaddr += PAGE_SIZE)
        {
            mmu_map_page(vaddr, paddr, AP_SYSRW_USRRW, VM_CACHE_ENABLE, VM_BUFFER_ENABLE);
            paddr += PAGE_SIZE;
        }
        mmu_invalidate_tlb();
        //

    #endif
    // xTaskCreate(vVmBlockTask, "VM Swap IO Waiting", 12, NULL, 6, &vmBlockTask);
    // vTaskSuspend(vmBlockTask);

    for (int i = 0; i < NUM_CACHEPAGE; i++) {
        VM_INFO("CACHE_PAGEn_BASE:%d,%08x\n", i, CACHE_PAGEn_BASE(i));
    }
    INFO("Virtual Memory Enable.\n");
    vmMgrInit = true;
}

bool vmMgrInited() {
    return vmMgrInit;
}
