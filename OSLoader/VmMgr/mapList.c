

#include "FreeRTOS.h"

#include "vmMgr.h"
#include "mapList.h"

#include "../debug.h"

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
