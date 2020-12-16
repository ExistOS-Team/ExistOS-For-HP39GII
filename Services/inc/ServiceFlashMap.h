

#ifndef SERVICE_FLASH_MAP_H
#define SERVICE_FLASH_MAP_H

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define GC_RATIO 4

void vServiceFlashMap( void *pvParameters );


#endif

