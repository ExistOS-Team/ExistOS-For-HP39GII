#ifndef __SYS_LLAPI_H__
#define __SYS_LLAPI_H__

#include <stdint.h>
#include <stdbool.h>

#include "llapi_code.h"

void ll_putChr(char c);
void ll_putStr(char *s);
void ll_delay(uint32_t ms);
uint32_t ll_gettime_us(void);
void ll_putStr2(char *s, uint32_t len);


uint32_t ll_DispFlush(DispFlushInfo_t *s);


void ll_setTimer(bool enbale, uint32_t period_ms);
void ll_set_irq_vector(uint32_t addr);
void ll_set_irq_stack(uint32_t addr);
void ll_load_context(uint32_t addr) __attribute__((naked));


void ll_set_configAddr(uint32_t addr) __attribute__((naked));

void ll_enable_irq(bool enable);

 uint32_t ll_create_task(CreateTaskInfo_t *s);


 
void ll_DispPutStr(char *s, uint32_t x0, uint32_t y0, uint8_t fg, uint8_t bg, uint8_t fontsize);
void ll_DispPutBox(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, bool fill, uint8_t color);

void ll_DispHLine(uint32_t y, uint32_t x0, uint8_t x1, uint8_t color);
void ll_DispVLine(uint32_t x, uint32_t y0, uint8_t y1, uint8_t color);

void ll_DispPutArea(uint8_t *dat, uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1);
void ll_DispSetIndicate(uint32_t indicateBit, uint8_t BatInt);
void ll_DispSendScreen(void);

#endif