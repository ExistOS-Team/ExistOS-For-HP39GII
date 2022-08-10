#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

// Character codes
#define KEY_CHAR_0          0x30
#define KEY_CHAR_1          0x31
#define KEY_CHAR_2          0x32
#define KEY_CHAR_3          0x33
#define KEY_CHAR_4          0x34
#define KEY_CHAR_5          0x35
#define KEY_CHAR_6          0x36
#define KEY_CHAR_7          0x37
#define KEY_CHAR_8          0x38
#define KEY_CHAR_9          0x39
#define KEY_CHAR_DP         0x2e
#define KEY_CHAR_EXP        0x0f
#define KEY_CHAR_PMINUS     0x87
#define KEY_CHAR_PLUS       0x89
#define KEY_CHAR_MINUS      0x99
#define KEY_CHAR_MULT       0xa9
#define KEY_CHAR_DIV        0xb9
#define KEY_CHAR_FRAC       0xbb
#define KEY_CHAR_LPAR       0x28
#define KEY_CHAR_RPAR       0x29
#define KEY_CHAR_COMMA      0x2c
#define KEY_CHAR_STORE      0x0e
#define KEY_CHAR_LOG        0x95
#define KEY_CHAR_LN         0x85
#define KEY_CHAR_SIN        0x81
#define KEY_CHAR_COS        0x82
#define KEY_CHAR_TAN        0x83
#define KEY_CHAR_SQUARE     0x8b
#define KEY_CHAR_POW        0xa8
#define KEY_CHAR_IMGNRY     0x7f50
#define KEY_CHAR_LIST       0x7f51
#define KEY_CHAR_MAT        0x7f40
#define KEY_CHAR_EQUAL      0x3d
#define KEY_CHAR_PI         0xd0
#define KEY_CHAR_ANS        0xc0
#define KEY_CHAR_LBRCKT     0x5b
#define KEY_CHAR_RBRCKT     0x5d
#define KEY_CHAR_LBRACE     0x7b
#define KEY_CHAR_RBRACE     0x7d
#define KEY_CHAR_CR         0x0d
#define KEY_CHAR_CUBEROOT   0x96
#define KEY_CHAR_RECIP      0x9b
#define KEY_CHAR_ANGLE      0x7f54
#define KEY_CHAR_EXPN10     0xb5
#define KEY_CHAR_EXPN       0xa5
#define KEY_CHAR_ASIN       0x91
#define KEY_CHAR_ACOS       0x92
#define KEY_CHAR_ATAN       0x93
#define KEY_CHAR_ROOT       0x86
#define KEY_CHAR_POWROOT    0xb8
#define KEY_CHAR_SPACE      0x20
#define KEY_CHAR_DQUATE     0x22
#define KEY_CHAR_VALR       0xcd
#define KEY_CHAR_THETA      0xce
#define KEY_CHAR_A          0x41
#define KEY_CHAR_B          0x42
#define KEY_CHAR_C          0x43
#define KEY_CHAR_D          0x44
#define KEY_CHAR_E          0x45
#define KEY_CHAR_F          0x46
#define KEY_CHAR_G          0x47
#define KEY_CHAR_H          0x48
#define KEY_CHAR_I          0x49
#define KEY_CHAR_J          0x4a
#define KEY_CHAR_K          0x4b
#define KEY_CHAR_L          0x4c
#define KEY_CHAR_M          0x4d
#define KEY_CHAR_N          0x4e
#define KEY_CHAR_O          0x4f
#define KEY_CHAR_P          0x50
#define KEY_CHAR_Q          0x51
#define KEY_CHAR_R          0x52
#define KEY_CHAR_S          0x53
#define KEY_CHAR_T          0x54
#define KEY_CHAR_U          0x55
#define KEY_CHAR_V          0x56
#define KEY_CHAR_W          0x57
#define KEY_CHAR_X          0x58
#define KEY_CHAR_Y          0x59
#define KEY_CHAR_Z          0x5a

/* non-capital char keys, possible in the emulator when writing with the computer keyboard
   and eventually in some text-entry modes. Note that one only needs to add 0x20 to the
   uppercase char key codes to get the codes for the lowercase keys.
*/
#define KEY_CHAR_LOWER_A    0x61
#define KEY_CHAR_LOWER_B    0x62
#define KEY_CHAR_LOWER_C    0x63
#define KEY_CHAR_LOWER_D    0x64
#define KEY_CHAR_LOWER_E    0x65
#define KEY_CHAR_LOWER_F    0x66
#define KEY_CHAR_LOWER_G    0x67
#define KEY_CHAR_LOWER_H    0x68
#define KEY_CHAR_LOWER_I    0x69
#define KEY_CHAR_LOWER_J    0x6A
#define KEY_CHAR_LOWER_K    0x6B
#define KEY_CHAR_LOWER_L    0x6C
#define KEY_CHAR_LOWER_M    0x6D
#define KEY_CHAR_LOWER_N    0x6E
#define KEY_CHAR_LOWER_O    0x6F
#define KEY_CHAR_LOWER_P    0x70
#define KEY_CHAR_LOWER_Q    0x71
#define KEY_CHAR_LOWER_R    0x72
#define KEY_CHAR_LOWER_S    0x73
#define KEY_CHAR_LOWER_T    0x74
#define KEY_CHAR_LOWER_U    0x75
#define KEY_CHAR_LOWER_V    0x76
#define KEY_CHAR_LOWER_W    0x77
#define KEY_CHAR_LOWER_X    0x78
#define KEY_CHAR_LOWER_Y    0x79
#define KEY_CHAR_LOWER_Z    0x7A

// Control codes
#define KEY_CTRL_NOP        0
#define KEY_CTRL_EXE        30004
#define KEY_CTRL_DEL        30025
#define KEY_CTRL_AC         30015
#define KEY_CTRL_FD         30046
#define KEY_CTRL_UNDO	    30045	
#define KEY_CTRL_XTT        30001
#define KEY_CTRL_EXIT       30002
#define KEY_CTRL_SHIFT      30006
#define KEY_CTRL_ALPHA      30007
#define KEY_CTRL_OPTN       30008
#define KEY_CTRL_VARS       30016
#define KEY_CTRL_UP         30018
#define KEY_CTRL_LEFT       30020
#define KEY_CTRL_RIGHT      30021
#define KEY_CTRL_DOWN       30023
#define KEY_CTRL_F1         30009
#define KEY_CTRL_F2         30010
#define KEY_CTRL_F3         30011
#define KEY_CTRL_F4         30012
#define KEY_CTRL_F5         30013
#define KEY_CTRL_F6         30014

#define KEY_CTRL_F7         30075
#define KEY_CTRL_F8         30076
#define KEY_CTRL_F9         30077
#define KEY_CTRL_F10        30078
#define KEY_CTRL_F11        30079
#define KEY_CTRL_F12        30080
#define KEY_CTRL_F13        30081
#define KEY_CTRL_F14        30082

#define KEY_CTRL_CATALOG    30100
#define KEY_CTRL_FORMAT     30101
#define KEY_CTRL_CAPTURE    30055
#define KEY_CTRL_CLIP       30050
#define KEY_CTRL_PASTE      30036
#define KEY_CTRL_INS        30033
#define KEY_CTRL_MIXEDFRAC  30054
#define KEY_CTRL_FRACCNVRT  30026
#define KEY_CTRL_QUIT       30029
#define KEY_CTRL_PRGM       30028
#define KEY_CTRL_SETUP      30037
#define KEY_CTRL_PAGEUP     30052
#define KEY_CTRL_PAGEDOWN   30053
#define KEY_CTRL_MENU       30003
#define KEY_SHIFT_OPTN	    30059
#define KEY_CTRL_RESERVE1	30060
#define KEY_CTRL_RESERVE2	30061
#define KEY_SHIFT_LEFT		30062
#define KEY_SHIFT_RIGHT		30063

#define KEY_PRGM_ACON 10
#define KEY_PRGM_DOWN 37
#define KEY_PRGM_EXIT 47
#define KEY_PRGM_F1 79
#define KEY_PRGM_F2 69
#define KEY_PRGM_F3 59
#define KEY_PRGM_F4 49
#define KEY_PRGM_F5 39
#define KEY_PRGM_F6 29
#define KEY_PRGM_LEFT 38
#define KEY_PRGM_NONE 0
#define KEY_PRGM_RETURN 31
#define KEY_PRGM_RIGHT 27
#define KEY_PRGM_UP 28
#define KEY_PRGM_1 72
#define KEY_PRGM_2 62
#define KEY_PRGM_3 52
#define KEY_PRGM_4 73
#define KEY_PRGM_5 63
#define KEY_PRGM_6 53
#define KEY_PRGM_7 74
#define KEY_PRGM_8 64
#define KEY_PRGM_9 54
#define KEY_PRGM_A 76
#define KEY_PRGM_F 26
#define KEY_PRGM_ALPHA 77 
#define KEY_PRGM_SHIFT 78
#define KEY_PRGM_OPTN 68
#define KEY_PRGM_MENU 48

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
int GetKey(int*key);
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
 
