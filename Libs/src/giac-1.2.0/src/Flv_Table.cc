//	======================================================================
//	File:    Flv_Table.cxx - Flv_Table implementation
//	Program: Flv_Table - FLTK Table Widget
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
//	Uses absolute cell references.
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
#include "Flv_Table.H"
#include <FL/Enumerations.H>
#include <FL/fl_draw.H>
#include <stdio.h>

#define DOcb(x) ((callback_when() & (x))==(x))

//	Resizing constants
#define FUDGE 2
#define MOVE_X 1
#define MOVE_Y 2
#define MOVE_XY (MOVE_X|MOVE_Y)

#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

static Fl_Cursor last_cursor = FL_CURSOR_DEFAULT;
static int drag_col=-4, drag_row=-4, anchor_left, anchor_top;


Flv_Table::Flv_Table( int X, int Y, int W, int H, const char *l ) :Flv_List(X,Y,W,H,l){
  edit_col = -1;
  vcol = 0;
  vcols = 0;
  vcol_width = 40;
  vmove_on_enter = FLV_MOVE_ON_ENTER_COL_ROW;
  vselect_col = 0;
  vbuttons = FLV_BUTTON1 | FLV_BUTTON2 | FLV_BUTTON3;
  click_fill = false;
}

Flv_Table::~Flv_Table()
{
}

//	Probbably won't need to over-ride this for future table widgets
void Flv_Table::draw_row( int Offset, int &X, int &Y, int &, int &H, int R )
{
	int c, cw, CX, FW;
	int dX, dY, dW, dH;
	int TX, TY, TW, TH;
	Flv_Style s;

	//	Calculate clipping height
	client_area(dX,dY,dW,dH);

	FW = (col_footer()?col_width(-2):0);

	CX = X;

	//	Draw column header
	if (col_header())
	{
		cw = col_width(-1);		//	Column width
		TX = CX;	TY = Y;	TW = cw;	TH = H;
		draw_cell( 0, TX, TY, TW, TH, R, -1 );
		CX += cw;
		dX += cw;
		dW -= cw;
	}

	dW -= FW;
	//	Draw column footer
	if (FW)
	{
		TX = dX+dW;	TY = Y;	TW = FW;	TH = H;
		draw_cell( 0, TX, TY, TW, TH, R, -2 );
	}

	fl_clip( dX, Y, dW, H );			//	Clip data area
	for (c=0;	c<vcols && CX-Offset<dX+dW;	c++, CX+=cw )
	{
		cw = col_width(c);					//	Column width
		if (CX-Offset+cw<dX)				//	Before left
			continue;
		fl_clip( CX-Offset, Y, cw, H );
		TX = CX;	TY = Y;	TW = cw;	TH = H;
		draw_cell( Offset, TX, TY, TW, TH, R, c );
		fl_pop_clip();
	}
	//	If we're selecting a row, put the box around it.
	if (R==row() && select_row() )
	{
		fl_color( fl_contrast(FL_BLACK, selection_color()) );
		fl_rect( dX, Y, dW, H );
	}
	//	Fill-in area at right of list
	if (CX-Offset<dX+dW)
	{
		cw = dX+dW-(CX-Offset);
		fl_color( dead_space_color() );
		fl_rectf( CX-Offset, Y, cw, H );
	}
	fl_pop_clip();
}

//	You will certainly want to override this
void Flv_Table::draw_cell( int Offset, int &X, int &Y, int &W, int &H, int R, int C )
{
	Fl_Boxtype bt;
	Flv_Style s;

	X -= Offset;

	get_style(s, R, C);
	if (Fl::focus()==this || persist_select())
		add_selection_style(s, R, C);

	if (row_divider())
  	s.border( s.border()|FLVB_BOTTOM );
	if (col_divider())
  	s.border( s.border()|FLVB_RIGHT );

  draw_border( s, X, Y, W, H );
	bt = s.frame();

  fl_color( s.background() );
  fl_rectf(X,Y,W,H );
#ifdef FLTK_2
	bt->draw(X,Y,W,H,s.background());
	//	Normally you would use the next line to get the client area to draw
	bt->inset( X, Y, W, H );
#else
	draw_box( bt, X, Y, W, H, s.background() );
	//	Normally you would use the next lines to get the client area to draw
	X+= (Fl::box_dx(bt));
	Y+= (Fl::box_dy(bt));
	W-= (Fl::box_dw(bt));
	H-= (Fl::box_dh(bt));
#endif
		//	Drawing selection rectangle for cell
		if (R>-1 && C>-1 && R==row() && C==col() && !select_row() &&
				(Fl::focus()==this || persist_select()))
		{
			fl_color( fl_contrast(text_color(), selection_color()) );
			fl_rect( X, Y, W, H);
		}

	X+=s.x_margin();
	Y+=s.y_margin();
	W-=s.x_margin()*2;
	H-=s.y_margin()*2;
  X += Offset;
	//	Get set-up to draw text
	fl_font( s.font(), s.font_size() );
	if (!active())
		s.foreground( fl_inactive(s.foreground()) );
	fl_color(s.foreground());
}

bool Flv_Table::get_cell_bounds( int &X, int &Y, int &W, int &H, int R, int C )
{
	int x, y, w, h, r, rh, B, cx;

	X = Y = W = H = 0;
	cell_area(x,y,w,h);
	B = y+h;
		
	for (r=top_row();	r<rows() && r<R;	y += rh, r++ )
	{
		rh = row_height(r);
		if (y>B)
			break;
	}
	if (r!=R)
		return false;
	Y = y;
	H = row_height(R);
	if (Y+H>B)
		H = B-Y;

	cx = x - row_offset();
	for (r=0;	r<cols() && r<C;	cx += rh, r++ )
	{
		rh = col_width(r);
		if (cx>x+w)
			break;
	}
	rh = col_width(r);
	if (r!=C || cx+rh<x)
	{
		X = Y = W = H = 0;
		return false;
	}

	X = cx;
	if (X<x)
	{
		rh -= (x-X);
		X = x;
	}
	if (X+rh>x+w)
		rh = (x+w)-X;
	if (rh>w)
		rh = w;
	if (rh<0)
		rh = 0;
	W = rh;
	return true;
}

void Flv_Table::draw(void)
{
	int r, rh, rw;
	int X, Y, W, H, B, FW;
	int CX, CY, CW, CH;
	Flv_Style s;
	int t, c;
	char buf[30];

	//	Initially verify we aren't on a locked cell
	r = row();
	c = col();
	while(!select_locked())
	{
		get_style(s,r,c);
		if (!s.locked())
		{
			row(r);
			col(c);
			break;
		}
		c++;
		if (c==cols())
		{
			c = 0;
			r++;
			if (r==rows())
				break;
		}
	}

	//	Make sure we have an editor if editing!
//	if (!veditor && vediting)
//		switch_editor(row(),col());

	//	We need to know what the row width will be
	//	so we'll calculate that and then let normal drawing
	//	take over.
	if (!feature_test(FLVF_MULTI_SELECT))
		select_start_col(vcol);

	for (c=cols(), rw=t=0;	t<c;	t++ )
		rw += col_width(t);
	if (col_header())
		rw += col_width(-1);
	if (col_footer())
		rw += col_width(-2);

	row_width(rw);					//	Set the row width so we can draw intelligently

	start_draw(X,Y,W,H,rw);

	//	This is why draw is here and we're not using the code from
	//	Flv_List... It sucks, but I really didn't like the flickering
	//	from the column footers getting erased and then redrawn...
	FW = (col_footer()?col_width(-2):0);

	B = W-(rw-row_offset())-FW;
	//	Fill-in area at right of list
	if (B>0)
	{
		fl_color( dead_space_color() );
		fl_rectf( X+rw-row_offset(), Y, B, H );
	}

	B = Y + H;
	fl_clip( X, Y, W, H );
	//	Draw rows
	for (	r=top_row();	Y<B && r<rows();	r++, Y+=rh )
	{
		rh = row_height(r);
		if ( vlast_row==row() || (vlast_row!=row() && (r==vlast_row || r==row())) )
		{
			fl_clip( X, Y, W, rh);
	    CX=X;	CY=Y;	CW=rw;	CH=rh;
			draw_row( row_offset(), CX, CY, CW, CH, r );
			fl_pop_clip();
		}
	}
	vlast_row = row();

	//	Fill-in area at bottom of list
	if (Y<B)
	{
		if (parent())
			fl_color( parent()->color() );
		else
			fl_color( FL_WHITE );
		fl_rectf( X, Y, W, B-Y );
	}
	fl_pop_clip();
}

void Flv_Table::add_selection_style( Flv_Style &s, int R, int C )
{
	if (!multi_select())		//	If not multi row selection
	{
		select_start_row( row() );
		select_start_col( col() );
	}
	if (R>-1 && C>-1)
	if ( (select_row() && row_selected(R)) ||
			 (!select_row() && cell_selected(R,C)) )
	{
		s.background( selection_color() );
		s.foreground( selection_text_color());
	}
}

void Flv_Table::cell_area(int &X, int &Y, int &W, int &H )
{
	client_area(X,Y,W,H);
	if (label() && *label())
	{
		Y += row_height(FLV_TITLE);
		H -= row_height(FLV_TITLE);
	}

	if (row_header())
	{
		Y += row_height(FLV_ROW_HEADER);
		H -= row_height(FLV_ROW_HEADER);
	}
	if (row_footer())
	{
		H -= row_height(FLV_ROW_FOOTER);
	}
	if (col_header())
	{
		X += col_width(FLV_COL_HEADER);
		W -= col_width(FLV_COL_HEADER);
	}
	if (col_footer())
	{
		W -= col_width(FLV_COL_FOOTER);
	}
}

bool Flv_Table::cell_selected(int R, int C)
{
	return (col_selected(C) && row_selected(R));
}

int Flv_Table::col_width(int C)					//	Get column width
{
	int fw = vcol_width;
	Flv_Style *cols;

	if (global_style.width_defined())
		fw = global_style.width();
	cols = col_style.find(C);
	if (cols)
		if (cols->width_defined())
			fw = cols->width();
	return fw;
}

int Flv_Table::col_width(int n, int c)	//	Set column width
{
	int cw = col_width(c);

	if (c<-3)	c=-3;
	if (c>=vcols) c=vcols-1;
	if (n<0) n=0;
	if (n!=cw)
	{
		col_style[c].width(n);
		damage(FL_DAMAGE_CHILD);
	}
	return col_width(c);
}

void Flv_Table::get_style( Flv_Style &s, int R, int C )
{
	Flv_Style *rows, *cols, *cells;

	Flv_List::get_style( s, R );
	rows = row_style.skip_to(R);

	if (R!=-3)
	{
		cols = col_style.skip_to(C);
		if (cols) s = *cols;
	}

	if (C<0 || R<0)							//	Headers/Labels have different default
	{                           //	Note: we can still override at cell level
		if (parent())
			s.background( parent()->color() );
		else
			s.background( FL_WHITE );
		s.frame(FL_THIN_UP_BOX);
		s.border( FLVB_NONE );
		s.border_spacing(0);
	}

	cells = (rows?rows->cell_style.skip_to(C):NULL);
	if (cells)	s = *cells;
}

int Flv_Table::handle(int event)
{
	int stat=0, x, y, X,Y,W,H, r, c;
	Flv_Style s;

	switch(event)
	{
		case FL_RELEASE:
		case FL_DRAG:
			if (!vediting || !veditor)
				break;

		case FL_PUSH:
			if (Fl::event_button1()==0)
				break;
			if (drag_row!=-4 || drag_col!=-4)
				break;
			x = Fl::event_x();
			y = Fl::event_y();
			if (!vediting)
			{
				if (edit_when()==FLV_EDIT_MANUAL)
					break;
				r = row();
				c = col();
				if (r<0 || c<0)
					break;
				cell_area(X,Y,W,H);
				stat = internal_handle(event);
				if (stat && r==row() && c==col() && x>=X && x<X+W && y>=Y && y<Y+H )
				{
					start_edit();
					return 1;
				}
				return 0;
			}

			//	If these are occur outside the editor, we don't want the
			//	child widget processing them
			if (x<veditor->x() || y<veditor->y() || x>veditor->x()+veditor->w() ||
						y>veditor->y()+veditor->h())
				break;
			stat = veditor->handle(event);
			if (stat)
			{
				veditor->draw();
				return 1;
			}
			break;
	}

	if (event==FL_SHORTCUT && vediting)
	{
		if (Fl::event_key()==FL_Enter)
		{
			end_edit();													//	Save editor/ quit editing
			Fl::focus(this);
//			take_focus();
			internal_handle(FL_KEYBOARD);
			damage(FL_DAMAGE_CHILD);
			return 1;
		}
		switch( Fl::event_key() )
		{
			case FL_Shift_L:
			case FL_Shift_R:
			case FL_Control_L:
			case FL_Control_R:
			case FL_Meta_L:
			case FL_Meta_R:
			case FL_Alt_L:
			case FL_Alt_R:
				break;
			default:
				stat = internal_handle(FL_KEYBOARD);
		}
	} else
	{
		stat = internal_handle(event);
		if (!stat)
		{
			//	Jump start editing if automatic
			if (event==FL_KEYBOARD && !vediting && edit_when()==FLV_EDIT_AUTOMATIC )
			{
				switch( Fl::event_key() )
				{
					case FL_Shift_L:
					case FL_Shift_R:
					case FL_Control_L:
					case FL_Control_R:
					case FL_Meta_L:
					case FL_Meta_R:
					case FL_Alt_L:
					case FL_Alt_R:
						break;
					default:
						start_edit();
						if (veditor)
						{
							stat = veditor->handle(event);
							if (stat)
							{
								Fl::focus(veditor);
//								veditor->take_focus();
								veditor->draw();
								return 1;
							}
						}
						cancel_edit();
				}
			}
		}
	}

	if (veditor && Fl::focus()==this)
	{
		Fl::focus(veditor);
//		veditor->take_focus();
		veditor->handle(FL_FOCUS);
	}
	if (stat && veditor)
		veditor->draw();
	return stat;
}

int Flv_Table::internal_handle(int event)
{
	int TX, TY, r, c, cd, rd;
	Flv_Style s;
	static int LX, LY;


	switch( event )
	{
		case FL_KEYBOARD:
			break;

		case FL_ENTER:
		case FL_LEAVE:
			vclicks = 0;
			return Fl_Group::handle(event);
			
		case FL_FOCUS:
		case FL_UNFOCUS:
			return 1;

		case FL_MOVE:
			TY = Fl::event_y();
			TX = Fl::event_x();
			if ( LX-TX<-3 || LX-TX>3 || LY-TY<-3 || LY-TY>3 )
			{
				LX = TX;
				LY = TY;
				vclicks = 0;
			}
			check_cursor();
			return Fl_Group::handle(event);

		case FL_RELEASE:
			drag_row = drag_col = -4;
			Fl_Group::handle(event);
			return 1;

		case FL_DRAG:
			vclicks=0;
			if (check_resize())
				return 1;
		case FL_PUSH:
			//	Dragging not clicking
			if (drag_row!=-4 || drag_col != -4)
				return 1;
			r = 0;
			if (Fl::event_button1() && (buttons() & FLV_BUTTON1)) r=1;
			if (Fl::event_button2() && (buttons() & FLV_BUTTON2)) r=1;
			if (Fl::event_button3() && (buttons() & FLV_BUTTON3)) r=1;
			if (r==0)
			{
				vclicks = 0;
				return 0;
			}

			//	Determine if col was clicked and highlight it
			TY = Fl::event_y();
			TX = Fl::event_x();
			r = get_row(TX,TY);
			c = get_col(TX,TY);
			if (r==-4 && c==-4)
			{
				vclicks = 0;
				return Fl_Group::handle(event);
			}
			if ( LX-TX>-3 && LX-TX<3 && LY-TY>-3 && LY-TY<3)
				vclicks++;
			else
			{
				vclicks=1;
				LX = TX;
				LY = TY;
			}

			damage(FL_DAMAGE_CHILD);
			rd = (r>row()?1:r==row()?0:-1);
			cd = (c>col()?1:c==col()?0:-1);
			if (r>=0)
				row(r);
			if (c>=0)
				col(c);

			if (!multi_select() ||
					(event==FL_PUSH && !Fl::event_state(FL_SHIFT)))
			{
				select_start_row(row());
				select_start_col(col());
			}

			//	At least one header clicked
			if (r<0 || c<0)
			{
				if (r>-4 && c>-4 && r<0 && c<0 && r!=-3)
				{
					if ( DOcb(FLVEcb_ALL_CLICKED) )
					{
						vwhy_event = FLVE_ALL_CLICKED;
						do_callback(this, user_data());
						vwhy_event = 0;
					}
					return 1;
				}

				if ( c>=0 || r==-3 )
				{
					vwhy_event = 0;
					switch( r )
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
						return 1;
					}
				}
				if ( r>=0 )
				{
					vwhy_event = 0;
					switch( c )
					{
						case -2:
							if (DOcb(FLVEcb_COL_FOOTER_CLICKED))
								vwhy_event = FLVE_COL_FOOTER_CLICKED;
							break;
						case -1:
							if (DOcb(FLVEcb_COL_HEADER_CLICKED))
								vwhy_event = FLVE_COL_HEADER_CLICKED;
							break;
					}
					if (vwhy_event)
					{
						do_callback(this, user_data());
						vwhy_event = 0;
						return 1;
					}
				}
				return 0;
			}


			if (event==FL_PUSH && (rd || cd))
			{
				//	Skip over locked cells
				while(!select_locked())
				{
					get_style(s,r,c);
					if (!s.locked())
					{
						if (r!=row() || c!=col())
							vclicks=0;
						row(r);
						col(c);
						break;
					}
					r += rd;
					c += cd;
					if ( r<0 || r>=rows() || c<0 || c>=cols() )
						break;
				}
			}
			if (event==FL_PUSH)
			{
				if (DOcb(FLVEcb_CLICKED))
				{
					vwhy_event = FLVE_CLICKED;
					do_callback(this, user_data());
					vwhy_event = 0;
				}
				if (vclicks>=vmax_clicks)
					vclicks=0;
			}
			return 1;

		default:
			return Fl_Group::handle(event);
	}

	switch(Fl::event_key())
	{
		case FL_Enter:
			switch( vmove_on_enter)
			{
				case FLV_MOVE_ON_ENTER_ROW_COL:
					if (!move_row(1))
					{
						row(0);
						col(col()+1);
						if (!select_locked())
						{
							get_style(s,r,col());
							if (!s.locked())
								move_row(1);
						}
					}
					return 1;
				case FLV_MOVE_ON_ENTER_COL_ROW:
					if (!move_col(1))
					{
						col(0);
						row(row()+1);
						if (!select_locked())
						{
							get_style(s,r,col());
							if (!s.locked())
								move_row(1);
						}
					}
					return 1;
			}
			return 0;

		case FL_Up:
			if (Fl::event_state(FL_CTRL))
				move_row(-row());
			else
				move_row(-1);
			break;

		case FL_Down:
			if (Fl::event_state(FL_CTRL))
				move_row(rows());
			else
				move_row(1);
			break;

		case FL_Page_Down:
			if (Fl::event_state(FL_CTRL))
				move_row( rows() );
			else
				move_row(page_size());
			break;

		case FL_Page_Up:
			if (Fl::event_state(FL_CTRL))
				move_row(-row());
			else
				move_row(-page_size());
			break;

		case FL_Home:
			//	Adjust rows before columns so we redraw everything
			if (Fl::event_state(FL_CTRL))
				move_row(-rows());
			move_col(-cols());
			break;
		case FL_End:
			//	Adjust rows before columns so we redraw everything
			if (Fl::event_state(FL_CTRL))
				move_row(rows());
			move_col(cols());
			break;

		case FL_Right:
			if (select_row())
				return 0;
			if (Fl::event_state(FL_CTRL))
				move_col(cols());
			else
				move_col(1);
			break;

		case FL_Left:
			if (select_row())
				return 0;
			if (Fl::event_state(FL_CTRL))
				move_col(-col());
			else
				move_col(-1);
			break;

		default:
			return Fl_Group::handle(event);
	}

	if (!multi_select() || !Fl::event_state(FL_SHIFT))
	{
		select_start_col(col());
		select_start_row(row());
	}
	return 1;
}

int Flv_Table::row(int n)
{
	int X,Y,W,H;

	if (n>=rows())
		n=rows()-1;
	if (n<0)
		n=0;
	if (n!=vrow)
	{
		vrow = n;
		client_area(X,Y,W,H);
		update_top_row(H);
		end_edit();
		if (edit_when()==FLV_EDIT_ALWAYS)
			switch_editor( row(), col() );
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

int Flv_Table::col( int n )
{
	Flv_Style s;

	if (n>=vcols)
		n=vcols-1;
	if (n<0)
		n=0;
	if (n!=vcol)
	{
		vcol = n;
		end_edit();
		if (edit_when()==FLV_EDIT_ALWAYS)
			switch_editor( row(), col() );

		adjust_for_cell();
		if (DOcb(FLVEcb_COL_CHANGED))
		{
			vwhy_event = FLVE_COL_CHANGED;
			do_callback(this, user_data());
			vwhy_event = 0;
		}
		damage(FL_DAMAGE_CHILD);
	}
	return vcol;
}

bool Flv_Table::col_resizable(int c)								//	Get/set column locked status
{
	Flv_Style *s;
	bool l=true;
	if (global_style.resizable_defined())
		l = global_style.resizable();
	s = col_style.find(c);
	if (s)
		if (s->resizable_defined())
			l = s->resizable();
	return l;
}

bool Flv_Table::col_resizable( bool n, int c)
{
	col_style[c].resizable(n);
	return n;
}

int Flv_Table::cols( int n )
{
	if (n>=0 && n!=vcols)
	{
		vcols = n;
		if (vcol>=vcols)
			col(vcols-1);
		if (vselect_col>vcol)
			select_start_col(vcol);
		update_width();
		if (DOcb(FLVEcb_COLS_CHANGED))
		{
			vwhy_event = FLVE_COLS_CHANGED;
			do_callback(this, user_data());
			vwhy_event = 0;
		}
		damage(FL_DAMAGE_CHILD);
	}
	return vcols;
}

bool Flv_Table::col_selected(int n)
{
	if (vselect_col<vcol)
		return (vselect_col<=n && n<=vcol);
	else
		return (vcol<=n && n<=vselect_col);
}

//	Get column from x,y
int Flv_Table::get_col( int x, int y ){
  int X, Y, W, H, CX;
  int rw, cw, t, Offset;

  at_end_col = false;
  client_area(X,Y,W,H);
  if (label()){
    cw = row_height(-3);
    Y+=cw;
    H-=cw;
  }
  if (col_header()){
    cw = col_width(-1);
    int rh = row_height(-1); // FIXME -1 or -2 or -3?
    if (X<=x && x<=X+cw && Y+H>=y && y>Y+rh){
      return -1;
    }
    X+=cw;
    W-=cw;
  }
  if (col_footer()){
    cw = col_width(-2);
    if (X+W>=x && x>=X+W-cw){
      return -2;
    }
    W -= cw;
  }
  
  rw = row_width();
  if (!rw)
    rw = W;
  if ( x<X || x>=X+W || y<Y || y>=Y+H || x>X-row_offset()+rw){
    return -4;
  }
  
  Offset = row_offset();
  for (CX=X, t=0;	t<vcols && CX-Offset<X+W;	t++, CX+=cw ){
    cw = col_width(t);
    if (x>=CX-Offset && x<CX-Offset+cw){
      at_end_col=(x>=CX+(4*cw)/5-Offset);
      return t;
    }
  }
  return -4;	//	In grey area at bottom?
}

int Flv_Table::select_start_col(int n)
{
	if (n>=vcols)
		n=vcols-1;
	if (n<0)
		n=0;
	if (n!=vselect_col)
	{
		vselect_col = n;
		if (DOcb(FLVEcb_SELECTION_CHANGED))
		{
			vwhy_event = FLVE_SELECTION_CHANGED;
			do_callback(this, user_data());
			vwhy_event = 0;
		}
		damage(FL_DAMAGE_CHILD);
	}
	return vselect_col;
}

void Flv_Table::update_width()
{
	int rw, n;

	for (rw=n=0;	n<vcols;	n++ )
		rw+=(col_width(n));
	if (col_header())
		rw+=col_width(-1);
	if (col_footer())
		rw+=col_width(-2);
	if (rw!=row_width())
	{
		row_width(rw);
		damage(FL_DAMAGE_CHILD);
	}
}

void Flv_Table::adjust_for_cell()
{
	int n, o, cw;
	int X, Y, W, H;

	for (n=o=0;	n<col();	n++ )
		o+=col_width(n);
	if (row_offset()>o)
		row_offset(o);
	else
	{
		client_area(X,Y,W,H);
		if (col_footer())
			W -= col_width(-2);
		if (col_header())
			W -= col_width(-2);
		cw = col_width(col());
		if (o+cw-row_offset()>W)
		{
			row_offset(o+cw-W);
		  damage(FL_DAMAGE_CHILD);
		}
	}
}

bool Flv_Table::check_resize(void)
{
	int ex, ey, v;
	int X, Y, W, H;

	if (drag_row<-3 && drag_col<-2)
		return false;

	client_area(X,Y,W,H);
	ex = Fl::event_x();
	ey = Fl::event_y();

	if (drag_row==-3)
	{
		v = ey-anchor_top;
		if (v<2) v=2;
		row_style[drag_row].height(v);
		damage(FL_DAMAGE_CHILD);
		return true;
	}

	if (label() && *label())
	{
		Y += row_height(-3);
		H -= row_height(-3);
	}

	if (drag_col>-3)
	{
		if (drag_col==-2)
		{
			v = anchor_left - ex + col_width(drag_col);
			if (col_header())
			{
				X += col_width(-1);
				W -= col_width(-1);
			}
			if (v>W-1)
			{
				v = W-1;
				anchor_left = X+W-v;
			}
			if (v<2)
			{
				v=2;
				anchor_left = X+W-2;
			}
			col_style[drag_col].width(v);
			damage(FL_DAMAGE_CHILD);
			if (v!=W-1 && v!=2)
				anchor_left = ex;
		} else
		{
			v = ex-anchor_left;
			if (drag_col==-1)
			{
      	//	Make sure it's in the grid
				if (col_footer())
					W-=col_width(-2);
				if (v > W-1 )
					v = W-1;
			}
			if (v<2)
				v=2;
			col_width(v,drag_col);
			damage(FL_DAMAGE_CHILD);
		}
	}

	//	Resize row
	if (drag_row>-4)
	{
		if (drag_row==-2)
		{
			v = anchor_top - ey + row_height(drag_row);
			if (row_header())
			{
				H-=row_height(-1);
				Y+=row_height(-1);
			}
			if (v>H-1)
			{
				v = H-1;
				anchor_top = Y+H-v;
			}
			if (v<2)
			{
				v = 2;
				anchor_top = Y+H-2;
			}
			row_style[drag_row].height(v);
			damage(FL_DAMAGE_CHILD);
			if (v!=2 && v!=H-1)
				anchor_top = ey;
		} else
		{
			v = ey-anchor_top;
			if (drag_row==-1)
			{
				if (row_footer())
					H-=row_height(-2);
				if (v>H-1)
					v = H-1;
			}
			if (v<2) v=2;
			row_height(v,drag_row);
			damage(FL_DAMAGE_CHILD);
		}
	}
	return true;
}

//	See if we can resize, if so change cursor
void Flv_Table::check_cursor(void){
  int X, Y, W, H, R, ey, ex, move=0, WW, size;
  int v;
  bool resize, inh, inv;
  Fl_Cursor cursor;
  
  //	Assume total miss
  drag_row = drag_col = -4;
  cursor = FL_CURSOR_DEFAULT;
  ex = Fl::event_x();
  ey = Fl::event_y();
  client_area(X,Y,W,H);
  inh = (ex>=X && ex<X+W);
  inv = (ey>=Y && ey<Y+H);
  if (inh && inv){ // set at_end_col if we are near the right of a cell
    get_col(ex,ey);
    if (at_end_col){
      get_row(ex,ey);
      click_fill=at_end_row;
      if (click_fill)
	cursor=FL_CURSOR_HAND;
    }
    else
      click_fill=false;
  }
  
  if (label() && *label()){
    size = row_height(-3);
    Y+=size;
    H-=size;
    resize = (ey>=Y-FUDGE && ey<=Y+FUDGE && inh);
    if (resize){
      if (row_resizable(-3)){
	drag_row = -3;
	anchor_top = Y-size;
	move |= MOVE_Y;
      }
    }
  }

  //	Trival tests to see if we're in region
  resize = full_resize();
  if (!resize){
    if (row_header()){
      size = row_height(-1);
      resize |= (ey>=Y && ey<=Y+size+FUDGE && inh);
    }
    if (!resize){
      if (row_footer()){
	size = row_height(-2);
	resize |= (ey<=Y+H && ey>=Y+H-size-FUDGE && inh);
      }
      if (!resize){
	if (col_header()){
	  size = col_width(-1);
	  resize |= (ex>=X && ex<=X+size+FUDGE && inh);
	}
	if (!resize){
	  if (col_footer()){
	    size = col_width(-2);
	    resize |= (ex<=X+W && ex>=X+W-size-FUDGE && inh);
	  }
	}
      }
    }
  }

  if (!resize){	//	In general region?
    if (cursor!=last_cursor)
      {
	fl_cursor(cursor,FL_BLACK,FL_WHITE);
	last_cursor = cursor;
      }
    return;
  }

  //	==================================================================
  //	Sweep columns
  WW = X;
  if (col_header()) {
    size = col_width(-1);
    if (ex>=WW+size-FUDGE && ex<=WW+size+FUDGE && inv){
      if (col_resizable(-1) && (move & MOVE_Y)==0){
	drag_col = -1;
	anchor_left = WW;
	move |= MOVE_X;
      }
    }
    WW += size;
    X += size;
    W -= size;
  }

  if (col_footer()){
    size = col_width(-2);
    if (ex>=X+W-size-FUDGE && ex<=X+W-size+FUDGE && inv){
      if (col_resizable(-2))
	{
	  drag_col = -2;
	  anchor_left = ex;
	  move |= MOVE_X;
	}
    }
    W -= size;
  }

  if ( (move & MOVE_X)==0 ){
    R = X-row_offset()+row_width()+FUDGE;  // Right edge of row
    for (WW-=row_offset(),v=0 ; WW<R && WW<X+W && v<cols(); WW+=size,v++ ){
      size = col_width(v);
      if (WW+size<X)		//	Off left
	continue;
      if (ex>=WW+size-FUDGE && ex<=WW+size+FUDGE && inv){
	if (col_resizable(v)){
	  drag_col = v;
	  anchor_left = WW;
	  move |= MOVE_X;		//	Moving col
	}
	break;
      }
    }
  }
  if (col_header()){
    X-=col_width(-1);
    W+=col_width(-1);
  }
  if (col_footer())
    W+=col_width(-2);

  //	==================================================================
  //	Sweep rows
  WW = Y;
  if (row_header()){
    size = row_height(-1);
    if (ey>=WW+size-FUDGE && ey<=WW+size+FUDGE && inh){
      if (row_resizable(-1)){
	drag_row = -1;
	anchor_top = WW;
	move |= MOVE_Y;
      }
    }
    WW += size;
    Y += size;
    H -= size;
  }

  if (row_footer()) {
    size = row_height(-2);
    if (ey>=Y+H-size-FUDGE && ey<=Y+H-size+FUDGE && inh){
      if (row_resizable(-2)){
	drag_row = -2;
	anchor_top = ey;
	move |= MOVE_Y;
      }
    }
    H -= size;
  } // end if(row_footer())

  if ( (move & MOVE_Y)==0 ){
    for (v=top_row();	v<rows();	WW+=size, v++ ){
      size = row_height(v);
      if (WW+size-FUDGE>=Y+H)
	break;
      if (ey>=WW+size-FUDGE && ey<=WW+size+FUDGE && inh){
	if (row_resizable(v)){
	  drag_row = v;
	  anchor_top = WW;
	  move |= MOVE_Y;		//	Moving
	}
	break;
      }
    } // end for
  } // end if move & MOVE_Y
  
  switch( move ){
  case MOVE_X:	cursor = FL_CURSOR_WE; drag_row=-4;	break;
  case MOVE_Y:	cursor = FL_CURSOR_NS; drag_col=-4;	break;
  case MOVE_XY:	cursor = FL_CURSOR_NWSE;		break;
  default:
    drag_row = drag_col = -4;
    cursor = FL_CURSOR_DEFAULT;
    break;
  }
  if (cursor!=last_cursor){
    fl_cursor(cursor,FL_BLACK,FL_WHITE);
    last_cursor = cursor;
  }
}

bool Flv_Table::move_row( int amount )
{
	Flv_Style s;
	int r = row();

	if (!amount)
		return true;

	r += amount;
	if (r>=rows())
		r = rows()-1;
	if (r<0)
		r = 0;

	while(!select_locked())
	{
		get_style(s,r,col());
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

bool Flv_Table::move_col( int amount )
{
	Flv_Style s;
	int c = col();

	if (!amount)
		return true;

	c += amount;
	if (c>=cols())
		c = cols()-1;
	if (c<0)
		c = 0;

	while(!select_locked())
	{
		get_style(s,row(),c);
		if (!s.locked())
			break;
		c += (amount<0?-1:1);
		if ( c<0 || c>=rows() )
			return false;
	}
	if (c!=col())
	{
		col(c);
		return true;
	}
	return false;
}


int Flv_Table::edit_when( int v )
{
	int wfocused = (Fl::focus()==veditor);
	if (v!=vedit_when)
	{
		vedit_when = v;
		if (vedit_when!=FLV_EDIT_ALWAYS)
			end_edit();
		else
			start_edit();
	}
	if (wfocused && !vediting)
	{
		Fl::focus(this);
//		take_focus();
		redraw();
	}
	return vedit_when;
}


void Flv_Table::start_edit(void)											//	Start editing
{
	if (!vediting)
	{
		vediting = true;
		switch_editor( row(), col() );
	}
}

void Flv_Table::end_edit(void)
{
	int wfocused = (Fl::focus()==veditor);
	if (veditor)
		switch_editor(-1,-1);
	if (wfocused && !vediting)
	{
		Fl::focus(this);
//		take_focus();
		redraw();
	}
}

void Flv_Table::cancel_edit(void)											//	Cancel editing
{
	int wfocused = (Fl::focus()==veditor);
	if (veditor)
	{
		veditor->hide();
		veditor->draw();
	}
	veditor = NULL;
	edit_row = -1;
	edit_col = -1;
	vediting = false;
//	switch_editor(-1, -1);
	if (wfocused && !vediting)
	{
		Fl::focus(this);
//		take_focus();
		redraw();
	}
}

void Flv_Table::switch_editor( int nr, int nc )
{
	Flv_Style s;
	int x, y, w, h, wfocused;
	char buf[30];
	
	wfocused = (Fl::focus()==veditor);

	if (veditor)
	{
		if (edit_row>-1 && edit_col>-1)
			save_editor( veditor, edit_row, edit_col );
		edit_row=-1;
		edit_col=-1;
		veditor->hide();
		veditor->draw();
		veditor = NULL;
	}
	if (edit_when()==FLV_EDIT_ALWAYS)
	{
		vediting = true;
		if (nr<0)
			nr = row();
		if (nc<0)
			nc = col();
	}
		
	if (nr>-1 && nc>-1 && vediting)
	{
		get_style( s, nr, nc );
		if (s.editor_defined() && !s.locked())
		{
			veditor = s.editor();
			if (veditor)
			{
				edit_row = nr;
				edit_col = nc;
				veditor->hide();
				get_cell_bounds(x,y,w,h,nr,nc);
				position_editor(veditor, x,y,w,h, s);
				load_editor( veditor, nr, nc );
				veditor->show();
				Fl::focus(veditor);
//				veditor->take_focus();
				veditor->handle(FL_FOCUS);
				veditor->damage(FL_DAMAGE_ALL);
				veditor->draw();
			}
		}
	}
	if (!veditor)
	{
		vediting=false;
		edit_row=-1;
		edit_col=-1;
	}
	if (!veditor && wfocused)
	{
		Fl::focus(this);
//		take_focus();
		handle(FL_FOCUS);
	}
}

#endif
