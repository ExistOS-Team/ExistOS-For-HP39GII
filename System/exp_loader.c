
#include <stdint.h>
#include <stdio.h>

#include "SystemUI.h"
#include "VROMLoader.h"

typedef struct exp_header_t
{
    uint32_t Mark0;     //0x5AA52333
    uint32_t Mark1;     //1936291909    Exis
    uint32_t Mark2;     //1347436916    tApp

    uint32_t exp_ver;   //1

    uint32_t sys_hash;

    uint32_t entry;

    uint32_t reloc_load_addr;
    uint32_t data_load_addr;
    uint32_t text_load_addr;

    uint32_t reloc_fo;
    uint32_t data_fo;
    uint32_t text_fo;

    uint32_t reloc_sz;
    uint32_t data_sz;
    uint32_t text_sz;

    uint32_t dynsym_of;
    uint32_t dynstr_of;
    uint32_t rel_dyn_of;
    uint32_t rel_plt_of;
    uint32_t got_of;

    uint32_t num_rel_dyn;
    uint32_t num_rel_plt;

    char sys_build_date[32];

}exp_header_t;



extern const char system_build_time[];
extern const uint32_t g_system_symtab_hash;
extern bool g_allow_load_err_app;

static TaskHandle_t exp_loader_task_handel;


static lv_group_t *group_backup;
static lv_group_t *group_default_backup;
static lv_group_t *group_msgbox;
static const char *mbox_btns[] = {"OK", "Cancel" , ""};
static lv_obj_t *mbox;
static bool wait_msg_exit;

static void exp_ldr_msgbox_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *msgbox = lv_event_get_current_target(e);
    if (code == LV_EVENT_VALUE_CHANGED) {
        uint16_t select_btn = lv_msgbox_get_active_btn(msgbox); // 0 or 1

        if(select_btn == 0){

        }else{

        }
        wait_msg_exit = false;
        lv_msgbox_close_async(msgbox);
    }
}


void exp_exec(void *par) {
    FILINFO finfo;
    FRESULT fr;
    UINT br;
    int res;
    exp_header_t exp_h;
    static FIL *f;
    void (*app_entry)();
    exp_loader_task_handel = xTaskGetCurrentTaskHandle();

    group_default_backup = lv_group_get_default();
    group_backup = SystemGetInKeypad()->group;
    group_msgbox = lv_group_create();
    
    printf("pre exec:%s\n", par);
    f = pvPortMalloc(sizeof(FIL));
    if (!f) {
        printf("Failed to alloc memory!\n");
        vTaskDelete(NULL);
    }
    fr = f_open(f, par, FA_OPEN_EXISTING | FA_READ);
    if (fr) {
        printf("Failed to open file:%s,%d!\n", par,fr);
        goto exp_load_exit0;
    }

    fr = f_read(f, &exp_h, sizeof(exp_header_t), &br);
    if (br != sizeof(exp_header_t)) {
        printf("Failed to read file:%d!\n", fr);
        goto exp_load_exit1;
    }

    if (
        exp_h.Mark0 != 0x5AA52333 ||
        exp_h.Mark1 != 1936291909 ||
        exp_h.Mark2 != 1347436916) {
        printf("File header ERROR\n:%08x,%d,%d!\n", exp_h.Mark0,exp_h.Mark1,exp_h.Mark2);
        goto exp_load_exit1;
    }
    app_entry = (void (*)())exp_h.entry;
    printf("exp entry:%08x\n", exp_h.entry);
    printf("exp reloc_fo:%08x\n", exp_h.reloc_fo);
    printf("exp data_fo:%08x\n", exp_h.data_fo);
    printf("exp reloc_load_addr:%08x\n", exp_h.reloc_load_addr);
    printf("exp data_load_addr:%08x\n", exp_h.data_load_addr);
    printf("exp sys_build_date:%s == %s\n", exp_h.sys_build_date, system_build_time); 
    printf("exp syshash:%08x == %08x\n", exp_h.sys_hash, g_system_symtab_hash);

    if(g_allow_load_err_app == false)
        if(exp_h.sys_hash != g_system_symtab_hash)
        {
            printf("This application is not compatible this system version.\n");
            wait_msg_exit = true;
            lv_group_remove_all_objs(group_msgbox);
            lv_group_set_default(group_msgbox);
            lv_indev_set_group(SystemGetInKeypad(), group_msgbox);
            mbox = lv_msgbox_create(lv_scr_act(), "ERROR", "This application is not compatible this system version.", (const char **)mbox_btns, false);
            lv_obj_add_event_cb(mbox, exp_ldr_msgbox_cb, LV_EVENT_ALL, NULL);
            lv_obj_align(mbox, LV_ALIGN_CENTER, 0, 0);
            lv_obj_center(mbox);
            while(wait_msg_exit)
            {
                vTaskDelay(pdMS_TO_TICKS(100));
            }
            goto exp_load_exit1;
        }

    fr = f_lseek(f, exp_h.reloc_fo);
    fr = f_read(f, (void *)exp_h.reloc_load_addr, exp_h.reloc_sz, &br);

    fr = f_lseek(f, exp_h.data_fo);
    fr = f_read(f, (void *)exp_h.data_load_addr, exp_h.data_sz, &br);

    res = VROMLoaderCreateFileMap(f, exp_h.text_fo, exp_h.text_load_addr, exp_h.text_sz);
    
    SystemUISuspend();

    
    xTaskCreate(app_entry, "app", 32768, NULL, configMAX_PRIORITIES - 3, NULL);

    vTaskSuspend(NULL);

    vTaskDelay(pdMS_TO_TICKS(500));


    lv_obj_invalidate(lv_scr_act());
    SystemUIResume();
    VROMLoaderDeleteMap(exp_h.text_load_addr);

    

exp_load_exit1:

    lv_group_set_default(group_default_backup);
    lv_indev_set_group(SystemGetInKeypad(), group_backup);
    lv_group_del(group_msgbox);

    f_close(f);
exp_load_exit0:
    vPortFree(f);
    vTaskDelete(NULL);

}


void exp_app_exit()
{
    vTaskResume(exp_loader_task_handel);
}



