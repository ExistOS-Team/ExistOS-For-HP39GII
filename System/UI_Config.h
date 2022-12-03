#pragma once

#include "keyboard_gii39.h"

#define UI_LANG_EN      0
#define UI_LANG_CN      1

#define KEY_TRIG            0
#define KEY_LONG_PRESS      1
#define KEY_RELEASE         2

#define KEY_DEFAULT_PREV    KEY_UP
#define KEY_DEFAULT_NEXT    KEY_DOWN


#define KEY_MESSAGE_RET_CAPTURE               1
#define KEY_MESSAGE_RET_FOCUS_NEXT            2
#define KEY_MESSAGE_RET_FOCUS_PREV            3

#define WIN_DEFAULT_BG_COLOR            0xFF
#define WIN_DEFAULT_BORDER_COLOR        0x70
#define WIN_DEFAULT_FONTSIZE            (16)

#define WIN_DEFAULT_TITLE_BG_COLOR      (0x30)
#define WIN_DEFAULT_TITLE_FONT_COLOR    (0xFF)

#define FUNCKEY_FONTSIZE                (16)

#define FUNCKEY_BAR_Y                   (LCD_PIX_H - FUNCKEY_FONTSIZE)
#define FUNCKEY_BAR_X                   (1)
#define FUNCKEY_BAR_WITDH               (LCD_PIX_W - FUNCKEY_BAR_X)
#define FUNCKEY_BAR_BG_COLOR            (0)

#define FUNCKEY_ITEM_MAX_CHAR           (6)

