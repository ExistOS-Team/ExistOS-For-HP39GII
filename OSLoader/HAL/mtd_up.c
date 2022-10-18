#include "SystemConfig.h"
#include "FreeRTOS.h"

#include "board_up.h"

#include "mtd_up.h"
#include "nand.h"
#include "../debug.h"

static QueueHandle_t MTD_Operates_Queue;
//static EventGroupHandle_t MTDLockEventGroup;
//static EventGroupHandle_t MTDDriverOpaDone;

static mtdInfo_t mtdinfo;
static MTD_Operates curOpa;
static bool deviceInited = false;
//static int32_t MTDLockBit = 0;

static uint32_t ECCResult;

//static int32_t MTDStatusBuf[32];
//static int32_t pMTDStatus = 0;

static uint8_t *pMetadata;

static volatile bool mtd_opa_done = false;

uint32_t g_mtd_write_cnt = 0;
uint32_t g_mtd_read_cnt = 0;
uint32_t g_mtd_erase_cnt = 0;
uint32_t g_mtd_ecc_cnt = 0;
uint32_t g_mtd_ecc_fatal_cnt = 0;

uint32_t last_read_page = 0xFFFFFFFF;

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
    BaseType_t flag = false;
    ECCResult = eccResult;
    mtd_opa_done = true;
    //xEventGroupSetBitsFromISR(MTDDriverOpaDone, 1, &flag);
    return flag;

}

uint32_t retry_cnt;
void MTD_Task()
{
    //vTaskDelay(pdMS_TO_TICKS(1000));
    while(1){
        
        if(xQueueReceive(MTD_Operates_Queue, &curOpa, portMAX_DELAY) == pdTRUE)
        {
            enterSlowDown();


            retry_cnt = 5;
            retry:

            MTD_INFO("MTD REC OPA\n");
            mtd_opa_done = false;

            switch (curOpa.opa)
            {

            case MTD_PHY_READ:

                MTD_INFO_READ("QUEUE READ, page:%d, buf:%p, len: %d, needMove:%d\n",curOpa.page, curOpa.buf, curOpa.len, curOpa.needToMoveData);

                if(curOpa.needToMoveData)
                {
                    MTD_INFO_READ("Move Dat Read,page:%d,%p\n",curOpa.page,MTD_PageBuffer);
                    portMTDReadPage(curOpa.page, MTD_PageBuffer);
                }else{
                    portMTDReadPage(curOpa.page, curOpa.buf);
                }
                
                break;
            case MTD_PHY_READ_META:
                MTD_INFO_READ("MTD READ META:page:%d\n", curOpa.page);
                portMTDReadPage(curOpa.page, NULL);
                break;


            case MTD_PHY_ERASE:
                MTD_INFO_ERASE("MTD ERASE:b %d\n", curOpa.page);
                portMTDEraseBlock(curOpa.page);
                break;
            case MTD_PHY_WRITE:
                MTD_INFO_WRITE("MTD WRITE, page:%d,data:%08x, move:%d\n", curOpa.page, curOpa.buf, curOpa.needToMoveData);
                if(curOpa.needToMoveData)
                {
                    memcpy(MTD_PageBuffer, curOpa.buf, mtdinfo.PageSize_B);
                    portMTDWritePage(curOpa.page, MTD_PageBuffer);

                }else{
                    portMTDWritePage(curOpa.page, curOpa.buf);
                }
                
                break;
            case MTD_PHY_WRITE_META:
                MTD_INFO_WRITE("MTD WRITE META, page:%d,data:%08x, len:%d\n", curOpa.page, curOpa.metaDat, curOpa.len);
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
                MTD_WARN("UNEXCEPTED MTD REC OPA!\n");
                break;
            }
            uint32_t start_tick = xTaskGetTickCount();
            while(mtd_opa_done == false)
            {
                if(xTaskGetTickCount() - start_tick > pdMS_TO_TICKS(2000)){
                    INFO("MTD Waiting Timeout! %ld\n", retry_cnt);
                    INFO("Cur opa:%d\n", curOpa.opa);
                    INFO("Cur opa.page:%ld\n", curOpa.page);
                    INFO("Cur opa.offset:%ld\n", curOpa.offset);
                    INFO("Cur opa.needToMoveData:%d\n", curOpa.needToMoveData);
                    INFO("Cur opa.buf:%p\n", curOpa.buf);
                    if(retry_cnt)
                    {
                        goto retry;
                        retry_cnt--;
                    }else{
                        mtd_opa_done = true;
                        ECCResult = 0x0E0E0E0E;
                    }
                    
                    //while(1);


                }
                
                
            }
            //xEventGroupWaitBits(MTDDriverOpaDone, 1, pdTRUE, pdFALSE, portMAX_DELAY);


            switch (curOpa.opa)
            {
            case MTD_PHY_READ_META:
                memcpy(curOpa.buf, portMTDGetMetaData(), curOpa.len);
            case MTD_PHY_READ:
                if(curOpa.needToMoveData)
                {
                    if(curOpa.len > mtdinfo.PageSize_B - curOpa.offset){
                        MTD_WARN("MTD READ DATA Corrupted.page:%ld len:%ld offset:%ld\n",curOpa.page, curOpa.len, curOpa.offset);
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
                    MTD_WARN("BAD BLOCK:%ld\n", curOpa.page);
                    //*curOpa.StatusBuf =  -1;
                    xTaskNotify(curOpa.task, -1, eSetValueWithOverwrite);
                }else if(ECCResult == 0x0F0F0F0F){
                    xTaskNotify(curOpa.task, 1, eSetValueWithOverwrite);
                    MTD_INFO_READ("EMPTY PAGE\n");
                    //*curOpa.StatusBuf = 1;          //EMPTY
                }else {
                    xTaskNotify(curOpa.task, 0, eSetValueWithOverwrite);
                    //*curOpa.StatusBuf = 0;
                }

                if((ECCResult > 1) && (ECCResult < 0x0F0F0F0F)){
                //if((ECCResult != 0))
                    //printf("ECC Err found:%08lX, PhySector:%ld\n",ECCResult, curOpa.page);
                    g_mtd_ecc_cnt++;
                
                }
                last_read_page = curOpa.page;

                break;
            
                

                case MTD_PHY_ERASE:
                    //*curOpa.StatusBuf = ECCResult;
                    xTaskNotify(curOpa.task, ECCResult, eSetValueWithOverwrite);
                    break;

                case MTD_PHY_WRITE_META:
                case MTD_PHY_WRITE:
                    xTaskNotify(curOpa.task, ECCResult, eSetValueWithOverwrite);
                    //*curOpa.StatusBuf = ECCResult;
                    break;

                case MTD_PHY_COPY:
                    if( 
                        (((ECCResult ) & 0xF)      == 0xE) || 
                        (((ECCResult >> 8) & 0xF)  == 0xE) || 
                        (((ECCResult >> 16) & 0xF) == 0xE) || 
                        (((ECCResult >> 24) & 0xF) == 0xE) 
                    ){
                        g_mtd_ecc_fatal_cnt++;
                        MTD_WARN("BAD BLOCK:%ld\n", curOpa.page);
                        //*curOpa.StatusBuf =  -1;
                        xTaskNotify(curOpa.task, -1, eSetValueWithOverwrite);
                    }else if(ECCResult == 0x0F0F0F0F){
                        xTaskNotify(curOpa.task, 0, eSetValueWithOverwrite);
                        //*curOpa.StatusBuf = 0;      //EMPTY
                    }else {
                        xTaskNotify(curOpa.task, 0, eSetValueWithOverwrite);
                        //*curOpa.StatusBuf = 0;
                    }

                    break;

            default:
                break;
            }

            //xEventGroupSetBits(MTDLockEventGroup , (1 << curOpa.BLock));
            exitSlowDown();
            
        }

    }
}
/*
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

*/

int MTD_ReadPhyPage(uint32_t page, uint32_t offset, uint32_t len, uint8_t *buffer)
{
    MTD_Operates newOpa;
    int retVal;

    newOpa.opa = MTD_PHY_READ;
    newOpa.page = page;
    newOpa.offset = offset;
    newOpa.buf = buffer;
    newOpa.len = len;
    newOpa.needToMoveData = false;
    newOpa.task = xTaskGetCurrentTaskHandle();


    if((offset != 0) || (len != mtdinfo.PageSize_B) || (((uint32_t)buffer & 3) != 0)){
        newOpa.needToMoveData = true;
    }
    if(buffer == NULL){
        newOpa.needToMoveData = false;
    }
    while (!deviceInited)
    {
        vTaskDelay(2);
    }
    MTD_INFO("POST READ CMD, queue num:%lx\n",uxQueueGetQueueNumber(MTD_Operates_Queue));
    
    g_mtd_read_cnt++;
    xTaskNotifyStateClear(NULL);

    xQueueSend(MTD_Operates_Queue, &newOpa, portMAX_DELAY);

    MTD_INFO("POST READ CMD END:%lx\n", uxQueueGetQueueNumber(MTD_Operates_Queue));

    xTaskNotifyWait(0, 0xFFFFFFFF, (uint32_t *)&retVal, portMAX_DELAY);
    
    MTD_INFO("POST READ RET:%d\n", retVal);
    return retVal;
}

int MTD_WritePhyPage(uint32_t page,uint8_t *buffer)
{
    MTD_Operates newOpa;
    int retVal = 0;

    newOpa.opa = MTD_PHY_WRITE;
    newOpa.page = page;
    newOpa.buf = buffer;
    //newOpa.BLock = MTD_getLock();
    //newOpa.StatusBuf = MTD_GetStatusBuf();
    newOpa.needToMoveData = false;
    newOpa.task = xTaskGetCurrentTaskHandle();
/*
    if(((uint32_t)buffer) >= MEMORY_SIZE ){
        newOpa.needToMoveData = true;
        INFO("Data is not loaded in RAM.\n");
    }
    */
    if(((uint32_t)buffer) & 0x3)
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
    
    g_mtd_write_cnt++;
    xTaskNotifyStateClear(NULL);
    xQueueSend(MTD_Operates_Queue, &newOpa, portMAX_DELAY);
    /*
    xEventGroupWaitBits(MTDLockEventGroup, (1 << newOpa.BLock), pdTRUE, pdFALSE, portMAX_DELAY);
    if(*newOpa.StatusBuf){
        retVal = -1;
    }
    *newOpa.StatusBuf = -5;*/
    
    xTaskNotifyWait(0, 0xFFFFFFFF, (uint32_t *)&retVal, portMAX_DELAY);

    //MTD_WARN("BAD BLOCK 2:%d\n", retVal);

    return retVal;
}




int MTD_ErasePhyBlock(uint32_t block)
{
    MTD_Operates newOpa;
    int retVal = 0;

    newOpa.opa = MTD_PHY_ERASE;
    newOpa.page = block;
    //newOpa.BLock = MTD_getLock();
    //newOpa.StatusBuf = MTD_GetStatusBuf();
    newOpa.task = xTaskGetCurrentTaskHandle();
    while (!deviceInited)
    {
        vTaskDelay(2);
    }
    
    g_mtd_erase_cnt++;
    xTaskNotifyStateClear(NULL);
    xQueueSend(MTD_Operates_Queue, &newOpa, portMAX_DELAY);

    xTaskNotifyWait(0, 0xFFFFFFFF, (uint32_t *)&retVal, portMAX_DELAY);
/*
    xEventGroupWaitBits(MTDLockEventGroup, (1 << newOpa.BLock), pdTRUE, pdFALSE, portMAX_DELAY);
    if(*newOpa.StatusBuf){
        retVal = -1;
    }
    *newOpa.StatusBuf = -5;
*/

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
    //newOpa.BLock = MTD_getLock();
    //newOpa.StatusBuf = MTD_GetStatusBuf();
    newOpa.needToMoveData = false;
    newOpa.task = xTaskGetCurrentTaskHandle();

    g_mtd_read_cnt++;
    while (!deviceInited)
    {
        vTaskDelay(2);
    }
    xTaskNotifyStateClear(NULL);
    xQueueSend(MTD_Operates_Queue, &newOpa, portMAX_DELAY);
    /*
    xEventGroupWaitBits(MTDLockEventGroup, (1 << newOpa.BLock), pdTRUE, pdFALSE, portMAX_DELAY);

    retVal = *newOpa.StatusBuf;
    *newOpa.StatusBuf = -5;*/

    xTaskNotifyWait(0, 0xFFFFFFFF, (uint32_t *)&retVal, portMAX_DELAY);

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
        vTaskDelay(2);
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
    //newOpa.BLock = MTD_getLock();
    //newOpa.StatusBuf = MTD_GetStatusBuf();
    newOpa.task = xTaskGetCurrentTaskHandle();

    if(((uint32_t)buffer) & 3)
    {
        newOpa.needToMoveData = true;
    }

    g_mtd_write_cnt++;

    while (!deviceInited)
    {
        vTaskDelay(2);
    }
    xTaskNotifyStateClear(NULL);
    xQueueSend(MTD_Operates_Queue, &newOpa, portMAX_DELAY);
/*
    xEventGroupWaitBits(MTDLockEventGroup, (1 << newOpa.BLock), pdTRUE, pdFALSE, portMAX_DELAY);
    if(*newOpa.StatusBuf){
        retVal = -1;
    }
    *newOpa.StatusBuf = -5;*/

    xTaskNotifyWait(0, 0xFFFFFFFF, (uint32_t *)&retVal, portMAX_DELAY);

    return retVal;
}


int MTD_CopyPhyPage(uint32_t srcPage, uint32_t dstPage)
{

    
    MTD_Operates newOpa;
    int retVal = 0;

    newOpa.opa = MTD_PHY_COPY;
    newOpa.page = srcPage;
    newOpa.copyDstPage = dstPage;
    //newOpa.BLock = MTD_getLock();
    //newOpa.StatusBuf = MTD_GetStatusBuf();
    newOpa.task = xTaskGetCurrentTaskHandle();

    while (!deviceInited)
    {
        vTaskDelay(2);
    }

    xTaskNotifyStateClear(NULL);
    xQueueSend(MTD_Operates_Queue, &newOpa, portMAX_DELAY);

    g_mtd_read_cnt++;
    g_mtd_write_cnt++;
/*
    xEventGroupWaitBits(MTDLockEventGroup, (1 << newOpa.BLock), pdTRUE, pdFALSE, portMAX_DELAY);
    retVal = *newOpa.StatusBuf;
    *newOpa.StatusBuf = -5;*/

    xTaskNotifyWait(0, 0xFFFFFFFF, (uint32_t *)&retVal, portMAX_DELAY);

    return retVal;
}


void MTD_DeviceInit()
{
    MTD_Operates_Queue = xQueueCreate(16, sizeof(MTD_Operates));
    printf("MTD_Operates_Queue:%p\n", MTD_Operates_Queue);
    portMTDDeviceInit(&mtdinfo);

    
    //MTDLockEventGroup = xEventGroupCreate();
//    MTDDriverOpaDone = xEventGroupCreate();
/*
    MTDLockBit = 0;
    for(int i=0; i<(sizeof(MTDStatusBuf) / sizeof(int32_t)); i++){
        MTDStatusBuf[i] = -5;
    }
    pMTDStatus = 0;
    */
    pMetadata = portMTDGetMetaData();
    deviceInited = true;
}
