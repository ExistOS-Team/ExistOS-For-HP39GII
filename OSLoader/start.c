
#include <stdio.h>

#include "SystemConfig.h"

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
#include "stmp37xxNandConf.h"

#include "OSloaderMenu.h"
#include "vmMgr.h"

#include "llapi.h"


PARTITION VolToPart[FF_VOLUMES] = {
    {0, 1},     /* "0:" ==> 1st partition on the pd#0 */
    {0, 2}      /* "1:" ==> 2nd partition on the pd#0 */
};




TaskHandle_t pSysTask;

uint32_t CurMount = 0;
uint32_t FTL_status = 0;

static bool fileSystemOK = false;
static bool diskInit = false;

char pcWriteBuffer[4096];
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
		    
		vTaskDelay(pdMS_TO_TICKS(17000));
		printTaskList();
    }
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
  PartitionInfo_t *PartitionInfo = FTL_GetPartitionInfo();
  *start = PartitionInfo->SectorStart[p];
  *sectors = PartitionInfo->Sectors[p];
}

uint32_t MSCpartStartSector;
uint32_t MSCpartSectors;
void setMountPartition(uint32_t p)
{
  if(p == 2){
    MSCpartStartSector = 0;
    MSCpartSectors = FTL_GetSectorCount();
    return;
  }
  GetPartitionInfo(p, &MSCpartStartSector, &MSCpartSectors);
}


uint32_t *bootAddr = (uint32_t *)VM_ROM_BASE;
void System(void *par)
{
  __asm volatile("mrs r1,cpsr_all");
  __asm volatile("bic r1,r1,#0x1f");
  __asm volatile("orr r1,r1,#0x10");
  __asm volatile("msr cpsr_all,r1");

  __asm volatile("ldr r0,=bootAddr");
  __asm volatile("ldr r0,[r0]");
  __asm volatile("mov pc,r0");
  
  while(1);
}

bool bootSystem()
{

    FRESULT ret;
    FILINFO sysFinfo;
    FATFS *fs = pvPortMalloc(sizeof(FATFS));
    f_mount(fs, "/SYS/", 1);
    ret = f_stat("/SYS/ExistOS.sys", &sysFinfo);
    INFO("fsize:%lu \n",sysFinfo.fsize);
    INFO("f_stat:%d\n",ret);
    if(ret != FR_OK){
        f_unmount("/SYS/");
        vPortFree(fs);
        return false;
    }

    FIL *sysFile = pvPortMalloc(sizeof(FIL));
    ret = f_open(sysFile, "/SYS/ExistOS.sys" ,FA_READ);
    INFO("f_open:%d\n",ret);
    if(ret != FR_OK){
        f_unmount("/SYS/");
        vPortFree(fs);
        vPortFree(sysFile);
        return false;
    }

    while(!vmMgrInited()){
      vTaskDelay(pdMS_TO_TICKS(10));
    }


    vmMgr_mapSwap();
    vmMgr_mapFile(sysFile, PERM_R, VM_ROM_BASE, 0, (sysFinfo.fsize + PAGE_SIZE) & 0xFFFFF000);
    vTaskResume(pSysTask);
    
    vTaskDelete(xTaskGetCurrentTaskHandle());

    return true;

}


void vMainThread(void *pvParameters) 
{
    Keys_t key;
    int selectItem = 0;
    int AbortRes = 0;
    int inSubLevel = 0;

    bool inSubMenu = false;
    bool keyEnter = false;
    bool needClean = false;


    mtdInfo_t *nand_info;    

    

    if(kb_isKeyPress(KEY_BACKSPACE) == true){
      AbortRes = 1;
      diskInit = false;
      if(!FTL_status){
        FTL_ScanPartition();
        diskInit = true;
      }

      goto Configuration;
    }

    if(FTL_status){
      diskInit = false;
      AbortRes = 2;
      goto Configuration;
    }

    if(!FTL_ScanPartition()){
      diskInit = false;
      AbortRes = 2;
      goto Configuration;
    }

    {
      diskInit = true;
      FRESULT fres;
      FATFS *fs = pvPortMalloc(sizeof(FATFS));
      fres = f_mount(fs, "/SYS/", 1);
      INFO("Test Mount 1:%d\n", fres);
      if(fres != FR_OK){
        AbortRes = 2;
        vPortFree(fs);
        goto Configuration;
      }
      f_unmount("/SYS/");
/*
      fres = f_mount(fs, "/DATA/", 1);
      INFO("Test Mount 2:%d\n", fres);
      if(fres != FR_OK){
        AbortRes = 2;
        vPortFree(fs);
        goto Configuration;
      }
      f_unmount("/DATA/");
*/
      vPortFree(fs);
    }

    /* Normal Boot */
    fileSystemOK = true;  

    if(bootSystem() == false){
        AbortRes = 0;
        goto Configuration;
    }


    while(1)
    {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }



    Configuration:

    RenderMainW(selectItem);
    xTaskCreate(vTaskIndicate, "RenderInd", configMINIMAL_STACK_SIZE, NULL, 1, NULL );


    
    
    nand_info = MTD_getDeviceInfo();

    for (;;) {
      flushInd = false;
      vTaskDelay(pdMS_TO_TICKS(40));
      switch (selectItem)
      {
      case 0:
        inSubLevel = 0;
        RenderFaultReason(AbortRes);
        break;
      case 1: //Boot ExistOS
        inSubLevel = 0;
        DisplayFillBox(103, 17, 254, 124, 0);
        DisplayPutStr(104,18 + 12 * 0,"Press ENTER to",255,0, 12);
        DisplayPutStr(104,18 + 12 * 1,"Boot /SYS/ExistOS.sys",255,0, 12);

        if(keyEnter)
        {
          if(bootSystem() == false){
            DisplayFillBox(40, 30, 220, 80, 0);
            DisplayBox(40, 30, 220, 80, 0xFF);
            DisplayPutStr(40 + 1*4,31 + 12 * 1,"Boot failed.",255,0, 12);
          }else{
            DisplayClean();
            vTaskDelete(xTaskGetCurrentTaskHandle());
          }
        }

        break;
      
      case 2: //Mount USB MSC
        DisplayFillBox(103, 17, 254, 124, 0);
        if(diskInit)
        {
          if( inSubLevel > 4){
            inSubLevel = 1;
          }
          DisplayPutStr(104,18 + 12 * 0,"Select which Partition",255,0, 12);
          DisplayPutStr(104,18 + 12 * 1,"to mount on USB MSC.",255,0, 12);

          DisplayPutStr(104 + 10         ,18 + 12 * 3,"NONE",(inSubLevel == 1) ? 0 : 255,(inSubLevel == 1) ? 255 : 0, 12);
          DisplayPutStr(104 + 10 + (6*6) ,18 + 12 * 3,"DATA",(inSubLevel == 2) ? 0 : 255,(inSubLevel == 2) ? 255 : 0, 12);
          DisplayPutStr(104 + 10 + (12*6),18 + 12 * 3,"SYS ",(inSubLevel == 3) ? 0 : 255,(inSubLevel == 3) ? 255 : 0, 12);
          DisplayPutStr(104 + 10 + (17*6),18 + 12 * 3,"ALL ",(inSubLevel == 4) ? 0 : 255,(inSubLevel == 4) ? 255 : 0, 12);

          if(keyEnter && (inSubLevel > 0)){
            CurMount = inSubLevel - 1;
            switch (CurMount)
            {
            case 0:
              tud_disconnect();
              FTL_Sync();
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
            
            case 3:
              {
                tud_disconnect();
                setMountPartition(2);
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
              DisplayPutStr(104 + 10 + (6*6)  ,18 + 12 * 4,"^^^^",255,0, 12);
              break;
            case 2:
              DisplayPutStr(104 + 10 + (12*6) ,18 + 12 * 4,"^^^^",255,0, 12);
              break;
            case 3:
              DisplayPutStr(104 + 10 + (17*6) ,18 + 12 * 4,"^^^^",255,0, 12);
              break;  
            default:
              break;
            }

        }else{
          
          DisplayPutStr(104,18 + 12 * 0,"Flash not Formatted, ",255,0, 12);
          DisplayPutStr(104,18 + 12 * 1,"please format before",255,0, 12);
          DisplayPutStr(104,18 + 12 * 2,"mount DATA or SYS",255,0, 12);
          DisplayPutStr(104,18 + 12 * 3,"partition.",255,0, 12);
          if( inSubLevel > 2){
            inSubLevel = 1;
          }
          DisplayPutStr(104 + 10          ,18 + 12 * 5,"NONE",(inSubLevel == 1) ? 0 : 255,(inSubLevel == 1) ? 255 : 0, 12);
          DisplayPutStr(104 + 10 + (14*6) ,18 + 12 * 5,"ALL ",(inSubLevel == 2) ? 0 : 255,(inSubLevel == 2) ? 255 : 0, 12);

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
                setMountPartition(2);
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
              DisplayPutStr(104 + 10          ,18 + 12 * 6,"^^^^",255,0, 12);
              break;
            case 1:
              DisplayPutStr(104 + 10 + (14*6)  ,18 + 12 * 6,"^^^^",255,0, 12);
              break;
            default:
              break;
            }



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

        DisplayPutStr(104 + 10         , 18 + 12 * 3,"Erase ",(inSubLevel == 1) ? 0 : 255,(inSubLevel == 1) ? 255 : 0, 12);
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

              LBA_t plist[] = DISK_PARTITION;
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

                FTL_Sync();
                FTL_MapInit();

                DisplayPutStr(40 + 6*4,31 + 12 * 1,"Format Finish.",255,0, 12);
                diskInit = true;
              }
              
            }

            inSubLevel = 0;
            break;

          default:
            break;
          }
          FTL_ScanPartition();
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

          RestoreLDLB2();

          f_close(f);
          f_unmount("/DATA/");
          vPortFree(fs);
          vPortFree(f);
          vPortFree(pgbuff);
          vPortFree(mtbuff);

          EraseWithDisp(80);


          DisplayFillBox(40, 30, 220, 80, 0);
          DisplayBox(40, 30, 220, 80, 0xFF);
          DisplayPutStr(40 + 1*4,31 + 12 * 1,"Restore Completed?",255,0, 12);

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
        RenderMainW(selectItem);
        needClean = false;
      }
      
      switch (key)
      {
      case KEY_DOWN:
        if(!inSubMenu && (inSubLevel == 0)){
          selectItem++;
          if(selectItem > getMenuItemNum() - 1)
            selectItem = 0;
          RenderMainW(selectItem);
        }

        break;
      case KEY_UP:
        if(!inSubMenu && (inSubLevel == 0)){
          selectItem--;
          if(selectItem < 0)
            selectItem = getMenuItemNum() - 1;
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




void vVMMgrSvc(void *pvParameters)
{

  vmMgr_init();

  for(;;){
    vmMgr_task();
  }
}

void vLLAPISvc(void *pvParameters)
{

  LLAPI_init();
  for(;;){
    LLAPI_Task();
  }
}

void vDispSvc(void *pvParameters)
{

  DisplayInit();
  for(;;){
    DisplayTask();
  }
}

extern int bootTimes;
void _startup(){

  printf("Starting.(rebootTimes: %d)\n",bootTimes);

  boardInit();

	xTaskCreate( vMTDSvc, "MTD Svc", configMINIMAL_STACK_SIZE, NULL, 5, NULL );
	xTaskCreate( vFTLSvc, "FTL Svc", configMINIMAL_STACK_SIZE, NULL, 4, NULL );

  xTaskCreate( vVMMgrSvc, "VM Svc", configMINIMAL_STACK_SIZE, NULL, 3, NULL );
	xTaskCreate( vTaskTinyUSB, "TinyUSB", configMINIMAL_STACK_SIZE, NULL, 3, NULL );

	xTaskCreate( vKeysSvc, "Keys Svc", configMINIMAL_STACK_SIZE, NULL, 2, NULL );
	xTaskCreate( vDispSvc, "Display Svc", configMINIMAL_STACK_SIZE, NULL, 2, NULL );
  xTaskCreate( vLLAPISvc, "LLAPI Svc", configMINIMAL_STACK_SIZE, NULL, 2, NULL );
  
	xTaskCreate( vMainThread, "Main Thread", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
  xTaskCreate( System, "System", configMINIMAL_STACK_SIZE, NULL, 1, &pSysTask);
	xTaskCreate( vTask1, "Status Print", configMINIMAL_STACK_SIZE, NULL, 0, NULL );

  vTaskSuspend(pSysTask);

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