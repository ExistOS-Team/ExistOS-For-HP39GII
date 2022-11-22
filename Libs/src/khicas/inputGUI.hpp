#ifndef __INPUTGUI_H
#define __INPUTGUI_H

#include <fxcg/display.h>
#include <fxcg/file.h>
#include <fxcg/keyboard.h>
#include <fxcg/system.h>
#include <fxcg/misc.h>
#include <fxcg/app.h>
#include <fxcg/serial.h>
#include <fxcg/rtc.h>
#include <fxcg/heap.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define INPUTTYPE_NORMAL 0
#define INPUTTYPE_DATE 1
#define INPUTTYPE_TIME 2

#define INPUT_RETURN_EXIT 0
#define INPUT_RETURN_CONFIRM 1
#define INPUT_RETURN_KEYCODE 2

typedef struct {
  int type=INPUTTYPE_NORMAL;
  int x=1; // x and y are in character coordinates (21*8..)
  int y=3;
  int width=21; // again, in character coordinates. note that last space of the input is reserved for scrolling and never gets hit by a char, only the cursor
  int forcetext=0; // if 1, user will be forced to enter text
  int charlimit; // maximum number of chars to admit in bytes (which means that if users enter multibyte it will allow for less chars)
  int symbols=1; // if 1, user will be able to enter symbols with the OS's character select screen
  int key=0; // put a key here to provide for the initial keypress. also, when input returns INPUT_RETURN_KEYCODE, the keycode is here.
  int acceptF6=0; // accept F6 as a way to confirm the input (useful for wizards)
  int cursor=0;
  int start=0;
  char* buffer;
} textInput;

int doTextInput(textInput* input);

#endif