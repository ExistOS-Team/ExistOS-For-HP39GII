#ifndef __MTD_UP_H__
#define __MTD_UP_H__

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef struct 
{
    uint32_t PageSize_B;
    uint32_t SpareSizePerPage_B;
    uint32_t BlockSize_KB;
    uint32_t PagesPerBlock;
    uint32_t MetaSize_B;
    uint32_t Blocks;
}mtdInfo_t;


typedef enum {
    MTD_PHY_READ,
    MTD_PHY_WRITE,
    MTD_PHY_ERASE,
    MTD_PHY_READ_META,
    MTD_PHY_WRITE_META,
    MTD_PHY_COPY
    
}MTD_OPAS;

typedef struct
{
    MTD_OPAS opa;
    uint32_t page;
    uint32_t copyDstPage;
    uint32_t offset;
    uint8_t *buf;
    uint8_t *metaDat;
    uint32_t len;
    bool needToMoveData;
    TaskHandle_t task;
    
}MTD_Operates;


void portMTDInterfaceInit(void);
void portMTDDeviceInit(mtdInfo_t *mtdinfo);
void portMTDReadPage(uint32_t page, uint8_t *buf);
void portMTDWritePage(uint32_t page, uint8_t *buf);
void portMTDEraseBlock(uint32_t block);
uint8_t *portMTDGetMetaData(void);
void portMTDWritePageMeta(uint32_t page, uint8_t *buf, uint8_t *metaBuf);

void portMTDCopyPage(uint32_t src, uint32_t dst);

bool MTD_upOpaFin(uint32_t eccResult);

void MTD_InterfaceInit(void);
void MTD_DeviceInit(void);
void MTD_Task(void);
bool MTD_isDeviceInited(void);
mtdInfo_t *MTD_getDeviceInfo();

int MTD_ReadPhyPage(uint32_t page, uint32_t offset, uint32_t len, uint8_t *buffer);
int MTD_WritePhyPage(uint32_t page,uint8_t *buffer);
int MTD_ErasePhyBlock(uint32_t block);
//int MTD_WritePhyPageMeta(uint32_t page, uint32_t len, uint8_t *buffer);

int MTD_WritePhyPageWithMeta(uint32_t page, uint32_t meta_len, uint8_t *buffer, uint8_t *meta);

int MTD_ReadPhyPageMeta(uint32_t page, uint32_t len, uint8_t *buffer);
int MTD_CopyPhyPage(uint32_t srcPage, uint32_t dstPage);
int MTD_EraseAllBLock(void);


#endif