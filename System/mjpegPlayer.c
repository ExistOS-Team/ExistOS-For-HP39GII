#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SystemFs.h"
#include "SystemUI.h"

#include "FreeRTOS.h"
#include "task.h"

#include "KLib/tjpgdec/tjpgd.h"

#include "ff.h"

#include "sys_llapi.h"

#include "keyboard_gii39.h"

typedef struct
{
    DWORD fcc;                   // 必须为 avih
    DWORD cb;                    // 本数据结构的大小，不包括最初的8个字节（fcc和cb两个域）
    DWORD dwMicroSecPerFrame;    // 视频帧间隔时间（以毫秒为单位）
    DWORD dwMaxBytesPerSec;      // 这个AVI文件的最大数据率
    DWORD dwPaddingGranularity;  // 数据填充的粒度
    DWORD dwFlags;               // AVI文件的全局标记，比如是否含有索引块等
    DWORD dwTotalFrames;         // 总帧数
    DWORD dwInitialFrames;       // 为交互格式指定初始帧数（非交互格式应该指定为0）
    DWORD dwStreams;             // 本文件包含的流的个数
    DWORD dwSuggestedBufferSize; // 建议读取本文件的缓存大小（应能容纳最大的块）
    DWORD dwWidth;               // 视频图像的宽（以像素为单位）
    DWORD dwHeight;              // 视频图像的高（以像素为单位）
    DWORD dwReserved[4];         // 保留
} AVIMainHeader;

typedef struct {
    DWORD dwList;
    DWORD dwSize; // dwFourcc + data
    DWORD dwFourCC;
    // BYTE data[dwSize-4]; // contains Lists and Chunks
} LIST;

typedef struct {
    DWORD fcc;
    DWORD dwSize; // dwFourcc + data
} BLOCK;

typedef struct
{
    DWORD fcc;                   // 必须为 strh
    DWORD cb;                    // 本数据结构的大小,不包括最初的8个字节(fcc和cb两个域)
    DWORD fccType;               // 流的类型: auds(音频流) vids(视频流) mids(MIDI流) txts(文字流)
    DWORD fccHandler;            // 指定流的处理者，对于音视频来说就是解码器
    DWORD dwFlags;               // 标记：是否允许这个流输出？调色板是否变化？
    WORD wPriority;              // 流的优先级（当有多个相同类型的流时优先级最高的为默认流）
    WORD wLanguage;              // 语言
    DWORD dwInitialFrames;       // 为交互格式指定初始帧数
    DWORD dwScale;               // 每帧视频大小或者音频采样大小
    DWORD dwRate;                // dwScale/dwRate，每秒采样率
    DWORD dwStart;               // 流的开始时间
    DWORD dwLength;              // 流的长度（单位与dwScale和dwRate的定义有关）
    DWORD dwSuggestedBufferSize; // 读取这个流数据建议使用的缓存大小
    DWORD dwQuality;             // 流数据的质量指标（0 ~ 10,000）
    DWORD dwSampleSize;          // Sample的大小
    // RECT rcFrame;               // 指定这个流（视频流或文字流）在视频主窗口中的显示位置，视频主窗口由AVIMAINHEADER结构中的dwWidth和dwHeight决定
} AVIStreamHeader;

static lv_group_t *group_backup;
static lv_group_t *group_default_backup;
static lv_group_t *group_mjpgp_mbox;

static lv_obj_t *mbox;
static const char *mbox_btns[] = {"OK", ""};

static uint8_t vrambuf[256 * 128] __attribute__((aligned(1024)));

static FIL *avifile;

static uint8_t scale = 0;
static uint32_t off_x = 0, off_y = 0;

static void mjpg_player_msg_cb(lv_event_t *e) {
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

static void err_msgbox(char *title, char *info) {
    group_backup = SystemGetInKeypad()->group;
    group_default_backup = lv_group_get_default();
    lv_group_remove_all_objs(group_mjpgp_mbox);
    lv_group_set_default(group_mjpgp_mbox);
    mbox = lv_msgbox_create(lv_scr_act(), title, info, (const char **)mbox_btns, false);
    lv_obj_add_event_cb(mbox, mjpg_player_msg_cb, LV_EVENT_ALL, NULL);
    lv_obj_align(mbox, LV_ALIGN_CENTER, 0, 0);
    lv_obj_center(mbox);
    lv_indev_set_group(SystemGetInKeypad(), group_mjpgp_mbox);
}

static size_t tjpg_in_func(
    JDEC *jdec,    /* Pointer to the decompression object */
    uint8_t *buff, /* Pointer to buffer to store the read data */
    size_t ndata   /* Number of bytes to read/remove */
) {
    // FIL *jpgfile = (FIL *)jdec->device;
    FRESULT fr;
    UINT br;
    if (buff) {
        fr = f_read(avifile, buff, ndata, &br);
        if (fr == 0) {
            return br;
        } else {
            return 0;
        }
    } else {
        fr = f_lseek(avifile, f_tell(avifile) + ndata);
        if (fr == FR_OK) {
            return ndata;
        } else {
            return 0;
        }
    }
}

static int tjpg_out_func(
    JDEC *jdec,   /* Pointer to the decompression object */
    void *bitmap, /* Bitmap to be output */
    JRECT *rect   /* Rectangle to output */
) {

    uint8_t *src;
    src = (uint8_t *)bitmap;
    // uint16_t *src;
    // src = (uint16_t *)bitmap;

    for (uint32_t y = rect->top - off_y; y < rect->bottom + 1 - off_y; y++) {
        for (uint32_t x = rect->left - off_x; x < rect->right + 1 - off_x; x++) {

            if ((y < 127) && (x < 256)) {
                // vrambuf[x + 256 * y] = (((*src >> 11)/5)<<5) |  ((((*src >> 5) & 0x3F)/9)<<2) | ((*src & 5)/10)      ;
                // src++;
                vrambuf[x + 256 * y] = *src++;
            }
        }
    }

    return 1;
}

void mjpegPlayer(void *par) {
    FRESULT fr;
    JRESULT jr;
    UINT br;
    JDEC jdecobj;
    void *mempool;
    char *path = par;
    char msg[256];
    group_mjpgp_mbox = lv_group_create();

    mempool = pvPortMalloc(65536);
    if (mempool == NULL) {
        err_msgbox("ERROR", "Insufficient memory! 1");
        goto mjpgPlayerExit1;
    }

    avifile = pvPortMalloc(sizeof(FIL));

    if (avifile == NULL) {
        err_msgbox("ERROR", "Insufficient memory! 2");
        goto mjpgPlayerExit1;
    }

    fr = f_open(avifile, path, FA_OPEN_EXISTING | FA_READ);
    if (fr != FR_OK) {
        sprintf(msg, "Failed to open:%s", path);
        err_msgbox("ERROR", msg);
        goto mjpgPlayerExit2;
    }

    AVIMainHeader aviHead;
    AVIStreamHeader strh;
    LIST curList;

    f_lseek(avifile, 24);
    fr = f_read(avifile, &aviHead, sizeof(AVIMainHeader), &br);
    if ((br != sizeof(AVIMainHeader)) || fr) {
        err_msgbox("ERROR", "Read file error!");
        goto mjpgPlayerExit3;
    }

    if (memcmp(&aviHead.fcc, "avih", 4)) {
        err_msgbox("ERROR", "AVI Format Error!");
        goto mjpgPlayerExit3;
    }

    printf("Video MicroSecPerFrame:%d\n", aviHead.dwMicroSecPerFrame);
    printf("Video Frames:%d\n", aviHead.dwTotalFrames);
    printf("Video SuggestedBufferSize:%d\n", aviHead.dwSuggestedBufferSize);
    printf("Video Width:%d\n", aviHead.dwWidth);
    printf("Video Height:%d\n", aviHead.dwHeight);

    uint32_t f_pointer = f_tell(avifile);

    fr = f_read(avifile, &curList, sizeof(curList), &br);
    if ((br != sizeof(curList)) || fr) {
        err_msgbox("ERROR", "Read file error!");
        goto mjpgPlayerExit3;
    }
    if (memcmp(&curList.dwList, "LIST", 4)) {
        sprintf(msg, "AVI Format Error! reading List:%08x", curList.dwList);
        err_msgbox("ERROR", msg);
        goto mjpgPlayerExit3;
    }

    fr = f_read(avifile, &strh, sizeof(AVIStreamHeader), &br);
    if ((br != sizeof(AVIStreamHeader)) || fr) {
        err_msgbox("ERROR", "Read file error!");
        goto mjpgPlayerExit3;
    }

    if (memcmp(&strh.fcc, "strh", 4)) {
        err_msgbox("ERROR", "AVI Format Error!");
        goto mjpgPlayerExit3;
    }

    if (memcmp(&strh.fccType, "vids", 4)) {
        err_msgbox("ERROR", "AVI Format Error, not video stream!");
        goto mjpgPlayerExit3;
    }

    if (memcmp(&strh.fccHandler, "MJPG", 4)) {
        err_msgbox("ERROR", "AVI Coding Error, not MJPEG !");
        goto mjpgPlayerExit3;
    }

    do {
        f_lseek(avifile, f_pointer + 8 + curList.dwSize);
        f_pointer = f_tell(avifile);
        fr = f_read(avifile, &curList, sizeof(curList), &br);

        if ((br != sizeof(curList)) || fr) {
            err_msgbox("ERROR", "Read file error! Search movi.");
            goto mjpgPlayerExit3;
        }
    } while (memcmp(&curList.dwFourCC, "movi", 4));

    // printf("movi LIST:%d\n", f_tell(avifile));

    uint32_t movi_start = f_tell(avifile);
    uint32_t movi_size = curList.dwSize;

    BLOCK curBk;

    uint32_t save_pointer;

    SystemUISuspend();

    uint32_t last_tick_ms = ll_get_time_ms();

    bool pause = false, skip = false;

    int32_t frame_time_ms = aviHead.dwMicroSecPerFrame / 1000;
    if(frame_time_ms - 10 > 0)
    {
        frame_time_ms -= 10;
    }
    memset(vrambuf,0,sizeof(vrambuf));
    uint32_t keys, key, kpress;
    do {
        if (!pause) {
            f_pointer = f_tell(avifile);
            fr = f_read(avifile, &curBk, sizeof(BLOCK), &br);
            if ((br != sizeof(BLOCK)) || fr) {
                err_msgbox("ERROR", "Read file error! Search movi.");
                goto mjpgPlayerExit4;
            }
            if ((curBk.fcc == 0x63643030) && (skip == false)) {
                    while (ll_get_time_ms() - last_tick_ms < frame_time_ms) {
                    }
                

                save_pointer = f_tell(avifile);
                jr = jd_prepare(&jdecobj, tjpg_in_func, mempool, 65536, NULL);
                switch (jr) {
                case JDR_INP:
                    err_msgbox("MJpeg decode ERR", "Termination of input stream.");
                    goto mjpgPlayerExit4;
                case JDR_MEM1:
                    err_msgbox("MJpeg decode ERR", "Insufficient memory pool for the image.");
                    goto mjpgPlayerExit4;
                case JDR_MEM2:
                    err_msgbox("MJpeg decode ERR", "Insufficient stream input buffer.");
                    goto mjpgPlayerExit4;
                case JDR_PAR:
                    err_msgbox("MJpeg decode ERR", "Parameter error.");
                    goto mjpgPlayerExit4;
                case JDR_FMT1:
                    err_msgbox("MJpeg decode ERR", "Data format error (may be broken data).");
                    goto mjpgPlayerExit4;
                case JDR_FMT2:
                    err_msgbox("MJpeg decode ERR", "Format not supported.");
                    goto mjpgPlayerExit4;
                case JDR_FMT3:
                    err_msgbox("MJpeg decode ERR", "Not supported JPEG standard.");
                    goto mjpgPlayerExit4;
                case JDR_OK:
                    break;
                default:
                    err_msgbox("MJpeg decode ERR", "Unknown ERROR.");
                    goto mjpgPlayerExit4;
                }

                jd_decomp(&jdecobj, tjpg_out_func, 0);
                ll_disp_put_area(vrambuf, 0, 0, 255, 126);

                f_lseek(avifile, save_pointer);

                last_tick_ms = ll_get_time_ms();
            }

            f_lseek(avifile, f_tell(avifile) + curBk.dwSize + (curBk.dwSize % 2));

            if (f_tell(avifile) > movi_start + movi_size) {
                break;
            }
        }

        keys = ll_vm_check_key();
        key = keys & 0xFFFF;
        kpress = keys >> 16;

        if (key == KEY_ON) {
            goto mjpgPlayerExit4;
        }

        if (kpress) {
            switch (key) {
            case KEY_ENTER:
                pause = !pause;
                break;

            case KEY_PLUS:
                skip = !skip;
                break;

            default:
                break;
            }

            do {
                keys = ll_vm_check_key();
                key = keys & 0xFFFF;
                kpress = keys >> 16;
            } while (kpress);
        }

    } while (1);

mjpgPlayerExit4:
    lv_obj_invalidate(lv_scr_act());
    lv_refr_now(NULL);
    SystemUIResume();

mjpgPlayerExit3:

    f_close(avifile);

mjpgPlayerExit2:

    vPortFree(avifile);
    vPortFree(mempool);

mjpgPlayerExit1:
    vTaskDelete(NULL);
}
