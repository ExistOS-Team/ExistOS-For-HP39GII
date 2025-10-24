

#include "FreeRTOS.h"
#include "event_groups.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "../debug.h"
#include "display_up.h"
#include "font_ascii.h"

uint32_t g_lcd_contrast = 62;

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
    //DISPOPA_CIRCLE,
} DispOpa;

typedef struct DisplayOpaQueue_t {
    DispOpa opa;
    uint32_t parNum;
    uint32_t *pars;
} DisplayOpaQueue_t;

QueueHandle_t DisplayOpaQueue;

static uint32_t DispIndicatorBit = 0, DispBatteryBit = 0;
static uint32_t Last_DispIndicatorBit = 0, Last_DispBatteryBit = 0;

void DisplayClean(void) {
    DisplayOpaQueue_t opa;
    opa.opa = DISPOPA_CLEAN;
    opa.parNum = 0;
    xQueueSend(DisplayOpaQueue, &opa, portMAX_DELAY);
}

void DisplayReadArea(uint32_t x_start, uint32_t y_start, uint32_t x_end, uint32_t y_end, uint8_t *buf, bool *fin) {
    *fin = false;
    if ((x_end - x_start + 1) * (y_end - y_start + 1) > 4096) {
        *fin = true;
        return;
    }
    DisplayOpaQueue_t opa;
    uint32_t *pars = pvPortMalloc(6 * sizeof(uint32_t));
    if (!pars)
        return;
    pars[0] = x_start;
    pars[1] = y_start;
    pars[2] = x_end;
    pars[3] = y_end;
    pars[4] = (uint32_t)buf;
    pars[5] = (uint32_t)fin;
    opa.opa = DISPOPA_READ_VRAM;
    opa.parNum = 5;
    opa.pars = pars;
    xQueueSend(DisplayOpaQueue, &opa, portMAX_DELAY);
}

void DisplayFlushArea(uint32_t x_start, uint32_t y_start, uint32_t x_end, uint32_t y_end, uint8_t *buf, bool block) {
    DisplayOpaQueue_t opa;
    uint32_t *pars = pvPortMalloc(6 * sizeof(uint32_t));
    if (!pars)
        return;

    volatile bool fin = false;

    pars[0] = x_start;
    pars[1] = y_start;
    pars[2] = x_end;
    pars[3] = y_end;
    pars[4] = (uint32_t)buf;
    pars[5] = (uint32_t)&fin;

    opa.opa = DISPOPA_FLUSH_AREA;
    opa.parNum = 6;
    opa.pars = pars;
    xQueueSend(DisplayOpaQueue, &opa, portMAX_DELAY);
    if (block) {
        while (fin == false) {
            vTaskDelay(2);
        }
    }
}

void DisplayPutChar(uint32_t x, uint32_t y, char c, uint8_t fg, uint8_t bg, uint8_t fontSize) {
    DisplayOpaQueue_t opa;
    uint32_t *pars = pvPortMalloc(6 * sizeof(uint32_t));
    if (!pars)
        return;
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

bool DisplayPutStr(uint32_t x, uint32_t y, char *s, uint8_t fg, uint8_t bg, uint8_t fontSize) {
    DisplayOpaQueue_t opa;
    uint32_t *pars = pvPortMalloc(6 * sizeof(uint32_t));
    char *str = pvPortMalloc(strlen(s));
    strcpy(str, s);
    if (!pars)
        return false;

    pars[0] = x;
    pars[1] = y;
    pars[2] = (uint32_t)str;
    pars[3] = fg;
    pars[4] = bg;
    pars[5] = fontSize;

    opa.opa = DISPOPA_PUT_STR;
    opa.parNum = 6;
    opa.pars = pars;
    xQueueSend(DisplayOpaQueue, &opa, portMAX_DELAY);

    return true;
}

void DisplayHLine(uint32_t x0, uint32_t x1, uint32_t y, uint8_t c) {
    DisplayOpaQueue_t opa;
    uint32_t *pars = pvPortMalloc(4 * sizeof(uint32_t));
    if (!pars)
        return;
    pars[0] = x0;
    pars[1] = x1;
    pars[2] = y;
    pars[3] = c;
    opa.opa = DISPOPA_HLINE;
    opa.parNum = 4;
    opa.pars = pars;
    xQueueSend(DisplayOpaQueue, &opa, portMAX_DELAY);
}

void DisplayVLine(uint32_t y0, uint32_t y1, uint32_t x, uint8_t c) {
    DisplayOpaQueue_t opa;
    uint32_t *pars = pvPortMalloc(4 * sizeof(uint32_t));
    if (!pars)
        return;
    pars[0] = y0;
    pars[1] = y1;
    pars[2] = x;
    pars[3] = c;
    opa.opa = DISPOPA_VLINE;
    opa.parNum = 4;
    opa.pars = pars;
    xQueueSend(DisplayOpaQueue, &opa, portMAX_DELAY);
}

void DisplayBoxBlock(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint8_t c) {
    DisplayOpaQueue_t opa;
    bool fin = false;
    uint32_t *pars = pvPortMalloc(6 * sizeof(uint32_t));
    if (!pars)
        return;
    pars[0] = x0;
    pars[1] = y0;
    pars[2] = x1;
    pars[3] = y1;
    pars[4] = c;
    pars[5] = (uint32_t)&fin;
    opa.opa = DISPOPA_BOX;
    opa.parNum = 6;
    opa.pars = pars;
    xQueueSend(DisplayOpaQueue, &opa, portMAX_DELAY);
    while (fin == false)
        ;
}

void DisplayBox(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint8_t c) {
    DisplayOpaQueue_t opa;
    uint32_t *pars = pvPortMalloc(6 * sizeof(uint32_t));
    if (!pars)
        return;
    pars[0] = x0;
    pars[1] = y0;
    pars[2] = x1;
    pars[3] = y1;
    pars[4] = c;
    pars[5] = 0;
    opa.opa = DISPOPA_BOX;
    opa.parNum = 6;
    opa.pars = pars;
    xQueueSend(DisplayOpaQueue, &opa, portMAX_DELAY);
}

void DisplayFillBoxBlock(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint8_t c) {
    DisplayOpaQueue_t opa;
    bool fin = false;
    uint32_t *pars = pvPortMalloc(6 * sizeof(uint32_t));
    if (!pars)
        return;
    pars[0] = x0;
    pars[1] = y0;
    pars[2] = x1;
    pars[3] = y1;
    pars[4] = c;
    pars[5] = (uint32_t)&fin;
    opa.opa = DISPOPA_FILL_BOX;
    opa.parNum = 6;
    opa.pars = pars;
    xQueueSend(DisplayOpaQueue, &opa, portMAX_DELAY);
    while (fin == false)
        ;
}

void DisplayFillBox(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint8_t c) {
    DisplayOpaQueue_t opa;
    uint32_t *pars = pvPortMalloc(6 * sizeof(uint32_t));
    if (!pars)
        return;
    pars[0] = x0;
    pars[1] = y0;
    pars[2] = x1;
    pars[3] = y1;
    pars[4] = c;
    pars[5] = 0;
    opa.opa = DISPOPA_FILL_BOX;
    opa.parNum = 6;
    opa.pars = pars;
    xQueueSend(DisplayOpaQueue, &opa, portMAX_DELAY);
}

/*
void DisplayCircle(uint32_t x0, uint32_t y0, uint32_t r, uint8_t c, bool isFill) {
    DisplayOpaQueue_t opa;
    uint32_t *pars = pvPortMalloc(6 * sizeof(uint32_t));
    if (!pars)
        return;
    pars[0] = x0;
    pars[1] = y0;
    pars[2] = r;
    pars[3] = c;
    pars[4] = isFill;
    pars[5] = 0;
    opa.opa = DISPOPA_CIRCLE;
    opa.parNum = 5;
    opa.pars = pars;
    xQueueSend(DisplayOpaQueue, &opa, portMAX_DELAY);
}
*/
/*
bool DisplayOperatesFin() {
    uint32_t len = uxQueueGetQueueNumber((QueueHandle_t)&DisplayOpaQueue);
    // printf("Qlen:%d\n",len);
    return len == 0;
}*/

void DisplaySetIndicate(int Indicate, int batInd) {
    DispIndicatorBit = Indicate;
    DispBatteryBit = batInd;
    /*
        return;
        DisplayOpaQueue_t opa;
        uint32_t *pars = pvPortMalloc(2 * sizeof(uint32_t));
        if(!pars)return;
        pars[0] = Indicate;
        pars[1] = batInd;
        opa.opa = DISPOPA_SET_INDICATE;
        opa.parNum = 2;
        opa.pars = pars;
        xQueueSend(DisplayOpaQueue, &opa, portMAX_DELAY);*/
}

static void innerDispHLine(uint32_t x0, uint32_t x1, uint32_t y, uint8_t c) {
    if (x1 < x0) {
        return;
    }
    uint8_t *buf = pvPortMalloc(x1 - x0 + 1);
    memset(buf, c, x1 - x0 + 1);
    portDispFlushAreaBuf(x0, y, x1, y, buf);
    vPortFree(buf);
}

static void innerDispVLine(uint32_t y0, uint32_t y1, uint32_t x, uint8_t c) {
    uint8_t *buf = pvPortMalloc(y1 - y0 + 1);
    memset(buf, c, y1 - y0 + 1);
    portDispFlushAreaBuf(x, y0, x, y1, buf);
    vPortFree(buf);
}

/*
static void innerDispPoint(uint32_t x0, uint32_t y0, uint8_t c) {
    uint8_t *buf = pvPortMalloc(3);
    memset(buf, c, 3);
    portDispFlushAreaBuf(x0 - 1, y0, x0 + 1, y0, buf);
    vPortFree(buf);
}
*/

void Display_InterfaceInit() {
    portDispInterfaceInit();
}
/*
volatile static bool led_refresh = false;

void vTaskDispLedRefresh() {
    for (;;) {
        if ((Last_DispBatteryBit != DispBatteryBit) || (Last_DispIndicatorBit != DispIndicatorBit)) {
            led_refresh = true;
            portDispSetIndicate(DispIndicatorBit, DispBatteryBit);
        }
        Last_DispBatteryBit = DispBatteryBit;
        Last_DispIndicatorBit = DispIndicatorBit;
        led_refresh = false;
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}*/

void DisplayTask() {
    DisplayOpaQueue_t curOpa;

    // xTaskCreate(vTaskDispLedRefresh, "disp led", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 3, NULL);

    for (;;) {
        if ((Last_DispBatteryBit != DispBatteryBit) || (Last_DispIndicatorBit != DispIndicatorBit)) {
            portDispSetIndicate(DispIndicatorBit, DispBatteryBit);
        }
        Last_DispBatteryBit = DispBatteryBit;
        Last_DispIndicatorBit = DispIndicatorBit;

        if (xQueueReceive(DisplayOpaQueue, &curOpa, pdMS_TO_TICKS(50)) == pdTRUE) {
            // INFO("g:%d\n", curOpa.opa);
            // vTaskDelay(pdMS_TO_TICKS(10));
            // vTaskSuspendAll();

            switch (curOpa.opa) {
            case DISPOPA_CLEAN:
                portDispClean();
                break;
            case DISPOPA_FLUSH_AREA: {
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
            case DISPOPA_PUT_CHAR: {
                if (curOpa.parNum != 6) {
                    break;
                }
                uint32_t x = curOpa.pars[0];
                uint32_t y = curOpa.pars[1];
                char c = curOpa.pars[2];
                uint8_t fg = curOpa.pars[3];
                uint8_t bg = curOpa.pars[4];
                uint8_t fontSize = curOpa.pars[5];

                uint32_t n = c - ' ';
                uint8_t *p = &Ascii1608[fontSize * n];
                uint8_t *buf = pvPortMalloc(8 * fontSize);
                for (int y = 0; y < fontSize; y++) {
                    for (int x = 0; x < 8; x++) {
                        buf[8 * y + x] = ((p[y] >> (7 - x)) & 1) ? fg : bg;
                    }
                }
                portDispFlushAreaBuf(x, y, x + (8 - 1), y + (fontSize - 1), buf);
                vPortFree(buf);
                vPortFree(curOpa.pars);
            }

            break;
            case DISPOPA_PUT_STR: {
                if (curOpa.parNum != 6) {
                    break;
                }
                uint32_t x = curOpa.pars[0];
                uint32_t y = curOpa.pars[1];
                char *s = (char *)curOpa.pars[2];
                uint8_t fg = curOpa.pars[3];
                uint8_t bg = curOpa.pars[4];
                uint8_t fontSize = curOpa.pars[5];

                uint32_t str_len = strlen(s);
                uint8_t *font;
                uint32_t fontWidth;
                // if(fontSize == 16)
                {
                    font = Ascii1608;
                    fontWidth = 8;
                }
                // else
                {
                    // font = Ascii1206;
                    // fontWidth = 7;
                }

                uint8_t *buf = pvPortMalloc(8 * str_len * fontSize);
                uint32_t x0 = 0;
                for (int n = 0; n < str_len; n++) {
                    char c = s[n];
                    uint32_t ch = c - ' ';
                    uint8_t *p = &font[fontSize * ch];
                    for (int y_ = 0; y_ < fontSize; y_++) {
                        for (int x_ = x0; x_ < x0 + 8; x_++) {
                            buf[fontWidth * str_len * y_ + x_] = ((p[y_] >> (7 - (x_ - x0))) & 1) ? fg : bg;
                        }
                    }
                    x0 += fontWidth;
                }

                portDispFlushAreaBuf(x, y, x + fontWidth * str_len - 1, y + (fontSize - 1), buf);

                vPortFree(buf);
                vPortFree(curOpa.pars);
                vPortFree(s);
            } break;
            case DISPOPA_HLINE: {
                uint32_t x0 = curOpa.pars[0];
                uint32_t x1 = curOpa.pars[1];
                uint32_t y = curOpa.pars[2];
                uint8_t c = curOpa.pars[3];
                innerDispHLine(x0, x1, y, c);
                vPortFree(curOpa.pars);
            } break;
            case DISPOPA_VLINE: {
                uint32_t y0 = curOpa.pars[0];
                uint32_t y1 = curOpa.pars[1];
                uint32_t x = curOpa.pars[2];
                uint8_t c = curOpa.pars[3];
                innerDispVLine(y0, y1, x, c);
                vPortFree(curOpa.pars);
            } break;
            case DISPOPA_BOX: {
                uint32_t x0 = curOpa.pars[0];
                uint32_t y0 = curOpa.pars[1];
                uint32_t x1 = curOpa.pars[2];
                uint32_t y1 = curOpa.pars[3];
                uint8_t c = curOpa.pars[4];

                innerDispHLine(x0, x1, y0, c);
                innerDispHLine(x0, x1, y1, c);
                innerDispVLine(y0, y1, x0, c);
                innerDispVLine(y0, y1, x1, c);

                if (curOpa.pars[5] != 0) {
                    bool *fin = (bool *)curOpa.pars[5];
                    *fin = true;
                }
                vPortFree(curOpa.pars);

            } break;
            case DISPOPA_FILL_BOX: {
                uint32_t x0 = curOpa.pars[0];
                uint32_t y0 = curOpa.pars[1];
                uint32_t x1 = curOpa.pars[2];
                uint32_t y1 = curOpa.pars[3];
                uint8_t c = curOpa.pars[4];
                for (int y = y0; y <= y1; y++) {
                    innerDispHLine(x0, x1, y, c);
                }

                if (curOpa.pars[5] != 0) {
                    bool *fin = (bool *)curOpa.pars[5];
                    *fin = true;
                }
                vPortFree(curOpa.pars);
            } break;
            case DISPOPA_SET_INDICATE: {
                /*
                int indicate = curOpa.pars[0];
                int batIndicate = curOpa.pars[1];
                */
                // portDispSetIndicate(indicate, batIndicate);
                vPortFree(curOpa.pars);
            } break;
            case DISPOPA_READ_VRAM: {
                uint32_t x_start = curOpa.pars[0];
                uint32_t y_start = curOpa.pars[1];
                uint32_t x_end = curOpa.pars[2];
                uint32_t y_end = curOpa.pars[3];
                uint8_t *buf = (uint8_t *)curOpa.pars[4];
                bool *fin = (bool *)curOpa.pars[5];
                portDispReadBackVRAM(x_start, y_start, x_end, y_end, buf);

                vPortFree(curOpa.pars);
                *fin = true;
            } break;
            /*
            case DISPOPA_CIRCLE: {
                uint32_t x0 = curOpa.pars[0];
                uint32_t y0 = curOpa.pars[1];
                uint32_t r = curOpa.pars[2];
                uint32_t c = curOpa.pars[3];
                bool isFill = curOpa.pars[4];

                int x1, y1;
                int j;
                int pow_r;
                int y_last;

                for (int rr = r; rr >= 0; rr--) {
                    pow_r = rr * rr;
                    for (x1 = x0 - rr, y_last = y0; x1 <= x0; x1++) {
                        y1 = y0 + __sqrt(pow_r - (x1 - x0) * (x1 - x0));
                        if (y1 - y_last > 1) {

                            for (j = y_last; j < (y1 + y_last) / 2; j++) { // add pixel from y_last to harf
                                innerDispPoint(x1 - 1, j, c);
                                innerDispPoint((x0 << 1) - x1 + 1, j, c);
                                innerDispPoint((x0 << 1) - x1 + 1, (y0 << 1) - j, c);
                                innerDispPoint(x1 - 1, (y0 << 1) - j, c);
                            }
                            for (j = (y1 + y_last) / 2; j < y1; j++) { // add pixel from hart to y_now
                                innerDispPoint(x1, j, c);
                                innerDispPoint((x0 << 1) - x1, j, c);
                                innerDispPoint((x0 << 1) - x1, (y0 << 1) - j, c);
                                innerDispPoint(x1, (y0 << 1) - j, c);
                            }
                        }
                        innerDispPoint(x1, y1, c);
                        innerDispPoint((x0 << 1) - x1, y1, c);
                        innerDispPoint((x0 << 1) - x1, (y0 << 1) - y1, c);
                        innerDispPoint(x1, (y0 << 1) - y1, c);
                        y_last = y1;
                    }
                    if (!isFill)
                        break;
                }

                vPortFree(curOpa.pars);
            } break;
            */
            default:
                break;
            }

            // xTaskResumeAll();
        }
    }
}

void DisplayInit() {
    DisplayOpaQueue = xQueueCreate(4, sizeof(DisplayOpaQueue_t));
    portDispDeviceInit();
}

/*
float __sqrt(int x) {
    x = x << 8; 
    uint32_t mask = 0x800;
    uint32_t root = 0;
    uint32_t trial;
    // uint16_t i, f;
    do {
        trial = root + mask;
        if (trial * trial <= x)
            root = trial;
        mask = mask >> 1;
    } while (mask);

    // i = root >> 4;
    // f = (root & 0xf) << 4;
    return (root >> 4);
}
*/