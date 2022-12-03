
#include "FreeRTOS.h"
#include "task.h"

#include "SystemUI.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "keyboard_gii39.h"

#include "SysConf.h"

#include "UICore.h"
#include "UI_Config.h"
#include "UI_Language.h"

#include "Fatfs/ff.h"
#include "SystemFs.h"

#include <malloc.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CONF_SUBPAGES (3)

extern const unsigned char gImage_khicas_ico[48 * 48];

static char power_save = 'X';

UI_Display *uidisp;
UI_Window *mainw;

static int curPage = 0;
static int page3Subpage = 0;
static int appPage_select = 0;

static int language = UI_LANG_EN;

void memtest(uint32_t testSize);

extern uint32_t OnChipMemorySize;
extern uint32_t TotalAllocatableSize;

size_t getOnChipHeapAllocated();
size_t getSwapMemHeapAllocated();

void UI_keyScanner(void *_);
void drawPage(int page);

void getTimeStr(char *s) {
    uint32_t rtc_time_sec = ll_rtc_get_sec();
    sprintf(s, "%02d:%02d:%02d", (rtc_time_sec / (60 * 60)) % 24, (rtc_time_sec / 60) % 60, rtc_time_sec % 60);
}

static void timeChange(int hh, int mm, int ss) {
    uint32_t rtc_time_sec = ll_rtc_get_sec();

    rtc_time_sec += ss;
    rtc_time_sec += mm * 60;
    rtc_time_sec += hh * 60 * 60;

    ll_rtc_set_sec(rtc_time_sec);
    drawPage(curPage);
}

int exf_getfree(uint8_t *drv, uint32_t *total, uint32_t *free) {
    FATFS *fs1;
    FRESULT res;
    DWORD fre_clust = 0, fre_sect = 0, tot_sect = 0;
    res = f_getfree((const TCHAR *)drv, &fre_clust, &fs1);
    if (res == 0) {
        tot_sect = (fs1->n_fatent - 2) * fs1->csize;
        fre_sect = fre_clust * fs1->csize;
#if _MAX_SS != 512
        tot_sect *= fs1->ssize / 512;
        fre_sect *= fs1->ssize / 512;
#endif
        *total = tot_sect >> 1;
        *free = fre_sect >> 1;
    }
    return res;
}

uint32_t getHeapAllocateSize() {
    struct mallinfo info = mallinfo();
    return info.uordblks;
}

void UI_SetLang(int lang) {
    switch (lang) {
    case UI_LANG_EN:
        MAIN_WIN_TITLE = MAIN_WIN_TITLE_EN;
        MAIN_WIN_FKEY_BAR = MAIN_WIN_FKEY_BAR_EN;
        MAIN_WIN_FKEY_BAR2 = MAIN_WIN_FKEY_BAR2_EN;
        UI_TEMPERRATURE = UI_TEMPERRATURE_EN;
        UI_MEMUSE = UI_MEMUSED_EN;
        UI_BATTERY = UI_BATTERY_EN;
        UI_CHARGING = UI_CHARGING_EN;
        UI_TIME = UI_TIME_EN;
        UI_Power_Save_Mode = UI_Power_Save_Mode_EN;
        UI_Enable_Charge = UI_Enable_Charge_EN;
        UI_LANGUAGE = UI_LANGUAGE_EN;
        UI_Hours = UI_Hours_EN;
        UI_Minutes = UI_Minutes_EN;
        UI_Seconds = UI_Seconds_EN;
        UI_Storage_Space = UI_Storage_Space_EN;
        UI_ONF5Format = UI_ONF5Format_EN;
        UI_PhyMem = UI_PhyMem_EN;
        UI_Allocate_Mem = UI_Allocate_Mem_EN;
        UI_Compression_rate = UI_Compression_rate_EN;
        break;
    case UI_LANG_CN:
        MAIN_WIN_TITLE = MAIN_WIN_TITLE_CN;
        MAIN_WIN_FKEY_BAR = MAIN_WIN_FKEY_BAR_CN;
        MAIN_WIN_FKEY_BAR2 = MAIN_WIN_FKEY_BAR2_CN;
        UI_TEMPERRATURE = UI_TEMPERRATURE_CN;
        UI_MEMUSE = UI_MEMUSED_CN;
        UI_BATTERY = UI_BATTERY_CN;
        UI_CHARGING = UI_CHARGING_CN;
        UI_TIME = UI_TIME_CN;
        UI_Power_Save_Mode = UI_Power_Save_Mode_CN;
        UI_Enable_Charge = UI_Enable_Charge_CN;
        UI_LANGUAGE = UI_LANGUAGE_CN;
        UI_Hours = UI_Hours_CN;
        UI_Minutes = UI_Minutes_CN;
        UI_Seconds = UI_Seconds_CN;
        UI_Storage_Space = UI_Storage_Space_CN;
        UI_ONF5Format = UI_ONF5Format_CN;
        UI_PhyMem = UI_PhyMem_CN;
        UI_Allocate_Mem = UI_Allocate_Mem_CN;
        UI_Compression_rate = UI_Compression_rate_CN;
        break;
    default:
        break;
    }
}

#define DISPX (mainw->content_x0)
#define DISPY (mainw->content_y0)
#define DISPH (mainw->content_height)
#define DISPW (mainw->content_width)

void pageUpdate() {
    uint32_t clk_div[3];
    char timeStr[16];
    int line = 0;

    getTimeStr(timeStr);

    if (curPage <= 2) {
        uidisp->draw_box(180, 0, 255, 11, -1, 0);
        uidisp->draw_printf(180, 0, 16, 255, -1, "%s", timeStr);
    }

    if (curPage == 3) {
        uidisp->draw_box(180, 0, 255, 11, -1, 0);
        uidisp->draw_printf(200, 0, 16, 255, -1, "%d/%d", page3Subpage + 1, CONF_SUBPAGES);
        if (page3Subpage == 0) {
            ll_get_clkctrl_div(clk_div);

            uint32_t cur_cpu_div = clk_div[0];
            uint32_t cur_cpu_frac = clk_div[1];
            uint32_t cur_hclk_div = clk_div[2];

            uint32_t Charging = ll_get_charge_status();

            uidisp->draw_printf(DISPX, DISPY + 16 * line++, 16, 0, 255, "CPU:%3d/%d MHz, %s:%d `C", ll_get_cur_freq(), 480 * 18 / cur_cpu_div / cur_cpu_frac, UI_TEMPERRATURE, ll_get_core_temp());
            uidisp->draw_printf(DISPX, DISPY + 16 * line++, 16, 0, 255, "%s: %d/%d KB", UI_MEMUSE, getHeapAllocateSize() / 1024, TotalAllocatableSize / 1024);
            uidisp->draw_printf(DISPX, DISPY + 16 * line++, 16, 0, 255, "%s: %d mv, %s: %s  ", UI_BATTERY, ll_get_bat_voltage(), UI_CHARGING, Charging ? "Yes" : "NO");
            uidisp->draw_printf(DISPX, DISPY + 16 * line++, 16, 0, 255, "%s: %s", UI_TIME, timeStr);

            uidisp->draw_printf(DISPX, DISPY + 16 * line++, 16, 0, 255, "[%c]%s .. (1)", power_save, UI_Power_Save_Mode);
            uidisp->draw_printf(DISPX, DISPY + 16 * line++, 16, 0, 255, "[%c]%s .... (2)", Charging ? 'X' : ' ', UI_Enable_Charge);
        }

        if (page3Subpage == 1) {

            // sprintf(s, "%02d:%02d:%02d", (rtc_time_sec / (60 * 60)) % 24, (rtc_time_sec / 60) % 60, rtc_time_sec % 60);

            uidisp->draw_printf(DISPX, DISPY + 16 * line++, 16, 0, 255, "%s:[1]%s [2]%s", UI_LANGUAGE, UI_LANGUAGE_ENGLISH_EN, UI_LANGUAGE_CHINESE_CN);
            uidisp->draw_printf(DISPX, DISPY + 16 * line++, 16, 0, 255, "%s: %s", UI_TIME, timeStr);
            uidisp->draw_printf(DISPX, DISPY + 16 * line++, 16, 0, 255, "%s+ [7] %s+ [8] %s+ [9]", UI_Hours, UI_Minutes, UI_Seconds);
            uidisp->draw_printf(DISPX, DISPY + 16 * line++, 16, 0, 255, "%s- [4] %s- [5] %s- [6]", UI_Hours, UI_Minutes, UI_Seconds);
            uint32_t total, free;
            exf_getfree((uint8_t *)"0:", &total, &free);

            uidisp->draw_printf(DISPX, DISPY + 16 * line++, 16, 0, 255, "%s: %d/%d KB", UI_Storage_Space, total - free, total);
            uidisp->draw_printf(DISPX, DISPY + 16 * line++, 16, 0, 255, "%s", UI_ONF5Format);
        }
        if (page3Subpage == 2) {
            uint32_t free, total;
            float mem_cmpr = ll_mem_comprate();
            uint32_t total_phy_mem = ll_mem_phy_info(&free, &total);
            total_phy_mem /= 1024;
            free /= 1024;
            total /= 1024;
            uidisp->draw_printf(DISPX, DISPY + 16 * line++, 16, 0, 255, "%s:%d/%d KB   ", UI_Allocate_Mem, getHeapAllocateSize() / 1024, TotalAllocatableSize / 1024);
            uidisp->draw_printf(DISPX, DISPY + 16 * line++, 16, 0, 255, "%s:%d/%d KB   ", UI_PhyMem, total - free, total);
            uidisp->draw_printf(DISPX, DISPY + 16 * line++, 16, 0, 255, "%s:%.2f", UI_Compression_rate, mem_cmpr);
            uidisp->draw_printf(DISPX, DISPY + 16 * line++, 16, 0, 255, "SRAM Heap Pre-allocated: %d KB   ", getOnChipHeapAllocated() / 1024);
            uidisp->draw_printf(DISPX, DISPY + 16 * line++, 16, 0, 255, "Swap Heap Pre-allocated: %d KB   ", getSwapMemHeapAllocated() / 1024);
            uidisp->draw_printf(DISPX, DISPY + 16 * line++, 16, 0, 255, "[%c] Enable Memory Swap (1)", ll_mem_swap_size() ? 'X' : ' ');
        }
    }
}

void drawPage(int page) {

    uidisp->draw_box(mainw->content_x0,
                     mainw->content_y0,
                     mainw->content_x0 + mainw->content_width - 0,
                     mainw->content_y0 + mainw->content_height - 0,
                     -1, 0xFF);
    switch (page) {
    case 0:
        uidisp->draw_bmp((char *)gImage_khicas_ico, mainw->content_x0 + 12, mainw->content_y0 + 12, 48, 48);

        uidisp->draw_printf(mainw->content_x0 + 12,
                            mainw->content_y0 + 12 + 48 + 1, 16, 0, 0xFF, "KhiCAS");

        uidisp->draw_box((mainw->content_x0 + 12) + appPage_select * (48 + appPage_select * 32),
                         mainw->content_y0 + 12,
                         (mainw->content_x0 + 12) + 48 + appPage_select * (48 + appPage_select * 32),
                         mainw->content_y0 + 12 + 48,
                         0,
                         -1);
        break;

    case 3:
        pageUpdate();
        break;
    default:
        break;
    }
}

void keyMsg(uint32_t key, int state) {

    if ((state == KEY_TRIG) || (state == KEY_LONG_PRESS)) {
        switch (key) {

        case KEY_4:
            if ((curPage == 3) && (page3Subpage == 1)) {
                timeChange(-1, 0, 0);
            }
            break;
        case KEY_5:
            if ((curPage == 3) && (page3Subpage == 1)) {
                timeChange(0, -1, 0);
            }
            break;
        case KEY_6:
            if ((curPage == 3) && (page3Subpage == 1)) {
                timeChange(0, 0, -1);
            }
            break;

        case KEY_7:
            if ((curPage == 3) && (page3Subpage == 1)) {
                timeChange(1, 0, 0);
            }
            break;
        case KEY_8:
            if ((curPage == 3) && (page3Subpage == 1)) {
                timeChange(0, 1, 0);
            }
            break;
        case KEY_9:
            if ((curPage == 3) && (page3Subpage == 1)) {
                timeChange(0, 0, 1);
            }
            break;
        default:
            break;
        }
    }

    if (state == KEY_TRIG) {
        switch (key) {
        case KEY_F1:
            curPage = 0;
            drawPage(curPage);

            mainw->setFuncKeys(MAIN_WIN_FKEY_BAR);
            break;

        case KEY_F4: {
            if (curPage == 3) {
                if (page3Subpage < CONF_SUBPAGES - 1)
                    page3Subpage++;

                drawPage(curPage);
            }
            break;
        }

        case KEY_F3: {
            if (curPage == 3) {
                if (page3Subpage > 0)
                    page3Subpage--;

                drawPage(curPage);
            }
            break;
        }

        case KEY_F6:
            curPage = 3;
            page3Subpage = 0;
            drawPage(curPage);

            mainw->setFuncKeys(MAIN_WIN_FKEY_BAR2);

            break;

        case KEY_LEFT:
            if (curPage == 0) {
                if (appPage_select > 0)
                    appPage_select--;
                drawPage(curPage);
            }
            break;

        case KEY_RIGHT:
            if (curPage == 0) {
                if (appPage_select < 1)
                    appPage_select++;
                drawPage(curPage);
            }
            break;

        case KEY_ENTER:
            if (curPage == 0) {
                if (appPage_select == 0) {

                    void khicasTask(void *_);
                    xTaskCreate(khicasTask, "KhiCAS", KhiCAS_STACK_SIZE / 4, NULL, configMAX_PRIORITIES - 3, (NULL));
                }
            }
            break;
        case KEY_1:
            if (curPage == 3) {
                switch (page3Subpage) {
                case 0:
                    if (power_save == 'X') {
                        power_save = ' ';
                        ll_cpu_slowdown_enable(false);
                    } else {
                        power_save = 'X';
                        ll_cpu_slowdown_enable(true);
                    }
                    break;

                case 1: {
                    UI_SetLang(UI_LANG_EN);
                    mainw->setFuncKeys(MAIN_WIN_FKEY_BAR2);
                    mainw->refreshWindow();

                    pageUpdate();
                } break;

                case 2: {
                    printf("enableMemSwap\n");
                    void enableMemSwap(bool enable);
                    if (ll_mem_swap_size()) {
                        enableMemSwap( false);
                    } else {
                        enableMemSwap(true);
                    }
                } break;

                default:
                    break;
                }
                drawPage(curPage);
            }
            break;
        case KEY_2:
            if (curPage == 3) {
                switch (page3Subpage) {
                case 0:
                    if (ll_get_charge_status()) {

                        ll_charge_enable(false);
                    } else {

                        ll_charge_enable(true);
                    }
                    break;
                case 1: {
                    UI_SetLang(UI_LANG_CN);
                    mainw->setFuncKeys(MAIN_WIN_FKEY_BAR2);
                    mainw->refreshWindow();
                    drawPage(curPage);

                } break;

                default:
                    break;
                }
                pageUpdate();
            }
            break;
            /* 
        case KEY_3:
            if((curPage == 3) && (page3Subpage == 2))
            {
                memtest(32 * 1024);
            }
            break;
        case KEY_4:
            if((curPage == 3) && (page3Subpage == 2))
            {
                memtest(128 * 1024);
            }
            break;
        case KEY_5:
            if((curPage == 3) && (page3Subpage == 2))
            {
                memtest(256 * 1024);
            }
            break;
        case KEY_6:
            if((curPage == 3) && (page3Subpage == 2))
            {
                memtest(768 * 1024);
            }
            break;
            */

        default:
            break;
        }
    }
}
// static UI_Window *main_win;

static void checkFS() {
    uint32_t disp_off_y = DISPY + 2;
    FRESULT fres;
    FATFS *fs = (FATFS *)pvPortMalloc(sizeof(FATFS));
    uint32_t keys = ll_vm_check_key() & 0xFFFF;

    fres = f_mount(fs, FS_FLASH_PATH, 1);
    if (fres != FR_OK) {
        uidisp->draw_printf(0, disp_off_y + 16 * 0, 16, 0, -1, "The flash is not initialized.");
        uidisp->draw_printf(0, disp_off_y + 16 * 1, 16, 0, -1, "Press [F2] to format.");

        uidisp->draw_printf(0, disp_off_y + 16 * 2, 16, 0, -1, UI_FS_init1);
        uidisp->draw_printf(0, disp_off_y + 16 * 3, 16, 0, -1, UI_FS_init2);

        while (keys != KEY_F2) {
            vTaskDelay(pdMS_TO_TICKS(20));
            keys = ll_vm_check_key() & 0xFFFF;
        }

        uidisp->draw_printf(0, disp_off_y + 16 * 5, 16, 0, -1, "Formatting...");
        uidisp->draw_printf(0, disp_off_y + 16 * 6, 16, 0, -1, UI_FS_init3);

        BYTE *work = (BYTE *)pvPortMalloc(FF_MAX_SS);
        fres = f_mkfs(FS_FLASH_PATH, 0, work, FF_MAX_SS);
        // printf("mkfs:%d\n", fres);
        vPortFree(work);
    }
}

void UI_Task(void *_) {

    uidisp = new UI_Display(LCD_PIX_W, LCD_PIX_H, ll_disp_put_area);
    mainw = new UI_Window(NULL, NULL, MAIN_WIN_TITLE, uidisp, 0, 0, LCD_PIX_W, LCD_PIX_H);
    UI_SetLang(UI_LANG_EN);

    checkFS();

    mainw->setFuncKeys(MAIN_WIN_FKEY_BAR);
    mainw->enableFuncKey(true);

    drawPage(curPage);
    UI_keyScanner(NULL);

    /*
    main_win = &mainw;
    mainw.setFuncKeys(MAIN_WIN_FKEY_BAR);
    mainw.enableFuncKey(true);

    //printf("mw:%d,%d,%d,%d\n",mainw.content_x0, mainw.content_y0, mainw.content_width, mainw.content_height  );
    UI_WidgetPage of = UI_WidgetPage(mainw.content_x0, mainw.content_y0, mainw.content_width, mainw.content_height);
    mainw.addWidget(&of);

    UI_Widget *mainw_page[3];
    mainw_page[0] = of.newPage(0);
    mainw_page[1] = of.newPage(1);
    mainw_page[2] = of.newPage(5);

    of.setDisplayPage(0);

    of.bindPageSwitchWithKey(0, KEY_F1, KEY_TRIG);
    of.bindPageSwitchWithKey(1, KEY_F2, KEY_TRIG);
    of.bindPageSwitchWithKey(5, KEY_F6, KEY_TRIG);
    sizeof(of);

    UI_WidgetImgButton app_btn = UI_WidgetImgButton(of.x + 16, of.y + 12, 48, 48);
    UI_WidgetImgButton app_btn2 = UI_WidgetImgButton(of.x + 32 + 64, of.y + 12, 48, 48);

    extern const unsigned char gImage_khicas_ico[48*48];

    app_btn.setPic((uint8_t *)gImage_khicas_ico);
    app_btn.setTitle("KhiCAS");


    app_btn2.setTitle("TestBtn");


    mainw_page[0]->addSubWidget(&app_btn);
    mainw_page[0]->addSubWidget(&app_btn2);

    mainw.refreshWindow();

    //xTaskCreate((TaskFunction_t)UI_keyScanner, "UIKeyScan", 200, NULL, configMAX_PRIORITIES - 3, NULL);
    */
}

void UI_keyScanner(void *_) {
    uint32_t key;
    uint32_t keyVal = 0;

    uint32_t delay = 0;
    uint32_t delayRel = 0;

    uint32_t cnt = 0;
    for (;;) {

        key = ll_vm_check_key();
        uint32_t press = key >> 16;
        uint32_t keyVal = key & 0xFFFF;

        if (press) {
            delay++;
            delayRel = 0;

            if (delay == 1) {
                keyMsg(keyVal, KEY_TRIG);
                // printf("Trig:%d\n", keyVal);
                // main_win->winKeyMessage(keyVal, KEY_TRIG);
            }
            if (delay > 5) {
                if (delay % 5 == 0) {
                    keyMsg(keyVal, KEY_LONG_PRESS);
                    // main_win->winKeyMessage(keyVal, KEY_LONG_PRESS);
                    // printf("Long Press:%d\n", keyVal);
                }
            }

        } else {

            delay = 0;
            delayRel++;
            if (delayRel == 1) {
                keyMsg(keyVal, KEY_RELEASE);
                // main_win->winKeyMessage(keyVal, KEY_RELEASE);
                // printf("Rel:%d\n", keyVal);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(20));
        cnt++;
        if (cnt % 45 == 0) {
            pageUpdate();
        }
    }
}

#ifdef __cplusplus
}
#endif