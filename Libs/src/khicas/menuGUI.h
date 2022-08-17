#ifndef __MENUGUI_H
#define __MENUGUI_H

#include "libfx.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MENUITEM_NORMAL 0
#define MENUITEM_CHECKBOX 1
#define MENUITEM_SEPARATOR 2
#define MENUITEM_VALUE_NONE 0
#define MENUITEM_VALUE_CHECKED 1
  struct MenuItem {
    char* text; // text to be shown on screen. mandatory, must be a valid pointer to a string.
    int token:20; // for syntax help on keywords not in the catalog
    int type:4; // type of the menu item. use MENUITEM_* to set this
    int value:4; // value of the menu item. For example, if type is MENUITEM_CHECKBOX and the checkbox is checked, the value of this var will be MENUITEM_VALUE_CHECKED
    int isselected:4; // for file browsers and other multi-select screens, this will show an arrow before the item
    short int isfolder; // for file browsers, this will signal the item is a folder
    signed char color; // color of the menu item (use TEXT_COLOR_* to define)
    // The following two settings require the menu type to be set to MENUTYPE_MULTISELECT
#if 0
    signed char icon=-1; //for file browsers, to show a file icon. -1 shows no icon (default)
#endif
    MenuItem():token(0),type(MENUITEM_NORMAL),value(MENUITEM_VALUE_NONE),isselected(0),isfolder(0),color(TEXT_COLOR_BLACK) {}
  } ;

typedef struct
{
  unsigned short data[0x12*0x18];
} MenuItemIcon;

#define MENUTYPE_NORMAL 0
#define MENUTYPE_MULTISELECT 1
#define MENUTYPE_INSTANT_RETURN 2 // this type of menu insantly returns even if user hasn't selected an option (allows for e.g. redrawing the GUI behind it). if user hasn't exited or selected an option, menu will return MENU_RETURN_INSTANT
#define MENUTYPE_NO_KEY_HANDLING 3 //this type of menu doesn't handle any keys, only draws.
#define MENUTYPE_FKEYS 4 // returns GetKey value of a Fkey when one is pressed
typedef struct {
  char* statusText = NULL; // text to be shown on the status bar, may be empty
  char* title = NULL; // title to be shown on the first line if not null
  char* subtitle = NULL;
  int titleColor=TEXT_COLOR_BLUE; //color of the title
  char* nodatamsg; // message to show when there are no menu items to display
  int startX=0; //X where to start drawing the menu. NOTE this is not absolute pixel coordinates but rather character coordinates
  int startY=0; //Y where to start drawing the menu. NOTE this is not absolute pixel coordinates but rather character coordinates
  int width=32; // NOTE this is not absolute pixel coordinates but rather character coordinates 
  int height=8; // NOTE this is not absolute pixel coordinates but rather character coordinates 
  int scrollbar=1; // 1 to show scrollbar, 0 to not show it.
  int scrollout=0; // whether the scrollbar goes out of the menu area (1) or it overlaps some of the menu area (0)
  int numitems; // number of items in menu
  int type=MENUTYPE_NORMAL; // set to MENUTYPE_* .
  int selection=1; // currently selected item. starts counting at 1
  int scroll=0; // current scrolling position
  int fkeypage=0; // for MULTISELECT menu if it should allow file selecting and show the fkey label
  int numselitems=0; // number of selected items
  int returnOnInfiniteScrolling=0; //whether the menu should return when user reaches the last item and presses the down key (or the first item and presses the up key)
  int darken=0; // for dark theme on homeGUI menus
  int miniMiniTitle=0; // if true, title will be drawn in minimini. for calendar week view
  int pBaRtR=0; //preserve Background And Return To Redraw. Rarely used
  MenuItem* items; // items in menu
} Menu;

#define MENU_RETURN_EXIT 0
#define MENU_RETURN_SELECTION 1
#define MENU_RETURN_INSTANT 2
#define MENU_RETURN_SCROLLING 3 //for returnOnInfiniteScrolling

typedef struct {
  const char* name;
  const char* insert;
  const char* desc;
  const char * example;
  const char * example2;
  int category;
} catalogFunc;

int showCatalog(char* insertText,int preselect=0,int menupos=0);
int doMenu(Menu* menu, MenuItemIcon* icontable=NULL);
void reset_alpha();
// category=0 for CATALOG, 1 for OPTN
// returns 0 on exit, 1 on success
int doCatalogMenu(char* insertText, const char* title, int category,const char * cmdname=0);
extern const char shortcuts_string[];
extern const char apropos_string[];
void init_locale();
#endif
