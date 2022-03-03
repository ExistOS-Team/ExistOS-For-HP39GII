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

#ifndef FLV_STYLE_H
#define FLV_STYLE_H

#include <FL/Fl.H>
#ifdef FLTK_2
#include <FL/Fl_Font.H>
#endif

//	Border constants
#define FLVB_NONE						0
#define FLVB_LEFT						1
#define FLVB_TOP						2
#define FLVB_RIGHT					4
#define FLVB_BOTTOM					8
#define FLVB_INNER_LEFT			16
#define FLVB_INNER_TOP			32
#define FLVB_INNER_RIGHT		64
#define FLVB_INNER_BOTTOM		128
#define FLVB_OUTER_VERTICALS (FLVB_LEFT|FLVB_RIGHT)
#define FLVB_OUTER_HORIZONTALS (FLVB_TOP|FLVB_BOTTOM)
#define FLVB_OUTER_ALL (FLVB_VERTICALS|FLVB_HORIZONTALS)
#define FLVB_INNER_VERTICALS (FLVB_INNER_LEFT|FLVB_INNER_RIGHT)
#define FLVB_INNER_HORIZONTALS (FLVB_INNER_TOP|FLVB_INNER_BOTTOM)
#define FLVB_INNER_ALL (FLVB_INNER_VERTICALS|FLVB_INNER_HORIZONTALS)
#define FLVB_VERTICALS (FLVB_OUTER_VERTICALS|FLVB_INNER_VERTICALS)
#define FLVB_HORIZONTALS (FLVB_OUTER_HORIZONTALS|FLVB_INNER_HORIZONTALS)
#define FLVB_ALL (FLVB_OUTER_ALL|FLVB_INNER_ALL)
#define FLVB_LEFTS (FLVB_LEFT|FLVB_INNER_LEFT)
#define FLVB_TOPS (FLVB_TOP|FLVB_INNER_TOP)
#define FLVB_RIGHTS (FLVB_RIGHT|FLVB_INNER_RIGHT)
#define FLVB_BOTTOMS (FLVB_BOTTOM|FLVB_INNER_BOTTOM)

class Flv_Style;

//	Note: it is undefined behavior to insert a non-dynamically allocated
//	styles into this list!
class Flv_Style_List
{
public:
	Flv_Style_List();

	void clear(void);									//	Undefine all styles in list
  void compact(void);								//	Release any unused style memory
	void release(void);								//	Free memory for all (including cell
	Flv_Style *current(void);					//	Current node
	Flv_Style *find( int n );					//	Find value n (Random access method)
	Flv_Style *first(void);						//	Get first style
	bool insert( Flv_Style *n );			//	Add style (if doesn't exist)
	Flv_Style *next(void);						//	Next style
	Flv_Style *skip_to(int v );				//	Find value n (Sequential processing)
	Flv_Style *prior(void);						//	Previous style
	bool clear_current(void);					//	Undefine optionally release
	bool release_current(void);				//	Remove current style
	int count(void)										//	# of styles
		{	return vcount;	}
	Flv_Style &operator[](int value);	//	Note: this could be a little
																		//	weird.  It's actually going
																		//	to return a style with value
																		//	not index value.
private:
	Flv_Style **list;							//	Array of style pointers
	int vcount;														//	# of style pointers defined
	int vallocated;												//	# of style pointers allocated
	int vcurrent;													//	Current position
};

class Flv_Style
{
public:
	friend class Flv_Style_List;									//	Hack for value
	Flv_Style();
	Flv_Style( int value );

	bool all_clear(void)										//	Is all style info cleared?
		{	return (vdefined==0);	}
	void clear_all(void)										//	Clear all style info
		{	vdefined=0;	}
	bool all_defined(void)									//	Everything defined?
		{	return (vdefined&1023)==1023;	}

	const Fl_Align &align(void) const				//	Get drawing alignment
		{	return valign;	};
	const Fl_Align &align(const Fl_Align &n);	//	Set drawing alignment
	void clear_align(void);									//	Undefine drawing alignment
	bool align_defined(void) const;					//	Is drawing alignment defined?

	Fl_Color background(void) const					//	Get background color
		{	return vbackground;	};
	Fl_Color background(Fl_Color n);				//	Set background color
	void clear_background(void);						//	Undefine background color
	bool background_defined(void) const;		//	Is background defined?

  int border(void) const									//	Get borders
  	{	return vborder;	}
  int border(int n);											//	Set borders
  void clear_border(void);								//	Undefine border
  bool border_defined(void) const;				//	Is border defined?
  bool left_border(void) const						//	Left border?
  	{	return (vborder&FLVB_LEFT)==FLVB_LEFT;	}
  bool top_border(void) const							//	Top border?
  	{	return (vborder&FLVB_TOP)==FLVB_TOP;	}
  bool right_border(void) const						//	Right border?
  	{	return (vborder&FLVB_RIGHT)==FLVB_RIGHT;	}
  bool bottom_border(void) const					//	Bottom border?
  	{	return (vborder&FLVB_BOTTOM)==FLVB_BOTTOM;	}
  bool inner_left_border(void) const			//	Inner left border?
  	{	return (vborder&FLVB_INNER_LEFT)==FLVB_INNER_LEFT;	}
  bool inner_top_border(void) const				//	Inner top border?
  	{	return (vborder&FLVB_INNER_TOP)==FLVB_INNER_TOP;	}
  bool inner_right_border(void) const			//	Inner right border?
  	{	return (vborder&FLVB_INNER_RIGHT)==FLVB_INNER_RIGHT;	}
  bool inner_bottom_border(void) const		//	Inner bottom border?
  	{	return (vborder&FLVB_INNER_BOTTOM)==FLVB_INNER_BOTTOM;	}

  Fl_Color border_color(void) const				//	Get border colors
  	{	return vborder_color;	}
  Fl_Color border_color(Fl_Color n);			//	Set border colors
  void clear_border_color(void);					//	Undefine border color
  bool border_color_defined(void) const;	//	Is border color defined?

  int border_spacing(void) const					//	Get border spacings
  	{	return vborder_spacing;	}
  int border_spacing(int n);							//	Set border spacings
  void clear_border_spacing(void);				//	Undefine border spacing
  bool border_spacing_defined(void) const;	//	Is border spacing defined?

	Fl_Widget *editor(void) const						//	Get content editor
		{	return veditor;	};
	Fl_Widget *editor(Fl_Widget *v);
	void clear_editor(void);								//	Undefine content editor
	bool editor_defined(void) const;				//	Is content editor defined?

	const Fl_Font &font(void) const					//	Get current font
		{	return vfont;	};
	const Fl_Font &font(const Fl_Font &n);	//	Set current font
	void clear_font(void);									//	Undefine font
	bool font_defined(void) const;					//	Is font defined

	int font_size(void)	const								//	Get font size
		{	return vfont_size;	};
	int font_size(int n);										//	Set font size
	void clear_font_size(void);							//	Undefine font size
	bool font_size_defined(void) const;			//	Is font size defined?

	Fl_Color foreground(void) const					//	Get foreground color
		{	return vforeground;	};
	Fl_Color foreground(Fl_Color n);				//	Set foreground color
	void clear_foreground(void);						//	Undefine foreground color
	bool foreground_defined(void) const;		//	Is foreground defined?

	const Fl_Boxtype &frame(void) const			//	Get frame type
		{	return vframe;	};
	const Fl_Boxtype &frame(const Fl_Boxtype &n) ;	//	Set frame type
	void clear_frame(void);									//	Undefine frame type
	bool frame_defined(void) const;					//	Is frame type defined?

	int height(void) const									//	Get height
		{	return vheight;	};
	int height(int n );											//	Set height
	void clear_height(void);								//	Undefine row height
	bool height_defined(void) const;				//	Is row height defined


	bool locked(void) const									//	Get locked
		{	return vlocked;	}
	bool locked(bool n);										//	Set locked
	void clear_locked(void);								//	Undefine locked
	bool locked_defined(void) const;				//	Is locked defined?

	bool resizable(void) const							//	Get resizable (Not for Cell)
		{	return vresizable;	};
	bool resizable(bool n);									//	Set resizable
	void clear_resizable(void);							//	Undefine resizable
	bool resizable_defined(void) const;			//	Is resizable defined?

	int width(void) const										//	Get column width
		{	return vwidth;	}
	int width(int n);												//	Set column width
	void clear_width(void);									//	Undefine column width
	bool width_defined(void) const;					//	Is column width defined?

  int x_margin(void) const								//	Get x margin
  	{	return vx_margin;	}
  int x_margin(int x);										//	Set x margin
  void clear_x_margin(void);							//	Undefine x margin
  bool x_margin_defined(void) const;			//	Is x margin defined?

	int y_margin(void) const								//	Get y margin
  	{	return vy_margin;	}
  int y_margin(int y);										//	Set y margin
  void clear_y_margin(void);							//	Undefine y margin
  bool y_margin_defined(void) const;			//	Is y margin defined?

	//	Cumulative assignment operator
	//	This will only assign portions that are defined.
	const Flv_Style &operator=(const Flv_Style &n);

	Flv_Style_List cell_style;

protected:
	int value(void) const										//	Get row/column #
		{	return vvalue;	};
	int value(int n)												//	Set row/column #
		{	return (vvalue=n);	}

private:
	unsigned int vdefined;									//	Which parts are defined?
	Fl_Align valign;												//	Drawing alignment
	Fl_Color vbackground;										//	Background color
  unsigned char vborder;									//	Borders around cell
  unsigned char vborder_spacing;					//	Spacing between inner/outer border
	Fl_Color vborder_color;									//	Outer border color
	Fl_Widget *veditor;											//	Content editor
	unsigned char vx_margin;								//	X margin (Left/Right)
	unsigned char vy_margin;								//	Y margin (Top/Bottom)
	Flv_Style_List vcell;										//	Cell list
	Fl_Font vfont;													//	Font to draw with
	int vfont_size;													//	Size of font
	Fl_Color vforeground;										//	Foreground color
	Fl_Boxtype vframe;											//	Frame around cell
	int vheight;														//	Row height
	bool vlocked;														//	Group locked?
	bool vresizable;												//	Allow resizing?
	int vvalue;															//	Row or Column #
	int vwidth;															//	Column height
};
#endif

#endif
