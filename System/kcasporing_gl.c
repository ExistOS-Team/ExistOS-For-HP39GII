
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

// extern "C" {
char *virtual_screen=0;

char *scale_vir_screen=0;

char * screen_1bpp=0;

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
      char * dest=tab,*end=dest+VIR_LCD_PIX_W;
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

    // printf("clra:%d,%d,%d,%d\n", x0, x1, y0, y1);
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
  memset(screen_1bpp, COLOR_WHITE, VIR_LCD_PIX_H * VIR_LCD_PIX_W);

  if (!virtual_screen)
    virtual_screen = pvPortMalloc(VIR_LCD_PIX_H * VIR_LCD_PIX_W);
  if (!virtual_screen) {
    vPortFree(screen_1bpp);
    printf("Failed to alloca virtual screen memory!\n");
    return -1;
  }
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

//}
