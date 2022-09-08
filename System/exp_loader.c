
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

void exp_exec(void *par) {
    FILINFO finfo;
    FRESULT fr;
    UINT br;
    int res;
    exp_header_t exp_h;
    static FIL *f;
    void (*app_entry)();
    exp_loader_task_handel = xTaskGetCurrentTaskHandle();
    

    f = pvPortMalloc(sizeof(FIL));
    if (!f) {
        vTaskDelete(NULL);
    }
    fr = f_open(f, par, FA_OPEN_EXISTING | FA_READ);
    if (fr) {
        goto exp_load_exit0;
    }

    fr = f_read(f, &exp_h, sizeof(exp_header_t), &br);
    if (br != sizeof(exp_header_t)) {
        goto exp_load_exit1;
    }

    if (
        exp_h.Mark0 != 0x5AA52333 ||
        exp_h.Mark1 != 1936291909 ||
        exp_h.Mark2 != 1347436916) {
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
    f_close(f);
exp_load_exit0:
    vPortFree(f);
    vTaskDelete(NULL);

}


void exp_app_exit()
{
    vTaskResume(exp_loader_task_handel);
}



