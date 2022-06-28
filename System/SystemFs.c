#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "ff.h"
#include "lvgl.h"

#include "debug.h"

#include "SystemFs.h"
#include "SystemUI.h"

FATFS *fs;

static char textbuf[128];

static lv_fs_dir_t dir;

void SystemFSInit() {
    uint32_t sel;

    FRESULT fres;
    fs = pvPortMalloc(sizeof(FATFS));

mount_flash:

    fres = f_mount(fs, FS_FLASH_PATH, 1);

    printf("f_mount res:%d\n", fres);
    if (fres != FR_OK) {
        sprintf(textbuf, "Mount Fatfs Failed:-%d, would you like to format the flash?", fres);
        sel = SystemUIMsgBox(textbuf, "Mount " FS_FLASH_PATH " Failed", SYSTEMUI_MSGBOX_BUTTON_CANCAL | SYSTEMUI_MSGBOX_BUTTON_OK);
        if (sel == 0) {

            lv_obj_t *spinner = lv_spinner_create(lv_scr_act(), 1000, 60);
            lv_obj_set_size(spinner, 50, 50);
            lv_obj_center(spinner);

            BYTE *work = pvPortMalloc(FF_MAX_SS);
            fres = f_mkfs(FS_FLASH_PATH, 0, work, FF_MAX_SS);
            printf("mkfs:%d\n", fres);

            lv_obj_del(spinner);

            vPortFree(work);

            if (fres == FR_OK) {
                SystemUIMsgBox("Format " FS_FLASH_PATH " Succeeded.", "Format", 0);
            } else {
                SystemUIMsgBox("Format " FS_FLASH_PATH " Failed.", "Format", 0);
            }
        }
        goto mount_flash;
    }

    lv_fs_res_t res;
    res = lv_fs_dir_open(&dir, FS_LVGL_FLASH_PATH "/" FS_SYSTEM_PATH);
    INFO("dir open:%d\n", res);
    if (res != LV_FS_RES_OK) {
        fres = f_mkdir(FS_SYSTEM_PATH);
        INFO("mkdir " FS_SYSTEM_PATH ", %d\n", fres);
    }
    lv_fs_dir_close(&dir);

    res = lv_fs_dir_open(&dir, FS_LVGL_FLASH_PATH "/" FS_SYSTEM_PATH "/" FS_FONTS_PATH);
    INFO("dir open:%d\n", res);
    if (res != LV_FS_RES_OK) {
        fres = f_mkdir(FS_SYSTEM_PATH "/" FS_FONTS_PATH);
        INFO("mkdir " FS_SYSTEM_PATH "/" FS_FONTS_PATH ", %d\n", fres);
    }
    lv_fs_dir_close(&dir);
}
