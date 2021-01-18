#ifndef CDC_CONSOLE_H
#define CDC_CONSOLE_H

/* Kernel includes. */
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

void vInit();
void vCDC_Console();

void cdc_p(const char *fmt, ...);
void cdc_flush();
void cdc_clear();

#endif