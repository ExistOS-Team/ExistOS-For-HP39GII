#ifndef __VMMGR_H__
#define __VMMGR_H__


#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "ff.h"

#define    SWAP_PART    (2)

#define    PERM_None    (0)
#define    PERM_R       (1)     
#define    PERM_W       (1 << 1)

#define    FSR_DATA_UNALIGN             1
#define    FSR_DATA_ACCESS_UNMAP        2
#define    FSR_DATA_WR_RDONLY           3
#define    FSR_INST_FETCH               4
#define    FSR_UNKNOWN                  0xF

typedef struct pageFaultInfo_t
{
    TaskHandle_t FaultTask;
    uint32_t FaultMemAddr;
    uint32_t FSR;
}pageFaultInfo_t;

QueueHandle_t PageFaultQueue;

void vmMgr_init(void);
void vmMgr_task(void);
bool vmMgrInited(void);
void vmMgr_mapSwap(void);

uint32_t vmMgr_mapFile(FIL *file, uint32_t perm, uint32_t MemAddrStart, uint32_t FileAddrStart, uint32_t memSize);


#endif