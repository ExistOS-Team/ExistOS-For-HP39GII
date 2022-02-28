
#ifndef __OSLOADER_MENU_H__
#define __OSLOADER_MENU_H__

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "board_up.h"
#include "mtd_up.h"
#include "FTL_up.h"
#include "display_up.h"
#include "keyboard_up.h"



void RenderMainW(uint32_t selectMenu);
void vTaskIndicate(void *pvParameters);
void RenderFaultReason(uint32_t AbortRes);
uint32_t getMenuItemNum(void);

extern bool flushInd;;

#endif