

#ifndef SERVICE_FLASH_MAP_H
#define SERVICE_FLASH_MAP_H
#include "stdbool.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define GC_RATIO 4
void flashSyncNow();
void vServiceFlashMap( void *pvParameters );
void flashMapReset();
bool isMaplock();
void lockFmap(bool lock);
void flashMapClear();

#endif

