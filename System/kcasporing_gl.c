
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
extern const unsigned char orp_Ascii_6x12[];
extern const unsigned char VGA_Ascii_8x16[];
extern const unsigned char VGA_Ascii_7x14[];

// extern "C" {
char *virtual_screen=0;

char *scale_vir_screen=0;

char * screen_1bpp =(char *)0x02000000;

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
extern bool khicasRunning;
int khicas_1bpp=1; // assumes W is a multiple of 8

void vGL_FlushVScreen()
{
  if (khicas_1bpp){
    char *src=screen_1bpp;
    for (int r=0;r<VIR_LCD_PIX_H;++r){
      char tab[VIR_LCD_PIX_W];
      char *dest=tab,*end=dest+VIR_LCD_PIX_W;
      for (;dest<end;dest+=8,++src){
        char cur=*src;
        if (cur){
          dest[0]=(cur&1)?0xff:0; 
          dest[1]=(cur&2)?0xff:0; 
          dest[2]=(cur&4)?0xff:0; 
          dest[3]=(cur&8)?0xff:0; 
          dest[4]=(cur&16)?0xff:0; 
          dest[5]=(cur&32)?0xff:0; 
          dest[6]=(cur&64)?0xff:0; 
          dest[7]=(cur&128)?0xff:0; 
        }
        else {
          *((unsigned *) dest)=0;
          *((unsigned *) &dest[4])=0;
        }
      }
      ll_disp_put_area(tab, 0, r, VIR_LCD_PIX_W - 1, r);      
    }
    return;
  }
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

void vGL_set_pixel(unsigned x,unsigned y,int c){
  //if (y==93) printf("glsetp:%d,%d,%d\n",x,y,c);
  if (x>=VIR_LCD_PIX_W || y>=VIR_LCD_PIX_H)
    return;
  if (khicas_1bpp){
    char shift = 1<<(x&7);
    int pos=(x+VIR_LCD_PIX_W*y)>>3;
    if (c)
      screen_1bpp[pos] |= shift;
    else
      screen_1bpp[pos] &= ~shift;
  }
  else
    virtual_screen[x + y * VIR_LCD_PIX_W] = c;
}

void vGL_SetPoint(unsigned int x, unsigned int y, int c)
{
    if ((x >= VIR_LCD_PIX_W)) {
        x = VIR_LCD_PIX_W - 1;
    }
    if ((y >= VIR_LCD_PIX_H)) {
        y = VIR_LCD_PIX_H - 1;
    }
    vGL_set_pixel(x,y,c);
} 

int vGL_GetPoint(unsigned int x,unsigned int y)
{
    if ((x >= VIR_LCD_PIX_W)) {
        x = VIR_LCD_PIX_W - 1;
    }
    if ((y >= VIR_LCD_PIX_H)) {
        y = VIR_LCD_PIX_H - 1;
    }
    if (khicas_1bpp){
      char shift = 1<<(x&7);
      int pos=(x+VIR_LCD_PIX_W*y)>>3;
      return (screen_1bpp[pos] & shift)?0xff:0;
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
        pCh = orp_Ascii_6x12 + (ch - ' ') * font_h;
        break;

    case 16:
        font_w = 8;
        font_h = 16;
        pCh = VGA_Ascii_8x16 + (ch - ' ') * font_h;
        break;

    case 14:
        font_w = 7;
        font_h = 14;
        pCh = VGA_Ascii_7x14 + (ch - ' ') * font_h;
        break;

    default:
        return;
    }

    while (y < font_h) {
        while (x < font_w) {
            if (((x0 + x) < VIR_LCD_PIX_W) && ((y0 + y) < VIR_LCD_PIX_H))
              //virtual_screen[(x0 + x) + VIR_LCD_PIX_W * (y0 + y)] = ((*pCh << x) & 0x80U)?fg:bg;
              vGL_set_pixel(x0+x,y0+y, ((*pCh << x) & 0x80U)?fg:bg);
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
        case 14:
            font_w = 7;
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
            if (x > VIR_LCD_PIX_W) { break;
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

    printf("clra:%d,%d,%d,%d\n", x0, x1, y0, y1);
    for (int y = y0; y < y1; y++){
      for (int x = x0; x < x1; x++) {
        vGL_set_pixel(x,y,COLOR_WHITE); // virtual_screen[x + y * VIR_LCD_PIX_W] = COLOR_WHITE;
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
              vGL_set_pixel(x,y,color); // virtual_screen[x + y * VIR_LCD_PIX_W] = color;
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
    if (khicas_1bpp){
      for (int y = y0; y < y1; y++){
        for (int x = x0; x < x1; x++) {
          char shift = 1<<(x&7);
          int pos=(x+VIR_LCD_PIX_W*y)>>3;
          screen_1bpp[pos] ^= shift; 
        }
      }
    }
    else {
      for (int y = y0; y < y1; y++){
        for (int x = x0; x < x1; x++) {
          virtual_screen[x + y * VIR_LCD_PIX_W] = ~virtual_screen[x + y * VIR_LCD_PIX_W];
        }
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

        if(!khicasRunning)
        {
            vTaskDelete(NULL);
        }
    } 
}

extern int shell_fontw,shell_fonth;

static void vGL_concur_reverse()
{
    concur_reverse = !concur_reverse;
    vGL_reverseArea((cursor_x + 1) * shell_fontw, (cursor_y + 1) * shell_fonth, (cursor_x + 1) * shell_fontw + 1, (cursor_y + 1) * shell_fonth + shell_fonth);
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
        if(!khicasRunning)
        {
            vTaskDelete(NULL);
        }
    }
}

int vGL_Initialize() {
  if (!screen_1bpp)
    screen_1bpp=pvPortMalloc(VIR_LCD_PIX_H * VIR_LCD_PIX_W/8);
  if (!screen_1bpp) 
    return -1;
  memset(screen_1bpp, COLOR_WHITE, VIR_LCD_PIX_H * VIR_LCD_PIX_W / 8);

  if (!virtual_screen)
    virtual_screen = pvPortMalloc(VIR_LCD_PIX_H * VIR_LCD_PIX_W);
  if (!virtual_screen) {
    vPortFree(screen_1bpp);
    printf("Failed to alloca virtual screen memory!\n");
    return -1;
  }
  if (!khicas_1bpp)
    memset(virtual_screen, COLOR_WHITE, VIR_LCD_PIX_H * VIR_LCD_PIX_W);

#if SCALE_ENABLE
  scale_vir_screen = pvPortMalloc(VIR_LCD_PIX_W * VIR_LCD_PIX_W);
  if (!scale_vir_screen) {
    printf("Failed to alloca virtual scale screen memory!\n");
    vPortFree(virtual_screen);
    vPortFree(screen_1bpp);
    virtual_screen=0;
    return -1;
  }
  memset(scale_vir_screen, COLOR_WHITE, VIR_LCD_PIX_H * VIR_LCD_PIX_W);
#endif


    xTaskCreate(vGL_flushTask, "vGLRefTsk", 512, NULL, configMAX_PRIORITIES - 3, NULL);
    xTaskCreate(vGL_consoleTask, "vGLConsoleTsk", 512, NULL, configMAX_PRIORITIES - 3, NULL);

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
        keys = ll_vm_check_key();
        key = keys & 0xFFFF;
        kpress = keys >> 16;
    last_key = key;
    last_press = kpress;
    do{
        keys = ll_vm_check_key();
        key = keys & 0xFFFF;
        kpress = keys >> 16;

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


// Created from bdf2c Version 3, (c) 2009, 2010 by Lutz Sammer
//	License AGPLv3: GNU Affero General Public License version 3
	/// @{ defines to have human readable font files
#define ________ 0x00
#define _______X 0x01
#define ______X_ 0x02
#define ______XX 0x03
#define _____X__ 0x04
#define _____X_X 0x05
#define _____XX_ 0x06
#define _____XXX 0x07
#define ____X___ 0x08
#define ____X__X 0x09
#define ____X_X_ 0x0A
#define ____X_XX 0x0B
#define ____XX__ 0x0C
#define ____XX_X 0x0D
#define ____XXX_ 0x0E
#define ____XXXX 0x0F
#define ___X____ 0x10
#define ___X___X 0x11
#define ___X__X_ 0x12
#define ___X__XX 0x13
#define ___X_X__ 0x14
#define ___X_X_X 0x15
#define ___X_XX_ 0x16
#define ___X_XXX 0x17
#define ___XX___ 0x18
#define ___XX__X 0x19
#define ___XX_X_ 0x1A
#define ___XX_XX 0x1B
#define ___XXX__ 0x1C
#define ___XXX_X 0x1D
#define ___XXXX_ 0x1E
#define ___XXXXX 0x1F
#define __X_____ 0x20
#define __X____X 0x21
#define __X___X_ 0x22
#define __X___XX 0x23
#define __X__X__ 0x24
#define __X__X_X 0x25
#define __X__XX_ 0x26
#define __X__XXX 0x27
#define __X_X___ 0x28
#define __X_X__X 0x29
#define __X_X_X_ 0x2A
#define __X_X_XX 0x2B
#define __X_XX__ 0x2C
#define __X_XX_X 0x2D
#define __X_XXX_ 0x2E
#define __X_XXXX 0x2F
#define __XX____ 0x30
#define __XX___X 0x31
#define __XX__X_ 0x32
#define __XX__XX 0x33
#define __XX_X__ 0x34
#define __XX_X_X 0x35
#define __XX_XX_ 0x36
#define __XX_XXX 0x37
#define __XXX___ 0x38
#define __XXX__X 0x39
#define __XXX_X_ 0x3A
#define __XXX_XX 0x3B
#define __XXXX__ 0x3C
#define __XXXX_X 0x3D
#define __XXXXX_ 0x3E
#define __XXXXXX 0x3F
#define _X______ 0x40
#define _X_____X 0x41
#define _X____X_ 0x42
#define _X____XX 0x43
#define _X___X__ 0x44
#define _X___X_X 0x45
#define _X___XX_ 0x46
#define _X___XXX 0x47
#define _X__X___ 0x48
#define _X__X__X 0x49
#define _X__X_X_ 0x4A
#define _X__X_XX 0x4B
#define _X__XX__ 0x4C
#define _X__XX_X 0x4D
#define _X__XXX_ 0x4E
#define _X__XXXX 0x4F
#define _X_X____ 0x50
#define _X_X___X 0x51
#define _X_X__X_ 0x52
#define _X_X__XX 0x53
#define _X_X_X__ 0x54
#define _X_X_X_X 0x55
#define _X_X_XX_ 0x56
#define _X_X_XXX 0x57
#define _X_XX___ 0x58
#define _X_XX__X 0x59
#define _X_XX_X_ 0x5A
#define _X_XX_XX 0x5B
#define _X_XXX__ 0x5C
#define _X_XXX_X 0x5D
#define _X_XXXX_ 0x5E
#define _X_XXXXX 0x5F
#define _XX_____ 0x60
#define _XX____X 0x61
#define _XX___X_ 0x62
#define _XX___XX 0x63
#define _XX__X__ 0x64
#define _XX__X_X 0x65
#define _XX__XX_ 0x66
#define _XX__XXX 0x67
#define _XX_X___ 0x68
#define _XX_X__X 0x69
#define _XX_X_X_ 0x6A
#define _XX_X_XX 0x6B
#define _XX_XX__ 0x6C
#define _XX_XX_X 0x6D
#define _XX_XXX_ 0x6E
#define _XX_XXXX 0x6F
#define _XXX____ 0x70
#define _XXX___X 0x71
#define _XXX__X_ 0x72
#define _XXX__XX 0x73
#define _XXX_X__ 0x74
#define _XXX_X_X 0x75
#define _XXX_XX_ 0x76
#define _XXX_XXX 0x77
#define _XXXX___ 0x78
#define _XXXX__X 0x79
#define _XXXX_X_ 0x7A
#define _XXXX_XX 0x7B
#define _XXXXX__ 0x7C
#define _XXXXX_X 0x7D
#define _XXXXXX_ 0x7E
#define _XXXXXXX 0x7F
#define X_______ 0x80
#define X______X 0x81
#define X_____X_ 0x82
#define X_____XX 0x83
#define X____X__ 0x84
#define X____X_X 0x85
#define X____XX_ 0x86
#define X____XXX 0x87
#define X___X___ 0x88
#define X___X__X 0x89
#define X___X_X_ 0x8A
#define X___X_XX 0x8B
#define X___XX__ 0x8C
#define X___XX_X 0x8D
#define X___XXX_ 0x8E
#define X___XXXX 0x8F
#define X__X____ 0x90
#define X__X___X 0x91
#define X__X__X_ 0x92
#define X__X__XX 0x93
#define X__X_X__ 0x94
#define X__X_X_X 0x95
#define X__X_XX_ 0x96
#define X__X_XXX 0x97
#define X__XX___ 0x98
#define X__XX__X 0x99
#define X__XX_X_ 0x9A
#define X__XX_XX 0x9B
#define X__XXX__ 0x9C
#define X__XXX_X 0x9D
#define X__XXXX_ 0x9E
#define X__XXXXX 0x9F
#define X_X_____ 0xA0
#define X_X____X 0xA1
#define X_X___X_ 0xA2
#define X_X___XX 0xA3
#define X_X__X__ 0xA4
#define X_X__X_X 0xA5
#define X_X__XX_ 0xA6
#define X_X__XXX 0xA7
#define X_X_X___ 0xA8
#define X_X_X__X 0xA9
#define X_X_X_X_ 0xAA
#define X_X_X_XX 0xAB
#define X_X_XX__ 0xAC
#define X_X_XX_X 0xAD
#define X_X_XXX_ 0xAE
#define X_X_XXXX 0xAF
#define X_XX____ 0xB0
#define X_XX___X 0xB1
#define X_XX__X_ 0xB2
#define X_XX__XX 0xB3
#define X_XX_X__ 0xB4
#define X_XX_X_X 0xB5
#define X_XX_XX_ 0xB6
#define X_XX_XXX 0xB7
#define X_XXX___ 0xB8
#define X_XXX__X 0xB9
#define X_XXX_X_ 0xBA
#define X_XXX_XX 0xBB
#define X_XXXX__ 0xBC
#define X_XXXX_X 0xBD
#define X_XXXXX_ 0xBE
#define X_XXXXXX 0xBF
#define XX______ 0xC0
#define XX_____X 0xC1
#define XX____X_ 0xC2
#define XX____XX 0xC3
#define XX___X__ 0xC4
#define XX___X_X 0xC5
#define XX___XX_ 0xC6
#define XX___XXX 0xC7
#define XX__X___ 0xC8
#define XX__X__X 0xC9
#define XX__X_X_ 0xCA
#define XX__X_XX 0xCB
#define XX__XX__ 0xCC
#define XX__XX_X 0xCD
#define XX__XXX_ 0xCE
#define XX__XXXX 0xCF
#define XX_X____ 0xD0
#define XX_X___X 0xD1
#define XX_X__X_ 0xD2
#define XX_X__XX 0xD3
#define XX_X_X__ 0xD4
#define XX_X_X_X 0xD5
#define XX_X_XX_ 0xD6
#define XX_X_XXX 0xD7
#define XX_XX___ 0xD8
#define XX_XX__X 0xD9
#define XX_XX_X_ 0xDA
#define XX_XX_XX 0xDB
#define XX_XXX__ 0xDC
#define XX_XXX_X 0xDD
#define XX_XXXX_ 0xDE
#define XX_XXXXX 0xDF
#define XXX_____ 0xE0
#define XXX____X 0xE1
#define XXX___X_ 0xE2
#define XXX___XX 0xE3
#define XXX__X__ 0xE4
#define XXX__X_X 0xE5
#define XXX__XX_ 0xE6
#define XXX__XXX 0xE7
#define XXX_X___ 0xE8
#define XXX_X__X 0xE9
#define XXX_X_X_ 0xEA
#define XXX_X_XX 0xEB
#define XXX_XX__ 0xEC
#define XXX_XX_X 0xED
#define XXX_XXX_ 0xEE
#define XXX_XXXX 0xEF
#define XXXX____ 0xF0
#define XXXX___X 0xF1
#define XXXX__X_ 0xF2
#define XXXX__XX 0xF3
#define XXXX_X__ 0xF4
#define XXXX_X_X 0xF5
#define XXXX_XX_ 0xF6
#define XXXX_XXX 0xF7
#define XXXXX___ 0xF8
#define XXXXX__X 0xF9
#define XXXXX_X_ 0xFA
#define XXXXX_XX 0xFB
#define XXXXXX__ 0xFC
#define XXXXXX_X 0xFD
#define XXXXXXX_ 0xFE
#define XXXXXXXX 0xFF
	/// @}


	/// character bitmap for each encoding
const unsigned char VGA_Ascii_7x14[] =   {           // ASCII
//  32 $20 'char32'
//	width 7, bbx 0, bby 0, bbw 1, bbh 1
	________,
	________,
	________,
	________,
	________,
	________,
	________,
	________,
	________,
	________,
	________,
	________,
	________,
	________,
//  33 $21 'char33'
//	width 7, bbx 2, bby 0, bbw 2, bbh 10
	__XX____,
	__XX____,
	__XX____,
	__XX____,
	__XX____,
	__XX____,
	________,
	________,
	__XX____,
	__XX____,
	________,
	________,
	________,
	________,
//  34 $22 'char34'
//	width 7, bbx 1, bby 6, bbw 5, bbh 4
	_XX_XX__,
	_XX_XX__,
	_XX_XX__,
	_XX_XX__,
	________,
	________,
	________,
	________,
	________,
	________,
	________,
	________,
	________,
	________,
//  35 $23 'char35'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	_X__X___,
	_XX_X___,
	XXXXXX__,
	XXXXXX__,
	_X__X___,
	_X__X___,
	XXXXXX__,
	XXXXXX__,
	_X_XX___,
	_X__X___,
	________,
	________,
	________,
	________,
//  36 $24 'char36'
//	width 7, bbx 0, bby -1, bbw 6, bbh 12
	__XX____,
	__XX____,
	_XXXXX__,
	XX__XX__,
	XX______,
	_XXX____,
	__XXX___,
	____XX__,
	XX__XX__,
	XXXXX___,
	__XX____,
	__XX____,
	________,
	________,
//  37 $25 'char37'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	XX__XX__,
	XX__XX__,
	___XX___,
	___XX___,
	__XX____,
	__XX____,
	_XX_____,
	_XX_____,
	XX__XX__,
	XX__XX__,
	________,
	________,
	________,
	________,
//  38 $26 'char38'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	_XXX____,
	XX______,
	XX__X___,
	XX_XX___,
	_XXXXX__,
	XX_XX___,
	XX_XX___,
	XX_XX___,
	XX_XX___,
	_XX_XX__,
	________,
	________,
	________,
	________,
//  39 $27 'char39'
//	width 7, bbx 2, bby 6, bbw 2, bbh 4
	__XX____,
	__XX____,
	__XX____,
	__XX____,
	________,
	________,
	________,
	________,
	________,
	________,
	________,
	________,
	________,
	________,
//  40 $28 'char40'
//	width 7, bbx 1, bby -1, bbw 4, bbh 12
	___XX___,
	__XX____,
	__XX____,
	_XX_____,
	_XX_____,
	_XX_____,
	_XX_____,
	_XX_____,
	_XX_____,
	__XX____,
	__XX____,
	___XX___,
	________,
	________,
//  41 $29 'char41'
//	width 7, bbx 1, bby -1, bbw 4, bbh 12
	_XX_____,
	__XX____,
	__XX____,
	___XX___,
	___XX___,
	___XX___,
	___XX___,
	___XX___,
	___XX___,
	__XX____,
	__XX____,
	_XX_____,
	________,
	________,
//  42 $2a 'char42'
//	width 7, bbx 0, bby 4, bbw 6, bbh 6
	________,
	________,
	________,
	________,
	__XX____,
	__XX____,
	XXXXXX__,
	__XX____,
	_XXXX___,
	_X__X___,
	________,
	________,
	________,
	________,
//  43 $2b 'char43'
//	width 7, bbx 0, bby 1, bbw 6, bbh 7
	________,
	________,
	________,
	__XX____,
	__XX____,
	__XX____,
	XXXXXX__,
	__XX____,
	__XX____,
	__XX____,
	________,
	________,
	________,
	________,
//  44 $2c 'char44'
//	width 7, bbx 1, bby -2, bbw 3, bbh 4
	________,
	________,
	________,
	________,
	________,
	________,
	________,
	________,
	__XX____,
	__XX____,
	__XX____,
	_XX_____,
	________,
	________,
//  45 $2d 'char45'
//	width 7, bbx 0, bby 4, bbw 6, bbh 1
	________,
	________,
	________,
	________,
	________,
	________,
	XXXXXX__,
	________,
	________,
	________,
	________,
	________,
	________,
	________,
//  46 $2e 'char46'
//	width 7, bbx 2, bby 0, bbw 2, bbh 2
	________,
	________,
	________,
	________,
	________,
	________,
	________,
	________,
	__XX____,
	__XX____,
	________,
	________,
	________,
	________,
//  47 $2f 'char47'
//	width 7, bbx 0, bby -1, bbw 6, bbh 12
	____XX__,
	____XX__,
	___XX___,
	___XX___,
	___XX___,
	__XX____,
	__XX____,
	_XX_____,
	_XX_____,
	_XX_____,
	XX______,
	XX______,
	________,
	________,
//  48 $30 'char48'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	_XXXX___,
	XX__XX__,
	XX__XX__,
	XX_XXX__,
	XX_XXX__,
	XXX_XX__,
	XXX_XX__,
	XX__XX__,
	XX__XX__,
	_XXXX___,
	________,
	________,
	________,
	________,
//  49 $31 'char49'
//	width 7, bbx 0, bby 0, bbw 4, bbh 10
	__XX____,
	_XXX____,
	XXXX____,
	__XX____,
	__XX____,
	__XX____,
	__XX____,
	__XX____,
	__XX____,
	__XX____,
	________,
	________,
	________,
	________,
//  50 $32 'char50'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	_XXXX___,
	XX__XX__,
	XX__XX__,
	____XX__,
	___XX___,
	__XX____,
	_XX_____,
	XX______,
	XX______,
	XXXXXX__,
	________,
	________,
	________,
	________,
//  51 $33 'char51'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	_XXXX___,
	XX__XX__,
	XX__XX__,
	____XX__,
	__XXX___,
	____XX__,
	____XX__,
	XX__XX__,
	XX__XX__,
	_XXXX___,
	________,
	________,
	________,
	________,
//  52 $34 'char52'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	___XX___,
	__XXX___,
	__XXX___,
	_XXXX___,
	_X_XX___,
	XX_XX___,
	XX_XX___,
	XXXXXX__,
	___XX___,
	___XX___,
	________,
	________,
	________,
	________,
//  53 $35 'char53'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	XXXXXX__,
	XX______,
	XX______,
	XXXXX___,
	XX__XX__,
	____XX__,
	____XX__,
	XX__XX__,
	XX__XX__,
	_XXXX___,
	________,
	________,
	________,
	________,
//  54 $36 'char54'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	__XX____,
	__XX____,
	_XX_____,
	_XX_____,
	XXXXX___,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	_XXXX___,
	________,
	________,
	________,
	________,
//  55 $37 'char55'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	XXXXXX__,
	XX__XX__,
	XX__XX__,
	___XX___,
	___XX___,
	___XX___,
	__XX____,
	__XX____,
	__XX____,
	__XX____,
	________,
	________,
	________,
	________,
//  56 $38 'char56'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	_XXXX___,
	XX__XX__,
	XX__XX__,
	_X_XX___,
	__XX____,
	_XX_X___,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	_XXXX___,
	________,
	________,
	________,
	________,
//  57 $39 'char57'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	_XXXX___,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	_XXXXX__,
	___XX___,
	___XX___,
	__XX____,
	__XX____,
	________,
	________,
	________,
	________,
//  58 $3a 'char58'
//	width 7, bbx 2, bby 1, bbw 2, bbh 7
	________,
	________,
	__XX____,
	__XX____,
	________,
	________,
	________,
	__XX____,
	__XX____,
	________,
	________,
	________,
	________,
	________,
//  59 $3b 'char59'
//	width 7, bbx 1, bby -1, bbw 3, bbh 9
	________,
	________,
	________,
	________,
	__XX____,
	__XX____,
	________,
	________,
	________,
	__XX____,
	__XX____,
	__XX____,
	_XX_____,
	________,
//  60 $3c 'char60'
//	width 7, bbx 0, bby 1, bbw 5, bbh 7
	________,
	________,
	________,
	___XX___,
	__XX____,
	_XX_____,
	XX______,
	_XX_____,
	__XX____,
	___XX___,
	________,
	________,
	________,
	________,
//  61 $3d 'char61'
//	width 7, bbx 0, bby 2, bbw 6, bbh 5
	________,
	________,
	________,
	XXXXXX__,
	________,
	________,
	________,
	XXXXXX__,
	________,
	________,
	________,
	________,
	________,
	________,
//  62 $3e 'char62'
//	width 7, bbx 1, bby 1, bbw 5, bbh 7
	________,
	________,
	________,
	_XX_____,
	__XX____,
	___XX___,
	____XX__,
	___XX___,
	__XX____,
	_XX_____,
	________,
	________,
	________,
	________,
//  63 $3f 'char63'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	_XXXX___,
	XX__XX__,
	XX__XX__,
	____XX__,
	___XX___,
	__XX____,
	________,
	________,
	__XX____,
	__XX____,
	________,
	________,
	________,
	________,
//  64 $40 'char64'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	__XXX___,
	_XX_XX__,
	XX___X__,
	XX_XXX__,
	XX_X_X__,
	XX_X_X__,
	XX_XXX__,
	XX__XX__,
	_XX_____,
	__XXX___,
	________,
	________,
	________,
	________,
//  65 $41 'char65'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	__XX____,
	__XX____,
	_XXXX___,
	_X_XX___,
	_X__X___,
	XX__XX__,
	XXXXXX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	________,
	________,
	________,
	________,
//  66 $42 'char66'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	XXXX____,
	XX_XX___,
	XX_XX___,
	XX_XX___,
	XXXX____,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XXXXX___,
	________,
	________,
	________,
	________,
//  67 $43 'char67'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	_XXXX___,
	XX__XX__,
	XX__XX__,
	XX______,
	XX______,
	XX______,
	XX______,
	XX__XX__,
	XX__XX__,
	_XXXX___,
	________,
	________,
	________,
	________,
//  68 $44 'char68'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	XXXX____,
	XX_XX___,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX_XX___,
	XXXX____,
	________,
	________,
	________,
	________,
//  69 $45 'char69'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	XXXXXX__,
	XX______,
	XX______,
	XX______,
	XXXXX___,
	XX______,
	XX______,
	XX______,
	XX______,
	XXXXXX__,
	________,
	________,
	________,
	________,
//  70 $46 'char70'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	XXXXXX__,
	XX______,
	XX______,
	XX______,
	XXXXX___,
	XX______,
	XX______,
	XX______,
	XX______,
	XX______,
	________,
	________,
	________,
	________,
//  71 $47 'char71'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	_XXXX___,
	XX__XX__,
	XX__XX__,
	XX______,
	XX______,
	XX_XXX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	_XXX_X__,
	________,
	________,
	________,
	________,
//  72 $48 'char72'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XXXXXX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	________,
	________,
	________,
	________,
//  73 $49 'char73'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	XXXXXX__,
	__XX____,
	__XX____,
	__XX____,
	__XX____,
	__XX____,
	__XX____,
	__XX____,
	__XX____,
	XXXXXX__,
	________,
	________,
	________,
	________,
//  74 $4a 'char74'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	_XXXXX__,
	____XX__,
	____XX__,
	____XX__,
	____XX__,
	____XX__,
	____XX__,
	XX__XX__,
	XX__XX__,
	_XXXX___,
	________,
	________,
	________,
	________,
//  75 $4b 'char75'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	XX__XX__,
	XX__XX__,
	XX_XX___,
	XX_XX___,
	XXXX____,
	XX_XX___,
	XX_XX___,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	________,
	________,
	________,
	________,
//  76 $4c 'char76'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	XX______,
	XX______,
	XX______,
	XX______,
	XX______,
	XX______,
	XX______,
	XX______,
	XX______,
	XXXXXX__,
	________,
	________,
	________,
	________,
//  77 $4d 'char77'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	XX__XX__,
	XX__XX__,
	XXXXXX__,
	XXXXXX__,
	XX_X_X__,
	XX_X_X__,
	XX_X_X__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	________,
	________,
	________,
	________,
//  78 $4e 'char78'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	XX__XX__,
	XX__XX__,
	XXX_XX__,
	XXX_XX__,
	XXX_XX__,
	XX_XXX__,
	XX_XXX__,
	XX_XXX__,
	XX__XX__,
	XX__XX__,
	________,
	________,
	________,
	________,
//  79 $4f 'char79'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	_XXXX___,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	_XXXX___,
	________,
	________,
	________,
	________,
//  80 $50 'char80'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	XXXXX___,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XXXXX___,
	XX______,
	XX______,
	XX______,
	XX______,
	________,
	________,
	________,
	________,
//  81 $51 'char81'
//	width 7, bbx 0, bby -2, bbw 6, bbh 12
	_XXXX___,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX_XX___,
	_XXX____,
	___XXX__,
	____XX__,
	________,
	________,
//  82 $52 'char82'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	XXXXX___,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XXXXX___,
	XX_XX___,
	XX_XX___,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	________,
	________,
	________,
	________,
//  83 $53 'char83'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	_XXXX___,
	XX__XX__,
	XX__XX__,
	XX______,
	_XX_____,
	___XX___,
	____XX__,
	XX__XX__,
	XX__XX__,
	_XXXX___,
	________,
	________,
	________,
	________,
//  84 $54 'char84'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	XXXXXX__,
	__XX____,
	__XX____,
	__XX____,
	__XX____,
	__XX____,
	__XX____,
	__XX____,
	__XX____,
	__XX____,
	________,
	________,
	________,
	________,
//  85 $55 'char85'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	_XXXX___,
	________,
	________,
	________,
	________,
//  86 $56 'char86'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	_X__X___,
	_X__X___,
	_XXXX___,
	__XX____,
	__XX____,
	________,
	________,
	________,
	________,
//  87 $57 'char87'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX_X_X__,
	XX_X_X__,
	XX_X_X__,
	XX_X_X__,
	XX_X_X__,
	XXXXXX__,
	_XXXX___,
	________,
	________,
	________,
	________,
//  88 $58 'char88'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	XX__XX__,
	XX__XX__,
	_X__X___,
	_XXXX___,
	__XX____,
	__XX____,
	_XXXX___,
	_X__X___,
	XX__XX__,
	XX__XX__,
	________,
	________,
	________,
	________,
//  89 $59 'char89'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	_X__X___,
	_XXXX___,
	__XX____,
	__XX____,
	__XX____,
	__XX____,
	________,
	________,
	________,
	________,
//  90 $5a 'char90'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	XXXXXX__,
	____XX__,
	___XX___,
	___XX___,
	__XX____,
	__XX____,
	_XX_____,
	_XX_____,
	XX______,
	XXXXXX__,
	________,
	________,
	________,
	________,
//  91 $5b 'char91'
//	width 7, bbx 1, bby -1, bbw 4, bbh 12
	_XXXX___,
	_XX_____,
	_XX_____,
	_XX_____,
	_XX_____,
	_XX_____,
	_XX_____,
	_XX_____,
	_XX_____,
	_XX_____,
	_XX_____,
	_XXXX___,
	________,
	________,
//  92 $5c 'char92'
//	width 7, bbx 0, bby -1, bbw 6, bbh 12
	XX______,
	XX______,
	_XX_____,
	_XX_____,
	_XX_____,
	__XX____,
	__XX____,
	___XX___,
	___XX___,
	___XX___,
	____XX__,
	____XX__,
	________,
	________,
//  93 $5d 'char93'
//	width 7, bbx 1, bby -1, bbw 4, bbh 12
	_XXXX___,
	___XX___,
	___XX___,
	___XX___,
	___XX___,
	___XX___,
	___XX___,
	___XX___,
	___XX___,
	___XX___,
	___XX___,
	_XXXX___,
	________,
	________,
//  94 $5e 'char94'
//	width 7, bbx 0, bby 4, bbw 6, bbh 6
	__XX____,
	__XX____,
	_XXXX___,
	_XXXX___,
	XX__XX__,
	XX__XX__,
	________,
	________,
	________,
	________,
	________,
	________,
	________,
	________,
//  95 $5f 'char95'
//	width 7, bbx 0, bby -1, bbw 6, bbh 1
	________,
	________,
	________,
	________,
	________,
	________,
	________,
	________,
	XXXXXX__,
	________,
	________,
	________,
	________,
	________,
//  96 $60 'char96'
//	width 7, bbx 1, bby 6, bbw 4, bbh 4
	_XX_____,
	_XXX____,
	__XXX___,
	___XX___,
	________,
	________,
	________,
	________,
	________,
	________,
	________,
	________,
	________,
	________,
//  97 $61 'char97'
//	width 7, bbx 0, bby 0, bbw 6, bbh 7
	________,
	________,
	________,
	_XXXX___,
	XX__XX__,
	____XX__,
	_XXXXX__,
	XX__XX__,
	XX__XX__,
	_XXXXX__,
	________,
	________,
	________,
	________,
//  98 $62 'char98'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	XX______,
	XX______,
	XX______,
	XX_XX___,
	XXX_XX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XXXXX___,
	________,
	________,
	________,
	________,
//  99 $63 'char99'
//	width 7, bbx 0, bby 0, bbw 6, bbh 7
	________,
	________,
	________,
	_XXXX___,
	XX__XX__,
	XX__XX__,
	XX______,
	XX______,
	XX__XX__,
	_XXXX___,
	________,
	________,
	________,
	________,
// 100 $64 'char100'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	____XX__,
	____XX__,
	____XX__,
	_XXXXX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX_XXX__,
	_XX_XX__,
	________,
	________,
	________,
	________,
// 101 $65 'char101'
//	width 7, bbx 0, bby 0, bbw 6, bbh 7
	________,
	________,
	________,
	_XXXX___,
	XX__XX__,
	XX__XX__,
	XXXXXX__,
	XX______,
	XX__XX__,
	_XXXX___,
	________,
	________,
	________,
	________,
// 102 $66 'char102'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	___XXX__,
	__XX____,
	__XX____,
	XXXXXX__,
	__XX____,
	__XX____,
	__XX____,
	__XX____,
	__XX____,
	__XX____,
	________,
	________,
	________,
	________,
// 103 $67 'char103'
//	width 7, bbx 0, bby -2, bbw 6, bbh 9
	________,
	________,
	________,
	_XXXXX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	_XXXX___,
	_XX_____,
	___XXX__,
	XX__XX__,
	_XXXX___,
	________,
	________,
// 104 $68 'char104'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	XX______,
	XX______,
	XX______,
	XX_XX___,
	XXX_XX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	________,
	________,
	________,
	________,
// 105 $69 'char105'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	__XX____,
	__XX____,
	________,
	_XXX____,
	__XX____,
	__XX____,
	__XX____,
	__XX____,
	__XX____,
	XXXXXX__,
	________,
	________,
	________,
	________,
// 106 $6a 'char106'
//	width 7, bbx 0, bby -2, bbw 5, bbh 12
	___XX___,
	___XX___,
	________,
	_XXXX___,
	___XX___,
	___XX___,
	___XX___,
	___XX___,
	___XX___,
	___XX___,
	XX_XX___,
	_XXX____,
	________,
	________,
// 107 $6b 'char107'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	XX______,
	XX______,
	XX______,
	XX__XX__,
	XX_XX___,
	XXXX____,
	XXXX____,
	XX_XX___,
	XX__XX__,
	XX__XX__,
	________,
	________,
	________,
	________,
// 108 $6c 'char108'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	_XXX____,
	__XX____,
	__XX____,
	__XX____,
	__XX____,
	__XX____,
	__XX____,
	__XX____,
	__XX____,
	XXXXXX__,
	________,
	________,
	________,
	________,
// 109 $6d 'char109'
//	width 7, bbx 0, bby 0, bbw 6, bbh 7
	________,
	________,
	________,
	XX_XX___,
	XXXXXX__,
	XX_X_X__,
	XX_X_X__,
	XX_X_X__,
	XX_X_X__,
	XX_X_X__,
	________,
	________,
	________,
	________,
// 110 $6e 'char110'
//	width 7, bbx 0, bby 0, bbw 6, bbh 7
	________,
	________,
	________,
	XX_XX___,
	XXX_XX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	________,
	________,
	________,
	________,
// 111 $6f 'char111'
//	width 7, bbx 0, bby 0, bbw 6, bbh 7
	________,
	________,
	________,
	_XXXX___,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	_XXXX___,
	________,
	________,
	________,
	________,
// 112 $70 'char112'
//	width 7, bbx 0, bby -2, bbw 6, bbh 9
	________,
	________,
	________,
	________,
	XX_XX___,
	XXX_XX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XXXXX___,
	XX______,
	XX______,
	XX______,
	________,
// 113 $71 'char113'
//	width 7, bbx 0, bby -2, bbw 6, bbh 9
	________,
	________,
	________,
	________,
	_XXXXX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX_XXX__,
	_XX_XX__,
	____XX__,
	____XX__,
	____XX__,
	________,
// 114 $72 'char114'
//	width 7, bbx 0, bby 0, bbw 6, bbh 7
	________,
	________,
	________,
	XX_XX___,
	XXX_XX__,
	XX__XX__,
	XX______,
	XX______,
	XX______,
	XX______,
	________,
	________,
	________,
	________,
// 115 $73 'char115'
//	width 7, bbx 0, bby 0, bbw 6, bbh 7
	________,
	________,
	________,
	_XXXX___,
	XX__XX__,
	XX______,
	_XXXX___,
	____XX__,
	XX__XX__,
	_XXXX___,
	________,
	________,
	________,
	________,
// 116 $74 'char116'
//	width 7, bbx 0, bby 0, bbw 6, bbh 10
	___X____,
	__XX____,
	__XX____,
	XXXXXX__,
	__XX____,
	__XX____,
	__XX____,
	__XX____,
	__XX____,
	___XXX__,
	________,
	________,
	________,
	________,
// 117 $75 'char117'
//	width 7, bbx 0, bby 0, bbw 6, bbh 7
	________,
	________,
	________,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	XX_XXX__,
	_XX_XX__,
	________,
	________,
	________,
	________,
// 118 $76 'char118'
//	width 7, bbx 0, bby 0, bbw 6, bbh 7
	________,
	________,
	________,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	_X__X___,
	_XXXX___,
	__XX____,
	__XX____,
	________,
	________,
	________,
	________,
// 119 $77 'char119'
//	width 7, bbx 0, bby 0, bbw 6, bbh 7
	________,
	________,
	________,
	XX__XX__,
	XX__XX__,
	XX_X_X__,
	XX_X_X__,
	XX_X_X__,
	XX_X_X__,
	_XXXX___,
	________,
	________,
	________,
	________,
// 120 $78 'char120'
//	width 7, bbx 0, bby 0, bbw 6, bbh 7
	________,
	________,
	________,
	XX__XX__,
	XX__XX__,
	_XXXX___,
	__XX____,
	_XXXX___,
	XX__XX__,
	XX__XX__,
	________,
	________,
	________,
	________,
// 121 $79 'char121'
//	width 7, bbx 0, bby -2, bbw 6, bbh 9
	________,
	________,
	________,
	XX__XX__,
	XX__XX__,
	XX__XX__,
	_X__X___,
	_XXXX___,
	__XX____,
	__XX____,
	_XX_____,
	_XX_____,
	________,
	________,
// 122 $7a 'char122'
//	width 7, bbx 0, bby 0, bbw 6, bbh 7
	________,
	________,
	________,
	XXXXXX__,
	____XX__,
	___XX___,
	__XX____,
	_XX_____,
	XX______,
	XXXXXX__,
	________,
	________,
	________,
	________,
// 123 $7b 'char123'
//	width 7, bbx 0, bby -1, bbw 6, bbh 12
	___XXX__,
	__XX____,
	__XX____,
	__XX____,
	___X____,
	XXX_____,
	XXX_____,
	___X____,
	__XX____,
	__XX____,
	__XX____,
	___XXX__,
	________,
	________,
// 124 $7c 'char124'
//	width 7, bbx 2, bby -1, bbw 2, bbh 12
	__XX____,
	__XX____,
	__XX____,
	__XX____,
	__XX____,
	__XX____,
	__XX____,
	__XX____,
	__XX____,
	__XX____,
	__XX____,
	__XX____,
	________,
	________,
// 125 $7d 'char125'
//	width 7, bbx 0, bby -1, bbw 6, bbh 12
	XXX_____,
	__XX____,
	__XX____,
	__XX____,
	__X_____,
	___XXX__,
	___XXX__,
	__X_____,
	__XX____,
	__XX____,
	__XX____,
	XXX_____,
	________,
	________,
// 126 $7e 'char126'
//	width 7, bbx 0, bby 3, bbw 6, bbh 4
	_XX__X__,
	XXXX_X__,
	X_XXXX__,
	X__XX___,
	________,
	________,
	________,
	________,
	________,
	________,
	________,
	________,
	________,
	________,
// 160 $a0 'char160'
//	width 7, bbx 0, bby 0, bbw 1, bbh 1
	________,
	________,
	________,
	________,
	________,
	________,
	________,
	________,
	________,
	________,
	________,
	________,
	________,
	________,
};
