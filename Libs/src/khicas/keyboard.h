#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__
#include <stdbool.h>
// Character codes are in k_defs.h
#include "k_defs.h"
// in Bkey_GetKeyWait function
#define KEYWAIT_HALTON_TIMEROFF     0
#define KEYWAIT_HALTOFF_TIMEROFF    1
#define KEYWAIT_HALTON_TIMERON      2

#define KEYREP_NOEVENT              0
#define KEYREP_KEYEVENT             1
#define KEYREP_TIMEREVENT           2

#ifdef __cplusplus
extern "C" {
#endif

void Set_FKeys2( unsigned int p1 );
void Set_FKeys1( unsigned int p1, unsigned int*P2 );
void PRGM_GetKey_OS( unsigned char*p );
void GetKey(int*key);
bool IsKeyDown(int key);
int GetKeyWait_OS(int*column, int*row, int type_of_waiting, int timeout_period, int menu, unsigned short*keycode );
int PRGM_GetKey();
void EditMBStringCtrl(unsigned char *MB_string, int posmax, int *start, int *xpos, int *key, int x, int y);
void EditMBStringCtrl2( unsigned char*MB_string, int xposmax, int*P3, int*xpos, int*key, int x, int y, int enable_pos_to_clear, int pos_to_clear );
void EditMBStringCtrl3( unsigned char*, int xposmax, void*, void*, void*, int, int, int, int, int );
void EditMBStringCtrl4( unsigned char*, int xposmax, void*, void*, void*, int, int, int, int, int, int );
int EditMBStringChar(unsigned char *MB_string, int posmax, int xpos, int char_to_insert);
void DisplayMBString(unsigned char *MB_string, int start, int xpos, int x, int y);
void DisplayMBString2( int P1, unsigned char*MB_string, int start, int xpos, int x, int y, int pos_to_clear, int P8, int P9 );
void Bkey_ClrAllFlags( void );
void Bkey_SetFlag(short flagpattern);
void Bkey_SetAllFlags(short flags);
short Bkey_GetAllFlags( void );

#ifdef __cplusplus
}
#endif

#endif
 
