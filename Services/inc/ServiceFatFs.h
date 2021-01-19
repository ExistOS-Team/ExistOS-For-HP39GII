

#ifndef SERVICE_FATFS_H
#define SERVICE_FATFS_H

/* Kernel includes. */
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

void vServiceFatfs(void *pvParameters);
unsigned int isfatFsInited();

#endif
