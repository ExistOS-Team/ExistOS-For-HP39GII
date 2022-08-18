

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

QueueHandle_t keyQueue;

uint32_t ck = 0, cp = 0;
uint32_t key_notify = 0;
int capt_ON_Key(int ck, int cp);
void key_task_capt()
{
    int state = 0;
    uint32_t kval;
    int capt_ck = 0;

    for(;;)
    {
        xQueueReceive(keyQueue, &kval, portMAX_DELAY);
        ck = kval & 0xFFFF;
        cp = (kval >> 16) & 1;

        switch (state)
        {
        case 0:
            if(ck == KEY_ON && cp)
            {
                state = 1;
            }else{
                g_latest_key_status = kval;
            }
            break;
        
        case 1:
            if((ck == KEY_ON) && !cp)
            {
                state = 0;
                capt_ck = 0;
                g_latest_key_status = (1 << 16) | KEY_ON;
            }else if(ck != KEY_ON && cp){
                state = 2;
                capt_ck = ck;
                capt_ON_Key(capt_ck, 1);
            }
            break;
        case 2:
            if(cp == 0)
            {
                capt_ON_Key(capt_ck, 0);
                capt_ck = 0;
                state = 0;
                
            }
            break;
        default:
            break;
        }

/*
        if(ck == KEY_ON && cp)
        {
            vTaskDelay(pdMS_TO_TICKS(100));
            if(ck == KEY_F5 && cp){
                printf("KF5\n");
            }
        }
*/

        //vTaskDelay(pdMS_TO_TICKS(100));
    }
}


void key_task() {
    Keys_t k;
    uint8_t press = 0;
    
/*
    uint32_t lcp = 0;
    uint32_t capt_ck = 0;*/

    

    keyQueue = xQueueCreate(32, sizeof(int));

    xTaskCreate(key_task_capt, "keyCapt", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 2, NULL);

    for (;;) {

        {

            /*
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
            }*/

            //lcp = cp;
        }

        vTaskDelay(pdMS_TO_TICKS(20));
        portKeyScan();
        k = portGetChangedKey();
        if (k == 255) {
            continue;
        } else {
            press = portIsKeyDown(k);
            key_notify = (press << 16) | (k & 0xFFFF);

            //ck = k;
            //cp = press;

            xQueueSend(keyQueue, &key_notify, portMAX_DELAY);
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
