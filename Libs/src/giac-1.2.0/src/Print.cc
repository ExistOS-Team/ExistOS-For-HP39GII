// -*- mode:C++ ; compile-command: "g++ -I.. -g -c Print.cc" -*-
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifndef IN_GIAC
#include <giac/first.h>
#else
#include "first.h"
#endif
#include <string>
#ifdef HAVE_LIBFLTK
#include "Print.h"
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Window.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Return_Button.H>
#include "History.h"
#include "Equation.h"
#include "Graph.h"
#include "Graph3d.h"
#include "Tableur.h"
#include <iostream>
#include <fstream>

using namespace std;
using namespace giac;

#ifdef FL_DEVICE 

//#include <FL/Fl_Printer.H>
#include <FL/Fl_PS_Printer.H>

#ifdef WIN32
  #include <FL/Fl_GDI_Printer.H>
#endif

#include <FL/fl_printer_chooser.H>

  #include "Fl_PS_Printer.cxx"

#ifdef WIN32
  #include "Fl_GDI_Printer.cxx"
#endif

#define Fl_PostScript_Graphics_Driver Fl_PS_Printer
#define Fl_PostScript_File_Device Fl_PS_Printer
#define Fl_PrintDevice Fl_Printer
#else // FL_DEVICE
#include <FL/Fl_PostScript.H>
#define Fl_PrintDevice Fl_Paged_Device
#endif // FL_DEVICE



#ifndef NO_NAMESPACE_XCAS
namespace xcas {
#endif // ndef NO_NAMESPACE_XCAS

#ifdef FL_DEVICE 
  double pixel_scale=0.21*FL_MM; // 1000 pixels=21 cm
#else
  double FL_MM=1.0;
  double FL_INCH=23.0;
  double pixel_scale=1; // should be fixed to work with scales
#endif

  int change_background_color(Fl_Widget * w,int c,int newc){
    int oldc=w->color();
    if (oldc==c)
      w->color(newc);
    if (Fl_Group * g=dynamic_cast<Fl_Group *>(w)){
      int n=g->children();
      for (int i=0;i<n;++i)
	change_background_color(g->child(i),c,newc);
    }
    return oldc;
  }

#ifdef Fl_Printer_H 
#ifdef FL_DEVICE
  int printer_format=Fl_Printer::A4;
#else
  int printer_format=Fl_Paged_Device::A4;
#endif
#else // Fl_Printer_H
  int printer_format=-1;
#endif // Fl_Printer_H
  bool printer_landscape=false;

#ifdef FL_DEVICE

  unsigned ck_page_height(Fl_Printer * p){
    int h=p->page_height();
    if (h<=0){
      cerr << "Bad page height " << h << endl;
      h=806;
    }
  }

  unsigned ck_page_width(Fl_Printer * p){
    int w=p->page_width();
    if (w<=0){
      cerr << "Bad page width " << w << endl;
      w=559;
    }
  }

  void print_newpage(Fl_Printer * p){
    if (printer_landscape)
      p->page(printer_format | Fl_Printer::LANDSCAPE);
    else
      p->page(printer_format);
  }

  void Flv_Table_Gen_widget_print(Flv_Table_Gen * g,Fl_Printer * p,bool eps){
    print_newpage(p);
    double margin=FL_INCH;
    int hp=int((ck_page_height(p)-2*margin)/pixel_scale);
    int wp=int((ck_page_width(p)-2*margin)/pixel_scale);
    g->resize(0,0,wp,hp);
    int i=0,j=0; // current row/column
    for (;j<g->cols()-1;){
      if (i || j)
	print_newpage(p);
      g->row(g->rows());
      g->col(g->cols());
      g->select_start_row(g->rows());
      g->select_start_col(g->cols());
      g->redraw();
      g->row(i);
      g->col(j);
      g->select_start_row(i);
      g->select_start_col(j);
      g->redraw();
      p->place(0.0, 0.0, margin,margin, pixel_scale);
      fl_draw(g);
      i = g->get_row(100,hp-20);
      i = max(i,g->get_row(100,hp-25));
      i = max(i,g->get_row(95,hp-20));
      i = max(i,g->get_row(95,hp-25));
      if (i<0)
	i=g->rows();
      if (i>=g->rows()-3){
	i=0;
	g->row(i);
	j=g->get_col(wp-30,hp-20);
	j=max(j,g->get_col(wp-30,hp-25));
	j=max(j,g->get_col(wp-35,hp-20));
	j=max(j,g->get_col(wp-35,hp-25));
	if (j<0)
	  j=g->cols();
      }
    }    
  }

  void Equation_widget_print(Equation * g,Fl_Printer * p,bool eps){
    print_newpage(p);
    int x=g->x(),y=g->y();//,h=g->h(),w=g->w();
    eqwdata e=Equation_total_size(g->data);
    double margin=FL_INCH;
    int hp=int((ck_page_height(p)-2*margin)/pixel_scale);
    int wp=int((ck_page_width(p)-2*margin)/pixel_scale);
    g->resize(x,y,wp,hp);
    int np=e.dy/hp;
    for (int i=0;i<=np;++i){
      if (i)
	print_newpage(p);
      //if (i)
      //p->page(Fl_Printer::A4);
      // fprintf(f,"%sPage: %d %d\n","%%",i+1,i+1);
      g->xleft=e.x;
      g->ytop=e.y+e.dy-i*hp;
      p->place(g->xleft, 0, margin,margin, pixel_scale);
      fl_draw(g);
    }
  }

  void Graph2d3d_widget_print(Graph2d3d * i,Fl_Printer * p,bool eps){
    Graph3d * g3=dynamic_cast<Graph3d *>(i);
    if (g3){
      if (Fl_PS_Printer * ps=dynamic_cast<Fl_PS_Printer * >(p))
	g3->printing=ps->file();
    }
    int x=i->x(),y=i->y(),w=i->w(),h=i->h();
    if (eps){
      i->resize(0,0,w,h);
      p->place(0.0,0.0,0.0,0.0,pixel_scale);
    }
    else {
      print_newpage(p);
      double margin=FL_INCH;
      int hp=int((ck_page_height(p)-2*margin)/pixel_scale);
      int wp=int((ck_page_width(p)-2*margin)/pixel_scale);
      if (wp*h<hp*w)
	i->resize(0,0,wp,(h*wp)/w);
      else
	i->resize(0,0,(hp*w)/h,hp);
      p->place(0.0, 0.0, margin,margin, pixel_scale);
    }
    fl_draw(i);
    i->resize(x,y,w,h);
    if (g3)
      g3->printing=0;
  }

  void generic_widget_print(Fl_Widget * i,Fl_Printer * p,bool eps){
    int x=i->x(),y=i->y(),w=i->w(),h=i->h();
    if (eps){
      i->resize(0,0,w,h);
      p->place(0.0,0.0,0.0,0.0,pixel_scale);
    }
    else {
      print_newpage(p);
      double margin=FL_INCH;
      int hp=int((ck_page_height(p)-2*margin)/pixel_scale);
      int wp=int((ck_page_width(p)-2*margin)/pixel_scale);
      if (wp*h<hp*w)
	i->resize(0,0,wp,(h*wp)/w);
      else
	i->resize(0,0,(hp*w)/h,hp);
      p->place(0.0, 0.0, margin,margin, pixel_scale);
    }
    fl_draw(i);
    i->resize(x,y,w,h);
  }

  void Fl_Text_Editor_widget_print(Fl_Text_Editor * g,Fl_Printer * p,bool eps){
    print_newpage(p);
    Fl_Text_Buffer * b=g->buffer();
    double margin=FL_INCH;
    int hp=int((ck_page_height(p)-2*margin)/pixel_scale);
    int wp=int((ck_page_width(p)-2*margin)/pixel_scale);
    int endpos=string(b->text()).size();
    int nlines=b->count_lines(0,endpos)+1;
    int t=g->textsize();
    int npoints=nlines*t,np=npoints/hp;
    if (npoints%hp)
      np += 1;
    // np=numbre of pages, adjust hp
    int ligneparpage=nlines/np;
    if (nlines%np){
      ligneparpage +=1 ;
      /*
      int j=np*ligneparpage-nlines;
      // insert blank lines at end
      string ins;
      for (;j;--j)
	ins += " \n";
      b->insert(endpos-1,ins.c_str());
      */
    }
    hp=ligneparpage*t+g->scrollbar_width()+10;
    g->resize(0,0,wp,hp);
    for (int i=0;i<np;++i){
      if (i)
	print_newpage(p);
      if (i==np-1)
	g->resize(0,0,wp,(nlines-i*ligneparpage)*t+g->scrollbar_width()+10);
      g->scroll(i*ligneparpage+1,0);
      p->place(0.0, 0.0, margin,margin, pixel_scale);
      fl_draw(g);
    }
  }

  void setgraph3dprintstate(Fl_Widget * widget,FILE * stream,bool hidemouseparam){
    Fl_Group * g=dynamic_cast<Fl_Group *>(widget);
    if (!g)
      return;
    int n=g->children();
    for (int i=0;i<n;++i){
      Fl_Widget * wid = g->child(i);
      if (Fl_Group * gr=dynamic_cast<Fl_Group * >(wid))
	setgraph3dprintstate(gr,stream,hidemouseparam);
      if (Graph2d3d * gr=dynamic_cast<Graph2d3d * >(wid)){
	if (gr->mouse_param_group){
	  Fl_Group * G=gr->parent();
	  if (stream){
	    if (hidemouseparam)
	      gr->mouse_param_group->hide();
	  }
	  else {	    
	    gr->mouse_param_group->show();
	    if (G){ // restore correct sizes
	      gr->resize(gr->x(),gr->y(),G->w()+G->x()-gr->x()-gr->legende_size,gr->h());
	      gr->resize_mouse_param_group(gr->legende_size);
	    }
	  }
	}
	if (Graph3d * gr3=dynamic_cast<Graph3d * >(wid))
	  gr3->printing=stream;
      }
    }
  }

  // scan s children for last y() <= pos
  int scan(Fl_Group * s,int pos,int hp){
    if (s->y()+s->h()<=pos)
      return s->y()+s->h();
    int n=s->children(),i;
    int curpos=0;
    for (i=0;i<n;++i){
      int y=s->child(i)->y();
      if (y>pos)
	break;
      else
	curpos=y;
    }
    if (curpos<pos-hp/3)
      return pos;
    else
      return curpos;
  }

  void History_Pack_widget_print(History_Pack * g,Fl_Printer *p,bool eps){
    // Save parent widget and position
    Fl_Group * gp=g->parent();
    if (!gp)
      return;
    int pos=gp->find(g);
    print_newpage(p);
    int x=g->x(),y=g->y(),h=g->h(),w=g->w();
    double margin=FL_INCH;
    int hp=int((ck_page_height(p)-2*margin)/pixel_scale);
    int wp=int((ck_page_width(p)-2*margin)/pixel_scale);
    // Create a scroll group of size hp/wp and put g in it
    Fl_Group::current(0);
    Fl_Window * win = new Fl_Window(0,0,wp,hp);
    Fl_Scroll * s = new Fl_Scroll(0,0,wp,hp);
    s->end();
    win->end();
    s->color(g->color());
    s->box(FL_FLAT_BOX);
    s->add(g);
    g->Fl_Group::resize(0,0,wp-g->labelsize(),h);
    g->resize();
    int ypos=0,newpos=0;
    for (;ypos<h;){
      if (ypos)
	print_newpage(p);
#ifdef _HAVE_FL_UTF8_HDR_
      s->scroll_to(0,0);
#else
      s->position(0,0);
#endif
      newpos=scan(g,ypos+hp,hp);
      s->resize(0,0,wp,newpos-ypos);
#ifdef _HAVE_FL_UTF8_HDR_
      s->scroll_to(0,ypos);
#else
      s->position(0,ypos);
#endif
      p->place(0.0,0, margin,margin, pixel_scale);
      fl_draw(s);
      ypos = newpos;
    }
    // Restore g in parent widget
    g->Fl_Group::resize(x,y,w,h);
    g->resize();
    gp->insert(*g,pos);
    delete s;
  }

  void inner_widget_print(Fl_Widget * widget,Fl_Printer * p,bool eps){
    if (History_Pack * f=dynamic_cast<History_Pack *>(widget)){
      History_Pack_widget_print(f,p,eps);
      return ;
    }
    if (Fl_Text_Editor * b=dynamic_cast<Fl_Text_Editor *>(widget)){
      Fl_Text_Editor_widget_print(b,p,eps);
      return;
    }
    if (Equation * g = dynamic_cast<Equation *> (widget)){
      Equation_widget_print(g,p,eps);
      return;
    }
    if (Graph2d3d * i=dynamic_cast<Graph2d3d *>(widget)){
      Graph2d3d_widget_print(i,p,eps);
      return;
    }
    if (Flv_Table_Gen * t=dynamic_cast<Flv_Table_Gen *>(widget)){
      Flv_Table_Gen_widget_print(t,p,eps);
      return;
    }
    generic_widget_print(widget,p,eps);
    /*
    if (Fl_Group * gr=dynamic_cast<Fl_Group *>(widget)){ 
      // print all elements of the group
      int n=gr->children();
      for (int i=0;i<n;++i)
	inner_widget_print(gr->child(i),p,eps);
      return;
    }
    */
  }

  void in_widget_print(Fl_Widget * widget,Fl_Printer * p,bool eps){
    if (!widget) return;
    bool screen_capture=false;
    if (dynamic_cast<Fl_Window *>(widget))
      screen_capture=true;
    if (!screen_capture)
      change_background_color(widget,Xcas_background_color,7);
    Fl_Output_Device * c = p->set_current();
    int x=widget->x(),y=widget->y(),h=widget->h(),w=widget->w();
    inner_widget_print(widget,p,eps);
    widget->resize(x,y,w,h);
    if (!screen_capture){
      change_background_color(widget,7,Xcas_background_color);
      set_colors(widget);
    }
    c->set_current();
  }

#else // FL_DEVICE

#ifdef Fl_Printer_H

  void print_newpage(Fl_PrintDevice * p){
    p->start_page();
  }
  void endpage(Fl_PrintDevice *p){ 
    p->end_page();
  }

  void compute_w_h(Fl_PrintDevice *p,double margin,double pixel_scale,int & wp,int &hp){
    p->printable_rect(&wp,&hp);
    if (wp<=1){
      cerr << "Bad width " << wp << endl;
      wp=559;
    }
    if (hp<=1){
      cerr << "Bad height " << hp << endl;
      hp=806;
    }
  }
  void do_print(Fl_Widget * w,Fl_PrintDevice *p,double margin,double pixel_scale){
    p->print_widget(w,0,0);
  }

  void Flv_Table_Gen_widget_print(Flv_Table_Gen * g,Fl_PrintDevice * p,bool eps){
    print_newpage(p);
    double margin=FL_INCH;
    int hp,wp;
    compute_w_h(p,margin,pixel_scale,wp,hp);
    g->resize(0,0,wp,hp);
    int i=0,j=0; // current row/column
    for (;j<g->cols()-1;){
      if (i || j)
	print_newpage(p);
      g->row(g->rows());
      g->col(g->cols());
      g->select_start_row(g->rows());
      g->select_start_col(g->cols());
      g->redraw();
      g->row(i);
      g->col(j);
      g->select_start_row(i);
      g->select_start_col(j);
      g->redraw();
      do_print(g,p,margin,pixel_scale);
      endpage(p);
      i = g->get_row(100,hp-20);
      i = max(i,g->get_row(100,hp-25));
      i = max(i,g->get_row(95,hp-20));
      i = max(i,g->get_row(95,hp-25));
      if (i<0)
	i=g->rows();
      if (i>=g->rows()-3){
	i=0;
	g->row(i);
	j=g->get_col(wp-30,hp-20);
	j=max(j,g->get_col(wp-30,hp-25));
	j=max(j,g->get_col(wp-35,hp-20));
	j=max(j,g->get_col(wp-35,hp-25));
	if (j<0)
	  j=g->cols();
      }
    }    
  }

  void Equation_widget_print(Equation * g,Fl_PrintDevice * p,bool eps){
    print_newpage(p);
    int x=g->x(),y=g->y();//,h=g->h(),w=g->w();
    eqwdata e=Equation_total_size(g->data);
    double margin=FL_INCH;
    int hp,wp;
    compute_w_h(p,margin,pixel_scale,wp,hp);
    g->resize(x,y,wp,hp);
    int np=e.dy/hp;
    for (int i=0;i<=np;++i){
      if (i)
	print_newpage(p);
      //if (i)
      //p->page(Fl_PrintDevice::A4);
      // fprintf(f,"%sPage: %d %d\n","%%",i+1,i+1);
      g->xleft=e.x;
      g->ytop=e.y+e.dy-i*hp;
      p->print_widget(g,g->xleft,0);
      endpage(p);
    }
  }

  class ps_file_device:public Fl_PostScript_File_Device {
  public:
    Fl_PostScript_Graphics_Driver * driver() { return Fl_PostScript_File_Device::driver(); };
    ps_file_device():Fl_PostScript_File_Device(){};
  };

  void Graph2d3d_widget_print(Graph2d3d * i,Fl_PrintDevice * p,bool eps){
    Graph3d * g3=dynamic_cast<Graph3d *>(i);
    if (g3){
      if (ps_file_device * ps=dynamic_cast<ps_file_device * >(p)){
	g3->printing=ps->driver()->file();
      }
    }
    int x=i->x(),y=i->y(),w=i->w(),h=i->h();
    if (eps){
      i->resize(0,0,w,h);
      print_newpage(p);
      do_print(i,p,0,pixel_scale);
    }
    else {
      print_newpage(p);
      double margin=FL_INCH;
      int hp,wp;
      compute_w_h(p,margin,pixel_scale,wp,hp);
      if (wp*h<hp*w)
	i->resize(0,0,wp,(h*wp)/w);
      else
	i->resize(0,0,(hp*w)/h,hp);
      do_print(i,p,margin,pixel_scale);
      endpage(p);
    }
    i->resize(x,y,w,h);
    if (g3)
      g3->printing=0;
  }

  void generic_widget_print(Fl_Widget * i,Fl_PrintDevice * p,bool eps){
    int x=i->x(),y=i->y(),w=i->w(),h=i->h();
    if (eps){
      print_newpage(p);
      i->resize(0,0,w,h);
      do_print(i,p,0,pixel_scale);
      // endpage(p);
    }
    else {
      print_newpage(p);
      double margin=FL_INCH;
      int hp,wp;
      compute_w_h(p,margin,pixel_scale,wp,hp);
      if (wp*h<hp*w)
	i->resize(0,0,wp,(h*wp)/w);
      else
	i->resize(0,0,(hp*w)/h,hp);
      do_print(i,p,margin,pixel_scale);
      endpage(p);
    }
    i->resize(x,y,w,h);
  }

  void Fl_Text_Editor_widget_print(Fl_Text_Editor * g,Fl_PrintDevice * p,bool eps){
    print_newpage(p);
    Fl_Text_Buffer * b=g->buffer();
    double margin=FL_INCH;
    int hp,wp;
    compute_w_h(p,margin,pixel_scale,wp,hp);
    int endpos=string(b->text()).size();
    int nlines=b->count_lines(0,endpos)+1;
    int t=g->textsize();
    int npoints=nlines*t,np=npoints/hp;
    if (npoints%hp)
      np += 1;
    // np=numbre of pages, adjust hp
    int ligneparpage=nlines/np;
    if (nlines%np){
      ligneparpage +=1 ;
      /*
      int j=np*ligneparpage-nlines;
      // insert blank lines at end
      string ins;
      for (;j;--j)
	ins += " \n";
      b->insert(endpos-1,ins.c_str());
      */
    }
    hp=ligneparpage*t+g->scrollbar_width()+10;
    g->resize(0,0,wp,hp);
    for (int i=0;i<np;++i){
      if (i)
	print_newpage(p);
      if (i==np-1)
	g->resize(0,0,wp,(nlines-i*ligneparpage)*t+g->scrollbar_width()+10);
      g->scroll(i*ligneparpage+1,0);
      do_print(g,p,margin,pixel_scale);
      endpage(p);
    }
  }

  // also show or hide mouse param group
  void setgraph3dprintstate(Fl_Widget * widget,FILE * stream,bool hidemouseparam){
    Fl_Group * g=dynamic_cast<Fl_Group *>(widget);
    if (!g)
      return;
    int n=g->children();
    for (int i=0;i<n;++i){
      Fl_Widget * wid = g->child(i);
      if (Fl_Group * gr=dynamic_cast<Fl_Group * >(wid))
	setgraph3dprintstate(gr,stream,hidemouseparam);
      if (Graph2d3d * gr=dynamic_cast<Graph2d3d * >(wid)){
	if (gr->mouse_param_group){
	  Fl_Group * G=gr->parent();
	  if (stream){
	    if (hidemouseparam)
	      gr->mouse_param_group->hide();
	  }
	  else {	    
	    gr->mouse_param_group->show();
	    if (G){ // restore correct sizes
	      gr->resize(gr->x(),gr->y(),G->w()+G->x()-gr->x()-gr->legende_size,gr->h());
	      gr->resize_mouse_param_group(gr->legende_size);
	    }
	  }
	}
	if (Graph3d * gr3=dynamic_cast<Graph3d * >(wid))
	  gr3->printing=stream;
      }
    }
  }

  // scan s children for last y() <= pos
  int scan(Fl_Group * s,int pos,int hp){
    if (s->y()+s->h()<=pos)
      return s->y()+s->h();
    int n=s->children(),i;
    int curpos=0;
    for (i=0;i<n;++i){
      int y=s->child(i)->y();
      if (y>pos)
	break;
      else
	curpos=y;
    }
    if (curpos<pos-hp/3)
      return pos;
    else
      return curpos;
  }

  void History_Pack_widget_print(History_Pack * g,Fl_PrintDevice *p,bool eps){
    // Save parent widget and position
    Fl_Group * gp=g->parent();
    if (!gp)
      return;
    int pos=gp->find(g);
    print_newpage(p);
    int x=g->x(),y=g->y(),h=g->h(),w=g->w();
    double margin=FL_INCH;
    int hp,wp;
    compute_w_h(p,margin,pixel_scale,wp,hp);
    // Create a scroll group of size hp/wp and put g in it
    Fl_Group::current(0);
    Fl_Window * win = new Fl_Window(0,0,wp,hp);
    Fl_Scroll * s = new Fl_Scroll(0,0,wp,hp);
    s->end();
    win->end();
    s->color(g->color());
    s->box(FL_FLAT_BOX);
    s->add(g);
    g->Fl_Group::resize(0,0,wp-g->labelsize(),h);
    g->resize();
    int ypos=0,newpos=0;
    for (;ypos<h;){
      if (ypos)
	print_newpage(p);
#ifdef _HAVE_FL_UTF8_HDR_
      s->scroll_to(0,0);
#else
      s->position(0,0);
#endif
      newpos=scan(g,ypos+hp,hp);
      s->resize(0,0,wp,newpos-ypos);
#ifdef _HAVE_FL_UTF8_HDR_
      s->scroll_to(0,ypos);
#else
      s->position(0,ypos);
#endif
      do_print(s,p,margin,pixel_scale);
      endpage(p);
      ypos = newpos;
    }
    // Restore g in parent widget
    g->Fl_Group::resize(x,y,w,h);
    g->resize();
    gp->insert(*g,pos);
    delete s;
  }

  void inner_widget_print(Fl_Widget * widget,Fl_PrintDevice * p,bool eps){
    if (History_Pack * f=dynamic_cast<History_Pack *>(widget)){
      History_Pack_widget_print(f,p,eps);
      return ;
    }
    if (Fl_Text_Editor * b=dynamic_cast<Fl_Text_Editor *>(widget)){
      Fl_Text_Editor_widget_print(b,p,eps);
      return;
    }
    if (Equation * g = dynamic_cast<Equation *> (widget)){
      Equation_widget_print(g,p,eps);
      return;
    }
    if (Graph2d3d * i=dynamic_cast<Graph2d3d *>(widget)){
      Graph2d3d_widget_print(i,p,eps);
      return;
    }
    if (Flv_Table_Gen * t=dynamic_cast<Flv_Table_Gen *>(widget)){
      Flv_Table_Gen_widget_print(t,p,eps);
      return;
    }
    generic_widget_print(widget,p,eps);
    /*
    if (Fl_Group * gr=dynamic_cast<Fl_Group *>(widget)){ 
      // print all elements of the group
      int n=gr->children();
      for (int i=0;i<n;++i)
	inner_widget_print(gr->child(i),p,eps);
      return;
    }
    */
  }

  void in_widget_print(Fl_Widget * widget,Fl_PrintDevice * p,bool eps){
    if (!widget) return;
    bool screen_capture=false;
    if (dynamic_cast<Fl_Window *>(widget))
      screen_capture=true;
    if (!screen_capture)
      change_background_color(widget,Xcas_background_color,7);
    int x=widget->x(),y=widget->y(),h=widget->h(),w=widget->w();
    inner_widget_print(widget,p,eps);
    widget->resize(x,y,w,h);
    if (!screen_capture){
      change_background_color(widget,7,Xcas_background_color);
      set_colors(widget);
    }
  }
#endif // Fl_Printer_H

#endif // FL_DEVICE

#ifdef WIN32
  string ps_preview="!";
#else
#ifdef __APPLE__
  string ps_preview="open"; // "/Applications/Preview.app/Contents/MacOS/Preview";
#else
  string ps_preview="gv";
#endif
#endif

  bool new_widget_size(int & neww,int & newh){
    static Fl_Window * w = 0;
    static Fl_Value_Input * fw=0, *fh=0,* pxl=0; 
    static Fl_Return_Button * button0 = 0 ;
    static Fl_Button * button1 =0;
    if (!w){
      int dx=300, dy=100;
      Fl_Group::current(0);
      w=new Fl_Window(dx,dy);
      fw=new Fl_Value_Input(dx/4,2,dx/4-2,dy/3-4,gettext("Width"));
      fw->tooltip(gettext("Width in pixels for export"));
      fh=new Fl_Value_Input(3*dx/4,2,dx/4-2,dy/3-4,gettext("Height"));
      fh->tooltip(gettext("Height in pixels for export"));
      pxl=new Fl_Value_Input(3*dx/4,2+dy/3,dx/4-2,dy/3-4,gettext("Pixel size"));
      pxl->tooltip(gettext("Pixel size in mm"));
      button0 = new Fl_Return_Button(2,2+2*dy/3,dx/2-4,dy/3-4);
      button0->shortcut(0xff0d);
      button0->label(gettext("OK"));
      button1 = new Fl_Button(dx/2+2,2+2*dy/3,dx/2-4,dy/3-4);
      button1->shortcut(0xff1b);
      button1->label(gettext("Cancel"));
      w->end();
      w->resizable(w);
      w->label(gettext("Size in pixels for export"));
    }
    fw->value(neww);
    fh->value(newh);
    pxl->value(pixel_scale/FL_MM);
    int r=-1;
    w->set_modal();
    w->show();
    w->hotspot(w);
    Fl::focus(w);
    for (;;) {
      Fl_Widget *o = Fl::readqueue();
      if (!o) Fl::wait();
      else {
	if (o == button0) {r = 0; break;}
	if (o == button1) {r = 1; break;}
	if (o == w) { r=1; break; }
      }
    }
    w->hide();
    if (r)
      return false;
    neww=int(std::max(1.0,fw->value()));
    newh=int(std::max(1.0,fh->value()));
    pixel_scale=pxl->value()*FL_MM;
    return true;
  }

  void widget_ps_print(Fl_Widget * widget,const std::string & fname0,bool eps,int pngpdf,bool preview,bool hidemouseparam){
    if (!eps)
      pngpdf=0;
#ifdef Fl_Printer_H
    // Fl_PrintDevice * p = fl_printer_chooser();
    int x=widget->x(),y=widget->y(),w=widget->w(),h=widget->h(),neww(w),newh(h);
    if (eps){
      if (!new_widget_size(neww,newh))
	return;
      widget->resize(x,y,neww,newh);
    }
    string fname1=replace(fname0,' ','_');
    if (fname1.empty())
      fname1="session";
    if (fname1[0]=='<')
      fname1="session";
    string fname(remove_extension(fname1)+(eps?".eps":".ps"));
    if (!eps || pngpdf){
      char * filename = file_chooser(eps?"Print to EPS/PNG/PDF file":"Print to PS file",eps?"*.eps":"*.ps",fname.c_str());
      if(!filename) return;
      fname=remove_extension(filename)+(eps?".eps":".ps");
    }
    FILE * f = fopen(fname.c_str(),"w");
    if(!f) return;
    Fl_PrintDevice * p;
#ifdef FL_DEVICE
    if (eps)
      p= new Fl_PS_Printer(f, 3,0,0,int(widget->w()*pixel_scale),int(widget->h()*pixel_scale));
    else
      p=new Fl_PS_Printer(f, 3);
#else
    ps_file_device * ptr=new ps_file_device();
    if (!ptr) return;
    ptr->driver()->pw_=eps?int(widget->w()*pixel_scale):0;
    ptr->driver()->ph_=eps?int(widget->h()*pixel_scale):0;
    ptr->start_job(f,eps?0:3);
    p=ptr;
#endif
    if (p){
      // Scan all graph3d widget inside and move their state to printing=p stream
#ifdef FL_DEVICE
      if (Fl_PostScript_File_Device * ps=dynamic_cast<Fl_PostScript_File_Device * >(p)){
	setgraph3dprintstate(widget,ps->file(),hidemouseparam);
      }
#else
      if (ps_file_device * ps=dynamic_cast<ps_file_device * >(p)){
	setgraph3dprintstate(widget,ps->driver()->file(),hidemouseparam);
      }
#endif
      in_widget_print(widget,p,eps);
#ifndef FL_DEVICE
      p->end_job();
#endif
      // Scan all graph3d widget inside and move their state to printing=true
      setgraph3dprintstate(widget,0,false);
#ifdef FL_DEVICE
      delete p;
#else
      delete ptr;
#endif
    }
    fclose(f); //we need close file here
    if (getenv("GIAC_PREVIEW"))
      ps_preview=getenv("GIAC_PREVIEW");
#ifdef WIN32
    if (ps_preview=="!"){
      ps_preview="/cygdrive/c/";
      if (getenv("XCAS_ROOT")){
	string s(getenv("XCAS_ROOT"));
	if (s.substr(0,10)=="/cygdrive/")
	  ps_preview[10]=s[10];
      }
      ps_preview +="Program\\ Files/Ghostgum/gsview/gsview32.exe";
    }
    pngpdf |= 0x4;
#endif
    // translate into png, (inside GIMP Image->Transform->rotate, not anymore)
    // inside latex use includegraphics[width=\textwidth{filename}
    // FIXME add pstopnm pnmtopng libnetpbm to the window mac distributions
    if (eps){
      widget->resize(x,y,w,h);
#ifndef WIN32 // def __APPLE__
      if (pngpdf & 0x1)
	system(("convert "+fname+" "+remove_extension(fname)+".png &").c_str()); 
      if (pngpdf & 0x2)
	system(("convert "+fname+" "+remove_extension(fname)+".pdf &").c_str()); 
      if (pngpdf & 0x4)
	system(("convert "+fname+" "+remove_extension(fname)+".jpg &").c_str()); 
#else
#ifdef WIN32
      if (pngpdf & 0x4)
	system((xcasroot()+"pstopnm -stdout -portrait '"+fname+"' | "+xcasroot()+"pnmtojpeg > '"+remove_extension(fname)+".jpg'").c_str()); 
      if (pngpdf & 0x1)
	system((xcasroot()+"pstopnm -stdout -portrait '"+fname+"' | "+xcasroot()+"pnmtopng > '"+remove_extension(fname)+".png' &").c_str()); 
      if (pngpdf & 0x2){
	// epstopdf is a perl script, it won't work wo perl
	system(("cp -f '"+fname+"' tmpeps.eps && epstopdf tmpeps.eps && cp -f tmpeps.pdf '"+remove_extension(fname)+".pdf' &").c_str());
      }
#else // not used anymore, assuming convert is avail. on linux
      if (pngpdf & 0x1)
	system(("pstopnm -stdout -portrait "+fname+" | pnmtopng > "+remove_extension(fname)+".png &").c_str()); 
      if (pngpdf & 0x2)
	system(("epstopdf "+fname+" &").c_str()); 
      if (pngpdf & 0x4)
	system(("pstopnm -stdout -portrait "+fname+" | pnmtojpeg > "+remove_extension(fname)+".jpg &").c_str()); 
#endif
#endif
    }
    // Preview
    if (ps_preview!="no" && preview){
#ifdef WIN32
      system_browser_command(fname);
      /*
      string fn=absolute_path(fname);
      if (system((((ps_preview+" ")+fn)+" &").c_str())){
	cerr << (("cygstart.exe '"+remove_extension(fname)+".jpg' &").c_str()) << endl;
	system((xcasroot()+"cygstart.exe '"+remove_extension(fname)+".jpg' &").c_str());
      }
      */
#else
      system((((ps_preview+" ")+fname)+" &").c_str());
#endif
    }
    if (eps && pngpdf){
      fname=remove_path(remove_extension(fname));
      unsigned fnames=fname.size();
      static string tmp;
      tmp=fname+".eps/.png ready for import.\nUse \\includegraphics[width=\\textwidth]{"+fname+"}\nwhere [] is optional, width may also be e.g. 10cm\ninside your latex document, with header\n\\usepackage{graphicx}\n(PNG export requires pstopnm and pnmtopng)";
      if (Xcas_help_output){
	Xcas_help_output->value(tmp.c_str());
	Xcas_help_output->position(32+fnames,68+2*fnames);
	Fl::copy(Xcas_help_output->Fl_Output::value()+32+fnames,36+fnames,0);
	Fl::copy(Xcas_help_output->Fl_Output::value()+32+fnames,36+fnames,1);
	Fl::focus(Xcas_help_output);
      }
      else
	fl_message("%s",tmp.c_str());
    }
#endif
  }

  void widget_print(Fl_Widget * widget){
#ifdef Fl_Printer_H
#ifdef FL_DEVICE
    Fl_PrintDevice * p = fl_printer_chooser(); 
    if (p){
      // Scan all graph3d widget inside and move their state to printing=p stream
      if (Fl_PostScript_Graphics_Driver * ps=dynamic_cast<Fl_PostScript_Graphics_Driver * >(p))
	setgraph3dprintstate(widget,ps->file(),true);
      in_widget_print(widget,p,false);
      delete p;
      // Scan all graph3d widget inside and move their state to printing=true
      setgraph3dprintstate(widget,0,false);
    }
#else
    Fl_Printer * p = new Fl_Printer();
    if (p->start_job(1))
      return; // unable to print
    if (p){
      // Scan all graph3d widget inside and move their state to printing=p stream
      if (Fl_PostScript_Graphics_Driver * ps=dynamic_cast<Fl_PostScript_Graphics_Driver * >(p))
	setgraph3dprintstate(widget,ps->file(),true);
      in_widget_print(widget,p,false);
      p->end_job();
      delete p;
      // Scan all graph3d widget inside and move their state to printing=true
      setgraph3dprintstate(widget,0,false);
    }
#endif
#endif
  }

#ifndef NO_NAMESPACE_XCAS
} // namespace xcas
#endif // ndef NO_NAMESPACE_XCAS

#endif // HAVE_LIBFLTK
