// -*- mode:C++ ; compile-command: "g++ -I.. -g help.o aide.cc" -*-
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "first.h"
/*
 *  Copyright (C) 2000 B. Parisse, Institut Fourier, 38402 St Martin d'Heres
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

#include "help.h"
#include "global.h"
#include <iostream>
using namespace std;
using namespace giac;

int main(int ARGC, char *ARGV[]){
  if (ARGC<2){
    cerr << "Syntax: " << ARGV[0] << " command_name" << endl;
    return 1;
  }
  // find current language: aide=1 (francais), help=2 (english), etc.
  int language=1;
  string current(ARGV[0]);  
  if (current=="fr_cas_help")
    language=1;
  if ( (current=="cas_help") || (current=="en_cas_help") )
    language=2;
  if (current=="es_cas_help")
    language=3;
  // read all help
  int count;
  vector<aide> v;
  giac::readhelp(v,"aide_cas",count,false);
  if (!count){
    if (getenv("XCAS_HELP"))
      giac::readhelp(v,getenv("XCAS_HELP"),count,true);
    else
      giac::readhelp(v,(giac::giac_aide_dir()+"aide_cas").c_str(),count,true);
  }
  multimap<string,string> mtt,mall;
  string html_help_dir("doc/");
  switch (language){
  case 1:
    html_help_dir +="fr/";
    break;
  case 2:
    html_help_dir +="en/";
    break;
  case 3:
    html_help_dir +="es/";
    break;
  }
  if (string(ARGV[1])=="*"){
    // output all example
    int s=v.size();
    for (int i=0;i<s;++i){
      cout << "//" << v[i].cmd_name << endl;
      int examples=v[i].examples.size();
      for (int j=0;j<examples;++j)
	cout << v[i].examples[j] << ";" << endl;
    }
    return 0;
  }
#ifndef EMCC
  find_all_index(html_help_dir,mtt,mall);
#endif
  for (int j=1;j<ARGC;++j){
    // search for ARGV
    cout << "Help for " << ARGV[j] << ":" << endl;
    current = string(ARGV[j]);
    aide cur_aide=helpon(current,v,language,count);
    string result=writehelp(cur_aide,language);
    cout << result;
#ifndef EMCC
    vector<string> v(html_help(mtt,current));
    vector<string>::const_iterator it=v.begin(),itend=v.end();
    for (;it!=itend;++it){
      cout << *it << endl;
    }
#endif
  } 
  return 0;
} // end main
