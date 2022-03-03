// -*- mode:C++ ; compile-command: "g++ -DHAVE_CONFIG_H -I. -I.. -I../include -I../../giac/include -g -c Editeur.cc" -*-
#include "Editeur.h"
#include "Input.h"
#include "Tableur.h"
/*
 *  Copyright (C) 2000,2014 B. Parisse, Institut Fourier, 38402 St Martin d'Heres
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
#include <FL/fl_ask.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Tooltip.H>
#include <fstream>
#include "vector.h"
#include <algorithm>
#include <fcntl.h>
#include <cmath>
#include <time.h> // for nanosleep
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h> // auto-recovery function
#include "History.h"
#include "Print.h"
#include "Equation.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

using namespace std;
using namespace giac;

#ifndef NO_NAMESPACE_XCAS
namespace xcas {
#endif // ndef NO_NAMESPACE_XCAS

  int alt_ctrl=0;
  void (*alt_ctrl_cb)(int)=0;
  
  std::vector<std::string> Xcas_Text_Editor::history;
  int Xcas_Text_Editor::count=0;

  // Highlighting, borrowed from FLTK1.2/test/editor.cxx
  // Syntax highlighting stuff...

  Fl_Text_Display::Style_Table_Entry styletable_init[] = {	// Style table
    { FL_BLACK,      FL_COURIER,        14 }, // A - Plain
    { FL_DARK_GREEN, FL_COURIER_ITALIC, 14 }, // B - Line comments
    { FL_DARK_GREEN, FL_COURIER_ITALIC, 14 }, // C - Block comments
    { FL_CYAN,       FL_COURIER,        14 }, // D - Strings
    { FL_DARK_RED,   FL_COURIER,        14 }, // E - Directives
    { FL_DARK_RED,   FL_COURIER_BOLD,   14 }, // F - Types
    { FL_BLUE,       FL_COURIER_BOLD,   14 }  // G - Keywords
  };
  int styletable_n=sizeof(styletable_init) / sizeof(styletable_init[0]);
  const char         *code_keywords[] = {   // List of known giac keywords...
    "BEGIN",
    "BREAK",
    "CATCH",
    "CONTINUE",
    "Cycle",
    "DO",
    "Dialog",
    "ELIF",
    "ELSE",
    "END",
    "Else",
    "ElseIf",
    "EndDlog",
    "EndFor",
    "EndFunc",
    "EndIf",
    "EndLoop",
    "EndPrgm",
    "EndTry",
    "EndWhile",
    "Exit",
    "FOR",
    "FROM",
    "For",
    "Func",
    "Goto",
    "IF",
    "If",
    "LOCAL",
    "Lbl",
    "Local"
    "Loop",
    "Prgm",
    "REPEAT",
    "Return",
    "STEP",
    "THEN",
    "TO",
    "TRY",
    "Then",
    "Try",
    "WHILE",
    "While",
    "alors",
    "and",
    "break",
    "by",
    "case",
    "catch",
    "continue",
    "de",
    "default",
    "do",
    "downto",
    "elif",
    "else",
    "end",
    "end_case",
    "end_for",
    "end_if",
    "end_proc",
    "end_while",
    "et",
    "faire",
    "false",
    "ffaire",
    "ffonction",
    "fi",
    "fonction",
    "for",
    "fpour",
    "from",
    "fsi",
    "ftantque",
    "goto",
    "if",
    "jusqu_a",
    "jusqua",
    "jusque",
    "label",
    "local",
    "non",
    "not",
    "od",
    "operator",
    "or",
    "ou",
    "pas",
    "pour",
    "proc",
    "repeat",
    "repeter",
    "retourne",
    "return",
    "si",
    "sinon",
    "step",
    "switch",
    "tantque",
    "then",
    "throw",
    "to",
    "true",
    "try",
    "until",
    "while",
    "xor"
  };
  
//
// 'compare_keywords()' - Compare two keywords...
//

  int
  compare_keywords(const void *a,
		   const void *b) {
    return (strcmp(*((const char **)a), *((const char **)b)));
}
  
  bool alpha_order(const string & S1,const string & S2){
    string s1 =S1;
    string s2 =S2;
    for (unsigned i=0;i<s1.size();++i) 
      s1[i]=tolower(s1[i]);
    for (unsigned i=0;i<s2.size();++i) 
      s2[i]=tolower(s2[i]);
    if (s1!=s2)
      return s1<s2;
    return S1< S2;
  }
  
//
// 'style_parse()' - Parse text and produce style data.
//

  void
  style_parse(const char *text,
	      char       *style,
	      int        length) {
    char	     current;
    int	     col;
    int	     last;
    char	     buf[255],
      *bufptr;
    const char *temp;
    
    for (current = *style, col = 0, last = 0; length > 0; length --, text ++) {
      if (current == 'B' || current>='E') current = 'A';
      if (current == 'A') {
	// Check for directives, comments, strings, and keywords...
	if (col == 0 && *text == '#') {
	  // Set style to directive
	  current = 'E';
	} else if (strncmp(text, "//", 2) == 0) {
	  current = 'B';
	  for (; length > 0 && *text != '\n'; length --, text ++) *style++ = 'B';
	  
	  if (length == 0) break;
	} else if (strncmp(text, "/*", 2) == 0) {
	  current = 'C';
	} else if (strncmp(text, "\\\"", 2) == 0) {
	  // Quoted quote...
	  *style++ = current;
	  *style++ = current;
	  text ++;
	  length --;
	  col += 2;
	  continue;
	} else if (*text == '\"') {
	  current = 'D';
	} else if (!last && isalphan(*text)) {
	  // Might be a keyword...
	  for (temp = text, bufptr = buf;
	       isalphan(*temp) && bufptr < (buf + sizeof(buf) - 1);
	       *bufptr++ = *temp++);
	  
	  if (!isalphan(*temp)) {
	    *bufptr = '\0';
	    
	    bufptr = buf;
	    
	    if (bsearch(&bufptr, code_keywords,
			       sizeof(code_keywords) / sizeof(code_keywords[0]),
			       sizeof(code_keywords[0]), compare_keywords)) {
	      current='A';
	      while (text < temp) {
		*style++ = 'G';
		text ++;
		length --;
		col ++;
	      }
	      
	      text --;
	      length ++;
	      last = 1;
	      continue;
	    } else if (giac::vector_completions_ptr() && binary_search(giac::vector_completions_ptr()->begin(),giac::vector_completions_ptr()->end(),bufptr,alpha_order)) {
	      current='A';
	      while (text < temp) {
		*style++ = 'F';
		text ++;
		length --;
		col ++;
	      }
	      
	      text --;
	      length ++;
	      last = 1;
	      continue;
	    }
	  }
	}
      } else if (current == 'C' && strncmp(text, "*/", 2) == 0) {
	// Close a C comment...
	*style++ = current;
	*style++ = current;
	text ++;
	length --;
	current = 'A';
	col += 2;
	continue;
      } else if (current == 'D') {
	// Continuing in string...
	if (strncmp(text, "\\\"", 2) == 0) {
	  // Quoted end quote...
	  *style++ = current;
	  *style++ = current;
	  text ++;
	  length --;
	  col += 2;
	  continue;
	} else if (*text == '\"') {
	  // End quote...
	  *style++ = current;
	  col ++;
	  current = 'A';
	  continue;
	}
      } else if ( (current=='F' || current=='G') && !isalphan(*text)){
	current='A';
      }
      // Copy style info...
      if (current == 'A' && (*text == '{' || *text == '}')) *style++ = 'G';
      else *style++ = current;
      col ++;
      
      last = isalphan(*text) || *text == '.';
      
      if (*text == '\n') {
	// Reset column and possibly reset the style
	col = 0;
	if (current == 'B' || current >= 'E') current = 'A';
      }
    }
  }
  

//
// 'style_update()' - Update the style buffer...
//

  void
  style_update(int        pos,		// I - Position of update
	       int        nInserted,	// I - Number of inserted chars
	       int        nDeleted,	// I - Number of deleted chars
	       int        /*nRestyled*/,	// I - Number of restyled chars
	       const char * /*deletedText*/,// I - Text that was deleted
	       void       *cbArg
	       ) {	// I - Callback data
    int	start,				// Start of text
      end;				// End of text
    char	last,				// Last style on line
      *style,				// Style data
      *text;				// Text data
    
    Fl_Text_Buffer * stylebuf= ((Xcas_Text_Editor *) cbArg)->stylebuf;
    // If this is just a selection change, just unselect the style buffer...
    if (nInserted == 0 && nDeleted == 0) {
      stylebuf->unselect();
      return;
    }
    
    // Track changes in the text buffer...
    if (nInserted > 0) {
      // Insert characters into the style buffer...
      style = new char[nInserted + 1];
      memset(style, 'A', nInserted);
      style[nInserted] = '\0';
      
      stylebuf->replace(pos, pos + nDeleted, style);
      delete[] style;
    } else {
      // Just delete characters in the style buffer...
      stylebuf->remove(pos, pos + nDeleted);
    }
    
    // Select the area that was just updated to avoid unnecessary
    // callbacks...
    stylebuf->select(pos, pos + nInserted - nDeleted);
    
    // Re-parse the changed region; we do this by parsing from the
    // beginning of the line of the changed region to the end of
    // the line of the changed region...  Then we check the last
    // style character and keep updating if we have a multi-line
    // comment character...
    Fl_Text_Buffer * textbuf=((Fl_Text_Editor *)cbArg)->buffer();
    start = textbuf->line_start(pos);
    end   = textbuf->line_end(pos + nInserted);
    text  = textbuf->text_range(start, end);
    style = stylebuf->text_range(start, end);
    last  = start==end?0:style[end - start - 1];
    
    //  printf("start = %d, end = %d, text = \"%s\", style = \"%s\"...\n",
    //         start, end, text, style);
    
    style_parse(text, style, end - start);
    
    //  printf("new style = \"%s\"...\n", style);
    
    stylebuf->replace(start, end, style);
    ((Fl_Text_Editor *)cbArg)->redisplay_range(start, end);
    
    if (start==end || last != style[end - start - 1]) {
      // The last character on the line changed styles, so reparse the
      // remainder of the buffer...
      free(text);
      free(style);
      
      end   = textbuf->length();
      text  = textbuf->text_range(start, end);
      style = stylebuf->text_range(start, end);
      
      style_parse(text, style, end - start);
      
      stylebuf->replace(start, end, style);
      ((Fl_Text_Editor *)cbArg)->redisplay_range(start, end);
    }
    
    free(text);
    free(style);
  }

  //
  // 'style_unfinished_cb()' - Update unfinished styles.
  //
  
  void
  style_unfinished_cb(int, void*) {
  }


  //
  // 'style_init()' - Initialize the style buffer...
  //
  
  void  style_init(Xcas_Text_Editor * textbuf  ) {
  }

  bool editor_changed=false;

  void Editor_changed_cb(int, int nInserted, int nDeleted,int, const char*, void* v) {
    if ((nInserted || nDeleted)) 
      editor_changed = true;
  }

  Fl_Text_Editor * do_find_editor(Fl_Widget * widget){
    if (!widget)
      return 0;
    Fl_Group * gr = widget->parent();
    if (!gr)
      return 0;
    Editeur * e=dynamic_cast<Editeur *>(gr);
    if (e)
      return e->editor;
    else
      return 0;
  }

  Fl_Text_Editor * find_editor(Fl_Widget * widget){
    Fl_Text_Editor * ed=do_find_editor(widget);
    if (ed)
      return ed;
    ed=do_find_editor(Fl::focus());
    if (!ed)
      fl_alert("%s",gettext("No program editor found. Please click in or add one"));
    return ed;
  }

  static void cb_Editeur(Fl_Menu_* m , void*) {
    Fl_Text_Editor * e = find_editor(m);
    if (e){
    }
  }

  static void cb_Editeur_Save(Fl_Widget * m , void*);

  std::string editeur_load(Fl_Text_Editor * e){
    if (e){
      char * newfile = load_file_chooser("Insert program", ("*."+dynamic_cast<Editeur *>(e->parent())->extension).c_str(), "",0,false);
      if ( file_not_available(newfile) )
	return "";
      e->buffer()->insertfile(newfile,e->insert_position());
      return newfile;
    }
    return "";
  }

  static void cb_Editeur_Load(Fl_Widget * m , void*) {
    Fl_Text_Editor * e = find_editor(m);
    if (e){
      if (e->changed()){
	int i=fl_ask("%s","Buffer changed. Save?");
	if (i)
	  cb_Editeur_Save(m,0);
      }
      char * newfile = load_file_chooser("Insert program", ("*."+dynamic_cast<Editeur *>(e->parent())->extension).c_str(), "",0,false);
      if ( file_not_available(newfile) )
	return;
      e->buffer()->loadfile(newfile);
      e->label(newfile);
      if (Editeur * ed=dynamic_cast<Editeur *>(m->parent())){
	ed->output->value(remove_path(e->label()).c_str());
	ed->output->redraw();
      }
    }
  }

  static void cb_Editeur_Insert(Fl_Menu_* m , void*) {
    Fl_Text_Editor * e = find_editor(m);
    if (e)
      editeur_load(e);
  }

  void editeur_insert(Fl_Text_Editor * e,const std::string & newfile,int mode){
    ifstream in(newfile.c_str());
    char ch;
    string tmp;
    const context * contextptr = get_context(e);
    while (in){
      in.get(ch);
      if (in.eof())
	break;
      tmp += ch;
    }
    if (mode<0 || mode==xcas_mode(contextptr)){
      e->insert(tmp.c_str());
      return;
    }
    int save_maple_mode=xcas_mode(contextptr);
    xcas_mode(contextptr)=mode;
    gen g;
    try {
      g=gen(tmp,contextptr);
    }
    catch (std::runtime_error & err){
      cerr << err.what() << endl;
    }
    xcas_mode(contextptr)=save_maple_mode;
    if (g.type==_VECT && g.subtype==_SEQ__VECT && xcas_mode(contextptr) !=3 ){
      const_iterateur it=g._VECTptr->begin(),itend=g._VECTptr->end();
      for (;it!=itend;++it){
	e->insert(it->print(contextptr).c_str());
	e->insert(";\n");
      }
    }
    else
      e->insert(g.print(contextptr).c_str());
  }

  void editeur_translate(Fl_Text_Editor * e,int mode){
    if (!Xcas_help_output)
      return;
    static string res;
    gen g;
    context * contextptr = get_context(e);
    try {
      char * ch=e->buffer()->text(); 
      string res(ch); 
      free(ch);
      g=gen(res,contextptr);
    }
    catch (std::runtime_error & err){
      cerr << err.what() << endl;
      return;
    }
    int save_maple_mode=xcas_mode(contextptr);
    xcas_mode(contextptr)=mode;
    res="";
    if (g.type==_VECT && g.subtype==_SEQ__VECT && xcas_mode(contextptr) !=3 ){
      const_iterateur it=g._VECTptr->begin(),itend=g._VECTptr->end();
      for (;it!=itend;++it){
	res +=  it->print_universal(contextptr) ;
	res += ";\n";
      }
    }
    else
      res = g.print_universal(contextptr) + ((xcas_mode(contextptr)==3)?"\n":";\n");
    xcas_mode(contextptr)=save_maple_mode;
    Xcas_help_output->value(res.c_str());
    Xcas_help_output->position(0,res.size());
    // Xcas_help_output->copy();
    Fl::copy(Xcas_help_output->Fl_Output::value(),res.size(),1);
    Fl::selection(*Xcas_help_output,Xcas_help_output->Fl_Output::value(),res.size());
  }

  void editeur_export(Fl_Text_Editor * e,const std::string & newfile,int mode){
    context * contextptr = get_context(e);
    gen g;
    try {
      char * ch=e->buffer()->text(); 
      string res(ch); 
      free(ch);
      g=gen(res,contextptr);
    }
    catch (std::runtime_error & err){
      cerr << err.what() << endl;
      return;
    }
    if (is_file_available(newfile.c_str())){
      int i=fl_ask("%s",gettext("File exists. Overwrite?"));
      if (!i)
	return;
    }
    int save_maple_mode=xcas_mode(contextptr);
    xcas_mode(contextptr)=mode;
    ofstream of(newfile.c_str());
    if (g.type==_VECT && g.subtype==_SEQ__VECT && xcas_mode(contextptr) !=3 ){
      const_iterateur it=g._VECTptr->begin(),itend=g._VECTptr->end();
      for (;it!=itend;++it){
	of << it->print_universal(contextptr) ;
	of << ";\n";
      }
    }
    else
      of << g.print_universal(contextptr) << ((xcas_mode(contextptr)==3)?"\n":";\n");
    xcas_mode(contextptr)=save_maple_mode;
  }

  void cb_editeur_insert(Fl_Menu_ * m,const string & extension,int mode){
    Fl_Text_Editor * e = find_editor(m);
    if (e){
      char * newfile = load_file_chooser("Insert program",("*"+extension).c_str(), ("session"+extension).c_str(),0,false);
      if ( file_not_available(newfile) )
	return;
      editeur_insert(e,newfile,mode);
    }
  }

  static void cb_Editeur_Insert_File(Fl_Menu_* m , void*) {
    cb_editeur_insert(m,"",-1);
  }

  static void cb_Editeur_Insert_Xcas(Fl_Menu_* m , void*) {
    cb_editeur_insert(m,".cxx",0);
  }

  static void cb_Editeur_Insert_Maple(Fl_Menu_* m , void*) {
    cb_editeur_insert(m,".map",1);
  }

  static void cb_Editeur_Insert_Mupad(Fl_Menu_* m , void*) {
    cb_editeur_insert(m,".mu",2);
  }

  static void cb_Editeur_Insert_Ti(Fl_Menu_* m , void*) {
    cb_editeur_insert(m,".ti",3);
  }

  static void cb_Editeur_Export_Maple(Fl_Menu_* m , void*) {
    Fl_Text_Editor * e = find_editor(m);
    if (e){
      char * newfile = file_chooser("Export program", "*.map", "session.map");
      editeur_export(e,newfile,1);
    }
  }

  static void cb_Editeur_Export_Xcas(Fl_Menu_* m , void*) {
    Fl_Text_Editor * e = find_editor(m);
    if (e){
      char * newfile = file_chooser("Export program", "*.cxx", "session.cxx");
      editeur_export(e,newfile,0);
    }
  }

  static void cb_Editeur_Export_Mupad(Fl_Menu_* m , void*) {
    Fl_Text_Editor * e = find_editor(m);
    if (e){
      char * newfile = file_chooser("Export program", "*.mu", "session.mu");
      editeur_export(e,newfile,2);
    }
  }

  static void cb_Editeur_Export_Ti(Fl_Menu_* m , void*) {
    Fl_Text_Editor * e = find_editor(m);
    if (e){
      char * newfile = file_chooser("Export program", "*.ti", "session.ti");
      editeur_export(e,newfile,3);
    }
  }

  static void cb_Editeur_Translate_Maple(Fl_Menu_* m , void*) {
    Fl_Text_Editor * e = find_editor(m);
    if (e){
      editeur_translate(e,1);
    }
  }

  static void cb_Editeur_Translate_Xcas(Fl_Menu_* m , void*) {
    Fl_Text_Editor * e = find_editor(m);
    if (e){
      editeur_translate(e,0);
    }
  }

  static void cb_Editeur_Translate_Mupad(Fl_Menu_* m , void*) {
    Fl_Text_Editor * e = find_editor(m);
    if (e){
      editeur_translate(e,2);
    }
  }

  static void cb_Editeur_Translate_Ti(Fl_Menu_* m , void*) {
    Fl_Text_Editor * e = find_editor(m);
    if (e){
      editeur_translate(e,3);
    }
  }

  static void cb_Editeur_Save_as(Fl_Widget * m , void*) {
    static int program_counter=0;
    Fl_Text_Editor * e = find_editor(m);
    if (e){
      string tmp,extension;
      if (Editeur * ed = dynamic_cast<Editeur *>(m->parent()))
	extension=ed->extension;
      for (;;){
	char * newfile ;
	if (extension.empty())
	  newfile = file_chooser("Store program", "*", ("session"+print_INT_(program_counter)).c_str());
	else
	  newfile = file_chooser("Store program", ("*."+extension).c_str(), ("session"+print_INT_(program_counter)+"."+extension).c_str());
	if ( (!newfile) || (!*newfile))
	  return;
	tmp=newfile;
	if (!extension.empty())
	  tmp=remove_extension(tmp.substr(0,1000).c_str())+"."+extension;
	if (access(tmp.c_str(),R_OK))
	  break;
	int i=fl_ask("%s",(tmp+gettext(": file exists. Overwrite?")).c_str());
	if (i==1)
	  break;
      }
      e->label(tmp.c_str());
      if (Editeur * ed=dynamic_cast<Editeur *>(m->parent())){
	ed->output->value(remove_path(e->label()).c_str());
	ed->output->redraw();
      }
      cb_Editeur_Save(m,0);
    }
  }

  static void cb_Editeur_Save(Fl_Widget * m , void*) {
    Fl_Text_Editor * e = find_editor(m);
    if (e // && e->changed()
	){
      if (e->label() && e->label()[0]){
	e->buffer()->savefile(e->label());
	e->clear_changed();
      }
      else 
	cb_Editeur_Save_as(m,0);
    }
  }

  Fl_Widget * Save_Focus_Button::widget=0;
  int Save_Focus_Button::handle(int event){
    if (event==FL_FOCUS)
      return 0;
    if (event==FL_MOVE){ // search if focus is one of the parent of the button
      Fl_Widget * w1 =Fl::focus();
      Fl_Widget * w2 = this;
      while (w2){
	if (w1==w2)
	  break;
	w2=w2->parent();
      }
      if (w1 && !w2 && widget!=w1){ 
	// it's not a parent, save it
	widget=w1;
      }
    }
    return Fl_Button::handle(event);
  }

  int No_Focus_Button::handle(int event){
    switch (event) {
    case FL_ENTER:
    case FL_LEAVE:
      return 1;
    case FL_RELEASE:
      do_callback();
    case FL_PUSH: case FL_DRAG:
      return 1;
    case FL_SHORTCUT:
      if (!(shortcut() ?
	    Fl::test_shortcut(shortcut()) : test_shortcut())) return 0;      
      if (type() == FL_RADIO_BUTTON && !value()) {
	setonly();
	if (when() & FL_WHEN_CHANGED) do_callback();
      } else if (type() == FL_TOGGLE_BUTTON) {
	value(!value());
	if (when() & FL_WHEN_CHANGED) do_callback();
      }
      if (when() & FL_WHEN_RELEASE) do_callback(); else set_changed();
      return 1;
    default:
      return 0;
    }
  }

  void in_Xcas_input_1arg(Fl_Widget * widget,const std::string & chaine ,bool eval){
    if (!widget)
      return;
    Fl::focus(widget);
    const context * contextptr = get_context(widget);
    if ( Equation * eqwptr=dynamic_cast<Equation *> (widget) ){
      vecteur position;
      gen * act=Equation_selected(eqwptr->data,eqwptr->attr,eqwptr->w(),position,1,contextptr);
      if (act){
	eqwptr->handle_text(chaine,act);
	return;
      }
      gen f("'"+string(chaine)+"'",contextptr);
      if (eval || f.type!=_FUNC)
	eqwptr->eval_function(f);
      else {
	// make_new_help(chaine);
	gen g=eqwptr->get_selection();
	if (g.type!=_VECT){
	  if (f==at_limit)
	    g=gen(makevecteur(g,vx_var,0),_SEQ__VECT);
	  if (f==at_integrate || f==at_derive)
	    g=gen(makevecteur(g,vx_var),_SEQ__VECT);
	  if (f==at_sum)
	    g=gen(makevecteur(g,k__IDNT_e,1,n__IDNT_e),_SEQ__VECT);
	}
	eqwptr->replace_selection(symbolic(*f._FUNCptr,g));
      }
      return;
    }
    bool par= chaine.empty() || chaine[0]!='_';
    string s(chaine);
    if (Multiline_Input_tab * the_input =dynamic_cast<Multiline_Input_tab *>(widget)){
      if (par)
	s = chaine+"(";
      the_input->insert_replace(s,false);
      return;
    }
    if (Fl_Input * the_input =dynamic_cast<Fl_Input *>(widget)){
      string t(the_input->value());
      size_t sel1=the_input->position(),sel2=the_input->mark();
      if (sel1>sel2){
	size_t tmp=sel1;
	sel1=sel2;
	sel2=tmp;
      }
      string t_before=t.substr(0,sel1);
      string t_selected=t.substr(sel1,sel2-sel1);
      string t_after=t.substr(sel2,t.size()-sel2);
      if (par)
	s += '(';
      s = t_before+s;
      s += t_selected;
      s += t_after;
      the_input->value( s.c_str());
      size_t pos1=t_before.size();
      size_t pos2=s.size()-t_after.size();
      if (sel1==sel2)
	the_input->position(pos2-1,pos2-1);
      else
	the_input->position(pos1,pos2);
      return;
    }
    if (Editeur * ed=dynamic_cast<Editeur *>(widget)){
      widget=ed->editor;
    }
    if (Xcas_Text_Editor * ed=dynamic_cast<Xcas_Text_Editor *>(widget)){
      int i=ed->insert_position();
      ed->buffer()->insert(i,(chaine+'(').c_str());
      ed->insert_position(i+chaine.size()+1);
      ed->set_tooltip();
      if (History_Pack * hp=get_history_pack(ed))
	hp->modified(false);
    }
  }

  void Xcas_input_1arg(Save_Focus_Button * m , void*) {
    in_Xcas_input_1arg(m->widget,m->label());
  }
    
  void Xcas_input_arg(Save_Focus_Button * m , const std::string & chaine) {
    in_Xcas_input_1arg(m->widget,chaine);
  }

  void Xcas_input_1arg(No_Focus_Button * m , void*) {
    in_Xcas_input_1arg(Fl::focus(),m->label());
  }
    
  void Xcas_input_arg(No_Focus_Button * m , const std::string & chaine) {
    in_Xcas_input_1arg(Fl::focus(),chaine);
  }

  void in_Xcas_input_char(Fl_Widget * widget,const std::string & chaine,char keysym){
    if (alt_ctrl_cb && alt_ctrl)
      alt_ctrl_cb(alt_ctrl);
    if (!widget)
      return;
    static string s;
    if (chaine.empty())
      s=" ";
    else 
      s=chaine;
    if (alt_ctrl & 2)
      s[0]=s[0] & 0x1f;
    if (alt_ctrl & 4)
      s[0]=s[0] | 0x80;
    Fl::e_text= (char *) s.c_str();
    Fl::e_length=s.size();
    if (alt_ctrl & 2)
      Fl::e_keysym=keysym & 0x1f;
    else {
      if (alt_ctrl & 4)
	Fl::e_keysym=keysym | 0x80;
      Fl::e_keysym=keysym;
    }
    alt_ctrl=0;
    fl_handle(widget);
  }

  void Xcas_input_char(Save_Focus_Button * m , void*) {
    in_Xcas_input_char(m->widget,m->label(),m->label()[0]);
  }

  void Xcas_input_label(No_Focus_Button * m , void*) {
    in_Xcas_input_char(Fl::focus(),m->label(),m->label()[0]);
  }

  void Xcas_input_char(No_Focus_Button * m , void*) {
    std::string s(m->label());
    if (m->labelfont()==FL_SYMBOL && s.size()==1){
      switch (s[0]){
      case 'a':
	s="alpha";
	break;
      case 'b':
	s="beta";
	break;
      case 'c':
	s="chi";
	break;
      case 'd':
	s="delta";
	break;
      case 'e':
	s="epsilon";
	break;
      case 'f':
	s="phi";
	break;
      case 'g':
	s="gamma";
	break;
      case 'h':
	s="eta";
	break;
      case 'i':
	s="iota";
	break;
      case 'j':
	s="varphi";
	break;
      case 'k':
	s="kappa";
	break;
      case 'l':
	s="lambda";
	break;
      case 'm':
	s="mu";
	break;
      case 'n':
	s="nu";
	break;
      case 'p':
	s="pi";
	break;
      case 'q':
	s="theta";
	break;
      case 'r':
	s="rho";
	break;
      case 's':
	s="sigma";
	break;
      case 't':
	s="tau";
	break;
      case 'u':
	s="upsilon";
	break;
      case 'v':
	s="omega";
	break;
      case 'x':
	s="xi";
	break;
      case 'y':
	s="psi";
	break;
      case 'z':
	s="zeta";
	break;
      case 'C':
	s="Chi";
	break;
      case 'D':
	s="Delta";
	break;
      case 'F':
	s="Phi";
	break;
      case 'G':
	s="Gamma";
	break;
      case 'L':
	s="Lambda";
	break;
      case 'P':
	s="Pi";
	break;
      case 'Q':
	s="Theta";
	break;
      case 'S':
	s="Sigma";
	break;
      case 'W':
	s="Omega";
	break;
      case 'X':
	s="Xi";
	break;
      case 'Y':
	s="Psi";
	break;
      }
    }
    in_Xcas_input_char(Fl::focus(),s,m->label()[0]);
  }

  void Xcas_binary_op(Save_Focus_Button * m , void*) {
    Xcas_input_char(m,0);
  }

  void Xcas_binary_op(No_Focus_Button * m , void*) {
    Xcas_input_char(m,0);
  }

  void Xcas_input_0arg(Save_Focus_Button * m , const std::string & chaine) {
    Fl_Widget * widget = m->widget;
    if (!widget)
      return;
    string s;
    if (chaine.empty())
      s=" ";
    else
      s=chaine;
    Fl::e_text= (char *) s.c_str();
    Fl::e_length=s.size();
    Fl::e_keysym=s[0];
    fl_handle(widget);        
  }

  void Xcas_input_0arg(No_Focus_Button * , const std::string & chaine) {
    static string s;
    if (chaine.empty())
      s=" ";
    else
      s=chaine;
    Fl_Widget * wid = Fl::focus();
    const context * contextptr = get_context(wid);
    if (Equation * eq=dynamic_cast<Equation * >(wid)){
      vecteur position;
      gen * act=Equation_selected(eq->data,eq->attr,eq->w(),position,1,contextptr);
      if (act){
	eq->handle_text(s,act);
	return;
      }
      if (s.size()>1){
	eq->parse_desactivate();
	gen f(s,contextptr);
	eq->replace_selection(f);
	return;
      }
    }
    else {
      Fl::e_text= (char *) s.c_str();
      Fl::e_length=s.size();
      Fl::e_keysym=s[0];
      fl_handle(wid);
    }
  }

  void Xcas_input_0arg(const std::string & chaine) {
    Xcas_input_0arg((No_Focus_Button *)0,chaine);
  }

  static void cb_Editeur_Exec(Save_Focus_Button * m , void*) {
    Fl_Widget * widget = m->widget;
    Fl_Text_Editor * Program_editor = find_editor(m);
    if (Program_editor){
      const context * contextptr = get_context(Program_editor);
      if (!Program_editor || !widget)
	return ;
      if ( !dynamic_cast<Fl_Input *>(widget)){
	m->window()->show();
	Fl::focus(Program_editor);
	return ;
      }
      Editeur * ed = dynamic_cast<Editeur *>(Program_editor->parent());
      int r= Program_editor->insert_position();
      char * ch=Program_editor->buffer()->line_text(r);
      string s=ch;
      free(ch);
      // Scan for a line ending with //
      int ss=s.size();
      for (int i=ss-2;i>=0;--i){
	if (s[i]=='/' && s[i+1]=='/')
	  s=s.substr(0,i);
      }
      ss=s.size();
      bool comment=s.empty(),nextline=true;
      for (int i=0;i<ss;++i){
	if (s[i]!=' ')
	  break;
	if (i==ss-1)
	  comment=true;
      }
      if (!comment){
	gen g;
	bool close=lexer_close_parenthesis(contextptr);
	lexer_close_parenthesis(false,contextptr);
	try {
	  if (calc_mode(contextptr)==38)
	    calc_mode(contextptr)=-38;
	  g=gen(s,contextptr);
	}
	catch (std::runtime_error & e){
	  cerr << e.what() << endl;
	}
	lexer_close_parenthesis(close,contextptr);
	if (giac::first_error_line(contextptr)){
	  if (ed && ed->log){
	    ed->log->value((gettext("Parse error line ")+print_INT_(giac::first_error_line(contextptr))+ gettext(" column ")+print_INT_(giac::lexer_column_number(contextptr))+gettext(" at ") +giac::error_token_name(contextptr)).c_str());
	    ed->log->redraw();
	    Fl_Group * gr=ed->log->parent();
	    if (gr->children()>2){
	      int ah = gr->child(2)->h(); 
	      Fl_Widget * wid=gr->child(2);
	      gr->remove(wid);
	      delete wid;
	      gr->Fl_Widget::resize(gr->x(),gr->y(),gr->w(),gr->h()-ah);
	      History_Pack * hp=get_history_pack(gr);
	      if (hp)
		hp->redraw();
	    }
	  }
	  nextline=false;
	}
      }
      if (nextline) {
	int next=Program_editor->buffer()->line_end(r)+1;
	int nextend=Program_editor->buffer()->line_end(next);
	Program_editor->insert_position(next);
	Program_editor->show_insert_position();
	Program_editor->buffer()->select(next,nextend);
	Program_editor->redraw();
	if (!comment){
	  Fl::e_text= (char *) s.c_str();
	  Fl::e_length=s.size();
	  widget->window()->show();
	  fl_handle(widget);
	  widget->do_callback();
	  widget=Fl::focus();
	}
	m->window()->show();
	Fl::focus(Program_editor);
	Fl::flush();
	usleep(500000);
	widget->window()->show();
	Fl::focus(widget);
      }
    }
  }

  static void cb_Editeur_Preview(Fl_Menu_* m , void*) {
    Fl_Text_Editor * e = find_editor(m);
    if (e)
      widget_ps_print(e,e->label(),false);
  }

  static void cb_Editeur_Print(Fl_Menu_* m , void*) {
    Fl_Text_Editor * e = find_editor(m);
    if (e)
      widget_print(e);
  }

  bool Editeur::eval(){
    log = find_log_output(parent());
    const context * contextptr = get_context(this);
    static string logs;
    if (log){
      logs=gettext("Syntax compatibility mode: ")+print_program_syntax(xcas_mode(contextptr))+"\n";
    }
    gen g;
    bool close=lexer_close_parenthesis(contextptr);
    lexer_close_parenthesis(false,contextptr);
    try {
      char * ch=editor->buffer()->text(); 
      string res(ch); 
      free(ch);
      if (calc_mode(contextptr)==38)
	calc_mode(contextptr)=-38;
      g=gen(res,contextptr);
    }
    catch (std::runtime_error & er){
      cerr << er.what() << endl;
    }
    lexer_close_parenthesis(close,contextptr);
    if (giac::first_error_line(contextptr)){
      int pos1=editor->buffer()->skip_lines(0,giac::first_error_line(contextptr)-1);
      // int pos2=editor->buffer()->skip_lines(pos1,1);
      int pos2=pos1+giac::lexer_column_number(contextptr)-1;
      pos1=giacmax(pos2-giac::error_token_name(contextptr).size(),0);
      editor->buffer()->select(pos1,pos2);
      editor->show_insert_position();
      editor->redraw();
      if (log){
	logs+=gettext("Parse error line ")+print_INT_(giac::first_error_line(contextptr))+ gettext( " column ")+print_INT_(giac::lexer_column_number(contextptr))+gettext(" at ") +giac::error_token_name(contextptr);
	log->value(logs.c_str());
	Fl_Group * gr=log->parent();
	if (gr->children()>2){
	  int ah = gr->child(2)->h(); 
	  Fl_Widget * wid=gr->child(2);
	  gr->remove(wid);
	  delete wid;
	  gr->Fl_Widget::resize(gr->x(),gr->y(),gr->w(),gr->h()-ah);
	}
	// if (Parse_error_output)
	//  Parse_error_output->value(logs.c_str());
	output_resize_parent(log);
      }
      return false;
    }
    else {
      if (log){
	string locals=check_local_assign(g,contextptr);
	if (locals.empty())
	  logs = gettext("Success!");
	else {
	  logs+=string(gettext("Success but ... "))+locals;
	}
	log->value(logs.c_str());
	// if (Parse_error_output)
	//  Parse_error_output->value(logs.c_str());
	output_resize_parent(log);
      }
      // NOTE: do_callback clears the logs
      // protecteval(g,2,contextptr);
      do_callback();
      return true;
      /* Exec g
      context * contextptr = 0;
      if (History_Pack * p =get_history_pack(this)){
      contextptr = p->contextptr;
	// focus to next
	p->_sel_begin=-1;
	int pos=p->set_sel_begin(this)+1;
	p->focus(pos);
      }
      if (giac::is_context_busy(contextptr) && log)
	log->value((log->value()+string("\nCan not evaluate, system busy")).c_str());
      else
	thread_eval(g,eval_level(contextptr),contextptr);
      */
    }
  }

  static void cb_Editeur_Test(Fl_Widget* m , void*) {
    Fl_Text_Editor * e = find_editor(m);
    if (e){
      Fl::focus(e);
      Editeur * ed = dynamic_cast<Editeur *>(m->parent());
      if (ed){
	ed->eval();
      }
    }
  }

  static void cb_Editeur_Gotoline(Fl_Widget* m , void*) {
    Fl_Text_Editor * e = find_editor(m);
    if (e){
      Fl::focus(e);
      Fl_Value_Input * in = dynamic_cast<Fl_Value_Input *>(m);
      if (in){
	double l=in->value();
	if (l<1)
	  l=1;
	int newpos=e->buffer()->skip_lines(0,int(l)-1);
	e->insert_position(newpos);
      }
    }
  }

  static void cb_Editeur_Indent_line(Fl_Widget* m , void*) {
    Fl_Text_Editor * e = find_editor(m);
    if (e){
      Fl::focus(e);
      Editeur * ed = dynamic_cast<Editeur *>(m->parent());
      if (ed){
	ed->editor->indent(e->insert_position());
      }
    }
  }

  static void cb_Editeur_Indent_all(Fl_Widget* m , void*) {
    Fl_Text_Editor * e = find_editor(m);
    if (e){
      Fl::focus(e);
      Editeur * ed = dynamic_cast<Editeur *>(m->parent());
      if (ed){
	ed->editor->indent();
      }
    }
  }

  static void cb_Editeur_Next(Fl_Widget * m , void*) {
    Editeur * e = dynamic_cast<Editeur *>(m->parent());
    if (e && e->editor){
      Fl_Text_Editor * ed=e->editor;
      int pos = ed->insert_position();
      int found = ed->buffer()->search_forward(pos, e->search.c_str(), &pos);
      if (found) {
	// Found a match; select and update the position...
	ed->buffer()->select(pos, pos+e->search.size());
	ed->insert_position(pos+e->search.size());
	ed->show_insert_position();
	ed->redraw();
      }
      else {
	fl_alert("%s","No more occurrences of '%s' found!", e->search.c_str());
	ed->insert_position(0);
      }
      Fl::focus(ed);
    }
  }

  static void cb_Editeur_Extension(Fl_Menu_* m , void*) {
    Editeur * e=dynamic_cast<Editeur *>(m->parent());
    if (e){
      const char * res=fl_input("New extension",e->extension.c_str());
      if (res)
	e->extension=res;
    }
  }

  void cb_Editeur_Extend(Fl_Widget * m , void*) {
    Editeur * e=dynamic_cast<Editeur *>(m->parent());
    if (e->h()<=e->window()->h()-100){
      increase_size(e->editor,100);
      e->editor->redraw();
    }
  }

  void cb_Editeur_Shrink(Fl_Widget * m , void*) {
    Editeur * e=dynamic_cast<Editeur *>(m->parent());
    if (e->h()>200){
      increase_size(e->editor,-100);
      e->editor->redraw();
    }
  }

  void logo_eval_callback(const giac::gen & evaled_g,void * param){
    Fl_Widget * wid=static_cast<Fl_Widget *>(param);
    if (!wid || !wid->parent())
      return;
    wid->parent()->redraw();
  }

  void cb_Editeur_Exec_All(Fl_Widget * m , void*) {
    Editeur * e=dynamic_cast<Editeur *>(m->parent());
    context * contextptr = get_context(e);
    if (e){
      if (Logo * l=dynamic_cast<Logo *>(e->parent())){
	char * ch = e->editor->buffer()->text();
	string s=ch;
	free(ch);
	gen g;
	bool close=lexer_close_parenthesis(contextptr);
	lexer_close_parenthesis(false,contextptr);
	try { 
	  if (calc_mode(contextptr)==38)
	    calc_mode(contextptr)=-38;
	  g=gen(s,contextptr); 
	} 
	catch (std::runtime_error & e){ cerr << e.what() << endl; }
	lexer_close_parenthesis(close,contextptr);
	if (giac::first_error_line(contextptr)){
	  int pos1=e->editor->buffer()->skip_lines(0,giac::first_error_line(contextptr)-1);
	  // int pos2=e->editor->buffer()->skip_lines(pos1,1);
	  int pos2=pos1+giac::lexer_column_number(contextptr)-1;
	  pos1=giacmax(pos2-giac::error_token_name(contextptr).size(),0);
	  e->editor->buffer()->select(pos1,pos2);
	  e->editor->show_insert_position();
	  e->editor->redraw();
	  fl_alert("%s",(gettext("Parse error line ")+print_INT_(giac::first_error_line(contextptr))+ gettext(" column ")+print_INT_(giac::lexer_column_number(contextptr)) + gettext(" at ") +giac::error_token_name(contextptr)).c_str());
	}
	else {
	  if (!is_context_busy(contextptr)){
#if 1
	    make_thread(g,eval_level(contextptr),logo_eval_callback,e,contextptr);
	    // protecteval(g,eval_level(contextptr),contextptr);
#else // crashes sometimes, perhaps something non reentrant
	      thread_eval(g,eval_level(contextptr),contextptr);
#endif
	  }
	  l->t->redraw();
	  e->editor->insert_position(s.size());
	}
	return;
      }
      // Not a logo, try to run it inside focus
      fl_message("%s",gettext("Should be used inside a Logo level"));
      return;
      Fl_Widget * wid=Fl::focus();
      History_Pack * hp=get_history_pack(wid);
      if (hp){
	hp->set_sel_begin(wid);
	int n=hp->_sel_begin;
	int r=0;
	int taille=e->editor->buffer()->length();
	for (;r<taille;++n){
	  char * ch=e->editor->buffer()->line_text(r);
	  r=e->editor->buffer()->line_end(r)+1;
	  Fl_Multiline_Input * widget=dynamic_cast<Fl_Multiline_Input *>(new_question_multiline_input(hp->w()-hp->_printlevel_w,hp->labelsize()+4));
	  if (!widget) break;
	  widget->value(ch);
	  widget->set_changed();
	  free(ch);
	  hp->add_entry(n,widget);
	  widget->do_callback();
	}
      }
    }
  }

  static void cb_Editeur_Search(Fl_Widget* m , void*) {
    static Fl_Window * w = 0;
    static Fl_Input * i1=0, * i2=0; // i1=search, i2=replace
    static Fl_Button * button0 = 0 ; // cancel
    static Fl_Button * button1 =0; // next
    static Fl_Button * button2 =0; // replace + next
    static Fl_Button * button3 =0; // replace all
    Fl_Text_Editor * ed = find_editor(m);
    if (!ed) return;
    Editeur * e =dynamic_cast<Editeur *>(m->parent());
    if (ed && e){
      int dx=400,dy=100;
      if (!w){
	Fl_Group::current(0);
	w=new Fl_Window(dx,dy);
	i1=new Fl_Input(dx/6,2,dx/3-2,dy/3-4,gettext("Search"));
	i1->tooltip(gettext("Word to search"));
	i1->when(FL_WHEN_ENTER_KEY|FL_WHEN_NOT_CHANGED);
	i2=new Fl_Input((2*dx)/3,2,dx/3-2,dy/3-4,"Replace");
	i2->tooltip(gettext("Replace by"));
	i2->when(FL_WHEN_ENTER_KEY|FL_WHEN_NOT_CHANGED);
	button0 = new Fl_Button(2,2+(2*dy/3),dx/2-4,dy/3-4);
	button0->shortcut(0xff0d);
	button0->label(gettext("Cancel"));
	button1 = new Fl_Button(dx/2+2,2+(2*dy)/3,dx/2-4,dy/3-4);
	button1->shortcut(0xff1b);
	button1->label(gettext("Next"));
	button1->when(FL_WHEN_RELEASE);
	button2 = new Fl_Button(2,2+(dy)/3,dx/2-4,dy/3-4);
	button2->shortcut(0xff1b);
	button2->label(gettext("Replace"));
	button2->when(FL_WHEN_RELEASE);
	button3 = new Fl_Button(dx/2+2,2+(dy)/3,dx/2-4,dy/3-4);
	button3->shortcut(0xff1b);
	button3->label(gettext("Replace all"));
	button3->when(FL_WHEN_RELEASE);
	w->end();
	w->resizable(w);
      }
      i1->value(e->search.c_str());
      History_Pack * hp=get_history_pack(ed);
      w->label("Search/Replace");
      w->set_modal();
      w->show();
      w->hotspot(w);
      Fl::focus(i1);
      autosave_disabled=true;
      for (;;) {
	Fl_Widget *o = Fl::readqueue();
	if (!o) Fl::wait();
	else {
	  if (o == button0) break;
	  if (o == w) break;
	  if (o==button3){ // Replace all
	    e->search=i1->value();
	    const char *find = e->search.c_str();
	    const char *replace = i2->value();
	    int i=1;
	    if (!replace[0])
	      i=fl_ask("%s","Really replace by nothing?");
	    if (i && find[0] != 0){
	      ed->previous_word();
	      int pos = ed->insert_position();
	      while (ed->buffer()->search_forward(pos, find, &pos)) {
		ed->buffer()->select(pos, pos+strlen(find));
		ed->buffer()->remove_selection();
		ed->buffer()->insert(pos, replace);
		ed->redraw();
		pos += strlen(replace);
		ed->insert_position(pos);
	      }
	      if (hp)
		hp->modified(false);
	      break;
	    }
	  } // end button3
	  if (o==button2 || o==i2){ // Replace+next
	    if (ed->buffer()->selection_text()==e->search){
	      e->search=i1->value();
	      const char *find = e->search.c_str();
	      const char *replace = i2->value();
	      if (find[0] != 0){
		if (ed->insert_position()>0)
		  ed->previous_word();
		int pos = ed->insert_position();
		if (ed->buffer()->search_forward(pos, find, &pos)) {
		  ed->buffer()->select(pos, pos+strlen(find));
		  ed->buffer()->remove_selection();
		  ed->buffer()->insert(pos, replace);
		  if (hp)
		    hp->modified(false);
		  ed->redraw();
		  pos += strlen(replace);
		  ed->insert_position(pos);
		}
	      }
	    } // end I1!=I2
	  } // end button2
	  if (o==i1 || o==i2 || o==button1 || o==button2){ // Find next
	    e->search=i1->value();
	    if (e->search.empty())
	      break;
	    int pos = ed->insert_position();
	    int found = ed->buffer()->search_forward(pos, e->search.c_str(), &pos);
	    if (found) {
	      // Found a match; select and update the position...
	      ed->buffer()->select(pos, pos+e->search.size());
	      ed->insert_position(pos+e->search.size());
	      ed->show_insert_position();
	      ed->redraw();
	    }
	    else {
	      fl_alert("%s","No occurrences of '%s' found!", e->search.c_str());
	      ed->insert_position(0);
	      ed->show_insert_position();
	    }
	    Fl::focus(ed);
	  } // end button1
	  if (o==i1){
	    ed->window()->show();
	    Fl::focus(ed);
	    break;
	  }
	} // end else of if Fl::wait()
      } // end for (;;)
      autosave_disabled=false;
      w->hide();
      Fl::focus(ed);
    }
  }


  static void cb_dialog_test(Fl_Text_Editor * ed){
    int dx=240,dy=300,l=14;
    if (ed->window()){
      dx=int(0.5*ed->window()->w());
      dy=int(0.3*ed->window()->h());
      l=ed->window()->labelsize();
    }
    static Fl_Window * w = 0;
    static Fl_Return_Button * button0 = 0 ; // ok
    static Fl_Button * button1 =0; // cancel, quit
    static Fl_Input * cond=0; // condition
    static Fl_Input * ifclause=0; // if
    static Fl_Input * elseclause=0; // else
    if (!w){
      Fl_Group::current(0);
      w=new Fl_Window(dx,dy);
      int dh=dy/5;
      int dw=dx/2;
      int y_=2;
      cond=new Fl_Input(dw/2,y_,3*dw/2-2,dh-2,gettext("if"));
      cond->tooltip(gettext("Enter test e.g. x<=0"));
      cond->value("x>1");
      y_ += dh;
      ifclause=new Fl_Input(dw/2,y_,3*dw/2-2,dh-2,gettext("then"));
      ifclause->tooltip(gettext("Enter instruction(s) executed when test returns true, e.g. x:=-x;"));
      ifclause->value("x:=1-x;");
      y_ += dh;
      elseclause=new Fl_Input(dw/2,y_,3*dw/2-2,dh-2,gettext("else"));
      elseclause->tooltip(gettext("Enter instruction(s) executed when test returns false, leave empty if none."));
      elseclause->value("");
      y_ += dh;
      y_ += dh;
      button0 = new Fl_Return_Button(2,y_,dx/5-4,dh-4);
      button0->label(gettext("OK"));
      button0->shortcut(0xff0d);
      button1 = new Fl_Button(dx/5+2,y_,dx/5-4,dh-4);
      button1->label(gettext("Cancel"));
      button1->shortcut(0xff1b);
    }
    w->label((gettext("New test")));
    change_group_fontsize(w,l);
    w->set_modal();
    w->show();
    w->hotspot(w);
    Fl::focus(cond);
    autosave_disabled=true;
    context * contextptr=get_context(ed);
    int r=-2;
    for (;;) {
      if (r==-2){
	// re-init fields value for function?
      }
      r=-1;
      Fl_Widget *o = Fl::readqueue();
      if (!o) Fl::wait();
      else {
	if (o == button0) r = 0; // apply and quit
	if (o == button1) r = 1; // cancel changes, quit
	if (r==0){
	  gen g(cond->value(),contextptr);
	  if (g.type!=_SYMB && g.type!=_IDNT)
	    fl_message("%s",gettext("Invalid condition"));
	  g=gen(ifclause->value(),contextptr);
	  if (is_undef(g))
	    fl_message("%s",gettext("Invalid if clause"));
	}
	if (o == w) r=1; 
	if (r==0 || r==1)
	  break;
      }
    }
    autosave_disabled=false;
    w->hide();
    if (r==0){
      int i=ed->insert_position();
      string s=("si ");
      s += cond->value();
      s += " alors ";
      s += ifclause->value();
      gen g(elseclause->value(),contextptr);
      if (!is_undef(g)){
	s += " sinon ";
	s += elseclause->value();
      }
      s += " fsi;\n";
      ed->buffer()->insert(i,s.c_str());
      int delta=s.size();
      if (Xcas_Text_Editor * xed=dynamic_cast<Xcas_Text_Editor *>(ed)){
	int j=ed->buffer()->line_end(i)+1;
	delta += xed->indent(j)-j;
      }
      ed->insert_position(i+delta);
    }
  }

  static void cb_dialog_loop(Fl_Text_Editor * ed){
    int dx=240,dy=300,l=14;
    if (ed->window()){
      dx=int(0.5*ed->window()->w());
      dy=int(0.3*ed->window()->h());
      l=ed->window()->labelsize();
    }
    static Fl_Window * w = 0;
    static Fl_Return_Button * button0 = 0 ; // ok
    static Fl_Button * button1 =0; // cancel, quit
    static Fl_Check_Button * pour=0, * tantque=0;
    static Fl_Input * count=0; // for variable
    static Fl_Input * start=0; 
    static Fl_Input * stop=0; 
    static Fl_Input * step=0;
    static Fl_Input * loop=0;
    static Fl_Input * cond=0;
    if (!w){
      Fl_Group::current(0);
      w=new Fl_Window(dx,dy);
      int dh=dy/5;
      int dw=dx/2;
      int y_=2;
      pour=new Fl_Check_Button(dw/2,y_,dw/2-2,dh-2,gettext("for"));
      pour->tooltip(gettext("Check this button for a defined loop"));
      pour->value(1);
      tantque=new Fl_Check_Button(3*dw/2,y_,dw/2-2,dh-2,gettext("while"));
      tantque->tooltip(gettext("Check this button for a undefined loop"));
      tantque->value(0);
      y_ += dh;
      count=new Fl_Input(dw/4,y_,dw/4-2,dh-2,gettext("Var"));
      count->tooltip(gettext("Enter counter name, e.g. k"));
      count->value("k");
      start=new Fl_Input(3*dw/4,y_,dw/4-2,dh-2,gettext("Start"));
      start->tooltip(gettext("First value of counter in the loop"));
      start->value("1");
      stop=new Fl_Input(5*dw/4,y_,dw/4-2,dh-2,gettext("Stop"));
      stop->tooltip(gettext("Last value of counter in the loop"));
      stop->value("10");
      step=new Fl_Input(7*dw/4,y_,dw/4-2,dh-2,gettext("Step"));
      step->tooltip(gettext("Increment the counter value at each new execution of the loop"));
      step->value("");
      cond=new Fl_Input(dw,y_,dw-2,dh-2,gettext("Condition"));
      cond->tooltip(gettext("Enter condition for continuation of loop"));
      cond->value("k>=0");
      cond->hide();
      y_ += dh;
      loop=new Fl_Input(dw/3,y_,2*dw-dw/3-2,dh-2,gettext("Loop"));
      loop->tooltip(gettext("Enter instruction(s) to be executed, for example print(y); print(y*y);"));
      loop->value("print(k);");
      y_ += dh;
      y_ += dh;
      button0 = new Fl_Return_Button(2,y_,dx/5-4,dh-4);
      button0->label(gettext("OK"));
      button0->shortcut(0xff0d);
      button1 = new Fl_Button(dx/5+2,y_,dx/5-4,dh-4);
      button1->label(gettext("Cancel"));
      button1->shortcut(0xff1b);
    }
    w->label((gettext("New loop")));
    change_group_fontsize(w,l);
    w->set_modal();
    w->show();
    autosave_disabled=true;
    w->hotspot(w);
    Fl::focus(loop);
    context * contextptr=get_context(ed);
    int r=-2;
    for (;;) {
      if (r==-2){
	// re-init fields value for function?
      }
      r=-1;
      Fl_Widget *o = Fl::readqueue();
      if (!o) Fl::wait();
      else {
	if (o == button0) r = 0; // apply and quit
	if (o == button1) r = 1; // cancel changes, quit
	if (r==0 && pour->value()){
	  gen g(count->value(),contextptr);
	  if (g.type!=_IDNT)
	    fl_message("%s",gettext("Invalid counter"));
	}
	if (o==tantque)
	  pour->value(1-tantque->value());
	if (o==pour)
	  tantque->value(1-pour->value());
	if (tantque->value()==1){
	  count->hide(); start->hide(); stop->hide(); step->hide(); cond->show();
	}
	else {
	  count->show(); start->show(); stop->show(); step->show(); cond->hide();
	}
	if (o == w) r=1; 
	if (r==0 || r==1)
	  break;
      }
    }
    autosave_disabled=false;
    w->hide();
    if (r==0){
      int i=ed->insert_position();
      string s;
      if (tantque->value()){
	s = "tantque ";
	s += cond->value();
	s += " faire\n";
	s +=  loop->value();
	s += "\nftantque;\n";
      }
      else {
	s="pour ";
	s += count->value();
	s += " de ";
	s += start->value();
	s += " jusque ";
	s += stop->value();
	gen g(step->value(),contextptr);
	if (!is_undef(g)){
	  s += " pas ";
	  s += step->value();
	}
	s += " faire\n";
	s += loop->value();
	s += "\nfpour;\n";
      }
      ed->buffer()->insert(i,s.c_str());
      int delta=s.size();
      if (Xcas_Text_Editor * xed=dynamic_cast<Xcas_Text_Editor *>(ed)){
	int j=ed->buffer()->line_end(i)+1;
	delta += xed->indent(j)-j;
	j=ed->buffer()->line_end(j)+1;
	delta += xed->indent(j)-j;
	j=ed->buffer()->line_end(j)+1;
	delta += xed->indent(j)-j;
      }
      ed->insert_position(i+delta);
    }
  }

  void cb_choose_func(Fl_Text_Editor * ed){
    int dx=240,dy=300,l=14;
    if (ed->window()){
      dx=int(0.5*ed->window()->w());
      dy=int(0.3*ed->window()->h());
      l=ed->window()->labelsize();
    }
    static Fl_Window * w = 0;
    static Fl_Return_Button * button0 = 0 ; // ok
    static Fl_Button * button1 =0; // cancel, quit
    static Fl_Input * args=0; // arguments
    static Fl_Input * name=0; // name
    static Fl_Input * locs=0; // local variables
    static Fl_Input * ret=0; // return value
    if (!w){
      Fl_Group::current(0);
      w=new Fl_Window(dx,dy);
      int dh=dy/5;
      int dw=dx/2;
      int y_=2;
      name=new Fl_Input(dw,y_,dw-2,dh-2,gettext("Name"));
      name->tooltip(gettext("Enter function name, e.g. f or myfunc"));
      name->value("f");
      y_ += dh;
      args=new Fl_Input(dw,y_,dw-2,dh-2,gettext("Arguments"));
      args->tooltip(gettext("Enter function argument(s), e.g. x or a,b,c. Leave empty if no argument"));
      args->value("x");
      y_ += dh;
      locs=new Fl_Input(dw,y_,dw-2,dh-2,gettext("Locals"));
      locs->tooltip(gettext("Enter local variable(s), e.g. k or n1,n2,n3. Leave empty if no local variables"));
      locs->value("k");
      y_ += dh;
      ret=new Fl_Input(dw,y_,dw-2,dh-2,gettext("Return value"));
      ret->tooltip(gettext("Enter return value, e.g. x+1 or [a,b]. Leave empty if no local variables"));
      ret->value("x*x");
      y_ += dh;
      button0 = new Fl_Return_Button(2,y_,dx/5-4,dh-4);
      button0->label(gettext("OK"));
      button0->shortcut(0xff0d);
      button1 = new Fl_Button(dx/5+2,y_,dx/5-4,dh-4);
      button1->label(gettext("Cancel"));
      button1->shortcut(0xff1b);
    }
    w->label((gettext("New function")));
    change_group_fontsize(w,l);
    w->set_modal();
    w->show();
    autosave_disabled=true;
    w->hotspot(w);
    Fl::focus(name);
    context * contextptr=get_context(ed);
    int lang=language(contextptr);
    int r=-2;
    for (;;) {
      if (r==-2){
	// re-init fields value for function?
      }
      r=-1;
      Fl_Widget *o = Fl::readqueue();
      if (!o) Fl::wait();
      else {
	if (o == button0) r = 0; // apply and quit
	if (o == button1) r = 1; // cancel changes, quit
	if (r==0){
	  gen g(name->value(),contextptr);
	  if (g.type!=_IDNT)
	    fl_message("%s",gettext("Invalid functionname"));
	}
	if (o == w) r=1; 
	if (r==0 || r==1)
	  break;
      }
    }
    autosave_disabled=false;
    w->hide();
    if (r==0){
      int i=0,addi=0; // i=ed->insert_position(),addi=0;
      string s;
      giac::context * contextptr = get_context(ed);
      switch (xcas_mode(contextptr)){
      case 0:
	s+=name->value();
	s+='(';
	s+=args->value();
	s+=")";
	if (0 && lang==1)
	  s+=" fonction";
	else 
	  s+=":={";
	s+="\n";
	if (strlen(locs->value())){
	  s+="  local ";
	  s+=locs->value();
	  s+=";\n  ";
	}
	addi=s.size();
	s += "\n";
	if (strlen(ret->value())){
	  if (lang==1)
	    s+="  retourne ";
	  else
	    s+="  return ";
	  s+=ret->value();
	  s+=";";
	}	  
	s += "\n";
	if (0 && lang==1)
	  s+="ffonction";
	else
	  s+="}";
	s+=":;\n\n";
	ed->buffer()->insert(i,s.c_str());  
	ed->insert_position(i+addi);
	break;
      case 1:
	s+=name->value();
	s+=":=proc(";
	s+=args->value();
	s+=")\n";
	if (strlen(locs->value())){
	  s+="  local ";
	  s+=locs->value();
	  s+=";\n  ";
	}
	addi=s.size();
	s += "\n";
	if (strlen(ret->value())){
	  s+="  return ";
	  s+=ret->value();
	  s+=";";
	}	  
	s += "\nend:\n\n";
	ed->buffer()->insert(i,s.c_str());  
	ed->insert_position(i+addi);
	break;
      case 2:
	s+=name->value();
	s+=":=proc(";
	s+=args->value();
	s+=")\n";
	if (strlen(locs->value())){
	  s+="  local ";
	  s+=locs->value();
	  s+=";\n  ";
	}
	addi=s.size();
	s += "\n";
	if (strlen(ret->value())){
	  s+="  return ";
	  s+=ret->value();
	  s+=";";
	}	  
	s += "\nend_proc:\n\n";
	ed->buffer()->insert(i,s.c_str());  
	ed->insert_position(i+addi);
	break;
      case 3:
	s+=':';
	s+=name->value();
	s+='(';
	s+=args->value();
	s+=")\n:Func\n";
	if (strlen(locs->value())){
	  s+="\n:Local ";
	  s+=locs->value();
	  s+="\n:";
	}
	addi=s.size();
	s += "\n:";
	if (strlen(ret->value())){
	  s+="\n:Return ";
	  s+=ret->value();
	}
	s += "\n:EndFunc\n\n";
	ed->buffer()->insert(i,s.c_str());  
	ed->insert_position(i+addi);
	break;
      }
    }
  }
 
 static void cb_prg_func(Fl_Menu_* m , void*) {
    Fl_Text_Editor * ed = do_find_editor(m);
    if (ed){
      cb_choose_func(ed);
      Fl::focus(ed);
    }
    else
      Xcas_input_0arg("f():={ local ; ; return ;}");
  }

 static void cb_prg_si(Fl_Menu_* m , void*) {
    Fl_Text_Editor * ed = do_find_editor(m);
    if (ed){
      cb_dialog_test(ed);
      Fl::focus(ed);
    }
    else
      Xcas_input_0arg("si alors sinon fsi;");
  }

 static void cb_prg_pour(Fl_Menu_* m , void*) {
    Fl_Text_Editor * ed = do_find_editor(m);
    if (ed){
      cb_dialog_loop(ed);
      Fl::focus(ed);
    }
    else
      Xcas_input_0arg("pour de jusque faire fpour;");
  }

  static void cb_prg_local(Fl_Menu_* m , void*) {
    Fl_Text_Editor * ed = do_find_editor(m);
    if (ed){
      giac::context * contextptr = get_context(ed);
      int i=ed->insert_position();
      ed->buffer()->insert(i,xcas_mode(contextptr)==3?"\n:Local ":"local ;\n  ");
      ed->insert_position(i+6);
    }
    else
      Xcas_input_0arg("local ;");    
  }

  static void cb_prg_return(Fl_Menu_* m , void*) {
    Fl_Text_Editor * ed = do_find_editor(m);
    if (ed){
      giac::context * contextptr = get_context(ed);
      int i=ed->insert_position();
      ed->buffer()->insert(i,xcas_mode(contextptr)==3?"\n:Return ":"return ;");
      ed->insert_position(i+7);
    }
    else
      Xcas_input_0arg("return ;");    
  }

  static void cb_prg_ifthenelse(Fl_Menu_* m , void*) {
    Fl_Text_Editor * ed = do_find_editor(m);
    if (ed){
      giac::context * contextptr = get_context(ed);
      int i=ed->insert_position();
      switch (xcas_mode(contextptr)){
      case 0:
	ed->buffer()->insert(i,"if () { ;} else { ;}\n  ");
	ed->insert_position(i+4);
	break;
      case 1: 
	ed->buffer()->insert(i,"if  then ; else ; fi;\n  ");
	ed->insert_position(i+3);
	break;
      case 2:
	ed->buffer()->insert(i,"if  then ; else ; end_if;\n  ");
	ed->insert_position(i+3);
	break;
      case 3:
	ed->buffer()->insert(i,"\n:If Then\n:Else \n:EndIf;\n");
	ed->insert_position(i+4);
	break;
      }
    }
    else {
      giac::context * contextptr = get_context(Xcas_input_focus);
      switch (xcas_mode(contextptr)){
      case 0:
	Xcas_input_0arg("if () { ; } else { ; }");
	break;
      case 1: 
	Xcas_input_0arg("if  then ; else ; fi;");
	break;
      case 2:
	Xcas_input_0arg("if  then ; else ; end_if;");
	break;
      case 3:
	Xcas_input_0arg("\n:If Then\n:Else \n:EndIf;\n");
	break;
      }
    }
  }

  static void cb_prg_sialorssinon(Fl_Menu_* m , void*) {
    Fl_Text_Editor * ed = do_find_editor(m);
    if (ed){
      giac::context * contextptr = get_context(ed);
      int i=ed->insert_position();
      ed->buffer()->insert(i,"si  alors ; sinon ; fsi;\n  ");
      ed->insert_position(i+3);
    }
    else
      Xcas_input_0arg("si  alors ; sinon  ; fsi");
  }

  static void cb_prg_pour_(Fl_Menu_* m , void*) {
    Fl_Text_Editor * ed = do_find_editor(m);
    if (ed){
      giac::context * contextptr = get_context(ed);
      int i=ed->insert_position();
      ed->buffer()->insert(i,"pour  de  jusque  faire\n    ;\n  fpour;\n  ");
      ed->insert_position(i+5);
    }
    else
      Xcas_input_0arg("pour  de  jusque  faire ; fpour;");
  }

  static void cb_prg_tantque(Fl_Menu_* m , void*) {
    Fl_Text_Editor * ed = do_find_editor(m);
    if (ed){
      giac::context * contextptr = get_context(ed);
      int i=ed->insert_position();
      ed->buffer()->insert(i,"tantque  faire\n    ;\n  ftantque;\n  ");
      ed->insert_position(i+8);
    }
    else
      Xcas_input_0arg("tantque  faire ; ftantque;");
  }

  static void cb_prg_repeter(Fl_Menu_* m , void*) {
    Fl_Text_Editor * ed = do_find_editor(m);
    if (ed){
      giac::context * contextptr = get_context(ed);
      int i=ed->insert_position();
      ed->buffer()->insert(i,"repeter\n    ;\n  jusqu_a ;\n");
      ed->insert_position(i+12);
    }
    else
      Xcas_input_0arg("repeter jusqu_a ;");
  }

  static void cb_prg_ifthen(Fl_Menu_* m , void*) {
    Fl_Text_Editor * ed = do_find_editor(m);
    if (ed){
      giac::context * contextptr = get_context(ed);
      int i=ed->insert_position();
      switch (xcas_mode(contextptr)){
      case 0:
	ed->buffer()->insert(i,"if () { ;}\n");
	ed->insert_position(i+5);
	break;
      case 1: 
	ed->buffer()->insert(i,"if  then ; fi;\n");
	ed->insert_position(i+4);
	break;
      case 2: 
	ed->buffer()->insert(i,"if  then ; end_if;\n");
	ed->insert_position(i+4);
	break;
      case 3: 
	ed->buffer()->insert(i,"\n:If Then\n:EndIf\n");
	ed->insert_position(i+4);
	break;
      }
    }
    else {
      giac::context * contextptr = get_context(Xcas_input_focus);
      switch (xcas_mode(contextptr)){
      case 0:
	Xcas_input_0arg("if () { ;}");
	break;
      case 1:
	Xcas_input_0arg("if then ; fi;");
	break;
      case 2:
	Xcas_input_0arg("if then ; end_if;");
	break;
      case 3:
	Xcas_input_0arg("\n:If Then\n:EndIf\n");
	break;
      }
    }
  }

  static void cb_prg_print(Fl_Menu_* m , void*) {
    Fl_Text_Editor * ed = do_find_editor(m);
    if (ed){
      int i=ed->insert_position();
      ed->buffer()->insert(i,gettext("print();"));
      ed->insert_position(i+strlen(gettext("print();"))-2);
    }
    else
      Xcas_input_0arg(gettext("print();"));
  }

  static void cb_prg_input(Fl_Menu_* m , void*) {
    Fl_Text_Editor * ed = do_find_editor(m);
    if (ed){
      int i=ed->insert_position();
      ed->buffer()->insert(i,gettext("input();"));
      ed->insert_position(i+strlen(gettext("input();"))-2);
    }
    else
      Xcas_input_0arg(gettext("input();"));
  }

  static void cb_prg_switch(Fl_Menu_* m , void*) {
    Fl_Text_Editor * ed = do_find_editor(m);
    if (ed){
      int i=ed->insert_position();
      ed->buffer()->insert(i,"switch(){\n    case : break;\n  }\n  ");
      ed->insert_position(i+8);
    }
    else
      Xcas_input_0arg("switch () {case : break;}");
  }

  static void cb_prg_trycatch(Fl_Menu_* m , void*) {
    Fl_Text_Editor * ed = do_find_editor(m);
    if (ed){
      giac::context * contextptr = get_context(ed);
      int i=ed->insert_position();
      ed->buffer()->insert(i,xcas_mode(contextptr)==3?"\n:Try\n\n:Else\n\n:EndTry":"\ntry {\n ;\n}\ncatch(){\n ;}\n");
      ed->insert_position(i+7);
    }
    else {
      giac::context * contextptr = get_context(Xcas_input_focus);      
      Xcas_input_0arg(xcas_mode(contextptr)==3?"\n:Try\n\n:Else\n\n:EndTry":"try { ;}catch(){ ;}");
    }
  }

  static void cb_prg_for(Fl_Menu_* m , void*) {
    Fl_Text_Editor * ed = do_find_editor(m);
    if (ed){
      giac::context * contextptr = get_context(ed);
      int i=ed->insert_position();
      switch (xcas_mode(contextptr)){
      case 0:
	ed->buffer()->insert(i,"for(;;){\n    ;\n  }\n  ");
	ed->insert_position(i+4);
	break;
      case 1:
	ed->buffer()->insert(i,"for  from to do\n    ;\n  od;\n  ");
	ed->insert_position(i+4);
	break;
      case 2:
	ed->buffer()->insert(i,"for  from to do\n    ;\n  end_for;\n  ");
	ed->insert_position(i+4);
	break;
      case 3:
	ed->buffer()->insert(i,"\n:For ,,\n\n:EndFor\n");
	ed->insert_position(i+5);
	break;
      }
    }
    else {
      giac::context * contextptr = get_context(Xcas_input_focus);
      switch (xcas_mode(contextptr)){
      case 0:
	Xcas_input_0arg("for( ; ; ){ ;}");
	break;
      case 1: 
	Xcas_input_0arg("for  from to do ; od;");
	break;
      case 2:
	Xcas_input_0arg("for  from to do ; end_for;");
	break;
      case 3:
	Xcas_input_0arg("\n:For ,,\n:\n:EndFor;\n");
	break;
      }
    }
  }

  static void cb_prg_while(Fl_Menu_* m , void*) {
    Fl_Text_Editor * ed = do_find_editor(m);
    if (ed){
      giac::context * contextptr = get_context(ed);
      int i=ed->insert_position();
      switch (xcas_mode(contextptr)){
      case 0:
	ed->buffer()->insert(i,"while(){\n    ;\n  }\n  ");
	ed->insert_position(i+6);
	break;
      case 1:
	ed->buffer()->insert(i,"while  do\n    ;\n  od;\n  ");
	ed->insert_position(i+6);
	break;
      case 2:
	ed->buffer()->insert(i,"while  do\n    ;\n  end_while;\n  ");
	ed->insert_position(i+6);
	break;
      case 3:
	ed->buffer()->insert(i,"\n:While \n:\n:EndWhile\n");
	ed->insert_position(i+7);
	break;
      }
    }
    else {
      giac::context * contextptr = get_context(Xcas_input_focus);
      switch (xcas_mode(contextptr)){
      case 0:
	Xcas_input_0arg("while( ){ ;}");
	break;
      case 1: 
	Xcas_input_0arg("while  do ; od;");
	break;
      case 2:
	Xcas_input_0arg("while  do ; end_while;");
	break;
      case 3:
	Xcas_input_0arg("\n:While \n:\n:EndWhile;\n");
	break;
      }
    }
  }

  static void cb_prg_break(Fl_Menu_* m , void*) {
    Fl_Text_Editor * ed = do_find_editor(m);
    if (ed){
      giac::context * contextptr = get_context(ed);
      int i=ed->insert_position();
      ed->buffer()->insert(i,xcas_mode(contextptr)==3?":Stop \n:":"break;\n  ");
      ed->insert_position(i+8);
    }
    else {
      giac::context * contextptr = get_context(Xcas_input_focus);
      Xcas_input_0arg(xcas_mode(contextptr)==3?":Stop \n":"break;");
    }
  }

  static void cb_prg_continue(Fl_Menu_* m , void*) {
    Fl_Text_Editor * ed = do_find_editor(m);
    if (ed){
      giac::context * contextptr = get_context(ed);
      int i=ed->insert_position();
      ed->buffer()->insert(i,xcas_mode(contextptr)==3?":Cycle  \n":"continue;\n  ");
      ed->insert_position(i+11);
    }
    else {
      giac::context * contextptr = get_context(Xcas_input_focus);
      Xcas_input_0arg(xcas_mode(contextptr)==3?":Cycle  \n":"continue;");
    }
  }

  Fl_Menu_Item Editeur_menu[] = {
    {gettext("Prog"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("Load"), 0,  (Fl_Callback*)cb_Editeur_Load, 0, 0, 0, 0, 14, 56},
    {gettext("Insert"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("file"), 0,  (Fl_Callback*)cb_Editeur_Insert_File, 0, 0, 0, 0, 14, 56},
    {gettext("xcas text"), 0,  (Fl_Callback*)cb_Editeur_Insert_Xcas, 0, 0, 0, 0, 14, 56},
    {gettext("maple text"), 0,  (Fl_Callback*)cb_Editeur_Insert_Maple, 0, 0, 0, 0, 14, 56},
    {gettext("mupad text"), 0,  (Fl_Callback*)cb_Editeur_Insert_Mupad, 0, 0, 0, 0, 14, 56},
    {gettext("ti text"), 0,  (Fl_Callback*)cb_Editeur_Insert_Ti, 0, 0, 0, 0, 14, 56},
    {0},
    {gettext("Save"), 0,  (Fl_Callback*)cb_Editeur_Save, 0, 0, 0, 0, 14, 56},
    {gettext("Save as"), 0,  (Fl_Callback*)cb_Editeur_Save_as, 0, 0, 0, 0, 14, 56},
    {gettext("File extension"), 0,  (Fl_Callback*)cb_Editeur_Extension, 0, 0, 0, 0, 14, 56},
    {gettext("Export"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("xcas text"), 0,  (Fl_Callback*)cb_Editeur_Export_Xcas, 0, 0, 0, 0, 14, 56},
    {gettext("maple text"), 0,  (Fl_Callback*)cb_Editeur_Export_Maple, 0, 0, 0, 0, 14, 56},
    {gettext("mupad text"), 0,  (Fl_Callback*)cb_Editeur_Export_Mupad, 0, 0, 0, 0, 14, 56},
    {gettext("ti text"), 0,  (Fl_Callback*)cb_Editeur_Export_Ti, 0, 0, 0, 0, 14, 56},
    {0},
    {gettext("Translate"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("xcas"), 0,  (Fl_Callback*)cb_Editeur_Translate_Xcas, 0, 0, 0, 0, 14, 56},
    {gettext("maple"), 0,  (Fl_Callback*)cb_Editeur_Translate_Maple, 0, 0, 0, 0, 14, 56},
    {gettext("mupad"), 0,  (Fl_Callback*)cb_Editeur_Translate_Mupad, 0, 0, 0, 0, 14, 56},
    {gettext("ti"), 0,  (Fl_Callback*)cb_Editeur_Translate_Ti, 0, 0, 0, 0, 14, 56},
    {0},
    {gettext("Export/Print"), 0,  0, 0, 64, 0, 0, 14, 56},
    //    {gettext("latex preview"), 0,  (Fl_Callback*)cb_Tableur_LaTeX_Preview, 0, 0, 0, 0, 14, 56},
    //    {gettext("latex printer"), 0,  (Fl_Callback*)cb_Tableur_LaTeX_Print, 0, 0, 0, 0, 14, 56},
    {gettext("Preview"), 0,  (Fl_Callback*)cb_Editeur_Preview, 0, 0, 0, 0, 14, 56},
    {gettext("to printer"), 0,  (Fl_Callback*)cb_Editeur_Print, 0, 0, 0, 0, 14, 56},
    {0}, // end Print
    {0}, // end File
    {gettext("Edit"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("Paste"), 0,  (Fl_Callback *) cb_Paste, 0, 0, 0, 0, 14, 56},
    {gettext("Search (Ctrl-F)"), 0,  (Fl_Callback *) cb_Editeur_Search, 0, 0, 0, 0, 14, 56},
    {gettext("Indent line (Esc)"), 0,  (Fl_Callback *) cb_Editeur_Indent_line, 0, 0, 0, 0, 14, 56},
    {gettext("Indent all"), 0,  (Fl_Callback *) cb_Editeur_Indent_all, 0, 0, 0, 0, 14, 56},
    {gettext("Parse"), 0,  (Fl_Callback *) cb_Editeur_Test, 0, 0, 0, 0, 14, 56},
    {gettext("Exec all"), 0xffc4,  (Fl_Callback *) cb_Editeur_Exec_All, 0, 0, 0, 0, 14, 56},
    {gettext("Extend editor"), 0xffc2,  (Fl_Callback *) cb_Editeur_Extend, 0, 0, 0, 0, 14, 56},
    {gettext("Shrink editor"), 0xffc3,  (Fl_Callback *) cb_Editeur_Shrink, 0, 0, 0, 0, 14, 56},
    {0}, // end Edit
    {gettext("Add"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("Func"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("new function"), 0,  (Fl_Callback *) cb_prg_func, 0, 0, 0, 0, 14, 56},
    {gettext("local"), 0,  (Fl_Callback *) cb_prg_local, 0, 0, 0, 0, 14, 56},
    {gettext("return"), 0,  (Fl_Callback *) cb_prg_return, 0, 0, 0, 0, 14, 56},
    {0}, // end Func
    {gettext("IO"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("print"), 0,  (Fl_Callback *) cb_prg_print, 0, 0, 0, 0, 14, 56},
    {gettext("input"), 0,  (Fl_Callback *) cb_prg_input, 0, 0, 0, 0, 14, 56},
    {0}, // end IO
    {gettext("Test"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("new test"), 0,  (Fl_Callback *) cb_prg_si, 0, 0, 0, 0, 14, 56},
    {gettext("si alors sinon"), 0,  (Fl_Callback *) cb_prg_sialorssinon, 0, 0, 0, 0, 14, 56},
    {gettext("if [then]"), 0,  (Fl_Callback *) cb_prg_ifthen, 0, 0, 0, 0, 14, 56},
    {gettext("if [then] else"), 0,  (Fl_Callback *) cb_prg_ifthenelse, 0, 0, 0, 0, 14, 56},
    {gettext("switch"), 0,  (Fl_Callback *) cb_prg_switch, 0, 0, 0, 0, 14, 56},
    {gettext("try catch"), 0,  (Fl_Callback *) cb_prg_trycatch, 0, 0, 0, 0, 14, 56},
    {0}, // end Test
    {gettext("Loop"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("new loop"), 0,  (Fl_Callback *) cb_prg_pour, 0, 0, 0, 0, 14, 56},
    {gettext("pour"), 0,  (Fl_Callback *) cb_prg_pour_, 0, 0, 0, 0, 14, 56},
    {gettext("tantque"), 0,  (Fl_Callback *) cb_prg_tantque, 0, 0, 0, 0, 14, 56},
    {gettext("repeter jusqu_a"), 0,  (Fl_Callback *) cb_prg_repeter, 0, 0, 0, 0, 14, 56},
    {gettext("for "), 0,  (Fl_Callback *) cb_prg_for, 0, 0, 0, 0, 14, 56},
    {gettext("while "), 0,  (Fl_Callback *) cb_prg_while, 0, 0, 0, 0, 14, 56},
    {gettext("break"), 0,  (Fl_Callback *) cb_prg_break, 0, 0, 0, 0, 14, 56},
    {gettext("continue"), 0,  (Fl_Callback *) cb_prg_continue, 0, 0, 0, 0, 14, 56},
    {0}, // end Loop
    {0}, // end Prg
    {0} // end menu
  };

  void Xcas_Text_Editor::clear_gchanged(){
    gchanged=false;
  }

  void Xcas_Text_Editor::set_gchanged(){
    gchanged=true;
  }

  Xcas_Text_Editor::Xcas_Text_Editor(int X, int Y, int W, int H, Fl_Text_Buffer *b,const char* l ):Fl_Text_Editor(X,Y,W,H,l){
    styletable=vector<Fl_Text_Display::Style_Table_Entry>(styletable_init,styletable_init+styletable_n);
    tableur=0;
    gchanged=true;
    labeltype(FL_NO_LABEL);
    color(FL_WHITE);
    buffer(b); 
    char *style = new char[buffer()->length()+1];
    char *text = buffer()->text();
    
    
    memset(style, 'A', buffer()->length());
    style[buffer()->length()] = '\0';
    
    stylebuf = new Fl_Text_Buffer(buffer()->length());
    
    style_parse(text, style, buffer()->length());
    
    stylebuf->text(style);
    delete[] style;
    free(text);
    highlight_data(stylebuf, &styletable.front(),styletable.size(),'A', style_unfinished_cb, 0);

   }

  Editeur::Editeur(int x,int y,int w,int h,const char * l):Fl_Group(x,y,w,h,l),contextptr(0){
    bool logo=false;
    if (parent()){
      labelsize(parent()->labelsize());
      logo=dynamic_cast<Logo *>(parent());
    }
    Fl_Group::current(this);
    int L=(3*labelsize())/2;
    Fl_Text_Buffer * b = new Fl_Text_Buffer;
    editor=new Xcas_Text_Editor(x,y+L,w,h-L,b,l);
    editor->Fl_Text_Display::textsize(labelsize());
    editor->labelsize(labelsize());
    log = 0;
    if (logo){
      menubar = new Fl_Menu_Bar(x,y,w/2,L);
    }
    else {
      menubar = new Fl_Menu_Bar(x,y,w/4,L);
    }
    int s= Editeur_menu->size();
    Fl_Menu_Item * menuitem = new Fl_Menu_Item[s];
    for (int i=0;i<s;++i)
      *(menuitem+i)=*(Editeur_menu+i);
    menubar->menu (menuitem);
    change_menu_fontsize(menuitem,3,labelsize()); // 3=#submenus
    linenumber=0; nxt_button=0; exec_button=0; func_button=0; si_button=0; pour_button=0;
    if (!logo){
      linenumber = new Fl_Value_Input(x+w/3-w/12,y,w/12,L);
      linenumber->tooltip(gettext("Line number"));
      linenumber->callback((Fl_Callback *)cb_Editeur_Gotoline);
      linenumber->when(FL_WHEN_ENTER_KEY|FL_WHEN_NOT_CHANGED);
      nxt_button = new Fl_Button(x+w/3,y,w/12,L);
      nxt_button->labelsize(labelsize());
      nxt_button->label(gettext("nxt"));
      nxt_button->tooltip(gettext("Find next occurence (defined by Edit->Search)"));
      nxt_button->callback((Fl_Callback *) cb_Editeur_Next);
      if (parent()==window()){
	exec_button = new Save_Focus_Button(x+w/3-w/12,y,w/12,L);
	exec_button->callback((Fl_Callback *)cb_Editeur_Exec);
	exec_button->labelsize(labelsize());
	exec_button->label(gettext("exe"));
	exec_button->tooltip(gettext("Exec line in current widget"));
      }
      else {
	func_button = new Fl_Button(nxt_button->x()+nxt_button->w(),y,w/12,L);
	func_button->labelsize(labelsize());
	func_button->label(gettext("Func"));
	func_button->tooltip(gettext("Assistant for function creation"));
	func_button->callback((Fl_Callback *) cb_prg_func);
	si_button = new Fl_Button(func_button->x()+func_button->w(),y,w/12,L);
	si_button->labelsize(labelsize());
	si_button->label(gettext("Test"));
	si_button->tooltip(gettext("Assistant for test creation"));
	si_button->callback((Fl_Callback *) cb_prg_si);
	pour_button = new Fl_Button(si_button->x()+si_button->w(),y,w/12,L);
	pour_button->labelsize(labelsize());
	pour_button->label(gettext("Loop"));
	pour_button->tooltip(gettext("Assistant for loop creation"));
	pour_button->callback((Fl_Callback *) cb_prg_pour);	
      }
    }
    button = new Fl_Button(x+w/2+w/6,y,w/12,L);
    button->labelsize(labelsize());
    // button->label(logo?"OK":"OK (F9)");
    button->label("OK");
    button->labelcolor(FL_DARK_GREEN);
    button->tooltip(gettext("Parse current program"));
    button->callback((Fl_Callback *) logo?cb_Editeur_Exec_All:cb_Editeur_Test);
    button->shortcut(0xffc6); // FIXME: quick fix, otherwise Esc leaves xcas
    save_button = new Fl_Button(x+w/2+w/4,y,w/12,L);
    save_button->labelsize(labelsize());
    save_button->label("Save");
    save_button->tooltip(gettext("Save current program"));
    save_button->callback((Fl_Callback *) cb_Editeur_Save);
    output = new Fl_Output(x+w/2+w/4+save_button->w(),y,w-w/2-w/4-save_button->w(),L);
    output->labelsize(labelsize());
    end();

    b->add_modify_callback(style_update, editor); 
    b->add_modify_callback(Editor_changed_cb, editor); 
    resizable(editor);
    switch (xcas_mode(contextptr)){
    case 0:
      extension="cxx";
      break;
    case 1:
      extension="map";
      break;
    case 2:
      extension="mu";
      break;
    case 3:
      extension="ti";
      break;
    }
    parent_redraw(this);
  }

  void Editeur::position(int & i,int & j) const {
    if (editor->buffer()->selected())
      editor->buffer()->selection_position(&i,&j); 
    else {
      i=editor->insert_position();
      j=i; 
    }
  }

  std::string Editeur::value() const {
    char * ch=editor->buffer()->text(); 
    string res=ch; 
    free(ch);
    return res;
  }

  std::string Xcas_Text_Editor::value() const {
    char * ch=buffer()->text(); 
    string res=ch; 
    free(ch);
    return res;
  }

  void Xcas_Text_Editor::insert_replace(const string & s,bool select){
    if (buffer()->selected()){
      int b,e;
      buffer()->selection_position(&b,&e);
      buffer()->remove_selection();
      buffer()->insert(b,s.c_str());
      buffer()->select(b,b+s.size());
    }
    else {
      int b=insert_position();
      buffer()->insert(b,s.c_str());
      buffer()->select(b,b+s.size());
    }
    set_gchanged();
  }

  int Xcas_Text_Editor::mark() const {
    int b,e;
    buffer()->selection_position(&b,&e);
    return e;
  }

  void Xcas_Text_Editor::match(){
    static bool recursive=false;
    if (buffer()->selected())
      return;
    int pos=insert_position();
    int lastkey=Fl::event_key();
    if (lastkey!='(' && lastkey!='[' && lastkey!='{' && lastkey!='}' && lastkey!=']' && lastkey!=')' && lastkey!='0' && lastkey!='9' && lastkey!='\'' && lastkey != '=' && lastkey !=FL_Left && lastkey !=FL_Right)
      return;
    if (recursive)
      return;
    recursive=true;
    // check if cursor is on [, (, ), ]
    int p0=pos;
    char * ch=buffer()->text();
    string c(ch);
    free(ch);
    int pmax=c.size();
    bool closing=false,opening=false;
    if (pos<pmax)
      opening= c[pos]=='(' || c[pos]=='[' || c[pos]=='{';
    if (!opening && pos){
      closing = c[pos-1]==')' || c[pos-1]==']' || c[pos-1]=='}';
      if (closing)
	--p0;
    }
    if (opening || closing){
      bool ok=giac::matchpos(c,p0);
      if (!ok){
	if (closing)
	  p0=pos-1;
	else
	  p0=pos+1;
      }
      else {
	if (opening && pos<pmax)
	  p0=p0+1;
      }
      Fl::flush();
      usleep(100000);      
      if (true || !Fl::get_key(lastkey)){
	Fl::check();
	buffer()->select(pos,p0);
	unsigned s=selection_color();
	if (ok){
	  if (opening)
	    selection_color(FL_GREEN);
	  else
	    selection_color(fl_color_cube(0,FL_NUM_GREEN-1,2)); 
	}
	else
	  selection_color(FL_RED);
	damage(damage() | FL_DAMAGE_ALL);
	redraw();
	Fl::flush();
	usleep(70000);
	if (!Fl::ready()){
	  for (int i=0;i<giac::PARENTHESIS_NWAIT;++i){
	    usleep(50000);
	    if (Fl::ready())
	      break;
	  }
	}
	selection_color(s);
	buffer()->select(pos,pos);
	damage(damage() | FL_DAMAGE_ALL);
	redraw();
	Fl::flush();
      }
    }
    recursive=false;
  }

  void Xcas_Text_Editor::draw(){
    int clip_x,clip_y,clip_w,clip_h;
    fl_clip_box(x(),y(),w(),h(),clip_x,clip_y,clip_w,clip_h);
    fl_push_clip(clip_x,clip_y,clip_w,clip_h);
    Fl_Text_Editor::draw();
    fl_pop_clip();
  }

  const string motscleftab[] = {   
    "{",
    "Else",
    "ElseIf",
    "EndDlog",
    "EndFor",
    "EndFunc",
    "EndIf",
    "EndLoop",
    "EndPrgm",
    "EndTry",
    "EndWhile",
    "Exit",
    "Func",
    "Then",
    "alors",
    "and",
    "break",
    "by",
    "case",
    "catch",
    "continue",
    "de",
    "default",
    "do",
    "downto",
    "elif",
    "else",
    "end",
    "end_case",
    "end_for",
    "end_if",
    "end_proc",
    "end_while",
    "et",
    "faire",
    "ffaire",
    "fi",
    "fpour",
    "from",
    "fsi",
    "ftantque",
    "jusqu_a",
    "jusqua",
    "jusque",
    "non",
    "not",
    "od",
    "or",
    "ou",
    "pas",
    "sinon",
    "step",
    "then",
    "to",
    "until",
    "xor"
  };

  const std::vector<string> motsclef(motscleftab,motscleftab+sizeof(motscleftab)/sizeof(motscleftab[0]));

  // indent current line at position pos, return new current position
  int Xcas_Text_Editor::indent(int pos){
    int debut_ligne=buffer()->line_start(pos),indent=0;
    // Tab pressed -> indent current line
    char Lastchar;
    if (debut_ligne){
      char * ch_ = buffer()->line_text(pos-1),*ch=ch_;
      bool empty_line=true;
      int position=buffer()->line_start(debut_ligne-2);
      indent = 2;
      while (1){
	free(ch_);
	ch_ = buffer()->line_text(position);
	ch=ch_;
	int save_indent=indent;
	// Count spaces in ch
	for (;*ch;++ch,++indent){
	  if (*ch=='/' && *(ch+1)=='/')
	    break;
	  if (*ch!=' '){
	    empty_line=false;
	    break;
	  }
	}
	if (position<2 || !empty_line)
	  break;
	// line was empty, restore indent and go one line above
	indent = save_indent; 
	position=buffer()->line_start(position-2);
      }
      if (empty_line)
	indent = 0;
      else {
	int firstchar=*ch,lastchar = *ch,prevlast=0;
	if (lastchar ==';' && !*(ch+1))
	  indent -= 2;
	// Add spaces for each open (, open {, open [, 
	// remove spaces for ] } )
	for (;*ch;++ch){
	  switch (*ch){
	  case '(': case '[': case '{':
	    indent += 2;
		break;
	  case ')': case ']': case '}':
	    indent -=2;
	    break;
	  }
	  if (*ch=='/' && *(ch+1)=='/')
	    break;
	  if (*ch!=' '){
	    prevlast=lastchar;
	    lastchar=*ch;
	  }
	}
	Lastchar=lastchar;
	    // Last non space should be { or ; 
	if (lastchar=='{' || (lastchar=='}' && firstchar!='}') || (lastchar==';' && prevlast!='}' ) )
	  indent -=2;
	    free(ch_);
      }
    }
    // Now indent line
    char * ch_ = buffer()->line_text(pos), *ch=ch_;
    int delta=0;
    for (;*ch;++ch,--delta){
      if (*ch!=' ')
	break;
	}
    if (*ch=='}')
      indent -= 2;
    string mot;
    mot += *ch;
    for(char * ch1=ch+1;*ch1;++ch1){
      if (!isalpha(*ch1))
	break;
      mot += *ch1;
    }
    if (Lastchar!='}' && (equalposcomp(motsclef,mot)))
      indent -=2;
    indent=max(indent,0);
    delta += indent;
    string s(indent,' ');
    s += ch;
    free(ch_);
    int fin=buffer()->line_end(pos);
    buffer()->remove(debut_ligne,fin);
    buffer()->insert(debut_ligne,s.c_str());
    insert_position(pos+delta);
    redraw();
    return pos+delta;
  }

  void Xcas_Text_Editor::indent(){
    int pos=0;
    for (;pos<buffer()->length();){
      pos=indent(pos);
      pos=buffer()->line_end(pos)+1;
    }
  }

  void Xcas_Text_Editor::value(const char * s,bool select){
    if (buffer()->length()>0) 
      buffer()->remove(0,buffer()->length());
    buffer()->insert(0,s);
    insert_position(0);
    if (select)
      buffer()->select(0,strlen(s));
    else
      buffer()->select(0,0);
    set_gchanged();
  }

  void Xcas_Text_Editor::position(int b,int e){
    insert_position(b);
    buffer()->select(b,e);
  }

  void Xcas_Text_Editor::resize_nl_before(unsigned nl){
    if (tableur)
      return;
    /*
    Fl_Widget * wid=this;
    while (wid){
      if (dynamic_cast<Figure *>(wid))
	break;
      wid=wid->parent();
    }
    */
    string s = value();
    unsigned i=0,l=s.size();
    for (;i<l;++i){
      if (s[i]=='\n')
	++nl;
    }
    // increase_size(this,((nl>1 ||wid)?22:9)+nl*(labelsize()+3)-h());
    double fs = 1.05*fl_height(textfont(), textsize()); 
    int newsize=(nl>1?23:10)+int(nl*fs+.5);
#ifdef WIN32
    newsize += 2;
#endif
    increase_size(this,newsize-h());
    check_scrollbarsize();
    show_insert_position();
  }

  void Xcas_Text_Editor::set_tooltip(){
    static string toolt;
    History_Pack * hp=get_history_pack(this);
    int pos=insert_position();
    int wbeg=buffer()->line_start(pos);
    string s(buffer()->text_range(wbeg,pos));
    s=motclef(s);
    unsigned k=0; // quick check : is s a number?
    for (;k<s.size();++k){
      if (s[k]!='.' && s[k]!='e' && s[k]!='-' && (s[k]<'0' || s[k]>'9') )
	break;
    }
    if (s.size()>1 && k<s.size()){
      const aide & help=helpon(s,*giac::vector_aide_ptr(),giac::language(hp?hp->contextptr:0),giac::vector_aide_ptr()->size());
      toolt=writehelp(help,giac::language(hp?hp->contextptr:0));
      toolt += '\n';
      const char * ch=gettext("No help");
      if (toolt.size()>20 && toolt.substr(0,strlen(ch))!=ch)
	toolt += gettext("Press F1 to copy examples or for interactive help");
      else {
	if (s.size()<4){
	  tooltip("");
	  return;
	}
	toolt += gettext("Press F1 to open help index at ");
	toolt += s;
      }
      tooltip(toolt.c_str());
      int hh=height(toolt.c_str(),Fl_Tooltip::size());
      Fl_Tooltip::enter_area(this,0,-hh,0,0,toolt.c_str());
    }
    else
      tooltip("");
  }

  gen Xcas_Text_Editor::g() {
    if (!gchanged)
      return _g;
    giac::context * contextptr=get_context(this);
    _g=giac::gen(value(),contextptr);
    clear_gchanged();
    return _g;
  }

  int Xcas_Text_Editor::completion(){
    Editeur * ed =dynamic_cast<Editeur *>(parent());
    History_Pack * hp=get_history_pack(this);
    int pos=insert_position();
    if (pos)
      --pos;
#ifdef FL_DEVICE
    char car=buffer()->character(pos);
#else
    char car=buffer()->char_at(pos);
#endif
    if (car=='\n'){
      ++pos;
#ifdef FL_DEVICE
      car=buffer()->character(pos);
#else
      car=buffer()->char_at(pos);
#endif
    } 
    bool remove1=false;
    if (pos && car=='('){
      --pos;
      remove1=true;
#ifdef FL_DEVICE
      car=buffer()->character(pos);
#else
      car=buffer()->char_at(pos);
#endif
    } 
    if (1 || isalphan(car)){
      int wbeg=buffer()->word_start(pos);
      int wend=buffer()->word_end(pos);
      if (remove1) ++wend;
      pos=wend;
      char * cha=buffer()->text_range(wbeg,wend);
      string s(cha),ans;
      free(cha);
      int remove;
      if (int ii=handle_tab(s,*giac::vector_completions_ptr(),window()->w(),window()->h()/3,remove,ans,/* immediate out not allowed*/false)){
	window()->show();
	Fl::focus(this);
	handle(FL_FOCUS);
	pos=wend-remove;
	buffer()->remove(pos,wend);
	if (ii==1){
	  buffer()->insert(pos,(ans+"(").c_str());
	  insert_position(pos+ans.size()+1);
	}
	else {
	  buffer()->insert(pos,ans.c_str());
	  insert_position(pos+ans.size());
	}
	set_gchanged();
	if (hp)
	  hp->modified(false);
	set_tooltip();
	if (parent())
	  parent_redraw(parent());
      }
      return 1;
    }
    return 0;
  }

  void Xcas_Text_Editor::set_g(const gen & g){
    const giac::context * contextptr = get_context(this);
    value(g.print(contextptr).c_str());
    redraw();
    clear_gchanged();
    _g=g;
  }

  void Xcas_Text_Editor::check_scrollbarsize(){
    if (h()<25+labelsize()){
      char * ch=buffer()->text(),*ptr=ch; 
      if (ch){
	for (;*ptr;++ptr){
	  if (*ptr=='\n')
	    break;
	}
	if (!*ptr){
	  fl_font(FL_HELVETICA,labelsize());
	  double taille=1.4*fl_width(ch);
	  // cerr << ch << " " << taille << " " << labelsize() << endl;
	  if (taille>w()){ // make enough room for scrollbar
	    increase_size(this,25+labelsize()-h());
	  }
	}
	free(ch);
      }
    }
  }

  int Xcas_Text_Editor::handle(int event){    
    if (event==FL_UNFOCUS)
      return 1;
    if (event==FL_FOCUS || event==FL_PUSH)
      Xcas_input_focus=this;
    if (event==FL_PUSH && tableur)
      tableur->editing=true;
    Editeur * ed =dynamic_cast<Editeur *>(parent());
    if (event==FL_MOUSEWHEEL){
      if (!Fl::event_inside(this))
	return 0;
      int n=buffer()->count_lines(0,buffer()->length())+1;
      if (n*(labelsize()+1)<=h()+10)
	return 0;
      int l=buffer()->count_lines(0,insert_position())+1;
      if (Fl::e_dy<0){
	for (;l>n-h()/(2*labelsize());--l)
	  move_up();
	if (l<=1)
	  return 0;
	move_up();
      }
      else {
	for (;l<h()/(2*labelsize());++l)
	  move_down();
	if (l>=n)
	  return 0;
	move_down();
      }
      show_cursor();
      show_insert_position();
    }
    if (ed){
      if (ed->linenumber){
	int n=buffer()->count_lines(0,insert_position())+1;
	ed->linenumber->value(n);
      }
    }
    if (event==FL_MOUSEWHEEL)
      return 1;// Fl_Text_Editor::handle(event);
    giac::context * contextptr = get_context(this);
    History_Pack * hp=get_history_pack(this);
    if (h()<25+labelsize() && !ed && !tableur)
      check_scrollbarsize();
    if (event==FL_KEYBOARD){
      Xcas_input_focus=this;
      // FIXME: add 2 ([]) and 12 ({})
      if (Fl::event_text() && Fl::event_text()[0]==6){
	cb_Editeur_Search(this,0);
	return 1;
      }
      if (Fl::event_text() && Fl::event_text()[0]==14){
	cb_Editeur_Next(this,0);
	return 1;
      }
      if (!tableur && Fl::event_key()==FL_F+5){
	if (h()<window()->h()){
	  increase_size(this,100);
	  redraw();
	  return 1;
	}
      }
      if (!tableur && Fl::event_key()==FL_F+6){
	if (h()>200)
	  increase_size(this,-100);
	return 1;
      }
      int change_focus=0;
      if (!ed && Fl::event_text() && (Fl::event_text()[0]=='\r') && !Fl::event_state(FL_SHIFT | FL_CTRL | FL_ALT) ){
	if (tableur){
	  do_callback();
	  return 1;
	}
	else {
	  if (value().empty())
	    change_focus=1;
	  else {
	    history.push_back(value());
	    count=history.size();
	    int pos=-1;
	    History_Pack * hp=get_history_pack(this,pos);
	    if (hp)
	      hp->update_pos=pos;
	    History_Pack_cb_eval(this,0);
	    return 1;
	  }
	}
      }
      if (Fl::event_key()==FL_F+1 || (!ed && Fl::event_text() && (Fl::event_text()[0]==9)) ){
	if (completion())
	  return 1;
      }
      if (Fl::event_text() && (Fl::event_text()[0]==9 || Fl::event_key()==FL_Escape)){
	if (tableur){
	  if (tableur->editing){
	    tableur->editing=false;
	    Fl::focus(tableur);
	  }
	  else {
	    Fl::selection(*this,value().c_str(),value().size());
	    // Fl::selection(*this,value(),strlen(value()));
	    value("");
	  }
	  set_gchanged();
	  if (hp)
	    hp->modified(false);
	  return 1;
	}
	else {
	  int pos=insert_position();
	  indent(pos);
	  return 1;
	}
      } // end if (...) tabulation test
      int key=Fl::event_key();
      if (!tableur && buffer()->line_start(insert_position())==0 && 
	  ((key==FL_Up && !Fl::event_state(FL_SHIFT |FL_CTRL | FL_ALT)) 
	   || key==FL_Page_Up)
	  ) 
	change_focus=-1;
      if (!tableur && buffer()->line_end(insert_position())==buffer()->length() && 
	  ( (key==FL_Down  && !Fl::event_state(FL_SHIFT |FL_CTRL | FL_ALT))
	    || key==FL_Page_Down)
	  ) 
	change_focus=1;
      if (change_focus && hp){
	int pos=hp->focus(this);
	if (pos+change_focus>=0 && pos+change_focus<hp->children()){
	  hp->focus(pos+change_focus,true);
	  return 1;
	}
      }
    } // end if event==FL_KEYBOARD  
    if (event==FL_KEYBOARD && !ed && !tableur){
      if (Fl::event_text() && (Fl::event_text()[0]=='\n' || Fl::event_text()[0]=='\r')){
	resize_nl_before(2);
      }
    }
    if (event==FL_KEYBOARD && !ed && Fl::event_state(FL_CTRL | FL_ALT) && (Fl::event_key()==FL_Up || Fl::event_key()==FL_Down)){
      // not handled by the input, use history
      int s=history.size();
      Fl::focus(this);
      int start, end;
      if (count==s && buffer()->selection_position(&start,&end) && start>=0 && end>start){
	char * ch=buffer()->text_range(start,end);
	string S(ch);
	free(ch);
	if (history.empty() || S!=history.back()){
	  history.push_back(S);
	  ++s;
	}
      }
      if (Fl::event_key()==FL_Up){
	--count;
	if (count<0)
	  count=0;
      }
      else {
	++count;
	if (count>=s)
	  count=max(0,s-1);
      }
      if (count<s){
	if (buffer()->selection_position(&start,&end) && start>=0 && end>start)
	  buffer()->remove(start,end);
	else
	  start=insert_position();
	buffer()->insert(start,history[count].c_str());
	end = start + history[count].size();
	buffer()->select(start,end);
	set_gchanged();
	if (hp)
	  hp->modified(false);
      }
      return 1;
    }
    int res=Fl_Text_Editor::handle(event);
    if (!ed && event==FL_PASTE)
      resize_nl_before(1);
    if (event==FL_KEYBOARD){
#if 1 //ndef __APPLE__
      if (Fl::event_text() && Fl::event_text()[0]>32){
	count=history.size();
	set_gchanged();
	if (hp)
	  hp->modified(false);
	if (Fl::event_text()[0]==')')
	  tooltip("");
	else
	  set_tooltip();
      }
#endif
      if (!ed && !tableur && Fl::event_key()==FL_BackSpace){
	resize_nl_before(1);
	if (hp)
	  hp->modified(true);
      }
      if (tableur && tableur->editing && Fl::event_key()==FL_BackSpace && value().empty()){
	tableur->editing=false;
	Fl::focus(tableur);
	return 1;
      }
      if (ed && !tableur && Fl::event_text() && (Fl::event_text()[0]=='\n' || Fl::event_text()[0]=='\r')){
	indent(insert_position());
      }
      else
	match();
    }
    return res;
  }

#ifndef NO_NAMESPACE_XCAS
} // namespace giac
#endif // ndef NO_NAMESPACE_XCAS

#endif // HAVE_LIBFLTK
