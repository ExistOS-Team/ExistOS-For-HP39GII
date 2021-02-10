#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* System serive includes. */
#include "ServiceDebug.h"
#include "ServiceKeyboard.h"

/* Library includes. */
#include "display.h"
#include "irq.h"

#include "regsuartdbg.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

//KeyMessage *CurrentKeyMessage;

QueueHandle_t key_msg_queue = NULL;

struct s_kb_cb_list
{
    struct kb_cb_list *next;
    void (*cb)(unsigned int);
} *kb_cb_list;




int register_keyboard_callback(void (*cb)(unsigned int)){
    struct s_kb_cb_list *current_info;
    
    if(kb_cb_list == NULL){
        kb_cb_list = pvPortMalloc( sizeof (struct s_kb_cb_list) );
        if(kb_cb_list == NULL){
            return -1;
        }
        kb_cb_list->next = NULL;
        kb_cb_list->cb = cb;
        return 0;
    }

    current_info = kb_cb_list;
    while(current_info->next != NULL){
        current_info = current_info->next;
    }

    current_info->next = pvPortMalloc( sizeof (struct s_kb_cb_list) );
    if(current_info->next == NULL){
            return -1;
    }
    current_info = current_info->next;
    current_info->next=NULL;
    current_info->cb = cb;


}

unsigned char key2alpha_lut[100];
char key_map_to_alpha(keys key){
    switch (key)
    {
    case KEY_0: return '"';
    case KEY_DOT: return ':';
    case KEY_NEGATIVE: return ';';
    case KEY_1: return 'X';
    case KEY_2: return 'Y';
    case KEY_3: return 'Z';
    case KEY_PLUS: return ' ';
    case KEY_4: return 'T';
    case KEY_5: return 'U';
    case KEY_6: return 'V';
    case KEY_SUBTRACTION: return 'W';
    case KEY_COMMA: return 'O';
    case KEY_7: return 'P';
    case KEY_8: return 'Q';
    case KEY_9: return 'R';
    case KEY_MULTIPLICATION: return 'S';
    case KEY_X2: return 'J';
    case KEY_XY: return 'K';
    case KEY_LEFTBRACKET: return 'L';
    case KEY_RIGHTBRACET: return 'M';
    case KEY_DIVISION: return 'N';
    case KEY_SIN: return 'E';
    case KEY_COS: return 'F';
    case KEY_TAN: return 'G';
    case KEY_LN: return 'H';
    case KEY_LOG: return 'I';
    case KEY_VARS: return 'A';
    case KEY_MATH: return 'B';
    case KEY_ABC: return 'C';
    case KEY_XTPHIN: return 'D';
    default:
        return 0;
    }
}

void init_key_lut(){
    key2alpha_lut[KEY_0] = '"';
    key2alpha_lut[KEY_DOT] = ':';
    key2alpha_lut[KEY_NEGATIVE] = ';';
    key2alpha_lut[KEY_1] = 'X';
    key2alpha_lut[KEY_2] = 'Y';
    key2alpha_lut[KEY_3] = 'Z';
    key2alpha_lut[KEY_PLUS] = ' ';
    key2alpha_lut[KEY_4] = 'T';
    key2alpha_lut[KEY_5] = 'U';
    key2alpha_lut[KEY_6] = 'V';
    key2alpha_lut[KEY_SUBTRACTION] = 'W';
    key2alpha_lut[KEY_COMMA] = 'O';
    key2alpha_lut[KEY_7] = 'P';
    key2alpha_lut[KEY_8] = 'Q';
    key2alpha_lut[KEY_9] = 'R';
    key2alpha_lut[KEY_MULTIPLICATION] = 'S';
    key2alpha_lut[KEY_X2] = 'J';
    key2alpha_lut[KEY_XY] = 'K';
    key2alpha_lut[KEY_LEFTBRACKET] = 'L';
    key2alpha_lut[KEY_RIGHTBRACET] = 'M';
    key2alpha_lut[KEY_DIVISION] = 'N';
    key2alpha_lut[KEY_SIN] = 'E';
    key2alpha_lut[KEY_COS] = 'F';
    key2alpha_lut[KEY_TAN] = 'G';
    key2alpha_lut[KEY_LN] = 'H';
    key2alpha_lut[KEY_LOG] = 'I';
    key2alpha_lut[KEY_VARS] = 'A';
    key2alpha_lut[KEY_MATH] = 'B';
    key2alpha_lut[KEY_ABC] = 'C';
    key2alpha_lut[KEY_XTPHIN] = 'D';
}

void vServiceKeyboard(void *pvParameters) {
    struct s_kb_cb_list *current_info;
    key_msg_queue = xQueueCreate(128, sizeof(unsigned int));
    init_key_lut();

    unsigned int key_msg;
    for (;;) {
        xQueueReceive( key_msg_queue, &( key_msg ), ( TickType_t ) portMAX_DELAY );
        current_info = kb_cb_list;
        while(current_info != NULL){
            current_info->cb(key_msg);
            current_info=current_info->next;
        }
        //printf("get key:%x\n",key_msg);
    }
}