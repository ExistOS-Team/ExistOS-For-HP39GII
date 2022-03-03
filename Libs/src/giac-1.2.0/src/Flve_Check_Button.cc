//	======================================================================
//	File:    Flve_Input.cxx - Flve_Input implementation
//	Library: flvw - FLTK Virtual widget library
//	Version: 0.1.0
//	Started: 01/12/2000
//
//	Copyright (C) 1999 Laurence Charlton
//
//	Description:
//	Flve_Input implements cell text editing for a list/table.
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

#include <FL/Fl.H>
#include "Flve_Check_Button.H"
#include <FL/fl_draw.H>

int Flve_Check_Button::handle(int event)
{
	int stat;

	stat = Fl_Check_Button::handle(event);
	if (!stat)
	{
		switch( event )
		{
			case FL_FOCUS:
			case FL_UNFOCUS:
				return 1;

			case FL_KEYBOARD:
				switch(Fl::event_key())
				{
					case ' ':
						value( !value() );
						redraw();
						return 1;
				}
				break;
		}

		if (owner && event==FL_KEYBOARD)
			if ( owner->handle(FL_SHORTCUT) )
				return 1;
	}
	return stat;
}

void draw_flve_check_button( int X, int Y, int W, int H, Flve_Check_Button *b, char *v )
{
	int x, y,  w, h;
	w = W;
	h = H;
	if (H<W)
		w = h;
	else
		h = w;
	x = X;	// + (W-w)/2;
	y = Y;	// + (H-h)/2;
	x++;
	y++;
	w-=2;
	h-=2;
	fl_color( b?b->color():(Fl_Color)(FL_GRAY_RAMP+17) );
	fl_rectf( X, Y, W, H );
	if (b)
	{
		if (*v=='1')
			fl_draw_box( b->down_box(), x, y, w, h, b->selection_color() );
		else
			fl_draw_box( b->down_box(), x, y, w, h, b->color() );
	} else
	{
		if (*v=='1')
			fl_draw_box( FL_THIN_DOWN_BOX, x, y, w, h, FL_RED );
		else
			fl_draw_box( FL_THIN_DOWN_BOX, x, y, w, h, (Fl_Color)(FL_GRAY_RAMP+17) );
	}
}


#endif
