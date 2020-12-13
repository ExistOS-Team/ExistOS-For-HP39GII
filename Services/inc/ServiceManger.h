#ifndef SERVICE_MANGER_H
#define SERVICE_MANGER_H

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

void vServiceManger( void *pvParameters );

#endif
