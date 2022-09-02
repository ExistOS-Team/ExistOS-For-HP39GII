
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

}apiFuncList_t;

apiFuncList_t apiFuncList;


static void api_taskSleepMs(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

void bin_exec(void *par)
{
    FILINFO finfo;
    FRESULT fr;
    int res;
    void (*app_entry)();
    app_entry = (void (*)())0x0300000C;

    f_stat(par, &finfo);

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

        apiFuncList.taskSleepMs = api_taskSleepMs;
        apiFuncList.taskCreate = (pvoid_ApiFunc)xTaskCreate;
        
        app_entry(&apiFuncList);

        lv_obj_invalidate(lv_scr_act());

        SystemUIResume();

    }
    VROMLoaderDeleteMap(0x03000000);

    vTaskDelete(NULL);
}




