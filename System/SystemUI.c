#include "FreeRTOS.h"
#include "task.h"


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

//#define DISP_HOR_RES 256
//#define VBUFFER_LINE 127

//uint8_t g_ShiftStatus = 0;
//uint8_t g_AlphaStatus = 0; //2; // 0:normal  1:A..Z  2:a..Z


extern bool OS_UISuspend;

//static TaskHandle_t lvgl_svc_task;
//static TaskHandle_t lvgl_tick_task;

uint32_t g_key;
uint32_t g_ket_press;



/*
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

*/

static TaskHandle_t pUITask;

void SystemUIInit() {

    //UI_Init();
    xTaskCreate(UI_Task, "UICore", 800, NULL, configMAX_CO_ROUTINE_PRIORITIES - 3, &pUITask);
}

void UI_Resume();
void UI_Suspend();

extern bool UIForceRefresh ;
//void keyMsg(uint32_t key, int state);
void SystemUIRefresh() 
{
    vTaskDelay(pdMS_TO_TICKS(100));
    UIForceRefresh= true;
    //keyMsg(0, -1);
}

void SystemUISuspend() {
    vTaskSuspend(pUITask);
    UI_Suspend();

}

void SystemUIResume() {
    UI_Resume();
    vTaskResume(pUITask);
    ll_disp_set_indicator(0, -1);
    SystemUIRefresh();
}