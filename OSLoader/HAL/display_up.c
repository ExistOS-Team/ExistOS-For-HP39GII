

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"

#include "display_up.h"
#include "font_ascii.h"

typedef enum {
    DISPOPA_CLEAN,
    DISPOPA_FLUSH_AREA,
    DISPOPA_PUT_CHAR,
    DISPOPA_PUT_STR,
    DISPOPA_HLINE,
    DISPOPA_VLINE,
    DISPOPA_BOX,
    DISPOPA_FILL_BOX,
    DISPOPA_SET_INDICATE,
    DISPOPA_READ_VRAM
}DispOpa;

typedef struct DisplayOpaQueue_t
{
    DispOpa opa;
    uint32_t parNum;
    uint32_t *pars;
}DisplayOpaQueue_t;

QueueHandle_t DisplayOpaQueue;

void DisplayClean(void)
{
    DisplayOpaQueue_t opa;
    opa.opa = DISPOPA_CLEAN;
    opa.parNum = 0;
    xQueueSend(DisplayOpaQueue, &opa, portMAX_DELAY);
}


void DisplayReadArea(uint32_t x_start, uint32_t y_start, uint32_t x_end, uint32_t y_end, uint8_t *buf, bool *fin)
{
    *fin = false;
    if((x_end - x_start + 1) * (y_end - y_start + 1) > 4096){
        *fin = true;
        return;
    }
    DisplayOpaQueue_t opa;
    uint32_t *pars = pvPortMalloc(6 * sizeof(uint32_t));
    if(!pars)return;
    pars[0] = x_start;
    pars[1] = y_start;
    pars[2] = x_end;
    pars[3] = y_end;
    pars[4] = (uint32_t )buf;
    pars[5] = (uint32_t )fin;
    opa.opa = DISPOPA_READ_VRAM;
    opa.parNum = 5;
    opa.pars = pars;
    xQueueSend(DisplayOpaQueue, &opa, portMAX_DELAY);
}

void DisplayFlushArea(uint32_t x_start, uint32_t y_start, uint32_t x_end, uint32_t y_end, uint8_t *buf, bool block)
{
    DisplayOpaQueue_t opa;
    uint32_t *pars = pvPortMalloc(6 * sizeof(uint32_t));
    if(!pars)return;

    volatile bool fin = false;

    pars[0] = x_start;
    pars[1] = y_start;
    pars[2] = x_end;
    pars[3] = y_end;
    pars[4] = (uint32_t )buf;
    pars[5] = (uint32_t )&fin;

    opa.opa = DISPOPA_FLUSH_AREA;
    opa.parNum = 6;
    opa.pars = pars;
    xQueueSend(DisplayOpaQueue, &opa, portMAX_DELAY);
    if(block)
    {
        while(fin == false){
            //vTaskDelay(10);
        }
    }
    
}

void DisplayPutChar(uint32_t x, uint32_t y, char c, uint8_t fg, uint8_t bg, uint8_t fontSize)
{
    DisplayOpaQueue_t opa;
    uint32_t *pars = pvPortMalloc(6 * sizeof(uint32_t));
    if(!pars)return;
    pars[0] = x;
    pars[1] = y;
    pars[2] = c;
    pars[3] = fg;
    pars[4] = bg;
    pars[5] = fontSize;
    opa.opa = DISPOPA_PUT_CHAR;
    opa.parNum = 6;
    opa.pars = pars;
    xQueueSend(DisplayOpaQueue, &opa, portMAX_DELAY);
}

bool DisplayPutStr(uint32_t x, uint32_t y, char *s, uint8_t fg, uint8_t bg,  uint8_t fontSize)
{
    DisplayOpaQueue_t opa;
    uint32_t *pars = pvPortMalloc(6 * sizeof(uint32_t));
    if(!pars)return false;
    pars[0] = x;
    pars[1] = y;
    pars[2] = (uint32_t)s;
    pars[3] = fg;
    pars[4] = bg;
    pars[5] = fontSize;
    opa.opa = DISPOPA_PUT_STR;
    opa.parNum = 6;
    opa.pars = pars;
    xQueueSend(DisplayOpaQueue, &opa, portMAX_DELAY);

    return true;
}


void DisplayHLine(uint32_t x0, uint32_t x1, uint32_t y,uint8_t c)
{
    DisplayOpaQueue_t opa;
    uint32_t *pars = pvPortMalloc(4 * sizeof(uint32_t));
    if(!pars)return;
    pars[0] = x0;
    pars[1] = x1;
    pars[2] = y;
    pars[3] = c;
    opa.opa = DISPOPA_HLINE;
    opa.parNum = 4;
    opa.pars = pars;
    xQueueSend(DisplayOpaQueue, &opa, portMAX_DELAY);
}

void DisplayVLine(uint32_t y0, uint32_t y1, uint32_t x, uint8_t c)
{
    DisplayOpaQueue_t opa;
    uint32_t *pars = pvPortMalloc(4 * sizeof(uint32_t));
    if(!pars)return;
    pars[0] = y0;
    pars[1] = y1;
    pars[2] = x;
    pars[3] = c;
    opa.opa = DISPOPA_VLINE;
    opa.parNum = 4;
    opa.pars = pars;
    xQueueSend(DisplayOpaQueue, &opa, portMAX_DELAY);
}

void DisplayBox(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint8_t c)
{
    DisplayOpaQueue_t opa;
    uint32_t *pars = pvPortMalloc(5 * sizeof(uint32_t));
    if(!pars)return;
    pars[0] = x0;
    pars[1] = y0;
    pars[2] = x1;
    pars[3] = y1;
    pars[4] = c;
    opa.opa = DISPOPA_BOX;
    opa.parNum = 5;
    opa.pars = pars;
    xQueueSend(DisplayOpaQueue, &opa, portMAX_DELAY);
}

void DisplayFillBox(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint8_t c)
{
    DisplayOpaQueue_t opa;
    uint32_t *pars = pvPortMalloc(5 * sizeof(uint32_t));
    if(!pars)return;
    pars[0] = x0;
    pars[1] = y0;
    pars[2] = x1;
    pars[3] = y1;
    pars[4] = c;
    opa.opa = DISPOPA_FILL_BOX;
    opa.parNum = 5;
    opa.pars = pars;
    xQueueSend(DisplayOpaQueue, &opa, portMAX_DELAY);
}

void DisplaySetIndicate(uint32_t Indicate, uint32_t batInd)
{
    DisplayOpaQueue_t opa;
    uint32_t *pars = pvPortMalloc(2 * sizeof(uint32_t));
    if(!pars)return;
    pars[0] = Indicate;
    pars[1] = batInd;
    opa.opa = DISPOPA_SET_INDICATE;
    opa.parNum = 2;
    opa.pars = pars;
    xQueueSend(DisplayOpaQueue, &opa, portMAX_DELAY);
}

static void innerDispHLine(uint32_t x0, uint32_t x1, uint32_t y,uint8_t c)
{
    uint8_t* buf = pvPortMalloc(x1 - x0 + 1);
    memset(buf, c, x1 - x0 + 1);
    portDispFlushAreaBuf(x0, y, x1, y, buf);
    vPortFree(buf);
}

static void innerDispVLine(uint32_t y0, uint32_t y1, uint32_t x, uint8_t c)
{
    uint8_t* buf = pvPortMalloc(y1 - y0 + 1);
    memset(buf, c, y1 - y0 + 1);
    portDispFlushAreaBuf(x, y0, x, y1, buf);
    vPortFree(buf);
}

void Display_InterfaceInit()
{
    portDispInterfaceInit();
}

void DisplayTask()
{
    DisplayOpaQueue_t curOpa;
    while(xQueueReceive(DisplayOpaQueue, &curOpa, portMAX_DELAY) == pdTRUE){

        switch (curOpa.opa)
        {
            case DISPOPA_CLEAN:
                portDispClean();
                break;
            case DISPOPA_FLUSH_AREA:
                {
                    uint32_t x_start = curOpa.pars[0];
                    uint32_t y_start = curOpa.pars[1]; 
                    uint32_t x_end = curOpa.pars[2]; 
                    uint32_t y_end = curOpa.pars[3]; 
                    uint8_t *buf = (uint8_t *)curOpa.pars[4];
                    portDispFlushAreaBuf(x_start, y_start, x_end, y_end, buf);
                    
                    

                    bool *fin = (bool *)curOpa.pars[5];
                    *fin = true;

                    vPortFree(curOpa.pars);
                }

                break;
            case DISPOPA_PUT_CHAR:
                {
                    if(curOpa.parNum != 6){
                        break;
                    }
                    uint32_t x = curOpa.pars[0];
                    uint32_t y = curOpa.pars[1];
                    char c = curOpa.pars[2];
                    uint8_t fg = curOpa.pars[3];
                    uint8_t bg = curOpa.pars[4];
                    uint8_t fontSize = curOpa.pars[5];

                    uint32_t n = c - ' ';
                    uint8_t* p = &Ascii1608[fontSize * n];
                    uint8_t* buf = pvPortMalloc(8 * fontSize);
                    for(int y = 0; y < fontSize; y++){
                        for(int x = 0; x < 8; x++){
                            buf[8*y + x] = ((p[y] >> (7 - x))&1) ? fg : bg;
                        }
                    }
                    portDispFlushAreaBuf(x, y, x + (8 - 1), y + (fontSize - 1), buf);
                    vPortFree(buf);
                    vPortFree(curOpa.pars);
                }

                break;
            case DISPOPA_PUT_STR:
                {
                    if(curOpa.parNum != 6){
                        break;
                    }
                    uint32_t x = curOpa.pars[0];
                    uint32_t y = curOpa.pars[1];
                    char* s    = (char *)curOpa.pars[2];
                    uint8_t fg = curOpa.pars[3];
                    uint8_t bg = curOpa.pars[4];
                    uint8_t fontSize = curOpa.pars[5];

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

                    portDispFlushAreaBuf(x, y, x + fontWidth*str_len - 1, y + (fontSize - 1), buf);

                    vPortFree(buf);
                    vPortFree(curOpa.pars);
                }
                break;
            case DISPOPA_HLINE:
                
                {
                    uint32_t x0 = curOpa.pars[0];
                    uint32_t x1 = curOpa.pars[1];
                    uint32_t y = curOpa.pars[2];
                    uint8_t c = curOpa.pars[3];
                    innerDispHLine(x0, x1, y, c);
                    vPortFree(curOpa.pars);
                }
                break;
            case DISPOPA_VLINE:
                {
                    uint32_t y0 = curOpa.pars[0];
                    uint32_t y1 = curOpa.pars[1];
                    uint32_t x = curOpa.pars[2];
                    uint8_t c = curOpa.pars[3];
                    innerDispVLine(y0, y1, x, c);
                    vPortFree(curOpa.pars);
                }
                break;
            case DISPOPA_BOX:
                {
                    uint32_t x0 = curOpa.pars[0];
                    uint32_t y0 = curOpa.pars[1];
                    uint32_t x1 = curOpa.pars[2]; 
                    uint32_t y1 = curOpa.pars[3];
                    uint8_t c = curOpa.pars[4];

                    innerDispHLine(x0, x1, y0, c);
                    innerDispHLine(x0, x1, y1, c);
                    innerDispVLine(y0, y1, x0, c);
                    innerDispVLine(y0, y1, x1, c);

                    vPortFree(curOpa.pars);
                }

                break;
            case DISPOPA_FILL_BOX:
            {
                    uint32_t x0 = curOpa.pars[0];
                    uint32_t y0 = curOpa.pars[1];
                    uint32_t x1 = curOpa.pars[2]; 
                    uint32_t y1 = curOpa.pars[3];
                    uint8_t c = curOpa.pars[4];
                    for(int y = y0; y <= y1; y++){
                        innerDispHLine(x0, x1, y, c);
                    }
                    vPortFree(curOpa.pars);
            }
                break;

            case DISPOPA_SET_INDICATE:
                {
                    uint32_t indicate = curOpa.pars[0];
                    uint32_t batIndicate = curOpa.pars[1];
                    portDispSetIndicate(indicate, batIndicate);
                    vPortFree(curOpa.pars);
                }
                break;
            case DISPOPA_READ_VRAM:
                {   
                    uint32_t x_start = curOpa.pars[0];
                    uint32_t y_start = curOpa.pars[1]; 
                    uint32_t x_end = curOpa.pars[2]; 
                    uint32_t y_end = curOpa.pars[3]; 
                    uint8_t *buf = (uint8_t *)curOpa.pars[4];
                    bool *fin = (bool *)curOpa.pars[5];
                    portDispReadBackVRAM(x_start, y_start, x_end, y_end, buf);
                    
                    vPortFree(curOpa.pars);
                    *fin = true;
                }
                break;
        default:
            break;
        }
    }
}

void DisplayInit()
{
    DisplayOpaQueue = xQueueCreate(32, sizeof(DisplayOpaQueue_t));
    portDispDeviceInit();
}
