// -*- mode:C++ ; compile-command: "g++ -I.. -g -c History.cc" -*-
#ifndef _HISTORY_H
#define _HISTORY_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifndef IN_GIAC
#include <giac/first.h>
#else
#include "first.h"
#endif
#ifdef HAVE_LIBFLTK
#include <FL/Fl_Pack.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Multiline_Input.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Tabs.H>
#endif
#include <string>
#ifndef IN_GIAC
#include <giac/giac.h>
#else
#include "giac.h"
#endif

#ifdef HAVE_LIBFLTK
char *file_chooser(const char *message,const char *pat,const char *fname,int relative=0);
char *load_file_chooser(const char *message,const char *pat,const char *fname,int relative,bool save);
#endif

#ifndef NO_NAMESPACE_XCAS
namespace xcas {
#endif // ndef NO_NAMESPACE_XCAS

#ifdef HAVE_LIBFLTK
  void add_recent_filename(const std::string & s,bool writerecent);
  void read_recent_filenames(Fl_Menu_ * menu); // read xcas_recent and init recent_filenames_menu
  extern void (*Xcas_load_filename)(const char * filename,bool modified); // set in hist.fl

#define FL_VERT_PALE 167
#define FL_ROUGE_PALE 211

  int confirm_close(const std::string & message);
  std::string unix_path(const std::string & winpath);
  extern std::string autosave_folder;

  extern Fl_Color Xcas_comment_color,Xcas_comment_background_color,Xcas_input_color,Xcas_input_background_color,Xcas_log_color,Xcas_log_background_color,Xcas_equation_color,Xcas_equation_background_color,Xcas_equation_input_color,Xcas_equation_input_background_color,Xcas_editor_color,Xcas_editor_background_color,Xcas_background_color;

  extern void (* Keyboard_Switch )(unsigned);

  void set_colors(Fl_Widget *,bool do_redraw=true);
  void set_cursor(Fl_Widget *t, Fl_Cursor c) ;

  // redraw parent widget if exists, otherwise redraw w
  void parent_redraw(Fl_Widget * w);
  // redraw all parent widgets in the hierarchy
  void parents_redraw(Fl_Widget * w);
  // Return parent or parent of fl_scroll if the parent is a fl_scroll
  Fl_Group * parent_skip_scroll(const Fl_Widget * g);

  // Output widget stream borrowed from Dietmar Kühl
  // http://www.inf.uni-konstanz.de/~kuehl/c++/iostream/
  class widgetbuf: public std::streambuf{
  private:
    Fl_Output	* text;
    
    void	put_buffer(void);
    void	put_char(int);
    
  protected:
    int	overflow(int);
    int	sync();
    
  public:
    widgetbuf(Fl_Output *, int = 0);
    ~widgetbuf();
  };
  
#ifdef WITH_MYOSTREAM
  class owstream: public giac::my_ostream
#else
  class owstream: public my_ostream
#endif
  {
  public:
    Fl_Output * output;
    giac::context * contextptr;
    owstream(Fl_Output *, giac::context * contextpt , int = 0);
    virtual ~owstream();
    void resize();
  };

  class Log_Output : public Fl_Multiline_Output {
  public:
    Log_Output(int X,int Y,int W,int H,char * ch=0):Fl_Multiline_Output(X,Y,W,H,ch){ textcolor(Xcas_log_color);};
    virtual int handle(int event);
  };

  Log_Output * find_log_output(Fl_Group * g);
  void output_resize(Fl_Input_ * output);
  void output_resize_parent(Fl_Input_ * output,bool resize_hp=true);
  // End output widget stream
  giac::gen warn_equal(const giac::gen & g,const giac::context * contextptr);

  class Clip_Scroll:public Fl_Scroll {
  public:
    Clip_Scroll(int X,int Y,int W,int H,const char * l=0):Fl_Scroll(X,Y,W,H,l) {};
    virtual void draw();
  };

  void cb_Paste(Fl_Widget* m , void*);

  extern Fl_Output * Parse_error_output ; // for syntax error msgs
  extern bool fl_handle_lock; // True when Fl::handle is busy
  void fl_handle(Fl_Widget * w); // Send event after fl_handle_lock check
  void xdvi(const std::string & name); // latex preview
  void dvips(const std::string & name); // latex print

  // a callback if the question widget is a multiline input
  void History_Pack_cb_eval(Fl_Widget * q , void*);

  class History_Fold;
  class No_Focus_Button;

  // Like a horizontal Fl_Pack, except that it handles dragging the space
  // and draws widgets number
  class History_Pack:public Fl_Group {
    int _my_push,_widget_i,_widget_h; // internally used by handle
    bool _selecting,_pushed,_moving,_saving;
    // Used for undo/redo mechanism: when a widget is created
    // inside a History_Pack the indexed_string has index -1
    // just before a widget is deleted, the index_string is created
    // with widget_sprint and index is set to the undo position
    std::multimap<Fl_Widget *,giac::indexed_string> widget_history_map;
  public:
    int undo_position;
    std::vector< std::vector<Fl_Widget *> > undo_history;
    void add_history_map(Fl_Widget * g,int undo_position);
    Fl_Widget * restore_history_map(Fl_Widget * g,int undo_position);
    bool _modified,_resize_above; 
    // NB resize_above is also used for next in evaluation
    // if true then the next entry in above history pack is evaled
    int _spacing,_printlevel_w,_sel_begin,_sel_end;
    int pretty_output;
    int next_delay;
    // set to true if you want to reeval the remaining items of the pack 
    bool eval_below,doing_eval; 
    bool eval_below_once,eval_next;
    giac::gen queue_val; // value to be evaled at queue_pos
    int queue_pos,update_pos; // position in queue, position of 1st eval
    giac::context * contextptr;
    // new_question() is provided by the class user
    // it must return a question widget, it is called every time add_entry
    // is called.
    Fl_Widget * (*new_question) (int W,int H); 
    // User defined callback for selection, called when mouse select occurs
    const char * (*_select)(const History_Pack * pack,int sel_begin,int sel_end); 
    // User defined callback for insertion, called by paste or by insert
    // should parse widgets and insert them before the int
    // If before_position==-1, insert at the end
    int (*_insert)(History_Pack * pack,const char * chaine,int length,int before_position);
    // Current harddisk filename (FIXME use url syntax)
    std::string * url;
    // This function is an "eval" function
    // it will return an answer widget, given a question widget
    // For example in a CAS, it would get the value of the arg widget
    // eval it and return it in an appropriate form (text or eqw...)
    Fl_Widget * (*eval) (Fl_Widget *); 
    // *******
    // Methods
    // *******
    History_Pack(int X,int Y,int W,int H,const char*l=0);
    virtual FL_EXPORT int handle(int);
    virtual FL_EXPORT void draw(); 
    void modified(bool do_backup); // backup + in_modified
    void in_modified(); // no save to undo
    void backup();
    void restore(int dpos);
    void clear_modified();
    void add_entry(int n,Fl_Widget * question);
    void add_entry(int n,Fl_Group * group);
    bool add_entry(int n);
    bool save(const char * ch=0); // use *url as filename
    bool save_all(); // save all History_Fold inside this history_pack
    bool save_as(const char * ch); // ask for a filename
    bool save_as(const char * filename,const char * ch,bool autosave_rm,bool warn_user,bool file_save_context) ;
    // bool save_as(const char * filename,const char * ch,bool autosave_rm=true,bool warn_user=true,bool save_context=true) ;
    bool close(const char * ch); 
    bool insert_before(int before_position,bool newurl=false,int mws=0); 
    // ask for a filename (xws or mws extension, mws does a xws translation)
    bool insert_url(const char * url,int before_position);
    void new_url(const char * newfile);
    void fold_selection();
    void flatten();
    bool resize();
    bool resize(int maxh);
    bool resize(int minh,int maxh);
    giac::gen parse(int n); // return the gen value of the n-th entry
    std::string value(int n) const; // return the string value of the n-th entry
    void set_gen_value(int n,const giac::gen & g,bool exec=true); // set n-th entry value
    Fl_Group * widget_group(int n); // group corresponding to position n
    void set_value(int n,const std::string & s,bool exec=true);
    bool remove_entry(int n,bool check=true); // does not update, call resize() and update()
    bool remove_selected_levels(bool check_in_history_fold);
    void update(int n=0); // if eval_below is true, recompute pack
    void next(int hp_pos=0); //  eval next child if eval_below is true
    int set_sel_begin(const Fl_Widget * w);
    int focus(const Fl_Widget * w) const; // find focus position
    bool focus(int pos,bool dofocus=false); // set focus on input no pos
    void set_scroller(Fl_Group * gr); // make gr visible by scrolling history pack
  };

  // Exported so that we can use it inside Geometry Edit menu
  void cb_Rm_Answers(Fl_Widget* m , void*) ;
  void cb_Delete(Fl_Widget* m , void*) ;
  void History_cb_Save(Fl_Widget* m , void*) ;
  void History_cb_Save_as(Fl_Widget* m , void*) ;
  void History_cb_Save_as_xcas_text(Fl_Widget* m , void*) ;
  void History_cb_Save_as_maple_text(Fl_Widget* m , void*) ;
  void History_cb_Save_as_mupad_text(Fl_Widget* m , void*) ;
  void History_cb_Save_as_ti_text(Fl_Widget* m , void*) ;
  void History_cb_Load(Fl_Widget* m , void*) ;
  void History_cb_Insert(Fl_Widget* m , void*) ;
  void History_cb_Insert_Figure(Fl_Widget* m , void*) ;
  void History_cb_Insert_Tableur(Fl_Widget* m , void*) ;
  void History_cb_Insert_Program(Fl_Widget* m , void*) ;
  void History_cb_Kill(Fl_Widget* m , void*) ;
  void History_cb_Preview(Fl_Widget* m , void*) ;
  void History_cb_Print(Fl_Widget* m , void*) ;
  void History_cb_Preview_selected(Fl_Widget* m , void*) ;
  void History_cb_LaTeX_Preview(Fl_Widget* m , void*) ;
  void History_cb_LaTeX_Print(Fl_Widget* m , void*) ;
  void History_cb_LaTeX_Preview_sel(Fl_Widget* m , void*) ;
  void History_cb_LaTeX_Print_sel(Fl_Widget* m , void*) ;
  void cb_New_Input(Fl_Widget* m , void*) ;
  void History_cb_New_Comment_Input(Fl_Widget* m , void*) ;
  void History_cb_New_Tableur(Fl_Widget* m , void*) ;
  void History_cb_New_Figure(Fl_Widget* m , void*) ;
  void History_cb_New_Figure3d(Fl_Widget* m , void*) ;
  void History_cb_New_Figurex(Fl_Widget* m , void*) ;
  void History_cb_New_Figure3dx(Fl_Widget* m , void*) ;
  void History_cb_New_Logo(Fl_Widget* m , void*) ;
  void History_cb_New_Equation(Fl_Widget* m , void*) ;
  void History_cb_New_Xcas_Text_Editor(Fl_Widget* m , void*) ;
  void History_cb_New_Program(Fl_Widget* m , void*) ;
  void History_cb_New_HF(Fl_Widget* m , void*) ;
  void History_cb_Run_Worksheet(Fl_Widget* m , void*) ;
  void History_cb_Run_Below(Fl_Widget* m , void*) ;
  void History_cb_Newline(Fl_Widget* m , void*) ;
  void History_cb_Fold(Fl_Widget* m , void*) ;
  void History_cb_Merge(Fl_Widget* m , void*) ;
  void History_cb_Flatten(Fl_Widget* m , void*) ;
  void History_cb_help_index(Fl_Widget * wid,void *);
  void History_cb_help_button(Fl_Widget* b , void*);
  void History_cb_Redo(Fl_Widget* m , void*) ;
  void History_cb_Undo(Fl_Widget* m , void*) ;

  // if w is an Fl_Input or Equation, put it's value in g
  int parse(Fl_Widget * w,giac::gen & g);
  bool set_gen_value(Fl_Widget * w,const giac::gen & g);
  bool set_value(Fl_Widget * w,const std::string & s,bool exec);

  // Get surrounding History_Pack if it exists
  History_Pack * get_history_pack(const Fl_Widget * w);
  giac::context * get_context(const Fl_Widget * w);
  void set_context(Fl_Widget * w,giac::context * contextptr);
  History_Pack * get_history_pack(const Fl_Widget * w,int & pos);
  History_Fold * get_history_fold(const Fl_Widget * wid);

  // History is a class for generic history support like in many
  // scientific softwares (e.g. CAS or matlab-like,...)
  // It shows a scrollable pack made of a collection of group of widgets
  // (which are stacked one over the next one)
  // Each group first widget is a question
  // When pressing ENTER in a question widget 
  // the following widget will be replaced by new_answer(question)
  // Each group widget is separated from the next one and the separation
  // may be dragged
  // If it is dragged to the bottom, then the group widget will be enlarged
  // and subsequent widgets will be repositionned
  // If it is dragged to the top, the group widget will be smaller
  class History_Fold : public Fl_Group {
  public:
    // members
    bool _folded,_folding;
    int _horig,_hsorig; // save size of History (and Scroll above) when folding
    int _bordersize; // size for the history number
    std::string autosave_filename;
    char * mode;
    Fl_Scroll * scroll;
    History_Pack * pack;
    Fl_Input * input;
    Fl_Group * group;
    Fl_Button * fold_button, * save_button, * close_button,* current_status;
    No_Focus_Button * help_button,*keyboard_button,*stop_button,*bnd_button,*msg_button;
    Fl_Menu_Button * mb; // mb=0 if foldable
    int scroll_position;
    int update_status_count;
    // Methods

    History_Fold(int X,int Y,int W,int H,int showmenu=0,const char*l=0);
    void autosave_rm(); // delete autosave file
    bool add_entry(int n); // insert a new group at position n (-1=at end)
    void add_entry(int n,Fl_Widget * question);
    bool remove_entry(int n); // remove a group at position n
    void resize(int,int,int,int); // resizes also pack horizontally
    virtual FL_EXPORT int handle(int);
    virtual FL_EXPORT void draw(); 
    void fold();
    void unfold();
    bool folded(){ return _folded; }
    bool reeval(int n); // reeval all question widgets starting at position n
    void resize_parent(int H);
    bool autosave(bool warn_user);
    bool close();
    void clear_modified();
    void eval();
    void update_status();
  };

  class Xcas_Tabs:public Fl_Tabs {
  public:
    bool up;
    Xcas_Tabs(int X,int Y,int W,int H,const char*l=0):Fl_Tabs(X,Y,W,H,l),up(true) {};
    virtual void resize(int X,int Y,int W,int H);    
  };

  class HScroll:public Fl_Scroll {
  public:
    bool resizechildren;
    HScroll(int X,int Y,int W,int H,const char*l=0):Fl_Scroll(X,Y,W,H,l),resizechildren(false) {};
    virtual void resize(int X,int Y,int W,int H);
    virtual void draw();
    void resize(); 
    // if HScroll contains a History_Pack, 
    // resize all its childrens to the same size
  };

  class DispG_Window:public Fl_Window {
  public:
    DispG_Window(int x,int y,int w,int h,const char * l=0):Fl_Window(x,y,w,h,l){}
    DispG_Window(int x,int y,const char * l=0):Fl_Window(x,y,l){}
    virtual int handle(int);
    virtual FL_EXPORT void draw();
  };

  // new_question widget creator
  Fl_Widget * new_question_multiline_input(int W,int H);
  Fl_Widget * new_question_equation(int W,int H);
  Fl_Widget * new_comment_input(int W,int H);
  Fl_Widget * new_question_editor(int W,int H);

  Fl_Widget * new_program(int W,int H,const History_Pack * pack);
  Fl_Widget * new_text_input(int W,int H);
  Fl_Widget * new_tableur(int W,int H,const History_Pack * pack);
  Fl_Group * new_figure(int W,int H,const History_Pack * pack,bool dim3=false,bool approx=true);
  Fl_Group * new_logo(int W,int H,const History_Pack * pack);

  void Comment_cb_eval(Fl_Multiline_Input * q , void*);

#endif // HAVE_LIBFLTK

#ifndef NO_NAMESPACE_XCAS
} // namespace xcas
#endif // ndef NO_NAMESPACE_XCAS

#endif // _HISTORY_H
