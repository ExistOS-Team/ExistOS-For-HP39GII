// -*- mode:C++ ; compile-command: "g++ -I.. -g -c Xcas1.cc" -*-
#ifndef _XCAS_H
#define _XCAS_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifndef IN_GIAC
#include <giac/first.h>
#else
#include "first.h"
#endif
#ifdef HAVE_LIBFLTK
#include <FL/Fl_Widget.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Multiline_Input.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Double_Window.H>
#endif
#include <string>
#include <iostream>
#include "History.h"
#include "Graph.h"
#include "Input.h"
#ifndef IN_GIAC
#include <giac/giac.h>
#include <giac/input_lexer.h>
#else
#include "giac.h"
#include "input_lexer.h"
#endif

#ifndef NO_NAMESPACE_XCAS
namespace xcas {
#endif // ndef NO_NAMESPACE_XCAS

#ifdef HAVE_LIBFLTK
  // when true a figure is run after load
  extern bool geo_run;
  // when true a sheet is reevaled after load
  extern bool sheet_run;
  // when true context is not restored
  extern bool recovery_mode;
  // number of fonts available
  extern int fonts_available;
  // save format is compatible with Xcas < 0.8.1
  extern bool file_save_context;

  extern bool autosave_disabled;

  // List of widgets that might be defined by the application
  extern Enlargable_Multiline_Output *Xcas_help_output ;  
  class Graph2d;
  extern Graph2d *Xcas_DispG;
  class Equation;
  extern Equation * Xcas_PrintG;
  void xcas_gprintf(unsigned special,const std::string & format,const giac::vecteur & v,const giac::context *);
  extern Fl_Window * Xcas_DispG_Window;
  extern Fl_Window * Xcas_Main_Window;
  extern Fl_Button *Xcas_DispG_Cancel;
  extern Fl_Button *Xcas_Cancel;
  extern Fl_Double_Window * Xcas_Debug_Window;
  extern int xcas_dispg_entries;

  bool stream_copy(std::istream & in,std::ostream & out);
  extern bool interrupt_button;
  std::string print_DOUBLE_(double d);
  extern void (*initialize_function)();
  // used by has_autorecover_data
  struct time_string {
    long t;
    std::string s;
    time_string():t(0){};
    time_string(long _t,std::string _s):t(_t),s(_s){};
  };

  bool operator < (const time_string & ts1,const time_string & ts2);

  bool has_autorecover_data(const std::string & s,std::vector<time_string> & newest);

  // replace c1 by c2 in s
  std::string replace(const std::string & s,char c1,char c2);

  // Check for auto-recovery data in directory s
  bool has_autorecover_data(const std::string & s,std::string & newest);
  extern int autosave_time ; /* default save session every 60 seconds */
  extern bool (*autosave_function)(bool); 
  extern void (*idle_function)(); 
  /* invoked for autosaving, default find above History_Fold and autosave */

  void Xcas_debugguer(int status,giac::context * contextptr);
  void Xcas_idle_function(void *);
  extern void (* menu2rpn_callback)(Fl_Widget *,void *);
  // wait max 0.0001 second and call Xcas_idle_function
  void fl_wait_0001(giac::context *);
  giac::gen thread_eval(const giac::gen & g,int level,giac::context * contextptr);

  void copy_menu(Fl_Menu_ * menu,const std::string & prefix,Fl_Menu_Item * & m);
  bool add_user_menu(Fl_Menu_ *m,const std::string & s,const std::string & doc_prefix, Fl_Callback * cb);// load user menu
  void browser_help(const giac::gen & f,int language); // Help on object f
  void check_browser_help(const giac::gen & g,int language); // Help on g is it's a ?()
  void help_output(const std::string & s,int language); // help in Xcas_help_output

  // Apparence
  void nextfl_menu(Fl_Menu_Item * & m);
  giac::vecteur fl_menu2rpn_menu(Fl_Menu_Item * & m);
  void change_menu_fontsize(Fl_Menu_Item * m,int n,int labelfontsize);
  void change_menu_fontsize(Fl_Menu * & m,int labelfontsize);
  void change_group_fontsize(Fl_Widget * g,int labelfontsize);
  // Giac interactive functions fltk implementation
  giac::gen Xcas_fltk_interactive(const giac::gen & g,const giac::context * contextptr);
  giac::gen Xcas_fltk_getKey(const giac::gen & g,const giac::context * contextptr);
  giac::gen Xcas_fltk_input(const giac::gen & arg,const giac::context * contextptr);
  giac::gen Xcas_fltk_inputform(const giac::gen & arg,const giac::context * contextptr);
  giac::gen makeform(const giac::vecteur & v,const giac::context * contextptr); // interface for inputforms

  class Xcas_config_type {
  public:
    int fontsize;
    int help_fontsize;
    double window_xmin,window_xmax,window_ymin,window_ymax,window_zmin,window_zmax;
    bool ortho,autoscale;
#ifdef IPAQ
    Xcas_config_type():fontsize(14),help_fontsize(14),window_xmin(-4),window_xmax(4),window_ymin(-5),window_ymax(5),window_zmin(-5),window_zmax(5),ortho(false),autoscale(true) {};
#else
    Xcas_config_type():fontsize(14),help_fontsize(14),window_xmin(-5),window_xmax(5),window_ymin(-5),window_ymax(5),window_zmin(-5),window_zmax(5),ortho(false),autoscale(true) {};
#endif
    Xcas_config_type(int f,int hf,double x,double X,double y,double Y,bool _ortho):fontsize(f),help_fontsize(hf),window_xmin(x),window_xmax(X),window_ymin(y),window_ymax(Y),window_zmin(-5),window_zmax(5),ortho(_ortho),autoscale(true) {}
    Xcas_config_type(int f,int hf,double x,double X,double y,double Y,double z,double Z,bool _ortho):fontsize(f),help_fontsize(hf),window_xmin(x),window_xmax(X),window_ymin(y),window_ymax(Y),window_zmin(z),window_zmax(Z),ortho(_ortho),autoscale(true) {}
  };
  extern Xcas_config_type Xcas_config;

  std::string cut_help(const std::string & s,int fontsize,int w);

  Fl_Widget * Xcas_eval(Fl_Widget * w) ;
  const char * Xcas_pack_select(const xcas::History_Pack * pack,int sel_begin,int sel_end);
  int Xcas_pack_insert(xcas::History_Pack * pack,const char * chaine,int length,int before_position);
  Fl_Widget * widget_load(const std::string & s,int L,int & i,const giac::context *,int widgetw=0);
  std::string widget_sprint(const Fl_Widget * o);
  std::string widget_html5(const Fl_Widget * o);

  void cb_Insert_ItemName(Fl_Widget * w , void*) ;

  bool find_fold_autosave_function(bool);
  bool get_font(Fl_Font & police,int & taille);

  History_Fold * load_history_fold(int sx,int sy,int sw,int sh,int sl,const char * filename,bool modified); // might be used from icas.cc to call xcas online
#endif

  bool fltk_view(const giac::gen & g,giac::gen & ge,const std::string & filename,std::string & figure_filename,int file_type,const giac::context *);

  void icas_eval(giac::gen & g,giac::gen & gg,int & reading_file,std::string &filename,giac::context * contextptr);

  int read_file(const giac::gen & g);


#ifndef NO_NAMESPACE_XCAS
} // namespace xcas
#endif // ndef NO_NAMESPACE_XCAS

#endif // _XCAS_H
