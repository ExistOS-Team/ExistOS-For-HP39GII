



#include "SystemConfig.h"
#include "ff.h"
#include "FTL_up.h"

#include "../debug.h"

#include "vmMgr.h"
#include "mmu.h"
#include "mapList.h"



#include "queue.h"

typedef struct CachePageInfo_t
{
    uint32_t mapToVirtAddr;
    uint32_t PageOnPhyAddr;
    FIL *onFile;
    uint32_t onFileAddr;
    int32_t onPart;
    uint32_t onSector;
    uint32_t LRUCount;
    bool dirty;
}CachePageInfo_t;

static uint8_t CachePage[PAGE_SIZE * NUM_CACHEPAGE] __attribute__((aligned(PAGE_SIZE)));
static CachePageInfo_t CachePageInfo[NUM_CACHEPAGE];

#define CACHE_PAGEn_BASE(n)     (uint32_t)(&CachePage[n * PAGE_SIZE])

static FATFS fsys;
static FRESULT fres;
static FIL fswap;
static bool vmMgrInit = false;


static uint32_t SwapPartSectorStart;
static uint32_t SwapPartSectors;


uint32_t vmMgr_mapFile(FIL *file, uint32_t perm, uint32_t MemAddrStart, uint32_t FileAddrStart, uint32_t memSize)
{
    if(mapList_checkCollision(file, MemAddrStart, FileAddrStart, memSize) == NULL)
    {
        mapList_AddFileMap(file, perm, MemAddrStart, FileAddrStart, memSize);
        return 0;
    }
    return 1;
}

inline static bool vmMgr_CheckAddrVaild(uint32_t addr)
{
    MapList_t *L;
    L = mapList_findVirtAddrInWhichMap(addr);

    if(L == NULL)
    {
        return false;
    }

    if((addr - L->VMemStartAddr) >= L->memSize){
        return false;
    }

    return true;
}

inline static uint32_t findOptimisticCachePage()
{
    uint32_t MaxLRU = 0;
    uint32_t MaxLRU_i = 0;
    for(int i = 0; i < NUM_CACHEPAGE; i++)
    {
        if(CachePageInfo[i].mapToVirtAddr == 0xFFFFFFFF){
            return i;
        }
    }

    for(int i = 0; i < NUM_CACHEPAGE; i++)
    {
        if(CachePageInfo[i].LRUCount > MaxLRU)
        {
            MaxLRU = CachePageInfo[i].LRUCount;
            MaxLRU_i = i;
        }
    }

    for(int i = 0; i < NUM_CACHEPAGE; i++)
    {
        CachePageInfo[i].LRUCount++;
    }

    CachePageInfo[MaxLRU_i].LRUCount = 0;
    return MaxLRU_i;
}

inline static uint32_t findVirtAddrInCachePage(uint32_t virtAddr)
{
    for(int i = 0; i < NUM_CACHEPAGE; i++)
    {
        if((CachePageInfo[i].mapToVirtAddr / PAGE_SIZE) == (virtAddr / PAGE_SIZE)){
            return i;
        }
    }
    return 0xFFFFFFFF;
}

inline static void mapCachePageInNewVirtMem(uint32_t CachePage_i, uint32_t newVirtMemAddr, MapList_t *mapinfo)
{
    FRESULT fres;
    UINT bw;
    uint32_t faddr ;
    uint32_t sec;
    if(CachePageInfo[CachePage_i].dirty)
    {
        

        mmu_clean_invalidated_dcache(CachePageInfo[CachePage_i].mapToVirtAddr, PAGE_SIZE);


        if(CachePageInfo[CachePage_i].onPart == -1){
            faddr = CachePageInfo[CachePage_i].onFileAddr;
            fres = f_lseek(CachePageInfo[CachePage_i].onFile, faddr);
            VM_INFO("save page flseek:%d, faddr:%08x\n",fres);
            fres = f_write(CachePageInfo[CachePage_i].onFile, (uint32_t *)CachePageInfo[CachePage_i].PageOnPhyAddr, PAGE_SIZE, &bw);
            VM_INFO("save page fwrite:%d, bw:%d\n",fres, bw);
        }else{

            
            FTL_WriteSector(CachePageInfo[CachePage_i].onSector, 2, (uint8_t *)CachePageInfo[CachePage_i].PageOnPhyAddr);
            VM_INFO("Write Sec:%d\n",CachePageInfo[CachePage_i].onSector);
        }
        CachePageInfo[CachePage_i].dirty = false;
    }

    mmu_unmap_page(CachePageInfo[CachePage_i].mapToVirtAddr);
    
    if(mapinfo->part == -1){

        faddr = mapinfo->FileStartAddr + ((newVirtMemAddr & 0xFFFFF000) - mapinfo->VMemStartAddr);
        fres = f_lseek(mapinfo->file, faddr );
        VM_INFO("load page flseek:%d, faddr:%08x\n",fres, faddr);

        fres = f_read(mapinfo->file, (uint32_t *)CachePageInfo[CachePage_i].PageOnPhyAddr, PAGE_SIZE, &bw);

        VM_INFO("load page fread:%d, bw:%d\n",fres, bw);

        CachePageInfo[CachePage_i].mapToVirtAddr = newVirtMemAddr & 0xFFFFF000;
        CachePageInfo[CachePage_i].onFile = mapinfo->file;
        CachePageInfo[CachePage_i].onFileAddr = faddr;
        CachePageInfo[CachePage_i].onPart = -1;


    }else{

        sec =  mapinfo->PartStartSector + ( (newVirtMemAddr - mapinfo->VMemStartAddr) & 0xFFFFF000 ) / 2048;

        if(CachePageInfo[CachePage_i].mapToVirtAddr != 0xFFFFFFFF)  //has been mapped?
        {
            FTL_ReadSector(sec, 2, (uint8_t *)CachePageInfo[CachePage_i].PageOnPhyAddr);
        }else{
            memset((uint8_t *)CachePageInfo[CachePage_i].PageOnPhyAddr, 0, PAGE_SIZE);
        }

        CachePageInfo[CachePage_i].mapToVirtAddr = newVirtMemAddr & 0xFFFFF000;
        CachePageInfo[CachePage_i].onFile = NULL;
        CachePageInfo[CachePage_i].onFileAddr = 0;

        CachePageInfo[CachePage_i].onPart = mapinfo->part;
        CachePageInfo[CachePage_i].onSector = sec;

        //INFO("Read Sec:%d\n",sec);
    
        
    }

    CachePageInfo[CachePage_i].dirty = false;
    mmu_map_page(newVirtMemAddr, CachePageInfo[CachePage_i].PageOnPhyAddr, 
    AP_READONLY, true, true);

    mmu_clean_invalidated_dcache(CachePageInfo[CachePage_i].mapToVirtAddr, PAGE_SIZE);
    mmu_invalidate_icache();


}

inline static void remapCacheWithRW(uint32_t ind)
{


    CachePageInfo[ind].LRUCount = 0;

    mmu_unmap_page(CachePageInfo[ind].mapToVirtAddr);

    mmu_map_page(CachePageInfo[ind].mapToVirtAddr, 
        CachePageInfo[ind].PageOnPhyAddr, 
        AP_SYSRW_USRRW, true, true);

}

static void vmMgr_createSwapfile()
{
    UINT bw;
    fres = f_mount(&fsys, "/SYS/", 1);
    VM_INFO("vm_mount:%d\n", fres);
    fres = f_open(&fswap, "/SYS/swapfile", 	FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
    VM_INFO("vm_open:%d\n", fres);

    fres = f_lseek(&fswap, SIZE_SWAPAREA_MB * 1048576);
    VM_INFO("f_lseek:%d\n", fres);
    fres = f_write(&fswap, "1", 1, &bw);
    fres = f_sync(&fswap);
    VM_INFO("f_sync:%d\n", fres);
}

static void mkswap()
{
    PartitionInfo_t *p;
    p = FTL_GetPartitionInfo();

    SwapPartSectorStart = p->SectorStart[2];
    SwapPartSectors = p->Sectors[2];
    
    for(int i = p->SectorStart[2]; i < p->SectorStart[2] + p->Sectors[2]; i++)
    {
        FTL_TrimSector(i);
    }
    VM_INFO("Swap FTL_TrimSector Fin\n");
    
}

static void taskAccessFaultAddr(pageFaultInfo_t *info)
{
    VM_ERR("Access Invalid Address.\n");
}

void vmMgr_task()
{
    pageFaultInfo_t currentFault;
    uint32_t OptimisticCachePage;
    MapList_t *mapinfo;

    for(;;)
    {
        while(xQueueReceive(PageFaultQueue, &currentFault, portMAX_DELAY) == pdTRUE){
            vTaskSuspend(currentFault.FaultTask);
            /*
            VM_INFO("REC FAULT TASK [%s] DAB. access %08x, FSR:%08x\n", 
                pcTaskGetName(currentFault.FaultTask), currentFault.FaultMemAddr, currentFault.FSR);*/

            if(vmMgr_CheckAddrVaild(currentFault.FaultMemAddr) == false)
            {
                    taskAccessFaultAddr(&currentFault);
                    continue;
            }

            switch (currentFault.FSR)
            {
            case FSR_DATA_ACCESS_UNMAP:
                {
                    
                    VM_INFO("In Map Area, Need To Map.\n");
                    mapinfo = mapList_findVirtAddrInWhichMap(currentFault.FaultMemAddr);
                    //if((currentFault.FaultMemAddr - mapinfo->VMemStartAddr) > mapinfo->memSize)


                    OptimisticCachePage = findOptimisticCachePage();
                    VM_INFO("got OptimisticCachePage:%d\n", OptimisticCachePage);
                    mapCachePageInNewVirtMem(OptimisticCachePage, currentFault.FaultMemAddr, mapinfo);
                    mmu_invalidate_tlb();
                    
                    //mmu_dumpMapInfo();

                    vTaskResume(currentFault.FaultTask);
                }
                break;
            
            case FSR_DATA_WR_RDONLY:
                {
                    mapinfo = mapList_findVirtAddrInWhichMap(currentFault.FaultMemAddr);
                    if((mapinfo->perm & PERM_W) == 0){
                        VM_ERR("READ ONLY PAGE!\n");
                        taskAccessFaultAddr(&currentFault);
                        continue;
                    }

                    OptimisticCachePage = findVirtAddrInCachePage(currentFault.FaultMemAddr);
                    VM_INFO("got DAB RDONLY Page:%d\n", OptimisticCachePage);
                    if(OptimisticCachePage == 0xFFFFFFFF){
                        taskAccessFaultAddr(&currentFault);
                        continue;
                    }
                    CachePageInfo[OptimisticCachePage].dirty = true;
                    remapCacheWithRW(OptimisticCachePage);
                    mmu_invalidate_tlb();


                    //mmu_dumpMapInfo();

                    vTaskResume(currentFault.FaultTask);
                }
                break;
            default:
                VM_ERR("Unknown Task Operation.\n");
                break;
            }

        }
    }
}

void vmMgr_init()
{
    PageFaultQueue = xQueueCreate(32, sizeof(pageFaultInfo_t));

    memset(CachePage, 0, sizeof(CachePage));
    for(int i = 0; i < NUM_CACHEPAGE; i++){
        CachePageInfo[i].dirty = false;
        CachePageInfo[i].LRUCount = 9999;
        CachePageInfo[i].onFile = NULL;
        CachePageInfo[i].onFileAddr = 0;
        CachePageInfo[i].onPart = -1;
        CachePageInfo[i].onSector = 0;        
        CachePageInfo[i].PageOnPhyAddr = CACHE_PAGEn_BASE(i);
        CachePageInfo[i].mapToVirtAddr = 0xFFFFFFFF;
    }
    
    //vmMgr_createSwapfile();
    mkswap();

    mapListInit();

    //mapList_AddFileMap(&fswap, PERM_R | PERM_W, VM_RAM_BASE, 0, SIZE_SWAPAREA_MB * 1048576);
    

    mapList_AddPartitionMap(
        2,
        PERM_R | PERM_W,
        VM_RAM_BASE,
        SwapPartSectorStart,
        SIZE_SWAPAREA_MB * 1048576
        );
    

    mmu_init();


    for(int i =0 ;i< NUM_CACHEPAGE; i++)
    {
        VM_INFO("CACHE_PAGEn_BASE:%d,%08x\n",i,CACHE_PAGEn_BASE(i));

    }
    VM_INFO("Virtual Memory Enable.\n");
    vmMgrInit = true;


}


bool vmMgrInited()
{
    return vmMgrInit;
}
