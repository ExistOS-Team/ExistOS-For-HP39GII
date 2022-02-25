#ifndef __FTL_UP_H__
#define __FTL_UP_H__


#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"

#include <stdint.h>
#include <stdbool.h>
#include "map.h"


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
    EventBits_t BLock;
    int32_t *StatusBuf;
}FTL_Operates;

int FTL_init(void);
bool FTL_inited(void);
void FTL_task(void);

int FTL_GetSectorCount(void);
int FTL_GetSectorSize(void);

int FTL_ReadSector(uint32_t sector, uint32_t num, uint8_t *buf);
int FTL_WriteSector(uint32_t sector, uint32_t num, uint8_t *buf);
int FTL_TrimSector(uint32_t sector);
int FTL_Sync(void);
void FTL_ClearAllSector(void);


#endif