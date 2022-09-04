#include <stdio.h>

#include "SystemUI.h"
#include "FreeRTOS.h"
#include "task.h"

#include "timer.h"
#include "rom.h"
#include "mem.h"
#include "cpu.h"
#include "lcd.h"
#include "sdl.h"

#include "sys_llapi.h"

#include "ff.h"

#include "keyboard_gii39.h"

void sdl_frame_i(void);
void sdl_frame_clr(uint8_t c);

extern const unsigned char VGA_Ascii_5x8[];
extern const unsigned char VGA_Ascii_6x12[];
extern const unsigned char VGA_Ascii_8x16[];

void gb_putChar(int x0, int y0, char ch, int fg, int bg, int fontSize) {
	char *fbuff = (char *)sdl_get_framebuffer();
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

void gb_putString(int x0, int y0, char *s, int fg, int bg, int fontSize) {
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
            gb_putChar((x0) + x, (y0) + y, *s, fg, bg, fontSize);
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
    sdl_frame_i();
}

static char line[256];
static DIR dp;
static FILINFO finfo;
//int gb_main(int argc, char *argv[])
void gb_main_thumb(void *_)
{
	uint32_t keys = ll_vm_check_key();
	uint32_t key = keys & 0xFFFF;
	uint32_t press = keys >> 16;

	char *filenames;
	int filecnt = 0;
	
	SystemUISuspend();
	
	ll_cpu_slowdown_enable(false);
	
	int r;
	sdl_frame_clr(255);

	filenames = pvPortMalloc(9 * 128);
	memset(filenames, 0, 9*128);
	FRESULT fr;
	fr = f_mkdir("/gbrom");
    if(fr != FR_OK)
	{
		if(fr != FR_EXIST){
			goto out;
		}
	}
	
	gb_putString(0,0, "Select ROM at: /gbrom/*.gb  [0] exit", 0, 255, 12);

	fr = f_findfirst(&dp, &finfo, "/gbrom/", "*.gb");
	if((fr) || (finfo.fname[0] == 0))
	{
		goto find_fin;
	}
	while(fr == 0 || (finfo.fname[0]))
	{
		strcpy(&filenames[filecnt * 256], finfo.fname);
		sprintf(line, "[%d]:%s",filecnt + 1, finfo.fname);
		gb_putString(0, (filecnt + 1) * 12, line , 0, 255, 12);
		//printf("name:%s\n", finfo.fname);

		filecnt++;
		if(filecnt >= 9)
		{
			break;
		}
		fr = f_findnext(&dp, &finfo);
	}


	find_fin:

	while(press == 0)
	{
		keys = ll_vm_check_key();
		key = keys & 0xFFFF;
		press = keys >> 16;
	}

	char *loadFilename;

	#define chkf(k, n)  case k:if(filenames[n * 256] != 0){loadFilename = &filenames[n * 256];break;}else{goto out;}
	switch (key)
	{
	chkf(KEY_1, 0);
	chkf(KEY_2, 1);
	chkf(KEY_3, 2);
	chkf(KEY_4, 3);
	chkf(KEY_5, 4);
	chkf(KEY_6, 5);
	chkf(KEY_7, 6);
	chkf(KEY_8, 7);
	chkf(KEY_9, 8);

	default:
		goto out;
		break;
	}
	
	sprintf(line, "/gbrom/%s",loadFilename);

	r = rom_load(line);
	if(!r)
		goto out;

	printf("ROM OK!\n");

	r = lcd_init();
	if(r)
		goto out;
	
	printf("LCD OK!\n");

	mem_init();
	printf("Mem OK!\n");

	cpu_init();
	printf("CPU OK!\n");


	sdl_frame_clr(0);
	gb_putString(161, 12*0, "SYMB:A", 255, 0, 12);
	gb_putString(161, 12*1, "HOME:B", 255, 0, 12);
	gb_putString(161, 12*2, "NUM:START", 255, 0, 12);
	gb_putString(161, 12*3, "VIEWS:SELECT", 255, 0, 12);

	r = 0;


	while(1)
	{
		int now;

		if(!cpu_cycle())
			break;

		now = cpu_get_cycles();

		while(now != r)
		{
			int i;

			for(i = 0; i < 4; i++)
				if(!lcd_cycle())
					goto out;

			r++;
		}

		timer_cycle();

		r = now;
	}
out:
	vPortFree(filenames);
	sdl_quit();
}

void  __attribute__((target("arm"))) gb_main(void *_)
{

	gb_main_thumb(NULL);

	lv_obj_invalidate(lv_scr_act());

	
	SystemUIResume();

	vTaskDelete(NULL);
}

