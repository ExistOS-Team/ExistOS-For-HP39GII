//	======================================================================
//	File:    Flve_Check_Button.h - Flve_Check_Button implementation
//	Library: flvw - FLTK Widget
//	Version: 0.1.0
//	Started: 01/12/2000
//
//	Copyright (C) 1999 Laurence Charlton
//
//	Description:
//	Flve_Check_Button implements a toggle editor for a list/table
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

#ifndef FLVE_CHECK_BUTTON_H
#define FLVE_CHECK_BUTTON_H

#include <FL/Fl_Check_Button.H>
#include "Flv_List.H"
#include <stdio.h>

class Flve_Check_Button : public Fl_Check_Button
{
public:
	Flve_Check_Button( int X, int Y, int W, int H, const char *l=0 ) :
		Fl_Check_Button(X,Y,W,H,l) {	owner=NULL;	};
	Fl_Widget *owner;
protected:
	int handle(int event );
};

void draw_flve_check_button( int X, int Y, int W, int H,
															Flve_Check_Button *b, char *v );
#endif

#endif
