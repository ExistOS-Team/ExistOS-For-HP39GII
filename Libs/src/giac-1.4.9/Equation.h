// -*- mode:C++ ; compile-command: "g++ -I.. -g -c Equation.cc" -*-
#ifndef _EQUATION_H
#define _EQUATION_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "vector.h"
#include <string>
#ifndef IN_GIAC
#include <giac/giac.h>
#else
#include "giac.h"
#endif
#ifdef HAVE_LIBFLTK
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Scrollbar.H>
#include <FL/Fl_Multiline_Input.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Help_Dialog.H>
#endif
#ifdef HAVE_LC_MESSAGES
#include <locale.h>
#endif
#ifndef IN_GIAC
#include <giac/giacintl.h>
#else
#include "giacintl.h"
#endif

#ifndef NO_NAMESPACE_XCAS
namespace xcas {
#endif // ndef NO_NAMESPACE_XCAS

#ifdef HAVE_LIBFLTK

  Fl_Font cst_greek_translate(std::string & s);
  // utility for gen embedded widget memory desallocation
  void fltk_fl_widget_delete_function(void * ptr);

  void Equation_cb_scroll(Fl_Widget*, void*);

  // maximum "size" of symbolics displayed in an Equation (pretty print)
  extern unsigned max_prettyprint_equation; 
  class Equation;

  class Equation_Scrollbar : public Fl_Scrollbar {
  public:
    Equation * eqwptr;
    bool vertical;
    Equation_Scrollbar(int x,int y,int w,int h,Equation * ptr,bool is_vertical);
  };

  // Usage: in fluid, declare a Group, then make a Box with class Equation
  // Make sure there is enough room in the group for the scrollbars
  class Equation:public Fl_Group {
    virtual FL_EXPORT void draw();
    virtual FL_EXPORT int handle(int);
    virtual FL_EXPORT int in_handle(int);
  public:
    giac::gen lastdata;
    giac::gen data; // of type eqwdata or undef if empty
    giac::attributs attr;
    bool modifiable;
    bool output_equation; // true for output equation, false for input eq
    bool need_active_parse; // true if the active cell must be parsed
    int xleft,ytop; // position in pixels of the origin (global shift of image)
    int xsel,ysel; // begin mouse selection (PUSH event)
    int xcur,ycur; // position of the cursor (RELEASE event)
    int active_pos; // position of the cursor in string (active mode)
    int begin_sel,end_sel; // -1 do not take care, >=0 position of sel 
    int clip_x,clip_y,clip_w,clip_h;
    void (* cb_enter) (Fl_Widget *,void * ); // callback when enter pressed
    void (* cb_escape) (Equation * ); // callback when escape pressed
    void (* cb_backspace) (Equation * ); // callback when full level selected and backspace pressed
    void (* cb_select) (const char * ); // callback when selection changed
    Equation_Scrollbar * vscroll,* hscroll;
    Fl_Menu_Bar * menubar;
    Equation(int x, int y, int w, int h,const char * l=0);
    Equation(int x, int y, int w, int h, const char* l,const giac::gen & g);
    Equation(int x, int y, int w, int h, const char* l,const giac::gen & g,giac::attributs mya);
    Equation(int x, int y, int w, int h, const char* l,const giac::gen & g,giac::attributs mya,const giac::context * contextptr);
    ~Equation();
    void add_scroll_menu();
    void deselect();
    void select(); // Does not copy to clipboard
    void fl_select(); // Copy selection to clipboard
    void select_rectangle(int x,int y); // x,y are the end of the mouse selection
    void select_up(int mode); // mode is 1 for shifted key
    void select_down(int mode);
    void select_right(int mode);
    void select_left(int mode);
    void adjust_widget_size(); // adjust widget size
    void adjust_xy(); // recalculate xsel, ysel to begin top of selection
    // and xcur, ycur to end bottom of selection
    void adjust_xy_sel(); // same + recalc xleft
    void eval_function(const giac::gen & f);
    // active_search is 1 if replacing active cell, 0 for selected cell
    // Active cell should always be an _EQW terminal data of type string
    bool replace_selection(const giac::gen & f,bool active_search=false);
    bool replace_selection_wo_save(const giac::gen & f,bool active_search=false);
    void set_data(const giac::gen & g);
    giac::gen get_selection();
    giac::gen get_data() const;
    std::string value() const;
    void setscroll();
    void deselect_and_activate();
    void desactivate_and_select();
    void insure_something_selected(bool selectall=false);
    bool handle_key(unsigned char c,giac::gen * act);
    bool handle_text(const std::string & s,giac::gen * act);
    bool handle_text(const std::string & paste_s_orig);
    void remove_selection();
    giac::gen * set_active(int x,int y); // activate cell at position x,y
    // return a pointer to the active cell giac::gen or 0
    bool parse_desactivate(); // if false nothing was active, else select
    void replace_down_left_activate(const giac::gen & g);
    int undo_history_pos; // current position in eqw history
    void save_data(); // save data to undo history
    void rcl_data(int dpos); // move from dpos in history and rcl data
    // keep undo operations max_history_size times in undo_history
    giac::vecteur undo_history; 
    bool is_selected(bool inside=false,bool active_search=false);
    void resize(int x, int y, int w, int h);
  };

  void Equation_eval_callback(const giac::gen & evaled_g,void * param);

  // initialize to the inner form of a null active string
  giac::gen Equation_nullstring();
  giac::gen Equation_nullstring(const giac::attributs & a,int windowhsize,const giac::context * contextptr);
  giac::gen Equation_nullhistlevel();
  // function used internally
  void Equation_select(giac::gen & g,bool select,bool active_search=false);
  giac::gen Equation_compute_size(const giac::gen & g,const giac::attributs & a,int windowhsize,const giac::context * contextptr);
  giac::eqwdata Equation_total_size(const giac::gen & g);  
  // Equation_translate(giac::gen & g,int deltax,int deltay);
  void Equation_vertical_adjust(int hp,int yp,int & h,int & y);
  bool Equation_find_vector_pos(giac::const_iterateur it,giac::const_iterateur itend,int & i,int &nrows);
  // return -1 if not in a multistring, position otherwise
  int in_multistring(const giac::gen & data,giac::vecteur * & vptr);
  int in_multistring(const giac::gen & data);
  void handle_newline(Equation * eqwptr);
  // return a pointer to the selected object
  // it will modify g if for example a subsum of a sum is selected
  giac::gen * Equation_selected(giac::gen & g,giac::attributs & attr,int windowhsize,giac::vecteur & position,bool active_search,const giac::context *);

  // return a variable from the list of vars
  bool select_user_var(int dx,int dy,giac::gen & ans,const giac::context * contextptr);

#endif // HAVE_LIBFLTK

#ifndef NO_NAMESPACE_XCAS
} // namespace xcas
#endif // ndef NO_NAMESPACE_XCAS

#endif // _EQUATION_H
