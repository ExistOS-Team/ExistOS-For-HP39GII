//	======================================================================
//	File:    Flv_Data_Source.h - Flv_Data_Source implementation
//	Program: Flv_Data_Source - FLTK Widget
//	Version: 0.1.0
//	Started: 11/21/99
//
//	Copyright (C) 1999 Laurence Charlton
//
//	Description:
//	Flv_Data_Source implements an abstract data source for managing a table
//	of information to be supplied to Flv_List or Flv_Table
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

#ifndef FLV_DATA_SOURCE_H
#define FLV_DATA_SOURCE_H

Table Map Data_Source
Table -> Map (Layout,Row,Col) -> (DataSource,Row,Col)
class Flv_Data_Source
{
public:
	const char *AsString(int R, int C);
	int AsLong(int R, int C);

protected:
};
#endif

#endif
