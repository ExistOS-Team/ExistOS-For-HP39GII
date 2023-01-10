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

#ifdef HP39
#endif

#ifdef NSPIRE_NEWLIB
// NB changes for the nspire cx ii
// on_key_pressed() should be modified (returns always true)
// https://hackspire.org/index.php?title=Memory-mapped_I/O_ports_on_CX_II#90140000_-_Power_management
// cx ii power management 0x90140000,
// cx 900B0018 (R/W), 900B0020 (?)
// cx ii 90140050 (R/W): Disable bus access to peripherals. Reads will just return the last word read from anywhere in the address range, and writes will be ignored.
// cx 900F0020 (R/W): LCD contrast/backlight. Valid range for contrast: 0x11a to 0x1ce; normal value is 0x174. However, it can range from 0x100 (backlight off) to about 0x1d0 (about max brightness).
// -> cx ii The OS controls the LCD backlight by writing to 90130018.
#include <libndls.h>
#include "os.h" // Ndless/ndless-sdk/include/os.h
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <ngc.h>
#include "k_defs.h"


int c_rgb565to888(int c){
  c &= 0xffff;
  int r=(c>>11)&0x1f,g=(c>>5)&0x3f,b=c&0x1f;
  return (r<<19)|(g<<10)|(b<<3);
}

const int nspire_statusarea=18;
bool nspireemu=false;

bool waitforvblank(){
}

bool back_key_pressed(){
  return isKeyPressed(KEY_NSPIRE_DEL);
}
// next 3 functions may be void if not inside a window class hierarchy
void os_show_graph(){} // show graph inside Python shell (Numworks), not used
void os_hide_graph(){} // hide graph, not used anymore
void os_redraw(){} // force redraw of window class hierarchy

bool os_set_angle_unit(int mode){
  return false;
}

int os_get_angle_unit(){
  return 0;
}

double millis(){
  unsigned NSPIRE_RTC_ADDR=0x90090000;
  unsigned t1= * (volatile unsigned *) NSPIRE_RTC_ADDR;
  return 1000.0*t1;
}

void get_hms(int *h,int *m,int *s){
  unsigned NSPIRE_RTC_ADDR=0x90090000;
  unsigned t1= * (volatile unsigned *) NSPIRE_RTC_ADDR;
  if (exam_mode){
    unsigned t=t1-exam_start;
    if (exam_duration>0 && t>exam_duration){
      ;//set_exam_mode(0);
    }
    else {
      if (exam_duration>0)
	t1=exam_duration-t;
      else {
	if (exam_duration<0 && t<-exam_duration)
	  t1=-exam_duration-t;
      }	
    }
  }
  unsigned d=t1/86400;
  *s=t1%86400;
  *h=*s/3600;
  *m=(*s-3600* *h)/60;
  *s%=60;
}

#ifndef is_cx2
#define is_cx2 false
#endif

double loopsleep(int ms){
  double n=ms*(is_cx2?3000:1000),j=0.0;
  for (double i=0;i<n;++i){
    j+=i;
  }
  return j;
}

void ck_msleep(int ms){
  //msleep(ms);
  loopsleep(ms);
}

void os_wait_1ms(int ms){
  ck_msleep(ms);
}

bool file_exists(const char * filename){
  if (access(filename,R_OK))
    return false;
  return true;
}

bool erase_file(const char * filename){
  return remove(filename)==0;
}

char nspire_filebuf[NSPIRE_FILEBUFFER];

const char * read_file(const char * filename){
  if (exam_mode &&
      (strcmp(filename,"session.xw")!=0 &&
       strcmp(filename,"session.xw.tns")!=0 &&
       strcmp(filename,"session.py")!=0 &&
       strcmp(filename,"session.py.tns")!=0 
       )
      )
    return 0;
  FILE * f = fopen(filename,"r");
  if (!f) return 0;
  fseek(f,0L,SEEK_END);
  unsigned s = ftell(f);
  fseek(f,0L,SEEK_SET);
  if (s>NSPIRE_FILEBUFFER-1){
    fclose(f);    
    return 0;
  }
  for (int i=0;i<s;++i){
    if (feof(f))
      break;
    nspire_filebuf[i]=fgetc(f);
  }
  nspire_filebuf[s]=0;
  fclose(f);
  return nspire_filebuf;
}

bool write_file(const char * filename,const char * s,size_t len){
  if (exam_mode &&
      (strcmp(filename,"session.xw")!=0 &&
       strcmp(filename,"session.xw.tns")!=0 &&
       strcmp(filename,"session.py")!=0 &&
       strcmp(filename,"session.py.tns")!=0 
       )
      )
    return false;
  FILE * f=fopen(filename,"w");
  if (!f) return false;
  if (!len) len=strlen(s);
  for (int i=0;i<len;++i)
    fputc(s[i],f);
  fclose(f);
  return true;
}

#define FILENAME_MAXRECORDS 64
  int c_trialpha(const void *p1,const void * p2){
    int i=strcmp(* (char * const *) p1, * (char * const *) p2);
    return i;
  }

char os_filenames[32][FILENAME_MAXRECORDS];
int os_file_browser(const char ** filenames,int maxrecords,const char * extension,int storage){ // storage is ignored on nspire
  DIR *dp;
  struct dirent *ep;
  if (maxrecords>FILENAME_MAXRECORDS-1)
    maxrecords=FILENAME_MAXRECORDS-1;
  dp = opendir (".");
  if (dp == NULL){
    filenames[0]=0;
    return 0;
  }
  int cur=0;
  while ( (ep = readdir (dp)) && cur<maxrecords){
    const char * s_=ep->d_name,*ext=0;
    int l=strlen(s_),j;
    char s[l+1];
    strcpy(s,s_);
    for (j=l-1;j>0;--j){
      if (s[j]=='.'){
	ext=s+j+1;
	break;
      }
    }
    if (ext && strcmp(ext,"tns")==0){
      s[j]=0;
      for (;j>0;--j){
	if (s[j]=='.'){
	  ext=s+j+1;
	  break;
	}
      }
    }
    if (ext && strcmp(ext,extension)==0){
      if (exam_mode &&
	  (strcmp(s_,"session.xw")!=0 &&
	   strcmp(s_,"session.xw.tns")!=0 &&
	   strcmp(s_,"session.py")!=0 &&
	   strcmp(s_,"session.py.tns")!=0 
	   )
	  )
	continue;
      strcpy(os_filenames[cur],s_);
      filenames[cur]=os_filenames[cur];
      ++cur;
    }
  }
  closedir (dp);
  filenames[cur]=NULL;
#if 0
  qsort(filenames,cur,sizeof(char *),c_trialpha);
#else
  // qsort would be faster for large n, but here n<FILENAME_MAXRECORDS
  for (;;){
    bool finished=true;
    for (int i=1;i<cur;++i){
      if (strcmp(filenames[i-1],filenames[i])>0){
	finished=false;
	const char * tmp=filenames[i-1];
	filenames[i-1]=filenames[i];
	filenames[i]=tmp;
      }
    }
    if (finished)
      break;
  }
#endif
  return cur;
}

Gc nspire_gc=0;

void reset_gc(){
  if (nspire_gc){
    gui_gc_finish(nspire_gc);
    //gui_gc_free(nspire_gc);
  }
  nspire_gc=0;
}

Gc * get_gc(){
  if (!nspire_gc){
    nspire_gc=gui_gc_global_GC();
    gui_gc_setRegion(nspire_gc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    gui_gc_begin(nspire_gc);
  }
  return &nspire_gc;
}

void os_set_pixel(int x,int y,int c){
  get_gc();
  gui_gc_setColor(nspire_gc,c_rgb565to888(c));
  gui_gc_drawRect(nspire_gc,x,y+nspire_statusarea,0,0);
}

void os_fill_rect(int x,int y,int w,int h,int c){
  get_gc();
  gui_gc_setColor(nspire_gc,c_rgb565to888(c));
  gui_gc_fillRect(nspire_gc,x,y+nspire_statusarea,w,h);
}

int os_get_pixel(int x,int y){
  if (x<0 || x>=SCREEN_WIDTH || y<0 || y>=SCREEN_HEIGHT)
    return -1;
#if 1
  get_gc();
  char ** off_buff = ((((char *****)nspire_gc)[9])[0])[0x8];
  int res = *(unsigned short *) (off_buff[y+nspire_statusarea] + 2*x);
  return res;
#else
  unsigned short * addr=*(unsigned short **) 0xC0000010;
  int r=addr[(y+nspire_statusarea)*SCREEN_WIDTH+x];
  return r;
#endif
}

int nspire_draw_string(int x,int y,int c,int bg,int f,const char * s,bool fake){
  // void ascii2utf16(void *buf, const char *str, int max_size): converts the UTF-8 string str to the UTF-16 string buf of size max_size.
  int l=strlen(s);
  char utf16[2*l+2];
  ascii2utf16(utf16,s,l);
  utf16[2*l]=0;
  utf16[2*l+1]=0;
  get_gc();
  gui_gc_setFont(nspire_gc,f);
  int dx=gui_gc_getStringWidth(nspire_gc, f, utf16, 0, l) ;
  if (fake)
    return x+dx;
  int dy=17;
  if (f==Regular9)
    dy=13;
  if (f==Regular11)
    dy=16;
  gui_gc_setColor(nspire_gc,c_rgb565to888(bg));
  gui_gc_fillRect(nspire_gc,x,y,dx,dy);
  gui_gc_setColor(nspire_gc,c_rgb565to888(c));
  //gui_gc_setPen(nspire_gc, GC_PS_MEDIUM, GC_PM_SMOOTH);
  gui_gc_drawString(nspire_gc, utf16, x, y-1, GC_SM_NORMAL | GC_SM_TOP); // normal mode
  return x+dx;
}

int os_draw_string(int x,int y,int c,int bg,const char * s,bool fake){
  get_gc();
  gui_gc_clipRect(nspire_gc,0,nspire_statusarea,SCREEN_WIDTH,SCREEN_HEIGHT-nspire_statusarea,0);
  int i=nspire_draw_string(x,y+nspire_statusarea,c,bg,Regular12,s,fake);
  gui_gc_clipRect(nspire_gc,0,0,SCREEN_WIDTH,SCREEN_HEIGHT,GC_CRO_RESET);
  return i;
}
int os_draw_string_small(int x,int y,int c,int bg,const char * s,bool fake){
  get_gc();
  gui_gc_clipRect(nspire_gc,0,nspire_statusarea,SCREEN_WIDTH,SCREEN_HEIGHT-nspire_statusarea,GC_CRO_SET);
  int i=nspire_draw_string(x,y+nspire_statusarea,c,bg,Regular9,s,fake);
  gui_gc_clipRect(nspire_gc,0,0,SCREEN_WIDTH,SCREEN_HEIGHT,GC_CRO_RESET);
  return i;
}

int os_draw_string_medium(int x,int y,int c,int bg,const char * s,bool fake){
  get_gc();
  gui_gc_clipRect(nspire_gc,0,nspire_statusarea,SCREEN_WIDTH,SCREEN_HEIGHT-nspire_statusarea,GC_CRO_SET);
  int i=nspire_draw_string(x,y+nspire_statusarea,c,bg,Regular11,s,fake);
  gui_gc_clipRect(nspire_gc,0,0,SCREEN_WIDTH,SCREEN_HEIGHT,GC_CRO_RESET);
  return i;
}

void statuslinemsg(const char * msg){
  get_gc();
  int bg=exam_bg();
  gui_gc_setColor(nspire_gc,c_rgb565to888(bg));
  gui_gc_fillRect(nspire_gc,0,0,SCREEN_WIDTH,nspire_statusarea);
  nspire_draw_string(0,0,exam_mode?0xffff:0,bg,Regular9,msg,false);
}

void display_time(){
  int h,m,s;
  get_hms(&h,&m,&s);
  char msg[10];
  msg[0]=' ';
  msg[1]='0'+(h/10);
  msg[2]='0'+(h%10);
  msg[3]= 'h';
  msg[4]= ('0'+(m/10));
  msg[5]= ('0'+(m%10));
  msg[6]=0;
  //msg[6]= 'm';
  //msg[7] = ('0'+(s/10));
  //msg[8] = ('0'+(s%10));
  //msg[9]=0;
  int bg=exam_bg();
  gui_gc_setColor(nspire_gc,c_rgb565to888(bg));
  gui_gc_fillRect(nspire_gc,270,0,SCREEN_WIDTH-270,nspire_statusarea);
  nspire_draw_string(270,0,exam_mode?0xffff:0,bg,Regular9,msg,false);
}

void sync_screen(){
  get_gc();
  //gui_gc_finish(nspire_gc);
  gui_gc_blit_to_screen(nspire_gc);
  ck_msleep(10);
  //nspire_gc=0;
  // gui_gc_begin(nspire_gc);
}

// Nspire peripheral reset :
// https://github.com/nDroidProject/nDroid-bootloader/blob/master/kernel.c
// https://hackspire.org/index.php?title=Memory-mapped_I/O_ports_on_CX#CC000000_-_SHA-256_hash_generator
// hardware ports
// https://hackspire.org/index.php?title=Memory-mapped_I/O_ports_on_CX

bool nspire_shift=false;
bool nspire_ctrl=false;
bool nspire_select=false;
void statusline(int mode){
  char *msg=0;
  if (nspire_ctrl){
    if (nspire_shift)
      msg="shift ctrl";
    else
      msg="        ctrl";
  }
  else {
    if (nspire_shift)
      msg="shift";
    else
      msg="";
  }
  int bg=exam_bg();
  gui_gc_setColor(nspire_gc,c_rgb565to888(bg));
  gui_gc_fillRect(nspire_gc,210,0,SCREEN_WIDTH-210,nspire_statusarea);
  nspire_draw_string(220,0,exam_mode?0xffff:0,bg,Regular9,msg,false);
  if (nspireemu)
    nspire_draw_string(210,0,exam_mode?0xffff:0,bg,Regular9,"emu",false);
  display_time();
  if (mode==0)
    return;
  sync_screen();
}


#define SHIFTCTRL(x, y, z) (nspire_ctrl ? (z) : nspire_shift ? (y) : (x))
#define SHIFT(x, y) SHIFTCTRL(x, y, x)
#define CTRL(x, y) SHIFTCTRL(x, x, y)
#define NORMAL(x) SHIFTCTRL(x, x, x)

int ascii_get(int* adaptive_cursor_state){
  if (isKeyPressed(KEY_NSPIRE_CTRL)){
    nspire_ctrl=!nspire_ctrl;
    statusline(0);
    sync_screen();
    return -2;
  }
  if (isKeyPressed(KEY_NSPIRE_SHIFT)){
    nspire_shift=!nspire_shift;
    statusline(0);
    sync_screen();
    return -1;
  }
  *adaptive_cursor_state = SHIFTCTRL(0, 1, 4);
  if (isKeyPressed(KEY_NSPIRE_LEFT)|| isKeyPressed(KEY_NSPIRE_LEFTUP) || isKeyPressed(KEY_NSPIRE_DOWNLEFT))		return SHIFTCTRL(KEY_CTRL_LEFT,KEY_SHIFT_LEFT,KEY_LEFT_CTRL);
  if (isKeyPressed(KEY_NSPIRE_RIGHT)|| isKeyPressed(KEY_NSPIRE_UPRIGHT) || isKeyPressed(KEY_NSPIRE_RIGHTDOWN))		return SHIFTCTRL(KEY_CTRL_RIGHT,KEY_SHIFT_RIGHT,KEY_RIGHT_CTRL);
  if (isKeyPressed(KEY_NSPIRE_UP))		return SHIFTCTRL(KEY_CTRL_UP,KEY_CTRL_PAGEUP,KEY_UP_CTRL);
  if (isKeyPressed(KEY_NSPIRE_DOWN))		return SHIFTCTRL(KEY_CTRL_DOWN,KEY_CTRL_PAGEDOWN,KEY_DOWN_CTRL);
	
  if (isKeyPressed(KEY_NSPIRE_ESC)) return KEY_CTRL_EXIT ;
  if (isKeyPressed(KEY_NSPIRE_HOME)) return KEY_CTRL_MENU ;
  if (isKeyPressed(KEY_NSPIRE_MENU)) return KEY_CTRL_CATALOG ;
  
  // Characters
  if (isKeyPressed(KEY_NSPIRE_A)) return SHIFTCTRL('a','A',KEY_CTRL_A);
  if (isKeyPressed(KEY_NSPIRE_B)) return SHIFTCTRL('b','B',KEY_BOOK);
  if (isKeyPressed(KEY_NSPIRE_C)) return SHIFTCTRL('c','C',KEY_CTRL_CLIP);
  if (isKeyPressed(KEY_NSPIRE_D)) return SHIFTCTRL('d','D',KEY_CTRL_D);
  if (isKeyPressed(KEY_NSPIRE_E)) return SHIFTCTRL('e','E',KEY_CTRL_F10);
  if (isKeyPressed(KEY_NSPIRE_F)) return SHIFTCTRL('f','F',KEY_CTRL_F11);
  if (isKeyPressed(KEY_NSPIRE_G)) return SHIFTCTRL('g','G',KEY_CTRL_F12);
  if (isKeyPressed(KEY_NSPIRE_H)) return SHIFTCTRL('h','H',KEY_CTRL_F13);
  if (isKeyPressed(KEY_NSPIRE_I)) return SHIFTCTRL('i','I',KEY_CTRL_F14);
  if (isKeyPressed(KEY_NSPIRE_J)) return SHIFTCTRL('j','J',KEY_CTRL_F15);
  if (isKeyPressed(KEY_NSPIRE_K)) return SHIFTCTRL('k','K',KEY_CTRL_AC);
  if (isKeyPressed(KEY_NSPIRE_L)) return SHIFTCTRL('l','L',KEY_CTRL_F14);
  if (isKeyPressed(KEY_NSPIRE_M)) return SHIFTCTRL('m','M',KEY_CTRL_CATALOG);
  if (isKeyPressed(KEY_NSPIRE_N)) return SHIFTCTRL('n','N',KEY_CTRL_N);
  if (isKeyPressed(KEY_NSPIRE_O)) return SHIFTCTRL('o','O',KEY_SHIFT_OPTN);
  if (isKeyPressed(KEY_NSPIRE_P)) return SHIFTCTRL('p','P',KEY_CTRL_PRGM);
  if (isKeyPressed(KEY_NSPIRE_Q)) return SHIFT('q','Q');
  if (isKeyPressed(KEY_NSPIRE_R)) return SHIFTCTRL('r','R',KEY_CTRL_R);
  if (isKeyPressed(KEY_NSPIRE_S)) return SHIFTCTRL('s','S',KEY_CTRL_S);
  if (isKeyPressed(KEY_NSPIRE_T)) return SHIFTCTRL('t','T',KEY_CTRL_T);
  if (isKeyPressed(KEY_NSPIRE_U)) return SHIFTCTRL('u','U',KEY_CTRL_F13);
  if (isKeyPressed(KEY_NSPIRE_V)) return SHIFTCTRL('v','V',KEY_CTRL_PASTE);
  if (isKeyPressed(KEY_NSPIRE_W)) return SHIFT('w','W');
  if (isKeyPressed(KEY_NSPIRE_X)) return SHIFTCTRL('x','X',KEY_CTRL_CUT);
  if (isKeyPressed(KEY_NSPIRE_Y)) return SHIFT('y','Y');
  if (isKeyPressed(KEY_NSPIRE_Z)) return SHIFTCTRL('z','Z',KEY_CTRL_UNDO);

  // Numbers
  if (nspireemu){ // for firebird, redefine ctrl
    if (isKeyPressed(KEY_NSPIRE_0)) return SHIFTCTRL('0',KEY_CTRL_F10,')');
    if (isKeyPressed(KEY_NSPIRE_1)) return SHIFTCTRL('1',KEY_CTRL_F1,'!');
    if (isKeyPressed(KEY_NSPIRE_2)) return SHIFTCTRL('2',KEY_CTRL_F2,'@');
    if (isKeyPressed(KEY_NSPIRE_3)) return SHIFTCTRL('3',KEY_CTRL_F3,'#');
    if (isKeyPressed(KEY_NSPIRE_4)) return SHIFTCTRL('4',KEY_CTRL_F4,'$');
    if (isKeyPressed(KEY_NSPIRE_5)) return SHIFTCTRL('5',KEY_CTRL_F5,'%');
    if (isKeyPressed(KEY_NSPIRE_6)) return SHIFTCTRL('6',KEY_CTRL_F6,'^');
    if (isKeyPressed(KEY_NSPIRE_7)) return SHIFTCTRL('7',KEY_CTRL_F7,'&');
    if (isKeyPressed(KEY_NSPIRE_8)) return SHIFTCTRL('8',KEY_CTRL_F8,'*');
    if (isKeyPressed(KEY_NSPIRE_9)) return SHIFTCTRL('9',KEY_CTRL_F9,'(');
  }
  else {
    if (isKeyPressed(KEY_NSPIRE_0)) return SHIFTCTRL('0',KEY_CTRL_F10,KEY_CTRL_F10);
    if (isKeyPressed(KEY_NSPIRE_1)) return SHIFTCTRL('1',KEY_CTRL_F1,KEY_CTRL_F1);
    if (isKeyPressed(KEY_NSPIRE_2)) return SHIFTCTRL('2',KEY_CTRL_F2,KEY_CTRL_F2);
    if (isKeyPressed(KEY_NSPIRE_3)) return SHIFTCTRL('3',KEY_CTRL_F3,KEY_CTRL_F3);
    if (isKeyPressed(KEY_NSPIRE_4)) return SHIFTCTRL('4',KEY_CTRL_F4,KEY_CTRL_F4);
    if (isKeyPressed(KEY_NSPIRE_5)) return SHIFTCTRL('5',KEY_CTRL_F5,KEY_CTRL_F5);
    if (isKeyPressed(KEY_NSPIRE_6)) return SHIFTCTRL('6',KEY_CTRL_F6,KEY_CTRL_F6);
    if (isKeyPressed(KEY_NSPIRE_7)) return SHIFTCTRL('7',KEY_CTRL_F7,KEY_CTRL_F7);
    if (isKeyPressed(KEY_NSPIRE_8)) return SHIFTCTRL('8',KEY_CTRL_F8,KEY_CTRL_F8);
    if (isKeyPressed(KEY_NSPIRE_9)) return SHIFTCTRL('9',KEY_CTRL_F9,KEY_CTRL_F9);
  }
  
  // Symbols
  if (isKeyPressed(KEY_NSPIRE_FRAC)) return SHIFTCTRL(KEY_EQW_TEMPLATE,KEY_AFFECT,KEY_AFFECT);
  if (isKeyPressed(KEY_NSPIRE_SQU)) return CTRL(KEY_CHAR_SQUARE,KEY_CHAR_ROOT);
  if (isKeyPressed(KEY_NSPIRE_TENX)) return CTRL(KEY_CHAR_EXPN10,KEY_CHAR_LOG);
  if (isKeyPressed(KEY_NSPIRE_eEXP)) return CTRL(KEY_CHAR_EXPN,KEY_CHAR_LN);
  if (isKeyPressed(KEY_NSPIRE_COMMA))		return SHIFTCTRL(',',';',':');
  if (isKeyPressed(KEY_NSPIRE_PERIOD)) 	return SHIFTCTRL('.',KEY_CTRL_F11,':');
  if (isKeyPressed(KEY_NSPIRE_COLON))		return NORMAL(':');
  if (isKeyPressed(KEY_NSPIRE_LP))			return SHIFTCTRL('(',KEY_CTRL_F13,KEY_CHAR_CROCHETS);
  if (isKeyPressed(KEY_NSPIRE_RP))			return SHIFTCTRL(')',KEY_CTRL_F14,KEY_CHAR_ACCOLADES);
  if (isKeyPressed(KEY_NSPIRE_SPACE))		return SHIFTCTRL(' ','_','_');
  if (isKeyPressed(KEY_NSPIRE_DIVIDE))
    return SHIFTCTRL('/','%','\\');
  if (isKeyPressed(KEY_NSPIRE_MULTIPLY))	return SHIFTCTRL('*','\'','\"');
  if (isKeyPressed(KEY_NSPIRE_MINUS))		return SHIFTCTRL('-','_', '<');
  if (isKeyPressed(KEY_NSPIRE_NEGATIVE))	return SHIFTCTRL('-',KEY_CTRL_F12,KEY_CHAR_ANS);
  if (isKeyPressed(KEY_NSPIRE_PLUS))		return SHIFTCTRL('+', '>','>');
  if (isKeyPressed(KEY_NSPIRE_EQU))		return SHIFTCTRL('=', '|',KEY_CHAR_STORE);
  if (isKeyPressed(KEY_NSPIRE_LTHAN))		return NORMAL('<');
  if (isKeyPressed(KEY_NSPIRE_GTHAN))		return NORMAL('>');
  if (isKeyPressed(KEY_NSPIRE_QUOTE))		return NORMAL('\"');
  if (isKeyPressed(KEY_NSPIRE_APOSTROPHE))	return NORMAL('\'');
  if (isKeyPressed(KEY_NSPIRE_QUES))		return SHIFTCTRL('?','|','!');
  if (isKeyPressed(KEY_NSPIRE_QUESEXCL))	return SHIFTCTRL('?','|','!');
  if (isKeyPressed(KEY_NSPIRE_BAR))		return NORMAL('|');
  if (isKeyPressed(KEY_NSPIRE_EXP))		return SHIFT('^',KEY_CHAR_RECIP);
  if (isKeyPressed(KEY_NSPIRE_EE))		return SHIFTCTRL('&','%', '@');
  if (isKeyPressed(KEY_NSPIRE_PI)) return KEY_CHAR_PI;
  if (isKeyPressed(KEY_NSPIRE_FLAG)) return SHIFTCTRL(';',':',KEY_CHAR_IMGNRY);
  if (isKeyPressed(KEY_NSPIRE_ENTER))		return SHIFTCTRL(KEY_CTRL_OK,'~',KEY_CTRL_EXE);
  if (isKeyPressed(KEY_NSPIRE_TRIG))		return SHIFTCTRL(KEY_CHAR_SIN,KEY_CHAR_COS,KEY_CHAR_TAN);
  
  // Special chars
  if (isKeyPressed(KEY_NSPIRE_SCRATCHPAD)) return SHIFTCTRL(KEY_CTRL_SETUP,KEY_LOAD,KEY_SAVE);
  if (isKeyPressed(KEY_NSPIRE_VAR)) return CTRL(KEY_CTRL_VARS,KEY_CHAR_STORE);
  if (isKeyPressed(KEY_NSPIRE_DOC))		return KEY_CTRL_MENU;
  if (isKeyPressed(KEY_NSPIRE_CAT))		return KEY_BOOK;
  if (isKeyPressed(KEY_NSPIRE_DEL))		return SHIFTCTRL(KEY_CTRL_DEL,KEY_CTRL_DEL,KEY_CTRL_AC);
  if (isKeyPressed(KEY_NSPIRE_RET))		return KEY_CTRL_EXE;
  if (isKeyPressed(KEY_NSPIRE_TAB))		return '\t';
  
  return 0;
}

bool iskeydown(int key){
  t_key t=KEY_NSPIRE_SPACE;
  switch (key){
  case 0:
    t=KEY_NSPIRE_LEFT;
    break;
  case 1:
    t=KEY_NSPIRE_UP;
    break;
  case 2:
    t=KEY_NSPIRE_DOWN;
    break;
  case 3:
    t=KEY_NSPIRE_RIGHT;
    break;
  case 4:
    t=KEY_NSPIRE_ENTER;
    break;
  case 5:
    t=KEY_NSPIRE_ESC;
    break;
  case 6:
    t=KEY_NSPIRE_HOME;
    break;
  case 7:
    t=KEY_NSPIRE_MENU;
    break;
  case 12:
    t=KEY_NSPIRE_SHIFT;
    break;
  case 13:
    t=KEY_NSPIRE_CTRL;
    break;
  case 14:
    t=KEY_NSPIRE_SCRATCHPAD;
    break;
  case 15:
    t=KEY_NSPIRE_VAR;
    break;
  case 16:
    t=KEY_NSPIRE_DOC;
    break;
  case 17:
    t=KEY_NSPIRE_DEL;
    break;
  case 18:
    t=KEY_NSPIRE_eEXP;
    break;
  case 19:
    t=KEY_NSPIRE_EQU;
    break;
  case 20:
    t=KEY_NSPIRE_TENX;
    break;
  case 21:
    t=KEY_NSPIRE_I;
    break;
  case 22:
    t=KEY_NSPIRE_COMMA;
    break;
  case 23:
    t=KEY_NSPIRE_EXP;
    break;
  case 24:
    t=KEY_NSPIRE_TRIG;
    break;
  case 25:
    t=KEY_NSPIRE_C;
    break;
  case 26:
    t=KEY_NSPIRE_T;
    break;
  case 27:
    t=KEY_NSPIRE_PI;
    break;
  case 28:
    t=KEY_NSPIRE_S;
    break;
  case 29:
    t=KEY_NSPIRE_SQU;
    break;
  case 30:
    t=KEY_NSPIRE_7;
    break;
  case 31:
    t=KEY_NSPIRE_8;
    break;
  case 32:
    t=KEY_NSPIRE_9;
    break;
  case 33:
    t=KEY_NSPIRE_LP;
    break;
  case 34:
    t=KEY_NSPIRE_RP;
    break;
  case 36:
    t=KEY_NSPIRE_4;
    break;
  case 37:
    t=KEY_NSPIRE_5;
    break;
  case 38:
    t=KEY_NSPIRE_6;
    break;
  case 39:
    t=KEY_NSPIRE_MULTIPLY;
    break;
  case 40:
    t=KEY_NSPIRE_DIVIDE;
    break;
  case 42:
    t=KEY_NSPIRE_1;
    break;
  case 43:
    t=KEY_NSPIRE_2;
    break;
  case 44:
    t=KEY_NSPIRE_3;
    break;
  case 45:
    t=KEY_NSPIRE_PLUS;
    break;
  case 46:
    t=KEY_NSPIRE_MINUS;
    break;
  case 48:
    t=KEY_NSPIRE_0;
    break;
  case 49:
    t=KEY_NSPIRE_PERIOD;
    break;
  case 50:
    t=KEY_NSPIRE_EE;
    break;
  case 51:
    t=KEY_NSPIRE_NEGATIVE;
    break;
  case 52:
    t=KEY_NSPIRE_RET;
    break;
  }
  return isKeyPressed(t);
}


// ? see also ndless-sdk/thirdparty/nspire-io/arch-nspire/nspire.c nio_ascii_get
int getkey(int allow_suspend){
  sync_screen();
  if (shutdown_state)
    return KEY_SHUTDOWN;
  int lastkey=-1;
  unsigned NSPIRE_RTC_ADDR=0x90090000;
  static unsigned lastt=0;
  for (;;){
    unsigned t1= * (volatile unsigned *) NSPIRE_RTC_ADDR;
    if (lastt==0)
      lastt=t1;
    if (t1-lastt>10){
      display_time();
      sync_screen();
    }
    bool autosuspend=(t1-lastt>=100);
    if (nspire_exam_mode!=2 &&
	is_cx2 && nspire_ctrl && on_key_pressed()){
      os_fill_rect(50,90,200,40,0x1234);
      nspire_draw_string(60,120,0,0xffff,Regular12,"Quit KhiCAS to shutdown",false);
      nspire_ctrl=false;
      statusline(1);
      continue;
    }
    if ( (nspire_exam_mode==2 || !is_cx2) &&
	allow_suspend && (autosuspend || (nspire_ctrl && on_key_pressed()))){
      nspire_ctrl=nspire_shift=false;
      while (!autosuspend && on_key_pressed())
	loopsleep(10);
      // somewhat OFF by setting LCD to 0
      unsigned NSPIRE_CONTRAST_ADDR=is_cx2?0x90130014:0x900f0020;
      unsigned oldval=*(volatile unsigned *)NSPIRE_CONTRAST_ADDR,oldval2;
      if (is_cx2){
	oldval2=*(volatile unsigned *) (NSPIRE_CONTRAST_ADDR+4);
	*(volatile unsigned *) (NSPIRE_CONTRAST_ADDR+4)=0xffff;
      }
      *(volatile unsigned *)NSPIRE_CONTRAST_ADDR=is_cx2?0xffff:0x100;
      static volatile uint32_t *lcd_controller = (volatile uint32_t*) 0xC0000000;
      lcd_controller[6] &= ~(0b1 << 11);
      loopsleep(20);
      lcd_controller[6] &= ~ 0b1;
      unsigned offtime=* (volatile unsigned *) NSPIRE_RTC_ADDR;
      for (int n=0;!on_key_pressed();++n){
	loopsleep(100);
	idle();
	if (!exam_mode && nspire_exam_mode!=2 && shutdown
	    // && n&0xff==0
	    ){
	  unsigned curtime=* (volatile unsigned *) NSPIRE_RTC_ADDR;
	  if (curtime-offtime>7200){
	    shutdown_state=1;
	    // after 2 hours, leave KhiCAS
	    // that way the OS will really shutdown the calc
	    lcd_controller[6] |= 0b1;
	    loopsleep(20);
	    lcd_controller[6]|= 0b1 << 11;
	    if (is_cx2)
	      *(volatile unsigned *)(NSPIRE_CONTRAST_ADDR+4)=oldval2;
	    *(volatile unsigned *)NSPIRE_CONTRAST_ADDR=oldval;
	    statuslinemsg("Press ON to disable KhiCAS auto shutdown");
	    //os_fill_rect(0,0,320,222,0xffff);
	    sync_screen();
	    int m=0,mmax=150;
	    for (;m<mmax;++m){
	      if (on_key_pressed()){
		break;
	      }
	      loopsleep(100);
	      idle();
	    }
	    if (m==mmax){
	      if (shutdown())
		return KEY_SHUTDOWN;
	    }
	    else {
	      shutdown_state=0;
	      break;
	    }
	  }
	}
      }
      if (nspire_exam_mode==2){
	os_fill_rect(20,30,280,150,0x1234);
	os_fill_rect(25,45,270,121,0xffff);
	nspire_draw_string(25,60,0,0xffff,Regular12,"Mode examen de KhiCAS, avec CAS.",false);
	nspire_draw_string(25,77,0,0xffff,Regular12,"Mode conforme à la règlementation",false);
	nspire_draw_string(25,94,0,0xffff,Regular12,"du bac en France (les fichiers non",false);
	nspire_draw_string(25,111,0,0xffff,Regular12,"autorisés ont été effacés et les",false);
	nspire_draw_string(25,128,0,0xffff,Regular12,"sauvegardes sont desactivées).",false);
	nspire_draw_string(25,150,0,0xffff,Regular12,"Quitter KhiCAS (ou appuyer sur reset)",false);
	nspire_draw_string(25,167,0,0xffff,Regular12,"relancera le clignotement des leds.",false);
      }
      lcd_controller[6] |= 0b1;
      loopsleep(20);
      lcd_controller[6]|= 0b1 << 11;
      if (is_cx2)
	*(volatile unsigned *)(NSPIRE_CONTRAST_ADDR+4)=oldval2;
      *(volatile unsigned *)NSPIRE_CONTRAST_ADDR=oldval;
      statusline(0);
      sync_screen();
      lastt=* (volatile unsigned *) NSPIRE_RTC_ADDR;
      continue;
    }
    if (!any_key_pressed()){
      if (nspireemu)
	ck_msleep(50); // 100?
      else // real calculator
	ck_msleep(1);
      continue;
    }
    lastt=t1;
    int cursor_state=0;
    int i=0;
    if (isKeyPressed(KEY_NSPIRE_SHIFT)){
      while (i==0 && isKeyPressed(KEY_NSPIRE_SHIFT)){
	if (isKeyPressed(KEY_NSPIRE_LEFT)|| isKeyPressed(KEY_NSPIRE_LEFTUP) || isKeyPressed(KEY_NSPIRE_DOWNLEFT))
	  i=KEY_SELECT_LEFT;
	if (isKeyPressed(KEY_NSPIRE_RIGHT)|| isKeyPressed(KEY_NSPIRE_UPRIGHT) || isKeyPressed(KEY_NSPIRE_RIGHTDOWN))
	  i=KEY_SELECT_RIGHT;
	if (isKeyPressed(KEY_NSPIRE_UP))
	  i=KEY_SELECT_UP;
	if (isKeyPressed(KEY_NSPIRE_DOWN))
	  i=KEY_SELECT_DOWN;
      }
      if (i!=0){
	nspire_select=true;
      }
      if (i==0){
	nspire_shift=!nspire_shift;
	statusline(0);
	sync_screen();
	i=-1;
      }
    }
    else {
      if (nspire_select){
	nspire_select=nspire_shift=false;
	statusline(0);
	sync_screen();
	continue;
      }
      i=ascii_get(&cursor_state);
    }
    if (i<0){
      wait_no_key_pressed();
      continue;
    }
    if (i==KEY_CTRL_N){
      nspireemu=!nspireemu;
      nspire_ctrl=nspire_shift=false;
      statusline(0);
      sync_screen();
      continue;
    }
    if ( (i>=KEY_CTRL_LEFT && i<=KEY_CTRL_RIGHT) ||
	 (i>=KEY_UP_CTRL && i<=KEY_RIGHT_CTRL) ||
	 (i>=KEY_SELECT_LEFT && i<=KEY_SELECT_RIGHT) ||
	 i==KEY_CTRL_DEL){
      int delay=(lastkey==i)?5:60,j;
      for (j=0;j<delay && any_key_pressed();++j){
	if (nspireemu)
	  ck_msleep(14);
	else
	  ck_msleep(1);
      }
      if (any_key_pressed())
	lastkey=i;
      else 
	lastkey=-1;
    }
    else {
      wait_no_key_pressed();
      lastkey=-1;
    }
    if (nspire_ctrl || nspire_shift){
      nspire_ctrl=nspire_shift=false;
      statusline(0);
      sync_screen();
    }
    return i;
  }
  // void send_key_event(struct s_ns_event* eventbuf, unsigned short keycode_asciicode, BOOL is_key_up, BOOL unknown): since r721. Simulate a key event
}

// void idle(void)
// void msleep(unsigned ms)
// cfg_register_fileext(const char *ext, const char *prgm): (since v3.1 r797) associate for Ndless the file extension ext (without leading '.') to the program name prgm. Does nothing if the extension is already registered.

void GetKey(int * key){
  *key=getkey(true);
}

bool alphawasactive(int * key){
  if (*key==KEY_DOWN_CTRL){
    *key=KEY_CTRL_DOWN;
    return true;
  }
  if (*key==KEY_UP_CTRL){
    *key=KEY_CTRL_UP;
    return true;
  }
  if (*key==KEY_LEFT_CTRL){
    *key=KEY_CTRL_LEFT;
    return true;
  }
  if (*key==KEY_RIGHT_CTRL){
    *key=KEY_CTRL_RIGHT;
    return true;
  }
  return false;
}

bool isalphaactive(){
  return false;//nspire_ctrl;
}

void lock_alpha(){
  //nspire_ctrl=true;
}

void reset_kbd(){
  nspire_ctrl=nspire_shift=false;
}

bool on_key_enabled=true;

void enable_back_interrupt(){
  on_key_enabled=true;
}

void disable_back_interrupt(){
  on_key_enabled=false;
}
#else // NSPIRE_NEWLIB

void set_exam_mode(int i){
  exam_mode=i;
}
#endif // NSPIRE_NEWLIB
