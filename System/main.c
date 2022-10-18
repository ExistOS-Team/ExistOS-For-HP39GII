

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>

#include "keyboard.h"
#include "llapi_code.h"
#include "sys_llapi.h"

#include "SysConf.h"

#include "FreeRTOS.h"
#include "task.h"

//#include "ff.h"

#include "lv_conf.h"
#define LV_CONF_INCLUDE_SIMPLE
#include "lvgl.h"

#include "debug.h"

#include "SystemFs.h"
#include "SystemUI.h"

#include "VROMLoader.h"

#include "Fatfs/ff.h"
//#include "mpy_port.h"

volatile unsigned long ulHighFrequencyTimerTicks;

char pcWriteBuffer[4096];
void printTaskList() {
    vTaskList((char *)&pcWriteBuffer);
    printf("=============SYSTEM STATUS=================\r\n");
    printf("Task Name         Task Status   Priority   Stack   ID\n");
    printf("%s\n", pcWriteBuffer);
    printf("Task Name                Running Count         CPU %%\n");
    vTaskGetRunTimeStats((char *)&pcWriteBuffer);
    printf("%s\n", pcWriteBuffer);
    printf("Status:  X-Running  R-Ready  B-Block  S-Suspend  D-Delete\n");
    printf("Free memory:   %d Bytes\n", (unsigned int)xPortGetFreeHeapSize());
}

void vTask1(void *par1) {
    while (1) {
        printTaskList();
        vTaskDelay(pdMS_TO_TICKS(5678));
    }
}

void softDelayMs(uint32_t ms) {
    uint32_t cur = ll_get_time_ms();
    while ((ll_get_time_ms() - cur) < ms) {
        ;
    }
}

void vTask2(void *par1) {
    uint32_t ticks = 0;
    while (1) {
        printf("SYS Run Time: %d s\n", ticks);
        ticks++;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void vApplicationIdleHook(void) {
    ll_system_idle();
}

void exp_exec(void *par);
void emu48Btn(lv_event_t *e);
void khicasBtn(lv_event_t *e);
bool OS_UISuspend = false;
static lv_obj_t *screen;
static lv_obj_t *win;

static lv_group_t *group_null;
static lv_group_t *group_tabview;
static lv_group_t *group_apps;
static lv_group_t *group_files;
static lv_group_t *group_status;
static lv_group_t *group_msgbox;
static lv_group_t *group_imgexp;

static bool time_lable_refresh = true;
static lv_obj_t *label_time;

static lv_style_t style_screen_list_1_extra_btns_main_default;

#define EMU_DATA_PORT ((volatile uint8_t *)0x20000000)
extern bool g_system_in_emulator;

void check_emulator_status() {

    if (g_system_in_emulator) {
        if (EMU_DATA_PORT[0]) {
            FIL savef;
            FRESULT fr;
            UINT br;
            char *fname = (char *)&EMU_DATA_PORT[10];
            uint32_t fsz = ((uint32_t *)(&EMU_DATA_PORT[4]))[0];
            printf("File send command detected.\n");
            printf("Receive file:%s\n", fname);
            printf("file size:%d\n", fsz);

            fname--;
            *fname = '/';
            fr = f_open(&savef, fname, FA_CREATE_ALWAYS | FA_WRITE);
            if (fr) {
                printf("Failed to create file:%s\n", fname);
            } else {
                f_write(&savef, (const void *)&EMU_DATA_PORT[200], fsz, &br);
                printf("File wrote to:%s, wsz:%d\n", fname, br);
                f_close(&savef);

                char *testname = fname + 1;
                while (*testname) {
                    if (
                        (testname[0] == '.') &&
                        (testname[1] == 'e') &&
                        (testname[2] == 'x') &&
                        (testname[3] == 'p') &&
                        (testname[4] == 0)) {
                        xTaskCreate(exp_exec, fname + 1, configMINIMAL_STACK_SIZE, fname, configMAX_PRIORITIES - 3, NULL);
                    }
                    testname++;
                }
            }

            EMU_DATA_PORT[0] = 0;
        }
    }
}

void draw_main_win() {

    screen = lv_obj_create(lv_scr_act());

    lv_obj_set_scrollbar_mode(lv_scr_act(), LV_SCROLLBAR_MODE_OFF);

    static lv_style_t style_screen_main_main_default;
    if (style_screen_main_main_default.prop_cnt > 1)
        lv_style_reset(&style_screen_main_main_default);
    else
        lv_style_init(&style_screen_main_main_default);
    lv_style_set_bg_color(&style_screen_main_main_default, lv_color_black());
    lv_style_set_bg_opa(&style_screen_main_main_default, LV_OPA_40);
    lv_obj_add_style(screen, &style_screen_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_x(screen, 0);
    lv_obj_set_y(screen, 0);
    lv_obj_set_size(screen, 256, 128);

    win = lv_win_create(lv_scr_act(), 13);
    lv_win_add_title(win, "Exist OS");
    lv_obj_set_pos(win, 2, 2);
    lv_obj_set_size(win, 252, 125);

    static lv_style_t style_screen_win_1_main_main_default;
    if (style_screen_win_1_main_main_default.prop_cnt > 1)
        lv_style_reset(&style_screen_win_1_main_main_default);
    else
        lv_style_init(&style_screen_win_1_main_main_default);
    lv_style_set_bg_color(&style_screen_win_1_main_main_default, lv_color_make(0xee, 0xee, 0xf6));
    lv_style_set_bg_grad_color(&style_screen_win_1_main_main_default, lv_color_make(0xee, 0xee, 0xf6));
    lv_style_set_bg_grad_dir(&style_screen_win_1_main_main_default, LV_GRAD_DIR_NONE);
    lv_style_set_bg_opa(&style_screen_win_1_main_main_default, 255);
    lv_style_set_outline_color(&style_screen_win_1_main_main_default, lv_color_make(0x08, 0x1A, 0x0F));
    lv_style_set_outline_width(&style_screen_win_1_main_main_default, 1);
    lv_style_set_outline_opa(&style_screen_win_1_main_main_default, 255);
    lv_obj_add_style(win, &style_screen_win_1_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write style state: LV_STATE_DEFAULT for style_screen_win_1_extra_content_main_default
    static lv_style_t style_screen_win_1_extra_content_main_default;
    if (style_screen_win_1_extra_content_main_default.prop_cnt > 1)
        lv_style_reset(&style_screen_win_1_extra_content_main_default);
    else
        lv_style_init(&style_screen_win_1_extra_content_main_default);
    lv_style_set_bg_color(&style_screen_win_1_extra_content_main_default, lv_color_make(0xee, 0xee, 0xf6));
    lv_style_set_bg_grad_color(&style_screen_win_1_extra_content_main_default, lv_color_make(0xee, 0xee, 0xf6));
    lv_style_set_bg_grad_dir(&style_screen_win_1_extra_content_main_default, LV_GRAD_DIR_NONE);
    lv_style_set_bg_opa(&style_screen_win_1_extra_content_main_default, 255);
    lv_style_set_text_color(&style_screen_win_1_extra_content_main_default, lv_color_make(0x39, 0x3c, 0x41));
    lv_style_set_text_font(&style_screen_win_1_extra_content_main_default, &lv_font_montserrat_12);
    lv_style_set_text_letter_space(&style_screen_win_1_extra_content_main_default, 0);
    lv_style_set_text_line_space(&style_screen_win_1_extra_content_main_default, 2);
    lv_obj_add_style(lv_win_get_content(win), &style_screen_win_1_extra_content_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write style state: LV_STATE_DEFAULT for style_screen_win_1_extra_header_main_default
    static lv_style_t style_screen_win_1_extra_header_main_default;
    if (style_screen_win_1_extra_header_main_default.prop_cnt > 1)
        lv_style_reset(&style_screen_win_1_extra_header_main_default);
    else
        lv_style_init(&style_screen_win_1_extra_header_main_default);

    lv_style_set_bg_color(&style_screen_win_1_extra_header_main_default, lv_color_make(0x00, 0x00, 0x00));
    lv_style_set_bg_grad_color(&style_screen_win_1_extra_header_main_default, lv_color_make(0xff, 0xff, 0xff));

    lv_style_set_bg_main_stop(&style_screen_win_1_extra_header_main_default, 0);
    lv_style_set_bg_grad_stop(&style_screen_win_1_extra_header_main_default, 255);

    lv_style_set_bg_grad_dir(&style_screen_win_1_extra_header_main_default, LV_GRAD_DIR_HOR);
    lv_style_set_bg_opa(&style_screen_win_1_extra_header_main_default, 255);
    lv_style_set_text_color(&style_screen_win_1_extra_header_main_default, lv_color_make(0xff, 0xff, 0xff));
    lv_style_set_text_font(&style_screen_win_1_extra_header_main_default, &lv_font_montserrat_12);
    lv_style_set_text_letter_space(&style_screen_win_1_extra_header_main_default, 0);
    lv_style_set_text_line_space(&style_screen_win_1_extra_header_main_default, 2);
    lv_obj_add_style(lv_win_get_header(win), &style_screen_win_1_extra_header_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write style state: LV_STATE_DEFAULT for style_screen_win_1_extra_btns_main_default
    static lv_style_t style_screen_win_1_extra_btns_main_default;
    if (style_screen_win_1_extra_btns_main_default.prop_cnt > 1)
        lv_style_reset(&style_screen_win_1_extra_btns_main_default);
    else
        lv_style_init(&style_screen_win_1_extra_btns_main_default);
    lv_style_set_radius(&style_screen_win_1_extra_btns_main_default, 8);
    lv_style_set_bg_color(&style_screen_win_1_extra_btns_main_default, lv_color_make(0x21, 0x95, 0xf6));
    lv_style_set_bg_grad_color(&style_screen_win_1_extra_btns_main_default, lv_color_make(0x21, 0x95, 0xf6));
    lv_style_set_bg_grad_dir(&style_screen_win_1_extra_btns_main_default, LV_GRAD_DIR_NONE);
    lv_style_set_bg_opa(&style_screen_win_1_extra_btns_main_default, 255);
    lv_obj_t *screen_win_1_btn;
    lv_obj_t *screen_win_1_label = lv_label_create(lv_win_get_content(win));
    lv_label_set_text(screen_win_1_label, "");

    /*
        obj = lv_textarea_create(lv_scr_act());
        lv_textarea_add_text(obj, "字体：思源黑体 Light\n 字号：11\n");
        lv_textarea_add_text(obj, te);
        lv_textarea_set_align(obj, LV_TEXT_ALIGN_LEFT);
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
    */

    // char* argv[] = {"gb", "test.gb", NULL};
    // int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    // extern int gb_main(int argc, char *argv[]);
    // gb_main(argc, argv);

    // SystemTest();

    // lv_scr_act();

    // void mpy_main();

    // mpy_main();
}

void gb_main(void *_);

static lv_obj_t *title, *tv, *t1, *t2, *t3, *imgbtn, *imgbtn2;
static lv_obj_t *file_list;

static DIR dp;
static FILINFO finfo;

static char FileExplorerPwd[256];
static char FilePath[256];
static char LvFilePath[256];
static char FileExt[12];

static void FileExplorerRefresh(char *path);

static void tabview_switch(uint32_t key) {
    switch (key) {
    case KEY_F1 | 0xE000:
        lv_tabview_set_act(tv, 0, false);
        lv_indev_set_group(SystemGetInKeypad(), group_apps);

        break;
    case KEY_F2 | 0xE000:
        lv_tabview_set_act(tv, 1, false);
        lv_indev_set_group(SystemGetInKeypad(), group_files);
        FileExplorerRefresh(FileExplorerPwd);

        break;
    case KEY_F3 | 0xE000:
        lv_tabview_set_act(tv, 2, false);

        lv_indev_set_group(SystemGetInKeypad(), group_status);
        break;

    case LV_KEY_LEFT:
        lv_group_focus_prev(SystemGetInKeypad()->group);
        break;
    case LV_KEY_RIGHT:
        lv_group_focus_next(SystemGetInKeypad()->group);

        break;

    default:
        break;
    }
}

static lv_obj_t *charge_chb, *slow_down_chb, *load_hash_err_app;

static void charge_msgbox_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *msgbox = lv_event_get_current_target(e);

    if (code == LV_EVENT_VALUE_CHANGED) {
        uint16_t select_btn = lv_msgbox_get_active_btn(msgbox); // 0 or 1

        if (select_btn == 0) {
            ll_charge_enable(true);

        } else {
            ll_charge_enable(false);
            lv_obj_clear_state(charge_chb, LV_STATE_CHECKED);
        }

        lv_indev_set_group(SystemGetInKeypad(), group_status);
        lv_msgbox_close_async(msgbox);
    }
}

static void slowdown_chb_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_VALUE_CHANGED) {
        // const char * txt = lv_checkbox_get_text(obj);

        if (lv_obj_get_state(obj) & LV_STATE_CHECKED) {
            ll_cpu_slowdown_enable(true);
        } else {
            ll_cpu_slowdown_enable(false);
        }
    } else if (code == LV_EVENT_KEY) {
        uint32_t key = lv_event_get_key(e);
        tabview_switch(key);
    }
}

bool g_allow_load_err_app = false;
static void loaderrapp_chb_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_VALUE_CHANGED) {
        // const char * txt = lv_checkbox_get_text(obj);

        if (lv_obj_get_state(obj) & LV_STATE_CHECKED) {
            g_allow_load_err_app = (true);
        } else {
            g_allow_load_err_app = (false);
        }
    } else if (code == LV_EVENT_KEY) {
        uint32_t key = lv_event_get_key(e);
        tabview_switch(key);
    }
}

static void charge_chb_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    static char *msgbox_button[] = {"OK", "Cancel", ""};

    if (code == LV_EVENT_VALUE_CHANGED) {
        // const char * txt = lv_checkbox_get_text(obj);

        if (lv_obj_get_state(obj) & LV_STATE_CHECKED) {
            lv_group_remove_all_objs(group_msgbox);
            lv_group_set_default(group_msgbox);

            lv_obj_t *mbox = lv_msgbox_create(lv_scr_act(), "!!! Warning !!!", "[Experimental Features]\nPLEASE make sure to use \n1.2 V Rechargeable Battery, i.e. NiCd, NiMH, etc.", (const char **)msgbox_button, false);

            lv_obj_add_event_cb(mbox, charge_msgbox_event_cb, LV_EVENT_ALL, NULL);
            lv_obj_align(mbox, LV_ALIGN_CENTER, 0, 0);
            lv_obj_center(mbox);
            lv_obj_set_size(mbox, 240, 120);

            // group_null = SystemGetInKeypad()->group;
            lv_indev_set_group(SystemGetInKeypad(), group_msgbox);

        } else {
            ll_charge_enable(false);
        }

    } else if (code == LV_EVENT_KEY) {
        uint32_t key = lv_event_get_key(e);
        tabview_switch(key);
    }
}

lv_obj_t *label_cpuminirac;
void slider_cpu_minimal_frac_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    if (code == LV_EVENT_KEY) {
        uint32_t key = lv_event_get_key(e);
        tabview_switch(key);
    } else {
        lv_obj_t *slider = lv_event_get_target(e);
        char buf[32];
        int val = (int)lv_slider_get_value(slider);
        lv_snprintf(buf, sizeof(buf), "CPU Freq Minimal Frac: %d", val);
        lv_label_set_text(label_cpuminirac, buf);
        ll_cpu_slowdown_min_frac(val);
    }
}
/*
static lv_obj_t *imgexp;
static void imgexp_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_KEY)
    {
        uint32_t key = lv_event_get_key(e);
        switch (key)
        {
        case LV_KEY_ESC:

            lv_indev_set_group(SystemGetInKeypad(), group_files);

            time_lable_refresh = true;
            lv_obj_del_delayed(obj, 10);
            break;

        default:
            break;
        }
    }else if(code == LV_EVENT_DRAW_MAIN_BEGIN)
    {
        SystemUISetBusy(true);
    }else if(code == LV_EVENT_DRAW_MAIN_END)
    {
        SystemUISetBusy(false);
        lv_img_set_src(obj, NULL);
    }
}
*/

static void fexplorer_file_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED) {
        const char *fname = lv_list_get_btn_text(file_list, obj);
        int i = strlen(fname);
        memset(FileExt, 0, sizeof(FileExt));
        memset(FilePath, 0, sizeof(FilePath));
        memset(LvFilePath, 0, sizeof(LvFilePath));
        while (i > 0) {
            i--;
            if (fname[i] == '.') {
                strcpy(FileExt, &fname[i + 1]);
                break;
            }
        }

        strcat(FilePath, FileExplorerPwd);

        // printf("NotProcPath:%s\n", FilePath);
        for (int fpPtr = 1; fpPtr < 255; fpPtr++) {
            if (FilePath[fpPtr] == 0) {
                if (FilePath[fpPtr - 1] != '/') {
                    strcat(FilePath, "/");
                }
                break;
            }
        }
        // printf("fpath1:%s\n", FilePath);

        strcat(FilePath, fname);
        // printf("fpathAndFName1:%s\n", FilePath);

        strcat(LvFilePath, "A:");
        strcat(LvFilePath, FilePath);

        /*
        if((strcmp(FileExt,"gif") == 0))
        {
            printf("fpath:%s\n", LvFilePath);
            lv_group_remove_all_objs(group_imgexp);
            lv_group_set_default(group_imgexp);
            imgexp = lv_gif_create(lv_scr_act());//lv_img_create(lv_scr_act());
            lv_obj_add_event_cb(imgexp, imgexp_handler, LV_EVENT_ALL, NULL);
            time_lable_refresh = false;
            lv_gif_set_src(imgexp, LvFilePath);

            lv_obj_align(imgexp, LV_ALIGN_CENTER, 0, 0);
            lv_obj_set_size(imgexp, 256, 127);

            lv_group_add_obj(group_imgexp, imgexp);

            //group_null = SystemGetInKeypad()->group;
            lv_indev_set_group(SystemGetInKeypad(), group_imgexp);
        }else*/
        if ((strcmp(FileExt, "jpg") == 0)) {
            printf("fpath:%s\n", FilePath);

            extern void jpgViewer(void *par);
            xTaskCreate(jpgViewer, "jpgViewer", configMINIMAL_STACK_SIZE, FilePath, configMAX_PRIORITIES - 3, NULL);
        } else if ((strcmp(FileExt, "avi") == 0)) {
            printf("fpath:%s\n", FilePath);

            extern void mjpegPlayer(void *par);
            xTaskCreate(mjpegPlayer, "mjpegPlayer", configMINIMAL_STACK_SIZE, FilePath, configMAX_PRIORITIES - 3, NULL);
        } else if ((strcmp(FileExt, "bin") == 0)) {
            void bin_exec(void *par);
            xTaskCreate(bin_exec, fname, 65535, FilePath, configMAX_PRIORITIES - 3, NULL);
        } else if ((strcmp(FileExt, "exp") == 0)) {
            xTaskCreate(exp_exec, fname, configMINIMAL_STACK_SIZE, FilePath, configMAX_PRIORITIES - 3, NULL);
        } else if ((strcmp(FileExt, "wav") == 0))
        {
           extern void wavPlayer(void *par);
            xTaskCreate(wavPlayer, "wavPlayer", configMINIMAL_STACK_SIZE, FilePath, configMAX_PRIORITIES - 3, NULL);
        }

    } else if (code == LV_EVENT_KEY) {
        uint32_t key = lv_event_get_key(e);
        tabview_switch(key);
    }
}

static void fexplorer_dir_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED) {
        if (strcmp(lv_list_get_btn_text(file_list, obj), "..") == 0) {
            int i = strlen(FileExplorerPwd);

            for (; i >= 0; i--) {
                if (FileExplorerPwd[i] == '/') {
                    if (i == 0) {
                        FileExplorerPwd[++i] = 0;
                        break;
                    } else {
                        FileExplorerPwd[i] = 0;
                        break;
                    }
                }
            }

        } else {
            if (FileExplorerPwd[strlen(FileExplorerPwd) - 1] != '/')
                strcat(FileExplorerPwd, "/");

            strcat(FileExplorerPwd, lv_list_get_btn_text(file_list, obj));
        }
        FileExplorerRefresh(FileExplorerPwd);
    } else if (code == LV_EVENT_KEY) {
        uint32_t key = lv_event_get_key(e);
        tabview_switch(key);
    }
}

static void FileExplorerRefresh(char *path) {
    bool firstItem = true;

    lv_obj_clean(file_list);

    lv_obj_t *screen_list_1_btn;

    FRESULT fr;

    screen_list_1_btn = lv_list_add_text(file_list, FileExplorerPwd);
    lv_group_add_obj(group_files, screen_list_1_btn);

    lv_obj_add_flag(screen_list_1_btn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

    if (strcmp(path, "/") != 0) {
        screen_list_1_btn = lv_list_add_btn(file_list, LV_SYMBOL_DIRECTORY, "..");
        lv_group_add_obj(group_files, screen_list_1_btn);
        lv_obj_add_event_cb(screen_list_1_btn, fexplorer_dir_handler, LV_EVENT_ALL, NULL);
        lv_group_focus_obj(screen_list_1_btn);
        firstItem = false;
    }

    fr = f_findfirst(&dp, &finfo, path, "*");
    if ((fr) && (finfo.fname[0] == 0)) {
        goto find_fin;
    }

    while (fr == 0 && (finfo.fname[0])) {
        if (finfo.fattrib == AM_DIR) {
            screen_list_1_btn = lv_list_add_btn(file_list, LV_SYMBOL_DIRECTORY, finfo.fname);
            lv_obj_add_event_cb(screen_list_1_btn, fexplorer_dir_handler, LV_EVENT_ALL, NULL);
        } else {
            screen_list_1_btn = lv_list_add_btn(file_list, LV_SYMBOL_FILE, finfo.fname);
            lv_obj_add_event_cb(screen_list_1_btn, fexplorer_file_handler, LV_EVENT_ALL, NULL);
        }

        lv_obj_add_style(screen_list_1_btn, &style_screen_list_1_extra_btns_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_group_add_obj(group_files, screen_list_1_btn);

        if (firstItem) {
            lv_group_focus_obj(screen_list_1_btn);
            firstItem = false;
        }

        fr = f_findnext(&dp, &finfo);
    }

find_fin:
    return;
}

static void FileExplorerCreate() {

    if (style_screen_list_1_extra_btns_main_default.prop_cnt > 1)
        lv_style_reset(&style_screen_list_1_extra_btns_main_default);
    else
        lv_style_init(&style_screen_list_1_extra_btns_main_default);
    lv_style_set_radius(&style_screen_list_1_extra_btns_main_default, 3);
    lv_style_set_bg_color(&style_screen_list_1_extra_btns_main_default, lv_color_make(0xff, 0xff, 0xff));
    lv_style_set_bg_grad_color(&style_screen_list_1_extra_btns_main_default, lv_color_make(0xff, 0xff, 0xff));
    lv_style_set_bg_grad_dir(&style_screen_list_1_extra_btns_main_default, LV_GRAD_DIR_NONE);
    lv_style_set_bg_opa(&style_screen_list_1_extra_btns_main_default, 255);
    lv_style_set_text_color(&style_screen_list_1_extra_btns_main_default, lv_color_make(0x0D, 0x30, 0x55));
    lv_style_set_text_font(&style_screen_list_1_extra_btns_main_default, &SourceHanSans11);

    file_list = lv_list_create(t3);
    lv_obj_set_pos(file_list, 0, 0);
    lv_obj_set_size(file_list, 230, 100);
    lv_obj_set_scrollbar_mode(file_list, LV_SCROLLBAR_MODE_OFF);

    static lv_style_t style_screen_list_1_main_main_default;
    if (style_screen_list_1_main_main_default.prop_cnt > 1)
        lv_style_reset(&style_screen_list_1_main_main_default);
    else
        lv_style_init(&style_screen_list_1_main_main_default);
    lv_style_set_radius(&style_screen_list_1_main_main_default, 0);
    lv_style_set_bg_color(&style_screen_list_1_main_main_default, lv_color_make(0xff, 0xff, 0xff));
    lv_style_set_bg_grad_color(&style_screen_list_1_main_main_default, lv_color_make(0xff, 0xff, 0xff));
    lv_style_set_bg_grad_dir(&style_screen_list_1_main_main_default, LV_GRAD_DIR_NONE);
    lv_style_set_bg_opa(&style_screen_list_1_main_main_default, 255);
    lv_style_set_border_color(&style_screen_list_1_main_main_default, lv_color_make(0xe1, 0xe6, 0xee));
    lv_style_set_border_width(&style_screen_list_1_main_main_default, 0);
    lv_style_set_border_opa(&style_screen_list_1_main_main_default, 255);
    lv_style_set_pad_left(&style_screen_list_1_main_main_default, 0);
    lv_style_set_pad_right(&style_screen_list_1_main_main_default, 0);
    lv_style_set_pad_top(&style_screen_list_1_main_main_default, 0);
    lv_style_set_pad_bottom(&style_screen_list_1_main_main_default, 0);
    lv_obj_add_style(file_list, &style_screen_list_1_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);

    strcpy(FileExplorerPwd, "/");

    // lv_group_add_obj(group_files, file_list);
}

static void tabview_event(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    if (code == LV_EVENT_KEY) {
        uint32_t key = lv_event_get_key(e);
        tabview_switch(key);
    }
}

static void status_label_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    if (code == LV_EVENT_KEY) {
        uint32_t key = lv_event_get_key(e);
        tabview_switch(key);
    }
}

static lv_obj_t *emu48_msgbox;
const char *msgboxbtn[] = {""};

static void emu48_msgbox_event_cb(lv_event_t *e) {

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *msgbox = lv_event_get_current_target(e);

    if (code == LV_EVENT_DELETE) {
        lv_indev_set_group(SystemGetInKeypad(), group_apps);
    }
}

void khicasTask(void *arg) {
    lv_obj_t *win = lv_win_create(lv_scr_act(), 0);
    lv_obj_t *cont = lv_win_get_content(win);
    lv_obj_t *text = lv_label_create(cont);

    lv_label_set_text(text, "Loading...");
    vTaskDelay(pdMS_TO_TICKS(1000));

    lv_obj_del_async(win);

    SystemUISuspend();
    void testcpp();
    testcpp();

    SystemUIResume();

    vTaskDelete(NULL);
}

static void app_btn_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_t *label = lv_obj_get_child(obj, 0);
        if (label) {
            if (strcmp(lv_label_get_text(label), "KhiCAS") == 0) {
                xTaskCreate(khicasTask, "KhiCAS", 16384, NULL, configMAX_PRIORITIES - 3, NULL);
            } else if (strcmp(lv_label_get_text(label), "Emu48") == 0) {
                FIL *f = pvPortMalloc(sizeof(FIL));
                FRESULT fr;
                fr = f_open(f, "/rom.39g", FA_OPEN_EXISTING);
                if (fr == FR_OK) {
                    f_close(f);
                    vPortFree(f);
                    lv_obj_t *win = lv_win_create(lv_scr_act(), 0);
                    lv_obj_t *cont = lv_win_get_content(win);
                    lv_obj_t *text = lv_label_create(cont);
                    lv_label_set_text(text, "Loading...");
                    // vTaskDelay(pdMS_TO_TICKS(1000));
                    void emu48_task(void *_);
                    xTaskCreate(emu48_task, "emu48", 16384, NULL, configMAX_PRIORITIES - 3, NULL);

                } else {
                    vPortFree(f);
                    lv_group_remove_all_objs(group_msgbox);
                    lv_group_set_default(group_msgbox);

                    emu48_msgbox = lv_msgbox_create(lv_scr_act(), "Error", "Could not find the ROM: /rom.39g", msgboxbtn, true);
                    lv_obj_add_event_cb(emu48_msgbox, emu48_msgbox_event_cb, LV_EVENT_DELETE, 0);
                    lv_obj_align(emu48_msgbox, LV_ALIGN_CENTER, 0, 0);
                    lv_obj_center(emu48_msgbox);

                    // group_null = SystemGetInKeypad()->group;
                    lv_indev_set_group(SystemGetInKeypad(), group_msgbox);
                }

            } /*else if(strcmp(lv_label_get_text(label), "GameBoy") == 0)
             {
                 xTaskCreate(gb_main, "GB Emu", 16384, NULL, configMAX_PRIORITIES - 3, NULL);
             }*/
            else if (strcmp(lv_label_get_text(label), "Test") == 0) {
                printf("testRead:%08x\n", *((uint32_t *)(0x03000000)));

                FIL *f;
                FRESULT fr;
                UINT br;

                f = pvPortMalloc(sizeof(FIL));
                fr = f_open(f, "/test.txt", FA_WRITE | FA_CREATE_ALWAYS | FA_READ);
                printf("fr:%d\n", fr);
                fr = f_printf(f, "hello world!\n");
                printf("fr:%d\n", fr);
                fr = f_write(f, "tetea", 6, &br);
                printf("fr:%d, %d\n", fr, br);
                f_close(f);
                vPortFree(f);
            }
        }
    } else if (code == LV_EVENT_KEY) {
        uint32_t key = lv_event_get_key(e);
        tabview_switch(key);
    }
}

void main_thread() {

    // printf("R13:%08x\n", get_stack());

    void SystemUIInit();
    SystemUIInit();
    SystemFSInit();

    /*
    lv_obj_t * img1 = lv_img_create(lv_scr_act());
    lv_img_set_src(img1, &existos_logo_grey2);
    lv_obj_align(img1, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_size(img1, 256, 128);
    for(;;)vTaskDelay(pdMS_TO_TICKS(1000)); */
    /*
        ll_cpu_slowdown_enable(false);
        extern void doom_main(int argc, char *argv[]);

        char *argv[] = {"doom", "-iwad", "doom.wad", NULL};
        int argc = sizeof(argv) / sizeof(argv[0]) - 1;

        doom_main(argc, argv);

        for(;;)
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }*/

    draw_main_win();

    lv_obj_t *cont = lv_scr_act();

    // title = lv_label_create(cont);
    // lv_label_set_text(title, "Exist OS");

    {
        group_tabview = lv_group_create();
        group_apps = lv_group_create();
        group_files = lv_group_create();
        group_status = lv_group_create();
        group_null = lv_group_create();
        group_msgbox = lv_group_create();
        group_imgexp = lv_group_create();

        lv_indev_set_group(SystemGetInKeypad(), group_apps);
        tv = lv_tabview_create(cont, LV_DIR_TOP, LV_DPI_DEF / 4);
        lv_obj_set_pos(tv, 4, 15);
        lv_obj_set_size(tv, 248, 109);

        t1 = lv_tabview_add_tab(tv, "Apps(F1)");
        t3 = lv_tabview_add_tab(tv, "Files(F2)");
        t2 = lv_tabview_add_tab(tv, "Status(F3)");

        lv_group_add_obj(group_tabview, tv);
        // lv_group_add_obj(group_apps, tv);

        lv_obj_add_event_cb(tv, tabview_event, LV_EVENT_VALUE_CHANGED, NULL);

        lv_obj_add_event_cb(tv, tabview_event, LV_EVENT_KEY, NULL);

        LV_IMG_DECLARE(xcaslogo_s);
        imgbtn = lv_imgbtn_create(t1);

        lv_obj_set_scrollbar_mode(imgbtn, LV_SCROLLBAR_MODE_OFF);

        lv_obj_set_size(imgbtn, 48, 48);
        lv_obj_set_pos(imgbtn, 12, 2);

        static lv_style_t style_screen_imgbtn_1_main_main_default;
        if (style_screen_imgbtn_1_main_main_default.prop_cnt > 1)
            lv_style_reset(&style_screen_imgbtn_1_main_main_default);
        else
            lv_style_init(&style_screen_imgbtn_1_main_main_default);
        lv_style_set_shadow_width(&style_screen_imgbtn_1_main_main_default, 2);
        lv_style_set_shadow_color(&style_screen_imgbtn_1_main_main_default, lv_color_make(0x00, 0x00, 0x00));
        lv_style_set_shadow_opa(&style_screen_imgbtn_1_main_main_default, 128);
        lv_style_set_shadow_spread(&style_screen_imgbtn_1_main_main_default, 2);
        lv_style_set_shadow_ofs_x(&style_screen_imgbtn_1_main_main_default, 0);
        lv_style_set_shadow_ofs_y(&style_screen_imgbtn_1_main_main_default, 0);
        lv_style_set_text_color(&style_screen_imgbtn_1_main_main_default, lv_color_make(0x00, 0x00, 0x00));
        lv_style_set_text_align(&style_screen_imgbtn_1_main_main_default, LV_TEXT_ALIGN_CENTER);
        lv_style_set_img_recolor(&style_screen_imgbtn_1_main_main_default, lv_color_make(0xff, 0xff, 0xff));
        lv_style_set_img_recolor_opa(&style_screen_imgbtn_1_main_main_default, 0);
        lv_style_set_img_opa(&style_screen_imgbtn_1_main_main_default, 255);
        lv_obj_add_style(imgbtn, &style_screen_imgbtn_1_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);

        // Write style state: LV_STATE_PRESSED for style_screen_imgbtn_1_main_main_pressed
        /*
            static lv_style_t style_screen_imgbtn_1_main_main_pressed;
            if (style_screen_imgbtn_1_main_main_pressed.prop_cnt > 1)
                    lv_style_reset(&style_screen_imgbtn_1_main_main_pressed);
            else
                    lv_style_init(&style_screen_imgbtn_1_main_main_pressed);
            lv_style_set_text_color(&style_screen_imgbtn_1_main_main_pressed, lv_color_make(0xFF, 0x33, 0xFF));
            lv_style_set_text_align(&style_screen_imgbtn_1_main_main_pressed, LV_TEXT_ALIGN_CENTER);
            lv_style_set_img_recolor(&style_screen_imgbtn_1_main_main_pressed, lv_color_make(0x00, 0x00, 0x00));
            lv_style_set_img_recolor_opa(&style_screen_imgbtn_1_main_main_pressed, 0);
            lv_style_set_img_opa(&style_screen_imgbtn_1_main_main_pressed, 255);
            lv_obj_add_style(imgbtn, &style_screen_imgbtn_1_main_main_pressed, LV_PART_MAIN|LV_STATE_PRESSED);
    */
        /*
                //Write style state: LV_STATE_CHECKED for style_screen_imgbtn_1_main_main_checked
                static lv_style_t style_screen_imgbtn_1_main_main_checked;
                if (style_screen_imgbtn_1_main_main_checked.prop_cnt > 1)
                        lv_style_reset(&style_screen_imgbtn_1_main_main_checked);
                else
                        lv_style_init(&style_screen_imgbtn_1_main_main_checked);
                lv_style_set_text_color(&style_screen_imgbtn_1_main_main_checked, lv_color_make(0xFF, 0x33, 0xFF));
                lv_style_set_text_align(&style_screen_imgbtn_1_main_main_checked, LV_TEXT_ALIGN_CENTER);
                lv_style_set_img_recolor(&style_screen_imgbtn_1_main_main_checked, lv_color_make(0x00, 0x00, 0x00));
                lv_style_set_img_recolor_opa(&style_screen_imgbtn_1_main_main_checked, 0);
                lv_style_set_img_opa(&style_screen_imgbtn_1_main_main_checked, 255);
                lv_obj_add_style(imgbtn, &style_screen_imgbtn_1_main_main_checked, LV_PART_MAIN|LV_STATE_CHECKED);
        */

        LV_IMG_DECLARE(emu48ico);

        lv_imgbtn_set_src(imgbtn, LV_IMGBTN_STATE_RELEASED, NULL, &xcaslogo_s, NULL);
        lv_obj_add_flag(imgbtn, LV_OBJ_FLAG_CHECKABLE);

        lv_obj_t *btn1;
        btn1 = lv_btn_create(t1);
        lv_obj_set_size(btn1, 48, 13);
        lv_obj_set_pos(btn1, 12, 52);
        lv_obj_t *label = lv_label_create(btn1);

        static lv_style_t style_screen_btn_1_main_main_default;
        if (style_screen_btn_1_main_main_default.prop_cnt > 1)
            lv_style_reset(&style_screen_btn_1_main_main_default);
        else
            lv_style_init(&style_screen_btn_1_main_main_default);
        lv_style_set_radius(&style_screen_btn_1_main_main_default, 2);
        lv_style_set_bg_color(&style_screen_btn_1_main_main_default, lv_color_make(0xff, 0xff, 0xff));
        lv_style_set_bg_grad_color(&style_screen_btn_1_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
        lv_style_set_bg_grad_dir(&style_screen_btn_1_main_main_default, LV_GRAD_DIR_NONE);
        lv_style_set_bg_opa(&style_screen_btn_1_main_main_default, 255);
        lv_style_set_border_color(&style_screen_btn_1_main_main_default, lv_color_make(0x00, 0x00, 0x00));
        lv_style_set_border_width(&style_screen_btn_1_main_main_default, 0);
        lv_style_set_border_opa(&style_screen_btn_1_main_main_default, 100);
        lv_style_set_text_color(&style_screen_btn_1_main_main_default, lv_color_make(0x00, 0x00, 0x00));
        lv_style_set_text_font(&style_screen_btn_1_main_main_default, &lv_font_montserrat_12);
        lv_style_set_text_align(&style_screen_btn_1_main_main_default, LV_TEXT_ALIGN_CENTER);
        lv_obj_add_style(btn1, &style_screen_btn_1_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_label_set_text(label, "KhiCAS");
        lv_obj_set_style_pad_all(btn1, 0, LV_STATE_DEFAULT);
        lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
        lv_obj_add_event_cb(btn1, app_btn_handler, LV_EVENT_ALL, NULL);

        imgbtn2 = lv_imgbtn_create(t1);

        lv_obj_set_scrollbar_mode(imgbtn2, LV_SCROLLBAR_MODE_OFF);

        lv_obj_set_size(imgbtn2, 48, 48);
        lv_obj_set_pos(imgbtn2, 48 + 45, 2);

        lv_obj_add_style(imgbtn2, &style_screen_imgbtn_1_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_imgbtn_set_src(imgbtn2, LV_IMGBTN_STATE_RELEASED, NULL, &emu48ico, NULL);
        lv_obj_add_flag(imgbtn2, LV_OBJ_FLAG_CHECKABLE);

        lv_obj_t *btn2;
        btn2 = lv_btn_create(t1);
        lv_obj_set_size(btn2, 48, 13);
        lv_obj_set_pos(btn2, 12 + 40 * 2, 52);
        lv_obj_t *label2 = lv_label_create(btn2);
        lv_label_set_text(label2, "Emu48");
        lv_obj_set_style_pad_all(btn2, 0, LV_STATE_DEFAULT);
        lv_obj_align(label2, LV_ALIGN_CENTER, 0, 0);
        lv_obj_add_style(btn2, &style_screen_btn_1_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_add_event_cb(btn2, app_btn_handler, LV_EVENT_ALL, NULL);

        /*
            lv_obj_t *imgbtn3 = lv_imgbtn_create(t1);

                lv_obj_set_scrollbar_mode(imgbtn3, LV_SCROLLBAR_MODE_OFF);

            lv_obj_set_size(imgbtn3, 48, 48);
            lv_obj_set_pos(imgbtn3, 48 + 48 + 76, 2);

            LV_IMG_DECLARE(gbico);

                lv_obj_add_style(imgbtn3, &style_screen_imgbtn_1_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
                lv_imgbtn_set_src(imgbtn3, LV_IMGBTN_STATE_RELEASED, NULL, &gbico, NULL);
                lv_obj_add_flag(imgbtn3, LV_OBJ_FLAG_CHECKABLE);
        */

        /*
            lv_obj_t *btn3;
            btn3 = lv_btn_create(t1);
            lv_obj_set_size(btn3, 62, 13);
            lv_obj_set_pos(btn3, 60 + 40 + 64 , 52);
            lv_obj_t * label3 = lv_label_create(btn3);
                lv_label_set_text(label3, "GameBoy");
                lv_obj_set_style_pad_all(btn3, 0, LV_STATE_DEFAULT);
                lv_obj_align(label3, LV_ALIGN_CENTER, 0, 0);
                lv_obj_add_style(btn3, &style_screen_btn_1_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

            void gb_entry();
            lv_obj_add_event_cb(btn3, app_btn_handler, LV_EVENT_ALL, NULL);
        */
        lv_group_add_obj(group_apps, btn1);
        lv_group_add_obj(group_apps, btn2);
        // lv_group_add_obj(group_apps, btn3);
    }

    // lv_obj_clear_flag(btn1, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    FileExplorerCreate();

    //===============================================================
    // tv2
    // lv_obj_set_scrollbar_mode(t2, LV_SCROLLBAR_MODE_ON);

    label_time = lv_label_create(cont);
    uint32_t rtc_time_sec = ll_rtc_get_sec();

    lv_obj_set_size(label_time, 48, 13);
    lv_obj_set_pos(label_time, 0, 0);
    lv_obj_align(label_time, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_set_flex_flow(t2, LV_FLEX_FLOW_COLUMN);

    extern const char system_build_time[];

#define DEF_INFO_LABEL(label_name, fcous_able)                    \
    lv_obj_t *label_name;                                         \
    label_name = lv_label_create(t2);                             \
    lv_obj_set_flex_grow(label_name, 0);                          \
    if (fcous_able) {                                             \
        lv_group_add_obj(group_status, label_name);               \
        lv_obj_add_flag(label_name, LV_OBJ_FLAG_SCROLL_ON_FOCUS); \
    }                                                             \
    lv_obj_add_event_cb(label_name, status_label_cb, LV_EVENT_ALL, NULL);

#define SET_LABEL_TEXT(label, ...) lv_label_set_text_fmt(label, __VA_ARGS__)

    DEF_INFO_LABEL(info_line1, true);
    DEF_INFO_LABEL(info_line2, false);
    DEF_INFO_LABEL(info_line3, false);
    DEF_INFO_LABEL(info_line4, false);

    DEF_INFO_LABEL(info_line5, true);
    SET_LABEL_TEXT(info_line5, "Build time:%s", system_build_time);

    label_cpuminirac = lv_label_create(t2);
    SET_LABEL_TEXT(label_cpuminirac, "CPU Freq Minimal Frac: 6");
    lv_obj_set_flex_grow(label_cpuminirac, 0);

    lv_obj_t *cpu_minpwr_slider = lv_slider_create(t2);
    lv_obj_set_flex_grow(cpu_minpwr_slider, 0);
    lv_slider_set_range(cpu_minpwr_slider, 2, 14);
    lv_obj_add_flag(cpu_minpwr_slider, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_slider_set_value(cpu_minpwr_slider, 6, false);
    lv_obj_add_event_cb(cpu_minpwr_slider, slider_cpu_minimal_frac_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_group_add_obj(group_status, cpu_minpwr_slider);

    slow_down_chb = lv_checkbox_create(t2);
    lv_checkbox_set_text(slow_down_chb, "CPU Auto Slow-Down");
    lv_obj_set_flex_grow(slow_down_chb, 0);
    lv_obj_add_event_cb(slow_down_chb, slowdown_chb_handler, LV_EVENT_ALL, NULL);
    lv_obj_add_state(slow_down_chb, LV_STATE_CHECKED);
    lv_group_add_obj(group_status, slow_down_chb);

    charge_chb = lv_checkbox_create(t2);
    lv_checkbox_set_text(charge_chb, "Enable Charging");
    lv_obj_set_flex_grow(charge_chb, 0);
    lv_obj_add_event_cb(charge_chb, charge_chb_handler, LV_EVENT_ALL, NULL);
    lv_group_add_obj(group_status, charge_chb);

    load_hash_err_app = lv_checkbox_create(t2);
    lv_checkbox_set_text(load_hash_err_app, "Load app with hash error");
    lv_obj_set_flex_grow(load_hash_err_app, 0);
    lv_obj_add_event_cb(load_hash_err_app, loaderrapp_chb_handler, LV_EVENT_ALL, NULL);
    lv_group_add_obj(group_status, load_hash_err_app);

    lv_obj_t *btn4;
    btn4 = lv_btn_create(t2);
    lv_obj_set_size(btn4, 62, 13);
    lv_obj_set_pos(btn4, 60 + 40 + 64, 52);
    lv_obj_t *label4 = lv_label_create(btn4);
    lv_label_set_text(label4, "Test");
    lv_obj_set_style_pad_all(btn4, 0, LV_STATE_DEFAULT);
    lv_obj_align(label4, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(btn4, app_btn_handler, LV_EVENT_ALL, NULL);
    lv_group_add_obj(group_status, btn4);

    // lv_demo_keypad_encoder();
    int cur_cpu_div = 1;
    int cur_cpu_frac = 25;
    int cur_hclk_div = 2;
    unsigned int runTime = 0;

    uint32_t tmp[3];
    uint32_t cur_fcpu;
    uint32_t cur_batt_volt;
    uint32_t cur_soc_temp;

    FIL *f = pvPortMalloc(sizeof(FIL));

    for (;;) {

        runTime++;

        if (!OS_UISuspend) {

            check_emulator_status();

            ll_get_clkctrl_div(tmp);
            cur_cpu_div = tmp[0];
            cur_cpu_frac = tmp[1];
            cur_hclk_div = tmp[2];

            cur_fcpu = ll_get_cur_freq();
            cur_batt_volt = ll_get_bat_voltage();
            cur_soc_temp = ll_get_core_temp();

            SET_LABEL_TEXT(info_line1, "CPU Freq:%d / %d MHz,  Temp:%d °C", cur_fcpu, 480 * 18 / cur_cpu_div / cur_cpu_frac, cur_soc_temp);
            SET_LABEL_TEXT(info_line2, "Mem: %d / %d KB,  Ticks:%d s", xPortGetFreeHeapSize() / 1024, RAM_SIZE / 1024, runTime);
            SET_LABEL_TEXT(info_line3, "Batt: %d mv,  Charging: %s", cur_batt_volt, ll_get_charge_status() ? "Yes" : "NO");
            SET_LABEL_TEXT(info_line4, "Pwr Speed: %d Ticks", ll_get_pwrspeed());

            if (time_lable_refresh) {
                rtc_time_sec = ll_rtc_get_sec();
                lv_label_set_text_fmt(label_time, "%02d:%02d", (rtc_time_sec / (60 * 60)) % 24, (rtc_time_sec / 60) % 60);
            }
            if (cur_batt_volt > 1408) {
                ll_charge_enable(false);
                lv_obj_clear_state(charge_chb, LV_STATE_CHECKED);
            }

            // printf("f=%d,v=%d,t=%d\r\n", cur_fcpu, cur_batt_volt, cur_soc_temp);
            /*
            if(runTime % 20 == 0)
            {


                if(f)
                {
                    f_open(f, "pwr_infolog.txt", FA_OPEN_ALWAYS | FA_OPEN_APPEND | FA_WRITE);
                    f_printf(f, "%d\t%d\t%d\t%d\r\n", runTime, cur_fcpu, cur_batt_volt, cur_soc_temp);
                    f_sync(f);
                    f_close(f);

                }

            }*/

            /*
                        SET_LABEL_TEXT(lab_cpu_freq, "CPU Freq:%d/%d MHz", ll_get_cur_freq() , 480 * 18 / cur_cpu_div / cur_cpu_frac);
                        lv_label_set_text_fmt(lab_runtime, "RunTime:%d s", runTime);
                        lv_label_set_text_fmt(lab_free_mem, "Free:%d KB", xPortGetFreeHeapSize() / 1024);
                        lv_label_set_text_fmt(lab_voltage, "Batt:%d mV", ll_get_bat_voltage());
                        lv_label_set_text_fmt(lab_cpu_temp, "Core: %d °C" , ll_get_core_temp());*/
        }

        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

void main() {

    // SYS STACK      0x023FA000
    // IRQ STACK      0x023FFFF0
    void IRQ_ISR();
    void SWI_ISR();
    ll_set_irq_stack(IRQ_STACK_ADDR);
    ll_set_irq_vector(((uint32_t)IRQ_ISR) + 4);
    ll_set_svc_stack(SWI_STACK_ADDR);
    ll_set_svc_vector(((uint32_t)SWI_ISR) + 4);
    ll_enable_irq(false);
    // ll_set_keyboard(true);
    ll_cpu_slowdown_enable(false);

    // memset(&__HEAP_START[0], 0xFF, 384 * 1024);

    VROMLoader_Initialize();

    printf("System Booting...\n");

    // xTaskCreate(vTask1, "Task1", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, NULL);
    // xTaskCreate(vTask2, "Task2", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, NULL);

    xTaskCreate(main_thread, "System", 16384, NULL, configMAX_PRIORITIES - 3, NULL);

    ll_cpu_slowdown_enable(true);

    vTaskStartScheduler();

    for (;;) {
        *((double *)0x45678901) = 114514.1919810f;
        void symtab_def();
        symtab_def();
    }
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    PANIC("StackOverflowHook:%s\n", pcTaskName);
}

void vAssertCalled(char *file, int line) {
    PANIC("ASSERTION FAILED AT %s:%d\n", file, line);
}

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize) {
    *ppxTimerTaskTCBBuffer = (StaticTask_t *)pvPortMalloc(sizeof(StaticTask_t));
    *ppxTimerTaskStackBuffer = (StackType_t *)pvPortMalloc(configMINIMAL_STACK_SIZE * 4);
    *pulTimerTaskStackSize = configMINIMAL_STACK_SIZE;
}

void vApplicationMallocFailedHook() {
    PANIC("ASSERT: Out of Memory.\n");
}
