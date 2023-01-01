#include "k_csdk.h"
// #include "kdisplay.h"
#include "porting.h"
#include "kcasporing_gl.h"
#include <stdlib.h>
#include <stdio.h>

// implementation of the minimal C SDK for KhiCAS
int (*shutdown)()=0;

short shutdown_state=0;
short exam_mode=0,nspire_exam_mode=0;
unsigned exam_start=0; // RTC start
int exam_duration=0;
// <0: indicative duration, ==0 time displayed during exam, >0 end exam_mode after
const int exam_bg1=0x4321,exam_bg2=0x1234;
int exam_bg(){
  return exam_mode?(exam_duration>0?exam_bg1:exam_bg2):0x50719;
}

void os_fill_rect(int x, int y, int width, int height, int color) {
  const int clip_ymin=0;
  if (x < 0){
    width += x;
    x = 0;
  }
  if (y < clip_ymin){
    height += y - clip_ymin;
    y = clip_ymin;
  }
  if (width <= 0 || height <= 0)
    return;
  if (x + width > LCD_WIDTH_PX)
    width = LCD_WIDTH_PX - x;
  if (y + height > LCD_HEIGHT_PX)
    height = LCD_HEIGHT_PX - y;
  if (width <= 0 || height <= 0)
    return;
#if 1
  DISPBOX d; d.left=x; d.right=x + width ; d.top=y; d.bottom=y + height ;
  //Bdisp_AreaClr_VRAM(&d);
  Bdisp_AreaFillVRAM(&d, color);
  //if (color)
  //  Bdisp_AreaReverseVRAM(x, y, x + width - 1, y + height - 1);
#else
  unsigned short *VRAM = (unsigned short *)GetVRAMAddress();
  VRAM += (y * 128) + x;
  while (height--){
    int i = width;
    while (i--){
      *VRAM++ = color;
    }
    VRAM += 128 - width;
  }
#endif
}

void sync_screen(){
  vGL_FlushVScreen();
}

void os_set_pixel(int x, int y, int c) {
  vGL_SetPoint(x, y, c);
}
int os_get_pixel(int x, int y) {
  return vGL_GetPoint(x, y);
}
extern "C" int RTC_GetTicks(); // khicas_stub.cpp
double millis(){ //extern int time_shift;
  return RTC_GetTicks()/1000.;
}
bool waitforvblank(){
  return true;
}
bool back_key_pressed(){
  return false;
}

void os_show_graph(){
  // show graph inside Python shell (Numworks), not used
}
void os_hide_graph(){ // hide graph, not used anymore
}
void statuslinemsg(const char * msg){
  os_fill_rect(0,0,LCD_WIDTH_PX,12,SDK_WHITE);
  vGL_putString(0,0,msg,SDK_BLACK , SDK_WHITE, 12);
}
void statusline(int mode){
  // FIXME
}

int os_get_angle_unit(){
  return 0;
}
bool os_set_angle_unit(int){
  return false;
}
bool alphawasactive(int * key){
  return false; // FIXME
}
extern "C" char Setup_GetEntry(unsigned int index);
extern "C" char *Setup_SetEntry(unsigned int index, char setting);
bool isalphaactive(){
  int k=Setup_GetEntry(0x14);
  return k&0xc;
}
void lock_alpha(){
  Setup_SetEntry(0x14,0x88);
}
void reset_kbd(){
  Setup_SetEntry(0x14,0);
}

extern "C" bool IsKeyDown(int test_key);
bool iskeydown(int key){
  return IsKeyDown(key);
}
int getkey(int allow_suspend){
  // transformed
  int k;
  GetKey(&k);
  return k;
}
void enable_back_interrupt(){
}
void disable_back_interrupt(){
}
bool erase_file(const char * filename){
  unsigned short pFile[256];
  Bfile_StrToName_ncpy(pFile, (const unsigned char *)filename, strlen(filename) + 1);
  int hf = Bfile_OpenFile_OS(pFile, _OPENMODE_READWRITE); // Get handle
  if (hf < 0)
    return false; // nothing to load
  Bfile_CloseFile_OS(hf);
  Bfile_DeleteEntry(pFile);
  return true;
}
const char * read_file(const char * filename){
  unsigned short pFile[256];
  Bfile_StrToName_ncpy(pFile, (const unsigned char *)filename, strlen(filename) + 1);
  int hf = Bfile_OpenFile_OS(pFile, _OPENMODE_READWRITE); // Get handle
  if (hf < 0)
    return 0; // nothing to load
  // int Bfile_ReadFile(int HANDLE,void *buf,int size,int readpos);
  // read variables and modes
  int L = Bfile_GetFileSize_OS(hf);
  if (L<0 || L>=(1<<20))
    return 0;
  static char * BUF=0;
  if (BUF)
    free(BUF);
  BUF=(char *) malloc(L+1);
  if (Bfile_ReadFile(hf, BUF, L, -1) != L || L == 0)
  {
    Bfile_CloseFile_OS(hf);
    return 0;
  }
  BUF[L] = 0;
  return BUF;
}
bool write_file(const char * filename,const char * s,size_t len){
  printf("write_file %s %i\n",filename,len);
  unsigned short pFile[256];
  Bfile_StrToName_ncpy(pFile, (const unsigned char *)filename, strlen(filename) + 1);
  if (!file_exists(filename)){
    int res=Bfile_CreateFile(pFile,len);
    if (res<0)
      return false;
  }
  int hf = Bfile_OpenFile_OS(pFile, _OPENMODE_READWRITE); // Get handle
  if (hf < 0){
    return false;
  }
  Bfile_WriteFile_OS(hf,s,len);
  Bfile_CloseFile_OS(hf);
  return true;
}
int os_file_browser(const char ** filenames,int maxrecords,const char * extension,int storage){
  return -1;
}


extern "C" void Sleep(int millisecond); // khicas_stub.cpp
void os_wait_1ms(int ms){
  return Sleep(ms);
}
