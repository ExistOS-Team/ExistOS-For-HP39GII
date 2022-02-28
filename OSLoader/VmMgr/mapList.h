
#ifndef __MAPLIST_H__
#define __MAPLIST_H__

#include "ff.h"

typedef struct MapList_t
{
    struct MapList_t *next;
    FIL *file;
    uint32_t perm;
    uint32_t VMemStartAddr;
    uint32_t FileStartAddr;
    uint32_t memSize;
    int part;
    uint32_t PartStartSector;

}MapList_t;

void mapListInit(void);

MapList_t *mapList_checkCollision(FIL *file, uint32_t VMemStartAddr, uint32_t FileStartAddr, uint32_t memSize);
uint32_t mapList_AddFileMap(FIL *file, uint32_t perm, uint32_t VMemStartAddr, uint32_t FileStartAddr, uint32_t memSize);
MapList_t *mapList_findVirtAddrInWhichMap(uint32_t Addr);

uint32_t mapList_AddPartitionMap(int part, uint32_t perm, uint32_t VMemStartAddr, uint32_t PartStartSector, uint32_t memSize);


#endif
