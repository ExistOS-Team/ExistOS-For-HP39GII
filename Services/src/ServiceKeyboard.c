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
 
void vServiceKeyboard(void *pvParameters) {
    struct s_kb_cb_list *current_info;
    key_msg_queue = xQueueCreate(128, sizeof(unsigned int));
 
    unsigned int key_msg;
    for (;;) {
        xQueueReceive( key_msg_queue, &( key_msg ), ( TickType_t ) portMAX_DELAY );
        current_info = kb_cb_list;
        while(current_info != NULL){
            current_info->cb(key_msg);
            current_info=current_info->next;
        }
        printf("get key:%x\n",key_msg);
    }
}