#pragma once

#include <stdio.h>
#include <stdint.h>

void vram_initialize(uint8_t *vram_addr);
void vram_put_char(int x0, int y0, char ch, int fg, int bg, int fontSize);
void vram_put_string(int x0, int y0, char *s, int fg, int bg, int fontSize);

