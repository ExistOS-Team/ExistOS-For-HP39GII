// -*- compile-command: "g++ -DHAVE_CONFIG_H -I. -I.. -g -c Tableur.cc -Wall" -*-
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifndef IN_GIAC
#include <giac/first.h>
#else
#include "first.h"
#endif
/*
 *  Copyright (C) 2002,2014 B. Parisse, Institut Fourier, 38402 St Martin d'Heres
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
#include "Tableur.h"
#include "Xcas1.h"
#include "Print.h"
#ifndef IN_GIAC
#include <giac/vecteur.h>
#else
#include "vecteur.h"
#endif
#ifndef IN_GIAC
#include <giac/identificateur.h>
#include <giac/usual.h>
#include <giac/prog.h>
#include <giac/misc.h>
#else
#include "identificateur.h"
#include "usual.h"
#include "prog.h"
#include "misc.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Check_Button.H>
#include <sys/stat.h>
#include <cerrno>
using namespace std;
using namespace giac;

#ifndef NO_NAMESPACE_XCAS
namespace xcas {
#endif // ndef NO_NAMESPACE_XCAS

  objet_bidon mon_objet_bidon_tableur;

#ifdef IPAQ
  int Flv_Table_Gen::def_rows=10,Flv_Table_Gen::def_cols=4;
#else
  int Flv_Table_Gen::def_rows=40,Flv_Table_Gen::def_cols=10;
#endif

  vecteur fillsheet(bool is_spreadsheet){
    vecteur res;
    if (is_spreadsheet)
      res=vecteur(3,string2gen(string(""),false));
    else
      res=vecteur(3,zero);
    res[2]=plus_two;
    return res;
  }

  Flv_Table_Gen * do_find_table_brother(Fl_Widget * widget){
    if (!widget)
      return 0;
    Fl_Group * gr = widget->parent();
    if (!gr)
      return 0;
    Flv_Table_Gen * spread_ptr=0;
    int n=gr->children();
    for (int i=0;i<n;++i){
      if ( (spread_ptr= dynamic_cast<Flv_Table_Gen *>(gr->child(i))) )
	break;
    }
    if (!spread_ptr)
      return do_find_table_brother(gr);
    return spread_ptr;
  }

  Flv_Table_Gen * find_table_brother(Fl_Widget * widget){
    Flv_Table_Gen * ptr = do_find_table_brother(widget);
    if (!ptr)
      ptr=do_find_table_brother(Fl::focus());
    if (!ptr)
      fl_alert("%s",gettext("No spreadsheet found. Please click in or add one"));
    return ptr;
  }

   void cb_Tableur_Recompute(Fl_Menu_* m , void*) {
    Flv_Table_Gen * spread_ptr=find_table_brother(m);
    if (spread_ptr){
      spread_ptr->spread_eval_interrupt();
      spread_ptr->redraw();
    }
  }

  void Flv_Table_Gen::changed(){
    changed_=true;
    backup();
  }

  void Flv_Table_Gen::config(){
    static Fl_Window * w = 0;
    static Fl_Input * varname=0,*input_init=0; // sheet variable name
    static Fl_Check_Button* evaltype=0,*moveright=0,*mat2cell=0,* issheet=0,*horiz=0,*viewgr=0;
    static Fl_Value_Input * nrow=0,*ncol=0,*max_hist=0;
    static Fl_Return_Button * button0 = 0 ;
    static Fl_Button * button1 =0;
    if (!w){
      int nlignes=8;
#ifdef IPAQ
      int dx=240,dy=300;
#else
      int dx=20*labelsize(), dy=14*labelsize();
#endif
      Fl_Group::current(0);
      w=new Fl_Window(dx,dy);
      varname=new Fl_Input(dx/2,2,dx/2-4,dy/nlignes-4,gettext("Variable"));
      varname->tooltip(gettext("Save the spreadsheet as a matrix in this variable"));
      nrow = new Fl_Value_Input(dx/4,2+dy/nlignes,dx/4-2,dy/nlignes-4,gettext("Rows"));
      nrow->minimum(1);
      nrow->maximum(1000);
      nrow->step(1);
      ncol = new Fl_Value_Input(3*dx/4,2+dy/nlignes,dx/4-2,dy/nlignes-4,gettext("Cols"));
      ncol->minimum(1);
      ncol->maximum(1000);
      ncol->step(1);
      evaltype = new Fl_Check_Button(2,2+2*dy/nlignes,dx/4-2,dy/nlignes-4,gettext("Eval"));
      evaltype->tooltip(gettext("Reeval spreadsheet automatically"));
      // evaltype->callback((Fl_Callback *) cb_Tableur_Recompute);
      moveright = new Fl_Check_Button(2+dx/2,2+2*dy/nlignes,dx/4-2,dy/nlignes-4,gettext("Move right"));
      moveright->tooltip(gettext("Move right or down after Enter"));
      mat2cell = new Fl_Check_Button(2,2+3*dy/nlignes,dx/4-2,dy/nlignes-4,gettext("Distribute"));
      mat2cell->tooltip(gettext("Matrix input is distributed or kept inside a cell"));
      issheet = new Fl_Check_Button(2+dx/2,2+3*dy/nlignes,dx/4-2,dy/nlignes-4,gettext("Spreadsheet"));
      issheet->tooltip(gettext("Matrix or spreadsheet"));
      horiz = new Fl_Check_Button(2,2+4*dy/nlignes,dx/4-2,dy/nlignes-4,gettext("Landscape"));
      horiz->tooltip(gettext("Split sheet/graph horizontally or vertically"));
      viewgr = new Fl_Check_Button(2+dx/2,2+4*dy/nlignes,dx/4-2,dy/nlignes-4,gettext("Graph"));
      viewgr->tooltip(gettext("Show or hide graph connected to this sheet"));
      max_hist = new Fl_Value_Input(3*dx/4,2+5*dy/nlignes,dx/4-2,dy/nlignes-4,gettext("Undo history"));
      input_init = new Fl_Input(3*dx/4,2+6*dy/nlignes,dx/4-2,dy/nlignes-4,gettext("Init sheet"));
      button0 = new Fl_Return_Button(2,2+7*dy/nlignes,dx/2-4,dy/nlignes-4);
      button0->shortcut(0xff0d);
      button0->label(gettext("OK"));
      button1 = new Fl_Button(dx/2+2,2+7*dy/nlignes,dx/2-4,dy/nlignes-4);
      button1->shortcut(0xff1b);
      button1->label(gettext("Cancel"));
      w->end();
      change_group_fontsize(w,labelsize());
      w->resizable(w);
      w->label(gettext("Sheet configuration"));
    }
    Tableur_Group * gr = dynamic_cast<Tableur_Group *>(parent());
    contextptr = get_context(gr);
    varname->value(name.print(contextptr).c_str());
    nrow->value(rows());
    ncol->value(cols());
    max_hist->value(max_history);
    issheet->value(is_spreadsheet);
    mat2cell->value(matrix_fill_cells);
    moveright->value(move_right);
    evaltype->value(spreadsheet_recompute);
    input_init->value(init.print(contextptr).c_str());
    if (gr){
      viewgr->value(gr->disposition/2);
      horiz->value(!(gr->disposition % 2));
    }
    int r=-1;
    w->set_modal();
    w->show();
    autosave_disabled=true;
    w->hotspot(w);
    Fl::focus(varname);
    for (;;) {
      Fl_Widget *o = Fl::readqueue();
      if (!o) Fl::wait();
      else {
	if (o == button0) {r = 0; break;}
	if (o == button1) {r = 1; break;}
	if (o == w) { r=1; break; }
      }
    }
    autosave_disabled=false;
    w->hide();
    if (!r){
      max_history=max(int(max_hist->value()),2);
      if ( nrow->value()>=1 && ncol->value()>=1 && (nrow->value()!=rows() || ncol->value()!= cols()) ){
	changed();
	resizesheet(int(nrow->value()),int(ncol->value()));
	if ((7+nrow->value())*(labelsize()+1)<h()){
	  increase_size(this,(7+nrow->value())*(labelsize()+1)-h());
	}
      }
      is_spreadsheet=issheet->value();
      matrix_fill_cells=mat2cell->value();
      move_right=moveright->value();
      is_spreadsheet=issheet->value();
      init=gen(input_init->value(),contextptr);
      if (init.type==_SYMB)
	protecteval(init,eval_level(contextptr),contextptr);
      if (!spreadsheet_recompute && evaltype->value()){
	spreadsheet_recompute=evaltype->value();
	spread_eval_interrupt();
      }
      else
	spreadsheet_recompute=evaltype->value();
      gen tmp(varname->value(),contextptr);
      if (tmp.type==_IDNT && !is_undef(tmp)){
	name=tmp;
	if (filename)
	  delete filename;
	filename = new string(tmp.print(contextptr)+".tab");
	if (filename){
	  prefix_filename="Save "+ remove_path(* filename);
	  if (Tableur_Group * gr=dynamic_cast<Tableur_Group *>(parent())){
	    gr->fname->label(prefix_filename.c_str());
	    gr->fname->show();
	  }
	}
	sto(extractmatricefromsheet(m),name,contextptr);
      }
      if (gr){
	if (!viewgr->value() && (gr->disposition & 0x2))
	  gr->save_dparam();
	gr->disposition=2*viewgr->value()+!horiz->value();
	gr->resize2();
      }
      update_status();
    }
    if (gr) Fl::focus(gr->table);
  }

  void Flv_Table_Gen::spread_erase(int nrows,int ncols){
    changed();
    if ( (nrows>=rows()) || (ncols>=cols()) )
      return;
    contextptr=get_context(this);
    m=matrice_erase(m,row(),col(),nrows,ncols,contextptr);
    rows(rows()-nrows);
    cols(cols()-ncols);
    row(min(rows()-1,row()));
    col(min(cols()-1,col()));
    update_status();
    update_name();
  }


  void Flv_Table_Gen::blank(int row_min,int r,int col_min,int c){
    changed();
    vecteur fill(fillsheet(is_spreadsheet));
    if (c<col_min)
      swap(c,col_min);
    if (r<row_min)
      swap(r,row_min);
    ++r;
    r=min(rows(),r);
    ++c;
    c=min(cols(),c);
    if ( (c==col_min) || (r==row_min) )
      return;
    selected.clear();
    selected.reserve(r-row_min);
    for (int i=row_min;i<r;++i){
      vecteur & v=*m[i]._VECTptr;
      vecteur tmp;
      tmp.reserve(c-col_min);
      for (int j=col_min;j<c;++j){
	tmp.push_back(v[j]);
	v[j]=freecopy(fill);
      }
      selected.push_back(tmp);
    }
    redraw();
    update_name();
  }

  void Flv_Table_Gen::blank(){
    changed();
    int row_min=select_start_row(),r=row();
    int col_min=select_start_col(),c=col();
    blank(row_min,r,col_min,c);
  }

  void Flv_Table_Gen::spread_insert(int nrows,int ncols){
    changed();
    vecteur fill(fillsheet(is_spreadsheet));
    contextptr=get_context(this);
    m=matrice_insert(m,row(),col(),nrows,ncols,fill,contextptr);
    rows(rows()+nrows);
    cols(cols()+ncols);
    update_status();
    redraw();
    update_name();
  }

  void Flv_Table_Gen::addrowatend(){
    changed();
    vecteur fill(fillsheet(is_spreadsheet));
    int C=cols();
    vecteur addrow(C);
    for (int j=0;j<C;++j)
      addrow[j]=freecopy(fill);
    m.push_back(addrow);
    rows(rows()+1);
    update_status();
    redraw();
    update_name();
  }

  void Flv_Table_Gen::addcolatend(){
    changed();
    vecteur fill(fillsheet(is_spreadsheet));
    int R=rows();
    for (int i=0;i<R;++i){
      m[i]._VECTptr->push_back(freecopy(fill));
    }
    cols(cols()+1);
    update_status();
    redraw();
    update_name();
  }

  void Flv_Table_Gen::resizesheet(int nr,int nc){
    changed();
    int cur_r=rows();
    vecteur fill(fillsheet(is_spreadsheet));
    if (nr<cur_r) // erase rows
      m=vecteur(m.begin(),m.begin()+nr);
    else {
      for (;cur_r<nr;++cur_r){
	vecteur tmp;
	for (int j=0;j<nc;++j)
	  tmp.push_back(freecopy(fill));
	m.push_back(tmp);
      }
    }
    for (int i=0;i<nr;++i){
      vecteur & v=*m[i]._VECTptr;
      int cur_c=v.size();
      if (nc<cur_c){
	m[i]=vecteur(v.begin(),v.begin()+nc);
      }
      else {
	for (;cur_c<nc;++cur_c)
	  v.push_back(freecopy(fill));
      }
    }
    rows(nr);
    cols(nc);
    row(0);
    col(0);
    spread_eval_interrupt();
    update_status();
    redraw();
    update_name();
  }

  void Flv_Table_Gen::copy(int clipboard){
    contextptr = get_context(this);
    int row_min=select_start_row(),r=row();
    int col_min=select_start_col(),c=col();
    selected.clear();
    if (c<col_min)
      swap(c,col_min);
    if (r<row_min)
      swap(r,row_min);
    ++r;
    ++c;
    if ( (c==col_min) || (r==row_min) )
      return;
    selected.reserve(r-row_min);
    for (int i=row_min;i<r;++i){
      vecteur tmp;
      tmp.reserve(c-col_min);
      for (int j=col_min;j<c;++j){
	tmp.push_back(m[i][j]);
      }
      selected.push_back(tmp);
    }
    gen g(extractmatricefromsheet(selected));
    static string s;
    s=g.print(contextptr);
    // Fl::selection(*this,s.c_str(),s.size());
    Fl::copy(s.c_str(),s.size(),clipboard);
    update_name();
  }

  void Flv_Table_Gen::copy_right(){
    changed();
    int R=row(),C=col();
    int c=cols(),r=rows();
    int R0=select_start_row();
    if (R0<0 || R0>=r)
      R0=R;
    if (R0<R)
      giac::swapint(R0,R);
    for (;R<=R0;++R){
      vecteur & v=*m[R]._VECTptr;
      gen g=v[C];
      for (int i=C+1;i<c;++i){
	v[i]=freecopy(g);
      }
    }
    spread_eval_interrupt();
    redraw();
  }

  void Flv_Table_Gen::copy_down(){
    changed();
    int R=row(),C=col();
    int r=rows(),c=cols();
    int C0=select_start_col();
    if (C0<0 || C0>=c)
      C0=C;
    if (C0<C)
      giac::swapint(C0,C);
    for (;C<=C0;++C){
      gen g=m[R][C];
      for (int i=R+1;i<r;++i){
	vecteur & v=*m[i]._VECTptr;
	v[C]=freecopy(g);
      }
    }
    spread_eval_interrupt();
    redraw();
  }

  void Flv_Table_Gen::copy_first_in_selection(){
    changed();
    int R=row(),C=col();
    int r=select_start_row(),c=select_start_col();
    if (r==R && c==C)
      return;
    gen g=m[r][c];
    if (r<R)
      giac::swapint(r,R);
    if (c<C)
      giac::swapint(c,C);
    R=max(R,0); C=max(C,0); r=min(r,rows()-1); c=min(c,cols()-1);
    for (int i=R;i<=r;++i){
      vecteur & v=*m[i]._VECTptr;
      for (int j=C;j<=c;++j)
	v[j]=freecopy(g);
    }
    spread_eval_interrupt();
    update_input();
    redraw();
  }

  void Flv_Table_Gen::paste(const matrice & m_orig,bool reeval){
    changed();
    contextptr=get_context(this);
    matrice m_copy(makefreematrice(m_orig));
    makespreadsheetmatrice(m_copy,contextptr);
    int nr,nc;
    mdims(m_copy,nr,nc);
    int r=row(),c=col();
    if (r+nr>rows()||c+nc>cols())
      resizesheet(max(r+nr,rows()),max(c+nc,cols()));
    int R=min(r+nr,rows()),C=min(c+nc,cols());
    for (int i=r;i<R;++i){
      vecteur & v=*m[i]._VECTptr;
      for (int j=c;j<C;++j){
	v[j]=m_copy[i-r][j-c];
      }
    }
    if (reeval)
      spread_eval_interrupt();
    update_input();
    redraw();
  }

  void Flv_Table_Gen::erase_row_col(int i){
    int R=row(),C=col();
    // erase rows push_row -> R && push_col -> C
    if (i==2)
      i=fl_choice(gettext("Remove what?"),gettext("Rows"),gettext("Cols"),gettext("Cancel"));
    int r1=min(push_row,R),r2=absint(push_row-R)+1,c1=min(push_col,C),c2=absint(push_col-C)+1;
    if (i==0){
      row(r1);
      push_row=r1;
      spread_erase(r2,0);
    }
    if (i==1){
      col(c1);
      push_col=c1;
      spread_erase(0,c2);
    }
  }

  void Flv_Table_Gen::update_goto(){
    contextptr = get_context(this);
    if (_goto){
      printcell_current_row(contextptr)=row();
      printcell_current_col(contextptr)=col();
      string s_s=printcell(makevecteur(makevecteur(0),makevecteur(0)),contextptr);
      _goto->value(s_s.c_str());
    }
  }

  void Flv_Table_Gen::update_input(){
    contextptr = get_context(this);
    int R=row();
    int C=col();
    edit_row=R;
    edit_col=C;
    if (input && R<rows() && C<cols()){
      gen tmp=m[R][col()];
      if (tmp.type==_VECT && tmp._VECTptr->size()==3)
	tmp=tmp._VECTptr->front();
      input->value(tmp.print(contextptr).c_str());
      input->position(0,input->value().size());
      // input->position(0,strlen(input->value()));
    }
  }

  void Flv_Table_Gen::enter_move(){
    int R=row(),C=col();
    if (!move_right){
      if (R<rows()-1)
	row(R+1);
      else {
	row(0);
	if (C<cols()-1)
	  col(C+1);
	else
	  col(0);
      }
    }
    else {
      if (C<cols()-1)
	col(C+1);
      else {
	col(0);
	if (R<rows()-1)
	  row(R+1);
	else
	  row(0);
      }
    }
    select_start_row(row());
    select_start_col(col());
    update_goto();
    redraw();
    update_input();
  }


  int Flv_Table_Gen::handle(int event){
    if (event==FL_MOUSEWHEEL){
      if (!Fl::event_inside(this))
	return 0;
    }
    contextptr = get_context(this);
    static string s;
    if (Fl::event_button()== FL_RIGHT_MOUSE)
      return 0;
    last_event=event;
    if (event==FL_KEYBOARD){ 
      Xcas_input_focus=this;
      // int mode=Fl::event_state(FL_CTRL);
      if (Fl::event_key()==FL_Escape){
	editing=false;
	// cerr << "Mtrw edit leave"<<endl;
      }
      if (Fl::event_key()==FL_Delete){
	erase_row_col(2);
	return 1;
      }
      if (Fl::event_key()==FL_Enter || Fl::event_key()==FL_KP_Enter){
	enter_move();
	return 1;
      }
      switch (Fl::event_key()){
      case FL_Control_R: case FL_Control_L: case FL_Alt_L: case FL_Alt_R:
	return 0;
      }
      char ch=Fl::event_text()?Fl::event_text()[0]:0;
      if (ch==3){
	if (selected.empty() && row()>=0 && row()<rows() && col()>=0 && col()<cols())
	  selected_1=vecteur(1,vecteur(1,m[row()][col()]));
	else
	  selected_1=selected;
	copy(1);
	return 1;
      }
      if (ch==22){
	if (selected_1.empty()){
	  Fl::paste(*this,1);
	}
	else 
	  paste(selected_1);
	return 1;
      }
      if (ch==25){
	restore(1);
	return 1;
      }
      if (ch==26){
	restore(-1);
	return 1;
      }
      if (ch==4 || ch==18)
	return 0;
    }
    int res=Flv_Table::handle(event);
    if (!editing){
      edit_row=row();
      edit_col=col();
    }
    bool shift=Fl::event_state(FL_SHIFT);
    if (event==FL_FOCUS){
      Fl::focus(this);
      return 1;
    }
    if (event==FL_PUSH && Fl::focus()!=this)
      Fl::focus(this);
    if (event==FL_RELEASE){
      set_cursor(this,FL_CURSOR_ARROW);
      if (editing)
	click_fill=false;
      if (click_fill){ // fill area 
	copy_first_in_selection();
	click_fill=false;
	return 1;
      }
    }
    if (event==FL_RELEASE && Fl::event_button()== FL_MIDDLE_MOUSE ){
      Fl::paste(*this);
      return 1;
    }
    if (event==FL_PASTE){
      if (selected.empty()){
	Fl::focus(input);
	input->handle(FL_PASTE);
	input->do_callback();
      }
      else
	paste(selected);
      return 1;
    }
    if ( event==FL_UNFOCUS || event==FL_HIDE || event==FL_SHOW ){
      /*
      if (event==FL_UNFOCUS){
	selected.clear();
	selected_1.clear();
      }
      */
      return res;
    }
    if (res){
      int R=row(),C=col();
      printcell_current_row(contextptr)=R;
      printcell_current_col(contextptr)=C;
      s=printcell(makevecteur(makevecteur(0),makevecteur(0)),contextptr);
      bool head=header_event==FLVE_ROW_HEADER_CLICKED || header_event==FLVE_COL_HEADER_CLICKED ||header_event==FLVE_ROW_FOOTER_CLICKED || header_event==FLVE_COL_FOOTER_CLICKED;
      if (head){
	if (event==FL_PUSH){
	  if (header_event==FLVE_ROW_HEADER_CLICKED){
	    push_col=col();
	    push_row=rows()-1;
	  }
	  if (header_event==FLVE_ROW_FOOTER_CLICKED){
	    push_col=col();
	    push_row=0;
	  }
	  if (header_event==FLVE_COL_HEADER_CLICKED){
	    push_col=cols()-1;
	    push_row=row();
	  }
	  if (header_event==FLVE_COL_FOOTER_CLICKED){
	    push_col=0;
	    push_row=row();
	  }
	}
	s=printcell(makevecteur(makevecteur(push_row-R),makevecteur(push_col-C)),contextptr)+":"+s;
      }
      else { // else head
	if (event==FL_PUSH){
	  if (shift){
	    editing=false;
	    select_start_row(push_row);
	    select_start_col(push_col);
	  }
	  else { 
	    push_row=R;
	    push_col=C;
	  }
	  if (!editing){
	    edit_row=R;
	    edit_col=C;
	  }
	}
	else {
	  if (push_row!=R || push_col!=C) {
	    if (editing)
	      s=printcell(makevecteur(makevecteur(push_row-R),makevecteur(push_col-C)),contextptr)+":"+s;
	    else 
	      if (event==FL_DRAG || event==FL_RELEASE ){
		s=printcell(makevecteur(makevecteur(push_row-R),makevecteur(push_col-C)),contextptr)+":"+s;
	      }
	  }
	}
      } // end else header_event
      if (event!=FL_ENTER && event!=FL_LEAVE && event!=FL_MOVE && _goto)
	_goto->value(s.c_str());
      if (editing && (header_event || (event==FL_PUSH || event ==FL_DRAG || event == FL_RELEASE) ) ){
	header_event = 0;
	if (event==FL_PUSH || event ==FL_DRAG || event == FL_RELEASE)
	  input->insert_replace(s.c_str(),true);
	if (event==FL_RELEASE){
	  row(edit_row); col(edit_col); select_start_row(edit_row); select_start_col(edit_col); 
	  Fl::focus(input);
	  // int s=strlen(input->value());
	  input->position(input->mark(),input->mark());
	  if (_goto){
	    _goto->value(printcell(makevecteur(makevecteur(edit_row-R),makevecteur(edit_col-C)),contextptr).c_str());
	  }
	  return 1;
	}
	if (editing)
	  return 1;
      }
      if (head || (event==FL_RELEASE && (push_row!=R || push_col!=C)) ){
	header_event = 0;
	copy();
	if (Xcas_help_output){
	  gen g=extractmatricefromsheet(selected);
	  static string s1;
	  s1=g.print(contextptr);
	  Xcas_help_output->value(s1.c_str());
	}
	return 1;
      }
      if ( !editing && (C>=0) && (R>=0) && (event!=FL_MOVE) && (event!=FL_ENTER) && (event!=FL_LEAVE) ){
	if ( (m[R][C].type==_VECT) && (m[R][C]._VECTptr->size()==3) ){
	  int save_r=printcell_current_row(contextptr),save_c=printcell_current_col(contextptr);
	  printcell_current_row(contextptr)=R,printcell_current_col(contextptr)=C;
	  s=m[R][C][0].print(contextptr);
	  printcell_current_row(contextptr)=save_r;printcell_current_col(contextptr)=save_c;
	}
	else
	  s=m[R][C].print(contextptr);
	input->value(s.c_str());
	input->position(0,s.size());
      }
    }
    if (!res && !shift && event==FL_KEYBOARD && !editing){
      editing=true;
      edit_row=row(); edit_col=col();
      Fl::focus(input);
      int r=Fl::handle(FL_KEYBOARD,window());
      return 1;
    }
    return res;
  }

#ifdef HAVE_LIBPTHREAD
  void * in_thread_spread_eval(void * arg){
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
    vecteur *v = (vecteur *) arg;
    context * contextptr=(context *) (*v)[2]._POINTER_val;
    matrice * mptr = (matrice *) (*v)[0]._POINTER_val;
    giac::spread_eval(*mptr,contextptr);
    thread_eval_status(0,contextptr);
    pthread_exit(0);
  }

  void thread_spread_eval(matrice &m,GIAC_CONTEXT){
    if (is_context_busy(contextptr))
      return;
    bool old_io_graph=io_graph(contextptr);
    io_graph(false,contextptr);
    pthread_t spread_eval_thread;
    giac::vecteur v(3);
    v[0]=gen((void *)&m,3);
    v[2]=gen((void *) contextptr,3);
    thread_eval_status(1,contextptr);
    pthread_mutex_lock(mutexptr(contextptr));
    int cres=pthread_create (&spread_eval_thread, (pthread_attr_t *) NULL, in_thread_spread_eval,(void *)&v);
    if (!cres){
      Fl::remove_idle(xcas::Xcas_idle_function,0);
      for (;;){
	int eval_status=thread_eval_status(contextptr);
	if (!eval_status)
	  break;
	Fl::wait(0.0001);
	if (kill_thread(contextptr)){
	  kill_thread(false,contextptr);
	  try {
	    pthread_cancel(spread_eval_thread) ;
	  } catch (std::runtime_error & err){
	  }
	  pthread_mutex_unlock(mutexptr(contextptr));
	  Fl::add_idle(xcas::Xcas_idle_function,0);
	  fl_message("%s",gettext("computation aborted"));
	  io_graph(old_io_graph,contextptr);
	  return ;
	}
	Xcas_idle_function((void *)contextptr);
      }
      void * ptr;
      pthread_join(spread_eval_thread,&ptr);
      // Restore pointers 
      pthread_mutex_unlock(mutexptr(contextptr));
      Fl::add_idle(xcas::Xcas_idle_function,0);
      return ;
    }
    pthread_mutex_unlock(mutexptr(contextptr));
    spread_eval(m,contextptr);
    io_graph(old_io_graph,contextptr);
  }

#else

  void thread_spread_eval(matrice &m,GIAC_CONTEXT){
    spread_eval(m,contextptr);
  }
#endif  

  void Flv_Table_Gen::set_matrix(const matrice & mym,bool interruptible,bool reeval){
    changed();
    m=makefreematrice(mym);
    cols(mcols(m));
    rows(m.size());
    contextptr=get_context(this);
    makespreadsheetmatrice(m,contextptr);
    if (is_spreadsheet && reeval) {
      if (interruptible)
	spread_eval_interrupt();
      else {
	evaled_table(contextptr)=(void *) this;
	spread_eval(m,contextptr);
      }
    }
    /*
    if (is_spreadsheet) 
      spread_eval_interrupt();
    */
    is_spreadsheet=false;
    if ( !mym.empty() && (!mym.front()._VECTptr->empty()) )
      is_spreadsheet=(mym[0][0].type==_VECT);
    move_on_enter(FLV_MOVE_ON_ENTER_COL_ROW);
    col(0);
    row(0);
    select_start_row(0);
    select_start_col(0);
    redraw();
  }
  
  void Flv_Table_Gen::update_status() {
    contextptr = get_context(this);
    redraw();
    status_string = "Sheet config: ";
    status_string += changed_?"* ":"- ";
    status_string += is_spreadsheet?"Spreadsheet ":"Matrix ";
    status_string += (name.type==_IDNT?name.print(contextptr):string("<>"))+" ";
    if (filename){
      if (Fl_Group * g=parent()){
	int n=g->children();
	for (int i=0;i<n;++i){
	  if (Fl_Output * o=dynamic_cast<Fl_Output *>(g->child(i))){
	    o->value(filename->c_str());
	  }
	}
      }
    }
    status_string += "R" +print_INT_(rows())+"C"+print_INT_(cols())+" ";
    status_string += spreadsheet_recompute?"auto ":"manual ";
    switch(matrix_symmetry){
    case 1:
      status_string += "Antihermitian ";
      break;
    case 2:
      status_string += "Hermitian ";
      break;
    case 3:
      status_string += "Antisymmetric ";
      break;
    case 4:
      status_string += "Symmetric ";
    }
    status_string += move_right?"right ":"down ";
    status_string += matrix_fill_cells?"fill":"cell";
    label(status_string.c_str());
  }

  void Flv_Table_Gen::set_matrix(const gen & g,bool interruptible,bool reeval){
    changed();
    if (!ckmatrix(g,true))
      settypeerr();
    set_matrix(*g._VECTptr,interruptible,reeval);
  }

  void Flv_Table_Gen::backup(){
    int vs=m_history.size();
    if (cur_history==vs){
      if (vs==max_history)
	m_history.erase(m_history.begin());
      else
	++cur_history; 
      m_history.push_back(makefreematrice(m));
    }
    else {
      m_history[cur_history]=makefreematrice(m);
      ++cur_history;
    }
  }

  void Flv_Table_Gen::restore(int delta){
    int vs=m_history.size();
    if (delta==-1 && cur_history==vs){
      backup();
      --delta;
    }
    cur_history += delta;
    if (cur_history<0) 
      cur_history=0;
    if (vs && cur_history<vs){
      m=*m_history[cur_history]._VECTptr;
      changed_=true;
      int r,c;
      mdims(m,r,c);
      rows(r);
      cols(c);
      redraw();
    }
    else
      cur_history=vs;
  }

  Flv_Table_Gen::~Flv_Table_Gen(){ 
    if (Xcas_input_focus==this) Xcas_input_focus=0;
    contextptr=get_context(this);
    if (evaled_table(contextptr)==this) 
      evaled_table(contextptr)=0; 
    if (win2){
      win2->hide();
      delete win2;
    }
    if (win3){
      win3->hide();
      delete win3;
    }
  }

  Flv_Table_Gen::Flv_Table_Gen( int X, int Y, int W, int H ,const matrice & mym, const char *l) : Flv_Table(X,Y,W,H,l),max_history(4),cur_history(0),matrix_fill_cells(true),move_right(false),matrix_symmetry(0),spreadsheet_recompute(true),push_row(0),push_col(0),contextptr(0),editing(false),computing(false),filename(0),max_printsize(1024) {
    if (!ckmatrix(mym))
      settypeerr();
    set_matrix(mym,false);
    changed_=false;
    graph3d=0; graph2d=0; graph=0; _goto=0; input=0;
    name=0; init=0;
    finish_flv_table_gen();
  }
  
  Flv_Table_Gen::Flv_Table_Gen( int X, int Y, int W, int H ,const gen &g, const char *l) :   Flv_Table(X,Y,W,H,l),matrix_fill_cells(true),move_right(false),matrix_symmetry(0),spreadsheet_recompute(true),push_row(0),push_col(0),contextptr(0),editing(false),computing(false),filename(0),max_printsize(1024) {
    set_matrix(g,false);
    changed_=false;
    graph2d=0; graph3d=0; graph=0; _goto=0; input=0;
    name=0; init=0;
    finish_flv_table_gen();
  }

  gen parse_interval(const char * ch){
    gen g=undef;
    string s;
    int l=strlen(ch);
    for (int i=0;i<l;++i,++ch){
      if (*ch==':')
	s += "..";
      else
	s+=*ch;
    }
    try {
      g=gen(s,context0); // ok
    }
    catch (std::runtime_error & e){
      if (Xcas_help_output)
	Xcas_help_output->value(e.what());
    }
    return g;
  } 

  string next_cell(const string & s,Flv_Table_Gen * ptr){
    if (!ptr) return "";
    const giac::context * contextptr = get_context(ptr);
    int maxr=ptr->rows(),maxc=ptr->cols();
    gen g=parse_interval(s.c_str());
    if (g.is_symb_of_sommet(at_interval)){
      gen & f=g._SYMBptr->feuille;
      if (f.type!=_VECT || f._VECTptr->size()!=2)
	return "";
      int r1,r2,c1,c2;
      if (!iscell(f._VECTptr->front(),c1,r1,contextptr) || !iscell(f._VECTptr->back(),c2,r2,contextptr))
	return "";
      if (r1>r2)
	giac::swapint(r1,r2);
      if (c1>c2)
	giac::swapint(c1,c2);
      // search for an empty cell
      for (int j=c2+1;j<maxc;++j){
	for (int i=0;i<maxr;++i){
	  gen tmp=ptr->m[i][j][0];
	  if (is_zero(tmp,contextptr)|| (tmp.type==_STRNG && tmp._STRNGptr->empty())){
	    int pr1=printcell_current_row(contextptr),pc1=printcell_current_col(contextptr);
	    printcell_current_row(contextptr)=printcell_current_col(contextptr)=0;
	    string s=printcell(makevecteur(vecteur(1,i),vecteur(1,j)),contextptr);
	    printcell_current_row(contextptr)=pr1;
	    printcell_current_col(contextptr)=pc1;
	    return s;
	  }
	}
      }
    }
    return "";
  }

  void cb_Spread_goto(Fl_Widget* widg, void*) {
    // Find a spreadsheet in the brothers of widget (parent's children)
    Flv_Table_Gen * spread_ptr=find_table_brother(widg);
    const giac::context * contextptr = get_context(spread_ptr);
    if (!spread_ptr)
      return;
    Fl::focus(spread_ptr);
    Fl_Input * widget=dynamic_cast<Fl_Input *>(widg);
    gen g=undef;
    if (widget)
      g=parse_interval(widget->value());
    int r,c;
    if (iscell(g,c,r,contextptr) ){
      if (r>=spread_ptr->rows()||c>=spread_ptr->cols())
	spread_ptr->resizesheet(max(r+1,spread_ptr->rows()),max(c+1,spread_ptr->cols()));
      // cerr << g << " " << r << " " << c << endl;
      spread_ptr->row(r);
      spread_ptr->col(c);
      spread_ptr->select_start_row(r);
      spread_ptr->select_start_col(c);
      printcell_current_row(contextptr)=r;
      printcell_current_col(contextptr)=c;
      if (spread_ptr->input){
	string s(spread_ptr->m[r][c][0].print(contextptr));
	spread_ptr->input->value(s.c_str());
	spread_ptr->input->position(s.size(),0);
      }
      return;
    }
    matrice m;
    if (iscell_range(g,spread_ptr->m,spread_ptr->selected,spread_ptr) ){
      g=extractmatricefromsheet(spread_ptr->selected);
      string s(g.print(contextptr));
      Fl::selection(*spread_ptr,s.c_str(),s.size());
      if (Xcas_help_output)
	Xcas_help_output->value(s.c_str());
    }
  }

  bool csv_guess(const char * data,int count,char & sep,char & nl,char & decsep){
    bool ans=true;
    int nb[256],pointdecsep=0,commadecsep=0; 
    for (int i=0;i<256;++i)
      nb[i]=0;
    // count occurence of each char
    // and detect decimal separator between . or ,
    for (int i=1;i<count-1;++i){
      if (data[i]=='[' || data[i]==']')
	ans=false;
      ++nb[(unsigned char) data[i]];
      if (data[i-1]>='0' && data[i-1]<='9' && data[i+1]>='0' && data[i+1]<='9'){
	if (data[i]=='.')
	  ++pointdecsep;
	if (data[i]==',')
	  ++commadecsep;
      }
    }
    decsep=commadecsep>pointdecsep?',':'.';
    // detect nl (ctrl-M or ctrl-J)
    nl=nb[10]>nb[13]?10:13;
    // find in control characters and : ; the most used (except 10/13)
    int nbmax=0,imax=-1;
    for (int i=0;i<60;++i){
      if (i==10 || i==13 || (i>=' ' && i<='9') )
	continue;
      if (nb[i]>nbmax){
	imax=i;
	nbmax=nb[i];
      }
    }
    // compare . with , (44)
    if (nb[unsigned(',')]>=nbmax){
      imax=',';
      nbmax=nb[unsigned(',')];
    }
    if (nbmax>=nb[unsigned(nl)])
      sep=imax;
    else
      sep=' ';
    return ans;
  }

  // scan filename supposed to be a CSV data file
  // guess separator and newline character
  bool csv_guess(const string & filename,char & sep,char & nl,char & decsep){
    struct stat st;
    if (stat(filename.c_str(),&st)==ENOENT)
      return false;
    FILE * f=fopen(filename.c_str(),"r");
    if (!f)
      return false;
    size_t size=1,count=st.st_size;
    char data[count];
    fread(data,size,count,f);
    fclose(f);
    return csv_guess(data,count,sep,nl,decsep);
  }

  void cb_Sheet_Input(Fl_Widget* widg, void*) {
    Flv_Table_Gen * spread_ptr=find_table_brother(widg);    
    if (!spread_ptr)
      return;    
    context * contextptr=get_context(spread_ptr);
    Fl::focus(spread_ptr);
    if (spread_ptr && spread_ptr->editing){
      spread_ptr->editing=false;
      // cerr << "Mtrw edit end" << endl;
      spread_ptr->row(spread_ptr->edit_row);
      spread_ptr->col(spread_ptr->edit_col);
    }
    spread_ptr->changed();
    int R=spread_ptr->row(),C=spread_ptr->col();
    string str(spread_ptr->input->value());
    gen g;
    try {
      g=gen(str,contextptr);
    }
    catch (std::runtime_error & e){
      cerr << e.what() << endl;
    }
    // count number of newline
    unsigned strs=str.size();
    unsigned nlpos=str.find('\n');
    if (giac::first_error_line(contextptr)>0 && nlpos>0 && nlpos<strs-1){
      char nl,sep,decsep,eof=-1;
      // removed the test sep!=' '
      if (csv_guess(str.c_str(),strs,sep,nl,decsep)){ 
	if (str[strs-1]!=nl)
	  str += nl;
	istringstream i(str);
	matrice M(csv2gen(i,sep,nl,decsep,eof,contextptr));
	int nc=M.front()._VECTptr->size();
	if (nc>=2 && ckmatrix(M,true)){
	  makespreadsheetmatrice(M,contextptr); // in place modifications of value
	  spread_ptr->paste(M);
	  if (spread_ptr->move_right){
	    C += nc-1;
	    if (C<spread_ptr->cols()){
	      spread_ptr->col(C);
	    spread_ptr->enter_move();
	    }
	  }
	  else {
	    R += M.size()-1;
	    if (R<spread_ptr->rows()){
	      spread_ptr->row(R);
	      spread_ptr->enter_move();
	    }
	  }
	  return;
	}
      }
    }
    int dofill=0;
    if ( (R>=0) && (C>=0) ){
      vecteur & v=*spread_ptr->m[R]._VECTptr;
      if (g.type==_SYMB){ 
         if ( (g._SYMBptr->sommet==at_tablefunc ) || (g._SYMBptr->sommet==at_tableseq) ) {
           if (g._SYMBptr->sommet==at_tablefunc)
             dofill=2;
           else
             dofill=1;
           spread_ptr->is_spreadsheet=true;
           // FIXME update_sheet_label();
           R=-1;
           g=protecteval(g,eval_level(contextptr),contextptr);
           spread_ptr->row(0);
         }
         else {
           if (!spread_ptr->is_spreadsheet || spread_ptr->matrix_fill_cells)
             g=protecteval(g,eval_level(contextptr),contextptr);
         }
      }
      if ( spread_ptr->matrix_fill_cells && g.type==_VECT){
        matrice & m=*g._VECTptr;
        if (!ckmatrix(m))
          m=vecteur(1,m);
        spread_ptr->paste(m);
        spread_ptr->redraw();
        Fl::focus(spread_ptr);
        spread_ptr->row(giacmin(R+m.size(),spread_ptr->rows()));
        if (dofill){
          spread_ptr->copy_down();
          if (dofill==2){
            spread_ptr->col(spread_ptr->col()+1);
            spread_ptr->copy_down();
          }
        }
        spread_ptr->select_start_row(spread_ptr->row());
        spread_ptr->select_start_col(spread_ptr->col());
	spread_ptr->update_goto();
	spread_ptr->update_input();
        return;
      }
      if ( (v[C].type==_VECT) && (v[C]._VECTptr->size()==3)){
	v[C]._VECTptr->front()=spread_convert(g,R,C,contextptr);
	if (!spread_ptr->is_spreadsheet)
	  (*v[C]._VECTptr)[1]=g;
	if (spread_ptr->spreadsheet_recompute)
          spread_ptr->spread_eval_interrupt();
	else 
	  (*v[C]._VECTptr)[1]=g;
      }
      else
	v[C]=g;
      if (spread_ptr->matrix_symmetry && spread_ptr->matrix_symmetry %2)
        g=-g;
      if (spread_ptr->matrix_symmetry && spread_ptr->matrix_symmetry/2)
        g=conj(g,contextptr);
      if (spread_ptr->matrix_symmetry && !spread_ptr->is_spreadsheet && (spread_ptr->rows()==spread_ptr->cols()) ){
        vecteur & v=*spread_ptr->m[C]._VECTptr;
        if ( (v[R].type==_VECT) && (v[R]._VECTptr->size()==3)){
	  v[R]._VECTptr->front()=spread_convert(g,C,R,contextptr);
	  (*v[R]._VECTptr)[1]=g;
        }
        else
	  v[R]=g;
      }
      spread_ptr->enter_move();
    }
  }


   void cb_Tableur_Eval_Sheet(Fl_Widget * m , void*) {
    Flv_Table_Gen * tg=find_table_brother(m);
    if (tg)
      tg->spread_eval_interrupt();
  }

   void cb_Tableur_Init(Fl_Widget * m , void*param) {
     Flv_Table_Gen * spread_ptr=find_table_brother(m);
     const giac::context * contextptr =get_context(spread_ptr);
     if (spread_ptr){
       protecteval(spread_ptr->init,eval_level(contextptr),contextptr);
       cb_Tableur_Eval_Sheet(m,param);
     }
   }

   void cb_Tableur_Value(Fl_Widget * m , void*) {
    Flv_Table_Gen * spread_ptr=find_table_brother(m);
    const giac::context * contextptr =get_context(spread_ptr);
    if (spread_ptr){
      int R=spread_ptr->row(),C=spread_ptr->col();
      if (spread_ptr->m.size()>unsigned(R)){
	gen g=spread_ptr->m[R];
	if (g.type==_VECT && g._VECTptr->size()>unsigned(C)){
	  g=(*g._VECTptr)[C];
	  if (g.type==_VECT && g._VECTptr->size()>2)
	    g=(*g._VECTptr)[1];
	  spread_ptr->input->value(g.print(contextptr).c_str());
	}
      }
    }
  }

  /*
   void cb_Tableur_LaTeX_Preview(Fl_Menu_* m , void*) {
    Flv_Table_Gen * tg=find_table_brother(m);
  }

   void cb_Tableur_LaTeX_Print(Fl_Menu_* m , void*) {
    Flv_Table_Gen * tg=find_table_brother(m);
  }
  */

   void cb_Tableur_Save(Fl_Widget* m , void*);

  bool get_filename(string & tmp,const string & extension){
    for (;;){
      char * newfile = file_chooser(gettext("Save sheet"), ("*."+extension).c_str(), "");
      if ( (!newfile) || (!*newfile))
	return false;
      tmp=newfile;
      //      tmp=remove_path(newfile);
      if (tmp.size()>1000)
	tmp=tmp.substr(0,1000);
      tmp=remove_extension(tmp.c_str())+"."+extension;
      if (access(tmp.c_str(),R_OK))
	return true;
      int i=fl_ask("%s",(tmp+gettext(": file exists. Overwrite?")).c_str());
      if (i==1)
	return true;
    }
    return false;
  }

   void cb_Tableur_Graph2d(Fl_Widget* m , void*) {
    Flv_Table_Gen * tg=find_table_brother(m);
    if (tg && tg->graph2d)
      tg->graph2d->window()->show();
  }

   void cb_Tableur_Graph3d(Fl_Widget* m , void*) {
    Flv_Table_Gen * tg=find_table_brother(m);
    if (tg){
      if (tg->win3){
	delete tg->graph3d;
	delete tg->win3;
      }
      Fl_Group::current(0);
      tg->win3=new Fl_Window(tg->x()+10,tg->y()+10,2*tg->w()/3,2*tg->h()/3);
#ifdef HAVE_LIBFLTK_GL
      tg->graph3d = new Graph3d(0,0,tg->win3->w(),tg->win3->h(),"",0);
      tg->graph3d->ylegende=1.5;
      tg->win3->end();
      tg->win3->resizable(tg->win3);
      tg->win3->show();
      tg->update_spread_graph();
#endif
    }
  }

   void cb_Tableur_Save_as(Fl_Widget* m , void*) {
    Flv_Table_Gen * tg=find_table_brother(m);
    if (!tg)
      return;
    string tmp;
    if (!get_filename(tmp,"tab"))
      return ;
    if (tg->filename) 
      delete tg->filename;
    tg->filename = new string(tmp);
    if (tg->filename){
      gen tmp(remove_extension(remove_path(*tg->filename)),context0);
      if (tmp.type==_IDNT && !is_undef(tmp)){
	tg->name=tmp;
	tg->update_name();
      }
      tg->prefix_filename="Save "+remove_path(*tg->filename);
      if (Tableur_Group * gr=dynamic_cast<Tableur_Group *>(tg->parent())){
	gr->fname->label(tg->prefix_filename.c_str());
	gr->fname->show();
      }
    }
    cb_Tableur_Save(m,0);
  }


   void cb_Tableur_Save_mathml(Fl_Menu_* m , void*) {
    Flv_Table_Gen * tg=find_table_brother(m);
    if (!tg) return;
    const giac::context * contextptr = get_context(tg);
    string tmp;
    if (!get_filename(tmp,"htm"))
      return ;
    ofstream of(tmp.c_str());
    if (!of){
      fl_message("%s","Write error");
      return;
    }
    of << mathml_preamble << endl;
    int formule=tg->is_spreadsheet?0:1;
    of << spread2mathml(tg->m,formule,contextptr);
    of << mathml_end << endl;
  }

   void cb_Tableur_Save_CSV(Fl_Menu_* m , void*) {
    Flv_Table_Gen * tg=find_table_brother(m);
    if (!tg) return;
    string tmp;
    if (!get_filename(tmp,"csv"))
      return ;
    ofstream of(tmp.c_str());
    if (!of){
      fl_message("%s","Write error");
      return;
    }
    matrice M(extractmatricefromsheet(tg->m));
    const_iterateur it=M.begin(),itend=M.end();
    for (;it!=itend;++it){
      if (it->type==_VECT){
	const_iterateur jt=it->_VECTptr->begin(),jtend=it->_VECTptr->end();
	for (;jt!=jtend;++jt){
	  of << *jt << ";";
	}
	of << endl;
      }
    }
    of << endl;
    of.close();
  }


   void cb_Tableur_Save(Fl_Widget * m , void*) {
    Flv_Table_Gen * tg=find_table_brother(m);
    if (!tg) return;
    if (!tg->filename)
      cb_Tableur_Save_as(m,0);
    if (tg->filename){
      ofstream of(tg->filename->c_str());
      if (!of){
	fl_message("%s","Write error");
	return;
      }
      if (tg->is_spreadsheet)
	of << gen(tg->m,_SPREAD__VECT) << endl;
      else
	of << gen(extractmatricefromsheet(tg->m)) << endl;
      of.close();
      tg->changed_ = false;
    }
    tg->update_status();
  }

   void cb_Tableur_Save_var(Fl_Widget* m , void*) {
    Flv_Table_Gen * tg=find_table_brother(m);
    if (!tg) return;
    const giac::context * contextptr = get_context(tg);
    if (tg){
      matrice mat=extractmatricefromsheet(tg->selected);
      if (mat.empty() && tg->row()<tg->rows() && tg->col() < tg->cols()){
	gen res=tg->m[tg->row()][tg->col()];
	if (res.type==_VECT && res._VECTptr->size()==3)
	  res=(*res._VECTptr)[1];
	mat=vecteur(1,vecteur(1,res));
      }
      const char * varname = fl_input(gettext("Variable name?"));
      if (varname){
	gen g(varname,contextptr);
	if (g.type!=_IDNT){
	  fl_message("%s",(g.print(contextptr)+gettext("is not a variable name")).c_str());
	  cb_Tableur_Save_var(m,0);
	}
	protecteval(symb_sto(mat,g),eval_level(contextptr),contextptr);
      }
    }
  }

  // Insure that all elements of m that are vectors have length <= 3
  void spread_ck(matrice & m){
    iterateur it=m.begin(),itend=m.end();
    for (;it!=itend;++it){
      if (it->type==_VECT){
	iterateur jt=it->_VECTptr->begin(),jtend=it->_VECTptr->end();
	for (;jt!=jtend;++jt){
	  if (jt->type==_VECT && jt->_VECTptr->size()>3){
	    *jt=makevecteur(jt->_VECTptr->front(),vecteur(jt->_VECTptr->begin()+1,jt->_VECTptr->end()-1),jt->_VECTptr->back());
	  }
	}
      }
    }
  }

  std::string tableur_insert(Flv_Table_Gen * tg){
    char * newfile = load_file_chooser(gettext("Insert sheet"), "*.tab", "",0,false);
    if (!newfile || file_not_available(newfile))
      return "";
    ifstream inf(newfile);
    const giac::context * contextptr = get_context(tg);
    gen value(read1arg_from_stream(inf,contextptr));
    if (!ckmatrix(value,true))
      return "";
    spread_ck(*value._VECTptr); // in place modifications of value
    tg->paste(*value._VECTptr);
    return remove_path(remove_extension(newfile));
  }

   void cb_Tableur_Insert(Fl_Menu_* m , void*) {
    Flv_Table_Gen * tg=find_table_brother(m);
    if (!tg) return;
    tableur_insert(tg);
  }

   void cb_Tableur_Insert_CSV(Fl_Menu_* m , void*) {
    Flv_Table_Gen * tg=find_table_brother(m);
    if (!tg) return;
    const giac::context * contextptr = get_context(tg);
    char * newfile = load_file_chooser(gettext("Insert CSV sheet"), "*.csv", "",0,false);
    if (!newfile || file_not_available(newfile))
      return;
    char sep=';',nl='\n',decsep=',';
    csv_guess(newfile,sep,nl,decsep);
    static Fl_Window * w = 0;
    static Fl_Input * separator=0, * newline=0,*decimal_sep=0,*end_file=0; // field separator, newline, decimal sep, end of file
    static Fl_Check_Button* import_syntax=0;
    static Fl_Return_Button * button0 = 0 ;
    static Fl_Button * button1 =0;
    if (!w){
#ifdef IPAQ
      int dx=240, dy=300;
#else
      int dx=300, dy=120;
#endif
      Fl_Group::current(0);
      w=new Fl_Window(dx,dy);
      separator=new Fl_Input(dx/4,2,dx/4-2,dy/4-4,gettext("Separator"));
      separator->tooltip(gettext("One character used to separate fields, like ; Use ^I for tab"));
      newline=new Fl_Input(3*dx/4,2,dx/4-2,dy/4-4,gettext("Newline"));
      newline->tooltip(gettext("One character used to separate lines, like <return> Use ^I for tab"));
      decimal_sep=new Fl_Input(dx/4,2+dy/4,dx/4-2,dy/4-4,gettext("Decimal"));
      decimal_sep->tooltip(gettext("One character used for decimal digit separator, like . or ,"));
      end_file=new Fl_Input(3*dx/4,2+dy/4,dx/4-2,dy/4-4,gettext("Endfile"));
      end_file->tooltip(gettext("Stop reading at first occurence of this character, leave blank for none"));
      import_syntax = new Fl_Check_Button(dx/4,2+2*dy/4,dx/4-2,dy/4-4,gettext("Start row=1"));
      import_syntax->value(1);
      button0 = new Fl_Return_Button(2,2+3*dy/4,dx/2-4,dy/4-4);
      button0->shortcut(0xff0d);
      button0->label(gettext("OK"));
      button1 = new Fl_Button(dx/2+2,2+3*dy/4,dx/2-4,dy/4-4);
      button1->shortcut(0xff1b);
      button1->label(gettext("Cancel"));
      w->end();
      w->resizable(w);
      w->label(gettext("CSV Import filter"));
    }
    // Guess separator and newline values from file scanning
    separator->value(string(1,sep).c_str());
    newline->value(string(1,nl).c_str());
    decimal_sep->value(string(1,decsep).c_str());
    int r=-1;
    w->set_modal();
    w->show();
    autosave_disabled=true;
    w->hotspot(w);
    Fl::focus(separator);
    for (;;) {
      Fl_Widget *o = Fl::readqueue();
      if (!o) Fl::wait();
      else {
	if (o == button0) {r = 0; break;}
	if (o == button1) {r = 1; break;}
	if (o == w) { r=1; break; }
      }
    }
    autosave_disabled=false;
    w->hide();
    if (!r){
      ifstream i(newfile);
      char sep=';',nl='\n',decsep=',',eof=0;
      if (strlen(separator->value()) && separator->value()[0]){
	if (separator->value()[1])
	  sep=separator->value()[1]-64;
	else
	  sep=separator->value()[0];
      }
      if (strlen(newline->value()) && newline->value()[0]){
	if (newline->value()[1])
	  nl=newline->value()[1]-64;
	else
	  nl=newline->value()[0];
      }
      if (decimal_sep->value()[0])
	decsep=decimal_sep->value()[0];
      if (end_file->value()[0])
	eof=end_file->value()[0];
      int save_maple_mode=xcas_mode(contextptr);
      xcas_mode(contextptr)=import_syntax->value();
      matrice M(csv2gen(i,sep,nl,decsep,eof,contextptr));
      if (!ckmatrix(M,true))
	return;
      context * contextptr=get_context(tg);
      makespreadsheetmatrice(M,contextptr); // in place modifications of value
      tg->paste(M);
      xcas_mode(contextptr)=save_maple_mode;
    }
  }

   void cb_Tableur_Preview(Fl_Menu_* m , void*) {
    Flv_Table_Gen * tg=find_table_brother(m);
    if (!tg) return;
    widget_ps_print(tg,tg->filename?*tg->filename:"table",false);
  }

   void cb_Tableur_Print(Fl_Menu_* m , void*) {
    Flv_Table_Gen * tg=find_table_brother(m);
    if (!tg) return;
    widget_print(tg);
  }

   void cb_Tableur_Copy_Cell(Fl_Menu_* m , void*) {
    Flv_Table_Gen * tg=find_table_brother(m);
    if (!tg) return;
    tg->copy();
  }

   void cb_Tableur_Copy_Right(Fl_Menu_* m , void*) {
    Flv_Table_Gen * spread_ptr=find_table_brother(m);
    if (!spread_ptr) return;
    spread_ptr->copy_right();
  }

   void cb_Tableur_Copy_Down(Fl_Menu_* m , void*) {
    Flv_Table_Gen * spread_ptr=find_table_brother(m);
    if (!spread_ptr) return;
    spread_ptr->copy_down();
  }

   void cb_Tableur_DelRow(Fl_Menu_* m , void*) {
    Flv_Table_Gen * tg=find_table_brother(m);
    if (!tg) return;
    tg->spread_erase(1,0); 
  }

   void cb_Tableur_Del(Fl_Menu_* m , void*) {
    Flv_Table_Gen * spread_ptr=find_table_brother(m);
    if (!spread_ptr) return;
    bool tmp=spread_ptr->is_spreadsheet;
    spread_ptr->is_spreadsheet=false;
    spread_ptr->blank(0,spread_ptr->rows(),0,spread_ptr->cols());
    spread_ptr->is_spreadsheet=tmp;
  }

   void cb_Tableur_DelRows(Fl_Menu_* m , void*) {
    Flv_Table_Gen * tg=find_table_brother(m);
    if (!tg) return;
    tg->erase_row_col(0); 
  }

   void cb_Tableur_DelCol(Fl_Menu_* m , void*) {
    Flv_Table_Gen * tg=find_table_brother(m);
    if (!tg) return;
    tg->spread_erase(0,1);
  }

   void cb_Tableur_DelCols(Fl_Menu_* m , void*) {
    Flv_Table_Gen * tg=find_table_brother(m);
    if (!tg) return;
    tg->erase_row_col(1); 
  }

   void cb_Tableur_InsRow(Fl_Menu_* m , void*) {
    Flv_Table_Gen * tg=find_table_brother(m);
    if (!tg) return;
    tg->spread_insert(1,0); 
  }

   void cb_Tableur_InsCol(Fl_Menu_* m , void*) {
    Flv_Table_Gen * tg=find_table_brother(m);
    if (!tg) return;
    tg->spread_insert(0,1); 
  }

   void cb_Tableur_InsCol_End(Fl_Menu_* m , void*) {
    Flv_Table_Gen * tg=find_table_brother(m);
    if (!tg) return;
    tg->addcolatend(); 
  }

   void cb_Tableur_InsRow_End(Fl_Menu_* m , void*) {
    Flv_Table_Gen * tg=find_table_brother(m);
    if (!tg) return;
    tg->addrowatend(); 
  }

   void cb_Tableur_MoveRight(Fl_Menu_* m , void*) {
    Flv_Table_Gen * spread_ptr=find_table_brother(m);
    if (!spread_ptr) return;
    spread_ptr->move_on_enter(FLV_MOVE_ON_ENTER_COL_ROW); 
    spread_ptr->move_right=true;
    spread_ptr->update_status();
  }

   void cb_Tableur_MoveDown(Fl_Menu_* m , void*) {
    Flv_Table_Gen * spread_ptr=find_table_brother(m);
    if (!spread_ptr) return;
    spread_ptr->move_on_enter(FLV_MOVE_ON_ENTER_ROW_COL); 
    spread_ptr->move_right=false;
    spread_ptr->update_status();
  }

   void cb_Tableur_FillSelection(Fl_Menu_* m , void*) {
    Flv_Table_Gen * spread_ptr=find_table_brother(m);
    if (!spread_ptr) return;
    spread_ptr->copy_first_in_selection();
  }

   void cb_Tableur_Col_Larger(Fl_Menu_* m , void*) {
    Flv_Table_Gen * spread_ptr=find_table_brother(m);
    if (!spread_ptr) return;
    spread_ptr->col_width(spread_ptr->col_width(spread_ptr->col())+5,spread_ptr->col());
  }

   void cb_Tableur_Col_Smaller(Fl_Menu_* m , void*) {
    Flv_Table_Gen * spread_ptr=find_table_brother(m);
    if (!spread_ptr) return;
    spread_ptr->col_width(spread_ptr->col_width(spread_ptr->col())-5,spread_ptr->col());
  }

   void cb_Tableur_Fill0(Fl_Menu_* m , void*) {
    Flv_Table_Gen * spread_ptr=find_table_brother(m);
    if (!spread_ptr) return;
    bool tmp=spread_ptr->is_spreadsheet;
    spread_ptr->is_spreadsheet=false;
    // int r1=spread_ptr->select_start_row(),r2=spread_ptr->row(),c1=spread_ptr->select_start_col(),c2=spread_ptr->col();
    // spread_ptr->blank(r1,r2,c1,c2);
    spread_ptr->blank();
    spread_ptr->is_spreadsheet=tmp;
  }

  void tableur_insert_replace(Flv_Table_Gen * spread_ptr,const string & s){
    if (!spread_ptr) return;
    if (!spread_ptr->editing){
      spread_ptr->edit_row=spread_ptr->row();
      spread_ptr->edit_col=spread_ptr->col();
      spread_ptr->editing=true;
    }
    spread_ptr->input->insert_replace(s,true);
    spread_ptr->input->position(spread_ptr->input->mark(),spread_ptr->input->mark());
    Fl::focus(spread_ptr->input);
  }


  std::string coltocolname(int i){
    string tmp;
    for(int j=0;;++j){
      tmp=char('A'+i%26-(j!=0))+tmp;
      i=i/26;
      if (!i)
	break;
    }
    return tmp;
  }

  // return true if s parsed has identifiers which are cell names
  bool has_cell(const string & s,gen & cell,GIAC_CONTEXT){
    gen g(s,contextptr);
    vecteur l(lidnt(g));
    int ls=l.size(),r,c;
    for (int i=0;i<ls;++i){
      if (iscell((cell=l[i]),r,c,contextptr))
	return true;
    }
    return false;
  }

  int xcas_integration_method=0;
  Fl_Output * Xcas_Methodes_Output=0;
  void cb_Xcas_Methodes_builtin(Fl_Menu_* m , void*) {
    xcas_integration_method=0;
    if (Xcas_Methodes_Output)
      Xcas_Methodes_Output->value(gettext("builtin integration"));
  }

  void cb_Xcas_Methodes_right_rectangle(Fl_Menu_* m , void*) {
    xcas_integration_method=1;
    if (Xcas_Methodes_Output)
      Xcas_Methodes_Output->value(gettext("right rectangle"));
  }

  void cb_Xcas_Methodes_left_rectangle(Fl_Menu_* m , void*) {
    xcas_integration_method=2;
    if (Xcas_Methodes_Output)
      Xcas_Methodes_Output->value(gettext("left rectangle"));
  }

  void cb_Xcas_Methodes_middle_point(Fl_Menu_* m , void*) {
    xcas_integration_method=3;
    if (Xcas_Methodes_Output)
      Xcas_Methodes_Output->value(gettext("middle point"));
  }

  void cb_Xcas_Methodes_trapezoid(Fl_Menu_* m , void*) {
    xcas_integration_method=4;
    if (Xcas_Methodes_Output)
      Xcas_Methodes_Output->value(gettext("trapezoid"));
  }

  Fl_Menu_Item Xcas_Menu_Methodes[] = {
    {gettext("builtin integration"), 0,  (Fl_Callback*)cb_Xcas_Methodes_builtin, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
    {gettext("right_rectangle"), 0,  (Fl_Callback*)cb_Xcas_Methodes_right_rectangle, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
    {gettext("left_rectangle"), 0,  (Fl_Callback*)cb_Xcas_Methodes_left_rectangle, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
    {gettext("middle_point"), 0,  (Fl_Callback*)cb_Xcas_Methodes_middle_point, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
    {gettext("trapezoid"), 0,  (Fl_Callback*)cb_Xcas_Methodes_trapezoid, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
    {0,0,0,0,0,0,0,0,0}
  };

  // type 1= plotfunc 3d, 0 plotfunc 2d, 2 plotarea
  bool tablefunc_dialog(Fl_Widget * spread_ptr,std::string & arg,bool plot,int type,const string & title){
    static Fl_Window * w = 0;
    static Fl_Input *fcn=0, *fcn3d=0,* varname=0, * varnamey=0,*target=0; 
    static Fl_Value_Input * xmin=0,*xstep=0,*xmax=0;
    static Fl_Value_Input * ymin=0,*ystep=0,*ymax=0;
    static Fl_Return_Button * button0 = 0 ;
    static Fl_Menu_Button * methode=0;
    static Fl_Button * button1 =0;
    static Line_Type * ltres=0;
    if (!w){
#ifdef IPAQ
      int dx=240,dy=300;
#else
      int dx=(spread_ptr?20*spread_ptr->labelsize():400), 
	dy=spread_ptr?8*spread_ptr->labelsize():300;
#endif
      int lignes=5;
      Fl_Group::current(0);
      w=new Fl_Window(dx,dy);
      ltres = new Line_Type(2,2,dx/15,dy/lignes-4,_MAGENTA+_FILL_POLYGON);
      ltres->show_pnt(true);
      ltres->show_poly(true);
      ltres->tooltip(gettext("Plot style (select a point type or width for dotted plot)"));
      fcn=new Fl_Input(dx/2,2,dx/2-4,dy/lignes-4,gettext("Expression"));
      fcn->value("x^2");
      fcn->tooltip(gettext("Expression of the function (e.g sin(x))"));
      fcn3d=new Fl_Input(dx/2,2,dx/2-4,dy/lignes-4,gettext("Expression"));
      fcn3d->value("x^2-y^2");
      fcn3d->tooltip(gettext("Expression of the function (e.g x-y^2)"));
      varname=new Fl_Input(dx/4,2+dy/lignes,dx/4-4,dy/lignes-4,gettext("1st var"));
      varname->value("x");
      varname->tooltip(gettext("Independant variable name (e.g x)"));
      varnamey=new Fl_Input(3*dx/4,2+dy/lignes,dx/4-4,dy/lignes-4,gettext("2nd var"));
      varnamey->value("y");
      varnamey->tooltip(gettext("Second independant variable name (e.g y)"));
      xmin = new Fl_Value_Input(dx/6,2+2*dy/lignes,dx/6-2,dy/lignes-4,gettext("xmin"));
      xmin->value(-4);
      xmin->step(0.5);
      xmin->tooltip(gettext("Minimal value for 1st independant variable"));
      xstep = new Fl_Value_Input(3*dx/6,2+2*dy/lignes,dx/6-2,dy/lignes-4,gettext("xstep"));
      xstep->value(0.25);
      xstep->step(0.01);
      xstep->tooltip(gettext("Discretization step for 1st independant variable"));
      xmax = new Fl_Value_Input(5*dx/6,2+2*dy/lignes,dx/6-2,dy/lignes-4,gettext("xmax"));
      xmax->value(4);
      xmax->step(0.5);
      xmax->tooltip(gettext("Maximal value for 1st independant variable"));
      target = new Fl_Input(dx/6,2+3*dy/lignes,dx/6-2,dy/lignes-4,gettext("Target"));
      target->tooltip(gettext("Name of the first of two columns that will be overwritten by the tablefunc"));
      ymin = new Fl_Value_Input(dx/6,2+3*dy/lignes,dx/6-2,dy/lignes-4,gettext("ymin"));
      ymin->value(-4);
      ymin->step(0.5);
      ymin->tooltip(gettext("Minimal value for 2nd independant variable"));
      ystep = new Fl_Value_Input(3*dx/6,2+3*dy/lignes,dx/6-2,dy/lignes-4,gettext("ystep"));
      ystep->value(0.25);
      ystep->step(0.01);
      ystep->tooltip(gettext("Discretization step for 2nd independant variable"));
      ymax = new Fl_Value_Input(5*dx/6,2+3*dy/lignes,dx/6-2,dy/lignes-4,gettext("ymax"));
      ymax->value(4);
      ymax->step(0.5);
      ymin->tooltip(gettext("Maximal value for 2nd independant variable"));
      methode = new Fl_Menu_Button(0,2+3*dy/lignes,dx/3-2,dy/lignes-4,gettext("Choose"));
      methode->menu(Xcas_Menu_Methodes);
      Xcas_Methodes_Output = new Fl_Output(4*dx/6,2+3*dy/lignes,dx/3-2,dy/lignes-4,gettext("int. method"));
      Xcas_Methodes_Output->value(gettext("builtin integration"));
      button0 = new Fl_Return_Button(2,2+4*dy/lignes,dx/2-4,dy/lignes-4);
      button0->shortcut(0xff0d);
      button0->label(gettext("OK"));
      button1 = new Fl_Button(dx/2+2,2+4*dy/lignes,dx/2-4,dy/lignes-4);
      button1->shortcut(0xff1b);
      button1->label(gettext("Cancel"));
      w->end();
      change_group_fontsize(w,spread_ptr?spread_ptr->labelsize():14);
      w->resizable(w);
    }
    w->label(title.c_str());
    Flv_Table_Gen * tbl=dynamic_cast<Flv_Table_Gen *>(spread_ptr);
    if (tbl)
      target->value(coltocolname(tbl->col()).c_str());
    if (plot){
      target->hide();
    }
    else {
      target->show();
    }
    if (type==2){
      methode->show();
      Xcas_Methodes_Output->show();
    }
    else {
      methode->hide();
      Xcas_Methodes_Output->hide();
    }
    if (type==1){
      ymax->show();
      ymin->show();
      ystep->show();
      varnamey->show();
      fcn->hide();
      fcn3d->show();
    }
    else {
      ymax->hide();
      ymin->hide();
      ystep->hide();
      varnamey->hide();
      fcn->show();
      fcn3d->hide();
    }
    int r=-1;
    w->set_modal();
    w->show();
    autosave_disabled=true;
    w->hotspot(w);
    Fl::focus(varname);
    for (;;) {
      Fl_Widget *o = Fl::readqueue();
      if (!o) Fl::wait();
      else {
	if (o==ltres){
	  int i=ltres->line_type();
	  bool formel=false,untranslate=false,approx=false;
	  change_line_type(i,true,approx,"",fcn3d->visible(),formel,untranslate,false,spread_ptr?spread_ptr->labelsize():14);
	  ltres->line_type(i);
	}
	if (o == button0) {r = 0; break;}
	if (o == button1) {r = 1; break;}
	if (o == w) { r=1; break; }
      }
    }
    autosave_disabled=false;
    w->hide();
    if (!r){
      if (tbl){
	int r;
	if (!alphaposcell(target->value(),r)){
	  fl_alert("%s","Invalid column");
	  return false;
	}
	tbl->col(r);
      }
      arg=(type==1?fcn3d:fcn)->value();
      gen cell;
      if (has_cell(arg,cell,context0)){
	fl_alert("%s",("Expression contains a cell name "+cell.print()).c_str());
	return false;
      }
      arg += string(",");
      if (plot){
	if (type==1){
	  arg += "["+string(varname->value())+"="+print_DOUBLE_(xmin->value())+".."+print_DOUBLE_(xmax->value()) + string(",")+varnamey->value()+string("=")+print_DOUBLE_(ymin->value())+".."+print_DOUBLE_(ymax->value())+"],xstep="+print_DOUBLE_(xstep->value())+",ystep="+print_DOUBLE_(ystep->value())+",display="+print_color(ltres->line_type());
	}
	else {
	  arg += string(varname->value())+"="+print_DOUBLE_(xmin->value())+".."+print_DOUBLE_(xmax->value());
	  if (type==2 && xcas_integration_method){
	    arg += ","+print_INT_(int((xmax->value()-xmin->value())/xstep->value()));
	    switch (xcas_integration_method){
	    case 1:
	      arg += ",right_rectangle";
	      break;
	    case 2:
	      arg += ",left_rectangle";
	      break;
	    case 3:
	      arg += ",middle_point";
	      break;
	    case 4:
	      arg += ",trapezoid";
	      break;
	    }
	  }
	  else
	    arg += ",xstep="+print_DOUBLE_(xstep->value());
	  arg += ",display="+print_color(ltres->line_type());
	}
      }
      else 
	arg += string(varname->value())+","+print_DOUBLE_(xmin->value())+","+print_DOUBLE_(xstep->value())+","+print_DOUBLE_(xmax->value())+",display="+print_color(ltres->line_type());
      return true;
    }
    return false;
  }

   void cb_Tableur_Tablefunc(Fl_Menu_* m , void*) {
    Flv_Table_Gen * spread_ptr=find_table_brother(m);
    if (!spread_ptr) return;
    string arg;
    if (spread_ptr && tablefunc_dialog(spread_ptr,arg,false,0,gettext("Table of value of a function"))){
      arg="tablefunc("+arg+")";
      spread_ptr->input->value("");
      tableur_insert_replace(spread_ptr,arg);
      Tableur_Group * gr = dynamic_cast<Tableur_Group *>(spread_ptr->parent());
      if (gr && !(gr->disposition & 2) && !spread_ptr->graph2d->window()->visible() ){
	gr->disposition |= 2;
	gr->resize2();
      }
      spread_ptr->input->do_callback();
      // browser_help(gen("tablefunc"),language(get_context(spread_ptr)));
    }
  }

  bool random_dialog(Fl_Widget * spread_ptr,gen & arg,double & Xmin,double & Xmax,bool & intg,bool & once){
    static Fl_Window * w = 0;
    static Fl_Input *target=0; 
    static Fl_Value_Input *xmin=0,*xmax=0;
    static Fl_Check_Button * intg_button=0, * once_button=0;
    static Fl_Return_Button * button0 = 0 ;
    static Fl_Button * button1 =0;
    if (!w){
      int lignes=4;
#ifdef IPAQ
      int dx=240,dy=300;
#else
      int dx=(spread_ptr?15*spread_ptr->labelsize():240), 
	dy=spread_ptr?8*spread_ptr->labelsize():300;
#endif
      Fl_Group::current(0);
      w=new Fl_Window(dx,dy);
      target=new Fl_Input(dx/2,2,dx/2-2,dy/lignes-4,gettext("Target cell range"));
      target->tooltip(gettext("Target cell range will be filled by random numbers, e.g. A1:A8"));
      xmin=new Fl_Value_Input(dx/4,2+dy/lignes,dx/4-2,dy/lignes-4,gettext("x-"));
      xmin->value(1);
      xmin->tooltip(gettext("Minimal value"));
      xmax = new Fl_Value_Input(dx/2+dx/4,2+dy/lignes,dx/4-2,dy/lignes-4,gettext("x+"));
      xmax->value(6);
      xmax->tooltip(gettext("Maximal value"));
      intg_button=new Fl_Check_Button(4, 2+2*dy/lignes,dx/4,dy/lignes-4,gettext("integers"));
      intg_button->tooltip(gettext("Real or integers"));
      intg_button->down_box(FL_DOWN_BOX);
      intg_button->value(true);
      once_button=new Fl_Check_Button(dx/2+4, 2+2*dy/lignes,dx/4,dy/lignes-4,gettext("static"));
      once_button->tooltip(gettext("Immediate static value or formula reevaled each time"));
      once_button->down_box(FL_DOWN_BOX);
      once_button->value(true);
      button0 = new Fl_Return_Button(2,2+(lignes-1)*dy/lignes,dx/2-4,dy/lignes-4);
      button0->shortcut(0xff0d);
      button0->label(gettext("OK"));
      button1 = new Fl_Button(dx/2+2,2+(lignes-1)*dy/lignes,dx/2-4,dy/lignes-4);
      button1->shortcut(0xff1b);
      button1->label(gettext("Cancel"));
      w->end();
      change_group_fontsize(w,spread_ptr?spread_ptr->labelsize():14);
      w->resizable(w);
    }
    w->label("Random numbers");
    Flv_Table_Gen * tbl=dynamic_cast<Flv_Table_Gen *>(spread_ptr);
    if (tbl && strlen(tbl->_goto->value()))
      target->value(tbl->_goto->value());
    int r=-1;
    w->set_modal();
    w->show();
    autosave_disabled=true;
    w->hotspot(w);
    Fl::focus(target);
    for (;;) {
      Fl_Widget *o = Fl::readqueue();
      if (!o) Fl::wait();
      else {
	if (o == button0) {r = 0; break;}
	if (o == button1) {r = 1; break;}
	if (o == w) { r=1; break; }
      }
    }
    autosave_disabled=false;
    w->hide();
    if (r==1)
      return false;
    intg=intg_button->value();
    once=once_button->value();
    arg=parse_interval(target->value());
    Xmin=xmin->value();
    Xmax=xmax->value();
    return true;
  }

  bool tableseq_dialog(Fl_Widget * spread_ptr,std::string & arg,bool plot,const string & title,std::string & u0param){
    static Fl_Window * w = 0;
    static Fl_Input *fcn=0, * varname=0; 
    static Fl_Value_Input *xmin=0,*xmax=0;
    static Fl_Input *u0name=0,* u0=0,* target=0;
    static Fl_Value_Input * nterms=0;
    static Fl_Return_Button * button0 = 0 ;
    static Fl_Button * button1 =0;
    if (!w){
      int lignes=5;
#ifdef IPAQ
      int dx=240,dy=300;
#else
      int dx=(spread_ptr?15*spread_ptr->labelsize():240), 
	dy=spread_ptr?8*spread_ptr->labelsize():300;
#endif
      Fl_Group::current(0);
      w=new Fl_Window(dx,dy);
      fcn=new Fl_Input(dx/2,2,dx/2-4,dy/lignes-4,gettext("Expression"));
      fcn->value("(x+2)/(x+1)");
      fcn->tooltip(gettext("Expression of u_(n+1) in terms of a variable=u_n, e.g. 1/2*(x+2/x)\nOr expression of u_(n+k+1) in terms of a list of variables=[u_n,...,u_(n+k)], e.g. x+y"));
      varname=new Fl_Input(dx/2,2+dy/lignes,dx/2-4,dy/lignes-4,gettext("Variable(s)"));
      varname->value("x");
      varname->tooltip(gettext("Independant variable name representing u_n, e.g. x\nOr list of independant variables representing u_n,...,u_(n+k), e.g. [x,y]"));
      u0name = new Fl_Input(0,2+2*dy/lignes,dx/8,dy/lignes-4);
      u0name->value("u0");
      u0name->tooltip(gettext("Name of the initial value of un"));
      u0 = new Fl_Input(dx/6,2+2*dy/lignes,dx/3,dy/lignes-4,gettext("="));
      u0->value("1.0");
      u0->tooltip(gettext("Initial value, e.g. 1.0\nOr list of initial values, e.g. [1.0,1.0]"));
      xmin = new Fl_Value_Input(dx/2+dx/8,2+2*dy/lignes,dx/8-2,dy/lignes-4,gettext("x-"));
      xmin->value(gnuplot_xmin);
      xmin->tooltip(gettext("Minimal value for the function graph"));
      xmax = new Fl_Value_Input(dx/2+3*dx/8,2+2*dy/lignes,dx/8-2,dy/lignes-4,gettext("x+"));
      xmax->value(gnuplot_xmax);
      xmax->tooltip(gettext("Maximal value for the function graph"));
      target = new Fl_Input(dx/2,2+3*dy/lignes,dx/2-2,dy/lignes-4,gettext("Target column"));
      target->tooltip("Name of the column that will be overwritten by the table of the sequence");
      nterms = new Fl_Value_Input(dx/2,2+3*dy/lignes,dx/2-2,dy/lignes-4,gettext("Number of terms"));
      nterms->value(10);
      nterms->step(1);
      button0 = new Fl_Return_Button(2,2+4*dy/lignes,dx/2-4,dy/lignes-4);
      button0->shortcut(0xff0d);
      button0->label(gettext("OK"));
      button1 = new Fl_Button(dx/2+2,2+4*dy/lignes,dx/2-4,dy/lignes-4);
      button1->shortcut(0xff1b);
      button1->label(gettext("Cancel"));
      w->end();
      change_group_fontsize(w,spread_ptr?spread_ptr->labelsize():14);
      w->resizable(w);
    }
    Flv_Table_Gen * tbl=dynamic_cast<Flv_Table_Gen *>(spread_ptr);
    if (tbl){
      target->show();
      target->value(coltocolname(tbl->col()).c_str());
      nterms->hide();
      u0name->hide();
      u0->label("u0");
    }
    else {
      target->hide();
      nterms->show();
      u0name->show();
      u0->label("=");
    }
    w->label(title.c_str());
    int r=-1;
    w->set_modal();
    w->show();
    autosave_disabled=true;
    w->hotspot(w);
    Fl::focus(fcn);
    for (;;) {
      Fl_Widget *o = Fl::readqueue();
      if (!o) Fl::wait();
      else {
	if (o == button0) {r = 0; break;}
	if (o == button1) {r = 1; break;}
	if (o == w) { r=1; break; }
      }
    }
    autosave_disabled=false;
    w->hide();
    if (!r){
      if (tbl){
	int r;
	if (!alphaposcell(target->value(),r)){
	  fl_alert("%s","Invalid column");
	  return false;
	}
	tbl->col(r);
	tbl->row(1);
	if (Tableur_Group * par=dynamic_cast<Tableur_Group *> (tbl->parent())){
	  par->disposition=3;
	  par->resize2();
	}
      }
      gen cell;
      if (has_cell(fcn->value(),cell,context0)){
	fl_alert("%s",(gettext("Expression contains a cell name ")+cell.print()).c_str());
	return false;
      }
      if (gen(u0->value(),context0).type==_VECT){
	fl_alert("%s",(string(gettext("Invalid list initial value "))+u0->value()).c_str());
	return false;
      }
      if (Figure * fig = dynamic_cast<Figure *>(spread_ptr)){
	fig->geo->window_xmin=xmin->value();
	fig->geo->window_xmax=xmax->value();
	fig->geo->orthonormalize();
      }
      u0param="assume("+string(u0name->value())+string("=[")+u0->value()+string(",")+print_DOUBLE_(xmin->value())+string(",")+print_DOUBLE_(xmax->value())+string("])");
      arg=fcn->value()+string(",")+varname->value();
      string u0s=tbl?u0->value():u0name->value();
      if (plot)
	arg += string("=[")+u0s+string(",")+print_DOUBLE_(xmin->value())+string(",")+print_DOUBLE_(xmax->value())+string("],")+print_DOUBLE_(nterms->value());
      else
	arg += string(",[")+u0s+string(",")+print_DOUBLE_(xmin->value())+string(",")+print_DOUBLE_(xmax->value())+string("]");
      return true;
    }
    return false;
  }

   void cb_Tableur_ranm(Fl_Menu_* m , void*) {
    Flv_Table_Gen * spread_ptr=find_table_brother(m);
    if (!spread_ptr) return;
    const giac::context * contextptr = get_context(spread_ptr);
    gen g;
    bool intg,once;
    double xmin,xmax;
    if (spread_ptr && random_dialog(spread_ptr,g,xmin,xmax,intg,once)){
      int r,c;
      string formulas;
      if (!once)
	formulas="=";
      if (intg)
	formulas+="floor(";
      formulas += "rand(";
      formulas += print_DOUBLE_(xmin);
      formulas += ",";
      formulas += print_DOUBLE_(intg?xmax+1:xmax);
      formulas += ")";
      if (intg)
	formulas+=")";
      if (iscell(g,c,r,contextptr) ){
	if (r>=spread_ptr->rows()||c>=spread_ptr->cols())
	  spread_ptr->resizesheet(max(r+1,spread_ptr->rows()),max(c+1,spread_ptr->cols()));
	// cerr << g << " " << r << " " << c << endl;
	spread_ptr->row(r);
	spread_ptr->col(c);
	spread_ptr->select_start_row(r);
	spread_ptr->select_start_col(c);
	if (spread_ptr->input){
	  spread_ptr->input->value(formulas.c_str());
	  spread_ptr->input->do_callback();
	}
      } // end if iscell(g,r,c)
      if (!g.is_symb_of_sommet(at_interval))
	return;
      gen & f=g._SYMBptr->feuille;
      if (f.type!=_VECT || f._VECTptr->size()!=2)
	return;
      int r1,r2,c1,c2;
      if (!iscell(f._VECTptr->front(),c1,r1,contextptr) || !iscell(f._VECTptr->back(),c2,r2,contextptr))
	return;
      if (r1>r2)
	giac::swapint(r1,r2);
      r2=min(r2+1,spread_ptr->rows());
      r1=max(r1,0);
      if (c1>c2)
	giac::swapint(c1,c2);
      c2=min(c2+1,spread_ptr->cols());
      c1=max(c1,0);
      gen formula(formulas,contextptr);
      for (int i=r1;i<r2;++i){
	vecteur & v=*spread_ptr->m[i]._VECTptr;
	for (int j=c1;j<c2;++j){
	  v[j]=makevecteur((once?eval(formula,1,0):formula),0,2);
	}
      }
      spread_ptr->spread_eval_interrupt();
      spread_ptr->redraw();
    }
  }

   void cb_Tableur_Tableseq(Fl_Menu_* m , void*) {
    Flv_Table_Gen * spread_ptr=find_table_brother(m);
    if (!spread_ptr) return;
    string arg,u0param;
    if (spread_ptr && tableseq_dialog(spread_ptr,arg,false,gettext("Table of value of a recurrent sequence"),u0param)){
      arg="tableseq("+arg+")";
      spread_ptr->input->value("");
      tableur_insert_replace(spread_ptr,arg);
      Tableur_Group * gr = dynamic_cast<Tableur_Group *>(spread_ptr->parent());
      if (gr && !(gr->disposition & 2) && !spread_ptr->graph2d->window()->visible()){
	gr->disposition |= 2;
	gr->resize2();
      }
      spread_ptr->input->do_callback();
      // browser_help(gen("tableseq"),language(get_context(spread_ptr)));
    }
  }

  int Flv_Table_Gen::set_graphic_dialog(const string & title,const string & aide,gen & in, gen & out,bool show_class,bool & absolu,bool & transpose,double & degree){
    const giac::context * contextptr = get_context(this);
    static Fl_Window * w = 0;
    static Fl_Output * help=0;
    static Fl_Input * i1=0, * i2=0; // Cells to analyse and cell output
    static Fl_Value_Input * v1=0, *v2=0,*v3=0; // Class_min/size, poly degree
    static Fl_Return_Button * button0 = 0 ;
    static Fl_Button * button1 =0;
    static Fl_Check_Button* c1=0,*c2=0,*c3=0;
    int dx=(2*this->w())/3,dy=(2*h())/3;
    if (dy<200)
      dy=200;
    if (dy>6*labelsize())
      dy=6*labelsize();
    if (dx<200)
      dx=200;
    if (!w){
      Fl_Group::current(0);
      w=new Fl_Window(dx,dy);
      int lignes=4;
      help = new Fl_Output(dx/2,2,0,0,"");
      help->align(FL_ALIGN_BOTTOM);
      i1=new Fl_Input(dx/4,2+dy/lignes,dx/4-2,dy/lignes-4,gettext("cells input"));
      i1->tooltip(gettext("Zone over which the function will be applied, e.g. A1..B7 or A1..B7,D,E"));
      i2=new Fl_Input((3*dx)/4,2+dy/lignes,dx/4-2,dy/lignes-4,gettext("target cell"));
      i2->tooltip(gettext("Target cell(s) will be filled by the formula, e.g. A8"));
      v1=new Fl_Value_Input(dx/4,2+2*dy/lignes,dx/4-2,dy/lignes-4,gettext("class min"));
      v1->tooltip(gettext("Minimal value of classes for histogramsl, e.g. -10"));
      v2=new Fl_Value_Input((3*dx)/4,2+2*dy/lignes,dx/4-2,dy/lignes-4,gettext("class size"));
      v2->tooltip(gettext("Size of a class for histograms, e.g. 2"));
      c1=new Fl_Check_Button(0, 2+2*dy/lignes,dx/8,dy/lignes-4,gettext("value"));
      c1->tooltip(gettext("Use submatrix by reference (dynamic) or by value (static)"));
      c1->down_box(FL_DOWN_BOX);
      c2=new Fl_Check_Button(dx/4, 2+2*dy/lignes,dx/8,dy/lignes-4,gettext("lines"));
      c2->tooltip(gettext("Use line or columns"));
      c2->down_box(FL_DOWN_BOX);
      c3=new Fl_Check_Button(dx/2, 2+2*dy/lignes,dx/8,dy/lignes-4,gettext("outside"));
      c3->tooltip(gettext("Sheet graph or standalone graph"));
      c3->down_box(FL_DOWN_BOX);
      v3=new Fl_Value_Input(3*dx/4+dx/8, 2+2*dy/lignes,dx/8,dy/lignes-4,gettext("degree"));
      v3->tooltip(gettext("Degree of the polynomial regression"));
      button0 = new Fl_Return_Button(2,2+(3*dy/lignes),dx/2-4,dy/lignes-4);
      button0->shortcut(0xff0d);
      button0->label(gettext("OK"));
      button1 = new Fl_Button(dx/2+2,2+(3*dy)/lignes,dx/2-4,dy/lignes-4);
      button1->shortcut(0xff1b);
      button1->label(gettext("Cancel"));
      w->end();
      w->resizable(w);
    }
    help->label(aide.c_str());
    w->label(title.c_str());
    change_group_fontsize(w,labelsize());
    /*
      printcell_current_row=row()+1;
      printcell_current_col=select_start_col();
      string s=printcell(makevecteur(makevecteur(0),makevecteur(0)));
      i2->value(s.c_str()); // removed so that the user MUST specify target
    */
    if (strlen(_goto->value()))
      i1->value(_goto->value());
    if (show_class){
      c3->value(false);
      c3->hide();
    }
    else
      c3->show();
    static string i2s;
    i2s=next_cell(i1->value(),this);
    i2->value(i2s.c_str());
    v1->value(class_minimum);
    v2->value(class_size);
    v3->value(degree);
    if (show_class){
      v1->show(); v2->show(); c1->hide(); c2->hide(); v3->hide(); 
    }
    else {
      v1->hide(); v2->hide(); c1->show();  c2->show();
      if (degree){
	v3->label(degree==1?"y1":"degree");
	v3->show();
      }
      else
	v3->hide(); 
    }
    int r=-1;
    for (;;){
      w->set_modal();
      w->show();
      autosave_disabled=true;
      w->hotspot(w);
      Fl::focus(i1);
      for (;;) {
	Fl_Widget *o = Fl::readqueue();
	if (!o) Fl::wait();
	else {
	  if (o == button0) {r = 0; break;}
	  if (o == button1) {r = 1; break;}
	  if (o==c3){ if (c3->value()) i2->deactivate(); else i2->activate();}
	  if (o == w) { r=1; break; }
	}
      }
      autosave_disabled=false;
      w->hide();
      if (r)
	break;
      // r=0, not cancelled
      degree=int(v3->value());
      absolu=c1->value();
      transpose=c2->value();
      class_minimum=v1->value();
      class_size=v2->value();
      const char * ch =i1->value();
      string s;
      int l=strlen(ch);
      for (int i=0;i<l;++i,++ch){
	if (*ch==':')
	  s += "..";
	else
	  s+=*ch;
      }
      try {
	in=gen(s,contextptr);
	out=(c3->value() || !strlen(i2->value()))?undef:gen(i2->value(),contextptr);
	Tableur_Group * gr = dynamic_cast<Tableur_Group *>(parent());
	if (!is_undef(out) && gr && !(gr->disposition & 2) ){
	  gr->disposition |= 2;
	  gr->resize2();
	}
      }
      catch (std::runtime_error & e){
	fl_message("%s",(gettext("Syntax error")+string(e.what())).c_str());
	continue;
      }
      break;
    } // end endless for(;;) loop
    return r;
  }

  gen interval2deuxpoints(const gen & g){
    return g.is_symb_of_sommet(at_interval)?symbolic(at_deuxpoints,g._SYMBptr->feuille):g;
  }

  void Flv_Table_Gen::set_graphic(const gen & function,const std::string & aide){
    if (function.type!=_FUNC)
      return;
    gen in,out;
    bool absolu=false,transpose;
    string fs=function.print(contextptr);
    double deg=0;
    bool polyreg=(fs=="'polynomial_regression_plot'");
    bool logisreg=(fs=="'logistic_regression_plot'");
    if (polyreg)
      deg=2;
    if (logisreg)
      deg=1;
    int r=set_graphic_dialog(fs,aide,in,out,fs=="'histogram'",absolu,transpose,deg);
    if (r==0){
      gen tmp;
      int r1,r2,res,R,C;
      vector<int> vc;
      matrice mselect;
      bool absolu2=false;
      if ( (res=iscell_range(in,m,mselect,this,r1,r2,vc,absolu2)) ){
	bool inside=iscell(out,C,R,contextptr);
	// If vc.size()!=#cols mselect colmuns are non-contiguous, 
	// use mselect else use matrix(r2-r1+1,vc.size(),g)
	if (absolu || mselect.front()._VECTptr->size()!=vc.size() || (inside?absolu2:name.type!=_IDNT) ){
	  tmp=extractmatricefromsheet(mselect);
	  if (transpose)
	    tmp=_tran(tmp,contextptr);
	}
	else {
	  if (inside){
	    in=apply(in,interval2deuxpoints);
	    tmp=logisreg?in:symbolic(at_matrix,makevecteur(r2-r1,vc.size(),in));
	  }
	  else {
	    vecteur vcg;
	    vector_int2vecteur(vc,vcg);
	    in=gen(makevecteur(symb_interval(r1,r2-1),vcg),_SEQ__VECT);
	    tmp=symbolic(at_at,gen(makevecteur(name,in),_SEQ__VECT));
	  }
	  if (transpose)
	    tmp=symbolic(at_tran,tmp);
	}
	if (polyreg)
	  tmp=gen(makevecteur(tmp,deg),_SEQ__VECT);
	if (logisreg)
	  tmp=gen(makevecteur(tmp,1,deg),_SEQ__VECT);
	tmp=symbolic(*function._FUNCptr,tmp);
	if (inside){
	  tmp.change_subtype(_SPREAD__SYMB);
	  row(R);
	  col(C);
	  input->value(tmp.print(contextptr).c_str());
	  // input->set_g(tmp);
	  input->do_callback();
	}
	else {
	  int pos;
	  History_Pack * hp=get_history_pack(this,pos);
	  if (hp){
	    hp->add_entry(pos+1);
	    hp->set_gen_value(pos+1,tmp,true);
	  }
	}
      }
    }
  }

   void cb_Tableur_SetRows(Fl_Menu_* m , void*) {
    Flv_Table_Gen * tg=find_table_brother(m);
    if (!tg) return;
    const char * ch=fl_input(gettext("New row number"),print_INT_(tg->rows()).c_str());
    if (ch){
      int i=atoi(ch);
      if (i<tg->rows()){
	int j=fl_ask("%s",gettext("Really delete rows?"));
	if (!j)
	  return;
      }
      if (i>0 && double(i)*tg->cols()<7e4)
	tg->resizesheet(i,tg->cols());
    }
  }

   void cb_Tableur_SetCols(Fl_Menu_* m , void*) {
    Flv_Table_Gen * tg=find_table_brother(m);
    if (!tg) return;
    const char * ch=fl_input(gettext("New col number"),print_INT_(tg->cols()).c_str());
    if (ch){
      int i=atoi(ch);
      if (i<tg->cols()){
	int j=fl_ask("%s",gettext("Really delete columns?"));
	if (!j)
	  return;
      }
      if (i>0 && double(i)*tg->rows()<7e4){
	tg->resizesheet(tg->rows(),i);
      }
    }
  }

   void cb_Tableur_Histogram(Fl_Menu_* m , void*) {
    Flv_Table_Gen * tg=find_table_brother(m);
    if (!tg) return;
    const giac::context * contextptr = get_context(tg);
    tg->set_graphic(gen("histogram",contextptr),gettext("Histogram 1 or 2 columns: data or data/eff"));
  }

   void cb_Tableur_Classes(Fl_Menu_* m , void*) {
    Flv_Table_Gen * tg=find_table_brother(m);
    if (!tg) return;
    const giac::context * contextptr = get_context(tg);
    gen in,out;
    bool absolu,transpose; // always true for histograms
    double deg=0;
    int r=tg->set_graphic_dialog("classes",gettext("Make classes from column data or data/eff into two adjacent columns"),in,out,true,absolu,transpose,deg);
    if (r==0){
      gen tmp;
      int r1,r2,R,C;
      vector<int> vc;
      matrice mselect;
      if ( (iscell_range(in,tg->m,mselect,tg,r1,r2,vc,absolu)==1) && iscell(out,C,R,contextptr) ){
	matrice g;
	try {
	  tg->select_start_row(r1); tg->row(r2-1);
	  tg->select_start_col(vc.front()); tg->col(vc.back());
	  tg->copy();
	  matrice tmp=extractmatricefromsheet(tg->selected);
	  if (transpose)
	    tmp=mtran(tmp);
	  g=effectifs(tmp,class_minimum,class_size,get_context(tg));
	} catch (std::runtime_error & e){
	  return;
	}
	tg->row(R); tg->col(C);
	tg->paste(g);
      }      
    }
  }

   void cb_Tableur_Boxwhisker(Fl_Menu_* m , void*) {
    Flv_Table_Gen * tg=find_table_brother(m);
    if (!tg) return;
    const giac::context * contextptr = get_context(tg);
    tg->set_graphic(gen("boxwhisker",contextptr),gettext("One boxwhisker per column"));
  }

   void cb_Tableur_plotlist(Fl_Menu_* m , void*) {
    Flv_Table_Gen * tg=find_table_brother(m);
    if (!tg) return;
    const giac::context * contextptr = get_context(tg);
    tg->set_graphic(gen("plotlist",contextptr),gettext("Plotlist: One column (y) or two columns (x,y)"));
  }

   void cb_Tableur_camembert(Fl_Menu_* m , void*) {
    Flv_Table_Gen * tg=find_table_brother(m);
    if (!tg) return;
    const giac::context * contextptr = get_context(tg);
    tg->set_graphic(gen("camembert",contextptr),gettext("Camembert: column 1: legends, column 2: data"));
  }

   void cb_Tableur_batons(Fl_Menu_* m , void*) {
    Flv_Table_Gen * tg=find_table_brother(m);
    if (!tg) return;
    const giac::context * contextptr = get_context(tg);
    tg->set_graphic(gen("bar_plot",contextptr),gettext("Bar_plot: column 1: legends, column 2: data"));
  }

   void cb_Tableur_Scatterplot(Fl_Menu_* m , void*) {
    Flv_Table_Gen * tg=find_table_brother(m);
    if (!tg) return;
    const giac::context * contextptr = get_context(tg);
    tg->set_graphic(gen("scatterplot",contextptr),gettext("Column 1: x, one scatterplot per remaining column"));
  }

   void cb_Tableur_Polygonplot(Fl_Menu_* m , void*) {
    Flv_Table_Gen * tg=find_table_brother(m);
    if (!tg) return;
    const giac::context * contextptr = get_context(tg);
    tg->set_graphic(gen("polygonplot",contextptr),gettext("Column 1: x, one polygonplot per remaining column"));
  }

   void cb_Tableur_Polygonscatterplot(Fl_Menu_* m , void*) {
    Flv_Table_Gen * tg=find_table_brother(m);
    if (!tg) return;
    const giac::context * contextptr = get_context(tg);
    tg->set_graphic(gen("polygonscatterplot",contextptr),gettext("Column 1: x, one polygon line per remaining column"));
  }

   void cb_Tableur_linear_regression_plot(Fl_Menu_* m , void*) {
    Flv_Table_Gen * tg=find_table_brother(m);
    if (!tg) return;
    const giac::context * contextptr = get_context(tg);
    tg->set_graphic(gen("linear_regression_plot",contextptr),gettext("Linear regression: column 1: x, column 2: y"));
  }

   void cb_Tableur_power_regression_plot(Fl_Menu_* m , void*) {
    Flv_Table_Gen * tg=find_table_brother(m);
    if (!tg) return;
    const giac::context * contextptr = get_context(tg);
    tg->set_graphic(gen("power_regression_plot",contextptr),gettext("Power regression: Column 1: x, column 2: y"));
  }

   void cb_Tableur_logarithmic_regression_plot(Fl_Menu_* m , void*) {
    Flv_Table_Gen * tg=find_table_brother(m);
    if (!tg) return;
    const giac::context * contextptr = get_context(tg);
    tg->set_graphic(gen("logarithmic_regression_plot",contextptr),gettext("Log regression: Column 1: x, column 2: y"));
  }

   void cb_Tableur_exponential_regression_plot(Fl_Menu_* m , void*) {
    Flv_Table_Gen * tg=find_table_brother(m);
    if (!tg) return;
    const giac::context * contextptr = get_context(tg);
    tg->set_graphic(gen("exponential_regression_plot",contextptr),gettext("Exp regression: Column 1: x, column 2: y"));
  }

   void cb_Tableur_polynomial_regression_plot(Fl_Menu_* m , void*) {
    Flv_Table_Gen * tg=find_table_brother(m);
    if (!tg) return;
    const giac::context * contextptr = get_context(tg);
    tg->set_graphic(gen("polynomial_regression_plot",contextptr),gettext("Polynomial regression degree d: Column 1: x, column 2: y"));
  }

   void cb_Tableur_logistic_regression_plot(Fl_Menu_* m , void*) {
    Flv_Table_Gen * tg=find_table_brother(m);
    if (!tg) return;
    const giac::context * contextptr = get_context(tg);
    tg->set_graphic(gen("logistic_regression_plot",contextptr),gettext("Logistic regression for y', with y(x=1)=y1"));
  }

  int find_sort_row(int c2,bool isrow,GIAC_CONTEXT){
    int c;
    if (xcas_mode(contextptr))
      c=c2+1;
    else
      c=c2;
    string question=gettext("Sort with respect to ");
    if (isrow)
      question += gettext("row");
    else
      question += gettext("column");
    const char * chptr=fl_input("%s",print_INT_(c).c_str(),question.c_str());
    if (!chptr)
      return -1;
    string colonne(chptr);
    gen g(colonne,contextptr);
    if (xcas_mode(contextptr))
      g = g + minus_one;
    if (g.type!=_INT_ )
      return -1;
    return g.val;
  }

  // Global variable is used for sorting only.
  static Flv_Table_Gen * current_spread_ptr;
  bool thesheetsort(const gen & a,const gen &b){
    const giac::context * contextptr = get_context(current_spread_ptr);
    gen a1=a[current_spread_ptr->sort_col][1].evalf_double(1,contextptr),a2=b[current_spread_ptr->sort_col][1].evalf_double(1,contextptr); 
    if (a1.type!=_DOUBLE_ || a2.type!=_DOUBLE_)
      return a1.islesscomplexthan(a2);
    return a1._DOUBLE_val<a2._DOUBLE_val;
  }

  void sheetsort(Flv_Table_Gen * spread_ptr,bool row_sort,bool increasing){
    const giac::context * contextptr = get_context(spread_ptr);
    current_spread_ptr=spread_ptr;
    if (spread_ptr->is_spreadsheet){
      int i=fl_ask("%s",gettext("Sorting is not compatible with some cell references. Sort anyway"),gettext("Yes"),gettext("No"));
      if (i!=1) return ;
    }
    int r1,r2,c1,c2;
    matrice m;
    if (row_sort){ // Transpose if sort w.r. to current row
      m=mtran(spread_ptr->m);
      r1=spread_ptr->col();
      r2=spread_ptr->push_col;
      c1=spread_ptr->row();
      c2=spread_ptr->push_row;
      int r=find_sort_row(c2,true,contextptr);
      if (r<0 || r>=spread_ptr->rows())
	return;
      spread_ptr->sort_col=r;
    }
    else {
      m=spread_ptr->m;
      r1=spread_ptr->row();
      r2=spread_ptr->push_row;
      c1=spread_ptr->col();
      c2=spread_ptr->push_col;
      if (spread_ptr->is_spreadsheet){
	int r,c;
	string tmp=coltocolname(c2);
	string question=gettext("Sort with respect to ");
	if (row_sort)
	  question += gettext("row");
	else
	  question += gettext("column");
	const char * chptr=fl_input("%s",tmp.c_str(),question.c_str());
	if (!chptr){
	  return;
	}
	string therow=chptr;
	therow +='1';
	if (!iscell(gen(therow,contextptr),r,c,contextptr))
	  return;
	spread_ptr->sort_col=r;
      }
      else {
	int c=find_sort_row(c2,false,contextptr);
	if (c<0 || c>= spread_ptr->cols() )
	  return;
	spread_ptr->sort_col=c;
      }
    }
    if (r1==r2)
      return;
    if (r1>r2)
      giac::swapint(r1,r2);
    if (c1>c2)
      giac::swapint(c1,c2);
    // keep only rows r1->r2 and cols c1->c2
    m=mtran(matrice(m.begin()+r1,m.begin()+r2+1));
    m=mtran(matrice(m.begin()+c1,m.begin()+c2+1));
    // sort
    spread_ptr->sort_col -= c1;
    sort(m.begin(),m.end(),thesheetsort);
    if (!increasing)
      reverse(m.begin(),m.end());
    // put in the matrix
    if (row_sort){ // Back transpose
      m=mtran(m);
      spread_ptr->row(c1);
      spread_ptr->col(r1);
    }
    else {
      spread_ptr->row(r1);
      spread_ptr->col(c1);
    }
    spread_ptr->paste(m);
  }

   void cb_Tableur_Sort_Dec_Col(Fl_Menu_* m , void*) {
    Flv_Table_Gen * spread_ptr=find_table_brother(m);
    if (!spread_ptr) return;
    sheetsort(spread_ptr,false,false);
    spread_ptr->spread_eval_interrupt();
    spread_ptr->redraw();
  }

   void cb_Tableur_Sort_Inc_Col(Fl_Menu_* m , void*) {
    Flv_Table_Gen * spread_ptr=find_table_brother(m);
    if (!spread_ptr) return;
    sheetsort(spread_ptr,false,true);
    spread_ptr->spread_eval_interrupt();
    spread_ptr->redraw();
  }

   void cb_Tableur_Sort_Dec_Row(Fl_Menu_* m , void*) {
    Flv_Table_Gen * spread_ptr=find_table_brother(m);
    if (!spread_ptr) return;
    sheetsort(spread_ptr,true,false);
    spread_ptr->spread_eval_interrupt();
    spread_ptr->redraw();
  }

   void cb_Tableur_Sort_Inc_Row(Fl_Menu_* m , void*) {
    Flv_Table_Gen * spread_ptr=find_table_brother(m);
    if (!spread_ptr) return;
    sheetsort(spread_ptr,true,true);
    spread_ptr->spread_eval_interrupt();
    spread_ptr->redraw();
  }

   void cb_Tableur_Auto_Recompute(Fl_Menu_* m , void*) {
    Flv_Table_Gen * spread_ptr=find_table_brother(m);
    if (!spread_ptr) return;
    spread_ptr->spreadsheet_recompute=1;
    spread_ptr->update_status();
  }

   void cb_Tableur_No_Recompute(Fl_Menu_* m , void*) {
    Flv_Table_Gen * spread_ptr=find_table_brother(m);
    if (!spread_ptr) return;
    spread_ptr->spreadsheet_recompute=0;
    spread_ptr->update_status();
  }

   void cb_Tableur_Matrix_Fill(Fl_Menu_* m , void*) {
    Flv_Table_Gen * spread_ptr=find_table_brother(m);
    if (!spread_ptr) return;
    spread_ptr->matrix_fill_cells=1;
    spread_ptr->update_status();
  }

   void cb_Tableur_No_Matrix_Fill(Fl_Menu_* m , void*) {
    Flv_Table_Gen * spread_ptr=find_table_brother(m);
    if (!spread_ptr) return;
    spread_ptr->matrix_fill_cells=0;
    spread_ptr->update_status();
  }

   void cb_Tableur_Variable_Name(Fl_Menu_* m , void*) {
    Flv_Table_Gen * spread_ptr=find_table_brother(m);
    if (!spread_ptr) return;
    spread_ptr->config();
  }

   void cb_Tableur_Spreadsheet_Mode(Fl_Menu_* m , void*) {
    Flv_Table_Gen * spread_ptr=find_table_brother(m);
    if (!spread_ptr) return;
    spread_ptr->is_spreadsheet=1;
    spread_ptr->update_status();
  }

   void cb_Tableur_Matrix_Mode(Fl_Menu_* m , void*) {
    Flv_Table_Gen * spread_ptr=find_table_brother(m);
    if (!spread_ptr) return;
    spread_ptr->is_spreadsheet=0;
    spread_ptr->update_status();
  }

   void cb_Tableur_Normal_Matrix(Fl_Menu_* m , void*) {
    Flv_Table_Gen * spread_ptr=find_table_brother(m);
    if (!spread_ptr) return;
    spread_ptr->matrix_symmetry=0;
    spread_ptr->update_status();
  }

   void cb_Tableur_Symmetric_Matrix(Fl_Menu_* m , void*) {
    Flv_Table_Gen * spread_ptr=find_table_brother(m);
    if (!spread_ptr) return;
    if (spread_ptr->rows()!=spread_ptr->cols()){
      fl_message("%s",gettext("Make rows==cols first!"));
      return;
    }
    spread_ptr->is_spreadsheet=0;
    spread_ptr->matrix_symmetry=4;
    spread_ptr->update_status();
  }

   void cb_Tableur_AntiSymmetric_Matrix(Fl_Menu_* m , void*) {
    Flv_Table_Gen * spread_ptr=find_table_brother(m);
    if (!spread_ptr) return;
    if (spread_ptr->rows()!=spread_ptr->cols()){
      fl_message("%s",gettext("Make rows==cols first!"));
      return;
    }
    spread_ptr->is_spreadsheet=0;
    spread_ptr->matrix_symmetry=3;
    spread_ptr->update_status();
  }

   void cb_Tableur_Hermitian_Matrix(Fl_Menu_* m , void*) {
    Flv_Table_Gen * spread_ptr=find_table_brother(m);
    if (!spread_ptr) return;
    if (spread_ptr->rows()!=spread_ptr->cols()){
      fl_message("%s",gettext("Make rows==cols first!"));
      return;
    }
    spread_ptr->is_spreadsheet=0;
    spread_ptr->matrix_symmetry=2;
    spread_ptr->update_status();
  }

   void cb_Tableur_AntiHermitian_Matrix(Fl_Menu_* m , void*) {
    Flv_Table_Gen * spread_ptr=find_table_brother(m);
    if (!spread_ptr) return;
    if (spread_ptr->rows()!=spread_ptr->cols()){
      fl_message("%s",gettext("Make rows==cols first!"));
      return;
    }
    spread_ptr->is_spreadsheet=0;
    spread_ptr->matrix_symmetry=1;
    spread_ptr->update_status();
  }

   void cb_Tableur_Hide_Graph(Fl_Menu_* m , void*) {
    Flv_Table_Gen * spread_ptr=find_table_brother(m);
    if (!spread_ptr) return;
    Tableur_Group * tg=dynamic_cast<Tableur_Group *>(spread_ptr->parent());
    if (tg){
      if (tg->disposition & 0x2)
	tg->save_dparam();
      tg->disposition=0;
      tg->resize2();
      tg->redraw();
    }
  }

   void cb_Tableur_Portrait(Fl_Menu_* m , void*) {
    Flv_Table_Gen * spread_ptr=find_table_brother(m);
    if (!spread_ptr) return;
    Tableur_Group * tg=dynamic_cast<Tableur_Group *>(spread_ptr->parent());
    if (tg){
      tg->disposition=3;
      tg->resize2();
      tg->redraw();
    }
  }

   void cb_Tableur_Landscape(Fl_Menu_* m , void*) {
    Flv_Table_Gen * spread_ptr=find_table_brother(m);
    if (!spread_ptr) return;
    Tableur_Group * tg=dynamic_cast<Tableur_Group *>(spread_ptr->parent());
    if (tg){
      tg->disposition=2;
      tg->resize2();
      tg->redraw();
    }
  }

   void cb_Tableur_Autoscale(Fl_Menu_* m , void*) {
    Flv_Table_Gen * spread_ptr=find_table_brother(m);
    if (!spread_ptr) return;
    Tableur_Group * tg=dynamic_cast<Tableur_Group *>(spread_ptr->parent());
    if (tg){
      tg->disposition |= 2;
      tg->resize2();
      tg->table->graph->autoscale();      
      tg->redraw();
    }
  }

   void cb_Tableur_Cfg_Window(Fl_Menu_* m , void*) {
    Flv_Table_Gen * spread_ptr=find_table_brother(m);
    if (!spread_ptr) return;
    if (spread_ptr)
      spread_ptr->config();
  }

   void cb_Tableur_Undo(Fl_Menu_* m , void*) {
    Flv_Table_Gen * spread_ptr=find_table_brother(m);
    if (spread_ptr)
      spread_ptr->restore(-1);
  }

   void cb_Tableur_Redo(Fl_Menu_* m , void*) {
    Flv_Table_Gen * spread_ptr=find_table_brother(m);
    if (spread_ptr)
      spread_ptr->restore(1);
  }

  Fl_Menu_Item Tableur_menu[] = {
    {gettext("Table"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("Save sheet as text"), 0,  (Fl_Callback*)cb_Tableur_Save, 0, 0, 0, 0, 14, 56},
    {gettext("Save as alternate filename"), 0,  (Fl_Callback*)cb_Tableur_Save_as, 0, 0, 0, 0, 14, 56},
    {gettext("Save as CSV"), 0,  (Fl_Callback*)cb_Tableur_Save_CSV, 0, 0, 0, 0, 14, 56},
    {gettext("Save as mathml"), 0,  (Fl_Callback*)cb_Tableur_Save_mathml, 0, 0, 0, 0, 14, 56},
    {gettext("Save selection to variable"), 0,  (Fl_Callback*)cb_Tableur_Save_var, 0, 0, 0, 0, 14, 56},
    {gettext("Insert"), 0,  (Fl_Callback*)cb_Tableur_Insert, 0, 0, 0, 0, 14, 56},
    {gettext("Insert CSV"), 0,  (Fl_Callback*)cb_Tableur_Insert_CSV, 0, 0, 0, 0, 14, 56},
    {gettext("Sheet configuration"), 0,  (Fl_Callback*)cb_Tableur_Variable_Name, 0, 0, 0, 0, 14, 56},
    {gettext("Print Export"), 0,  0, 0, 64, 0, 0, 14, 56},
    //    {gettext("latex preview"), 0,  (Fl_Callback*)cb_Tableur_LaTeX_Preview, 0, 0, 0, 0, 14, 56},
    //    {gettext("latex printer"), 0,  (Fl_Callback*)cb_Tableur_LaTeX_Print, 0, 0, 0, 0, 14, 56},
    {gettext("EPS PNG and preview"), 0,  (Fl_Callback*)cb_Tableur_Preview, 0, 0, 0, 0, 14, 56},
    {gettext("to printer"), 0,  (Fl_Callback*)cb_Tableur_Print, 0, 0, 0, 0, 14, 56},
    {0},
    {0},
    {gettext("Edit"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("Eval sheet"), 0xffc6,  (Fl_Callback *) cb_Tableur_Eval_Sheet, 0, 0, 0, 0, 14, 56},
    {gettext("Copy Cell"), 0,  (Fl_Callback *) cb_Tableur_Copy_Cell, 0, 0, 0, 0, 14, 56},
    {gettext("Paste"), 0,  (Fl_Callback *) cb_Paste, 0, 0, 0, 0, 14, 56},
    {gettext("Undo"), 0x4007a,  (Fl_Callback *) cb_Tableur_Undo, 0, 0, 0, 0, 14, 56},
    {gettext("Redo"), 0x40079,  (Fl_Callback *) cb_Tableur_Redo, 0, 0, 0, 0, 14, 56},
    {gettext("Configuration"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("Cfg window"), 0,  (Fl_Callback *) cb_Tableur_Cfg_Window, 0, 0, 0, 0, 14, 56},
    {gettext("Graph"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("Autoscale graph"), 0,  (Fl_Callback *) cb_Tableur_Autoscale, 0, 0, 0, 0, 14, 56},
    {gettext("Graph portrait"), 0,  (Fl_Callback *) cb_Tableur_Portrait, 0, 0, 0, 0, 14, 56},
    {gettext("Graph landscape"), 0,  (Fl_Callback *) cb_Tableur_Landscape, 0, 0, 0, 0, 14, 56},
    {gettext("Hide Graph"), 0,  (Fl_Callback *) cb_Tableur_Hide_Graph, 0, 0, 0, 0, 14, 56},
    {0}, // end graph
    {gettext("Format"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("Spreadsheet"), 0,  (Fl_Callback *) cb_Tableur_Spreadsheet_Mode, 0, 0, 0, 0, 14, 56},
    {gettext("Matrix"), 0,  (Fl_Callback *) cb_Tableur_Matrix_Mode, 0, 0, 0, 0, 14, 56},
    {gettext("Normal matrix"), 0,  (Fl_Callback *) cb_Tableur_Normal_Matrix, 0, 0, 0, 0, 14, 56},
    {gettext("Symmetric matrix"), 0,  (Fl_Callback *) cb_Tableur_Symmetric_Matrix, 0, 0, 0, 0, 14, 56},
    {gettext("AntiSymmetric matrix"), 0,  (Fl_Callback *) cb_Tableur_AntiSymmetric_Matrix, 0, 0, 0, 0, 14, 56},
    {gettext("Hermitian matrix"), 0,  (Fl_Callback *) cb_Tableur_Hermitian_Matrix, 0, 0, 0, 0, 14, 56},
    {gettext("AntiHermitian matrix"), 0,  (Fl_Callback *) cb_Tableur_AntiHermitian_Matrix, 0, 0, 0, 0, 14, 56},
    {0}, // end format
    {gettext("Set Rows Number"), 0,  (Fl_Callback *) cb_Tableur_SetRows, 0, 0, 0, 0, 14, 56},
    {gettext("Set Cols Number"), 0,  (Fl_Callback *) cb_Tableur_SetCols, 0, 0, 0, 0, 14, 56},
    {gettext("Move ->"), 0,  (Fl_Callback *) cb_Tableur_MoveRight, 0, 0, 0, 0, 14, 56},
    {gettext("Move down"), 0,  (Fl_Callback *) cb_Tableur_MoveDown, 0, 0, 0, 0, 14, 56},
    {gettext("Auto Recompute"), 0,  (Fl_Callback *) cb_Tableur_Auto_Recompute, 0, 0, 0, 0, 14, 56},
    {gettext("No Recompute"), 0,  (Fl_Callback *) cb_Tableur_No_Recompute, 0, 0, 0, 0, 14, 56},
    {gettext("Dispatch matrice to cells"), 0,  (Fl_Callback *) cb_Tableur_Matrix_Fill, 0, 0, 0, 0, 14, 56},
    {gettext("Keep matrice in one cell"), 0,  (Fl_Callback *) cb_Tableur_No_Matrix_Fill, 0, 0, 0, 0, 14, 56},
    {0}, // end Configuration
    {gettext("Fill"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("Copy right"), 0x40072,  (Fl_Callback *) cb_Tableur_Copy_Right, 0, 0, 0, 0, 14, 56},
    {gettext("Copy down"), 0x40064,  (Fl_Callback *) cb_Tableur_Copy_Down, 0, 0, 0, 0, 14, 56},
    {gettext("Fill selection with pushed cell"), 0,  (Fl_Callback *) cb_Tableur_FillSelection, 0, 0, 0, 0, 14, 56},
    {gettext("Fill selection with 0"), 0,  (Fl_Callback *) cb_Tableur_Fill0, 0, 0, 0, 0, 14, 56},
    {gettext("Fill sheet with 0"), 0,  (Fl_Callback *) cb_Tableur_Del, 0, 0, 0, 0, 14, 56},
    {0},
    {gettext("Add or delete"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("Insert Row"), 0,  (Fl_Callback *) cb_Tableur_InsRow, 0, 0, 0, 0, 14, 56},
    {gettext("Row+ at end"), 0,  (Fl_Callback *) cb_Tableur_InsRow_End, 0, 0, 0, 0, 14, 56},
    {gettext("Insert Col"), 0,  (Fl_Callback *) cb_Tableur_InsCol, 0, 0, 0, 0, 14, 56},
    {gettext("Col+ at end"), 0,  (Fl_Callback *) cb_Tableur_InsCol_End, 0, 0, 0, 0, 14, 56},
    {gettext("Erase current row"), 0,  (Fl_Callback *) cb_Tableur_DelRow, 0, 0, 0, 0, 14, 56},
    {gettext("Erase selection rows"), 0,  (Fl_Callback *) cb_Tableur_DelRows, 0, 0, 0, 0, 14, 56},
    {gettext("Erase current col"), 0,  (Fl_Callback *) cb_Tableur_DelCol, 0, 0, 0, 0, 14, 56},
    {gettext("Erase selection cols"), 0,  (Fl_Callback *) cb_Tableur_DelCols, 0, 0, 0, 0, 14, 56},
    {0},
    {gettext("Sort"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("Inc col"), 0,  (Fl_Callback *) cb_Tableur_Sort_Inc_Col, 0, 0, 0, 0, 14, 56},
    {gettext("Dec col"), 0,  (Fl_Callback *) cb_Tableur_Sort_Dec_Col, 0, 0, 0, 0, 14, 56},
    {gettext("Inc row"), 0,  (Fl_Callback *) cb_Tableur_Sort_Inc_Row, 0, 0, 0, 0, 14, 56},
    {gettext("Dec row"), 0,  (Fl_Callback *) cb_Tableur_Sort_Dec_Row, 0, 0, 0, 0, 14, 56},
    {0},
    {gettext("Col larger"), 0,  (Fl_Callback *) cb_Tableur_Col_Larger, 0, 0, 0, 0, 14, 56},
    {gettext("Col smaller"), 0,  (Fl_Callback *) cb_Tableur_Col_Smaller, 0, 0, 0, 0, 14, 56},
#ifndef IPAQ // on ipaq put Stat inside Edit
    {0}, // end Edit
#endif
    {gettext("Maths"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("Function"), 0,  (Fl_Callback *) cb_Tableur_Tablefunc, 0, 0, 0, 0, 14, 56},
    {gettext("Random values"), 0,  (Fl_Callback *) cb_Tableur_ranm, 0, 0, 0, 0, 14, 56},
    {gettext("Sequences"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("Recurrent sequence"), 0,  (Fl_Callback *) cb_Tableur_Tableseq, 0, 0, 0, 0, 14, 56},
    {gettext("Bar plot"), 0,  (Fl_Callback *) cb_Tableur_batons, 0, 0, 0, 0, 14, 56},
    {gettext("plotlist"), 0,  (Fl_Callback *) cb_Tableur_plotlist, 0, 0, 0, 0, 14, 56},
    {gettext("Polygonscatterplot"), 0,  (Fl_Callback *) cb_Tableur_Polygonscatterplot, 0, 0, 0, 0, 14, 56},
    {gettext("Scatterplot"), 0,  (Fl_Callback *) cb_Tableur_Scatterplot, 0, 0, 0, 0, 14, 56},
    {gettext("Polygonplot"), 0,  (Fl_Callback *) cb_Tableur_Polygonplot, 0, 0, 0, 0, 14, 56},
    {0},
    {gettext("1-d stats"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("camembert"), 0,  (Fl_Callback *) cb_Tableur_camembert, 0, 0, 0, 0, 14, 56},
    {gettext("Bar plot"), 0,  (Fl_Callback *) cb_Tableur_batons, 0, 0, 0, 0, 14, 56},
    {gettext("plotlist"), 0,  (Fl_Callback *) cb_Tableur_plotlist, 0, 0, 0, 0, 14, 56},
    {gettext("Boxwhisker"), 0,  (Fl_Callback *) cb_Tableur_Boxwhisker, 0, 0, 0, 0, 14, 56},
    {gettext("Classes (data or data,eff)"), 0,  (Fl_Callback *) cb_Tableur_Classes, 0, 0, 0, 0, 14, 56},
    {gettext("Histogram (interval,eff)"), 0,  (Fl_Callback *) cb_Tableur_Histogram, 0, 0, 0, 0, 14, 56},
    {0},
    {gettext("2-d stats"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("Polygonscatterplot"), 0,  (Fl_Callback *) cb_Tableur_Polygonscatterplot, 0, 0, 0, 0, 14, 56},
    {gettext("Scatterplot"), 0,  (Fl_Callback *) cb_Tableur_Scatterplot, 0, 0, 0, 0, 14, 56},
    {gettext("Polygonplot"), 0,  (Fl_Callback *) cb_Tableur_Polygonplot, 0, 0, 0, 0, 14, 56},
    {0}, // end 2-d
    {gettext("Regressions"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("Linear"), 0,  (Fl_Callback *) cb_Tableur_linear_regression_plot, 0, 0, 0, 0, 14, 56},
    {gettext("Polynomial"), 0,  (Fl_Callback *) cb_Tableur_polynomial_regression_plot, 0, 0, 0, 0, 14, 56},
    {gettext("Exponential"), 0,  (Fl_Callback *) cb_Tableur_exponential_regression_plot, 0, 0, 0, 0, 14, 56},
    {gettext("Logarithmic"), 0,  (Fl_Callback *) cb_Tableur_logarithmic_regression_plot, 0, 0, 0, 0, 14, 56},
    {gettext("Power"), 0,  (Fl_Callback *) cb_Tableur_power_regression_plot, 0, 0, 0, 0, 14, 56},
    {gettext("Logistic"), 0,  (Fl_Callback *) cb_Tableur_logistic_regression_plot, 0, 0, 0, 0, 14, 56},
    {0}, // end regressions
    {0}, // end Statistics
#ifdef IPAQ
    {0}, // end Edit
#endif
    {0} // end menu
  };

  void Tableur_callback(Flv_Table_Gen *l, void * ){
    l->header_event=l->why_event();
    // FLVE_TITLE_CLICKED, FLVE_ROW_HEADER_CLICKED, FLVE_COL_HEADER_CLICKED
    // FLVE_CLICKED
    if (l->header_event==FLVE_TITLE_CLICKED && l->last_event==FL_PUSH){
      if (l->computing)
	kill_thread(true,l->contextptr);
      else
	l->config();
      return;
    }
    /* 
       if (!l->editing && l->header_event==FLVE_CLICKED && l->clicks()>1 && l->row()==l->select_start_row() && l->select_start_col()==l->col()){
      // double-click, begin copy cell to area
      l->click_fill=true;
      set_cursor(l,FL_CURSOR_HAND);
    }
    */
    if (l->header_event==FLVE_ROW_HEADER_CLICKED && l->rows()){
      if (l->last_event!=FL_PUSH)
	l->row(l->row()-1);
      else {
	l->row(0);
	l->select_start_row(l->rows()-1);
      }
    }
    if (l->header_event==FLVE_ROW_FOOTER_CLICKED && l->rows()){
      if (l->last_event!=FL_PUSH)
	l->row(l->row()+1);
      else {
	l->row(l->rows()-1);
	l->select_start_row(0);
      }
    }
    if (l->header_event==FLVE_COL_HEADER_CLICKED && l->cols()){
      if (l->last_event!=FL_PUSH)
	l->col(l->col()-1);
      else {
	l->col(0);
	l->select_start_col(l->cols()-1);
      }
    }
    if (l->header_event==FLVE_COL_FOOTER_CLICKED && l->cols()){
      if (l->last_event!=FL_PUSH)
	l->col(l->col()+1);
      else {
	l->select_start_col(0);
	l->col(l->cols()-1);
      }
    }
  }

  void Flv_Table_Gen::finish_flv_table_gen(){
    if (Fl_Group * gr=parent()){
      labelsize(gr->labelsize());
      mb = new Fl_Menu_Button(x(),y(),w(),h(),"&Table");
      mb->type(Fl_Menu_Button::POPUP3);
      mb->box(FL_NO_BOX);
      mb->menu(Tableur_menu);
      gr->insert(*mb,gr->find(this));
    }
    else
      mb=0;
    // Style
    is_spreadsheet=true;
    header_event=0;
    // spread_editor = new Flve_Input( 0, 0, 0,0);
    // edit_when(FLV_EDIT_AUTOMATIC);
    // global_style.editor(spread_editor);
    // spread_editor->textsize(12);
    callback_when( FLVEcb_ROW_HEADER_CLICKED | FLVEcb_COL_HEADER_CLICKED | FLVEcb_CLICKED | FLVEcb_TITLE_CLICKED | FLVEcb_ROW_FOOTER_CLICKED | FLVEcb_COL_FOOTER_CLICKED | FLVEcb_CLICKED);
    callback((Fl_Callback*)Tableur_callback);
    select_locked(false);
    global_style.font_size(labelsize());
    global_style.x_margin(5);
    global_style.locked(false);
    
#ifdef IPAQ
    col_width(25,-1);
#else // IPAQ
    col_width(50,-1);
#endif // IPAQ      
    feature(FLVF_HEADERS|FLVF_ROW_FOOTER|FLVF_DIVIDERS|FLVF_MULTI_SELECT|FLVF_PERSIST_SELECT ); // add FLVF_COL_FOOTER for right footers
    global_style.resizable(true);
#ifdef IPAQ
    global_style.width(40);
#else // IPAQ
    global_style.width(10+10*labelsize());
#endif // IPAQ
    global_style.height(labelsize()+4);
    row_style[-1].align(FL_ALIGN_CLIP);
    col_style[-1].align(FL_ALIGN_CLIP);
    update_status();
    Fl::focus(this);
    // Fl::focus(input);
  }  

  void Flv_Table_Gen::draw_cell( int Offset, int &X, int &Y, int &W, int &H, int R, int C ){
    Flv_Style s;
    
    get_style(s, R, C);
    Flv_Table::draw_cell(Offset,X,Y,W,H,R,C);
    string ss;
    if (C<0){
      if (R>=0){
	if (xcas_mode(contextptr)>0)
	  ss=gen(R+1).print(contextptr);
	else
	  ss=gen(R).print(contextptr);
      }
    }
    else {
      if (R<0){
	if (is_spreadsheet && R==-1){
	  int i=C;
	  for(int j=0;;++j){
	    ss=char('A'+i%26-(j!=0))+ss;
	    i=i/26;
	    if (!i)
	      break;
	  }
	}
	else {
	  if (xcas_mode(contextptr)>0)
	    ss=gen(C+1).print(contextptr);
	  else
	    ss=gen(C).print(contextptr);
	}
      }
      else {
	const gen & g=m[R][C];
	if ((g.type==_VECT) && (g._VECTptr->size()==3) ){
	  int save_r=printcell_current_row(contextptr),save_c=printcell_current_col(contextptr);
	  printcell_current_row(contextptr)=R,printcell_current_col(contextptr)=C;
	  ss=pnt2string((*g._VECTptr)[1],contextptr);
	  printcell_current_row(contextptr)=save_r;printcell_current_col(contextptr)=save_c;
	}
	else
	  ss=g.print(contextptr);
      }
    }
    fl_color(FL_BLACK);
    if (ss.size()>max_printsize)
      ss=gettext("Too large for cell display");
    fl_draw(ss.c_str(), X-Offset, Y, W, H, s.align() );
  }

  /*
  void Flv_Table_Gen::save_editor( Fl_Widget *e, int R, int C ){
    if ( (R>=0) && (C>=0) ){
      vecteur & v=*m[R]._VECTptr;
      gen g(string( ((Flve_Input *) e)->value() ));
      if ( (v[C].type==_VECT) && (v[C]._VECTptr->size()==3)){
	v[C]._VECTptr->front()=spread_convert(g,R,C);
	if (is_spreadsheet) spread_eval_interrupt();
      }
      else
	v[C]=g;
    }
  }

  void Flv_Table_Gen::load_editor( Fl_Widget *e, int R, int C ){
    if ((C>=0) && (R>=0)){
      if ( (m[R][C].type==_VECT) && (m[R][C]._VECTptr->size()==3) ){
	int save_r=printcell_current_row,save_c=printcell_current_col;
	printcell_current_row=R,printcell_current_col=C;
	((Flve_Input *)e)->value( m[R][C][0].print(contextptr).c_str() );
	printcell_current_row=save_r;printcell_current_col=save_c;
      }
      else
	((Flve_Input *)e)->value( m[R][C].print(contextptr).c_str() );
      ((Flve_Input *)e)->position(((Flve_Input *)e)->size(), 0 );
    }
  }

  void Flv_Table_Gen::position_editor( Fl_Widget *e, int x, int y, int w, int h, Flv_Style &s ){

    //	Out of cell
    //	e->resize( 10, 10, 200, 20 );
    
    //	In cell
    //	Flv_Table::position_editor(e,x+s.x_margin(),y,w-s.x_margin(),h,s);
    Flv_Table::position_editor(e,x,y,w,h,s);
  }
  */

  void sheet2pnt(const matrice & m,vecteur & v){
    v.clear();
    gen tmp;
    const_iterateur it=m.begin(),itend=m.end();
    for (;it!=itend;++it){
      if (it->type==_VECT){
	const_iterateur jt=it->_VECTptr->begin(),jtend=it->_VECTptr->end();
	for (;jt!=jtend;++jt){
	  if (jt->type==_VECT && jt->_VECTptr->size()==3)
	    tmp=(*jt->_VECTptr)[1];
	  else
	    tmp=*jt;
	  bool test=tmp.is_symb_of_sommet(at_pnt);
	  if (tmp.type==_VECT && !tmp._VECTptr->empty())
	    test=tmp._VECTptr->back().is_symb_of_sommet(at_pnt);
	  if (test)
	    v.push_back(tmp);
	}
      }
    }
  }

  void Flv_Table_Gen::update_name(){
    if (name.type==_IDNT)
      sto(extractmatricefromsheet(m),name,contextptr);
  }
  
  void Flv_Table_Gen::update_spread_graph(){
    // Update graph
    if (graph){
      sheet2pnt(m,graph->plot_instructions);
      graph2d->plot_instructions=graph->plot_instructions;
      if (graph3d){
	graph3d->plot_instructions=graph->plot_instructions;
	graph3d->redraw();
      }
      graph->redraw();
      graph2d->redraw();
    }
  }

  void Flv_Table_Gen::spread_eval_interrupt(){
    if (computing || is_context_busy(contextptr)){
      fl_message("%s",gettext("CAS is busy"));
      return;
    }
    bool save_interrupt_button=interrupt_button;
    interrupt_button = true;
    if (is_spreadsheet){
      computing=true;
      label("Recomputing: click here to interrupt");
      redraw();
      evaled_table(contextptr)=this;
      thread_spread_eval(m,contextptr);
      update_spread_graph();
      computing=false;
    }
    update_name();
    update_status();
    interrupt_button=save_interrupt_button;
    redraw();
  }


  // Check that g is one of the following form
  // col1,col2...coln
  // cell1..row,col2,...,coln
  // return the corresponding submatrix of m in mselect
  int iscell_range(const giac::gen & g,const matrice & m,matrice & mselect,Flv_Table_Gen * sptr,int & r1,int & r2,std::vector<int> & vc,bool & absolu){
    vc.clear();
    const giac::context * contextptr = get_context(sptr);
    if (g.is_symb_of_sommet(at_interval)){
      gen & f=g._SYMBptr->feuille;
      if (f.type!=_VECT || f._VECTptr->size()!=2)
	return 0;
      int c1,c2;
      if (!iscell(f._VECTptr->front(),c1,r1,contextptr) || !iscell(f._VECTptr->back(),c2,r2,contextptr))
	return 0;
      if (r1>r2)
	giac::swapint(r1,r2);
      r2=giacmin(r2+1,m.size());
      r1=giacmax(r1,0);
      mselect=vecteur(m.begin()+r1,m.begin()+r2);
      mselect=mtran(mselect);
      if (c1>c2)
	giac::swapint(c1,c2);
      c2=giacmin(c2+1,m.size());
      c1=giacmax(c1,0);
      mselect=vecteur(mselect.begin()+c1,mselect.begin()+c2);
      mselect=mtran(mselect);
      if (sptr){
	sptr->select_start_row(r1);
	if (r2)
	  sptr->row(r2-1);
	sptr->select_start_col(c1);
	if (c2)
	  sptr->col(c2-1);
      }
      for (int j=c1;j<c2;++j)
	vc.push_back(j);
      return 1;
    }
    if (g.type!=_VECT)
      return 0;
    vecteur v=*g._VECTptr;
    int s=v.size();
    if (s<2)
      return 0;
    r1=0; r2=m.size();
    if (v.front().is_symb_of_sommet(at_interval) || v.front().is_symb_of_sommet(at_deuxpoints)){ // compute r1, r2
      gen & f=v.front()._SYMBptr->feuille;
      if (f.type!=_VECT || f._VECTptr->size()!=2)
	return 0;
      vecteur & w=*f._VECTptr;
      int c;
      if (!iscell(w[0],c,r1,contextptr))
	return 0;
      r1=giacmax(giacmin(r1,m.size()),0);
      if (w[1].type==_INT_)
	r2=w[1].val-(xcas_mode(contextptr)!=0);
      else {
	if (!iscell(w[1],c,r2,contextptr))
	  return 0;
      }
      r2=giacmax(giacmin(r2+1,m.size()),0);
      if (r2<r1) giac::swapint(r1,r2);
    }
    // full columns selection
    matrice mt(mtran(m));
    int mts=mt.size();
    matrice tselect;
    for (int i=0;i<s;++i){
      gen h=v[i];
      if (h.type==_INT_ && h.val>=0 && h.val<mts){
	tselect.push_back(mt[h.val]);
	vc.push_back(h.val);
	continue;
      }
      if (h.is_symb_of_sommet(at_interval)){
	gen & f=h._SYMBptr->feuille;
	if (f.type!=_VECT || f._VECTptr->size()!=2)
	  return 0;
	vecteur & w=*f._VECTptr;
	int ca,ra,cb,rb;
	if (!iscell(w[0],ca,ra,contextptr))
	  return 0;
	if (!iscell(w[1],cb,rb,contextptr))
	  return 0;
	if (ca>cb) giac::swapint(ca,cb);
	if (ra>rb) giac::swapint(ra,rb);
	if (ra!=r1 || rb+1!=r2)
	  return 0;
	for (int c=ca;c<=cb;++c){
	  tselect.push_back(mt[c]);
	  vc.push_back(c);
	}
      }
      if (h.type!=_IDNT)
	continue;
      absolu=true;
      const string & ss=h._IDNTptr->name();
      int sss=ss.size();
      if (sss<1)
	return 0;
      int c;
      alphaposcell(ss,c);
      if (c>=0 && c<mts){
	tselect.push_back(mt[c]);
	vc.push_back(c);
      }
    }
    mselect=mtran(tselect);
    mselect=vecteur(mselect.begin()+r1,mselect.begin()+r2);
    return vc.size();
  }

  bool iscell_range(const gen & g,const matrice & m,matrice & mselect,Flv_Table_Gen * sptr){
    int r1,r2;
    vector<int> vc;
    bool absolu;
    return iscell_range(g,m,mselect,sptr,r1,r2,vc,absolu);
  }

  int Tableur_Group::handle(int event){
    if (table && event==FL_UNFOCUS){
      if (Fl::event_x()<x() || Fl::event_x()>x()+w() || Fl::event_y()<y() || Fl::event_y()>y()+h()){
	// cerr << "unfocus " << endl;
	table->editing=false;
      }
    }
    return Fl_Tile::handle(event);
  }

  Tableur_Group::Tableur_Group(int X,int Y,int W,int H,int L,int disp):Fl_Tile(X,Y,W,H),disposition(disp),dtable(0),dgraph(0),dparam(0){
    labelsize(L);
    int l=L+6,inputh=L+18;
    borderbox = new Fl_Box(X,Y,w()-labelsize(),h()-labelsize());
    resizable(borderbox);
    end();
    box(FL_FLAT_BOX);
    matrice m(Flv_Table_Gen::def_rows,vecteur(Flv_Table_Gen::def_cols,0));
    Fl_Group::current(this);
    table=new Flv_Table_Gen(X,Y,W,H,m,"Spreadsheet");
    Fl_Group::current(this);
    if (!table->graph){
      table->graph = new Graph2d(X,Y+l,W,H-inputh,"");
      table->graph->ylegende=1.5;
    }
    if (!table->input){
      Fl_Text_Buffer * ptr=new Fl_Text_Buffer;
      table->input = new Xcas_Text_Editor(X,Y,W,inputh,ptr);
      table->input->scrollbar_width(12);
      table->input->buffer()->add_modify_callback(style_update, table->input);
      // table->input = new Multiline_Input_tab(X,Y,W,l);
      table->input->tableur=table;
      table->input->textsize(labelsize());
      table->input->callback(cb_Sheet_Input);
      table->input->tooltip(gettext("Spreadsheet commandline"));
      table->input->when(FL_WHEN_ENTER_KEY);
    }
    if (!table->_goto && table->input){
      table->_goto = new Multiline_Input_tab(X,Y,W,inputh);
      table->_goto->callback(cb_Spread_goto);
      table->_goto->textsize(labelsize());
      table->_goto->tooltip(gettext("Selection area"));
      table->_goto->when(FL_WHEN_ENTER_KEY|FL_WHEN_NOT_CHANGED);
    }
    // Attach input just after the sheet
    menubar = new Fl_Menu_Bar(X,Y,4*l,l);
    /* int s= Tableur_menu->size();
    Fl_Menu_Item * menuitem = new Fl_Menu_Item[s];
    for (int i=0;i<s;++i)
      *(menuitem+i)=*(Tableur_menu+i);
    menubar->menu (menuitem);
    */
    menubar->menu(Tableur_menu);
    int n=find(table)+1;
    insert(*table->graph,n);
    insert(*table->graph->mouse_param_group,n);
    insert(*table->input,n);
    insert(*table->_goto,n);
    insert(*menubar,n);
    // Add buttons here
    int bx=menubar->x()+menubar->w(),bs=w()-menubar->w(),by=Y,bw=3*l;
    if (bs<4*bw)
      bw=bs/4;
    reevalsave = new Fl_Group(bx,by,bs,H);
    Fl_Button * reeval=new Fl_Button(bx,by,bw,H,"eval");
    reeval->callback(cb_Tableur_Eval_Sheet);
    reeval->tooltip(gettext("Reeval current sheet"));
    bx += bw ; bs -= bw ;
    Fl_Button * seevalue=new Fl_Button(bx,by,bw,H,"val");
    seevalue->callback(cb_Tableur_Value);
    seevalue->tooltip(gettext("See value instead of formula"));
    bx += bw; bs -= bw;
    Fl_Button * reinit=new Fl_Button(bx,by,bw,H,"init");
    reinit->callback(cb_Tableur_Init);
    reinit->tooltip(gettext("Initialize the sheet using init formula"));
    bx += bw; bs -= bw;
    Fl_Button * viewgraph2d=new Fl_Button(bx,by,bw,H,"2-d");
    viewgraph2d->callback(cb_Tableur_Graph2d);
    viewgraph2d->tooltip(gettext("View attached graph 2-d"));
    bx += bw; bs -= bw;
    Fl_Button * viewgraph3d=new Fl_Button(bx,by,bw,H,"3-d");
    viewgraph3d->callback(cb_Tableur_Graph3d);
    viewgraph3d->tooltip(gettext("View attached graph 3-d"));
    bx += bw; bs -= bw;
    // Fill the remainder with an output (filename)
    fname =new Fl_Button(bx,by,bs,H);
    fname->label(gettext("<Save sheet as text>"));
    fname->callback(cb_Tableur_Save);
    fname->tooltip(gettext("Save spreadsheet independently of session"));
    fname->hide();
    reevalsave->end();
    insert(*reevalsave,n);
    change_group_fontsize(this,L); 
    Fl_Group::current(0);
    table->win2=new Fl_Window(X,Y,2*W/3,2*H/3);
    if (!table->graph2d){
      table->graph2d = new Graph2d(X,Y,2*W/3,2*H/3,"");
      table->graph2d->ylegende=1.5;
    }
    table->win2->end();
    table->win2->resizable(table->win2);
    table->win2->hide();
    table->win3=0;
    resize2();
    // handle(FL_FOCUS);
  }

  void Tableur_Group::save_dparam(){
    if (disposition & 0x1){
      dtable=table->w();
      dgraph=table->graph->w();
      dparam=table->graph->mouse_param_group->w();
    }
    else {
      dtable=double(table->h())/h();
      dgraph=((1-dtable)*table->graph->w())/table->w();
      dparam=max(1-dtable-dgraph,0.05);
    }
  }

  void Tableur_Group::resize2(double dt,double dg,double dp){
    if (dt>0 && dg>0 && dp>0){
      dtable=dt;
      dgraph=dg;
      dparam=dp;
    }
    if (dtable<=0 || dgraph<=0 || dparam<=0 ){
      dtable=0.56;
      dgraph=0.37;
      dparam=0.07;
    }
    double total=dtable+dgraph+dparam;
    dtable/=total;
    dgraph/=total;
    dparam/=total;
    int X=x(),Y=y(),W=w(),H=h();
    int L=labelsize();
    int l=L+6,inputh=L+18;
    menubar->resize(X,Y,W/3,l);
    reevalsave->resize(X+W/3,Y,2*W/3,l);
    if (disposition/2){
      if (disposition %2){ // table || graph 
	int itable=int(W*dtable+.5);
	int igraph=int(W*dgraph+.5);
	table->_goto->resize(X,Y+l,itable/3,inputh);
	table->input->resize(X+itable/3,Y+l,itable-itable/3,inputh);
	int itg=itable+igraph;
	table->resize(X,Y+l+inputh,itable,H-l-inputh);
	if (table->mb)
	  table->mb->resize(X,Y+l+inputh,itable,H-l-inputh);
	table->graph->resize(X+itable,Y+l,igraph,H-l);
	/*
	table->graph->mouse_param_group->resize(X+itg,Y+l,W-itg,H-l);
	int hh=(H-l);
	if (hh>8*labelsize())
	  hh=8*labelsize();
	table->graph->mouse_position->resize(X+itg,Y+l,W-itg,hh/4);
	table->graph->button_group->resize(X+itg,Y+l+hh/4,W-itg,hh/2);
	*/
	table->graph->resize_mouse_param_group(W-itg);
      }
      else { // graph below
	table->_goto->resize(X,Y+l,W/4,inputh);
	table->input->resize(X+W/4,Y+l,3*W/4,inputh);
	int itable=int(H*dtable+.5);
	int igraph=int(W*dgraph/(dgraph+dparam)+.5);
	table->resize(X,Y+l+inputh,W,itable);
	if (table->mb)
	  table->mb->resize(X,Y+l+inputh,W,itable);
	table->graph->resize(X,Y+l+inputh+itable,igraph,H-itable-l-inputh);
	/*
	int hh=H-itable-2*l;
	if (hh>8*labelsize())
	  hh=8*labelsize();
	int Y1=Y+2*l+itable;
	table->graph->mouse_param_group->resize(X+igraph,Y1,W-igraph,hh);
	int dh=hh/4;
	table->graph->mouse_position->resize(X+igraph,Y1,W-igraph,dh);
	table->graph->button_group->resize(X+igraph,Y1+dh,W-igraph,hh/2);
	dh += hh/2;
	*/
	table->graph->resize_mouse_param_group(W-igraph);
      }
      table->graph->show();
      table->graph->mouse_param_group->show();
    }
    else {
      table->resize(X,Y+l+inputh,W,H-l-inputh);
      if (table->mb)
	table->mb->resize(X,Y+l+inputh,W,H-(l+inputh));
      table->_goto->resize(X,Y+l,W/4,inputh);
      table->input->resize(X+W/4,Y+l,3*W/4,inputh);
      table->graph->resize(X,Y+H,5*W/6,0);
      table->graph->mouse_param_group->resize(X+5*W/6,Y+H,W/6,0);
      table->graph->hide();
      table->graph->mouse_param_group->hide();
    }
    redraw();
    init_sizes();
  }

  gen Xcas_fltk_Row(const gen & g,GIAC_CONTEXT){
    Flv_Table_Gen * tptr=(Flv_Table_Gen * )evaled_table(contextptr);
    if (tptr)
      return tptr->row();
    return -1;
  }  

  gen Xcas_fltk_current_sheet(const gen & g,GIAC_CONTEXT){
    Flv_Table_Gen * tptr=(Flv_Table_Gen * )evaled_table(contextptr);
    if (tptr){
      const giac::context * contextptr = get_context((tptr));
      int r,c;
      if (iscell(g,c,r,contextptr)){
	if (r>=tptr->rows()||c>=tptr->cols())
	  return undef;
	gen tmp=tptr->m[r];
	tmp=tmp[c];
	return tmp[1];
      }
      if (g.type==_VECT && g._VECTptr->size()==2 && g._VECTptr->front().type==_INT_ && g._VECTptr->back().type==_INT_)
	return gen(extractmatricefromsheet(tptr->m),_MATRIX__VECT)[g];
      if (iscell_range(g,tptr->m,tptr->selected,tptr) ){
	return extractmatricefromsheet(tptr->selected);
      }
      return  gen(extractmatricefromsheet(tptr->m),_MATRIX__VECT)[g];
    }
    return 0;
  }  

  gen Xcas_fltk_Col(const gen & g,GIAC_CONTEXT){
    Flv_Table_Gen * tptr=(Flv_Table_Gen * )evaled_table(contextptr);
    if (tptr)
      return tptr->col();
    return -1;
  }  

#ifndef NO_NAMESPACE_XCAS
} // namespace xcas
#endif // ndef NO_NAMESPACE_XCAS

#endif // HAVE_LIBFLTK
