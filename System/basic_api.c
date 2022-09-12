#include <stdio.h>
#include <stdint.h>

#include "SystemUI.h"
#include "FreeRTOS.h"
#include "task.h"

#include "sys_llapi.h"

#include "ff.h"

#include "keyboard_gii39.h"

#define max(a,b) ((a) >= (b) ? (a) : (b))
#define min(a,b) ((a) <= (b) ? (a) : (b))
#define abs(x) (((int32_t)x) < 0 ? (-x) : (x))

extern const unsigned char VGA_Ascii_5x8[];
extern const unsigned char VGA_Ascii_6x12[];
extern const unsigned char VGA_Ascii_8x16[];

static uint8_t *svram = NULL; 
void api_vram_initialize(uint8_t *vram_addr)
{
    svram = vram_addr;
}

void api_vram_flush(void)
{
    if(svram)
    {
        ll_disp_put_area(svram, 0, 0, 255, 126);
    }
}

void *api_vram_get_current(void)
{
    return svram;
}

void api_vram_clear(uint16_t color)
{
    memset(svram, color, 256 * 127);
    api_vram_flush();
}

void api_vram_put_char(int x0, int y0, char ch, int fg, int bg, int fontSize) 
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

void api_vram_put_string(int x0, int y0, char *s, int fg, int bg, int fontSize) 
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
            api_vram_put_char((x0) + x, (y0) + y, *s, fg, bg, fontSize);
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

static inline void setPixelUnsafe(uint32_t x, uint32_t y, uint8_t c)
{
    svram[(x) + 256 * (y)] = c;
}

void api_vram_set_pixel(uint32_t x, uint32_t y, uint8_t c)
{
    if((x < 256) && (y < 128))
       svram[(x) + 256 * (y)] = c;
}



void api_vram_draw_HLine(int y, int x1, int x2, unsigned short c)
{
	unsigned int _x1, _x2;
	if((x1 & x2) >> 31 || x1 + x2 >= 256 || (unsigned)y > 127)
	{
		return;
	}
	
	if(x1 < x2)
	{
		_x1 = max(x1, 0);
		_x2 = min(x2, 255);
	}
	else
	{
		_x1 = max(x2, 0);
		_x2 = min(x1, 255);
	}
	for(; _x1 <= _x2; _x1++)
		setPixelUnsafe(_x1, y, c);
}

void api_vram_draw_VLine(int x, int y1, int y2, unsigned short c)
{
	unsigned int _y1, _y2;
	if((y1 & y2) >> 31 || y1 + y2 >= 128 || (unsigned)x > 255)
	{
		return;
	}
	
	if(y1 < y2)
	{
		_y1 = max(y1, 0);
		_y2 = min(y2, 127);
	}
	else
	{
		_y1 = max(y2, 0);
		_y2 = min(y1, 127);
	}
	for(; _y1 <= _y2; _y1++)
		setPixelUnsafe(x, _y1, c);
}

void api_vram_draw_line(int x1, int y1, int x2, int y2, unsigned short c)
{
	int dx = abs(x2-x1);
	int dy = abs(y2-y1);
	int sx = (x1 < x2)?1:-1;
	int sy = (y1 < y2)?1:-1;
	int err = dx-dy;
	int e2;

	while (!(x1 == x2 && y1 == y2))
	{
		api_vram_set_pixel(x1,y1,c);
		e2 = 2*err;
		if (e2 > -dy)
		{		 
			err = err - dy;
			x1 = x1 + sx;
		}
		if (e2 < dx)
		{		 
			err = err + dx;
			y1 = y1 + sy;
		}
	}
}

void api_vram_fill_rect(int x, int y, int w, int h, unsigned short c)
{
	unsigned int _x = max(x, 0), _y = max(y, 0), _w = min(256 - _x, w - _x + x), _h = min(127 - _y, h - _y + y), i, j;
	if(_x < 256 && _y < 127)
	{
		for(j = _y; j < _y + _h; j++)
			for(i = _x; i < _x + _w; i++)
				setPixelUnsafe(i, j, c);
	}
}


int api_get_key(int check_key)
{
    uint32_t keys = ll_vm_check_key();
    uint32_t keys2 = keys;
    uint16_t key, press;
    key = keys & 0xFFFF;
    press = keys >> 16;

    if(check_key >= 0)
    {
        if(check_key == key)
        {
            return press;
        }
    }else{
        while((keys = ll_vm_check_key()) == keys2){
            ;
        }
        return keys & 0xFFFF;
    }
    return 0;
}
