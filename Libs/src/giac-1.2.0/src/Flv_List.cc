//	======================================================================
//	File:    Flv_List.cxx - Flv_List implementation
//	Program: Flv_List - FLTK List Widget
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
// version 3 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.
//
// Please report all bugs and problems to "lcharlto@mail.coin.missouri.edu"
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

#include "Flv_List.H"
#include <FL/fl_draw.H>
#include <FL/Fl_Output.H>
#include <stdio.h>

#define DOcb(x) ((vcallback_when & (x))==(x))

//	Resizing constants
#define FUDGE 2
#define MOVE_X 1
#define MOVE_Y 2
#define MOVE_XY (MOVE_X|MOVE_Y)

static void vscrollbar_cb(Fl_Widget* o, void*)
{
	Flv_List *s = (Flv_List *)(o->parent());
	s->top_row( ((Fl_Scrollbar *)o)->value() );
	s->damage(FL_DAMAGE_CHILD);
//	s->select_start_row( ((Fl_Scrollbar *)o)->value() );
}

static void hscrollbar_cb(Fl_Widget* o, void*)
{
	Flv_List *s = (Flv_List *)(o->parent());
	s->row_offset( ((Fl_Scrollbar *)o)->value() );
}

Flv_List::Flv_List( int X, int Y, int W, int H, const char *l ) :
	Fl_Group(X,Y,W,H,l),
	scrollbar(0,0,0,0,0),
	hscrollbar(0,0,0,0,0)
{
	int r, rh;

	edit_row=-1;
#ifdef FLTK_2
	style(Fl_Output::default_style);
#else
	box(FL_THIN_DOWN_BOX);
#endif
	fl_font( text_font(), text_size() );
	fl_measure("X", r, rh );

//	Leave global_style & row_style undefined
//	get_default_style(global_style);

	if (parent())
		vdead_space_color = parent()->color();
	else
		vdead_space_color = FL_GRAY;

	scrollbar.callback(vscrollbar_cb);
	hscrollbar.callback(hscrollbar_cb);
	hscrollbar.type(FL_HORIZONTAL);

	vclicks = 0;
	vmax_clicks = 2;								//	Double click the most
	vcallback_when = 0xffff;				//	All Events
	veditor = NULL;
	vediting = false;
	vedit_when = FLV_EDIT_MANUAL;
  vwhy_event = 0;
	vfeature = FLVF_PERSIST_SELECT;
	vhas_scrollbars = FLVS_BOTH;
	vlast_row = 0;
	vrow = 0;
	vrow_offset = 0;
	vrow_width = 0;
	vrows = 0;
	vrows_per_page = 0;
	vscrollbar_width = 17;
	vselect_locked = true;
	vselect_row = 0;
	vtop_row = 0;

	end();									//	Don't put other widgets in this one.
}

Flv_List::~Flv_List()
{
	row_style.release();		//	Free any memory allocated
}

//================================================================
//	Virtual functions
//================================================================
void Flv_List::save_editor( Fl_Widget *, int , int )
{
}

void Flv_List::load_editor( Fl_Widget *, int , int )
{
}

void Flv_List::position_editor( Fl_Widget *e, int x, int y, int w, int h, Flv_Style & )
{
	e->resize( x, y, w, h );
}

//	Draw a single row
void Flv_List::draw_row( int Offset, int &X, int &Y, int &W, int &H, int R )
{
	Fl_Boxtype bt;
	Flv_Style s;

	get_style( s, R );						//	Guaranteed to fill in style
	if (Fl::focus()==this || persist_select())
		add_selection_style( s, R );	//	Add selection coloring if applicable
  if (row_divider())
  	s.border( s.border()|FLVB_BOTTOM );	//	Be sure bottom is on

	X -= Offset;
  
  draw_border(s,X,Y,W,H);
	bt = s.frame();

  fl_color( s.background() );
  fl_rectf(X,Y,W,H );
#ifdef FLTK_2
	bt->draw(X,Y,W,H,s.background());
	bt->inset( X, Y, W, H );
#else
	draw_box( bt, X, Y, W, H, s.background() );
	X+= (Fl::box_dx(bt));
	Y+= (Fl::box_dy(bt));
	W-= (Fl::box_dw(bt));
	H-= (Fl::box_dh(bt));
#endif
	if (R==row() && (Fl::focus()==this || persist_select()))
	{
		fl_color( fl_contrast(FL_BLACK,selection_color()) );
		fl_rect( X, Y, W, H);
	}
	X+=s.x_margin();
	Y+=s.y_margin();
	W-=s.x_margin()*2;
	H-=s.y_margin()*2;

	fl_font( s.font(), s.font_size() );
	if (!active())
		s.foreground( fl_inactive(s.foreground()) );
	fl_color( s.foreground() );
  X += Offset;
	if (R==-3)										//	Draw title
		fl_draw(label(), X, Y, W, H, s.align() );
}

//	Handle events
int Flv_List::handle(int event)
{
	int t, x, y;
	bool bs;

	bs = (vselect_row!=vrow);
	switch( event )
	{
		case FL_DEACTIVATE:
		case FL_HIDE:
		case FL_LEAVE:
		case FL_ENTER:
		case FL_ACTIVATE:
		case FL_SHOW:
			return 1;

		case FL_MOVE:
			check_cursor();
			Fl_Group::handle(event);
			return 1;
		case FL_FOCUS:
			Fl::focus(this);
			damage(FL_DAMAGE_CHILD);
			Fl_Group::handle(event);
			return 1;
		case FL_UNFOCUS:
			damage(FL_DAMAGE_CHILD);
			Fl_Group::handle(event);
			return 1;
		case FL_KEYBOARD:
			break;
		case FL_RELEASE:
			Fl_Group::handle(event);
			return 1;
		case FL_DRAG:
			if (check_resize())
				return 1;
		case FL_PUSH:
			Fl::focus(this);
			damage(FL_DAMAGE_CHILD);
			x = Fl::event_x();
			y = Fl::event_y();
			t = get_row( x, y );

			//	Row header clicked
			if (t<0)
			{
				vwhy_event = 0;
				switch( t )
				{
					case -3:
						if (DOcb(FLVEcb_TITLE_CLICKED))
							vwhy_event = FLVE_TITLE_CLICKED;
						break;
					case -2:
						if (DOcb(FLVEcb_ROW_FOOTER_CLICKED))
							vwhy_event = FLVE_ROW_FOOTER_CLICKED;
						break;
					case -1:
						if (DOcb(FLVEcb_ROW_HEADER_CLICKED))
							vwhy_event = FLVE_ROW_HEADER_CLICKED;
						break;
				}
				if (vwhy_event)
				{
					do_callback(this, user_data());
					vwhy_event = 0;
				}
				Fl_Group::handle(event);
				return 1;
			}
			row(t);

			if (!multi_select() ||
					(event==FL_PUSH && !Fl::event_state(FL_SHIFT)))
			{
				if (bs)
					vlast_row = vrow;
				if (vselect_row!=vrow)
					select_start_row( vrow );
			}
			if (event==FL_PUSH)
			{
				if (DOcb(FLVEcb_CLICKED))
				{
					vwhy_event = FLVE_CLICKED;
					do_callback(this, user_data());
					vwhy_event = 0;
				}
			}
			Fl_Group::handle(event);
			return 1;

		default:
			return Fl_Group::handle(event);
	}

	switch(Fl::event_key())
	{

		case FL_Up:
			if (Fl::event_state(FL_CTRL))
			{
				if (vrow>0)
					row(0);
			} else
				row(vrow-1);
			break;

		case FL_Down:
			if (Fl::event_state(FL_CTRL))
			{
				if (vrow<vrows-1)
					row( vrows-1 );
			} else
			{
				if (vrow<vrows-1)
					row (vrow+1);
			}
			break;

		case FL_Page_Down:
			if (Fl::event_state(FL_CTRL))
			{
				if (vrow<vrows-1)
					row(vrows-1);
			} else
			{
				if (vrow<vrows-1)
					row(vrow+page_size());
			}
			break;

		case FL_Page_Up:
			if (Fl::event_state(FL_CTRL))
			{
				if (vrow>0)
					row(0);
			} else
				row( vrow-page_size() );
			break;

		case FL_Right:
			row_offset(vrow_offset+10);
			break;

		case FL_Left:
			row_offset(vrow_offset-10);
			break;

		default:
			return Fl_Group::handle(event);
	}
	if (!multi_select())
	{
		if (bs)
			vlast_row = vrow;
		if (vselect_row!=vrow)
			select_start_row(vrow);
	}
	if (!Fl::event_state(FL_SHIFT) && vselect_row!=vrow)
		select_start_row(vrow);
	Fl_Group::handle(event);
	return 1;
}

int Flv_List::row_height( int r )
{
	int rh, x;
	Flv_Style *rows;

	if (!global_style.height_defined())
	{
		fl_font( text_font(), text_size() );
		fl_measure("X", x, rh );
	} else
		rh = global_style.height();
	if (r<0)
		rh += 4;
	rows = row_style.find(r);
	if (rows)
		if (rows->height_defined())
			rh = rows->height();
	return rh;
}

//	H is set to height of drawn row not including any grids
int Flv_List::row_height( int n, int r )
{
	if (r>-4 && r<rows())
		row_style[r].height(n);
	return row_height(r);
	//	Note: this is only the constant value, if row_height(r) is a calculated
	//	value, it's the only way we can really determine row height.
	//	Don't rely on the value of this function, ALWAYS call row_height(r)
}

int Flv_List::get_row( int x, int y ){
  int X, Y, W, H;
  int rh, rw, t, CY;
  
  at_end_row=false;
  // Determine if row was clicked and highlight it
  client_area(X,Y,W,H);
  if (label()){
    rh = row_height(-3);
    if (Y<=y && y<=Y+rh && x>=X && x<=X+W){
      return -3;
    }
    Y+=rh;
    H-=rh;
  }
  if (row_header()){
    rh = row_height(-1);
    if (Y<=y && y<=Y+rh && x>=X && x<=X+W){
      return -1;
    }
    Y+=rh;
    H-=rh;
  }
  if (row_footer()){
    rh = row_height(-2);
    if (Y+H>=y && y>Y+H-rh && x>=X && x<=X+W){
      return -2;
    }
    H -= rh;
  }
  rw = row_width();
  if (!rw)
    rw = W;
  if ( x<X || x>X+W || y<Y || y>=Y+H || x>X-vrow_offset+rw)
    return -4;				//	Out of bounds
  
  for (CY=Y, t=vtop_row;	t<vrows && CY<Y+H;	t++, CY+=rh ){
    rh = row_height(t);	
    if (CY<=y && y<=CY+rh){
      at_end_row=(y>=CY+(4*rh)/5);
      return t;
    }
  }
  return -4;		//	In grey area?
}

//	get trickle down style
void Flv_List::get_style(Flv_Style &s, int R, int )
{
	Flv_Style *rows;

	get_default_style(s);				//	Get default style information
	s = global_style;						//	Add global style information


	rows = row_style.skip_to(R);
	if (rows) s = *rows;				//	Add row style information
	if (R<0)										//	Headers/Labels have different default
	{                           //	Note: we can still override at cell level
		if (parent())
			s.background( parent()->color() );
		else
			s.background( FL_WHITE );
		s.frame(FL_THIN_UP_BOX);
		s.border( FLVB_NONE );
    s.border_spacing(0);
	}
	if (R==-3)									//	If title use label information
	{
		s.font(label_font());
		s.font_size(label_size());
		s.foreground( label_color() );
		s.align(FL_ALIGN_CLIP);
	}
	if (rows && R<0)
  {
		rows = rows->cell_style.skip_to(0);
	  if (rows && R<0)
  	  s = *rows;
  }
}

Flv_Feature Flv_List::feature(Flv_Feature v)
{
	if (v!=vfeature)
	{
		vfeature = v;
		vlast_row = vrow;					//	Redraw all!
		if (DOcb(FLVEcb_FEATURE_CHANGED))
		{
			vwhy_event = FLVE_FEATURE_CHANGED;
			do_callback(this, user_data());
			vwhy_event = 0;
		}
		damage(FL_DAMAGE_CHILD);	//	Because features are visible
	}
	return vfeature;
}

Flv_Feature Flv_List::feature_add(Flv_Feature v)
{
	if ( (vfeature&v)!=v )
	{
		vfeature |= v;
		if (DOcb(FLVEcb_FEATURE_CHANGED))
		{
			vwhy_event = FLVE_FEATURE_CHANGED;
			do_callback(this, user_data());
			vwhy_event = 0;
		}
		damage(FL_DAMAGE_CHILD);	//	Because features are visible
	}
	return vfeature;
}

Flv_Feature Flv_List::feature_remove(Flv_Feature v)
{
	if ( (vfeature&v)!=0 )
	{
		vfeature &= (Flv_Feature)(~v);
		if (DOcb(FLVEcb_FEATURE_CHANGED))
		{
			vwhy_event = FLVE_FEATURE_CHANGED;
			do_callback(this, user_data());
			vwhy_event = 0;
		}
		damage(FL_DAMAGE_CHILD);	//	Because features are visible
	}
	return vfeature;
}

Flv_ShowScrollbar Flv_List::has_scrollbar( Flv_ShowScrollbar v )
{
	if (v!=vhas_scrollbars)
	{
		vhas_scrollbars = v;
		damage(FL_DAMAGE_CHILD);
	}
	return vhas_scrollbars;
}

int Flv_List::edit_when( int v )
{
	if (v!=vedit_when)
	{
		vedit_when = v;
		if (vedit_when!=FLV_EDIT_ALWAYS)
			end_edit();
		else
			start_edit();
	}
	return vedit_when;
}

int Flv_List::bottom_row(void)
{
	int r, rh;
	int X, Y, W, H, B;

	client_area( X, Y, W, H );
	B = Y + H;

	for ( r=vtop_row; Y<B && r<vrows; r++, Y+=rh )
		rh = row_height(r);
	if (r==vrows)
		r = vrows-1;
	return r;
}

int Flv_List::row(int n)
{
	int X, Y, W, H;
	if (n>=vrows)
		n=vrows-1;
	if (n<0)
		n=0;
	if (n!=vrow)
	{
		vrow = n;
		client_area(X,Y,W,H);
		update_top_row(H);
		vlast_row = vrow;
		if (DOcb(FLVEcb_ROW_CHANGED))
		{
			vwhy_event = FLVE_ROW_CHANGED;
			do_callback(this, user_data());
			vwhy_event = 0;
		}
		damage(FL_DAMAGE_CHILD);
	}
	return vrow;
}

bool Flv_List::row_resizable( int r )					//	Get/Set row locked status
{
	Flv_Style s;
	get_style(s,r);
	return s.resizable();
}

bool Flv_List::row_resizable( bool n, int r )
{
	row_style[r].resizable(n);
	return n;
}

int Flv_List::row_offset( int n )
{
	if (n>vrow_width)
		n = vrow_width;
	if (n<0)
		n = 0;
	if (n!=vrow_offset)
	{
		vrow_offset = n;
		vlast_row = vrow;			//	Make sure we draw everything
		damage(FL_DAMAGE_CHILD);
	}
	return vrow_offset;
}

int Flv_List::rows(int n)
{
	if (n>=0 && n!=vrows)
	{
		vrows = n;
		if (vrow>=vrows)
			row(vrows-1);
		if (vselect_row>vrow)
			select_start_row(vrow);
		if (DOcb(FLVEcb_ROW_CHANGED))
		{
			vwhy_event = FLVE_ROW_CHANGED;
			do_callback(this, user_data());
			vwhy_event = 0;
		}
		damage(FL_DAMAGE_CHILD);
	}
	return vrows;
}

int Flv_List::rows_per_page( int n )
{
	if (n!=vrows_per_page && n>=0)
		vrows_per_page = n;
	return vrows_per_page;
}

bool Flv_List::row_selected( int n )
{
	if (vselect_row<vrow)
		return (vselect_row<=n && n<=vrow);
	else
		return (vrow<=n && n<=vselect_row);
}

int Flv_List::row_width( int n )
{
	if (n>=0 && n!=vrow_width)
	{
		vrow_width = n;
		damage(FL_DAMAGE_CHILD);
	}
	return vrow_width;
}

int Flv_List::scrollbar_width(int n)
{
	if (n!=vscrollbar_width && n>0)
	{
		vscrollbar_width = n;
		damage(FL_DAMAGE_CHILD);
	}
	return vscrollbar_width;
}

int Flv_List::select_start_row(int n)			//	Set first selected row
{
	if (n>=vrows)	n=vrows-1;
	if (n<0) n=0;
	if (n!=vselect_row)
	{
		vselect_row = n;
		vlast_row = vrow;
		if (DOcb(FLVEcb_SELECTION_CHANGED))
		{
			vwhy_event = FLVE_SELECTION_CHANGED;
			do_callback(this, user_data());
			vwhy_event = 0;
		}
		damage(FL_DAMAGE_CHILD);
	}
	return vselect_row;
}

//================================================================
//	Protected functions
//================================================================
void Flv_List::client_area( int &X, int &Y, int &W, int &H )
{
	char sv=0, sh=0;
	int th, v, rw;

	X = x(); Y = y(); W = w(); H = h();
#ifdef FLTK_2
	box()->inset(X,Y,W,H);
#else
	Fl_Boxtype b = box();
	X += Fl::box_dx(b);
	Y += Fl::box_dy(b);
	W -= Fl::box_dw(b);
	H -= Fl::box_dh(b);
#endif

	rw = vrow_width;
	if (rw==0) rw = W;

	if ( (vhas_scrollbars & FLVS_HORIZONTAL_ALWAYS)==FLVS_HORIZONTAL_ALWAYS)
		sh = 1;
	else if ( (vhas_scrollbars & FLVS_HORIZONTAL)==FLVS_HORIZONTAL )
	{
		if (vrow_width!=0)
		{
			if (rw>W)
				sh=1;
			else if (rw>W-vscrollbar_width)
				sh=-1;	//	Turn on if rows won't fit
		}
	}
	//	We need total height
	if ( (vhas_scrollbars & FLVS_VERTICAL_ALWAYS)==FLVS_VERTICAL_ALWAYS )
		sv = 1;
	else if ( (vhas_scrollbars & FLVS_VERTICAL)==FLVS_VERTICAL )
	{
		th = (label()?row_height(-3):0);
		th += (row_header()?row_height(-1):0);
		th += (row_footer()?row_height(-2):0);
		for (v=0;	th<=H && v<rows();	v++ )
			th += row_height(v);
		if ( th>H)
			sv = 1;
		else if ( th > H-vscrollbar_width )
			sv = -1;	//	Turn on if width in region
	}
	if (sh<0 && sv>0) sh=1;	// Extends to scrollbar obscured region so turn on
	if (sv<0 && sh>0) sv=1;	// Extends to scrollbar obscured region so turn on

	if (sv>0) W-=vscrollbar_width;
	if (sh>0) H-=vscrollbar_width;
}

void Flv_List::draw_border(Flv_Style &s, int &X, int &Y, int &W, int &H )
{
	int t;

  //	Draw outer border if defined
	fl_color( s.border_color() );
	if (s.left_border())
  	fl_yxline(X, Y, Y+H-1);
  if (s.right_border())
  	fl_yxline(X+W-1, Y, Y+H-1 );
  if (s.left_border())
  {
  	X++;
    W--;
  }
  if (s.right_border())
  	W--;

  if (s.top_border())
		fl_xyline(X, Y, X+W-1 );
  if (s.bottom_border())
  	fl_xyline(X, Y+H-1, X+W-1 );
	if (s.top_border())
  {
  	Y++;
    H--;
  }
  if (s.bottom_border())
  	H--;

  //	Draw spacing between borders
	fl_color( color() );
  for (t=0;	t<s.border_spacing();	t++ )
  {

		fl_rect( X, Y, W, H );
    if (s.left_border())
    {
	    X++;
      W--;
		}
    if (s.right_border())
    	W--;
		if (s.top_border())
    {
	    Y++;
      H--;
    }
    if (s.bottom_border())
    	H--;
  }

  //	Draw inner border if defined
	fl_color( s.border_color() );
	if (s.inner_left_border())
  	fl_yxline(X, Y, Y+H-1);
  if (s.inner_right_border())
  	fl_yxline(X+W-1, Y, Y+H-1 );
  if (s.inner_left_border())
  {
  	X++;
		W--;
  }
  if (s.inner_right_border())
		W--;

  if (s.inner_top_border())
  	fl_xyline(X, Y, X+W-1 );
  if (s.inner_bottom_border())
  	fl_xyline(X, Y+H-1, X+W-1 );
  if (s.inner_top_border())
  {
  	Y++;
    H--;
  }
  if (s.inner_bottom_border())
  	H--;
}

//	Determine if scrollbars are visible/position and draw
//	them if nessasary, also update X,Y,W,H to be inside box
void Flv_List::draw_scrollbars(int &X, int &Y, int &W, int &H )
{
	char sv=0, sh=0;
	int th, x, rw;


	rw = vrow_width;
	if (rw==0) rw = W;

	if ( (vhas_scrollbars & FLVS_HORIZONTAL_ALWAYS)==FLVS_HORIZONTAL_ALWAYS)
		sh = 1;
	else if ( (vhas_scrollbars & FLVS_HORIZONTAL)==FLVS_HORIZONTAL )
	{
		if (vrow_width!=0)
		{
			if (rw>W)
				sh=1;
			else if (rw>W-vscrollbar_width)
				sh=-1;	//	Turn on if rows won't fit
		}
	}
	//	We need total height
	if ( (vhas_scrollbars & FLVS_VERTICAL_ALWAYS)==FLVS_VERTICAL_ALWAYS )
		sv = 1;
	else if ( (vhas_scrollbars & FLVS_VERTICAL)==FLVS_VERTICAL )
	{
		th = (label()?row_height(-3):0);
		th += (row_header()?row_height(-1):0);
		th += (row_footer()?row_height(-2):0);
		for (x=0;	th<=H && x<rows();	x++ )
			th += row_height(x);
		if ( th>H)
			sv = 1;
		else if ( th > H-vscrollbar_width )
			sv = -1;	//	Turn on if width in region
	}
	if (sh<0 && sv>0) sh=1;	// Extends to scrollbar obscured region so turn on
	if (sv<0 && sh>0) sv=1;	// Extends to scrollbar obscured region so turn on

	if (sv>0) W-=vscrollbar_width;
	if (sh>0) H-=vscrollbar_width;

	//	Place scrollbars where they should be
	if (sv>0)
	{
		scrollbar.damage_resize(X+W,Y,vscrollbar_width,H);
		scrollbar.value( top_row(), page_size()+1,	0, vrows );	//	Fake out page size
		scrollbar.linesize( 1 );
		scrollbar.minimum(0);
		scrollbar.maximum(vrows-1);
		x = H - (vrows*2) - vscrollbar_width*2;
		if (x<vscrollbar_width) x = vscrollbar_width;
#ifdef FLTK_2
		scrollbar.slider_size(x);
#else
		scrollbar.slider_size((double)((double)x/(double)(H-vscrollbar_width*2)));
#endif
		scrollbar.Fl_Valuator::value( top_row() );	//	, 1,	0, vrows );
		if (!scrollbar.visible())
			scrollbar.set_visible();
		draw_child(scrollbar);
	} else
		scrollbar.clear_visible();


	if (rw-vrow_offset<W && rw>W)
	{
		vrow_offset = rw - W;
		vlast_row = vrow;	//	Make sure we update the entire widget
	}

	if (sh>0)
	{
		hscrollbar.damage_resize(X,Y+H,W,vscrollbar_width);
		hscrollbar.value( vrow_offset, 50,	0, vrow_width );	//	Fake out page size
		hscrollbar.linesize( 10 );
		hscrollbar.minimum(0);
		hscrollbar.maximum(vrow_width-W);
#ifdef FLTK_2
		x = W - vrow_width/10 - vscrollbar_width*2;
		if (x<vscrollbar_width) x = vscrollbar_width;
		hscrollbar.slider_size(x);
#else
		x = vrow_width / 10;
		hscrollbar.slider_size( (double)((double)x/(double)(W-vscrollbar_width*2)));
#endif
		hscrollbar.Fl_Valuator::value( vrow_offset );	//	, 1,	0, vrows );
		if (!hscrollbar.visible())
			hscrollbar.set_visible();
		draw_child(hscrollbar);
	} else
		hscrollbar.clear_visible();

	if (sh>0 && sv>0)	//	Draw box at ends
	{
		if (parent())
			fl_color( parent()->color() );
		else
			fl_color( FL_WHITE );
		fl_rectf( X+W, Y+H, vscrollbar_width, vscrollbar_width );
	}
}

void Flv_List::update_top_row(int H)
{
	int rh, r;

	if (vrow<vtop_row)
	{
		vtop_row = vrow;
		vlast_row = vrow;	//	Make sure we update the entire widget
	} else
	{
		if (label())
			H -= row_height(-3);
		if (row_header())
			H -= row_height(-1);
		if (row_footer())
			H -= row_height(-2);
		for (r=vtop_row;	r<=vrow && r<vrows;	r++, H-=rh )
		{
			rh = row_height(r);
			if (rh>H)
				break;
		}
		if (r<=vrow)		//	Move top row down, so we can see current line
		{
			vlast_row = vrow;	//	Make sure we update the entire widget
			for( ;	r<=vrow;	r++, H -= rh )
			{
				rh = row_height(r);
				vtop_row++;
			}
		}
		//	If there're visible rows below the current row
//		if (rh<H)
//		{
//			//	Finish computing visible rows
//			for (;	r<vrows;	r++, H-=rh )
//			{
//				rh = row_height(r);
//				if (rh>H)
//					break;
//			}
//		}
//		//	Do we need to move the top row up?
//		if (rh<H)
//		{
//			//	Too much space at bottom, we need to move top row toward 0
//			//	If possible.
//			vlast_row = vrow;	//	Make sure we update the entire widget
//			for (; vtop_row>0; vtop_row--, H -= rh )
//			{
//				rh = row_height(vtop_row);
//				if (rh>H)
//					break;
//			}
//		}
	}
}

void Flv_List::start_draw(int &X, int &Y, int &W, int &H, int &trow_width )
{
	int rh, CX, CY, CW, CH;
	label_type(FL_NO_LABEL);

	if (damage()&FL_DAMAGE_ALL)
	{
#ifdef FLTK_2
		draw_frame();
#else
		draw_box();
#endif
	}


	//	Get dimension inside box
	X = x(); Y = y(); W = w(); H = h();
#ifdef FLTK_2
	box()->inset(X,Y,W,H);
#else
	Fl_Boxtype b = box();
	X += Fl::box_dx(b);
	Y += Fl::box_dy(b);
	W -= Fl::box_dw(b);
	H -= Fl::box_dh(b);
#endif

	draw_scrollbars( X, Y, W, H );	// Place/set values, update bounding box

	trow_width = vrow_width;
	if (!trow_width)
		trow_width = W;

	//	Update top row if nessasary
//	update_top_row(H);

	//	Draw Title
	if (label())
	{
		rh = row_height(-3);
		fl_clip( X, Y, W, rh );
    CX=X;	CY=Y;	CW=W;	CH=rh;
		Flv_List::draw_row( 0, CX, CY, CW, CH, -3);
		fl_pop_clip();
		Y += rh;
		H -= rh;
	}

	//	Draw header if visible
	if (row_header())
	{
		rh = row_height(-1);
		fl_clip( X, Y, W, rh );
    CX=X;	CY=Y;	CW=trow_width;	CH=rh;
		draw_row( vrow_offset, CX, CY, CW, CH, -1);
		fl_pop_clip();
		Y += rh;
		H -= rh;
	}
	//	Draw footer
	if (row_footer())
	{
		rh = row_height(-2);
		H -=rh;
		fl_clip( X, Y+H, W, rh );
    CX=X;	CY=Y+H;	CW=trow_width;	CH=rh;
		draw_row( vrow_offset, CX, CY, CW, CH, -2 );
		fl_pop_clip();
	}
}

//	Perform actual draw function
void Flv_List::draw()
{
	int r, rh, rw;
	int X, Y, W, H, B;
	int CX, CY, CW, CH;
	Flv_Style s;

	//	Initially verify we aren't on a locked cell
	r = row();
	while(!select_locked())
	{
		get_style(s,r);
		if (!s.locked())
		{
			row(r);
			break;
		}
		r++;
		if (r==rows())
			break;
	}
	//	Make sure we have an editor if editing!
	if (vediting && !veditor)
		switch_editor(row());

	start_draw(X,Y,W,H,rw);

	B = W-(rw-vrow_offset);
	//	Fill-in area at right of list
	if (B>0)
	{
		fl_color( dead_space_color() );
    CY = Y;	CH = H;
    if (row_header())
    {
    	CY -= row_height(-1);
      CH += row_height(-1);
    }
    if (row_footer())
    	CH += row_height(-2);
		fl_rectf( X+rw-vrow_offset, CY, B, CH );
	}

	B = Y + H;
	fl_clip( X, Y, W, H );
	//	Draw rows
	for (	r=vtop_row;	Y<B && r<vrows;	r++, Y+=rh )
	{
		rh = row_height(r);
		if ( vlast_row==vrow || (vlast_row!=vrow && (r==vlast_row || r==vrow)) )
		{
			fl_clip( X, Y, rw, rh);
	    CX=X;	CY=Y;	CW=rw;	CH=rh;
			draw_row( vrow_offset, CX, CY, CW, CH, r );
			fl_pop_clip();
		}
	}
	vlast_row = vrow;

	//	Fill-in area at bottom of list
	if (Y<B)
	{
		fl_color( dead_space_color() );
		fl_rectf( X, Y, W, B-Y );
	}
	fl_pop_clip();
}

int Flv_List::page_size(void)
{
	int ps, H;

	if (vrows_per_page)
		ps = vrows_per_page;
	else
	{
		H = h() - 2 * vscrollbar_width;
		ps=11;
		if (H)
			ps=(H/row_height(0));
		ps--;
		if (ps<1)
		  ps=1;
	}
	return ps;
}

bool Flv_List::move_row( int amount )
{
	int r = row();
	Flv_Style s;

	r += amount;
	if (r>=rows())
		r = rows()-1;
	if (r<0)
		r = 0;

	while(!select_locked())
	{
		get_style(s,r);
		if (!s.locked())
			break;
		r += (amount<0?-1:1);
		if ( r<0 || r>=rows() )
			return false;
	}
	if (r!=row())
	{
		row(r);
		return true;
	}
	return false;
}

void Flv_List::get_default_style( Flv_Style &s )
{
	int r, rh;

	//	Make sure EVERY feature is defined
	s.align(FL_ALIGN_LEFT);
#ifdef FLTK_2
	s.background(color());
#else
	s.background(FL_WHITE);
#endif
	s.border(FLVB_NONE);
	if (parent())
		s.border_color(parent()->color());
	else
		s.border_color(FL_WHITE);
	s.border_spacing(0);
	s.editor(NULL);								//	No editor
	s.font(text_font());
	s.font_size(text_size());
	s.foreground(text_color());
	s.frame(FL_FLAT_BOX);

	fl_font( text_font(), text_size() );
	fl_measure("X", r, rh );
	s.height(rh);
	s.locked(true);
	s.resizable(false);
	s.width(40);
  s.x_margin(2);
  s.y_margin(1);
}

void Flv_List::add_selection_style( Flv_Style &s, int R, int  )
{
	if (!multi_select())		//	If not multi row selection
		select_start_row( row() );

	//	Handle row selection
	if (row_selected(R))
	{
			s.background( selection_color() );
			s.foreground( fl_contrast( text_color(), selection_color() ) );
	}
}

static Fl_Cursor last_cursor = FL_CURSOR_DEFAULT;
static int drag_row=-4, anchor_top;

bool Flv_List::check_resize(void)
{
	int ey, h;
	bool w=false;

	ey = Fl::event_y();

	if (drag_row>-4)
	{
		if (drag_row==-2)
		{
			h = anchor_top - ey + row_height(drag_row);
			if (h>1)
			{
				row_style[drag_row].height(h);
				damage(FL_DAMAGE_CHILD);
				anchor_top = ey;
				w = true;
			}
		} else
		{
			h = ey-anchor_top;
			if (h<1) h=1;
			row_style[drag_row].height(h);
			damage(FL_DAMAGE_CHILD);
			w=true;
		}
	}
	return w;
}

//	See if we can resize, if so change cursor
void Flv_List::check_cursor(void)
{
	int X, Y, W, H, ey, move=0, WW, size;
	int v;
	Fl_Cursor cursor;

	ey = Fl::event_y();
	client_area(X,Y,W,H);

	if (label() && *label())
	{
		Y+=row_height(-3);
		H-=row_height(-3);
	}

//	if (full_resize())	//	Trival test first
	{
		if (row_header())
		{
			size = row_height(-1);
			if (ey>=Y+size-FUDGE && ey<=Y+size+FUDGE)
			{
				if (row_resizable(-1))
				{
					drag_row = -1;
					anchor_top = Y;
					move |= MOVE_Y;		//	Moving
				}
			}
			Y += size;
			H -= size;
		}
		if (row_footer())
		{
			size = row_height(-2);
			if (ey>=Y+H-size-FUDGE && ey<=Y+H-size+FUDGE)
			{
				if (row_resizable(-2))
				{
					drag_row = -2;
					anchor_top = ey;
					move |= MOVE_Y;		//	Moving
				}
			}
			H -= size;
		}

		if ((move & MOVE_Y)==0)
		{
			WW = Y;
			for (v=top_row();	v<rows();	WW+=size, v++ )
			{
				size = row_height(v);
				if (WW+size+FUDGE>=Y+H)
					break;
				if (ey>=WW+size-FUDGE && ey<=WW+size+FUDGE)
				{
					if (row_resizable(v))
					{
						drag_row = v;
						anchor_top = WW;
						move |= MOVE_Y;		//	Moving
					}
					break;
				}
			}
		}
	}

	switch( move )
	{
		case MOVE_Y:	cursor = FL_CURSOR_NS;			break;
		default:
			drag_row = -4;
			cursor = FL_CURSOR_DEFAULT;
			break;
	}
	if (cursor!=last_cursor)
	{
		fl_cursor(cursor,FL_BLACK,FL_WHITE);
		last_cursor = cursor;
	}
}

void Flv_List::end_edit(void)
{
	switch_editor(-1);
}

void Flv_List::start_edit(void)											//	Start editing
{
	if (!vediting)
	{
		vediting = true;
		switch_editor( row() );
	}
}

void Flv_List::cancel_edit(void)											//	Cancel editing
{
	if (veditor)
		veditor->hide();
	veditor = NULL;
	edit_row = -1;
	if (edit_when()!=FLV_EDIT_ALWAYS)
		vediting = false;
	switch_editor( row() );
}

void Flv_List::switch_editor( int nr )
{
	Flv_Style s;

	if (veditor)
	{
		if (edit_row>-1)
			save_editor( veditor, edit_row );
		edit_row = -1;
		veditor->hide();
		veditor = NULL;
	}
	if (edit_when()==FLV_EDIT_ALWAYS)
		vediting = true;
	if (nr && vediting )
	{
		get_style( s, nr );
		if (s.editor_defined() && !s.locked())
		{
			veditor = s.editor();
			if (veditor)
			{
				edit_row = nr;
				load_editor( veditor, nr );
				veditor->damage(FL_DAMAGE_ALL);
				veditor->hide();
				veditor->show();
				Fl::focus(veditor);
			}
		}
	}
	if (veditor && veditor->parent()!=this)
		veditor->parent(this);
}

#ifdef FLTK_2
//================================================================
//	Style stuff?
//================================================================
static void revert(Fl_Style* s) {
	s->selection_color = FL_BLUE_SELECTION_COLOR;
	s->selection_text_color = FL_WHITE;
	s->off_color = FL_BLACK;
	s->box = FL_THIN_DOWN_BOX;
	s->color = FL_GRAY_RAMP+1;
}

Fl_Style* Flv_List::default_style =
		new Fl_Named_Style("Browser", revert, &Flv_List::default_style);
#endif



#endif
