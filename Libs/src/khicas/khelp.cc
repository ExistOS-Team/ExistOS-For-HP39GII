// -*- mode:C++ ; compile-command: "g++ -I.. -g -c help.cc -Wall" -*-
//#define _SCL_SECURE_NO_WARNINGS
#include "giacPCH.h"

#include "path.h"
/*
 *  Copyright (C) 2000,14 B. Parisse, Institut Fourier, 38402 St Martin d'Heres
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
using namespace std;
#include <algorithm>
#include "gen.h"
#include "help.h"
#include "iostream"
#include "global.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#if defined VISUALC || defined BESTA_OS


#define opendir FindFirstFile
#define readdir FindNextFile
#define closedir FindClose
#define DIR WIN32_FIND_DATA
#define GNUWINCE 1

#else // VISUALC or BESTA_OS

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#if !defined BESTA_OS && !defined NSPIRE && !defined FXCG && !defined NUMWORKS // test should always return true
#include <dirent.h>
#endif

#endif // VISUALC or BESTA_OS

#include "input_lexer.h"

#ifndef NO_NAMESPACE_GIAC
namespace giac {
#endif // ndef NO_NAMESPACE_GIAC
  
  const int HELP_LANGUAGES=5;

  struct static_help_t {
    const char * cmd_name;
    const char * cmd_howto[HELP_LANGUAGES];
    const char * cmd_syntax;
    const char * cmd_related;
    const char * cmd_examples;
  };

  const static_help_t static_help[]={
#if 0
#include "static_help.h"
#else
    { "", { "", "", "", "",""}, "", "", "" },
#endif
  };

  const int static_help_size=sizeof(static_help)/sizeof(static_help_t);

  struct static_help_sort {
    static_help_sort() {}
    inline bool operator () (const static_help_t & a ,const static_help_t & b){
      return strcmp(a.cmd_name, b.cmd_name) < 0;
    }
  };

  inline int mon_max(int a,int b){
    if (a>b)
      return a;
    else
      return b;
  }

  bool seconddec (const pair<int,int> & a,const pair<int,int> & b){
    return a.second>b.second;
  }

  // NB: cmd_name may be localized but related is not localized
  bool has_static_help(const char * cmd_name,int lang,const char * & howto,const char * & syntax,const char * & related,const char * & examples){
#ifdef GIAC_HAS_STO_38
    const char nullstring[]=" ";
#else
    const char nullstring[]="";
#endif
    if (cmd_name[0]==0)
      return false;
    if (lang<=0)
      lang=2;
    if (lang>HELP_LANGUAGES)
      lang=2;
    string s=unlocalize(cmd_name[0]==' '?cmd_name+1:cmd_name);
    int l=int(s.size());
    if ( l>2 && s[0]=='\'' && s[l-1]=='\'' ){
      s=s.substr(1,l-2);
      l-=2;
    }
    if (l>2 && (s[l-1]==' ' || s[l-1]=='(')){
      s=s.substr(0,l-1);
      --l;
    }
    static_help_t h={s.c_str(),{0,0,0,0,0},0,0,0};
    std::pair<const static_help_t *,const static_help_t *> p=equal_range(static_help,static_help+static_help_size,h,static_help_sort());
    if (p.first!=p.second && p.first!=static_help+static_help_size){
      howto=p.first->cmd_howto[lang-1];
      if (!howto)
	howto=p.first->cmd_howto[1];
      syntax=p.first->cmd_syntax;
      if (!syntax)
	syntax=nullstring;
      related=p.first->cmd_related;
      if (!related)
	related=nullstring;
      examples=p.first->cmd_examples;
      if (!examples)
	examples=nullstring;
      return true;
    }
    return false;
  }

  static std::string output_quote(const string s){
    string res;
    int ss=int(s.size());
    for (int i=0;i<ss;++i){
      switch (s[i]){
      case '"':
      case '\\':
	res += '\\';
      default:
	res += s[i];
      }
    }
    return res;
  }

  bool operator < (const indexed_string & is1,const indexed_string & is2){ 
    if (is1.index!=is2.index) return is1.index<is2.index;
    return (is1.chaine<is2.chaine);
  }

  const char default_helpfile[]=giac_aide_location; // help filename
  const int HELP_MAXLENSIZE = 1600; // less than 20 lines of 80 chars

  string printint(int i){
    if (!i)
      return string("0");
    if (i<0)
      return string("-")+printint(-i);      
    int length = (int) std::floor(std::log10((double) i));
#if defined VISUALC || defined BESTA_OS
    char * s =new char[length+2];
#else
    char s[length+2];
#endif
    s[length+1]=0;
    for (;length>-1;--length,i/=10)
      s[length]=i%10+'0';
#if defined VISUALC || defined BESTA_OS
     string res=s;
     delete [] s;
     return res;
#else
    return s;
#endif
  }

  inline int max(int a,int b,int c){
    if (a>=b){
      if (a>=c)
	return a;
      else
	return c;
    }
    if (b>=c)
      return b;
    else
      return c;
  }

  vector<aide> readhelp(const char * f_name,int & count,bool warn){
    vector<aide> v(1);
    readhelp(v,f_name,count,warn);
    return v;
  }
  // FIXME: aide_cas may end with synonyms (# cmd synonym1 ...)
  void readhelp(vector<aide> & v,const char * f_name,int & count,bool warn){
    return;
  }


  aide helpon(const string & demande,const vector<aide> & v,int language,int count,bool with_op){
    aide result;
    return result;
  }

  string writehelp(const aide & cur_aide,int language){
    return "";
  }

  // static char otherchars[]="_.~ ¡¢£¤¥¦§¨©ª«¬­®¯°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿ";

  bool isalphan(char ch){
    if (ch>='0' && ch<='9')
      return true;
    if (ch>='a' && ch<='z')
      return true;
    if (ch>='A' && ch<='Z')
      return true;
    if (unsigned(ch)>128)
      return true;
    if (ch=='_' || ch=='.' || ch=='~')
      return true;
    /*
    char * ptr=otherchars;
    for (;*ptr;++ptr){
      if (ch==*ptr)
	return true;
    }
    */
    return false;
  }

  std::string unlocalize(const std::string & s){
    return s;
  }

  std::string localize(const std::string & s,int language){
    return s;
  }

#ifndef NO_NAMESPACE_GIAC
} // namespace giac
#endif // ndef NO_NAMESPACE_GIAC
