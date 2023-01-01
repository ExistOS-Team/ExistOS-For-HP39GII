#include "giacPCH.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "kdisplay.h"
#include "file.h"
#include "syscalls.h"

//!!
#include "porting.h"
#include <alloca.h> 
using namespace giac;
// FIXME!!!
int matchmask(const char * mask,const char * filename){
  if (strlen(mask)==4 && mask[0]=='*' && mask[1]=='.'){
    int s=strlen(filename);
    if (s<4) return 0;
    return filename[s-3]=='.' && filename[s-2]==mask[2] && filename[s-1]==mask[3];
  }
  return 1;
}

int compareFileStructs(File* f1, File* f2, int type) {
  if(f1->isfolder < f2->isfolder) return 1;
  else if(f1->isfolder > f2->isfolder) return -1;
  switch(type) {
    case 1:
      return strcmp( f1->filename, f2->filename );
    case 2:
      return -strcmp( f1->filename, f2->filename );
    case 3:
      return f1->size-f2->size;
    case 4:
    default:
      return f2->size-f1->size;
  }
}

void insertSortFileMenuArray(File* data, MenuItem* mdata, int size) {
  int sort = 1;//GetSetting(SETTING_FILE_MANAGER_SORT);
  if(!sort) return;
  int i, j;
  File temp;
  MenuItem mtemp;

  for(i = 1; i < size; i++) {
    temp = data[i];
    mtemp = mdata[i];
    for (j = i - 1; j >= 0 && compareFileStructs(&data[j], &temp, sort) > 0; j--) {
      data[j + 1] = data[j];
      mdata[j + 1] = mdata[j];
    }
    data[j + 1] = temp;
    mdata[j + 1] = mtemp;
  }
  // update menu text pointers (these are still pointing to the old text locations):
  for(i = 0; i < size; i++) mdata[i].text = data[i].visname;
}

bool end_with(const char * buffer,const char * end){
  int i=strlen(buffer);
  //return i<=10;
  int j=strlen(end);
  if (i<5 || j<4) return false;
  for (int k=-4;k<0;k++){
    if (buffer[i+k]!=end[j+k]) return false;
  }
  return true;
}

int GetFiles(File* files, MenuItem* menuitems, char* basepath, int* count, char* filter) {
  // searches storage memory for folders and files, puts their count in int* count
  // if File* files is NULL, function will only count files. If it is not null, MenuItem* menuitems will also be updated
  // this function always returns status codes defined on fileProvider.hpp
  // basepath should start with \\fls0\ and should always have a slash (\) at the end
  // filter is the filter for the files to list
  unsigned short path[MAX_FILENAME_SIZE+1], found[MAX_FILENAME_SIZE+1];
  char buffer[MAX_FILENAME_SIZE+1];

  // make the buffer
  strcpy(buffer, basepath);
  //strcat(buffer, "*");
  
  *count = 0;
  file_type_t fileinfo;
  int findhandle;
  Bfile_StrToName_ncpy(path, (const unsigned char *) buffer, MAX_FILENAME_SIZE+1);
  int ret = Bfile_FindFirst(path, &findhandle, found, &fileinfo);
  //Bfile_StrToName_ncpy(path, (const unsigned char *)filter, MAX_FILENAME_SIZE+1);
  while(!ret) {
    Bfile_NameToStr_ncpy((unsigned char *)buffer, found, MAX_FILENAME_SIZE+1);
    if(!(strcmp((char*)buffer, "..") == 0 || strcmp((char*)buffer, ".") == 0 || strcmp((char*)buffer, "@MainMem") == 0)
      && (fileinfo.fsize == 0 ||
	  //end_with(buffer,filter)
	  //Bfile_Name_MatchMask((const short int*)found, (const short int*)path)
	  matchmask(filter, buffer)
	  ))
    {
      if(files != NULL) {
        strncpy(files[*count].visname, (char*)buffer, 40);
        strcpy(files[*count].filename, basepath); 
        strcat(files[*count].filename, (char*)buffer);
        files[*count].size = fileinfo.fsize;
        files[*count].isfolder = menuitems[*count].isfolder = !fileinfo.fsize;
#if FILEICON
        if(fileinfo.fsize == 0) menuitems[*count].icon = FILE_ICON_FOLDER; // it would be a folder icon anyway, because isfolder is true
        else menuitems[*count].icon = fileIconFromName((char*)buffer);
#endif
        menuitems[*count].isselected = 0; //clear selection. this means selection is cleared when changing directory (doesn't happen with native file manager)
        // because usually alloca is used to declare space for MenuItem*, the space is not cleared. which means we need to explicitly set each field:
        menuitems[*count].text = files[*count].visname;
        printf("menu %i %s\n",*count,files[*count].visname);
        menuitems[*count].color=TEXT_COLOR_BLACK;
        menuitems[*count].type=MENUITEM_NORMAL;
        menuitems[*count].value=MENUITEM_VALUE_NONE;
      }
      *count=*count+1;
    }
    if (*count==MAX_ITEMS_IN_DIR) {
      Bfile_FindClose(findhandle);
      if(files != NULL && menuitems != NULL) insertSortFileMenuArray(files, menuitems, *count);
      return GETFILES_MAX_FILES_REACHED; // Don't find more files, the array is full. 
    } else ret = Bfile_FindNext(findhandle, found, &fileinfo);
  }
  Bfile_FindClose(findhandle);
  if(*count > 1 && files != NULL && menuitems != NULL) insertSortFileMenuArray(files, menuitems, *count);
  return GETFILES_SUCCESS;
}

void nameFromFilename(char* filename, char* name) {
  //this function takes a full filename like \\fls0\Folder\file.123
  //and puts file.123 in name.
  strcpy(name, (char*)"");
  int i=strlen(filename)-1;
  while (i>=0 && filename[i] != '\\')
          i--;
  if (filename[i] == '\\') {
    strcpy(name, filename+i+1);
  }
}

int fileBrowser(char* filename, char* filter, char* title) {
  // returns 1 when user selects a file, 0 when aborts (exit)
  printf("filebrowser %s\n",filter);
  int res = 1;
  char browserbasepath[MAX_FILENAME_SIZE+1] = "\\\\fls0\\";  
  while(res) {
    strcpy(filename, (char*)"");
    res = fileBrowserSub(browserbasepath, filename, filter, title);
    if(res==2) return 1; // user selected a file
  }
  return 0;
}

int fileBrowserSub(char* browserbasepath, char* filename, char* filter, char* title) {
  Menu menu;
#if FILEICON
  MenuItemIcon icontable[12];
  buildIconTable(icontable);
#endif
  
  // first get file count so we know how much to alloc
  GetFiles(NULL, NULL, browserbasepath, &menu.numitems, filter);
  MenuItem* menuitems = NULL;
  File* files = NULL;
  if(menu.numitems > 0) {
    menuitems = (MenuItem*)alloca(menu.numitems*sizeof(MenuItem));
    files = (File*)alloca(menu.numitems*sizeof(File));
    // populate arrays
    GetFiles(files, menuitems, browserbasepath, &menu.numitems, filter);
    menu.items = menuitems;
  }
  
  char titleBuffer[120];
  char titleBufferBuf[120];
  int smemfree=16384;
  unsigned short smemMedia[10]={'\\','\\','f','l','s','0',0};
  // Bfile_GetMediaFree_OS( smemMedia, &smemfree ); // inexistant syscall
  
  char friendlypath[MAX_FILENAME_SIZE];
  strcpy(friendlypath, browserbasepath+6);
  friendlypath[strlen(friendlypath)-1] = '\0'; //remove ending slash like OS does
  // test to see if friendlypath is too big
  int jump4=0;
  while(1) {
    int temptextX=7*4+10; // px length of menu title + 10, like menuGUI goes.
    int temptextY=0;
    temptextX += strlen(friendlypath)*4; // PrintMini(temptextX, temptextY, friendlypath, 0, 0xFFFFFFFF, 0, 0, COLOR_BLACK, COLOR_WHITE, 0, 0); // fake draw
    if(temptextX>LCD_WIDTH_PX-6) {
      char newfriendlypath[MAX_FILENAME_SIZE];
      shortenDisplayPath(friendlypath, newfriendlypath, (jump4 ? 4 : 1));
      if(strlen(friendlypath) > strlen(newfriendlypath) && strlen(newfriendlypath) > 3) { // check if len > 3 because shortenDisplayPath may return just "..." when the folder name is too big
        // shortenDisplayPath still managed to shorten, copy and continue
        jump4 = 1; //it has been shortened already, so next time jump the first four characters
        strcpy(friendlypath, newfriendlypath);
      } else {
        // shortenDisplayPath can't shorten any more even if it still
        // doesn't fit in the screen, so give up.
        break;
      }
    } else break;
  }
  menu.subtitle = friendlypath;
  //menu.type = MENUTYPE_MULTISELECT;
  menu.scrollout=1;
  menu.nodatamsg = (char*)"No Data";
  menu.title = title;
  while(1) {
    Bdisp_AllClr_VRAM();
    //itoa(smemfree, (char*)titleBuffer, 10);
    sprintf(titleBuffer, "%d", smemfree);
    //LocalizeMessage1( 340, titleBufferBuf ); //"bytes free"
    strncat((char*)titleBuffer, (char*)titleBufferBuf, 65);
    menu.statusText = (char*)titleBuffer;
#if FILEICON
    int res = doMenu(&menu, icontable);
#else
    int res = doMenu(&menu, 0);
#endif
    // printf("menu res %i\n",res);
    switch(res) {
      case MENU_RETURN_EXIT:
        if(!strcmp(browserbasepath,"\\\\fls0\\")) { //check that we aren't already in the root folder
          //we are, return 0 so we exit
          return 0;
        } else {
          int i=strlen(browserbasepath)-2;
          while (i>=0 && browserbasepath[i] != '\\')
                  i--;
          if (browserbasepath[i] == '\\') {
            char tmp[MAX_FILENAME_SIZE] = "";
            memcpy(tmp,browserbasepath,i+1);
            tmp[i+1] = '\0';
            strcpy(browserbasepath, tmp);
          }
          return 1; //reload at new folder
        }
        break;
    case MENU_RETURN_SELECTION: case KEY_CTRL_EXE:
        if(menuitems[menu.selection-1].isfolder) {
          strcpy(browserbasepath, files[menu.selection-1].filename); //switch to selected folder
          strcat(browserbasepath, "\\");
          return 1; //reload at new folder
        } else {
          strcpy(filename,files[menu.selection-1].filename);
          return 2;
        }
        break;

    }
  }
  return 1;
}

void shortenDisplayPath(char* longpath, char* shortpath, int jump) {
  //this function takes a long path for display, like \myfolder\long\display\path
  //and shortens it one level, like this: ...\long\display\path
  //putting the result in shortpath
  strcpy(shortpath, (char*)"...");
  int i = jump; // jump the specified amount of characters... by default it jumps the first /
  // but it can also be made to jump e.g. 4 characters, which would jump ".../" (useful for when the text has been through this function already)
  int max = strlen(longpath);
  while (i<max && longpath[i] != '\\')
          i++;
  if (longpath[i] == '\\') {
    strcat(shortpath, longpath+i);
  }
}

#if FILEICON
int fileIconFromName(char* name) {
  if(EndsIWith(name, (char*)".g1m") || EndsIWith(name, (char*)".g2m") || EndsIWith(name, (char*)".g3m"))
    return FILE_ICON_G3M;
  else if (EndsIWith(name, (char*)".g1e") || EndsIWith(name, (char*)".g2e") || EndsIWith(name, (char*)".g3e"))
    return FILE_ICON_G3E;
  else if (EndsIWith(name, (char*)".g3a") || EndsIWith(name, (char*)".g3l"))
    return FILE_ICON_G3A;
  else if (EndsIWith(name, (char*)".g3p"))
    return FILE_ICON_G3P;
  else if (EndsIWith(name, (char*)".g3b"))
    return FILE_ICON_G3B;
  else if (EndsIWith(name, (char*)".bmp"))
    return FILE_ICON_BMP;
  else if (EndsIWith(name, (char*)".txt"))
    return FILE_ICON_TXT;
  else if (EndsIWith(name, (char*)".csv"))
    return FILE_ICON_CSV;
  else return FILE_ICON_OTHER;
}

void buildIconTable(MenuItemIcon* icontable) {
  unsigned int msgno;
  unsigned short folder[7]={'\\','\\','f','l','s','0',0};

  const char *bogusFiles[] = {"t", // for folder
                              "t.g3m",
                              "t.g3e",
                              "t.g3a",
                              "t.g3p",
                              "t.g3b",
                              "t.bmp",
                              "t.txt",
                              "t.csv",
                              "t.abc" //for "unsupported" files
                             };

  for(int i = 0; i < 10; i++)
    SMEM_MapIconToExt( (unsigned char*)bogusFiles[i], (i==0 ? folder : (unsigned short*)"\x000\x000"), &msgno, icontable[i].data );
}
#endif
