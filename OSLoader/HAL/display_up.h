#ifndef __DISPLAY_UP_H__
#define __DISPLAY_UP_H__

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>


void portDispInterfaceInit(void);
void portDispDeviceInit(void);
void portDispFlushAreaBuf(uint32_t x_start, uint32_t y_start, uint32_t x_end, uint32_t y_end, uint8_t *buf);
void portDispClean(void);


void DisplayPutChar(uint32_t x, uint32_t y, char c, uint8_t fg, uint8_t bg, uint8_t fontSize);
bool DisplayPutStr(uint32_t x, uint32_t y, char *s, uint8_t fg, uint8_t bg, uint8_t fontSize);
void DisplayBox(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint8_t c);
void DisplayHLine(uint32_t y0, uint32_t y1, uint32_t x, uint8_t c);
void DisplayVLine(uint32_t y0, uint32_t y1, uint32_t x, uint8_t c);
void DisplayFillBox(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint8_t c);
void DisplayClean(void);

void Display_InterfaceInit(void);
void DisplayInit(void);



#endif


