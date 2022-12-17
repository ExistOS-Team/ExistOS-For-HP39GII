#include "giacPCH.h"
#include <stdio.h>
#include "console.h"
//#include "menu_config.h"
#include "menuGUI.h"
#include "textGUI.h"
#include "file.h"
#include "syscalls.h"
extern "C"
{
#ifdef TEX
#include "TeX.h"
#endif
#include "memory.h"
}
#include "main.h"
#include "porting.h"

//#define POPUP_PRETTY 1
#define POPUP_PRETTY_STR "Pretty print"

  int shell_x=0,shell_y=0,shell_fontw=7,shell_fonth=14;
struct line *Line;
char menu_f1[8] = {0}, menu_f2[8] = {0}, menu_f3[8] = {0}, menu_f4[8] = {0}, menu_f5[8] = {0}, menu_f6[8];
char session_filename[MAX_FILENAME_SIZE + 1] = "session";
char *FMenu_entries_name[6] = {menu_f1, menu_f2, menu_f3, menu_f4, menu_f5, menu_f6};
static struct location Cursor;
static unsigned char *Edit_Line;
static int Start_Line, Last_Line, editline_cursor;
static int Case;
int console_changed = 0; // 1 if something new in history
int dconsole_mode = 1;   // 0 disables dConsole commands

#define Current_Line (Start_Line + Cursor.y)
#define Current_Col (Line[Cursor.y + Start_Line].start_col + Cursor.x)

void locate(int x, int y) {
  shell_x=x; shell_y=y;
  // vGL_ConsLocate(x, y);
}

extern "C" void vGL_putString(int x0, int y0, char *s, int fg, int bg, int fontSize) ;

void PrintRev(unsigned char *s) {
  vGL_putString(shell_x*shell_fontw, shell_y*shell_fonth, (char *)s, 255, 0, shell_fonth);
  // vGL_ConsOut((char *)s, true);
} 
void Print(unsigned char *s) {
  vGL_putString(shell_x*shell_fontw, shell_y*shell_fonth, (char *)s, 0,255, shell_fonth);
  //vGL_ConsOut((char *)s, false);
}


#if 0
void create_data_folder() {
  unsigned short pFile[MAX_FILENAME_SIZE+1];
  Bfile_StrToName_ncpy(pFile, (const unsigned char *)DATAFOLDER, strlen(DATAFOLDER)+1);
  Bfile_CreateDirectory(pFile);
}
#endif

const int max_lines_saved = 50;

void save_console_state_smem(const char *filename)
{
  console_changed = 0;
  unsigned short pFile[MAX_FILENAME_SIZE + 1];
  Bfile_StrToName_ncpy(pFile, (const unsigned char *)filename, strlen(filename) + 1);
  string state(khicas_state());
  int statesize = state.size();
  string script;
  if (edptr)
    script = merge_area(edptr->elements);
  int scriptsize = script.size();
  // save format: line_size (2), start_col(2), line_type (1), readonly (1), line
  int size = 2 * sizeof(int) + statesize + scriptsize;
  int start_row = Last_Line - max_lines_saved;
  if (start_row < 0)
    start_row = 0;
  for (int i = start_row; i <= Last_Line; ++i)
  {
    size += 2 * sizeof(short) + 2 * sizeof(char) + strlen((const char *)Line[i].str);
  }
  // there's no need to delete and create again, because there's no problem
  // if there's junk at the end of the file.
  int hFile = Bfile_OpenFile_OS(pFile, _OPENMODE_READWRITE); // Get handle
  if (hFile < 0)
  {
    // could not open file. file might not exist yet, or the data folder might not exist at all.
    // try creating both and try opening again
    // create_data_folder();
    Bfile_CreateFile(pFile, size);
    hFile = Bfile_OpenFile_OS(pFile, _OPENMODE_READWRITE); // Get handle
    if (hFile < 0)
      return; // if it still fails, there's nothing we can do
  }
  unsigned char BUF[size + 2];
  BUF[size] = 0;
  BUF[size + 1] = 0;
  unsigned char *ptr = BUF;
  // save variables and modes
  memcpy(ptr, &statesize, sizeof(statesize));
  ptr += sizeof(statesize);
  memcpy(ptr, state.c_str(), statesize);
  ptr += statesize;
  // save script
  memcpy(ptr, &scriptsize, sizeof(scriptsize));
  ptr += sizeof(scriptsize);
  if (scriptsize)
    memcpy(ptr, script.c_str(), scriptsize);
  ptr += scriptsize;
  // save console state
  for (int i = start_row; i <= Last_Line; ++i)
  {
    line &cur = Line[i];
    unsigned short l = strlen((const char *)cur.str);
    memcpy(ptr, &l, sizeof(l));
    ptr += sizeof(l); // Bfile_WriteFile_OS(hFile, &l, sizeof(l));
    unsigned short s = cur.start_col;
    memcpy(ptr, &s, sizeof(s));
    ptr += sizeof(s); // Bfile_WriteFile_OS(hFile, &s, sizeof(s));
    unsigned char c = cur.type;
    memcpy(ptr, &c, sizeof(c));
    ptr += sizeof(c); // Bfile_WriteFile_OS(hFile, &c, sizeof(c));
    c = true;         // cur.readonly;
    memcpy(ptr, &c, sizeof(c));
    ptr += sizeof(c);        // Bfile_WriteFile_OS(hFile, &c, sizeof(c));
    memcpy(ptr, cur.str, l); // ptr+=l;// Bfile_WriteFile_OS(hFile, cur.str, l);
    unsigned char *strend = ptr + l;
    for (; ptr < strend; ++ptr)
    {
      if (*ptr == 0x9c)
        *ptr = '\n';
    }
  }
  Bfile_WriteFile_OS(hFile, (const char *)BUF, size + 2);
  Bfile_CloseFile_OS(hFile);
}

#if 1

int run_session(int start = 0)
{
  std::vector<std::string> v;
  for (int i = start; i < Last_Line; ++i)
  {
    if (Line[i].type == LINE_TYPE_INPUT)
      v.push_back((const char *)Line[i].str);
    free(Line[i].str);
    Line[i].str = 0;
    Line[i].readonly = 0;
    Line[i].type = LINE_TYPE_INPUT;
    Line[i].start_col = 0;
    Line[i].disp_len = 0;
  }
  Line[Last_Line].str = 0;
  Last_Line = start;
  int savestartline = Start_Line;
  Start_Line = Last_Line > LINE_DISP_MAX ? Last_Line - LINE_DISP_MAX : 0;
  Cursor.x = 0;
  Cursor.y = start - Start_Line;
  Line[start].str = Edit_Line;
  Edit_Line[0] = 0;
  if (v.empty())
    return 0;
  // Console_Init();
  for (int i=0;i<v.size();++i){
    Console_Output((const unsigned char *)v[i].c_str());
    //int j=Last_Line;
    Console_NewLine(LINE_TYPE_INPUT, 1);
    Console_Disp();
    Bdisp_PutDisp_DD();
    // Line[j].type=LINE_TYPE_INPUT;
    run(v[i].c_str(),6); /* show logo and graph but not eqw */
    // j=Last_Line;
    Console_NewLine(LINE_TYPE_OUTPUT, 1);    
    // Line[j].type=LINE_TYPE_OUTPUT;
  }
  int cl=Current_Line;
  Cursor.y += (Start_Line-savestartline);
  if (Cursor.y<0) Cursor.y=0;
  Start_Line=savestartline;
  int l8=LINE_DISP_MAX+1;
  if (Current_Line>cl || Cursor.y>l8){
    if (cl>l8){
      Start_Line=cl-l8;
      Cursor.y=l8;
    }
    else {
      Start_Line=0;
      Cursor.y=cl;
    }
  }
  Console_Disp();
  Bdisp_PutDisp_DD();
  return 0;
}
#else
int run_session()
{
  std::vector<std::string> v;
  for (int i = 0; i < Last_Line - 1; ++i)
  {
    if (Line[i].type == LINE_TYPE_INPUT)
      v.push_back((const char *)Line[i].str);
  }
  if (v.empty())
    return 0;
  Console_Init();
  for (int i = 0; i < v.size(); ++i)
  {
    Console_Output((const unsigned char *)v[i].c_str());
    int j = Last_Line;
    Console_NewLine(LINE_TYPE_INPUT, 1);
    Line[j].type = LINE_TYPE_INPUT;
    run(v[i].c_str(), 6); /* logo and graph display */
    j = Last_Line;
    Console_NewLine(LINE_TYPE_INPUT, 1);
    Line[j].type = LINE_TYPE_OUTPUT;
    Console_Disp();
    Bdisp_PutDisp_DD();
  }
  return 0;
}
#endif

bool load_console_state_smem(const char *filename)
{
  unsigned short pFile[MAX_FILENAME_SIZE + 1];
  Bfile_StrToName_ncpy(pFile, (const unsigned char *)filename, strlen(filename) + 1);
  int hf = Bfile_OpenFile_OS(pFile, _OPENMODE_READWRITE); // Get handle
  if (hf < 0)
    return false; // nothing to load
  // int Bfile_ReadFile(int HANDLE,void *buf,int size,int readpos);
  // read variables and modes
  int L = 0;
  if (Bfile_ReadFile(hf, &L, sizeof(L), -1) != sizeof(L) || L == 0)
  {
    Bfile_CloseFile_OS(hf);
    return false;
  }
  char BUF[L + 4];
  BUF[1] = BUF[0] = '/'; // avoid trying python compat.
  BUF[2] = '\n';
  if (Bfile_ReadFile_OS(hf, BUF + 3, L, -1) != L)
  {
    Bfile_CloseFile_OS(hf);
    return false;
  }
  BUF[L + 3] = 0;
  giac::gen g, ge;
  dconsole_mode = 0;
  xcas_mode(contextptr) = python_compat(contextptr) = 0;
  do_run((char *)BUF, g, ge);
  dconsole_mode = 1;
  // read script
  if (Bfile_ReadFile(hf, &L, sizeof(L), -1) != sizeof(L))
  {
    Bfile_CloseFile_OS(hf);
    return false;
  }
  if (L > 0)
  {
    char bufscript[L + 1];
    if (Bfile_ReadFile(hf, bufscript, L, -1) != L)
    {
      Bfile_CloseFile_OS(hf);
      return false;
    }
    bufscript[L] = 0;
    if (edptr == 0)
      edptr = new textArea;
    if (edptr)
    {
      edptr->elements.clear();
      edptr->clipline = -1;
      edptr->filename = "\\\\fls0\\" + remove_path(giac::remove_extension(filename)) + ".py";
      // cout << "script " << edptr->filename << endl;
      edptr->editable = true;
      edptr->changed = false;
      edptr->python = python_compat(contextptr);
      edptr->elements.clear();
      edptr->y = 12; //!!!!!!!!! 7
      edptr->lineHeight=16;
      add(edptr, bufscript);
      edptr->line = 0;
      // edptr->line=edptr->elements.size()-1;
      edptr->pos = 0;
    }
  }
  // read console state
  // insure parse messages are cleared
  Console_Init();
  Console_Clear_EditLine();
  for (int pos = 0;; ++pos)
  {
    unsigned short int l, curs;
    unsigned char type, readonly;
    if (Bfile_ReadFile(hf, &l, sizeof(l), -1) != sizeof(l) || l == 0)
      break;
    if (Bfile_ReadFile(hf, &curs, sizeof(curs), -1) != sizeof(curs))
      break;
    if (Bfile_ReadFile(hf, &type, sizeof(type), -1) != sizeof(type))
      break;
    if (Bfile_ReadFile(hf, &readonly, sizeof(readonly), -1) != sizeof(readonly))
      break;
    char buf[l + 1];
    buf[l] = 0;
    if (Bfile_ReadFile(hf, buf, l, -1) != l)
      break;
    // ok line ready in buf
    while (Line[Current_Line].readonly)
      Console_MoveCursor(CURSOR_DOWN);
    Console_Input((const unsigned char *)buf);
    Console_NewLine(LINE_TYPE_INPUT, 1);
#if 1
    if (Current_Line > 0)
    {
      line &cur = Line[Current_Line - 1];
      cur.type = type;
      cur.readonly = readonly;
      cur.start_col += curs;
    }
#endif
  }
  Bfile_CloseFile_OS(hf);
  console_changed = 0;
  return true;
}

void menu_setup()
{
  Menu smallmenu;
  smallmenu.numitems = 7;
  MenuItem smallmenuitems[smallmenu.numitems];
  smallmenu.items = smallmenuitems;
  smallmenu.height = 8; //!!!!!
  smallmenu.scrollbar = 1;
  smallmenu.scrollout = 1;
  smallmenu.title = (char *)"Config";
  smallmenuitems[0].type = MENUITEM_CHECKBOX;
  smallmenuitems[0].text = (char *)"X,Theta,T=t";
  smallmenuitems[1].type = MENUITEM_CHECKBOX;
  smallmenuitems[1].text = (char *)"Python";
  smallmenuitems[2].type = MENUITEM_CHECKBOX;
  smallmenuitems[2].text = (char *)"Radians";
  smallmenuitems[3].type = MENUITEM_CHECKBOX;
  smallmenuitems[3].text = (char *)"Sqrt";
  smallmenuitems[4].text = (char *)(giac::lang ? "Raccourcis" : "Shortcuts");
  smallmenuitems[5].text = (char *)(giac::lang ? "A propos" : "About");
  smallmenuitems[6].text = (char *)"Quit";
  // smallmenuitems[2].text = (char*)(isRecording ? "Stop Recording" : "Record Script");
  while (1)
  {
    smallmenuitems[0].value = giac::xthetat;
    smallmenuitems[1].value = giac::python_compat(contextptr);
    smallmenuitems[2].value = giac::angle_radian(contextptr);
    smallmenuitems[3].value = giac::withsqrt(contextptr);
    int sres = doMenu(&smallmenu);
    if (sres == MENU_RETURN_EXIT)
      break;
    if (sres == MENU_RETURN_SELECTION)
    {
      if (smallmenu.selection == 7)
        break;
      if (smallmenu.selection == 1)
      {
        giac::xthetat = 1 - giac::xthetat;
        continue;
      }
      if (smallmenu.selection == 2)
      {
        bool b = !giac::python_compat(contextptr);
        giac::python_compat(b, contextptr);
        warn_python(b, false);
        if (edptr)
          edptr->python = b;
        continue;
      }
      if (smallmenu.selection == 3)
      {
        giac::angle_radian(!giac::angle_radian(contextptr), contextptr);
        continue;
      }
      if (smallmenu.selection == 4)
      {
        giac::withsqrt(!giac::withsqrt(contextptr), contextptr);
        continue;
      }
      if (smallmenu.selection >= 5)
      {
        textArea text;
        text.editable = false;
        text.clipline = -1;
        text.title = smallmenuitems[smallmenu.selection - 1].text;
        add(&text, smallmenu.selection == 5 ? shortcuts_string : apropos_string);
        doTextArea(&text);
        continue;
      }
    }
  }
}

const char *input_matrix(bool list)
{
  static ustl::string *sptr = 0;
  if (!sptr)
    sptr = new ustl::string;
  *sptr = "";
  giac::gen v(giac::_VARS(0, contextptr));
  giac::vecteur w;
  if (v.type == giac::_VECT)
  {
    for (size_t i = 0; i < v._VECTptr->size(); ++i)
    {
      giac::gen &tmp = (*v._VECTptr)[i];
      if (tmp.type == giac::_IDNT)
      {
        giac::gen tmpe(eval(tmp, 1, contextptr));
        if (list)
        {
          if (tmpe.type == giac::_VECT && !ckmatrix(tmpe))
            w.push_back(tmp);
        }
        else
        {
          if (ckmatrix(tmpe))
            w.push_back(tmp);
        }
      }
    }
  }
  ustl::string msg;
  if (w.empty())
    msg = giac::lang ? "Creer nouveau" : "Create new";
  else
    msg = ((giac::lang ? "Creer nouveau ou editer " : "Create new or edit ") + (w.size() == 1 ? w.front() : giac::gen(w, giac::_SEQ__VECT)).print(contextptr));
  giac::handle_f5();
  if (giac::inputline(msg.c_str(), (giac::lang ? "Nom de variable:" : "Variable name:"), *sptr, false) && !sptr->empty() && isalpha((*sptr)[0]))
  {
    SetSetupSetting((unsigned int)0x14, 0);
    giac::gen g(*sptr, contextptr);
    giac::gen ge(eval(g, 1, contextptr));
    if (g.type == giac::_IDNT)
    {
      if (ge.type == giac::_VECT)
      {
        ge = eqw(ge, true);
        ge = eval(ge, 1, contextptr);
        if (ge.type == giac::_VECT)
          sto(ge, g, contextptr);
        else
          std::cout << "edited " << ge << endl;
        // *sptr += ":="+ge.print(contextptr)+":;";
        // cleanup(*sptr);
        return ""; // return sptr->c_str();
      }
      if (ge == g || giac::confirm_overwrite())
      {
        *sptr = "";
        if (giac::inputline((giac::lang ? "Nombre de lignes" : "Line number"), "", *sptr, true))
        {
          int l = strtol(sptr->c_str(), 0, 10);
          if (l > 0 && l < 256)
          {
            int c;
            if (list)
              c = 0;
            else
            {
              ustl::string tmp(*sptr + (giac::lang ? " lignes." : " lines."));
              *sptr = "";
              giac::inputline(tmp.c_str(), giac::lang ? "Colonnes:" : "Columns:", *sptr, true);
              c = strtol(sptr->c_str(), 0, 10);
            }
            if (c == 0)
            {
              ge = giac::vecteur(l);
            }
            else
            {
              if (c > 0 && l * c < 256)
                ge = giac::_matrix(giac::makesequence(l, c), contextptr);
            }
            ge = eqw(ge, true);
            ge = eval(ge, 1, contextptr);
            if (ge.type == giac::_VECT)
              sto(ge, g, contextptr);
            return "";
          } // l<256
        }
      } // ge==g || overwrite confirmed
    }   // g.type==_IDNT
    else
    {
      giac::invalid_varname();
    }
  } // isalpha
  return 0;
}

/*
  ÒÔÏÂº¯Êý½«ÓÃÓÚÉ¾³ý×Ö·û´®Ö¸¶¨Î»ÖÃÖ®Ç°¹²n¸ö×Ö·û¡£ÆäÖÐ£¬1¸ö¿í×Ö·û£¨Õ¼2×Ö½Ú£©½«Ëã×÷1¸ö×Ö·û¡£

  ÀýÈç£¬ÎÒÃÇÓÐÈçÏÂ×Ö·û´®str£º

  Î»ÖÃ | 0 | 1 | 2 | 3 | 4 | 5 | 6 |
  ×Ö·û |'a'|'b'|'c'|'d'|'e'|'f'| 0 |

  Ôòµ÷ÓÃConsole_DelStr(str, 3, 2)ºó£¬Î»ÖÃ1¡¢2µÄ×Ö·û½«±»É¾³ý£¬ÆäºóµÄ×Ö·û½«±»ÌáÇ°¡£
  ½á¹ûÈçÏÂ£º

  Î»ÖÃ | 0 | 1 | 2 | 3 | 4 | 5 | 6 |
  ×Ö·û |'a'|'d'|'e'|'f'| 0 |'f'| 0 |

  £¨×¢Òâ£º¶àÓàµÄÎ»ÖÃ²¢²»»á±»Ìî³äÎª'\0'£¬µ«ÊÇÔ­×Ö·û´®Ä©Î²µÄ'\0'½«±»¿½±´¡££©

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

int Console_DelStr(unsigned char *str, int end_pos, int n)
{
  int str_len, actual_end_pos, start_pos, actual_start_pos, del_len, i;

  str_len = strlen((const char *)str);
  if ((start_pos = end_pos - n) < 0)
    return CONSOLE_ARG_ERR;

  if ((actual_end_pos = Console_GetActualPos(str, end_pos)) == CONSOLE_ARG_ERR)
    return CONSOLE_ARG_ERR;
  if ((actual_start_pos = Console_GetActualPos(str, start_pos)) == CONSOLE_ARG_ERR)
    return CONSOLE_ARG_ERR;

  del_len = actual_end_pos - actual_start_pos;

  for (i = actual_start_pos; i < str_len; i++)
  {
    str[i] = str[i + del_len];
  }

  return CONSOLE_SUCCEEDED;
}

/*
  ÒÔÏÂº¯ÊýÓÃÓÚÔÚÖ¸¶¨Î»ÖÃ²åÈëÖ¸¶¨µÄ×Ö·û´®¡£

  £¨×¢Òâ£ºÕâÀïµÄÎ»ÖÃÖ¸µÄÊÇ´òÓ¡Ê±µÄÎ»ÖÃ£¬¶ø²»ÊÇÊµ¼ÊµÄÎ»ÖÃ¡££©

  The following functions are used to specify the location of the insertion in the specified string.
  (Note: This refers to the position of the printing position when, rather than the actual position.)
*/

int Console_InsStr(unsigned char *dest, const unsigned char *src, int disp_pos)
{
  int i, ins_len, str_len, actual_pos;

  ins_len = strlen((const char *)src);
  str_len = strlen((const char *)dest);

  actual_pos = Console_GetActualPos(dest, disp_pos);

  if (ins_len + str_len >= EDIT_LINE_MAX)
    return CONSOLE_MEM_ERR;
  if (actual_pos > str_len)
    return CONSOLE_ARG_ERR;

  for (i = str_len; i >= actual_pos; i--)
  {
    dest[i + ins_len] = dest[i];
  }

  for (i = 0; i < ins_len; i++)
  {
    unsigned char c = src[i];
    if (c == '\n')
      c = 0x9c;
    dest[actual_pos + i] = (c == 0x0a ? ' ' : c);
  }

  return CONSOLE_SUCCEEDED;
}

/*
  ÒÔÏÂº¯ÊýÓÃÓÚÈ·¶¨¶ÔÓ¦ÓÚ×Ö·û´®´òÓ¡Î»ÖÃµÄÕæÊµÎ»ÖÃ¡£

  ÀýÈç£¬ÔÚÒÔÏÂÕâÒ»°üº¬¿í×Ö·ûµÄ×Ö·û´®strÖÐ£¬´òÓ¡Ê±µÄÎ»ÖÃÈçÏÂ£º

  Î»ÖÃ | 00 | 01 | 02 | 03 | 04 | 05 | 06 |
  ×Ö·û | Ò» | ¶þ | Èý | ËÄ | Îå | Áù | \0 |

  ¶øÔÚÊµ¼Ê´æ´¢Ê±µÄÎ»ÖÃÈçÏÂ£º

  Î»ÖÃ |  00  |  01  |  02  |  03  |  04  |  05  |  06  |  07  |  08  |  09  |  10  |  11  |
  Öµ   | 0xD2 | 0xBB | 0xB6 | 0xFE | 0xC8 | 0xFD | 0xCB | 0xC4 | 0xCE | 0xE5 | 0xC1 | 0xF9 |

  ¿ÉÒÔ·¢ÏÖ£¬µÚ4¸ö×Ö·û¡®Îå¡¯Êµ¼ÊÉÏ´æ´¢ÓÚµÚ8µÄÎ»ÖÃ¡£
  Òò´Ë£¬µ±µ÷ÓÃConsole_GetActualPos(str, 4)Ê±£¬½«·µ»Ø8¡£


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

int Console_GetActualPos(const unsigned char *str, int disp_pos)
{
  int actual_pos, count;

  for (actual_pos = count = 0; count < disp_pos; count++)
  {
    if (str[actual_pos] == '\0')
      return CONSOLE_ARG_ERR;

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
  ÒÔÏÂº¯ÊýÓÃÓÚ»ñÈ¡×Ö·û´®µÄ´òÓ¡³¤¶È£¬¼´£¬1¸ö¿í×Ö·û£¨Õ¼ÓÃ2×Ö½Ú£©¼Ç×÷1×Ö·û¡£
  The following functions are used to obtain a string of print length, ie, a wide character (2 bytes) recorded as a character.
*/

int Console_GetDispLen(const unsigned char *str)
{
  int i, len;

  for (i = len = 0; str[i] != '\0'; len++)
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
  ÒÔÏÂº¯ÊýÓÃÓÚÒÆ¶¯¹â±ê¡£
  The following functions are used to move the cursor.
*/

int Console_MoveCursor(int direction)
{
  switch (direction)
  {
  case CURSOR_UP:
    if (Current_Line == Last_Line)
      editline_cursor = Cursor.x;
    //Èç¹ûÐèÒª²Ù×÷¡£
    // If you need to operate.
    if ((Cursor.y > 0) || (Start_Line > 0))
    {
      //Èç¹ûµ±Ç°ÐÐ²»ÊÇÖ»¶ÁµÄ,Ôò½«Edit_Line¿½±´¸øµ±Ç°ÐÐ¡£
      // If the current line is not read-only, then Edit_Line copy to the current line.
      if (!Line[Current_Line].readonly)
      {
        if ((Line[Current_Line].str = (unsigned char *)malloc(strlen((const char *)Edit_Line) + 1)) == NULL)
          return CONSOLE_MEM_ERR;
        strcpy((char *)Line[Current_Line].str, (const char *)Edit_Line);
        Line[Current_Line].disp_len = Console_GetDispLen(Line[Current_Line].str);
        Line[Current_Line].type = LINE_TYPE_INPUT;
      }

      //Èç¹û¹â±êÎ´ÒÆµ½×îÉÏ·½,ÔòÖ±½Ó½«¹â±êÏòÉÏÒÆ¡£
      // If the cursor does not move to the top of, directly move the cursor upward.
      if (Cursor.y > 0)
      {
        Cursor.y--;
      }
      //·ñÔò£¬Èç¹ûÆÁÄ»ÉÏÊ×ÐÐ²»ÊÇµÚÒ»ÐÐ£¬Ôò½«¿ªÊ¼ÏÔÊ¾µÄÐÐÊý¼õÒ»¡£
      // Otherwise, the number of rows, if the screen's first line is not the first line, then began to show minus one.
      else if (Start_Line > 0)
      {
        Start_Line--;
      }

      //Èç¹ûÒÆ¶¯ºó¹â±êË®Æ½Î»ÖÃ³¬¹ýÐÐÄ©£¬Ôò½«¹â±êÒÆÖÁÐÐÄ©¡£
      // End if the horizontal position after moving the cursor over the line, then move the cursor to the end of the line.
      if (Cursor.x > Line[Current_Line].disp_len)
      {
        Cursor.x = Line[Current_Line].disp_len;
      }
      else if (Line[Current_Line].disp_len - Line[Current_Line].start_col > COL_DISP_MAX)
      {
        if (Cursor.x == COL_DISP_MAX)
          Cursor.x = COL_DISP_MAX - 1;
      }

      //Èç¹ûÒÆ¶¯ºó¹â±êÔÚÐÐÊ×£¬ÇÒ¸ÃÐÐÇ°ÃæÓÐ×Ö·ûÎ´ÏÔÊ¾£¬Ôò½«¹â±êÒÆÖÁÎ»ÖÃ1¡£
      // If you move the cursor to the line after the first, and the front of the line there is a character does not appear, then move the cursor to position 1.
      if (Cursor.x == 0 && Line[Current_Line].start_col > 0)
        Cursor.x = 1;

      //Èç¹ûÏÖÔÚ¹â±êËùÔÚÐÐ²»ÊÇÖ»¶ÁµÄ£¬Ôò½«Æä×Ö·û´®¿½±´¸øEdit_LineÒÔ¹©±à¼­¡£
      // If the current cursor line is not read-only, then it is a string copy to Edit_Line for editing.
      if (!Line[Current_Line].readonly && Line[Current_Line].str)
      {
        strcpy((char *)Edit_Line, (const char *)Line[Current_Line].str);
        free(Line[Current_Line].str);
        Line[Current_Line].str = Edit_Line;
      }
    }
    break;
  case CURSOR_ALPHA_UP:
  {
    int pos1 = Start_Line + Cursor.y;
    Console_MoveCursor(CURSOR_UP);
    int pos2 = Start_Line + Cursor.y;
    if (pos1 < Last_Line && pos2 < Last_Line && pos1 != pos2)
    {
      line curline = Line[pos1];
      Line[pos1] = Line[pos2];
      Line[pos2] = curline;
    }
    break;
  }
  case CURSOR_ALPHA_DOWN:
  {
    int pos1 = Start_Line + Cursor.y;
    Console_MoveCursor(CURSOR_DOWN);
    int pos2 = Start_Line + Cursor.y;
    if (pos1 < Last_Line && pos2 < Last_Line && pos1 != pos2)
    {
      line curline = Line[pos1];
      Line[pos1] = Line[pos2];
      Line[pos2] = curline;
    }
    break;
  }
  case CURSOR_DOWN:
    if (Current_Line == Last_Line)
      editline_cursor = Cursor.x;
    //Èç¹ûÐèÒª²Ù×÷¡£
    // If you need to operate.
    if ((Cursor.y < LINE_DISP_MAX - 1) && (Current_Line < Last_Line) || (Start_Line + LINE_DISP_MAX - 1 < Last_Line))
    {
      //Èç¹ûµ±Ç°ÐÐ²»ÊÇÖ»¶ÁµÄ,Ôò½«Edit_Line¿½±´¸øµ±Ç°ÐÐ¡£
      // If the current line is not read-only, then Edit_Line copy to the current line.
      if (!Line[Current_Line].readonly)
      {
        if ((Line[Current_Line].str = (unsigned char *)malloc(strlen((const char *)Edit_Line) + 1)) == NULL)
          return CONSOLE_MEM_ERR;
        strcpy((char *)Line[Current_Line].str, (const char *)Edit_Line);
        Line[Current_Line].disp_len = Console_GetDispLen(Line[Current_Line].str);
        Line[Current_Line].type = LINE_TYPE_INPUT;
      }

      //Èç¹û¹â±êÎ´ÒÆµ½×îÏÂ·½,ÔòÖ±½Ó½«¹â±êÏòÏÂÒÆ¡£
      // If the cursor does not move to the bottom, the cursor moves down directly.
      if (Cursor.y < LINE_DISP_MAX - 1 && Current_Line < Last_Line)
      {
        Cursor.y++;
      }
      //·ñÔò£¬Èç¹ûÆÁÄ»ÉÏÄ©ÐÐ²»ÊÇ×îºóÒ»ÐÐ£¬Ôò½«¿ªÊ¼ÏÔÊ¾µÄÐÐÊý¼ÓÒ»¡£
      // The number of rows Otherwise, if the last line is not the last line on the screen, it will begin to show a plus.
      else if (Start_Line + LINE_DISP_MAX - 1 < Last_Line)
      {
        Start_Line++;
      }

      //Èç¹ûÒÆ¶¯ºó¹â±êË®Æ½Î»ÖÃ³¬¹ýÐÐÄ©£¬Ôò½«¹â±êÒÆÖÁÐÐÄ©¡£
      // If you move the cursor after the end of the horizontal position over the line, then move the cursor to the end of the line.
      if (Cursor.x > Line[Current_Line].disp_len)
      {
        Cursor.x = Line[Current_Line].disp_len;
      }
      else if (Line[Current_Line].disp_len - Line[Current_Line].start_col >= COL_DISP_MAX)
      {
        if (Cursor.x == COL_DISP_MAX)
          Cursor.x = COL_DISP_MAX - 1;
      }

      //Èç¹ûÒÆ¶¯ºó¹â±êÔÚÐÐÊ×£¬ÇÒ¸ÃÐÐÇ°ÃæÓÐ×Ö·ûÎ´ÏÔÊ¾£¬Ôò½«¹â±êÒÆÖÁÎ»ÖÃ1¡£
      // If you move the cursor to the line after the first, and the front of the line there is a character does not appear, then move the cursor to position 1.
      if (Cursor.x == 0 && Line[Current_Line].start_col > 0)
        Cursor.x = 1;

      //Èç¹ûÏÖÔÚ¹â±êËùÔÚÐÐ²»ÊÇÖ»¶ÁµÄ£¬Ôò½«Æä×Ö·û´®¿½±´¸øEdit_LineÒÔ¹©±à¼­¡£
      // If the current cursor line is not read-only, then it is a string copy to Edit_Line for editing.
      if (!Line[Current_Line].readonly && Line[Current_Line].str)
      {
        strcpy((char *)Edit_Line, (const char *)Line[Current_Line].str);
        free(Line[Current_Line].str);
        Line[Current_Line].str = Edit_Line;
      }
    }
    break;
  case CURSOR_LEFT:
    if (Line[Current_Line].readonly)
    {
      if (Line[Current_Line].start_col > 0)
      {
        Line[Current_Line].start_col--;
      }
      break;
    }
    else
    {
      if (Line[Current_Line].start_col > 0)
      {
        if (Cursor.x > 1)
          Cursor.x--;
        else
          Line[Current_Line].start_col--;
        break;
      }
      if (Cursor.x > 0)
      {
        Cursor.x--;
        break;
      }
    }
  case CURSOR_SHIFT_RIGHT:
    if (!Line[Current_Line].readonly)
      Cursor.x = min(Line[Current_Line].disp_len, (int)COL_DISP_MAX);
    if (Line[Current_Line].disp_len > COL_DISP_MAX)
      Line[Current_Line].start_col = Line[Current_Line].disp_len - COL_DISP_MAX;
    break;
  case CURSOR_RIGHT:
    if (Line[Current_Line].readonly)
    {
      if (Line[Current_Line].disp_len - Line[Current_Line].start_col > COL_DISP_MAX)
      {
        Line[Current_Line].start_col++;
      }
      break;
    }
    else
    {
      if (Line[Current_Line].disp_len - Line[Current_Line].start_col > COL_DISP_MAX)
      {
        if (Cursor.x < COL_DISP_MAX - 1)
          Cursor.x++;
        else
          Line[Current_Line].start_col++;
        break;
      }
      if (Cursor.x < Line[Current_Line].disp_len - Line[Current_Line].start_col)
      {
        Cursor.x++;
        break;
      }
    }
  case CURSOR_SHIFT_LEFT:
    if (!Line[Current_Line].readonly)
      Cursor.x = 0;
    Line[Current_Line].start_col = 0;
    break;
  default:
    return CONSOLE_ARG_ERR;
    break;
  }
  return CONSOLE_SUCCEEDED;
}

/*
  ÒÔÏÂº¯ÊýÓÃÓÚÊäÈë¡£
  ×Ö·û´®½«ÊäÈëµ½¹â±ê´¦£¬¹â±ê½«×Ô¶¯ÒÆ¶¯¡£

  The following function is used for input.
  String input to the cursor, the cursor will automatically move.
*/

int Console_Input(const unsigned char *str)
{
  console_changed = 1;
  int old_len, i, return_val;

  if (!Line[Current_Line].readonly)
  {
    old_len = Line[Current_Line].disp_len;
    return_val = Console_InsStr(Edit_Line, str, Current_Col);
    if (return_val != CONSOLE_SUCCEEDED)
      return return_val;
    if ((Line[Current_Line].disp_len = Console_GetDispLen(Edit_Line)) == CONSOLE_ARG_ERR)
      return CONSOLE_ARG_ERR;
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
  ÒÔÏÂº¯ÊýÓÃÓÚÊä³ö×Ö·û´®µ½µ±Ç°ÐÐ¡£
  The following functions are used to output the string to the current line.
*/

int Console_Output(const unsigned char *str)
{
  console_changed = 1;
  int return_val, old_len, i;

  if (!Line[Current_Line].readonly)
  {
    old_len = Line[Current_Line].disp_len;

    return_val = Console_InsStr(Edit_Line, str, Current_Col);
    if (return_val != CONSOLE_SUCCEEDED)
      return return_val;
    if ((Line[Current_Line].disp_len = Console_GetDispLen(Edit_Line)) == CONSOLE_ARG_ERR)
      return CONSOLE_ARG_ERR;
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

void dConsolePut(const char *S)
{
  if (!dconsole_mode)
    return;
  int l = strlen(S);
  char s[l + 1];
  strcpy(s, S);
  for (int i = 0; i < l - 1; ++i)
  {
    if (s[i] == '\n' ||
        s[i] == 10)
      s[i] = ' ';
  }
  Console_Output((const unsigned char *)s);
  if (l && S[l - 1] == '\n')
    Console_NewLine(LINE_TYPE_OUTPUT, 1);
}

#define PUTCHAR_LEN 35
static char putchar_buf[PUTCHAR_LEN + 2];
static int putchar_pos = 0;
void dConsolePutChar(const char ch)
{
  if (!dconsole_mode)
    return;
  if (putchar_pos == PUTCHAR_LEN)
    dConsolePutChar('\n');
  if (ch == '\n')
  {
    putchar_buf[putchar_pos] = '\n';
    putchar_buf[putchar_pos + 1] = 0;
    putchar_pos = 0;
    dConsolePut(putchar_buf);
  }
  else
  {
    putchar_buf[putchar_pos] = ch;
    ++putchar_pos;
  }
}

/*
  Clear the current output line
*/

void Console_Clear_EditLine()
{
  if (!Line[Current_Line].readonly)
  {
    Edit_Line[0] = '\0';
    Line[Current_Line].start_col = 0;
    Line[Current_Line].disp_len = 0;
    Cursor.x = 0;
  }
}

/*
  ÒÔÏÂº¯ÊýÓÃÓÚ´´½¨ÐÂÐÐ¡£
  ²ÎÊýpre_line_typeÓÃÓÚÖ¸¶¨ÉÏÒ»ÐÐµÄÀàÐÍ£¬²ÎÊýpre_line_readonlyÓÃÓÚÖ¸¶¨ÉÏÒ»ÐÐÊÇ·ñÖ»¶Á¡£
  ²ÎÊýnew_line_typeÓÃÓÚÖ¸¶¨ÏÂÒ»ÐÐµÄÀàÐÍ£¬²ÎÊýnew_line_readonlyÓÃÓÚÖ¸¶¨ÏÂÒ»ÐÐÊÇ·ñÖ»¶Á¡£

  The following functions are used to create a new line.
  Pre_line_type type parameter is used to specify the line, pre_line_readonly parameter is used to specify the line is read-only.
  New_line_type parameter is used to specify the type of the next line, new_line_readonly parameter is used to specify the next line is read-only.
*/

int Console_NewLine(int pre_line_type, int pre_line_readonly)
{
  console_changed = 1;
  int i;

  if (strlen((const char *)Edit_Line) || Line[Current_Line].type == LINE_TYPE_OUTPUT)
  {
    //Èç¹ûÒÑ¾­ÊÇËùÄÜ´æ´¢µÄ×îºóÒ»ÐÐ£¬ÔòÉ¾³ýµÚÒ»ÐÐ¡£
    // If this is the last line we can store, delete the first line.
    if (Last_Line == COL_LINE_MAX - 1)
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

      if (Start_Line > 0)
        Start_Line--;
    }

    if (Line[Last_Line].type == LINE_TYPE_OUTPUT && strlen((const char *)Edit_Line) == 0)
      Console_Output((const unsigned char *)"Done");

#ifdef TEX
    if (TeX_isTeX((char *)Edit_Line))
      Line[Last_Line].tex_flag = 1;
    if (Line[Last_Line].type == LINE_TYPE_OUTPUT && Line[Last_Line].tex_flag)
      TeX_sizeComplex((char *)Edit_Line, &(Line[Last_Line].tex_width), &(Line[Last_Line].tex_height), NULL);
    else
      Line[Last_Line].tex_flag = 0;
#endif

      //½«Edit_LineµÄÄÚÈÝ¿½±´¸ø×îºóÒ»ÐÐ¡£
      // Edit_Line copy the contents to the last line.

#ifdef POPUP_PRETTY
    if (Line[Last_Line].tex_flag)
    {
      if ((Line[Last_Line].str = (unsigned char *)malloc(strlen(POPUP_PRETTY_STR) + 1)) == NULL)
        return CONSOLE_MEM_ERR;
      strcpy((char *)Line[Last_Line].str, (const char *)POPUP_PRETTY_STR);
      if ((Line[Last_Line].tex_str = (unsigned char *)malloc(strlen((const char *)Edit_Line) + 1)) == NULL)
        return CONSOLE_MEM_ERR;
      strcpy((char *)Line[Last_Line].tex_str, (const char *)Edit_Line);
    }
    else
    {
      if ((Line[Last_Line].str = (unsigned char *)malloc(strlen((const char *)Edit_Line) + 1)) == NULL)
        return CONSOLE_MEM_ERR;
      strcpy((char *)Line[Last_Line].str, (const char *)Edit_Line);
    }
#else
    if ((Line[Last_Line].str = (unsigned char *)malloc(strlen((const char *)Edit_Line) + 1)) == NULL)
      return CONSOLE_MEM_ERR;
    strcpy((char *)Line[Last_Line].str, (const char *)Edit_Line);
#endif

    if ((Line[Last_Line].disp_len = Console_GetDispLen(Line[Last_Line].str)) == CONSOLE_ARG_ERR)
      return CONSOLE_ARG_ERR;
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

void Console_Insert_Line()
{
  if (Last_Line >= COL_LINE_MAX - 1)
    return;
  for (int i = Last_Line; i >= Current_Line; --i)
  {
    Line[i + 1] = Line[i];
  }
  ++Last_Line;
  int i = Current_Line;
  line &l = Line[i];
  l.str = (unsigned char *)malloc(2);
  strcpy((char *)l.str, "0");
  l.type = Line[i + 1].type == LINE_TYPE_INPUT ? LINE_TYPE_OUTPUT : LINE_TYPE_INPUT;
  l.start_col = 0;
  l.readonly = 1;
  l.disp_len = Console_GetDispLen(l.str);
}

/*
  The following function is used to delete a character before the cursor.
*/

int Console_Backspace()
{
  console_changed = 1;
  if (Last_Line > 0 && Current_Line < Last_Line)
  {
    int i = Current_Line;
    if (Edit_Line == Line[i].str)
      Edit_Line = Line[i + 1].str;
    if (Line[i].str)
    {
      giac::copy_clipboard((const char *)Line[i].str);
      free(Line[i].str);
    }
    for (; i < Last_Line; ++i)
    {
      Line[i] = Line[i + 1];
    }
    Line[i].readonly = 0;
    Line[i].type = LINE_TYPE_INPUT;
    Line[i].start_col = 0;
    Line[i].disp_len = 0;
    Line[i].str = 0;
    --Last_Line;
    if (Start_Line > 0)
      --Start_Line;
    else
    {
      if (Cursor.y > 0)
        --Cursor.y;
    }
#if 1
    if (Last_Line == 0 && Current_Line == 0)
    { // workaround
      char buf[strlen((const char *)Edit_Line) + 1];
      strcpy(buf, (const char *)Edit_Line);
      Console_Init();
      Console_Clear_EditLine();
      if (buf[0])
        Console_Input((const unsigned char *)buf);
      // ustl::string status(giac::print_INT_(Last_Line)+" "+(giac::print_INT_(Current_Line)+" ")+giac::print_INT_(Line[Current_Line].str)+" "+(const char*)Line[Current_Line].str);
      // DefineStatusMessage(status.c_str(),1,0,0);
      // DisplayStatusArea();
    }
#endif
    Console_Disp();
    return CONSOLE_SUCCEEDED;
  }
  int return_val;
  return_val = Console_DelStr(Edit_Line, Current_Col, 1);
  if (return_val != CONSOLE_SUCCEEDED)
    return return_val;
  Line[Current_Line].disp_len = Console_GetDispLen(Edit_Line);
  return Console_MoveCursor(CURSOR_LEFT);
}

/*
  ÒÔÏÂº¯ÊýÓÃÓÚ´¦Àí°´¼ü¡£
  The following functions are used to deal with the key.
*/

void translate_fkey(unsigned int &input_key)
{
  if (input_key == KEY_CTRL_MIXEDFRAC)
    input_key = KEY_CTRL_F10;
  if (input_key == KEY_CTRL_FRACCNVRT)
    input_key = KEY_CTRL_F7;
  if (input_key == KEY_CHAR_LIST)
    input_key = KEY_CTRL_F9;
  if (input_key == KEY_CHAR_MAT)
    input_key = KEY_CTRL_F8;
  if (input_key == KEY_CTRL_PRGM)
    input_key = KEY_CTRL_F12;
  if (input_key == KEY_CTRL_FD)
    input_key = KEY_CTRL_F11;
  if (input_key == KEY_CHAR_ANGLE)
    input_key = KEY_CTRL_F13;
  if (input_key == KEY_CHAR_FRAC)
    input_key = KEY_CTRL_F14;
}

void chk_clearscreen()
{
  giac::drawRectangle(0, 24, LCD_WIDTH_PX, LCD_HEIGHT_PX - 24, COLOR_WHITE); //!!!!
  if (giac::confirm(giac::lang ? "Effacer l'historique?" : "Clear history?", giac::lang ? "F1: annuler,   F6: effacer" : "F1: cancel,   F6: erase", true) == KEY_CTRL_F6)
  {
    Console_Init();
    Console_Clear_EditLine();
  }
  Console_Disp();
}

void reload_edptr(const char *filename, textArea *edptr)
{
  if (edptr)
  {
    ustl::string s(merge_area(edptr->elements));
    giac::copy_clipboard(s);
    s = "\n";
    edptr->elements.clear();
    edptr->clipline = -1;
    edptr->filename = "\\\\fls0\\" + remove_path(giac::remove_extension(filename)) + ".py";
    load_script(edptr->filename.c_str(), s);
    if (s.empty())
      s = "\n";
    // cout << "script " << edptr->filename << endl;
    edptr->editable = true;
    edptr->changed = false;
    edptr->python = python_compat(contextptr);
    edptr->elements.clear();
    edptr->y = 7; //!!!!!!!!! 7
    add(edptr, s);
    edptr->line = 0;
    edptr->pos = 0;
  }
}

int Console_Eval(const char *buf)
{
  int start = Current_Line;
  free(Line[start].str);
  Line[start].str = (unsigned char *)malloc(strlen(buf) + 1);
  strcpy((char *)Line[start].str, buf);
  run_session(start);
  int move_line = Last_Line - start;
  for (int i = 0; i < move_line; i++)
    Console_MoveCursor(CURSOR_UP);
  return CONSOLE_SUCCEEDED;
}

bool inputdouble(const char *msg1, double &d)
{
  ustl::string s1;
  giac::inputline(msg1, giac::lang ? "Nouvelle valeur?" : "New value?", s1, false);
  return stringtodouble(s1, d);
}

// back is the number of char that should be deleted before inserting
string help_insert(const char *cmdline, int &back, bool warn)
{
  back = 0;
  int l = strlen(cmdline);
  char buf[l + 1024];
  strcpy(buf, cmdline);
  bool openpar = l && buf[l - 1] == '(';
  if (openpar)
  {
    buf[l - 1] = 0;
    --l;
    ++back;
  }
  for (; l > 0; --l)
  {
    if (!is_alphanum(buf[l - 1]) && buf[l - 1] != '_')
      break;
  }
  // cmdname in buf+l
  const char *cmdname = buf + l, *cmdnameorig = cmdname;
  l = strlen(cmdname);
  int res = doCatalogMenu(buf, (char *)"Index", 0, cmdname);
  if (!res)
    return "";
  return cmdname + l;
}

bool console_help_insert(bool warn = true)
{
  if (!Edit_Line)
    return false;
  char buf[strlen((char *)Edit_Line) + 1];
  strcpy(buf, (char *)Edit_Line);
  buf[Line[Current_Line].start_col + Cursor.x] = 0;
  int back;
  string s = help_insert(buf, back, warn);
  if (s.empty())
    return false;
  for (int i = 0; i < back; ++i)
    Console_Backspace();
  Console_Input((const unsigned char *)s.c_str());
  Console_Disp();
  return true;
}

int Console_GetKey()
{
  unsigned int key, i, move_line, move_col;
  unsigned char tmp_str[2];
  unsigned char *tmp;
  for (;;)
  {
    int keyflag = (unsigned char)Setup_GetEntry(0x14);
    bool alph = keyflag == 4 || keyflag == 0x84 || keyflag == 8 || keyflag == 0x88;
    ck_getkey((int *)&key);
    translate_fkey(key);
    if (key >= '0' && key <= '9')
    {
      tmp_str[0] = key;
      tmp_str[1] = '\0';
      return Console_Input(tmp_str);
    }
    if (key == KEY_CTRL_F5)
    {
      giac::handle_f5();
      Console_Disp();
      continue;
    }
    if (key == KEY_CTRL_PASTE)
      return Console_Input((const unsigned char *)giac::paste_clipboard());
    if (key == KEY_CTRL_VARS)
    {
      const char *ptr = giac::keytostring(key, 0, 0, contextptr);
      if (ptr)
        return Console_Input((const unsigned char *)ptr);
    }
    if (key == KEY_CTRL_OPTN)
    {
      char buf[256];
      if (doCatalogMenu(buf, "OPTIONS", 14)) // should be doCatalogMenu(buf,"OPTION",1)
        return Console_Input((const unsigned char *)buf);
      Console_Disp();
      continue;
    }
#if 0 // CATALOG is not returned by GetKey, we will use OPTN instead
    if (key==KEY_CTRL_CATALOG){
      char buf[256];
      if (doCatalogMenu(buf,"CATALOG",0))
	return Console_Input((const unsigned char *)buf);
    }
#endif
    if ((key == KEY_CHAR_PLUS || key == KEY_CHAR_MINUS || key == KEY_CHAR_MULT || key == KEY_CHAR_DIV) && Current_Line < Last_Line - 1)
    {
      line *nxt = &Line[Current_Line];
      if (strncmp((const char *)nxt->str, "parameter([", 11) == 0)
        Console_MoveCursor(CURSOR_UP);
      nxt = &Line[Current_Line + 1];
      if (strncmp((const char *)nxt->str, "parameter([", 11) == 0)
      {
        giac::gen g((const char *)nxt->str, contextptr);
        if (g.is_symb_of_sommet(giac::at_parameter))
        {
          g = g._SYMBptr->feuille;
          if (g.type == giac::_VECT && g._VECTptr->size() >= 5)
          {
            giac::vecteur &v = *g._VECTptr;
            if (v[0].type == giac::_IDNT && v[1].type == giac::_DOUBLE_ && v[2].type == giac::_DOUBLE_ && v[3].type == giac::_DOUBLE_ && v[4].type == giac::_DOUBLE_)
            {
              ustl::string s("assume(");
              s += v[0]._IDNTptr->id_name;
              s += "=[";
              int val = 1;
              if (key == KEY_CHAR_MINUS)
                val = -1;
              if (key == KEY_CHAR_MULT)
                val = 5;
              if (key == KEY_CHAR_DIV)
                val = -5;
              s += giac::print_DOUBLE_(v[3]._DOUBLE_val + val * v[4]._DOUBLE_val, 3);
              s += ',';
              s += giac::print_DOUBLE_(v[1]._DOUBLE_val, 3);
              s += ',';
              s += giac::print_DOUBLE_(v[2]._DOUBLE_val, 3);
              s += ',';
              s += giac::print_DOUBLE_(v[4]._DOUBLE_val, 3);
              s += "])";
              return Console_Eval(s.c_str());
            }
          }
        }
      }
    }
    if (key == KEY_CTRL_F3 || ((key == KEY_CTRL_RIGHT || key == KEY_CTRL_LEFT) && Current_Line < Last_Line))
    {
      int l = Current_Line;
      bool graph = strcmp((const char *)Line[l].str, "Graphic object") == 0;
      if (graph && l > 0)
        --l;
      char buf[max(512, (int)strlen((const char *)Line[l].str + 1))];
      strcpy(buf, (const char *)Line[l].str);
      if ((alph || key == KEY_CTRL_RIGHT) ? textedit(buf) : eqws(buf, graph))
      {
        if (Current_Line == Last_Line)
        {
          Console_Clear_EditLine();
          return Console_Input((const unsigned char *)buf);
        }
        else
        {
#if 1
          if (Line[l].type == LINE_TYPE_INPUT && l < Last_Line - 1 && Line[l + 1].type == LINE_TYPE_OUTPUT)
            return Console_Eval(buf);
          else
          {
            free(Line[l].str);
            Line[l].str = (unsigned char *)malloc(strlen(buf) + 1);
            Line[l].disp_len = Console_GetDispLen(Line[l].str);
            strcpy((char *)Line[l].str, buf);
          }
#else
          int x = editline_cursor;
          move_line = Last_Line - Current_Line;
          for (i = 0; i <= move_line; i++)
            Console_MoveCursor(CURSOR_DOWN);
          Cursor.x = x;
          return Console_Input((const unsigned char *)buf);
#endif
        }
      }
      Console_Disp();
      continue;
    }
    if (key == KEY_CTRL_F4)
    {
      char buf[512];
      if (!showCatalog(buf, 0, 0))
        buf[0] = 0;
      return Console_Input((const unsigned char *)buf);
    }
    if (key == KEY_CTRL_F6)
    {
#if 1
      Menu smallmenu;
      smallmenu.numitems = 15;
      MenuItem smallmenuitems[smallmenu.numitems];

      smallmenu.items = smallmenuitems;
      smallmenu.height =  8; //!!!!!
      smallmenu.scrollbar = 1;
      smallmenu.scrollout = 1;
      // smallmenu.title = "KhiCAS";
      smallmenuitems[0].text = (char *)(giac::lang ? "Enregistrer session" : "Save session ");
      smallmenuitems[1].text = (char *)(giac::lang ? "Enregistrer sous" : "Save session as");
      smallmenuitems[2].text = (char *)(giac::lang ? "Charger session" : "Load session");
      smallmenuitems[3].text = (char *)(giac::lang ? "Nouvelle session" : "New session");
      smallmenuitems[4].text = (char *)(giac::lang ? "Executer session" : "Run session");
      smallmenuitems[5].text = (char *)(giac::lang ? "Editeur script" : "Script editor");
      smallmenuitems[6].text = (char *)(giac::lang ? "Ouvrir script" : "Open script");
      smallmenuitems[7].text = (char *)(giac::lang ? "Executer script" : "Run script");
      smallmenuitems[8].text = (char *)(giac::lang ? "Effacer historique" : "Clear history");
      smallmenuitems[9].text = (char *)(giac::lang ? "Effacer script" : "Clear script");
      smallmenuitems[10].text = (char *)(giac::lang ? "Editer matrice" : "Matrix editor");
      smallmenuitems[11].text = (char *)"Parameter";
      smallmenuitems[12].text = (char *)"Config shift-SETUP";
      smallmenuitems[13].text = (char *)(giac::lang ? "Raccourcis" : "Shortcuts");
      smallmenuitems[14].text = (char *)(giac::lang ? "A propos" : "About");
      // smallmenuitems[2].text = (char*)(isRecording ? "Stop Recording" : "Record Script");
      while (1)
      {
        int sres = doMenu(&smallmenu);
        if (sres == MENU_RETURN_SELECTION)
        {
          const char *ptr = 0;
          if (smallmenu.selection == 1)
          {
            if (strcmp(session_filename, "session") == 0)
              smallmenu.selection = 2;
            else
            {
              save(session_filename);
              break;
            }
          }
          if (smallmenu.selection == 2)
          {
            char buf[270];
            if (get_filename(buf, ".xw"))
            {
              save(buf);
              string fname(remove_path(giac::remove_extension(buf)));
              strcpy(session_filename, fname.c_str());
              if (edptr)
                edptr->filename = "\\\\fls0\\" + fname + ".py";
            }
            break;
          }
          if (smallmenu.selection == 3)
          {
            char filename[MAX_FILENAME_SIZE + 1];
            if (fileBrowser(filename, (char *)"*.xw", (char *)"Sessions"))
            {
              if (console_changed == 0 || strcmp(session_filename, "session") == 0 || giac::confirm(giac::lang ? "Session courante perdue?" : "Current session will be lost", giac::lang ? "F1: annul, F6: ok" : "F1: cancel, F6: ok") == KEY_CTRL_F6)
              {
                giac::clip_pasted = true;
                giac::_restart(giac::gen(giac::vecteur(0), giac::_SEQ__VECT), contextptr);
                restore_session(filename);
                strcpy(session_filename, remove_path(giac::remove_extension(filename)).c_str());
                // reload_edptr(session_filename,edptr);
              }
            }
            break;
          }
          if (0 && smallmenu.selection == 3)
          {
            // FIXME: make a menu catalog?
            char buf[512];
            if (doCatalogMenu(buf, "CATALOG", 0))
              return Console_Input((const unsigned char *)buf);
            break;
          }
          if (smallmenu.selection == 4)
          {
            char filename[MAX_FILENAME_SIZE + 1];
            giac::drawRectangle(0, 0, LCD_WIDTH_PX, LCD_HEIGHT_PX - 8, COLOR_WHITE);
            if (get_filename(filename, ".xw"))
            {
              if (console_changed == 0 || strcmp(session_filename, "session") == 0 || giac::confirm(giac::lang ? "Session courante perdue?" : "Current session will be lost", giac::lang ? "F1: annul, F6: ok" : "F1: cancel, F6: ok") == KEY_CTRL_F6)
              {
                giac::clip_pasted = true;
                Console_Init();
                Console_Clear_EditLine();
                giac::_restart(giac::gen(giac::vecteur(0), giac::_SEQ__VECT), contextptr);
                ustl::string s(remove_path(giac::remove_extension(filename)));
                strcpy(session_filename, s.c_str());
                reload_edptr(session_filename, edptr);
              }
            }
            break;
          }
          if (smallmenu.selection == 5)
          {
            run_session();
            break;
          }
          if (smallmenu.selection == 6)
          {
            if (!edptr || merge_area(edptr->elements).size() < 2)
              edit_script(("\\\\fls0\\" + giac::remove_extension(session_filename) + ".py").c_str());
            else
              doTextArea(edptr);
            break;
          }
          if (smallmenu.selection == 7)
          {
            char filename[MAX_FILENAME_SIZE + 1];
            giac::drawRectangle(0, 0, LCD_WIDTH_PX, LCD_HEIGHT_PX - 8, COLOR_WHITE);
            if (fileBrowser(filename, (char *)"*.py", (char *)"Scripts"))
              edit_script(filename);
            break;
          }
          if (smallmenu.selection == 8)
          {
            char filename[MAX_FILENAME_SIZE + 1];
            giac::drawRectangle(0, 0, LCD_WIDTH_PX, LCD_HEIGHT_PX - 8, COLOR_WHITE);
            if (fileBrowser(filename, (char *)"*.py", (char *)"Scripts"))
              run_script(filename);
            break;
          }
          if (smallmenu.selection == 9)
          {
            chk_restart();
            Console_Init();
            Console_Clear_EditLine();
            break;
          }
          if (smallmenu.selection == 10)
          {
            erase_script();
            break;
          }
          if (smallmenu.selection == 11)
          {
            giac::drawRectangle(0, 0, LCD_WIDTH_PX, LCD_HEIGHT_PX - 8, COLOR_WHITE);
            if (ptr = input_matrix(false))
            {
              return Console_Input((const unsigned char *)ptr);
            }
            break;
          }
          if (smallmenu.selection == 12)
          {
            Menu paramenu;
            paramenu.numitems = 6;
            MenuItem paramenuitems[paramenu.numitems];
            paramenu.items = paramenuitems;
            paramenu.height =  8; //!!!!!
            paramenu.title = (char *)"Parameter";
            char menu_xcur[32], menu_xmin[32], menu_xmax[32], menu_xstep[32], menu_name[16] = "name a";
            static char curname = 'a';
            menu_name[5] = curname;
            ++curname;
            double pcur = 0, pmin = -5, pmax = 5, pstep = 0.1;
            ustl::string s;
            bool doit;
            for (;;)
            {
              s = "cur " + giac::print_DOUBLE_(pcur, 6);
              strcpy(menu_xcur, s.c_str());
              s = "min " + giac::print_DOUBLE_(pmin, 6);
              strcpy(menu_xmin, s.c_str());
              s = "max " + giac::print_DOUBLE_(pmax, 6);
              strcpy(menu_xmax, s.c_str());
              s = "step " + giac::print_DOUBLE_(pstep, 6);
              strcpy(menu_xstep, s.c_str());
              paramenuitems[0].text = (char *)"OK";
              paramenuitems[1].text = (char *)menu_name;
              paramenuitems[2].text = (char *)menu_xcur;
              paramenuitems[3].text = (char *)menu_xmin;
              paramenuitems[4].text = (char *)menu_xmax;
              paramenuitems[5].text = (char *)menu_xstep;
              int sres = doMenu(&paramenu);
              doit = sres == MENU_RETURN_SELECTION;
              if (doit)
              {
                ustl::string s1;
                double d;
                if (paramenu.selection == 2)
                {
                  giac::handle_f5();
                  if (giac::inputline(menu_name, giac::lang ? "Nouvelle valeur?" : "New value?", s1, false) == KEY_CTRL_EXE && s1.size() > 0 && isalpha(s1[0]))
                  {
                    if (s1.size() > 10)
                      s1 = s1.substr(0, 10);
                    strcpy(menu_name, ("name " + s1).c_str());
                  }
                  continue;
                }
                if (paramenu.selection == 3)
                {
                  inputdouble(menu_xcur, pcur);
                  continue;
                }
                if (paramenu.selection == 4)
                {
                  inputdouble(menu_xmin, pmin);
                  continue;
                }
                if (paramenu.selection == 5)
                {
                  inputdouble(menu_xmax, pmax);
                  continue;
                }
                if (paramenu.selection == 6)
                {
                  inputdouble(menu_xstep, pstep);
                  pstep = fabs(pstep);
                  continue;
                }
                // if (paramenu.selection==6) break;
              } // end menu
              break;
            } // end for (;;)
            if (doit && pmin < pmax && pstep > 0)
            {
              s = "assume(";
              s += (menu_name + 5);
              s += "=[";
              s += (menu_xcur + 4);
              s += ',';
              s += (menu_xmin + 4);
              s += ',';
              s += (menu_xmax + 4);
              s += ',';
              s += (menu_xstep + 5);
              s += "])";
              return Console_Input((const unsigned char *)s.c_str());
            }
            continue;
          }
          if (smallmenu.selection == 13)
          {
            menu_setup();
            continue;
          }
          if (smallmenu.selection >= 14)
          {
            textArea text;
            text.editable = false;
            text.clipline = -1;
            text.title = smallmenuitems[smallmenu.selection - 1].text;
            add(&text, smallmenu.selection == 14 ? shortcuts_string : apropos_string);
            doTextArea(&text);
            continue;
          }
        }
        break;
      } // end while(1)
      Console_Disp();
      return CONSOLE_SUCCEEDED;
#else
      char filename[MAX_FILENAME_SIZE + 1];
      // drawRectangle(0, 24, LCD_WIDTH_PX, LCD_HEIGHT_PX-24, COLOR_WHITE);
      if (get_filename(filename))
        edit_script(filename);
      // edit_script(0);
      return CONSOLE_SUCCEEDED;
#endif
    }
    if (key == KEY_CTRL_QUIT
        // || key==KEY_CTRL_SETUP
    )
    {
      const char *ptr = giac::keytostring(key, keyflag, false, contextptr);
      if (ptr)
        return Console_Input((const unsigned char *)ptr);
    }
    if (keyflag == 1)
    {
      if (key == KEY_CTRL_F1)
        return Console_Input((const unsigned char *)"simplify(");
      if (key == KEY_CTRL_F2)
        return Console_Input((const unsigned char *)"limit(");
      if (key == KEY_CTRL_F6)
        return Console_Input((const unsigned char *)" % ");
    }
    if ((key >= KEY_CTRL_F1 && key <= KEY_CTRL_F6) ||
        (key >= KEY_CTRL_F7 && key <= KEY_CTRL_F14))
    {
      return Console_FMenu(key);
    }
    if (key == KEY_CHAR_IMGNRY)
      return Console_Input((const unsigned char *)"i");
    // if (key == KEY_CHAR_MAT)		return Console_FMenu(KEY_CTRL_F8);//return Console_Input((const unsigned char *)"matrix(");
    // if (key == KEY_CHAR_LIST)		return Console_FMenu(KEY_CTRL_F9);//return Console_Input((const unsigned char *)"seq(");
    if (key == KEY_CHAR_ANGLE)
      return Console_Input((const unsigned char *)"polar_complex(");
    // if (key==KEY_CTRL_QUIT) return Console_Input((const unsigned char *)"restart");
    if (key == KEY_CHAR_DP)
      return Console_Input((const unsigned char *)".");
    if (key == KEY_CHAR_EQUAL)
      return Console_Input((const unsigned char *)"=");
    if (key == KEY_CHAR_EXP)
      return Console_Input((const unsigned char *)"ee");
    if (key == KEY_CHAR_DQUATE)
      return Console_Input((const unsigned char *)"\"");
    if (key == KEY_CHAR_SPACE)
      return Console_Input((const unsigned char *)" ");
    if (key == KEY_CHAR_PI)
      return Console_Input((const unsigned char *)"pi");
    if (key == KEY_CHAR_PMINUS)
      return Console_Input((const unsigned char *)"-");
    if (key == KEY_CHAR_MINUS)
      return Console_Input((const unsigned char *)"-");
    if (key == KEY_CHAR_ANS)
      return Console_Input((const unsigned char *)"ans()");
    if (key == KEY_CHAR_PLUS)
      return Console_Input((const unsigned char *)"+");
    if (key == KEY_CHAR_COLON)
      return Console_Input((const unsigned char *)":");
    if (key == KEY_CHAR_SEMI)
      return Console_Input((const unsigned char *)";");
    if (key == KEY_CHAR_VERBAR)
      return Console_Input((const unsigned char *)"|");
    if (key == KEY_CHAR_LBRCKT)
      return Console_Input((const unsigned char *)"[");
    if (key == KEY_CHAR_RBRCKT)
      return Console_Input((const unsigned char *)"]");
    if (key == KEY_CHAR_MULT)
      return Console_Input((const unsigned char *)"*");
    if (key == KEY_CHAR_LBRACE)
      return Console_Input((const unsigned char *)"{");
    if (key == KEY_CHAR_DIV)
      return Console_Input((const unsigned char *)"/");
    if (key == KEY_CHAR_BACKSLASH)
      return Console_Input((const unsigned char *)"\\");
    if (key == KEY_CHAR_GRAVE)
      return Console_Input((const unsigned char *)"`");
    if (key == KEY_CHAR_RBRACE)
      return Console_Input((const unsigned char *)"}");
    // if (key == KEY_CTRL_MIXEDFRAC) return Console_FMenu(KEY_CTRL_F10);//	return Console_Input((const unsigned char *)"limit(");
    // if (key==KEY_CTRL_FORMAT) return Console_Input((const unsigned char *)"restart");
    // if (key == KEY_CTRL_FD) return Console_FMenu(KEY_CTRL_F11); // return Console_Input((const unsigned char *)"exact(");
    // if (key==KEY_CTRL_PRGM) return Console_FMenu(KEY_CTRL_F12);
    // if (key == KEY_CTRL_FRACCNVRT)	return Console_FMenu(KEY_CTRL_F7);// return Console_Input((const unsigned char *)"approx(");
    if (key == KEY_CHAR_LPAR)
      return Console_Input((const unsigned char *)"(");
    if (key == KEY_CHAR_RPAR)
      return Console_Input((const unsigned char *)")");
    if (key == KEY_CHAR_CUBEROOT)
      return Console_Input((const unsigned char *)"^(1/3)");
    if (key == KEY_CHAR_RECIP)
      return Console_Input((const unsigned char *)"^(-1)");
    if (key == KEY_CHAR_COMMA)
      return Console_Input((const unsigned char *)",");
    if (key == KEY_CHAR_STORE)
      return Console_Input((const unsigned char *)"=>");
    if (key == KEY_CTRL_XTT)
      return Console_Input((const unsigned char *)"x");
    if (key == KEY_CHAR_LOG)
      return Console_Input((const unsigned char *)"log10(");
    if (key == KEY_CHAR_EXPN10)
      return Console_Input((const unsigned char *)"10^(");
    if (key == KEY_CHAR_LN)
      return Console_Input((const unsigned char *)"ln(");
    if (key == KEY_CHAR_EXPN)
      return Console_Input((const unsigned char *)"e^(");
    if (key == KEY_CHAR_SIN)
      return Console_Input((const unsigned char *)"sin(");
    if (key == KEY_CHAR_ASIN)
      return Console_Input((const unsigned char *)"asin(");
    if (key == KEY_CHAR_COS)
      return Console_Input((const unsigned char *)"cos(");
    if (key == KEY_CHAR_ACOS)
      return Console_Input((const unsigned char *)"acos(");
    if (key == KEY_CHAR_TAN)
      return Console_Input((const unsigned char *)"tan(");
    if (key == KEY_CHAR_ATAN)
      return Console_Input((const unsigned char *)"atan(");
    if (key == KEY_CHAR_SQUARE)
      return Console_Input((const unsigned char *)"^2");
    if (key == KEY_CHAR_POW)
      return Console_Input((const unsigned char *)"^");
    if (key == KEY_CHAR_ROOT)
      return Console_Input((const unsigned char *)"sqrt(");
    if (key == KEY_CHAR_VALR)
      return Console_Input((const unsigned char *)"abs(");
    if (key == KEY_CHAR_POWROOT)
      return Console_Input((const unsigned char *)"^(1/");
    if (key == KEY_CHAR_THETA)
      return Console_Input((const unsigned char *)"arg(");

    if ((key >= 'A' && key <= 'Z') ||
        (key >= 'a' && key <= 'z'))
    {
      // if (Case == LOWER_CASE) key += 'a' - 'A';
      tmp_str[0] = key;
      tmp_str[1] = 0;
      return Console_Input(tmp_str);
    }

    if (key == KEY_CTRL_UP)
      return Console_MoveCursor(alph ? CURSOR_ALPHA_UP : CURSOR_UP);
    if ((key == KEY_CTRL_DOWN || key == KEY_CTRL_F10
         //|| key==KEY_CHAR_FRAC
         ) &&
        Current_Line == Last_Line && !Line[Current_Line].readonly && Current_Col > 0)
    {
      // find cmdname
      console_help_insert(false);
      Console_Disp();
      continue;
      // keytooltip=Console_tooltip(contextptr);
    }
    if (key == KEY_CTRL_DOWN)
      return Console_MoveCursor(alph ? CURSOR_ALPHA_DOWN : CURSOR_DOWN);
    if (key == KEY_CTRL_LEFT){
      return Console_MoveCursor(CURSOR_LEFT);
    }
    if (key == KEY_CTRL_RIGHT)
      return Console_MoveCursor(CURSOR_RIGHT);
    if (key == KEY_SHIFT_LEFT)
      return Console_MoveCursor(CURSOR_SHIFT_LEFT);
    if (key == KEY_SHIFT_RIGHT)
      return Console_MoveCursor(CURSOR_SHIFT_RIGHT);

    if (key == KEY_CTRL_EXIT)
    {
      if (Last_Line == Current_Line)
      {
        if (!edptr)
          edit_script(("\\\\fls0\\" + giac::remove_extension(session_filename) + ".py").c_str());
        else
          doTextArea(edptr);
        Console_Disp();
      }
      else
      {
        move_line = Last_Line - Current_Line;
        for (i = 0; i <= move_line; i++)
          Console_MoveCursor(CURSOR_DOWN);
      }
      return CONSOLE_SUCCEEDED;
    }
    if (key == KEY_CTRL_AC)
    {
      if (Line[Current_Line].readonly)
      {
        move_line = Last_Line - Current_Line;
        for (i = 0; i <= move_line; i++)
          Console_MoveCursor(CURSOR_DOWN);
        return CONSOLE_SUCCEEDED;
      }
      if (Edit_Line[0] == '\0')
      {
        // return Console_Input((const unsigned char *)"restart");
        chk_clearscreen();
        continue;
      }
      Edit_Line[0] = '\0';
      Line[Current_Line].start_col = 0;
      Line[Current_Line].type = LINE_TYPE_INPUT;
      Line[Current_Line].disp_len = 0;
      Cursor.x = 0;
      return CONSOLE_SUCCEEDED;
    }

    if (key == KEY_CTRL_INS)
    {
      if (Current_Line < Last_Line)
      {
        Console_Insert_Line();
        Console_Insert_Line();
      }
      else {
        int c=giac::chartab();
        char s[2]={0};
        if (c>32 && c<127) s[0]=char(c);
        Console_Input(s);
      }
      Console_Disp();
      continue;
    }

    if (key == KEY_CTRL_SETUP)
    {
      menu_setup();
      Console_Disp();
    }

    if (key == KEY_CTRL_EXE)
    {
#ifdef TEX
      if (Line[Current_Line].tex_flag)
        Console_Draw_TeX_Popup(Line[Current_Line].tex_str, Line[Current_Line].tex_width, Line[Current_Line].tex_height);
      else
#endif
      {
        if (Current_Line == Last_Line)
        {
          return Console_NewLine(LINE_TYPE_INPUT, 1);
        }
      }
    }

    if (key == KEY_CTRL_DEL)
      return Console_Backspace();
    if (key == KEY_CHAR_CR && (Current_Line != Last_Line || Cursor.x == 0))
    {
      run_session(0);
      return 0;
    }
    if (key == KEY_CTRL_CLIP)
    {
      giac::copy_clipboard((const char *)Line[Current_Line].str);
    }
    if (key == KEY_CTRL_EXE)
    {
#ifdef TEX
      if (Line[Current_Line].tex_flag)
        Console_Draw_TeX_Popup(Line[Current_Line].tex_str, Line[Current_Line].tex_width, Line[Current_Line].tex_height);
      else
#endif
      {
        tmp = Line[Current_Line].str;

#if 1
        int x = editline_cursor;
        move_line = Last_Line - Current_Line;
        for (i = 0; i <= move_line; i++)
          Console_MoveCursor(CURSOR_DOWN);
        Cursor.x = x;
        if (Cursor.x > COL_DISP_MAX)
          Line[Last_Line].start_col = Cursor.x - COL_DISP_MAX;
#else
        move_line = Last_Line - Current_Line;
        for (i = 0; i <= move_line; i++)
          Console_MoveCursor(CURSOR_DOWN);
        move_col = Line[Current_Line].disp_len - Current_Col;
        for (i = 0; i <= move_col; i++)
          Console_MoveCursor(CURSOR_RIGHT);
#endif
        return Console_Input(tmp);
      }
    }
  }
  return CONSOLE_NO_EVENT;
}

static unsigned char *original_cfg = 0;

int Console_FMenu(int key)
{
  const char *s = console_menu(key, original_cfg, 0), *ptr = 0;
  if (!s)
  {
    // cout << "console " << unsigned(s) << endl;
    return CONSOLE_NO_EVENT;
  }
  if (strcmp("matrix(", s) == 0 && (ptr = input_matrix(false)))
    s = ptr;
  if (strcmp("makelist(", s) == 0 && (ptr = input_matrix(true)))
    s = ptr;
  return Console_Input((const unsigned char *)s);
}

const char *console_menu(int key, int active_app)
{
  return console_menu(key, original_cfg, active_app);
}

const char *console_menu(int key, unsigned char *cfg_, int active_app)
{
  unsigned char *cfg = cfg_;
  if (key >= KEY_CTRL_F7 && key <= KEY_CTRL_F14)
    key -= 94;
  int i, matched = 0;
  const char *ret = 0;
  const int maxentry_size = 64;
  static char console_buf[maxentry_size];
  char temp[maxentry_size], menu1[maxentry_size], menu2[maxentry_size], menu3[maxentry_size], menu4[maxentry_size], menu5[maxentry_size], menu6[maxentry_size], menu7[maxentry_size], menu8[maxentry_size];
  char *tabmenu[8] = {menu1, menu2, menu3, menu4, menu5, menu6, menu7, menu8};
  struct FMenu entry = {0, tabmenu, 0};
  // char* cfg = (char *)memory_load((char *)"\\\\fls0\\FMENU.cfg");

  while (*cfg)
  {
    // Get each line
    for (i = 0; i < maxentry_size - 1 && *cfg && *cfg != '\r' && *cfg != '\n'; i++, cfg++)
    {
      temp[i] = *cfg;
    }
    temp[i] = 0;
    // If starting by 'F' followed by the right number, start filling the structure.
    if (temp[0] == 'F' && temp[1] == (key - KEY_CTRL_F1) + '1')
    {
      matched = 1;
      continue;
    }
    if (temp[0] == 'F' && temp[1] != (key - KEY_CTRL_F1) + '0')
    {
      matched = 0;
      continue;
    }
    // Fill the structure
    if (matched && temp[0] && entry.count < 8)
    {
      strcpy(tabmenu[entry.count], temp);
      entry.count++;
    }
    cfg++;
  }
  if (entry.count > 0)
  {
    ret = Console_Draw_FMenu(key, &entry, cfg, active_app);
    // cout << "console0 " << (unsigned) ret << endl;
    if (!ret)
      return ret;
    strcpy(console_buf, ret);
    return console_buf;
  }
  return 0;
}

char *Console_Make_Entry(const unsigned char *str)
{
  char *entry = NULL;
  entry = (char *)calloc((strlen((const char *)str) + 1), sizeof(unsigned char *));
  if (entry)
    memcpy(entry, (const char *)str, strlen((const char *)str) + 1);

  return entry;
}

// Draws and runs the asked for menu.
const char *Console_Draw_FMenu(int key, struct FMenu *menu, unsigned char *cfg, int active_app)
{
  int i, nb_entries = 0, selector = 0, position_number, position_x, ret, longest = 0;
  unsigned int input_key;
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
  box.bottom = 110;  
  box.top = box.bottom - nb_entries * 12; 
  //giac::confirm((giac::print_INT_(box.left)+" "+giac::print_INT_(box.top)).c_str(),(giac::print_INT_(box.right)+" "+giac::print_INT_(box.bottom)).c_str(),false);
  Bdisp_AreaClr_VRAM(&box);
  giac::freeze=true; // temporary workaround
  giac::draw_line(box.left, box.bottom, box.left, box.top,0,0xffff);
  giac::draw_line(box.right, box.bottom, box.right, box.top,0,0xffff);
  giac::freeze=false;

  // If the cursor is flashing on the opening box, disable it. //!!!!!!
  if (((Cursor.x * (256 / 21) < box.right && Cursor.x * (256 / 21) > box.left)) && ((Cursor.y * (128 / 8) < box.bottom) && (Cursor.y * (128 / 8) > box.top)))
    Cursor_SetFlashOff();

  for (;;)
  {
    for (i = 0; i < nb_entries; i++)
    {
      quick[0] = '0' + (i + 1);
      PrintMini(3 + position_x, box.bottom - 11 * (i + 1), (unsigned char *)quick, MINI_OVER); //!!!!!
      PrintMini(3 + position_x + quick_len * 8, box.bottom - 11 * (i + 1), (unsigned char *)entries[i], MINI_OVER); //!!!!!
    }
    PrintMini(3 + position_x + quick_len * 8, box.bottom - 11 * (selector + 1), (unsigned char *)entries[selector], MINI_REV); //!!!!!
    ck_getkey((int *)&input_key);
    if (input_key == KEY_CTRL_EXIT || input_key == KEY_CTRL_AC)
      return 0;
    if (input_key == KEY_CTRL_UP && selector < nb_entries - 1)
      selector++;
    if (input_key == KEY_CTRL_DOWN && selector > 0)
      selector--;
    const char *howto = "", *syntax = "", *related = "", *examples = "";
    if (input_key == KEY_CTRL_RIGHT && giac::has_static_help(entries[selector], 1, howto, syntax, examples, related))
    {
      unsigned int key;
      PopUpWin(6);
      PrintMini(12, 6, (unsigned char *)howto, MINI_OVER);
      PrintMini(12, 14, (unsigned char *)entries[selector], MINI_OVER);
      PrintMini(16, 22, (unsigned char *)syntax, MINI_OVER);
      PrintMini(12, 30, (unsigned char *)"Example (EXE)", MINI_OVER);
      PrintMini(12, 38, (unsigned char *)examples, MINI_OVER);
      PrintMini(12, 46, (unsigned char *)"See also", MINI_OVER);
      PrintMini(12, 54, (unsigned char *)related, MINI_OVER);
      ck_getkey((int *)&key);
      if (key == KEY_CTRL_EXE)
        return examples;
      Console_Disp();
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
      Console_Disp();
      key = input_key;
      return console_menu(key, cfg, active_app);
    }
  } // end while input_key!=EXE/EXIT

  return 0; // never reached
}

/*
  ÒÔÏÂº¯ÊýÓÃÓÚ³õÊ¼»¯¡£
*/

int Console_Init()
{
  console_changed = 1;
  int i;
  if (!Line)
  {
    Line = new line[COL_LINE_MAX];
    for (i = 0; i < COL_LINE_MAX; i++)
    {
      Line[i].str = 0;
    }
  }

  Start_Line = 0;
  Last_Line = 0;

  for (i = 0; i < COL_LINE_MAX; i++)
  {
    if (Line[i].str)
    {
      free(Line[i].str);
      Line[i].str = 0;
    }
    Line[i].readonly = 0;
    Line[i].type = LINE_TYPE_INPUT;
    Line[i].start_col = 0;
    Line[i].disp_len = 0;
  }

  if ((Edit_Line = (unsigned char *)malloc(EDIT_LINE_MAX + 1)) == NULL)
    return CONSOLE_MEM_ERR;
  Edit_Line[0] = 0;
  Line[0].str = Edit_Line;

  Cursor.x = 0;
  Cursor.y = 0;

  Case = LOWER_CASE;

  /*for(i = 0; i < 6; i++) {
    FMenu_entries[i].name = NULL;
    FMenu_entries[i].count = 0;
    }*/

  Console_FMenu_Init();

  return CONSOLE_SUCCEEDED;
}

const char conf_standard[] = "F1 algb\nsimplify(\nfactor(\npartfrac(\ntcollect(\ntexpand(\nsum(\noo\nproduct(\nF2 calc\n'\ndiff(\nintegrate(\nlimit(\nseries(\nsolve(\ndesolve(\nrsolve(\nF3  2d \nreserved\nF4 menu\nreserved\nF5 A<>a\nreserved\nF6 poly\nproot(\npcoeff(\nquo(\nrem(\ngcd(\negcd(\nresultant(\nF7 arit\n mod \nirem(\nifactor(\ngcd(\nisprime(\nnextprime(\npowmod(\niegcd(\nF8 lin\nmatrix(\ndet(\nmatpow(\nranm(\ntran(\nrref(\negvl(\negv(\nF9 list\nmakelist(\nrange(\nseq(\nsize(\nappend(\nranv(\nsort(\napply(\nF: plot\nplot(\nplotseq(\nplotlist(\nplotparam(\nplotpolar(\nplotfield(\nhistogram(\nbarplot(\nF; real\nexact(\napprox(\nfloor(\nceil(\nround(\nsign(\nmax(\nmin(\nF< prog\n;\n:\n&\n#\n>\nf(x):=\ndebug(\npython(\nF= cplx\nabs(\narg(\nre(\nim(\nconj(\ncsolve(\ncfactor(\ncpartfrac(\nF> misc\n<\n>\n_\n!\n % \nrand(\nbinomial(\nnormald(";

// Loads the FMenus' data into memory, from a cfg file
void Console_FMenu_Init()
{
  int i, number = 0, key, handle;
  unsigned char *tmp_realloc = NULL;
  unsigned char temp[20] = {'\0'};
  if (!original_cfg)
  {
    original_cfg = (unsigned char *)memory_load((char *)"\\\\fls0\\FMENU.cfg");
    // Does the file exists ?
    // Todo : check the error codes...
    if (!original_cfg)
    {
#if 1
      save_script((const char *)"\\\\fls0\\FMENU.cfg", conf_standard);
#else
      memory_createfile((char *)"\\\\fls0\\FMENU.cfg", strlen((char *)conf_standard) + 1);
      handle = memory_openfile((char *)"\\\\fls0\\FMENU.cfg", _OPENMODE_READWRITE);
      memory_writefile(handle, conf_standard, strlen((char *)conf_standard) + 1);
      memory_closefile(handle);
#endif
      original_cfg = (unsigned char *)conf_standard;
    }
  }

  unsigned char *cfg = original_cfg;

  while (*cfg)
  {
    // Get each line
    for (i = 0; i < 20, *cfg && *cfg != '\r' && *cfg != '\n'; i++, cfg++)
    {
      temp[i] = *cfg;
    }
    temp[i] = 0;
    // If starting by 'F', adjust the number and eventually set the name of the menu
    if (temp[0] == 'F' && temp[1] >= '1' && temp[1] <= '6')
    {
      number = temp[1] - '0' - 1;
      if (temp[3])
      {
        strcpy(FMenu_entries_name[number], (char *)temp + 3);
        // FMenu_entries[number].name[4] = '\0';
      }
    }

    memset(temp, '\0', 20);
    cfg++;
  }
  // free(original_cfg);
}

/*
  ÒÔÏÂº¯ÊýÓÃÓÚÏÔÊ¾ËùÓÐÐÐ¡£
  ×¢Òâ£ºµ÷ÓÃ¸Ãº¯Êýºó£¬½«Ê×ÏÈÇå¿ÕÏÔ´æ¡£
  The following functions are used to display all lines.
  Note: After calling this function, the first clear the memory.
*/

int Console_Disp()
{
  unsigned int *pBitmap;
  int i, alpha_shift_status;
  DISPBOX ficon;
  int print_y = 0; // pixel y cursor
  int print_y_locate;

  Bdisp_AllClr_VRAM();

  // GetFKeyIconPointer( 0x01BE, &ficon );
  // DisplayFKeyIcon( i, ficon);

  // Reading each "line" that will be printed
  for (i = 0; (i < LINE_DISP_MAX) && (i + Start_Line <= Last_Line); i++)
  {
    line &curline = Line[i + Start_Line];

    if (i == Cursor.y)
    {
      if (curline.type == LINE_TYPE_INPUT || curline.type == LINE_TYPE_OUTPUT && curline.disp_len >= COL_DISP_MAX)
      {
        locate(1, i + 1);
        if (curline.readonly)
        {
          Cursor_SetFlashMode(0);
          PrintRev(curline.str + curline.start_col);
        }
        else
        {
          if (Cursor.x > COL_DISP_MAX - 1)
          {
            Print(curline.str + curline.start_col + 1);
          }
          else
          {
            Print(curline.str + curline.start_col);
          }
        }
      }
      else
      {
        locate(COL_DISP_MAX - curline.disp_len + 1, i + 1); 

        if (curline.readonly)
        {
          Cursor_SetFlashMode(0);
#ifdef TEX
          if (Line[i + Start_Line].tex_flag)
          {
            locate(COL_DISP_MAX - strlen(POPUP_PRETTY_STR) + 1, i + 1);
            PrintRev((unsigned char *)POPUP_PRETTY_STR);
          }
          else
#endif
          {
            PrintRev(curline.str);
          }
        }
        else
        {
          Print(curline.str);
        }
      }

      if (curline.start_col > 0)
      {
        locate(0, i + 1);

        if (curline.readonly)
        {
          Cursor_SetFlashMode(0);
          Print((unsigned char *)"<");
        }
        else
        {
          PrintRev((unsigned char *)"<");
        }
      }

      if (curline.disp_len - curline.start_col > COL_DISP_MAX - 1)
      {
        locate(COL_DISP_MAX, i + 1);
        if (curline.readonly)
        {
          if (curline.disp_len - curline.start_col != COL_DISP_MAX)
          {
            Cursor_SetFlashMode(0);
            Print((unsigned char *)"> "); 
          }
        }
        else if (Cursor.x < COL_DISP_MAX - 1)
        {
          PrintRev((unsigned char *)"> ");
        }
      }

      if (!curline.readonly)
      {
        switch (Setup_GetEntry(0x14))
        {
        case 0:
          alpha_shift_status = 0;
          break;
        case 1: // Shift enabled
          alpha_shift_status = 1;
          break;
        case 4:
        case (char)0x84: // Alpha enabled
          alpha_shift_status = 2;
          break;
        case 8:
        case (char)0x88:
          alpha_shift_status = 4;
          break;
        default:
          alpha_shift_status = 0;
          break;
        }
        Cursor_SetPosition(Cursor.x, Cursor.y);
        Cursor_SetFlashMode(1);
        Cursor_SetFlashOn(alpha_shift_status);
        // Cursor_SetFlashStyle(alpha_shift_status); //Potential 2.00 OS incompatibilty (cf Simon's doc)
      }
    } // end current cursor line display
    else
    {
      bool bigoutput = curline.type == LINE_TYPE_OUTPUT && curline.disp_len >= COL_DISP_MAX - 3;
      if (curline.type == LINE_TYPE_INPUT || bigoutput)
      {
        locate(bigoutput ? 3 : 1, i + 1);
        Print(curline.str + curline.start_col);
      }
      else
      {
#ifdef TEX
        if (Line[i + Start_Line].tex_flag)
        {
          locate(COL_DISP_MAX - strlen(POPUP_PRETTY_STR) + 1, i + 1);
          Print((unsigned char *)POPUP_PRETTY_STR);
        }
        else
#endif
        {
          locate(COL_DISP_MAX - curline.disp_len + 1, i + 1);
          Print(curline.str);
        }
      }

      if (curline.start_col > 0)
      {
        locate(1, i + 1);
        Print((unsigned char *)"<");
      }

      if (curline.disp_len - curline.start_col > COL_DISP_MAX)
      {
        locate(COL_DISP_MAX, i + 1);
        Print((unsigned char *)"> ");
      }
    }
  }

  // Draw the "fkeys icons"
#if 0
  for(i=0; i<6; i++) {
    ficon.bottom = 64;
    ficon.top = 64-8;
    if (FMenu_entries_name[i][0]!= '\0') {
      ficon.left = 1+i*21;
      ficon.right = ficon.left + (127-2)/6 - 1;
      Bdisp_AreaClr_VRAM(&ficon);
      Bdisp_AreaReverseVRAM(ficon.left, ficon.top, ficon.right, ficon.bottom);
      PrintMini(ficon.left + 2, ficon.top +2, (unsigned char*)FMenu_entries_name[i], MINI_REV);
    }
  }
#else
  string menu(menu_f1);
  while (menu.size() < F1_CHARS_LEN)
    menu += " ";
  menu += "|";
  menu += string(menu_f2);
  while (menu.size() < F1_CHARS_LEN + F2_CHARS_LEN)
    menu += " ";
  //menu += giac::lang ? "|voir |cmds |A<>a |Fich" : "|view |cmds |A<>a |File"; //!!!
  menu += CONSOLE_BAR2;
  giac::drawRectangle(0, F_KEY_BAR_Y_START - 2, LCD_WIDTH_PX, 8, COLOR_BLACK);
  PrintMini(0, F_KEY_BAR_Y_START, (const unsigned char *)menu.c_str(), MINI_REV); //!!!!!!
#endif
  // clock, if there is room in editlin
  if (1 || Last_Line < 5 || strlen((const char *)Line[Last_Line].str) < 5)
  {
    ustl::string status;
    if (1)
    {
      int heure, minute;
      giac::get_time(heure, minute);
      status += char('0' + heure / 10);
      status += char('0' + (heure % 10));
      status += ':';
      status += char('0' + (minute / 10));
      status += char('0' + (minute % 10));
      if (giac::python_compat(contextptr))
        status += " Pyth ";
      else
        status += " Xcas ";
      if (giac::angle_radian(contextptr))
        status += "RAD ";
      else
        status += "DEG ";
      status += remove_path(giac::remove_extension(session_filename));
    }
    else
    {
      void *ptr = malloc(1024);
      size_t k = (size_t)ptr;
      free(ptr);
      // size_t k=(size_t) status.c_str();
      // k &= 0x7fffffff;
      status += giac::hexa_print_INT_((int)k);
    }
    PrintMini(CLOCK_STATUS_BAR_OFFSET_X, 0, (unsigned char *)status.c_str(), MINI_REV); //!!!!
  }
  Bdisp_PutDisp_DD();
  return CONSOLE_SUCCEEDED;
}

void dConsoleRedraw()
{
  Console_Disp();
}

/*
  Draw a popup at the center of the screen containing the str expression drawn in pretty print.
*/
#if defined POPUP_PRETTY && defined TEX
void Console_Draw_TeX_Popup(unsigned char *str, int width, int height)
{
  DISPBOX popup;
  DISPBOX temp;
  unsigned char arrows[4 * 3] = {0xE6, 0x9A, '\0', 0xE6, 0x9B, '\0', 0xE6, 0x9C, '\0', 0xE6, 0x9D, '\0'};
  int margin = 2, border = 1;
  int scroll_lateral = 0, scroll_lateral_flag = 0, scroll_vertical = 0, scroll_vertical_flag = 0;
  int key;

  if (width > 115)
  {
    popup.left = 5;
    popup.right = 122;

    scroll_lateral_flag = 1;
  }
  else
  {
    popup.left = 64 - width / 2 - margin - border;
    popup.right = 128 - popup.left;
  }

  if (height > 50)
  {
    popup.top = 5;
    popup.bottom = 57;

    scroll_vertical_flag = 1;
  }
  else
  {
    popup.top = 32 - height / 2 - margin - border;
    popup.bottom = 64 - popup.top;
  }

  /*temp.left = 0; temp.top = 0; temp.right = 128; temp.bottom = 64;
    Bdisp_ReadArea_VRAM (&temp, vram_copy);*/

  while (key != KEY_CTRL_EXIT)
  {
    Bdisp_AreaClr_VRAM(&popup);
    Bdisp_AreaReverseVRAM(popup.left, popup.top, popup.right, popup.bottom);

    Bdisp_AreaReverseVRAM(popup.left + border, popup.top + border, popup.right - border, popup.bottom - border);

    TeX_drawComplex((char *)str, popup.left + border + margin + scroll_lateral, popup.top + border + margin + scroll_vertical);

    if (scroll_lateral_flag || scroll_vertical_flag)
    {
      temp.left = 0;
      temp.top = 0;
      temp.right = popup.left - 1;
      temp.bottom = popup.bottom;
      Bdisp_AreaClr_VRAM(&temp);
      temp.left = 0;
      temp.top = popup.bottom + 1;
      temp.right = 127;
      temp.bottom = 63;
      Bdisp_AreaClr_VRAM(&temp);
      temp.left = popup.left - 1;
      temp.top = 0;
      temp.right = 127;
      temp.bottom = popup.top - 1;
      Bdisp_AreaClr_VRAM(&temp);
      temp.left = popup.right + 1;
      temp.top = popup.top - 1;
      temp.right = 127;
      temp.bottom = 63;
      Bdisp_AreaClr_VRAM(&temp);

      if (scroll_lateral < 0)
        PrintMini(1, 30, arrows, 0);
      if (scroll_lateral > -(width - 115))
        PrintMini(123, 30, arrows + 3, 0);
      if (scroll_vertical < 0)
        PrintMini(61, 0, arrows + 6, 0);
      if (scroll_vertical > -(height - 47))
        PrintMini(61, 58, arrows + 9, 0);

      Bdisp_DrawLineVRAM(popup.left, popup.top, popup.left, popup.bottom);
      Bdisp_DrawLineVRAM(popup.left, popup.top, popup.right, popup.top);
      Bdisp_DrawLineVRAM(popup.left, popup.bottom, popup.right, popup.bottom);
      Bdisp_DrawLineVRAM(popup.right, popup.top, popup.right, popup.bottom);
    }

    ck_getkey((int *)&key);

    if (scroll_lateral_flag)
    {
      if (key == KEY_CTRL_LEFT && scroll_lateral < 0)
        scroll_lateral += 5;
      if (key == KEY_CTRL_RIGHT && scroll_lateral > -(width - 115))
        scroll_lateral -= 5;

      if (scroll_lateral > 0)
        scroll_lateral = 0;
    }
    if (scroll_vertical_flag)
    {
      if (key == KEY_CTRL_UP && scroll_vertical < 0)
        scroll_vertical += 3;
      if (key == KEY_CTRL_DOWN && scroll_vertical > -(height - 47))
        scroll_vertical -= 3;

      if (scroll_vertical > 0)
        scroll_vertical = 0;
    }
  }
}
#endif

/*
  ÒÔÏÂº¯ÊýÓÃÓÚÊäÈëÐÐ£¬³É¹¦ºó½«·µ»Ø¸ÃÐÐµÄ×Ö·û´®¡£
*/

unsigned char *Console_GetLine()
{
  int return_val;

  do
  {
    return_val = Console_GetKey();
    Console_Disp();
    if (return_val == CONSOLE_MEM_ERR)
      return NULL;
  } while (return_val != CONSOLE_NEW_LINE_SET);

  return Line[Current_Line - 1].str;
}

/*
  Simple accessor to the Edit_Line buffer.
*/
unsigned char *Console_GetEditLine()
{
  return Edit_Line;
}
