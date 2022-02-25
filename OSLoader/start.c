
#include <stdio.h>


#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "ff.h"
#include "tusb.h"
#include "usbd.h"

#include "board_up.h"
#include "mtd_up.h"
#include "FTL_up.h"
#include "display_up.h"

#include "keyboard_up.h"

#include "../debug.h"

#include "stmp_NandControlBlock.h"


uint8_t LDLB2[144] = {
	0x03, 0x02, 0x01, 0x00, 0x0B, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x06, 0x00, 0x00, 0x00, 
	0x2A, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x50, 0x00, 0x00, 0x00, 0x31, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 
	0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x47, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 
	0x70, 0x00, 0x00, 0x00, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x47, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0xA9, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xF0, 0x03, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 
	0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x03, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF
};


PARTITION VolToPart[FF_VOLUMES] = {
    {0, 1},     /* "0:" ==> 1st partition on the pd#0 */
    {0, 2}     /* "1:" ==> 2nd partition on the pd#0 */
};


static bool diskInit = false;
uint32_t CurMount = 0;
uint32_t FTL_status = 0;

char pcWriteBuffer[2048];
void printTaskList() {
    vTaskList((char *)&pcWriteBuffer);
    printf("=============================================\r\n");
    printf("任务名                 任务状态   优先级   剩余栈   任务序号\n");
    printf("%s\n", pcWriteBuffer);
    printf("任务名                运行计数           CPU使用率\n");
    vTaskGetRunTimeStats((char *)&pcWriteBuffer);
    printf("%s\n", pcWriteBuffer);
    printf("任务状态:  X-运行  R-就绪  B-阻塞  S-挂起  D-删除\n");
    printf("内存剩余:   %d Bytes\n", (unsigned int)xPortGetFreeHeapSize());
}

void vTask1(void *pvParameters) {
    //printf("Start vTask1\n");
    for (;;) {
		    
		vTaskDelay(pdMS_TO_TICKS(5000));
		printTaskList();
		printf("task 111\n");
		
    }
}



bool flushInd;
char stList[] = {'\\', '|', '/', '-'};
void vTaskIndicate(void *pvParameters)
{
  flushInd = true;
  for (;;) {
    for(int i = 0; i < sizeof(stList); i++){
        if(flushInd)
          DisplayPutChar(31*8,0,stList[i],0,255, 16);
        vTaskDelay(pdMS_TO_TICKS(300));
    }
  }
}

char *MenuItems[] = 
{
  "Status",
  "Boot ExistOS",
  "Mount USB MSC",
  "Install System",
  "Format Flash",
  "Restore IMG",
  "Shutdown",  
  "Reboot"
};

char *StatusItems[] = 
{
  "System File Not",
  "Found.",
  "Manual Configure.",
  "Disk Format Error.",
};

#define MenuItemNum  (sizeof(MenuItems)/sizeof(uint32_t))

void RenderMainW(uint32_t selectMenu)
{
    flushInd = false;

    DisplayPutStr(0,0," Exist OS Loader Configuration ",0,255, 16);
    DisplayBox(0,0,255,125, 255);
    DisplayVLine(16, 125, 102, 255);
    

    for(int i = 0; i < sizeof(MenuItems) / sizeof(uint32_t); i++){
      if(i == selectMenu)
        DisplayPutStr(2, (i + 1)*13 + 5, MenuItems[i], 0, 255, 12);
      else
        DisplayPutStr(2, (i + 1)*13 + 5, MenuItems[i], 255, 0, 12);
    }

    flushInd = true;
}


void EraseWithDisp(uint32_t startBlock)
{
    DisplayFillBox(40, 30, 180, 80, 0);
    DisplayBox(40, 30, 180, 80, 0xFF);
    DisplayPutStr(40 + 6*6,31 + 12 * 0,"Erasing...",255,0, 12);
    int ret = 0;

    for(int i = startBlock; i < 1024; i++){
      DisplayVLine(31 + 12 * 1, 31 + 12 * 2, 45 + (i/((1024 - startBlock)/120)) ,0xFF );
      ret = MTD_ErasePhyBlock(i);
      if(ret){
        break;
      }
    }

    DisplayFillBox(40, 30, 180, 80, 0);
    DisplayBox(40, 30, 180, 80, 0xFF);
    if(!ret)
      DisplayPutStr(40 + 6*4,31 + 12 * 1,"Erase Finish.",255,0, 12);
    else
      DisplayPutStr(40 + 6*4,31 + 12 * 1,"Erase ERROR!",255,0, 12);

}

void GetPartitionInfo(uint32_t p, uint32_t *start, uint32_t *sectors)
{
  uint8_t *buf;
  buf = pvPortMalloc(FTL_GetSectorSize());
  FTL_ReadSector(0, 1, buf);

  switch (p)
  {
  case 0:
    //*start   = *(uint32_t *)&buf[0x1C6];
    //*sectors = *(uint32_t *)&buf[0x1CA];
    memcpy(start  , &buf[0x1C6], 4);
    memcpy(sectors, &buf[0x1CA], 4);
    break;
  case 1:
    //*start   = *(uint32_t *)&buf[0x1D6];
    //*sectors = *(uint32_t *)&buf[0x1DA];
    memcpy(start  , &buf[0x1D6], 4);
    memcpy(sectors, &buf[0x1DA], 4);
    break;
  default:
    break;
  }

  vPortFree(buf);
}

uint32_t MSCpartStartSector;
uint32_t MSCpartSectors;
void setMountPartition(uint32_t p)
{
  GetPartitionInfo(p, &MSCpartStartSector, &MSCpartSectors);
}

void mkNCB(uint32_t inBlock)
{
  mtdInfo_t *info = MTD_getDeviceInfo();
  
  uint8_t *pageBuff = pvPortMalloc(info->PageSize_B);
  
  uint8_t *metaBuff = pvPortMalloc(info->MetaSize_B);

	memset(pageBuff, 0xff, info->PageSize_B);
	memset(metaBuff, 0xff, info->MetaSize_B);

  NAND_Control_Block *NCB = (NAND_Control_Block *)pageBuff;

	NCB->m_u32Fingerprint1 = 0x504D5453;
	NCB->NAND_Timing.m_u8DataSetup = 0x10;
	NCB->NAND_Timing.m_u8DataHold = 0x0C;
	NCB->NAND_Timing.m_u8AddressSetup = 0x14;
	NCB->NAND_Timing.m_u8DSAMPLE_TIME = 0x06;
	NCB->m_u32DataPageSize = 0x800;
	NCB->m_u32TotalPageSize = 0x840;
	NCB->m_u32SectorsPerBlock = 0x40;
	NCB->m_u32SectorInPageMask = 0;
	NCB->m_u32SectorToPageShift = 0;
	NCB->m_u32NumberOfNANDs = 1;
	
	NCB->m_u32Fingerprint2 = 0x2042434E;
	
	NCB->m_u32NumRowBytes = 2;
	NCB->m_u32NumColumnBytes = 2;
	NCB->m_u32TotalInternalDie = 1;
	NCB->m_u32InternalPlanesPerDie = 1;
	NCB->m_u32CellType = 2;
	NCB->m_u32ECCType = 0;
	NCB->m_u32Read1stCode = 0x00;
	NCB->m_u32Read2ndCode = 0x30;
	NCB->m_u32Fingerprint3 = 0x4E494252;

	
	metaBuff[2] = 0x42;
	metaBuff[3] = 0x43;
	metaBuff[4] = 0x42;
	metaBuff[5] = 0x20;

  MTD_ErasePhyBlock(inBlock);

/*
  MTD_WritePhyPageMeta(inBlock * info->PagesPerBlock, info->MetaSize_B, metaBuff);
  MTD_WritePhyPage(inBlock * info->PagesPerBlock, pageBuff);
*/

  MTD_WritePhyPageWithMeta(inBlock * info->PagesPerBlock, info->MetaSize_B, pageBuff, metaBuff);

  vPortFree(pageBuff);
  vPortFree(metaBuff);

}

void mkDBBT(uint32_t inBlock)
{
  mtdInfo_t *info = MTD_getDeviceInfo();
  uint8_t *pageBuff = pvPortMalloc(info->PageSize_B);
  uint8_t *metaBuff = pvPortMalloc(info->MetaSize_B);

	memset(pageBuff, 0x00, info->PageSize_B);
	memset(metaBuff, 0xff, info->MetaSize_B);

  DiscoveredBadBlockTable_t *DBBT = (DiscoveredBadBlockTable_t *)pageBuff;

  DBBT->m_u32Fingerprint1 = 0x504D5453;
  DBBT->m_u32Number2KPagesBB_NAND0 = 1;
  DBBT->m_u32Number2KPagesBB_NAND1 = 1;
  DBBT->m_u32Number2KPagesBB_NAND2 = 1;
  DBBT->m_u32Number2KPagesBB_NAND3 = 1;

  DBBT->m_u32Fingerprint2 = 0x54424244;
  DBBT->m_u32Fingerprint3 = 0x44494252;

  memset(&metaBuff[2], 0, 6);

  MTD_ErasePhyBlock(inBlock);

  MTD_WritePhyPageWithMeta(inBlock * info->PagesPerBlock, info->MetaSize_B, pageBuff, metaBuff);

  vPortFree(pageBuff);
  vPortFree(metaBuff);

}

void mkLDLB(uint32_t inBlock, uint32_t fwPageOffset, uint32_t fwPageTotal, uint32_t DBBT1_page, uint32_t DBBT2_page)
{
  mtdInfo_t *info = MTD_getDeviceInfo();
  uint8_t *pageBuff = pvPortMalloc(info->PageSize_B);
  uint8_t *metaBuff = pvPortMalloc(info->MetaSize_B);

	memset(pageBuff, 0xff, info->PageSize_B);
	memset(metaBuff, 0xff, info->MetaSize_B);

	LogicalDriveLayoutBlock_t *LDLB = (LogicalDriveLayoutBlock_t *)pageBuff;
			
	LDLB->m_u32Fingerprint1	= 0x504D5453;
	LDLB->LDLB_Version.m_u16Major = 1;
	LDLB->LDLB_Version.m_u16Minor = 0;
	LDLB->LDLB_Version.m_u16Sub = 0;
	
	LDLB->m_u32Fingerprint2 = 0x424C444C;
	LDLB->m_u32Firmware_startingNAND = 0;
	LDLB->m_u32Firmware_startingSector = fwPageOffset;
	LDLB->m_u32Firmware_sectorStride = 0;
	LDLB->m_u32SectorsInFirmware = fwPageTotal;

	LDLB->m_u32Firmware_StartingNAND2 = 0;
	LDLB->m_u32Firmware_StartingSector2 = fwPageOffset;
	LDLB->m_u32Firmware_SectorStride2 = 0;
	LDLB->m_u32SectorsInFirmware2 = fwPageTotal;
	
	LDLB->FirmwareVersion.m_u16Major = 1;
	LDLB->FirmwareVersion.m_u16Minor = 0;
	LDLB->FirmwareVersion.m_u16Sub = 0;
	
	LDLB->m_u32DiscoveredBBTableSector = DBBT1_page;
	LDLB->m_u32DiscoveredBBTableSector2 = DBBT2_page;

	LDLB->m_u32Fingerprint3 = 0x4C494252;

	metaBuff[2] = 0x42; //B
	metaBuff[3] = 0x43; //C
	metaBuff[4] = 0x42; //B
	metaBuff[5] = 0x20; //' '

  MTD_ErasePhyBlock(inBlock);

  MTD_WritePhyPageWithMeta(inBlock * info->PagesPerBlock, info->MetaSize_B, pageBuff, metaBuff);

  vPortFree(pageBuff);
  vPortFree(metaBuff);

}



void vTask2(void *pvParameters) 
{
    Keys_t key;
    int selectItem = 0;
    int AbortRes = 0;
    int inSubLevel = 0;

    bool inSubMenu = false;
    bool keyEnter = false;
    bool needClean = false;

    mtdInfo_t *nand_info;    

    DisplayInit();
    RenderMainW(selectItem);
    xTaskCreate(vTaskIndicate, "RenderInd", configMINIMAL_STACK_SIZE, NULL, 1, NULL );


    if(FTL_status){
      diskInit = false;
      AbortRes = 2;
      goto Configuration;
    }else{
      diskInit = true;

    }



    Configuration:
    
    nand_info = MTD_getDeviceInfo();

    for (;;) {
      flushInd = false;
      vTaskDelay(pdMS_TO_TICKS(40));
      switch (selectItem)
      {
      case 0:
        inSubLevel = 0;
        switch (AbortRes)
        {
        case 0:   //Status
          DisplayFillBox(103, 17, 254, 124, 0);
          DisplayPutStr(104,18,"Boot Aborted:",255,0, 12);
          DisplayPutStr(104 + 6 * 4,18 + 12 * 1,StatusItems[0] ,255,0, 12);
          DisplayPutStr(104 + 6 * 4,18 + 12 * 2,StatusItems[1] ,255,0, 12);
          break;
        case 1:
          DisplayFillBox(103, 17, 254, 124, 0);
          DisplayPutStr(104,18,"Boot Aborted:",255,0, 12);
          DisplayPutStr(104 + 6 * 4,18 + 12 * 1,StatusItems[2] ,255,0, 12);
          break;
        case 2:
          DisplayFillBox(103, 17, 254, 124, 0);
          DisplayPutStr(104,18,"Boot Aborted:",255,0, 12);
          DisplayPutStr(104 + 6 * 4,18 + 12 * 1,StatusItems[3] ,255,0, 12);
          break;
        default:
          break;
        }
        DisplayPutStr(104,18 + 12 * 6,"Version: 0.1.0.1",255,0, 12);
      break;
      case 1: //Boot ExistOS
        inSubLevel = 0;
        DisplayFillBox(103, 17, 254, 124, 0);
        DisplayPutStr(104,18 + 12 * 0,"Press ENTER to",255,0, 12);
        DisplayPutStr(104,18 + 12 * 1,"Boot /SYS/ExistOS.sys",255,0, 12);
        break;
      
      case 2: //Mount USB MSC
        DisplayFillBox(103, 17, 254, 124, 0);
        if(diskInit)
        {
          if( inSubLevel> 3){
            inSubLevel = 3;
          }
          DisplayPutStr(104,18 + 12 * 0,"Select which Partition",255,0, 12);
          DisplayPutStr(104,18 + 12 * 1,"to mount on USB MSC.",255,0, 12);

          DisplayPutStr(104 + 10         ,18 + 12 * 3,"NONE",(inSubLevel == 1) ? 0 : 255,(inSubLevel == 1) ? 255 : 0, 12);
          DisplayPutStr(104 + 10 + (8*6) ,18 + 12 * 3,"DATA",(inSubLevel == 2) ? 0 : 255,(inSubLevel == 2) ? 255 : 0, 12);
          DisplayPutStr(104 + 10 + (16*6),18 + 12 * 3,"SYS ",(inSubLevel == 3) ? 0 : 255,(inSubLevel == 3) ? 255 : 0, 12);

          if(keyEnter && (inSubLevel > 0)){
            CurMount = inSubLevel - 1;
            switch (CurMount)
            {
            case 0:
              tud_disconnect();
              vTaskDelay(pdMS_TO_TICKS(500));
              tud_connect();
              break;
            case 1:
              {
                tud_disconnect();
                setMountPartition(0);
                vTaskDelay(pdMS_TO_TICKS(500));


                tud_connect();
              }

              break;
            case 2:
              {
                tud_disconnect();
                setMountPartition(1);
                vTaskDelay(pdMS_TO_TICKS(500));
                tud_connect();
              }
              break;

            default:
              break;
            }
          }

            switch (CurMount)
            {
            case 0:
              DisplayPutStr(104 + 10          ,18 + 12 * 4,"^^^^",255,0, 12);
              break;
            case 1:
              DisplayPutStr(104 + 10 + (8*6)  ,18 + 12 * 4,"^^^^",255,0, 12);
              break;
            case 2:
              DisplayPutStr(104 + 10 + (16*6) ,18 + 12 * 4,"^^^^",255,0, 12);
              break;
            default:
              break;
            }




        }else{
          inSubLevel = 0;
          DisplayPutStr(104,18 + 12 * 0,"Flash not Formatted, ",255,0, 12);
          DisplayPutStr(104,18 + 12 * 1,"please format before",255,0, 12);
          DisplayPutStr(104,18 + 12 * 2,"mount.",255,0, 12);
        }
        break;
      
      case 3:   //Install System
        inSubLevel = 0;
        DisplayFillBox(103, 17, 254, 124, 0);
        DisplayPutStr(104,18 + 12 * 0,"Please copy system",255,0, 12);
        DisplayPutStr(104,18 + 12 * 1,"file to /SYS/  ",255,0, 12);
        DisplayPutStr(104,18 + 12 * 3,"press ENTER to write",255,0, 12);
        DisplayPutStr(104,18 + 12 * 4,"boot sector.",255,0, 12);


        if(keyEnter)
        {
          FRESULT ret;
          FILINFO finfo;
          FATFS *fs = pvPortMalloc(sizeof(FATFS));
          f_mount(fs, "/SYS/", 1);

          ret = f_stat("/SYS/OSLoader.sb", &finfo);
          uint32_t fsize;
          fsize = finfo.fsize;
          INFO("fsize:%lu \n",fsize);

          INFO("f_stat:%d\n",ret);
          if(ret != FR_OK){
              DisplayFillBox(40, 30, 220, 80, 0);
              DisplayBox(40, 30, 220, 80, 0xFF);
              DisplayPutStr(40 + 1*4,31 + 12 * 1,"/SYS/OSLoader.sb is not",255,0, 12);
              DisplayPutStr(40 + 1*4,31 + 12 * 2,"existed!",255,0, 12);
              needClean = true;
              f_unmount("/SYS/");
              vPortFree(fs);
            break;
          }
          
          uint32_t  fwSectors = (fsize + nand_info->PageSize_B - 1) / (nand_info->PageSize_B);
          uint32_t  fwBlocks = (fwSectors + nand_info->PagesPerBlock - 1) / (nand_info->PagesPerBlock);

          INFO("fsize:%u, fwSectors:%u, fwBlocks:%u \n",fsize, fwSectors, fwBlocks  );
          
          DisplayFillBox(40, 30, 220, 80, 0);
          DisplayBox(40, 30, 220, 80, 0xFF);
          DisplayPutStr(40 + 1*4,31 + 12 * 1,"Flashing...",255,0, 12);

          FIL *f = pvPortMalloc(sizeof(FIL));
          ret = f_open(f, "/SYS/OSLoader.sb" ,FA_READ);
          INFO("f_open:%d\n",ret);
          if(ret != FR_OK){
              DisplayFillBox(40, 30, 220, 80, 0);
              DisplayBox(40, 30, 220, 80, 0xFF);
              DisplayPutStr(40 + 1*4,31 + 12 * 1,"Reading /SYS/OSLoader.sb",255,0, 12);
              DisplayPutStr(40 + 1*4,31 + 12 * 2,"Failed!",255,0, 12);
              needClean = true;
              f_unmount("/SYS/");
              vPortFree(fs);
              vPortFree(f);
            break;
          }




          uint8_t *pgbuff = pvPortMalloc(nand_info->PageSize_B);
          uint8_t *mtbuff = pvPortMalloc(nand_info->MetaSize_B);

          memset(mtbuff, 0xFF, nand_info->MetaSize_B);
          mtbuff[1] = 0x00; 
          mtbuff[2] = 0x53; //S
          mtbuff[3] = 0x54; //T
          mtbuff[4] = 0x4D; //M
          mtbuff[5] = 0x50; //P

          for(int i = 22; i < 22 + fwBlocks; i++){
              MTD_ErasePhyBlock(i);
          }

          UINT br;
          for(int i = 0; i < fwSectors; i++)
          {
            memset(pgbuff, 0xFF, nand_info->PageSize_B);

            ret = f_read(f, pgbuff, nand_info->PageSize_B, &br);
            
            MTD_WritePhyPageWithMeta(22 * nand_info->PagesPerBlock + i, nand_info->MetaSize_B, pgbuff, mtbuff);
          }
          
          mkNCB(0);
          mkNCB(4);

          mkDBBT(16);
          mkDBBT(19);
          
          mkLDLB(8,  22 * 64, fwSectors, 16 * 64, 19 * 64);
          mkLDLB(12, 22 * 64, fwSectors, 16 * 64, 19 * 64);


          f_close(f);

          f_unmount("/SYS/");
          vPortFree(fs);
          vPortFree(f);
          vPortFree(pgbuff);
          vPortFree(mtbuff);


          DisplayFillBox(40, 30, 220, 80, 0);
          DisplayBox(40, 30, 220, 80, 0xFF);
          DisplayPutStr(40 + 1*4,31 + 12 * 1,"Install Completed.",255,0, 12);

        }

        break;


      case 4:   //Format Flash
        if( inSubLevel > 2){
            inSubLevel = 2;
          }
        DisplayFillBox(103, 17, 254, 124, 0);
        DisplayPutStr(104,18 + 12 * 0,"Select only Erase ",255,0, 12);
        DisplayPutStr(104,18 + 12 * 1,"or format flash.",255,0, 12);

        DisplayPutStr(104 +10        , 18 + 12 * 3,  "Erase",(inSubLevel == 1) ? 0 : 255,(inSubLevel == 1) ? 255 : 0, 12);
        DisplayPutStr(104 + 10 + (12*6) ,18 + 12 * 3,"Format",(inSubLevel == 2) ? 0 : 255,(inSubLevel == 2) ? 255 : 0, 12);
        if(keyEnter)
        {
          switch (inSubLevel)
          {
          case 1:
            needClean = true;
            EraseWithDisp(0);
            diskInit = false;

            inSubLevel = 0;
            break;
          
          case 2:
            needClean = true;
            EraseWithDisp(0);
            FTL_ClearAllSector();
            {

              LBA_t plist[] = {80, 20, 0};
              MKFS_PARM opt;
              FRESULT ret;
              ret = f_fdisk(0, plist, NULL);
              opt.fmt = FM_FAT;
              opt.au_size = 2048;

              DisplayPutStr(40 + 1*4,31 + 12 * 1,"Formatting... /DATA",255,0, 12);
              ret = f_mkfs("/DATA/", &opt, NULL, FF_MAX_SS);

              if(ret != FR_OK)
                DisplayPutStr(40 + 6*4,31 + 12 * 1,"Format ERROR!",255,0, 12);

              DisplayPutStr(40 + 1*4,31 + 12 * 1,"Formatting... /SYS ",255,0, 12);
              ret = f_mkfs("/SYS/", &opt, NULL, FF_MAX_SS);

              DisplayFillBox(40, 30, 180, 80, 0);
              DisplayBox(40, 30, 180, 80, 0xFF);

              if(ret != FR_OK)
              {
                DisplayPutStr(40 + 6*4,31 + 12 * 1,"Format ERROR!",255,0, 12);
                diskInit = false;
              }else{
                DisplayPutStr(40 + 6*4,31 + 12 * 1,"Format Finish.",255,0, 12);
                diskInit = true;
              }
              
            }

            inSubLevel = 0;
            break;

          default:
            break;
          }
        }

        break;
      case 5:   //Restore IMG
        inSubLevel = 0;
        DisplayFillBox(103, 17, 254, 124, 0);
        DisplayPutStr(104,18 + 12 * 0,"Press ENTER to flash",255,0, 12);
        DisplayPutStr(104,18 + 12 * 1,"/DATA/firmware.sb",255,0, 12);
        DisplayPutStr(104,18 + 12 * 2,"to NAND Flash.",255,0, 12);


        if(keyEnter)
        {
          FRESULT ret;
          FILINFO finfo;
          FATFS *fs = pvPortMalloc(sizeof(FATFS));
          f_mount(fs, "/DATA/", 1);

          ret = f_stat("/DATA/firmware.sb", &finfo);
          uint32_t fsize;
          fsize = finfo.fsize;
          INFO("fsize:%lu \n",fsize);

          INFO("f_stat:%d\n",ret);
          if(ret != FR_OK){
              DisplayFillBox(40, 30, 220, 80, 0);
              DisplayBox(40, 30, 220, 80, 0xFF);
              DisplayPutStr(40 + 1*4,31 + 12 * 1,"/DATA/firmware.sb is",255,0, 12);
              DisplayPutStr(40 + 1*4,31 + 12 * 2,"not existed!",255,0, 12);
              needClean = true;
              f_unmount("/DATA/");
              vPortFree(fs);
            break;
          }
          
          uint32_t  fwSectors = (fsize + nand_info->PageSize_B - 1) / (nand_info->PageSize_B);
          uint32_t  fwBlocks = (fwSectors + nand_info->PagesPerBlock - 1) / (nand_info->PagesPerBlock);
          INFO("fsize:%u, fwSectors:%u, fwBlocks:%u \n",fsize, fwSectors, fwBlocks  );
          
          DisplayFillBox(40, 30, 220, 80, 0);
          DisplayBox(40, 30, 220, 80, 0xFF);
          DisplayPutStr(40 + 1*4,31 + 12 * 1,"Flashing...",255,0, 12);

          FIL *f = pvPortMalloc(sizeof(FIL));
          ret = f_open(f, "/DATA/firmware.sb" ,FA_READ);
          INFO("f_open:%d\n",ret);
          if(ret != FR_OK){
              DisplayFillBox(40, 30, 220, 80, 0);
              DisplayBox(40, 30, 220, 80, 0xFF);
              DisplayPutStr(40 + 1*4,31 + 12 * 1,"Reading /DATA/firmware.sb",255,0, 12);
              DisplayPutStr(40 + 1*4,31 + 12 * 2,"Failed!",255,0, 12);
              needClean = true;
              f_unmount("/DATA/");
              vPortFree(fs);
              vPortFree(f);
            break;
          }




          uint8_t *pgbuff = pvPortMalloc(nand_info->PageSize_B);
          uint8_t *mtbuff = pvPortMalloc(nand_info->MetaSize_B);

          memset(mtbuff, 0xFF, nand_info->MetaSize_B);
          mtbuff[1] = 0x00; 
          mtbuff[2] = 0x53; //S
          mtbuff[3] = 0x54; //T
          mtbuff[4] = 0x4D; //M
          mtbuff[5] = 0x50; //P

          for(int i = 22; i < 22 + fwBlocks; i++){
              MTD_ErasePhyBlock(i);
          }

          UINT br;
          for(int i = 0; i < fwSectors; i++)
          {
            memset(pgbuff, 0xFF, nand_info->PageSize_B);

            ret = f_read(f, pgbuff, nand_info->PageSize_B, &br);
            if((i % 64) == 0){
              mtbuff[1]++;
            }
            MTD_WritePhyPageWithMeta(22 * nand_info->PagesPerBlock + i, nand_info->MetaSize_B, pgbuff, mtbuff);
          }
          
          mkNCB(0);
          mkNCB(4);

          mkDBBT(16);
          mkDBBT(19);
          
          mkLDLB(8,  22 * 64, fwSectors, 16 * 64, 19 * 64);
          mkLDLB(12, 22 * 64, fwSectors, 16 * 64, 19 * 64);

          memset(pgbuff, 0xFF, nand_info->PageSize_B);
          memcpy(pgbuff, LDLB2, sizeof(LDLB2));
          memset(mtbuff, 0xFF, nand_info->MetaSize_B);
        	mtbuff[2] = 0x42; //B
	        mtbuff[3] = 0x43; //C
	        mtbuff[4] = 0x42; //B
	        mtbuff[5] = 0x20; //' ' 
          MTD_WritePhyPageWithMeta(8*64+1, nand_info->MetaSize_B, pgbuff, mtbuff);
          MTD_WritePhyPageWithMeta(12*64+1, nand_info->MetaSize_B, pgbuff, mtbuff);


          f_close(f);
          f_unmount("/DATA/");
          vPortFree(fs);
          vPortFree(f);
          vPortFree(pgbuff);
          vPortFree(mtbuff);

          EraseWithDisp(80);


          DisplayFillBox(40, 30, 220, 80, 0);
          DisplayBox(40, 30, 220, 80, 0xFF);
          DisplayPutStr(40 + 1*4,31 + 12 * 1,"Install Completed.",255,0, 12);

        }



        break;
      case 6:   //Shutdown
        inSubLevel = 0;
        DisplayFillBox(103, 17, 254, 124, 0);
        DisplayPutStr(104,18 + 12 * 0,"Power off the device.",255,0, 12);
        if(keyEnter){
          portBoardPowerOff();
        }
        break;

      case 7:   //Reboot
        inSubLevel = 0;
        DisplayFillBox(103, 17, 254, 124, 0);
        DisplayPutStr(104,18 + 12 * 0,"Reboot the device.",255,0, 12);
        if(keyEnter){
          portBoardReset();
        }
        break;

      default:
        break;
      }

      flushInd = true;
      keyEnter = false;
      key = kb_waitAnyKeyPress();
      if(needClean)
      {
        DisplayClean();
        needClean = false;
      }
      
      switch (key)
      {
      case KEY_DOWN:
        if(!inSubMenu && (inSubLevel == 0)){
          selectItem++;
          if(selectItem > MenuItemNum - 1)
            selectItem = 0;
          RenderMainW(selectItem);
        }

        break;
      case KEY_UP:
        if(!inSubMenu && (inSubLevel == 0)){
          selectItem--;
          if(selectItem < 0)
            selectItem = MenuItemNum - 1;
          RenderMainW(selectItem);
        }
        break;

      case KEY_LEFT:
        if(inSubLevel > 0)
          inSubLevel--;
        break;
      case KEY_RIGHT:
        inSubLevel++;
        break;

      case KEY_ENTER:
        keyEnter = true;
        break;

      default:
        break;
      }


      
    }
}


void vTaskTinyUSB(void *pvParameters)
{
  tusb_init();
  for(;;)
    tud_task();
}

void vMTDSvc(void *pvParameters)
{
  MTD_DeviceInit();
  for(;;)
    MTD_Task();
}

void vFTLSvc(void *pvParameters)
{
  FTL_status = FTL_init();
  for(;;)
    FTL_task();
}

void vKeysSvc(void *pvParameters)
{
  key_svcInit();
  for(;;)
    key_task();
  
}

extern int bootTimes;
void _startup(){

  printf("Starting.(rebootTimes: %d)\n",bootTimes);

  boardInit();

	xTaskCreate( vTaskTinyUSB, "TinyUSB", configMINIMAL_STACK_SIZE, NULL, 3, NULL );
	xTaskCreate( vMTDSvc, "MTD Svc", configMINIMAL_STACK_SIZE, NULL, 4, NULL );
	xTaskCreate( vFTLSvc, "FTL Svc", configMINIMAL_STACK_SIZE, NULL, 3, NULL );
	xTaskCreate( vKeysSvc, "Keys Svc", configMINIMAL_STACK_SIZE, NULL, 3, NULL );

	xTaskCreate( vTask1, "test task1", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
	xTaskCreate( vTask2, "Main Thread", configMINIMAL_STACK_SIZE, NULL, 1, NULL );



	vTaskStartScheduler();
	printf("booting fail.\n");

	while(1);
}


void vApplicationStackOverflowHook( TaskHandle_t xTask,char * pcTaskName )
{
  PANNIC("StackOverflowHook:%s\n", pcTaskName); 
}

void vAssertCalled(char *file, int line)
{
  PANNIC("ASSERT %s:%d\n",file,line);
}

void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer,
                                     StackType_t ** ppxTimerTaskStackBuffer,
                                     uint32_t * pulTimerTaskStackSize 
)
{
  *ppxTimerTaskTCBBuffer =  (StaticTask_t * )pvPortMalloc(2048);
  *ppxTimerTaskStackBuffer = (StackType_t * )pvPortMalloc(configMINIMAL_STACK_SIZE * 4);
  *pulTimerTaskStackSize = configMINIMAL_STACK_SIZE;
}


void vApplicationMallocFailedHook()
{
  PANNIC("ASSERT: Out of Memory.\n");

}