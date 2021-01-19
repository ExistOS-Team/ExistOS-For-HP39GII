#ifndef CDC_CONSOLE_H
#define CDC_CONSOLE_H

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

void vInit();
void vCDC_Console();

void cdc_p(const char *fmt, ...);
void cdc_flush();
void cdc_clear();

#endif