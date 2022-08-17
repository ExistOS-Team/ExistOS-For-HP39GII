#ifndef SYSCALLS_H
#define SYSCALLS_H

#ifdef __cplusplus
extern "C" {
#endif

  void GetFKeyIconPointer( int FKeyNo, unsigned char *pBitmap );
  void DisplayFKeyIcons( void );
  void DisplayFKeyIcon( int FKeyPos, unsigned char *pBitmap );
  int Cursor_SetPosition(char column, char row);
  int Cursor_SetFlashStyle(short int flashstyle);
  void Cursor_SetFlashMode(long flashmode);
  void Cursor_SetFlashOff(void);
  void Cursor_SetFlashOn( char flash_style ); 
  char Setup_GetEntry(unsigned int index);
#define GetSetupSetting Setup_GetEntry
  char * Setup_SetEntry(unsigned int index,char setting);
#define SetSetupSetting Setup_SetEntry
  void TestMode(void);
  void* GetVRAMAddress(void);
  void SetQuitHandler( void (*callback)(void) );
  unsigned IsAnyKeyDown(unsigned * key);
  int RTC_GetTicks();
  void RTC_SetDateTime(unsigned char* time);
  int getkeywait(int*column, int*row, int type_of_waiting, int timeout_period, int menu, unsigned short*keycode );
  int MB_ElementCount(const char* buf) ;
  void Bkey_SetAllFlags(short flags) ;
#ifdef __cplusplus
}
#endif

#endif //SYSCALLS_H
