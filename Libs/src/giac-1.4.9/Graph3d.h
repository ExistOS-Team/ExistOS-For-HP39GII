// -*- mode:C++ ; compile-command: "g++ -I.. -g -c Graph3d.cc" -*-
#ifndef _GRAPH3D_H
#define _GRAPH3D_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifndef IN_GIAC
#include <giac/giac.h>
#else
#include "giac.h"
#endif
#include <fstream>
#include <string>
#include <stdio.h>
#ifdef HAVE_LIBFLTK_GL
#include <FL/Fl_Menu.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Tile.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/gl.h>
#endif

#ifdef HAVE_LC_MESSAGES
#include <locale.h>
#endif
#include "giacintl.h"
#include "Xcas1.h"
#include "Input.h"
#include "Editeur.h"

#ifndef NO_NAMESPACE_XCAS
namespace xcas {
#endif // ndef NO_NAMESPACE_XCAS
#ifdef HAVE_LIBFLTK

#ifdef HAVE_LIBFLTK_GL

  // translate giac GL constant to open GL constant
  unsigned gl_translate(unsigned i);
  // utilities for matrix 4x4 represented as a double[16] 
  // written in columns
  void mult4(double * colmat,double * vect,double * res);
  void mult4(double * colmat,float * vect,double * res);
  void mult4(double * c,double k,double * res);
  double det4(double * c);
  void inv4(double * c,double * res);
  // return in i and j the distance to the BOTTOM LEFT of the window
  // use window()->h()-j for the FLTK coordinates in this window
  void dim32dim2(double * view,double * proj,double * model,double x0,double y0,double z0,double & i,double & j,double & depth);
  // quaternion for the rotation of axis (x,y,z) angle theta
  quaternion_double rotation_2_quaternion_double(double x, double y, double z,double theta);

  class Graph3d : public Graph2d3d {
  public:
    Graph3d(int x,int y,int width, int height, const char* title,History_Pack * hp_);
    virtual ~Graph3d();
    double theta_z,theta_x,theta_y; // rotations
    double delta_theta; // rotation increment
    int draw_mode; // for sphere drawing
    FILE * printing; // non 0 if printing with gl2ps
    void * glcontext;
    // save values of the projection and modelview matrices
    double proj[16],model[16],proj_inv[16],model_inv[16]; 
    double view[4];
    int dragi,dragj;
    bool push_in_area;
    double depth;
    bool below_depth_hidden;
    unsigned char * screenbuf;
    virtual void resize(int X,int Y,int W,int H);
    virtual void draw();
    virtual void orthonormalize();
    void display(); 
    // internally callled by draw, maybe multiple times when printing
    void print(); // assumes that printing is assigned to a FILE *
    virtual int in_handle(int event);
    void indraw(const giac::vecteur & v);
    void indraw(const giac::gen & g);
    void legende_draw(const giac::gen & g,const std::string & s,int mode);
    void draw_string(const std::string & s);
    virtual const char * latex(const char * filename);
    // i,j,z -> x,y
    virtual void find_xyz(double i,double j,double depth,double & x,double & y,double & z)  ;
    // x,y,z -> FLTK coordinates i,j
    void find_ij(double x,double y,double z,double & i,double & j,double & depth) ;
    void current_normal(double & a,double &b,double &c) ;
    void normal2plan(double & a,double &b,double &c);
    virtual void geometry_round(double x,double y,double z,double eps,giac::gen & tmp,const giac::context *) ;
    int opengl2png(const std::string & filename);
  };

  class Geo3d : public Graph3d {
  public:
    virtual FL_EXPORT void draw();
    virtual int in_handle(int event);
    Geo3d(int x,int y,int width, int height, History_Pack * _hp);
  };
#else
  class Graph3d : public Graph2d3d {
  public:
    void print(){}; // assumes that printing is assigned to a FILE *
    Graph3d(int x,int y,int width, int height, const char* title,History_Pack * hp_):Graph2d3d(x,y,width,height,title,hp_),printing(0) {};
    double theta_z,theta_x,theta_y; // rotations
    double delta_theta; // rotation increment
    int draw_mode; // for sphere drawing
    FILE * printing; // non 0 if printing with gl2ps
    void * glcontext;
    // save values of the projection and modelview matrices
    double proj[16],model[16],proj_inv[16],model_inv[16]; 
    double view[4];
    int dragi,dragj;
    bool push_in_area;
    double depth;
    bool below_depth_hidden;
    unsigned char * screenbuf;
    // virtual void resize(int X,int Y,int W,int H);
    // virtual void draw();
    // virtual void orthonormalize();
    // void display(); 
    // void indraw(const giac::vecteur & v);
    // void indraw(const giac::gen & g);
    // void legende_draw(const giac::gen & g,const std::string & s,int mode);
    // void draw_string(const std::string & s);
    // virtual const char * latex(const char * filename);
    // i,j,z -> x,y
    // virtual void find_xyz(double i,double j,double depth,double & x,double & y,double & z)  ;
    // x,y,z -> FLTK coordinates i,j
    // void find_ij(double x,double y,double z,double & i,double & j,double & depth) ;
    void current_normal(double & a,double &b,double &c) {};
    void normal2plan(double & a,double &b,double &c){};
    // virtual void geometry_round(double x,double y,double z,double eps,giac::gen & tmp,const giac::context *) ;
    int opengl2png(const std::string & filename){ return 0;};
  };

  class Geo3d : public Graph3d {
  public:
    Geo3d(int x,int y,int width, int height, History_Pack * _hp):Graph3d(x,y,width,height,0,_hp){};
    virtual FL_EXPORT void draw(){};
  };

  inline void mult4(double * colmat,double * vect,double * res){};
  inline void mult4(double * colmat,float * vect,double * res){};
  inline void mult4(double * c,double k,double * res){};
  inline quaternion_double rotation_2_quaternion_double(double x, double y, double z,double theta){};

#endif // HAVE_LIBFLTK_GL
#endif // HAVE_LIBFLTK

#ifndef NO_NAMESPACE_XCAS
} // namespace xcas
#endif // ndef NO_NAMESPACE_XCAS

#endif // _GRAPH3D_H
