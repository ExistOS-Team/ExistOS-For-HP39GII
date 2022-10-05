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

#define INDICATE_LEFT      (1 << 0)
#define INDICATE_RIGHT     (1 << 1)
#define INDICATE_A__Z      (1 << 2)
#define INDICATE_a__z      (1 << 3)
#define INDICATE_BUSY      (1 << 4)
#define INDICATE_TX        (1 << 5)
#define INDICATE_RX        (1 << 6)


void portDispInterfaceInit(void);
void portDispDeviceInit(void);
void portDispFlushAreaBuf(uint32_t x_start, uint32_t y_start, uint32_t x_end, uint32_t y_end, uint8_t *buf);
void portDispReadBackVRAM(uint32_t x_start, uint32_t y_start, uint32_t x_end, uint32_t y_end, uint8_t *buf);
void portDispSetIndicate(int indicateBit, int batteryBit);
void portDispClean(void);
void portDispSetContrast(uint8_t contrast);

bool DisplayOperatesFin();
void DisplayPrepareBatchIn(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1);
void DisplayBatchIn(uint8_t *dat, uint32_t len);
void DisplayReadArea(uint32_t x_start, uint32_t y_start, uint32_t x_end, uint32_t y_end, uint8_t *buf, bool *fin);
void DisplayFlushArea(uint32_t x_start, uint32_t y_start, uint32_t x_end, uint32_t y_end, uint8_t *buf, bool block);
void DisplayPutChar(uint32_t x, uint32_t y, char c, uint8_t fg, uint8_t bg, uint8_t fontSize);
bool DisplayPutStr(uint32_t x, uint32_t y, char *s, uint8_t fg, uint8_t bg, uint8_t fontSize);
void DisplayBox(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint8_t c);
void DisplayBoxBlock(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint8_t c);
void DisplayHLine(uint32_t y0, uint32_t y1, uint32_t x, uint8_t c);
void DisplayVLine(uint32_t y0, uint32_t y1, uint32_t x, uint8_t c);
void DisplayFillBox(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint8_t c);
//void DisplayCircle(uint32_t x0, uint32_t y0, uint32_t r, uint8_t c, bool isFill);
void DisplaySetIndicate(int Indicate, int batInd);
void DisplayClean(void);

void Display_InterfaceInit(void);
void DisplayInit(void);
void DisplayTask(void);

//float __sqrt(int x);

#endif


