#include "debug.h"

#ifdef DEBUG

int debug_handle;

#ifndef DEBUG_PATH
  #define DEBUG_PATH "\\\\crd0\\TeX.log"
#endif

#ifndef DEBUG_FILE
  #define DEBUG_FILE 10000
#endif

#ifndef DEBUG_SIZE
  #define DEBUG_SIZE 100
#endif

void debug_init(void)
{
  char debug_path[] = DEBUG_PATH;
  unsigned short *bfile_path;
  int i=0;

  while(debug_path[i]) i++;
  bfile_path = calloc(i+1,2);
  for(i=0;debug_path[i];i++) bfile_path[i] = debug_path[i];
  bfile_path[i] = 0x0;

  Bfile_DeleteFile(bfile_path);
  Bfile_CreateFile(bfile_path,DEBUG_FILE);
  debug_handle = Bfile_OpenFile(bfile_path,_OPENMODE_WRITE);

  free(bfile_path);
}

void debug(const char *format, ...)
{
  char ch[DEBUG_SIZE+1];
  va_list args;

  va_start(args,format);
  vsprintf(ch,format,args);
  va_end(args);
  ch[DEBUG_SIZE] = 0;

  Bfile_WriteFile(debug_handle,ch,strlen(ch));
}

void debug_quit(void)
{
  Bfile_CloseFile(debug_handle);
}

#endif // DEBUG
