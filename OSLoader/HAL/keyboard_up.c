

#include "keyboard_up.h"


typedef struct CheckKey_t
{
    Keys_t key;
    TaskHandle_t curTask;
}CheckKey_t;

uint32_t g_latest_key_status;
static QueueHandle_t KeyCheckList;

void key_svcInit()
{
    KeyCheckList = xQueueCreate(4, sizeof(TaskHandle_t));

}

void key_register_notify(TaskHandle_t task)
{
    xQueueSend(KeyCheckList, &task, 0);
}
/*
uint8_t key_getChanged()
{
    return portGetChangedKey();
}
*/

void key_task()
{
    Keys_t k;
    uint8_t press = 0;
    uint32_t key_notify;

    for(;;){
        vTaskDelay(pdMS_TO_TICKS(20));
        portKeyScan();
        k = portGetChangedKey();
        if(k == 255){
            continue;
        }else{
            press = portIsKeyDown(k);
            key_notify = (press << 16) | (k & 0xFFFF);
            g_latest_key_status = key_notify;
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

