

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"

#include "display_up.h"
#include "font_ascii.h"

void DisplayClean(void)
{
    portDispClean();
}


void DisplayFlushArea(uint32_t x_start, uint32_t y_start, uint32_t x_end, uint32_t y_end, uint8_t *buf)
{
    portDispFlushAreaBuf(x_start, y_start, x_end, y_end, buf);
}

void DisplayPutChar(uint32_t x, uint32_t y, char c, uint8_t fg, uint8_t bg, uint8_t fontSize)
{
    uint32_t n = c - ' ';
    uint8_t* p = &Ascii1608[fontSize * n];
    
    uint8_t* buf = pvPortMalloc(8 * fontSize);

    for(int y = 0; y < fontSize; y++){
        for(int x = 0; x < 8; x++){
            buf[8*y + x] = ((p[y] >> (7 - x))&1) ? fg : bg;
        }
    }
    DisplayFlushArea(x, y, x + (8 - 1), y + (fontSize - 1), buf);
    vPortFree(buf);
}

bool DisplayPutStr(uint32_t x, uint32_t y, char *s, uint8_t fg, uint8_t bg,  uint8_t fontSize)
{
    uint32_t str_len = strlen(s);

    uint8_t *font;
    uint32_t fontWidth;
    if(fontSize == 16){
        font = Ascii1608;
        fontWidth = 8;
    }else{
        font = Ascii1206;
        fontWidth = 7;
    }
    
    uint8_t* buf = pvPortMalloc(8 * str_len * fontSize);
    uint32_t x0 = 0;
    for(int n=0; n<str_len; n++)
    {
        char c = s[n];
        uint32_t ch = c - ' ';
        uint8_t* p = &font[fontSize * ch];
        for(int y_ = 0; y_ < fontSize; y_++){
            for(int x_ = x0; x_ < x0+8; x_++){
                buf[fontWidth*str_len*y_ + x_] = ((p[y_] >> (7 - (x_ - x0)))&1) ? fg : bg;
            }
        }
        x0 += fontWidth;
    }

    DisplayFlushArea(x, y, x + fontWidth*str_len - 1, y + (fontSize - 1), buf);

    vPortFree(buf);
    
    return true;
}


void DisplayHLine(uint32_t x0, uint32_t x1, uint32_t y,uint8_t c)
{
    uint8_t* buf = pvPortMalloc(x1 - x0 + 1);
    if(!buf){
        return;
    }
    memset(buf, c, x1 - x0 + 1);

    DisplayFlushArea(x0, y, x1, y, buf);
    vPortFree(buf);
}

void DisplayVLine(uint32_t y0, uint32_t y1, uint32_t x, uint8_t c)
{
    uint8_t* buf = pvPortMalloc(y1 - y0 + 1);
    if(!buf){
        return;
    }
    memset(buf, c, y1 - y0 + 1);
    DisplayFlushArea(x, y0, x, y1, buf);
    vPortFree(buf);
}

void DisplayBox(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint8_t c)
{
    DisplayHLine(x0, x1, y0, c);
    DisplayHLine(x0, x1, y1, c);
    DisplayVLine(y0, y1, x0, c);
    DisplayVLine(y0, y1, x1, c);
}

void DisplayFillBox(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint8_t c)
{
    for(int y = y0; y <= y1; y++){
        DisplayHLine(x0, x1, y, c);
    }
}

void Display_InterfaceInit()
{
    portDispInterfaceInit();
}


void DisplayInit()
{
    portDispDeviceInit();
}
