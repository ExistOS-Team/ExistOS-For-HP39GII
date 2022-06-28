

#include "FreeRTOS.h"

#include "vmMgr.h"
#include "mapList.h"

#include "../debug.h"

MapList_t *maplist;


void mapListInit()
{
    maplist = pvPortMalloc(sizeof(MapList_t));
    memset(maplist, 0, sizeof(MapList_t));
}


uint32_t mapList_AddPartitionMap(int part, uint32_t perm, uint32_t VMemStartAddr, uint32_t PartStartSector, uint32_t memSize)
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
/*
uint32_t mapList_AddFileMap(FIL *file, uint32_t perm, uint32_t VMemStartAddr, uint32_t FileStartAddr, uint32_t memSize)
{
    MapList_t *tmp = pvPortMalloc(sizeof(MapList_t));
    MapList_t *chain = maplist;
    if(tmp == NULL){
        return 1;
    }

    tmp->next = NULL;
    tmp->file = file;
    tmp->perm = perm;
    tmp->VMemStartAddr = VMemStartAddr;
    tmp->FileStartAddr = FileStartAddr;
    tmp->memSize = memSize;
    tmp->part = -1;
    tmp->PartStartSector = 0;

    while(chain->next != NULL){
        chain = chain->next;
    }
    chain->next = tmp;
    return 0;
}*/

MapList_t *mapList_findVirtAddrInWhichMap(uint32_t Addr)
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