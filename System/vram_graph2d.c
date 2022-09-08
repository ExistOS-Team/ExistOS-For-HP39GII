#include <stdio.h>
#include <stdint.h>

#include "SystemUI.h"
#include "FreeRTOS.h"
#include "task.h"

#include "sys_llapi.h"

#include "ff.h"

#include "keyboard_gii39.h"

void sdl_frame_i(void);
void sdl_frame_clr(uint8_t c);

extern const unsigned char VGA_Ascii_5x8[];
extern const unsigned char VGA_Ascii_6x12[];
extern const unsigned char VGA_Ascii_8x16[];

static uint8_t *svram; 
void vram_initialize(uint8_t *vram_addr)
{
    svram = vram_addr;
}

void vram_put_char(int x0, int y0, char ch, int fg, int bg, int fontSize) 
{
	char *fbuff = (char *)svram;
    int font_w;
    int font_h;
    const unsigned char *pCh;
    unsigned int x = 0, y = 0, i = 0, j = 0;

    if ((ch < ' ') || (ch > '~' + 1)) {
        return;
    }

    switch (fontSize) {
    case 8:
        font_w = 8;
        font_h = 8;
        pCh = VGA_Ascii_5x8 + (ch - ' ') * font_h;
        break;

    case 12:
        font_w = 8;
        font_h = 12;
        pCh = VGA_Ascii_6x12 + (ch - ' ') * font_h;
        break;

    case 16:
        font_w = 8;
        font_h = 16;
        pCh = VGA_Ascii_8x16 + (ch - ' ') * font_h;
        break;

    default:
        return;
    }

    while (y < font_h) {
        while (x < font_w) {
            if (((x0 + x) < 256) && ((y0 + y) < 127))
                if ((*pCh << x) & 0x80U) {
                    fbuff[(x0 + x) + 256 * (y0 + y)] = fg;
                } else {
                    fbuff[(x0 + x) + 256 * (y0 + y)] = bg;
                }
            x++;
        }
        x = 0;
        y++;
        pCh++;
    }

}

void vram_put_string(int x0, int y0, char *s, int fg, int bg, int fontSize) 
{
    int font_w;
    int font_h;
    int len = strlen(s);
    int x = 0, y = 0;

    if (fontSize <= 16) {
        switch (fontSize) {
        case 8:
            font_w = 5;
            break;
        case 12:
            font_w = 6;
            break;
        case 16:
            font_w = 8;
            break;
        default:
            font_w = 8;
            break;
        }

        font_h = fontSize;
        while (*s) {
            vram_put_char((x0) + x, (y0) + y, *s, fg, bg, fontSize);
            s++;
            x += font_w;
            if (x > 256) {
                x = 0;
                y += font_h;
                if (y > 127) {
                    break;
                }
            }
        }
    }

}

