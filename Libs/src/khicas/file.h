#ifndef __FILEPROVIDER_H
#define __FILEPROVIDER_H
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "porting.h"
#include "kdisplay.h"

// #define MAX_FILENAME_SIZE 270 //full path with //fls0/, extension and everything
#define MAX_NAME_SIZE 128 //friendly name (in "//fls0/folder/file.txt", this would be "file.txt")
#define MAX_ITEMS_IN_DIR 200
#define MAX_ITEMS_IN_CLIPBOARD 51
#define MAX_TEXTVIEWER_FILESIZE 64*1024
typedef struct
{
  char filename[MAX_FILENAME_SIZE]; //filename, not proper for use with Bfile.
  char visname[42]; //visible name, only for menus. use nameFromFilename to get the proper name.
  short action; // mostly for clipboard, can be used to tag something to do with the file
  short isfolder; // because menuitem shouldn't be the only struct holding this info
  int size; // file size
} File; // right now File only holds the filename as other fields are now set directly on a MenuItem array

typedef FILE_INFO file_type_t;
#if 0 // definition of FILE_INFO
typedef struct
{
  unsigned short id, type;
  unsigned long fsize, dsize;
  unsigned int property;
  unsigned long address;
} file_type_t;
#endif

#define GETFILES_SUCCESS 0
#define GETFILES_MAX_FILES_REACHED 1

int GetFiles(File* files, giac::MenuItem* menuitems, char* basepath, int* count, char* filter);
void nameFromFilename(char* filename, char* name);

#define FILE_ICON_FOLDER 0
#define FILE_ICON_G3M 1
#define FILE_ICON_G3E 2
#define FILE_ICON_G3A 3
#define FILE_ICON_G3P 4
#define FILE_ICON_G3B 5
#define FILE_ICON_BMP 6
#define FILE_ICON_TXT 7
#define FILE_ICON_CSV 8
#define FILE_ICON_OTHER 9
int fileIconFromName(char* name);

int fileBrowser(char* filename, char* filter, char* title);
int fileBrowserSub(char* browserbasepath, char* filename, char* filter, char* title);
void shortenDisplayPath(char* longpath, char* shortpath, int jump=1);
//void buildIconTable(MenuItemIcon* icontable);

#endif
