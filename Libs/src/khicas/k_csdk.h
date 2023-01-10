/* C header for Khicas interface with calculator OS */
#ifndef K_CSDK_H
#define K_CSDK_H
#include "k_defs.h"
// Defaults parameters do not work if included from C 
#define SDK_BLACK 0
#define SDK_WHITE 65535
#ifdef __cplusplus
extern "C" {
#endif
  extern short exam_mode,nspire_exam_mode;
  // nspire_exam_mode==0 normal mode
  // ==1 led are blinking
  // ==2 OS exam mode is on but led are not blinking
  // Set the Nspire in normal exam mode at home, transfer ndless and khicas
  // activate khicas: will detect led blinking (mode ==1)
  // prepare exam mode with Khicas menu item (clear led, mode==2),
  // in that mode menu menu will not leave KhiCAS anymore but
  // will clean data and do a hardware reset (launch again exam mode and leds)
  extern unsigned exam_start;
  extern int exam_duration;
#ifdef HP39
  #include <string.h>
  void vGL_putString(int x0, int y0,const char *s, int fg, int bg, int fontSize);
  inline int os_draw_string(int x,int y,int c,int bg,const char * s,bool fake){
    if (!fake) vGL_putString(x,y,s,c,bg,16);
    return strlen(s)*8+x;
  }
  inline int os_draw_string_medium(int x,int y,int c,int bg,const char * s,bool fake){
    if (!fake) vGL_putString(x,y,s,c,bg,14);
    return strlen(s)*7+x;
  }
  inline int os_draw_string_small(int x,int y,int c,int bg,const char * s,bool fake){
    if (!fake) vGL_putString(x,y,s,c,bg,12);
    return strlen(s)*6+x;
  }
#endif
  int ext_main();
  bool waitforvblank();
  bool back_key_pressed() ;
  // next 3 functions may be void if not inside a window class hierarchy
  void os_show_graph(); // show graph inside Python shell (Numworks), not used
  void os_hide_graph(); // hide graph, not used anymore
  void os_redraw(); // force redraw of window class hierarchy
#ifdef NUMWORKS
  void raisememerr();
  extern unsigned _heap_size;
  extern void * _heap_ptr;
  extern void * _heap_base;
  bool inexammode();
  int extapp_restorebackup(int mode);
  void numworks_set_pixel(int x,int y,int c);
  int numworks_get_pixel(int x,int y);
  void numworks_fill_rect(int x,int y,int w,int h,int c);
#ifdef __cplusplus
  int numworks_draw_string(int x,int y,int c,int bg,const char * s,bool fake=false);
  int numworks_draw_string_small(int x,int y,int c,int bg,const char * s,bool fake=false);
#else
  int numworks_draw_string(int x,int y,int c,int bg,const char * s,bool fake);
  int numworks_draw_string_small(int x,int y,int c,int bg,const char * s,bool fake);
#endif
  void numworks_show_graph();
  void numworks_hide_graph();
  void numworks_redraw();
  void numworks_wait_1ms(int ms);
  // access to Numworks OS, defined in port.cpp (or modkandinsky.cpp)
  inline void os_set_pixel(int x,int y,int c){
    numworks_set_pixel(x,y,c);
  }
  inline int os_get_pixel(int x,int y){
    return numworks_get_pixel(x,y);
  }
  inline void os_fill_rect(int x,int y,int w,int h,int c){
    numworks_fill_rect(x,y,w,h,c);
  }
  inline int os_draw_string(int x,int y,int c,int bg,const char * s,bool fake){
    return numworks_draw_string(x,y,c,bg,s,fake);
  }
  inline int os_draw_string_small(int x,int y,int c,int bg,const char * s,bool fake){
    return numworks_draw_string_small(x,y,c,bg,s,fake);
  }
  inline void os_shaw_graph(){ return numworks_show_graph(); }
  inline void os_hide_graph(){ return numworks_hide_graph(); }
  inline void os_redraw(){ return numworks_redraw(); }
  inline void os_wait_1ms(int ms) { numworks_wait_1ms(ms); }
  int getkey_raw(int allow_suspend); // Numworks scan code
  inline void sync_screen(){}
#endif // NUMWORKS


  inline void Printmini(int x,int y,const char * s,int i){ os_draw_string_small(x,y,i?0xffff:0,i?0:0xffff,s,false);}
  inline void Printxy(int x,int y,const char * s,int i){ os_draw_string_medium(x,y,i?0xffff:0,i?0:0xffff,s,false);}
   
  void os_wait_1ms(int ms);
  bool os_set_angle_unit(int mode);
  int os_get_angle_unit();
  double millis(); //extern int time_shift;
  bool file_exists(const char * filename);
  bool erase_file(const char * filename);
  const char * read_file(const char * filename);
#ifdef __cplusplus
  bool write_file(const char * filename,const char * s,size_t len=0);
#else
  bool write_file(const char * filename,const char * s,size_t len);
#endif
#define MAX_NUMBER_OF_FILENAMES 255
  int os_file_browser(const char ** filenames,int maxrecords,const char * extension,int storage);
  void sync_screen();
  void os_set_pixel(int x,int y,int c);
  void os_fill_rect(int x,int y,int w,int h,int c);
  inline void drawRectangle(int x,int y,int w,int h,int c){
    if (w>=0 && h>=0)
      os_fill_rect(x,y,w,h,c);
  }
  int os_get_pixel(int x,int y);
  /* returns new x position */
#ifdef __cplusplus
  int os_draw_string(int x,int y,int c,int bg,const char * s,bool fake=false);
#else
  int os_draw_string(int x,int y,int c,int bg,const char * s,bool fake);
#endif
  inline int os_draw_string_(int x,int y,const char * s){ return os_draw_string(x,y,SDK_BLACK,SDK_WHITE,s,false);}
#ifdef __cplusplus
  int os_draw_string_small(int x,int y,int c,int bg,const char * s,bool fake=false);
#else
  int os_draw_string_small(int x,int y,int c,int bg,const char * s,bool fake);
#endif
  inline int os_draw_string_small_(int x,int y,const char * s){ return os_draw_string_small(x,y,SDK_BLACK,SDK_WHITE,s,false);}
  
#ifdef __cplusplus
#ifdef NUMWORKS
  inline int os_draw_string_medium(int x,int y,int c,int bg,const char * s,bool fake=false){ return os_draw_string_small(x,y,c,bg,s,fake);}
#else
  int os_draw_string_medium(int x,int y,int c,int bg,const char * s,bool fake=false);
#endif
#else
#ifdef NUMWORKS
  inline int os_draw_string_medium(int x,int y,int c,int bg,const char * s,bool fake){ return os_draw_string_small(x,y,c,bg,s,fake);}
#else
  int os_draw_string_medium(int x,int y,int c,int bg,const char * s,bool fake);
#endif
#endif
  inline int os_draw_string_medium_(int x,int y,const char * s){ return os_draw_string_medium(x,y,SDK_BLACK,SDK_WHITE,s,false);}
  void GetKey(int * key);
  int getkey(int allow_suspend); // transformed
  void enable_back_interrupt();
  inline void set_abort(){  enable_back_interrupt(); }
  void disable_back_interrupt();
  inline void clear_abort(){  disable_back_interrupt(); }
  bool isalphaactive();
  bool alphawasactive(int * key);
  void lock_alpha();
  void reset_kbd();
  void statuslinemsg(const char * msg);
#ifdef __cplusplus
  void statusline(int mode=0);
#else
  void statusline(int mode);
#endif
#ifdef NUMWORKS
  inline bool iskeydown(int key){ return getkey(key | 0x80000000); }
#else
  bool iskeydown(int key);
#endif
  
#if defined NSPIRE || defined NSPIRE_NEWLIB
  extern bool nspireemu;
  extern char nspire_filebuf[NSPIRE_FILEBUFFER];
  extern bool on_key_enabled;
  void get_hms(int *h,int *m,int *s);
  void reset_gc();  
#endif

  extern int (*shutdown)(); // function called after 2 hours of idle
  extern short int shutdown_state;
#ifdef __cplusplus
}
#endif
#endif // K_CSDK_H
