// -*- mode:C++ ; compile-command: "g++ -DHAVE_CONFIG_H -I. -I.. -I/usr/local/include -g -c History.cc -Wall" -*-
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifndef IN_GIAC
#include <giac/first.h>
#else
#include "first.h"
#endif
/*
 *  Copyright (C) 2005,2014 B. Parisse, Institut Fourier, 38402 St Martin d'Heres
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

#ifdef HAVE_LIBFLTK
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Tile.H>
#include <FL/fl_ask.H>
#include <FL/Fl_File_Chooser.H>
#include "History.h"
#include "Input.h"
#include "Equation.h"
#include "Print.h"
#include "Graph.h"
#include "Graph3d.h"
#include "Tableur.h"
#include "Editeur.h"
#include "Cfg.h"
#ifndef IN_GIAC
#include <giac/tex.h>
#else
#include "tex.h"
#endif
#include <FL/Fl_Multiline_Output.H>
#include <iostream>
#include <fstream>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#ifdef HAVE_SSTREAM
#include <sstream>
#else
#include <strstream>
#endif
#ifdef WIN32
#ifndef GNUWINCE
#include <sys/cygwin.h>
#endif
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

using namespace std;
using namespace giac;

#ifdef HAVE_LIBFLTK
#if defined(FL_DEVICE) || (!defined(__APPLE__) && !defined(WIN32)) 
#include <FL/Fl_File_Chooser.H>
char *file_chooser(const char *message,const char *pat,const char *fname,int relative){
  return fl_file_chooser(message,pat,fname,relative);
}
char *load_file_chooser(const char *message,const char *pat,const char *fname,int relative,bool save){
  return fl_file_chooser(message,pat,fname,relative);
}
#else // FL_DEVICE
#include <FL/Fl_Native_File_Chooser.H>
char *load_file_chooser(const char *message,const char *pat,const char *fname,int relative,bool save){
  // Create native chooser
  Fl_Native_File_Chooser native(Fl_Native_File_Chooser::BROWSE_FILE);
  static string filename="";
  string path(pat);
  int l=path.find('\n');
  if (l<0 || l>=int(path.size()))
    path += '\n';
  native.title(message);
  if (save)
    native.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
  else
    native.type(Fl_Native_File_Chooser::BROWSE_FILE);
  native.filter(path.c_str());
  // native.filter("Text\t*.txt\n"); 
  native.preset_file(fname);
  // Show native chooser
  switch ( native.show() ) {
  case -1: 
    return 0;	// ERROR
  case  1: 
    return 0;		// CANCEL
  default: // PICKED FILE
    filename=native.filename();
    fname=filename.c_str();
    return (char *) filename.c_str();
  }
}


#if 0 // def __APPLE__
Fl_Input * file_chooser_filename = NULL;
Fl_Window * file_chooser_win =0;
Fl_Button * file_chooser_but1=0;
Fl_Button * file_chooser_but2=0;
Fl_Button * file_chooser_but3=0;

char *file_chooser(const char *message,const char *pat,const char *fname,int relative){
  if (!file_chooser_win){
    Fl_Group::current(0);
    file_chooser_win = new Fl_Window(600, 80,gettext("Choose file"));
    file_chooser_win->begin();
    int y = 10;
    file_chooser_filename = new Fl_Input(100, y, file_chooser_win->w()-120, 25, gettext("Filename"));
    if (fname) file_chooser_filename->value(fname);
    file_chooser_filename->tooltip(gettext("Current filename"));
    y += file_chooser_filename->h() + 5;
    file_chooser_but1 = new Fl_Button(20, y, file_chooser_win->w()/3-40, 25, gettext("Choose file to overwrite"));
    file_chooser_but2 = new Fl_Button(file_chooser_win->w()/3+20, y,file_chooser_win->w()/3-40, 25, gettext("OK"));
    file_chooser_but3 = new Fl_Button(2*file_chooser_win->w()/3+20, y,file_chooser_win->w()/3-40, 25, gettext("Cancel"));
    y += file_chooser_filename->h() + 5;
    file_chooser_win->end();
  }
  file_chooser_win->set_modal();
  file_chooser_win->show();
  file_chooser_win->hotspot(file_chooser_win);
  int r=-1;
  for (;r<0;) {
    Fl_Widget *o = Fl::readqueue();
    if (!o) Fl::wait();
    else {
      if (o == file_chooser_but1){
	Fl_Native_File_Chooser native(Fl_Native_File_Chooser::BROWSE_FILE);
	string path(pat);
	int l=path.find('\n');
	if (l<0 || l>=path.size())
	  path += '\n';
	native.title(message);
	native.type(Fl_Native_File_Chooser::BROWSE_FILE);
	native.filter(path.c_str());
	// native.filter("Text\t*.txt\n"); 
	native.preset_file(fname);
	// Show native chooser
	switch ( native.show() ) {
	case -1: // Error
	  fl_alert("%s",(string(gettext("Error"))+native.errmsg()).c_str()); 
	  break;  
	case  1: // Cancel
	  break;
	default:
	  if ( native.filename() ) {
	    file_chooser_filename->value(native.filename());
	    file_chooser_filename->position(0,0);
	  } else {
	    file_chooser_filename->value("<filename>");
	  }
	  break;
	}
      }
      if (o == file_chooser_but2) r = 2; // ok
      if (o == file_chooser_but3) r = 3; // cancel
    }
  } // end for (;r<0;)
  file_chooser_win->hide();
  if (r==2)
    return (char *)(file_chooser_filename->value());
  else
    return 0;
}
#else
char *file_chooser(const char *message,const char *pat,const char *fname,int relative){
  return load_file_chooser(message,pat,fname,relative,true);
}
#endif // __APPLE__

#endif // FL_DEVICE
#endif // HAVE_LIBFLTK

#ifndef NO_NAMESPACE_XCAS
namespace xcas {
#endif // ndef NO_NAMESPACE_XCAS

  static std::vector<std::string> Xcas_recent_filenames;
  static std::string Xcas_recent_filenames_filename("xcas_recent");
  void (*Xcas_load_filename)(const char * filename,bool modified)=0;
  static const unsigned Xcas_recent_filenames_size=10;
  static Fl_Menu_ * recent_filenames_menu=0;

  void cb_recent_filename(Fl_Widget * wid , void* ptr){
    Fl_Menu_ * M =dynamic_cast<Fl_Menu_ *>(wid);
    if (!M)
      return ;
    const Fl_Menu_Item * m=M->mvalue();
    unsigned n=0;
    for (--m;;++n,--m){
      if (m->submenu())
	break;
    }
    if (n==0 || n>Xcas_recent_filenames.size())
      return ;
    if (Xcas_load_filename)
      (*Xcas_load_filename)(Xcas_recent_filenames[n-1].c_str(),false);
  }
  void add_recent_filename(const std::string & s,bool writerecent){
    Fl_Menu_ * menu = recent_filenames_menu;
    string s_=remove_path(s);
    if (s_.substr(0,10)=="xcas_auto_")
      return;
    unsigned i=0;
    for (;i<Xcas_recent_filenames.size();++i){
      if (s_==remove_path(Xcas_recent_filenames[i]))
	break;
    }
    if (i==Xcas_recent_filenames.size()){
      Xcas_recent_filenames.push_back(s);
      int offset=menu->add((gettext("File")+string("/")+gettext("Open")+string("/")+remove_path(s)).c_str(),0,(Fl_Callback*)cb_recent_filename);
      if (Xcas_recent_filenames.size()>Xcas_recent_filenames_size){
	Xcas_recent_filenames.erase(Xcas_recent_filenames.begin());
	menu->remove(4);
      }
      menu->textsize(menu->labelsize());
      if (writerecent){
#ifdef WIN32
	ofstream of((giac::xcasroot()+Xcas_recent_filenames_filename).c_str());
#else
	ofstream of((giac::home_directory()+Xcas_recent_filenames_filename).c_str());
#endif
	for (unsigned i=0;i<Xcas_recent_filenames.size();++i)
	  of << Xcas_recent_filenames[i] << endl;
	of.close();
      }
    }
  }
  void read_recent_filenames(Fl_Menu_ * menu){
    recent_filenames_menu=menu;
#ifdef WIN32
    return;
    ifstream of((giac::xcasroot()+Xcas_recent_filenames_filename).c_str());
#else
    ifstream of((giac::home_directory()+Xcas_recent_filenames_filename).c_str());
#endif
    Xcas_recent_filenames.clear();
    for (;;){
      char buf[65536];
      of.getline(buf,65535);
      if (!of)
	break;
      add_recent_filename(buf,false);
    }
  }
  std::string autosave_folder;
  int GRABAREA=4;
  bool fl_handle_lock=false; // True when Fl::handle is busy
  int labeladd=6;
  Fl_Output * Parse_error_output = 0;
  owstream * logstream = 0;
  Fl_Color Xcas_input_color=FL_BLACK,Xcas_input_background_color=FL_WHITE,
    Xcas_comment_color=FL_DARK_GREEN,Xcas_comment_background_color=FL_WHITE,
    Xcas_log_color=0x3a,Xcas_log_background_color=FL_WHITE,
    Xcas_equation_input_color=FL_BLACK,Xcas_equation_input_background_color=FL_WHITE,
    Xcas_equation_color=FL_BLUE,Xcas_equation_background_color=FL_WHITE,
    Xcas_editor_color=FL_BLACK,Xcas_editor_background_color=FL_WHITE,
    Xcas_background_color =FL_GRAY;
  int Xcas_save_saved_color=FL_VERT_PALE,Xcas_save_unsaved_color=FL_ROUGE_PALE;
  // NB search for 167/211 for save button color
  void (* Keyboard_Switch )(unsigned)=0;

  void set_colors(Fl_Widget * w,bool do_redraw){
    if (do_redraw)
      w->redraw();
    if (Fl_Group * g= dynamic_cast<Fl_Group *>(w)){
      int n=g->children();
      for (int i=0;i<n;++i)
	set_colors(g->child(i));
      return;
    }
    if (Comment_Multiline_Input * c=dynamic_cast<Comment_Multiline_Input *>(w)){
      c->textcolor(Xcas_comment_color);
      c->color(Xcas_comment_background_color);
      return;
    }
    if (Multiline_Input_tab * i=dynamic_cast<Multiline_Input_tab *>(w)){
      i->textcolor(Xcas_input_color);
      i->color(Xcas_input_background_color);
      return;
    }
    if (Equation * eq=dynamic_cast<Equation *>(w)){
      eq->attr.text_color=eq->output_equation?Xcas_equation_color:Xcas_equation_input_color;
      eq->color(eq->output_equation?Xcas_equation_background_color:Xcas_equation_input_background_color);
      return;
    }
    if (Log_Output * lg =dynamic_cast<Log_Output *>(w)){
      lg->textcolor(Xcas_log_color);
      lg->color(Xcas_log_background_color);
      return;
    }
    if (Editeur * ed=dynamic_cast<Editeur *>(w)){
      ed->color(Xcas_editor_background_color);
      return;
    }
    if (Xcas_Text_Editor * ed=dynamic_cast<Xcas_Text_Editor *>(w)){
      ed->color(Xcas_editor_background_color);
      return;
    }
    if (dynamic_cast<Fl_Menu_ *>(w))
      w->color(Xcas_background_color);
  }

  static inline int max(int i,int j){
    if (i>j)
      return i;
    else
      return j;
  }

  static inline int min(int i,int j){
    if (i>j)
      return j;
    else
      return i;
  }

  static inline int abs(int x){
    if (x<0) return -x; else return x;
  }

  History_Pack::History_Pack(int X,int Y,int W,int H,const char*l):Fl_Group(X,Y,W,H,l),_selecting(false),_pushed(false),_moving(false),_saving(false),undo_position(0),_modified(false),_resize_above(false),_spacing(2),_printlevel_w(labelsize()),_sel_begin(-1),_sel_end(-1),pretty_output(1),eval_below(false),doing_eval(false),eval_below_once(true),eval_next(false),queue_pos(-1),update_pos(-1),contextptr(0),
#if 1
								   new_question(new_question_editor),
#else
								   new_question(new_question_multiline_input),
#endif
								   _select(0),_insert(0),url(0),eval(0) {
    box(FL_FLAT_BOX);
    end();
    set_colors(this,false);
    if (parent()){
      labelsize(parent()->labelsize());
      labelfont(parent()->labelfont());
    }
    parent_redraw(this);
  }

  int History_Pack::set_sel_begin(const Fl_Widget * w){
    if (_sel_begin!=-1)
      return _sel_begin;
    _sel_begin=_sel_end=-1;
    for (;w;){
      if (w->parent()==this)
	break;
      w=w->parent();
    }
    if (w)
      return _sel_begin=find(w);
    else
      return -1;
  }

  int History_Pack::focus(const Fl_Widget * w) const {
    for (;w;){
      if (w->parent()==this)
	break;
      w=w->parent();
    }
    if (w)
      return find(w);
    else
      return -1;
  }

  void History_Pack::set_scroller(Fl_Group * gr){
    if (!gr)
      return;
    if (Fl_Scroll * s = dynamic_cast<Fl_Scroll *>(parent())){
      Fl_Widget * ch=gr->child(0);
      int spos=s->yposition();
      int chpos=ch->y()-y();
      int sh=s->h()-10;
      if (chpos<spos)
#ifdef _HAVE_FL_UTF8_HDR_
	s->scroll_to(0,max(0,min(chpos,h()-sh)));
#else
      s->position(0,max(0,min(chpos,h()-sh)));
#endif
      else {
	if (chpos+gr->h()>spos+sh)
#ifdef _HAVE_FL_UTF8_HDR_
	  s->scroll_to(0,chpos+gr->h()-sh);
#else
	s->position(0,chpos+gr->h()-sh);
#endif
	else
#ifdef _HAVE_FL_UTF8_HDR_
	  s->scroll_to(0,spos);
#else
	s->position(0,spos);
#endif
      }
      s->redraw();
    }
  }

  // Focus on history pack level no pos if it's an input
  bool History_Pack::focus(int pos,bool do_focus){
    if (pos>=children() || pos<0)
      pos=children()-1;
    if (pos<0)
      return false;
    Fl_Tile * gr=dynamic_cast<Fl_Tile *>(child(pos));
    if (gr && gr->children()){
      Fl_Widget * ch=gr->child(0);
      if (do_focus){
	if (Fl_Input_ * in=dynamic_cast<Fl_Input_ *>(ch)){
	  in->position(in->position(),in->position());
	}
	if (Fl::focus())
	  Fl::focus()->damage(FL_DAMAGE_ALL);
	Fl::belowmouse(ch);
	Fl::focus(ch);
	ch->handle(FL_FOCUS);
	ch->damage(FL_DAMAGE_ALL);
	ch->redraw();
      }
      // set the scroller if there is one above
      set_scroller(gr);
      if (Editeur * ed = dynamic_cast<Editeur *>(ch)){
	if (do_focus)
	  Fl::focus(ed->editor);
      }
      if (Xcas_Text_Editor * ed = dynamic_cast<Xcas_Text_Editor *>(ch)){
	if (do_focus)
	  Fl::focus(ed);
      }
      return true;
    }
    return false;
  }

  giac::gen warn_equal(const giac::gen & g,GIAC_CONTEXT){
    if (g.is_symb_of_sommet(at_equal) && g._SYMBptr->feuille.type==_VECT && g._SYMBptr->feuille._VECTptr->size()==2 && g._SYMBptr->feuille._VECTptr->front().type!=_INT_){
      if (g._SYMBptr->feuille._VECTptr->front().is_symb_of_sommet(at_at) || g._SYMBptr->feuille._VECTptr->front().is_symb_of_sommet(at_of) || g._SYMBptr->feuille._VECTptr->front().type!=_SYMB)
	*logptr(contextptr) << gettext("Warning evaluating = at top level, you must use := to assign ") << g._SYMBptr->feuille._VECTptr->back() << gettext(" to ") << g._SYMBptr->feuille._VECTptr->front() << gettext(" or == to test equality") << endl;
    }
    return g;
  }

  int parse(Fl_Widget * w,giac::gen & g){
    const context * contextptr=context0;
    const  History_Pack * whp = get_history_pack(w);
    if (whp)
      contextptr = whp->contextptr;
    else
      contextptr=get_context(w);
    if (Multiline_Input_tab * m=dynamic_cast<Multiline_Input_tab *>(w)){
      if (m && !m->changed()){
	g = warn_equal(m->g(),contextptr);
	return !is_undef(g);
      }
      if (!*m->value())
	return 0;
      g=warn_equal(giac::gen(m->value(),contextptr),contextptr);
      if (m)
	m->_g=g;
      if (giac::first_error_line(contextptr))
	*logptr(contextptr) << gettext("Syntax compatibility mode ") << print_program_syntax(xcas_mode(contextptr)) << gettext("\nParse error line ") << giac::first_error_line(contextptr) <<  gettext(" at ")  << giac::error_token_name(contextptr) ;
      return 1;
    }
    if (Equation * e = dynamic_cast<Equation *>(w)){
      e->parse_desactivate();
      e->select();
      g=e->get_data();
      return 1;
    }
    if (Editeur * ed=dynamic_cast<Editeur *>(w))
      return parse(ed->editor,g);
    if (Xcas_Text_Editor * ed=dynamic_cast<Xcas_Text_Editor *>(w)){
      if (ed->buffer()->length()==0)
	return 0;
      if (!ed->changed()){
	g = warn_equal(ed->g(),contextptr);
	return !is_undef(g);
      }
      g=warn_equal(giac::gen(ed->value(),contextptr),contextptr);
      if (ed)
	ed->_g=g;
      if (giac::first_error_line(contextptr))
	*logptr(contextptr) << gettext("Syntax compatibility mode ") << print_program_syntax(xcas_mode(contextptr)) << gettext("\nParse error line ") << giac::first_error_line(contextptr) <<  gettext(" at ")  << giac::error_token_name(contextptr) ;
      return 1;
    }
    if (Figure * f = dynamic_cast<Figure *>(w)){
      if (f->geo && f->geo->hp){
	if (whp)
	  f->geo->hp->eval_next=whp->eval_below;
	f->geo->hp->update();
	return -1;
      }
    }
    if (History_Fold * hf=dynamic_cast<History_Fold *>(w)){
      hf->eval();
      return -1;
    }
    if (History_Pack * hp=dynamic_cast<History_Pack *>(w)){
      hp->update(0);
      return -1;
    }
    const History_Pack * hp = get_history_pack(w);
    if (Tableur_Group * t=dynamic_cast<Tableur_Group *>(w)){
      Flv_Table_Gen * tg = t->table;
      if (hp)
	tg->contextptr=hp->contextptr;
      tg->spread_eval_interrupt();
    }
    return 0;
  }

  std::string History_Pack::value(int n) const {
    int m=children();
    if (n<0 || n>=m)
      return "undef";
    Fl_Widget * wid = child(n);
    if (Fl_Scroll * s=dynamic_cast<Fl_Scroll *>(wid))
      if(s->children())
	wid=s->child(0);
    if (Fl_Group * g=dynamic_cast<Fl_Group *>(wid))
      if (g->children())
	wid=g->child(0);
    if (Fl_Input * i = dynamic_cast<Fl_Input *>(wid))
      return i->value();
    if (Xcas_Text_Editor * i = dynamic_cast<Xcas_Text_Editor *>(wid))
      return i->value();
    if (Equation * e = dynamic_cast<Equation *>(wid)){
      e->parse_desactivate();
      e->select();
      gen g=e->get_data();
      return g.print(contextptr).c_str();
    }
    return "undef";
  }

  giac::gen History_Pack::parse(int n){
    int m=children();
    if (n>=m){
      n=m;
      add_entry(-1);
      ++m;
    }
    Fl_Widget * wid = child(n);
    if (Fl_Scroll * s=dynamic_cast<Fl_Scroll *>(wid))
      if(s->children())
	wid=s->child(0);
    if (Fl_Group * g=dynamic_cast<Fl_Group *>(wid))
      if (g->children())
	wid=g->child(0);
    gen res;
    if (!xcas::parse(wid,res))
      return undef;
    return res;
  }

  bool set_gen_value(Fl_Widget * w,const giac::gen & g,bool exec){
    const giac::context * contextptr = get_context(w);
    if (Xcas_Text_Editor * ed=dynamic_cast<Xcas_Text_Editor *>(w)){
      ed->_g=g;
      bool res=set_value(ed,g.print(contextptr),exec);
      ed->clear_gchanged();
      return res;
    }
    if (Fl_Input_ * i = dynamic_cast<Fl_Input_ *>(w)){
      i->value(g.print(contextptr).c_str());
      i->position(0,0);
      if (Multiline_Input_tab * m = dynamic_cast<Multiline_Input_tab *>(i)){
	m->_g=g;
	i->clear_changed();
      }
      else
	i->set_changed();
      if (exec)
	i->do_callback();
      return true;
    }
    if (Equation * e = dynamic_cast<Equation *>(w)){
      e->parse_desactivate();
      e->select();
      e->set_data(g);
      if (exec)
	e->do_callback();
      return true;
    }
    return false;
  }

  void History_Pack::set_gen_value(int n,const gen & g,bool exec){
    int m=children();
    if (n<0 || n>=m){
      if (value(m-1)!=""){
	n=m;
	add_entry(-1);
	++m;
      }
      else
	n=m-1;
    }
    if (is_context_busy(contextptr) || doing_eval){
      queue_val=g;
      queue_pos=n;
    }
    else {
      Fl_Widget * wid = child(n);
      if (Fl_Scroll * s=dynamic_cast<Fl_Scroll *>(wid))
	if(s->children())
	  wid=s->child(0);
      if (Fl_Group * g=dynamic_cast<Fl_Group *>(wid))
	if (g->children())
	  wid=g->child(0);
      xcas::set_gen_value(wid,g,exec);
    }
  }

  Fl_Group * History_Pack::widget_group(int n){
    int m=children();
    if (n<0 || n>=m)
      return 0;
    Fl_Widget * wid = child(n);
    if (Fl_Scroll * s=dynamic_cast<Fl_Scroll *>(wid))
      if(s->children())
	wid=s->child(0);
    if (Fl_Group * g=dynamic_cast<Fl_Group *>(wid))
      return g;
    return 0;
  }

  bool set_value(Fl_Widget * w,const std::string & s,bool exec){
    context * contextptr=get_context(w);
    if (Fl_Input_ * i = dynamic_cast<Fl_Input_ *>(w)){
      i->value(s.c_str());
      i->position(0,0);
      i->set_changed();
      if (exec)
	i->do_callback();
      return true;
    }
    if (Xcas_Text_Editor * i = dynamic_cast<Xcas_Text_Editor *>(w)){
      if (i->buffer()->length()>0) 
	i->buffer()->remove(0,i->buffer()->length());
      i->buffer()->insert(0,s.c_str());
      i->insert_position(0);
      i->set_gchanged();
      if (exec)
	i->do_callback();
      return true;
    }
    if (Equation * e = dynamic_cast<Equation *>(w)){
      e->parse_desactivate();
      e->select();
      e->set_data(gen(s,contextptr));
      if (exec)
	e->do_callback();
      return true;
    }
    return false;
  }

  void History_Pack::set_value(int n,const std::string & s,bool exec){
    int m=children();
    if (n<0 || n>=m){
      if (value(m-1)!=""){
	n=m;
	add_entry(-1);
	++m;
      }
      else
	n=m-1;
    }
    Fl_Widget * wid = child(n);
    if (Fl_Scroll * s=dynamic_cast<Fl_Scroll *>(wid))
      if(s->children())
	wid=s->child(0);
    if (Fl_Group * g=dynamic_cast<Fl_Group *>(wid))
      if (g->children())
	wid=g->child(0);
    xcas::set_value(wid,s,exec);
  }

  History_Fold * get_history_fold(const Fl_Widget * wid){
    History_Fold * f =0 ;
    if (!wid)
      return f;
    Fl_Group * w=wid->parent();
    for (;w;w=w->parent()){
      if ( (f=dynamic_cast<History_Fold *>(w)) )
	break;
    }
    return f;
  }

  History_Fold * get_history_fold_focus(const Fl_Widget * wid){
    History_Fold * f=get_history_fold(xcas::Xcas_input_focus);
    return f?f:get_history_fold(wid);
  }

  History_Pack * get_history_pack(const Fl_Widget * w,int & pos){
    pos = -1;
    if (!w)
      return 0;
    Fl_Group * g = w->parent();
    if (History_Pack * h = dynamic_cast<History_Pack *>(g)){
      pos=h->find(w);
      return h;
    }
    return get_history_pack(g,pos);
  }

  History_Pack * get_history_pack(const Fl_Widget * w){
    int pos;
    return get_history_pack(w,pos);
  }

  History_Pack * get_history_pack2(const Fl_Widget * w){
    if (!w)
      return 0;
    if (const History_Pack * h = dynamic_cast<const History_Pack *>(w))
      return (History_Pack *) h;
    if (const History_Fold * h = dynamic_cast<const History_Fold *>(w))
      return (History_Pack *) h->pack;
    return get_history_pack2(w->parent());
  }

  context * get_context(const Fl_Widget * w){
    const History_Pack * hp = get_history_pack2(w);
    if (hp)
      return hp->contextptr;
    if (const Flv_Table_Gen * t=dynamic_cast<const Flv_Table_Gen *>(w))
      return t->contextptr;
    if (const Tableur_Group * gr = dynamic_cast<const Tableur_Group *>(w))
      return gr->table->contextptr;
    return 0;
  }

  void set_context(Fl_Widget * w,giac::context * contextptr){
    if (Graph2d3d *gr=dynamic_cast<Graph2d3d *>(w))
      gr->no_handle=false;
    Fl_Group * g=dynamic_cast<Fl_Group *>(w);
    if (!g)
      return;
    if (History_Pack * hp=dynamic_cast<History_Pack *>(g))
      hp->contextptr=contextptr;
    if (Logo * l=dynamic_cast<Logo *>(g))
      l->t->turtleptr=&giac::turtle_stack(contextptr);
    int n=g->children();
    for (int i=0;i<n;++i){
      set_context(g->child(i),contextptr);
    }
  }

  void History_Fold::clear_modified(){
    if (fold_button){
      fold_button->color(Xcas_save_saved_color);
      fold_button->redraw();
    }
    if (save_button){
      save_button->color(Xcas_save_saved_color);
      save_button->redraw();
    }
  }

  void History_Pack::clear_modified(){
    _modified = false ;
    if (History_Fold * f = get_history_fold(this))
      f->clear_modified();
    int n=children();
    for (int i=0;i<n;++i){
      if (History_Fold * f = dynamic_cast<History_Fold *>(child(i)))
	f->clear_modified();
    }
  }

  void History_Pack::restore(int dpos){
    int undo_pos_save=undo_position;
    int us=undo_history.size();
    if (undo_position==us){
      if (dpos>=0)
	return;
      if (dpos==-1){
	backup();
	undo_position -=2;
	++us;
      }
    }
    if (undo_position<=0 && dpos<0)
      return;
    undo_position += dpos;
    if (undo_position<0)
      undo_position=0;
    if (us){
      if (undo_position>=us)
	undo_position=us-1; 
      vector<Fl_Widget *> & cur =undo_history[undo_position];
      int curs=cur.size();
      // delete child widgets that are not inside cur
      int ns=children();
      for (int i=ns-1;i>=0;--i){
	Fl_Widget * wid=child(i);
	int j=0;
	for (;j<curs;++j){
	  if (cur[j]==wid)
	    break;
	}
	if (j==curs){
	  add_history_map(wid,undo_pos_save);
	  remove(wid);
	  delete wid;
	}
	else
	  remove(wid);
      }
      // restore 
      for (int j=0;j<curs;++j){
	Fl_Widget * wid = restore_history_map(cur[j],undo_position);
	if (wid)
	  add(wid);
      }
      resize();
      in_modified();
    }
  }

  void History_Pack::backup(){
    // save widget disposition
    vector<Fl_Widget *> current;
    int n=children();
    // cerr << n << " ";
    for (int i=0;i<n;++i){
      current.push_back(child(i));
      // cerr << child(i) << " ";
    }
    // cerr << endl;
    int us=undo_history.size();
    if (us>undo_position)
      undo_history.erase(undo_history.begin()+undo_position,undo_history.end());
    undo_history.push_back(current);
    ++undo_position;
  }

  void History_Pack::modified(bool do_backup){
    if (do_backup)
      backup();
    in_modified();
  }

  void History_Pack::in_modified(){
    // if (_modified) return;
    _modified = true ;
    if (History_Fold * f = get_history_fold(this)){
      if (f->fold_button){
	f->fold_button->color(Xcas_save_unsaved_color);
	f->fold_button->redraw();
      }
      if (f->save_button){
	f->save_button->color(Xcas_save_unsaved_color);
	f->save_button->redraw();
      }
      f->pack->_modified=true;
      Fl_Widget * w = f->parent();
      if (History_Pack * f=dynamic_cast<History_Pack *>(w)){
	f->modified(false);
	// parent_redraw(f);
      }
      return ;
    }
    Fl_Group * g = parent();
    if (!g)
      return;
    g=parent_skip_scroll(g);
    if (!g)
      return;
    if (History_Pack * f=dynamic_cast<History_Pack *>(g->parent())){
      f->modified(false);
      parent_redraw(f);
      return ;
    }
  }

  void History_Pack::fold_selection(){
    int n=children();
    if (_sel_begin<0 || _sel_end<0 || n<=0 )
      return;
    // Get widget at position _sel_begin to have dimensions
    int s1=min(min(_sel_begin,_sel_end),n-1);
    int s2=min(max(_sel_begin,_sel_end),n-1);
    Fl_Widget * c =child(s1);
    Fl_Group::current(this);
    History_Fold * res = new History_Fold(c->x(),c->y(),c->w(),c->h());
    res->labelsize(labelsize());
    res->labelfont(labelfont());
    change_group_fontsize(res,labelsize());
    res->pack->_select=_select;
    res->pack->_insert=_insert;
    res->pack->new_question=new_question;
    res->pack->_spacing=_spacing;
    res->pack->eval=eval;
    res->pack->_resize_above=true;
    res->pack->contextptr = contextptr;
    // Move _sel_begin to _sel_end to res
    for (int i=s1;i<=s2;++i){
      Fl_Widget * c = child(s1);
      c->resize(c->x(),c->y(),res->pack->w()-res->pack->_printlevel_w,c->h());
      res->pack->add(c);
    }
    insert(*res,s1);
    res->fold();
    _sel_begin=_sel_end=s1;
    modified(true);
    parent_redraw(this);
  }

  void History_Pack::flatten(){
    int n=children();
    if (_sel_begin<0 || _sel_end<0 || n<=0 )
      return;
    // Flatten all history_fold's starting from the last one
    int s1=min(min(_sel_begin,_sel_end),n-1);
    int s2=min(max(_sel_begin,_sel_end),n-1);
    for (int i=s2;i>=s1;--i){
      Fl_Widget * w=child(i);
      if (!w)
	continue;
      History_Fold * f=dynamic_cast<History_Fold *>(w);
      if (!f)
	continue;
      // Copy all childrens
      int m=f->pack->children();
      for (int j=m-1;j>=0;--j){
	insert(*f->pack->child(j),i+1);
      }
      f->autosave_rm();
      add_history_map(w,undo_position);
      remove(*w);
      delete w;
    }
    modified(true);
    _sel_begin=_sel_end=-1;
    parent_redraw(this);
  }

  bool History_Pack::resize(){
    char chaine[25]; // enough even for very large numbers!
    int n=children();
    Fl_Widget ** a = (Fl_Widget **) array();
    sprintf(chaine,"%i",n);
    fl_font(labelfont(),labelsize());
    _printlevel_w=int(fl_width(chaine))+2;
    // Find how much space is required horizontally
    int W=w(),H=0,X=x(),Y=y();
    for (int i=0;i<n;++i){
      Fl_Widget * tmp = *(a+i);
      tmp->labelsize(labelsize());
      if (Fl_Input_ * fi=dynamic_cast<Fl_Input_ *>(tmp))
	fi->textsize(labelsize());
      if (tmp->visible()){
	if (tmp->h()<2)
	  tmp->resize(tmp->x(),tmp->y(),tmp->w(),2);
	H+=absint(tmp->h())+_spacing;
	// W=max(W,tmp->w());
      }
    }
    // H -= _spacing;
    // New horizontal size is therefore _printlevel_w+W
    int newh=min(max(H,1),
#ifdef _HAVE_FL_UTF8_HDR_
		 1<<30
#else
		 1<<14
#endif
		 );
    if (newh<H){
      cerr << "Too many large widgets. Compressing" << endl;
      double ratio=double(newh)/H;
      newh=0;
      int y0=y(),hh;
      for (int i=0;i<n;++i){
	Fl_Widget * tmp = *(a+i);
	hh=int(tmp->h()*ratio);
	tmp->resize(tmp->x(),y0,tmp->w(),hh);
	y0 += hh;
	newh += hh;
      }
    }
    if (newh==h())
      return false;
    int oldh=h();
    if (_resize_above){ // if(newh>oldh)
      // cerr << newh-h() << endl;
      increase_size(this,newh-oldh);
    }
    else
      Fl_Widget::resize(X,Y,W,newh);
    bool modif=false;
    X=x(); Y=y(); W=w()-_printlevel_w; H=h();
    int X1=X+_printlevel_w,Y1=Y;
    // check_fl_rectf(x(),y(),w(),h(),clip_x,clip_y,clip_w,clip_h,0,0);
    for (int i=0;i<n;++i){
      Fl_Widget * tmp = *(a+i);
      if (Fl_Tile * tl = dynamic_cast<Fl_Tile *>(tmp)){
	// check that the first child of tl has non-0 size
	if (tl->children() && tl->child(0)->h()<1){
	  Fl_Widget * tl0=tl->child(0);
	  tl0->resize(tl0->x(),tl0->y(),tl0->w(),1);
	}
      }
      if ( tmp->x()!=X1 || tmp->y()!=Y1 || tmp->w()!=W ){
	tmp->resize(X1,Y1,W,max(absint(tmp->h()),2));
	modif=true;
      }
      if (tmp->visible()){
	Y1 += tmp->h()+_spacing;
      }
    }
    if (modif)
      init_sizes();
    if (!doing_eval)
      parent_redraw(this);
    // cerr << "resize " << this << " " << newh << " " << h() << " " << int(damage()) << endl;
    return true;
  }

  bool History_Pack::resize(int maxh){
    int n=children();
    for (int i=0;i<n;++i){
      Fl_Widget * wid = child(i);
      if (wid->h()>maxh)
	wid->resize(wid->x(),wid->y(),wid->w(),maxh);
    }
    return resize();
  }

  bool History_Pack::resize(int minh,int maxh){
    int n=children();
    for (int i=0;i<n;++i){
      Fl_Widget * wid = child(i);
      if (wid->h()<minh)
	wid->resize(wid->x(),wid->y(),wid->w(),minh);
      if (wid->h()>maxh)
	wid->resize(wid->x(),wid->y(),wid->w(),maxh);
    }
    return resize();
  }

  // This call is required for embedded subwindows to be hidden or shown
  // because they do not use the clipping mechanism of usual widgets
  void hide_show_windows(const Fl_Group * g,int cx,int cy,int cw,int ch){
    int n=g->children();
    Fl_Widget ** a = (Fl_Widget **) g->array();
    for (int i=0;i<n;++i,++a){
      Fl_Widget * tmp = *a;
      if (Fl_Window * win = dynamic_cast<Fl_Window *>(tmp)){
	cerr << win->x()<< " " << cx << " " << win->y()<< " " << cy << " " << win->w() << " " << cw << " " << win->h() << " " << ch << endl; 
	if (win->x()<cx || win->y()<cy || win->x()+win->w() >cx+cw || win->y()+win->h() >cy+ch )
	  win->hide();
	else {
	  win->show();
	  win->redraw();
	}
	continue;
      }
      if (Fl_Group * gr = dynamic_cast<Fl_Group *>(tmp)){
	hide_show_windows(gr,cx,cy,cw,ch);
      }
    }
  }

  void History_Pack::draw(){
    resize();
    int clip_x,clip_y,clip_w,clip_h;
    fl_clip_box(x(),y(),w(),h(),clip_x,clip_y,clip_w,clip_h);
    if (!clip_w || !clip_h)
      return;
    // cerr << this << " " << x() << " " << y() << " " << w() << " " << h() << " " << clip_x << " " << clip_y << " " << clip_w << " " << clip_h << " " << int(damage()) << endl;
    fl_push_clip(clip_x,clip_y,clip_w,clip_h);
    // does not work...
    // hide_show_windows(this,clip_x,clip_y,clip_w,clip_h);
    // if (window()) window()->show();
    fl_font(labelfont(),labelsize());
    char chaine[25]; // enough even for very large numbers!
    int n=children();
    Fl_Widget ** a = (Fl_Widget **) array();
    int X=x(),Y=y(),W=w()-_printlevel_w,H=h(),X1=X+_printlevel_w,Y1=Y;
    // Replace widgets if necessary and redraw spacings
    bool modif=false;
    fl_color(color());
    // check_fl_rectf(x(),y(),w(),h(),clip_x,clip_y,clip_w,clip_h,0,0);
    for (int i=0;i<n;++i){
      Fl_Widget * tmp = *(a+i);
      if ( tmp->x()!=X1 || tmp->y()!=Y1 || tmp->w()!=W ){
	tmp->resize(X1,Y1,W,max(absint(tmp->h()),1));
	modif=true;
	// if (tmp->visible() && parent()) parent()->redraw();
      }
      if (tmp->visible()){
	// tmp->redraw();
	Y1 += tmp->h()+_spacing;
      }
    }
    if (modif){
      // cerr << "init_sizes" << endl;
      init_sizes();
    }
    // Redraw widgets
    Fl_Group::draw();
    // Draw spacings
    // Print history level numbers
    X=x()+1; Y=y(); Y1=Y; W=w()-_printlevel_w;H=h();
    fl_font(labelfont(),labelsize());
    fl_color(color());
    check_fl_rectf(x(),y(),_printlevel_w,H,clip_x,clip_y,clip_w,clip_h,0,0);
    int l=labelsize()+labeladd,m=min(_sel_begin,_sel_end),M=max(_sel_begin,_sel_end);
    for (int i=0;i<n;++i,++a){
      Fl_Widget * tmp = *a;
      if (tmp->visible()){
	sprintf(chaine,"%i",i+1);
	int j=(*a)->y();
	bool inverse ;
	inverse = m>=0 && i>=m && i<=M ;
	if (inverse)
	  fl_color(FL_BLACK);
	else
	  fl_color(FL_WHITE);
	// if (fl_not_clipped(X, j, _printlevel_w, l)){
	  fl_rectf(X,j,_printlevel_w,l);
	  // check_fl_rectf(X,j,_printlevel_w,l,clip_x,clip_y,clip_w,clip_h,0,0);
	  if (inverse)
	    fl_color(FL_WHITE);
	  else
	    fl_color(FL_BLACK);
	  fl_rect(X,j,_printlevel_w,l);
	  // check_fl_rect(X,j,_printlevel_w,l,clip_x,clip_y,clip_w,clip_h,0,0);
	  fl_draw(chaine,X+1,j+l-labeladd/2-1);
	  // check_fl_draw(chaine,X+2,j+l-labeladd/2-1,clip_x,clip_y,clip_w,clip_h,0,0);
	  // }
	// redraw spacing
	if (_spacing){
	  Y1 += (*a)->h();
	  fl_color(FL_BLACK);
	  check_fl_line(X,Y1+1,X+w(),Y1+1,clip_x,clip_y,clip_w,clip_h,0,0);
	  check_fl_line(X,Y1+_spacing-1,X+w(),Y1+_spacing-1,clip_x,clip_y,clip_w,clip_h,0,0);
	  Y1 += _spacing;
	  fl_color(color());
	}
      }
    }
    fl_pop_clip();
  }

  

  // From Fl_Tile.cxx
  void set_cursor(Fl_Widget *t, Fl_Cursor c) {
    static Fl_Cursor cursor;
    if (cursor == c || !t->window()) return;
    cursor = c;
#ifdef __sgi
    t->window()->cursor(c,FL_RED,FL_WHITE);
#else
    t->window()->cursor(c);
#endif
  }
  
  static Fl_Cursor cursors[4] = {
    FL_CURSOR_DEFAULT,
    FL_CURSOR_WE,
    FL_CURSOR_NS,
    FL_CURSOR_MOVE
  };
  
  int History_Pack::handle(int event){
    if (event==FL_MOUSEWHEEL){
#if 1
      if (!Fl::event_inside(this)) return 0;
      return Fl_Group::handle(event); 
      for (unsigned i=0;i<children();i++){
	int res=child(i)->handle(event);
	if (res)
	  return res;
      }
      return 0;
#else
      return 0;
#endif
    }
    if (event==FL_FOCUS){
      return Fl_Group::handle(event);
    }
    if (event==FL_UNFOCUS){
      // _sel_end = _sel_begin = -1; 
      // this prevents middle mouse insertion
      return Fl_Group::handle(event);
    }
    if (event==FL_KEYBOARD){
      if (Fl::event_alt() && Fl::event_key()){
	if (Fl_Window * w=window()){
	  int n=w->children();
	  for (int i=0;i<n;++i){
	    if (Fl_Menu_Bar * mb=dynamic_cast<Fl_Menu_Bar *>(w->child(i)))
	      return mb->handle(FL_SHORTCUT);
	  }
	}	
      }
      if (Fl::event_key()==FL_BackSpace || Fl::event_key()==FL_Delete){
	remove_selected_levels(false);
	return 1;
      }
      char ch=Fl::event_text()?Fl::event_text()[0]:0;
      if (ch==17 && !children()){ // Ctrl-Q -> search for a main menu shortcut
	if (Fl_Window * w=window()){
	  int n=w->children();
	  for (int i=0;i<n;++i){
	    if (Fl_Menu_Bar * mb=dynamic_cast<Fl_Menu_Bar *>(w->child(i)))
	      return mb->handle(FL_SHORTCUT);
	  }
	}
      }
      if (ch==3 || ch==24){ // Ctrl-C
	if (_select){
	  const char * ch=_select(this,_sel_begin,_sel_end);
	  Fl::copy(ch,strlen(ch),1);
	}
	if (ch==24)
	  remove_selected_levels(false);
	return 1;
      }
      if (ch==22){ // Ctrl-V
	Fl::paste(*this, 1);
	return 1;
      }
      if (ch==25){
	History_cb_Redo(this,0);
	return 1;
      }
      if (ch==26){
	History_cb_Undo(this,0);
	return 1;
      }
    }
    if (Fl::event_button()== FL_MIDDLE_MOUSE ){
      if (event==FL_RELEASE){
	Fl::paste(*this);
	_selecting = _moving = _pushed = false;
	return 1;      
      }
    }
    if (event==FL_PASTE){
      if (!_insert)
	return 0;
      _insert(this,Fl::event_text(),-1,_sel_begin);
      _sel_begin = _sel_end = -1;
      update(); // commented
      parent_redraw(this);
      return 1;
    }
    int mx = Fl::event_x();
    int my = Fl::event_y();
    if (event==FL_DRAG){
      if (Fl_Scroll * s=dynamic_cast<Fl_Scroll *>(parent())){
	int mousey=my-y(),Y=s->yposition(),dh=h()-s->h();
	if (mousey>Y+s->h() && dh>Y)
#ifdef _HAVE_FL_UTF8_HDR_
	  s->scroll_to(s->xposition(),min(mousey-s->h(),dh));
#else
	  s->position(s->xposition(),min(mousey-s->h(),dh));
#endif
	if (mousey<Y && Y>0)
#ifdef _HAVE_FL_UTF8_HDR_
	  s->scroll_to(s->xposition(),max(0,mousey));
#else
	  s->position(s->xposition(),max(0,mousey));
#endif
      }
    }
    int n=children();
    if (!n)
      return 0;
    Fl_Widget*const* a = array();
    if (_selecting){
      switch(event){
      case FL_RELEASE:
	_selecting = false;
	_moving = _pushed = false;
      case FL_DRAG:
	if (!Fl::event_state(FL_SHIFT)){
	  int old_sel_end=_sel_end;
	  if (my<(*a)->y()){
	    _sel_end=0;
	  }
	  else {
	    int i;
	    for (i=0; i<n;i++ ) {
	      Fl_Widget* o = *(a+i);
	      int yh=o->y();
	      if ( my >= yh && my < yh + o->h() + _spacing )
		break;
	    }
	    _sel_end=i;
	  }
	  if (old_sel_end!=_sel_end)
	    damage(1);
	}
	if (_select && event==FL_RELEASE){
	  const char * ch=_select(this,_sel_begin,_sel_end);
	  if (ch) Fl::selection(*this,ch,strlen(ch));
	}
	break;
      } // end switch
      return 1;
    }
    if (_moving){
      if (event==FL_RELEASE){
	_selecting = false;
	_moving = _pushed = false;
	// Check if final position is in current History_Pack
	if (mx>=x() && mx <= x()+w()){
	  int i;
	  for (i=0; i<n;i++ ) {
	    Fl_Widget* o = *(a+i);
	    int yh=o->y();
	    bool truehf=false;
	    if (History_Fold * hf=dynamic_cast<History_Fold*>(o))
	      truehf=hf->pack->_resize_above; // hf->group->visible() doesn't work
	    if ( my >= yh && my < yh + o->h() && !truehf){
	      // Move _sel_begin to _sel_end after i (if i>_sel_end)
	      // before i (if i<_sel_begin)
	      set_cursor(this, FL_CURSOR_ARROW);
	      int m=min(_sel_begin,_sel_end),M=max(_sel_begin,_sel_end);
	      if (i<=M && i>=m) return 1;
	      if (i>M){
		for (;M>=m;--M)
		  insert(*child(m),i+1);
	      }
	      else {
		for (;m<=M;++m)
		  insert(*child(M),i);
	      }
	      update();
	      _sel_end = _sel_begin=-1;
	      modified(true);
	      redraw();
	      return 1;
	    }
	  } // end for
	} // end if mx...
	// try to find an History_Pack above mx,my
	// using parent() to go up and child() to go down
	Fl_Widget * o = this;
	for (;o;){
	  if (mx>=o->x() && mx<=o->x()+o->w() && my>=o->y() && my<=o->y()+o->h())
	    break;
	  o=o->parent();
	}
	if (!o)
	  return 1;
	// release on a tab?
	if (Fl_Tabs * tab=dynamic_cast<Fl_Tabs *>(o)){
	  o=tab->which(mx,my);
	  if (History_Fold * hf=dynamic_cast<History_Fold *>(o)){
	    hf->pack->_sel_begin=-1;
	    Fl::paste(*hf->pack);
	    return 1;
	  }
	}
	// go down to find an History_Pack with 
	Fl_Group * g = dynamic_cast<Fl_Group *>(o);
	o=0;
	History_Pack * h=dynamic_cast<History_Pack *>(g);
	bool set_i=h;
	int i_=-1;
	for (;g;){
	  int n=g->children(),i;
	  for (i=0;i<n;++i){
	    // Stop at first children found
	    o=g->child(i);
	    if (mx>=o->x() && mx<=o->x()+o->w() && my>=o->y() && my<=o->y()+o->h())
	      break;
	  }
	  // No children found, stop the search
	  if (i==n)
	    break;
	  if (set_i)
	    i_=i;
	  if (History_Pack * hp=dynamic_cast<History_Pack *>(o)){
	    h=hp;
	    set_i=true;
	  }
	  else {
	    if (History_Fold * hf=dynamic_cast<History_Fold *>(o)){
	      h=hf->pack;
	      set_i=true;
	    }
	    else
	      set_i=false;
	  }
	  // Not an history pack, try again
	  g=dynamic_cast<Fl_Group *>(o);
	}
	if (h && h!=this){
	  h->_sel_begin=i_;
	  Fl::paste(*h);
	}
	return 1;
      } // end if event==FL_RELEASE
      return 1;
    } // end if (_moving)
    if (!_pushed && mx>=x() && mx<x()+_printlevel_w-1){
      if (event==FL_MOVE){
	for (int i=0; i<n;i++ ) {
	  Fl_Widget* o = *(a+i);
	  int yh=o->y();
	  if ( my >= yh && my < yh + o->h() ) {
	    if (my<yh+labelsize())
	      set_cursor(this,FL_CURSOR_HAND);
	    else
	      set_cursor(this,FL_CURSOR_ARROW);
	    return 1;
	  }
	}
      }
      if (event==FL_PUSH){
	if (Fl::focus()!=this)
	  Fl::focus(this);
	_selecting = _moving = _pushed = false;
	if (Fl::event_button()== FL_MIDDLE_MOUSE ){
	  for (int i=0; i<n;i++ ) {
	    Fl_Widget* o = *(a+i);
	    int yh=o->y();
	    if ( my >= yh && my < yh + o->h() ) {
	      _sel_begin = _sel_end = i;
	    }
	  }
	  return 1;
	}
	_selecting=true;
	for (int i=0; i<n;i++ ) {
	  Fl_Widget* o = *(a+i);
	  int yh=o->y();
	  if ( my >= yh && my < yh + o->h() ) {
	    // If i is between _sel_begin and _sel_end, move
	    int m=min(_sel_begin,_sel_end),M=max(_sel_begin,_sel_end);
	    if (m==-1 && M ==-1 && my<yh+labelsize())
	      m = M = _sel_begin = _sel_end = i;
	    if (m>-1 && m<=i && i<=M){
	      _moving=true;
	      _selecting = false;
	      if (_select){
		const char * ch=_select(this,_sel_begin,_sel_end);
		if (ch) Fl::selection(*this,ch,strlen(ch));
	      }
	      set_cursor(this,FL_CURSOR_MOVE);
	    }
	    else {
	      if(Fl::event_state(FL_SHIFT)){
		if (i<m){
		  _sel_begin=i;
		  _sel_end=M;
		}
		else {
		  if (i>M){
		    _sel_begin=m;
		    _sel_end=i;
		  }
		}
	      }
	      else {
		_sel_begin=i;
		_sel_end=i;
	      }
	    }
	    damage(1);
	    return 1;
	  } // end if (my >= yh && my < yh + o->h())
	}
      }
      set_cursor(this, FL_CURSOR_ARROW);
      return 1;
    }
    else {
      switch (event) {
      case FL_MOVE:
      case FL_ENTER:
      case FL_PUSH: 
	{
	  _moving = _selecting = false;
	  _pushed = event==FL_PUSH;
	  if (_pushed){
	    _sel_end = _sel_begin = -1;
	    damage(1);
	  }
	  for (int i=0; i<n;i++ ) {
	    Fl_Widget* o = *(a+i);
	    int yh=o->y()+o->h();
	    if ( yh <= my+GRABAREA && yh >= my-GRABAREA ) {
	      _my_push=my;
	      _widget_i=i;
	      _widget_h=o->h();
	      if (_pushed) modified(true);
	      set_cursor(this, cursors[2]);
	      return 1;
	    }
	  }
	  _pushed = false;
	  set_cursor(this, FL_CURSOR_DEFAULT);
	  return Fl_Group::handle(event);
	}
      case FL_LEAVE:
	set_cursor(this, FL_CURSOR_DEFAULT);
	_moving = _pushed = false;
	break;
      case FL_DRAG:
      case FL_RELEASE: 
	if (_pushed){
	  Fl_Widget * o = child(_widget_i);
	  HScroll * hs = dynamic_cast<HScroll *>(parent());
	  int oldh=o->h();
	  int newh=max(_widget_h+my-_my_push,10);
	  if (hs && hs->resizechildren && _widget_i+1<children()){
	    Fl_Widget * nexto=child(_widget_i+1);
	    int nextnewh=nexto->h()+oldh-newh;
	    if (nextnewh<10){
	      newh -= (10-nextnewh);
	      nextnewh=10;
	    }
	    nexto->resize(o->x(),o->y()+o->h(),nexto->w(),nextnewh);
	  }
	  if (!hs || (hs->resizechildren && _widget_i+1<children() )){
	    o->resize(o->x(),o->y(),o->w(),max(newh,1));
	    parent_redraw(this);
	  }
	  _pushed = event!=FL_RELEASE ;
	  _moving = _selecting = false ;
	  return 1;
	}
      } // end switch
    } // end if !_pushed && mx<labelsize()
    return Fl_Group::handle(event);
  }

  bool History_Pack::save(const char * ch){
    if (!_modified && !url)
      return false;
    if (url)
      return save_as(url->c_str(),ch,true,true,file_save_context);
    else
      return save_as(ch);
  }

  bool History_Fold::autosave(bool warn_user){
    if (!pack->_modified || (input && input->visible()))
      return false;
    if (debug_infolevel)
      cerr << "Autosaving " << autosave_filename << endl;
    bool res=pack->save_as(autosave_filename.c_str(),0,false,warn_user,false);
    pack->modified(false);
    return res;
  }

  int confirm_close(const string & message){
    int i=fl_choice("%s",gettext("Cancel"),gettext("Yes"),gettext("No"),message.c_str());
    return i;
  }

  bool History_Pack::close(const char * ch){
    if (_modified){
      int i=confirm_close(string(ch)+gettext(" modified. Save?"));
      if (!i)
	return false;
      if (i==1){
	if (!save(ch))
	  return false;
      }
    }
    clear();
    Fl_Group * w =parent();
    if (w){
      w->remove(this);
      parent_redraw(w);
    }
    if (logstream){      
      delete logstream;
      logstream=0;
    }
    return true;
  }

  bool History_Pack::save_all(){
    int n=children();
    for (int i=0;i<n;++i){
      if (History_Fold * f=dynamic_cast<History_Fold *>(child(i))){
	if (f->pack->_modified){
	  char chaine[25]; // enough even for very large numbers!
	  sprintf(chaine,"%i",i+1);
	  string name="session "+string(chaine)+" ";
	  if (f->pack->url)
	    name += *f->pack->url ;
	  int j=confirm_close((name+gettext(" has changed. Save?")).c_str());
	  if (j==0)
	    return false;
	  if (j!=2)
	    f->pack->save(name.c_str());
	  else
	    f->pack->clear_modified();
	  f->autosave_rm();
	}
      }
    }
    return true;
  }

#ifdef WIN32
  std::string unix_path(const std::string & winpath){
#ifdef __x86_64__
    int s = cygwin_conv_path (CCP_WIN_W_TO_POSIX , winpath.c_str(), NULL, 0);
    char * unixpath = (char *) malloc(s);
    cygwin_conv_path(CCP_WIN_W_TO_POSIX,winpath.c_str(), unixpath,s);
#else
    char * unixpath = (char *) malloc (cygwin_win32_to_posix_path_list_buf_size (winpath.c_str()));
    cygwin_win32_to_posix_path_list (winpath.c_str(), unixpath);
#endif
    string res=unixpath;
    free(unixpath);
    return res;
  }
#else

  std::string unix_path(const std::string & winpath){
    return winpath;
  }

#endif

  void History_Pack::new_url(const char * newfile){
    if (url)
      delete url;
    string newf=newfile;
    char buf[1024];
    if (getcwd(buf,1023)){
      string newf1=remove_path(newfile);
      if (buf+('/'+newf1)==newf)
	newf=newf1;
    }
    url = new string(unix_path(newf));
    if (History_Fold * hf=get_history_fold(this)){
      hf->label(url->c_str());
      if (hf->parent())
	hf->parent()->redraw();
    }
  }

  bool History_Pack::save_as(const char * ch){
    if (!_select)
      return false;
    Fl_Widget * wid = xcas::Xcas_input_focus;
    // get filename
    string tmp="Save worksheet ";
    if (ch)
      tmp += ch;
    string fname;
    if (url)
      fname=remove_path(remove_extension(*url))+"_.xws";
    else
      fname="session.xws";
    char * newfile = file_chooser(tmp.c_str(), "*.xws", fname.c_str());
    if (wid)
      Fl::focus(wid);
    // check filename
    if ( !newfile )
      return false;
#if 1 // ndef WIN32
    string sn=get_path(unix_path(newfile));
    if (!sn.empty()){
#ifndef HAVE_NO_CWD
      // _cd(string2gen(sn,false),contextptr);
      chdir(sn.c_str());
#endif
    }
#endif
    string s=remove_extension(newfile)+".xws";
    if (is_file_available(s.c_str())){
      int i=fl_ask("%s",("File "+s+" exists. Overwrite?").c_str());
      if ( !i )
	return false;
    }
    // Save filename
    new_url(s.c_str());
    return save_as(s.c_str(),ch,true,true,file_save_context);
  }

  bool History_Pack::save_as(const char * filename,const char * ch,bool autosave_rm,bool warn_user,bool savecontext){
    if (!filename || !_select || _saving)
      return false;
    const char * chs=_select(this,0,children()-1);
    if (!chs)
      return false;
    _saving = true;
    FILE * f(fopen(filename,"w"));
    if (!f){ 
      string message=string(gettext("Unable to open file "))+filename;
      if (warn_user)
	fl_alert("%s",message.c_str());
      cerr << message << endl;
      if (url){
	delete url;
	url=0;
      }
      if (History_Fold * hf=get_history_fold(this)){
	hf->label(gettext("Unnamed"));
      }
      _saving = false;
      return false; 
    }
    if (recent_filenames_menu)
      add_recent_filename(filename,true);
    int savepos=_sel_begin,pos,yscrollerpos;
    Fl_Scroll * scroller = dynamic_cast<Fl_Scroll *>(parent());
    if (scroller)
      yscrollerpos=scroller->yposition();
    pos=focus(Xcas_input_focus);
    if (pos==-1)
      pos=_sel_begin;
    bool usepos=false,dbg=false;
    if (xcas::Xcas_Debug_Window && xcas::Xcas_Debug_Window->shown())
      dbg=true;
    if (pos<0)
      pos=update_pos;
    else 
      usepos=(dynamic_cast<Equation *>(Xcas_input_focus)==0);
    _sel_begin=savepos;
    fprintf(f,"// xcas version=%s fontsize=%i font=%i currentlevel=%i\n",VERSION,labelsize(),labelfont(),pos);
    fprintf(f,"%s",chs);
    if (savecontext){
      string tmps=archive_session(false,contextptr);
      fprintf(f,"// context ");
      int L=tmps.size();
      fprintf(f,"%i ",L);
      for (int i=0;i<L;++i){
	fprintf(f,"%c",tmps[i]);
      }
      fprintf(f,"\n");
    }
    fclose(f);
    _saving = false;
    clear_modified();
    if (autosave_rm){
      if (History_Fold * hf=dynamic_cast<History_Fold *>(parent_skip_scroll(this)))
	hf->autosave_rm();
    }
    if (!usepos && Xcas_input_focus){
      Fl::focus(Xcas_input_focus);
      if (Fl_Input_ * ptr=dynamic_cast<Fl_Input_*>(Xcas_input_focus))
	ptr->position(ptr->position(),ptr->position());
    }
    else {
      focus(pos,true);
      if (Fl_Input_ * ptr=dynamic_cast<Fl_Input_*>(Fl::focus()))
	ptr->position(ptr->position(),ptr->position());
    }
    if (dbg)
      Fl::focus(xcas::Xcas_Debug_Window);
    if (scroller){
#ifdef _HAVE_FL_UTF8_HDR_
      scroller->scroll_to(0,yscrollerpos);
#else
      scroller->position(0,yscrollerpos);
#endif
    }
    return true;
  }

  int count(const char * ch,char c){
    int res=0;
    for (;*ch;++ch){
      if (*ch==c)
	++res;
    }
    return res;
  }

  // Maple worksheet translate, returns new y position
  int mws2xws(istream & inf,ostream & of,int x,int y,int w,int h){
    string mapletxt;
    while (inf && of && !inf.eof()){
      inf >> mapletxt;
      int n1,n2,n3;
      n1=mapletxt.size();
      if (n1>7 && mapletxt.substr(n1-7,7)=="MPLTEXT"){
        inf >> n1 >> n2 >> n3; // n1=1? n2=0? n3=length
#ifdef HAVE_SSTREAM
	ostringstream os;
#else
	ostrstream os;
#endif
	in_mws_translate(inf,os);
	int dh=5+h*(1+count(string(os.str()).c_str(),'\n'));
	of << "// fltk 7Fl_Tile " << x << " " << y << " "<< w << " " << dh << endl;
	of << "[" << endl;
	of << "// fltk maple_Multiline_Input_tab "<< x << " " << y << " "<< w << " " << dh << endl;
	y += dh;
	of << replace(os.str(),'\n','£');
	of << "\n,\n]\n";
      }
      else {
	if ( (n1>4 && mapletxt.substr(n1-4,4)=="TEXT") || (n1>7 && mapletxt.substr(n1-7,7)=="XPPEDIT") ){
	  inf >> n1 >> n2;
#ifdef HAVE_SSTREAM
	  ostringstream os;
#else
	  ostrstream os;
#endif
	  in_mws_translate(inf,os);
	  int dh=5+h*(1+count(string(os.str()).c_str(),'\n'));
	  of << "// fltk 7Fl_Tile " << x << " " << y << " "<< w << " " << dh << endl;
	  of << "[" << endl;
	  of << "// fltk maple_Multiline_Input "<< x << " " << y << " "<< w << " " << dh << endl;
	  y += dh;
	  of << replace(os.str(),'\n','£');
	  of << "\n,\n]\n";
	}
      }
    }
    return y;
  }

  bool History_Pack::insert_before(int before_position,bool newurl,int mws){
    if (!_insert)
      return false;
    // get filename
    const char * newfile ;
    switch (mws){
    case 1:
      newfile = load_file_chooser("Load maple worksheet","*.mws","*.mws",0,false);
      break;
    case 3:
      newfile = load_file_chooser("Load ti89 program","*.89*","*.89*",0,false);
      break;
    case 7:
      newfile = load_file_chooser("Load V200 program","*.v2*","*.v2*",0,false);
      break;
    default:
      newfile = load_file_chooser("Load worksheet", "*.xws", "session.xws",0,false);
    }
    // check filename
    if ( !newfile )
      return false;
    if (!is_file_available(newfile)){
      fl_message("%s",("File "+string(newfile)+" does not exist.").c_str());
      return false;
    }
    if (mws==3 || mws==7){
      if (xcas_mode(contextptr)!=3){
	int i=fl_ask("%s",gettext("Set compatibility mode to TI?"));
	if (i)
	  xcas_mode(contextptr)=3;
      }
      gen tmp=_unarchive_ti(string2gen(newfile,false),contextptr);
      if (is_undef(tmp))
	return false;
      // create 
      newfile=(remove_extension(newfile)+".xws").c_str();
      if (newurl) 
	new_url(newfile);
      string xcasti=remove_extension(newfile)+".ti";
      ofstream out(xcasti.c_str());
      out << tmp << endl;
      out.close();
      History_Fold * o = get_history_fold(this);
      if (o){
	o->pack->set_sel_begin(xcas::Xcas_input_focus);
	int pos=o->pack->_sel_begin;
	Fl_Widget * e=new_program(max(o->pack->w()-o->pack->_printlevel_w,1),o->h()/2,o->pack);
	change_group_fontsize(e,o->labelsize());
	o->pack->add_entry(pos,e);
	if (Editeur * ed=dynamic_cast<Editeur *>(e)){
	  ed->editor->buffer()->insertfile(xcasti.c_str(),ed->editor->insert_position());
	  ed->editor->label(xcasti.c_str());
	  ed->output->value(remove_path(xcasti).c_str());
	  ed->output->redraw();
	  Fl::focus(ed->editor);
	}
      }
      return true;
    }
    if (mws==1){
      // Translate to xcas worksheet
      string xcasws=remove_extension(newfile)+".xws";
      if (is_file_available(xcasws.c_str())){
	int i=fl_ask("%s",(gettext("File ")+xcasws+gettext(" exists. Overwrite?")).c_str());
	if (!i)
	  return false;
      }
      if (xcas_mode(contextptr)!=mws){
	fl_message("%s",gettext("Translating worksheet and setting compatibility mode to maple"));
	xcas_mode(contextptr)=mws;
      }
      ifstream in(newfile);
      ofstream out(xcasws.c_str());
      mws2xws(in,out,49,87,626,labelsize()+1);
      in.close();
      out.close();
      newfile = xcasws.c_str();
    }
    if (newurl) 
      new_url(newfile);
    return insert_url(newfile,before_position);
  }

  bool History_Pack::insert_url(const char * urlname0,int before_position){
    if (!urlname0 || !_insert )
      return false;
    string urlname = unix_path(urlname0).c_str();
#if 1 // ndef WIN32
    string sn=get_path(urlname);
    if (!sn.empty()){
#ifndef HAVE_NO_CWD
      //_cd(string2gen(sn,false),contextptr);
      chdir(sn.c_str());
#endif
    }
#endif
    FILE * f=fopen(urlname.c_str(),"r");
    if (!f){
      fl_message("%s",("Unable to load "+urlname).c_str());
      return false;
    }
    string s;
    char c;
    while (1){
      c=fgetc(f);
      if (feof(f))
	break;
      s += c;
    }
    fclose(f);
    unsigned ss=s.size();
    bool exec=false;
    int fontsize=labelsize();
    int newpos=-1;
    Fl_Font police=FL_HELVETICA;
    if (ss>7 && s.substr(0,8)=="// xcas "){
      unsigned pos=s.find('\n');
      if (pos>=0 && pos<ss){
	string options=s.substr(8,pos-8);
	// remaining session
	s=s.substr(pos+1,ss-pos-1);
	// parse options
	pos=0;
	unsigned optsize=options.size();
	string optname,optvalue;
	bool afterequal=false;
	for (;pos<optsize;++pos){
	  if (options[pos]=='='){
	    optvalue="";
	    afterequal=true;
	    continue;
	  }
	  if (options[pos]==' ' || pos==optsize-1){
	    if (pos==optsize-1)
	      optvalue += options[pos];
	    if (afterequal && !optname.empty() && !optvalue.empty()){
	      if (optname=="exec")
		exec = (optvalue=="true");
	      if (optname=="fontsize")
		fontsize = atoi(optvalue.c_str());
	      if (optname=="font")
		police = Fl_Font(atoi(optvalue.c_str()));
	      if (optname=="xcas_mode")
		xcas_mode(contextptr)=atoi(optvalue.c_str());
	      if (optname=="currentlevel"){
		newpos=atoi(optvalue.c_str());
		if (before_position>0)
		  newpos += before_position;
	      }
	    }
	    optname="";
	    afterequal=false;
	  }
	  else {
	    if (afterequal)
	      optvalue += options[pos];
	    else
	      optname +=  options[pos];
	  }
	}
      }
    }
    if (ss>8  && s.substr(0,8)=="{VERSION"){
      // Maple translate
#ifdef HAVE_SSTREAM
      istringstream in(s);
      ostringstream out;
#else
      istrstream in(s.c_str());
      ostrstream out;
#endif
      mws2xws(in,out,49,87,626,labelsize()+1);
      s=out.str();
      if (xcas_mode(contextptr)!=1){
	fl_message("%s",gettext("Translating maple worksheet and setting compatibility mode to maple"));
	xcas_mode(contextptr)=1;
      }
      new_url((remove_extension(urlname)+".xws").c_str());
    }
    bool res= _insert(this,s.c_str(),s.size(),before_position);
    if (exec){
      Fl_Group * g = dynamic_cast<Fl_Group *>(child(0));
      if (g && !dynamic_cast<Figure *>(g) && g->children()){
	eval_below=true;
	History_Pack_cb_eval(g->child(0),0);
      }
    }
    focus(newpos,true);
    labelfont(police);
    change_group_fontsize(this,max(8,fontsize));
    return res;
  }

  int editor_hsize(Fl_Widget * w){
    if (!w)
      return 0;
    return w->labelsize()+12;
    Fl_Group * wd = w->parent();
    while (wd && wd->parent())
      wd=wd->parent();
    if (wd && w->w()<wd->w()*0.7)
      return w->labelsize()+24;
    else
      return w->labelsize()+12;
  }

  bool History_Pack::add_entry(int n){
    if (!new_question)
      return false;
    Fl_Widget * q=new_question(max(w()-_printlevel_w,1),editor_hsize(this));
    add_entry(n,q);
    return true;
  }


  void History_Pack::add_entry(int n,Fl_Widget * q){ 
    q->labelsize(labelsize());
    q->labelfont(labelfont());
    if (Fl_Input_ * i=dynamic_cast<Fl_Input_ *>(q)){
      i->textsize(labelsize());
      i->textfont(labelfont());
    }
    if (Xcas_Text_Editor * ed=dynamic_cast<Xcas_Text_Editor *>(q)){
      ed->Fl_Text_Display::textsize(labelsize());
      vector<Fl_Text_Display::Style_Table_Entry> & v=ed->styletable;
      for (unsigned i=0;i<v.size();++i)
	v[i].size=labelsize();
    }
    if (q->w()>w()-_printlevel_w)
      q->resize(q->x(),q->y(),w()-_printlevel_w,q->h());
    int addh=1;
    Fl_Group::current(0);
    Fl_Tile * g = new Fl_Tile(q->x(),q->y(),q->w(),q->h()+addh);
    g->labelfont(q->labelfont());
    g->end();
    g->add(q);
    Log_Output * lg = new Log_Output(q->x(),q->y()+q->h(),q->w(),addh);
    lg->labelsize(labelsize());
    lg->labelfont(labelfont());
    lg->textsize(labelsize());
    lg->textfont(labelfont());
    lg->textcolor(Xcas_log_color);
    lg->color(Xcas_log_background_color);
    g->add(lg);
    g->resizable(q); 
    add_entry(n,g);
    focus(n,true);
  }

  void History_Pack::add_entry(int n,Fl_Group * g){
    add_history_map(g,undo_position);
    int hh=h()+_spacing+g->h();
    if (_resize_above)
      increase_size(this,_spacing+g->h());
    int m=children();
    int ynew=y();
    if (n==-1 || n>=m)
      ynew += h();
    else
      ynew = child(n)->y();
    Fl_Widget::resize(x(),y(),w(),hh);
    g->resize(x(),ynew,min(w(),g->w()),g->h());
    if (n==-1 || n>=m)
      add(g);
    else 
      insert(*g,n);
    modified(true);
    resize();
    Fl_Widget * p= parent();
    if (dynamic_cast<HScroll *>(p)) // this will resize children horizontally
      p->resize(p->x(),p->y(),p->w(),p->h());
    parent_redraw(this);
    redraw();
  }

  Fl_Widget * new_question_multiline_input(int W,int H){
    Fl_Group::current(0);
    Multiline_Input_tab * w=new Multiline_Input_tab(0,0,W,H);
    w->when(FL_WHEN_ENTER_KEY|FL_WHEN_NOT_CHANGED);
    w->callback(History_Pack_cb_eval,0);
    w->textcolor(Xcas_input_color);
    w->color(Xcas_input_background_color);
    return w;
  }

  Fl_Widget * new_question_editor(int W,int H){
    Fl_Group::current(0);
    Fl_Text_Buffer * b = new Fl_Text_Buffer;
    Xcas_Text_Editor * w=new Xcas_Text_Editor(0,0,W,H,b);
    w->scrollbar_width(12);
    w->when(FL_WHEN_ENTER_KEY|FL_WHEN_NOT_CHANGED);
    w->callback(History_Pack_cb_eval,0);
    w->textcolor(Xcas_input_color);
    w->color(Xcas_input_background_color);
    w->buffer()->add_modify_callback(style_update, w); 
    return w;
  }

  Fl_Widget * new_question_equation(int W,int H){
    Fl_Group::current(0);
    Equation * w=new Equation(0,0,W,H,"",0);
    w->output_equation=false;
    w->select();
    w->cb_enter=History_Pack_cb_eval;
    w->attr.text_color=Xcas_equation_input_color;
    w->color(Xcas_equation_input_background_color);
    return w;
  }

  void Comment_cb_eval(Fl_Multiline_Input * q , void*){
    if(!Fl::event_shift()){
      // focus on next input
      History_Pack * hp=get_history_pack(q);
      if (hp){
	hp->_sel_begin=-1;
	hp->set_sel_begin(xcas::Xcas_input_focus);
	if (hp->_sel_begin<hp->children()-1){
	  Fl_Widget * w=hp->child(hp->_sel_begin+1);
	  if (Fl_Group * g=dynamic_cast<Fl_Group *>(w)){
	    if (g->children()){
	      w=g->child(0);
	      if ( (g=dynamic_cast<Fl_Group *>(w)) ){ 
		if (g->children())
		  w=g->child(0);
	      }
	      Fl::focus(w);
	      if (Fl_Input_ * i=dynamic_cast<Fl_Input_ *>(w)){
		int is=strlen(i->value());
		i->position(is,is);
	      }
	    }
	  }
	}
      }
    }
  }

  Fl_Widget * new_comment_input(int W,int H){
    Fl_Group::current(0);
    Comment_Multiline_Input * w=new Comment_Multiline_Input(0,0,W,H,"");
    w->textcolor(Xcas_comment_color);
    w->color(Xcas_comment_background_color);
    w->callback((Fl_Callback *) Comment_cb_eval);
    w->when(FL_WHEN_ENTER_KEY|FL_WHEN_NOT_CHANGED);
    return w;
  }

  Fl_Widget * new_tableur(int W,int H,const History_Pack * pack){
    Fl_Group::current(0);
    Tableur_Group * t = new Tableur_Group(0,0,W,H,pack->labelsize(),0);
    t->labelfont(pack->labelfont());
    t->table->labelfont(pack->labelfont());
    if (t && t->table)
      Fl::focus(t->table);
    set_colors(t);
    return t;
  }

  Fl_Widget * new_program(int W,int H,const History_Pack * pack){
    Fl_Group::current((Fl_Group *)pack);
    Editeur * t = new Editeur(0,0,W,H);
    t->labelfont(pack->labelfont());
    t->callback(History_Pack_cb_eval,0);
    set_colors(t);
    return t;
  }

  Fl_Group * new_figure(int W,int H,const History_Pack * pack,bool dim3,bool approx){
    Fl_Group::current(0);
    Figure * t = new Figure(0,0,W,H,pack->labelsize(),dim3);
    t->labelfont(pack->labelfont());
    t->geo->hp->labelfont(pack->labelfont());
    t->geo->approx=approx;
    t->geo->hp->contextptr=pack->contextptr;
    if (dim3)
      t->lt->line_type(_POINT_WIDTH_5);
    set_colors(t);
    return t;
  }

  Fl_Group * new_logo(int W,int H,const History_Pack * pack){
    Fl_Group::current(0);
    Logo * t = new Logo(0,0,W,H,pack->labelsize());
    t->hp->contextptr=pack->contextptr;
    t->t->turtleptr=&giac::turtle_stack(pack->contextptr);
    turtle(pack->contextptr).widget = t->t;
    set_colors(t);
    return t;
  }

  Log_Output * find_log_output(Fl_Group * g){
    if (!g)
      return 0;
    Log_Output * otmp=0;
    int m=g->children();
    for (int i=0;i<m;++i){
      if ( (otmp=dynamic_cast<Log_Output *>(g->child(i))) )
	break;
    }
    return otmp;
  }

  void History_Fold::eval(){
    if (pack->children()){
      pack->eval_below=true;
      const History_Pack * hp=get_history_pack(this);
      if (hp)
	pack->eval_next=hp->eval_below;
      Fl_Group * g = dynamic_cast<Fl_Group *>(pack->child(0));
      if (History_Fold * hf=dynamic_cast<History_Fold *>(g))
	hf->eval();
      else
	if (g && !dynamic_cast<Figure *>(g) && g->children())
	  History_Pack_cb_eval(g->child(0),0);
    }
  }

  const int logptrinitlines=4;

  void History_Pack_Group_eval(Fl_Group * g,bool add_group){
    if (History_Fold * hf=dynamic_cast<History_Fold *>(g)){
      hf->eval();
      return;
    }
    int m=g->children();
    if (!m)
      return;
    Fl_Widget * q = g->child(0);
    History_Pack * p = dynamic_cast<History_Pack * >(g->parent());
    if (!p) return;
    // add an entry if none are below or focus on next entry
    int N=p->find(g),M=p->children();
    // Search in g->children if there is a Log_Output available
    if (Xcas_DispG)
      xcas_dispg_entries=Xcas_DispG->plot_instructions.size();
    Log_Output * otmp=find_log_output(g);
    int deltalog=0;
    if (!otmp){
      Fl_Group::current(0);
      otmp = new Log_Output(q->x(),q->y()+q->h(),q->w(),logptrinitlines*q->labelsize()+14);
      deltalog=otmp->h();
      g->Fl_Widget::resize(g->x(),g->y(),g->w(),g->h()+deltalog);
      otmp->labelsize(q->labelsize());
      otmp->labelfont(q->labelfont());
      otmp->textsize(q->labelsize());
      otmp->textfont(q->labelfont());
      otmp->textcolor(Xcas_log_color);
      g->insert(*otmp,1);
      ++m;
    }
    else {
      deltalog=otmp->h();
      otmp->value("");
      // resize otmp
      otmp->resize(otmp->x(),otmp->y(),otmp->w(),logptrinitlines*otmp->labelsize()+14);
      deltalog = otmp->h()-deltalog;
      g->Fl_Widget::resize(g->x(),g->y(),g->w(),g->h()+deltalog);
      g->insert(*otmp,1);
    }
    for (int i=g->find(otmp)+1;i<g->children();i++){
      Fl_Widget * wtmp=g->child(i);
      wtmp->resize(wtmp->x(),wtmp->y()+deltalog,wtmp->w(),wtmp->h());
    }
    if (logstream)      
      delete logstream;
    logstream = new owstream(otmp,p->contextptr) ;
    logptr(logstream,p->contextptr);
    // p->modified(false);
    g->resizable(NULL);
    Fl_Widget * a, * b=q;
    if (Fl_Scroll * s = dynamic_cast<Fl_Scroll *>(q))
      b = s->child(0);
    a=p->eval(b);
    if (a==0 && g->children()==3){
      Fl_Widget * tmp =g->child(2);
      int tmph=tmp->h();
      g->remove(tmp);
      delete tmp;
      g->Fl_Widget::resize(g->x(),g->y(),g->w(),g->h()-tmph);
      g->resizable(g);
    }
    if (a==0 || a==b) { // a==0 or a==b: no output
      output_resize_parent(otmp);
      // add_group=false;
      p->doing_eval=false;
      if (!a && p->eval_below) // if a==b, widget is an eval-terminal widget
	p->next(N);
      return;
    }
    // bool computing=false;
    if (Fl_Output * out =dynamic_cast<Fl_Output *>(a)){
      if (strcmp(out->value(),gettext("Unable to launch thread. Press STOP to interrupt."))==0)
	output_resize_parent(otmp);
    } 
    p->resize();
    int ah=0;
    if (m>2){
      ah = g->child(2)->h(); 
      Fl_Widget * wid=g->child(2);
      g->remove(wid);
      if (!dynamic_cast<Gen_Value_Slider *>(wid)){
	delete wid; // must not delete a slider because we are in it's callback
      }
    }
    a->resize(q->x(),otmp->y()+otmp->h(),g->w(),a->h());
    g->Fl_Widget::resize(g->x(),g->y(),g->w(),g->h()+a->h()-ah);
    g->insert(*a,2);
    g->resizable(g);
    bool fin_pack=N>=M-1;
    if (!fin_pack){ // if next level is not a multiline_input do like at end
      Fl_Widget * tmp = p->child(N+1);
      if (Fl_Tile * tmps = dynamic_cast<Fl_Tile *>(tmp)){
	if (tmps->children())
	  tmp=tmps->child(0);
      }
      if (!dynamic_cast<Fl_Input *>(tmp) && !dynamic_cast<Xcas_Text_Editor*>(tmp))
	fin_pack =true;
    }
    if (fin_pack){
      if (add_group){
	p->add_entry(N+1);
	if (p->children()>N+1)
	  p->child(N+1)->resize(p->child(N)->x(),p->child(N)->y()+p->child(N)->h()+p->_spacing,p->child(N+1)->w(),p->child(N+1)->h());
      }
    }
    else {
      Fl_Widget * tmp = p->child(N+1);
      Fl_Group * tmpg = dynamic_cast<Fl_Group *>(tmp);
      if (tmpg && tmpg->children()){
	Fl::focus(tmpg->child(0));
	if (Fl_Input * i = dynamic_cast<Fl_Input *>(tmpg->child(0))){
	  /* int is=strlen(i->value()); */
	  i->position(0,0); // was i->position(is,is);
	}
      }
    }
    Fl_Scroll * s = dynamic_cast<Fl_Scroll * > (p->parent());
    if (s){
      N=min(N,p->children()-1);
      if (p->child(N)->y()+p->child(N)->h()-p->y()>s->yposition()+s->h()-2*p->labelsize()){
	// int s1=s->yposition()+p->child(N)->h();
	int s1=p->child(N)->y()-p->y(); // begin of evaluated widget
	if (p->child(N)->h()>s->h()-2*p->labelsize()){ // evaluated widget too large
	  s1=p->child(N+1)->y()+2*p->labelsize()-p->y()-s->h();
	}
	s1=min(p->h()-s->h(),s1);
#ifdef _HAVE_FL_UTF8_HDR_
	s->scroll_to(0,max(0,s1));
#else
	s->position(0,max(0,s1));
#endif
	if (!p->doing_eval && s->parent())
	  s->parent()->redraw();
      }
    }
    p->_sel_begin=N+1;
    p->_sel_end=-1;
    p->redraw();
  }

  Fl_Group * parent_skip_scroll(const Fl_Widget * g){
    if (!g)
      return 0;
    Fl_Group * res = g->parent();
    if (res){
      if (Fl_Scroll * s = dynamic_cast<Fl_Scroll *>(res))
	res=s->parent();
    }
    return res;
  }

  void History_Pack_cb_eval(Fl_Widget * q , void*){
    Fl_Group * g = q->parent();
    if (g){ // Check if the widget is embedded in a Fl_Scroll
      if (Fl_Scroll * s=dynamic_cast<Fl_Scroll *>(g)){
	g=s->parent();
	q=s;
      }
    }
    History_Pack * hp = dynamic_cast<History_Pack *>(g->parent());
    if (!hp || hp->doing_eval)
      return;
    if (hp->eval_below){
      hp->doing_eval=true;
      io_graph(false,hp->contextptr);
    }
    History_Pack_Group_eval(g,true);
    hp->doing_eval=false;
    io_graph(true,hp->contextptr);
    /*
#ifdef HAVE_LIBPTHREAD
    // If this a concurrent call to a thread_eval in the same context
    // we do nothing
    if (is_context_busy(hp->contextptr)){
      *logptr(hp->contextptr) << "Thread is busy. Try again later." << endl;
      return;
    }
#endif
    if (hp && hp->eval_below) {
      hp->doing_eval=true;
      io_graph(false,hp->contextptr);
      int n=hp->children();
      int i=hp->find(g);
      int pos=i;
      for (;i<n;++i){
	Fl_Group * tmp = dynamic_cast<Fl_Group *>(hp->child(i));
	History_Pack_Group_eval(tmp,i==n-1);
      }
      // Check for a Geometry child in parent, update it
      Fl_Group * hpp = parent_skip_scroll(hp);
      if (hpp){
	int N=hpp->children();
	for (int i=0;i<N;++i){
	  Graph2d3d * geo = dynamic_cast<Graph2d3d *>(hpp->child(i));
	  if (geo)
	    geo->update(hp,pos);
	}
      }
      io_graph(true,hp->contextptr);
      hp->doing_eval=false;
    }
    else {
      if (g && !g->find(q) )
	History_Pack_Group_eval(g,true);
    }
    */
  }

  void History_cb_Save(Fl_Widget* m , void*) {
    if (m && m->parent()){
      History_Fold * o = get_history_fold(m);
      if (o)
	o->pack->save();
    }
  }

  bool History_Fold::close(){
    if (pack){
      if (!pack->close("Buffer"))
	return false;
    }
    autosave_rm();
    Fl_Group * g = parent();
    Fl::focus(g);
    clear();
    if (g){
      g->remove(this);
      parent_redraw(g);
      g = g->parent();
      if (HScroll * hs=dynamic_cast<HScroll *>(g))
	hs->resize();
    }
    return true;
  }

  void History_cb_Kill(Fl_Widget * m , void*) {
    if (m && m->parent()){
      History_Fold * o = get_history_fold(m);
      if (o) o->close();
    }
  }

  void hf_Kill(Fl_Button * m , void*) {
    if (m && m->parent()){
      History_Fold * o = get_history_fold(m);
      if (o && o->parent()){ 
	Fl::focus(o->parent());
	o->close();
      }
    }
  }

  void History_cb_Save_as(Fl_Widget* m , void*) {
    if (m && m->parent()){
      History_Fold * o = get_history_fold(m);
      if (o)
	o->pack->save_as(0);
    }
  }

  string mode2extension(int mode){
    switch (mode){
    case 1:
      return "map";
    case 2:
      return "mu";
    case 3:
      return "ti";
    default:
      return "cas";
    }
  }

  void save_as_text(ostream & of,int mode,History_Pack * pack){
    const giac::context * contextptr=pack?pack->contextptr:context0;
    int save_maple_mode=xcas_mode(contextptr);
    int n=pack->children();
    for (int i=0;i<n;i++){
      if (!of)
	break;
      Fl_Widget * wid=pack->child(i);
      if (History_Fold * hf=dynamic_cast<History_Fold *>(wid)){
	save_as_text(of,mode,hf->pack);
	continue;
      }
      Fl_Group * g;
      while ( (g=dynamic_cast<Fl_Group *>(wid)) ){ 
	if (Figure * fig=dynamic_cast<Figure *>(g))
	  break;
	if (Editeur * ed=dynamic_cast<Editeur *>(wid))
	  break;
	if (Xcas_Text_Editor * ed=dynamic_cast<Xcas_Text_Editor *>(wid))
	  break;
	if (g->children())
	  wid=g->child(0);
	else
	  break;
      }
      if (Editeur * ed=dynamic_cast<Editeur *>(wid)){
	xcas_mode(contextptr)=mode;
	of << unlocalize(ed->value()) << endl;
	xcas_mode(contextptr)=save_maple_mode;
      }
      if (Xcas_Text_Editor * xed=dynamic_cast<Xcas_Text_Editor *>(wid)){
	xcas_mode(contextptr)=mode;
	of << unlocalize(xed->value()) << endl;
	xcas_mode(contextptr)=save_maple_mode;
      }
      if (Figure * fig=dynamic_cast<Figure *>(g)){
	save_as_text(of,mode,fig->geo->hp);
	continue;
      }
      if (Comment_Multiline_Input * co=dynamic_cast<Comment_Multiline_Input *>(wid))
	of << (mode==1?"++ ":"/* ") << co->value() << (mode==1?" ++":" */") << endl;
      if (Multiline_Input_tab * mi=dynamic_cast<Multiline_Input_tab *>(wid)){
	if (strlen(mi->value())){
	  gen tmp=mi->g();
	  xcas_mode(contextptr)=mode;
	  of << tmp.print(contextptr) << " ;" << endl;
	  xcas_mode(contextptr)=save_maple_mode;
	}
      }
      if (Equation * eq=dynamic_cast<Equation *>(wid)){
	xcas_mode(contextptr)=mode;
	of << eq->get_data().print(contextptr) << " ;" << endl;
	xcas_mode(contextptr)=save_maple_mode;
      }
    }
  }

  void History_cb_save_as_text(Fl_Widget * m,int mode){
    if (m && m->parent()){
      History_Fold * o = get_history_fold(m);
      if (o){
	string tmp=o->pack->url?remove_extension(*o->pack->url):"session";
	tmp += "."+mode2extension(mode);
	char * newfile = file_chooser(gettext("Export worksheet as"), ("*."+mode2extension(mode)).c_str(), tmp.c_str());
	// check filename
	if ( !newfile )
	  return ;
	ofstream of(newfile);
	save_as_text(of,mode,o->pack);
      }
    }
  }

  void History_cb_Save_as_xcas_text(Fl_Widget* m , void*) {
    History_cb_save_as_text(m,0);
  }

  void History_cb_Save_as_maple_text(Fl_Widget* m , void*) {
    History_cb_save_as_text(m,1);
  }

  void History_cb_Save_as_mupad_text(Fl_Widget* m , void*) {
    History_cb_save_as_text(m,2);
  }

  void History_cb_Save_as_ti_text(Fl_Widget* m , void*) {
    History_cb_save_as_text(m,3);
  }

  void History_cb_Insert(Fl_Widget* m , void*) {
    if (m && m->parent()){
      History_Fold * o = get_history_fold(m);
      if (o){
	o->pack->insert_before(o->pack->_sel_begin);
      }
    }
  }

  void History_cb_Load(Fl_Widget* m , void*) {
    if (m && m->parent()){
      History_Fold * o = get_history_fold(m);
      if (o){
	if (o->pack->_modified){
	  int i=confirm_close(gettext("Current buffer modified. Save?"));
	  if (i==0)
	    return;
	  if (i!=2)
	    if (!o->pack->save())
	      return;
	}
	// Clear current pack
	o->pack->clear();
	if (o->pack->insert_before(-1,true))
	  o->pack->clear_modified();
	else
	  o->close();
      }
    }
  }

  void History_cb_Fold(Fl_Widget* m , void*) {
    if (m ){
      History_Fold * o = get_history_fold_focus(m);
      if (o)
	o->pack->fold_selection();
    }
  }

  void History_cb_Merge(Fl_Widget* m , void*) {
    if (m && m->parent()){
      History_Pack * hp = dynamic_cast<History_Pack *>(Fl::focus());
      if (!hp)
	hp=get_history_pack(xcas::Xcas_input_focus);
      if (hp){
	int a=hp->_sel_begin,b= hp->_sel_end;
	if (a>=0 && b>=0){
	  string s;
	  int m=min(a,b),M=max(a,b);
	  int w=hp->child(min(m,hp->children()-1))->w();
	  for (int i=m;i<=M;++i){
	    s += hp->value(m);
	    if (!s.empty() && s[s.size()-1]!=';')
	      s += ';';
	    if (i!=M)
	      s += '\n';
	    Fl_Widget * wid= hp->child(m);
	    hp->add_history_map(wid,hp->undo_position);
	    hp->remove(wid);
	    delete wid;
	  }
	  Multiline_Input_tab * mi = dynamic_cast<Multiline_Input_tab *>(new_question_multiline_input(w,(3+hp->labelsize())*(M-m+1)));
	  hp->add_entry(m,mi);
	  mi->value(s.c_str());
	  mi->set_changed();
	  hp->_sel_begin=hp->_sel_end=m;
	  parent_redraw(hp);
	}
      }
    }
  }

  void History_cb_Flatten(Fl_Widget* m , void*) {
    if (m && m->parent()){
      History_Fold * o = get_history_fold_focus(m);
      if (o)
	o->pack->flatten();
    }
  }

  void cb_New_Input(Fl_Widget* m , void*) {
    Fl_Widget * w = xcas::Xcas_input_focus;
    if (dynamic_cast<Xcas_Text_Editor *>(w) || !dynamic_cast<Fl_Group *>(w))
      w=parent_skip_scroll(parent_skip_scroll(w));
    History_Pack * hp=dynamic_cast<History_Pack *>(w);
    History_Fold * o = get_history_fold_focus(m);
    if (hp && o && !get_history_pack(hp->parent()))
      hp=0;
    if (!hp && m && m->parent()){
      if (o)
	hp=o->pack;
    }
    if (hp){
      // hp->_sel_begin=-1;
      hp->set_sel_begin(xcas::Xcas_input_focus);
      hp->add_entry(hp->_sel_begin);
    }
  }

  void History_cb_New_Xcas_Text_Editor(Fl_Widget* m , void*) {
    if (m && m->parent()){
      History_Fold * o = get_history_fold_focus(m);
      if (o){
	o->pack->set_sel_begin(xcas::Xcas_input_focus);
	Fl_Widget * e=new_question_editor(max(o->pack->w()-o->pack->_printlevel_w,1),editor_hsize(o->pack));
	if (Xcas_Text_Editor * ed=dynamic_cast<Xcas_Text_Editor *>(e)){
	  ed->Fl_Text_Display::textsize(o->labelsize());
	  vector<Fl_Text_Display::Style_Table_Entry> & v=ed->styletable;
	  for (unsigned i=0;i<v.size();++i)
	    v[i].size=o->labelsize();
	}
	change_group_fontsize(e,o->labelsize());
	o->pack->add_entry(o->pack->_sel_begin,e);
	if (Keyboard_Switch)
	  Keyboard_Switch(0x8000 | 0x1);
      }
    }
  }

  void History_cb_New_Equation(Fl_Widget* m , void*) {
    if (m && m->parent()){
      History_Fold * o = get_history_fold_focus(m);
      if (o){
	o->pack->set_sel_begin(xcas::Xcas_input_focus);
	Fl_Widget * e=new_question_equation(max(o->pack->w()-o->pack->_printlevel_w,1),o->pack->labelsize()+10);
	change_group_fontsize(e,o->labelsize());
	if (Equation * eq=dynamic_cast<Equation *>(e))
	  eq->select();
	o->pack->add_entry(o->pack->_sel_begin,e);
	if (Keyboard_Switch)
	  Keyboard_Switch(0x8000 | 0x1);
      }
    }
  }

  void new_tableur(Fl_Widget * m,bool load){
    if (m && m->parent()){
      History_Fold * o = get_history_fold_focus(m);
      if (o){
	o->pack->set_sel_begin(xcas::Xcas_input_focus);
	int pos=o->pack->_sel_begin;
	Fl_Widget * e=new_tableur(max(o->pack->w()-o->pack->_printlevel_w,1),max(o->h()-4*o->labelsize(),200),o->pack);
	change_group_fontsize(e,o->labelsize());
	o->pack->add_entry(pos,e);
	Tableur_Group * t =dynamic_cast<Tableur_Group *>(e);
	if (t){
	  context * contextptr=get_context(o);
	  if (load){
	    string s(tableur_insert(t->table));
	    if (!s.empty()){
	      gen tmp(s,contextptr);
	      if (tmp.type==_IDNT){
		t->table->name=tmp;
		if (t->table->filename)
		  delete t->table->filename;
		if ( (t->table->filename = new string(tmp.print(contextptr)+".tab"))){
		  t->fname->label(t->table->filename->c_str());
		  t->fname->redraw();
		}
		t->table->update_status();
	      }
	    }
	  }
	  else
	    if (!t->table->filename)
	      t->table->config();
	  t->table->row(0);
	  t->table->col(0);
	  t->table->_goto->value("A0");
	  if (t->table->rows() && t->table->cols()){
	    gen tmp=t->table->m[0][0];
	    if (tmp.type==_VECT && tmp._VECTptr->size()==3)
	      tmp=tmp._VECTptr->front();
	    t->table->input->value(tmp.print(contextptr).c_str(),true);
	  }
	}
      }
    }
  }

  void History_cb_New_Tableur(Fl_Widget* m , void*) {
    new_tableur(m,false);
  }

  void History_cb_Insert_Tableur(Fl_Widget* m , void*) {
    new_tableur(m,true);
  }

  void History_cb_New_Comment_Input(Fl_Widget* m , void*) {
    if (m && m->parent()){
      History_Fold * o = get_history_fold_focus(m);
      if (o){
	o->pack->set_sel_begin(xcas::Xcas_input_focus);
	Fl_Widget * e=new_comment_input(max(o->pack->w()-o->pack->_printlevel_w,1),o->labelsize()+10);
	change_group_fontsize(e,o->labelsize());
	o->pack->add_entry(o->pack->_sel_begin,e);
      }
    }
  }

  void new_program(Fl_Widget * m, bool load){
    if (m && m->parent()){
      History_Fold * o = get_history_fold_focus(m);
      if (o){
	o->pack->set_sel_begin(xcas::Xcas_input_focus);
	int pos=o->pack->_sel_begin;
	Fl_Widget * e=new_program(max(o->pack->w()-o->pack->_printlevel_w,1),o->h()/2,o->pack);
	change_group_fontsize(e,o->labelsize());
	o->pack->add_entry(pos,e);
	if (Editeur * ed=dynamic_cast<Editeur *>(e)){
	  if (load){
	    string s=editeur_load(ed->editor);
	    if (!s.empty()){
	      ed->editor->label(s.c_str());
	      ed->output->value(remove_path(s).c_str());
	      ed->output->redraw();
	    }
	  }
	  else
	    cb_choose_func(ed->editor);
	  // else ed->editor->buffer()->insert(0,"\n:;");
	  Fl::focus(ed->editor);
	}
      }
    }
  }

  void History_cb_New_Program(Fl_Widget* m , void*) {
    new_program(m,false);
  }

  void History_cb_Insert_Program(Fl_Widget* m , void*) {
    new_program(m,true);
  }

  void new_figure(Fl_Widget * m,bool load,bool dim3,bool approx=true){
    if (m && m->parent()){
      History_Fold * o = get_history_fold_focus(m);
      if (o){
	o->pack->set_sel_begin(xcas::Xcas_input_focus);
	int pos=o->pack->_sel_begin;
	Fl_Widget * e=new_figure(max(o->pack->w()-o->pack->_printlevel_w,1),max(4*o->h()/5,340),o->pack,dim3,approx);
	change_group_fontsize(e,o->labelsize());
	o->pack->add_entry(pos,e);	
	if (Figure * f=dynamic_cast<Figure *>(e)){
	  if (load){
	    string s=figure_insert(f);
	    f->rename(remove_path(s));
	  }
	  else {
	    // f->geo->set_mode(0,0,1);
	    // f->mode->value("point");
	  }
	}
      }
    }
  }

  void History_cb_New_Figure(Fl_Widget* m , void*) {
    new_figure(m,false,false,true);
  }

  void History_cb_New_Figurex(Fl_Widget* m , void*) {
    new_figure(m,false,false,false);
  }

  void History_cb_Insert_Figure(Fl_Widget* m , void*) {
    new_figure(m,true,false,true);
  }

  void History_cb_New_Figure3d(Fl_Widget* m , void*) {
    new_figure(m,false,true,true);
  }

  void History_cb_New_Figure3dx(Fl_Widget* m , void*) {
    new_figure(m,false,true,false);
  }

  void History_cb_New_Logo(Fl_Widget* m , void*) {
    if (m && m->parent()){
      History_Fold * o = get_history_fold_focus(m);
      if (o){
	o->pack->set_sel_begin(xcas::Xcas_input_focus);
	int pos=o->pack->_sel_begin;
	Fl_Widget * e=new_logo(max(o->pack->w()-o->pack->_printlevel_w,1),2*o->h()/3,o->pack);
	change_group_fontsize(e,o->labelsize());
	o->pack->add_entry(pos,e);
      }
    }
  }

  void History_cb_New_HF(Fl_Widget* m , void*) {
    if (m && m->parent()){
      History_Fold * o = get_history_fold_focus(m);
      if (o){
	o->pack->set_sel_begin(xcas::Xcas_input_focus);
	int pos=o->pack->_sel_begin;
	Fl_Group::current(o->pack);
	History_Fold * e=new History_Fold(0,0,max(o->pack->w()-o->pack->_printlevel_w,1),4*o->labelsize());
	if (e){
	  e->pack->_select=o->pack->_select;
	  e->pack->_insert=o->pack->_insert;
	  e->pack->new_question=o->pack->new_question;
	  e->pack->_spacing=o->pack->_spacing;
	  e->pack->eval=o->pack->eval;
	  e->pack->add_entry(-1);
	  e->pack->_resize_above=true;
	  e->pack->contextptr=o->pack->contextptr;
	  change_group_fontsize(e,o->labelsize());
	  o->pack->add_entry(pos,e);
	}
      }
    }
  }

  void cb_Delete(Fl_Widget* m , void*) {
    History_Pack * hp=0;
    History_Fold * o=0;
    if ( (hp=dynamic_cast<History_Pack *>(Fl::focus())) )
      ;
    else
      if (m && m->parent()){
	o = get_history_fold_focus(m);
	if (o)
	  hp = o->pack;
      }
    if (hp)
      hp->remove_selected_levels(false);
  }

  void History_Pack::update(int n){
    if (n<0)
      n=0;
    if (eval_below && n<children()){
      Fl_Group * hp0g = dynamic_cast<Fl_Group *>(child(n));
      if (History_Fold * hf=dynamic_cast<History_Fold *>(hp0g)){
	hf->eval();
	return;
      }
      if (Logo * l=dynamic_cast<Logo *>(hp0g)){
	if (l->hp){
	  l->hp->eval_below=true;
	  l->hp->eval_next=true;
	  l->hp->update(0);
	}
	return;
      }
      if (Figure * f=dynamic_cast<Figure *>(hp0g))
	f->geo->hp->eval_next=true;
      if (hp0g && hp0g->children())
	History_Pack_cb_eval(hp0g->child(0),0);
    }
  }

  void History_Pack::next(int hp_pos){
    int hp_n=children();
    ++hp_pos;
    if (hp_pos<hp_n)
      update(hp_pos);
    else {
      // Check for a Graphic child in parent, update it
      Fl_Group * hpp = parent_skip_scroll(this);
      bool nextup=eval_next;
      if (hpp){
	int N=hpp->children();
	for (int i=0;i<N;++i){
	  Graph2d3d * geo = dynamic_cast<Graph2d3d *>(hpp->child(i));
	  if (geo){
	    geo->update(this,update_pos>=0?update_pos:0);
	    geo->no_handle=false;
	    if (update_pos>=0 && update_pos<children())
	      set_scroller(dynamic_cast<Fl_Group *>(child(update_pos)));
	  }
	}
      }
      if (eval_below_once)
	eval_below=false;
      if (eval_below ){
	if (queue_pos>=0 && queue_pos<hp_n){
	  update_pos=queue_pos;
	  queue_pos=-1;
	  set_gen_value(update_pos,queue_val,true);
	  nextup=false;
	}
      }
      if (nextup){
	eval_next=false;
	// check if parent history_pack recomputes
	int posup;
	History_Pack * hpup= get_history_pack(hpp,posup);
	if (hpup)
	  hpup->next(posup);
      }
    }
  }

  void History_cb_Run_Worksheet(Fl_Widget* m , void*) {
    if (m && m->parent()){
      History_Fold * o = get_history_fold(m);
      if (o) o->eval();
    }
  }

  void History_cb_Run_Below(Fl_Widget* m , void*) {
    if (m && m->parent()){
      History_Fold * o = get_history_fold_focus(m);
      if (o ){
	int n=o->pack->children(),i=n;
	o->pack->eval_below=true;
	Fl_Widget * w =xcas::Xcas_input_focus;
	for (;w;){
	  i=o->pack->find(w);
	  if (i!=n)
	    break;
	  w=w->parent();
	}
	if (i<n){
	  Fl_Group * g = dynamic_cast<Fl_Group *>(o->pack->child(i));
	  if (g && !dynamic_cast<Figure *>(g) && g->children())
	    History_Pack_cb_eval(g->child(0),0);
	}
      }
    }
  }

  // Model callback function (not used!)
  void History_cb_(Fl_Widget* m , void*) {
    if (m && m->parent()){
      History_Fold * o = get_history_fold_focus(m);
      if (o ){
      }
    }
  }

  void cb_Rm_Answers(History_Fold * o,int i){
    if (!o)
      return;
    int n=o->pack->children();
    for (;i<n;++i){
      Fl_Group * g = dynamic_cast<Fl_Group *>(o->pack->child(i));
      if (History_Fold * hf=dynamic_cast<History_Fold *>(g)){
	cb_Rm_Answers(hf,0);
	continue;
      }
      int c=g->children();
      bool is_input=dynamic_cast<Multiline_Input_tab *>(g->child(0));
      bool is_equation=dynamic_cast<Equation *>(g->child(0));
      bool is_editor=dynamic_cast<Xcas_Text_Editor *>(g->child(0));
      if (g && c>=2 && (is_input || is_equation || is_editor)){
	for (int j=c-1;j;--j)
	  g->remove(g->child(j));
	g->resize(g->x(),g->y(),g->w(),g->child(0)->h());
	o->redraw();
      }
    }    
  }

  void cb_Rm_Answers(Fl_Widget* m , void*) {
    if (m && m->parent()){
      History_Fold * o = get_history_fold_focus(m);
      if (o){
	int n=o->pack->children(),i=n;
	Fl_Widget * w =xcas::Xcas_input_focus;
	for (;w;){
	  if (w->parent()==o->pack){
	    i=o->pack->find(w);
	    break;
	  }
	  w=w->parent();
	}
	cb_Rm_Answers(o,i);
      }
    }
  }

  void xdvi(const std::string & name){
    string s=remove_extension(name);
#ifdef WIN32
    system((xcasroot()+"latex.bat "+s).c_str());
    cerr << xcasroot()+"latex.bat "+s << endl;
    system((xcasroot()+"xdvi.bat "+s+" &").c_str());
    cerr << xcasroot()+"xdvi.bat "+s+" &" << endl;
#else
    string path=get_path(s);
    s=remove_path(s);
    if (path.empty()){
      char buf[1024];
      path=getcwd(buf,1024);
    }
    int tmpresult=system(("cd "+path+" ; latex "+s+" ").c_str());
    tmpresult=system(("cd "+path+" ; xdvi "+s+" &").c_str());
    tmpresult=system(("cd "+path+" ; dvips "+s+" -o "+s+".ps && pstopnm -stdout "+s+" | pnmtopng > "+s+".png").c_str());
    fl_message("%s",("->"+s+".tex/.dvi/.ps/.png").c_str());
#endif //    
  }
  
  void dvips(const std::string & name){
#ifdef WIN32
    system((xcasroot()+"latex.bat "+remove_extension(name)).c_str());
    system((xcasroot()+"dvips.bat "+remove_extension(name)+" &").c_str());
#else
    string s=remove_extension(name);
    string path=get_path(s);
    s=remove_path(s);
    int tmpresult=system(("cd "+path+" ; latex "+s+" ").c_str());
    tmpresult=system(("cd "+path+" ; dvips "+s+" -o "+s+".ps && pstopnm -stdout "+s+" | pnmtopng > "+s+".png").c_str());
#endif //    
  }

  void file2stream(const char * filename,ostream & texof){
    // copy f to the current stream and remove
    FILE * f=fopen(filename,"r");
    for (;f;){
      char ch=fgetc(f);
      if (feof(f))
	break;
      texof << ch;
    }
    fclose(f);
    unlink(filename);
  }

  void historypack2tex(Fl_Widget * wid,int level,ostream & texof,History_Pack * pack,const std::string & name,int & number){
    // const giac::context * contextptr = get_context(wid);
    static unsigned count=0;
    if (dynamic_cast<Figure *>(wid))
      texof << "\\\\" << endl;
    if (Fl_Group * g = dynamic_cast<Fl_Group *>(wid)){
      if (!g || !g->children())
	return;
      int jmax=g->children();
      for (int j=0;j<jmax;j++){
	Fl_Widget * wid = g->child(j);
	historypack2tex(wid,level,texof,pack,name,number);
      }
    }
    if (Multiline_Input_tab * i =dynamic_cast<Multiline_Input_tab *>(wid)){
      texof << "{\\tt " << translate_underscore(i->value()) << " } \\\\" << endl;
      return;
    }
    if (Editeur * ed=dynamic_cast<Editeur *>(wid)){
      texof << endl << "\\begin{verbatim}"<<endl;
      texof << ed->value() << endl;
      texof  << "\\end{verbatim}" << endl;
      return;
    }
    if (Xcas_Text_Editor * ed=dynamic_cast<Xcas_Text_Editor *>(wid)){
      texof << "{\\tt " << translate_underscore(ed->value()) << " } \\\\" << endl;
      return;
    }
    if (const Flv_Table_Gen * t = dynamic_cast<const Flv_Table_Gen *>(wid)){
      texof << "\n\\noindent\n" <<spread2tex(t->m,0,pack->contextptr) << endl ; // formule
      texof << "\n\\noindent\n" << spread2tex(t->m,1,pack->contextptr) << endl << endl ; // formule
      return ;
    }
    if (Equation * eq=dynamic_cast<Equation *>(wid)){
      texof << "\\begin{equation} \\label{eq:" << level << "}" << endl;
      texof << gen2tex(eq->get_data(),pack->contextptr) ;
      texof << "\n\\end{equation}" << endl;
      return;
    }
    if (Fl_Input * i = dynamic_cast<Fl_Input *>(wid)){
      string s=i->value();
      if (s.empty() || (s.size()>4 && s.substr(0,4)=="pnt("))
	return;
      texof << "{\\em " << translate_underscore(s) << "\\/} \\\\\n";
      return;
    }
    if (Graph2d * g=dynamic_cast<Graph2d *>(wid)){
      double xunit=giac::horiz_latex/(g->window_xmax-g->window_xmin);
      double yunit=giac::vert_latex/(g->window_ymax-g->window_ymin);
      string filename="#g2"+print_INT_(count);
      ++count;
      FILE * f=fopen(filename.c_str(),"w");
      if (f){
	graph2tex(f,g->plot_instructions,g->window_xmin,g->window_xmax,g->window_ymin,g->window_ymax,xunit,yunit,filename.c_str(),false,get_context(g));
	fclose(f);
	file2stream(filename.c_str(),texof); // copy f to the current stream
      }
      return;
    }
    if (Graph3d * g=dynamic_cast<Graph3d *>(wid)){
      /*
      double xunit=giac::horiz_latex/(g->window_xmax-g->window_xmin);
      double yunit=giac::vert_latex/(g->window_ymax-g->window_ymin);
      string filename="#g3"+print_INT_(unsigned(g));
      FILE * f=fopen(filename.c_str(),"w");
      if (f){
	int gw=g->w();
	g->resize(g->x(),g->y(),400,g->h());
	fprintf(f,"\n\n{\n\\setlength{\\unitlength}{1pt}\n\\begin{picture}(%d,%d)(%d,%d)\n{\\special{\"\n",g->w(),g->h(),0,0);
	g->printing=f;
	g->print(contextptr);
	g->printing=0;
	g->resize(g->x(),g->y(),gw,g->h());
	fprintf(f,"%s","}\n}\\end{picture}\n}\n\n");
	fclose(f);
	file2stream(filename.c_str(),texof);
      }
      */
      ++number;
      string filename=remove_extension(name)+print_INT_(number)+".eps";
      FILE * f=fopen(filename.c_str(),"w");
      if (f){
	texof <<  "\\includegraphics[bb=0 0 400 "<< g->h() <<"]{" << filename << "}" << endl << endl ;
	int gw=g->w();
	g->resize(g->x(),g->y(),400,g->h());
	g->printing=f;
	g->print();
	g->printing=0;
	g->resize(g->x(),g->y(),gw,g->h());
	fclose(f);
      }
      return;
    }
    if (Turtle * g=dynamic_cast<Turtle *>(wid)){
      double xunit=giac::horiz_latex/g->w();
      double yunit=giac::vert_latex/g->h();
      string filename="#t"+print_INT_(count);
      ++count;
      FILE * f=fopen(filename.c_str(),"w");
      if (f && g->turtleptr){
	graph2tex(f,turtlevect2vecteur(*g->turtleptr),0,g->w(),0,g->h(),xunit,yunit,filename.c_str(),true,get_context(g));
	fclose(f);
	file2stream(filename.c_str(),texof); // copy f to the current stream
      }
      return;
    }
  }

  void history_pack_selection(History_Pack * hp, int & m,int & M){
    int l=hp->children();
    m=std::min(hp->_sel_begin,hp->_sel_end+1);
    M=std::max(hp->_sel_begin,hp->_sel_end+1);
    m=std::max(m,0);
    M=std::min(M,l);
  }

  void historypack2tex(History_Pack * hp,const std::string & name,ostream & texof,bool selection){
    int l=hp->children();
    int m=0,M=l;
    int number=1;
    if (selection)
      history_pack_selection(hp,m,M);
    for (int i=m;i<M;++i){
      if (!texof)
	return;
      texof << "\\noindent \\framebox{"<<i+1<<"} ";
      historypack2tex(hp->child(i),i,texof,hp,name,number);
    }
  }

  void historypack2tex(History_Pack * hp,const std::string & name,bool texheader,bool selection){
    ofstream texof(name.c_str());
    if (!texof)
      return;
    if (texheader){
      texof << "% Generated by xcas" << endl;
      texof << giac::tex_preamble ;
    }
    historypack2tex(hp,name,texof,selection);
    if (texheader)
      texof << giac::tex_end << endl;
    texof.close();
  }

  string historypack2tex(History_Pack * hp,bool texheader,bool selection=false){
    string name;
    static int i=0;
    ++i;
    if (hp->url)
      name=remove_extension(*hp->url)+".tex";
    else
      name="session"+print_INT_(i)+".tex";
    historypack2tex(hp,name,texheader,selection);
    return name;
  }


  void history_pack_latex_print(Fl_Widget* m ,bool preview,bool selection){
    if (m && m->parent()){
      History_Fold * o = get_history_fold(m);
      if (o){
	string s=historypack2tex(o->pack,true,selection);
	if (preview)
	  xdvi(s);
	else
	  dvips(s);
      }
    }
  }

  void History_cb_LaTeX_Preview(Fl_Widget* m , void*) {
    history_pack_latex_print(m,true,false);
  }

  void History_cb_LaTeX_Preview_sel(Fl_Widget* m , void*) {
    history_pack_latex_print(m,true,true);
  }

  void History_cb_LaTeX_Print(Fl_Widget* m , void*) {
    history_pack_latex_print(m,false,false);
  }

  void History_cb_LaTeX_Print_sel(Fl_Widget* m , void*) {
    history_pack_latex_print(m,false,true);
  }

  void History_cb_Print(Fl_Widget* m , void*) {
    if (m && m->parent()){
      History_Fold * o = get_history_fold(m);
      if (o)
	widget_print(o->pack);
    }
  }


  void History_cb_Preview(Fl_Widget* m , void*) {
    if (m && m->parent()){
      History_Fold * o = get_history_fold(m);
      if (o)
	widget_ps_print(o->pack,o->pack->url?*o->pack->url:"session",false,0,true);
    }
  }

  void History_cb_Preview_selected(Fl_Widget* m , void*) {
    if (m && m->parent()){
      History_Fold * o = get_history_fold(m);
      if (o){
	int i=o->pack->_sel_begin,j=o->pack->_sel_end;
	if (i>j)
	  std::swap(i,j);
	if (i<0)
	  i=0;
	if (j<0)
	  return;
	int n=o->pack->children();
	for (;i<=j && i<n;++i)
	  widget_ps_print(o->pack->child(i),("session"+print_INT_(i)).c_str(),true);
      }
    }
  }

  void fl_handle(Fl_Widget * w){
    if (fl_handle_lock)
      return;
    fl_handle_lock=true;
    Fl::focus(w);
    Fl::flush();
    Fl::handle(FL_KEYBOARD,w->window());
    Fl::flush();
    fl_handle_lock=false;
  } 

  void History_cb_Newline(Fl_Widget* m , void*) {
    if (fl_handle_lock)
      return;
    static char petit_buffer[2];
    petit_buffer[0]='\n';
    petit_buffer[1]=0;
    Fl::e_text= petit_buffer;
    Fl::e_length=1;
    Fl::e_keysym='\n';
    fl_handle(xcas::Xcas_input_focus);  
  }


  void cb_Paste(Fl_Widget* m , void*){
    Fl_Widget * w=Fl::focus();
    if (w){
      if (Flv_Table_Gen * spread_ptr=dynamic_cast<Flv_Table_Gen *> (w)){
	spread_ptr->paste(spread_ptr->selected);
      }
      else
	Fl::paste(*w);
    }
  }

  void synchronize(const History_Pack * hp,Graph2d3d * geo){
    geo->clear();
    int s=hp->children();
    for (int i=0;i<s;++i){
      Fl_Widget * wid=hp->child(i);
      if (Fl_Group * gr=dynamic_cast<Fl_Group *>(wid)){
	if (gr->children()>=3){
	  wid=gr->child(2);
	  if (Fl_Scroll * scroll = dynamic_cast<Fl_Scroll *>(wid))
	    wid=scroll->child(0);
	  if (Gen_Output * out=dynamic_cast<Gen_Output *>(wid)){
	    geo->add(out->value());
	    continue;
	  }
	}
      }
      geo->add(undef);
    }
  }

  void History_cb_Undo(Fl_Widget* m , void*) {
    if (m && m->parent()){
      Figure * f=find_figure(m);
      if (f && f->geo){
	f->geo->hp->restore(-1);
	synchronize(f->geo->hp,f->geo);
      }
      else {
	History_Pack * o = get_history_pack(m);
	if (o){
	  o->restore(-1);
	  return;
	}
	History_Fold * hf = get_history_fold(m);
	if (hf)
	  hf->pack->restore(-1);
      }
    }
  }

  void History_cb_Redo(Fl_Widget* m , void*) {
    if (m && m->parent()){
      Figure * f=find_figure(m);
      if (f && f->geo){
	f->geo->hp->restore(1);
	synchronize(f->geo->hp,f->geo);
      }
      else {
	History_Pack * o = get_history_pack(m);
	if (o){
	  o->restore(1);
	  return;
	}
	History_Fold * hf = get_history_fold(m);
	if (hf)
	  hf->pack->restore(1);
      }
    }
  }

  Fl_Menu_Item History_menu[] = {
    {gettext("File"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("Save"), 0,  (Fl_Callback*)History_cb_Save, 0, 0, 0, 0, 14, 56},
    {gettext("Save as"), 0,  (Fl_Callback*)History_cb_Save_as, 0, 0, 0, 0, 14, 56},
    {gettext("Export as"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("xcas text"), 0,  (Fl_Callback*)History_cb_Save_as_xcas_text, 0, 0, 0, 0, 14, 56},
    {gettext("maple text"), 0,  (Fl_Callback*)History_cb_Save_as_maple_text, 0, 0, 0, 0, 14, 56},
    {gettext("mupad text"), 0,  (Fl_Callback*)History_cb_Save_as_mupad_text, 0, 0, 0, 0, 14, 56},
    {gettext("ti text"), 0,  (Fl_Callback*)History_cb_Save_as_ti_text, 0, 0, 0, 0, 14, 56},
    {0},
    {gettext("Load"), 0x8006c,  (Fl_Callback*)History_cb_Load, 0, 0, 0, 0, 14, 56},
    {gettext("Insert"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("xcas session"), 0,  (Fl_Callback*)History_cb_Insert, 0, 0, 0, 0, 14, 56},
    {gettext("figure"), 0,  (Fl_Callback*)History_cb_Insert_Figure, 0, 0, 0, 0, 14, 56},
    {gettext("spreadsheet"), 0,  (Fl_Callback*)History_cb_Insert_Tableur, 0, 0, 0, 0, 14, 56},
    {gettext("program"), 0,  (Fl_Callback*)History_cb_Insert_Program, 0, 0, 0, 0, 14, 56},
    {0},
    {gettext("Kill"), 0x40071,  (Fl_Callback*)History_cb_Kill, 0, 0, 0, 0, 14, 56},
    {gettext("Print"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("preview"), 0,  (Fl_Callback*)History_cb_Preview, 0, 0, 0, 0, 14, 56},
    {gettext("to printer"), 0,  (Fl_Callback*)History_cb_Print, 0, 0, 0, 0, 14, 56},
    {gettext("preview selected levels"), 0,  (Fl_Callback*)History_cb_Preview_selected, 0, 0, 0, 0, 14, 56},
    {gettext("latex preview"), 0,  (Fl_Callback*)History_cb_LaTeX_Preview, 0, 0, 0, 0, 14, 56},
    {gettext("latex print"), 0,  (Fl_Callback*)History_cb_LaTeX_Print, 0, 0, 0, 0, 14, 56},
    {gettext("latex preview selection"), 0,  (Fl_Callback*)History_cb_LaTeX_Preview_sel, 0, 0, 0, 0, 14, 56},
    {gettext("latex print selection"), 0,  (Fl_Callback*)History_cb_LaTeX_Print_sel, 0, 0, 0, 0, 14, 56},
    {0},
    {0},
    {gettext("Edit"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("Add"), 0,  0, 0, 64, 0, 0, 14, 56},
#ifdef IPAQ
    {gettext("comment"), 0,  (Fl_Callback *) History_cb_New_Comment_Input, 0, 0, 0, 0, 14, 56},
    {gettext("entry"), 0,  (Fl_Callback *) cb_New_Input, 0, 0, 0, 0, 14, 56},
    {gettext("expression"), 0x80065,  (Fl_Callback *) History_cb_New_Equation, 0, 0, 0, 0, 14, 56},
    {gettext("syntax colored entry"), 0x80065,  (Fl_Callback *) History_cb_New_Xcas_Text_Editor, 0, 0, 0, 0, 14, 56},
    {gettext("spreadsheet"), 0,  (Fl_Callback *) History_cb_New_Tableur, 0, 0, 0, 0, 14, 56},
    {gettext("graph, geo2d"), 0,  (Fl_Callback *) History_cb_New_Figure, 0, 0, 0, 0, 14, 56},
    {gettext("graph, geo3d"), 0,  (Fl_Callback *) History_cb_New_Figure3d, 0, 0, 0, 0, 14, 56},
    {gettext("geo2d exact"), 0,  (Fl_Callback *) History_cb_New_Figurex, 0, 0, 0, 0, 14, 56},
    {gettext("geo3d exact"), 0,  (Fl_Callback *) History_cb_New_Figure3dx, 0, 0, 0, 0, 14, 56},
    {gettext("turtle"), 0,  (Fl_Callback *) History_cb_New_Logo, 0, 0, 0, 0, 14, 56},
    {gettext("program"), 0,  (Fl_Callback *) History_cb_New_Program, 0, 0, 0, 0, 14, 56},
    {gettext("group"), 0,  (Fl_Callback *) History_cb_New_HF, 0, 0, 0, 0, 14, 56},
#else
    {gettext("comment"), 0x80063,  (Fl_Callback *) History_cb_New_Comment_Input, 0, 0, 0, 0, 14, 56},
    {gettext("new entry"), 0x8006e,  (Fl_Callback *) cb_New_Input, 0, 0, 0, 0, 14, 56},
    {gettext("equation"), 0x80065,  (Fl_Callback *) History_cb_New_Equation, 0, 0, 0, 0, 14, 56},
    {gettext("spreadsheet"), 0x80074,  (Fl_Callback *) History_cb_New_Tableur, 0, 0, 0, 0, 14, 56},
    {gettext("graph, geo2d"), 0x80067,  (Fl_Callback *) History_cb_New_Figure, 0, 0, 0, 0, 14, 56},
    {gettext("graph, geo3d"), 0x80068,  (Fl_Callback *) History_cb_New_Figure3d, 0, 0, 0, 0, 14, 56},
    {gettext("geo2d exact"), 0x80078,  (Fl_Callback *) History_cb_New_Figurex, 0, 0, 0, 0, 14, 56},
    {gettext("geo3d exact"), 0x80079,  (Fl_Callback *) History_cb_New_Figure3dx, 0, 0, 0, 0, 14, 56},
    {gettext("turtle picture"), 0x80064,  (Fl_Callback *) History_cb_New_Logo, 0, 0, 0, 0, 14, 56},
    {gettext("program"), 0x80070,  (Fl_Callback *) History_cb_New_Program, 0, 0, 0, 0, 14, 56},
    {gettext("group"), 0,  (Fl_Callback *) History_cb_New_HF, 0, 0, 0, 0, 14, 56},
#endif // IPAQ
    {0},
    {gettext("Execute"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("worksheet"), 0,  (Fl_Callback *) History_cb_Run_Worksheet, 0, 0, 0, 0, 14, 56},
    {gettext("below"), 0,  (Fl_Callback *) History_cb_Run_Below, 0, 0, 0, 0, 14, 56},
    {gettext("remove answers below"), 0,  (Fl_Callback *) cb_Rm_Answers, 0, 0, 0, 0, 14, 56},
    {0},
    {gettext("Insert newline"), 0,  (Fl_Callback *) History_cb_Newline, 0, 0, 0, 0, 14, 56},
    {gettext("Paste"), 0,  (Fl_Callback *) cb_Paste, 0, 0, 0, 0, 14, 56},
    {gettext("Delete selected levels"), 0,  (Fl_Callback *) cb_Delete, 0, 0, 0, 0, 14, 56},
    {gettext("Group selected levels"), 0,  (Fl_Callback *) History_cb_Fold, 0, 0, 0, 0, 14, 56},
    {gettext("Merge selected levels"), 0,  (Fl_Callback *) History_cb_Merge, 0, 0, 0, 0, 14, 56},
    {gettext("Degroup current fold"), 0,  (Fl_Callback *) History_cb_Flatten, 0, 0, 0, 0, 14, 56},
    {gettext("Undo"), 0,  (Fl_Callback *) History_cb_Undo, 0, 0, 0, 0, 14, 56},
    {gettext("Redo"), 0,  (Fl_Callback *) History_cb_Redo, 0, 0, 0, 0, 14, 56},
    {0},
    {0}
  };
  
  void History_Fold::autosave_rm(){
    if (is_file_available(autosave_filename.c_str())){
      cerr << "Autosave remove " << autosave_filename << endl;
      unlink(autosave_filename.c_str());
    }
  }

  void History_cb_fold_button(Fl_Button* b , void*){
    if (!b) return;
    History_Fold * hf = dynamic_cast<History_Fold *>(b->parent());
    if (hf){
      if (hf->_folded)
	hf->unfold();
      else
	hf->fold();
    }
  }

  void History_cb_current_status(Fl_Button* b , void*){  
    const giac::context * contextptr = get_context(b);
    Xcas_load_cas_setup(contextptr);
  }

  void History_cb_keyboard_button(Fl_Button* b , void*){  
    if (Keyboard_Switch)
      Keyboard_Switch(1);
  }

  void History_cb_cmds_button(Fl_Button* b , void*){  
    if (Keyboard_Switch)
      Keyboard_Switch(4);
  }

  void History_cb_msg_button(Fl_Button* b , void*){  
    if (Keyboard_Switch)
      Keyboard_Switch(2);
  }

  void History_cb_save_button(Fl_Button* b , void*){
    if (!b) return;
    History_Fold * hf = get_history_fold(b);
    if (hf){
      hf->pack->save();
      hf->redraw();
    }
  }

  void History_cb_help_index(Fl_Widget * wid,void *){
    static std::string ans; 
    if (!wid)
      return;
    int remove,ii;
    Fl_Widget * w=xcas::Xcas_input_focus;
    Fl_Window * wd=wid->window();
    if (
	(ii=xcas::handle_tab("",(*giac::vector_completions_ptr()),wd->w()/3,wd->h()/3,remove,ans)) ){ 
      if (ii==1)
	ans = ans +"()";
      Fl::e_text = (char * ) ans.c_str();
      Fl::e_length = ans.size();
      if (w){
	xcas::fl_handle(w);
	if (Fl_Input * in =dynamic_cast<Fl_Input *>(w)){
	  if (ii==1) in->position(in->position()-1);
	}
      }
    }
  }

  void History_cb_tex_button(Fl_Widget* b , void*){
    static std::string s; 
    giac::gen g;
    const context * contextptr=get_context(b);
    try {
      if (xcas::Equation * ptr=dynamic_cast<xcas::Equation *> (Fl::focus()))
	g=ptr->get_selection();
      if (xcas::Flv_Table_Gen * ptr=dynamic_cast<xcas::Flv_Table_Gen *> (Fl::focus()))
	g=extractmatricefromsheet(ptr->selected);
      if (Fl_Input_ * ptr=dynamic_cast<Fl_Input *>(Fl::focus())){
	int i=ptr->position(),j=ptr->mark();
	if (i>j) swapint(i,j);
	s=ptr->value();
	s=s.substr(i,j-i);
	g=giac::gen(s,contextptr);
      }
      s=giac::gen2tex(g,0);
    } 
    catch (std::runtime_error & e){
      s = e.what()+g.print(contextptr);
    }
    int ss=s.size();
    Fl::copy(s.c_str(),ss,0);
    Fl::copy(s.c_str(),ss,1);  
  }

  void History_cb_help_button(Fl_Widget* b , void*){
    static char petit_buffer[]={9,0};
    xcas::Xcas_Text_Editor * ed=dynamic_cast<xcas::Xcas_Text_Editor *>(Fl::focus());
    if (ed){
      ed->completion();
      return;
    }
    xcas::Multiline_Input_tab * in=dynamic_cast<xcas::Multiline_Input_tab *>(Fl::focus());
    if (in){
      Fl::e_length=1;
      Fl::e_text=petit_buffer;
      xcas::fl_handle(Fl::focus());
    }
    else 
      History_cb_help_index(b,0);
  }

  void History_cb_stop_button(Fl_Widget* b , void*){
#ifndef __APPLE__
    if (xcas::interrupt_button){ 
      xcas::interrupt_button=false;
      xcas::History_Pack * hp =xcas::get_history_fold(b)->pack;
      context * cptr=hp?hp->contextptr:0;
      cerr << gettext("STOP pressed. Trying to cancel cleanly") << std::endl;
      if (!Fl::event_state(FL_SHIFT)){
	giac::ctrl_c=true;
	for (int j=0;j<300;j++){
	  if (!giac::ctrl_c){
	    *logptr(cptr) << gettext("Cleanly cancelled.") << std::endl;
	    giac::interrupted=false;
	    xcas::interrupt_button=true;
	    return;
	  }
	  usleep(10000);
	}
	giac::ctrl_c=false;  giac::interrupted=false;
      }
      *logptr(cptr) << gettext("Cancelling thread. Xcas may crash now or later :-( Consider saving and restarting Xcas.") << std::endl;
      xcas::interrupt_button=true;
      if (giac::is_context_busy(hp?hp->contextptr:0)){
        giac::kill_thread(true,hp?hp->contextptr:0);
        return;
      }
    }
#else
    static Fl_Window * w = 0;
    static Fl_Button * button = 0;
    static string s("10");
    if (xcas::interrupt_button){ 
      xcas::interrupt_button=false;
      xcas::History_Pack * hp =xcas::get_history_fold(b)->pack;
      context * cptr=hp?hp->contextptr:0;
      *logptr(cptr) << gettext("STOP pressed. Trying to cancel cleanly") << std::endl;
      if (!w){
	Fl_Group::current(0);
	w=new Fl_Window(200,50);
	button = new Fl_Button(2,2,w->w()-4,w->h()-4);
	w->label(gettext("Cancelling"));
	w->end();
      }
      w->set_modal();
      w->show();
      w->hotspot(w);
      Fl::focus(w);
      Fl::flush();
      if (!Fl::event_state(FL_SHIFT)){
	giac::ctrl_c=true;
	for (int k=5;k>=0;--k){
	  s=gettext("Trying to cancel cleanly: ")+print_INT_(k);
	  button->label(s.c_str());
	  Fl::flush();
#ifdef HAVE_LIBPTHREAD
	  Fl::unlock();
#endif
	  for (int j=0;j<100;j++){
	    if (!giac::ctrl_c){
	       giac::interrupted=false;
	      w->hide();
	      *logptr(cptr) << gettext("Cleanly cancelled.") << std::endl;
	      xcas::interrupt_button=true;
	      return;
	    }
	    usleep(10000);
	  }
	}
	giac::ctrl_c=false; giac::interrupted=false;
      }
      w->hide();
      xcas::interrupt_button=true;
      if (giac::is_context_busy(hp?hp->contextptr:0)){
	*logptr(cptr) << gettext("Cancelling thread. Xcas may crash now or later :-( Consider saving and restarting Xcas.") << std::endl;
        giac::kill_thread(true,hp?hp->contextptr:0);
        return;
      }
    }
#endif
  }

  void History_Fold::update_status(){
    const giac::context * ptr = pack->contextptr;
    if (is_context_busy(ptr))
      stop_button->activate();
    else {
      ++update_status_count;
      if (!stop_button->active() && 
#ifdef WIN32
	  (update_status_count%64) 
#else
	  (update_status_count%64) 
#endif
	  )
	return;
      stop_button->deactivate();
    }
    if (current_status){
      string mode_s="Config ";
      if (pack->url)
	mode_s += remove_path(*pack->url);
      mode_s += " :";
#ifdef IPAQ
      if (giac::approx_mode(ptr))
	mode_s += " ~ ";
      else
	mode_s += " = ";
#else
      if (giac::approx_mode(ptr))
	mode_s += " approx ";
      else
	mode_s += " exact ";
#endif
      if (giac::complex_mode(ptr)){ 
	if (giac::complex_variables(ptr)) 
	  mode_s += "CPLX ";
	else
	  mode_s += "cplx ";
      }
      else
	mode_s += "real ";
      if (giac::angle_radian(ptr))
	mode_s+="RAD ";
      else
	mode_s+="DEG ";
      mode_s += giac::print_INT_(giac::decimal_digits(ptr));
      mode_s += ' ';
      switch (giac::xcas_mode(ptr)){
      case 0: 
	mode_s+="xcas "; break;
      case 1: 
	mode_s+="maple "; break;
      case 2: 
	mode_s+="mupad "; break;
      case 3: 
	mode_s+="ti89 "; break;
      }
      /*
	if (giac::withsqrt(0))
	mode_s += "sqrt ";
	else
	mode_s += "Q[X] ";
      */
      // mode_s += "Time: ";
      // double t=double(clock());
      // mode_s += xcas::print_DOUBLE_(t/CLOCKS_PER_SEC);
#ifdef HAVE_MALLOC_H //
      struct mallinfo mem=mallinfo();
      double memd=mem.arena+mem.hblkhd;
      mode_s +=xcas::print_DOUBLE_(memd/1048576);
      mode_s += "M ";
#endif //
      /*
	char * ch = getcwd(0,0);
	mode_s += ch;
	free(ch);
      */
      mode_s += ' ';
      // mode_s += label();
      if (!current_status->label() || mode_s!= current_status->label()){
	if (mode)
	  delete [] mode;
	mode = new char[mode_s.size()+1];
	strcpy(mode,mode_s.c_str());
	current_status->label(mode);
      }
    }
  }

  History_Fold::History_Fold(int X,int Y,int W,int H,int showmenu,const char*l):Fl_Group(X,Y,W,H,l),_folded(false),mode(0) { 
    end();
    update_status_count=-1;
    if (!l)
      label(gettext("Unnamed"));
    if (parent()){
      labelsize(parent()->labelsize());
      labelfont(parent()->labelfont());
    }
    int L=labelsize()+labeladd;
    scroll_position=-1;
    // Fl_Group::current(this);
    // box(FL_FLAT_BOX);
    _bordersize=labelsize();
    autosave_filename = autosave_folder+"xcas_auto_";
    size_t ul=(size_t) this;
    autosave_filename += print_INT_(ul);
    autosave_filename += ".xws";
    scroll = new Fl_Scroll(X,Y+L,W,H-L);
    scroll->end();
    scroll->box(FL_FLAT_BOX);
    pack=new History_Pack(scroll->x(),scroll->y(),max(W-labelsize(),_bordersize),H-2*L); 
    pack->end();
    scroll->add(pack);
    scroll->resizable(pack);
    Fl_Group::add(scroll);
    Fl_Group::resizable(scroll);
    bnd_button=0;
    group=0;
    save_button=0;
    current_status=0;
    stop_button=0;
    keyboard_button=0;
    msg_button=0;
    close_button=0;
    mb=0;
    fold_button=0;
    input=0;
    if (showmenu){
      group = new Fl_Group(X,Y+2,W,L-2);
      help_button = new No_Focus_Button(X,Y+1,L,L-2,"?");
      help_button->callback((Fl_Callback*) History_cb_help_button);
      help_button->tooltip(gettext("Online help and command completion"));
      help_button->color(FL_CYAN);
      save_button = new Fl_Button(X+L,Y+1,3*L,L-2,gettext("Save"));
      save_button->color(Xcas_save_saved_color);
      save_button->callback((Fl_Callback*) History_cb_save_button);
      save_button->tooltip(gettext("Save current session"));
      save_button->align(Fl_Align(68|FL_ALIGN_INSIDE));
#ifdef IPAQ
      bnd_button = new No_Focus_Button(X+8*L,Y+1,2*L,L-2,"bnd");
      bnd_button->callback((Fl_Callback*) History_cb_tex_button);
      bnd_button->tooltip(gettext("Switch bandeau on or off"));
      current_status = new Fl_Button(X+6*L,Y+1,max(W-14*L,0),L-2);
      current_status->align(FL_ALIGN_LEFT|FL_ALIGN_CLIP |FL_ALIGN_INSIDE);
#else
      current_status = new Fl_Button(X+4*L,Y+1,max(W-12*L,0),L-2);
      current_status->align(FL_ALIGN_CENTER|FL_ALIGN_CLIP |FL_ALIGN_INSIDE);
#endif
      current_status->label("");
      current_status->labelfont(FL_HELVETICA_ITALIC);
      current_status->callback((Fl_Callback*) History_cb_current_status);
      current_status->tooltip(gettext("Current CAS status. Click to modify"));
      stop_button = new No_Focus_Button(current_status->x()+current_status->w(),current_status->y(),3*L,L-2);
      stop_button->label("STOP");
      stop_button->callback((Fl_Callback*) History_cb_stop_button);
      stop_button->labelcolor(FL_RED);
      stop_button->tooltip(gettext("Interrupt current computation"));
      stop_button->deactivate();
      xcas::Xcas_Cancel=stop_button; // FIXME when history fold destroyed
      keyboard_button = new No_Focus_Button(stop_button->x()+stop_button->w(),Y+1,3*L,L-2,gettext("Kbd"));
      keyboard_button->callback((Fl_Callback*) History_cb_keyboard_button);
      keyboard_button->tooltip(gettext("Switch keyboard on or off"));
      // msg_button = new No_Focus_Button(keyboard_button->x()+keyboard_button->w(),Y+1,2*L,L-2,"Msg");
      // msg_button->callback((Fl_Callback*) History_cb_msg_button);
      // msg_button->tooltip("Show messages line");
      // close_button = new Fl_Button(msg_button->x()+msg_button->w(),stop_button->y(),L,L-2);
      close_button = new Fl_Button(keyboard_button->x()+keyboard_button->w()+L,stop_button->y(),L,L-2);
      close_button->label("X");
      close_button->callback((Fl_Callback*) hf_Kill);
      close_button->tooltip(gettext("Close current session"));
      group->end();
      group->resizable(current_status);
      Fl_Group::add(group);
      /*
      mb = new Fl_Menu_Button(X,Y,W,L);
      mb->type(Fl_Menu_Button::POPUP3);
      mb->box(FL_NO_BOX);
      mb->menu(History_menu);
      Fl_Group::add(mb);
      */
    }
    else {
      labeltype(FL_NO_LABEL);
      fold_button = new Fl_Button(X,Y+1,L,L-2,"-");
      fold_button->color(Xcas_save_saved_color);
      fold_button->callback((Fl_Callback*) History_cb_fold_button);
      Fl_Group::add(fold_button);
      // only one of input or group will be visible
      input = new Fl_Input(X+L,Y+1,W-L,L-2);
      input->textfont(FL_HELVETICA_BOLD_ITALIC);
      // input->label("");
      Fl_Group::add(input);
    }
    change_group_fontsize(this,labelsize());
    set_colors(this);
    parent_redraw(this);
  }

  bool History_Fold::add_entry(int n){
    if (!pack->new_question )
      return false;
    int edw=max(w()-pack->_printlevel_w,_bordersize);
    int edh=editor_hsize(this);
    Fl_Widget * q=pack->new_question(edw,edh);
    q->resize(pack->x(),pack->y(),q->w(),q->h());
    pack->add_entry(n,q);
    return true;
  }

  void History_Fold::add_entry(int n,Fl_Widget * q){ 
    pack->add_entry(n,q);
  }

  void History_Pack::add_history_map(Fl_Widget * g,int undo_position){
    std::multimap<Fl_Widget *,indexed_string>::iterator it0=widget_history_map.lower_bound(g),it=it0,itend=widget_history_map.upper_bound(g);
    for (;it!=itend;++it){
      if (it->second.index>undo_position)
	break;
    }
    if (it==it0)
      widget_history_map.insert(it0,pair<Fl_Widget *,indexed_string>(g,indexed_string(undo_position,"")));
    else {
      --it;
      if (it->second.chaine.empty())
	it->second.chaine=widget_sprint(g);
      else
      widget_history_map.insert(it,pair<Fl_Widget *,indexed_string>(g,indexed_string(undo_position,"")));
    }
  }

  Fl_Widget * History_Pack::restore_history_map(Fl_Widget * g,int undo_position){
    std::multimap<Fl_Widget *,indexed_string>::const_iterator it0=widget_history_map.lower_bound(g),it=it0,itend=widget_history_map.upper_bound(g);
    if (it==itend)
      return 0;
    for (;it!=itend;++it){
      if (it->second.index>undo_position)
	break;
    }
    if (it==it0)
      return 0;
    --it;
    if (it->second.chaine.empty())
      return it->first; // widget pointer points to an actual widget
    // restore widget from string representation
    string s=it->second.chaine;
    int ss=s.size(),pos=0;
    Fl_Group::current(0);
    Fl_Widget * res=widget_load(s,ss,pos,contextptr);
    add_history_map(res,undo_position);
    return res;
  }

  bool History_Pack::remove_entry(int n,bool check){
    if (check && is_context_busy(contextptr)){
      fl_message("%s",gettext("Unable to cut. Xcas is busy."));
      return false;
    }
    int m=children();
    if (n>=m)
      return false;
    Fl_Widget * g= child(n);
    int pos;
    if (get_history_pack(Xcas_input_focus,pos) && pos==n)
      Xcas_input_focus=0;
    add_history_map(g,undo_position);
    remove(g);
    delete g;
    modified(false);
    parent_redraw(this);
    return true;
  }

  // Note: all calls are done with check=false
  // If true they don't work inside an icas session geo2d("");
  // I don't remember why check_in_history_fold was introduced...
  bool History_Pack::remove_selected_levels(bool check_in_history_fold){
    if (check_in_history_fold){
      if (!get_history_fold(this))
	return false;
    }
    if (is_context_busy(contextptr)){
      fl_message("%s",gettext("Unable to cut. Xcas is busy."));
      return false;
    }
    int s1=_sel_begin, s2= _sel_end;
    if (s1<0 || s2<0){
      fl_alert("%s",gettext("First select level(s):\nclick on the level numbers near the left border"));
    }
    if (s1>=0 && s2>=0){
      int m=min(s1,s2),M=max(s1,s2);
      update_pos=m;
      for (int i=m;i<=M;++i)
	remove_entry(m);
      _sel_end=_sel_begin=-1;
      backup();
      update();
      resize();
      return true;
    }
    return false;
  }

  bool History_Fold::remove_entry(int n){
    return pack->remove_entry(n);
  }

  int History_Fold::handle(int event){
    // if (Fl::event_button()== FL_RIGHT_MOUSE)
    //  return 0;
    return Fl_Group::handle(event);
  }

  void History_Fold::resize(int X,int Y,int W,int H){
    scroll_position = scroll->yposition();
    Fl_Widget::resize(X,Y,W,H);
    int L=labelsize()+labeladd;
    if (L>H)
      L=H;
    if (mb) mb->resize(X,Y,W,L); // (X,Y,W,H) not compatible w/ geo right-click
    if (fold_button) fold_button->resize(X,Y+2,L,L-2);
    if (input) input->resize(X+L,Y+2,W-L,L-2);
    if (group) group->resize(X,Y+2,W,L-2);
    // save_button->resize(X+3*L,Y+2,3*L,L-2);
    scroll->resize(X,Y+L,W,H-L);
    int sw=min(max(labelsize(),10),16);
    Fl_Scrollbar & ss = scroll->scrollbar;
    ss.resize(W-sw,ss.y(),sw,ss.h());
    // FIXME should use the vertical scrollbar size instead of bordersize
    pack->Fl_Widget::resize(pack->x(),pack->y(),max(W-sw-L,_bordersize),pack->h());
    pack->resize();
  }

  void parent_redraw(Fl_Widget * w){
    if (w->parent())
      w->parent()->redraw();
    else
      w->redraw();
  }

  void parents_redraw(Fl_Widget * w){
    if (w->parent())
      parents_redraw(w->parent());
    w->damage(FL_DAMAGE_ALL);
    w->redraw();
  }

  void brothers_redraw(Fl_Widget * w){
    if (!w->parent())
      return;
    Fl_Group * gr = w->parent();
    int n=gr->children();
    for (int i=0;i<n;++i){
      gr->child(i)->redraw();
    }
    gr->redraw();
  }

  void History_Fold::resize_parent(int H){
    Fl_Group * g=parent();
    if (g && g->children()==1){
      if (_folded)
	_hsorig=g->h();
      g->Fl_Widget::resize(g->x(),g->y(),g->w(),H);
    }
  }

  void History_Fold::fold(){ 
    if (!mb && !_folded){
      _folded=true;
      if (fold_button){
	fold_button->label("+");
	fold_button->redraw();
      }
      _horig=h();
      scroll->hide();
      int L=labelsize()+labeladd;
      resize(x(),y(),w(),L);
      resize_parent(2*L);
      if (window()) window()->redraw();
    }
  }

  void History_Fold::unfold(){
    if (!mb && _folded){
      _folded=false;
      if (fold_button){
	fold_button->label("-");
	fold_button->redraw();
      }
      scroll->show();
      resize_parent(_hsorig);
      resize(x(),y(),w(),_horig);
      Fl::flush();
      brothers_redraw(parent());
      if (pack->children())
	pack->focus(0);
    }
  }

  void History_Fold::draw(){
    int clip_x,clip_y,clip_w,clip_h;
    fl_clip_box(x(),y(),w(),h(),clip_x,clip_y,clip_w,clip_h);
    fl_push_clip(clip_x,clip_y,clip_w,clip_h);
    /*
    if (scroll_position>0){    
      Fl_Group::draw();
      scroll->position(0,scroll_position);
      scroll_position=-1;
      Fl_Group::draw();
    }
    else
    */
      Fl_Group::draw();
    fl_pop_clip();
  }


  void HScroll::draw(){
#ifdef __APPLE__
    redraw();
#endif
    Fl_Scroll::draw();
  }

  void group_resize(Fl_Group * gr){
    int n=gr->children();
    if (n!=3)
      return;
    History_Pack * Main_Pack = dynamic_cast<History_Pack *>(gr->child(0));
    if (Main_Pack){
      int ch=Main_Pack->children();
      if (ch){
	int widy=Main_Pack->child(0)->y();
	int widx=Main_Pack->child(0)->x();
	int widw=gr->w()-6-gr->labelsize();
	int space=Main_Pack->_spacing;
	int widh=(gr->h()-10-(ch-1)*space)/ch;
	for (int i=0;i<ch;i++){
	  Fl_Widget * wid =Main_Pack->child(i);
	  wid->resize(widx,widy,widw,widh);
	  widy += widh+space;
	}
      }
      Main_Pack->Fl_Widget::resize(gr->x()+2,gr->y()+2,gr->w()-6,gr->h()-10);
      gr->Fl_Group::resize(gr->x(),gr->y(),gr->w(),gr->h());
    }
  }

  void HScroll::resize(){
    group_resize(this);
  }

  void HScroll::resize(int X,int Y,int W,int H){
    Fl_Scroll::resize(X,Y,W,H);
    int L=labelsize();
    L=L>14?14:L;
    L=L<8?8:L;
    scrollbar.resize(scrollbar.x()+scrollbar.w()-L,scrollbar.y(),L,scrollbar.h());
    if (resizechildren)
      resize();
    else {
      // resize children horizontally
      int W1 = max(W -L-6,labelsize());
      Fl_Widget ** a=(Fl_Widget **) array();
      int n=children();
      int H1=0;
      for (int i=2;i<n;++i,++a){
	H1 += (*a)->h();
      }
      if (H1<H)
	W1=W-6;
      a=(Fl_Widget **) array();
      for (int i=2;i<n;++i,++a){
	Fl_Widget * o = *a;
	o->resize(X+2,o->y(),W1,o->h());
      }
    }
    redraw(); // since children have been modified
  }

  int DispG_Window::handle(int event){
    int res=Fl_Window::handle(event);
    if (event==FL_HIDE){
      int n=children();
      for (int i=0;i<n;++i){
	Fl_Group * t = dynamic_cast<Fl_Group *>(child(i));
	if (t && t->children()){
	  Graph2d3d * gr=dynamic_cast<Graph2d3d *>(t->child(0));
	  // if (gr) gr->clear(); // commented otherwise iconize clears graph
	}
      }
    }
    return res;
  }

  // Implementation of widget output stream
#ifdef WITH_MYOSTREAM
  owstream::owstream(Fl_Output * wid,context * ptr, int bsize):my_ostream(new widgetbuf(wid, bsize)),output(wid),contextptr(ptr) {}

  owstream::~owstream(){
    ostream * ptr=logptr(contextptr);
    if (ptr==this)
      logptr(&my_cerr,contextptr);
    //if (tie())      delete tie();
  }
#else
  owstream::owstream(Fl_Output * wid,context * ptr, int bsize):ostream(new widgetbuf(wid, bsize)),output(wid),contextptr(ptr) {}

  owstream::~owstream(){
    ostream * ptr=logptr(contextptr);
    if (ptr==this)
      logptr(&std::cerr,contextptr);
    //if (tie())      delete tie();
  }
#endif

  void output_resize(Fl_Input_ * output){
    if (!output)
      return;
    Fl_Widget * ptr = output->window();
    int maxh = ptr?(ptr->h()/3):400;
    const char * ch=output->value();
    int n=strlen(ch);
    int h0=output->labelsize()+4,res=n?h0+2:1;
    for (int i=0;i<n-1;++i,++ch){
      if (*ch=='\n'){
	res += h0;
	if (res>maxh)
	  break;
      }
    }
    output->resize(output->x(),output->y(),output->w(),res);
    if (n){
      output->position(n-1,n-1);
    }
  }

  void output_resize_parent(Fl_Input_ * output,bool resize_hp){
    int dhlog = -output->h();
    output_resize(output);
    dhlog += output->h();
    Fl_Group * gr = output->parent();
    if (gr){
      int m=gr->children();
      gr->Fl_Widget::resize(gr->x(),gr->y(),gr->w(),gr->h()+dhlog);
      for (int i=0;i<m;i++){
	Fl_Widget * tmp=gr->child(i);
	if (tmp->y()>output->y())
	  tmp->resize(tmp->x(),tmp->y()+dhlog,tmp->w(),tmp->h());
      }
    }  
    if (resize_hp){
      if (History_Pack * hp=get_history_pack(gr)){
	hp->resize();
	if (Fl_Scroll * s = dynamic_cast<Fl_Scroll *>(hp->parent())){
	  int spos=s->yposition();
	  if (spos+s->h()>hp->h()){
#ifdef _HAVE_FL_UTF8_HDR_
	    s->scroll_to(0,max(min(hp->h()-s->h(),spos+dhlog),0));
#else
	    s->position(0,max(min(hp->h()-s->h(),spos+dhlog),0));
#endif
	  }
	  s->redraw();
	}
      }
    }
  }

  // resize log widget stream
  void owstream::resize(){
    output_resize(output);
  }

  widgetbuf::widgetbuf(Fl_Output * wid, int bsize): streambuf(),text(wid){
    if (bsize)
      {
	char	*ptr = new char[bsize];
	setp(ptr, ptr + bsize);
      }
    else
      setp(0, 0);
    
    setg(0, 0, 0);
  }
  
  widgetbuf::~widgetbuf(){
    sync();
    delete[] pbase();
  }
  
  int widgetbuf::overflow(int c){
    put_buffer();
    if (c != EOF){
      if (pbase() == epptr())
	put_char(c);
      else
	sputc(c);
    }
    return 0;
  }
  
  int	widgetbuf::sync()
  {
    put_buffer();
    return 0;
  }
  
  void	widgetbuf::put_char(int chr)
  {
    char ch[2];
    ch[0]=chr;
    ch[1]=0;
#ifdef HAVE_LIBPTHREAD
    Fl::lock();
#endif
    if (chr=='')
      text->value("");
    else {
      int n=strlen(text->value());
      text->position(n,n);
      text->insert(ch);
    }
#ifdef HAVE_LIBPTHREAD
    Fl::unlock();
#endif
    if (chr=='\n'){
      int n=text->position();
      if (n)
	text->position(n-1,n-1);
      Fl::awake(text);
    }
  }

  void	widgetbuf::put_buffer()
  {
    if (pbase() != pptr())
      {
	int     len = (pptr() - pbase());
	char    *buffer = new char[len + 1];
        
        strncpy(buffer, pbase(), len);
	buffer[len] = 0;
#ifdef HAVE_LIBPTHREAD
	Fl::lock();
#endif
	text->insert(buffer);
#ifdef HAVE_LIBPTHREAD
	Fl::unlock();
#endif
	Fl::awake(text);
        
        setp(pbase(), epptr());
	delete [] buffer;
      }
  }

  void Clip_Scroll::draw(){
    int clip_x,clip_y,clip_w,clip_h;
    fl_clip_box(x(),y(),w(),h(),clip_x,clip_y,clip_w,clip_h);
    fl_push_clip(clip_x,clip_y,clip_w,clip_h);
    // cerr << this << " " << x() << " " << y() << " " << w() << " " << h() << " " << clip_x << " " << clip_y << " " << clip_w << " " << clip_h << " " << int(damage()) << endl;
    Fl_Scroll::draw();
    fl_pop_clip();
  }

  void Xcas_Tabs::resize(int X,int Y,int W,int H){
    Fl_Tabs::resize(X,Y,W,H);
    int n=children();
    int L=labelsize()+4;
    X += 1;
    W -= 2;
    H -= L-1;
    if (up)
      Y +=L;
    for (int i=0;i<n;++i){
      child(i)->resize(X,Y,W,H);
    }
  }

  int Log_Output::handle(int event){
    if (event!=FL_MOUSEWHEEL)
      return Fl_Multiline_Output::handle(event);
    return 0; // FIXME
    if (!Fl::event_inside(this))
      return 0;
    int n=Fl::e_dy;
    int i=position(),iorig=i,s=size();
    if (!s)
      return 0;
    if (n>0){
      for (;n;--n){
	for (++i;i<s-1;++i){
	  if (value()[i]==10)
	    break;
	}
      }
    }
    else {
      for (;n;++n){
	for (--i;i>0;--i){
	  if (value()[i]==10)
	    break;
	}
      }
    }
    if (i==iorig)
      return 0;
    position(i,i);
    return 1;
  }

#ifndef NO_NAMESPACE_XCAS
} // namespace xcas
#endif // ndef NO_NAMESPACE_XCAS

#endif // HAVE_LIBFLTK
