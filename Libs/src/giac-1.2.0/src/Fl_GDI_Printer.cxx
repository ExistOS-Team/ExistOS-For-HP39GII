//
// "$Id: Fl_GDI_Printer.cxx,v 1.1 2004/11/24 20:31:46 cursorstar Exp $"
//
// WIN32 GDI printing device for the Fast Light Tool Kit (FLTK).
//
// Copyright (c) 2002  O'ksi'D
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
// Please report all bugs and problems to "oksid@bluewin.ch".
//
#include <stdlib.h>
#include <math.h>
#include <FL/Fl.H>
#include <windows.h>
#include <wingdi.h>
#include <FL/math.h>

#include <FL/Fl_GDI_Printer.H>
//#include <commdlg.h>
#include <FL/x.H>
#include <FL/fl_draw.H>

//#include <FL/Fl_Group.H>



extern FL_EXPORT void set_xmaps(HPEN tmppen_, HBRUSH tmpbrush_, HPEN savepen_, Fl_Brush *brushes_, Fl_XMap * fl_xmap_, Fl_XMap xmap_);
extern FL_EXPORT void pop_xmaps();
extern FL_EXPORT void push_xmaps();


Fl_GDI_Printer::~Fl_GDI_Printer(){

  
  if(nPages)
      EndPage(gc_);
  EndDoc(gc_);

  // cleaning pens&brushes...
  if(xmap.pen)
    DeleteObject((HGDIOBJ)(xmap.pen));
  int i;
  for(i=0; i<256; i++)
    if(fl_xmap[i].pen)
      DeleteObject((HGDIOBJ)(fl_xmap[i].pen));

  for(i=0; i<FL_N_BRUSH; i++)
  if(brushes[i].brush)
    DeleteObject(brushes[i].brush);
  //end clean
  
  DeleteDC(gc_);
  if(!GlobalUnlock(mode_))
    GlobalFree(mode_);
}


extern HDC fl_gc_save;

Fl_Output_Device * Fl_GDI_Printer::set_current(){
  if(fl == this) return this;
  Fl_Output_Device * c = fl;
  if(fl == &fltk){
    fl_gc_save = fl_gc;
    Fl::flush();
    push_xmaps();
  }
  set_xmaps(tmppen, tmpbrush, savepen, brushes, fl_xmap, xmap);
  fl = this;
  fl_gc = gc_;
  fl_clip_region(0);
  return c;
}




void Fl_GDI_Printer::draw(Fl_Widget * w){
  Fl_Output_Device * cd =fl;
  set_current();

  w->redraw(); // making dirty

  w->draw();

  Fl_Output_Device::current(cd);
}





////////////// gets the paper size from DEVMODE /////////////////////

static int gdipaper(DEVMODE  *mod){
  if(!(mod->dmFields & DM_PAPERSIZE)) return 0;

  int paper;
  switch (mod->dmPaperSize){
    case DMPAPER_A3:
      paper = Fl_Printer::A3; break;
    case DMPAPER_A4:
      paper = Fl_Printer::A4; break;
    case DMPAPER_A5:
      paper = Fl_Printer::A5; break;
    //case DMPAPER_A6: paper = FL_A6; break;
    case DMPAPER_B4:
      paper = Fl_Printer::B4; break;
    case DMPAPER_B5: paper = Fl_Printer::B5; break;
    //case DMPAPER_B6: paper = FL_B6; break;
    case DMPAPER_EXECUTIVE:
      paper = Fl_Printer::EXECUTIVE; break;
    case DMPAPER_FOLIO:
      paper = Fl_Printer::FOLIO; break;
    case DMPAPER_LEDGER:
      paper = Fl_Printer::LEDGER; break;
    case DMPAPER_LEGAL:
      paper = Fl_Printer::LEGAL; break;
    case DMPAPER_LETTER:
      paper = Fl_Printer::LETTER; break;
    case DMPAPER_TABLOID:
      paper = Fl_Printer::TABLOID; break;
    case DMPAPER_ENV_10:
      paper = Fl_Printer::ENVELOPE; break;
    default:
      paper = -1;
  }
  return paper;
}

////////////// sets the paper size in DEVMODE /////////////////////

static int gdipaper(DEVMODE  *mode, int format){
  switch(format){
    case Fl_Printer::A3: mode->dmPaperSize = DMPAPER_A3; break;
    case Fl_Printer::A4: mode->dmPaperSize = DMPAPER_A4; break;
    case Fl_Printer::A5: mode->dmPaperSize = DMPAPER_A5; break;

    case Fl_Printer::B4: mode->dmPaperSize = DMPAPER_B4; break;
    case Fl_Printer::B5: mode->dmPaperSize = DMPAPER_B5; break;

    case Fl_Printer::EXECUTIVE: mode->dmPaperSize = DMPAPER_EXECUTIVE; break;
    case Fl_Printer::FOLIO: mode->dmPaperSize = DMPAPER_FOLIO; break;
    case Fl_Printer::LEDGER: mode->dmPaperSize = DMPAPER_LEDGER; break;
    case Fl_Printer::LEGAL: mode->dmPaperSize = DMPAPER_LEGAL; break;
    case Fl_Printer::LETTER: mode->dmPaperSize = DMPAPER_LETTER; break;
    case Fl_Printer::TABLOID: mode->dmPaperSize = DMPAPER_TABLOID; break;
    case Fl_Printer::ENVELOPE: mode->dmPaperSize = DMPAPER_ENV_9; break;
    default: return -1;
  }
  return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////

Fl_GDI_Printer::Fl_GDI_Printer(HDC gc, DEVMODE * mode ):Fl_Printer(),/*delete_mode_(0),*/ style_(0),width_(0),dashes_(0),mask(0){
  
  gc_=gc;

  static DOCINFO DocInfo = { sizeof(DOCINFO), "FLTK Document", NULL,0 };// declare DocInfo for use and set the name of the print job as 'Name Of Document'
  mode_  = (DEVMODE *)GlobalLock(mode);
  int orientation_ = 0;
  if(mode_->dmOrientation==DMORIENT_LANDSCAPE)
    orientation_ = 1;
  int paper;
  if(mode_->dmFields & DM_PAPERSIZE){
    paper = gdipaper(mode_);
    if(mode_->dmOrientation == DMORIENT_PORTRAIT){
      pw_ = Fl_Printer::page_formats[paper][0];
      ph_ = Fl_Printer::page_formats[paper][1];
    }else{
      pw_ = Fl_Printer::page_formats[paper][1];
      ph_ = Fl_Printer::page_formats[paper][0];
    }
  }

  ResetDC(gc_,mode_);
  //GlobalUnlock(mode_);
  SetMapMode(gc_, MM_ANISOTROPIC);
  SetTextAlign(gc_, TA_BASELINE|TA_LEFT);
  SetBkMode(gc_, TRANSPARENT);
  StartDoc(gc_, &DocInfo);
  ix = GetDeviceCaps(gc_, LOGPIXELSX);
  iy = GetDeviceCaps(gc_, LOGPIXELSY);
  ox = GetDeviceCaps(gc_, PHYSICALOFFSETX);
  oy = GetDeviceCaps(gc_, PHYSICALOFFSETY);
  if(ix<iy)
	  max_res_=ix;
  else
	  max_res_=iy;

  nPages = 0;

  int i;
  // Unlike for static vars, here I have to zero the memory
  for(i=0;i<FL_N_BRUSH;i++){
    brushes[i].brush = 0;
    brushes[i].usage = 0;
    brushes[i].backref = 0;
  }
  tmppen = 0;
  tmpbrush = 0;
  savepen = 0;
  for(i=0;i<256;i++){
    fl_xmap[i].rgb = 0;
    fl_xmap[i].pen = 0;	// pen, 0 if none created yet
    fl_xmap[i].brush = 0;
  }
  xmap.rgb=0;
  xmap.brush =0;
  xmap.pen = 0;
  type_ = 0x200;
}


/////////////////////////////////   paging   //////////////////////////////////////


void Fl_GDI_Printer::set_page(int page){
  if(page){
    ResetDC(gc_,mode_);
    SetMapMode(gc_, MM_ANISOTROPIC);
    SetTextAlign(gc_, TA_BASELINE|TA_LEFT);
    SetBkMode(gc_, TRANSPARENT);
    StartPage(gc_);
    ix = GetDeviceCaps(gc_, LOGPIXELSX);
    iy = GetDeviceCaps(gc_, LOGPIXELSY);
    ox = GetDeviceCaps(gc_, PHYSICALOFFSETX);
    oy = GetDeviceCaps(gc_, PHYSICALOFFSETY);
  }
  SetViewportOrgEx(gc_,VOx =  - ox, VOy = - oy,0); //setting origin to the upper left corner
  SetViewportExtEx(gc_, VEx = (long)(ix * iy),  VEy = (long)(iy *ix ),0);
  SetWindowExtEx(gc_, WEx = iy * 72,  WEy = ix * 72, 0); //72 pixels per inch mapping
  SetWindowOrgEx(gc_, WOx = 0, WOy = 0,0);
}



void Fl_GDI_Printer::page(double pw, double ph, int media){
  if(nPages)
    EndPage(gc_);
  nPages++;
  pw_=pw;
  ph_=ph;
//  if(pw>ph)
//    orientation_ = 1;
  //DEVMODE * mode = (DEVMODE *)GlobalLock(mode_);
  if(ph>pw)
    mode_->dmOrientation = DMORIENT_PORTRAIT;
  else
    mode_->dmOrientation = DMORIENT_LANDSCAPE;
  if(media){
    mode_->dmPaperWidth = (int) (pw*254/72);
    mode_->dmPaperLength = (int) (ph*254/72);
    mode_->dmFields |= DM_PAPERLENGTH | DM_PAPERWIDTH;
    mode_->dmFields &= ~DM_PAPERSIZE;
  }
  //GlobalUnlock(mode_);
  set_page(1);
};


void Fl_GDI_Printer::page(int format){
 // DEVMODE * mode = (DEVMODE *)GlobalLock(mode_);
  if(nPages)
    EndPage(gc_);
  //DEVMODE * mode = (DEVMODE *)GlobalLock(mode_);
  if(format & LANDSCAPE){
    mode_->dmOrientation = DMORIENT_LANDSCAPE;
    pw_ = page_formats[format & 0xFF][1];
    ph_ = page_formats[format & 0xFF][0];
  }else{
    mode_->dmOrientation = DMORIENT_PORTRAIT;
    pw_ = page_formats[format & 0xFF][0];
    ph_ = page_formats[format & 0xFF][1];
  }
  if(format & MEDIA){
    if(!gdipaper(mode_, format& 0xFF)){
        mode_->dmFields &= ~DM_PAPERLENGTH & ~DM_PAPERWIDTH;
        mode_->dmFields |= DM_PAPERSIZE;
    }else{
       mode_->dmPaperWidth = (int) (pw_*254/72);
       mode_->dmPaperLength = (int) (ph_*254/72);
       mode_->dmFields |= DM_PAPERLENGTH | DM_PAPERWIDTH;
       mode_->dmFields &= ~DM_PAPERSIZE;
    }
  }
  if(format & LANDSCAPE){
    mode_->dmOrientation = DMORIENT_LANDSCAPE;
  }else{
    mode_->dmOrientation = DMORIENT_PORTRAIT;

  }
 
  //GlobalUnlock(mode_);
  set_page(1);
};

////////////////////////  margins and placing of drawinds on the page ///////////////////////////


void Fl_GDI_Printer::sty(int style, int width, char *dashes, int vex){
  if(!(width)){
    width=1;
    if(!style && !dashes)
      style |= FL_CAP_SQUARE | FL_JOIN_MITER; //adjustment for system drawings
  }
  if(!dashes || !(*dashes)){
    if((style & 0xff) && ((style & 0xf00)==FL_CAP_SQUARE)) // square caps do not seem to work with dasges, not sure why
      style = (style & ~0xf00)|FL_CAP_FLAT;
    Fl_Output_Device::line_style(style, width*vex);
  }else{

    // Following is shameless copy from original fl_line_style with modifications.
    // Has to be changed to avoid code redundance...

    static DWORD Cap[4]= {PS_ENDCAP_FLAT, PS_ENDCAP_FLAT, PS_ENDCAP_ROUND, PS_ENDCAP_SQUARE};
    static DWORD Join[4]={PS_JOIN_ROUND, PS_JOIN_MITER, PS_JOIN_ROUND, PS_JOIN_BEVEL};
    int s1 = PS_GEOMETRIC | Cap[(style>>8)&3] | Join[(style>>12)&3];
    DWORD a[16]; int n = 0;
    s1 |= PS_USERSTYLE;
    for (n = 0; n < 16 && *dashes; n++) a[n] = vex * *dashes++;
    if ((style || n) && !width) width = 1; // fix cards that do nothing for 0?
    LOGBRUSH penbrush = {BS_SOLID,fl_RGB(),0}; // can this be fl_brush()?
    HPEN newpen = ExtCreatePen(s1, vex * width, &penbrush, n, n ? a : 0);
    if (!newpen) {
      Fl::error("Fl_GDI_Printer::line_style(): Could not create GDI pen object.");
      return;
    }
    HPEN oldpen = (HPEN)SelectObject(fl_gc, newpen);
    DeleteObject(oldpen);
    fl_current_xmap->pen = newpen;
  }
}


void Fl_GDI_Printer::line_style(int style, int width, char * dashes){
   sty(style_ = style, width_ = width, dashes_ = dashes);
};

void Fl_GDI_Printer::place(double x, double y, double tx, double ty, double s){
//  SetViewportOrgEx(gc_, VOx = (long)(ix * (lm_+tx) /72 - ox + dx), VOy = (long)( iy *(tm_+th) /72 - oy + dy),0); //setting origin to the upper left corner inside margins

  SetViewportOrgEx(gc_, VOx = (long)(ix * tx /72 - ox ), VOy = (long)( iy *ty /72 - oy),0); //setting origin to the upper left corner inside margins
  SetWindowOrgEx(gc_, WOx = (long)x,  WOy = (long)y,0);
  SetViewportExtEx(gc_,  VEx =(long)(ix * iy *s),  VEy =(long)(iy * ix * s), 0);
  SetWindowExtEx(gc_, WEx = iy *72, WEy = ix * 72,0);
};

/*
void Fl_GDI_Printer::place(double x, double y, double w, double h, double tx, double ty, double tw, double th,int align){
  double dx, dy;
  double s = tw/w;

  if(s<(th/h)){
    dx = 0;
    dy = (th - s * h)*iy/144;
  }else{
    s=th/h;
    dy = 0;
    dx = (tw - s * w)*ix/144;
  }

  if(align & 3)
    if(align & FL_ALIGN_TOP)
      dy=0;
    else
      dy *= 2;
  if(align & 12)
    if(align & FL_ALIGN_LEFT)
      dx = 0;
    else
      dx *= 2;

//  SetViewportOrgEx(gc_, VOx = (long)(ix * (lm_+tx) /72 - ox + dx), VOy = (long)( iy *(tm_+th) /72 - oy + dy),0); //setting origin to the upper left corner inside margins

   SetViewportOrgEx(gc_, VOx = (long)(ix * tx /72 - ox + dx), VOy = (long)( iy * ty /72 - oy + dy),0); //setting origin to the upper left corner inside margins

  SetWindowOrgEx(gc_, WOx = (long)x,  WOy = (long)y,0);
  SetViewportExtEx(gc_,  VEx =(long)(ix * iy *s),  VEy =(long)(iy * ix * s), 0);
  SetWindowExtEx(gc_, WEx = iy *72, WEy = ix * 72,0);
};

*/
///////////////// we need to re-implement path drawings (double) for sub-pixel placement of vertices /////////////

FL_EXPORT void Fl_GDI_Printer::transformed_vertex(double x, double y){
  Fl_Output_Device::transformed_vertex((max_res_) * x, (max_res_) * y);
};

//struct matrix {double a, b, c, d, x, y;};

extern FL_EXPORT matrix * fl_get_matrix();

FL_EXPORT void Fl_GDI_Printer::vertex(double x, double y){
         transformed_vertex(fl_transform_x(x,y), fl_transform_y(x,y));
};

extern int fl_what_loop;
enum {LINE, LOOP, POLYGON, POINT_};

FL_EXPORT void Fl_GDI_Printer::circle(double x, double y, double r){
  matrix * m = fl_get_matrix();
  double xt = fl_transform_x(x,y);
  double yt = fl_transform_y(x,y);
/*
// very ugly hack because m matrix is static, to be modified.
  double ma = fl_transform_dx(1,0);
  double mb = fl_transform_dy(1,0);
  double mc = fl_transform_dx(0,1);
  double md = fl_transform_dy(0,1);
*/

  double rx = r * (m->c ? sqrt(m->a*m->a+m->c*m->c) : fabs(m->a));
  double ry = r * (m->b ? sqrt(m->b*m->b+m->d*m->d) : fabs(m->d));
  double llx = xt-rx;
  double w = xt + rx - llx;
  double lly = yt - ry;
  double h = yt + ry - lly;

  SetWindowExtEx(gc_, WEx*(max_res_), WEy*(max_res_), 0);
  SetWindowOrgEx(gc_, WOx*(max_res_) ,  WOy*(max_res_) ,0);

  if (fl_what_loop==POLYGON) {
    SelectObject(fl_gc, fl_brush());
    Pie(fl_gc, (int)rint((max_res_)*llx), (int)rint((max_res_)*lly), (int)rint((max_res_)*(llx+w)), (int)rint((max_res_)*(lly+h)), 0, 0, 0, 0);
	  SetWindowExtEx(gc_, WEx, WEy, 0);
	  SetWindowOrgEx(gc_, WOx, WOy,0);
  }else{
    sty(style_, width_, dashes_, max_res_);
    Arc(fl_gc, (int)rint((max_res_)*llx), (int)rint((max_res_)*lly), (int)rint((max_res_)*(llx+w)), (int)rint((max_res_)*(lly+h)), 0, 0, 0, 0);
	  SetWindowExtEx(gc_, WEx, WEy, 0);
	  SetWindowOrgEx(gc_, WOx, WOy,0);
    sty(style_, width_, dashes_);
  }

}


void Fl_GDI_Printer::set_subpixel(){
  SetWindowExtEx(gc_, WEx*(max_res_), WEy*(max_res_), 0);
  SetWindowOrgEx(gc_, WOx*(max_res_),  WOy*(max_res_) ,0); 
  sty(style_, width_, dashes_, max_res_);
};

void Fl_GDI_Printer::set_normal(){
  SetWindowExtEx(gc_, WEx, WEy, 0);
  SetWindowOrgEx(gc_, WOx, WOy,0);
  sty(style_, width_, dashes_);
};

void Fl_GDI_Printer::rectf(int x, int y, int w, int h){
  //Fl_Output_Device::rectf(x,y,w,h);
  SetWindowExtEx(gc_, WEx * max_res_, WEy*(max_res_), 0);
  SetWindowOrgEx(gc_, WOx * max_res_,  WOy*(max_res_) ,0);
  Fl_Output_Device::rectf((int)(max_res_ * (x-0.5)),(int)(max_res_ * (y-0.5)),(int)(max_res_*(w)), (int)(max_res_*(h)));
  SetWindowExtEx(gc_, WEx, WEy, 0);
  SetWindowOrgEx(gc_, WOx, WOy,0);
};

void Fl_GDI_Printer::point(int x, int y){
  rectf(x,y,1,1);
};


void Fl_GDI_Printer::arc(int x, int y, int w, int h, double a1, double a2){
  set_subpixel();
  Fl_Output_Device::arc((int)(max_res_ * (x)),(int)(max_res_ * (y)),(int)(max_res_*(w-1)), (int)(max_res_*(h-1)), a1, a2);
  set_normal();
};
void Fl_GDI_Printer::pie(int x, int y, int w, int h, double a1, double a2){
  set_subpixel();
  Fl_Output_Device::pie((int)(max_res_ * (x)),(int)(max_res_ * (y)),(int)(max_res_*(w-1)), (int)(max_res_*(h-1)), a1, a2);
  set_normal();
};


void Fl_GDI_Printer::xyline(int x,int y, int x1){
  MoveToEx(fl_gc, x, y, 0); LineTo(fl_gc, x1, y);
}

void Fl_GDI_Printer::yxline(int x, int y, int y1){
  MoveToEx(fl_gc, x, y, 0); LineTo(fl_gc, x, y1);
}

void Fl_GDI_Printer::xyline(int x, int y, int x1, int y2) {
  MoveToEx(fl_gc, x, y, 0); 
  LineTo(fl_gc, x1, y);
  LineTo(fl_gc, x1, y2);
}

void Fl_GDI_Printer::xyline(int x, int y, int x1, int y2, int x3) {
  MoveToEx(fl_gc, x, y, 0); 
  LineTo(fl_gc, x1, y);
  LineTo(fl_gc, x1, y2);
  LineTo(fl_gc, x3, y2);
};

void Fl_GDI_Printer::yxline(int x, int y, int y1, int x2) {
  MoveToEx(fl_gc, x, y, 0); 
  LineTo(fl_gc, x, y1);
  LineTo(fl_gc, x2, y1);
}

void Fl_GDI_Printer::yxline(int x, int y, int y1, int x2, int y3) {
  MoveToEx(fl_gc, x, y, 0); 
  LineTo(fl_gc, x, y1);
  LineTo(fl_gc, x2, y1);
  LineTo(fl_gc, x2, y3);
};


void Fl_GDI_Printer::line(int x, int y, int x1, int y1) {
  MoveToEx(fl_gc, x, y, 0L); 
  LineTo(fl_gc, x1, y1);
};


void Fl_GDI_Printer::line(int x, int y, int x1, int y1, int x2, int y2) {
  MoveToEx(fl_gc, x, y, 0L); 
  LineTo(fl_gc, x1, y1);
  LineTo(fl_gc, x2, y2);
};

FL_EXPORT void Fl_GDI_Printer::end_line(){
  //LPSIZE l;
  //GetViewportExtEx( gc_, l );
  /*
  SetWindowExtEx(gc_, WEx*(max_res_), WEy*(max_res_), 0);
  SetWindowOrgEx(gc_, WOx*(max_res_),  WOy*(max_res_) ,0); 
  //GetViewportExtEx( gc_, l );
  sty(style_, width_, dashes_, max_res_);
  */
  set_subpixel();
  Fl_Output_Device::end_line();
  set_normal();
  /*
  SetWindowExtEx(gc_, WEx, WEy, 0);
  SetWindowOrgEx(gc_, WOx, WOy,0);
  sty(style_, width_, dashes_);
  */
};


FL_EXPORT void Fl_GDI_Printer::end_polygon(){
  set_subpixel();
  /*
  SetWindowExtEx(gc_, WEx*(max_res_), WEy*(max_res_), 0);
  SetWindowOrgEx(gc_, WOx * (max_res_), WOy *(max_res_),0);
  sty(style_, width_, dashes_, max_res_);
  */
  Fl_Output_Device::end_polygon();
  set_normal();
  /*
  SetWindowExtEx(gc_, WEx, WEy, 0);
  SetWindowOrgEx(gc_, WOx, WOy,0);
  sty(style_, width_, dashes_);
  */
};


FL_EXPORT void Fl_GDI_Printer::end_loop(){
  set_subpixel();
  Fl_Output_Device::end_loop();
  set_normal();
};

FL_EXPORT void Fl_GDI_Printer::end_complex_polygon(){
  set_subpixel();
  Fl_Output_Device::end_complex_polygon();
  set_normal();
};




////////  clipping, need to be re-implemented as orig. win32 functions are in device coordinates  ///////////

FL_EXPORT void Fl_GDI_Printer::push_clip(int x, int y, int w, int h){
  Fl_Output_Device::push_clip(((x - WOx) * VEx - VEx/2) / WEx + VOx,  ((y - WOy) * VEy - VEy/2 ) / WEy + VOy, w * VEx / WEx, h * VEy / WEy);
}

extern Fl_Region * fl_clip_stack;
extern int * fl_clip_stack_pointer;

FL_EXPORT int Fl_GDI_Printer::not_clipped(int x, int y, int w, int h){
  Fl_Region r = fl_clip_stack[* fl_clip_stack_pointer];
  if (!r) return 1;
  RECT rect;
  rect.left = ((x - WOx) * VEx -VEx/2)/ WEx + VOx;
  rect.top = ((y - WOy) * VEy-VEy/2) / WEy + VOy;
  rect.right  = ((x + w - WOx) * VEx  - VEx/2)/ WEx + VOx;
  rect.bottom  = ((y + h - WOy) * VEy-VEy/2) / WEy + VOy;
  return RectInRegion(r,&rect);
}

FL_EXPORT int Fl_GDI_Printer::clip_box(int x, int y, int w, int h, int &X, int &Y, int &W, int &H){
  int ret = Fl_Output_Device::clip_box(((x - WOx) * VEx - VEx/2)/ WEx + VOx,  ((y - WOy) * VEy -VEy/2)/ WEy + VOy, w * VEx / WEx, h * VEy / WEy, X, Y, W, H);
  X = (X - VOx ) * WEx / VEx + WOx;
  Y = (Y - VOy ) * WEy / VEy + WOy;
  W = W * WEx / VEx;
  H = H * WEy / VEy;
  return ret;
}

// -
//extern uchar **fl_mask_bitmap;


