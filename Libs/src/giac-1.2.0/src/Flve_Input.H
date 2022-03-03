//	======================================================================
//	File:    Flve_Input.h - Flve_Input implementation
//	Library: flvw - FLTK Widget
//	Version: 0.1.0
//	Started: 01/12/2000
//
//	Copyright (C) 1999 Laurence Charlton
//
//	Description:
//	Flve_Input implements cell text editing for a list/table.
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

#ifndef FLVE_INPUT_H
#define FLVE_INPUT_H

#include <FL/Fl_Input.H>
#include "Flv_List.H"
#include <stdio.h>

class Flve_Input : public Fl_Input
{
public:
	Flve_Input( int X, int Y, int W, int H, const char *l=0 ) :
		Fl_Input(X,Y,W,H) {	owner=NULL;	};
	Fl_Widget *owner;
protected:
	int handle(int event );
};

#endif
