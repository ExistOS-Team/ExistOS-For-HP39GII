//	======================================================================
//	File:    Flv_Style.H - Flv_Style definition
//	Program: Flv_Style - FLTK Virtual List/Table Styles Widget
//	Version: 0.1.0
//	Started: 11/21/99
//
//	Copyright (C) 1999 Laurence Charlton
//
//	Description:
//	This has nothing to do with styles and themes being implemented
//	in FLTK 2.0 the name is just coincidental.  We will however try to use
//	true 2.0 style information as much as possible.
//
//	The virtual style classes are designed to handle style informaton for
//	the Flv_List class and derivatives.  (Although anyone could use
//	them)  The concept of virtual styles is to create a trickle down style
//	protocol that allows only defined attributes to be created.  The trickle
//	down is as follows:
//	Global style
//	Row style
//	Column style
//	Cell style
//
//	Note: Global Style will be a Flv_Cell_Style since that will
//				maximally define attributes for the table.
//	The global style will inherit whatever the current style information
//	dictates, so it will be completely defined.  (Whether it used in 1.x or 2.x
//	FLTK.  From there we will only require definition of styles where you
//	need them.  For instance:
//	If you want to override the header and footer styles and redefine
//	5 column layouts, you will have a total of 8 styles defined:
//	1 global style (always defined)
//	2 row styles
//	5 column styles
//
//	Hopefully only requiring the styles you really want to define will help
//	offset the fact that it just takes memory to do this... :)
//	Of course you could choose not to define any styles and then you'll just
//	have the global style defined. But you know the table is pretty bare...
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

#ifndef FLV_CSTYLE_H
#define FLV_CSTYLE_H

#include <FL/Fl.H>
#ifdef FLTK_2
#include <FL/Fl_Font.H>
#endif
#include "Flv_Style.H"

class Flv_CStyle : public Flv_Style
{
public:
	Flv_CStyle();

	int x(void)  const											//	Get x
		{	return vx;	};
	int x(int n);														//	Set x

	int y(void)  const											//	Get y
		{	return vy;	};
	int y(int n);														//	Set y

	const Flv_CStyle &operator=(const Flv_CStyle &n);
private:
	int vx, vy;															//	Row relative cell location
};

#endif

#endif
