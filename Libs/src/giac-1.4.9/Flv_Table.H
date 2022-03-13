//	======================================================================
//	File:    Flv_Table.h - Flv_Table implementation
//	Program: Flv_Table - FLTK Widget
//	Version: 0.1.0
//	Started: 11/21/99
//
//	Copyright (C) 1999 Laurence Charlton
//
//	Description:
//	Flv_Table implements a table/grid.  No data is stored
//	in the widget.  Supports headers/footers for rows and columns,
//	natively supports a single row height and column width per table.
//	Row and column grids can be turned on and off.  Supports no scroll
//	bars as well as horizontal/vertical automatic or always on scroll bars.
//	Also support cell selection and row selection modes.  In row selection
//	mode it acts like a pumped-up list widget.
//
//	row -1 is defined as the row header
//	row -2 is defined as the row footer
//	column -1 is defined as the column header
//	column -2 is defined as the column footer
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.
//	======================================================================

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifndef IN_GIAC
#include <giac/first.h>
#else
#include "first.h"
#endif
#ifdef HAVE_LIBFLTK

#ifndef FLV_TABLE_H
#define FLV_TABLE_H

#include "Flv_List.H"

#define FLV_COL_HEADER -1
#define FLV_COL_FOOTER -2

#define FLV_MOVE_ON_ENTER_ROW_COL   1
#define FLV_MOVE_ON_ENTER_COL_ROW   2
#define FLV_MOVE_ON_ENTER_NEVER     3

#define FLV_BUTTON1 1
#define FLV_BUTTON2 2
#define FLV_BUTTON3 4


class Flv_Table : public Flv_List{
public:
  Flv_Table( int X, int Y, int W, int H, const char *l=0 );
  ~Flv_Table();
  
  virtual int col_width(int c);	 //  Get column width
  virtual int col_width(int n, int c);	// Set column width

  //	Add selection info if applicable
  void add_selection_style( Flv_Style &s, int R, int C=0 );

  int buttons(void) {	return vbuttons;	}
  int buttons(int v) {	return (vbuttons=v);	}
		
  void cell_area( int &X, int &Y, int &W, int &H);

  bool cell_selected(int R, int C);  //	Is cell selected?

  //	Convenience functions to tell if a feature is ON.
  bool col_footer(void)	  // Column footer convenience function
  {	return (Flv_Feature)(feature() & FLVF_COL_FOOTER)!=0;	}
  bool col_header(void)  // Column header convenience function
  {	return (Flv_Feature)(feature() & FLVF_COL_HEADER)!=0;	}
  bool col_divider(void)	//	Column divider convenience function
  {	return (Flv_Feature)(feature() & FLVF_COL_DIVIDER)!=0;	}
  bool select_row(void)		//	Selecting row convenience function
  {	return feature_test(FLVF_ROW_SELECT);	}

  //	These are guaranteed style retrieval functions and get
  //	the trickle down style information.  Any style elements
  //	not defined are set to default values.
  virtual void get_style( Flv_Style &s, int R, int C=0 );
  int edit_when(void)	//	Get when to edit
  {	return vedit_when;	}
  int edit_when( int v );		//	When to edit
  
  bool move_row(int amount);  //	# of rows to move
  bool move_col(int amount);  //	# of cols to move
  int col(void)		//	Get current column #
  {	return vcol;	}
  int col( int n );	//	Set current column #
  int row(void)	//	Get current row #
  {	return vrow;	}
  int row(int n);	//	Set current row #

  bool col_resizable(int c);	//	Get/set column locked status
  bool col_resizable( bool n, int c);

  int cols(void)  //	Get number of columns
  {	return vcols;	}
  int cols( int n );	//	Set number of columns

  bool col_selected(int n);	//	Is column selected

  bool get_cell_bounds( int &X, int &Y, int &W, int &H, int R, int C );
	
  int get_col( int x, int y );	  //	Get column from x,y

  int move_on_enter(void) const	  //	How do we move when enter pressed?
  {	return vmove_on_enter;	}
  int move_on_enter(int v)//	Set how we move on enter
  {	return (vmove_on_enter=v);	}

  int select_start_col(void)	//	Get column selection starts in
  {	return vselect_col;	}
  int select_start_col(int n);	//	Set column selection starts in

  void start_edit(void);	//	Start editing
  void end_edit(void);	//	End editing (with save)
  void cancel_edit(void);	//	Cancel editing (no save)
	
  Flv_Style_List col_style;	//	Column styles

protected:
  int edit_col;
  bool click_fill; 
  bool at_end_col; // this bool is set every time get_col is called
  void switch_editor( int nr, int nc );
  void draw(void);
  int handle(int event);
  int internal_handle(int event);
  virtual void draw_row( int Offset, int &X, int &Y, int &W, int &H, int R );
  virtual void draw_cell(int Offset, int &X, int &Y, int &W, int &H, int R, int C);

private:
  void check_cursor(void); //	Check if resizing allowed here
  bool check_resize(void); //	true if resizing
  void update_width();	//	Update scroll width
  void adjust_for_cell(); //	Guarantee cell is as visible as possible
  int vbuttons;	 // Click buttons
  int vcol;  //	Current column
  short vcol_width; //	Default width
  int vcols; //	Total # of columns
  int vmove_on_enter; //	How to move when enter pressed.
  int vselect_col; //	First column selected
};
#endif

#endif
