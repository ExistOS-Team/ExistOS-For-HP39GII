// -*- compile-command: "g++ -I.. -I. -g -c Tableur.cc " -*-
#ifndef _TABLEUR_H
#define _TABLEUR_H

#ifndef IN_GIAC
#include <giac/first.h>
#else
#include "first.h"
#endif
/*
 *  Copyright (C) 2002,2014 B. Parisse, Institut Fourier, 38402 St Martin d'Heres
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

#ifndef IN_GIAC
#include <giac/gen.h>
#else
#include "gen.h"
#endif
#ifdef HAVE_LIBFLTK
#include "Flv_Table.H"
#include "Flve_Input.H"
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Double_Window.H>
#include <FL/fl_draw.H>
#include <FL/fl_message.H>
#endif
#include "Input.h"
#include "Graph.h"
#include "Graph3d.h"

#ifndef NO_NAMESPACE_XCAS
namespace xcas {
#endif // ndef NO_NAMESPACE_XCAS
#ifdef HAVE_LIBFLTK
  extern Fl_Menu_Item Tableur_menu[];
  class Flv_Table_Gen;
  bool iscell_range(const giac::gen & g,const giac::matrice & m,giac::matrice & mselect,Flv_Table_Gen * ptr);
  // Returns 0 if not a cell range, 1 if vc is made of successives integers, n if of n independant columns, set absolu to true if IDNT are used for cols
  int iscell_range(const giac::gen & g,const giac::matrice & m,giac::matrice & mselect,Flv_Table_Gen * sptr,int & r1,int & r2,std::vector<int> & vc,bool & absolu);

  bool tablefunc_dialog(Fl_Widget * spread_ptr,std::string & arg,bool plot,int type,const std::string & title);
  bool tableseq_dialog(Fl_Widget * spread_ptr,std::string & arg,bool plot,const std::string & title,std::string & u0param);

  // Should be inside a Tableur Group
  class Flv_Table_Gen : public Flv_Table{
  public:
    static int def_rows,def_cols;
    giac::matrice m;
    giac::vecteur m_history;
    int max_history,cur_history;
    giac::matrice selected,selected_1;
    giac::gen name,init; // sheet evaluation saves the matrix in name
    bool is_spreadsheet,matrix_fill_cells;
    bool move_right;
    bool changed_;
    int sort_col,matrix_symmetry,spreadsheet_recompute;
    int push_row,push_col;
    int prev_row,prev_col;
    giac::context * contextptr;
    bool editing,computing;
    int edit_row,edit_col;
    Xcas_Text_Editor * input;
    Multiline_Input_tab* _goto;
    Graph2d * graph,*graph2d;
    Graph3d * graph3d;
    Fl_Menu_Button * mb;
    Fl_Window * win2,*win3;
    std::string status_string;
    std::string * filename,prefix_filename;
    unsigned max_printsize;
    int header_event,last_event;
    // if printing cell requires more than max_printsize, cell is not displayed
    virtual int handle(int event);
    void finish_flv_table_gen();
    void update_name();
    void config();
  Flv_Table_Gen(int X,int Y,int W,int H,const char *l=0):Flv_Table(X,Y,W,H,l),matrix_fill_cells(true),matrix_symmetry(0),spreadsheet_recompute(true),max_printsize(1024){ contextptr=0; computing=editing=false; graph=0;graph2d=0; graph3d=0; _goto=0;input=0; name=0; finish_flv_table_gen();};
    Flv_Table_Gen( int X, int Y, int W, int H,const giac::gen & g,const char *l=0 );
    Flv_Table_Gen( int X, int Y, int W, int H,const giac::matrice & mym,const char *l=0);
    ~Flv_Table_Gen() ;
    virtual void draw_cell(int Offset,int &X,int &Y,int &W,int &H,int R,int C );
    void enter_move();
    //	Required for editing
    //virtual void save_editor( Fl_Widget *e, int R, int C );
    //virtual void load_editor( Fl_Widget *e, int R, int C );
    //virtual void position_editor( Fl_Widget *e, int x, int y, int w, int h, Flv_Style &s );
    void spread_erase(int nrows,int ncols);
    void spread_insert(int nrows,int ncols);
    void copy(int clipboard=0);
    void blank();
    void blank(int row_min,int r,int col_min,int c);
    void erase_row_col(int i); // 2 ask, 1 col, 0 row
    void copy_right();
    void copy_down();
    void copy_first_in_selection();
    void paste(const giac::matrice & m,bool reeval=true);
    void addrowatend();
    void addcolatend();
    void set_matrix(const giac::matrice & m,bool interruptible=false,bool reeval=true);
    void set_matrix(const giac::gen & g,bool interruptible=false,bool reeval=true);
    void resizesheet(int nr,int nc);
    void spread_eval_interrupt();
    void update_spread_graph();
    int set_graphic_dialog(const std::string & title,const std::string & aide,giac::gen & in, giac::gen & out,bool show_class,bool & absolu,bool & transpose,double & degree);
    void set_graphic(const giac::gen & function,const std::string & aide);
    void update_status() ;
    void update_goto() ;
    void update_input() ;
    void backup();
    void restore(int i);
    void changed();
  };

  std::string tableur_insert(Flv_Table_Gen * tg);

  giac::gen Xcas_fltk_Row(const giac::gen & g,const giac::context * contextptr);
  giac::gen Xcas_fltk_Col(const giac::gen & g,const giac::context * contextptr);
  giac::gen Xcas_fltk_current_sheet(const giac::gen & g,const giac::context * contextptr);

  class Tableur_Group : public Fl_Tile {
  public:
    int disposition; // 0, 1: graphe invisible, 2: portait, 3: landscape
    double dtable,dgraph,dparam;
    Flv_Table_Gen * table;
    Fl_Menu_Bar * menubar;
    Fl_Group * reevalsave;
    Fl_Button * fname;
    Fl_Box * borderbox;
    Tableur_Group(int X,int Y,int W,int H,int L,int disp=2);
    virtual void resize2(double dt=0,double dg=0,double dp=0);
    virtual int handle(int event);
    void save_dparam();
  };

  // Insure that all elements of m that are vectors have length <= 3
  void spread_ck(giac::matrice & m);

  struct xcas_matrix {
    giac::matrice m;
    bool changed,is_spreadsheet;
    int rows,cols,row,row_end,col,col_end;
    xcas_matrix(const giac::matrice & mym,bool b,bool is_sp,int nr,int nc,int rs,int re,int cs,int ce): m(makefreematrice(mym)),changed(b),is_spreadsheet(is_sp),rows(nr),cols(nc),row(rs),row_end(re),col(cs),col_end(ce) {}
    xcas_matrix(): changed(false),is_spreadsheet(true),rows(50),cols(10),row(0),row_end(0),col(0),col_end(0) { m=makefreematrice(giac::vecteur(50,giac::vecteur(10,giac::zero))); makespreadsheetmatrice(m,giac::context0); }
  };

  struct editor_string {
    std::string s; 
    bool changed; 
    int pos1,pos2; 
    editor_string(const std::string & mys,bool b,int i,int j):s(mys),changed(b),pos1(i),pos2(j) {}
    editor_string():s(""),changed(false),pos1(0),pos2(0) {}
  };
#endif
#ifndef NO_NAMESPACE_XCAS
} // namespace xcas
#endif // ndef NO_NAMESPACE_XCAS

#endif // _TABLEUR_H
