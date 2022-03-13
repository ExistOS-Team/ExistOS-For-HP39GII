// -*- mode:C++ ; compile-command: "g++ -I.. -g -c Cfg.cc" -*-
#ifndef _CFG_H
#define _CFG_H
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



#ifndef NO_NAMESPACE_XCAS
namespace xcas {
#endif // ndef NO_NAMESPACE_XCAS

#ifdef HAVE_LIBFLTK


  void Xcas_load_cas_setup(const giac::context * contextptr);
  void Xcas_load_graph_setup(const giac::context * contextptr);
  extern Fl_Double_Window *Xcas_Cas_Setup ;
  extern Fl_Double_Window *Xcas_Plot_Setup ;

  extern void (* Xcas_save_config_ptr)(const giac::context * contextptr) ;
  extern void (* Xcas_update_mode_ptr)(void) ;

#endif // HAVE_LIBFLTK

#ifndef NO_NAMESPACE_XCAS
} // namespace xcas
#endif // ndef NO_NAMESPACE_XCAS

#endif // _CFG_H
