//	======================================================================
//	File:    Flv_CStyle.cxx - Flv_CStyle implementation
//	Program: Flv_Style - FLTK Virtual List/Table Styles Widget
//	Version: 0.1.0
//	Started: 11/21/99
//
//	Copyright (C) 1999 Laurence Charlton
//
//	Description:
//	The complex styles will be used in the complex table.  Should be basically
//	the same as Flv_Style with x,y positional data.
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

#include "Flv_CStyle.H"
#include <stdio.h>
#ifdef WIN32
#include <memory.h>
#else
#include <memory.h>
#endif

#define ADDSIZE 10

//	**********************************************************************
//	Routines for Flv_CStyle
//
//	Defines the additional properties for a complex layout table.
//	A complex layout table allows the placement of cells (columns) within
//	a row.  The overall layout is used to determine minimum row heights/
//	widths, etc.
//	**********************************************************************
Flv_CStyle::Flv_CStyle() :
	Flv_Style()
{
	vx = 0;
	vy = 0;
	width(0);
	height(0);
}

//	Set x
int Flv_CStyle::x(int n)
{
	if (n < 0)
		n = 0;
	return (vx = n);
}

//	Set y
int Flv_CStyle::y(int n)
{
	if (n < 0)
		n = 0;
	return (vy = n);
}


const Flv_CStyle &Flv_CStyle::operator=(const Flv_CStyle &n)
{
	Flv_Style::operator=(n);
	vx = n.vx;
	vy = n.vy;
	return *this;
}


#endif
