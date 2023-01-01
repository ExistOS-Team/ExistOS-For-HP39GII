// -*- mode:C++ ; compile-command: "g++-3.4 -I. -I.. -g -c Equation.cc -DHAVE_CONFIG_H -DIN_GIAC -Wall" -*-
/*
 *  Copyright (C) 2005,2014 B. Parisse, Institut Fourier, 38402 St Martin d'Heres
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "config.h"
#include "giacPCH.h"
#if defined HAVE_UNISTD_H && !defined NUMWORKS && !defined HP39
#include <dirent.h>
#endif
#ifdef NSPIRE_NEWLIB
#include <fstream>
#include <libndls.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <os.h>
#include <syscall.h>
#include "sha256.h"
#endif
#include <alloca.h>
#ifndef is_cx2
#define is_cx2 false
#endif
int osok=1;
extern "C" int shell_x,shell_y,shell_fontw,shell_fonth;
#ifdef HP39
extern "C" char Setup_GetEntry(unsigned int index);
#define MINI_OVER 0
#define MINI_REV 1
int shell_x=0,shell_y=0,shell_fontw=7,shell_fonth=14;
int fileBrowser(char* filename, char* filter, char* title);
#define _green 0
#define _red 0
#define dbgprintf printf
#else
int shell_x=0,shell_y=0,shell_fontw=12,shell_fonth=18;
#define _green _GREEN
#define _red _red
#define dbgprintf(...) 
#endif 
// pour le mode examen cx2, il y a 2 endroits ou is_cx2 est utilise dans smallmenu.selection==1
// soit par extinction des leds (marche avec OS 5.2)
// soit comme sur la CX si l'ecriture en flash NAND marche un jour

#ifdef KHICAS

#ifdef NUMWORKS
char * freeptr=0;
#ifndef DEVICE
const char * flash_buf=file_gettar_aligned("apps.tar",freeptr);
extern "C" const char * flash_read(const char * filename){
  return tar_loadfile(flash_buf,filename,0);
}
extern "C" int flash_filebrowser(const char ** filenames,int maxrecords,const char * extension){
  return tar_filebrowser(flash_buf,filenames,maxrecords,extension);
}
#else
const char * flash_buf=(const char *)0x90200000;
#endif
#endif

#if defined NUMWORKS && !defined DEVICE //ndef NSPIRE_NEWLIB
extern "C" {
  short int nspire_exam_mode=0;
}
#endif
#define XWASPY 1 // save .xw file as _xw.py (to be recognized by Numworks workshop)
const int xwaspy_shift=33; // must be between 32 and 63, reflect in xcas.js and History.cc
#include "kdisplay.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include "input_lexer.h"
#include "input_parser.h"
#if defined NUMWORKS && defined DEVICE
  void py_ck_ctrl_c(){
    if (giac::ctrl_c || giac::interrupted)
      raisememerr();
  }
#else
  void py_ck_ctrl_c(){}
#endif
//giac::context * contextptr=0;
int clip_ymin=0;
int lang=1;
short int nspirelua=0;
bool warn_nr=true; 
bool xthetat=false;
//bool freezeturtle=false;
bool global_show_axes=true;
int esc_flag=0;
int xcas_python_eval=0;
char * python_heap=0;

#ifdef QUICKJS
#include "qjsgiac.h"
#endif

#ifdef MICROPY_LIB
extern "C" int mp_token(const char * line);

void python_free(){
  if (!python_heap) return;
  mp_deinit(); free(python_heap); python_heap=0;
}

int python_init(int stack_size,int heap_size){
#if 1 // defined NUMWORKS
  python_free();
  python_heap=micropy_init(stack_size,heap_size);
  if (!python_heap)
    return 0;
#endif
  return 1;
}

int micropy_ck_eval(const char *line){
#if 1 // def NUMWORKS
  giac::ctrl_c=giac::interrupted=false;
  if (python_heap && line[0]==0)
    return 1;
  if (!python_heap){
    python_init(pythonjs_stack_size,pythonjs_heap_size);
  }
  if (!python_heap){
    console_output("Memory full",11);
    return RAND_MAX;
  }
#endif
  return micropy_eval(line);
  // if MP_PARSE_SINGLE_INPUT is used, split input if newline not followed by a space, return shift
  int shift=0,nl=0;
  const char * ptr=line;
  for (;;++ptr){
    if (*ptr=='\n')
      ++nl;
    if (*ptr==0 || (*ptr=='\n' && *(ptr+1)!=' ')){
      int n=ptr-line;
      char buf[n+1];
      strncpy(buf,line,n);
      buf[n]=0;
      micropy_eval(buf);
      if (parser_errorline)
	return shift;
      if (*ptr==0)
	return 0;
      line=ptr+1;
      shift=nl;
    }
  }
  return 0;
}
#endif

using namespace std;
using namespace giac;
#ifdef HP39
const int LCD_WIDTH_PX=256;
const int LCD_HEIGHT_PX=127;
#else
const int LCD_WIDTH_PX=320;
const int LCD_HEIGHT_PX=222;
#endif
char* fmenu_cfg=0;
int khicas_addins_menu(GIAC_CONTEXT); // in kadd.cc
#ifdef MICROPY_LIB
extern "C" const char * const * mp_vars();
#endif
#if defined NUMWORKS && defined DEVICE
extern "C" void extapp_clipboardStore(const char *text);
extern "C" const char * extapp_clipboardText();
#endif

// Numworks Logo commands
#ifndef NO_NAMESPACE_GIAC
namespace giac {
#endif // ndef NO_NAMESPACE_GIAC
  void Bdisp_PutDisp_DD(){
    sync_screen();
  }
  void Bdisp_AllClr_VRAM(){
    waitforvblank();
    drawRectangle(0,0,LCD_WIDTH_PX,LCD_HEIGHT_PX,_WHITE);
  }
  void drawLine(int x1,int y1,int x2,int y2,int c){
    draw_line(x1,y1,x2,y2,c,context0);
  }
  void stroke_rectangle(int x,int y,int w,int h,int c){
    drawLine(x,y,x+w,y,c);
    drawLine(x,y+h,x+w,y+h,c);
    drawLine(x,y,x,y+h,c);
    drawLine(x+w,y,x+w,y+h,c);
  }
  void DefineStatusMessage(const char * s,int a,int b,int c){
    statuslinemsg(s);
  }

  void DisplayStatusArea(){
    sync_screen();
  }

  void set_xcas_status(){
    statusline(1+2*xcas_python_eval);
  }
  int GetSetupSetting(int mode){
    return 0;
  }

  void SetSetupSetting(int mode,int){
  }

  void handle_f5(){
    lock_alpha();
  }
  
  int chartab(){
    static int row=0,col=0;
    for (;;){
      int cur=32+16*row+col;
      col &= 0xf;
      if (row<0) row=5; else if (row>5) row=0;
      // display table
      drawRectangle(0,0,LCD_WIDTH_PX,LCD_HEIGHT_PX,_WHITE);
      os_draw_string_medium(0,0,_BLACK,_WHITE,lang==1?"Selectionner caractere":"Select char");
#ifdef HP39
      int dy=12;
      for (int r=0;r<6;++r){
        for (int c=0;c<16;++c){
          int currc=32+16*r+c;
          unsigned char buf[2]={currc==127?(unsigned char)'X':(unsigned char)currc,0};
          os_draw_string(12*c,dy+16*r,cur==currc?_WHITE:_BLACK,cur==currc?_BLACK:_WHITE,buf);
        }
      }
#else
      for (int r=0;r<6;++r){
        for (int c=0;c<16;++c){
          char buf[2]={char(32+16*r+c),0}; 
          os_draw_string(20*c,20+20*r,_BLACK,(r==row && c==col?color_gris:_WHITE),buf);
        }
      }
#endif
      string s("Current ");
      s += char(cur);
      s += " ";
      s += print_INT_(cur);
      s += " ";
      s += hexa_print_INT_(cur);
#ifdef HP39
      os_draw_string_medium(0,112,_BLACK,_WHITE,(const unsigned char *)s.c_str());
#else      
      os_draw_string(0,160,_BLACK,_WHITE,s.c_str());
      os_draw_string(0,180,_BLACK,_WHITE,lang==1?"EXE: copier caractere":"EXE: copy char");
#endif
      // interaction
      int key=getkey(1);
      //dbgprintf("key %i %i\n",key,cur);
      if (key==KEY_CTRL_EXIT)
        return -1;
      if (key==KEY_CTRL_OK || key==KEY_CTRL_EXE)
        return cur;
      if (key==KEY_CTRL_LEFT)
        --col;
      if (key==KEY_CTRL_RIGHT)
        ++col;
      if (key==KEY_CTRL_UP)
        --row;
      if (key==KEY_CTRL_DOWN)
        ++row;
    }
  }

  void delete_clipboard(){}

  bool clip_pasted=true;
  
  string * clipboard(){
    static string * ptr=0;
    if (!ptr)
      ptr=new string;
    return ptr;
  }
  
  void copy_clipboard(const string & s,bool status){
    dbgprintf("clip %s\n",s.c_str());
#if defined NUMWORKS && defined DEVICE
    extapp_clipboardStore(s.c_str());
#else
    if (1 || clip_pasted) // adding to clipboard is sometimes annoying
      *clipboard()=s;
    else
      *clipboard()+=s;
#endif
    clip_pasted=false;
    if (status){
      DefineStatusMessage((char*)((lang==1)?"Selection copiee vers presse-papiers.":"Selection copied to clipboard"), 1, 0, 0);
      DisplayStatusArea();
    }
  }
  
  const char * paste_clipboard(){
    dbgprintf("clip %s\n",clipboard()->c_str());
    clip_pasted=true;
#if defined NUMWORKS && defined DEVICE
    return extapp_clipboardText();
#endif
    return clipboard()->c_str();
  }
  
  int print_msg12(const char * msg1,const char * msg2,int textY=40){
    drawRectangle(0, textY+10, LCD_WIDTH_PX, 44, COLOR_WHITE);
    drawRectangle(3,textY+10,316,3, COLOR_BLACK);
    drawRectangle(3,textY+10,3,44, COLOR_BLACK);
    drawRectangle(316,textY+10,3,44, COLOR_BLACK);
    drawRectangle(3,textY+54,316,3, COLOR_BLACK);
    int textX=30;
    if (msg1){
      if (strlen(msg1)>=30)
	os_draw_string_small_(textX,textY+15,msg1);
      else
	os_draw_string_(textX,textY+15,msg1);
    }
    textX=10;
    textY+=33;
    if (msg2){
      if (strlen(msg2)>=30)
	os_draw_string_small_(textX,textY,msg2);
      else      
	textX=os_draw_string_(textX,textY,msg2);
    }
    return textX;
  }
  
  void insert(string & s,int pos,const char * add){
    if (pos>s.size())
      pos=s.size();
    if (pos<0)
      pos=0;
    s=s.substr(0,pos)+add+s.substr(pos,s.size()-pos);
  }
  
  bool do_confirm(const char * s){
#ifdef NSPIRE_NEWLIB
    return confirm(s,((lang==1)?"enter: oui,  esc:annuler":"enter: yes,   esc: cancel"))==KEY_CTRL_F1;
#else
    return confirm(s,((lang==1)?"OK: oui,  Back:annuler":"OK: yes,   Back: cancel"))==KEY_CTRL_F1;
#endif
  }
  
  int confirm(const char * msg1,const char * msg2,bool acexit,int y){
    int key=0;
    print_msg12(msg1,msg2,y);
    while (key!=KEY_CTRL_F1 && key!=KEY_CTRL_F6){
      GetKey(&key);
      if (key==KEY_SHUTDOWN)
	return key;
      if (key==KEY_CTRL_EXE || key==KEY_CTRL_OK || key==KEY_CHAR_CR)
	key=KEY_CTRL_F1;
      if (key==KEY_CTRL_AC || key==KEY_CTRL_EXIT || key==KEY_CTRL_MENU){
	if (acexit) return -1;
	key=KEY_CTRL_F6;
      }
      set_xcas_status();
    }
    return key;
  }  
  
  bool confirm_overwrite(){
#ifdef NSPIRE_NEWLIB
    return do_confirm((lang==1)?"enter: oui,  esc:annuler":"enter: yes,   esc: cancel")==KEY_CTRL_F1;
#else
    return do_confirm((lang==1)?"OK: oui,  Back:annuler":"OK: yes,   Back: cancel")==KEY_CTRL_F1;
#endif
  }
  
  void invalid_varname(){
    confirm((lang==1)?"Nom de variable incorrect":"Invalid variable name",
#ifdef NSPIRE_NEWLIB
	    (lang==1)?"enter: ok":"enter: ok"
#else
	    (lang==1)?"OK: ok":"OK: ok"
#endif
	    );
  }


#ifdef SCROLLBAR
  typedef scrollbar TScrollbar;
#endif

#ifdef HP39
#define C24 16 // 24 on 90
#define C18 16 // 18
#define C10 8 // 18
#define C6 6 // 6
#else
#define C24 18 // 24 on 90
#define C18 18 // 18
#define C10 10 // 18
#define C6 6 // 6
#endif

  int MB_ElementCount(const char * s){
    return strlen(s); // FIXME for UTF8
  }

  void PrintXY(int x,int y,const char * s,int mode,int c=giac::_BLACK,int bg=giac::_WHITE){
    if (mode==TEXT_MODE_NORMAL)
      os_draw_string(x,y,c,bg,s);
    else {
#ifndef HP39
      if (c==giac::_BLACK && bg==giac::_WHITE)
	os_draw_string(x,y,c,color_gris,s);
      else
#endif
	os_draw_string(x,y,bg,c,s);
    }
  }

  int PrintMiniMini(int x,int y,const char * s,int mode,int c=giac::_BLACK,int bg=giac::_WHITE,bool fake=false){
    if (mode==TEXT_MODE_NORMAL)
      return os_draw_string_small(x,y,c,bg,s,fake);
    else {
#ifndef HP39      
      if (c==giac::_BLACK && bg==giac::_WHITE)
	return os_draw_string_small(x,y,c,color_gris,s,fake);
      else
#endif
	return os_draw_string_small(x,y,bg,c,s,fake);	
    }
  }
  
  int PrintMini(int x,int y,const char * s,int mode,int c=giac::_BLACK,int bg=giac::_WHITE,bool fake=false){
    if (mode==TEXT_MODE_NORMAL)
      return os_draw_string_medium(x,y,c,bg,s,fake);
    else {
#ifndef HP39
      if (c==giac::_BLACK && bg==giac::_WHITE)
	return os_draw_string_medium(x,y,c,color_gris,s,fake);
      else
#endif
	return os_draw_string_medium(x,y,bg,c,s,fake);
    }
  }
  
  void printCentered(const char* text, int y) {
    int len = strlen(text);
    int x = LCD_WIDTH_PX/2-(len*6)/2;
    PrintXY(x,y,text,0);
  }

  int doMenu(Menu* menu, MenuItemIcon* icontable) { // returns code telling what user did. selection is on menu->selection. menu->selection starts at 1!
    int itemsStartY=menu->startY; // char Y where to start drawing the menu items. Having a title increases this by one
    int itemsHeight=menu->height;
    int showtitle = menu->title != NULL;
    if (showtitle) {
      itemsStartY++;
      itemsHeight--;
    }
    char keyword[5];
    keyword[0]=0;
    if(menu->selection > menu->scroll+(menu->numitems>itemsHeight ? itemsHeight : menu->numitems))
      menu->scroll = menu->selection -(menu->numitems>itemsHeight ? itemsHeight : menu->numitems);
    if(menu->selection-1 < menu->scroll)
      menu->scroll = menu->selection -1;
  
    while(1) {
      // Cursor_SetFlashOff();
      if (menu->selection <=1)
        menu->selection=1;
      if (menu->selection > menu->scroll+(menu->numitems>itemsHeight ? itemsHeight : menu->numitems))
        menu->scroll = menu->selection -(menu->numitems>itemsHeight ? itemsHeight : menu->numitems);
      if (menu->selection-1 < menu->scroll)
        menu->scroll = menu->selection -1;
      if(menu->statusText != NULL) DefineStatusMessage(menu->statusText, 1, 0, 0);
      // Clear the area of the screen we are going to draw on
      if(0 == menu->pBaRtR) {
        int x=C10*menu->startX-1,
          y=C24*(menu->miniMiniTitle ? itemsStartY:menu->startY)-1,
          w=2+C10*menu->width /* + ((menu->scrollbar && menu->scrollout)?C10:0) */,
          h=2+C24*menu->height-(menu->miniMiniTitle ? C24:0);
        // drawRectangle(x, y, w, h, COLOR_WHITE);
        draw_line(x,y,x+w,y,COLOR_BLACK,context0);
        draw_line(x,y+h,x+w,y+h,COLOR_BLACK,context0);
        draw_line(x,y,x,y+h,COLOR_BLACK,context0);
        draw_line(x+w,y,x+w,y+h,COLOR_BLACK,context0);
      }
      if (menu->numitems>0) {
        for(int curitem=0; curitem < menu->numitems; curitem++) {
          // print the menu item only when appropriate
          if(menu->scroll <= curitem && menu->scroll > curitem-itemsHeight) {
            if ((curitem-menu->scroll) % 6==0)
              waitforvblank();
            char menuitem[256] = "";
            if(menu->numitems>=100 || menu->type == MENUTYPE_MULTISELECT){
              strcpy(menuitem, "  "); //allow for the folder and selection icons on MULTISELECT menus (e.g. file browser)
              strcpy(menuitem+2,menu->items[curitem].text);
            }
            else if (menu->type==MENUTYPE_NO_NUMBER)
              strcpy(menuitem,menu->items[curitem].text);
            else {
              int cur=curitem+1;
              if (menu->numitems<10){
                menuitem[0]='0'+cur;
                menuitem[1]=' ';
                menuitem[2]=0;
              }
              else {
                menuitem[0]=cur>=10?('0'+(cur/10)):' ';
                menuitem[1]='0'+(cur%10);
                menuitem[2]=' ';
                menuitem[3]=0;
              }
              strncat(menuitem, menu->items[curitem].text, 250);
            }
            if(menu->items[curitem].type != MENUITEM_SEPARATOR) {
              //make sure we have a string big enough to have background when item is selected:          
              // MB_ElementCount is used instead of strlen because multibyte chars count as two with strlen, while graphically they are just one char, making fillerRequired become wrong
              int fillerRequired = menu->width - MB_ElementCount(menu->items[curitem].text) - (menu->type == MENUTYPE_MULTISELECT ? 2 : 3);
              for(int i = 0; i < fillerRequired; i++)
                strcat(menuitem, " ");
              //dbgprintf("menu %i %i\n",curitem,C10*menu->width);
              drawRectangle(C10*menu->startX,C18*(curitem+itemsStartY-menu->scroll),C10*menu->width,C24,(menu->selection == curitem+1 ? color_gris : _WHITE));
              PrintXY(C10*menu->startX,C18*(curitem+itemsStartY-menu->scroll),menuitem, (menu->selection == curitem+1 ? TEXT_MODE_INVERT : TEXT_MODE_NORMAL));
            } else {
              /*int textX = (menu->startX-1) * C18;
                int textY = curitem*C24+itemsStartY*C24-menu->scroll*C24-C24+C10;
                clearLine(menu->startX, curitem+itemsStartY-menu->scroll, (menu->selection == curitem+1 ? textColorToFullColor(menu->items[curitem].color) : COLOR_WHITE));
                drawLine(textX, textY+C24-4, LCD_WIDTH_PX-2, textY+C24-4, COLOR_GRAY);
                PrintMini(&textX, &textY, (unsigned char*)menuitem, 0, 0xFFFFFFFF, 0, 0, (menu->selection == curitem+1 ? COLOR_WHITE : textColorToFullColor(menu->items[curitem].color)), (menu->selection == curitem+1 ? textColorToFullColor(menu->items[curitem].color) : COLOR_WHITE), 1, 0);*/
            }
            // deal with menu items of type MENUITEM_CHECKBOX
            if(menu->items[curitem].type == MENUITEM_CHECKBOX) {
              PrintXY(C10*(menu->startX+menu->width-4),C18*(curitem+itemsStartY-menu->scroll),
                      (menu->items[curitem].value == MENUITEM_VALUE_CHECKED ? " [+]" : " [-]"),
                      (menu->selection == curitem+1 ? TEXT_MODE_INVERT : (menu->pBaRtR == 1? TEXT_MODE_NORMAL : TEXT_MODE_NORMAL)));
            }
            // deal with multiselect menus
            if(menu->type == MENUTYPE_MULTISELECT) {
              if((curitem+itemsStartY-menu->scroll)>=itemsStartY &&
                 (curitem+itemsStartY-menu->scroll)<=(itemsStartY+itemsHeight) &&
                 icontable != NULL
                 ) {
#if 0
                if (menu->items[curitem].isfolder == 1) {
                  // assumes first icon in icontable is the folder icon
                  CopySpriteMasked(icontable[0].data, (menu->startX)*C18, (curitem+itemsStartY-menu->scroll)*C24, 0x12, 0x18, 0xf81f  );
                } else {
                  if(menu->items[curitem].icon >= 0) CopySpriteMasked(icontable[menu->items[curitem].icon].data, (menu->startX)*C18, (curitem+itemsStartY-menu->scroll)*C24, 0x12, 0x18, 0xf81f  );
                }
#endif
              }
              if (menu->items[curitem].isselected) {
                if (menu->selection == curitem+1) {
                  PrintXY(C10*menu->startX,C18*(curitem+itemsStartY-menu->scroll),"\xe6\x9b", TEXT_MODE_NORMAL);
                } else {
                  PrintXY(C10*menu->startX,C18*(curitem+itemsStartY-menu->scroll),"\xe6\x9b", TEXT_MODE_NORMAL);
                }
              }
            }
          }
        } // end for curitem<menu->numitem
        int dh=menu->height-menu->numitems-(showtitle?1:0);
        if (dh>0)
          drawRectangle(C10*menu->startX,C24*(menu->numitems+(showtitle?1:0)),C10*menu->width,C24*dh,_WHITE);
        if (menu->scrollbar) {
#ifdef SCROLLBAR
          TScrollbar sb;
          sb.I1 = 0;
          sb.I5 = 0;
          sb.indicatormaximum = menu->numitems;
          sb.indicatorheight = itemsHeight;
          sb.indicatorpos = menu->scroll;
          sb.barheight = itemsHeight*C24;
          sb.bartop = (itemsStartY-1)*C24;
          sb.barleft = menu->startX*C18+menu->width*C18 - C18 - (menu->scrollout ? 0 : 5);
          sb.barwidth = C10;
          Scrollbar(&sb);
#endif
        }
        //if(menu->type==MENUTYPE_MULTISELECT && menu->fkeypage == 0) drawFkeyLabels(0x0037); // SELECT (white)
      } else {
        giac::printCentered(menu->nodatamsg, (itemsStartY*C24)+(itemsHeight*C24)/2-12);
      }
      if(showtitle) {
        int textX = C10*menu->startX, textY=menu->startY*C24;
        drawRectangle(textX,textY,C10*menu->width,C24,_WHITE);
        if (menu->miniMiniTitle) 
          PrintMini( textX, textY, menu->title, 0 );
        else
          PrintXY(textX, textY, menu->title, TEXT_MODE_NORMAL);
        if(menu->subtitle != NULL) {
          int textX=(MB_ElementCount(menu->title)+menu->startX-1)*C18+C10, textY=C10;
          PrintMini(textX, textY, menu->subtitle, 0);
        }
        int xpos=textX+C10*(menu->width-5);
        PrintXY(xpos, 1, "____", 0);
        PrintXY(xpos, 1, keyword, 0);
      }
      /*if(menu->darken) {
	DrawFrame(COLOR_BLACK);
	VRAMInvertArea(menu->startX*C18-C18, menu->startY*C24, menu->width*C18-(menu->scrollout || !menu->scrollbar ? 0 : 5), menu->height*C24);
	}*/
      if(menu->type == MENUTYPE_NO_KEY_HANDLING) return MENU_RETURN_INSTANT; // we don't want to handle keys
      int key;
      GetKey(&key);
      if (key==KEY_SHUTDOWN)
	return key;
      if (key==KEY_CTRL_MENU){
	menu->selection=menu->numitems;
	return MENU_RETURN_SELECTION;
      }
      if (key<256 && isalpha(key)){
	key=tolower(key);
	int pos=strlen(keyword);
	if (pos>=4)
	  pos=0;
	keyword[pos]=key;
	keyword[pos+1]=0;
	int cur=0;
	for (;cur<menu->numitems;++cur){
#if 1
	  if (strcmp(menu->items[cur].text,keyword)>=0)
	    break;
#else
	  char c=menu->items[cur].text[0];
	  if (key<=c)
	    break;
#endif
	}
	if (cur<menu->numitems){
	  menu->selection=cur+1;
	  if(menu->selection > menu->scroll+(menu->numitems>itemsHeight ? itemsHeight : menu->numitems))
	    menu->scroll = menu->selection -(menu->numitems>itemsHeight ? itemsHeight : menu->numitems);
	  if(menu->selection-1 < menu->scroll)
	    menu->scroll = menu->selection -1;
	}
	continue;
      }
      switch(key) {
      case KEY_CTRL_PAGEDOWN:
	menu->selection+=6;
	if (menu->selection >= menu->numitems)
	  menu->selection=menu->numitems;
	if(menu->selection > menu->scroll+(menu->numitems>itemsHeight ? itemsHeight : menu->numitems))
	  menu->scroll = menu->selection -(menu->numitems>itemsHeight ? itemsHeight : menu->numitems);
	break;
      case KEY_CTRL_DOWN:
	if(menu->selection == menu->numitems)
	  {
	    if(menu->returnOnInfiniteScrolling) {
	      return MENU_RETURN_SCROLLING;
	    } else {
	      menu->selection = 1;
	      menu->scroll = 0;
	    }
	  }
	else
	  {
	    menu->selection++;
	    if(menu->selection > menu->scroll+(menu->numitems>itemsHeight ? itemsHeight : menu->numitems))
	      menu->scroll = menu->selection -(menu->numitems>itemsHeight ? itemsHeight : menu->numitems);
	  }
	if(menu->pBaRtR==1) return MENU_RETURN_INSTANT;
	break;
      case KEY_CTRL_PAGEUP:
	menu->selection-=6;
	if (menu->selection <=1)
	  menu->selection=1;
	if(menu->selection-1 < menu->scroll)
	  menu->scroll = menu->selection -1;
	break;
      case KEY_CTRL_UP:
	if(menu->selection == 1)
	  {
	    if(menu->returnOnInfiniteScrolling) {
	      return MENU_RETURN_SCROLLING;
	    } else {
	      menu->selection = menu->numitems;
	      menu->scroll = menu->selection-(menu->numitems>itemsHeight ? itemsHeight : menu->numitems);
	    }
	  }
	else
	  {
	    menu->selection--;
	    if(menu->selection-1 < menu->scroll)
	      menu->scroll = menu->selection -1;
	  }
	if(menu->pBaRtR==1) return MENU_RETURN_INSTANT;
	break;
      case KEY_CTRL_F1:
	if(menu->type==MENUTYPE_MULTISELECT && menu->fkeypage == 0 && menu->numitems > 0) {
          /*if(menu->items[menu->selection-1].isselected) {
            menu->items[menu->selection-1].isselected=0;
            menu->numselitems = menu->numselitems-1;
	    } else {
            menu->items[menu->selection-1].isselected=1;
            menu->numselitems = menu->numselitems+1;
	    }
	    return key; //return on F1 too so that parent subroutines have a chance to e.g. redraw fkeys*/
	} else if (menu->type == MENUTYPE_FKEYS || menu->type==MENUTYPE_NO_NUMBER) {
	  return key;
	}
	break;
      case KEY_CTRL_F2:
      case KEY_CTRL_F3:
      case KEY_CTRL_F4:
      case KEY_CTRL_F5:
      case KEY_CTRL_F6: case KEY_CTRL_CATALOG: case KEY_BOOK: case '\t':
      case KEY_CHAR_ANS: 
	if (menu->type == MENUTYPE_FKEYS || menu->type==MENUTYPE_NO_NUMBER || menu->type==MENUTYPE_MULTISELECT) return key; // MULTISELECT also returns on Fkeys
	break;
      case KEY_CTRL_PASTE:
	if (menu->type==MENUTYPE_MULTISELECT) return key; // MULTISELECT also returns on paste
      case KEY_CTRL_OPTN:
	if (menu->type==MENUTYPE_FKEYS || menu->type==MENUTYPE_MULTISELECT) return key;
	break;
      case KEY_CTRL_FORMAT:
	if (menu->type==MENUTYPE_FKEYS) return key; // return on the Format key so that event lists can prompt to change event category
	break;
      case KEY_CTRL_RIGHT:
	if(menu->type != MENUTYPE_MULTISELECT) return KEY_BOOK; // break;
	// else fallthrough
      case KEY_CTRL_EXE: case KEY_CTRL_OK: case KEY_CHAR_CR:
	if(menu->numitems>0) return key==KEY_CTRL_OK?MENU_RETURN_SELECTION:key;
	break;
      case KEY_CTRL_LEFT:
	if(menu->type != MENUTYPE_MULTISELECT) break;
	// else fallthrough
      case KEY_CTRL_DEL:
	if (strlen(keyword))
	  keyword[strlen(keyword)-1]=0;
	else {
	  if (strcmp(menu->title,"Variables")==0)
	    return key;
	}
	break;
      case KEY_CTRL_AC:
	if (strlen(keyword)){
	  keyword[0]=0;
	  lock_alpha();//SetSetupSetting( (unsigned int)0x14, 0x88);	
	  //DisplayStatusArea();
	  break;
	}
      case KEY_CTRL_EXIT: 
	return MENU_RETURN_EXIT;
	break;
      case KEY_CHAR_1:
      case KEY_CHAR_2:
      case KEY_CHAR_3:
      case KEY_CHAR_4:
      case KEY_CHAR_5:
      case KEY_CHAR_6:
      case KEY_CHAR_7:
      case KEY_CHAR_8:
      case KEY_CHAR_9:
	if (menu->type==MENUTYPE_NO_NUMBER)
	  return key;
	if(menu->numitems>=(key-0x30)) {
	  menu->selection = (key-0x30);
	  if (menu->type != MENUTYPE_FKEYS) return MENU_RETURN_SELECTION;
	}
	break;
      case KEY_CHAR_0:
	if (menu->type==MENUTYPE_NO_NUMBER)
	  return key;
	if(menu->numitems>=10) {
	  menu->selection = 10;
	  if (menu->type != MENUTYPE_FKEYS)  return MENU_RETURN_SELECTION;
	}
	break;
      case KEY_CHAR_EXPN:
	if(menu->numitems>=11) {
	  menu->selection = 11;
	  if (menu->type != MENUTYPE_FKEYS)  return MENU_RETURN_SELECTION;
	}
	break;
      case KEY_CHAR_LN:
	if(menu->numitems>=12) {
	  menu->selection = 12;
	  if (menu->type != MENUTYPE_FKEYS)  return MENU_RETURN_SELECTION;
	}
	break;
      case KEY_CHAR_LOG:
	if(menu->numitems>=13) {
	  menu->selection = 13;
	  if (menu->type != MENUTYPE_FKEYS)  return MENU_RETURN_SELECTION;
	}
	break;
      case KEY_CHAR_IMGNRY:
	if(menu->numitems>=14) {
	  menu->selection = 14;
	  if (menu->type != MENUTYPE_FKEYS)  return MENU_RETURN_SELECTION;
	}
	break;
      case KEY_CHAR_COMMA:
	if(menu->numitems>=15) {
	  menu->selection = 15;
	  if (menu->type != MENUTYPE_FKEYS)  return MENU_RETURN_SELECTION;
	}
	break;
      case KEY_CHAR_POW:
	if(menu->numitems>=16) {
	  menu->selection = 16;
	  if (menu->type != MENUTYPE_FKEYS)  return MENU_RETURN_SELECTION;
	}
	break;
      case KEY_CHAR_SIN:
      case KEY_CHAR_COS:
      case KEY_CHAR_TAN:
	if(menu->numitems>=(key-112)) {
	  menu->selection = (key-112);
	  if (menu->type != MENUTYPE_FKEYS)  return MENU_RETURN_SELECTION;
	}
	break;
      case KEY_CHAR_PI:
	if(menu->numitems>=20) {
	  menu->selection = 20;
	  if (menu->type != MENUTYPE_FKEYS)  return MENU_RETURN_SELECTION;
	}
	break;
      case KEY_CHAR_ROOT:
	if(menu->numitems>=21) {
	  menu->selection = 21;
	  if (menu->type != MENUTYPE_FKEYS)  return MENU_RETURN_SELECTION;
	}
	break;
      case KEY_CHAR_SQUARE:
	if(menu->numitems>=22) {
	  menu->selection = 22;
	  if (menu->type != MENUTYPE_FKEYS)  return MENU_RETURN_SELECTION;
	}
	break;
      case KEY_CHAR_LPAR:
      case KEY_CHAR_RPAR:
	if(menu->numitems>=(key-17)) {
	  menu->selection = (key-17);
	  if (menu->type != MENUTYPE_FKEYS)  return MENU_RETURN_SELECTION;
	}
	break;
      }
    }
    return MENU_RETURN_EXIT;
  }

#define CAT_CATEGORY_ALL 0
#define CAT_CATEGORY_ALGEBRA 1
#define CAT_CATEGORY_LINALG 2
#define CAT_CATEGORY_CALCULUS 3
#define CAT_CATEGORY_ARIT 4
#define CAT_CATEGORY_COMPLEXNUM 5
#define CAT_CATEGORY_PLOT 6
#define CAT_CATEGORY_POLYNOMIAL 7
#define CAT_CATEGORY_PROBA 8
#define CAT_CATEGORY_PROGCMD 9
#define CAT_CATEGORY_REAL 10
#define CAT_CATEGORY_SOLVE 11
#define CAT_CATEGORY_STATS 12
#define CAT_CATEGORY_TRIG 13
#define CAT_CATEGORY_OPTIONS 14
#define CAT_CATEGORY_LIST 15
#define CAT_CATEGORY_MATRIX 16
#define CAT_CATEGORY_PROG 17
#define CAT_CATEGORY_SOFUS 18
#define CAT_CATEGORY_PHYS 19
#define CAT_CATEGORY_UNIT 20
#define CAT_CATEGORY_2D 21
#define CAT_CATEGORY_3D 22
#define CAT_CATEGORY_LOGO 23 // should be the last one
#define XCAS_ONLY 0x80000000
  void init_locale(){
    lang=1;
  }

  const catalogFunc completeCatfr[] = { // list of all functions (including some not in any category)
    // {"cosh(x)", 0, "Hyperbolic cosine of x.", 0, 0, CAT_CATEGORY_TRIG},
    // {"exp(x)", 0, "Renvoie e^x.", "1.2", 0, CAT_CATEGORY_REAL},
    // {"log(x)", 0, "Logarithme naturel de x.", 0, 0, CAT_CATEGORY_REAL},
    // {"sinh(x)", 0, "Hyperbolic sine of x.", 0, 0, CAT_CATEGORY_TRIG},
    // {"tanh(x)", 0, "Hyperbolic tangent of x.", 0, 0, CAT_CATEGORY_TRIG},
    {" boucle for (pour)", "for ", "Boucle definie pour un indice variant entre 2 valeurs fixees", "#\nfor ", 0, CAT_CATEGORY_PROG},
    {" boucle liste", "for in", "Boucle sur tous les elements d'une liste.", "#\nfor in", 0, CAT_CATEGORY_PROG},
    {" boucle while (tantque)", "while ", "Boucle indefinie tantque.", "#\nwhile ", 0, CAT_CATEGORY_PROG},
    {" test si alors", "if ", "Test", "#\nif ", 0, CAT_CATEGORY_PROG},
    {" test sinon", "else ", "Clause fausse du test", 0, 0, CAT_CATEGORY_PROG},
    {" fonction def.", "f(x):=", "Definition de fonction.", "#\nf(x):=", 0, CAT_CATEGORY_PROG},
    {" local j,k;", "local ", "Declaration de variables locales Xcas", 0, 0, CAT_CATEGORY_PROG | XCAS_ONLY},
    {" range(a,b)", "in range(", "Dans l'intervalle [a,b[ (a inclus, b exclus)", "# in range(1,10)", 0, CAT_CATEGORY_PROG},
    {" return res;", "return ", "return ou retourne quitte la fonction et renvoie le resultat res", 0, 0, CAT_CATEGORY_PROG},
    //{" edit list ", "list(", "Assistant creation de liste.", 0, 0, CAT_CATEGORY_LIST},
    //{" edit matrix ", "matrix(", "Assistant creation de matrice.", 0, 0, CAT_CATEGORY_MATRIX },
    {" mksa(x)", 0, "Conversion en unites MKSA", 0, 0, CAT_CATEGORY_PHYS | (CAT_CATEGORY_UNIT << 8)  | XCAS_ONLY},
    {" ufactor(a,b)", 0, "Factorise l'unite b dans a", "100_J,1_kW", 0, CAT_CATEGORY_PHYS | (CAT_CATEGORY_UNIT << 8) | XCAS_ONLY},
    {" usimplify(a)", 0, "Simplifie l'unite dans a", "100_l/10_cm^2", 0, CAT_CATEGORY_PHYS | (CAT_CATEGORY_UNIT << 8) | XCAS_ONLY},
    //{"fonction def Xcas", "fonction f(x) local y;   ffonction:;", "Definition de fonction.", "#fonction f(x) local y; y:=x^2; return y; ffonction:;", 0, CAT_CATEGORY_PROG},
    {"!", "!", "Non logique (prefixe) ou factorielle de n (suffixe).", "#7!", "#!b", CAT_CATEGORY_PROGCMD},
    {"#", "#", "Commentaire Python, en Xcas taper //.", 0, 0, CAT_CATEGORY_PROG},
    {"%", "%", "a % b signifie a modulo b", 0, 0, CAT_CATEGORY_ARIT | (CAT_CATEGORY_PROGCMD << 8)},
    {"&", "&", "Et logique ou +", "#1&2", 0, CAT_CATEGORY_PROGCMD},
    {":=", ":=", "Affectation vers la gauche (inverse de =>).", "#a:=3", 0, CAT_CATEGORY_PROGCMD|(CAT_CATEGORY_SOFUS<<8)|XCAS_ONLY},
    {"<", "<", "Inferieur strict. Raccourci SHIFT F2", 0, 0, CAT_CATEGORY_PROGCMD},
    {"=>", "=>", "Affectation vers la droite ou conversion en (touche ->). Par exemple 5=>a ou x^4-1=>* ou (x+1)^2=>+ ou sin(x)^2=>cos.", "#5=>a", "#15_m=>_cm", CAT_CATEGORY_PROGCMD | (CAT_CATEGORY_PHYS <<8) | (CAT_CATEGORY_UNIT << 16) | XCAS_ONLY},
    {">", ">", "Superieur strict. Raccourci F2.", 0, 0, CAT_CATEGORY_PROGCMD},
    {"\\", "\\", "Caractere \\", 0, 0, CAT_CATEGORY_PROGCMD},
    {"_", "_", "Caractere _. Prefixe d'unites.", 0, 0, CAT_CATEGORY_PROGCMD},
    {"_(km/h)", "_(km/h)", "Vitesse en kilometre/heure", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_(m/s)", "_(m/s)", "Vitesse en metre/seconde", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_(m/s^2)", "_(m/s^2)", "Acceleration en metre par seconde au carre", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_(m^2/s)", "_(m^2/s)", "Viscosite", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_A", 0, "Intensite electrique en Ampere", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_Bq", 0, "Radioactivite: Becquerel", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_C", 0, "Charge electrique en Coulomb", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_Ci", 0, "Radioactivite: Curie", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_F", 0, "Farad", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_F_", 0, "constante de Faraday (charge globale d'une mole de charges élémentaires).", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_G_", 0, "constante de gravitation universelle. Force=_G_*m1*m2/r^2", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_H", 0, "Henry", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_Hz", 0, "Hertz", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_J", 0, "Energie en Joule=kg*m^2/s^2", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_K", 0, "Temperature en Kelvin", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_Kcal", 0, "Energie en kilo-calorier", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_MeV", 0, "Energie en mega-electron-Volt", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_N", 0, "Force en Newton=kg*m/s^2", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_NA_", 0, "Avogadro", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_Ohm", 0, "Resistance electrique en Ohm", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_PSun_", 0, "puissance du Soleil", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_Pa", 0, "Pression en Pascal=kg/m/s^2", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_REarth_", 0, "Rayon de la Terre", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_RSun_", 0, "rayon du Soleil", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_R_", 0, "constante des gaz (de Boltzmann par mole)", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_S", 0, "", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_StdP_", 0, "Pression standard (au niveau de la mer)", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_StdT_", 0, "temperature standard (0 degre Celsius exprimes en Kelvins)", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_Sv", 0, "Radioactivite: Sievert", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_T", 0, "Tesla", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_V", 0, "Tension electrique en Volt", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_Vm_", 0, "Volume molaire", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_W", 0, "Puissance en Watt=kg*m^2/s^3", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_Wb", 0, "Weber", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_alpha_", 0, "constante de structure fine", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_c_", 0, "vitesse de la lumiere", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_cd", 0, "Luminosite en candela", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_cdf", "_cdf", "Suffixe pour obtenir une distribution cumulee. Taper F2 pour la distribution cumulee inverse.", "#_icdf", 0, CAT_CATEGORY_PROBA|XCAS_ONLY},
    {"_d", 0, "Temps: jour", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_deg", 0, "Angle en degres", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_eV", 0, "Energie en electron-Volt", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_epsilon0_", 0, "permittivite du vide", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_ft", 0, "Longueur en pieds", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_g_", 0, "gravite au sol", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_grad", 0, "Angle en grades", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_h", 0, "Heure", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_h_", 0, "constante de Planck", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_ha", 0, "Aire en hectare", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_hbar_", 0, "constante de Planck/(2*pi)", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_inch", 0, "Longueur en pouces", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_kWh", 0, "Energie en kWh", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_k_", 0, "constante de Boltzmann", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_kg", 0, "Masse en kilogramme", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_l", 0, "Volume en litre", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_m", 0, "Longueur en metre", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_mEarth_", 0, "masse de la Terre", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_m^2", 0, "Aire en m^2", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_m^3", 0, "Volume en m^3", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_me_", 0, "masse electron", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_miUS", 0, "Longueur en miles US", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_mn", 0, "Temps: minute", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_mp_", 0, "masse proton", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_mpme_", 0, "ratio de masse proton/electron", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_mu0_", 0, "permeabilite du vide", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_phi_", 0, "quantum flux magnetique", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_plot", "_plot", "Suffixe pour obtenir le graphe d'une regression.", "#X,Y:=[1,2,3,4,5],[0,1,3,4,4];polynomial_regression_plot(X,Y,2);scatterplot(X,Y)", 0, CAT_CATEGORY_STATS| XCAS_ONLY},
    {"_qe_", 0, "charge de l'electron", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_qme_", 0, "_q_/_me_", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_rad", 0, "Angle en radians", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_rem", 0, "Radioactivite: rem", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_s", 0, "Temps: seconde", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_sd_", 0, "Jour sideral", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_syr_", 0, "Annee siderale", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_tr", 0, "Angle en tours", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_yd", 0, "Longueur en yards", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"a and b", " and ", "Et logique", 0, 0, CAT_CATEGORY_PROGCMD},
    {"a or b", " or ", "Ou logique", 0, 0, CAT_CATEGORY_PROGCMD},
    {"abcuv(a,b,c)", 0, "Cherche 2 polynomes u,v tels que a*u+b*v=c","x+1,x^2-2,x", 0, CAT_CATEGORY_POLYNOMIAL| XCAS_ONLY},
    {"abs(x)", 0, "Valeur absolue, module ou norme de x", "-3", "[1,2,3]", CAT_CATEGORY_COMPLEXNUM | (CAT_CATEGORY_REAL<<8)},
    {"add(u,v)", 0, "En Python, additionne des listes ou listes de listes u et v comme des vecteurs ou matrices.","[1,2,3],[0,1,3]", "[[1,2]],[[3,4]]", CAT_CATEGORY_LINALG},
    {"append", 0, "Ajoute un element en fin de liste l","#l.append(x)", 0, CAT_CATEGORY_LIST},
    {"approx(x)", 0, "Valeur approchee de x. Raccourci S-D", "pi", 0, CAT_CATEGORY_REAL| XCAS_ONLY},
    {"aire(objet)", 0, "Aire algebrique", "cercle(0,1)", "triangle(-1,1+i,3)", CAT_CATEGORY_2D  },
    {"arg(z)", 0, "Argument du complexe z.", "1+i", 0, CAT_CATEGORY_COMPLEXNUM | XCAS_ONLY},
    {"asc(string)", 0, "Liste des codes ASCII d'une chaine", "\"Bonjour\"", 0, CAT_CATEGORY_ARIT},
    {"assume(hyp)", 0, "Hypothese sur une variable.", "x>1", "x>-1 and x<1", CAT_CATEGORY_PROGCMD | (CAT_CATEGORY_SOFUS<<8) | XCAS_ONLY},
    {"avance n", "avance ", "La tortue avance de n pas, par defaut n=10", "#avance 40", 0, CAT_CATEGORY_LOGO},
    {"axes", "axes", "Axes visibles ou non axes=1 ou 0", "#axes=0", "#axes=1", CAT_CATEGORY_PROGCMD << 8|XCAS_ONLY},
    {"baisse_crayon ", "baisse_crayon ", "La tortue se deplace en marquant son passage.", 0, 0, CAT_CATEGORY_LOGO},
    {"barplot(list)", 0, "Diagramme en batons d'une serie statistique 1d.", "[3/2,2,1,1/2,3,2,3/2]", 0, CAT_CATEGORY_STATS | (CAT_CATEGORY_PLOT<<8)},
    {"barycentre([pnt,coeff],...)", 0, "Barycentre d'une sequnence de [points,coefficients]. Utiliser isobarycenter si tous les coefficients sont egaux.", "[1,1],[i,1],[2,3]", 0, CAT_CATEGORY_2D | (CAT_CATEGORY_3D << 8) },
    {"binomial(n,p,k)", 0, "binomial(n,p,k) probabilite de k succes avec n essais ou p est la proba de succes d'un essai. binomial_cdf(n,p,k) est la probabilite d'obtenir au plus k succes avec n essais. binomial_icdf(n,p,t) renvoie le plus petit k tel que binomial_cdf(n,p,k)>=t", "10,.5,4", 0, CAT_CATEGORY_PROBA | XCAS_ONLY},
    {"bissectrice(A,B,C)", 0, "Bissectrice de l'angle AB,AC", "1,i,2+i", 0,CAT_CATEGORY_2D},
    {"bitxor", "bitxor", "Ou exclusif", "#bitxor(1,2)", 0, CAT_CATEGORY_PROGCMD | XCAS_ONLY},
    {"black", "black", "Option d'affichage", "#display=black", 0, CAT_CATEGORY_PROGCMD},
    {"blue", "blue", "Option d'affichage", "#display=blue", 0, CAT_CATEGORY_PROGCMD},
    {"caseval", "caseval", "Evalue une chaine de caractere en appelant le CAS.", "caseval(\"limit(sin(x)/x,x=0)\")", "caseval(\"factor(x^10-1)\")", CAT_CATEGORY_ALGEBRA | (CAT_CATEGORY_CALCULUS <<8)},
    {"cache_tortue ", "cache_tortue ", "Cache la tortue apres avoir trace le dessin.", 0, 0, CAT_CATEGORY_LOGO},
    {"camembert(list)", 0, "Diagramme en camembert d'une serie statistique 1d.", "[[\"France\",6],[\"Allemagne\",12],[\"Suisse\",5]]", 0, CAT_CATEGORY_STATS | XCAS_ONLY},
    {"ceil(x)", 0, "Partie entiere superieure", "1.2", 0, CAT_CATEGORY_REAL},
    {"centre(objet)", 0, "Centre d'un cercle ou d'une sphere. Pour une conique a centre, renvoie le centre, un foyer et un point de la conique. Pour une parabole, renvoie le foyer et le sommet.", "cercle(0,1)", "sphere([0,0,0],[1,1,1])", CAT_CATEGORY_2D | (CAT_CATEGORY_3D << 8) },
    {"cercle(centre,rayon)", 0, "Cercle donne par centre et rayon ou par un diametre", "2+i,3", "1-i,1+i", CAT_CATEGORY_PROGCMD | (CAT_CATEGORY_2D << 8) | XCAS_ONLY},
    {"circonscrit(A,B,C)", 0, "Cercle circonscrit", "-1,2+i,3", 0, CAT_CATEGORY_PROGCMD | (CAT_CATEGORY_2D << 8) | XCAS_ONLY},
    {"cfactor(p)", 0, "Factorisation sur C.", "x^4-1", 0, CAT_CATEGORY_ALGEBRA | (CAT_CATEGORY_COMPLEXNUM << 8) | XCAS_ONLY},
    {"char(liste)", 0, "Chaine donnee par une liste de code ASCII", "[97,98,99]", 0, CAT_CATEGORY_ARIT},
    {"charpoly(M,x)", 0, "Polynome caracteristique de la matrice M en la variable x.", "[[1,2],[3,4]],x", 0, CAT_CATEGORY_MATRIX | XCAS_ONLY},
    {"clearscreen()", "clearscreen()", "Efface l'ecran.", 0, 0, CAT_CATEGORY_PROGCMD|XCAS_ONLY},
    {"coeff(p,x,n)", 0, "Coefficient de x^n dans le polynome p.", "(1+x)^6,x,3", 0, CAT_CATEGORY_POLYNOMIAL | XCAS_ONLY},
    {"comb(n,k)", 0, "Renvoie k parmi n.", "10,4", 0, CAT_CATEGORY_PROBA | XCAS_ONLY},
    {"cond(A,[1,2,inf])", 0, "Nombre de condition d'une matrice par rapport a la norme specifiee (par defaut 1)", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX | XCAS_ONLY},
    {"cone(A,v,theta,[h])", 0, "Cone de sommet A, axe v, angle theta, hauteur h optionnelle", "[0,0,0],[0,0,1],pi/6", "[0,0,0],[0,0,1],pi/6,4", CAT_CATEGORY_3D},
    {"conique(expression)", 0, "Conique donnee par une equation polynomiale de degre 2 ou passant par 5 points", "x^2+x*y+y^2=5", "1,i,2+i,3-i,4+2i", CAT_CATEGORY_2D},
    {"coordonnees(object)", 0, "Coordonnees (cartesiennes)", "point(1,2)", "point(1,2,3)", CAT_CATEGORY_2D | (CAT_CATEGORY_3D << 8) },
    {"conj(z)", 0, "Conjugue complexe de z.", "1+i", 0, CAT_CATEGORY_COMPLEXNUM},
    {"correlation(l1,l2)", 0, "Correlation listes l1 et l2", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_STATS | XCAS_ONLY},
    {"covariance(l1,l2)", 0, "Covariance listes l1 et l2", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_STATS | XCAS_ONLY},
    {"cpartfrac(p,x)", 0, "Decomposition en elements simples sur C.", "1/(x^4-1)", 0, CAT_CATEGORY_ALGEBRA | (CAT_CATEGORY_COMPLEXNUM << 8) | XCAS_ONLY},
    {"crayon ", "crayon ", "Couleur de trace de la tortue", "#crayon rouge", 0, CAT_CATEGORY_LOGO},
    {"cross(u,v)", 0, "Produit vectoriel de u et v.","[1,2,3],[0,1,3]", 0, CAT_CATEGORY_LINALG | (CAT_CATEGORY_2D << 8)},
    {"csolve(equation,x)", 0, "Resolution exacte dans C d'une equation en x (ou d'un systeme polynomial).","x^2+x+1=0", 0, CAT_CATEGORY_SOLVE | (CAT_CATEGORY_COMPLEXNUM << 8) | XCAS_ONLY},
    {"cube(A,B,C)", 0, "Cube d'arete AB avec une face dans le plan ABC", "[0,0,0],[1,0,0],[0,1,0]","[0,0,0],[0,2,sqrt(5)/2+3/2],[0,0,1]", CAT_CATEGORY_3D},
    {"curl(u,vars)", 0, "Rotationnel du vecteur u.", "[2*x*y,x*z,y*z],[x,y,z]", 0, CAT_CATEGORY_LINALG | XCAS_ONLY},
    {"curvature([x(t),y(t)],t,t0)", 0, "Courbure de la courbe parametree [x(t),y(t)] en t0", "[t,t^2],t,1", "[t,t^2],t", CAT_CATEGORY_CALCULUS | (CAT_CATEGORY_2D << 8) | XCAS_ONLY},
    {"cyan", "cyan", "Option d'affichage", "#display=cyan", 0, CAT_CATEGORY_PROGCMD},
    {"cylinder(A,v,r,[h])", 0, "Cylindre d'axe A,v de rayon r et de hauteur optionnelle h", "[0,0,0],[0,1,0],2", "[0,0,0],[0,1,0],2,3", CAT_CATEGORY_3D},
    {"debug(f(args))", 0, "Execute la fonction f en mode pas a pas.", 0, 0, CAT_CATEGORY_PROG | XCAS_ONLY},
    {"degree(p,x)", 0, "Degre du polynome p en x.", "x^4-1", 0, CAT_CATEGORY_POLYNOMIAL | XCAS_ONLY},
    {"denom(x)", 0, "Denominateur de l'expression x.", "3/4", 0, CAT_CATEGORY_POLYNOMIAL | XCAS_ONLY},
    {"desolve(equation,t,y)", 0, "Resolution exacte d'equation differentielle ou de systeme differentiel lineaire a coefficients constants.", "[y'+y=exp(x),y(0)=1]", "[y'=[[1,2],[2,1]]*y+[x,x+1],y(0)=[1,2]]", CAT_CATEGORY_SOLVE | (CAT_CATEGORY_CALCULUS << 8) | XCAS_ONLY},
    {"det(A)", 0, "Determinant de la matrice A.", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX | XCAS_ONLY},
    {"diff(f,var,[n])", 0, "Derivee de l'expression f par rapport a var (a l'ordre n, n=1 par defaut), par exemple diff(sin(x),x) ou diff(x^3,x,2). Pour deriver f par rapport a x, utiliser f' (raccourci F3). Pour le gradient de f, var est la liste des variables.", "sin(x),x", "sin(x^2),x,3", CAT_CATEGORY_CALCULUS | XCAS_ONLY},
    {"display", "display", "Option d'affichage", "#display=red", 0, CAT_CATEGORY_PROGCMD | XCAS_ONLY},
    {"disque n", "disque ", "Cercle rempli tangent a la tortue, de rayon n. Utiliser disque n,theta pour remplir un morceau de camembert ou disque n,theta,segment pour remplir un segment de disque", "#disque 30", "#disque(30,90)", CAT_CATEGORY_LOGO},
  {"distance(A,B)", 0, "Distance de 2 objets geometriques", "point(1,2,3),point(4,1,2)", 0, CAT_CATEGORY_3D | (CAT_CATEGORY_2D << 8) },
    {"dot(a,b)", 0, "Produit scalaire de 2 vecteurs. Raccourci: *", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_LINALG | (CAT_CATEGORY_2D << 8)},
    {"dodecahedron(A,B,C)", 0, "Dodecaedre d'arete AB avec une face dans le plan ABC", "[0,0,0],[0,2,sqrt(5)/2+3/2],[0,0,1]", 0, CAT_CATEGORY_3D},
    {"draw_arc(x1,y1,rx,ry,theta1,theta2,c)", 0, "Arc d'ellipse pixelise.", "100,100,60,80,0,pi,magenta", 0, CAT_CATEGORY_PROGCMD},
    {"draw_circle(x1,y1,r,c)", 0, "Cercle pixelise. Option filled pour le remplir.", "100,100,60,cyan+filled", 0, CAT_CATEGORY_PROGCMD},
    {"draw_line(x1,y1,x2,y2,c)", 0, "Droite pixelisee.", "100,50,300,200,blue", 0, CAT_CATEGORY_PROGCMD},
    {"draw_pixel(x,y,color)", 0, "Colorie le pixel x,y. Faire draw_pixel() pour synchroniser l'ecran.", 0, 0, CAT_CATEGORY_PROGCMD},
    {"draw_polygon([[x1,y1],...],c)", 0, "Polygone pixelise.", "[[100,50],[30,20],[60,70]],red+filled", 0, CAT_CATEGORY_PROGCMD},
    {"draw_rectangle(x,y,w,h,c)", 0, "Rectangle pixelise.", "100,50,30,20,red+filled", 0, CAT_CATEGORY_PROGCMD},
    {"draw_string(s,x,y,c)", 0, "Affiche la chaine s en x,y", "\"Bonjour\",80,60", 0, CAT_CATEGORY_PROGCMD},
    {"droite(equation)", 0, "Droite donnee par une equation ou 2 points", "y=2x+1", "1+i,2-i", CAT_CATEGORY_PROGCMD | (CAT_CATEGORY_2D << 8) | (CAT_CATEGORY_2D << 16) | XCAS_ONLY},
    {"ecris ", "ecris ", "Ecrire a la position de la tortue", "#ecris \"coucou\"", 0, CAT_CATEGORY_LOGO},
    {"efface", "efface", "Remise a zero de la tortue", 0, 0, CAT_CATEGORY_LOGO | XCAS_ONLY},
    {"egcd(A,B)", 0, "Cherche des polynomes U,V,D tels que A*U+B*V=D=gcd(A,B)","x^2+3x+1,x^2-5x-1", 0, CAT_CATEGORY_POLYNOMIAL | XCAS_ONLY},
    {"eigenvals(A)", 0, "Valeurs propres de la matrice A.", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX | XCAS_ONLY},
    {"eigenvects(A)", 0, "Vecteurs propres de la matrice A.", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX},
    {"elif (test)", "elif", "Tests en cascade", 0, 0, CAT_CATEGORY_PROG | XCAS_ONLY},
				     //{"end", "end", "Fin de bloc", 0, 0, CAT_CATEGORY_PROG},
    {"ellipse(F1,F2,M)", 0, "Ellipse donnee par les 2 foyers et un point", "-1,1,2", 0, CAT_CATEGORY_2D},
    {"equation(objet)", 0, "Equation cartesienne. Utiliser parameq pour parametrique.", "circle(0,1)", "ellipse(-1,1,3)", CAT_CATEGORY_2D | (CAT_CATEGORY_3D << 8) },
    {"erf(x)", 0, "Fonction erreur en x.", "1.2", 0, CAT_CATEGORY_PROBA},
    {"erfc(x)", 0, "Fonction erreur complementaire en x.", "1.2", 0, CAT_CATEGORY_PROBA},
    {"euler(n)",0,"Indicatrice d'Euler: nombre d'entiers < n premiers avec n","25",0,CAT_CATEGORY_ARIT},
    {"eval(f)", 0, "Evalue f.", 0, 0, CAT_CATEGORY_PROGCMD},
    {"evalc(z)", 0, "Ecrit z=x+i*y.", "1/(1+i*sqrt(3))", 0, CAT_CATEGORY_COMPLEXNUM | XCAS_ONLY},
    {"exact(x)", 0, "Convertit x en rationnel. Raccourci shift S-D", "1.2", 0, CAT_CATEGORY_REAL | XCAS_ONLY},
    {"exp2trig(expr)", 0, "Conversion d'exponentielles complexes en sin/cos", "exp(i*x)", 0, CAT_CATEGORY_TRIG | XCAS_ONLY},
    {"exponential_regression(Xlist,Ylist)", 0, "Regression exponentielle.", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_STATS | XCAS_ONLY},
    {"exponential_regression_plot(Xlist,Ylist)", 0, "Graphe d'une regression exponentielle.", "#X,Y:=[1,2,3,4,5],[1,3,4,6,8];exponential_regression_plot(X,Y);", 0, CAT_CATEGORY_STATS | XCAS_ONLY},
    {"exponentiald(lambda,x)", 0, "Loi exponentielle de parametre lambda. exponentiald_cdf(lambda,x) probabilite que \"loi exponentielle <=x\" par ex. exponentiald_cdf(2,3). exponentiald_icdf(lambda,t) renvoie x tel que \"loi exponentielle <=x\" vaut t, par ex. exponentiald_icdf(2,0.95) ", "5.1,3.4", 0, CAT_CATEGORY_PROBA | XCAS_ONLY},
    {"extend", 0, "Concatene 2 listes. Attention en Xcas, ne pas utiliser + qui effectue l'addition de 2 vecteurs.","#l1.extend(l2)", 0, CAT_CATEGORY_LIST},
    {"factor(p,[x])", 0, "Factorisation du polynome p (utiliser ifactor pour un entier). Raccourci: p=>*", "x^4-1", "x^6+1,sqrt(3)", CAT_CATEGORY_ALGEBRA | (CAT_CATEGORY_POLYNOMIAL << 8) | XCAS_ONLY},
    {"filled", "filled", "Option d'affichage", 0, 0, CAT_CATEGORY_PROGCMD | XCAS_ONLY},
    {"float(x)", 0, "Convertit x en nombre approche (flottant).", "pi", 0, CAT_CATEGORY_REAL},
    {"floor(x)", 0, "Partie entiere de x", "pi", 0, CAT_CATEGORY_REAL},
    {"fonction f(x)", "fonction", "Definition de fonction (Xcas). Par exemple\nfonction f(x)\n local y;\ny:=x*x;\nreturn y;\nffonction", 0, 0, CAT_CATEGORY_PROG | XCAS_ONLY},
    {"frenet([x(t),y(t)],t,t0)", 0, "Courbure, centre de courbure et repere de Frenet de la courbe parametree [x(t),y(t)] en t0", "[t,t^2],t,1", "[t,t^2],t", CAT_CATEGORY_CALCULUS | (CAT_CATEGORY_2D << 8) | XCAS_ONLY},
    {"from arit import *", "from arit import *", "Instruction pour utiliser les fonctions d'arithmetique entiere en Python", "#from arit import *", "#import arit", CAT_CATEGORY_ARIT},
    {"from cas import *", "from cas import *", "Permet d'utiliser le calcul formel depuis Python", "#from cas import *", "#import cas", CAT_CATEGORY_ALGEBRA|(CAT_CATEGORY_CALCULUS<<8)},
    {"from cmath import *", "from cmath import *", "Instruction pour utiliser les fonctions de maths sur les complexes (trigo, exponentielle, log, ...) en Python", "#from cmath import *;i=1j", "#import cmath", CAT_CATEGORY_COMPLEXNUM},
    {"from linalg import *", "from linalg import *", "Instruction pour utiliser les fonctions d'algebre lineaire en Python", "#from linalg import *;i=1j", "#import linalg", CAT_CATEGORY_LINALG | (CAT_CATEGORY_MATRIX<<8) | (CAT_CATEGORY_POLYNOMIAL<<16)},
    {"from numpy import *", "from numpy import *", "Instruction pour utiliser les fonctions sur les matrice en Python", "#from numpy import *;i=1j", "#import numpy", CAT_CATEGORY_LINALG | (CAT_CATEGORY_MATRIX <<8) | (CAT_CATEGORY_COMPLEXNUM << 16)},
    {"from math import *", "from math import *", "Instruction pour utiliser les fonctions de maths (trigo, exponentielle, log, ...) en Python", "#from math import *", "#import math", CAT_CATEGORY_REAL},
    {"from matplotl import *", "from matplotl import *", "Instruction pour utiliser les fonctions de trace en Python", "#from matplotl import *", "#import matplotl", CAT_CATEGORY_PROBA|(CAT_CATEGORY_PLOT <<8)|(CAT_CATEGORY_STATS<<16)},
    {"from random import *", "from random import *", "Instruction pour utiliser les fonctions aleatoires en Python", "#from random import *", "#import random", CAT_CATEGORY_PROBA},
    {"from turtle import *", "from turtle import *", "Instruction pour utiliser la tortue en Python", "#from turtle import *", "#import turtle", CAT_CATEGORY_LOGO},
    {"fsolve(equation,x=a[..b])", 0, "Resolution approchee de equation pour x dans l'intervalle a..b ou en partant de x=a.","cos(x)=x,x=0..1", "cos(x)-x,x=0.0", CAT_CATEGORY_SOLVE | XCAS_ONLY},
    {"gauss(q)", 0, "Reduction de Gauss d'une forme quadratique q", "x^2+x*y+x*z,[x,y,z]", "x^2+4*x*y,[]", CAT_CATEGORY_LINALG | XCAS_ONLY },
    {"gcd(a,b,...)", 0, "Plus grand commun diviseur. En Python ne fonctionne qu'avec des entiers. Voir iegcd ou egcd pour Bezout.", "23,13", "x^2-1,x^3-1", CAT_CATEGORY_ARIT | (CAT_CATEGORY_POLYNOMIAL << 8)},
    {"gl_x", "gl_x", "Reglage graphique X gl_x=xmin..xmax", "#gl_x=0..2", 0, CAT_CATEGORY_PROGCMD << 8 | XCAS_ONLY},
    {"gl_y", "gl_y", "Reglage graphique Y gl_y=ymin..ymax", "#gl_y=-1..1", 0, CAT_CATEGORY_PROGCMD << 8 | XCAS_ONLY},
    {"green", "green", "Option d'affichage", "#display=green", 0, CAT_CATEGORY_PROGCMD},
    {"halftan(expr)", 0, "Exprime cos, sin, tan avec tan(angle/2).","cos(x)", 0, CAT_CATEGORY_TRIG | XCAS_ONLY},
    {"hauteur(A,B,C)", 0, "Hauteur du triangle ABC issue de A", "1,i,2+i", 0,CAT_CATEGORY_2D},
    {"hermite(n)", 0, "n-ieme polynome de Hermite", "10", "10,t", CAT_CATEGORY_POLYNOMIAL | XCAS_ONLY},
    {"hilbert(n)", 0, "Matrice de Hilbert de taille n.", "4", 0, CAT_CATEGORY_MATRIX | XCAS_ONLY},
    {"histogram(list,min,size)", 0, "Histogramme d'une liste de donneees, classes commencant a min de taille size.","ranv(100,uniformd,0,1),0,0.1", 0, CAT_CATEGORY_STATS | (CAT_CATEGORY_PLOT<<8)},
    {"homothetie(centre,rapport,objet)", 0, "Image de l'objet par homothetie de rapport", "0,2,circle(1,1)", 0, CAT_CATEGORY_2D },
    {"hyperbole(F1,F2,M)", 0, "Hyperbole donnee par 2 foyers et un point", "-2-i,2+i,1", 0, CAT_CATEGORY_2D},
    {"iabcuv(a,b,c)", 0, "Cherche 2 entiers u,v tels que a*u+b*v=c","23,13,15", 0, CAT_CATEGORY_ARIT | XCAS_ONLY},
    {"ichinrem([a,m],[b,n])", 0,"Restes chinois entiers de a mod m et b mod n.", "[3,13],[2,7]", 0, CAT_CATEGORY_ARIT | XCAS_ONLY},
    {"icosahedron(A,B,C)", 0, "Icosaedre de centre A, de sommet B où le plan ABC contient le sommet le plus proche (parmi les 5) de B", "[0,0,0],[sqrt(5),0,0],[1,2,0]", 0, CAT_CATEGORY_3D},
    {"idivis(n)", 0, "Liste des diviseurs d'un entier n.", "10", 0, CAT_CATEGORY_ARIT},
    {"idn(n)", 0, "matrice identite n * n", "4", 0, CAT_CATEGORY_MATRIX},
    {"iegcd(a,b)", 0, "Determine les entiers u,v,d tels que a*u+b*v=d=gcd(a,b)","23,13", 0, CAT_CATEGORY_ARIT},
    {"ifactor(n)", 0, "Factorisation d'un entier (pas trop grand!). Raccourci n=>*", "1234", 0, CAT_CATEGORY_ARIT},
    {"ilaplace(f,s,x)", 0, "Transformee inverse de Laplace de f", "s/(s^2+1),s,x", 0, CAT_CATEGORY_CALCULUS | XCAS_ONLY},
    {"im(z)", 0, "Partie imaginaire (z.im en Python)", "1+i", 0, CAT_CATEGORY_COMPLEXNUM},
    {"inscrit(A,B,C)", 0, "Cercle inscrit", "-1,2+i,3", 0, CAT_CATEGORY_PROGCMD | (CAT_CATEGORY_2D << 8) | XCAS_ONLY},
    {"inf", "inf", "Plus l'infini. Utiliser -inf pour moins l'infini ou infinity pour l'infini complexe. Raccourci shift INS.", "-inf", "infinity", CAT_CATEGORY_CALCULUS | XCAS_ONLY},
    {"input()", "input()", "Lire une chaine au clavier", "\"Valeur ?\"", 0, CAT_CATEGORY_PROG},
    {"integrate(f,x,[,a,b])", 0, "Primitive de f par rapport a la variable x, par ex. integrate(x*sin(x),x). Pour calculer une integrale definie, entrer les arguments optionnels a et b, par ex. integrate(x*sin(x),x,0,pi). Raccourci SHIFT F3.", "x*sin(x),x", "cos(x)/(1+x^4),x,0,inf", CAT_CATEGORY_CALCULUS | XCAS_ONLY},
    {"interp(X,Y[,interp])", 0, "Interpolation de Lagrange aux points (xi,yi) avec X la liste des xi et Y des yi. Renvoie la liste des differences divisees si interp est passe en parametre.", "[1,2,3,4,5],[0,1,3,4,4]", "[1,2,3,4,5],[0,1,3,4,4],interp", CAT_CATEGORY_POLYNOMIAL | XCAS_ONLY},
    {"inter(A,B)", 0, "Liste des intersections. Utiliser single_inter si l'intersection est unique.", "line(y=x),circle(0,1)", 0, CAT_CATEGORY_3D | (CAT_CATEGORY_2D << 8) | XCAS_ONLY},
    {"inter_unique(A,B)", 0, "Premiere intersection. Utiliser inter pour une liste d'intersections.", "line(y=x),line(x+y=3)", 0, CAT_CATEGORY_3D | (CAT_CATEGORY_2D << 8) | XCAS_ONLY},
    {"inv(A)", 0, "Inverse de A.", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX|(CAT_CATEGORY_LINALG<<8)},
    {"inverser(v)", "inverser ", "La variable v est remplacee par son inverse", "#v:=3; inverser v", 0, CAT_CATEGORY_SOFUS | XCAS_ONLY},
    {"iquo(a,b)", 0, "Quotient euclidien de deux entiers.", "23,13", 0, CAT_CATEGORY_ARIT | XCAS_ONLY},
    {"irem(a,b)", 0,"Reste euclidien de deux entiers", "23,13", 0, CAT_CATEGORY_ARIT | XCAS_ONLY},
    {"isprime(n)", 0, "Renvoie 1 si n est premier, 0 sinon.", "11", "10", CAT_CATEGORY_ARIT},
  {"is_collinear(A,B,C)", 0, "Renvoie 1 ou 2 si A, B, C sont alignes, 0 sinon.", "1,i,-1", "i,0,-i", CAT_CATEGORY_2D | XCAS_ONLY },
  {"is_concyclic(A,B,C,D)", 0, "Renvoie 1 si A, B, C, D sont cocyliques, 0 sinon.", "1,i,-1,-i", "1,i,0,-i", CAT_CATEGORY_2D | XCAS_ONLY },
  {"is_element(A,G)", 0, "Renvoie 1 si A appartient a G, 0 sinon.", "point(0),circle(0,1)", "point(i),square(0,1)", CAT_CATEGORY_2D | XCAS_ONLY },
  {"is_parallel(D,E)", 0, "Renvoie 1 si D et E sont paralleles, 0 sinon.", "line(y=x),line(y=-x)", "line(y=x),line(y=x+1)", CAT_CATEGORY_2D | XCAS_ONLY },
  {"is_perpendicular(D,E)", 0, "Renvoie 1 si D et E sont perpendiculaires, 0 sinon.", "line(y=x),line(y=-x)", "line(y=x),line(y=x+1)", CAT_CATEGORY_2D | XCAS_ONLY },
    {"jordan(A)", 0, "Forme normale de Jordan de la matrice A, renvoie P et D tels que P^-1*A*P=D", "[[1,2],[3,4]]", "[[1,1,-1,2,-1],[2,0,1,-4,-1],[0,1,1,1,1],[0,1,2,0,1],[0,0,-3,3,-1]]", CAT_CATEGORY_MATRIX | XCAS_ONLY},
    {"laguerre(n,a,x)", 0, "n-ieme polynome de Laguerre (a=0 par defaut).", "10", 0, CAT_CATEGORY_POLYNOMIAL | XCAS_ONLY},
    {"laplace(f,x,s)", 0, "Transformee de Laplace de f","sin(x),x,s", 0, CAT_CATEGORY_CALCULUS | XCAS_ONLY},
    {"lcm(a,b,...)", 0, "Plus petit commun multiple.", "23,13", "x^2-1,x^3-1", CAT_CATEGORY_ARIT | (CAT_CATEGORY_POLYNOMIAL << 8) | XCAS_ONLY},
    {"lcoeff(p,x)", 0, "Coefficient dominant du polynome p.", "x^4-1", 0, CAT_CATEGORY_POLYNOMIAL | XCAS_ONLY},
    {"legendre(n)", 0, "n-ieme polynome de Legendre.", "10", "10,t", CAT_CATEGORY_POLYNOMIAL | XCAS_ONLY},
#ifdef RELEASE
    {"len(l)", 0, "Taille d'une liste.", "[3/2,2,1,1/2,3,2,3/2]", 0, CAT_CATEGORY_LIST},
#endif
    {"leve_crayon ", "leve_crayon ", "La tortue se deplace sans marquer son passage", 0, 0, CAT_CATEGORY_LOGO},
    {"limit(f,x=a)", 0, "Limite de f en x = a. Ajouter 1 ou -1 pour une limite a droite ou a gauche, limit(sin(x)/x,x=0) ou limit(abs(x)/x,x=0,1). Raccourci: SHIFT MIXEDFRAC", "sin(x)/x,x=0", "exp(-1/x),x=0,1", CAT_CATEGORY_CALCULUS | XCAS_ONLY},
    {"droite(A,B)", 0, "Droite donnee par equation ou 2 points", "y=x-1", "[0,0,0],[1,-2,3]", CAT_CATEGORY_2D | XCAS_ONLY},
    {"line_width_", "line_width_", "Prefixe d'epaisseur (2 a 8)", 0, 0, CAT_CATEGORY_PROGCMD | XCAS_ONLY},
    {"linear_regression(Xlist,Ylist)", 0, "Regression lineaire.", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_STATS | XCAS_ONLY},
    {"linear_regression_plot(Xlist,Ylist)", 0, "Graphe d'une regression lineaire.", "#X,Y:=[1,2,3,4,5],[0,1,3,4,4];linear_regression_plot(X,Y);", 0, CAT_CATEGORY_STATS | (CAT_CATEGORY_PLOT<<8)},
    {"linetan(expr,x,x0)", 0, "Tangente au graphe en x=x0.", "sin(x),x,pi/2", 0, CAT_CATEGORY_PLOT | XCAS_ONLY},
    {"linsolve([eq1,eq2,..],[x,y,..])", 0, "Resolution de systeme lineaire. Peut utiliser le resultat de lu pour resolution en O(n^2).","[x+y=1,x-y=2],[x,y]", "#p,l,u:=lu([[1,2],[3,4]]); linsolve(p,l,u,[5,6])", CAT_CATEGORY_SOLVE | (CAT_CATEGORY_LINALG <<8) | (CAT_CATEGORY_MATRIX << 16) | XCAS_ONLY},
    {"logarithmic_regression(Xlist,Ylist)", 0, "Regression logarithmique.", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_STATS | XCAS_ONLY},
    {"logarithmic_regression_plot(Xlist,Ylist)", 0, "Graphe d'une regression logarithmique.", "#X,Y:=[1,2,3,4,5],[0,1,3,4,4];logarithmic_regression_plot(X,Y);", 0, CAT_CATEGORY_STATS | XCAS_ONLY},
    {"lu(A)", 0, "decomposition LU de la matrice A, P*A=L*U, renvoie P permutation, L et U triangulaires inferieure et superieure", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX | XCAS_ONLY},
    {"magenta", "magenta", "Option d'affichage", "#display=magenta", 0, CAT_CATEGORY_PROGCMD},
    {"map(f,l)", 0, "Applique f aux elements de la liste l.","lambda x:x*x,[1,2,3]", 0, CAT_CATEGORY_LIST},
    {"matpow(A,n)", 0, "Renvoie A^n, la matrice A la puissance n", "[[1,2],[3,4]],n","#assume(n>=1);matpow([[0,2],[0,4]],n)", CAT_CATEGORY_MATRIX | XCAS_ONLY},
    {"matrix(l,c,func)", 0, "Matrice de terme general donne.", "2,3,(j,k)->j^k", 0, CAT_CATEGORY_MATRIX},
    {"mean(l)", 0, "Moyenne arithmetique liste l", "[3/2,2,1,1/2,3,2,3/2]", 0, CAT_CATEGORY_STATS | XCAS_ONLY},
    {"median(l)", 0, "Mediane", "[3/2,2,1,1/2,3,2,3/2]", 0, CAT_CATEGORY_STATS | XCAS_ONLY},
    {"mediane(A,B,C)", 0, "Mediane du triangle ABC issue de A", "1,i,2+i", 0,CAT_CATEGORY_2D},
    {"mediatrice(A,B)", 0, "Mediatrice du segment AB", "1,i", 0,CAT_CATEGORY_2D},
    {"milieu(A,B)", 0, "Milieu de AB", "1,i", 0,CAT_CATEGORY_2D | (CAT_CATEGORY_3D << 8)},
    {"montre_tortue ", "montre_tortue ", "Affiche la tortue", 0, 0, CAT_CATEGORY_LOGO},
    {"mul(A,B)", 0, "En Python, multiplie des listes de listes u et v comme des matrices.","[[1,2],[3,4]],[5,6]", "[[1,2],[3,4]].[[5,6],[7,8]]", CAT_CATEGORY_LINALG},
    {"mult_c_conjugate", 0, "Multiplier par le conjugue complexe.", "1+2*i", 0,  (CAT_CATEGORY_COMPLEXNUM << 8) | XCAS_ONLY},
    {"mult_conjugate", 0, "Multiplier par le conjugue (sqrt).", "sqrt(2)-sqrt(3)", 0, CAT_CATEGORY_ALGEBRA | XCAS_ONLY},
    {"normald([mu,sigma],x)", 0, "Loi normale, par defaut mu=0 et sigma=1. normald_cdf([mu,sigma],x) probabilite que \"loi normale <=x\" par ex. normald_cdf(1.96). normald_icdf([mu,sigma],t) renvoie x tel que \"loi normale <=x\" vaut t, par ex. normald_icdf(0.975) ", "1.2", 0, CAT_CATEGORY_PROBA | XCAS_ONLY},
    {"not(x)", 0, "Non logique.", 0, 0, CAT_CATEGORY_PROGCMD},
    {"numer(x)", 0, "Numerateur de x.", "3/4", 0, CAT_CATEGORY_POLYNOMIAL | XCAS_ONLY},
    {"octahedron(A,B,C)", 0, "Octaedre d'arete AB avec une face dans le plan ABC", "[0,0,0],[3,0,0],[0,1,0]", 0, CAT_CATEGORY_3D},
    {"odesolve(f(t,y),[t,y],[t0,y0],t1)", 0, "Solution approchee d'equation differentielle y'=f(t,y) et y(t0)=y0, valeur en t1 (ajouter curve pour les valeurs intermediaires de y)", "sin(t*y),[t,y],[0,1],2", "0..pi,(t,v)->{[-v[1],v[0]]},[0,1]", CAT_CATEGORY_SOLVE | XCAS_ONLY},
    {"osculating_circle([x(t),y(t)],t,t0)", 0, "Cercle osculateur de la courbe parametree [x(t),y(t)] en t0", "[t,t^2],t,1", "[t,t^2],t", CAT_CATEGORY_CALCULUS | (CAT_CATEGORY_2D << 8) | XCAS_ONLY},
    {"parabole(F,A)", 0, "Parabole donnee par foyer et sommet", "-2-i,2+i", 0, CAT_CATEGORY_2D},
    {"parameq(objet)", 0, "Equations parametriques. Utiliser equation pour une equation cartesienne", "circle(0,1)", "ellipse(-1,1,3)", CAT_CATEGORY_2D | (CAT_CATEGORY_3D << 8) },
    {"partfrac(p,x)", 0, "Decomposition en elements simples. Raccourci p=>+", "1/(x^4-1)", 0, CAT_CATEGORY_ALGEBRA | XCAS_ONLY},
    {"pas_de_cote n", "pas_de_cote ", "Saut lateral de la tortue, par defaut n=10", "#pas_de_cote 30", 0, CAT_CATEGORY_LOGO},
    {"plan(equation)", 0, "Plan donne par equation ou 3 points", "z=x+y-1", "[0,0,0],[1,0,0],[0,1,0]", CAT_CATEGORY_3D | XCAS_ONLY},
    {"plot(expr,x)", 0, "Xcas: graphe de fonction, par exemple plot(sin(x)), plot(ln(x),x.0,5), plot(x^2-y^2), plot(x^2-y^2<1), plot(x^2-y^2=1). Python et Xcas: plot(Xlist,Ylist) ligne polygonale", "[1,2,3,4,5,6],[2,3,5,2,1,4]","ln(x),x=0..5,xstep=0.1", CAT_CATEGORY_PLOT },
    {"plotfunc(expr,[x,y])", 0, "Xcas: graphe de fonction 3d", "x^2-y^2,[x,y]","x^2-y^2,[x=-2..2,y=-2..2],nstep=700", CAT_CATEGORY_PLOT | (CAT_CATEGORY_3D << 8) | XCAS_ONLY },
    {"plotarea(expr,x=a..b,[n,meth])", 0, "Aire sous la courbe selon une methode d'integration.", "1/x,x=1..5,4,rectangle_gauche", 0, CAT_CATEGORY_PLOT | XCAS_ONLY},
    {"plotcontour(expr,[x=xm..xM,y=ym..yM],niveaux)", 0, "Lignes de niveau de expr.", "x^2+2y^2, [x=-2..2,y=-2..2],[1,2]", 0, CAT_CATEGORY_PLOT | XCAS_ONLY},
    {"plotdensity(expr,[x=xm..xM,y=ym..yM])", 0, "Representation en niveaux de couleurs d'une expression de 2 variables.", "x^2-y^2,[x=-3..3,y=-2..2]", 0, CAT_CATEGORY_PLOT | XCAS_ONLY},
    {"plotfield(f(t,y), [t=tmin..tmax,y=ymin..ymax])", 0, "Champ des tangentes de y'=f(t,y), optionnellement graphe avec plotode=[t0,y0]", "sin(t*y), [t=-3..3,y=-3..3],plotode=[0,1]", "5*[-y,x], [x=-1..1,y=-1..1]", CAT_CATEGORY_PLOT | XCAS_ONLY},
    {"plotlist(list)", 0, "Graphe d'une liste", "[3/2,2,1,1/2,3,2,3/2]", "[1,13],[2,10],[3,15],[4,16]", CAT_CATEGORY_PLOT | XCAS_ONLY},
    {"plotode(f(t,y), [t=tmin..tmax,y],[t0,y0])", 0, "Graphe de solution d'equation differentielle y'=f(t,y), y(t0)=y0.", "sin(t*y),[t=-3..3,y],[0,1]", 0, CAT_CATEGORY_PLOT | XCAS_ONLY},
    {"plotparam([x,y],t)", 0, "Graphe en parametriques. Par exemple plotparam([sin(3t),cos(2t)],t,0,pi) ou plotparam(exp(i*t),t,0,pi)", "[sin(3t),cos(2t)],t,0,pi", "[t^2,t^3],t=-1..1,tstep=0.1", CAT_CATEGORY_PLOT | (CAT_CATEGORY_3D << 8) | XCAS_ONLY},
    {"plotpolar(r,theta)", 0, "Graphe en polaire.","cos(3*x),x,0,pi", "1/(1+cos(x)),x=0..pi,xstep=0.05", CAT_CATEGORY_PLOT | XCAS_ONLY},
    {"plotseq(f(x),x=[u0,m,M],n)", 0, "Trace f(x) sur [m,M] et n termes de la suite recurrente u_{n+1}=f(u_n) de 1er terme u0.","sqrt(2+x),x=[6,0,7],5", 0, CAT_CATEGORY_PLOT | XCAS_ONLY},
    {"plus_point", "plus_point", "Option d'affichage", "#display=blue+plus_point", 0, CAT_CATEGORY_PROGCMD  | XCAS_ONLY},
    {"point(x,y[,z])", 0, "Point", "1,2", "1,2,3", CAT_CATEGORY_PLOT | (CAT_CATEGORY_2D << 8) |  (CAT_CATEGORY_3D << 16) | XCAS_ONLY},
    {"polygone(list)", 0, "Polygone ferme donne par la liste de ses sommets.", "1-i,2+i,3,3-2i", 0, CAT_CATEGORY_PROGCMD | (CAT_CATEGORY_2D << 8) | XCAS_ONLY},
    {"polygonscatterplot(Xlist,Ylist)", 0, "Nuage de points relies.", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_STATS | XCAS_ONLY},
    {"polyhedron(A,B,C,D,...)", 0, "Polyedre convexe dont les sommets sont parmi A,B,C,D,...", "[0,0,0],[0,5,0],[0,0,5],[1,2,6]", 0, CAT_CATEGORY_3D},
    {"polynomial_regression(Xlist,Ylist,n)", 0, "Regression polynomiale de degre <= n.", "[1,2,3,4,5],[0,1,3,4,4],2", 0, CAT_CATEGORY_STATS | XCAS_ONLY},
    {"polynomial_regression_plot(Xlist,Ylist,n)", 0, "Graphe d'une regression polynomiale de degre <= n.", "#X,Y:=[1,2,3,4,5],[0,1,3,4,4];polynomial_regression_plot(X,Y,2);scatterplot(X,Y);", 0, CAT_CATEGORY_STATS | XCAS_ONLY},
    {"pour (boucle Xcas)", "pour  de  to  faire  fpour;", "Boucle definie.","#pour j de 1 to 10 faire print(j,j^2); fpour;", 0, CAT_CATEGORY_PROG | XCAS_ONLY},
    {"power_regression(Xlist,Ylist,n)", 0, "Regression puissance.", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_STATS | XCAS_ONLY},
    {"power_regression_plot(Xlist,Ylist,n)", 0, "Graphe d'une regression puissance.", "#X,Y:=[1,2,3,4,5],[1,1,3,4,4];power_regression_plot(X,Y);", 0, CAT_CATEGORY_STATS | XCAS_ONLY},
    {"pow(a,n,p)", 0, "Renvoie a^n mod p","123,456,789", 0, CAT_CATEGORY_ARIT},
    {"powmod(a,n,p[,P,x])", 0, "Renvoie a^n mod p, ou a^n mod un entier p et un polynome P en x.","123,456,789", "x+1,452,19,x^4+x+1,x", CAT_CATEGORY_ARIT | XCAS_ONLY},
    {"print(expr)", 0, "Afficher dans la console", 0, 0, CAT_CATEGORY_PROG},
    {"projection(obj1,obj2)", 0, "Projection sur obj1 de obj2", "line(y=x),point(2,3)", 0, CAT_CATEGORY_2D },
    {"pcoeff(p)", 0, "Polynome unitaire dont on donne la liste des racines (fonction reciproque de proot)", "[1,2,3]", 0, CAT_CATEGORY_POLYNOMIAL},
    {"peval(p,x)", 0, "Valeur d'un polynome en un point", "[1,2,3],4", 0, CAT_CATEGORY_POLYNOMIAL},
    {"proot(p)", 0, "Racines reelles et complexes approchees d'un polynome. Exemple proot([1,2.1,3,4.2]) ou proot(x^3+2.1*x^2+3x+4.2)", "[1.,2.1,3,4.2]","x^3+2.1*x^2+3x+4.2", CAT_CATEGORY_POLYNOMIAL|(CAT_CATEGORY_SOLVE<<8)},
    {"purge(x)", 0, "Purge le contenu de la variable x. Raccourci SHIFT-FORMAT", 0, 0, CAT_CATEGORY_PROGCMD | (CAT_CATEGORY_SOFUS<<8) | XCAS_ONLY},
    {"pyramid(A,B,C)", 0, "Tetraedre d'arete AB avec une face dans le plan ABC", "[0,0,0],[3,0,0],[0,1,0]", "[0,0,0],[3,0,0],[0,3,0],[0,0,4]", CAT_CATEGORY_3D},
    {"python(f)", 0, "Affiche la fonction f en syntaxe Python.", 0, 0, CAT_CATEGORY_PROGCMD | XCAS_ONLY},
    {"python_compat(0|1|2|4)", 0, "python_compat(0) syntaxe Xcas, python_compat(1) syntaxe Python avec ^ interprete comme puissance, python_compat(2) ^ interprete comme ou exclusif bit a bit", "0", "1", CAT_CATEGORY_PROG | XCAS_ONLY},
    {"qr(A)", 0, "Factorisation A=Q*R avec Q orthogonale et R triangulaire superieure", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX | XCAS_ONLY},
    {"quadric(equation)", 0, "Quadrique donnee par une equation (ou 9 points)", "x^2-y^2+z^2", "x^2+x*y+y^2+z^2-3", CAT_CATEGORY_3D},
    {"quartile1(l)", 0, "1er quartile", "[3/2,2,1,1/2,3,2,3/2]", 0, CAT_CATEGORY_STATS | XCAS_ONLY},
    {"quartile3(l)", 0, "3eme quartile", "[3/2,2,1,1/2,3,2,3/2]", 0, CAT_CATEGORY_STATS | XCAS_ONLY},
    {"quo(p,q,x)", 0, "Quotient de division euclidienne polynomiale en x.", 0, 0, CAT_CATEGORY_POLYNOMIAL | XCAS_ONLY},
    {"quote(x)", 0, "Renvoie l'expression x non evaluee.", 0, 0, CAT_CATEGORY_ALGEBRA | XCAS_ONLY},
    {"rayon(objet)", 0, "Rayon d'un cercle ou d'une sphere", "circle(0,1)", "sphere([0,0,0],[1,1,1])", CAT_CATEGORY_2D | (CAT_CATEGORY_3D << 8) },
    {"rand()", "rand()", "Reel aleatoire entre 0 et 1", 0, 0, CAT_CATEGORY_PROBA},
    {"randint(a,b)", 0, "Entier aleatoire entre a et b. En Xcas, avec un seul argument n, entier entre 1 et n.", "5,20", "6", CAT_CATEGORY_PROBA},
    {"ranm(n,m,[loi,parametres])", 0, "Matrice aleatoire a coefficients entiers ou selon une loi de probabilites (ranv pour un vecteur). Exemples ranm(2,3), ranm(3,2,binomial,20,.3), ranm(4,2,normald,0,1)", "3,3","4,2,normald,0,1",  CAT_CATEGORY_MATRIX},
    {"ranv(n,[loi,parametres])", 0, "Vecteur aleatoire", "4,normald,0,1", "10,30", CAT_CATEGORY_LINALG},
    {"ratnormal(x)", 0, "Ecrit sous forme d'une fraction irreductible.", "(x+1)/(x^2-1)^2", 0, CAT_CATEGORY_ALGEBRA | XCAS_ONLY},
    {"re(z)", 0, "Partie reelle (z.re en Python)", "1+i", 0, CAT_CATEGORY_COMPLEXNUM},
    {"read(\"filename\")", "read(\"", "Lire un fichier. Voir aussi write", 0, 0, CAT_CATEGORY_PROGCMD | XCAS_ONLY},
    {"rectangle_plein a,b", "rectangle_plein ", "Rectangle direct rempli depuis la tortue de cotes a et b (si b est omis, la tortue remplit un carre)", "#rectangle_plein 30", "#rectangle_plein(20,40)", CAT_CATEGORY_LOGO | XCAS_ONLY},
    {"recule n", "recule ", "La tortue recule de n pas, par defaut n=10", "#recule 30", 0, CAT_CATEGORY_LOGO},
    {"red", "red", "Option d'affichage", "#display=red", 0, CAT_CATEGORY_PROGCMD},
    {"reflection(obj1,obj2)", 0, "Symetrique de obj2", "line(y=x),cercle(1,1)", 0, CAT_CATEGORY_2D },
    {"rem(p,q,x)", 0, "Reste de division euclidienne polynomiale en x", 0, 0, CAT_CATEGORY_POLYNOMIAL | XCAS_ONLY},
    {"repete(n,...)", "repete( ", "Repete plusieurs fois les instructions", "#repete(4,avance,tourne_gauche)", 0, CAT_CATEGORY_LOGO | XCAS_ONLY},
#ifdef RELEASE
    {"residue(f(z),z,z0)", 0, "Residu de l'expression en z0.", "1/(x^2+1),x,i", 0, CAT_CATEGORY_COMPLEXNUM | XCAS_ONLY},
#endif
    {"resultant(p,q,x)", 0, "Resultant en x des polynomes p et q.", "#P:=x^3+p*x+q;resultant(P,P',x);", 0, CAT_CATEGORY_POLYNOMIAL | XCAS_ONLY},
    {"revert(p[,x])", 0, "Developpement de Taylor reciproque, p doit etre nul en 0","x+x^2+x^4", 0, CAT_CATEGORY_CALCULUS | XCAS_ONLY},
    {"rgb(r,g,b)", 0, "couleur definie par niveau de rouge, vert, bleu entre 0 et 255", "255,0,255", 0, CAT_CATEGORY_PROGCMD},
    {"rhombus_point", "rhombus_point", "Option d'affichage", "#display=magenta+rhombus_point", 0, CAT_CATEGORY_PROGCMD  | XCAS_ONLY},
    {"rond n", "rond ", "Cercle tangent a la tortue de rayon n. Utiliser rond n,theta pour un arc de cercle.", "#rond 30", "#rond(30,90)", CAT_CATEGORY_LOGO},
    {"rotation(centre,angle,objet)", 0, "Image de l'objet par la rotation de centre et angle donnes en argyment", "2-i,pi/2,circle(0,1)", "sphere([0,0,0],[1,1,1])", CAT_CATEGORY_2D | (CAT_CATEGORY_3D << 8) },
    {"rref(M)","rref","Reduction d'une matrice par le pivot de Gauss.","[[1,2,3],[4,5,6]]",0,CAT_CATEGORY_MATRIX|(CAT_CATEGORY_LINALG<<8)},
    {"rsolve(equation,u(n),[init])", 0, "Expression d'une suite donnee par une recurrence.","u(n+1)=2*u(n)+3,u(n),u(0)=1", "([u(n+1)=3*v(n)+u(n),v(n+1)=v(n)+u(n)],[u(n),v(n)],[u(0)=1,v(0)=2]", CAT_CATEGORY_SOLVE | XCAS_ONLY},
    {"saute n", "saute ", "La tortue fait un saut de n pas, par defaut n=10", "#saute 30", 0, CAT_CATEGORY_LOGO},
    {"scatterplot(Xlist,Ylist)", 0, "Nuage de points (scatter en Python)", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_STATS| (CAT_CATEGORY_PLOT<<8)},
    {"segment(A,B)", 0, "Segment", "1,2+i", "[1,2,1],[-1,3,2]", CAT_CATEGORY_PROGCMD | (CAT_CATEGORY_2D << 8) | XCAS_ONLY},
    {"seq(expr,var,a,b[,pas])", 0, "Liste de terme general donne.","j^2,j,1,10", "j^2,j,1,10,2", CAT_CATEGORY_LIST | XCAS_ONLY},
    {"si (test Xcas)", "si  alors  sinon  fsi;", "Test.", "#f(x):=si x>0 alors x; sinon -x; fsi;", 0, CAT_CATEGORY_PROG | XCAS_ONLY},
    {"sign(x)", 0, "Renvoie -1 si x est negatif, 0 si x est nul et 1 si x est positif.", 0, 0, CAT_CATEGORY_REAL | XCAS_ONLY},
    {"similitude(centre,rapport,angle,objet)", 0, "Image de l'objet par similitude", "0,2,pi/2,circle(1,1)", 0, CAT_CATEGORY_2D },
    {"simplify(expr)", 0, "Renvoie en general expr sous forme simplifiee. Raccourci expr=>/", "sin(3x)/sin(x)", "ln(4)-ln(2)", CAT_CATEGORY_ALGEBRA | XCAS_ONLY},
    {"sin_regression(Xlist,Ylist)", 0, "Regression trigonometrique.", "[1,2,3,4,5,6,7,8,9,10,11,12,13,14],[0.1,0.5,0.8,1,0.7,0.5,0.05,-.5,-.75,-1,-.7,-.4,0.1,.5]", 0, CAT_CATEGORY_STATS | XCAS_ONLY},
    {"sin_regression_plot(Xlist,Ylist)", 0, "Graphe d'une regression trigonometrique.", "#X,Y:=[1,2,3,4,5,6,7,8,9,10,11,12,13,14],[0.1,0.5,0.8,1,0.7,0.5,0.05,-.5,-.75,-1,-.7,-.4,0.1,.5];sin_regression_plot(X,Y);", 0, CAT_CATEGORY_STATS  | XCAS_ONLY},
    {"solve()", 0, "Xcas: solve(equation,x) resolution exacte d'une equation en x (ou d'un systeme polynomial). Utiliser csolve pour les solutions complexes, linsolve pour un systeme lineaire. Python et Xcas: solve(A,b) resolution d'un systeme de Cramer A*x=b", "x^2-x-1=0,x", "[x^2-y^2=0,x^2-z^2=0],[x,y,z]", CAT_CATEGORY_SOLVE},
    {"sommets(objet)", 0, "Liste des sommets d'un polygone ou polyedre", "triangle(1,i,2)", "cube([0,0,0],[1,0,0],[0,1,0])", CAT_CATEGORY_2D | (CAT_CATEGORY_3D << 8) },
    {"sorted(l)", 0, "Trie une liste.","[3/2,2,1,1/2,3,2,3/2]", "[[1,2],[2,3],[4,3]],(x,y)->when(x[1]==y[1],x[0]>y[0],x[1]>y[1]", CAT_CATEGORY_LIST},
    {"sphere(A,r)", 0, "Sphere de centre A et rayon r ou de diametre AB", "[0,0,0],1", "[0,0,0],[1,1,1]", CAT_CATEGORY_3D},
    {"square_point", "square_point", "Option d'affichage", "#display=cyan+square_point", 0, CAT_CATEGORY_PROGCMD | XCAS_ONLY },
    {"star_point", "star_point", "Option d'affichage", "#display=magenta+star_point", 0, CAT_CATEGORY_PROGCMD  | XCAS_ONLY},
    {"stddev(l)", 0, "Ecart-type d'une liste l", "[3/2,2,1,1/2,3,2,3/2]", 0, CAT_CATEGORY_STATS | XCAS_ONLY},
    {"sub(u,v)", 0, "En Python, soustrait des listes ou listes de listes u et v comme des vecteurs ou matrices.","[1,2,3],[0,1,3]", "[[1,2]],[[3,4]]", CAT_CATEGORY_LINALG},
    {"subst(a,b=c)", 0, "Remplace b par c dans a. Raccourci a(b=c). Pour faire plusieurs remplacements, saisir subst(expr,[b1,b2...],[c1,c2...])", "x^2,x=3", "x+y^2,[x,y],[1,2]", CAT_CATEGORY_ALGEBRA | XCAS_ONLY},
    {"sum(f,k,m,M)", 0, "Somme de l'expression f dependant de k pour k variant de m a M. Exemple sum(k^2,k,1,n)=>*. Raccourci ALPHA F3", "k,k,1,n", "k^2,k", CAT_CATEGORY_CALCULUS | XCAS_ONLY},
    {"svd(A)", 0, "Singular Value Decomposition, renvoie U orthogonale, S vecteur des valeurs singulières, Q orthogonale tels que A=U*diag(S)*tran(Q).", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX | XCAS_ONLY},
    {"tabvar(f,[x=a..b])", 0, "Tableau de variations de l'expression f, avec arguments optionnels la variable x dans l'intervalle a..b.", "sqrt(x^2+x+1)", "[cos(2t),sin(3t)],t", CAT_CATEGORY_CALCULUS | XCAS_ONLY},
    {"tantque (boucle Xcas)", "tantque  faire  ftantque;", "Boucle indefinie.", "#j:=13; tantque j!=1 faire j:=ifte(even(j),j/2,3j+1); print(j); ftantque;", 0, CAT_CATEGORY_PROG | XCAS_ONLY},
    {"taylor(f,x=a,n,[polynom])", 0, "Developpement de Taylor de l'expression f en x=a a l'ordre n, ajouter le parametre polynom pour enlever le terme de reste.","sin(x),x=0,5", "sin(x),x=0,5,polynom", CAT_CATEGORY_CALCULUS | XCAS_ONLY},
    {"tchebyshev1(n)", 0, "Polynome de Tchebyshev de 1ere espece: cos(n*x)=T_n(cos(x))", "10", 0, CAT_CATEGORY_POLYNOMIAL | XCAS_ONLY},
    {"tchebyshev2(n)", 0, "Polynome de Tchebyshev de 2eme espece: sin((n+1)*x)=sin(x)*U_n(cos(x))", "10", 0, CAT_CATEGORY_POLYNOMIAL | XCAS_ONLY},
    {"tcollect(expr)", 0, "Linearisation trigonometrique et regroupement.","sin(x)+cos(x)", 0, CAT_CATEGORY_TRIG | XCAS_ONLY},
    {"texpand(expr)", 0, "Developpe les fonctions trigonometriques, exp et ln.","sin(3x)", "ln(x*y)", CAT_CATEGORY_TRIG | XCAS_ONLY},
    {"time(cmd)", 0, "Temps pour effectuer une commande ou mise a l'heure de horloge","int(1/(x^4+1),x)","8,0", CAT_CATEGORY_PROG},
    {"tlin(expr)", 0, "Linearisation trigonometrique de l'expression.","sin(x)^3", 0, CAT_CATEGORY_TRIG | XCAS_ONLY},
    {"tourne_droite n", "tourne_droite ", "La tortue tourne de n degres, par defaut n=90", "#tourne_droite 45", 0, CAT_CATEGORY_LOGO},
    {"tourne_gauche n", "tourne_gauche ", "La tortue tourne de n degres, par defaut n=90", "#tourne_gauche 45", 0, CAT_CATEGORY_LOGO},
    {"trace(A)", 0, "Trace de la matrice A.", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX},
    {"transpose(A)", 0, "Transposee de la matrice A. Pour la transconjuguee utiliser trn(A) ou A^*.", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX},
    {"translation(vect,obj)", 0, "Translation par vect de obj", "[1,2],cercle(0,1)", 0, CAT_CATEGORY_2D },
    {"triangle(A,B,C)", 0, "Triangle donne par 3 sommets", "1+i,1-i,-1", "A,B,C", CAT_CATEGORY_2D},
    {"triangle_point", "triangle_point", "Option d'affichage", "#display=yellow+triangle_point", 0, CAT_CATEGORY_PROGCMD | XCAS_ONLY},
    {"trig2exp(expr)", 0, "Convertit les fonctions trigonometriques en exponentielles.","cos(x)^3", 0, CAT_CATEGORY_TRIG | XCAS_ONLY},
    {"trigcos(expr)", 0, "Exprime sin^2 et tan^2 avec cos^2.","sin(x)^4", 0, CAT_CATEGORY_TRIG | XCAS_ONLY},
    {"trigsin(expr)", 0, "Exprime cos^2 et tan^2 avec sin^2.","cos(x)^4", 0, CAT_CATEGORY_TRIG | XCAS_ONLY},
    {"trigtan(expr)", 0, "Exprime cos^2 et sin^2 avec tan^2.","cos(x)^4", 0, CAT_CATEGORY_TRIG | XCAS_ONLY},
    {"uniformd(a,b,x)", 0, "loi uniforme sur [a,b] de densite 1/(b-a)", 0, 0, CAT_CATEGORY_PROBA | XCAS_ONLY},
    {"v augmente_de n", " augmente_de ", "La variable v augmente de n, ou de n %", "#v:=3; v augmente_de 1", 0, CAT_CATEGORY_SOFUS | XCAS_ONLY},
    {"v diminue_de n", " diminue_de ", "La variable v diminue de n, ou de n %", "#v:=3; v diminue_de 1", 0, CAT_CATEGORY_SOFUS | XCAS_ONLY},
    {"v est_divise_par n", " est_divise_par ", "La variable v est divisee par n", "#v:=3; v est_divise_par 2", 0, CAT_CATEGORY_SOFUS | XCAS_ONLY},
    {"v est_eleve_puissance n", " est_eleve_puissance ", "La variable v est eleveee a la puissance n", "#v:=3; v est_eleve_puissance 2", 0, CAT_CATEGORY_SOFUS | XCAS_ONLY},
    {"v est_multiplie_par n", " est_multiplie_par ", "La variable v est multipliee par n", "#v:=3; v est_multiplie_par 2", 0, CAT_CATEGORY_SOFUS | XCAS_ONLY},
  {"vector(A,B)", 0, "vecteur AB", 0, 0, CAT_CATEGORY_2D | (CAT_CATEGORY_3D << 8)},
  {"volume(P)", 0, "volume d'un polyedre ou d'une sphere", 0, 0, (CAT_CATEGORY_3D )},
				     //{"version", "version()", "Khicas 1.5.0, (c) B. Parisse et al. www-fourier.ujf-grenoble.fr/~parisse. License GPL version 2. Interface adaptee d'Eigenmath pour Casio, G. Maia, http://gbl08ma.com", 0, 0, CAT_CATEGORY_PROGCMD},
    {"write(\"filename\",var)", "write(\"", "Sauvegarde une ou plusieurs variables dans un fichier. Par exemple f(x):=x^2; write(\"func_f\",f).",  0, 0, CAT_CATEGORY_PROGCMD | XCAS_ONLY},
    {"yellow", "yellow", "Option d'affichage", "#display=yellow", 0, CAT_CATEGORY_PROGCMD},
    {"|", "|", "Ou logique", "#1|2", 0, CAT_CATEGORY_PROGCMD},
    {"~", "~", "Complement", "#~7", 0, CAT_CATEGORY_PROGCMD},
  };

const catalogFunc completeCaten[] = { // list of all functions (including some not in any category)
  {" loop for", "for ", "Defined loop.", "#\nfor ", 0, CAT_CATEGORY_PROG},
  {" loop in list", "for in", "Loop on all elements of a list.", "#\nfor in", 0, CAT_CATEGORY_PROG},
  {" loop while", "while ", "Undefined loop.", "#\nwhile ", 0, CAT_CATEGORY_PROG},
  {" test if", "if ", "Test", "#\nif ", 0, CAT_CATEGORY_PROG},
  {" test else", "else ", "Test false case", 0, 0, CAT_CATEGORY_PROG},
  {" function def", "f(x):=", "Definition of function.", "#\nf(x):=", 0, CAT_CATEGORY_PROG},
  {" local j,k;", "local ", "Local variables declaration (Xcas)", 0, 0, CAT_CATEGORY_PROG},
  {" range(a,b)", 0, "In range [a,b) (a included, b excluded)", "# in range(1,10)", 0, CAT_CATEGORY_PROG},
  {" return res", "return ", "Leaves current function and returns res.", 0, 0, CAT_CATEGORY_PROG},
  {" edit list ", "list ", "List creation wizzard.", 0, 0, CAT_CATEGORY_LIST},
  {" edit matrix ", "matrix ", "Matrix creation wizzard.", 0, 0, CAT_CATEGORY_MATRIX},
    {" mksa(x)", 0, "Conversion to MKSA units", 0, 0, CAT_CATEGORY_PHYS | (CAT_CATEGORY_UNIT << 8) | XCAS_ONLY},
    {" ufactor(a,b)", 0, "Factorize unit b in a", "100_J,1_kW", 0, CAT_CATEGORY_PHYS | (CAT_CATEGORY_UNIT << 8) | XCAS_ONLY},
    {" usimplify(a)", 0, "Simplify unit", "100_l/10_cm^2", 0, CAT_CATEGORY_PHYS | (CAT_CATEGORY_UNIT << 8) | XCAS_ONLY},
  {"!", "!", "Logical not (prefix) or factorial of n (suffix).", "#7!", "~!b", CAT_CATEGORY_PROGCMD},
  {"#", "#", "Python comment, for Xcas comment type //. Shortcut ALPHA F2", 0, 0, CAT_CATEGORY_PROG},
  {"%", "%", "a % b means a modulo b", 0, 0, CAT_CATEGORY_ARIT | (CAT_CATEGORY_PROGCMD << 8)},
  {"&", "&", "Logical and or +", "#1&2", 0, CAT_CATEGORY_PROGCMD},
  {":=", ":=", "Set variable value. Shortcut SHIFT F1", "#a:=3", 0, CAT_CATEGORY_PROGCMD|(CAT_CATEGORY_SOFUS<<8)|XCAS_ONLY},
  {"<", "<", "Shortcut SHIFT F2", 0, 0, CAT_CATEGORY_PROGCMD},
  {"=>", "=>", "Store value in variable or conversion (touche ->). For example 5=>a or x^4-1=>* or (x+1)^2=>+ or sin(x)^2=>cos.", "#5=>a", "#15_ft=>_cm", CAT_CATEGORY_PROGCMD | (CAT_CATEGORY_PHYS <<8) | (CAT_CATEGORY_UNIT << 16) | XCAS_ONLY},
  {">", ">", "Shortcut F2.", 0, 0, CAT_CATEGORY_PROGCMD},
  {"\\", "\\", "\\ char", 0, 0, CAT_CATEGORY_PROGCMD},
  {"_", "_", "_ char, shortcut (-).", 0, 0, CAT_CATEGORY_PROGCMD},
    {"_(km/h)", "_(km/h)", "Speed kilometer per hour", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_(m/s)", "_(m/s)", "Speed meter/second", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_(m/s^2)", "_(m/s^2)", "Acceleration", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_(m^2/s)", "_(m^2/s)", "Viscosity", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_A", 0, "Ampere", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_Bq", 0, "Becquerel", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_C", 0, "Coulomb", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_Ci", 0, "Curie", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_F", 0, "Farad", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_F_", 0, "Faraday constant", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_G_", 0, "Gravitation force=_G_*m1*m2/r^2", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_H", 0, "Henry", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_Hz", 0, "Hertz", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_J", 0, "Joule=kg*m^2/s^2", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_K", 0, "Temperature in Kelvin", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_Kcal", 0, "Energy kilo-calorie", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_MeV", 0, "Energy mega-electron-Volt", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_N", 0, "Force Newton=kg*m/s^2", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_NA_", 0, "Avogadro constant", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_Ohm", 0, "Ohm", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_PSun_", 0, "Sun power", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_Pa", 0, "Pressure in Pascal=kg/m/s^2", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_REarth_", 0, "Earth radius", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_RSun_", 0, "Sun radius", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_R_", 0, "Boltzmann constant (per mol)", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_S", 0, "", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_StdP_", 0, "Standard pressure", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_StdT_", 0, "Standard temperature (0 degre Celsius in Kelvins)", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_Sv", 0, "Sievert", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_T", 0, "Tesla", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_V", 0, "Volt", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_Vm_", 0, "Volume molaire", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_W", 0, "Watt=kg*m^2/s^3", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_Wb", 0, "Weber", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_alpha_", 0, "fine structure constant", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_c_", 0, "speed of light", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_cd", 0, "candela", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
  {"_cdf", "_cdf", "Suffix to get a cumulative distribution function. Type F2 for inverse cumulative distribution function _icdf suffix.", "#_icdf", 0, CAT_CATEGORY_PROBA|XCAS_ONLY},
    {"_d", 0, "day", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_deg", 0, "degree", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_eV", 0, "electron-Volt", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_epsilon0_", 0, "vacuum permittivity", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_ft", 0, "feet", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_g_", 0, "Earth gravity (ground)", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_grad", 0, "grades (angle unit(", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_h", 0, "Hour", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_h_", 0, "Planck constant", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_ha", 0, "hectare", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_hbar_", 0, "Planck constant/(2*pi)", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_inch", 0, "inches", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_kWh", 0, "kWh", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_k_", 0, "Boltzmann constant", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_kg", 0, "kilogram", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_l", 0, "liter", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_m", 0, "meter", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_mEarth_", 0, "Earth mass", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_m^2", 0, "Area in m^2", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_m^3", 0, "Volume in m^3", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_me_", 0, "electron mass", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_miUS", 0, "US miles", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_mn", 0, "minute", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_mp_", 0, "proton mass", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_mpme_", 0, "proton/electron mass-ratio", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_mu0_", 0, "", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_phi_", 0, "magnetic flux quantum", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_plot", "_plot", "Suffix for a regression graph.", "#X,Y:=[1,2,3,4,5],[0,1,3,4,4];polynomial_regression_plot(X,Y,2);scatterplot(X,Y)", 0, CAT_CATEGORY_STATS},
    {"_qe_", 0, "electron charge", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_qme_", 0, "_q_/_me_", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_rad", 0, "radians", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_rem", 0, "rem", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_s", 0, "second", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_sd_", 0, "Sideral day", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_syr_", 0, "Siderale year", 0, 0, CAT_CATEGORY_PHYS | XCAS_ONLY},
    {"_tr", 0, "tour (angle unit)", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
    {"_yd", 0, "yards", 0, 0, CAT_CATEGORY_UNIT | XCAS_ONLY},
  {"a and b", " and ", "Logical and", 0, 0, CAT_CATEGORY_PROGCMD},
  {"a or b", " or ", "Logical or", 0, 0, CAT_CATEGORY_PROGCMD},
  {"abcuv(a,b,c)", 0, "Find 2 polynomial u,v such that a*u+b*v=c","x+1,x^2-2,x", 0, CAT_CATEGORY_POLYNOMIAL},
  {"abs(x)", 0, "Absolute value or norm of x x", "-3", "[1,2,3]", CAT_CATEGORY_COMPLEXNUM | (CAT_CATEGORY_REAL<<8)},
  {"altitude(A,B,C)", 0, "Altitude in triangle ABC from A", "1,i,2+i", 0,CAT_CATEGORY_2D},
  {"append", 0, "Adds an element at the end of a list","#l.append(x)", 0, CAT_CATEGORY_LIST},
  {"approx(x)", 0, "Approx. value x. Shortcut S-D", "pi", 0, CAT_CATEGORY_REAL},
  {"area(objet)", 0, "Algebric area", "circle(0,1)", "triangle(-1,1+i,3)", CAT_CATEGORY_2D  },
  {"arg(z)", 0, "Angle of complex z.", "1+i", 0, CAT_CATEGORY_COMPLEXNUM},
  {"asc(string)", 0, "List of ASCII codes os a string", "\"Hello\"", 0, CAT_CATEGORY_ARIT},
  {"assume(hyp)", 0, "Assumption on variable.", "x>1", "x>-1 and x<1", CAT_CATEGORY_PROGCMD|(CAT_CATEGORY_SOFUS<<8)},
  {"avance n", "avance ", "Turtle forward n steps, default n=10", "#avance 30", 0, CAT_CATEGORY_LOGO},
  {"axes", "axes", "Axes visible or not axes=1 or 0", "#axes=0", 0, CAT_CATEGORY_PROGCMD << 8|XCAS_ONLY},
  {"baisse_crayon ", "baisse_crayon ", "Turtle moves with the pen writing.", 0, 0, CAT_CATEGORY_LOGO},
  {"barplot(list)", 0, "Bar plot of 1-d statistic series data in list.", "[3/2,2,1,1/2,3,2,3/2]", 0, CAT_CATEGORY_STATS},
  {"barycenter([pnt,coeff],...)", 0, "Barycenter of a sequence of [point,coefficient]. Run isobarycenter if all coefficients are equal", "[1,1],[i,1],[2,3]", 0, CAT_CATEGORY_2D | (CAT_CATEGORY_3D << 8) },
  {"binomial(n,p,k)", 0, "binomial(n,p,k) probability to get k success with n trials where p is the probability of success of 1 trial. binomial_cdf(n,p,k) is the probability to get at most k successes. binomial_icdf(n,p,t) returns the smallest k such that binomial_cdf(n,p,k)>=t", "10,.5,4", 0, CAT_CATEGORY_PROBA},
    {"bisector(A,B,C)", 0, "Bisector of angle AB,AC", "1,i,2+i", 0,CAT_CATEGORY_2D},
  {"bitxor", "bitxor", "Exclusive or", "#bitxor(1,2)", 0, CAT_CATEGORY_PROGCMD},
  {"black", "black", "Display option", "#display=black", 0, CAT_CATEGORY_PROGCMD},
  {"blue", "blue", "Display option", "#display=blue", 0, CAT_CATEGORY_PROGCMD},
  {"camembert(list)", 0, "Camembert pie-chart of a 1-d statistical series.", "[[\"France\",6],[\"Germany\",12],[\"Switzerland\",5]]", 0, CAT_CATEGORY_STATS},
  {"cache_tortue ", "cache_tortue ", "Hide turtle (once the picture has been drawn).", 0, 0, CAT_CATEGORY_LOGO},
  {"ceil(x)", 0, "Smallest integer not less than x", "1.2", 0, CAT_CATEGORY_REAL},
  {"center(objet)", 0, "Circle or sphere center. For ellipse or hyperbola, returns center, one focus and a point on the conic. For a parabola, returns focus and vertex.", "circle(0,1)", "sphere([0,0,0],[1,1,1])", CAT_CATEGORY_2D | (CAT_CATEGORY_3D << 8) },
  {"cfactor(p)", 0, "Factorization over C.", "x^4-1", 0, CAT_CATEGORY_ALGEBRA | (CAT_CATEGORY_COMPLEXNUM << 8)},
  {"char(liste)", 0, "Converts a list of ASCII codes to a string.", "[97,98,99]", 0, CAT_CATEGORY_ARIT},
  {"charpoly(M,x)", 0, "Characteristic polynomial of matrix M in variable x.", "[[1,2],[3,4]],x", 0, CAT_CATEGORY_MATRIX},
  {"circle(center,radius)", 0, "Circle", "2+i,3", "1-i,1+i", CAT_CATEGORY_PROGCMD | (CAT_CATEGORY_2D << 8)},
  {"circumcircle(A,B,C)", 0, "Circumcircle", "-1,2+i,3", 0, CAT_CATEGORY_PROGCMD | (CAT_CATEGORY_2D << 8) | XCAS_ONLY},
  {"clearscreen()", "clearscreen()", "Clear screen.", 0, 0, CAT_CATEGORY_PROGCMD|XCAS_ONLY},
  {"coeff(p,x,n)", 0, "Coefficient of x^n in polynomial p.", 0, 0, CAT_CATEGORY_POLYNOMIAL},
  {"comb(n,k)", 0, "Returns nCk", "10,4", 0, CAT_CATEGORY_PROBA},
  {"cond(A,[1,2,inf])", 0, "Nombre de condition d'une matrice par rapport a la norme specifiee (par defaut 1)", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX},
  {"cone(A,v,theta,[h])", 0, " cone with vertex A, direction v, and with half_angle t [and with altitudes h and -h]", "[0,0,0],[0,0,1],pi/6", "[0,0,0],[0,0,1],pi/6,4", CAT_CATEGORY_3D},
  {"conic(expression)", 0, "Conic given by a polynomial equation of degree 2 or by 5 vertices", "x^2+x*y+y^2=5", "1,i,2+i,3-i,4+2i", CAT_CATEGORY_2D},
  {"coordinates(object)", 0, "Coordonnees (cartesian))", "point(1,2)", "point(1,2,3)", CAT_CATEGORY_2D | (CAT_CATEGORY_3D << 8) },
  {"conj(z)", 0, "Complex conjugate of z.", "1+i", 0, CAT_CATEGORY_COMPLEXNUM},
  {"correlation(l1,l2)", 0, "Correlation of lists l1 and l2", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_STATS},
  {"covariance(l1,l2)", 0, "Covariance of lists l1 and l2", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_STATS},
  {"cpartfrac(p,x)", 0, "Partial fraction decomposition over C.", "1/(x^4-1)", 0, CAT_CATEGORY_ALGEBRA | (CAT_CATEGORY_COMPLEXNUM << 8)},
  {"crayon ", "crayon ", "Turtle drawing color", "#crayon red", 0, CAT_CATEGORY_LOGO},
  {"cross(u,v)", 0, "Cross product of vectors u and v.","[1,2,3],[0,1,3]", 0, CAT_CATEGORY_LINALG},
  {"csolve(equation,x)", 0, "Solve equation (or polynomial system) in exact mode over the complex numbers.","x^2+x+1=0", 0, CAT_CATEGORY_SOLVE| (CAT_CATEGORY_COMPLEXNUM << 8)},
  {"cube(A,B,C)", 0, "Cube of edge AB with one face in plane ABC", "[0,0,0],[1,0,0],[0,1,0]","[0,0,0],[0,2,sqrt(5)/2+3/2],[0,0,1]", CAT_CATEGORY_3D},
  {"curl(u,vars)", 0, "Curl of vector u.", "[2*x*y,x*z,y*z],[x,y,z]", 0, CAT_CATEGORY_LINALG},
  {"cyan", "cyan", "Display option", "#display=cyan", 0, CAT_CATEGORY_PROGCMD},
  {"cylinder(A,v,r,[h])", 0, "Cylinder of axis A,v and radius r [and optional altitude h]", "[0,0,0],[0,1,0],2", "[0,0,0],[0,1,0],2,3", CAT_CATEGORY_3D},
  {"debug(f(args))", 0, "Runs user function f in step by step mode.", 0, 0, CAT_CATEGORY_PROG},
  {"degree(p,x)", 0, "Degre of polynomial p in x.", "x^4-1", 0, CAT_CATEGORY_POLYNOMIAL},
  {"denom(x)", 0, "Denominator of expression x.", "3/4", 0, CAT_CATEGORY_POLYNOMIAL},
  {"desolve(equation,t,y)", 0, "Exact differential equation solving.", "desolve([y'+y=exp(x),y(0)=1])", "[y'=[[1,2],[2,1]]*y+[x,x+1],y(0)=[1,2]]", CAT_CATEGORY_SOLVE | (CAT_CATEGORY_CALCULUS << 8)},
  {"det(A)", 0, "Determinant of matrix A.", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX},
  {"diff(f,var,[n])", 0, "Derivative of expression f with respect to var (order n, n=1 by default), for example diff(sin(x),x) or diff(x^3,x,2). For derivation with respect to x, run f' (shortcut F3). For the gradient of f, var is the list of variables.", "sin(x),x", "sin(x^2),x,3", CAT_CATEGORY_CALCULUS},
  {"display", "display", "Display option", "#display=red", 0, CAT_CATEGORY_PROGCMD},
  {"disque n", "disque ", "Filled circle tangent to the turtle, radius n. Run disque n,theta for a filled arc of circle, theta in degrees, or disque n,theta,segment for a segment of circle.", "#disque 30", "#disque(30,90)", CAT_CATEGORY_LOGO},
  {"dodecahedron(A,B,C)", 0, "Dodecahedron of edge AB with one face in plane ABC", "[0,0,0],[0,2,sqrt(5)/2+3/2],[0,0,1]", 0, CAT_CATEGORY_3D},
  {"dot(a,b)", 0, "Dot product of 2 vectors. Shortcut: *", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_LINALG},
  {"draw_arc(x1,y1,rx,ry,theta1,theta2,c)", 0, "Pixelised arc of ellipse.", "100,100,60,80,0,pi,magenta", 0, CAT_CATEGORY_PROGCMD},
  {"draw_circle(x1,y1,r,c)", 0, "Pixelised circle. Option: filled", "100,100,60,cyan+filled", 0, CAT_CATEGORY_PROGCMD},
  {"draw_line(x1,y1,x2,y2,c)", 0, "Pixelised line.", "100,50,300,200,blue", 0, CAT_CATEGORY_PROGCMD},
  {"draw_pixel(x,y,color)", 0, "Colors pixel x,y. Run draw_pixel() to synchronise screen.", 0, 0, CAT_CATEGORY_PROGCMD},
  {"draw_polygon([[x1,y1],...],c)", 0, "Pixelised polygon.", "[[100,50],[30,20],[60,70]],red+filled", 0, CAT_CATEGORY_PROGCMD},
  {"draw_rectangle(x,y,w,h,c)", 0, "Rectangle.", "100,50,30,20,red+filled", 0, CAT_CATEGORY_PROGCMD},
  {"draw_string(s,x,y,c)", 0, "Draw string s at pixel x,y", "\"Bonjour\",80,60", 0, CAT_CATEGORY_PROGCMD},
#ifndef TURTLETAB
  {"ecris ", "ecris ", "Write at turtle position", "#ecris \"hello\"", 0, CAT_CATEGORY_LOGO},
#endif
  {"efface", "efface", "Reset turtle", 0, 0, CAT_CATEGORY_LOGO},
  {"egcd(A,B)", 0, "Find polynomials U,V,D such that A*U+B*V=D=gcd(A,B)","x^2+3x+1,x^2-5x-1", 0, CAT_CATEGORY_POLYNOMIAL},
  {"elif test", "elif ", "Test cascade", 0, 0, CAT_CATEGORY_PROG},
  {"ellipse(F1,F2,M)", 0, "Ellipse given by 2 focus and one point", "-1,1,2", 0, CAT_CATEGORY_2D},
  {"eigenvals(A)", 0, "Eigenvalues of matrix  A.", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX |XCAS_ONLY},
  {"eigenvects(A)", 0, "Eigenvectors of matrix A.", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX},
  {"equation(object)", 0, "Cartesian equation. Run parameq for parametric equation", "circle(0,1)", "ellipse(-1,1,3)", CAT_CATEGORY_2D | (CAT_CATEGORY_3D << 8) },
  {"erf(x)", 0, "Error function of x.", "1.2", 0, CAT_CATEGORY_PROBA},
  {"erfc(x)", 0, "Complementary error function of x.", "1.2", 0, CAT_CATEGORY_PROBA},
  {"euler(n)",0,"Euler indicatrix: number of integers < n coprime with n","25",0,CAT_CATEGORY_ARIT},
  {"eval(f)", 0, "Evals f.", 0, 0, CAT_CATEGORY_PROGCMD},
  {"evalc(z)", 0, "Write z=x+i*y.", "1/(1+i*sqrt(3))", 0, CAT_CATEGORY_COMPLEXNUM},
  {"exact(x)", 0, "Converts x to a rational. Shortcut shift S-D", "1.2", 0, CAT_CATEGORY_REAL},
  {"exp2trig(expr)", 0, "Convert complex exponentials to sin/cos", "exp(i*x)", 0, CAT_CATEGORY_TRIG},
  {"exponential_regression(Xlist,Ylist)", 0, "Exponential regression.", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_STATS},
  {"exponential_regression_plot(Xlist,Ylist)", 0, "Exponential regression plot.", "#X,Y:=[1,2,3,4,5],[0,1,3,4,4];exponential_regression_plot(X,Y);scatterplot(X,Y)", 0, CAT_CATEGORY_STATS},
  {"exponentiald(lambda,x)", 0, "Exponential distribution law of  parameter lambda. exponentiald_cdf(lambda,x) probability that \"exponential distribution <=x\" e.g. exponentiald_cdf(2,3). exponentiald_icdf(lambda,t) returns x such that \"exponential distribution <=x\" has probability t, e.g, exponentiald_icdf(2,0.95) ", "5.1,3.4", 0, CAT_CATEGORY_PROBA},
  {"extend", 0, "Merge 2 lists. Note that + does not merge lists, it adds vectors","#l1.extend(l2)", 0, CAT_CATEGORY_LIST},
  {"factor(p,[x])", 0, "Factors polynomial p (run ifactor for an integer). Shortcut: p=>*", "x^4-1", "x^6+1,sqrt(3)", CAT_CATEGORY_ALGEBRA| (CAT_CATEGORY_POLYNOMIAL << 8)},
  {"filled", "filled", "Display option", 0, 0, CAT_CATEGORY_PROGCMD},
  {"float(x)", 0, "Converts x to a floating point value.", "pi", 0, CAT_CATEGORY_REAL},
  {"floor(x)", 0, "Largest integer not greater than x", "pi", 0, CAT_CATEGORY_REAL},
  {"fourier_an(f,x,T,n,a)", 0, "Cosine Fourier coefficients of f", "x^2,x,2*pi,n,-pi", 0, CAT_CATEGORY_CALCULUS},
  {"fourier_bn(f,x,T,n,a)", 0, "Sine Fourier coefficients of f", "x^2,x,2*pi,n,-pi", 0, CAT_CATEGORY_CALCULUS},
  {"fourier_cn(f,x,T,n,a)", 0, "Exponential Fourier coefficients of f", "x^2,x,2*pi,n,-pi", 0, CAT_CATEGORY_CALCULUS},
  {"from math/... import *", "from math import *", "Access to math or to random functions ([random]) or turtle with English commandnames [turtle]. Math import is not required in KhiCAS", "#from random import *", "#from turtle import *", CAT_CATEGORY_PROG},
  {"fsolve(equation,x=a..b)", 0, "Approx equation solving in interval a..b.","cos(x)=x,x=0..1", "cos(x)-x,x=0.0", CAT_CATEGORY_SOLVE},
  // {"function f(x):...", "function f(x) local y;   ffunction:;", "Function definition.", "#function f(x) local y; y:=x^2; return y; ffunction:;", 0, CAT_CATEGORY_PROG},
  {"gauss(q)", 0, "Quadratic form reduction", "x^2+x*y+x*z+y^2+z^2,[x,y,z]", 0, CAT_CATEGORY_LINALG},
  {"gcd(a,b,...)", 0, "Greatest common divisor. See also iegcd and egcd for extended GCD.", "23,13", "x^2-1,x^3-1", CAT_CATEGORY_ARIT | (CAT_CATEGORY_POLYNOMIAL << 8)},
  {"gl_x", "gl_x", "Display settings X gl_x=xmin..xmax", "#gl_x=0..2", 0, CAT_CATEGORY_PROGCMD},
  {"gl_y", "gl_y", "Display settings Y gl_y=ymin..ymax", "#gl_y=-1..1", 0, CAT_CATEGORY_PROGCMD},
  {"gramschmidt(M)", 0, "Gram-Schmidt orthonormalization (line vectors or linearly independent set of vectors)", "[[1,2,3],[4,5,6]]", "[1,1+x],(p,q)->integrate(p*q,x,-1,1)", CAT_CATEGORY_LINALG},
  {"green", "green", "Display option", "#display=green", 0, CAT_CATEGORY_PROGCMD},
  {"halftan(expr)", 0, "Convert cos, sin, tan with tan(angle/2).","cos(x)", 0, CAT_CATEGORY_TRIG},
  {"hermite(n)", 0, "n-th Hermite polynomial", "10", 0, CAT_CATEGORY_POLYNOMIAL},
  {"hilbert(n)", 0, "Hilbert matrix of order n.", "4", 0, CAT_CATEGORY_MATRIX},
  {"histogram(list,min,size)", 0, "Histogram of data in list, classes begin at min of size size.","ranv(100,uniformd,0,1),0,0.1", 0, CAT_CATEGORY_STATS},
  {"homothety(center,ratio,object)", 0, "Image of object by homothety of ratio", "0,2,circle(1,1)", 0, CAT_CATEGORY_2D },
  {"hyperbola(F1,F2,M)", 0, "Hyperbola given by 2 focus and one point", "-2-i,2+i,1", 0, CAT_CATEGORY_2D},
  {"iabcuv(a,b,c)", 0, "Find 2 integers u,v such that a*u+b*v=c","23,13,15", 0, CAT_CATEGORY_ARIT},
  {"ichinrem([a,m],[b,n])", 0,"Integer chinese remainder of a mod m and b mod n.", "[3,13],[2,7]", 0, CAT_CATEGORY_ARIT},
  {"icosahedron(A,B,C)", 0, "Icosahedron with center A, vertex B and such that the plane ABC contains one vertex among the 5 nearest vertices from B ", "[0,0,0],[sqrt(5),0,0],[1,2,0]", 0, CAT_CATEGORY_3D},
   {"idivis(n)", 0, "Returns the list of divisors of an integer n.", "10", 0, CAT_CATEGORY_ARIT},
  {"idn(n)", 0, "Identity matrix of order n", "4", 0, CAT_CATEGORY_MATRIX},
  {"iegcd(a,b)", 0, "Find integers u,v,d such that a*u+b*v=d=gcd(a,b)","23,13", 0, CAT_CATEGORY_ARIT},
  {"ifactor(n)", 0, "Factorization of an integer (not too large!). Shortcut n=>*", 0, 0, CAT_CATEGORY_ARIT},
  {"ilaplace(f,s,x)", 0, "Inverse Laplace transform of f", "s/(s^2+1),s,x", 0, CAT_CATEGORY_CALCULUS},
  {"im(z)", 0, "Imaginary part.", "1+i", 0, CAT_CATEGORY_COMPLEXNUM},
  {"incircle(A,B,C)", 0, "Incircle", "-1,2+i,3", 0, CAT_CATEGORY_PROGCMD | (CAT_CATEGORY_2D << 8) | XCAS_ONLY},
  {"inf", "inf", "Plus infinity. -inf for minus infinity and infinity for unsigned/complex infinity. Shortcut shift INS.", "oo", 0, CAT_CATEGORY_CALCULUS},
  {"input()", "input()", "Read a string from keyboard", 0, 0, CAT_CATEGORY_PROG},
  {"integrate(f,x,[a,b])", 0, "Antiderivative of f with respect to x, like integrate(x*sin(x),x). For definite integral enter optional arguments a and b, like integrate(x*sin(x),x,0,pi). Shortcut SHIFT F3.", "x*sin(x),x", "cos(x)/(1+x^4),x,0,inf", CAT_CATEGORY_CALCULUS},
  {"interp(X,Y)", 0, "Lagrange interpolation at points (xi,yi) where X is the list of xi and Y of yi. If interp is passed as 3rd argument, returns the divided differences list.", "[1,2,3,4,5],[0,1,3,4,4]", "[1,2,3,4,5],[0,1,3,4,4],interp", CAT_CATEGORY_POLYNOMIAL},
  {"inter(A,B)", 0, "Intersections list. Run single_inter if intersection is unique.", "line(y=x),circle(0,1)", 0, CAT_CATEGORY_3D | (CAT_CATEGORY_2D << 8) | XCAS_ONLY},
  {"inv(A)", 0, "Inverse of A.", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX},
  {"iquo(a,b)", 0, "Integer quotient of a and b.", "23,13", 0, CAT_CATEGORY_ARIT},
  {"irem(a,b)", 0,"Integer remainder of a and b.", "23,13", 0, CAT_CATEGORY_ARIT},
  {"isprime(n)", 0, "Returns 1 if n is prime, 0 otherwise.", "11", "10", CAT_CATEGORY_ARIT},
  {"is_collinear(A,B,C)", 0, "Returns 1 if A, B, C are collinear, 0 otherwise", "1,i,-1", "i,0,-i", CAT_CATEGORY_2D | XCAS_ONLY },
  {"is_concyclic(A,B,C,D)", 0, "Returns 1 if A, B, C, D are concyclic, 0 otherwise", "1,i,-1,-i", "1,i,0,-i", CAT_CATEGORY_2D | XCAS_ONLY },
  {"is_element(A,G)", 0, "Returns 1 if A belongs to G, 0 otherwise.", "point(0),circle(0,1)", "point(i),square(0,1)", CAT_CATEGORY_2D | XCAS_ONLY },
  {"is_parallel(D,E)", 0, "Returns 1 if D and E are parallel, 0 otherwise", "line(y=x),line(y=-x)", "line(y=x),line(y=x+1)", CAT_CATEGORY_2D | XCAS_ONLY },
  {"is_perpendicular(D,E)", 0, "Returns 1 if D and E are perpendicular, 0 otherwise", "line(y=x),line(y=-x)", "line(y=x),line(y=x+1)", CAT_CATEGORY_2D | XCAS_ONLY },
  {"jordan(A)", 0, "Jordan normal form of matrix A, returns P and D such that P^-1*A*P=D", "[[1,2],[3,4]]", "[[1,1,-1,2,-1],[2,0,1,-4,-1],[0,1,1,1,1],[0,1,2,0,1],[0,0,-3,3,-1]]", CAT_CATEGORY_MATRIX},
  {"laguerre(n,a,x)", 0, "n-ieme Laguerre polynomial (default a=0).", "10", 0, CAT_CATEGORY_POLYNOMIAL},
  {"laplace(f,x,s)", 0, "Laplace transform of f","sin(x),x,s", 0, CAT_CATEGORY_CALCULUS},
  {"lcm(a,b,...)", 0, "Least common multiple.", "23,13", "x^2-1,x^3-1", CAT_CATEGORY_ARIT | (CAT_CATEGORY_POLYNOMIAL << 8)},
  {"lcoeff(p,x)", 0, "Leading coefficient of polynomial p in x.", "x^4-1", 0, CAT_CATEGORY_POLYNOMIAL},
  {"legendre(n)", 0, "n-the Legendre polynomial.", "10", "10,t", CAT_CATEGORY_POLYNOMIAL},
#ifdef RELEASE
  {"len(l)", 0, "Size of a list.", "[3/2,2,1,1/2,3,2,3/2]", 0, CAT_CATEGORY_LIST},
#endif
  {"leve_crayon ", "leve_crayon ", "Turtle moves without trace.", 0, 0, CAT_CATEGORY_LOGO},
  {"limit(f,x=a)", 0, "Limit of f at x = a. Add 1 or -1 for unidirectional limits, e.g. limit(sin(x)/x,x=0) or limit(abs(x)/x,x=0,1). Shortcut: SHIFT MIXEDFRAC", "sin(x)/x,x=0", "exp(-1/x),x=0,1", CAT_CATEGORY_CALCULUS},
  {"line(equation)", 0, "Line of equation", "y=2x+1", "[0,0,0],[1,-2,3]", CAT_CATEGORY_PROGCMD |(CAT_CATEGORY_2D << 8)|(CAT_CATEGORY_2D << 16)},
  {"line_width_", "line_width_", "Width prefix (2 to 8)", 0, 0, CAT_CATEGORY_PROGCMD},
  {"linear_regression(Xlist,Ylist)", 0, "Linear regression.", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_STATS},
  {"linear_regression_plot(Xlist,Ylist)", 0, "Linear regression plot.", "#X,Y:=[1,2,3,4,5],[0,1,3,4,4];linear_regression_plot(X,Y);scatterplot(X,Y)", 0, CAT_CATEGORY_STATS},
  {"linetan(expr,x,x0)", 0, "Tangent to the graph at x=x0.", "sin(x),x,pi/2", 0, CAT_CATEGORY_PLOT},
  {"linsolve([eq1,eq2,..],[x,y,..])", 0, "Linear system solving. May use the output of lu for O(n^2) solving (see example 2).","[x+y=1,x-y=2],[x,y]", "#p,l,u:=lu([[1,2],[3,4]]); linsolve(p,l,u,[5,6])", CAT_CATEGORY_SOLVE | (CAT_CATEGORY_LINALG <<8) | (CAT_CATEGORY_MATRIX << 16)},
  {"logarithmic_regression(Xlist,Ylist)", 0, "Logarithmic egression.", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_STATS},
  {"logarithmic_regression_plot(Xlist,Ylist)", 0, "Logarithmic regression plot.", "#X,Y:=[1,2,3,4,5],[0,1,3,4,4];logarithmic_regression_plot(X,Y);scatterplot(X,Y)", 0, CAT_CATEGORY_STATS},
  {"lu(A)", 0, "LU decomposition LU of matrix A, P*A=L*U", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX},
  {"magenta", "magenta", "Display option", "#display=magenta", 0, CAT_CATEGORY_PROGCMD},
  {"map(f,l)", 0, "Maps f on element of list l.","lambda x:x*x,[1,2,3]", 0, CAT_CATEGORY_LIST},
  {"matpow(A,n)", 0, "Returns matrix A^n", "[[1,2],[3,4]],n","#assume(n>=1);matpow([[0,2],[0,4]],n)",  CAT_CATEGORY_MATRIX},
  {"matrix(r,c,func)", 0, "Matrix from a defining function.", "2,3,(j,k)->j^k", 0, CAT_CATEGORY_MATRIX},
  {"mean(l)", 0, "Arithmetic mean of list l", "[3/2,2,1,1/2,3,2,3/2]", 0, CAT_CATEGORY_STATS},
  {"median(l)", 0, "Median", "[3/2,2,1,1/2,3,2,3/2]", 0, CAT_CATEGORY_STATS},
  {"median_line(A,B,C)", 0, "Median line of triangle ABC from vertex A", "1,i,2+i", 0,CAT_CATEGORY_2D},
  {"midpoint(A,B)", 0, "Midpoint of segment AB", "1,i", 0,CAT_CATEGORY_2D | (CAT_CATEGORY_3D << 8)},
  {"montre_tortue ", "montre_tortue ", "Displays the turtle", 0, 0, CAT_CATEGORY_LOGO},
  {"mult_c_conjugate", 0, "Multiplier par le conjugue complexe.", "1+2*i", 0,  (CAT_CATEGORY_COMPLEXNUM << 8)},
  {"mult_conjugate", 0, "Multiplier par le conjugue (sqrt).", "sqrt(2)-sqrt(3)", 0, CAT_CATEGORY_ALGEBRA},
  {"normald([mu,sigma],x)", 0, "Normal distribution probability density, by default mu=0 and sigma=1. normald_cdf([mu,sigma],x) probability that \"normal distribution <=x\" e.g. normald_cdf(1.96). normald_icdf([mu,sigma],t) returns x such that \"normal distribution <=x\" has probability t, e.g. normald_icdf(0.975) ", "1.2", 0, CAT_CATEGORY_PROBA},
  {"not(x)", 0, "Logical not.", 0, 0, CAT_CATEGORY_PROGCMD},
  {"numer(x)", 0, "Numerator of x.", "3/4", 0, CAT_CATEGORY_POLYNOMIAL},
  {"octahedron(A,B,C)", 0, "Octahedron of edge AB with one face in plane ABC", "[0,0,0],[3,0,0],[0,1,0]", 0, CAT_CATEGORY_3D},
  {"odesolve(f(t,y),[t,y],[t0,y0],t1)", 0, "Approx. solution of differential equation y'=f(t,y) and y(t0)=y0, value for t=t1 (add curve to get intermediate values of y)", "sin(t*y),[t,y],[0,1],2", "0..pi,(t,v)->{[-v[1],v[0]]},[0,1]", CAT_CATEGORY_SOLVE},
  {"parabola(F,A)", 0, "Parabola given by focus and vertex", "-2-i,2+i", 0, CAT_CATEGORY_2D},
  {"parameq(object)", 0, "Parametric equations. Run equation for cartesian equation", "circle(0,1)", "ellipse(-1,1,3)", CAT_CATEGORY_2D | (CAT_CATEGORY_3D << 8) },
  {"partfrac(p,x)", 0, "Partial fraction expansion. Shortcut p=>+", "1/(x^4-1)", 0, CAT_CATEGORY_ALGEBRA},
  {"pas_de_cote n", "pas_de_cote ", "Turtle side jump from n steps, by default n=10", "#pas_de_cote 30", 0, CAT_CATEGORY_LOGO},
  {"perpen_bisector(A,B)", 0, "Perpendicular bisector of segment AB", "1,i", 0,CAT_CATEGORY_2D},
  {"plane(equation)", 0, "Plane given by equation or by 3 points", "z=x+y-1", "[0,0,0],[1,0,0],[0,1,0]", CAT_CATEGORY_3D | XCAS_ONLY},
  {"plot(expr,x)", 0, "Plot an expression. For example plot(sin(x)), plot(ln(x),x.0,5), plot(x^2-y^2), plot(x^2-y^2<1), plot(x^2-y^2=1)", "ln(x),x,0,5", "1/x,x=1..5,xstep=1", (CAT_CATEGORY_PLOT << 8) | (CAT_CATEGORY_3D)},
#ifdef RELEASE
  {"plotarea(expr,x=a..b,[n,meth])", 0, "Area under curve with specified quadrature.", "1/x,x=1..3,2,trapezoid", 0, CAT_CATEGORY_PLOT},
#endif
  {"plotcontour(expr,[x=xm..xM,y=ym..yM],levels)", 0, "Levels of expr.", "x^2+2y^2,[x=-2..2,y=-2..2],[1,2]", 0, CAT_CATEGORY_PLOT},
  {"plotfield(f(t,y),[t=tmin..tmax,y=ymin..ymax])", 0, "Plot field of differential equation y'=f(t,y), an optionally one solution by adding plotode=[t0,y0]", "sin(t*y),[t=-3..3,y=-3..3],plotode=[0,1]", 0, CAT_CATEGORY_PLOT},
  {"plotfunc(expr,[x,y])", 0, "Xcas: graph of a 3d function", "x^2-y^2,[x,y]","x^2-y^2,[x=-2..2,y=-2..2],nstep=700", CAT_CATEGORY_PLOT | (CAT_CATEGORY_3D << 8) | XCAS_ONLY },
  {"plotlist(list)", 0, "Plot a list", "[3/2,2,1,1/2,3,2,3/2]", 0, CAT_CATEGORY_PLOT},
  {"plotode(f(t,y),[t=tmin..tmax,y],[t0,y0])", 0, "Plot solution of differential equation y'=f(t,y), y(t0)=y0.", "sin(t*y),[t=-3..3,y],[0,1]", 0, CAT_CATEGORY_PLOT},
  {"plotparam([x,y],t)", 0, "Parametric plot. For example plotparam([sin(3t),cos(2t)],t,0,pi) or plotparam(exp(i*t),t,0,pi)", "[sin(3t),cos(2t)],t,0,pi", "[t^2,t^3],t=-1..1,tstep=0.1", CAT_CATEGORY_PLOT},
  {"plotpolar(r,theta)", 0, "Polar plot.","cos(3*x),x,0,pi", "1/(1+cos(x)),x=0..pi,xstep=0.05", CAT_CATEGORY_PLOT},
  {"plotseq(f(x),x=[u0,m,M],n)", 0, "Plot f(x) on [m,M] and n terms of the sequence defined by u_{n+1}=f(u_n) and u0.","sqrt(2+x),x=[6,0,7],5", 0, CAT_CATEGORY_PLOT},
  {"plus_point", "plus_point", "Display option", "#display=blue+plus_point", 0, CAT_CATEGORY_PROGCMD},
  {"point(x,y[,z])", 0, "Point", "1,2", "1,2,3", CAT_CATEGORY_PLOT | (CAT_CATEGORY_2D << 8)},
  {"polygon(list)", 0, "Closed polygon given by a list of vertices.", "1-i,2+i,3,3-2i", 0, CAT_CATEGORY_PROGCMD | (CAT_CATEGORY_2D << 8) },
  {"polygonscatterplot(Xlist,Ylist)", 0, "Plot points and polygonal line.", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_STATS},
  {"polyhedron(A,B,C,D,...)", 0, "Convex polyhedron of vertices in A,B,C,D,...", "[0,0,0],[0,5,0],[0,0,5],[1,2,6]", 0, CAT_CATEGORY_3D},
  {"polynomial_regression(Xlist,Ylist,n)", 0, "Polynomial regression, degree <= n.", "[1,2,3,4,5],[0,1,3,4,4],2", 0, CAT_CATEGORY_STATS},
  {"polynomial_regression_plot(Xlist,Ylist,n)", 0, "Polynomial regression plot, degree <= n.", "#X,Y:=[1,2,3,4,5],[0,1,3,4,4];polynomial_regression_plot(X,Y,2);scatterplot(X,Y)", 0, CAT_CATEGORY_STATS},
  //{"pour", "pour j de 1 jusque  faire  fpour;", "For loop.","#pour j de 1 jusque 10 faire print(j,j^2); fpour;", 0, CAT_CATEGORY_PROG},
  {"power_regression(Xlist,Ylist,n)", 0, "Power regression.", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_STATS},
  {"power_regression_plot(Xlist,Ylist,n)", 0, "Power regression graph", "#X,Y:=[1,2,3,4,5],[0,1,3,4,4];power_regression_plot(X,Y);scatterplot(X,Y)", 0, CAT_CATEGORY_STATS},
  {"powmod(a,n,p)", 0, "Returns a^n mod p.","123,456,789", 0, CAT_CATEGORY_ARIT},
  {"print(expr)", 0, "Print expr in console", 0, 0, CAT_CATEGORY_PROG},
  {"projection(obj1,obj2)", 0, "Projection on obj1 of obj2", "line(y=x),point(2,3)", 0, CAT_CATEGORY_2D },
  {"proot(p)", 0, "Returns real and complex roots, of polynomial p. Exemple proot([1,2.1,3,4.2]) or proot(x^3+2.1*x^2+3x+4.2)", "x^3+2.1*x^2+3x+4.2", 0, CAT_CATEGORY_POLYNOMIAL},
  {"purge(x)", 0, "Clear assigned variable x. Shortcut SHIFT-FORMAT", 0, 0, CAT_CATEGORY_PROGCMD|(CAT_CATEGORY_SOFUS<<8)},
  {"python(f)", 0, "Displays f in Python syntax.", 0, 0, CAT_CATEGORY_PROGCMD},
  {"python_compat(0|1|2)", 0, "python_compat(0) Xcas syntax, python_compat(1) Python syntax with ^ interpreted as power, python_compat(2) ^ as bit xor", "0", "1", CAT_CATEGORY_PROG},
  {"qr(A)", 0, "A=Q*R factorization with Q orthogonal and R upper triangular", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX},
  {"quadric(equation)", 0, "Quadric given by equation (or 9 points)", "x^2-y^2+z^2", "x^2+x*y+y^2+z^2-3", CAT_CATEGORY_3D},
  {"quartile1(l)", 0, "1st quartile", "[3/2,2,1,1/2,3,2,3/2]", 0, CAT_CATEGORY_STATS},
  {"quartile3(l)", 0, "3rd quartile", "[3/2,2,1,1/2,3,2,3/2]", 0, CAT_CATEGORY_STATS},
  {"quo(p,q,x)", 0, "Quotient of synthetic division of polynomials p and q (variable x).", 0, 0, CAT_CATEGORY_POLYNOMIAL},
  {"quote(x)", 0, "Returns expression x unevaluated.", 0, 0, CAT_CATEGORY_ALGEBRA},
  {"radius(objet)", 0, "Radius of a circle or sphere", "circle(0,1)", "sphere([0,0,0],[1,1,1])", CAT_CATEGORY_2D | (CAT_CATEGORY_3D << 8) },
  {"rand()", "rand()", "Random real between 0 and 1", 0, 0, CAT_CATEGORY_PROBA},
  {"randint(a,b)", 0, "Random integer between a and b. With 1 argument in Xcas, random integer between 1 and n.", "5,25", "6", CAT_CATEGORY_PROBA},
  {"ranm(n,m,[loi,parametres])", 0, "Random matrix with integer coefficients or according to a probability law (ranv for a vector). Examples ranm(2,3), ranm(3,2,binomial,20,.3), ranm(4,2,normald,0,1)", "3,3","4,2,normald,0,1",  CAT_CATEGORY_MATRIX},
  {"ranv(n,[loi,parametres])", 0, "Random vector.", "10","4,normald,0,1", CAT_CATEGORY_LINALG},
  {"ratnormal(x)", 0, "Puts everything over a common denominator.", 0, 0, CAT_CATEGORY_ALGEBRA},
  {"re(z)", 0, "Real part.", "1+i", 0, CAT_CATEGORY_COMPLEXNUM},
  {"read(\"filename\")", "read(\"", "Read a file.", 0, 0, CAT_CATEGORY_PROGCMD},
  {"rectangle_plein a,b", "rectangle_plein ", "Direct filled rectangle from turtle position, if b is omitted b==a", "#rectangle_plein 30","#rectangle_plein 20,40", CAT_CATEGORY_LOGO},
  {"recule n", "recule ", "Turtle backward n steps, n=10 by default", "#recule 30", 0, CAT_CATEGORY_LOGO},
  {"red", "red", "Display option", "#display=red", 0, CAT_CATEGORY_PROGCMD},
  {"reflection(obj1,obj2)", 0, "Reflection or symmetrical of obj2", "line(y=x),cercle(1,1)", 0, CAT_CATEGORY_2D },
  {"rem(p,q,x)", 0, "Remainder of synthetic division of polynomials p and q (variable x)", 0, 0, CAT_CATEGORY_POLYNOMIAL},
#ifdef RELEASE
  {"residue(f(z),z,z0)", 0, "Residue of an expression at z0.", "1/(x^2+1),x,i", 0, CAT_CATEGORY_COMPLEXNUM},
#endif
  {"resultant(p,q,x)", 0, "Resultant in x of polynomials p and q.", "#P:=x^3+p*x+q;resultant(P,P',x);", 0, CAT_CATEGORY_POLYNOMIAL},
  {"revert(p[,x])", 0, "Revert Taylor series","x+x^2+x^4", 0, CAT_CATEGORY_CALCULUS},
  {"rgb(r,g,b)", 0, "color defined from red, green, blue from 0 to 255", "255,0,255", 0, CAT_CATEGORY_PROGCMD},
  {"rhombus_point", "rhombus_point", "Display option", "#display=magenta+rhombus_point", 0, CAT_CATEGORY_PROGCMD},
  {"rond n", "rond ", "Circle tangent to the turtle, radius n. Run rond n,theta for an arc of circle of theta degrees", 0, 0, CAT_CATEGORY_LOGO},
  {"rotation(center,angle,objcet)", 0, "Image of object by rotation", "2-i,pi/2,circle(0,1)", "sphere([0,0,0],[1,1,1])", CAT_CATEGORY_2D | (CAT_CATEGORY_3D << 8) },
  {"rsolve(equation,u(n),[init])", 0, "Solve a recurrence relation.","u(n+1)=2*u(n)+3,u(n),u(0)=1", "([u(n+1)=3*v(n)+u(n),v(n+1)=v(n)+u(n)],[u(n),v(n)],[u(0)=1,v(0)=2]", CAT_CATEGORY_SOLVE},
  {"saute n", "saute ", "Turtle jumps n steps, by default n=10", "#saute 30", 0, CAT_CATEGORY_LOGO},
  {"scatterplot(Xlist,Ylist)", 0, "Draws points", "[1,2,3,4,5],[0,1,3,4,4]", 0, CAT_CATEGORY_STATS},
  {"segment(A,B)", 0, "Segment", "1,2+i", "[1,2,1],[-1,3,2]", CAT_CATEGORY_PROGCMD | (CAT_CATEGORY_2D << 8) | XCAS_ONLY},
  {"seq(expr,var,a,b)", 0, "Generates a list from an expression.","j^2,j,1,10", 0, CAT_CATEGORY_PROGCMD},
  //{"si", "si  alors  sinon  fsi;", "Test.", "#f(x):=si x>0 alors x; sinon -x; fsi;// valeur absolue", 0, CAT_CATEGORY_PROG},
  {"sign(x)", 0, "Returns -1 if x is negative, 0 if x is zero and 1 if x is positive.", 0, 0, CAT_CATEGORY_REAL|XCAS_ONLY},
  {"similarity(center,ratio,angle,object)", 0, "Image of object by similarity", "0,2,pi/2,circle(1,1)", 0, CAT_CATEGORY_2D },
  {"simplify(expr)", 0, "Returns x in a simpler form. Shortcut expr=>/", "sin(3x)/sin(x)", 0, CAT_CATEGORY_ALGEBRA},
  {"single_inter(A,B)", 0, "First intersection. Run inter for a list of intersections.", "line(y=x),line(x+y=3)", 0, CAT_CATEGORY_3D | (CAT_CATEGORY_2D << 8) | XCAS_ONLY},
  {"solve(equation,x)", 0, "Exact solving of equation w.r.t. x (or of a polynomial system). Run csolve for complex solutions, linsolve for a linear system. Shortcut SHIFT XthetaT", "x^2-x-1=0,x", "[x^2-y^2=0,x^2-z^2=0],[x,y,z]", CAT_CATEGORY_SOLVE},
  {"sorted(l)", 0, "Sorts a list.","[3/2,2,1,1/2,3,2,3/2]", "[[1,2],[2,3],[4,3]],(x,y)->when(x[1]==y[1],x[0]>y[0],x[1]>y[1]", CAT_CATEGORY_LIST},
  {"sphere(A,r)", 0, "Sphere of center A and radius r or diameter AB", "[0,0,0],1", "[0,0,0],[1,1,1]", CAT_CATEGORY_3D},
  {"square_point", "square_point", "Display option", "#display=cyan+square_point", 0, CAT_CATEGORY_PROGCMD},
  {"star_point", "star_point", "Display option", "#display=magenta+star_point", 0, CAT_CATEGORY_PROGCMD},
  {"stddev(l)", 0, "Standard deviation of list l", "[3/2,2,1,1/2,3,2,3/2]", 0, CAT_CATEGORY_STATS},
  {"subst(a,b=c)", 0, "Substitutes b for c in a. Shortcut a(b=c).", "x^2,x=3", 0, CAT_CATEGORY_ALGEBRA},
  {"sum(f,k,m,M)", 0, "Summation of expression f for k from m to M. Exemple sum(k^2,k,1,n)=>*. Shortcut ALPHA F3", "k,k,1,n", 0, CAT_CATEGORY_CALCULUS},
  {"svd(A)", 0, "Singular Value Decomposition, returns U orthogonal, S vector of singular values, Q orthogonal such that A=U*diag(S)*tran(Q).", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX},
  {"tabvar(f,[x=a..b])", 0, "Table of variations of expression f, optional arguments variable x in interval a..b", "sqrt(x^2+x+1)",  "[cos(t),sin(3t)],t", CAT_CATEGORY_CALCULUS},
  //{"tantque", "tantque  faire   ftantque;", "While loop.", "#j:=13; tantque j!=1 faire j:=when(even(j),j/2,3j+1); print(j); ftantque;", 0, CAT_CATEGORY_PROG},
  {"taylor(f,x=a,n,[polynom])", 0, "Taylor expansion of f of x at a order n, add parameter polynom to remove remainder term.","sin(x),x=0,5", "sin(x),x=0,5,polynom", CAT_CATEGORY_CALCULUS},
  {"tchebyshev1(n)", 0, "Tchebyshev polynomial 1st kind: cos(n*x)=T_n(cos(x))", "10", 0, CAT_CATEGORY_POLYNOMIAL},
  {"tchebyshev2(n)", 0, "Tchebyshev polynomial 2nd kind: sin((n+1)*x)=sin(x)*U_n(cos(x))", "10", 0, CAT_CATEGORY_POLYNOMIAL},
  {"tcollect(expr)", 0, "Linearize and collect trig functions.","sin(x)+cos(x)", 0, CAT_CATEGORY_TRIG},
  {"texpand(expr)", 0, "Expand trigonometric, exp and ln functions.","sin(3x)", 0, CAT_CATEGORY_TRIG},
  {"time(cmd)", 0, "Time to run a command or set the clock","int(1/(x^4+1),x)","8,0", CAT_CATEGORY_PROG},
  {"tlin(expr)", 0, "Trigonometric linearization of expr.","sin(x)^3", 0, CAT_CATEGORY_TRIG},
  {"tourne_droite n", "tourne_droite ", "Turtle turns right n degrees, n=90 by default", 0, 0, CAT_CATEGORY_LOGO},
  {"tourne_gauche n", "tourne_gauche ", "Turtle turns left n degrees, n=90 by default", 0, 0, CAT_CATEGORY_LOGO},
  {"trace(A)", 0, "Trace of the matrix A.", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX},
  {"transpose(A)", 0, "Transposes matrix A. Transconjugate command is trn(A) or A^*.", "[[1,2],[3,4]]", 0, CAT_CATEGORY_MATRIX},
  {"translation(vect,obj)", 0, "Translate by vect obj", "[1,2],cercle(0,1)", 0, CAT_CATEGORY_2D },
  {"triangle(A,B,C)", 0, "Triangle given by 3 vertices", "1+i,1-i,-1", "A,B,C", CAT_CATEGORY_2D},
  {"triangle_point", "triangle_point", "Display option", "#display=yellow+triangle_point", 0, CAT_CATEGORY_PROGCMD},
  {"trig2exp(expr)", 0, "Convert complex exponentials to trigonometric functions","cos(x)^3", 0, CAT_CATEGORY_TRIG},
  {"trigcos(expr)", 0, "Convert sin^2 and tan^2 to cos^2.","sin(x)^4", 0, CAT_CATEGORY_TRIG},
  {"trigsin(expr)", 0, "Convert cos^2 and tan^2 to sin^2.","cos(x)^4", 0, CAT_CATEGORY_TRIG},
  {"trigtan(expr)", 0, "Convert cos^2 and sin^2 to tan^2.","cos(x)^4", 0, CAT_CATEGORY_TRIG},
  {"uniformd(a,b,x)", "uniformd", "uniform law on [a,b] of density 1/(b-a)", 0, 0, CAT_CATEGORY_PROBA},
  {"vertices(objet)", 0, "List of vertices of a polygon or polyhedra", "triangle(1,i,2)", "cube([0,0,0],[1,0,0],[0,1,0])", CAT_CATEGORY_2D | (CAT_CATEGORY_3D << 8) },
  //{"version", "version()", "Khicas 1.5.0, (c) B. Parisse et al. www-fourier.ujf-grenoble.fr/~parisse\nLicense GPL version 2. Interface adapted from Eigenmath for Casio, G. Maia, http://gbl08ma.com. Do not use if CAS calculators are forbidden.", 0, 0, CAT_CATEGORY_PROGCMD},
  {"write(\"filename\",var)", "write(\"", "Save 1 or more variables in a file. For example f(x):=x^2; write(\"func_f\",f).",  0, 0, CAT_CATEGORY_PROGCMD},
  {"yellow", "yellow", "Display option", "#display=yellow", 0, CAT_CATEGORY_PROGCMD},
  {"|", "|", "Logical or", "#1|2", 0, CAT_CATEGORY_PROGCMD},
  {"~", "~", "Complement", "#~7", 0, CAT_CATEGORY_PROGCMD},
};

  const char aide_khicas_string[]="Aide Khicas";
#ifdef NUMWORKS
  const char shortcuts_fr_string[]="Raccourcis clavier (shell et editeur)\nshift-/: %\nalpha shift \": '\nshift--: \\\nshift-ans: completion\nshift-*: factor\nshift-+: normal\nshift-1 a 6: selon bandeau en bas\nshift-7: matrices\nshift-8: complexes\nshift-9:arithmetique entiere\nshift-0: probas\nshift-.: reels\nshift-10^: polynomes\nvar: liste des variables\nans: figure tortue (editeur)\n\nshift-x^y (sto) renvoie =>\n=>+: partfrac\n=>*: factor\n=>sin/cos/tan\n=>=>: solve\n\nShell:\nshift-5: Editeur 2d ou graphique ou texte selon objet\nshift-6: editeur texte\n+ ou - modifie un parametre en surbrillance\n\nEditeur d'expressions\nshift-cut: defaire/refaire (1 fois)\npave directionnel: deplace la selection dans l'arborescence de l'expression\nshift-droit/gauche echange selection avec argument a droite ou a gauche\nalpha-droit/gauche dans une somme ou un produit: augmente la selection avec argument droit ou gauche\nshift-4: Editer selection, shift-5: taille police + ou - grande\nEXE: evaluer la selection\nshift-6: valeur approchee\nBackspace: supprime l'operateur racine de la selection\n\nEditeur de scripts\nEXE: passage a la ligne\nshift-CUT: documentation\nshift COPY (ou shift et deplacement curseur simultanement): marque le debut de la selection, deplacer le curseur vers la fin puis Backspace pour effacer ou shift-COPY pour copier sans effacer. shift-PASTE pour coller.\nHome-6 recherche seule: entrer un mot puis EXE puis EXE. Taper EXE pour l'occurence suivante, Back pour annuler.\nHome-6 remplacer: entrer un mot puis EXE puis le remplacement et EXE. Taper EXE ou Back pour remplacer ou non et passer a l'occurence suivante, AC pour annuler\nOK: tester syntaxe\n\nRaccourcis Graphes:\n+ - zoom\n(-): zoomout selon y\n*: autoscale\n/: orthonormalisation\nOPTN: axes on/off";
  const char shortcuts_en_string[]="Keyboard shortcuts (shell and editor)\nshift-/: %\nalpha shift \": '\nshift--: \\\nshift ans: completion\nshift-*: factor\nshift-+: normal\nshift-1 to 6: cf. screen bottom\nshift-7: matrices\nshift-8: complexes\nshift-9:arithmetic\nshift-0: proba\nshift-.: reals\nshift-10^: polynomials\nvar: variables list\nans: turtle screen (editor)\n\nshift-x^y (sto) returns =>\n=>+: partfrac\n=>*: factor\n=>sin/cos/tan\n=>=>: solve\n\nShell:\nshift-5: 2d editor or graph or text\nshift-6: text edit\n+ ou - modifies selected slider\n\nExpressions editor\nshift-cut: undo/redo (1 fois)\nkeypad: move selection inside expression tree\nshift-right/left exchange selection with right or left argument\nalpha-right/left: inside a sum or product: increase selection with right or left argument\nshift-4: Edit selection, shift-5: change fontsize\nEXE: eval selection\nshift-6: approx value\nBackspace: suppress selection's rootnode operator\n\nScript Editor\nEXE: newline\nshift-CUT: documentation\nshift-COPY: marks selection begin, move the cursor to the end, then hit Backspace to erase or shift-COPY to copy (no erase). shift-PASTE to paste.\nHome-6 search: enter a word then EXE then again EXE. Type EXE for next occurence, Back to cancel.\nHome-6 replace: enter a word then EXE then replacement word then EXE. Type EXE or Back to replace or ignore and go to next occurence, AC to cancel\nOK: test syntax\n\nGraph shortcuts:\n+ - zoom\n(-): zoomout along y\n*: autoscale\n/: orthonormalization\nOPTN: axes on/off";
#else
  const char shortcuts_fr_string[]="Raccourcis clavier (shell et editeur)\nlivre: aide/complete\ntab: complete (shell)/indente (editeur)\nshift-/: %\nshift *: '\nctrl-/: \\\nshift-1 a 6: selon bandeau en bas\nshift-7: matrices\nshift-8: complexes\nshift-9:arithmetique\nshift-0: probas\nshift-.: reels\nctrl P: programme\nvar: liste des variables\nans (shift (-)): figure tortue (editeur)\n\nctrl-var (sto) renvoie =>\n=>+: partfrac\n=>*: factor\n=>sin/cos/tan\n=>=>: solve\n\nShell:\nshift-5: Editeur 2d ou graphique ou texte selon objet\nshift-4: editeur texte\n+ ou - modifie un parametre en surbrillance\n\nEditeur d'expressions\nctrl z: defaire/refaire (1 fois)\npave directionnel: deplace la selection dans l'arborescence de l'expression\nshift-droit/gauche echange selection avec argument a droite ou a gauche\nctrl droit/gauche dans une somme ou un produit: augmente la selection avec argument droit ou gauche\nshift-4: Editer selection, shift-5: taille police + ou - grande\nenter: evaluer la selection\nshift-6: valeur approchee\nDel: supprime l'operateur racine de la selection\n\nEditeur de scripts\nenter: passage a la ligne\nctrl z: defaire/refaire (1 fois)\nctrl c ou shift et touche curseur simultanement: marque le debut de la selection, deplacer le curseur vers la fin puis Del pour effacer ou ctrl c pour copier sans effacer. ctrl v pour coller.\ndoc-6 recherche seule: entrer un mot puis enter puis enter. Taper enter pour l'occurence suivante, esc pour annuler.\ndoc-6 remplacer: entrer un mot puis enter puis le remplacement et enter. Taper enter ou esc pour remplacer ou non et passer a l'occurence suivante, ctrl del pour annuler\nvalidation (a droite de U): tester syntaxe\n\nRaccourcis Graphes:\n+ - zoom\n(-): zoomout selon y\n*: autoscale\n/: orthonormalisation\nOPTN: axes on/off";
  const char shortcuts_en_string[]="Keyboard shortcuts (shell and editor)\nbook: help or completion\ntab: completion (shell), indent (editor)\nshift-/: %\nalpha shift *: '\nctrl-/: \\\nshift-1 a 6: see at bottom\nshift-7: matrices\nshift-8: complexes\nshift-9:arithmetic\nshift-0: probas\nshift-.: reals\nctrl P: program\nvar: variables list\n ans (shift (-)): turtle screen (editor)\n\nctrl var (sto) returns =>\n=>+: partfrac\n=>*: factor\n=>sin/cos/tan\n=>=>: solve\n\nShell:\nshift-5: 2d editor or graph or text\nshift-4: text edit\n+ ou - modifies selected slider\n\nExpressions editor\nctrl z: undo/redo (1 fois)\nkeypad: move selection inside expression tree\nshift-right/left exchange selection with right or left argument\nalpha-right/left: inside a sum or product: increase selection with right or left argument\nshift-4: Edit selection, shift-5: change fontsize\nenter: eval selection\nshift-6: approx value\nDel: suppress selection's rootnode operator\n\nScript Editor\nenter: newline\nctrl z: undo/redo (1 time)\nctrl c or shift + cursor key simultaneously: marks selection begin, move the cursor to the end, then hit Del to erase or ctrl c to copy (no erase). ctrl v to paste.\ndoc-6 search: enter a word then enter then again enter. Type enter for next occurence, esc to cancel.\ndoc-6 replace: enter a word then enter then replacement word then enter. Type enter or esc to replace or ignore and go to next occurence, AC to cancel\nOK: test syntax\n\nGraph shortcuts:\n+ - zoom\n(-): zoomout along y\n*: autoscale\n/: orthonormalization\nOPTN: axes on/off";
#endif
  
  const char apropos_fr_string[]="Giac/Xcas 1.6.0, (c) 2020 B. Parisse et R. De Graeve, www-fourier.univ-grenoble-alpes.fr/~parisse.\nKhicas, interface pour calculatrices par B. Parisse, license GPL version 2, adaptee de l'interface d'Eigenmath pour Casio, G. Maia (http://gbl08ma.com), Mike Smith, Nemhardy, LePhenixNoir, ...\nPortage sur Numworks par Damien Nicolet. Remerciements a Jean-Baptiste Boric et Maxime Friess\nPortage sur Nspire grace a Fabian Vogt (firebird-emu, ndless...).\nTable periodique d'apres Maxime Friess\nRemerciements au site tiplanet, en particulier Xavier Andreani, Adrien Bertrand, Lionel Debroux";

  const char apropos_en_string[]="Giac/Xcas 1.6.0, (c) 2020 B. Parisse et R. De Graeve, www-fourier.univ-grenoble-alpes.fr/~parisse.\nKhicas, calculators interface by B. Parisse, GPL license version 2, adapted from Eigenmath for Casio, G. Maia (http://gbl08ma.com), Mike Smith, Nemhardy, LePhenixNoir, ...\nPorted on Numworks by Damien Nicolet. Thanks to Jean-Baptiste Boric and Maxime Friess\nPorted on Nspire thanks to Fabian Vogt (firebird-emu, ndless...)\nPeriodic table by Maxime Friess\nThanks to tiplanet, especially Xavier Andreani, Adrien Bertrand, Lionel Debroux";

  const int CAT_COMPLETE_COUNT_FR=sizeof(completeCatfr)/sizeof(catalogFunc);
  const int CAT_COMPLETE_COUNT_EN=sizeof(completeCaten)/sizeof(catalogFunc);

  std::string insert_string(int index){
    std::string s;
    const catalogFunc * completeCat=(lang==1)?completeCatfr:completeCaten;
    if (completeCat[index].insert)
      s=completeCat[index].insert;
    else {
      s=completeCat[index].name;
      int pos=s.find('(');
      if (pos>=0 && pos<s.size())
	s=s.substr(0,pos+1);
    }
    return s;//s+' ';
  }

  // not tested
  void aide2catalogFunc(const giac::aide & a,catalogFunc & c){
    static aide as=a;
    static string desc;
    string descrip;
    c.name=as.cmd_name.c_str();
    c.insert=c.name;
    desc=as.syntax+'\n';
    for (int i=0;i<as.blabla.size();++i){
      localized_string & ls=as.blabla[i];
      if (ls.language==lang){ // exact match
	descrip=as.blabla[i].chaine.c_str();
	break;
      }
      if (ls.language==0) // default
	descrip=as.blabla[i].chaine.c_str();
    }
    desc += descrip;
    c.desc=desc.c_str();
    c.example=as.examples.size()?as.examples[0].c_str():0;
    c.example2=as.examples.size()>=2?as.examples[1].c_str():0;
    c.category=-1;
  }
  int showCatalog(char* insertText,int preselect,int menupos,GIAC_CONTEXT) {
    // returns 0 on failure (user exit) and 1 on success (user chose a option)
    MenuItem menuitems[CAT_CATEGORY_LOGO+1];
    menuitems[CAT_CATEGORY_ALL].text = (char*)((lang==1)?"Tout":"All");
    menuitems[CAT_CATEGORY_ALGEBRA].text = (char*)((lang==1)?"Algebre":"Algebra");
    menuitems[CAT_CATEGORY_LINALG].text = (char*)((lang==1)?"Algebre lineaire":"Linear algebra");
    menuitems[CAT_CATEGORY_CALCULUS].text = (char*)((lang==1)?"Analyse":"Calculus");
    menuitems[CAT_CATEGORY_ARIT].text = (char*)"Arithmetic, crypto";
    menuitems[CAT_CATEGORY_COMPLEXNUM].text = (char*)"Complexes";
    menuitems[CAT_CATEGORY_PLOT].text = (char*)((lang==1)?"Courbes":"Curves");
    menuitems[CAT_CATEGORY_POLYNOMIAL].text = (char*)((lang==1)?"Polynomes":"Polynomials");
    menuitems[CAT_CATEGORY_PROBA].text = (char*)((lang==1)?"Probabilites":"Probabilities");
    menuitems[CAT_CATEGORY_PROGCMD].text = (char*)((lang==1)?"Programmes cmds (0)":"Program cmds (0)");
    menuitems[CAT_CATEGORY_REAL].text = (char*)((lang==1)?"Reels (e^)":"Reals");
    menuitems[CAT_CATEGORY_SOLVE].text = (char*)((lang==1)?"Resoudre (ln)":"Solve (ln)");
    menuitems[CAT_CATEGORY_STATS].text = (char*)((lang==1)?"Statistiques (log)":"Statistics (log)");
    menuitems[CAT_CATEGORY_TRIG].text = (char*)((lang==1)?"Trigonometrie (i)":"Trigonometry (i)");
    menuitems[CAT_CATEGORY_OPTIONS].text = (char*)"Options (,)";
    menuitems[CAT_CATEGORY_LIST].text = (char*)((lang==1)?"Listes (x^y)":"Lists (x^y)");
    menuitems[CAT_CATEGORY_MATRIX].text = (char*)"Matrices (sin)";
    menuitems[CAT_CATEGORY_PROG].text = (char*)((lang==1)?"Programmes (cos)":"Programs");
    menuitems[CAT_CATEGORY_SOFUS].text = (char*)((lang==1)?"Modifier variables (tan)":"Change variables (tan)");
    menuitems[CAT_CATEGORY_PHYS].text = (char*)((lang==1)?"Constantes physique (pi)":"Physics constants (pi)");
    menuitems[CAT_CATEGORY_UNIT].text = (char*)((lang==1)?"Unites physiques (sqrt)":"Units (sqrt)");
    menuitems[CAT_CATEGORY_2D].text = (char*)((lang==1)?"Geometrie (x^2)":"Geometry (x^2)");
    menuitems[CAT_CATEGORY_3D].text = (char*)((lang==1)?"3D (()":"3D (()");
    menuitems[CAT_CATEGORY_LOGO].text = (char*)((lang==1)?"Tortue ())":"Turtle ())");
  
    Menu menu;
    menu.items=menuitems;
    menu.numitems=sizeof(menuitems)/sizeof(MenuItem);
    menu.height=MENUHEIGHT;
    menu.scrollout=1;
    menu.title = (char*)((lang==1)?"Liste de commandes":"Commands list");
    //puts("catalog 1");
    while(1) {
      if (preselect)
        menu.selection=preselect;
      else {
        if (menupos>0)
          menu.selection=menupos;
        int sres = doMenu(&menu);
        if (sres != MENU_RETURN_SELECTION && sres!=KEY_CTRL_EXE)
          return 0;
      }
      // puts("catalog 3");
      if(doCatalogMenu(insertText, menuitems[menu.selection-1].text, menu.selection-1,contextptr)) 
        return 1;
      if (preselect)
        return 0;
    }
    return 0;
  }

  int showCatalog(char * text,int nmenu,GIAC_CONTEXT){
    return showCatalog(text,0,nmenu,contextptr);
  }

  bool isalphanum(char c){
    return (c>='a' && c<='z') || (c>='A' && c<='Z') || (c>='0' && c<='9');
  }

  string remove_accents(const string & s){
    string r;
    for (int i=0;i<s.size();++i){
      unsigned char ch=s[i];
      if (ch==195 && i<s.size()-1){
	++i;
	switch ((unsigned char)s[i]){
	case 160: case 161: case 162:
	  r+='a';
	  continue;
	case 168: case 169: case 170:
	  r+='e';
	  continue;
	case 172: case 173: case 174:
	  r+='i';
	  continue;
	case 178: case 179: case 180:
	  r += 'o';
	  continue;
	case 185: case 186: case 187:
	  r+='u';
	  continue;
	}
	r += '?';
	continue;
      }
      r+=ch;
    }
    return r;
  }

  // back is the number of char that should be deleted before inserting
  string help_insert(const char * cmdline,int & back,int exec,GIAC_CONTEXT,bool warn){
    if (exec==KEY_CTRL_OK)
      exec=MENU_RETURN_SELECTION;
    back=0;
    int l=strlen(cmdline);
    char buf[l+1];
    strcpy(buf,cmdline);
    bool openpar=l && buf[l-1]=='(';
    if (openpar){
      buf[l-1]=0;
      --l;
      ++back;
    }
    for (;l>0;--l){
      if (!isalphanum(buf[l-1]) && buf[l-1]!='_')
	break;
    }
    // cmdname in buf+l
    const char * cmdname=buf+l,*cmdnameorig=cmdname;
    l=strlen(cmdname);
    // search in catalog: dichotomy would be more efficient
    // but leading spaces cmdnames would be missed
    int nfunc=(lang==1)?CAT_COMPLETE_COUNT_FR:CAT_COMPLETE_COUNT_EN;//sizeof(completeCat)/sizeof(catalogFunc);
#if defined NSPIRE_NEWLIB || defined NUMWORKS // should match static_help[] in help.cc
    int iii=nfunc; // no search in completeCat, directly in static_help.h
    //if (xcas_python_eval) iii=0;
#else
    int iii=0;
#endif
    const catalogFunc * completeCat=(lang==1)?completeCatfr:completeCaten;
    for (;iii<nfunc;++iii){
      if (xcas_python_eval>0 && (completeCat[iii].category & XCAS_ONLY) )
	continue;
      const char * name=completeCat[iii].name;
      while (*name==' ')
	++name;
      int j=0;
      for (;j<l;++j){
	if (name[j]!=cmdname[j])
	  break;
      }
      if (j==l)
	break;
    }
    const catalogFunc * catf=iii==nfunc?0:completeCat+iii;
    const char * fhowto=0,* fsyntax=0,* frelated=0,* fexamples=0;
    string cf="";
    char fbuf[1024];
    if (iii==nfunc){
      if (!has_static_help(cmdname,exec?(lang==0?-2:-lang):lang,fhowto,fsyntax,fexamples,frelated)){
	if (warn) confirm("Pas d'aide disponible pour",cmdname,true);
	return "";
      }
      cf=frelated;
      if (!fexamples || fexamples[0]==0){
	fexamples=frelated;
	frelated=0;
      }
      // cut example at ; if there is one
      for (int i=0;i<sizeof(fbuf);++i){
	if (fexamples[i]==0)
	  break;
	if (i>0 && fexamples[i]==';' && fexamples[i-1]!=' '){
	  strcpy(fbuf,fexamples);
	  fbuf[i]=0;
	  fexamples=fbuf;
	  frelated=fbuf+i+1;
	  while (*frelated==' ')
	    ++frelated;
	  for (++i;i<sizeof(fbuf);++i){
	    if (fbuf[i]==0)
	      break;
	    if (fbuf[i]==';'){
	      fbuf[i]=0;
	      break;
	    }
	  }
	  break;
	}
      }
    }
    const char * example=catf?catf->example:fexamples;
    const char * example2=catf?catf->example2:frelated;
    if (exec){
      if (!fsyntax){
	cmdname=example;
	example=example2;
      }
    }
    else {
      xcas::textArea text;
      text.editable=false;
      text.clipline=-1;
      text.title = (char*)((lang==1)?"Aide sur la commande":"Help on command");
      text.allowF1=true;
      text.python=false;
      std::vector<xcas::textElement> & elem=text.elements;
      elem = std::vector<xcas::textElement> (example2?5:4);
      elem[0].s = catf?catf->name:cmdname;
      elem[0].newLine = 0;
      elem[1].lineSpacing = 0;
      if (fsyntax){
	elem[1].newLine = 1;
	elem[1].s=(lang==1?"Syntaxe: ":"Syntax: ")+elem[0].s+"("+(strlen(fsyntax)?fsyntax:"arg")+")";
      }
      else {
	elem[1].newLine = 0;
	elem[1].s=elem[0].s;
      }
      if (cf.size())
	elem[0].s += " (cf. "+cf+")";
      if (elem[0].s.size()<16)
	elem[0].s=string(16-elem[0].s.size()/2,' ')+elem[0].s;
      //elem[0].color = COLOR_BLUE;
      elem[2].newLine = 1;
      elem[2].lineSpacing = 1;
      elem[2].minimini=1;
      std::string autoexample;
      if (catf && catf->desc==0){
	// if (token==T_UNARY_OP || token==T_UNARY_OP_38)
	elem[2].s=elem[0].s+"(args)";
      }
      else {
#ifdef NUMWORKS
	elem[2].s = remove_accents(catf?catf->desc:fhowto);
#else
	elem[2].s = catf?catf->desc:fhowto;
#endif
      }
#ifdef NSPIRE_NEWLIB
      std::string ex("tab: ");
#else
      std::string ex("Ans: ");
#endif
      elem[3].newLine = 1;
      elem[3].lineSpacing = 0;
      //elem[2].minimini=1;
      if (example){
	if (example[0]=='#')
	  ex += example+1;
	else {
	  if (iii==nfunc)
	    ex += fexamples;
	  else {
	    ex += insert_string(iii);
	    ex += example;
	    ex += ")";
	  }
	}
	elem[3].s = ex;
	if (example2){
#ifdef NSPIRE_NEWLIB
	  string ex2="ret: ";
#else
	  string ex2="EXE: ";
#endif
	  if (example2[0]=='#')
	    ex2 += example2+1;
	  else {
	    if (iii==nfunc)
	      ex2 += example2;
	    else {
	      ex2 += insert_string(iii);
	      ex2 += example2;
	      ex2 += ")";
	    }
	  }
	  elem[4].newLine = 1;
	  // elem[3].lineSpacing = 0;
	  //elem[3].minimini=1;
	  elem[4].s=ex2;
	}
      }
      else {
	if (autoexample.size())
	  elem[3].s=ex+autoexample;
	else
	  elem.pop_back();
      }
      exec=doTextArea(&text,contextptr);
    }
    if (exec==KEY_SHUTDOWN)
      return "";
    if (exec==MENU_RETURN_SELECTION){
      while (*cmdname && *cmdname==*cmdnameorig){
	++cmdname; ++cmdnameorig;
      }
      return cmdname;
    }
    if (exec == KEY_CHAR_ANS || exec==KEY_BOOK || exec=='\t' || exec==KEY_CTRL_EXE) {
      reset_kbd();
      std::string s;
      const char * example=0;
      if (exec==KEY_CHAR_ANS || exec==KEY_BOOK || exec=='\t')
	example=catf?catf->example:fexamples;
      else
	example=catf?catf->example2:frelated;
      if (example){
	while (*example && *example==*cmdnameorig){
	  ++example; ++cmdnameorig;
	}
	while (*cmdnameorig){
	  ++back;
	  ++cmdnameorig;
	}
	if (example[0]=='#')
	  s=example+1;
	else {
	  s += example;
	  //if (catf && s[s.size()-1]!=')') s += ")";
	}
      }
      if (python_compat(contextptr)<0 || (python_compat(contextptr) & 4)){
	// replace := by =
	for (int i=1;i<s.size();++i){
	  if (s[i]=='=' && s[i-1]==':')
	    s.erase(s.begin()+i-1);
	}
      }
      return s;
    }
    return "";
  }

#if 0 // def NUMWORKS
#define MENUITEM_MALLOC
#endif

  // 0 on exit, 1 on success
  int doCatalogMenu(char* insertText, const char* title, int category,GIAC_CONTEXT) {
    const catalogFunc * completeCat=(lang==1)?completeCatfr:completeCaten;
    for (;;){
      int allcmds=builtin_lexer_functions_end()-builtin_lexer_functions_begin();
      int allopts=lexer_tab_int_values_end-lexer_tab_int_values_begin;
      bool isall=category==CAT_CATEGORY_ALL;
      bool isopt=category==CAT_CATEGORY_OPTIONS;
      const int CAT_COMPLETE_COUNT=((lang==1)?CAT_COMPLETE_COUNT_FR:CAT_COMPLETE_COUNT_EN);
      int nitems = isall? allcmds:(isopt?allopts:CAT_COMPLETE_COUNT);
#ifdef MENUITEM_MALLOC
      int memsize=sizeof(MenuItem)*nitems;
      *logptr(contextptr) << "malloc " << memsize << ' ' << (size_t) &memsize << '\n';
      MenuItem *menuitems=(MenuItem *) malloc(memsize);
      if (!menuitems)
        return 0;
#else
      MenuItem menuitems[nitems];
#endif
      int cur = 0,curmi = 0,i=0;
#ifdef MICROPY_LIB
      if (xcas_python_eval==1)
        micropy_ck_eval("1");
#endif
      gen g;
      while(cur<nitems) {
        menuitems[curmi].type = MENUITEM_NORMAL;
        menuitems[curmi].color = _BLACK;    
        if (isall || isopt) {
          const char * text=isall?(builtin_lexer_functions_begin()+cur)->first:(lexer_tab_int_values_begin+curmi)->keyword;
#ifdef MICROPY_LIB
          if (xcas_python_eval==1 && xcas::find_color(text,contextptr)!=3){
            ++cur;
            continue;
          }
#endif
          menuitems[curmi].text = (char*) text;
          menuitems[curmi].isfolder = allcmds; // assumes allcmds>allopts
          menuitems[curmi].token=isall?((builtin_lexer_functions_begin()+curmi)->second.subtype+256):((lexer_tab_int_values_begin+curmi)->subtype+(lexer_tab_int_values_begin+curmi)->return_value*256);
          // menuitems[curmi].token=isall?find_or_make_symbol(text,g,0,false,contextptr):((lexer_tab_int_values_begin+curmi)->subtype+(lexer_tab_int_values_begin+curmi)->return_value*256);
          for (;i<CAT_COMPLETE_COUNT;++i){
            const char * catname=completeCat[i].name;
            int tmp=strcmp(catname,text);
            if (tmp>=0){
              size_t st=strlen(text),j=tmp?0:st;
              for (;j<st;++j){
                if (catname[j]!=text[j])
                  break;
              }
              if (j==st && (!isalphanum(catname[j]))){
                menuitems[curmi].isfolder = i;
                ++i;
              }
              break;
            }
          }
          // compare text with completeCat
          ++curmi;
        }
        else {
          int cat=completeCat[cur].category;
          if (
              (xcas_python_eval==0 || !(cat & XCAS_ONLY) ) &&
              ((cat & 0xff) == category ||
               (cat & 0xff00) == (category<<8) ||
               (cat & 0xff0000) == (category <<16) )
              ){
            menuitems[curmi].isfolder = cur; // little hack: store index of the command in the full list in the isfolder property (unused by the menu system in this case)
            menuitems[curmi].text = (char *) completeCat[cur].name;
            curmi++;
          }
        }
        cur++;
      }
      
      Menu menu;
      menu.items=menuitems;
      menu.numitems=curmi;
      if (isopt){ menu.selection=5; menu.scroll=4; }
      if (curmi>=100)
        lock_alpha(); //SetSetupSetting( (unsigned int)0x14, 0x88);	
      // DisplayStatusArea();
      menu.scrollout=1;
      menu.title = (char *) title;
      menu.type = MENUTYPE_FKEYS;
      menu.height = MENUHEIGHT-1;
      while(1) {
#ifdef HP39
	drawRectangle(0,114,LCD_WIDTH_PX,14,giac::_WHITE);
	PrintMini(0,114,"input | ex1 | ex2 |     |     | help  ",4);
#else
	drawRectangle(0,200,LCD_WIDTH_PX,22,giac::_WHITE);
#ifdef NSPIRE_NEWLIB
	PrintMini(0,200,(category==CAT_CATEGORY_ALL?"menu: help | ret: ex1 | tab: ex2":"menu: help | ret ex1 | tab ex2"),4,33333,giac::_WHITE);
#else
	PrintMini(0,200,(category==CAT_CATEGORY_ALL?"Toolbox help | Ans ex1 | EXE  ex2":"Toolbox help | EXE ex1 | Ans ex2"),4,33333,giac::_WHITE);
#endif
#endif
	int sres = 0;
	if (curmi==0){
	  do_confirm(lang==1?"Commandes seulement en mode Xcas":"Commands only in Xcas mode");
	  sres=MENU_RETURN_EXIT;
	}
	else
	  sres=doMenu(&menu);
	if (sres==KEY_CTRL_F4 && category!=CAT_CATEGORY_ALL){
	  break;
	}
	if(sres == MENU_RETURN_EXIT){
	  reset_kbd();
#ifdef MENUITEM_MALLOC
	  free(menuitems);
#endif
	  return sres;
	}
	int index=menuitems[menu.selection-1].isfolder;
	if(sres == KEY_CTRL_CATALOG || sres==KEY_BOOK || sres==KEY_CTRL_F6) {
	  const char * example=index<allcmds?completeCat[index].example:0;
	  const char * example2=index<allcmds?completeCat[index].example2:0;
	  xcas::textArea text;
	  text.editable=false;
	  text.clipline=-1;
	  text.title = (char*)((lang==1)?"Aide sur la commande":"Help on command");
	  text.allowF1=true;
	  text.python=python_compat(contextptr);
	  std::vector<xcas::textElement> & elem=text.elements;
	  elem = std::vector<xcas::textElement> (example2?4:3);
	  elem[0].s = index<allcmds?completeCat[index].name:menuitems[menu.selection-1].text;
	  if (index<allcmds && (completeCat[index].category & XCAS_ONLY) )
	    elem[0].s += lang==1?" (Xcas seulement)":" (Xcas only)";
	  elem[0].newLine = 0;
	  //elem[0].color = COLOR_BLUE;
	  elem[1].newLine = 1;
	  elem[1].lineSpacing = 1;
	  elem[1].minimini=1;
	  std::string autoexample;
	  if (index<allcmds)
	    elem[1].s = completeCat[index].desc;
	  else {
	    int token=menuitems[menu.selection-1].token;
	    elem[1].s="Desole, pas d'aide disponible...";
	    const char *fcmdname=menuitems[menu.selection-1].text,* fhowto=0,*fsyntax=0,*fexamples=0,*frelated=0;
	    if (has_static_help(fcmdname,lang,fhowto,fsyntax,fexamples,frelated)){
	      elem[1].s=fhowto;
	      example=fexamples;
	    }
	    else {
	      // *logptr(contextptr) << token << endl;
	      if (isopt){
          if (token==_INT_PLOT+T_NUMBER*256){
            autoexample="display="+elem[0].s;
            elem[1].s ="Option d'affichage: "+ autoexample;
          }
          if (token==_INT_COLOR+T_NUMBER*256){
            autoexample="display="+elem[0].s;
            elem[1].s="Option de couleur: "+ autoexample;
          }
          if (token==_INT_SOLVER+T_NUMBER*256){
            autoexample=elem[0].s;
            elem[1].s="Option de fsolve: " + autoexample;
          }
          if (token==_INT_TYPE+T_TYPE_ID*256){
            autoexample=elem[0].s;
            elem[1].s="Type d'objet: " + autoexample;
          }
	      }
	      if (isall){
          if (token==T_UNARY_OP || token==T_UNARY_OP_38)
            elem[1].s=elem[0].s+"(args)";
	      }
	    }
	  }
#ifdef NSPIRE_NEWLIB
	  std::string ex("ret: ");
#else
	  std::string ex("EXE: ");
#endif
	  elem[2].newLine = 1;
	  elem[2].lineSpacing = 0;
	  //elem[2].minimini=1;
	  if (example){
	    if (example[0]=='#')
	      ex += example+1;
	    else {
	      if (index<allcmds){
          ex += insert_string(index);
          ex += example;
          ex += ")";
	      }
	      else ex+=example;
	    }
	    elem[2].s = ex;
	    if (example2){
#ifdef NSPIRE_NEWLIB
	      string ex2="tab: ";
#else
	      string ex2="Ans: ";
#endif
	      if (example2[0]=='#')
          ex2 += example2+1;
	      else {
          if (index<allcmds){
            ex2 += insert_string(index);
            ex2 += example2;
            ex2 += ")";
          }
          else
            ex2 += example2;
	      }
	      elem[3].newLine = 1;
	      // elem[3].lineSpacing = 0;
	      //elem[3].minimini=1;
	      elem[3].s=ex2;
	    }
	  }
	  else {
	    if (autoexample.size())
	      elem[2].s=ex+autoexample;
	    else
	      elem.pop_back();
	  }
	  sres=doTextArea(&text,contextptr);
	}
	if (sres == KEY_CHAR_ANS || sres=='\t' ||sres==KEY_BOOK || sres==KEY_CTRL_EXE || sres==KEY_CTRL_F2 || sres==KEY_CTRL_F3) {
	  reset_kbd();
	  const char * example=0;
	  std::string s;
	  if (index<allcmds ){
	    s=insert_string(index);
	    if (sres==KEY_CHAR_ANS || sres=='\t' || sres==KEY_BOOK || sres==KEY_CTRL_F3)
	      example=completeCat[index].example2;
	    else
	      example=completeCat[index].example;
	  }
	  else {
	    const char *fcmdname=menuitems[menu.selection-1].text,* fhowto=0,*fsyntax=0,*fexamples=0,*frelated=0;
	    if (has_static_help(fcmdname,lang,fhowto,fsyntax,fexamples,frelated)){
	      example=fexamples;
	    }
	  }
	  if (example){
	    if (example[0]=='#')
	      s=example+1;
	    else {
	      s += example;
	      if (s[s.size()-1]!=')')
          s += ")";
	    }
	    strcpy(insertText, s.c_str());
#ifdef MENUITEM_MALLOC
	    free(menuitems);
#endif
	    return 1;
	  }
	  else {
	    if (isopt){
	      int token=menuitems[menu.selection-1].token;
	      if (token==_INT_PLOT+T_NUMBER*256 || token==_INT_COLOR+T_NUMBER*256)
          strcpy(insertText,"display=");
	      else
          *insertText=0;
	      strcat(insertText,menuitems[menu.selection-1].text);
#ifdef MENUITEM_MALLOC
	      free(menuitems);
#endif
	      return 1;
	    }
	  }
	  sres=KEY_CTRL_OK;
	}
	if(sres == MENU_RETURN_SELECTION || sres == KEY_CTRL_OK || sres==KEY_CTRL_F1) {
	  reset_kbd();
	  strcpy(insertText,index<allcmds?insert_string(index).c_str():menuitems[menu.selection-1].text);
#ifdef MENUITEM_MALLOC
	  free(menuitems);
#endif
	  return 1;
	}
      }
      title="CATALOG";
      category=0;
    } // end endless for
    return 0; // never reached
  }

  int trialpha(const void *p1,const void * p2){
    int i=strcmp(* (char * const *) p1, * (char * const *) p2);
    return i;
  }

  // geo_print / geoprint
  std::string _pnt2string(const giac::gen & g,const giac::context * contextptr){
    unsigned ta=taille(g,100);
    if (ta>100)
      return "Done";
    if (g.is_symb_of_sommet(giac::at_pnt)){
      giac::gen & f=g._SYMBptr->feuille;
      giac::gen fp=remove_at_pnt(g);
      if (fp.is_symb_of_sommet(giac::at_hyperplan)){
	return gettext("plan")+string("(")+_equation(g,contextptr).print(contextptr)+string(")");
      }
      if (f.type==giac::_VECT && !f._VECTptr->empty()){
	giac::gen f0=f._VECTptr->front();
	if (f0.is_symb_of_sommet(giac::at_legende)){
	  return g.print(contextptr);
	}
	if (f0.is_symb_of_sommet(giac::at_curve)){
	  giac::gen f1=f[0]._SYMBptr->feuille;
	  if (f1.type==giac::_VECT && !f1._VECTptr->empty() ){
	    giac::gen f1f=f1._VECTptr->front();
	    if (f1f.type==giac::_VECT && f1f._VECTptr->size()>=4){
	      giac::vecteur f1v=*f1f._VECTptr;
	      return "plotparam("+_pnt2string(f1v[0],contextptr)+","+f1v[1].print(contextptr)+"="+f1v[2].print(contextptr)+".."+f1v[3].print(contextptr)+")";
	    }
	  }
	}
	if (f0.is_symb_of_sommet(giac::at_cercle) && f0._SYMBptr->feuille.type==giac::_VECT){
	  if (f0._SYMBptr->feuille._VECTptr->size()==3 && ((*f0._SYMBptr->feuille._VECTptr)[2]!=giac::cst_two_pi || (*f0._SYMBptr->feuille._VECTptr)[1]!=0))
	    return f0.print(contextptr);
	  giac::gen centre,rayon;
	  if (!giac::centre_rayon(f0,centre,rayon,true,0))
	    return "cercle_error";
	  if (!complex_mode(contextptr) && (centre.type<giac::_IDNT || centre.type==giac::_FRAC) )
	    return gettext("circle")+string("(point(")+giac::re(centre,contextptr).print(contextptr)+","+giac::im(centre,contextptr).print(contextptr)+"),"+rayon.print(contextptr)+")";
	  else
	    return gettext("circle")+string("(point(")+centre.print(contextptr)+"),"+rayon.print(contextptr)+")";
	}
	if (f0.type==giac::_VECT &&f0.subtype!=giac::_POINT__VECT){
	  std::string s=gettext("polygon")+string("(");
	  giac::const_iterateur it=f0._VECTptr->begin(),itend=f0._VECTptr->end();
	  if ( itend-it==2){ 
	    switch(f0.subtype){
	    case giac::_LINE__VECT:
	      s=gettext("line")+string("(");
	      break;
	    case giac::_HALFLINE__VECT:
	      s=gettext("half_line")+string("(");
	      break;
	    case giac::_GROUP__VECT:
	      s=gettext("segment")+string("(");
	      break;
	    }
	    if (f0.subtype==giac::_LINE__VECT && it->type!=giac::_VECT){ // 2-d line
	      s += _equation(g,contextptr).print(contextptr) + ")";
	      return s;
	    }
	  }
	  for (;it!=itend;){
	    s += "point(";
	    if (!complex_mode(contextptr) && (it->type<giac::_IDNT || it->type==giac::_FRAC) )
	      s += giac::re(*it,contextptr).print(contextptr)+","+giac::im(*it,contextptr).print(contextptr);
	    else {
	      gen f=*it;
	      if (f.type==_VECT && f.subtype==_POINT__VECT)
		f.subtype=_SEQ__VECT;
	      s += f.print(contextptr);
	    }
	    s+=")";
	    ++it;
	    s += it==itend?")":",";
	  }
	  return s;
	}
	if ( (f0.type!=giac::_FRAC && f0.type>=giac::_IDNT) || is3d(g) || complex_mode(contextptr)){
	  if (f0.type==_VECT && f0.subtype==_POINT__VECT)
	    f0.subtype=_SEQ__VECT;
	  return "point("+f0.print(contextptr)+")";
	}
	else
	  return "point("+giac::re(f0,contextptr).print(contextptr)+","+giac::im(f0,contextptr).print(contextptr)+")";
      }
    } 
    if (g.type==giac::_VECT && !g._VECTptr->empty() && g._VECTptr->back().is_symb_of_sommet(giac::at_pnt)){
      std::string s = "[";
      giac::const_iterateur it=g._VECTptr->begin(),itend=g._VECTptr->end();
      for (;it!=itend;){
	s += _pnt2string(*it,contextptr);
	++it;
	s += it==itend?"]":",";
      }
      return s;
    }
    return g.print(contextptr);
  }

  std::string pnt2string(const giac::gen & g,const giac::context * contextptr){
    int p=python_compat(contextptr);
    python_compat(0,contextptr);    
    string s=_pnt2string(g,contextptr);
    python_compat(p,contextptr);
    return s;
  }
  
  gen select_var(GIAC_CONTEXT){
    kbd_interrupted=giac::ctrl_c=giac::interrupted=false;
#ifdef QUICKJS
    if (xcas_python_eval<0){
      update_js_vars();
      size_t vs=js_vars.size();
      char s[vs+1];
      strcpy(s,js_vars.c_str());
      unsigned n=0,N=0;
      for (size_t i=0;i<vs;++i){
	if (s[i]==' ') 
	  ++N;
      }
      const char * tab[N+1];
      tab[0]=s;
      for (size_t i=0;i<js_vars.size();++i){
	if (s[i]==' '){
	  s[i]=0;
	  ++i; ++n;
	  tab[n]=s+i;
	}
      }
      tab[N]=0;
      qsort(tab,N,sizeof(char *),trialpha);
      int i=select_item(tab,"VARS",true);
      gen g=undef;
      if (i>=0 && tab[i])
	g=string2gen(tab[i],false);
      return g;
    }
#endif
#ifdef MICROPY_LIB
    if (xcas_python_eval==1){
      micropy_ck_eval("");
      const char ** tab=(const char **)mp_vars();
      const char **ptr=tab;
      for (;*ptr;)
	++ptr;
      // del at end should not be sorted
      if (ptr-tab>=1 && strcmp(*(ptr-1),"del ")==0)
	--ptr;
      qsort(tab,ptr-tab,sizeof(char *),trialpha);
      if (tab){
	int i=select_item(tab,"VARS",true);
	gen g=undef;
	if (i>=0 && tab[i])
	  g=string2gen(tab[i],false);
	free(tab);
	return g;
      }
    }
#endif
    gen g(_VARS(0,contextptr));
    if (g.type!=_VECT
	//|| g._VECTptr->empty()
	){
      confirm((lang==1)?"Pas de variables. Exemples pour en creer":"No variables. Examples to create",(lang==1)?"a=1 ou f(x):=sin(x^2)":"a=1 or f(x):=sin(x^2)",true);
      return undef;
    }
    vecteur & v=*g._VECTptr;
    MenuItem smallmenuitems[v.size()+3];
    vector<std::string> vs(v.size()+1);
    int i,total=0;
    const char typ[]="idzDcpiveSfEsFRmuMwgPF";
    for (i=0;i<v.size();++i){
      vs[i]=v[i].print(contextptr);
      if (v[i].type==giac::_IDNT){
	giac::gen w;
	v[i]._IDNTptr->in_eval(0,v[i],w,contextptr,true);
#if 1
	vector<int> vi(9);
	tailles(w,vi);
	total += vi[8];
	if (vi[8]<w.is_symb_of_sommet(at_pnt)?1500:500)
	  vs[i]+=":="+pnt2string(w,contextptr);
	else {
	  vs[i] += " ~";
	  vs[i] += giac::print_INT_(vi[8]);
	  vs[i] += ',';
	  vs[i] += typ[w.type];
	}
#else
	if (taille(w,50)<50)
	  vs[i]+=": "+w.print(contextptr);
#endif
      }
      smallmenuitems[i].text=(char *) vs[i].c_str();
    }
    total +=
      // giac::syms().capacity()*(sizeof(string)+sizeof(giac::gen)+8)+sizeof(giac::sym_string_tab) +
      giac::turtle_stack().capacity()*sizeof(giac::logo_turtle) +
      // sizeof(giac::context)+contextptr->tabptr->capacity()*(sizeof(const char *)+sizeof(giac::gen)+8)+
      bytesize(giac::history_in(contextptr))+bytesize(giac::history_out(contextptr));
    vs[i]="purge(~"+giac::print_INT_(total)+')';
    smallmenuitems[i].text=(char *)vs[i].c_str();
    smallmenuitems[i+1].text=(char *)"assume(";
    smallmenuitems[i+2].text=(char *)"restart";
    Menu smallmenu;
    smallmenu.numitems=v.size()+3; 
    smallmenu.items=smallmenuitems;
    smallmenu.height=MENUHEIGHT;
    smallmenu.scrollbar=1;
    smallmenu.scrollout=1;
    string vars="Variables";
#if defined NUMWORKS && defined DEVICE
    vars += ", free >= ";
    vars += print_INT_(_heap_size-((int)_heap_ptr-(int)_heap_base));
#endif
    smallmenu.title = (char*) vars.c_str();
    //MsgBoxPush(5);
    int sres = doMenu(&smallmenu);
    //MsgBoxPop();
    if (sres==KEY_CTRL_DEL && smallmenu.selection<=v.size())
      return symbolic(at_purge,v[smallmenu.selection-1]);
    if (sres!=MENU_RETURN_SELECTION && sres!=KEY_CTRL_EXE)
      return undef;
    if (smallmenu.selection==1+v.size())
      return string2gen("purge(",false);
    if (smallmenu.selection==2+v.size())
      return string2gen("assume(",false);
    if (smallmenu.selection==3+v.size())
      return string2gen("restart",false);
    return v[smallmenu.selection-1];
  }
  
  const char * keytostring(int key,int keyflag,bool py,const giac::context * contextptr){
    const int textsize=512;
    static char text[textsize];
    if (key>=0x20 && key<=0x7e){
      text[0]=key;
      text[1]=0;
      return text;
    }
    switch (key){
    case KEY_CHAR_PLUS:
      return "+";
    case KEY_CHAR_MINUS:
      return "-";
    case KEY_CHAR_PMINUS:
      return "_";
    case KEY_CHAR_MULT:
      return "*";
    case KEY_CHAR_FRAC:
      return py?"\\":"solve(";
    case KEY_CHAR_DIV: 
      return "/";
    case KEY_CHAR_POW:
      return "^";
    case KEY_CHAR_ROOT:
      return "sqrt(";
    case KEY_CHAR_SQUARE:
      return py?"**2":"^2";
    case KEY_CHAR_CUBEROOT:
      return py?"**(1/3)":"^(1/3)";
    case KEY_CHAR_POWROOT:
      return py?"**(1/":"^(1/";
    case KEY_CHAR_RECIP:
      return py?"**-1":"^-1";
    case KEY_CHAR_THETA:
      return "arg(";
    case KEY_CHAR_VALR:
      return "abs(";
    case KEY_CHAR_ANGLE:
      return "polar_complex(";
    case KEY_CTRL_XTT:
      return xthetat?"t":"x";
    case KEY_CHAR_LN:
      return "ln(";
    case KEY_CHAR_LOG:
      return "log10(";
    case KEY_CHAR_EXPN10:
      return "10^";
    case KEY_CHAR_EXPN:
      return "exp(";
    case KEY_CHAR_SIN:
      return "sin(";
    case KEY_CHAR_COS:
      return "cos(";
    case KEY_CHAR_TAN:
      return "tan(";
    case KEY_CHAR_ASIN:
      return "asin(";
    case KEY_CHAR_ACOS:
      return "acos(";
    case KEY_CHAR_ATAN:
      return "atan(";
    case KEY_CTRL_MIXEDFRAC:
      return "limit(";
    case KEY_CTRL_FRACCNVRT:
      return "exact(";
      // case KEY_CTRL_FORMAT: return "purge(";
    case KEY_CTRL_FD:
      return "approx(";
    case KEY_CHAR_STORE:
      // if (keyflag==1) return "inf";
      return "=>";
    case KEY_CHAR_IMGNRY:
      return "i";
    case KEY_CHAR_PI:
      return "pi";
    case KEY_CTRL_VARS: {
      giac::gen var=select_var(contextptr);
      if (!giac::is_undef(var)){
	strcpy(text,(var.type==giac::_STRNG?*var._STRNGptr:var.print(contextptr)).c_str());
	return text;
      }
      return "";//"VARS()";
    }
    case KEY_CHAR_EXP:
      return "e";
    case KEY_CHAR_ANS:
      return "ans()";
    case KEY_CHAR_CROCHETS:
      return "[]";
    case KEY_CHAR_ACCOLADES:
      return "{}";
    case KEY_CTRL_INS:
      return ":=";
    case KEY_CHAR_MAT:{
      const char * ptr=xcas::input_matrix(false,contextptr); if (ptr) return ptr;
      if (showCatalog(text,17,contextptr)) return text;
      return "";
    }
    case KEY_CHAR_LIST: {
      const char * ptr=xcas::input_matrix(true,contextptr); if (ptr) return ptr;
      if (showCatalog(text,16,contextptr)) return text;
      return "";
    }
    case KEY_CTRL_PRGM:
      // open functions catalog, prgm submenu
      if(showCatalog(text,18,contextptr))
	return text;
      return "";
    case KEY_CTRL_CATALOG: case KEY_BOOK:
      if(showCatalog(text,0,contextptr)) 
	return text;
      return "";
    case KEY_CTRL_F4:
      if(showCatalog(text,0,contextptr)) 
	return text;
      return "";
    case KEY_CTRL_OPTN:
      if(showCatalog(text,15,contextptr))
	return text;
      return "";
    case KEY_CTRL_QUIT: 
      if(showCatalog(text,20,contextptr))
	return text;
      return "";
    case KEY_CTRL_PASTE:
      return paste_clipboard();
    case KEY_CHAR_DQUATE:
      return "\"";
    case KEY_CHAR_FACTOR:
      return "factor(";
    case KEY_CHAR_NORMAL:
      return "normal(";
    }
    return 0;
  }
  
  const char * keytostring(int key,int keyflag,GIAC_CONTEXT){
    return keytostring(key,keyflag,python_compat(contextptr),contextptr);
  }
  
  bool stringtodouble(const string & s1,double & d){
    gen g(s1,context0);
    g=evalf(g,1,context0);
    if (g.type!=_DOUBLE_){
      confirm("Invalid value",s1.c_str());
      return false;
    }
    d=g._DOUBLE_val;
    return true;
  }

  bool inputdouble(const char * msg1,double & d,GIAC_CONTEXT){
    int di=d;
    string s1;
    if (di==d)
      s1=print_INT_(di);
    else
      s1=print_DOUBLE_(d,3);
    inputline(msg1,((lang==1)?"Nouvelle valeur? ":"New value? "),s1,false,65,contextptr);
    return stringtodouble(s1,d);
  }
  
  bool inputdouble(const char * msg1,double & d,int ypos,GIAC_CONTEXT){
    int di=d;
    string s1;
    if (di==d)
      s1=print_INT_(di);
    else
      s1=print_DOUBLE_(d,3);
    inputline(msg1,((lang==1)?"Nouvelle valeur? ":"New value? "),s1,false,ypos,contextptr);
    return stringtodouble(s1,d);
  }
  
  int inputline(const char * msg1,const char * msg2,string & s,bool numeric,int ypos,GIAC_CONTEXT){
    //s=msg2;
    int pos=s.size(),beg=0;
    for (;;){
      int X1=print_msg12(msg1,msg2,ypos-30);
      int textX=X1,textY=ypos;
      drawRectangle(textX,textY,LCD_WIDTH_PX-textX-4,18,COLOR_WHITE);
      if (pos-beg>36)
	beg=pos-12;
      if (int(s.size())-beg<36)
	beg=giac::giacmax(0,int(s.size())-36);
      if (beg>pos)
	beg=pos;
      textX=X1;
#if 0
      os_draw_string_(textX,textY,(s.substr(beg,pos-beg)+"|"+s.substr(pos,s.size()-pos)).c_str());
#else
      textX=os_draw_string_(textX,textY+2,s.substr(beg,pos-beg).c_str());
      os_draw_string_(textX,textY+2,s.substr(pos,s.size()-pos).c_str());
      drawRectangle(textX,textY+4,2,13,COLOR_BLACK); // cursor
      // PrintMini(0,58,"         |        |        |        |  A<>a  |       ",4);
#endif
      int key;
      GetKey(&key);
      if (key==KEY_SHUTDOWN)
	return key;      
      // if (!giac::freeze) set_xcas_status();    
      if (key==KEY_CTRL_EXE || key==KEY_CTRL_OK || key==KEY_CHAR_CR)
	return KEY_CTRL_EXE;
      if (key>=32 && key<128){
	if (!numeric || key=='-' || (key>='0' && key<='9')){
	  s.insert(s.begin()+pos,char(key));
	  ++pos;
	}
	continue;
      }
      if (key==KEY_CHAR_ACCOLADES || key==KEY_CHAR_CROCHETS){
	s.insert(s.begin()+pos,key==KEY_CHAR_ACCOLADES?'}':']');
	s.insert(s.begin()+pos,key==KEY_CHAR_ACCOLADES?'{':'[');
	++pos;
	continue;	
      }
      if (key==KEY_CTRL_DEL){
	if (pos){
	  s.erase(s.begin()+pos-1);
	  --pos;
	}
	continue;
      }
      if (key==KEY_CTRL_AC){
	if (s=="")
	  return KEY_CTRL_EXIT;
	s="";
	pos=0;
	continue;
      }
      if (key==KEY_CTRL_EXIT)
	return key;
      if (key==KEY_CTRL_RIGHT){
	if (pos<s.size())
	  ++pos;
	continue;
      }
      if (key==KEY_SHIFT_RIGHT){
	pos=s.size();
	continue;
      }
      if (key==KEY_CTRL_LEFT){
	if (pos)
	  --pos;
	continue;
      }
      if (key==KEY_SHIFT_LEFT){
	pos=0;
	continue;
      }
      if (const char * ans=keytostring(key,0,false,contextptr)){
	insert(s,pos,ans);
	pos+=strlen(ans);
	continue;
      }
    }
  }

  logo_turtle * turtleptr=0;
  
  logo_turtle & turtle(){
    if (!turtleptr)
      turtleptr=new logo_turtle;
    return * turtleptr;
  }

#ifdef NSPIRE_NEWLIB
  const int MAX_LOGO=8192; 
#else
  const int MAX_LOGO=368; // 512?
#endif

  std::vector<logo_turtle> & turtle_stack(){
    static std::vector<logo_turtle> * ans = 0;
    if (!ans){
      // initialize from python app storage
      ans=new std::vector<logo_turtle>(1,(*turtleptr));
     
    }
    return *ans;
  }

  logo_turtle vecteur2turtle(const vecteur & v){
    int s=int(v.size());
    if (s>=5 && v[0].type==_DOUBLE_ && v[1].type==_DOUBLE_ && v[2].type==_DOUBLE_ && v[3].type==_INT_ && v[4].type==_INT_ ){
      logo_turtle t;
      t.x=v[0]._DOUBLE_val;
      t.y=v[1]._DOUBLE_val;
      t.theta=v[2]._DOUBLE_val;
      int i=v[3].val;
      t.mark=(i%2)!=0;
      i=i >> 1;
      t.visible=(i%2)!=0;
      i=i >> 1;
      t.direct = (i%2)!=0;
      i=i >> 1;
      t.turtle_width = i & 0xff;
      i=i >> 8;
      t.color = i;
      t.radius = v[4].val;
      if (s>5 && v[5].type==_INT_)
	t.s=v[5].val;
      else
	t.s=-1;
      return t;
    }
#ifndef NO_STDEXCEPT
    setsizeerr(gettext("vecteur2turtle")); // FIXME
#endif
    return logo_turtle();
  }

  static int turtle_status(const logo_turtle & turtle){
    int status= (turtle.color << 11) | ( (turtle.turtle_width & 0xff) << 3) ;
    if (turtle.direct)
      status += 4;
    if (turtle.visible)
      status += 2;
    if (turtle.mark)
      status += 1;
    return status;
  }

#if defined NUMWORKS && defined DEVICE
  bool ck_turtle_size(){
    vector<logo_turtle> & v=turtle_stack();
    if (v.size()<v.capacity())
      return true;
    int l=2*v.size();
    if (1024+l*sizeof(logo_turtle)<_heap_size-((int)_heap_ptr-(int)_heap_base))
      return true;
    ctrl_c=interrupted=true;
    return false;
  }

#else
  int ck_turtle_size(){
      return 1;
  }
#endif

  bool set_turtle_state(const vecteur & v,GIAC_CONTEXT){
    if (v.size()>=2 && v[0].type==_DOUBLE_ && v[1].type==_DOUBLE_){
      vecteur w(v);
      int s=int(w.size());
      if (s==2)
	w.push_back(double((*turtleptr).theta));
      if (s<4)
	w.push_back(turtle_status((*turtleptr)));
      if (s<5)
	w.push_back(0);
      if (w[2].type==_DOUBLE_ && w[3].type==_INT_ && w[4].type==_INT_){
	(*turtleptr)=vecteur2turtle(w);
	if (!ck_turtle_size())
	  return false;
#ifdef TURTLETAB
	turtle_stack_push_back(*turtleptr);
#else
	turtle_stack().push_back((*turtleptr));
#endif
	return true;
      }
    }
    return false;
  }

  gen turtle2gen(const logo_turtle & turtle){
    return gen(makevecteur(turtle.x,turtle.y,double(turtle.theta),turtle_status(turtle),turtle.radius,turtle.s),_LOGO__VECT);
  }

  gen turtle_state(GIAC_CONTEXT){
    return turtle2gen((*turtleptr));
  }

  static gen update_turtle_state(bool clrstring,GIAC_CONTEXT){
#if defined NUMWORKS && defined DEVICE
    if (!ck_turtle_size()){
      if (ctrl_c || interrupted)
	return undef;
      ctrl_c=true; interrupted=true;
      return gensizeerr("Not enough memory");
    }
#else
#ifdef TURTLETAB
    if (turtle_stack_size>=MAX_LOGO)
      return gensizeerr("Not enough memory");
#else
    if (turtle_stack().size()>=MAX_LOGO){
      ctrl_c=true; interrupted=true;
      return gensizeerr("Not enough memory");
    }
#endif
#endif
    if (clrstring)
      (*turtleptr).s=-1;
    (*turtleptr).theta = (*turtleptr).theta - floor((*turtleptr).theta/360)*360;
    bool push=true;
    if (push){
#ifdef TURTLETAB
      turtle_stack_push_back((*turtleptr));
#else
      if (!turtle_stack().empty()){
	logo_turtle & t=turtle_stack().back();
	if (t.equal_except_nomark(*turtleptr)){
	  t.theta=turtleptr->theta;
	  t.mark=turtleptr->mark;
	  t.visible=turtleptr->visible;
	  t.color=turtleptr->color;
	  push=false;
	}
      }
      turtle_stack().push_back((*turtleptr));
#endif
    }
    gen res=turtle_state(contextptr);
#if defined EMCC || defined (EMCC2) // should directly interact with canvas
    return gen(turtlevect2vecteur(turtle_stack()),_LOGO__VECT);
#endif
    return res;
  }

  int turtle_speed=0;
  gen _speed(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    if (g.type==_VECT && g._VECTptr->empty())
      return turtle_speed;
    if (g.type!=_INT_)
      return gensizeerr(contextptr);
    int i=g.val;
    if (i<0) i=0;
    if (i>1000) i=1000;
    turtle_speed=i;
    return i;
  }  
  static const char _speed_s []="speed";
  static define_unary_function_eval2 (__speed,&_speed,_speed_s,&printastifunction);
  define_unary_function_ptr5( at_speed ,alias_at_speed,&__speed,0,T_LOGO);

  gen _avance(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    // logo instruction
    double i;
    if (g.type!=_INT_){
      if (g.type==_VECT)
	i=turtle_length;
      else {
	gen g1=evalf_double(g,1,contextptr);
	if (g1.type==_DOUBLE_)
	  i=g1._DOUBLE_val;
	else
	  return gensizeerr(contextptr);
      }
    }
    else
      i=g.val;
    (*turtleptr).x += i * std::cos((*turtleptr).theta*deg2rad_d);
    (*turtleptr).y += i * std::sin((*turtleptr).theta*deg2rad_d) ;
    (*turtleptr).radius = 0;
    return update_turtle_state(true,contextptr);
  }
  static const char _avance_s []="avance";
  static define_unary_function_eval2 (__avance,&_avance,_avance_s,&printastifunction);
  define_unary_function_ptr5( at_avance ,alias_at_avance,&__avance,0,T_LOGO);

  static const char _forward_s []="forward";
  static define_unary_function_eval (__forward,&_avance,_forward_s);
  define_unary_function_ptr5( at_forward ,alias_at_forward,&__forward,0,true);

  gen _recule(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    // logo instruction
    if (g.type==_VECT)
      return _avance(-turtle_length,contextptr);
    return _avance(-g,contextptr);
  }
  static const char _recule_s []="recule";
  static define_unary_function_eval2 (__recule,&_recule,_recule_s,&printastifunction);
  define_unary_function_ptr5( at_recule ,alias_at_recule,&__recule,0,T_LOGO);

  gen _towards(const gen & g,GIAC_CONTEXT){
    // logo instruction
    if (g.type!=_VECT || g._VECTptr->size()!=2)
      return gensizeerr(contextptr);
    gen z=g._VECTptr->front()-(*turtleptr).x+cst_i*(g._VECTptr->back()-(*turtleptr).y);
    int m=get_mode_set_radian(contextptr);
    z=arg(z,contextptr);
    angle_mode(m,contextptr);
    return 180/M_PI*z;
  }
  static const char _towards_s []="towards";
  static define_unary_function_eval2 (__towards,&_towards,_towards_s,&printastifunction);
  define_unary_function_ptr5( at_towards ,alias_at_towards,&__towards,0,T_LOGO);

  static const char _backward_s []="backward";
  static define_unary_function_eval (__backward,&_recule,_backward_s);
  define_unary_function_ptr5( at_backward ,alias_at_backward,&__backward,0,true);

  gen _position(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    // logo instruction
    if (g.type!=_VECT)
      return makevecteur((*turtleptr).x,(*turtleptr).y);
    // return turtle_state();
    vecteur v = *g._VECTptr;
    int s=int(v.size());
    if (!s)
      return makevecteur((*turtleptr).x,(*turtleptr).y);
    v[0]=evalf_double(v[0],1,contextptr);
    if (s>1)
      v[1]=evalf_double(v[1],1,contextptr);
    if (s>2)
      v[2]=evalf_double(v[2],1,contextptr); 
    if (set_turtle_state(v,contextptr))
      return update_turtle_state(true,contextptr);
    return zero;
  }
  static const char _position_s []="position";
  static define_unary_function_eval2 (__position,&_position,_position_s,&printastifunction);
  define_unary_function_ptr5( at_position ,alias_at_position,&__position,0,T_LOGO);

  gen _cap(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    // logo instruction
    gen gg=evalf_double(g,1,contextptr);
    if (gg.type!=_DOUBLE_)
      return double((*turtleptr).theta);
    (*turtleptr).theta=gg._DOUBLE_val;
    (*turtleptr).radius = 0;
    return update_turtle_state(true,contextptr);
  }
  static const char _cap_s []="cap";
  static define_unary_function_eval2 (__cap,&_cap,_cap_s,&printastifunction);
  define_unary_function_ptr5( at_cap ,alias_at_cap,&__cap,0,T_LOGO);

  static const char _heading_s []="heading";
  static define_unary_function_eval (__heading,&_cap,_heading_s);
  define_unary_function_ptr5( at_heading ,alias_at_heading,&__heading,0,true);


  gen _tourne_droite(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    // logo instruction
    if (g.type!=_INT_){
      if (g.type==_VECT)
	(*turtleptr).theta -= 90;
      else {
	gen g1=evalf_double(g,1,contextptr);
	if (g1.type==_DOUBLE_)
	  (*turtleptr).theta -= g1._DOUBLE_val;
	else
	  return gensizeerr(contextptr);
      }
    }
    else
      (*turtleptr).theta -= g.val;
    (*turtleptr).radius = 0;
    return update_turtle_state(true,contextptr);
  }
  static const char _tourne_droite_s []="tourne_droite";
  static define_unary_function_eval2 (__tourne_droite,&_tourne_droite,_tourne_droite_s,&printastifunction);
  define_unary_function_ptr5( at_tourne_droite ,alias_at_tourne_droite,&__tourne_droite,0,T_LOGO);

  gen _tourne_gauche(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    // logo instruction
    if (g.type==_VECT){
      (*turtleptr).theta += 90;
      (*turtleptr).radius = 0;
      return update_turtle_state(true,contextptr);
    }
    return _tourne_droite(-g,contextptr);
  }
  static const char _tourne_gauche_s []="tourne_gauche";
  static define_unary_function_eval2 (__tourne_gauche,&_tourne_gauche,_tourne_gauche_s,&printastifunction);
  define_unary_function_ptr5( at_tourne_gauche ,alias_at_tourne_gauche,&__tourne_gauche,0,T_LOGO);

  gen _leve_crayon(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    // logo instruction
    (*turtleptr).mark = false;
    (*turtleptr).radius = 0;
    return update_turtle_state(true,contextptr);
  }
  static const char _leve_crayon_s []="leve_crayon";
  static define_unary_function_eval2 (__leve_crayon,&_leve_crayon,_leve_crayon_s,&printastifunction);
  define_unary_function_ptr5( at_leve_crayon ,alias_at_leve_crayon,&__leve_crayon,0,T_LOGO);

  static const char _penup_s []="penup";
  static define_unary_function_eval (__penup,&_leve_crayon,_penup_s);
  define_unary_function_ptr5( at_penup ,alias_at_penup,&__penup,0,T_LOGO);

  gen _baisse_crayon(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    // logo instruction
    (*turtleptr).mark = true;
    (*turtleptr).radius = 0;
    return update_turtle_state(true,contextptr);
  }
  static const char _baisse_crayon_s []="baisse_crayon";
  static define_unary_function_eval2 (__baisse_crayon,&_baisse_crayon,_baisse_crayon_s,&printastifunction);
  define_unary_function_ptr5( at_baisse_crayon ,alias_at_baisse_crayon,&__baisse_crayon,0,T_LOGO);

  static const char _pendown_s []="pendown";
  static define_unary_function_eval (__pendown,&_baisse_crayon,_pendown_s);
  define_unary_function_ptr5( at_pendown ,alias_at_pendown,&__pendown,0,T_LOGO);

  vector<string> * ecrisptr=0;
  vector<string> & ecristab(){
    if (!ecrisptr)
      ecrisptr=new vector<string>;
    return * ecrisptr;
  }
  gen _ecris(const gen & g,GIAC_CONTEXT){    
    if ( g.type==_STRNG && g.subtype==-1) return  g;
#if 0 //def TURTLETAB
    return gensizeerr("String support does not work with static turtle table");
#endif
    // logo instruction
    (*turtleptr).radius=14;
    if (g.type==_VECT){ 
      vecteur & v =*g._VECTptr;
      int s=int(v.size());
      if (s==2 && v[1].type==_INT_){
	(*turtleptr).radius=absint(v[1].val);
	(*turtleptr).s=ecristab().size();
	ecristab().push_back(gen2string(v.front()));
	return update_turtle_state(false,contextptr);
      }
      if (s==4 && v[1].type==_INT_ && v[2].type==_INT_ && v[3].type==_INT_){
	logo_turtle t=(*turtleptr);
	_leve_crayon(0,contextptr);
	_position(makevecteur(v[2],v[3]),contextptr);
	(*turtleptr).radius=absint(v[1].val);
	(*turtleptr).s=ecristab().size();
	ecristab().push_back(gen2string(v.front()));
	update_turtle_state(false,contextptr);
	(*turtleptr)=t;
	return update_turtle_state(true,contextptr);
      }
    }
    (*turtleptr).s=ecristab().size();
    ecristab().push_back(gen2string(g));
    return update_turtle_state(false,contextptr);
  }
  static const char _ecris_s []="ecris";
  static define_unary_function_eval2 (__ecris,&_ecris,_ecris_s,&printastifunction);
  define_unary_function_ptr5( at_ecris ,alias_at_ecris,&__ecris,0,T_LOGO);

  gen _signe(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    // logo instruction
    return _ecris(makevecteur(g,20,10,10),contextptr);
  }
  static const char _signe_s []="signe";
  static define_unary_function_eval2 (__signe,&_signe,_signe_s,&printastifunction);
  define_unary_function_ptr5( at_signe ,alias_at_signe,&__signe,0,T_LOGO);

  gen _saute(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    _leve_crayon(0,contextptr);
    _avance(g,contextptr);
    return _baisse_crayon(0,contextptr);
  }
  static const char _saute_s []="saute";
  static define_unary_function_eval2 (__saute,&_saute,_saute_s,&printastifunction);
  define_unary_function_ptr5( at_saute ,alias_at_saute,&__saute,0,T_LOGO);

  static const char _jump_s []="jump";
  static define_unary_function_eval2 (__jump,&_saute,_jump_s,&printastifunction);
  define_unary_function_ptr5( at_jump ,alias_at_jump,&__jump,0,T_LOGO);

  gen _pas_de_cote(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    _leve_crayon(0,contextptr);
    _tourne_droite(-90,contextptr);
    _avance(g,contextptr);
    _tourne_droite(90,contextptr);
    return _baisse_crayon(0,contextptr);
  }
  static const char _pas_de_cote_s []="pas_de_cote";
  static define_unary_function_eval2 (__pas_de_cote,&_pas_de_cote,_pas_de_cote_s,&printastifunction);
  define_unary_function_ptr5( at_pas_de_cote ,alias_at_pas_de_cote,&__pas_de_cote,0,T_LOGO);

  static const char _skip_s []="skip";
  static define_unary_function_eval2 (__skip,&_pas_de_cote,_skip_s,&printastifunction);
  define_unary_function_ptr5( at_skip ,alias_at_skip,&__skip,0,T_LOGO);

  gen _cache_tortue(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    // logo instruction
    (*turtleptr).visible=false;
    (*turtleptr).radius = 0;
    return update_turtle_state(true,contextptr);
  }
  static const char _cache_tortue_s []="cache_tortue";
  static define_unary_function_eval2 (__cache_tortue,&_cache_tortue,_cache_tortue_s,&printastifunction);
  define_unary_function_ptr5( at_cache_tortue ,alias_at_cache_tortue,&__cache_tortue,0,T_LOGO);

  static const char _hideturtle_s []="hideturtle";
  static define_unary_function_eval (__hideturtle,&_cache_tortue,_hideturtle_s);
  define_unary_function_ptr5( at_hideturtle ,alias_at_hideturtle,&__hideturtle,0,true);

  gen _montre_tortue(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    // logo instruction
    (*turtleptr).visible=true;
    (*turtleptr).radius = 0;
    return update_turtle_state(true,contextptr);
  }
  static const char _montre_tortue_s []="montre_tortue";
  static define_unary_function_eval2 (__montre_tortue,&_montre_tortue,_montre_tortue_s,&printastifunction);
  define_unary_function_ptr5( at_montre_tortue ,alias_at_montre_tortue,&__montre_tortue,0,T_LOGO);

  static const char _showturtle_s []="showturtle";
  static define_unary_function_eval (__showturtle,&_montre_tortue,_showturtle_s);
  define_unary_function_ptr5( at_showturtle ,alias_at_showturtle,&__showturtle,0,true);


  gen _repete(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    if (g.type!=_VECT || g._VECTptr->size()<2)
      return gensizeerr(contextptr);
    // logo instruction
    vecteur v = *g._VECTptr;
    v[0]=eval(v[0],contextptr);
    if (v.front().type!=_INT_)
      return gentypeerr(contextptr);
    gen prog=vecteur(v.begin()+1,v.end());
    int i=absint(v.front().val);
    gen res;
    for (int j=0;j<i;++j){
      res=eval(prog,contextptr);
    }
    return res;
  }
  static const char _repete_s []="repete";
  static define_unary_function_eval_quoted (__repete,&_repete,_repete_s);
  define_unary_function_ptr5( at_repete ,alias_at_repete,&__repete,_QUOTE_ARGUMENTS,T_RETURN);

  gen _crayon(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    if (g.type==_STRNG) return _crayon(gen(*g._STRNGptr,contextptr),contextptr);
    // logo instruction
    if (g.type==_VECT && g._VECTptr->size()==3)
      return _crayon(_rgb(g,contextptr),contextptr);
    if (g.type!=_INT_){
      gen res=(*turtleptr).color;
      res.subtype=_INT_COLOR;
      return res;
    }
    if (g.val<0){
      if (g.val<-64)
	return (*turtleptr).turtle_width;
      (*turtleptr).turtle_width=-g.val;
    }
    else
      (*turtleptr).color=g.val;
    (*turtleptr).radius = 0;
    return update_turtle_state(true,contextptr);
  }
  static const char _crayon_s []="crayon";
  static define_unary_function_eval2 (__crayon,&_crayon,_crayon_s,&printastifunction);
  define_unary_function_ptr5( at_crayon ,alias_at_crayon,&__crayon,0,T_LOGO);

  static const char _pencolor_s []="pencolor";
  static define_unary_function_eval (__pencolor,&_crayon,_pencolor_s);
  define_unary_function_ptr5( at_pencolor ,alias_at_pencolor,&__pencolor,0,T_LOGO);

  gen _efface_logo(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    if (g.type==_INT_){
      _crayon(int(FL_WHITE),contextptr);
      _recule(g,contextptr);
      return _crayon(0,contextptr);
    }
    // logo instruction
    (*turtleptr) = logo_turtle();
#ifdef TURTLETAB
    turtle_stack_size=0;
#else
    turtle_stack().clear();
#endif
    ecristab().clear();
    if (g.type==_VECT && g._VECTptr->size()==2){
      vecteur v = *g._VECTptr;
      int s=int(v.size());
      v[0]=evalf_double(v[0],1,contextptr);
      if (s>1)
	v[1]=evalf_double(v[1],1,contextptr);
      (*turtleptr).mark = false; // leve_crayon
      (*turtleptr).radius = 0;
      update_turtle_state(true,contextptr);
      set_turtle_state(v,contextptr); // baisse_crayon
      update_turtle_state(true,contextptr);
      (*turtleptr).mark = true;
      (*turtleptr).radius = 0;
    }
    return update_turtle_state(true,contextptr);
  }
  static const char _efface_logo_s []="efface";
  static define_unary_function_eval2 (__efface_logo,&_efface_logo,_efface_logo_s,&printastifunction);
  define_unary_function_ptr5( at_efface_logo ,alias_at_efface_logo,&__efface_logo,0,T_LOGO);

  static const char _efface_s []="efface";
  static define_unary_function_eval2 (__efface,&_efface_logo,_efface_s,&printastifunction);
  define_unary_function_ptr5( at_efface ,alias_at_efface,&__efface,0,T_LOGO);

  static const char _reset_s []="reset";
  static define_unary_function_eval2 (__reset,&_efface_logo,_reset_s,&printastifunction);
  define_unary_function_ptr5( at_reset ,alias_at_reset,&__reset,0,T_LOGO);

  static const char _clearscreen_s []="clearscreen";
  static define_unary_function_eval2 (__clearscreen,&_efface_logo,_clearscreen_s,&printastifunction);
  define_unary_function_ptr5( at_clearscreen ,alias_at_clearscreen,&__clearscreen,0,T_LOGO);

  gen _debut_enregistrement(const gen &g,GIAC_CONTEXT){
    return undef;
  }
  static const char _debut_enregistrement_s []="debut_enregistrement";
  static define_unary_function_eval2 (__debut_enregistrement,&_debut_enregistrement,_debut_enregistrement_s,&printastifunction);
  define_unary_function_ptr5( at_debut_enregistrement ,alias_at_debut_enregistrement,&__debut_enregistrement,0,T_LOGO);

  static const char _fin_enregistrement_s []="fin_enregistrement";
  static define_unary_function_eval2 (__fin_enregistrement,&_debut_enregistrement,_fin_enregistrement_s,&printastifunction);
  define_unary_function_ptr5( at_fin_enregistrement ,alias_at_fin_enregistrement,&__fin_enregistrement,0,T_LOGO);

  static const char _turtle_stack_s []="turtle_stack";
  static define_unary_function_eval2 (__turtle_stack,&_debut_enregistrement,_turtle_stack_s,&printastifunction);
  define_unary_function_ptr5( at_turtle_stack ,alias_at_turtle_stack,&__turtle_stack,0,T_LOGO);

  gen _vers(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    // logo instruction
    if (g.type!=_VECT || g._VECTptr->size()!=2)
      return gensizeerr(contextptr);
    gen x=evalf_double(g._VECTptr->front(),1,contextptr),
      y=evalf_double(g._VECTptr->back(),1,contextptr);
    if (x.type!=_DOUBLE_ || y.type!=_DOUBLE_)
      return gensizeerr(contextptr);
    double xv=x._DOUBLE_val,yv=y._DOUBLE_val,xt=(*turtleptr).x,yt=(*turtleptr).y;
    double theta=atan2(yv-yt,xv-xt);
    return _cap(theta*180/M_PI,contextptr);
  }
  static const char _vers_s []="vers";
  static define_unary_function_eval2 (__vers,&_vers,_vers_s,&printastifunction);
  define_unary_function_ptr5( at_vers ,alias_at_vers,&__vers,0,T_LOGO);

  static int find_radius(const gen & g,int & r,int & theta2,bool & direct){
    int radius;
    direct=true;
    theta2 = 360 ;
    // logo instruction
    if (g.type==_VECT && !g._VECTptr->empty()){
      vecteur v = *g._VECTptr;
      bool seg=false;
      if (v.back()==at_segment){
	v.pop_back();
	seg=true;
      }
      if (v.size()<2)
	return RAND_MAX; // setdimerr(contextptr);
      if (v[0].type==_INT_)
	r=v[0].val;
      else {
	gen v0=evalf_double(v[0],1,context0);
	if (v0.type==_DOUBLE_)
	  r=int(v0._DOUBLE_val+0.5);
	else 
	  return RAND_MAX; // setsizeerr(contextptr);
      }
      if (r<0){
	r=-r;
	direct=false;
      }
      int theta1;
      if (v[1].type==_DOUBLE_)
	theta1=int(v[1]._DOUBLE_val+0.5);
      else { 
	if (v[1].type==_INT_)
	  theta1=v[1].val;
	else return RAND_MAX; // setsizeerr(contextptr);
      }
      while (theta1<0)
	theta1 += 360;
      if (v.size()>=3){
	if (v[2].type==_DOUBLE_)
	  theta2 = int(v[2]._DOUBLE_val+0.5);
	else {
	  if (v[2].type==_INT_)
	    theta2 = v[2].val;
	  else return RAND_MAX; // setsizeerr(contextptr);
	}
	while (theta2<0)
	  theta2 += 360;
	radius = giacmin(r,512) | (giacmin(theta1,360) << 9) | (giacmin(theta2,360) << 18 ) | (seg?(1<<28):0);
      }
      else {// angle 1=0
	theta2 = theta1;
	if (theta2<0)
	  theta2 += 360;
	radius = giacmin(r,512) | (giacmin(theta2,360) << 18 ) | (seg?(1<<28):0);
      }
      return radius;
    }
    radius = 10;
    if (g.type==_INT_)
      radius= (r=g.val);
    if (g.type==_DOUBLE_)
      radius= (r=int(g._DOUBLE_val));
    if (radius<=0){
      radius = -radius;
      direct=false;
    }
    radius = giacmin(radius,512 )+(360 << 18) ; // 2nd angle = 360 degrees
    return radius;
  }

  static void c_turtle_move(int r,int theta2){
    double theta0;
    if ((*turtleptr).direct)
      theta0=(*turtleptr).theta-90;
    else {
      theta0=(*turtleptr).theta+90;
      theta2=-theta2;
    }
    (*turtleptr).x += r*(std::cos(M_PI/180*(theta2+theta0))-std::cos(M_PI/180*theta0));
    (*turtleptr).y += r*(std::sin(M_PI/180*(theta2+theta0))-std::sin(M_PI/180*theta0));
    (*turtleptr).theta = (*turtleptr).theta+theta2 ;
    if ((*turtleptr).theta<0)
      (*turtleptr).theta += 360;
    if ((*turtleptr).theta>360)
      (*turtleptr).theta -= 360;
  }

  static void turtle_move(int r,int theta2,GIAC_CONTEXT){
    c_turtle_move(r,theta2);
  }
  gen _rond(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    int r,theta2,tmpr;
    tmpr=find_radius(g,r,theta2,(*turtleptr).direct);
    if (tmpr==RAND_MAX)
      return gensizeerr(contextptr);
    (*turtleptr).radius=tmpr;
    turtle_move(r,theta2,contextptr);
    return update_turtle_state(true,contextptr);
  }
  static const char _rond_s []="rond";
  static define_unary_function_eval2 (__rond,&_rond,_rond_s,&printastifunction);
  define_unary_function_ptr5( at_rond ,alias_at_rond,&__rond,0,T_LOGO);

  gen _disque(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    int r,theta2,tmpr=find_radius(g,r,theta2,(*turtleptr).direct);
    if (tmpr==RAND_MAX)
      return gensizeerr(contextptr);
    (*turtleptr).radius=tmpr;
    turtle_move(r,theta2,contextptr);
    (*turtleptr).radius += 1 << 27;
    return update_turtle_state(true,contextptr);
  }
  static const char _disque_s []="disque";
  static define_unary_function_eval2 (__disque,&_disque,_disque_s,&printastifunction);
  define_unary_function_ptr5( at_disque ,alias_at_disque,&__disque,0,T_LOGO);

  gen _disque_centre(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    int r,theta2;
    bool direct;
    int radius=find_radius(g,r,theta2,direct);
    if (radius==RAND_MAX)
      return gensizeerr(contextptr);
    r=absint(r);
    _saute(r,contextptr);
    _tourne_gauche(direct?90:-90,contextptr);
    (*turtleptr).radius = radius;
    (*turtleptr).direct=direct;
    turtle_move(r,theta2,contextptr);
    (*turtleptr).radius += 1 << 27;
    update_turtle_state(true,contextptr);
    _tourne_droite(direct?90:-90,contextptr);
    return _saute(-r,contextptr);
  }
  static const char _disque_centre_s []="disque_centre";
  static define_unary_function_eval2 (__disque_centre,&_disque_centre,_disque_centre_s,&printastifunction);
  define_unary_function_ptr5( at_disque_centre ,alias_at_disque_centre,&__disque_centre,0,T_LOGO);

  gen _polygone_rempli(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    static int turtle_fill_begin=-1,turtle_fill_color=-1;
    if (g.type==_VECT && g._VECTptr->size()==3){
      turtle_fill_color=_rgb(g,contextptr).val;
      return change_subtype(turtle_fill_color,_INT_COLOR);
    }
    if (g.type==_VECT && !g._VECTptr->empty() && g._VECTptr->front().type==_INT_){
      if (g._VECTptr->front().val>=0)
	turtle_fill_color= g._VECTptr->front().val;
      return change_subtype(turtle_fill_color,_INT_COLOR);
    }
    if (g.type==_INT_ 
	//&& g.subtype==_INT_COLOR
	){
      if (g.val<-1 && g.val>-1024){
	(*turtleptr).radius=-absint(g.val);
	if ((*turtleptr).radius<-1)
	  return update_turtle_state(true,contextptr);
      }
      turtle_fill_color= g.val;
      return g;
    }
    if (g.type!=_VECT && is_zero(g)){ // 0.0
      turtle_fill_begin=turtle_stack().size();
      return 1;
    }
    if (g.type==_VECT && g._VECTptr->empty()){
      if (turtle_fill_begin<0){
	if (g.subtype==0)
	  turtle_fill_begin=turtle_stack().size();
	else
	  return gensizeerr();
	return 1;
      }
      int c=turtleptr->color;
      if (turtle_fill_color>=0)
	_crayon(turtle_fill_color,contextptr);
      int n=turtle_stack().size()- turtle_fill_begin;
      turtle_fill_begin=-1;
      turtleptr->radius=-absint(n);
      gen res=update_turtle_state(true,contextptr);
      if (turtle_fill_color>=0){
	turtleptr->radius=0;
	_crayon(c,contextptr);
      }
      return res;
    }
    return gensizeerr(gettext("Integer argument >= 2"));
  }
  static const char _polygone_rempli_s []="polygone_rempli";
  static define_unary_function_eval2 (__polygone_rempli,&_polygone_rempli,_polygone_rempli_s,&printastifunction);
  define_unary_function_ptr5( at_polygone_rempli ,alias_at_polygone_rempli,&__polygone_rempli,0,T_LOGO);

  gen _rectangle_plein(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    gen gx=g,gy=g;
    if (g.type==_VECT && g._VECTptr->size()==2){
      gx=g._VECTptr->front();
      gy=g._VECTptr->back();
    }
    for (int i=0;i<2;++i){
      _avance(gx,contextptr);
      _tourne_droite(-90,contextptr);
      _avance(gy,contextptr);
      _tourne_droite(-90,contextptr);
    }
    //for (int i=0;i<turtle_stack().size();++i){ *logptr(contextptr) << turtle2gen(turtle_stack()[i]) <<endl;}
    return _polygone_rempli(-8,contextptr);
  }
  static const char _rectangle_plein_s []="rectangle_plein";
  static define_unary_function_eval2 (__rectangle_plein,&_rectangle_plein,_rectangle_plein_s,&printastifunction);
  define_unary_function_ptr5( at_rectangle_plein ,alias_at_rectangle_plein,&__rectangle_plein,0,T_LOGO);

  gen _triangle_plein(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    gen gx=g,gy=g,gtheta=60;
    if (g.type==_VECT && g._VECTptr->size()>=2){
      vecteur & v=*g._VECTptr;
      gx=v.front();
      gy=v[1];
      gtheta=90;
      if (v.size()>2)
	gtheta=v[2];
    }
    logo_turtle t=(*turtleptr);
    _avance(gx,contextptr);
    double save_x=(*turtleptr).x,save_y=(*turtleptr).y;
    _recule(gx,contextptr);
    _tourne_gauche(gtheta,contextptr);
    _avance(gy,contextptr);
    (*turtleptr).x=save_x;
    (*turtleptr).y=save_y;
    update_turtle_state(true,contextptr);
    (*turtleptr)=t;
    (*turtleptr).radius=0;
    update_turtle_state(true,contextptr);
    return _polygone_rempli(-3,contextptr);
  }
  static const char _triangle_plein_s []="triangle_plein";
  static define_unary_function_eval2 (__triangle_plein,&_triangle_plein,_triangle_plein_s,&printastifunction);
  define_unary_function_ptr5( at_triangle_plein ,alias_at_triangle_plein,&__triangle_plein,0,T_LOGO);

  gen _dessine_tortue(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    // logo instruction
    /*
      _triangle_plein(makevecteur(17,5));
      _tourne_droite(90);
      _triangle_plein(makevecteur(5,17));
      return _tourne_droite(-90);
    */
    double save_x=(*turtleptr).x,save_y=(*turtleptr).y;
    _tourne_droite(90,contextptr);
    _avance(5,contextptr);
    _tourne_gauche(106,contextptr);
    _avance(18,contextptr);
    _tourne_gauche(148,contextptr);
    _avance(18,contextptr);
    _tourne_gauche(106,contextptr);
    _avance(5,contextptr);
    (*turtleptr).x=save_x; (*turtleptr).y=save_y;
    gen res(_tourne_gauche(90,contextptr));
    if (is_one(g))
      return res;
    return _polygone_rempli(-9,contextptr);
  }
  static const char _dessine_tortue_s []="dessine_tortue";
  static define_unary_function_eval2 (__dessine_tortue,&_dessine_tortue,_dessine_tortue_s,&printastifunction);
  define_unary_function_ptr5( at_dessine_tortue ,alias_at_dessine_tortue,&__dessine_tortue,0,T_LOGO);
  
#ifndef NO_NAMESPACE_GIAC
} // namespace giac
#endif // ndef NO_NAMESPACE_GIAC


#ifndef NO_NAMESPACE_XCAS
namespace xcas {
#endif // ndef NO_NAMESPACE_XCAS
  void drawRectangle(int x,int y,int w,int h,int c){
    draw_rectangle(x,y,w,h,c,context0);
  }
  void draw_rectangle(int x,int y,int w,int h,int c){
    draw_rectangle(x,y,w,h,c,context0);
  }
  void draw_line(int x0,int y0,int x1,int y1,int c){
#ifdef HP39
	draw_line(x0,y0,x1,y1,c,context0);
#else    
    if (x0==x1){
      if (y0<=y1)
	draw_rectangle(x0,y0,1,y1-y0+1,c);
      else
	draw_rectangle(x0,y1,1,y0-y1+1,c);
    }
    else {
      if (y0==y1){
	if (x0<=x1)
	  draw_rectangle(x0,y0,x1-x0+1,1,c);
	else
	  draw_rectangle(x1,y0,x0-x1+1,1,c);
      }
      else
	draw_line(x0,y0,x1,y1,c,context0);
    }
#endif
  }
  void draw_circle(int xc,int yc,int r,int color,bool q1,bool q2,bool q3,bool q4){
    draw_circle(xc,yc,r,color,q1,q2,q3,q4,context0);
  }
  void draw_filled_circle(int xc,int yc,int r,int color,bool left,bool right){
    draw_filled_circle(xc,yc,r,color,left,right,context0);
  }
  void draw_polygon(std::vector< std::vector<int> > & v1,int color){
    draw_polygon(v1,color,context0);
  }
  void draw_filled_polygon(std::vector< vector<int> > &L,int xmin,int xmax,int ymin,int ymax,int color){
    draw_filled_polygon(L,xmin,xmax,ymin,ymax,color,context0);
  }
  void draw_arc(int xc,int yc,int rx,int ry,int color,double theta1, double theta2){
    draw_arc(xc,yc,rx,ry,color,theta1,theta2,giac::context0);
  }
  void draw_filled_arc(int x,int y,int rx,int ry,int theta1_deg,int theta2_deg,int color,int xmin,int xmax,int ymin,int ymax,bool segment){
    draw_filled_arc(x,y,rx,ry,theta1_deg,theta2_deg,color,xmin,xmax,ymin,ymax,segment,context0);
  }


  unsigned max_prettyprint_equation=256;

  // make a free copy of g
  gen Equation_copy(const gen & g){
    if (g.type==_EQW)
      return *g._EQWptr;
    if (g.type!=_VECT)
      return g;
    vecteur & v = *g._VECTptr;
    const_iterateur it=v.begin(),itend=v.end();
    vecteur res;
    res.reserve(itend-it);
    for (;it!=itend;++it)
      res.push_back(Equation_copy(*it));
    return gen(res,g.subtype);
  }

  // matrix/list select
  bool do_select(gen & eql,bool select,gen & value){
    if (eql.type==_VECT && !eql._VECTptr->empty()){
      vecteur & v=*eql._VECTptr;
      size_t s=v.size();
      if (v[s-1].type!=_EQW)
	return false;
      v[s-1]._EQWptr->selected=select;
      gen sommet=v[s-1]._EQWptr->g;
      --s;
      vecteur args(s);
      for (size_t i=0;i<s;++i){
	if (!do_select(v[i],select,args[i]))
	  return false;
	if (args[i].type==_EQW)
	  args[i]=args[i]._EQWptr->g;
      }
      gen va=s==1?args[0]:gen(args,_SEQ__VECT);
      if (sommet.type==_FUNC)
	va=symbolic(*sommet._FUNCptr,va);
      else
	va=sommet(va,context0);
      //cout << "va " << va << endl;
      value=*v[s]._EQWptr;
      value._EQWptr->g=va;
      //cout << "value " << value << endl;
      return true;
    }
    if (eql.type!=_EQW)
      return false;
    eql._EQWptr->selected=select;
    value=eql;
    return true;
  }
  
  bool Equation_box_sizes(const gen & g,int & l,int & h,int & x,int & y,attributs & attr,bool & selected){
    if (g.type==_EQW){
      eqwdata & w=*g._EQWptr;
      x=w.x;
      y=w.y;
      l=w.dx;
      h=w.dy;
      selected=w.selected;
      attr=w.eqw_attributs;
      //cout << g << endl;
      return true;
    }
    else {
      if (g.type!=_VECT || g._VECTptr->empty() ){
	l=0;
	h=0;
	x=0;
	y=0;
	attr=attributs(0,0,0);
	selected=false;
	return true;
      }
      gen & g1=g._VECTptr->back();
      Equation_box_sizes(g1,l,h,x,y,attr,selected);
      return false;
    }
  }

  // return true if g has some selection inside, gsel points to the selection
  bool Equation_adjust_xy(gen & g,int & xleft,int & ytop,int & xright,int & ybottom,gen * & gsel,gen * & gselparent,int &gselpos,std::vector<int> * goto_ptr){
    gsel=0;
    gselparent=0;
    gselpos=0;
    int x,y,w,h;
    attributs f(0,0,0);
    bool selected;
    Equation_box_sizes(g,w,h,x,y,f,selected);
    if ( (g.type==_EQW__VECT) || selected ){ // terminal or selected
      xleft=x;
      ybottom=y;
      if (selected){ // g is selected
	ytop=y+h;
	xright=x+w;
	gsel =  &g;
	//cout << "adjust " << *gsel << endl;
	return true;
      }
      else { // no selection
	xright=x;
	ytop=y;
	return false;
      }
    }
    if (g.type!=_VECT)
      return false;
    // last not selected, recurse
    iterateur it=g._VECTptr->begin(),itend=g._VECTptr->end()-1;
    for (;it!=itend;++it){
      if (Equation_adjust_xy(*it,xleft,ytop,xright,ybottom,gsel,gselparent,gselpos,goto_ptr)){
	if (goto_ptr){
	  goto_ptr->push_back(it-g._VECTptr->begin());
	  //cout << g << ":" << *goto_ptr << endl;
	}
	if (gsel==&*it){
	  // check next siblings
	  
	  gselparent= &g;
	  gselpos=it-g._VECTptr->begin();
	  //cout << "gselparent " << g << endl;
	}
	return true;
      }
    }
    return false;
  }
 
  // select or deselect part of the current eqution
  // This is done *in place*
  void Equation_select(gen & g,bool select){
    if (g.type==_EQW){
      eqwdata & e=*g._EQWptr;
      e.selected=select;
    }
    if (g.type!=_VECT)
      return;
    vecteur & v=*g._VECTptr;
    iterateur it=v.begin(),itend=v.end();
    for (;it!=itend;++it)
      Equation_select(*it,select);
  }

  // decrease selection (like HP49 eqw Down key)
  int eqw_select_down(gen & g){
    int xleft,ytop,xright,ybottom,gselpos;
    int newxleft,newytop,newxright,newybottom;
    gen * gsel,*gselparent;
    if (Equation_adjust_xy(g,xleft,ytop,xright,ybottom,gsel,gselparent,gselpos)){
      //cout << "select down before " << *gsel << endl;
      if (gsel->type==_VECT && !gsel->_VECTptr->empty()){
	Equation_select(*gsel,false);
	Equation_select(gsel->_VECTptr->front(),true);
	//cout << "select down after " << *gsel << endl;
	Equation_adjust_xy(g,newxleft,newytop,newxright,newybottom,gsel,gselparent,gselpos);
	return newytop-ytop;
      }
    }
    return 0;
  }

  int eqw_select_up(gen & g){
    int xleft,ytop,xright,ybottom,gselpos;
    int newxleft,newytop,newxright,newybottom;
    gen * gsel,*gselparent;
    if (Equation_adjust_xy(g,xleft,ytop,xright,ybottom,gsel,gselparent,gselpos) && gselparent){
      Equation_select(*gselparent,true);
      //cout << "gselparent " << *gselparent << endl;
      Equation_adjust_xy(g,newxleft,newytop,newxright,newybottom,gsel,gselparent,gselpos);
      return newytop-ytop;
    }
    return false;
  }

  // exchange==0 move selection to left or right sibling, ==2 add left or right
  // sibling, ==1 exchange selection with left or right sibling
  int eqw_select_leftright(Equation & eq,bool left,int exchange,GIAC_CONTEXT){
    gen & g=eq.data;
    int xleft,ytop,xright,ybottom,gselpos;
    int newxleft,newytop,newxright,newybottom;
    gen * gsel,*gselparent;
    vector<int> goto_sel;
    if (Equation_adjust_xy(g,xleft,ytop,xright,ybottom,gsel,gselparent,gselpos,&goto_sel) && gselparent && gselparent->type==_VECT){
      vecteur & gselv=*gselparent->_VECTptr;
      int n=gselv.size()-1,gselpos_orig=gselpos;
      if (n<1) return 0;
      if (left) {
	if (gselpos==0)
	  gselpos=n-1;
	else
	  gselpos--;
      }
      else {
	if (gselpos==n-1)
	  gselpos=0;
	else
	  gselpos++;
      }
      if (exchange==1){ // exchange gselpos_orig and gselpos
	swapgen(gselv[gselpos],gselv[gselpos_orig]);
	gsel=&gselv[gselpos_orig];
	gen value;
	if (xcas::do_select(*gsel,true,value) && value.type==_EQW)
	  replace_selection(eq,value._EQWptr->g,gsel,&goto_sel,contextptr);
      }
      else {
	// increase selection to next sibling possible for + and * only
	if (n>2 && exchange==2 && gselv[n].type==_EQW && (gselv[n]._EQWptr->g==at_plus || gselv[n]._EQWptr->g==at_prod)){
	  gen value1, value2,tmp;
	  if (gselpos_orig<gselpos)
	    swapint(gselpos_orig,gselpos);
	  // now gselpos<gselpos_orig
	  xcas::do_select(gselv[gselpos_orig],true,value1);
	  xcas::do_select(gselv[gselpos],true,value2);
	  if (value1.type==_EQW && value2.type==_EQW){
	    tmp=gselv[n]._EQWptr->g==at_plus?value1._EQWptr->g+value2._EQWptr->g:value1._EQWptr->g*value2._EQWptr->g;
	    gselv.erase(gselv.begin()+gselpos_orig);
	    replace_selection(eq,tmp,&gselv[gselpos],&goto_sel,contextptr);
	  }
	}
	else {
	  Equation_select(*gselparent,false);
	  gen & tmp=(*gselparent->_VECTptr)[gselpos];
	  Equation_select(tmp,true);
	}
      }
      Equation_adjust_xy(g,newxleft,newytop,newxright,newybottom,gsel,gselparent,gselpos);
      return newxleft-xleft;
    }
    return 0;
  }

  bool eqw_select(const gen & eq,int l,int c,bool select,gen & value){
    value=undef;
    if (l<0 || eq.type!=_VECT || eq._VECTptr->size()<=l)
      return false;
    gen & eql=(*eq._VECTptr)[l];
    if (c<0)
      return do_select(eql,select,value);
    if (eql.type!=_VECT || eql._VECTptr->size()<=c)
      return false;
    gen & eqlc=(*eql._VECTptr)[c];
    return do_select(eqlc,select,value);
  }

  gen Equation_compute_size(const gen & g,const attributs & a,int windowhsize,GIAC_CONTEXT);
  
  // void Bdisp_MMPrint(int x, int y, const char* string, int mode_flags, int xlimit, int P6, int P7, int color, int back_color, int writeflag, int P11); 
  // void PrintCXY(int x, int y, const char *cptr, int mode_flags, int P5, int color, int back_color, int P8, int P9)
  // void PrintMini( int* x, int* y, const char* string, int mode_flags, unsigned int xlimit, int P6, int P7, int color, int back_color, int writeflag, int P11) 
  void text_print(int fontsize,const char * s,int x,int y,int c=COLOR_BLACK,int bg=COLOR_WHITE,int mode=0){
    // *logptr(contextptr) << x << " " << y << " " << fontsize << " " << s << endl; return;
    c=(unsigned short) c;
#ifndef HP39
    if (mode==4 && c==COLOR_BLACK && bg==COLOR_WHITE){
      bg=color_gris;
      mode=0;
    }
#endif
    if (x>LCD_WIDTH_PX) return;
    int ss=strlen(s);
    if (ss==1 && s[0]==0x1e){ // arrow for limit
      if (mode==4)
	c=bg;
      draw_line(x,y-4,x+fontsize/2,y-4,c);
      draw_line(x,y-3,x+fontsize/2,y-3,c);
      draw_line(x+fontsize/2-4,y,x+fontsize/2,y-4,c);
      draw_line(x+fontsize/2-3,y,x+fontsize/2+1,y-4,c);
      draw_line(x+fontsize/2-4,y-7,x+fontsize/2,y-3,c);   
      draw_line(x+fontsize/2-3,y-7,x+fontsize/2+1,y-3,c);   
      return;
    }
    if (ss==2 && strcmp(s,"pi")==0){
      if (mode==4){
	drawRectangle(x,y+2-fontsize,fontsize,fontsize,c);
	c=bg;
      }
      draw_line(x+fontsize/3-1,y+1,x+fontsize/3,y+6-fontsize,c);
      draw_line(x+fontsize/3-2,y+1,x+fontsize/3-1,y+6-fontsize,c);
      draw_line(x+2*fontsize/3,y+1,x+2*fontsize/3,y+6-fontsize,c);
      draw_line(x+2*fontsize/3+1,y+1,x+2*fontsize/3+1,y+6-fontsize,c);
      draw_line(x+2,y+6-fontsize,x+fontsize,y+6-fontsize,c);
      draw_line(x+2,y+5-fontsize,x+fontsize,y+5-fontsize,c);
      return;
    }
    if (fontsize>=16 && ss==2 && s[0]==char(0xe5) && (s[1]==char(0xea) || s[1]==char(0xeb))) // special handling for increasing and decreasing in tabvar output
      fontsize=18;
    if (fontsize>=18){
      y -= 16;// status area shift
      os_draw_string(x,y,mode==4?bg:c,mode==4?c:bg,s);
      // PrintMini(&x,&y,(unsigned char *)s,mode,0xffffffff,0,0,c,bg,1,0);
      return;
    }
    y -= 12;
    x=os_draw_string_small(x,y,mode==4?bg:c,mode==4?c:bg,s);// PrintMiniMini( &x, &y, (unsigned char *)s, mode,c, 0 );
    return;
  }

  int text_width(int fontsize,const char * s){
#ifdef NSPIRE_NEWLIB
    int x=0;
    if (fontsize>=18)
      x=os_draw_string(0,0,0,1,s,true);
    else
      x=os_draw_string_small(0,0,0,1,s,true);
    return x;
#else
    if (fontsize>=18)
      return strlen(s)*11;
    return strlen(s)*7;
#endif
  }

  int fl_width(const char * s){
    return text_width(14,s);
  }
  
  void fl_arc(int x,int y,int rx,int ry,int theta1_deg,int theta2_deg,int c=COLOR_BLACK){
    rx/=2;
    ry/=2;
    // *logptr(contextptr) << "theta " << theta1_deg << " " << theta2_deg << endl;
    if (ry==rx){
      if (theta2_deg-theta1_deg==360){
	draw_circle(x+rx,y+rx,rx,c);
	return;
      }
      if (theta1_deg==0 && theta2_deg==180){
	draw_circle(x+rx,y+rx,rx,c,true,true,false,false);
	return;
      }
      if (theta1_deg==180 && theta2_deg==360){
	draw_circle(x+rx,y+rx,rx,c,false,false,true,true);
	return;
      }
    }
    // *logptr(contextptr) << "draw_arc" << theta1_deg*M_PI/180. << " " << theta2_deg*M_PI/180. << endl;
    draw_arc(x+rx,y+ry,rx,ry,c,theta1_deg*M_PI/180.,theta2_deg*M_PI/180.,context0);
  }

  void fl_pie(int x,int y,int rx,int ry,int theta1_deg,int theta2_deg,int c=COLOR_BLACK,bool segment=false){
    //cout << "fl_pie " << theta1_deg << " " << theta2_deg << " " << c << endl;
    if (!segment && ry==rx){
      if (theta2_deg-theta1_deg>=360){
	rx/=2;
	draw_filled_circle(x+rx,y+rx,rx,c);
	return;
      }
      if (theta1_deg==-90 && theta2_deg==90){
	rx/=2;
	draw_filled_circle(x+rx,y+rx,rx,c,false,true);
	return;
      }
      if (theta1_deg==90 && theta2_deg==270){
	rx/=2;
	draw_filled_circle(x+rx,y+rx,rx,c,true,false);
	return;
      }
    }
    // approximation by a filled polygon
    // points: (x,y), (x+rx*cos(theta)/2,y+ry*sin(theta)/2) theta=theta1..theta2
    while (theta2_deg<theta1_deg)
      theta2_deg+=360;
    if (theta2_deg-theta1_deg>=360){
      theta1_deg=0;
      theta2_deg=360;
    }
    int N0=theta2_deg-theta1_deg+1;
    // reduce N if rx or ry is small
    double red=double(rx)/LCD_WIDTH_PX*double(ry)/LCD_HEIGHT_PX;
    if (red>1) red=1;
    if (red<0.1) red=0.1;
    int N=red*N0;
    if (N<5)
      N=N0>5?5:N0;
    if (N<2)
      N=2;
    vector< vector<int> > v(segment?N+1:N+2,vector<int>(2));
    x += rx/2;
    y += ry/2;
    int i=0;
    if (!segment){
      v[0][0]=x;
      v[0][1]=y;
      ++i;
    }
    double theta=theta1_deg*M_PI/180;
    double thetastep=(theta2_deg-theta1_deg)*M_PI/(180*(N-1));
    for (;i<v.size()-1;++i){
      v[i][0]=int(x+rx*std::cos(theta)/2+.5);
      v[i][1]=int(y-ry*std::sin(theta)/2+.5); // y is inverted
      theta += thetastep;
    }
    v.back()=v.front();
    draw_filled_polygon(v,0,LCD_WIDTH_PX,24,LCD_HEIGHT_PX,c);
  }

  bool binary_op(const unary_function_ptr & u){
    const unary_function_ptr binary_op_tab_ptr []={*at_plus,*at_prod,*at_pow,*at_and,*at_ou,*at_xor,*at_different,*at_same,*at_equal,*at_unit,*at_compose,*at_composepow,*at_deuxpoints,*at_tilocal,*at_pointprod,*at_pointdivision,*at_pointpow,*at_division,*at_normalmod,*at_minus,*at_intersect,*at_union,*at_interval,*at_inferieur_egal,*at_inferieur_strict,*at_superieur_egal,*at_superieur_strict,*at_equal2,0};
    return equalposcomp(binary_op_tab_ptr,u);
  }
  
  eqwdata Equation_total_size(const gen & g){
    if (g.type==_EQW)
      return *g._EQWptr;
    if (g.type!=_VECT || g._VECTptr->empty())
      return eqwdata(0,0,0,0,attributs(0,0,0),undef);
    return Equation_total_size(g._VECTptr->back());
  }

  // find smallest value of y and height
  void Equation_y_dy(const gen & g,int & y,int & dy){
    y=0; dy=0;
    if (g.type==_EQW){
      y=g._EQWptr->y;
      dy=g._EQWptr->dy;
    }
    if (g.type==_VECT){
      iterateur it=g._VECTptr->begin(),itend=g._VECTptr->end();
      for (;it!=itend;++it){
	int Y,dY;
	Equation_y_dy(*it,Y,dY);
	// Y, Y+dY and y,y+dy
	int ymax=giacmax(y+dy,Y+dY);
	if (Y<y)
	  y=Y;
	dy=ymax-y;
      }
    }
  }

  void Equation_translate(gen & g,int deltax,int deltay){
    if (g.type==_EQW){
      g._EQWptr->x += deltax;
      g._EQWptr->y += deltay;
      g._EQWptr->baseline += deltay;
      return ;
    }
    if (g.type!=_VECT)
      setsizeerr();
    vecteur & v=*g._VECTptr;
    iterateur it=v.begin(),itend=v.end();
    for (;it!=itend;++it)
      Equation_translate(*it,deltax,deltay);
  }

  gen Equation_change_attributs(const gen & g,const attributs & newa){
    if (g.type==_EQW){
      gen res(*g._EQWptr);
      res._EQWptr->eqw_attributs = newa;
      return res;
    }
    if (g.type!=_VECT)
      return gensizeerr();
    vecteur v=*g._VECTptr;
    iterateur it=v.begin(),itend=v.end();
    for (;it!=itend;++it)
      *it=Equation_change_attributs(*it,newa);
    return gen(v,g.subtype);
  }

  vecteur Equation_subsizes(const gen & arg,const attributs & a,int windowhsize,GIAC_CONTEXT){
    vecteur v;
    if ( (arg.type==_VECT) && ( (arg.subtype==_SEQ__VECT) 
				// || (!ckmatrix(arg)) 
				) ){
      const_iterateur it=arg._VECTptr->begin(),itend=arg._VECTptr->end();
      for (;it!=itend;++it)
	v.push_back(Equation_compute_size(*it,a,windowhsize,contextptr));
    }
    else {
      v.push_back(Equation_compute_size(arg,a,windowhsize,contextptr));
    }
    return v;
  }

  // vertical merge with same baseline
  // for vertical merge of hp,yp at top (like ^) add fontsize to yp
  // at bottom (like lower bound of int) subtract fontsize from yp
  void Equation_vertical_adjust(int hp,int yp,int & h,int & y){
    int yf=min(y,yp);
    h=max(y+h,yp+hp)-yf;
    y=yf;
  }

  gen Equation_compute_symb_size(const gen & g,const attributs & a,int windowhsize,GIAC_CONTEXT){
    if (g.type!=_SYMB)
      return Equation_compute_size(g,a,windowhsize,contextptr);
    unary_function_ptr & u=g._SYMBptr->sommet;
    gen arg=g._SYMBptr->feuille,rootof_value;
    if (u==at_makevector){
      vecteur v(1,arg);
      if (arg.type==_VECT)
	v=*arg._VECTptr;
      iterateur it=v.begin(),itend=v.end();
      for (;it!=itend;++it){
	if ( (it->type==_SYMB) && (it->_SYMBptr->sommet==at_makevector) )
	  *it=_makevector(it->_SYMBptr->feuille,contextptr);
      }
      return Equation_compute_size(v,a,windowhsize,contextptr);
    }
    if (u==at_makesuite){
      if (arg.type==_VECT)
	return Equation_compute_size(gen(*arg._VECTptr,_SEQ__VECT),a,windowhsize,contextptr);
      else
	return Equation_compute_size(arg,a,windowhsize,contextptr);
    }
    if (u==at_sqrt)
      return Equation_compute_size(symb_pow(arg,plus_one_half),a,windowhsize,contextptr);
    if (u==at_division){
      if (arg.type!=_VECT || arg._VECTptr->size()!=2)
	return Equation_compute_size(arg,a,windowhsize,contextptr);
      gen tmp=Tfraction<gen>(arg._VECTptr->front(),arg._VECTptr->back());
      return Equation_compute_size(tmp,a,windowhsize,contextptr);
    }
    if (u==at_prod){
      gen n,d;
      if (rewrite_prod_inv(arg,n,d)){
	if (n.is_symb_of_sommet(at_neg))
	  return Equation_compute_size(symbolic(at_neg,Tfraction<gen>(-n,d)),a,windowhsize,contextptr);
	return Equation_compute_size(Tfraction<gen>(n,d),a,windowhsize,contextptr);
      }
    }
    if (u==at_inv){
      if ( (is_integer(arg) && is_positive(-arg,contextptr))
	   || (arg.is_symb_of_sommet(at_neg)))
	return Equation_compute_size(symbolic(at_neg,Tfraction<gen>(plus_one,-arg)),a,windowhsize,contextptr);
      return Equation_compute_size(Tfraction<gen>(plus_one,arg),a,windowhsize,contextptr);
    }
    if (u==at_expr && arg.type==_VECT && arg.subtype==_SEQ__VECT && arg._VECTptr->size()==2 && arg._VECTptr->back().type==_INT_){
      gen varg1=Equation_compute_size(arg._VECTptr->front(),a,windowhsize,contextptr);
      eqwdata vv(Equation_total_size(varg1));
      gen varg2=eqwdata(0,0,0,0,a,arg._VECTptr->back());
      vecteur v12(makevecteur(varg1,varg2));
      v12.push_back(eqwdata(vv.dx,vv.dy,0,vv.y,a,at_expr,0));
      return gen(v12,_SEQ__VECT);
    }
    int llp=int(text_width(a.fontsize,("(")))-1;
    int lrp=llp;
    int lc=int(text_width(a.fontsize,(",")));
    string us=u.ptr()->s;
    int ls=int(text_width(a.fontsize,(us.c_str())));
    // if (isalpha(u.ptr()->s[0])) ls += 1;
    if (u==at_abs)
      ls = 2;
    // special cases first int, sigma, /, ^
    // and if printed as printsommetasoperator
    // otherwise print with usual functional notation
    int x=0;
    int h=a.fontsize;
    int y=0;
#if 1
    if ((u==at_integrate) || (u==at_sum) ){ // Int
      int s=1;
      if (arg.type==_VECT)
	s=arg._VECTptr->size();
      else
	arg=vecteur(1,arg);
      // s==1 -> general case
      if ( (s==1) || (s==2) ){ // int f(x) dx and sum f(n) n
	vecteur v(Equation_subsizes(gen(*arg._VECTptr,_SEQ__VECT),a,windowhsize,contextptr));
	eqwdata vv(Equation_total_size(v[0]));
	if (s==1){
	  x=a.fontsize;
	  Equation_translate(v[0],x,0);
	  x += int(text_width(a.fontsize,(" dx")));
	}
	if (s==2){
	  if (u==at_integrate){
	    x=a.fontsize;
	    Equation_translate(v[0],x,0);
	    x += vv.dx+int(text_width(a.fontsize,(" d")));
	    Equation_vertical_adjust(vv.dy,vv.y,h,y);
	    vv=Equation_total_size(v[1]);
	    Equation_translate(v[1],x,0);
	    Equation_vertical_adjust(vv.dy,vv.y,h,y);
	  }
	  else {
	    Equation_vertical_adjust(vv.dy,vv.y,h,y);
	    eqwdata v1=Equation_total_size(v[1]);
	    x=max((int)a.fontsize,(int)v1.dx)+2*a.fontsize/3; // var name size
	    Equation_translate(v[1],0,-v1.dy-v1.y);
	    Equation_vertical_adjust(v1.dy,-v1.dy,h,y);
	    Equation_translate(v[0],x,0);
	    x += vv.dx; // add function size
	  }
	}
	if (u==at_integrate){
	  x += vv.dx;
	  if (h==a.fontsize)
	    h+=2*a.fontsize/3;
	  if (y==0){
	    y=-2*a.fontsize/3;
	    h+=2*a.fontsize/3;
	  }
	}
	v.push_back(eqwdata(x,h,0,y,a,u,0));
	return gen(v,_SEQ__VECT);
      }
      if (s>=3){ // int _a^b f(x) dx
	vecteur & intarg=*arg._VECTptr;
	gen tmp_l,tmp_u,tmp_f,tmp_x;
	attributs aa(a);
	if (a.fontsize>=10)
	  aa.fontsize -= 2;
	tmp_f=Equation_compute_size(intarg[0],a,windowhsize,contextptr);
	tmp_x=Equation_compute_size(intarg[1],a,windowhsize,contextptr);
	tmp_l=Equation_compute_size(intarg[2],aa,windowhsize,contextptr);
	if (s==4)
	  tmp_u=Equation_compute_size(intarg[3],aa,windowhsize,contextptr);
	x=a.fontsize+(u==at_integrate?-2:+4);
	eqwdata vv(Equation_total_size(tmp_l));
	Equation_translate(tmp_l,x,-vv.y-vv.dy);
	vv=Equation_total_size(tmp_l);
	Equation_vertical_adjust(vv.dy,vv.y,h,y);
	int lx = vv.dx;
	if (s==4){
	  vv=Equation_total_size(tmp_u);
	  Equation_translate(tmp_u,x,a.fontsize-3-vv.y);
	  vv=Equation_total_size(tmp_u);
	  Equation_vertical_adjust(vv.dy,vv.y,h,y);
	}
	x += max(lx,(int)vv.dx);
	Equation_translate(tmp_f,x,0);
	vv=Equation_total_size(tmp_f);
	Equation_vertical_adjust(vv.dy,vv.y,h,y);
	if (u==at_integrate){
	  x += vv.dx+int(text_width(a.fontsize,(" d")));
	  Equation_translate(tmp_x,x,0);
	  vv=Equation_total_size(tmp_x);
	  Equation_vertical_adjust(vv.dy,vv.y,h,y);
	  x += vv.dx;
	}
	else {
	  x += vv.dx;
	  Equation_vertical_adjust(vv.dy,vv.y,h,y);
	  vv=Equation_total_size(tmp_x);
	  x=max(x,(int)vv.dx)+a.fontsize/3;
	  Equation_translate(tmp_x,0,-vv.dy-vv.y);
	  //Equation_translate(tmp_l,0,-1);	  
	  if (s==4) Equation_translate(tmp_u,-2,0);	  
	  Equation_vertical_adjust(vv.dy,-vv.dy,h,y);
	}
	vecteur res(makevecteur(tmp_f,tmp_x,tmp_l));
	if (s==4)
	  res.push_back(tmp_u);
	res.push_back(eqwdata(x,h,0,y,a,u,0));
	return gen(res,_SEQ__VECT);
      }
    }
    if (u==at_limit && arg.type==_VECT){ // limit
      vecteur limarg=*arg._VECTptr;
      int s=limarg.size();
      if (s==2 && limarg[1].is_symb_of_sommet(at_equal)){
	limarg.push_back(limarg[1]._SYMBptr->feuille[1]);
	limarg[1]=limarg[1]._SYMBptr->feuille[0];
	++s;
      }
      if (s>=3){
	gen tmp_l,tmp_f,tmp_x,tmp_dir;
	attributs aa(a);
	if (a.fontsize>=10)
	  aa.fontsize -= 2;
	tmp_f=Equation_compute_size(limarg[0],a,windowhsize,contextptr);
	tmp_x=Equation_compute_size(limarg[1],aa,windowhsize,contextptr);
	tmp_l=Equation_compute_size(limarg[2],aa,windowhsize,contextptr);
	if (s==4)
	  tmp_dir=Equation_compute_size(limarg[3],aa,windowhsize,contextptr);
	eqwdata vf(Equation_total_size(tmp_f));
	eqwdata vx(Equation_total_size(tmp_x));
	eqwdata vl(Equation_total_size(tmp_l));
	eqwdata vdir(Equation_total_size(tmp_dir));
	int sous=max(vx.dy,vl.dy);
	if (s==4)
	  Equation_translate(tmp_f,vx.dx+vl.dx+vdir.dx+a.fontsize+4,0);
	else
	  Equation_translate(tmp_f,vx.dx+vl.dx+a.fontsize+2,0);
	Equation_translate(tmp_x,0,-sous-vl.y);
	Equation_translate(tmp_l,vx.dx+a.fontsize+2,-sous-vl.y);
	if (s==4)
	  Equation_translate(tmp_dir,vx.dx+vl.dx+a.fontsize+4,-sous-vl.y);
	h=vf.dy;
	y=vf.y;
	vl=Equation_total_size(tmp_l);
	Equation_vertical_adjust(vl.dy,vl.y,h,y);
	vecteur res(makevecteur(tmp_f,tmp_x,tmp_l));
	if (s==4){
	  res.push_back(tmp_dir);
	  res.push_back(eqwdata(vf.dx+vx.dx+a.fontsize+4+vl.dx+vdir.dx,h,0,y,a,u,0));
	}
	else
	  res.push_back(eqwdata(vf.dx+vx.dx+a.fontsize+2+vl.dx,h,0,y,a,u,0));
	return gen(res,_SEQ__VECT);
      }
    }
#endif
    if ( (u==at_of || u==at_at) && arg.type==_VECT && arg._VECTptr->size()==2 ){
      // user function, function in 1st arg, arguments in 2nd arg
      gen varg1=Equation_compute_size(arg._VECTptr->front(),a,windowhsize,contextptr);
      eqwdata vv=Equation_total_size(varg1);
      Equation_vertical_adjust(vv.dy,vv.y,h,y);
      gen arg2=arg._VECTptr->back();
      if (u==at_at && xcas_mode(contextptr)!=0){
	if (arg2.type==_VECT)
	  arg2=gen(addvecteur(*arg2._VECTptr,vecteur(arg2._VECTptr->size(),plus_one)),_SEQ__VECT);
	else
	  arg2=arg2+plus_one; 
      }
      gen varg2=Equation_compute_size(arg2,a,windowhsize,contextptr);
      Equation_translate(varg2,vv.dx+llp,0);
      vv=Equation_total_size(varg2);
      Equation_vertical_adjust(vv.dy,vv.y,h,y);
      vecteur res(makevecteur(varg1,varg2));
      res.push_back(eqwdata(vv.dx+vv.x+lrp,h,0,y,a,u,0));
      return gen(res,_SEQ__VECT);
    }
    if (u==at_pow){ 
      // first arg not translated
      gen varg=Equation_compute_size(arg._VECTptr->front(),a,windowhsize,contextptr);
      eqwdata vv=Equation_total_size(varg);
      // 1/2 ->sqrt, otherwise as exponent
      if (arg._VECTptr->back()==plus_one_half){
	Equation_translate(varg,a.fontsize,0);
	vecteur res(1,varg);
	res.push_back(eqwdata(vv.dx+a.fontsize,vv.dy+4,vv.x,vv.y,a,at_sqrt,0));
	return gen(res,_SEQ__VECT);
      }
      bool needpar=vv.g.type==_FUNC || vv.g.is_symb_of_sommet(at_pow) || need_parenthesis(vv.g);
      if (needpar)
	x=llp;
      Equation_translate(varg,x,0);
      Equation_vertical_adjust(vv.dy,vv.y,h,y);
      vecteur res(1,varg);
      // 2nd arg translated 
      if (needpar)
	x+=vv.dx+lrp;
      else
	x+=vv.dx+1;
      int arg1dy=vv.dy,arg1y=vv.y;
      if (a.fontsize>=16){
	attributs aa(a);
	aa.fontsize -= 2;
	varg=Equation_compute_size(arg._VECTptr->back(),aa,windowhsize,contextptr);
      }
      else
	varg=Equation_compute_size(arg._VECTptr->back(),a,windowhsize,contextptr);
      vv=Equation_total_size(varg);
      Equation_translate(varg,x,arg1y+(3*arg1dy)/4-vv.y);
      res.push_back(varg);
      vv=Equation_total_size(varg);
      Equation_vertical_adjust(vv.dy,vv.y,h,y);
      x += vv.dx;
      res.push_back(eqwdata(x,h,0,y,a,u,0));
      return gen(res,_SEQ__VECT);
    }
    if (u==at_factorial){
      vecteur v;
      gen varg=Equation_compute_size(arg,a,windowhsize,contextptr);
      eqwdata vv=Equation_total_size(varg);
      bool paren=need_parenthesis(vv.g) || vv.g==at_prod || vv.g==at_division || vv.g==at_pow;
      if (paren)
	x+=llp;
      Equation_translate(varg,x,0);
      Equation_vertical_adjust(vv.dy,vv.y,h,y);
      v.push_back(varg);
      x += vv.dx;
      if (paren)
	x+=lrp;
      varg=eqwdata(x+4,h,0,y,a,u,0);
      v.push_back(varg);
      return gen(v,_SEQ__VECT);
    }
    if (u==at_sto){ // A:=B, *it -> B
      gen varg=Equation_compute_size(arg._VECTptr->back(),a,windowhsize,contextptr);
      eqwdata vv=Equation_total_size(varg);
      Equation_vertical_adjust(vv.dy,vv.y,h,y);
      Equation_translate(varg,x,0);
      vecteur v(2);
      v[1]=varg;
      x+=vv.dx;
      x+=ls+3;
      // first arg not translated
      varg=Equation_compute_size(arg._VECTptr->front(),a,windowhsize,contextptr);
      vv=Equation_total_size(varg);
      if (need_parenthesis(vv.g))
	x+=llp;
      Equation_translate(varg,x,0);
      Equation_vertical_adjust(vv.dy,vv.y,h,y);
      v[0]=varg;
      x += vv.dx;
      if (need_parenthesis(vv.g))
	x+=lrp;
      v.push_back(eqwdata(x,h,0,y,a,u,0));
      return gen(v,_SEQ__VECT);
    }
    if (u==at_program && arg._VECTptr->back().type!=_VECT && !arg._VECTptr->back().is_symb_of_sommet(at_local) ){
      gen varg=Equation_compute_size(arg._VECTptr->front(),a,windowhsize,contextptr);
      eqwdata vv=Equation_total_size(varg);
      Equation_vertical_adjust(vv.dy,vv.y,h,y);
      Equation_translate(varg,x,0);
      vecteur v(2);
      v[0]=varg;
      x+=vv.dx;
      x+=int(text_width(a.fontsize,("->")))+3;
      varg=Equation_compute_size(arg._VECTptr->back(),a,windowhsize,contextptr);
      vv=Equation_total_size(varg);
      Equation_translate(varg,x,0);
      Equation_vertical_adjust(vv.dy,vv.y,h,y);
      v[1]=varg;
      x += vv.dx;
      v.push_back(eqwdata(x,h,0,y,a,u,0));
      return gen(v,_SEQ__VECT);      
    }
    bool binaryop= (u.ptr()->printsommet==&printsommetasoperator) || binary_op(u);
    if ( u!=at_sto && u.ptr()->printsommet!=NULL && !binaryop ){
      gen tmp=string2gen(g.print(contextptr),false);
      return Equation_compute_size(symbolic(at_expr,makesequence(tmp,xcas_mode(contextptr))),a,windowhsize,contextptr);
    }
    vecteur v;
    if (!binaryop || arg.type!=_VECT)
      v=Equation_subsizes(arg,a,windowhsize,contextptr);
    else
      v=Equation_subsizes(gen(*arg._VECTptr,_SEQ__VECT),a,windowhsize,contextptr);
    iterateur it=v.begin(),itend=v.end();
    if ( it==itend || (itend-it==1) ){ 
      gen gtmp;
      if (it==itend)
	gtmp=Equation_compute_size(gen(vecteur(0),_SEQ__VECT),a,windowhsize,contextptr);
      else
	gtmp=*it;
      // unary op, shift arg position horizontally
      eqwdata vv=Equation_total_size(gtmp);
      bool paren = u!=at_neg || (vv.g!=at_prod && need_parenthesis(vv.g)) ;
      x=ls+(paren?llp:0);
      gen tmp=gtmp; Equation_translate(tmp,x,0);
      x=x+vv.dx+(paren?lrp:0);
      Equation_vertical_adjust(vv.dy,vv.y,h,y);
      return gen(makevecteur(tmp,eqwdata(x,h,0,y,a,u,0)),_EQW__VECT);
    }
    if (binaryop){ // op (default with par)
      int currenth=h,largeur=0;
      iterateur itprec=v.begin();
      h=0;
      if (u==at_plus){ // op without parenthesis
	if (it->type==_VECT && !it->_VECTptr->empty() && it->_VECTptr->back().type==_EQW && it->_VECTptr->back()._EQWptr->g==at_equal)
	  ;
	else {
	  llp=0;
	  lrp=0;
	}
      }
      for (;;){
	eqwdata vv=Equation_total_size(*it);
	if (need_parenthesis(vv.g))
	  x+=llp;
	if (u==at_plus && it!=v.begin() &&
	    ( 
	     (it->type==_VECT && it->_VECTptr->back().type==_EQW && it->_VECTptr->back()._EQWptr->g==at_neg) 
	     || 
	     ( it->type==_EQW && (is_integer(it->_EQWptr->g) || it->_EQWptr->g.type==_DOUBLE_) && is_strictly_positive(-it->_EQWptr->g,contextptr) ) 
	      ) 
	    )
	  x -= ls;
#if 0 //
	if (x>windowhsize-vv.dx && x>windowhsize/2 && (itend-it)*vv.dx>windowhsize/2){
	  largeur=max(x,largeur);
	  x=0;
	  if (need_parenthesis(vv.g))
	    x+=llp;
	  h+=currenth;
	  Equation_translate(*it,x,0);
	  for (iterateur kt=v.begin();kt!=itprec;++kt)
	    Equation_translate(*kt,0,currenth);
	  if (y){
	    for (iterateur kt=itprec;kt!=it;++kt)
	      Equation_translate(*kt,0,-y);
	  }
	  itprec=it;
	  currenth=vv.dy;
	  y=vv.y;
	}
	else
#endif
	  {
	    Equation_translate(*it,x,0);
	    vv=Equation_total_size(*it);
	    Equation_vertical_adjust(vv.dy,vv.y,currenth,y);
	  }
	x+=vv.dx;
	if (need_parenthesis(vv.g))
	  x+=lrp;
	++it;
	if (it==itend){
	  for (iterateur kt=v.begin();kt!=itprec;++kt)
	    Equation_translate(*kt,0,currenth+y);
	  h+=currenth;
	  v.push_back(eqwdata(max(x,largeur),h,0,y,a,u,0));
	  //cout << v << endl;
	  return gen(v,_SEQ__VECT);
	}
	x += ls+3;
      } 
    }
    // normal printing
    x=ls+llp;
    for (;;){
      eqwdata vv=Equation_total_size(*it);
      Equation_translate(*it,x,0);
      Equation_vertical_adjust(vv.dy,vv.y,h,y);
      x+=vv.dx;
      ++it;
      if (it==itend){
	x+=lrp;
	v.push_back(eqwdata(x,h,0,y,a,u,0));
	return gen(v,_SEQ__VECT);
      }
      x+=lc;
    }
  }

  // windowhsize is used for g of type HIST__VECT (history) right justify answers
  // Returns either a eqwdata type object (terminal) or a vector 
  // (of subtype _EQW__VECT or _HIST__VECT)
  gen Equation_compute_size(const gen & g,const attributs & a,int windowhsize,GIAC_CONTEXT){
    /*****************
     *   FRACTIONS   *
     *****************/
    if (g.type==_FRAC){
      if (is_integer(g._FRACptr->num) && is_positive(-g._FRACptr->num,contextptr))
	return Equation_compute_size(symbolic(at_neg,fraction(-g._FRACptr->num,g._FRACptr->den)),a,windowhsize,contextptr);
      gen v1=Equation_compute_size(g._FRACptr->num,a,windowhsize,contextptr);
      eqwdata vv1=Equation_total_size(v1);
      gen v2=Equation_compute_size(g._FRACptr->den,a,windowhsize,contextptr);
      eqwdata vv2=Equation_total_size(v2);
      // Center the fraction
      int w1=vv1.dx,w2=vv2.dx;
      int w=max(w1,w2)+6;
      vecteur v(3);
      v[0]=v1; Equation_translate(v[0],(w-w1)/2,11-vv1.y);
      v[1]=v2; Equation_translate(v[1],(w-w2)/2,5-vv2.dy-vv2.y);
      v[2]=eqwdata(w,a.fontsize/2+vv1.dy+vv2.dy+1,0,(a.fontsize<=14?4:3)-vv2.dy,a,at_division,0);
      return gen(v,_SEQ__VECT);
    }
    /***************
     *   VECTORS   *
     ***************/
    if ( (g.type==_VECT) && !g._VECTptr->empty() ){
      vecteur v;
      const_iterateur it=g._VECTptr->begin(),itend=g._VECTptr->end();
      int x=0,y=0,h=a.fontsize; 
      /***************
       *   MATRICE   *
       ***************/
      bool gmat=ckmatrix(g);
      vector<int> V; int p=0;
      if (!gmat && is_mod_vecteur(*g._VECTptr,V,p) && p!=0){
	gen gm=makemodquoted(unmod(g),p);
	return Equation_compute_size(gm,a,windowhsize,contextptr);
      }
      vector< vector<int> > M; 
      if (gmat && is_mod_matrice(*g._VECTptr,M,p) && p!=0){
	gen gm=makemodquoted(unmod(g),p);
	return Equation_compute_size(gm,a,windowhsize,contextptr);
      }
      if (gmat && g.subtype!=_SEQ__VECT && g.subtype!=_SET__VECT && g.subtype!=_POLY1__VECT && g._VECTptr->front().subtype!=_SEQ__VECT){
	gen mkvect(at_makevector);
	mkvect.subtype=_SEQ__VECT;
	gen mkmat(at_makevector);
	mkmat.subtype=_MATRIX__VECT;
	int nrows,ncols;
	mdims(*g._VECTptr,nrows,ncols);
	if (ncols){
	  vecteur all_sizes;
	  all_sizes.reserve(nrows);
	  vector<int> row_heights(nrows),row_bases(nrows),col_widths(ncols);
	  // vertical gluing
	  for (int i=0;it!=itend;++it,++i){
	    gen tmpg=*it;
	    tmpg.subtype=_SEQ__VECT;
	    vecteur tmp(Equation_subsizes(tmpg,a,max(windowhsize/ncols-a.fontsize,230),contextptr));
	    int h=a.fontsize,y=0;
	    const_iterateur jt=tmp.begin(),jtend=tmp.end();
	    for (int j=0;jt!=jtend;++jt,++j){
	      eqwdata w(Equation_total_size(*jt));
	      Equation_vertical_adjust(w.dy,w.y,h,y);
	      col_widths[j]=max(col_widths[j],(int)w.dx);
	    }
	    if (i)
	      row_heights[i]=row_heights[i-1]+h+a.fontsize/2;
	    else
	      row_heights[i]=h;
	    row_bases[i]=y;
	    all_sizes.push_back(tmp);
	  }
	  // accumulate col widths
	  col_widths.front() +=(3*a.fontsize)/2;
	  vector<int>::iterator iit=col_widths.begin()+1,iitend=col_widths.end();
	  for (;iit!=iitend;++iit)
	    *iit += *(iit-1)+a.fontsize;
	  // translate each cell
	  it=all_sizes.begin();
	  itend=all_sizes.end();
	  int h,y,prev_h=0;
	  for (int i=0;it!=itend;++it,++i){
	    h=row_heights[i];
	    y=row_bases[i];
	    iterateur jt=it->_VECTptr->begin(),jtend=it->_VECTptr->end();
	    for (int j=0;jt!=jtend;++jt,++j){
	      eqwdata w(Equation_total_size(*jt));
	      if (j)
		Equation_translate(*jt,col_widths[j-1]-w.x,-h-y);
	      else
		Equation_translate(*jt,-w.x+a.fontsize/2,-h-y);
	    }
	    it->_VECTptr->push_back(eqwdata(col_widths.back(),h-prev_h,0,-h,a,mkvect,0));
	    prev_h=h;
	  }
	  all_sizes.push_back(eqwdata(col_widths.back(),row_heights.back(),0,-row_heights.back(),a,mkmat,-row_heights.back()/2));
	  gen all_sizesg=all_sizes; Equation_translate(all_sizesg,0,row_heights.back()/2); return all_sizesg;
	}
      } // end matrices
      /*************************
       *   SEQUENCES/VECTORS   *
       *************************/
      // horizontal gluing
      if (g.subtype!=_PRINT__VECT) x += a.fontsize/2;
      int ncols=itend-it;
      //ncols=min(ncols,5);
      for (;it!=itend;++it){
	gen cur_size=Equation_compute_size(*it,a,
					   max(windowhsize/ncols-a.fontsize,
#ifdef IPAQ
					       200
#else
					       480
#endif
					       ),contextptr);
	eqwdata tmp=Equation_total_size(cur_size);
	Equation_translate(cur_size,x-tmp.x,0); v.push_back(cur_size);
	x=x+tmp.dx+((g.subtype==_PRINT__VECT)?2:a.fontsize);
	Equation_vertical_adjust(tmp.dy,tmp.y,h,y);
      }
      gen mkvect(at_makevector);
      if (g.subtype==_SEQ__VECT)
	mkvect=at_makesuite;
      else
	mkvect.subtype=g.subtype;
      v.push_back(eqwdata(x,h,0,y,a,mkvect,0));
      return gen(v,_EQW__VECT);
    } // end sequences
    if (g.type==_MOD){ 
      int x=0;
      int h=a.fontsize;
      int y=0;
      int py=python_compat(contextptr);
      int modsize=int(text_width(a.fontsize,(py?" mod":"%")))+4;
      bool paren=is_positive(-*g._MODptr,contextptr);
      int llp=int(text_width(a.fontsize,("(")));
      int lrp=int(text_width(a.fontsize,(")")));
      gen varg1=Equation_compute_size(*g._MODptr,a,windowhsize,contextptr);
      if (paren) Equation_translate(varg1,llp,0);
      eqwdata vv=Equation_total_size(varg1);
      Equation_vertical_adjust(vv.dy,vv.y,h,y);
      gen arg2=*(g._MODptr+1);
      gen varg2=Equation_compute_size(arg2,a,windowhsize,contextptr);
      if (paren)
	Equation_translate(varg2,vv.dx+modsize+lrp,0);
      else
	Equation_translate(varg2,vv.dx+modsize,0);
      vv=Equation_total_size(varg2);
      Equation_vertical_adjust(vv.dy,vv.y,h,y);
      vecteur res(makevecteur(varg1,varg2));
      res.push_back(eqwdata(vv.dx+vv.x,h,0,y,a,at_normalmod,0));
      return gen(res,_SEQ__VECT);
    }
    if (g.type!=_SYMB){
      string s=g.type==_STRNG?*g._STRNGptr:g.print(contextptr);
      //if (g==cst_pi) s=char(129);
      if (s.size()>2000)
	s=s.substr(0,2000)+"...";
      int i=int(text_width(a.fontsize,(s.c_str())));
      gen tmp=eqwdata(i,a.fontsize,0,0,a,g);
      return tmp;
    }
    /**********************
     *  SYMBOLIC HANDLING *
     **********************/
    return Equation_compute_symb_size(g,a,windowhsize,contextptr);
    // return Equation_compute_symb_size(aplatir_fois_plus(g),a,windowhsize,contextptr);
    // aplatir_fois_plus is a problem for Equation_replace_selection
    // because it will modify the structure of the data
  }

  void Equation_draw(const eqwdata & e,int x,int y,int rightx,int lowery,Equation * eq,GIAC_CONTEXT){
    if ( (e.dx+e.x<x) || (e.x>rightx) || (e.y>y) || e.y+e.dy<lowery)
      ; // return; // nothing to draw, out of window
    gen gg=e.g;
    int fontsize=e.eqw_attributs.fontsize;
    int text_color=COLOR_BLACK;
    int background=COLOR_WHITE;
    string s=gg.type==_STRNG?*gg._STRNGptr:gg.print(contextptr);
    if (gg.type==_IDNT && !s.empty() && s[0]=='_')
      s=s.substr(1,s.size()-1);
    // if (gg==cst_pi){      s="p";      s[0]=(unsigned char)129;    }
    if (s.size()>2000)
      s=s.substr(0,2000)+"...";
    // cerr << s.size() << endl;
    text_print(fontsize,s.c_str(),eq->x()+e.x-x,eq->y()+y-e.y,text_color,background,e.selected?4:0);
    return;
  }

  inline void check_fl_rectf(int x,int y,int w,int h,int imin,int jmin,int di,int dj,int delta_i,int delta_j,int c){
    drawRectangle(x+delta_i,y+delta_j,w,h,c);
    //fl_rectf(x+delta_i,y+delta_j,w,h,c);
  }

  void Equation_draw(const gen & g,int x,int y,int rightx,int lowery,Equation * equat,GIAC_CONTEXT){
    int eqx=equat->x(),eqy=equat->y();
    if (g.type==_EQW){ // terminal
      eqwdata & e=*g._EQWptr;
      Equation_draw(e,x,y,rightx,lowery,equat,contextptr);
    }
    if (g.type!=_VECT)
      return;
    vecteur & v=*g._VECTptr;
    if (v.empty())
      return;
    gen tmp=v.back();
    if (tmp.type!=_EQW){
      cout << "EQW error:" << v << endl;
      return;
    }
    eqwdata & w=*tmp._EQWptr;
    if ( (w.dx+w.x-x<0) || (w.x>rightx) || (w.y>y) || (w.y+w.dy<lowery) )
      ; // return; // nothing to draw, out of window
    /*******************
     * draw the vector *
     *******************/
    // v is the vector, w the master operator eqwdata
    gen oper=w.g; 
    bool selected=w.selected ;
    int fontsize=w.eqw_attributs.fontsize;
    int background=w.eqw_attributs.background;
    int text_color=w.eqw_attributs.text_color;
    int mode=selected?4:0;
#ifdef HP39
    int draw_line_color=selected?255:0;
#else
    int draw_line_color=text_color; 
#endif
    int x0=w.x;
    int y0=w.y; // lower coordinate of the master vector
    int y1=y0+w.dy; // upper coordinate of the master vector
    if (selected)
      drawRectangle(eqx+w.x-x,eqy+y-w.y-w.dy+1,w.dx,w.dy+1,color_gris); // text_color);
    // draw arguments of v
    const_iterateur it=v.begin(),itend=v.end()-1;
    if (oper==at_expr && v.size()==3){
      Equation_draw(*it,x,y,rightx,lowery,equat,contextptr);
      return;
    }
    for (;it!=itend;++it)
      Equation_draw(*it,x,y,rightx,lowery,equat,contextptr);
    if (oper==at_multistring)
      return;
    string s;
    if (oper.type==_FUNC){
      // catch here special cases user function, vect/matr, ^, int, sqrt, etc.
      unary_function_ptr & u=*oper._FUNCptr;
      if (u==at_at){ // draw brackets around 2nd arg
	gen arg2=v[1]; // 2nd arg of at_of, i.e. what's inside the parenth.
	eqwdata varg2=Equation_total_size(arg2);
	x0=varg2.x;
	y0=varg2.y;
	y1=y0+varg2.dy;
	fontsize=varg2.eqw_attributs.fontsize;
	if (x0<rightx)
	  text_print(fontsize,"[",eqx+x0-x-int(text_width(fontsize,("["))),eqy+y-varg2.baseline,text_color,background,mode);
	x0 += varg2.dx ;
	if (x0<rightx)
	  text_print(fontsize,"]",eqx+x0-x,eqy+y-varg2.baseline,text_color,background,mode);
	return;
      }
      if (u==at_of){ // do we need to draw some parenthesis?
	gen arg2=v[1]; // 2nd arg of at_of, i.e. what's inside the parenth.
	if (arg2.type!=_VECT || arg2._VECTptr->back().type !=_EQW || arg2._VECTptr->back()._EQWptr->g!=at_makesuite){ // Yes (if not _EQW it's a sequence with parent)
	  eqwdata varg2=Equation_total_size(arg2);
	  x0=varg2.x;
	  y0=varg2.y;
	  y1=y0+varg2.dy;
	  fontsize=varg2.eqw_attributs.fontsize;
	  int pfontsize=max(fontsize,(fontsize+(varg2.baseline-varg2.y))/2);
	  if (x0<rightx)
	    text_print(pfontsize,"(",eqx+x0-x-int(text_width(fontsize,("("))),eqy+y-varg2.baseline,text_color,background,mode);
	  x0 += varg2.dx ;
	  if (x0<rightx)
	    text_print(pfontsize,")",eqx+x0-x,eqy+y-varg2.baseline,text_color,background,mode);
	}
	return;
      }
      if (u==at_makesuite){
	bool paren=v.size()!=2; // Sequences with 1 arg don't show parenthesis
	int pfontsize=max(fontsize,(fontsize+(w.baseline-w.y))/2);
	if (paren && x0<rightx)
	  text_print(pfontsize,"(",eqx+x0-x-int(text_width(fontsize,("(")))/2,eqy+y-w.baseline,text_color,background,mode);
	x0 += w.dx;
	if (paren && x0<rightx)
	  text_print(pfontsize,")",eqx+x0-x-int(text_width(fontsize,("(")))/2,eqy+y-w.baseline,text_color,background,mode);
	// print commas between args
	it=v.begin(),itend=v.end()-2;
	for (;it!=itend;++it){
	  eqwdata varg2=Equation_total_size(*it);
	  fontsize=varg2.eqw_attributs.fontsize;
	  if (varg2.x+varg2.dx<rightx)
	    text_print(fontsize,",",eqx+varg2.x+varg2.dx-x+1,eqy+y-varg2.baseline,text_color,background,mode);
	}
	return;
      }
      if (u==at_makevector){ // draw [] delimiters for vector/matrices
	if (oper.subtype!=_SEQ__VECT && oper.subtype!=_PRINT__VECT){
	  int decal=1;
	  switch (oper.subtype){
	  case _MATRIX__VECT: decal=2; break;
	  case _SET__VECT: decal=4; break;
	  case _POLY1__VECT: decal=6; break;
	  }
	  if (eqx+x0-x+1>=0){
	    draw_line(eqx+x0-x+1,eqy+y-y0+1,eqx+x0-x+1,eqy+y-y1+1,draw_line_color);
	    draw_line(eqx+x0-x+decal,eqy+y-y0+1,eqx+x0-x+decal,eqy+y-y1+1,draw_line_color);
	    draw_line(eqx+x0-x+1,eqy+y-y0+1,eqx+x0-x+fontsize/4,eqy+y-y0+1,draw_line_color);
	    draw_line(eqx+x0-x+1,eqy+y-y1+1,eqx+x0-x+fontsize/4,eqy+y-y1+1,draw_line_color);
	  }
	  x0 += w.dx ;
	  if (eqx+x0-x-1<LCD_WIDTH_PX){
	    draw_line(eqx+x0-x-1,eqy+y-y0+1,eqx+x0-x-1,eqy+y-y1+1,draw_line_color);
	    draw_line(eqx+x0-x-decal,eqy+y-y0+1,eqx+x0-x-decal,eqy+y-y1+1,draw_line_color);
	    draw_line(eqx+x0-x-1,eqy+y-y0+1,eqx+x0-x-fontsize/4,eqy+y-y0+1,draw_line_color);
	    draw_line(eqx+x0-x-1,eqy+y-y1+1,eqx+x0-x-fontsize/4,eqy+y-y1+1,draw_line_color);
	  }
	} // end if oper.subtype!=SEQ__VECT
	if (oper.subtype!=_MATRIX__VECT && oper.subtype!=_PRINT__VECT){
	  // print commas between args
	  it=v.begin(),itend=v.end()-2;
	  for (;it!=itend;++it){
	    eqwdata varg2=Equation_total_size(*it);
	    fontsize=varg2.eqw_attributs.fontsize;
	    if (varg2.x+varg2.dx<rightx)
	      text_print(fontsize,",",eqx+varg2.x+varg2.dx-x+1,eqy+y-varg2.baseline,text_color,background,mode);
	  }
	}
	return;
      }
      int lpsize=int(text_width(fontsize,("(")));
      int rpsize=int(text_width(fontsize,(")")));
      eqwdata tmp=Equation_total_size(v.front()); // tmp= 1st arg eqwdata
      if (u==at_sto)
	tmp=Equation_total_size(v[1]);
      x0=w.x-x;
      y0=y-w.baseline;
      if (u==at_pow){
	if (!need_parenthesis(tmp.g)&& tmp.g!=at_pow && tmp.g!=at_prod && tmp.g!=at_division)
	  return;
	if (tmp.g==at_pow){
	  fontsize=tmp.eqw_attributs.fontsize+2;
	}
	if (tmp.x-lpsize<rightx)
	  text_print(fontsize,"(",eqx+tmp.x-x-lpsize,eqy+y-tmp.baseline,text_color,background,mode);
	if (tmp.x+tmp.dx<rightx)
	  text_print(fontsize,")",eqx+tmp.x+tmp.dx-x,eqy+y-tmp.baseline,text_color,background,mode);
	return;
      }
      if (u==at_program){
	if (tmp.x+tmp.dx<rightx)
	  text_print(fontsize,"->",eqx+tmp.x+tmp.dx-x,eqy+y-tmp.baseline,text_color,background,mode);
	return;
      }
#if 1
      if (u==at_sum){
	if (x0<rightx){
	  draw_line(eqx+x0,eqy+y0,eqx+x0+(2*fontsize)/3,eqy+y0,draw_line_color);
	  draw_line(eqx+x0,eqy+y0-fontsize,eqx+x0+(2*fontsize)/3,eqy+y0-fontsize,draw_line_color);
	  draw_line(eqx+x0,eqy+y0,eqx+x0+fontsize/2,eqy+y0-fontsize/2,draw_line_color);
	  draw_line(eqx+x0+fontsize/2,eqy+y0-fontsize/2,eqx+x0,eqy+y0-fontsize,draw_line_color);
	  if (v.size()>2){ // draw the =
	    eqwdata ptmp=Equation_total_size(v[1]);
	    if (ptmp.x+ptmp.dx<rightx)
	      text_print(fontsize,"=",eqx+ptmp.x+ptmp.dx-x-2,eqy+y-ptmp.baseline,text_color,background,mode);
	  }
	}
	return;
      }
#endif
      if (u==at_abs){
	y0 =1+y-w.y;
	int h=w.dy;
	if (x0<rightx){
	  draw_line(eqx+x0+2,eqy+y0-1,eqx+x0+2,eqy+y0-h+3,draw_line_color);
	  draw_line(eqx+x0+1,eqy+y0-1,eqx+x0+1,eqy+y0-h+3,draw_line_color);
	  draw_line(eqx+x0+w.dx-1,eqy+y0-1,eqx+x0+w.dx-1,eqy+y0-h+3,draw_line_color);
	  draw_line(eqx+x0+w.dx,eqy+y0-1,eqx+x0+w.dx,eqy+y0-h+3,draw_line_color);
	}
	return;
      }
      if (u==at_sqrt){
	y0 =1+y-w.y;
	int h=w.dy;
	if (x0<rightx){
	  draw_line(eqx+x0+2,eqy+y0-h/2,eqx+x0+fontsize/2,eqy+y0-1,draw_line_color);
	  draw_line(eqx+x0+fontsize/2,eqy+y0-1,eqx+x0+fontsize,eqy+y0-h+3,draw_line_color);
	  draw_line(eqx+x0+fontsize,eqy+y0-h+3,eqx+x0+w.dx-1,eqy+y0-h+3,draw_line_color);
	  ++y0;
	  draw_line(eqx+x0+2,eqy+y0-h/2,eqx+x0+fontsize/2,eqy+y0-1,draw_line_color);
	  draw_line(eqx+x0+fontsize/2,eqy+y0-1,eqx+x0+fontsize,eqy+y0-h+3,draw_line_color);
	  draw_line(eqx+x0+fontsize,eqy+y0-h+3,eqx+x0+w.dx-1,eqy+y0-h+3,draw_line_color);
	}
	return;
      }
      if (u==at_factorial){
	text_print(fontsize,"!",eqx+w.x+w.dx-4-x,eqy+y-w.baseline,text_color,background,mode);
	if (!need_parenthesis(tmp.g)
	    && tmp.g!=at_pow && tmp.g!=at_prod && tmp.g!=at_division
	    )
	  return;
	if (tmp.x-lpsize<rightx)
	  text_print(fontsize,"(",eqx+tmp.x-x-lpsize,eqy+y-tmp.baseline,text_color,background,mode);
	if (tmp.x+tmp.dx<rightx)
	  text_print(fontsize,")",eqx+tmp.x+tmp.dx-x,eqy+y-tmp.baseline,text_color,background,mode);
	return;
      }
#if 1
      if (u==at_integrate){
	x0+=2;
	y0+=fontsize/2;
	if (x0<rightx){
	  fl_arc(eqx+x0,eqy+y0,fontsize/3,fontsize/3,180,360,draw_line_color);
	  draw_line(eqx+x0+fontsize/3,eqy+y0,eqx+x0+fontsize/3,eqy+y0-2*fontsize+4,draw_line_color);
	  fl_arc(eqx+x0+fontsize/3,eqy+y0-2*fontsize+3,fontsize/3,fontsize/3,0,180,draw_line_color);
	}
	if (v.size()!=2){ // if arg has size > 1 draw the d
	  eqwdata ptmp=Equation_total_size(v[1]);
	  if (ptmp.x<rightx)
	    text_print(fontsize," d",eqx+ptmp.x-x-int(text_width(fontsize,(" d"))),eqy+y-ptmp.baseline,text_color,background,mode);
	}
	else {
	  eqwdata ptmp=Equation_total_size(v[0]);
	  if (ptmp.x+ptmp.dx<rightx)
	    text_print(fontsize," dx",eqx+ptmp.x+ptmp.dx-x,eqy+y-ptmp.baseline,text_color,background,mode);
	}
	return;
      }
#endif
      if (u==at_division){
	if (x0<rightx){
	  int yy=eqy+y0-8;
	  draw_line(eqx+x0+2,yy,eqx+x0+w.dx-2,yy,draw_line_color);
	  ++yy;
	  draw_line(eqx+x0+2,yy,eqx+x0+w.dx-2,yy,draw_line_color);
	}
	return;
      }
#if 1
      if (u==at_limit && v.size()>=4){
	if (x0<rightx)
	  text_print(fontsize,"lim",eqx+w.x-x,eqy+y-w.baseline,text_color,background,mode);
	gen arg2=v[1]; // 2nd arg of limit, i.e. the variable
	if (arg2.type==_EQW){ 
	  eqwdata & varg2=*arg2._EQWptr;
	  if (varg2.x+varg2.dx+2<rightx)
	    text_print(fontsize,"\x1e",eqx+varg2.x+varg2.dx+2-x,eqy+y-varg2.y,text_color,background,mode);
	}
	if (v.size()>=5){
	  arg2=v[2]; // 3rd arg of lim, the point, draw a comma after if dir.
	  if (arg2.type==_EQW){ 
	    eqwdata & varg2=*arg2._EQWptr;
	    if (varg2.x+varg2.dx<rightx)
	      text_print(fontsize,",",eqx+varg2.x+varg2.dx-x,eqy+y-varg2.baseline,text_color,background,mode);
	  }
	}
	return;
      } // limit
#endif
      bool parenthesis=true;
      string opstring(",");
      if (u.ptr()->printsommet==&printsommetasoperator || binary_op(u) ){
	if (u==at_normalmod && python_compat(contextptr))
	  opstring=" mod";
	else
	  opstring=u.ptr()->s;
      }
      else {
	if (u==at_sto)
	  opstring=":=";
	parenthesis=false;
      }
      // int yy=y0; // y0 is the lower coordinate of the whole eqwdata
      // int opsize=int(text_width(fontsize,(opstring.c_str())))+3;
      it=v.begin();
      itend=v.end()-1;
      // Reminder: here tmp is the 1st arg eqwdata, w the whole eqwdata
      if ( (itend-it==1) && ( (u==at_neg) 
			      || (u==at_plus) // uncommented for +infinity
			      ) ){ 
	if ( (u==at_neg &&need_parenthesis(tmp.g) && tmp.g!=at_prod)){
	  if (tmp.x-lpsize<rightx)
	    text_print(fontsize,"(",eqx+tmp.x-x-lpsize,eqy+y-tmp.baseline,text_color,background,mode);
	  if (tmp.x+tmp.dx<rightx)
	    text_print(fontsize,")",eqx+tmp.x-x+tmp.dx,eqy+y-tmp.baseline,text_color,background,mode);
	}
	if (w.x<rightx){
	  text_print(fontsize,u.ptr()->s,eqx+w.x-x,eqy+y-w.baseline,text_color,background,mode);
	}
	return;
      }
      // write first open parenthesis
      if (u==at_plus && tmp.g!=at_equal)
	parenthesis=false;
      else {
	if (parenthesis && need_parenthesis(tmp.g)){
	  if (w.x<rightx){
	    int pfontsize=max(fontsize,(fontsize+(tmp.baseline-tmp.y))/2);
	    text_print(pfontsize,"(",eqx+w.x-x,eqy+y-tmp.baseline,text_color,background,mode);
	  }
	}
      }
      for (;;){
	// write close parenthesis at end
	int xx=tmp.dx+tmp.x-x;
	if (parenthesis && need_parenthesis(tmp.g)){
	  if (xx<rightx){
	    int pfontsize=min(max(fontsize,(fontsize+(tmp.baseline-tmp.y))/2),fontsize*2);
	    int deltapary=(2*(pfontsize-fontsize))/3;
	    text_print(pfontsize,")",eqx+xx,eqy+y-tmp.baseline+deltapary,text_color,background,mode);
	  }
	  xx +=rpsize;
	}
	++it;
	if (it==itend){
	  if (u.ptr()->printsommet==&printsommetasoperator || u==at_sto || binary_op(u))
	    return;
	  else
	    break;
	}
	// write operator
	if (u==at_prod){
	  // text_print(fontsize,".",eqx+xx+3,eqy+y-tmp.baseline-fontsize/3);
	  text_print(fontsize,opstring.c_str(),eqx+xx+1,eqy+y-tmp.baseline,text_color,background,mode);
	}
	else {
	  gen tmpgen;
	  if (u==at_plus && ( 
			     (it->type==_VECT && it->_VECTptr->back().type==_EQW && it->_VECTptr->back()._EQWptr->g==at_neg) 
			     || 
			     ( it->type==_EQW && (is_integer(it->_EQWptr->g) || it->_EQWptr->g.type==_DOUBLE_) && is_strictly_positive(-it->_EQWptr->g,contextptr) ) 
			      )
	      )
	    ;
	  else {
	    if (xx+1<rightx)
	      // fl_draw(opstring.c_str(),xx+1,y-tmp.y-tmp.dy/2+fontsize/2);
	      text_print(fontsize,opstring.c_str(),eqx+xx+1,eqy+y-tmp.baseline,text_color,background,mode);
	  }
	}
	// write right parent, update tmp
	tmp=Equation_total_size(*it);
	if (parenthesis && (need_parenthesis(tmp.g)) ){
	  if (tmp.x-lpsize<rightx){
	    int pfontsize=min(max(fontsize,(fontsize+(tmp.baseline-tmp.y))/2),fontsize*2);
	    int deltapary=(2*(pfontsize-fontsize))/3;
	    text_print(pfontsize,"(",eqx+tmp.x-pfontsize*lpsize/fontsize-x,eqy+y-tmp.baseline+deltapary,text_color,background,mode);
	  }
	}
      } // end for (;;)
      if (w.x<rightx){
	s = u.ptr()->s;
	s += '(';
	text_print(fontsize,s.c_str(),eqx+w.x-x,eqy+y-w.baseline,text_color,background,mode);
      }
      if (w.x+w.dx-rpsize<rightx)
	text_print(fontsize,")",eqx+w.x+w.dx-x-rpsize+2,eqy+y-w.baseline,text_color,background,mode);
      return;
    }
    s=oper.print(contextptr);
    if (w.x<rightx){
      text_print(fontsize,s.c_str(),eqx+w.x-x,eqy+y-w.baseline,text_color,background,mode);
    }
  }

  Equation::Equation(int x_, int y_, const gen & g,const giac::context * cptr){
    _x=x_;
    _y=y_;
    attr=attributs(18,COLOR_WHITE,COLOR_BLACK);
    contextptr=cptr;
    if (taille(g,max_prettyprint_equation)<max_prettyprint_equation)
      data=Equation_compute_size(g,attr,LCD_WIDTH_PX,contextptr);
    else
      data=Equation_compute_size(string2gen("Object_too_large",false),attr,LCD_WIDTH_PX,contextptr);
    undodata=Equation_copy(data);
  }

  void replace_selection(Equation & eq,const gen & tmp,gen * gsel,const vector<int> * gotoptr,GIAC_CONTEXT){
    int xleft,ytop,xright,ybottom,gselpos; gen *gselparent;
    vector<int> goto_sel;
    eq.undodata=Equation_copy(eq.data);
    if (gotoptr==0){
      if (xcas::Equation_adjust_xy(eq.data,xleft,ytop,xright,ybottom,gsel,gselparent,gselpos,&goto_sel) && gsel)
	gotoptr=&goto_sel;
      else
	return;
    }
    *gsel=xcas::Equation_compute_size(tmp,eq.attr,LCD_WIDTH_PX,contextptr);
    gen value;
    xcas::do_select(eq.data,true,value);
    if (value.type==_EQW)
      eq.data=xcas::Equation_compute_size(value._EQWptr->g,eq.attr,LCD_WIDTH_PX,contextptr);
    //cout << "new value " << value << " " << eq.data << " " << *gotoptr << endl;
    xcas::Equation_select(eq.data,false);
    gen * gptr=&eq.data;
    for (int i=gotoptr->size()-1;i>=0;--i){
      int pos=(*gotoptr)[i];
      if (gptr->type==_VECT &&gptr->_VECTptr->size()>pos)
	gptr=&(*gptr->_VECTptr)[pos];
    }
    xcas::Equation_select(*gptr,true);
    //cout << "new sel " << *gptr << endl;
  }

  void display(Equation & eq,int x,int y,GIAC_CONTEXT){
    // Equation_draw(eq.data,x,y,LCD_WIDTH_PX,0,&eq,contextptr);
    int xleft,ytop,xright,ybottom,gselpos; gen * gsel,*gselparent;
    eqwdata eqdata=Equation_total_size(eq.data);
    if ( (eqdata.dx>LCD_WIDTH_PX || eqdata.dy>LCD_HEIGHT_PX-STATUS_AREA_PX) && Equation_adjust_xy(eq.data,xleft,ytop,xright,ybottom,gsel,gselparent,gselpos)){
      if (x<xleft){
	if (x+LCD_WIDTH_PX<xright)
	  x=giacmin(xleft,xright-LCD_WIDTH_PX);
      }
      if (x>=xleft && x+LCD_WIDTH_PX>=xright){
	if (xright-x<LCD_WIDTH_PX)
	  x=giacmax(xright-LCD_WIDTH_PX,0);
      }
#if 0
      cout << "avant " << y << " " << ytop << " " << ybottom << endl;
      if (y<ytop){
	if (y+LCD_HEIGHT_PX<ybottom)
	  y=giacmin(ytop,ybottom-LCD_HEIGHT_PX);
      }
      if (y>=ytop && y+LCD_HEIGHT_PX>=ybottom){
	if (ybottom-y<LCD_HEIGHT_PX)
	  y=giacmax(ybottom-LCD_HEIGHT_PX,0);
      }
      cout << "apres " << y << " " << ytop << " " << ybottom << endl;
#endif
    }
    int save_ymin_clip=clip_ymin;
    clip_ymin=STATUS_AREA_PX;
    Equation_draw(eq.data,x,y,RAND_MAX,0,&eq,contextptr);
    clip_ymin=save_ymin_clip;
  }
  
  /* ******************* *
   *      GRAPH          *
   * ******************* *
   */
#if 1

  double find_tick(double dx){
    double d=std::pow(10.0,std::floor(std::log10(absdouble(dx))));
    if (dx<2*d)
      d=d/5;
    else {
      if (dx<5*d)
	d=d/2;
    }
    return d;
  }

  // check if point is inside triangle
  inline bool inside(double x0,double x1,double x2,
		     double y0,double y1,double y2,
		     double x,double y){
#if 1
    double y02=y2-y0,y10=y0-y1;
    double invarea = x0*(y1-y2) + x1*y02 + x2*y10;
    if (invarea==0) return false;
    invarea=1/invarea;
    double yy0=y0-y, s=invarea*(x*y02 + x2*yy0 + x0*(y-y2));
    if (s<-1e-12) return false;
    double t = invarea*(x*y10 + x0*(y1-y) - x1*yy0);
    return t>=-1e-12 && s+t<=1+1e-12;
#else
    double as_x = x-x0;
    double as_y = y-y0;
    bool s_01 = (x1-x0)*as_y-(y1-y0)*as_x>0; // dot product P0P1.P0P>0
    if ( (x2-x0)*as_y-(y2-y0)*as_x>0 == s_01)
      return false;
    if ((x2-x1)*(y-y1)-(y2-y1)*(x-x1) > 0 != s_01)
      return false;
    return true;
#endif
  }

  /* 3d rotation handling */
  void normalize(double & a,double &b,double &c){
    double n=std::sqrt(a*a+b*b+c*c);
    a /= n;
    b /= n;
    c /= n;
  }

  inline int Min(int i,int j) {return i>j?j:i;}

  inline int Max(int i,int j) {return i>j?i:j;}

  quaternion_double::quaternion_double(double theta_x,double theta_y,double theta_z) { 
    *this=euler_deg_to_quaternion_double(theta_x,theta_y,theta_z); 
  }

  quaternion_double euler_deg_to_quaternion_double(double a,double b,double c){
    double phi=a*M_PI/180, theta=b*M_PI/180, psi=c*M_PI/180;
    double c1 = std::cos(phi/2);
    double s1 = std::sin(phi/2);
    double c2 = std::cos(theta/2);
    double s2 = std::sin(theta/2);
    double c3 = std::cos(psi/2);
    double s3 = std::sin(psi/2);
    double c1c2 = c1*c2;
    double s1s2 = s1*s2;
    double w =c1c2*c3 - s1s2*s3;
    double x =c1c2*s3 + s1s2*c3;
    double y =s1*c2*c3 + c1*s2*s3;
    double z =c1*s2*c3 - s1*c2*s3;
    return quaternion_double(w,x,y,z);
  }

  void quaternion_double_to_euler_deg(const quaternion_double & q,double & phi,double & theta, double & psi){
    double test = q.x*q.y + q.z*q.w;
    if (test > 0.499) { // singularity at north pole
      phi = 2 * atan2(q.x,q.w) * 180/M_PI;
      theta = 90; 
      psi = 0;
      return;
    }
    if (test < -0.499) { // singularity at south pole
      phi = -2 * atan2(q.x,q.w) * 180/M_PI;
      theta = - 90;
      psi = 0;
      return;
    }
    double sqx = q.x*q.x;
    double sqy = q.y*q.y;
    double sqz = q.z*q.z;
    phi = atan2(2*q.y*q.w-2*q.x*q.z , 1 - 2*sqy - 2*sqz) * 180/M_PI;
    theta = asin(2*test) * 180/M_PI;
    psi = atan2(2*q.x*q.w-2*q.y*q.z , 1 - 2*sqx - 2*sqz) * 180/M_PI;
  }

  quaternion_double operator * (const quaternion_double & q1,const quaternion_double & q2){ 
    double z=q1.w*q2.z+q2.w*q1.z+q1.x*q2.y-q2.x*q1.y;
    double x=q1.w*q2.x+q2.w*q1.x+q1.y*q2.z-q2.y*q1.z;
    double y=q1.w*q2.y+q2.w*q1.y+q1.z*q2.x-q2.z*q1.x;
    double w=q1.w*q2.w-q1.x*q2.x-q1.y*q2.y-q1.z*q2.z;
    return quaternion_double(w,x,y,z);
  }

  // q must be a unit
  void get_axis_angle_deg(const quaternion_double & q,double &x,double &y,double & z, double &theta){
    double scale=1-q.w*q.w;
    if (scale>1e-6){
      scale=std::sqrt(scale);
      theta=2*std::acos(q.w)*180/M_PI;
      x=q.x/scale;
      y=q.y/scale;
      z=q.z/scale;
    }
    else {
      x=0; y=0; z=1;
      theta=0;
    }
  }

  quaternion_double rotation_2_quaternion_double(double x, double y, double z,double theta){
    double t=theta*M_PI/180;
    double qx,qy,qz,qw,s=std::sin(t/2),c=std::cos(t/2);
    qx=x*s;
    qy=y*s;
    qz=z*s;
    qw=c;
    double n=std::sqrt(qx*qx+qy*qy+qz*qz+qw*qw);
    return quaternion_double(qw/n,qx/n,qy/n,qz/n);
  }

  // image of (x,y,z) by rotation around axis r(rx,ry,rz) of angle theta
  void rotate(double rx,double ry,double rz,double theta,double x,double y,double z,double & X,double & Y,double & Z){
    /*
    quaternion_double q=rotation_2_quaternion_double(rx,ry,rz,theta);
    quaternion_double qx(x,y,z,0);
    quaternion_double qX=conj(q)*qx*q;
    */
    // r(rx,ry,rz) the axis, v(x,y,z) projects on w=a*r with a such that
    // w.r=a*r.r=v.r
    double r2=rx*rx+ry*ry+rz*rz;
    double r=std::sqrt(r2);
    double a=(rx*x+ry*y+rz*z)/r2;
    // v=w+V, w remains stable, V=v-w=v-a*r rotates
    // Rv=w+RV, where RV=cos(theta)*V+sin(theta)*(r cross V)/sqrt(r2)
    double Vx=x-a*rx,Vy=y-a*ry,Vz=z-a*rz;
    // cross product of k with V
    double kVx=ry*Vz-rz*Vy, kVy=rz*Vx-rx*Vz,kVz=rx*Vy-ry*Vx;
    double c=std::cos(theta),s=std::sin(theta);
    X=a*rx+c*Vx+s*kVx/r;
    Y=a*ry+c*Vy+s*kVy/r;
    Z=a*rz+c*Vz+s*kVz/r;
  }

  int diffuse(int color_orig,double diffusionz){
    if (diffusionz<1.1)
      return color_orig;
    int color=rgb565to888(color_orig);
    int r=(color&0xff0000)>>16,g=(color & 0xff00)>>8,b=192;
    double attenuate=(.3*(diffusionz-1));
    attenuate=1.0/(1+attenuate*attenuate);
    r*=attenuate; g*=attenuate; b*=attenuate;
    return rgb888to565((r<<16)|(g<<8)|b);
  }
  
  void glinter1(double z,double dz,
		double *zmin,double *zmax,double ZMIN,double ZMAX,
		int ih,int lcdz,
		int upcolor,int downcolor,int diffusionz,int diffusionz_limit,bool interval
		){
    if (ZMIN<z && z<ZMAX)
      return;
    // lcdz tests below: avoid marking too large regions
    if (*zmax<*zmin || z<*zmin-lcdz || z>*zmax+lcdz)
      *zmax=*zmin=z;
    bool diffus=diffusionz<diffusionz_limit;
    double deltaz;
    if (interval){
      bool intervalonly=false;
      if (z<0) {
	// return;
	z=0; intervalonly=true;
      }
      if (z>=LCD_HEIGHT_PX) {
	// return;
	z=LCD_HEIGHT_PX-1; intervalonly=true;
      }
      deltaz=diffus?1:diffusionz;
      if (z>*zmax+deltaz){
	if (diffus){
	  drawRectangle(ih,*zmax,1,std::ceil(z-*zmax),diffuse(downcolor,diffusionz));
	  if (!intervalonly)
	    os_set_pixel(ih,z,downcolor);
	}
	else {
	  drawRectangle(ih,*zmax,1,std::ceil(z-*zmax),_BLACK);
	  // draw interval
	  int nstep=int(z-*zmax)/diffusionz;
	  double zstep=(z-*zmax)/nstep;
	  for (double zz=*zmax+zstep;zz<=z;zz+=zstep)
	    os_set_pixel(ih,zz,downcolor);
	}
	*zmax=z;
	return;
      }
      else if (z<*zmin-deltaz){
	if (diffus){
	  drawRectangle(ih,z,1,std::ceil(*zmin-z),diffuse(upcolor,diffusionz));
	  if (!intervalonly)
	    os_set_pixel(ih,z,upcolor);
	}
	else {
	  drawRectangle(ih,z,1,std::ceil(*zmin-z),_BLACK);
	  // draw interval
	  int nstep=int(*zmin-z)/diffusionz;
	  double zstep=(z-*zmin)/nstep; // zstep<0
	  for (double zz=*zmin+zstep;zz>=z;zz+=zstep)
	    os_set_pixel(ih,zz,upcolor);
	}
	*zmin=z;
	return;
      }
    } // end if interval
    if (z>=0 &&  z<=LCD_HEIGHT_PX){
      int color=-1;
      if (diffus){
	if (z<=*zmin){
	  // mark all points with diffuse color from upcolor
	  drawRectangle(ih,z,1,std::ceil(*zmin-z),diffuse(upcolor,std::min(double(diffusionz),std::max(-dz,1.0))));
	  color=upcolor;
	  *zmin=z;
	}
	if (z>=*zmax){
	  // mark all points with diffuse color from downcolor
	  drawRectangle(ih,*zmax,1,std::ceil(z-*zmax),diffuse(downcolor,std::min(double(diffusionz),std::max(dz,1.0))));
	  *zmax=z;
	}
	return;
      }
      if (z>*zmax){ // mark only 1 point
	color=downcolor;
	drawRectangle(ih,*zmax+1,1,z-*zmax-1,_BLACK);
	*zmax=z;
      }
      if (z<*zmin){ // mark 1 point
	color=upcolor;
	// drawRectangle(ih,z+1,1,*zmin-z-1,_BLACK);
	*zmin=z;
      }
      if (color>=0) os_set_pixel(ih,z,color);
    }
  }
  
  void glinter(double a,double b,double c,
	       double xscale,double xc,double yscale,double yc,
	       double *zmin,double *zmax,double ZMIN,double ZMAX,
	       int i,int horiz,int j,int w,int h,int lcdz,
	       int upcolor,int downcolor,int diffusionz,int diffusionz_limit,bool interval
	       ){
    double dz=lcdz*(a+b)*yscale-1;
    //if (dz<-10 || dz>10) cout << "dz=" << dz << "\n";
    // plane equation solved
    if (//0
	h==1 && w==1
	){
      int ih=i+horiz;
      double x = yscale*j-xscale*i + xc;
      // if (x<xmin) continue;
      double y = yscale*j+xscale*i + yc;
      // if (y<ymin) continue;
      double z = (a*x+b*y+c);
      z=LCD_HEIGHT_PX/2+j-lcdz*z;
      if (ZMIN<z && z<ZMAX)
	return;
      bool intervalonly=false;
      // lcdz tests below: avoid marking too large regions
      if (*zmax<*zmin || z<*zmin-lcdz || z>*zmax+lcdz)
	*zmax=*zmin=z;
      if (0 && (*zmax<50 || *zmin<50 || z<50))
	cout << *zmax << " "; // debug
      bool diffus=diffusionz<diffusionz_limit;
      double deltaz;
      if (interval){
	if (z<0) {
	  // return;
	  z=0; intervalonly=true;
	}
	if (z>=LCD_HEIGHT_PX) {
	  // return;
	  z=LCD_HEIGHT_PX-1; intervalonly=true;
	}
	deltaz=diffus?1:diffusionz;
	if (z>*zmax+deltaz){
	  if (diffus){
	    drawRectangle(ih,*zmax,1,std::ceil(z-*zmax),diffuse(downcolor,diffusionz));
	    if (!intervalonly)
	      os_set_pixel(ih,z,downcolor);
	  }
	  else {
	    drawRectangle(ih,*zmax,1,std::ceil(z-*zmax),_BLACK);
	    // draw interval
	    int nstep=int(z-*zmax)/diffusionz;
	    double zstep=(z-*zmax)/nstep;
	    for (double zz=*zmax+zstep;zz<=z;zz+=zstep)
	      os_set_pixel(ih,zz,downcolor);
	  }
	  *zmax=z;
	  return;
	}
	else if (z<*zmin-deltaz){
	  if (diffus){
	    drawRectangle(ih,z,1,std::ceil(*zmin-z),diffuse(upcolor,diffusionz));
	    if (!intervalonly)
	      os_set_pixel(ih,z,upcolor);
	  }
	  else {
	    drawRectangle(ih,z,1,std::ceil(*zmin-z),_BLACK);
	    // draw interval
	    int nstep=int(*zmin-z)/diffusionz;
	    double zstep=(z-*zmin)/nstep; // zstep<0
	    for (double zz=*zmin+zstep;zz>=z;zz+=zstep)
	      os_set_pixel(ih,zz,upcolor);
	  }
	  *zmin=z;
	  return;
	}
      } // end if interval
      if (z>=0 &&  z<=LCD_HEIGHT_PX){
	int color=-1;
	if (diffus){
	  if (z<=*zmin){
	    // mark all points with diffuse color from upcolor
	    drawRectangle(ih,z,1,std::ceil(*zmin-z),diffuse(upcolor,std::min(double(diffusionz),std::max(-dz,1.0))));
	    color=upcolor;
	    *zmin=z;
	  }
	  if (z>=*zmax){
	    // mark all points with diffuse color from downcolor
	    drawRectangle(ih,*zmax,1,std::ceil(z-*zmax),diffuse(downcolor,std::min(double(diffusionz),std::max(dz,1.0))));
	    *zmax=z;
	  }
	  return;
	}
	if (z>=*zmax){ // mark only 1 point
	  color=downcolor;
	  drawRectangle(ih,*zmax+1,1,z-*zmax-1,_BLACK);
	  *zmax=z;
	}
	if (z<=*zmin){ // mark 1 point
	  color=upcolor;
	// drawRectangle(ih,z+1,1,*zmin-z-1,_BLACK);
	  *zmin=z;
	}
	if (color>=0) os_set_pixel(ih,z,color);
      }
      return; // end h==1 and w==1
    }
    for (int I=i;I<i+w;++I,++zmax,++zmin){
      int ih=I+horiz;
      double x = yscale*j-xscale*I + xc;
      // if (x<xmin) continue;
      double y = yscale*j+xscale*I + yc;
      // if (y<ymin) continue;
      double z = (a*x+b*y+c);
      bool intervalonly=false;
      z=LCD_HEIGHT_PX/2+j-lcdz*z;
      if (ZMIN<z && z<ZMAX)
	return;
      if (interval){
	if (z<0) {
	  z=0; intervalonly=true;
	}
	if (z>=LCD_HEIGHT_PX) {
	  z=LCD_HEIGHT_PX-1; intervalonly=true;
	}
      }
      if (i==0)
	; // cout << "i=" << i << " j=" << j << ", zmin=" << *zmin << " z=" << z << " zmax=" << *zmax << " dz=" << dz << ", a=" << a << " b=" << b << " c=" << c <<"\n";
      if (*zmax<*zmin || z<*zmin-lcdz || z>*zmax+lcdz)
	*zmax=*zmin=z;
      int deltaz=(diffusionz<diffusionz_limit?1:diffusionz);
      if (interval && z>*zmax+deltaz){
	if (//0
	    diffusionz<diffusionz_limit
	    )	  
	  drawRectangle(ih,*zmax,1,std::ceil(z-*zmax),diffuse(downcolor,diffusionz));
	else {
	  drawRectangle(ih,*zmax,1,std::ceil(z-*zmax),_BLACK);
	  // draw interval
	  int nstep=int(z-*zmax)/diffusionz;
	  double zstep=(z-*zmax)/nstep;
	  for (double zz=*zmax+zstep;zz<=z;zz+=zstep)
	    os_set_pixel(ih,zz,downcolor);
	}
	if (intervalonly){
	  *zmax=z;
	  continue;
	}
      }
      if (interval && z<*zmin-deltaz){
	if (//0
	    diffusionz<diffusionz_limit
	    )
	  drawRectangle(ih,z,1,std::ceil(*zmin-z),diffuse(upcolor,diffusionz));
	else {
	  drawRectangle(ih,z,1,std::ceil(*zmin-z),_BLACK);
	  // draw interval
	  int nstep=int(*zmin-z)/diffusionz;
	  double zstep=(z-*zmin)/nstep; // zstep<0
	  for (double zz=*zmin+zstep;zz>=z;zz+=zstep)
	    os_set_pixel(ih,zz,upcolor);
	}
	if (intervalonly){
	  *zmin=z;
	  continue;
	}
      }
      if (//x-(h-1)*yscale>xmin && y-(h-1)*yscale>ymin &&
	  z>=0 && z+(h-1)*dz>=0 && z<=LCD_HEIGHT_PX && z+(h-1)*dz<=LCD_HEIGHT_PX
	  ){
	int color=-1;
	if ( (h>1 || diffusionz<diffusionz_limit) && dz>0 && z>=*zmax){
	  // mark all points with downcolor
	  *zmax=z+(h-1)*dz;
	  color=downcolor;
	  if (diffusionz>=diffusionz_limit && dz>diffusionz){
	    drawRectangle(ih,z,1,std::ceil(*zmax-z),_BLACK);
	    // draw interval
	    int nstep=int(std::ceil((*zmax-z)/diffusionz));
	    double zstep=(*zmax-z)/nstep;
	    for (int i=0;i<=nstep;++i)
	      os_set_pixel(ih,z+i*zstep,color);		    
	    continue;
	  }
	}
	if ( (h>1 || diffusionz<diffusionz_limit) && dz<0 && z<=*zmin){
	  // mark all points with upcolor
	  *zmin=z+(h-1)*dz;
	  color=upcolor;
	  if (diffusionz>=diffusionz_limit && dz<-diffusionz){
	    drawRectangle(ih,z,1,std::ceil(z-*zmin),_BLACK);
	    // draw interval
	    int nstep=int(std::ceil((z-*zmin)/diffusionz));
	    double zstep=(*zmin-z)/nstep;
	    for (int i=0;i<=nstep;++i)
	      os_set_pixel(ih,z+i*zstep,color);
	    continue;
	  }
	}
	if (color>=0){
	  if (diffusionz<diffusionz_limit){
	    if (dz>0)
	      drawRectangle(ih,z,1,std::ceil(h*dz),diffuse(color,std::min(double(diffusionz),std::max(dz,1.0))));
	    else
	      drawRectangle(ih,z-std::ceil(-h*dz),1,std::ceil(-h*dz),diffuse(color,std::min(double(diffusionz),std::max(-dz,1.0))));
	    continue;
	  }
	  if (dz>1)
	    drawRectangle(ih,z,1,std::ceil(h*dz),_BLACK);
	  os_set_pixel(ih,z,color);
	  if (h==1) continue;
	  z += dz;
	  os_set_pixel(ih,z,color);
	  if (h==2) continue;
	  z += dz;
	  os_set_pixel(ih,z,color);
	  if (h==3) continue;
	  z += dz;
	  os_set_pixel(ih,z,color);
	  if (h==4) continue;
	  z += dz;
	  os_set_pixel(ih,z,color);
	  if (h==5) continue;
	  z += dz;
	  os_set_pixel(ih,z,color);
	  if (h==6) continue;
	  z += dz;
	  os_set_pixel(ih,z,color);
	  if (h==7) continue;
	  z += dz;
	  os_set_pixel(ih,z,color);
	  if (h==8) continue;
	  z += dz;
	  os_set_pixel(ih,z,color);
	  continue;
	}
	if (dz<=0 && z>=*zmax && z+(h-1)*dz>=*zmin){ // mark only 1 point
	  *zmax=z;
	  color=downcolor;
	}
	if (dz>=0 && z<=*zmin && z+(h-1)*dz<=*zmax){ // mark 1 point
	  *zmin=z;
	  color=upcolor;
	}
	if (color>=0){
	  os_set_pixel(ih,z,color);
	  continue;
	}
      }
      for (int J=0;J<h
	     // && x>=xmin && y>=ymin
	     ;++J,z+=dz,x-=yscale,y-=yscale){
	int color=-1;
	if (z>*zmax){
	  drawRectangle(i,*zmax,1,z-*zmax,_BLACK);
	  *zmax=z;
	  color=downcolor;
	}
	if (z<*zmin){
	  drawRectangle(i,z,1,*zmin-z,_BLACK);
	  *zmin=z;
	  color=upcolor;
	}
	if (z<=-0.5 || z>=LCD_HEIGHT_PX)
	  continue;
	if (color>=0)
	  os_set_pixel(ih,z,color); // drawRectangle(i,z,w,h,color);
      }
    }
  }

  void find_abc(double x1,double x2,double x3,
		double y1,double y2,double y3,
		double z1,double z2,double z3,
		double &a,double &b,double &c){
    // solve([a*x1+b*y1+c=z1,a*x2+b*y2+c=z2,a*x3+b*y3+c=z3],[a,b,c])
    // double d=(x1*y2-x1*y3-x2*y1+x2*y3+x3*y1-x3*y2);
    double d=(x1*(y2-y3)+x2*(y3-y1)+x3*(y1-y2));
    if (d==0) return;
    d=1/d;
    double z12=z2-z1,z23=z3-z2,z31=z1-z3;
    //double a=(-y1*z2+y1*z3+y2*z1-y2*z3-y3*z1+y3*z2)/d;
    a=d*(y1*z23+y2*z31+y3*z12);
    // double b=(x1*z2-x1*z3-x2*z1+x2*z3+x3*z1-x3*z2)/d;
    b=-d*(x1*z23+x2*z31+x3*z12);
    //double c=(x1*y2*z3-x1*y3*z2-x2*y1*z3+x2*y3*z1+x3*y1*z2-x3*y2*z1)/d;
    c=d*(x1*(y2*z3-y3*z2)+x2*(y3*z1-y1*z3)+x3*(y1*z2-y2*z1));
  }

  void glinter(double x1,double x2,double x3,
	       double y1,double y2,double y3,
	       double z1,double z2,double z3,
	       double xscale,double xc,double yscale,double yc,
	       double *zmin,double *zmax,double ZMIN,double ZMAX,
	       int i,int horiz,int j,int w,int h,int lcdz,
	       int upcolor,int downcolor,int diffusionz,int diffusionz_limit,bool interval
	       ){
    double a,b,c;
    find_abc(x1,x2,x3,y1,y2,y3,z1,z2,z3,a,b,c);
    glinter(a,b,c,xscale,xc,yscale,yc,zmin,zmax,ZMIN,ZMAX,i,horiz,j,w,h,lcdz,upcolor,downcolor,diffusionz,diffusionz_limit,interval);
  }
  
  void update12(bool & found,bool &found2,
		double x1,double x2,double x3,double y1,double y2,double y3,double z1,double z2,double z3,
		int upcolor,int downcolor,int downupcolor,int downdowncolor,
		double & curx1, double &curx2, double &curx3, double &cury1, double &cury2, double &cury3, double &curz1, double &curz2, double &curz3, 
		double &cur2x1, double &cur2x2, double &cur2x3, double &cur2y1, double &cur2y2, double &cur2y3, double &cur2z1, double &cur2z2, double &cur2z3,
		int & u,int & d,int & du,int & dd){
    if (found){
      if (z1+z2+z3<curz1+curz2+curz3){
	// no need to update cur, perhaps cur2?
	if (found2 && cur2z1+cur2z2+cur2z3<z1+z2+z3)
	  return;
	found2=true;
	cur2x1=x1; cur2x2=x2; cur2x3=x3;
	cur2y1=y1; cur2y2=y2; cur2y3=y3;
	cur2z1=z1; cur2z2=z2; cur2z3=z3;
	du=downupcolor; dd=downdowncolor;
	return;
      }
      else { // need to update cur, and perhaps cur2
	if (!found2 || curz1+curz2+curz3<cur2z1+cur2z2+cur2z3){
	  found2=true;
	  cur2x1=curx1; cur2x2=curx2; cur2x3=curx3;
	  cur2y1=cury1; cur2y2=cury2; cur2y3=cury3;
	  cur2z1=curz1; cur2z2=curz2; cur2z3=curz3;
	  du=downupcolor; dd=downdowncolor;
	}
	curx1=x1; curx2=x2; curx3=x3;
	cury1=y1; cury2=y2; cury3=y3;
	curz1=z1; curz2=z2; curz3=z3;
	u=upcolor; d=downcolor;
	return;
      }
    }
    found=true;
    curx1=x1; curx2=x2; curx3=x3;
    cury1=y1; cury2=y2; cury3=y3;
    curz1=z1; curz2=z2; curz3=z3;
    u=upcolor; d=downcolor;
  }
	      
  void update12(bool & found,bool &found2,
		double a,double b,double c,double z,
		int upcolor,int downcolor,int downupcolor,int downdowncolor,
		double & cura1,double & curb1,double & curc1,double & curz1,
		double & cura2,double & curb2,double & curc2,double & curz2,
		int & u,int & d,int & du,int & dd){
    if (!found || z>curz1){
      if (found){ // update cur2
	found2=true;
	cura2=cura1; curb2=curb1; curc2=curc1; curz2=curz1;
      }
      found=true;
      cura1=a; curb1=b; curc1=c; curz1=z;
      u=upcolor; d=downcolor;
      return;
    }
    if (z>curz2){
      found2=true;
      cura2=a; curb2=b; curc2=c; curz2=z;
      du=downupcolor; dd=downdowncolor;
    }
  }
	      
  
  // 3d demo prototype
  void do_transform(const double mat[16],double x,double y,double z,double & X,double & Y,double &Z){
    X=mat[0]*x+mat[1]*y+mat[2]*z+mat[3];
    Y=mat[4]*x+mat[5]*y+mat[6]*z+mat[7];
    Z=mat[8]*x+mat[9]*y+mat[10]*z+mat[11];
    // double t=mat[12]*x+mat[13]*y+mat[14]*z+mat[15];
    // X/=t; Y/=t; Z/=t;
  }

#if 1
  bool inside(const vector<double3> & v,double x,double y){
    int n=0;
    for (int i=1;i<v.size();++i){
      const double3 & prev=v[i-1];
      const double3 & cur=v[i];
      double prevx=prev.x,prevy=prev.y,curx=cur.x,cury=cur.y,m=cur.z;
      if (prevx==curx){
	if (x==curx && (y-prevy)*(cury-y)>0) // on vertical edge
	  return false;
	continue;
      }
      if (x==prevx) 
	continue;
      if ((x-prevx)*(curx-x)<0)
	continue;
      //double Y=cury+m*(x-curx);
      double Y=cury+(cur.y-prev.y)/(cur.x-prev.x)*(x-curx);
      if (Y>=y)
	++n; 
    }
    if (n%2)
      return true;
    return false;
  }
#else
  bool inside(const vector<double3> & v,double x,double y){
    int n=0;
    for (int i=1;i<v.size();++i){
      double3 prev=v[i-1],cur=v[i];
      double prevx=prev.x,prevy=prev.y,curx=cur.x,cury=cur.y,m=cur.z;
      if (prevx==curx){
	if (prevx!=x) continue;
	if (y==cury || (y-prevy)*(cury-y)>0)
	  ++n;
	continue;
      }
      if (x==prevx || (x-prevx)*(curx-x)<0)
	continue;
      double Y=cury+m*(x-curx);
      if (Y>=y)
	++n; 
    }
    if (n%2)
      return true;
    return false;
  }
#endif
  
  // intersect plane x-y=xy with line m+t*v
  // m.x+t*v.x-m.y-t*v.y=-xy
  double intersect(const double3 & m,const double3 & v,double xy){
    return (-xy+m.y-m.x)/(v.x-v.y);
  }

  // returns true if filled, false otherwise
  bool get_colors(gen attr,int & upcolor,int & downcolor,int & downupcolor,int & downdowncolor){
    if (attr.is_symb_of_sommet(at_pnt)){
      attr=attr[1];
    }
    if (attr.type==_INT_ && (attr.val & 0xffff)!=0){
      upcolor=attr.val &0xffff;
      int color=rgb565to888(upcolor);
      int r=(color&0xff0000)>>16,g=(color & 0xff00)>>8,b=192;
      r >>= 2;
      g >>= 2;
      downcolor=rgb888to565((r<<16)|(g<<8)|b);
      r >>= 1;
      g >>= 1;
      downupcolor=rgb888to565((r<<16)|(g<<8)|b);
      r >>= 2;
      g >>= 2;
      downdowncolor=rgb888to565((r<<16)|(g<<8)|b);
    }
    if (attr.type==_INT_)
      return attr.val & 0x40000000;
    return false;
  }

#define ABC3D

  // 2d coordinates of m+t*v
  void grmtv2ij(const Graph2d & gr,const double3 & m,const double3 & v,double t,int & i,int & j){
    double x=m.x+t*v.x;
    double y=m.y+t*v.y;
    double z=m.z+t*v.z;
    gr.XYZ2ij(double3(x,y,z),i,j);
  }

  const int4 tabcolorcplx[]={
{63488,47104,30720,14336},
{63489,47105,30720,14336},
{63491,47106,30721,14336},
{63492,47107,30722,14337},
{63494,47108,30723,14337},
{63495,47109,30723,14337},
{63497,47110,30724,14338},
{63498,47111,30725,14338},
{63500,47113,30726,14339},
{63501,47114,30726,14339},
{63503,47115,30727,14339},
{63504,47116,30728,14340},
{63506,47117,30729,14340},
{63507,47118,30729,14340},
{63509,47119,30730,14341},
{63510,47120,30731,14341},
{63512,47122,30732,14342},
{63513,47123,30732,14342},
{63515,47124,30733,14342},
{63516,47125,30734,14343},
{63518,47126,30735,14343},
{63519,47127,30735,14343},
{59423,45079,28687,14343},
{57375,43031,28687,14343},
{53279,40983,26639,12295},
{51231,38935,24591,12295},
{47135,34839,22543,10247},
{45087,32791,22543,10247},
{40991,30743,20495,10247},
{38943,28695,18447,8199},
{34847,26647,16399,8199},
{32799,24599,16399,8199},
{28703,22551,14351,6151},
{26655,20503,12303,6151},
{22559,16407,10255,4103},
{20511,14359,10255,4103},
{16415,12311,8207,4103},
{14367,10263,6159,2055},
{10271,8215,4111,2055},
{8223,6167,4111,2055},
{4127,4119,2063,7},
{2079,2071,15,7},
{2079,2071,2063,2055},
{2175,2135,2095,2055},
{2271,2199,2159,2087},
{2367,2263,2191,2119},
{2463,2359,2255,2151},
{2559,2423,2287,2151},
{2655,2487,2351,2183},
{2751,2551,2383,2215},
{2847,2647,2447,2247},
{2943,2711,2479,2247},
{3039,2775,2543,2279},
{3135,2839,2575,2311},
{3231,2935,2639,2343},
{3327,2999,2671,2343},
{3423,3063,2735,2375},
{3519,3127,2767,2407},
{3615,3223,2831,2439},
{3711,3287,2863,2439},
{3807,3351,2927,2471},
{3903,3415,2959,2503},
{3999,3511,3023,2535},
{4063,3575,3055,2535},
{4061,3574,3054,2535},
{4060,3573,3054,2535},
{4058,3572,3053,2534},
{4057,3571,3052,2534},
{4055,3569,3051,2533},
{4054,3568,3051,2533},
{4052,3567,3050,2533},
{4051,3566,3049,2532},
{4049,3565,3048,2532},
{4048,3564,3048,2532},
{4046,3563,3047,2531},
{4045,3562,3046,2531},
{4043,3560,3045,2530},
{4042,3559,3045,2530},
{4040,3558,3044,2530},
{4039,3557,3043,2529},
{4037,3556,3042,2529},
{4036,3555,3042,2529},
{4034,3554,3041,2528},
{4033,3553,3040,2528},
{4032,3552,3040,2528},
{4032,3552,992,480},
{8128,5600,3040,480},
{10176,7648,5088,2528},
{14272,9696,7136,2528},
{16320,11744,7136,2528},
{20416,13792,9184,4576},
{22464,15840,11232,4576},
{26560,19936,13280,6624},
{28608,21984,13280,6624},
{32704,24032,15328,6624},
{34752,26080,17376,8672},
{38848,28128,19424,8672},
{40896,30176,19424,8672},
{44992,32224,21472,10720},
{47040,34272,23520,10720},
{51136,38368,25568,12768},
{53184,40416,25568,12768},
{57280,42464,27616,12768},
{59328,44512,29664,14816},
{63424,46560,31712,14816},
{65472,48608,31712,14816},
{65376,48512,31648,14784},
{65280,48448,31616,14784},
{65184,48384,31552,14752},
{65088,48320,31520,14720},
{64992,48224,31456,14688},
{64896,48160,31424,14688},
{64800,48096,31360,14656},
{64704,48032,31328,14624},
{64608,47936,31264,14592},
{64512,47872,31232,14592},
{64416,47808,31168,14560},
{64320,47744,31136,14528},
{64224,47648,31072,14496},
{64128,47584,31040,14496},
{64032,47520,30976,14464},
{63936,47456,30944,14432},
{63840,47360,30880,14400},
{63744,47296,30848,14400},
{63648,47232,30784,14368},
{63552,47168,30752,14336},
  };
  
  struct hypertriangle_t {
    const int4 * colorptr; // hypersurface color 
    double xmin,xmax,ymin,ymax; // minmax values intersection with plane y-x=Cte
    double a,b,c; // plane equation of triangle
    double zG; // altitude for gravity center 
  }  ; // data struct for hypesurface triangulation cache

#define HYPERQUAD
#ifdef HYPERQUAD
  
  void compute(double yx,double3 * cur,hypertriangle_t & res){
    double xmin=1e307,xmax=-1e307,ymin=1e307,ymax=-1e307;
    for (int l=0;l<4;++l){
      int prev=l==0?3:l-1;
      double3 & d3=cur[prev];
      double x0=d3.x,y0=d3.y,x1=cur[l].x,y1=cur[l].y;
      double yx0=y0-x0,yx1=y1-x1,m=yx1-yx0;
      if (m==0){
	if (yx==yx1){
	  if (x0>xmax) xmax=x0; if (x0<xmin) xmin=x0;
	  if (x1>xmax) xmax=x1; if (x1<xmin) xmin=x1;
	  if (y0>ymax) ymax=y0; if (y0<ymin) ymin=y0;
	  if (y1>ymax) ymax=y1; if (y1<ymin) ymin=y1;
	}
	continue;
      }
      double t=(yx-yx0)/m;
      if (t>=0 && t<=1){
	double X=x0+t*(x1-x0),Y=y0+t*(y1-y0);
	if (X>xmax) xmax=X; if (X<xmin) xmin=X;
	if (Y>ymax) ymax=Y; if (Y<ymin) ymin=Y;
      }
    }
    res.zG=(cur[0].z+cur[1].z+cur[2].z+cur[3].z)/4;
    res.xmin=xmin; res.xmax=xmax; res.ymin=ymin; res.ymax=ymax;
    find_abc(cur[0].x,cur[1].x,cur[2].x,
	     cur[0].y,cur[1].y,cur[2].y,
	     cur[0].z,cur[1].z,cur[2].z,
	     res.a,res.b,res.c);
  }

  
#else
  void compute(double yx,double3 * cur,hypertriangle_t & res){
    res.zG=(cur[0].z+cur[1].z+cur[2].z)/3;
    double xmin=1e307,xmax=-1e307,ymin=1e307,ymax=-1e307;
    for (int l=0;l<3;++l){
      double3 & d3=cur[l?l-1:2];
      double x0=d3.x,y0=d3.y,x1=cur[l].x,y1=cur[l].y;
      double yx0=y0-x0,yx1=y1-x1,m=yx1-yx0;
      if (m==0){
	if (yx==yx1){
	  if (x0>xmax) xmax=x0; if (x0<xmin) xmin=x0;
	  if (x1>xmax) xmax=x1; if (x1<xmin) xmin=x1;
	  if (y0>ymax) ymax=y0; if (y0<ymin) ymin=y0;
	  if (y1>ymax) ymax=y1; if (y1<ymin) ymin=y1;
	}
	continue;
      }
      double t=(yx-yx0)/m;
      if (t>=0 && t<=1){
	double X=x0+t*(x1-x0),Y=y0+t*(y1-y0);
	if (X>xmax) xmax=X; if (X<xmin) xmin=X;
	if (Y>ymax) ymax=Y; if (Y<ymin) ymin=Y;
      }
    }
    res.xmin=xmin; res.xmax=xmax; res.ymin=ymin; res.ymax=ymax;
    find_abc(cur[0].x,cur[1].x,cur[2].x,
	     cur[0].y,cur[1].y,cur[2].y,
	     cur[0].z,cur[1].z,cur[2].z,
	     res.a,res.b,res.c);
  }
#endif
  
  void update_hypertri(const vector<hypertriangle_t> & hypertriangles,double x,double y,
		       bool & found,bool &found2,
		       double3 & curabc1,double & curz1,
		       double3 & curabc2,double & curz2,
		       int & upcolor,int & downcolor,int & downupcolor,int & downdowncolor){
    vector<hypertriangle_t>::const_iterator it=hypertriangles.begin(),itend=hypertriangles.end();
    for (;it!=itend;++it){
      if (x<it->xmin){
	++it;
	if (it==itend) break;
	if (x<it->xmin){
	  ++it;
	  if (it==itend) break;
	  if (x<it->xmin){
	    ++it;
	    if (it==itend) break;
	  }
	}
      }
      else if (x>it->xmax){
	++it;
	if (it==itend) break;
	if (x>it->xmax){
	  ++it;
	  if (it==itend) break;
	  if (x>it->xmax){
	    ++it;
	    if (it==itend) break;
	  }
	}
      }
      const hypertriangle_t & cur=*it;
      if (x<cur.xmin || x>cur.xmax || y<cur.ymin || y>cur.ymax)
	continue;
      if (!found || cur.zG>curz1){
	if (found){
	  found2=true;
	  curabc2=curabc1;
	  curz2=curz1;
	}
	found=true;
	curabc1.x=cur.a; curabc1.y=cur.b; curabc1.z=cur.c;
	curz1=cur.zG;
	upcolor=cur.colorptr->u; downcolor=cur.colorptr->d;
	continue;
      }
      if (cur.zG>curz2){
	found2=true;
	curabc2.x=cur.a; curabc2.y=cur.b; curabc2.z=cur.c;
	curz2=cur.zG;
	downupcolor=cur.colorptr->du; downdowncolor=cur.colorptr->dd;
	continue;
      }
    } // end loop on k
  }

  struct float2 {
    float f,a;
  } ;
  double absarg(const gen & g,double & argcolor){
    if (g.type==_DOUBLE_){
      double d=g._DOUBLE_val;
      if (d>=0){ argcolor=0;  return d; }
      argcolor=M_PI; return -d;
    }
    double x=g._CPLXptr->_DOUBLE_val,y=(g._CPLXptr+1)->_DOUBLE_val;
    argcolor=std::atan2(x,y);
    double n=std::sqrt(x*x+y*y); // will be encoded in a float, no overflow care
    return n;
  }

  // hpersurface encoded as a matrix
  // with lines containing 3 coordinates per point
  bool Graph2d::glsurface(int w,int h,int lcdz,GIAC_CONTEXT,
			  int upcolor_,int downcolor_,int downupcolor_,int downdowncolor_)  {
    if (w>9) w=9; if (w<1) w=1;
    if (h>9) h=9; if (h<1) h=1;
    // save zmin/zmax on the stack (4K required)
    const int jmintabsize=512;
    short int *jmintab=(short int *)alloca(jmintabsize*sizeof(short int)), * jmaxtab=(short int *)alloca(jmintabsize*sizeof(short int)); // assumes LCD_WIDTH_PX<=jmintabsize
    for (int i=0;i<jmintabsize;++i){
      jmintab[i]=LCD_HEIGHT_PX;
      jmaxtab[i]=0;
    }
    vecteur attrv(gen2vecteur(g));
    std::vector< std::vector< vector<float3d> >::const_iterator > hypv; // 3 iterateurs per hypersurface
    int upcolor,downcolor,downupcolor,downdowncolor;
    for (int i=0;i<int(attrv.size());++i){
      gen attr=attrv[i];
      upcolor=upcolor_;downcolor=downcolor_;downupcolor=downupcolor_;downdowncolor=downdowncolor_;
      get_colors(attrv[i],upcolor,downcolor,downupcolor,downdowncolor);
    }
    for (int i=0;i<int(surfacev.size());++i){
      hypv.push_back(surfacev[i].begin());
      hypv.push_back(surfacev[i].end());
    }
    int horiz=LCD_WIDTH_PX/2,vert=horiz/2;//LCD_HEIGHT_PX/2;
    int imin=Ai,imax=Ai,itmp;
    // 12 segments from cube visualization
    double segments_x1[12]={Ai,Bi,Ei,Fi,Ai,Bi,Ci,Di,Ai,Ci,Ei,Gi};
    double segments_x2[12]={Ci,Di,Gi,Hi,Ei,Fi,Gi,Hi,Bi,Di,Fi,Hi};
    double segments_y1[12]={Aj,Bj,Ej,Fj,Aj,Bj,Cj,Dj,Aj,Cj,Ej,Gj};
    double segments_y2[12]={Cj,Dj,Gj,Hj,Ej,Fj,Gj,Hj,Bj,Dj,Fj,Hj};
    double segments_m[12];
    for (int i=0;i<12;++i){
      segments_m[i]=(segments_y2[i]-segments_y1[i])/(segments_x2[i]-segments_x1[i]);
      itmp=segments_x1[i];
      if (itmp<imin) imin=itmp; if (itmp>imax) imax=itmp;
      itmp=segments_x2[i];
      if (itmp<imin) imin=itmp; if (itmp>imax) imax=itmp;      
    }
    double xmin=-1,ymin=-1,xmax=1,ymax=1,xscale=0.6*(xmax-xmin)/horiz,yscale=0.6*(ymax-ymin)/vert,x,y,z,xc=(xmin+xmax)/2,yc=(ymin+ymax)/2;
    drawRectangle(0,0,imin,LCD_HEIGHT_PX,COLOR_BLACK); // clear    
    drawRectangle(imax,0,LCD_WIDTH_PX-imax,LCD_HEIGHT_PX,COLOR_BLACK); // clear
    sync_screen();
    int count=0;
    vector<int> polyedrei; polyedrei.reserve(polyedrev.size()); // cache for polyedres polygons edges
    vector<double> polyedrexmin,polyedrexmax,polyedreymin,polyedreymax;
    polyedrexmin.reserve(polyedrev.size());polyedrexmax.reserve(polyedrev.size());
    polyedreymin.reserve(polyedrev.size());polyedreymax.reserve(polyedrev.size());
    vector<hypertriangle_t> hypertriangles;
    for (int i=imin-horiz;i<imax-horiz;i+=w,++count){
    //for (int i=-horiz;i<horiz;i+=w){
#ifdef NSPIRE_NEWLIB
      if (count%16==15)
	sync_screen();
      control_c();
      if (ctrl_c || interrupted){
	// w+=2; h+=2;
	ctrl_c=interrupted=false;
	return true;
      }
#endif
#if defined NUMWORKS && defined DEVICE
      if (iskeydown(KEY_CTRL_EXIT))
	return true;
      if (iskeydown(KEY_CTRL_OK)){
	w++; h++;
      }
#endif
      drawRectangle(i+horiz,0,w,LCD_HEIGHT_PX,COLOR_BLACK); // clear
      // find min and max values for j using vertical intersections of line x=i
      // with segments
      int ih=i+horiz+w/2,jmin=RAND_MAX,jmax=-RAND_MAX;
      for (int k=0;k<12;++k){
	if ( !is_inf(segments_m[k]) && (ih-segments_x1[k])*(segments_x2[k]-ih)>=0 ){
	  double y=segments_y1[k]+segments_m[k]*(ih-segments_x1[k]);
	  if (y<=jmin)
	    jmin=std::floor(y);
	  if (y>=jmax)
	    jmax=std::ceil(y);
	}
      }
      if (jmin>jmax) continue;
      if (jmin<0) jmin=0;
      if (jmax>LCD_HEIGHT_PX) jmax=LCD_HEIGHT_PX;
      jmin -= LCD_HEIGHT_PX/2;
      jmax -= LCD_HEIGHT_PX/2;      
      double yx=2*xscale*(i+(w-1)/2.0)+yc-xc;
      // poledrev indices for yx, and xmin/xmax/ymin/ymax values
      // (xmin/xmax should be enough, except limit cases)
      polyedrei.clear(); polyedrexmin.clear(); polyedrexmax.clear(); polyedreymin.clear(); polyedreymax.clear();
      for (int k=0;k<int(polyedrev.size());++k){
	double facemin=polyedre_xyminmax[2*k],facemax=polyedre_xyminmax[2*k+1];
	if (yx<facemin || yx>facemax)
	  continue;
	polyedrei.push_back(k);
	vector<double3> & cur=polyedrev[k];
	double xmin=1e307,xmax=-1e307,ymin=1e307,ymax=-1e307;
	for (int l=0;l<int(cur.size());++l){
	  double3 & d3=cur[l?l-1:cur.size()-1];
	  double x0=d3.x,y0=d3.y,x1=cur[l].x,y1=cur[l].y;
	  double yx0=y0-x0,yx1=y1-x1,m=yx1-yx0;
	  if (m==0){
	    if (yx==yx1){
	      if (x0>xmax) xmax=x0; if (x0<xmin) xmin=x0;
	      if (x1>xmax) xmax=x1; if (x1<xmin) xmin=x1;
	      if (y0>ymax) ymax=y0; if (y0<ymin) ymin=y0;
	      if (y1>ymax) ymax=y1; if (y1<ymin) ymin=y1;
	    }
	    continue;
	  }
	  double t=(yx-yx0)/m;
	  if (t>=0 && t<=1){
	    double X=x0+t*(x1-x0),Y=y0+t*(y1-y0);
	    if (X>xmax) xmax=X; if (X<xmin) xmin=X;
	    if (Y>ymax) ymax=Y; if (Y<ymin) ymin=Y;
	  }
	}
	polyedrexmin.push_back(xmin);
	polyedrexmax.push_back(xmax);
	polyedreymin.push_back(ymin);
	polyedreymax.push_back(ymax);
      }
      // hypersurfaces: find triangles
      hypertriangles.clear();
      double hyperxymax=-1e307,hyperxymin=1e307;
      double3 tri[4]; 
      for (int k=0;k<int(hypv.size());k+=2){
	bool cplx=hyp_color[k].u==0 && hyp_color[k].d==0 && hyp_color[k].du==0 && hyp_color[k].dd==0;
	vector< vector<float3d> >::const_iterator sbeg=hypv[k],send=hypv[k+1],sprec,scur;
	vector<float3d>::const_iterator itprec,itcur,itprecend;
	for (sprec=sbeg,scur=sprec+1;scur<send;++sprec,++scur){
	  itprec=sprec->begin(); 
	  itprecend=sprec->end();
	  itcur=scur->begin();
	  double yx1,yx2=*(itprec+1)-*itprec,yx3,yx4=*(itcur+1)-*itcur;
	  for (itprec+=3,itcur+=3;itprec<itprecend;itprec+=3,itcur+=3){
	    yx1=yx2;
	    yx2=*(itprec+1)-*itprec;
	    yx3=yx4;
	    yx4=*(itcur+1)-*itcur;
	    if (yx<yx1 && yx<yx2 && yx<yx3 && yx<yx4){
	      for (;;){
		// per iteration: 2 incr, 1 test, 2 read, 2 comp, && , test
		itprec+=3;itcur+=3;
		if (itprec<itprecend && yx<(yx2=*(itprec+1)-*itprec) && yx<(yx4=*(itcur+1)-*itcur)){
		  itprec+=3;itcur+=3;
		  if (itprec<itprecend && yx<(yx2=*(itprec+1)-*itprec) && yx<(yx4=*(itcur+1)-*itcur)){
		    itprec+=3;itcur+=3;
		    if (itprec<itprecend && yx<(yx2=*(itprec+1)-*itprec) && yx<(yx4=*(itcur+1)-*itcur)){
		      itprec+=3;itcur+=3;
		      if (itprec<itprecend && yx<(yx2=*(itprec+1)-*itprec) && yx<(yx4=*(itcur+1)-*itcur)){
			continue;
		      }
		    }
		  }
		}
		break;
	      }
	      if (yx<yx2 && yx<yx4) continue;
	    } // end yx<yxk
	    else if (yx>yx1 && yx>yx2 && yx>yx3 && yx>yx4){
	      for (;;){
		// per iteration: 2 incr, 1 test, 2 read, 2 comp, && , test
		itprec+=3;itcur+=3;
		if (itprec<itprecend && yx>(yx2=*(itprec+1)-*itprec) && yx>(yx4=*(itcur+1)-*itcur)){
		  itprec+=3;itcur+=3;
		  if (itprec<itprecend && yx>(yx2=*(itprec+1)-*itprec) && yx>(yx4=*(itcur+1)-*itcur)){
		    itprec+=3;itcur+=3;
		    if (itprec<itprecend && yx>(yx2=*(itprec+1)-*itprec) && yx>(yx4=*(itcur+1)-*itcur)){
		      itprec+=3;itcur+=3;
		      if (itprec<itprecend && yx>(yx2=*(itprec+1)-*itprec) && yx>(yx4=*(itcur+1)-*itcur)){
			continue;
		      }
		    }
		  }
		}
		break;
	      }
	      if (yx>yx2 && yx>yx4) continue;
	    }
	    // found one quad intersecting plane
	    double x1=*(itprec-3),x2=*(itprec),x3=*(itcur-3),x4=*(itcur);
	    double y1=*(itprec-2),y2=*(itprec+1),y3=*(itcur-2),y4=*(itcur+1);
	    double z1=*(itprec-1),z2=*(itprec+2),z3=*(itcur-1),z4=*(itcur+2);
	    double a1,a2,a3,a4;
	    if (cplx){
	      a1 = ((float2 *)&z1)->a;
	      z1 = ((float2 *)&z1)->f;
	      a2 = ((float2 *)&z2)->a;
	      z2 = ((float2 *)&z2)->f;
	      a3 = ((float2 *)&z3)->a;
	      z3 = ((float2 *)&z3)->f;
	      a4 = ((float2 *)&z4)->a;
	      z4 = ((float2 *)&z4)->f;
	    }
	    yx1=y1-x1; yx2=y2-x2; yx3=y3-x3; yx4=y4-x4;
#ifdef HYPERQUAD
	    tri[0]=double3(x1,y1,z1);
	    tri[1]=double3(x2,y2,z2);
	    tri[2]=double3(x4,y4,z4);
	    tri[3]=double3(x3,y3,z3);
	    double x123=(x1+x2+x3+x4)/4,y123=(y1+y2+y3+y4)/4,z123=(z1+z2+z3+z4)/4,X,Y,Z;
	    double xy123=x123+y123;
	    if (xy123<hyperxymin) hyperxymin=xy123;
	    if (xy123>hyperxymax) hyperxymax=xy123;
	    do_transform(invtransform,x123,y123,z123,X,Y,Z);
	    if (Z>=window_zmin && Z<=window_zmax && X>=window_xmin && X<=window_xmax && Y>=window_ymin && Y<=window_ymax ){
	      hypertriangle_t res;
	      if (cplx){
		int idx=(a1+M_PI)*sizeof(tabcolorcplx)/(sizeof(int4)*2*M_PI);
		if (idx<0 || idx >=sizeof(tabcolorcplx)/(sizeof(int4)))
		  idx = 0;
		//CERR << idx << " ";
		res.colorptr=&tabcolorcplx[idx];
	      }
	      else
		res.colorptr=&hyp_color[k];
	      compute(yx,tri,res);
	      hypertriangles.push_back(res);
	    }
#else // HYPERQUAD
	    tri[1]=double3(x2,y2,z2);
	    tri[2]=double3(x3,y3,z3);
	    if ( (yx>yx1 && yx>yx2 && yx>yx3) ||
		 (yx<yx1 && yx<yx2 && yx<yx3) )
	      ; // not intersecting
	    else {
	      double x123=(x1+x2+x3)/3,y123=(y1+y2+y3)/3,z123=(z1+z2+z3)/3,X,Y,Z;
	      double xy123=x123+y123;
	      if (xy123<hyperxymin) hyperxymin=xy123;
	      if (xy123>hyperxymax) hyperxymax=xy123;
	      do_transform(invtransform,x123,y123,z123,X,Y,Z);
	      if (Z>=window_zmin && Z<=window_zmax && X>=window_xmin && X<=window_xmax && Y>=window_ymin && Y<=window_ymax ){
		tri[0]=double3(x1,y1,z1);
		hypertriangle_t res; res.colorptr=&hyp_color[k];
		compute(yx,tri,res);
		hypertriangles.push_back(res);
	      }
	    }
	    if ( (yx>yx4 && yx>yx2 && yx>yx3) ||
		 (yx<yx4 && yx<yx2 && yx<yx3) )
	      ; // not intersecting
	    else {
	      double x423=(x4+x2+x3)/3,y423=(y4+y2+y3)/3,z423=(z4+z2+z3)/3,X,Y,Z;
	      double xy423=x423+y423;
	      if (xy423<hyperxymin) hyperxymin=xy423;
	      if (xy423>hyperxymax) hyperxymax=xy423;
	      do_transform(invtransform,x423,y423,z423,X,Y,Z);
	      if (Z>=window_zmin && Z<=window_zmax && X>=window_xmin && X<=window_xmax && Y>=window_ymin && Y<=window_ymax ){
		tri[0]=double3(x4,y4,z4);
		hypertriangle_t res; res.colorptr=&hyp_color[k];
		compute(yx,tri,res);
		hypertriangles.push_back(res);
	      }
	    }
#endif // HYPERQUAD
	  }
	}
      }
      vector<int> spheres(sphere_centerv.size()); // is plane y-x=yx intersecting sphere, vector<bool> does not work with Keil
      for (int k=0;k<int(sphere_centerv.size());++k){
	const double3 & c=sphere_centerv[k];
	double xc=c.x,yc=c.y;
	double r=sphere_radiusv[k];
	const matrice & m=*sphere_quadraticv[k]._VECTptr;
	const vecteur & m0=*m[0]._VECTptr;
	const vecteur & m1=*m[1]._VECTptr;
	const vecteur & m2=*m[2]._VECTptr;
	double m00=m0[0]._DOUBLE_val,m01=m0[1]._DOUBLE_val,m02=m0[2]._DOUBLE_val,m11=m1[1]._DOUBLE_val,m12=m1[2]._DOUBLE_val,m22=m2[2]._DOUBLE_val;
	/* q0:=m00*x^2+2*m01*x*y+2*m02*x*z+m11*y^2+2*m12*y*z+m22*z^2; q:=subst(q0,[x,y],[x-xc,y-yc]);
	   a,b,c:=coeffs(q(y=yx+x)-r^2,x);
	   delta:=b^2-4*a*c; 
	   // if delta<0 for all z, there is no intersection
	   // delta is a second order polynomial in z, check discriminant
	   A,B,C:=coeffs(delta,z);
	   D:=B^2-4*A*C;  // if D<0 no intersection
	*/
	double A=4*m02*m02+4*m12*m12-4*m00*m22-8*m01*m22+8*m02*m12-4*m11*m22,
	  B=-8*m00*m12*xc+8*m00*m12*yc-8*m00*m12*yx+8*m01*m02*xc-8*m01*m02*yc+8*m01*m02*yx-8*m01*m12*xc+8*m01*m12*yc-8*m01*m12*yx+8*m02*m11*xc-8*m02*m11*yc+8*m02*m11*yx,
	  C=4*m01*m01*xc*xc+4*m01*m01*yc*yc+4*m01*m01*yx*yx-4*m00*m11*xc*xc-4*m00*m11*yc*yc-4*m00*m11*yx*yx-8*m01*m01*xc*yc+8*m01*m01*xc*yx-8*m01*m01*yc*yx+8*m00*m11*xc*yc-8*m00*m11*xc*yx+8*m00*m11*yc*yx+4*m00*r*r+8*m01*r*r+4*m11*r*r,
	  D=B*B-4*A*C;
	spheres[k]=D>=0;
      }
      double zmin[10]={220.220,220,220,220,220,220,220,220,220},
	zmax[10]={0,0,0,0,0,0,0,0,0,0},
	zmin2[10]={220.220,220,220,220,220,220,220,220,220},
	zmax2[10]={0,0,0,0,0,0,0,0,0,0}	; // initialize for these vertical lines
#ifdef ABC3D
      double3 curabc1,curabc2; 
      double curz1=-1e306,curz2=1e306;
#else
      double curx1,curx2,curx3,cury1,cury2,cury3,curz1=-1e306,curz2=-1e306,curz3=-1e306;
      double cur2x1,cur2x2,cur2x3,cur2y1,cur2y2,cur2y3,cur2z1=-1e306,cur2z2=-1e306,cur2z3=-1e306;
#endif
      int u,d,du,dd;
      // loop earlier if there are only hypersurfaces
      bool only_hypertri=true;
      for (int ki=0;ki<int(polyedrei.size());++ki){
	if (polyedrexmin[ki]<=polyedrexmax[ki]){ only_hypertri=false; break; }
      }
      for (int k=0;k<int(sphere_centerv.size());++k){
	if (spheres[k]){ only_hypertri=false; break; }
      }
      for (int k=0;k<int(plan_abcv.size());++k){
	if (plan_filled[k]){ only_hypertri=false; break; }
      }
      if (only_hypertri){
	if (hypertriangles.empty()) goto suite3d;
	int effjmax=(hyperxymax-xc-yc)/yscale/2.0,effjmin=(hyperxymin-xc-yc)/yscale/2.0;
	if (effjmax+1<jmax)
	  jmax=effjmax+1;
	if (effjmin-1>jmin)
	  jmin=effjmin-1;
	x = yscale*(jmax-(h-1)/2.0)-xscale*(i+(w-1)/2.0) + xc;
	y = yscale*(jmax-(h-1)/2.0)+xscale*(i+(w-1)/2.0) + yc;
	for (int j=jmax;j>=jmin;j-=h,x-=yscale*h,y-=yscale*h){
	  bool found=false,found2=false;
	  update_hypertri(hypertriangles,x,y,found,found2,curabc1,curz1,curabc2,curz2,upcolor,downcolor,downupcolor,downdowncolor);
	  if (!found) continue;
	  if (h==1 && w==1){
	    if (found2 && !hide2nd){
	      double dz=lcdz*(curabc2.x+curabc2.y)*yscale-1;
	      // if (y<ymin) continue;
	      double z = (curabc2.x*x+curabc2.y*y+curabc2.z);
	      z=LCD_HEIGHT_PX/2+j-lcdz*z;
	      glinter1(z,dz,
		       zmin2,zmax2,zmin[0],zmax[0],
		       ih,lcdz,
		       downupcolor,downdowncolor,diffusionz,diffusionz_limit,interval);
	    }
	    double dz=lcdz*(curabc1.x+curabc1.y)*yscale-1;
	    // if (y<ymin) continue;
	    double z = (curabc1.x*x+curabc1.y*y+curabc1.z);
	    z=LCD_HEIGHT_PX/2+j-lcdz*z;

	    glinter1(z,dz,
		     zmin,zmax,1e307,-1e307,
		     ih,lcdz,
		     upcolor,downcolor,diffusionz,diffusionz_limit,interval);
	  }
	  else {
	    if (found2 && !hide2nd)
	    glinter(curabc2.x,curabc2.y,curabc2.z,xscale,xc,yscale,yc,zmin2,zmax2,zmin[0],zmax[0],i,horiz,j,w,h,lcdz,downupcolor,downdowncolor,diffusionz,diffusionz_limit,interval);
	    glinter(curabc1.x,curabc1.y,curabc1.z,xscale,xc,yscale,yc,zmin,zmax,1e307,-1e307,i,horiz,j,w,h,lcdz,upcolor,downcolor,diffusionz,diffusionz_limit,interval);
	  }
	}
      }
      else {
	x = yscale*(jmax-(h-1)/2.0)-xscale*(i+(w-1)/2.0) + xc;
	y = yscale*(jmax-(h-1)/2.0)+xscale*(i+(w-1)/2.0) + yc;
	for (int j=jmax;j>=jmin;j-=h,x-=yscale*h,y-=yscale*h){
	  if (0 && i==-35 && j==-44)
	    u=0; // debug
	  // x = yscale*(j-(h-1)/2.0)-xscale*(i+(w-1)/2.0) + xc;
	  // y = yscale*(j-(h-1)/2.0)+xscale*(i+(w-1)/2.0) + yc;
	  bool found=false,found2=false;
	  if (x+y>=hyperxymin && x+y<=hyperxymax)
	    update_hypertri(hypertriangles,x,y,found,found2,curabc1,curz1,curabc2,curz2,upcolor,downcolor,downupcolor,downdowncolor);
	  for (int ki=0;ki<int(polyedrei.size());++ki){
	    int k=polyedrei[ki];
	    vector<double3> & cur=polyedrev[k];
	    if (
#if 1
		x>=polyedrexmin[ki] && x<=polyedrexmax[ki] && y>=polyedreymin[ki] && y<=polyedreymax[ki]
#else
		inside(cur,x,y)
#endif
		){
	      const double3 & abc=polyedre_abcv[k];
	      const int4 & color=polyedre_color[k];
	      // std::cout << k << " " << x << " " << y << " " << color.u << "\n";
	      double a=abc.x,b=abc.y,c=abc.z;
	      z=a*x+b*y+c;
	      bool is_clipped=polyedre_faceisclipped[k];
	      if (!is_clipped){
		double X,Y,Z;
		do_transform(invtransform,x,y,z,X,Y,Z);
		is_clipped=X>=window_xmin && X<=window_xmax && Y>=window_ymin && Y<=window_ymax && Z>=window_zmin && Z<=window_zmax;
	      }
	      if (is_clipped){
#ifdef ABC3D
		update12(found,found2,
			 a,b,c,z,
			 color.u,color.d,color.du,color.dd,
			 curabc1.x,curabc1.y,curabc1.z,curz1,
			 curabc2.x,curabc2.y,curabc2.z,curz2,
			 upcolor,downcolor,downupcolor,downdowncolor);
#else
		update12(found,found2,
			 x-.5,x-.5,x+1,y+0.866,y-0.866,y,z-.5*a+.866*b,z-.5*a-.866*b,z+a,color.u,color.d,color.du,color.dd,
			 curx1,curx2,curx3,cury1,cury2,cury3,curz1,curz2,curz3,
			 cur2x1,cur2x2,cur2x3,cur2y1,cur2y2,cur2y3,cur2z1,cur2z2,cur2z3,
			 upcolor,downcolor,downupcolor,downdowncolor);
#endif
	      }
	    } // end if inside(cur,x,y)
	  }
	  for (int k=0;k<int(sphere_centerv.size());++k){
	    if (!spheres[k]) continue;
	    const double3 & c=sphere_centerv[k];
	    double R=sphere_radiusv[k];
	    const matrice & m=*sphere_quadraticv[k]._VECTptr;
	    const vecteur & m0=*m[0]._VECTptr;
	    const vecteur & m1=*m[1]._VECTptr;
	    const vecteur & m2=*m[2]._VECTptr;
	    double v0=x-c.x,v1=y-c.y;
	    double a=m2[2]._DOUBLE_val,b=2*(m0[2]._DOUBLE_val*v0+m1[2]._DOUBLE_val*v1),C=(m0[0]._DOUBLE_val*v0+2*m0[1]._DOUBLE_val*v1)*v0+m1[1]._DOUBLE_val*v1*v1-R*R;
	    double delta=b*b-4*a*C;
	    if (delta<0)
	      continue;
	    const int4 & color=sphere_color[k];
	    delta=std::sqrt(delta);
	    double sol1,sol2;
	    if (b>0){
	      sol1=(-b-delta)/2/a;
	      sol2=2*C/(-b-delta); // (-b+delta)/2/a;
	    }
	    else {
	      sol1=2*C/(-b+delta);//(-b-delta)/2/a;
	      sol2=(-b+delta)/2/a;
	    }
	    double v2=sol1;
	    z=v2+c.z;
	    bool is_clipped=sphere_isclipped[k];
	    if (!is_clipped){
	      double X,Y,Z;
	      do_transform(invtransform,x,y,z,X,Y,Z);
	      is_clipped=X>=window_xmin && X<=window_xmax && Y>=window_ymin && Y<=window_ymax && Z>=window_zmin && Z<=window_zmax;
	    }
	    if (is_clipped){
	      double w0=v0*m0[0]._DOUBLE_val+v1*m1[0]._DOUBLE_val+v2*m2[0]._DOUBLE_val;
	      double w1=v0*m0[1]._DOUBLE_val+v1*m1[1]._DOUBLE_val+v2*m2[1]._DOUBLE_val;
	      double w2=v0*m0[2]._DOUBLE_val+v1*m1[2]._DOUBLE_val+v2*m2[2]._DOUBLE_val;
#ifdef ABC3D
	      double a=-w0/w2,b=-w1/w2,c=z-(a*x+b*y);
	      update12(found,found2,
		       a,b,c,z,
		       color.u,color.d,color.du,color.dd,
		       curabc1.x,curabc1.y,curabc1.z,curz1,
		       curabc2.x,curabc2.y,curabc2.z,curz2,
		       upcolor,downcolor,downupcolor,downdowncolor);
#else
	      update12(found,found2,
		       //x-w2,x,x,y,y,y-w2,z+w0,z,z+w1,
		       x-0.5,x-.5,x+1,y+.866,y-.866,y,z+.5*w0/w2-.866*w1/w2,z+.5*w0/w2+.866*w1/w2,z-w0/w2,
		       color.u,color.d,color.du,color.dd,
		       curx1,curx2,curx3,cury1,cury2,cury3,curz1,curz2,curz3,
		       cur2x1,cur2x2,cur2x3,cur2y1,cur2y2,cur2y3,cur2z1,cur2z2,cur2z3,
		       upcolor,downcolor,downupcolor,downdowncolor);
#endif
	    }
	    if (delta<=0) continue; // delta==0, twice the same point
	    v2=sol2;
	    z=v2+c.z;
	    is_clipped=sphere_isclipped[k];
	    if (!is_clipped){
	      double X,Y,Z;
	      do_transform(invtransform,x,y,z,X,Y,Z);
	      is_clipped=X>=window_xmin && X<=window_xmax && Y>=window_ymin && Y<=window_ymax && Z>=window_zmin && Z<=window_zmax;
	    }
	    if (is_clipped){
	      double w0=v0*m0[0]._DOUBLE_val+v1*m1[0]._DOUBLE_val+v2*m2[0]._DOUBLE_val;
	      double w1=v0*m0[1]._DOUBLE_val+v1*m1[1]._DOUBLE_val+v2*m2[1]._DOUBLE_val;
	      double w2=v0*m0[2]._DOUBLE_val+v1*m1[2]._DOUBLE_val+v2*m2[2]._DOUBLE_val;
#ifdef ABC3D
	      double a=-w0/w2,b=-w1/w2,c=z-(a*x+b*y);
	      update12(found,found2,
		       a,b,c,z,
		       color.u,color.d,color.du,color.dd,
		       curabc1.x,curabc1.y,curabc1.z,curz1,
		       curabc2.x,curabc2.y,curabc2.z,curz2,
		       upcolor,downcolor,downupcolor,downdowncolor);
#else
	      update12(found,found2,
		       //x-w2,x,x,y,y,y-w2,z+w0,z,z+w1,
		       x-0.5,x-.5,x+1,y+.866,y-.866,y,z+.5*w0/w2-.866*w1/w2,z+.5*w0/w2+.866*w1/w2,z-w0/w2,
		       color.u,color.d,color.du,color.dd,
		       curx1,curx2,curx3,cury1,cury2,cury3,curz1,curz2,curz3,
		       cur2x1,cur2x2,cur2x3,cur2y1,cur2y2,cur2y3,cur2z1,cur2z2,cur2z3,
		       upcolor,downcolor,downupcolor,downdowncolor);
#endif
	    }
	  } // end hypersphere loop
	  for (int k=0;k<int(plan_abcv.size());++k){
	    if (!plan_filled[k])
	      continue;
	    double3 abc=plan_abcv[k];
	    int4 color=plan_color[k];
	    // z=a*x+b*y+c
	    double z=abc.x*x+abc.y*y+abc.z,X,Y,Z;
	    do_transform(invtransform,x,y,z,X,Y,Z);
	    if (X>=window_xmin && X<=window_xmax && Y>=window_ymin && Y<=window_ymax && Z>=window_zmin && Z<=window_zmax)
#ifdef ABC3D
	      update12(found,found2,
		       abc.x,abc.y,abc.z,z,
		       color.u,color.d,color.du,color.dd,
		       curabc1.x,curabc1.y,curabc1.z,curz1,
		       curabc2.x,curabc2.y,curabc2.z,curz2,
		       upcolor,downcolor,downupcolor,downdowncolor);
#else
	    update12(found,found2,
		     x-1,x,x,y,y,y+1,z-abc.x,z,z+abc.y,color.u,color.d,color.du,color.dd,
		     curx1,curx2,curx3,cury1,cury2,cury3,curz1,curz2,curz3,
		     cur2x1,cur2x2,cur2x3,cur2y1,cur2y2,cur2y3,cur2z1,cur2z2,cur2z3,
		     upcolor,downcolor,downupcolor,downdowncolor);
#endif
	  } // end hyperplan loop
	  if (found){
#ifdef ABC3D
	    if (found2){
	      if (!hide2nd)
		glinter(curabc2.x,curabc2.y,curabc2.z,xscale,xc,yscale,yc,zmin2,zmax2,zmin[0],zmax[0],i,horiz,j,w,h,lcdz,downupcolor,downdowncolor,diffusionz,diffusionz_limit,interval);
	      glinter(curabc1.x,curabc1.y,curabc1.z,xscale,xc,yscale,yc,zmin,zmax,1e307,-1e307,i,horiz,j,w,h,lcdz,upcolor,downcolor,diffusionz,diffusionz_limit,interval);
	    }
	    else
	      glinter(curabc1.x,curabc1.y,curabc1.z,xscale,xc,yscale,yc,zmin,zmax,1e307,-1e307,i,horiz,j,w,h,lcdz,upcolor,downcolor,diffusionz,diffusionz_limit,interval);
#else
	    if (found2){
	      if (!hide2nd)
		glinter(cur2x1,cur2x2,cur2x3,cur2y1,cur2y2,cur2y3,cur2z1,cur2z2,cur2z3,xscale,xc,yscale,yc,zmin2,zmax2,zmin[0],zmax[0],i,horiz,j,w,h,lcdz,downupcolor,downdowncolor,diffusionz,diffusionz_limit,interval);
	      glinter(curx1,curx2,curx3,cury1,cury2,cury3,curz1,curz2,curz3,xscale,xc,yscale,yc,zmin,zmax,1e307,-1e307,i,horiz,j,w,h,lcdz,upcolor,downcolor,diffusionz,diffusionz_limit,interval);
	    }
	    else
	      glinter(curx1,curx2,curx3,cury1,cury2,cury3,curz1,curz2,curz3,xscale,xc,yscale,yc,zmin,zmax,1e307,-1e307,i,horiz,j,w,h,lcdz,upcolor,downcolor,diffusionz,diffusionz_limit,interval);
#endif
	  }
	  else {
	    //std::cout << "not inside " << i << " " << j << " " << x << " " << y << "\n";	      
	  }
	} // end pixel vertical loop on j
      } // end else only_hypertri
    suite3d:
      // update jmintab/jmaxtab
      if (i+horiz+w<jmintabsize){
	for (int I=0;I<w;++I){
	  jmintab[i+horiz+I]=zmin[I];
	  jmaxtab[i+horiz+I]=zmax[I];
	}
      }
      // now render line/segments/curves: find intersection with plane
      // y-x=yc-xc+xscale*(2*i+I), 0<=I<w
      for (int j=0;j<int(curvev.size());++j){
	vector<double3> & cur=curvev[j];
	int s=cur.size();
	if (s<2) continue;
	int4 color=curve_color[j];
	double xy=yc-xc+xscale*2*i;
	for (int l=0;l<s-1;++l){
	  double3 m=cur[l];
	  double3 n=cur[l+1];
	  double3 v(n.x-m.x,n.y-m.y,n.z-m.z);
	  if (std::abs(v.y-v.x)<1e-4*(std::abs(v.y)+std::abs(v.x))){
	    v.y=v.x+1e-4*(std::abs(v.y)+std::abs(v.x));
	  }
	  double t1=intersect(m,v,xy);
	  if (t1<0 || t1>1)
	    continue;
	  double dt=2*xscale/(v.y-v.x); // di==1
	  double x1=m.x+t1*v.x;
	  double y1=m.y+t1*v.y;
	  double z1=m.z+t1*v.z;
	  double X1,Y1,Z1;
	  do_transform(invtransform,x1,y1,z1,X1,Y1,Z1);
	  if (X1<window_xmin || X1>window_xmax || Y1<window_ymin || Y1>window_ymax || Z1<window_zmin || Z1>window_zmax)
	    continue;
	  z1=LCD_HEIGHT_PX/2-lcdz*z1+(x1+y1)/2/yscale;
	  double dz=-dt*v.z*lcdz+dt*(v.x+v.y)/2/yscale;
	  int horiz=LCD_WIDTH_PX/2;
	  for (int k=0;k<w;++k){
	    double Z1=z1,Z2=z1+dz;
	    z1=Z2;
	    if (Z1>Z2) std::swap(Z1,Z2);
	    // line [ (i+k,Z1), (i+k,Z2) ]
	    if (Z2<zmin[k]){
	      drawRectangle(i+horiz+k,Z1,1,std::ceil(Z2-Z1),color.u);
	      continue;
	    }
	    if (Z1>zmax[k]){
	      drawRectangle(i+horiz+k,Z1,1,std::ceil(Z2-Z1),color.d);
	      continue;
	    }
	    drawRectangle(i+horiz+k,Z1,1,std::ceil(Z2-Z1),color.du);
	    if (Z1<zmin[k])
	      drawRectangle(i+horiz+k,Z1,1,std::ceil(zmin[k]-Z1),color.u);
	    if (Z2>zmax[k])
	      drawRectangle(i+horiz+k,zmax[k],1,std::ceil(Z2-zmax[k]),color.d);
	  }
	} // end l loop on curve discretization
      } // end loop on curves
#ifdef OLD_LINE_RENDERING
      for (int j=0;j<linetypev.size();j++){
	double3 m=linev[2*j];
	double3 v=linev[2*j+1];
	int4 color=line_color[j];
	if (std::abs(v.y-v.x)<1e-4*(std::abs(v.y)+std::abs(v.x))){
	  v.y=v.x+1e-6*(std::abs(v.y)+std::abs(v.x));
	}
	double xy=yc-xc+xscale*2*i;
	double t1=intersect(m,v,xy);
	// for halfline, t1 must be >=0, for segments between 0 and 1
	if (linetypev[j]==_HALFLINE__VECT && t1<0)
	  continue;
	if (linetypev[j]==_GROUP__VECT && (t1<0 || t1>1))
	  continue;
	double dt=2*xscale/(v.y-v.x); // di==1
	double x1=m.x+t1*v.x;
	double y1=m.y+t1*v.y;
	double z1=m.z+t1*v.z;
	double X1,Y1,Z1;
	do_transform(invtransform,x1,y1,z1,X1,Y1,Z1);
	// int dbgi,dbgj; xyz2ij(double3(x1,y1,z1),dbgi,dbgj);
	/// double x2=x1+dt*v.x,y2=y1+dt*v.y,z2=z1+dt*v.z;
	if (X1<window_xmin || X1>window_xmax || Y1<window_ymin || Y1>window_ymax || Z1<window_zmin || Z1>window_zmax)
	  continue;
	z1=LCD_HEIGHT_PX/2-lcdz*z1+(x1+y1)/2/yscale;
	double dz=-dt*v.z*lcdz+dt*(v.x+v.y)/2/yscale;
	int horiz=LCD_WIDTH_PX/2;
	for (int k=0;k<w;++k){
	  double Z1=z1,Z2=z1+dz;
	  z1=Z2;
	  if (Z1>Z2) std::swap(Z1,Z2);
	  // line [ (i+k,Z1), (i+k,Z2) ]
	  if (Z2<zmin[k]){
	    drawRectangle(i+horiz+k,Z1,1,std::ceil(Z2-Z1),color.u);
	    continue;
	  }
	  if (Z1>zmax[k]){
	    drawRectangle(i+horiz+k,Z1,1,std::ceil(Z2-Z1),color.d);
	    continue;
	  }
	  drawRectangle(i+horiz+k,Z1,1,std::ceil(Z2-Z1),color.du);
	  if (Z1<zmin[k])
	    drawRectangle(i+horiz+k,Z1,1,std::ceil(zmin[k]-Z1),color.u);
	  if (Z2>zmax[k])
	    drawRectangle(i+horiz+k,zmax[k],1,std::ceil(Z2-zmax[k]),color.d);	    
	}
      } // end lines rendering
#endif
      // points rendering
      for (int j=0;j<int(pointv.size());++j){
	const double3 & m=pointv[j];
	if (m.x<i+horiz || m.x>=i+horiz+w)
	  continue;
	const int4 & c=point_color[j];
	int k=m.x-i-horiz,color=-1;
	double mz=LCD_HEIGHT_PX/2-lcdz*m.z;
	double dz=(zmax[k]-zmin[k])*1e-3;
	if (mz>=zmax[k]-dz)
	  color=c.u;
	else if (mz<=zmin[k]+dz)
	  color=c.u; // c.d?
	else color=c.du;
	drawRectangle(m.x,m.y,3,3,color);
	if (points[j]){
	  // int dx=RAND_MAX+os_draw_string(-RAND_MAX,0,color,0,points[j],false); // fake print
	  int dx=os_draw_string_small(0,0,color,0,points[j],true); // fake print
	  os_draw_string_small(m.x-dx,m.y,color,0,points[j],false); 
	}
      } // end points rendering
    } // end pixel horizontal loop on i
#ifndef OLD_LINE_RENDERING
    // new line rendering
    for (int j=0;j<int(linetypev.size());j++){
      double mx,my,mz,vx,vy,vz;
      double3 m=linev[2*j];
      do_transform(invtransform,m.x,m.y,m.z,mx,my,mz);
      double3 v=linev[2*j+1];
      do_transform(invtransform,m.x+v.x,m.y+v.y,m.z+v.z,vx,vy,vz);
      vx -= mx; vy -= my; vz -= mz;
      int4 color=line_color[j];
      // find min/max parameter value for hidden/visible
      double tmax=1e306,tmin=-1e306;
      if (linetypev[j]==_HALFLINE__VECT){
	tmin=0;
      }
      if (linetypev[j]==_GROUP__VECT){
	tmin=0;
	tmax=1;
      }
      if (vx!=0){ // intersect with x=window_xmin and x=window_xmax
	// m.x+v.x*t=window_xmin/max
	double t1=(window_xmin-mx)/vx;
	double t2=(window_xmax-mx)/vx;
	if (t1>t2)
	  std::swap<double>(t1,t2);
	if (t1>tmin)
	  tmin=t1;
	if (t2<tmax)
	  tmax=t2;
      }
      if (vy!=0){ // intersect with y=window_ymin and y=window_ymax
	double t1=(window_ymin-my)/vy;
	double t2=(window_ymax-my)/vy;
	if (t1>t2)
	  std::swap<double>(t1,t2);
	if (t1>tmin)
	  tmin=t1;
	if (t2<tmax)
	  tmax=t2;
      }
      if (vz!=0){ // intersect with z=window_zmin and z=window_zmax
	double t1=(window_zmin-mz)/vz;
	double t2=(window_zmax-mz)/vz;
	if (t1>t2)
	  std::swap<double>(t1,t2);
	if (t1>tmin)
	  tmin=t1;
	if (t2<tmax)
	  tmax=t2;
      }
      // find which intersection we want
      bool usetmax=v.x+v.y==0?v.z>=0:v.x+v.y>0; 
      double tmin_=usetmax?tmin:tmax,
	tmax_=usetmax?tmin:tmax;
      for (int k=0;k<int(plan_abcv.size());++k){
	if (!plan_filled[k]) continue;
	// z >= z_plan=a*x+b*y+c where (x,y,z)=m+t*v
	// m.z+t*v.z >= a*m.x+t*a*v.x+b*m.y+t*b*v.y+c
	// t*(v.z-a*v.x-b.v.y) >= a*m.x+b*m.y+c-m.z
	double3 abc=plan_abcv[k];
	double A=v.z-abc.x*v.x-abc.y*v.y,B=abc.x*m.x+abc.y*m.y+abc.z-m.z;
	if (A==0) continue;
	double t=B/A;
	if (t<=tmin || t>=tmax)
	  continue;
	if (tmax_<t)
	  tmax_=t;
	if (tmin_>t)
	  tmin_=t;
      }
      for (int k=0;k<int(sphere_centerv.size());++k){
	// sphere interesect line 
	double3 c=sphere_centerv[k];
	double R=sphere_radiusv[k];
	int4 color=sphere_color[k];
	double cx,cy,cz;
	do_transform(invtransform,c.x,c.y,c.z,cx,cy,cz);
	// (mx+t*vx-cx)^2+(my+t*vy-cy)^2+(mz+t*vz-cz)^2=R^2
	double a=vx*vx+vy*vy+vz*vz,b=(vx*(mx-cx)+vy*(my-cy)+vz*(mz-cz)),C=(mx-cx)*(mx-cx)+(my-cy)*(my-cy)+(mz-cz)*(mz-cz)-R*R;
	double deltaprime=b*b-a*C;
	if (deltaprime>0){
	  deltaprime=std::sqrt(deltaprime);
	  double t1=(-b-deltaprime)/a,t2=(-b+deltaprime)/a;
	  if (t1>t2)
	    std::swap(t1,t2);
	  if (t1<tmin || t1>tmax)
	    t1=t2;
	  if (t2<tmin || t2>tmax)
	    t2=t1;
	  double t=usetmax?t2:t1;
	  if (t<=tmin || t>=tmax)
	    continue;
	  if (tmin_>t)
	    tmin_=t;
	  if (tmax_<t)
	    tmax_=t;	  
	}
      }
      vector<double> interpoly;
      for (int k=0;k<int(polyedre_abcv.size());++k){
	if (!polyedre_filled[k]) continue;
	double3 abc=polyedre_abcv[k];
	double a=abc.x,b=abc.y,c=abc.z;
	// intersect z=a*x+b*y+c with line m+t*v
	// m.z+t*v.z=a*(m.x+t*v.x)+b*(m.y+t*v.y)+c
	double A=v.z-a*v.x-b*v.y,B=a*m.x+b*m.y+c-m.z;
	if (A!=0){
	  double t=B/A;
	  double x=m.x+t*v.x;
	  double y=m.y+t*v.y;
	  if (t>tmin && t<tmax && inside(polyedrev[k],x,y)){
	    interpoly.push_back(t);
	  }
	}
      }
      if (interpoly.size()>=1){
	sort(interpoly.begin(),interpoly.end());
	double t=usetmax?interpoly.back():interpoly.front();
	if (tmin_>t)
	  tmin_=t;
	if (tmax_<t)
	  tmax_=t;
      }
      vecteur sv(gen2vecteur(g));
      for (int k=0;k<int(sv.size());++k){
	gen surf=remove_at_pnt(sv[k]);
	if (surf.is_symb_of_sommet(at_hypersurface)){
	  const vecteur & hyp=*surf._SYMBptr->feuille._VECTptr;
	  if (hyp.size()>2 && !is_undef(hyp[1])){
	    gen eq=hyp[1],vars=hyp[2];
	    if (_is_polynomial(makesequence(eq,vars[0]),contextptr)==1 && _is_polynomial(makesequence(eq,vars[1]),contextptr)==1 && _is_polynomial(makesequence(eq,vars[2]),contextptr)==1){
	      vecteur V(makevecteur(mx+vx*t__IDNT_e,my+vy*t__IDNT_e,mz+vz*t__IDNT_e));
	      gen eqt=subst(eq,vars,V,false,contextptr);
	      gen gradeq=subst(derive(eq,vars,contextptr),vars,V,false,contextptr); // not used
	      vecteur tval_=gen2vecteur(_sort(solve(eqt,t__IDNT_e,0,contextptr),contextptr));
	      int s=tval_.size();
	      vector<double> tval;
	      for (int l=0;l<s;++l){
		gen tg=evalf_double(tval_[l],1,contextptr);
		if (tg.type==_DOUBLE_){
		  double t=tg._DOUBLE_val;
		  if (t>=tmin && t<=tmax){
		    if (tval.size()>1)
		      tval.back()=t;
		    else
		      tval.push_back(t);
		  }
		}
	      }
#if 1
	      if (!tval.empty()){
		double t=usetmax?tval.back():tval.front();
		if (tmin_>t)
		  tmin_=t;
		if (tmax_<t)
		  tmax_=t;
	      }
#else
	      if (tval.size()==1){
		// grad of eq dot [vx,vy.vz] is positive if entering surface
		gen gradt=subst(gradeq,t__IDNT_e,tval[0],false,contextptr);
		gen dotp=dotvecteur(gen2vecteur(gradt),makevecteur(vx,vy,vz));
		if (is_positive(dotp,contextptr)){
		  if (tmin_>tval[0])
		    tmin_=tval[0];
		  tmax_=tmax;
		}
		else {
		  if (tmax_<tval[0])
		    tmax_=tval[0];
		  tmin_=tmin;
		}
	      }
	      else if (tval.size()==2){
		if (tmin_>tval[0])
		  tmin_=tval[0];
		if (tmax_<tval[1])
		  tmax_=tval[1];
	      }
#endif
	    } // end polynomial hypersurface
	  }
	} // end hypersurface	
      }
      int i1,j1,i2,j2;
      grmtv2ij(*this,m,v,tmin,i1,j1);
      grmtv2ij(*this,m,v,tmax,i2,j2);
      if (tmin==tmin_ && tmax==tmax_){
	int curcolor=color.u;
	if (0 && i1<jmintabsize){
	  if (jmaxtab[i1]>=jmintab[i1] && j1>=jmaxtab[i1] )
	    curcolor=color.d;
	}
	drawLine(i1,j1,i2,j2,curcolor);
      }
      else {
	int curcolor=color.d;
	if (0 && i1<jmintabsize){
	  if (jmaxtab[i1]>=jmintab[i1] && j1<jmintab[i1])
	    curcolor=color.u;
	}
	drawLine(i1,j1,i2,j2,curcolor | 0x400000);
      }
      bool name=show_names && lines[j];
      if (tmin<tmin_){
	grmtv2ij(*this,m,v,tmin,i1,j1);
	grmtv2ij(*this,m,v,tmin_,i2,j2);
	int curcolor=color.u;
	if (0 && i1<jmintabsize){
	  if (jmaxtab[i1]>=jmintab[i1] && j1>=jmaxtab[i1] )
	    curcolor=color.d;
	}
	drawLine(i1,j1,i2,j2,curcolor);
	if (name){
	  os_draw_string(i1,j1,color.u,0,lines[j]);
	  name=false;
	}
	if (tmin_!=tmax) os_fill_rect(i2-2,j2-2,4,4,curcolor);
      }
      if (tmax>tmax_){
	grmtv2ij(*this,m,v,tmax_,i1,j1);
	grmtv2ij(*this,m,v,tmax,i2,j2);
	int curcolor=color.u;
	if (0 && i1<jmintabsize){
	  if (jmaxtab[i1]>=jmintab[i1] && j1>=jmaxtab[i1] )
	    curcolor=color.d;
	}
	drawLine(i1,j1,i2,j2,curcolor);
	if (name){
	  os_draw_string(i2,j2,color.u,0,lines[j]);
	  name=false;
	}
	if (tmax_!=tmin) os_fill_rect(i1-2,j1-2,4,4,curcolor);
      }
    }
#endif // OLD_LINE_RENDERING
    sync_screen();
    return true;
  }

  Graph2d::Graph2d(const giac::gen & g_,const giac::context * cptr):window_xmin(gnuplot_xmin),window_xmax(gnuplot_xmax),window_ymin(gnuplot_ymin),window_ymax(gnuplot_ymax),window_zmin(gnuplot_zmin),window_zmax(gnuplot_zmax),g(g_),display_mode(0x45),show_axes(1),show_edges(1),show_names(1),labelsize(16),precision(1),contextptr(cptr),hp(0),npixels(5),couleur(0),nparams(0) {
    tracemode=0; tracemode_n=0; tracemode_i=0;
    current_i=LCD_WIDTH_PX/3;
    current_j=LCD_HEIGHT_PX/3;
    push_depth=current_depth=0;
    diffusionz=5; diffusionz_limit=5; hide2nd=false; interval=false;
    default_upcolor=giac3d_default_upcolor;
    default_downcolor=giac3d_default_downcolor;
    default_downupcolor=giac3d_default_downupcolor;
    default_downdowncolor=giac3d_default_downdowncolor;
    doprecise=false;
    lcdz= LCD_HEIGHT_PX/4;
    is3d=giac::is3d(g);
    //theta_x=theta_y=theta_z=0;
    q=quaternion_double(0,0,0);//rotation_2_quaternion_double(0.707,0.707,0,1); 
    update_scales();
    autoscale(false,!is3d);
    update_rotation();
    if (is3d){
      if (surfacev.empty()){
	// no hypersurface inside, 2 for polyhedron
	precision=1;
      }
    }
  }

  // q=quaternion_double(dragi*180/h(),0,0)*rotation_2_quaternion_double(1,0,0,dragj*180/w())*q;
  
  void Graph2d::zoomx(double d,bool round,bool doupdate){
    double x_center=(window_xmin+window_xmax)/2;
    double dx=(window_xmax-window_xmin);
    if (dx==0)
      dx=gnuplot_xmax-gnuplot_xmin;
    dx *= d/2;
    x_tick = find_tick(dx);
    window_xmin = x_center - dx;
    if (round) 
      window_xmin=int( window_xmin/x_tick -1)*x_tick;
    window_xmax = x_center + dx;
    if (round)
      window_xmax=int( window_xmax/x_tick +1)*x_tick;
    if (doupdate)
      update();
  }

  void Graph2d::zoomy(double d,bool round,bool doupdate){
    double y_center=(window_ymin+window_ymax)/2;
    double dy=(window_ymax-window_ymin);
    if (dy==0)
      dy=gnuplot_ymax-gnuplot_ymin;
    dy *= d/2;
    y_tick = find_tick(dy);
    window_ymin = y_center - dy;
    if (round)
      window_ymin=int( window_ymin/y_tick -1)*y_tick;
    window_ymax = y_center + dy;
    if (round)
      window_ymax=int( window_ymax/y_tick +1)*y_tick;
    if (doupdate)
      update();
  }

  void Graph2d::zoomz(double d,bool round,bool doupdate){
    double z_center=(window_zmin+window_zmax)/2;
    double dz=(window_zmax-window_zmin);
    if (dz==0)
      dz=gnuplot_zmax-gnuplot_zmin;
    dz *= d/2;
    z_tick = find_tick(dz);
    window_zmin = z_center - dz;
    if (round) 
      window_zmin=int( window_zmin/z_tick -1)*z_tick;
    window_zmax = z_center + dz;
    if (round)
      window_zmax=int( window_zmax/z_tick +1)*z_tick;
    if (doupdate)
      update();
  }

  void Graph2d::zoom(double d,bool doupdate){ 
    zoomx(d,false,false);
    zoomy(d,false,false);
    zoomz(d,false,false);
    if (doupdate)
      update();
  }

  void Graph2d::autoscale(bool fullview,bool doupdate){
    // Find the largest and lowest x/y/z in objects (except lines/plans)
    vector<double> vx,vy,vz;
    int s;
    bool ortho=autoscaleg(g,vx,vy,vz,contextptr);
    autoscaleminmax(vx,window_xmin,window_xmax,fullview);
    double zf=is3d?1.03:1+1e-14;
    zoomx(zf,false,false);
    autoscaleminmax(vy,window_ymin,window_ymax,fullview);
    zoomy(zf,false,false);
    autoscaleminmax(vz,window_zmin,window_zmax,fullview);
    zoomz(zf,false,false);
    if (window_xmax-window_xmin<1e-100){
      window_xmax=gnuplot_xmax;
      window_xmin=gnuplot_xmin;
    }
    if (window_ymax-window_ymin<1e-100){
      window_ymax=gnuplot_ymax;
      window_ymin=gnuplot_ymin;
    }
    if (window_zmax-window_zmin<1e-100){
      window_zmax=gnuplot_zmax;
      window_zmin=gnuplot_zmin;
    }
    bool do_ortho=ortho;
    if (!do_ortho){
      double w=LCD_WIDTH_PX;
      double h=LCD_HEIGHT_PX-STATUS_AREA_PX;
      double window_w=window_xmax-window_xmin,window_h=window_ymax-window_ymin;
      double tst=h/w*window_w/window_h;
      double tst2=(window_xmax-window_zmin)/window_h;
      if (tst>0.7 && tst<1.4 && (!is3d || (tst2>0.7 && tst2<1.4))){
	do_ortho=true;
      }
    }
    if (do_ortho )
      orthonormalize(false);
    y_tick=find_tick(window_ymax-window_ymin);
    if (doupdate)
      update();
  }

  void Graph2d::orthonormalize(bool doupdate){
    // Center of the directions, orthonormalize
    double w=LCD_WIDTH_PX;
    double h=LCD_HEIGHT_PX-STATUS_AREA_PX;
    double window_w=window_xmax-window_xmin,window_h=window_ymax-window_ymin;
    if (is3d){
      double window_xcenter=(window_xmin+window_xmax)/2;
      double window_ycenter=(window_ymin+window_ymax)/2;
      double window_zcenter=(window_zmin+window_zmax)/2;
      double window_z=window_zmax-window_zmin;
      double wmax=std::max(window_w,std::max(window_h,window_z));
      window_xmin=window_xcenter-wmax/2;
      window_xmax=window_xcenter+wmax/2;
      window_ymin=window_ycenter-wmax/2;
      window_ymax=window_ycenter+wmax/2;
      window_zmin=window_zcenter-wmax/2;
      window_zmax=window_zcenter+wmax/2;
      z_tick=find_tick(window_zmax-window_zmin);
    }
    else {
      double window_hsize=h/w*window_w;
      if (window_h > window_hsize*1.01){ // enlarge horizontally
	double window_xcenter=(window_xmin+window_xmax)/2;
	double window_wsize=w/h*window_h;
	window_xmin=window_xcenter-window_wsize/2;
	window_xmax=window_xcenter+window_wsize/2;
      }
      if (window_h < window_hsize*0.99) { // enlarge vertically
	double window_ycenter=(window_ymin+window_ymax)/2;
	window_ymin=window_ycenter-window_hsize/2;
	window_ymax=window_ycenter+window_hsize/2;
      }
    }
    x_tick=find_tick(window_xmax-window_xmin);
    y_tick=find_tick(window_ymax-window_ymin);
    if (doupdate)
      update();
  }

  void Graph2d::update_scales(){
    if (is3d){
      x_scale=1.414*(LCD_HEIGHT_PX-STATUS_AREA_PX)/(window_xmax-window_xmin);    
      y_scale=1.414*(LCD_HEIGHT_PX-STATUS_AREA_PX)/(window_ymax-window_ymin);    
      z_scale=(LCD_HEIGHT_PX-STATUS_AREA_PX)/(window_zmax-window_zmin);    
    }
    else {
      x_scale=LCD_WIDTH_PX/(window_xmax-window_xmin);    
      y_scale=(LCD_HEIGHT_PX-STATUS_AREA_PX)/(window_ymax-window_ymin);    
    }
  }

  vecteur mark_selected(const vecteur & v,const vector<int> & selected,bool is3d){
    vecteur w(v);
    vector<int> s(selected); sort(s.begin(),s.end());
    int pos=0;
    for (int i=0;i<w.size();++i){
      if (pos>=s.size())
        break;
      if (i==s[pos]){
        ++pos;
        gen g=w[i];
        if (g.is_symb_of_sommet(at_pnt)){
          g=g._SYMBptr->feuille;
          if (g.type==_VECT && g._VECTptr->size()>=2){
            vecteur gv(*g._VECTptr);
#ifdef HP39
            gv[1]=4<<22;
#else
            gv[1]=is3d?_CYAN:_BLUE;
#endif
            g=gen(gv,g.subtype);
            w[i]=symbolic(at_pnt,g);
          }
        }
      }
    }
    return w;
  }

  vecteur Graph2d::selected_names(bool allobjects,bool withdef) const {
    vector<int>::const_iterator it=selected.begin(),itend=selected.end();
    vecteur res;
    for (;it!=itend;++it){
      gen g=symbolic_instructions[*it];
      if (g.is_symb_of_sommet(at_sto)){
	gen tmp=g._SYMBptr->feuille[0];
	if (allobjects || tmp.is_symb_of_sommet(at_point) || tmp.is_symb_of_sommet(at_element))
	  res.push_back(withdef?g:g._SYMBptr->feuille[1]);
      }
    }
    return res;
  }

  void Graph2d::adjust_cursor_point_type(){
    if (hp){
      double newx,newy,newz;
      find_xyz(current_i,current_j,current_depth,newx,newy,newz);
      int pos=-1;
      gen orig;
      gen res=geometry_round(newx,newy,newz,find_eps(),orig,pos);
      if (mode==0){
	if (pos>=0)
	  selected=vector<int>(1,pos);
	else
	  selected.clear();
      }
      cursor_point_type=pos>=0?6:3;
    }
  }

  void Graph2d::update_g(){
    if (hp){
      adjust_cursor_point_type();
      find_title_plot(title_tmp,plot_tmp);
      vecteur v(mergevecteur(get_current_animation(),trace_instructions));
      if (!is_undef(plot_tmp)) v.push_back(plot_tmp);
      // geometry: update g from plot_instructions
      g=mergevecteur(selected.empty()?plot_instructions:mark_selected(plot_instructions,selected,is3d),v);
      if (is3d)
	update_rotation();
    }
  }    

  void Graph2d::update(){
    update_g();
    update_scales();
    update_rotation();
  }
  
  void mult4(double * c,double k,double * res){
    for (int i=0;i<16;i++)
      res[i]=k*c[i];
  }
  
  double det4(double * c){
    return c[0]*c[5]*c[10]*c[15]-c[0]*c[5]*c[14]*c[11]-c[0]*c[9]*c[6]*c[15]+c[0]*c[9]*c[14]*c[7]+c[0]*c[13]*c[6]*c[11]-c[0]*c[13]*c[10]*c[7]-c[4]*c[1]*c[10]*c[15]+c[4]*c[1]*c[14]*c[11]+c[4]*c[9]*c[2]*c[15]-c[4]*c[9]*c[14]*c[3]-c[4]*c[13]*c[2]*c[11]+c[4]*c[13]*c[10]*c[3]+c[8]*c[1]*c[6]*c[15]-c[8]*c[1]*c[14]*c[7]-c[8]*c[5]*c[2]*c[15]+c[8]*c[5]*c[14]*c[3]+c[8]*c[13]*c[2]*c[7]-c[8]*c[13]*c[6]*c[3]-c[12]*c[1]*c[6]*c[11]+c[12]*c[1]*c[10]*c[7]+c[12]*c[5]*c[2]*c[11]-c[12]*c[5]*c[10]*c[3]-c[12]*c[9]*c[2]*c[7]+c[12]*c[9]*c[6]*c[3];
  }

  void inv4(double * c,double * res){
    res[0]=c[5]*c[10]*c[15]-c[5]*c[14]*c[11]-c[10]*c[7]*c[13]-c[15]*c[9]*c[6]+c[14]*c[9]*c[7]+c[11]*c[6]*c[13];
    res[1]=-c[1]*c[10]*c[15]+c[1]*c[14]*c[11]+c[10]*c[3]*c[13]+c[15]*c[9]*c[2]-c[14]*c[9]*c[3]-c[11]*c[2]*c[13];
    res[2]=c[1]*c[6]*c[15]-c[1]*c[14]*c[7]-c[6]*c[3]*c[13]-c[15]*c[5]*c[2]+c[14]*c[5]*c[3]+c[7]*c[2]*c[13];
    res[3]=-c[1]*c[6]*c[11]+c[1]*c[10]*c[7]+c[6]*c[3]*c[9]+c[11]*c[5]*c[2]-c[10]*c[5]*c[3]-c[7]*c[2]*c[9];
    res[4]=-c[4]*c[10]*c[15]+c[4]*c[14]*c[11]+c[10]*c[7]*c[12]+c[15]*c[8]*c[6]-c[14]*c[8]*c[7]-c[11]*c[6]*c[12];
    res[5]=c[0]*c[10]*c[15]-c[0]*c[14]*c[11]-c[10]*c[3]*c[12]-c[15]*c[8]*c[2]+c[14]*c[8]*c[3]+c[11]*c[2]*c[12];
    res[6]=-c[0]*c[6]*c[15]+c[0]*c[14]*c[7]+c[6]*c[3]*c[12]+c[15]*c[4]*c[2]-c[14]*c[4]*c[3]-c[7]*c[2]*c[12];
    res[7]=c[0]*c[6]*c[11]-c[0]*c[10]*c[7]-c[6]*c[3]*c[8]-c[11]*c[4]*c[2]+c[10]*c[4]*c[3]+c[7]*c[2]*c[8];
    res[8]=c[4]*c[9]*c[15]-c[4]*c[13]*c[11]-c[9]*c[7]*c[12]-c[15]*c[8]*c[5]+c[13]*c[8]*c[7]+c[11]*c[5]*c[12];
    res[9]=-c[0]*c[9]*c[15]+c[0]*c[13]*c[11]+c[9]*c[3]*c[12]+c[15]*c[8]*c[1]-c[13]*c[8]*c[3]-c[11]*c[1]*c[12];
    res[10]=c[0]*c[5]*c[15]-c[0]*c[13]*c[7]-c[5]*c[3]*c[12]-c[15]*c[4]*c[1]+c[13]*c[4]*c[3]+c[7]*c[1]*c[12];
    res[11]=-c[0]*c[5]*c[11]+c[0]*c[9]*c[7]+c[5]*c[3]*c[8]+c[11]*c[4]*c[1]-c[9]*c[4]*c[3]-c[7]*c[1]*c[8];
    res[12]=-c[4]*c[9]*c[14]+c[4]*c[13]*c[10]+c[9]*c[6]*c[12]+c[14]*c[8]*c[5]-c[13]*c[8]*c[6]-c[10]*c[5]*c[12];
    res[13]=c[0]*c[9]*c[14]-c[0]*c[13]*c[10]-c[9]*c[2]*c[12]-c[14]*c[8]*c[1]+c[13]*c[8]*c[2]+c[10]*c[1]*c[12];
    res[14]=-c[0]*c[5]*c[14]+c[0]*c[13]*c[6]+c[5]*c[2]*c[12]+c[14]*c[4]*c[1]-c[13]*c[4]*c[2]-c[6]*c[1]*c[12];
    res[15]=c[0]*c[5]*c[10]-c[0]*c[9]*c[6]-c[5]*c[2]*c[8]-c[10]*c[4]*c[1]+c[9]*c[4]*c[2]+c[6]*c[1]*c[8];
    double det=det4(c);
    mult4(res,1/det,res);
  }
  
  void Graph2d::xyz2ij(const double3 &d,int &i,int &j) const {
    double X,Y,Z;
    do_transform(transform,d.x,d.y,d.z,X,Y,Z);
    i=LCD_WIDTH_PX/2+(Y-X)/4.8*LCD_WIDTH_PX;
    j=LCD_HEIGHT_PX/2-Z*lcdz+(Y+X)/9.6*LCD_WIDTH_PX;    
  }
  
  void Graph2d::xyz2ij(const double3 &d,double &i,double &j) const {
    double X,Y,Z;
    do_transform(transform,d.x,d.y,d.z,X,Y,Z);
    i=LCD_WIDTH_PX/2+(Y-X)/4.8*LCD_WIDTH_PX;
    j=LCD_HEIGHT_PX/2-Z*lcdz+(Y+X)/9.6*LCD_WIDTH_PX;    
  }
  
  void Graph2d::xyz2ij(const double3 &d,double &i,double &j,double3 & d3) const {
    do_transform(transform,d.x,d.y,d.z,d3.x,d3.y,d3.z);
    i=LCD_WIDTH_PX/2+(d3.y-d3.x)/4.8*LCD_WIDTH_PX;
    j=LCD_HEIGHT_PX/2-d3.z*lcdz+(d3.y+d3.x)/9.6*LCD_WIDTH_PX;    
  }
  
  void Graph2d::XYZ2ij(const double3 &d,int &i,int &j) const {
    double X=d.x,Y=d.y,Z=d.z;
    i=LCD_WIDTH_PX/2+(Y-X)/4.8*LCD_WIDTH_PX;
    j=LCD_HEIGHT_PX/2-Z*lcdz+(Y+X)/9.6*LCD_WIDTH_PX;    
  }

  void Graph2d::update_rotation(){
    if (!is3d)
      return;
    solid3d=false;
    double rx,ry,rz,theta;
    get_axis_angle_deg(q,rx,ry,rz,theta);
    // rx=-0.51; ry=-.197; rz=-.835; theta=327.88;
    // rx=0;ry=1;rz=0; //theta=60;
    // cout << rx << " " << ry << " " << rz << " " << theta << "\n";
    // rx=0;ry=0;rz=1;
    // theta=10;
    theta *= M_PI/180;
    double r2=rx*rx+ry*ry+rz*rz,invr2=1/r2;
    double r=std::sqrt(r2);
    double c=std::cos(theta),s=std::sin(theta);
    // mkisom([[rx,ry,rz],theta],1)
    // 1/r2*[[rx*rx+ry*ry*c+rz*rz*c,rx*ry-rx*ry*c-rz*s*r,rx*rz-rx*rz*c+ry*s*r],[rx*ry-rx*ry*c+rz*s*r,ry*ry+rx*rx*c+rz*rz*c,ry*rz-rx*s*r-ry*rz*c],[rx*rz-rx*rz*c-ry*s*r,ry*rz+rx*s*r-ry*rz*c,rz*rz+rx*rx*c+ry*ry*c]]
    double a11=invr2*(rx*rx+ry*ry*c+rz*rz*c),a12=invr2*(rx*ry-rx*ry*c-rz*s*r),a13=invr2*(rx*rz-rx*rz*c+ry*s*r);
    double a21=invr2*(rx*ry-rx*ry*c+rz*s*r),a22=invr2*(ry*ry+rx*rx*c+rz*rz*c),a23=invr2*(ry*rz-rx*s*r-ry*rz*c);
    double a31=invr2*(rx*rz-rx*rz*c-ry*s*r),a32=invr2*(ry*rz+rx*s*r-ry*rz*c),a33=invr2*(rz*rz+rx*rx*c+ry*ry*c);
    double xt=(window_xmin+window_xmax)/2,xs=2.0/(window_xmax-window_xmin),yt=(window_ymin+window_ymax)/2,ys=2.0/(window_ymax-window_ymin),zt=(window_zmin+window_zmax)/2,zs=2.0/(window_zmax-window_zmin);
    double mat[16]={a11*xs,a12*ys,a13*zs,-a11*xt*xs-a12*yt*ys-a13*zt*zs,
		    a21*xs,a22*ys,a23*zs,-a21*xt*xs-a22*yt*ys-a23*zt*zs,
		    a31*xs,a32*ys,a33*zs,-a31*xt*xs-a32*yt*ys-a33*zt*zs,
		    0,0,0,1
    };
    for (int i=0;i<sizeof(mat)/sizeof(double);++i)
      transform[i]=mat[i];
    inv4(transform,invtransform);
    surfacev.clear();
    polyedrev.clear(); polyedre_xyminmax.clear(); polyedre_abcv.clear(); polyedre_faceisclipped.clear(); polyedre_filled.clear();
    plan_pointv.clear(); plan_abcv.clear(); plan_filled.clear();
    sphere_centerv.clear(); sphere_radiusv.clear(); sphere_quadraticv.clear();
    linev.clear(); linetypev.clear(); curvev.clear();
    pointv.clear(); points.clear();
    plan_color.clear();sphere_color.clear();polyedre_color.clear();polyedre_faceisclipped.clear();line_color.clear();curve_color.clear(); hyp_color.clear(); point_color.clear();
    // rotate+translate+scale g
    vecteur v;
    aplatir(gen2vecteur(g),v);
    for (int i=0;i<v.size();++i){
      int u=default_upcolor,d=default_downcolor,du=default_downupcolor,dd=default_downdowncolor;
      const char * ptr=0;
      bool fill_polyedre=false;
      if (v[i].is_symb_of_sommet(at_pnt)){
	vecteur & attrv=*v[i]._SYMBptr->feuille._VECTptr;
	if (attrv.size()>1){
	  gen attr=attrv[1];
	  fill_polyedre=get_colors(attr,u,d,du,dd);
	  if (fill_polyedre) solid3d=true;
	  if (attrv.size()>2){
	    attr=attrv[2];
	    if (attr.type==_STRNG)
	      ptr=attr._STRNGptr->c_str();
	    if (attr.type==_IDNT)
	      ptr=attr._IDNTptr->id_name;
	  }
	}
      }
      gen G=remove_at_pnt(v[i]);
      if (G.is_symb_of_sommet(at_curve)){
	gen f=G._SYMBptr->feuille;
	f=f[0];
	f=f[4];
	if (ckmatrix(f)){
	  vecteur v=*f._VECTptr;
	  int n=v.size();
	  curvev.push_back(vector<double3>(n));
	  curve_color.push_back(int4(u,d,du,dd));
	  vector<double3> & cur=curvev.back();
	  for (int k=0;k<n;++k){
	    gen P=v[k];
	    if (P.type==_VECT && P._VECTptr->size()==3){
	      double X,Y,Z;
	      do_transform(transform,P[0]._DOUBLE_val,P[1]._DOUBLE_val,P[2]._DOUBLE_val,X,Y,Z);
	      cur[k]=double3(X,Y,Z);
	    }
	  }
	}
	continue;
      }
      if (G.type==_VECT && G.subtype==_POINT__VECT){
	gen m=evalf_double(G,1,contextptr);
	if (m.type==_VECT && m._VECTptr->size()==3){
	  vecteur & A=*m._VECTptr;
	  if (A[0].type==_DOUBLE_ && A[1].type==_DOUBLE_ && A[2].type==_DOUBLE_ ){
	    double X,Y,Z; int I,J;
	    do_transform(transform,A[0]._DOUBLE_val,A[1]._DOUBLE_val,A[2]._DOUBLE_val,X,Y,Z);
	    xyz2ij(double3(A[0]._DOUBLE_val,A[1]._DOUBLE_val,A[2]._DOUBLE_val),I,J);
	    pointv.push_back(double3(I,J,Z));
	    points.push_back(ptr);
	    point_color.push_back(int4(u,d,du,dd));
	  }
	}
	continue;
      }
      bool line=G.subtype==_LINE__VECT,halfline=G.subtype==_HALFLINE__VECT,segment= G.subtype==_GROUP__VECT;
      if (G.type==_VECT && G._VECTptr->size()>=2 && (line || halfline || segment)){
	for (int n=1;n<G._VECTptr->size();++n){
	  gen a=evalf_double((*G._VECTptr)[n-1],1,contextptr),b=evalf_double((*G._VECTptr)[n],1,contextptr);
	  if (a.type==_VECT && b.type==_VECT && a._VECTptr->size()==3 && b._VECTptr->size()==3){
	    vecteur & A=*a._VECTptr;
	    vecteur & B=*b._VECTptr;
	    if (A[0].type==_DOUBLE_ && A[1].type==_DOUBLE_ && A[2].type==_DOUBLE_ && B[0].type==_DOUBLE_ && B[1].type==_DOUBLE_ && B[2].type==_DOUBLE_ ){
	      lines.push_back(ptr);
	      double x=A[0]._DOUBLE_val,y=A[1]._DOUBLE_val,z=A[2]._DOUBLE_val;
#if 0 // ndef OLD_LINE_RENDERING
	      double3 prev(x,y,z);
	      linev.push_back(prev);
#endif
	      double X,Y,Z;
	      do_transform(transform,x,y,z,X,Y,Z);
	      double3 M(X,Y,Z);
	      x=B[0]._DOUBLE_val;y=B[1]._DOUBLE_val;z=B[2]._DOUBLE_val;
#if 0 // ndef OLD_LINE_RENDERING
	      linev.push_back(double3(x-prev.x,y-prev.y,z-prev.z));
#endif
	      do_transform(transform,x,y,z,X,Y,Z);
	      double3 N(X,Y,Z);
	      double3 v(N.x-M.x,N.y-M.y,N.z-M.z);
	      linev.push_back(M); linev.push_back(v);
	      linetypev.push_back(G.subtype);
	      line_color.push_back(int4(u,d,du,dd));
	    }
	  }
	}
	continue;
      }
      if (G.is_symb_of_sommet(at_hypersphere)){
	solid3d=true;
	vecteur hyp=*G._SYMBptr->feuille._VECTptr;
	gen c=evalf_double(hyp[0],1,contextptr);
	double x=c[0]._DOUBLE_val,y=c[1]._DOUBLE_val,z=c[2]._DOUBLE_val,X,Y,Z;
	do_transform(transform,x,y,z,X,Y,Z);
	sphere_centerv.push_back(double3(X,Y,Z));
	gen R=evalf(hyp[1],1,contextptr);
	double r=R._DOUBLE_val;
	sphere_radiusv.push_back(r);
	double * mat=invtransform;
	matrice qmat(makevecteur(
				 makevecteur(mat[0],mat[4],mat[8]),
				 makevecteur(mat[1],mat[5],mat[9]),
				 makevecteur(mat[2],mat[6],mat[10])
				 ));
	qmat=mmult(mtran(qmat),qmat);
	sphere_quadraticv.push_back(qmat);
	sphere_color.push_back(int4(u,d,du,dd));
	bool isclipped=x>=window_xmin+r && x<=window_xmax-r && y>=window_ymin+r && y<=window_ymax-r && z>=window_zmin+r && z<=window_zmax-r;
	// check if distance of center to window_x/y/xmin/max is <=R
	sphere_isclipped.push_back(isclipped);
	continue;
      }
      if (G.is_symb_of_sommet(at_hyperplan)){
	plan_filled.push_back(fill_polyedre);
	vecteur hyp=*G._SYMBptr->feuille._VECTptr;
	gen hyp1=evalf_double(hyp[1],1,contextptr);
	vecteur & hyp1v=*hyp1._VECTptr;
	double A,B,C,x,y,z,X,Y,Z;
	A=hyp1v[0]._DOUBLE_val;B=hyp1v[1]._DOUBLE_val;C=hyp1v[2]._DOUBLE_val;
	do_transform(transform,A,B,C,X,Y,Z);
	gen hyp0=evalf_double(hyp[0],1,contextptr);
	vecteur & hyp0v=*hyp0._VECTptr;
	x=hyp0v[0]._DOUBLE_val;y=hyp0v[1]._DOUBLE_val;z=hyp0v[2]._DOUBLE_val;
	double * mat=invtransform;
	A=mat[0]*x+mat[4]*y+mat[8]*z;
	B=mat[1]*x+mat[5]*y+mat[9]*z;
	C=mat[2]*x+mat[6]*y+mat[10]*z;
	double AB=std::abs(A)+std::abs(B);
	if (std::abs(C)<1e-2*AB){
	  // almost vertical plane, equation A*(x-X)+B*(y-Y)=0
	  continue;
	}
	A/=C; B/=C;
	plan_pointv.push_back(double3(X,Y,Z));	
	plan_abcv.push_back(double3(-A,-B,Z+A*X+B*Y));
	plan_color.push_back(int4(u,d,du,dd));
	continue;
      }
      if (G.is_symb_of_sommet(at_hypersurface)){
	solid3d=true;
	const vecteur & hyp=*G._SYMBptr->feuille._VECTptr;
	gen hyp0=hyp[0];
	const vecteur & hyp0v=*hyp0._VECTptr;
	gen h=hyp0v[4];
	if (h.type==_VECT && h.subtype==_POLYEDRE__VECT)
	  G=h;
	else if (ckmatrix(h,true)){
	  bool cplx=has_i(h); // 4d hypersurface, encode color in a float+int
	  double argcplx;
	  surfacev.push_back(vector< vector<float3d> >(0));
	  vector< vector<float3d> > & S=surfacev.back();
	  const vecteur & V=*h._VECTptr;
	  S.reserve(V.size());
	  for (int j=0;j<V.size();++j){
	    gen Vj=V[j];
	    const vecteur & vj=*Vj._VECTptr;
	    S.push_back(vector<float3d>(0));
	    vector<float3d> &S_=S.back();
	    S_.reserve(vj.size());
	    for (int k=0;k<vj.size();k+=3){
	      double X,Y,Z;
	      if (cplx)
		do_transform(mat,vj[k]._DOUBLE_val,vj[k+1]._DOUBLE_val,absarg(vj[k+2],argcplx),X,Y,Z);
	      else
		do_transform(mat,vj[k]._DOUBLE_val,vj[k+1]._DOUBLE_val,vj[k+2]._DOUBLE_val,X,Y,Z);		
	      // vj[k]=X; vj[k+1]=Y; vj[k+2]=Z;
	      S_.push_back(X); S_.push_back(Y);
	      if (cplx){
		float2 * fptr=(float2 *) &Z;
		fptr->f = Z;
		fptr->a = argcplx;
		S_.push_back(Z);
	      }
	      else
		S_.push_back(Z);
	    }
	  }
	  hyp_color.push_back(cplx?int4(0,0,0,0):int4(u,d,du,dd));
	  continue;
	} // end quad hypersurface
      } // end hypersurface
      if (G.type==_VECT && G.subtype==_GROUP__VECT){
	G=gen(makevecteur(G),_POLYEDRE__VECT);
      }
      if (G.type==_VECT && G.subtype==_POLYEDRE__VECT){
	vecteur p=*G._VECTptr;
	polyedrev.reserve(polyedrev.size()+p.size());
	polyedre_color.reserve(polyedre_color.size()+p.size());
	polyedre_xyminmax.reserve(polyedre_xyminmax.size()+2*p.size());
	for (int j=0;j<p.size();++j){
	  bool is_clipped=false;
	  gen g=p[j];
	  if (g.type==_VECT){
	    vector<double3> cur;
	    vecteur w=*g._VECTptr;
	    cur.reserve(w.size()+1);
	    for (int k=0;k<w.size();++k){
	      gen P=evalf_double(w[k],1,contextptr);
	      if (P.type==_VECT && P._VECTptr->size()==3){
		double x=P[0]._DOUBLE_val,y=P[1]._DOUBLE_val,z=P[2]._DOUBLE_val;
		if (is_clipped && (x<window_xmin || x>window_xmax || y<window_ymin || y>window_ymax || z<window_zmin || z>window_zmax) )
		  is_clipped=false;
		double X,Y,Z;
		do_transform(transform,x,y,z,X,Y,Z);
		cur.push_back(double3(X,Y,Z));
	      }
	    }
	    if (cur.size()>=3){
	      double Z3;int l=0;
	      for (;l<cur.size()-2;++l){
		double x0=cur[l].x,y0=cur[l].y,z0=cur[l].z;
		double x1=cur[l+1].x,y1=cur[l+1].y,z1=cur[l+1].z;
		double x2=cur[l+2].x,y2=cur[l+2].y,z2=cur[l+2].z;
		double X1=x1-x0,Y1=y1-y0,Z1=z1-z0;
		double X2=x2-x0,Y2=y2-y0,Z2=z2-z0;
		double X3=Y1*Z2-Y2*Z1,Y3=X2*Z1-X1*Z2;Z3=X1*Y2-X2*Y1;
		double prec=1e-3;
#if 1
		if ( (X3!=0 || Y3!=0) && std::abs(Z3)<prec*(std::abs(X3)+std::abs(Y3))){
		  Z3=1.0001*prec*(std::abs(X3)+std::abs(Y3));
		}
#endif
		if (std::abs(Z3)>prec*(std::abs(X3)+std::abs(Y3))){
		  // X3*(x-x0)+Y3*(y-y0)+Z3*(z-z0)=0
		  X3/=Z3; Y3/=Z3;
		  polyedre_abcv.push_back(double3(-X3,-Y3,z0+X3*x0+Y3*y0));
		  break;
		}
	      }
	      if (l==cur.size()-2)
		continue;
	      cur.push_back(cur.front());
	      double facemin=1e306,facemax=-1e306;
	      if (fill_polyedre){
		for (int l=1;l<cur.size();++l){
		  double xy=cur[l].y-cur[l].x;
		  if (xy<facemin)
		    facemin=xy;
		  if (xy>facemax)
		    facemax=xy;
		  // replace unused z coordinate by slope
		  // cur[l].z=(cur[l].y-cur[l-1].y)/(cur[l].x-cur[l-1].x);
		}
	      }
	      polyedrev.push_back(vector<double3>(0)); polyedrev.back().swap(cur); // polyedrev.push_back(cur);
	      polyedre_color.push_back(int4(u,d,du,dd));
	      polyedre_xyminmax.push_back(facemin);
	      polyedre_xyminmax.push_back(facemax);
	      polyedre_faceisclipped.push_back(is_clipped);
	      polyedre_filled.push_back(fill_polyedre);
	    } // end cur.size()>=3
	  } // end g.type==_VECT
	}
	continue;
      }      
    }
  }

  bool Graph2d::findij(const gen & e0,double x_scale,double y_scale,double & i0,double & j0,GIAC_CONTEXT) const {
    gen e,f0,f1;
    evalfdouble2reim(e0,e,f0,f1,contextptr);
    if ((f0.type==_DOUBLE_) && (f1.type==_DOUBLE_)){
      if (display_mode & 0x400){
	if (f0._DOUBLE_val<=0)
	  return false;
	f0=std::log10(f0._DOUBLE_val);
      }
      i0=(f0._DOUBLE_val-window_xmin)*x_scale;
      if (display_mode & 0x800){
	if (f1._DOUBLE_val<=0)
	  return false;
	f1=std::log10(f1._DOUBLE_val);
      }
      j0=(window_ymax-f1._DOUBLE_val)*y_scale;
      return true;
    }
    // cerr << "Invalid drawing data" << endl;
    return false;
  }

  inline void swapint(int & i0,int & i1){
    int tmp=i0;
    i0=i1;
    i1=tmp;
  }

  void check_fl_draw(int fontsize,const char * ch,int i0,int j0,int imin,int jmin,int di,int dj,int delta_i,int delta_j,int c){
    /* int n=fl_size();
       if (j0>=jmin-n && j0<=jmin+dj+n) */
    // cerr << i0 << " " << j0 << endl;
    if (strlen(ch)>200)
      text_print(fontsize,"String too long",i0+delta_i,j0+delta_j,c);
    else
      text_print(fontsize,ch,i0+delta_i,j0+delta_j,c);
  }

  inline void check_fl_point(int i0,int j0,int imin,int jmin,int di,int dj,int delta_i,int delta_j,int c){
    /* if (i0>=imin && i0<=imin+di && j0>=jmin && j0<=jmin+dj) */
    os_set_pixel(i0+delta_i,j0+delta_j,c);
  }

  static unsigned short int fl_line_width=1;
  void fl_line(int x0,int y0,int x1,int y1,int c){
    if (fl_line_width==0)
      return;
    if (fl_line_width==1){
      draw_line(x0,y0,x1,y1,c);
      return;
    }
    double dx=x1-x0,dy=y1-y0;
    double n=sqrt(dx*dx+dy*dy);
    dx/=n; dy/=n;
    for (int d=-fl_line_width/2;d<=(fl_line_width+1)/2;++d){
      draw_line(x0-d*dy,y0+d*dx,x1-d*dy,y1+d*dx,c);
    }
    draw_filled_circle(x0,y0,(fl_line_width-1)/2,c,true,true);
    draw_filled_circle(x1,y1,(fl_line_width-1)/2,c,true,true);
  }

  inline void fl_polygon(int x0,int y0,int x1,int y1,int x2,int y2,int c){
    fl_line(x0,y0,x1,y1,c);
    fl_line(x1,y1,x2,y2,c);
    fl_line(x2,y2,x0,y0,c);
  }

  inline void check_fl_line(int i0,int j0,int i1,int j1,int imin,int jmin,int di,int dj,int delta_i,int delta_j,int c){
    fl_line(i0+delta_i,j0+delta_j,i1+delta_i,j1+delta_j,c);
  }

  int logplot_points=20;

  void checklog_fl_line(double i0,double j0,double i1,double j1,double deltax,double deltay,bool logx,bool logy,double window_xmin,double x_scale,double window_ymax,double y_scale,int c){
    if (!logx && !logy){
      fl_line(round(i0+deltax),round(j0+deltay),round(i1+deltax),round(j1+deltay),c);
      return;
    }
  }

  void find_dxdy(const string & legendes,int labelpos,int labelsize,int & dx,int & dy){
    int l=text_width(labelsize,legendes.c_str());
    dx=3;
    dy=1;
    switch (labelpos){
    case 1:
      dx=-l-3;
      break;
    case 2:
      dx=-l-3;
      dy=labelsize-2;
      break;
    case 3:
      dy=labelsize-2;
      break;
    }
    //dy += labelsize;
  }

  void draw_legende(const vecteur & f,int i0,int j0,int labelpos,const Graph2d * iptr,int clip_x,int clip_y,int clip_w,int clip_h,int deltax,int deltay,int c,GIAC_CONTEXT){
    if (f.empty() ||!iptr->show_names )
      return;
    string legendes;
    if (f[0].is_symb_of_sommet(at_curve)){
      gen & f0=f[0]._SYMBptr->feuille;
      if (f0.type==_VECT && !f0._VECTptr->empty()){
	gen & f1 = f0._VECTptr->front();
	if (f1.type==_VECT && f1._VECTptr->size()>4 && (!is_zero((*f1._VECTptr)[4]) || (iptr->show_names & 2)) ){
	  gen legende=f1._VECTptr->front();
	  gen var=(*f1._VECTptr)[1];
	  gen r=re(legende,contextptr),i=im(legende,contextptr),a,b;
	  if (var.type==_IDNT && is_linear_wrt(r,*var._IDNTptr,a,b,contextptr)){
	    i=subst(i,var,(var-b)/a,false,contextptr);
	    legendes=i.print(contextptr);
	  }
	  else
	    legendes=r.print(contextptr)+","+i.print(contextptr);
	  if (legendes.size()>18){
	    if (legendes.size()>30)
	      legendes="";
	    else
	      legendes=legendes.substr(0,16)+"...";
	  }
	}
      }
    }
    if (f.size()>2)
      legendes=gen2string(f[2])+(legendes.empty()?"":":")+legendes;
    if (legendes.empty())
      return;
    int fontsize=iptr->labelsize;
    int dx=3,dy=1;
    find_dxdy(legendes,labelpos,fontsize,dx,dy);
    check_fl_draw(fontsize,legendes.c_str(),i0+dx,j0+dy,clip_x,clip_y,clip_w,clip_h,deltax,deltay,c);
  }

  void petite_fleche(double i1,double j1,double dx,double dy,int deltax,int deltay,int width,int c){
    double dxy=std::sqrt(dx*dx+dy*dy);
    if (dxy){
      dxy/=max(2,min(5,int(dxy/10)))+width;
      dx/=dxy;
      dy/=dxy;
      double dxp=-dy,dyp=dx; // perpendicular
      dx*=std::sqrt(3.0);
      dy*=sqrt(3.0);
      fl_polygon(round(i1)+deltax,round(j1)+deltay,round(i1+dx+dxp)+deltax,round(j1+dy+dyp)+deltay,round(i1+dx-dxp)+deltax,round(j1+dy-dyp)+deltay,c);
    }
  }

  void fltk_point(int deltax,int deltay,int i0,int j0,int epaisseur_point,int type_point,int c){
    switch (type_point){
    case 1: // losange
      fl_line(deltax+i0-epaisseur_point,deltay+j0,deltax+i0,deltay+j0-epaisseur_point,c);
      fl_line(deltax+i0,deltay+j0-epaisseur_point,deltax+i0+epaisseur_point,deltay+j0,c);
      fl_line(deltax+i0-epaisseur_point,deltay+j0,deltax+i0,deltay+j0+epaisseur_point,c);
      fl_line(deltax+i0,deltay+j0+epaisseur_point,deltax+i0+epaisseur_point,deltay+j0,c);
      break;
    case 2: // croix verticale
      fl_line(deltax+i0,deltay+j0-epaisseur_point,deltax+i0,deltay+j0+epaisseur_point,c);
      fl_line(deltax+i0-epaisseur_point,deltay+j0,deltax+i0+epaisseur_point,deltay+j0,c);
      break;
    case 3: // carre
      fl_line(deltax+i0-epaisseur_point,deltay+j0-epaisseur_point,deltax+i0-epaisseur_point,deltay+j0+epaisseur_point,c);
      fl_line(deltax+i0+epaisseur_point,deltay+j0-epaisseur_point,deltax+i0+epaisseur_point,deltay+j0+epaisseur_point,c);
      fl_line(deltax+i0-epaisseur_point,deltay+j0-epaisseur_point,deltax+i0+epaisseur_point,deltay+j0-epaisseur_point,c);
      fl_line(deltax+i0-epaisseur_point,deltay+j0+epaisseur_point,deltax+i0+epaisseur_point,deltay+j0+epaisseur_point,c);
      break;
    case 5: // triangle
      fl_line(deltax+i0-epaisseur_point,deltay+j0,deltax+i0,deltay+j0-epaisseur_point,c);
      fl_line(deltax+i0,deltay+j0-epaisseur_point,deltax+i0+epaisseur_point,deltay+j0,c);
      fl_line(deltax+i0-epaisseur_point,deltay+j0,deltax+i0+epaisseur_point,deltay+j0,c);
      break;
    case 7: // point
      if (epaisseur_point>2)
	fl_arc(deltax+i0-(epaisseur_point-1),deltay+j0-(epaisseur_point-1),2*(epaisseur_point-1),2*(epaisseur_point-1),0,360,c);
      else
	fl_line(deltax+i0,deltay+j0,deltax+i0+1,deltay+j0,c);
      break;
    case 6: // etoile
      fl_line(deltax+i0-epaisseur_point,deltay+j0,deltax+i0+epaisseur_point,deltay+j0,c);
      // no break to add the following lines
    case 0: // 0 croix diagonale
      fl_line(deltax+i0-epaisseur_point,deltay+j0-epaisseur_point,deltax+i0+epaisseur_point,deltay+j0+epaisseur_point,c);
      fl_line(deltax+i0-epaisseur_point,deltay+j0+epaisseur_point,deltax+i0+epaisseur_point,deltay+j0-epaisseur_point,c);
      break;
    default: // 4 nothing drawn
      break;
    }
  }

  int horiz_or_vert(const_iterateur jt,GIAC_CONTEXT){
    gen tmp(*(jt+1)-*jt),r,i;
    reim(tmp,r,i,contextptr);
    if (is_zero(r,contextptr)) return 1;
    if (is_zero(i,contextptr)) return 2;
    return 0;
  }

  void fltk_draw(Graph2d & Mon_image,const gen & g,double x_scale,double y_scale,int clip_x,int clip_y,int clip_w,int clip_h,GIAC_CONTEXT){
    int deltax=0,deltay=STATUS_AREA_PX,fontsize=Mon_image.labelsize;
    if (g.type==_VECT){
      const vecteur & v=*g._VECTptr;
      const_iterateur it=v.begin(),itend=v.end();
      for (;it!=itend;++it)
	fltk_draw(Mon_image,*it,x_scale,y_scale,clip_x,clip_y,clip_w,clip_h,contextptr);
    }
    if (g.type!=_SYMB)
      return;
    unary_function_ptr s=g._SYMBptr->sommet;
    if (g._SYMBptr->feuille.type!=_VECT)
      return;
    vecteur f=*g._SYMBptr->feuille._VECTptr;
    int mxw=LCD_WIDTH_PX,myw=LCD_HEIGHT_PX-STATUS_AREA_PX;
    double i0,j0,i0save,j0save,i1,j1;
    int fs=f.size();
    if (fs>=4 && s==at_parameter && f[0].type==_IDNT){
      // display parameter from the left upper, f[0] name and f[3] value
      char ch[128];
      strcpy(ch,f[0]._IDNTptr->id_name);
      int pos=strlen(ch);
      ch[pos]='=';
      ++pos;
      ch[pos]=0;
      gen g=evalf_double(f[3],1,contextptr);
      if (g.type==_DOUBLE_)
        strcpy(ch+pos,g.print(contextptr).c_str());
      else {
        ch[pos]='?';
        ++pos;
        ch[pos]=0;
      }
      ++Mon_image.nparams;
      int dw=fl_width(ch);
      int fheight=14;
      int ypos=(fheight+1)*Mon_image.nparams+fheight;
      drawRectangle(1,ypos,dw,fheight-1,Mon_image.is3d?_BLACK:_WHITE);
      os_draw_string_small_(1,ypos-fheight,ch);
      if (Mon_image.pushed && Mon_image.moving_param){
        drawLine(64,ypos-2,192,ypos-2,Mon_image.is3d?_WHITE:_BLACK);
        drawLine(64,ypos,64,ypos-fheight,Mon_image.is3d?_WHITE:_BLACK);
        drawLine(192,ypos,192,ypos-fheight,Mon_image.is3d?_WHITE:_BLACK);
	os_draw_string_small_(65,ypos-fheight-2,f[1].print(contextptr).c_str());
	os_draw_string_small_(193,ypos-fheight-2,f[2].print(contextptr).c_str());
	gen gxpos=64+128*(g-f[1])/(f[2]-f[1]);
	if (gxpos.type==_DOUBLE_){
	  int xpos=gxpos._DOUBLE_val;
	  drawLine(xpos,ypos,xpos,ypos-fheight,_red);
	}
      }
      return ;
    }
    string the_legend;
    vecteur style(get_style(f,the_legend));
    int styles=style.size();
    // color
    int ensemble_attributs = style.front().val;
    bool hidden_name = false;
    if (style.front().type==_ZINT){
      ensemble_attributs = mpz_get_si(*style.front()._ZINTptr);
      hidden_name=true;
    }
    else
      hidden_name=ensemble_attributs<0;
    int width           =(ensemble_attributs & 0x00070000) >> 16; // 3 bits
    int epaisseur_point =(ensemble_attributs & 0x00380000) >> 19; // 3 bits
    int type_line       =(ensemble_attributs & 0x01c00000) >> 22; // 3 bits
    if (type_line>4)
      type_line=(type_line-4)<<8;
    int type_point      =(ensemble_attributs & 0x0e000000) >> 25; // 3 bits
    int labelpos        =(ensemble_attributs & 0x30000000) >> 28; // 2 bits
    bool fill_polygon   =(ensemble_attributs & 0x40000000) >> 30;
    int couleur         =(ensemble_attributs & 0x0007ffff);
    epaisseur_point += 2;
    if (s==at_pnt){ 
      // f[0]=complex pnt or vector of complex pnts or symbolic
      // f[1] -> style 
      // f[2] optional=label
      gen point=f[0];
      if (point.type==_VECT && point.subtype==_POINT__VECT)
	return;
      if ( (f[0].type==_SYMB) && (f[0]._SYMBptr->sommet==at_curve) && (f[0]._SYMBptr->feuille.type==_VECT) && (f[0]._SYMBptr->feuille._VECTptr->size()) ){
	// Mon_image.show_mouse_on_object=false;
	point=f[0]._SYMBptr->feuille._VECTptr->back();
	if (type_line>=4 && point.type==_VECT && point._VECTptr->size()>2){
	  vecteur v=*point._VECTptr;
	  int vs=v.size()/2; // 3 -> 1
	  if (Mon_image.findij(v[vs],x_scale,y_scale,i0,j0,contextptr) && Mon_image.findij(v[vs+1],x_scale,y_scale,i1,j1,contextptr)){
	    bool logx=Mon_image.display_mode & 0x400,logy=Mon_image.display_mode & 0x800;
	    checklog_fl_line(i0,j0,i1,j1,deltax,deltay,logx,logy,Mon_image.window_xmin,x_scale,Mon_image.window_ymax,y_scale,couleur);
	    double dx=i0-i1,dy=j0-j1;
	    petite_fleche(i1,j1,dx,dy,deltax,deltay,width+3,couleur);
	  }
	}
      }
      if (is_undef(point))
	return;
      // fl_line_style(type_line,width+1,0); 
      if (point.type==_SYMB) {
	if (point._SYMBptr->sommet==at_cercle){
	  vecteur v=*point._SYMBptr->feuille._VECTptr;
	  gen diametre=remove_at_pnt(v[0]);
	  gen e1=diametre._VECTptr->front().evalf_double(1,contextptr),e2=diametre._VECTptr->back().evalf_double(1,contextptr);
	  gen centre=rdiv(e1+e2,2.0,contextptr);
	  gen e12=e2-e1;
	  double ex=evalf_double(re(e12,contextptr),1,contextptr)._DOUBLE_val,ey=evalf_double(im(e12,contextptr),1,contextptr)._DOUBLE_val;
	  if (!Mon_image.findij(centre,x_scale,y_scale,i0,j0,contextptr))
	    return;
	  gen diam=std::sqrt(ex*ex+ey*ey);
	  gen angle=std::atan2(ey,ex);
	  gen a1=v[1].evalf_double(1,contextptr),a2=v[2].evalf_double(1,contextptr);
	  bool full=v[1]==0 && v[2]==cst_two_pi;
	  if ( (diam.type==_DOUBLE_) && (a1.type==_DOUBLE_) && (a2.type==_DOUBLE_) ){
	    i1=diam._DOUBLE_val*x_scale/2.0;
	    j1=diam._DOUBLE_val*y_scale/2.0;
	    double a1d=a1._DOUBLE_val,a2d=a2._DOUBLE_val,angled=angle._DOUBLE_val;
	    bool changer_sens=a1d>a2d;
	    if (changer_sens){
	      double tmp=a1d;
	      a1d=a2d;
	      a2d=tmp;
	    }
	    double anglei=(angled+a1d),anglef=(angled+a2d),anglem=(anglei+anglef)/2;
	    if (fill_polygon)
	      fl_pie(deltax+round(i0-i1),deltay+round(j0-j1),round(2*i1),round(2*j1),full?0:anglei*180/M_PI+.5,full?360:anglef*180/M_PI+.5,couleur,false);
	    else {
	      fl_arc(deltax+round(i0-i1),deltay+round(j0-j1),round(2*i1),round(2*j1),full?0:anglei*180/M_PI+.5,full?360:anglef*180/M_PI+.5,couleur);
	      if (v.size()>=4){ // if cercle has the optional 5th arg
		if (v[3]==2)
		  petite_fleche(i0+i1*std::cos(anglem),j0-j1*std::sin(anglem),-i1*std::sin(anglem),-j1*std::cos(anglem),deltax,deltay,width,couleur);
		else {
		  if (changer_sens)
		    petite_fleche(i0+i1*std::cos(anglei),j0-j1*std::sin(anglei),-i1*std::sin(anglei),-j1*std::cos(anglei),deltax,deltay,width,couleur);
		  else
		    petite_fleche(i0+i1*std::cos(anglef),j0-j1*std::sin(anglef),i1*std::sin(anglef),j1*std::cos(anglef),deltax,deltay,width,couleur);
		}
	      }
	    }
	    // Label a few degrees from the start angle, 
	    // FIXME should use labelpos
	    double anglel=angled+a1d+0.3;
	    if (v.size()>=4 && v[3]==2)
	      anglel=angled+(0.45*a1d+0.55*a2d);
	    i0=i0+i1*std::cos(anglel); 
	    j0=j0-j1*std::sin(anglel);
	    if (!hidden_name)
	      draw_legende(f,round(i0),round(j0),labelpos,&Mon_image,clip_x,clip_y,clip_w,clip_h,0,0,couleur,contextptr);
	    return;
	  }
	} // end circle
#if 1
	if (point._SYMBptr->sommet==at_legende){
	  gen & f=point._SYMBptr->feuille;
	  if (f.type==_VECT && f._VECTptr->size()==3){
	    vecteur & fv=*f._VECTptr;
	    if (fv[0].type==_VECT && fv[0]._VECTptr->size()>=2 && fv[1].type==_STRNG && fv[2].type==_INT_){
	      vecteur & fvv=*fv[0]._VECTptr;
	      if (fvv[0].type==_INT_ && fvv[1].type==_INT_){
		int dx=0,dy=0;
		string legendes(*fv[1]._STRNGptr);
		find_dxdy(legendes,labelpos,fontsize,dx,dy);
		text_print(fontsize,legendes.c_str(),deltax+fvv[0].val+dx,deltay+fvv[1].val+dy,fv[2].val);
	      }
	    }
	  }
	}
#endif
      } // end point.type==_SYMB
      if (point.type!=_VECT || (point.type==_VECT && (point.subtype==_GROUP__VECT || point.subtype==_VECTOR__VECT) && point._VECTptr->size()==2 && is_zero(point._VECTptr->back()-point._VECTptr->front())) ){ // single point
	if (!Mon_image.findij((point.type==_VECT?point._VECTptr->front():point),x_scale,y_scale,i0,j0,contextptr))
	  return;
	if (i0>0 && i0<mxw && j0>0 && j0<myw)
	  fltk_point(deltax,deltay,round(i0),round(j0),epaisseur_point,type_point,couleur);
	if (!hidden_name)
	  draw_legende(f,round(i0),round(j0),labelpos,&Mon_image,clip_x,clip_y,clip_w,clip_h,0,0,couleur,contextptr);
	return;
      }
      // path
      const_iterateur jt=point._VECTptr->begin(),jtend=point._VECTptr->end();
      if (jt==jtend)
	return;
      bool logx=Mon_image.display_mode & 0x400,logy=Mon_image.display_mode & 0x800;
      if (jt->type==_VECT)
	return;
      if ( (type_point || epaisseur_point>2) && type_line==0 && width==0){
	for (;jt!=jtend;++jt){
	  if (!Mon_image.findij(*jt,x_scale,y_scale,i0,j0,contextptr))
	    return;
	  if (i0>0 && i0<mxw && j0>0 && j0<myw)
	    fltk_point(deltax,deltay,round(i0),round(j0),epaisseur_point,type_point,couleur);
	}
	if (!hidden_name)
	  draw_legende(f,round(i0),round(j0),labelpos,&Mon_image,clip_x,clip_y,clip_w,clip_h,0,0,couleur,contextptr);
	return;
      }
      // initial point
      if (!Mon_image.findij(*jt,x_scale,y_scale,i0,j0,contextptr))
	return;
      i0save=i0;
      j0save=j0;
      if (fill_polygon){
	if (jtend-jt==5 && *(jt+4)==*jt){
	  // check rectangle parallel to axes -> draw_rectangle (filled)
	  int cote1=horiz_or_vert(jt,contextptr);
	  if (cote1 && horiz_or_vert(jt+1,contextptr)==3-cote1 && horiz_or_vert(jt+2,contextptr)==cote1 && horiz_or_vert(jt+3,contextptr)==3-cote1){
	    if (!Mon_image.findij(*(jt+2),x_scale,y_scale,i0,j0,contextptr))
	      return;
	    int x,y,w,h;
	    if (i0<i0save){
	      x=i0;
	      w=i0save-i0;
	    }
	    else {
	      x=i0save;
	      w=i0-i0save;
	    }
	    if (j0<j0save){
	      y=j0;
	      h=j0save-j0;
	    }
	    else {
	      y=j0save;
	      h=j0-j0save;
	    }
	    draw_rectangle(deltax+x,deltay+y,w+1,h+1,couleur);
	    if (!hidden_name)
	      draw_legende(f,deltax+x,deltay+y,labelpos,&Mon_image,clip_x,clip_y,clip_w,clip_h,0,0,couleur,contextptr);
	    return;
	  }
	} // end rectangle check
	bool closed=*jt==*(jtend-1);
	vector< vector<int> > vi(jtend-jt+(closed?0:1),vector<int>(2));
	for (int pos=0;jt!=jtend;++pos,++jt){
	  if (!Mon_image.findij(*jt,x_scale,y_scale,i0,j0,contextptr))
	    return;
	  vi[pos][0]=i0+deltax;
	  vi[pos][1]=j0+deltay;
	}
	if (!closed)
	  vi.back()=vi.front();
	draw_filled_polygon(vi,0,LCD_WIDTH_PX,0,LCD_HEIGHT_PX,couleur);
	if (!hidden_name)
	  draw_legende(f,round(i0),round(j0),labelpos,&Mon_image,clip_x,clip_y,clip_w,clip_h,0,0,couleur,contextptr);
	return;
      }
      ++jt;
      if (jt==jtend){
	if (i0>0 && i0<mxw && j0>0 && j0<myw)
	  check_fl_point(deltax+round(i0),deltay+round(j0),clip_x,clip_y,clip_w,clip_h,0,0,couleur);
	if (!hidden_name)
	  draw_legende(f,round(i0),round(j0),labelpos,&Mon_image,clip_x,clip_y,clip_w,clip_h,0,0,couleur,contextptr);
	return;
      }
      bool seghalfline=( point.subtype==_LINE__VECT || point.subtype==_HALFLINE__VECT ) && (point._VECTptr->size()==2);
      // rest of the path
      for (;;){
	if (!Mon_image.findij(*jt,x_scale,y_scale,i1,j1,contextptr))
	  return;
	if (!seghalfline){
	  checklog_fl_line(i0,j0,i1,j1,deltax,deltay,logx,logy,Mon_image.window_xmin,x_scale,Mon_image.window_ymax,y_scale,couleur);
	  if (point.subtype==_VECTOR__VECT){
	    double dx=i0-i1,dy=j0-j1;
	    petite_fleche(i1,j1,dx,dy,deltax,deltay,width,couleur);
	  }
	}
	++jt;
	if (jt==jtend){ // label of line at midpoint
	  if (point.subtype==_LINE__VECT){
	    i0=(6*i1-i0)/5-8;
	    j0=(6*j1-j0)/5-8;
	  }
	  else {
	    i0=(i0+i1)/2-8;
	    j0=(j0+j1)/2;
	  }
	  break;
	}
	i0=i1;
	j0=j1;
      }
      // check for a segment/halfline/line
      if ( seghalfline){
	double deltai=i1-i0save,adeltai=absdouble(deltai);
	double deltaj=j1-j0save,adeltaj=absdouble(deltaj);
	if (point.subtype==_LINE__VECT){
	  if (deltai==0)
	    checklog_fl_line(i1,0,i1,clip_h,deltax,deltay,logx,logy,Mon_image.window_xmin,x_scale,Mon_image.window_ymax,y_scale,couleur);
	  else {
	    if (deltaj==0)
	      checklog_fl_line(0,j1,clip_w,j1,deltax,deltay,Mon_image.display_mode & 0x400,Mon_image.display_mode & 0x800,Mon_image.window_xmin,x_scale,Mon_image.window_ymax,y_scale,couleur);
	    else {
	      // Find the intersections with the 4 rectangle segments
	      // Horizontal x=0 or w =i1+t*deltai: y=j1+t*deltaj
	      vector< complex<double> > pts;
	      double y0=j1-i1/deltai*deltaj,tol=clip_h*1e-6;
	      if (y0>=-tol && y0<=clip_h+tol)
		pts.push_back(complex<double>(0.0,y0));
	      double yw=j1+(clip_w-i1)/deltai*deltaj;
	      if (yw>=-tol && yw<=clip_h+tol)
		pts.push_back(complex<double>(clip_w,yw));
	      // Vertical y=0 or h=j1+t*deltaj, x=i1+t*deltai
	      double x0=i1-j1/deltaj*deltai;
	      tol=clip_w*1e-6;
	      if (x0>=-tol && x0<=clip_w+tol)
		pts.push_back(complex<double>(x0,0.0));
	      double xh=i1+(clip_h-j1)/deltaj*deltai;
	      if (xh>=-tol && xh<=clip_w+tol)
		pts.push_back(complex<double>(xh,clip_h));
	      if (pts.size()>=2)
		checklog_fl_line(pts[0].real(),pts[0].imag(),pts[1].real(),pts[1].imag(),deltax,deltay,Mon_image.display_mode & 0x400,Mon_image.display_mode & 0x800,Mon_image.window_xmin,x_scale,Mon_image.window_ymax,y_scale,couleur);
	    } // end else adeltai==0 , adeltaj==0
	  } // end else adeltai==0
	} // end LINE_VECT
	else {
	  double N=1;
	  if (adeltai){
	    N=clip_w/adeltai+1;
	    if (adeltaj)
	      N=max(N,clip_h/adeltaj+1);
	  }
	  else {
	    if (adeltaj)
	      N=clip_h/adeltaj+1;
	  }
	  N *= 2; // increase N since rounding might introduce too small clipping
	  while (fabs(N*deltai)>10000)
	    N /= 2;
	  while (fabs(N*deltaj)>10000)
	    N /= 2;
	  checklog_fl_line(i0save,j0save,i1+N*deltai,j1+N*deltaj,deltax,deltay,Mon_image.display_mode & 0x400,Mon_image.display_mode & 0x800,Mon_image.window_xmin,x_scale,Mon_image.window_ymax,y_scale,couleur);
	}
      } // end seghalfline
      if ( (point.subtype==_GROUP__VECT) && (point._VECTptr->size()==2))
	; // no legend for segment
      else {
	if (!hidden_name)
	  draw_legende(f,round(i0),round(j0),labelpos,&Mon_image,clip_x,clip_y,clip_w,clip_h,0,0,couleur,contextptr);
      }
    } // end pnt subcase
  }
#endif

  // return a vector of values with simple decimal representation
  // between xmin/xmax or including xmin/xmax (if bounds is true)
  vecteur ticks(double xmin,double xmax,bool bounds){
    if (xmax<xmin)
      swapdouble(xmin,xmax);
    double dx=xmax-xmin;
    vecteur res;
    if (dx==0)
      return res;
    double d=std::pow(10.0,std::floor(std::log10(dx)));
    if (dx<2*d)
      d=d/5;
    else {
      if (dx<5*d)
	d=d/2;
    }
    double x1=std::floor(xmin/d)*d;
    double x2=(bounds?std::ceil(xmax/d):std::floor(xmax/d))*d;
    for (double x=x1+(bounds?0:d);x<=x2;x+=d){
      if (absdouble(x-int(x+.5))<1e-6*d)
	res.push_back(int(x+.5));
      else
	res.push_back(x);
    }
    return res;
  }

  void normalize(double & x, double & y){
    double fx=fabs(x),fy=fabs(y);
    if (fx>fy){
      y/=fx;
      x/=fx;
    }
    else {
      x/=fy;
      y/=fy;
    }
    double n=std::sqrt(x*x+y*y);
    x/=n; y/=n;
  }

  void Graph2d::adddepth(vector<int2> & polyg,const double3 &A,const double3 &B,int2 & IJmin) const {
    if ((A.z-current_depth)*(B.z-current_depth)>0)
      return;
    double t=(current_depth-A.z)/(B.z-A.z);
    double x=A.x+t*(B.x-A.x);
    double y=A.y+t*(B.y-A.y);
    int I,J;
    XYZ2ij(double3(x,y,current_depth),I,J);
    int2 IJ(I,J);
    polyg.push_back(IJ);
    if (IJ<IJmin)
      IJmin=IJ;
  }

  void Graph2d::addpolyg(vector<int2> & polyg,double x,double y,double z,int2 & IJmin) const {
    int I,J;
    xyz2ij(double3(x,y,z),I,J);
    int2 IJ(I,J);
    polyg.push_back(IJ);
    if (IJ<IJmin)
      IJmin=IJ;
  }

  int roundint (double r) {
    int tmp = static_cast<int> (r);
    tmp += (r-tmp>=.5) - (r-tmp<=-.5);
    return tmp;
  }

  bool Graph2d::find_dxdy(double & dx, double & dy) const {
    double xmin=window_xmin,xmax=window_xmax;
    int hp=LCD_WIDTH_PX-1;
    dx=(xmax-xmin)/hp;
    double ymin=window_ymin,ymax=window_ymax;
    int vp=LCD_HEIGHT_PX-1;
    dy=(ymax-ymin)/vp;
    return abs(dx-dy) < 0.000001;
  }

  void Graph2d::find_xy(double i,double j,double & x,double & y) const {
    double xmin=window_xmin,xmax=window_xmax;
    x=xmin+i*(xmax-xmin)/LCD_WIDTH_PX;
    double ymin=window_ymin,ymax=window_ymax;
    y=ymax-j*(ymax-ymin)/LCD_HEIGHT_PX;
  }

  void Graph2d::round_xy(double & x, double & y) const {
    double dx,dy;
    find_dxdy(dx,dy);
    double range = pow(10,log10(1.0/dx));
    x = roundint(x * range) / range;
    y = roundint(y * range) / range;
  }
  
  void round3(double & x,double xmin,double xmax){
    double dx=std::abs(xmax-xmin);
    double logdx=std::log10(dx);
    int ndec=int(logdx)-3;
    double xpow=std::pow(10.0,ndec);
    int newx=x>=0?int(x/xpow+0.5):int(x/xpow-0.5);
    x=newx*xpow;
  }

  vecteur Graph2d::param(double d) const {
    const_iterateur it=plot_instructions.begin(),itend=plot_instructions.end();
    vecteur res;
    double pos=0.5;
    for (int i=0 ;it!=itend;++i,++it){
      gen tmp=*it;
      if (tmp.is_symb_of_sommet(at_parameter)){
	tmp=tmp._SYMBptr->feuille;
	if (tmp.type==_VECT && tmp._VECTptr->size()>=4){
	  if (std::abs(d-pos)<0.50001){
	    res.push_back(tmp);
	    res.push_back(i);
	  }
	  ++pos;
	}
      }
    }
    return res;
  }
  
  void Graph2d::draw_decorations(const gen & title_tmp){
    if (args_tmp.empty()){ // add selected names
      char s[256]; strcpy(s,modestr.c_str());
      int pos=0,modestrsize=modestr.size();
      pos += modestrsize;
      s[pos++]=' ';
      if (mode!=0 && drag_name.type==_IDNT){
	strcpy(s+pos,drag_name._IDNTptr->id_name);
	pos += strlen(drag_name._IDNTptr->id_name);
      }
      else {
	if (1 || mode==0 || mode==255){ // print selected names 
	  vecteur v;
	  if (mode!=255) v=selected_names(true,false);
	  if (v.empty() && current_i<=192 && current_j<14*nparams+21){
	    double d=current_j/14.-1;
	    v=param(d);
	    if (v.size()!=2)
	      v.clear();
	    else
	      v=vecteur(1,v.front()[0]);
	  }
	  int vs=v.size();
	  if (!vs){ // print current coordinates
	    double i=current_i,j=current_j,x,y,z;
	    if (is3d){
	      find_xyz(i,j,current_depth,x,y,z);
	      round_xy(x,y); round3(z,window_zmin,window_zmax);
	      sprintf(s+pos," %.3g,%.3g,%.3g",x,y,z);
	    }
	    else {
	      find_xy(i,j,x,y);
	      // round to maximum pixel range
	      round_xy(x,y);
	      sprintf(s+pos," x=%.3g,y=%.3g",x,y);
	    }
	    pos=strlen(s);
	  }
	  for (int i=0;i<vs && pos<100;++i){
	    if (v[i].type==_IDNT){
	      strcpy(s+pos,v[i]._IDNTptr->id_name);
	      pos += strlen(v[i]._IDNTptr->id_name);
	      if (i<vs-1){
		s[pos++]=',';
	      }
	    }
	  }
	}
      }
      s[pos]=0;
      if (tracemode){
	if (tracemode_add.size())
	  os_draw_string_small_(0,0,tracemode_add.c_str());
	else
	  os_draw_string_small_(LCD_WIDTH_PX-fl_width(s),0,s);	  
	if (!tracemode_disp.empty())
	  fltk_draw(*this,tracemode_disp,x_scale,y_scale,0,0,LCD_WIDTH_PX,LCD_HEIGHT_PX,contextptr);
      }
      else
	os_draw_string_small_(LCD_WIDTH_PX-fl_width(s),LCD_HEIGHT_PX-14,s);
    }
    if (mode && title.size()<100 && (!title.empty() || !is_zero(title_tmp))){
      std::string mytitle;
      if (!is_zero(title_tmp) && function_final.type==_FUNC){
	if (function_final==at_point && title_tmp.is_symb_of_sommet(at_point))
	  mytitle=title_tmp.print(contextptr);
	else
	  mytitle=function_final._FUNCptr->ptr()->s+('('+title_tmp.print(contextptr)+')'); // gen(symbolic(*function_final._FUNCptr,title_tmp)).print(contextptr);
      }
      else
	mytitle=title;
      if (!mytitle.empty()){
	int dt=int(fl_width(mytitle.c_str()));
	if (dt>LCD_WIDTH_PX)
	  dt=LCD_WIDTH_PX;
	os_draw_string_small_((LCD_WIDTH_PX-dt)/2,LCD_HEIGHT_PX-14,mytitle.c_str());
      }
    }
    if (hp || tracemode){ // draw cursor at current_i,current_j
      int taille=(mode==255 && !tracemode) ?2:5;
#ifdef HP39
      fl_line(current_i-taille,current_j,current_i+taille,current_j,0);
      fl_line(current_i,current_j-taille,current_i,current_j+taille,0);
#else
      fl_line(current_i-taille,current_j,current_i+taille,current_j,is3d?_CYAN:_BLUE);
      fl_line(current_i,current_j-taille,current_i,current_j+taille,is3d?_CYAN:_BLUE);
#endif
      if (cursor_point_type==6){
        fl_line(current_i-2,current_j+2,current_i+2,current_j+2,_red);
        fl_line(current_i-2,current_j+2,current_i+2,current_j+2,_red);
        fl_line(current_i-2,current_j-2,current_i-2,current_j+2,_red);
        fl_line(current_i+2,current_j-2,current_i+2,current_j+2,_red);
      }
    }
  }

  void displaypolyg(const vector<int2> & polyg,const int2 & IJmin,int color,int & Px,int & Py,GIAC_CONTEXT){
    if (polyg.empty())
      return;
    // sort list of arguments
    vector<int2_double2> p;
    for (int k=0;k<polyg.size();++k){
      const int2 & cur=polyg[k];
      if (cur==IJmin){
	int2_double2 id={cur.i,cur.j,0,0};
	p.push_back(id);
      } else {
	double di=cur.i-IJmin.i,dj=cur.j-IJmin.j;
	int2_double2 id={cur.i,cur.j,atan2(di,dj),di*di+dj*dj};
	p.push_back(id);
      }
    }
    sort(p.begin(),p.end());
    // draw polygon
    vector< vector<int> > P;
    for (int k=0;k<p.size();++k){
      vector<int> vi(2);
      vi[0]=p[k].i;
      vi[1]=p[k].j;
      P.push_back(vi);
    }
    draw_polygon(P,color 
		 // | 0x400000
		 ,contextptr);
    Px=P[0][0];
    Py=P[0][1];
  }

  void Graph2d::draw(){
    waitforvblank();
    nparams=0; // reset number of parameters (shown from left upper)
    if (hp) history_plot(contextptr).clear();
    if (is3d){
      if (lang==1)
	statuslinemsg("Toolbox: aide");
      else
	statuslinemsg("Toolbox: help");
      double3 A(window_xmin,window_ymin,window_zmin),
	B(window_xmin,window_ymin,window_zmax),
	C(window_xmax,window_ymin,window_zmin),
	D(window_xmax,window_ymin,window_zmax),
	E(window_xmin,window_ymax,window_zmin),
	F(window_xmin,window_ymax,window_zmax),
	G(window_xmax,window_ymax,window_zmin),
	H(window_xmax,window_ymax,window_zmax);
      double3 A3,B3,C3,D3,E3,F3,G3,H3;
      xyz2ij(A,Ai,Aj,A3);
      xyz2ij(B,Bi,Bj,B3);
      xyz2ij(C,Ci,Cj,C3);
      xyz2ij(D,Di,Dj,D3);
      xyz2ij(E,Ei,Ej,E3);
      xyz2ij(F,Fi,Fj,F3);
      xyz2ij(G,Gi,Gj,G3);
      xyz2ij(H,Hi,Hj,H3);
      set_abort();
      int prec=precision;
      if (mode==0 && precision<3)
	precision += 2;
      glsurface(precision,precision,lcdz,contextptr,default_upcolor,default_downcolor,default_downupcolor,default_downdowncolor);
      precision=prec;
      clear_abort();
      if (show_edges){
	// polyhedrons
	for (int k=0;k<int(polyedrev.size());++k){
	  const vector<double3> & cur=polyedrev[k]; // current face
	  const int4 & col=polyedre_color[k];
	  for (int l=1;l<int(cur.size());++l){
	    const double3 & p=cur[l?l-1:cur.size()-1];
	    const double3 & c=cur[l];
	    // is edge visible?
	    double3 m(p.x/2+c.x/2,p.y/2+c.y/2,p.z/2+c.z/2);
	    double xy=m.x+m.y;
	    int mi,mj;
	    XYZ2ij(m,mi,mj);
	    int kk,jmin=RAND_MAX,jmax=-RAND_MAX;
	    for (kk=0;kk<int(polyedrev.size());++kk){
	      if (k==kk)
		continue;
	      const vector<double3> & Cur=polyedrev[kk];
	      int ll;
	      // first check if point is in face
	      for (ll=1;ll<int(Cur.size());++ll){
		const double3 & P=Cur[ll?ll-1:Cur.size()-1];
		const double3 & C=Cur[ll];
		double3 M(P.x/2+C.x/2,P.y/2+C.y/2,P.z/2+C.z/2);
		if (M.x==m.x && M.y==m.y && M.z==m.z){
		  break; // edge PC has same midpoint, will ignore face
		}
	      }
	      if (ll<int(Cur.size())) // point is in face, ignore face
		continue;
	      double3 M0; bool found1st=false;
	      for (ll=1;ll<int(Cur.size());++ll){
		const double3 & P=Cur[ll?ll-1:Cur.size()-1];
		const double3 & C=Cur[ll];
		// intersect plane y-x=m.y-m.x with PC edge P+t*PC
		double PCx=C.x-P.x,PCy=C.y-P.y,dPC=PCy-PCx;
		// P.y-P.x + t*dPC=m.y-m.x
		if (dPC==0) // edge is parallel
		  continue;
		double t=((m.y-m.x)+(P.x-P.y))/dPC;
		if (t<0 || t>1)
		  continue;
		double x=P.x+t*PCx;
		double y=P.y+t*PCy;
		double z=P.z+t*(C.z-P.z);
		if (!found1st){
		  M0=double3(x,y,z);
		  found1st=true;
		  continue;
		}
		if (x==M0.x && y==M0.y && z==M0.z)
		  continue;
		// segment([x,y,z],M0) has same y-x as m,
		// find segment position for same y+x as m [x,y,z]+t*(M0-[x,y,z])
		// yx=x+y+t*(M0.x-x+M0.y-y)
		double M0xy=M0.x-x+M0.y-y;
		int i1,j1,i2,j2; // N.B. i1,i2 should be the same as mi
		if (std::abs(M0xy)<1e-14)
		  t=-1;
		else
		  t=(xy-x-y)/M0xy;
		if (t<=0 || t>=1){
		  if (x+y<=xy) // segment is behind midpoint m
		    continue;
		  XYZ2ij(M0,i1,j1); 
		  XYZ2ij(double3(x,y,z),i2,j2);
		  if (j1>j2) swapint(j1,j2);
		  if (jmin>j1) jmin=j1;
		  if (jmax<j2) jmax=j2;
		  if (jmin<mj && mj<jmax){
		    break;
		  }
		  continue;
		}
		// find segment part that might mask midpoint m
		double X = x+t*(M0.x-x);
		double Y = y+t*(M0.y-y);
		double Z = z+t*(M0.z-z);
		XYZ2ij(double3(X,Y,Z),i1,j1);
		if (x+y<=xy)
		  XYZ2ij(M0,i2,j2);
		else
		  XYZ2ij(double3(x,y,z),i2,j2);
		if (j1>j2) swapint(j1,j2);
		if (jmin>j1) jmin=j1;
		if (jmax<j2) jmax=j2;
		if (jmin<mj && mj<jmax){
		  break;
		}
	      } // end for
	      if (ll<int(Cur.size())){
		// means edge is not visible
		break;
	      }
	    }
	    // polyedre attribute: filled/not filled
	    bool filled=polyedre_filled[k];
	    bool hidden=kk<int(polyedrev.size());
	    if (filled && hidden)
	      continue;
	    int i1,j1,i2,j2;
	    XYZ2ij(p,i1,j1);
	    XYZ2ij(c,i2,j2);
	    if (i1>i2 || (i1==i2 && j1>j2)){
	      swapint(i1,i2); swapint(j1,j2);
	    }
	    drawLine(i1,j1,i2,j2,
		     // col.d | 0x400000
		     col.u | ((hidden || filled)?0x400000:0)
		     );
	  }
	}
      }
      if (show_axes){
	// cube A,B,C,D,E,F,G,H
	// X
	drawLine(Ai,Aj,Ci,Cj,_red | 0x800000);
	drawLine(Bi,Bj,Di,Dj,_red | 0x800000);
	drawLine(Ei,Ej,Gi,Gj,_red | 0x800000);
	drawLine(Fi,Fj,Hi,Hj,_red | 0x800000);
	// Y
	drawLine(Ai,Aj,Ei,Ej,_green | 0x800000);
	drawLine(Bi,Bj,Fi,Fj,_green | 0x800000);
	drawLine(Ci,Cj,Gi,Gj,_green | 0x800000);
	drawLine(Di,Dj,Hi,Hj,_green | 0x800000);
	// Z
	drawLine(Ai,Aj,Bi,Bj,COLOR_CYAN | 0x800000);
	drawLine(Ci,Cj,Di,Dj,COLOR_CYAN | 0x800000);
	drawLine(Ei,Ej,Fi,Fj,COLOR_CYAN | 0x800000);
	drawLine(Gi,Gj,Hi,Hj,COLOR_CYAN | 0x800000);
	// current_depth
	if (hp){
	  vector<int2> polyg; int2 IJmin={RAND_MAX,RAND_MAX};
	  // x: A3-C3, B3-D3; E3-G3,F3-H3
	  adddepth(polyg,A3,C3,IJmin);
	  adddepth(polyg,B3,D3,IJmin);
	  adddepth(polyg,E3,G3,IJmin);
	  adddepth(polyg,F3,H3,IJmin);
	  // y: A3-E3; B3-F3; C3-G3, D3-H3
	  adddepth(polyg,A3,E3,IJmin);
	  adddepth(polyg,B3,F3,IJmin);
	  adddepth(polyg,C3,G3,IJmin);
	  adddepth(polyg,D3,H3,IJmin);
	  // z: A3-B3, C3-D3, E3-F3, G3-H3
	  adddepth(polyg,A3,B3,IJmin);
	  adddepth(polyg,C3,D3,IJmin);
	  adddepth(polyg,E3,F3,IJmin);
	  adddepth(polyg,G3,H3,IJmin);
	  int Px,Py;
	  displaypolyg(polyg,IJmin,COLOR_YELLOW | 0x400000,Px,Py,contextptr);
	}
	// planes
	vecteur attrv(gen2vecteur(g));
	for (int i=0;i<attrv.size();++i){
	  gen attr=attrv[i];
	  gen cur=remove_at_pnt(attr);
	  int upcolor=44444;
	  const char * nameptr=0;
	  if (attr.is_symb_of_sommet(at_pnt)){
	    if (show_names && attr._SYMBptr->feuille.type==_VECT && attr._SYMBptr->feuille._VECTptr->size()==3){
	      gen name=attr._SYMBptr->feuille._VECTptr->back();
	      if (name.type==_IDNT)
		nameptr=name._IDNTptr->id_name;
	      if (name.type==_STRNG)
		nameptr=name._STRNGptr->c_str();
	    }
	    attr=attr._SYMBptr->feuille[1];
	    if (attr.type==_INT_ && (attr.val & 0xffff)!=0){
	      upcolor=attr.val &0xffff;
	    }
	  }
	  if (cur.is_symb_of_sommet(at_hyperplan)){
	    vecteur & w=*cur._SYMBptr->feuille._VECTptr;
	    gen m=evalf_double(w[1],1,contextptr),n=evalf_double(w[0],1,contextptr);
	    double a=n[0]._DOUBLE_val,b=n[1]._DOUBLE_val,c=n[2]._DOUBLE_val;
	    double x0=m[0]._DOUBLE_val,y0=m[1]._DOUBLE_val,z0=m[2]._DOUBLE_val;
	    // a*(x-x0)+b*(y-y0)+c*(z-z0)=0
	    // replace 2 coordinates of M with window_xyzminmax and find last coord
	    vector<int2> polyg; int2 IJmin={RAND_MAX,RAND_MAX};
	    // x
	    if (a!=0){
	      double x=x0-1/a*(b*(window_ymin-y0)+c*(window_zmin-z0));
	      if (x>=window_xmin && x<=window_xmax)
		addpolyg(polyg,x,window_ymin,window_zmin,IJmin);
	      x=x0-1/a*(b*(window_ymin-y0)+c*(window_zmax-z0));
	      if (x>=window_xmin && x<=window_xmax)
		addpolyg(polyg,x,window_ymin,window_zmax,IJmin);
	      x=x0-1/a*(b*(window_ymax-y0)+c*(window_zmin-z0));
	      if (x>=window_xmin && x<=window_xmax)
		addpolyg(polyg,x,window_ymax,window_zmin,IJmin);
	      x=x0-1/a*(b*(window_ymax-y0)+c*(window_zmax-z0));
	      if (x>=window_xmin && x<=window_xmax)
		addpolyg(polyg,x,window_ymax,window_zmax,IJmin);
	    }
	    // y
	    if (b!=0){
	      double y=y0-1/b*(a*(window_xmin-x0)+c*(window_zmin-z0));
	      if (y>=window_ymin && y<=window_ymax)
		addpolyg(polyg,window_xmin,y,window_zmin,IJmin);
	      y=y0-1/b*(a*(window_xmin-x0)+c*(window_zmax-z0));
	      if (y>=window_ymin && y<=window_ymax)
		addpolyg(polyg,window_xmin,y,window_zmax,IJmin);
	      y=y0-1/b*(a*(window_xmax-x0)+c*(window_zmin-z0));
	      if (y>=window_ymin && y<=window_ymax)
		addpolyg(polyg,window_xmax,y,window_zmin,IJmin);
	      y=y0-1/b*(a*(window_xmax-x0)+c*(window_zmax-z0));
	      if (y>=window_ymin && y<=window_ymax)
		addpolyg(polyg,window_xmax,y,window_zmax,IJmin);
	    }
	    // z
	    if (c!=0){
	      double z=z0-1/c*(a*(window_xmin-x0)+b*(window_ymin-y0));
	      if (z>=window_zmin && z<=window_zmax)
		addpolyg(polyg,window_xmin,window_ymin,z,IJmin);
	      z=z0-1/c*(a*(window_xmin-x0)+b*(window_ymax-y0));
	      if (z>=window_zmin && z<=window_zmax)
		addpolyg(polyg,window_xmin,window_ymax,z,IJmin);
	      z=z0-1/c*(a*(window_xmax-x0)+b*(window_ymin-y0));
	      if (z>=window_zmin && z<=window_zmax)
		addpolyg(polyg,window_xmax,window_ymin,z,IJmin);
	      z=z0-1/c*(a*(window_xmax-x0)+b*(window_ymax-y0));
	      if (z>=window_zmin && z<=window_zmax)
		addpolyg(polyg,window_xmax,window_ymax,z,IJmin);
	    }
	    int Px,Py;
	    displaypolyg(polyg,IJmin,upcolor,Px,Py,contextptr);
	    if (nameptr){
	      int x=os_draw_string_small(0,0,0,upcolor,nameptr,true);
	      os_draw_string_small(Px-x,Py,upcolor,0,nameptr);
	    }
	  }
	}
	// frame
	double xi=Ci-Ai,xj=Cj-Aj;
	normalize(xi,xj);
	int decal=180;
	drawLine(20,decal,20+20*xi,decal+20*xj,_red);
	os_draw_string_small(20+20*xi,decal+20*xj,_red,COLOR_BLACK,"x");
	double yi=Ei-Ai,yj=Ej-Aj;
	normalize(yi,yj);
	drawLine(20,decal,20+20*yi,decal+20*yj,_green);
	os_draw_string_small(20+20*yi,decal+20*yj,_green,COLOR_BLACK,"y");
	double zi=Bi-Ai,zj=Bj-Aj;
	normalize(zi,zj);
	drawLine(20,decal,20+20*zi,decal+20*zj,COLOR_CYAN);
	os_draw_string_small(20+20*zi,decal+20*zj,COLOR_CYAN,COLOR_BLACK,"z");
      } // end show_axes
      // now handle legend([x,y],string)
      vecteur V(gen2vecteur(g));
      for (int i=0;i<V.size();++i){
	gen attr=V[i];
	if (attr.is_symb_of_sommet(at_parameter) && attr._SYMBptr->feuille.type==_VECT){
	  vecteur f=*attr._SYMBptr->feuille._VECTptr;
	  int fs=f.size();
	  if (fs>=4 && f[0].type==_IDNT){
	    // display parameter from the left upper, f[0] name and f[3] value
	    char ch[128];
	    strcpy(ch,f[0]._IDNTptr->id_name);
	    int pos=strlen(ch);
	    ch[pos]='=';
	    ++pos;
	    ch[pos]=0;
	    gen g=evalf_double(f[3],1,contextptr);
	    if (g.type==_DOUBLE_)
	      strcpy(ch+pos,g.print(contextptr).c_str());
	    else {
	      ch[pos]='?';
	      ++pos;
	      ch[pos]=0;
	    }
	    ++nparams;
	    int dw=fl_width(ch);
	    int fheight=14;
	    int ypos=(fheight+1)*nparams+fheight;
	    drawRectangle(1,ypos-fheight,dw,fheight-1,_WHITE);
	    os_draw_string_small_(1,ypos-fheight,ch);
	    if (pushed && moving_param){
	      drawLine(64,ypos-2,192,ypos-2,is3d?_WHITE:_BLACK);
	      drawLine(64,ypos,64,ypos-fheight,is3d?_WHITE:_BLACK);
	      drawLine(192,ypos,192,ypos-fheight,is3d?_WHITE:_BLACK);
	      os_draw_string_small_(65,ypos-fheight-2,f[1].print(contextptr).c_str());
	      os_draw_string_small_(193,ypos-fheight-2,f[2].print(contextptr).c_str());
	      gen gxpos=64+128*(g-f[1])/(f[2]-f[1]);
	      if (gxpos.type==_DOUBLE_){
		int xpos=gxpos._DOUBLE_val;
		drawLine(xpos,ypos,xpos,ypos-fheight,_red);
	      }
	    }
	  }
	} // end parameter
	if (attr.is_symb_of_sommet(at_pnt)){
	  attr=attr._SYMBptr->feuille;
	  if (attr.type==_VECT && attr._VECTptr->size()>1){
	    int color=65535;
	    gen attr0=attr._VECTptr->front();
	    attr=attr[1];
	    if (attr.type==_INT_ && (attr.val & 0xffff)!=0){
	      color=attr.val &0xffff;
	    }
	    if (attr0.is_symb_of_sommet(at_legende)){
	      gen leg=attr0._SYMBptr->feuille;
	      if (leg.type==_VECT && leg._VECTptr->size()>=2){
		gen pos=leg._VECTptr->front();
		leg=leg[1];
		if (pos.type==_VECT && pos._VECTptr->size()==2 && leg.type==_STRNG){
		  gen x=pos._VECTptr->front(),y=pos._VECTptr->back();
		  if (x.type==_INT_ && y.type==_INT_)
		    os_draw_string(x.val,y.val,color,0,leg._STRNGptr->c_str());
		}
	      }
	    }
	  }
	}
      }      
#ifdef NSPIRE_NEWLIB
      DefineStatusMessage((char*)"menu: menu, esc: quit", 1, 0, 0);
#else
      DefineStatusMessage((char*)"shift-1: help, home: menu, back: quit", 1, 0, 0);
#endif
      DisplayStatusArea();
      if (hp || tracemode)
	draw_decorations(title_tmp);
      return;
    }
    int save_clip_ymin=clip_ymin;
    clip_ymin=STATUS_AREA_PX;
    int horizontal_pixels=LCD_WIDTH_PX,vertical_pixels=LCD_HEIGHT_PX-STATUS_AREA_PX,deltax=0,deltay=STATUS_AREA_PX,clip_x=0,clip_y=0,clip_w=horizontal_pixels,clip_h=vertical_pixels;
    drawRectangle(0, STATUS_AREA_PX, horizontal_pixels, vertical_pixels,COLOR_WHITE);
    // Draw axis
    double I0,J0;
    findij(zero,x_scale,y_scale,I0,J0,contextptr); // origin
    int i_0=round(I0),j_0=round(J0);
    if (show_axes &&  (window_ymax>=0) && (window_ymin<=0)){ // X-axis
      vecteur aff; int affs;
      char ch[256];
      check_fl_line(deltax,deltay+j_0,deltax+horizontal_pixels,deltay+j_0,clip_x,clip_y,clip_w,clip_h,0,0,_green); 
      check_fl_line(deltax+i_0,deltay+j_0,deltax+i_0+int(x_scale),deltay+j_0,clip_x,clip_y,clip_w,clip_h,0,0,_CYAN);
      aff=ticks(window_xmin,window_xmax,true);
      affs=aff.size();
      for (int i=0;i<affs;++i){
	double d=evalf_double(aff[i],1,contextptr)._DOUBLE_val;
	if (fabs(d)<1e-6) strcpy(ch,"0"); else sprint_double(ch,d);
	int delta=int(horizontal_pixels*(d-window_xmin)/(window_xmax-window_xmin));
	int taille=strlen(ch)*9;
	fl_line(delta,deltay+j_0,delta,deltay+j_0-4,_green);
      }
      check_fl_draw(labelsize,"x",deltax+horizontal_pixels-40,deltay+j_0-4,clip_x,clip_y,clip_w,clip_h,0,0,_green);
    }
    if ( show_axes && (window_xmax>=0) && (window_xmin<=0) ) {// Y-axis
      vecteur aff; int affs;
      char ch[256];
      check_fl_line(deltax+i_0,deltay,deltax+i_0,deltay+vertical_pixels,clip_x,clip_y,clip_w,clip_h,0,0,_red);
      check_fl_line(deltax+i_0,deltay+j_0,deltax+i_0,deltay+j_0-int(y_scale),clip_x,clip_y,clip_w,clip_h,0,0,_CYAN);
      aff=ticks(window_ymin,window_ymax,true);
      affs=aff.size();
      int taille=5;
      for (int j=0;j<affs;++j){
	double d=evalf_double(aff[j],1,contextptr)._DOUBLE_val;
	if (fabs(d)<1e-6) strcpy(ch,"0"); else sprint_double(ch,d);
	int delta=int(vertical_pixels*(window_ymax-d)/(window_ymax-window_ymin));
	if (delta>=taille && delta<=vertical_pixels-taille){
	  fl_line(deltax+i_0,STATUS_AREA_PX+delta,deltax+i_0+4,STATUS_AREA_PX+delta,_red);
	}
      }
      check_fl_draw(labelsize,"y",deltax+i_0+2,deltay+labelsize,clip_x,clip_y,clip_w,clip_h,0,0,_red);
    }
#if 0 // if ticks are enabled, don't forget to set freeze to false
    // Ticks
    if (show_axes && (horizontal_pixels)/(x_scale*x_tick) < 40 && vertical_pixels/(y_tick*y_scale) <40  ){
      if (x_tick>0 && y_tick>0 ){
	double nticks=(horizontal_pixels-I0)/(x_scale*x_tick);
	double mticks=(vertical_pixels-J0)/(y_tick*y_scale);
	int count=0;
	for (int ii=int(-I0/(x_tick*x_scale));ii<=nticks;++ii){
	  int iii=int(I0+ii*x_scale*x_tick+.5);
	  for (int jj=int(-J0/(y_tick*y_scale));jj<=mticks && count<1600;++jj,++count){
	    int jjj=int(J0+jj*y_scale*y_tick+.5);
	    check_fl_point(deltax+iii,deltay+jjj,clip_x,clip_y,clip_w,clip_h,0,0,COLOR_BLACK);
	  }
	}
      }
    }
#endif
    if (show_axes){ 
      int taille,affs,delta;
      vecteur aff;
      char ch[256];
      // X
      aff=ticks(window_xmin,window_xmax,true);
      affs=aff.size();
      for (int i=0;i<affs;++i){
	double d=evalf_double(aff[i],1,contextptr)._DOUBLE_val;
	sprint_double(ch,d);
	delta=int(horizontal_pixels*(d-window_xmin)/(window_xmax-window_xmin));
	taille=strlen(ch)*9;
	fl_line(delta,vertical_pixels+STATUS_AREA_PX-6,delta,vertical_pixels+STATUS_AREA_PX-1,_green);
	if (delta>=taille/2 && delta<=horizontal_pixels){
	  text_print(10,ch,delta-taille/2,vertical_pixels+STATUS_AREA_PX-7,_green);
	}
      }
      // Y
      aff=ticks(window_ymin,window_ymax,true);
      affs=aff.size();
      taille=5;
      for (int j=0;j<affs;++j){
	double d=evalf_double(aff[j],1,contextptr)._DOUBLE_val;
	sprint_double(ch,d);
	delta=int(vertical_pixels*(window_ymax-d)/(window_ymax-window_ymin));
	if (delta>=taille && delta<=vertical_pixels-taille){
	  fl_line(horizontal_pixels-5,STATUS_AREA_PX+delta,horizontal_pixels-1,STATUS_AREA_PX+delta,_red);
	  text_print(10,ch,horizontal_pixels-strlen(ch)*9,STATUS_AREA_PX+delta+taille,_red);
	}
      }
    }
    
    // draw
    fltk_draw(*this,g,x_scale,y_scale,clip_x,clip_y,clip_w,clip_h,contextptr);
    clip_ymin=save_clip_ymin;
    if (hp || tracemode)
      draw_decorations(title_tmp);
  }
  
  void Graph2d::left(double d){ 
    window_xmin -= d;
    window_xmax -= d;
  }

  void Graph2d::right(double d){ 
    window_xmin += d;
    window_xmax += d;
  }

  void Graph2d::up(double d){ 
    window_ymin += d;
    window_ymax += d;
  }

  void Graph2d::down(double d){ 
    window_ymin -= d;
    window_ymax -= d;
  }

  void Graph2d::z_up(double d){ 
    window_zmin += d;
    window_zmax += d;
  }

  void Graph2d::z_down(double d){ 
    window_zmin -= d;
    window_zmax -= d;
  }

  void Turtle::draw(){
    const int deltax=0,deltay=0;
    int horizontal_pixels=LCD_WIDTH_PX-2*giac::COORD_SIZE;
    // Check for fast redraw
    // Then redraw the background
    drawRectangle(deltax, deltay, LCD_WIDTH_PX, LCD_HEIGHT_PX,COLOR_WHITE);
#ifdef TURTLETAB
    if (turtleptr && turtle_stack_size==0){
      turtleptr[0]=logo_turtle();
      ++turtle_stack_size;
    }
#endif
    if (turtleptr &&
#ifdef TURTLETAB
	turtle_stack_size
#else
	!turtleptr->empty()
#endif
	){
      if (turtlezoom>8)
	turtlezoom=8;
      if (turtlezoom<0.125)
	turtlezoom=0.125;
      // check that position is not out of screen
#ifdef TURTLETAB
      logo_turtle t=turtleptr[turtle_stack_size-1];
#else
      logo_turtle t=turtleptr->back();
#endif
      double x=turtlezoom*(t.x-turtlex);
      double y=turtlezoom*(t.y-turtley);
#if 0
      if (x<0)
	turtlex += int(x/turtlezoom);
      if (x>=LCD_WIDTH_PX-10)
	turtlex += int((x-LCD_WIDTH_PX+10)/turtlezoom);
      if (y<0)
	turtley += int(y/turtlezoom);
      if (y>LCD_HEIGHT_PX-10)
	turtley += int((y-LCD_HEIGHT_PX+10)/turtlezoom);
#endif
    }
#if 0
    if (maillage & 0x3){
      fl_color(FL_BLACK);
      double xdecal=std::floor(turtlex/10.0)*10;
      double ydecal=std::floor(turtley/10.0)*10;
      if ( (maillage & 0x3)==1){
	for (double i=xdecal;i<LCD_WIDTH_PX+xdecal;i+=10){
	  for (double j=ydecal;j<LCD_HEIGHT_PX+ydecal;j+=10){
	    fl_point(deltax+int((i-turtlex)*turtlezoom+.5),deltay+LCD_HEIGHT_PX-int((j-turtley)*turtlezoom+.5));
	  }
	}
      }
      else {
	double dj=std::sqrt(3.0)*10,i0=xdecal;
	for (double j=ydecal;j<LCD_HEIGHT_PX+ydecal;j+=dj){
	  int J=deltay+int(LCD_HEIGHT_PX-(j-turtley)*turtlezoom);
	  for (double i=i0;i<LCD_WIDTH_PX+xdecal;i+=10){
	    fl_point(deltax+int((i-turtlex)*turtlezoom+.5),J);
	  }
	  i0 += dj;
	  while (i0>=10)
	    i0 -= 10;
	}
      }
    }
#endif
    // Show turtle position/cap
    if (turtleptr &&
#ifdef TURTLETAB
	turtle_stack_size &&
#else
	!turtleptr->empty() &&
#endif
	!(maillage & 0x4)){
#ifdef TURTLETAB
      logo_turtle turtle=turtleptr[turtle_stack_size-1];
#else
      logo_turtle turtle=turtleptr->back();
#endif
      drawRectangle(deltax+horizontal_pixels,deltay,LCD_WIDTH_PX-horizontal_pixels,2*COORD_SIZE,_YELLOW);
      // drawRectangle(deltax, deltay, LCD_WIDTH_PX, LCD_HEIGHT_PX,COLOR_BLACK);
      char buf[32];
      sprintf(buf,"x %i   ",int(turtle.x+.5));
      text_print(18,buf,deltax+horizontal_pixels,deltay+(2*COORD_SIZE)/3-2,COLOR_BLACK,_YELLOW);
      sprintf(buf,"y %i   ",int(turtle.y+.5));
      text_print(18,buf,deltax+horizontal_pixels,deltay+(4*COORD_SIZE)/3-3,COLOR_BLACK,_YELLOW);
      sprintf(buf,"t %i   ",int(turtle.theta+.5));
      text_print(18,buf,deltax+horizontal_pixels,deltay+2*COORD_SIZE-4,COLOR_BLACK,_YELLOW);
    }
    // draw turtle Logo
    if (turtleptr){
      int save_width=fl_line_width;
#ifdef TURTLETAB
      int l=turtle_stack_size;
#else
      int l=turtleptr->size();
#endif
      if (l>0){
#ifdef TURTLETAB
	logo_turtle prec =turtleptr[0];
#else
	logo_turtle prec =(*turtleptr)[0];
#endif
	int sp=speed;
	for (int k=1;k<l;++k){
	  if (k>=2 && sp){
	    sync_screen();
	    for (int i=0;i<speed;++i){
	      for (int j=0;j<1000;++j){
		if (iskeydown(5) || iskeydown(4) || iskeydown(22)){
		  sp=0;
		  break;
		}
	      }
	    }
	  }
#ifdef TURTLETAB
	  logo_turtle current =(turtleptr)[k];
#else
	  logo_turtle current =(*turtleptr)[k];
#endif
#if 1
	  if (current.s>=0){ // Write a string
	    //cout << current.radius << " " << current.s << endl;
	    if (current.s<ecristab().size())
	      text_print(current.radius,ecristab()[current.s].c_str(),int(deltax+turtlezoom*(current.x-turtlex)),int(deltay+LCD_HEIGHT_PX-turtlezoom*(current.y-turtley)),current.color);
	  }
	  else
#endif
	    {
	      int width=current.turtle_width & 0x1f;
	      fl_line_width=width;
	      if (current.radius>0){
		int r=current.radius & 0x1ff; // bit 0-8
		double theta1,theta2;
		if (current.direct){
		  theta1=prec.theta+double((current.radius >> 9) & 0x1ff); // bit 9-17
		  theta2=prec.theta+double((current.radius >> 18) & 0x1ff); // bit 18-26
		}
		else {
		  theta1=prec.theta-double((current.radius >> 9) & 0x1ff); // bit 9-17
		  theta2=prec.theta-double((current.radius >> 18) & 0x1ff); // bit 18-26
		}
		bool rempli=(current.radius >> 27) & 0x1;
		bool seg=(current.radius >> 28) & 0x1;
		double angle;
		int x,y,R;
		R=int(2*turtlezoom*r+.5);
		angle = M_PI/180*(theta2-90);
		if (current.direct){
		  x=int(turtlezoom*(current.x-turtlex-r*std::cos(angle) - r)+.5);
		  y=int(turtlezoom*(current.y-turtley-r*std::sin(angle) + r)+.5);
		}
		else {
		  x=int(turtlezoom*(current.x-turtlex+r*std::cos(angle) -r)+.5);
		  y=int(turtlezoom*(current.y-turtley+r*std::sin(angle) +r)+.5);
		}
		if (current.direct){
		  if (rempli)
		    fl_pie(deltax+x,deltay+LCD_HEIGHT_PX-y,R,R,theta1-90,theta2-90,current.color,seg);
		  else {
		    for (int d=giacmax(1-r,-(width-1)/2);d<=width/2;++d){
		      x=int(turtlezoom*(current.x-turtlex-r*std::cos(angle) - (r+d))+.5);
		      y=int(turtlezoom*(current.y-turtley-r*std::sin(angle) + (r+d))+.5);
		      R=int(2*turtlezoom*(r+d)+.5);
		      fl_arc(deltax+x,deltay+LCD_HEIGHT_PX-y,R,R,theta1-90,theta2-90,current.color);
		    }
		  }
		}
		else {
		  if (rempli)
		    fl_pie(deltax+x,deltay+LCD_HEIGHT_PX-y,R,R,90+theta2,90+theta1,current.color,seg);
		  else {
		    for (int d=giacmax(1-r,-(width-1)/2);d<=width/2;++d){
		      x=int(turtlezoom*(current.x-turtlex+r*std::cos(angle) -(r+d))+.5);
		      y=int(turtlezoom*(current.y-turtley+r*std::sin(angle) +(r+d))+.5);
		      R=int(2*turtlezoom*(r+d)+.5);
		      fl_arc(deltax+x,deltay+LCD_HEIGHT_PX-y,R,R,90+theta2,90+theta1,current.color);
		    }
		  }
		}
	      } // end radius>0
	      else {
		if (prec.mark){
		  fl_line(deltax+int(turtlezoom*(prec.x-turtlex)+.5),deltay+int(LCD_HEIGHT_PX+turtlezoom*(turtley-prec.y)+.5),deltax+int(turtlezoom*(current.x-turtlex)+.5),deltay+int(LCD_HEIGHT_PX+turtlezoom*(turtley-current.y)+.5),prec.color);
		}
	      }
	      if (current.radius<-1 && k+current.radius>=0){
		// poly-line from (*turtleptr)[k+current.radius] to (*turtleptr)[k]
		vector< vector<int> > vi(1-current.radius,vector<int>(2));
		for (int i=0;i>=current.radius;--i){
#ifdef TURTLETAB
		  logo_turtle & t=(turtleptr)[k+i];
#else
		  logo_turtle & t=(*turtleptr)[k+i];
#endif
		  if (t.radius>0){
		    int r=t.radius & 0x1ff; // bit 0-8
		    int x,y,R;
		    R=int(2*turtlezoom*r+.5);
		    double angle = M_PI/180*(current.theta-90);
		    if (t.direct){
		      x=int(turtlezoom*(t.x-turtlex-r*std::cos(angle) - r)+.5);
		      y=int(turtlezoom*(t.y-turtley-r*std::sin(angle) + r)+.5);
		    }
		    else {
		      x=int(turtlezoom*(t.x-turtlex+r*std::cos(angle) -r)+.5);
		      y=int(turtlezoom*(t.y-turtley+r*std::sin(angle) +r)+.5);
		    }
		    fl_pie(deltax+x,deltay+LCD_HEIGHT_PX-y,R,R,0,360,current.color,false);
		  }
		  vi[-i][0]=deltax+turtlezoom*(t.x-turtlex);
		  vi[-i][1]=deltay+LCD_HEIGHT_PX+turtlezoom*(turtley-t.y);
		  //*logptr(contextptr) << i << " " << vi[-i][0] << " " << vi[-i][1] << endl;
		}
		//vi.back()=vi.front();
		draw_filled_polygon(vi,0,LCD_WIDTH_PX,24,LCD_HEIGHT_PX,current.color);
	      }
	    } // end else (non-string turtle record)
	  prec=current;
	} // end for (all turtle records)
#ifdef TURTLETAB
	logo_turtle & t = (turtleptr)[l-1];
#else
	logo_turtle & t = (*turtleptr)[l-1];
#endif
	int x=int(turtlezoom*(t.x-turtlex)+.5);
	int y=int(turtlezoom*(t.y-turtley)+.5);
	double cost=std::cos(t.theta*deg2rad_d);
	double sint=std::sin(t.theta*deg2rad_d);
	int Dx=int(turtlezoom*turtle_length*cost/2+.5);
	int Dy=int(turtlezoom*turtle_length*sint/2+.5);
	if (t.visible){
	  fl_line(deltax+x+Dy,deltay+LCD_HEIGHT_PX-(y-Dx),deltax+x-Dy,deltay+LCD_HEIGHT_PX-(y+Dx),t.color);
	  int c=t.color;
	  if (!t.mark)
	    c=t.color ^ 0x7777;
	  fl_line(deltax+x+Dy,deltay+LCD_HEIGHT_PX-(y-Dx),deltax+x+3*Dx,deltay+LCD_HEIGHT_PX-(y+3*Dy),c);
	  fl_line(deltax+x-Dy,deltay+LCD_HEIGHT_PX-(y+Dx),deltax+x+3*Dx,deltay+LCD_HEIGHT_PX-(y+3*Dy),c);
	}
      }
      fl_line_width=save_width;
      return;
    } // End logo mode
  }  
  
  int displaygraph(const giac::gen & ge,const gen & gs,GIAC_CONTEXT){
    // graph display
    //if (aborttimer > 0) { Timer_Stop(aborttimer); Timer_Deinstall(aborttimer);}
    xcas::Graph2d gr(ge,contextptr);
    if (gs!=0) gr.symbolic_instructions=gen2vecteur(gs);
    gr.show_axes=global_show_axes;
    gr.init_tracemode();
    if (gr.tracemode & 4)
      gr.orthonormalize(true);
    // initial setting for x and y
    if (ge.type==_VECT){
      const_iterateur it=ge._VECTptr->begin(),itend=ge._VECTptr->end();
      for (;it!=itend;++it){
	if (it->is_symb_of_sommet(at_equal)){
	  const gen & f=it->_SYMBptr->feuille;
	  gen & optname = f._VECTptr->front();
	  gen & optvalue= f._VECTptr->back();
	  if (optname.val==_AXES && optvalue.type==_INT_)
	    gr.show_axes=optvalue.val;
	  if (optname.type==_INT_ && optname.subtype == _INT_PLOT && optname.val>=_GL_X && optname.val<=_GL_Z && optvalue.is_symb_of_sommet(at_interval)){
	    //*logptr(contextptr) << optname << " " << optvalue << endl;
	    gen optvf=evalf_double(optvalue._SYMBptr->feuille,1,contextptr);
	    if (optvf.type==_VECT && optvf._VECTptr->size()==2){
	      gen a=optvf._VECTptr->front();
	      gen b=optvf._VECTptr->back();
	      if (a.type==_DOUBLE_ && b.type==_DOUBLE_){
		switch (optname.val){
		case _GL_X:
		  gr.window_xmin=a._DOUBLE_val;
		  gr.window_xmax=b._DOUBLE_val;
		  gr.update();
		  break;
		case _GL_Y:
		  gr.window_ymin=a._DOUBLE_val;
		  gr.window_ymax=b._DOUBLE_val;
		  gr.update();
		  break;
		case _GL_Z:
		  gr.window_zmin=a._DOUBLE_val;
		  gr.window_zmax=b._DOUBLE_val;
		  gr.update();
		  break;
		}
	      }
	    }
	  }
	}
      }
    }
    return gr.ui();
  }

  vecteur Graph2d::get_current_animation() const {
    if (animation_instructions_pos>=0 && animation_instructions_pos<animation_instructions.size())
      return gen2vecteur(animation_instructions[animation_instructions_pos]);
    return 0;
  }

  void Graph2d::find_title_plot(gen & title_tmp,gen & plot_tmp){
    title_tmp=plot_tmp=0;
    if (//in_area &&
	hp && mode && !args_tmp.empty()){
      if (args_tmp.size()>=2){
	gen function=(mode==int(args_tmp.size()))?function_final:function_tmp;
	if (function.type==_FUNC){
	  bool dim2=!is3d;
	  vecteur args2=args_tmp;
	  if ( *function._FUNCptr==(dim2?at_cercle:at_sphere)){
	    gen argv1;
#ifdef NO_STDEXCEPT
	    argv1=evalf(args_tmp.back(),1,contextptr);
	    argv1=evalf_double(argv1,1,contextptr);
#else
	    try {
	      argv1=evalf(args_tmp.back(),1,contextptr);
	      argv1=evalf_double(argv1,1,contextptr);
	    }
	    catch (std::runtime_error & e){
	      argv1=undef;
	    }
#endif
	    if (argv1.is_symb_of_sommet(at_pnt) ||argv1.type==_IDNT){
	      argv1=remove_at_pnt(argv1);
	      if ( (argv1.type==_VECT && argv1.subtype==_POINT__VECT) || argv1.type==_CPLX || argv1.type==_IDNT)
		args2.back()=args_tmp.back()-args_tmp.front();
	    }
	  }
	  if (function==at_ellipse)
	    ;
	  title_tmp=gen(args2,_SEQ__VECT);
	  bool b=approx_mode(contextptr);
	  if (!b)
	    approx_mode(true,contextptr);
	  plot_tmp=symbolic(*function._FUNCptr,title_tmp);
	  if (!lidnt(title_tmp).empty())
	    ; // cerr << plot_tmp << '\n';
	  bool bb=io_graph(contextptr);
	  int locked=0;
	  if (bb){
#ifdef HAVE_LIBPTHREAD
	    // cerr << "plot title lock" << '\n';
	    locked=pthread_mutex_trylock(&interactive_mutex);
#endif
	    if (!locked)
	      io_graph(false,contextptr);
	  }
	  plot_tmp=protecteval(plot_tmp,1,contextptr);
	  if (bb && !locked){
	    io_graph(bb,contextptr);
#ifdef HAVE_LIBPTHREAD
	    pthread_mutex_unlock(&interactive_mutex);
	    // cerr << "plot title unlock" << '\n';
#endif
	  }
	  if (!b)
	    approx_mode(false,contextptr);	
	} // end function.type==_FUNC
	else
	  title_tmp=gen(args_tmp,_SEQ__VECT);
      } // end size()>=2
      else	
	title_tmp=args_tmp;
    }
  }

  void Graph2d::eval(int start){
    plot_instructions.resize(symbolic_instructions.size());
    if (plot_instructions.empty()) return;
    int level=prog_eval_level_val(contextptr);
    for (size_t i=start;i<symbolic_instructions.size();++i){
      gen g=symbolic_instructions[i];
      set_abort();
      g=protecteval(g,level,contextptr);
      clear_abort();
      giac::ctrl_c=false;
      kbd_interrupted=giac::interrupted=false;
      if (i<plot_instructions.size())
	plot_instructions[i]=g;
      else
	plot_instructions.push_back(g);
      if (g.is_symb_of_sommet(at_trace)){
	gen f=symbolic(at_evalf,g._SYMBptr->feuille);
	f=protecteval(f,1,contextptr);
#ifdef NUMWORKS
	const int maxtrace=128;
#else
	const int maxtrace=512;
#endif
	if (trace_instructions.size()>=maxtrace)
	  trace_instructions.erase(trace_instructions.begin(),trace_instructions.begin()+maxtrace/2);
	trace_instructions.push_back(f);
      }
    }
    is3d=false;
    for (size_t i=0;i<plot_instructions.size();++i){
      gen g=plot_instructions[i];
      if (giac::is3d(g)){
	is3d=true;
	update_rotation();
	break;
      }
    }
    update_g();
  }

  double Graph2d::find_eps() const {
    double dx=window_xmax-window_xmin;
    double dy=window_ymax-window_ymin;
    double dz=window_zmax-window_zmin;
    double eps,epsx,epsy;
    int L=LCD_WIDTH_PX;
    epsx=(npixels*dx)/L;
    epsy=(npixels*dy)/(is3d?L:LCD_HEIGHT_PX);
    eps=(epsx<epsy)?epsy:epsx;
    if (is3d && dz>dy && dz >dx){
      eps=npixels*dz/L;
      eps *= 2;
    }
    return eps;
  }

  void Graph2d::set_gen_value(int n,const giac::gen & g,bool exec){
    // set n-th entry value, if n==-1 add a level
    if (!hp) return;
    if (n==-1 || n>=symbolic_instructions.size()){
      symbolic_instructions.push_back(g);
      n=symbolic_instructions.size()-1;
    } else symbolic_instructions[n]=g;
    hp->set_string_value(n,g.print(contextptr));
    if (exec)
      eval(n);
  }

  void  Graph2d::find_xyz(double i,double j,double k,double & x,double & y,double & z) const {
    if (is3d){ // FIXME
      int horiz=LCD_WIDTH_PX/2,vert=horiz/2;//LCD_HEIGHT_PX/2;
      double lcdz= LCD_HEIGHT_PX/4;
      double xmin=-1,ymin=-1,xmax=1,ymax=1,xscale=0.6*(xmax-xmin)/horiz,yscale=0.6*(ymax-ymin)/vert;
      double Z=current_depth; // -1..1
      double I=i-horiz;
      double J=j-LCD_HEIGHT_PX/2+lcdz*Z;
      double X=yscale*J-xscale*I;
      double Y=yscale*J+xscale*I;
      do_transform(invtransform,X,Y,Z,x,y,z);
    }
    else {
      z=k;
      x=window_xmin+i*(window_xmax-window_xmin)/LCD_WIDTH_PX;
      y=window_ymax-j*(window_ymax-window_ymin)/LCD_HEIGHT_PX;
    }
  }

  gen geometry_round_numeric(double x,double y,double eps,bool approx){
    return approx?gen(x,y):exact_double(x,eps)+cst_i*exact_double(y,eps);
  }

  gen geometry_round_numeric(double x,double y,double z,double eps,bool approx){
    return gen(approx?makevecteur(x,y,z):makevecteur(exact_double(x,eps),exact_double(y,eps),exact_double(z,eps)),_POINT__VECT);
  }

  gen int2color(int couleur_){
    gen col;
    if (couleur_){
      gen tmp;
      int val;
      vecteur colv;
      if ( (val=(couleur_ & 0x0000ffff))){
	tmp=val;
	tmp.subtype=_INT_COLOR;
	colv.push_back(tmp);
      }
      if ((val =(couleur_ & 0x00070000))){
	tmp=val;
	tmp.subtype=_INT_COLOR;
	colv.push_back(tmp);
      }
      if ((val =(couleur_ & 0x00380000))){
	tmp=val;
	tmp.subtype=_INT_COLOR;
	colv.push_back(tmp);
      }
      if ((val =(couleur_ & 0x01c00000))){
	tmp=val;
	tmp.subtype=_INT_COLOR;
	colv.push_back(tmp);
      }
      if ((val =(couleur_ & 0x0e000000))){
	tmp=val;
	tmp.subtype=_INT_COLOR;
	colv.push_back(tmp);
      }
      if ((val =(couleur_ & 0x30000000))){
	tmp=val;
	tmp.subtype=_INT_COLOR;
	colv.push_back(tmp);
      }
      if ((val =(couleur_ & 0x40000000))){
	tmp=val;
	tmp.subtype=_INT_COLOR;
	colv.push_back(tmp);
      }
      if ((val =(couleur_ & 0x80000000))){
	tmp=val;
	tmp.subtype=_INT_COLOR;
	colv.push_back(tmp);
      }
      if (colv.size()==1)
	col=colv.front();
      else
	col=symbolic(at_plus,gen(colv,_SEQ__VECT));
    }
    return col;
  }

  std::string print_color(int couleur){
    return int2color(couleur).print(context0);
  }

  giac::gen add_attributs(const giac::gen & g,int couleur_,GIAC_CONTEXT) {
    if (g.type!=_SYMB)
      return g;
    gen & f=g._SYMBptr->feuille;
    if (g._SYMBptr->sommet==at_couleur && f.type==_VECT && !f._VECTptr->empty()){
      gen col=couleur_;
      col.subtype=_INT_COLOR;
      vecteur v(*f._VECTptr);
      v.back()=col;
      return symbolic(at_couleur,gen(v,_SEQ__VECT));
    }
    if (couleur_==default_color(contextptr))
      return g;
    if (g._SYMBptr->sommet==at_of){
      gen col=couleur_;
      col.subtype=_INT_COLOR;
      return symbolic(at_couleur,gen(makevecteur(g,col),_SEQ__VECT));
    }
    vecteur v =gen2vecteur(f);
    gen col=int2color(couleur_);
    v.push_back(symbolic(at_equal,gen(makevecteur(at_display,col),_SEQ__VECT)));
    return symbolic(g._SYMBptr->sommet,(v.size()==1 && f.type!=_VECT)?f:gen(v,f.type==_VECT?f.subtype:_SEQ__VECT));
  }

  void Graph2d::do_handle(const gen & g){
    if (hp){
      set_gen_value(hp_pos,g,true);
    }
  }

  std::string printn(const gen & g,int n){
    if (g.type!=_DOUBLE_)
      return g.print();
    return giac::print_DOUBLE_(g._DOUBLE_val,n);
  }
  void Graph2d::tracemode_set(int operation){
    if (plot_instructions.empty())
      plot_instructions=gen2vecteur(g);
    if (is_zero(plot_instructions.back())) // workaround for 0 at end in geometry (?)
      plot_instructions.pop_back();
    gen sol(undef);
    if (operation==1 || operation==8){
      double d=tracemode_mark;
      if (!inputdouble(lang==1?"Valeur du parametre?":"Parameter value",d,contextptr))
	return;
      if (operation==8)
	tracemode_mark=d;
      sol=d;
    }
    // handle curves with more than one connected component
    vecteur tracemode_v;
    for (int i=0;i<plot_instructions.size();++i){
      gen g=plot_instructions[i];
      if (g.type==_VECT && !g._VECTptr->empty() && g._VECTptr->front().is_symb_of_sommet(at_curve)){
	vecteur & v=*g._VECTptr;
	for (int j=0;j<v.size();++j)
	  tracemode_v.push_back(v[j]);
      }
      else
	tracemode_v.push_back(g);
    }
    gen G;
    if (tracemode_n<0)
      tracemode_n=tracemode_v.size()-1;
    bool retry=tracemode_n>0;
    for (;tracemode_n<tracemode_v.size();++tracemode_n){
      G=tracemode_v[tracemode_n];
      if (G.is_symb_of_sommet(at_pnt))
	break;
    }
    if (tracemode_n>=tracemode_v.size()){
      // retry
      if (retry){
	for (tracemode_n=0;tracemode_n<tracemode_v.size();++tracemode_n){
	  G=tracemode_v[tracemode_n];
	  if (G.is_symb_of_sommet(at_pnt))
	    break;
	}
      }
      if (tracemode_n>=tracemode_v.size()){
	tracemode=false;
	return;
      }
    }
    int p=python_compat(contextptr);
    python_compat(0,contextptr);
    gen G_orig(G);
    G=remove_at_pnt(G);
    tracemode_disp.clear();
    string curve_infos1,curve_infos2;
    gen parameq,x,y,t,tmin,tmax,tstep;
    // extract position at tracemode_i
    if (G.is_symb_of_sommet(at_curve)){
      gen c=G._SYMBptr->feuille[0];
      parameq=c[0];
      // simple expand for i*ln(x)
      bool b=do_lnabs(contextptr);
      do_lnabs(false,contextptr);
      reim(parameq,x,y,contextptr);
      do_lnabs(b,contextptr);
      t=c[1];
      gen x1=derive(x,t,contextptr);
      gen x2=derive(x1,t,contextptr);
      gen y1=derive(y,t,contextptr);
      gen y2=derive(y1,t,contextptr);
      sto(x,gen("x0",contextptr),contextptr);
      sto(x1,gen("x1",contextptr),contextptr);
      sto(x2,gen("x2",contextptr),contextptr);
      sto(y,gen("y0",contextptr),contextptr);
      sto(y1,gen("y1",contextptr),contextptr);
      sto(y2,gen("y2",contextptr),contextptr);
      tmin=c[2];
      tmax=c[3];
      tmin=evalf_double(tmin,1,contextptr);
      tmax=evalf_double(tmax,1,contextptr);
      if (tmin._DOUBLE_val>tracemode_mark)
	tracemode_mark=tmin._DOUBLE_val;
      if (tmax._DOUBLE_val<tracemode_mark)
	tracemode_mark=tmax._DOUBLE_val;
      G=G._SYMBptr->feuille[1];
      if (G.type==_VECT){
	vecteur &Gv=*G._VECTptr;
	tstep=(tmax-tmin)/(Gv.size()-1);
      }
      double eps=1e-6; // epsilon(contextptr)
      double curt=(tmin+tracemode_i*tstep)._DOUBLE_val;
      if (abs(curt-tracemode_mark)<tstep._DOUBLE_val)
	curt=tracemode_mark;
      if (operation==-1){
	gen A,B,C,R; // detect ellipse/hyperbola
	if (
	    ( x!=t && c.type==_VECT && c._VECTptr->size()>7 && centre_rayon(G_orig,C,R,false,contextptr,true) ) ||
	    is_quadratic_wrt(parameq,t,A,B,C,contextptr)
	    ){
	  if (C.type!=_VECT){ // x+i*y=A*t^2+B*t+C
	    curve_infos1="Parabola";
	    curve_infos2=_equation(G_orig,contextptr).print(contextptr);
	  }
	  else {
	    vecteur V(*C._VECTptr);
	    curve_infos1=V[0].print(contextptr);
	    curve_infos1=curve_infos1.substr(1,curve_infos1.size()-2);
	    curve_infos1+=" O=";
	    curve_infos1+=V[1].print(contextptr);
	    curve_infos1+=", F=";
	    curve_infos1+=V[2].print(contextptr);
	    // curve_infos1=change_subtype(C,_SEQ__VECT).print(contextptr);
	    curve_infos2=change_subtype(R,_SEQ__VECT).print(contextptr);
	  }
	}
	else {
	  if (x==t) curve_infos1="Function "+y.print(contextptr); else curve_infos1="Parametric "+x.print(contextptr)+","+y.print(contextptr);
	  curve_infos2 = t.print(contextptr)+"="+tmin.print(contextptr)+".."+tmax.print(contextptr)+',';
	  curve_infos2 += (x==t?"xstep=":"tstep=")+tstep.print(contextptr);
	}
      }
      if (operation==1)
	curt=sol._DOUBLE_val;
      if (operation==7)
	sol=tracemode_mark=curt;
      if (operation==2){ // root near curt
	sol=newton(y,t,curt,NEWTON_DEFAULT_ITERATION,eps,1e-12,true,tmin._DOUBLE_val,tmax._DOUBLE_val,1,0,1,contextptr);
	if (sol.type==_DOUBLE_){
	  confirm(lang==1?"Racine en":"Root at",sol.print(contextptr).c_str());
	  sto(sol,gen("Zero",contextptr),contextptr);
	}
      }
      if (operation==4){ // horizontal tangent near curt
	sol=newton(y1,t,curt,NEWTON_DEFAULT_ITERATION,eps,1e-12,true,tmin._DOUBLE_val,tmax._DOUBLE_val,1,0,1,contextptr);
	if (sol.type==_DOUBLE_){
	  confirm(lang==1?"y'=0, extremum/pt singulier en":"y'=0, extremum/singular pt at",sol.print(contextptr).c_str());
	  sto(sol,gen("Extremum",contextptr),contextptr);
	}
      }
      if (operation==5){ // vertical tangent near curt
	if (x1==1)
	  do_confirm(lang==1?"Outil pour courbes parametriques!":"Tool for parametric curves!");
	else {
	  sol=newton(x1,t,curt,NEWTON_DEFAULT_ITERATION,eps,1e-12,true,tmin._DOUBLE_val,tmax._DOUBLE_val,1,0,1,contextptr);
	  if (sol.type==_DOUBLE_){
	    confirm("x'=0, vertical or singular",sol.print(contextptr).c_str());
	    sto(sol,gen("Vertical",contextptr),contextptr);
	  }
	}
      }
      if (operation==6){ // inflexion
	sol=newton(x1*y2-x2*y1,t,curt,NEWTON_DEFAULT_ITERATION,eps,1e-12,true,tmin._DOUBLE_val,tmax._DOUBLE_val,1,0,1,contextptr);
	if (sol.type==_DOUBLE_){
	  confirm("x'*y''-x''*y'=0",sol.print(contextptr).c_str());
	  sto(sol,gen("Inflexion",contextptr),contextptr);
	}
      }
      gen M(put_attributs(_point(subst(parameq,t,tracemode_mark,false,contextptr),contextptr),vecteur(1,_POINT_WIDTH_4 | _BLUE),contextptr));
      tracemode_disp.push_back(M);      
      gen f;
      if (operation==9)
	f=y*derive(x,t,contextptr);
      if (operation==10){
	f=sqrt(pow(x1,2,contextptr)+pow(y1,2,contextptr),contextptr);
      }
      if (operation==9 || operation==10){
	double a=tracemode_mark,b=curt;
	if (a>b)
	  swapdouble(a,b);
	gen res=symbolic( (operation==9 && x==t?at_plotarea:at_integrate),
			  makesequence(f,symb_equal(t,symb_interval(a,b))));
	if (operation==9)
	  tracemode_disp.push_back(giac::eval(res,1,contextptr));
	string ss=res.print(contextptr);
	if (!tegral(f,t,a,b,1e-6,1<<10,res,false,contextptr))
	  confirm("Numerical Integration Error",ss.c_str());
	else {
	  confirm(ss.c_str(),res.print(contextptr).c_str());
	  sto(res,gen((operation==9?"Area":"Arclength"),contextptr),contextptr);	  
	}
      }
      if (operation>=1 && operation<=8 && sol.type==_DOUBLE_ && !is_zero(tstep)){
	tracemode_i=(sol._DOUBLE_val-tmin._DOUBLE_val)/tstep._DOUBLE_val;
	G=subst(parameq,t,sol._DOUBLE_val,false,contextptr);
      }
    }
    if (G.is_symb_of_sommet(at_cercle)){
      if (operation==-1){
	gen c,r;
	centre_rayon(G,c,r,true,contextptr);
	curve_infos1="Circle radius "+r.print(contextptr);
	curve_infos2="Center "+_coordonnees(c,contextptr).print(contextptr);
      }
      G=G._SYMBptr->feuille[0];
    }
    if (G.type==_VECT){
      vecteur & v=*G._VECTptr;
      if (operation==-1 && curve_infos1.size()==0){
	if (v.size()==2)
	  curve_infos1=_equation(G_orig,contextptr).print(contextptr);
	else if (v.size()==4)
	  curve_infos1="Triangle";
	else curve_infos1="Polygon";
	curve_infos2=G.print(contextptr);
      }
      int i=std::floor(tracemode_i);
      double id=tracemode_i-i;
      if (i>=int(v.size()-1)){
	tracemode_i=i=v.size()-1;
	id=0;
      }
      if (i<0){
	tracemode_i=i=0;
	id=0;
      }
      G=v[i];
      if (!is_zero(tstep) && id>0)
	G=v[i]+id*tstep*(v[i+1]-v[i]);
    }
    G=evalf(G,1,contextptr);
    if (operation==3){ // intersect this curve with all other curves
      vecteur V;
      for (int j=0;j<tracemode_v.size();++j){
	if (j==tracemode_n)
	  continue;
	gen H=tracemode_v[j];
	gen I=_inter(makesequence(G_orig,H),contextptr);
	if (I.type==_VECT)
	  V=mergevecteur(V,*I._VECTptr);
      }
      sto(V,gen("Intersect",contextptr),contextptr);
      tracemode_disp.clear();
      tracemode_disp.push_back(put_attributs(V,vecteur(1,_POINT_WIDTH_6 | _red),contextptr));
      if (!V.empty()){
	gen I1(undef),I2(undef),d1(plus_inf),d2(plus_inf);
	for (int i=0;i<V.size();++i){
	  gen cur=evalf_double(V[i],1,contextptr);
	  if (i==0){
	    I1=cur; d1=distance2pp(I1,G,contextptr);
	    continue;
	  }
	  if (i==1){
	    I2=cur; d2=distance2pp(I2,G,contextptr);
	    if (is_strictly_greater(d1,d2,contextptr)){
	      swapgen(I1,I2); swapgen(d1,d2);
	    }
	    continue;
	  }
	  gen d=distance2pp(cur,G,contextptr);
	  if (is_strictly_greater(d1,d,contextptr)){
	    I2=I1; d2=d1;
	    I1=cur; d1=d;
	    continue;
	  }
	  if (is_strictly_greater(d2,d,contextptr)){
	    I2=cur; d2=d;
	  }
	} // end for loop in V
	G=remove_at_pnt(I2);
	I1=put_attributs(I1,vecteur(1,_POINT_WIDTH_6 | _BLUE),contextptr);
	tracemode_disp.push_back(I1);      
	if (is_undef(I2)) I2=I1;
	I2=put_attributs(I2,vecteur(1,_POINT_WIDTH_6 | _BLUE),contextptr);
	tracemode_disp.push_back(I2);      
	// function curve: set nearest intersection as mark/position
	if (t==x && !is_zero(tstep)){
	  gen Ix,Iy;
	  reim(remove_at_pnt(I1),Ix,Iy,contextptr);
	  tracemode_mark=Ix._DOUBLE_val;
	  reim(remove_at_pnt(I2),Ix,Iy,contextptr);
	  tracemode_i=((Ix-tmin)/tstep)._DOUBLE_val;
	}
      }
    } // end intersect
    gen Gx,Gy; reim(G,Gx,Gy,contextptr);
    Gx=evalf_double(Gx,1,contextptr);
    Gy=evalf_double(Gy,1,contextptr);
    if (operation==-1){
      if (curve_infos1.size()==0)
	curve_infos1="Position "+Gx.print(contextptr)+","+Gy.print(contextptr);
      if (G_orig.is_symb_of_sommet(at_pnt)){
	gen f=G_orig._SYMBptr->feuille;
	if (f.type==_VECT && f._VECTptr->size()==3){
	  f=f._VECTptr->back();
	  curve_infos1 = f.print(contextptr)+": "+curve_infos1;
	}
      }
      if (confirm(curve_infos1.c_str(),curve_infos2.c_str())==KEY_CTRL_F1 && tstep!=0){
	double t0=tmin._DOUBLE_val,ts,tc=t0;
	ts=find_tick(tstep._DOUBLE_val*5);
	t0=int(t0/ts)*ts;
	int ndisp=10,N=6,dy=0;
	for (;;){
#ifdef NUMWORKS
	  statuslinemsg("Back: quit, up/down: move");
#else
	  statuslinemsg("esc: quit, up/down: move");
#endif
	  // table of values
	  drawRectangle(0,dy,LCD_WIDTH_PX,LCD_HEIGHT_PX-dy,_WHITE);
	  if (t==x){
	    os_draw_string(0,dy,_BLACK,_WHITE,"x");
	    os_draw_string(120,dy,_BLACK,_WHITE,y.print().c_str());
	  }
	  else {
	    os_draw_string(0,dy,_BLACK,_WHITE,"t");
	    os_draw_string(107,dy,_BLACK,_WHITE,"x");
	    os_draw_string(214,dy,_BLACK,_WHITE,"y");
	  }
	  vecteur V;
	  for (int i=1;i<=ndisp;++i){
	    double tcur=tc+(i-1)*ts;
	    vecteur L(1,tcur);
	    os_draw_string(0,dy+i*18,_BLACK,_WHITE,printn(tcur,N).c_str());
	    if (t==x){
	      gen cur=subst(y,t,tcur,false,contextptr);
	      L.push_back(cur);
	      os_draw_string(120,dy+i*18,_BLACK,_WHITE,printn(cur,N).c_str());
	    }
	    else {
	      gen cur=subst(x,t,tcur,false,contextptr);
	      L.push_back(cur);
	      os_draw_string(107,dy+i*18,_BLACK,_WHITE,printn(cur,N).c_str());
	      cur=subst(y,t,tcur,false,contextptr);
	      L.push_back(cur);
	      os_draw_string(214,dy+i*18,_BLACK,_WHITE,printn(cur,N).c_str());	      
	    }
	    V.push_back(L);
	  }
	  int key=getkey(1);
	  if (key==KEY_CTRL_EXIT || key==KEY_CTRL_OK)
	    break;
	  if (key==KEY_CTRL_UP)
	    tc -= (ndisp/2)*ts;
	  if (key==KEY_CTRL_DOWN)
	    tc += (ndisp/2)*ts;
	  if (key=='+')
	    ts /= 2;
	  if (key=='-')
	    ts *= 2;
	  if (key==KEY_CTRL_DEL && inputdouble("step",ts,contextptr))
	    ts=fabs(ts);
	  if (key==KEY_CTRL_LEFT)
	    inputdouble("min",tc,contextptr);
	  if (key==KEY_CTRL_CLIP)
	    copy_clipboard(gen(V).print(contextptr),true);
	}
      }
    }
    tracemode_add="";
    if (Gx.type==_DOUBLE_ && Gy.type==_DOUBLE_){
      tracemode_add += "x="+print_DOUBLE_(Gx._DOUBLE_val,3)+",y="+print_DOUBLE_(Gy._DOUBLE_val,3);
      if (tstep!=0){
	gen curt=tmin+tracemode_i*tstep;
	if (curt.type==_DOUBLE_){
	  if (t!=x)
	    tracemode_add += ", t="+print_DOUBLE_(curt._DOUBLE_val,3);
	  if (tracemode & 2){
	    gen G1=derive(parameq,t,contextptr);
	    gen G1t=subst(G1,t,curt,false,contextptr);
	    gen G1x,G1y; reim(G1t,G1x,G1y,contextptr);
	    gen m=evalf_double(G1y/G1x,1,contextptr);
	    if (m.type==_DOUBLE_)
	      tracemode_add += ", m="+print_DOUBLE_(m._DOUBLE_val,3);
	    gen T(_vector(makesequence(_point(G,contextptr),_point(G+G1t,contextptr)),contextptr));
	    tracemode_disp.push_back(T);
	    gen G2(derive(G1,t,contextptr));
	    gen G2t=subst(G2,t,curt,false,contextptr);
	    gen G2x,G2y; reim(G2t,G2x,G2y,contextptr);
	    gen det(G1x*G2y-G2x*G1y);
	    gen Tn=sqrt(G1x*G1x+G1y*G1y,contextptr);
	    gen R=evalf_double(Tn*Tn*Tn/det,1,contextptr);
	    gen centre=G+R*(-G1y+cst_i*G1x)/Tn;
	    if (tracemode & 4){
	      gen N(_vector(makesequence(_point(G,contextptr),_point(centre,contextptr)),contextptr));
	      tracemode_disp.push_back(N);
	    }
	    if (tracemode & 8){
	      if (R.type==_DOUBLE_)
		tracemode_add += ", R="+print_DOUBLE_(R._DOUBLE_val,3);
	      tracemode_disp.push_back(_cercle(makesequence(centre,R),contextptr));
	    }
	  }
	}
      }
    }
    double x_scale=LCD_WIDTH_PX/(window_xmax-window_xmin);
    double y_scale=LCD_HEIGHT_PX/(window_ymax-window_ymin);
    double i,j;
    findij(G,x_scale,y_scale,i,j,contextptr);
    current_i=int(i+.5);
    current_j=int(j+.5);
    python_compat(p,contextptr);
  }

  void Graph2d::invert_tracemode(){
    if (!tracemode)
      init_tracemode();
    else
      tracemode=0;
  }

  void Graph2d::set_mode(const giac::gen & f_tmp,const giac::gen & f_final,int m,const string & help){
    approx=true;
    mode=m;
    selected.clear();
    redraw();
    args_help.clear();
    if (mode!=0 && mode!=255){
      int oldmode=calc_mode(contextptr);
      calc_mode(0,contextptr);
      gen g(help,contextptr);
      calc_mode(oldmode,contextptr);
      if (g.type==_VECT){
	const_iterateur it = g._VECTptr->begin(),itend=g._VECTptr->end();
	for (;it!=itend;++it)
	  args_help.push_back(it->print(contextptr));
      }
      else
	args_help.push_back(g.print(contextptr));
    }
    if (mode==255)
      modestr=gettext("Frame");
    else
      modestr=mode?gen2string(f_final):gettext("Pointer");
    if (mode>=-1){
      pushed=false;
      moving_param=moving=moving_frame=false;
      // history_pos=-1;
      mode=m;
      function_final=f_final;
      function_tmp=f_tmp;
      args_tmp.clear();
      geo_handle(FL_MOVE,0);
      update_g();
    }
  }

  vecteur Graph2d::selection2vecteur(const vector<int> & v){
    int n=v.size();
    vecteur res(n);
    for (int i=0;i<n;++i){
      res[i]=plot_instructions[v[i]];
    }
    return res;
  }

  int findfirstclosedcurve(const vecteur & v){
    int s=v.size();
    for (int i=0;i<s;++i){
      gen g=remove_at_pnt(v[i]);
      if (g.is_symb_of_sommet(at_cercle))
	return i;
      if (g.type==_VECT && g.subtype==_GROUP__VECT){
	vecteur & w=*g._VECTptr;
	if (!w.empty() && w.front()==w.back())
	  return i;
      }
    }
    return -1;
  }

  void Graph2d::geometry_round(double x,double y,double z,double eps,gen & tmp,GIAC_CONTEXT)  {
    tmp=is3d?geometry_round_numeric(x,y,z,eps,approx):geometry_round_numeric(x,y,eps,approx);
    selected=nearest_point(plot_instructions,is3d?geometry_round_numeric(x,y,z,eps,true):geometry_round_numeric(x,y,eps,true),eps,contextptr);
    // bug bonux: when a figure is saved, plot_instructions is saved
    // if there are sequences in plot_instructions
    // they are not put back in an individual level
    while (!selected.empty() && selected.back()>=hp->elements.size())
      selected.pop_back();
  }

  gen Graph2d::geometry_round(double x,double y,double z,double eps,gen & original,int & pos,bool selectfirstlevel,bool setscroller) {
    if (!hp)
      return undef;
    gen tmp;
    pos=-1;
    geometry_round(x,y,z,eps,tmp,contextptr);
    if (selected.empty())
      return tmp;
    if (function_final==at_areaatraw || function_final==at_areaat || function_final==at_perimeteratraw || function_final==at_perimeterat){
      int p=findfirstclosedcurve(selection2vecteur(selected));
      if (p>0){
	pos=p;
      }
    }
    if (pos==-1){
      if (selectfirstlevel){
	sort(selected.begin(),selected.end());
	// patch so that we move element and not the curve
	int p=findfirstpoint(selection2vecteur(selected));
	if (p>0){
	  pos=p;
	}
      }
      else
	pos=findfirstpoint(selection2vecteur(selected));
    }
    gen g=symbolic_instructions[ (pos<0)?(pos=selected.front()):(pos=selected[pos]) ];
    if (pos>=0 && pos<hp->elements.size()){
      // hp->_sel_begin=hp->_sel_end=pos;
      // if (setscroller) hp->line=pos;
    }
    if (g.is_symb_of_sommet(at_plus) && g._SYMBptr->feuille.type==_VECT && !g._SYMBptr->feuille._VECTptr->empty())
      g=g._SYMBptr->feuille._VECTptr->front();
    if (g.is_symb_of_sommet(at_sto) && g._SYMBptr->feuille.type==_VECT ){
      vecteur & v = *g._SYMBptr->feuille._VECTptr;
      if (v.size()==2){
	original = v[0];
	tmp = v[1];
	if (tmp.type==_IDNT){
	  gen valeur=protecteval(original,1,contextptr);
	  if (valeur.is_symb_of_sommet(at_pnt)){
	    gen & valf = valeur._SYMBptr->feuille;
	    if (valf.type==_VECT){
	      vecteur & valv = *valf._VECTptr;
	      int s=v.size();
	      if (s>1){
		gen valv1=valv[1];
		if (valv1.type==_VECT && valv1._VECTptr->size()>2){
		  tmp=symbolic(at_extract_measure,v[1]);
		}
	      }
	    }
	  }
	}
      }
    }
    return tmp;
  }

  void Graph2d::autoname_plus_plus(){
    if (hp){
      string s=autoname(contextptr);
      giac::autoname_plus_plus(s);
      autoname(s,contextptr);
    }
  }
  
  int Graph2d::geo_handle(int event,int key){
    double eps=find_eps();
    int pos;
    gen tmp,tmp2,decal;
    if (event==FL_PUSH)
      moving_param=false;
    if ( pushed && !moving && !moving_frame && mode ==0 && in_area && event==FL_DRAG){
      // FIXME? redraw();
      return 1;
    }
    if (mode>=2 && event==FL_MOVE && args_tmp.size()>mode)
      event=FL_RELEASE;
    if ( in_area && ((mode!=1 && event==FL_DRAG) || event==FL_PUSH || event==FL_RELEASE || (mode>=2 && event==FL_MOVE)) ){
      double newx,newy,newz;
      find_xyz(current_i,current_j,current_depth,newx,newy,newz);
      round3(newx,window_xmin,window_xmax);
      round3(newy,window_ymin,window_ymax);
      if (is3d)
	round3(newz,window_zmin,window_zmax);
      tmp=geometry_round(newx,newy,newz,eps,tmp2,pos,mode==0 || (args_tmp.size()==mode && function_final.type==_FUNC && equalposcomp(transformation_functions,*function_final._FUNCptr)),event==FL_RELEASE);
      if (tmp.type!=_IDNT && !tmp.is_symb_of_sommet(at_extract_measure)){
	bool done=false;
	if (mode==0 && event==FL_PUSH && current_i<192 && current_j<14*nparams+21){
	  double d=current_j/14.-1;
	  vecteur vp=param(d);
	  if (vp.size()==2){
	    tmp=vp[0][0];
	    tmp2=vp[0];
	    pos=vp[1].val;
	    done=moving_param=true;
	    param_min=evalf_double(tmp2[1],1,contextptr)._DOUBLE_val;
	    param_max=evalf_double(tmp2[2],1,contextptr)._DOUBLE_val;
	    param_step=evalf_double(tmp2[4],1,contextptr)._DOUBLE_val;
	    param_orig=param_value=evalf_double(tmp2[3],1,contextptr)._DOUBLE_val;
	  }
	}
	if (!done){
	  if (tmp.type==_VECT && tmp._VECTptr->size()==3){
	    tmp.subtype=_SEQ__VECT;
	    tmp=symbolic(at_point,tmp);
	  }
	  else
	    tmp=symbolic(at_point,makevecteur(re(tmp,contextptr),im(tmp,contextptr)));
	}
      }
    }
    double newx,newy,newz;
    if (is3d){
      double x1,y1,z1,x2,y2,z2;
      find_xyz(current_i,current_j,current_depth,x1,y1,z1);
      find_xyz(push_i,push_j,push_depth,x2,y2,z2);
      newx=x1-x2; newy=y1-y2; newz=z1-z2;
    } else {
      int dw=LCD_WIDTH_PX,dh=LCD_HEIGHT_PX;
      double dx=window_xmax-window_xmin;
      double dy=window_ymax-window_ymin;
      double x_scale=dx/dw,y_scale=dy/dh;
      newx=(current_i-push_i)*x_scale;
      newy=(push_j-current_j)*y_scale;
      newz=0;
    }
    round3(newx,window_xmin,window_xmax);
    round3(newy,window_ymin,window_ymax);      
    if (is3d){
      round3(newz,window_zmin,window_zmax);
      decal=in_area?geometry_round_numeric(newx,newy,newz,eps,approx):0;
      if (decal.type==_VECT && decal.subtype==_POINT__VECT)
	decal.subtype=0;
    }
    else
      decal=in_area?geometry_round_numeric(newx,newy,eps,approx):0;
    // cerr << in_area << " " << decal << '\n';
    if (mode==0 || mode==255) {
      if (event==FL_PUSH){ 
	// select object && flag to move it 
	if (mode==0 && pos>=0){
	  if (tmp.type==_IDNT){
	    drag_original_value=tmp2;
	    drag_name=tmp;
	  }
	  else {
	    drag_original_value=symbolic_instructions[pos];
	    drag_name=0;
	  }
	  hp_pos=pos;
	  moving = true;
	}
	else { // nothing selected, move frame
	  if (!(display_mode & 0x80)) // disabled by default in 3-d 
	    moving_frame=true;
	}
	return 1;
      }
      if (moving_frame && (event==FL_DRAG || event==FL_RELEASE) ){
	window_xmin -= newx;
	window_xmax -= newx;
	window_ymin -= newy;
	window_ymax -= newy;
	window_zmin -= newz;
	window_zmax -= newz;
	push_i = current_i;
	push_j = current_j;
	push_depth = current_depth;
	redraw();
	if (event==FL_RELEASE)
	  moving_frame=false;
	return 1;
      }
      if (mode==255)
	return 0;
      if (moving_param && (event==FL_DRAG || event==FL_RELEASE) ){
	// key ->
	if (key==KEY_CTRL_EXIT)
	  param_value=param_orig;
	double ps=param_step;
	if (ps<=0)
	  ps=(param_max-param_min)/100;
	if (key==KEY_CTRL_LEFT)
	  param_value -= ps;
	if (key==KEY_SHIFT_LEFT)
	  param_value -= 10*ps;
	if (key==KEY_CTRL_RIGHT)
	  param_value += ps;
	if (key==KEY_SHIFT_RIGHT)
	  param_value += 10*ps;
	if (param_value<param_min)
	  param_value=param_min;
	if (param_value>param_max)
	  param_value=param_max;
	current_i=64+128*(param_value-param_min)/(param_max-param_min);
	if (param_step<=0)
	  do_handle(symbolic(at_assume,symb_equal(drag_name,param_value)));
	else {
	  gen newval=symbolic(at_element,makesequence(symb_interval(param_min,param_max),param_value));
	  do_handle(symbolic(at_sto,makevecteur(newval,drag_name)));
	}
	if (event==FL_RELEASE){
	  moving_param=moving=false;
	}
	return 1;
      }
      if (moving && (event==FL_DRAG || event==FL_RELEASE) ){
	// cerr << current_i << " " << current_j << '\n';
	// avoid point()+complex+complex+complex
	gen newval=drag_original_value;
	if (in_area && key!=KEY_CTRL_EXIT){
	  if (drag_original_value.is_symb_of_sommet(at_plus) && drag_original_value._SYMBptr->feuille.type==_VECT && drag_original_value._SYMBptr->feuille._VECTptr->size()>=2){
	    vecteur v=*drag_original_value._SYMBptr->feuille._VECTptr;
	    if (v[1].is_symb_of_sommet(at_nop))
	      v[1]=v[1]._SYMBptr->feuille;
	    newval=symbolic(at_plus,makevecteur(v[0],symbolic(at_nop,ratnormal(_plus(vecteur(v.begin()+1,v.end()),contextptr)+decal))));
	  }
	  else {
	    newval=is_zero(decal)?drag_original_value:symbolic(at_plus,makevecteur(drag_original_value,symbolic(at_nop,decal)));
	  }
	}
	int dclick = 0 || drag_original_value.type==_VECT;
	if (!dclick){
	  if (drag_name.type==_IDNT)
	    do_handle(symbolic(at_sto,makevecteur(newval,drag_name)));
	  else
	    do_handle(newval);
	}
	if (event==FL_RELEASE)
	  moving=false;
	selected.clear();
	redraw();
	return 1;
      }
      return 0;
    }
    selected.clear();
    if (mode==1){
      if (function_final!=at_point){
	if (event==FL_RELEASE){
	  string args=autoname(contextptr)+":=";
	  if (function_final.type==_FUNC)
	    args += function_final._FUNCptr->ptr()->s;
	  args +="(";
	  if (function_final==at_plotode)
	    args += fcnfield + "," + fcnvars + "," +tmp.print(contextptr) + ",plan)";
	  else
	    args += tmp.print(contextptr) + ")";
	  autoname_plus_plus();
	  set_gen_value(-1,gen(args,contextptr),true);
	}
	if (event==FL_PUSH || event==FL_DRAG || event==FL_RELEASE)
	  return 1;
	return 0;
      }
      // point|segment mode
      if (event==FL_RELEASE){
	hp_pos=-1;
	if (hp && !args_tmp.empty() && (std::abs(push_i-current_i)>npixels || std::abs(push_j-current_j)>npixels || (is3d && std::abs(push_depth-current_depth) >0)) ){
	  // make a segment
	  gen val1,val2;
	  if (in_area && args_tmp.front().is_symb_of_sommet(at_point)){
	    val1=gen(autoname(contextptr),contextptr);
	    // put in last history pack level
	    set_gen_value(-1,symbolic(at_sto,makevecteur(add_attributs(args_tmp.front(),couleur,contextptr),val1)),false);
	    hp_pos=hp->elements.size()-1;
	    autoname_plus_plus();
	  }
	  else 
	    val1=args_tmp.front();
	  if (in_area && tmp.is_symb_of_sommet(at_point)){
	    val2=gen(autoname(contextptr),contextptr);
	    gen tmp3=symbolic(at_sto,makevecteur(add_attributs(tmp,couleur,contextptr),val2));
	    set_gen_value(-1,tmp3,false);
	    if (hp_pos<0) hp_pos=hp->elements.size()-1;
	    autoname_plus_plus();
	  }
	  else 
	    val2=tmp;
	  if (in_area){
	    gen tmp3=add_attributs(symbolic(at_segment,makevecteur(val1,val2)),couleur,contextptr);
	    string v1v2=val1.print(contextptr)+val2.print(contextptr);
	    gen g1g2(v1v2,contextptr);
	    if (g1g2.type!=_IDNT)
	      g1g2=gen(v1v2+"_",contextptr);
	    tmp3=symbolic(at_sto,makevecteur(tmp3,g1g2));
	    set_gen_value(-1,tmp3,false);
	    if (hp_pos<0) hp_pos=hp->elements.size()-1;
	    eval(hp_pos);
	  }
	  return 1;
	}
	if (in_area && tmp.type!=_IDNT)
	  do_handle(symbolic(at_sto,makevecteur(add_attributs(tmp,couleur,contextptr),gen(autoname(contextptr),contextptr))));
	// element
	if (tmp.type==_IDNT && tmp2.type==_SYMB && !equalposcomp(point_sommet_tab_op,tmp2._SYMBptr->sommet)){
	  // tmp2 is the geo object, find parameter value
	  double newx,newy,newz;
	  find_xyz(current_i,current_j,current_depth,newx,newy,newz);
	  round3(newx,window_xmin,window_xmax);
	  round3(newy,window_ymin,window_ymax);
	  gen t=projection(evalf(tmp2,1,contextptr),gen(newx,newy),contextptr);
	  if (is_undef(t))
	    return 0;
	  gen tmp3=symbolic(at_element,( (t.type<_IDNT || t.type==_VECT)?gen(makevecteur(tmp,t),_SEQ__VECT):tmp));
	  tmp3=symbolic(at_sto,makevecteur(add_attributs(tmp3,couleur,contextptr),gen(autoname(contextptr),contextptr)));
	  set_gen_value(-1,tmp3,false);
	  if (hp_pos<0) hp_pos=hp->elements.size()-1;
	  eval(hp_pos);
	}
	return 1;
      }
      if (event==FL_PUSH){
	args_tmp=vecteur(1,tmp);
	return 1;
      }
      if (event==FL_DRAG){
	redraw();
	return 1;
      }
      return 0;
    }
    gen tmpval=remove_at_pnt(tmp.eval(1,contextptr));
    gen somm=symbolic(at_sommets,tmp);
    int npoints=1;
    if (!equalposcomp(nosplit_polygon_function,*function_final._FUNCptr)){
      if (tmpval.type==_VECT && tmpval.subtype==_GROUP__VECT)
	npoints=tmpval._VECTptr->size();
      if (tmpval.is_symb_of_sommet(at_cercle))
	npoints=is3d?3:2;
    }
    unsigned args_size=args_tmp.size();
    // mode>=2
    if (event==FL_MOVE || event==FL_DRAG || event==FL_RELEASE || event==FL_PUSH){
      if (args_size<args_tmp_push_size)
	args_tmp_push_size=args_size;
      args_tmp.erase(args_tmp.begin()+args_tmp_push_size,args_tmp.end());
    }
    unsigned new_args_size=args_tmp.size();
    gen tmp_push=tmp;
    bool swapargs=false;
    if (npoints==2 && (new_args_size==2 || new_args_size==1) && (function_final==at_angleat || function_final==at_angleatraw) ){
      // search if args_tmp[0] or args_tmp[1] is a vertex of tmp
      gen tmp2=remove_at_pnt(evalf(tmp,1,contextptr));
      gen somm=symbolic(at_sommets,tmp);
      if (tmp2.type==_VECT && tmp2._VECTptr->size()==2){
	gen tmpa=remove_at_pnt(evalf(args_tmp[0],1,contextptr));
	gen tmpb=new_args_size==2?remove_at_pnt(evalf(args_tmp[1],1,contextptr)):undef;
	if (npoints==2 && tmpa==tmp2._VECTptr->front() && tmpb!=tmp2._VECTptr->back()){
	  tmp=symbolic(at_at,gen(makevecteur(somm,1),_SEQ__VECT));
	  npoints=1;
	}
	if (npoints==2 && tmpa==tmp2._VECTptr->back() && tmpb!=tmp2._VECTptr->front()){
	  tmp=symbolic(at_at,gen(makevecteur(somm,0),_SEQ__VECT));
	  npoints=1;
	}
	if (npoints==2 && tmpb==tmp2._VECTptr->front() && tmpa!=tmp2._VECTptr->back() ){
	  swapargs=true;
	  tmp=symbolic(at_at,gen(makevecteur(somm,1),_SEQ__VECT));
	  npoints=1;
	}
	if (npoints==2 && tmpb==tmp2._VECTptr->back() && tmpa!=tmp2._VECTptr->front()){
	  swapargs=true;
	  tmp=symbolic(at_at,gen(makevecteur(somm,0),_SEQ__VECT));
	  npoints=1;
	}
      }
    }
    if (npoints+args_tmp.size()>mode)
      npoints=1;
    if (event==FL_MOVE || event==FL_DRAG || event==FL_RELEASE){
      if (args_size && args_tmp_push_size && args_push!=tmp_push){
	// replace by current mouse position
	if (npoints==1)
	  args_tmp.push_back(tmp);
	else {
	  gen somm=symbolic(at_sommets,tmp);
	  for (int i=0;i<npoints;++i){
	    args_tmp.push_back(symbolic(at_at,gen(makevecteur(somm,i),_SEQ__VECT)));
	  }
	}
	redraw();
	if (event!=FL_RELEASE || (abs(push_i-current_i)<=5 && abs(push_j-current_j)<=5))
	  return 1;
      }
    }
    if (event==FL_PUSH){
      if (swapargs)
	swapgen(args_tmp[0],args_tmp[1]);
      args_push=tmp_push;
      if (npoints==1)
	args_tmp.push_back(tmp);
      else {
	for (int i=0;i<npoints;++i){
	  args_tmp.push_back(symbolic(at_at,gen(makevecteur(somm,i),_SEQ__VECT)));
	}
      }
      args_tmp_push_size=args_tmp.size();
      redraw();
      return 1;
    }
    if (event==FL_RELEASE){
      int s=args_tmp.size();
      args_tmp_push_size=s;
      if (mode>1 && s>=mode){
	if (s>mode){ 
	  args_tmp=vecteur(args_tmp.begin(),args_tmp.begin()+mode);
	  s=mode;
	}
	gen tmp_plot;
	if (in_area && function_final.type==_FUNC) {
	  gen res,objname=gen(autoname(contextptr),contextptr);
	  hp_pos=hp->elements.size();
	  if (hp_pos && hp->elements[hp_pos-1].s.empty())
	    --hp_pos;
	  // hp->update_pos=hp_pos;
	  int pos0=hp_pos;
	  unary_function_ptr * ptr=function_final._FUNCptr;
	  int ifinal=mode;
	  if (equalposcomp(measure_functions,*ptr))
	    ifinal--;
	  // first replace points in args_tmp by assignations
	  for (int i=0;i<ifinal;++i){
	    tmp_plot=args_tmp[i];
	    if (tmp_plot.is_symb_of_sommet(at_point)){
	      tmp_plot=symbolic(at_sto,makevecteur(add_attributs(tmp_plot,couleur,contextptr),objname));
	      args_tmp[i]=objname;
	      set_gen_value(hp_pos,tmp_plot,false);
	      add_entry(hp_pos+1);
	      ++hp_pos;
	      autoname_plus_plus();
	      objname=gen(autoname(contextptr),contextptr);
	    }
	  }
	  vecteur argv=args_tmp;
	  if (*ptr==(is3d?at_sphere:at_cercle)){
	    gen argv1;
#ifdef NO_STDEXCEPT
	    argv1=evalf(args_tmp.back(),1,contextptr);
	    argv1=evalf_double(argv1,1,contextptr);
#else
	    try {
	      argv1=evalf(args_tmp.back(),1,contextptr);
	      argv1=evalf_double(argv1,1,contextptr);
	    }
	    catch (std::runtime_error & e){
	      argv1=undef;
	    }
#endif
	    if (argv1.is_symb_of_sommet(at_pnt) ||argv1.type==_IDNT){
	      argv1=remove_at_pnt(argv1);
	      if ( (argv1.type==_VECT && argv1.subtype==_POINT__VECT) || argv1.type==_CPLX || argv1.type==_IDNT)
	      argv.back()=args_tmp.back()-args_tmp.front();
	    }
	  }
	  tmp_plot=symbolic(*ptr,gen(argv,_SEQ__VECT));
#ifdef NO_STDEXCEPT
	  res=evalf(tmp_plot,1,contextptr);
#else
	  try {
	    res=evalf(tmp_plot,1,contextptr);
	  }
	  catch (std::runtime_error & err){
	    res=undef;
	  }
#endif
	  tmp_plot=symbolic(at_sto,makevecteur(add_attributs(tmp_plot,couleur,contextptr),objname));
	  set_gen_value(hp_pos,tmp_plot,false);
	  add_entry(hp_pos+1);
	  ++hp_pos;
	  if (res.is_symb_of_sommet(at_pnt)){
	    res=remove_at_pnt(res);
	    int ns=0;
	    if (res.type==_VECT && res.subtype==_GROUP__VECT && (ns=res._VECTptr->size())>2){
	      vecteur l;
	      if (res._VECTptr->back()==res._VECTptr->front())
		--ns;
	      if (function_final.type==_FUNC && equalposcomp(transformation_functions,*function_final._FUNCptr)){
		vecteur argv;
		gen objn,som=symbolic(at_sommets,objname);
		for (int i=1;i<=ns;++i){
		  tmp_plot=symbolic(at_at,gen(makevecteur(som,i-1),_SEQ__VECT));
		  objn=gen(autoname(contextptr)+print_INT_(i),contextptr);
		  argv.push_back(objn);
		  set_gen_value(hp_pos,symbolic(at_sto,gen(makevecteur(tmp_plot,objn),_SEQ__VECT)),false);
		  add_entry(hp_pos+1);
		  ++hp_pos;
		}
		for (int i=1;i<=ns;++i){
		  tmp_plot=symbolic(at_segment,makevecteur(argv[i-1],argv[i%ns]));
		  set_gen_value(hp_pos,symbolic(at_sto,makevecteur(add_attributs(tmp_plot,couleur,contextptr),gen(autoname(contextptr)+print_INT_(ns+i),contextptr))),false);
		  add_entry(hp_pos+1);
		  ++hp_pos;
		}		
	      }
	      else {
		for (int i=mode;i<ns;++i){
		  tmp_plot=symb_at(
				   symbolic(at_sommets,gen(autoname(contextptr),contextptr))
				   ,i,contextptr);
		  gen newname=gen(autoname(contextptr)+print_INT_(i),contextptr);
		  set_gen_value(hp_pos,symbolic(at_sto,makevecteur(tmp_plot,newname)),false);
		  args_tmp.push_back(newname);
		  add_entry(hp_pos+1);
		  ++hp_pos;
		}
		for (int i=1;i<=ns;++i){
		  tmp_plot=symbolic(at_segment,makevecteur(args_tmp[i-1],args_tmp[i%ns]));
		  set_gen_value(hp_pos,symbolic(at_sto,makevecteur(add_attributs(tmp_plot,couleur,contextptr),gen(autoname(contextptr)+print_INT_(ns+i),contextptr))),false);
		  add_entry(hp_pos+1);
		  ++hp_pos;
		}
	      }
	    } // if res.type==_VECT
	  }
	  autoname_plus_plus();
	  // hp->undo_position=save_undo_position;
	  eval(pos0);
	}
	args_tmp.clear();
	args_tmp_push_size=0;
      }
      redraw();
      return 1;
    }
    return 0;
  }

  void geosave(textArea * text,GIAC_CONTEXT){
    string s=remove_extension(text->filename);
    gen tmp(s,contextptr);
    if (tmp.type==_IDNT){
      sto(makevecteur(at_pnt,string2gen(merge_area(text->elements),false)),tmp,contextptr);
      return;
    }
    else {
      for (int i=0;i<10;++i){
	string s1=s+print_INT_(i);
	tmp=gen(s1,contextptr);
	if (tmp.type==_IDNT){
	  confirm(lang==1?"Nom de sauvegarde reserve":"Unable to use reserved name",((lang==1?"Nom utilise ":"Name used ")+s1).c_str());
	  sto(makevecteur(at_pnt,string2gen(merge_area(text->elements),false)),tmp,contextptr);
	  return;
	}
      }
    }
    confirm(lang==1?"Nom de sauvegarde reserve":"Unable to use reserved name",lang==1?"Sauvegarde impossible":"Unable to save");
  }

  void geohelp(GIAC_CONTEXT){
    textArea text;
    text.editable=false;
    text.clipline=-1;
    text.title = (char*)((lang==1)?"Aide":"Help");
    text.allowF1=false;
    text.python=false;
    add(&text,lang==1?
	"x,n,t ou tab: etude courbe\nshift-2: info courbe\nshift-3: tangente, pente\nshift-4: normale\nshift-5: cercle osculateur\nshift-7: infos courbe on/off\nhaut/bas/droit/gauche: deplace pointeur ou change point de vue\nalpha-haut/bas/droit/gauche: modifie fenetre\ny^x ou e^x: trace 3d precis\nEsc/Back: quitte ou interrompt le trace 3d en cours\n( et ): modifie le rendu des surfaces raides 3d\n0: surfaces cachees 3d ON/OFF\n.: remplissage surface 3d raide ON/OFF\n5 reset 3d view\n7,8,9,1,2,3: deplacement 3d\n\nGeometrie\nF4: change le mode\nLe mode repere (shift 7) permet de changer le point de vue\nLe mode pointeur (shift 8) permet de bouger un objet et les objets dependants avec enter/OK et les touches de deplacement\nLes autres modes permettent de creer des objets\nEsc/Back: permet de passer en vue symbolique et de creer/modifier des objets par des commandes, taper enter/OK pour revenir en vue graphique\n4,6: modifie la profondeur du clic":
	"x,n,t or tab: curve study\nshift-2: curve info\nshift-3: tangent, slope\nshift-4: normal\nshift-5: osculating circle\nshift-7: curve infos on/off\nup/down/right/left: move pointer or modify viewpoint\nalpha-up/down/right/left: move window\nEsc/Back: leave or interrupt 3d rendering\ny^x or e^x: precise 3d\n( and ): modify stiff surfaces 3d rendering\n0: hidden 3d surfaces ON/OFF\n.: fill stiff 3d surfacesON/OFF\n5 reset 3d view\n7,8,9,1,2,3: move 3d view\n\nGeometry\nF4: change geometry mode\nFrame mode (shift F1): modify viewpoint\nPointer mode (shift F2): select an object and move it with enter/OK and cursor keys\nOther modes: create an object\nEsc/Back: go to symbolic view where you can create/modify objects with commands, press enter/OK to go back to graphic view");
    int exec=doTextArea(&text,contextptr);
  }

  string inputparam(char curname,int symbolic,GIAC_CONTEXT){
    Menu paramenu;
    paramenu.numitems=7;
    MenuItem paramenuitems[paramenu.numitems];
    paramenu.items=paramenuitems;
    paramenu.height=8;
    paramenu.title = (char *)"Parameter";
    char menu_xcur[32],menu_xmin[32],menu_xmax[32],menu_xstep[32],menu_name[16]="name a";
    menu_name[5]=curname;
    double pcur=0,pmin=-5,pmax=5,pstep=0.1;
    std::string s;
    bool doit; 
    for (;;){
      s="cur "+giac::print_DOUBLE_(pcur,contextptr);
      strcpy(menu_xcur,s.c_str());
      s="min "+giac::print_DOUBLE_(pmin,contextptr);
      strcpy(menu_xmin,s.c_str());
      s="max "+giac::print_DOUBLE_(pmax,contextptr);
      strcpy(menu_xmax,s.c_str());
      s="step "+giac::print_DOUBLE_(pstep,contextptr);
      strcpy(menu_xstep,s.c_str());
      paramenuitems[0].text = (char *) "OK";
      paramenuitems[1].text = (char *) menu_name;
      paramenuitems[2].text = (char *) menu_xcur;
      paramenuitems[3].text = (char *) menu_xmin;
      paramenuitems[4].text = (char *) menu_xmax;
      paramenuitems[5].text = (char *) menu_xstep;
      paramenuitems[6].text = (char *) "Symbolic";      
      paramenuitems[6].type = MENUITEM_CHECKBOX;
      paramenuitems[6].value = symbolic;
      int sres = doMenu(&paramenu);
      doit = sres==MENU_RETURN_SELECTION  || sres==KEY_CTRL_EXE;
      if (doit) {
	std::string s1; double d;
	if (paramenu.selection==2){
	  handle_f5();
	  if (inputline(menu_name,(lang==1)?"Nouvelle valeur?":"New value?",s1,false)==KEY_CTRL_EXE && s1.size()>0 && isalpha(s1[0])){
	    if (s1.size()>10)
	      s1=s1.substr(0,10);
	    strcpy(menu_name,("name "+s1).c_str());
	  }
	  continue;
	}	
	if (paramenu.selection==3){
	  inputdouble(menu_xcur,pcur,contextptr);
	  continue;
	}
	if (paramenu.selection==4){
	  inputdouble(menu_xmin,pmin,contextptr);
	  continue;
	}
	if (paramenu.selection==5){
	  inputdouble(menu_xmax,pmax,contextptr);
	  continue;
	}
	if (paramenu.selection==6){
	  inputdouble(menu_xstep,pstep,contextptr);
	  pstep=fabs(pstep);
	  continue;
	}
	if (paramenu.selection==7){
	  symbolic=1-symbolic;
	  continue;
	}
	// if (paramenu.selection==6) break;
      } // end menu
      break;
    } // end for (;;)
    if (doit && pmin<pmax && pstep>0){
      if (symbolic){
	s="assume(";
	s += (menu_name+5);
	s += "=[";
	s += (menu_xcur+4);
	s += ',';
	s += (menu_xmin+4);
	s += ',';
	s += (menu_xmax+4);
	s += ',';
	s += (menu_xstep+5);
	s += "])";
      }
      else {
	s=(menu_name+5);
	s += ":=element(";
	s += (menu_xmin+4);
	s += "..";
	s += (menu_xmax+4);
	s += ',';
	s += (menu_xcur+4);
	s += ")";
      }
    } else s="";
    return s;
  }

  void Graph2d::init_tracemode(){
    if (is3d){
      tracemode=0;
      return;
    }
    tracemode_mark=0.0;
    double w=LCD_WIDTH_PX;
    double h=LCD_HEIGHT_PX-STATUS_AREA_PX;
    double window_w=window_xmax-window_xmin,window_h=window_ymax-window_ymin;
    double r=h/w*window_w/window_h;
    tracemode=(r>0.7 && r<1.4)?7:3;
    tracemode_set();
  }

  void Graph2d::curve_infos(){
    if (!tracemode)
      init_tracemode();
    const char *
      tab[]={
	     lang==1?"Infos objet (shift-2)":"Object infos (shift-2)",  // 0
#ifdef NUMWORKS
	     lang==1?"Quitte mode etude (x,n,t)":"Quit study mode (x,n,t)",
#else
	     lang==1?"Quitte mode etude (tab)":"Quit study mode (tab)",
#endif
	     lang==1?"Entrer t ou x":"Set t or x", // 1
	     lang==1?"y=0, racine":"y=0, root",
	     "Intersection", // 3
	     "y'=0, extremum",
	     lang==1?"x'=0 (parametriques)":"x'=0 (parametric)", // 5
	     "Inflexion",
	     lang==1?"Marquer la position":"Mark position",
	     lang==1?"Entrer t ou x, marquer":"Set t or x, mark", // 8
	     lang==1?"Aire":"Area",
	     lang==1?"Longueur d'arc":"Arc length", // 10
	     0};
    const int s=sizeof(tab)/sizeof(char *);
    int choix=select_item(tab,lang==1?"Etude courbes":"Curve study",true);
    if (choix<0 || choix>s)
      return;
    if (choix==1)
      tracemode=0;
    else 
      tracemode_set(choix-1);
  }

  int Graph2d::ui(){
    Graph2d & gr=*this;
    // UI
    int saveprecision=gr.precision;
    gr.precision += 2; // fast draw first
    gr.draw();
    gr.precision=saveprecision;    
    gr.must_redraw=true;
    for (;;){
#ifdef NSPIRE_NEWLIB
      DefineStatusMessage((char*)"shift-1: help, menu: menu, esc: quit", 1, 0, 0);
#else
      DefineStatusMessage((char*)"shift-1: help, home: menu, back: quit", 1, 0, 0);
#endif
      DisplayStatusArea();
      int saveprec=gr.precision;
      if (gr.doprecise){
	gr.doprecise=false;
	gr.precision=1;//gr.precision-=2;
      }
      if (gr.must_redraw)
	gr.draw();
      if (hp){
	string msg=hp->filename+":"+(mode==255?" Frame. Shift-1: help":modestr);
	// help
	int help_pos=args_tmp.empty()?0:args_tmp.size()-1;
	if (help_pos<args_help.size()){
	  msg += " "+args_help[help_pos];
	}
	DefineStatusMessage((char *)msg.c_str(),1,0,0);
	DisplayStatusArea();
      }
      gr.must_redraw=true;
      gr.precision=saveprec;
      if (0 && !hp){
#ifdef NUMWORKS
	os_draw_string(0,LCD_HEIGHT_PX-STATUS_AREA_PX-17,COLOR_BLACK,COLOR_WHITE,"home: cfg");
#else
	os_draw_string(0,LCD_HEIGHT_PX-STATUS_AREA_PX-17,COLOR_BLACK,COLOR_WHITE,"doc: cfg");
#endif
      }
      int key=-1;
      GetKey(&key);
      bool alph=alphawasactive(&key);
      if (key==KEY_SHUTDOWN || key==KEY_CTRL_SYMB)
        return key;
      if (key==KEY_CTRL_F1){
        geohelp(contextptr);
        continue;
      }
      if (key==KEY_CTRL_F2){
	tracemode_set(-1); // object info
	continue;
      }
      if (key==KEY_CTRL_F3){
	if (tracemode & 2)
	  tracemode &= ~2;
	else
	  tracemode |= 2;
	tracemode_set();
	continue;
      }
      if (key==KEY_CTRL_F4){
	if (tracemode & 4)
	  tracemode &= ~4;
	else
	  tracemode |= 4;
	tracemode_set();
	continue;
      }
      if (key==KEY_CTRL_F5){
	if (tracemode & 8)
	  tracemode &= ~8;
	else {
	  tracemode |= 8;
	  orthonormalize(true);
	}
	tracemode_set();
	continue;
      }
      if (key==KEY_CTRL_XTT || key=='\t'){
	curve_infos();
	continue;
      }
      if (!hp && key==KEY_CTRL_F7)
	invert_tracemode();
      if (hp){
	if (key==KEY_CTRL_F7 ){
	  if (mode==255)
	    invert_tracemode();
	  else
	    set_mode(0,0,255,"");
	}
	if (key==KEY_CTRL_F8 )
	  set_mode(0,0,0,"");
	if (key==KEY_CTRL_F9 )
	  set_mode(at_point,at_point,1,"Point");
	if (key==KEY_CTRL_F10)
	  set_mode(at_segment,is3d?at_sphere:at_cercle,2,"Center,Point");
	if (key==KEY_CTRL_F11)
	  set_mode(at_segment,at_triangle,3,"Point1,Point2,Point3");
	if (key>='a' && key<='z'){
	  bool found=false;
	  char ch=key;
	  gen tmp=gen(string("")+ch,contextptr);
	  if (tmp.type==_IDNT){
	    int pos=0;
	    for (int i=0;i<plot_instructions.size();++i){
	      if (plot_instructions[i].is_symb_of_sommet(at_parameter)){
		gen name=plot_instructions[i]._SYMBptr->feuille[0];
		++pos;
		if (name==tmp){
		  current_j=7+14*pos;
		  found=true;
		  break;
		}
	      }
	    }
	  }
	  if (found)
	    continue;
	  key -= 'a'-'A';
	}
	if (key>='A' && key<='Z'){
	  char ch=key;
	  gen tmp=gen(string("")+ch,contextptr);
	  if (tmp.type==_IDNT){
	    tmp=evalf(tmp,1,contextptr);
	    if (tmp.is_symb_of_sommet(at_pnt)){
	      tmp=remove_at_pnt(tmp);
	      if (tmp.is_symb_of_sommet(at_cercle))
		tmp=(tmp._SYMBptr->feuille[0]+tmp._SYMBptr->feuille[1])/2;
	      if (tmp.type==_SYMB)
		tmp=tmp._SYMBptr->feuille;
	      if (tmp.type==_VECT && tmp.subtype!=_POINT__VECT && !tmp._VECTptr->empty())
		tmp=tmp._VECTptr->front();
	      if (is3d && tmp.type==_VECT && tmp._VECTptr->size()==3 && tmp.subtype==_POINT__VECT){
		const vecteur & tv=*tmp._VECTptr;
		gen x=tv[0],y=tv[1],z=tv[2];
		x=evalf_double(x,1,contextptr); 
		y=evalf_double(y,1,contextptr); 
		z=evalf_double(z,1,contextptr);
		if (x.type==_DOUBLE_ && y.type==_DOUBLE_ && z.type==_DOUBLE_){
		  double i,j; double3 d3;
		  xyz2ij(double3(x._DOUBLE_val,y._DOUBLE_val,z._DOUBLE_val),i,j,d3);
		  current_i=i; current_j=j; current_depth=d3.z;
		}
	      }
	      if (!is3d && (tmp.type==_DOUBLE_ || tmp.type==_CPLX)){
		double x_scale=LCD_WIDTH_PX/(window_xmax-window_xmin);
		double y_scale=LCD_HEIGHT_PX/(window_ymax-window_ymin);
		double i,j;
		findij(tmp,x_scale,y_scale,i,j,contextptr);
		current_i=int(i+.5);
		current_j=int(j+.5);
		adjust_cursor_point_type();
		geo_handle(moving?FL_DRAG:FL_MOVE,key);
		continue;
	      }
	    }
	  }
	}
      }
      if (hp && (key==KEY_CTRL_CATALOG || key==KEY_BOOK )){
	tracemode=0;
	const char *
	  tab[]={
		 lang==1?"Mode repere":"Frame mode", // 0
		 lang==1?"Pointeur":"Pointer",
		 lang==1?"Point":"Point", // 2
		 is3d?"Sphere":"Circle",
		 lang==1?"Triangle":"Triangle", // 4
		 lang==1?"Points":"Points",
		 lang==1?"Droites, plans":"Lines, planes", // 6
		 lang==1?"Polygone, polyedre":"Polygon, polyhedron",
		 lang==1?"Cercle, conique, sphere":"Circle, conic, sphere", // 8
		 lang==1?"Courbe, surface":"Curve, surface", // 9
		 lang==1?"Curseur":"Cursor", // 10
		 lang==1?"Transformations":"Transforms",
		 lang==1?"Mesures":"Mesures", // 12
		 lang==1?"Effacer trace":"Clear trace", // -1
		 0};
	const int s=sizeof(tab)/sizeof(char *);
	int choix=select_item(tab,"Mode",true);
	if (choix<0 || choix>s)
	  continue;
	if (choix==s-1){
	  trace_instructions.clear();
	  update_g();
	  continue;
	}
	if (choix<=4){
	  gen ftmp[]={0,0,at_point,at_segment,at_segment};
	  gen ffinal[]={0,0,at_point,is3d?at_sphere:at_cercle,at_triangle};
	  int mode[]={255,0,1,2,3};
	  const char * help[]={"","","Point","Center,Point","Point1,Point2,Point3"};
	  set_mode(ftmp[choix],ffinal[choix],mode[choix],help[choix]);
	  continue;
	}
	draw(); // for small choosebox, we must clean up previous choosebox
	if (choix==5){ // Points
	  const char *
	    tab[]={
		   lang==1?"Point":"Point",
		   lang==1?"Milieu":"Middle point",
		   lang==1?"Centre":"Center",
		   lang==1?"Intersection unique":"Single intersection",
		   lang==1?"Liste d'intersections":"List of intersections",
		   lang==1?"Element":"Element",
		   0};
	  const int s=sizeof(tab)/sizeof(char *);
	  int choix=select_item(tab,"Points",true);
	  if (choix<0 || choix>s)
	    continue;
	  gen ftmp[]={at_point,at_segment,at_centre,at_inter_unique,at_inter,at_element};
	  gen ffinal[]={at_point,at_milieu,at_centre,at_inter_unique,at_inter,at_element};
	  int mode[]={1,2,1,2,2,1};
	  const char * help[]={
			       "Point",
			       "Point1,Point2",
			       "Circle",
			       "Line1,Line2",
			       "Curve1,Curve2",
			       "Curve",
	  };
	  set_mode(ftmp[choix],ffinal[choix],mode[choix],help[choix]);
	  continue;
	}
	if (choix==6){ // Droites
	  const char *
	    tab[]={
		   lang==1?"Segment":"Segment", 
		   lang==1?"Vecteur":"Vector",
		   lang==1?"Demi-droite":"Halfline",
		   lang==1?"Droite":"Line",
		   lang==1?"Plan":"Plane",
		   lang==1?"Parallele":"Parallel",
		   lang==1?"Perpendiculaire":"Perpendicular",
		   lang==1?"Mediatrice":"Perpen_bisector",
		   lang==1?"Bissectrice":"Bisector",
		   lang==1?"Mediane":"Median line",
		   lang==1?"Tangente":"Tangent",
		   0};
	  const int s=sizeof(tab)/sizeof(char *);
	  int choix=select_item(tab,"Droites, segments...",true);
	  if (choix<0 || choix>s)
	    continue;
	  gen ftmp[]={at_segment,at_vector,at_demi_droite,at_droite,at_segment,at_parallele,at_perpendiculaire,at_mediatrice,at_segment,at_segment,at_segment};
	  gen ffinal[]={at_segment,at_vector,at_demi_droite,at_droite,at_plan,at_parallele,at_perpendiculaire,at_mediatrice,at_bissectrice,at_mediane,at_tangent};
	  int mode[]={2,2,2,2,2,3,2,2,3,3,2};
	  const char * help[]={
			       "Point1,Point2",
			       "Point1,Point2",
			       "Point1,Point2",
			       "Point1,Point2",
			       "Point1,Point2,Point3",
			       "Point,Line",
			       "Point,Line",
			       "Point1,Point2",
			       "Sommet_angle,Point2,Point3",
			       "Sommet_angle,Point2,Point3",
			       "Curve,Point"
	  };
	  set_mode(ftmp[choix],ffinal[choix],mode[choix],help[choix]);
	  continue;
	}
	if (choix==7){ // Polygons
	  const char *
	    tab[]={
		   lang==1?"Triangle":"Triangle",
		   lang==1?"Triangle equilateral":"Equilateral triangle",
		   lang==1?"Carre":"Square",
		   lang==1?"Quadrilatere":"Quadrilateral",
		   lang==1?"Polygone":"Polygon",
		   lang==1?"Tetraedre (pyramide)":"Tetrahedron (Pyramid)",
		   lang==1?"Tetraedre regulier":"Regular tetrahedron",
		   lang==1?"Cube":"Cube",
		   lang==1?"Octaedre":"Octahedron",
		   lang==1?"Dodecaedre":"Dodecahedron",
		   lang==1?"Icosaedre":"Icosahedron",
		   0};
	  const int s=sizeof(tab)/sizeof(char *);
	  int choix=select_item(tab,"Droites, segments...",true);
	  if (choix<0 || choix>s)
	    continue;
	  gen ftmp[]={at_polygone_ouvert,at_segment,at_segment,at_polygone_ouvert,at_polygone_ouvert,at_polygone_ouvert,at_polygone_ouvert,at_polygone_ouvert,at_polygone_ouvert,at_polygone_ouvert,at_polygone_ouvert};
	  gen ffinal[]={at_triangle,at_triangle_equilateral,at_carre,at_quadrilatere,at_polygone,at_tetraedre,at_tetraedre,at_cube,at_octaedre,at_dodecaedre,at_icosaedre};
	  int mode[]={3,2,2,4,5,4,3,3,3,3,3};
	  int m=mode[choix];
	  if (choix==4){
	    double d=5;
	    if (inputdouble(lang==1?"Nombre de sommets?":"Number of vertices?",d,contextptr) && d==int(d) && d>=3 && d<20){
	      m=d;
	    }
	    else continue;
	  }
	  const char * help[]={
			       "Point1,Point2,Point3",
			       "Point1,Point2",
			       "Point1,Point2",
			       "Point1,Point2,Point3,Point4",
			       "Point1,Point2,Point3,Point4,Point5",
			       "Point1,Point2,Point3,Point4",
			       "Point1,Point2,Point3",
			       "Point1,Point2,Point3",
			       "Point1,Point2,Point3",
			       "Point1,Point2,Point3",
			       "Point1,Point2,Point3",
	  };
	  set_mode(ftmp[choix],ffinal[choix],m,help[choix]);
	  continue;
	}
	if (choix==8){ // Conics
	  const char *
	    tab[]={
		   lang==1?"cercle":"circle",
		   lang==1?"circonscrit":"circumcircle",
		   lang==1?"inscrit":"incircle",
		   lang==1?"ellipse":"ellipse",
		   lang==1?"hyperbole":"hyperbola",
		   lang==1?"parabole":"parabola",
		   lang==1?"sphere":"sphere",
		   0};
	  const int s=sizeof(tab)/sizeof(char *);
	  int choix=select_item(tab,"Conic",true);
	  if (choix<0 || choix>s)
	    continue;
	  gen ftmp[]={at_segment,at_segment,at_segment,at_segment,at_segment,at_segment,at_segment};
	  gen ffinal[]={at_cercle,at_circonscrit,at_inscrit,at_ellipse,at_hyperbole,at_parabole,at_sphere};
	  int mode[]={2,3,3,3,3,2,2};
	  const char * help[]={
			       "Center,Point",
			       "Point1,Point2,Point3",
			       "Point1,Point2,Point3",
			       "Focus1,Focus2,Point_on_ellipse",
			       "Focus1,Focus2,Point_on_hyperbola",
			       "Focus,Point_or_line",
			       "Center,Point",
	  };
	  set_mode(ftmp[choix],ffinal[choix],mode[choix],help[choix]);
	  continue;
	}
	if (choix==9){ // Curves
	  const char *
	    tab[]={
		   lang==1?"Fonction plot(sin(x))":"Function plot(sin(x))",
		   lang==1?"Param. plotparam([x^2,x^3])":"Param. plotparam([x^2,x^3])",
		   lang==1?"Polaire plotpolar(x)":"Polar plotpolar(x)",
		   lang==1?"Implicit plot(x^2+y^4=6)":"Implicit plot(x^2+y^4=6)",
		   lang==1?"Champ des tangentes":"Plotfield",
		   lang==1?"Solution equa. diff.":"Diff. equa. solution",
		   0};
	  const int s=sizeof(tab)/sizeof(char *);
	  int choix=select_item(tab,"Courbe",true);
	  if (choix<0 || choix>s)
	    continue;
	  const char * cmd[]={"plot()","plotparam()","plotpolar()","plot()","plotfield()","plotode()"};
	  hp->line=hp->add_entry(-1);
	  string mycmd=autoname(contextptr)+":="+cmd[choix];
	  autoname_plus_plus();
	  hp->set_string_value(hp->line,mycmd);
	  hp->pos=mycmd.size()-1;
	  return KEY_CTRL_OK;
	}
	if (choix==10){
	  gen param=0;
	  for (char ch='a';ch<='z';++ch){
	    gen tmp(string("")+ch,contextptr);
	    if (tmp.type!=_IDNT) continue;
	    param=tmp.eval(1,contextptr);
	    if (param==tmp)
	      break;
	  }
	  if (param==0){
	    confirm(lang==1?"Plus de variables libres.":"No more free variable available",lang==1?"Essayez purge(a) ou purge(b) ou ...":"Try purge(a) or purge(b) or ...");
	    continue;
	  }
	  string mycmd=inputparam(param.print()[0],0,contextptr);
	  if (!mycmd.empty()){
	    hp->line=hp->add_entry(-1);
	    // string mycmd=param.print()+":=element(0..1,0.5)";
	    // autoname_plus_plus();
	    hp->set_string_value(hp->line,mycmd);
	    hp->pos=mycmd.size()-1;
	  }
	  return KEY_CTRL_OK;	  
	}
	if (choix==11){ // Transforms
	  const char *
	    tab[]={
		   lang==1?"symetrie":"reflexion",
		   lang==1?"rotation":"rotation",
		   lang==1?"translation":"translation",
		   lang==1?"projection":"projection",
		   lang==1?"homothetie":"homothety",
		   lang==1?"similitude":"similarity",
		   // lang==1?"":"",
		   // lang==1?"":"",
		   0};
	  const int s=sizeof(tab)/sizeof(char *);
	  int choix=select_item(tab,"Transform",true);
	  if (choix<0 || choix>s)
	    continue;
	  gen ftmp[]={at_segment,at_polygone_ouvert,at_segment,at_segment,at_segment,at_polygone_ouvert};
	  gen ffinal[]={at_symetrie,at_rotation,at_translation,at_projection,at_homothetie,at_similitude};
	  int mode[]={2,3,2,2,2,3};
	  const char * help[]={
			       "Symmetry_center_axis,Object",
			       "Center,Angle,Object",
			       "Vector,Object",
			       "Curve,Object",
			       "Center,Ratio,Object",
			       "Center,Ratio,Angle,Object"
	  };
	  set_mode(ftmp[choix],ffinal[choix],mode[choix],help[choix]);
	  continue;
	}
	if (choix==12){ // Mesures
	  const char *
	    tab[]={
		   lang==1?"distance":"distance",
		   lang==1?"angle":"angle",
		   lang==1?"aire":"area",
		   lang==1?"perimetre":"perimeter",
		   lang==1?"pente":"slope",
		   lang==1?"distance seule":"distance raw",
		   lang==1?"angle seul":"angle raw",
		   lang==1?"aire seule":"area raw",
		   lang==1?"perimetre seul":"perimeter raw",
		   lang==1?"pente seule":"slope raw",
		   0};
	  const int s=sizeof(tab)/sizeof(char *);
	  int choix=select_item(tab,"Mesures",true);
	  if (choix<0 || choix>s)
	    continue;
	  gen ftmp[]={at_segment,at_triangle,at_areaat,at_perimeterat,at_slopeat,at_segment,at_triangle,at_areaatraw,at_perimeteratraw,at_slopeatraw};
	  gen ffinal[]={at_distanceat,at_angleat,at_areaat,at_perimeterat,at_slopeat,at_distanceatraw,at_angleatraw,at_areaatraw,at_perimeteratraw,at_slopeatraw};
	  int mode[]={3,4,2,2,2,3,4,2,2,2};
	  const char * help[]={
			       "Object1,Object2,Position",
			       "Angle_vertex,Direction1,Direction2,Position",
			       "Object,Position",
			       "Object,Position",
			       "Object,Position",
			       "Object1,Object2,Position",
			       "Angle_vertex,Direction1,Direction2,Position",
			       "Object,Position",
			       "Object,Position",
			       "Object,Position",
	  };
	  set_mode(ftmp[choix],ffinal[choix],mode[choix],help[choix]);
	  continue;
	}
	continue;
      }      

      if (key==KEY_CTRL_MENU || key==KEY_CTRL_F6 ||
	  (!hp && (key==KEY_CTRL_CATALOG || key==KEY_BOOK))){
	char menu_xmin[32],menu_xmax[32],menu_ymin[32],menu_ymax[32],menu_zmin[32],menu_zmax[32],menu_depth[32];
	Menu smallmenu;
	smallmenu.numitems=22;
	MenuItem smallmenuitems[smallmenu.numitems];
	smallmenu.items=smallmenuitems;
	smallmenu.height=MENUHEIGHT;
	for (;;){
	  string s;
	  s="xmin "+print_DOUBLE_(gr.window_xmin,contextptr);
	  strcpy(menu_xmin,s.c_str());
	  s="xmax "+print_DOUBLE_(gr.window_xmax,contextptr);
	  strcpy(menu_xmax,s.c_str());
	  s="ymin "+print_DOUBLE_(gr.window_ymin,contextptr);
	  strcpy(menu_ymin,s.c_str());
	  s="ymax "+print_DOUBLE_(gr.window_ymax,contextptr);
	  strcpy(menu_ymax,s.c_str());
	  s="zmin 3d "+print_DOUBLE_(gr.window_zmin,contextptr);
	  strcpy(menu_zmin,s.c_str());
	  s="zmax 3d "+print_DOUBLE_(gr.window_zmax,contextptr);
	  strcpy(menu_zmax,s.c_str());
	  s="depth 3d "+print_DOUBLE_(gr.current_depth,contextptr);
	  strcpy(menu_depth,s.c_str());
	  //smallmenu.title = "KhiCAS";
	  smallmenuitems[0].text = (char *) ((lang==1)?"Aide":"Help");
#ifdef NUMWORKS
	  smallmenuitems[1].text = (char*) ((lang==1)?"Etude courbe (x,n,t)":"Curve study (x,n,t)");
#else
	  smallmenuitems[1].text = (char*) ((lang==1)?"Etude courbe (tab)":"Curve study (tab)");
#endif
	  smallmenuitems[2].text = (char *) menu_xmin;
	  smallmenuitems[3].text = (char *) menu_xmax;
	  smallmenuitems[4].text = (char *) menu_ymin;
	  smallmenuitems[5].text = (char *) menu_ymax;
	  smallmenuitems[6].text = (char *) menu_zmin;
	  smallmenuitems[7].text = (char *) menu_zmax;
	  smallmenuitems[8].text = (char *) menu_depth;
	  smallmenuitems[9].text = (char*) (lang==1?"Sauvegarder figure":"Save figure");
	  smallmenuitems[10].text = (char*) (lang==1?"Sauvegarder comme":"Save as");
	  smallmenuitems[11].text = (char*)((lang==1)?"Quitter":"Quit");
	  smallmenuitems[12].text = (char*) "Orthonormalize /";
	  smallmenuitems[13].text = (char*) "Autoscale *";
	  smallmenuitems[14].text = (char *) ("Zoom in +");
	  smallmenuitems[15].text = (char *) ("Zoom out -");
	  smallmenuitems[16].text = (char *) ("Y-Zoom out (-)");
	  smallmenuitems[17].text = (char*) ((lang==1)?"Voir axes":"Show axes");
	  smallmenuitems[17].type = MENUITEM_CHECKBOX;
	  smallmenuitems[17].value = gr.show_axes;
	  smallmenuitems[18].text = (char*) ((lang==1)?"Voir tangente (F3)":"Show tangent (F3)");
	  smallmenuitems[18].type = MENUITEM_CHECKBOX;
	  smallmenuitems[18].value = (gr.tracemode & 2)!=0;
	  smallmenuitems[19].text = (char*) ((lang==1)?"Voir normale (F4)":"Show normal (F4)");
	  smallmenuitems[19].type = MENUITEM_CHECKBOX;
	  smallmenuitems[19].value = (gr.tracemode & 4)!=0;
	  smallmenuitems[20].text = (char*) ((lang==1)?"Voir cercle (F5)":"Show circle (F5)");
	  smallmenuitems[20].type = MENUITEM_CHECKBOX;
	  smallmenuitems[20].value = (gr.tracemode & 8)!=0;
	  smallmenuitems[21].text = (char*) ((lang==1)?"Effacer traces geometrie":"Clear geometry traces");
	  drawRectangle(0,180,LCD_WIDTH_PX,60,_BLACK);
	  int sres = doMenu(&smallmenu);
	  if (sres == MENU_RETURN_EXIT)
	    break;
	  if (sres == MENU_RETURN_SELECTION || sres==KEY_CTRL_EXE) {
	    const char * ptr=0;
	    string s1; double d;
	    if (smallmenu.selection==1){
	      geohelp(contextptr); continue;
	      // gr.q=quaternion_double(0,0,0); gr.update();
	    }
	    if (smallmenu.selection==2)
	      gr.curve_infos();
	    if (smallmenu.selection==3){
	      d=gr.window_xmin;
	      if (inputdouble(menu_xmin,d,200,contextptr)){
		gr.window_xmin=d;
		gr.update();
	      }
	    }
	    if (smallmenu.selection==4){
	      d=gr.window_xmax;
	      if (inputdouble(menu_xmax,d,200,contextptr)){
		gr.window_xmax=d;
		gr.update();
	      }
	    }
	    if (smallmenu.selection==5){
	      d=gr.window_ymin;
	      if (inputdouble(menu_ymin,d,200,contextptr)){
		gr.window_ymin=d;
		gr.update();
	      }
	    }
	    if (smallmenu.selection==6){
	      d=gr.window_ymax;
	      if (inputdouble(menu_ymax,d,200,contextptr)){
		gr.window_ymax=d;
		gr.update();
	      }
	    }
	    if (smallmenu.selection==7){
	      d=gr.window_zmin;
	      if (inputdouble(menu_zmin,d,200,contextptr)){
		gr.window_zmin=d;
		gr.update();
	      }
	    }
	    if (smallmenu.selection==8){
	      d=gr.window_zmax;
	      if (inputdouble(menu_zmax,d,200,contextptr)){
		gr.window_zmax=d;
		gr.update();
	      }
	    }
	    if (smallmenu.selection==9){
	      d=gr.current_depth;
	      if (inputdouble(menu_depth,d,200,contextptr)){
		if (d<-1) d=-1;
		if (d>1) d=1;
		gr.current_depth=d;
		gr.update();
	      }
	    }
	    if (hp && smallmenu.selection==10){
	      // save
	      geosave(hp,contextptr);
	      continue;
	    }
	    if (smallmenu.selection==10 || smallmenu.selection==11){
	      // save as
	      char filename[MAX_FILENAME_SIZE+1];
	      if (get_filename(filename,".py") && newgeo(contextptr)==0){
		geoptr->hp->filename=filename;
		if (geoptr!=this && !symbolic_instructions.empty()){
		  geoptr->symbolic_instructions=symbolic_instructions;
		  geoptr->hp->elements.clear();
		  for (int i=0;i<symbolic_instructions.size();++i){
		    geoptr->hp->set_string_value(i,symbolic_instructions[i].print(contextptr));
		  }
		  geoloop(geoptr);
		  return 0;
		}
	      }
	    }
	    if (smallmenu.selection==12)
	      return -4;
	    if (smallmenu.selection==13)
	      gr.orthonormalize();
	    if (smallmenu.selection==14)
	      gr.autoscale();	
	    if (smallmenu.selection==15)
	      gr.zoom(0.7);	
	    if (smallmenu.selection==16)
	      gr.zoom(1/0.7);	
	    if (smallmenu.selection==17)
	      gr.zoomy(1/0.7);
	    if (smallmenu.selection==18)
	      gr.show_axes=!gr.show_axes;	
	    if (smallmenu.selection==19){
	      if (gr.tracemode & 2)
		gr.tracemode &= ~2;
	      else
		gr.tracemode |= 2;
	      gr.tracemode_set();
	    }
	    if (smallmenu.selection==20){
	      if (gr.tracemode & 4)
		gr.tracemode &= ~4;
	      else {
		gr.tracemode |= 4;
		gr.orthonormalize();
	      }
	      gr.tracemode_set();
	    }
	    if (smallmenu.selection==21){
	      if (gr.tracemode & 8)
		gr.tracemode &= ~8;
	      else {
		gr.tracemode |= 8;
		gr.orthonormalize();
	      }
	      gr.tracemode_set();
	    }
	    if (smallmenu.selection==19){
	      gr.trace_instructions.clear();
	      update_g();
	    }
	  }
	}
	gr.draw();
	gr.must_redraw=false;
	continue;
      }

      if (hp && (key==KEY_CTRL_OK || key==KEY_CTRL_EXE)){
	if (mode==255)
	  return key;
	if (!moving){
	  pushed=true;
	  push_i=current_i;
	  push_j=current_j;
	  push_depth = current_depth;
	  geo_handle(FL_PUSH,KEY_CTRL_OK);
	  if (moving){
	    update_g();
	    continue;
	  }
	}
	int res=geo_handle(FL_RELEASE,KEY_CTRL_OK);
	pushed=false;
	update_g();
	continue;
      }
      if (hp && key==KEY_CTRL_EXIT && mode!=255){
	if (mode==0){ // restore original value and reeval
	  geo_handle(FL_RELEASE,KEY_CTRL_EXIT);
	  // do_handle(symbolic(at_sto,makevecteur(drag_original_value,drag_name)));
	  if (!pushed)
	    set_mode(0,0,255,"");
	}
	else
	  if (args_tmp.empty())
	    set_mode(0,0,255,"");
	pushed=false;
	moving=moving_frame=false;
	args_tmp.clear();
	update_g();
	continue;
      }
      if (key==KEY_CTRL_EXIT || key==KEY_CTRL_OK || key==KEY_CTRL_EXE){
	os_hide_graph();
	return key;
      }
      if (key==KEY_CHAR_NORMAL || key=='>'){ // shift-+
	if (gr.is3d && gr.precision<9)
	  gr.precision++;
      }
      if (key=='\\' || key=='<'){ // shift--
	if (gr.is3d && gr.precision>1)
	 gr.precision--;
      }
      if (key==KEY_CTRL_UP){
	if (tracemode && !alph){
	  --tracemode_n;
	  tracemode_set();
	  continue;
	}
	if (hp && mode!=255 && !alph){
	  --current_j;
	  if (current_j<0){
	    gr.up((gr.window_ymax-gr.window_ymin)/5);
	    current_j += LCD_HEIGHT_PX/5;
	  }
	  geo_handle(moving?FL_DRAG:FL_MOVE,key);
	  update_g();
	  continue;
	}
	if (gr.is3d && !alph){
	  int curprec=gr.precision;
	  gr.precision += 2;
	  if (gr.precision>9) gr.precision=9;
	  while (1){
	    //double X,Y,Z;
	    //do_transform(gr.invtransform,0.707,0.707,0,X,Y,Z);
	    //normalize(X,Y,Z);
	    //gr.q=rotation_2_quaternion_double(X,Y,Z,15)*gr.q;
	    //gr.q=quaternion_double(15,0,15)*gr.q;
	    gr.q=rotation_2_quaternion_double(0.707,0.707,0,15)*gr.q;// quaternion_double(15,0,0)*gr.q;
	    gr.update_rotation();
	    gr.draw();
	    gr.must_redraw=gr.solid3d;
#ifndef SIMU
	    if (!iskeydown(KEY_CTRL_UP))
	      break;
#else
	    getkey(key); break;
#endif
	  }
	  gr.precision=curprec;
	  continue;
	}
	gr.up((gr.window_ymax-gr.window_ymin)/16);
      }
      if (key==KEY_CTRL_PAGEUP) {
	if (tracemode && !alph){
	  tracemode_n-=2;
	  tracemode_set();
	  continue;
	}
	if (hp && mode!=255 && !alph){
	  current_j-=LCD_HEIGHT_PX/5;;
	  if (current_j<0){
	    gr.up((gr.window_ymax-gr.window_ymin)/2);
	    current_j += LCD_HEIGHT_PX/2;
	  }
	  geo_handle(moving?FL_DRAG:FL_MOVE,key);
	  update_g();
	  continue;
	}
	gr.up((gr.window_ymax-gr.window_ymin)/4);
      }
      if (key==KEY_CTRL_DOWN) {
	if (tracemode && !alph){
	  ++tracemode_n;
	  tracemode_set();
	  continue;
	}
	if (hp && mode!=255 && !alph){
	  ++current_j;
	  if (current_j>=LCD_HEIGHT_PX-24){
	    gr.down((gr.window_ymax-gr.window_ymin)/5);
	    current_j -= LCD_HEIGHT_PX/5;
	  }
	  geo_handle(moving?FL_DRAG:FL_MOVE,key);
	  update_g();
	  continue;
	}
	if (gr.is3d && !alph){
	  int curprec=gr.precision;
	  gr.precision += 2;
	  if (gr.precision>9) gr.precision=9;
	  while (1){
	    //double X,Y,Z;
	    //do_transform(gr.invtransform,0.707,0.707,0,X,Y,Z);
	    //normalize(X,Y,Z);
	    //gr.q=rotation_2_quaternion_double(X,Y,Z,-15)*gr.q;
	    // gr.q=quaternion_double(-15,0,-15)*gr.q;
	    gr.q=rotation_2_quaternion_double(0.707,0.707,0,-15)*gr.q; // quaternion_double(-15,0,0)*gr.q;
	    gr.update_rotation();
	    gr.draw();
	    gr.must_redraw=gr.solid3d;
#ifndef SIMU
	    if (!iskeydown(KEY_CTRL_DOWN))
	      break;
#else
	    getkey(key); break;
#endif
	  }
	  gr.precision=curprec;
	  continue;
	}
	gr.down((gr.window_ymax-gr.window_ymin)/16);
      }
      if (key==KEY_CTRL_PAGEDOWN) {
	if (tracemode && !alph){
	  tracemode_n+=2;
	  tracemode_set();
	  continue;
	}
	if (hp && mode!=255 && !alph){
	  current_j += LCD_HEIGHT_PX/5;
	  if (current_j>=LCD_HEIGHT_PX-24){
	    gr.down((gr.window_ymax-gr.window_ymin)/2);
	    current_j -= LCD_HEIGHT_PX/2;
	  }
	  geo_handle(moving?FL_DRAG:FL_MOVE,key);
	  update_g();
	  continue;
	}
	gr.down((gr.window_ymax-gr.window_ymin)/4);
      }
      if (key==KEY_CTRL_LEFT) {
	if (tracemode && !alph){
	  if (tracemode_i!=int(tracemode_i))
	    tracemode_i=std::floor(tracemode_i);
	  else
	    --tracemode_i;
	  tracemode_set();
	  continue;
	}
	if (hp && mode!=255 && !alph){
	  --current_i;
	  if (current_i<0){
	    gr.left((gr.window_xmax-gr.window_xmin)/5);
	    current_i += LCD_WIDTH_PX/5;
	  }
	  geo_handle(moving?FL_DRAG:FL_MOVE,key);
	  update_g();
	  continue;
	}
	if (gr.is3d && !alph){
	  int curprec=gr.precision;
	  gr.precision += 2;
	  if (gr.precision>9) gr.precision=9;
	  while (1){
	    gr.q=quaternion_double(0,15,0)*gr.q;
	    gr.update_rotation();
	    gr.draw();
	    gr.must_redraw=gr.solid3d;
#ifndef SIMU
	    if (!iskeydown(KEY_CTRL_LEFT))
	      break;
#else
	    getkey(key); break;
#endif
	  }
	  gr.precision=curprec;
	  continue;
	}
	gr.left((gr.window_xmax-gr.window_xmin)/16);
      }
      if (key==KEY_SHIFT_LEFT) {
	if (tracemode && !alph){
	  tracemode_i-=5;
	  tracemode_set();
	  continue;
	}
	if (hp && mode!=255 && !alph){
	  current_i -= LCD_WIDTH_PX/5;
	  if (current_i<0){
	    gr.left((gr.window_xmax-gr.window_xmin)/2);
	    current_i += LCD_WIDTH_PX/2;
	  }
	  geo_handle(moving?FL_DRAG:FL_MOVE,key);
	  update_g();
	  continue;
	}
	gr.left((gr.window_xmax-gr.window_xmin)/4);
      }
      if (key==KEY_CTRL_RIGHT) {
	if (tracemode && !alph){
	  if (int(tracemode_i)!=tracemode_i)
	    tracemode_i=std::ceil(tracemode_i);
	  else
	    ++tracemode_i;
	  tracemode_set();
	  continue;
	}
	if (hp && mode!=255 && !alph){
	  ++current_i;
	  if (current_i>=LCD_WIDTH_PX){
	    gr.right((gr.window_xmax-gr.window_xmin)/5);
	    current_i -= LCD_WIDTH_PX/5;
	  }
	  geo_handle(moving?FL_DRAG:FL_MOVE,key);
	  update_g();
	  continue;
	}
	if (gr.is3d && !alph){
	  int curprec=gr.precision;
	  gr.precision += 2;
	  if (gr.precision>9) gr.precision=9;
	  while (1){
	    gr.q=quaternion_double(0,-15,0)*gr.q;
	    gr.update_rotation();
	    gr.draw();
	    gr.must_redraw=gr.solid3d;
#ifndef SIMU
	    if (!iskeydown(KEY_CTRL_RIGHT))
	      break;
#else
	    getkey(key); break;
#endif
	  }
	  gr.precision=curprec;
	  continue;
	}
	gr.right((gr.window_xmax-gr.window_xmin)/16);
      }
      if (key==KEY_SHIFT_RIGHT) {
	if (tracemode && !alph){
	  tracemode_i+=5;
	  tracemode_set();
	  continue;
	}
	if (hp && mode!=255 && !alph){
	  current_i += LCD_WIDTH_PX/5;
	  if (current_i>=LCD_WIDTH_PX){
	    gr.right((gr.window_xmax-gr.window_xmin)/2);
	    current_i -= LCD_WIDTH_PX/2;
	  }
	  geo_handle(moving?FL_DRAG:FL_MOVE,key);
	  update_g();
	  continue;
	}
	gr.right((gr.window_xmax-gr.window_xmin)/4);
      }
      if (key==KEY_CHAR_PLUS) {
	gr.zoom(0.7);
      }
      if (key==KEY_CHAR_MINUS){
	gr.zoom(1/0.7);
      }
      if (key==KEY_CHAR_PMINUS){
	gr.zoomy(1/0.7);
      }
      if (key==KEY_CHAR_MULT){
	gr.autoscale();
      }
      if (key==KEY_CHAR_DIV) {
	gr.orthonormalize();
      }
      if (gr.is3d){
	if (key==KEY_CHAR_0){
	  gr.hide2nd=!gr.hide2nd;
	}
	if (key==KEY_CHAR_DP){
	  gr.interval=!gr.interval;
	}
	if (key==KEY_CHAR_ANS){
	  gr.show_edges=!gr.show_edges;
	}
	if (key==KEY_CHAR_4){
	  if (current_depth>-1)
	    current_depth-=0.1;
	}
	if (key==KEY_CHAR_6){
	  if (current_depth<1)
	    current_depth+=0.1;
	}
	if (key==KEY_CHAR_5){
	  gr.q=quaternion_double(0,0,0);
	  gr.update();
	}	  
	if (key==KEY_CHAR_8){
	  gr.z_up((gr.window_zmax-gr.window_zmin)/5);
	  gr.update_rotation();
	}
	if (key==KEY_CHAR_2){
	  gr.z_down((gr.window_zmax-gr.window_zmin)/5);
	  gr.update_rotation();
	}
	if (key==KEY_CHAR_1){
	  gr.right((gr.window_xmax-gr.window_xmin)/5);
	  gr.update_rotation();
	}
	if (key==KEY_CHAR_9){
	  gr.left((gr.window_xmax-gr.window_xmin)/5);
	  gr.update_rotation();
	}
	if (key==KEY_CHAR_3){
	  gr.up((gr.window_ymax-gr.window_ymin)/5);
	  gr.update_rotation();
	}
	if (key==KEY_CHAR_7){
	  gr.down((gr.window_ymax-gr.window_ymin)/5);
	  gr.update_rotation();
	}
	if (key==KEY_CHAR_POW || key==KEY_CHAR_EXPN)
	  gr.doprecise=true;
	if (key==KEY_CHAR_LPAR && gr.diffusionz<64)
	  gr.diffusionz++;
	if (key==KEY_CHAR_RPAR && gr.diffusionz>2)
	  gr.diffusionz--;
      }
      if (key==KEY_CHAR_SIN) {
	gr.show_axes=!gr.show_axes;
	gr.update();
	gr.draw();
	gr.must_redraw=false;
      }
      if (key==KEY_CTRL_VARS){
	select_var(contextptr);
	gr.update();
	gr.draw();
	gr.must_redraw=false;
      }
    }
    // aborttimer = Timer_Install(0, check_execution_abort, 100); if (aborttimer > 0) { Timer_Start(aborttimer); }
    return 0;
  }

  int displaylogo(){
#ifdef TURTLETAB
    xcas::Turtle t={tablogo,0,0,1,1,(short) turtle_speed};
#else
    xcas::Turtle t;
    t.turtleptr=&turtle_stack();
    t.turtlex=t.turtley=0;
    t.turtlezoom=1;
    t.maillage=1;
    t.speed=(short) turtle_speed;
#endif
#ifdef NSPIRE_NEWLIB
    DefineStatusMessage((char*)"+-: zoom, pad: move, esc: quit", 1, 0, 0);
#else
    DefineStatusMessage((char*)"+-: zoom, pad: move, EXIT: quit", 1, 0, 0);
#endif
    DisplayStatusArea();
    bool redraw=true;
    while (1){
      int save_ymin=clip_ymin;
      clip_ymin=24;
      if (redraw)
	t.draw();
      redraw=false;
      clip_ymin=save_ymin;
      int key;
      GetKey(&key);
      if (key==KEY_SHUTDOWN)
	return key;
      if (key==KEY_CTRL_EXIT || key==KEY_CTRL_OK || key==KEY_PRGM_ACON || key==KEY_CTRL_MENU || key==KEY_CTRL_EXE || key==KEY_CTRL_VARS || key==KEY_CHAR_ANS)
	break;
      if (key==KEY_CTRL_UP){ t.turtley += 10; redraw=true; }
      if (key==KEY_CTRL_PAGEUP) { t.turtley += 100; redraw=true;}
      if (key==KEY_CTRL_DOWN) { t.turtley -= 10; redraw=true;}
      if (key==KEY_CTRL_PAGEDOWN) { t.turtley -= 100;redraw=true;}
      if (key==KEY_CTRL_LEFT) { t.turtlex -= 10; redraw=true;}
      if (key==KEY_SHIFT_LEFT) { t.turtlex -= 100; redraw=true;}
      if (key==KEY_CTRL_RIGHT) { t.turtlex += 10; redraw=true;}
      if (key==KEY_SHIFT_RIGHT) { t.turtlex += 100;redraw=true;}
      if (key==KEY_CHAR_PLUS) { t.turtlezoom *= 2;redraw=true;}
      if (key==KEY_CHAR_MINUS){ t.turtlezoom /= 2; redraw=true; }
      if (key==KEY_CHAR_MULT){ if (t.speed) t.speed *=2; else t.speed=10; redraw=true; }
      if (key==KEY_CHAR_DIV){ t.speed /=2; redraw=true; }
      if (key=='='){ redraw=true; }
    }
    os_hide_graph();
    return 0;
  }

  bool ispnt(const gen & g){
    if (g.is_symb_of_sommet(giac::at_pnt))
      return true;
    if (g.type!=_VECT || g._VECTptr->empty())
      return false;
    return ispnt(g._VECTptr->back());
  }

  void translate_fkey(int & input_key){
    if (input_key==KEY_CTRL_MIXEDFRAC) input_key=KEY_CTRL_F10;
    if (input_key==KEY_CTRL_FRACCNVRT) input_key=KEY_CTRL_F7;
    if (input_key==KEY_CHAR_LIST) input_key=KEY_CTRL_F9;
    if (input_key==KEY_CHAR_MAT) input_key=KEY_CTRL_F8;
    if (input_key==KEY_CTRL_PRGM) input_key=KEY_CTRL_F12;
    if (input_key==KEY_CTRL_FD) input_key=KEY_CTRL_F11;
    if (input_key==KEY_CHAR_ANGLE) input_key=KEY_CTRL_F13;
    if (input_key==KEY_CHAR_FRAC) input_key=KEY_CTRL_F14;
  }

  giac::gen eqw(const giac::gen & ge,bool editable,GIAC_CONTEXT){
    if (ge.is_symb_of_sommet(at_equal) && ge._SYMBptr->feuille.type==_VECT && ge._SYMBptr->feuille._VECTptr->size()==2 && ge._SYMBptr->feuille._VECTptr->front().type==_INT_ && ge._SYMBptr->feuille._VECTptr->back().type==_INT_){
      global_show_axes=ge._SYMBptr->feuille._VECTptr->back().val;
      return ge;
    }
    if (ge.is_symb_of_sommet(at_erase)){
      global_show_axes=1;
      return ge;
    }
    bool edited=false;
    const int margin=16;
#ifdef CURSOR
    Cursor_SetFlashOff();
#endif
    giac::gen geq(_copy(ge,contextptr));
    // if (ge.type!=giac::_DOUBLE_ && giac::has_evalf(ge,geq,1,contextptr)) geq=giac::symb_equal(ge,geq);
    int line=-1,col=-1,nlines=0,ncols=0,listormat=0;
    xcas::Equation eq(0,0,geq,contextptr);
    giac::eqwdata eqdata=xcas::Equation_total_size(eq.data);
    if (eqdata.dx>1.5*LCD_WIDTH_PX || eqdata.dy>1.5*LCD_HEIGHT_PX){
      if (eqdata.dx>2.25*LCD_WIDTH_PX || eqdata.dy>2.25*LCD_HEIGHT_PX)
	eq.attr=giac::attributs(14,COLOR_WHITE,COLOR_BLACK);
      else
	eq.attr=giac::attributs(16,COLOR_WHITE,COLOR_BLACK);
      eq.data=0; // clear memory
      eq.data=xcas::Equation_compute_size(geq,eq.attr,LCD_WIDTH_PX,contextptr);
      eqdata=xcas::Equation_total_size(eq.data);
    }
    int dx=(eqdata.dx-LCD_WIDTH_PX)/2,dy=LCD_HEIGHT_PX-2*margin+eqdata.y;
    if (geq.type==_VECT){
      nlines=geq._VECTptr->size();
      if (eqdata.dx>=LCD_WIDTH_PX)
	dx=-20; // line=nlines/2;
      //else
      if (geq.subtype!=_SEQ__VECT){
	line=0;
	listormat=1;
	if (ckmatrix(geq)){
	  ncols=geq._VECTptr->front()._VECTptr->size();
	  if (eqdata.dy>=LCD_HEIGHT_PX-margin)
	    dy=eqdata.y+eqdata.dy+32;// col=ncols/2;
	  // else
	  col=0;
	  listormat=2;
	}
      }
    }
    if (!listormat){
      xcas::Equation_select(eq.data,true);
      xcas::eqw_select_down(eq.data);
    }
    //cout << eq.data << endl;
    int firstrun=2;
    for (;;){
#if 1
      if (firstrun==2){
#ifdef NSPIRE_NEWLIB
	DefineStatusMessage((char*)((lang==1)?"ctrl enter: eval, esc: quitte, ":"ctrl enter: eval, esc: exit"), 1, 0, 0);
#else
	DefineStatusMessage((char*)((lang==1)?"EXE: quitte, resultat dans last":"EXE: quit, result stored in last"), 1, 0, 0);
#endif
	DisplayStatusArea();
	firstrun=1;
      }
      else
	set_xcas_status();
#else
      DefineStatusMessage((char*)"+-: zoom, pad: move, EXIT: quit", 1, 0, 0);
      DisplayStatusArea();
#endif
      gen value;
      if (listormat) // select line l, col c
	xcas::eqw_select(eq.data,line,col,true,value);
      if (eqdata.dx>LCD_WIDTH_PX){
	if (dx<-20)
	  dx=-20;
	if (dx>eqdata.dx-LCD_WIDTH_PX+20)
	  dx=eqdata.dx-LCD_WIDTH_PX+20;
      }
#define EQW_TAILLE 18
      if (eqdata.dy>LCD_HEIGHT_PX-2*EQW_TAILLE){
	if (dy-eqdata.y<LCD_HEIGHT_PX-2*EQW_TAILLE)
	  dy=eqdata.y+LCD_HEIGHT_PX-2*EQW_TAILLE;
	if (dy-eqdata.y>eqdata.dy+32)
	  dy=eqdata.y+eqdata.dy+32;
      }
      waitforvblank();
      drawRectangle(0, 0, LCD_WIDTH_PX, 205,COLOR_WHITE);
      // Bdisp_AllClr_VRAM();
      int save_clip_ymin=clip_ymin;
      clip_ymin=STATUS_AREA_PX;
      xcas::display(eq,dx,dy,contextptr);
      string menu;
#ifndef HP39
      menu +="shift-1 ";
#endif
      menu += string(menu_f1);
      menu += "| ";
#ifndef HP39
      menu += "2 ";
#endif
      menu += string(menu_f2);
#ifdef HP39
      menu += "| undo| edit| +- | approx";
      drawRectangle(0,114,LCD_WIDTH_PX,14,giac::_BLACK);
      PrintMini(0,114,menu.c_str(),4);
#else
      menu += "|3 undo|4 edt|5 +-|6 approx";
      drawRectangle(0,205,LCD_WIDTH_PX,17,22222);
      PrintMiniMini(0,205,menu.c_str(),0,giac::_BLACK,22222);
#endif
      //draw_menu(2);
      clip_ymin=save_clip_ymin;
      int keyflag = GetSetupSetting( (unsigned int)0x14);
      if (firstrun){ // workaround for e.g. 1+x/2 partly not displayed
	firstrun=0;
	continue;
      }
      int key;
      //cout << eq.data << endl;
      GetKey(&key);
      if (key==KEY_SHUTDOWN)
	return undef;
      bool alph=alphawasactive(&key);
      if (key==KEY_CTRL_OK || key==KEY_CTRL_EXE || key==KEY_CTRL_MENU){
	os_hide_graph();
	if (edited && xcas::do_select(eq.data,true,value) && value.type==_EQW){
	  //cout << "ok " << value._EQWptr->g << endl;
	  DefineStatusMessage(((lang==1)?"resultat stocke dans last":"result stored in last"), 1, 0, 0);
	  //DisplayStatusArea();
	  giac::sto(value._EQWptr->g,giac::gen("last",contextptr),contextptr);
	  return value._EQWptr->g;
	}
	//cout << "no " << eq.data << endl; if (value.type==_EQW) cout << value._EQWptr->g << endl ;
	return geq;
      }
      if (key==KEY_CTRL_EXIT || key==KEY_CTRL_AC ){
	if (!edited){
	  os_hide_graph();
	  return geq;
	}
	if (confirm(
#ifdef NSPIRE_NEWLIB
		    (lang==1)?"Vraiment abandonner?":"Really leave",(lang==1)?"esc: editeur,  enter: confirmer":"esc: editor,  enter: confirm"
#else
		    (lang==1)?"Vraiment abandonner?":"Really leave",(lang==1)?"Back: editeur,  OK: confirmer":"Back: editor,  OK: confirm"
#endif
		    )==KEY_CTRL_F1){
	  os_hide_graph();
	  return geq;
	}
      }
      if (key==KEY_CTRL_F3)
	key=KEY_CTRL_UNDO;
      if (key==KEY_CTRL_UNDO){
	giac::swapgen(eq.undodata,eq.data);
	if (listormat){
	  xcas::do_select(eq.data,true,value);
	  if (value.type==_EQW){
	    gen g=eval(value._EQWptr->g,1,contextptr);
	    if (g.type==_VECT){
	      const vecteur & v=*g._VECTptr;
	      nlines=v.size();
	      if (line >= nlines)
		line=nlines-1;
	      if (col!=-1 &&v.front().type==_VECT){
		ncols=v.front()._VECTptr->size();
		if (col>=ncols)
		  col=ncols-1;
	      }
	      xcas::do_select(eq.data,false,value);
	      xcas::eqw_select(eq.data,line,col,true,value);
	    }
	  }
	}
	continue;
      }
      int redo=0; 
      if (listormat){
	if (key==KEY_CHAR_COMMA || key==KEY_CTRL_DEL ){
	  xcas::do_select(eq.data,true,value);
	  if (value.type==_EQW){
	    gen g=eval(value._EQWptr->g,1,contextptr);
	    if (g.type==_VECT){
	      edited=true; eq.undodata=Equation_copy(eq.data);
	      vecteur v=*g._VECTptr;
	      if (key==KEY_CHAR_COMMA){
		if (col==-1 || (line>0 && line==nlines-1)){
		  v.insert(v.begin()+line+1,0*v.front());
		  ++line; ++nlines;
		}
		else {
		  v=mtran(v);
		  v.insert(v.begin()+col+1,0*v.front());
		  v=mtran(v);
		  ++col; ++ncols;
		}
	      }
	      else {
		if (col==-1 || (nlines>=3 && line==nlines-1)){
		  if (nlines>=(col==-1?2:3)){
		    v.erase(v.begin()+line,v.begin()+line+1);
		    if (line) --line;
		    --nlines;
		  }
		}
		else {
		  if (ncols>=2){
		    v=mtran(v);
		    v.erase(v.begin()+col,v.begin()+col+1);
		    v=mtran(v);
		    if (col) --col; --ncols;
		  }
		}
	      }
	      geq=gen(v,g.subtype);
	      key=0; redo=1;
	      // continue;
	    }
	  }
	}
      }
      bool ins=key==KEY_CHAR_STORE  || key==KEY_CHAR_RPAR || key==KEY_CHAR_LPAR || key==KEY_CHAR_COMMA || key==KEY_CTRL_PASTE || key==KEY_CTRL_F4;
      int xleft,ytop,xright,ybottom,gselpos; gen * gsel=0,*gselparent=0;
      if (key==KEY_CTRL_CLIP){
	xcas::Equation_adjust_xy(eq.data,xleft,ytop,xright,ybottom,gsel,gselparent,gselpos,0);
	if (gsel==0)
	  gsel==&eq.data;
	// cout << "var " << g << " " << eq.data << endl;
	if (xcas::do_select(*gsel,true,value) && value.type==_EQW){
	  //cout << g << ":=" << value._EQWptr->g << endl;
	  copy_clipboard(value._EQWptr->g.print(contextptr),true);
	  continue;
	}
      }
      if (key==KEY_CHAR_STORE){
	int keyflag = GetSetupSetting( (unsigned int)0x14);
	if (keyflag==0)
	  handle_f5();
	std::string varname;
	if (inputline(((lang==1)?"Stocker la selection dans":"Save selection in",(lang==1)?"Nom de variable: ":"Variable name: "),0,varname,false,65,contextptr) && !varname.empty() && isalpha(varname[0])){
	  giac::gen g(varname,contextptr);
	  giac::gen ge(protecteval(g,1,contextptr));
	  if (g.type!=_IDNT){
	    invalid_varname();
	    continue;
	  }
	  if (ge==g || confirm_overwrite()){
	    vector<int> goto_sel;
	    xcas::Equation_adjust_xy(eq.data,xleft,ytop,xright,ybottom,gsel,gselparent,gselpos,&goto_sel);
	    if (gsel==0)
	      gsel==&eq.data;
	    // cout << "var " << g << " " << eq.data << endl;
	    if (xcas::do_select(*gsel,true,value) && value.type==_EQW){
	      //cout << g << ":=" << value._EQWptr->g << endl;
	      giac::gen gg(value._EQWptr->g);
	      if (gg.is_symb_of_sommet(at_makevector))
		gg=giac::eval(gg,1,contextptr);
	      giac::sto(gg,g,contextptr);
	    }
	  }
	}
	continue;
      }
      if (key==KEY_CTRL_DEL){
	vector<int> goto_sel;
	if (xcas::Equation_adjust_xy(eq.data,xleft,ytop,xright,ybottom,gsel,gselparent,gselpos,&goto_sel) && gsel && xcas::do_select(*gsel,true,value) && value.type==_EQW){
	  value=value._EQWptr->g;
	  if (value.type==_SYMB){
	    gen tmp=value._SYMBptr->feuille;
	    if (tmp.type!=_VECT || tmp.subtype!=_SEQ__VECT){
	      xcas::replace_selection(eq,tmp,gsel,&goto_sel,contextptr);
	      continue;
	    }
	  }
	  if (!goto_sel.empty() && gselparent && gselparent->type==_VECT && !gselparent->_VECTptr->empty()){
	    vecteur & v=*gselparent->_VECTptr;
	    if (v.back().type==_EQW){
	      gen opg=v.back()._EQWptr->g;
	      if (opg.type==_FUNC){
		int i=0;
		for (;i<v.size()-1;++i){
		  if (&v[i]==gsel)
		    break;
		}
		if (i<v.size()-1){
		  if (v.size()==5 && (opg==at_integrate || opg==at_sum) && i>=2)
		    v.erase(v.begin()+2,v.begin()+4);
		  else
		    v.erase(v.begin()+i);
		  xcas::do_select(*gselparent,true,value);
		  if (value.type==_EQW){
		    value=value._EQWptr->g;
		    // cout << goto_sel << " " << value << endl; continue;
		    if (v.size()==2 && (opg==at_plus || opg==at_prod || opg==at_pow))
		      value=protecteval(value,1,contextptr);
		    goto_sel.erase(goto_sel.begin());
		    xcas::replace_selection(eq,value,gselparent,&goto_sel,contextptr);
		    continue;
		  }
		}
	      }
	    }
	  }
	}
      }
      if (key=='\\' || key==KEY_CTRL_F5){
	xcas::do_select(eq.data,true,value);
	if (value.type==_EQW)
	  geq=value._EQWptr->g;
	if (eq.attr.fontsize<=14)
	  eq.attr.fontsize=18;
	else
	  eq.attr.fontsize=14;
	redo=1;
      }
      if (key==KEY_CHAR_IMGNRY)
	key='i';
      const char keybuf[2]={(key==KEY_CHAR_PMINUS?'-':char(key)),0};
      const char * adds=(key==KEY_CTRL_F4 || key==KEY_CHAR_PMINUS ||
			 (key==char(key) && (isalphanum(key)|| key=='.' ))
			 )?keybuf:keytostring(key,keyflag,contextptr);
      if (adds && !strcmp(adds,"VARS()"))
	continue;
      translate_fkey(key);
      if ( key==KEY_CTRL_F1 || key==KEY_CTRL_F2 ||
	   (key>=KEY_CTRL_F7 && key<=KEY_CTRL_F14)){
	adds=console_menu(key,fmenu_cfg,1);//alph?"simplify":(keyflag==1?"factor":"partfrac");
	// workaround for infinitiy
	if (!adds) continue;
	if (strlen(adds)>=2 && adds[0]=='o' && adds[1]=='o')
	  key=KEY_CTRL_F3;      
      }
      if (key==KEY_CTRL_F6 || key==KEY_CTRL_EXE){
	adds= (key==KEY_CTRL_F6?"evalf":"eval");
      }
      if (key==KEY_CHAR_MINUS)
	adds="-";
      if (key==KEY_CHAR_EQUAL)
	adds="=";
      if (key==KEY_CHAR_RECIP)
	adds="inv";
      if (key==KEY_CHAR_SQUARE)
	adds="sq";
      if (key==KEY_CHAR_POWROOT)
	adds="surd";
      if (key==KEY_CHAR_CUBEROOT)
	adds="surd";
      if (key==KEY_CHAR_FACTOR)
	adds="factor";
      if (key==KEY_CHAR_NORMAL)
	adds="normal";
      int addssize=adds?strlen(adds):0;
      // cout << addssize << " " << adds << endl;
      if (0 && key==KEY_CTRL_EXE){
	if (xcas::do_select(eq.data,true,value) && value.type==_EQW){
	  //cout << "ok " << value._EQWptr->g << endl;
	  DefineStatusMessage(((lang==1)?"resultat stocke dans last":"result stored in last"), 1, 0, 0);
	  //DisplayStatusArea();
	  giac::sto(value._EQWptr->g,giac::gen("last",contextptr),contextptr);
	  return value._EQWptr->g;
	}
	//cout << "no " << eq.data << endl; if (value.type==_EQW) cout << value._EQWptr->g << endl ;
	return geq;
      }
      if ( key!=KEY_CHAR_MINUS && key!=KEY_CHAR_EQUAL && key!=0 &&
	   (ins || key==KEY_CHAR_PI || key==KEY_CTRL_VARS || key==KEY_CTRL_F3 || (addssize==1 && (isalphanum(adds[0])|| adds[0]=='.' || adds[0]=='-') ) )
	   ){
	edited=true;
	if (line>=0 && xcas::eqw_select(eq.data,line,col,true,value)){
	  string s;
	  if (ins){
	    if (key==KEY_CTRL_PASTE)
	      s=paste_clipboard();
	    else {
	      if (value.type==_EQW){
		s=value._EQWptr->g.print(contextptr);
	      }
	      else
		s=value.print(contextptr);
	    }
	  }
	  else
	    s = adds;
	  string msg("Line ");
	  msg += print_INT_(line+1);
	  msg += " Col ";
	  msg += print_INT_(col+1);
	  if (inputline(msg.c_str(),0,s,false,65,contextptr)==KEY_CTRL_EXE){
	    value=gen(s,contextptr);
	    if (col<0)
	      (*geq._VECTptr)[line]=value;
	    else
	      (*((*geq._VECTptr)[line]._VECTptr))[col]=value;
	    redo=2;
	    key=KEY_SHIFT_RIGHT;
	  }
	  else
	    continue;
	}
	else {
	  vector<int> goto_sel;
	  if (xcas::Equation_adjust_xy(eq.data,xleft,ytop,xright,ybottom,gsel,gselparent,gselpos,&goto_sel) && gsel && xcas::do_select(*gsel,true,value) && value.type==_EQW){
	    string s;
	    if (ins){
	      if (key==KEY_CTRL_PASTE)
		s=paste_clipboard();
	      else {
		s = value._EQWptr->g.print(contextptr);
		if (key==KEY_CHAR_COMMA)
		  s += ',';
	      }
	    }
	    else
	      s = adds;
	    if (inputline(value._EQWptr->g.print(contextptr).c_str(),0,s,false)==KEY_CTRL_EXE){
	      value=gen(s,contextptr);
	      //cout << value << " goto " << goto_sel << endl;
	      xcas::replace_selection(eq,value,gsel,&goto_sel,contextptr);
	      firstrun=-1; // workaround, force 2 times display
	    }
	    continue;
	  }
	}
      }
      if (redo){
	eq.data=0; // clear memory
	eq.data=xcas::Equation_compute_size(geq,eq.attr,LCD_WIDTH_PX,contextptr);
	eqdata=xcas::Equation_total_size(eq.data);
	if (redo==1){
	  dx=(eqdata.dx-LCD_WIDTH_PX)/2;
	  dy=LCD_HEIGHT_PX-2*margin+eqdata.y;
	  if (listormat) // select line l, col c
	    xcas::eqw_select(eq.data,line,col,true,value);
	  else {
	    xcas::Equation_select(eq.data,true);
	    xcas::eqw_select_down(eq.data);
	  }
	  continue;
	}
      }
      bool doit=eqdata.dx>=LCD_WIDTH_PX;
      int delta=0;
      if (listormat){
	if (key==KEY_CTRL_LEFT  || (!doit && key==KEY_SHIFT_LEFT)){
	  if (line>=0 && xcas::eqw_select(eq.data,line,col,false,value)){
	    if (col>=0){
	      --col;
	      if (col<0){
		col=ncols-1;
		if (line>0){
		  --line;
		  dy += value._EQWptr->dy+eq.attr.fontsize/2;
		}
	      }
	    }
	    else {
	      if (line>0)
		--line;
	    }
	    xcas::eqw_select(eq.data,line,col,true,value);
	    if (doit) dx -= value._EQWptr->dx;
	  }
	  continue;
	}
	if (doit && key==KEY_SHIFT_LEFT){
	  dx -= 20;
	  continue;
	}
	if (key==KEY_CTRL_RIGHT  || (!doit && key==KEY_SHIFT_RIGHT)) {
	  if (line>=0 && xcas::eqw_select(eq.data,line,col,false,value)){
	    if (doit)
	      dx += value._EQWptr->dx;
	    if (col>=0){
	      ++col;
	      if (col==ncols){
		col=0;
		if (line<nlines-1){
		  ++line;
		  dy -= value._EQWptr->dy+eq.attr.fontsize/2;
		}		  
	      }
	    } else {
	      if (line<nlines-1)
		++line;
	    }
	    xcas::eqw_select(eq.data,line,col,true,value);
	  }
	  continue;
	}
	if (key==KEY_SHIFT_RIGHT && doit){
	  dx += 20;
	  continue;
	}
	doit=eqdata.dy>=LCD_HEIGHT_PX-2*margin;
	if (key==KEY_CTRL_UP || (!doit && key==KEY_CTRL_PAGEUP)){
	  if (line>0 && col>=0 && xcas::eqw_select(eq.data,line,col,false,value)){
	    --line;
	    xcas::eqw_select(eq.data,line,col,true,value);
	    if (doit)
	      dy += value._EQWptr->dy+eq.attr.fontsize/2;
	  }
	  continue;
	}
	if (key==KEY_CTRL_PAGEUP && doit){
	  dy += 10;
	  continue;
	}
	if (key==KEY_CTRL_DOWN  || (!doit && key==KEY_CTRL_PAGEDOWN)){
	  if (line<nlines-1 && col>=0 && xcas::eqw_select(eq.data,line,col,false,value)){
	    if (doit)
	      dy -= value._EQWptr->dy+eq.attr.fontsize/2;
	    ++line;
	    xcas::eqw_select(eq.data,line,col,true,value);
	  }
	  continue;
	}
	if ( key==KEY_CTRL_PAGEDOWN && doit){
	  dy -= 10;
	  continue;
	}
      }
      else { // else listormat
	if (key==KEY_CTRL_LEFT){
	  delta=xcas::eqw_select_leftright(eq,true,alph?2:0,contextptr);
	  // cout << "left " << delta << endl;
	  if (doit) dx += (delta?delta:-20);
	  continue;
	}
	if (key==KEY_SHIFT_LEFT){
	  delta=xcas::eqw_select_leftright(eq,true,1,contextptr);
	  vector<int> goto_sel;
	  if (doit) dx += (delta?delta:-20);
	  continue;
	}
	if (key==KEY_CTRL_RIGHT){
	  delta=xcas::eqw_select_leftright(eq,false,alph?2:0,contextptr);
	  // cout << "right " << delta << endl;
	  if (doit)
	    dx += (delta?delta:20);
	  continue;
	}
	if (key==KEY_SHIFT_RIGHT){
	  delta=xcas::eqw_select_leftright(eq,false,1,contextptr);
	  // cout << "right " << delta << endl;
	  if (doit)
	    dx += (delta?delta:20);
	  // dx=eqdata.dx-LCD_WIDTH_PX+20;
	  continue;
	}
	doit=eqdata.dy>=LCD_HEIGHT_PX-2*margin;
	if (key==KEY_CTRL_UP){
	  delta=xcas::eqw_select_up(eq.data);
	  // cout << "up " << delta << endl;
	  continue;
	}
	//cout << "up " << eq.data << endl;
	if (key==KEY_CTRL_PAGEUP && doit){
	  dy=eqdata.y+eqdata.dy+20;
	  continue;
	}
	if (key==KEY_CTRL_DOWN){
	  delta=xcas::eqw_select_down(eq.data);
	  // cout << "down " << delta << endl;
	  continue;
	}
	//cout << "down " << eq.data << endl;
	if ( key==KEY_CTRL_PAGEDOWN && doit){
	  dy=eqdata.y+LCD_HEIGHT_PX-margin;
	  continue;
	}
      }
      if (adds){
	edited=true;
	if (strcmp(adds,"'")==0)
	  adds="diff";
	if (strcmp(adds,"^2")==0)
	  adds="sq";
	if (strcmp(adds,">")==0)
	  adds="simplify";
	if (strcmp(adds,"<")==0)
	  adds="factor";
	if (strcmp(adds,"#")==0)
	  adds="partfrac";
	string cmd(adds);
	if (cmd.size() && cmd[cmd.size()-1]=='(')
	  cmd ='\''+cmd.substr(0,cmd.size()-1)+'\'';
	vector<int> goto_sel;
	if (xcas::Equation_adjust_xy(eq.data,xleft,ytop,xright,ybottom,gsel,gselparent,gselpos,&goto_sel) && gsel){
	  gen op;
	  int addarg=0;
	  if (addssize==1){
	    switch (adds[0]){
	    case '+':
	      addarg=1;
	      op=at_plus;
	      break;
	    case '^':
	      addarg=1;
	      op=at_pow;
	      break;
	    case '=':
	      addarg=1;
	      op=at_equal;
	      break;
	    case '-':
	      addarg=1;
	      op=at_binary_minus;
	      break;
	    case '*':
	      addarg=1;
	      op=at_prod;
	      break;
	    case '/':
	      addarg=1;
	      op=at_division;
	      break;
	    case '\'':
	      addarg=1;
	      op=at_derive;
	      break;
	    }
	  }
	  if (op==0)
	    op=gen(cmd,contextptr);
	  if (op.type==_SYMB)
	    op=op._SYMBptr->sommet;
	  // cout << "keyed " << adds << " " << op << " " << op.type << endl;
	  if (op.type==_FUNC){
	    edited=true;
	    // execute command on selection
	    gen tmp,value;
	    if (xcas::do_select(*gsel,true,value) && value.type==_EQW){
	      if (op==at_integrate || op==at_sum)
		addarg=3;
	      if (op==at_limit)
		addarg=2;
	      gen args=protecteval(value._EQWptr->g,1,contextptr);
	      gen vx=xthetat?t__IDNT_e:x__IDNT_e;
	      if (addarg==1)
		args=makesequence(args,0);
	      if (addarg==2)
		args=makesequence(args,vx_var,0);
	      if (addarg==3)
		args=makesequence(args,vx_var,0,1);
	      if (op==at_surd)
		args=makesequence(args,key==KEY_CHAR_CUBEROOT?3:4);
	      if (op==at_subst)
		args=makesequence(args,giac::symb_equal(vx_var,0));
	      unary_function_ptr immediate_op[]={*at_eval,*at_evalf,*at_evalc,*at_regrouper,*at_simplify,*at_normal,*at_ratnormal,*at_factor,*at_cfactor,*at_partfrac,*at_cpartfrac,*at_expand,*at_canonical_form,*at_exp2trig,*at_trig2exp,*at_sincos,*at_lin,*at_tlin,*at_tcollect,*at_texpand,*at_trigexpand,*at_trigcos,*at_trigsin,*at_trigtan,*at_halftan};
	      if (equalposcomp(immediate_op,*op._FUNCptr)){
		set_abort();
		tmp=(*op._FUNCptr)(args,contextptr);
		clear_abort();
		esc_flag=0;
		giac::ctrl_c=false;
		kbd_interrupted=giac::interrupted=false;
	      }
	      else
		tmp=symbolic(*op._FUNCptr,args);
	      //cout << "sel " << value._EQWptr->g << " " << tmp << " " << goto_sel << endl;
	      esc_flag=0;
	      giac::ctrl_c=false;
	      kbd_interrupted=giac::interrupted=false;
	      if (!is_undef(tmp)){
		xcas::replace_selection(eq,tmp,gsel,&goto_sel,contextptr);
		if (addarg){
		  xcas::eqw_select_down(eq.data);
		  xcas::eqw_select_leftright(eq,false,0,contextptr);
		}
		eqdata=xcas::Equation_total_size(eq.data);
		dx=(eqdata.dx-LCD_WIDTH_PX)/2;
		dy=LCD_HEIGHT_PX-2*margin+eqdata.y;
		firstrun=-1; // workaround, force 2 times display
	      }
	    }
	  } // if (op.type==_FUNC)
	  
	} // if adjust_xy
      } // if (adds)
    }
    //*logptr(contextptr) << eq.data << endl;
  }
  
  void clear_turtle_history(GIAC_CONTEXT){
    history_in(contextptr).clear();
    history_out(contextptr).clear();
    turtle_stack()=vector<logo_turtle>(1,logo_turtle());
  }    
  
  void do_restart(GIAC_CONTEXT){
    if (contextptr){
      if (contextptr->globalcontextptr && contextptr->globalcontextptr->tabptr)
	contextptr->globalcontextptr->tabptr->clear();
    }
    else
      _restart(0,contextptr);
  }

  void switch_to_micropy(GIAC_CONTEXT){
    xcas_python_eval=1;
    int p=python_compat(contextptr);
    if (p<0) p=0;
    python_compat(4|p,contextptr);
    if (edptr)
      edptr->python=1;
    if (do_confirm((lang==1)?"Effacer les variables Xcas?":"Clear Xcas variables?"))
      do_restart(contextptr);
    *logptr(contextptr) << "Micropython interpreter\n";
    Console_FMenu_Init(contextptr);
  }

  void switch_to_js(GIAC_CONTEXT){
    //js_ck_eval("1",&global_js_context);
    xcas_python_eval=-1;
    python_compat(-1,contextptr);
    if (edptr)
      edptr->python=-1;
    if (0 && do_confirm((lang==1)?"Effacer les variables Xcas?":"Clear Xcas variables?"))
      do_restart(contextptr);
    *logptr(contextptr) << "QuickJS interpreter\n";
    Console_FMenu_Init(contextptr);
  }

  void do_run(const char * s,gen & g,gen & ge,const context * & contextptr){
    warn_nr=os_shell=true;
    if (!contextptr)
      contextptr=new giac::context;
    if (!strcmp(s,"restart")){
      clear_turtle_history(contextptr);
      do_restart(contextptr);
      return;
    }
    int S=strlen(s);
    char buf[S+1];
    buf[S]=0;
    for (int i=0;i<S;++i){
      char c=s[i];
      if (c==0x1e || c==char(0x9c))
	buf[i]='\n';
      else {
	if (c==0x0d)
	  buf[i]=' ';
	else
	  buf[i]=c;
      }
    }
    g=gen(buf,contextptr);
    //Console_Output(g.print(contextptr).c_str()); return ;
    giac::freeze=false;
    // execution_in_progress = 1;
    set_abort();
    ge=protecteval(equaltosto(g,contextptr),1,contextptr);
    clear_abort();
    // execution_in_progress = 0;
    if (esc_flag || ctrl_c){
      esc_flag=ctrl_c=interrupted=false;
      while (confirm("Interrupted","OK",true)==-1)
	; // insure ON has been removed from keyboard buffer
      ge=string2gen("Interrupted",false);
      // memory full?
      if (!kbd_interrupted){
	// clear turtle, display msg
	clear_turtle_history(contextptr);
	int res=confirm((lang==1)?"Memoire remplie! Purger":"Memory full. Purge",
#ifdef NSPIRE_NEWLIB
			(lang==1)?"enter: variable, esc: tout.":"enter: variables, esc: all",
#else
			(lang==1)?"EXE variable, Back: tout.":"EXE variables, Back: all",
#endif
			false);
	if (res==KEY_CTRL_F1 && select_var(contextptr).type==_IDNT){
	  size_t savestackptr = stackptr;
#ifdef x86_64
	  stackptr=0xffffffffffffffff;
#else
	  stackptr=0xffffffff;
#endif
	  _purge(g,contextptr);
	  stackptr=savestackptr;
	}
	else
	  do_restart(contextptr);
      }
    }
    //Console_Output("Done"); return ;
    esc_flag=0;
    giac::ctrl_c=false;
    giac::kbd_interrupted=giac::interrupted=false;
  }

#ifdef NSPIRE_NEWLIB
  const unsigned char rsa_n_tab[]=
  {
   0xf2,0x0e,0xd4,0x9d,0x44,0x04,0xc4,0xc8,0x6a,0x5b,0xc6,0x9a,0xd6,0xdf,
   0x9c,0xf5,0x56,0xf2,0x0d,0xad,0x6c,0x34,0xb4,0x48,0xf7,0xa7,0xa8,0x27,0xa0,
   0xc8,0xbe,0x36,0xb1,0xc0,0x95,0xf8,0xc2,0x72,0xfb,0x78,0x0f,0x3f,0x15,0x22,
   0xaf,0x51,0x96,0xe3,0xdc,0x39,0xb4,0xc6,0x40,0x6d,0x58,0x56,0x1f,0xad,0x55,
   0x55,0x08,0xf1,0xde,0x5a,0xbc,0xd3,0xcc,0x16,0x3d,0x33,0xee,0x83,0x3f,0x32,
   0xa7,0xa7,0xb8,0x95,0x2f,0x35,0xeb,0xf6,0x32,0x4d,0x22,0xd9,0x60,0xb7,0x5e,
   0xbd,0xea,0xa5,0xcb,0x9c,0x69,0xeb,0xfd,0x9f,0x2b,0x5f,0x3d,0x38,0x5a,0xe1,
   0x2b,0x63,0xf8,0x92,0x35,0x91,0xea,0x77,0x07,0xcc,0x4b,0x7a,0xbc,0xe0,0xa0,
   0x8b,0x82,0x98,0xa2,0x87,0x10,0x2c,0xe2,0x23,0x53,0x2f,0x70,0x03,0xec,0x2d,
   0x22,0x34,0x72,0x57,0x4d,0x24,0x2e,0x97,0xc9,0xfb,0x23,0xb0,0x05,0xff,0x87,
   0x6e,0xbf,0x94,0x2d,0xf0,0x36,0xed,0xd7,0x9a,0xac,0x0c,0x21,0x94,0xa2,0x75,
   0xfc,0x39,0x9b,0xba,0xf2,0xc6,0xc9,0x34,0xa0,0xb2,0x66,0x5a,0xcc,0xc9,0x5c,
   0xc7,0xdb,0xce,0xfb,0x3a,0x10,0xee,0xc1,0x82,0x9a,0x43,0xef,0xed,0x87,0xbd,
   0x6c,0xe4,0xc1,0x36,0xd0,0x0a,0x85,0x6e,0xca,0xcd,0x13,0x29,0x65,0xb5,0xd4,
   0x13,0x4a,0x14,0xaa,0x65,0xac,0x0e,0x6f,0x19,0xb0,0x62,0x47,0x65,0x0e,0x40,
   0x82,0x37,0xd6,0xf0,0x17,0x48,0xaa,0x8c,0x7b,0xc4,0x5e,0x4a,0x72,0x26,0xa6,
   0x08,0x2e,0xff,0x2d,0x9d,0x0e,0x2e,0x19,0xe9,0x6a,0x4c,0x7c,0x3e,0xe9,0xbc,
   0x78,0x95
  };

  gen tabunsignedchar2gen(const unsigned char tab[],int len){
    gen res=0;
    for (int i=0;i<len;++i){
      res=256*res;
      res+=tab[i];
    }
    return res;
  }
  // rsa_check will return a number of keys if the file is made of encrypted sha256
  // fingerprints, and will set the list of keys accordingly
  // decrypted sha256 keys must be written in basis 256
  // as decimal strings of 3 digits
  // a key has 32 bytes -> 96 digits -> crypted as a 96 hexadecimal BCD number
  // 96 bytes = 768 bits + 1280 leadings bits ignored
  // if after decryption one byte is not in '0'..'9' then the key file is wrong
  int rsa_check(const char * filename,int maxkeys,BYTE hash[][SHA256_BLOCK_SIZE]){
    // 2048 bits key
    //gen rsa_n("30556983006074777238153119417050033796377803388439527860005340326999902386793820226251074714511561407075812479599501874865302578278319769475202313110451510448783794266461205935851713896070734772609406958034158877973097041361961511770051269836310307170258399115935233789006376756279696914861909994161265089406023979340582770078210602481999222884431385627202086122099546391904669923221616360112943964540439315592530076604901633280666259500385969154248745363924897530806256116825070881718288938659701112718863366914419207811508217802754887145264781681001930842410022363032920896943814827354941650810105635438172850387093",context0);
    gen rsa_n(tabunsignedchar2gen(rsa_n_tab,sizeof(rsa_n_tab)));
    gen N=pow(gen(2),768),q;
    // read by blocks of 2048 bits=256 bytes
    FILE * f=fopen(filename,"r");
    if (!f)
      return -1;
    for (int i=0;;++i){
      gen key=0;
      // skip 0x prefix
      for (;;){
	unsigned char c=fgetc(f);
	if (feof(f))
	  break;
	if (c=='\n' || c==' ' || c=='0')
	  continue;
	if (c=='x')
	  break;
	// invalid char
	unlink(filename);
	return -2;
      }
      if (feof(f)){
	fclose(f);
	return i;
      }
      for (int j=0;j<256;++j){
	key = 256*key;
	unsigned char c=fgetc(f);
	if (feof(f)){
	  fclose(f);
	  if (j!=0){ unlink(filename); return 0; }
	  return i;
	}
	if (c==' ' || c=='\n')
	  break;
	if (c>='0' && c<='9')
	  c=c-'0';
	else {
	  if (c>='a' && c<='f')
	    c=10+c-'a';
	  else {
	    fclose(f);
	    unlink(filename);
	    return -3;
	  }
	}
	unsigned char d=fgetc(f);
	if (feof(f)){
	  fclose(f);
	  unlink(filename);
	  return -4;
	}
	if (d==' ' || d=='\n'){
	  key = key/16+int(c);
	  break;
	}
	if (d>='0' && d<='9')
	  d=d-'0';
	else {
	  if (d>='a' && d<='f')
	    d=10+d-'a';
	  else {
	    fclose(f);
	    unlink(filename);
	    return -5;
	  }
	}
	key = key+int(c)*16+int(d);
      }
      // public key decrypt and keep only 768 low bits
      //std::cout << key << '\n' ;
      key=powmod(key,65537,rsa_n);
      key=irem(key,N,q); // q should be irem(rsa_n,12345)
      if (q!=12345){
	fclose(f);
	//unlink(filename);
	return -6;
      }
      // check that key is valid and write in hash[i]
      for (int j=0;j<32;++j){
	// divide 3 times by 256, remainder must be in '0'..'9'
	int o=0;
	int tab[]={1,10,100};
	for (int k=0;k<3;++k){
	  gen r=irem(key,256,q);
	  key=q;
	  if (r.type!=_INT_ || r.val>'9' || r.val<'0'){
	    fclose(f);
	    unlink(filename);
	    return -7;
	  }
	  o+=(r.val-'0')*tab[k];
	}
	if (o<0 || o>255){
	  fclose(f);
	  unlink(filename);
	  return -8;
	}
	if (i<maxkeys)
	  hash[i][31-j]=o;
      }
    }
    fclose(f);
    return maxkeys;
  }

  bool sha_check(const char * filename,int nkeys,BYTE hash[][SHA256_BLOCK_SIZE]){
    // must contain sha256 hash for ndless and khicas files (max 32 hash keys)
    // if more keys are needed modify maxkeys here and in buildsha.cc
    // Keys are generated with buildsha.cc (private program)
    // ./a.out ndless/* khicas*tns luagiac.luax.tns 
    BYTE buf[SHA256_BLOCK_SIZE];
    SHA256_CTX ctx;
    string text;
    FILE * f=fopen(filename,"r");
    if (!f)
      return false;
    for (;;){
      unsigned char c=fgetc(f);
      if (feof(f))
	break;
      text += c;
    }
    fclose(f);
    unsigned char * ptr=(unsigned char *)text.c_str();
    sha256_init(&ctx);
    sha256_update(&ctx, ptr, text.size());
    sha256_final(&ctx, buf);
    for (int i=0;i<nkeys;++i){
      if (!memcmp(hash[i], buf, SHA256_BLOCK_SIZE))
	return true;
    }
    return false;
  }

  void nspire_copy_data(int nkeys,BYTE hash[][SHA256_BLOCK_SIZE],GIAC_CONTEXT){
    const char * dirname="/documents/ndless";
    const char * targetdirname="/exammode/usr/ndless";
    DIR *dp;
    dp = opendir(targetdirname);
    if (!dp)
      mkdir(targetdirname,0755);
    else
      closedir(dp);
    struct dirent *ep;
    dp = opendir (dirname);
    if (!dp)
      return ;
    while ( (ep = readdir (dp)) ){
      string s=ep->d_name;
      int t=s.size();
      if (t<4 || s.substr(t-4,4)!=".tns")
	continue;
      if ( (s=="khicas.tns" || s=="luagiac.luax.tns" || s=="khicaslua.tns" || s=="ptt.tns" || s.substr(0,17)=="ndless_installer_" || s=="ndless_resources.tns" || s=="ndless.cfg.tns")){ // shakeys.tns is not copied, it is retrieved from non-exam mode
	string ss=dirname+("/"+s);
	*logptr(contextptr) << "processing " << s << "\n" ; //" " << ss << " " << (targetdirname+("/"+s)) << '\n';
	if (1 || sha_check(ss.c_str(),nkeys,hash)){ // check done when setting exam mode
	  cp(ss.c_str(),(targetdirname+("/"+s)).c_str());
	}
      }
    }     
  }
  
  DIR * nspire_clear_data(const char * dirname,int nkeys,BYTE hash[][SHA256_BLOCK_SIZE],GIAC_CONTEXT){
    bool toplevel=strcmp(dirname,"/exammode/usr")==0;
    bool ndless=strcmp(dirname,"/exammode/usr/ndless")==0;
    DIR *dp;
    struct dirent *ep;
    dp = opendir (dirname);
    if (!dp)
      return dp;
    string s;
    int t;
    while ( (ep = readdir (dp)) ){
      s=ep->d_name;
      t=s.size();
      if (s=="." || s==".." || s=="NspireLogs.zip" || s=="themes.csv")
	continue;
      int res=0;
      if (t<4 || s.substr(t-4,4)!=".tns"){ // dev, tmp, phoenix, documents, logs, widgets, ptt, metric, wlan, temp_ccp, images
	s=dirname+("/"+s);
	DIR * ptr=nspire_clear_data(s.c_str(),nkeys,hash,contextptr);
	if (ptr && s!="/exammode/usr/ndless" && s!="/exammode/usr/Press-to-Test")
	  res=rmdir(s.c_str());
	// else *logptr(contextptr) << s << '\n'; //
      }
      else {
	if (toplevel || ndless){
	  if (ndless && s=="shakeys.tns")
	    continue;
	  if ( (s=="khicas.tns" || s=="luagiac.luax.tns" || s=="khicaslua.tns" || s=="ptt.tns")){
	    string ss=dirname+("/"+s);
	    if (sha_check(ss.c_str(),nkeys,hash))
	      continue;
	  }
	}
	if (ndless){
	  if ((s.substr(0,17)=="ndless_installer_" || s=="ndless_resources.tns" || s=="ndless.cfg.tns") ){
	    string ss=dirname+("/"+s);
	    if (sha_check(ss.c_str(),nkeys,hash))
	      continue;
	  }
	}
	s=dirname+("/"+s);
	res=unlink(s.c_str());//*logptr(contextptr) << s << '\n'; //
      }
    }
    closedir (dp);
    return dp;
  }
  
  void nspire_clear_data(GIAC_CONTEXT,bool copy){
    int maxkeys=32;
    BYTE hash[maxkeys][SHA256_BLOCK_SIZE]={
    };
    int nkeys=rsa_check("/exammode/usr/ndless/shakeys.tns",maxkeys,hash);
    if (nkeys<=0)
      nkeys=rsa_check("/documents/ndless/shakeys.tns",maxkeys,hash);
    if (lang==1)
      *logptr(contextptr) << "Il y a " << nkeys << " empreintes cryptees de fichiers autorises\n";
    else
      *logptr(contextptr) << "Found " << nkeys << " valid crypted keys of secure files\n";
#if 0
    for (int i=0;i<nkeys;++i){
      *logptr(contextptr) << "{";
      for (int j=0;j<SHA256_BLOCK_SIZE;j++){
	*logptr(contextptr) << hash[i][j] <<",";
      }
      *logptr(contextptr) << "}\n";
    }
#endif
    *logptr(contextptr) << (lang==1?"Teste et efface les fichiers non autorises\n":"Checking and clearing non secure files\n");
    nspire_clear_data("/exammode/usr",nkeys,hash,contextptr);
    *logptr(contextptr) << (lang==1?"Fichiers non autorises effaces\n":"Filesystem checked.\n");
    if (copy)
      nspire_copy_data(nkeys,hash,contextptr);
    else
      *logptr(contextptr) << (lang==1?"Tapez doc doc pour relancer le mode examen\n":"Press doc doc to restart exam mode\n");
  }
#endif

  // maybe we should use int nl_exec(const char *prgm_path, int argsn, char *args[])
#ifdef NSPIRE_LED
#include "kled.cc"
#else
#ifdef NSPIRE_NEWLIB
  // #include "ptt"
  void set_exam_mode(int i,GIAC_CONTEXT){
    unsigned NSPIRE_RTC_ADDR=0x90090000;
    unsigned t1= * (volatile unsigned *) NSPIRE_RTC_ADDR;
    gen n=tabunsignedchar2gen(rsa_n_tab,sizeof(rsa_n_tab));
    gen key=powmod(longlong(t1),65537,n);
    key.uncoerce();
    const char * exec=0;
    if (i==-1)
      exec="/documents/ndless/ptt.tns";
    else
      exec="/exammode/usr/ndless/ptt.tns";
    char clef[]="/documents/rtc.tns";
    char mode[3]="0";
    mode[0] += i;
    if (i==-1){
      mode[0]='-';
      mode[1]='1';
      mode[2]=0;
    }
    char * args[]={clef,mode,0};
    FILE * f=fopen(clef,"w");
    mpz_out_str(f,10,*key._ZINTptr);
    fclose(f);
    // main_ptt(1,0);
#ifdef MICROPY_LIB
    python_free();
#endif   
#ifdef QUICKJS
    js_end(global_js_context);
#endif
    int res=nl_exec(exec,2,args);
    //*logptr(contextptr) << "exam mode " << res << '\n';
    //int res=nl_exec(exec,1,0);
    // int res=nl_exec("/documents/ndless/ptt.tns",1,filenames);
    unlink(clef);
    if (i!=-1)
      exam_mode=i;
  }
#else
  void set_exam_mode(int i,GIAC_CONTEXT){
    exam_mode=i;
  }
#endif
#endif

#if defined NUMWORKS && defined DEVICE
  BYTE bootloader_hash[]={116,198,71,80,107,25,110,250,180,171,154,127,1,174,88,153,108,172,2,218,82,101,93,157,148,76,37,33,102,53,12,136,};
  const int bootloader_size=65480;
  // check bootloade code, skipping exam mode buffer sector
  bool bootloader_sha256_check(size_t addr){
    BYTE buf[SHA256_BLOCK_SIZE];
    SHA256_CTX ctx;
    unsigned char * ptr=(unsigned char *)addr;
    giac_sha256_init(&ctx);
    giac_sha256_update(&ctx, ptr, 16*1024);
    giac_sha256_update(&ctx, ptr+32*1024, bootloader_size-32*1024);
    giac_sha256_final(&ctx, buf);
    if (0) confirm(("@"+hexa_print_INT_(addr)).c_str(),("hash "+print_INT_(buf[0])+","+print_INT_(buf[1])).c_str());
    if (!memcmp(bootloader_hash, buf, SHA256_BLOCK_SIZE))
      return true;
    return false;
  }
#endif
  
  string print_duration(double & duration){
    if (duration<=0)
      return "";
    int s=std::floor(duration+.5);
    int h=s/3600;
    int m=((s+30)%3600)/60;
    char ch[6]="00h00";
    ch[0] += h/10;
    ch[1] += h%10;
    ch[3] += m/10;
    ch[4] += m%10;
    duration=h+m/100.0;
    return ch;
  }
  const char conf_standard[] = "F1 algb\nsimplify(\nfactor(\npartfrac(\ntcollect(\ntexpand(\nsum(\noo\nproduct(\nF2 calc\n'\ndiff(\nintegrate(\nlimit(\nseries(\nsolve(\ndesolve(\nrsolve(\nF5  2d \nreserved\nF4 menu\nreserved\nF6 reg\nlinear_regression_plot(\nlogarithmic_regression_plot(\nexponential_regression_plot(\npower_regression_plot(\npolynomial_regression_plot(\nsin_regression_plot(\nscatterplot(\nmatrix(\nF< poly\nproot(\npcoeff(\nquo(\nrem(\ngcd(\negcd(\nresultant(\nGF(\nF9 arit\n mod \nirem(\nifactor(\ngcd(\nisprime(\nnextprime(\npowmod(\niegcd(\nF7 lin\nmatrix(\ndet(\nmatpow(\nranm(\nrref(\ntran(\negvl(\negv(\nF= list\nmakelist(\nrange(\nseq(\nlen(\nappend(\nranv(\nsort(\napply(\nF3 plot\nplot(\nplotseq(\nplotlist(\nplotparam(\nplotpolar(\nplotfield(\nhistogram(\nbarplot(\nF; real\nexact(\napprox(\nfloor(\nceil(\nround(\nsign(\nmax(\nmin(\nF> prog\n:\n&\n#\nhexprint(\nbinprint(\nf(x):=\ndebug(\npython(\nF8 cplx\nabs(\narg(\nre(\nim(\nconj(\ncsolve(\ncfactor(\ncpartfrac(\nF: misc\n!\nrand(\nbinomial(\nnormald(\nexponentiald(\n and \n or \nperiodic_table\nF? geo\npoint(\nline(\ncircle(\nplane(\nF@ color\ncolor=\nred\ncyan\ngreen\nblue\nmagenta\nyellow\n";

  const char python_conf_standard[] = "F1 misc\nprint(\ninput(\n;\n:\n[]\ndef f(x): return \ntime()\nfrom time import *\nF2 math\nfloor(\nceil(\nround(\nmin(\nmax(\nabs(\nsqrt(\nfrom math import *\nF3 c&rand\nrandint(\nrandom()\nchoice(\nfrom random import *\n.real\n.imag\nphase(\nfrom cmath import *;i=1j\nF4 menu\nreserved\nF5  2d\nreserved\nF6 tortue\nforward(\nbackward(\nleft(\nright(\npencolor(\ncircle(\nreset()\nfrom turtle import *\nF7 linalg\nmatrix(\nadd(\nsub(\nmul(\ninv(\nrref(\ntranspose(\nfrom linalg import *;i=1j\nF8 numpy\narray(\nreshape(\narange(\nlinspace(\nsolve(\neig(\ninv(\nfrom numpy import *;i=1j\nF9 arit\npow(\nisprime(\nnextprime(\nifactor(\ngcd(\nlcm(\niegcd(\nfrom arit import *\nF< color\nred\nblue\ngreen\ncyan\nyellow\nmagenta\nblack\nwhite\nF; draw\nclear_screen();\nshow_screen();\nset_pixel(\ndraw_line(\ndraw_rectangle(\n\ndraw_circle(\ndraw_string(\nfrom graphic import *\nF: plot\nclf()\nplot(\ntext(\narrow(\nscatter(\nbar(\nshow()\nfrom matplotl import *\nF= list\nlist(\nrange(\nlen(\nappend(\nzip(\nsorted(\nmap(\nreversed(\nF> prog\n|\n&\n#\nhex(\nbin(\ndebug(\nfrom cas import *\ncaseval(\"\")\n";
  
  int eqws(char * s,bool eval,GIAC_CONTEXT){ // s buffer must be at least 512 char
    gen g,ge;
    int dconsole_save=dconsole_mode;
    int ss=strlen(s);
    for (int i=0;i<ss;++i){
      if (s[i]==char(0x9c))
	s[i]='\n';
    }
    if (ss>=2 && (s[0]=='#' || s[0]=='"' ||
		  (s[0]=='/' && (s[1]=='/' || s[1]=='*'))
		  ))
      return textedit(s,giacmax(512,ss),contextptr);
    dconsole_mode=0;
    if (eval)
      do_run(s,g,ge,contextptr);
    else {
      if (s[0]==0)
	ge=0;
      else
	ge=gen(s,contextptr);
    }
    dconsole_mode=dconsole_save;
    if (is_undef(ge))
      return textedit(s,giacmax(512,ss),contextptr);
    if (ge.type==giac::_SYMB || (ge.type==giac::_VECT && !ge._VECTptr->empty() && !is_numericv(*ge._VECTptr)) ){
      if (islogo(ge)){
	if (displaylogo()==KEY_SHUTDOWN)
	  return KEY_SHUTDOWN;
	return 0;
      }
      if (ispnt(ge)){
	if (displaygraph(ge,g,contextptr)==KEY_SHUTDOWN)
	  return KEY_SHUTDOWN;
	// aborttimer = Timer_Install(0, check_execution_abort, 100); if (aborttimer > 0) { Timer_Start(aborttimer); }
	return 0;
      }
      if (ge.is_symb_of_sommet(at_program))
	return textedit(s,giacmax(ss,512),contextptr);
      if (taille(ge,256)>=256)
	return 0;
    }
    int xp=xcas_python_eval;
    xcas_python_eval=0;
    Console_FMenu_Init(contextptr);
    gen tmp=eqw(ge,true,contextptr);
    xcas_python_eval=xp;
    Console_FMenu_Init(contextptr);
    if (is_undef(tmp) || tmp==ge || taille(ge,64)>=64)
      return 0;
    string S(tmp.print(contextptr));
    if (S.size()>=512)
      return 0;
    strcpy(s,S.c_str());
    return 1;
  }

  
#define GIAC_TEXTAREA 1
#if GIAC_TEXTAREA
  textArea * edptr=0;
#ifdef SCROLLBAR
  typedef scrollbar TScrollbar;
#endif

  int get_line_number(const char * msg1,const char * msg2){
    string s;
    int res=inputline(msg1,msg2,s,false);
    if (res==KEY_CTRL_EXIT)
      return 0;
    res=strtol(s.c_str(),0,10);
    return res;
  }

  void warn_python(int mode,bool autochange){
    if (mode==-1){
      confirm((lang==1)?"Interpreteur Javascript":"Javascript interpreter",
#ifdef NSPIRE_NEWLIB
	      (lang==1)?"enter: ok":"enter: ok"
#else
	      (lang==1)?"OK: ok":"OK: ok"
#endif	      
	      );
      return;
    }
    if (mode==0)
      confirm(autochange?((lang==1)?"Source en syntaxe Xcas detecte.":"Xcas syntax source code detected."):((lang==1)?"Syntaxe Xcas.":"Xcas syntax."),
#ifdef NSPIRE_NEWLIB
	      "enter: ok"
#else
	      "OK: ok"
#endif
	      );
    if (mode==1)
      if (autochange)
	confirm((lang==1)?"Source en syntaxe Python. Passage":"Python syntax source detected. Setting",
#ifdef NSPIRE_NEWLIB
		(lang==1)?"en Python avec ^=**, enter: ok":"Python mode with ^=**, enter:ok"
#else
		(lang==1)?"en Python avec ^=**, OK: ok":"Python mode with ^=**, OK:ok"
#endif
		);
      else
	confirm((lang==1)?"Syntaxe Python avec ^==**, tapez":"Python syntax with ^==**, type",
#ifdef NSPIRE_NEWLIB
		(lang==1)?"python_compat(2) pour xor. enter: ok":"python_compat(2) for xor. enter: ok"
#else
		(lang==1)?"python_compat(2) pour xor. OK: ok":"python_compat(2) for xor. OK: ok"
#endif
		);
    if (mode==2){
      confirm((lang==1)?"Syntaxe Python avec ^==xor":"Python syntax with ^==xor",
#ifdef NSPIRE_NEWLIB
	      (lang==1)?"python_compat(1) pour **. enter: ok":"python_compat(1) for **. enter: ok"
#else
	      (lang==1)?"python_compat(1) pour **. OK: ok":"python_compat(1) for **. OK: ok"
#endif	      
	      );
    }
    if (mode & 4){
      confirm((lang==1)?"Interpreteur MicroPython":"MicroPython interpreter",
#ifdef NSPIRE_NEWLIB
	      (lang==1)?"enter: ok":"enter: ok"
#else
	      (lang==1)?"OK: ok":"OK: ok"
#endif	      
	      );
    }
  }

  int check_do_graph(giac::gen & ge,const gen & gs,int do_logo_graph_eqw,GIAC_CONTEXT) {
    if (ge.type==giac::_SYMB || (ge.type==giac::_VECT && !ge._VECTptr->empty() && !is_numericv(*ge._VECTptr)) ){
      if (islogo(ge)){
	if (do_logo_graph_eqw & 4){
	  if (displaylogo()==KEY_SHUTDOWN)
	    return KEY_SHUTDOWN;
	}
	return 0;
      }
      if (ispnt(ge)){
	if (do_logo_graph_eqw & 2){
	  if (displaygraph(ge,gs,contextptr)==KEY_SHUTDOWN)
	    return KEY_SHUTDOWN;
	}
	// aborttimer = Timer_Install(0, check_execution_abort, 100); if (aborttimer > 0) { Timer_Start(aborttimer); }
	return 0;
      }
      if ( do_logo_graph_eqw % 2 ==0)
	return 0;
      if (taille(ge,256)>=256 || ge.is_symb_of_sommet(at_program))
	return 0; // sizeof(eqwdata)=44
      gen tmp=eqw(ge,false,contextptr);
      if (!is_undef(tmp) && tmp!=ge){
	//dConsolePutChar(147);
	*giac::logptr(contextptr) << ge.print(contextptr) << '\n';
	ge=tmp;
      }
    }
    return 0;
  }

  // called from editor, return 
  int check_parse(textArea * text,const std::vector<textElement> & v,int python,GIAC_CONTEXT){
    if (v.empty())
      return 0;
    char status[256];
    for (int i=0;i<sizeof(status);++i)
      status[i]=0;
    int shift=0;
#ifdef QUICKJS
    if (xcas_python_eval==-1){
      string s=merge_area(vector<textElement>(v.begin(),v.end()));
      if (s.size() && s[0]=='@')
	s=s.substr(1,s.size()-1);
      else
	s="\"use math\";"+s;
      char * js=js_ck_eval(s.c_str(),&global_js_context);
      if (js){
	s=js;
	free(js);
	process_freeze();
	update_js_vars();
      }
      else
	s="Error";
      if (s.size()>=sizeof(status))
	s=s.substr(0,sizeof(status)-5)+"...";
      sprintf(status,"%s",s.c_str());
      DefineStatusMessage(status,1,0,0);
      return 0;
    }
#endif
#ifdef MICROPY_LIB
    if (xcas_python_eval==1){
#if 0
      if (text->changed){
	std::string tmp=text->filename;
	tmp += (lang==1)?" a ete modifie!":" was modified!";
	if (confirm(tmp.c_str(),
#ifdef NSPIRE_NEWLIB
		    (lang==1)?"enter: sauvegarder, esc: tant pis":"enter: save, esc: discard changes"
#else
		    (lang==1)?"OK: sauvegarder, Back: tant pis":"OK: save, Back: discard changes"
#endif
		    )==KEY_CTRL_F1){
	  save_script(text->filename.c_str(),merge_area(text->elements));
	  text->changed=false;
	}
      }
      string tmp="from "+remove_extension(text->filename)+" import *"; // os error 2 ??
      micropy_ck_eval(tmp.c_str());
#else
      freezeturtle=false;
#if 1
      string s=merge_area(vector<textElement>(v.begin(),v.end()));
      micropy_ck_eval(s.c_str());
#else
      // newlines do not work correctly unless we cut the input
      for (int i=0;i<=v.size();++i){
	if (i==v.size() || (v[i].s.size() && v[i].s[0]!=' ')){
	  string s=merge_area(vector<textElement>(v.begin()+shift,v.begin()+i));
	  micropy_ck_eval(s.c_str());
	  if (parser_errorline>0){
	    parser_errorline += shift;
	    break;
	  }
	  shift=i;
	}
      }
#endif
#endif
      // should detect syntax errors here and return line number
      if (parser_errorline>0){
	//--parser_errorline; // ?? something strange 
	sprintf(status,(lang==1)?"Erreur ligne %i":"Error line %i",parser_errorline);	
      }
      else {
	process_freeze();
	sprintf(status,"%s",(lang==1)?"Syntaxe correcte":"Parse OK");
      }
      DefineStatusMessage(status,1,0,0);
      return parser_errorline;
    }
#endif
    std::string s=merge_area(v); 
    giac::python_compat(python,contextptr);
    if (python>0) s="@@"+s; // force Python translation
    freeze=true;
    giac::gen g(s,contextptr);
    freeze=false;
    int lineerr=giac::first_error_line(contextptr);
    if (lineerr){
      std::string tok=giac::error_token_name(contextptr);
      int pos=-1;
      if (lineerr>=1 && lineerr<=v.size()){
	pos=v[lineerr-1].s.find(tok);
	const std::string & err=v[lineerr-1].s;
	if (pos>=err.size())
	  pos=-1;
	if (python>0){
	  // find 1st token, check if it's def/if/elseif/for/while
	  size_t i=0,j=0;
	  for (;i<err.size();++i){
	    if (err[i]!=' ')
	      break;
	  }
	  std::string firsterr;
	  for (j=i;j<err.size();++j){
	    if (!isalpha(err[j]))
	      break;
	    firsterr += err[j];
	  }
	  // if there is no : at end set pos=-2
	  if (firsterr=="for" || firsterr=="def" || firsterr=="if" || firsterr=="elseif" || firsterr=="while"){
	    for (i=err.size()-1;i>0;--i){
	      if (err[i]!=' ')
		break;
	    }
	    if (err[i]!=':')
	      pos=-2;
	  }
	}
      }
      else {
	lineerr=v.size();
	tok=(lang==1)?"la fin":"end";
	pos=0;
      }
      if (pos>=0)
	sprintf(status,(lang==1)?"Erreur ligne %i a %s":"Error line %i at %s",lineerr,tok.c_str());
      else
	sprintf(status,(lang==1)?"Erreur ligne %i %s":"Error line %i %s",lineerr,(pos==-2?((lang==1)?", : manquant ?":", missing :?"):""));
      DefineStatusMessage(status,1,0,0);
    }
    else {
      set_abort();
      gen gs=g;
      g=protecteval(g,1,contextptr);
      clear_abort();
      giac::ctrl_c=false;
      kbd_interrupted=giac::interrupted=false;
      // define the function
      if (check_do_graph(g,gs,7,contextptr)==KEY_SHUTDOWN)
	return KEY_SHUTDOWN;
      DefineStatusMessage((char *)((lang==1)?"Syntaxe correcte":"Parse OK"),1,0,0);
    }
    DisplayStatusArea();    
    return lineerr;
  }

  void fix_newlines(textArea * edptr){
    edptr->elements[0].newLine=0;
    for (size_t i=1;i<edptr->elements.size();++i){
      edptr->elements[i].newLine=1;
    }
  for (size_t i=0;i<edptr->elements.size();++i){
    string S=edptr->elements[i].s;
    const int cut=160;
    if (S.size()>cut){
      // string too long, cut it
      int j;
      for (j=(4*cut)/5;j>=(2*cut)/5;--j){
	if (!giac::isalphanum(S[j]))
	  break;
      }
      textElement elem; elem.newLine=1; elem.s=S.substr(j,S.size()-j);
      edptr->elements[i].s=S.substr(0,j);
      edptr->elements.insert(edptr->elements.begin()+i+1,elem);
    }
  }
  }

  void fix_mini(textArea * edptr){
    bool minimini=edptr->elements[0].minimini;
    for (size_t i=1;i<edptr->elements.size();++i){
      edptr->elements[i].minimini=minimini;
    }
  }

  int end_do_then(const std::string & s){
    // skip spaces from end
    int l=s.size(),i,i0;
    const char * ptr=s.c_str();
    for (i=l-1;i>0;--i){
      if (ptr[i]!=' '){
	if (ptr[i]==':' || ptr[i]=='{')
	  return 1;
	if (ptr[i]=='}')
	  return -1;
	if (i && ptr[i]==';' && ptr[i-1]=='}')
	  return -1;
	if (i>=2 && ptr[i]==';' && ptr[i-1]==':' && ptr[i-2]=='}')
	  return -1;
	break;
      }
    }
    if (i>0){
      for (i0=i;i0>=0;--i0){
	if (!isalphanum(ptr[i0]) && ptr[i0]!=';' && ptr[i0]!=':')
	  break;
      }
      if (i>i0+2){
	if (ptr[i]==';')
	  --i;
	if (ptr[i]==':')
	  --i;
      }
      std::string keyw(ptr+i0+1,ptr+i+1);
      const char * ptr=keyw.c_str();
      if (strcmp(ptr,"faire")==0 || strcmp(ptr,"do")==0 || strcmp(ptr,"alors")==0 || strcmp(ptr,"then")==0)
	return 1;
      if (strcmp(ptr,"fsi")==0 || strcmp(ptr,"end")==0 || strcmp(ptr,"fi")==0 || strcmp(ptr,"od")==0 || strcmp(ptr,"ftantque")==0 || strcmp(ptr,"fpour")==0 || strcmp(ptr,"ffonction")==0 || strcmp(ptr,"ffunction")==0)
	return -1;
    }
    return 0;
  }

  void add(textArea *edptr,const std::string & s){
    int r=1;
    for (size_t i=0;i<s.size();++i){
      if (s[i]=='\n' || s[i]==char(0x9c))
	++r;
    }
    edptr->elements.reserve(edptr->elements.size()+r);
    textElement cur;
    cur.lineSpacing=2;
    for (size_t i=0;i<s.size();++i){
      char c=s[i];
      if (c!='\n' && c!=char(0x9c)){
	if (c!=char(0x0d))
	  cur.s += c;
	continue;
      }
      string tmp=string(cur.s.begin(),cur.s.end());
      cur.s.swap(tmp);
      edptr->elements.push_back(cur);
      ++edptr->line;
      cur.s="";
    }
    if (cur.s.size()){
      edptr->elements.push_back(cur);
      ++edptr->line;
    }
    fix_newlines(edptr);
  }

  void textArea::set_string_value(int n,const string & s){    
    if (n==-1 || n>=elements.size()){
      textElement t; t.s=s;
      if (!elements.empty())
	t.newLine=1;
      elements.push_back(t);
    }
    else {
      elements[n].s=s;
      if (n)
	elements[n].newLine=1;
    }
    changed=true;
  }

  int textArea::add_entry(int n){
    textElement t; 
    if (n==-1 || n>=elements.size()){
      if (elements.empty())
	elements.push_back(t);
      else {
	t.newLine=1;
	if (!elements.back().s.empty())
	  elements.push_back(t);
      }
      n=elements.size()-1;
    }
    else {
      if (n) t.newLine=1;
      elements.insert(elements.begin()+n,t);
    }
    return n;
  }

  void Graph2d::add_entry(int n){
    if (!hp)
      return;
    if (n==-1 || n>=symbolic_instructions.size()){
      symbolic_instructions.push_back(0);
      n=symbolic_instructions.size();
    }
    hp->add_entry(n);
  }

  int find_indentation(const std::string & s){
    size_t indent=0;
    for (;indent<s.size();++indent){
      if (s[indent]!=' ')
	break;
    }
    return indent;
  }

  void add_indented_line(std::vector<textElement> & v,int & textline,int & textpos){
    // add line
    v.insert(v.begin()+textline+1,v[textline]);
    std::string & s=v[textline].s;
    int indent=find_indentation(s);
    if (!s.empty())
      indent += 2*end_do_then(s);
    //cout << indent << s << ":" << endl;
    if (indent<0)
      indent=0;
    v[textline+1].s=std::string(indent,' ')+s.substr(textpos,s.size()-textpos);
    v[textline+1].newLine=1;
    v[textline].s=s.substr(0,textpos);
    ++textline;
    v[textline].nlines=1; // will be recomputed by cursor moves
    textpos=indent;
  }

  void undo(textArea * text){
    if (text->undoelements.empty())
      return;
    giac::swapint(text->line,text->undoline);
    giac::swapint(text->pos,text->undopos);
    giac::swapint(text->clipline,text->undoclipline);
    giac::swapint(text->clippos,text->undoclippos);
    swap(text->elements,text->undoelements);
  }

  void set_undo(textArea * text){
    text->changed=true;
    text->undoelements=text->elements;
    text->undopos=text->pos;
    text->undoline=text->line;
    text->undoclippos=text->clippos;
    text->undoclipline=text->clipline;
  }

  void add_nl(textArea * text,const std::string & ins){
    std::vector<textElement> & v=text->elements;
    std::vector<textElement> w(v.begin()+text->line+1,v.end());
    v.erase(v.begin()+text->line+1,v.end());
    add(text,ins);
    for (size_t i=0;i<w.size();++i)
      v.push_back(w[i]);
    fix_newlines(text);
    text->changed=true;
  }

  void insert(textArea * text,const char * adds,bool indent){
    size_t n=strlen(adds),i=0;
    if (!n)
      return;
    set_undo(text);
    int l=text->line;
    if (l<0 || l>=text->elements.size())
      return; // invalid line number
    std::string & s=text->elements[l].s;
    int ss=int(s.size());
    int & pos=text->pos;
    if (pos>ss)
      pos=ss;
    std::string ins=s.substr(0,pos);
    for (;i<n;++i){
      if (adds[i]=='\n' || adds[i]==0x1e) {
	break;
      }
      else {
	if (adds[i]!=char(0x0d))
	  ins += adds[i];
      }
    }
    if (i==n){ // no newline in inserted string
      s=ins+s.substr(pos,ss-pos);
      pos += n;
      return;
    }
    std::string S(adds+i+1);
    int decal=ss-pos;
    S += s.substr(pos,decal);
    // cout << S << " " << ins << endl;
    s=ins;
    if (indent){
      pos=s.size();
      int debut=0;
      for (i=0;i<S.size();++i){
	if (S[i]=='\n' || S[i]==0x1e){
	  add_indented_line(text->elements,text->line,pos);
	  // cout << S.substr(debut,i-debut) << endl;
	  text->elements[text->line].s += S.substr(debut,i-debut);
	  pos = text->elements[text->line].s.size();
	  debut=i+1;
	}
      }
      //cout << S << " " << debut << " " << i << S.c_str()+debut << endl;
      add_indented_line(text->elements,text->line,pos);
      text->elements[text->line].s += (S.c_str()+debut);
      fix_newlines(text);
    }
    else 
      add_nl(text,S);
    pos = text->elements[text->line].s.size()-decal;
    fix_mini(text);
  }

  std::string merge_area(const std::vector<textElement> & v){
    std::string s;
    size_t l=0;
    for (size_t i=0;i<v.size();++i)
      l += v[i].s.size()+1;
    s.reserve(l);
    for (size_t i=0;i<v.size();++i){
      s += v[i].s;
      s += '\n';
    }
    return s;
  }

  bool isalphanum(char c){
    return isalpha(c) || (c>='0' && c<='9');
  }

  void search_msg(){
#ifdef NSPIRE_NEWLIB
    DefineStatusMessage((char *)((lang==1)?"enter: suivant, DEL: annuler":"enter: next, DEL: cancel"),1,0,0);
#else
    DefineStatusMessage((char *)((lang==1)?"enter: suivant, DEL: annuler":"enter: next, DEL: cancel"),1,0,0);
#endif
    DisplayStatusArea();    	    
  }  


  void show_status(textArea * text,const std::string & search,const std::string & replace){
    if (text->editable && text->clipline>=0)
      DefineStatusMessage((char *)"PAD: select, COPY: copy, DEL: cut",1,0,0);
    else {
      std::string status("edit ");
#ifdef GIAC_SHOWTIME
      int d=(int(millis()/60000) +time_shift) % (24*60); // minutes
      int heure=d/60;
      int minute=d%60;
      status += char('0'+heure/10);
      status += char('0'+(heure%10));
      status += ':';
      status += char('0'+(minute/10));
      status += char('0'+(minute%10));
#endif
      if (text->editable){
#ifndef NSPIRE_NEWLIB
        status += (xthetat?" t":" x");
#endif
        if (text->python<0){
          status += " QuickJS ";
        }
        else {
          if (text->python & 4)
            status += " MicroPython ";
          else
            status += text->python?(text->python==2?" Py ^xor ":" Py ^=** "):" Xcas ";
        }
        status += giac::remove_extension(text->filename.c_str());
        status += text->changed?" * ":" - ";
        status += giac::printint(text->line+1);
        status += '/';
        status += giac::printint(text->elements.size());
#ifdef HP39
        int k=Setup_GetEntry(0x14);
        if (k&0x4){
          if (k&0x80)
            status +=" ALOCK";
          else
            status += " ALPHA";
        }
        else if (k&0x8){
          if (k&0x80)
            status +=" alock";
          else
            status += " alpha";
        }
#endif
      }
      if (search.size()){
#ifdef NSPIRE_NEWLIB
        status += " enter: " + search;
#else
        status += " EXE: " + search;
#endif
        if (replace.size())
          status += "->"+replace;
      }
      DefineStatusMessage((char *)status.c_str(), 1, 0, 0);
    }
    DisplayStatusArea();    
  }

  bool chk_replace(textArea * text,const std::string & search,const std::string & replace){
    if (replace.size()){
#ifdef NSPIRE_NEWLIB      
      DefineStatusMessage((char *)((lang==1)?"Remplacer? enter: Oui, 8 ou N: Non":"Replace? enter: Yes, 8 or N: No"),1,0,0);
#else
      DefineStatusMessage((char *)((lang==1)?"Remplacer? EXE: Oui, 8 ou N: Non":"Replace? EXE: Yes, 8 or N: No"),1,0,0);
#endif
    }
    else
      search_msg();
    DisplayStatusArea();
    for (;;){
      int key;
      GetKey(&key);
      if (key==KEY_CHAR_MINUS || key==KEY_CHAR_Y || key==KEY_CHAR_9 || key==KEY_CHAR_O || key==KEY_CTRL_EXE || key==KEY_CTRL_OK){
	if (replace.size()){
	  set_undo(text);
	  std::string & s = text->elements[text->line].s;
	  s=s.substr(0,text->pos-search.size())+replace+s.substr(text->pos,s.size()-text->pos);
	  search_msg();
	}
	return true;
      }
      if (key==KEY_CTRL_DEL || (replace.empty() && key==KEY_CTRL_EXIT) || key==KEY_CTRL_LEFT || key==KEY_CTRL_RIGHT || key==KEY_CTRL_UP || key==KEY_CTRL_DOWN){
	show_status(text,search,replace);
	return false;
      }
      if (key==KEY_CHAR_8 || key==KEY_CHAR_N || key==KEY_CTRL_EXIT){
	search_msg();
	return true;
      }
    }
  }

  int check_leave(textArea * text){
    if (nspire_exam_mode==2)
      return 0;
    if (text->editable && text->filename.size()){
      if (text->changed){
	// save or cancel?
	std::string tmp=text->filename;
	if (strcmp(tmp.c_str(),"temp.py")==0){
	  if (confirm((lang==1)?"Les modifications seront perdues":"Changes will be lost",
#ifdef NSPIRE_NEWLIB
		      (lang==1)?"enter: annuler, esc: tant pis":"enter: cancel, esc: confirm"
#else
		      (lang==1)?"OK: annuler, Back: tant pis":"OK: cancel, Back: confirm"
#endif
		      )==KEY_CTRL_F1)
	    return 2;
	  else {
	    return 0;
	  }
	}
	tmp += (lang==1)?" a ete modifie!":" was modified!";
	if (confirm(tmp.c_str(),
#ifdef NSPIRE_NEWLIB
		    (lang==1)?"enter: sauvegarder, esc: tant pis":"enter: save, esc: discard changes"
#else
		    (lang==1)?"OK: sauvegarder, Back: tant pis":"OK: save, Back: discard changes"
#endif
		    )==KEY_CTRL_F1){
	  save_script(text->filename.c_str(),merge_area(text->elements));
	  text->changed=false;
	  return 1;
	}
	return 0;
      }
      return 1;
    }
    return 0;
  }
#ifdef HP39
// 0 not alpha symbol, blue (7) Xcas command, red (2) keyword, cyan (3) number,  green (4) comment, yellow (6) string
  void print(int &X, int &Y, const char *buf, int color, bool revert, bool fake, bool minimini){
    //if (!fake) dbgprintf("print %s X=%i Y=%i color=%i revert=%i\n",buf,X,Y,color,revert);
    if (!buf)
      return;
    // if (!fake) cout << "print:" << buf << " " << strlen(buf) << " " << color << endl;
    if (!isalpha(buf[0]) && color != 2016 && color != 4)
      color = 0;
    if (!fake){
      if (minimini || color == 2016 || color == 4) // comment in small font
        PrintMiniMini(X, Y, buf, revert ? 4 : 0,COLOR_BLACK,COLOR_WHITE);
      else {
        PrintMini(X, Y, buf, revert ? 4 : 0,COLOR_BLACK,COLOR_WHITE);
        // overline/underline style according to color
        if (!revert){
          if (color == 3){ 
            giac::draw_line(X, Y + 13, X + 8 * strlen(buf), Y + 13, 4<<22,context0); 
          }
          if (color == 1){ 
            giac::draw_line(X, Y + 13, X + 8 * strlen(buf), Y + 13, COLOR_BLACK,context0); 
          }
        }
      }
    }
    X += ((minimini || color == 2016 || color == 4) ? 6 : 7) * strlen(buf);
  }

#else
  void print(int &X,int&Y,const char * buf_,int color,bool revert,bool fake,bool minimini){
    int s=strlen(buf_);
    char buf[s+1];
    strcpy(buf,buf_);
    for (int i=0;i<s;++i){
      char & ch=buf[i];
      if (ch=='\n')
	ch='\\';
    }
    if(minimini) 
      X=PrintMiniMini(X, Y, buf, revert?4:0, color, COLOR_WHITE,fake);
    else
      X=PrintMini(X, Y, buf, revert?4:0, color, COLOR_WHITE, fake);
  }
#endif

  void match_print(char * singleword,int delta,int X,int Y,bool match,bool minimini){
    // char buflog[128];sprintf(buflog,"%i %i %s               ",delta,(int)match,singleword);puts(buflog);
    char ch=singleword[delta];
    singleword[delta]=0;
    print(X,Y,singleword,0,false,/* fake*/true,minimini);
    singleword[delta]=ch;
    char buf[4];
    buf[0]=ch;
    buf[1]=0;
    // inverted print: colors are reverted too!
    int color;
    if (minimini)
      color=match?_green:_red;
    else
      color=match?_green:_red;
    print(X,Y,buf,color,true,/*fake*/false,minimini);
  }

  bool match(textArea * text,int pos,int & line1,int & pos1,int & line2,int & pos2){
    line2=-1;line1=-1;
    int linepos=text->line;
    const std::vector<textElement> & v=text->elements;
    if (linepos<0 || linepos>=v.size()) return false;
    const std::string * s=&v[linepos].s;
    int ss=s->size();
    if (pos<0 || pos>=ss) return false;
    char ch=(*s)[pos];
    int open1=0,open2=0,open3=0,inc=0;
    if (ch=='(' || ch=='['
	|| ch=='{'
	){
      line1=linepos;
      pos1=pos;
      inc=1;
    }
    if (
	ch=='}' ||
	ch==']' || ch==')'
	){
      line2=linepos;
      pos2=pos;
      inc=-1;
    }
    if (!inc) return false;
    bool instring=false;
    for (;;){
      for (;pos>=0 && pos<ss;pos+=inc){
	if ((*s)[pos]=='"' && (pos==0 || (*s)[pos-1]!='\\'))
	  instring=!instring;
	if (instring)
	  continue;
	switch ((*s)[pos]){
	case '(':
	  open1++;
	  break;
	case '[':
	  open2++;
	  break;
	case '{':
	  open3++;
	  break;
	case ')':
	  open1--;
	  break;
	case ']':
	  open2--;
	  break;
	case '}':
	  open3--;
	  break;
	}
	if (open1==0 && open2==0 && open3==0){
	  //char buf[128];sprintf(buf,"%i %i",pos_orig,pos);puts(buf);
	  if (inc>0){
	    line2=linepos; pos2=pos;
	  }
	  else {
	    line1=linepos; pos1=pos;
	  }
	  return true;
	} // end if
      } // end for pos
      linepos+=inc;
      if (linepos<0 || linepos>=v.size())
	return false;
      s=&v[linepos].s;
      ss=s->size();
      pos=inc>0?0:ss-1;
    } // end for linepos
    return false;
  }

  std::string get_selection(textArea * text,bool erase){
    int sel_line1,sel_line2,sel_pos1,sel_pos2;
    int clipline=text->clipline,clippos=text->clippos,textline=text->line,textpos=text->pos;
    if (clipline>=0){
      if (clipline<textline || (clipline==textline && clippos<textpos)){
	sel_line1=clipline;
	sel_line2=textline;
	sel_pos1=clippos;
	sel_pos2=textpos;
      }
      else {
	sel_line1=textline;
	sel_line2=clipline;
	sel_pos1=textpos;
	sel_pos2=clippos;
      }
    }
    std::string s(text->elements[sel_line1].s);
    if (erase){
      set_undo(text);
      text->line=sel_line1;
      text->pos=sel_pos1;
      text->elements[sel_line1].s=s.substr(0,sel_pos1)+text->elements[sel_line2].s.substr(sel_pos2,text->elements[sel_line2].s.size()-sel_pos2);
    }
    if (sel_line1==sel_line2){
      return s.substr(sel_pos1,sel_pos2-sel_pos1);
    }
    s=s.substr(sel_pos1,s.size()-sel_pos1)+'\n';
    int sel_line1_=sel_line1;
    for (sel_line1++;sel_line1<sel_line2;sel_line1++){
      s += text->elements[sel_line1].s;
      s += '\n';
    }
    s += text->elements[sel_line2].s.substr(0,sel_pos2);
    if (erase)
      text->elements.erase(text->elements.begin()+sel_line1_+1,text->elements.begin()+sel_line2+1);
    return s;
  }

  void change_mode(textArea * text,int flag,GIAC_CONTEXT){
    if (text->python==-1 || text->python==4)
      return;
    if (text->python!=flag){
      text->python=flag;
      python_compat(text->python,contextptr);
      if (text->python<0)
	xcas_python_eval=-1;
      else if (text->python & 4)
	xcas_python_eval=1;
      show_status(text,"","");
      warn_python(flag,true);
    }
  }  

  void clearLine(int x, int y, color_t color=_WHITE) {
    // clear text line. x and y are text cursor coordinates
    // this is meant to achieve the same effect as using PrintXY with a line full of spaces (except it doesn't waste strings).
    int width=LCD_WIDTH_PX;
    if(x>1) width = C24*(21-x);
    drawRectangle((x-1)*C18, (y-1)*C24, width, C24, color); // was y???
  }

  void mPrintXY(int x, int y, char*msg, int mode, int color) {
    char nmsg[50];
    nmsg[0] = 0x20;
    nmsg[1] = 0x20;
    nmsg[2] = '\0';
    strncat(nmsg, msg, 48);
    PrintXY(x, y, nmsg, mode ,color);
  }

  void drawScreenTitle(char* title, char* subtitle=0) {
#ifdef HP39
    if(title != NULL) mPrintXY(1, 1, title, TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
    if(subtitle != NULL) mPrintXY(1, 2, subtitle, TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
#else
    if(title != NULL) mPrintXY(1, 1, title, TEXT_MODE_NORMAL, TEXT_COLOR_BLUE);
    if(subtitle != NULL) mPrintXY(1, 2, subtitle, TEXT_MODE_NORMAL, TEXT_COLOR_BLACK);
#endif
  }

  int find_color(const char * s,GIAC_CONTEXT){
    if (s[0]=='"')
      return 4;
    if (!isalpha(s[0]))
      return 0;
    char buf[256];
    const char * ptr=s;
    for (int i=0;i<255 && (isalphanum(*ptr) || *ptr=='_'); ++i){
      ++ptr;
    }
    strncpy(buf,s,ptr-s);
    buf[ptr-s]=0;
    if (xcas_python_eval>=0 && (strcmp(buf,"def")==0 || strcmp(buf,"import")==0))
      return 1;
#ifdef MICROPY_LIB
    if (is_python_builtin(buf))
      return 3;
#endif
    //int pos=dichotomic_search(keywords,sizeof(keywords),buf);
    //if (pos>=0) return 1;
    gen g;
    int token=find_or_make_symbol(buf,g,0,false,contextptr);
    //*logptr(contextptr) << s << " " << buf << " " << token << " " << g << endl;
#ifdef QUICKJS
    if (xcas_python_eval==-1){
      if (is_js_keyword(buf))
	return 1;
      return js_token(js_vars.c_str(),buf);
    }
#endif
#ifdef MICROPY_LIB
    if (xcas_python_eval==1){
      micropy_ck_eval("");
      int tok=mp_token(buf);
      if (tok) return tok;
      if (token==T_NUMBER)
	return 2;
      if (token==T_UNARY_OP || token==T_UNARY_OP_38 || token==T_LOGO)
	return 0;
      if (token!=T_SYMBOL)
	return 1;
      return 0;
    }
#endif
   if (token==T_UNARY_OP || token==T_UNARY_OP_38 || token==T_LOGO){
      return 3;
    }
    if (token==T_NUMBER)
      return 2;
    if (token!=T_SYMBOL)
      return 1;
    return 0;
  }

  int strncasecmp_duplicate(const char *s1, const char *s2, size_t n)
  {
    if (n != 0) {
      const unsigned char *us1 = (const unsigned char *)s1;
      const unsigned char *us2 = (const unsigned char *)s2;

      do {
	if (tolower(*us1) != tolower(*us2++))
	  return (tolower(*us1) - tolower(*--us2));
	if (*us1++ == '\0')
	  break;
      } while (--n != 0);
    }
    return (0);
  }

#ifdef HP39
int strncasecmp(const char *s1, const char *s2, size_t n) {
    if(n <= 0) return 0;
    while (*s1 != 0 && *s2 != 0) {
        n--;
        if (tolower(*s1) != tolower(*s2) || n == 0)
            break;
        s1++;
        s2++;
    }

    return tolower(*s1) - tolower(*s2);
}

#endif

  char *strcasestr_duplicate(const char *s, const char *find)
  {
    char c;

    if ((c = *find++) != 0) {
      c = tolower((unsigned char)c);
      size_t len = strlen(find);
      do {
	char sc;
	do {
	  if ((sc = *s++) == 0)
	    return (NULL);
	} while ((char)tolower((unsigned char)sc) != c);
      } while (strncasecmp(s, find, len) != 0);
      s--;
    }
    return ((char *)s);
  }


  /* copy over the next token from an input string, WITHOUT
     skipping leading blanks. The token is terminated by the
     first appearance of tokchar, or by the end of the source
     string.

     The caller must supply sufficient space in token to
     receive any token, Otherwise tokens will be truncated.

     Returns: a pointer past the terminating tokchar.

     This will happily return an infinity of empty tokens if
     called with src pointing to the end of a string. Tokens
     will never include a copy of tokchar.

     A better name would be "strtkn", except that is reserved
     for the system namespace. Change to that at your risk.

     released to Public Domain, by C.B. Falconer.
     Published 2006-02-20. Attribution appreciated.
     Modified by gbl08ma not to skip blanks at the beginning.
  */

  const unsigned char *toksplit(const unsigned char *src, /* Source of tokens */
				char tokchar, /* token delimiting char */
				unsigned char *token, /* receiver of parsed token */
				int lgh) /* length token can receive */
  /* not including final '\0' */
  {
    if (src) {
      while (*src && (tokchar != *src)) {
	if (lgh) {
	  *token++ = *src;
	  --lgh;
	}
	src++;
      }
      if (*src && (tokchar == *src)) src++;
    }
    *token = '\0';
    return src;
  } /* toksplit */

void draw_editor_menu(bool textgr,bool textpython){
#ifdef HP39
    drawRectangle(0,114,LCD_WIDTH_PX,14,giac::_BLACK);
    if (textgr)
      PrintMini(0,114,"pnts | lines| undo| cmds| A<>a | File",4);
    else
      PrintMiniMini(0,114,"tests|struct| undo| cmds| A<>a | File",4);
#else
    waitforvblank();
    drawRectangle(0,205,LCD_WIDTH_PX,17,44444);
    if (textgr)
      PrintMiniMini(0,205,"shift-1 pnts|2 lines|3 undo|4 disp|5 +-|6 curves|7 triangle|8 polygon|9 solid",4,giac::_CYAN,giac::_BLACK);
    else
      PrintMiniMini(0,205,textpython>0?"shift-1 test|2 loop|3 undo|4 misc|5 +-|6 logo|7 lin|8 list|9arit":"shift-1 test|2 loop|3 undo|4 misc|5 +-|6 logo|7 matr|8 cplx",4,44444,giac::_BLACK);
    //draw_menu(1);
#endif
  }


#ifdef HP39
#define C19 16 // 17?
#define C154 96
#define F_KEY_BAR_Y_START 114
static void display(textArea *text, int &isFirstDraw, int &totalTextY, int &scroll, int &textY, GIAC_CONTEXT)
{
  // *logptr(contextptr) << text->lineHeight << '\n';
  bool editable = text->editable;
  int showtitle = !editable && (text->title != NULL);
  ustl::vector<textElement> &v = text->elements;
  if (v.empty())
  {
    textElement cur;
    cur.lineSpacing = 0;
    v.push_back(cur);
  }
  drawRectangle(text->x, text->y, text->width, LCD_HEIGHT_PX, COLOR_WHITE);
  // insure cursor is visible
  if (editable && !isFirstDraw)
  {
    int linesbefore = 0;
    for (int cur = 0; cur < text->line; ++cur)
    {
      linesbefore += v[cur].nlines;
    }
    // line begin Y is at scroll+linesbefore*17, must be positive
    if (linesbefore * C19 + scroll < 0)
      scroll = -C19 * linesbefore;
    linesbefore += v[text->line].nlines;
    // after line Y is at scroll+linesbefore*17
    if (linesbefore * C19 + scroll > C154)
      scroll = C154 - C19 * linesbefore;
  }
  textY = scroll + (showtitle ? C24 : 0) + text->y; // 24 pixels for title (or not)
  int deltax = 0;
  if (editable)
  { // number of pixels between line number and text
    if (v.size() < 10)
    {
      deltax = 8; // 4+2 //!!! 6+2
    }
    else
    {
      if (v.size() < 100)
        deltax = 14; // 2*4+2 //!!! 2*6+2
      else
        deltax = 20; // 3*4+2 //!!! 3*6+2
    }
  }
  int &clipline = text->clipline;
  int &clippos = text->clippos;
  int &textline = text->line;
  int &textpos = text->pos;
  if (textline < 0)
    textline = 0;
  if (textline >= text->elements.size())
    textline = text->elements.size() - 1;
  if (textpos < 0)
    textpos = 0;
  if (textpos > text->elements[textline].s.size())
    textpos = text->elements[textline].s.size();
  // char bufpos[512];  sprintf(bufpos,"%i,%i:%i,%i       ",textpos,textline,text->elements[textline].s.size(),text->elements.size());  puts(bufpos);
  if (clipline >= 0)
  {
    if (clipline >= v.size())
      clipline = -1;
    else
    {
      if (clippos < 0)
        clippos = 0;
      if (clippos >= v[clipline].s.size())
        clippos = v[clipline].s.size() - 1;
    }
  }
  int line1, line2, pos1 = 0, pos2 = 0;
  if (!match(text, text->pos, line1, pos1, line2, pos2) && line1 == -1 && line2 == -1)
    match(text, text->pos - 1, line1, pos1, line2, pos2);
  // char bufpos[512];  sprintf(bufpos,"%i,%i:%i,%i       ",line1,pos1,line2,pos2);  puts(bufpos);
  // if (editable) PrintMini(0, F_KEY_BAR_Y_START, "tests|struct|misc|cmds|A<>a|Fich", MINI_REV);
  if (editable) draw_editor_menu(text->gr,text->python);
  // giac::drawRectangle(text->x, text->y, text->width, LCD_HEIGHT_PX-text->y-editable?8:0, COLOR_WHITE);
  for (int cur = 0; cur < v.size(); ++cur)
  {
    const char *src = v[cur].s.c_str();
    if (cur == 0)
    {
      int l = v[cur].s.size();
      if (l >= 1 && src[0] == '#')
        change_mode(text, 1,contextptr); // text->python=true;
      if (l >= 2 && src[0] == '/' && src[1] == '/')
        change_mode(text, 0,contextptr); // text->python=false;
      if (l >= 8 && src[0] == 'f' && (src[1] == 'o' || src[1] == 'u') && src[2] == 'n' && src[3] == 'c' && src[4] == 't' && src[5] == 'i' && src[6] == 'o' && src[7] == 'n')
        change_mode(text, 0,contextptr); // text->python=false;
      if (l >= 4 && src[0] == 'd' && src[1] == 'e' && src[2] == 'f' && src[3] == ' ')
        change_mode(text, 1,contextptr);                                                                                       // text->python=true;
      drawRectangle(text->x, text->y, text->width, LCD_HEIGHT_PX - text->y - editable ? 12 : 0, COLOR_WHITE); //!!!!! 8
    }
    int textX = text->x;
    bool minimini = v[cur].minimini ? v[cur].minimini == 1 : text->minimini;
    if (v[cur].newLine)
    {
      textY = textY + text->lineHeight + v[cur].lineSpacing;
      if (minimini && cur)
        textY -= 4;
      //*logptr(contextptr) << cur << " " << minimini << " " << textY << '\n';
    }
    if (editable)
    {
      char line_s[16];
      //!!!!!
      // sprint_int(line_s,cur+1);
      sprintf(line_s, "%d", cur + 1);
      if (textY >= text->y && textY <= LCD_HEIGHT_PX - 24) //!!!! 13
        PrintMini(textX, textY, line_s, 0);
    }
    textX = text->x + deltax;
    int tlen = v[cur].s.size();
    char singleword[tlen + 32]; // because of this, a single text element can't have more bytes than 511
    if (cur == textline)
    {
      if (textpos < 0 || textpos > tlen)
        textpos = tlen;
      if (tlen == 0 && text->editable)
      { // cursor on empty line
#if 0
        Cursor_SetPosition(textX,textY+1);
        Cursor_SetFlashMode(1);
        Cursor_SetFlashOn(Setup_GetEntry(0x14));
#else
        drawRectangle(textX, textY, 2, 13, COLOR_BLACK);  
#endif
      }
    }
    bool chksel = false;
    int sel_line1, sel_line2, sel_pos1, sel_pos2;
    if (clipline >= 0)
    {
      if (clipline < textline || (clipline == textline && clippos < textpos))
      {
        sel_line1 = clipline;
        sel_line2 = textline;
        sel_pos1 = clippos;
        sel_pos2 = textpos;
      }
      else
      {
        sel_line1 = textline;
        sel_line2 = clipline;
        sel_pos1 = textpos;
        sel_pos2 = clippos;
      }
      chksel = (sel_line1 <= cur && cur <= sel_line2);
    }
    const char *match1 = 0; // matching parenthesis (or brackets?)
    const char *match2 = 0;
    if (cur == line1)
      match1 = v[cur].s.c_str() + pos1;
    else
      match1 = 0;
    if (cur == line2)
      match2 = v[cur].s.c_str() + pos2;
    else
      match2 = 0;
    // if (cur==textline && !match(v[cur].s.c_str(),textpos,match1,match2) && !match1 && !match2) match(v[cur].s.c_str(),textpos-1,match1,match2);
    // char buf[128];sprintf(buf,"%i %i %i        ",cur,(int)match1,(int)match2);puts(buf);
    const char *srcpos = src + textpos;
    int couleur = v[cur].color;
    int nlines = 1;
    bool linecomment = false;
    while (*src)
    {
      const char *oldsrc = src;
      if ((text->python && *src == '#') ||
          (!text->python && *src == '/' && *(src + 1) == '/')){
        linecomment = true;
        couleur = giac::_GREEN;
        // cout << "comment " << *src << endl;
      }
      if (linecomment || !text->editable)
        src = (const char *)toksplit((const unsigned char *)src, ' ', (unsigned char *)singleword, minimini ? 33 : 22); // break into words; next word
      else
      { // skip string (only with delimiters " ")
        if (*src == '"')
        {
          for (++src; *src; ++src)
          {
            if (*src == '"' && *(src - 1) != '\\')
              break;
          }
          if (*src == '"')
            ++src;
          int i = src - oldsrc;
          strncpy(singleword, oldsrc, i);
          singleword[i] = 0;
        }
        else
        {
          size_t i = 0;
          for (; *src == ' '; ++src)
          { // skip initial whitespaces
            ++i;
          }
          if (i == 0)
          {
            if (isalpha(*src))
            { // skip keyword
              for (; giac::isalphanum(*src) || *src == '_'; ++src)
              {
                ++i;
              }
            }
            // go to next space or alphabetic char
            for (; *src; ++i, ++src)
            {
              if (*src == ' ' || (i && *src == ',') || (text->python && *src == '#') || (!text->python && *src == '/' && *(src + 1) == '/') || *src == '"' || isalpha(*src))
                break;
            }
          }
          strncpy(singleword, oldsrc, i);
          singleword[i] = 0;
          if (i == 0)
          {
            puts(src); // free(singleword);
            return;    // return KEY_CTRL_F2;
          }
        } // end normal case
      }   // end else linecomment case
          // take care of selection
      bool invert = false;
      if (chksel)
      {
        if (cur < sel_line1 || cur > sel_line2)
          invert = false;
        else
        {
          int printpos1 = oldsrc - v[cur].s.c_str();
          int printpos2 = src - v[cur].s.c_str();
          if (cur == sel_line1 && printpos1 < sel_pos1 && printpos2 > sel_pos1)
          {
            // cut word in 2 parts: first part not selected
            src = oldsrc + sel_pos1 - printpos1;
            singleword[sel_pos1 - printpos1] = 0;
            printpos2 = sel_pos1;
          }
          if (cur == sel_line2 && printpos1 < sel_pos2 && printpos2 > sel_pos2)
          {
            src = oldsrc + sel_pos2 - printpos1;
            singleword[sel_pos2 - printpos1] = 0;
            printpos2 = sel_pos2;
          }
          // now singleword is totally unselected or totally selected
          // which one?
          if (cur == sel_line1)
          {
            if (cur == sel_line2)
              invert = printpos1 >= sel_pos1 && printpos2 <= sel_pos2;
            else
              invert = printpos1 >= sel_pos1;
          }
          else
          {
            if (cur == sel_line2)
              invert = printpos2 <= sel_pos2;
            else
              invert = true;
          }
        }
      }
      // check if printing this word would go off the screen, with fake PrintMini drawing:
      int temptextX = 0, temptextY = 0;
      print(temptextX, temptextY, singleword, couleur, false, /*fake*/ true, minimini);
      if (temptextX < text->width && temptextX + textX > text->width - 6)
      {
        if (editable)
          PrintMini(textX, textY, ">", 0);
        // time for a new line
        textX = text->x + deltax;
        textY = textY + text->lineHeight + v[cur].lineSpacing;
        if (minimini)
          textY -= 1;
        ++nlines;
      } // else still fits, print new word normally (or just increment textX, if we are not "on stage" yet)
      if (textY >= text->y && textY <= LCD_HEIGHT_PX - 14){
        temptextX = textX;
        if (editable){
          couleur = linecomment ? giac::_GREEN : find_color(singleword,contextptr);
          // cout << singleword << " " << couleur << endl;
          // 0 symbol, red keyword cyan number, blue command, yellow string
          // cout << singleword << " " << couleur << endl;
          // char ch[32];
          // sprint_int(ch,couleur);
          // puts(singleword); puts(ch);
        }
        else {
          couleur = COLOR_BLACK;
          invert=false;
        }
        if (linecomment || !text->editable || singleword[0] == '"')
          print(textX, textY, singleword, couleur, invert, /*fake*/ false, minimini);
        else { // print two parts, commandname in color and remain in black
          char *ptr = singleword;
          if (isalpha(*ptr)){
            while (giac::isalphanum(*ptr) || *ptr == '_')
              ++ptr;
          }
          char ch = *ptr;
          *ptr = 0;
          print(textX, textY, singleword, couleur, invert, /*fake*/ false, minimini);
          *ptr = ch;
          print(textX, textY, ptr, COLOR_BLACK, invert, /*fake*/ false, minimini);
        }
        // ?add a space removed from token
        if (((linecomment || !text->editable) ? *src : *src == ' ') || v[cur].spaceAtEnd)
        {
          if (*src == ' ')
            ++src;
          print(textX, textY, " ", COLOR_BLACK, invert, false, minimini);
        }
        // ?print cursor, and par. matching
        if (editable)
        {
          if (match1 && oldsrc <= match1 && match1 < src)
            match_print(singleword, match1 - oldsrc, temptextX, textY,
                        line2 != -1,
                        // match2,
                        minimini);
          if (match2 && oldsrc <= match2 && match2 < src)
            match_print(singleword, match2 - oldsrc, temptextX, textY,
                        line1 != -1,
                        // match1,
                        minimini);
        }
        if (editable && cur == textline)
        {
          if (oldsrc <= srcpos && (srcpos < src || (srcpos == src && textpos == tlen)))
          {
            if (textpos >= 2 && v[cur].s[textpos - 1] == ' ' && v[cur].s[textpos - 2] != ' ' && srcpos - oldsrc == strlen(singleword) + 1)
            { // fix cursor position after space
              // char ch[512];
              // sprintf(ch,"%s %i %i %i %i",singleword,strlen(singleword),srcpos-oldsrc,textpos,v[cur].s[textpos-2]);
              // puts(ch);
              singleword[srcpos - oldsrc - 1] = ' ';
            }
            singleword[srcpos - oldsrc] = 0;
            print(temptextX, temptextY, singleword, couleur, false, /*fake*/ true, minimini);
            // drawLine(temptextX, textY+14, temptextX, textY-14, COLOR_BLACK);
            // drawLine(temptextX+1, textY+14, temptextX+1, textY-14, COLOR_BLACK);
#if 0
            Cursor_SetPosition(temptextX,textY+1);
            Cursor_SetFlashMode(1);
            Cursor_SetFlashOn(Setup_GetEntry(0x14));
#else
            drawRectangle(temptextX, textY, 2, 12, COLOR_BLACK); //!!!!
#endif
          }
        }
      } // end if testY visible
      else
      {
        textX += temptextX;
        if (*src || v[cur].spaceAtEnd)
          textX += 4; // size of a PrintMini space
      }
    }
    // free(singleword);
    v[cur].nlines = nlines;
    if (isFirstDraw)
    {
      totalTextY = textY + text->lineHeight + (showtitle ? 0 : C24);
    }
    else if (textY > LCD_HEIGHT_PX - 12)
    {
      break;
    }
  } // end main draw loop
  isFirstDraw = 0;
  if (showtitle)
  {
    clearLine(1, 1);
    drawScreenTitle((char *)text->title);
  }
  // if (editable) draw_menu(1);
}

#else // HP39

  void display(textArea * text,int & isFirstDraw,int & totalTextY,int & scroll,int & textY,GIAC_CONTEXT){
#ifdef CURSOR  
    Cursor_SetFlashOff();
#endif
    // waitforvblank(); drawRectangle(text->x, text->y, LCD_WIDTH_PX, LCD_HEIGHT_PX-text->y, COLOR_WHITE);
    bool editable=text->editable;
    int showtitle = !editable && (text->title != NULL);
    std::vector<textElement> & v=text->elements;
    if (v.empty()) v.push_back(textElement());
    //drawRectangle(text->x, text->y+24, text->width, LCD_HEIGHT_PX-24, COLOR_WHITE);
    // insure cursor is visible
    if (editable && !isFirstDraw){
      int linesbefore=0,cur;
      for (cur=0;cur<text->line;++cur){
	linesbefore += (v[cur].newLine+(v[cur].nlines-1))*(text->lineHeight+v[cur].lineSpacing); //*logptr(contextptr) << cur << "," << v[cur].nlines << " ";
      }
      // line begin Y is at scroll+linesbefore*17, must be positive
      if (linesbefore+scroll<0)
	scroll = -linesbefore;
      linesbefore += (v[cur].newLine+(v[cur].nlines-1))*(text->lineHeight+v[cur].lineSpacing); //*logptr(contextptr) << '\n';
      // after line Y is at scroll+linesbefore*17
      if (linesbefore+scroll>148)
	scroll = 148-linesbefore;
    }
    textY = scroll+(showtitle ? 24 : 0)+text->y; // 24 pixels for title (or not)
    int deltax=0;
    if (editable){
      if (v.size()<10){
	deltax=9;
      }
      else {
	if (v.size()<100)
	  deltax=18;
	else
	  deltax=27;
      }
    }
    if (v.empty()){
      textElement cur;
      cur.s="";
      v.push_back(cur);
    }
    int & clipline=text->clipline;
    int & clippos=text->clippos;
    int & textline=text->line;
    int & textpos=text->pos;
    if (textline<0) textline=0;
    if (textline>=text->elements.size())
      textline=text->elements.size()-1;
    if (textpos<0) textpos=0;
    if (textpos>text->elements[textline].s.size())
      textpos=text->elements[textline].s.size();
    //char bufpos[512];  sprintf(bufpos,"%i,%i:%i,%i       ",textpos,textline,text->elements[textline].s.size(),text->elements.size());  puts(bufpos);
    if (clipline>=0){
      if (clipline>=v.size())
	clipline=-1;
      else {
	if (clippos<0)
	  clippos=0;
	if (clippos>=v[clipline].s.size())
	  clippos=v[clipline].s.size()-1;
      }
    }
    int line1,line2,pos1=0,pos2=0;
    if (!match(text,text->pos,line1,pos1,line2,pos2) && line1==-1 && line2==-1)
      match(text,text->pos-1,line1,pos1,line2,pos2);
    //char bufpos[512];  sprintf(bufpos,"%i,%i:%i,%i       ",line1,pos1,line2,pos2);  puts(bufpos);
    bool firstrect=true;
    for (int cur=0;cur < v.size();++cur) {
      const char* src = v[cur].s.c_str();
      if (cur==0){
	int l=v[cur].s.size();
	if (l>=1 && src[0]=='#')
	  change_mode(text,1,contextptr); // text->python=true;
	if (l>=2 && src[0]=='/' && src[1]=='/')
	  change_mode(text,0,contextptr); // text->python=false;
	if (l>=8 && src[0]=='f' && (src[1]=='o' || src[1]=='u') && src[2]=='n' && src[3]=='c' && src[4]=='t' && src[5]=='i' && src[6]=='o' && src[7]=='n')
	  change_mode(text,0,contextptr); // text->python=false;
	if (l>=4 && src[0]=='d' && src[1]=='e' && src[2]=='f' && src[3]==' ')
	  change_mode(text,1,contextptr); // text->python=true;
	//drawRectangle(text->x, text->y, text->width, LCD_HEIGHT_PX-(editable?17:0), COLOR_WHITE);
      }
      if (cur%3==0 && textY>=(showtitle?24:0) && textY<LCD_HEIGHT_PX)
	waitforvblank();
      int textX=text->x,saveY=textY;
      if(v[cur].newLine) {
	if (v[cur].lineSpacing>4) // avoid large skip
	  v[cur].lineSpacing=4;
	textY=textY+text->lineHeight+v[cur].lineSpacing;
      }
      if (!isFirstDraw && clipline==-1){
	// check if we can skip directly to the next line
	int y=textY+(v[cur].nlines-1)*(text->lineHeight+v[cur].lineSpacing);
	if (y<-text->lineHeight){
	  textY=y;
	  continue;
	}
      }
      int dh=18+v[cur].lineSpacing;
      if (textY+dh+(editable?17:0)>LCD_HEIGHT_PX){
	if (isFirstDraw)
	  dh -= textY+dh+(editable?17:0)-LCD_HEIGHT_PX;
	else {
	  textY = saveY;
	  break;
	}
      }
      if (dh>0 && textY>=(showtitle?24:0)){
	if (firstrect){
	  drawRectangle(textX,text->y,LCD_WIDTH_PX,textY-text->y,COLOR_WHITE);
	  firstrect=false;
	}
	drawRectangle(textX, textY, LCD_WIDTH_PX, dh, COLOR_WHITE);
      }
      if (editable && textY>=(showtitle?24:0)){
	char line_s[16];
	sprint_int(line_s,cur+1);
	os_draw_string_small(textX,textY,COLOR_MAGENTA,_WHITE,line_s);
      }
      textX=text->x+deltax;
      int tlen = v[cur].s.size();
      char singleword[tlen+32];
      // char* singleword = (char*)malloc(tlen+1); // because of this, a single text element can't have more bytes than malloc can provide
      if (cur==textline){
	if (textpos<0 || textpos>tlen)
	  textpos=tlen;
	if (tlen==0 && text->editable){ // cursor on empty line
	  drawRectangle(textX,textY,3,16,COLOR_BLACK);
	  text->cursorx=textX; text->cursory=textY;
	}
      }
      bool chksel=false;
      int sel_line1,sel_line2,sel_pos1,sel_pos2;
      if (clipline>=0){
	if (clipline<textline || (clipline==textline && clippos<textpos)){
	  sel_line1=clipline;
	  sel_line2=textline;
	  sel_pos1=clippos;
	  sel_pos2=textpos;
	}
	else {
	  sel_line1=textline;
	  sel_line2=clipline;
	  sel_pos1=textpos;
	  sel_pos2=clippos;
	}
	chksel=(sel_line1<=cur && cur<=sel_line2);
      }
      const char * match1=0; // matching parenthesis (or brackets?)
      const char * match2=0;
      if (cur==line1)
	match1=v[cur].s.c_str()+pos1;
      else
	match1=0;
      if (cur==line2)
	match2=v[cur].s.c_str()+pos2;
      else
	match2=0;
      // if (cur==textline && !match(v[cur].s.c_str(),textpos,match1,match2) && !match1 && !match2) match(v[cur].s.c_str(),textpos-1,match1,match2);
      // char buf[128];sprintf(buf,"%i %i %i        ",cur,(int)match1,(int)match2);puts(buf);
      const char * srcpos=src+textpos;
      bool minimini=v[cur].minimini;
      int couleur=v[cur].color;
      int nlines=1;
      bool linecomment=false;
      while (*src){
	const char * oldsrc=src;
	if ( (text->python>0 && *src=='#') ||
	     (text->python<=0 && *src=='/' && *(src+1)=='/')){
	  linecomment=true;
	  couleur=4;
	}
	if (linecomment || !text->editable)
	  src = (char*)toksplit((unsigned char*)src, ' ', (unsigned char*)singleword, minimini?50:35); //break into words; next word
	else { // skip string (only with delimiters " ")
	  if (*src=='"'){
	    for (++src;*src;++src){
	      if (*src=='"' && *(src-1)!='\\')
		break;
	    }
	    if (*src=='"')
	      ++src;
	    int i=src-oldsrc;
	    strncpy(singleword,oldsrc,i);
	    singleword[i]=0;
	  }
	  else {
	    size_t i=0;
	    for (;*src==' ';++src){ // skip initial whitespaces
	      ++i;
	    }
	    if (i==0){
	      if (isalpha(*src)){ // skip keyword
		for (;isalphanum(*src) || *src=='_';++src){
		  ++i;
		}
	      }
	      // go to next space or alphabetic char
	      for (;*src;++i,++src){
		if (*src==' ' || (i && *src>=' ' && *src<='/') || (text->python>0 && *src=='#') || (text->python<=0 && *src=='/' && *(src+1)=='/')|| *src=='"' || isalpha(*src))
		  break;
	      }
	    }
	    strncpy(singleword,oldsrc,i);
	    singleword[i]=0;
	    if (i==0){
	      puts(src); // free(singleword);
	      return ; // FIXME KEY_CTRL_F2;
	    }
	  } // end normal case
	} // end else linecomment case
	// take care of selection
	bool invert=false;
	if (chksel){
	  if (cur<sel_line1 || cur>sel_line2)
	    invert=false;
	  else {
	    int printpos1=oldsrc-v[cur].s.c_str();
	    int printpos2=src-v[cur].s.c_str();
	    if (cur==sel_line1 && printpos1<sel_pos1 && printpos2>sel_pos1){
	      // cut word in 2 parts: first part not selected
	      src=oldsrc+sel_pos1-printpos1;
	      singleword[sel_pos1-printpos1]=0;
	      printpos2=sel_pos1;
	    }
	    if (cur==sel_line2 && printpos1<sel_pos2 && printpos2>sel_pos2){
	      src=oldsrc+sel_pos2-printpos1;
	      singleword[sel_pos2-printpos1]=0;
	      printpos2=sel_pos2;
	    }
	    // now singleword is totally unselected or totally selected
	    // which one?
	    if (cur==sel_line1){
	      if (cur==sel_line2)
		invert=printpos1>=sel_pos1 && printpos2<=sel_pos2;
	      else
		invert=printpos1>=sel_pos1;
	    }
	    else {
	      if (cur==sel_line2)
		invert=printpos2<=sel_pos2;
	      else
		invert=true;
	    }
	  }
	}
	//check if printing this word would go off the screen, with fake PrintMini drawing:
	int temptextX = 0,temptextY=0;
	print(temptextX,temptextY,singleword,couleur,false,/*fake*/true,minimini);
	if(temptextX<text->width && temptextX + textX > text->width-6) {
	  if (editable)
	    textX=PrintMini(textX, textY, ">", 4, COLOR_MAGENTA, COLOR_WHITE);	  
	  //time for a new line
	  textX=text->x+deltax;
	  textY=textY+text->lineHeight+v[cur].lineSpacing;
	  if (textY>=(showtitle?24:0)){
	    if (firstrect){
	      drawRectangle(textX,text->y,LCD_WIDTH_PX,textY-text->y,COLOR_WHITE);
	      firstrect=false;
	    }
	    drawRectangle(text->x, textY, LCD_WIDTH_PX, 18+v[cur].lineSpacing, COLOR_WHITE);
	  }
	  ++nlines;
	} //else still fits, print new word normally (or just increment textX, if we are not "on stage" yet)
	if(textY >= (showtitle?24:0) && textY < LCD_HEIGHT_PX) {
	  temptextX=textX;
	  if (editable){
	    couleur=linecomment?5:find_color(singleword,contextptr);
	    if (couleur==1) couleur=COLOR_BLUE;
	    if (couleur==2) couleur=49432; //was COLOR_YELLOWDARK;
	    if (couleur==3) couleur=51712;//33024;
	    if (couleur==4) couleur=COLOR_MAGENTA;
	    if (couleur==5) couleur=_green;
	    //char ch[32];
	    //sprint_int(ch,couleur);
	    //puts(singleword); puts(ch);
	  }
	  if (linecomment || !text->editable || singleword[0]=='"')
	    print(textX,textY,singleword,couleur,invert,/*fake*/false,minimini);
	  else { // print two parts, commandname in color and remain in black
	    char * ptr=singleword;
	    if (isalpha(*ptr)){
	      while (isalphanum(*ptr) || *ptr=='_')
		++ptr;
	    }
	    char ch=*ptr;
	    *ptr=0;
	    print(textX,textY,singleword,couleur,invert,/*fake*/false,minimini);
	    *ptr=ch;
	    print(textX,textY,ptr,COLOR_BLACK,invert,/*fake*/false,minimini);
	  }
	  // ?add a space removed from token
	  if( ((linecomment || !text->editable)?*src:*src==' ') || v[cur].spaceAtEnd){
	    if (*src==' ')
	      ++src;
	    print(textX,textY," ",COLOR_BLACK,invert,false,minimini);
	  }
	  // ?print cursor, and par. matching
	  if (editable){
	    if (match1 && oldsrc<=match1 && match1<src)
	      match_print(singleword,match1-oldsrc,temptextX,textY,
			  line2!=-1,
			  // match2,
			  minimini);
	    if (match2 && oldsrc<=match2 && match2<src)
	      match_print(singleword,match2-oldsrc,temptextX,textY,
			  line1!=-1,
			  //match1,
			  minimini);
	  }
	  if (editable && cur==textline){
	    if (oldsrc<=srcpos && (srcpos<src || (srcpos==src && textpos==tlen))){
	      if (textpos>=2 && v[cur].s[textpos-1]==' ' && v[cur].s[textpos-2]!=' ' && srcpos-oldsrc==strlen(singleword)+1){ // fix cursor position after space
		//char ch[512];
		//sprintf(ch,"%s %i %i %i %i",singleword,strlen(singleword),srcpos-oldsrc,textpos,v[cur].s[textpos-2]);
		//puts(ch);
		singleword[srcpos-oldsrc-1]=' ';
	      }
	      singleword[srcpos-oldsrc]=0;
	      print(temptextX,temptextY,singleword,couleur,false,/*fake*/true,minimini);
	      //drawLine(temptextX, textY+14, temptextX, textY-14, COLOR_BLACK);
	      //drawLine(temptextX+1, textY+14, temptextX+1, textY-14, COLOR_BLACK);
	      drawRectangle(temptextX-1,textY,3,16,COLOR_BLACK);
	      text->cursorx=temptextX-1; text->cursory=textY;
	    }
	  }
	} // end if testY visible
	else {
	  textX += temptextX;
	  if(*src || v[cur].spaceAtEnd) textX += 7; // size of a PrintMini space
	}
      } // end while (*src)
      // free(singleword);
      v[cur].nlines=nlines; //if (cur<6) *logptr(contextptr) << cur << ":" << src << nlines << '\n';
      if (isFirstDraw) 
        totalTextY = textY+(showtitle ? 0 : C24);
    } // end main draw loop (for cur<v.size())
    int dh=LCD_HEIGHT_PX-textY-text->lineHeight-(editable?17:0);
    if (dh>0)
      drawRectangle(0, textY+text->lineHeight, LCD_WIDTH_PX, dh, COLOR_WHITE);
    isFirstDraw=0;
    if(showtitle) {
      waitforvblank();
      drawRectangle(0, 0, LCD_WIDTH_PX, 24, _WHITE);
      drawScreenTitle((char*)text->title);
    }
    //if (editable)
    if (editable){
      draw_editor_menu(text->gr,text->python);
    }
#ifdef SCROLLBAR
    int scrollableHeight = LCD_HEIGHT_PX-24*(showtitle ? 2 : 1)-text->y;
    //draw a scrollbar:
    if(text->scrollbar) {
      TScrollbar sb;
      sb.I1 = 0;
      sb.I5 = 0;
      sb.indicatormaximum = totalTextY;
      sb.indicatorheight = scrollableHeight;
      sb.indicatorpos = -scroll;
      sb.barheight = scrollableHeight;
      sb.bartop = (showtitle ? 24 : 0)+text->y;
      sb.barleft = text->width - 6;
      sb.barwidth = 6;
    
      Scrollbar(&sb);
    }
#endif
  }
#endif // HP39

  bool move_to_word(textArea * text,const std::string & s,const std::string & replace,int & isFirstDraw,int & totalTextY,int & scroll,int & textY,GIAC_CONTEXT){
    if (!s.size())
      return false;
    int line=text->line,pos=text->pos;
    if (line>=text->elements.size())
      line=0;
    if (pos>=text->elements[line].s.size())
      pos=0;
    for (;line<text->elements.size();++line){
      int p=text->elements[line].s.find(s,pos);
      if (p>=0 && p<text->elements[line].s.size()){
	text->line=line;
	text->clipline=line;
	text->clippos=p;
	text->pos=p+s.size();
	display(text,isFirstDraw,totalTextY,scroll,textY,contextptr); // this modifies text->elements[].nlines (no idea why), 2 calls insure scrolling is adequate
	display(text,isFirstDraw,totalTextY,scroll,textY,contextptr);
	text->clipline=-1;
	return chk_replace(text,s,replace);
      }
      pos=0;
    }
    for (line=0;line<text->line;++line){
      int p=text->elements[line].s.find(s,0);
      if (p>=0 && p<text->elements[line].s.size()){
	text->line=line;
	text->clipline=line;
	text->clippos=p;
	text->pos=p+s.size();
	display(text,isFirstDraw,totalTextY,scroll,textY,contextptr);
	display(text,isFirstDraw,totalTextY,scroll,textY,contextptr); // 2 callslike above
	text->clipline=-1;
	return chk_replace(text,s,replace);
      }
    }
    return false;
  }

  int load_script(const char * filename,std::string & s){
    const char * ch =read_file(filename);
    s=ch?ch:"";
    return 1;
  }

  void save_script(const char * filename,const string & s){
    if (nspire_exam_mode==2)
      return;
#ifdef NUMWORKS
    char buf[s.size()+2];
    buf[0]=1;
    strcpy(buf+1,s.c_str());
#else
    char buf[s.size()+1];
    strcpy(buf,s.c_str());    
#endif
#ifdef NSPIRE_NEWLIB
    char filenametns[strlen(filename)+5];
    strcpy(filenametns,filename);
    int l=strlen(filenametns);
    if (l<4 || strncmp(filename+l-4,".tns",4))
      strcpy(filenametns+strlen(filename),".tns");
    write_file(filenametns,buf);
#else
    write_file(filename,buf);
#endif
  }

  bool textedit(char * s,int bufsize,bool OKparse,const giac::context * contextptr,const char * filename){
    if (!s)
      return false;
    int ss=strlen(s);
    if (ss==0){
      *s=' ';
      s[1]=0;
      ss=1;
    }
    textArea ta;
    ta.elements.clear();
    ta.editable=true;
    ta.clipline=-1;
    ta.changed=false;
    ta.filename=filename?filename:"temp.py";
#ifdef HP39
    ta.y=12;
#else
    ta.y=0;
#endif
    ta.python=python_compat(contextptr);
    ta.allowEXE=false;//true; // set back to true later
    ta.OKparse=OKparse;
    bool str=s[0]=='"' && s[ss-1]=='"';
    if (str){
      s[ss-1]=0;
      add(&ta,s+1);
    }
    else
      add(&ta,s);
    ta.line=0;
    ta.pos=ta.elements[ta.line].s.size();
    int res=doTextArea(&ta,contextptr);
    drawRectangle(0,0,LCD_WIDTH_PX,LCD_HEIGHT_PX,_WHITE);
    os_hide_graph();
    if (res==TEXTAREA_RETURN_EXIT)
      return false;
    string S(merge_area(ta.elements));
    if (str)
      S='"'+S+'"';
    int Ssize=S.size();
    if (Ssize<bufsize){
      strcpy(s,S.c_str());
      for (--Ssize;Ssize>=0;--Ssize){
	if ((unsigned char)s[Ssize]==0x9c || s[Ssize]=='\n')
	  s[Ssize]=0;
	if (s[Ssize]!=' ')
	  break;
      }
      return true;
    }
    return false;
  }

  bool textedit(char * s,int bufsize,const giac::context * contextptr){
    return textedit(s,bufsize,false,contextptr);
  }

#if 0
  int get_filename(char * filename,const char * extension){
    return 0;
  }
#else
  int get_filename(char * filename,const char * extension){
    handle_f5();
    string str;
#ifdef NSPIRE_NEWLIB
    int res=inputline((lang==1)?"esc ou chaine vide: annulation":"esc or empty string: cancel",(lang==1)?"Nom de fichier:":"Filename:",str,false);
#else
    int res=inputline((lang==1)?"EXIT ou chaine vide: annulation":"EXIT or empty string: cancel",(lang==1)?"Nom de fichier:":"Filename:",str,false);
#endif
    if (res==KEY_CTRL_EXIT || str.empty())
      return 0;
    strcpy(filename,str.c_str());
    int s=strlen(filename);
    if (strcmp(filename+s-3,extension))
      strcpy(filename+s,extension);
    // if file already exists, warn, otherwise create
    if (!file_exists(filename))
      return 1;
    if (confirm((lang==1)?"  Le fichier existe!":"  File exists!",
#ifdef NSPIRE_NEWLIB
		(lang==1)?"enter: ecraser, esc: annuler":"enter:overwrite, esc: cancel"
#else
		(lang==1)?"OK: ecraser,Back: annuler":"OK:overwrite, Back: cancel"
#endif
		)==KEY_CTRL_F1)
      return 1;
    return 0;
  }
#endif

  const char * input_matrix(const gen &g,gen & ge,GIAC_CONTEXT){
#if defined MICROPY_LIB || defined QUICKJS
    if (xcas_python_eval){
      if (ge.type==_VECT)
	ge.subtype=0;
      static string input_matrix_s=g.print(contextptr)+'='+ge.print(contextptr);
      return input_matrix_s.c_str();
    }
#endif
    if (ge.type==giac::_VECT)
      sto(ge,g,contextptr);
    return "";
  }    
  
  const char * input_matrix(bool list,GIAC_CONTEXT){
    static std::string * sptr=0;
    if (!sptr)
      sptr=new std::string;
    *sptr="";
    giac::gen v(giac::_VARS(0,contextptr));
    giac::vecteur w;
    if (v.type==giac::_VECT){
      for (size_t i=0;i<v._VECTptr->size();++i){
	giac::gen & tmp = (*v._VECTptr)[i];
	if (tmp.type==giac::_IDNT){
	  giac::gen tmpe(protecteval(tmp,1,contextptr));
	  if (list){
	    if (tmpe.type==giac::_VECT && !ckmatrix(tmpe))
	      w.push_back(tmp);
	  }
	  else {
	    if (ckmatrix(tmpe))
	      w.push_back(tmp);
	  }
	}
      }
    }
    std::string msg;
    if (w.empty())
      msg=(lang==1)?(list?"Creer nouvelle liste":"Creer nouvelle matrice"):(list?"Create new list":"Create new matrix");
    else
      msg=(((lang==1)?"Creer nouveau ou editer ":"Create new or edit ")+(w.size()==1?w.front():giac::gen(w,giac::_SEQ__VECT)).print(contextptr));
    handle_f5();
    if (inputline(msg.c_str(),((lang==1)?"Nom de variable:":"Variable name:"),*sptr,false) && !sptr->empty() && isalpha((*sptr)[0])){
      giac::gen g(*sptr,contextptr);
      giac::gen ge(protecteval(g,1,contextptr));
      if (g.type==giac::_IDNT){
	if (ge.type==giac::_VECT){
	  ge=eqw(ge,true,contextptr);
	  ge=protecteval(ge,1,contextptr);
	  return input_matrix(g,ge,contextptr);
	  if (ge.type==giac::_VECT)
	    sto(ge,g,contextptr);
	  else
	    cout << "edited " << ge << endl;
	  return ""; // return sptr->c_str();
	}
	if (ge==g || confirm_overwrite()){
	  *sptr="";
	  if (inputline(((lang==1)?(list?"Nombre d'elements":"Nombre de lignes"):(list?"Elements number":"Line number")),"",*sptr,true)){
	    int l=strtol(sptr->c_str(),0,10);
	    if (l>0 && l<256){
	      int c;
	      if (list)
		c=0;
	      else {
		std::string tmp(*sptr+((lang==1)?" lignes.":" lines."));
		*sptr="";
		inputline(tmp.c_str(),(lang==1)?"Colonnes:":"Columns:",*sptr,true);
		c=strtol(sptr->c_str(),0,10);
	      }
	      if (c==0){
		ge=giac::vecteur(l);
	      }
	      else {
		if (c>0 && l*c<256)
		  ge=giac::_matrix(giac::makesequence(l,c),contextptr);
	      }
	      ge=eqw(ge,true,contextptr);
	      ge=protecteval(ge,1,contextptr);
	      return input_matrix(g,ge,contextptr);
	    } // l<256
	  }
	} // ge==g || overwrite confirmed
      } // g.type==_IDNT
      else {
	invalid_varname();
      }	
    } // isalpha
    return 0;
  }

  std::string get_searchitem(std::string & replace){
    replace="";
    std::string search;
    handle_f5();
#ifdef NSPIRE_NEWLIB
    int res=inputline((lang==1)?"esc ou chaine vide: annulation":"esc or empty string: cancel",(lang==1)?"Chercher:":"Search:",search,false);
    if (search.empty() || res==KEY_CTRL_EXIT)
      return "";
    replace="";
    std::string tmp=((lang==1)?"esc: recherche seule de ":"esc: search only ")+search;
#else
    int res=inputline((lang==1)?"EXIT ou chaine vide: annulation":"EXIT or empty string: cancel",(lang==1)?"Chercher:":"Search:",search,false);
    if (search.empty() || res==KEY_CTRL_EXIT)
      return "";
    replace="";
    std::string tmp=((lang==1)?"EXIT: recherche seule de ":"EXIT: search only ")+search;
#endif
    handle_f5();
    res=inputline(tmp.c_str(),(lang==1)?"Remplacer par:":"Replace by:",replace,false);
    if (res==KEY_CTRL_EXIT)
      replace="";
    return search;
  }

  bool tooltip(int x,int y,int pos,const char * editline,GIAC_CONTEXT){
    char cmdline[strlen(editline)+1];
    strcpy(cmdline,editline);
    cmdline[pos]=0;
    int l=strlen(cmdline);
    char buf[l+1];
    strcpy(buf,cmdline);
    bool openpar=l && buf[l-1]=='(';
    if (openpar){
      buf[l-1]=0;
      --l;
    }
    for (;l>0;--l){
      if (!isalphanum(buf[l-1]) && buf[l-1]!='_')
	break;
    }
    // cmdname in buf+l
    const char * cmdname=buf+l,*cmdnameorig=cmdname;
    int l1=strlen(cmdname);
    if (l1<2)
      return false;
    const char * howto=0,*syntax=0,*related=0,*examples=0;
    if (l1>0 && has_static_help(cmdname,lang | 0x100,howto,syntax,related,examples) && examples){
      // display tooltip
      if (x<0)
	x=os_draw_string(0,y,_BLACK,1234,editline,true); // fake print -> x position // replaced cmdline by editline so that tooltip is at end
      x+=2;
      y+=4;
      drawRectangle(x,y,6,10,65529);
      draw_line(x,y,x+6,y,_BLACK);
      draw_line(x,y,x+3,y+3,_BLACK);
      draw_line(x+6,y,x+3,y+3,_BLACK);
      y-=4;
      x+=7;
      int bg=65529; // background
      x=os_draw_string_small(x,y,_BLACK,bg,": ",false);
      if (howto && strlen(howto)){
#ifdef NSPIRE_NEWLIB
	y-=2;
#endif
	os_draw_string_small(x,y,_BLACK,bg,
#ifdef NUMWORKS
			     remove_accents(howto).c_str(),
#else
			     howto,
#endif
			     false);
#ifdef NSPIRE_NEWLIB
	y+=12;
#else
	y+=11;
#endif
      }
      string toolt;
      if (related && strlen(related)){
	toolt += cmdname;
	toolt += '(';
	if (syntax && strlen(syntax))
	  toolt += syntax;
	else
	  toolt += "arg";
	toolt += ')';
	toolt += ' ';
	if (related)
	  toolt += related;
      }
      else
	toolt+=examples;
      os_draw_string_small(x,y,_BLACK,bg,toolt.c_str(),false);
      return true;
    }
    return false;
  }

  void textarea_help_insert(textArea * text,int exec,GIAC_CONTEXT){
    string curs=text->elements[text->line].s.substr(0,text->pos);
    if (!curs.empty()){
      int b;
      string adds=help_insert(curs.c_str(),b,exec,contextptr);
      if (!adds.empty()){
	if (b>0){
	  std::string & s=text->elements[text->line].s;
	  if (b>text->pos)
	    b=text->pos;
	  if (b>s.size())
	    b=s.size();
	  s=s.substr(0,text->pos-b)+s.substr(text->pos,s.size()-text->pos);//+s.substr(b,s.size()-b);
	}
	insert(text,adds.c_str(),false);
      }
    }
  }
  
  int doTextArea(textArea* text,GIAC_CONTEXT) {
#ifdef QUICKJS
    update_js_vars();
#endif
    int scroll = 0;
    int isFirstDraw = 1;
    int totalTextY = 0,textY=0;
    bool editable=text->editable;
    int showtitle = !editable && (text->title != NULL);
    int scrollableHeight = LCD_HEIGHT_PX-24*(showtitle ? 2 : 1)-text->y;
    std::vector<textElement> & v=text->elements;
    std::string search,replace;
    show_status(text,search,replace);
    if (text->line>=v.size())
      text->line=0;
    display(text,isFirstDraw,totalTextY,scroll,textY,contextptr);
    bool keytooltip=false;
    while(1) {
      if (text->line>=v.size())
	text->line=0;
      if (!keytooltip)
        display(text,isFirstDraw,totalTextY,scroll,textY,contextptr);
      if(text->type == TEXTAREATYPE_INSTANT_RETURN) return 0;
      int keyflag = GetSetupSetting( (unsigned int)0x14);
      int key;
      GetKey(&key);
#ifdef HP39
      show_status(text,"","");
      if (key==KEY_CTRL_F5){
        handle_f5();
        continue;
      }
      if (key==KEY_CTRL_F6)
        key=KEY_CTRL_MENU;
      if (key==KEY_CTRL_F4){
        char buf[512];
        if (showCatalog(buf,0,0))
          insert(text,buf,true);
        continue;
      }
#endif
      if (keytooltip){
	keytooltip=false;
	if (key==KEY_CTRL_RIGHT && text->pos==text->elements[text->line].s.size())
	  key=KEY_CTRL_OK;
	if (key==KEY_CTRL_EXIT)
	  continue;
	if (key==KEY_CTRL_DOWN || key==KEY_CTRL_VARS)
	  key=KEY_BOOK;
	if (key==KEY_CTRL_OK ){
	  textarea_help_insert(text,key,contextptr);
	  continue;
	}
      }
      if (key==KEY_SHUTDOWN)
	return key;
      if (key==KEY_CTRL_F3) // Numworks has no UNDO key
	key=KEY_CTRL_UNDO;
#if 1
      if (key == KEY_CTRL_SETUP) {
	menu_setup(contextptr);
	continue;
      }
#endif
      if (key!=KEY_CTRL_PRGM && key!=KEY_CHAR_FRAC)
	translate_fkey(key);
      if (key==KEY_CHAR_NORMAL)
	key=KEY_CTRL_F15;
      if (key==KEY_CHAR_FACTOR)
	key=KEY_CTRL_F16;
      //char keylog[32];sprint_int(keylog,key); puts(keylog);
      show_status(text,search,replace);
      int & clipline=text->clipline;
      int & clippos=text->clippos;
      int & textline=text->line;
      int & textpos=text->pos;
      if (key==KEY_CTRL_CUT && clipline<0) // if no selection, CUT -> pixel menu
	key=KEY_CTRL_F3;
      if (!editable && (key==KEY_CHAR_ANS || key==KEY_BOOK || key=='\t' || key==KEY_CTRL_EXE))
	return key;
      if (editable){
	if (key=='\t'){
	  int indent=0; // indent deduced from prev line
	  if (textline!=0){
	    std::string & s=v[textline-1].s;
	    indent=find_indentation(s);
	    if (!s.empty())
	      indent+=2*end_do_then(s);
	  }
	  std::string & s=v[textline].s;
	  int curindent=find_indentation(s);
	  int diff=curindent-indent;
	  if (diff>0){
	    s=s.substr(diff,s.size()-diff);
	    if (textpos>diff)
	      textpos -= diff;
	    else
	      textpos = 0;
	    continue;
	  }
	  if (diff<0){
	    s=string(-diff,' ')+s;
	    textpos += -diff;
	    continue;
	  }
	  key=KEY_BOOK;
	}
	if (key==KEY_BOOK){
	  textarea_help_insert(text,0,contextptr);
	  continue;
	}
	if (key==KEY_CHAR_FRAC && clipline<0){
	  if (textline==0) continue;
	  std::string & s=v[textline].s;
	  std::string & prev_s=v[textline-1].s;
	  int indent=find_indentation(s),prev_indent=find_indentation(prev_s);
	  if (!prev_s.empty())
	    prev_indent += 2*end_do_then(prev_s);
	  int diff=indent-prev_indent; 
	  if (diff>0 && diff<=s.size())
	    s=s.substr(diff,s.size()-diff);
	  if (diff<0)
	    s=string(-diff,' ')+s;
	  textpos -= diff;
	  continue;
	}
	if (key==KEY_CHAR_ANS){
	  displaylogo();
	  continue;
	}
	if (key>=KEY_SELECT_LEFT && key<=KEY_SELECT_RIGHT){
	  if (clipline<0){
	    clipline=textline;
	    clippos=textpos;
	    show_status(text,search,replace);
	  }
	  if (key==KEY_SELECT_LEFT){
	    if (textpos)
	      --textpos;
	    else {
	      if (textline){
		--textline;
		textpos=v[textline].s.size();
	      }
	    }
	  }
	  if (key==KEY_SELECT_RIGHT){
	    if (textpos<v[textline].s.size())
	      ++textpos;
	    else {
	      if (textline<v.size()){
		++textline;
		textpos=0;
	      }
	    }
	  }
	  if (key==KEY_SELECT_UP){
	    if (textline){
	      --textline;
	      textpos=giacmin(textpos,v[textline].s.size());
	    }
	  }
	  if (key==KEY_SELECT_DOWN){
	    if (textline<v.size()){
	      ++textline;
	      textpos=giacmin(textpos,v[textline].s.size());
	    }
	  }
	}
	if (key==KEY_CTRL_CLIP) {
#if 1
	  if (clipline>=0){
	    copy_clipboard(get_selection(text,false),true);
	    clipline=-1;
	  }
	  else {
	    clipline=textline;
	    clippos=textpos;
	    show_status(text,search,replace);
	  }
#else
	  copy_clipboard(v[textline].s,false);
	  DefineStatusMessage((char*)"Line copied to clipboard", 1, 0, 0);
	  DisplayStatusArea();
#endif
	  continue;
	}
	if (key==KEY_CTRL_F5){
	  bool minimini=!v[0].minimini;
	  for (int i=0;i<v.size();++i)
	    v[i].minimini=minimini;
	  text->lineHeight=minimini?13:17;
	  isFirstDraw=1;
	  display(text,isFirstDraw,totalTextY,scroll,textY,contextptr);
	  continue;
	}
	if (clipline<0){
	  const char * adds;
    //dbgprintf("key 4 %i %i\n",key,clipline);
#if 1
	  if ( (key>=KEY_CTRL_F1 && key<=KEY_CTRL_F4) || key==KEY_CTRL_F6 ||
	       (key >= KEY_CTRL_F7 && key <= KEY_CTRL_F16)
	       ){
	    string le_menu;
	    if (text->gr) { // geometry menu
	      le_menu="F1 points\npoint(\nmidpoint(\ncenter(\nelement(\nsingle_inter(\ninter(\nlegende(\ntrace(\nF2 lines\nsegment(\nline(\nhalf_line(\nvector(\nparallel(\nperpendicular(\ntangent(\nplane(\ncircle(\nF4 disp\ndisplay=\nfilled\nred\nblue\ngreen\ncyan\nmagenta\nyellow\nF6 curves\ncircle(\nellipse(\nhyperbola(\nparabola(\nplot(\nplotparam(\nplotpolar(\nplotode(\nF7 triangle\ntriangle(\nequilateral_triangle(\nmedian(\nperpen_bisector(\nbisector(\nisobarycenter(\nF8 polygon\nsquare(\nrectangle(\nquadrilateral(\nhexagon(\npolygon(\nisopolygon(\nvertices(\nF9 3d\nplane(\ncube(\ntetrahedron(\nsphere(\ncone(\nhalf_cone(\ncylinder(\nplot3d(\nF: transf\nprojection(\nreflection(\ntranslation(\nrotation(\nhomothety(\nsimilarity(\nF; geodiff\ntangent(\nosculating_circle(\nevolute(\ncurvature(\nfrenet(\noctahedron(\ndodecahedron(\nicosahedron(\nF< mesures\ndistance(\ndistance2(\nradius(\naire(\nperimetre(\npente(\nangle(\nF= test\nis_collinear(\nis_concyclic(\nis_coplanar(\nis_cospherical(\nis_element(\nis_parallel(\nis_perpendicular(\nF> analyt\ncoordonnees(\nequation(\nparameq(\nabscisse(\nordonnee(\naffixe(\narg(\n";
	    } else {
	      if (xcas_python_eval==1)//text->python?
		le_menu="F1 test\nif \nelse \n<\n>\n==\n!=\n&&\n||\nF2 loop\nfor \nfor in\nrange(\nwhile \nbreak\ndef\nreturn \n#\nF4 misc\n:\n;\n_\n!\n%\nfrom  import *\nprint(\ninput(\nF6 tortue\nforward(\nbackward(\nleft(\nright(\npencolor(\ncircle(\nreset()\nfrom turtle import *\nF: plot\nplot(\ntext(\narrow(\nlinear_regression_plot(\nscatter(\naxis(\nbar(\nfrom matplotl import *\nF7 linalg\nadd(\nsub(\nmul(\ninv(\ndet(\nrref(\ntranspose(\nfrom linalg import *\nF< color\nred\nblue\ngreen\ncyan\nyellow\nmagenta\nblack\nwhite\nF; draw\nset_pixel(\ndraw_line(\ndraw_rectangle(\nfill_rect(\ndraw_polygon(\ndraw_circle(\ndraw_string(\nfrom graphic import *\nF8 numpy\narray(\nreshape(\narange(\nlinspace(\nsolve(\neig(\ninv(\nfrom numpy import *\nF9 arit\npow(\nisprime(\nnextprime(\nifactor(\ngcd(\nlcm(\niegcd(\nfrom arit import *\n";
	      if (xcas_python_eval<=0)
		le_menu="F1 test\nif \nelse \n<\n>\n==\n!=\nand\nor\nF2 loop\nfor \nfor in\nrange(\nwhile \nbreak\nf(x):=\nreturn \nvar\nF4 misc\n;\n:\n_\n!\n%\n&\nprint(\ninput(\nF6 tortue\navance\nrecule\ntourne_gauche\ntourne_droite\nrond\ndisque\nrepete\nefface\nF7 lin\nmatrix(\ndet(\nmatpow(\nranm(\nrref(\ntran(\negvl(\negv(\nF9 arit\n mod \nirem(\nifactor(\ngcd(\nisprime(\nnextprime(\npowmod(\niegcd(\nF< plot\nplot(\nplotseq(\nplotlist(\nplotparam(\nplotpolar(\nplotfield(\nhistogram(\nbarplot(\nF: misc\n<\n>\n_\n!\n % \nrand(\nbinomial(\nnormald(\nF8 cplx\nabs(\narg(\nre(\nim(\nconj(\ncsolve(\ncfactor(\ncpartfrac(\n";
	      if (xcas_python_eval>=0)
		le_menu += "F= list\nmakelist(\nrange(\nseq(\nlen(\nappend(\nranv(\nsort(\napply(\nF; real\nexact(\napprox(\nfloor(\nceil(\nround(\nsign(\nmax(\nmin(\nF> prog\n;\n:\n\\\n&\n?\n!\ndebug(\npython(\nF? geo\npoint(\nline(\nsegment(\ncircle(\ntriangle(\nplane(\nsphere(\nsingle_inter(\nF@ color\ncolor=\nred\ncyan\ngreen\nblue\nmagenta\nyellow\nlegend(";
	    } // else not geometry
	    const char * ptr=console_menu(key,(char*)(le_menu.c_str()),2);
	    if (!ptr){
	      show_status(text,search,replace);
	      continue;
	    }
	    adds=ptr;
	  }
	  else
#endif
	    adds=keytostring(key,keyflag,text->python,contextptr);
	  if (key!=KEY_CHAR_ANS && adds){
	    bool isex=adds[0]=='\n';
	    if (isex)
	      ++adds;
	    bool isif=strcmp(adds,"if ")==0,
	      iselse=strcmp(adds,"else ")==0,
	      isfor=strcmp(adds,"for ")==0,
	      isforin=strcmp(adds,"for in")==0,
	      isdef=strcmp(adds,"f(x):=")==0 || strcmp(adds,"def")==0,
	      iswhile=strcmp(adds,"while ")==0,
	      islist=strcmp(adds,"list ")==0,
	      ismat=strcmp(adds,"matrix ")==0;
	    if (islist){
	      input_matrix(true,contextptr);
	      continue;
	    }
	    if (ismat){
	      input_matrix(false,contextptr);
	      continue;
	    }
	    if (text->python>0){
	      if (isif)
		adds=isex?"if x<0:\nx=-x":"if :\n";
	      if (iselse)
		adds="else:\n";
	      if (isfor)
		adds=isex?"for j in range(10):\nprint(j*j)":"for  in range():\n";
	      if (isforin)
		adds=isex?"for j in [1,4,9,16]:\nprint(j)":"for  in :\n";
	      if (iswhile && isex)
		adds="a,b=25,15\nwhile b!=0:\na,b=b,a%b";
	      if (isdef)
		adds=isex?"def f(x):\nreturn x*x*x\n":"def f(x):\n\nreturn\n";
	    } else {
	      if (isif)
		adds=(lang==1 && text->python==0)?(isex?"si x<0 alors x:=-x; fsi;":"si  alors\n\nsinon\n\nfsi;"):(isex?"if (x<0) { x:=-x;}":"if (){\n\n} else {\n\n}");
	      if (lang==1 && text->python==0 && iselse)
		adds="sinon ";
	      if (isfor)
		adds=(lang==1 && text->python==0)?(isex?"pour j de 1 jusque 10 faire\nprint(j*j);\nfpour;":"pour  de  jusque  faire\n\nfpour;"):(isex?"for (j=1;j<=10;j++){ \nprint(j*j);\n}":"for(;;){\n\n};");
	      if (isforin)
		adds=(lang==1 && text->python==0)?(isex?"pour j in [1,4,9,16] faire\nprint(j)\nfpour;":"pour  in  faire\n\nfpour;"):(isex?"for j in [1,4,9,16] do\nprint(j);od;":"for  in  do\n\nod;");
	      if (iswhile)
		adds=(lang==1 && text->python==0)?(isex?"a,b:=25,15;\ntantque b!=0 faire\na,b:=b,irem(a,b);\nftantque;a;":"tantque  faire\n\nftantque;"):(isex?"a,b:=25,15;\nwhile (b!=0) {\na,b=b,irem(a,b);\n}\na;":"while () {\n\n}");
	      if (isdef)
		adds=(lang==1 && text->python==0)?(isex?"fonction f(x)\nlocal j;\nj:=x*x;\nreturn j;\nffonction:;\n":"fonction f(x)\nlocal j;\n\nreturn ;\nffonction:;"):(isex?"function f(x){\nvar j;\nj=x*x;\nreturn j;\n}\n":"function f(x){\n  var j;\n\n return ;\n};");
	    }
	    insert(text,adds,key!=KEY_CTRL_PASTE); // was true, but we should not indent when pasting
	    display(text,isFirstDraw,totalTextY,scroll,textY,contextptr);
	    const string & s=v[textline].s;
	    int cx=text->cursorx,cy=text->cursory,cp=text->pos;
	    if (tooltip(cx,cy,cp,s.substr(0,cp).c_str(),contextptr)){
	      keytooltip=true;
	    }
	    if (text->pos>0 && (key==KEY_CHAR_ACCOLADES || key==KEY_CHAR_CROCHETS))
	      --text->pos;
	    show_status(text,search,replace);
	    continue;
	  }
	}
      }
      textElement * ptr=& v[textline];
      const int interligne=16;
      switch(key){
      case KEY_CTRL_DEL:
	if (clipline>=0){
	  copy_clipboard(get_selection(text,true),true);
	  // erase selection
	  clipline=-1;
	}
	else {
	  if (editable){
	    if (textpos){
	      set_undo(text);
	      std::string & s=v[textline].s;
	      int nextpos=textpos-1;
	      if (textpos==find_indentation(s)){
		for (int line=textline-1;line>=0;--line){
		  int ind=find_indentation(v[line].s);
		  if (textpos>ind){
		    nextpos=ind;
		    break;
		  }
		}
	      }
	      s.erase(s.begin()+nextpos,s.begin()+textpos);
	      textpos=nextpos;
	    }
	    else {
	      if (textline){
		set_undo(text);
		--textline;
		textpos=v[textline].s.size();
		v[textline].s += v[textline+1].s;
		v[textline].nlines += v[textline+1].nlines;
		v.erase(v.begin()+textline+1);
	      }
	    }
	  }
	  show_status(text,search,replace);
	}
	break;
      case KEY_CTRL_S:
        display(text,isFirstDraw,totalTextY,scroll,textY,contextptr);
	search=get_searchitem(replace);
	if (!search.empty()){
	  for (;;){
	    if (!move_to_word(text,search,replace,isFirstDraw,totalTextY,scroll,textY,contextptr)){
	      break;
	    }
	  }
	  show_status(text,search,replace);
	}
	continue;
      case KEY_CTRL_OK:
	if (text->gr || text->allowEXE || !text->editable) return TEXTAREA_RETURN_EXE;
	if (search.size()){
	  for (;;){
	    if (!move_to_word(text,search,replace,isFirstDraw,totalTextY,scroll,textY,contextptr))
	      break;
	  }
	  show_status(text,search,replace);
	  continue;
	}
	else {
          if (!text->OKparse) return TEXTAREA_RETURN_EXE;
	  int err=check_parse(text,v,text->python,contextptr);
	  if (err==KEY_SHUTDOWN)
	    return err;
	  if (err) // move cursor to the error line
	    textline=err-1;
	  continue;
	}
	break;
      case KEY_CTRL_EXE: 
	if (search.size()){
	  for (;;){
	    if (!move_to_word(text,search,replace,isFirstDraw,totalTextY,scroll,textY,contextptr))
	      break;
	  }
	  show_status(text,search,replace);
	  continue;
	}
	if (clipline<0 && editable){
	  set_undo(text);
	  add_indented_line(v,textline,textpos);
	  show_status(text,search,replace);
	}
	break;
      case KEY_CTRL_UNDO:
	undo(text);
	break;
      case KEY_SHIFT_LEFT:
	textpos=0;
	break;
      case KEY_SHIFT_RIGHT:
	textpos=v[textline].s.size();
	break;
      case KEY_CTRL_LEFT:
	if (editable){
	  --textpos;
	  if (textpos<0){
	    if (textline==0)
	      textpos=0;
	    else {
	      --textline;
	      show_status(text,search,replace);
	      textpos=v[textline].s.size();
	    }
	  }
	  if (textpos>=0)
	    break;
	}
      case KEY_CTRL_UP:
	if (editable){
	  if (textline>0){
	    --textline;
	    show_status(text,search,replace);
	  }
	  else {
	    textline=0;
	    textpos=0;
	  }
	} else {
	  if (scroll < 0) {
	    scroll = scroll + interligne;
	    if(scroll > 0) scroll = 0;
	  }
	}
	break;
      case KEY_CTRL_RIGHT:
	++textpos;
	if (textpos<=ptr->s.size())
	  break;
	if (textline==v.size()-1){
	  textpos=ptr->s.size();
	  break;
	}
	textpos=0;
      case KEY_CTRL_DOWN:
	if (editable){
	  if (textline<v.size()-1)
	    ++textline;
	  else {
	    textline=v.size()-1;
	    textpos=v[textline].s.size();
	  }
	  show_status(text,search,replace);
	}
	else {
	  if (textY > scrollableHeight-(showtitle ? 0 : interligne)) {
	    scroll = scroll - interligne;
	    if(scroll < -totalTextY+scrollableHeight-(showtitle ? 0 : interligne)) scroll = -totalTextY+scrollableHeight-(showtitle ? 0 : interligne);
	  }
	}
	break;
      case KEY_CTRL_PAGEDOWN:
	if (editable){
	  textline=v.size()-1;
	  textpos=v[textline].s.size();
	}
	else {
	  if (textY > scrollableHeight-(showtitle ? 0 : interligne)) {
	    scroll = scroll - scrollableHeight;
	    if(scroll < -totalTextY+scrollableHeight-(showtitle ? 0 : interligne)) scroll = -totalTextY+scrollableHeight-(showtitle ? 0 : interligne);
	  }
	}
	break;
      case KEY_CTRL_PAGEUP:
	if (editable)
	  textline=0;
	else {
	  if (scroll < 0) {
	    scroll = scroll + scrollableHeight;
	    if(scroll > 0) scroll = 0;
	  }
	}
	break;
      case KEY_SAVE:
	if (nspire_exam_mode==2)
	  continue;
	save_script(text->filename.c_str(),merge_area(v));
	text->changed=false;
	char status[256];
	sprintf(status,(lang==1)?"%s sauvegarde":"%s saved",text->filename.c_str());
	DefineStatusMessage(status, 1, 0, 0);
	DisplayStatusArea();    	    
	continue;      
      case KEY_CTRL_F1:
	if(text->allowF1) return KEY_CTRL_F1;
	break;
      case KEY_CTRL_MENU: case KEY_CTRL_F6:
	// case KEY_CHAR_ANS:
	if (!text->editable) 	return TEXTAREA_RETURN_EXIT;
	if (clipline<0 && text->editable && text->filename.size()){
	  Menu smallmenu;
	  smallmenu.numitems=12;
	  MenuItem smallmenuitems[smallmenu.numitems];
	  smallmenu.items=smallmenuitems;
	  smallmenu.height=MENUHEIGHT;
	  smallmenu.scrollbar=0;
	  //smallmenu.title = "KhiCAS";
	  smallmenuitems[0].text = (char*)((lang==1)?"Tester syntaxe":"Check syntax");
	  smallmenuitems[1].text = (char*)((lang==1)?"Sauvegarder":"Save");
	  smallmenuitems[2].text = (char*)((lang==1)?"Sauvegarder comme":"Save as");
	  if (nspire_exam_mode==2) smallmenuitems[1].text = (char*)(lang==1?"Sauvegarde desactivee":"Saving disabled");
	  if (exam_mode || nspire_exam_mode==2) smallmenuitems[2].text = (char*)"";
	  smallmenuitems[3].text = (char*)((lang==1)?"Inserer":"Insert");
	  smallmenuitems[4].text = (char*)((lang==1)?"Effacer":"Clear");
	  smallmenuitems[5].text = (char*)((lang==1)?"Chercher,remplacer":"Search, replace");
	  smallmenuitems[6].text = (char*)((lang==1)?"Aller a la ligne":"Goto line");
	  int p=python_compat(contextptr);
	  if (p<0){
	      smallmenuitems[7].text = (char*)"Syntax [QuickJS]";
	  } else  {
	    if (p&4)
	      smallmenuitems[7].text = (char*)"Syntax [MicroPython]";
	    else {
	      if (p==0)
		smallmenuitems[7].text = (char*)"Syntax [Xcas francais]";
	      if (p==1)
		smallmenuitems[7].text = (char*)"Syntax [Xcas comp Python ^=**]";
	      if (p==2)
		smallmenuitems[7].text = (char*)"Syntax [Xcas comp Python ^=xor]";
	    }
	  }
	  smallmenuitems[8].text = (char *)((lang==1)?"Changer taille caracteres":"Change fontsize");
	  smallmenuitems[9].text = (char *)aide_khicas_string;
	  smallmenuitems[10].text = (char *)((lang==1)?"A propos":"About");
	  smallmenuitems[11].text = (char*)((lang==1)?"Quitter":"Quit");
	  int sres = doMenu(&smallmenu);
	  if(sres == MENU_RETURN_SELECTION || sres==KEY_CTRL_EXE) {
	    sres=smallmenu.selection;
	    if (sres==12){
	      int res=check_leave(text);
	      if (res==2)
		continue;
	      return TEXTAREA_RETURN_EXIT;
	    }
	    if(sres >= 10) {
	      textArea text;
	      text.editable=false;
	      text.clipline=-1;
	      text.title = smallmenuitems[sres-1].text;
	      add(&text,smallmenu.selection==10?((lang==1)?shortcuts_fr_string:shortcuts_en_string):((lang==1)?apropos_fr_string:apropos_en_string));
	      if (doTextArea(&text,contextptr)==KEY_SHUTDOWN)
		return KEY_SHUTDOWN;
	      continue;
	    }
	    if (sres==9 && editable){
	      bool minimini=!v[0].minimini;
	      for (int i=0;i<v.size();++i)
          v[i].minimini=minimini;
#ifdef HP39
	      text->lineHeight=minimini?13:15;
#else
	      text->lineHeight=minimini?13:17;
#endif
	      continue;
	    }
	    if (sres==1){
	      int err=check_parse(text,v,text->python,contextptr);
	      if (err==KEY_SHUTDOWN)
		return err;
	      if (err) // move cursor to the error line
		textline=err-1;
	    } 
	    if (sres==3 && exam_mode==0 && nspire_exam_mode!=2){
	      char filename[MAX_FILENAME_SIZE+1];
	      if (get_filename(filename,".py")){
		text->filename=filename;
		sres=2;
	      }
	    }
	    if(sres == 2 && nspire_exam_mode!=2) {
	      save_script(text->filename.c_str(),merge_area(v));
	      text->changed=false;
	      char status[256];
	      sprintf(status,(lang==1)?"%s sauvegarde":"%s saved",text->filename.c_str());
	      DefineStatusMessage(status, 1, 0, 0);
	      DisplayStatusArea();    	    
	    }
	    if (sres==4){
	      char filename[MAX_FILENAME_SIZE+1];
	      std::string ins;
	      if (giac_filebrowser(filename, "py", "Scripts",2) && load_script(filename,ins))
		insert(text,ins.c_str(),false);//add_nl(text,ins);
	    }
	    if (sres==5){
	      std::string s(merge_area(v));
#if 0
	      for (size_t i=0;i<s.size();++i){
		if (s[i]=='\n')
		  s[i]=0x1e;
	      }
	      CLIP_Store(s.c_str(),s.size()+1);
#endif
	      copy_clipboard(s,false);
	      set_undo(text);
	      v.resize(1);
	      v[0].s="";
	      textline=0;
	    }
	    if (sres==6){
	      display(text,isFirstDraw,totalTextY,scroll,textY,contextptr);
	      search=get_searchitem(replace);
	      if (!search.empty()){
		for (;;){
		  if (!move_to_word(text,search,replace,isFirstDraw,totalTextY,scroll,textY,contextptr)){
		    break;
		  }
		}
		show_status(text,search,replace);
	      }
	    }
	    if (sres==7){
	      display(text,isFirstDraw,totalTextY,scroll,textY,contextptr);
	      int l=get_line_number((lang==1)?"Negatif: en partant de la fin":"Negative: counted from the end",(lang==1)?"Numero de ligne:":"Line number:");
	      if (l>0)
		text->line=l-1;
	      if (l<0)
		text->line=v.size()+l;
	    }
	    if (sres==8){
	      int c=select_interpreter();
	      if (c>=0){
		int p=text->python;
		if (c==4)
		  p=-1;
		else {
		  if (c==3)
		    p |= 0x4;
		  else 
		    p=c;
		}
		giac::python_compat(p,contextptr);
		text->python=p;
		xcas_python_eval=(c==3?1:(c==4?-1:0));
		show_status(text,search,replace);
		warn_python(text->python,false);
    draw_editor_menu(text->gr,text->python);
	      }
	    }
	  }
	}
	break;
      case KEY_CTRL_SETUP: // inactive
	text->python=text->python?0:1; // FIXME QUICKJS
	show_status(text,search,replace);
	python_compat(text->python,contextptr);
	warn_python(text->python,false);
  draw_editor_menu(text->gr,text->python);
	continue;
      case KEY_CTRL_F2:
	if (clipline<0)
	  return KEY_CTRL_F2;
      case KEY_CTRL_EXIT:
	if (clipline>=0){
	  clipline=-1;
	  show_status(text,search,replace);
	  continue;
	}
	if (!search.empty()){
	  search="";
	  show_status(text,search,replace);
	  continue;
	}
	if (check_leave(text)==2)
	  continue;
	return TEXTAREA_RETURN_EXIT;
      case KEY_CTRL_INS:
	break;
      default:
	if (clipline<0 && key>=32 && key<128 && editable){
	  char buf[2]={char(key),0};
	  insert(text,buf,false);
	  show_status(text,search,replace);
	}
	if (key==KEY_CTRL_AC){
	  if (clipline>=0){
	    clipline=-1;
	    show_status(text,search,replace);
	  }
	  else {
	    if (search.size()){
	      search="";
	      show_status(text,search,replace);
	    }
	    else {
	      copy_clipboard(v[textline].s+'\n',true);
	      if (v.size()==1)
		v[0].s="";
	      else {
		v.erase(v.begin()+textline);
		if (textline>=v.size())
		  --textline;
	      }
	      DefineStatusMessage((char*)"Line cut and copied to clipboard", 1, 0, 0);
	      DisplayStatusArea();
	    }
	  }
	}
      }
    }
  }

  void reload_edptr(const char * filename,textArea *edptr,GIAC_CONTEXT){
    if (edptr){
      std::string s(merge_area(edptr->elements));
      copy_clipboard(s,true);
      s="\n";
      edptr->elements.clear();
      edptr->clipline=-1;
      edptr->filename=remove_path(giac::remove_extension(filename))+".py";
      load_script((char *)edptr->filename.c_str(),s);
      if (s.empty())
	s="\n";
      // cout << "script " << edptr->filename << endl;
      edptr->editable=true;
      edptr->changed=false;
      edptr->python=python_compat(contextptr);
      edptr->elements.clear();
#ifdef HP39
      edptr->y=12;
#else
      edptr->y=0; // 7;
#endif
      add(edptr,s);
      edptr->line=0;
      edptr->pos=0;
    }
  }  

  console_line * Line=0;//[_LINE_MAX];//={data_line};
  char menu_f1[8]={0},menu_f2[8]={0},menu_f3[8]={0},menu_f4[8]={0},menu_f5[8]={0},menu_f6[8];
  char session_filename[MAX_FILENAME_SIZE+1]="session";
  char * FMenu_entries_name[6]={menu_f1,menu_f2,menu_f3,menu_f4,menu_f5,menu_f6};
  location Cursor;
  char *Edit_Line=0;
  int Start_Line, Last_Line,editline_cursor;
  int Case;
  int console_changed=0; // 1 if something new in history
  int dconsole_mode=1; // 0 disables dConsole commands

#define Current_Line (Start_Line + Cursor.y)
#define Current_Col (Line[Cursor.y + Start_Line].start_col + Cursor.x)

  void console_disp_status(GIAC_CONTEXT){
    int i=python_compat(contextptr);
    string msg("shell ");
    if (i<0)
      msg += "QuickJS";
    else {
      if (i&4)
	msg+="MicroPython";
      else {
	if (i==0)
	  msg+="Xcas";
	else {
	  if (i==1)
	    msg+="Py ^=**";
	  else
	    msg+="Py ^=xor";
	}
      }
    }
    if (angle_radian(contextptr))
      msg += " RAD ";
    else
      msg += " DEG ";
    msg += session_filename;
    if (console_changed)
      msg += " *";
    statuslinemsg(msg.c_str());
    set_xcas_status();
    Bdisp_PutDisp_DD();
  }

  void leave_exam_mode(GIAC_CONTEXT){
#ifdef NSPIRE_NEWLIB
    // FIXME test USB connection instead
    unsigned NSPIRE_RTC_ADDR=0x90090000;
    unsigned t1= * (volatile unsigned *) NSPIRE_RTC_ADDR;
    int chk=0;
    if (exam_duration<=0 || (t1-exam_start<exam_duration)){
      chk=-1;
#if 1 // checkin the  power management addresses range
      unsigned poweraddr=0x900b0028;
      unsigned u=*(unsigned *)poweraddr;
      //*logptr(contextptr) << "power " << u << '\n';
      if ( is_cx2 || (u&0xff0000)==0x070000) // connected 0x11070114, disconnected 0x11110114
	chk=0;
#endif
#if 0 /// check connection, works only if graph link connection before
      unsigned powermanagement_lockaddr=0x900b0018;
      // Bit 5: #B0000000 - USB OTG controller
      // Bit 6: #B4000000 - USB HOST controller
      *(unsigned *)powermanagement_lockaddr=0x8400a5d;
      unsigned HW_USBCTRL_PORTSC1=0xb0000184;
      unsigned u=*(unsigned *) HW_USBCTRL_PORTSC1;
      if ( (u&0xff000000)==0x11000000) // 0x11000805 vs 0x1d000004
	chk=0;
      // B00001A4 might be used as well: HW_USBCTRL_OTGSC 1f202d20 vs 1f3c1120
#endif
#if 0 // check USB does not work
      nn_ch_t ch = NULL;
      nn_oh_t oh = NULL;
      nn_nh_t nh = NULL;
      oh = TI_NN_CreateOperationHandle();
      int ans=TI_NN_NodeEnumInit((nn_ch_t) oh);//(ch);
      *logptr(contextptr) << "enuminit" << ans << '\n';
      if (ans>=0){
	ans=TI_NN_NodeEnumNext(oh, &nh);
	*logptr(contextptr) << "enumnext" << ans << '\n';
	if (ans>=0){
	  ans=TI_NN_Connect(nh, 0x4060, &ch);
	  *logptr(contextptr) << "connect" << ans << '\n';
	  if (ans>=0){
	    if(ch){
	      TI_NN_Disconnect(ch);
	      chk=0;
	    }
	  }
	}
	TI_NN_NodeEnumDone(oh);
	TI_NN_DestroyOperationHandle(oh);
      }
#endif
    }
#else
    int chk=0;
#endif
    if (chk>=0){
      set_exam_mode(0,contextptr);
    }
    if (exam_mode)
      confirm((lang==1)?"Pour arreter le mode examen":"To stop exam mode",(lang==1)?"branchez la calculatrice puis doc doc":"plug in the calculator then doc doc");
    else
      confirm((lang==1)?"Fin du mode examen":"End exam mode","enter: OK");
  }    

#ifdef NUMWORKS
/* begin section (c) B. Parisse, write exam mode in *all* firmwares 
   code is dual-licensed for use in Epsilon and here
 */
// write exam mode mode to flash at offset pos/8
  void write_exammode(unsigned char * ptr,int pos,int mode,int modulo){
    int curmode=pos%modulo;
    if (curmode == mode)
      return;
    int delta=(mode+modulo-curmode)%modulo; // 0<delta<modulo, number of bits that we will set to 0
    unsigned char * target = ptr + pos/8;
    int pos8=pos % 8; // number of bits already set to 0 at target
    pos8 += delta; // 0<pos8<modulo+7
    unsigned char postab[]={0b11111111,0b1111111,0b111111,0b11111,0b1111,0b111,0b11,0b1,0}; // postab[i] is a byte with i bits set to 0 
    unsigned char tab[2];
    bool writenext=pos8>8;
    tab[0]=postab[writenext?8:pos8];
    tab[1]=postab[writenext?pos8-8:0];
#if 1 // KHICAS
    WriteMemory((char *)target, (const char *)tab, writenext?2:1);
#else
    target[0]=tab[0];
    if (writenext) target[1]=tab[1];
#endif
  }

  // check that region is split in two parts: bits set to 0 followed by bits set to 1, returns position of the first 1 bit or -1
  int pos_01(unsigned * start,int l){
    unsigned *ptr=start,* end=start+l/sizeof(unsigned);
    for (;ptr<end;++ptr){
      if (*ptr)
	break;
    }
    if (ptr==end)
      return -1;
    int pos=(ptr-start)*32;
    unsigned char * ptr8=(unsigned char *) ptr;
    if (*ptr8==0){
      pos+=8; ++ptr8;
    }
    if (*ptr8==0){
      pos+=8; ++ptr8;
    }
    if (*ptr8==0){
      pos+=8; ++ptr8;
    }
    switch (*ptr8){
    case 0b1: pos+=7; break; // 7 bits are already set to 0
    case 0b11: pos+=6; break;
    case 0b111: pos+=5; break;
    case 0b1111: pos+=4; break;
    case 0b11111: pos+=3; break;
    case 0b111111: pos+=2; break;
    case 0b1111111: pos+=1; break;
    case 0b11111111: break;
    default: return -1;
    }
    ++ptr;
    for (;ptr<end;++ptr){
      if (*ptr!=0xffffffff)
	return -1;
    }
    return pos;
  }

  char print_hex(int val){
    val &= 0xf;
    if (val>=0 && val<=9)
      return '0'+val;
    return 'A'+(val-10);
  }

  void print_hex(unsigned val,char * ptr){
    for (int i=0;i<8;++i){
      ptr[i]=print_hex(val>>(28-4*i));
    }
  }

  void set_exammode(int mode,int modulo){
    if (mode<0 || mode>=modulo)
      return;
    // scan external flash every dflash bytes
    unsigned * flashptr=(unsigned *) 0x90000000;
    unsigned dflash=0x10000,ntests=8*1024*1024/dflash/sizeof(unsigned);
    for (unsigned i=0;i<ntests;++i){
      unsigned * ptr=flashptr+i*dflash;
      // kernel header?
      if (*ptr!=0xffffffff || *(ptr+1)!=0xffffffff || *(ptr+2)!= 0xdec00df0 /* f0 0d c0 de*/)
	continue;
      ptr += 0x1000/sizeof(unsigned); // exam mode buffer?
      int pos=pos_01(ptr,0x1000);
      if (pos==-1)
	continue;
      if (pos>=0x1000-8){
	erase_sector((const char *)ptr);
	pos=0;
      }
      write_exammode((unsigned char *)ptr,pos,mode,modulo);
    }
  }

  const size_t baseaddr[]={0x90000000,0x90180000,0x90400000};
  void boot_firmware(int slot){
    const int taille=int(sizeof(baseaddr)/sizeof(size_t));
    if (slot<0 || slot>taille)
      slot=taille;
    unsigned * ptr=(unsigned *) 0x08004000;
    int pos=pos_01(ptr,0x1000);
    if (pos==-1)
      return;
    if (pos>=0x8000-8){
      erase_sector((const char *)ptr);
      pos=0;
    }
    write_exammode((unsigned char *)ptr,pos,slot,4);
    wait_1ms(10); // Ion::Timing::msleep(10);
    erase_sector(0); // Ion::Device::Reset::core();
  }

  const size_t kernelheader=0xdec00df0;
  const size_t userheader=0xDEC0EDFE;
  const size_t omegaheader=0xEFBEADDE;
  const size_t upsilonheader=0x55707369;

  // return 0 if invalid, 1 for Khi, 2 for Omega, 3 for Upsilon, 4 for Epsilon<=18.2.3
  int is_valid(int slot){ 
    if (slot<0 || slot>=int(sizeof(baseaddr)/sizeof(size_t)))
      return 0;
    // kernel header
    size_t * addr=(size_t *) baseaddr[slot];
    if (addr[0]!=0xffffffff || addr[1]!=0xffffffff || addr[2]!=kernelheader || addr[7]!=kernelheader)
      return 0;
    // userland header
    addr=(size_t *) (baseaddr[slot]+0x10000);
    if (addr[0]!=userheader || addr[9]!=userheader)
      return 0;
    if (addr[10]==omegaheader){
      char * version=(char *)&addr[11];
      if (version[1]=='.')
	return 2; // Omega
      return 1; // Khi
    }
    if (addr[10]==upsilonheader)
      return 3;
    // epsilon version
    char * version=(char *)&addr[1];
    if (strcmp(version,"18.2.3")>0)
      return 0;
    return 4;
  }

  size_t find_storage(){
    // find storage
    int slot=0;
    size_t addr[]={0x90000000,0x90180000,0x90400000};
    for (;slot<sizeof(addr)/sizeof(size_t);++slot){
      unsigned char * r=(unsigned char *) addr[slot];
      bool externalinfo=r[8]==0xf0 && r[9]==0x0d && r[10]==0xc0 && r[11]==0xde;
      if (!externalinfo)
	continue;
      r += 0x10000;
      if (r[15]!=0x20) // ram is at 0x20000000 (+256K)
	continue;
      size_t start=((r[15]*256U+r[14])*256+r[13])*256+r[12];
      r=(unsigned char *)start;
      if (r[0]==0xba && r[1]==0xdd && r[2]==0x0b && r[3]==0xee){ // ba dd 0b ee begin of scriptstore
	return start;
      }
    }
    return 0;
  }    

  bool save_backup(int no){
    if (no!=1 && no!=0)
      return false;
    int L=32*1024;
    size_t backupaddr=0x90800000-2*L;
    size_t storageaddr=find_storage();
    //confirm("Storage @",hexa_print_INT_(storageaddr).c_str());
    if (storageaddr){
      char * sptr=(char *)storageaddr;
      int L=32*1024;
      if (no==0){
	// check if sector is already formatted
	unsigned * ptr=(unsigned *) (backupaddr);
	for (int i=0;i<L/2;++i){
	  if (ptr[i]!=0xffffffff){ // it's not, format
	    erase_sector((const char *)backupaddr);
	    break;
	  }
	}
	if (sptr[4]==0 && sptr[5]==0){
	  return false; // nothing to save 
	}
	WriteMemory((char *)backupaddr,(const char *)storageaddr,L);
	return true;
      }
      if (no==1){
	if (sptr[4]==0 && sptr[5]==0){
	  return false; // nothing to save 
	}
	// check if sector is already formatted
	unsigned * ptr=(unsigned *) (backupaddr+L);
	for (int i=0;i<L/4;++i){
	  if (ptr[i]!=0xffffffff){ // it's not, format
	    char * buf=(char *)malloc(L);
	    memcpy(buf,(char *)backupaddr,L);
	    erase_sector((const char *)backupaddr);
	    WriteMemory((char *)backupaddr,buf,L);
	    free(buf);
	    break;
	  }
	}
	WriteMemory((char *)(backupaddr+L),(const char *)storageaddr,L);
	return true;
      } 
      return true; // never reached
    }
    return false;
  }

  // 0 or 1 restore, 2 or 3 check if there is something to restore
  bool restore_backup(int no){
    if (no<0)
      return false;
    int L=32*1024;
    size_t backupaddr=0x90800000-2*L;
    size_t storageaddr=find_storage();
    //confirm("Storage @",hexa_print_INT_(storageaddr).c_str());
    if (!storageaddr)
      return false;
    char * sptr=(char *)(backupaddr+(no & 1)*L);
    // ba dd 0b ee
    if (sptr[0]!=0xba || sptr[1]!=0xdd || sptr[2]!=0x0b || sptr[3]!=0xee )
      return false;
    if (sptr[4]==0 && sptr[5]==0)
      return false; // nothing to restore
    if (no>=2)
      return true;
    memcpy((char *)storageaddr,sptr,L);
    return true;
  }
  /* end section (c) B. Parisse, */
#endif
  
  void menu_setup(GIAC_CONTEXT){
    Menu smallmenu;
    smallmenu.numitems=15;
    MenuItem smallmenuitems[smallmenu.numitems];
    smallmenu.items=smallmenuitems;
    smallmenu.height=MENUHEIGHT;
    smallmenu.scrollbar=1;
    smallmenu.scrollout=1;
    smallmenu.title = (char *)"Config";
  
#ifdef NUMWORKS
#ifdef DEVICE
    string titles="Config (Memory "+print_INT_(_heap_size)+")";
    smallmenu.title = (char*) titles.c_str();
#endif
    smallmenuitems[0].type = MENUITEM_CHECKBOX;
    smallmenuitems[0].text = (char*)"x,n,t -> t";
#endif
    smallmenuitems[1].text = (char*)"Syntaxe (Xcas/Py/JS)";
    smallmenuitems[2].type = MENUITEM_CHECKBOX;
    smallmenuitems[2].text = (char*)"Radians (in Xcas)";
    smallmenuitems[3].type = MENUITEM_CHECKBOX;
    smallmenuitems[3].text = (char*)"Sqrt (in Xcas)";
    smallmenuitems[4].text = (char*)"Francais";
    smallmenuitems[5].text = (char*)"English";
    smallmenuitems[6].text = (char*)"Spanish&English";
    smallmenuitems[7].text = (char*)"Greek&English";
    smallmenuitems[8].text = (char*)"Deutsch&English";
    smallmenuitems[9].text = (char *) ((lang==1)?"Raccourcis clavier (0)":"Shortcuts (0)");
#ifdef NUMWORKS
    smallmenuitems[10].text = (char*) ((lang==1)?"Backup, mode examen (e^x)":"Backup, exam mode (e^x)");
#else
    smallmenuitems[10].text = (char*) ((lang==1)?"Mode examen (e^x)":"Exam mode (e^x)");
    if (osok==0)
      smallmenuitems[10].text = (char*) ((lang==1)?"Incompatible mode examen":"Exam mode incompatible");
    if (osok==-1)
      smallmenuitems[10].text = (char*) ((lang==1)?"Avertissement mode examen":"Exam mode warning");
#endif
    smallmenuitems[11].text = (char*) ((lang==1)?"A propos":"About");
    smallmenuitems[14].text = (char*) "Quit";
    if (exam_mode)
      smallmenuitems[14].text = (char*)((lang==1)?"Quitter le mode examen":"Quit exam mode");
    
    // smallmenuitems[2].text = (char*)(isRecording ? "Stop Recording" : "Record Script");
    while(1) {
#ifdef NUMWORKS
      smallmenuitems[0].value = xthetat;
#else
      string dig("Digits (in Xcas): ");
      dig += print_INT_(decimal_digits(contextptr));
      smallmenuitems[0].text = (char*)dig.c_str();
#endif
      string heaps("Micropy/JS heap "+print_INT_(pythonjs_heap_size/1024)+"K");
      smallmenuitems[12].text = (char *) heaps.c_str();
      string stacks("-------------");
      // string stacks("Micropython stack "+print_INT_(pythonjs_stack_size/1024)+"K"); // enable in micropython mpconfig.h + call to pystack_init + remove continue below
      smallmenuitems[13].text = (char *) stacks.c_str();
      int p=python_compat(contextptr);
      if (p<0){
	smallmenuitems[1].text = (char*)"Change syntax (QuickJS)";
      } else {
	if (p&4)
	  smallmenuitems[1].text = (char*)"Change syntax (MicroPython)";
	else {
	  if (p==0)
	    smallmenuitems[1].text = (char*)"Change syntax (Xcas)";
	  if (p==1)
	    smallmenuitems[1].text = (char*)"Change syntax (Xcas comp Python ^=**)";
	  if (p==2)
	    smallmenuitems[1].text = (char*)"Change syntax (Xcas comp Python ^=xor)";
	}
      }
      smallmenuitems[2].value = giac::angle_radian(contextptr);
      smallmenuitems[3].value = giac::withsqrt(contextptr);
      int sres = doMenu(&smallmenu);
      if (sres==MENU_RETURN_EXIT)
	break;
      if (sres == MENU_RETURN_SELECTION  || sres==KEY_CTRL_EXE) {
	if (smallmenu.selection == 1){
#ifdef NUMWORKS	 
	  xthetat=1-xthetat;
#else
	  double d=decimal_digits(contextptr);
	  if (inputdouble("Nombre de digits?",d,contextptr) && d==int(d) && d>0){
	    decimal_digits(d,contextptr);
	  }
#endif
	  continue;
	}
#if defined MICROPY_LIB || defined QUICKJS
	if (smallmenu.selection == 2){
	  int c=select_interpreter();
	  if (c>=0){
	    int p=giac::python_compat(contextptr);
	    if (c==4){
	      p=-1;
	    } else {
	      if (c==3)
		p |= 0x4;
	      else
		p=c;
	    }
	    int old_xcas_python_eval=xcas_python_eval;
	    xcas_python_eval=c<0?c:(c==3?1:0);
	    giac::python_compat(p<0?0:p,contextptr);
	    if (edptr)
	      edptr->python=p;
	    if (xcas_python_eval!=old_xcas_python_eval){
	      if (old_xcas_python_eval==0 && xcas_python_eval>0 &&
		  do_confirm((lang==1)?"Effacer les variables Xcas?":"Clear Xcas variables?"))
		do_restart(contextptr);
	    }
#ifdef MICROPY_LIB
	    if (old_xcas_python_eval==1 && do_confirm((lang==1)?"Effacer le tas MicroPython?":"Clear MicroPython heap?"))
	      python_free();
#endif
#ifdef QUICKKS
	    if (0 && old_xcas_python_eval==-1 && do_confirm((lang==1)?"Effacer le tas QuickJS?":"Clear QuickJS heap?"))
	      js_end(global_js_context);
#endif
	    warn_python(p,false);
	    Console_FMenu_Init(contextptr);
	    console_disp_status(contextptr);
	    break;
	  }
	}
#endif
	if (smallmenu.selection == 3){
	  giac::angle_radian(!giac::angle_radian(contextptr),contextptr);
	  os_set_angle_unit(giac::angle_radian(contextptr)?0:1);
	  statusline(2*xcas_python_eval);
	  continue;
	}
	if (smallmenu.selection == 4){
	  giac::withsqrt(!giac::withsqrt(contextptr),contextptr);
	  continue;
	}
	if (smallmenu.selection>=5 && smallmenu.selection<=9){
	  lang=smallmenu.selection-4;
	  giac::language(lang,contextptr);
	  break;
	}
	if (smallmenu.selection==11 && osok==-1){
	  confirm(lang==1?"Activez une fois le mode examen TI":"Activate one time TI exam mode",lang==1?"pour utiliser ensuite celui de KhiCAS":"to enable KhiCAS exam mode");
	  continue;
	}
	if (smallmenu.selection==11 && osok==0){
	  confirm(lang==1?"Ce modele n'est pas compatible":"This model is not compatible",lang==1?"avec le mode examen de KhiCAS":"with KhiCAS exam mode");
	  continue;
	}
	if (smallmenu.selection == 11 && osok>0){
#ifdef NSPIRE_NEWLIB
	  if (nspire_exam_mode==1
	      // && !is_cx2
	      ){
	    if (confirm((lang==1?"Pour relancer le mode examen, il faudra":"To re-enter exam mode, you'll have"),(lang==1?"quitter Xcas. enter OK, esc annul":"to quit Xcas. enter OK, esc cancel."))!=KEY_CTRL_F1)
	      break;
	    do_restart(contextptr);
	    clear_turtle_history(contextptr);
	    Console_Init(contextptr);
	    Console_Clear_EditLine();
	    console_changed=0;
	    *logptr(contextptr) << (lang==1?"Nettoyage du repertoire du mode examen\n":"Cleaning exam mode directory.\n");
	    *logptr(contextptr) << (lang==1?"Patientez environ 30 secondes\n":"Please wait about 30 secondes\n");
	    // nspire_clear_data(contextptr,false); // done by set_exam_mode
	    nspire_exam_mode=2;
	    set_exam_mode(0,contextptr);
	    *logptr(contextptr) << (lang==1?"Leds eteintes. Mode restreint (pas d'acces aux fichiers).\n":"Leds off. Restricted mode (no file access).\n");
	    *logptr(contextptr) << (lang==1?"Quittez KhiCAS (doc doc) pour rebooter le mode examen\n":"Leave KhiCAS (doc doc) to reboot in exam mode.\n");
	    break;
	  }
	  else {
	    if (osok>0 ||
		!is_cx2){
	      if (do_confirm((lang==1)?"Lancer le mode examen avec CAS ?":"Run exam mode with CAS?")){
		*logptr(contextptr) << (lang==1?"Patientez environ 2 minutes\n":"Please wait about 2 minutes\n");
		*logptr(contextptr) << (lang==1?"Verifie et recopie les ressources necessaires\n":"Check and copy required ressources\n");
		rm("/exammode/usr/ndless");
		nspire_clear_data(contextptr,true);
		*logptr(contextptr) << (lang==1?"Verification finale puis reboot en mode examen\n":"Final check then reboot in exam mode\n");
		*logptr(contextptr) << (lang==1?"Patientez 30 secondes environ\n":"Please wait about 30 secondes\n");
		set_exam_mode(-1,contextptr); // end up with reset()
	      }
	      break;
	    }
	    else {
	      confirm((lang==1)?"Desole. Le mode examen de KhiCAS":"Sorry, KhiCAS exam mode",(lang==1)?"ne fonctionne pas sur Nspire CX II":"is not supported on Nspire CX II");
	      break;
	    }
	    if (1
		|| is_cx2
		){
	      textArea text;
	      text.editable=false;
	      text.clipline=-1;
	      text.title = lang==1?"KhiCAS et mode examen":"KhiCAS and exam mode";
	      add(&text,(lang==1)?
		  "Attention, verifiez que le calcul formel est autorise avant d'utiliser KhiCAS en mode examen. En France, c'est en principe autorise lorsque la calculatrice graphique l'est (par exemple au bac)":
		  "Warning! Check that CAS is allowed before running KhiCAS in exam mode.");
	      const char exam_mode_fr_string[]="Pour utiliser KhiCAS en mode examen, il faut effectuer une preparation chez soi quelques heures avant avec une connection PC ou quelques minutes avant l'examen avec un autre etudiant ayant une Nspire CX ou CX II.\nLancer le mode examen sur la calculatrice cible (esc-on), recopier ndless et khicas.tns (ou luagiac.luax.tns et khicaslua.tns) sur la calculatrice cible en mode examen. Avec 2 calculatrices, recommencez sur l'autre calculatrice (mettre l'autre calculatrice en mode examen et copiez dessus ndless et khicas).\nActiver ndless (cable debranche) puis lancez KhiCAS puis touche calculatrice (en-dessous de esc) puis selectionner l'item 11. mode examen, valider : ceci va effacer les donnees et desactiver le clignotement des leds.\n\nAu debut de l'examen, lorsque le surveillant demande d'activer le mode examen, quittez KhiCAS en tapant doc doc (ou appuyez sur reset), le mode examen sera a nouveau actif et les leds clignoteront. Vous pouvez activer ndless et lancez KhiCAS.\nPour les institutions n'acceptant pas KhiCAS en mode examen: demandez a vos etudiants de redemarrer la calculatrice, puis faire esc-on et reinitialiser le mode examen.";
	      const char exam_mode_en_string[]="Running KhiCAS in exam mode requires preparation at home with a PC or a few minutes with another student having a Nspire CX/CXII.\nActivate exam mode on the target calculator (esc-on), connect the PC or the other calculator, copy ndless and khicas.tns (or luagiac.luax.tns and khicaslua.tns) to the target calc (kept in exam mode). With 2 calculators, repeat on the other calculator.\n Activate ndless (disconnect the link) and run KhiCAS. Type the calculator key below esc then select 11. Exam mode. This will desactivate leds blinking and clear data. When exam begins, quit KhiCAS (doc doc) or press reset, exam mode will be active again and leds will blink. Activate ndless and run KhiCAS.\n\nFor institutions who do not want to allow KhiCAS, ask your students to reset their calculator, press esc-on and restart exam mode, this will clear ndless and KhiCAS.";
	      add(&text,(lang==1)?exam_mode_fr_string:exam_mode_en_string);
	      if (doTextArea(&text,contextptr)==KEY_SHUTDOWN)
		return ;
	      break;
	    }
	  }
#endif // NSPIRE_NEWLIB
#ifdef NUMWORKS
#ifdef DEVICE
	  const char * tab[]={lang==1?"Sauvegarde multi-firmwares":"Backup for multi-firmware",lang==1?"Restauration multifirmwares":"Restore multi-firmware backup",lang==1?"Lancer le mode examen":"Run exam mode",lang==1?"Backup du mode examen":"Restore exam mode backup",0};
	  int choix=select_item(tab,"Mode examen",true);
	  if (choix<0 || choix>4)
	    break;
	  if (choix==0){
	    if (!save_backup(1))
	      confirm(lang==1?"Rien a sauvegarder":"Nothing to save","OK?");
	    break;
	  }
	  if (choix==1){
	    if (restore_backup(3)){
	      if (confirm(lang==1?"Les donnees actuelles vont etre effacees":"Current data will be erased!","OK/Back?")==KEY_CTRL_F1){
		confirm(restore_backup(1)?"Success!":"Failure!","OK?");
	      }
	    }
	    else
	      confirm(lang==1?"Pas de donnees":"No data.","OK/Back?");	      
	    break;
	  }
	  if (choix==2){
	    if (confirm(lang==1?"Sauvegarde, effacement puis mode examen":"Backup, clear then exam mode","OK/Back?")==KEY_CTRL_F1){
	      if (!save_backup(0))
		confirm(lang==1?"Rien a sauvegarder":"Nothing to save","OK?");
	      // save to exam mode backup sector and format reset backup sector
	      set_exammode(1,4);
	      erase_sector(0); // means reset
	    }
	  }
	  if (choix==3){
	    if (inexammode()){ 
	      confirm(lang==1?"Sortez d'abord du mode examen!":"Leave exam mode first!",lang==1?"(Connectez la calculatrice)":"(Connect calculator)");
	      break;
	    }
	    confirm(restore_backup(0)?"Success!":"Failure!","OK?");
	    break;
	  }
#endif
	  // if (do_confirm(lang==1?"Le mode examen se lance depuis Parametres":"Enter Exam mode from Settings")) shutdown_state=1;
	  break;
#else
	  if (!exam_mode && confirm((lang==1?"Verifiez que le calcul formel est autorise.":"Please check that the CAS is allowed."),(lang==1?"France: autorise au bac. Enter: ok, esc: annul":"enter: yes, esc: no"))!=KEY_CTRL_F1)
	    break;
#endif
	  // confirmation, duree (>=0 French indicative, else not indicative)
	  double duration=exam_mode?absint(exam_duration):0;
	  string msg=(lang==1)?"Compte a rebours en h.min ou 0 pour horloge":"Exam duration in h.min (0: end by pluging)";
	  msg += print_duration(duration);
	  if (inputdouble(msg.c_str(),duration,contextptr)){
	    bool indicative=lang==1?duration>=0:duration<=0;
	    if (exam_mode)
	      indicative=exam_duration<=0;
	    else {
	      if (lang==1 && !indicative && confirm("Attention, mode non conforme au bac en France","enter: corriger, esc: tant pis")!=KEY_CTRL_F6)
		indicative=true;
	    }
	    if (duration<0)
	      duration=-duration;
	    if (duration>10)
	      duration=duration/60;
	    else
	      duration=std::floor(duration)+100.0/60*(duration-std::floor(duration));
	    if (duration){
	      msg=lang==1?"Duree compte a rebours ":"Exam duration ";
	      double d=giacmax(duration*3600,absint(exam_duration));
	      msg += print_duration(d);
	    }
	    else
	      msg="Mode examen.";
	    if (indicative)
	      msg += lang==1?" Fin par branchement":" Exit by pluging";
	    if (confirm(msg.c_str(),(lang==1?"!Blocage dans Xcas en mode exam! enter OK, esc annul":"!Trapped in Xcas in exam mode! enter OK, esc cancel."))==KEY_CTRL_F1){
#ifdef NSPIRE_NEWLIB
	      if (exam_mode) 
		exam_duration=duration?giacmax(absint(exam_duration),duration*3600+30):0;
	      else {
		unsigned NSPIRE_RTC_ADDR=0x90090000;
		exam_start= * (volatile unsigned *) NSPIRE_RTC_ADDR;
		exam_duration = duration?duration*3600+30:0;
	      }
	      if (indicative)
		exam_duration=-absint(exam_duration);
#else
	      exam_start=0;
	      exam_duration=1;
#endif
	      do_restart(contextptr);
	      clear_turtle_history(contextptr);
	      Console_Init(contextptr);
	      Console_Clear_EditLine();
	      set_exam_mode(1,contextptr);
	      strcpy(session_filename,"session.xw");
	      console_changed=0;
	      save_session(contextptr);
	      if (edptr){
		edptr->elements.resize(1);
		edptr->elements[0].s="";
		edptr->undoelements=edptr->elements;
		edptr->line=0;
		edptr->pos=0;
	      }
	      save_script("session.py","");
	    }
	  }
	  break;
	}
#if (defined MICROPY_LIB || defined QUICKJS) 
	if (smallmenu.selection==13){
	  double d=pythonjs_heap_size/1024;
	  if (
#ifdef NSPIRE_NEWLIB
	      0 &&
#endif
	      inputdouble(
#if defined NUMWORKS && defined DEVICE
			  string("Tas MicroPy/JS en K (16-"+print_INT_(_heap_size/1024-52)+")?").c_str()
#else
			  "Tas MicroPy/JS en K (64-1728)?"
#endif
			  ,d,contextptr) && d==int(d) &&
#if defined NUMWORKS && defined DEVICE
	      d>=16 && d<=_heap_size/1024-52
#else
	      d>=64 && d<=1728
#endif
	      ){
	    pythonjs_heap_size=d*1024;
#ifdef MICROPY_LIB
	    python_free();
#endif
#ifdef QUICKJS
	    js_end(global_js_context);
#endif
	  }
	  continue;
	}
	if (smallmenu.selection==14){
	  continue;
	  double d=pythonjs_stack_size/1024;
	  if (inputdouble(
#if defined NUMWORKS && defined DEVICE
			  "Pile MicroPy/JS en K (8-20)?"
#else
			  "Pile MicroPy/JS en K (32-512)?"
#endif
			  ,d,contextptr) && d==int(d) &&
#if defined NUMWORKS && defined DEVICE
	      d>=8 && d<=20
#else
	      d>=32 && d<=512
#endif
	      ){
	    pythonjs_stack_size=d*1024;
#ifdef MICROPY_LIB
	    python_free();
#endif
#ifdef QUICKJS
	    js_end(global_js_context);
#endif
	  }
	  continue;
	}
#endif // MICROPY_LIB
	if (smallmenu.selection == 15){
	  if (exam_mode)
	    leave_exam_mode(contextptr);
	  break;
	}
	if (smallmenu.selection >= 10) {
	  textArea text;
	  text.editable=false;
	  text.clipline=-1;
	  text.title = smallmenuitems[smallmenu.selection-1].text;
	  add(&text,smallmenu.selection==10?((lang==1)?shortcuts_fr_string:shortcuts_en_string):((lang==1)?apropos_fr_string:apropos_en_string));
	  if (doTextArea(&text,contextptr)==KEY_SHUTDOWN)
	    return ;
	  continue;
	} 
      }	
    }      
  }

  void * console_malloc(unsigned s){
    return new char [s];
    // return malloc(s);
  }

  void console_free(void * ptr){
    delete [] (char *) ptr;
    // free(ptr);
  }

  void cleanup(std::string & s){
    for (size_t i=0;i<s.size();++i){
      if (s[i]=='\n')
	s[i]=' ';
    }
  }

  const int max_lines_saved=50;

  int run(const char * s,int do_logo_graph_eqw,GIAC_CONTEXT){
    if (strlen(s)>=2 && (s[0]=='#' ||
			 (s[0]=='/' && (s[1]=='/' || s[1]=='*'))
			 ))
      return 0;
    if (strcmp(s,"caseval(\"\")")==0 || strcmp(s,"eval_expr(\"\")")==0 || (strlen(s)>=4 && strlen(s)<6 && strncmp(s,"xcas",4)==0)){
      xcas_python_eval=0;
      int p=python_compat(contextptr);
      python_compat(p>0?p&3:0,contextptr);
      if (edptr)
	edptr->python=p>0?p&3:0;
#ifdef MICROPY_LIB
      if (p==4 && do_confirm((lang==1)?"Effacer le tas MicroPython?":"Clear MicroPython heap?"))
	python_free();
#endif
#ifdef QUICKJS
      if (0 && p==-1 && do_confirm((lang==1)?"Effacer le tas QuickJS?":"Clear QuickJS heap?"))
	js_end(global_js_context);
#endif
      *logptr(contextptr) << "Xcas interpreter\n";
      Console_FMenu_Init(contextptr);
      return 0;
    }
    gen g,ge;
#ifdef QUICKJS
    if (strlen(s)>=2 && strlen(s)<4 && strncmp(s,"js",2)==0){
      switch_to_js(contextptr);
      return 0;
    }
    if (xcas_python_eval==-1){
      string S(s); 
      if (S.size() && S[0]=='@')
	S=S.substr(1,S.size()-1);
      else
	S="\"use math\";"+S;
      S+='\n';
      char * js=js_ck_eval(S.c_str(),&global_js_context);
      if (js){
	S=js;
	free(js);
	process_freeze();
	update_js_vars();
      }
      else S="Error";
      Console_Output(S.c_str());
      return 0;
    }
#endif
#ifdef MICROPY_LIB
    if (strlen(s)>=6 && strlen(s)<8 && strncmp(s,"python",6)==0){
      switch_to_micropy(contextptr);
      return 0;
    }
    if (xcas_python_eval==1){
      freezeturtle=false;
      micropy_ck_eval(s);
    }
    else 
      do_run(s,g,ge,contextptr);
#else
    do_run(s,g,ge,contextptr);
#endif
    process_freeze();
#ifdef MICROPY_LIB
    if (xcas_python_eval==1)
      return 0;
#endif
#ifdef QUICKJS
    if (xcas_python_eval==-1)
      return 0;
#endif
    int t=giac::taille(g,GIAC_HISTORY_MAX_TAILLE);  
    int te=giac::taille(ge,GIAC_HISTORY_MAX_TAILLE);
    bool do_tex=false;
    if (t<GIAC_HISTORY_MAX_TAILLE && te<GIAC_HISTORY_MAX_TAILLE){
      giac::vecteur &vin=history_in(contextptr);
      giac::vecteur &vout=history_out(contextptr);
      if (vin.size()>GIAC_HISTORY_SIZE)
	vin.erase(vin.begin());
      vin.push_back(g);
      if (vout.size()>GIAC_HISTORY_SIZE)
	vout.erase(vout.begin());
      vout.push_back(ge);
    }
    if (check_do_graph(ge,g,do_logo_graph_eqw,contextptr)==KEY_SHUTDOWN)
      return KEY_SHUTDOWN;
    string s_;
    if (ge.type==giac::_STRNG)
      s_='"'+*ge._STRNGptr+'"';
    else {
      if (te>256)
	s_="Object too large";
      else {
	if (ge.is_symb_of_sommet(giac::at_pnt) || (ge.type==giac::_VECT && !ge._VECTptr->empty() && ge._VECTptr->back().is_symb_of_sommet(giac::at_pnt)))
	  s_="Graphic object";
	else {
	  //do_tex=ge.type==giac::_SYMB && has_op(ge,*giac::at_inv);
	  // tex support has been disabled!
	  s_=ge.print(contextptr);
	  // translate to tex? set do_tex to true
	}
      }
    }
#ifdef NUMWORKS
    if (s_.size()>512)
      s_=s_.substr(0,509)+"...";
#else
    if (s_.size()>8192)
      s_=s_.substr(0,8189)+"...";
#endif
    char* edit_line = (char*)Console_GetEditLine();
    Console_Output((const char*)s_.c_str());
    return 0; 
  }

  int run_session(int start,GIAC_CONTEXT){
    std::vector<std::string> v;
    for (int i=start;i<Last_Line;++i){
      if (Line[i].type==LINE_TYPE_INPUT)
	v.push_back((const char *)Line[i].str);
      console_free(Line[i].str);
      Line[i].str=0;
      Line[i].readonly = 0;
      Line[i].type = LINE_TYPE_INPUT;
      Line[i].start_col = 0;
      Line[i].disp_len = 0;
    }
    Line[Last_Line].str=0;
    Last_Line=start;
    if (start<Start_Line)
      Start_Line=start;
    int savestartline=Start_Line;
    Start_Line=Last_Line>LINE_DISP_MAX?Last_Line-LINE_DISP_MAX:0;
    Cursor.x=0;
    Cursor.y=start-Start_Line;
    Line[start].str=Edit_Line;
    Edit_Line[0]=0;
    if (v.empty()) return 0;
    //Console_Init(contextptr);
    for (int i=0;i<v.size();++i){
      Console_Output((const char *)v[i].c_str());
      //int j=Last_Line;
      Console_NewLine(LINE_TYPE_INPUT, 1);
      // Line[j].type=LINE_TYPE_INPUT;
      Console_Disp(1,contextptr);
      Bdisp_PutDisp_DD();
      run(v[i].c_str(),6,contextptr); /* show logo and graph but not eqw */
      // j=Last_Line;
      Console_NewLine(LINE_TYPE_OUTPUT, 1);    
      // Line[j].type=LINE_TYPE_OUTPUT;
    }
    int cl=Current_Line;
    Cursor.y += (Start_Line-savestartline);
    if (Cursor.y<0) Cursor.y=0;
    Start_Line=savestartline;
    if (Current_Line>cl || Cursor.y>10){
      if (cl>10){
	Start_Line=cl-10;
	Cursor.y=10;
      }
      else {
	Start_Line=0;
	Cursor.y=cl;
      }
    }
    Console_Disp(1,contextptr);
    Bdisp_PutDisp_DD();
    return 0;
  }

  string khicas_state(GIAC_CONTEXT){
    dbgprintf("khicas_state %08lx \n",contextptr);
    giac::gen g(giac::_VARS(-1,contextptr)); 
    dbgprintf("khicas_state 0.0\n");
    int b=python_compat(contextptr);
    dbgprintf("khicas_state 0.1\n");
    python_compat(0,contextptr);
    dbgprintf("khicas_state 0.2\n");
#if 1
#ifdef NSPIRE_NEWLIB
    char *buf=nspire_filebuf;
    buf[0]=0;
    int bufsize=NSPIRE_FILEBUFFER;
#else
#ifdef HP39
    int bufsize=6144;
    dbgprintf("khicas_state 0.3\n");
    char * buf=(char *)malloc(bufsize);
    dbgprintf("khicas_state 0.5\n");
    if (!buf) return "";
    buf[0]=0;
#else
    char buf[6144]="";
    int bufsize=sizeof(buf);
#endif
#endif
    if (g.type==giac::_VECT){
      bool ok=true;
      for (int i=0;i<g._VECTptr->size();++i){
        string s((*g._VECTptr)[i].print(contextptr));
        if (strlen(buf)+s.size()+128<bufsize){
          strcat(buf,s.c_str());
          strcat(buf,":;");
        }
        else
          ok=false;
      }
      if (!ok){
        confirm((lang==1)?"Contexte trop lourd, non sauvegarde":"Context too havy, not saved.",(lang==1)?"Re-executez scripts au chargement (esc enter)":"Re-run scripts at load time (esc enter)",true,64);
        buf[0]=0;
      }
    }
    dbgprintf("khicas_state 1\n");    
    python_compat(b,contextptr);
    if (strlen(buf)+184<bufsize){
      strcat(buf,"python_compat(");
      strcat(buf,giac::print_INT_(b).c_str());
      strcat(buf,",");
      strcat(buf,giac::print_INT_(pythonjs_heap_size).c_str());
      strcat(buf,",");
      strcat(buf,giac::print_INT_(pythonjs_stack_size).c_str());
      strcat(buf,");angle_radian(");
      strcat(buf,angle_radian(contextptr)?"1":"0");
      strcat(buf,");with_sqrt(");
      strcat(buf,withsqrt(contextptr)?"1":"0");
      strcat(buf,");integer_format(");
      strcat(buf,integer_format(contextptr)==16?"16":"10");
      strcat(buf,");set_language(");
      char l[]="0";
      l[0]+=lang;
      strcat(buf,l);
      strcat(buf,");");
    }
    dbgprintf("khicas_state 2\n");    
    if (sheetptr){
      string s(current_sheet(vecteur(0),contextptr).print(contextptr));
      if (strlen(buf)+s.size()+20<bufsize){
        strcat(buf,"current_sheet(");
        strcat(buf,s.c_str());
        strcat(buf,");");
      }
    }
    dbgprintf("khicas_state 3\n");
#ifdef HP39
    string res(buf);
    free(buf);
    return res;
#endif
    return buf;
#else
    string s(g.print(contextptr));
    python_compat(b,contextptr);
    s += "; python_compat(";
    s +=  giac::print_INT_(b);
    s += ");angle_radian(";
    s += angle_radian(contextptr)?'1':'0';
    s += ");with_sqrt(";
    s += withsqrt(contextptr)?'1':'0';
    s += ");";
    return s;
#endif
  }
  void Bfile_WriteFile_OS(char * & buf,const void * ptr,size_t len){
    memcpy(buf,ptr,len);
    buf += len;
  }
  void Bfile_WriteFile_OS4(char * & buf,size_t n){
    buf[0]= n>>24;
    buf[1]= (n>>16) & 0xff;
    buf[2]= (n & 0xffff)>>8;
    buf[3]= n & 0xff;
    buf += 4;
  }
  void Bfile_WriteFile_OS2(char * & buf,unsigned short n){
    buf[0]= n>>8;
    buf[1]= n & 0xff;
    buf += 2;
  }
  void save_console_state_smem(const char * filename,bool xwaspy,GIAC_CONTEXT){
    console_changed=0;
    dbgprintf("save_console_state %s\n",filename);
    string state(khicas_state(contextptr));
    int statesize=state.size();
    dbgprintf("save_console_state %s %i\n",filename,statesize);
    string script;
    if (edptr)
      script=merge_area(edptr->elements);
    int scriptsize=script.size();
    // save format: line_size (2), start_col(2), line_type (1), readonly (1), line
    int size=2*sizeof(int)+statesize+scriptsize;
    int start_row=Last_Line-max_lines_saved; 
    if (start_row<0) start_row=0;
    for (int i=start_row;i<=Last_Line;++i){
      size += 2*sizeof(short)+2*sizeof(char)+strlen((const char *)Line[i].str);
    }
    char savebuf[size+4];
#ifdef NUMWORKS
    char * hFile=savebuf+1;
#else
    char * hFile=savebuf;
#endif
    // save variables and modes
    Bfile_WriteFile_OS4(hFile, statesize);
    Bfile_WriteFile_OS(hFile, state.c_str(), statesize);
    // save script
    Bfile_WriteFile_OS4(hFile, scriptsize);
    Bfile_WriteFile_OS(hFile, script.c_str(), scriptsize);
    // save console state
    // save console state
    for (int i=start_row;i<=Last_Line;++i){
      console_line & cur=Line[i];
      unsigned short l=strlen((const char *)cur.str);
      Bfile_WriteFile_OS2(hFile, l);
      unsigned short s=cur.start_col;
      Bfile_WriteFile_OS2(hFile, s);
      unsigned char c=cur.type;
      Bfile_WriteFile_OS(hFile, &c, sizeof(c));
      c=1;//cur.readonly;
      Bfile_WriteFile_OS(hFile, &c, sizeof(c));
      unsigned char buf[l+1];
      buf[l]=0;
      strcpy((char *)buf,(const char*)cur.str); 
      unsigned char *ptr=buf,*strend=ptr+l;
      for (;ptr<strend;++ptr){
        if (*ptr==0x9c)
          *ptr='\n';
      }
      Bfile_WriteFile_OS(hFile, buf, l);
    }
    char BUF[2]={0,0};
    Bfile_WriteFile_OS(hFile, BUF, sizeof(BUF));
#ifdef NUMWORKS
    savebuf[0]=1;
#endif
    int len=hFile-savebuf;
    if (
#ifdef XWASPY
        xwaspy && len<8192
#else
        0
#endif
        ){
      // save as an ascii file beginning with #xwaspy
#ifdef NUMWORKS 
      --len;
      char * buf=savebuf+1;
      int newlen=4*(len+2)/3+11; // 4/3 oldlen + 8(#swaspy\n) +1 + 2 for ending  zeros
      char newbuf[newlen];
      strcpy(newbuf,"##xwaspy\n");
      newbuf[0]=1;
      hFile=newbuf+9;
#else
      char * buf=savebuf;
      int newlen=4*(len+2)/3+10;
      char newbuf[newlen];
      strcpy(newbuf,"#xwaspy\n");
      hFile=newbuf+8;
#endif
      for (int i=0;i<len;i+=3,hFile+=4){
        // keep space \n and a..z chars
        char c;
        while (i<len && ((c=buf[i])==' ' || c=='\n' || c=='{' || c==')' || c==';' || c==':' || c=='\n' || (c>='a' && c<='z')) ){
          if (c==')')
            c='}';
          if (c==':')
            c='~';
          if (c==';')
            c='|';
          *hFile=c;
          ++hFile;
          ++i;
        }
        unsigned char a=buf[i],b=i+1<len?buf[i+1]:0,C=i+2<len?buf[i+2]:0;
        hFile[0]=xwaspy_shift+(a>>2);
        hFile[1]=xwaspy_shift+(((a&3)<<4)|(b>>4));
        hFile[2]=xwaspy_shift+(((b&0xf)<<2)|(C>>6));
        hFile[3]=xwaspy_shift+(C&0x3f);
      }
      //*hFile=0; ++hFile; 
      //*hFile=0; ++hFile;
      write_file(filename,newbuf,hFile-newbuf);
    }
    else {
      write_file(filename,savebuf,len);
    }
  }

  size_t Bfile_ReadFile_OS4(const char * & hf_){
    const unsigned char * hf=(const unsigned char *)hf_;
    size_t n=(((((hf[0]<<8)+hf[1])<<8)+hf[2])<<8)+hf[3];
    hf_ += 4;
    return n;
  }

  size_t Bfile_ReadFile_OS2(const char * & hf_){
    const unsigned char * hf=(const unsigned char *)hf_;
    size_t n=(hf[0]<<8)+hf[1];
    hf_ += 2;
    return n;
  }

  void Bfile_ReadFile_OS(const char * &hf,char * dest,size_t len){
    memcpy(dest,hf,len);
    hf += len;
  }

  bool load_console_state_smem(const char * filename,GIAC_CONTEXT){
    dbgprintf("load_console_state %s\n",filename);
    const char * hf=read_file(filename);
    //if (!hf){ console_output(filename,strlen(filename)); console_output(" not found\n",11); return true; }
    // if (strcmp(filename,"session.xw")){ console_output(hf,8); return true; }
    if (!hf) return false;
    string str;
    if (strncmp(hf,"#xwaspy\n",8)==0){
      hf+=8;
      const char * source=hf;
      for (;*source;source+=4){
	while (*source=='\n' || *source==' ' || (*source>='a' && *source<='~')){
	  char c=*source;
	  if (c=='}')
	    c=')';
	  if (c=='|')
	    c=';';
	  if (c=='~')
	    c=':';
	  str += c;
	  ++source;
	}
	if (!*source)
	  break;
	unsigned char a=source[0]-xwaspy_shift,b=source[1]-xwaspy_shift,c=source[2]-xwaspy_shift,d=source[3]-xwaspy_shift;
	str += (a<<2)|(b>>4);
	str += (b<<4)|(c>>2);
	str += (c<<6)|d;
      }
      hf=str.c_str();
    }
    size_t L=Bfile_ReadFile_OS4(hf);
    char BUF[L+4];
    BUF[1]=BUF[0]='/'; // avoid trying python compat.
    BUF[2]='\n';
    Bfile_ReadFile_OS(hf,BUF+3,L);
    BUF[L+3]=0;
    giac::gen g,ge;
    dconsole_mode=0; python_compat(contextptr)=0; xcas_mode(contextptr)=0;
    bool bi=try_parse_i(contextptr);
    try_parse_i(false,contextptr);
    do_run((char*)BUF,g,ge,contextptr);
    try_parse_i(bi,contextptr);
    dconsole_mode=1;
    // read script
    L=Bfile_ReadFile_OS4(hf);
    if (L>0){
      char bufscript[L+1];
      Bfile_ReadFile_OS(hf,bufscript,L);
      bufscript[L]=0;
      if (edptr==0)
	edptr=new textArea;
      if (edptr){
	edptr->elements.clear();
	edptr->clipline=-1;
	edptr->filename=remove_path(giac::remove_extension(filename))+".py";
	//cout << "script " << edptr->filename << endl;
	edptr->editable=true;
	edptr->changed=false;
	edptr->python=python_compat(contextptr);
	edptr->elements.clear();
#ifdef HP39
  edptr->y = 12;
  edptr->lineHeight=14;
  edptr->longlinescut=false;
#else  
	edptr->y=0;
#endif
	add(edptr,bufscript);
	edptr->line=0;
	//edptr->line=edptr->elements.size()-1;
	edptr->pos=0;
      }    
    }
    // read console state
    // insure parse messages are cleared
    Console_Init(contextptr);
    Console_Clear_EditLine();
    for (int pos=0;;++pos){
      unsigned short int l,curs;
      unsigned char type,readonly;
      if ( (l=Bfile_ReadFile_OS2(hf))==0) break;
      curs=Bfile_ReadFile_OS2(hf);
      type = *hf; ++hf;
      readonly=*hf; ++hf;
      char buf[l+1];
      Bfile_ReadFile_OS(hf,buf,l);
      buf[l]=0;
      // ok line ready in buf
      while (Line[Current_Line].readonly)
	Console_MoveCursor(CURSOR_DOWN);
      Console_Input(buf);
      Console_NewLine(LINE_TYPE_INPUT, 1);
#if 1
      if (Current_Line>0){
	console_line & cur=Line[Current_Line-1];
	cur.type=type;
	cur.readonly=readonly;
	cur.start_col+=curs;
      }
#endif
    }
    console_changed=0;
    int p=python_compat(contextptr);
    if (p>=0 && p&4){
      xcas_python_eval=1;
      if (edptr){
	check_parse(edptr,edptr->elements,python_compat(contextptr),contextptr);
      }
    }
    else
      xcas_python_eval=p<0?-1:0;
    if (p==-1){
      //js_ck_eval("1",&global_js_context);
      if (edptr)
	check_parse(edptr,edptr->elements,-1,contextptr);
    }
    Console_FMenu_Init(contextptr); // insure the menus are sync-ed
    return true;
  }

  /*

    The following functions will be used to specify the location before deleting a string of n characters altogether. Among them, a wide character (2 bytes) will be counted as a character.
	
    For example, we have the following string str:
	
    Location  |  0  |  1  |  2  |  3  |  4  |  5  | 6 |
    Character | 'a' | 'b' | 'c' | 'd' | 'e' | 'f' | 0 |

    After the call Console_DelStr (str, 3, 2), position 1 and 2 characters will be deleted, then the characters will be in advance.
	
    Results are as follows:

    Location  |  0  |  1  |  2  |  3  | 4 |  5  | 6 |
    Character | 'a' | 'd' | 'e' | 'f' | 0 | 'f' | 0 |

    (Note: the extra positions will not be filled with '\ 0', but '\ 0' will be a copy of the original end of the string.)

  */

  int Console_DelStr(char *str, int end_pos, int n)
  {
    int str_len, actual_end_pos, start_pos, actual_start_pos, del_len, i;

    str_len = strlen((const char *)str);
    if ((start_pos = end_pos - n) < 0) return CONSOLE_ARG_ERR;

    if ((actual_end_pos = Console_GetActualPos(str, end_pos)) == CONSOLE_ARG_ERR) return CONSOLE_ARG_ERR;
    if ((actual_start_pos = Console_GetActualPos(str, start_pos)) == CONSOLE_ARG_ERR) return CONSOLE_ARG_ERR;

    del_len = actual_end_pos - actual_start_pos;

    for (i = actual_start_pos; i < str_len; i++)
      {
	str[i] = str[i + del_len];
      }

    return CONSOLE_SUCCEEDED;
  }

  /*

    The following functions are used to specify the location of the insertion in the specified string.
    (Note: This refers to the position of the printing position when, rather than the actual position.)
  */

  int Console_InsStr(char *dest, const char *src, int disp_pos)
  {
    int i, ins_len, str_len, actual_pos;

    ins_len = strlen((const char *)src);
    str_len = strlen((const char *)dest);

    actual_pos = Console_GetActualPos(dest, disp_pos);

    if (ins_len + str_len >= EDIT_LINE_MAX) return CONSOLE_MEM_ERR;
    if (actual_pos > str_len) return CONSOLE_ARG_ERR;

    for (i = str_len; i >= actual_pos; i--)
      {
	dest[i + ins_len] = dest[i];
      }

    for (i = 0; i < ins_len; i++)
      {
	char c=src[i];
	if (c=='\n') c=0x9c;
	dest[actual_pos + i] = (c==0x0a?' ':c);
      }

    return CONSOLE_SUCCEEDED;
  }

  /*

    The following function is used to determine the true position of the string corresponding to the printing position.
    For example, in the following this string str contains wide characters, the location of the print is as follows:

    Location  | 00  |  01 |   02  |  03  | 04   | 05  | 06  |
    Character | one | two | three | four | five | six | \ 0 |

    The actual storage location is as follows:

    Location | 	00  | 01   |  02  |  03  |  04  |  05  |  06  |  07  |  08  |  09  |  10  |  11  |
    Value 	 | 0xD2 | 0xBB | 0xB6 | 0xFE | 0xC8 | 0xFD | 0xCB | 0xC4 | 0xCE | 0xE5 | 0xC1 | 0xF9 |

    You can find the first four characters 'five' is actually stored in the eighth position.
    So, when you call Console_GetActualPos (str, 4), it will return 8.
  */

  int Console_GetActualPos(const char *str, int disp_pos)
  {
    int actual_pos, count;

    for (actual_pos = count = 0; count < disp_pos; count++)
      {
	if (str[actual_pos] == '\0') return CONSOLE_ARG_ERR;

	if (is_wchar(str[actual_pos]))
	  {
	    actual_pos += 2;
	  }
	else
	  {
	    actual_pos++;
	  }
      }

    return actual_pos;
  }

  /*
    The following functions are used to obtain a string of print length, ie, a wide character (2 bytes) recorded as a character.
  */

  int Console_GetDispLen(const char *str)
  {
    int i, len;

    for (i = len = 0; str[i]!='\0'; len++)
      {
	if (is_wchar(str[i]))
	  {
	    i += 2;
	  }
	else
	  {
	    i++;
	  }
      }

    return len;
  }

  /*
    The following functions are used to move the cursor.
  */

  int Console_MoveCursor(int direction)
  {
    switch (direction)
      {
      case CURSOR_UP:
	if (Current_Line==Last_Line)
	  editline_cursor=Cursor.x;
	//If you need to operate.
	if ((Cursor.y > 0) || (Start_Line > 0)){
	  //If the current line is not read-only, then Edit_Line copy to the current line.
	  if (!Line[Current_Line].readonly){
	    if ((Line[Current_Line].str = (char *)console_malloc(strlen((const char *)Edit_Line) + 1)) == NULL) return CONSOLE_MEM_ERR;
	    strcpy((char *)Line[Current_Line].str, (const char *)Edit_Line);
	    Line[Current_Line].disp_len = Console_GetDispLen(Line[Current_Line].str);
	    Line[Current_Line].type = LINE_TYPE_INPUT;
	  }
	  //If the cursor does not move to the top of, directly move the cursor upward.
	  if (Cursor.y > 0)
	    Cursor.y--;
	  //Otherwise, the number of rows, if the screen's first line is not the first line, then began to show minus one.
	  else {
	    if (Start_Line > 0)
	      Start_Line--;
	  }
	  //End if the horizontal position after moving the cursor over the line, then move the cursor to the end of the line.
	  if (Cursor.x > Line[Current_Line].disp_len){
	    Cursor.x = Line[Current_Line].disp_len;
	  }
	  else {
	    if (Line[Current_Line].disp_len - Line[Current_Line].start_col > COL_DISP_MAX){
	      if (Cursor.x == COL_DISP_MAX)
		Cursor.x = COL_DISP_MAX - 1;
	    }
	  }
	  //If you move the cursor to the line after the first, and the front of the line there is a character does not appear, then move the cursor to position 1.
	  if (Cursor.x == 0 && Line[Current_Line].start_col > 0)
	    Cursor.x = 1;
	  //If the current cursor line is not read-only, then it is a string copy to Edit_Line for editing.
	  if (!Line[Current_Line].readonly && Line[Current_Line].str){
	    strcpy((char *)Edit_Line, (const char *)Line[Current_Line].str);
	    console_free(Line[Current_Line].str);
	    Line[Current_Line].str = Edit_Line;
	  }
	}
	break;
      case CURSOR_ALPHA_UP:{
	int pos1=Start_Line+Cursor.y;
	Console_MoveCursor(CURSOR_UP);
	int pos2=Start_Line+Cursor.y;
	if (pos1<Last_Line && pos2<Last_Line && pos1!=pos2){
	  console_line curline=Line[pos1];
	  Line[pos1]=Line[pos2];
	  Line[pos2]=curline;
	}
	break;
      }
      case CURSOR_ALPHA_DOWN: {
	int pos1=Start_Line+Cursor.y;
	Console_MoveCursor(CURSOR_DOWN);
	int pos2=Start_Line+Cursor.y;
	if (pos1<Last_Line && pos2<Last_Line && pos1!=pos2){
	  console_line curline=Line[pos1];
	  Line[pos1]=Line[pos2];
	  Line[pos2]=curline;
	}
	break;
      }
      case CURSOR_DOWN:
	if (Current_Line==Last_Line)
	  editline_cursor=Cursor.x;
	//If you need to operate.
	if ((Cursor.y < LINE_DISP_MAX - 1) && (Current_Line < Last_Line) || (Start_Line + LINE_DISP_MAX - 1 < Last_Line))
	  {
	    //If the current line is not read-only, then Edit_Line copy to the current line.
	    if (!Line[Current_Line].readonly)
	      {
		if ((Line[Current_Line].str = (char *)console_malloc(strlen((const char *)Edit_Line) + 1)) == NULL) return CONSOLE_MEM_ERR;
		strcpy((char *)Line[Current_Line].str, (const char *)Edit_Line);
		Line[Current_Line].disp_len = Console_GetDispLen(Line[Current_Line].str);
		Line[Current_Line].type = LINE_TYPE_INPUT;
	      }

	    //If the cursor does not move to the bottom, the cursor moves down directly.
	    if (Cursor.y < LINE_DISP_MAX - 1 && Current_Line < Last_Line)
	      {
		Cursor.y++;
	      }
	    //The number of rows Otherwise, if the last line is not the last line on the screen, it will begin to show a plus.
	    else if (Start_Line + LINE_DISP_MAX - 1 < Last_Line)
	      {
		Start_Line++;
	      }

	    //If you move the cursor after the end of the horizontal position over the line, then move the cursor to the end of the line.
	    if (Cursor.x > Line[Current_Line].disp_len)
	      {
		Cursor.x = Line[Current_Line].disp_len;
	      }
	    else if (Line[Current_Line].disp_len - Line[Current_Line].start_col >= COL_DISP_MAX)
	      {
		if (Cursor.x == COL_DISP_MAX) Cursor.x = COL_DISP_MAX - 1;
	      }

	    //If you move the cursor to the line after the first, and the front of the line there is a character does not appear, then move the cursor to position 1.
	    if (Cursor.x == 0 && Line[Current_Line].start_col > 0) Cursor.x = 1;

	    //If the current cursor line is not read-only, then it is a string copy to Edit_Line for editing.
	    if (!Line[Current_Line].readonly && Line[Current_Line].str)
	      {
		strcpy((char *)Edit_Line, (const char *)Line[Current_Line].str);
		console_free(Line[Current_Line].str);
		Line[Current_Line].str = Edit_Line;
	      }
	  }
	break;
      case CURSOR_LEFT:
	if (Line[Current_Line].readonly){
	  if (Line[Current_Line].start_col > 0){
	    Line[Current_Line].start_col--;
	  }
	  break;
	}
	else {
	  if (Line[Current_Line].start_col > 0){
	    if (Cursor.x > 1)
	      Cursor.x--;
	    else {
	      int dx=COL_DISP_MAX/3;
	      if (Line[Current_Line].start_col>=dx){
		Line[Current_Line].start_col-=dx;
		Cursor.x += dx-1;
	      } else
		Line[Current_Line].start_col--;
	    }
	    break;
	  }
	  if (Cursor.x > 0){
	    Cursor.x--;
	    break;
	  }
	}
      case CURSOR_SHIFT_RIGHT:
	if (!Line[Current_Line].readonly)
	  Cursor.x=giacmin(Line[Current_Line].disp_len,COL_DISP_MAX);
	if (Line[Current_Line].disp_len > COL_DISP_MAX)
	  Line[Current_Line].start_col = Line[Current_Line].disp_len - COL_DISP_MAX;
	break;
      case CURSOR_RIGHT:
	if (Line[Current_Line].readonly){
	  if (Line[Current_Line].disp_len - Line[Current_Line].start_col > COL_DISP_MAX){
	    Line[Current_Line].start_col++;
	  }
	  break;
	}
	else {
	  if (Line[Current_Line].disp_len - Line[Current_Line].start_col > COL_DISP_MAX){
	    if (Cursor.x < COL_DISP_MAX - 1)
	      Cursor.x++;
	    else {
	      int dx=COL_DISP_MAX/3;
	      if (dx>Cursor.x)
		dx=Cursor.x;
	      Line[Current_Line].start_col+=dx;
	      Cursor.x -= (dx-1);
	    }
	    break;	  
	  }
	  if (Cursor.x < Line[Current_Line].disp_len - Line[Current_Line].start_col){
	    Cursor.x++;
	    break;
	  }
	}
      case CURSOR_SHIFT_LEFT:
	if (!Line[Current_Line].readonly)
	  Cursor.x=0;
	Line[Current_Line].start_col=0;
	break;
      default:
	return CONSOLE_ARG_ERR;
	break;
      }
    return CONSOLE_SUCCEEDED;
  }

  /*
    The following function is used for input.
    String input to the cursor, the cursor will automatically move.
  */

  int Console_Input(const char *str)
  {
    console_changed=1;
    int old_len,i,return_val;

    if (!Line[Current_Line].readonly)
      {
	old_len = Line[Current_Line].disp_len;
	return_val = Console_InsStr(Edit_Line, str, Current_Col);
	if (return_val != CONSOLE_SUCCEEDED) return return_val;
	if ((Line[Current_Line].disp_len = Console_GetDispLen(Edit_Line)) == CONSOLE_ARG_ERR) return CONSOLE_ARG_ERR;
	for (i = 0; i < Line[Current_Line].disp_len - old_len; i++)
	  {
	    Console_MoveCursor(CURSOR_RIGHT);
	  }
	return CONSOLE_SUCCEEDED;
      }
    else
      {
	return CONSOLE_ARG_ERR;
      }
  }

  /*
    The following functions are used to output the string to the current line.
  */

  int Console_Output(const char *str)  {
    if (!Line) return 0;
    console_changed=1;
    int return_val, old_len, i;

    if (!Line[Current_Line].readonly)
      {
	old_len = Line[Current_Line].disp_len;

	return_val = Console_InsStr(Edit_Line, str, Current_Col);
	if (return_val != CONSOLE_SUCCEEDED) return return_val;
	if ((Line[Current_Line].disp_len = Console_GetDispLen(Edit_Line)) == CONSOLE_ARG_ERR) return CONSOLE_ARG_ERR;
	Line[Current_Line].type = LINE_TYPE_OUTPUT;

	for (i = 0; i < Line[Current_Line].disp_len - old_len; i++)
	  {
	    Console_MoveCursor(CURSOR_RIGHT);
	  }
	return CONSOLE_SUCCEEDED;
      }
    else
      {
	return CONSOLE_ARG_ERR;
      }
  }

  void dConsolePut(const char * S){
    if (!dconsole_mode)
      return;
    int l=strlen(S);
    char s[l+1];
    strcpy(s,S);
    for (int i=0;i<l;++i){
      if (s[i]=='\n' ||
	  s[i]==10)
	s[i]=' ';
    }
    Console_Output((const char *)s);
    if (l && S[l-1]=='\n'){
      Console_NewLine(LINE_TYPE_OUTPUT, 1);
      if (!freeze)
	Console_Disp(1,0);
    }
  }

  void dPuts(const char * s){
    dConsolePut(s);
  }

#define PUTCHAR_LEN 35
  static char putchar_buf[PUTCHAR_LEN+2];
  static int putchar_pos=0;
  void dConsolePutChar(const char ch){
    if (!dconsole_mode)
      return;
    if (putchar_pos==PUTCHAR_LEN)
      dConsolePutChar('\n');
    if (ch=='\n'){
      putchar_buf[putchar_pos]='\n';
      putchar_buf[putchar_pos+1]=0;
      putchar_pos=0;
      dConsolePut(putchar_buf);
    }
    else {
      putchar_buf[putchar_pos]=ch;
      ++putchar_pos;
    }
  }

  /*
    Clear the current output line
  */

  void Console_Clear_EditLine()
  {
    if(!Line[Current_Line].readonly) {
      Edit_Line[0] = '\0';
      Line[Current_Line].start_col = 0;
      Line[Current_Line].disp_len = 0;
      Cursor.x = 0;
    }
  }

  /*

    The following functions are used to create a new line.
    Pre_line_type type parameter is used to specify the line, pre_line_readonly parameter is used to specify the line is read-only.
    New_line_type parameter is used to specify the type of the next line, new_line_readonly parameter is used to specify the next line is read-only.
  */

  int Console_NewLine(int pre_line_type, int pre_line_readonly)
  {
    if (!Line) return 0;
    console_changed=1;
    int i;

    if (strlen((const char *)Edit_Line)||Line[Current_Line].type==LINE_TYPE_OUTPUT)
      {
	//Èç¹ûÒÑ¾­ÊÇËùÄÜ´æ´¢µÄ×îºóÒ»ÐÐ£¬ÔòÉ¾³ýµÚÒ»ÐÐ¡£
	//If this is the last line we can store, delete the first line.
	if (Last_Line == _LINE_MAX - 1)
	  {
	    for (i = 0; i < Last_Line; i++)
	      {
		Line[i].disp_len = Line[i + 1].disp_len;
		Line[i].readonly = Line[i + 1].readonly;
		Line[i].start_col = Line[i + 1].start_col;
		Line[i].str = Line[i + 1].str;
		Line[i].type = Line[i + 1].type;
	      }
	    Last_Line--;

	    if (Start_Line > 0) Start_Line--;
	  }

	if (Line[Last_Line].type == LINE_TYPE_OUTPUT && strlen((const char *)Edit_Line) == 0) Console_Output((const char *)"Done");

	//Edit_Line copy the contents to the last line.

	if ((Line[Last_Line].str = (char *)console_malloc(strlen((const char *)Edit_Line) + 1)) == NULL) return CONSOLE_MEM_ERR;
	strcpy((char *)Line[Last_Line].str, (const char *)Edit_Line);

	if ((Line[Last_Line].disp_len = Console_GetDispLen(Line[Last_Line].str)) == CONSOLE_ARG_ERR) return CONSOLE_ARG_ERR;
	Line[Last_Line].type = pre_line_type;
	Line[Last_Line].readonly = pre_line_readonly;
	Line[Last_Line].start_col = 0;

	Edit_Line[0] = '\0';

	Last_Line++;

	Cursor.x = 0;

	if ((Last_Line - Start_Line) == LINE_DISP_MAX)
	  {
	    Start_Line++;
	  }
	else
	  {
	    Cursor.y++;
	  }

	Line[Last_Line].str = Edit_Line;
	Line[Last_Line].readonly = 0;
	Line[Last_Line].type = LINE_TYPE_INPUT;
	Line[Last_Line].start_col = 0;
	Line[Last_Line].disp_len = 0;

	return CONSOLE_NEW_LINE_SET;
      }
    else
      {
	return CONSOLE_NO_EVENT;
      }
  }

  void Console_Insert_Line(){
    if (Last_Line>=_LINE_MAX-1)
      return;
    for (int i=Last_Line;i>=Current_Line;--i){
      Line[i+1]=Line[i];
    }
    ++Last_Line;
    int i=Current_Line;
    console_line & l=Line[i];
    l.str=(char *)console_malloc(2);
    strcpy((char *)l.str,"0");
    l.type=Line[i+1].type==LINE_TYPE_INPUT?LINE_TYPE_OUTPUT:LINE_TYPE_INPUT;
    l.start_col=0;
    l.readonly=1;
    l.disp_len=Console_GetDispLen(l.str);
  }

  /*
    The following function is used to delete a character before the cursor.
  */

  int Console_Backspace(GIAC_CONTEXT){
    console_changed=1;
    if (Last_Line>0 && Current_Line<Last_Line){
      int i=Current_Line;
      if (Edit_Line==Line[i].str)
	Edit_Line=Line[i+1].str;
      if (Line[i].str){
	copy_clipboard((const char *)Line[i].str,true);
	console_free(Line[i].str);
      }
      for (;i<Last_Line;++i){
	Line[i]=Line[i+1];
      }
      Line[i].readonly = 0;
      Line[i].type = LINE_TYPE_INPUT;
      Line[i].start_col = 0;
      Line[i].disp_len = 0;
      Line[i].str=0;
      --Last_Line;
      if (Start_Line>0)
	--Start_Line;
      else {
	if (Cursor.y>0)
	  --Cursor.y;
      }
#if 1
      if (Last_Line==0 && Current_Line==0){ // workaround
	char buf[strlen((const char*)Edit_Line)+1];
	strcpy(buf,(const char*)Edit_Line);
	Console_Init(contextptr);
	Console_Clear_EditLine();
	if (buf[0])
	  Console_Input((const char *)buf);
	//std::string status(giac::print_INT_(Last_Line)+" "+(giac::print_INT_(Current_Line)+" ")+giac::print_INT_(Line[Current_Line].str)+" "+(const char*)Line[Current_Line].str);
	//DefineStatusMessage(status.c_str(),1,0,0);
	//DisplayStatusArea();
      }
#endif
      Console_Disp(1,0);
      return CONSOLE_SUCCEEDED;
    }
    int return_val;
    return_val = Console_DelStr(Edit_Line, Current_Col, 1);
    if (return_val != CONSOLE_SUCCEEDED) return return_val;
    Line[Current_Line].disp_len = Console_GetDispLen(Edit_Line);
    return Console_MoveCursor(CURSOR_LEFT);
  }

  /*
    The following functions are used to deal with the key.
  */

  void chk_clearscreen(GIAC_CONTEXT){
    drawRectangle(0, 24, LCD_WIDTH_PX, LCD_HEIGHT_PX-24, COLOR_WHITE);
    if (confirm((lang==1)?"Effacer l'historique?":"Clear history?",
#ifdef NSPIRE_NEWLIB
		(lang==1)?"enter: oui, esc: conserver":"enter: yes, esc: keep",
#else
		(lang==1)?"OK: oui, Back: conserver":"OK: yes, Back: keep",
#endif
		false)==KEY_CTRL_F1){
      Console_Init(contextptr);
      Console_Clear_EditLine();
    }    
    Console_Disp(1,0);
  }


  /*
    int handle_f5(){
    int keyflag = GetSetupSetting( (unsigned int)0x14);
    if (keyflag == 0x04 || keyflag == 0x08 || keyflag == 0x84 || keyflag == 0x88) {
    // ^only applies if some sort of alpha (not locked) is already on
    if (keyflag == 0x08 || keyflag == 0x88) { //if lowercase
    SetSetupSetting( (unsigned int)0x14, keyflag-0x04);
    DisplayStatusArea();
    return 1; //do not process the key, because otherwise we will leave alpha status
    } else {
    SetSetupSetting( (unsigned int)0x14, keyflag+0x04);
    DisplayStatusArea();
    return 1; //do not process the key, because otherwise we will leave alpha status
    }
    }
    if (keyflag==0) {
    SetSetupSetting( (unsigned int)0x14, 0x88);	
    DisplayStatusArea();
    }
    return 0;
    }
  */

  int Console_Eval(const char * buf,GIAC_CONTEXT){
    int start=Current_Line;
    console_free(Line[start].str);
    Line[start].str=(char *)console_malloc(strlen(buf)+1);
    strcpy((char *)Line[start].str,buf);
    run_session(start,contextptr);
    int move_line = Last_Line - start;
    for (int i = 0; i < move_line; i++)
      Console_MoveCursor(CURSOR_UP);
    return CONSOLE_SUCCEEDED;
  }


  void save(const char * fname,GIAC_CONTEXT){
    dbgprintf("save %s %08lx \n",fname,contextptr);
    if (nspire_exam_mode==2)
      return;
    clear_abort();
#if 0
    return;
#else
    string filename(remove_path(remove_extension(fname)));
#if defined NUMWORKS && defined XWASPY
    bool xwaspy=filename!="session"; // xw will be saved as a fake .py file
#else
    bool xwaspy=false;
#endif
    if (xwaspy){
      if (filename.size()>3 && filename.substr(filename.size()-3,3)=="_xw")
	filename += ".py";
      else
	filename += "_xw.py";
    }
    else
      filename+=".xw";
#ifdef NSPIRE_NEWLIB
    filename+=".tns";
#endif
    save_console_state_smem(filename.c_str(),xwaspy,contextptr); // call before save_khicas_symbols_smem(), because this calls create_data_folder if necessary!
    // save_khicas_symbols_smem(("\\\\fls0\\"+filename+".xw").c_str());
    if (edptr)
      check_leave(edptr);
#endif
  }

  int restore_script(string &filename,bool msg,GIAC_CONTEXT){
    // it's not a session, but a script, restore last session settings and load script
#ifdef NSPIRE_NEWLIB
    const char sessionname[]="session.xw.tns";
#else
    const char sessionname[]="session.xw";
#endif 
    if (file_exists(sessionname)){
      load_console_state_smem(sessionname,contextptr);
      Console_Init(contextptr);
      Console_Clear_EditLine();
    }
    else python_compat(1,contextptr);
    //return 1;
    string s;
    filename=remove_path(remove_extension(filename));
    if (msg && filename!="session"){
      *logptr(contextptr) << (lang==1?"shift ) 8 ou python/xcas pour changer d'interpreteur\n":"shift ) 8 or python/xcas to change interpreter\n");
      *logptr(contextptr) << (lang==1?"Taper esc pour editeur ou avec Micropython executez\n":"Press esc for editor or in MicroPython exec\n");
      *logptr(contextptr) << "from "+filename+" import *\n";
    }
#ifdef NSPIRE_NEWLIB
    filename += ".py.tns";
#else
    filename += ".py";
#endif
    load_script(filename.c_str(),s);
    if (s.empty())
      s="\n";
    if (edptr==0)
      edptr=new textArea;
    edptr->filename=filename;
    edptr->editable=true;
    edptr->changed=false;
    edptr->python=python_compat(contextptr);
    edptr->elements.clear();
#ifdef HP39
    edptr->y = 12;
    edptr->lineHeight=14;
    edptr->longlinescut=false;
#else  
    edptr->y=0; // 7??
#endif
    add(edptr,s);
    edptr->line=0;
    edptr->pos=0;
    return 2;
  }
#if defined NUMWORKS && defined DEVICE
  void numworks_certify_internal(){
    // check internal flash sha256 signature
    size_t internal_flash_start=0x08000000;
    if (bootloader_sha256_check(internal_flash_start)){
      confirm(lang==1?"Amorcage calculatrice certifie!":"Boot sector certified!",lang==1?"Acces amorcage par Power ln":"Access boot with Power ln");
      Bdisp_AllClr_VRAM();
      return;
    }
    PrintMini(0,0,lang==1?"Amorcage non certifie.":"Boot sector not certified.",TEXT_MODE_NORMAL,COLOR_BLACK, COLOR_WHITE);
    std::vector<fileinfo_t> v=tar_fileinfo(flash_buf,0);
    int i=0;
    for (;i<v.size();++i){
      fileinfo_t & f=v[i];
      if (f.filename=="bootloader.bin" && f.size==bootloader_size)
	break;
    }
    // check file content signature
    unsigned romaddr=((unsigned) flash_buf) +v[i].header_offset+0x200;
    if (i<v.size() && !bootloader_sha256_check(romaddr))
      i=v.size();
    if (i==v.size()){
      PrintMini(0,18,lang==1?"Pour mettre a jour:":"Please upgrade from:",TEXT_MODE_NORMAL,COLOR_BLACK, COLOR_WHITE);
      PrintMini(0,36,"www-fourier.univ-grenoble-alpes.fr",TEXT_MODE_NORMAL,COLOR_BLACK, COLOR_WHITE);
      PrintMini(0,54,"/~parisse/nw",TEXT_MODE_NORMAL,COLOR_BLACK, COLOR_WHITE);
      int key; GetKey(&key);
    }
    else {
      if (confirm(lang==1?"Mise a jour de l'amorcage ?":"Update bootsector?","OK/Annul.")==KEY_CTRL_F1){
	// sector size=16K -> erase 4 sector for 64K
	erase_sector((const char *) internal_flash_start);
	erase_sector((const char *) (internal_flash_start+16*1024));
	erase_sector((const char *) (internal_flash_start+32*1024));
	erase_sector((const char *) (internal_flash_start+48*1024));
	WriteMemory((char *)internal_flash_start,(const char *) romaddr,bootloader_size);
	confirm(lang==1?"Mise a jour faite":"Update done",lang==1?"Acces amorcage par Power ln":"Access boot with Power ln");
      }
    }
    Bdisp_AllClr_VRAM();
  }
  				   
  int restore_session(const char * fname,GIAC_CONTEXT){
    // cout << "0" << fname << endl; Console_Disp(1); GetKey(&key);
    string filename(remove_path(remove_extension(fname)));
#ifdef NSPIRE_NEWLIB
    if (file_exists((filename+".xw.tns").c_str()))
      filename += ".xw.tns";
    else
      filename += ".py.tns";
#else
    if (file_exists((filename+".xw").c_str()))
      filename += ".xw";
    else
      filename += ".py";
#endif
    if (!load_console_state_smem(filename.c_str(),contextptr)){
      dbgprintf("restore_session not found\n");
      if (confirm("OK: Francais, Back: English","set_language(1|0)")==KEY_CTRL_F6)
        lang=0;
      numworks_certify_internal();
      Bdisp_AllClr_VRAM();
      int x=0,y=0;
      PrintMini(x,y,"KhiCAS 1.9 (c) 2022 B. Parisse",TEXT_MODE_NORMAL, COLOR_BLACK, COLOR_WHITE);
      y +=18;
      PrintMini(x,y,"et al, License GPL 2",TEXT_MODE_NORMAL,COLOR_BLACK, COLOR_WHITE);
      y += 18;
#ifdef NSPIRE_NEWLIB
      PrintMini(x,y,((lang==1)?"Taper menu plusieurs fois":"Type menu several times"),TEXT_MODE_NORMAL,COLOR_BLACK, COLOR_WHITE);
#else
      PrintMini(x,y,((lang==1)?"Taper HOME plusieurs fois":"Type HOME several times"),TEXT_MODE_NORMAL,COLOR_BLACK, COLOR_WHITE);
#endif
      y += 18;
      PrintMini(x,y,((lang==1)?"pour quitter KhiCAS.":"to leave KhiCAS."),TEXT_MODE_NORMAL,COLOR_BLACK, COLOR_WHITE);
      y += 18;
      PrintMini(x,y,(lang==1)?"Si le calcul formel est interdit":"If CAS is forbidden!",TEXT_MODE_NORMAL, _red, COLOR_WHITE);
      y += 18;
#ifdef NSPIRE_NEWLIB
      PrintMini(x,y,(lang==1)?"quittez Khicas (menu menu menu)":"Leave Khicas (menu menu menu)",TEXT_MODE_NORMAL, _red, COLOR_WHITE);
      if (confirm("Interpreter? enter: Xcas, esc: MicroPython",(lang==1?"Peut se modifier depuis menu configuration":"May be changed later from menu configuration"),false,130)==KEY_CTRL_F6){
	python_compat(4,contextptr);
	xcas_python_eval=1;
	*logptr(contextptr) << "Micropython interpreter\n";
	Console_FMenu_Init(contextptr);
      }
      else {
	python_compat(1,contextptr);
	*logptr(contextptr) << "Xcas interpreter, Python compatible mode\n";
      }
#else
      PrintMini(x,y,(lang==1)?"quittez Khicas (HOME HOME HOME)":"Leave Khicas (HOME HOME HOME)",TEXT_MODE_NORMAL, _red, COLOR_WHITE);
      if (confirm("Interpreter? OK: Xcas, Back: MicroPython",(lang==1?"Peut se modifier depuis menu configuration":"May be changed later from menu configuration"),false,130)==KEY_CTRL_F6){
	python_compat(4,contextptr);
	xcas_python_eval=1;
	*logptr(contextptr) << "Micropython interpreter\n";
	Console_FMenu_Init(contextptr);
      }
      else {
	python_compat(1,contextptr);
	// fake lexer required to initialize color syntax
	gen g("abs",contextptr);
	*logptr(contextptr) << "Xcas interpreter, Python compatible mode\n";
      }
#endif
#ifdef NUMWORKS
      if (lang==1){
	*logptr(contextptr) << "!!! ATTENTION !!!\n";
	*logptr(contextptr) << "Ne faites pas de mises a jour\n";
	*logptr(contextptr) << "depuis le site de Numworks.\n";
	*logptr(contextptr) << "Cela verrouille la Numworks\n";
	*logptr(contextptr) << "et empeche d'utiliser KhiCAS\n";
      } else {
	*logptr(contextptr) << "!!! BEWARE !!!\n";
	*logptr(contextptr) << "Don't make upgrades\n";
	*logptr(contextptr) << "from Numworks website\n";
	*logptr(contextptr) << "They lock your calculator\n";
	*logptr(contextptr) << "it's incompatible with KhiCAS\n";
      }
#endif
      Bdisp_AllClr_VRAM();
#ifdef GIAC_SHOWTIME
      Console_Output("Reglage de l'heure, exemple");
      Console_NewLine(LINE_TYPE_OUTPUT, 1);          
      Console_Output("12,37=>,");
      Console_NewLine(LINE_TYPE_OUTPUT, 1);
#endif
      //menu_about();
      return 0;
    }
    return 1;
  }

#else // NUMWORKS && DEVICE

  int restore_session(const char * fname,GIAC_CONTEXT){
    // cout << "0" << fname << endl; Console_Disp(1); GetKey(&key);
    string filename(fname); //filename="mandel.py.tns";
    if (filename.size()>4 && filename.substr(filename.size()-4,4)==".tns")
      filename=filename.substr(0,filename.size()-4);
    if (filename.size()>3 && filename.substr(filename.size()-3,3)==".py"){
      if (filename.size()<5 || filename.substr(filename.size()-5,5)!="xw.py")
	return restore_script(filename,true,contextptr);
    }
    filename=remove_path(remove_extension(fname));
#ifdef NSPIRE_NEWLIB
    if (file_exists((filename+".xw.tns").c_str())){
      strcpy(session_filename,filename.c_str());
      filename += ".xw.tns";
    }
    else {
      if (file_exists((filename+".py.tns").c_str()))
	return restore_script(filename,true,contextptr);
    }
#else
    if (file_exists((filename+".xw").c_str())){
      strcpy(session_filename,filename.c_str());
      filename += ".xw";
    }
    else {
      if ((filename.size()<2 || filename.substr(filename.size()-2,2)!="xw") && file_exists((filename+".py").c_str()))
	return restore_script(filename,true,contextptr);
    }
#endif
    if (!load_console_state_smem(filename.c_str(),contextptr)){
#ifdef NUMWORKS
      if (confirm("OK: Francais, Back: English","set_language(1|0)")==KEY_CTRL_F6)
	lang=0;
      Bdisp_AllClr_VRAM();
#endif
      int x=0,y=0;
      PrintMini(x,y,"KhiCAS 1.6 (c) 2020 B. Parisse",TEXT_MODE_NORMAL, COLOR_BLACK, COLOR_WHITE);
      y +=18;
      PrintMini(x,y,"et al, License GPL 2",TEXT_MODE_NORMAL,COLOR_BLACK, COLOR_WHITE);
      y += 18;
#ifdef NSPIRE_NEWLIB
      PrintMini(x,y,((lang==1)?"Taper menu plusieurs fois":"Type menu several times"),TEXT_MODE_NORMAL,COLOR_BLACK, COLOR_WHITE);
#else
      PrintMini(x,y,((lang==1)?"Taper HOME plusieurs fois":"Type HOME several times"),TEXT_MODE_NORMAL,COLOR_BLACK, COLOR_WHITE);
#endif
      y += 18;
      PrintMini(x,y,((lang==1)?"pour quitter KhiCAS.":"to leave KhiCAS."),TEXT_MODE_NORMAL,COLOR_BLACK, COLOR_WHITE);
      y += 18;
      PrintMini(x,y,(lang==1)?"Si le calcul formel est interdit":"If CAS is forbidden!",TEXT_MODE_NORMAL, _red, COLOR_WHITE);
      y += 18;
#ifdef NSPIRE_NEWLIB
      PrintMini(x,y,(lang==1)?"quittez Khicas (doc doc doc)":"Leave Khicas (doc doc doc)",TEXT_MODE_NORMAL, _red, COLOR_WHITE);
      if (confirm("Interpreter? enter: Xcas, esc: MicroPython",(lang==1?"Peut se modifier depuis menu configuration":"May be changed later from menu configuration"),false,130)==KEY_CTRL_F6){
	python_compat(4,contextptr);
	xcas_python_eval=1;
	*logptr(contextptr) << "Micropython interpreter\n";
	Console_FMenu_Init(contextptr);
      }
      else {
	python_compat(1,contextptr);
	*logptr(contextptr) << "Xcas interpreter, Python compatible mode\n";
      }
#else
      PrintMini(x,y,(lang==1)?"quittez Khicas (HOME HOME HOME)":"Leave Khicas (HOME HOME HOME)",TEXT_MODE_NORMAL, _red, COLOR_WHITE);
      if (confirm("Interpreter? OK: Xcas, Back: MicroPython",(lang==1?"Peut se modifier depuis menu configuration":"May be changed later from menu configuration"),false,130)==KEY_CTRL_F6){
	python_compat(4,contextptr);
	xcas_python_eval=1;
	*logptr(contextptr) << "Micropython interpreter\n";
	Console_FMenu_Init(contextptr);
      }
      else {
	python_compat(1,contextptr);
	// fake lexer required to initialize color syntax
	gen g("abs",contextptr);
	*logptr(contextptr) << "Xcas interpreter, Python compatible mode\n";
      }
#endif
      Bdisp_AllClr_VRAM();
#if defined GIAC_SHOWTIME || defined NSPIRE_NEWLIB
      Console_Output("Reglage de l'heure, exemple");
      Console_NewLine(LINE_TYPE_OUTPUT, 1);          
      Console_Output("12,37=>,");
      Console_NewLine(LINE_TYPE_OUTPUT, 1);
#endif
      //menu_about();
      return 0;
    }
    return 1;
  }
#endif

  string extract_name(const char * s){
    int l=strlen(s),i,j;
    for (i=l-1;i>=0;--i){
      if (s[i]=='.')
	break;
    }
    if (i<=0)
      return "f";
    for (j=i-1;j>=0;--j){
      if (s[j]=='\\')
	break;
    }
    if (j<0)
      return "f";
    return string(s+j+1).substr(0,i-j-1);
  }

  // storage==0 (default) ram on numworks, ==1 flash on numworks, ==2 both on numworks, ignored on other calcs
  int giac_filebrowser(char * filename,const char * extension,const char * title,int storage){
#ifdef HP39
    if (strlen(extension)<=3 && extension[0]!='*'){
      char ext[16]="*.";
      strcat(ext,extension);
      return fileBrowser(filename,ext,title);
    }
    return fileBrowser(filename,extension,title);
#endif
    //storage=2; // debug
    // char dbg[]="0\n"; dbg[0] += storage;   console_output(dbg,2);
    const char * filenames[MAX_NUMBER_OF_FILENAMES+1];
#if 1 // def XWASPY
    int n,choix;
    bool isxw=strcmp(extension,"xw")==0,ispy=strcmp(extension,"py")==0;
    if (isxw || ispy){
      n=os_file_browser(filenames,MAX_NUMBER_OF_FILENAMES,"py",storage);
      if (n==0 && ispy) return 0;
      int N=0;
      // isxw: keep only filenames ending with _xw
      // ispy: remove filenames ending with _xw
      const char * fnames[MAX_NUMBER_OF_FILENAMES+1];
      for (int i=0;i<n;++i){
	const char * f=filenames[i];
	//console_output(f,strlen(f));
	f+=strlen(f)-6;
	bool isfxw=strcmp(f,"_xw.py")==0;
	if (isxw?isfxw:!isfxw){
	  fnames[N]=filenames[i];
	  ++N;
	}
      }
      if (isxw){ // add regular .xw extensions
	n=os_file_browser(filenames,MAX_NUMBER_OF_FILENAMES,"xw",storage);
	if (n+N>MAX_NUMBER_OF_FILENAMES)
	  n=MAX_NUMBER_OF_FILENAMES-N;
	for (int i=0;i<n;++i,++N){
	  fnames[N]=filenames[i];
	}
      }
      fnames[N]=0;
      choix=select_item(fnames,title?title:"Scripts");
      if (choix<0 || choix>=N) return 0;
      strcpy(filename,fnames[choix]);
      return choix+1;
    }
    else  {
      int n=os_file_browser(filenames,MAX_NUMBER_OF_FILENAMES,extension,storage);
      if (n==0) return 0;
      int choix=select_item(filenames,title?title:"Scripts");
    }
#else
    int n=os_file_browser(filenames,MAX_NUMBER_OF_FILENAMES,extension,storage);
    if (n==0) return 0;
    int choix=select_item(filenames,title?title:"Scripts");
#endif
    if (choix<0 || choix>=n) return 0;
    strcpy(filename,filenames[choix]);
    return choix+1;
  }
  
  void erase_script(){
    char filename[MAX_FILENAME_SIZE+1];
    int res=giac_filebrowser(filename, "py", "Scripts");
    if (res && do_confirm((lang==1)?"Vraiment effacer":"Really erase?")){
      erase_file(filename);
    }
  }

  int run_script(const char* filename,GIAC_CONTEXT) {
#if 0
    return 1;
#else
    string s;
    load_script(filename,s);
    // execution_in_progress = 1;
    run(s.c_str(),7,contextptr);
    // execution_in_progress = 0;
    if (s.size()>=4){
      if (s[0]=='#' || (s[0]=='d' && s[1]=='e' && s[2]=='f' && s[3]==' '))
	return 2;
      if ( (s[0]=='/' && s[1]=='/') ||
	   (s.size()>8 && s[0]=='f' && (s[1]=='o' || s[1]=='u') && s[2]=='n' && s[3]=='c' && s[4]=='t' && s[5]=='i' && s[6]=='o' && s[7]=='n' && s[8]==' ')
	   )
	return 3;
    }
    return 1;
#endif
  }

  int edit_script(char * fname,GIAC_CONTEXT){
    char fname_[MAX_FILENAME_SIZE+1];
    char * filename=0;
    int res=1;
    if (fname)
      filename=fname;
    else {
      res=giac_filebrowser(fname_, "py", "Scripts",2);
      filename=fname_;
    }
    if(res) {
      string s;
      load_script(filename,s);
      if (s.empty()){
	s=python_compat(contextptr)?((lang==1)?"Prog. Python, sinon taper":"Python prog., for Xcas"):((lang==1)?"Prog. Xcas, sinon taper":"Xcas prog., for Python");
	s += " AC F6 12";
	int k=confirm(s.c_str(),
#ifdef NSPIRE_NEWLIB
		      "enter: Prog, esc: Tortue"
#else
		      "OK: Prog, Back: Tortue"
#endif
		      );
	if (k==-1)
	  return 0;
	if (k==KEY_CTRL_F6)
	  s=python_compat(contextptr)?"from turtle import *\nreset()\n":"\nefface;\n ";
	else
	  s=python_compat(contextptr)?"def "+extract_name(filename)+"(x):\n  \n  return x":"function "+extract_name(filename)+"(x)\nvar j;\n  \n  return x;\nffunction";
      }
      // split s at newlines
      if (edptr==0)
        edptr=new textArea;
      if (!edptr) return -1;
      edptr->elements.clear();
      edptr->clipline=-1;
      edptr->filename=filename;
      edptr->editable=true;
      edptr->changed=false;
      edptr->python=python_compat(contextptr);
      edptr->elements.clear();
      add(edptr,s);
      s.clear();
      edptr->line=0;
      //edptr->line=edptr->elements.size()-1;
      edptr->pos=0;
#ifdef HP39
      edptr->y = 12;
      edptr->lineHeight=14;
      edptr->longlinescut=false;
#else
      edptr->y = 0;
#endif
      int res=doTextArea(edptr,contextptr);
      if (res==KEY_SHUTDOWN)
	return res;
      if (res==-1)
	python_compat(edptr->python,contextptr);
      dConsolePutChar('\x1e');
    }
    return 0;
  }

  void chk_restart(GIAC_CONTEXT){
    drawRectangle(0, 24, LCD_WIDTH_PX, LCD_HEIGHT_PX-24, COLOR_WHITE);
    if (confirm((lang==1)?"Conserver les variables?":"Keep variables?",
#ifdef NSPIRE_NEWLIB
		(lang==1)?"enter: conserver, esc: effacer":"enter: keep, esc: erase"
#else
		(lang==1)?"OK: conserver, Back: effacer":"OK: keep, Back: erase"
#endif
		)==KEY_CTRL_F6)
      do_restart(contextptr);
  }

  void load(GIAC_CONTEXT){
    char filename[MAX_FILENAME_SIZE+1];
    if (giac_filebrowser(filename, "xw", "Sessions",2)){
      if (console_changed==0 ||
	  strcmp(session_filename,"session")==0 ||
	  confirm((lang==1)?"Session courante perdue?":"Current session will be lost",
#ifdef NSPIRE_NEWLIB
		  (lang==1)?"enter: ok, esc: annul":"enter: ok, esc: cancel"
#else
		  (lang==1)?"OK: ok, Back: annul":"OK: ok, Back: cancel"
#endif
		  )==KEY_CTRL_F1){
#ifndef NUMWORKS
	giac::_restart(giac::gen(giac::vecteur(0),giac::_SEQ__VECT),contextptr);
#endif
	restore_session(filename,contextptr);
	clip_pasted=true;
	strcpy(session_filename,remove_path(giac::remove_extension(filename)).c_str());
	static bool ctrl_r=true;
	if (ctrl_r){
#ifdef NSPIRE_NEWLIB
	  confirm((lang==1)?"Taper ctrl puis r pour executer session ":"Type ctrl then r to run session","Enter: OK");
#endif
#ifdef NUMWORKS
	  confirm((lang==1)?"Taper shift EXE pour executer session ":"Type shift then EXE to run session","Enter: OK");
#endif
	  ctrl_r=false;
	}
	Console_Disp(0,contextptr);
	// reload_edptr(session_filename,edptr);
      }     
    }
  }

  bool Console_tooltip(GIAC_CONTEXT){
    if (Current_Line==Last_Line && !Line[Current_Line].readonly && Current_Col>0){
      int y=(Current_Line>10?180:Current_Line*18);
      return tooltip(-1 /* means compute size before cursor*/,y,Cursor.x,Edit_Line,contextptr);
    }
    return false;
  }

  bool console_help_insert(int exec,GIAC_CONTEXT,bool warn=true){
    if (!Edit_Line)
      return false;
    char buf[strlen(Edit_Line)+1];
    strcpy(buf,Edit_Line);
    buf[Line[Current_Line].start_col+Cursor.x]=0;
    int back;
    string s=help_insert(buf,back,exec,contextptr,warn);
    if (s.empty())
      return false;
    for (int i=0;i<back;++i)
      Console_Backspace(contextptr);
    Console_Input(s.c_str());
    Console_Disp(1,contextptr);
    return true;
  }

#ifdef NSPIRE_NEWLIB
  void check_nspire_exam_mode(GIAC_CONTEXT){
    refresh_osscr();
    if (nspire_exam_mode==2){
      // reset
      if (is_cx2)
	*(unsigned *) 0x90140020=8*16;
      else
	*(unsigned *) 0x900a0008=2;
    }
    if (nspire_exam_mode==1){
      // disabled: restore LED state
      // set_exam_mode(3,contextptr);
      exam_mode=0;
    }
  }
#else
  void check_nspire_exam_mode(GIAC_CONTEXT){}
#endif

  int Console_GetKey(GIAC_CONTEXT){
    int key;
    bool keytooltip=false;
    unsigned int i, move_line, move_col;
    char tmp_str[2];
    char *tmp;
    for (;;){
      if (shutdown_state)
        return KEY_SHUTDOWN;
      int keyflag = GetSetupSetting(0x14);
      GetKey(&key);
      if (key==KEY_SHUTDOWN)
        return key;
      if (keytooltip){
        keytooltip=false;
        if (key==KEY_CTRL_EXIT){
          Console_Disp(1,contextptr);
          continue;
        }
        if (Current_Line==Last_Line && Line[Current_Line].start_col+Cursor.x==strlen(Edit_Line) && (key==KEY_CTRL_OK || key==KEY_CHAR_ANS || key==KEY_CTRL_RIGHT)){
          if (console_help_insert(key==KEY_CTRL_RIGHT?KEY_CTRL_OK:key,contextptr,false)){
            Console_Disp(1,contextptr);
            keytooltip=Console_tooltip(contextptr);
            continue;
          }
        }
        if (key==KEY_CTRL_VARS)
          key=KEY_BOOK;	
      }
#ifdef HP39
      if (key==KEY_CTRL_F5){
        handle_f5();
        continue;
      }
      if (key==KEY_CTRL_F6)
        key=KEY_CTRL_MENU;
      if (key==KEY_CTRL_F4){
        char buf[512];
        if (!showCatalog(buf,0,0))
          buf[0]=0;
        return Console_Input((const char*)buf);
      }
#endif      
      bool alph=alphawasactive(&key);
      if (key==KEY_PRGM_ACON)
        Console_Disp(1,contextptr);
      translate_fkey(key);
      if (key==KEY_CTRL_PASTE)
	return Console_Input((const char*) paste_clipboard());
      if ( (key==KEY_CHAR_PLUS || key==KEY_CHAR_MINUS || key==KEY_CHAR_MULT || key==KEY_CHAR_DIV) && Current_Line<Last_Line-1){
	console_line * nxt=&Line[Current_Line];
	if (strncmp((const char *)nxt->str,"parameter([",11)==0)
	  Console_MoveCursor(CURSOR_UP);
	nxt=&Line[Current_Line+1];
	if (strncmp((const char *)nxt->str,"parameter([",11)==0){
	  giac::gen g((const char *)nxt->str,contextptr);
	  if (g.is_symb_of_sommet(giac::at_parameter)){
	    g=g._SYMBptr->feuille;
	    if (g.type==giac::_VECT && g._VECTptr->size()>=5){
	      giac::vecteur & v=*g._VECTptr;
	      for (int i=1;i<v.size();++i)
		v[i]=evalf_double(v[i],1,contextptr);
	      if (v[0].type==giac::_IDNT && v[1].type==giac::_DOUBLE_ && v[2].type==giac::_DOUBLE_ && v[3].type==giac::_DOUBLE_ && v[4].type==giac::_DOUBLE_){
		std::string s("assume(");
		s += v[0]._IDNTptr->id_name;
		s += "=[";
		int val=1;
		if (key==KEY_CHAR_MINUS) val=-1;
		if (key==KEY_CHAR_MULT) val=5;
		if (key==KEY_CHAR_DIV) val=-5;
		s += giac::print_DOUBLE_(v[3]._DOUBLE_val + val*v[4]._DOUBLE_val,contextptr);
		s += ',';
		s += giac::print_DOUBLE_(v[1]._DOUBLE_val,contextptr);
		s += ',';
		s += giac::print_DOUBLE_(v[2]._DOUBLE_val,contextptr);
		s += ',';
		s += giac::print_DOUBLE_(v[4]._DOUBLE_val,contextptr);
		s += "])";
		return Console_Eval(s.c_str(),contextptr);
	      }
	    }
	  }
	}
      }
      if (key==KEY_CHAR_ACCOLADES || key==KEY_CHAR_CROCHETS){
	Console_Input(key==KEY_CHAR_ACCOLADES?"{}":"[]");
	Console_MoveCursor(CURSOR_LEFT);
	Console_Disp(1,contextptr);
	continue;	
      }
      if ( (key >= ' ' && key <= '~' )
	   // (key>='0' && key<='9')|| (key >= 'A' && key <= 'Z') || (key >= 'a' && key <= 'z')
	   ){
	tmp_str[0] = key;
	tmp_str[1] = '\0';
	Console_Input(tmp_str);
	Console_Disp(1,contextptr);
	// tooltip
	keytooltip=Console_tooltip(contextptr);
	continue;
      }
      if (key == KEY_CTRL_F5 || key==KEY_EQW_TEMPLATE || key==KEY_CTRL_F4 || ( (key==KEY_CTRL_RIGHT || key==KEY_CTRL_LEFT) && Current_Line<Last_Line) ){
	int l=Current_Line;
	bool graph=strcmp((const char *)Line[l].str,"Graphic object")==0;
	if (graph && l>0) --l;
	char buf[giacmax(512,strlen((const char *)Line[l].str+1))];
	strcpy(buf,(const char *)Line[l].str);
	int ret=(alph || key==KEY_CTRL_RIGHT || key==KEY_CTRL_F4) ?textedit(buf,512,false,contextptr):eqws(buf,graph,contextptr);
	if (ret==KEY_SHUTDOWN)
	  return ret;
	if (ret){
	  if (Current_Line==Last_Line){
	    Console_Clear_EditLine();
	    return Console_Input((const char *)buf);
	  }
	  else {
#if 1
	    if (Line[l].type==LINE_TYPE_INPUT && l<Last_Line-1 && Line[l+1].type==LINE_TYPE_OUTPUT)
	      return Console_Eval(buf,contextptr);
	    else {
	      console_free(Line[l].str);
	      Line[l].str=(char*)console_malloc(strlen(buf)+1);
	      Line[l].disp_len = Console_GetDispLen(Line[l].str);
	      strcpy((char *)Line[l].str,buf);
	    }
#else
	    int x=editline_cursor;
	    move_line = Last_Line - Current_Line;
	    for (i = 0; i <=move_line; i++) Console_MoveCursor(CURSOR_DOWN);
	    Cursor.x=x;
	    return Console_Input((const char *)buf);
#endif
	  }	  
	}
	Console_Disp(1,contextptr);
	continue;
      }
      if (key==KEY_CTRL_S || key==KEY_CTRL_T){
	giac::gen g=sheet(contextptr);
	if (g.type==_INT_ && g.val==KEY_SHUTDOWN)
	  return KEY_SHUTDOWN;
	if (g.type==_VECT)
	  return Console_Input(g.print(contextptr).c_str());
	Console_Disp(1,contextptr);
	continue;
      }
      if (key==KEY_SAVE){
	save(session_filename,contextptr);
	console_changed=false;
	console_disp_status(contextptr);
	continue;
      }
      if (key==KEY_LOAD){
	load(contextptr);
	Console_Disp(1,contextptr);
	continue;
      }
      if (key==KEY_CTRL_MENU){
#if 1
	Menu smallmenu;
#if defined NUMWORKS && defined DEVICE
	smallmenu.numitems=20;
#else
	smallmenu.numitems=17;
#endif
	MenuItem smallmenuitems[smallmenu.numitems];
      
	smallmenu.items=smallmenuitems;
	smallmenu.height=MENUHEIGHT;
	smallmenu.scrollbar=1;
	smallmenu.scrollout=1;
	//smallmenu.title = "KhiCAS";
	// smallmenuitems[2].text = (char*)(isRecording ? "Stop Recording" : "Record Script");
	while(1) {
	  // moved inside the loop because lang might change
#ifdef NUMWORKS
	  smallmenuitems[0].text = (char*)"Applications (shift ANS)";
#else
	  smallmenuitems[0].text = (char*)"Applications (shift doc)";
#endif
	  string sess=(lang==1)?"Enregistrer ":"Save ";
	  sess += session_filename;
	  smallmenuitems[1].text = (char *) (sess.c_str());
	  smallmenuitems[2].text = (char *) ((lang==1)?"Enregistrer sous":"Save session as");
	  if (nspire_exam_mode==2) smallmenuitems[1].text = (char*)(lang==1?"Sauvegarde desactivee":"Saving disabled");
	  if (exam_mode || nspire_exam_mode==2)
	    smallmenuitems[2].text = (char *) "";
	  smallmenuitems[3].text = (char*) ((lang==1)?"Charger session":"Load session");
	  smallmenuitems[4].text = (char*)((lang==1)?"Nouvelle session":"New session");
	  smallmenuitems[5].text = (char*)((lang==1)?"Executer session":"Run session");
	  smallmenuitems[6].text = (char*)((lang==1)?"Editeur script":"Script editor");
	  smallmenuitems[7].text = (char*)((lang==1)?"Ouvrir script":"Open script");
	  smallmenuitems[8].text = (char*)((lang==1)?"Executer script":"Run script");
	  smallmenuitems[9].text = (char*)((lang==1)?"Effacer historique (0)":"Clear history");
	  smallmenuitems[10].text = (char*)((lang==1)?"Effacer script (e^)":"Clear script");
	  smallmenuitems[11].text = (char*)"Configuration/examen (ln)";
	  smallmenuitems[12].text = (char *) ((lang==1)?"Aide interface (log)":"Shortcuts");
	  smallmenuitems[13].text = (char*)((lang==1)?"Editer matrice (i)":"Matrix editor");
	  smallmenuitems[14].text = (char*) ((lang==1)?"Creer parametre (,)":"Create slider (,)");
	  smallmenuitems[15].text = (char*) ((lang==1)?"A propos (x^y)":"About");
#ifdef NSPIRE_NEWLIB
	  smallmenuitems[16].text = (char*) ((lang==1)?"Quitter (menu)":"Quit");
#else
	  smallmenuitems[16].text = (char*) ((lang==1)?"Quitter (HOME)":"Quit");
#endif
#if defined NUMWORKS && defined DEVICE
	  smallmenuitems[16].text = (char*) ((lang==1)?"Reboot autre firmware":"Reboot alt. firmware");
	  smallmenuitems[17].text = (char*) ((lang==1)?"Sauvegarde multi-firmware":"Backup multi-firmware");
	  smallmenuitems[18].text = (char*) ((lang==1)?"Restauration multi-firmware":"Restore multi-firmware");
	  smallmenuitems[19].text = (char*) ((lang==1)?"Quitter (HOME)":"Quit");
#endif
	  if (exam_mode)
	    smallmenuitems[16].text = (char*)((lang==1)?"Quitter le mode examen":"Quit exam mode");
	  if (nspire_exam_mode==2)
	    smallmenuitems[16].text = (char*)((lang==1)?"Relancer le mode examen":"Restart exam mode");
	  if (shutdown_state)
	    return KEY_SHUTDOWN;
	  int sres = doMenu(&smallmenu);
	  if(sres == MENU_RETURN_SELECTION || sres==KEY_CTRL_EXE) {
#if defined NUMWORKS && defined DEVICE
	    if (smallmenu.selection==17){
	      int b1=is_valid(0),b2=is_valid(1),b3=is_valid(2);
	      const char * boot_tab[]={b1?"Slot 1":"Invalid slot 1",b2?"Slot 2":"Invalid slot 2",b3?"Slot 3":"Invalid slot 3","Bootloader","Cancel/Annuler",0};
	      int choix=select_item(boot_tab,"Reboot firmware",true);
	      if (choix>=0 && choix<=2 && !is_valid(choix)){
		break;
	      }
	      if (choix>=0 && choix<=3){
		if (confirm(lang==1?"Les donnees seront perdues!":"Data will be lost!",lang==1?"OK/Back?":"OK/Back?")==KEY_CTRL_F1)
		  boot_firmware(choix);
	      }
	      break;
	    }
	    if (smallmenu.selection==18){
	      if (!save_backup(1))
		confirm(lang==1?"Rien a sauvegarder":"Nothing to save","OK?");
	      break;
	    }
	    if (smallmenu.selection==19){
	      if (restore_backup(3)){
		if (confirm(lang==1?"Les donnees actuelles vont etre effacees":"Current data will be erased!","OK/Back?")==KEY_CTRL_F1){
		  confirm(restore_backup(1)?"Success!":"Failure!","OK?");
		}
	      }
	      break;
	    }
#endif
	    if (smallmenu.selection==smallmenu.numitems){
	      if (nspire_exam_mode==2)
		check_nspire_exam_mode(contextptr);
	      if (!exam_mode)
		return KEY_CTRL_MENU;
	      leave_exam_mode(contextptr);
	      break;
	    }
	    const char * ptr=0;
	    if (smallmenu.selection==1){
	      key=KEY_SHIFT_ANS;
	      break;
	    }
	    if (smallmenu.selection==2){
	      if (strcmp(session_filename,"session")==0)
		smallmenu.selection=3;
	      else {
		save(session_filename,contextptr);
		break;
	      }
	    }
	    if (smallmenu.selection==3 && !exam_mode && nspire_exam_mode!=2){
	      char buf[270];
	      if (get_filename(buf,".xw")){
		save(buf,contextptr);
		string fname(remove_path(giac::remove_extension(buf)));
		strcpy(session_filename,fname.c_str());
		if (edptr)
		  edptr->filename=fname+".py";
	      }
	      break;
	    }
	    if (smallmenu.selection==4){
	      load(contextptr);
	      break;
	    }
	    if (0 && smallmenu.selection==5) {
	      // FIXME: make a menu catalog?
	      char buf[512];
	      if (doCatalogMenu(buf,(char*)"CATALOG",0,contextptr))
		return Console_Input((const char *)buf);
	      break;
	    }
	    if (smallmenu.selection==5) {
#if 0 // SIMU
	      // simulator debug load calc.nws into scriptstore
	      void * ptr=ion_storage();
	      FILE * f=fopen("calc.nws","r");
	      if (f){
		fread(ptr,1,32768,f);
		fclose(f);
	      }
#endif
	      if (exam_mode){
		if (do_confirm((lang==1)?"Tout effacer?":"Really clear?")){
		  Console_Init(contextptr);
		  Console_Clear_EditLine();
		  giac::_restart(giac::gen(giac::vecteur(0),giac::_SEQ__VECT),contextptr);
		}
	      }
	      else {
		char filename[MAX_FILENAME_SIZE+1];
		drawRectangle(0, 0, LCD_WIDTH_PX, LCD_HEIGHT_PX, COLOR_WHITE);
		if (get_filename(filename,".xw")){
		  if (console_changed==0 ||
		      strcmp(session_filename,"session")==0 ||
		      confirm((lang==1)?"Session courante perdue?":"Current session will be lost",
#ifdef NSPIRE_NEWLIB
			      (lang==1)?"enter: annul, esc: ok":"enter: cancel, esc: ok"
#else
			      (lang==1)?"OK: annul, Back: ok":"OK: cancel, Back: ok"
#endif
			      )==KEY_CTRL_F6){
		    clip_pasted=true;
		    Console_Init(contextptr);
		    Console_Clear_EditLine();
		    giac::_restart(giac::gen(giac::vecteur(0),giac::_SEQ__VECT),contextptr);
		    std::string s(remove_path(giac::remove_extension(filename)));
		    strcpy(session_filename,s.c_str());
		    reload_edptr(session_filename,edptr,contextptr);
		  }
		}
	      }
	      break;
	    }
	    if (smallmenu.selection==6) {
	      run_session(0,contextptr);
	      break;
	    }
	    if (smallmenu.selection==7) {
	      if (!edptr || merge_area(edptr->elements).size()<2)
		edit_script((char *)(giac::remove_extension(session_filename)+".py").c_str(),contextptr);
	      else
		doTextArea(edptr,contextptr);
	      break;
	    }
	    if (smallmenu.selection==8) {
	      char filename[MAX_FILENAME_SIZE+1];
	      drawRectangle(0, 0, LCD_WIDTH_PX, LCD_HEIGHT_PX-8, COLOR_WHITE);
	      if (giac_filebrowser(filename, "py", "Scripts",2))
		edit_script(filename,contextptr);
	      break;
	    }
	    if (smallmenu.selection==9) {
	      char filename[MAX_FILENAME_SIZE+1];
	      drawRectangle(0, 0, LCD_WIDTH_PX, LCD_HEIGHT_PX-8, COLOR_WHITE);
	      if (giac_filebrowser(filename, "py", "Scripts",2))
		run_script(filename,contextptr);
	      Console_Clear_EditLine();
	      break;
	    }
	    if(smallmenu.selection == 10) {
	      chk_restart(contextptr);
	      Console_Init(contextptr);
	      Console_Clear_EditLine();
	      break;
	    }
	    if (smallmenu.selection==11){
	      erase_script();
	      break;
	    }
	    if (smallmenu.selection == 12){
	      menu_setup(contextptr);
	      continue;
	    }
	    if(smallmenu.selection == 13 ||smallmenu.selection == 16 ) {
	      textArea text;
	      text.editable=false;
	      text.clipline=-1;
	      text.title = smallmenuitems[smallmenu.selection-1].text;
	      add(&text,smallmenu.selection==13?((lang==1)?shortcuts_fr_string:shortcuts_en_string):((lang==1)?apropos_fr_string:apropos_en_string));
	      doTextArea(&text,contextptr);
	      continue;
	    } 
	    if (smallmenu.selection==14){
	      drawRectangle(0, 0, LCD_WIDTH_PX, LCD_HEIGHT_PX-8, COLOR_WHITE);
	      if (ptr=input_matrix(false,contextptr)) {
		return Console_Input((const char *)ptr);
	      }
	      break;
	    }
	    if (smallmenu.selection == 15){
	      static char curname='a';
	      string s=inputparam(curname,1,contextptr);
	      if (!s.empty()){
		++curname;
		return Console_Input((const char *)s.c_str());
	      }
	      continue;
	    }
	  }
	  break;
	} // end while(1)
	if (key!=KEY_SHIFT_ANS){
	  Console_Disp(1,contextptr);
	  return CONSOLE_SUCCEEDED;
	}
#else
	char filename[MAX_FILENAME_SIZE+1];
	//drawRectangle(0, 24, LCD_WIDTH_PX, LCD_HEIGHT_PX-24, COLOR_WHITE);
	if (get_filename(filename))
	  edit_script(filename,contextptr);
	//edit_script(0);
	return CONSOLE_SUCCEEDED;
#endif
      }
      if (key==KEY_SHIFT_ANS || key==KEY_CTRL_SD){ // 3rd party app
	int res=khicas_addins_menu(contextptr);
	if (res==KEY_CTRL_MENU)
	  return res;
	Console_Disp(1,contextptr);
	return CONSOLE_SUCCEEDED;
      }
      if ( (key >= KEY_CTRL_F1 && key <= KEY_CTRL_F6) ||
	   (key >= KEY_CTRL_F7 && key <= KEY_CTRL_F14) 
	   ){
	Console_FMenu(key,contextptr);
	Console_Disp(1,contextptr);
	keytooltip=Console_tooltip(contextptr);
	continue;
      }
      if (key==KEY_CTRL_PAGEDOWN){
	int j=0;
	for (int i=0;i<10;++i)
	  j=Console_MoveCursor(CURSOR_DOWN);
	return j;
      }
      if (key==KEY_CTRL_PAGEUP){
	int j=0;
	for (int i=0;i<10;++i)
	  j=Console_MoveCursor(CURSOR_UP);
	return j;
      }
      if (key == KEY_CTRL_UP)
	return Console_MoveCursor(alph?CURSOR_ALPHA_UP:CURSOR_UP);
      if (key == KEY_CTRL_DOWN || key=='\t'
	  // FIREBIRDEMU
	  || key==KEY_BOOK
	  ){
	if (Current_Line==Last_Line && !Line[Current_Line].readonly && Current_Col>0){
	  console_help_insert(0,contextptr);
	  Console_Disp(1,contextptr);	  
	  continue;
	}
	return Console_MoveCursor(alph?CURSOR_ALPHA_DOWN:CURSOR_DOWN);
      }
      //if (key == KEY_CTRL_PAGEUP)  return Console_MoveCursor(CURSOR_ALPHA_UP);
      //if (key == KEY_CTRL_PAGEDOWN) return Console_MoveCursor(CURSOR_ALPHA_DOWN);
      if (key == KEY_CTRL_LEFT)
	Console_MoveCursor(CURSOR_LEFT);
      if (key == KEY_CTRL_RIGHT)
	Console_MoveCursor(CURSOR_RIGHT);
      if (key == KEY_SHIFT_LEFT)
	Console_MoveCursor(CURSOR_SHIFT_LEFT);
      if (key == KEY_SHIFT_RIGHT)
	Console_MoveCursor(CURSOR_SHIFT_RIGHT);
      if (key == KEY_SHIFT_RIGHT || key == KEY_SHIFT_LEFT ||
	  key == KEY_CTRL_RIGHT || key == KEY_CTRL_LEFT){
	Console_Disp(0,contextptr);
	continue;
      }
      if (key == KEY_CTRL_EXIT){
	if (Last_Line==Current_Line){
	  if (!edptr)
	    edit_script((char *)(giac::remove_extension(session_filename)+".py").c_str(),contextptr);
	  else {
#ifdef HP39
	    edptr->y=12;
#else
	    edptr->y=0;
#endif
	    doTextArea(edptr,contextptr);
	  }
	  Console_Disp(1,contextptr);
	}
	else {
	  move_line = Last_Line - Current_Line;
	  for (i = 0; i <= move_line; i++) Console_MoveCursor(CURSOR_DOWN);
	}
	return CONSOLE_SUCCEEDED;
      }
      if (key == KEY_CTRL_AC)
	{
	  if (Line[Current_Line].readonly){
	    move_line = Last_Line - Current_Line;
	    for (i = 0; i <= move_line; i++) Console_MoveCursor(CURSOR_DOWN);
	    return CONSOLE_SUCCEEDED;
	  }
	  if (Edit_Line[0]=='\0'){
	    //return Console_Input((const char *)"restart");
	    chk_clearscreen(contextptr);
	    continue;
	  }
	  Edit_Line[0] = '\0';
	  Line[Current_Line].start_col = 0;
	  Line[Current_Line].type = LINE_TYPE_INPUT;
	  Line[Current_Line].disp_len = 0;
	  Cursor.x = 0;
	  return CONSOLE_SUCCEEDED;
	}

      if (key == KEY_CTRL_INS) {
        if (Current_Line<Last_Line){
          Console_Insert_Line();
          Console_Insert_Line();
        }
        else {
          int c=giac::chartab();
          char s[2]={0};
          if (c>32 && c<127) s[0]=char(c);
          Console_Input(s);
        }
        //Console_Input((const char*)":=");
        Console_Disp(1,contextptr);
        continue;
      }
      if (key==KEY_AFFECT){
	Console_Input((const char*)":=");
	Console_Disp(1,contextptr);
	continue;
      }	
      if (key==KEY_CTRL_D){
	Console_Input((const char*)"debug(");
	Console_Disp(1,contextptr);
	continue;
      }	
      if (key == KEY_CTRL_SETUP) {
	menu_setup(contextptr);
	Console_Disp(1,contextptr);
	continue;
      }

      if (key == KEY_CTRL_EXE || key==KEY_CTRL_OK){
	if (Current_Line == Last_Line)
	  {
	    return Console_NewLine(LINE_TYPE_INPUT, 1);
	  }
      }
      if (key == KEY_CTRL_DEL){
	Console_Backspace(contextptr);
	Console_Disp(1,contextptr);
	keytooltip=Console_tooltip(contextptr);
	continue;	
      }
      if (key == KEY_CTRL_R && (Current_Line!=Last_Line || Cursor.x==0)){
	run_session(0,contextptr);
	return 0;
      }
      if (key == KEY_CTRL_CLIP){
	copy_clipboard((const char *)Line[Current_Line].str,true);
      }
      if (key==KEY_CTRL_EXE || key==KEY_CTRL_OK){
	tmp = Line[Current_Line].str;
      
#if 1
	int x=editline_cursor;
	move_line = Last_Line - Current_Line;
	for (i = 0; i <= move_line; i++) Console_MoveCursor(CURSOR_DOWN);
	Cursor.x=x;
	if (Cursor.x>COL_DISP_MAX)
	  Line[Last_Line].start_col=Cursor.x-COL_DISP_MAX;
#else
	move_line = Last_Line - Current_Line;
	for (i = 0; i <= move_line; i++) Console_MoveCursor(CURSOR_DOWN);
	move_col = Line[Current_Line].disp_len - Current_Col;
	for (i = 0; i <= move_col; i++) Console_MoveCursor(CURSOR_RIGHT);
#endif
	return Console_Input(tmp);
      }
      const char * ptr=keytostring(key,keyflag,0,contextptr);
      if (ptr){
	Console_Input((const char *)ptr);
	Console_Disp(1,contextptr);
	keytooltip=Console_tooltip(contextptr);
	continue;
      }
    
    }
    return CONSOLE_NO_EVENT;
  }

  int Console_FMenu(int key,GIAC_CONTEXT){
    const char * s=console_menu(key,fmenu_cfg,0),*ptr=0;
    if (!s){
      //cout << "console " << unsigned(s) << endl;
      return CONSOLE_NO_EVENT;
    }
    if (strcmp("matrix(",s)==0 && (ptr=input_matrix(false,contextptr)) )
      s=ptr;
    if (strcmp("makelist(",s)==0 && (ptr=input_matrix(true,contextptr)) )
      s=ptr;
    return Console_Input((const char *)s);
  }

  const char * console_menu(int key,int active_app){
    return console_menu(key,fmenu_cfg,active_app);
  }

  const char * console_menu(int key,char* cfg_,int active_app){
    if (key>=KEY_CTRL_F7 && key<=KEY_CTRL_F14) key-=900;
    char * cfg=cfg_;
    int i, matched = 0;
    const char * ret=0;
    const int maxentry_size=64;
    static char console_buf[maxentry_size];
    char temp[maxentry_size],menu1[maxentry_size],menu2[maxentry_size],menu3[maxentry_size],menu4[maxentry_size],menu5[maxentry_size],menu6[maxentry_size],menu7[maxentry_size],menu8[maxentry_size];
    char * tabmenu[8]={menu1,menu2,menu3,menu4,menu5,menu6,menu7,menu8};
    struct FMenu entry = {0,tabmenu,0};
    // char* cfg = (char *)memory_load((char *)"\\\\fls0\\FMENU.cfg");

    while (*cfg) {
      //Get each line
      for(i=0; i<maxentry_size-1 && *cfg && *cfg!='\r' && *cfg!='\n'; i++, cfg++) {
	temp[i] = *cfg;
      }
      temp[i]=0;
      //If starting by 'F' followed by the right number, start filling the structure.
      if (temp[0] == 'F' && temp[1]==(key-KEY_CTRL_F1)+'1'){
	matched = 1;
	continue;
      }
      if (temp[0] == 'F' && temp[1]!=(key-KEY_CTRL_F1)+'0'){
	matched = 0;
	continue;
      }
      //Fill the structure
      if (matched && temp[0] && entry.count<8) {
	strcpy(tabmenu[entry.count], temp);
	entry.count++;
      }
      cfg++;
    }
    if(entry.count > 0) {
      ret = Console_Draw_FMenu(key, &entry,cfg,active_app);
      // cout << "console0 " << (unsigned) ret << endl;
      if (!ret) return ret;
      if (!strcmp("periodic_table",ret)){
	const char * name,*symbol;
	char protons[32],nucleons[32],mass[32],electroneg[32];
	int res=periodic_table(name,symbol,protons,nucleons,mass,electroneg);
	if (!res)
	  return 0;
	char * ptr=console_buf;
	if (res & 1)
	  ptr=strcpy(ptr,name)+strlen(ptr);
	if (res & 2){
	  if (res & 1)
	    ptr=strcpy(ptr,",")+strlen(ptr);
	  ptr=strcpy(ptr,symbol)+strlen(ptr);
	}
	if (res & 4){
	  if (res&3)
	    ptr=strcpy(ptr,",")+strlen(ptr);
	  ptr=strcpy(ptr,protons)+strlen(ptr);
	}
	if (res & 8){
	  if (res&7)
	    ptr=strcpy(ptr,",")+strlen(ptr);
	  ptr=strcpy(ptr,nucleons)+strlen(ptr);
	}
	if (res & 16){
	  if (res&15)
	    ptr=strcpy(ptr,",")+strlen(ptr);
	  ptr=strcpy(ptr,mass+2)+strlen(ptr);
	}
	if (res & 32){
	  if (res&31)
	    ptr=strcpy(ptr,",")+strlen(ptr);
	  ptr=strcpy(ptr,electroneg+4)+strlen(ptr);
	}
      }
      else
	strcpy(console_buf,ret);
      return console_buf;
    }
    return 0;
  }

  char *Console_Make_Entry(const char* str)
  {
    char* entry = NULL;
    entry = (char*)calloc((strlen((const char *)str)+1), sizeof(char*));
    if(entry) memcpy(entry, (const char *)str, strlen((const char *)str)+1);

    return entry;
  }

#ifdef HP39
void ck_getkey(int * i){
  GetKey(i);
}
// Draws and runs the asked for menu.
const char *Console_Draw_FMenu(int key, struct FMenu *menu, char *cfg, int active_app)
{
  int i, nb_entries = 0, selector = 0, position_number, position_x, ret, longest = 0;
  int input_key;
  char quick[] = "*: ";
  int quick_len = 2;
  char **entries;
  DISPBOX box;

  position_number = key - KEY_CTRL_F1;
  if (position_number < 0 || position_number > 5)
    position_number = 4;

  entries = menu->str;
  nb_entries = menu->count;

  for (i = 0; i < nb_entries; i++)
    if (strlen(entries[i]) > longest)
      longest = strlen(entries[i]);

  position_x = 21 * position_number;
  if (position_x + longest * 8 + quick_len * 8 > 115)
    position_x = 115 - longest * 8 - quick_len * 8;

  box.left = position_x;
  box.right = position_x + longest * 8 + quick_len * 8 + 6;
  box.bottom = 113;  
  box.top = box.bottom - nb_entries * 14; 
  //giac::confirm((giac::print_INT_(box.left)+" "+giac::print_INT_(box.top)).c_str(),(giac::print_INT_(box.right)+" "+giac::print_INT_(box.bottom)).c_str(),false);
  drawRectangle(box.left,box.top,box.right-box.left+1,box.bottom-box.top+1,_WHITE);
  giac::freeze=true; // temporary workaround
  giac::draw_line(box.left, box.bottom, box.left, box.top,0,contextptr);
  giac::draw_line(box.right, box.bottom, box.right, box.top,0,contextptr);
  giac::freeze=false;

  // If the cursor is flashing on the opening box, disable it. //!!!!!!
  // if (((Cursor.x * (256 / 21) < box.right && Cursor.x * (256 / 21) > box.left)) && ((Cursor.y * (128 / 8) < box.bottom) && (Cursor.y * (128 / 8) > box.top))) Cursor_SetFlashOff();

  for (;;)
  {
    for (i = 0; i < nb_entries; i++)
    {
      quick[0] = '0' + (i + 1);
      PrintMini(3 + position_x, box.bottom - 14 * (i + 1), quick, MINI_OVER); //!!!!!
      PrintMini(3 + position_x + quick_len * 8, box.bottom - 14 * (i + 1), entries[i], MINI_OVER); //!!!!!
    }
    PrintMini(3 + position_x + quick_len * 8, box.bottom - 14 * (selector + 1), entries[selector], MINI_REV); //!!!!!
    ck_getkey((int *)&input_key);
    if (input_key == KEY_CTRL_EXIT || input_key == KEY_CTRL_AC)
      return 0;
    if (input_key == KEY_CTRL_UP && selector < nb_entries - 1)
      selector++;
    if (input_key == KEY_CTRL_DOWN && selector > 0)
      selector--;
    const char *howto = "", *syntax = "", *related = "", *examples = "";
    if (0
      //input_key == KEY_CTRL_RIGHT && giac::has_static_help(entries[selector], 1, howto, syntax, examples, related)
        )
    {
      unsigned int key;
      //PopUpWin(6);
      PrintMini(12, 6, howto, MINI_OVER);
      PrintMini(12, 14, entries[selector], MINI_OVER);
      PrintMini(16, 22, syntax, MINI_OVER);
      PrintMini(12, 30, "Example (EXE)", MINI_OVER);
      PrintMini(12, 38, examples, MINI_OVER);
      PrintMini(12, 46, "See also", MINI_OVER);
      PrintMini(12, 54, related, MINI_OVER);
      ck_getkey((int *)&key);
      if (key == KEY_CTRL_EXE)
        return examples;
      Console_Disp(1,contextptr);
      continue;
    }

    if (input_key == KEY_CTRL_EXE)
      return entries[selector];

    if (input_key >= KEY_CHAR_1 && input_key < KEY_CHAR_1 + nb_entries)
      return entries[input_key - KEY_CHAR_1];

    translate_fkey(input_key);

    if (active_app == 0 &&
        ((input_key >= KEY_CTRL_F1 && input_key <= KEY_CTRL_F6) ||
         (input_key >= KEY_CTRL_F7 && input_key <= KEY_CTRL_F12)))
    {
      Console_Disp(1,contextptr);
      key = input_key;
      return console_menu(key, cfg, active_app);
    }
  } // end while input_key!=EXE/EXIT

  return 0; // never reached
}

#else // hp39
  void PrintMini(int x,int y,const char * s,int mode){
    x *=3;
    y *=3;
    PrintMini(x,y,(char *)s,mode,COLOR_BLACK, COLOR_WHITE);
  }

  //Draws and runs the asked for menu.
  const char * Console_Draw_FMenu(int key, struct FMenu* menu,char * cfg,int active_app)
  {
    int i, nb_entries = 0, selector = 0, position_number, position_x, ret, longest = 0;
    int input_key;
    char quick[] = "*: ";
    int quick_len = 2;
    char **entries;
    DISPBOX box,box3;
    
    position_number = key - KEY_CTRL_F1;
    if (position_number<0 || position_number>=5)
      position_number=4;
    
    entries  = menu->str;
    nb_entries = menu->count;
    
    for(i=0; i<nb_entries; i++)
      if(strlen(entries[i]) > longest) longest = strlen(entries[i]);
    if (longest>15)
      longest=15;
    // screen resolution Graph90 384x(216-24), Graph35 128x64
    // factor 3x3
    position_x = 17*position_number;
    if(position_x + longest*4 + quick_len*4 > 115) position_x = 115 - longest*4 - quick_len*4;
    
    box.left = position_x;
    box.right = position_x + longest*4 + quick_len*4  + 6;
    box.bottom = 63-7;
    box.top = 63-7-nb_entries*7;
    box3.left=3*box.left;
    box3.right=3*box.right;
    box3.bottom=3*box.bottom+22;
    box3.top=3*box.top+20;
  
    drawRectangle(box3.left,box3.top,box3.right-box3.left,box3.bottom-box3.top,COLOR_WHITE);
    drawLine(box3.left, box3.top, box3.right, box3.top,COLOR_BLACK);
    drawLine(box3.left, box3.bottom, box3.left, box3.top,COLOR_BLACK);
    drawLine(box3.right, box3.bottom, box3.right, box3.top,COLOR_BLACK);
    drawLine(box3.left, box3.bottom, box3.right, box3.bottom,COLOR_BLACK);
    
    // Cursor_SetFlashOff();
    
    for (;;){
      for(i=0; i<nb_entries; i++) {
	quick[0] = '0'+(i+1);
	PrintMini(3+position_x, box.bottom-7*i, quick, 0);
	PrintMini(3+position_x+quick_len*4, box.bottom-7*i, entries[i], 0);
      }
      PrintMini(3+position_x+quick_len*4,box.bottom-7*selector, entries[selector], 4);
      GetKey(&input_key);
      if (input_key==KEY_PRGM_ACON) Console_Disp(1,0);
      if (input_key == KEY_CTRL_EXIT || input_key==KEY_CTRL_AC) return 0;
      if (input_key == KEY_CTRL_UP && selector < nb_entries-1) selector++;	
      if (input_key == KEY_CTRL_DOWN && selector > 0) selector--;
      
      if (input_key == KEY_CTRL_EXE || input_key==KEY_CTRL_OK) return entries[selector];
      
      if (input_key >= KEY_CHAR_1 && input_key < KEY_CHAR_1 + nb_entries) return entries[input_key-KEY_CHAR_1];
      
      translate_fkey(input_key);
      
      if ( active_app==0 &&
	   ((input_key >= KEY_CTRL_F1 && input_key <= KEY_CTRL_F6) ||
	    (input_key >= KEY_CTRL_F7 && input_key <= KEY_CTRL_F16) )
	   ){
	Console_Disp(1,0);
	key=input_key;
	return console_menu(key,cfg,active_app);
      }
    } // end while input_key!=EXE/EXIT

    return 0; // never reached
  }
#endif

  void Console_Free(){
    for (int i = 0; i < _LINE_MAX; i++){
      if (Line[i].str){
	if (Line[i].str==Edit_Line)
	  Edit_Line=0;
	console_free(Line[i].str);
	Line[i].str=0;
      }
    }
    if (Edit_Line)
      console_free(Edit_Line);
    if (Line){
      delete [] Line;
      Line = 0;
    }
  }

  int Console_Init(GIAC_CONTEXT){
    console_changed=1;
    int i;
    if (!Line){
      Line=new console_line[_LINE_MAX];
      for (i = 0; i < _LINE_MAX; i++){
	Line[i].str=0;
      }
    }
    Start_Line = 0;
    Last_Line = 0;

    for (i = 0; i < _LINE_MAX; i++){
      if (Line[i].str){
	if (Line[i].str==Edit_Line)
	  Edit_Line=0;
	console_free(Line[i].str);
	Line[i].str=0;
      }
      Line[i].readonly = 0;
      Line[i].type = LINE_TYPE_INPUT;
      Line[i].start_col = 0;
      Line[i].disp_len = 0;
    }
    if (Edit_Line)
      console_free(Edit_Line);
    if ((Edit_Line = (char *)console_malloc(EDIT_LINE_MAX + 1)) == NULL) return CONSOLE_MEM_ERR;
    Edit_Line[0]=0;
    Line[0].str = Edit_Line;

    Cursor.x = 0;
    Cursor.y = 0;

    Case = LOWER_CASE;

    /*for(i = 0; i < 6; i++) {
      FMenu_entries[i].name = NULL;
      FMenu_entries[i].count = 0;
      }*/

    Console_FMenu_Init(contextptr);

    return CONSOLE_SUCCEEDED;
  }

  // Loads the FMenus' data into memory, from a cfg file
  void Console_FMenu_Init(GIAC_CONTEXT)
  {
    char temp[32] = {'\0'};
#if 0
    if (!fmenu_cfg){
      fmenu_cfg = (char *)conf_standard;
      std::string cfg_s;
      // Does the file exists ?
      if (load_script((char*)"FMENU.cfg",cfg_s)){
	char * ptr=new char[cfg_s.size()+1];
	strcpy(ptr,cfg_s.c_str());
	fmenu_cfg=(char *)ptr;
      }
      if(!fmenu_cfg) {
	save_script((const char *)"FMENU.cfg",conf_standard);
	fmenu_cfg = (char *)conf_standard;
      }
    }
#else
    if (xcas_python_eval==1){
      fmenu_cfg=(char *)python_conf_standard;
    }
    else {
      fmenu_cfg=(char *)conf_standard;
    }
#endif
    const char *cfg=fmenu_cfg;
    while(*cfg) {
      //Get each line
      int i;
      for(i=0; i<20 && *cfg && *cfg!='\r' && *cfg!='\n'; i++, cfg++) {
	temp[i] = *cfg;
      }
      temp[i]=0;
      //If starting by 'F', adjust the number and eventually set the name of the menu
      if(temp[0] == 'F' && temp[1]>='1' && temp[1]<='6') {
	int number = temp[1]-'0' - 1;
	if(temp[3] && number<6) {
	  strcpy(FMenu_entries_name[number], (char*)temp+3);
	  //FMenu_entries[number].name[4] = '\0';
	}
      }

      memset(temp, '\0', 20);
      cfg++;
    }
    //free(fmenu_cfg);
  }

  /*
    The following functions are used to display all lines.
    Note: After calling this function, the first clear the memory.
  */

#ifndef CURSOR
int print_x=1,print_y=0;
#endif

  void locate(int x,int y){
    shell_x=x; shell_y=y;
#ifdef CURSOR
    return locate_OS(x,y);
#else
    print_x=1+(x-1)*shell_fontw;
    print_y=(y-1)*shell_fonth;
#endif
  }

  void Cursor_SetPosition(int x,int y){
    return locate(x+1,y+1);
  }

  int print_color(int print_x,int print_y,const char *s,int color,bool invert,bool minimini,GIAC_CONTEXT){
    int python=python_compat(contextptr);
    const char * src=s;
    char singleword[128];
    bool linecomment=false;
    int couleur=color;
    while (*src && print_y<LCD_WIDTH_PX){
      const char * oldsrc=src;
      if ( (python && *src=='#') ||
           (!python && *src=='/' && *(src+1)=='/')){
        linecomment=true;
        couleur=4;
      }
      if (linecomment)
        src = (char*)toksplit((unsigned char*)src, ' ', (unsigned char*)singleword, minimini?50:35); //break into words; next word
      else { // skip string (only with delimiters " ")
        if (*src=='"'){
          for (++src;*src;++src){
            if (*src=='"' && *(src-1)!='\\')
              break;
          }
          if (*src=='"')
            ++src;
          int i=src-oldsrc;
          strncpy(singleword,oldsrc,i);
          singleword[i]=0;
        }
        else {
          size_t i=0;
          for (;*src==' ';++src){ // skip initial whitespaces
            ++i;
          }
          if (i==0){
            if (isalpha(*src)){ // skip keyword
              for (;isalphanum(*src) || *src=='_';++src){
                ++i;
              }
            }
            // go to next space or alphabetic char
            for (;*src;++i,++src){
              if (*src==' ' || (i && *src>=' ' && *src<='/') || (python && *src=='#') || (!python && *src=='/' && *(src+1)=='/')|| *src=='"' || isalpha(*src))
                break;
            }
          }
          strncpy(singleword,oldsrc,i);
          singleword[i]=0;
          if (i==0){
            puts(src); // free(singleword);
            return print_x; // FIXME KEY_CTRL_F2;
          }
        } // end normal case
      } // end else linecomment case
      couleur=linecomment?5:find_color(singleword,contextptr);
      if (couleur==1) couleur=COLOR_BLUE;
      if (couleur==2) couleur=49432; //was COLOR_YELLOWDARK;
      if (couleur==3) couleur=51712;//33024;
      if (couleur==4) couleur=COLOR_MAGENTA;
      if (couleur==5) couleur=_green;
      if (linecomment || singleword[0]=='"')
        print(print_x,print_y,singleword,couleur,invert,/*fake*/false,minimini);
      else { // print two parts, commandname in color and remain in black
        char * ptr=singleword;
        if (isalpha(*ptr)){
          while (isalphanum(*ptr) || *ptr=='_')
            ++ptr;
        }
        char ch=*ptr;
        *ptr=0;
#ifdef HP39
        print(print_x,print_y,singleword,_BLACK,invert,/*fake*/false,minimini);
        // FIXME underline for some colors
#else
        print(print_x,print_y,singleword,couleur,invert,/*fake*/false,minimini);
#endif
        *ptr=ch;
        print(print_x,print_y,ptr,COLOR_BLACK,invert,/*fake*/false,minimini);
      }
      // ?add a space removed from token
      if( linecomment?*src:*src==' ' ){
        if (*src==' ')
          ++src;
        print(print_x,print_y," ",COLOR_BLACK,invert,false,minimini);
      }
    }
    return print_x;
  }    

  void print_color(const char *s,int color,bool invert,bool minimini,GIAC_CONTEXT){
    print_x=print_color(print_x,print_y,s,color,invert,minimini,contextptr);
  }

#ifdef HP39
extern "C" void vGL_putString(int x0, int y0, const char *s, int fg, int bg, int fontSize) ;

void PrintRev(const char *s,int color,bool colorsyntax,GIAC_CONTEXT) {
  vGL_putString(shell_x*shell_fontw, shell_y*shell_fonth, (char *)s, 255, 0, shell_fonth);
  // vGL_ConsOut((char *)s, true);
} 
  void Print(const char *s,int color,bool colorsyntax,GIAC_CONTEXT) {
  vGL_putString(shell_x*shell_fontw, shell_y*shell_fonth, (char *)s, 0,255, shell_fonth);
  //vGL_ConsOut((char *)s, false);
}

#else

  void PrintRev(const char * s,int color,bool colorsyntax,GIAC_CONTEXT){
#ifdef CURSOR
    Print_OS((char *)s,TEXT_MODE_INVERT,0);
#else
    print(print_x,print_y,(const char *)s,color,true/* revert*/,false,false);
#endif  
  }

  void Print(const char * s,int color,bool colorsyntax,GIAC_CONTEXT){
#ifdef CURSOR
    Print_OS((char *)s,TEXT_MODE_NORMAL,0);
#else
    if (!colorsyntax || (strlen(s)==1 && (s[0]=='>' || s[0]=='<')))
      print(print_x,print_y,(const char *)s,color,false,false,false);
    else
      print_color(s,color,false,false,contextptr);
#endif
  }
#endif

  // redraw_mode=1 clear area
  int Console_Disp(int redraw_mode,GIAC_CONTEXT){
#ifdef HP39
    int istatus=1;
#else
    int istatus=0;
#endif
    bool minimini=false;
    unsigned int* pBitmap;
    int i, alpha_shift_status;
    DISPBOX ficon;
    int print_y = 0; //pixel y cursor
    int print_y_locate;
#ifdef HP39
    if (redraw_mode & 1) Bdisp_AllClr_VRAM();
#endif

    //GetFKeyIconPointer( 0x01BE, &ficon );
    //DisplayFKeyIcon( i, ficon);

    //Reading each "line" that will be printed
    for (i = 0; (i < LINE_DISP_MAX) && (i + Start_Line <= Last_Line); i++){
      console_line & curline=Line[i+Start_Line];
      bool colorsyntax=curline.type == LINE_TYPE_INPUT;
      if (i == Cursor.y){
        // cursor line
        //if ((redraw_mode & 1)==0)
        drawRectangle(0,(i+istatus)*shell_fonth,LCD_WIDTH_PX,shell_fonth+1,_WHITE);
        if (curline.type == LINE_TYPE_INPUT || curline.type == LINE_TYPE_OUTPUT && curline.disp_len >= COL_DISP_MAX){
          locate(1, i + 1);
          if (curline.readonly){
#ifdef CURSOR
            Cursor_SetFlashOff();
#endif
            PrintRev(curline.str + curline.start_col,TEXT_COLOR_BLACK,colorsyntax,contextptr);
          }
          else 
            Print(curline.str+curline.start_col+(Cursor.x>COL_DISP_MAX-1?1:0),TEXT_COLOR_BLACK,colorsyntax,contextptr);
        }
        else {
          locate(1, i + 1);
          print(print_x,print_y,(const char *)curline.str,TEXT_COLOR_BLACK,false,true/*fake*/,minimini); // fake print
          print_x=LCD_WIDTH_PX-print_x;
          //CERR << curline.str << " " << print_x << " \n";
          shell_x=print_x/shell_fontw;
          if (curline.readonly){
#ifdef CURSOR
            Cursor_SetFlashOff();
#endif
            PrintRev(curline.str,TEXT_COLOR_BLACK,colorsyntax,contextptr);
          }
          else 
            Print(curline.str,TEXT_COLOR_BLACK,colorsyntax,contextptr);
        }
        
        if (
#if 1 //def CURSOR
            curline.disp_len - curline.start_col > COL_DISP_MAX-1
#else
            print_x>LCD_WIDTH_PX-shell_fontw
#endif
            ){
#ifdef CURSOR
          locate(COL_DISP_MAX, i + 1);
#else
          print_y=i*shell_fonth;
          print_x=LCD_WIDTH_PX+2-shell_fontw;
          //CERR << curline.str << " " << print_x << " \n";
          shell_x=print_x/shell_fontw;
#endif
          if (curline.readonly){
            if(curline.disp_len - curline.start_col != COL_DISP_MAX) {
#ifdef CURSOR
              Cursor_SetFlashOff();
#endif
              PrintRev((char *)">",COLOR_MAGENTA,colorsyntax,contextptr);
            }
          }
          else if (Cursor.x < COL_DISP_MAX-1){
            Print((char *)">",COLOR_MAGENTA,colorsyntax,contextptr);
          }
        }
        
        if (curline.start_col > 0){
          locate(1, i + 1);	
          if (curline.readonly){
#ifdef CURSOR
            Cursor_SetFlashOff();
#endif		  
            PrintRev((char *)"<",COLOR_MAGENTA,colorsyntax,contextptr);
          }
          else {
            Print((char *)"<",COLOR_MAGENTA,colorsyntax,contextptr);
          }
        }
        
        if (!curline.readonly){
          int fakestart=curline.start_col+(Cursor.x > COL_DISP_MAX-1?1:0);
          int fakex,fakey=Cursor.y*shell_fonth;
#ifdef HP39
          fakey+=shell_fonth;
#endif
          string fakes;
          // parenthese match
          const char * str=curline.str;
          int pos=Cursor.x+fakestart,pos2;
          int l=strlen(str);
          char ch=0;
          if (pos<l)
            ch=str[pos];
          int matchdirection=0,paren=0,crochet=0,accolade=0;
          if (ch=='(' || ch=='[' || ch=='{')
            matchdirection=1;
          if (ch=='}' || ch==']' || ch==')')
            matchdirection=-1;
          if (!matchdirection && pos){
            --pos;
            ch=str[pos];
            if (ch=='(' || ch=='[' || ch=='{')
              matchdirection=1;
            if (ch=='}' || ch==']' || ch==')')
              matchdirection=-1;
          }
          if (matchdirection){
            char buf[2]={0,0};
            bool ok=true;
            for (pos2=pos;ok && (pos2>=0 && pos2<l);pos2+=matchdirection){
              ch=str[pos2];
              if (ch=='(') ++paren;
              if (ch==')') --paren;
              if (ch=='[') ++crochet;
              if (ch==']') --crochet;
              if (ch=='{') ++accolade;
              if (ch=='}') --accolade;
              if (matchdirection>0 && (paren<0 || crochet<0 || accolade<0) )
                ok=false;
              if (matchdirection<0 && (paren>0 || crochet>0 || accolade>0) )
                ok=false;
              if (paren==0 && crochet==0 && accolade==0)
                break;
            }
            ok = paren==0 && crochet==0 && accolade==0;
            if (pos>=fakestart){
              fakex=0;
#ifdef HP39
              fakex+=shell_fontw; 
#endif
              buf[0]=str[pos];
              fakes=string((const char *)curline.str).substr(fakestart,pos-fakestart);
              print(fakex,fakey,fakes.c_str(),TEXT_COLOR_BLACK,false,true/* fake*/,minimini); // fake print
              print(fakex,fakey,buf,ok?_green:_red,true/* revert*/,false,minimini);
            }
            if (ok){
              fakex=0;
#ifdef HP39
              fakex+=shell_fontw; 
#endif
              if (pos2>fakestart){
                fakes=string((const char *)curline.str).substr(fakestart,pos2-fakestart);
                print(fakex,fakey,fakes.c_str(),TEXT_COLOR_BLACK,false,true/* fake*/,false); // fake print
                buf[0]=str[pos2];
                print(fakex,fakey,buf,_green,true/* revert*/,false,minimini);
              }
            }
          }
#ifdef CURSOR
          switch(GetSetupSetting( (unsigned int)0x14)) {
          case 0: 
            alpha_shift_status = 0;
            break;
          case 1: //Shift enabled
            alpha_shift_status = 1;
            break;
          case 4: case 0x84:	//Alpha enabled
            alpha_shift_status = 2;
            break;
          case 8: case 0x88:
            alpha_shift_status = 4;
            break;
          default: 
            alpha_shift_status = 0;
            break;
          }
          Cursor_SetPosition(Cursor.x, Cursor.y);
          Cursor_SetFlashOn(alpha_shift_status);
          //Cursor_SetFlashStyle(alpha_shift_status); //Potential 2.00 OS incompatibilty (cf Simon's doc)
#else
          //locate(Cursor.x+1,Cursor.y+1);
          //DefineStatusMessage((giac::print_DOUBLE_(Cursor.y,6)+","+giac::print_DOUBLE_(print_y,6)).c_str(),1,0,0);
          //DisplayStatusArea();
          fakes=string((const char *)curline.str).substr(fakestart,Cursor.x);
#ifdef HP39
          fakey=Cursor.y*shell_fonth;
          drawRectangle(shell_fontw*(1+fakes.size()),fakey+istatus*shell_fonth,2,shell_fonth,COLOR_BLACK);
#else
          fakex=0;
          print(fakex,fakey,fakes.c_str(),TEXT_COLOR_BLACK,false,true/* fake*/,minimini); // fake print
          drawRectangle(fakex,fakey,2,shell_fonth,COLOR_BLACK);
#endif
          //drawRectangle(Cursor.x*shell_fontw,24+Cursor.y*shell_fonth,2,shell_fonth,COLOR_BLACK);
#endif
        }
      } // end cursor line
      else {
        if ((redraw_mode & 1)==0)
          continue;
        drawRectangle(0,(i+istatus)*shell_fonth,LCD_WIDTH_PX,shell_fonth,_WHITE);
        bool bigoutput = curline.type==LINE_TYPE_OUTPUT && curline.disp_len>=COL_DISP_MAX-3;
        locate(bigoutput?3:1,i+1);
        if (curline.type==LINE_TYPE_INPUT || bigoutput)
          Print(curline.str + curline.start_col,TEXT_COLOR_BLACK,colorsyntax,contextptr);
        else {
#ifdef CURSOR
          locate(COL_DISP_MAX - Line[i + Start_Line].disp_len + 1, i + 1);
#else
          print(print_x,print_y,(const char *)curline.str,TEXT_COLOR_BLACK,false,true/*fake*/,minimini);
          print_x=LCD_WIDTH_PX-print_x;
          //CERR << curline.str << " " << print_x << " \n";
          shell_x=print_x/shell_fontw;
#endif
          Print(curline.str,TEXT_COLOR_BLACK,colorsyntax,contextptr);
        }
        if (curline.disp_len - curline.start_col > COL_DISP_MAX){
#ifdef CURSOR
          locate(COL_DISP_MAX, i + 1);
#else
          print_x=LCD_WIDTH_PX+2-shell_fontw;
          shell_x=print_x/shell_fontw;
#endif
          Print((char *)">",COLOR_BLUE,colorsyntax,contextptr);
        }
        if (curline.start_col > 0){
#ifdef CURSOR
          locate(1, i + 1);
#else
          print_x=0; shell_x=0; 
#endif
          Print((char *)"<",COLOR_BLUE,colorsyntax,contextptr);
        }      
      } // end non cursor line
    } // end loop on all lines
#ifdef HP39
    const int C205=114;
#else
    const int C205=205;
#endif
    drawRectangle(0,(i+istatus)*shell_fonth,LCD_WIDTH_PX,C205-(i+istatus)*shell_fonth,_WHITE);

    if ((redraw_mode & 1)==1){
      for (; (i < LINE_DISP_MAX) ; i++)
        drawRectangle(0,(i+istatus)*shell_fonth,LCD_WIDTH_PX,shell_fonth,_WHITE);
      string menu;
#ifndef HP39 
      menu += "shift-1 ";
#endif
      menu += string(menu_f1);
#ifdef HP39
      menu += " |";
#else
      menu += "|2 ";
#endif
      menu += string(menu_f2);
#ifdef HP39
      menu += " |";
#else
      menu += "|3 ";
#endif
      menu += string(menu_f3);
#ifdef HP39
      menu += " |cmds |A<>a |Fich";
      drawRectangle(0,C205,LCD_WIDTH_PX,17,giac::_BLACK);
      PrintMini(0,C205,menu.c_str(),4);
#else
      menu += xcas_python_eval==1?"|4 edt|5 2d|6 logo|7 lin|8 matr|9arit|0 plt":"|4 edt|5 2d|6 regr|7 matr|8 cplx|9 arit|0 rand";
      int xcas_color=65055,python_color=52832,js_color=63048;
      int interp_color=xcas_python_eval==-1?js_color:(xcas_python_eval==1?python_color:xcas_color);
      drawRectangle(0,C205,LCD_WIDTH_PX,17,interp_color);
      PrintMiniMini(0,C205,menu.c_str(),0,giac::_BLACK,interp_color);
#endif
    }
    
    // status, clock,
#ifdef HP39
    drawRectangle(0,0,LCD_WIDTH_PX,shell_fonth,giac::_WHITE);
#endif
    console_disp_status(contextptr);
    return CONSOLE_SUCCEEDED;
  }

  void dConsoleRedraw(){
    Console_Disp(1,0);
  }

  char *Console_GetLine(GIAC_CONTEXT)
  {
    int return_val;
	
    do
      {
	return_val = Console_GetKey(contextptr);
	if (return_val==KEY_SHUTDOWN)
	  return 0;
	Console_Disp(1,contextptr);
	if (return_val == KEY_CTRL_MENU) return 0;
	if (return_val == CONSOLE_MEM_ERR) return NULL;
      } while (return_val != CONSOLE_NEW_LINE_SET);

    return Line[Current_Line - 1].str;
  }

  /*
    Simple accessor to the Edit_Line buffer.
  */
  char* Console_GetEditLine()
  {
    return Edit_Line;
  }

  void save_session(GIAC_CONTEXT){
    if (nspire_exam_mode==2)
      return;
    if (strcmp(session_filename,"session") && console_changed){
      string tmp(session_filename);
      tmp += (lang==1)?" a ete modifie!":" was modified!";
      if (confirm(tmp.c_str(),
#ifdef NSPIRE_NEWLIB
		  (lang==1)?"enter: sauve, esc: tant pis":"enter: save, esc: discard changes"
#else
		  (lang==1)?"OK: sauve, Back: tant pis":"OK: save, Back: discard changes"
#endif
		  )==KEY_CTRL_F1){
	save(session_filename,contextptr);
	console_changed=0;
      }    
    }
    save("session",contextptr);
    // this is only called on exit, no need to reinstall the check_execution_abort timer.
    if (edptr && edptr->changed && edptr->filename!="session.py"){
      if (!check_leave(edptr)){
	save_script("lastprg.py",merge_area(edptr->elements));
      }
    }
  }

#ifdef NSPIRE_NEWLIB
  bool nspire_fr(){
    char16_t input_w[] = u"getLangInfo()";
    void *math_expr = nullptr;
    int str_offset = 0;
    
    int error = TI_MS_evaluateExpr_ACBER(NULL, NULL, (const uint16_t*)input_w, &math_expr, &str_offset);
    if (error)
      return false;
    
    char16_t *output_w;
    error = TI_MS_MathExprToStr(math_expr, NULL, (uint16_t**)&output_w);
    syscall<e_free, void>(math_expr); // Should be TI_MS_DeleteMathExpr
    
    if (error)
      return false;
    int l=0;
    for (l=0;l<64;++l){
      if (output_w[l]==0)
	break;
    }
    bool b=l==4 && output_w[1]=='f' && output_w[2]=='r';
    // Do something with output_w, it's u"42." here
    
    syscall<e_free, void>(output_w);
    return b;
  }
#endif

  Graph2d * geoptr=0;

  // return true if there is a syntax error and user asked to correct
  bool geoparse(textArea *text,GIAC_CONTEXT){
    Graph2d * geoptr=text->gr;
    if (!geoptr)
      return false;
    std::vector<textElement> & v=text->elements;
    geoptr->symbolic_instructions.resize(v.size());
    int pos=-1,i=0;
    for (;i<int(v.size());++i){
      std::string s=v[i].s; 
      giac::python_compat(0,contextptr);
      freeze=true;
      giac::gen g(s,contextptr);
      freeze=false;
      g=equaltosto(g,contextptr);
      int lineerr=giac::first_error_line(contextptr);
      char status[256]={0};
      geoptr->symbolic_instructions[i]=g;
      if (lineerr){
	std::string tok=giac::error_token_name(contextptr);
	if (lineerr==1){
	  pos=v[i].s.find(tok);
	  const std::string & err=v[i].s;
	  if (pos>=err.size())
	    pos=-1;
	}
	else {
	  tok=(lang==1)?"la fin":"end";
	  pos=0;
	}
	if (pos>=0)
	  sprintf(status,(lang==1)?"Erreur ligne %i a %s":"Error line %i at %s",i+1,tok.c_str());
	else
	  sprintf(status,(lang==1)?"Erreur ligne %i %s":"Error line %i %s",i+1,(pos==-2?((lang==1)?", : manquant ?":", missing :?"):""));
	if (confirm(status,(lang==1)?"OK: corrige, back: continue":"OK: fix",1)==KEY_CTRL_F1){
	  text->line=i;
	  if (pos>=0 && pos<v[i].s.size()) text->pos=pos;
	  return true;
	}
      }
    } // loop on lines
    return false;
  }

  int geoloop(Graph2d * geoptr){
    if (!geoptr || !geoptr->hp) return -1;
    const context * contextptr=geoptr->contextptr;
    textArea * text=geoptr->hp;
#ifdef HP39
    text->y=12;
#endif
    // main loop: alternate between plot and symb view
    // start in plot view
    // end plot view with EXIT or OK -> symb view editor
    // end with OK or EXIT: OK will modify, EXIT will leave geo app
    // (press twice EXIT to leave geo app from plot view)
    for (;;){
      geoptr->eval();
      geoptr->update();
      if (geoptr->is3d)
        geoptr->update_rotation();
      int key=geoptr->ui();
      if (key==KEY_SHUTDOWN){
        geosave(text,contextptr);
        return key;
      }
      // symb view editor
      for (;;){
        key=doTextArea(text,contextptr);
        if (key== TEXTAREA_RETURN_EXIT || key==KEY_SHUTDOWN){
          geosave(text,contextptr);
          return key;
        }
        // key was OK, parse step: synchronize symbolic_instructions from text
        bool corrige=geoparse(text,contextptr);
        if (!corrige)
          break;
      } // end edition loop
    } // end plot/symb view infinite loop
  }

  void cleargeo(){
    if (!geoptr)
      return;
    if (geoptr->hp)
      delete geoptr->hp;
    delete geoptr;
    geoptr=0;
  }

  int newgeo(GIAC_CONTEXT){
    if (!geoptr){
      geoptr=new Graph2d(0,contextptr);
      geoptr->window_xmin=-5;
      geoptr->window_ymin=-5;
      geoptr->window_zmin=-5;
      geoptr->window_xmax=5;
      geoptr->window_ymax=5;
      geoptr->window_zmax=5;
      geoptr->orthonormalize();
    }
    if (!geoptr)
      return -1;
    if (!geoptr->hp){
      geoptr->hp=new textArea;
      geoptr->hp->filename="figure0.py";
      geoptr->hp->python=0;
    }
    if (!geoptr->hp)
      return -2;
    textArea * text=geoptr->hp;
    text->editable=true;
    text->clipline=-1;
    text->gr=geoptr;
    geoptr->set_mode(0,0,255,""); // start in frame mode
    return 0;
  }
  
  tableur * sheetptr=0;

  string print_tableur(const tableur & t,GIAC_CONTEXT){
    string s="spreadsheet[";
    for (int i=0;i<t.nrows;++i){
      printcell_current_row(contextptr)=i;
      s += "[";
      gen g=t.m[i];
      if (g.type!=_VECT) continue;
      vecteur & v=*g._VECTptr;
      for (int j=0;j<t.ncols;++j){
	gen vj=v[j];
	if (vj.type==_VECT && vj._VECTptr->size()==3){
	  vecteur vjv=*vj._VECTptr;
	  vjv[1]=0;
	  vj=gen(vjv,vj.subtype);
	}
	printcell_current_col(contextptr)=j;
	s += vj.print(contextptr);
	if (j==t.ncols-1)
	  s += "]";
	else
	  s += ",";
      }
      if (i==t.nrows-1)
	s += "]";
      else
	s += ",";      
    }
    return s;
  }  
  
  void fix_sheet(tableur & t,GIAC_CONTEXT){
    for (int i=0;i<t.nrows;++i){
      vecteur & v = *t.m[i]._VECTptr;
      for (int j=0;j<t.ncols;++j){
	gen & g=v[j];
	if (g.type==_VECT){
	  vecteur & w=*g._VECTptr;
	  if (w[0].type==_SYMB){
	    // cout << "fix " << w[0] << "\n";
	    w[0]=spread_convert(w[0],i,j,contextptr);
	  }
	}
      }
    }
  }


#ifdef NUMWORKS
  extern "C" void mp_stack_ctrl_init();
  extern "C" void mp_stack_set_top(void *);
  extern "C" void mp_stack_set_limit(size_t);
#endif // NUMWORKS


  int console_main(GIAC_CONTEXT,const char * sessionname){
#if defined NUMWORKS && defined DEVICE
    // insure value not too high (_heap_size depends on launcher firmware)
    if (pythonjs_heap_size>_heap_size-52*1024)
      pythonjs_heap_size=_heap_size-52*1024;
#endif
#if defined MICROPY_LIB
    mp_stack_ctrl_init();
#endif
    //volatile int stackTop;
    //mp_stack_set_top((void *)(&stackTop));
    //mp_stack_set_limit(24*1024);
#ifdef QUICKJS
    quickjs_ck_eval("0");
#endif
#ifdef MICROPY_LIB
    giac::micropy_ptr=micropy_ck_eval;
#endif
    python_heap=0;
    sheetptr=0;
    shutdown=do_shutdown;
#ifdef NSPIRE_NEWLIB
    unsigned osid=0,osidcx52noncasnont=0x1040E4D0;
    osid=* (unsigned *) 0x10000020;
    // values
    // OS 5.2 cxcas 1040f3b0
    // OS 5.2 cx2 0x1040E4D0
    // OS 5.2 cx2t 0x1040EAE0
    // OS 5.3 cx2cas 10417da0
    // OS 5.3 cx2 10416cc0
    // OS 5.3 cx2t 10417460
    osok=osid!=osidcx52noncasnont?1:0;
    if ((osid & 0xffff0000)==0x10410000){
      confirm("KhiCAS exammode is incompatible with OS 5.3","Downgrade to 5.2 with backSpire");
    }
#if 0
    if (osok && is_cx2){
      int N=0x800;
      long int nand_offset = 5*64*N;
      long int nand_size = 64*N;
      char flashdata[N];
      const char erased_char=(char) 255;
      int i; 
      for(i=0; i<nand_size; i+=N) {
	read_nand(flashdata, N, nand_offset+i, 0, 0, NULL);
	if (flashdata[0]==erased_char && flashdata[1]==erased_char && flashdata[2]==erased_char && flashdata[3]==erased_char && flashdata[4]==erased_char && flashdata[5]==erased_char && flashdata[6]==erased_char && flashdata[7]==erased_char)
	  break;
      }
      if (i==nand_size){
	confirm(lang==1?"Activez une fois le mode examen TI":"Activate one time TI exam mode",lang==1?"pour utiliser ensuite celui de KhiCAS":"to enable KhiCAS exam mode");
	osok=-1;
      }
    }
#endif
    // detect if leds are blinking
    unsigned green=*(unsigned *) 0x90110b04;
    unsigned red=*(unsigned *) 0x90110b0c;
    if (green || red){
      nspire_exam_mode=1;
    }
    // CX and CX II we should modify the led colors to match CAS exam mode
    // red value should be the same as green value -> yellow
    // try to detect emulator or real calc
    unsigned NSPIRE_SPEED=0x900B0000;
    unsigned speed=*(unsigned *)NSPIRE_SPEED;
    nspireemu= (speed==1445890);
    mkdir("Xcas",0755);
    //mkdir("/Xcas",0755);
    //mkdir("A:/Xcas",0755);
    //mkdir("A:\\Xcas",0755);
    int err=chdir("Xcas");
    if (err)
      err=chdir("ndless");
    bool b=nspire_fr();
    lang=b?1:0;
#endif
    // SetQuitHandler(save_session); // automatically save session when exiting
    int key;
    Console_Init(contextptr);
    if (!turtleptr){
      turtle();
      _efface_logo(vecteur(0),contextptr);
    }
    caseval("floor"); // init xcas parser for Python syntax coloration (!)
    Bdisp_AllClr_VRAM();
    rand_seed(millis(),contextptr);
    if (nspire_exam_mode){ // disabled: save LED state for restoration at end
      // set_exam_mode(2,contextptr);
      exam_mode=0;
      if (1 || is_cx2){
	if (!do_confirm(lang?"Le CAS est-il autorise en examen?":"Is CAS allowed during exam?"))
	  return 0;
	Bdisp_AllClr_VRAM();
      }
    }
    restore_session(sessionname,contextptr);
#ifdef SIMU
    giac::set_language(lang,contextptr);
#endif
    giac::angle_radian(os_get_angle_unit()==0,contextptr);
    //GetKey(&key);
    Console_Disp(1,contextptr);
    // GetKey(&key);
    char *expr=0;
#ifndef NO_STDEXCEPT
    try {
#endif    
    while(1){
      if ((expr=Console_GetLine(contextptr))==NULL){
	save_session(contextptr);
#ifdef NUMWORKS
	return 0;
#endif
	check_nspire_exam_mode(contextptr);
#ifdef MICROPY_LIB
	python_free();
#endif
	Console_Free();
	release_globals();
	if (sheetptr){
	  // sheetptr->m.clear();
	  delete sheetptr;
	  sheetptr=0;
	}
	return 0;
      }
      if (strcmp((const char *)expr,"restart")==0){
	if (confirm((lang==1)?"Effacer variables?":"Clear variables?",
#ifdef NSPIRE_NEWLIB
		    (lang==1)?"enter: confirmer,  esc: annuler":"enter: confirm,  esc: cancel"
#else
		    (lang==1)?"OK: confirmer,  Back: annuler":"OK: confirm,  Back: cancel"
#endif
		    )!=KEY_CTRL_F1){
	  Console_Output(" cancelled");
	  Console_NewLine(LINE_TYPE_OUTPUT,1);
	  //GetKey(&key);
	  Console_Disp(1,contextptr);
	  continue;
	}
      }
      // should save in another file
      if (strcmp((const char *)expr,"=>")==0 || strcmp((const char *)expr,"=>\n")==0){
	save_session(contextptr);
	Console_Output("Session saved");
      }
      else {
#ifdef NUMWORKS // add auto-save, to avoid Memory full data loss
	save("session",contextptr);
#endif
	run(expr,7,contextptr);
      }
      //print_mem_info();
      Console_NewLine(LINE_TYPE_OUTPUT,1);
      //GetKey(&key);
      Console_Disp(1,contextptr);
    }
#ifndef NO_STDEXCEPT
    } catch(autoshutdown & e) {
    }
#endif    
#ifdef NUMWORKS
    return 0;
#endif
    check_nspire_exam_mode(contextptr);
    Console_Free();
    release_globals();
#ifdef MICROPY_LIB
    python_free();
#endif
    if (sheetptr){
      // sheetptr->m.clear();
      delete sheetptr;
      sheetptr=0;
    }
    return 0;
  }


#endif // TEXTAREA

  int rgb24to16(int c){
    int r=(c>>16)&0xff,g=(c>>8)&0xff,b=c&0xff;
    return (((r*32)/256)<<11) | (((g*64)/256)<<5) | (b*32/256);
  }

  // table periodique, code adapte de https://github.com/M4xi1m3/nw-atom
  // avec l'aimable autorisation de diffusion sous licence GPL de Maxime Friess
  // https://tiplanet.org/forum/viewtopic.php?f=97&t=23094&p=247471#p247471
enum AtomType {
  ALKALI_METAL,
  ALKALI_EARTH_METAL,
  LANTHANIDE,
  ACTINIDE,
  TRANSITION_METAL,
  POST_TRANSITION_METAL,
  METALLOID,
  HALOGEN,
  REACTIVE_NONMETAL,
  NOBLE_GAS,
  UNKNOWN
};

struct AtomDef {
  uint8_t num;
  uint8_t x;
  uint8_t y;
  AtomType type;
  const char* name;
  const char* symbol;
  uint8_t neutrons;
  double mass;
  double electroneg;
};

const AtomDef atomsdefs[] = {
  {  1,  0,  0, REACTIVE_NONMETAL       , "Hydrogen"     , "H"   ,   0, 1.00784     , 2.2   },
  {  2, 17,  0, NOBLE_GAS               , "Helium"       , "He"  ,   2, 4.002602    , -1    },
  
  
  {  3,  0,  1, ALKALI_METAL            , "Lithium"      , "Li"  ,   4, 6.938       , 0.98  },
  {  4,  1,  1, ALKALI_EARTH_METAL      , "Beryllium"    , "Be"  ,   5, 9.012182    , 1.57  },
  {  5, 12,  1, METALLOID               , "Boron"        , "B"   ,   6, 10.806      , 2.04  },
  {  6, 13,  1, REACTIVE_NONMETAL       , "Carbon"       , "C"   ,   6, 12.0096     , 2.55  },
  {  7, 14,  1, REACTIVE_NONMETAL       , "Nitrogen"     , "N"   ,   7, 14.00643    , 3.04  },
  {  8, 15,  1, REACTIVE_NONMETAL       , "Oxygen"       , "O"   ,   8, 15.99903    , 3.44  },
  {  9, 16,  1, HALOGEN                 , "Fluorine"     , "F"   ,  10, 18.9984032  , 3.98  },
  { 10, 17,  1, NOBLE_GAS               , "Neon"         , "Ne"  ,  10, 20.1797     , -1    },
  
  
  { 11,  0,  2, ALKALI_METAL            , "Sodium"       , "Na"  ,  12, 22.9897693  , 0.93  },
  { 12,  1,  2, ALKALI_EARTH_METAL      , "Magnesium"    , "Mg"  ,  12, 24.3050     , 1.31  },
  { 13, 12,  2, POST_TRANSITION_METAL   , "Aluminium"    , "Al"  ,  14, 26.9815386  , 1.61  },
  { 14, 13,  2, METALLOID               , "Silicon"      , "Si"  ,  14, 28.084      , 1.9   },
  { 15, 14,  2, REACTIVE_NONMETAL       , "Phosphorus"   , "P"   ,  16, 30.973762   , 2.19  },
  { 16, 15,  2, REACTIVE_NONMETAL       , "Sulfur"       , "S"   ,  16, 32.059      , 2.58  },
  { 17, 16,  2, HALOGEN                 , "Chlorine"     , "Cl"  ,  18, 35.446      , 3.16  },
  { 18, 17,  2, NOBLE_GAS               , "Argon"        , "Ar"  ,  22, 39.948      , -1    },
  
  
  { 19,  0,  3, ALKALI_METAL            , "Potassium"    , "K"   ,  20, 39.0983     , 0.82  },
  { 20,  1,  3, ALKALI_EARTH_METAL      , "Calcium"      , "Ca"  ,  20, 40.078      , 1     },
  { 21,  2,  3, TRANSITION_METAL        , "Scandium"     , "Sc"  ,  24, 44.955912   , 1.36  },
  { 22,  3,  3, TRANSITION_METAL        , "Titanium"     , "Ti"  ,  26, 47.867      , 1.54  },
  { 23,  4,  3, TRANSITION_METAL        , "Vanadium"     , "V"   ,  28, 50.9415     , 1.63  },
  { 24,  5,  3, TRANSITION_METAL        , "Chromium"     , "Cr"  ,  28, 51.9961     , 1.66  },
  { 25,  6,  3, TRANSITION_METAL        , "Manganese"    , "Mn"  ,  30, 54.938045   , 1.55  },
  { 26,  7,  3, TRANSITION_METAL        , "Iron"         , "Fe"  ,  30, 55.845      , 1.83  },
  { 27,  8,  3, TRANSITION_METAL        , "Cobalt"       , "Co"  ,  32, 58.933195   , 1.88  },
  { 28,  9,  3, TRANSITION_METAL        , "Nickel"       , "Ni"  ,  30, 58.6934     , 1.91  },
  { 29, 10,  3, TRANSITION_METAL        , "Copper"       , "Cu"  ,  34, 63.546      , 1.9   },
  { 30, 11,  3, POST_TRANSITION_METAL   , "Zinc"         , "Zn"  ,  34, 65.38       , 1.65  },
  { 31, 12,  3, POST_TRANSITION_METAL   , "Gallium"      , "Ga"  ,  38, 69.723      , 1.81  },
  { 32, 13,  3, METALLOID               , "Germanium"    , "Ge"  ,  42, 72.63       , 2.01  },
  { 33, 14,  3, METALLOID               , "Arsenic"      , "As"  ,  42, 74.92160    , 2.18  },
  { 34, 15,  3, REACTIVE_NONMETAL       , "Selenium"     , "Se"  ,  46, 78.96       , 2.55  },
  { 35, 16,  3, HALOGEN                 , "Bromine"      , "Br"  ,  44, 79.904      , 2.96  },
  { 36, 17,  3, NOBLE_GAS               , "Krypton"      , "Kr"  ,  48, 83.798      , -1    },
  
  { 37,  0,  4, ALKALI_METAL            , "Rubidium"     , "Rb"  ,  48, 85.4678     , 0.82  },
  { 38,  1,  4, ALKALI_EARTH_METAL      , "Strontium"    , "Sr"  ,  50, 87.62       , 0.95  },
  { 39,  2,  4, TRANSITION_METAL        , "Yttrium"      , "Y"   ,  50, 88.90585    , 1.22  },
  { 40,  3,  4, TRANSITION_METAL        , "Zirconium"    , "Zr"  ,  50, 91.224      , 1.33  },
  { 41,  4,  4, TRANSITION_METAL        , "Niobium"      , "Nb"  ,  52, 92.90638    , 1.6   },
  { 42,  5,  4, TRANSITION_METAL        , "Molybdenum"   , "Mo"  ,  56, 95.96       , 2.16  },
  { 43,  6,  4, TRANSITION_METAL        , "Technetium"   , "Tc"  ,  55, 98          , 2.10  },
  { 44,  7,  4, TRANSITION_METAL        , "Ruthemium"    , "Ru"  ,  58, 101.07      , 2.2   },
  { 45,  8,  4, TRANSITION_METAL        , "Rhodium"      , "Rh"  ,  58, 102.90550   , 2.28  },
  { 46,  9,  4, TRANSITION_METAL        , "Palladium"    , "Pd"  ,  60, 106.42      , 2.20  },
  { 47, 10,  4, TRANSITION_METAL        , "Silver"       , "Ag"  ,  60, 107.8682    , 1.93  },
  { 48, 11,  4, POST_TRANSITION_METAL   , "Cadmium"      , "Cd"  ,  66, 112.411     , 1.69  },
  { 49, 12,  4, POST_TRANSITION_METAL   , "Indium"       , "In"  ,  66, 114.818     , 1.78  },
  { 50, 13,  4, POST_TRANSITION_METAL   , "Tin"          , "Sn"  ,  70, 118.710     , 1.96  },
  { 51, 14,  4, METALLOID               , "Antimony"     , "Sb"  ,  70, 121.760     , 2.05  },
  { 52, 15,  4, METALLOID               , "Tellurium"    , "Te"  ,  78, 127.60      , 2.1   },
  { 53, 16,  4, HALOGEN                 , "Indine"       , "I"   ,  74, 126.90447   , 2.66  },
  { 54, 17,  4, NOBLE_GAS               , "Xenon"        , "Xe"  ,  78, 131.293     , 2.60  },
  
  
  { 55,  0,  5, ALKALI_METAL            , "Caesium"      , "Cs"  ,  78, 132.905452  , 0.79  },
  { 56,  1,  5, ALKALI_EARTH_METAL      , "Barium"       , "Ba"  ,  82, 137.327     , 0.89  },

  { 57,  3,  7, LANTHANIDE              , "Lanthanum"    , "La"  ,  82, 138.90547   , 1.10  },
  { 58,  4,  7, LANTHANIDE              , "Cerium"       , "Ce"  ,  82, 140.116     , 1.12  },
  { 59,  5,  7, LANTHANIDE              , "Praseodymium" , "Pr"  ,  82, 140.90765   , 1.13  },
  { 60,  6,  7, LANTHANIDE              , "Neodymium"    , "Nd"  ,  82, 144.242     , 1.14  },
  { 61,  7,  7, LANTHANIDE              , "Promethium"   , "Pm"  ,  84, 145         , 1.13  },
  { 62,  8,  7, LANTHANIDE              , "Samarium"     , "Sm"  ,  90, 150.36      , 1.17  },
  { 63,  9,  7, LANTHANIDE              , "Europium"     , "Eu"  ,  90, 151.964     , 1.12  },
  { 64, 10,  7, LANTHANIDE              , "Gadolinium"   , "Gd"  ,  94, 157.25      , 1.20  },
  { 65, 11,  7, LANTHANIDE              , "Terbium"      , "Tb"  ,  94, 158.92535   , 1.12  },
  { 66, 12,  7, LANTHANIDE              , "Dyxprosium"   , "Dy"  ,  98, 162.500     , 1.22  },
  { 67, 13,  7, LANTHANIDE              , "Holmium"      , "Ho"  ,  98, 164.93032   , 1.23  },
  { 68, 14,  7, LANTHANIDE              , "Erbium"       , "Er"  ,  98, 167.259     , 1.24  },
  { 69, 15,  7, LANTHANIDE              , "Thulium"      , "Tm"  , 100, 168.93421   , 1.25  },
  { 70, 16,  7, LANTHANIDE              , "Ytterbium"    , "Yb"  , 104, 173.054     , 1.1   },
  { 71, 17,  7, LANTHANIDE              , "Lutetium"     , "Lu"  , 104, 174.9668    , 1.0   },

  { 72,  3,  5, TRANSITION_METAL        , "Hafnium"      , "Hf"  , 108, 178.49      , 1.3   },
  { 73,  4,  5, TRANSITION_METAL        , "Tantalum"     , "Ta"  , 108, 180.94788   , 1.5   },
  { 74,  5,  5, TRANSITION_METAL        , "Tungsten"     , "W"   , 110, 183.84      , 1.7   },
  { 75,  6,  5, TRANSITION_METAL        , "Rhenium"      , "Re"  , 112, 186.207     , 1.9   },
  { 76,  7,  5, TRANSITION_METAL        , "Osmium"       , "Os"  , 116, 190.23      , 2.2   },
  { 77,  8,  5, TRANSITION_METAL        , "Iridium"      , "Ir"  , 116, 192.217     , 2.2   },
  { 78,  9,  5, TRANSITION_METAL        , "Platinum"     , "Pt"  , 117, 195.084     , 2.2   },
  { 79, 10,  5, TRANSITION_METAL        , "Gold"         , "Au"  , 118, 196.966569  , 2.4   },
  { 80, 11,  5, POST_TRANSITION_METAL   , "Mercury"      , "Hg"  , 122, 200.59      , 1.9   },
  { 81, 12,  5, POST_TRANSITION_METAL   , "Thalium"      , "Tl"  , 124, 204.382     , 1.8   },
  { 82, 13,  5, POST_TRANSITION_METAL   , "Lead"         , "Pb"  , 126, 207.2       , 1.8   },
  { 83, 14,  5, POST_TRANSITION_METAL   , "Bismuth"      , "Bi"  , 126, 208.98040   , 1.9   },
  { 84, 15,  5, POST_TRANSITION_METAL   , "Polonium"     , "Po"  , 126, 209         , 2.0   },
  { 85, 16,  5, HALOGEN                 , "Astatine"     , "At"  , 125, 210         , 2.2   },
  { 86, 17,  5, NOBLE_GAS               , "Radon"        , "Rn"  , 136, 222         , 2.2   },
  
  
  { 87,  0,  6, ALKALI_METAL            , "Francium"     , "Fr"  , 136, 223         , 0.7   },
  { 88,  1,  6, ALKALI_EARTH_METAL      , "Radium"       , "Ra"  , 138, 226         , 0.9   },

  { 89,  3,  8, ACTINIDE                , "Actinium"     , "Ac"  , 138, 227         , 1.1   },
  { 90,  4,  8, ACTINIDE                , "Thorium"      , "Th"  , 142, 232.03806   , 1.3   },
  { 91,  5,  8, ACTINIDE                , "Protactinium" , "Pa"  , 140, 231.03588   , 1.5   },
  { 92,  6,  8, ACTINIDE                , "Uranium"      , "U"   , 146, 238.02891   , 1.38   },
  { 93,  7,  8, ACTINIDE                , "Neptunium"    , "Np"  , 144, 237         , 1.36   },
  { 94,  8,  8, ACTINIDE                , "Plutonium"    , "Pu"  , 150, 244         , 1.28   },
  { 95,  9,  8, ACTINIDE                , "Americium"    , "Am"  , 148, 243         , 1.13  },
  { 96, 10,  8, ACTINIDE                , "Curium"       , "Cm"  , 151, 247         , 1.28  },
  { 97, 11,  8, ACTINIDE                , "Berkellum"    , "Bk"  , 150, 247         , 1.3   },
  { 98, 12,  8, ACTINIDE                , "Californium"  , "Cf"  , 153, 251         , 1.3   },
  { 99, 13,  8, ACTINIDE                , "Einsteinium"  , "Es"  , 153, 252         , 1.3   },
  {100, 14,  8, ACTINIDE                , "Fermium"      , "Fm"  , 157, 257         , 1.3   },
  {101, 15,  8, ACTINIDE                , "Mendelevium"  , "Md"  , 157, 258         , 1.3   },
  {102, 16,  8, ACTINIDE                , "Nobelium"     , "No"  , 157, 259         , 1.3   },
  {103, 17,  8, ACTINIDE                , "Lawrencium"   , "Lr"  , 163, 262         , 1.3   },

  {104,  3,  6, TRANSITION_METAL        , "Rutherfordium", "Rf"  , 163, 261         , -1    },
  {105,  4,  6, TRANSITION_METAL        , "Dubnium"      , "Db"  , 163, 262         , -1    },
  {106,  5,  6, TRANSITION_METAL        , "Seaborgium"   , "Sg"  , 163, 263         , -1    },
  {107,  6,  6, TRANSITION_METAL        , "Bohrium"      , "Bh"  , 163, 264         , -1    },
  {108,  7,  6, TRANSITION_METAL        , "Hassium"      , "Hs"  , 169, 265         , -1    },
  {109,  8,  6, UNKNOWN                 , "Meitnerium"   , "Mt"  , 169, 268         , -1    },
  {110,  9,  6, UNKNOWN                 , "Damstadtium"  , "Ds"  , 171, 281         , -1    },
  {111, 10,  6, UNKNOWN                 , "Roentgenium"  , "Rg"  , 171, 273         , -1    },
  {112, 11,  6, POST_TRANSITION_METAL   , "Coppernicium" , "Cn"  , 173, 277         , -1    },
  {113, 12,  6, UNKNOWN                 , "Nihonium"     , "Nh"  , 173, 283         , -1    },
  {114, 13,  6, UNKNOWN                 , "Flerovium"    , "Fl"  , 175, 285         , -1    },
  {115, 14,  6, UNKNOWN                 , "Moscovium"    , "Mv"  , 174, 287         , -1    },
  {116, 15,  6, UNKNOWN                 , "Livermorium"  , "Lv"  , 177, 289         , -1    },
  {117, 16,  6, UNKNOWN                 , "Tennessine"   , "Ts"  , 177, 294         , -1    },
  {118, 17,  6, NOBLE_GAS               , "Oganesson"    , "Og"  , 176, 293         , -1    },
  
};
  
#ifdef HP39
  const int C16=13;
  const int C17=14;
  const int c18=15;
  const int c6=1;
#else
  const int C16=16;
  const int C17=17;
  const int c18=18;
  const int c6=6;
#endif  

void drawAtom(uint8_t id) {
  int fill = rgb24to16(0xeeeeee);

  switch(atomsdefs[id].type) {
    case ALKALI_METAL:
      fill = rgb24to16(0xffaa00);
      break;
    case ALKALI_EARTH_METAL:
      fill = rgb24to16(0xf6f200);
      break;
    case LANTHANIDE:
      fill = rgb24to16(0xffaa8b);
      break;
    case ACTINIDE:
      fill = rgb24to16(0xdeaacd);
      break;
    case TRANSITION_METAL:
      fill = rgb24to16(0xde999c);
      break;
    case POST_TRANSITION_METAL:
      fill = rgb24to16(0x9cbaac);
      break;
    case METALLOID:
      fill = rgb24to16(0x52ce8b);
      break;
    case REACTIVE_NONMETAL:
      fill = rgb24to16(0x00ee00);
      break;
    case NOBLE_GAS:
      fill = rgb24to16(0x8baaff);
      break;
    case HALOGEN:
      fill = rgb24to16(0x00debd);
      break;
    default:
      break;
  }
  if (atomsdefs[id].y >= 7) {
    drawRectangle(c6 + atomsdefs[id].x * C17, c6+2 + atomsdefs[id].y * C17, c18, c18, fill);
    stroke_rectangle(c6 + atomsdefs[id].x * C17, c6+2 + atomsdefs[id].y * C17, c18, c18, rgb24to16(0x525552));
    os_draw_string_small(c6+2 + atomsdefs[id].x * C17, c6+4 + atomsdefs[id].y * C17, _BLACK, fill, atomsdefs[id].symbol);
  } else {
    drawRectangle(c6 + atomsdefs[id].x * C17, c6 + atomsdefs[id].y * C17, c18, c18, fill);
    stroke_rectangle(c6 + atomsdefs[id].x * C17, c6 + atomsdefs[id].y * C17, c18, c18, rgb24to16(0x525552));
    os_draw_string_small(c6+2 + atomsdefs[id].x * C17, c6+2 + atomsdefs[id].y * C17, _BLACK, fill, atomsdefs[id].symbol);
  }
}

  int periodic_table(const char * & name,const char * & symbol,char * protons,char * nucleons,char * mass,char * electroneg){
    bool partial_draw=false,redraw=true;
    int cursor_pos=0;
    const int ATOM_NUMS=sizeof(atomsdefs)/sizeof(AtomDef);
    for (;;){
      if (redraw){
	if (partial_draw) {
	  partial_draw = false;
	  drawRectangle(50, 0, 169, 57, _WHITE);
	  drawRectangle(0, 185, LCD_WIDTH_PX, 15, _WHITE);
	} else {
	  drawRectangle(0,0,LCD_WIDTH_PX,LCD_HEIGHT_PX,_WHITE);
	}
#ifdef NSPIRE_NEWLIB
	os_draw_string_small_(0,200,gettext("enter: tout, P:protons, N:nucleons, M:mass, E:khi"));
#else
	os_draw_string_small_(0,200,gettext("OK: tout, P:protons, N:nucleons, M:mass, E:khi"));
#endif
	for(int i = 0; i < ATOM_NUMS; i++) {
	  drawAtom(i);
	}
	if (atomsdefs[cursor_pos].y >= 7) {
	  stroke_rectangle(c6 + atomsdefs[cursor_pos].x * C17, c6+2 + atomsdefs[cursor_pos].y * C17, c18, c18, 0x000000);
	  stroke_rectangle(c6+1 + atomsdefs[cursor_pos].x * C17, c6+3 + atomsdefs[cursor_pos].y * C17, C16, C16, 0x000000);
	} else {
	  stroke_rectangle(c6 + atomsdefs[cursor_pos].x * C17, c6 + atomsdefs[cursor_pos].y * C17, c18, c18, 0x000000);
	  stroke_rectangle(c6+1 + atomsdefs[cursor_pos].x * C17, c6+1 + atomsdefs[cursor_pos].y * C17, C16, C16, 0x000000);
	}
  
	drawRectangle(48,  99, 2, 61,rgb24to16(0x525552));
	drawRectangle(48, 141, 9,  2, rgb24to16(0x525552));
	drawRectangle(48, 158, 9,  2, rgb24to16(0x525552));

	int prot=atomsdefs[cursor_pos].num;
	sprint_int(protons,prot);
	int nuc=atomsdefs[cursor_pos].neutrons+atomsdefs[cursor_pos].num;
	sprint_int(nucleons,nuc);
	
	symbol=atomsdefs[cursor_pos].symbol;
	os_draw_string_(73,23,symbol);
	name=atomsdefs[cursor_pos].name;
#ifdef HP39
	os_draw_string_small_(100,27,gettext(name));
#else
	os_draw_string_small_(110,27,gettext(name));
#endif
	os_draw_string_small_(50,18,nucleons);
	os_draw_string_small_(50,31,protons);
	strcpy(mass,"M:");
	strcpy(electroneg,"khi:");
	sprint_double(mass+2,atomsdefs[cursor_pos].mass);
	sprint_double(electroneg+4,atomsdefs[cursor_pos].electroneg);
#ifdef HP39
	os_draw_string_small_(60,2,mass);
	os_draw_string_small_(135,2,electroneg);
#else
	os_draw_string_small_(0,186,mass);
	os_draw_string_small_(160,186,electroneg);
#endif
      }
      redraw=false;
      int key;
      GetKey(&key);
      if (key==KEY_SHUTDOWN)
	return key;
      if (key==KEY_PRGM_ACON)
	redraw=true;
      if (key==KEY_CTRL_EXIT)
	return 0;
      if (key==KEY_CTRL_EXE || key==KEY_CTRL_OK)
	return 1|4|8|16|32;
      if (key=='s' || key==KEY_CHAR_5)
	return 2;
      if (key=='p' || key==KEY_CHAR_LPAR)
	return 4;
      if (key=='n' || key==KEY_CHAR_8)
	return 8;
      if (key=='m' || key==KEY_CHAR_7)
	return 16;
      if (key=='e' || key==KEY_CHAR_COMMA)
	return 32;
      if (key==KEY_CTRL_LEFT){
	if (cursor_pos>0)
	  --cursor_pos;
	redraw=partial_draw=true;
      }
      if (key==KEY_CTRL_RIGHT){
	if (cursor_pos< ATOM_NUMS-1)
	  ++cursor_pos;
	redraw=partial_draw=true;
      }
      if (key==KEY_CTRL_UP){
	uint8_t curr_x = atomsdefs[cursor_pos].x;
	uint8_t curr_y = atomsdefs[cursor_pos].y;
	bool updated = false;
	
	if (curr_y > 0 && curr_y <= 9) {
	  for(uint8_t i = 0; i < ATOM_NUMS; i++) {
	    if (atomsdefs[i].x == curr_x && atomsdefs[i].y == curr_y - 1) {
	      cursor_pos = i;
	      redraw=partial_draw = true;
	    }
	  }
	}
	
      }
      if (key==KEY_CTRL_DOWN){
	uint8_t curr_x = atomsdefs[cursor_pos].x;
	uint8_t curr_y = atomsdefs[cursor_pos].y;
	bool updated = false;
	
	if (curr_y >= 0 && curr_y < 9) {
	  for (uint8_t i = 0; i < ATOM_NUMS; i++) {
	    if (atomsdefs[i].x == curr_x && atomsdefs[i].y == curr_y + 1) {
	      cursor_pos = i;
	      redraw=partial_draw = true;
	      break;
	    }
	  }
	}
      }
    } // end endless for
  } // end periodic_table

#ifndef NO_NAMESPACE_XCAS
} // namespace xcas
#endif // ndef NO_NAMESPACE_XCAS

void console_output(const char * s,int l){
  char buf[l+1];
  strncpy(buf,s,l);
  buf[l]=0;
  xcas::dConsolePut(buf);
}

const char * console_input(const char * msg1,const char * msg2,bool numeric,int ypos){
  static string str;
  if (!giac::inputline(msg1,msg2,str,numeric,ypos,context0))
    return 0;
  return str.c_str();
}

void c_draw_rectangle(int x,int y,int w,int h,int c){
  giac::freeze=true;
  xcas::draw_line(x,y,x+w,y,c);
  xcas::draw_line(x+w,y,x+w,y+h,c);
  xcas::draw_line(x,y+h,x+w,y+h,c);
  xcas::draw_line(x,y,x,y+h,c);
}
void c_draw_line(int x0,int y0,int x1,int y1,int c){
  giac::freeze=true;
  xcas::draw_line(x0,y0,x1,y1,c);
}
void c_draw_circle(int xc,int yc,int r,int color,bool q1,bool q2,bool q3,bool q4){
  giac::freeze=true;
  xcas::draw_circle(xc,yc,r,color,q1,q2,q3,q4);
}
void c_draw_filled_circle(int xc,int yc,int r,int color,bool left,bool right){
  giac::freeze=true;
  xcas::draw_filled_circle(xc,yc,r,color,left,right);
}
void c_convert(int *x,int*y,vector< vector<int> > & v){
  for (int i=0;i<v.size();++i,++x,++y){
    v[i].push_back(*x);
    v[i].push_back(*y);
  }
}
void c_draw_polygon(int * x,int *y ,int n,int color){
  giac::freeze=true;
  vector< vector<int> > v(n);
  c_convert(x,y,v);
  xcas::draw_polygon(v,color);
}
void c_draw_filled_polygon(int * x,int *y, int n,int xmin,int xmax,int ymin,int ymax,int color){
  giac::freeze=true;
  vector< vector<int> > v(n);
  c_convert(x,y,v);
  xcas::draw_filled_polygon(v,xmin,xmax,ymin,ymax,color);
}
void c_draw_arc(int xc,int yc,int rx,int ry,int color,double theta1, double theta2){
  giac::freeze=true;
  xcas::draw_arc(xc,yc,rx,ry,color,theta1,theta2);
}
void c_draw_filled_arc(int x,int y,int rx,int ry,int theta1_deg,int theta2_deg,int color,int xmin,int xmax,int ymin,int ymax,bool segment){
  giac::freeze=true;
  xcas::draw_filled_arc(x,y,rx,ry,theta1_deg,theta2_deg,color,xmin,xmax,ymin,ymax,segment);
}
void c_set_pixel(int x,int y,int c){
  giac::freeze=true;
  os_set_pixel(x,y,c);
}
void c_fill_rect(int x,int y,int w,int h,int c){
  giac::freeze=true;
  if (w<0){
    w=-w;
    x -= w; 
  }
  if (h<0){
    h=-h;
    y -= h; 
  }
  if (x<0){ w+=x; x=0;}
  if (y<0){ h+=y; y=0;}
  os_fill_rect(x,y,w,h,c);
}
int c_draw_string(int x,int y,int c,int bg,const char * s,bool fake){
  giac::freeze=true;
  return os_draw_string(x,y,c,bg,s,fake);
}
int c_draw_string_small(int x,int y,int c,int bg,const char * s,bool fake){
  giac::freeze=true;
  return os_draw_string_small(x,y,c,bg,s,fake);
}
int c_draw_string_medium(int x,int y,int c,int bg,const char * s,bool fake){
  giac::freeze=true;
  return os_draw_string_medium(x,y,c,bg,s,fake);
}

int select_item(const char ** ptr,const char * title,bool askfor1){
  int nitems=0;
  for (const char ** p=ptr;*p;++p)
    ++nitems;
  if (nitems==0 || nitems>=256)
    return -1;
  if (!askfor1 && nitems==1)
    return 0;
  MenuItem smallmenuitems[nitems];
  for (int i=0;i<nitems;++i){
    smallmenuitems[i].text=(char *) ptr[i];
  }
  Menu smallmenu;
  smallmenu.numitems=nitems; 
  smallmenu.items=smallmenuitems;
  smallmenu.height=nitems<MENUHEIGHT?nitems+1:MENUHEIGHT;
  smallmenu.scrollbar=1;
  smallmenu.scrollout=1;
  smallmenu.title = (char*) title;
  //MsgBoxPush(5);
  int sres = doMenu(&smallmenu);
  //MsgBoxPop();
  if (sres!=MENU_RETURN_SELECTION && sres!=KEY_CTRL_EXE)
    return -1;
  return smallmenu.selection-1;
}

int select_interpreter(){
  const char * choix[]={"Xcas interpreter","Xcas compat Python ^=**","Xcas compat Python ^=xor","MicroPython interpreter","Javascript (QuickJS)",0};
  return select_item(choix,"Syntax",false);
}

ulonglong double2gen(double d){
  giac::gen g(d);
  return *(ulonglong *) &g;
}

ulonglong int2gen(int d){
  giac::gen g(d);
  return *(ulonglong *) &g;
}

void turtle_freeze(){
  freezeturtle=true;
}

void doubleptr2matrice(double * x,int n,int m,giac::matrice & M){
  M.resize(n);
  for (int i=0;i<n;++i){
    M[i]=giac::vecteur(m);
    giac::vecteur & w=*M[i]._VECTptr;
    for (int j=0;j<m;++j){
      w[j]=*x;
      ++x;
    }
  }
}

// x must have enough space!
bool matrice2doubleptr(const giac::matrice &M,double *x){
  int n=M.size();
  if (n==0 || M.front().type!=giac::_VECT)
    return false;
  int m=M.front()._VECTptr->size();
  for (int i=0;i<n;++i){
    if (M[i].type!=giac::_VECT || M[i]._VECTptr->size()!=m)
      return false;
    giac::vecteur & w=*M[i]._VECTptr;
    for (int j=0;j<m;++j){
      giac::gen g =giac::evalf_double(w[j],1,giac::context0);
      if (g.type!=giac::_DOUBLE_)
	return false;
      *x=g._DOUBLE_val;
      ++x;
    }
  }
  return true;
}

bool r_inv(double * x,int n){
  giac::matrice M(n);
  doubleptr2matrice(x,n,n,M);
  M=giac::minv(M,giac::context0);
  return matrice2doubleptr(M,x);
}


bool r_rref(double * x,int n,int m){
  giac::matrice M(n);
  doubleptr2matrice(x,n,m,M);
  giac::gen g=giac::_rref(M,giac::context0);
  if (g.type!=giac::_VECT)
    return false;
  return matrice2doubleptr(*g._VECTptr,x);
}

double r_det(double *x,int n){
  giac::matrice M(n);
  doubleptr2matrice(x,n,n,M);
  giac::gen g=giac::mdet(M,giac::context0);
  g=giac::evalf_double(g,1,giac::context0);
  double d=1.0,e=1.0;
  if (g.type!=_DOUBLE_)
    return 0.0/(d-e);
  return g._DOUBLE_val;
}

void c_complexptr2matrice(c_complex * x,int n,int m,giac::matrice & M){
  M.resize(n);
  for (int i=0;i<n;++i){
    if (m==0){
      M[i]=gen(x->r,x->i);
      ++x;
      continue;
    }
    M[i]=giac::vecteur(m);
    giac::vecteur & w=*M[i]._VECTptr;
    for (int j=0;j<m;++j){
      w[j]=gen(x->r,x->i);
      ++x;
    }
  }
}

c_complex gen2c_complex(giac::gen & g){
  double d=1.0,e=1.0;
  c_complex c={0,0};
  if (g.type!=giac::_DOUBLE_ && g.type!=giac::_CPLX)
    c.r=c.i=0.0/(d-e);
  else {
    if (g.type==giac::_DOUBLE_)
      c.r=g._DOUBLE_val;
    else {
      if (g.subtype!=3)
	c.r=c.i=0.0/(d-e);
      c.r=g._CPLXptr->_DOUBLE_val;
      c.i=(g._CPLXptr+1)->_DOUBLE_val;
    }
  }
  return c;
}

// x must have enough space!
bool matrice2c_complexptr(const giac::matrice &M,c_complex *x){
  int n=M.size();
  if (n==0)
    return false;
  if (M.front().type!=giac::_VECT){
    for (int i=0;i<n;++i){
      giac::gen g =giac::evalf_double(M[i],1,giac::context0);
      if (g.type!=giac::_DOUBLE_ && g.type!=giac::_CPLX)
	return false;
      *x=gen2c_complex(g);
      ++x;
    }
    return true;
  }
  int m=M.front()._VECTptr->size();
  for (int i=0;i<n;++i){
    if (M[i].type!=giac::_VECT || M[i]._VECTptr->size()!=m)
      return false;
    giac::vecteur & w=*M[i]._VECTptr;
    for (int j=0;j<m;++j){
      giac::gen g =giac::evalf_double(w[j],1,giac::context0);
      if (g.type!=giac::_DOUBLE_ && g.type!=giac::_CPLX)
	return false;
      *x=gen2c_complex(g);
      ++x;
    }
  }
  return true;
}


c_complex operator +(const c_complex & a,const c_complex & b){
  c_complex c={a.r+b.r,a.i+b.i};
  return c;
}

c_complex c_complex::operator +=(const c_complex & b){
  r += b.r;
  i += b.i;
  return *this;
}

c_complex c_complex::operator -=(const c_complex & b){
  r -= b.r;
  i -= b.i;
  return *this;
}

c_complex operator -(const c_complex & a,const c_complex & b){
  c_complex c={a.r-b.r,a.i-b.i};
  return c;
}

c_complex operator -(const c_complex & a,double b){
  c_complex c={a.r-b,a.i};
  return c;
}

c_complex operator -(const c_complex & a){
  c_complex c={-a.r,-a.i};
  return c;
}

c_complex operator /(const c_complex & a,double d){
  c_complex c={a.r/d,a.i/d};
  return c;
}

c_complex operator *(const c_complex & a,double d){
  c_complex c={a.r*d,a.i*d};
  return c;
}

c_complex operator *(double d,const c_complex & a){
  c_complex c={a.r*d,a.i*d};
  return c;
}

c_complex operator *(const c_complex & a,const c_complex & b){
  c_complex c={a.r*b.r-a.i*b.i,a.r*b.i+a.i*b.r};
  return c;
}

  static void fft2( c_complex *A, int n, c_complex *W, c_complex *T ) {  
    if ( n==1 ) return;
    // if p is fixed, the code is about 2* faster
    if (n==4){
      c_complex w1=W[1];
      c_complex f0=A[0],f1=A[1],f2=A[2],f3=A[3],f01=(f1-f3)*w1;
      A[0]=(f0+f1+f2+f3);
      A[1]=(f0-f2+f01);
      A[2]=(f0-f1+f2-f3);
      A[3]=(f0-f2-f01);
      return;
    }
    if (n==2){
      c_complex f0=A[0],f1=A[1];
      A[0]=(f0+f1);
      A[1]=(f0-f1);
      return;
    }
    int i,n2;
    n2 = n/2;
    // Step 1 : arithmetic
    c_complex * Tn2=T+n2,*An2=A+n2;
    for( i=0; i<n2; ++i ) {
      c_complex Ai,An2i;
      Ai=A[i];
      An2i=An2[i];
      T[i] = Ai+An2i; // addmod(Ai,An2i,p);
      Tn2[i] = (Ai-An2i)*W[i]; // submod(Ai,An2i,p); mulmod(t,W[i],p); 
      i++;
      Ai=A[i];
      An2i=An2[i];
      T[i] = Ai+An2i; // addmod(Ai,An2i,p);
      Tn2[i] = (Ai-An2i)*W[i]; // submod(Ai,An2i,p); mulmod(t,W[i],p); 
    }
    // Step 2 : recursive calls
    fft2( T,    n2, W+n2, A    );
    fft2( Tn2, n2, W+n2, A+n2 );
    // Step 3 : permute
    for( i=0; i<n2; ++i ) {
      A[  2*i] = T[i];
      A[2*i+1] = Tn2[i]; 
      ++i;
      A[  2*i] = T[i];
      A[2*i+1] = Tn2[i]; 
    }
    return;
  }  

  void fft2( c_complex * A, int n, double theta){
    vector< c_complex > W,T(n);
    W.reserve(n); 
    double thetak(theta);
    for (int N=n/2;N;N/=2,thetak*=2){
      c_complex ww={1,0};
      c_complex wk={std::cos(thetak),std::sin(thetak)};
      for (int i=0;i<N;ww=ww*wk,++i){
	if (i%64==0){
	  ww.r=std::cos(i*thetak);
	  ww.i=std::sin(i*thetak);
	}
	W.push_back(ww);
      }
    }
    fft2(A,n,&W.front(),&T.front());
  }

bool c_fft(c_complex * x,int n,bool inverse){
  c_complex * X=(c_complex *) x;
  double theta=2*M_PI/n;
  if (!inverse)
    theta=-theta;
  fft2(X,n,theta);
  if (inverse){
    for (int i=0;i<n;++i)
      X[i]=X[i]/double(n);
  }
  return true;
}
//inline double absdouble(double x){ return x<0?-x:x;}
double abs(const c_complex & c){
  double X=absdouble(c.r),Y=absdouble(c.i);
  if (X==0 && Y==0) return 0;
  if (X<Y){
    X/=Y;
    return Y*sqrt(1+X*X);
  }
  Y/=X;
  return X*sqrt(1+Y*Y);
}

double norm(const c_complex & c){
  return c.r*c.r+c.i*c.i;
}

c_complex inv(const c_complex & a){
  double n=abs(a);
  c_complex c={a.r/n/n,-a.i/n/n};
  return c;
}

bool is_zero(const c_complex & a){
  return a.r==0 && a.i==0;
}

bool operator ==(const c_complex & a,const c_complex &b){
  return a.r==b.r && a.i==b.i;
}

bool operator !=(const c_complex & a,const c_complex &b){
  return a.r!=b.r || a.i!=b.i;
}

typedef vector< vector< c_complex> > cmatrice;
typedef vector< c_complex> cvecteur;

c_complex cdot(const cvecteur & v,const cvecteur & w){
  int n=v.size(),m=w.size();
  if (n>m) n=m;
  c_complex r={0,0};
  for (int i=0;i<n;++i)
    r += v[i]*w[i];
  return r;
}

bool cmult(const cmatrice & A,const cmatrice & B,cmatrice &C){
  int An=A.size(),Bn=B.size();
  if (!An || !Bn) return false;
  int Ac=A[0].size(),Bc=B[0].size();
  for (int i=0;i<An;++i){
    if (B[i].size()!=Bc)
      return false;
  }
  C.resize(An);
  for (int i=0;i<An;++i){
    const cvecteur & Ai=A[i];
    if (Ai.size()!=Ac)
      return false;
    cvecteur & Ci=C[i];
    Ci.resize(Bc);
    for (int j=0;j<Bc;++j){
      c_complex r={0,0};
      for (int k=0;k<Ac;++k){
	r += Ai[k]*B[k][j];
      }
      Ci[j]=r;
    }
  }
  return true;
}

// v1=v1+c2*v2 
void linear_combination(cvecteur & v1,const c_complex & c2,const cvecteur & v2,int cstart,int cend){
  if (!is_zero(c2)){
    cvecteur::iterator it1=v1.begin()+cstart,it1end=v1.end();
    if (cend && cend>=cstart && cend<it1end-v1.begin())
      it1end=v1.begin()+cend;
    cvecteur::const_iterator it2=v2.begin()+cstart;
    for (;it1!=it1end;++it1,++it2)
      *it1 += c2*(*it2);
  }
}

string print(const c_complex & c){
  char buf[32];
  sprint_double(buf,c.r);
  if (c.i==0)
    return buf;
  string s="(";
  s+=buf;
  s+=',';
  sprint_double(buf,c.i);
  s+=buf;
  s+=')';
  return s;
}

string print(const cvecteur & v){
  string s="[";
  for (int i=0;i<v.size();++i){
    s+=print(v[i]);
    s+=',';
  }
  s+=']';
  return s;
}

string print(const cmatrice & v){
  string s="[";
  for (int i=0;i<v.size();++i){
    s+=print(v[i]);
    s+=',';
  }
  s+=']';
  return s;
}

void crref(cmatrice & N,cvecteur & pivots,vector<int> & permutation,vector<int> & maxrankcols,c_complex & idet,int l, int lmax, int c,int cmax,int fullreduction,double eps,int rref_or_det_or_lu){
  bool use_cstart=!c;
  bool inverting=fullreduction==2;
  int linit=l;//,previous_l=l;
  // Reduction
  c_complex pivot,temp;
  // cvecteur vtemp;
  int pivotline,pivotcol;
  idet.r=1; idet.i=0;
  pivots.clear();
  pivots.reserve(cmax-c);
  permutation.clear();
  maxrankcols.clear();
  for (int i=0;i<lmax;++i)
    permutation.push_back(i);
  bool noswap=true;
  double epspivot=(eps<1e-13)?1e-13:eps;
  for (;(l<lmax) && (c<cmax);){
    pivot=N[l][c];
    if (abs(pivot)<epspivot)
      N[l][c].r=N[l][c].i=pivot.r=pivot.i=0;
    if (rref_or_det_or_lu==3 && is_zero(pivot)){
      idet.r=idet.i=0;
      return;
    }
    if ( rref_or_det_or_lu==1 && l==lmax-1 ){
      idet = (idet * pivot);
      break;
    }
    pivotline=l;
    pivotcol=c;
    noswap=false;
    // scan N current column for the best pivot available
    for (int ltemp=l+1;ltemp<lmax;++ltemp){
      temp=N[ltemp][c];
      if (abs(temp)<epspivot)
	temp.r=temp.i=N[ltemp][c].r=N[ltemp][c].i=0;
      if (abs(temp)>abs(pivot)){
	pivot=temp;
	pivotline=ltemp;
      }
    }
    if (!is_zero(pivot)){
      epspivot=eps*abs(pivot);
      maxrankcols.push_back(c);
      if (l!=pivotline){
	swap(N[l],N[pivotline]);
	swap(permutation[l],permutation[pivotline]);
	pivotline=l;
	idet = -idet;
      }
      // save pivot for annulation test purposes
      if (rref_or_det_or_lu!=1)
	pivots.push_back(pivot);
      // invert pivot 
      temp=inv(pivot);
      // multiply det
      idet = idet * pivot ;
      if (fullreduction || rref_or_det_or_lu<2){ // not LU decomp
	cvecteur::iterator it=N[pivotline].begin(),itend=N[pivotline].end();
	c_complex invpivot=inv(pivot);
	for (;it!=itend;++it){
	  *it = *it*invpivot;
	}
      }
      // if there are 0 at the end, ignore them in linear combination
      int effcmax=cmax-1;
      const cvecteur & Npiv=N[pivotline];
      for (;effcmax>=c;--effcmax){
	if (!is_zero(Npiv[effcmax]))
	  break;
      }
      ++effcmax;
      if (fullreduction && inverting && noswap)
	effcmax=giacmax(effcmax,c+1+lmax);
      // make the reduction
      if (fullreduction){
	for (int ltemp=linit;ltemp<lmax;++ltemp){
	  if (ltemp==l)
	    continue;
	  linear_combination(N[ltemp],-N[ltemp][pivotcol],N[l],(use_cstart?c:cmax),effcmax);
	}
      }
      else {
	for (int ltemp=l+1;ltemp<lmax;++ltemp){
	  if (rref_or_det_or_lu>=2) // LU decomp
	    N[ltemp][pivotcol] =  N[ltemp][pivotcol]*temp;
	  linear_combination(N[ltemp],-N[ltemp][pivotcol],N[l],(rref_or_det_or_lu>0)?(c+1):(use_cstart?c:cmax),effcmax);
	}
      } // end else
      // increment column number 
      ++c;
      // increment line number since reduction has been done
      ++l;	  
    } // end if (!is_zero(pivot)
    else { // if pivot is 0 increment col
      idet.r = idet.i=0;
      if (rref_or_det_or_lu==1)
	return;
      c++;
    }
  }
}

void c_complextab2cmatrice(c_complex * x,int n,int m,cmatrice & M){
  M.resize(n);
  for (int i=0;i<n;++i){
    M[i].resize(m);
    cvecteur & v=M[i];
    for (int j=0;j<m;++j){
      v[j]=*x; ++x;
    }
  }
}

void cmatrice2c_complextab(const cmatrice &M,c_complex * x){
  int n=M.size();
  for (int i=0;i<n;++i){
    const cvecteur & v=M[i];
    int m=v.size();
    for (int j=0;j<m;++j){
      *x=v[j];
      ++x;
    }
  }
}

void c_complextab2cvecteur(c_complex * x,int n,cvecteur & v){
  v.resize(n);
  for (int j=0;j<n;++j){
    v[j]=*x; ++x;
  }
}

void cvecteur2c_complextab(const cvecteur &v,c_complex * x){
  int m=v.size();
  for (int j=0;j<m;++j){
    *x=v[j];
    ++x;
  }
}

// add identity matrix, modifies arref in place
void add_identity(cmatrice & arref){
  int s=int(arref.size());
  for (int i=0;i<s;++i){
    cvecteur &v=arref[i];
    v.reserve(2*s);
    for (int j=0;j<s;++j){
      c_complex c={i==j?1.0:0.0,0};
      v.push_back(c);
    }
  }
}

void cidn(cmatrice & m){
  int s=int(m.size());
  for (int i=0;i<s;++i){
    cvecteur &v=m[i];
    v.clear();
    for (int j=0;j<s;++j){
      c_complex c={i==j?1.0:0.0,0};
      v.push_back(c);
    }
  }
}

bool remove_identity(cmatrice & res){
  int s=int(res.size());
  // "shrink" res
  for (int i=0;i<s;++i){
    cvecteur & v = res[i];
    if (is_zero(v[i]))
      return false;
    c_complex p=inv(v[i]);
    cvecteur d(s);
    for (int j=0;j<s;++j)
      d[j]=p*v[s+j];
    res[i].swap(d);
  }
  return true;
}

cmatrice companion(const cvecteur & w){
  cvecteur v(w);
  int s=int(v.size())-1;
  if (s<=0)
    return cmatrice(0);
  c_complex v0=inv(v[0]);
  cmatrice m;
  m.reserve(s);
  for (int i=0;i<s;++i){
    cvecteur w(s);
    w[s-1]=-v0*v[s-i];
    if (i>0)
      w[i-1].r=1;
    m.push_back(w);
  }
  return m;
}

bool cinv(cmatrice &M){
  int n=M.size();
  add_identity(M);
  cvecteur pivots; vector<int> perm,maxrankcols; c_complex idet;
  crref(M,pivots,perm,maxrankcols,idet,0,n,0,2*n,2,1e-13,0);
  if (abs(idet)<1e-13)
    return false;
  remove_identity(M);
  return true;
}

c_complex sqrt(const c_complex & c){
  double r=c.r,i=c.i;
  if (c.i==0) {
    if (c.r<0){
      c_complex res={0,sqrt(-c.r)}; return res;      
    }
    c_complex res={sqrt(c.r),0}; return res;
  }
  double rho=abs(c);
  double rrho=r<0?i*i/(rho-r):(rho+r); // accuracy if r<0
  double sqrtr=sqrt(rrho/2);
  double sqrti=i*sqrtr/rrho;
  c_complex res={sqrtr,sqrti};
  return res;
}

c_complex conj(const c_complex & c){
  c_complex C={c.r,-c.i};
  return C;
}

double real(const c_complex &c){
  return c.r;
}

double imag(const c_complex &c){
  return c.i;
}

bool ctrn(const cmatrice & M){
  int n=M.size();
  if (!n) return false;
  int c=M[0].size();
  for (int i=0;i<n;++i)
    if (M[i].size()!=c)
      return false;
  cmatrice T(c);
  for (int i=0;i<c;++i){
    cvecteur &Ti=T[i];
    Ti.resize(n);
    for (int j=0;j<n;++j){
      Ti[j]=conj(M[j][i]);
    }
  }
  return true;
}

  // conj(a)*A+conj(c)*C->C
  // c*A-a*C->A
  void bi_linear_combination( c_complex  a,vector< c_complex > & A, c_complex  c,vector< c_complex > & C,int cstart,int cend){
    c_complex  * Aptr=&A.front()+cstart;
    c_complex  * Cptr=&C.front()+cstart,* Cend=Cptr+(cend-cstart);
    c_complex ac=conj(a),cc=conj(c);
    for (;Cptr!=Cend;++Aptr,++Cptr){
      c_complex  tmp=c*(*Aptr)-a*(*Cptr);
      *Cptr=ac*(*Aptr)+cc*(*Cptr);
      *Aptr=tmp;
    }
  }

  void hessenberg_ortho(cmatrice & H,cmatrice & P,int firstrow,int n,bool compute_P,int already_zero){
    int nH=int(H.size());
    if (n<0 || n>nH) 
      n=nH;
    if (firstrow<0 || firstrow>n)
      firstrow=0;
    c_complex  t,u,tc,uc;
    double norme;
    for (int m=firstrow;m<n-2;++m){
      // if initial Hessenberg check for a non zero coeff in the column m below ligne m+1
      int i=m+1;
      int nend=n;
      if (already_zero){
	if (i+already_zero<n)
	  nend=i+already_zero;
      }
      else {
	double pivot=0;
	int pivotline=0;
	for (;i<nend;++i){
	  double t=abs(H[i][m]);
	  if (t>pivot){
	    pivotline=i;
	    pivot=t;
	  }
	}
	if (pivot==0)
	  continue;
	i=pivotline;
	// exchange line and columns
	if (i>m+1){
	  swap(H[i],H[m+1]);
	  if (compute_P)
	    swap(P[i],P[m+1]);
	  for (int j=0;j<n;++j){
	    vector< c_complex > & Hj=H[j];
#ifdef VISUALC
	    c_complex cc=Hj[i];
	    Hj[i]=Hj[m+1];
	    Hj[m+1]=cc;
#else
	    swap< c_complex >(Hj[i],Hj[m+1]);
#endif
	  }
	}
      }
      // now coeff at line m+1 column m is H[m+1][m]=t!=0
      for (i=m+2;i<nend;++i){
	u=H[i][m];
	if (is_zero(u))
	  continue;
	// line operation
	t=H[m+1][m];
	norme=std::sqrt(norm(u)+norm(t));
	u=u/norme; t=t/norme;
	uc=conj(u); tc=conj(t);
	// H[m+1]=uc*H[i]+tc*H[m+1] and H[i]=t*H[i]-u*H[m+1];
	bi_linear_combination(u,H[i],t,H[m+1],m,nH);
	// column operation:
	int nstop=already_zero?nend+already_zero-1:nH;
	if (nstop>nH)
	  nstop=nH;
	cmatrice::iterator Hjptr=H.begin(),Hjend=Hjptr+nstop;
	for (;Hjptr!=Hjend;++Hjptr){
	  c_complex  *Hj=&Hjptr->front();
	  c_complex  Hjm=Hj[m+1],Hji=Hj[i];
	  Hj[i]=-uc*Hjm+tc*Hji;
	  Hj[m+1]=t*Hjm+u*Hji;
	}
	if (compute_P){
	  bi_linear_combination(u,P[i],t,P[m+1],0,nH);
	}
      } // for i=m+2...
    } // for int m=firstrow ...
  }

  // a*A+c*C->A
  // c*A-a*C->C
  void bi_linear_combination(double a,vector< c_complex > & A,c_complex c,vector< c_complex > & C){
    c_complex * Aptr=&A.front();
    c_complex * Cptr=&C.front(),* Cend=Cptr+C.size();
    c_complex cc=conj(c);
    for (;Cptr!=Cend;++Aptr,++Cptr){
      c_complex tmp=a*(*Aptr)+cc*(*Cptr);
      *Cptr=c*(*Aptr)-a*(*Cptr);
      *Aptr=tmp;
    }
  }

  void francis_iterate1(cmatrice & H,int n1,int n2,cmatrice & P,double eps,bool compute_P,c_complex l1,bool finish){
    int n_orig=int(H.size());
    c_complex x,y,yc;
    if (finish){
      // [[a,b],[c,d]] -> [b,l1-a] or [l1-d,c] as first eigenvector
      c_complex a=H[n2-2][n2-2],b=H[n2-2][n2-1],c=H[n2-1][n2-2],d=H[n2-1][n2-1];
      c_complex l1a=l1-a,l1d=l1-d;
      if (abs(l1a)>abs(l1d)){
	x=b; y=l1a;
      }
      else {
	x=l1d; y=c;
      }
    }
    else {
      x=H[n1][n1]-l1,y=H[n1+1][n1];
      if (abs(x)<eps && abs(y-1.0)<eps){
	x.r = double(rand())/RAND_MAX;
	x.i=0;
      }
    }
    // make x real
    double xr=real(x),xi=imag(x),yr=real(y),yi=imag(y),X;
    X = std::sqrt(xr*xr+xi*xi);
    if (X!=0){
      // gen xy = gen(xr/x,-xi/x); y=y*xy;
      y.r=(yr*xr+yi*xi)/X; y.i=(yi*xr-yr*xi)/X; 
      yr=real(y); yi=imag(y);
    }
    double xy=std::sqrt(X*X+yr*yr+yi*yi);
    // normalize eigenvector
    X = X/xy; y = y/xy;	yc=conj(y);
    // compute reflection matrix such that Q*[1,0]=[x,y]
    // hence column 1 is [x,y] and column2 is [conj(y),-x]
    // apply Q on H and P: line operations on H and P
    // c_complex c11=x, c12=conj(y,contextptr),
    //                 c21=y, c22=-x;
    // apply Q on H and P: line operations on H and P
    bi_linear_combination(X,H[n1],y,H[n1+1]);
    if (compute_P)
      bi_linear_combination(X,P[n1],y,P[n1+1]);
    // now columns operations on H (not on P)
    for (int j=0;j<n_orig;++j){
      vector< c_complex > & Hj=H[j];
      c_complex & Hjm1=Hj[n1];
      c_complex & Hjm2=Hj[n1+1];
      c_complex tmp1=Hjm1*X+Hjm2*y; // tmp1=Hjm1*c11+Hjm2*c21;
      Hjm2=Hjm1*yc-Hjm2*X; // tmp2=Hjm1*c12+Hjm2*c22;
      Hjm1=tmp1;
    }
    hessenberg_ortho(H,P,n1,n2,compute_P,2); 
  }

  bool in_francis_schur(cmatrice & H,int n1,int n2,cmatrice & P,int maxiter,double eps,bool compute_P,cmatrice & Haux,bool only_one);

  void francis_iterate2(cmatrice & H,int n1,int n2,cmatrice & P,double eps,bool compute_P,cmatrice & Haux,bool only_one){
    // int n_orig(H.size());
    // now H is proper hessenberg (indices n1 to n2-1)
    c_complex s=H[n2-1][n2-1]; 
    double ok=abs(H[n2-1][n2-2])/abs(H[n2-1][n2-1]);
    if (n2-n1==2 ||(ok>1e-1 && n2-n1>2 && abs(H[n2-2][n2-3])<1e-2*abs(H[n2-2][n2-2]))){
      c_complex a=H[n2-2][n2-2],b=H[n2-2][n2-1],c=H[n2-1][n2-2],d=H[n2-1][n2-1];
      c_complex delta=a*a-2*a*d+d*d+4*b*c;
      delta=sqrt(delta);
      c_complex l1=(a+d+delta)/2.0;
      // c_complex l2=(a+d-delta)/2.0;
      s=l1;
    }
    francis_iterate1(H,n1,n2,P,eps,compute_P,s,false);
  }

  // EIGENVALUES 
  bool eigenval2(cmatrice & H,int n2,c_complex & l1, c_complex & l2){
    c_complex a=H[n2-2][n2-2],b=H[n2-2][n2-1],c=H[n2-1][n2-2],d=H[n2-1][n2-1];
    c_complex delta=a*a-2*a*d+d*d+4*b*c;
    delta=sqrt(delta);
    l1=(a+d+delta)/2; 
    l2=(a+d-delta)/2; 
    return true;
  }

  bool in_francis_schur(cmatrice & H,int n1,int n2,cmatrice & P,int maxiter,double eps,bool compute_P,cmatrice & Haux,bool only_one){
    if (n2-n1<=1)
      return true; // nothing to do
    if (n2-n1==2){ // 2x2 submatrix, we know how to diagonalize
      c_complex l1,l2;
      if (eigenval2(H,n2,l1,l2)){
	francis_iterate1(H,n1,n2,P,eps,compute_P,l1,true);
      }
      return true;
    }
    for (int niter=0;n2-n1>1 && niter<maxiter;niter++){
      //xcas::dConsolePut(("niter "+print_INT_(niter)+" "+print(H)).c_str()); xcas::Console_NewLine(xcas::LINE_TYPE_OUTPUT,1);
      // check if one subdiagonal element is sufficiently small, if so 
      // we can increase n1 or decrease n2 or split
      double ratio,coeff=1;
      if (niter>maxiter-3)
	coeff=100;
      for (int i=n2-2;i>=n1;--i){
	ratio=abs(H[i+1][i])/abs(H[i][i]);
	if (ratio<coeff*eps){ 
	  // do a final iteration if i==n2-2 or n2-3? does not improve much precision
	  // if (i>=n2-3) francis_iterate2(H,n1,n2,P,eps,true,complex_schur,compute_P,v1,v2);
	  // submatrices n1..i and i+1..n2-1
	  if (only_one && n2-(i+1)<=2)
	    return true;
	  if (!only_one && !in_francis_schur(H,n1,i+1,P,maxiter,eps,compute_P,Haux,only_one)){
	    in_francis_schur(H,i+1,n2,P,maxiter,eps,compute_P,Haux,only_one);
	    return false;
	  }
	  return in_francis_schur(H,i+1,n2,P,maxiter,eps,compute_P,Haux,only_one);
	}
      }
      francis_iterate2(H,n1,n2,P,eps,compute_P,Haux,only_one);
    } // end for loop on niter
    return false;
  }

  // Francis algorithm on submatrix rows and columns n1..n2-1
  // Invariant: trn(P)*H*P=orig matrix, complex_schur not used for giac_double coeffs
  bool francis_schur(cmatrice & H,int n1,int n2,cmatrice & P,int maxiter,double eps,bool is_hessenberg,bool compute_P){
    int n_orig=int(H.size());//,nitershift0=0;
    if (!is_hessenberg){
      hessenberg_ortho(H,P,0,n_orig,compute_P,0); // insure Hessenberg form (on the whole matrix)
    }
    cmatrice Haux(n2/2);
    return in_francis_schur(H,n1,n2,P,maxiter,eps,compute_P,Haux,false);
  }

bool schur_eigenvalues(cmatrice &d,double eps){
  int dim=d.size();
    bool ans=true;
    for (int i=0;i<dim;++i){
      cvecteur & di= d[i];
      for (int j=0;j<dim;++j){
	if (j==i) continue;
	if (ans && j==i-1 && abs(di[j])/abs(di[j+1])>eps){
	  // *logptr(contextptr) << gettext("Low accuracy for Schur row ") << j << " " << d[i] << '\n';
	  ans=false;
	}
	di[j].r=di[j].i=0;
      }
    }
    return ans;
}

  // input trn(p)*d*p=original matrix, d upper triangular
  // output p*d*inv(p)=original matrix, d diagonal
  bool schur_eigenvectors(cmatrice &p,cmatrice & d,double eps){
    int dim=int(p.size());
    cmatrice m(dim);
    cidn(m);
    // columns of m are the vector of the basis of the Schur decomposition
    // in terms of the eigenvector
    for (int k=1;k<dim;++k){
      // compute column k of m
      for (int j=0;j<k;++j){
	c_complex tmp={0,0};
	for (int i=0;i<k;++i){
	  tmp += d[i][k]*m[j][i];
	}
	if (!is_zero(tmp)) 
	  tmp = tmp*inv(d[j][j]-d[k][k]);
	m[j][k]=tmp;
      }
    }
    if (!cinv(m))
      return false;
    ctrn(p);
    cmatrice pm;
    cmult(p,m,pm);
    swap(p,pm);
    // set d to its diagonal
    return schur_eigenvalues(d,eps);
  }

bool c_pcoeff(c_complex * x,int n){
  c_complex tab[n+1];
  tab[0].r=1; tab[0].i=0; // init tab to polynomial 1
  for (int i=0;i<n;++i){
    // tab:=tab*(X-x[i]): leading coeff unchanged
    tab[i+1].r=tab[i+1].i=0;
    c_complex & xi=x[i];
    for (int j=i;j>=0;--j){
      tab[j+1] -= tab[j]*xi;
    }
  }
  // copy result in x
  for (int i=0;i<=n;++i)
    x[i]=tab[i];
  return true;
}

#if 1
bool c_rref(c_complex * x,int n,int m){
  cmatrice M;
  c_complextab2cmatrice(x,n,m,M);
  cvecteur pivots; vector<int> perm,maxrankcols; c_complex idet;
  crref(M,pivots,perm,maxrankcols,idet,0,n,0,m,1,1e-13,0);
  cmatrice2c_complextab(M,x);
  return true;
}

c_complex c_det(c_complex *x,int n){
  cmatrice M;
  c_complextab2cmatrice(x,n,n,M);
  cvecteur pivots; vector<int> perm,maxrankcols; c_complex idet;
  crref(M,pivots,perm,maxrankcols,idet,0,n,0,n,0,1e-13,1);
  return idet;
}

bool c_inv(c_complex * x,int n){
  cmatrice M;
  c_complextab2cmatrice(x,n,n,M);
  if (!cinv(M))
    return false;
  cmatrice2c_complextab(M,x);
  return true;
}

bool c_eig(c_complex * x,c_complex * d,int n){
  cmatrice H;
  c_complextab2cmatrice(x,n,n,H);
  // load identity
  cmatrice P(n); c_complex z={0,0};
  for (int i=0;i<n;++i){
    P[i]=vector<c_complex>(n,z);
    P[i][i].r=1;
  }
  double eps=1e-11;
  if (!francis_schur(H,0,n,P,100,eps,false,true))
    return false;
  if (!schur_eigenvectors(P,H,eps))
    return false;
  cmatrice2c_complextab(H,d);
  cmatrice2c_complextab(P,x);  
  return true;
}

bool c_egv(c_complex * x,int n){
  cmatrice H;
  c_complextab2cmatrice(x,n,n,H);
  cmatrice P(n); 
  double eps=1e-11;
  if (!francis_schur(H,0,n,P,100,eps,false,false))
    return false;
  if (!schur_eigenvalues(H,eps))
    return false;
  cmatrice2c_complextab(H,x);
  return true;
}

bool c_proot(c_complex * x,int n){
  cvecteur v;
  c_complextab2cvecteur(x,n,v);
  cmatrice H(companion(v));
  n--; // size -> degree
  cmatrice P(n); 
  double eps=1e-11;
  bool dbg=false;
  if (dbg) xcas::dConsolePut(print(v).c_str()); 	xcas::Console_NewLine(xcas::LINE_TYPE_OUTPUT,1);
  if (!francis_schur(H,0,n,P,100,eps,true,false)) // companion is Hessenberg
    return false;
  if (dbg) xcas::dConsolePut(print(H).c_str()); 	xcas::Console_NewLine(xcas::LINE_TYPE_OUTPUT,1);
  if (!schur_eigenvalues(H,eps))
    return false;
  if (dbg) xcas::dConsolePut(print(H).c_str()); 	xcas::Console_NewLine(xcas::LINE_TYPE_OUTPUT,1);
  // copy diag of H in x
  for (int i=0;i<n;++i,++x){
    *x=H[i][i];
  }
  return true;
}

#else
bool c_rref(c_complex * x,int n,int m){
  giac::matrice M(n);
  c_complexptr2matrice(x,n,m,M);
  giac::gen g=giac::_rref(M,giac::context0);
  if (g.type!=giac::_VECT)
    return false;
  return matrice2c_complexptr(*g._VECTptr,x);
}

c_complex c_det(c_complex *x,int n){
  giac::matrice M(n);
  c_complexptr2matrice(x,n,n,M);
  giac::gen g=giac::mdet(M,giac::context0);
  g=giac::evalf_double(g,1,giac::context0);
  return gen2c_complex(g);
}

bool c_inv(c_complex * x,int n){
  giac::matrice M(n);
  c_complexptr2matrice(x,n,n,M);
  M=giac::minv(M,giac::context0);
  return matrice2c_complexptr(M,x);
}

bool c_eig(c_complex * x,c_complex * d,int n){
  giac::matrice M(n);
  c_complexptr2matrice(x,n,n,M);
  gen g=giac::_jordan(M,giac::context0);
  if (g.type!=_VECT || g._VECTptr->size()!=2 || !ckmatrix(g[0]) || !ckmatrix(g[1]))
    return false;
  return matrice2c_complexptr(*g[0]._VECTptr,x) && matrice2c_complexptr(*g[1]._VECTptr,d);
}

bool c_egv(c_complex * x,int n){
  giac::matrice M(n);
  c_complexptr2matrice(x,n,n,M);
  gen g=giac::_egv(M,giac::context0);
  if (!ckmatrix(g))
    return false;
  return matrice2c_complexptr(*g._VECTptr,x);
}

bool c_proot(c_complex * x,int n){
  giac::matrice M(n);
  c_complexptr2matrice(x,n,0,M);
  M=giac::proot(M);
  return matrice2c_complexptr(M,x);
}

/*
bool c_pcoeff(c_complex * x,int n){
  giac::matrice M(n);
  c_complexptr2matrice(x,n,0,M);
  M=giac::pcoeff(M);
  return matrice2c_complexptr(M,x);
}
*/

#endif

void c_sprint_double(char * s,double d){
  giac::sprint_double(s,d);
}

static void c_update_turtle_state(bool clrstring){
#if defined NUMWORKS && defined DEVICE
  if (!ck_turtle_size()){
    ctrl_c=true; interrupted=true;
    return;
  }
#endif
  if (clrstring)
    (*turtleptr).s=-1;
  (*turtleptr).theta = (*turtleptr).theta - floor((*turtleptr).theta/360)*360;
  if (!turtle_stack().empty()){
    logo_turtle & t=turtle_stack().back();
    if (t.equal_except_nomark(*turtleptr)){
      t.theta=turtleptr->theta;
      t.mark=turtleptr->mark;
      t.visible=turtleptr->visible;
      t.color=turtleptr->color;
    }
    else
      turtle_stack().push_back((*turtleptr));
  }
  else
    turtle_stack().push_back((*turtleptr));    
}

void c_turtle_clear(int clrpos){
  turtle_stack().clear();
  if (clrpos) (*turtleptr) = logo_turtle();
  c_update_turtle_state(true);
}

void c_turtle_forward(double d){
  (*turtleptr).x += d * std::cos((*turtleptr).theta*deg2rad_d);
  (*turtleptr).y += d * std::sin((*turtleptr).theta*deg2rad_d) ;
  (*turtleptr).radius = 0;
  c_update_turtle_state(true);
  py_ck_ctrl_c();
}

void c_turtle_left(double d){
  (*turtleptr).theta += d;
  (*turtleptr).radius = 0;
  c_update_turtle_state(true);
  py_ck_ctrl_c();
}

void c_turtle_up(int i){
  if (i)
    (*turtleptr).mark = false;
  else
    (*turtleptr).mark = true;
  c_update_turtle_state(true);
  py_ck_ctrl_c();
}

void c_turtle_goto(double x,double y){
  (*turtleptr).x=x;
  (*turtleptr).y=y;
  (*turtleptr).radius = 0;
  c_update_turtle_state(true);
  py_ck_ctrl_c();
}

void c_turtle_cap(double x){
  (*turtleptr).theta=x;
  (*turtleptr).radius = 0;
  c_update_turtle_state(true);
  py_ck_ctrl_c();
}

int c_turtle_getcap(){
  return (*turtleptr).theta;
}

int c_turtle_crayon(int i){
  if (i==-128)
    return (*turtleptr).turtle_width;
  if (i<0)
    (*turtleptr).turtle_width=-i;
  else
    (*turtleptr).color=i;
  c_update_turtle_state(true);
  py_ck_ctrl_c();
  return 0;
}

int c_find_radius(int & r,int & t1,int & t2,int &direct){
  direct=r>=0;
  if (r<0) r=-r;
  if (r>512) r=512;
  return r | (t1 << 9) | (t2 << 18 );
}

void c_turtle_rond(int r,int t1,int t2){
  int direct;
  int radius=c_find_radius(r,t1,t2,direct);
  (*turtleptr).radius=radius;
  (*turtleptr).direct=direct;
  while (t1<0)
    t1 += 360;
  while (t2<0)
    t2 += 360;
  c_turtle_move(r,t2);
  c_update_turtle_state(true);
  py_ck_ctrl_c();
}

void c_turtle_disque(int r,int t1,int t2,int centre){
  int direct,radius=c_find_radius(r,t1,t2,direct);
  if (centre){
    // saute(r); tourne_gauche(direct?90:-90)
  }
  (*turtleptr).radius=radius;
  (*turtleptr).direct=direct;
  c_turtle_move(r,t2);
  (*turtleptr).radius += 1 << 27;
  c_update_turtle_state(true);
  if (centre){
    // _tourne_droite(direct?90:-90,contextptr); _saute(-r,contextptr);
  }
  py_ck_ctrl_c();
}
int turtle_fillbegin=-1,turtle_fillcolor=_BLACK;

void c_turtle_fill(int i){
  if (i==1){
    turtle_fillbegin=turtle_stack().size();
    return;
  }
  int c=turtleptr->color;
  c_turtle_crayon(turtle_fillcolor);
  int n=turtle_stack().size()- turtle_fillbegin;
  turtle_fillbegin=-1;
  turtleptr->radius=-absint(n);
  c_update_turtle_state(true);
  if (turtle_fillcolor>=0){
    turtleptr->radius=0;
    c_turtle_crayon(c);
  }
  py_ck_ctrl_c();
}

int rgb(int r,int g,int b){
  if (r<0) r=0; if(r>255) r=255;
  if (g<0) g=0; if(g>255) g=255;
  if (b<0) b=0; if(b>255) b=255;
  return (((r*32)/256)<<11) | (((g*64)/256)<<5) | (b*32/256);

}
void c_turtle_fillcolor(double r,double g,double b,int entier){
  if (entier)
    turtle_fillcolor=rgb(int(r),int(g),int(b));
  else
    turtle_fillcolor=rgb(int(r*256),int(g*256),int(b*256));
  py_ck_ctrl_c();
}

void c_turtle_getposition(double * x,double * y){
  *x=turtleptr->x;
  *y=turtleptr->y;
}

void c_turtle_show(int visible){
  (*turtleptr).visible=visible;
  (*turtleptr).radius = 0;
  c_update_turtle_state(true);
}

void c_turtle_towards(double x,double y){
  double x0=turtleptr->x,y0=turtleptr->y;
  double t=atan2(x-x0,y-y0);
  c_turtle_cap(t*180/M_PI);
}

int c_turtle_getcolor(){
  return turtleptr->color;
}

void c_turtle_color(int c){
  turtleptr->color=c;
  (*turtleptr).radius = 0;
  c_update_turtle_state(true);  
}

void c_turtle_fillcolor1(int c){
  turtle_fillcolor=c;
}

// auto-shutdown
int do_shutdown(){
  xcas::save_console_state_smem("session.xw.tns",false,giac::context0);
#ifdef NO_STDEXCEPT
  return 1;
#else
  throw autoshutdown();
#endif
}

// string translations
#ifdef NUMWORKS
#include "numworks_translate.h"
#else
#include "aspen_translate.h"
#endif
bool tri2(const char4 & a,const char4 & b){
  int res= strcmp(a[0],b[0]);
  return res<0;
}

int giac2aspen(int lang){
  switch (lang){
  case 0: case 2:
    return 1;
  case 1:
    return 3;
  case 3:
    return 5;
  case 6:
    return 7;
  case 8:
    return 2;
  case 5:
    return 4;
  }
  return 0;
}

const char * gettext(const char * s) { 
  // 0 and 2 english 1 french 3 sp 4 el 5 de 6 it 7 tr 8 zh 9 pt
  int aspenlang=giac2aspen(lang);
  char4 s4={s};
  std::pair<char4 * const,char4 *const> pp=equal_range(aspen_giac_translations,aspen_giac_translations+aspen_giac_records,s4,tri2);
  if (pp.first!=pp.second && 
      pp.second!=aspen_giac_translations+aspen_giac_records &&
      (*pp.first)[aspenlang]){
    return (*pp.first)[aspenlang];
  }
  return s;
}

  void process_freeze(){
    if (freezeturtle){
      xcas::displaylogo();
      freezeturtle=false;
      return;
    }
    if (giac::freeze){
      giac::freeze=false;
      for (;;){
#ifdef NSPIRE_NEWLIB
	DefineStatusMessage((char*)((lang==1)?"Ecran fige. Taper esc":"Screen frozen. Press esc."), 1, 0, 0);
#else
	DefineStatusMessage((char*)((lang==1)?"Ecran fige. Taper clear":"Screen frozen. Press clear."), 1, 0, 0);
#endif
	DisplayStatusArea();
	int key;
	GetKey(&key);
	if (key==KEY_CTRL_EXIT || key==KEY_CTRL_AC)
	  break;
      }
    }
  }    

#ifdef HP39
giac::context * contextptr=0; 
extern "C" void SetQuitHandler( void (*callback)(void) ); // syscalls.h
void quit_save_session(){
  xcas::save_session(contextptr);
}
int kcas_main(int isAppli, unsigned short OptionNum)
{ 
  size_t rambase=0x02000000+4096; // 4096 for 1 bpp screen buf
  tab16=(four_int*) rambase;     // ALLOC16*16=4096 ALLOC16=256
#if 1
  tab24=(six_int*) ((size_t) tab16 +4096);    // ALLOC24*24 ALLOC24=16*24
  tab48=(twelve_int*) ((size_t) tab24+16*32*24); // kgen.cc ALLOC48*48=2*4096, ALLOC48=128
#else  
  // tab16=(four_int*) malloc(4096);    // ALLOC16=256, ALLOC16*16=4K
  tab24=(six_int*) malloc(16*32*24);    // ALLOC24=16*32, ALLOC24*24 =12K
  tab48=(twelve_int*) malloc(2*4096); // kgen.cc ALLOC48=8*32, ALLOC48*48=12K
#endif
  unsigned int key;
  char *expr;

  int i = 0, j = 0;

  SetQuitHandler(quit_save_session); // automatically save session when exiting

  turtle();
#ifdef TURTLETAB
  turtle_stack_size = 0;
#else
  turtle_stack(); // required to init turtle
#endif

  context ct;
  contextptr = &ct;
  xcas::Console_Init(contextptr);
  giac::_srand(vecteur(0), contextptr);
  xcas::restore_session("session", contextptr);
  // load_config();
  xcas::Console_Disp(1,contextptr);
  //init_locale();
  lang = 0;
  i = 0;

  while (1)
  {
    
    if ((expr = xcas::Console_GetLine(contextptr)) == NULL){
      confirm("memory error","");
      break;
    }
    if (strcmp((const char *)expr, "restart") == 0)
    {
      if (confirm(lang ? "Effacer variables?" : "Clear variables?", lang ? "F1: annul,  F6: confirmer" : "F1: cancel,  F6: confirm") != KEY_CTRL_F6)
      {
        xcas::Console_Output((const char *)" cancelled");
        xcas::Console_NewLine(xcas::LINE_TYPE_OUTPUT, 1);
        // ck_getkey((int *)&key);
        xcas::Console_Disp(1,contextptr);
        continue;
      }
    }
    // should save in another file
    if (strcmp((const char *)expr, "=>") == 0 || strcmp((const char *)expr, "=>\n") == 0)
    {
      xcas::save_session(contextptr);
      xcas::Console_Output("Session saved");
    }
    else
      xcas::run((char *)expr,7,contextptr);
    // print_mem_info();
    xcas::Console_NewLine(xcas::LINE_TYPE_OUTPUT, 1);
    // ck_getkey((int *)&key);
    xcas::Console_Disp(1,contextptr);
  }
  for (;;)
    GetKey((int *)&key);
  return 1;
}

#endif // hp39

#endif // KHICAS
