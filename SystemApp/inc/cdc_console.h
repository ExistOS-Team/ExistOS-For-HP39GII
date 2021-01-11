#ifndef CDC_CONSOLE_H
#define CDC_CONSOLE_H

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

void vInit();
void vCDC_Console();

void cdc_p();
void cdc_flush();

#endif