/****************************************************************/
/*                                                              */
/*                            Memory                            */
/*                                                              */
/*  Description: Fonctions de manipulation de la memoire        */
/*  Auteur:      LePhenixNoir                                   */
/*  Version:     3.0                                            */
/*  Date:        11.06.2014                                     */
/*  Fichier:     memory.c - Code des fonctions                  */
/*                                                              */
/****************************************************************/

#ifndef __FXLIB_H__
#include "libfx.h"
#endif

#ifndef _STDIO
#include <stdio.h>
#endif

#ifndef _STDLIB
#include <stdlib.h>
#endif

#ifndef _STRING
#include <string.h>
#endif

#include "memory.h"
#include "porting.h"

int memory_errors = 0;

void memory_seterrors(int e)
{
  memory_errors = (e != 0);
}

void memory_error(char *from, char *func, int val)
{
  unsigned int key;
  char info[20];
  if (!memory_errors)
    return;

  sprintf(info, "%d", val);
  PopUpWin(6);

  locate(4, 2);
  Print((unsigned char *)"Memory ERROR !!");
  locate(3, 4);
  Print((unsigned char *)"FROM:");
  locate(8, 4);
  Print((unsigned char *)from);
  locate(3, 5);
  Print((unsigned char *)"FUNC:");
  locate(8, 5);
  Print((unsigned char *)func);
  locate(3, 6);
  Print((unsigned char *)"INFO:");
  locate(8, 6);
  Print((unsigned char *)info);
  locate(3, 7);
  Print((unsigned char *)"META:");
  locate(8, 7);

  switch (val)
  {
  case 1:
    Print((unsigned char *)"NotEnoughRAM");
    break;
  case -1:
    Print((unsigned char *)"Nonexisting");
    break;
  case -5:
    Print((unsigned char *)"WrongDevice");
    break;
  case -8:
    Print((unsigned char *)"AccessDenied");
    break;
  case -14:
    Print((unsigned char *)"ReadOnly");
    break;
  case -31:
    Print((unsigned char *)"DeviceError");
    break;
  case -35:
    Print((unsigned char *)"NotEmpty");
    break;
  default:
    Print((unsigned char *)"Other");
    break;
  }
  GetKey(&key);
}

FONTCHARACTER *memory_char2font(char *adresse)
{
  FONTCHARACTER *adr;
  int i;

  adr = calloc((strlen(adresse) + 1), sizeof(FONTCHARACTER));
  for (i = 0; i < strlen(adresse); i++)
    *(adr + i) = *(adresse + i);
  return adr;
}

int memory_createfile(char *adresse, int size)
{
  FONTCHARACTER *adr = memory_char2font(adresse);
  int i = Bfile_CreateFile(adr, size);
  if (i < 0)
    memory_error("createfile()", "CreateFile()", i);

  free(adr);
  return i;
}

int memory_createdir(char *adresse)
{
  FONTCHARACTER *adr = memory_char2font(adresse);
  int i = Bfile_CreateDirectory(adr);
  if (i < 0)
    memory_error("createdir()", "CreateDir.()", i);

  free(adr);
  return 1;
}

int memory_openfile(char *adresse, int mode)
{
  FONTCHARACTER *adr = memory_char2font(adresse);
  int i = Bfile_OpenFile(adr, mode);
  if (i < 0)
    memory_error("openfile()", "OpenFile()", i);

  free(adr);
  return i;
}

int memory_deletefile(char *adresse)
{
  FONTCHARACTER *adr = memory_char2font(adresse);
  int i = Bfile_DeleteFile(adr);
  if (i < 0)
    memory_error("deletefil.()", "DeleteFil.()", i);

  free(adr);
  return i;
}

char **memory_alloc(int l)
{
  char **p = calloc(l, sizeof(char *));
  int i;
  for (i = 0; i < l; i++)
    *(p + i) = calloc(20, 1);
  return p;
}

void memory_free(char **p, int l)
{
  int i;
  for (i = 0; i < l; i++)
    free(*(p + i));
  free(p);
}

int memory_find(char *adresse, char **files, int max)
{
  FONTCHARACTER *adr = memory_char2font(adresse);
  FONTCHARACTER found[30];
  FILE_INFO fileInfo;
  int searchHandle, i = 1, j, x;

  if (x = Bfile_FindFirst(adr, &searchHandle, found, &fileInfo))
    return 0;
  for (j = 0; j < 14 && *(found + j); j++)
    *(*files + j) = *(found + j);

  while (Bfile_FindNext(searchHandle, found, &fileInfo) == 0 && i < max)
  {
    for (j = 0; j < 14 && *(found + j); j++)
      *(*(files + i) + j) = *(found + j);
    i++;
  }

  Bfile_FindClose(searchHandle);
  free(adr);
  return i;
}

int memory_exists(char *adresse)
{
  char *file[1];
  int x;

  *file = malloc(14);
  **file = 0;
  x = memory_find(adresse, file, 1);
  free(*file);

  return x != 0;
}

void *memory_load(char *adresse)
{
  FONTCHARACTER *adr = memory_char2font(adresse);
  int handle, x, size;
  void *p;

  if ((handle = Bfile_OpenFile(adr, _OPENMODE_READ)) < 0)
  {
    memory_error("load()", "OpenFile()", handle);
    return NULL;
  }
  size = Bfile_GetFileSize(handle) + 1;
  p = calloc(size, 1);

  if (!p)
  {
    memory_error("load()", "malloc()", 1);
    Bfile_CloseFile(handle);
    free(adr);
    return NULL;
  }
  if ((x = Bfile_ReadFile(handle, p, size, 0)) < 0)
  {
    memory_error("load()", "ReadFile()", x);
    Bfile_CloseFile(handle);
    free(adr);
    return NULL;
  }

  Bfile_CloseFile(handle);
  free(adr);
  return p;
}

int memory_save(char *adresse, void *data, int l)
{
  FONTCHARACTER *adr = memory_char2font(adresse);
  int x = 0, handle;

  if (memory_exists(adresse))
    x = Bfile_DeleteFile(adr);
  if (x < 0)
  {
    memory_error("save()", "DeleteFile()", x);
    free(adr);
    return x;
  }
  x = Bfile_CreateFile(adr, l + 1);
  if (x < 0)
  {
    memory_error("save()", "CreateFile()", x);
    free(adr);
    return x;
  }
  handle = Bfile_OpenFile(adr, 0x02);
  if (handle < 0)
  {
    memory_error("save()", "OpenFile()", handle);
    free(adr);
    return handle;
  }
  x = memory_writefile(handle, data, l);
  if (x < 0)
  {
    memory_error("save()", "WriteFile()", x);
    free(adr);
    return x;
  }
  memory_closefile(handle);

  free(adr);
  return 0;
}

int memory_user_select(char **files, int n, int extension, int exit)
{
  const unsigned char icons[7][32] = {
      {0x0, 0x3c, 0xf, 0xc4, 0xf0, 0x4, 0x80, 0x4, 0x80, 0x2, 0x80, 0x2, 0x40, 0x2, 0x40, 0x2, 0x40, 0x2, 0x40, 0x2, 0x40, 0x1, 0x40, 0x1, 0x20, 0x1, 0x20, 0xf, 0x23, 0xf0, 0x3c, 0x0},
      {0x0, 0x3c, 0xf, 0xc4, 0xf0, 0x4, 0x80, 0x74, 0x87, 0x82, 0x98, 0x2, 0x40, 0x2, 0x40, 0x3a, 0x43, 0xc2, 0x5c, 0x2, 0x40, 0x39, 0x43, 0xc1, 0x2c, 0x1, 0x20, 0xf, 0x23, 0xf0, 0x3c, 0x0},
      {0x0, 0x3c, 0xf, 0xc4, 0xf0, 0x74, 0x87, 0x94, 0xb8, 0x12, 0xa0, 0xa, 0x63, 0x8a, 0x52, 0x8a, 0x54, 0x4a, 0x54, 0x66, 0x54, 0x25, 0x48, 0x1d, 0x29, 0xe1, 0x2e, 0xf, 0x23, 0xf0, 0x3c, 0x0},
      {0x0, 0x3c, 0xf, 0xc4, 0xf0, 0x4, 0x87, 0xc4, 0x88, 0x22, 0x8c, 0x62, 0x4b, 0xa2, 0x44, 0x42, 0x42, 0x82, 0x42, 0x82, 0x42, 0x81, 0x44, 0x41, 0x2f, 0xe1, 0x20, 0xf, 0x23, 0xf0, 0x3c, 0x0},
      {0x0, 0x3c, 0xf, 0xc4, 0xf0, 0x4, 0x87, 0xe4, 0x88, 0x12, 0x88, 0x12, 0x48, 0x12, 0x47, 0xe2, 0x44, 0x22, 0x44, 0x22, 0x44, 0x21, 0x44, 0x21, 0x23, 0xc1, 0x20, 0xf, 0x23, 0xf0, 0x3c, 0x0},
      {0x0, 0x3c, 0xf, 0xc4, 0xf0, 0x4, 0x80, 0x64, 0x87, 0xb2, 0x98, 0x52, 0x51, 0xb2, 0x57, 0x52, 0x51, 0xd2, 0x4b, 0xa, 0x48, 0x19, 0x49, 0xe1, 0x2e, 0x1, 0x20, 0xf, 0x23, 0xf0, 0x3c, 0x0},
      {0x0, 0x3c, 0xf, 0xc4, 0xf0, 0x4, 0x80, 0xe4, 0x9c, 0xa2, 0x90, 0xa2, 0x58, 0xe2, 0x50, 0x2, 0x40, 0x12, 0x4a, 0x2a, 0x4a, 0x39, 0x4e, 0x29, 0x22, 0x1, 0x20, 0xf, 0x23, 0xf0, 0x3c, 0x0}};
  char *exts[19] = {".txt", ".c", ".h", ".cpp", ".hpp", ".bmp", ".jpg", ".png", ".gif", ".sav", ".g1m", ".g2m", ".g1r", ".g2r", ".g1e", ".g2e", ".g1a", ".hex", ".bin"};
  unsigned char indexs[19] = {1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 4, 4, 4, 4, 5, 5, 5, 6, 6};
  unsigned char *icoind = malloc(n);
  unsigned int key;

  int i, j, k, t;
  int p = 0, offset = 0;

  if (!icoind)
  {
    memory_error("user_sele.()", "malloc()", 1);
    return -2;
  }

  for (i = 0; i < n; i++)
  {
    for (t = -1, j = 0; *(*(files + i) + j); j++)
      if (*(*(files + i) + j) == '.')
        t = j;
    icoind[i] = (t == -1 ? 1 : 0);
    for (k = 0; k < 19; k++)
      if (!strcmp(*(files + i) + t, exts[k]))
      {
        icoind[i] = indexs[k];
        break;
      }
    if (!extension && t + 1)
      *(*(files + i) + t) = 0;
  }

  while (1)
  {
    Bdisp_AllClr_VRAM();

    for (t = 0; t < (n > 3 ? 3 : n); t++)
    {
      if (icoind[offset + i] != 255)
        for (i = 0; i < 32; i++)
        {
          k = icons[icoind[offset + t]][i];
          for (j = 0; j < 8; j++)
          {
            if (k & 1)
              Bdisp_SetPoint_VRAM(11 - j + 8 * (i & 1), 20 * t + 4 + (i >> 1), 1);
            k >>= 1;
          }
        }
      PrintXY(24, 20 * t + 9, (const unsigned char *)*(files + offset + t), 0);
    }
    Bdisp_DrawLineVRAM(2, 20 * p + 3, 2, 20 * p + 20);
    Bdisp_DrawLineVRAM(3, 20 * p + 2, 99, 20 * p + 2);
    Bdisp_DrawLineVRAM(3, 20 * p + 21, 99, 20 * p + 21);
    Bdisp_DrawLineVRAM(100, 20 * p + 3, 100, 20 * p + 20);
    if (offset > 0)
      PrintXY(114, 6, (const unsigned char *)"\346\234", 0);
    if (offset + 3 < n)
      PrintXY(114, 51, (const unsigned char *)"\346\235", 0);

    while (1)
    {
      GetKey(&key);
      if (key == 30002 && exit)
      {
        free(icoind);
        return -1;
      }
      if (key == 30004)
        break;
      if (key == 30018 && (offset || p))
      {
        if (p == 2)
          p--;
        else if (offset)
          offset--;
        else
          p--;
        break;
      }
      if (key == 30023 && (offset + p + 1 < n))
      {
        if (p == 0)
          p++;
        else if (offset + 3 < n)
          offset++;
        else
          p++;
        break;
      }
    }

    if (key == 30004)
      break;
  }

  free(icoind);
  return offset + p;
}

void *memory_user_autoload(char *prefix, char *selector, int l, int extension, int exit)
{
  char **files = memory_alloc(l);
  char *adr = malloc(strlen(prefix) + strlen(selector) + 1);
  void *data;
  int x;

  sprintf(adr, "%s%s", prefix, selector);
  x = memory_find(adr, files, l);
  free(adr);
  x = memory_user_select(files, x, extension, exit);

  adr = malloc(strlen(prefix) + strlen(files[x]) + 1);
  sprintf(adr, "%s%s", prefix, files[x]);
  data = memory_load(adr);

  free(adr);
  memory_free(files, l);
  return data;
}
