#include <stdio.h>
#include <math.h>
#include <string.h>

#include <FL/Fl_PS_Printer.H>
#include <FL/Fl.H>
//#include <FL/Fl_Printer.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_Bitmap.H>





static int dashes_flat[5][7]={
  {-1,0,0,0,0,0,0},
  {3,1,-1,0,0,0,0},
  {1,1,-1,0,0,0,0},
  {3,1,1,1,-1,0,0},
  {3,1,1,1,1,1,-1}
};


//yeah, hack...
static double dashes_cap[5][7]={
  {-1,0,0,0,0,0,0},
  {2,2,-1,0,0,0,0},
  {0.01,1.99,-1,0,0,0,0},
  {2,2,0.01,1.99,-1,0,0},
  {2,2,0.01,1.99,0.01,1.99,-1}
};





  


////////////////////// Prolog string ////////////////////////////////////////


static const char * prolog =
"%%%%BeginProlog\n"
"/L { /y2 exch def\n"
  "/x2 exch def\n"
  "/y1 exch def\n"
  "/x1 exch def\n"
  "newpath   x1 y1 moveto x2 y2 lineto\n"
  "stroke}\n"
"bind def\n"


"/R { /dy exch def\n"
  "/dx exch def\n"
  "/y exch def\n"
  "/x exch def\n"
  "newpath\n"
  "x y moveto\n"
  "dx 0 rlineto\n"
  "0 dy rlineto\n"
  "dx neg 0 rlineto\n"
  "closepath stroke\n"
"} bind def\n"

"/CL {\n"
  "/dy exch def\n"
  "/dx exch def\n"
  "/y exch def\n"
  "/x exch def\n"
  "newpath\n"
  "x y moveto\n"
  "dx 0 rlineto\n"
  "0 dy rlineto\n"
  "dx neg 0 rlineto\n"
  "closepath\n"
  "clip\n"
"} bind def\n"

"/FR { /dy exch def\n"
  "/dx exch def\n"
  "/y exch def\n"
  "/x exch def\n"
  "currentlinewidth 0 setlinewidth newpath\n"
  "x y moveto\n"
  "dx 0 rlineto\n"
  "0 dy rlineto\n"
  "dx neg 0 rlineto\n"
  "closepath fill setlinewidth\n"
"} bind def\n"

"/GS { gsave } bind  def\n"
"/GR { grestore } bind def\n"

"/SP { showpage } bind def\n"
"/LW { setlinewidth } bind def\n"
"/CF /Courier def\n"
"/SF { /CF exch def } bind def\n"
"/fsize 12 def\n"
"/FS { /fsize exch def fsize CF findfont exch scalefont setfont }def \n"


"/GL { setgray } bind def\n"
"/SRGB { setrgbcolor } bind def\n"

//////////////////// color images ////////////////////////

"/CI { GS /py exch def /px exch def /sy exch def /sx exch def\n"
  "translate \n"
  "sx sy scale px py 8 \n"
  "[ px 0 0 py neg 0 py ]\n"
  "currentfile /ASCIIHexDecode filter\n false 3"
  " colorimage GR\n"
"} bind def\n"

///////////////////  gray images //////////////////////////

"/GI { GS /py exch def /px exch def /sy exch def /sx exch def \n"
  "translate \n"
  "sx sy scale px py 8 \n"


  "[ px 0 0 py neg 0 py ]\n"
  "currentfile /ASCIIHexDecode filter\n"
  "image GR\n"
"} bind def\n"

////////////////// single-color bitmask ///////////////////

"/MI { GS /py exch def /px exch def /sy exch def /sx exch def \n"
  "translate \n"
  "sx sy scale px py false \n"
  "[ px 0 0 py neg 0 py ]\n"
  "currentfile /ASCIIHexDecode filter\n"
  "imagemask GR\n"
"} bind def\n"

////////////////   color image dict /////////////

"/CII {GS /inter exch def /py exch def /px exch def /sy exch def /sx exch def \n"
  "translate \n"
  "sx sy scale\n"
  "/DeviceRGB setcolorspace\n"
  "/IDD 8 dict def\n"
  "IDD begin\n"
    "/ImageType 1 def\n"
    "/Width px def\n"
    "/Height py def\n"
    "/BitsPerComponent 8 def\n"
    "/Interpolate inter def\n"
    "/DataSource currentfile /ASCIIHexDecode filter def\n"
    "/MultipleDataSources false def\n"
    "/ImageMatrix [ px 0 0 py neg 0 py ] def\n"
    "/Decode [ 0 1 0 1 0 1 ] def\n"
  "end\n"
"IDD image GR} bind def\n"

//////////////// gray image dict ///////////////////


"/GII {GS /inter exch def /py exch def /px exch def /sy exch def /sx exch def \n"
  "translate \n"












  "sx sy scale\n"
  "/DeviceGray setcolorspace\n"
  "/IDD 8 dict def\n"
  "IDD begin\n"
    "/ImageType 1 def\n"
    "/Width px def\n"
    "/Height py def\n"
    "/BitsPerComponent 8 def\n"

    "/Interpolate inter def\n"
    "/DataSource currentfile /ASCIIHexDecode filter def\n"
    "/MultipleDataSources false def\n"
    "/ImageMatrix [ px 0 0 py neg 0 py ] def\n"
    "/Decode [ 0 1 ] def\n"
  "end\n"
"IDD image GR} bind def\n"


///////////////////  masked color images   ///////
"/CIM {GS /inter exch def /my exch def /mx exch def /py exch def /px exch def /sy exch def /sx exch def \n"
  "translate \n"
  "sx sy scale\n"
  "/DeviceRGB setcolorspace\n"
  


"/IDD 8 dict def\n"

"IDD begin\n"
    "/ImageType 1 def\n"
    "/Width px def\n"
    "/Height py def\n"
    "/BitsPerComponent 8 def\n"
  "/Interpolate inter def\n"
    "/DataSource currentfile /ASCIIHexDecode filter def\n"
    "/MultipleDataSources false def\n"
    "/ImageMatrix [ px 0 0 py neg 0 py ] def\n"

    "/Decode [ 0 1 0 1 0 1 ] def\n"
"end\n"

"/IMD 8 dict def\n"
"IMD begin\n"
    "/ImageType 1 def\n"
    "/Width mx def\n"           
    "/Height my def\n"
    "/BitsPerComponent 1 def\n"
//  "/Interpolate inter def\n"
    "/ImageMatrix [ mx 0 0 my neg 0 my ] def\n"
    "/Decode [ 1 0 ] def\n"
"end\n"

"<<\n"
  "/ImageType 3\n"
  "/InterleaveType 2\n"
  "/MaskDict IMD\n"
  "/DataDict IDD\n"
">> image GR\n"
"} bind def\n"



///////////////////  masked gray images   ////////////////





"/GIM {GS /inter exch def /my exch def /mx exch def /py exch def /px exch def /sy exch def /sx exch def \n"
  "translate \n"
  "sx sy scale\n"
  "/DeviceGray setcolorspace\n"

  "/IDD 8 dict def\n"


  "IDD begin\n"
    "/ImageType 1 def\n"
    "/Width px def\n"
    "/Height py def\n"
    "/BitsPerComponent 8 def\n"
    "/Interpolate inter def\n"
    "/DataSource currentfile /ASCIIHexDecode filter def\n"
    "/MultipleDataSources false def\n"
    "/ImageMatrix [ px 0 0 py neg 0 py ] def\n"

    "/Decode [ 0 1 ] def\n"
  "end\n"

  "/IMD 8 dict def\n"

  "IMD begin\n"
    "/ImageType 1 def\n"
    "/Width mx def\n"           
    "/Height my def\n"
    "/BitsPerComponent 1 def\n"
    "/ImageMatrix [ mx 0 0 my neg 0 my ] def\n"
    "/Decode [ 1 0 ] def\n"
  "end\n"

  "<<\n"
    "/ImageType 3\n"
    "/InterleaveType 2\n"
    "/MaskDict IMD\n"
    "/DataDict IDD\n"
  ">> image GR\n"
"} bind def\n"


"\n"
        ///////////////////////////  path ////////////////////

"/BFP { newpath moveto }  def\n"
"/BP { newpath } bind def \n"
"/PL { lineto } bind def \n"
"/PM { moveto } bind def \n"
"/MT { moveto } bind def \n"
"/LT { lineto } bind def \n"
"/EFP { closepath fill } bind def\n"  //was:stroke
"/ELP { stroke } bind def\n"  
"/ECP { closepath stroke } bind def\n"  // Closed (loop)
"/LW { setlinewidth } bind def\n"

        //////////////////////////// misc ////////////////
"/TR { translate } bind def\n"
"/CT { concat } bind def\n"
"/RCT { matrix invertmatrix concat} bind def\n"
"/SC { scale } bind def\n"
//"/GPD { currentpagedevice /PageSize get} def\n"

;


////////////////////// end prolog ////////////////////////

//////////////////////  fonts   ////////////////////////////////////

static const char *_fontNames[] = {
  "Helvetica",
  "Helvetica-Bold",
  "Helvetica-Oblique",
  "Helvetica-BoldOblique",
  "Courier",
  "Courier-Bold",
  "Courier-Oblique",
  "Courier-BoldOblique",
  "Times",
  "Times-Bold",
  "Times-Italic",

  "Times-BoldItalic",
  "Symbol",
  "Courier",
  "CourierBold",
  "ZapfDingbats"
};

struct matrix {double a, b, c, d, x, y;};
extern matrix  * fl_get_matrix();


///////////////////////// Implementations : matrix ////////////////////////////////////////

void Fl_PS_Printer::concat(){
  matrix * m = fl_get_matrix();
  //double a,b,c,d,x,y;
  //fl_matrix(a,b,c,d,x,y);
  fprintf(output,"[%g %g %g %g %g %g] CT\n", m->a , m->b , m->c , m->d , m->x , m->y);
}

void Fl_PS_Printer::reconcat(){
  matrix * m = fl_get_matrix();
  //double a,b,c,d,x,y;
  //fl_matrix(a,b,c,d,x,y);
  fprintf(output, "[%g %g %g %g %g %g] RCT\n" , m->a , m->b , m->c , m->d , m->x , m->y);
}


//////////////// for language level <3 ///////////////////////

void Fl_PS_Printer::recover(){
  //if (colored_) 
    color(cr_,cg_,cb_);
  //if (line_styled_) 
    line_style(linestyle_,linewidth_,linedash_);
  //if (fonted_) 
    font(font_,size_);
  //colored_=line_styled_=fonted_=0;
};
  
void Fl_PS_Printer::reset(){
  gap_=1;
  clip_=0;
  cr_=cg_=cb_=0;
  font_=FL_HELVETICA;
  size_=12;
  linewidth_=0;
  linestyle_=FL_SOLID;
  strcpy(linedash_,"");
  Clip *c=clip_;   ////just not to have memory leaks for badly writen code (forgotten clip popping)

  while(c){
    clip_=clip_->prev;
    delete c;
    c=clip_;
  }
  //line_style(0);
  //colored_=1;
  //line_styled_=1;
  //fonted_=1;
};



///////////////// destructor, finishes postscript, closes FILE  ///////////////
 
Fl_PS_Printer::~Fl_PS_Printer() {
  if(nPages){  // for eps nPages is 0 so it is fine ....
    fprintf(output, "CR\nGR\n GR\nSP\n restore\n");
    if(!pages_){
      fprintf(output, "%%%%Trailer\n");
      fprintf(output, "%%%%Pages: %i\n" , nPages);
    };
  }else
    fprintf(output, "GR\n restore\n");
  reset();
  //fclose(output);

  while(clip_){
    Clip * c= clip_;
    clip_= clip_->prev;
    delete c;
  }
  if(close_cmd_)
    (*close_cmd_)(output);

}


///////////////// PostScript constructors /////////////////////////////////////


Fl_PS_Printer::Fl_PS_Printer(FILE *o, int lang_level, int pages):clip_(0),interpolate_(0){
  close_cmd_=0;
  lang_level_=lang_level;
  output=o;
  mask = 0;
  //orientation_=0;
  //orientation_=orientation;
//  lm_=rm_=bm_=tm_=72;
  //pw_ =pw;
  //ph_= ph;
  bg_=FL_GRAY;
  fprintf(output, "%%!PS-Adobe-3.0\n");
  if(lang_level_>1)
    fprintf(output, "%%%%LanguageLevel: %i\n" , lang_level_);
  if(pages_==pages)
    fprintf(output, "%%%%Pages: %i\n", pages);
  else
    fprintf(output, "%%%%Pages: (atend)\n");
  fprintf(output, "%%%%EndComments\n");
  fprintf(output, prolog);
  if(lang_level_>=3){
    fprintf(output, "/CS { clipsave } bind def\n");
    fprintf(output, "/CR { cliprestore } bind def\n");
  }else{
    fprintf(output, "/CS { GS } bind def\n");
    fprintf(output, "/CR { GR } bind def\n");
  }
  page_policy_=1;
 

  fprintf(output, "%%%%EndProlog\n");
  if(lang_level_>=2)
    fprintf(output,"<< /Policies << /Pagesize 1 >> >> setpagedevice\n");

  reset();
  nPages=0;
  type_ = 0x100;

};

void Fl_PS_Printer::page_policy(int p){
  page_policy_ = p;
  if(lang_level_>=2)
    fprintf(output,"<< /Policies << /Pagesize %i >> >> setpagedevice\n", p);
};


/////////////////////////////////////////////////////
 /*
Fl_PS_Printer::Fl_PS_Printer(FILE *o, int lang_level, int pages)
  :clip_(0),interpolate_(0)
{
  lang_level_=lang_level;
  output=o;
  mask = 0;
  //clip_=0;
  lm_=rm_=bm_=tm_=72;
  bg_=FL_GRAY;

  if (orientation&1){
    ph_= Fl_Printer::page_formats[format][0];
    pw_= Fl_Printer::page_formats[format][1];
  }else{
    ph_= Fl_Printer::page_formats[format][1];
    pw_= Fl_Printer::page_formats[format][0];
  }

  pw_ = ph_ = 0;
  fprintf(output, "%%!PS-Adobe-3.0\n");
  if(lang_level_>1)
    fprintf(output, "%%%%LanguageLevel: %i\n" , lang_level_);
  if(pages)
    fprintf(output, "%%%%Pages: %i\n", pages);
  else
    fprintf(output, "%%%%Pages: (atend)\n");

  fprintf(output, prolog);
  if(lang_level_>=3){
    fprintf(output, "/CS { clipsave } bind def\n");
    fprintf(output, "/CR { cliprestore } bind def\n");
  }else{
    fprintf(output, "/CS { GS } bind def\n");
    fprintf(output, "/CR { GR } bind def\n");
  }
  fprintf(output, "%%%%EndProlog\n");

  reset();
  nPages=0;
  line_style(0);

};

*/

///////////////////  eps constructor ////////////////////////////////////
  
Fl_PS_Printer::Fl_PS_Printer(FILE *o, int lang_level, int x, int y, int w, int h)
  :clip_(0),interpolate_(0)
{
  close_cmd_=0;
  output=o;
  mask = 0;
  pages_=0;
  //clip_=0;
  lang_level_=lang_level;
  bg_=FL_GRAY;
  fprintf(output, "%%!PS-Adobe-3.0 EPSF-3.0\n");
  if(lang_level_>1)
    fprintf(output, "%%%%LanguageLevel: %i\n" , lang_level_);
  fprintf(output, "%%%%BoundingBox: %i %i %i %i\n", x , y , x+w , y+h);
  width_ = w;
  height_ = h;
 // lm_=x;
 // tm_=0;
 // rm_=0;
 // bm_=y;
  fprintf(output, prolog);
  if(lang_level_>=3){
    fprintf(output, "/CS { clipsave } bind def\n");
    fprintf(output, "/CR { cliprestore } bind def\n");
  }else{
    fprintf(output, "/CS { GS } bind def\n");
    fprintf(output, "/CR { GR } bind def\n");
  }
  fprintf(output, "%%%%EndProlog\n");
  fprintf(output, "%%%%Page: 1 1\n");
  fprintf(output, "%%%%PageOrientation: Portrait\n");
  fprintf(output, "save\n");
  fprintf(output, "GS\n");

  reset();
  fprintf(output, "%g %g TR\n", double(x) , double(y+h));
  fprintf(output, "1 -1  SC\n");
  fprintf(output, "GS\nCS\n");
  line_style(0);

  nPages=0;  //must be 0 also for eps!
  type_= 0x100;

};


////////////////////// paging //////////////////////////////////////////
/*
void Fl_PS_Printer::page(){
  if (nPages){
    fprintf(output, "CR\nGR\nGR\nSP\nrestore\n");
  }
  ++nPages;
  fprintf(output, "%%%%Page: %i %i\n" , nPages , nPages);
  fprintf(output, "save\n");
  fprintf(output, "GS\n");
  if(ph_)
     fprintf(output, "%g %g TR\n", lm_ , ph_ - tm_);
  else{
    fprintf(output, "%g GPD exch pop %g sub TR\n", lm_, tm_);
  }
  fprintf(output, "1 -1 SC\n");
  fprintf(output, "GS\nCS\n");
};
*/


void Fl_PS_Printer::page(double pw, double ph, int media) {

  if (nPages){
    fprintf(output, "CR\nGR\nGR\nSP\nrestore\n");
  }

  ++nPages;
  fprintf(output, "%%%%Page: %i %i\n" , nPages , nPages);



  if (pw>ph){
    fprintf(output, "%%%%PageOrientation: Landscape\n");
    //fprintf(output, "%i Orientation\n", 1);
  }else{
    fprintf(output, "%%%%PageOrientation: Portrait\n");
    //fprintf(output, "%i Orientation\n", 0);
  }



  fprintf(output, "%%%%BeginPageSetup\n");

  if((media & MEDIA) &&(lang_level_>1)){
      int r = media & REVERSED;
      if(r) r = 2;
      fprintf(output, "<< /PageSize [%i %i] /Orientation %i>> setpagedevice\n", (int)(pw+.5), (int)(ph+.5), r);
  }else
    if(pw>ph)
      if(media & REVERSED)
        fprintf(output, "-90 rotate %i 0 translate\n", int(-pw));
      else
        fprintf(output, "90 rotate 0 %i translate\n", int(-ph));
    else
      if(media & REVERSED)
        fprintf(output, "180 rotate %i %i translate\n", int(-pw), int(-ph));
  



  fprintf(output, "%%%%EndPageSetup\n");


  pw_=pw;
  ph_=ph;

  reset();

  fprintf(output, "save\n");
  fprintf(output, "GS\n");

  fprintf(output, "%g %g TR\n", (double)0 /*lm_*/ , ph_ /* - tm_*/);
  fprintf(output, "1 -1 SC\n");
  line_style(0);
  fprintf(output, "GS\nCS\n");

};


void Fl_PS_Printer::page(int format){


  //orientation_=orientation;
  if(format &  LANDSCAPE){
    ph_=Fl_Printer::page_formats[format & 0xFF][0];
    pw_=Fl_Printer::page_formats[format & 0xFF][1];
  }else{
    pw_=Fl_Printer::page_formats[format & 0xFF][0];
    ph_=Fl_Printer::page_formats[format & 0xFF][1];
  }
  page(pw_,ph_,format & 0xFF00);//,orientation only;
};




void Fl_PS_Printer::place(double x, double y, double tx, double ty, double scale){

 fprintf(output, "CR\nGR\nGS\n");
 reset();
 fprintf(output, "%g %g TR\n", -x*scale + tx , -y*scale + ty);
 fprintf(output, "%g %g SC\n", scale , scale );
 fprintf(output, "CS\n");


}








//////////////////////////////  setting background for alpha /////////////////////////////////

void Fl_PS_Printer::bg_color(Fl_Color bg){bg_=bg;};

//////////////////////////////// Primitives: Colors  ////////////////////////////////////////////

void Fl_PS_Printer::color(Fl_Color c) {
  //colored_=1;
  color_=c;
  Fl::get_color(c, cr_, cg_, cb_);
  if (cr_==cg_ && cg_==cb_) {
    double gray = cr_/255.0;
    fprintf(output, "%g GL\n", gray);

  } else {
    double fr, fg, fb;
    fr = cr_/255.0;
    fg = cg_/255.0;
    fb = cb_/255.0;
    fprintf(output,"%g %g %g SRGB\n", fr , fg , fb);
  }
}

void Fl_PS_Printer::color(unsigned char r, unsigned char g, unsigned char b) {

  //colored_=1;
  cr_=r;cg_=g;cb_=b;
  if (r==g && g==b) {
    double gray = r/255.0;
    fprintf(output, "%g GL\n", gray);
  } else {
    double fr, fg, fb;
    fr = r/255.0;
    fg = g/255.0;
    fb = b/255.0;
    fprintf(output, "%g %g %g SRGB\n", fr , fg , fb);
  }
}

/////////////////////////////   Clipping /////////////////////////////////////////////

void Fl_PS_Printer::push_clip(int x, int y, int w, int h) {
  Clip * c=new Clip();
  clip_box(x,y,w,h,c->x,c->y,c->w,c->h);
  c->prev=clip_;
  clip_=c;
  fprintf(output, "CR\nCS\n");
  if(lang_level_<3)
    recover();
  fprintf(output, "%g %g %i %i CL\n", clip_->x-0.5 , clip_->y-0.5 , clip_->w  , clip_->h);

}
void Fl_PS_Printer::push_no_clip() {
  Clip * c = new Clip();
  c->prev=clip_;
  clip_=c;
  clip_->x = clip_->y = clip_->w = clip_->h = -1;
  fprintf(output, "CR\nCS\n");
  if(lang_level_<3)
    recover();
}

void Fl_PS_Printer::pop_clip() {
  if(!clip_)return;
  Clip * c=clip_;
  clip_=clip_->prev;
  delete c;
  fprintf(output, "CR\nCS\n");
  if(clip_ && clip_->w >0)
    fprintf(output, "%g %g %i %i CL\n", clip_->x - 0.5, clip_->y - 0.5, clip_->w  , clip_->h);
    // uh, -0.5 is to match screen clipping, for floats there should be something beter
  if(lang_level_<3)
    recover();
}



int Fl_PS_Printer::clip_box(int x, int y, int w, int h, int &X, int &Y, int &W, int &H){
  if(!clip_){
    X=x;Y=y;W=w;H=h;
    return 1;
  }
  if(clip_->w < 0){
    X=x;Y=y;W=w;H=h;
    return 1;
  }
  int ret=0;
  if (x > (X=clip_->x)) {X=x; ret=1;}
  if (y > (Y=clip_->y)) {Y=y; ret=1;}
  if ((x+w) < (clip_->x+clip_->w)) {
    W=x+w-X;

    ret=1;

  }else
    W = clip_->x + clip_->w - X;
  if(W<0){
    W=0;
    return 1;
  }
  if ((y+h) < (clip_->y+clip_->h)) {
    H=y+h-Y;
    ret=1;
  }else
    H = clip_->y + clip_->h - Y;
  if(H<0){
    W=0;
    H=0;
    return 1;
  }
  return ret;
};


int Fl_PS_Printer::not_clipped(int x, int y, int w, int h){
  if(!clip_) return 1;
  if(clip_->w < 0) return 1;
  int X, Y, W, H;
  clip_box(x, y, w, h, X, Y, W, H);
  if(W) return 1;
  return 0;
};


///////////////////////////////// rect  /////////////////////////////////////////





void Fl_PS_Printer::rect(int x, int y, int w, int h) {
// Commented code does not work, i can't find the bug ;-(
// fprintf(output, "GS\n");
//  fprintf(output, "%i, %i, %i, %i R\n", x , y , w, h);
//  fprintf(output, "GR\n");


  fprintf(output, "GS\n");
  fprintf(output,"BP\n");
  fprintf(output, "%i %i MT\n", x , y);
  fprintf(output, "%i %i LT\n", x+w-1 , y);
  fprintf(output, "%i %i LT\n", x+w-1 , y+h-1);
  fprintf(output, "%i %i LT\n", x , y+h-1);
  fprintf(output, "ECP\n");
  fprintf(output, "GR\n");
}



void Fl_PS_Printer::rectf(int x, int y, int w, int h) {
  fprintf(output, "%g %g %i %i FR\n", x-0.5, y-0.5, w, h);
}

void Fl_PS_Printer::point(int x, int y){
  rectf(x,y,1,1);
}

void Fl_PS_Printer::rectf(int x, int y, int w, int h, uchar r, uchar g, uchar b) {

  fprintf(output, "GS\n");
  double fr = r/255.0;
  double fg = g/255.0;
  double fb = b/255.0;
  fprintf(output, "%g %g %g SRGB\n",fr , fg , fb);
  rectf(x,y,w,h);
  //fprintf(output, "%i %i %i %i FR\n", x , y , w  , h );
  fprintf(output, "GR\n");
}

///////////////////////////////// lines  /////////////////////////////////////////

void Fl_PS_Printer::line(int x1, int y1, int x2, int y2) {
  fprintf(output, "GS\n");
  fprintf(output, "%i %i %i %i L\n", x1 , y1, x2 ,y2);
  fprintf(output, "GR\n");
}

void Fl_PS_Printer::line(int x0, int y0, int x1, int y1, int x2, int y2) {
  fprintf(output, "GS\n");
  fprintf(output,"BP\n");
  fprintf(output, "%i %i MT\n", x0 , y0);
  fprintf(output, "%i %i LT\n", x1 , y1);
  fprintf(output, "%i %i LT\n", x2 , y2);
  fprintf(output, "ELP\n");
  fprintf(output, "GR\n");
}

void Fl_PS_Printer::loop(int x0, int y0, int x1, int y1, int x2, int y2) {
  fprintf(output, "GS\n");
  fprintf(output,"BP\n");
  fprintf(output, "%i %i MT\n", x0 , y0);
  fprintf(output, "%i %i LT\n", x1 , y1);
  fprintf(output, "%i %i LT\n", x2 , y2);
  fprintf(output, "ECP\n");
  fprintf(output, "GR\n");
}

void Fl_PS_Printer::loop(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) {
  fprintf(output, "GS\n");
  fprintf(output,"BP\n");
  fprintf(output, "%i %i MT\n", x0 , y0);
  fprintf(output, "%i %i LT\n", x1 , y1);
  fprintf(output, "%i %i LT\n", x2 , y2);
  fprintf(output, "%i %i LT\n", x3 , y3);
  fprintf(output, "ECP\n");
  fprintf(output, "GR\n");
}

void Fl_PS_Printer::polygon(int x0, int y0, int x1, int y1, int x2, int y2) {
  fprintf(output, "GS\n");
  fprintf(output,"BP\n");
  fprintf(output, "%i %i MT\n", x0 , y0);
  fprintf(output,"%i %i LT\n", x1 , y1);
  fprintf(output, "%i %i LT\n", x2 , y2);
  fprintf(output, "EFP\n");
  fprintf(output, "GR\n");
}

void Fl_PS_Printer::polygon(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) {
  fprintf(output, "GS\n");
  fprintf(output,"BP\n");
  fprintf(output, "%i %i MT\n", x0 , y0 );
  fprintf(output, "%i %i LT\n", x1 , y1 );
  fprintf(output, "%i %i LT\n", x2 , y2 );
  fprintf(output, "%i %i LT\n", x3 , y3 );

  fprintf(output, "EFP\n");
  fprintf(output, "GR\n");
}

void Fl_PS_Printer::xyline(int x, int y, int x1, int y2, int x3){
  fprintf(output, "GS\n");
  fprintf(output,"BP\n");
  fprintf(output, "%i %i MT\n", x , y );
  fprintf(output, "%i %i LT\n", x1 , y );
  fprintf(output, "%i %i LT\n", x1 , y2);
  fprintf(output,"%i %i LT\n", x3 , y2);
  fprintf(output, "ELP\n");
  fprintf(output, "GR\n");
};


void Fl_PS_Printer::xyline(int x, int y, int x1, int y2){

  fprintf(output, "GS\n");
  fprintf(output,"BP\n");
  fprintf(output, "%i %i MT\n", x , y);
  fprintf(output,"%i %i LT\n", x1 , y);
  fprintf(output, "%i %i LT\n", x1 , y2 );
  fprintf(output, "ELP\n");
  fprintf(output, "GR\n");
};

void Fl_PS_Printer::xyline(int x, int y, int x1){
  fprintf(output, "GS\n");
  fprintf(output,"BP\n");
  fprintf(output, "%i %i MT\n", x , y);
  fprintf(output, "%i %i LT\n", x1 , y );
  fprintf(output, "ELP\n");

  fprintf(output, "GR\n");
};

void Fl_PS_Printer::yxline(int x, int y, int y1, int x2, int y3){
  fprintf(output, "GS\n");

  fprintf(output,"BP\n");
  fprintf(output,"%i %i MT\n", x , y);
  fprintf(output, "%i %i LT\n", x , y1 );
  fprintf(output, "%i %i LT\n", x2 , y1 );
  fprintf(output , "%i %i LT\n", x2 , y3);
  fprintf(output, "ELP\n");
  fprintf(output, "GR\n");
};

void Fl_PS_Printer::yxline(int x, int y, int y1, int x2){
  fprintf(output, "GS\n");
  fprintf(output,"BP\n");
  fprintf(output, "%i %i MT\n", x , y);
  fprintf(output, "%i %i LT\n", x , y1);
  fprintf(output, "%i %i LT\n", x2 , y1);
  fprintf(output, "ELP\n");
  fprintf(output, "GR\n");
};

void Fl_PS_Printer::yxline(int x, int y, int y1){
  fprintf(output, "GS\n");
  fprintf(output,"BP\n");
  fprintf(output, "%i %i MT\n", x , y);
  fprintf(output, "%i %i LT\n", x , y1);
  fprintf(output, "ELP\n");

  fprintf(output, "GR\n");
};



void Fl_PS_Printer::arc(int x, int y, int w, int h, double a1, double a2) {


  fprintf(output, "GS\n");
  //fprintf(output, "BP\n");
  begin_line();
  fprintf(output, "%g %g TR\n", x + w/2.0 -0.5 , y + h/2.0 - 0.5);
  fprintf(output, "%g %g SC\n", (w-1)/2.0 , (h-1)/2.0 );
  arc(0,0,1,a2,a1);
//  fprintf(output, "0 0 1 %g %g arc\n" , -a1 , -a2);
  fprintf(output, "%g %g SC\n", 2.0/(w-1) , 2.0/(h-1) );
  fprintf(output, "%g %g TR\n", -x - w/2.0 +0.5 , -y - h/2.0 +0.5);
  end_line();


//  fprintf(output, "%g setlinewidth\n",  2/sqrt(w*h));
  //fprintf(output, "ELP\n");
//  fprintf(output, 2.0/w , 2.0/w , " SC\n";

//  fprintf(output, (-x - w/2.0) , (-y - h/2)  , " TR\n";
  fprintf(output, "GR\n");
}

void Fl_PS_Printer::pie(int x, int y, int w, int h, double a1, double a2) {

  fprintf(output, "GS\n");
  fprintf(output, "%g %g TR\n", x + w/2.0 -0.5 , y + h/2.0 - 0.5);
  fprintf(output, "%g %g SC\n", (w-1)/2.0 , (h-1)/2.0 );
  begin_polygon();
  vertex(0,0);
  arc(0.0,0.0, 1, a2, a1);
  end_polygon();
  fprintf(output, "GR\n");

}

/////////////////  transformed (double) drawings ////////////////////////////////


void Fl_PS_Printer::begin_points(){
  fprintf(output, "GS\n");
  concat();

  fprintf(output, "BP\n");
  gap_=1;
  shape_=POINTS;
};

void Fl_PS_Printer::begin_line(){
  fprintf(output, "GS\n");
  concat();
  fprintf(output, "BP\n");
  gap_=1;
  shape_=LINE;
};

void Fl_PS_Printer::begin_loop(){
  fprintf(output, "GS\n");
  concat();
  fprintf(output, "BP\n");
  gap_=1;
  shape_=LOOP;
};

void Fl_PS_Printer::begin_polygon(){
  fprintf(output, "GS\n");
  concat();
  fprintf(output, "BP\n");
  gap_=1;
  shape_=POLYGON;
};

void Fl_PS_Printer::vertex(double x, double y){
  if(shape_==POINTS){
    fprintf(output,"%g %g MT\n", x , y);
    gap_=1;
    return;
  }
  if(gap_){
    fprintf(output,"%g %g MT\n", x , y);
    gap_=0;
  }else
    fprintf(output, "%g %g LT\n", x , y);
};

void Fl_PS_Printer::curve(double x, double y, double x1, double y1, double x2, double y2, double x3, double y3){
  if(shape_==NONE) return;
    if(gap_)
      fprintf(output,"%g %g MT\n", x , y);
    else
      fprintf(output, "%g %g LT\n", x , y);
    gap_=0;

  fprintf(output, "%g %g %g %g %g %g curveto \n", x1 , y1 , x2 , y2 , x3 , y3);
};


void Fl_PS_Printer::circle(double x, double y, double r){
  if(shape_==NONE){
    fprintf(output, "GS\n");
    concat();
//    fprintf(output, "BP\n");
    fprintf(output,"%g %g %g 0 360 arc\n", x , y , r);
    reconcat();
//    fprintf(output, "ELP\n");
    fprintf(output, "GR\n");
  }else

    fprintf(output, "%g %g %g 0 360 arc\n", x , y , r);

};





void Fl_PS_Printer::arc(double x, double y, double r, double start, double a){
  if(shape_==NONE) return;
  gap_=0;
  if(start>a)
    fprintf(output, "%g %g %g %g %g arc\n", x , y , r , -start, -a);
  else
    fprintf(output, "%g %g %g %g %g arcn\n", x , y , r , -start, -a);

};

void Fl_PS_Printer::end_points(){
  gap_=1;
  reconcat();
  fprintf(output, "ELP\n"); //??
  fprintf(output, "GR\n");
  shape_=NONE;
}

void Fl_PS_Printer::end_line(){
  gap_=1;
  reconcat();
  fprintf(output, "ELP\n");
  fprintf(output, "GR\n");
  shape_=NONE;
}
void Fl_PS_Printer::end_loop(){
  gap_=1;
  reconcat();
  fprintf(output, "ECP\n");
  fprintf(output, "GR\n");
  shape_=NONE;
}

void Fl_PS_Printer::end_polygon(){

  gap_=1;
  reconcat();
  fprintf(output, "EFP\n");
  fprintf(output, "GR\n");
  shape_=NONE;
}

void Fl_PS_Printer::transformed_vertex(double x, double y){
  reconcat();
  if(gap_){
    fprintf(output, "%g %g MT\n", x , y);
    gap_=0;
  }else
    fprintf(output, "%g %g LT\n", x , y);
  concat();
};


///////////////////////// misc ////////////////////////////////////////////

void Fl_PS_Printer::font(int f, int s) {

  //fonted_=1;
  if (f >= FL_FREE_FONT)
    f = FL_COURIER;
  fprintf(output, "/%s SF\n" , _fontNames[f]);
  fprintf(output,"%i FS\n", s);
  fltk.font(f,s); //Dirty hack for font measurement ;-(
  font_=f; size_=s;
};


void Fl_PS_Printer::line_style(int style, int width, char* dashes){
  //line_styled_=1;
 
  linewidth_=width;
  linestyle_=style;
  //dashes_= dashes;
  if(dashes){
    if(dashes != linedash_)
      strcpy(linedash_,dashes);

  }else
    linedash_[0]=0;
  char width0 = 0;
  if(!width){
    width=1; //for screen drawing compatability
    width0=1;
  }

  fprintf(output, "%i setlinewidth\n", width);

  if(!style && (!dashes || !(*dashes)) && width0) //system lines
    style = FL_CAP_SQUARE;

  int cap = (style &0xf00) >> 8;
  if(cap) cap--;
  fprintf(output,"%i setlinecap\n", cap);

  int join = (style & 0xf000) >> 12;

  if(join) join--;
  fprintf(output,"%i setlinejoin\n", join);
  
  
  fprintf(output, "[");
  if(dashes && *dashes){
    while(*dashes){
      fprintf(output, "%i ", *dashes);
      dashes++;
    }
  }else{
    int * ds; 
    if(style & 0x200){ // round and square caps, dash length need to be adjusted
        double *dt = dashes_cap[style & 0xff];
        while (*dt >= 0){
          fprintf(output, "%g ",width * (*dt));
          dt++;
        }
      }else{

        ds = dashes_flat[style & 0xff];
        while (*ds >= 0){
          fprintf(output, "%i ",width * (*ds));
        ds++;
      }
    }
  }
  fprintf(output, "] 0 setdash\n");
};

double Fl_PS_Printer::width(const char* s){
  return fltk.width(s); //Dirty...
}

double Fl_PS_Printer::width(uchar c){
  return fltk.width(c); //Dirty...
}

double Fl_PS_Printer::width(const char* s, int n){;
  return fltk.width(s,n); //Very Dirty...
}
int Fl_PS_Printer::descent(){
  return fltk.descent(); //A bit Dirty...
}
int Fl_PS_Printer::height(){
  return fltk.height(); //Still Dirty...
}

///////////////////////////////  text ////////////////////////////////////

/* a pathetic quick & dirty UTF-8 parser 
 *
 * adapted from Markus Kuhn -- Public Domain --
 * (Get uniset from <http://www.cl.cam.ac.uk/~mgk25/download/uniset.tar.gz>.)
 */

// convert UTF-8 string to wchar_t *, return adjusted length
unsigned int wchar_convert(wchar_t * wline,const char * line,unsigned int n){
  unsigned int i=0,j=0,c;
  for (;i<n;i++){
    c=line[i];
    if ( (c & 0xc0) == 0x80)
      continue;
    if (c < 128){
      wline[j]=c;
      j++;
      continue;
    }
    if ( (c & 0xe0) == 0xc0) {
      i++;
      c = (c & 0x1f) << 6 | (line[i] & 0x3f);
      wline[j]=c;
      j++;
      continue;
    } 
    if ( (c & 0xf0) == 0xe0) {
      i++;
      c = (c & 0x0f) << 6 | (line[i] & 0x3f);
      i++;
      c = c << 6 | (line[i] & 0x3f);
      wline[j]=c;
      j++;
      continue;
    } 
    if ( (c & 0xf1) == 0xf0) {
      i++;
      c = (c & 0x0f) << 6 | (line[i] & 0x3f);
      i++;
      c = c << 6 | (line[i] & 0x3f);
      i++;
      c = c << 6 | (line[i] & 0x3f);
    } else 
      c = 0xfffd;
    wline[j]=c;
    j++;
  }
  wline[j]=0;
  return j;
}


#define GRAVE        " (\301) "
#define ACUTE        " (\302) "
#define CIRCUMFLEX   " (\303) "
#define TILDE        " (\304) "
#define MACRON       " (\305) "
#define BREVE        " (\306) "
#define DOTACCENT    " (\307) "
#define DIERESIS     " (\310) "
#define RING         " (\312) "
#define CEDILLA      " (\313) "
#define HUNGARUMLAUT " (\315) "
#define OGONEK       " (\316) "
#define CARON        " (\317) "
#define DOTLESSI     "(\365)"

struct charentry {
  unsigned short ucs;
  char execute; /* 0: s is a glyphname, 1: s is executable code */ 
  const char *s;
} chartab[] ={
  { 0x0027, 0, "quotesingle" },
  { 0x0060, 0, "grave" },
  { 0x00A0, 0, "space" },
  { 0x00A1, 0, "exclamdown" },
  { 0x00A2, 0, "cent" },
  { 0x00A3, 0, "sterling" },
  { 0x00A4, 0, "currency" },
  { 0x00A5, 0, "yen" },
  { 0x00A6, 0, "brokenbar" },
  { 0x00A7, 0, "section" },
  { 0x00A8, 0, "dieresis" },
  { 0x00A9, 0, "copyright" },
  { 0x00AA, 0, "ordfeminine" },
  { 0x00AB, 0, "guillemotleft" },
  { 0x00AC, 0, "logicalnot" },
  { 0x00AD, 0, "hyphen" },
  { 0x00AE, 0, "registered" },
  { 0x00AF, 0, "macron" },
  { 0x00B0, 0, "degree" },
  { 0x00B1, 0, "plusminus" },
  { 0x00B2, 0, "twosuperior" },
  { 0x00B3, 0, "threesuperior" },
  { 0x00B4, 0, "acute" },
  { 0x00B5, 0, "mu" },
  { 0x00B6, 0, "paragraph" },
  { 0x00B7, 0, "periodcentered" },
  { 0x00B8, 0, "cedilla" },
  { 0x00B9, 0, "onesuperior" },
  { 0x00BA, 0, "ordmasculine" },
  { 0x00BB, 0, "guillemotright" },
  { 0x00BC, 0, "onequarter" },
  { 0x00BD, 0, "onehalf" },
  { 0x00BE, 0, "threequarters" },
  { 0x00BF, 0, "questiondown" },
  { 0x00C0, 0, "Agrave" },
  { 0x00C1, 0, "Aacute" },
  { 0x00C2, 0, "Acircumflex" },
  { 0x00C3, 0, "Atilde" },
  { 0x00C4, 0, "Adieresis" },
  { 0x00C5, 0, "Aring" },
  { 0x00C6, 0, "AE" },
  { 0x00C7, 0, "Ccedilla" },
  { 0x00C8, 0, "Egrave" },
  { 0x00C9, 0, "Eacute" },
  { 0x00CA, 0, "Ecircumflex" },
  { 0x00CB, 0, "Edieresis" },
  { 0x00CC, 0, "Igrave" },
  { 0x00CD, 0, "Iacute" },
  { 0x00CE, 0, "Icircumflex" },
  { 0x00CF, 0, "Idieresis" },
  { 0x00D0, 0, "Eth" },
  { 0x00D1, 0, "Ntilde" },
  { 0x00D2, 0, "Ograve" },
  { 0x00D3, 0, "Oacute" },
  { 0x00D4, 0, "Ocircumflex" },
  { 0x00D5, 0, "Otilde" },
  { 0x00D6, 0, "Odieresis" },
  { 0x00D7, 0, "multiply" },
  { 0x00D8, 0, "Oslash" },
  { 0x00D9, 0, "Ugrave" },
  { 0x00DA, 0, "Uacute" },
  { 0x00DB, 0, "Ucircumflex" },
  { 0x00DC, 0, "Udieresis" },
  { 0x00DD, 0, "Yacute" },
  { 0x00DE, 0, "Thorn" },
  { 0x00DF, 0, "germandbls" },
  { 0x00E0, 0, "agrave" },
  { 0x00E1, 0, "aacute" },
  { 0x00E2, 0, "acircumflex" },
  { 0x00E3, 0, "atilde" },
  { 0x00E4, 0, "adieresis" },
  { 0x00E5, 0, "aring" },
  { 0x00E6, 0, "ae" },
  { 0x00E7, 0, "ccedilla" },
  { 0x00E8, 0, "egrave" },
  { 0x00E9, 0, "eacute" },
  { 0x00EA, 0, "ecircumflex" },
  { 0x00EB, 0, "edieresis" },
  { 0x00EC, 0, "igrave" },
  { 0x00ED, 0, "iacute" },
  { 0x00EE, 0, "icircumflex" },
  { 0x00EF, 0, "idieresis" },
  { 0x00F0, 0, "eth" },
  { 0x00F1, 0, "ntilde" },
  { 0x00F2, 0, "ograve" },
  { 0x00F3, 0, "oacute" },
  { 0x00F4, 0, "ocircumflex" },
  { 0x00F5, 0, "otilde" },
  { 0x00F6, 0, "odieresis" },
  { 0x00F7, 0, "divide" },
  { 0x00F8, 0, "oslash" },
  { 0x00F9, 0, "ugrave" },
  { 0x00FA, 0, "uacute" },
  { 0x00FB, 0, "ucircumflex" },
  { 0x00FC, 0, "udieresis" },
  { 0x00FD, 0, "yacute" },
  { 0x00FE, 0, "thorn" },
  { 0x00FF, 0, "ydieresis" },
  { 0x0100, 1, "(A)" MACRON "Cp"},
  { 0x0101, 1, "(a)" MACRON "cp"},
  { 0x0102, 1, "(A)" BREVE  "Cp"},
  { 0x0103, 1, "(a)" BREVE  "cp"},
  { 0x0104, 1, "(A)" OGONEK "cp" },
  { 0x0105, 1, "(a)" OGONEK "cp" },
  { 0x0106, 1, "(C)" ACUTE "Cp" },
  { 0x0107, 1, "(c)" ACUTE "cp" },
  { 0x0108, 1, "(C)" CIRCUMFLEX "Cp" },
  { 0x0109, 1, "(c)" CIRCUMFLEX "cp" },
  { 0x010A, 1, "(C)" DOTACCENT "Cp" },
  { 0x010B, 1, "(c)" DOTACCENT "cp" },
  { 0x010C, 1, "(C)" CARON "Cp" },
  { 0x010D, 1, "(c)" CARON "cp" },
  { 0x010E, 1, "(D)" CARON "Cp" },
  { 0x010F, 1, "(d)" CARON "Cp" },
  { 0x0110, 0, "Eth" },
  { 0x0111, 1, "(d) (-) 0.1 0.2 scp" },
  { 0x0112, 1, "(E)" MACRON "Cp" },
  { 0x0113, 1, "(e)" MACRON "cp" },
  { 0x0114, 1, "(E)" BREVE "Cp" },
  { 0x0115, 1, "(e)" BREVE "cp" },
  { 0x0116, 1, "(E)" DOTACCENT "Cp" },
  { 0x0117, 1, "(e)" DOTACCENT "cp" },
  { 0x0118, 1, "(E)" OGONEK "cp" },
  { 0x0119, 1, "(e)" OGONEK "cp" },
  { 0x011A, 1, "(E)" CARON "Cp" },
  { 0x011B, 1, "(e)" CARON "cp" },
  { 0x011C, 1, "(G)" CIRCUMFLEX "Cp" },
  { 0x011D, 1, "(g)" CIRCUMFLEX "cp" },
  { 0x011E, 1, "(G)" BREVE "Cp" },
  { 0x011F, 1, "(g)" BREVE "cp" },
  { 0x0120, 1, "(G)" DOTACCENT "Cp" },
  { 0x0121, 1, "(g)" DOTACCENT "cp" },
  { 0x0122, 1, "(G)" CEDILLA "cp" },
  { 0x0124, 1, "(H)" CIRCUMFLEX "Cp" },
  { 0x0125, 1, "(h)" CIRCUMFLEX "Cp" },
  { 0x0126, 1, "(H) (-) 0 0.15 scp" },
  { 0x0127, 1, "(h) (-) -0.1 0.2 scp" },
  { 0x0128, 1, "(I)" TILDE "Cp" },
  { 0x0129, 1, DOTLESSI TILDE "cp" },
  { 0x012A, 1, "(I)" MACRON "Cp" },
  { 0x012B, 1, DOTLESSI MACRON "cp" },
  { 0x012C, 1, "(I)" BREVE "Cp" },
  { 0x012D, 1, DOTLESSI BREVE "cp" },
  { 0x012E, 1, "(I)" OGONEK "cp" },
  { 0x012F, 1, "(i)" OGONEK "cp" },
  { 0x0130, 1, "(I)" DOTACCENT "Cp" },
  { 0x0131, 0, "dotlessi" },
  { 0x0134, 1, "(J)" CIRCUMFLEX "Cp" },
  { 0x0135, 1, "(j)" CIRCUMFLEX "cp" },
  { 0x0136, 1, "(K)" CEDILLA "cp" },
  { 0x0137, 1, "(k)" CEDILLA "cp" },
  { 0x0138, 1, "gsave currentpoint translate 1 0.7 scale (K) show grestore "
    "(K) stringwidth rmoveto" },
  { 0x0139, 1, "(L)" ACUTE "Cp" },
  { 0x013A, 1, "(l)" ACUTE "Cp" },
  { 0x013B, 1, "(L)" CEDILLA "cp" },
  { 0x013C, 1, "(l)" CEDILLA "cp" },
  { 0x013D, 1, "(L)" CARON "Cp" },
  { 0x013E, 1, "(l)" CARON "Cp" },
  { 0x0141, 0, "Lslash" },
  { 0x0142, 0, "lslash" },
  { 0x0143, 1, "(N)" ACUTE "Cp" },
  { 0x0144, 1, "(n)" ACUTE "cp" },
  { 0x0145, 1, "(N)" CEDILLA "cp" },
  { 0x0146, 1, "(n)" CEDILLA "cp" },
  { 0x0147, 1, "(N)" CARON "Cp" },
  { 0x0148, 1, "(n)" CARON "cp" },
  { 0x014C, 1, "(O)" MACRON "Cp" },
  { 0x014D, 1, "(o)" MACRON "cp" },
  { 0x014E, 1, "(O)" BREVE "Cp" },
  { 0x014F, 1, "(o)" BREVE "cp" },
  { 0x0150, 1, "(O)" HUNGARUMLAUT "Cp" },
  { 0x0151, 1, "(o)" HUNGARUMLAUT "cp" },
  { 0x0152, 0, "OE" },
  { 0x0153, 0, "oe" },
  { 0x0154, 1, "(R)" ACUTE "Cp" },
  { 0x0155, 1, "(r)" ACUTE "cp" },
  { 0x0156, 1, "(R)" CEDILLA "cp" },
  { 0x0157, 1, "(r)" CEDILLA "cp" },
  { 0x0158, 1, "(R)" CARON "Cp" },
  { 0x0159, 1, "(r)" CARON "cp" },
  { 0x015A, 1, "(S)" ACUTE "Cp" },
  { 0x015B, 1, "(s)" ACUTE "cp" },
  { 0x015C, 1, "(S)" CIRCUMFLEX "Cp" },
  { 0x015D, 1, "(s)" CIRCUMFLEX "cp" },
  { 0x015E, 1, "(S)" CEDILLA "cp" },
  { 0x015F, 1, "(s)" CEDILLA "cp" },
  { 0x0160, 0, "Scaron" },
  { 0x0161, 0, "scaron" },
  { 0x0162, 1, "(T)" CEDILLA "cp" },
  { 0x0163, 1, "(t)" CEDILLA "cp" },
  { 0x0164, 1, "(T)" CARON "Cp" },
  { 0x0165, 1, "(t)" CARON "Cp" },
  { 0x0166, 1, "(T) (-) cp" },
  { 0x0167, 1, "(t) (-) -0.1 0 scp" },
  { 0x0168, 1, "(U)" TILDE "Cp" },
  { 0x0169, 1, "(u)" TILDE "cp" },
  { 0x016A, 1, "(U)" MACRON "Cp" },
  { 0x016B, 1, "(u)" MACRON "cp" },
  { 0x016C, 1, "(U)" BREVE "Cp" },
  { 0x016D, 1, "(u)" BREVE "cp" },
  { 0x016E, 1, "(U)" RING "Cp" },
  { 0x016F, 1, "(u)" RING "cp" },
  { 0x0170, 1, "(U)" HUNGARUMLAUT "Cp" },
  { 0x0171, 1, "(u)" HUNGARUMLAUT "cp" },
  { 0x0172, 1, "(U)" OGONEK "cp" },
  { 0x0173, 1, "(u)" OGONEK "cp" },
  { 0x0174, 1, "(W)" CIRCUMFLEX "Cp" },
  { 0x0175, 1, "(w)" CIRCUMFLEX "cp" },
  { 0x0176, 1, "(Y)" CIRCUMFLEX "Cp" },
  { 0x0177, 1, "(y)" CIRCUMFLEX "cp" },
  { 0x0178, 0, "Ydieresis" },
  { 0x0179, 1, "(Z)" ACUTE "Cp" },
  { 0x017A, 1, "(z)" ACUTE "cp" },
  { 0x017B, 1, "(Z)" DOTACCENT "Cp" },
  { 0x017C, 1, "(z)" DOTACCENT "cp" },
  { 0x017D, 0, "Zcaron" },
  { 0x017E, 0, "zcaron" },
  { 0x0189, 0, "Eth" },
  { 0x0192, 0, "florin" },
  { 0x02C6, 0, "circumflex" },
  { 0x02C7, 0, "caron" },
  { 0x02C9, 0, "macron" },
  { 0x02D8, 0, "breve" },
  { 0x02D9, 0, "dotaccent" },
  { 0x02DA, 0, "ring" },
  { 0x02DB, 0, "ogonek" },
  { 0x02DC, 0, "tilde" },
  { 0x02DD, 0, "hungarumlaut" },
  { 0x0386, 3, "A"}, // FIXME
  { 0x0388, 3, "E"}, // FIXME
  { 0x0389, 3, "H"}, // FIXME
  { 0x038A, 3, "I"}, // FIXME
  { 0x038C, 3, "O"}, // FIXME
  { 0x038E, 3, "Y"}, // FIXME
  { 0x038F, 3, "W"}, // FIXME
  { 0x0390, 3, "i"}, // FIXME
  { 0x0391, 3, "A"},
  { 0x0392, 3, "B"},
  { 0x0393, 3, "G"},
  { 0x0394, 3, "D"},
  { 0x0395, 3, "E"},
  { 0x0396, 3, "Z"},
  { 0x0397, 3, "H"},
  { 0x0398, 3, "Q"},
  { 0x0399, 3, "I"},
  { 0x039A, 3, "K"},
  { 0x039B, 3, "L"},
  { 0x039C, 3, "M"},
  { 0x039D, 3, "N"},
  { 0x039E, 3, "X"},
  { 0x039F, 3, "O"},
  { 0x03A0, 3, "P"},
  { 0x03A1, 3, "R"},
  { 0x03A3, 3, "S"},
  { 0x03A4, 3, "T"},
  { 0x03A5, 3, "U"},
  { 0x03A6, 3, "F"},
  { 0x03A7, 3, "C"},
  { 0x03A8, 3, "Y"},
  { 0x03A9, 3, "W"},
  { 0x03AA, 3, "I"}, // FIXME
  { 0x03AB, 3, "Y"}, // FIXME
  { 0x03AC, 3, "a"}, // FIXME
  { 0x03AD, 3, "e"}, // FIXME
  { 0x03AE, 3, "h"}, // FIXME
  { 0x03AF, 3, "i"}, // FIXME
  { 0x03B0, 3, "u"}, // FIXME
  { 0x03B1, 3, "a"},
  { 0x03B2, 3, "b"},
  { 0x03B3, 3, "g"},
  { 0x03B4, 3, "d"},
  { 0x03B5, 3, "e"},
  { 0x03B6, 3, "z"},
  { 0x03B7, 3, "h"},
  { 0x03B8, 3, "q"},
  { 0x03B9, 3, "i"},
  { 0x03BA, 3, "k"},
  { 0x03BB, 3, "l"},
  { 0x03BC, 3, "m" },
  { 0x03BD, 3, "n"},
  { 0x03BE, 3, "x"},
  { 0x03BF, 3, "o"},
  { 0x03C0, 3, "p"},
  { 0x03C1, 3, "r"},
  { 0x03C2, 3, "V"},
  { 0x03C3, 3, "s"},
  { 0x03C4, 3, "t"},
  { 0x03C5, 3, "u"},
  { 0x03C6, 3, "f"},
  { 0x03C7, 3, "c"},
  { 0x03C8, 3, "y"},
  { 0x03C9, 3, "w"},
  { 0x03CA, 3, "i"},
  { 0x03CB, 3, "u"},
  { 0x03CC, 3, "o"},
  { 0x03CD, 3, "u"},
  { 0x03CE, 3, "w"},
  { 0x03D1, 3, "J"},
  { 0x03D2, 3, "j"},
  { 0x03D6, 3, "v"},
  { 0x1F00, 3, "a"}, // FIXME accents
  { 0x1F01, 3, "a"},
  { 0x1F02, 3, "a"},
  { 0x1F03, 3, "a"},
  { 0x1F04, 3, "a"},
  { 0x1F05, 3, "a"},
  { 0x1F06, 3, "a"},
  { 0x1F07, 3, "a"},
  { 0x1F08, 3, "A"},
  { 0x1F09, 3, "A"},
  { 0x1F0A, 3, "A"},
  { 0x1F0B, 3, "A"},
  { 0x1F0C, 3, "A"},
  { 0x1F0D, 3, "A"},
  { 0x1F0E, 3, "A"},
  { 0x1F0F, 3, "A"},
  { 0x1F10, 3, "e"}, // FIXME eccents
  { 0x1F11, 3, "e"},
  { 0x1F12, 3, "e"},
  { 0x1F13, 3, "e"},
  { 0x1F14, 3, "e"},
  { 0x1F15, 3, "e"},
  { 0x1F16, 3, "e"},
  { 0x1F17, 3, "e"},
  { 0x1F18, 3, "E"},
  { 0x1F19, 3, "E"},
  { 0x1F1A, 3, "E"},
  { 0x1F1B, 3, "E"},
  { 0x1F1C, 3, "E"},
  { 0x1F1D, 3, "E"},
  { 0x1F1E, 3, "E"},
  { 0x1F1F, 3, "E"},
  { 0x1F20, 3, "h"}, // FIXME hccents
  { 0x1F21, 3, "h"},
  { 0x1F22, 3, "h"},
  { 0x1F23, 3, "h"},
  { 0x1F24, 3, "h"},
  { 0x1F25, 3, "h"},
  { 0x1F26, 3, "h"},
  { 0x1F27, 3, "h"},
  { 0x1F28, 3, "H"},
  { 0x1F29, 3, "H"},
  { 0x1F2A, 3, "H"},
  { 0x1F2B, 3, "H"},
  { 0x1F2C, 3, "H"},
  { 0x1F2D, 3, "H"},
  { 0x1F2E, 3, "H"},
  { 0x1F2F, 3, "H"},
  { 0x1F30, 3, "i"}, // FIXME iccents
  { 0x1F31, 3, "i"},
  { 0x1F32, 3, "i"},
  { 0x1F33, 3, "i"},
  { 0x1F34, 3, "i"},
  { 0x1F35, 3, "i"},
  { 0x1F36, 3, "i"},
  { 0x1F37, 3, "i"},
  { 0x1F38, 3, "I"},
  { 0x1F39, 3, "I"},
  { 0x1F3A, 3, "I"},
  { 0x1F3B, 3, "I"},
  { 0x1F3C, 3, "I"},
  { 0x1F3D, 3, "I"},
  { 0x1F3E, 3, "I"},
  { 0x1F3F, 3, "I"},
  { 0x1F40, 3, "o"}, // FIXME occents
  { 0x1F41, 3, "o"},
  { 0x1F42, 3, "o"},
  { 0x1F43, 3, "o"},
  { 0x1F44, 3, "o"},
  { 0x1F45, 3, "o"},
  { 0x1F46, 3, "o"},
  { 0x1F47, 3, "o"},
  { 0x1F48, 3, "O"},
  { 0x1F49, 3, "O"},
  { 0x1F4A, 3, "O"},
  { 0x1F4B, 3, "O"},
  { 0x1F4C, 3, "O"},
  { 0x1F4D, 3, "O"},
  { 0x1F4E, 3, "O"},
  { 0x1F4F, 3, "O"},
  { 0x1F50, 3, "u"}, // FIXME accents
  { 0x1F51, 3, "u"},
  { 0x1F52, 3, "u"},
  { 0x1F53, 3, "u"},
  { 0x1F54, 3, "u"},
  { 0x1F55, 3, "u"},
  { 0x1F56, 3, "u"},
  { 0x1F57, 3, "u"},
  { 0x1F58, 3, "U"},
  { 0x1F59, 3, "U"},
  { 0x1F5A, 3, "U"},
  { 0x1F5B, 3, "U"},
  { 0x1F5C, 3, "U"},
  { 0x1F5D, 3, "U"},
  { 0x1F5E, 3, "U"},
  { 0x1F5F, 3, "U"},
  { 0x1F60, 3, "w"}, // FIXME accents
  { 0x1F61, 3, "w"},
  { 0x1F62, 3, "w"},
  { 0x1F63, 3, "w"},
  { 0x1F64, 3, "w"},
  { 0x1F65, 3, "w"},
  { 0x1F66, 3, "w"},
  { 0x1F67, 3, "w"},
  { 0x1F68, 3, "W"},
  { 0x1F69, 3, "W"},
  { 0x1F6A, 3, "W"},
  { 0x1F6B, 3, "W"},
  { 0x1F6C, 3, "W"},
  { 0x1F6D, 3, "W"},
  { 0x1F6E, 3, "W"},
  { 0x1F6F, 3, "W"},
  { 0x1F70, 3, "a"}, // FIXME accents
  { 0x1F71, 3, "a"},
  { 0x1F72, 3, "e"},
  { 0x1F73, 3, "e"},
  { 0x1F74, 3, "h"},
  { 0x1F75, 3, "h"},
  { 0x1F76, 3, "i"},
  { 0x1F77, 3, "i"},
  { 0x1F78, 3, "o"},
  { 0x1F79, 3, "o"},
  { 0x1F7A, 3, "u"},
  { 0x1F7B, 3, "u"},
  { 0x1F7C, 3, "w"},
  { 0x1F7D, 3, "w"},
  { 0x1F7E, 3, "A"},
  { 0x1F7F, 3, "A"},
  { 0x1F80, 3, "a"}, // FIXME accents
  { 0x1F81, 3, "a"},
  { 0x1F82, 3, "a"},
  { 0x1F83, 3, "a"},
  { 0x1F84, 3, "a"},
  { 0x1F85, 3, "a"},
  { 0x1F86, 3, "a"},
  { 0x1F87, 3, "a"},
  { 0x1F88, 3, "A"},
  { 0x1F89, 3, "A"},
  { 0x1F8A, 3, "A"},
  { 0x1F8B, 3, "A"},
  { 0x1F8C, 3, "A"},
  { 0x1F8D, 3, "A"},
  { 0x1F8E, 3, "A"},
  { 0x1F8F, 3, "A"},
  { 0x1F90, 3, "h"}, // FIXME hccents
  { 0x1F91, 3, "h"},
  { 0x1F92, 3, "h"},
  { 0x1F93, 3, "h"},
  { 0x1F94, 3, "h"},
  { 0x1F95, 3, "h"},
  { 0x1F96, 3, "h"},
  { 0x1F97, 3, "h"},
  { 0x1F98, 3, "H"},
  { 0x1F99, 3, "H"},
  { 0x1F9A, 3, "H"},
  { 0x1F9B, 3, "H"},
  { 0x1F9C, 3, "H"},
  { 0x1F9D, 3, "H"},
  { 0x1F9E, 3, "H"},
  { 0x1F9F, 3, "H"},
  { 0x1FA0, 3, "w"}, // FIXME accents
  { 0x1FA1, 3, "w"},
  { 0x1FA2, 3, "w"},
  { 0x1FA3, 3, "w"},
  { 0x1FA4, 3, "w"},
  { 0x1FA5, 3, "w"},
  { 0x1FA6, 3, "w"},
  { 0x1FA7, 3, "w"},
  { 0x1FA8, 3, "W"},
  { 0x1FA9, 3, "W"},
  { 0x1FAA, 3, "W"},
  { 0x1FAB, 3, "W"},
  { 0x1FAC, 3, "W"},
  { 0x1FAD, 3, "W"},
  { 0x1FAE, 3, "W"},
  { 0x1FAF, 3, "W"},
  { 0x1FB0, 3, "a"}, // FIXME accents
  { 0x1FB1, 3, "a"},
  { 0x1FB2, 3, "a"},
  { 0x1FB3, 3, "a"},
  { 0x1FB4, 3, "a"},
  { 0x1FB5, 3, "a"},
  { 0x1FB6, 3, "a"},
  { 0x1FB7, 3, "a"},
  { 0x1FB8, 3, "A"},
  { 0x1FB9, 3, "A"},
  { 0x1FBA, 3, "A"},
  { 0x1FBB, 3, "A"},
  { 0x1FBC, 3, "A"},
  { 0x1FBD, 3, "?"},
  { 0x1FBE, 3, "?"},
  { 0x1FBF, 3, "?"},
  { 0x1FC0, 3, "~"}, // FIXME accents
  { 0x1FC1, 3, "~"},
  { 0x1FC2, 3, "h"},
  { 0x1FC3, 3, "h"},
  { 0x1FC4, 3, "h"},
  { 0x1FC5, 3, "h"},
  { 0x1FC6, 3, "h"},
  { 0x1FC7, 3, "h"},
  { 0x1FC8, 3, "E"},
  { 0x1FC9, 3, "E"},
  { 0x1FCA, 3, "H"},
  { 0x1FCB, 3, "H"},
  { 0x1FCC, 3, "H"},
  { 0x1FCD, 3, "?"},
  { 0x1FCE, 3, "?"},
  { 0x1FCF, 3, "?"},
  { 0x1FD0, 3, "i"}, // FIXME accents
  { 0x1FD1, 3, "i"},
  { 0x1FD2, 3, "i"},
  { 0x1FD3, 3, "i"},
  { 0x1FD4, 3, "i"},
  { 0x1FD5, 3, "i"},
  { 0x1FD6, 3, "i"},
  { 0x1FD7, 3, "i"},
  { 0x1FD8, 3, "I"},
  { 0x1FD9, 3, "I"},
  { 0x1FDA, 3, "I"},
  { 0x1FDB, 3, "I"},
  { 0x1FDC, 3, "?"},
  { 0x1FDD, 3, "?"},
  { 0x1FDE, 3, "?"},
  { 0x1FDF, 3, "?"},
  { 0x1FE0, 3, "u"}, // FIXME accents
  { 0x1FE1, 3, "u"},
  { 0x1FE2, 3, "u"},
  { 0x1FE3, 3, "u"},
  { 0x1FE4, 3, "r"},
  { 0x1FE5, 3, "r"},
  { 0x1FE6, 3, "u"},
  { 0x1FE7, 3, "u"},
  { 0x1FE8, 3, "Y"},
  { 0x1FE9, 3, "Y"},
  { 0x1FEA, 3, "Y"},
  { 0x1FEB, 3, "Y"},
  { 0x1FEC, 3, "P"},
  { 0x1FED, 3, "?"},
  { 0x1FEE, 3, "?"},
  { 0x1FEF, 3, "?"},
  { 0x1FF0, 3, "w"}, // FIXME accents
  { 0x1FF1, 3, "w"},
  { 0x1FF2, 3, "w"},
  { 0x1FF3, 3, "w"},
  { 0x1FF4, 3, "w"},
  { 0x1FF5, 3, "w"},
  { 0x1FF6, 3, "w"},
  { 0x1FF7, 3, "w"},
  { 0x1FF8, 3, "O"},
  { 0x1FF9, 3, "O"},
  { 0x1FFA, 3, "W"},
  { 0x1FFB, 3, "W"},
  { 0x1FFC, 3, "W"},
  { 0x1FFD, 3, "?"},
  { 0x1FFE, 3, "?"},
  { 0x1FFF, 3, "?"},
  { 0x2000, 0, "space" },
  { 0x2001, 0, "space" },
  { 0x2002, 0, "space" },
  { 0x2003, 0, "space" },
  { 0x2004, 0, "space" },
  { 0x2005, 0, "space" },
  { 0x2006, 0, "space" },
  { 0x2007, 1, "(0) stringwidth rmoveto" },
  { 0x2008, 1, "(.) stringwidth rmoveto" },
  { 0x2009, 0, "space" },
  { 0x200A, 0, "space" },
  { 0x200B, 1, "" },
  { 0x2010, 0, "hyphen" },
  { 0x2011, 0, "hyphen" },
  { 0x2012, 0, "endash" },
  { 0x2013, 0, "endash" },
  { 0x2014, 0, "emdash" },
  { 0x2015, 0, "emdash" },
  { 0x2018, 0, "quoteleft" },
  { 0x2019, 0, "quoteright" },
  { 0x201A, 0, "quotesinglbase" },
  { 0x201B, 1, "(\047) rev" },
  { 0x201C, 0, "quotedblleft" },
  { 0x201D, 0, "quotedblright" },
  { 0x201E, 0, "quotedblbase" },
  { 0x201F, 1, "(\272) rev" },
  { 0x2020, 0, "dagger" },
  { 0x2021, 0, "daggerdbl" },
  { 0x2022, 0, "bullet" },
  { 0x2026, 0, "ellipsis" },
  { 0x2030, 0, "perthousand" },
  { 0x2039, 0, "guilsinglleft" },
  { 0x203A, 0, "guilsinglright" },
  { 0x203D, 1, "(!) (?) cp" },
  { 0x2044, 0, "fraction" },
  { 0x20AC, 1, "(C) (=) -0.1 0 scp" },
  { 0x2122, 0, "trademark" },
  { 0x2212, 0, "minus" },
  { 0x2215, 0, "slash" },
  { 0x2219, 0, "periodcentered" },
  { 0x2259, 1, "(=)" CIRCUMFLEX "cp" },
  { 0x2260, 1, "(=) (/) cp" },
  { 0x2264, 1, "(<) (-) 0 -0.3 scp" },
  { 0x2265, 1, "(>) (-) 0 -0.3 scp" },
  { 0x226E, 1, "(<) (/) cp" },
  { 0x226F, 1, "(>) (/) cp" },
  { 0x2500, 1, "{0.05 setlinewidth 0 1 moveto 1 1 lineto stroke} bgr" },
  { 0x2501, 1, "{0.2 setlinewidth 0 1 moveto 1 1 lineto stroke} bgr" },
  { 0x2502, 1, "{0.05 setlinewidth 0.5 0 moveto 0.5 2 lineto stroke} bgr" },
  { 0x2503, 1, "{0.2 setlinewidth 0.5 0 moveto 0.5 2 lineto stroke} bgr" },
  { 0x250C, 1, "{0.05 setlinewidth 0.5 0 moveto 0.5 1 lineto 1 1 lineto stroke} bgr" },
  { 0x2510, 1, "{0.05 setlinewidth 0.5 0 moveto 0.5 1 lineto 0 1 lineto stroke} bgr" },
  { 0x2514, 1, "{0.05 setlinewidth 0.5 2 moveto 0.5 1 lineto 1 1 lineto stroke} bgr" },
  { 0x2518, 1, "{0.05 setlinewidth 0.5 2 moveto 0.5 1 lineto 0 1 lineto stroke} bgr" },
  { 0x253C, 1, "{0.05 setlinewidth 0.5 0 moveto 0.5 2 lineto stroke "
    "0 1 moveto 1 1 lineto stroke} bgr" },
  { 0x254B, 1, "{0.2 setlinewidth 0.5 0 moveto 0.5 2 lineto stroke "
    "0 1 moveto 1 1 lineto stroke} bgr" },
  { 0xFB01, 0, "fi" },
  { 0xFB02, 0, "fl" }
};


int get_charentry(wchar_t c)
{
  int min = 0;
  int max = sizeof(chartab) / sizeof(struct charentry) - 1;
  int mid;

  /* binary search in table */
  while (max >= min) {
    mid = (min + max) / 2;
    if (chartab[mid].ucs < c)
      min = mid + 1;
    else if (chartab[mid].ucs > c)
      max = mid - 1;
    else {
      /* found it */
      return mid;
    }
  }

  return -1;
}

void show_line(wchar_t *line,FILE * output,int font_,int size_)
{
  int i, g;

  fprintf(output,"(");
  for (i = 0; line[i]; i++) {
    if (line[i] == '(' || line[i] == ')' || line[i] == '\\'){
      fprintf(output,"\\%c", (int) line[i]);
      continue;
    }
    if (line[i] > 128 || line[i] == '\'' || line[i] == '`') {
      g = get_charentry(line[i]);
      if (g < 0){
	fprintf(output,"?");
	continue;
      }
      if (chartab[g].execute==1){
	fprintf(output,") show %s (", chartab[g].s);
	continue;
      }
      if (chartab[g].execute==0){
	fprintf(output,") show /%s glyphshow (", chartab[g].s);
	continue;
      }
      if (chartab[g].execute==3){
	fprintf(output,") show /Symbol SF %d FS (%s) show /%s SF %d FS (",size_,chartab[g].s,_fontNames[font_],size_);
	continue;
      }
      if (chartab[g].execute==2){
	fprintf(output,") show /Symbol SF %d FS /%s glyphshow /%s SF %d FS (",size_,chartab[g].s,_fontNames[font_],size_);
	continue;
      }
    } 
    else
      fprintf(output,"%c", (int) line[i]);
  } // end for
  fprintf(output,") show\n");
}

void Fl_PS_Printer::transformed_draw(const char* STR, int n, double x, double y){

  if (!n||!STR||!*STR)return;
  wchar_t * str=new wchar_t[n+1];
  n=wchar_convert(str,STR,n);
  fprintf(output, "GS\n");
  fprintf(output,"%g %g moveto\n", x , y);
  fprintf(output, "[1 0 0 -1 0 0] concat\n");
  int i=1;
  show_line(str,output,font_,size_);
  fprintf(output, "GR\n");
  delete [] str;
}



void Fl_PS_Printer::transformed_draw(const char* s, double x, double y){
  transformed_draw(s,strlen(s),x,y);
};

////////////////////////////      Images      /////////////////////////////////////




int Fl_PS_Printer::alpha_mask(const uchar * data, int w, int h, int D, int LD){

  mask = 0;
  if((D/2)*2 != D){ //no mask info
    return 0;
  }
  int xx;
  int i,j, k, l;
  LD += w*D;
  int V255=0;
  int V0 =0;
  int V_=0;
//  uchar d;
  for(j=0;j<h;j++){
    for(i=0;i<w;i++)
      switch(data[j*LD+D*i+D-1]){
        case 255: V255 = 1; break;
        case 0: V0 = 1; break;
        default: V_= 1;
      }
    if(V_) break;
  };
  if(!V_){
    if(V0)
      if(V255){// not true alpha, only masking
        xx = (w+7)/8;
        mask = new uchar[h * xx];
        for(i=0;i<h * xx;i++) mask[i]=0;
        for(j=0;j<h;j++)
          for(i=0;i<w;i++)
            if(data[j*LD+D*i+D-1])
              mask[j*xx+i/8] |= 1 << (i % 8);
        mx = w;
        my = h; //mask imensions
        return 0;
      }else{
        mask=0;
        return 1; //everything masked
      }
    else
      return 0;
  }



  /////   Alpha dither, generating (4*w) * 4 mask area       /////
  /////         with Floyd-Steinberg error diffusion         /////

  mask = new uchar[((w+1)/2) * h * 4];

  for(i=0;i<((w+1)/2) * h * 4; i++) mask[i] = 0; //cleaning



  mx= w*4;
  my=h*4; // mask dimensions

  xx = (w+1)/2;                //  mask line width in bytes

  short * errors1 = new short [w*4+2]; //  two rows of dither errors
  short * errors2 = new short [w*4+2]; //  two rows of dither errors

  for(i=0;i<w*4+2;i++) errors2[i] = 0; // cleaning,after first swap will become current
  for(i=0;i<w*4+2;i++) errors1[i] = 0; // cleaning,after first swap will become current

  short * current = errors1;
  short * next = errors2;
  short * swap;

  for(j=0;j<h;j++){
    for(l=0;l<4;){           // generating 4 rows of mask lines for 1 RGB line
      int jj = j*4+l;

      /// mask row index
      swap = next;
      next = current;
      current = swap;
      *(next+1) = 0;          // must clean the first cell, next are overriden by *1
      for(i=0;i<w;i++){
        for(k=0;k<4;k++){   // generating 4 x-pixels for 1 RGB
          short error, o1, o2, o3;
          int ii = i*4+k;   // mask cell index
          short val = data[j*LD+D*i+D-1] + current[1+ii];
          if (val>127){
            mask[jj*xx+ii/8]  |= 1 << (ii % 8); //set mask bit
            error =  val-255;
          }else
            error = val;

          ////// error spreading /////
          if(error >0){
            next[ii] +=  o1 = (error * 3 + 8)/16;
            current[ii+2] += o2 = (error * 7 + 8)/16;
            next[ii+2] = o3 =(error + 8)/16;  // *1 - ok replacing (cleaning)
          }else{
            next[ii] += o1 = (error * 3 - 8)/16;
            current[ii+2] += o2 = (error * 7 - 8)/16;
            next[ii+2] = o3 = (error - 8)/16;
          }
          next[1+ii] += error - o1 - o2 - o3;
        }
      }
      l++;

      ////// backward

      jj = j*4+l;
      swap = next;
      next = current;
      current = swap;
      *(next+1) = 0;          // must clean the first cell, next are overriden by *1

      for(i=w-1;i>=0;i--){

        for(k=3;k>=0;k--){   // generating 4 x-pixels for 1 RGB
          short error, o1, o2, o3;

          int ii = i*4+k;   // mask cell index
          short val = data[j*LD+D*i+D-1] + current[1+ii];
          if (val>127){

            mask[jj*xx+ii/8]  |= 1 << (ii % 8); //set mask bit
            error =  val-255;
          }else
            error = val;

          ////// error spreading /////
          if(error >0){
            next[ii+2] +=  o1 = (error * 3 + 8)/16;
            current[ii] += o2 = (error * 7 + 8)/16;
            next[ii] = o3 =(error + 8)/16;  // *1 - ok replacing (cleaning)
          }else{
            next[ii+2] += o1 = (error * 3 - 8)/16;

            current[ii] += o2 = (error * 7 - 8)/16;
            next[ii] = o3 = (error - 8)/16;
          }
          next[1+ii] += error - o1 - o2 - o3;
        }
      }
      l++;
    }
  }
  delete[] errors1;
  delete[] errors2;
  return 0;
}




static inline uchar swap_byte(const uchar i){
  uchar b =0;
  if(i & 1) b |= 128;
  if(i & 2) b |= 64;
  if(i & 4) b |= 32;
  if(i & 8) b |= 16;
  if(i & 16) b |= 8;
  if(i & 32) b |= 4;
  if(i & 64) b |= 2;
  if(i & 128) b |= 1;
  return b;
}


extern uchar **fl_mask_bitmap;


void Fl_PS_Printer::draw_scalled_image(const uchar *data, double x, double y, double w, double h, int iw, int ih, int D, int LD) {


  if(D<3){ //mono
    draw_scalled_image_mono(data, x, y, w, h, iw, ih, D, LD);
    return;
  }


  int i,j, k;

  fprintf(output,"save\n");

  char * interpol;
  if(lang_level_>1){
    if(interpolate_)
      interpol="true";
    else
      interpol="false";
    if(mask && lang_level_>2)
      fprintf(output, "%g %g %g %g %i %i %i %i %s CIM\n", x , y+h , w , -h , iw , ih, mx, my, interpol);
    else
      fprintf(output, "%g %g %g %g %i %i %s CII\n", x , y+h , w , -h , iw , ih, interpol);
  }else
    fprintf(output , "%g %g %g %g %i %i CI", x , y+h , w , -h , iw , ih);


  if(!LD) LD = iw*D;
  uchar *curmask=mask;
  uchar bg_r, bg_g, bg_b;

  Fl::get_color(bg_, bg_r,bg_g,bg_b);
  for (j=0; j<ih;j++){
    if(mask){

      for(k=0;k<my/ih;k++){
        for (i=0; i<((mx+7)/8);i++){
          if (!(i%80)) fprintf(output, "\n");
          fprintf(output, "%.2x",swap_byte(*curmask));
          curmask++;
        }
        fprintf(output,"\n");
      }
    }
    const uchar *curdata=data+j*LD;
    for(i=0 ; i<iw ; i++) {
      uchar r = curdata[0];
      uchar g =  curdata[1];
      uchar b =  curdata[2];
      if(lang_level_<3 && D>3) { //can do  mixing using bg_* colors)
        unsigned int a2 = curdata[3]; //must be int
        unsigned int a = 255-a2;
        r = (a2 * r + bg_r * a)/255;
        g = (a2 * g + bg_g * a)/255;
        b = (a2 * b + bg_b * a)/255;
      }
      if (!(i%40)) fprintf(output, "\n");
      fprintf(output, "%.2x%.2x%.2x", r, g, b);
      curdata +=D;
    }
    fprintf(output,"\n");

  }

  fprintf(output," >\nrestore\n" );


};

void Fl_PS_Printer::draw_scalled_image(Fl_Draw_Image_Cb call, void *data, double x, double y, double w, double h, int iw, int ih, int D) {


  fprintf(output,"save\n");
  int i,j,k;
  char * interpol;
  if(lang_level_>1){
    if(interpolate_) interpol="true";
    else interpol="false";
    if(mask && lang_level_>2)
      fprintf(output, "%g %g %g %g %i %i %i %i %s CIM\n", x , y+h , w , -h , iw , ih, mx, my, interpol);
    else
      fprintf(output, "%g %g %g %g %i %i %s CII\n", x , y+h , w , -h , iw , ih, interpol);
  }else
    fprintf(output , "%g %g %g %g %i %i CI", x , y+h , w , -h , iw , ih);

  int LD=iw*D;
  uchar *rgbdata=new uchar[LD];
  uchar *curmask=mask;

  for (j=0; j<ih;j++){
    if(mask && lang_level_>2){  // InterleaveType 2 mask data
      for(k=0; k<my/ih;k++){ //for alpha pseudo-masking
        for (i=0; i<((mx+7)/8);i++){
          if (!(i%40)) fprintf(output, "\n");
          fprintf(output, "%.2x",swap_byte(*curmask));
          curmask++;
        }
        fprintf(output,"\n");
      }
    }
    call(data,0,j,iw,rgbdata);
    uchar *curdata=rgbdata;
    for(i=0 ; i<iw ; i++) {
      uchar r = curdata[0];
      uchar g =  curdata[1];
      uchar b =  curdata[2];


      if (!(i%40)) fprintf(output, "\n");
      fprintf(output, "%.2x%.2x%.2x", r, g, b);

      curdata +=D;
    }
    fprintf(output,"\n");

  }
  fprintf(output,">\n");

  fprintf(output,"restore\n");
  delete[] rgbdata;
}

void Fl_PS_Printer::draw_scalled_image_mono(const uchar *data, double x, double y, double w, double h, int iw, int ih, int D, int LD) {

  fprintf(output,"save\n");

  int i,j, k;

  char * interpol;
  if(lang_level_>1){
    if(interpolate_)
      interpol="true";
    else
      interpol="false";
    if(mask && lang_level_>2)
      fprintf(output, "%g %g %g %g %i %i %i %i %s GIM\n", x , y+h , w , -h , iw , ih, mx, my, interpol);
    else
      fprintf(output, "%g %g %g %g %i %i %s GII\n", x , y+h , w , -h , iw , ih, interpol);
  }else
    fprintf(output , "%g %g %g %g %i %i GI", x , y+h , w , -h , iw , ih);


  if(!LD) LD = iw*D;

  uchar bg_r, bg_g, bg_b;
  Fl::get_color(bg_, bg_r,bg_g,bg_b);
  int bg = (bg_r + bg_g + bg_b)/3;

  uchar *curmask=mask;
  for (j=0; j<ih;j++){
    if(mask){
      for(k=0;k<my/ih;k++){
        for (i=0; i<((mx+7)/8);i++){
          if (!(i%80)) fprintf(output, "\n");
          fprintf(output, "%.2x",swap_byte(*curmask));
          curmask++;
        }
        fprintf(output,"\n");
      }
    }
    const uchar *curdata=data+j*LD;
    for(i=0 ; i<iw ; i++) {
      if (!(i%80)) fprintf(output, "\n");
      uchar r = curdata[0];
      if(lang_level_<3 && D>1) { //can do  mixing

        unsigned int a2 = curdata[1]; //must be int
        unsigned int a = 255-a2;
        r = (a2 * r + bg * a)/255;
      }
      if (!(i%120)) fprintf(output, "\n");
      fprintf(output, "%.2x", r);
      curdata +=D;
    }
    fprintf(output,"\n");

  }

  fprintf(output," >\nrestore\n" );

};



void Fl_PS_Printer::draw_scalled_image_mono(Fl_Draw_Image_Cb call, void *data, double x, double y, double w, double h, int iw, int ih, int D) {

  fprintf(output,"save\n");
  int i,j,k;
  char * interpol;
  if(lang_level_>1){
    if(interpolate_) interpol="true";
    else interpol="false";
    if(mask && lang_level_>2)
      fprintf(output, "%g %g %g %g %i %i %i %i %s GIM\n", x , y+h , w , -h , iw , ih, mx, my, interpol);
    else
      fprintf(output, "%g %g %g %g %i %i %s GII\n", x , y+h , w , -h , iw , ih, interpol);
  }else
    fprintf(output , "%g %g %g %g %i %i GI", x , y+h , w , -h , iw , ih);

  int LD=iw*D;
  uchar *rgbdata=new uchar[LD];
  uchar *curmask=mask;
  for (j=0; j<ih;j++){

    if(mask && lang_level_>2){  // InterleaveType 2 mask data
      for(k=0; k<my/ih;k++){ //for alpha pseudo-masking
        for (i=0; i<((mx+7)/8);i++){
          if (!(i%40)) fprintf(output, "\n");
          fprintf(output, "%.2x",swap_byte(*curmask));
          curmask++;
        }
        fprintf(output,"\n");
      }
    }
    call(data,0,j,iw,rgbdata);
    uchar *curdata=rgbdata;
    for(i=0 ; i<iw ; i++) {
      uchar r = curdata[0];
      if (!(i%120)) fprintf(output, "\n");
      fprintf(output, "%.2x", r);
      curdata +=D;
    }
    fprintf(output,"\n");
  }
  fprintf(output,">\n");
  fprintf(output,"restore\n");
  delete[] rgbdata;
}


////////////////////////////// Image classes //////////////////////


void Fl_PS_Printer::draw(Fl_Pixmap * pxm,int XP, int YP, int WP, int HP, int cx, int cy){
  const char * const * di =pxm->data();
  int w,h;
  if (!fl_measure_pixmap(di, w, h)) return;
  mask=0;
  fl_mask_bitmap=&mask;
  mx = WP;
  my = HP;
  push_clip(XP, YP, WP, HP);
  fl_draw_pixmap(di,XP -cx, YP -cy, bg_); //yes, it is dirty, but fl is dispatched, so it works!
  pop_clip();
  delete[] mask;
  mask=0;
  fl_mask_bitmap=0;
};

void Fl_PS_Printer::draw(Fl_RGB_Image * rgb,int XP, int YP, int WP, int HP, int cx, int cy){
  const uchar  * di = rgb->array;
  int w = rgb->w();
  int h = rgb->h();
  mask=0;
  if(lang_level_>2) //when not true, not making alphamask, mixing colors instead...
  if (alpha_mask(di, w, h, rgb->d(),rgb->ld())) return; //everthing masked, no need for painting!
  push_clip(XP, YP, WP, HP);
  draw_scalled_image(di, XP + cx, YP + cy, w, h,  w,  h, rgb->d(), rgb->ld());
  pop_clip();
  delete[]mask;
  mask=0;
};

void Fl_PS_Printer::draw(Fl_Bitmap * bitmap,int XP, int YP, int WP, int HP, int cx, int cy){
  const uchar  * di = bitmap->array;
  int w,h;
  int LD=(bitmap->w()+7)/8;
  int xx;

  if (WP> bitmap->w() - cx){// to assure that it does not go out of bounds;
     w = bitmap->w() - cx;
     xx = (bitmap->w()+7)/8 - cx/8; //length of mask in bytes
  }else{
    w =WP;
    xx = (w+7)/8 - cx/8;
  }
  if( HP > bitmap->h()-cy)
    h = bitmap->h() - cy;
  else
    h = HP;

  di += cy*LD + cx/8;
  int si = cx % 8; // small shift to be clipped, it is simpler than shifting whole mask

  int i,j;
  push_clip(XP, YP, WP, HP);
  fprintf(output , "%i %i %i %i %i %i MI", XP - si, YP + HP , WP , -HP , w , h);

  for (j=0; j<HP; j++){
    for (i=0; i<xx; i++){
      if (!(i%80)) fprintf(output, "\n"); // not have lines longer than 255 chars
      fprintf(output, "%.2x",swap_byte(~(*di)));
      di++;
    }
    fprintf(output,"\n");
  }
  fprintf(output,">\n");
  pop_clip();
};

#ifndef WIN32

char * kprinter = "/usr/bin/kprinter";
FL_EXPORT char * fl_ps_command = access(kprinter,X_OK)?0:kprinter; 


#endif










  







