//#define DBG 0
#include "giacPCH.h"
#include "input_lexer.h"
#include "input_parser.h"
#include "kdisplay.h"
#include "porting.h"
#include <stdio.h>
#include <setjmp.h>
#include <alloca.h>

#include "defs.h"

#include "console.h"
#include "libfx.h"
extern "C"
{
#include "syscalls.h"
}
#include "memory.h"
//#include "menu_config.h"
#include "menuGUI.h"
#include "textGUI.h"
#include "file.h"
#include "main.h"
//#include <gint/kmalloc.h>
#define EXPR_BUF_SIZE 256
#define GIAC_HISTORY_SIZE 2
#define GIAC_HISTORY_MAX_TAILLE 32

#define USER_FUNCTIONS "\\\\fls0\\USER.eig"
#define USER_FUNCTIONS_MAX_LENGTH 200
int clip_ymin=0;

int ck_getkey(int *keyptr)
{
  int keyflag = GetSetupSetting((unsigned int)0x14);
  GetKey(keyptr);
  if (*keyptr >= KEY_CTRL_F1 && *keyptr <= KEY_CTRL_F6 && (keyflag & 1))
    *keyptr += 100;
  return 1;
}

using namespace giac;
giac::context *contextptr = 0;
int esc_flag = 0;

// extern U ** mem;
// extern unsigned int **free_stack;

#define SYMBOLSSTATEFILE (char *)"\\\\fls0\\lastvar.py"

int find_color(const char *s)
{
  if (!s)
    return 0;
  char ch = s[0];
  if (ch == '"')
    return _YELLOW;
  if (!isalpha(s[0]))
    return 0;
  char buf[256];
  const char *ptr = s;
  for (int i = 0; i < 255 && (is_alphanum(*ptr) || *ptr == '_'); ++i)
  {
    ++ptr;
  }
  strncpy(buf, s, ptr - s);
  buf[ptr - s] = 0;
  // cout << "find:" << buf << " " << strlen(buf) << endl;
  if (strcmp(buf, "def") == 0)
    return _RED;
  // int pos=dichotomic_search(keywords,sizeof(keywords),buf);
  // if (pos>=0) return 1;
  gen g;
  int token = find_or_make_symbol(buf, g, 0, false, contextptr);
  //*logptr(contextptr) << s << " " << buf << " " << token << " " << g << endl;
  if (token == T_UNARY_OP || token == T_UNARY_OP_38 || token == T_LOGO)
    return _BLUE;
  if (token == T_NUMBER)
    return _CYAN;
  if (token != T_SYMBOL)
    return _RED;
  return 0;
}

int handle_f5(){
  int keyflag = GetSetupSetting( (unsigned int)0x14);
  if (keyflag == 0x04 || keyflag == 0x08 || keyflag == 0x84 || keyflag == 0x88) {
    // ^only applies if some sort of alpha (not locked) is already on
    if (keyflag == 0x08 || keyflag == 0x88) { //if lowercase
      SetSetupSetting( (unsigned int)0x14, keyflag-0x04);
      // DisplayStatusArea();
      return 1; //do not process the key, because otherwise we will leave alpha status
    } else {
      SetSetupSetting( (unsigned int)0x14, keyflag+0x04);
      // DisplayStatusArea();
      return 1; //do not process the key, because otherwise we will leave alpha status
    }
  }
  if (keyflag==0) {
    SetSetupSetting( (unsigned int)0x14, 0x88);	
    // DisplayStatusArea();
  }
  return 0;
}

ustl::string get_searchitem(ustl::string &replace)
{
  replace = "";
  ustl::string search;
  handle_f5();
  int res = inputline(lang ? "EXIT ou chaine vide: annulation" : "EXIT or empty string: cancel", lang ? "Chercher:" : "Search:", search, false);
  if (search.empty() || res == KEY_CTRL_EXIT)
    return "";
  replace = "";
  ustl::string tmp = (lang ? "EXIT: recherche seule de " : "EXIT: search only ") + search;
  handle_f5();
  res = inputline(tmp.c_str(), lang ? "Remplacer par:" : "Replace by:", replace, false);
  if (res == KEY_CTRL_EXIT)
    replace = "";
  return search;
}

bool save_script(const char *filename, const ustl::string &s)
{
  // cout << "save " << filename << " " << strlen(filename) << endl;
  unsigned short pFile[MAX_FILENAME_SIZE + 1];
  // create file in data folder (assumes data folder already exists)
  Bfile_StrToName_ncpy(pFile, (const unsigned char *)filename, strlen(filename) + 1);
  // even if it already exists, there's no problem,
  // in the event that our file shrinks, we just let junk be at the end of
  // the file (after two null bytes, of course).
  int hFile = Bfile_OpenFile_OS(pFile, _OPENMODE_READWRITE); // Get handle
  bool open = false;
  if (hFile < 0)
  {
  }
  else
  {
    size_t old = Bfile_GetFileSize_OS(hFile);
    if (old <= s.size())
      open = true;
    else
    {
      Bfile_CloseFile_OS(hFile);
      Bfile_DeleteEntry(pFile);
    }
  }
  if (!open)
  {
    // file does not exist yet. try creating it
    // (data folder should exist already, as save_console_state_smem() should have been called before this function)
    size_t size = 1;
    Bfile_CreateFile(pFile, size);
    // now try opening
    hFile = Bfile_OpenFile_OS(pFile, _OPENMODE_READWRITE); // Get handle
    if (hFile < 0)
      return false; // if it still fails, there's nothing we can do
  }
  Bfile_WriteFile_OS(hFile, s.c_str(), s.size());
  char buf[2] = {0, 0};
  Bfile_WriteFile_OS(hFile, buf, 2);
  Bfile_CloseFile_OS(hFile);
  return true;
}

int load_script(const char *filename, string &s)
{
  unsigned short pFile[MAX_FILENAME_SIZE + 1];
  Bfile_StrToName_ncpy(pFile, (const unsigned char *)filename, strlen(filename) + 1);
  int hFile = Bfile_OpenFile_OS(pFile, _OPENMODE_READWRITE); // Get handle
  // Check for file existence
  if (hFile < 0)
    return -1;
  // Returned no error, file exists, open it
  int size = Bfile_GetFileSize_OS(hFile);
  // File exists and has size 'size'
  // Read file into a buffer
  if ((unsigned int)size > MAX_TEXTVIEWER_FILESIZE)
  {
    Bfile_CloseFile_OS(hFile);
    puts("Stop: script too big");
    return 0; // file too big, return
  }
  unsigned char *asrc = (unsigned char *)alloca(size * sizeof(unsigned char) + 5); // 5 more bytes to make sure it fits...
  memset(asrc, 0, size + 5);                                                       // alloca does not clear the allocated space. Make sure the string is null-terminated this way.
  int rsize = Bfile_ReadFile_OS(hFile, asrc, size, 0);
  Bfile_CloseFile_OS(hFile); // we got file contents, close it
  asrc[rsize] = '\0';
  s = string((char *)asrc);
  return 1;
}

int run_script(const char *filename)
{
  // returns 1 if script was run, 0 otherwise
  unsigned short pFile[MAX_FILENAME_SIZE + 1];
  Bfile_StrToName_ncpy(pFile, (unsigned char *)filename, strlen(filename) + 1);
  int hFile = Bfile_OpenFile_OS(pFile, _OPENMODE_READWRITE); // Get handle
  if (hFile < 0)
    return 0;
  // Check for file existence
  if (hFile >= 0) // Check if it opened
  {
    // Returned no error, file exists, open it
    int size = Bfile_GetFileSize_OS(hFile);
    // File exists and has size 'size'
    // Read file into a buffer
    if ((unsigned int)size > MAX_TEXTVIEWER_FILESIZE)
    {
      Bfile_CloseFile_OS(hFile);
      puts("Stop: script too big");
      return 0; // file too big, return
    }
    unsigned char *asrc = (unsigned char *)alloca(size * sizeof(unsigned char) + 5); // 5 more bytes to make sure it fits...
    memset(asrc, 0, size + 5);                                                       // alloca does not clear the allocated space. Make sure the string is null-terminated this way.
    int rsize = Bfile_ReadFile_OS(hFile, asrc, size, 0);
    Bfile_CloseFile_OS(hFile); // we got file contents, close it
    asrc[rsize] = '\0';
    gen g, ge;
    do_run((char *)asrc, g, ge);
    if (asrc[0] == '#' || (asrc[0] == 'd' && asrc[1] == 'e' && asrc[2] == 'f' && asrc[3] == ' '))
      return 2;
    if ((asrc[0] == '/' && asrc[1] == '/') ||
        (rsize > 8 && asrc[0] == 'f' && (asrc[1] == 'o' || asrc[1] == 'u') && asrc[2] == 'n' && asrc[3] == 'c' && asrc[4] == 't' && asrc[5] == 'i' && asrc[6] == 'o' && asrc[7] == 'n' && asrc[8] == ' '))
      return 3;
    return 1;
  }
  return 0;
}

int select_script_and_run()
{
  char filename[MAX_FILENAME_SIZE + 1];
  if (fileBrowser(filename, (char *)"*.py", (char *)"Scripts"))
    return run_script(filename);
  return 0;
}

void erase_script()
{
  char filename[MAX_FILENAME_SIZE + 1];
  int res = fileBrowser(filename, (char *)"*.py", (char *)"Scripts");
  if (res && do_confirm(lang ? "Vraiment effacer" : "Really erase?"))
  {
    unsigned short pFile[MAX_FILENAME_SIZE + 1];
    // create file in data folder (assumes data folder already exists)
    Bfile_StrToName_ncpy(pFile, (const unsigned char *)filename, strlen(filename) + 1);
    int hFile = Bfile_OpenFile_OS(pFile, _OPENMODE_READWRITE); // Get handle
    if (hFile >= 0)
    {
      Bfile_CloseFile_OS(hFile);
      Bfile_DeleteEntry(pFile);
    }
  }
}

string extract_name(const char *s)
{
  int l = strlen(s), i, j;
  for (i = l - 1; i >= 0; --i)
  {
    if (s[i] == '.')
      break;
  }
  if (i <= 0)
    return "f";
  for (j = i - 1; j >= 0; --j)
  {
    if (s[j] == '\\')
      break;
  }
  if (j < 0)
    return "f";
  return string(s + j + 1).substr(0, i - j - 1);
}

void edit_script(const char *fname)
{
  clear_abort();
  char fname_[MAX_FILENAME_SIZE + 1];
  char *filename = 0;
  int res = 1;
  if (fname)
    filename = (char *)fname; // safe, it will not be modified
  else
  {
    res = fileBrowser(fname_, (char *)"*.py", (char *)"Scripts");
    filename = fname_;
  }
  if (res)
  {
    string s;
    load_script(filename, s);
    if (s.empty())
    {
      s = python_compat(contextptr) ? (lang ? "Prg Python sinon " : "Python prog., for Xcas") : (lang ? "Prog Xcas, sinon " : "Xcas prog., for Python");
      s += " AC F6 8";
      int k = confirm(s.c_str(), "F1: Tortue, F6: Prog", true);
      if (k == -1)
        return;
      if (k == KEY_CTRL_F1)
        s = "\nefface;\n ";
      else
        s = python_compat(contextptr) ? "def " + extract_name(filename) + "(x):\n  \n  return x" : "function " + extract_name(filename) + "(x)\nlocal j;\n  \n  return j;\nffunction";
    }
    // split s at newlines
    if (edptr == 0)
      edptr = new textArea;
    if (!edptr)
      return;
    edptr->elements.clear();
    edptr->clipline = -1;
    edptr->filename = "\\\\fls0\\" + remove_path(giac::remove_extension(filename)) + ".py";
    // cout << "script " << edptr->filename << endl;
    edptr->editable = true;
    edptr->changed = false;
    edptr->python = python_compat(contextptr);
    edptr->elements.clear();
    edptr->y = 12;//!!!!!
    edptr->lineHeight=16;
    edptr->longlinescut=false;
    add(edptr, s);
    s.clear();
    edptr->line = 0;
    // edptr->line=edptr->elements.size()-1;
    edptr->pos = 0;
    int res = doTextArea(edptr);
    if (res == -1)
      python_compat(edptr->python, contextptr);
  }
}

string khicas_state()
{
  giac::gen g(giac::_VARS(-1, contextptr));
  int b = python_compat(contextptr);
  python_compat(0, contextptr);
#if 1
  char buf[4096] = "";
  if (g.type == giac::_VECT)
  {
    for (int i = 0; i < g._VECTptr->size(); ++i)
    {
      string s((*g._VECTptr)[i].print(contextptr));
      if (strlen(buf) + s.size() + 128 < sizeof(buf))
      {
        strcat(buf, s.c_str());
        strcat(buf, ":;");
      }
    }
  }
  python_compat(b, contextptr);
  if (strlen(buf) + 128 < sizeof(buf))
  {
    strcat(buf, "python_compat(");
    strcat(buf, giac::print_INT_(b).c_str());
    strcat(buf, ");angle_radian(");
    strcat(buf, angle_radian(contextptr) ? "1" : "0");
    strcat(buf, ");with_sqrt(");
    strcat(buf, withsqrt(contextptr) ? "1" : "0");
    strcat(buf, ");");
  }
  return buf;
#else
  string s(g.print(contextptr));
  python_compat(b, contextptr);
  s += "; python_compat(";
  s += giac::print_INT_(b);
  s += ");angle_radian(";
  s += angle_radian(contextptr) ? '1' : '0';
  s += ");with_sqrt(";
  s += withsqrt(contextptr) ? '1' : '0';
  s += ");";
  return s;
#endif
}

void save_khicas_symbols_smem(const char *filename)
{
  // save variables in xcas mode,
  // because at load time the parser will be in xcas mode
  string s(khicas_state());
  save_script(filename, s);
}

void save(const char *fname)
{
  clear_abort();
  string filename(remove_path(remove_extension(fname)));
  save_console_state_smem(("\\\\fls0\\" + filename + ".xw").c_str()); // call before save_khicas_symbols_smem(), because this calls create_data_folder if necessary!
  // save_khicas_symbols_smem(("\\\\fls0\\"+filename+".xw").c_str());
  if (edptr)
    check_leave(edptr);
}

void save_session()
{
  if (strcmp(session_filename, "session") && console_changed)
  {
    ustl::string tmp(session_filename);
    tmp += lang ? " a ete modifie!" : " was modified!";
    if (giac::confirm(tmp.c_str(), lang ? "F1: sauvegarder, F6: tant pis" : "F1: save, F6: discard changes") == KEY_CTRL_F1)
    {
      save(session_filename);
      console_changed = 0;
    }
  }
  save("session");
  // this is only called on exit, no need to reinstall the check_execution_abort timer.
  if (edptr && edptr->changed && edptr->filename != "\\\\fls0\\session.py")
  {
    if (!check_leave(edptr))
    {
      save_script("\\\\fls0\\lastprg.py", merge_area(edptr->elements));
    }
  }
}

int restore_session(const char *fname)
{
  string filename(remove_path(remove_extension(fname)));
  if (load_console_state_smem(("\\\\fls0\\" + filename + ".xw").c_str()))
    return 1;
  else
  {
    PrintMini(0, 0, (unsigned char *)"KhiCAS 1.5 (c)  B. Parisse et al", 0); //!!!!
    //PrintMini(0, 12, (unsigned char *)"", 0); //!!!!
    PrintMini(0, 24, (unsigned char *)"License GPL 2", 0);
    PrintMini(0, 80, (unsigned char *)"Do not use if CAS is forbidden", 0);
    if (confirm("Syntax?", "F1: Xcas, F6: Python") == KEY_CTRL_F6)
      python_compat(true, contextptr);
    Bdisp_AllClr_VRAM();
    return 0;
  }
}

#ifdef TEX
#include "TeX.h"
int tex_flag = 1;
extern "C"
{
  int *get_tex_flag_address()
  {
    return &tex_flag;
  }
}

void TeX_init(void)
{
  Txt_Init(FONT_SYSTEM);
}

void TeX_quit(void)
{
  Txt_Quit();
}
#endif

bool ispnt(const gen &g)
{
  if (g.is_symb_of_sommet(giac::at_pnt))
    return true;
  if (g.type != _VECT || g._VECTptr->empty())
    return false;
  return ispnt(g._VECTptr->back());
}

#if 1
#define KEY_CTRL_UNDO 30045
#define EQW_TAILLE 30 //!!!!!
giac::gen eqw(const giac::gen &ge, bool editable)
{
  bool edited = false;
  giac::gen geq(_copy(ge, contextptr));
  // if (ge.type!=giac::_DOUBLE_ && giac::has_evalf(ge,geq,1,contextptr)) geq=giac::symb_equal(ge,geq);
  int line = -1, col = -1, nlines = 0, ncols = 0, listormat = 0;
  xcas::Equation eq(0, 0, geq,contextptr);
  giac::eqwdata eqdata = xcas::Equation_total_size(eq.data);
#if 1
  if (eqdata.dx > 1.5 * LCD_WIDTH_PX || eqdata.dy > 1.5 * LCD_HEIGHT_PX)
  {
    if (eqdata.dx > 2.25 * LCD_WIDTH_PX || eqdata.dy > 2.25 * LCD_HEIGHT_PX)
      eq.attr = giac::attributs(10, COLOR_WHITE, COLOR_BLACK);
    else
      eq.attr = giac::attributs(12, COLOR_WHITE, COLOR_BLACK);
    eq.data = 0; // clear memory
    eq.data = xcas::Equation_compute_size(geq, eq.attr, LCD_WIDTH_PX, contextptr);
    eqdata = xcas::Equation_total_size(eq.data);
  }
#endif
  int dx = (eqdata.dx - LCD_WIDTH_PX) / 2, dy = LCD_HEIGHT_PX - 2 * EQW_TAILLE + eqdata.y;
  if (geq.type == _VECT)
  {
    nlines = geq._VECTptr->size();
    if (eqdata.dx >= LCD_WIDTH_PX)
      dx = -20; // line=nlines/2;
    // else
    if (geq.subtype != _SEQ__VECT)
    {
      line = 0;
      listormat = 1;
      if (ckmatrix(geq))
      {
        ncols = geq._VECTptr->front()._VECTptr->size();
        if (eqdata.dy >= LCD_HEIGHT_PX - EQW_TAILLE)
          dy = eqdata.y + eqdata.dy + 20; // col=ncols/2;
        // else
        col = 0;
        listormat = 2;
      }
    }
  }
  if (!listormat)
  {
    xcas::Equation_select(eq.data, true);
    xcas::eqw_select_down(eq.data);
  }
  int firstrun = 1;
  for (;;)
  {
    Cursor_SetFlashOff();
    gen value;
    if (listormat) // select line l, col c
      xcas::eqw_select(eq.data, line, col, true, value);
    if (eqdata.dx > LCD_WIDTH_PX)
    {
      if (dx < -20)
        dx = -20;
      if (dx > eqdata.dx - LCD_WIDTH_PX + 20)
        dx = eqdata.dx - LCD_WIDTH_PX + 20;
    }
    if (eqdata.dy > LCD_HEIGHT_PX - 2 * EQW_TAILLE)
    {
      if (dy - eqdata.y < LCD_HEIGHT_PX - 2 * EQW_TAILLE)
        dy = eqdata.y + LCD_HEIGHT_PX - 2 * EQW_TAILLE;
      if (dy - eqdata.y > eqdata.dy + 12)  //!!!!!!
        dy = eqdata.y + eqdata.dy + 12; //!!!!!!
    }
    Bdisp_AllClr_VRAM();
    int save_clip_ymin = clip_ymin;
    clip_ymin = 0;
    xcas::display(eq, dx, dy,contextptr);
    // draw_menu(2); // FIXME menus
    clip_ymin = save_clip_ymin;
    int keyflag = (unsigned char)Setup_GetEntry(0x14);
    bool alph = keyflag == 4 || keyflag == 0x84 || keyflag == 8 || keyflag == 0x88;
    if (firstrun)
    { // workaround for e.g. 1+x/2 partly not displayed
      firstrun = 0;
      continue;
    }
    string menu(menu_f1);
    while (menu.size() < F1_CHARS_LEN)
      menu += " ";
    menu += "|";
    menu += string(menu_f2);
    while (menu.size() < F1_CHARS_LEN + F2_CHARS_LEN)
      menu += " ";
    menu += CAS_VIEW_BAR;
    drawRectangle(0, F_KEY_BAR_Y_START - 2, LCD_WIDTH_PX, 8, COLOR_BLACK);
    PrintMini(0, F_KEY_BAR_Y_START, (const unsigned char *)menu.c_str(), MINI_REV); //!!!!
    giac::print_alpha_shift(keyflag);
    unsigned int key;
    ck_getkey((int *)&key);
    if (key == KEY_CTRL_UNDO)
    {
      giac::swapgen(eq.undodata, eq.data);
      if (listormat)
      {
        xcas::do_select(eq.data, true, value);
        if (value.type == _EQW)
        {
          gen g = eval(value._EQWptr->g, 1, contextptr);
          if (g.type == _VECT)
          {
            const vecteur &v = *g._VECTptr;
            nlines = v.size();
            if (line >= nlines)
              line = nlines - 1;
            if (col != -1 && v.front().type == _VECT)
            {
              ncols = v.front()._VECTptr->size();
              if (col >= ncols)
                col = ncols - 1;
            }
            xcas::do_select(eq.data, false, value);
            xcas::eqw_select(eq.data, line, col, true, value);
          }
        }
      }
      continue;
    }
    if (key == KEY_CTRL_F5 && keyflag != 1)
    {
      handle_f5();
      continue;
    }
    int redo = 0;
    if (listormat)
    {
      if (key == KEY_CHAR_COMMA || key == KEY_CTRL_DEL)
      {
        xcas::do_select(eq.data, true, value);
        if (value.type == _EQW)
        {
          gen g = eval(value._EQWptr->g, 1, contextptr);
          if (g.type == _VECT)
          {
            edited = true;
            eq.undodata = xcas::Equation_copy(eq.data);
            vecteur v = *g._VECTptr;
            if (key == KEY_CHAR_COMMA)
            {
              if (col == -1 || (line > 0 && line == nlines - 1))
              {
                v.insert(v.begin() + line + 1, 0 * v.front());
                ++line;
                ++nlines;
              }
              else
              {
                v = mtran(v);
                v.insert(v.begin() + col + 1, 0 * v.front());
                v = mtran(v);
                ++col;
                ++ncols;
              }
            }
            else
            {
              if (col == -1 || (nlines >= 3 && line == nlines - 1))
              {
                if (nlines >= (col == -1 ? 2 : 3))
                {
                  v.erase(v.begin() + line, v.begin() + line + 1);
                  if (line)
                    --line;
                  --nlines;
                }
              }
              else
              {
                if (ncols >= 2)
                {
                  v = mtran(v);
                  v.erase(v.begin() + col, v.begin() + col + 1);
                  v = mtran(v);
                  if (col)
                    --col;
                  --ncols;
                }
              }
            }
            geq = gen(v, g.subtype);
            key = 0;
            redo = 1;
            // continue;
          }
        }
      }
    }
    bool ins = key == KEY_CHAR_STORE || key == KEY_CHAR_RPAR || key == KEY_CHAR_LPAR || key == KEY_CHAR_COMMA || key == KEY_CTRL_PASTE;
    int xleft, ytop, xright, ybottom, gselpos;
    gen *gsel = 0, *gselparent = 0;
    ustl::string varname;
    if (key == KEY_CTRL_CLIP)
    {
      xcas::Equation_adjust_xy(eq.data, xleft, ytop, xright, ybottom, gsel, gselparent, gselpos, 0);
      if (gsel == 0)
        gsel == &eq.data;
      // cout << "var " << g << " " << eq.data << endl;
      if (xcas::do_select(*gsel, true, value) && value.type == _EQW)
      {
        // cout << g << ":=" << value._EQWptr->g << endl;
        copy_clipboard(value._EQWptr->g.print(contextptr), true);
        continue;
      }
    }
    if (key == KEY_CHAR_STORE)
    {
      int keyflag = Setup_GetEntry(0x14);
      if (keyflag == 0)
        handle_f5();
      if (inputline(lang ? "Stocker la selection dans" : "Save selection in", lang ? "Nom de variable: " : "Variable name: ", varname, false) && !varname.empty() && isalpha(varname[0]))
      {
        giac::gen g(varname, contextptr);
        giac::gen ge(eval(g, 1, contextptr));
        if (g.type != _IDNT)
        {
          invalid_varname();
          continue;
        }
        if (ge == g || confirm_overwrite())
        {
          vector<int> goto_sel;
          xcas::Equation_adjust_xy(eq.data, xleft, ytop, xright, ybottom, gsel, gselparent, gselpos, &goto_sel);
          if (gsel == 0)
            gsel == &eq.data;
          // cout << "var " << g << " " << eq.data << endl;
          if (xcas::do_select(*gsel, true, value) && value.type == _EQW)
          {
            giac::gen gg(value._EQWptr->g);
            if (gg.is_symb_of_sommet(at_makevector))
              gg = giac::eval(gg, 1, contextptr);
            giac::sto(gg, g, contextptr);
            // cout << g << ":=" << value._EQWptr->g << endl;
          }
        }
      }
      continue;
    }
    if (key == KEY_CTRL_DEL)
    {
      vector<int> goto_sel;
      if (xcas::Equation_adjust_xy(eq.data, xleft, ytop, xright, ybottom, gsel, gselparent, gselpos, &goto_sel) && gsel && xcas::do_select(*gsel, true, value) && value.type == _EQW)
      {
        value = value._EQWptr->g;
        if (value.type == _SYMB)
        {
          gen tmp = value._SYMBptr->feuille;
          if (tmp.type != _VECT || tmp.subtype != _SEQ__VECT)
          {
            xcas::replace_selection(eq, tmp, gsel, &goto_sel,contextptr);
            continue;
          }
        }
        if (!goto_sel.empty() && gselparent && gselparent->type == _VECT && !gselparent->_VECTptr->empty())
        {
          vecteur &v = *gselparent->_VECTptr;
          if (v.back().type == _EQW)
          {
            gen opg = v.back()._EQWptr->g;
            if (opg.type == _FUNC)
            {
              int i = 0;
              for (; i < v.size() - 1; ++i)
              {
                if (&v[i] == gsel)
                  break;
              }
              if (i < v.size() - 1)
              {
                if (v.size() == 5 && (opg == at_integrate || opg == at_sum) && i >= 2)
                  v.erase(v.begin() + 2, v.begin() + 4);
                else
                  v.erase(v.begin() + i);
                xcas::do_select(*gselparent, true, value);
                if (value.type == _EQW)
                {
                  value = value._EQWptr->g;
                  // cout << goto_sel << " " << value << endl; continue;
                  if (v.size() == 2 && (opg == at_plus || opg == at_prod || opg == at_pow))
                    value = eval(value, 1, contextptr);
                  goto_sel.erase(goto_sel.begin());
                  xcas::replace_selection(eq, value, gselparent, &goto_sel,contextptr);
                  continue;
                }
              }
            }
          }
        }
      }
    }
    if (key == KEY_CTRL_SETUP)
    {
      xcas::do_select(eq.data, true, value);
      if (value.type == _EQW)
        geq = value._EQWptr->g;
      eq.attr.fontsize = 16 - eq.attr.fontsize; //!!!!14
      eq.data = 0; // clear memory
      eq.data = xcas::Equation_compute_size(geq, eq.attr, LCD_WIDTH_PX, contextptr);
      eqdata = xcas::Equation_total_size(eq.data);
      dx = (eqdata.dx - LCD_WIDTH_PX) / 2;
      dy = LCD_HEIGHT_PX - 2 * EQW_TAILLE + eqdata.y;
      if (!listormat)
      {
        xcas::Equation_select(eq.data, true);
        xcas::eqw_select_down(eq.data);
      }
      continue;
    }
    if (key == KEY_CTRL_F3)
    {
#if 0
      edited=true;
      ins=true;
#else
      if (keyflag == 1)
      {
        if (eq.attr.fontsize >= 16) //!!!!8
          continue;
        xcas::do_select(eq.data, true, value);
        if (value.type == _EQW)
          geq = value._EQWptr->g;
        eq.attr.fontsize = 16; //!!!!8
        redo = 1;
      }
      else
      {
        if (alph)
        {
          if (eq.attr.fontsize <= 12) //!!!!6
            continue;
          xcas::do_select(eq.data, true, value);
          if (value.type == _EQW)
            geq = value._EQWptr->g;
          eq.attr.fontsize = 12; //!!!!6
          redo = 1;
        }
        else
        {
          // Edit
          edited = true;
          ins = true;
        }
      }
#endif
    }
    if (key == KEY_CHAR_IMGNRY)
      key = 'i';
    const char keybuf[2] = {(key == KEY_CHAR_PMINUS ? '-' : char(key)), 0};
    const char *adds = (key == KEY_CHAR_PMINUS ||
                        (key == char(key) && (is_alphanum(key) || key == '.')))
                           ? keybuf
                           : keytostring(key, keyflag, 0, contextptr);
    char catopn[256];
    translate_fkey(key);
    if (key == KEY_CTRL_F6)
    {
      adds = alph ? "regroup" : (keyflag == 1 ? "evalf" : "eval");
    }
    if (key == KEY_CTRL_F2)
    {
      edited = true;
      adds = console_menu(key, 1); // alph?"sum":(keyflag==1?"integrate":"'");
      if (!adds)
        continue;
      // workaround for infinitiy
      if (strlen(adds) >= 2 && adds[0] == 'o' && adds[1] == 'o')
        key = KEY_CTRL_F5;
    }
    if (key == KEY_CTRL_F1 ||
        (key >= KEY_CTRL_F7 && key <= KEY_CTRL_F14))
    {
      edited = true;
      adds = console_menu(key, 1); // alph?"simplify":(keyflag==1?"factor":"partfrac");
      if (!adds)
        continue;
    }
    if (key == KEY_CTRL_F5)
      adds = "oo";
    if (key == KEY_CHAR_MINUS)
      adds = "-";
    if (key == KEY_CHAR_EQUAL)
      adds = "=";
    if (key == KEY_CHAR_RECIP)
      adds = "inv";
    if (key == KEY_CHAR_SQUARE)
      adds = "sq";
    if (key == KEY_CHAR_POWROOT)
      adds = "surd";
    if (key == KEY_CHAR_CUBEROOT)
      adds = "surd";
    int addssize = adds ? strlen(adds) : 0;
    // cout << addssize << " " << adds << endl;
    if (key == KEY_CTRL_EXE)
    {
      if (xcas::do_select(eq.data, true, value) && value.type == _EQW)
      {
        // cout << "ok " << value._EQWptr->g << endl;
        // DefineStatusMessage((char*)lang?"resultat stocke dans last":"result stored in last", 1, 0, 0);
        // DisplayStatusArea();
        giac::sto(value._EQWptr->g, giac::gen("last", contextptr), contextptr);
        return !edited ? geq : value._EQWptr->g;
      }
      // cout << "no " << eq.data << endl; if (value.type==_EQW) cout << value._EQWptr->g << endl ;
      return geq;
    }
    if (key != KEY_CHAR_MINUS && key != KEY_CHAR_EQUAL &&
        (ins || key == KEY_CHAR_PI || key == KEY_CTRL_F5 || (addssize == 1 && (is_alphanum(adds[0]) || adds[0] == '.' || adds[0] == '-'))))
    {
      edited = true;
      if (line >= 0 && xcas::eqw_select(eq.data, line, col, true, value))
      {
        ustl::string s;
        if (ins)
        {
          if (key == KEY_CTRL_PASTE)
            s = paste_clipboard();
          else
          {
            if (value.type == _EQW)
            {
              s = value._EQWptr->g.print(contextptr);
            }
            else
              s = value.print(contextptr);
          }
        }
        else
          s = adds;
        ustl::string msg("Line ");
        msg += print_INT_(line + 1);
        msg += " Col ";
        msg += print_INT_(col + 1);
        if (inputline(msg.c_str(), 0, s, false) == KEY_CTRL_EXE)
        {
          value = gen(s, contextptr);
          if (col < 0)
            (*geq._VECTptr)[line] = value;
          else
            (*((*geq._VECTptr)[line]._VECTptr))[col] = value;
          redo = 2;
          key = KEY_SHIFT_RIGHT;
        }
        else
          continue;
      }
      else
      {
        vector<int> goto_sel;
        if (xcas::Equation_adjust_xy(eq.data, xleft, ytop, xright, ybottom, gsel, gselparent, gselpos, &goto_sel) && gsel && xcas::do_select(*gsel, true, value) && value.type == _EQW)
        {
          ustl::string s;
          if (ins)
          {
            if (key == KEY_CTRL_PASTE)
              s = paste_clipboard();
            else
            {
              s = value._EQWptr->g.print(contextptr);
              if (key == KEY_CHAR_COMMA)
                s += ',';
            }
          }
          else
            s = adds;
          if (inputline(value._EQWptr->g.print(contextptr).c_str(), 0, s, false) == KEY_CTRL_EXE)
          {
            value = gen(s, contextptr);
            // cout << value << " goto " << goto_sel << endl;
            xcas::replace_selection(eq, value, gsel, &goto_sel,contextptr);
            firstrun = -1; // workaround, force 2 times display
          }
          continue;
        }
      }
    }
    if (redo)
    {
      eq.data = 0; // clear memory
      eq.data = xcas::Equation_compute_size(geq, eq.attr, LCD_WIDTH_PX, contextptr);
      eqdata = xcas::Equation_total_size(eq.data);
      if (redo == 1)
      {
        dx = (eqdata.dx - LCD_WIDTH_PX) / 2;
        dy = LCD_HEIGHT_PX - 2 * EQW_TAILLE + eqdata.y;
        if (listormat) // select line l, col c
          xcas::eqw_select(eq.data, line, col, true, value);
        else
        {
          xcas::Equation_select(eq.data, true);
          xcas::eqw_select_down(eq.data);
        }
        continue;
      }
    }
    if (key == KEY_CTRL_EXIT || key == KEY_CTRL_AC)
    {
      if (!edited)
        return geq;
      if (confirm(lang ? "Vraiment abandonner?" : "Really leave", lang ? "F1: annul,  F6: confirmer" : "F1: cancel,  F6: confirm") == KEY_CTRL_F6)
        return undef;
    }
    bool doit = eqdata.dx >= LCD_WIDTH_PX;
    int delta = 0;
    if (keyflag == 0x84 || keyflag == 0x88 || keyflag == 0x04 || keyflag == 0x08) //!!!
    {
      if (key == KEY_CTRL_LEFT)
        key = KEY_SHIFT_LEFT;
      if (key == KEY_CTRL_RIGHT)
        key = KEY_SHIFT_RIGHT;
    }
    if (listormat)
    {
      if (key == KEY_CTRL_LEFT || (!doit && key == KEY_SHIFT_LEFT))
      {
        if (line >= 0 && xcas::eqw_select(eq.data, line, col, false, value))
        {
          if (col >= 0)
          {
            --col;
            if (col < 0)
            {
              col = ncols - 1;
              if (line > 0)
                --line;
            }
          }
          else
          {
            if (line > 0)
              --line;
          }
          xcas::eqw_select(eq.data, line, col, true, value);
          if (doit)
            dx -= value._EQWptr->dx;
        }
        continue;
      }
      if (doit && key == KEY_SHIFT_LEFT)
      {
        dx -= 20;
        continue;
      }
      if (key == KEY_CTRL_RIGHT || (!doit && key == KEY_SHIFT_RIGHT))
      {
        if (line >= 0 && xcas::eqw_select(eq.data, line, col, false, value))
        {
          if (doit)
            dx += value._EQWptr->dx;
          if (col >= 0)
          {
            ++col;
            if (col == ncols)
            {
              col = 0;
              if (line < nlines - 1)
                ++line;
            }
          }
          else
          {
            if (line < nlines - 1)
              ++line;
          }
          xcas::eqw_select(eq.data, line, col, true, value);
        }
        continue;
      }
      if (key == KEY_SHIFT_RIGHT && doit)
      {
        dx += 20;
        continue;
      }
      doit = eqdata.dy >= LCD_HEIGHT_PX - 2 * EQW_TAILLE;
      if (key == KEY_CTRL_UP || (!doit && key == KEY_CTRL_PAGEUP))
      {
        if (line > 0 && col >= 0 && xcas::eqw_select(eq.data, line, col, false, value))
        {
          --line;
          xcas::eqw_select(eq.data, line, col, true, value);
          if (doit)
            dy += value._EQWptr->dy + (eq.attr.fontsize / 2);
        }
        continue;
      }
      if (key == KEY_CTRL_PAGEUP && doit)
      {
        dy += eq.attr.fontsize + 2;
        continue;
      }
      if (key == KEY_CTRL_DOWN || (!doit && key == KEY_CTRL_PAGEDOWN))
      {
        if (line < nlines - 1 && col >= 0 && xcas::eqw_select(eq.data, line, col, false, value))
        {
          if (doit)
            dy -= value._EQWptr->dy + (eq.attr.fontsize / 2);
          ++line;
          xcas::eqw_select(eq.data, line, col, true, value);
        }
        continue;
      }
      if (key == KEY_CTRL_PAGEDOWN && doit)
      {
        dy -= eq.attr.fontsize + 2;
        continue;
      }
    }
    else
    { // else listormat
      if (key == KEY_CTRL_LEFT)
      {
        delta = xcas::eqw_select_leftright(eq, true, alph ? 2 : 0);
        // cout << "left " << delta << endl;
        if (doit)
          dx += (delta ? delta : -20);
        continue;
      }
      if (key == KEY_SHIFT_LEFT)
      {
        delta = xcas::eqw_select_leftright(eq, true, 1);
        vector<int> goto_sel;
        if (doit)
          dx += (delta ? delta : -20);
        continue;
      }
      if (key == KEY_CTRL_RIGHT)
      {
        delta = xcas::eqw_select_leftright(eq, false, alph ? 2 : 0);
        // cout << "right " << delta << endl;
        if (doit)
          dx += (delta ? delta : 20);
        continue;
      }
      if (key == KEY_SHIFT_RIGHT)
      {
        delta = xcas::eqw_select_leftright(eq, false, 1);
        // cout << "right " << delta << endl;
        if (doit)
          dx += (delta ? delta : 20);
        // dx=eqdata.dx-LCD_WIDTH_PX+20;
        continue;
      }
      doit = eqdata.dy >= LCD_HEIGHT_PX - 2 * EQW_TAILLE;
      if (key == KEY_CTRL_UP)
      {
        delta = xcas::eqw_select_up(eq.data);
        // cout << "up " << delta << endl;
        continue;
      }
      // cout << "up " << eq.data << endl;
      if (key == KEY_CTRL_PAGEUP && doit)
      {
        dy = eqdata.y + eqdata.dy + 20;
        continue;
      }
      if (key == KEY_CTRL_DOWN)
      {
        delta = xcas::eqw_select_down(eq.data);
        // cout << "down " << delta << endl;
        continue;
      }
      // cout << "down " << eq.data << endl;
      if (key == KEY_CTRL_PAGEDOWN && doit)
      {
        dy = eqdata.y + LCD_HEIGHT_PX - EQW_TAILLE;
        continue;
      }
    }
    if (adds)
    {
      edited = true;
      if (strcmp(adds, "'") == 0)
        adds = "diff";
      if (strcmp(adds, "^2") == 0)
        adds = "sq";
      if (strcmp(adds, ">") == 0)
        adds = "simplify";
      if (strcmp(adds, "<") == 0)
        adds = "factor";
      if (strcmp(adds, "#") == 0)
        adds = "partfrac";
      string cmd(adds);
      if (cmd.size() && cmd[cmd.size() - 1] == '(')
        cmd = '\'' + cmd.substr(0, cmd.size() - 1) + '\'';
      vector<int> goto_sel;
      if (xcas::Equation_adjust_xy(eq.data, xleft, ytop, xright, ybottom, gsel, gselparent, gselpos, &goto_sel) && gsel)
      {
        gen op;
        int addarg = 0;
        if (addssize == 1)
        {
          switch (adds[0])
          {
          case '+':
            addarg = 1;
            op = at_plus;
            break;
          case '^':
            addarg = 1;
            op = at_pow;
            break;
          case '=':
            addarg = 1;
            op = at_equal;
            break;
          case '-':
            addarg = 1;
            op = at_binary_minus;
            break;
          case '*':
            addarg = 1;
            op = at_prod;
            break;
          case '/':
            addarg = 1;
            op = at_division;
            break;
          case '\'':
            addarg = 1;
            op = at_diff;
            break;
          }
        }
        if (op == 0)
          op = gen(cmd, contextptr);
        if (op.type == _SYMB)
          op = op._SYMBptr->sommet;
        // cout << "keyed " << adds << " " << op << " " << op.type << endl;
        if (op.type == _FUNC)
        {
          edited = true;
          // execute command on selection
          gen tmp, value;
          if (xcas::do_select(*gsel, true, value) && value.type == _EQW)
          {
            if (op == at_integrate || op == at_sum)
              addarg = 3;
            if (op == at_limit)
              addarg = 2;
            gen args = eval(value._EQWptr->g, 1, contextptr);
            gen vx = x__IDNT_e; // xthetat?t__IDNT_e:x__IDNT_e;
            if (addarg == 1)
              args = makesequence(args, 0);
            if (addarg == 2)
              args = makesequence(args, vx_var, 0);
            if (addarg == 3)
              args = makesequence(args, vx_var, 0, 1);
            if (op == at_surd)
              args = makesequence(args, key == KEY_CHAR_CUBEROOT ? 3 : 4);
            if (op == at_subst)
              args = makesequence(args, giac::symb_equal(vx_var, 0));
            unary_function_ptr immediate_op[] = {*at_eval, *at_evalf, *at_evalc, *at_regrouper, *at_simplify, *at_normal, *at_ratnormal, *at_factor, *at_cfactor, *at_partfrac, *at_cpartfrac, *at_expand, *at_canonical_form, *at_exp2trig, *at_trig2exp, *at_sincos, *at_lin, *at_tlin, *at_tcollect, *at_texpand, *at_trigexpand, *at_trigcos, *at_trigsin, *at_trigtan, *at_halftan, *at_approx, *at_exact};
            if (equalposcomp(immediate_op, *op._FUNCptr))
            {
              set_abort();
              tmp = (*op._FUNCptr)(args, contextptr);
              clear_abort();
              esc_flag = 0;
              ctrl_c = false;
              giac::kbd_interrupted = interrupted = false;
            }
            else
              tmp = symbolic(*op._FUNCptr, args);
            // cout << "sel " << value._EQWptr->g << " " << tmp << " " << goto_sel << endl;
            esc_flag = 0;
            ctrl_c = false;
            giac::kbd_interrupted = interrupted = false;
            if (!is_undef(tmp))
            {
              xcas::replace_selection(eq, tmp, gsel, &goto_sel,contextptr);
              if (addarg)
              {
                xcas::eqw_select_down(eq.data);
                xcas::eqw_select_leftright(eq, false,contextptr);
              }
              eqdata = xcas::Equation_total_size(eq.data);
              dx = (eqdata.dx - LCD_WIDTH_PX) / 2;
              dy = LCD_HEIGHT_PX - 2 * EQW_TAILLE + eqdata.y;
              firstrun = -1; // workaround, force 2 times display
            }
          }
        }
      }
    }
  }
  //*logptr(contextptr) << eq.data << endl;
}

bool textedit(char *s)
{
  if (!s)
    return false;
  int ss = strlen(s);
  if (ss == 0)
  {
    *s = ' ';
    s[1] = 0;
    ss = 1;
  }
  textArea ta;
  ta.elements.clear();
  ta.editable = true;
  ta.clipline = -1;
  ta.changed = false;
  ta.filename = "\\\\fls0\\temp.py";
  ta.y = 12; // !!!!
  ta.lineHeight=16;
  ta.allowEXE = true;
  bool str = s[0] == '"' && s[ss - 1] == '"';
  if (str)
  {
    s[ss - 1] = 0;
    add(&ta, s + 1);
  }
  else
    add(&ta, s);
  ta.line = 0;
  ta.pos = ta.elements[ta.line].s.size();
  int res = doTextArea(&ta);
  if (res == TEXTAREA_RETURN_EXIT)
    return false;
  string S(merge_area(ta.elements));
  if (str)
    S = '"' + S + '"';
  int Ssize = S.size();
  if (Ssize < 512)
  {
    strcpy(s, S.c_str());
    for (--Ssize; Ssize >= 0; --Ssize)
    {
      if ((unsigned char)s[Ssize] == 0x9c || s[Ssize] == '\n')
        s[Ssize] = 0;
      if (s[Ssize] != ' ')
        break;
    }
    return true;
  }
  return false;
}

bool eqws(char *s, bool eval)
{ // s buffer must be at least 512 char
  gen g, ge;
  int dconsole_save = dconsole_mode;
  int ss = strlen(s);
  for (int i = 0; i < ss; ++i)
  {
    if (s[i] == char(0x9c))
      s[i] = '\n';
  }
  if (ss >= 2 && (s[0] == '#' || s[0] == '"' ||
                  (s[0] == '/' && (s[1] == '/' || s[1] == '*'))))
    return textedit(s);
  dconsole_mode = 0;
  if (eval)
    do_run(s, g, ge);
  else
  {
    if (s[0] == 0)
      ge = 0;
    else
      ge = gen(s, contextptr);
  }
  dconsole_mode = dconsole_save;
  if (is_undef(ge))
    return textedit(s);
  if (ge.type == giac::_SYMB || (ge.type == giac::_VECT && !ge._VECTptr->empty() && !is_numericv(*ge._VECTptr)))
  {
    if (islogo(ge))
    {
      displaylogo();
      return false;
    }
    if (ispnt(ge))
    {
      displaygraph(ge);
      // aborttimer = Timer_Install(0, check_execution_abort, 100); if (aborttimer > 0) { Timer_Start(aborttimer); }
      return false;
    }
    if (ge.is_symb_of_sommet(at_program))
      return textedit(s);
    if (taille(ge, 256) >= 256)
      return false; // sizeof(eqwdata)=44
  }
  gen tmp = eqw(ge, true);
  if (is_undef(tmp) || tmp == ge || taille(ge, 64) >= 64)
    return false;
  string S(tmp.print(contextptr));
  if (S.size() >= 512)
    return false;
  strcpy(s, S.c_str());
  return true;
}

#endif

bool stringtodouble(const string &s1, double &d)
{
  gen g(s1, contextptr);
  g = evalf(g, 1, contextptr);
  if (g.type != _DOUBLE_)
  {
    confirm("Invalid value", s1.c_str());
    return false;
  }
  d = g._DOUBLE_val;
  return true;
}

void displaygraph(const giac::gen & ge){
  // graph display
  //if (aborttimer > 0) { Timer_Stop(aborttimer); Timer_Deinstall(aborttimer);}
  xcas::Graph2d gr(ge,contextptr);
  gr.show_axes=true;
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
	      }
	    }
	  }
	}
      }
    }
  }
  gr.init_tracemode();
  if (gr.tracemode & 4)
    gr.orthonormalize();
  // UI
  for (;;){
    gr.draw();
    PrintMini(1,114,(const unsigned char*)"menu",MINI_REV);
    unsigned int key;
    int keyflag = (unsigned char)Setup_GetEntry(0x14);
    bool alph=keyflag==4||keyflag==0x84||keyflag==8||keyflag==0x88;
    ck_getkey((int*)&key);
    if (key==KEY_CTRL_F1 || key==KEY_CTRL_F6){
      char menu_xmin[32],menu_xmax[32],menu_ymin[32],menu_ymax[32];
      ustl::string s;
      s="xmin "+print_DOUBLE_(gr.window_xmin,6);
      strcpy(menu_xmin,s.c_str());
      s="xmax "+print_DOUBLE_(gr.window_xmax,6);
      strcpy(menu_xmax,s.c_str());
      s="ymin "+print_DOUBLE_(gr.window_ymin,6);
      strcpy(menu_ymin,s.c_str());
      s="ymax "+print_DOUBLE_(gr.window_ymax,6);
      strcpy(menu_ymax,s.c_str());
      Menu smallmenu;
      smallmenu.numitems=15;
      MenuItem smallmenuitems[smallmenu.numitems];
      smallmenu.items=smallmenuitems;
      smallmenu.height=8;
      //smallmenu.title = "KhiCAS";
      smallmenuitems[0].text = (char *) (lang?"Etude graphe (xtt)":"Curve study (xtt)");
      smallmenuitems[1].text = (char *) menu_xmin;
      smallmenuitems[2].text = (char *) menu_xmax;
      smallmenuitems[3].text = (char *) menu_ymin;
      smallmenuitems[4].text = (char *) menu_ymax;
      smallmenuitems[5].text = (char*) "Orthonormalize /";
      smallmenuitems[6].text = (char*) "Autoscale *";
      smallmenuitems[7].text = (char *) ("Zoom in +");
      smallmenuitems[8].text = (char *) ("Zoom out -");
      smallmenuitems[9].text = (char *) ("Y-Zoom out (-)");
      smallmenuitems[10].text = (char*) ((lang==1)?"Voir axes":"Show axes");
      smallmenuitems[10].type = MENUITEM_CHECKBOX;
      smallmenuitems[10].value = gr.show_axes;
      smallmenuitems[11].text = (char*) ((lang==1)?"Voir tangent (F3)":"Show tangent (F3)");
      smallmenuitems[11].type = MENUITEM_CHECKBOX;
      smallmenuitems[11].value = (gr.tracemode & 2)!=0;
      smallmenuitems[12].text = (char*) ((lang==1)?"Voir normal (F4)":"Show normal (F4)");
      smallmenuitems[12].type = MENUITEM_CHECKBOX;
      smallmenuitems[12].value = (gr.tracemode & 4)!=0;
      smallmenuitems[13].text = (char*) ((lang==1)?"Voir cercle (F5)":"Show circle (F5)");
      smallmenuitems[13].type = MENUITEM_CHECKBOX;
      smallmenuitems[13].value = (gr.tracemode & 8)!=0;
      smallmenuitems[14].text = (char*)(lang?"Quitter":"Quit");
      int sres = doMenu(&smallmenu);
      if(sres == MENU_RETURN_SELECTION) {
	const char * ptr=0;
	ustl::string s1; double d;
	if (smallmenu.selection==1){
	  gr.curve_infos();
	  continue;
	}
	if (smallmenu.selection==2){
	  inputline(menu_xmin,lang?"Nouvelle valeur?":"New value?",s1,false); /* numeric value expected */
	  if (stringtodouble(s1,d)){
	    gr.window_xmin=d;
	    gr.update();
	  }
	}
	if (smallmenu.selection==3){
	  inputline(menu_xmax,lang?"Nouvelle valeur?":"New value?",s1,false); /* numeric value expected */
	  if (stringtodouble(s1,d)){
	    gr.window_xmax=d;
	    gr.update();
	  }
	}
	if (smallmenu.selection==4){
	  inputline(menu_ymin,lang?"Nouvelle valeur?":"New value?",s1,false); /* numeric value expected */
	  if (stringtodouble(s1,d)){
	    gr.window_ymin=d;
	    gr.update();
	  }
	}
	if (smallmenu.selection==5){
	  inputline(menu_ymax,lang?"Nouvelle valeur?":"New value?",s1,false); /* numeric value expected */
	  if (stringtodouble(s1,d)){
	    gr.window_ymax=d;
	    gr.update();
	  }
	}
	if (smallmenu.selection==6)
	  gr.orthonormalize();
	if (smallmenu.selection==7)
	  gr.autoscale();	
	if (smallmenu.selection==8)
	  gr.zoom(0.7);	
	if (smallmenu.selection==9)
	  gr.zoom(1/0.7);	
	if (smallmenu.selection==10)
	  gr.zoomy(1/0.7);
	if (smallmenu.selection==11)
	  gr.show_axes=!gr.show_axes;	
	if (smallmenu.selection==12){
	  if (gr.tracemode & 2)
	    gr.tracemode &= ~2;
	  else
	    gr.tracemode |= 2;
	  gr.tracemode_set();
	}
	if (smallmenu.selection==13){
	  if (gr.tracemode & 4)
	    gr.tracemode &= ~4;
	  else {
	    gr.tracemode |= 4;
	    gr.orthonormalize();
	  }
	  gr.tracemode_set();
	}
	if (smallmenu.selection==14){
	  if (gr.tracemode & 8)
	    gr.tracemode &= ~8;
	  else {
	    gr.tracemode |= 8;
	    gr.orthonormalize();
	  }
	  gr.tracemode_set();
	}
	if (smallmenu.selection==15)
	  break;
      }
    }
    if (key==KEY_CTRL_EXIT || key==KEY_CTRL_AC || key==KEY_CTRL_MENU)
      break;
    if (key==KEY_CTRL_F2){
      gr.tracemode_set(-1); // object info
      continue;
    }
    if (key==KEY_CTRL_F3){
      if (gr.tracemode & 2)
	gr.tracemode &= ~2;
      else
	gr.tracemode |= 2;
      gr.tracemode_set();
      continue;
    }
    if (key==KEY_CTRL_F4){
      if (gr.tracemode & 4)
	gr.tracemode &= ~4;
      else
	gr.tracemode |= 4;
      gr.tracemode_set();
      continue;
    }
    if (key==KEY_CTRL_F5){
      if (gr.tracemode & 8)
	gr.tracemode &= ~8;
      else {
	gr.tracemode |= 8;
	gr.orthonormalize();
      }
      gr.tracemode_set();
      continue;
    }
    if (key==KEY_CTRL_XTT || key=='\t'){
      gr.curve_infos();
      continue;
    }
    if (key==KEY_CTRL_UP){
      if (gr.tracemode && !alph){
	--gr.tracemode_n;
	gr.tracemode_set();
	continue;
      }
      gr.up((gr.window_ymax-gr.window_ymin)/5);
    }
    if (key==KEY_CTRL_PAGEUP) { gr.up((gr.window_ymax-gr.window_ymin)/2); }
    if (key==KEY_CTRL_DOWN) {
      if (gr.tracemode && !alph){
	++gr.tracemode_n;
	gr.tracemode_set();
	continue;
      }
      gr.down((gr.window_ymax-gr.window_ymin)/5);
    }
    if (key==KEY_CTRL_PAGEDOWN) { gr.down((gr.window_ymax-gr.window_ymin)/2);}
    if (key==KEY_CTRL_LEFT) {
      if (gr.tracemode && !alph){
	if (gr.tracemode_i!=int(gr.tracemode_i))
	  gr.tracemode_i=std::floor(gr.tracemode_i);
	else
	  --gr.tracemode_i;
	gr.tracemode_set();
	continue;
      }
      gr.left((gr.window_xmax-gr.window_xmin)/5);
    }
    if (key==KEY_CTRL_RIGHT) {
      if (gr.tracemode && !alph){
	if (int(gr.tracemode_i)!=gr.tracemode_i)
	  gr.tracemode_i=std::ceil(gr.tracemode_i);
	else
	  ++gr.tracemode_i;
	gr.tracemode_set();
	continue;
      }
      gr.right((gr.window_xmax-gr.window_xmin)/5);
    }
    if (key==KEY_CHAR_PLUS) { gr.zoom(0.7);}
    if (key==KEY_CHAR_MINUS){ gr.zoom(1/0.7); }
    if (key==KEY_CHAR_PMINUS){ gr.zoomy(1/0.7); }
    if (key==KEY_CHAR_MULT){ gr.autoscale(); }
    if (key==KEY_CHAR_DIV) { gr.orthonormalize(); }
    if (key==KEY_CTRL_VARS || key==KEY_CTRL_OPTN) {gr.show_axes=!gr.show_axes;}
  }
}  

void check_do_graph(giac::gen &ge, int do_logo_graph_eqw)
{
  if (ge.type == giac::_SYMB || (ge.type == giac::_VECT && !ge._VECTptr->empty() && !is_numericv(*ge._VECTptr)))
  {
    if (islogo(ge))
    {
      if (do_logo_graph_eqw & 4)
        displaylogo();
      return;
    }
    if (ispnt(ge))
    {
      if (do_logo_graph_eqw & 2)
        displaygraph(ge);
      // aborttimer = Timer_Install(0, check_execution_abort, 100); if (aborttimer > 0) { Timer_Start(aborttimer); }
      return;
    }
    if (do_logo_graph_eqw % 2 == 0)
      return;
    if (taille(ge, 144) >= 144 || ge.is_symb_of_sommet(at_program))
      return; // sizeof(eqwdata)=44
    gen tmp = eqw(ge, false);
    if (!is_undef(tmp) && tmp != ge)
    {
      // dConsolePutChar(147);
      Console_Output((const unsigned char *)ge.print(contextptr).c_str());
      Console_NewLine(LINE_TYPE_INPUT, 1);
      ge = tmp;
    }
  }
}

void do_run(const char *s, gen &g, gen &ge)
{
  if (!contextptr)
    contextptr = new giac::context;
  int S = strlen(s);
  char buf[S + 1];
  buf[S] = 0;
  for (int i = 0; i < S; ++i)
  {
    char c = s[i];
    if (c == 0x1e || c == char(0x9c))
      buf[i] = '\n';
    else
    {
      if (c == 0x0d)
        buf[i] = ' ';
      else
        buf[i] = c;
    }
  }
  g = gen(buf, contextptr);
  // Console_Output(g.print(contextptr).c_str()); return ;
  giac::freeze = false;
  execution_in_progress = 1;
  giac::set_abort();
  ge = eval(equaltosto(g, contextptr), 1, contextptr);
  giac::clear_abort();
  execution_in_progress = 0;
  if (esc_flag || ctrl_c)
  {
    while (confirm("Interrupted", "F1/F6: ok", true) == -1)
      ; // insure ON has been removed from keyboard buffer
    ge = string2gen("Interrupted", false);
    // memory full?
    if (!kbd_interrupted)
    {
      // clear turtle, display msg
      turtle_stack() = vector<logo_turtle>(1, logo_turtle());
      while (confirm((lang ? "Memoire remplie!" : "Memory full"), "Purge variable", true) == -1)
        ;
      gen g = select_var(contextptr);
      if (g.type == _IDNT)
        _purge(g, contextptr);
      else
        _restart(0, contextptr);
    }
  }
  // Console_Output("Done"); return ;
  esc_flag = 0;
  ctrl_c = false;
  giac::kbd_interrupted = interrupted = false;
}

void run(const char *s, int do_logo_graph_eqw)
{
  if (strlen(s) >= 2 && (s[0] == '#' ||
                         (s[0] == '/' && (s[1] == '/' || s[1] == '*'))))
    return;
  gen g, ge;
  do_run(s, g, ge);
  if (giac::freeze)
  {
    giac::freeze = false;
    // DefineStatusMessage((char*)lang?"Taper sur une touche":"Screen freezed. Press any key.", 1, 0, 0);
    // DisplayStatusArea();
    // int keyflag = GetSetupSetting( (unsigned int)0x14);
    unsigned int key;
    ck_getkey((int *)&key);
  }
  int t = giac::taille(g, GIAC_HISTORY_MAX_TAILLE);
  int te = giac::taille(ge, GIAC_HISTORY_MAX_TAILLE);
  bool do_tex = false;
  if (t < GIAC_HISTORY_MAX_TAILLE && te < GIAC_HISTORY_MAX_TAILLE)
  {
    giac::vecteur &vin = history_in(contextptr);
    giac::vecteur &vout = history_out(contextptr);
    if (vin.size() > GIAC_HISTORY_SIZE)
      vin.erase(vin.begin());
    vin.push_back(g);
    if (vout.size() > GIAC_HISTORY_SIZE)
      vout.erase(vout.begin());
    vout.push_back(ge);
  }
  check_do_graph(ge, do_logo_graph_eqw);
  string s_;
  if (ge.type == giac::_STRNG)
    s_ = '"' + *ge._STRNGptr + '"';
  else
  {
    if (te > 256)
      s_ = "Object too large";
    else
    {
      if (ge.is_symb_of_sommet(giac::at_pnt) || (ge.type == giac::_VECT && !ge._VECTptr->empty() && ge._VECTptr->back().is_symb_of_sommet(giac::at_pnt)))
        s_ = "Graphic object";
      else
      {
        // do_tex=ge.type==giac::_SYMB && has_op(ge,*giac::at_inv);
        //  tex support has been disabled!
        s_ = ge.print(contextptr);
        // translate to tex? set do_tex to true
      }
    }
  }
  if (s_.size() > 512)
    s_ = s_.substr(0, 509) + "...";
  char *edit_line = (char *)Console_GetEditLine();
  Console_Output((const unsigned char *)s_.c_str());
#ifdef TEX
  if (do_tex)
  {
    int old_tex_flag = tex_flag;
    int width, height, baseline;
    TeX_sizeComplex(edit_line, &width, &height, &baseline);
    /*if(width > MAX_TEX_WIDTH || height > MAX_TEX_HEIGHT) {
      tex_flag = 0;
      Console_Clear_EditLine();
      print_expr(p);
      }*/
    tex_flag = old_tex_flag;
  }
#endif
  // return ge;
}

void stop(const char *s)
{
  printf(s);
}

#ifdef GINT
#define VAR_HEAP
#ifdef VAR_HEAP
__attribute__((aligned(4))) char malloc_heap[110 * 1024];
#else
/* Unused part of user stack; provided by linker script */
extern char sextra, eextra;
#endif
#endif

int kcas_main(int isAppli, unsigned short OptionNum)
{
  confirm("khicas main start");
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
  unsigned char *expr;
  unsigned char *user_functions;

  int i = 0, j = 0;

  unsigned char *line = (unsigned char *)malloc(USER_FUNCTIONS_MAX_LENGTH * sizeof(unsigned char));

  SetQuitHandler(save_session); // automatically save session when exiting

  confirm("khicas turtle init");
  turtle();
#ifdef TURTLETAB
  turtle_stack_size = 0;
#else
  turtle_stack(); // required to init turtle
#endif

  confirm("khicas console init");
  Console_Init();
  context ct;
  contextptr = &ct;
  _srand(vecteur(0), contextptr);
  confirm("khicas session");
  restore_session("session");
  // load_config();
  Console_Disp();
  //init_locale();
  lang = 0;

  i = 0;


  while (1)
  {
    
    if ((expr = Console_GetLine()) == NULL)
      stop("memory error");
    if (strcmp((const char *)expr, "restart") == 0)
    {
      if (confirm(lang ? "Effacer variables?" : "Clear variables?", lang ? "F1: annul,  F6: confirmer" : "F1: cancel,  F6: confirm") != KEY_CTRL_F6)
      {
        Console_Output((const unsigned char *)" cancelled");
        Console_NewLine(LINE_TYPE_OUTPUT, 1);
        // ck_getkey((int *)&key);
        Console_Disp();
        continue;
      }
    }
    // should save in another file
    if (strcmp((const char *)expr, "=>") == 0 || strcmp((const char *)expr, "=>\n") == 0)
    {
      save_session();
      Console_Output((unsigned char *)"Session saved");
    }
    else
      run((char *)expr);
    // print_mem_info();
    Console_NewLine(LINE_TYPE_OUTPUT, 1);
    // ck_getkey((int *)&key);
    Console_Disp();
  }
#ifdef TEX
  TeX_quit();
#endif
  for (;;)
    ck_getkey((int *)&key);
  return 1;
}

#pragma section _BR_Size
unsigned long BR_Size;
#pragma section

#if 0
#pragma section _TOP
int InitializeSystem(int isAppli, unsigned short OptionNum)
{
  return INIT_ADDIN_APPLICATION(isAppli, OptionNum);
}
#pragma section
#endif
