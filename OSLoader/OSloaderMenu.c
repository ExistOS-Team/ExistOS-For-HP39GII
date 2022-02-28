



#include <stdio.h>
#include "SystemConfig.h"
#include "OSloaderMenu.h"
#include "../debug.h"

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



#define MenuItemNum  (sizeof(MenuItems)/sizeof(uint32_t))

uint32_t getMenuItemNum()
{
    return MenuItemNum;
}

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


void RenderFaultReason(uint32_t AbortRes)
{

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
        DisplayPutStr(104,18 + 12 * 6,"Version: " VERSION,255,0, 12);
}






