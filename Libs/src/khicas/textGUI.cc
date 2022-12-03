#include "giacPCH.h"
//!!!!
//#include "fxlib.h"
#include "porting.h"
#include "syscalls.h"
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include "menuGUI.h"
#include "textGUI.h"
#include "console.h"
#include "file.h"
#include "main.h"

// typedef scrollbar TScrollbar;
textArea *edptr = 0;
void displaylogo();
extern giac::context *contextptr;
#define C24 16 
#define C19 16 // 17?
#define C154 96

bool is_alphanum(char c)
{
  return isalpha(c) || (c >= '0' && c <= '9');
}

int get_line_number(const char *msg1, const char *msg2);

int get_filename(char *filename, const char *extension)
{
  giac::handle_f5();
  ustl::string str;
  int res = giac::inputline(giac::lang ? "EXIT/chaine vide: annulation" : "EXIT or empty string: cancel", giac::lang ? "Nom de fichier:" : "Filename:", str, false);
  if (res == KEY_CTRL_EXIT || str.empty())
    return 0;
  strcpy(filename, "\\\\fls0\\");
  strcpy(filename + 7, str.c_str());
  int s = strlen(filename);
  if (strcmp(filename + s - 3, extension))
    strcpy(filename + s, extension);
  // if file already exists, warn, otherwise create
  unsigned short pFile[MAX_FILENAME_SIZE + 1];
  Bfile_StrToName_ncpy(pFile, (const unsigned char *)filename, strlen(filename) + 1);
  int hFile = Bfile_OpenFile_OS(pFile, _OPENMODE_READWRITE); // Get handle
  if (hFile < 0)
  {
    save_script(filename, "");
    return 1;
    int size = 1, BCEres = Bfile_CreateFile(pFile, size);
    BCEres = Bfile_OpenFile_OS(pFile, _OPENMODE_READWRITE); // Get handle
    // cout << "create " << filename << " " << BCEres << endl;
    if (BCEres < 0)
      return 0;
    return 1;
  }
  Bfile_CloseFile_OS(hFile);
  if (giac::confirm(giac::lang ? "     Le fichier existe!" : "     File exists!", giac::lang ? "F1: ecraser,  F6: annuler" : "F1:overwrite,  F6: cancel") == KEY_CTRL_F1)
    return 1;
  return 0;
}

void clearLine(int x, int y, bool minimini)
{
  // clear text line. x and y are text cursor coordinates
  // this is meant to achieve the same effect as using PrintXY with a line full of spaces (except it doesn't waste strings).
  int X = (minimini ? 6 : 8) * (x - 1);
  int width = LCD_WIDTH_PX - X;
  giac::drawRectangle(X, (y - 1) * C24, width, C24, COLOR_WHITE);
}

void drawScreenTitle(char *title)
{
  if (title != NULL)
    PrintXY(0, 0, (const unsigned char *)title, 1);
}

char tolower(char c)
{
  if (c >= 'A' && c <= 'Z')
    c += 32;
  return c;
}
char toupper(char c)
{
  if (c >= 'a' && c <= 'z')
    c -= 32;
  return c;
}
int strncasecmp_duplicate(const char *s1, const char *s2, size_t n)
{
  if (n != 0)
  {
    const unsigned char *us1 = (const unsigned char *)s1;
    const unsigned char *us2 = (const unsigned char *)s2;

    do
    {
      if (tolower(*us1) != tolower(*us2++))
        return (tolower(*us1) - tolower(*--us2));
      if (*us1++ == '\0')
        break;
    } while (--n != 0);
  }
  return (0);
}
char *strcasestr_duplicate(const char *s, const char *find)
{
  char c;

  if ((c = *find++) != 0)
  {
    c = tolower((unsigned char)c);
    size_t len = strlen(find);
    do
    {
      char sc;
      do
      {
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
                              char tokchar,             /* token delimiting char */
                              unsigned char *token,     /* receiver of parsed token */
                              int lgh)                  /* length token can receive */
/* not including final '\0' */
{
  if (src)
  {
    while (*src && (tokchar != *src))
    {
      if (lgh)
      {
        *token++ = *src;
        --lgh;
      }
      src++;
    }
    if (*src && (tokchar == *src))
      src++;
  }
  *token = '\0';
  return src;
} /* toksplit */

int EndsIWith(const char *str, const char *suffix)
{
  if (!str || !suffix)
    return 0;
  size_t lenstr = strlen(str);
  size_t lensuffix = strlen(suffix);
  if (lensuffix > lenstr)
    return 0;
  // return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
  return strncasecmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

// not really for strings, but anyway:
// based on http://dsss.be/w/c:memmem
// added case-insensitive functionality
void *memmem(char *haystack, int hlen, char *needle, int nlen, int matchCase)
{
  if (nlen > hlen)
    return 0;
  int i, j = 0;
  switch (nlen)
  {       // we have a few specialized compares for certain needle sizes
  case 0: // no needle? just give the haystack
    return haystack;
  case 1: // just use memchr for 1-byte needle
    if (matchCase)
      return memchr(haystack, needle[0], hlen);
    else
    {
      void *lc = memchr(haystack, tolower(needle[0]), hlen);
      if (lc != NULL)
        return lc;
      else
        return memchr(haystack, toupper(needle[0]), hlen);
    }
  default: // generic compare for any other needle size
    // walk i through the haystack, matching j as long as needle[j] matches haystack[i]
    for (i = 0; i < hlen - nlen + 1; i++)
    {
      if (matchCase ? haystack[i] == needle[j] : tolower(haystack[i]) == tolower(needle[j]))
      {
        if (j == nlen - 1)
        { // end of needle and it all matched?  win.
          return haystack + i - j;
        }
        else
        { // keep advancing j (and i, implicitly)
          j++;
        }
      }
      else
      { // no match, rewind i the length of the failed match (j), and reset j
        i -= j;
        j = 0;
      }
    }
  }
  return NULL;
}

// convert a normal text string into a multibyte one where letters become their mini variants (F5 screen of the OS's character select dialog)
// dest must be at least double the size of orig.
void stringToMini(char *dest, char *orig)
{
  int len = strlen(orig);
  int dlen = 0;
  for (int i = 0; i < len; i++)
  {
    if ((orig[i] >= 65 && orig[i] <= 90) || (orig[i] >= 97 && orig[i] <= 122))
    { // A-Z a-z
      dest[dlen] = '\xe7';
      dlen++;
      dest[dlen] = orig[i];
    }
    else if ((orig[i] >= 48 && orig[i] <= 57))
    { // 0-9
      dest[dlen] = '\xe5';
      dlen++;
      dest[dlen] = orig[i] - 48 + 208;
    }
    else if (orig[i] == '+')
    {
      dest[dlen] = '\xe5';
      dlen++;
      dest[dlen] = '\xdb';
    }
    else
      dest[dlen] = orig[i];
    dlen++;
  }
  dest[dlen] = '\0';
}

void warning(const char *s)
{
  giac::draw_rectangle(0, 0, LCD_WIDTH_PX, 12, COLOR_WHITE);
  PrintMini(0, 0, (const unsigned char *)s, 0);
}

// called from editor, return
int check_parse(const ustl::vector<textElement> &v, int python)
{
  if (v.empty())
    return 0;
  ustl::string s = merge_area(v);
  giac::python_compat(python, contextptr);
  if (python)
    s = "@@" + s; // force Python translation
  giac::gen g(s, contextptr);
  int lineerr = giac::first_error_line(contextptr);
  if (lineerr)
  {
    char status[256];
    for (int i = 0; i < sizeof(status); ++i)
      status[i] = 0;
    ustl::string tok = giac::error_token_name(contextptr);
    int pos = -1;
    if (lineerr >= 1 && lineerr <= v.size())
    {
      pos = v[lineerr - 1].s.find(tok);
      const ustl::string &err = v[lineerr - 1].s;
      if (pos >= err.size())
        pos = -1;
      if (python)
      {
        // find 1st token, check if it's def/if/elseif/for/while
        size_t i = 0, j = 0;
        for (; i < err.size(); ++i)
        {
          if (err[i] != ' ')
            break;
        }
        ustl::string firsterr;
        for (j = i; j < err.size(); ++j)
        {
          if (!isalpha(err[j]))
            break;
          firsterr += err[j];
        }
        // if there is no : at end set pos=-2
        if (firsterr == "for" || firsterr == "def" || firsterr == "if" || firsterr == "elseif" || firsterr == "while")
        {
          for (i = err.size() - 1; i > 0; --i)
          {
            if (err[i] != ' ')
              break;
          }
          if (err[i] != ':')
            pos = -2;
        }
      }
    }
    else
    {
      lineerr = v.size();
      tok = giac::lang ? "la fin" : "end";
      pos = 0;
    }
    if (pos >= 0)
      sprintf(status, giac::lang ? "Erreur ligne %i a %s" : "Error line %i at %s", lineerr, tok.c_str());
    else
      sprintf(status, giac::lang ? "Erreur ligne %i %s" : "Error line %i %s", lineerr, (pos == -2 ? (giac::lang ? ", : manquant ?" : ", missing :?") : ""));
    warning(status);
  }
  else
  {
    giac::set_abort();
    g = eval(g, 1, contextptr);
    giac::clear_abort();
    ctrl_c = false;
    giac::kbd_interrupted = interrupted = false;
    check_do_graph(g); // define the function
    warning(giac::lang ? "Syntaxe correcte" : "Parse OK");
  }
  return lineerr;
}

void fix_newlines(textArea * edptr){
  edptr->elements[0].newLine=0;
  for (size_t i=1;i<edptr->elements.size();++i)
    edptr->elements[i].newLine=1;
  for (size_t i=0;i<edptr->elements.size();++i){
    string S=edptr->elements[i].s;
    if (S.size()>96)
      edptr->minimini=1;
    const int cut=160;
    if (edptr->longlinescut && S.size()>cut){
      // string too long, cut it, set font to minimini
      int j;
      for (j=(4*cut)/5;j>=(2*cut)/5;--j){
	if (!is_alphanum(S[j]))
	  break;
      }
      textElement elem; elem.newLine=1; elem.s=S.substr(j,S.size()-j);
      edptr->elements[i].s=S.substr(0,j);
      edptr->elements.insert(edptr->elements.begin()+i+1,elem);
    }
  }
  
}

int end_do_then(const ustl::string &s)
{
  // skip spaces from end
  int l = s.size(), i, i0;
  const char *ptr = s.c_str();
  for (i = l - 1; i > 0; --i)
  {
    if (ptr[i] != ' ')
    {
      if (ptr[i] == ':' || ptr[i] == '{')
        return 1;
      if (ptr[i] == '}')
        return -1;
      break;
    }
  }
  if (i > 0)
  {
    for (i0 = i; i0 >= 0; --i0)
    {
      if (!giac::isalphanum(ptr[i0]) && ptr[i0] != ';' && ptr[i0] != ':')
        break;
    }
    if (i > i0 + 2)
    {
      if (ptr[i] == ';')
        --i;
      if (ptr[i] == ':')
        --i;
    }
    ustl::string keyw(ptr + i0 + 1, ptr + i + 1);
    const char *ptr = keyw.c_str();
    if (strcmp(ptr, "faire") == 0 || strcmp(ptr, "do") == 0 || strcmp(ptr, "alors") == 0 || strcmp(ptr, "then") == 0)
      return 1;
    if (strcmp(ptr, "fsi") == 0 || strcmp(ptr, "end") == 0 || strcmp(ptr, "fi") == 0 || strcmp(ptr, "od") == 0 || strcmp(ptr, "ftantque") == 0 || strcmp(ptr, "fpour") == 0 || strcmp(ptr, "ffonction") == 0 || strcmp(ptr, "ffunction") == 0)
      return -1;
  }
  return 0;
}

void add(textArea *edptr, const ustl::string &s)
{
  int r = 1;
  for (size_t i = 0; i < s.size(); ++i)
  {
    if (s[i] == '\n' || s[i] == char(0x9c))
      ++r;
  }
  edptr->elements.reserve(edptr->elements.size() + r);
  textElement cur;
  cur.lineSpacing = 0;
  for (size_t i = 0; i < s.size(); ++i)
  {
    char c = s[i];
    if (c == char(0x9c))
      c = '\n';
    if (c != '\n')
    {
      if (c != char(0x0d))
        cur.s += c;
      continue;
    }
    string tmp = string(cur.s.begin(), cur.s.end());
    cur.s.swap(tmp);
    edptr->elements.push_back(cur);
    ++edptr->line;
    cur.s = "";
  }
  if (cur.s.size())
  {
    edptr->elements.push_back(cur);
    ++edptr->line;
  }
  fix_newlines(edptr);
}

int find_indentation(const ustl::string &s)
{
  size_t indent = 0;
  for (; indent < s.size(); ++indent)
  {
    if (s[indent] != ' ')
      break;
  }
  return indent;
}

string makestring(int n, char ch)
{
  char buf[n + 1];
  for (int i = 0; i < n; ++i)
    buf[i] = ch;
  buf[n] = 0;
  return buf;
}

void add_indented_line(ustl::vector<textElement> &v, int &textline, int &textpos)
{
  // add line
  v.insert(v.begin() + textline + 1, v[textline]);
  ustl::string &s = v[textline].s;
  int indent = find_indentation(s);
  if (!s.empty())
    indent += 2 * end_do_then(s);
  // cout << indent << s << ":" << endl;
  if (indent < 0)
    indent = 0;
  v[textline + 1].s = makestring(indent, ' ') + s.substr(textpos, s.size() - textpos);
  v[textline + 1].newLine = 1;
  v[textline].s = s.substr(0, textpos);
  ++textline;
  v[textline].nlines = 1; // will be recomputed by cursor moves
  textpos = indent;
}

void undo(textArea *text)
{
  if (text->undoelements.empty())
    return;
  giac::swapint(text->line, text->undoline);
  giac::swapint(text->pos, text->undopos);
  giac::swapint(text->clipline, text->undoclipline);
  giac::swapint(text->clippos, text->undoclippos);
  swap(text->elements, text->undoelements);
}

void set_undo(textArea *text)
{
  text->changed = true;
  text->undoelements = text->elements;
  text->undopos = text->pos;
  text->undoline = text->line;
  text->undoclippos = text->clippos;
  text->undoclipline = text->clipline;
}

void add_nl(textArea *text, const ustl::string &ins)
{
  ustl::vector<textElement> &v = text->elements;
  ustl::vector<textElement> w(v.begin() + text->line + 1, v.end());
  v.erase(v.begin() + text->line + 1, v.end());
  add(text, ins);
  for (size_t i = 0; i < w.size(); ++i)
    v.push_back(w[i]);
  fix_newlines(text);
  text->changed = true;
}

void insert(textArea *text, const char *adds, bool indent)
{
  size_t n = strlen(adds), i = 0;
  if (!n)
    return;
  set_undo(text);
  int l = text->line;
  if (l < 0 || l >= text->elements.size())
    return; // invalid line number
  ustl::string &s = text->elements[l].s;
  int ss = int(s.size());
  int &pos = text->pos;
  if (pos > ss)
    pos = ss;
  ustl::string ins = s.substr(0, pos);
  for (; i < n; ++i)
  {
    if (adds[i] == '\n' || adds[i] == 0x1e)
    {
      break;
    }
    else
    {
      if (adds[i] != char(0x0d))
        ins += adds[i];
    }
  }
  if (i == n)
  { // no newline in inserted string
    s = ins + s.substr(pos, ss - pos);
    pos += n;
    return;
  }
  ustl::string S(adds + i + 1);
  int decal = ss - pos;
  S += s.substr(pos, decal);
  // cout << S << " " << ins << endl;
  s = ins;
  if (indent)
  {
    pos = s.size();
    int debut = 0;
    for (i = 0; i < S.size(); ++i)
    {
      if (S[i] == '\n' || S[i] == 0x1e)
      {
        add_indented_line(text->elements, text->line, pos);
        // cout << S.substr(debut,i-debut) << endl;
        text->elements[text->line].s += S.substr(debut, i - debut);
        pos = text->elements[text->line].s.size();
        debut = i + 1;
      }
    }
    // cout << S << " " << debut << " " << i << S.c_str()+debut << endl;
    add_indented_line(text->elements, text->line, pos);
    text->elements[text->line].s += (S.c_str() + debut);
    fix_newlines(text);
  }
  else
    add_nl(text, S);
  pos = text->elements[text->line].s.size() - decal;
}

ustl::string merge_area(const ustl::vector<textElement> &v)
{
  ustl::string s;
  for (size_t i = 0; i < v.size(); ++i)
  {
    s += v[i].s;
    s += '\n';
  }
  return s;
}

void search_msg()
{
  warning(giac::lang ? "EXE: suivant, AC: annuler" : "EXE: next, AC: cancel");
}

void show_status(textArea *text, const ustl::string &search, const ustl::string &replace)
{
  if (text->editable && text->clipline >= 0)
    warning("PAD: select, CLIP: copy, AC: cancel");
  else
  {
    ustl::string status;
    int heure, minute;
    giac::get_time(heure, minute);
    status += char('0' + heure / 10);
    status += char('0' + (heure % 10));
    status += ':';
    status += char('0' + (minute / 10));
    status += char('0' + (minute % 10));
    // status += (xthetat?" t":" x");
    if (search.size())
    {
      status += "EXE: " + search;
      if (replace.size())
        status += "->" + replace;
    }
    else
    {
      if (text->editable)
      {
        status += text->python ? (text->python == 2 ? " Py xor " : " Py ** ") : " Xcas ";
        status += remove_path(text->filename);
        status += text->changed ? " * " : " - ";
        status += giac::printint(text->line + 1);
        status += '/';
        status += giac::printint(text->elements.size());
      }
    }
    warning(status.c_str());
  }
  char mode = Setup_GetEntry(0x14);
  if (mode == 1)
    PrintMini(100, 0, (const unsigned char *)" SHIFT  ", 0);
  if (mode == 4)
    PrintMini(100, 0, (const unsigned char *)" ALPHA  ", 0);
  if (mode == 8)
    PrintMini(100, 0, (const unsigned char *)" alpha  ", 0);
  if (mode == (char)0x84)
    PrintMini(100, 0, (const unsigned char *)" ALOCK  ", 0);
  if (mode == (char)0x88)
    PrintMini(100, 0, (const unsigned char *)" alock  ", 0);
  // DisplayStatusArea();
}

bool chk_replace(textArea *text, const ustl::string &search, const ustl::string &replace)
{
  if (replace.size())
    warning(giac::lang ? "Remplacer? EXE: Oui, 8 ou N: Non" : "Replace? EXE: Yes, 8 or N: No");
  else
    search_msg();
  for (;;)
  {
    unsigned int key;
    ck_getkey((int *)&key);
    if (key == KEY_CHAR_MINUS || key == KEY_CHAR_Y || key == KEY_CHAR_9 || key == KEY_CHAR_O || key == KEY_CTRL_EXE)
    {
      if (replace.size())
      {
        set_undo(text);
        ustl::string &s = text->elements[text->line].s;
        s = s.substr(0, text->pos - search.size()) + replace + s.substr(text->pos, s.size() - text->pos);
        search_msg();
      }
      return true;
    }
    if (key == KEY_CHAR_8 || key == KEY_CHAR_N || key == KEY_CTRL_EXIT)
    {
      search_msg();
      return true;
    }
    if (key == KEY_CTRL_AC)
    {
      show_status(text, search, replace);
      return false;
    }
  }
}

int check_leave(textArea *text)
{
  if (text->editable && text->filename.size())
  {
    if (text->changed)
    {
      // save or cancel?
      ustl::string tmp = text->filename;
      tmp = tmp.substr(7, tmp.size() - 7);
      if (strcmp(tmp.c_str(), "temp.py") == 0)
      {
        if (giac::confirm(giac::lang ? "Les modifs seront perdues" : "Changes will be lost", giac::lang ? "F1: annuler, F6: tant pis" : "F1: cancel, F6: confirm") == KEY_CTRL_F1)
          return 2;
        else
          return 0;
      }
      tmp += giac::lang ? " a ete modifie!" : " was modified!";
      if (giac::confirm(tmp.c_str(), giac::lang ? "F1: sauvegarder, F6: tant pis" : "F1: save, F6: discard changes") == KEY_CTRL_F1)
      {
        save_script(text->filename.c_str(), merge_area(text->elements));
        text->changed = false;
        return 1;
      }
      return 0;
    }
    return 1;
  }
  return 0;
}

void do_restart()
{
  giac::_restart(giac::gen(giac::vecteur(0), giac::_SEQ__VECT), contextptr);
}

void chk_restart()
{
  giac::drawRectangle(0, 24, LCD_WIDTH_PX, LCD_HEIGHT_PX - 24, COLOR_WHITE);
  if (giac::confirm(giac::lang ? "Conserver les variables?" : "Keep variables?", giac::lang ? "F1: conserver,   F6: effacer" : "F1: keep,   F6: erase") == KEY_CTRL_F6)
    do_restart();
}

// 0 not alpha symbol, blue (7) Xcas command, red (2) keyword, cyan (3) number,  green (4) comment, yellow (6) string
void print(int &X, int &Y, const char *buf, int color, bool revert, bool fake, bool minimini)
{
  //!!!!!!
  // std::cout << "print:" << buf << endl;
  if (!buf)
    return;
  // if (!fake) cout << "print:" << buf << " " << strlen(buf) << " " << color << endl;
  if (!isalpha(buf[0]) && color != giac::_YELLOW && color != giac::_GREEN)
    color = 0;
  if (!fake)
  {
    if (minimini || color == giac::_GREEN || color == giac::_YELLOW) // comment in small font
      PrintMini(X, Y, (const unsigned char *)buf, revert ? MINI_REV : 0);
    else
    {
      PrintXY(X, Y, (const unsigned char *)buf, revert ? 1 : 0);
      // overline/underline style according to color
      if (!revert)
      {
        if (color == giac::_BLUE)
        {                                                                               // 1 (command)
          giac::draw_line(X, Y + 13, X + 8 * strlen(buf), Y + 13, COLOR_BLACK, 0xAAAA); //!!!!!!!!! 7
          // giac::draw_line(X,Y+7,X+6,Y+7,COLOR_BLACK);
          // giac::draw_line(X+6*strlen(buf)-6,Y+7,X+6*strlen(buf),Y+7,COLOR_BLACK);
        }
        if (color == giac::_RED)
        {                                                                       // 2 (keyword)
          giac::draw_line(X, Y + 13, X + 8 * strlen(buf), Y + 13, COLOR_BLACK); //!!!!!!!!! 7
        }
      }
    }
  }
  X += ((minimini || color == giac::_GREEN || color == giac::_YELLOW) ? 6 : 8) * strlen(buf);
}

void match_print(char *singleword, int delta, int X, int Y, bool match, bool minimini)
{
  // char buflog[128];sprintf(buflog,"%i %i %s               ",delta,(int)match,singleword);puts(buflog);
  char ch = singleword[delta];
  singleword[delta] = 0;
  print(X, Y, singleword, 0, false, /* fake*/ true, minimini);
  singleword[delta] = ch;
  char buf[4];
  buf[0] = ch;
  buf[1] = 0;
  // inverted print: colors are reverted too!
  int color;
  if (minimini)
    color = match ? TEXT_COLOR_RED : TEXT_COLOR_GREEN;
  else
    color = match ? COLOR_RED : COLOR_GREEN;
  print(X, Y, buf, color, true, /*fake*/ false, minimini);
}

bool match(textArea *text, int pos, int &line1, int &pos1, int &line2, int &pos2)
{
  line2 = -1;
  line1 = -1;
  int linepos = text->line;
  const ustl::vector<textElement> &v = text->elements;
  if (linepos < 0 || linepos >= v.size())
    return false;
  const ustl::string *s = &v[linepos].s;
  int ss = s->size();
  if (pos < 0 || pos >= ss)
    return false;
  char ch = (*s)[pos];
  int open1 = 0, open2 = 0, open3 = 0, inc = 0;
  if (ch == '(' || ch == '[' || ch == '{')
  {
    line1 = linepos;
    pos1 = pos;
    inc = 1;
  }
  if (
      ch == '}' ||
      ch == ']' || ch == ')')
  {
    line2 = linepos;
    pos2 = pos;
    inc = -1;
  }
  if (!inc)
    return false;
  bool instring = false;
  for (;;)
  {
    for (; pos >= 0 && pos < ss; pos += inc)
    {
      if ((*s)[pos] == '"' && (pos == 0 || (*s)[pos - 1] != '\\'))
        instring = !instring;
      if (instring)
        continue;
      switch ((*s)[pos])
      {
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
      if (open1 == 0 && open2 == 0 && open3 == 0)
      {
        // char buf[128];sprintf(buf,"%i %i",pos_orig,pos);puts(buf);
        if (inc > 0)
        {
          line2 = linepos;
          pos2 = pos;
        }
        else
        {
          line1 = linepos;
          pos1 = pos;
        }
        return true;
      } // end if
    }   // end for pos
    linepos += inc;
    if (linepos < 0 || linepos >= v.size())
      return false;
    s = &v[linepos].s;
    ss = s->size();
    pos = inc > 0 ? 0 : ss - 1;
  } // end for linepos
  return false;
}
#if 0
bool match(const char * s,int pos_orig,const char * & match1,const char * & match2){
  match1=0; match2=0;
  int pos=pos_orig;
  int ss=strlen(s);
  if (pos<0 || pos>=ss) return false;
  char ch=s[pos_orig];
  int open1=0,open2=0,open3=0,inc=0;
  if (ch=='(' || ch=='['
      // || ch=='{'
      ){
    match1=s+pos_orig;
    inc=1;
  }
  if (
      //ch=='}' ||
      ch==']' || ch==')'
      ){
    match2=s+pos_orig;
    inc=-1;
  }
  if (!inc) return false;
  bool instring=false;
  for (pos=pos_orig;pos>=0 && pos<ss;pos+=inc){
    if (s[pos]=='"' && (pos==0 || s[pos-1]!='\\'))
      instring=!instring;
    if (instring)
      continue;
    switch (s[pos]){
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
      if (inc>0)
	match2=s+pos;
      else
	match1=s+pos;
      return true;
    }
  }
  return false;
}
#endif

ustl::string get_selection(textArea *text, bool erase)
{
  int sel_line1, sel_line2, sel_pos1, sel_pos2;
  int clipline = text->clipline, clippos = text->clippos, textline = text->line, textpos = text->pos;
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
  }
  ustl::string s(text->elements[sel_line1].s);
  if (erase)
  {
    set_undo(text);
    text->line = sel_line1;
    text->pos = sel_pos1;
    text->elements[sel_line1].s = s.substr(0, sel_pos1) + text->elements[sel_line2].s.substr(sel_pos2, text->elements[sel_line2].s.size() - sel_pos2);
  }
  if (sel_line1 == sel_line2)
  {
    return s.substr(sel_pos1, sel_pos2 - sel_pos1);
  }
  s = s.substr(sel_pos1, s.size() - sel_pos1) + '\n';
  int sel_line1_ = sel_line1;
  for (sel_line1++; sel_line1 < sel_line2; sel_line1++)
  {
    s += text->elements[sel_line1].s;
    s += '\n';
  }
  s += text->elements[sel_line2].s.substr(0, sel_pos2);
  if (erase)
    text->elements.erase(text->elements.begin() + sel_line1_ + 1, text->elements.begin() + sel_line2 + 1);
  return s;
}

void warn_python(int mode, bool autochange)
{
  if (mode == 0)
    giac::confirm(autochange ? (giac::lang ? "Source en syntaxe Xcas detecte." : "Xcas syntax source code detected.") : (giac::lang ? "Syntaxe Xcas." : "Xcas syntax."), "F1/F6: ok");
  if (mode == 1)
    if (autochange)
      giac::confirm(giac::lang ? "Passage en syntaxe Python." : "Setting Python syntax source.", giac::lang ? "avec ^=**, F1/F6: ok" : "with ^=**, F1/F6:ok");
    else
      giac::confirm(giac::lang ? "Syntaxe Python avec ^==**," : "Python syntax with ^==**,", giac::lang ? "python_compat(2): xor. F1: ok" : "python_compat(2): xor. F1: ok");
  if (mode == 2)
  {
    giac::confirm(giac::lang ? "Syntaxe Python avec ^==xor" : "Python syntax with ^==xor", giac::lang ? "python_compat(1): **. F1: ok" : "python_compat(1): **. F1: ok");
  }
}

bool change_mode(textArea *text, int flag)
{
  if (bool(text->python) != bool(flag))
  {
    text->python = flag;
    python_compat(text->python, contextptr);
    show_status(text, "", "");
    warn_python(flag, true);
    return true;
  }
  return false;
}

static void textarea_disp(textArea *text, int &isFirstDraw, int &totalTextY, int &scroll, int &textY, bool global_minimini)
{
  // *logptr(contextptr) << text->lineHeight << '\n';
  Cursor_SetFlashOff();
  bool editable = text->editable;
  int showtitle = !editable && (text->title != NULL);
  ustl::vector<textElement> &v = text->elements;
  if (v.empty())
  {
    textElement cur;
    cur.lineSpacing = 0;
    v.push_back(cur);
  }
  giac::drawRectangle(text->x, text->y, text->width, LCD_HEIGHT_PX, COLOR_WHITE);
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
  if (editable)
    PrintMini(0, F_KEY_BAR_Y_START, (const unsigned char *)EDITABLE_BAR, MINI_REV); //!!!!
  // giac::drawRectangle(text->x, text->y, text->width, LCD_HEIGHT_PX-text->y-editable?8:0, COLOR_WHITE);
  for (int cur = 0; cur < v.size(); ++cur)
  {
    const char *src = v[cur].s.c_str();
    if (cur == 0)
    {
      int l = v[cur].s.size();
      if (l >= 1 && src[0] == '#')
        change_mode(text, 1); // text->python=true;
      if (l >= 2 && src[0] == '/' && src[1] == '/')
        change_mode(text, 0); // text->python=false;
      if (l >= 8 && src[0] == 'f' && (src[1] == 'o' || src[1] == 'u') && src[2] == 'n' && src[3] == 'c' && src[4] == 't' && src[5] == 'i' && src[6] == 'o' && src[7] == 'n')
        change_mode(text, 0); // text->python=false;
      if (l >= 4 && src[0] == 'd' && src[1] == 'e' && src[2] == 'f' && src[3] == ' ')
        change_mode(text, 1);                                                                                       // text->python=true;
      giac::drawRectangle(text->x, text->y, text->width, LCD_HEIGHT_PX - text->y - editable ? 12 : 0, COLOR_WHITE); //!!!!! 8
    }
    int textX = text->x;
    bool minimini = v[cur].minimini ? v[cur].minimini == 1 : global_minimini;
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
        PrintMini(textX, textY, (const unsigned char *)line_s, 0);
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
        giac::drawRectangle(textX, textY, 2, 13, COLOR_BLACK);         /////!!!  6  //12
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
          (!text->python && *src == '/' && *(src + 1) == '/'))
      {
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
          PrintMini(textX, textY, (const unsigned char *)"\xe6\x9b", 0);
        // time for a new line
        textX = text->x + deltax;
        textY = textY + text->lineHeight + v[cur].lineSpacing;
        if (minimini)
          textY -= 1;
        ++nlines;
      } // else still fits, print new word normally (or just increment textX, if we are not "on stage" yet)
      if (textY >= text->y && textY <= LCD_HEIGHT_PX - 14)
      {
        temptextX = textX;
        if (editable)
        {
          couleur = linecomment ? giac::_GREEN : find_color(singleword);
          // cout << singleword << " " << couleur << endl;
          // 0 symbol, red keyword cyan number, blue command, yellow string
          // cout << singleword << " " << couleur << endl;
          // char ch[32];
          // sprint_int(ch,couleur);
          // puts(singleword); puts(ch);
        }
        else
          couleur = COLOR_BLACK;
        if (linecomment || !text->editable || singleword[0] == '"')
          print(textX, textY, singleword, couleur, invert, /*fake*/ false, minimini);
        else
        { // print two parts, commandname in color and remain in black
          char *ptr = singleword;
          if (isalpha(*ptr))
          {
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
            giac::drawRectangle(temptextX, textY, 2, 12, COLOR_BLACK); //!!!!
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
    clearLine(1, 1, false);
    drawScreenTitle((char *)text->title);
  }
  // if (editable) draw_menu(1);
#if 0
  int scrollableHeight = LCD_HEIGHT_PX-C24*(showtitle ? 2 : 1)-text->y;
  //draw a scrollbar:
  if(text->scrollbar) {
    TScrollbar sb;
    sb.I1 = 0;
    sb.I5 = 0;
    sb.indicatormaximum = totalTextY;
    sb.indicatorheight = scrollableHeight;
    sb.indicatorpos = -scroll;
    sb.barheight = scrollableHeight;
    sb.bartop = (showtitle ? C24 : 0)+text->y;
    sb.barleft = text->width - 6;
    sb.barwidth = 6;
    
    Scrollbar(&sb);
  }
#endif
}

#if 0
bool move_to_word(textArea * text,const ustl::string & s,const ustl::string & replace,int & isFirstDraw,int & totalTextY,int & scroll,int & textY){
  return false;
}
#else
bool move_to_word(textArea *text, const ustl::string &s, const ustl::string &replace, int &isFirstDraw, int &totalTextY, int &scroll, int &textY)
{
  if (!s.size())
    return false;
  int line = text->line, pos = text->pos;
  if (line >= text->elements.size())
    line = 0;
  if (pos >= text->elements[line].s.size())
    pos = 0;
  for (; line < text->elements.size(); ++line)
  {
    int p = text->elements[line].s.find(s, pos);
    if (p >= 0 && p < text->elements[line].s.size())
    {
      text->line = line;
      text->clipline = line;
      text->clippos = p;
      text->pos = p + s.size();
      textarea_disp(text, isFirstDraw, totalTextY, scroll, textY, text->minimini);
      text->clipline = -1;
      return chk_replace(text, s, replace);
    }
    pos = 0;
  }
  for (line = 0; line < text->line; ++line)
  {
    int p = text->elements[line].s.find(s, 0);
    if (p >= 0 && p < text->elements[line].s.size())
    {
      text->line = line;
      text->clipline = line;
      text->clippos = p;
      text->pos = p + s.size();
      textarea_disp(text, isFirstDraw, totalTextY, scroll, textY, text->minimini);
      text->clipline = -1;
      return chk_replace(text, s, replace);
    }
  }
  return false;
}
#endif

void textarea_help_insert(textArea *text, int exec)
{
  string curs = text->elements[text->line].s.substr(0, text->pos);
  if (!curs.empty())
  {
    int b;
    string adds = help_insert(curs.c_str(), b, exec);
    if (!adds.empty())
    {
      if (b > 0)
      {
        std::string &s = text->elements[text->line].s;
        if (b > text->pos)
          b = text->pos;
        if (b > s.size())
          b = s.size();
        s = s.substr(0, text->pos - b) + s.substr(text->pos, s.size() - text->pos); //+s.substr(b,s.size()-b);
      }
      insert(text, adds.c_str(), false);
    }
  }
}

int doTextArea(textArea *text)
{
  int scroll = 0;
  int isFirstDraw = 1;
  int totalTextY = 0, textY = 0;
  bool editable = text->editable;
  int showtitle = !editable && (text->title != NULL);
  int scrollableHeight = LCD_HEIGHT_PX - C24 * (showtitle ? 2 : 1) - text->y;
  ustl::vector<textElement> &v = text->elements;
  ustl::string search, replace;
  //std::cout << "text->y:" << text->y << endl;
  show_status(text, search, replace);
  if (text->line >= v.size())
    text->line = 0;
  textarea_disp(text, isFirstDraw, totalTextY, scroll, textY, text->minimini);
  while (1)
  {
    if (text->line >= v.size())
      text->line = 0;
    textarea_disp(text, isFirstDraw, totalTextY, scroll, textY, text->minimini);
    if (text->type == TEXTAREATYPE_INSTANT_RETURN)
      return 0;
    unsigned kf = GetSetupSetting((unsigned int)0x14);
    int keyflag = kf;
    unsigned int key;
    ck_getkey((int *)&key);
    if (key != KEY_CTRL_PRGM && key != KEY_CHAR_FRAC)
      translate_fkey(key);
    // char keylog[32];sprint_int(keylog,key); puts(keylog);
    show_status(text, search, replace);
    int &clipline = text->clipline;
    int &clippos = text->clippos;
    int &textline = text->line;
    int &textpos = text->pos;
    if (!editable && key >= KEY_CTRL_F1 && key <= KEY_CTRL_F3)
      return key;
    if (key == KEY_CTRL_SETUP)
    {
      text->minimini = !text->minimini;
      isFirstDraw = 0;
      continue;
    }
    if (editable)
    {
      if (key == KEY_CTRL_F10)
      { // mixedfrac key translated
        textarea_help_insert(text, key);
        continue;
      }
      if (key == KEY_CHAR_FRAC && clipline < 0)
      {
        if (textline == 0)
          continue;
        ustl::string &s = v[textline].s;
        ustl::string &prev_s = v[textline - 1].s;
        int indent = find_indentation(s), prev_indent = find_indentation(prev_s);
        if (!prev_s.empty())
          prev_indent += 2 * end_do_then(prev_s);
        int diff = indent - prev_indent;
        if (diff > 0 && diff <= s.size())
          s = s.substr(diff, s.size() - diff);
        if (diff < 0)
          s = string(-diff, ' ') + s;
        textpos -= diff;
        continue;
      }
      if (key == KEY_CTRL_VARS)
      {
        displaylogo();
        continue;
      }
      if (key == KEY_CHAR_ANS)
      {
        int err = check_parse(v, text->python);
        if (err) // move cursor to the error line
          textline = err - 1;
        continue;
      }
      if (key == KEY_CTRL_CLIP)
      {
#if 1
        if (clipline >= 0)
        {
          giac::copy_clipboard(get_selection(text, false), true);
          clipline = -1;
        }
        else
        {
          show_status(text, search, replace);
          clipline = textline;
          clippos = textpos;
        }
#else
        giac::copy_clipboard(v[textline].s, false);
        warning((char *)"Line copied to clipboard");
        // DisplayStatusArea();
#endif
        continue;
      }
      if (key == KEY_CTRL_F5)
      {
        giac::handle_f5();
        show_status(text, search, replace);
        continue;
      }
      if (clipline < 0)
      {
        const char *adds;
        if ((key >= KEY_CTRL_F1 && key <= KEY_CTRL_F3) ||
            (key >= KEY_CTRL_F7 && key <= KEY_CTRL_F14))
        {
          string le_menu = text->python ? "F1 test\nif \nelse \n<\n>\n==\n!=\n&&\n||\nF2 loop\nfor \nfor in\nrange(\nwhile \nbreak\ndef\nreturn \n#\nF3 misc\n:\n;\n_\n!\n%\n&\nprint(\ninput(\n" : "F1 test\nif \nelse \n<\n>\n==\n!=\nand\nor\nF2 loop\nfor \nfor in\nrange(\nwhile \nbreak\nf(x):=\nreturn \nlocal\nF3 misc\n;\n:\n_\n!\n%\n&\nprint(\ninput(\n";
          le_menu += "F7 arit\n mod \nirem(\nifactor(\ngcd(\nisprime(\nnextprime(\npowmod(\niegcd(\nF8 lin\nmatrix(\ndet(\nmatpow(\nranm(\ncross(\ncurl(\negvl(\negv(\nF9 list\nmakelist(\nrange(\nseq(\nsize(\nappend(\nranv(\nsort(\napply(\nF: plot\nplot(\nplotseq(\nplotlist(\nplotparam(\nplotpolar(\nplotfield(\nhistogram(\nbarplot(\nF; real\nexact(\napprox(\nfloor(\nceil(\nround(\nsign(\nmax(\nmin(\nF< prog\n;\n:\n\\\n&\n?\n!\ndebug(\npython(\nF= cplx\nabs(\narg(\nre(\nim(\nconj(\ncsolve(\ncfactor(\ncpartfrac(\nF> misc\n<\n>\n_\n!\n % \nrand(\nbinomial(\nnormald(";
          const char *ptr = console_menu(key, (unsigned char *)(le_menu.c_str()), 2);
          if (!ptr)
          {
            show_status(text, search, replace);
            continue;
          }
          adds = ptr;
        }
        else
          adds = giac::keytostring(key, keyflag, text->python, contextptr);
        if (adds)
        {
          bool isex = adds[0] == '\n';
          if (isex)
            ++adds;
          bool isif = strcmp(adds, "if ") == 0,
               iselse = strcmp(adds, "else ") == 0,
               isfor = strcmp(adds, "for ") == 0,
               isforin = strcmp(adds, "for in") == 0,
               isdef = (strcmp(adds, "f(x):=") == 0 || strcmp(adds, "def") == 0),
               iswhile = strcmp(adds, "while ") == 0,
               islist = strcmp(adds, "list ") == 0,
               ismat = strcmp(adds, "matrix ") == 0;
#if 0
	  if (islist){
	    input_matrix(true);
	    continue;
	  }
	  if (ismat){
	    input_matrix(false);
	    continue;
	  }
#endif
          if (text->python)
          {
            if (isif)
              adds = isex ? "if x<0:\nx=-x" : "if :\n";
            if (iselse)
              adds = "else:\n";
            if (isfor)
              adds = isex ? "for j in range(10):\nprint(j*j)" : "for  in range():\n";
            if (isforin)
              adds = isex ? "for j in [1,4,9,16]:\nprint(j)" : "for  in :\n";
            if (iswhile)
              adds = isex ? "a,b=25,15\nwhile b!=0:\na,b=b,a%b" : "while :\n";
            if (isdef)
              adds = isex ? "def f(x):\nreturn x*x*x\n" : "def f(x):\n\nreturn\n";
          }
          else
          {
            if (isif)
              adds = giac::lang ? (isex ? "si x<0 alors x:=-x; fsi;" : "si  alors\n\nsinon\n\nfsi;") : (isex ? "if x<0 then x:=-x; fi;" : "if  then\n\nelse\n\nfi;");
            if (giac::lang && iselse)
              adds = "sinon ";
            if (isfor)
              adds = giac::lang ? (isex ? "pour j de 1 jusque 10 faire\nprint(j*j);\nfpour;" : "pour  de  jusque  faire\n\nfpour;") : (isex ? "for j from 1 to 10 do\nprint(j*j);\nod;" : "for  from  to  do\n\nod;");
            if (isforin)
              adds = giac::lang ? (isex ? "pour j in [1,4,9,16] faire\nprint(j)\nfpour;" : "pour  in  faire\n\nfpour;") : (isex ? "for j in [1,4,9,16] do\nprint(j);od;" : "for  in  do\n\nod;");
            if (iswhile)
              adds = giac::lang ? (isex ? "a,b:=25,15;\ntantque b!=0 faire\na,b:=b,irem(a,b);\nftantque;a;" : "tantque  faire\n\nftantque;") : (isex ? "a,b:=25,15;\nwhile b!=0 do\na,b:=b,irem(a,b);\nod;a;" : "while  do\n\nod;");
            if (isdef)
              adds = giac::lang ? (isex ? "fonction f(x)\nlocal j;\nj:=x*x;\nreturn j;\nffonction:;\n" : "fonction f(x)\nlocal j;\n\nreturn ;\nffonction:;") : (isex ? "function f(x)\nlocal j;\nj:=x*x;\nreturn j;\nffunction:;\n" : "function f(x)\n  local j;\n\n return ;\nffunction:;");
          }
          insert(text, adds, key != KEY_CTRL_PASTE); // was true, but we should not indent when pasting
          show_status(text, search, replace);
          continue;
        }
      }
    }
    textElement *ptr = &v[textline];
    const int interligne = 12; //!!!!!
    switch (key)
    {
    case KEY_CTRL_DEL:
      if (clipline >= 0)
      {
        giac::copy_clipboard(get_selection(text, true), true);
        // erase selection
        clipline = -1;
      }
      else
      {
        if (editable)
        {
          if (textpos)
          {
            set_undo(text);
            ustl::string &s = v[textline].s;
            int nextpos = textpos - 1;
            if (textpos == find_indentation(s))
            {
              for (int line = textline - 1; line >= 0; --line)
              {
                int ind = find_indentation(v[line].s);
                if (textpos > ind)
                {
                  nextpos = ind;
                  break;
                }
              }
            }
            s.erase(s.begin() + nextpos, s.begin() + textpos);
            textpos = nextpos;
          }
          else
          {
            if (textline)
            {
              set_undo(text);
              --textline;
              textpos = v[textline].s.size();
              v[textline].s += v[textline + 1].s;
              v[textline].nlines += v[textline + 1].nlines;
              v.erase(v.begin() + textline + 1);
            }
          }
        }
        show_status(text, search, replace);
      }
      break;
    case KEY_CHAR_CR:
      if (clipline < 0 && editable)
      {
        set_undo(text);
        add_indented_line(v, textline, textpos);
        show_status(text, search, replace);
      }
      break;
    case KEY_CTRL_UNDO:
      undo(text);
      break;
    case KEY_CTRL_LEFT:
      if (editable)
      {
        --textpos;
        if (textpos < 0)
        {
          if (textline == 0)
            textpos = 0;
          else
          {
            --textline;
            show_status(text, search, replace);
            textpos = v[textline].s.size();
          }
        }
        if (textpos >= 0)
          break;
      }
    case KEY_CTRL_UP:
      if (editable)
      {
        if (textline > 0)
        {
          --textline;
          show_status(text, search, replace);
        }
        else
        {
          textline = 0;
          textpos = 0;
        }
      }
      else
      {
        if (scroll < 0)
        {
          scroll = scroll + interligne;
          if (scroll > 0)
            scroll = 0;
        }
      }
      break;
    case KEY_CTRL_RIGHT:
      ++textpos;
      if (textpos <= ptr->s.size())
        break;
      if (textline == v.size() - 1)
      {
        textpos = ptr->s.size();
        break;
      }
      textpos = 0;
    case KEY_CTRL_DOWN:
      if (editable)
      {
        if (textline < v.size() - 1)
          ++textline;
        else
        {
          textline = v.size() - 1;
          textpos = v[textline].s.size();
        }
        show_status(text, search, replace);
      }
      else
      {
        if (textY > scrollableHeight - (showtitle ? 0 : interligne))
        {
          scroll = scroll - interligne;
          if (scroll < -totalTextY + scrollableHeight - (showtitle ? 0 : interligne))
            scroll = -totalTextY + scrollableHeight - (showtitle ? 0 : interligne);
        }
      }
      break;
    case KEY_CTRL_F1:
      if (text->allowF1)
        return KEY_CTRL_F1;
      break;
    case KEY_SHIFT_LEFT:
      textpos = 0;
      break;
    case KEY_SHIFT_RIGHT:
      textpos = v[textline].s.size();
      break;
    case KEY_CTRL_PAGEDOWN:
      if (editable)
      {
        textline = v.size() - 1;
        show_status(text, search, replace);
        textpos = v[textline].s.size();
      }
      else
      {
        if (textY > scrollableHeight - (showtitle ? 0 : interligne))
        {
          scroll = scroll - scrollableHeight;
          if (scroll < -totalTextY + scrollableHeight - (showtitle ? 0 : interligne))
            scroll = -totalTextY + scrollableHeight - (showtitle ? 0 : interligne);
        }
      }
      break;
    case KEY_CTRL_PAGEUP:
      if (editable)
      {
        textline = 0;
        show_status(text, search, replace);
      }
      else
      {
        if (scroll < 0)
        {
          scroll = scroll + scrollableHeight;
          if (scroll > 0)
            scroll = 0;
        }
      }
      break;
    case KEY_CTRL_EXE:
      if (text->allowEXE)
        return TEXTAREA_RETURN_EXE;
      if (search.size())
      {
        for (;;)
        {
          if (!move_to_word(text, search, replace, isFirstDraw, totalTextY, scroll, textY))
            break;
        }
        show_status(text, search, replace);
        continue;
      }
      else
      {
        int err = check_parse(v, text->python);
        if (err) // move cursor to the error line
          textline = err - 1;
        continue;
      }
      break;
    case KEY_CTRL_F6:
      if (clipline < 0 && text->editable && text->filename.size())
      {
        Menu smallmenu;
        smallmenu.numitems = 12;
        MenuItem smallmenuitems[smallmenu.numitems];
        smallmenu.items = smallmenuitems;
        smallmenu.height = 8; //!!!!!
        smallmenu.scrollbar = 0;
        // smallmenu.title = "KhiCAS";
        smallmenuitems[0].text = (char *)(giac::lang ? "Tester syntaxe" : "Check syntax");
        smallmenuitems[1].text = (char *)(giac::lang ? "Sauvegarder" : "Save");
        smallmenuitems[2].text = (char *)(giac::lang ? "Sauvegarder comme" : "Save as");
        smallmenuitems[3].text = (char *)(giac::lang ? "Inserer" : "Insert");
        smallmenuitems[4].text = (char *)(giac::lang ? "Effacer" : "Clear");
        smallmenuitems[5].text = (char *)(giac::lang ? "Chercher,remplacer" : "Search, replace");
        smallmenuitems[6].text = (char *)(giac::lang ? "Aller a la ligne" : "Goto line");
        smallmenuitems[7].type = MENUITEM_CHECKBOX;
        smallmenuitems[7].text = (char *)"Python";
        smallmenuitems[7].value = text->python;
        smallmenuitems[8].type = MENUITEM_CHECKBOX;
        smallmenuitems[8].text = (char *)"Petite fonte";
        smallmenuitems[8].value = text->minimini;
        smallmenuitems[9].text = (char *)(giac::lang ? "Quitter" : "Quit");
        smallmenuitems[10].text = (char *)"Help";
        smallmenuitems[11].text = (char *)"A propos";
        int sres = doMenu(&smallmenu);
        show_status(text, search, replace);
        if (sres == MENU_RETURN_SELECTION)
        {
          sres = smallmenu.selection;
          if (sres >= 11)
          {
            textArea text;
            text.editable = false;
            text.clipline = -1;
            text.title = smallmenuitems[sres - 1].text;
            add(&text, smallmenu.selection == 11 ? shortcuts_string : apropos_string);
            doTextArea(&text);
            continue;
          }
          if (sres == 1)
          {
            int err = check_parse(v, text->python);
            if (err) // move cursor to the error line
              textline = err - 1;
          }
          if (sres == 3)
          {
            char filename[MAX_FILENAME_SIZE + 1];
            if (get_filename(filename, ".py"))
            {
              text->filename = filename;
              sres = 2;
            }
          }
          if (sres == 2)
          {
            save_script(text->filename.c_str(), merge_area(v));
            text->changed = false;
            char status[256];
            for (int i = 0; i < sizeof(status); ++i)
              status[i] = 0;
            sprintf(status, giac::lang ? "%s sauvegarde" : "%s saved", text->filename.c_str() + 7);
            warning(status);
            // DisplayStatusArea();
          }
          if (sres == 4)
          {
            char filename[MAX_FILENAME_SIZE + 1];
            ustl::string ins;
            if (fileBrowser(filename, (char *)"*.py", (char *)"Scripts") && load_script(filename, ins))
              insert(text, ins.c_str(), false); // add_nl(text,ins);
            show_status(text, search, replace);
          }
          if (sres == 5)
          {
            ustl::string s(merge_area(v));
            giac::copy_clipboard(s);
            set_undo(text);
            v.resize(1);
            v[0].s = "";
            textline = 0;
          }
          if (sres == 6)
          {
            textarea_disp(text, isFirstDraw, totalTextY, scroll, textY, text->minimini);
            search = get_searchitem(replace);
            if (!search.empty())
            {
              for (;;)
              {
                if (!move_to_word(text, search, replace, isFirstDraw, totalTextY, scroll, textY))
                {
                  break;
                }
              }
              show_status(text, search, replace);
            }
          }
          if (sres == 7)
          {
            textarea_disp(text, isFirstDraw, totalTextY, scroll, textY, text->minimini);
            int l = get_line_number(giac::lang ? "Negatif: en partant de la fin" : "Negative: counted from the end", giac::lang ? "Numero de ligne:" : "Line number:");
            if (l > 0)
              text->line = l - 1;
            if (l < 0)
              text->line = v.size() + l;
          }
          if (sres == 8)
          {
            text->python = text->python ? 0 : 1;
            show_status(text, search, replace);
            python_compat(text->python, contextptr);
            warn_python(text->python, false);
            textarea_disp(text, isFirstDraw, totalTextY, scroll, textY, text->minimini);
            // draw_menu(1);
          } 
          if (sres==9)
            text->minimini=!text->minimini;
          if (sres == 10)
          {
            int res = check_leave(text);
            if (res == 2)
              continue;
            return TEXTAREA_RETURN_EXIT;
          }
        }
      }
      break;
    case KEY_CTRL_F2:
      if (clipline < 0)
        return KEY_CTRL_F2;
    case KEY_CTRL_EXIT:
    {
      if (clipline >= 0)
      {
        clipline = -1;
        show_status(text, search, replace);
        continue;
      }
      int res = check_leave(text);
      if (res == 2)
        continue;
      return TEXTAREA_RETURN_EXIT;
    }
    case KEY_CTRL_INS:
      break;
    default:
      if (clipline < 0 && key >= 32 && key < 128 && editable)
      {
        char buf[2] = {char(key), 0};
        insert(text, buf, false);
        show_status(text, search, replace);
      }
      if (key == KEY_CTRL_AC)
      {
        if (clipline >= 0)
        {
          clipline = -1;
          show_status(text, search, replace);
        }
        else
        {
          if (search.size())
          {
            search = "";
            show_status(text, search, replace);
          }
          else
          {
            giac::copy_clipboard(v[textline].s + '\n');
            if (v.size() == 1)
              v[0].s = "";
            else
            {
              v.erase(v.begin() + textline);
              if (textline >= v.size())
                --textline;
            }
            warning((char *)"Line cut and copied to clipboard");
            // DisplayStatusArea();
          }
        }
      }
    }
  }
}

#if 0 // version that marks end of file with 0 0
void save_script(const char * filename,const ustl::string & s){
  unsigned short pFile[MAX_FILENAME_SIZE+1];
  // create file in data folder (assumes data folder already exists)
  Bfile_StrToName_ncpy(pFile, filename, strlen(filename)+1);
  // even if it already exists, there's no problem,
  // in the event that our file shrinks, we just let junk be at the end of
  // the file (after two null bytes, of course).
  int hFile = Bfile_OpenFile_OS(pFile, READWRITE); // Get handle
  if(hFile < 0) {
    // error. file does not exist yet. try creating it
    // (data folder should exist already, as save_console_state_smem() should have been called before this function)
    size_t size = 1;
    Bfile_CreateEntry_OS(pFile, CREATEMODE_FILE, &size);
    // now try opening
    hFile = Bfile_OpenFile_OS(pFile, READWRITE); // Get handle
    if(hFile < 0) return; // if it still fails, there's nothing we can do
  }
  Bfile_WriteFile_OS(hFile, s.c_str(), s.size());
  char buf[2]={0,0};
  Bfile_WriteFile_OS(hFile, buf, 2);
  Bfile_CloseFile_OS(hFile);  
}  


void save_script(const char * filename,const ustl::string & s){
  unsigned short pFile[MAX_FILENAME_SIZE+1];
  // create file in data folder (assumes data folder already exists)
  Bfile_StrToName_ncpy(pFile, (const unsigned char *)filename, strlen(filename)+1);
  // even if it already exists, there's no problem,
  // in the event that our file shrinks, we just let junk be at the end of
  // the file (after two null bytes, of course).
  int hFile = Bfile_OpenFile_OS(pFile, _OPENMODE_READWRITE); // Get handle
  bool open=false;
  if(hFile < 0){
  }
  else {
    size_t old=Bfile_GetFileSize_OS(hFile);
    if (old<=s.size())
      open=true;
    else {
      Bfile_CloseFile_OS(hFile);
      Bfile_DeleteEntry(pFile);
    }
  }
  if (!open){
    // file does not exist yet. try creating it
    // (data folder should exist already, as save_console_state_smem() should have been called before this function)
    size_t size = 1;
    Bfile_CreateFile(pFile, size);
    // now try opening
    hFile = Bfile_OpenFile_OS(pFile, _OPENMODE_READWRITE); // Get handle
    if(hFile < 0) return; // if it still fails, there's nothing we can do
  }
  Bfile_WriteFile_OS(hFile, s.c_str(), s.size());
  //char buf[2]={0,0};
  //Bfile_WriteFile_OS(hFile, buf, 2);
  Bfile_CloseFile_OS(hFile);  
}

#endif

int get_line_number(const char *msg1, const char *msg2)
{
  ustl::string s;
  int res = giac::inputline(msg1, msg2, s, false);
  if (res == KEY_CTRL_EXIT)
    return 0;
  res = strtol(s.c_str(), 0, 10);
  return res;
}
