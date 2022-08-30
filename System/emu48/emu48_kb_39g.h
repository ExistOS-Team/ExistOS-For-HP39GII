
#pragma once


#include "pch.h"
#include "emu48.h"
#include "io.h"

#include "emu48_keyboard.h"




DEC_EMU48_KEY(KEY_39G_F1, 11, 5, 1 << 0);
DEC_EMU48_KEY(KEY_39G_F2, 12, 5, 1 << 1);
DEC_EMU48_KEY(KEY_39G_F3, 13, 5, 1 << 2);
DEC_EMU48_KEY(KEY_39G_F4, 14, 5, 1 << 3);
DEC_EMU48_KEY(KEY_39G_F5, 15, 5, 1 << 4);
DEC_EMU48_KEY(KEY_39G_F6, 16, 5, 1 << 5);

DEC_EMU48_KEY(KEY_39G_SYMB,   21,   5, 1 << 7);
DEC_EMU48_KEY(KEY_39G_PLOT,   22,   4, 1 << 7);
DEC_EMU48_KEY(KEY_39G_NUM,    23,   3, 1 << 7);

DEC_EMU48_KEY(KEY_39G_HOME,   31,   2, 1 << 7);
DEC_EMU48_KEY(KEY_39G_APPS,   32,   1, 1 << 7);
DEC_EMU48_KEY(KEY_39G_VIEWS,  33,   0, 1 << 7);

DEC_EMU48_KEY(KEY_39G_VARS  , 41, 4, 1 << 6);
DEC_EMU48_KEY(KEY_39G_MATH  , 42, 3, 1 << 6);
DEC_EMU48_KEY(KEY_39G_ABC   , 43, 2, 1 << 6);
DEC_EMU48_KEY(KEY_39G_XTPHIN, 44, 1, 1 << 6);
DEC_EMU48_KEY(KEY_39G_CLEAR , 45, 0, 1 << 6);

DEC_EMU48_KEY(KEY_39G_SIN   , 51, 4, 1 << 5);
DEC_EMU48_KEY(KEY_39G_COS   , 52, 3, 1 << 5);
DEC_EMU48_KEY(KEY_39G_TAN   , 53, 2, 1 << 5);
DEC_EMU48_KEY(KEY_39G_LN    , 54, 1, 1 << 5);
DEC_EMU48_KEY(KEY_39G_LOG   , 55, 0, 1 << 5);

DEC_EMU48_KEY(KEY_39G_X2            , 61, 4, 1 << 4);
DEC_EMU48_KEY(KEY_39G_XY            , 62, 3, 1 << 4);
DEC_EMU48_KEY(KEY_39G_LEFTBRACKET   , 63, 2, 1 << 4);
DEC_EMU48_KEY(KEY_39G_RIGHTBRACKET  , 64, 1, 1 << 4);
DEC_EMU48_KEY(KEY_39G_DIVISION      , 65, 0, 1 << 4);

DEC_EMU48_KEY(KEY_39G_COMMA         , 71, 7, 1 << 3);
DEC_EMU48_KEY(KEY_39G_7             , 72, 3, 1 << 3);
DEC_EMU48_KEY(KEY_39G_8             , 73, 2, 1 << 3);
DEC_EMU48_KEY(KEY_39G_9             , 74, 1, 1 << 3);
DEC_EMU48_KEY(KEY_39G_MULTIPLICATION, 75, 0, 1 << 3);

DEC_EMU48_KEY(KEY_39G_ALPHA         , 81, 7, 1 << 2);
DEC_EMU48_KEY(KEY_39G_4             , 82, 3, 1 << 2);
DEC_EMU48_KEY(KEY_39G_5             , 83, 2, 1 << 2);
DEC_EMU48_KEY(KEY_39G_6             , 84, 1, 1 << 2);
DEC_EMU48_KEY(KEY_39G_SUBTRACTION   , 85, 0, 1 << 2);

DEC_EMU48_KEY(KEY_39G_SHIFT         , 91, 7, 1 << 1);
DEC_EMU48_KEY(KEY_39G_1             , 92, 3, 1 << 1);
DEC_EMU48_KEY(KEY_39G_2             , 93, 2, 1 << 1);
DEC_EMU48_KEY(KEY_39G_3             , 94, 1, 1 << 1);
DEC_EMU48_KEY(KEY_39G_PLUS          , 95, 0, 1 << 1);

DEC_EMU48_KEY(KEY_39G_ON            , 101, 0, 1 << 15);
DEC_EMU48_KEY(KEY_39G_0             , 102, 3, 1 << 0);
DEC_EMU48_KEY(KEY_39G_DOT           , 103, 2, 1 << 0);
DEC_EMU48_KEY(KEY_39G_NEGATIVE      , 104, 1, 1 << 0);
DEC_EMU48_KEY(KEY_39G_ENTER         , 105, 0, 1 << 0);


DEC_EMU48_KEY(KEY_39G_RIGHT         , 110, 6, 1 << 0);
DEC_EMU48_KEY(KEY_39G_DOWN          , 111, 6, 1 << 1);
DEC_EMU48_KEY(KEY_39G_LEFT          , 112, 6, 1 << 2);
DEC_EMU48_KEY(KEY_39G_UP            , 113, 6, 1 << 3);

