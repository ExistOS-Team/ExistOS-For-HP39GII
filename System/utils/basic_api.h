#pragma once

#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void api_vram_initialize(uint8_t *vram_addr);
void *api_vram_get_current(void);
void api_vram_flush(void);
void api_vram_clear(uint16_t color);
void api_vram_put_char(int x0, int y0, char ch, int fg, int bg, int fontSize);
void api_vram_put_string(int x0, int y0, char *s, int fg, int bg, int fontSize);
void api_vram_set_pixel(uint32_t x, uint32_t y, uint8_t c);
void api_vram_draw_HLine(int y, int x1, int x2, unsigned short c);
void api_vram_draw_VLine(int x, int y1, int y2, unsigned short c);
void api_vram_draw_line(int x1, int y1, int x2, int y2, unsigned short c);
void api_vram_fill_rect(int x, int y, int w, int h, unsigned short c);



int api_get_key(int check_key);


#ifdef __cplusplus
}
#endif

