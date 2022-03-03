// -*- mode:C++ ; compile-command: "g++ -I.. -g -c Help1.cc" -*-
#ifndef _HELP_H
#define _HELP_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifndef IN_GIAC
#include <giac/first.h>
#include <giac/gen.h>
#include <giac/identificateur.h>
#else
#include "first.h"
#include "gen.h"
#include "identificateur.h"
#endif
#include "vector.h"
#include <string>
#include <iostream>
#ifdef HAVE_LIBFLTK
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Scrollbar.H>
#include <FL/Fl_Multiline_Input.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Help_Dialog.H>
#endif
#ifdef HAVE_LC_MESSAGES
#include <locale.h>
#endif
#include "giacintl.h"

#ifndef NO_NAMESPACE_XCAS
namespace xcas {
#endif // ndef NO_NAMESPACE_XCAS

  std::string translate_html_title(const std::string & s);

  std::string help_translate(const std::string & s);

  void help_fltk(const std::string &);



#ifndef NO_NAMESPACE_XCAS
} // namespace xcas
#endif // ndef NO_NAMESPACE_XCAS

#endif // _HELP_H
