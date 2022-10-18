#ifndef __FTL_UP_H__
#define __FTL_UP_H__


#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"

#include "SystemConfig.h"

#include <stdint.h>
#include <stdbool.h>
#include "map.h"

#define BAD_BLOCK           (0)
#define DATA_BLOCK          (0xDADADADA)
#define GOOD_BLOCK          (0xFFFFFFFF)

#define DATA_START_BLOCK    FLASH_DATA_BLOCK
#define GC_RATIO        6

typedef enum {
    FTL_SECTOR_READ,
    FTL_SECTOR_WRITE,
    FTL_SECTOR_TRIM,
    FTL_SYNC
    
}FTL_OPAS;

typedef struct
{
    FTL_OPAS opa;
    uint32_t sector;
    uint32_t num;
    uint8_t *buf;
    //EventBits_t BLock;
    //int32_t *StatusBuf;
    TaskHandle_t task;
}FTL_Operates;

typedef struct PartitionInfo_t
{
  uint32_t Partitions;
  uint32_t SectorStart[4];
  uint32_t Sectors[4];
}PartitionInfo_t;

int FTL_init(void);
int FTL_MapInit(void);
bool FTL_inited(void);
void FTL_task(void);

int FTL_GetSectorCount(void);
int FTL_GetSectorSize(void);

int FTL_ReadSector(uint32_t sector, uint32_t num, uint8_t *buf);
int FTL_WriteSector(uint32_t sector, uint32_t num, uint8_t *buf);
int FTL_TrimSector(uint32_t sector);
int FTL_Sync(void);
void FTL_ClearAllSector(void);

bool FTL_ScanPartition(void);
PartitionInfo_t *FTL_GetPartitionInfo(void);

#endif