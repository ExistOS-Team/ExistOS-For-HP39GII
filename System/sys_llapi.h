#ifndef __SYS_LLAPI_H__
#define __SYS_LLAPI_H__

#include <stdint.h>
#include <stdbool.h>


volatile uint32_t ll_putChr(char c);
volatile uint32_t ll_putStr(char *s);
volatile void ll_delay(uint32_t ms);
volatile uint32_t ll_gettime_us(void);
volatile uint32_t ll_putStr2(char *s, uint32_t len);

volatile void ll_irq_set_Vector(void *p);
volatile void ll_irq_set_Stack(void *p);
volatile void ll_irq_enable(bool enable);
volatile void ll_irq_get_context(uint32_t *save);
volatile void ll_irq_set_context(uint32_t *load);
volatile void ll_irq_restore_context() __attribute__((naked));
volatile void ll_systick_enable(bool enable);
volatile void ll_systick_set_period(uint32_t periodMs);



#endif