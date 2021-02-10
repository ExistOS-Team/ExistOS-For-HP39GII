

#ifndef SERVICE_KEYBOARD_H
#define SERVICE_KEYBOARD_H

/* Kernel includes. */
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "keyboard.h"

void vServiceKeyboard(void *pvParameters);
int register_keyboard_callback(void (*cb)(unsigned int));
char key_map_to_alpha(keys key);

#endif
