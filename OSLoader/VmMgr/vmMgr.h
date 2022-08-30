#ifndef __VMMGR_H__
#define __VMMGR_H__


#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"


#include "mmu.h"
#include "SystemConfig.h"


#define    PERM_None    (0)
#define    PERM_R       (1)     
#define    PERM_W       (1 << 1)

#define    FSR_DATA_UNALIGN             1
#define    FSR_DATA_ACCESS_UNMAP_DAB    2
#define    FSR_DATA_ACCESS_UNMAP_PAB    3
#define    FSR_DATA_WR_RDONLY           4
#define    FSR_INST_FETCH               5
#define    FSR_UNKNOWN                  0xF

typedef struct pageFaultInfo_t
{
    TaskHandle_t FaultTask;
    uint32_t FaultMemAddr;
    uint32_t FSR;
    uint32_t FaultPC;
}pageFaultInfo_t;

extern QueueHandle_t PageFaultQueue;

void vmMgr_init(void);
void vmMgr_task(void);
bool vmMgrInited(void);
void vmMgr_mapSwap(void);

uint32_t vmMgr_LoadPageGetPAddr(uint32_t vaddr);
//int vmMgr_unlockMap(uint32_t vaddr);
uint8_t *VMMGR_GetCacheAddress(void);
bool vmMgr_checkAddressValid(uint32_t address, uint32_t perm);
void vmMgr_ReleaseAllPage();


//uint32_t vmMgr_getMountPhyAddressAndLock(uint32_t vaddr, uint32_t perm);


//uint32_t vmMgr_mapFile(FIL *file, uint32_t perm, uint32_t MemAddrStart, uint32_t FileAddrStart, uint32_t memSize);


#endif