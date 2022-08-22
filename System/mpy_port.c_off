

#include "mpy_port.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "py/compile.h"
#include "py/gc.h"
#include "py/mperrno.h"
#include "py/repl.h"
#include "py/runtime.h"
#include "py/stream.h"
#include "shared/runtime/pyexec.h"

#include "sys_llapi.h"

#include "lvgl.h"

#include "debug.h"

#include "keyboard_gii39.h"

static lv_obj_t *textarea;
static lv_style_t style;

volatile bool mpy_printing = false;

static char key_list[32];
static int key_ind = 0;

static char ringbuf[1024];
static uint32_t r_rptr = 0;
static uint32_t r_wptr = 0;

mp_uint_t mp_hal_ticks_ms(void) {
    return ll_get_time_ms();
    // return 0;
}

void mp_hal_delay_us(uint32_t us) {
    uint32_t n1 = ll_get_time_us();
    while (ll_get_time_us() - n1 < us) {
        ;
    }
}

void mp_hal_set_interrupt_char(int c) {
}

void mp_hal_stdout_tx_strn(const char *str, mp_uint_t len) {

    if(len < (r_wptr + sizeof(ringbuf)))
    {
        memcpy(&ringbuf[r_wptr], str, len);
        r_wptr += len;
        if(r_wptr >= sizeof(ringbuf))
        {
            r_wptr = 0;
        }
    }else{
        uint32_t rem = sizeof(ringbuf) - r_wptr + 0;
        memcpy(&ringbuf[r_wptr], str, rem);
        r_wptr = 0;
        memcpy(&ringbuf[r_wptr], &str[rem], len - rem);
        r_wptr = len - rem;
    }
    


/*
    if (len < 2048 + 1) {
        char *buf = pvPortMalloc(len + 1);
        if (buf) {
            memcpy(buf, str, len);
            buf[len] = 0;
            lv_textarea_add_text(textarea, buf);
        }
        vPortFree(buf);

    } else {
        uint32_t l = len;
        while (l--) {
            lv_textarea_add_char(textarea, *str++);
            // putchar(*str++);
        }
    }
*/
}


void vTaskMpyLog(void *n)
{
    static char tmpbuf[512];
    memset(ringbuf, 0, sizeof(ringbuf));
    memset(tmpbuf, 0, sizeof(tmpbuf));
    for(;;)
    {

        retest:

        if(r_rptr < r_wptr)
        {
            mpy_printing = true;
            lv_textarea_set_cursor_pos(textarea, LV_TEXTAREA_CURSOR_LAST);

                memcpy(tmpbuf, &ringbuf[r_rptr], r_wptr - r_rptr);
                tmpbuf[r_wptr - r_rptr] = 0;
                lv_textarea_add_text(textarea, tmpbuf);

            r_rptr = r_wptr;
            mpy_printing = false;
        } else if(r_rptr > r_wptr)
        {
            mpy_printing = true;
            lv_textarea_set_cursor_pos(textarea, LV_TEXTAREA_CURSOR_LAST);

                memcpy(tmpbuf, &ringbuf[r_rptr], sizeof(ringbuf) - r_rptr);
                tmpbuf[sizeof(ringbuf) - r_rptr] = 0;
                lv_textarea_add_text(textarea, tmpbuf);

            mpy_printing = false;
            r_rptr = 0;
            goto retest;
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}


int mp_hal_stdin_rx_chr(void) {
    int ret = 0;
    if (key_ind > 0) {
        ret = key_list[key_ind--];
    }
    return ret;
}

uintptr_t mp_hal_stdio_poll(uintptr_t poll_flags) {
    uintptr_t ret = 0;

    if (0) {
        ret |= MP_STREAM_POLL_RD;
    }
    return ret;
}

static void ta_event_insert_cb(lv_event_t *e) {

    lv_obj_t *ta = lv_event_get_target(e);
    char *str = e->param;

    if (mpy_printing == false) {
        if(*str != 0x7F)
        lv_textarea_set_insert_replace(ta, "");

    } else {
        for(int i = 0; i < strlen(str); i++)
        {
            if(str[i] == '\033')
            {
                if (memcmp(&str[i+1], "[K", 2) == 0) {
                //lv_textarea_set_insert_replace(ta, "");
                    memset(&str[i], 0, 3);

                    //lv_textarea_del_char(textarea);
                    //lv_textarea_del_char(textarea);

                    INFO("Backspace\n");
                }

            }
        }
    }
}

static void ta_event_key_cb(lv_event_t *e) {

    lv_obj_t *ta = lv_event_get_target(e);
    uint32_t key = lv_event_get_key(e);
    INFO("key:%02X\n", key);

    if (key_ind < sizeof(key_list)) {

        switch (key) {
        case LV_KEY_ENTER:
            key_list[key_ind + 1] = '\r';
            // key_list[key_ind + 2] = '\n';
            key_ind ++;
            break;
        
        case 0xE000 | KEY_F2:
            key_list[key_ind + 1] = '\t';
            key_ind ++;
            break;

        case 0xE000 | KEY_F3:
            key_list[key_ind + 1] = '\t';
            key_ind ++;
            break;

        case 0xE000 | KEY_F4:
            key_list[key_ind + 1] = '\t';
            key_ind ++;
            break;

        case LV_KEY_DEL:
            lv_textarea_set_text(textarea, ">>> ");
            break;

        default:
            key_list[key_ind + 1] = key;
            key_ind++;

            break;
        }
    }
}


void mpy_main() {

    vTaskDelay(pdMS_TO_TICKS(2000));

    memset(key_list, 0, sizeof(key_list));
    key_ind = 0;

    lv_style_init(&style);

    textarea = lv_textarea_create(lv_scr_act());

    lv_obj_align(textarea, LV_ALIGN_TOP_MID, 0, 2);
    lv_obj_set_size(textarea, 240, 100);
    lv_style_set_text_font(&style, &lv_font_montserrat_10);
    lv_obj_add_style(textarea, &style, 0);
    // lv_obj_add_event_cb(textarea, ta_event_cb, LV_EVENT_INSERT, NULL);

    lv_obj_clear_flag(textarea, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
    lv_obj_add_event_cb(textarea, ta_event_insert_cb, LV_EVENT_INSERT, NULL);
    lv_obj_add_event_cb(textarea, ta_event_key_cb, LV_EVENT_KEY, NULL);

    // lv_textarea_set_accepted_chars(textarea, "\xfe");


    xTaskCreate(vTaskMpyLog, "mpy log", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 2, NULL);
    
    vTaskDelay(pdMS_TO_TICKS(2000));
    mpy_init(pvPortMalloc(8192), 8192, NULL);
    
    pyexec_friendly_repl();
}
