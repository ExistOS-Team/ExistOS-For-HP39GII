
#include "FreeRTOS.h"

#include "mtd_up.h"
#include "nand.h"
#include "../debug.h"

static QueueHandle_t MTD_Operates_Queue;
static EventGroupHandle_t MTDLockEventGroup;
static EventGroupHandle_t MTDDriverOpaDone;

static mtdInfo_t mtdinfo;
static MTD_Operates curOpa;
static bool deviceInited = false;
static int32_t MTDLockBit = 0;

static uint32_t ECCResult;

static int32_t MTDStatusBuf[32];
static int32_t pMTDStatus = 0;

static uint8_t *pMetadata;


void MTD_InterfaceInit()
{
    portMTDInterfaceInit();
}


static uint8_t MTD_PageBuffer[ 2048 ];

bool MTD_isDeviceInited()
{
    return deviceInited;
}

mtdInfo_t *MTD_getDeviceInfo()
{
    return &mtdinfo;
}

bool MTD_upOpaFin(uint32_t eccResult)
{
    BaseType_t flag;

    ECCResult = eccResult;
    xEventGroupSetBitsFromISR(MTDDriverOpaDone, 1, &flag);

    return flag;

}


void MTD_Task()
{

    while(1){
        
        if(xQueueReceive(MTD_Operates_Queue, &curOpa, portMAX_DELAY) == pdTRUE)
        {


            switch (curOpa.opa)
            {

            case MTD_PHY_READ:

                MTD_INFO("QUEUE READ, page:%d, buf:%p, len: %d, needMove:%d\n",curOpa.page, curOpa.buf, curOpa.len, curOpa.needToMoveData);

                if(curOpa.needToMoveData)
                {
                    MTD_INFO("Move Dat Read,page:%d,%p\n",curOpa.page,MTD_PageBuffer);
                    portMTDReadPage(curOpa.page, MTD_PageBuffer);
                }else{
                    portMTDReadPage(curOpa.page, curOpa.buf);
                }
                
                break;
            case MTD_PHY_READ_META:
                portMTDReadPage(curOpa.page, NULL);
                break;


            case MTD_PHY_ERASE:
                portMTDEraseBlock(curOpa.page);
                break;
            case MTD_PHY_WRITE:
                if(curOpa.needToMoveData)
                {
                    memcpy(MTD_PageBuffer, curOpa.buf, mtdinfo.PageSize_B);
                    
                    portMTDWritePage(curOpa.page, MTD_PageBuffer);

                }else{
                    portMTDWritePage(curOpa.page, curOpa.buf);
                }
                
                break;
            case MTD_PHY_WRITE_META:
                memset(portMTDGetMetaData(), 0xFF, mtdinfo.MetaSize_B);
                memcpy(portMTDGetMetaData(), curOpa.metaDat, curOpa.len);

                if(curOpa.needToMoveData)
                {
                    memcpy(MTD_PageBuffer, curOpa.buf, mtdinfo.PageSize_B);
                    portMTDWritePageMeta(curOpa.page, MTD_PageBuffer,portMTDGetMetaData());

                }else{
                    portMTDWritePageMeta(curOpa.page, curOpa.buf,portMTDGetMetaData());
                }

                
                break;

            case MTD_PHY_COPY:
                portMTDCopyPage(curOpa.page, curOpa.copyDstPage);
                break;
                
            default:
                break;
            }
            
            xEventGroupWaitBits(MTDDriverOpaDone, 1, pdTRUE, pdFALSE, portMAX_DELAY);


            switch (curOpa.opa)
            {
            case MTD_PHY_READ_META:
                memcpy(curOpa.buf, portMTDGetMetaData(), curOpa.len);
            case MTD_PHY_READ:
                if(curOpa.needToMoveData)
                {
                    if(curOpa.len > mtdinfo.PageSize_B - curOpa.offset){
                        MTD_WARN("MTD READ DATA Corrupted.page:%d len:%d offset:%d\n",curOpa.page, curOpa.len, curOpa.offset);
                        curOpa.len = mtdinfo.PageSize_B - curOpa.offset;
                    }
                    MTD_INFO("Read Move Data,dst:%p,src:%p,len:%d\n",curOpa.buf, MTD_PageBuffer + curOpa.offset, curOpa.len);
                    memcpy(curOpa.buf, MTD_PageBuffer + curOpa.offset, curOpa.len);
                }

                if( 
                    (((ECCResult ) & 0xF)      == 0xE) || 
                    (((ECCResult >> 8) & 0xF)  == 0xE) || 
                    (((ECCResult >> 16) & 0xF) == 0xE) || 
                    (((ECCResult >> 24) & 0xF) == 0xE) 
                ){
                    *curOpa.StatusBuf =  -1;
                }else if(ECCResult == 0x0F0F0F0F){
                    *curOpa.StatusBuf = 1;          //EMPTY
                }else {
                    *curOpa.StatusBuf = 0;
                }
                //printf("READ ECC:%08X\n",ECCResult);
                break;
            
                

                case MTD_PHY_ERASE:
                    *curOpa.StatusBuf = ECCResult;
                    break;

                case MTD_PHY_WRITE_META:
                case MTD_PHY_WRITE:
                    *curOpa.StatusBuf = ECCResult;
                    break;

                case MTD_PHY_COPY:
                    if( 
                        (((ECCResult ) & 0xF)      == 0xE) || 
                        (((ECCResult >> 8) & 0xF)  == 0xE) || 
                        (((ECCResult >> 16) & 0xF) == 0xE) || 
                        (((ECCResult >> 24) & 0xF) == 0xE) 
                    ){
                        *curOpa.StatusBuf =  -1;
                    }else if(ECCResult == 0x0F0F0F0F){
                        *curOpa.StatusBuf = 0;      //EMPTY
                    }else {
                        *curOpa.StatusBuf = 0;
                    }

                    break;

            default:
                break;
            }

            xEventGroupSetBits(MTDLockEventGroup , (1 << curOpa.BLock));
        }
        portYIELD();
    }
}

static EventBits_t MTD_getLock()
{
    uint32_t bit;
    EventBits_t GroupBits;
    EventBits_t curBits;

    taskENTER_CRITICAL();
    
    GroupBits = xEventGroupGetBits(MTDLockEventGroup);

    curBits = (GroupBits >> MTDLockBit) & 1;
    while(curBits == 1)
    {
        GroupBits = xEventGroupGetBits(MTDLockEventGroup);
        MTDLockBit++;
        if(MTDLockBit > 23)
        {
            MTDLockBit = 0;
        }
        curBits = (GroupBits >> MTDLockBit) & 1;
    }

    bit = MTDLockBit;
    xEventGroupClearBits(MTDLockEventGroup, 1 << bit);

    MTDLockBit++;
    if(MTDLockBit > 23)
    {
        MTDLockBit = 0;
    }
    
    taskEXIT_CRITICAL();
    return bit;

}

static uint32_t *MTD_GetStatusBuf()
{
    int32_t *StatusBuf;
    taskENTER_CRITICAL();

    StatusBuf = &MTDStatusBuf[pMTDStatus];
    while(*StatusBuf != -5){
        pMTDStatus++;
        if(pMTDStatus >= (sizeof(MTDStatusBuf)/sizeof(int32_t))){
            pMTDStatus = 0;
        }
        StatusBuf = &MTDStatusBuf[pMTDStatus];
    }

    *StatusBuf = 0;

    pMTDStatus++;
    if(pMTDStatus >= (sizeof(MTDStatusBuf)/sizeof(int32_t))){
        pMTDStatus = 0;
    }
    //MTD_INFO("pMTDStatus:%d\n",pMTDStatus);
    taskEXIT_CRITICAL();
    return StatusBuf;
}



int MTD_ReadPhyPage(uint32_t page, uint32_t offset, uint32_t len, uint8_t *buffer)
{
    MTD_Operates newOpa;
    int retVal;

    newOpa.opa = MTD_PHY_READ;
    newOpa.page = page;
    newOpa.offset = offset;
    newOpa.buf = buffer;
    newOpa.len = len;
    newOpa.BLock = MTD_getLock();
    newOpa.StatusBuf = MTD_GetStatusBuf();
    newOpa.needToMoveData = false;

    if((offset != 0) || (len != mtdinfo.PageSize_B) || (((uint32_t)buffer % 4) != 0)){
        newOpa.needToMoveData = true;
    }
    if(buffer == NULL){
        newOpa.needToMoveData = false;
    }
    while (!deviceInited)
    {
        portYIELD();
    }
    MTD_INFO("POST READ CMD, queue num:%d\n",uxQueueGetQueueNumber(MTD_Operates_Queue));
    
    xQueueSend(MTD_Operates_Queue, &newOpa, portMAX_DELAY);

    MTD_INFO("POST READ CMD END\n");

    xEventGroupWaitBits(MTDLockEventGroup, (1 << newOpa.BLock), pdTRUE, pdFALSE, portMAX_DELAY);


    retVal = *newOpa.StatusBuf;
    *newOpa.StatusBuf = -5;
    MTD_INFO("POST READ RET\n");
    return retVal;
}

int MTD_WritePhyPage(uint32_t page,uint8_t *buffer)
{
    MTD_Operates newOpa;
    int retVal = 0;

    newOpa.opa = MTD_PHY_WRITE;
    newOpa.page = page;
    newOpa.buf = buffer;
    newOpa.BLock = MTD_getLock();
    newOpa.StatusBuf = MTD_GetStatusBuf();
    newOpa.needToMoveData = false;
    
    if(((uint32_t)buffer) % 4)
    {
        newOpa.needToMoveData = true;
    }

    if(buffer == NULL){
        newOpa.needToMoveData = false;
    }
    
    while (!deviceInited)
    {
        ;
    }
    
    

    xQueueSend(MTD_Operates_Queue, &newOpa, portMAX_DELAY);
    
    xEventGroupWaitBits(MTDLockEventGroup, (1 << newOpa.BLock), pdTRUE, pdFALSE, portMAX_DELAY);
    if(*newOpa.StatusBuf){
        retVal = -1;
    }
    *newOpa.StatusBuf = -5;
    return retVal;
}




int MTD_ErasePhyBlock(uint32_t block)
{
    MTD_Operates newOpa;
    int retVal = 0;

    newOpa.opa = MTD_PHY_ERASE;
    newOpa.page = block;
    newOpa.BLock = MTD_getLock();
    newOpa.StatusBuf = MTD_GetStatusBuf();


    while (!deviceInited)
    {
        portYIELD();
    }
    xQueueSend(MTD_Operates_Queue, &newOpa, portMAX_DELAY);

    xEventGroupWaitBits(MTDLockEventGroup, (1 << newOpa.BLock), pdTRUE, pdFALSE, portMAX_DELAY);
    if(*newOpa.StatusBuf){
        retVal = -1;
    }
    *newOpa.StatusBuf = -5;
    return retVal;
}

int MTD_EraseAllBLock(void)
{
    int ret = 0;
    for(int i=0; i<mtdinfo.Blocks; i++){
        ret = MTD_ErasePhyBlock(i);
        MTD_WARN("Erase Block:%d ret:%d\n",i , ret);
        if(ret)
        {
            return ret;
        }
    }
    return ret;
}

int MTD_ReadPhyPageMeta(uint32_t page, uint32_t len, uint8_t *buffer)
{
    MTD_Operates newOpa;
    int retVal;

    newOpa.opa = MTD_PHY_READ_META;
    newOpa.page = page;
    newOpa.offset = 0;
    newOpa.buf = buffer;
    newOpa.len = len > mtdinfo.MetaSize_B ? mtdinfo.MetaSize_B : len;
    newOpa.BLock = MTD_getLock();
    newOpa.StatusBuf = MTD_GetStatusBuf();
    newOpa.needToMoveData = false;

    while (!deviceInited)
    {
        portYIELD();
    }
    xQueueSend(MTD_Operates_Queue, &newOpa, portMAX_DELAY);
    
    xEventGroupWaitBits(MTDLockEventGroup, (1 << newOpa.BLock), pdTRUE, pdFALSE, portMAX_DELAY);

    retVal = *newOpa.StatusBuf;
    *newOpa.StatusBuf = -5;
    return retVal;
}

/*
int MTD_WritePhyPageMeta(uint32_t page, uint32_t len, uint8_t *buffer)
{
    MTD_Operates newOpa;
    int retVal = 0;

    newOpa.opa = MTD_PHY_WRITE_META;
    newOpa.page = page;
    newOpa.len = len > mtdinfo.MetaSize_B ? mtdinfo.MetaSize_B : len;
    newOpa.buf = buffer;
    newOpa.BLock = MTD_getLock();
    newOpa.StatusBuf = MTD_GetStatusBuf();

    while (!deviceInited)
    {
        portYIELD();
    }
    xQueueSend(MTD_Operates_Queue, &newOpa, portMAX_DELAY);

    xEventGroupWaitBits(MTDLockEventGroup, (1 << newOpa.BLock), pdTRUE, pdFALSE, portMAX_DELAY);
    if(*newOpa.StatusBuf){
        retVal = -1;
    }
    *newOpa.StatusBuf = -5;
    return retVal;
}
*/

int MTD_WritePhyPageWithMeta(uint32_t page, uint32_t meta_len, uint8_t *buffer, uint8_t *meta)
{
    MTD_Operates newOpa;
    int retVal = 0;

    newOpa.opa = MTD_PHY_WRITE_META;
    newOpa.page = page;
    newOpa.len = meta_len > mtdinfo.MetaSize_B ? mtdinfo.MetaSize_B : meta_len  ;
    newOpa.buf = buffer;
    newOpa.metaDat = meta;
    newOpa.BLock = MTD_getLock();
    newOpa.StatusBuf = MTD_GetStatusBuf();

    if(((uint32_t)buffer) % 4)
    {
        newOpa.needToMoveData = true;
    }

    while (!deviceInited)
    {
        portYIELD();
    }
    xQueueSend(MTD_Operates_Queue, &newOpa, portMAX_DELAY);

    xEventGroupWaitBits(MTDLockEventGroup, (1 << newOpa.BLock), pdTRUE, pdFALSE, portMAX_DELAY);
    if(*newOpa.StatusBuf){
        retVal = -1;
    }
    *newOpa.StatusBuf = -5;
    return retVal;
}


int MTD_CopyPhyPage(uint32_t srcPage, uint32_t dstPage)
{
    MTD_Operates newOpa;
    int retVal = 0;

    newOpa.opa = MTD_PHY_COPY;
    newOpa.page = srcPage;
    newOpa.copyDstPage = dstPage;
    newOpa.BLock = MTD_getLock();
    newOpa.StatusBuf = MTD_GetStatusBuf();

    while (!deviceInited)
    {
        portYIELD();
    }
    xQueueSend(MTD_Operates_Queue, &newOpa, portMAX_DELAY);

    xEventGroupWaitBits(MTDLockEventGroup, (1 << newOpa.BLock), pdTRUE, pdFALSE, portMAX_DELAY);
    retVal = *newOpa.StatusBuf;
    *newOpa.StatusBuf = -5;
    return retVal;
}


void MTD_DeviceInit()
{
    portMTDDeviceInit(&mtdinfo);
    

    MTD_Operates_Queue = xQueueCreate(32, sizeof(MTD_Operates));
    MTDLockEventGroup = xEventGroupCreate();
    MTDDriverOpaDone = xEventGroupCreate();

    MTDLockBit = 0;
    for(int i=0; i<(sizeof(MTDStatusBuf) / sizeof(int32_t)); i++){
        MTDStatusBuf[i] = -5;
    }
    pMTDStatus = 0;
    pMetadata = portMTDGetMetaData();
    deviceInited = true;
}
