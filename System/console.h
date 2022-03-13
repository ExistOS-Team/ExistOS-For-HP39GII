#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

#define ALIGN_LEFT      0
#define ALIGN_RIGHT     1

int console_put_block_align(char *s, char align);
int console_put_block(char *s);
void refresh_block(bool isfromSelectOn);
void refresh_menu(char *s[], uint8_t selectBit);

void console_init(void);
int console_printf(char *format, ...);


void setSelectBlock(int which);
int getTotalBlock(void);
char *getSelectBlockStr(void);


void InputBox_in_ch(char c);
void shiftIndicate(int num);
int enter_in(void);
void backspace_in(void);

char *console_get_block_string(uint32_t block);

#endif