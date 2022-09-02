#pragma once 

#include <stdint.h>
#include "Fatfs/ff.h"

#define MEM_PAGE_SIZE   (1024)

typedef struct VROMMapInfo_t
{
    struct VROMMapInfo_t *next;
    uint32_t map_vm_addr;
    uint32_t map_file_start;
    uint32_t map_size;
    FIL *map_f;

}VROMMapInfo_t;
 
void VROMLoader_Initialize();
int VROMLoaderCreateFileMap(FIL *f, uint32_t inFileStart, uint32_t memAddress, uint32_t mapSize);
int VROMIRQLoad(uint32_t vaddr);
int VROMLoaderDeleteMap(uint32_t vaddr);


