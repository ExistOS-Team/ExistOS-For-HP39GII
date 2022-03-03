//	*** TODO ***
//	Implement row styles, global styles
//	======================================================================
//	File:    Flv_List.h - Flv_List implementation
//	Program: Flv_List - FLTK Widget
//	Version: 0.1.0
//	Started: 11/21/99
//
//	Copyright (C) 1999 Laurence Charlton
//
//	Description:
//	Flv_List implements a scrollable list.  No data is stored
//	in the widget.  Supports headers/footers, natively supports a single
//	row height per list.	Row grids can be turned on and off.  Supports
//	no scroll bars as well as horizontal/vertical automatic or always
//	on scroll bars.
//	Uses absolute row references.
//
//	row -1 is defined as the row header
//	row -2 is defined as the row footer
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
// USA.
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

#ifndef FLV_LIST_H
#define FLV_LIST_H

#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Scrollbar.H>
#include <FL/Enumerations.H>
#include "Flv_Style.H"

#ifndef FLTK_2
#define text_color() FL_BLACK
#define text_font() FL_HELVETICA
#define text_size() 12


#define fl_contrast(x,y) FL_BLACK
#define selection_text_color() FL_WHITE
#define FL_DAMAGE_HIGHLIGHT FL_DAMAGE_CHILD
#define label_font labelfont
#define label_size labelsize
#define label_color labelcolor
#define label_type labeltype
#endif

#define FLV_ROW_HEADER -1
#define FLV_ROW_FOOTER -2
#define FLV_TITLE -3

#define SLIDER_WIDTH 17

typedef unsigned char Flv_ShowScrollbar;
typedef unsigned short Flv_Feature;

//	Call back events
#define FLVE_ROW_CHANGED        1
#define FLVE_ROWS_CHANGED       2
#define FLVE_SELECTION_CHANGED  3
#define FLVE_COL_CHANGED        4
#define FLVE_COLS_CHANGED       5
#define FLVE_FEATURE_CHANGED    6
#define FLVE_CLICKED						7
#define FLVE_ENTER_PRESSED      8
#define FLVE_ROW_HEADER_CLICKED 9
#define FLVE_ROW_FOOTER_CLICKED 10
#define FLVE_COL_HEADER_CLICKED 11
#define FLVE_COL_FOOTER_CLICKED 12
#define FLVE_LABEL_CLICKED			13
#define FLVE_TITLE_CLICKED			14
#define FLVE_ALL_CLICKED				15

//	Call back when flags
#define FLVEcb_ROW_CHANGED        0x0001
#define FLVEcb_ROWS_CHANGED       0x0002
#define FLVEcb_SELECTION_CHANGED  0x0004
#define FLVEcb_COL_CHANGED        0x0008
#define FLVEcb_COLS_CHANGED       0x0010
#define FLVEcb_FEATURE_CHANGED    0x0020
#define FLVEcb_CLICKED						0x0040
#define FLVEcb_ENTER_PRESSED      0x0080
#define FLVEcb_ROW_HEADER_CLICKED 0x0100
#define FLVEcb_ROW_FOOTER_CLICKED 0x0200
#define FLVEcb_COL_HEADER_CLICKED 0x0400
#define FLVEcb_COL_FOOTER_CLICKED 0x0800
#define FLVEcb_LABEL_CLICKED			0x1000
#define FLVEcb_TITLE_CLICKED			0x2000
#define FLVEcb_ALL_CLICKED				0x4000

//	Feature list
#define FLVF_NONE							0
#define FLVF_ROW_HEADER				1
#define FLVF_ROW_FOOTER				2
#define FLVF_ROW_DIVIDER			4
#define FLVF_COL_HEADER				8
#define FLVF_COL_FOOTER			 16
#define FLVF_COL_DIVIDER		 32
#define FLVF_MULTI_SELECT		 64
#define FLVF_ROW_SELECT			128
#define FLVF_PERSIST_SELECT	256
#define FLVF_FULL_RESIZE    512
#define FLVF_DIVIDERS			(FLVF_COL_DIVIDER|FLVF_ROW_DIVIDER)
#define FLVF_HEADERS			(FLVF_ROW_HEADER|FLVF_COL_HEADER)
#define FLVF_FOOTERS			(FLVF_ROW_FOOTER|FLVF_COL_FOOTER)
#define FLVF_ROW_ENDS			(FLVF_ROW_HEADER|FLVF_ROW_FOOTER)
#define FLVF_COL_ENDS			(FLVF_COL_HEADER|FLVF_COL_FOOTER)
#define FLVF_ALL_ROW			(FLVF_ROW_HEADER|FLVF_ROW_FOOTER|FLVF_ROW_DIVIDER)
#define FLVF_ALL_COL			(FLVF_COL_HEADER|FLVF_COL_FOOTER|FLVF_COL_DIVIDER)

//	Scroll bar visibility
#define FLVS_NONE	0
#define FLVS_HORIZONTAL	1
#define FLVS_VERTICAL		2
#define FLVS_BOTH				(FLVS_HORIZONTAL|FLVS_VERTICAL)

#define FLVS_AUTOMATIC	0
#define FLVS_ALWAYS			4

#define FLVS_HORIZONTAL_ALWAYS	(FLVS_HORIZONTAL|FLVS_ALWAYS)
#define FLVS_VERTICAL_ALWAYS		(FLVS_VERTICAL|FLVS_ALWAYS)
#define FLVS_BOTH_ALWAYS				(FLVS_BOTH|FLVS|FLVS_ALWAYS)

//	Edit when constants
#define FLV_EDIT_ALWAYS				1
#define FLV_EDIT_AUTOMATIC    2
#define FLV_EDIT_MANUAL		    3

class Flv_List : public Fl_Group{
public:
  Fl_Scrollbar scrollbar;		// Vertical scrollbar
  Fl_Scrollbar hscrollbar;	// Horizontal scrollbar
  
  Flv_List( int X, int Y, int W, int H, const char *l=0 );
  ~Flv_List();
  
  virtual int row_height( int n );
  virtual int row_height( int n, int R );
  
  virtual void get_style(Flv_Style &s, int R, int C=0);	// get trickle down style
  void add_selection_style( Flv_Style &s, int R, int C=0 ); // Add selection style
  
  virtual void save_editor( Fl_Widget *e, int R, int C=0 );// Save editor contents
  virtual void load_editor( Fl_Widget *e, int R, int C=0 );// Load editor contents
  virtual void position_editor( Fl_Widget *e, int x, int y, int w, int h, Flv_Style &s );
  
  int callback_when(void)  //	Get callback when
  {	return vcallback_when;	}
  int callback_when(int v) //	Set callback when
  {	return vcallback_when = v;	}
  int add_callback_when(int v) //	Add callback conditions
  {	return vcallback_when |= v;	}
  int clear_callback_when(int v) //	Clear callback conditions
  {	return vcallback_when &= ~v;	}
  bool callback_on(int v) //	See if callback should be done
  {	return (vcallback_when & v)==v;	}
  int edit_when(void) //	Get when to edit
  {	return vedit_when;	}
  int edit_when( int v ); //	Set when to edit
  void start_edit(void); //	Start editing
  void end_edit(void); //	End editing (w/save)
  void cancel_edit(void); //	Cancel editing (wo/save)
  Fl_Widget *editor(void) //	Return current editor
  {	return veditor;	}
  int bottom_row(void);	//	Return last partially visible row
  
  unsigned char clicks() //	# of clicks 1, 2, 3...
  {	return vclicks;	}
  void clear_clicks(void) //	Reset # of clicks
  {	vclicks = 0;	}
  unsigned char max_clicks(void) //	Get Max # of clicks
  {	return vmax_clicks;	}
  unsigned char max_clicks(unsigned char n)   //	Return max # of clicks
  {	return vmax_clicks=n;	}

  Fl_Color dead_space_color(void) const //	Get dead space color
  {	return vdead_space_color;	}
  Fl_Color dead_space_color(Fl_Color &n) //	Set dead space color
  {	return (vdead_space_color = n);	}

  Flv_Feature feature(void) //	Get features
  {	return vfeature;	}
  Flv_Feature feature(Flv_Feature v); // Set features
  Flv_Feature feature_add(Flv_Feature v); // Add (set of) feature(s)
  Flv_Feature feature_remove(Flv_Feature v); //	Remove (set of) feature(s)
  bool feature_test(Flv_Feature v) // Test if (set of) feature(s) on
  {	return ((vfeature&v)==v);	}

  // Convenience funtions to test if a feature is ON.
  bool row_footer(void) //	Row footer convenience function
  {	return feature_test(FLVF_ROW_FOOTER);	}
  bool row_header(void)	//	Row header convenience function
  {	return feature_test(FLVF_ROW_HEADER);	}
  bool row_divider(void) //	Row divider convenience function
  {	return feature_test(FLVF_ROW_DIVIDER);	}
  bool multi_select(void) //	Multi-select convenience function
  {	return feature_test(FLVF_MULTI_SELECT);	}
  bool persist_select(void) //	Persistant selection test
  {	return feature_test(FLVF_PERSIST_SELECT);	}
  bool full_resize(void) //	Full widget resize test
  {	return feature_test(FLVF_FULL_RESIZE);	}

  void get_default_style( Flv_Style &s ); // Fill in s with default style
  int get_row( int x, int y );	//	Get row from X value
  
  bool get_cell_bounds( int &X, int &Y, int &W, int &H, int R, int C=0 );

  Flv_ShowScrollbar has_scrollbar(void)	//	Get/set scrollbar visibility
  {	return vhas_scrollbars;	}
  Flv_ShowScrollbar has_scrollbar( Flv_ShowScrollbar v );

  bool move_row(int amount); //	Change row # by amount
  int row(void)	//	Get/set current row
  {	return vrow;	};
  int row(int n);

  bool row_resizable( int r );//	Get/Set row locked status
  bool row_resizable( bool n, int r );

  int row_offset(void)//	Get/Set current row offset
  {	return vrow_offset;	}
  int row_offset( int n );

  int rows(void) //	Get/Set total # of rows
  {	return vrows;	};
  int rows(int n);

  int rows_per_page(void) //	Get/set # of rows / page
  {	return vrows_per_page;	}
  int rows_per_page( int n ); //	Use 0 to allow calculating

  bool row_selected(int n); //	Is row selected?

  int row_width(void) //	Get/set total row width
  {	return vrow_width;	};
  int row_width( int n );

  int scrollbar_width(void)  //	Get/set scrollbar thickness
  {	return vscrollbar_width;	}
  int scrollbar_width(int n);

  bool select_locked(void) //	Allow selecting locked rows/cols?
  {	return vselect_locked;	}
  bool select_locked(bool n) //	Set whether allowing selection of 
  {	return (vselect_locked=n);	}   //		locked rows/cols

  int select_start_row(void) //	Get first selected row
  {	return vselect_row;	}
  int select_start_row(int n); //	Set first selected row

  int top_row(void) //	Get the current top row
  {	return vtop_row;	}
  int top_row(int v)
  {	return (vtop_row=v);	};//	Set the top row (Hint only)

  int why_event() //	Why was the event called?
  {	return vwhy_event;	}
  int why_event( int n ) //	Set why the event was called
  {	return (vwhy_event=n);	}

  //	Style routines
  Flv_Style global_style;		//	Global style
  Flv_Style_List row_style;	//	Row styles
protected:
  unsigned char vmax_clicks; //	Max clicks allowed
  unsigned char vclicks;
  int vedit_when; //	When does editing start
  int edit_row;
  void switch_editor( int nr );
  Fl_Widget *veditor; //	Current editor
  bool vediting; //	Are we editing now?
  int vwhy_event; //	Why was event called
  int vlast_row; //	Last row highlighted
  bool at_end_row; // Set by get_row() to true if event_y is near the end of row
  void update_top_row(int H);
  int handle(int event);
  void start_draw(int &X, int &Y, int &W, int &H, int &rw);
  virtual void draw_row( int Offset, int &X, int &Y, int &W, int &H, int R );
  void draw_border(Flv_Style &s, int &X, int &Y, int &W, int &H );
  void client_area( int &X, int &Y, int &W, int &H );
  void draw_scrollbars( int &X, int &Y, int &W, int &H );
  void draw();
  int page_size(void); //	Calculate useable page size
  
  int vrow; //	Current selected row

private:
#ifdef FLTK_2
  static Fl_Style* default_style;
#endif
  void check_cursor(void);   //	Check if resizing allowed here
  bool check_resize(void);   //	true if resizing
  int vcallback_when; //	When to call back
  Fl_Color vdead_space_color; // Dead space color
  Flv_ShowScrollbar vhas_scrollbars;	// determine scroll bar visibility
  Flv_Feature vfeature;	//	Features (Headers, footers, dividers)
  int vrow_offset; //	Current left offset for display
  int vrow_width; //	Total pixel width
  int vrows; //	Total # of rows to display
  int vrows_per_page;  //	# of rows to advance/page
  bool vselect_locked; //	Selected locked entries or skip?
  int vselect_row;  //	First selected row
  int vscrollbar_width;	//	"thickness" of scrollbar
  int vtop_row;	 //	Current top row
};

#endif
#endif
