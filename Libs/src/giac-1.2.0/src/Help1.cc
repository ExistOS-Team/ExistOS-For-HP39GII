// -*- mode:C++ ; compile-command: "g++ -I.. -I../include -I../../giac/include -g -c Help1.cc" -*-
#include "Help1.h"
#include "Xcas1.h"
/*
 *  Copyright (C) 2000,2014 B. Parisse, Institut Fourier, 38402 St Martin d'Heres
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


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_LIBFLTK
#include <FL/fl_ask.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Hold_Browser.H>
#include <fstream>
#include "vector.h"
#include <algorithm>
#include <fcntl.h>
#include <cmath>
#include <time.h> // for nanosleep
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h> // auto-recovery function

using namespace std;
using namespace giac;

#ifndef NO_NAMESPACE_XCAS
namespace xcas {
#endif // ndef NO_NAMESPACE_XCAS

  string translate_html_title(const string & s){
    if (debug_infolevel)
      cerr << s << endl;
    int l=s.size();
    string res;
    for (int i=0;i<l;++i){
      if (s[i]!='&'){
	switch(s[i]){
	case '\n':
	  res += ' ';
	  break;
	case 'é': case 'è': case 'É':
	  res += 'e';
	  break;
	case 'à':
	  res += 'a';
	  break;
	case 'ô':
	  res += 'o';
	  break;
	default:
	  res += s[i];
	}
      }
      else {
	int pos=s.find(';',i);
	if (pos>=l || pos<=i)
	  res +=s[i];
	else {
	  unsigned code=0,base=10;
	  char c;
	  for (int cur=i+2;cur<=pos;++cur){
	    c=s[cur];
	    if (c=='x' || c=='X')
	      base=16;
	    if (c=='o' || c=='O')
	      base=8;
	    if (c!=';'){
	      if (base!=16)
		code = code*base+c-'0';
	      else {
		if (c>='A' && c<='F')
		  code = code*base + c-'A'+10;
		if (c>='a' && c<='f')
		  code = code*base + c-'a'+10;
		if (c>='0' && c<='9')
		  code = code*base + c-'0';
	      }
	      continue;
	    }
	    switch (code){
	    case 0xe8: case 0xe9: case 0xea:
	      c='e';
	      break;
	    case 0xe0: case 0xe2:
	      c='a';
	      break;
	    case 0xf4:
	      c='o';
	      break;
	    case 0xf9: case 0xfb:
	      c='u';
	      break;
	    case 0xe7:
	      c='c';
	      break;
	    case 238:
	      c='i';
	      break;
	    case 0x2019:
	      c=' ';
	      break;
	    } // end switch
	    break;
	  } // end loop for (cur=i;cur<pos;++cur)
	  i=pos;
	  res += c;
	}
      }
    }
    return res;
  }

  string help_translate(const string & s){
    int l=s.size();
    string res;
    for (int i=0;i<l;++i){
      unsigned char c=s[i];
      if (i<l-1 && c==0xc3){
	++i;
	c=s[i];
	switch (c){
	case 0xa8: case 0xa9: case 0xaa: case 0xab:
	  res += 'e';
	  break;
	case 0xa7:
	  res += 'c';
	  break;
	case 0xa0: case 0xa1: case 0xa2: case 0xa4:
	  res +='a';
	  break;
	case 0xb9: case 0xba: case 0xbb: case 0xbc:
	  res +='u';
	  break;
	case 0xb4: case 0xb5: case 0xb6:
	  res +='o';
	  break;
	case 0xae: case 0xaf:
	  res += 'i';
	  break;
	}
	continue;
      }
      switch (s[i]){
	case 'é': case 'è': case 'É':
	  res += 'e';
	  break;
	case 'à':
	  res += 'a';
	  break;
	case 'ô':
	  res += 'o';
	  break;
	  /*
      case 'Ã©':
	res += 'e'; // "&#233;";
	break;
      case 'Ã¨':
	res +='e'; // "&#232;";
	break;
      case 'Ãª':
	res += 'e'; // "&#234;";
	break;
      case 'Ã ':
	res += 'a'; // "&#224;";
	break;
      case 'Ã®':
	res += 'i'; // "&#238;";
	break;
	  */
      default:
	res += tolower(s[i]);
      }
    }
    return res;
  }

  // help on topic s
  void help_fltk(const string & s_orig){
    static Fl_Window * w = 0;
    static Fl_Button * button0 = 0;
    static Fl_Button *button1 = 0;
    static Fl_Hold_Browser * br = 0;
    static Fl_Input * in=0;
    static vector<string> v;
    if (!w){
#ifdef IPAQ
      int dx=230,dy=140,l=10;
#else
      int dx=460,dy=260,l=20;
#endif
      if (xcas::Xcas_input_focus && xcas::Xcas_input_focus->window()){
	dx=max(dx,2*xcas::Xcas_input_focus->window()->w()/3);
	dy=max(dy,2*xcas::Xcas_input_focus->window()->h()/3);
	l=xcas::Xcas_input_focus->window()->labelsize();
      }
      l += 6;
      Fl_Group::current(0);
      w = new Fl_Window(dx,dy);
      w->label(gettext("Find word in HTML help"));
      button0 = new Fl_Button(2,2,dx/2-4,l);
      button1 = new Fl_Button(dx/2+2,2,dx/2-4,l);
      in = new Fl_Input(dx/4,l+4,3*dx/4-2,l);
      br = new Fl_Hold_Browser(2,2*l+6,dx-4,dy-2*l-8);
      w->end();
      l -= 6;
      change_group_fontsize(w,l);
      button0->shortcut("^[");
      button1->label(gettext("Finish"));
      button1->color(FL_ROUGE_PALE);
      in->label(gettext("Searching for "));
      in->when(FL_WHEN_ENTER_KEY|FL_WHEN_NOT_CHANGED);
    }
    if (!s_orig.empty()){
      in->value(s_orig.c_str());
      in->position(s_orig.size(),s_orig.size());
    }
    Fl::focus(in);
    // Xcas->hide();
    w->resizable(w);
    w->set_modal();
    w->show();
    Fl::wait(0.0001);
    int r=0;
    for (;r!=1;){
      button0->label(br->size()>0?gettext("View"):gettext("Find"));
      button0->color(FL_VERT_PALE);
      for (;;) {
	Fl_Widget *o = Fl::readqueue();
	if (!o) Fl::wait();
	else {
	  if (o == button0) { // view help file
	    int tmp=br->value(); 
	    if (tmp) {
	      if (use_external_browser)
		system_browser_command(v[tmp-1]);
	      else {
		use_external_browser=false;
		if (Xcas_help_window){
		  Xcas_help_window->load((v[tmp-1]).c_str());
		  xcas::Xcas_help_window->show();
		}
	      }
	    }
	    else {
	      if (!br->size())
		in->do_callback();
	      else
		fl_alert(gettext("Please select the title of the page"));
	    }
	  } 
	  if (o == button1) {r = 1; break;}
	  if (o == w) { r=1; break; }
	  if (o==in) { break; }
	}
      }
      if (r==0){
	string s=help_translate(in->value());
	button0->label(gettext("Stop"));
	button0->color(FL_MAGENTA);
	br->clear();
	// fill browser
	//multimap<string,string>::const_iterator it=html_mtt.begin(),itend=html_mtt.end();
	v.clear();
	vector<string>::const_iterator it=html_vall.begin(),itend=html_vall.end();
	for (;it!=itend;++it){
	  Fl_Widget *o = Fl::readqueue();
	  if (o){
	    if (o==button0) {break;}
	    if (o==button1 || o==w){ r=1; break; }
	  }
	  else {
	    if ( ! ((itend-it)%10) )
	      Fl::wait(0.0001);
	  }
	  //string fname(absolute_path(it->second));
	  string fname(absolute_path(*it));
	  // remove #... part at the end of the URL
	  int lf=fname.size()-1;
	  for (;lf>0;--lf){
	    if (fname[lf]=='#'){
	      fname=fname.substr(0,lf);
	      break;
	    }
	  }
	  if (grep(fname,s)){
	    FILE * f=fopen(fname.c_str(),"r");
	    if (f && grep(f,"<TITLE>")){
	      string title;
	      for (;!feof(f) && !ferror(f);){
		char c=fgetc(f);
		if (feof(f) || c=='<')
		  break;
		title += c;
	      }
	      fclose(f);
	      br->add(translate_html_title(title).c_str()); 
	      //v.push_back(it->second);
	      v.push_back(*it);
	      br->redraw();
	    }
	  }
	} // end for (;it!=itend;)
      } // end if (r==0)
    } // end for (;;)
    w->hide();
  }




#ifndef NO_NAMESPACE_XCAS
} // namespace giac
#endif // ndef NO_NAMESPACE_XCAS

#endif // HAVE_LIBFLTK
