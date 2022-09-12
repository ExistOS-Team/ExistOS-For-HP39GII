#include "FreeRTOS.h"
#include "task.h"

#include "lvgl.h"

#include "SystemUI.h"

#include "debug.h"
#include "keyboard_gii39.h"

#define SYSTEMUI_MSGBOX_BUTTON_OK       (0)
#define SYSTEMUI_MSGBOX_BUTTON_CANCAL   (1 << 1)
#define isKeyRelease(k) (last_key == k) && (last_press == 1) && (key == k) && (kpress == 0)
#define ALPHA_SELECT(k, a, b, c) \
    do {                         \
        switch (g_AlphaStatus) { \
        case 0:                  \
            k = a;               \
            break;               \
        case 1:                  \
            k = b;               \
            break;               \
        case 2:                  \
            k = c;               \
            break;               \
        }                        \
    } while (0)

#define SHIFT_ALPHA_SELECT(k, normal, s1, s2, a1, a2)   \
    do {                                                \
        k = normal;                                     \
        if (g_ShiftStatus) {                            \
            switch (g_ShiftStatus) {                    \
            case 1:                                     \
                k = s1;                                 \
                break;                                  \
            case 2:                                     \
                if (!g_AlphaStatus)                     \
                    k = s2;                             \
                else                                    \
                    k = (g_AlphaStatus == 1) ? a1 : a2; \
                break;                                  \
            default:                                    \
                break;                                  \
            }                                           \
        } else if (g_AlphaStatus) {                     \
            switch (g_AlphaStatus) {                    \
            case 1:                                     \
                k = a1;                                 \
                break;                                  \
            case 2:                                     \
                k = a2;                                 \
                break;                                  \
            default:                                    \
                break;                                  \
            }                                           \
        }                                               \
    } while (0)

static char *msgbox_button[] = {"OK", "Cancel", ""};

static uint8_t indicator = 0;

#define DISP_HOR_RES 256
#define VBUFFER_LINE 127

uint8_t g_ShiftStatus = 0;
uint8_t g_AlphaStatus = 0; //2; // 0:normal  1:A..Z  2:a..Z


extern bool OS_UISuspend;

static TaskHandle_t lvgl_svc_task;
static TaskHandle_t lvgl_tick_task;

uint32_t g_key;
uint32_t g_ket_press;

static lv_disp_draw_buf_t draw_buf_dsc_1;
static lv_color_t disp_buf_1[DISP_HOR_RES * VBUFFER_LINE];
static lv_disp_drv_t disp_1_drv;
static lv_indev_drv_t indev_drv;
static lv_indev_t *indev_keypad;

static lv_group_t *group;

static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
    // printf("vaddr:%08x\n", color_p);

    ll_disp_put_area((uint8_t *)color_p, area->x1, area->y1, area->x2, area->y2);
    ll_disp_set_indicator(indicator, -1);
    
    lv_disp_flush_ready(disp_drv);
    
}

static void keypad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data) {

    uint32_t keys, key, kpress;
    static uint32_t last_key;
    static uint32_t last_press;

    keys = ll_vm_check_key();
    key = keys & 0xFFFF;
    kpress = keys >> 16;

    if (isKeyRelease(KEY_SHIFT)) {
        g_ShiftStatus++;
        if (g_ShiftStatus >= 3) {
            g_ShiftStatus = 0;
        }
        indicator &= ~(INDICATE_LEFT | INDICATE_RIGHT);
        if (g_ShiftStatus == 1)
            indicator |= INDICATE_LEFT;
        if (g_ShiftStatus == 2)
            indicator |= INDICATE_RIGHT;
        ll_disp_set_indicator(indicator, -1);
    }

    if (isKeyRelease(KEY_ALPHA)) {
        g_AlphaStatus++;
        if (g_AlphaStatus >= 3) {
            g_AlphaStatus = 0;
        }
        indicator &= ~(INDICATE_A__Z | INDICATE_a__z);
        if (g_AlphaStatus == 1)
            indicator |= INDICATE_A__Z;
        if (g_AlphaStatus == 2)
            indicator |= INDICATE_a__z;
        ll_disp_set_indicator(indicator, -1);
    }

    {

        data->state = kpress;
        data->continue_reading = false;
        data->btn_id = 0;

        switch (key) {
        case KEY_LEFT:
            // data->key = LV_KEY_LEFT;

            SHIFT_ALPHA_SELECT(data->key, LV_KEY_LEFT, LV_KEY_HOME, LV_KEY_LEFT, LV_KEY_LEFT, LV_KEY_LEFT);
            break;
        case KEY_RIGHT:
            // data->key = LV_KEY_RIGHT;
            SHIFT_ALPHA_SELECT(data->key, LV_KEY_RIGHT, LV_KEY_END, LV_KEY_RIGHT, LV_KEY_RIGHT, LV_KEY_RIGHT);
            break;
        case KEY_UP:
            if (g_ShiftStatus != 2) {
                data->key = LV_KEY_PREV;
            } else {
                data->key = LV_KEY_UP;
            }
            break;
        case KEY_DOWN:
            if (g_ShiftStatus != 2) {
                data->key = LV_KEY_NEXT;
            } else {
                data->key = LV_KEY_DOWN;
            }
            break;
        case KEY_ENTER:
            data->key = LV_KEY_ENTER;
            break;
        case KEY_BACKSPACE:
            // data->key = LV_KEY_BACKSPACE;
            SHIFT_ALPHA_SELECT(data->key, LV_KEY_BACKSPACE, LV_KEY_DEL, LV_KEY_BACKSPACE, LV_KEY_BACKSPACE, LV_KEY_BACKSPACE);
            break;
        case KEY_SHIFT:
            data->key = last_key;
            data->state = 0;
            break;
        case KEY_ALPHA:
            data->key = last_key;
            data->state = 0;
            break;
        case KEY_F1:
            data->key = KEY_F1 | 0xE000;
            break;
        case KEY_F2:
            data->key = KEY_F2 | 0xE000;
            break;
        case KEY_F3:
            data->key = KEY_F3 | 0xE000;
            break;
        case KEY_F4:
            data->key = KEY_F4 | 0xE000;
            break;
        case KEY_F5:
            data->key = KEY_F5 | 0xE000;
            break;
        case KEY_F6:
            data->key = KEY_F6 | 0xE000;
            break;
        
        case KEY_VARS:
            SHIFT_ALPHA_SELECT(data->key, KEY_VARS | 0xE000, KEY_VARS | 0xE100, KEY_VARS | 0xE200, 'A', 'a');
            break;
        case KEY_MATH:
            SHIFT_ALPHA_SELECT(data->key, KEY_MATH | 0xE000, KEY_MATH | 0xE100, KEY_MATH | 0xE200, 'B', 'b');
            break;
        case KEY_ABC:
            SHIFT_ALPHA_SELECT(data->key, KEY_ABC | 0xE000, KEY_ABC | 0xE100, KEY_ABC | 0xE200, 'C', 'c');
            break;
        case KEY_XTPHIN:
            SHIFT_ALPHA_SELECT(data->key, KEY_XTPHIN | 0xE000, KEY_XTPHIN | 0xE100, KEY_XTPHIN | 0xE200, 'D', 'd');
            break;
        case KEY_SIN:
            SHIFT_ALPHA_SELECT(data->key, KEY_SIN | 0xE000, KEY_SIN | 0xE100, KEY_SIN | 0xE200, 'E', 'e');
            break;
        case KEY_COS:
            SHIFT_ALPHA_SELECT(data->key, KEY_COS | 0xE000, KEY_COS | 0xE100, KEY_COS | 0xE200, 'F', 'f');
            break;
        case KEY_TAN:
            SHIFT_ALPHA_SELECT(data->key, KEY_TAN | 0xE000, KEY_TAN | 0xE100, KEY_TAN | 0xE200, 'G', 'g');
            break;
        case KEY_LN:
            SHIFT_ALPHA_SELECT(data->key, KEY_LN | 0xE000, KEY_LN | 0xE100, KEY_LN | 0xE200, 'H', 'h');
            break;
        case KEY_LOG:
            SHIFT_ALPHA_SELECT(data->key, KEY_LOG | 0xE000, KEY_LOG | 0xE100, KEY_LOG | 0xE200, 'I', 'i');
            break;
        case KEY_X2:
            SHIFT_ALPHA_SELECT(data->key, KEY_X2 | 0xE000, KEY_X2 | 0xE100, KEY_X2 | 0xE200, 'J', 'j');
            break;
        case KEY_XY:
            SHIFT_ALPHA_SELECT(data->key, '^', KEY_XY | 0xE100, KEY_XY | 0xE200, 'K', 'k');
            break;
        case KEY_LEFTBRACKET:
            SHIFT_ALPHA_SELECT(data->key, '(', KEY_LEFTBRACKET | 0xE100, KEY_LEFTBRACKET | 0xE200, 'L', 'l');
            break;
        case KEY_RIGHTBRACKET:
            SHIFT_ALPHA_SELECT(data->key, ')', KEY_RIGHTBRACKET | 0xE100, KEY_RIGHTBRACKET | 0xE200, 'M', 'm');
            break;
        case KEY_DIVISION:
            SHIFT_ALPHA_SELECT(data->key, '/', KEY_DIVISION | 0xE100, KEY_DIVISION | 0xE200, 'N', 'n');
            break;
        case KEY_COMMA:
            SHIFT_ALPHA_SELECT(data->key, ',', KEY_COMMA | 0xE100, KEY_COMMA | 0xE200, 'O', 'o');
            break;
        case KEY_7:
            SHIFT_ALPHA_SELECT(data->key, '7', KEY_7 | 0xE100, KEY_7 | 0xE200, 'P', 'p');
            break;
        case KEY_8:
            SHIFT_ALPHA_SELECT(data->key, '8', '{', '{', 'Q', 'q');
            break;
        case KEY_9:
            SHIFT_ALPHA_SELECT(data->key, '9', '}', '}', 'R', 'r');
            break;
        case KEY_MULTIPLICATION:
            SHIFT_ALPHA_SELECT(data->key, '*', '!', '!', 'S', 's');
            break;
        case KEY_4:
            SHIFT_ALPHA_SELECT(data->key, '4', KEY_4 | 0xE100, KEY_4 | 0xE200, 'T', 't');
            break;
        case KEY_5:
            SHIFT_ALPHA_SELECT(data->key, '5', '[', '[', 'U', 'u');
            break;
        case KEY_6:
            SHIFT_ALPHA_SELECT(data->key, '6', ']', ']', 'V', 'v');
            break;
        case KEY_SUBTRACTION:
            SHIFT_ALPHA_SELECT(data->key, '-', KEY_SUBTRACTION | 0xE100, KEY_SUBTRACTION | 0xE200, 'W', 'w');
            break;
        case KEY_1:
            SHIFT_ALPHA_SELECT(data->key, '1', KEY_1 | 0xE100, KEY_1 | 0xE200, 'X', 'x');
            break;
        case KEY_2:
            SHIFT_ALPHA_SELECT(data->key, '2', 'i', 'i', 'Y', 'y');
            break;
        case KEY_3:
            SHIFT_ALPHA_SELECT(data->key, '3', KEY_3 | 0xE100, KEY_3 | 0xE200, 'Z', 'z');
            break;
        case KEY_PLUS:
            SHIFT_ALPHA_SELECT(data->key, '+', KEY_PLUS | 0xE100, KEY_PLUS | 0xE200, ' ', ' ');
            break;
        case KEY_ON:
        {
            if(g_ShiftStatus == 1)
            {
                ll_power_off();
            }else{
                SHIFT_ALPHA_SELECT(data->key, LV_KEY_ESC, KEY_ON | 0xE100, KEY_ON | 0xE200, KEY_ON | 0xE300, KEY_ON | 0xE400);
            }
        }
            
            break;
        case KEY_0:
            SHIFT_ALPHA_SELECT(data->key, '0', KEY_0 | 0xE100, KEY_0 | 0xE200, '"', '"');
            break;
        case KEY_DOT:
            SHIFT_ALPHA_SELECT(data->key, '.', '=', '=', ':', ':');
            break;
        case KEY_NEGATIVE:
            SHIFT_ALPHA_SELECT(data->key, KEY_NEGATIVE | 0xE000, KEY_NEGATIVE | 0xE100, KEY_NEGATIVE | 0xE200, KEY_NEGATIVE | 0xE300, KEY_NEGATIVE | 0xE400);
            break;
        default:

            break;
        }
    }

    last_key = key;
    last_press = kpress;
}

void lvgl_tick() {
    for (;;) {
        lv_tick_inc(51);
        vTaskDelay(pdMS_TO_TICKS(52));
    }
}

void lvgl_svc() {
    vTaskDelay(pdMS_TO_TICKS(400));
    for (;;) {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(62));
    }
}

lv_indev_t *SystemGetInKeypad()
{
    return indev_keypad;
}

void SystemUIInit() {

    lv_init();

    lv_disp_draw_buf_init(&draw_buf_dsc_1, disp_buf_1, NULL, DISP_HOR_RES * VBUFFER_LINE);
    lv_disp_drv_init(&disp_1_drv);
    disp_1_drv.hor_res = 256;
    disp_1_drv.ver_res = 127;
    disp_1_drv.flush_cb = disp_flush;
    disp_1_drv.draw_buf = &draw_buf_dsc_1;
    lv_disp_drv_register(&disp_1_drv);

    /*Register a button input device*/
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_KEYPAD;
    indev_drv.read_cb = keypad_read;
    indev_keypad = lv_indev_drv_register(&indev_drv);
    
    xTaskCreate(lvgl_svc, "lvgl svc", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 2, &lvgl_svc_task);
    xTaskCreate(lvgl_tick, "lvgl tick", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 2, &lvgl_tick_task);

    vTaskDelay(pdMS_TO_TICKS(100));
    
    group = lv_group_create();
    lv_group_set_default(group);

    lv_indev_t *cur_drv = NULL;
    for (;;) {
        cur_drv = lv_indev_get_next(cur_drv);
        if (!cur_drv) {
            break;
        }

        if (cur_drv->driver->type == LV_INDEV_TYPE_KEYPAD) {
            lv_indev_set_group(cur_drv, group);
        }

        // if (cur_drv->driver->type == LV_INDEV_TYPE_ENCODER) {
        //     lv_indev_set_group(cur_drv, group);
        // }
    }

    lv_group_set_refocus_policy(group, LV_GROUP_REFOCUS_POLICY_NEXT);

    indicator &= ~(INDICATE_LEFT | INDICATE_RIGHT);
    if (g_ShiftStatus == 1)
        indicator |= INDICATE_LEFT;
    if (g_ShiftStatus == 2)
        indicator |= INDICATE_RIGHT;
    ll_disp_set_indicator(indicator, -1);

    indicator &= ~(INDICATE_A__Z | INDICATE_a__z);
    if (g_AlphaStatus == 1)
        indicator |= INDICATE_A__Z;
    if (g_AlphaStatus == 2)
        indicator |= INDICATE_a__z;
    ll_disp_set_indicator(indicator, -1);
}

void SystemUIEditing(bool edit) {
    lv_group_set_editing(group, edit);
}

void SystemUISetBusy(bool enable)
{
    indicator &= ~INDICATE_BUSY;
    if(enable)
        indicator |= INDICATE_BUSY;
    ll_disp_set_indicator(indicator, -1);
}

static void systemui_msgbox_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *msgbox = lv_event_get_current_target(e);
    uint32_t *retval = lv_event_get_user_data(e);

    if (code == LV_EVENT_VALUE_CHANGED) {
        const char *txt = lv_msgbox_get_active_btn_text(msgbox);
        if (txt) {
            uint16_t select_btn = lv_msgbox_get_active_btn(msgbox);
            *retval = select_btn & 0xFFFF;
            *retval |= (1 << 16);

            lv_msgbox_close(msgbox);
            lv_group_focus_freeze(group, false);
        }
    }
}

uint32_t SystemUIMsgBox(lv_obj_t *parent,char *msg, char *title, uint32_t button) {
    char *btns[sizeof(msgbox_button) / sizeof(void *)];
    volatile uint32_t retval = 0;
    uint32_t ind = 0;

    btns[ind++] = msgbox_button[0];
    if (button & SYSTEMUI_MSGBOX_BUTTON_CANCAL) {
        btns[ind++] = msgbox_button[1];
    }

    btns[ind] = "";

    lv_obj_t *mbox = lv_msgbox_create( parent, title, msg, (const char **)btns, false);
    lv_obj_add_event_cb(mbox, systemui_msgbox_event_cb, LV_EVENT_ALL, (uint32_t *)&retval);
    lv_obj_align(mbox, LV_ALIGN_CENTER, 0, 0);
    lv_obj_center(mbox);
    lv_group_focus_freeze(group, true);

    while (!(retval >> 16)) {
        vTaskDelay(pdMS_TO_TICKS(30));
    }

    return retval & 0xFFFF;
}


void SystemUISuspend() {
    OS_UISuspend = true;
    vTaskSuspend(lvgl_svc_task);
    vTaskSuspend(lvgl_tick_task);
}

void SystemUIResume() {
    vTaskResume(lvgl_svc_task);
    vTaskResume(lvgl_tick_task);
    OS_UISuspend = false;
}