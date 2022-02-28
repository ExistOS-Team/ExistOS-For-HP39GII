#ifndef __SYS_LLAPI_H__
#define __SYS_LLAPI_H__


uint32_t ll_putChr(char c);
uint32_t ll_putStr(char *s);
void ll_delay(uint32_t ms);
uint32_t ll_gettime_us(void);
uint32_t ll_putStr2(char *s, uint32_t len);


#endif