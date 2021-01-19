

#ifndef SERVICE_KEYBOARD_H
#define SERVICE_KEYBOARD_H

/* Kernel includes. */
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

void vServiceKeyboard(void *pvParameters);

#endif
