// -*- mode:C++ ; compile-command: "g++ -I.. -g -c Equation.cc" -*-
#ifndef _EQUATION_H
#define _EQUATION_H
#include "config.h"
#include "giacPCH.h"
#include "misc.h"

#define STATUS_AREA_PX 0

#ifndef NO_NAMESPACE_XCAS
namespace xcas {
#endif // ndef NO_NAMESPACE_XCAS

  // maximum "size" of symbolics displayed in an Equation (pretty print)
  extern unsigned max_prettyprint_equation;
  // matrix select
  bool eqw_select(const giac::gen & eq,int l,int c,bool select,giac::gen & value);
  void Equation_select(giac::gen & eql,bool select);
  int eqw_select_down(giac::gen & g);
  int eqw_select_up(giac::gen & g);

  giac::gen Equation_compute_size(const giac::gen & g,const giac::attributs & a,int windowhsize,const giac::context * contextptr);
  giac::eqwdata Equation_total_size(const giac::gen & g);  
  // Equation_translate(giac::gen & g,int deltax,int deltay);
  void Equation_vertical_adjust(int hp,int yp,int & h,int & y);
  bool Equation_find_vector_pos(giac::const_iterateur it,giac::const_iterateur itend,int & i,int &nrows);
  bool Equation_adjust_xy(giac::gen & g,int & xleft,int & ytop,int & xright,int & ybottom,giac::gen * & gsel,giac::gen * & gselparent,int &gselpos,std::vector<int> * gotosel=0);
  // select and set value from eqwdata in eql
  bool do_select(giac::gen & eql,bool select,giac::gen & value);

  class Equation {
    int _x,_y;
  public:
    giac::gen data,undodata; // of type eqwdata or undef if empty
    giac::attributs attr;
    int x() const { return _x;}
    int y() const { return _y;}
    Equation(int x_, int y_, const giac::gen & g);
  };

  giac::gen Equation_copy(const giac::gen & g);
  void display(Equation &eq ,int x,int y);
  // replace selection in eq by tmp
  void replace_selection(Equation & eq,const giac::gen & tmp,giac::gen * gsel=0,const std::vector<int> * gotoptr=0);
  int eqw_select_leftright(xcas::Equation & g,bool left,int exchange=0);

  class Graph2d{
  public:
    double window_xmin,window_xmax,window_ymin,window_ymax,
      x_scale,y_scale,x_tick,y_tick;
    int display_mode,show_axes,show_names,labelsize;
    int current_i,current_j;
    int tracemode;
    int tracemode_n; double tracemode_i; string tracemode_add; giac::vecteur tracemode_disp; double tracemode_mark;
    giac::gen g;//,cache_parameq,cache_t,cache_x,cache_x1,cache_x2,cache_y0,cache_y,cache_y1,cache_y2;
    giac::vecteur plot_instructions;
    void invert_tracemode();
    void tracemode_set(int operation=0); // operation==1 if user is setting the value of t on a parametric curve, operation==2 for root, operation==3 for extremum, operation==4 mark current position, operation=5 for area
    void init_tracemode();
    void draw_decorations();
    void curve_infos();
    bool findij(const giac::gen & e0,double x_scale,double y_scale,double & i0,double & j0,const giac::context * ) const;
    void update();
    void zoomx(double d,bool round=false);
    void zoomy(double d,bool round=false);
    void zoom(double);
    void left(double d);
    void right(double d);
    void up(double d);
    void down(double d);
    void autoscale(bool fullview=false);
    void orthonormalize();
    void draw();
    Graph2d(const giac::gen & g_);
  };

  struct Turtle {
    void draw();
#ifdef TURTLETAB
    giac::logo_turtle * turtleptr;
#else
    std::vector<giac::logo_turtle> * turtleptr;
#endif
    int turtlex,turtley; // Turtle translate
    double turtlezoom; // Zoom factor for turtle screen
    int maillage; // 0 (none), 1 (square), 2 (triangle), bit3 used for printing
  };
  
#ifndef NO_NAMESPACE_XCAS
} // namespace xcas
#endif // ndef NO_NAMESPACE_XCAS

#endif // _EQUATION_H
