#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "SystemUI.h"
#include "SystemFs.h"

#include "FreeRTOS.h"
#include "task.h"

#include "KLib/tjpgdec/tjpgd.h"

#include "ff.h"

#include "sys_llapi.h"

#include "keyboard_gii39.h"

static lv_group_t *group_backup;
static lv_group_t *group_default_backup;
static lv_group_t *group_jpgv_mbox;

static lv_obj_t *mbox;
static const char *mbox_btns[] = {"OK" , ""};

static uint8_t vrambuf[256*128] __attribute__((aligned(1024)));

static uint8_t scale = 0;
static uint32_t off_x = 0, off_y = 0;

static void jpg_viewer_msg_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *msgbox = lv_event_get_current_target(e);
    

    if (code == LV_EVENT_VALUE_CHANGED) {
        uint16_t select_btn = lv_msgbox_get_active_btn(msgbox); // 0 or 1

        if(select_btn == 0){

        }else{

        }
        
        lv_group_set_default(group_default_backup);
        lv_indev_set_group(SystemGetInKeypad(), group_backup);
        lv_msgbox_close_async(msgbox);
    }
}


static void err_msgbox(char *title, char *info)
{
    group_backup = SystemGetInKeypad()->group;
    group_default_backup = lv_group_get_default();
    lv_group_remove_all_objs(group_jpgv_mbox);
    lv_group_set_default(group_jpgv_mbox);
    mbox = lv_msgbox_create(lv_scr_act(), title, info, (const char **)mbox_btns, false);
    lv_obj_add_event_cb(mbox, jpg_viewer_msg_cb, LV_EVENT_ALL, NULL);
    lv_obj_align(mbox, LV_ALIGN_CENTER, 0, 0);
    lv_obj_center(mbox);
    lv_indev_set_group(SystemGetInKeypad(), group_jpgv_mbox);
}

static size_t tjpg_in_func (
  JDEC* jdec,       /* Pointer to the decompression object */
  uint8_t* buff,    /* Pointer to buffer to store the read data */
  size_t ndata      /* Number of bytes to read/remove */
)
{
    FIL *jpgfile = (FIL *)jdec->device;
    FRESULT fr;
    UINT br;
    if(buff)
    {
        fr = f_read(jpgfile, buff, ndata, &br);
        if(fr == 0)
        {
            return br;
        }else{
            return 0;
        }
    }else{
        fr = f_lseek(jpgfile, f_tell(jpgfile) + ndata);
        if(fr == FR_OK)
        {
            return ndata;
        }else{
            return 0;
        }
    }

}

static int tjpg_out_func (
  JDEC* jdec,    /* Pointer to the decompression object */
  void* bitmap,  /* Bitmap to be output */
  JRECT* rect    /* Rectangle to output */
)
{

    uint8_t *src;
    src = (uint8_t *)bitmap;
    //uint16_t *src;
    //src = (uint16_t *)bitmap;

    for(uint32_t y = rect-> top -off_y ; y < rect->bottom + 1 -off_y; y++) {
        for(uint32_t x = rect-> left -off_x ; x < rect->right + 1 -off_x; x++) {
            
            if((y < 127) && (x < 256))
            {
                
                //vrambuf[x + 256 * y] = (((*src >> 11)/5)<<5) |  ((((*src >> 5) & 0x3F)/9)<<2) | ((*src & 5)/10)      ;
                //src++;
                vrambuf[x + 256 * y] = *src++;
            }
        }
    }

    return 1;
}

static key_t l_getKey()
{

    uint32_t keys, key, kpress;
    static uint32_t last_key;
    static uint32_t last_press;

    keys = ll_vm_check_key();
    key = keys & 0xFFFF;
    kpress = keys >> 16;
    last_key = key;
    last_press = kpress;

    do{
        keys = ll_vm_check_key();
        key = keys & 0xFFFF;
        kpress = keys >> 16;
        
        vTaskDelay(pdMS_TO_TICKS(20));
    }while((last_key == key) && (last_press == kpress));
    /*
    do{
        keys = ll_vm_check_key();
        key = keys & 0xFFFF;
        kpress = keys >> 16;
        if(key == KEY_ON)
        {
            return KEY_ON;
        }
        
        //vTaskDelay(pdMS_TO_TICKS(10));

    }while(kpress);*/
    vTaskDelay(pdMS_TO_TICKS(20));

    return key;
}

void jpgViewer(void *par)
{   
    FRESULT fr;
    JRESULT jr;
    JDEC jdecobj;
    FIL *jpgfile;
    void *mempool;
    char *path = par;
    char msg[256];
    group_jpgv_mbox = lv_group_create();

    mempool = pvPortMalloc(65536);
    if(mempool == NULL)
    {
        err_msgbox( "ERROR", "Insufficient memory! 1");
        goto jpgViewerExit1;
    }

    jpgfile = pvPortMalloc(sizeof(FIL));

    if(jpgfile == NULL)
    {
        err_msgbox( "ERROR", "Insufficient memory! 2");
        goto jpgViewerExit1;
    }

    //jdecobj = pvPortMalloc(sizeof(JDEC));

    fr = f_open(jpgfile, path, FA_OPEN_EXISTING | FA_READ);
    if(fr != FR_OK)
    {
        sprintf(msg, "Failed to open:%s", path);
        err_msgbox( "ERROR", msg);
        goto jpgViewerExit2;
    }

    SystemUISuspend();
    scale = 0;
    off_x = 0;
    off_y = 0;
jpg_decode_start:
    f_lseek(jpgfile, 0);
    jr = jd_prepare(&jdecobj, tjpg_in_func, mempool, 65536, jpgfile);
    switch (jr)
    {
    case JDR_INP:
        err_msgbox("Jpeg decode ERR", "Termination of input stream.");
        goto jpgViewerExit2;
    case JDR_MEM1:
        err_msgbox("Jpeg decode ERR", "Insufficient memory pool for the image.");
        goto jpgViewerExit2;
    case JDR_MEM2:
        err_msgbox("Jpeg decode ERR", "Insufficient stream input buffer.");
        goto jpgViewerExit2;
    case JDR_PAR:
        err_msgbox("Jpeg decode ERR", "Parameter error.");
        goto jpgViewerExit2;
    case JDR_FMT1:
        err_msgbox("Jpeg decode ERR", "Data format error (may be broken data).");
        goto jpgViewerExit2;
    case JDR_FMT2:
        err_msgbox("Jpeg decode ERR", "Format not supported.");
        goto jpgViewerExit2;
    case JDR_FMT3:
        err_msgbox("Jpeg decode ERR", "Not supported JPEG standard.");
        goto jpgViewerExit2;
    case JDR_OK:
        break;
    default:
        err_msgbox("Jpeg decode ERR", "Unknown ERROR.");
        goto jpgViewerExit2;
    }




    memset(vrambuf, 0xFF, sizeof(vrambuf));

    jr = jd_decomp(&jdecobj, tjpg_out_func, scale);

    if(jr == JDR_OK)
    {
        ll_disp_put_area(vrambuf, 0,0,255,126);

        for(;;)
        {
            switch (l_getKey())
            {
            case KEY_ENTER:
                goto jpgViewerExit2;
                break;

            case KEY_SUBTRACTION:
                scale++;
                if(scale > 3)
                    scale = 3;
                goto jpg_decode_start;
            case KEY_PLUS:
                if(scale > 0)
                    scale--;
                goto jpg_decode_start;

            case KEY_RIGHT:
                off_x += 128;
                goto jpg_decode_start;

            case KEY_LEFT:
                if(off_x > 127)
                    off_x -= 128;
                goto jpg_decode_start;   

            case KEY_DOWN:
                off_y += 64;
                goto jpg_decode_start;

            case  KEY_UP:
                if(off_y > 63)
                    off_y -= 64;
                goto jpg_decode_start;   


            
            default:
                break;
            }
        }


    }else{
        lv_obj_invalidate(lv_scr_act());
        lv_refr_now(NULL);
        SystemUIResume();

        err_msgbox("Jpeg decode ERR", "Decode error!");
        goto jpgViewerExit0;
    }
    

    jpgViewerExit2:
    lv_obj_invalidate(lv_scr_act());
    lv_refr_now(NULL);
    
    SystemUIResume();

    f_close(jpgfile);
    jpgViewerExit0:
        vPortFree(jpgfile);
        vPortFree(mempool);
    jpgViewerExit1:
        vTaskDelete(NULL);
}


