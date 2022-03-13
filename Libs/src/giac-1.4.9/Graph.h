// -*- mode:C++ ; compile-command: "g++ -I.. -g -c Graph.cc" -*-
#ifndef _GRAPH_H
#define _GRAPH_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifndef IN_GIAC
#include <giac/giac.h>
#else
#include "giac.h"
#endif
#include <fstream>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#define clock_t int
#define clock() 0
#endif
#ifdef HAVE_LIBFLTK
#include <FL/Fl_Menu.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Tile.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Counter.H>
#include <FL/Fl_Check_Button.H>
#endif
#ifdef HAVE_LC_MESSAGES
#include <locale.h>
#endif
#include <giacintl.h>
#include "Xcas1.h"
#include "Input.h"
#include "Editeur.h"
#include <string>

#ifndef NO_NAMESPACE_XCAS
namespace xcas {
#endif // ndef NO_NAMESPACE_XCAS

#ifdef HAVE_LIBFLTK

  // Locales initialization, should be done before history.o is loaded

  std::string & xcas_locale();
  class objet_bidon{
  public:
    objet_bidon();
  };
  // extern objet_bidon * mon_objet_bidon_ptr;

  void xcas_color(int color,bool dim3=false);

  int plotparam_dialog(Fl_Widget * spread_ptr,std::string & arg,int modeplot);
  std::string printstring(const giac::gen & g,const giac::context * contextptr);
  void round3(double & x,double xmin,double xmax);

  extern bool do_helpon;// true if Xcas_automatic_completion_browser enabled

  enum extended_colors {
    dark_blue_color=0x0f,
    dark_green_color =0x3d,
    braun_color=0x40,
    orange_color=0x5a
  };

  // quaternion struct for more intuitive rotations
  struct quaternion_double {
    double w,x,y,z;
    quaternion_double():w(1),x(0),y(0),z(0) {};
    quaternion_double(double theta_x,double theta_y,double theta_z);
    quaternion_double(double _w,double _x,double _y,double _z):w(_w),x(_x),y(_y),z(_z) {};
    double norm2() const { return w*w+x*x+y*y+z*z;}
  };

  quaternion_double operator * (const quaternion_double & q,const quaternion_double & q2);

  void get_axis_angle_deg(const quaternion_double & q,double &x,double &y,double & z, double &theta); // q must be a quaternion of unit norm, theta is in deg

  // Euler angle in degrees
  quaternion_double euler_deg_to_quaternion_double(double a,double b,double c);

  std::ostream & operator << (std::ostream & os,const quaternion_double & q);

  // end quaternion utilities
  // FLTK Drawing function with clipping checks, 
  // delta_i and delta_j should be 0
  void check_fl_draw(const char * ch,int i0,int j0,int imin,int jmin,int di,int dj,int delta_i,int delta_j);
  void check_fl_point(int i0,int j0,int imin,int jmin,int di,int dj,int delta_i,int delta_j);
  void check_fl_rect(int i0,int j0,int i1,int j1,int imin,int jmin,int di,int dj,int delta_i,int delta_j);
  void check_fl_rectf(int i0,int j0,int i1,int j1,int imin,int jmin,int di,int dj,int delta_i,int delta_j);
  void check_fl_line(int i0,int j0,int i1,int j1,int imin,int jmin,int di,int dj,int delta_i,int delta_j);

  class Graph2d3d;
  extern Fl_Menu_Item Graph2d3d_menu[];
  extern Fl_Menu_Item Figure_menu[];
  extern std::vector<Graph2d3d *> animations; // list of pointer to graph widgets

  class Mouse_Position:public Fl_Widget {
  public:
    Graph2d3d * graphic ;
    Mouse_Position(int x,int y,int w,int h,Graph2d3d * g):Fl_Widget(x,y,w,h),graphic(g) { box(FL_FLAT_BOX); }
    virtual FL_EXPORT void draw();
  };

  struct window_xyz { 
    double xmin,xmax,ymin,ymax,zmin,zmax; 
    window_xyz():xmin(-5),xmax(5),ymin(-5),ymax(5),zmin(-5),zmax(5) {};
    window_xyz(double x,double X,double y,double Y,double z,double Z):xmin(x),xmax(X),ymin(y),ymax(Y),zmin(z),zmax(Z) {};
  };

  giac::gen add_attributs(const giac::gen & g,int couleur_,const giac::context *) ;
  int change_line_type(int & res,bool show_approx,bool & approx,const std::string & title,bool dim3,bool & formel,bool & untranslate,bool del,int fontsize);

  class Graph2d3d:public Fl_Widget {
  public:
    int push_i,push_j,current_i,current_j,cursor_point_type; // position of mouse push/drag
  protected:
    double push_depth,current_depth;
    bool pushed; // true when mode==0 and push has occured
    bool in_area; // true if the mouse is in the area, updated by handle
    int mode; // 0 pointer, 1 1-arg, 2 2-args, etc.
    // plot_tmp=function_tmp(args_tmp) or function_final(args_tmp)
    // depends whether args.tmp.size()==mode
    giac::gen function_tmp,function_final,args_push; 
    giac::vecteur args_tmp; // WARNING should only contain numeric value
    unsigned args_tmp_push_size;
  public:
    std::vector<std::string> args_help;
    bool no_handle; // disable mouse handling
    bool show_mouse_on_object; // FL_MOVE always handled or not
    unsigned display_mode ; 
    // bit0=1 plot_instructions, bit1=1 animations_instruction
    // bit2=1 glFrustum/glOrtho, bit3=1 GL_LIGHTING, bit4=1 GL_FLAT, 
    // bit5=GL_BLEND, bit6=trace, bit7=1 move frame disabled
    // bit8=1 framebox, bit9=1 triedre
    // bit10=1 logplot 2d x, bit11=1 logplot 2d y, bit12=1 reserved for logplot 3d z
    double window_xmin,window_xmax,window_ymin,window_ymax,window_zmin,window_zmax;
    std::vector<window_xyz> history;
    int history_pos;
    quaternion_double q;
    int legende_size;
    double ylegende;
    // Fl_Tile * mouse_param_group ;
    Fl_Group * mouse_param_group ;
    Mouse_Position * mouse_position;
    Fl_Group * param_group ;
    Fl_Group * button_group ;
    Fl_Menu_Bar * menubar;
    History_Pack * hp;
    std::string title,x_axis_name,x_axis_unit,y_axis_name,y_axis_unit,z_axis_name,z_axis_unit,fcnfield,fcnvars;
    int npixels; // max # of pixels distance for click
    int show_axes,show_names;
    giac::vecteur plot_instructions,animation_instructions,trace_instructions ;
    double animation_dt; // rate for animated plot
    bool paused;
    struct timeval animation_last; // clock value at last display
    int animation_instructions_pos;
    int rotanim_type,rotanim_danim,rotanim_nstep;
    double rotanim_rx,rotanim_ry,rotanim_rz,rotanim_tstep;
    int last_event;
    double x_tick,y_tick,z_tick;
    int couleur; // used for new point creation in geometry
    bool approx; // exact or approx click mouse?
    std::vector<int> selected; // all items selected in plot_instructions
    giac::gen drag_original_value,drag_name;
    int hp_pos; // Position in hp for modification
    bool moving,moving_frame;
    // 3-d light information
    float light_x[8],light_y[8],light_z[8],light_w[8];
    float light_diffuse_r[8],light_diffuse_g[8],light_diffuse_b[8],light_diffuse_a[8];
    float light_specular_r[8],light_specular_g[8],light_specular_b[8],light_specular_a[8];
    float light_ambient_r[8],light_ambient_g[8],light_ambient_b[8],light_ambient_a[8];
    float light_spot_x[8],light_spot_y[8],light_spot_z[8],light_spot_w[8];
    float light_spot_exponent[8],light_spot_cutoff[8];
    float light_0[8],light_1[8],light_2[8];
    bool light_on[8];
    int ntheta,nphi; // default discretization params for sphere drawing
    std::pair<Fl_Image *,Fl_Image *> * background_image; // 2-d only
    int x_axis_color,y_axis_color,z_axis_color;
    Graph2d3d(int x,int y,int w,int h,const char * l,double xmin,double xmax,double ymin,double ymax,double zmin,double zmax,double ortho,History_Pack * hp_);
    Graph2d3d(int x,int y,int w,int h,const char * l=0,History_Pack * hp_=0);
    double find_eps(); // find value of a small real wrt the current graph
    void update_infos(const giac::gen & g,const giac::context * contextptr);
    virtual void zoom(double d);
    virtual void zoomx(double d,bool round=false);
    virtual void zoomy(double d,bool round=false);
    virtual void zoomz(double d,bool round=false);
    virtual void orthonormalize();
    virtual void autoscale(bool fullview=false);
    virtual void find_ylegende();
    virtual void up(double d);
    virtual void down(double d);
    virtual void up_z(double d);
    virtual void down_z(double d);
    virtual void left(double d);
    virtual void right(double d);
    virtual void move_cfg(int i); // moves forward/backward in cfg history
    virtual void push_cfg(); // save current config
    virtual void clear_cfg(); // reset history config
    virtual void config(); // show window for WX/Y/ortho/axis cfg
    virtual void config_light(unsigned i);
    virtual void reset_light(unsigned i);
    virtual void set_axes(int b);
    void glequal(bool equal); 
    void copy(const Graph2d3d & gr);
    void add_mouse_param_group(int x,int y,int w,int h);
    virtual void do_handle(const giac::gen & g);
    // if pos>=0, then update clears plot_instructions from pos included
    // except if the i-th child is a parameter
    void update(Fl_Group * pack,int pos=-1); 
    void clear(unsigned pos=0); // clear starting at position pos
    // add plot_instructions from widgets
    void add(const Fl_Widget * wid,int pos=0); 
    void add(const giac::gen & e);
    void add(const giac::vecteur & v);
    void autoname_plus_plus();
    virtual FL_EXPORT int handle(int);
    virtual int in_handle(int);
    int handle_mouse(int);
    int handle_keyboard(int);
    int geo_handle(int event);
    virtual const char * latex(const char * filename);
    giac::gen geometry_round(double x,double y,double z,double eps,giac::gen & original,int & pos,bool selectfirstlevel=false,bool setscroller=false) ;
    virtual void geometry_round(double x,double y,double z,double eps,giac::gen & tmp,const giac::context *) ;
    giac::vecteur selection2vecteur(const std::vector<int> & v);
    void set_mode(const giac::gen & f_tmp,const giac::gen & f_final,int m);
    virtual void find_xyz(double i,double j,double depth,double & x,double & y,double & z);
    void find_title_plot(giac::gen & title_tmp,giac::gen & plot_tmp,const giac::context * contextptr);
    void change_attributs();
    void change_attributs(int hp_pos);
    void resize_mouse_param_group(int W);
    virtual void resize(int X,int Y,int W,int H);    
    virtual ~Graph2d3d();
    giac::vecteur animate(int nframes);
    std::string current_config();
    // return a sequence of graphic objects, ready for animation
    void adjust_cursor_point_type();
  };

  class Graph2d:public Graph2d3d {
  protected:
  public:
    double orthonormal_factor;
    giac::gen waiting_click_value;
    bool waiting_click;
    void in_draw(int x_clip,int y_clip,int w_clip,int h_clip,int & vertical_pixels);
    virtual FL_EXPORT void draw();
    int common_in_handle(int event);
    virtual int in_handle(int);
    virtual void orthonormalize();
    void find_xy(double i,double j,double & x,double & y) const ;
    bool findij(const giac::gen & e0,double x_scale,double y_scale,double & i0,double & j0,const giac::context * contextptr) const ;
    Graph2d(int x,int y,int w,int h,const char * l=0,History_Pack * hp_=0);
    virtual const char * latex(const char * filename);
    virtual void find_xyz(double i,double j,double depth,double & x,double & y,double & z);
    virtual void geometry_round(double x,double y,double z,double eps,giac::gen & tmp,const giac::context *) ;
    int mouse_rescale();
  };

  Graph2d3d * find_graph2d3d(Fl_Widget * wid);

  // find how to move position for legende drawing
  void find_dxdy(const std::string & legendes,int labelpos,int labelsize,int & dx,int & dy);
  std::string print_color(int couleur);

  int figure_param_dialog(Fl_Widget * f,bool adjust,double & tmin,double & tmax,double & tcurrent,double & tstep,std::string & name,bool symb,std::string & tmp);

  giac::gen geometry_round_numeric(double x,double y,double eps,bool approx);
  giac::gen geometry_round_numeric(double x,double y,double z,double eps,bool approx);

  // Should be inside a Fl_Tile (1st element is a History_Pack
  // with Multiline_Input and 
  // 2nd element is a Geo2d object and 3rd element a Fl_Pack of Parameters
  class Geo2d:public Graph2d {
  public:
    virtual FL_EXPORT void draw();
    virtual int in_handle(int event);
    Geo2d(int x,int y,int w,int h,History_Pack * _hp=0,const char * l=0);
  };

  History_Pack * geo_find_history_pack(Fl_Widget * wid);

  class Line_Type:public Fl_Button {
    int line_type_;
    bool show_pnt_,show_line_,show_text_,show_poly_;
  public:
    Line_Type(int x,int y,int w,int h,int lt=0);
    void line_type(int l);
    int line_type() const { return line_type_; }
    void show_pnt(bool b){ show_pnt_=b; redraw(); }
    bool show_pnt(){ return show_pnt_; }
    void show_line(bool b){ show_line_=b; redraw(); }
    bool show_line(){ return show_line_; }
    void show_text(bool b){ show_text_=b; redraw(); }
    bool show_text(){ return show_text_; }
    void show_poly(bool b){ show_poly_=b; redraw(); }
    bool show_poly(){ return show_poly_; }
    virtual FL_EXPORT void draw();
  };

  class Figure:public Fl_Tile {
  public:
    Graph2d3d * geo;
    Fl_Button * name;
    char * namestr;
    Fl_Output * mode;
    Fl_Button * couleur;
    Line_Type * lt;
    Fl_Group * barre;
    xcas::HScroll * s;
    Fl_Check_Button * checkdisp;
    Fl_Window * win;
    int disposition;
    Figure(int X,int Y,int W,int H,int L,bool dim3=false);
    void save_figure_as(const std::string & s);
    std::string latex_save_figure();
    void rename(const std::string & s);
    virtual void resize(int X,int Y,int W,int H,double dhp=0.25,double dgeo=0.5,double dmp=0.25);    
    virtual int handle(int event);
    virtual void draw();
  };

  std::string figure_insert(Figure * f);

  void gen_value_slider_cb(Fl_Widget * widget,void *);

  class Gen_Value_Slider:public Fl_Counter {
  public:
    int pos;
    std::string paramname;
    Gen_Value_Slider(int x,int y,int w,int h,int _pos,double m,double M,double mystep,const std::string & pname);
    // adjust history level to the slider value, and eval history pack
    // if eval_hp is true
    void adjust(bool );
    virtual int handle(int event);
  };
  
  Gen_Value_Slider * parameter2slider(const giac::gen & e,const giac::context *);

  class Turtle:public Fl_Widget {
  public:
    virtual FL_EXPORT void draw();
    virtual FL_EXPORT void indraw();
    virtual FL_EXPORT int handle(int);
    std::vector<giac::logo_turtle> * turtleptr;
    int legende_size;
    int turtlex,turtley; // Turtle translate
    double turtlezoom; // Zoom factor for turtle screen
    int maillage; // 0 (none), 1 (square), 2 (triangle), bit3 used for printing
    bool redraw_cap_only;
    int push_x,push_y;
    Turtle(int x,int y,int w,int h,const char * l=0): Fl_Widget(x,y,w,h,l),turtleptr(0),legende_size(giac::LEGENDE_SIZE),turtlex(0),turtley(0),turtlezoom(1),maillage(1),redraw_cap_only(false) {};    
    std::string latex_save_figure();
    const char * latex(const char * filename_) const ;
    ~Turtle();
  };

  class Editeur; 

  class Logo:public Fl_Tile {
  public:
    History_Pack * hp;
    Turtle * t;
    Fl_Group * button_group;
    Editeur * ed;
    Fl_Menu_Bar * menubar ;
    int scroll_position;
    Logo(int X,int Y,int W,int H,int L);
    virtual void resize(int X,int Y,int W,int H);
    virtual void draw();
    virtual int handle(int);
  };


  std::ostream & fltk_fl_widget_archive_function(std::ostream & os,void * ptr);
  giac::gen fltk_fl_widget_unarchive_function(std::istream & os);
  std::string fltk_fl_widget_texprint_function(void * ptr);
  giac::gen fltk_fl_widget_updatepict_function(const giac::gen &g);

  // If an auto-recovery file is found in s directory, returns true
  // and sets newest to the filename
  bool has_autorecover_data(const std::string & s,std::string & newest);

  // for xyztrange to change focused graphic config
  giac::gen Xcas_xyztrange(const giac::gen & g,const giac::context *);
  Figure * find_figure(Fl_Widget * widget);

  class BorderBox:public Fl_Box {
  public:
    BorderBox(int X,int Y,int W,int H):Fl_Box(X,Y,W,H,"") {}
    virtual int handle(int event);
  };

#endif // HAVE_LIBFLTK

#ifndef NO_NAMESPACE_XCAS
} // namespace xcas
#endif // ndef NO_NAMESPACE_XCAS

#endif // _GRAPH_H
