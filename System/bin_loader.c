
#include <stdio.h>
#include <stdint.h>

#include "SystemUI.h"
#include "VROMLoader.h"



typedef void (*void_ApiFunc)();
typedef void* (*pvoid_ApiFunc)();
typedef uint32_t (*uint32_ApiFunc)();

typedef struct apiFuncList_t
{
    void_ApiFunc taskSleepMs;
    pvoid_ApiFunc taskCreate;
    pvoid_ApiFunc kmalloc;
    void_ApiFunc kfree;
    void_ApiFunc printf;
    void_ApiFunc puts;
    void_ApiFunc _exit;

}apiFuncList_t;

apiFuncList_t apiFuncList;

static bool apiListInited = false;

static void api_taskSleepMs(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

static void api_app_exit()
{
    lv_obj_invalidate(lv_scr_act());
    SystemUIResume();
    VROMLoaderDeleteMap(0x03000000);
    vTaskDelete(NULL);
}

static void build_api_list()
{
    apiFuncList.taskSleepMs = api_taskSleepMs;
    apiFuncList.taskCreate = (pvoid_ApiFunc)xTaskCreate;

    apiFuncList.kmalloc = (pvoid_ApiFunc)pvPortMalloc;

    apiFuncList.kfree = (pvoid_ApiFunc)vPortFree;
    apiFuncList.printf = (void_ApiFunc)printf;
    apiFuncList.puts = (void_ApiFunc)puts;
    apiFuncList._exit = (void_ApiFunc)api_app_exit;
    
}



void bin_exec(void *par)
{
    FILINFO finfo;
    int res;
    void (*app_entry)();
    app_entry = (void (*)())0x0300000C;

    if(!apiListInited){
        build_api_list();
        apiListInited = true;
    }

    f_stat(par, &finfo);

    FRESULT fr;
    FIL *f = pvPortMalloc(sizeof(FIL));
    if(!f)
    {
        vTaskDelete(NULL);
    }
    fr = f_open(f, par, FA_OPEN_EXISTING | FA_READ);

    if(fr)
    {
        vPortFree(f);
        vTaskDelete(NULL);
    }

    res = VROMLoaderCreateFileMap(f, 0, 0x03000000, finfo.fsize);
    if(res)
    {
        vTaskDelete(NULL);
    }

    if(
        (((uint32_t *)0x03000000)[0] == 0xA55AAA55) &&
        (((uint32_t *)0x03000000)[1] == 1936291909) &&
        (((uint32_t *)0x03000000)[2] == 1347436916) 
        )
    {
        SystemUISuspend();

        
        app_entry(&apiFuncList);

        lv_obj_invalidate(lv_scr_act());

        SystemUIResume();

    }
    VROMLoaderDeleteMap(0x03000000);
    f_close(f);
    vPortFree(f);

    vTaskDelete(NULL);
}




