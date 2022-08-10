

#include "keyboard_up.h"

typedef struct CheckKey_t {
    Keys_t key;
    TaskHandle_t curTask;
} CheckKey_t;

volatile uint32_t g_latest_key_status;
static QueueHandle_t KeyCheckList;

void key_svcInit() {
    KeyCheckList = xQueueCreate(4, sizeof(TaskHandle_t));
}

void key_register_notify(TaskHandle_t task) {
    xQueueSend(KeyCheckList, &task, 0);
}
/*
uint8_t key_getChanged()
{
    return portGetChangedKey();
}
*/

int takeOn_(int key) {
    if (key == KEY_F1) {
        printf("F1\n");
        return 0;
    }

    if (key == KEY_PLUS) {
        printf("+\n");
        return 1;
    }

    return -1;
}

static uint8_t cnt = 0;

void key_task() {
    Keys_t k;
    uint8_t press = 0;
    uint32_t key_notify = 0;

    uint32_t ck = 0, cp = 0;
    uint32_t lcp = 0;

    int capt_ON_Key(int ck, int cp);

    for (;;) {

        {
            if ((ck == KEY_ON)
                && (lcp == 0) && (cp == 1)) {
                cnt = 10;
            }

            if(cnt > 0)
            {
                cnt--;
                int ret = capt_ON_Key(ck, cp);
                if(ret == 0)
                {
                    cnt = 1;
                    goto capt;
                }
                if(ret == 1)
                {
                    cnt++;
                    goto capt;
                }

                if(cnt == 0)
                {
                    g_latest_key_status = key_notify;
                }
                capt:
                (void)cnt;
            }else{
                g_latest_key_status = key_notify;
            }

            lcp = cp;
        }

        vTaskDelay(pdMS_TO_TICKS(20));
        portKeyScan();
        k = portGetChangedKey();
        if (k == 255) {
            continue;
        } else {
            press = portIsKeyDown(k);
            key_notify = (press << 16) | (k & 0xFFFF);

            ck = k;
            cp = press;
            /*
            if(xQueueReceive(KeyCheckList, &task, 0) == pdTRUE)
            {
                xTaskNotify(task, key_notify, eSetValueWithOverwrite);
                xQueueSendToBack(KeyCheckList, &task, 0);
            }*/
        }
    }
}

/*
void kb_waitKeyPress(Keys_t key)
{
    while(!portIsKeyDown(key)){
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

Keys_t kb_waitAnyKeyPress()
{
    Keys_t k;

    for(;;){
        vTaskDelay(pdMS_TO_TICKS(20));
        k = portGetChangedKey();
        if(k == 255){
            continue;
        }
        if(portIsKeyDown(k)){
            return k;
        }
    }
}

void kb_isAnyKeyPress(Keys_t *key, bool *press)
{
    Keys_t k;
    *key = portGetChangedKey();
    *press = portIsKeyDown(*key);
}

bool kb_isKeyPress(Keys_t key)
{
    return portIsKeyDown(key);
}
*/
