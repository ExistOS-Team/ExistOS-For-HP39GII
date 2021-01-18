#ifndef SERVICE_MANGER_H
#define SERVICE_MANGER_H

/* Kernel includes. */
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

void vServiceManger(void *pvParameters);

#endif
