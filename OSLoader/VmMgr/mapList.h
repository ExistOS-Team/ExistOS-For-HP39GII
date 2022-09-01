
#ifndef __MAPLIST_H__
#define __MAPLIST_H__

#define MAP_PART_RAWFLASH   1
#define MAP_PART_FTL        2
#define MAP_PART_SYS        3

typedef struct MapList_t
{
    struct MapList_t *next;
    uint32_t perm;
    uint32_t VMemStartAddr;
    uint32_t PartStartSector;
    uint32_t memSize;
    int part;

}MapList_t;

void mapListInit(void);
MapList_t *mapList_findVirtAddrInWhichMap(uint32_t Addr);
uint32_t mapList_AddPartitionMap(int part, uint32_t perm, uint32_t VMemStartAddr, uint32_t PartStartSector, uint32_t memSize);


#endif
