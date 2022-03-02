#ifndef __SYS_LLAPI_H__
#define __SYS_LLAPI_H__

#include <stdint.h>
#include <stdbool.h>


uint32_t ll_putChr(char c);
uint32_t ll_putStr(char *s);
void ll_delay(uint32_t ms);
uint32_t ll_gettime_us(void);
uint32_t ll_putStr2(char *s, uint32_t len);

void ll_irq_set_Vector(void *p);
void ll_irq_set_Stack(void *p);
void ll_irq_enable(bool enable);
void ll_irq_get_context(uint32_t *save);
void ll_irq_set_context(uint32_t *load);
void ll_irq_restore_context() __attribute__((naked));
void ll_systick_enable(bool enable);
void ll_systick_set_period(uint32_t periodMs);



#endif