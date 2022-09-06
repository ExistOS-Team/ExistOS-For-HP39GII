#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"

#include "VROMLoader.h"

static VROMMapInfo_t *vmmap_list;

extern bool g_system_in_emulator;

void VROMLoader_Initialize() {
    printf("Running in Emulator:%d\n", g_system_in_emulator);

}

static inline int VROMMapCheck(uint32_t memAddress, uint32_t mapSize) {
    VROMMapInfo_t *map = vmmap_list;
    if (map == NULL) {
        return 0;
    }

    while (map) {
        if (map->next) {
            if ((memAddress >= map->map_vm_addr) && (memAddress < map->map_vm_addr + mapSize)) {
                return 1;
            }
            if (((memAddress + mapSize) >= map->map_vm_addr) && ((memAddress + mapSize) < (map->map_vm_addr + map->map_size))) {
                return 1;
            }
            map = map->next;
        }else{
            break;
        }
    }
}

int VROMLoaderCreateFileMap(FIL *f, uint32_t inFileStart, uint32_t memAddress, uint32_t mapSize) {
    /*
    FRESULT fr;
    FIL *f = pvPortMalloc(sizeof(FIL));
    if(!f)
    {
        return -1;
    }
    fr = f_open(f, filePath, FA_OPEN_EXISTING | FA_READ);

    if(fr)
    {
        vPortFree(f);
        return -1;
    }
    */

    VROMMapInfo_t *map;
    if (vmmap_list == NULL) {
        vmmap_list = pvPortMalloc(sizeof(VROMMapInfo_t));
        if (!vmmap_list) {
            return -1;
        }
        map = vmmap_list;
    } else {
        map = vmmap_list;
        while (map->next) {
            map = map->next;
        }
        map->next = pvPortMalloc(sizeof(VROMMapInfo_t));
        if (!map->next) {
            return -1;
        }
        map = map->next;
    }

    memset(map, 0, sizeof(VROMMapInfo_t));

    map->map_f = f;
    map->map_file_start = inFileStart;
    map->map_vm_addr = memAddress;
    map->map_size = mapSize;
    map->next = NULL;

    if(g_system_in_emulator)
    {
        UINT br;
        f_lseek(f, inFileStart);
        f_read(f, (void *)memAddress, mapSize, &br);
        f_lseek(f, 0);
    }

    return 0;
}

static VROMMapInfo_t *findMappedMap(uint32_t addr) {
    if (!vmmap_list) {
        return NULL;
    }
    VROMMapInfo_t *map = vmmap_list;

    while (map) {
        if ((addr >= map->map_vm_addr) && (addr < map->map_vm_addr + map->map_size)) {
            return map;
        }
        map = map->next;
    }

    return NULL;
}

int VROMLoaderDeleteMap(uint32_t vaddr) {
    if (!vmmap_list) {
        return 0;
    }
    VROMMapInfo_t *map = vmmap_list;
    VROMMapInfo_t *map_prev = vmmap_list;

    while (map) {
        if ((vaddr >= map->map_vm_addr) && (vaddr < map->map_vm_addr + map->map_size)) {
            if (map == vmmap_list) {
                vPortFree(map);
                vmmap_list = NULL;
                return 0;
            } else {
                map_prev->next = NULL;
                vPortFree(map);
                return 0;
            }
        }
        map_prev = map;
        map = map->next;
    }

    return 0;
}

int VROMIRQLoad(uint32_t vaddr) {
    UINT br;
    VROMMapInfo_t *map = findMappedMap(vaddr);
    if (map) {
        f_lseek(map->map_f, vaddr - map->map_vm_addr + map->map_file_start);
        f_read(map->map_f, (uint8_t *)vaddr, MEM_PAGE_SIZE, &br);
    }
}
