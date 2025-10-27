
// -*- coding: GBK -*-

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
#include "UI_build_stamp.h"
#include "SystemConfig.h"

// 声明enableMemSwap函数
extern "C" void enableMemSwap(bool enable);

#include "ExistOSlogo.h"

#include "filesystem/fatfs/ff.h"
#include "SystemFs.h"

#include <malloc.h>

#include "timestamp.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CONF_SUBPAGES (4)

#include "../applications/user/khicas/khicas_ico.c"

extern const unsigned char gImage_khicas_ico[48 * 48];

// 移除静态变量，改为使用配置系统
// static char power_save = ' ';
// static int language = UI_LANG_EN;

UI_Display *uidisp;
UI_Window *mainw;
UI_Msgbox *msgbox;
SimpShell *console;

bool UIForceRefresh = false;

bool isMsgBoxShow = false;

static int curPage = 0;
static int page3Subpage = 0;
static int appPage_select = 0;

static int alpha = 0, shift = 0;

void memtest(uint32_t testSize);

extern uint32_t OnChipMemorySize;
extern uint32_t TotalAllocatableSize;

struct strNode {
    TCHAR *str;
    struct strNode *prev;
    struct strNode *next;
};

TCHAR *suffix;
TCHAR *pathNow;
TCHAR **dirItemNames;
bool *dirItemInfos; // ture:file false:folder
unsigned long *filesCount;
unsigned int *pageNow, *pageAll;
unsigned char *selectedItem;
struct strNode *pathList;
struct strNode *pathList_firstNode;
static DIR fileManagerDir;
static FILINFO fileInfo;
char *conin; /* Console Input Buffer */
void initConsole();
void refreshConsole();
void keyupConsole(Keys_t key);
unsigned long getFileCounts(TCHAR *path);
void refreshFileNames(TCHAR *path, TCHAR **names, bool *info, unsigned long *counts);
void refreshDir();
void getWholePath(TCHAR *ans);
void getSuffix(TCHAR *ret, TCHAR *filename); // get suffix without a dot.

size_t getOnChipHeapAllocated();
size_t getSwapMemHeapAllocated();
uint32_t getHeapAllocateSize();

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

#define SET_LANG(lang)                                              \
    MAIN_WIN_TITLE = MAIN_WIN_TITLE_##lang;                         \
    MAIN_WIN_FKEY_BAR = MAIN_WIN_FKEY_BAR_##lang;                   \
    MAIN_WIN_FKEY_BAR2 = MAIN_WIN_FKEY_BAR2_##lang;                 \
    MAIN_WIN_FKEY_BARFILE = MAIN_WIN_FKEY_BARFILE_##lang;           \
    UI_Yes = UI_Yes_##lang;                                         \
    UI_No = UI_No_##lang;                                           \
    UI_TEMPERRATURE = UI_TEMPERRATURE_##lang;                       \
    UI_MEMUSE = UI_MEMUSED_##lang;                                  \
    UI_BATTERY = UI_BATTERY_##lang;                                 \
    UI_CHARGING = UI_CHARGING_##lang;                               \
    UI_TIME = UI_TIME_##lang;                                       \
    UI_Power_Save_Mode = UI_Power_Save_Mode_##lang;                 \
    UI_Enable_Charge = UI_Enable_Charge_##lang;                     \
    UI_LANGUAGE = UI_LANGUAGE_##lang;                               \
    UI_Hours = UI_Hours_##lang;                                     \
    UI_Minutes = UI_Minutes_##lang;                                 \
    UI_Seconds = UI_Seconds_##lang;                                 \
    UI_Storage_Space = UI_Storage_Space_##lang;                     \
    UI_ONF5Format = UI_ONF5Format_##lang;                           \
    UI_PhyMem = UI_PhyMem_##lang;                                   \
    UI_Allocate_Mem = UI_Allocate_Mem_##lang;                       \
    UI_Compression_rate = UI_Compression_rate_##lang;               \
    UI_SRAM_Heap_Pre_Allocated = UI_SRAM_Heap_Pre_Allocated_##lang; \
    UI_Swap_Heap_Pre_Allocated = UI_Swap_Heap_Pre_Allocated_##lang; \
    UI_Enable_Mem_Swap = UI_Enable_Mem_Swap_##lang;

void UI_SetLang(int lang) {
    // 保存语言设置到配置系统
    config_set_language(lang);
    
    switch (lang) {
    case UI_LANG_EN:
        SET_LANG(EN)
        break;
    case UI_LANG_CN:
        SET_LANG(CN)
        break;
    default:
        break;
    }
}

#define DISPX (mainw->content_x0)
#define DISPY (mainw->content_y0)
#define DISPH (mainw->content_height) /* 95 */
#define DISPW (mainw->content_width)  /* 255 */

static inline void __puts(char *s, int len, int line, uint8_t fsize) {
    for (int i = 0; i < len - 1; i++) {
        uidisp->draw_char_ascii(32 + i * (fsize == 16 ? 8 : 6), 16 + line * 16, s[i], fsize, 255, 0);
    }
}

static char msg1[] = "!!Out of Memory!!";
static char msg2[] = "Enable Mem Swap or use";
static char msg3[] = "memory Compression.";
static char msg4[] = "[ON]+[F6] > Reboot";
void UI_OOM() {
    uidisp->emergencyBuffer();
    uidisp->draw_box(0, 0, 255, 126, 192, 192);
    uidisp->draw_box(8, 8, 247, 118, 0, 0);
    uidisp->draw_box(214, 16, 228, 80, 128, 128);
    uidisp->draw_box(214, 96, 228, 108, 128, 128);
    __puts(msg1, sizeof(msg1), 0, 16);
    __puts(msg2, sizeof(msg2), 2, 12);
    __puts(msg3, sizeof(msg3), 3, 12);
    __puts(msg4, sizeof(msg4), 5, 12);
}

void pageUpdate() {
    uint32_t clk_div[3];
    char timeStr[16];
    int line = 0;

    getTimeStr(timeStr);

    if (curPage == 1) {
        console->blink();
    }

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
            uidisp->draw_printf(DISPX, DISPY + 16 * line++, 16, 0, 255, "%s: %d mv, %s: %s  ", UI_BATTERY, ll_get_bat_voltage(), UI_CHARGING, Charging ? UI_Yes : UI_No);
            uidisp->draw_printf(DISPX, DISPY + 16 * line++, 16, 0, 255, "%s: %s", UI_TIME, timeStr);

            uidisp->draw_printf(DISPX, DISPY + 16 * line++, 16, 0, 255, "[%c]%s (1)", config_get_power_save(), UI_Power_Save_Mode);
            uidisp->draw_printf(DISPX, DISPY + 16 * line++, 16, 0, 255, "[%c]%s (2)", config_get_enable_charge() ? 'X' : ' ', UI_Enable_Charge);
        } else if (page3Subpage == 1) {

            // sprintf(s, "%02d:%02d:%02d", (rtc_time_sec / (60 * 60)) % 24, (rtc_time_sec / 60) % 60, rtc_time_sec % 60);

            uidisp->draw_printf(DISPX, DISPY + 16 * line++, 16, 0, 255, "%s:[1]%s [2]%s", UI_LANGUAGE, UI_LANGUAGE_ENGLISH_EN, UI_LANGUAGE_CHINESE_CN);
            uidisp->draw_printf(DISPX, DISPY + 16 * line++, 16, 0, 255, "%s: %s", UI_TIME, timeStr);
            uidisp->draw_printf(DISPX, DISPY + 16 * line++, 16, 0, 255, "%s+ [7] %s+ [8] %s+ [9]", UI_Hours, UI_Minutes, UI_Seconds);
            uidisp->draw_printf(DISPX, DISPY + 16 * line++, 16, 0, 255, "%s- [4] %s- [5] %s- [6]", UI_Hours, UI_Minutes, UI_Seconds);
            uint32_t total, free;
            exf_getfree((uint8_t *)"0:", &total, &free);

            uidisp->draw_printf(DISPX, DISPY + 16 * line++, 16, 0, 255, "%s: %d/%d KB", UI_Storage_Space, total - free, total);
            uidisp->draw_printf(DISPX, DISPY + 16 * line++, 16, 0, 255, "%s", UI_ONF5Format);
        } else if (page3Subpage == 2) {
            uint32_t free, total;
            float mem_cmpr = ll_mem_comprate();
            uint32_t total_phy_mem = ll_mem_phy_info(&free, &total);
            total_phy_mem /= 1024;
            free /= 1024;
            total /= 1024;
            uidisp->draw_printf(DISPX, DISPY + 16 * line++, 16, 0, 255, "%s:%d/%d KB   ", UI_Allocate_Mem, getHeapAllocateSize() / 1024, TotalAllocatableSize / 1024);
            uidisp->draw_printf(DISPX, DISPY + 16 * line++, 16, 0, 255, "%s:%d/%d KB   ", UI_PhyMem, total - free, total);
            uidisp->draw_printf(DISPX, DISPY + 16 * line++, 16, 0, 255, "%s:%.2f", UI_Compression_rate, mem_cmpr);
            uidisp->draw_printf(DISPX, DISPY + 16 * line++, 16, 0, 255, "%s:%d KB   ", UI_SRAM_Heap_Pre_Allocated, getOnChipHeapAllocated() / 1024);
            uidisp->draw_printf(DISPX, DISPY + 16 * line++, 16, 0, 255, "%s:%d KB   ", UI_Swap_Heap_Pre_Allocated, getSwapMemHeapAllocated() / 1024);
            uidisp->draw_printf(DISPX, DISPY + 16 * line++, 16, 0, 255, "[%c] %s (1)", ll_mem_swap_size() ? 'X' : ' ', UI_Enable_Mem_Swap);
        } else if (page3Subpage == 3) {
            uidisp->draw_bmp((char *)logo, DISPX + 12, DISPY + 8, 50, 25);

            uidisp->draw_line(DISPX + 64, DISPY + 10, DISPX + 64, DISPY + 30, 64);
            uidisp->draw_line(DISPX + 63, DISPY + 10, DISPX + 63, DISPY + 30, 64);

            uidisp->draw_printf(DISPX + 72, DISPY + 11, 8, 64, 255, BUILD_STAMP);
            uidisp->draw_printf(DISPX + 72, DISPY + 23, 8, 64, 255, "%s", _TIMEZ_);

            line = 3;
            uidisp->draw_printf(DISPX + (DISPW - (8 * 29)) / 2, DISPY - 8 + 16 * line++, 16, 64, 255, "Open Source Firmware Project");
            uidisp->draw_printf(DISPX + (DISPW - (16 * 9 + 8 * 7)) / 2, DISPY - 8 + 16 * line++, 16, 64, 255, "HP39GII开源固件项目");
            uidisp->draw_printf(DISPX + 16, DISPY - 4 + 16 * line, 8, 64, 255, "github.com/ExistOS-Team");
            uidisp->draw_printf(DISPX + DISPW - 16 - 6 * 21, DISPY - 4 + 16 * line + 8, 8, 64, 255, "/ExistOS-For-HP39GII");
        }
    }
    if (isMsgBoxShow)
        msgbox->refresh();
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

    case 1:
        refreshConsole();
        break;

    case 2:
        uidisp->draw_box(DISPX, DISPY + 16, 255, DISPY + 16, -1, 0);
        uidisp->draw_printf(DISPX, DISPY, 16, 0, 255, "%d item(s) [%s] (%d/%d)", *filesCount, pathNow, (*filesCount == 0 ? 0 : *pageNow), *pageAll);
        for (int i = 1; i <= 5 && ((*pageNow - 1) * 5 + i) <= *filesCount; i++) {
            uidisp->draw_printf(DISPX, DISPY + i * 16 + 1, 12, (i == *selectedItem ? 255 : 0), (i == *selectedItem ? 0 : 255), "%s%s", (dirItemInfos[(*pageNow - 1) * 5 + i - 1] == false ? "/" : ""), dirItemNames[(*pageNow - 1) * 5 + i - 1]);
        }
        if (*filesCount == 0) {
            uidisp->draw_printf(DISPX + 64, DISPY + 48, 8, 0, 255, "Nothing here...");
        }
        break;

    case 3:
        pageUpdate();
        break;
    default:
        break;
    }

    if (isMsgBoxShow)
        msgbox->refresh();
}

void UI_Refrush() {
    mainw->refreshWindow();
    drawPage(curPage);
}

void UI_Suspend() {
    uidisp->releaseBuffer();
}

void UI_Resume() {
    uidisp->restoreBuffer();
}

void refreshIndicator() {
    uint32_t ind = 0;
    switch (alpha) {
    case 1:
        ind |= INDICATE_A__Z;
        break;
    case 2:
        ind |= INDICATE_a__z;
        break;
    default:
        ind &= ~INDICATE_a__z;
        ind &= ~INDICATE_A__Z;
        break;
    }
    switch (shift) {
    case 1:
        ind |= INDICATE_LEFT;
        break;
    case 2:
        ind |= INDICATE_RIGHT;
        break;
    default:
        ind &= ~INDICATE_LEFT;
        ind &= ~INDICATE_RIGHT;
        break;
    }
    ll_disp_set_indicator(ind, -1);
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

        case KEY_ON: {
            if (shift == 1) {
                uidisp->draw_box(0, 0, 255, 126, 255, 255);
                uidisp->draw_bmp((char *)logo, 103, 32, 50, 25);
                uidisp->draw_printf(128 - 14 * 6 / 2, 74, 12, 0, 255, "Shutting down");

                printf("Trig Power Off\n");

                if (curPage == 2) {
                    f_closedir(&fileManagerDir);
                }

                vTaskDelay(pdMS_TO_TICKS(500));
                ll_power_off();
                ll_power_off();
                vTaskDelay(pdMS_TO_TICKS(500));
                ll_power_off();
            }
            break;
        }

        default:
            break;
        }
    }

    if (state == KEY_TRIG) {
        if (curPage == 2 && (key == KEY_F1 || key == KEY_F2 || key == KEY_F6)) {
            f_closedir(&fileManagerDir);

            for (int i = 0; i < *filesCount; i++) {
                free(dirItemNames[i]);
            }
            free(dirItemNames);

            free(dirItemInfos);
            free(filesCount);
            free(pageNow);
            free(pageAll);
            free(selectedItem);
            free(pathNow);
            free(suffix);

            while (pathList != pathList_firstNode) {
                pathList = pathList->prev; // switch to prev node.

                free(pathList->next->str);
                pathList->next->str = nullptr;
                free(pathList->next);
                pathList->next = nullptr;
            }

            free(pathList->str);
            pathList->str = nullptr;
            free(pathList);
            pathList = nullptr;

            pathList_firstNode = nullptr;
        }

        switch (key) {
        case KEY_SHIFT:
            shift++;
            if (shift > 2)
                shift = 0;
            refreshIndicator();
            break;

        case KEY_ALPHA:
            alpha++;
            if (alpha > 2)
                alpha = 0;
            refreshIndicator();
            break;

        case KEY_F1:
            curPage = 0;
            drawPage(curPage);

            mainw->setFuncKeys(MAIN_WIN_FKEY_BAR);
            break;

        case KEY_F2:
            if (curPage != 1) {
                initConsole();
            }
            curPage = 1;
            drawPage(curPage);

            mainw->setFuncKeys(MAIN_WIN_FKEY_BAR);
            break;

        case KEY_F5:
            if (curPage == 2) {
                if (*filesCount > 0) {
                    if (dirItemInfos[(*pageNow - 1) * 5 + *selectedItem - 1]) {
                        msgbox = new UI_Msgbox(uidisp, 16, 32, 256 - 32, 64, "Delete File", "Press ENTER to confirm.");
                    } else {
                        msgbox = new UI_Msgbox(uidisp, 16, 32, 256 - 32, 64, "Delete Folder", "Press ENTER to confirm.");
                    }

                    isMsgBoxShow = true;
                    drawPage(curPage);

                    if (msgbox->show()) {
                        msgbox->setText("Please wait...");
                        drawPage(curPage);

                        strcat(pathNow, dirItemNames[(*pageNow - 1) * 5 + *selectedItem - 1]);

                        // if (dirItemInfos[(*pageNow - 1) * 5 + *selectedItem - 1]) {
                        //     f_unlink(pathNow);
                        // } else {
                        //     // strcat(pathNow, "/");
                        //     deleteFiles(pathNow);
                        // }

                        FS_DeleteFolderOrFile(pathNow);

                        getWholePath(pathNow);
                        refreshDir();
                    }
                    isMsgBoxShow = false;
                    delete msgbox;
                    drawPage(curPage);
                }

            } else {

                suffix = (TCHAR *)calloc(64, sizeof(TCHAR));
                filesCount = (unsigned long *)malloc(sizeof(unsigned long));
                pageNow = (unsigned int *)malloc(sizeof(unsigned int));
                pageAll = (unsigned int *)malloc(sizeof(unsigned int));
                selectedItem = (unsigned char *)malloc(sizeof(unsigned char)); // 1 to 5

                pathList = (struct strNode *)malloc(sizeof(struct strNode)); // head node.
                pathList->str = (TCHAR *)calloc(2, sizeof(TCHAR));
                pathNow = (TCHAR *)calloc(2048, sizeof(TCHAR));
                pathList->next = nullptr;
                pathList->prev = nullptr;
                pathList_firstNode = pathList; // record head node.

                strcpy(pathList->str, "/"); // set to root path.

                getWholePath(pathNow);

                if (f_opendir(&fileManagerDir, pathNow) == FR_OK) {
                    if (f_readdir(&fileManagerDir, &fileInfo) == FR_OK) {
                        f_closedir(&fileManagerDir);

                        *filesCount = getFileCounts(pathNow);
                        *pageNow = 1;
                        *selectedItem = 1;
                        *pageAll = *filesCount / 5 + (*filesCount % 5 == 0 ? 0 : 1);

                        dirItemNames = (TCHAR **)calloc(*filesCount, sizeof(TCHAR *));
                        dirItemInfos = (bool *)calloc(*filesCount, sizeof(bool));
                        for (int i = 0; i < *filesCount; i++) {
                            dirItemNames[i] = (TCHAR *)calloc(255, sizeof(TCHAR));
                        }

                        refreshFileNames(pathNow, dirItemNames, dirItemInfos, filesCount);
                        if (dirItemNames != NULL) {
                            curPage = 2;
                            drawPage(curPage);
                            mainw->setFuncKeys(MAIN_WIN_FKEY_BARFILE);
                        }
                    }
                }
            }

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
            if (curPage == 1) {
                goto CONSOLE_KEY_EVENT;
            }
            if (curPage == 0) {
                if (appPage_select > 0) {
                    appPage_select--;
                    drawPage(curPage);
                }
            } else if (curPage == 2) {
                if (pathList->prev != nullptr) {
                    pathList = pathList->prev; // switch to prev node.

                    free(pathList->next->str); // free str.
                    pathList->next->str = nullptr;
                    pathList->next->prev = nullptr;
                    free(pathList->next); // free next node.
                    pathList->next = nullptr;

                    getWholePath(pathNow);

                    refreshDir();
                    drawPage(curPage);
                }
            }
            break;

        case KEY_RIGHT:
            if (curPage == 1) {
                goto CONSOLE_KEY_EVENT;
            }
            if (curPage == 0) {
                if (appPage_select < 1) {
                    appPage_select++;
                    drawPage(curPage);
                }
            }
            break;

        case KEY_UP:
            if (curPage == 1) {
                goto CONSOLE_KEY_EVENT;
            }
            if (curPage == 2) {
                if (*selectedItem != 1) {
                    (*selectedItem)--;
                    drawPage(curPage);
                } else {
                    if (*pageNow > 1) {
                        (*pageNow)--;
                        *selectedItem = 5;
                        drawPage(curPage);
                    }
                }
            }
            break;

        case KEY_DOWN:
            if (curPage == 1) {
                goto CONSOLE_KEY_EVENT;
            }
            if (curPage == 2) {
                if (*selectedItem != 5 && (*pageNow - 1) * 5 + *selectedItem != *filesCount) {
                    (*selectedItem)++;
                    drawPage(curPage);
                } else {
                    if (*pageNow < *pageAll) {
                        (*pageNow)++;
                        *selectedItem = 1;
                        drawPage(curPage);
                    }
                }
            }
            break;

        case KEY_ENTER:
            if (curPage == 0) {
                if (appPage_select == 0) {

                    void StartKhiCAS();
                    StartKhiCAS();
                }
            } else if (curPage == 1) {
                goto CONSOLE_KEY_EVENT;
            } else if (curPage == 2) {
                if (*filesCount > 0) {
                    if (dirItemInfos[(*pageNow - 1) * 5 + *selectedItem - 1] == false) {
                        // open a folder

                        pathList->next = (struct strNode *)malloc(sizeof(struct strNode)); // new node.
                        (pathList->next)->prev = pathList;                                 // set prev node.
                        (pathList->next)->next = nullptr;

                        pathList = pathList->next; // switch to next node.

                        pathList->str = (TCHAR *)calloc(strlen(dirItemNames[(*pageNow - 1) * 5 + *selectedItem - 1]) + 1, sizeof(TCHAR)); // new str.

                        strcpy(pathList->str, dirItemNames[(*pageNow - 1) * 5 + *selectedItem - 1]);
                        strcat(pathList->str, "/");

                        getWholePath(pathNow);

                        refreshDir();
                        drawPage(curPage);
                    } else {
                        // do something with the file here...
                        // strcat(pathNow, dirItemNames[(*pageNow - 1) * 5 + *selectedItem - 1]);
                        // getSuffix(suffix, pathNow);

                        // if (strcmp(suffix, "jpg")) {
                        // }

                        // refreshDir();
                        // getWholePath(pathNow);
                    }
                }
            }
            break;

        case KEY_PLUS:
            if (curPage == 1) {
                goto CONSOLE_KEY_EVENT;
            }
            if (curPage == 2) {
                if (*pageNow < *pageAll) {
                    (*pageNow)++;
                    *selectedItem = 1;
                    drawPage(curPage);
                }
            }
            break;

        case KEY_SUBTRACTION:
            if (curPage == 1) {
                goto CONSOLE_KEY_EVENT;
            }
            if (curPage == 2) {
                if (*pageNow > 1) {
                    (*pageNow)--;
                    *selectedItem = 1;
                    drawPage(curPage);
                }
            }
            break;

        case KEY_1:
            if (curPage == 1) {
                goto CONSOLE_KEY_EVENT;
            }
            if (curPage == 3) {
                switch (page3Subpage) {
                case 0:
                    if (config_get_power_save() == 'S') {
                        config_set_power_save('L');
                        ll_cpu_slowdown_enable(2);
                    } else if (config_get_power_save() == 'L') {
                        config_set_power_save(' ');
                        ll_cpu_slowdown_enable(0);
                    } else if (config_get_power_save() == ' ') {
                        config_set_power_save('S');
                        ll_cpu_slowdown_enable(1);
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
                        enableMemSwap(false);
                        config_set_enable_mem_swap(false);
                    } else {
                        enableMemSwap(true);
                        config_set_enable_mem_swap(true);
                    }
                } break;

                default:
                    break;
                }
                drawPage(curPage);
            }
            break;
        case KEY_2:
            if (curPage == 1) {
                goto CONSOLE_KEY_EVENT;
            }
            if (curPage == 3) {
                switch (page3Subpage) {
                case 0:
                    if (config_get_enable_charge()) {
                        config_set_enable_charge(false);
                        ll_charge_enable(false);
                    } else {
                        if (config_get_power_save() != 'L') {
                            config_set_power_save('L');
                            ll_cpu_slowdown_enable(2);
                        }
                        config_set_enable_charge(true);
                        ll_charge_enable(true);
                    }
                    break;
                case 1: {
                    UI_SetLang(UI_LANG_CN);
                    mainw->setFuncKeys(MAIN_WIN_FKEY_BAR2);
                    mainw->refreshWindow();
                    pageUpdate();
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
            // mainw->refreshWindow();
            // drawPage(curPage);
            if (curPage == 1) {
            CONSOLE_KEY_EVENT :
#define K(KEY, NORM, SHIFT, ALPHA, ALPHAS) \
    case KEY: {                            \
        if (shift) {                       \
            console->puts(SHIFT);          \
            shift = 0;                     \
        } else {                           \
            if (abs(alpha) == 1) {         \
                console->puts(ALPHA);      \
            } else if (abs(alpha) == 2) {  \
                console->puts(ALPHAS);     \
            } else {                       \
                console->puts(NORM);       \
            }                              \
            if (alpha > 0) {               \
                alpha = 0;                 \
            }                              \
        }                                  \
    } break;
            {
                switch (key) {
                    K(KEY_0, "0", "@", "\"", "\"")
                    K(KEY_1, "1", "$", "X", "x")
                    K(KEY_2, "2", "2", "Y", "y")
                    K(KEY_3, "3", "3", "Z", "z")
                    K(KEY_4, "4", "#", "T", "t")
                    K(KEY_5, "5", "[", "U", "u")
                    K(KEY_6, "6", "]", "V", "v")
                    K(KEY_7, "7", "&", "P", "p")
                    K(KEY_8, "8", "{", "Q", "q")
                    K(KEY_9, "9", "}", "R", "r")
                    K(KEY_VARS, "~", "`", "A", "a")
                    K(KEY_MATH, "\b", "$", "B", "b")
                    K(KEY_ABC, "\'", "\'", "C", "c")
                    K(KEY_XTPHIN, "X", "e", "D", "d")
                    K(KEY_BACKSPACE, "\x7F", "\x7F", "\x7F", "\x7F")
                    K(KEY_SIN, "e", "E", "E", "e")
                    K(KEY_COS, "f", "F", "F", "f")
                    K(KEY_TAN, "g", "G", "G", "g")
                    K(KEY_LN, "h", "H", "H", "h")
                    K(KEY_LOG, "i", "H", "I", "i")
                    K(KEY_X2, "j", "J", "J", "j")
                    K(KEY_XY, "^", "^", "K", "k")
                    K(KEY_LEFTBRACKET, "(", "<", "L", "l")
                    K(KEY_RIGHTBRACKET, ")", ">", "M", "m")
                    K(KEY_DIVISION, "/", "/", "N", "n")
                    K(KEY_COMMA, ",", "`", "O", "o")
                    K(KEY_MULTIPLICATION, "*", "!", "S", "s")
                    K(KEY_SUBTRACTION, "-", "-", "W", "w")
                    K(KEY_PLUS, "+", "+", " ", " ")
                    K(KEY_DOT, ".", "=", ":", ":")
                    K(KEY_NEGATIVE, "_", "|", ";", ";")
                    K(KEY_ENTER, "\n", "\n", "\n", "\n")
                }
            }
#undef K
            }
            break;
        }
    }

    refreshIndicator();
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
#define CONSH (DISPH / 8) /* 11 */
#define CONSW (DISPW / 8) /* 31 */

inline void initConsole() {
    console = new SimpShell(uidisp);
    console->puts("\n"
        "ExistOS Console v0.0.0\n"
        "2025 (C) ExistOS Team\n"
        "ExistOS is licensed under GPL-3.0, for more information please visit <https://github.com/ExistOS-Team/ExistOS-For-HP39GII>\n"
        "Try `help` for commands\n");
    console->refresh();
}

inline void refreshConsole() {
    console->refresh();
}

/**
 * @brief Process key down event send to Console
 * @param key Keys_t
 */
void keyupConsole(Keys_t key) {
#define K(ORGIN_KEY, LEFT_SHIFT, RIGHT_SHIFT, CAPITAL_ALPHA, SMALL_ALPHA)
    switch (key) {
    case KEY_ALPHA:
        break;
    case KEY_SHIFT:
        break;
    default:
        break;
    }
#undef K
}

#undef CONSW
#undef CONSH

unsigned long getFileCounts(TCHAR *path) {
    unsigned long count = 0;
    DIR dir;
    FILINFO fno;
    f_opendir(&dir, path);
    for (;;) {
        if (f_readdir(&dir, &fno) == FR_OK) {
            if (fno.fname[0] != 0) {
                count++;
            } else {
                break;
            }
        } else {
            break;
        }
    }
    f_closedir(&dir);
    return count;
}

void refreshFileNames(TCHAR *path, TCHAR **names, bool *info, unsigned long *counts) {
    DIR dir;
    FILINFO fno;
    f_opendir(&dir, path);
    for (int i = 0; i < *counts; i++) {
        if (f_readdir(&dir, &fno) == FR_OK) {
            strcat(names[i], fno.fname);
            if (fno.fattrib & AM_DIR) {
                info[i] = false;
            } else {
                info[i] = true;
            }
        } else {
            break;
        }
    }
    f_closedir(&dir);
}

void refreshDir() {
    for (int i = 0; i < *filesCount; i++) {
        free(dirItemNames[i]);
    }
    free(dirItemNames);
    free(dirItemInfos);

    if (f_opendir(&fileManagerDir, pathNow) == FR_OK) {
        if (f_readdir(&fileManagerDir, &fileInfo) == FR_OK) {
            f_closedir(&fileManagerDir);

            *filesCount = getFileCounts(pathNow);
            *pageNow = 1;
            *selectedItem = 1;
            *pageAll = *filesCount / 5 + (*filesCount % 5 == 0 ? 0 : 1);

            dirItemNames = (TCHAR **)calloc(*filesCount, sizeof(TCHAR *));
            dirItemInfos = (bool *)calloc(*filesCount, sizeof(bool));
            for (int i = 0; i < *filesCount; i++) {
                dirItemNames[i] = (TCHAR *)calloc(255, sizeof(TCHAR));
            }

            refreshFileNames(pathNow, dirItemNames, dirItemInfos, filesCount);
        }
    }
}

void getWholePath(TCHAR *ans) {
    memset(ans, 0, 512);
    struct strNode *nodeNow = pathList_firstNode;

    for (;;) {
        strcat(ans, nodeNow->str);
        if (nodeNow->next == nullptr) {
            break;
        } else {
            nodeNow = nodeNow->next;
        }
    }
}

void getSuffix(TCHAR *ret, TCHAR *filename) {
    uint16_t dot = 0;
    uint16_t len = strlen(filename);
    if (filename != nullptr && len != 0) {
        for (uint16_t i = 0; i < strlen(filename); i++) {
            if (filename[i] == '.')
                dot = i;
        }
        dot++;
        strncpy(ret, filename + dot, len - dot);
    } else {
        strcpy(ret, "");
    }
}

void UI_Task(void *) {

    // 初始化配置系统
    config_init();

    uidisp = new UI_Display(LCD_PIX_W, LCD_PIX_H, ll_disp_put_area);
    mainw = new UI_Window(NULL, NULL, MAIN_WIN_TITLE, uidisp, 0, 0, LCD_PIX_W, LCD_PIX_H);

    // 先初始化文件系统
    checkFS();
    
    // 然后加载配置文件
    config_load();

    // 使用配置系统中的语言设置
    UI_SetLang(config_get_language());
    
    // 应用电源模式配置
    char power_save_mode = config_get_power_save();
    if (power_save_mode == 'S') {
        ll_cpu_slowdown_enable(1);
    } else if (power_save_mode == 'L') {
        ll_cpu_slowdown_enable(2);
    } else {
        ll_cpu_slowdown_enable(0);
    }
    
    // 应用充电状态配置
    if (config_get_enable_charge()) {
        ll_charge_enable(true);
    } else {
        ll_charge_enable(false);
    }
    
    // 应用内存交换配置
    if (config_get_enable_mem_swap()) {
        enableMemSwap(true);
    }

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

            if (UIForceRefresh) {
                mainw->refreshWindow();
                drawPage(curPage);
                UIForceRefresh = false;
                
                // 在界面刷新时保存配置
                config_save();
            }
        }
    }
}

#ifdef __cplusplus
}
#endif