#ifndef __PORTING_H__
#define __PORTING_H__
#include <stddef.h>

#include "keyboard.h"

#include <stdbool.h>

typedef struct DISPBOX
{
    int left;
    int right;
    int top;
    int bottom;
}DISPBOX;

#define F_KEY_BAR_Y_START  114
#define PRINT_ALPHA_SHIFT_STATUS_X_OFFSET   162
#define F_BAR_FONT_HEIGHT   12
#define CLOCK_STATUS_BAR_OFFSET_X    70

#define CONFIRM_BAR                 "  F1  |      |      |      |      |  F6   "
#define SPE_BAR                     "      |      |      |      |      |       "
#define EDITABLE_BAR                " tst  |loop  |misc  |cmds  |A<>a  |File   "

#define CONSOLE_BAR2                           "  |view  |cmds  |A<>a  |File   "
#define F1_CHARS_LEN    6
#define F2_CHARS_LEN    6

#define CAT_CATEGORY_ALL_BAR        "input | ex1  | ex2  |      |      | help  "
#define CAT_CATEGORY_BAR            "input | ex1  | ex2  |cmds  |      | help  "

#define CAS_VIEW_BAR                           "  |edt+- |cmds  |A<>a  |eval>< "

#define MINI_REV    1
#define MINI_OVER   0

#define LCD_WIDTH_PX   256
#define LCD_HEIGHT_PX  128

#define COLOR_CYAN   90
#define COLOR_RED    68
#define COLOR_GREEN  68
#define COLOR_WHITE  255
#define COLOR_BLACK  0


enum
{
  TEXT_COLOR_BLACK = 0,
  TEXT_COLOR_BLUE = 1,
  TEXT_COLOR_GREEN = 2,
  TEXT_COLOR_CYAN = 3,
  TEXT_COLOR_RED = 4,
  TEXT_COLOR_PURPLE = 5,
  TEXT_COLOR_YELLOW = 6,
  TEXT_COLOR_WHITE = 7
};

enum
{
  TEXT_MODE_NORMAL = 0x00,
  TEXT_MODE_INVERT = 0x01,
  TEXT_MODE_TRANSPARENT_BACKGROUND = 0x20,
  TEXT_MODE_AND = 0x21
};



#define _OPENMODE_READ          0
#define _OPENMODE_READWRITE     1

typedef char FONTCHARACTER;
//typedef int FILE_INFO;
typedef struct
{
  unsigned short id, type;
  unsigned long fsize, dsize;
  unsigned int property;
  unsigned long address;
} FILE_INFO;

void PrintMini(int x, int y, const unsigned char* s, int rev);
void PrintXY(int x, int y, const unsigned char* s, int rev);
void locate(int x, int y);
void PrintRev(unsigned char *s);
void Print(unsigned char *s);

void PopUpWin(int win);

void Bdisp_AreaClr_VRAM(DISPBOX *area);
void Bdisp_AreaFillVRAM(DISPBOX *area, unsigned short color);
void Bdisp_AreaReverseVRAM(int x0, int x1, int y0, int y1);
void Bdisp_PutDisp_DD();
void Bdisp_AllClr_VRAM();
void Bdisp_SetPoint_VRAM(int x, int y, int c);
int Bdisp_GetPoint_VRAM(int x, int y);
void Bdisp_DrawLineVRAM(int x0, int y0, int x1, int y1);

void Bfile_StrToName_ncpy(unsigned short *dest, const unsigned char *source, int len);
void Bfile_NameToStr_ncpy( unsigned char*source, const unsigned short*dest, int len );
int Bfile_OpenFile_OS(unsigned short * pFile, int mode);
int Bfile_CreateFile(unsigned short * pFile, int size);
void Bfile_CloseFile_OS(int hFile);
void Bfile_WriteFile_OS(int hFile, const char *data, size_t len);
void Bfile_DeleteEntry(unsigned short * pFile);
int Bfile_ReadFile_OS( int HANDLE, void *buf, int size, int readpos );
int Bfile_ReadFile(int HANDLE,void *buf,int size,int readpos);
size_t Bfile_GetFileSize_OS(int hFile);

int Bfile_FindFirst( const unsigned short *pathname, int *FindHandle, const unsigned short *foundfile, void *fileinfo );
int Bfile_FindFirst_NON_SMEM( const unsigned short *pathname, int *FindHandle, const unsigned short *foundfile, void *fileinfo );
int Bfile_FindNext( int FindHandle, const unsigned short *foundfile, void *fileinfo );
int Bfile_FindNext_NON_SMEM( int FindHandle, unsigned short *foundfile, void *fileinfo );
int Bfile_FindClose( int FindHandle );

void sprint_double(char *ch, double d);

void *memory_load(char *adresse);

bool chkEsc(void);
int SetTimer(int TimerID, int period, void (*callback)(void));
void KillTimer(int TimerID);


#endif
