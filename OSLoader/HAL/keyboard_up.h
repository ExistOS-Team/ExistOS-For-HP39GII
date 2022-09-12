#ifndef __KEYBOARD_UP_H__
#define __KEYBOARD_UP_H__

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>


// ┌─────┬───────┬───────┬────────┬──────────┬────────┐
// |     | 0     | 1     | 2      | 3        | 4      |
// ├─────┼───────┼───────┼────────┼──────────┼────────┤
// | 0   | f1    | f3    | f4     | f5       | up     |
// ├─────┼───────┼───────┼────────┼──────────┼────────┤
// | 1   | symb  | f2    | num    | f6       | right  |
// ├─────┼───────┼───────┼────────┼──────────┼────────┤
// | 2   | home  | plot  | views  | x,t,Φ,n  | left   |
// ├─────┼───────┼───────┼────────┼──────────┼────────┤
// | 3   | vars  | apps  | a b/c  | ←        | down   |
// ├─────┼───────┼───────┼────────┼──────────┼────────┤
// | 4   | sin   | math  | tan    | ln       | log    |
// ├─────┼───────┼───────┼────────┼──────────┼────────┤
// | 5   | x^2   | cos   | (      | )        | ÷      |
// ├─────┼───────┼───────┼────────┼──────────┼────────┤
// | 6   | ,     | x^y   | 8      | 9        | x      |
// ├─────┼───────┼───────┼────────┼──────────┼────────┤
// | 7   | alpha | 7     | 5      | 6        | -      |
// ├─────┼───────┼───────┼────────┼──────────┼────────┤
// | 8   | shift | 4     | 2      | 3        | +      |
// ├─────┼───────┼───────┼────────┼──────────┼────────┤
// | 9   | 0     | 1     | .      | (-)      | enter  |
// ├─────┼───────┼───────┼────────┼──────────┼────────┤
// | 10  | on/c  |       |        |          |        |
// └─────┴───────┴───────┴────────┴──────────┴────────┘

typedef enum Keys_t {
    KEY_F1 = (0 << 3) + 0,
    KEY_F2 = (1 << 3) + 1,
    KEY_F3 = (0 << 3) + 1,
    KEY_F4 = (0 << 3) + 2,
    KEY_F5 = (0 << 3) + 3,
    KEY_F6 = (1 << 3) + 3,
    KEY_UP = (0 << 3) + 4,
    KEY_RIGHT = (1 << 3) + 4,
    KEY_LEFT = (2 << 3) + 4,
    KEY_DOWN = (3 << 3) + 4,
    KEY_SYMB = (1 << 3) + 0,
    KEY_NUM = (1 << 3) + 2,
    KEY_HOME = (2 << 3) + 0,
    KEY_PLOT = (2 << 3) + 1,
    KEY_VIEWS = (2 << 3) + 2,
    KEY_XTPHIN = (2 << 3) + 3,
    KEY_VARS = (3 << 3) + 0,
    KEY_APPS = (3 << 3) + 1,
    KEY_ABC = (3 << 3) + 2,
    KEY_BACKSPACE = (3 << 3) + 3,
    KEY_SIN = (4 << 3) + 0,
    KEY_MATH = (4 << 3) + 1,
    KEY_TAN = (4 << 3) + 2,
    KEY_LN = (4 << 3) + 3,
    KEY_LOG = (4 << 3) + 4,
    KEY_X2 = (5 << 3) + 0,
    KEY_COS = (5 << 3) + 1,
    KEY_LEFTBRACKET = (5 << 3) + 2,
    KEY_RIGHTBRACKET = (5 << 3) + 3,
    KEY_DIVISION = (5 << 3) + 4,
    KEY_COMMA = (6 << 3) + 0,
    KEY_XY = (6 << 3) + 1,
    KEY_8 = (6 << 3) + 2,
    KEY_9 = (6 << 3) + 3,
    KEY_MULTIPLICATION = (6 << 3) + 4,
    KEY_ALPHA = (7 << 3) + 0,
    KEY_7 = (7 << 3) + 1,
    KEY_5 = (7 << 3) + 2,
    KEY_6 = (7 << 3) + 3,
    KEY_SUBTRACTION = (7 << 3) + 4,
    KEY_SHIFT = (8 << 3) + 0,
    KEY_4 = (8 << 3) + 1,
    KEY_2 = (8 << 3) + 2,
    KEY_3 = (8 << 3) + 3,
    KEY_PLUS = (8 << 3) + 4,
    KEY_0 = (9 << 3) + 0,
    KEY_1 = (9 << 3) + 1,
    KEY_DOT = (9 << 3) + 2,
    KEY_NEGATIVE = (9 << 3) + 3,
    KEY_ENTER = (9 << 3) + 4,
    KEY_ON = (10 << 3)
} Keys_t;


void portKeyboardGPIOInit(void);
void portKeyScan(void);
bool portIsKeyDown(Keys_t key);
Keys_t portGetChangedKey(void);

/*
Keys_t kb_waitAnyKeyPress();
void kb_waitKeyPress(Keys_t key);
bool kb_isKeyPress(Keys_t key);
*/

void key_task();
void key_svcInit();
void key_register_notify(TaskHandle_t task);

//uint8_t key_getChanged();
//void kb_isAnyKeyPress(Keys_t *key, bool *press);

#endif


