// -*- mode:C++ ; compile-command: "g++ -I.. -g -c Print.cc" -*-
#ifndef _PRINT_H
#define _PRINT_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "vector.h"
#include <string>
#ifndef IN_GIAC
#include <giac/first.h>
#include <giac/gen.h>
#else
#include "first.h"
#include "gen.h"
#endif
#ifdef HAVE_LIBFLTK
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Scrollbar.H>
#include <FL/Fl_Multiline_Input.H>
#include "FL/Fl_Help_Dialog.H"
#endif
#ifdef HAVE_LC_MESSAGES
#include <locale.h>
#endif
#include "giacintl.h"

#ifdef HAVE_LIBFLTK
#ifdef FL_DEVICE
#include <FL/fl_printer_chooser.H>
#else
#include <FL/Fl_Printer.H>
#endif // FL_DEVICE
#endif


#ifndef NO_NAMESPACE_XCAS
namespace xcas {
#endif // ndef NO_NAMESPACE_XCAS

#ifdef HAVE_LIBFLTK

  extern int printer_format;
  extern bool printer_landscape;
  extern double pixel_scale; // size of 1 pixel on a page for printing

  void widget_ps_print(Fl_Widget * widget,const std::string & fname0,bool eps=false,int pngpdf=7,bool preview=true,bool hidemouseparam=true,bool askusersize=true);
  void widget_print(Fl_Widget * widget);

#endif

#ifndef NO_NAMESPACE_XCAS
} // namespace xcas
#endif // ndef NO_NAMESPACE_XCAS

#endif // _PRINT_H
