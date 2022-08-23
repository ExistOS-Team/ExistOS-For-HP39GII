
#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kcasporing_gl.h"
#include "sys_llapi.h"

#include "porting.h"

#include "keyboard_gii39.h"

extern const unsigned char VGA_Ascii_5x8[];
extern const unsigned char VGA_Ascii_6x12[];
extern const unsigned char VGA_Ascii_8x16[];

// extern "C" {
char *virtual_screen;

char *scale_vir_screen;

#define X_OFFSET    (0)
#define Y_OFFSET    (-10)
#define X_SCALE     (0.68)
#define Y_SCALE     (0.55 + 0.1)

#define STRING_Y_SCALE  (1.0)
#define STRING_X_SCALE  (1.0)

#define SCALE_ENABLE    (0)

static int console_x = 0;
static int console_y = 0;

static int cursor_x = 0;
static int cursor_y = 0;

static bool cursor_enable = false;

static bool concur_reverse = false;

static bool ImmediateRefrush = false;

void vGL_FlushVScreen()
{

#if SCALE_ENABLE

    float xsrc = 0, ysrc = 0;
    int xdst = 0, ydst = 0; 

    while (ydst < VIR_LCD_PIX_H) {
        while (xdst < VIR_LCD_PIX_W) {
            scale_vir_screen[(int)(xdst ) + (int)(ydst ) * VIR_LCD_PIX_W] = virtual_screen[  ((int)xsrc + (X_OFFSET)) + ((int)ysrc + (Y_OFFSET)) * VIR_LCD_PIX_W  ];
            xsrc += X_SCALE;
            
            xdst++;    
        }
        xdst = 0;
        xsrc = 0;

        ydst++;
        ysrc += Y_SCALE;
    }
    ll_disp_put_area(scale_vir_screen, 0, 0, VIR_LCD_PIX_W - 1, VIR_LCD_PIX_H - 1);
#else
    ll_disp_put_area(virtual_screen, 0, 1, VIR_LCD_PIX_W - 1, VIR_LCD_PIX_H - 1);
#endif
}  

void vGL_SetPoint(unsigned int x, unsigned int y, int c)
{
    if ((x >= VIR_LCD_PIX_W)) {
        x = VIR_LCD_PIX_W - 1;
    }
    if ((y >= VIR_LCD_PIX_H)) {
        y = VIR_LCD_PIX_H - 1;
    }
    virtual_screen[x + y * VIR_LCD_PIX_W] = c;
} 

int vGL_GetPoint(unsigned int x,unsigned int y)
{
    if ((x >= VIR_LCD_PIX_W)) {
        x = VIR_LCD_PIX_W - 1;
    }
    if ((y >= VIR_LCD_PIX_H)) {
        y = VIR_LCD_PIX_H - 1;
    }
    return virtual_screen[x + y * VIR_LCD_PIX_W];
}

void vGL_putChar(int x0, int y0, char ch, int fg, int bg, int fontSize) {
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
            if (((x0 + x) < VIR_LCD_PIX_W) && ((y0 + y) < VIR_LCD_PIX_H))
                if ((*pCh << x) & 0x80U) {
                    virtual_screen[(x0 + x) + VIR_LCD_PIX_W * (y0 + y)] = fg;
                } else {
                    virtual_screen[(x0 + x) + VIR_LCD_PIX_W * (y0 + y)] = bg;
                }
            x++;
        }
        x = 0;
        y++;
        pCh++;
    }

}

void vGL_putString(int x0, int y0, char *s, int fg, int bg, int fontSize) {
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
            vGL_putChar((x0 * STRING_X_SCALE) + x, (y0 * STRING_Y_SCALE) + y, *s, fg, bg, fontSize);
            s++;
            x += font_w;
            if (x > VIR_LCD_PIX_W) {
                x = 0;
                y += font_h;
                if (y > VIR_LCD_PIX_H) {
                    break;
                }
            }
        }
    }
    
    ImmediateRefrush = true;
}

void vGL_clearArea(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1) {
    if ((x0 >= VIR_LCD_PIX_W)) {
        x0 = VIR_LCD_PIX_W - 1;
    }
    if ((y0 >= VIR_LCD_PIX_H)) {
        y0 = VIR_LCD_PIX_H - 1;
    }
    if ((x1 >= VIR_LCD_PIX_W)) {
        x1 = VIR_LCD_PIX_W - 1;
    }
    if ((y1 >= VIR_LCD_PIX_H)) {
        y1 = VIR_LCD_PIX_H - 1;
    }

    for (int y = y0; y < y1; y++)
        for (int x = x0; x < x1; x++) {
            {
                virtual_screen[x + y * VIR_LCD_PIX_W] = COLOR_WHITE;
            }
        }

    
    ImmediateRefrush = true;
}

void vGL_setArea(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1, unsigned int color) 
{
    if ((x0 >= VIR_LCD_PIX_W)) {
        x0 = VIR_LCD_PIX_W - 1;
    }
    if ((y0 >= VIR_LCD_PIX_H)) {
        y0 = VIR_LCD_PIX_H - 1;
    }
    if ((x1 >= VIR_LCD_PIX_W)) {
        x1 = VIR_LCD_PIX_W - 1;
    }
    if ((y1 >= VIR_LCD_PIX_H)) {
        y1 = VIR_LCD_PIX_H - 1;
    }

    for (int y = y0; y < y1; y++)
        for (int x = x0; x < x1; x++) {
            {
                virtual_screen[x + y * VIR_LCD_PIX_W] = color;
            }
        }

    ImmediateRefrush = true;
}

void vGL_ConsLocate(int x, int y)
{
    console_x = x;
    console_y = y;
}

void vGL_ConsOut(char *s, bool rev)
{
    if(rev)
        vGL_putString(console_x * 6, console_y * 12, (char *)s, COLOR_WHITE, COLOR_BLACK, 12);
    else
        vGL_putString(console_x * 6, console_y * 12, (char *)s, COLOR_BLACK, COLOR_WHITE, 12);
}

void vGL_reverseArea(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1) {
    if ((x0 >= VIR_LCD_PIX_W)) {
        x0 = VIR_LCD_PIX_W - 1;
    }
    if ((y0 >= VIR_LCD_PIX_H)) {
        y0 = VIR_LCD_PIX_H - 1;
    }
    if ((x1 >= VIR_LCD_PIX_W)) {
        x1 = VIR_LCD_PIX_W - 1;
    }
    if ((y1 >= VIR_LCD_PIX_H)) {
        y1 = VIR_LCD_PIX_H - 1;
    }

    for (int y = y0; y < y1; y++)
        for (int x = x0; x < x1; x++) {
            {
                virtual_screen[x + y * VIR_LCD_PIX_W] = ~virtual_screen[x + y * VIR_LCD_PIX_W];
            }
        }

    ImmediateRefrush = true;
    //vGL_FlushVScreen();
}

static void vGL_flushTask(void *arg)
{
    unsigned int _delay;
    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(50));
        if(ImmediateRefrush){
            vGL_FlushVScreen(); 
            ImmediateRefrush = false;
            _delay = 0;
        }else{
            _delay++;
            if(_delay > 5){
                _delay = 0;
                vGL_FlushVScreen(); 
            }
        }
    } 
}

static void vGL_concur_reverse()
{
    concur_reverse = !concur_reverse;
    vGL_reverseArea((cursor_x + 1) * 6, (cursor_y + 1) * 12, (cursor_x + 1) * 6 + 1, (cursor_y + 1) * 12 + 12);
}

void vGL_locateConcur(int x, int y)
{
    if(concur_reverse)
    {
        //vGL_concur_reverse();
    }
    cursor_x = x;
    cursor_y = y;
}

void vGL_concurEnable(bool enable)
{
    
    if(!enable && (cursor_enable))
    {
        vGL_concur_reverse();
    }
    cursor_enable = enable;
}



static void vGL_consoleTask(void *arg)
{
    while(1)
    { 
        if(cursor_enable)
        {
            vGL_concur_reverse();
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

int vGL_Initialize() {
    virtual_screen = pvPortMalloc(VIR_LCD_PIX_W * VIR_LCD_PIX_W);
    if (!virtual_screen) {
        printf("Failed to alloca virtual screen memory!\n");
        return -1;
    }

    scale_vir_screen = pvPortMalloc(VIR_LCD_PIX_W * VIR_LCD_PIX_W);
    if (!scale_vir_screen) {
        printf("Failed to alloca virtual scale screen memory!\n");
        vPortFree(virtual_screen);
        return -1;
    }

    memset(virtual_screen, COLOR_WHITE, VIR_LCD_PIX_H * VIR_LCD_PIX_W);
    memset(scale_vir_screen, COLOR_WHITE, VIR_LCD_PIX_H * VIR_LCD_PIX_W);

    xTaskCreate(vGL_flushTask, "vGLRefTsk", 1024, NULL, configMAX_PRIORITIES - 3, NULL);
    xTaskCreate(vGL_consoleTask, "vGLConsoleTsk", 1024, NULL, configMAX_PRIORITIES - 3, NULL);

    //vGL_FlushVScreen();

    return 0;
}

extern volatile bool interrupted ;
extern volatile bool ctrl_c ;

bool vGL_chkEsc()
{
    uint32_t keys, key, kpress;
    keys = ll_vm_check_key();
    key = keys & 0xFFFF;
    kpress = keys >> 16;
    if((key == KEY_ON) && kpress)
    {
        ctrl_c = true;
        interrupted = true;
        return true;
    }
    return false;
} 

bool vGL_getkey(int *keyid)
{
    uint32_t keys, key, kpress;
    static uint32_t last_key;
    static uint32_t last_press;


    do{
        keys = ll_vm_check_key();
        key = keys & 0xFFFF;
        kpress = keys >> 16;

        
        vTaskDelay(5);

    }while((last_key == key) && (last_press == kpress));


/*
    if((last_key == key) && (last_press == kpress))
    {
        *keyid = -1;
        return false;
    }*/
    
    last_key = key;
    last_press = kpress;

    *keyid = key;
    return kpress;
}

//}
