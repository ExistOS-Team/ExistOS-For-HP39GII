// -*- mode:C++ ; compile-command: "g++ -I. -I.. -g -c Cfg.cc -Wall" -*-
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
#include <FL/fl_ask.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Counter.H>
#include <FL/Fl_Tile.H>
#include <FL/Fl_Output.H>
#endif
#ifndef IN_GIAC
#include <giac/global.h>
#include <giac/gen.h>
#include <giac/prog.h>
#else
#include "global.h"
#include "gen.h"
#include "prog.h"
#endif
#include "History.h"
#include "Xcas1.h"
#include "Cfg.h"

using namespace std;
using namespace giac;

#ifndef NO_NAMESPACE_XCAS
namespace xcas {
#endif // ndef NO_NAMESPACE_XCAS

#ifdef HAVE_LIBFLTK

  // xcas_cas_setup

  int do_scientific_format,do_eval_level,do_maple_mode,do_integer_format,do_max_recursion_level,do_debug_infolevel;

  void (* Xcas_save_config_ptr)(GIAC_CONTEXT) = 0;
  void (* Xcas_update_mode_ptr)(void) = 0;

  Fl_Double_Window *Xcas_Cas_Setup=(Fl_Double_Window *)0;

  Fl_Menu_Button *Xcas_Float_style=(Fl_Menu_Button *)0;

  Fl_Output *Xcas_Float_style_output=(Fl_Output *)0;

  Fl_Value_Input *Xcas_Epsilon=(Fl_Value_Input *)0;

  Fl_Value_Input *Xcas_Proba_Epsilon=(Fl_Value_Input *)0;

  Fl_Check_Button *Xcas_Approx_mode=(Fl_Check_Button *)0;

  Fl_Input *Xcas_Autosimplify=(Fl_Input *)0;

  Fl_Check_Button *Xcas_Angle_radian=(Fl_Check_Button *)0;

  Fl_Check_Button *Xcas_Increasing_power=(Fl_Check_Button *)0;

  Fl_Menu_Button *Xcas_Prg_style=(Fl_Menu_Button *)0;

  Fl_Output *Xcas_Style=(Fl_Output *)0;

  Fl_Check_Button *Xcas_Complex_mode=(Fl_Check_Button *)0;

  Fl_Check_Button *Xcas_Complex_variables=(Fl_Check_Button *)0;

  Fl_Return_Button *Xcas_Cas_setup_OK=(Fl_Return_Button *)0;

  Fl_Button *Xcas_cas_setup_OKSave=(Fl_Button *)0;

  Fl_Button *Xcas_Cancel_cas_setup=(Fl_Button *)0;

  Fl_Value_Input *Xcas_Threads=(Fl_Value_Input *)0;

  Fl_Value_Input *Xcas_Newton=(Fl_Value_Input *)0;

  Fl_Check_Button *Xcas_sqrt=(Fl_Check_Button *)0;

  Fl_Check_Button *Xcas_all_trig_sol=(Fl_Check_Button *)0;

  Fl_Value_Input *Xcas_Set_Digits=(Fl_Value_Input *)0;

  Fl_Menu_Button *Xcas_Integer_style=(Fl_Menu_Button *)0;

  Fl_Output *Xcas_Integer_style_output=(Fl_Output *)0;

  Fl_Value_Input *Xcas_debug_infolevel=(Fl_Value_Input *)0;

  Fl_Value_Input *Xcas_eval_level=(Fl_Value_Input *)0;

  Fl_Value_Input *Xcas_recursion_level=(Fl_Value_Input *)0;

  Fl_Value_Input *Xcas_prog_eval_level=(Fl_Value_Input *)0;

  static void cb_Xcas_prog_eval_level(Fl_Value_Input*, void*) {
    do_eval_level=min(1,int(Xcas_eval_level->value()));
  }

  static void cb_Xcas_Float_style_standard(Fl_Menu_*, void*) {
    do_scientific_format=0; Xcas_Float_style_output->value("standard");
  }

  static void cb_Xcas_Float_style_scientific(Fl_Menu_*, void*) {
    do_scientific_format=1; Xcas_Float_style_output->value("scientific");
  }

  static void cb_Xcas_Float_style_engineer(Fl_Menu_*, void*) {
    do_scientific_format=2; Xcas_Float_style_output->value("engineer");
  }

  static void cb_Xcas_Float_style_hexa(Fl_Menu_*, void*) {
    do_scientific_format=3; Xcas_Float_style_output->value("hexa");
  }

  Fl_Menu_Item menu_Xcas_Float_style[] = {
    {gettext("Standard"), 0,  (Fl_Callback*)cb_Xcas_Float_style_standard, 0, 0, 0, 0, 14, 56},
    {gettext("Scientific"), 0,  (Fl_Callback*)cb_Xcas_Float_style_scientific, 0, 0, 0, 0, 14, 56},
    {gettext("Engineer"), 0,  (Fl_Callback*)cb_Xcas_Float_style_engineer, 0, 0, 0, 0, 14, 56},
    {gettext("Engineer"), 0,  (Fl_Callback*)cb_Xcas_Float_style_hexa, 0, 16, 0, 0, 14, 56},
    {0}
  };

  static void cb_Xcas_Set_CPP(Fl_Menu_*, void*) {
    do_maple_mode=0; Xcas_Style->value("Xcas");
  }

  static void cb_Xcas_Set_Maple(Fl_Menu_*, void*) {
    do_maple_mode=1; Xcas_Style->value("Maple");
  }

  static void cb_Xcas_Set_Mupad(Fl_Menu_*, void*) {
    do_maple_mode=2; Xcas_Style->value("Mupad");
  }

  static void cb_Xcas_Set_TI(Fl_Menu_*, void*) {
    do_maple_mode=3; Xcas_Style->value("TI89/92");
  }

  Fl_Menu_Item menu_Xcas_Prg_style[] = {
    {gettext("Xcas"), 0,  (Fl_Callback*)cb_Xcas_Set_CPP, 0, 0, 0, 0, 14, 56},
    {gettext("Maple"), 0,  (Fl_Callback*)cb_Xcas_Set_Maple, 0, 0, 0, 0, 14, 56},
    {gettext("Mupad"), 0,  (Fl_Callback*)cb_Xcas_Set_Mupad, 0, 0, 0, 0, 14, 56},
    {gettext("TI89/92"), 0,  (Fl_Callback*)cb_Xcas_Set_TI, 0, 0, 0, 0, 14, 56},
    {0}
  };

  static void cb_Xcas_Cas_setup_OK(Fl_Return_Button* rb, void* userdata) {
    const giac::context * contextptr = context0;
    if (userdata)
      contextptr=(const giac::context *)(userdata);
    Xcas_Cancel_cas_setup->window()->hide();
    giac::epsilon(fabs(Xcas_Epsilon->value()),contextptr);
    giac::proba_epsilon(contextptr)=fabs(Xcas_Proba_Epsilon->value());
    giac::threads=max(giac::absint(int(Xcas_Threads->value())),1);
    // giac::variables_are_files(Xcas_Save_var->value(),contextptr);
    giac::complex_mode(Xcas_Complex_mode->value(),contextptr);
    giac::complex_variables(Xcas_Complex_variables->value(),contextptr);
    giac::xcas_mode(contextptr)=do_maple_mode;
    giac::increasing_power(Xcas_Increasing_power->value(),contextptr);
    giac::angle_radian(Xcas_Angle_radian->value(),contextptr);
    giac::approx_mode(Xcas_Approx_mode->value(),contextptr);
    if (strlen(Xcas_Autosimplify->value())<3)
      giac::_autosimplify(0,contextptr);
    else
      giac::_autosimplify(gen(Xcas_Autosimplify->value(),contextptr),contextptr);
    giac::scientific_format(do_scientific_format,contextptr);
    giac::integer_format(do_integer_format,contextptr);
    giac::MAX_RECURSION_LEVEL=do_max_recursion_level;
    giac::eval_level(contextptr)= giac::DEFAULT_EVAL_LEVEL=do_eval_level;
    giac::prog_eval_level_val(int(Xcas_prog_eval_level->value()),contextptr);
    giac::NEWTON_DEFAULT_ITERATION=max(giac::absint(int(Xcas_Newton->value())),20);
    giac::debug_infolevel=do_debug_infolevel;
    giac::withsqrt(Xcas_sqrt->value(),contextptr);
    giac::all_trig_sol(Xcas_all_trig_sol->value(),contextptr);
    giac::set_decimal_digits(int(Xcas_Set_Digits->value()),contextptr);
    if (Xcas_update_mode_ptr)
      Xcas_update_mode_ptr();
    History_Fold * hf=get_history_fold(Xcas_input_focus);
    if (hf){
      hf->update_status_count=-1;
      hf->update_status();
    }
    if (Xcas_input_focus)
      Fl::focus(Xcas_input_focus);
  }

  static void cb_Xcas_cas_setup_OKSave(Fl_Button*, void* userdata) {
    cb_Xcas_Cas_setup_OK(0,userdata); 
    const giac::context * contextptr = giac::context0;
    if (userdata)
      contextptr=(const giac::context *) userdata;
    if (Xcas_save_config_ptr)
      Xcas_save_config_ptr(contextptr);
  }

  static void cb_Xcas_Cancel_cas_setup(Fl_Button*, void*) {
    Xcas_Cancel_cas_setup->window()->hide();
    if (Xcas_input_focus)
      Fl::focus(Xcas_input_focus);
  }

  static void cb_Xcas_Integer_style_standard(Fl_Menu_*, void*) {
    do_integer_format=10; 
    Xcas_Integer_style_output->value("10");
  }

  static void cb_Xcas_Integer_style_hexa(Fl_Menu_*, void*) {
    do_integer_format=16; 
    Xcas_Integer_style_output->value("16");
  }

  static void cb_Xcas_Integer_style_octal(Fl_Menu_*, void*) {
    do_integer_format=8; 
    Xcas_Integer_style_output->value("8");
  }

  Fl_Menu_Item menu_Xcas_Integer_style[] = {
    {gettext("10"), 0,  (Fl_Callback*)cb_Xcas_Integer_style_standard, 0, 0, 0, 0, 14, 56},
    {gettext("16"), 0,  (Fl_Callback*)cb_Xcas_Integer_style_hexa, 0, 0, 0, 0, 14, 56},
    {gettext("8"), 0,  (Fl_Callback*)cb_Xcas_Integer_style_octal, 0, 0, 0, 0, 14, 56},
    {0}
  };

  static void cb_Xcas_debug_infolevel(Fl_Value_Input*, void*) {
    do_debug_infolevel=int(Xcas_debug_infolevel->value());
  }

  static void cb_Xcas_eval_level(Fl_Value_Input*, void*) {
    do_eval_level=int(Xcas_eval_level->value());
  }

  static void cb_Xcas_recursion_level(Fl_Value_Input*, void*) {
    do_max_recursion_level=int(Xcas_recursion_level->value());
  }

  void xcas_cas_setup_init()
  {
    Fl_Group::current(0);
    Fl_Double_Window* o = Xcas_Cas_Setup = new Fl_Double_Window(20,80,355, 315, gettext("Xcas Cas Setup"));
    { Fl_Menu_Button* o = Xcas_Float_style = new Fl_Menu_Button(125, 15, 95, 25, gettext("Float format"));
    o->align(FL_ALIGN_CLIP);
    o->menu(menu_Xcas_Float_style);
    }
    { Fl_Output* o = Xcas_Float_style_output = new Fl_Output(125, 40, 95, 25);
    o->tooltip(gettext("Current float printing format"));
    o->labeltype(FL_NO_LABEL);
    o->align(68);
    }
    { Fl_Value_Input* o = Xcas_Epsilon = new Fl_Value_Input(125, 115, 50, 25, gettext("epsilon"));
    o->tooltip(gettext("Number with absolute value less than epsilon can be coerced to 0"));
    o->minimum(1e-12);
    o->maximum(0.01);
    o->value(1e-10);
    o->align(72);
    }
    { Fl_Value_Input* o = Xcas_Proba_Epsilon = new Fl_Value_Input(125, 145, 50, 25, gettext("proba"));
    o->tooltip(gettext("Maximal probability of a wrong answer for non-deterministic algorithms. Set to 0 for deterministic algorithms only."));
    o->minimum(0);
    o->maximum(1e-4);
    o->value(1e-15);
    o->align(72);
    }
    { Fl_Check_Button* o = Xcas_Approx_mode = new Fl_Check_Button(125, 175, 105, 25, gettext("approx"));
    o->tooltip(gettext("Approx or exact mode"));
    o->down_box(FL_DOWN_BOX);
    o->selection_color((Fl_Color)1);
    o->align(68|FL_ALIGN_INSIDE);
    }
    { Fl_Input* o = Xcas_Autosimplify = new Fl_Input(175, 205, 55, 25, gettext("autosimplify"));
    o->tooltip(gettext("Command automatically executed after evaluation (auto-simplification) for example nop or regroup or simplify"));
    o->value("regroup");
    }
    { Fl_Check_Button* o = Xcas_Angle_radian = new Fl_Check_Button(240, 75, 100, 25, gettext("radian"));
    o->tooltip(gettext("Radian or degree angle mode"));
    o->down_box(FL_DOWN_BOX);
    o->selection_color((Fl_Color)1);
    o->align(68|FL_ALIGN_INSIDE);
    }
    { Fl_Check_Button* o = Xcas_Increasing_power = new Fl_Check_Button(240, 180, 100, 25, gettext("increasing power"));
    o->tooltip(gettext("Show polynomials in increasin or decreasing order"));
    o->down_box(FL_DOWN_BOX);
    o->selection_color((Fl_Color)1);
    o->align(68|FL_ALIGN_INSIDE);
    }
    { Fl_Menu_Button* o = Xcas_Prg_style = new Fl_Menu_Button(15, 15, 90, 25, gettext("Prog style"));
    o->align(FL_ALIGN_CLIP);
    o->menu(menu_Xcas_Prg_style);
    }
    { Fl_Output* o = Xcas_Style = new Fl_Output(15, 40, 90, 25);
    o->tooltip(gettext("Current programmation syntax style"));
    o->labeltype(FL_NO_LABEL);
    o->align(68);
    }
    { Fl_Check_Button* o = Xcas_Complex_mode = new Fl_Check_Button(240, 111, 105, 25, gettext("Complex"));
    o->tooltip(gettext("Complex mode (e.g. for factorization)"));
    o->down_box(FL_DOWN_BOX);
    o->selection_color((Fl_Color)1);
    o->align(68|FL_ALIGN_INSIDE);
    }
    { Fl_Check_Button* o = Xcas_Complex_variables = new Fl_Check_Button(240, 146, 105, 25, gettext("Cmplx_var"));
    o->tooltip(gettext("Variables are complex or real"));
    o->down_box(FL_DOWN_BOX);
    o->selection_color((Fl_Color)1);
    o->align(68|FL_ALIGN_INSIDE);
    }
    { Fl_Return_Button* o = Xcas_Cas_setup_OK = new Fl_Return_Button(15, 280, 100, 25, gettext("Apply"));
    o->callback((Fl_Callback*)cb_Xcas_Cas_setup_OK);
    o->align(FL_ALIGN_CLIP);
    }
    { Fl_Button* o = Xcas_cas_setup_OKSave = new Fl_Button(130, 280, 95, 25, gettext("Save"));
    o->shortcut(0xff1b);
    o->callback((Fl_Callback*)cb_Xcas_cas_setup_OKSave);
    o->align(FL_ALIGN_CLIP);
    }
    { Fl_Button* o = Xcas_Cancel_cas_setup = new Fl_Button(245, 280, 95, 25, gettext("Cancel"));
    o->shortcut(0xff1b);
    o->callback((Fl_Callback*)cb_Xcas_Cancel_cas_setup);
    o->align(FL_ALIGN_CLIP);
    }
    { Fl_Value_Input* o = Xcas_Threads = new Fl_Value_Input(130, 230, 40, 25, gettext("threads"));
    o->tooltip(gettext("Maximal number of threads in parallel"));
    o->maximum(2*giac::threads);
    o->minimum(1);
    o->step(1);
    o->value(giac::threads);
    o->align(68);
    }
    { Fl_Check_Button* o = Xcas_sqrt = new Fl_Check_Button(240, 245, 100, 25, gettext("Sqrt"));
    o->tooltip(gettext("Factor 2nd order poly using sqrt"));
    o->down_box(FL_DOWN_BOX);
    o->selection_color((Fl_Color)1);
    o->align(68|FL_ALIGN_INSIDE);
    }
    { Fl_Check_Button* o = Xcas_all_trig_sol = new Fl_Check_Button(240, 214, 100, 25, gettext("All_trig_sol"));
    o->tooltip(gettext("Solver: principal solution vs all solutions"));
    o->down_box(FL_DOWN_BOX);
    o->selection_color((Fl_Color)1);
    o->align(68|FL_ALIGN_INSIDE);
    }
    { Fl_Value_Input* o = Xcas_Set_Digits = new Fl_Value_Input(125, 75, 50, 25, gettext("Digits"));
    o->tooltip(gettext("Digits"));
    o->minimum(2);
    o->maximum(1000);
    o->step(1);
    o->value(14);
    o->align(72);
    }
    { Fl_Menu_Button* o = Xcas_Integer_style = new Fl_Menu_Button(245, 15, 95, 25, gettext("Integer basis"));
    o->align(FL_ALIGN_CLIP);
    o->menu(menu_Xcas_Integer_style);
    }
    { Fl_Output* o = Xcas_Integer_style_output = new Fl_Output(245, 40, 95, 25);
    o->tooltip(gettext("Current basis for printing integers"));
    o->labeltype(FL_NO_LABEL);
    o->align(68);
    }
    { Fl_Value_Input* o = Xcas_debug_infolevel = new Fl_Value_Input(45, 190, 55, 25, gettext("debug"));
    o->tooltip(gettext("Level of information displayed by giac functions"));
    o->maximum(100);
    o->step(1);
    o->callback((Fl_Callback*)cb_Xcas_debug_infolevel);
    o->align(68);
    }
    { Fl_Value_Input* o = Xcas_eval_level = new Fl_Value_Input(45, 75, 55, 25, gettext("eval"));
    o->tooltip(gettext("Number of embedded variable names replacement in interactive evaluation"));
    o->maximum(100);
    o->step(1);
    o->value(25);
    o->callback((Fl_Callback*)cb_Xcas_eval_level);
    o->align(68);
    }
    { Fl_Value_Input* o = Xcas_recursion_level = new Fl_Value_Input(45, 150, 55, 25, gettext("recurs"));
    o->tooltip(gettext("Number of embedded function calls allowed"));
    o->maximum(100);
    o->step(1);
    o->value(25);
    o->callback((Fl_Callback*)cb_Xcas_recursion_level);
    o->align(68);
    }
    { Fl_Value_Input* o = Xcas_prog_eval_level = new Fl_Value_Input(45, 110, 55, 25, gettext("prog"));
    o->tooltip(gettext("Number of embedded variable names replacement in program evaluation (1 or 0 f\
or default eval level)"));
    o->step(1);
    o->value(1);
    o->callback((Fl_Callback*)cb_Xcas_prog_eval_level);
    o->align(68);
    }
    { Fl_Value_Input* o = Xcas_Newton = new Fl_Value_Input(45, 230, 55, 25, gettext("maxiter"));
    o->tooltip(gettext("Default maximal number of iterations in Newton method"));
    o->maximum(100);
    o->minimum(10);
    o->step(1);
    o->value(20);
    o->align(68);
    }
    o->end();
    o->resizable(o);
  } // end Xcas_Cas_Setup creation


  void Xcas_load_cas_setup(GIAC_CONTEXT) {
    if (!Xcas_Cas_Setup)
      xcas_cas_setup_init();
    Xcas_Cas_setup_OK->user_data((void *)contextptr);
    Xcas_cas_setup_OKSave->user_data((void *)contextptr);
    Xcas_Epsilon->value(giac::epsilon(contextptr));
    Xcas_Proba_Epsilon->value(giac::proba_epsilon(contextptr));
    Xcas_Complex_mode->value(giac::complex_mode(contextptr));
    Xcas_Complex_variables->value(giac::complex_variables(contextptr));
    switch(do_maple_mode=giac::xcas_mode(contextptr)){
    case 0: Xcas_Style->value("xcas"); break;
    case 1: Xcas_Style->value("maple"); break;
    case 2: Xcas_Style->value("mupad"); break;
    case 3: Xcas_Style->value("ti"); break;
    }
    Xcas_Increasing_power->value(giac::increasing_power(contextptr));
    Xcas_Angle_radian->value(giac::angle_radian(contextptr));
    Xcas_Approx_mode->value(giac::approx_mode(contextptr));
    Xcas_Autosimplify->value(giac::autosimplify(contextptr).c_str());
    Xcas_sqrt->value(giac::withsqrt(contextptr));
    Xcas_all_trig_sol->value(giac::all_trig_sol(contextptr));
    Xcas_Set_Digits->value(giac::decimal_digits(contextptr));
    switch (do_scientific_format=giac::scientific_format(contextptr)){
    case 0: Xcas_Float_style_output->value("standard"); break;
    case 1: Xcas_Float_style_output->value("scientific"); break;
    case 2: Xcas_Float_style_output->value("engineer"); break;
    case 3: Xcas_Float_style_output->value("hexa"); break;
    }
    switch (do_integer_format=giac::integer_format(contextptr)){
    case 16: Xcas_Integer_style_output->value("16"); break;
    case 8: Xcas_Integer_style_output->value("8"); break;
    default: Xcas_Integer_style_output->value("10"); 
    }
    do_max_recursion_level=giac::MAX_RECURSION_LEVEL;
    Xcas_recursion_level->value(do_max_recursion_level);
    do_eval_level=giac::DEFAULT_EVAL_LEVEL=giac::eval_level(contextptr);
    Xcas_eval_level->value(do_eval_level);
    Xcas_prog_eval_level->value(giac::prog_eval_level_val(contextptr));
    do_debug_infolevel=giac::debug_infolevel;
    Xcas_debug_infolevel->value(do_debug_infolevel);
    Fl_Widget * foc = Fl::focus();
    if (foc && (foc=Fl::focus()->window()) ){
      xcas::change_group_fontsize(Xcas_Cas_Setup,foc->labelsize());
      Xcas_Cas_Setup->resize(20,80,3*foc->w()/4,3*foc->h()/4);
    }
    Xcas_Cas_Setup->show();
  }


  // xcas geo setup
  Fl_Double_Window *Xcas_Plot_Setup=(Fl_Double_Window *)0;

  Fl_Group *Plot_setup_w=(Fl_Group *)0;

  Fl_Value_Input *Xcas_Xmin=(Fl_Value_Input *)0;

  Fl_Value_Input *Xcas_Xmax=(Fl_Value_Input *)0;

  Fl_Value_Input *Xcas_Ymin=(Fl_Value_Input *)0;

  Fl_Value_Input *Xcas_Ymax=(Fl_Value_Input *)0;

  Fl_Value_Input *Xcas_Zmin=(Fl_Value_Input *)0;

  Fl_Value_Input *Xcas_Zmax=(Fl_Value_Input *)0;

  Fl_Value_Input *Xcas_Tmin=(Fl_Value_Input *)0;

  Fl_Value_Input *Xcas_Tmax=(Fl_Value_Input *)0;

  Fl_Value_Input *Xcas_Class_min=(Fl_Value_Input *)0;

  Fl_Value_Input *Xcas_Class_size=(Fl_Value_Input *)0;

  Fl_Value_Input *Xcas_X_tick=(Fl_Value_Input *)0;

  Fl_Value_Input *Xcas_Y_tick=(Fl_Value_Input *)0;

  Fl_Check_Button *Xcas_autoscale=(Fl_Check_Button *)0;

  Fl_Return_Button *Xcas_Plot_Setup_OK=(Fl_Return_Button *)0;

  Fl_Button *Xcas_Plot_Setup_OKSave=(Fl_Button *)0;

  Fl_Button *Xcas_Plot_Setup_Cancel=(Fl_Button *)0;
  
  Fl_Value_Input *Xcas_WXmin=(Fl_Value_Input *)0;
  
  Fl_Value_Input *Xcas_WXmax=(Fl_Value_Input *)0;
  
  Fl_Value_Input *Xcas_WYmin=(Fl_Value_Input *)0;
  
  Fl_Value_Input *Xcas_WYmax=(Fl_Value_Input *)0;
  
  Fl_Check_Button *Xcas_orthonormal=(Fl_Check_Button *)0;
  
  Fl_Button *Xcas_To_W=(Fl_Button *)0;
  
  Fl_Button *Xcas_From_W=(Fl_Button *)0;
  
  static void cb_Xcas_Plot_Setup_OK(Fl_Return_Button*, void*) {
    //
    if (Xcas_Y_tick->value()>=0)
      giac::y_tick=Xcas_Y_tick->value();
    else
      giac::y_tick=1.0;
    if (Xcas_X_tick->value()>=0)
      giac::x_tick=Xcas_X_tick->value();
    else
      giac::x_tick=1.0;
    giac::gnuplot_xmin=Xcas_Xmin->value();
    giac::gnuplot_xmax=Xcas_Xmax->value();
    giac::gnuplot_ymin=Xcas_Ymin->value();
    giac::gnuplot_ymax=Xcas_Ymax->value();
    giac::gnuplot_zmin=Xcas_Zmin->value();
    giac::gnuplot_zmax=Xcas_Zmax->value();
    giac::gnuplot_tmin=Xcas_Tmin->value();
    giac::gnuplot_tmax=Xcas_Tmax->value();
    xcas::Xcas_config.window_xmin=Xcas_WXmin->value();
    xcas::Xcas_config.window_xmax=Xcas_WXmax->value();
    xcas::Xcas_config.window_ymin=Xcas_WYmin->value();
    xcas::Xcas_config.window_ymax=Xcas_WYmax->value();
    xcas::Xcas_config.ortho=Xcas_orthonormal->value();
    xcas::Xcas_config.autoscale=Xcas_autoscale->value();
    giac::class_minimum=Xcas_Class_min->value();
    giac::class_size=Xcas_Class_size->value();
    Xcas_Plot_Setup_OK->window()->hide();
    if (Xcas_input_focus)
      Fl::focus(Xcas_input_focus);
  }

  static void cb_Xcas_Plot_Setup_OKSave(Fl_Button*, void* userdata) {
    cb_Xcas_Plot_Setup_OK(0,userdata); 
    const giac::context * contextptr = giac::context0;
    if (userdata)
      contextptr=(const giac::context *) userdata;
    if (Xcas_save_config_ptr)
      Xcas_save_config_ptr(contextptr);
  }
  
  static void cb_Xcas_Plot_Setup_Cancel(Fl_Button*, void*) {
    Xcas_Plot_Setup_Cancel->window()->hide();
    if (Xcas_input_focus)
      Fl::focus(Xcas_input_focus);
  }
  
  static void cb_Xcas_To_W(Fl_Button*, void*) {
    Xcas_WXmax->value(Xcas_Xmax->value()); Xcas_WXmin->value(Xcas_Xmin->value());Xcas_WYmax->value(Xcas_Ymax->value()); Xcas_WYmin->value(Xcas_Ymin->value());
  }
  
  static void cb_Xcas_From_W(Fl_Button*, void*) {
    Xcas_Xmax->value(Xcas_WXmax->value()); Xcas_Xmin->value(Xcas_WXmin->value());Xcas_Ymax->value(Xcas_WYmax->value()); Xcas_Ymin->value(Xcas_WYmin->value());
  }

  void xcas_plot_setup_init()
  { 
    Fl_Group::current(0);
    Fl_Double_Window* o = Xcas_Plot_Setup = new Fl_Double_Window(20,80,300, 230, gettext("Xcas Plot Setup"));
    { Fl_Group* o = Plot_setup_w = new Fl_Group(0, 0, 300, 320, gettext("Plot setup"));
    o->box(FL_SHADOW_BOX);
    o->color(FL_BACKGROUND2_COLOR);
    o->labeltype(FL_NO_LABEL);
    o->labelsize(10);
    { Fl_Value_Input* o = Xcas_Xmin = new Fl_Value_Input(50, 8, 45, 22, gettext("X-"));
    o->tooltip(gettext("Xmin for computation"));
    o->minimum(-1e+99);
    o->maximum(1e+99);
    o->step(0.1);
    o->value(-6.5);
    o->textsize(12);
    o->align(68);
    }
    { Fl_Value_Input* o = Xcas_Xmax = new Fl_Value_Input(150, 8, 45, 22, gettext("X+"));
    o->tooltip(gettext("Xmax for computation"));
    o->minimum(-1e+99);
    o->maximum(1e+99);
    o->step(0.1);
    o->value(6.5);
    o->textsize(12);
    o->align(68);
    }
    { Fl_Value_Input* o = Xcas_Ymin = new Fl_Value_Input(50, 35, 45, 20, gettext("Y-"));
    o->tooltip(gettext("Ymin for computation"));
    o->minimum(-1e+99);
    o->maximum(1e+99);
    o->step(0.1);
    o->value(-3.1);
    o->textsize(12);
    o->align(68);
    }
    { Fl_Value_Input* o = Xcas_Ymax = new Fl_Value_Input(150, 35, 45, 20, gettext("Y+"));
    o->tooltip(gettext("Ymax for computation"));
    o->minimum(-1e+99);
    o->maximum(1e+99);
    o->step(0.1);
    o->value(3.1);
    o->textsize(12);
    o->align(68);
    }
    { Fl_Value_Input* o = Xcas_Zmin = new Fl_Value_Input(50, 60, 45, 20, gettext("Z-"));
    o->tooltip(gettext("Zmin (3-d)"));
    o->minimum(-1e+99);
    o->maximum(1e+99);
    o->step(0.1);
    o->value(-10);
    o->textsize(12);
    o->align(68);
    }
    { Fl_Value_Input* o = Xcas_Zmax = new Fl_Value_Input(150, 60, 45, 20, gettext("Z+"));
    o->tooltip(gettext("Z-max (3-d)"));
    o->minimum(-1e+99);
    o->maximum(1e+99);
    o->step(0.1);
    o->value(10);
    o->textsize(12);
    o->align(68);
    }
    { Fl_Value_Input* o = Xcas_Tmin = new Fl_Value_Input(50, 85, 45, 20, gettext("t-"));
    o->tooltip(gettext("T-min (parametric computation)"));
    o->minimum(-1e+99);
    o->maximum(1e+99);
    o->step(0.1);
    o->value(-10);
    o->textsize(12);
    o->align(68);
    }
    { Fl_Value_Input* o = Xcas_Tmax = new Fl_Value_Input(150, 85, 45, 20, gettext("t+"));
    o->tooltip(gettext("T-max (parametric computation)"));
    o->minimum(-1e+99);
    o->maximum(1e+99);
    o->step(0.1);
    o->value(-10);
    o->textsize(12);
    o->align(68);
    }
    { Fl_Value_Input* o = Xcas_Class_min = new Fl_Value_Input(105, 160, 45, 20, gettext("class_min"));
    o->tooltip(gettext("Minimum for classes (for e.g. histogram...)"));
    o->minimum(1e-99);
    o->maximum(1e+99);
    o->value(1);
    o->textsize(12);
    o->align(68);
    }
    { Fl_Value_Input* o = Xcas_Class_size = new Fl_Value_Input(245, 160, 45, 20, gettext("class_size"));
    o->tooltip(gettext("Size of a class (e.g. for histogram...)"));
    o->minimum(1e-99);
    o->maximum(1e+99);
    o->value(1);
    o->textsize(12);
    o->align(68);
    }
    { Fl_Value_Input* o = Xcas_X_tick = new Fl_Value_Input(235, 10, 40, 20, gettext("TX"));
    o->tooltip(gettext("Tick range on X axis"));
    o->maximum(1e+99);
    o->step(0.1);
    o->value(1);
    o->textsize(12);
    o->align(68);
    }
    { Fl_Value_Input* o = Xcas_Y_tick = new Fl_Value_Input(235, 35, 40, 20, gettext("TY"));
    o->tooltip(gettext("Tick range on Y axis"));
    o->maximum(1e+99);
    o->step(0.1);
    o->value(1);
    o->textsize(12);
    o->align(68);
    }
    { Fl_Check_Button* o = Xcas_autoscale = new Fl_Check_Button(210, 60, 75, 25, gettext("autoscale"));
    o->tooltip(gettext("Autoscale graphs"));
    o->down_box(FL_DOWN_BOX);
    o->value(1);
    o->selection_color(FL_FOREGROUND_COLOR);
    o->align(68|FL_ALIGN_INSIDE);
    }
    { Fl_Return_Button* o = Xcas_Plot_Setup_OK = new Fl_Return_Button(10, 195, 90, 25, gettext("Apply"));
    o->callback((Fl_Callback*)cb_Xcas_Plot_Setup_OK);
    o->align(FL_ALIGN_CLIP);
    }
    { Fl_Button* o = Xcas_Plot_Setup_OKSave = new Fl_Button(105, 195, 90, 25, gettext("Save"));
    o->callback((Fl_Callback*)cb_Xcas_Plot_Setup_OKSave);
    o->align(FL_ALIGN_CLIP);
    }
    { Fl_Button* o = Xcas_Plot_Setup_Cancel = new Fl_Button(200, 195, 90, 25, gettext("Cancel"));
    o->callback((Fl_Callback*)cb_Xcas_Plot_Setup_Cancel);
    o->align(FL_ALIGN_CLIP);
    }
    { Fl_Value_Input* o = Xcas_WXmin = new Fl_Value_Input(50, 108, 45, 22, gettext("WX-"));
    o->tooltip(gettext("Xmin for visualisation"));
    o->minimum(-1e+99);
    o->maximum(1e+99);
    o->step(0.1);
    o->value(-6.5);
    o->textsize(12);
    o->align(68);
    }
    { Fl_Value_Input* o = Xcas_WXmax = new Fl_Value_Input(150, 108, 45, 22, gettext("WX+"));
    o->tooltip(gettext("Xmax for visualization"));
    o->minimum(-1e+99);
    o->maximum(1e+99);
    o->step(0.1);
    o->value(6.5);
    o->textsize(12);
    o->align(68);
    }
    { Fl_Value_Input* o = Xcas_WYmin = new Fl_Value_Input(50, 135, 45, 20, gettext("WY-"));
    o->tooltip(gettext("Ymin for visualization"));
    o->minimum(-1e+99);
    o->maximum(1e+99);
    o->step(0.1);
    o->value(-3.1);
    o->textsize(12);
    o->align(68);
    }
    { Fl_Value_Input* o = Xcas_WYmax = new Fl_Value_Input(150, 135, 45, 20, gettext("WY+"));
    o->tooltip(gettext("Ymax for visualization"));
    o->minimum(-1e+99);
    o->maximum(1e+99);
    o->step(0.1);
    o->value(3.1);
    o->textsize(12);
    o->align(68);
    }
    { Fl_Check_Button* o = Xcas_orthonormal = new Fl_Check_Button(210, 135, 80, 20, gettext("ortho"));
    o->tooltip(gettext("Orthonormalize"));
    o->down_box(FL_DOWN_BOX);
    o->align(68|FL_ALIGN_INSIDE);
    }
    { Fl_Button* o = Xcas_To_W = new Fl_Button(205, 110, 30, 20, gettext(">W"));
    o->tooltip(gettext("Copy XYminmax to W"));
    o->callback((Fl_Callback*)cb_Xcas_To_W);
    o->align(FL_ALIGN_CLIP);
    }
    { Fl_Button* o = Xcas_From_W = new Fl_Button(250, 110, 30, 20, gettext("W>"));
    o->tooltip(gettext("Copy W to XYminmax"));
    o->callback((Fl_Callback*)cb_Xcas_From_W);
    o->align(FL_ALIGN_CLIP);
    }
    o->end();
    Fl_Group::current()->resizable(o);
    }
    o->end();
  }

  void Xcas_load_graph_setup(GIAC_CONTEXT) {
    if (!Xcas_Plot_Setup)
      xcas_plot_setup_init();
    Xcas_Plot_Setup_OK->user_data((void *)contextptr);
    Xcas_Plot_Setup_OKSave->user_data((void *)contextptr);
    Xcas_Y_tick->value(giac::y_tick);
    Xcas_X_tick->value(giac::x_tick);
    Xcas_Xmin->value(giac::gnuplot_xmin);
    Xcas_Xmax->value(giac::gnuplot_xmax);
    Xcas_Ymin->value(giac::gnuplot_ymin);
    Xcas_Ymax->value(giac::gnuplot_ymax);
    Xcas_Zmin->value(giac::gnuplot_zmin);
    Xcas_Zmax->value(giac::gnuplot_zmax);
    Xcas_Tmin->value(giac::gnuplot_tmin);
    Xcas_Tmax->value(giac::gnuplot_tmax);
    Xcas_WXmin->value(xcas::Xcas_config.window_xmin);
    Xcas_WXmax->value(xcas::Xcas_config.window_xmax);
    Xcas_WYmin->value(xcas::Xcas_config.window_ymin);
    Xcas_WYmax->value(xcas::Xcas_config.window_ymax);
    Xcas_Class_min->value(giac::class_minimum);
    Xcas_Class_size->value(giac::class_size);
    Fl_Widget * foc = Fl::focus();
    if (foc && (foc=foc->window())){
      xcas::change_group_fontsize(Xcas_Plot_Setup,foc->labelsize());
      Xcas_Plot_Setup->resize(20,80,3*foc->w()/4,3*foc->h()/4);
    }
    Xcas_Plot_Setup->show();
  }

#endif // HAVE_LIBFLTK

#ifndef NO_NAMESPACE_XCAS
} // namespace xcas
#endif // ndef NO_NAMESPACE_XCAS
