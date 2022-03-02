

#include "keyboard_up.h"


typedef struct CheckKey_t
{
    Keys_t key;
    TaskHandle_t curTask;
}CheckKey_t;


static QueueHandle_t KeyCheckList;

void key_svcInit()
{
    
}

uint8_t key_getChanged()
{
    return portGetChangedKey();
}

void key_task()
{
    for(;;){
        vTaskDelay(pdMS_TO_TICKS(20));
        portKeyScan();
    }

}

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


