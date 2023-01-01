/*
  Console Lib for CASIO fx-9860G
  by Mike Smith.
*/

#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdlib.h>
#include <string.h>
#include "libfx.h"

extern "C" int shell_fontw,shell_fonth;

const char * input_matrix(bool list);
ustl::string help_insert(const char * cmdline,int & back,bool warn);
//#define MAX_FILENAME_SIZE 270
#define DATAFOLDER "\\\\fls0\\@KHICAS"
//#define CONSOLESTATEFILE (char*)DATAFOLDER"\\khicas.erd"
void save_console_state_smem(const char * filename);
bool load_console_state_smem(const char * filename);

enum CONSOLE_SCREEN_SPEC{
  COL_LINE_MAX = 64,      //!!!!!!! 32
  LINE_DISP_MAX = 7,      //!!!!!!!  7
  COL_DISP_MAX = 35,//21  //!!!!!!! 21
  EDIT_LINE_MAX = 2048
};

enum CONSOLE_RETURN_VAL{
  CONSOLE_NEW_LINE_SET = 1,
  CONSOLE_SUCCEEDED = 0,
  CONSOLE_MEM_ERR = -1,
  CONSOLE_ARG_ERR = -2,
  CONSOLE_NO_EVENT = -3
};

enum CONSOLE_CURSOR_DIRECTION{
  CURSOR_UP,
  CURSOR_DOWN,
  CURSOR_LEFT,
  CURSOR_RIGHT,
  CURSOR_SHIFT_LEFT,
  CURSOR_SHIFT_RIGHT,
  CURSOR_ALPHA_UP,
  CURSOR_ALPHA_DOWN,
};

enum CONSOLE_LINE_TYPE{
  LINE_TYPE_INPUT=0,
  LINE_TYPE_OUTPUT=1
};

enum CONSOLE_CASE{
  LOWER_CASE,
  UPPER_CASE
};

struct line{
  unsigned char *str=0;
  short int readonly;
  short int type;
  int start_col;
  int disp_len;
#ifdef TEX
  unsigned char *tex_str;
  unsigned char tex_flag;
  int tex_height, tex_width;
#endif
};

struct FMenu{
  char* name;
  char** str;
  unsigned char count;
};

struct location{
  int x;
  int y;
};

#define MAX_FMENU_ITEMS 8
// #define FMENU_TITLE_LENGHT 12 //!!!!

#define is_wchar(c) ((c == 0x7F) || (c == 0xF7) || (c == 0xF9) || (c == 0xE5) || (c == 0xE6) || (c == 0xE7))
//#define printf(s) Console_Output((const unsigned char *)s);

int Console_DelStr(unsigned char *str, int end_pos, int n);
int Console_InsStr(unsigned char *dest, const unsigned char *src, int disp_pos);
int Console_GetActualPos(const unsigned char *str, int disp_pos);
int Console_GetDispLen(const unsigned char *str);
int Console_MoveCursor(int direction);
int Console_Input(const unsigned char *str);
int Console_Output(const unsigned char *str);
void Console_Clear_EditLine();
int Console_NewLine(int pre_line_type, int pre_line_readonly);
int Console_Backspace(void);
int Console_GetKey(void);
int Console_Init(void);
int Console_Disp(void);
int Console_FMenu(int key);
extern char menu_f1[8],menu_f2[8],menu_f3[8],menu_f4[8],menu_f5[8],menu_f6[8];
const char * console_menu(int key,unsigned char* cfg,int active_app);
const char * console_menu(int key,int active_app);
void Console_FMenu_Init(void);
const char * Console_Draw_FMenu(int key, struct FMenu* menu,unsigned char * cfg_,int active_app);
char *Console_Make_Entry(const unsigned char* str);
unsigned char *Console_GetLine(void);
unsigned char* Console_GetEditLine();
void Console_Draw_TeX_Popup(unsigned char* str, int width, int height);
void dConsolePut(const char *);
void dConsolePutChar(const char );
void dConsoleRedraw(void);
extern int dconsole_mode;
extern int console_changed; // 1 if something new in history
extern char session_filename[MAX_FILENAME_SIZE+1];
void translate_fkey(unsigned int & input_key);
bool inputdouble(const char * msg1,double & d);
#endif
