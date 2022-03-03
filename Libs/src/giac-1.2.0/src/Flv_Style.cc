//	======================================================================
//	File:    Flv_Style.cxx - Flv_Style implementation
//	Program: Flv_Style - FLTK Virtual List/Table Styles Widget
//	Version: 0.1.0
//	Started: 11/21/99
//
//	Copyright (C) 1999 Laurence Charlton
//
//	Description:
//	The styles classes are basically defined to make life easier while
//	working with styles for the virtual classes.
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

#include <FL/Fl_Widget.H>
#include "Flv_Style.H"
#include <stdio.h>
#ifdef WIN32
#include <memory.h>
#else
#include <memory.h>
#endif

#define ADDSIZE 10

//	Flv_Style bits
#define STYLE_DEFINE_FONT						0x0001
#define STYLE_DEFINE_FONT_SIZE			0x0002
#define STYLE_DEFINE_FOREGROUND			0x0004
#define STYLE_DEFINE_BACKGROUND			0x0008
#define STYLE_DEFINE_ALIGN					0x0010
#define STYLE_DEFINE_FRAME					0x0020
#define STYLE_DEFINE_RESIZABLE			0x0040
#define STYLE_DEFINE_HEIGHT					0x0080
#define STYLE_DEFINE_WIDTH					0x0100
#define STYLE_DEFINE_LOCKED					0x0200
#define STYLE_DEFINE_BORDER					0x0400
#define STYLE_DEFINE_BORDER_COLOR		0x0800
#define STYLE_DEFINE_BORDER_SPACING	0x1000
#define STYLE_DEFINE_X_MARGIN				0x2000
#define STYLE_DEFINE_Y_MARGIN				0x4000
#define STYLE_DEFINE_EDITOR					0x8000

#define CLEAR(n) vdefined &= ~(n)
#define DEFINED(n) ((vdefined & (n))!= 0)
#define DEFINED2(v,n) ((v.vdefined & (n))!= 0)

Flv_Style::Flv_Style()
{
	vdefined = 0;
	vvalue = 0;
	//	I'm not worried about initializing the rest of the private
	//	variables since they are undefined.
}

Flv_Style::Flv_Style( int value )
{
	vdefined = 0;
	vvalue = value;
	//	I'm not worried about initializing the rest of the private
	//	variables since they are undefined.
}

//	==================================================================
//	Set drawing alignment
const Fl_Align &Flv_Style::align(const Fl_Align &n)
{
	valign = n;
	vdefined |= STYLE_DEFINE_ALIGN;
	return valign;
}

//	Undefine drawing alignment
void Flv_Style::clear_align(void)
{
	CLEAR(STYLE_DEFINE_ALIGN);
}

//	Is drawing alignment defined?
bool Flv_Style::align_defined(void) const
{
	return DEFINED(STYLE_DEFINE_ALIGN);
}

//	==================================================================
//	Set background color
Fl_Color Flv_Style::background(Fl_Color n)
{
	vbackground = n;
	vdefined |= STYLE_DEFINE_BACKGROUND;
	return vbackground;
}

//	Undefine background color
void Flv_Style::clear_background(void)
{
	CLEAR(STYLE_DEFINE_BACKGROUND);
}

//	Is background defined?
bool Flv_Style::background_defined(void) const
{
	return DEFINED(STYLE_DEFINE_BACKGROUND);
}


//	==================================================================
//	Set border
int Flv_Style::border(int n)
{
	vborder = (unsigned char)n;
	vdefined |= STYLE_DEFINE_BORDER;
	return vborder;
}

//	Undefine border
void Flv_Style::clear_border(void)
{
	CLEAR(STYLE_DEFINE_BORDER);
}

//	Is border defined?
bool Flv_Style::border_defined(void) const
{
	return DEFINED(STYLE_DEFINE_BORDER);
}

//	==================================================================
//	Set border_color
Fl_Color Flv_Style::border_color(Fl_Color n)
{
	vborder_color = n;
	vdefined |= STYLE_DEFINE_BORDER_COLOR;
	return vborder_color;
}

//	Undefine border_color
void Flv_Style::clear_border_color(void)
{
	CLEAR(STYLE_DEFINE_BORDER_COLOR);
}

//	Is border_color defined?
bool Flv_Style::border_color_defined(void) const
{
	return DEFINED(STYLE_DEFINE_BORDER_COLOR);
}


//	==================================================================
//	Set border_spacing
int Flv_Style::border_spacing(int n)
{
	vborder_spacing = (unsigned char)n;
	vdefined |= STYLE_DEFINE_BORDER_SPACING;
	return vborder_spacing;
}

//	Undefine border_spacing
void Flv_Style::clear_border_spacing(void)
{
	CLEAR(STYLE_DEFINE_BORDER_SPACING);
}

//	Is border_spacing defined?
bool Flv_Style::border_spacing_defined(void) const
{
	return DEFINED(STYLE_DEFINE_BORDER_SPACING);
}

//	==================================================================
//	Set content editor
Fl_Widget *Flv_Style::editor(Fl_Widget *v)
{
	veditor = v;
	if (Fl::focus()!=v && veditor)
		veditor->hide();
	vdefined |= STYLE_DEFINE_EDITOR;
	return veditor;
}

//	Undefine border_spacing
void Flv_Style::clear_editor(void)
{
	CLEAR(STYLE_DEFINE_EDITOR);
}

//	Is border_spacing defined?
bool Flv_Style::editor_defined(void) const
{
	return DEFINED(STYLE_DEFINE_EDITOR);
}

//	==================================================================
//	Set current font
const Fl_Font &Flv_Style::font(const Fl_Font &n)
{
	vfont = n;
	vdefined |= STYLE_DEFINE_FONT;
	return vfont;
}

//	Undefine font
void Flv_Style::clear_font(void)
{
	CLEAR(STYLE_DEFINE_FONT);
}

//	Is font defined
bool Flv_Style::font_defined(void) const
{
	return DEFINED(STYLE_DEFINE_FONT);
}

//	==================================================================
//	Set font size
int Flv_Style::font_size(int n)
{
	if (n < 1)						//	Clip at 1 as the smallest font size
		n = 1;
	vfont_size = n;
	vdefined |= STYLE_DEFINE_FONT_SIZE;
	return vfont_size;
}

//	Undefine font size
void Flv_Style::clear_font_size(void)
{
	CLEAR(STYLE_DEFINE_FONT_SIZE);
}

//	Is font size defined?
bool Flv_Style::font_size_defined(void) const
{
	return DEFINED(STYLE_DEFINE_FONT_SIZE);
}

//	==================================================================
//	Set foreground color
Fl_Color Flv_Style::foreground(Fl_Color n)
{
	vforeground = n;
	vdefined |= STYLE_DEFINE_FOREGROUND;
	return vforeground;
}

//	Undefine foreground color
void Flv_Style::clear_foreground(void)
{
	CLEAR(STYLE_DEFINE_FOREGROUND);
}

//	Is foreground defined?
bool Flv_Style::foreground_defined(void) const
{
	return DEFINED(STYLE_DEFINE_FOREGROUND);
}

//	==================================================================
//	Set frame type
const Fl_Boxtype &Flv_Style::frame(const Fl_Boxtype &n)
{
	vframe = n;
	vdefined |= STYLE_DEFINE_FRAME;
	return vframe;
}

//	Undefine frame type
void Flv_Style::clear_frame(void)
{
	CLEAR(STYLE_DEFINE_FRAME);
}

//	Is frame type defined?
bool Flv_Style::frame_defined(void) const
{
	return DEFINED(STYLE_DEFINE_FRAME);
}

//	==================================================================
//	Set height
int Flv_Style::height(int n )
{
	if (n < 0)
		n = 0;
	vdefined |= STYLE_DEFINE_HEIGHT;
	return (vheight = n);
}

//	Undefine row height
void Flv_Style::clear_height(void)
{
	CLEAR(STYLE_DEFINE_HEIGHT);
}

//	Is row height defined?
bool Flv_Style::height_defined(void) const
{
	return DEFINED(STYLE_DEFINE_HEIGHT);
}

//	==================================================================
//	Set locked
bool Flv_Style::locked(bool n)
{
	vdefined |= STYLE_DEFINE_LOCKED;
	return (vlocked = n);
}

//	Undefine locked
void Flv_Style::clear_locked(void)
{
	CLEAR(STYLE_DEFINE_LOCKED);
}

//	Is locked defined?
bool Flv_Style::locked_defined(void) const
{
	return DEFINED(STYLE_DEFINE_LOCKED);
}

//	==================================================================
//	Set resizable
bool Flv_Style::resizable(bool n)
{
	vresizable = n;
	vdefined |= STYLE_DEFINE_RESIZABLE;
	return vresizable;
}

//	Undefine resizable
void Flv_Style::clear_resizable(void)
{
	CLEAR(STYLE_DEFINE_RESIZABLE);
}

//	Is resizable defined?
bool Flv_Style::resizable_defined(void) const
{
	return DEFINED(STYLE_DEFINE_RESIZABLE);
}

//	==================================================================
//	Set column width
int Flv_Style::width(int n)
{
	if (n < 0)
		n = 0;
	vdefined |= STYLE_DEFINE_WIDTH;
	return (vwidth = n);
}

//	Undefine column width
void Flv_Style::clear_width(void)
{
	CLEAR(STYLE_DEFINE_WIDTH);
}

//	Is column width defined?
bool Flv_Style::width_defined(void) const
{
	return DEFINED(STYLE_DEFINE_WIDTH);
}

//	==================================================================
//	Set x margin
int Flv_Style::x_margin(int x)
{
	if (x<0)
  	x=0;
	if (x!=vx_margin)
  {
  	vdefined |= STYLE_DEFINE_X_MARGIN;
    vx_margin = (unsigned char)x;
  }
  return vx_margin;
}

//	Undefine x margin
void Flv_Style::clear_x_margin(void)
{
	CLEAR(STYLE_DEFINE_X_MARGIN);
}

//	Is x margin defined
bool Flv_Style::x_margin_defined(void) const
{
	return DEFINED(STYLE_DEFINE_X_MARGIN);
}

//	==================================================================
//	Set y margin
int Flv_Style::y_margin(int y)
{
	if (y<0)
  	y=0;
	if (y!=vy_margin)
  {
  	vdefined |= STYLE_DEFINE_Y_MARGIN;
    vy_margin = (unsigned char)y;
  }
  return vy_margin;
}

//	Undefine y margin
void Flv_Style::clear_y_margin(void)
{
	CLEAR(STYLE_DEFINE_Y_MARGIN);
}

//	Is y margin defined
bool Flv_Style::y_margin_defined(void) const
{
	return DEFINED(STYLE_DEFINE_Y_MARGIN);
}

//	==================================================================
//	Cumulative assignment operator
//	This will only assign portions that are defined.
const Flv_Style &Flv_Style::operator=(const Flv_Style &n)
{
	if (n.align_defined())
		align(n.valign);
	if (n.background_defined())
		background(n.vbackground);
  if (n.border_defined())
  	border(n.vborder);
	if (n.border_color_defined())
		border_color(n.vborder_color);
	if (n.border_spacing_defined())
		border_spacing(n.vborder_spacing);
	if (n.editor_defined())
		editor(n.veditor);
	if (n.font_defined())
		font(n.vfont);
	if (n.font_size_defined())
		font_size(n.vfont_size);
	if (n.foreground_defined())
		foreground(n.vforeground);
	if (n.frame_defined())
		frame(n.vframe);
	if (n.height_defined())
		height(n.vheight);
	if (n.locked_defined())
		locked(n.vlocked);
	if (n.resizable_defined())
		resizable(n.vresizable);
	if (n.width_defined())
		width(n.vwidth);
  if (n.x_margin_defined())
  	x_margin(n.vx_margin);
	if (n.y_margin_defined())
		y_margin(n.vy_margin);


	//	I'm not copying value because it seems meaningless in every context
	//	I can think of.

	//	I'm not copying cell_style either for the same reason.  It just seems like
	//	a REALLY bad idea.
		return *this;
}

//	**********************************************************************
//	Routines for Flv_Style_List
//
//	Implemented as a dynamic sparse array
//	**********************************************************************
Flv_Style_List::Flv_Style_List()
{
	list = NULL;
	vcount = vallocated = vcurrent = 0;
}

void Flv_Style_List::compact(void)
{
	int n, t;

  //	Release memory for any dead items
	for (t=0; 	t<vcount;	t++ )
  {
  	list[t]->cell_style.compact();			//	Compact cells!
    if (list[t]->cell_style.count()==0 && list[t]->all_clear())
    {
			delete list[t];
      list[t] = NULL;
    }
  }
  //	Compact list now
	for (t=n=0;	t<vcount;	t++ )
  {
  	if (list[t])
    	list[n++] = list[t];
    else if (vcurrent<=t && vcurrent>0)
    	vcurrent--;
  }

  //	Make list easy to view, wasted CPU cycles
  for (t=n;	t<vcount;	t++ )
  	list[t] = NULL;

  vcount = n;	//	Update count

  if (!vcount && list)
  {
		delete []list;
    list = NULL;
    vcount = vcurrent = vallocated = 0;
  }
}
//	Undefine all styles in list
void Flv_Style_List::clear(void)
{
	int t;

  for (t=0;	t<vcount;	t++ )									//	Make all entries clear
  	list[t]->clear_all();
  compact();																//	Remove dead space thats left
}

//	Free memory for all (including cell
void Flv_Style_List::release(void)
{
	int t;
	for (t = 0; t < vcount; t++ )
	{
		list[t]->cell_style.release();
		delete list[t];
	}
	if (list)
		delete []list;
	list = NULL;
	vcurrent = vcount = vallocated = 0;
}

Flv_Style *Flv_Style_List::current(void)			//	Current node
{
	if (!list)
		return NULL;
	return list[vcurrent];
}

//	Find closest match
//	It will find the first value >= n
Flv_Style *Flv_Style_List::find( int n )
{
	int t, l, h;

	if (!list || vcount == 0)	//	If list is empty, there will be no matches
		return NULL;

	//	How a about a nice binary search?  It will be slower for sequential
	//	processing and a small number of styles, but worlds faster as the
	//	number of styles increases.  Use skip_to for sequential processing
	//	and find for random access.
	l = 0;
	h = vcount-1;
	while (l+1 < h)
	{
		vcurrent = (l+h) / 2;
		t = list[vcurrent]->value();
		if (t == n)
			return list[vcurrent];
		else if (t < n)
			l = vcurrent;
		else
			h = vcurrent;
	}

	//	This needs cleaning, I fairly certain there's way too much logic here
	//	While this will work, I think we only need to check one of the values
	//	but I've been wrong before... :)
	vcurrent = l;
	t = list[vcurrent]->value();
	if (t == n)
		return list[vcurrent];
	if (t < n && vcurrent < vcount-1)
	{
		vcurrent=h;
		t = list[vcurrent]->value();
		if (t == n)
			return list[vcurrent];
	}
	return NULL;
}

Flv_Style *Flv_Style_List::first(void)											//	Get first style
{
	if (!list)
		return NULL;
	vcurrent = 0;
	return list[vcurrent];
}

bool Flv_Style_List::insert( Flv_Style *n )		//	Add style (if doesn't exist)
{
	int t;
	//	Make sure there is room for a new item
	if (vcount == vallocated)
	{
		Flv_Style **a = new Flv_Style *[vallocated+ADDSIZE];
		if (!a)
			return false;
    //	Wasted CPU cycles, but list is pretty
    memset( a, 0, sizeof(Flv_Style *)*(vallocated+ADDSIZE) );
		if (vcount)
			memcpy( a, list, sizeof(Flv_Style *)*vcount );
		vallocated += ADDSIZE;
    if (list)
			delete []list;
		list = a;
	}

  if (vcount)
  {
		find(n->value());									//	Point to insert candidate
  	if (n->value()==list[vcurrent]->value())	//	No duplicates
  		return false;
	  if (n->value()>list[vcurrent]->value())		//	Insert at end of list
  		vcurrent++;
  }


	//	Make room for insert if not appending
  for (t=vcount;	t>vcurrent;	t-- )
  	list[t] = list[t-1];

	list[vcurrent] = n;
	vcount++;
	return true;
}

Flv_Style *Flv_Style_List::next(void)											//	Next style
{
	if (!list || vcurrent >= vcount-1)
		return NULL;

	vcurrent++;
	return list[vcurrent];
}

Flv_Style *Flv_Style_List::prior(void)
{

	if (!vcurrent || !list)
		return 0;
	vcurrent--;
	return list[vcurrent];
}

bool Flv_Style_List::clear_current(void)
{
	if (!list)
		return false;

	if (list[vcurrent]->cell_style.count() == 0)
		return release_current();
	list[vcurrent]->clear_all();
	return true;
}

bool Flv_Style_List::release_current(void)										//	Remove current style
{
	if (!list)
		return false;

	delete list[vcurrent];
	if (vcurrent < vcount-1)
	{
		memmove(list+vcurrent, list+vcurrent+1, sizeof(Flv_Style *)*(vcount-vcurrent) );
		vcount--;
		list[vcount] = NULL;
	}
	if (vcurrent == vcount)
		vcurrent--;

	return true;
}

//	From n skip up to value v
Flv_Style *Flv_Style_List::skip_to( int v )
{
	int c;

	if (!list || !vcount)
		return NULL;

	//	In case we're backing up or starting over
	//	We're checking vcurrent-1 so if the last search found
	//	an entry > the desired value, and this search isn't quite
	//	to the last found value, we don't want to start over, just
	//	stay where we are.
	//	Style 1, 2, 3, 7 & 10 defined
	//	search for 4 (current points to 7) returns false
	//	search for 5 (stay at seven since 3 is < value, return false)
	//		If we started at 0 we'd end up here anyway... :)
	//	search for 7 (stay at seven, we'll find it quick, return true)
	if (vcurrent)
	{
  	if (list[vcurrent-1]->value() >= v)
			vcurrent = 0;
  }

	for (; vcurrent < vcount; vcurrent++ )
	{
		c = list[vcurrent]->value();
		if (c == v)
			return list[vcurrent];
		else if (c > v)
			return NULL;
	}
	vcurrent--;
	return NULL;
}

//	Note: this could be a little wierd since it's actually returning
//	the style with value 'value' instead of list index 'value'
//	Plus: it's going to define the style if it doesn't already exist!
//
//	If you don't want extraneous styles getting inserted, be sure to
//	use the find operator first.  (I.e. If your reading a style value
//	and the style didn't previously exist, all the style information
//	will be undefined!
Flv_Style &Flv_Style_List::operator[](int value)
{
	Flv_Style *p;

	if (find(value))							//	If it exists
		return *(list[vcurrent]);		//		return it

	p = new Flv_Style;
	p->value(value);
	insert(p);
	return *p;
}

#endif
