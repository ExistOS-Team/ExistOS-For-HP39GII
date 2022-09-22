#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SystemFs.h"
#include "SystemUI.h"

#include "FreeRTOS.h"
#include "task.h"

#include "ff.h"

#include "sys_llapi.h"

#include "keyboard_gii39.h"

#define BUFFERSZ    (4096*4)

static lv_group_t *group_backup;
static lv_group_t *group_default_backup;
static lv_group_t *group_app_mbox;

static lv_obj_t *mbox;
static const char *mbox_btns[] = {"OK", ""};

static void app_msg_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *msgbox = lv_event_get_current_target(e);

    if (code == LV_EVENT_VALUE_CHANGED) {
        uint16_t select_btn = lv_msgbox_get_active_btn(msgbox); // 0 or 1

        if (select_btn == 0) {

        } else {
        }

        lv_group_set_default(group_default_backup);
        lv_indev_set_group(SystemGetInKeypad(), group_backup);
        lv_msgbox_close_async(msgbox);
    }
}

static void info_msgbox(char *title, char *info) {
    group_backup = SystemGetInKeypad()->group;
    group_default_backup = lv_group_get_default();
    lv_group_remove_all_objs(group_app_mbox);
    lv_group_set_default(group_app_mbox);
    mbox = lv_msgbox_create(lv_scr_act(), title, info, (const char **)mbox_btns, false);
    lv_obj_add_event_cb(mbox, app_msg_cb, LV_EVENT_ALL, NULL);
    lv_obj_align(mbox, LV_ALIGN_CENTER, 0, 0);
    lv_obj_center(mbox);
    lv_indev_set_group(SystemGetInKeypad(), group_app_mbox);
}

void wavPlayer(void *par) 
{
    char msg[256];
    FRESULT fr;
    FIL *wavFile;
    UINT br;

    void *mempool;

    group_app_mbox = lv_group_create();

    mempool = pvPortMalloc(BUFFERSZ);
    if (mempool == NULL) {
        info_msgbox("ERROR", "Insufficient memory! 1");
        return;
    }

    wavFile = pvPortMalloc(sizeof(FIL));
    if (wavFile == NULL) {
        info_msgbox("ERROR", "Insufficient memory! 1");
        goto wavp_exit1;
    }

    fr = f_open(wavFile, par, FA_OPEN_EXISTING | FA_READ);

    if (fr) {
        sprintf(msg, "Failed to open file:%d\n", fr);
        goto wavp_exit2;
    }

    
    info_msgbox("Now Playing", (char *)par);
    vTaskDelay(pdMS_TO_TICKS(5000));

    f_lseek(wavFile, 0x2C);

    
    SystemUISuspend();
    ll_cpu_slowdown_enable(false);

    fr = f_read(wavFile, mempool, BUFFERSZ, &br);
    while(fr == 0)
    {
        if(ll_pcm_buffer_idle())
        {
            fr = f_read(wavFile, mempool, BUFFERSZ, &br);
            ll_pcm_buffer_load((uint32_t)mempool);
        }
    }

    lv_obj_invalidate(lv_scr_act());
    lv_refr_now(NULL);
    SystemUIResume();


wavp_exit3:
    f_close(wavFile);
wavp_exit2:
    vPortFree(wavFile);
wavp_exit1:
    vPortFree(mempool);
}
