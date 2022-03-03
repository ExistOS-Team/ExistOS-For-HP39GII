// -*- mode:C++ ; compile-command: "g++ -DHAVE_CONFIG_H -I. -I.. -I../include -I../../giac/include -g -c Graph.cc -DHAVE_CONFIG_H -DIN_GIAC" -*-
#include "Graph.h"
/*
 *  Copyright (C) 2000,2014 B. Parisse, Institut Fourier, 38402 St Martin d'Heres
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_LIBFLTK
#include <FL/fl_ask.H>
#include <FL/fl_ask.H>
#include <FL/Fl.H>
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/fl_show_colormap.H>
#include <FL/Fl_Tiled_Image.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Shared_Image.H>
#include <fstream>
#include "vector.h"
#include <algorithm>
#include <fcntl.h>
#include <cmath>
#include <time.h> // for nanosleep
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h> // auto-recovery function
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include "path.h"
#ifndef IN_GIAC
#include <giac/plot.h>
#else
#include "plot.h"
#endif
#include "Equation.h"
#include "Editeur.h"
#include "Xcas1.h"
#include "Print.h"
#include "Graph3d.h"
#include <FL/gl.h>
#include "Tableur.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

using namespace std;
using namespace giac;


#ifndef NO_NAMESPACE_XCAS
namespace xcas {
#endif // ndef NO_NAMESPACE_XCAS

  bool do_helpon=true;

  std::map<std::string,std::pair<Fl_Image *,Fl_Image *> *> texture2d_cache;

  static double pow10(double d){
    return std::pow(10.,d);
  }

  void get_texture2d(const string & s,std::pair<Fl_Image *,Fl_Image *> * & texture){
    texture=0;
    std::map<std::string,std::pair<Fl_Image *,Fl_Image*> *>::const_iterator it,itend=texture2d_cache.end();
    it=texture2d_cache.find(s);
    if (it!=itend){
      texture=it->second;
      // texture->uncache();
    }
    else {
      Fl_Shared_Image * ptr =Fl_Shared_Image::get(s.c_str());
      if (ptr) 
	ptr->reload();
      texture=new std::pair<Fl_Image *,Fl_Image*>(ptr,0);
      texture2d_cache[s]=texture;
    }
  }

  void arc_en_ciel(int k,int & r,int & g,int & b){
    k += 21;
    k %= 126;
    if (k<0)
      k += 126;
    if (k<21){
      r=251; g=0; b=12*k;
    }
    if (k>=21 && k<42){
      r=251-(12*(k-21)); g=0; b=251;
    } 
    if (k>=42 && k<63){
      r=0; g=(k-42)*12; b=251;
    } 
    if (k>=63 && k<84){
      r=0; g=251; b=251-(k-63)*12;
    } 
    if (k>=84 && k<105){
      r=(k-84)*12; g=251; b=0;
    } 
    if (k>=105 && k<126){
      r=251; g=251-(k-105)*12; b=0;
    } 
  }

  void xcas_color(int color,bool dim3){
    if (color>=0x100 && color<0x17e){
      int r,g,b;
      arc_en_ciel(color-0x100,r,g,b);
#ifdef HAVE_LIBFLTK_GL
      if (dim3)
	glColor3f(r/255.,g/255.,b/255.);
      else
#endif
	fl_color(r,g,b);
    }
    else {
      if (dim3)
	gl_color(color);
      else
	fl_color(color);
    }
  }

  vector<Graph2d3d *> animations;

  inline int Min(int i,int j) {return i>j?j:i;}

  inline int Max(int i,int j) {return i>j?i:j;}

  quaternion_double::quaternion_double(double theta_x,double theta_y,double theta_z) { 
    *this=euler_deg_to_quaternion_double(theta_x,theta_y,theta_z); 
  }

  quaternion_double euler_deg_to_quaternion_double(double a,double b,double c){
    double phi=a*M_PI/180, theta=b*M_PI/180, psi=c*M_PI/180;
    double c1 = std::cos(phi/2);
    double s1 = std::sin(phi/2);
    double c2 = std::cos(theta/2);
    double s2 = std::sin(theta/2);
    double c3 = std::cos(psi/2);
    double s3 = std::sin(psi/2);
    double c1c2 = c1*c2;
    double s1s2 = s1*s2;
    double w =c1c2*c3 - s1s2*s3;
    double x =c1c2*s3 + s1s2*c3;
    double y =s1*c2*c3 + c1*s2*s3;
    double z =c1*s2*c3 - s1*c2*s3;
    return quaternion_double(w,x,y,z);
  }

  void quaternion_double_to_euler_deg(const quaternion_double & q,double & phi,double & theta, double & psi){
    double test = q.x*q.y + q.z*q.w;
    if (test > 0.499) { // singularity at north pole
      phi = 2 * atan2(q.x,q.w) * 180/M_PI;
      theta = 90; 
      psi = 0;
      return;
    }
    if (test < -0.499) { // singularity at south pole
      phi = -2 * atan2(q.x,q.w) * 180/M_PI;
      theta = - 90;
      psi = 0;
      return;
    }
    double sqx = q.x*q.x;
    double sqy = q.y*q.y;
    double sqz = q.z*q.z;
    phi = atan2(2*q.y*q.w-2*q.x*q.z , 1 - 2*sqy - 2*sqz) * 180/M_PI;
    theta = asin(2*test) * 180/M_PI;
    psi = atan2(2*q.x*q.w-2*q.y*q.z , 1 - 2*sqx - 2*sqz) * 180/M_PI;
  }

  quaternion_double operator * (const quaternion_double & q1,const quaternion_double & q2){ 
    double z=q1.w*q2.z+q2.w*q1.z+q1.x*q2.y-q2.x*q1.y;
    double x=q1.w*q2.x+q2.w*q1.x+q1.y*q2.z-q2.y*q1.z;
    double y=q1.w*q2.y+q2.w*q1.y+q1.z*q2.x-q2.z*q1.x;
    double w=q1.w*q2.w-q1.x*q2.x-q1.y*q2.y-q1.z*q2.z;
    return quaternion_double(w,x,y,z);
  }

  // q must be a unit
  void get_axis_angle_deg(const quaternion_double & q,double &x,double &y,double & z, double &theta){
    double scale=1-q.w*q.w;
    if (scale>1e-6){
      scale=std::sqrt(scale);
      theta=2*std::acos(q.w)*180/M_PI;
      x=q.x/scale;
      y=q.y/scale;
      z=q.z/scale;
    }
    else {
      x=0; y=0; z=1;
      theta=0;
    }
  }

  ostream & operator << (ostream & os,const quaternion_double & q){
    return os << q.w << "+" << q.x << "i+" << q.y << "j+" << q.z << "k";
  }

  bool readrgb(const string & s,int W,int H,gen & res){
    Fl_Image * image=Fl_Shared_Image::get(s.c_str());
    if (image && W && H)
      image=image->copy(W,H);
    if (!image || image->count()!=1)
      return false;
    const char * data = image->data()[0];
    unsigned ih=image->h(),iw=image->w(),id=image->d();
    gen tmpr=vecteur(ih),tmpg=vecteur(ih),tmpb=vecteur(ih),tmpa=vecteur(ih);
    // alpha is returned before blue because red==1, green==2, blue==4
    res=gen(makevecteur(makevecteur(4,int(ih),int(iw)),tmpr,tmpg,tmpa,tmpb),_RGBA__VECT);
    vecteur & vr=*tmpr._VECTptr;
    vecteur & vg=*tmpg._VECTptr;
    vecteur & vb=*tmpb._VECTptr;
    vecteur & va=*tmpa._VECTptr;
    unsigned l=0;
    for (unsigned i=0;i<ih;++i){
      if (debug_infolevel)
	cerr << "readrgb, reading row " << i << endl;
      vr[i]=vecteur(iw);
      vg[i]=vecteur(iw);
      vb[i]=vecteur(iw);
      va[i]=vecteur(iw,255);
      vecteur & wr = *vr[i]._VECTptr;
      vecteur & wg = *vg[i]._VECTptr;
      vecteur & wb = *vb[i]._VECTptr;
      vecteur & wa = *va[i]._VECTptr;
      for (unsigned j=0;j<iw;++j){
	for (unsigned k=0;k<id;++k,++l){
	  unsigned u=data[l] & 0xff;
	  switch(k){
	  case 0:
	    wr[j]=int(u);
	    break;
	  case 1:
	    wg[j]=int(u);
	    break;
	  case 2:
	    wb[j]=int(u);
	    break;
	  case 3:
	    wa[j]=int(u);
	    break;
	  }
	}
      }      
    }
    return true;
  }

  objet_bidon::objet_bidon(){
    // localization code and pointer to RGB image reader
    if (!giac::readrgb_ptr){
      giac::readrgb_ptr=readrgb;
#ifdef HAVE_LC_MESSAGES
      xcas_locale()=getenv("XCAS_LOCALE")?getenv("XCAS_LOCALE"):giac_locale_location;	
      cerr << "// Using locale " << xcas_locale() << endl;
      cerr << "// " << setlocale (LC_MESSAGES, "") << endl;
#if defined(HAVE_GETTEXT) 
      cerr << "// " << bindtextdomain (PACKAGE, xcas_locale().c_str()) << endl;
      cerr << "// " << textdomain (PACKAGE) << endl;
      cerr << "// " << bind_textdomain_codeset (PACKAGE, "UTF-8") << endl;
#endif
#endif
    }
  };

  std::string & xcas_locale(){
    static string * ans = new string;
    return * ans;
  }
  objet_bidon mon_objet_bidon_graph;

  // New buttons/menus: 
  // row1: z+   up_y  up_z
  // row2: left ortho right
  // row3: z-   down  down_z
  // row4: back forw  cfg_menu
  void cb_graph_buttons(Fl_Widget * b,void *){
    Fl_Group * g = b->parent();
    if (!g) return;
    int i=g->find(b); // position of button inside group of buttons
    g = g->parent(); // should be mouse_param_group
    if (!g) return;
    if (Mouse_Position * mp=dynamic_cast<Mouse_Position *>(g->child(0))){
      Graph3d * g3=dynamic_cast<Graph3d *>(mp->graphic);
      double dh=(mp->graphic->window_ymax-mp->graphic->window_ymin)/10;
      double dw=(mp->graphic->window_xmax-mp->graphic->window_xmin)/10;
      double dz=(mp->graphic->window_zmax-mp->graphic->window_zmin)/10;
      switch (i){
      case 0: 
	mp->graphic->zoom(0.707);
	break;
      case 1:
	mp->graphic->up(dh);
	break;
      case 2:
	if (g3)
	  mp->graphic->up_z(dz);
	else {
	  mp->graphic->zoomy(0.707);
	  mp->graphic->push_cfg();
	}
	break;	
      case 3:
	mp->graphic->left(dw);
	break;
      case 4:
	if (g3){
	  g3->below_depth_hidden=!g3->below_depth_hidden;
	  g3->redraw();
	}
	else 
	  mp->graphic->orthonormalize();
	break;
      case 5:
	mp->graphic->right(dw);
	break;
      case 6:
	mp->graphic->zoom(1.414);
	break;
      case 7:
	mp->graphic->down(dh);
	break;
      case 8:
	if (g3)
	  mp->graphic->down_z(dz);
	else {
	  mp->graphic->zoomy(1.414);
	  mp->graphic->push_cfg();
	}
	break;	
      case 9:
	mp->graphic->move_cfg(-1);
	break;
      case 10:
	mp->graphic->move_cfg(1);
	break;
      case 11:
	mp->graphic->config();
	break;
      case 12:
	mp->graphic->paused=!mp->graphic->paused;
	break;
      case 13:
	mp->graphic->autoscale(false);
	break;
      }
      if (Fl_Widget * g=dynamic_cast<Graph2d *>(mp->graphic))
	g->redraw();
    }
  }

  void Graph2d3d::update_infos(const gen & g,GIAC_CONTEXT){
    if (g.is_symb_of_sommet(at_equal)){
      // detect a title or a x/y-axis name
      gen & f = g._SYMBptr->feuille;
      if (f.type==_VECT && f._VECTptr->size()==2){
	gen & optname = f._VECTptr->front();
	gen & optvalue= f._VECTptr->back();
	if (optname==at_legende && optvalue.type==_VECT){
	  vecteur & optv=(*optvalue._VECTptr);
	  int optvs=optv.size();
	  if (optvs>=1)
	    x_axis_unit=printstring(optv[0],contextptr);
	  if (optvs>=2)
	    y_axis_unit=printstring(optv[1],contextptr);
	  if (optvs>=3)
	    z_axis_unit=printstring(optv[2],contextptr);
	}
	if (optname.type==_INT_ && optname.subtype == _INT_PLOT){ 
	  if (optname.val==_GL_TEXTURE){
	    if (optvalue.type==_VECT && optvalue._VECTptr->size()==2 && optvalue._VECTptr->front().type==_STRNG && is_undef(optvalue._VECTptr->back())){
	      // reload cached image
	      optvalue=optvalue._VECTptr->front();
	      std::map<std::string,std::pair<Fl_Image *,Fl_Image*> *>::iterator it,itend=texture2d_cache.end();
	      it=texture2d_cache.find(optvalue._STRNGptr->c_str());
	      if (it!=itend){
		std::pair<Fl_Image *,Fl_Image*> * old= it->second;
		delete old;
		texture2d_cache.erase(it);
	      }
	      get_texture2d(*optvalue._STRNGptr,background_image);
	    }
	    else {
	      if (optvalue.type==_STRNG){
		get_texture2d(*optvalue._STRNGptr,background_image);
	      }
	      else {
		background_image=0;
	      }
	    }
	  }
	  if (optname.val==_TITLE )
	    title=printstring(optvalue,contextptr);
	  if (optname.val==_AXES){
	    if (optvalue.type==_INT_)
	      show_axes=optvalue.val;
	  }
	  if (optname.val==_LABELS && optvalue.type==_VECT){
	    vecteur & optv=(*optvalue._VECTptr);
	    int optvs=optv.size();
	    if (optvs>=1)
	      x_axis_name=printstring(optv[0],contextptr);
	    if (optvs>=2)
	      y_axis_name=printstring(optv[1],contextptr);
	    if (optvs>=3)
	      z_axis_name=printstring(optv[2],contextptr);
	  }
	  if (optname.val==_GL_ORTHO && optvalue==1)
	    orthonormalize();
	  if (optname.val==_GL_X_AXIS_COLOR && optvalue.type==_INT_)
	    x_axis_color=optvalue.val;
	  if (optname.val==_GL_Y_AXIS_COLOR && optvalue.type==_INT_)
	    y_axis_color=optvalue.val;
	  if (optname.val==_GL_Z_AXIS_COLOR && optvalue.type==_INT_)
	    z_axis_color=optvalue.val;
	  if (optname.val>=_GL_X && optname.val<=_GL_Z && optvalue.is_symb_of_sommet(at_interval)){
	    gen optvf=evalf_double(optvalue._SYMBptr->feuille,1,contextptr);
	    if (optvf.type==_VECT && optvf._VECTptr->size()==2){
	      gen a=optvf._VECTptr->front();
	      gen b=optvf._VECTptr->back();
	      if (a.type==_DOUBLE_ && b.type==_DOUBLE_){
		switch (optname.val){
		case _GL_X:
		  window_xmin=a._DOUBLE_val;
		  window_xmax=b._DOUBLE_val;
		  break;
		case _GL_Y:
		  window_ymin=a._DOUBLE_val;
		  window_ymax=b._DOUBLE_val;
		  break;
		case _GL_Z:
		  window_zmin=a._DOUBLE_val;
		  window_zmax=b._DOUBLE_val;
		  break;
		}
	      }
	    }
	  }
	  gen optvalf=evalf_double(optvalue,1,contextptr);
	  if (optname.val==_GL_XTICK && optvalf.type==_DOUBLE_)
	    x_tick=optvalf._DOUBLE_val;
	  if (optname.val==_GL_YTICK && optvalf.type==_DOUBLE_)
	    y_tick=optvalf._DOUBLE_val;
	  if (optname.val==_GL_ZTICK && optvalf.type==_DOUBLE_)
	    z_tick=optvalf._DOUBLE_val;
	  if (optname.val==_GL_ANIMATE && optvalf.type==_DOUBLE_)
	    animation_dt=optvalf._DOUBLE_val;
	  if (optname.val==_GL_SHOWAXES && optvalue.type==_INT_)
	    show_axes=optvalue.val;
	  if (optname.val==_GL_SHOWNAMES && optvalue.type==_INT_)
	    show_names=optvalue.val;
	  if (optname.val>=_GL_X_AXIS_NAME && optname.val<=_GL_Z_AXIS_UNIT && optvalue.type==_STRNG){
	    if (optname.val==_GL_X_AXIS_NAME) x_axis_name=*optvalue._STRNGptr;
	    if (optname.val==_GL_Y_AXIS_NAME) y_axis_name=*optvalue._STRNGptr;
	    if (optname.val==_GL_Z_AXIS_NAME) z_axis_name=*optvalue._STRNGptr;
	    if (optname.val==_GL_X_AXIS_UNIT) x_axis_unit=*optvalue._STRNGptr;
	    if (optname.val==_GL_Y_AXIS_UNIT) y_axis_unit=*optvalue._STRNGptr;
	    if (optname.val==_GL_Z_AXIS_UNIT) z_axis_unit=*optvalue._STRNGptr;
	  }
	  if (optname.val==_GL_QUATERNION && optvalf.type==_VECT && optvalf._VECTptr->size()==4){
	    vecteur & optvalv=*optvalf._VECTptr;
	    if (optvalv[0].type==_DOUBLE_ && optvalv[1].type==_DOUBLE_ && 
		optvalv[2].type==_DOUBLE_ && optvalv[3].type==_DOUBLE_){
	      q.x=optvalv[0]._DOUBLE_val;
	      q.y=optvalv[1]._DOUBLE_val;
	      q.z=optvalv[2]._DOUBLE_val;
	      q.w=optvalv[3]._DOUBLE_val;
	    }
	  }
	  if (optname.val==_GL_LOGX && optvalue.type==_INT_){
	    display_mode &= (0xffff ^ 0x400);
	    if (optvalue.val)
	      display_mode |= 0x400;
	  }
	  if (optname.val==_GL_LOGY && optvalue.type==_INT_){
	    display_mode &= (0xffff ^ 0x800);
	    if (optvalue.val)
	      display_mode |= 0x800;
	  }
	  if (optname.val==_GL_LOGZ && optvalue.type==_INT_){
	    display_mode &= (0xffff ^ 0x1000);
	    if (optvalue.val)
	      display_mode |= 0x1000;
	  }
	  if (dynamic_cast<Graph3d *>(this)){
	    if (optname.val==_GL_ROTATION_AXIS && optvalf.type==_VECT && optvalf._VECTptr->size()==3){
	      vecteur & optvalv=*optvalf._VECTptr;
	      if (optvalv[0].type==_DOUBLE_ && optvalv[1].type==_DOUBLE_ && 
		  optvalv[2].type==_DOUBLE_ ){
		rotanim_rx=optvalv[0]._DOUBLE_val;
		rotanim_ry=optvalv[1]._DOUBLE_val;
		rotanim_rz=optvalv[2]._DOUBLE_val;	
	      }      
	    }
	    if (optname.val==_GL_FLAT && optvalue.type==_INT_){
	      display_mode &= (0xffff ^ 0x10);
	      if (optvalue.val)
		display_mode |= 0x10;
	    }
	    if (optname.val==_GL_LIGHT && optvalue.type==_INT_){
	      display_mode &= (0xffff ^ 0x8);
	      if (optvalue.val)
		display_mode |= 0x8;
	    }
	    if (optname.val==_GL_PERSPECTIVE && optvalue.type==_INT_){
	      display_mode &= (0xffff ^ 0x4);
	      if (!optvalue.val)
		display_mode |= 0x4;
	    }
	    // GL_LIGHT_MODEL_COLOR_CONTROL=GL_SEPARATE_SPECULAR_COLOR ||  GL_SINGLE_COLOR
#ifndef WIN32
	    if (optname.val==_GL_LIGHT_MODEL_COLOR_CONTROL && optvalue.type==_INT_)
	      glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,optvalue.val);
	    /* GL_LIGHT_MODEL_LOCAL_VIEWER=floating-point value that spec-
	       ifies how specular reflection angles are computed.  If params
	       is 0 (or 0.0),  specular  reflection  angles  take  the  view
	       direction  to  be  parallel to and in the direction of the -z
	       axis, regardless of the location of the vertex in eye coordi-
	       nates.  Otherwise, specular reflections are computed from the
	       origin of the eye coordinate system.  The initial value is 0. */
	    if (optname.val==_GL_LIGHT_MODEL_LOCAL_VIEWER){
	      if (optvalf.type==_DOUBLE_)
		glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER,optvalf._DOUBLE_val);
	    }
#endif
#ifdef HAVE_LIBFLTK_GL
	    /* GL_LIGHT_MODEL_TWO_SIDE = true /false */
	    if (optname.val==_GL_LIGHT_MODEL_TWO_SIDE && optvalue.type==_INT_){
	      glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,optvalue.val);
	    }
	    /* GL_LIGHT_MODEL_AMBIENT=[r,g,b,a] */
	    if (optname.val==_GL_LIGHT_MODEL_AMBIENT && optvalf.type==_VECT && optvalf._VECTptr->size()==4){
	      vecteur & w=*optvalf._VECTptr;
	      GLfloat tab[4]={w[0]._DOUBLE_val,w[1]._DOUBLE_val,w[2]._DOUBLE_val,w[3]._DOUBLE_val};
	      glLightModelfv(GL_LIGHT_MODEL_AMBIENT,tab);
	    }
	    // gl_blend=[d,s] 
	    // habituellement gl_blend=[gl_src_alpha,gl_one_minus_src_alpha]
	    if (optname.val==_GL_BLEND){
	      if (is_zero(optvalue)){
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
	      }
	      else {
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		if (optvalue.type==_VECT && optvalue._VECTptr->size()==2)
		  glBlendFunc(optvalue._VECTptr->front().val,optvalue._VECTptr->back().val);
		if (is_minus_one(optvalue))
		  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	      }
	    }
#endif
	    // gl_light0=[option1=value1,...]
	    if (optname.val>=_GL_LIGHT0 && optname.val<=_GL_LIGHT7 && optvalue.type==_VECT){
	      int j=optname.val-_GL_LIGHT0;
	      // reset light0+j
	      light_x[j]=0;light_y[j]=0;light_z[j]=0;light_w[j]=1;
	      float di=j?0:1;
	      light_diffuse_r[j]=di;light_diffuse_g[j]=di;light_diffuse_b[j]=di;light_diffuse_a[j]=di;
	      light_specular_r[j]=di;light_specular_g[j]=di;light_specular_b[j]=di;light_specular_a[j]=di;
	      light_ambient_r[j]=0;light_ambient_g[j]=0;light_ambient_b[j]=0;light_ambient_a[j]=1;
	      light_spot_x[j]=0;light_spot_y[j]=0;light_spot_z[j]=-1;light_spot_w[j]=0;
	      light_spot_exponent[j]=0;light_spot_cutoff[j]=180;
	      light_0[j]=1;light_1[j]=0;light_2[j]=0;
	      vecteur & optv=*optvalue._VECTptr;
	      for (unsigned i=0;i<optv.size();++i){
		gen & optg = optv[i];
		if ( (optg.is_symb_of_sommet(at_equal) || optg.is_symb_of_sommet(at_same) )  && optg._SYMBptr->feuille.type==_VECT && g._SYMBptr->feuille._VECTptr->size()==2){
		  gen & optgname = optg._SYMBptr->feuille._VECTptr->front();
		  gen optgval = evalf_double(optg._SYMBptr->feuille._VECTptr->back(),1,contextptr);
		  bool vect4=optgval.type==_VECT && optgval._VECTptr->size()==4;
		  vecteur xyzw;
		  if (vect4)
		    xyzw=*optgval._VECTptr;
		  switch (optgname.val){
		  case _GL_AMBIENT:
		    light_ambient_r[j]=xyzw[0]._DOUBLE_val;
		    light_ambient_g[j]=xyzw[1]._DOUBLE_val;
		    light_ambient_b[j]=xyzw[2]._DOUBLE_val;
		    light_ambient_a[j]=xyzw[3]._DOUBLE_val;
		    break;
		  case _GL_SPECULAR:
		    light_specular_r[j]=xyzw[0]._DOUBLE_val;
		    light_specular_g[j]=xyzw[1]._DOUBLE_val;
		    light_specular_b[j]=xyzw[2]._DOUBLE_val;
		    light_specular_a[j]=xyzw[3]._DOUBLE_val;
		    break;
		  case _GL_DIFFUSE:
		    light_diffuse_r[j]=xyzw[0]._DOUBLE_val;
		    light_diffuse_g[j]=xyzw[1]._DOUBLE_val;
		    light_diffuse_b[j]=xyzw[2]._DOUBLE_val;
		    light_diffuse_a[j]=xyzw[3]._DOUBLE_val;
		    break;
		  case _GL_POSITION:
		    light_x[j]=xyzw[0]._DOUBLE_val;
		    light_y[j]=xyzw[1]._DOUBLE_val;
		    light_z[j]=xyzw[2]._DOUBLE_val;
		    light_w[j]=xyzw[3]._DOUBLE_val;
		    break;
		  case _GL_SPOT_DIRECTION:
		    light_spot_x[j]=xyzw[0]._DOUBLE_val;
		    light_spot_y[j]=xyzw[1]._DOUBLE_val;
		    light_spot_z[j]=xyzw[2]._DOUBLE_val;
		    light_spot_w[j]=xyzw[3]._DOUBLE_val;
		    break;
		  case _GL_SPOT_EXPONENT:
		    light_spot_exponent[j]=optgval._DOUBLE_val;
		    break;
		  case _GL_SPOT_CUTOFF:
		    light_spot_cutoff[j]=optgval._DOUBLE_val;
		    break;
		  case _GL_CONSTANT_ATTENUATION:
		    light_0[j]=optgval._DOUBLE_val;
		    break;
		  case _GL_LINEAR_ATTENUATION:
		    light_1[j]=optgval._DOUBLE_val;
		    break;
		  case _GL_QUADRATIC_ATTENUATION:
		    light_2[j]=optgval._DOUBLE_val;
		    break;
		  }
		}
		;
	      } // end for i
	    }
	  } // end opengl options
	}
      }
    }
    if (g.type==_VECT){
      const_iterateur it=g._VECTptr->begin(),itend=g._VECTptr->end();
      for (;it!=itend;++it)
	update_infos(*it,contextptr);
    }
  }

  void Graph2d3d::move_cfg(int i){
    if (history.empty()) return;
    int j=i+history_pos;
    int s=history.size();
    if (j>s) j=s;
    if (j<1) j=1;
    history_pos=j;
    window_xyz & h = history[j-1];
    window_xmin=h.xmin;
    window_xmax=h.xmax;
    window_ymin=h.ymin;
    window_ymax=h.ymax;
    window_zmin=h.zmin;
    window_zmax=h.zmax;
  }

  void Graph2d3d::push_cfg(){
    int s=history.size();
    if (history_pos<s && history_pos>=0){
      history.erase(history.begin()+history_pos,history.end());
    }
    history.push_back(window_xyz(window_xmin,window_xmax,window_ymin,window_ymax,window_zmin,window_zmax));
    history_pos=history.size();
  }

  void Graph2d3d::clear_cfg(){
    history_pos=0;
    history.clear();
  }

  void Graph2d3d::find_xyz(double i,double j,double k,double &x,double&y,double &z){
    x=i; y=j; z=k;
  }

  void Graph2d::find_xy(double i,double j,double & x,double & y) const {
    x=window_xmin+i*(window_xmax-window_xmin)/(w()-(show_axes?ylegende*labelsize():0));
    y=window_ymax-j*(window_ymax-window_ymin)/(h()-(show_axes?((title.empty()?1:2)*labelsize()):0));
  }

  void Graph2d::find_xyz(double i,double j,double k,double & x,double & y,double & z) {
    z=k;
    find_xy(i,j,x,y);
  }

  Graph2d3d * in_find_graph2d3d(Fl_Widget * wid){
    if (Graph2d3d * g=dynamic_cast<Graph2d3d *>(wid))
      return g;
    if (Fl_Group * gr =dynamic_cast<Fl_Group *>(wid)){
      // search in children
      int n=gr->children();
      for (int i=0;i<n;++i){
	if (Graph2d3d * res = in_find_graph2d3d(gr->child(i)))
	  return res;
      }
    }
    return 0;
  }

  Graph2d3d * find_graph2d3d(Fl_Widget * wid){
    if (!wid) return 0;
    if (Graph2d3d * res = dynamic_cast<Graph2d3d *>(Fl::focus()) )
      return res;
    if (Graph2d3d * res=in_find_graph2d3d(wid))
      return res;
    return find_graph2d3d(wid->parent());
  }

  static void cb_Graph2d3d_LaTeX_Preview(Fl_Menu_* m , void*) {
    const char * filename=find_graph2d3d(m)->latex(0);
    if (filename)
      xdvi(filename);
  }

  static void cb_Graph2d3d_LaTeX_Print(Fl_Menu_* m , void*) {
    const char * filename=find_graph2d3d(m)->latex(0);
    if (filename)
      dvips(filename);
  }

  static void cb_Graph2d3d_Print(Fl_Menu_* m , void*) {
    Graph2d3d * gr = find_graph2d3d(m);
    if (gr)
      widget_print(gr);
  }

  static void cb_Graph2d3d_Preview(Fl_Menu_* m , void*) {
    Graph2d3d * gr = find_graph2d3d(m);
    if (gr)
      widget_ps_print(gr,gr->title,true);
  }

  static void cb_Graph3dpng(Fl_Menu_* m , void*) {
    Graph2d3d * gr = find_graph2d3d(m);
    if (Graph3d * gr3 = dynamic_cast<Graph3d *>(gr)){
      char * filename = file_chooser(gettext("Export to PNG file"),"*.png","session.png");
      if(!filename) return;
      gr3->opengl2png(filename);
    }
  }

  static void cb_Graph2d3d_Autoscale(Fl_Menu_* m , void*) {
    Graph2d3d * gr = find_graph2d3d(m);
    if (gr)
      gr->autoscale(false);
  }

  static void cb_Graph2d3d_AutoscaleFull(Fl_Menu_* m , void*) {
    Graph2d3d * gr = find_graph2d3d(m);
    if (gr)
      gr->autoscale(true);
  }

  static void cb_Graph2d3d_Orthonormalize(Fl_Menu_* m , void*) {
    Graph2d3d * gr = find_graph2d3d(m);
    if (gr)
      gr->orthonormalize();
  }

  static void cb_Graph2d3d_Next(Fl_Menu_* m , void*) {
    Graph2d3d * gr = find_graph2d3d(m);
    if (gr)
      gr->move_cfg(1);
  }

  static void cb_Graph2d3d_Previous(Fl_Menu_* m , void*) {
    Graph2d3d * gr = find_graph2d3d(m);
    if (gr)
      gr->move_cfg(-1);
  }

  static void cb_Graph2d3d_Zoomout(Fl_Menu_* m , void*) {
    Graph2d3d * gr = find_graph2d3d(m);
    if (gr)
      gr->zoom(1.414);
  }

  static void cb_Graph2d3d_Zoomin(Fl_Menu_* m , void*) {
    Graph2d3d * gr = find_graph2d3d(m);
    if (gr)
      gr->zoom(0.707);
  }

  static void cb_Graph2d3d_Config(Fl_Menu_* m , void*) {
    Graph2d3d * gr = find_graph2d3d(m);
    if (gr)
      gr->config();
  }

  static void cb_Graph2d3d_Pause(Fl_Menu_* m , void*) {
    Graph2d3d * gr = find_graph2d3d(m);
    if (gr)
      gr->paused=true;
  }

  static void cb_Graph2d3d_Stop(Fl_Menu_* m , void*) {
    Graph2d3d * gr = find_graph2d3d(m);
    if (gr){
      gr->animation_dt=0;
      gr->animation_instructions_pos=0;
    }
  }

  static void cb_Graph2d3d_Restart(Fl_Menu_* m , void*) {
    Graph2d3d * gr = find_graph2d3d(m);
    if (gr)
      gr->paused=false;
  }

  static void cb_Graph2d3d_Faster(Fl_Menu_* m , void*) {
    Graph2d3d * gr = find_graph2d3d(m);
    if (gr){
      if (gr->animation_dt)
	gr->animation_dt /= 2;
      else
	gr->animation_dt = 0.2;
    }
  }

  static void cb_Graph2d3d_Slower(Fl_Menu_* m , void*) {
    Graph2d3d * gr = find_graph2d3d(m);
    if (gr){
      if (gr->animation_dt)
	gr->animation_dt *= 2;
      else
	gr->animation_dt = 0.2;
    }
  }

  static void cb_Graph2d3d_hide(Fl_Menu_* m , void*) {
    Graph2d3d * gr = find_graph2d3d(m);
    if (Graph3d * gr3 = dynamic_cast<Graph3d *>(gr)){
      gr3->below_depth_hidden=true;
      gr3->redraw();
    }
  }

  static void cb_Graph2d3d_show(Fl_Menu_* m , void*) {
    Graph2d3d * gr = find_graph2d3d(m);
    if (Graph3d * gr3 = dynamic_cast<Graph3d *>(gr)){
      gr3->below_depth_hidden=false;
      gr3->redraw();
    }
  }

  static void cb_Graph2d3d_startview(Fl_Menu_* m , void*) {
    Graph2d3d * gr = find_graph2d3d(m);
    if (Graph3d * gr3 = dynamic_cast<Graph3d *>(gr)){
      gr3->theta_x=-13;
      gr3->theta_y=-95;
      gr3->theta_z=-110; 
      gr3->q=euler_deg_to_quaternion_double(gr3->theta_z,gr3->theta_x,gr3->theta_y);
      gr3->redraw();
    }
  }

  static void cb_Graph2d3d_xview(Fl_Menu_* m , void*) {
    Graph2d3d * gr = find_graph2d3d(m);
    if (Graph3d * gr3 = dynamic_cast<Graph3d *>(gr)){
      gr3->theta_x=0;
      gr3->theta_y=-90;
      gr3->theta_z=-90;      
      gr3->q=euler_deg_to_quaternion_double(gr3->theta_z,gr3->theta_x,gr3->theta_y);
      gr3->redraw();
    }
  }

  static void cb_Graph2d3d_yview(Fl_Menu_* m , void*) {
    Graph2d3d * gr = find_graph2d3d(m);
    if (Graph3d * gr3 = dynamic_cast<Graph3d *>(gr)){
      gr3->theta_x=0;
      gr3->theta_y=-90;
      gr3->theta_z=0;      
      gr3->q=euler_deg_to_quaternion_double(gr3->theta_z,gr3->theta_x,gr3->theta_y);
      gr3->redraw();
    }
  }

  static void cb_Graph2d3d_zview(Fl_Menu_* m , void*) {
    Graph2d3d * gr = find_graph2d3d(m);
    if (Graph3d * gr3 = dynamic_cast<Graph3d *>(gr)){
      gr3->theta_x=0;
      gr3->theta_y=0;
      gr3->theta_z=0;      
      gr3->q=euler_deg_to_quaternion_double(gr3->theta_z,gr3->theta_x,gr3->theta_y);
      gr3->redraw();
    }
  }

  static void cb_Graph2d3d_mouse_plan(Fl_Menu_* m , void*) {
    if (!Fl::focus())
      return;
    Graph2d3d * gr = find_graph2d3d(m);
    if (Graph3d * gr3d = dynamic_cast<Graph3d *>(gr)){
      double a,b,c;
      gr3d->current_normal(a,b,c);
      gr3d->normal2plan(a,b,c); // divides a,b,c by dx^2,...
      double x0,y0,z0,t0;
      gr3d->find_xyz(gr3d->x()+gr3d->w()/2,gr3d->y()+gr3d->h()/2,gr3d->depth,x0,y0,z0);
      t0=a*x0+b*y0+c*z0;
      if (std::abs(t0)<std::abs(gr3d->window_zmax-gr3d->window_zmin)/1000)
	t0=0;
      string s="plan("+print_DOUBLE_(a)+"*x+"+print_DOUBLE_(b)+"*y+"+print_DOUBLE_(c)+"*z="+print_DOUBLE_(t0)+")";
      in_Xcas_input_char(Fl::focus(),s,' ');
    }
  }

  // image of (x,y,z) by rotation around axis r(rx,ry,rz) of angle theta
  void rotate(double rx,double ry,double rz,double theta,double x,double y,double z,double & X,double & Y,double & Z){
    /*
    quaternion_double q=rotation_2_quaternion_double(rx,ry,rz,theta);
    quaternion_double qx(x,y,z,0);
    quaternion_double qX=conj(q)*qx*q;
    */
    // r(rx,ry,rz) the axis, v(x,y,z) projects on w=a*r with a such that
    // w.r=a*r.r=v.r
    double r2=rx*rx+ry*ry+rz*rz;
    double r=std::sqrt(r2);
    double a=(rx*x+ry*y+rz*z)/r2;
    // v=w+V, w remains stable, V=v-w=v-a*r rotates
    // Rv=w+RV, where RV=cos(theta)*V+sin(theta)*(r cross V)/sqrt(r2)
    double Vx=x-a*rx,Vy=y-a*ry,Vz=z-a*rz;
    // cross product of k with V
    double kVx=ry*Vz-rz*Vy, kVy=rz*Vx-rx*Vz,kVz=rx*Vy-ry*Vx;
    double c=std::cos(theta),s=std::sin(theta);
    X=a*rx+c*Vx+s*kVx/r;
    Y=a*ry+c*Vy+s*kVy/r;
    Z=a*rz+c*Vz+s*kVz/r;
  }

  // type -1 = view point rotation around an axis, 
  // type bit 0 to 7: spot rotation
  // step is in degree; tstep in seconds
  // [0,0,0],[rx,ry,rz] axis of the rotation
  void oxyz_rotate(Graph2d3d* gr,int type,int nstep,double tstep,int danim,double rx,double ry,double rz){
    static Fl_Window * w = 0;
    static Fl_Button * button = 0;
    if (!w){
      Fl_Group::current(0);
      w=new Fl_Window(100,24);
      button = new Fl_Button(2,2,w->w()-4,w->h()-4);
      button->label(gettext("Cancel"));
      w->label(gettext("Rotate Animation"));
      w->end();
    }
    if (Graph3d * gr3 = dynamic_cast<Graph3d *>(gr)){
      w->set_modal();
      w->show();
      w->hotspot(w);
      Fl::focus(button);
      double step=2*M_PI/nstep,lx[8],ly[8],lz[8],lX,lY,lZ;
      quaternion_double qsave(gr3->q);
      for (int j=0;j<8;++j){
	lx[j]=gr3->light_x[j];
	ly[j]=gr3->light_y[j];
	lz[j]=gr3->light_z[j];
      }
      for (int i=1;i<nstep;++i){
	double theta=step*i;
	// type 0 to 7, rotate light by theta
	for (int j=0;j<8;++j){ 
	  if (type & (1<<j)){
	    rotate(rx,ry,rz,theta,lx[j],ly[j],lz[j],lX,lY,lZ);
	    // cerr << theta << ":" << lX << "," << lY << "," << lZ << endl;
	    gr3->light_x[j]=lX;
	    gr3->light_y[j]=lY;
	    gr3->light_z[j]=lZ;
	  }
	}
	if (type & (1<<8))
	  gr3->q=qsave*rotation_2_quaternion_double(rx,ry,rz,theta*180/M_PI);
	gr3->animation_instructions_pos+=danim;
	gr3->redraw();
	int nwait=int(tstep/0.001),jwait;
	if (nwait<=0)
	  nwait=1;
	for (jwait=0;jwait<nwait;++jwait){
	  Fl_Widget *o = Fl::readqueue();
	  if (o==button || o==w)
	    break;
	  else {
	    Fl::wait(0.0001);
	    usleep(1000);
	  }
	}
	if (jwait<nwait)
	  break;
      } // end animation for
      w->hide();
      for (int j=0;j<8;++j){
	gr3->light_x[j]=lx[j];
	gr3->light_y[j]=ly[j];
	gr3->light_z[j]=lz[j];
      }
      gr3->q=qsave;
      gr3->animation_instructions_pos+=danim;
      gr3->redraw();
    }
  }
    
  static void cb_Graph2d3d_rotate(Fl_Menu_* m , void*) { 
    if (!Fl::focus())
      return;
    Graph2d3d * gr = find_graph2d3d(m);
    // control window for args of oxyz_rotate
    oxyz_rotate(gr,gr->rotanim_type,gr->rotanim_nstep,gr->rotanim_tstep,gr->rotanim_danim,gr->rotanim_rx,gr->rotanim_ry,gr->rotanim_rz);
  }

  static void cb_Graph_Traceclear(Fl_Widget * m , void*) {
    Graph2d3d * gr = find_graph2d3d(m);
    if (gr){
      gr->trace_instructions.clear(); 
      gr->redraw();
    }
  }

  Figure * do_find_figure(Fl_Widget * widget){
    Fl_Widget * gr = widget;
    for (;gr;gr=gr->parent()){
      Figure * f =dynamic_cast<Figure *>(gr);
      if (f)
	return f;
    }
    return 0;
  }

  Figure * find_figure(Fl_Widget * widget){
    Figure * gr = do_find_figure(widget);
    if (gr)
      return gr;
    gr=do_find_figure(Fl::focus());
    //if (!gr)
    //  fl_alert("%s","No figure found. Please click in or add one");
    return gr;
  }  

  static void cb_Graph_Traceobject(Fl_Widget * m , void*) {
    Fl_Widget * wid=Fl::focus();
    Figure * f=find_figure(m);
    if (f){
      const char * ch=fl_input(gettext("Trace which object?"));
      if (ch){
	History_Pack * hp=f->geo->hp;
	if (!hp)
	  return;
	int pos;
	if (hp!=get_history_pack(wid,pos))
	  pos=hp->children()-1;
	hp->add_entry(pos);
	string arg="trace("+string(ch)+")";
	hp->set_value(pos,arg,true);     
      }
    }
  }

  static void cb_Graph_Traceon(Fl_Widget * m , void*) {
    Graph2d3d * gr = find_graph2d3d(m);
    if (gr){
      gr->display_mode |= 0x40; // set bit 6
      gr->redraw();
    }
  }

  static void cb_Graph_Traceoff(Fl_Widget * m , void*) {
    Graph2d3d * gr = find_graph2d3d(m);
    if (gr){
      gr->display_mode &= (0xffff ^ 0x40);; // clear bit 6
      gr->redraw();
    }
  }

  static void cb_Graph_Graphon(Fl_Widget * m , void*) {
    Graph2d3d * gr = find_graph2d3d(m);
    if (gr){
      gr->display_mode |= 0x1; // set bit 0
      gr->redraw();
    }
  }

  static void cb_Graph_Graphoff(Fl_Widget * m , void*) {
    Graph2d3d * gr = find_graph2d3d(m);
    if (gr){
      gr->display_mode &= (0xffff ^ 0x1); // clear bit 0
      gr->redraw();
    }
  }

  static void cb_Graph2d3d_Frameoff(Fl_Widget * m , void*) {
    Graph2d3d * gr = find_graph2d3d(m);
    if (gr){
      gr->display_mode |= 0x80; // set bit 6
      gr->redraw();
    }
  }

  static void cb_Graph2d3d_Frameon(Fl_Widget * m , void*) {
    Graph2d3d * gr = find_graph2d3d(m);
    if (gr){
      gr->display_mode &= (0xffff ^ 0x80);; // clear bit 6
      gr->redraw();
    }
  }

  static void cb_Graph_Animate(Fl_Widget * m , void*) {
    Graph2d3d * gr = find_graph2d3d(m);
    if (gr){
      const char * ch=fl_input(gettext("Frames number? (>0 forward, <0 forward+backward)"),"10");
      if (ch){
	int n=atoi(ch);
	gr->animation_instructions=gr->animate(n);
	if (gr->animation_instructions.empty())
	  fl_alert("%s",gettext("Figure must depend on a parameter (Edit->Add parameter)"));
	gr->paused=false;
	gr->animation_dt=0.1;
	gr->display_mode |= 0x2; // set bit 1
	gr->redraw();
      }
    }
  }

  static void cb_Graph_Unanimate(Fl_Widget * m , void*) {
    Graph2d3d * gr = find_graph2d3d(m);
    if (gr){
      gr->animation_instructions.clear();
      gr->paused=false;
      gr->animation_dt=0;
      gr->display_mode &= (0xffff ^ 0x2);; // clear bit 1
      gr->redraw();
    }
  }

  Fl_Menu_Item Autoscale_menu[] = {
    {gettext("auto"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("Autoscale"), 0,  (Fl_Callback*)cb_Graph2d3d_Autoscale, 0, 0, 0, 0, 14, 56},
    {gettext("Autoscale (full)"), 0,  (Fl_Callback*)cb_Graph2d3d_AutoscaleFull, 0, 0, 0, 0, 14, 56},
    {0},
    {0}
  };

  Fl_Menu_Item Graph2d3d_menu[] = {
    {gettext("M"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("View"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("Autoscale"), 0,  (Fl_Callback*)cb_Graph2d3d_Autoscale, 0, 0, 0, 0, 14, 56},
    {gettext("Autoscale (full)"), 0,  (Fl_Callback*)cb_Graph2d3d_AutoscaleFull, 0, 0, 0, 0, 14, 56},
    {gettext("Orthonormalize (2-d)"), 0,  (Fl_Callback*)cb_Graph2d3d_Orthonormalize, 0, 0, 0, 0, 14, 56},
    {gettext("Zoom in"), 0,  (Fl_Callback*)cb_Graph2d3d_Zoomin, 0, 0, 0, 0, 14, 56},
    {gettext("Zoom out"), 0,  (Fl_Callback*)cb_Graph2d3d_Zoomout, 0, 0, 0, 0, 14, 56},
    {gettext("Frame move on"), 0,  (Fl_Callback*)cb_Graph2d3d_Frameon, 0, 0, 0, 0, 14, 56},
    {gettext("Frame move off"), 0,  (Fl_Callback*)cb_Graph2d3d_Frameoff, 0, 0, 0, 0, 14, 56},
    {gettext("Previous"), 0,  (Fl_Callback*)cb_Graph2d3d_Previous, 0, 0, 0, 0, 14, 56},
    {gettext("Next"), 0,  (Fl_Callback*)cb_Graph2d3d_Next, 0, 0, 0, 0, 14, 56},
    {gettext("Config"), 0,  (Fl_Callback*)cb_Graph2d3d_Config, 0, 0, 0, 0, 14, 56},
    {0},
    {gettext("Trace"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("Trace clear"), 0,  (Fl_Callback *) cb_Graph_Traceclear, 0, 0, 0, 0, 14, 56},
    {gettext("Trace on"), 0,  (Fl_Callback *) cb_Graph_Traceon, 0, 0, 0, 0, 14, 56},
    {gettext("Trace off"), 0,  (Fl_Callback *) cb_Graph_Traceoff, 0, 0, 0, 0, 14, 56},
    {gettext("Trace object"), 0,  (Fl_Callback *) cb_Graph_Traceobject, 0, 0, 0, 0, 14, 56},
    {0},
    {gettext("Animation"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("Pause"), 0,  (Fl_Callback*)cb_Graph2d3d_Pause, 0, 0, 0, 0, 14, 56},
    {gettext("Stop"), 0,  (Fl_Callback*)cb_Graph2d3d_Stop, 0, 0, 0, 0, 14, 56},
    {gettext("Restart"), 0,  (Fl_Callback*)cb_Graph2d3d_Restart, 0, 0, 0, 0, 14, 56},
    {gettext("Faster (2x)"), 0,  (Fl_Callback*)cb_Graph2d3d_Faster, 0, 0, 0, 0, 14, 56},
    {gettext("Slower (2x)"), 0,  (Fl_Callback*)cb_Graph2d3d_Slower, 0, 0, 0, 0, 14, 56},
    {gettext("Build animation"), 0,  (Fl_Callback *) cb_Graph_Animate, 0, 0, 0, 0, 14, 56},
    {gettext("Unanimate"), 0,  (Fl_Callback *) cb_Graph_Unanimate, 0, 0, 0, 0, 14, 56},
    {gettext("Graph on"), 0,  (Fl_Callback *) cb_Graph_Graphon, 0, 0, 0, 0, 14, 56},
    {gettext("Graph off"), 0,  (Fl_Callback *) cb_Graph_Graphoff, 0, 0, 0, 0, 14, 56},
    {0},
    {gettext("3-d"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("Mouse plan equation"), 0,  (Fl_Callback*)cb_Graph2d3d_mouse_plan, 0, 0, 0, 0, 14, 56},
    {gettext("Oyz view (x=cst=depth)"), 0,  (Fl_Callback*)cb_Graph2d3d_xview, 0, 0, 0, 0, 14, 56},
    {gettext("Oxz view (y=cst=depth)"), 0,  (Fl_Callback*)cb_Graph2d3d_yview, 0, 0, 0, 0, 14, 56},
    {gettext("upper view (z=cst=depth)"), 0,  (Fl_Callback*)cb_Graph2d3d_zview, 0, 0, 0, 0, 14, 56},
    {gettext("default view"), 0,  (Fl_Callback*)cb_Graph2d3d_startview, 0, 0, 0, 0, 14, 56},
    {gettext("Rotate animation"), 0,  (Fl_Callback*)cb_Graph2d3d_rotate, 0, 0, 0, 0, 14, 56},
    {gettext("Hide below depth"), 0,  (Fl_Callback*)cb_Graph2d3d_hide, 0, 0, 0, 0, 14, 56},
    {gettext("Show below depth"), 0,  (Fl_Callback*)cb_Graph2d3d_show, 0, 0, 0, 0, 14, 56},
    {0}, // end 3-d
    {gettext("Export Print"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("EPS PNG and preview"), 0,  (Fl_Callback*)cb_Graph2d3d_Preview, 0, 0, 0, 0, 14, 56},
    {gettext("3-d to PNG"), 0,  (Fl_Callback*)cb_Graph3dpng, 0, 0, 0, 0, 14, 56},
    {gettext("Print"), 0,  (Fl_Callback*)cb_Graph2d3d_Print, 0, 0, 0, 0, 14, 56},
    {gettext("Preview (with Latex)"), 0,  (Fl_Callback*)cb_Graph2d3d_LaTeX_Preview, 0, 0, 0, 0, 14, 56},
    {gettext("Print (with Latex)"), 0,  (Fl_Callback*)cb_Graph2d3d_LaTeX_Print, 0, 0, 0, 0, 14, 56},
    {0},
    {0},
    {0}
  };

  void Graph2d3d::add_mouse_param_group(int x,int y,int w,int h){
    Fl_Group * group = parent();
    if (!group) return;
    bool is3d=dynamic_cast<Graph3d *>(this);
    Fl_Group::current(group);
    labelsize(group->labelsize());
    //    mouse_param_group=new Fl_Tile(x+w-legende_size,y,legende_size,h); 
    mouse_param_group=new Fl_Group(x+w-legende_size,y,legende_size,h); 
    mouse_param_group->box(FL_FLAT_BOX);
    mouse_position=new Mouse_Position(x+w-legende_size,y,legende_size,(is3d?3:2)*labelsize(),this);
    int bx=x+w-legende_size,by=y+mouse_position->h(),bw=legende_size/3,bh=min(group->labelsize()+3,h/8);
    button_group = new Fl_Group(bx,by,legende_size,5*bh);
    // Buttons
    Fl_Button * bzoomin = new Fl_Button(bx,by,bw,bh,"in");
    bzoomin->tooltip(gettext("Zoom in"));
    bzoomin->callback(cb_graph_buttons);
    Fl_Button * bup = new Fl_Button(bx+bw,by,bw,bh,"@-18");
    bup->color(y_axis_color);
    bup->tooltip(gettext("Increase y"));
    bup->callback(cb_graph_buttons);
    Fl_Button * bupright = new Fl_Button(bx+2*bw,by,bw,bh,"@-18");
    bupright->color(z_axis_color);
    bupright->tooltip(gettext("Increase z (3-d)/Zoom in for y (2-d)"));
    bupright->callback(cb_graph_buttons);
    by += bh;
    Fl_Button * bleft = new Fl_Button(bx,by,bw,bh,"@-1<-");
    bleft->when(FL_WHEN_RELEASE);
    bleft->color(x_axis_color);
    bleft->tooltip(gettext("Decrease x"));
    bleft->callback(cb_graph_buttons);
    Fl_Button * bortho = new Fl_Button(bx+bw,by,bw,bh,"_|_");
    bortho->tooltip(gettext(is3d?"Hide or show below depth":"Orthonormalize"));
    bortho->callback(cb_graph_buttons);
    Fl_Button * bright = new Fl_Button(bx+2*bw,by,bw,bh,"@-1->");
    bright->color(x_axis_color);
    bright->tooltip(gettext("Increase x"));
    bright->callback(cb_graph_buttons);
    by += bh;
    Fl_Button * bzoomout = new Fl_Button(bx,by,bw,bh,"out");
    bzoomout->tooltip(gettext("Zoom out"));
    bzoomout->callback(cb_graph_buttons);
    Fl_Button * bdown = new Fl_Button(bx+bw,by,bw,bh,"@-12");
    bdown->color(y_axis_color);
    bdown->tooltip(gettext("Decrease y"));
    bdown->callback(cb_graph_buttons);
    Fl_Button * bdownright = new Fl_Button(bx+2*bw,by,bw,bh,"@-12");
    bdownright->color(z_axis_color);
    bdownright->tooltip(gettext("Decrease z (3-d)/Zoom out in y (2-d)"));
    bdownright->callback(cb_graph_buttons);
    by += bh;
    Fl_Button * bback = new Fl_Button(bx,by,bw,bh,"@-1<-");
    bback->tooltip(gettext("Back in cfg history"));
    bback->callback(cb_graph_buttons);
    Fl_Button * bforw = new Fl_Button(bx+bw,by,bw,bh,"@-1->");
    bforw->tooltip(gettext("Forward in cfg history"));
    bforw->callback(cb_graph_buttons);
    Fl_Button * bcfg = new Fl_Button(bx+2*bw,by,bw,bh,"cfg");
    bcfg->tooltip(gettext("Configure screen view and axis"));
    bcfg->callback(cb_graph_buttons);
    by += bh;
    Fl_Button * bpause = new Fl_Button(bx+bw,by,bw,bh,"@>|");
    bpause->tooltip(gettext("Stop or restart animation"));
    bpause->callback(cb_graph_buttons);
    Fl_Menu_Bar * bauto = new Fl_Menu_Bar(bx+2*bw,by,bw,bh,"auto");
    bauto->tooltip(gettext("Autoscale"));
    int s= Autoscale_menu->size();
    Fl_Menu_Item * automenuitem = new Fl_Menu_Item[Autoscale_menu->size()];
    for (int i=0;i<s;++i)
      *(automenuitem+i)=*(Autoscale_menu+i);
    bauto->menu (automenuitem);    
    menubar= new Fl_Menu_Bar(bx,by,bw,bh,"+");
    menubar->tooltip(gettext("Graphic menu"));
    s= Graph2d3d_menu->size();
    Fl_Menu_Item * menuitem = new Fl_Menu_Item[Graph2d3d_menu->size()];
    for (int i=0;i<s;++i)
      *(menuitem+i)=*(Graph2d3d_menu+i);
    menubar->menu (menuitem);    
    button_group->end();
    int param_group_y=button_group->y()+button_group->h();
    param_group = new Fl_Group(x+w-legende_size,param_group_y,legende_size,mouse_param_group->h()-button_group->h()-mouse_position->h());
    param_group->box(FL_FLAT_BOX);
    param_group->end();
    mouse_param_group->end();
    group->add(mouse_param_group);  
    set_colors(group);
  }

  Graph2d3d::Graph2d3d(int x,int y,int w,int h,const char * l,double xmin,double xmax,double ymin,double ymax,double zmin,double zmax,double ortho,History_Pack * hp_):
    Fl_Widget(x,y,w,h,l),pushed(false),
    show_mouse_on_object(false),
    mode(255),args_tmp_push_size(0),no_handle(false),
    display_mode(0x45),
    window_xmin(xmin),window_xmax(xmax),window_ymin(ymin),window_ymax(ymax),window_zmin(zmin),window_zmax(zmax),history_pos(0),
    ylegende(2.5),
    mouse_param_group(0),mouse_position(0),param_group(0),button_group(0),menubar(0),hp(hp_),
#ifdef IPAQ
    npixels(15),
#else
    npixels(8),
#endif
    show_axes(hp_?giac::show_axes(hp_->contextptr):1),show_names(1),
    animation_dt(0),paused(false),animation_instructions_pos(0),
    rotanim_type(256),rotanim_danim(0),rotanim_nstep(100),rotanim_tstep(0.03),
    rotanim_rx(0),rotanim_ry(0),rotanim_rz(1),
    last_event(0),x_tick(1.0),y_tick(1.0),couleur(0),approx(true),hp_pos(-1),moving(false),moving_frame(false),ntheta(24),nphi(18),background_image(0) {
    animations.push_back(this);
    push_cfg();
    struct timezone tz;
    gettimeofday(&animation_last,&tz);
    legende_size=giac::LEGENDE_SIZE;
    x_axis_color=FL_RED;
    y_axis_color=dark_green_color;
    z_axis_color=FL_BLUE;
    current_i=current_j=RAND_MAX;
    in_area=false;
  }

  Graph2d3d::Graph2d3d(int x,int y,int w,int h,const char * l,History_Pack * hp_):
    Fl_Widget(x,y,w,h,l),pushed(false),
    show_mouse_on_object(false),
    display_mode(0x45),
    mode(255),args_tmp_push_size(0),no_handle(false),
    window_xmin(Xcas_config.window_xmin),window_xmax(Xcas_config.window_xmax),window_ymin(Xcas_config.window_ymin),window_ymax(Xcas_config.window_ymax),window_zmin(Xcas_config.window_zmin),window_zmax(Xcas_config.window_zmax),history_pos(0),
    ylegende(2.5),
    mouse_param_group(0),mouse_position(0),param_group(0),button_group(0),menubar(0),hp(hp_),
#ifdef IPAQ
    npixels(15),
#else
    npixels(8),
#endif
    show_axes(hp_?giac::show_axes(hp_->contextptr):1),show_names(1),
    animation_dt(0),paused(false),animation_instructions_pos(0),
    rotanim_type(256),rotanim_danim(0),rotanim_nstep(100),rotanim_tstep(0.1),
    rotanim_rx(0),rotanim_ry(0),rotanim_rz(1),
    last_event(0),x_tick(1.0),y_tick(1.0),couleur(0),approx(true),hp_pos(-1),moving(false),moving_frame(false),ntheta(24),nphi(18),background_image(0) { 
    legende_size=giac::LEGENDE_SIZE;
    animations.push_back(this);
    struct timezone tz;
    gettimeofday(&animation_last,&tz);
    push_cfg();
    x_axis_color=FL_RED;
    y_axis_color=dark_green_color;
    z_axis_color=FL_BLUE;
  }

  Graph2d3d::~Graph2d3d(){
    vector<Graph2d3d *>::iterator it=animations.begin(),itend=animations.end();
    for (;it!=itend;++it){
      if (*it==this){
	animations.erase(it);
	return;
      }
    }    
  }

  Graph2d::Graph2d(int x,int y,int w,int h,const char * l,History_Pack * hp_):
    Graph2d3d(x,y,w,h,l,Xcas_config.window_xmin,Xcas_config.window_xmax,Xcas_config.window_ymin,Xcas_config.window_ymax,Xcas_config.window_zmin,Xcas_config.window_zmax,1.0,hp_),
    orthonormal_factor(1.0),waiting_click(false)
  { 
    box(FL_FLAT_BOX);
    legende_size=max(min(legende_size,w/4),w/6);
    resize(x,y,w-legende_size,h);
    add_mouse_param_group(x,y,w,h);
    waiting_click_value=0;
  }

  double find_tick(double dx){
    double res=std::pow(10.0,std::floor(std::log10(std::abs(dx))));
    int nticks=int(dx/res);
    if (nticks<4)
      res/=5;
    else {
      if (nticks<8)
	res/=2;
    }
    return res;
  }

  std::string Graph2d3d::current_config(){
    string res="gl_quaternion=[";
    res += print_DOUBLE_(q.x);
    res += ",";
    res += print_DOUBLE_(q.y);
    res += ",";
    res += print_DOUBLE_(q.z);
    res += ",";
    res += print_DOUBLE_(q.w);
    res += "]";
    return res;
  }

  void Graph2d3d::reset_light(unsigned i){
    light_on[i]=!i;
    light_x[i]=0;light_y[i]=0;light_z[i]=1;light_w[i]=0;
    float di=i?0:1;
    light_diffuse_r[i]=di;light_diffuse_g[i]=di;light_diffuse_b[i]=di;light_diffuse_a[i]=di;
    light_specular_r[i]=di;light_specular_g[i]=di;light_specular_b[i]=di;light_specular_a[i]=di;
    light_ambient_r[i]=0;light_ambient_g[i]=0;light_ambient_b[i]=0;light_ambient_a[i]=1;
    light_spot_x[i]=0;light_spot_y[i]=0;light_spot_z[i]=-1;light_spot_w[i]=0;
    light_spot_exponent[i]=0;light_spot_cutoff[i]=180;
    light_0[i]=1;light_1[i]=0;light_2[i]=0;
  }

  void Graph2d3d::config_light(unsigned i){
    int dx=240,dy=300,l=14;
    if (window()){
      dx=int(0.7*window()->w());
      dy=int(0.5*window()->h());
      l=window()->labelsize();
    }
    static Fl_Window * w = 0;
    static Fl_Value_Input * ambient_r=0, * ambient_g=0, *ambient_b=0, *ambient_a=0, 
      * diffuse_r=0, * diffuse_g=0, *diffuse_b=0, * diffuse_a=0, 
      * specular_r=0, *specular_g=0, *specular_b=0, * specular_a=0,
      * pos_x=0, * pos_y=0, * pos_z=0, * pos_w=0,
      * dir_x=0, * dir_y=0, * dir_z=0, 
      * spot_exponent=0, * spot_cutoff = 0,
      * coeff_0 =0, * coeff_1 = 0, * coeff_2 = 0;
    static Fl_Check_Button * enabled = 0;
    static Fl_Button * button0 = 0 ; // ok
    static Fl_Button * button1 =0; // cancel, quit
    static Fl_Button * button2 =0; // reset light, stay
    static Fl_Button * button3 =0; // cancel changes, stay
    static Fl_Button * button4 =0; // apply, stay
    if (!w){
      Fl_Group::current(0);
      w=new Fl_Window(dx,dy);
      int dh=dy/7;
      int dw=dx/8;
      int y_=2;
      pos_x=new Fl_Value_Input(dw,y_,dw-2,dh-2,"x");
      pos_x->tooltip(gettext("x position of spot"));
      pos_y=new Fl_Value_Input(3*dw,y_,dw-2,dh-2,"y");
      pos_y->tooltip(gettext("y position of spot"));
      pos_z=new Fl_Value_Input(5*dw,y_,dw-2,dh-2,"z");
      pos_z->tooltip(gettext("z position of spot"));
      pos_w=new Fl_Value_Input(7*dw,y_,dw-2,dh-2,"w");
      pos_w->tooltip(gettext("0 directional light source (e.g. sun), 1 position light (e.g. bulb)"));
      y_ += dh;
      dir_x=new Fl_Value_Input(dw,y_,dw-2,dh-2,"x->");
      dir_x->tooltip(gettext("x direction of spot"));
      dir_y=new Fl_Value_Input(3*dw,y_,dw-2,dh-2,"y->");
      dir_y->tooltip(gettext("y direction of spot"));
      dir_z=new Fl_Value_Input(5*dw,y_,dw-2,dh-2,"z->");
      dir_z->tooltip(gettext("z direction of spot"));
      enabled = new Fl_Check_Button(6*dw,y_,2*dw-2,dh-2,"on");
      enabled->tooltip(gettext("Turn on/off light"));
      // spot_w=new Fl_Value_Input(3*dw,y_,dw-2,dh-2,"w");
      // spot_w->tooltip(gettext("0 directional or 1 for position"));
      y_ += dh;
      ambient_r=new Fl_Value_Input(dw,y_,dw-2,dh-2,"amb_r");
      ambient_r->tooltip(gettext("ambient red component"));
      ambient_g=new Fl_Value_Input(3*dw,y_,dw-2,dh-2,"amb_g");
      ambient_g->tooltip(gettext("ambient green component"));
      ambient_b=new Fl_Value_Input(5*dw,y_,dw-2,dh-2,"amb_b");
      ambient_b->tooltip(gettext("ambient blue component"));
      ambient_a=new Fl_Value_Input(7*dw,y_,dw-2,dh-2,"amb_a");
      ambient_a->tooltip(gettext("ambient alpha component"));
      y_ += dh;
      diffuse_r=new Fl_Value_Input(dw,y_,dw-2,dh-2,"diff_r");
      diffuse_r->tooltip(gettext("diffuse red component"));
      diffuse_g=new Fl_Value_Input(3*dw,y_,dw-2,dh-2,"diff_g");
      diffuse_g->tooltip(gettext("diffuse green component"));
      diffuse_b=new Fl_Value_Input(5*dw,y_,dw-2,dh-2,"diff_b");
      diffuse_b->tooltip(gettext("diffuse blue component"));
      diffuse_a=new Fl_Value_Input(7*dw,y_,dw-2,dh-2,"diff_a");
      diffuse_a->tooltip(gettext("diffuse alpha component"));
      y_ += dh;
      specular_r=new Fl_Value_Input(dw,y_,dw-2,dh-2,"spec_r");
      specular_r->tooltip(gettext("specular red component"));
      specular_g=new Fl_Value_Input(3*dw,y_,dw-2,dh-2,"spec_g");
      specular_g->tooltip(gettext("specular green component"));
      specular_b=new Fl_Value_Input(5*dw,y_,dw-2,dh-2,"spec_b");
      specular_b->tooltip(gettext("specular blue component"));
      specular_a=new Fl_Value_Input(7*dw,y_,dw-2,dh-2,"spec_a");
      specular_a->tooltip(gettext("specular alpha component"));
      y_ += dh;
      dw=dx/10;
      spot_exponent=new Fl_Value_Input(dw,y_,dw-2,dh-2,"exp");
      spot_exponent->tooltip(gettext("Spot attenuation exponent: power of cos(theta), e.g. 1"));
      spot_cutoff=new Fl_Value_Input(3*dw,y_,dw-2,dh-2,"cutoff");
      spot_cutoff->tooltip(gettext("Spot cutoff angle in 0..90 or 180 for no cutoff"));
      coeff_0=new Fl_Value_Input(5*dw,y_,dw-2,dh-2,"att0");
      coeff_0->tooltip(gettext("Attenuation constant coefficient"));
      coeff_1=new Fl_Value_Input(7*dw,y_,dw-2,dh-2,"att1");
      coeff_1->tooltip(gettext("Attenuation linear coefficient"));
      coeff_2=new Fl_Value_Input(9*dw,y_,dw-2,dh-2,"att1");
      coeff_2->tooltip(gettext("Attenuation quadratic coefficient"));
      y_ += dh;
      button0 = new Fl_Button(2,y_,dx/5-4,dh-4);
      button0->label(gettext("OK"));
      button1 = new Fl_Button(dx/5+2,y_,dx/5-4,dh-4);
      button1->label(gettext("Cancel"));
      button1->shortcut(0xff1b);
      button2 = new Fl_Button(2*dx/5+2,y_,dx/5-4,dh-4);
      button2->label(gettext("Default"));
      button3 = new Fl_Button(3*dx/5+2,y_,dx/5-4,dh-4);
      button3->label(gettext("Reset"));
      button4 = new Fl_Button(4*dx/5+2,y_,dx/5-4,dh-4);
      button4->label(gettext("Apply"));
    }
    w->label((gettext("Light configuration")+print_INT_(i)).c_str());
    change_group_fontsize(w,l);
    w->set_modal();
    w->show();
    autosave_disabled=true;
    w->hotspot(w);
    Fl::focus(pos_x);
    int r=-2;
    for (;;) {
      if (r==-2){
	diffuse_r->value(light_diffuse_r[i]);
	diffuse_g->value(light_diffuse_g[i]);
	diffuse_b->value(light_diffuse_b[i]);
	diffuse_a->value(light_diffuse_a[i]);
	ambient_r->value(light_ambient_r[i]);
	ambient_g->value(light_ambient_g[i]);
	ambient_b->value(light_ambient_b[i]);
	ambient_a->value(light_ambient_a[i]);
	specular_r->value(light_specular_r[i]);
	specular_g->value(light_specular_g[i]);
	specular_b->value(light_specular_b[i]);
	specular_a->value(light_specular_a[i]);
	pos_x->value(light_x[i]);
	pos_y->value(light_y[i]);
	pos_z->value(light_z[i]);
	pos_w->value(light_w[i]);
	dir_x->value(light_spot_x[i]);
	dir_y->value(light_spot_y[i]);
	dir_z->value(light_spot_z[i]);
	spot_exponent->value(light_spot_exponent[i]);
	spot_cutoff->value(light_spot_cutoff[i]);
	coeff_0->value(light_0[i]);
	coeff_1->value(light_1[i]);
	coeff_2->value(light_2[i]);
	enabled->value(light_on[i]);
      }
      r=-1;
      Fl_Widget *o = Fl::readqueue();
      if (!o) Fl::wait();
      else {
	if (o == button0) r = 0; // apply and quit
	if (o == button1) r = 1; // cancel changes, quit
	if (o == w) r=1; 
	if (o == button2){ // reset light
	  reset_light(i);
	  r=-2;
	}
	if (o == button3) r=-2; // cancel changes, stay heer
	if (o == button4 ) r=2; // apply, stay
      }
      if (r==0 || r==2 ){
	light_on[i]=enabled->value();
	light_diffuse_r[i]=diffuse_r->value();
	light_diffuse_g[i]=diffuse_g->value();
	light_diffuse_b[i]=diffuse_b->value();
	light_diffuse_a[i]=diffuse_a->value();
	light_ambient_r[i]=ambient_r->value();
	light_ambient_g[i]=ambient_g->value();
	light_ambient_b[i]=ambient_b->value();
	light_ambient_a[i]=ambient_a->value();
	light_specular_r[i]=specular_r->value();
	light_specular_g[i]=specular_g->value();
	light_specular_b[i]=specular_b->value();
	light_specular_a[i]=specular_a->value();
	light_x[i]=pos_x->value();
	light_y[i]=pos_y->value();
	light_z[i]=pos_z->value();
	light_w[i]=pos_w->value();
	light_spot_x[i]=dir_x->value();
	light_spot_y[i]=dir_y->value();
	light_spot_z[i]=dir_z->value();
	light_spot_exponent[i]=spot_exponent->value();
	light_spot_cutoff[i]=spot_cutoff->value();
	light_0[i]=coeff_0->value();
	light_1[i]=coeff_1->value();
	light_2[i]=coeff_2->value();
	redraw();
      }
      if (r==0 || r==1)
	break;
    }
    autosave_disabled=false;
    w->hide();
  }
  
  // round to 3 decimals
  double setup_round(double x){
    if (x<0)
      return -setup_round(-x);
    if (x<1e-300)
      return 0;
    int n=int(floor(log10(x)+.5)); // round to nearest
    x=int(floor(x*pow(10.0,3.0-n)+.5));
    x=x*pow(10.0,n-3.0);
    return x;
  }

  void Graph2d3d::config(){
    int dx=240,dy=300,l=14;
    if (window()){
      dx=int(0.7*window()->w());
      dy=int(0.8*window()->h());
      l=window()->labelsize();
    }
    static Fl_Window * w = 0;
    static Fl_Value_Input * wxmin=0, * wxmax=0, *wymin=0, *wymax=0, *wzmin=0, *wzmax=0,*tx=0,*ty=0,*nx=0,*ny=0,*nz=0,*nd=0,*animate=0,*ylegendesize=0,*rx=0,*Theta=0,*Phi=0;
    static Fl_Value_Input * rotcfg_type=0,*rotcfg_danim=0,*rotcfg_nstep=0,*rotcfg_rx=0,*rotcfg_ry=0,*rotcfg_rz=0,*rotcfg_tstep=0;
    static Fl_Input * autoname_input=0;
    static Fl_Button * button0 = 0 ; // ok
    static Fl_Button * button1 =0; // cancel
    static Fl_Button * button2 =0; // apply
    static Fl_Button * button3 =0; // reset
    static Fl_Button * button4 =0; // autoscale
    static Fl_Button * button5 =0; // round
    static Fl_Button * l0=0,*l1=0,*l2=0,*l3=0,*l4=0,*l5=0,*l6=0,*l7=0; // lights
    static Fl_Check_Button* c1=0,*c2=0,*c3=0,*ct=0,*landscape=0,*notperspective=0,*lights=0,*shade=0,*blend=0,*fbox=0,*triedre=0,*logx=0,*logy=0;
    static Fl_Multiline_Output * currentcfg = 0; // display
    if (dy<240)
      dy=240;
    if (dx<240)
      dx=240;
    Graph3d * gr3d = dynamic_cast<Graph3d *>(this);
    Graph2d * gr2d = dynamic_cast<Graph2d *>(this);
    if (!w){
      Fl_Group::current(0);
      w=new Fl_Window(dx,dy);
      int dh=dy/12;
      int y_=2;
      wxmin=new Fl_Value_Input(dx/6,y_,dx/6-2,dh-4,"WX-");
      wxmin->tooltip(gettext("Xmin for visualisation"));
      wxmin->labelcolor(x_axis_color);
      wxmax=new Fl_Value_Input(dx/2,y_,dx/6-2,dh-4,"WX+");
      wxmax->tooltip(gettext("Xmax for visualisation"));
      wxmax->labelcolor(x_axis_color);
      logx=new Fl_Check_Button(5*dx/6,y_,dx/6-2,dh-4,"X-log");
      logx->tooltip(gettext("X Logarithmic scale"));
      logx->value(0);
      notperspective=new Fl_Check_Button(5*dx/6,y_,dx/6-2,dh-4,"Ortho proj");
      notperspective->tooltip(gettext("Orthonormal or perspective"));
      notperspective->value(0);
      y_ += dh;
      wymin=new Fl_Value_Input(dx/6,y_,dx/6-2,dh-4,"WY-");
      wymin->tooltip(gettext("Ymin for visualisation"));
      wymin->labelcolor(y_axis_color);
      wymax=new Fl_Value_Input(dx/2,y_,dx/6-2,dh-4,"WY+");
      wymax->tooltip(gettext("Ymax for visualisation"));
      wymax->labelcolor(y_axis_color);
      logy=new Fl_Check_Button(5*dx/6,y_,dx/6-2,dh-4,"Y-log");
      logy->tooltip(gettext("Y Logarithmic scale"));
      logy->value(0);
      shade=new Fl_Check_Button(5*dx/6,y_,dx/6-2,dh-4,"Gouraud");
      shade->tooltip(gettext("Flat or Gouraud shading"));
      shade->value(0);
      y_ += dh;
      wzmin=new Fl_Value_Input(dx/6,y_,dx/6-2,dh-4,"WZ-");
      wzmin->tooltip(gettext("Zmin for visualisation"));
      wzmin->labelcolor(z_axis_color);
      wzmax=new Fl_Value_Input(dx/2,y_,dx/6-2,dh-4,"WZ+");
      wzmax->tooltip(gettext("Zmax for visualisation"));
      wzmax->labelcolor(z_axis_color);
      blend=new Fl_Check_Button(5*dx/6,y_,dx/6-2,dh-4,"Blend");
      blend->tooltip(gettext("Enable blending"));
      blend->value(1);
      y_ += dh;
      wxmin->minimum(-1e300);
      wxmin->maximum(1e300);
      wymin->minimum(-1e300);
      wymin->maximum(1e300);
      wzmin->minimum(-1e300);
      wzmin->maximum(1e300);
      fbox=new Fl_Check_Button(2,y_,dx/6-4,dh-4,"Framebox");
      fbox->tooltip(gettext("Graphic displayed with a framebox"));
      fbox->value(1);
      ct=new Fl_Check_Button(dx/6+2,y_,dx/6-4,dh-4,"Pixels");
      ct->tooltip(gettext("Ticks in pixels or units"));
      ct->value(0);
      triedre=new Fl_Check_Button(2*dx/6+2,y_,dx/6-4,dh-4,"Triedre");
      triedre->tooltip(gettext("View triedre even if axes are hidden"));
      triedre->value(1);
      tx=new Fl_Value_Input(2*dx/4+dx/8,y_,dx/4-dx/8,dh-4,"X-tick");
      tx->tooltip(gettext("Number of units or pixels for ticks on X"));
      ty=new Fl_Value_Input(3*dx/4+dx/8,y_,dx/4-dx/8,dh-4,"Y-tick");
      ty->tooltip(gettext("Number of units or pixels for ticks on Y"));
      y_ += dh;
      nx=new Fl_Value_Input(30,y_,dx/7-30,dh-4,"x*");
      nx->tooltip(gettext("x coefficient of mouse plan equation"));
      nx->step(0.1);
      nx->minimum(-10.1);
      nx->maximum(10.1);
      ny=new Fl_Value_Input(dx/7+30,y_,dx/7-30,dh-4,"+y*");
      ny->tooltip(gettext("y coefficient of mouse plan equation"));
      ny->step(0.1);
      ny->minimum(-10.1);
      ny->maximum(10.1);
      nz=new Fl_Value_Input(2*dx/7+30,y_,dx/7-30,dh-4,"+z*");
      nz->tooltip(gettext("z coefficient of mouse plan equation"));
      nz->step(0.1);
      nz->minimum(-10.1);
      nz->maximum(10.1);
      nd=new Fl_Value_Input(3*dx/7+30,y_,dx/7-30,dh-4,"=");
      nd->tooltip(gettext("constant coefficient of mouse plan equation"));
      nd->step(0.1);
      nd->minimum(-30.1);
      nd->maximum(30.1);
      rx=new Fl_Value_Input(4*dx/7+30,y_,dx/7-30,dh-4,"r.x");
      rx->tooltip(gettext("3d rotation"));
      rx->step(1);
      rx->minimum(-360);
      rx->maximum(360);
      Theta=new Fl_Value_Input(5*dx/7+30,y_,dx/7-30,dh-4,"lat");
      Theta->tooltip(gettext("Number of latitude divisions for sphere drawing"));
      Theta->step(1);
      Theta->minimum(5);
      Theta->maximum(3000);
      Phi=new Fl_Value_Input(6*dx/7+30,y_,dx/7-30,dh-4,"long");
      Phi->tooltip(gettext("Number of longitude divisions for sphere drawing"));
      Phi->step(1);
      Phi->minimum(5);
      Phi->maximum(3000);
      ylegendesize=new Fl_Value_Input(3*dx/5+30,y_,dx/5-30,dh-4,"y legende size");
      ylegendesize->tooltip(gettext("Size in pixels for y graduation printing"));
      ylegendesize->step(1);
      ylegendesize->minimum(1);
      ylegendesize->maximum(200);
      y_ += dh;
      l0=new Fl_Button(2,y_,dx/10-4,dh-4,"L0");
      l0->tooltip(gettext("Configure light"));
      l1=new Fl_Button(dx/10+2,y_,dx/10-4,dh-4,"L1");
      l1->tooltip(gettext("Configure light"));
      l2=new Fl_Button(dx/5+2,y_,dx/10-4,dh-4,"L2");
      l2->tooltip(gettext("Configure light"));
      l3=new Fl_Button(3*dx/10+2,y_,dx/10-4,dh-4,"L3");
      l3->tooltip(gettext("Configure light"));
      l4=new Fl_Button(2*dx/5+2,y_,dx/10-4,dh-4,"L4");
      l4->tooltip(gettext("Configure light"));
      l5=new Fl_Button(dx/2+2,y_,dx/10-4,dh-4,"L5");
      l5->tooltip(gettext("Configure light"));
      l6=new Fl_Button(3*dx/5+2,y_,dx/10-4,dh-4,"L6");
      l6->tooltip(gettext("Configure light"));
      l7=new Fl_Button(7*dx/10+2,y_,dx/10-4,dh-4,"L7");
      l7->tooltip(gettext("Configure light"));
      lights=new Fl_Check_Button(8*dx/10,y_,dx/5-2,dh-4,"Lights");
      lights->tooltip(gettext("Show scene with lights"));
      lights->value(0);
      y_ += dh;
      c1=new Fl_Check_Button(0,y_,dx/8,dh-4,gettext("Show names"));
      c1->tooltip(gettext("Show/Hide names of geometric objects"));
      c1->down_box(FL_DOWN_BOX);
      c3=new Fl_Check_Button(2*dx/8,y_,dx/8,dh-4,gettext("Plot names"));
      c3->tooltip(gettext("Show/Hide names of parametric and function plots"));
      c3->down_box(FL_DOWN_BOX);
      landscape=new Fl_Check_Button(dx/2,y_,dx/8,dh-4,gettext("Portrait"));
      landscape->tooltip(gettext("Portrait or landscape split"));
      landscape->down_box(FL_DOWN_BOX);
      autoname_input = new Fl_Input(dx/2+3*dx/8,y_,dx/8,dh-4);
      autoname_input->label(gettext("Autoname"));
      y_ += dh;
      c2=new Fl_Check_Button(0,y_,dx/6,dh-4,gettext("Show axis"));
      c2->tooltip(gettext("Show or hide axis"));
      c2->down_box(FL_DOWN_BOX);
      animate=new Fl_Value_Input(dx/2,y_,dx/6-4,dh-4,"animate");
      animate->tooltip(gettext("Time between image changes for animations"));
      animate->step(0.1);
      animate->minimum(0);
      animate->maximum(2);
      button5 = new Fl_Button(2*dx/3+2,y_,dx/3-4,dh-4);
      button5->label(gettext("Round"));
      button5->tooltip(gettext("Round xmin/xmax/ymin/ymax/zmin/zmax"));
      y_ += dh;
      button3 = new Fl_Button(2,y_,dx/3-4,dh-4);
      button3->label(gettext("Default"));
      button4 = new Fl_Button(dx/3+2,y_,dx/3-4,dh-4);
      button4->label(gettext("Autoscale"));
      button2 = new Fl_Button(2*dx/3+2,y_,dx/3-4,dh-4);
      button2->label(gettext("Apply"));
      button2->shortcut(0xff0d);
      y_ += dh;
      currentcfg = new Fl_Multiline_Output(dx/6,y_,5*dx/6-4,dh-4);
      currentcfg->label(gettext("Cfg"));
      currentcfg->tooltip(gettext("Current gl_quaternion= value"));
      y_ += dh;
      rotcfg_type = new Fl_Value_Input(dx/14+dx/42,y_,dx/14,dh-4);
      rotcfg_type->label("Anim");
      rotcfg_type->tooltip(gettext("Rotate what: 1 l1, 2 l2, 4 l3, ..., 128 l8, 256 viewpoint"));
      rotcfg_tstep=new Fl_Value_Input(8*dx/42,y_,dx/14,dh-4);
      rotcfg_tstep->label("t");
      rotcfg_tstep->tooltip(gettext("Time for 1 step in s"));
      rotcfg_nstep=new Fl_Value_Input(7*dx/21,y_,dx/14,dh-4);
      rotcfg_nstep->label("n");
      rotcfg_nstep->tooltip(gettext("Number of steps for animation"));
      rotcfg_rx=new Fl_Value_Input(10*dx/21,y_,dx/14,dh-4);
      rotcfg_rx->label("x");
      rotcfg_rx->tooltip(gettext("x rotation axis coordinate"));
      rotcfg_ry=new Fl_Value_Input(13*dx/21,y_,dx/14,dh-4);
      rotcfg_ry->label("y");
      rotcfg_ry->tooltip(gettext("y rotation axis coordinate"));
      rotcfg_rz=new Fl_Value_Input(16*dx/21,y_,dx/14,dh-4);
      rotcfg_rz->label("z");
      rotcfg_rz->tooltip(gettext("z rotation axis coordinate"));
      rotcfg_danim=new Fl_Value_Input(19*dx/21,y_,dx/14,dh-4);
      rotcfg_danim->label("d");
      rotcfg_danim->tooltip(gettext("Other animation change (normally 0)"));
      y_ += dh;
      button0 = new Fl_Button(2,y_,dx/2-4,dh-4);
      button0->label(gettext("OK"));
      button1 = new Fl_Button(dx/2+2,y_,dx/2-4,dh-4);
      button1->label(gettext("Cancel"));
      button1->shortcut(0xff1b);
      w->end();
      w->resizable(w);
    }
    w->label(gettext("Graph configuration"));
    change_group_fontsize(w,l);
    if (window_xmin>window_xmax)
      std::swap(window_xmin,window_xmax);
    if (window_ymin>window_ymax)
      std::swap(window_ymin,window_ymax);
    if (window_zmin>window_zmax)
      std::swap(window_zmin,window_zmax);
    Figure * fig=dynamic_cast<Figure *>(parent());
    if (fig){
      // landscape->show();
      landscape->value(fig->disposition);
    }
    else
      landscape->hide();
    window_xmin=setup_round(window_xmin);
    window_xmax=setup_round(window_xmax);
    window_ymin=setup_round(window_ymin);
    window_ymax=setup_round(window_ymax);
    window_zmin=setup_round(window_zmin);
    window_zmax=setup_round(window_zmax);
    double dtmp=(window_xmax-window_xmin)/100;
    wxmin->value(window_xmin);
    wxmin->step(dtmp);
    wxmax->value(window_xmax);
    wxmax->step(dtmp);
    wxmin->minimum(window_xmin-100000*dtmp);
    wxmin->maximum(window_xmax+100000*dtmp);
    wxmax->minimum(window_xmin-100000*dtmp);
    wxmax->maximum(window_xmax+100000*dtmp);
    dtmp=(window_ymax-window_ymin)/100;
    wymin->step(dtmp);
    wymax->step(dtmp);
    wymin->value(window_ymin);
    wymax->value(window_ymax);
    wymin->minimum(window_ymin-100000*dtmp);
    wymin->maximum(window_ymax+100000*dtmp);
    wymax->minimum(window_ymin-100000*dtmp);
    wymax->maximum(window_ymax+100000*dtmp);
    dtmp=(window_zmax-window_zmin)/100;
    wzmin->step(dtmp);
    wzmax->step(dtmp);
    wzmin->value(window_zmin);
    wzmax->value(window_zmax);
    wzmin->minimum(window_zmin-100000*dtmp);
    wzmin->maximum(window_zmax+100000*dtmp);
    wzmax->minimum(window_zmin-100000*dtmp);
    wzmax->maximum(window_zmax+100000*dtmp);
    animate->value(animation_dt);
    double a,b,c,i,j,theta,wx=(window_xmax-window_xmin),wy=(window_ymax-window_ymin),wz=(window_zmax-window_zmin);
    if (gr3d){
      rotcfg_tstep->show(); rotcfg_nstep->show();
      rotcfg_rx->show(); rotcfg_ry->show(); rotcfg_rz->show();
      rotcfg_danim->show(); rotcfg_type->show();
      rotcfg_tstep->value(rotanim_tstep);
      rotcfg_nstep->value(rotanim_nstep);
      rotcfg_danim->value(rotanim_danim);
      rotcfg_type->value(rotanim_type);
      rotcfg_rx->value(rotanim_rx);
      rotcfg_ry->value(rotanim_ry);
      rotcfg_rz->value(rotanim_rz);
      notperspective->show();
      logx->hide();
      logy->hide();
      notperspective->value(display_mode & 0x4);
      lights->show();
      lights->value(display_mode & 0x8);
      l0->show();
      l1->show();
      l2->show();
      l3->show();
      l4->show();
      l5->show();
      l6->show();
      l7->show();
      shade->show();
      shade->value( !(display_mode & 0x10) );
      blend->show();
      blend->value( (display_mode & 0x20) );
      gr3d->current_normal(a,b,c);
      gr3d->normal2plan(a,b,c); // divides a,b,c by dx^2,...
      nx->value(a);
      ny->value(b);
      nz->value(c);
      double x0,y0,z0,t0;
      find_xyz(gr3d->x()+gr3d->w()/2,gr3d->y()+gr3d->h()/2,gr3d->depth,x0,y0,z0);
      t0=a*x0+b*y0+c*z0;
      if (std::abs(t0)<std::abs(window_zmax-window_zmin)/1000)
	t0=0;
      nd->value(t0);
      // angle
      double res1[4],vect[4];
      if (std::abs(b)>std::abs(c)){
	res1[0]=b*wx;
	res1[1]=-a*wy;
	res1[2]=0;
      }
      else {
	res1[0]=c*wx;
	res1[1]=0;
	res1[2]=-a*wz;
      }
      res1[0] += (window_xmax+window_xmin)/2;
      res1[1] += (window_ymax+window_ymin)/2;
      res1[2] += (window_zmax+window_zmin)/2;
      res1[3]=1;
      mult4(gr3d->model,res1,vect);
      i=vect[0]/vect[3];
      j=vect[1]/vect[3];
      // (i,j) are the coordinates of OX0 in (OX,OY)
      theta=-std::atan2(j,i)*180/M_PI;
      rx->value(theta);
      nx->show();
      ny->show();
      nz->show();
      nd->show();
      rx->show();
      Theta->value(ntheta);
      Theta->show();
      Phi->value(nphi);
      Phi->show();
      ylegendesize->hide();
    }
    else {
      rotcfg_tstep->hide(); rotcfg_nstep->hide();
      rotcfg_rx->hide(); rotcfg_ry->hide(); rotcfg_rz->hide();
      rotcfg_danim->hide(); rotcfg_type->hide();
      notperspective->hide();
      logx->show();
      logy->show();
      logx->value(display_mode & (1<<10));
      logy->value(display_mode & (1<<11));
      lights->hide();
      l0->hide();
      l1->hide();
      l2->hide();
      l3->hide();
      l4->hide();
      l5->hide();
      l6->hide();
      l7->hide();
      shade->hide();
      nx->hide();
      ny->hide();
      nz->hide();
      nd->hide();
      rx->hide();
      Theta->hide();
      Phi->hide();
      ylegendesize->value(ylegende*labelsize());
      ylegendesize->show();
    }
    double y_scale=(Graph2d3d::h()-((show_axes && gr2d)?((title.empty()?1:2)*labelsize()):0))/(window_ymax-window_ymin);
    double x_scale=(Graph2d3d::w()-ylegende*((show_axes && gr2d)?labelsize():0))/(window_xmax-window_xmin);
    if (ct->value()){
      tx->value(x_tick*x_scale);
      ty->value(y_tick*y_scale);
    }
    else {
      tx->value(x_tick);
      ty->value(y_tick);
    }
    fbox->value((display_mode & 0x100));
    triedre->value(display_mode & 0x200);
    if (!hp)
      hp=get_history_pack(this);
    if (hp)
      autoname_input->value(autoname(hp->contextptr).c_str());
    c1->value(show_names & 1);
    c3->value(show_names & 2);
    c2->value(show_axes);
    currentcfg->value(current_config().c_str());
    w->set_modal();
    w->show();
    autosave_disabled=true;
    w->hotspot(w);
    Fl::focus(wxmin);
    int r;
    for (;;) {
      r=-1;
      Fl_Widget *o = Fl::readqueue();
      if (!o) Fl::wait();
      else {
	if (o == button0) r = 0;
	if (o == button1) r = 1;
	if (o == button2) r = 2;
	if (o==notperspective){
	  display_mode &= (0xffff ^ 0x4);
	  if (notperspective->value())
	    display_mode |= 0x4;
	  redraw();
	}
	if (o==lights){
	  display_mode &= (0xffff ^ 0x8);
	  if (lights->value())
	    display_mode |= 0x8;
	  redraw();
	}
	if (o==shade){
	  display_mode &= (0xffff ^ 0x10);
	  if (!shade->value())
	    display_mode |= 0x10;	  
	  redraw();
	}
	if (o==blend){
	  display_mode &= (0xffff ^ 0x20);
	  if (blend->value())
	    display_mode |= 0x20;	  
	  redraw();
	}
	if (o==logx){
	  display_mode &= (0xffff ^ (1<<10));
	  if (logx->value())
	    display_mode |= (1<<10);	  
	  redraw();
	}
	if (o==logy){
	  display_mode &= (0xffff ^ (1<<11));
	  if (logy->value())
	    display_mode |= (1<<11);	  
	  redraw();
	}
	if (o==l0){
	  config_light(0);
	}
	if (o==l1){
	  config_light(1);
	  continue;
	}
	if (o==l2){
	  config_light(2);
	  continue;
	}
	if (o==l3){
	  config_light(3);
	  continue;
	}
	if (o==l4){
	  config_light(4);
	  continue;
	}
	if (o==l5){
	  config_light(5);
	  continue;
	}
	if (o==l6){
	  config_light(6);
	  continue;
	}
	if (o==l7){
	  config_light(7);
	  continue;
	}
	if (fig && o == landscape){
	  fig->disposition=landscape->value();
	  fig->resize(fig->x(),fig->y(),fig->w(),fig->h());
	  orthonormalize();
	  fig->redraw();
	}
	if (o==button3 ){
	  wxmin->value(gnuplot_xmin);
	  wxmax->value(gnuplot_xmax);
	  wymin->value(gnuplot_ymin);
	  wymax->value(gnuplot_ymax);
	  wzmin->value(gnuplot_zmin);
	  wzmax->value(gnuplot_zmax);
	  nx->value(0);
	  ny->value(0);
	  nz->value(1);
	  nd->value(0);
	  r=2;
	}
	if (o==ylegendesize){
	  ylegende=ylegendesize->value()/labelsize();
	}
	if (o==button4 ){
	  autoscale(false);
	  wxmin->value(window_xmin);
	  wxmax->value(window_xmax);
	  wymin->value(window_ymin);
	  wymax->value(window_ymax);
	  wzmin->value(window_zmin);
	  wzmax->value(window_zmax);
	}
	if (o==button5){
	  x_tick=find_tick(window_xmax-window_xmin);
	  wxmin->value(int( window_xmin/x_tick -1)*x_tick);
	  wxmax->value(int( window_xmax/x_tick +1)*x_tick);
	  y_tick=find_tick(window_ymax-window_ymin);
	  wymin->value(int( window_ymin/y_tick -1)*y_tick);
	  wymax->value(int( window_ymax/y_tick +1)*y_tick);
	  z_tick=find_tick(window_zmax-window_zmin);
	  wzmin->value(int( window_zmin/z_tick -1)*z_tick);
	  wzmax->value(int( window_zmax/z_tick +1)*z_tick);
	}
	if (o == w) r=1; 
	if (r==0 || r==2){
	  rotanim_type=int(rotcfg_type->value());
	  rotanim_danim=int(rotcfg_danim->value());
	  rotanim_tstep=std::abs(rotcfg_tstep->value());
	  rotanim_nstep=max(int(rotcfg_nstep->value()),2);
	  rotanim_rx=rotcfg_rx->value();
	  rotanim_ry=rotcfg_ry->value();
	  rotanim_rz=rotcfg_rz->value();
	  if (rotanim_rx*rotanim_rx+rotanim_ry*rotanim_ry+rotanim_rz*rotanim_rz<1e-6){
	    rotanim_rx=0; rotanim_ry=0; rotanim_rz=1;
	  }
	  animation_dt=std::abs(animate->value());
	  animation_instructions_pos=0;
	  window_xmin=wxmin->value();
	  window_xmax=wxmax->value();
	  window_ymin=wymin->value();
	  window_ymax=wymax->value();
	  window_zmin=wzmin->value();
	  window_zmax=wzmax->value();
	  if (window_xmin>window_xmax)
	    std::swap(window_xmin,window_xmax);
	  if (window_ymin>window_ymax)
	    std::swap(window_ymin,window_ymax);
	  if (window_zmin>window_zmax)
	    std::swap(window_zmin,window_zmax);
	  y_scale=(Graph2d3d::h()-((gr2d && show_axes)?((title.empty()?1:2)*labelsize()):0))/(window_ymax-window_ymin);
	  x_scale=(Graph2d3d::w()-ylegende*((gr2d && show_axes)?labelsize():0))/(window_xmax-window_xmin);
	  if (gr3d){
	    ntheta=int(Theta->value());
	    nphi=int(Phi->value());
	    double a=nx->value(),b=ny->value(),c=nz->value();
	    double aorig=a,borig=b,corig=c;
	    // de-scale a,b,c
	    a *= (window_xmax-window_xmin);
	    b *= (window_ymax-window_ymin);
	    c *= (window_zmax-window_zmin);
	    double n=std::sqrt(a*a+b*b+c*c);
	    if (aorig*aorig+borig*borig+corig*corig<1e-3){
	      q=quaternion_double(1,0,0,0);
	      gr3d->depth=nd->value();
	    }
	    else {
	      a=a/n; b=b/n; c=c/n; // coordinates of OZ
	      // (0,0,depth) -> depth*(a,b,c)
	      // rescale (x,y,z)=depth*(dx*a,dy*b,dz*c)+(tx,ty,tz)
	      // A*x+B*y+C*z=d -> depth=(d-A*tx-B*ty-C*tz)/(A*Zx+B*Zy+C*Zz)
	      n=aorig*(window_xmax-window_xmin)*a+borig*(window_ymax-window_ymin)*b+corig*(window_zmax-window_zmin)*c;
	      gr3d->depth = -2/std::sqrt(double(3.0))*(nd->value()-aorig*(window_xmax+window_xmin)/2-borig*(window_ymax+window_ymin)/2-corig*(window_zmax+window_zmin)/2)/n;
	      // cerr << gr3d->depth << endl;
	      double A0,B0,C0; // coordinates of OX0
	      if (std::abs(b)>std::abs(c)){
		A0=b;
		B0=-a;
		C0=0;
	      }
	      else {
		A0=c;
		B0=0;
		C0=-a;
	      }
	      n=std::sqrt(A0*A0+B0*B0+C0*C0);
	      A0=A0/n; B0=B0/n; C0=C0/n;
	      double A1,B1,C1; // coordinates of OY0
	      A1=b*C0-c*B0;
	      B1=c*A0-a*C0;
	      C1=a*B0-b*A0;
	      double t=rx->value()*M_PI/180;
	      double A,B,C; // OX coordinates
	      A=cos(t)*A0+sin(t)*A1;
	      B=cos(t)*B0+sin(t)*B1;
	      C=cos(t)*C0+sin(t)*C1;
	      double D,E,F; // OY coordinates
	      D=cos(t)*A1-sin(t)*A0;
	      E=cos(t)*B1-sin(t)*B0;
	      F=cos(t)*C1-sin(t)*C0;
	      // matrix of the rotation in column is [A,B,C] [D,E,F] [a,b,c]
	      /*  1 A D a
		  2 B E b
		  3 C F c
	      */
	      // cerr << "axis " << F-b << "," << a-C << "," << B-D << endl;
	      double qx,qy,qz,qw;
	      qw=std::sqrt(1+A+E+c)/2;
	      qx=(F-b)/4/qw;
	      qy=(a-C)/4/qw;
	      qz=(B-D)/4/qw;
	      n=std::sqrt(qw*qw+qx*qx+qy*qy+qz*qz);
	      q=quaternion_double(qw/n,-qx/n,-qy/n,-qz/n);
	    }
	  } // end if gr3d
	  show_axes=c2->value();
	  show_names=c1->value() | (c3->value()<< 1);
	  if (ct->value()){
	    x_tick=tx->value()/x_scale;
	    y_tick=ty->value()/y_scale;
	  }
	  else {
	    x_tick=tx->value();
	    y_tick=ty->value();
	  }
	  display_mode &= (0xffff ^ 0x100);
	  if (fbox->value())
	    display_mode |= 0x100;
	  display_mode &= (0xffff ^ 0x200);
	  if (triedre->value())
	    display_mode |= 0x200;
	  if (hp &&strlen(autoname_input->value()))
	    autoname(hp->contextptr)=autoname_input->value();
	  redraw();
	  if (r==0)
	    push_cfg();
	}
	if (r==0 || r==1)
	  break;
      } // end else
      currentcfg->value(current_config().c_str());
    } // end for
    autosave_disabled=false;
    w->hide();
  }

  Gen_Value_Slider * parameter2slider(const gen & e,const giac::context *contextptr){
    if (e.is_symb_of_sommet(at_parameter) && e._SYMBptr->feuille.type==_VECT){
      vecteur v = *e._SYMBptr->feuille.evalf(1,contextptr).evalf_double(1,contextptr)._VECTptr;
      if (v.size()>3 &&v[1].type==_DOUBLE_ && v[2].type==_DOUBLE_ && v[3].type == _DOUBLE_ ){
	gen name=e._SYMBptr->feuille._VECTptr->front();
	double step=(v[2]._DOUBLE_val-v[1]._DOUBLE_val)/100.;
	if (v.size()>4 && v[4].type==_DOUBLE_)
	  step=v[4]._DOUBLE_val;
	Fl_Group::current(0);
	Gen_Value_Slider * gvs = new Gen_Value_Slider(0,0,1,1,-2,v[1]._DOUBLE_val,v[2]._DOUBLE_val,step,name.print(contextptr));
	gvs->align(FL_ALIGN_RIGHT);
	gvs->value(v[3]._DOUBLE_val);
	return gvs;
      }
    }
    return 0;
  }

  void Graph2d3d::add(const gen & e){ 
    context * contextptr=hp?hp->contextptr:get_context(this);
    if (e.is_symb_of_sommet(at_trace)){
      gen f=symbolic(at_evalf,e._SYMBptr->feuille);
      f=protecteval(f,1,contextptr);
      trace_instructions.push_back(f);
      this->redraw();
      return;
    }
    plot_instructions.push_back(e); 
    update_infos(e,contextptr);
    if (e.is_symb_of_sommet(at_pnt) && e._SYMBptr->feuille.type==_VECT ){
      vecteur & v = *e._SYMBptr->feuille._VECTptr;
      if (v.size()==3 && v.back().type==_STRNG){
	string newauto=*v.back()._STRNGptr;
	if (newauto.size()>=1 && v[1].type!=_VECT && newauto>=autoname(contextptr)){
	  autoname(contextptr)=newauto;
	  autoname_plus_plus();
	}
      }
    }
    // gen_value_slider inside normal graphic do not work
    // especially if you save them, the output by widget_sprint is unusable
    Gen_Value_Slider * gvs=0;
    if ( (dynamic_cast<Geo2d *>(this) || dynamic_cast<Geo3d *>(this)) && param_group && (gvs=parameter2slider(e,contextptr)) ){
      int n=param_group->children();
      int x=param_group->x(), y=param_group->y(),h=param_group->h(),w=(6*param_group->w()/7);
      int nmax=int(0.5+param_group->h()/(1.5*labelsize()));
      if (n>nmax){ // Resize params height h/(n+1)
	for (int i=0;i<n;++i)
	  param_group->child(i)->resize(x,y+h/(n+1)*i,w,h/(n+1));
      }
      if (n)
	y=param_group->child(n-1)->y()+param_group->child(n-1)->h();
      if (n<nmax)
	n=nmax-1;
      gvs->resize(x,y,w,param_group->h()/(n+1));
      gvs->pos=plot_instructions.size()-1;
      param_group->add(gvs);
      param_group->redraw();
    }
    this->redraw(); 
  }

  void Graph2d3d::find_ylegende(){
    const giac::context * contextptr = get_context(this);
    vecteur aff=ticks(window_ymin,window_ymax,true);
    int affs=aff.size();
    fl_font(FL_HELVETICA,labelsize());
    int taille=5;
    for (int i=0;i<affs;++i){
      double d=evalf_double(aff[i],1,contextptr)._DOUBLE_val;
      string tmp=print_DOUBLE_(d)+y_axis_unit;
      int taillecur=5+int(fl_width(tmp.c_str()));
      if (taillecur>taille)
	taille=taillecur;
    }
    ylegende=double(taille)/labelsize();
  }

  void Graph2d3d::autoscale(bool fullview){
    if (!plot_instructions.empty()){
      // Find the largest and lowest x/y/z in objects (except lines/plans)
      vector<double> vx,vy,vz;
      int s;
      context * contextptr=hp?hp->contextptr:get_context(this);
      bool ortho=autoscaleg(plot_instructions,vx,vy,vz,contextptr);
      autoscaleminmax(vx,window_xmin,window_xmax,fullview);
      if (display_mode & 0x400){
	if (window_xmin<=0){
	  if (vx[0]<=0)
	    window_xmin=-309;
	  else
	    window_xmin=std::log10(vx[0]);
	}
	else
	  window_xmin=std::log10(window_xmin);
	if (window_xmax<=0)
	  window_xmax=-300;
	else
	  window_xmax=std::log10(window_xmax);
      }
      zoomx(1.0);
      autoscaleminmax(vy,window_ymin,window_ymax,fullview);
      if (display_mode & 0x800){
	if (window_ymin<=0){
	  if (vy[0]<=0)
	    window_ymin=-309;
	  else
	    window_ymin=std::log10(vy[0]);
	}
	else
	  window_ymin=std::log10(window_ymin);
	if (window_ymax<=0)
	  window_ymax=-300;
	else
	  window_ymax=std::log10(window_ymax);
      }
      zoomy(1.0);
      autoscaleminmax(vz,window_zmin,window_zmax,fullview);
      zoomz(1.0);
      if (ortho)
	orthonormalize();
    }
    find_ylegende();
    y_tick=find_tick(window_ymax-window_ymin);
    redraw();
    push_cfg();
  }

  void Graph2d3d::zoomx(double d,bool round){
    double x_center=(window_xmin+window_xmax)/2;
    double dx=(window_xmax-window_xmin);
    if (dx==0)
      dx=gnuplot_xmax-gnuplot_xmin;
    dx *= d/2;
    x_tick = find_tick(dx);
    window_xmin = x_center - dx;
    if (round) 
      window_xmin=int( window_xmin/x_tick -1)*x_tick;
    window_xmax = x_center + dx;
    if (round)
      window_xmax=int( window_xmax/x_tick +1)*x_tick;
    parent_redraw(this);
  }

  void Graph2d3d::zoomy(double d,bool round){
    double y_center=(window_ymin+window_ymax)/2;
    double dy=(window_ymax-window_ymin);
    if (dy==0)
      dy=gnuplot_ymax-gnuplot_ymin;
    dy *= d/2;
    y_tick = find_tick(dy);
    window_ymin = y_center - dy;
    if (round)
      window_ymin=int( window_ymin/y_tick -1)*y_tick;
    window_ymax = y_center + dy;
    if (round)
      window_ymax=int( window_ymax/y_tick +1)*y_tick;
    find_ylegende();
    parent_redraw(this);
  }

  void Graph2d3d::zoomz(double d,bool round){
    double z_center=(window_zmin+window_zmax)/2;
    double dz=(window_zmax-window_zmin);
    if (dz==0)
      dz=gnuplot_zmax-gnuplot_zmin;
    dz *= d/2;
    z_tick=find_tick(dz);
    window_zmin = z_center - dz;
    if (round)
      window_zmin=int(window_zmin/z_tick -1)*z_tick;
    window_zmax = z_center + dz;
    if (round)
      window_zmax=int(window_zmax/z_tick +1)*z_tick;
    parent_redraw(this);
  }


  void Graph2d3d::zoom(double d){ 
    zoomx(d);
    zoomy(d);
    zoomz(d);
    push_cfg();
  }


  void Graph2d3d::orthonormalize(){ 
    // don't do anything in base class
  }

  const char * latexfilename(const char * filename_){
    if (filename_)
      return filename_;
    char * filename=file_chooser(gettext("LaTeX filaneme"), "*.tex", "session.tex");
    if (!filename)
      return 0;
    string s=remove_extension(filename)+".tex";
    if (is_file_available(s.c_str())){
      int i=fl_ask("%s",("File "+s+" exists. Overwrite?").c_str());
      if ( !i )
	return 0;
    }
    return s.c_str();
  }

  const char * Graph2d3d::latex(const char * filename_){
    return latexfilename(filename_);
  }

  const char * Graph2d::latex(const char * filename_){
    const char * filename=0;
    if ( (filename=Graph2d3d::latex(filename_)) ){
      double xunit=giac::horiz_latex/(window_xmax-window_xmin);
      double yunit=(h()*giac::horiz_latex*orthonormal_factor)/w()/(window_ymax-window_ymin);
      graph2tex(filename,plot_instructions,window_xmin,window_xmax,window_ymin,window_ymax,xunit,yunit,false,get_context(this));
    }
    return filename;
  }

  void Graph2d3d::up(double d){ 
    window_ymin += d;
    window_ymax += d;
    parent_redraw(this);
    push_cfg();
  }

  void Graph2d3d::down(double d){ 
    window_ymin -= d;
    window_ymax -= d;
    parent_redraw(this);
    push_cfg();
  }

  void Graph2d3d::up_z(double d){ 
    window_zmin += d;
    window_zmax += d;
    parent_redraw(this);
    push_cfg();
  }

  void Graph2d3d::down_z(double d){ 
    window_zmin -= d;
    window_zmax -= d;
    parent_redraw(this);
    push_cfg();
  }

  void Graph2d3d::left(double d){ 
    window_xmin -= d;
    window_xmax -= d;
    parent_redraw(this);
    push_cfg();
  }

  void Graph2d3d::right(double d){ 
    window_xmin += d;
    window_xmax += d;
    parent_redraw(this);
    push_cfg();
  }

  void Graph2d3d::set_axes(int b){ 
    show_axes = b;
    parent_redraw(this);
  }

  void Graph2d3d::resize_mouse_param_group(int W){
    if (mouse_param_group){
      change_group_fontsize(mouse_param_group,labelsize());
      int x=this->x()+w();
      int l=labelsize();
      int bh=min(l+3,h()/8);
      int y=this->y();
      mouse_param_group->resize(x,y,W,h());
      bool is3d=dynamic_cast<Graph3d *>(this);
      if (mouse_position) mouse_position->resize(x,y,W,(is3d?3:2)*l);
      if (mouse_position) y += mouse_position->h();
      button_group->resize(x,y,W,5*bh);
      y += 5*bh;
      param_group->resize(x,y,W,this->y()-y+h());
      mouse_param_group->init_sizes();
      mouse_param_group->redraw();
    }
  }

  void Graph2d3d::resize(int X,int Y,int W,int H){
    /*
    if (parent() && !dynamic_cast<Figure *>(parent()) && !dynamic_cast<Tableur_Group *>(parent()) && mouse_param_group){
      if (parent()->w()!=w()+mouse_param_group->w())
	resize_mouse_param_group(parent()->w()-w());
    }
    */
    Fl_Widget::resize(X,Y,W,H);
  }

  void Graph2d3d::copy(const Graph2d3d & gr){
    window_xmin=gr.window_xmin;
    window_xmax=gr.window_xmax;
    window_ymin=gr.window_ymin;
    window_ymax=gr.window_ymax;
    window_zmin=gr.window_zmin;
    window_zmax=gr.window_zmax;
    npixels=gr.npixels;
    show_axes=gr.show_axes;
    show_names=gr.show_names;
    history = gr.history;
    labelsize(gr.labelsize());
    resize_mouse_param_group(gr.mouse_param_group->w());
    q=gr.q;
    display_mode=gr.display_mode;
    // copy lights
    for (int i=0;i<8;++i){
      light_on[i]=gr.light_on[i];
      light_x[i]=gr.light_x[i];
      light_y[i]=gr.light_y[i];
      light_z[i]=gr.light_z[i];
      light_w[i]=gr.light_w[i];;
      light_diffuse_r[i]=gr.light_diffuse_r[i];
      light_diffuse_g[i]=gr.light_diffuse_g[i];
      light_diffuse_b[i]=gr.light_diffuse_b[i];
      light_diffuse_a[i]=gr.light_diffuse_a[i];
      light_specular_r[i]=gr.light_specular_r[i];
      light_specular_g[i]=gr.light_specular_g[i];
      light_specular_b[i]=gr.light_specular_b[i];
      light_specular_a[i]=gr.light_specular_a[i];
      light_ambient_r[i]=gr.light_ambient_r[i];
      light_ambient_g[i]=gr.light_ambient_g[i];
      light_ambient_b[i]=gr.light_ambient_b[i];
      light_ambient_a[i]=gr.light_ambient_a[i];
      light_spot_x[i]=gr.light_spot_x[i];
      light_spot_y[i]=gr.light_spot_y[i];
      light_spot_z[i]=gr.light_spot_z[i];
      light_spot_w[i]=gr.light_spot_w[i];
      light_spot_exponent[i]=gr.light_spot_exponent[i];
      light_spot_cutoff[i]=gr.light_spot_cutoff[i];
      light_0[i]=gr.light_0[i];
      light_1[i]=gr.light_1[i];
      light_2[i]=gr.light_2[i];
    }
    ntheta=gr.ntheta;
    nphi=gr.nphi;
  }

  void Graph2d3d::add(const vecteur & v){
    const_iterateur it=v.begin(),itend=v.end();
    for (;it!=itend;++it)
      add(*it);
  }

  void Graph2d3d::do_handle(const gen & g){
    if (hp){
      hp->update_pos=hp_pos;
      hp->set_gen_value(hp_pos,g,true);
    }
  }

  int round(double d){
    int res=int(floor(d+0.5));
    int maxpixels=10000; // maximal number of horizontal or vertical pixels
    if (d>maxpixels)
      return maxpixels;
    if (d<-maxpixels)
      return -maxpixels;
    return res;
  }

  bool Graph2d::findij(const gen & e0,double x_scale,double y_scale,double & i0,double & j0,GIAC_CONTEXT) const {
    gen e,f0,f1;
    evalfdouble2reim(e0,e,f0,f1,contextptr);
    if ((f0.type==_DOUBLE_) && (f1.type==_DOUBLE_)){
      if (display_mode & 0x400){
	if (f0._DOUBLE_val<=0)
	  return false;
	f0=std::log10(f0._DOUBLE_val);
      }
      i0=(f0._DOUBLE_val-window_xmin)*x_scale;
      if (display_mode & 0x800){
	if (f1._DOUBLE_val<=0)
	  return false;
	f1=std::log10(f1._DOUBLE_val);
      }
      j0=(window_ymax-f1._DOUBLE_val)*y_scale;
      return true;
    }
    // cerr << "Invalid drawing data" << endl;
    return false;
  }

  int chooseinvecteur(const vecteur & v){
    int s=v.size();
    if (s==0)
      setsizeerr();
    if ((s==1) || (s>5))
      return 0;
    fl_font(FL_HELVETICA,10);
    fl_message_font(FL_HELVETICA,10);
    if (s==2)
      return fl_choice("Choose",v[0].print(giac::context0).c_str(),v[1].print(giac::context0).c_str(),NULL);
    if (s==3)
      return fl_choice("Choose",v[0].print(giac::context0).c_str(),v[1].print(giac::context0).c_str(),v[2].print(giac::context0).c_str());
    if (s==4)
      return fl_choice("Choose",v[0].print(giac::context0).c_str(),v[1].print(giac::context0).c_str(),v[2].print(giac::context0).c_str(),v[3].print(giac::context0).c_str());
    return fl_choice("Choose",v[0].print(giac::context0).c_str(),v[1].print(giac::context0).c_str(),v[2].print(giac::context0).c_str(),v[3].print(giac::context0).c_str(),v[4].print(giac::context0).c_str());
  }

  string printsemi(GIAC_CONTEXT){
    if (xcas_mode(contextptr)==3)
      return "§";
    else
      return ";";
  }


  string cas_recalc_name(){
    if (getenv("XCAS_TMP"))
      return getenv("XCAS_TMP")+("/#c#"+print_INT_(parent_id));
#ifdef WIN32
    return "#c#"+print_INT_(parent_id);
#endif
#ifdef IPAQ
    return "/tmp/#c#"+print_INT_(parent_id);
#endif
    return home_directory()+"#c#"+print_INT_(parent_id);
  }

  void Graph2d::orthonormalize(){ 
    // Center of the directions, orthonormalize
    double h=(this->h()-(show_axes?((title.empty()?1:2)*labelsize()):0))*orthonormal_factor;
    double w=this->w()-(show_axes?ylegende*labelsize():0);
    double window_w=window_xmax-window_xmin,window_h=window_ymax-window_ymin;
    double window_hsize=h/w*window_w;
    if (window_h > window_hsize*1.01){ // enlarge horizontally
      double window_xcenter=(window_xmin+window_xmax)/2;
      double window_wsize=w/h*window_h;
      window_xmin=window_xcenter-window_wsize/2;
      window_xmax=window_xcenter+window_wsize/2;
    }
    if (window_h < window_hsize*0.99) { // enlarge vertically
      double window_ycenter=(window_ymin+window_ymax)/2;
      window_ymin=window_ycenter-window_hsize/2;
      window_ymax=window_ycenter+window_hsize/2;
    }
    x_tick=find_tick(window_xmax-window_xmin);
    y_tick=find_tick(window_ymax-window_ymin);
    parent_redraw(this);
    push_cfg();
  }

  int Graph2d::mouse_rescale(){
    if ( current_i >=0 && current_i <= w() && current_j >= 0 && current_j <= h()){ 
      if (absint(current_i-push_i)>20 && absint(current_j-push_j)>20 ){
	double y_scale=(window_ymax-window_ymin)/(h()-(show_axes?((title.empty()?1:2)*labelsize()):0));
	double x_scale=(window_xmax-window_xmin)/(w()-(show_axes?ylegende*labelsize():0));
	window_xmax=window_xmin+max(current_i,push_i)*x_scale;
	window_xmin=window_xmin+min(current_i,push_i)*x_scale;
	window_ymin=window_ymax-max(current_j,push_j)*y_scale;
	window_ymax=window_ymax-min(current_j,push_j)*y_scale;
	parent_redraw(this);
	push_cfg();
      }
      else {
	++animation_instructions_pos;
	redraw();
      }
      return 1;
    }
    return 0;
  }


  void Graph2d3d::adjust_cursor_point_type(){
    context * contextptr=hp?hp->contextptr:get_context(this);
    if (abs_calc_mode(contextptr)==38){
      double newx,newy,newz;
      find_xyz(current_i,current_j,current_depth,newx,newy,newz);
      int pos=-1;
      gen orig;
      gen res=Graph2d3d::geometry_round(newx,newy,newz,find_eps(),orig,pos);
      cursor_point_type=pos>=0?6:3;
      parent_redraw(this);
    }
  }

  // common to Graph2d and Geo2d
  // MOVE: modify arg_tmp.back()
  // PUSH: set pushed to true and mark mouse position
  int Graph2d::common_in_handle(int event){
    last_event=event;
    if (event== FL_MOUSEWHEEL ){
      if (!Fl::event_inside(this))
	return 0;
      if (Fl::e_dy<0)
	zoom(0.707);
      else
	zoom(1.414);
      return 1;
    }
    int res=handle_mouse(event);
    if (event==FL_UNFOCUS){
      return 1;
    }
    int deltax=x(),deltay=y();
    context * contextptr=hp?hp->contextptr:get_context(this);
    if (event==FL_MOVE || event==FL_DRAG || event==FL_PUSH || event==FL_RELEASE){
      current_i=Fl::event_x()-deltax,current_j=Fl::event_y()-deltay; current_depth=0;
      if (abs_calc_mode(contextptr)==38)
	adjust_cursor_point_type();
    }
    int horizontal_pixels=w()-(show_axes?int(ylegende*labelsize()):0);
    int vertical_pixels= h()-((show_axes?1:0)+(!title.empty()))*labelsize();
    double y_scale=(window_ymax-window_ymin)/vertical_pixels;
    double x_scale=(window_xmax-window_xmin)/horizontal_pixels;
    if (mode!=255 && abs_calc_mode(contextptr)==38 && event==FL_KEYBOARD){
      switch (Fl::event_key()){
      case FL_Right:
	if (current_i<horizontal_pixels)
	  ++current_i;
	adjust_cursor_point_type();
	if (mouse_position) mouse_position->redraw();
	if (Geo2d * geo=dynamic_cast<Geo2d *>(this))
	  geo->geo_handle(moving?FL_DRAG:FL_MOVE);
	return 1;
      case FL_Left:
	if (current_i>0)
	  --current_i;
	adjust_cursor_point_type();
	if (mouse_position) mouse_position->redraw();
	if (Geo2d * geo=dynamic_cast<Geo2d *>(this))
	  geo->geo_handle(moving?FL_DRAG:FL_MOVE);
	return 1;
      case FL_Down:
	if (current_j<vertical_pixels)
	  ++current_j;
	adjust_cursor_point_type();
	if (mouse_position) mouse_position->redraw();
	if (Geo2d * geo=dynamic_cast<Geo2d *>(this))
	  geo->geo_handle(moving?FL_DRAG:FL_MOVE);
	return 1;
      case FL_Up:
	if (current_j>0)
	  --current_j;
	adjust_cursor_point_type();
	if (mouse_position) mouse_position->redraw();
	if (Geo2d * geo=dynamic_cast<Geo2d *>(this))
	  geo->geo_handle(moving?FL_DRAG:FL_MOVE);
	return 1;
      case FL_Page_Up:
	if (current_j>vertical_pixels/10)
	  current_j -= vertical_pixels/10;
	else
	  current_j =0;
	adjust_cursor_point_type();
	if (mouse_position) mouse_position->redraw();
	if (Geo2d * geo=dynamic_cast<Geo2d *>(this))
	  geo->geo_handle(moving?FL_DRAG:FL_MOVE);
	return 1;
      case FL_Page_Down:
	if (current_j<9*vertical_pixels/10)
	  current_j += vertical_pixels/10;
	else
	  current_j=vertical_pixels;
	adjust_cursor_point_type();
	if (mouse_position) mouse_position->redraw();
	if (Geo2d * geo=dynamic_cast<Geo2d *>(this))
	  geo->geo_handle(moving?FL_DRAG:FL_MOVE);
	return 1;
      case FL_End:
	if (current_i<9*horizontal_pixels/10)
	  current_i += horizontal_pixels/10;
	else
	  current_i=horizontal_pixels;
	adjust_cursor_point_type();
	if (mouse_position) mouse_position->redraw();
	if (Geo2d * geo=dynamic_cast<Geo2d *>(this))
	  geo->geo_handle(moving?FL_DRAG:FL_MOVE);
	return 1;
      case FL_Home:
	if (current_i>horizontal_pixels/10)
	  current_i -= horizontal_pixels/10;
	else
	  current_i=0;
	adjust_cursor_point_type();
	if (mouse_position) mouse_position->redraw();
	if (Geo2d * geo=dynamic_cast<Geo2d *>(this))
	  geo->geo_handle(moving?FL_DRAG:FL_MOVE);
	return 1;
      case FL_Enter:
	if (Geo2d * geo=dynamic_cast<Geo2d *>(this)){
	  if (!moving){
	    pushed=true;
	    push_i=current_i;
	    push_j=current_j;
	    geo->geo_handle(FL_PUSH);
	    if (moving)
	      return 1;
	  }
	  int res=geo->geo_handle(FL_RELEASE);
	  pushed=false;
	  return res;
	}
      } // end switch
      if (Graph2d * gr2d= dynamic_cast<Graph2d *>(this)){
	char ch=Fl::event_text()?Fl::event_text()[0]:0;
	if (ch>='a')
	  ch -= 'a'-'A';
	if (ch>='A' && ch<='Z'){
	  gen tmp=gen(string("G")+ch,contextptr);
	  if (tmp.type==_IDNT){
	    tmp=evalf(tmp,1,contextptr);
	    if (tmp.is_symb_of_sommet(at_pnt)){
	      tmp=remove_at_pnt(tmp);
	      if (tmp.is_symb_of_sommet(at_cercle))
		tmp=(tmp._SYMBptr->feuille[0]+tmp._SYMBptr->feuille[1])/2;
	      if (tmp.type==_SYMB)
		tmp=tmp._SYMBptr->feuille;
	      if (tmp.type==_VECT && !tmp._VECTptr->empty())
		tmp=tmp._VECTptr->front();
	      if (tmp.type==_CPLX){
		int horizontal_pixels=w()-(show_axes?int(ylegende*labelsize()):0);
		int vertical_pixels=h()-((show_axes?1:0)+(!title.empty()))*labelsize();
		double y_scale=vertical_pixels/(window_ymax-window_ymin);
		double x_scale=horizontal_pixels/(window_xmax-window_xmin);
		double i,j;
		gr2d->findij(tmp,x_scale,y_scale,i,j,contextptr);
		current_i=int(i+.5);
		current_j=int(j+.5);
		adjust_cursor_point_type();
		if (mouse_position) mouse_position->redraw();
		if (Geo2d * geo=dynamic_cast<Geo2d *>(this))
		  geo->geo_handle(moving?FL_DRAG:FL_MOVE);
		return 1;
	      }
	    }
	  }
	}
      }
    }
    else {
      int res=handle_keyboard(event);
      if (res)
	return res;
    }
    bool old_in_area=in_area;
    in_area=(current_i>=0 && current_i<=horizontal_pixels && current_j>=0 && current_j<=vertical_pixels);
    if (old_in_area!=in_area && !args_tmp.empty())
      redraw();
    if (event==FL_FOCUS){
      if (mouse_position)
	mouse_position->redraw();
      return 1;
    }
    Graph2d * gr2d =dynamic_cast<Graph2d *>(this);
    if (gr2d && pushed && mode==255 && Fl::event_button()==FL_RIGHT_MOUSE ){
      if (event==FL_DRAG){
	parent_redraw(this);
	return 1;
      }
      if (event==FL_RELEASE ){
	pushed = false;
	return gr2d->mouse_rescale();
      }
    }
    if (event==FL_RELEASE){
      pushed = false;
    }
    // If the child process expect a user entry, click -> complex number
    if ( waiting_click ){
      if ( (event==FL_RELEASE) && (current_i < horizontal_pixels) && (current_j < vertical_pixels) ){
	gen a(window_xmin+current_i*x_scale,window_ymax-current_j*y_scale);
	if (waiting_click){
	  waiting_click_value=a;
	  waiting_click=false;
	  return 1;
	}
      }
      return 1;
    }
    if (event==FL_PUSH ){
      if (this!=Fl::focus()){
	Fl::focus(this);
	in_handle(FL_FOCUS);
      }
      pushed=true;
      push_i=current_i;
      push_j=current_j;
      push_depth=current_depth;
      parent_redraw(this);
      return 1;
    }
    return res;
  }

  int Graph2d::in_handle(int event){
    if (parent() && mouse_param_group->parent()!=parent())
      parent()->add(mouse_param_group);
    int res=common_in_handle(event);
    if (res)
      return res;
    if (Fl::event_button()==FL_RIGHT_MOUSE){
      if (event==FL_DRAG){
	parent_redraw(this);
	return 1;
      }
      if ( event==FL_RELEASE )
	return mouse_rescale();
    }
    else {
      if (event==FL_DRAG || event==FL_RELEASE){
	double newx,newy,newz;
	int dw=w()-(show_axes?int(ylegende*labelsize()):0),dh=h()-(show_axes?((title.empty()?1:2)*labelsize()):0);
	double dx=window_xmax-window_xmin;
	double dy=window_ymax-window_ymin;
	double x_scale=dx/dw,y_scale=dy/dh;
	newx=(current_i-push_i)*x_scale;
	newy=(push_j-current_j)*y_scale;
	round3(newx,window_xmin,window_xmax);
	round3(newy,window_ymin,window_ymax);
	if (!(display_mode & 0x80)){
	  window_xmin -= newx;
	  window_xmax -= newx;
	  window_ymin -= newy;
	  window_ymax -= newy;
	}
	push_i = current_i;
	push_j = current_j;
	++animation_instructions_pos;
	parent_redraw(this);
	return 1;
      }
    }
    return 0;
  }

  History_Pack * geo_find_history_pack(Fl_Widget * wid){
    Fl_Group * g = wid->parent();
    if (g){
      int n=g->children();
      for (int i=0;i<n;++i){
	Fl_Widget * w = g->child(i);
	if (Fl_Scroll * s=dynamic_cast<Fl_Scroll *>(w))
	  if (s->children())
	    w=s->child(0);
	if (History_Pack * h=dynamic_cast<History_Pack *>(w)){
	  return h;
	}
      }
    }
    return 0;
  }

  gen geometry_round_numeric(double x,double y,double eps,bool approx){
    return approx?gen(x,y):exact_double(x,eps)+cst_i*exact_double(y,eps);
  }

  gen geometry_round_numeric(double x,double y,double z,double eps,bool approx){
    return approx?makevecteur(x,y,z):makevecteur(exact_double(x,eps),exact_double(y,eps),exact_double(z,eps));
  }

  void round3(double & x,double xmin,double xmax){
    double dx=std::abs(xmax-xmin);
    double logdx=std::log10(dx);
    int ndec=int(logdx)-4;
    double xpow=std::pow(10.0,ndec);
    int newx=int(x/xpow);
    x=newx*xpow;
  }

  int change_line_type(int & res,bool show_approx,bool & approx,const string & title,bool dim3,bool & formel,bool & untranslate,bool del,int fontsize){
    static Fl_Window * w = 0;
    static Fl_Value_Input * epaisseur=0,*epaisseur_point;
    static Fl_Return_Button * button0 = 0 ;
    static Fl_Button * button1 =0,*button_color=0,*button2=0;
    static Fl_Check_Button * button_approx=0, * button_formel=0,*button_untranslate=0;
    static Line_Type *ltres=0;
    static Line_Type *lt0=0,*lt1=0,*lt2=0,*lt3=0,*lt4=0,*lt5=0,*lt6=0,*lt7=0;
    static Line_Type *pt0=0,*pt1=0,*pt2=0,*pt3=0,*pt4=0,*pt5=0,*pt6=0,*pt7=0;
    static Line_Type *st0=0,*st1=0,*st2=0,*st3=0; // quadrant1,2,3,4
    static Line_Type *fp0=0,*fp1=0; // fill polygone
    static Line_Type *hn0=0,*hn1=0; // hidden_name
    if (!w){
#ifdef IPAQ
      int dx=240,dy=300;
#else
      int dx=25*fontsize, dy=10*fontsize;
#endif
      int lignes=6;
      Fl_Group::current(0);
      w=new Fl_Window(dx,dy);
      epaisseur=new Fl_Value_Input(dx/4,2,dx/4-2,dy/lignes-4,gettext("Thickness"));
      epaisseur->when(FL_WHEN_CHANGED);
      epaisseur->step(0.1);
      epaisseur->maximum(8);
      epaisseur->minimum(1);
      epaisseur_point=new Fl_Value_Input(3*dx/4,2,dx/4-2,dy/lignes-4,gettext("Point width"));
      epaisseur_point->when(FL_WHEN_CHANGED);
      epaisseur_point->step(0.1);
      epaisseur_point->maximum(8);
      epaisseur_point->minimum(1);
      // ltres->label(gettext("Line type"));
      int x=dx/10;
      lt0=new Line_Type(x,dy/lignes,dx/10,dy/lignes,0);
      x+=lt0->w();
      lt1=new Line_Type(x,dy/lignes,dx/10,dy/lignes,_DASH_LINE);
      x+=lt1->w();
      lt2=new Line_Type(x,dy/lignes,dx/10,dy/lignes,_DOT_LINE);
      x+=lt2->w();
      lt3=new Line_Type(x,dy/lignes,dx/10,dy/lignes,_DASHDOT_LINE);
      x+=lt3->w();
      lt4=new Line_Type(x,dy/lignes,dx/10,dy/lignes,_DASHDOTDOT_LINE);
      x+=lt4->w();
      lt5=new Line_Type(x,dy/lignes,dx/10,dy/lignes,_CAP_FLAT_LINE);
      x+=lt5->w();
      lt6=new Line_Type(x,dy/lignes,dx/10,dy/lignes,_CAP_ROUND_LINE);
      x+=lt6->w();
      lt7=new Line_Type(x,dy/lignes,dx/10,dy/lignes,_CAP_SQUARE_LINE);
      x+=lt7->w();
      x=dx/10;
      pt0=new Line_Type(x,2*dy/lignes,dx/10,dy/lignes,_POINT_WIDTH_3);
      pt0->show_pnt(true);
      pt0->show_line(false);
      x+=pt0->w();
      pt1=new Line_Type(x,2*dy/lignes,dx/10,dy/lignes,_POINT_LOSANGE | _POINT_WIDTH_3);
      pt1->show_pnt(true);
      pt1->show_line(false);
      x+=pt1->w();
      pt2=new Line_Type(x,2*dy/lignes,dx/10,dy/lignes,_POINT_PLUS | _POINT_WIDTH_3);
      pt2->show_pnt(true);
      pt2->show_line(false);
      x+=pt2->w();
      pt3=new Line_Type(x,2*dy/lignes,dx/10,dy/lignes,_POINT_CARRE| _POINT_WIDTH_3);
      pt3->show_pnt(true);
      pt3->show_line(false);
      x+=pt3->w();
      pt4=new Line_Type(x,2*dy/lignes,dx/10,dy/lignes,_POINT_INVISIBLE | _POINT_WIDTH_3);
      pt4->show_pnt(true);
      pt4->show_line(false);
      x+=pt4->w();
      pt5=new Line_Type(x,2*dy/lignes,dx/10,dy/lignes,_POINT_TRIANGLE| _POINT_WIDTH_3);
      x+=pt5->w();
      pt5->show_pnt(true);
      pt5->show_line(false);
      pt6=new Line_Type(x,2*dy/lignes,dx/10,dy/lignes,_POINT_ETOILE| _POINT_WIDTH_3);
      pt6->show_pnt(true);
      pt6->show_line(false);
      x+=pt6->w();
      pt7=new Line_Type(x,2*dy/lignes,dx/10,dy/lignes,_POINT_POINT |_POINT_WIDTH_3);
      pt7->show_pnt(true);
      pt7->show_line(false);
      x+=pt7->w();
      x=dx/20;
      st0=new Line_Type(x,3*dy/lignes,dx/10,dy/lignes,_QUADRANT1);
      st0->show_pnt(true);
      st0->show_text(true);
      st0->show_line(false);
      x+=st0->w();
      st1=new Line_Type(x,3*dy/lignes,dx/10,dy/lignes,_QUADRANT2);
      st1->show_pnt(true);
      st1->show_text(true);
      st1->show_line(false);
      x+=st1->w();
      st2=new Line_Type(x,3*dy/lignes,dx/10,dy/lignes,_QUADRANT3);
      st2->show_pnt(true);
      st2->show_text(true);
      st2->show_line(false);
      x+=st2->w();
      st3=new Line_Type(x,3*dy/lignes,dx/10,dy/lignes,_QUADRANT4);
      st3->show_pnt(true);
      st3->show_text(true);
      st3->show_line(false);
      x+=st3->w()+dx/20;
      hn0=new Line_Type(x,3*dy/lignes,dx/10,dy/lignes,0);
      hn0->show_text(true);
      hn0->show_line(false);
      x+=hn0->w();
      hn1=new Line_Type(x,3*dy/lignes,dx/10,dy/lignes,_HIDDEN_NAME);
      hn1->show_text(true);
      hn1->show_line(false);
      x+=hn1->w()+dx/20;
      fp0=new Line_Type(x,3*dy/lignes,dx/10,dy/lignes,0);
      fp0->show_poly(true);
      fp0->show_line(false);
      x+=fp0->w();
      fp1=new Line_Type(x,3*dy/lignes,dx/10,dy/lignes,_FILL_POLYGON);
      fp1->show_poly(true);
      fp1->show_line(false);
      x+=fp1->w();
      ltres = new Line_Type(2,2+4*dy/lignes,dx/5-4,dy/lignes-4,res);
      ltres->show_pnt(true);
      ltres->show_poly(true);
      button_color = new Fl_Button(dx/5+2,2+4*dy/lignes,dx/5-4,dy/lignes-4);
      button_formel=new Fl_Check_Button(2*dx/5+2,2+4*dy/lignes,dx/5-4,dy/lignes-4);
      button_formel->label("symb");
      button_formel->tooltip(gettext("Make point symbolic"));
      button_untranslate=new Fl_Check_Button(3*dx/5+2,2+4*dy/lignes,dx/5-4,dy/lignes-4);
      button_untranslate->label("untranslate");
      button_untranslate->tooltip(gettext("Remove translation"));
      button0 = new Fl_Return_Button(2,2+5*dy/lignes,dx/3-4,dy/lignes-4);
      button0->shortcut(0xff0d);
      button0->label(gettext("OK"));
      button1 = new Fl_Button(dx/3+2,2+5*dy/lignes,dx/3-4,dy/lignes-4);
      button1->shortcut(0xff1b);
      button1->label(gettext("Cancel"));
      button2 = new Fl_Button(2*dx/3+2,2+5*dy/lignes,dx/3-4,dy/lignes-4);
      button2->label(gettext("Delete"));
      button2->tooltip(gettext("Delete current object"));
      w->end();
      w->resizable(w);
      change_group_fontsize(w,fontsize);
    }
    if (formel){
      button_formel->show();
      button_formel->value(false);
      button_untranslate->show();
      button_untranslate->value(false);
    }
    else {
      button_formel->hide();
      button_untranslate->hide();
    }
    if (del)
      button2->show();
    else
      button2->hide();
    /*
    if (dim3){
      pt0->hide(); pt1->hide(); 
      pt2->hide(); pt3->hide(); 
      pt4->hide(); pt5->hide(); 
      pt6->hide(); pt7->hide(); 
    }
    else {
      pt0->show(); pt1->show(); 
      pt2->show(); pt3->show(); 
      pt4->show(); pt5->show(); 
      pt6->show(); pt7->show(); 
    }
    */
    static string titlestr;
    titlestr=gettext("Object attributs ")+title;
    w->label(titlestr.c_str());
    int r=-1;
    int lt=(res & 0x01c00000);
    int pt=(res & 0x0e000000);
    int st=(res & 0x30000000);
    bool fp=res&0x40000000;
    bool hn=res&0x80000000;
    int newcol=res;
    Fl_Color couleur=Fl_Color(res&0xffff);
    epaisseur->value( ((res&0x00070000) >> 16) +1);
    epaisseur_point->value( ((res&0x00380000) >> 19) +1);
    ltres->show_text(!hn);
    ltres->line_type(res);
    button_color->color(couleur);
    w->set_modal();
    w->show();
    autosave_disabled=true;
    w->hotspot(w);
    Fl::focus(epaisseur);
    for (;;) {
      Fl_Widget *o = Fl::readqueue();
      if (!o) Fl::wait();
      else {
	if (o == button0) {r = 0; break;}
	if (o == button1) {r = 1; break;}
	if (o == button2) {r = 2; break;}
	if (o==lt0 || o==lt1 || o==lt2 || o==lt3 || o==lt4 || o==lt5 || o==lt6 || o==lt7) { 
	  lt=(w->find(o)-w->find(lt0)) << 22;
	}
	if (o==pt0 || o==pt1 || o==pt2 || o==pt3 || o==pt4 || o==pt5 || o==pt6 || o==pt7) { 
	  pt=(w->find(o)-w->find(pt0)) << 25;
	}
	if (o==st0 || o==st1 || o==st2 || o==st3){
	  st=(w->find(o)-w->find(st0)) << 28;	  
	}
	if (o==button_color){
	  couleur=fl_show_colormap(couleur);
	  button_color->color(couleur);
	}
	// if (o==button_approx)
	//  button_approx->value(!button_approx->value());
	if (o==fp0)
	  fp=false;
	if (o==fp1)
	  fp=true;
	if (o==hn0)
	  hn=false;
	if (o==hn1)
	  hn=true;
	newcol = couleur;
	newcol += Min(8,Max(0,int(epaisseur->value())-1))*_LINE_WIDTH_2;
	newcol += Min(8,Max(0,int(epaisseur_point->value())-1))*_POINT_WIDTH_2;
	newcol += lt+pt+st;
	if (fp)
	  newcol += _FILL_POLYGON;
	if (hn)
	  newcol += _HIDDEN_NAME;
	ltres->line_type(newcol);
	if (o == w) { r=1; break; }
      }
    }
    autosave_disabled=false;
    w->hide();
    if (!r){
      res=newcol;
      if (formel){
	formel=button_formel->value();
	untranslate=button_untranslate->value();
      }
    }
    return r;
  }

  bool is_numeric(const gen & a);
  bool is_numeric(const vecteur & v){
    const_iterateur it=v.begin(),itend=v.end();
    for (;it!=itend;++it){
      if (!is_numeric(*it))
	return false;
    }
    return true;
  }

  bool is_numeric(const gen & a){
    switch (a.type){
    case _DOUBLE_: case _INT_: case _ZINT: case _REAL:
      return true;
    case _CPLX:
      return is_numeric(*a._CPLXptr) && is_numeric(*(a._CPLXptr+1));
    case _VECT:
      return is_numeric(*a._VECTptr);
    case _FRAC:
      return is_numeric(a._FRACptr->num) && is_numeric(a._FRACptr->den);
    case _SYMB:
      if (a.is_symb_of_sommet(at_prod) || a.is_symb_of_sommet(at_inv) || a.is_symb_of_sommet(at_neg) || a.is_symb_of_sommet(at_plus))
	return is_numeric(a._SYMBptr->feuille);
    default:
      return false;
    }
  }

  void Graph2d3d::change_attributs(int hp_pos){
    if (!hp || hp_pos<0)
      return;
    gen g=hp->parse(hp_pos);
    const giac::context * contextptr = hp->contextptr;
    gen g2(g),pntname,delta,description;
    if (g.is_symb_of_sommet(at_sto)){
      gen & gf=g._SYMBptr->feuille;
      if (gf.type==_VECT && gf._VECTptr->size()==2){
	g2=gf._VECTptr->front();
	pntname=gf._VECTptr->back();
	description=gf._VECTptr->front();
      }
    }
    bool changepntname=is_zero(pntname.type);
    if (changepntname){
      pntname=gen(autoname(hp->contextptr),hp->contextptr);
      description=g;
      autoname_plus_plus();
    }
    // A:=point() or A:=point()+...
    if (g2.is_symb_of_sommet(at_plus)){
      gen & g2f=g2._SYMBptr->feuille;
      if (g2f.type==_VECT && g2f._VECTptr->size()>=2){
	g2=g2f._VECTptr->front();
	delta=symbolic(at_plus,vecteur(g2f._VECTptr->begin()+1,g2f._VECTptr->end()));
      }
    }
    int def=default_color(hp->contextptr);
    if (g2.is_symb_of_sommet(at_couleur) || g2.is_symb_of_sommet(at_display)){
      g2=g2._SYMBptr->feuille;
      if (g2.type==_VECT && g2._VECTptr->size()==2 && g2._VECTptr->back().type==_INT_){
	def=g2._VECTptr->back().val;
	g2=g2._VECTptr->front();
      }
    }
    if (g2.type==_SYMB){
      gen g3=g2._SYMBptr->feuille;
      if (g3.type!=_VECT)
	g3=gen(vecteur(1,g3),_SEQ__VECT);
      if (g3.type==_VECT && !g3._VECTptr->empty()){
	vecteur attributs(1,def);
	int s=read_attributs(*g3._VECTptr,attributs,contextptr);
	vecteur v3(g3._VECTptr->begin(),g3._VECTptr->begin()+s);
	if (s){
	  int g6=attributs[0].val;
	  bool b,formel=g2.is_symb_of_sommet(at_point) && is_numeric(v3),untranslate=false;
	  int r=change_line_type(g6,false,b,pntname.print(contextptr)+": "+description.print(contextptr),(dynamic_cast<Graph3d *>(this)),formel,untranslate,true,labelsize());
	  if (r!=1){
	    if (untranslate)
	      delta=0;
	    if (r==2){
	      hp->remove_entry(hp_pos);
	      hp->update();
	      hp->resize();
	    }
	    else {
	      if (changepntname || formel || untranslate || g6!=attributs[0].val){
		int v3s=0;
		vecteur savev;
		if (formel){
		  v3s=v3.size();
		  if (v3s>3)
		    v3s=3;
		  if (v3s==1){
		    v3.push_back(im(v3[0],contextptr));
		    v3[0]=re(v3[0],contextptr);
		    v3s=2;
		  }
		  if (v3s>0){
		    savev.push_back(makevecteur(v3[0],window_xmin,window_xmax));
		    v3[0]=gen(pntname.print(contextptr)+"x",hp->contextptr);
		  }
		  if (v3s>1){
		    savev.push_back(makevecteur(v3[1],window_ymin,window_ymax));
		    v3[1]=gen(pntname.print(contextptr)+"y",hp->contextptr);
		  }
		  if (v3s>2){
		    savev.push_back(makevecteur(v3[2],window_zmin,window_zmax));
		    v3[2]=gen(pntname.print(contextptr)+"z",hp->contextptr);
		  }
		}
		gen g7(symbolic(g2._SYMBptr->sommet,gen(v3,_SEQ__VECT)));
		g7=add_attributs(g7,g6,contextptr);
		g7=symbolic(at_sto,makevecteur((is_zero(delta)?g7:symbolic(at_plus,makevecteur(g7,delta))),pntname));
		hp->set_gen_value(hp_pos,g7,false);
		for (int k=v3s-1;k>=0;k--){
		  hp->add_entry(hp_pos);
		  hp->set_gen_value(hp_pos,symbolic(at_assume,symbolic(at_equal,makevecteur(v3[k],savev[k]))),false);
		}
		hp->update(hp_pos-1);
	      }
	    } // end else r==-1
	  } // end if (r)
	} // end if (s)
      }
    }
    hp->focus(hp_pos);
  }

  void Graph2d3d::change_attributs(){
    if (!hp)
      return;
    if (!selected.empty()){
      int i=findfirstpoint(selection2vecteur(selected));
      if (i>=0 && i<selected.size())
	hp_pos=selected[i];
      else
	hp_pos=selected.front();
      change_attributs(hp_pos);
      selected.clear();
    } // end if selected.empty()
  }

  double Graph2d3d::find_eps(){
    double dx=window_xmax-window_xmin;
    double dy=window_ymax-window_ymin;
    double dz=window_zmax-window_zmin;
    double eps,epsx,epsy;
    int L=h()>w()?w():h();
    Graph3d * gr3d=dynamic_cast<Graph3d *>(this);
    epsx=(npixels*dx)/(gr3d?L:w());
    epsy=(npixels*dy)/(gr3d?L:h());
    eps=(epsx<epsy)?epsy:epsx;
    if (gr3d && dz>dy && dz >dx){
      eps=npixels*dz/L;
      eps *= 2;
    }
    return eps;
  }

  int Graph2d3d::geo_handle(int event){
    Graph3d * gr3d = dynamic_cast<Graph3d *>(this);
    Graph2d * gr2d = dynamic_cast<Graph2d *>(this);
    if (!gr2d && !gr3d)
      return 0;
    if (!hp)
      return 0;
    if (gr3d && !gr3d->push_in_area){
      if (event==FL_RELEASE && Fl::event_button()== FL_RIGHT_MOUSE ){
	pushed=moving=moving_frame=false;
	change_attributs();
	args_tmp.clear();
	return 1;
      }
      return 0;
    }
    if ( (event==FL_KEYBOARD && Fl::event_key()==FL_Escape ) || (!in_area && event==FL_RELEASE) ){
      pushed=false;
      moving=moving_frame=false;
      args_tmp.clear();
      redraw();
      return 1;
    }
    context * contextptr=hp?hp->contextptr:get_context(this);
    double eps=find_eps();
    int pos;
    gen tmp,tmp2,decal;
    if ( pushed && !moving && !moving_frame && mode ==0 && in_area && event==FL_DRAG){
      redraw();
      return 1;
    }
    if (mode>=2 && event==FL_MOVE && args_tmp.size()>mode)
      event=FL_RELEASE;
    if ( in_area && ((mode!=1 && event==FL_DRAG) || event==FL_PUSH || event==FL_RELEASE || (mode>=2 && event==FL_MOVE)) ){
      double newx,newy,newz;
      find_xyz(current_i,current_j,current_depth,newx,newy,newz);
      round3(newx,window_xmin,window_xmax);
      round3(newy,window_ymin,window_ymax);
      if (gr3d)
	round3(newz,window_zmin,window_zmax);
      tmp=geometry_round(newx,newy,newz,eps,tmp2,pos,mode==0 || (args_tmp.size()==mode && function_final.type==_FUNC && equalposcomp(transformation_functions,*function_final._FUNCptr)),event==FL_RELEASE);
      if (tmp.type==_VECT && tmp._VECTptr->size()==3){
	tmp.subtype=_SEQ__VECT;
	tmp=symbolic(at_point,tmp);
      }
      else {
	if (tmp.type!=_IDNT && !tmp.is_symb_of_sommet(at_extract_measure)){
	  tmp=symbolic(at_point,makevecteur(re(tmp,contextptr),im(tmp,contextptr)));
	}
      }
    }
    double newx,newy,newz;
    if (gr2d){
      int dw=w()-(show_axes?int(ylegende*labelsize()):0),dh=h()-(show_axes?((title.empty()?1:2)*labelsize()):0);
      double dx=window_xmax-window_xmin;
      double dy=window_ymax-window_ymin;
      double x_scale=dx/dw,y_scale=dy/dh;
      newx=(current_i-push_i)*x_scale;
      newy=(push_j-current_j)*y_scale;
      newz=0;
    }
    else {
      double x1,y1,z1,x2,y2,z2;
      find_xyz(current_i,current_j,current_depth,x1,y1,z1);
      find_xyz(push_i,push_j,push_depth,x2,y2,z2);
      newx=x1-x2; newy=y1-y2; newz=z1-z2;
    }
    round3(newx,window_xmin,window_xmax);
    round3(newy,window_ymin,window_ymax);      
    if (gr3d){
      round3(newz,window_zmin,window_zmax);
      decal=in_area?geometry_round_numeric(newx,newy,newz,eps,approx):0;
    }
    else
      decal=in_area?geometry_round_numeric(newx,newy,eps,approx):0;
    // cerr << in_area << " " << decal << endl;
    if (event==FL_RELEASE && Fl::event_button()== FL_RIGHT_MOUSE && (is_zero(decal)) ){
      pushed=moving=moving_frame=false;
      change_attributs();
      args_tmp.clear();
      return 1;
    }
    if (mode==0 || mode==255) {
      if (event==FL_PUSH){ 
	// select object && flag to move it 
	if (mode==0 && pos>=0){
	  if (tmp.type==_IDNT){
	    drag_original_value=tmp2;
	    drag_name=tmp;
	  }
	  else {
	    drag_original_value=hp->parse(pos);
	    drag_name=0;
	  }
	  hp_pos=pos;
	  moving = true;
	}
	else { // nothing selected, move frame
	  if (!(display_mode & 0x80)) // disabled by default in 3-d 
	    moving_frame=true;
	}
	return 1;
      }
      if (moving_frame && (event==FL_DRAG || event==FL_RELEASE) ){
	window_xmin -= newx;
	window_xmax -= newx;
	window_ymin -= newy;
	window_ymax -= newy;
	window_zmin -= newz;
	window_zmax -= newz;
	push_i = current_i;
	push_j = current_j;
	redraw();
	if (event==FL_RELEASE)
	  moving_frame=false;
	return 1;
      }
      if (mode==255)
	return 0;
      if (moving && (event==FL_DRAG || event==FL_RELEASE) ){
	if (mouse_position) mouse_position->redraw();
	// cerr << current_i << " " << current_j << endl;
	// avoid point()+complex+complex+complex
	gen newval;
	if (drag_original_value.is_symb_of_sommet(at_plus) && drag_original_value._SYMBptr->feuille.type==_VECT && drag_original_value._SYMBptr->feuille._VECTptr->size()>=2){
	  vecteur v=*drag_original_value._SYMBptr->feuille._VECTptr;
	  if (v[1].is_symb_of_sommet(at_nop))
	    v[1]=v[1]._SYMBptr->feuille;
	  newval=symbolic(at_plus,makevecteur(v[0],symbolic(at_nop,ratnormal(_plus(vecteur(v.begin()+1,v.end()),contextptr)+decal))));
	}
	else {
	  newval=is_zero(decal)?drag_original_value:symbolic(at_plus,makevecteur(drag_original_value,symbolic(at_nop,decal)));
	}
	int dclick = Fl::event_clicks() || drag_original_value.type==_VECT;
	if (!dclick){
	  if (drag_name.type==_IDNT)
	    do_handle(symbolic(at_sto,makevecteur(in_area?newval:drag_original_value,drag_name)));
	  else
	    do_handle(in_area?newval:drag_original_value);
	}
	if (event==FL_RELEASE)
	  moving=false;
	if (event==FL_RELEASE && dclick ){
	  if(is_zero(decal))
	    change_attributs(); // Change object properties
	}
	selected.clear();
	redraw();
	return 1;
      }
      return 0;
    }
    selected.clear();
    if (mode==1){
      if (function_final!=at_point){
	if (event==FL_RELEASE){
	  string args=autoname(contextptr)+":=";
	  if (function_final.type==_FUNC)
	    args += function_final._FUNCptr->ptr()->s;
	  args +="(";
	  if (function_final==at_plotode)
	    args += fcnfield + "," + fcnvars + "," +tmp.print(contextptr) + ",plan)";
	  else
	    args += tmp.print(contextptr) + ")";
	  autoname_plus_plus();
	  hp->set_value(-1,args,true);
	}
	if (event==FL_PUSH || event==FL_DRAG || event==FL_RELEASE)
	  return 1;
	return 0;
      }
      // point|segment mode
      if (event==FL_RELEASE){
	hp_pos=-1;
	if (hp && !args_tmp.empty() && (std::abs(push_i-current_i)>npixels || std::abs(push_j-current_j)>npixels || (gr3d && std::abs(push_depth-current_depth) >0)) ){
	  // make a segment
	  gen val1,val2;
	  if (in_area && args_tmp.front().is_symb_of_sommet(at_point)){
	    val1=gen(autoname(hp->contextptr),hp->contextptr);
	    // put in last history pack level
	    hp->set_gen_value(-1,symbolic(at_sto,makevecteur(add_attributs(args_tmp.front(),couleur,contextptr),val1)),false);
	    hp_pos=hp->children()-1;
	    autoname_plus_plus();
	  }
	  else 
	    val1=args_tmp.front();
	  if (in_area && tmp.is_symb_of_sommet(at_point)){
	    val2=gen(autoname(hp->contextptr),hp->contextptr);
	    gen tmp3=symbolic(at_sto,makevecteur(add_attributs(tmp,couleur,contextptr),val2));
	    hp->set_gen_value(-1,tmp3,false);
	    if (hp_pos<0) hp_pos=hp->children()-1;
	    autoname_plus_plus();
	  }
	  else 
	    val2=tmp;
	  if (in_area){
	    gen tmp3=add_attributs(symbolic(at_segment,makevecteur(val1,val2)),couleur,contextptr);
	    string v1v2=val1.print(contextptr)+val2.print(contextptr);
	    gen g1g2(v1v2,hp->contextptr);
	    if (g1g2.type!=_IDNT)
	      g1g2=gen(v1v2+"_",hp->contextptr);
	    tmp3=symbolic(at_sto,makevecteur(tmp3,g1g2));
	    hp->set_gen_value(-1,tmp3,false);
	    if (hp_pos<0) hp_pos=hp->children()-1;
	    hp->update(hp_pos);
	  }
	  return 1;
	}
	if (in_area && tmp.type!=_IDNT)
	  do_handle(symbolic(at_sto,makevecteur(add_attributs(tmp,couleur,contextptr),gen(autoname(hp->contextptr),hp->contextptr))));
	// element
	if (tmp.type==_IDNT && tmp2.type==_SYMB && !equalposcomp(point_sommet_tab_op,tmp2._SYMBptr->sommet)){
	  // tmp2 is the geo object, find parameter value
	  double newx,newy,newz;
	  find_xyz(current_i,current_j,current_depth,newx,newy,newz);
	  round3(newx,window_xmin,window_xmax);
	  round3(newy,window_ymin,window_ymax);
	  gen t=projection(evalf(tmp2,1,hp->contextptr),gen(newx,newy),hp->contextptr);
	  if (is_undef(t))
	    return 0;
	  gen tmp3=symbolic(at_element,( (t.type<_IDNT || t.type==_VECT)?gen(makevecteur(tmp,t),_SEQ__VECT):tmp));
	  tmp3=symbolic(at_sto,makevecteur(add_attributs(tmp3,couleur,contextptr),gen(autoname(hp->contextptr),hp->contextptr)));
	  hp->set_gen_value(-1,tmp3,false);
	  if (hp_pos<0) hp_pos=hp->children()-1;
	  hp->update(hp_pos);
	}
	return 1;
      }
      if (event==FL_PUSH){
	args_tmp=vecteur(1,tmp);
	return 1;
      }
      if (event==FL_DRAG){
	redraw();
	if (mouse_position) mouse_position->redraw();
	return 1;
      }
      return 0;
    }
    gen tmpval=remove_at_pnt(tmp.eval(1,contextptr));
    gen somm=symbolic(at_sommets,tmp);
    int npoints=1;
    if (!equalposcomp(nosplit_polygon_function,*function_final._FUNCptr)){
      if (tmpval.type==_VECT && tmpval.subtype==_GROUP__VECT)
	npoints=tmpval._VECTptr->size();
      if (tmpval.is_symb_of_sommet(at_cercle))
	npoints=gr2d?2:3;
    }
    unsigned args_size=args_tmp.size();
    // mode>=2
    if (event==FL_MOVE || event==FL_DRAG || event==FL_RELEASE || event==FL_PUSH){
      if (args_size<args_tmp_push_size)
	args_tmp_push_size=args_size;
      args_tmp.erase(args_tmp.begin()+args_tmp_push_size,args_tmp.end());
    }
    unsigned new_args_size=args_tmp.size();
    gen tmp_push=tmp;
    bool swapargs=false;
    if (npoints==2 && (new_args_size==2 || new_args_size==1) && (function_final==at_angleat || function_final==at_angleatraw) ){
      // search if args_tmp[0] or args_tmp[1] is a vertex of tmp
      gen tmp2=remove_at_pnt(evalf(tmp,1,contextptr));
      gen somm=symbolic(at_sommets,tmp);
      if (tmp2.type==_VECT && tmp2._VECTptr->size()==2){
	gen tmpa=remove_at_pnt(evalf(args_tmp[0],1,contextptr));
	gen tmpb=new_args_size==2?remove_at_pnt(evalf(args_tmp[1],1,contextptr)):undef;
	if (npoints==2 && tmpa==tmp2._VECTptr->front() && tmpb!=tmp2._VECTptr->back()){
	  tmp=symbolic(at_at,gen(makevecteur(somm,1),_SEQ__VECT));
	  npoints=1;
	}
	if (npoints==2 && tmpa==tmp2._VECTptr->back() && tmpb!=tmp2._VECTptr->front()){
	  tmp=symbolic(at_at,gen(makevecteur(somm,0),_SEQ__VECT));
	  npoints=1;
	}
	if (npoints==2 && tmpb==tmp2._VECTptr->front() && tmpa!=tmp2._VECTptr->back() ){
	  swapargs=true;
	  tmp=symbolic(at_at,gen(makevecteur(somm,1),_SEQ__VECT));
	  npoints=1;
	}
	if (npoints==2 && tmpb==tmp2._VECTptr->back() && tmpa!=tmp2._VECTptr->front()){
	  swapargs=true;
	  tmp=symbolic(at_at,gen(makevecteur(somm,0),_SEQ__VECT));
	  npoints=1;
	}
      }
    }
    if (npoints+args_tmp.size()>mode)
      npoints=1;
    if (event==FL_MOVE || event==FL_DRAG || event==FL_RELEASE){
      if (mouse_position) mouse_position->redraw();
      if (args_size && args_tmp_push_size && args_push!=tmp_push){
	// replace by current mouse position
	if (npoints==1)
	  args_tmp.push_back(tmp);
	else {
	  gen somm=symbolic(at_sommets,tmp);
	  for (int i=0;i<npoints;++i){
	    args_tmp.push_back(symbolic(at_at,gen(makevecteur(somm,i),_SEQ__VECT)));
	  }
	}
	redraw();
	if (event!=FL_RELEASE || (abs(push_i-current_i)<=5 && abs(push_j-current_j)<=5))
	  return 1;
      }
    }
    if (event==FL_PUSH){
      if (swapargs)
	swapgen(args_tmp[0],args_tmp[1]);
      args_push=tmp_push;
      if (npoints==1)
	args_tmp.push_back(tmp);
      else {
	for (int i=0;i<npoints;++i){
	  args_tmp.push_back(symbolic(at_at,gen(makevecteur(somm,i),_SEQ__VECT)));
	}
      }
      args_tmp_push_size=args_tmp.size();
      redraw();
      return 1;
    }
    if (event==FL_RELEASE){
      int s=args_tmp.size();
      args_tmp_push_size=s;
      int save_undo_position=hp->undo_position;
      if (mode>1 && s>=mode){
	if (s>mode){ 
	  args_tmp=vecteur(args_tmp.begin(),args_tmp.begin()+mode);
	  s=mode;
	}
	gen tmp_plot;
	if (in_area && function_final.type==_FUNC) {
	  gen res,objname=gen(autoname(hp->contextptr),hp->contextptr);
	  hp_pos=hp->children()-1;
	  hp->update_pos=hp_pos;
	  int pos0=hp_pos;
	  unary_function_ptr * ptr=function_final._FUNCptr;
	  int ifinal=mode;
	  if (equalposcomp(measure_functions,*ptr))
	    ifinal--;
	  // first replace points in args_tmp by assignations
	  for (int i=0;i<ifinal;++i){
	    tmp_plot=args_tmp[i];
	    if (tmp_plot.is_symb_of_sommet(at_point)){
	      tmp_plot=symbolic(at_sto,makevecteur(add_attributs(tmp_plot,couleur,contextptr),objname));
	      args_tmp[i]=objname;
	      hp->set_gen_value(hp_pos,tmp_plot,false);
	      hp->add_entry(hp_pos+1);
	      ++hp_pos;
	      autoname_plus_plus();
	      objname=gen(autoname(hp->contextptr),hp->contextptr);
	    }
	  }
	  vecteur argv=args_tmp;
	  if (*ptr==(gr3d?at_sphere:at_cercle)){
	    gen argv1;
	    try {
	      argv1=evalf(args_tmp.back(),1,contextptr);
	      argv1=evalf_double(argv1,1,contextptr);
	    }
	    catch (std::runtime_error & e){
	      argv1=undef;
	    }
	    if (argv1.is_symb_of_sommet(at_pnt) ||argv1.type==_IDNT){
	      argv1=remove_at_pnt(argv1);
	      if ( (argv1.type==_VECT && argv1.subtype==_POINT__VECT) || argv1.type==_CPLX || argv1.type==_IDNT)
	      argv.back()=args_tmp.back()-args_tmp.front();
	    }
	  }
	  tmp_plot=symbolic(*ptr,gen(argv,_SEQ__VECT));
	  try {
	    res=evalf(tmp_plot,1,hp->contextptr);
	  }
	  catch (std::runtime_error & err){
	    res=undef;
	  }
	  tmp_plot=symbolic(at_sto,makevecteur(add_attributs(tmp_plot,couleur,contextptr),objname));
	  hp->set_gen_value(hp_pos,tmp_plot,false);
	  hp->add_entry(hp_pos+1);
	  ++hp_pos;
	  if (res.is_symb_of_sommet(at_pnt)){
	    res=remove_at_pnt(res);
	    int ns=0;
	    if (res.type==_VECT && res.subtype==_GROUP__VECT && (ns=res._VECTptr->size())>2){
	      vecteur l;
	      if (res._VECTptr->back()==res._VECTptr->front())
		--ns;
	      if (function_final.type==_FUNC && equalposcomp(transformation_functions,*function_final._FUNCptr)){
		vecteur argv;
		gen objn,som=symbolic(at_sommets,objname);
		for (int i=1;i<=ns;++i){
		  tmp_plot=symbolic(at_at,gen(makevecteur(som,i-1),_SEQ__VECT));
		  objn=gen(autoname(hp->contextptr)+print_INT_(i),hp->contextptr);
		  argv.push_back(objn);
		  hp->set_gen_value(hp_pos,symbolic(at_sto,gen(makevecteur(tmp_plot,objn),_SEQ__VECT)),false);
		  hp->add_entry(hp_pos+1);
		  ++hp_pos;
		}
		for (int i=1;i<=ns;++i){
		  tmp_plot=symbolic(at_segment,makevecteur(argv[i-1],argv[i%ns]));
		  hp->set_gen_value(hp_pos,symbolic(at_sto,makevecteur(add_attributs(tmp_plot,couleur,contextptr),gen(autoname(hp->contextptr)+print_INT_(ns+i),hp->contextptr))),false);
		  hp->add_entry(hp_pos+1);
		  ++hp_pos;
		}		
	      }
	      else {
		for (int i=mode;i<ns;++i){
		  tmp_plot=symb_at(
				   symbolic(at_sommets,gen(autoname(hp->contextptr),hp->contextptr))
				   ,i,hp->contextptr);
		  gen newname=gen(autoname(hp->contextptr)+print_INT_(i),hp->contextptr);
		  hp->set_gen_value(hp_pos,symbolic(at_sto,makevecteur(tmp_plot,newname)),false);
		  args_tmp.push_back(newname);
		  hp->add_entry(hp_pos+1);
		  ++hp_pos;
		}
		for (int i=1;i<=ns;++i){
		  tmp_plot=symbolic(at_segment,makevecteur(args_tmp[i-1],args_tmp[i%ns]));
		  hp->set_gen_value(hp_pos,symbolic(at_sto,makevecteur(add_attributs(tmp_plot,couleur,contextptr),gen(autoname(hp->contextptr)+print_INT_(ns+i),hp->contextptr))),false);
		  hp->add_entry(hp_pos+1);
		  ++hp_pos;
		}
	      }
	    } // if res.type==_VECT
	  }
	  autoname_plus_plus();
	  // hp->undo_position=save_undo_position;
	  hp->update(pos0);
	}
	args_tmp.clear();
	args_tmp_push_size=0;
      }
      redraw();
      return 1;
    }
    return 0;
  }

  int Graph2d3d::handle(int event){
    if (no_handle)
      return 0;
    context * contextptr=hp?hp->contextptr:get_context(this);
#ifdef HAVE_LIBPTHREAD
    // cerr << "handle lock" << endl;
    int locked=pthread_mutex_trylock(&interactive_mutex);
    if (locked)
      return 0;
#endif
    no_handle=true;
    bool b=io_graph(contextptr);
    io_graph(false,contextptr);
    int res=in_handle(event);
    io_graph(b,contextptr);
    no_handle=false;
#ifdef HAVE_LIBPTHREAD
    pthread_mutex_unlock(&interactive_mutex);
    // cerr << "handle unlock" << endl;
#endif
    return res;
  }

  int Graph2d3d::handle_mouse(int event){
    if (event==FL_MOVE || event==FL_ENTER || event==FL_LEAVE){
      if (mouse_position) mouse_position->redraw();
      if (event==FL_MOVE && hp && show_mouse_on_object){
	double newx,newy,newz;
	Graph3d * gr3d = dynamic_cast<Graph3d *>(this);
	find_xyz(Fl::event_x()-(gr3d?0:x()),Fl::event_y()-(gr3d?0:y()),current_depth,newx,newy,newz);
	int pos=-1;
	gen orig;
	gen res=geometry_round(newx,newy,newz,find_eps(),orig,pos);
	set_cursor(this, pos>=0?FL_CURSOR_HAND:FL_CURSOR_ARROW);	
      }
      else
	set_cursor(this, FL_CURSOR_ARROW);
      return 1;
    }
    return 0;
  }

  int Graph2d3d::handle_keyboard(int event){
    if (event==FL_KEYBOARD){
      // Should bring this event to the current input in the parent() group
      switch (Fl::event_key()){
      case FL_Escape: case FL_BackSpace: case FL_Tab: case FL_Enter: 
      case FL_Print: case FL_Scroll_Lock: case FL_Pause: case FL_Insert: 
      case FL_Home: case FL_Delete: case FL_End: 
      case FL_Shift_L: case FL_Shift_R: case FL_Control_L: 
      case FL_Control_R: case FL_Caps_Lock: case FL_Alt_L: case FL_Alt_R: 
      case FL_Meta_L: case FL_Meta_R: case FL_Menu: case FL_Num_Lock: 
      case FL_KP_Enter:	
	return 1;
      case FL_Left:
	left((window_xmax-window_xmin)/10);
	return 1;
      case FL_Up:
	up((window_ymax-window_ymin)/10);
	return 1;
      case FL_Right: 
	right((window_xmax-window_xmin)/10);
	return 1;
      case FL_Down: 
	down((window_ymax-window_ymin)/10);
	return 1;
      case FL_Page_Up:
	up_z((window_zmax-window_zmin)/10);
	return 1;
      case FL_Page_Down:
	down_z((window_zmax-window_zmin)/10);
	return 1;	
      default:
	char ch=Fl::event_text()?Fl::event_text()[0]:0;
	switch (ch){
	case '-':
	  zoom(1.414);
	  return 1;
	case '+':
	  zoom(0.707);
	  return 1;
	case 'A': case 'a':
	  autoscale(false);
	  return 1;
	case 'V': case 'v':
	  autoscale(true);
	  return 1;
	case 'R': case 'r':
	  oxyz_rotate(this,rotanim_type,rotanim_nstep,rotanim_tstep,rotanim_danim,rotanim_rx,rotanim_ry,rotanim_rz);
	  return 1;
	case 'P': case 'p':
	  paused=!paused;
	  return 1;
	case 'N': case 'n': case 'F': case 'f':
	  animation_instructions_pos++;
	  redraw();
	  return 1;
	case 'B': case 'b':
	  animation_instructions_pos--;
	  redraw();
	  return 1;
	case 'C': case 'c': /* screen capture */
	  if (Graph3d * gr3 = dynamic_cast<Graph3d *>(this)){
	    char * filename = file_chooser(gettext("Export to PNG file"),"*.png","session.png");
	    if(!filename) return 1;
	    gr3->opengl2png(filename);
	    return 1;
	  }
	}	
      }
    }
    return 0;
  }

  int Graph2d3d::in_handle(int event){
    int res=handle_keyboard(event);
    return res?res:handle_mouse(event);
  }

  vecteur Graph2d3d::selection2vecteur(const vector<int> & v){
    int n=v.size();
    vecteur res(n);
    for (int i=0;i<n;++i){
      res[i]=plot_instructions[v[i]];
    }
    return res;
  }

  int findfirstclosedcurve(const vecteur & v){
    int s=v.size();
    for (int i=0;i<s;++i){
      gen g=remove_at_pnt(v[i]);
      if (g.is_symb_of_sommet(at_cercle))
	return i;
      if (g.type==_VECT && g.subtype==_GROUP__VECT){
	vecteur & w=*g._VECTptr;
	if (!w.empty() && w.front()==w.back())
	  return i;
      }
    }
    return -1;
  }

  // find nearest point of x+i*y, returns position in history and either
  // a numeric value or the point name
  gen Graph2d3d::geometry_round(double x,double y,double z,double eps,gen & original,int & pos,bool selectfirstlevel,bool setscroller) {
    if (!hp)
      return undef;
    context * contextptr=hp?hp->contextptr:get_context(this);
    gen tmp;
    pos=-1;
    if (block_signal || is_context_busy(contextptr))
      return undef;
    geometry_round(x,y,z,eps,tmp,contextptr);
    if (selected.empty())
      return tmp;
    if (function_final==at_areaatraw || function_final==at_areaat || function_final==at_perimeteratraw || function_final==at_perimeterat){
      int p=findfirstclosedcurve(selection2vecteur(selected));
      if (p>0){
	pos=p;
      }
    }
    if (pos==-1){
      if (selectfirstlevel){
	sort(selected.begin(),selected.end());
	// patch so that we move element and not the curve
	int p=findfirstpoint(selection2vecteur(selected));
	if (p>0){
	  /*
	  gen g=hp->parse(selected[p]);
	  if (g.is_symb_of_sommet(at_sto) && g._SYMBptr->feuille.type==_VECT ){
	    vecteur & v = *g._SYMBptr->feuille._VECTptr;
	    if (v.size()>1 && v[0].is_symb_of_sommet(at_element))
	      pos=p;
	  }
	  */
	  pos=p;
	}
      }
      else
	pos=findfirstpoint(selection2vecteur(selected));
    }
    gen g=hp->parse( (pos<0)?(pos=selected.front()):(pos=selected[pos]) );
    if (pos>=0 && pos<hp->children()){
      hp->_sel_begin=hp->_sel_end=pos;
      if (setscroller)
	hp->set_scroller(dynamic_cast<Fl_Group *>(hp->child(pos)));
    }
    if (g.is_symb_of_sommet(at_plus) && g._SYMBptr->feuille.type==_VECT && !g._SYMBptr->feuille._VECTptr->empty())
      g=g._SYMBptr->feuille._VECTptr->front();
    if (g.is_symb_of_sommet(at_sto) && g._SYMBptr->feuille.type==_VECT ){
      vecteur & v = *g._SYMBptr->feuille._VECTptr;
      if (v.size()==2){
	original = v[0];
	tmp = v[1];
	if (tmp.type==_IDNT){
	  gen valeur=protecteval(original,1,contextptr);
	  if (valeur.is_symb_of_sommet(at_pnt)){
	    gen & valf = valeur._SYMBptr->feuille;
	    if (valf.type==_VECT){
	      vecteur & valv = *valf._VECTptr;
	      int s=v.size();
	      if (s>1){
		gen valv1=valv[1];
		if (valv1.type==_VECT && valv1._VECTptr->size()>2){
		  tmp=symbolic(at_extract_measure,v[1]);
		}
	      }
	    }
	  }
	}
      }
    }
    return tmp;
  }

  void Graph2d3d::geometry_round(double x,double y,double z,double eps,gen & tmp,GIAC_CONTEXT) {
  }

  void Graph2d::geometry_round(double x,double y,double z,double eps,gen & tmp,GIAC_CONTEXT) {
    tmp= geometry_round_numeric(x,y,eps,approx);
    try {
      selected=nearest_point(plot_instructions,geometry_round_numeric(x,y,eps,true),eps,contextptr);
      // bug bonux: when a figure is saved, plot_instructions is saved
      // if there are sequences in plot_instructions
      // they are not put back in an individual level
      while (!selected.empty() && selected.back()>=hp->children())
	selected.pop_back();
    } catch (...){
    }
  }

  gen int2color(int couleur_){
    gen col;
    if (couleur_){
      gen tmp;
      int val;
      vecteur colv;
      if ( (val=(couleur_ & 0x0000ffff))){
	tmp=val;
	tmp.subtype=_INT_COLOR;
	colv.push_back(tmp);
      }
      if ((val =(couleur_ & 0x00070000))){
	tmp=val;
	tmp.subtype=_INT_COLOR;
	colv.push_back(tmp);
      }
      if ((val =(couleur_ & 0x00380000))){
	tmp=val;
	tmp.subtype=_INT_COLOR;
	colv.push_back(tmp);
      }
      if ((val =(couleur_ & 0x01c00000))){
	tmp=val;
	tmp.subtype=_INT_COLOR;
	colv.push_back(tmp);
      }
      if ((val =(couleur_ & 0x0e000000))){
	tmp=val;
	tmp.subtype=_INT_COLOR;
	colv.push_back(tmp);
      }
      if ((val =(couleur_ & 0x30000000))){
	tmp=val;
	tmp.subtype=_INT_COLOR;
	colv.push_back(tmp);
      }
      if ((val =(couleur_ & 0x40000000))){
	tmp=val;
	tmp.subtype=_INT_COLOR;
	colv.push_back(tmp);
      }
      if ((val =(couleur_ & 0x80000000))){
	tmp=val;
	tmp.subtype=_INT_COLOR;
	colv.push_back(tmp);
      }
      if (colv.size()==1)
	col=colv.front();
      else
	col=symbolic(at_plus,gen(colv,_SEQ__VECT));
    }
    return col;
  }

  std::string print_color(int couleur){
    return int2color(couleur).print(context0);
  }

  giac::gen add_attributs(const giac::gen & g,int couleur_,GIAC_CONTEXT) {
    if (g.type!=_SYMB)
      return g;
    gen & f=g._SYMBptr->feuille;
    if (g._SYMBptr->sommet==at_couleur && f.type==_VECT && !f._VECTptr->empty()){
      gen col=couleur_;
      col.subtype=_INT_COLOR;
      vecteur v(*f._VECTptr);
      v.back()=col;
      return symbolic(at_couleur,gen(v,_SEQ__VECT));
    }
    if (couleur_==default_color(contextptr))
      return g;
    if (g._SYMBptr->sommet==at_of){
      gen col=couleur_;
      col.subtype=_INT_COLOR;
      return symbolic(at_couleur,gen(makevecteur(g,col),_SEQ__VECT));
    }
    vecteur v =gen2vecteur(f);
    gen col=int2color(couleur_);
    v.push_back(symbolic(at_equal,gen(makevecteur(at_display,col),_SEQ__VECT)));
    return symbolic(g._SYMBptr->sommet,(v.size()==1 && f.type!=_VECT)?f:gen(v,f.type==_VECT?f.subtype:_SEQ__VECT));
  }

  void Graph2d3d::set_mode(const giac::gen & f_tmp,const giac::gen & f_final,int m){
    if (mode>=-1){
      pushed=false;
      moving=moving_frame=false;
      history_pos=-1;
      mode=m;
      function_final=f_final;
      function_tmp=f_tmp;
      args_tmp.clear();
    }
  }

  void cb_set_mode(Fl_Widget * m ,const gen & f_tmp,const gen & f_final,int mode,const string & help){
    static string modestr;
    static bool asked=false;
    Figure * f=find_figure(m);
    if (f && f->geo){
      if (!asked && mode==0 && !f->geo->approx){
	int i=fl_ask("%s","Dynamic geometry works faster in approx mode. Drag in approx mode?");
	if (i)
	  f->geo->approx=true;
	asked=true;
      }
      if (dynamic_cast<Geo2d *>(f->geo) || dynamic_cast<Geo3d *>(f->geo)){
	f->geo->selected.clear();
	f->geo->redraw();
	f->geo->handle(FL_FOCUS);
	context * contextptr=f->geo->hp?f->geo->hp->contextptr:0;
	f->geo->set_mode(f_tmp,f_final,mode);
	f->geo->args_help.clear();
	if (mode!=0 && mode!=255){
	  int oldmode=calc_mode(contextptr);
	  calc_mode(0,contextptr);
	  gen g(help,contextptr);
	  calc_mode(oldmode,contextptr);
	  if (g.type==_VECT){
	    const_iterateur it = g._VECTptr->begin(),itend=g._VECTptr->end();
	    for (;it!=itend;++it)
	      f->geo->args_help.push_back(it->print(contextptr));
	  }
	  else
	    f->geo->args_help.push_back(g.print(contextptr));
	}
      }
      if (mode==255)
	modestr=gettext("Frame");
      else
	modestr=mode?gen2string(f_final):gettext("Pointer");
      f->mode->value(modestr.c_str());
    }
  }

  Geo2d::Geo2d(int x,int y,int w,int h,History_Pack * _hp,const char * l): Graph2d(x,y,w,h,l) {
    orthonormalize();
    hp=_hp?_hp:geo_find_history_pack(this);
    if (hp)
      show_mouse_on_object=true;
  }

  int Geo2d::in_handle(int event){
    if (!event)
      return 1;
    if (!hp)
      hp=geo_find_history_pack(this);
    // cerr << event << " " << mode << endl;
    int res=common_in_handle(event);
    if (Fl::event_button()==FL_RIGHT_MOUSE && res && mode==255)
      return res; // right click desactivated
    if (event==FL_UNFOCUS){
      return 1;
    }
    if (event==FL_FOCUS){
      Fl::focus(this);
      return 1;
    }
    if (int gres=geo_handle(event))
      return gres;
    // event not handeld by geometry
    return res;
  }

  void fltk_point(int deltax,int deltay,int i0,int j0,int epaisseur_point,int type_point){
    fl_line_style(FL_SOLID,epaisseur_point-1,0);
    switch (type_point){
    case 1: // losange
      fl_line(deltax+i0-epaisseur_point,deltay+j0,deltax+i0,deltay+j0-epaisseur_point);
      fl_line(deltax+i0,deltay+j0-epaisseur_point,deltax+i0+epaisseur_point,deltay+j0);
      fl_line(deltax+i0-epaisseur_point,deltay+j0,deltax+i0,deltay+j0+epaisseur_point);
      fl_line(deltax+i0,deltay+j0+epaisseur_point,deltax+i0+epaisseur_point,deltay+j0);
      break;
    case 2: // croix verticale
      fl_line(deltax+i0,deltay+j0-epaisseur_point,deltax+i0,deltay+j0+epaisseur_point);
      fl_line(deltax+i0-epaisseur_point,deltay+j0,deltax+i0+epaisseur_point,deltay+j0);
      break;
    case 3: // carre
      fl_line(deltax+i0-epaisseur_point,deltay+j0-epaisseur_point,deltax+i0-epaisseur_point,deltay+j0+epaisseur_point);
      fl_line(deltax+i0+epaisseur_point,deltay+j0-epaisseur_point,deltax+i0+epaisseur_point,deltay+j0+epaisseur_point);
      fl_line(deltax+i0-epaisseur_point,deltay+j0-epaisseur_point,deltax+i0+epaisseur_point,deltay+j0-epaisseur_point);
      fl_line(deltax+i0-epaisseur_point,deltay+j0+epaisseur_point,deltax+i0+epaisseur_point,deltay+j0+epaisseur_point);
      break;
    case 5: // triangle
      fl_line(deltax+i0-epaisseur_point,deltay+j0,deltax+i0,deltay+j0-epaisseur_point);
      fl_line(deltax+i0,deltay+j0-epaisseur_point,deltax+i0+epaisseur_point,deltay+j0);
      fl_line(deltax+i0-epaisseur_point,deltay+j0,deltax+i0+epaisseur_point,deltay+j0);
      break;
    case 7: // point
      if (epaisseur_point>2)
	fl_arc(deltax+i0-(epaisseur_point-1),deltay+j0-(epaisseur_point-1),2*(epaisseur_point-1),2*(epaisseur_point-1),0,360);
      else
	fl_line(deltax+i0,deltay+j0,deltax+i0+1,deltay+j0);
      break;
    case 6: // etoile
      fl_line(deltax+i0-epaisseur_point,deltay+j0,deltax+i0+epaisseur_point,deltay+j0);
      // no break to add the following lines
    case 0: // 0 croix diagonale
      fl_line(deltax+i0-epaisseur_point,deltay+j0-epaisseur_point,deltax+i0+epaisseur_point,deltay+j0+epaisseur_point);
      fl_line(deltax+i0-epaisseur_point,deltay+j0+epaisseur_point,deltax+i0+epaisseur_point,deltay+j0-epaisseur_point);
      break;
    default: // 4 nothing drawn
      break;
    }
    fl_line_style(0);
  }

  void Geo2d::draw(){
    if (!hp)
      hp=geo_find_history_pack(this);
    context * contextptr = hp?hp->contextptr:0;
    int locked=0;
#ifdef HAVE_LIBPTHREAD
    // cerr << "geo2d draw lock" << endl;
    locked=pthread_mutex_trylock(&interactive_mutex);
#endif
    bool b,block;
    if (!locked){
      b=io_graph(contextptr);
      io_graph(contextptr)=false;
      block=block_signal;
      block_signal=true;
    }
    int clip_x,clip_y,clip_w,clip_h;
    // cerr << "geo2d draw block signal " << this << endl;
    fl_clip_box(x(),y(),w(),h(),clip_x,clip_y,clip_w,clip_h);
    fl_push_clip(clip_x,clip_y,clip_w,clip_h);
    int vertical_pixels;
    in_draw(clip_x,clip_y,clip_w,clip_h,vertical_pixels);
    int horizontal_pixels=w()-(show_axes?int(ylegende*labelsize()):0);
    int deltax=x(),deltay=y();
    if (mode<255 && abs_calc_mode(contextptr)==38){
      fl_color(FL_CYAN);
      fltk_point(deltax,deltay,current_i,current_j,cursor_point_type,cursor_point_type);
    }
    // Draw mouse drag
    if ( pushed && !waiting_click &&(current_i>=0 && current_i<=horizontal_pixels) && (current_j>=0 && current_j <=vertical_pixels) && (user_screen || (push_i!=current_i) || (push_j!=current_j))){
      fl_color(FL_RED);
      if ( (mode==255 && Fl::event_button()==FL_RIGHT_MOUSE) || (mode==0 && !moving && !moving_frame) )
	check_fl_rect(deltax+min(push_i,current_i),deltay+min(push_j,current_j),absint(current_i-push_i),absint(current_j-push_j),clip_x,clip_y,clip_w,clip_h,0,0);
      else
	check_fl_line(deltax+push_i,deltay+push_j,deltax+current_i,deltay+current_j,clip_x,clip_y,clip_w,clip_h,0,0);
    }
    fl_pop_clip();
    ++animation_instructions_pos;    
    if (!locked){
      block_signal=block;
      // cerr << "geo2d draw unblock signal " << this << endl;
      io_graph(contextptr)=b;
#ifdef HAVE_LIBPTHREAD
      pthread_mutex_unlock(&interactive_mutex);
    // cerr << "geo2d draw unlock" << endl;
#endif
    }
  }

  inline void swapint(int & i0,int & i1){
    int tmp=i0;
    i0=i1;
    i1=tmp;
  }

  void check_fl_draw(const char * ch,int i0,int j0,int imin,int jmin,int di,int dj,int delta_i,int delta_j){
    /* int n=fl_size();
       if (j0>=jmin-n && j0<=jmin+dj+n) */
    // cerr << i0 << " " << j0 << endl;
    if (strlen(ch)>2000)
      fl_draw("String too long for display",i0+delta_i,j0+delta_j);
    else
      fl_draw(ch,i0+delta_i,j0+delta_j);
  }

  void check_fl_point(int i0,int j0,int imin,int jmin,int di,int dj,int delta_i,int delta_j){
    /* if (i0>=imin && i0<=imin+di && j0>=jmin && j0<=jmin+dj) */
      fl_point(i0+delta_i,j0+delta_j);
  }

  void check_fl_rect(int i0,int j0,int i1,int j1,int imin,int jmin,int di,int dj,int delta_i,int delta_j){
    /*    bool clipped=false;
    if (imin>i0 || jmin>j0){ 
      check_fl_line(i0,j0,i0+i1,j0,imin,jmin,di,dj,delta_i,delta_j);
      check_fl_line(i0,j0,i0,j0+j1,imin,jmin,di,dj,delta_i,delta_j);
      check_fl_line(i0,j0+j1,i0+i1,j0+j1,imin,jmin,di,dj,delta_i,delta_j);
      check_fl_line(i0+i1,j0,i0+i1,j0+j1,imin,jmin,di,dj,delta_i,delta_j);
    }
    else 
      fl_rect(i0+delta_i,j0+delta_j,min(i1,di),min(j1,dj));
    */
    fl_rect(i0+delta_i,j0+delta_j,i1,j1);
  }

  void check_fl_rectf(int x,int y,int w,int h,int imin,int jmin,int di,int dj,int delta_i,int delta_j){
    /*    if (x<imin){ 
      w -= (imin-x); // di -= (imin-x); 
      x=imin; 
    }
    if (y<jmin){ 
      h -= (jmin-y); // dj -= (jmin-y); 
      y=jmin; 
    }
    if (w<=0 || di<=0 || h<=0 || dj<=0) 
      return ;
    fl_rectf(x+delta_i,y+delta_j,min(w,di),min(h,dj));
    */
    fl_rectf(x+delta_i,y+delta_j,w,h);
  }

  // Calls fl_line, checking with bounds
  void check_fl_line(int i0,int j0,int i1,int j1,int imin,int jmin,int di,int dj,int delta_i,int delta_j){
    /* int imax=imin+di,jmax=jmin+dj;
    if (i0>i1){
      swapint(i0,i1);
      swapint(j0,j1);
    }
    if (i0>=imax || i1<=imin)
      return;
    if (i0!=i1){
      // Compute line slope
      double m=(j1-j0)/double(i1-i0);
      if (i0<imin){ // replace i0 by imin, compute corresp. j0
	j0 += int(floor((imin-i0)*m+.5));
	i0 = imin;
      }
      if (i1>imax){
	j1 += int(floor((imax-i1)*m+.5));
	i1 = imax;
      }
    }
    if (j0>j1){
      if (j1>=jmax || j0<=jmin)
	return;
      // Compute line slope
      double m=(i1-i0)/double(j1-j0);
      if (j0>jmax){
	i0 += int(floor((jmax-j0)*m+.5));
	j0 = jmax;
      }
      if (j1<jmin){
	i1 += int(floor((jmin-j1)*m+.5));
	j1 = jmin;
      }
    }
    else {
      if (j0>=jmax || j1<=jmin)
	return;
      if (j0!=j1){
	// Compute line slope
	double m=(i1-i0)/double(j1-j0);
	if (j0<jmin){
	  i0 += int(floor((jmin-j0)*m+.5));
	  j0 = jmin;
	}
	if (j1>jmax){
	  i1 += int(floor((jmax-j1)*m+.5));
	  j1 = jmax;
	}
      }
      } */
    fl_line(i0+delta_i,j0+delta_j,i1+delta_i,j1+delta_j);
  }

  int logplot_points=20;

  void checklog_fl_line(double i0,double j0,double i1,double j1,double deltax,double deltay,bool logx,bool logy,double window_xmin,double x_scale,double window_ymax,double y_scale){
    if (!logx && !logy){
      fl_line(round(i0+deltax),round(j0+deltay),round(i1+deltax),round(j1+deltay));
      return;
    }
    // interpolate (i0,j0)->(i1,j1)
    // if log enabled i0=(log10(x0)-window_xmin)*x_scale -> x0=pow10(i0/x_scale+window_xmin)
    // j0=(window_ymax-log10(y1))*y_scale -> y1=pow10(window_ymax-j0/y_scale)
    if (logx && logy){
      double prevx=i0,prevy=j0,curx,cury;
      double I0=pow10(i0/x_scale+window_xmin),I1=pow10(i1/x_scale+window_xmin),
	J0=pow10(window_ymax-j0/y_scale),J1=pow10(window_ymax-j1/y_scale);
      for (int i=1;i<logplot_points;++i){
	double t=double(i)/logplot_points;
	curx=(log10(I0+t*(I1-I0))-window_xmin)*x_scale;
	cury=(window_ymax-log10(J0+t*(J1-J0)))*y_scale;
	fl_line(round(prevx+deltax),round(prevy+deltay),round(curx+deltax),round(cury+deltay));
	prevx=curx; prevy=cury;
      }
      return;
    }
    if (logy){
      double prevx=i0,prevy=j0,curx,cury;
      double J0=pow10(window_ymax-j0/y_scale),J1=pow10(window_ymax-j1/y_scale);
      for (int i=1;i<logplot_points;++i){
	double t=double(i)/logplot_points;
	curx=i0+t*(i1-i0);
	cury=(window_ymax-log10(J0+t*(J1-J0)))*y_scale;
	fl_line(round(prevx+deltax),round(prevy+deltay),round(curx+deltax),round(cury+deltay));
	prevx=curx; prevy=cury;
      }
      return;
    }
    // logx
    double prevx=i0,prevy=j0,curx,cury;
    double I0=pow10(i0/x_scale+window_xmin),I1=pow10(i1/x_scale+window_xmin);
    for (int i=1;i<logplot_points;++i){
      double t=double(i)/logplot_points;
      curx=(log10(I0+t*(I1-I0))-window_xmin)*x_scale;
      cury=j0+t*(j1-j0);
      fl_line(round(prevx+deltax),round(prevy+deltay),round(curx+deltax),round(cury+deltay));
      prevx=curx; prevy=cury;
    }
  }

  void find_dxdy(const string & legendes,int labelpos,int labelsize,int & dx,int & dy){
    int l=int(fl_width(legendes.c_str()));
    dx=3;
    dy=1;
    switch (labelpos){
    case 1:
      dx=-l-3;
      break;
    case 2:
      dx=-l-3;
      dy=labelsize-2;
      break;
    case 3:
      dy=labelsize-2;
      break;
    }
  }

  void draw_legende(const vecteur & f,int i0,int j0,int labelpos,const Graph2d * iptr,int clip_x,int clip_y,int clip_w,int clip_h,int deltax,int deltay){
    if (f.empty() ||!iptr->show_names )
      return;
    string legendes;
    const context * contextptr = get_context(iptr);
    if (f[0].is_symb_of_sommet(at_curve)){
      gen & f0=f[0]._SYMBptr->feuille;
      if (f0.type==_VECT && !f0._VECTptr->empty()){
	gen & f1 = f0._VECTptr->front();
	if (f1.type==_VECT && f1._VECTptr->size()>4 && (!is_zero((*f1._VECTptr)[4]) || (iptr->show_names & 2)) ){
	  gen legende=f1._VECTptr->front();
	  gen var=(*f1._VECTptr)[1];
	  gen r=re(legende,contextptr),i=im(legende,contextptr),a,b;
	  if (var.type==_IDNT && is_linear_wrt(r,*var._IDNTptr,a,b,contextptr)){
	    i=subst(i,var,(var-b)/a,false,contextptr);
	    legendes=i.print(contextptr);
	  }
	  else
	    legendes=r.print(contextptr)+","+i.print(contextptr);
	  if (legendes.size()>18){
	    if (legendes.size()>30)
	      legendes="";
	    else
	      legendes=legendes.substr(0,16)+"...";
	  }
	}
      }
    }
    if (f.size()>2)
      legendes=gen2string(f[2])+(legendes.empty()?"":":")+legendes;
    if (legendes.empty())
      return;
    if (abs_calc_mode(contextptr)==38 && legendes.size()>1 && legendes[0]=='G')
      legendes=legendes.substr(1,legendes.size()-1);
    fl_font(cst_greek_translate(legendes),iptr->labelsize());
    int dx=3,dy=1;
    find_dxdy(legendes,labelpos,iptr->labelsize(),dx,dy);
    check_fl_draw(legendes.c_str(),iptr->x()+i0+dx,iptr->y()+j0+dy,clip_x,clip_y,clip_w,clip_h,deltax,deltay);
  }

  void petite_fleche(double i1,double j1,double dx,double dy,int deltax,int deltay,int width){
    double dxy=std::sqrt(dx*dx+dy*dy);
    if (dxy){
      dxy/=min(5,int(dxy/10))+width;
      dx/=dxy;
      dy/=dxy;
      double dxp=-dy,dyp=dx;
      dx*=std::sqrt(3.0);
      dy*=sqrt(3.0);
      fl_polygon(round(i1)+deltax,round(j1)+deltay,round(i1+dx+dxp)+deltax,round(j1+dy+dyp)+deltay,round(i1+dx-dxp)+deltax,round(j1+dy-dyp)+deltay);
    }
  }

  // helper for Graph2d::draw method
  // plot_i is the position we are drawing in plot_instructions
  // f is the vector of arguments and s is the function: we draw s(f)
  // legende_x is the x position in pixels of the parameter subwindow
  // parameter_y is the current y position for parameter drawing
  // x_scale and y_scale are the current scales
  // horizontal_pixels and vertical_pixels the size of the window
  void fltk_draw(Graph2d & Mon_image,int plot_i,const gen & g,double x_scale,double y_scale,int clip_x,int clip_y,int clip_w,int clip_h){
    int deltax=Mon_image.x(),deltay=Mon_image.y();
    History_Pack * hp =get_history_pack(&Mon_image);
    context * contextptr=hp?hp->contextptr:get_context(hp);
    if (g.type==_VECT){
      vecteur & v =*g._VECTptr;
      const_iterateur it=v.begin(),itend=v.end();
      for (;it!=itend;++it){
	fltk_draw(Mon_image,plot_i,*it,x_scale,y_scale,clip_x,clip_y,clip_w,clip_h);
	fl_line_style(0); // back to default line style
      } // end for it
    }
    if (g.type!=_SYMB)
      return;
    unary_function_ptr s=g._SYMBptr->sommet;
    if (s==at_animation){
      fltk_draw(Mon_image,plot_i,get_animation_pnt(g,Mon_image.animation_instructions_pos),x_scale,y_scale,clip_x,clip_y,clip_w,clip_h);
      return;
    }
    if (g._SYMBptr->feuille.type!=_VECT)
      return;
    vecteur f=*g._SYMBptr->feuille._VECTptr;
    int mxw=Mon_image.w(),myw=Mon_image.h()-(Mon_image.show_axes?(Mon_image.title.empty()?1:2):0)*Mon_image.labelsize();
    double i0,j0,i0save,j0save,i1,j1;
    int fs=f.size();
    if ((fs==4) && (s==at_parameter)){
      return ;
    }
    string the_legend;
    vecteur style(get_style(f,the_legend));
    int styles=style.size();
    // color
    int ensemble_attributs = style.front().val;
    bool hidden_name = false;
    if (style.front().type==_ZINT){
      ensemble_attributs = mpz_get_si(*style.front()._ZINTptr);
      hidden_name=true;
    }
    else
      hidden_name=ensemble_attributs<0;
    int width           =(ensemble_attributs & 0x00070000) >> 16; // 3 bits
    int epaisseur_point =(ensemble_attributs & 0x00380000) >> 19; // 3 bits
    int type_line       =(ensemble_attributs & 0x01c00000) >> 22; // 3 bits
    if (type_line>4)
      type_line=(type_line-4)<<8;
    int type_point      =(ensemble_attributs & 0x0e000000) >> 25; // 3 bits
    int labelpos        =(ensemble_attributs & 0x30000000) >> 28; // 2 bits
    bool fill_polygon   =(ensemble_attributs & 0x40000000) >> 30;
    int couleur         =(ensemble_attributs & 0x0000ffff);
    epaisseur_point += 2;
    std::pair<Fl_Image *,Fl_Image *> * texture = 0;
    for (int i=2;i<styles;++i){
      gen & attr=style[i];
      if (attr.type==_VECT && attr._VECTptr->size()<=3 ){
	gen attrv0=attr._VECTptr->front();
	if (attrv0.type==_INT_ && attrv0.val==_GL_TEXTURE){
	  gen attrv1=(*attr._VECTptr)[1];
	  if (attrv1.type==_VECT && attrv1._VECTptr->size()==2 && attrv1._VECTptr->front().type==_STRNG && is_undef(attrv1._VECTptr->back())){
	    // reload cached image
	    attrv1=attrv1._VECTptr->front();
	    std::map<std::string,std::pair<Fl_Image *,Fl_Image*> *>::iterator it,itend=texture2d_cache.end();
	    it=texture2d_cache.find(attrv1._STRNGptr->c_str());
	    if (it!=itend){
	      std::pair<Fl_Image *,Fl_Image*> * old= it->second;
	      delete old;
	      texture2d_cache.erase(it);
	    }
	  }
	  if (attrv1.type==_STRNG){
	    get_texture2d(*attrv1._STRNGptr,texture);
	  }
	  // set texture
	  continue;
	} // end attrv0 = gl_texture
      }
    }
    if (s==at_pnt){ 
      // f[0]=complex pnt or vector of complex pnts or symbolic
      // f[1] -> style 
      // f[2] optional=label
      gen point=f[0];
      if (point.type==_VECT && point.subtype==_POINT__VECT)
	return;
      if ( (f[0].type==_SYMB) && (f[0]._SYMBptr->sommet==at_curve) && (f[0]._SYMBptr->feuille.type==_VECT) && (f[0]._SYMBptr->feuille._VECTptr->size()) ){
	// Mon_image.show_mouse_on_object=false;
	point=f[0]._SYMBptr->feuille._VECTptr->back();
      }
      if (is_undef(point))
	return;
      if ( equalposcomp(Mon_image.selected,plot_i))
	fl_color(FL_BLUE);
      else
	xcas_color(couleur);
      fl_line_style(type_line,width+1,0); 
      if (point.type==_SYMB) {
	if (point._SYMBptr->sommet==at_hyperplan || point._SYMBptr->sommet==at_hypersphere)
	  return;
	/* fl_curve is a filled curved, not a Bezier curve!
	if (point._SYMBptr->sommet==at_Bezier && point._SYMBptr->feuille.type==_VECT && point._SYMBptr->feuille._VECTptr->size()==4){
	  vecteur v=*point._SYMBptr->feuille._VECTptr;
	  double i2,j2,i3,j3; gen e,f0,f1;
	  evalfdouble2reim(v[0],e,f0,f1,contextptr);
	  i0=f0._DOUBLE_val; j0=f1._DOUBLE_val;
	  evalfdouble2reim(v[1],e,f0,f1,contextptr);
	  i1=f0._DOUBLE_val; j1=f1._DOUBLE_val;
	  evalfdouble2reim(v[2],e,f0,f1,contextptr);
	  i2=f0._DOUBLE_val; j2=f1._DOUBLE_val;
	  evalfdouble2reim(v[3],e,f0,f1,contextptr);
	  i3=f0._DOUBLE_val; j3=f1._DOUBLE_val;
	  fl_push_matrix();
	  fl_translate(deltax,deltay);
	  fl_mult_matrix(x_scale,0,0,-y_scale,0,0);
	  fl_translate(-Mon_image.window_xmin,-Mon_image.window_ymax);
	  fl_curve(i0,j0,i1,j1,i2,j2,i3,j3);
	  fl_end_complex_polygon();
	  fl_pop_matrix(); // Restore initial matrix
	  return;
	}
	*/
	if (point._SYMBptr->sommet==at_cercle && !(Mon_image.display_mode & 0xc00)){
	  vecteur v=*point._SYMBptr->feuille._VECTptr;
	  gen diametre=remove_at_pnt(v[0]);
	  gen e1=diametre._VECTptr->front().evalf_double(1,contextptr),e2=diametre._VECTptr->back().evalf_double(1,contextptr);
	  gen centre=rdiv(e1+e2,2.0,contextptr);
	  gen e12=e2-e1;
	  double ex=evalf_double(re(e12,contextptr),1,contextptr)._DOUBLE_val,ey=evalf_double(im(e12,contextptr),1,contextptr)._DOUBLE_val;
	  if (!Mon_image.findij(centre,x_scale,y_scale,i0,j0,contextptr))
	    return;
	  gen diam=std::sqrt(ex*ex+ey*ey);
	  gen angle=std::atan2(ey,ex);
	  gen a1=v[1].evalf_double(1,contextptr),a2=v[2].evalf_double(1,contextptr);
	  if ( (diam.type==_DOUBLE_) && (a1.type==_DOUBLE_) && (a2.type==_DOUBLE_) ){
	    i1=diam._DOUBLE_val*x_scale/2.0;
	    j1=diam._DOUBLE_val*y_scale/2.0;
	    double a1d=a1._DOUBLE_val,a2d=a2._DOUBLE_val,angled=angle._DOUBLE_val;
	    bool changer_sens=a1d>a2d;
	    if (changer_sens){
	      double tmp=a1d;
	      a1d=a2d;
	      a2d=tmp;
	    }
	    if (fill_polygon){
	      if (v[1]==0 && v[2]==cst_two_pi)
		fl_pie(deltax+round(i0-i1),deltay+round(j0-j1),round(2*i1),round(2*j1),0,360);
	      else
		fl_pie(deltax+round(i0-i1),deltay+round(j0-j1),round(2*i1),round(2*j1),(angled+a1d)*180/M_PI,(angled+a2d)*180/M_PI);
	    }
	    else {
	      double anglei=(angled+a1d),anglef=(angled+a2d),anglem=(anglei+anglef)/2;
	      fl_arc(deltax+round(i0-i1),deltay+round(j0-j1),round(2*i1),round(2*j1),anglei*180/M_PI,anglef*180/M_PI);
	      if (v.size()>=4){ // if cercle has the optionnal 5th arg
		if (v[3]==2)
		  petite_fleche(i0+i1*std::cos(anglem),j0-j1*std::sin(anglem),-i1*std::sin(anglem),-j1*std::cos(anglem),deltax,deltay,width);
		else {
		  if (changer_sens)
		    petite_fleche(i0+i1*std::cos(anglei),j0-j1*std::sin(anglei),-i1*std::sin(anglei),-j1*std::cos(anglei),deltax,deltay,width);
		  else
		    petite_fleche(i0+i1*std::cos(anglef),j0-j1*std::sin(anglef),i1*std::sin(anglef),j1*std::cos(anglef),deltax,deltay,width);
		}
	      }
	    }
	    // Label a few degrees from the start angle, 
	    // FIXME should use labelpos
	    double anglel=angled+a1d+0.3;
	    if (v.size()>=4 && v[3]==2)
	      anglel=angled+(0.45*a1d+0.55*a2d);
	    i0=i0+i1*std::cos(anglel); 
	    j0=j0-j1*std::sin(anglel);
	    if (!hidden_name)
	      draw_legende(f,round(i0),round(j0),labelpos,&Mon_image,clip_x,clip_y,clip_w,clip_h,0,0);
	    return;
	  }
	} // end circle
	if (point._SYMBptr->sommet==at_pixon){
	  // pixon (i,j,color)
	  if (point._SYMBptr->feuille.type!=_VECT)
	    return;
	  vecteur &v=*point._SYMBptr->feuille._VECTptr;
	  if (v.size()<3 || v[0].type!=_INT_ || v[1].type!=_INT_ || v[2].type!=_INT_)
	    return;
	  int delta_i=v[0].val,delta_j=v[1].val;
	  xcas_color(v[2].val);
#ifdef IPAQ
	  if (delta_i>0 && delta_i<mxw && delta_j>0 && delta_j<myw)
	    check_fl_point(deltax+delta_i,deltay+delta_j,clip_x,clip_y,clip_w,clip_h,0,0);
#else
	  delta_i *= 2;
	  delta_j *= 2;
	  if (delta_i>0 && delta_i<mxw && delta_j>0 && delta_j<myw){
	    check_fl_point(deltax+delta_i,deltay+delta_j,clip_x,clip_y,clip_w,clip_h,0,0);
	    check_fl_point(deltax+delta_i,deltay+delta_j+1,clip_x,clip_y,clip_w,clip_h,0,0);
	    check_fl_point(deltax+delta_i+1,deltay+delta_j,clip_x,clip_y,clip_w,clip_h,0,0);
	    check_fl_point(deltax+delta_i+1,deltay+delta_j+1,clip_x,clip_y,clip_w,clip_h,0,0);
	  }
#endif
	  return;
	}
	if (point._SYMBptr->sommet==at_bitmap){
	  // bitmap(vector of int (1 per line)), 1st line, 1st col, [type]
	  if (point._SYMBptr->feuille.type!=_VECT)
	    return;
	  vecteur &v=*point._SYMBptr->feuille._VECTptr;
	  if (v.size()<3 || v[0].type!=_VECT || v[1].type!=_INT_ || v[2].type!=_INT_ )
	    return;
	  int delta_i=v[1].val,delta_j=v[2].val;
	  double xmin=Mon_image.window_xmin,ymin=Mon_image.window_ymin,xmax=Mon_image.window_xmax,ymax=Mon_image.window_ymax;
	  //gen psize=_Pictsize(0);
	  int bitmap_w=Mon_image.w()-int(Mon_image.ylegende*(Mon_image.show_axes?Mon_image.labelsize():0)),
	    bitmap_h=Mon_image.h()-(Mon_image.show_axes?((Mon_image.title.empty()?1:2)*Mon_image.labelsize()):0);
	  if (v.size()>8){
	    xmin=v[3]._DOUBLE_val;
	    xmax=v[4]._DOUBLE_val;
	    ymin=v[5]._DOUBLE_val;
	    ymax=v[6]._DOUBLE_val;
	    bitmap_w=v[7].val;
	    bitmap_h=v[8].val;
	  }
	  double bitmap_scalex=(xmax-xmin)/bitmap_w,scalex=(Mon_image.window_xmax-Mon_image.window_xmin)/(Mon_image.w()-int(Mon_image.ylegende*(Mon_image.show_axes?Mon_image.labelsize():0)));
	  double bitmap_scaley=(ymax-ymin)/bitmap_h,scaley=(Mon_image.window_ymax-Mon_image.window_ymin)/(Mon_image.h()-(Mon_image.show_axes?((Mon_image.title.empty()?1:2)*Mon_image.labelsize()):0));
	  double X,Y;
	  int ii,jj;
	  const_iterateur it=v[0]._VECTptr->begin(),itend=v[0]._VECTptr->end();
	  for (;it!=itend;++it,++delta_i){
	    if (it->type!=_INT_ && it->type!=_ZINT)
	      continue;
	    gen z=*it;
	    mpz_t zz,zr;
	    if (it->type==_INT_)
	      mpz_init_set_ui(zz,it->val);
	    else
	      mpz_init_set(zz,*it->_ZINTptr);
	    mpz_init(zr);
	    for (int j=delta_j;mpz_sgn(zz);++j){
	      mpz_tdiv_r_2exp (zr, zz, 1);
	      mpz_tdiv_q_2exp (zz, zz, 1);
	      if (mpz_sgn(zr)){
		X=xmin+j*bitmap_scalex;
		ii=int(0.5+(X-Mon_image.window_xmin)/scalex);
		Y=ymax-delta_i*bitmap_scaley;
		jj=int(0.5+(Mon_image.window_ymax-Y)/scaley);
		if (ii>0 && ii<mxw && jj>0 && jj<myw)
		  check_fl_point(deltax+ii,deltay+jj,clip_x,clip_y,clip_w,clip_h,0,0);
	      }
	    }
	    mpz_clear(zr);
	    mpz_clear(zz);
	  }
	  return;
	} // end bitmap
	if (point._SYMBptr->sommet==at_legende){
	  gen & f=point._SYMBptr->feuille;
	  if (f.type==_VECT && f._VECTptr->size()==3){
	    vecteur & fv=*f._VECTptr;
	    if (fv[0].type==_VECT && fv[0]._VECTptr->size()>=2 && fv[1].type==_STRNG && fv[2].type==_INT_){
	      vecteur & fvv=*fv[0]._VECTptr;
	      if (fvv[0].type==_INT_ && fvv[1].type==_INT_){
		fl_font(FL_HELVETICA,Mon_image.labelsize());
		xcas_color(fv[2].val);
		int dx=0,dy=0;
		string legendes(*fv[1]._STRNGptr);
		find_dxdy(legendes,labelpos,Mon_image.labelsize(),dx,dy);
		fl_draw(legendes.c_str(),deltax+fvv[0].val+dx,deltay+fvv[1].val+dy);
	      }
	    }
	  }
	}
      } // end point.type==_SYMB
      if (point.type!=_VECT || (point.type==_VECT && (point.subtype==_GROUP__VECT || point.subtype==_VECTOR__VECT) && point._VECTptr->size()==2 && is_zero(point._VECTptr->back()-point._VECTptr->front())) ){ // single point
	if (!Mon_image.findij((point.type==_VECT?point._VECTptr->front():point),x_scale,y_scale,i0,j0,contextptr))
	  return;
	if (i0>0 && i0<mxw && j0>0 && j0<myw)
	  fltk_point(deltax,deltay,round(i0),round(j0),epaisseur_point,type_point);
	if (!hidden_name)
	  draw_legende(f,round(i0),round(j0),labelpos,&Mon_image,clip_x,clip_y,clip_w,clip_h,0,0);
	return;
      }
      // path
      const_iterateur jt=point._VECTptr->begin(),jtend=point._VECTptr->end();
      if (jt==jtend)
	return;
      bool logx=Mon_image.display_mode & 0x400,logy=Mon_image.display_mode & 0x800;
      if (jtend-jt==2 && logx && point.subtype==_LINE__VECT){
	// find points with + coordinates ax+cx*t=x, ay+cy*t=y=
	gen a=*jt,ax=re(a,contextptr),ay=im(a,contextptr),b=*(jt+1),c=b-a,cx=re(c,contextptr),cy=im(c,contextptr);
	if (!is_zero(cx)){
	  // y=ay+cy/cx*(x-ax)
	  gen x=pow(10,Mon_image.window_xmin,contextptr);
	  gen y=ay+cy/cx*(x-ax);
	  Mon_image.findij(x+cst_i*y,x_scale,y_scale,i0,j0,contextptr);
	  for (int i=1;i<=logplot_points;++i){
	    x=pow(10,Mon_image.window_xmin+i*(Mon_image.window_xmax-Mon_image.window_xmin)/logplot_points,contextptr);
	    y=ay+cy/cx*(x-ax);
	    Mon_image.findij(x+cst_i*y,x_scale,y_scale,i1,j1,contextptr);
	    fl_line(round(i0+deltax),round(j0+deltay),round(i1+deltax),round(j1+deltay));
	    i0=i1;
	    j0=j1;
	  }
	  return;
	}
      }
      if (texture && jtend-jt>2){
	// use *jt and *(jt+2) for the rectangle texture
	Mon_image.findij(*jt,x_scale,y_scale,i0,j0,contextptr);
	if (!Mon_image.findij(*(jt+2),x_scale,y_scale,i1,j1,contextptr))
	  return;
	if (i0>i1)
	  std::swap(i0,i1);
	if (j0>j1)
	  std::swap(j0,j1);
	int tx=int(i0+.5)+deltax;
	int tw=int(i1-i0+.5);
	int ty=int(j0+.5)+deltay;
	int th=int(j1-j0+.5);
	if (texture->second && texture->second->w()==tw && texture->second->h()==th)
	  texture->second->draw(tx,ty,tw,th);
	else {
	  if (texture->second)
	    delete texture->second;
	  if (texture->first){
	    texture->second=texture->first->copy(tw,th);
	    texture->second->draw(tx,ty,tw,th);
	  }
	}
	return;
      }
      if (jt->type==_VECT)
	return;
      if ( (type_point || epaisseur_point>2) && type_line==0 && width==0){
	for (;jt!=jtend;++jt){
	  if (!Mon_image.findij(*jt,x_scale,y_scale,i0,j0,contextptr))
	    return;
	  if (i0>0 && i0<mxw && j0>0 && j0<myw)
	    fltk_point(deltax,deltay,round(i0),round(j0),epaisseur_point,type_point);
	}
	if (!hidden_name)
	  draw_legende(f,round(i0),round(j0),labelpos,&Mon_image,clip_x,clip_y,clip_w,clip_h,0,0);
	return;
      }
      // initial point
      if (!Mon_image.findij(*jt,x_scale,y_scale,i0,j0,contextptr))
	return;
      if (fill_polygon && *jt==*(jtend-1)){
	const_iterateur jtsave=jt;
	gen e,f0,f1;
	// Compute matrix for complex drawing
	fl_push_matrix();
	fl_translate(deltax,deltay);
	fl_mult_matrix(x_scale,0,0,-y_scale,0,0);
	fl_translate(-Mon_image.window_xmin,-Mon_image.window_ymax);
	fl_begin_complex_polygon();
	for (;jt!=jtend;++jt){
	  evalfdouble2reim(*jt,e,f0,f1,contextptr);
	  if ((f0.type==_DOUBLE_) && (f1.type==_DOUBLE_))
	    fl_vertex(f0._DOUBLE_val,f1._DOUBLE_val);
	}
	fl_end_complex_polygon();
	fl_pop_matrix(); // Restore initial matrix
	if (!width){
	  if (!hidden_name)
	    draw_legende(f,round(i0),round(j0),labelpos,&Mon_image,clip_x,clip_y,clip_w,clip_h,0,0);
	  return;
	}
	jt=jtsave;
	fl_line_style(type_line,width,0); 
	fl_color(epaisseur_point-2+(type_point<<3));
      }
      i0save=i0;
      j0save=j0;
      ++jt;
      if (jt==jtend){
	if (i0>0 && i0<mxw && j0>0 && j0<myw)
	  check_fl_point(deltax+round(i0),deltay+round(j0),clip_x,clip_y,clip_w,clip_h,0,0);
	if (!hidden_name)
	  draw_legende(f,round(i0),round(j0),labelpos,&Mon_image,clip_x,clip_y,clip_w,clip_h,0,0);
	return;
      }
      bool seghalfline=( point.subtype==_LINE__VECT || point.subtype==_HALFLINE__VECT ) && (point._VECTptr->size()==2);
      // rest of the path
      for (;;){
	if (!Mon_image.findij(*jt,x_scale,y_scale,i1,j1,contextptr))
	  return;
	if (!seghalfline){
	  checklog_fl_line(i0,j0,i1,j1,deltax,deltay,logx,logy,Mon_image.window_xmin,x_scale,Mon_image.window_ymax,y_scale);
	  if (point.subtype==_VECTOR__VECT){
	    double dx=i0-i1,dy=j0-j1;
	    petite_fleche(i1,j1,dx,dy,deltax,deltay,width);
	  }
	}
	++jt;
	if (jt==jtend){ // label of line at midpoint
	  if (point.subtype==_LINE__VECT){
	    i0=(6*i1-i0)/5-8;
	    j0=(6*j1-j0)/5-8;
	  }
	  else {
	    i0=(i0+i1)/2-8;
	    j0=(j0+j1)/2;
	  }
	  break;
	}
	i0=i1;
	j0=j1;
      }
      // check for a segment/halfline/line
      if ( seghalfline){
	double deltai=i1-i0save,adeltai=std::abs(deltai);
	double deltaj=j1-j0save,adeltaj=std::abs(deltaj);
	if (point.subtype==_LINE__VECT){
	  if (deltai==0)
	    checklog_fl_line(i1,0,i1,clip_h,deltax,deltay,logx,logy,Mon_image.window_xmin,x_scale,Mon_image.window_ymax,y_scale);
	  else {
	    if (deltaj==0)
	      checklog_fl_line(0,j1,clip_w,j1,deltax,deltay,Mon_image.display_mode & 0x400,Mon_image.display_mode & 0x800,Mon_image.window_xmin,x_scale,Mon_image.window_ymax,y_scale);
	    else {
	      // Find the intersections with the 4 rectangle segments
	      // Horizontal x=0 or w =i1+t*deltai: y=j1+t*deltaj
	      vector< complex<double> > pts;
	      double y0=j1-i1/deltai*deltaj;
	      if (y0>=0 && y0<=clip_h)
		pts.push_back(complex<double>(0.0,y0));
	      double yw=j1+(clip_w-i1)/deltai*deltaj;
	      if (yw>=0 && yw<=clip_h)
		pts.push_back(complex<double>(clip_w,yw));
	      // Vertical y=0 or h=j1+t*deltaj, x=i1+t*deltai
	      double x0=i1-j1/deltaj*deltai;
	      if (x0>0 && x0<=clip_w)
		pts.push_back(complex<double>(x0,0.0));
	      double xh=i1+(clip_h-j1)/deltaj*deltai;
	      if (xh>=0 && xh<=clip_w)
		pts.push_back(complex<double>(xh,clip_h));
	      if (pts.size()>=2)
		checklog_fl_line(pts[0].real(),pts[0].imag(),pts[1].real(),pts[1].imag(),deltax,deltay,Mon_image.display_mode & 0x400,Mon_image.display_mode & 0x800,Mon_image.window_xmin,x_scale,Mon_image.window_ymax,y_scale);
	    } // end else adeltai==0 , adeltaj==0
	  } // end else adeltai==0
	} // end LINE_VECT
	else {
	  double N=1;
	  if (adeltai){
	    N=clip_w/adeltai+1;
	    if (adeltaj)
	      N=max(N,clip_h/adeltaj+1);
	  }
	  else {
	    if (adeltaj)
	      N=clip_h/adeltaj+1;
	  }
	  N *= 2; // increase N since rounding might introduce too small clipping
	  while (fabs(N*deltai)>10000)
	    N /= 2;
	  while (fabs(N*deltaj)>10000)
	    N /= 2;
	  checklog_fl_line(i0save,j0save,i1+N*deltai,j1+N*deltaj,deltax,deltay,Mon_image.display_mode & 0x400,Mon_image.display_mode & 0x800,Mon_image.window_xmin,x_scale,Mon_image.window_ymax,y_scale);
	}
      } // end seghalfline
      if ( (point.subtype==_GROUP__VECT) && (point._VECTptr->size()==2))
	; // no legend for segment
      else {
	if (!hidden_name)
	  draw_legende(f,round(i0),round(j0),labelpos,&Mon_image,clip_x,clip_y,clip_w,clip_h,0,0);
      }
    } // end pnt subcase
    
  }

  void Mouse_Position::draw(){
    int clip_x,clip_y,clip_w,clip_h;
    fl_clip_box(x(),y(),w(),h(),clip_x,clip_y,clip_w,clip_h);
    fl_push_clip(clip_x,clip_y,clip_w,clip_h);
    fl_color(FL_CYAN);
    check_fl_rectf(x(),y(),w(),h(),clip_x,clip_y,clip_w,clip_h,0,0);
    fl_color(FL_BLACK);
    check_fl_rect(x(), y(), w(), h(),clip_x,clip_y,clip_w,clip_h,0,0);
    fl_font(FL_HELVETICA,labelsize());
    int cur_x,cur_y;
    // giac::context * contextptr = get_context(graphic);
    if (Graph2d * gr=dynamic_cast<Graph2d *>(graphic)){
      cur_x=gr->current_i; // Fl::event_x()-gr->x();
      cur_y=gr->current_j; // Fl::event_y()-gr->y();
      double y_scale=(gr->h()-(gr->show_axes?((gr->title.empty()?1:2)*gr->labelsize()):0))/(graphic->window_ymax-graphic->window_ymin);
      double x_scale=(gr->w()-(gr->show_axes?gr->ylegende*gr->labelsize():0))/(graphic->window_xmax-graphic->window_xmin);
      double d_mouse_x=graphic->window_xmin+cur_x/x_scale;
      double d_mouse_y=graphic->window_ymax-cur_y/y_scale;
      check_fl_draw(("x:"+giac::print_DOUBLE_(d_mouse_x,3)).c_str(),x()+2,y()+h()/2-2,clip_x,clip_y,clip_w,clip_h,0,0);
      check_fl_draw(("y:"+giac::print_DOUBLE_(d_mouse_y,3)).c_str(),x()+2,y()+h()-2,clip_x,clip_y,clip_w,clip_h,0,0);
    }
    if (Graph3d * gr=dynamic_cast<Graph3d *>(graphic)){
      cur_x=gr->current_i; // Fl::event_x();
      cur_y=gr->current_j; // Fl::event_y();
      double x0,y0,z0;
      gr->find_xyz(cur_x,cur_y,gr->depth,x0,y0,z0);
      fl_draw(("x:"+giac::print_DOUBLE_(x0,3)).c_str(),x()+2,y()+labelsize()-2);
      fl_draw(("y:"+giac::print_DOUBLE_(y0,3)).c_str(),x()+2,y()+2*labelsize()-2);
      fl_draw(("z:"+giac::print_DOUBLE_(z0,3)).c_str(),x()+2,y()+3*labelsize()-2);
    }
    fl_pop_clip();
  }

  string printstring(const gen & g,GIAC_CONTEXT){
    if (g.type==_STRNG)
      return *g._STRNGptr;
    return g.print(contextptr);
  }

  void Graph2d3d::find_title_plot(gen & title_tmp,gen & plot_tmp,GIAC_CONTEXT){
    if (in_area && mode && !args_tmp.empty()){
      if (args_tmp.size()>=2){
	gen function=(mode==int(args_tmp.size()))?function_final:function_tmp;
	if (function.type==_FUNC){
	  bool dim2=dynamic_cast<Graph2d *>(this);
	  vecteur args2=args_tmp;
	  if ( *function._FUNCptr==(dim2?at_cercle:at_sphere)){
	    gen argv1;
	    try {
	      argv1=evalf(args_tmp.back(),1,contextptr);
	      argv1=evalf_double(argv1,1,contextptr);
	    }
	    catch (std::runtime_error & e){
	      argv1=undef;
	    }
	    if (argv1.is_symb_of_sommet(at_pnt) ||argv1.type==_IDNT){
	      argv1=remove_at_pnt(argv1);
	      if ( (argv1.type==_VECT && argv1.subtype==_POINT__VECT) || argv1.type==_CPLX || argv1.type==_IDNT)
		args2.back()=args_tmp.back()-args_tmp.front();
	    }
	  }
	  if (function==at_ellipse)
	    ;
	  title_tmp=gen(args2,_SEQ__VECT);
	  bool b=approx_mode(contextptr);
	  if (!b)
	    approx_mode(true,contextptr);
	  plot_tmp=symbolic(*function._FUNCptr,title_tmp);
	  if (!lidnt(title_tmp).empty())
	    ; // cerr << plot_tmp << endl;
	  bool bb=io_graph(contextptr);
	  int locked=0;
	  if (bb){
#ifdef HAVE_LIBPTHREAD
	    // cerr << "plot title lock" << endl;
	    locked=pthread_mutex_trylock(&interactive_mutex);
#endif
	    if (!locked)
	      io_graph(false,contextptr);
	  }
	  plot_tmp=protecteval(plot_tmp,1,contextptr);
	  if (bb && !locked){
	    io_graph(bb,contextptr);
#ifdef HAVE_LIBPTHREAD
	    pthread_mutex_unlock(&interactive_mutex);
	    // cerr << "plot title unlock" << endl;
#endif
	  }
	  if (!b)
	    approx_mode(false,contextptr);	
	} // end function.type==_FUNC
	else
	  title_tmp=gen(args_tmp,_SEQ__VECT);
      } // end size()>=2
      else	
	title_tmp=args_tmp;
    }
  }
  
  void Graph2d::in_draw(int clip_x,int clip_y,int clip_w,int clip_h,int & vertical_pixels){
    struct timezone tz;
    gettimeofday(&animation_last,&tz);
    gen title_tmp;
    gen plot_tmp;
    History_Pack * hp =get_history_pack(this);
    context * contextptr=hp?hp->contextptr:get_context(this);
    find_title_plot(title_tmp,plot_tmp,contextptr);
    int horizontal_pixels=w()-(show_axes?int(ylegende*labelsize()):0);
    vertical_pixels=h()-((show_axes?1:0)+(!title.empty()))*labelsize();
    int deltax=x(),deltay=y();
    double y_scale=vertical_pixels/(window_ymax-window_ymin);
    double x_scale=horizontal_pixels/(window_xmax-window_xmin);
    // Then redraw the background
    fl_color(FL_WHITE);
    fl_rectf(clip_x, clip_y, clip_w, clip_h);
    fl_color(FL_BLACK);
    if ( !(display_mode & 0x100) )
      fl_rect(x(), y(), horizontal_pixels, vertical_pixels);
    if (background_image){
      if (!background_image->second || background_image->second->w()!=w() || background_image->second->h()!=h()){
	if (background_image->second)
	  delete background_image->second;
	if (background_image->first)
	  background_image->second=background_image->first->copy(w(),h());
      }
      if (background_image->second)
	background_image->second->draw(x(),y(),w(),h());
    }
    // History draw
    /****************/
    int xx,yy,ww,hh;
    fl_clip_box(clip_x,clip_y,horizontal_pixels,vertical_pixels,xx,yy,ww,hh);
    fl_push_clip(xx,yy,ww,hh);
    // fl_push_clip(clip_x,clip_y,horizontal_pixels,vertical_pixels);
    /****************/
    fl_color(FL_BLACK);
    fl_font(FL_HELVETICA,labelsize());
    if ( (display_mode & 2) && !animation_instructions.empty()){
      gen tmp=animation_instructions[animation_instructions_pos % animation_instructions.size()];
      fltk_draw(*this,-1,tmp,x_scale,y_scale,clip_x,clip_y,clip_w,clip_h);
    }
    if ( display_mode & 0x40 ){
      fltk_draw(*this,-1,trace_instructions,x_scale,y_scale,clip_x,clip_y,clip_w,clip_h);
    }
    if (display_mode & 1) {
      const_iterateur at=plot_instructions.begin(),atend=plot_instructions.end(),it,itend;
      for (int plot_i=0;at!=atend;++at,++plot_i){
	if (at->type==_INT_)
	  continue;
	update_infos(*at,contextptr);
	if (at->is_symb_of_sommet(at_parameter)){
	  gen ff = at->_SYMBptr->feuille;
	  vecteur f;
	  if (ff.type==_VECT && (f=*ff._VECTptr).size()==4){
	    // parameters.push_back(f);
	  }
	  continue;
	}
	fltk_draw(*this,plot_i,*at,x_scale,y_scale,clip_x,clip_y,clip_w,clip_h);
      } // end for at
    }
    vecteur plot_tmp_v=gen2vecteur(plot_tmp);
    const_iterateur jt=plot_tmp_v.begin(),jtend=plot_tmp_v.end();
    for (;jt!=jtend;++jt){
      gen plot_tmp=*jt;
      if (plot_tmp.is_symb_of_sommet(at_pnt) && plot_tmp._SYMBptr->feuille.type==_VECT && !plot_tmp._SYMBptr->feuille._VECTptr->empty()){
	vecteur & v=*plot_tmp._SYMBptr->feuille._VECTptr;
	// cerr << v << endl;
	if (v[1].type==_INT_)
	  plot_tmp=symbolic(at_pnt,makevecteur(v[0],v[1].val | _DOT_LINE | _LINE_WIDTH_2));
	else
	  plot_tmp=symbolic(at_pnt,v);
	try {
	  fltk_draw(*this,-1,plot_tmp,x_scale,y_scale,clip_x,clip_y,clip_w,clip_h);
	}
	catch (...){
	}
      }
    }
    fl_line_style(0); // back to default line style
    // Draw axis
    double I0,J0;
    findij(zero,x_scale,y_scale,I0,J0,contextptr);
    int i_0=round(I0),j_0=round(J0);
    if ( show_axes &&  (window_ymax>=0) && (window_ymin<=0)){ // X-axis
      fl_color(x_axis_color);
      check_fl_line(deltax,deltay+j_0,deltax+horizontal_pixels,deltay+j_0,clip_x,clip_y,clip_w,clip_h,0,0); 
      fl_color(FL_CYAN);
      check_fl_line(deltax+i_0,deltay+j_0,deltax+i_0+int(x_scale),deltay+j_0,clip_x,clip_y,clip_w,clip_h,0,0);
      fl_color(x_axis_color);
      if (x_tick>0 && (horizontal_pixels)/(x_scale*x_tick) < 40){
	double nticks=(horizontal_pixels-I0)/(x_scale*x_tick);
	int count=0;
	for (int ii=int(-I0/(x_tick*x_scale));ii<=nticks && count<40;++ii,++count){
	  int iii=int(I0+ii*x_scale*x_tick+.5);
	  check_fl_line(deltax+iii,deltay+j_0,deltax+iii,deltay+j_0-4,clip_x,clip_y,clip_w,clip_h,0,0);
	}
      }
      string tmp=x_axis_name.empty()?"x":x_axis_name;
      check_fl_draw(tmp.c_str(),deltax+horizontal_pixels-int(fl_width(tmp.c_str())),deltay+j_0+labelsize(),clip_x,clip_y,clip_w,clip_h,0,0);
    }
    if ( show_axes && (window_xmax>=0) && (window_xmin<=0) ) {// Y-axis
      fl_color(y_axis_color);
      check_fl_line(deltax+i_0,deltay,deltax+i_0,deltay+vertical_pixels,clip_x,clip_y,clip_w,clip_h,0,0);
      fl_color(FL_CYAN);
      check_fl_line(deltax+i_0,deltay+j_0,deltax+i_0,deltay+j_0-int(y_scale),clip_x,clip_y,clip_w,clip_h,0,0);
      fl_color(y_axis_color);
      if (y_tick>0 && vertical_pixels/(y_tick*y_scale) <40 ){
	double nticks=(vertical_pixels-J0)/(y_tick*y_scale);
	int count=0;
	for (int jj=int(-J0/(y_tick*y_scale));jj<=nticks && count<25;++jj,++count){
	  int jjj=int(J0+jj*y_scale*y_tick+.5);
	  check_fl_line(deltax+i_0,deltay+jjj,deltax+i_0+4,deltay+jjj,clip_x,clip_y,clip_w,clip_h,0,0);
	}
      }
      check_fl_draw(y_axis_name.empty()?"y":y_axis_name.c_str(),deltax+i_0+2,deltay+labelsize(),clip_x,clip_y,clip_w,clip_h,0,0);
    }
    // Ticks
    if (show_axes && (horizontal_pixels)/(x_scale*x_tick) < 40 && vertical_pixels/(y_tick*y_scale) <40  ){
      if (x_tick>0 && y_tick>0 ){
	fl_color(FL_BLACK);
	double nticks=(horizontal_pixels-I0)/(x_scale*x_tick);
	double mticks=(vertical_pixels-J0)/(y_tick*y_scale);
	int count=0;
	for (int ii=int(-I0/(x_tick*x_scale));ii<=nticks;++ii){
	  int iii=int(I0+ii*x_scale*x_tick+.5);
	  for (int jj=int(-J0/(y_tick*y_scale));jj<=mticks && count<1600;++jj,++count){
	    int jjj=int(J0+jj*y_scale*y_tick+.5);
	    check_fl_point(deltax+iii,deltay+jjj,clip_x,clip_y,clip_w,clip_h,0,0);
	  }
	}
      }
    }
    /****************/
    fl_pop_clip();
    /****************/
    fl_color(FL_BLACK);
    fl_font(FL_HELVETICA,labelsize());
    if (!args_help.empty() && args_tmp.size()<= args_help.size()){
      fl_draw((gettext("Click ")+args_help[giacmax(1,args_tmp.size())-1]).c_str(),x(),y()+labelsize()-2);
    }
    string mytitle(title);
    if (!is_zero(title_tmp) && function_final.type==_FUNC)
      mytitle=gen(symbolic(*function_final._FUNCptr,title_tmp)).print(contextptr);
    if (!mytitle.empty()){
      int dt=int(fl_width(mytitle.c_str()));
      check_fl_draw(mytitle.c_str(),deltax+(horizontal_pixels-dt)/2,deltay+h()-labelsize()/4,clip_x,clip_y,clip_w,clip_h,0,0);
    }
    // Boundary values
    fl_font(FL_HELVETICA,labelsize());
    if (show_axes){ 
      int taille,affs,delta;
      vecteur aff;
      string tmp;
      bool logx=display_mode & 0x400,logy=display_mode & 0x800;
      // X
      fl_color(x_axis_color);
      aff=ticks(window_xmin,window_xmax,true);
      affs=aff.size();
      for (int i=0;i<affs;++i){
	double d=evalf_double(aff[i],1,contextptr)._DOUBLE_val;
	tmp=print_DOUBLE_(logx?std::pow(10,d):d);
	delta=int(horizontal_pixels*(d-window_xmin)/(window_xmax-window_xmin));
	taille=int(fl_width(tmp.c_str()));
	if (delta>=taille/2 && delta<=horizontal_pixels){
	  fl_line(x()+delta,y()+vertical_pixels,x()+delta,y()+vertical_pixels+3);
	  if (args_tmp.empty())
	    fl_draw(tmp.c_str(),x()+delta-taille/2,y()+vertical_pixels+labelsize()-2);
	}
      }
      if (args_tmp.empty())
	fl_draw(x_axis_unit.c_str(),x()+horizontal_pixels,y()+vertical_pixels+labelsize()-2);
      // Y
      fl_color(y_axis_color);
      aff=ticks(window_ymin,window_ymax,true);
      affs=aff.size();
      taille=labelsize()/2;
      for (int j=0;j<affs;++j){
	double d=evalf_double(aff[j],1,contextptr)._DOUBLE_val;
	tmp=print_DOUBLE_(logy?std::pow(10,d):d)+y_axis_unit;
	delta=int(vertical_pixels*(window_ymax-d)/(window_ymax-window_ymin));
	if (delta>=taille && delta<=vertical_pixels-taille){
	  fl_line(x()+horizontal_pixels,y()+delta,x()+horizontal_pixels+3,y()+delta);
	  fl_draw(tmp.c_str(),x()+horizontal_pixels+3,y()+delta+taille);
	  int tmpi=fl_width(tmp.c_str());
	  if (tmpi+horizontal_pixels+3>w()){
	    ylegende=(tmpi+3.)/labelsize();
	    redraw();
	  }
	}
      }
    }
  }

  void Graph2d::draw(){
    int clip_x,clip_y,clip_w,clip_h;
    if (!hp)
      hp=geo_find_history_pack(this);
    context * contextptr = hp?hp->contextptr:0;
#ifdef HAVE_LIBPTHREAD
    // cerr << "graph2d draw lock" << endl;
    int locked=pthread_mutex_trylock(&interactive_mutex);
    if (locked)
      return;
#endif
    bool b=io_graph(contextptr);
    io_graph(false,contextptr);
    bool block=block_signal;
    block_signal=true;
    // cerr << "graph2d draw " << this << " block_signal" << endl;
    fl_clip_box(x(),y(),w(),h(),clip_x,clip_y,clip_w,clip_h);
    fl_push_clip(clip_x,clip_y,clip_w,clip_h);
    int horizontal_pixels=w()-(show_axes?int(ylegende*labelsize()):0);
    int vertical_pixels;
    int deltax=x(),deltay=y();
    in_draw(clip_x,clip_y,clip_w,clip_h,vertical_pixels);
    // Draw mouse drag
    if ( pushed && !waiting_click && current_i>=0 && current_i<=horizontal_pixels && current_j>=0 && current_j <=vertical_pixels && (push_i!=current_i || push_j!=current_j ) ){
      fl_color(FL_RED);
      check_fl_rect(deltax+min(push_i,current_i),deltay+min(push_j,current_j),absint(current_i-push_i),absint(current_j-push_j),clip_x,clip_y,clip_w,clip_h,0,0);
    }
    fl_pop_clip();
    if (!paused)
      ++animation_instructions_pos;
    // cerr << "graph2d draw " << this << " restore block_signal" << endl;
    block_signal=block;
    io_graph(b,contextptr);
#ifdef HAVE_LIBPTHREAD
    pthread_mutex_unlock(&interactive_mutex);
    // cerr << "graph2d draw unlock" << endl;
#endif
  }

  void Graph2d3d::autoname_plus_plus(){
    if (hp){
      string s=autoname(hp->contextptr);
      giac::autoname_plus_plus(s);
      autoname(s,hp->contextptr);
    }
  }

  void Graph2d3d::update(Fl_Group * pack,int pos){
    if (!pack)
      return;
    show();
    if (pos>=0){ // Check if we need to update from pos or pos+1
      if ( (last_event=FL_DRAG || last_event==FL_RELEASE) && pack->children()>pos){
	Gen_Value_Slider * gs=dynamic_cast<Gen_Value_Slider *>(Fl::belowmouse());
	if (gs){ // last handle required by a cursor? 
	  Fl_Widget * wid=pack->child(pos);
	  Fl_Tile * t=dynamic_cast<Fl_Tile *>(wid);
	  if (t && t->children()>=3)
	    wid=t->child(2);
	  Fl_Scroll * s =dynamic_cast<Fl_Scroll *>(wid);
	  if (s)
	    wid=s->child(0);
	  if (const Gen_Output * go = dynamic_cast<const Gen_Output *>(wid)){
	    if (go->value().is_symb_of_sommet(at_parameter))
	      ++pos;
	  }
	  // cerr << clock() << " ++pos " << pos << " " << last_event << endl;
	}
	else
	  ; // cerr << clock() << " =pos " << pos << " " << last_event <<  endl;
      }
    }
    else
      ++pos;
    // pack is normally a History_Pack
    clear(pos);
    add(pack,pos);
  }

  void Graph2d3d::add(const Fl_Widget * wid,int pos){
    if (const Gen_Output * g = dynamic_cast<const Gen_Output *>(wid)){
      add(g->value());
      return;
    }
    if (const Fl_Group * g = dynamic_cast<const Fl_Group *>(wid)){
      int n=g->children();
      for (int i=pos;i<n;++i){
	const Fl_Widget * wid = g->child(i);
	add(wid);
      }
    }
  }

  void Graph2d3d::clear(unsigned pos){
    redraw();
    // Clear plot_instructions starting at pos+1,
    // clear parameters starting at pos+1
    if (pos<plot_instructions.size())
      plot_instructions.erase(plot_instructions.begin()+pos,plot_instructions.end()); 
    if (param_group){ 
      param_group->redraw();
      int n=param_group->children(),i=0;
      for (;i<n;++i){
	if (Gen_Value_Slider * gvs=dynamic_cast<Gen_Value_Slider *>(param_group->child(i))){
	  if (gvs->pos>=int(pos))
	    break;
	}
      }
      for (;i<n;--n){
	param_group->remove(param_group->child(n-1));
      }
    }
  }

  int Figure::handle(int event){
    if (event==FL_MOUSEWHEEL){
      if (!Fl::event_inside(this))
	return 0;
      if (Fl::focus()==geo){
	int res=geo->handle(event);
	if (res)
	  return res;
      }
      if (!Fl::event_inside(s))
	return 0;      
      return s->child(2)->handle(event);
    }
    return Fl_Tile::handle(event);
  }

  void Figure::rename(const string & s){
    if (namestr)
      delete namestr;
    string s1="Save "+s;
    namestr = new char[s1.size()+1];
    strcpy(namestr,s1.c_str());
    name->label(namestr);
    name->show();
  }

  void Figure::save_figure_as(const string & s_orig){
    if (!geo->hp)
      return;
    string s(s_orig);
    if (s.empty()){
      // Get filename
      for (;;){
	char * newfile = file_chooser(gettext("Save figure"), "*.cas", "session.cas");
	if ( (!newfile) || (!*newfile))
	  return;
	s=newfile; // remove_path(newfile);
	//s=remove_path(remove_extension(s.substr(0,1000).c_str()))+".cas";
	s=remove_extension(s.substr(0,1000).c_str())+".cas";
	if (access(s.c_str(),R_OK))
	  break;
	int i=fl_ask("%s",(s+gettext(": file exists. Overwrite?")).c_str());
	if (i==1)
	  break;
      }
    }
    rename(s);
    ofstream of(s.c_str());
    int n=geo->hp->children();
    // geo->hp->clear_modified();
    for (int i=0;i<n;i++){
      Fl_Widget * w = geo->hp->child(i);
      if (Fl_Group * g = dynamic_cast<Fl_Group *>(w)){
	if (g->children()){
	  w=g->child(0);
	  if (Fl_Scroll * s =dynamic_cast<Fl_Scroll *>(w))
	    w=s->child(0);
	  if (Multiline_Input_tab * m=dynamic_cast<Multiline_Input_tab *>(w)){
	    if (strlen(m->value()))
	      of << replace(m->value(),'\n',' ')+";" << endl;
	  }
	  if (Xcas_Text_Editor * m=dynamic_cast<Xcas_Text_Editor *>(w)){
	    string s=m->value();
	    if (!s.empty())
	      of << replace(s,'\n',' ')+";" << endl;
	  }
	}
      }
    }
  }

  std::string Figure::latex_save_figure(){
    static int nsession=0;
    nsession++;
    string thename="session"+print_INT_(nsession)+".tex";
    if (strlen(this->name->label())){
      thename=remove_extension(remove_path(this->name->label()))+".tex";
    }
    return geo->latex(thename.c_str());
  }
  
  static void cb_Figure_Save(Fl_Widget * m , void*) {
    Figure * f=find_figure(m);
    if (f){
      if (f->namestr){
	string tmp(f->namestr);
	f->save_figure_as(tmp.substr(5,tmp.size()-5));
      }
      else
	f->save_figure_as("");
    }
  }

  static void cb_Figure_Save_as(Fl_Widget * m , void*) {
    Figure * f=find_figure(m);
    if (f)
      f->save_figure_as("");
  }

  std::string figure_insert(Figure * f){
    if (!f->geo->hp)
      return "";
    if (f){
      char * newfile = load_file_chooser(gettext("Insert figure"), "*.cas", "",0,false);
      if ( file_not_available(newfile))
	return "";
      // Put newfile in selection and paste to f->geo->hp
      FILE * fich =fopen(newfile,"r");
      string s; char ch;
      for (;fich;){
	ch=fgetc(fich);
	if (feof(fich))
	  break;
	s += ch;
      }
      Fl::selection(*f,s.c_str(),s.size());
      Fl::focus(f->geo->hp);
      Fl::paste(*f->geo->hp);
      return newfile;
    }
    return "";
  }

  static void cb_Figure_Insert(Fl_Widget * m , void*) {
    Figure * f=find_figure(m);
    if (!f) return;
    figure_insert(f);
  }

  static void cb_Figure_Preview(Fl_Widget * m , void*) {
    Figure * f=find_figure(m);
    if (f)
      widget_ps_print(f->geo,remove_path(f->name->label()),true);
  }

  static void cb_Figure_Print(Fl_Widget * m , void*) {
    Figure * f=find_figure(m);
    if (f)
      widget_print(f->geo);
  }

  static void cb_Figure_LaTeX_Preview(Fl_Widget * m , void*) {
    Figure * f=find_figure(m);
    if (f){
      string name=f->latex_save_figure();
      xdvi(name);
    }
  }

  static void cb_Figure_LaTeX_Print(Fl_Widget * m , void*) {
    Figure * f=find_figure(m);
    if (f){
      string name=f->latex_save_figure();
      dvips(name);
    }
  }


  int figure_param_dialog(Fl_Widget * f,bool adjust,double & tmin,double & tmax,double & tcurrent,double & tstep,string & name,bool symb,string & tmp){
    static Fl_Window * w = 0;
    static Fl_Value_Input * wxmin=0, * wxmax=0, *wcurrent=0,*wstep=0; 
    static Fl_Input * wname=0;
    static Fl_Check_Button * wsymb = 0 ; 
    static Fl_Return_Button * button0 = 0 ; // ok
    static Fl_Button * button1 =0; // cancel
    static string paramname;
    if (f){
      if (!w){
	int l=f->labelsize();
	int dx=18*l,dy=6*l,dh=dy/3;
	Fl_Group::current(0);
	w=new Fl_Window(dx,dy);
	wxmin=new Fl_Value_Input(dx/9,2,2*dx/9-2,dh-4,"t-");
	wxmin->tooltip(gettext("Parameter minimal value"));
	wxmin->value(-5);
	wxmax=new Fl_Value_Input(dx/3+dx/9,2,2*dx/9-2,dh-4,"t+");
	wxmax->tooltip(gettext("Parameter maximal value"));
	wxmax->value(5);
	wcurrent=new Fl_Value_Input(2*dx/3+dx/9,2,2*dx/9-2,dh-4,"t=");
	wcurrent->tooltip(gettext("Parameter current value"));
	wstep=new Fl_Value_Input(dx/6,2+dh,dx/6-2,dh-4,"step");
	wstep->tooltip(gettext("Parameter step value"));
	wstep->minimum(0);
	wstep->maximum(1000);
	wstep->step(0.001);
	wstep->value(0.1);
	wname=new Fl_Input(dx/2,2+dh,dx/6-2,dh-4,"name");
	wname->tooltip(gettext("Parameter variable name"));
	paramname="a";
	wsymb=new Fl_Check_Button(2*dx/3+l+5,2+dh,l,dh-4,"symb");
	wsymb->tooltip(gettext("Numeric or formal parameter"));
	wsymb->value(true);
	button0 = new Fl_Return_Button(2,2+2*dh,dx/2-4,dh-4);
	button0->shortcut(0xff0d);
	button0->label(gettext("OK"));
	button1 = new Fl_Button(dx/2+2,2+ 2*dh,dx/2-4,dh-4);
	button1->shortcut(0xff1b);
	button1->label(gettext("Cancel"));
	w->end();
	change_group_fontsize(w,l);
	w->resizable(w);
	w->label(gettext("Parameter definition"));
      }
      if (adjust){
	wxmin->value(tmin);
	wxmax->value(tmax);
	wcurrent->value(tcurrent);
	wstep->value(tstep);
	paramname=name;
	wsymb->value(symb);
      }
      wname->value(paramname.c_str());
      w->set_modal();
      w->show();
      autosave_disabled=true;
      w->hotspot(w);
      Fl::focus(wxmin);
      int r;
      for (;;) {
	Fl_Widget *o = Fl::readqueue();
	if (!o) Fl::wait();
	else {
	  if (o == button0) {r = 0; break;}
	  if (o == button1) {r = 1; break;}
	  if (o == w) { r=1; break; }
	}
      }
      w->hide();
      autosave_disabled=false;
      if (!r){ // insert parameter in figure pack
	autoname_plus_plus(paramname);
	name=wname->value();
	tmin=wxmin->value();
	tmax=wxmax->value();
	tcurrent=wcurrent->value();
	tstep=wstep->value();
	if (wsymb->value())
	  tmp="assume("+string(wname->value())+"=["+print_DOUBLE_(tcurrent)+","+print_DOUBLE_(tmin)+","+print_DOUBLE_(tmax)+","+print_DOUBLE_(tstep)+"])";
	else
	  tmp=string(wname->value())+" := element("+print_DOUBLE_(tmin)+".."+print_DOUBLE_(tmax)+","+print_DOUBLE_(tcurrent)+","+print_DOUBLE_(tstep)+")";
	return 1;
      }
    } // end if (f)
    return 0;
  }

  static void cb_Figure_Parameter(Fl_Widget * m , void*) {
    Fl_Widget * wid=Fl::focus();
    Figure * f=find_figure(m);
    if (f){
      if (!f->geo->hp)
	return;
      string tmp,name;
      double tmin,tmax,tcur,tstep;
      if (figure_param_dialog(f,false,tmin,tmax,tcur,tstep,name,false,tmp)){
	History_Pack * hp=f->geo->hp;
	int pos;
	if (hp!=get_history_pack(wid,pos))
	  pos=hp->children()-1;
	hp->add_entry(pos);
	hp->set_value(pos,tmp,true);
      }
    }
  }


  // modeplot==0 (paramplot2d), 1 (paramplot3d), 2 (plotfield), 3 (implicit), -1 (polarplot)
  int plotparam_dialog(Fl_Widget * spread_ptr,std::string & arg,int modeplot){
    static Fl_Window * w = 0;
    static Fl_Input *fcnxt=0, *fcnyt=0, *fcnrhot=0, *fcnxuv=0, *fcnyuv=0, *fcnzuv=0, *fcnfield=0, *fcnimplicit=0,*varnamet=0, * varnameu=0, * varnamev=0, *varnametfield=0,*varnameyfield=0,*varnamex=0,*varnamey=0; 
    static Fl_Value_Input * tmin=0,*tstep=0,*tmax=0;
    static Fl_Value_Input * umin=0,*ustep=0,*umax=0;
    static Fl_Value_Input * vmin=0,*vstep=0,*vmax=0;
    static Fl_Value_Input * xmin=0,*xstep=0,*xmax=0;
    static Fl_Value_Input * ymin=0,*ystep=0,*ymax=0;
    static Fl_Return_Button * button0 = 0 ;
    static Fl_Button * button1 =0;
    static Fl_Check_Button * do_plotfield = 0,*do_normal=0;
    static Line_Type *ltres=0;
    // static int curvestyle=0;
    Graph2d3d * gr = dynamic_cast<Graph2d3d *>(spread_ptr);
    if (!gr){
      if (Figure * fig=dynamic_cast<Figure *>(spread_ptr))
	gr=fig->geo;
    }
    if (!w){
#ifdef IPAQ
      int dx=240,dy=300;
#else
      int dx=(spread_ptr?30*spread_ptr->labelsize():500), 
	dy=spread_ptr?12*spread_ptr->labelsize():400;
#endif
      int lignes=5;
      Fl_Group::current(0);
      w=new Fl_Window(dx,dy);
      int l=spread_ptr->labelsize();
      ltres = new Line_Type(2,2,l,l,_MAGENTA+_FILL_POLYGON);
      ltres->show_pnt(true);
      ltres->show_poly(true);
      do_plotfield= new Fl_Check_Button (l+2,2,dx/6-4-l,dy/lignes-4,"Field");
      do_plotfield->value(true);
      do_plotfield->tooltip(gettext("Draw slopefield"));
      do_normal= new Fl_Check_Button (2+dx/6,2,dx/6-4,dy/lignes-4,"||=1");
      do_normal->value(true);
      do_normal->tooltip(gettext("Normalize slopefield"));
      fcnimplicit=new Fl_Input(dx/2,2,dx/2-4,dy/lignes-4,gettext("F(x,y)="));
      fcnimplicit->value("x^4+y^4+x*y=25");
      fcnimplicit->tooltip(gettext("Implicit expression"));
      fcnfield=new Fl_Input(dx/2,2,dx/2-4,dy/lignes-4,gettext("dy/dt(t,y)="));
      fcnfield->value("sin(t*y)");
      fcnfield->tooltip(gettext("Expression of dy/dt in terms of y and t, e.g. sin(t*y)"));
      fcnrhot=new Fl_Input(dx/2,2,dx/2-4,dy/lignes-4,gettext("rho(t)="));
      fcnrhot->value("2*t");
      fcnrhot->tooltip(gettext("Expression of modulus rho wrt to angle t (e.g sin(t))"));
      fcnxt=new Fl_Input(dx/4,2,dx/4-4,dy/lignes-4,gettext("x(t)="));
      fcnxt->value("sin(2*t)");
      fcnxt->tooltip(gettext("Expression of x wrt to t (e.g sin(t))"));
      fcnyt=new Fl_Input(3*dx/4,2,dx/4-4,dy/lignes-4,gettext("y(t)="));
      fcnyt->value("cos(3*t)");
      fcnyt->tooltip(gettext("Expression of y wrt to t (e.g cos(t))"));
      fcnxuv=new Fl_Input(dx/6,2,dx/6-4,dy/lignes-4,gettext("x="));
      fcnxuv->value("u+v^2");
      fcnxuv->tooltip(gettext("Expression of x wrt to u and v (e.g. u+v)"));
      fcnyuv=new Fl_Input(3*dx/6,2,dx/6-4,dy/lignes-4,gettext("y="));
      fcnyuv->value("cos(u)+v");
      fcnyuv->tooltip(gettext("Expression of y wrt to u and v (e.g exp(u)-v)"));
      fcnzuv=new Fl_Input(5*dx/6,2,dx/6-4,dy/lignes-4,gettext("y="));
      fcnzuv->value("sin(u)+v");
      fcnzuv->tooltip(gettext("Expression of z wrt to u and v (e.g u-v)"));
      varnamet=new Fl_Input(dx/2,2+dy/lignes,dx/2-4,dy/lignes-4,gettext("Var"));
      varnamet->value("t");
      varnamet->tooltip(gettext("Independant variable name (e.g t)"));
      varnameu=new Fl_Input(dx/4,2+dy/lignes,dx/4-4,dy/lignes-4,gettext("1st var"));
      varnameu->value("u");
      varnameu->tooltip(gettext("First independant variable name (e.g u)"));
      varnamev=new Fl_Input(3*dx/4,2+dy/lignes,dx/4-4,dy/lignes-4,gettext("2nd var"));
      varnamev->value("v");
      varnamev->tooltip(gettext("Second independant variable name (e.g v)"));
      varnamex=new Fl_Input(dx/4,2+dy/lignes,dx/4-4,dy/lignes-4,gettext("1st var"));
      varnamex->value("x");
      varnamex->tooltip(gettext("First independant variable name (e.g x)"));
      varnamey=new Fl_Input(3*dx/4,2+dy/lignes,dx/4-4,dy/lignes-4,gettext("2nd var"));
      varnamey->value("y");
      varnamey->tooltip(gettext("Second independant variable name (e.g y)"));
      varnametfield=new Fl_Input(dx/4,2+dy/lignes,dx/4-4,dy/lignes-4,gettext("Time var"));
      varnametfield->value("t");
      varnametfield->tooltip(gettext("Time variable name"));
      varnameyfield=new Fl_Input(3*dx/4,2+dy/lignes,dx/4-4,dy/lignes-4,gettext("2nd var"));
      varnameyfield->value("y");
      varnameyfield->tooltip(gettext("Function variable name"));
      tmin = new Fl_Value_Input(dx/6,2+2*dy/lignes,dx/6-2,dy/lignes-4,gettext("tmin"));
      tmin->value(-3.14);
      tmin->step(0.5);
      tstep = new Fl_Value_Input(3*dx/6,2+2*dy/lignes,dx/6-2,dy/lignes-4,gettext("tstep"));
      tstep->value(0.1);
      tstep->step(0.01);
      tmax = new Fl_Value_Input(5*dx/6,2+2*dy/lignes,dx/6-2,dy/lignes-4,gettext("tmax"));
      tmax->value(3.14);
      tmax->step(0.5);
      umin = new Fl_Value_Input(dx/6,2+2*dy/lignes,dx/6-2,dy/lignes-4,gettext("umin"));
      umin->value(-3);
      umin->step(0.5);
      ustep = new Fl_Value_Input(3*dx/6,2+2*dy/lignes,dx/6-2,dy/lignes-4,gettext("ustep"));
      ustep->value(0.25);
      ustep->step(0.01);
      umax = new Fl_Value_Input(5*dx/6,2+2*dy/lignes,dx/6-2,dy/lignes-4,gettext("umax"));
      umax->value(3);
      umax->step(0.5);
      vmin = new Fl_Value_Input(dx/6,2+3*dy/lignes,dx/6-2,dy/lignes-4,gettext("vmin"));
      vmin->value(-3);
      vmin->step(0.5);
      vstep = new Fl_Value_Input(3*dx/6,2+3*dy/lignes,dx/6-2,dy/lignes-4,gettext("vstep"));
      vstep->value(0.25);
      vstep->step(0.01);
      vmax = new Fl_Value_Input(5*dx/6,2+3*dy/lignes,dx/6-2,dy/lignes-4,gettext("vmax"));
      vmax->value(3);
      vmax->step(0.5);
      xmin = new Fl_Value_Input(dx/6,2+2*dy/lignes,dx/6-2,dy/lignes-4,gettext("xmin"));
      xmin->value(-5);
      xmin->step(0.5);
      xstep = new Fl_Value_Input(3*dx/6,2+2*dy/lignes,dx/6-2,dy/lignes-4,gettext("xstep"));
      xstep->value(0.25);
      xstep->step(0.01);
      xmax = new Fl_Value_Input(5*dx/6,2+2*dy/lignes,dx/6-2,dy/lignes-4,gettext("xmax"));
      xmax->value(5);
      xmax->step(0.5);
      ymin = new Fl_Value_Input(dx/6,2+3*dy/lignes,dx/6-2,dy/lignes-4,gettext("ymin"));
      ymin->value(-5);
      ymin->step(0.5);
      ystep = new Fl_Value_Input(3*dx/6,2+3*dy/lignes,dx/6-2,dy/lignes-4,gettext("ystep"));
      ystep->value(0.25);
      ystep->step(0.01);
      ymax = new Fl_Value_Input(5*dx/6,2+3*dy/lignes,dx/6-2,dy/lignes-4,gettext("ymax"));
      ymax->value(5);
      ymax->step(0.5);
      button0 = new Fl_Return_Button(2,2+4*dy/lignes,dx/2-4,dy/lignes-4);
      button0->shortcut(0xff0d);
      button0->label(gettext("OK"));
      button1 = new Fl_Button(dx/2+2,2+4*dy/lignes,dx/2-4,dy/lignes-4);
      button1->shortcut(0xff1b);
      button1->label(gettext("Cancel"));
      w->end();
      change_group_fontsize(w,spread_ptr?spread_ptr->labelsize():14);
      w->resizable(w);
    }
    switch (modeplot){
    case -1:
      w->label(gettext("Polar plot"));
      break;
    case 0: case 1:
      w->label(gettext("Parametric plot"));
      break;
    case 2:
      w->label(gettext("Field/Odeplot"));
      break;
    case 3:
      w->label(gettext("Implicitplot"));
      break;
    }
    fcnxt->hide();
    fcnyt->hide();
    fcnrhot->hide();
    fcnxuv->hide();
    fcnyuv->hide();
    fcnzuv->hide();
    fcnfield->hide();
    fcnimplicit->hide();
    varnamet->hide();
    varnameu->hide();
    varnamev->hide();
    varnamex->hide();
    varnamey->hide();
    varnametfield->hide();
    varnameyfield->hide();
    tmax->hide();
    tmin->hide();
    tstep->hide();
    umax->hide();
    umin->hide();
    ustep->hide();
    vmax->hide();
    vmin->hide();
    vstep->hide();
    xmax->hide();
    xmin->hide();
    xstep->hide();
    ymax->hide();
    ymin->hide();
    ystep->hide();
    do_plotfield->hide();
    do_normal->hide();
    if (modeplot>=2){
      xmin->show();
      xstep->show();
      xmax->show();
      ymin->show();
      ystep->show();
      ymax->show();
      if (modeplot==2){
	varnametfield->show();
	varnameyfield->show();
	fcnfield->show();
	do_plotfield->show();
	do_normal->show();
      }
      else {
	varnamex->show();
	varnamey->show();
	fcnimplicit->show();
      }
    }
    if (modeplot==1){
      umax->show();
      umin->show();
      ustep->show();
      vmax->show();
      vmin->show();
      vstep->show();
      varnameu->show();
      varnamev->show();
      fcnxuv->show();
      fcnyuv->show();
      fcnzuv->show();
    }
    if (modeplot<=0) {
      tmax->show();
      tmin->show();
      tstep->show();
      varnamet->show();
      if (modeplot==-1){
	fcnrhot->show();
	tmin->value(0.0);
      }
      else {
	fcnxt->show();
	fcnyt->show();
      }
    }
    if (gr) {
      if (!gr->fcnfield.empty())
	fcnfield->value(gr->fcnfield.c_str());
      xmin->value(gr->window_xmin);
      xmax->value(gr->window_xmax);
      xstep->value((xmax->value()-xmin->value())/16);
      ymin->value(gr->window_xmin);
      ymax->value(gr->window_xmax);
      ystep->value((xmax->value()-xmin->value())/16);
    }
    int r=-1;
    w->set_modal();
    w->show();
    autosave_disabled=true;
    w->hotspot(w);
    Fl::focus(button0);
    for (;;) {
      Fl_Widget *o = Fl::readqueue();
      if (!o) Fl::wait();
      else {
	if (o==ltres){
	  int i=ltres->line_type();
	  bool formel=false,untranslate=false,approx=false;
	  change_line_type(i,true,approx,"",fcnzuv->visible(),formel,untranslate,false,spread_ptr?spread_ptr->labelsize():14);
	  ltres->line_type(i);
	}
	if (o == button0) {r = 0; break;}
	if (o == button1) {r = 1; break;}
	if (o == w) { r=1; break; }
      }
    }
    autosave_disabled=false;
    w->hide();
    if (!r){
      if (modeplot==3){
	arg=string(fcnimplicit->value())+",["+string(varnamex->value())+"="+print_DOUBLE_(xmin->value())+".."+print_DOUBLE_(xmax->value()) + string(",")+varnamey->value()+string("=")+print_DOUBLE_(ymin->value())+".."+print_DOUBLE_(ymax->value())+"],xstep="+print_DOUBLE_(xstep->value())+",ystep="+print_DOUBLE_(ystep->value())+",display="+print_color(ltres->line_type());
	return 1;
      }
      if (modeplot==2){
	if (gr){
	  gr->fcnfield=fcnfield->value();
	  gr->fcnvars="["+string(varnametfield->value())+","+string(varnameyfield->value())+"]";
	}
	arg=string(fcnfield->value())+",["+string(varnametfield->value())+"="+print_DOUBLE_(xmin->value())+".."+print_DOUBLE_(xmax->value()) + string(",")+varnameyfield->value()+string("=")+print_DOUBLE_(ymin->value())+".."+print_DOUBLE_(ymax->value())+"]";
	if (do_normal->value())
	  arg+=",normalize";
	arg+= ",xstep="+print_DOUBLE_(xstep->value())+",ystep="+print_DOUBLE_(ystep->value())+",display="+print_color(ltres->line_type());
	return do_plotfield->value()?2:1;
      }
      if (modeplot==1) {
	arg = "["+string(fcnxuv->value())+","+string(fcnyuv->value())+","+string(fcnzuv->value())+"]";
	arg += ",\n["+string(varnameu->value())+"="+print_DOUBLE_(umin->value())+".."+print_DOUBLE_(umax->value()) + string(",")+varnamev->value()+string("=")+print_DOUBLE_(vmin->value())+".."+print_DOUBLE_(vmax->value())+"],\nustep="+print_DOUBLE_(ustep->value())+",vstep="+print_DOUBLE_(vstep->value())+",display="+print_color(ltres->line_type());
	return 1;
      }
      if (modeplot==-1)
	arg = string(fcnrhot->value())+",";
      else
	arg = "["+string(fcnxt->value())+","+string(fcnyt->value())+"]\n,";
      arg += string(varnamet->value())+"="+print_DOUBLE_(tmin->value())+".."+print_DOUBLE_(tmax->value())+",tstep="+print_DOUBLE_(tstep->value())+",display="+print_color(ltres->line_type());
      return 1;
    }
    return 0;
  }

  void multiline_focus_error(){
    fl_alert("%s",gettext("Click first in a commandline"));
  }

  static void cb_Figure_Function(Fl_Widget * m , void*) {
    Fl_Widget * wid=Xcas_input_focus;
    Figure * f=find_figure(wid);
    if (f){
      string arg;
      bool dim3=dynamic_cast<Graph3d *>(f->geo);
      if (f && tablefunc_dialog(f,arg,true,(dim3?1:0),gettext("Graph of a function"))){
	History_Pack * hp=f->geo->hp;
	int pos;
	if (hp!=get_history_pack(wid,pos))
	  pos=hp->children()-1;
	hp->add_entry(pos);
	// arg=autoname(hp->contextptr)+(dim3?":= plotfunc(":":=plot(")+arg+")";
	arg=autoname(hp->contextptr)+":=plot("+arg+")";
	f->geo->autoname_plus_plus();
	hp->set_value(pos,arg,true);
      }
      else Fl::focus(wid);
    }
  }

  static void cb_Figure_helpon(Fl_Widget * m ,const string & cmd) {
    Fl_Widget * wid=Xcas_input_focus;
    Figure * f=find_figure(wid);
    if (f){
      string ans;
      int remove=0;
      int r;
      if (do_helpon)
	r=handle_tab(cmd,*giac::vector_completions_ptr(),2*f->w()/3,2*f->h()/3,remove,ans);
      else {
	ans=cmd+"()";
	r=1;
      }
      if (r){
	if (!f->geo->hp)
	  return;
	History_Pack * hp=f->geo->hp;
	int pos;
	if (hp!=get_history_pack(wid,pos))
	  pos=hp->children()-1;
	hp->add_entry(pos);
	hp->set_value(pos,ans,false);
      }
      else Fl::focus(wid);
    }	
  }

  static void cb_Figure_Tangent(Fl_Widget * m , void*) {
    // cb_set_mode(m,at_segment,at_tangent,2,gettext("Curve,Point"));
    cb_Figure_helpon(m,"tangent");
  }

  static void cb_Figure_Implicit(Fl_Widget * m , void*) {
    Fl_Widget * wid=Xcas_input_focus;
    Figure * f=find_figure(wid);
    if (f){
      if (!f->geo->hp)
	return;
      string arg;
      bool dim3=dynamic_cast<Graph3d *>(f->geo);
      if (dim3)
	fl_message("%s","Currently limited to 2-d");
      else {
	if (f && plotparam_dialog(f,arg,3)){
	  History_Pack * hp=f->geo->hp;
	  int pos;
	  if (hp!=get_history_pack(wid,pos))
	    pos=hp->children()-1;
	  hp->add_entry(pos);
	  arg=autoname(hp->contextptr)+":=plotimplicit("+arg+")";
	  f->geo->autoname_plus_plus();
	  hp->set_value(pos,arg,true);
	}
	else Fl::focus(wid);	
      }
    }
  }

  static void cb_Figure_Plotfield(Fl_Widget * m , void*) {
    Fl_Widget * wid=Xcas_input_focus;
    Figure * f=find_figure(wid);
    if (f){
      if (!f->geo->hp)
	return;
      string arg;
      bool dim3=dynamic_cast<Graph3d *>(f->geo);
      int res;
      if (dim3)
	fl_message("%s","Currently limited to 2-d");
      else {
	if (f && (res=plotparam_dialog(f,arg,2))){
	  if (res==2){
	    History_Pack * hp=f->geo->hp;
	    int pos;
	    if (hp!=get_history_pack(wid,pos))
	      pos=hp->children()-1;
	    hp->add_entry(pos);
	    arg="plotfield("+arg+")";
	    hp->set_value(pos,arg,true);
	  }
	  cb_set_mode(f,0,at_plotode,1,gettext("Point_on_ode_curve"));
	  fl_message("%s",gettext("Click initials conditions in graphic window. Change mode when finished."));
	}
	else Fl::focus(wid);
      }
    }
  }

  static void cb_plotode(Fl_Widget * m , void*) {
    Figure * f=find_figure(m);
    if (f){
      if (f->geo->fcnfield.empty())
	cb_Figure_Plotfield(m,0);
      else
	cb_set_mode(m,0,at_plotode,1,gettext("Point_on_ode_curve"));
    }
  }

  static void cb_Figure_Param(Fl_Widget * m , void*) {
    Fl_Widget * wid=Xcas_input_focus;
    Figure * f=find_figure(wid);
    if (f){
      if (!f->geo->hp)
	return;
      string arg;
      bool dim3=dynamic_cast<Graph3d *>(f->geo);
      if (f && plotparam_dialog(f,arg,(dim3?1:0))){
	History_Pack * hp=f->geo->hp;
	int pos;
	if (hp!=get_history_pack(wid,pos))
	  pos=hp->children()-1;
	hp->add_entry(pos);
	arg=autoname(hp->contextptr)+":=plotparam("+arg+")";
	f->geo->autoname_plus_plus();
	hp->set_value(pos,arg,true);
      }
      else Fl::focus(wid);
    }
  }

  static void cb_Figure_Polar(Fl_Widget * m , void*) {
    Fl_Widget * wid=Xcas_input_focus;
    Figure * f=find_figure(wid);
    if (f){
      if (!f->geo->hp)
	return;
      string arg;
      bool dim3=dynamic_cast<Graph3d *>(f->geo);
      if (dim3)
	fl_alert("%s","Not a 3-d graph");
      else {
	if (f && plotparam_dialog(f,arg,-1)){
	  History_Pack * hp=f->geo->hp;
	  int pos;
	  if (hp!=get_history_pack(wid,pos))
	    pos=hp->children()-1;
	  hp->add_entry(pos);
	  arg=autoname(hp->contextptr)+":=plotpolar("+arg+")";
	  f->geo->autoname_plus_plus();
	  hp->set_value(pos,arg,true);
	}	
	else Fl::focus(wid);
      }
    }
  }

  static void cb_Figure_Plotarea(Fl_Widget * m , void*) {
    Fl_Widget * wid=Xcas_input_focus;
    Figure * f=find_figure(wid);
    if (f){
      if (!f->geo->hp)
	return;
      string arg;
      bool dim3=dynamic_cast<Graph3d *>(f->geo);
      if (dim3)
	fl_alert("%s","Not a 3-d graph");
      else {
	if (f && tablefunc_dialog(f,arg,true,2,gettext("Shade area under a curve"))){
	  History_Pack * hp=f->geo->hp;
	  int pos;
	  if (hp!=get_history_pack(wid,pos))
	    pos=hp->children()-1;
	  hp->add_entry(pos);
	  arg="plotarea("+arg+")";
	  hp->set_value(pos,arg,true);
	}
	else Fl::focus(wid);
      }
    }
  }

  static void cb_Figure_Sequence(Fl_Widget * m , void*) {
    Fl_Widget * wid=Xcas_input_focus;
    Figure * f=find_figure(wid);
    if (f){
      if (!f->geo->hp)
	return;
      string arg,u0param;
      bool dim3=dynamic_cast<Graph3d *>(f->geo);
      if (dim3)
	fl_alert("%s","Not a 2-d graph");
      else {
	if (f && tableseq_dialog(f,arg,true,gettext("Graph of a recurrent sequence"),u0param)){
	  History_Pack * hp=f->geo->hp;
	  int pos;
	  if (hp!=get_history_pack(wid,pos))
	    pos=hp->children()-1;
	  hp->add_entry(pos);
	  arg="plotseq("+arg+")";
	  hp->set_value(pos,arg,false);
	  hp->add_entry(pos);
	  hp->set_value(pos,u0param,true);
	}
	else Fl::focus(wid);
      }
    }
  }

  /* model for new callbacks
  static void cb_Figure(Fl_Widget * m , void*) {
    Figure * f=find_figure(m);
    if (f){
    }
  }
  */

  int contrast(Fl_Color c){
    if (c<=7)
      return 7-c;
    if (c>=8 && c<0x10)
      return 7;
    if (c>=0x10 && c<0x50)
      return 0xf8;
    if (c & 0x4)
      return 0;
    return 7;
  }

  static void cb_Figure_Color(Fl_Button * b , void*) {
    static string s;
    Figure * f=find_figure(b);
    if (f){
      Fl_Color col=b->color();
      col=fl_show_colormap(col);
      f->couleur->color(col);
      Fl_Color col2=Fl_Color(contrast(col));
      f->couleur->labelcolor(col2);
      s=col<1000?print_INT_(col):"";
      f->couleur->label(s.c_str());
      f->geo->couleur=(col&0xffff) | (f->geo->couleur & 0xffff0000);
      f->lt->line_type(f->geo->couleur);
    }
  }

  static void cb_Figure_Disposition(Fl_Button * b , void*) {
    Figure * fig=find_figure(b);
    if (!fig) return;
    fig->disposition=b->value();
    fig->resize(fig->x(),fig->y(),fig->w(),fig->h());
    fig->geo->orthonormalize();
    fig->redraw();
  }

  static void cb_Figure_Approx(Fl_Button * b , void*) {
    Figure * fig=find_figure(b);
    if (!fig) return;
    fig->geo->approx=b->value();
  }

  static void cb_Figure_Autoeval(Fl_Button * b , void*) {
    static string s;
    Figure * f=find_figure(b);
    if (!f) return;
    f->geo->hp->eval_below=!b->value();
    f->geo->clear();
    if (!f->geo->hp->children())
      return;
    f->geo->hp->focus(0,true);
  }

  static void cb_Exact(Fl_Button * b , void*) {
    Figure * f=find_figure(b);
    if (f){
      f->geo->approx=false;
    }
  }

  static void cb_Approx(Fl_Button * b , void*) {
    Figure * f=find_figure(b);
    if (f){
      f->geo->approx=true;
    }
  }

  static void cb_Figure_Line_Type(Fl_Widget * b , void*) {
    static string s;
    Figure * f=find_figure(b);
    if (f){
      int i=f->lt->line_type();
      bool formel=false,untranslate=false;
      change_line_type(i,true,f->geo->approx," (next mouse created objects)",(dynamic_cast<Graph3d *>(f->geo)),formel,untranslate,false,f->geo->labelsize());
      Fl_Color col = Fl_Color(i&0xffff);
      f->couleur->color(col);
      Fl_Color col2=Fl_Color(contrast(col));
      f->couleur->labelcolor(col2);
      s=col<1000?print_INT_(col):"";
      f->couleur->label(s.c_str());
      f->lt->line_type(i);
      f->geo->couleur=i;
    }
  }

  static void cb_Figure_Portrait(Fl_Widget * b , void*) {
    Figure * f=find_figure(b);
    if (f){
      f->disposition=0;
      f->resize(f->x(),f->y(),f->w(),f->h());
      f->geo->orthonormalize();
      f->redraw();
    }
  }

  static void cb_Figure_Landscape(Fl_Widget * b , void*) {
    Figure * f=find_figure(b);
    if (f){
      f->disposition=1;
      f->resize(f->x(),f->y(),f->w(),f->h());
      f->geo->orthonormalize();
      f->redraw();
    }
  }

  static void cb_Figure_Focus(Fl_Widget * b , void*) {
    Figure * f=find_figure(b);
    if (f){
      f->geo->handle(FL_FOCUS);
    }
  }

  static void cb_Figure_Change_Attributs(Fl_Widget * b , void*) {
    Figure * f=find_figure(b);
    if (!f) return;
    if (!f->geo->hp)
      return;
    if (f && f->geo && f->geo->hp){
      int hp_pos=f->geo->hp->_sel_begin;
      f->geo->change_attributs(hp_pos);
    }
  }

  static void cb_NumericalEdit(Fl_Widget * b , void*) {
    Figure * f=find_figure(b);
    if (!f) return;
    if (!f->geo->hp)
      return;
    if (f && f->geo && f->geo->hp){
      History_Pack * hp =f->geo->hp;
      const char * ch=fl_input(gettext("Real number?"),"1");
      static string s;
      if (ch){
	double n=atof(ch);
	gen g=symbolic(at_legende,makevecteur(gen(f->geo->window_xmin+(f->geo->window_xmax-f->geo->window_xmin)/20,f->geo->window_ymin+(f->geo->window_ymax-f->geo->window_ymin)/20),n));
	gen pntname(autoname(hp->contextptr),hp->contextptr);
	g=symbolic(at_sto,gen(makevecteur(g,pntname),_SEQ__VECT));
	f->geo->autoname_plus_plus();
	f->geo->hp->set_gen_value(-1,g);
      }
    }
  }

  bool dim3(Fl_Widget * m){
    Figure * f=find_figure(m);
    if (f && f->geo)
      return dynamic_cast<Graph3d *>(f->geo);
    else
      return false;
  }

  static void cb_Frame(Fl_Widget * m , void*) {
    cb_set_mode(m,0,0,255," object_selects_level");
  }

  static void cb_Pointer(Fl_Widget * m , void*) {
    cb_set_mode(m,0,0,0," point_to_move");
  }

  static void cb_Point(Fl_Widget * m , void*) {
    cb_set_mode(m,0,at_point,1,"Point");
  }

  static void cb_Circle(Fl_Widget * m , void*) {
    cb_set_mode(m,at_segment,at_cercle,(dim3(m)?3:2),(dim3(m)?gettext("Center,Point_on_circle,Point_in_plane"):gettext("Center,Point_on_circle")));
  }

  static void cb_Inter_Unique(Fl_Widget * m , void*) {
    cb_set_mode(m,at_segment,at_inter_unique,2,gettext("Curve1,Curve2"));
  }

  static void cb_Inter(Fl_Widget * m , void*) {
    cb_set_mode(m,at_segment,at_inter,2,gettext("Curve1,Curve2"));
  }

  static void cb_Inter_3(Fl_Widget * m , void*) {
    cb_set_mode(m,at_polygone_ouvert,at_inter,3,gettext("Curve1,Curve2,Point"));
  }

  static void cb_Tangent(Fl_Widget * m , void*) {
    cb_set_mode(m,at_segment,at_tangent,2,gettext("Curve,Point"));
  }

  static void cb_Segment(Fl_Widget * m , void*) {
    cb_set_mode(m,at_segment,at_segment,2,gettext("Point1,Point2"));
  }

  static void cb_Vector(Fl_Widget * m , void*) {
    cb_set_mode(m,at_vector,at_vector,2,gettext("Point1,Point2"));
  }

  static void cb_Demi_Droite(Fl_Widget * m , void*) {
    cb_set_mode(m,at_demi_droite,at_demi_droite,2,gettext("Point1,Point2"));
  }

  static void cb_Droite(Fl_Widget * m , void*) {
    cb_set_mode(m,at_droite,at_droite,2,gettext("Point1,Point2"));
  }

  static void cb_DistanceAB(Fl_Widget * m , void*) {
    cb_set_mode(m,at_segment,at_distanceat,3,gettext("Object1,Object2,Position"));
  }
  static void cb_DistanceABraw(Fl_Widget * m , void*) {
    cb_set_mode(m,at_segment,at_distanceatraw,3,gettext("Object1,Object2,Position"));
  }

  static void cb_AngleABC(Fl_Widget * m , void*) {
    cb_set_mode(m,at_triangle,at_angleat,4,gettext("Angle_vertex,Direction1,Direction2,Position"));
  }
  static void cb_AngleABCraw(Fl_Widget * m , void*) {
    cb_set_mode(m,at_triangle,at_angleatraw,4,gettext("Angle_vertex,Direction1,Direction2,Position"));
  }

  static void cb_Arearaw(Fl_Widget * m , void*) {
    cb_set_mode(m,at_areaatraw,at_areaatraw,2,gettext("Object,Position"));
  }

  static void cb_Area(Fl_Widget * m , void*) {
    cb_set_mode(m,at_areaat,at_areaat,2,gettext("Object,Position"));
  }

  static void cb_Perimeterraw(Fl_Widget * m , void*) {
    cb_set_mode(m,at_perimeteratraw,at_perimeteratraw,2,gettext("Object,Position"));
  }

  static void cb_Perimeter(Fl_Widget * m , void*) {
    cb_set_mode(m,at_perimeterat,at_perimeterat,2,gettext("Object,Position"));
  }

  static void cb_Sloperaw(Fl_Widget * m , void*) {
    cb_set_mode(m,at_slopeatraw,at_slopeatraw,2,gettext("Object,Position"));
  }

  static void cb_Slope(Fl_Widget * m , void*) {
    cb_set_mode(m,at_slopeat,at_slopeat,2,gettext("Object,Position"));
  }

  static void cb_Symetrie(Fl_Widget * m , void*) {
    cb_set_mode(m,at_segment,at_symetrie,2,gettext("Symmetry_center_axis,Object"));
  }

  static void cb_Inversion(Fl_Widget * m , void*) {
    cb_set_mode(m,at_segment,at_inversion,3,gettext("Center,Ratio,Object"));
  }

  static void cb_Projection(Fl_Widget * m , void*) {
    cb_set_mode(m,at_segment,at_projection,2,gettext("Curve,Object"));
  }

  static void cb_Reciprocation(Fl_Widget * m , void*) {
    cb_set_mode(m,at_segment,at_polaire_reciproque,2,gettext("Circle,Object"));
  }

  static void cb_Rotation(Fl_Widget * m , void*) {
    cb_set_mode(m,at_polygone_ouvert,at_rotation,3,gettext("Center,Angle,Object"));
  }

  static void cb_Homothetie(Fl_Widget * m , void*) {
    cb_set_mode(m,at_polygone_ouvert,at_homothetie,3,gettext("Center,Ratio,Object"));
  }

  static void cb_Translation(Fl_Widget * m , void*) {
    cb_set_mode(m,at_segment,at_translation,2,gettext("Vector,Object"));
  }

  static void cb_Similitude(Fl_Widget * m , void*) {
    cb_set_mode(m,at_polygone_ouvert,at_similitude,4,gettext("Center,Ratio,Angle,Object"));
  }

  static void cb_Polygone(Fl_Widget * m , void*) {
    const char * ch=fl_input(gettext("Number of vertices?"),"5");
    static string s;
    if (ch){
      int n=atoi(ch);
      s="";
      if (n>=2){
	for (int i=1;i<=n;++i){
	  s+=gettext("Point")+print_INT_(i);
	  if (i<n)
	    s+=",";
	}
	cb_set_mode(m,at_polygone_ouvert,at_polygone,n,s);
      }
    }
  }

  static void cb_Triangle(Fl_Widget * m , void*) {
    cb_set_mode(m,at_segment,at_triangle,3,gettext("Point1,Point2,Point3"));
  }

  static void cb_Carre(Fl_Widget * m , void*) {
    cb_set_mode(m,at_carre,at_carre,2,gettext("Point1,Point2"));
  }

  static void cb_Mediatrice(Fl_Widget * m , void*) {
    cb_set_mode(m,at_segment,at_mediatrice,2,gettext("Point1,Point2"));
  }

  static void cb_Perpendiculaire(Fl_Widget * m , void*) {
    cb_set_mode(m,at_segment,at_perpendiculaire,2,gettext("Point,Line"));
  }

  static void cb_Parallele(Fl_Widget * m , void*) {
    cb_set_mode(m,at_segment,at_parallele,2,gettext("Point,Line"));
  }

  static void cb_Mediane(Fl_Widget * m , void*) {
    cb_set_mode(m,at_segment,at_mediane,3,gettext("Sommet_angle,Point2,Point3"));
  }

  static void cb_Bissectrice(Fl_Widget * m , void*) {
    cb_set_mode(m,at_segment,at_bissectrice,3,gettext("Sommet_angle,Point2,Point3"));
  }

  static void cb_Quadrilatere(Fl_Widget * m , void*) {
    cb_set_mode(m,at_polygone_ouvert,at_quadrilatere,4,gettext("Point1,Point2,Point3,Point4"));
  }

  static void cb_Triangle_Equilateral(Fl_Widget * m , void*) {
    cb_set_mode(m,at_segment,at_triangle_equilateral,(dim3(m)?3:2),(dim3(m)?gettext("Point1,Point2,Point_in_plane"):gettext("Point1,Point2")));
  }

  static void cb_inscrit(Fl_Widget * m , void*) {
    cb_set_mode(m,at_segment,at_inscrit,3,gettext("Point1,Point2,Point3"));
  }

  static void cb_exinscrit(Fl_Widget * m , void*) {
    cb_set_mode(m,at_segment,at_exinscrit,3,gettext("Point1,Point2,Point3"));
  }

  static void cb_circonscrit(Fl_Widget * m , void*) {
    cb_set_mode(m,at_segment,at_circonscrit,3,gettext("Point1,Point2,Point3"));
  }

  static void cb_ellipse(Fl_Widget * m , void*) {
    cb_set_mode(m,at_segment,at_ellipse,3,gettext("Focus1,Focus2,Point_on_ellipse"));
  }

  static void cb_parabole(Fl_Widget * m , void*) {
    cb_set_mode(m,at_segment,at_parabole,2,gettext("Focus,Point_or_line"));
  }

  static void cb_hyperbole(Fl_Widget * m , void*) {
    cb_set_mode(m,at_segment,at_hyperbole,3,gettext("Focus1,Focus2,Point_on_hyperbola"));
  }

  static void cb_plan(Fl_Widget * m , void*) {
    cb_set_mode(m,at_segment,at_plan,3,gettext("Point1,Point2,Point3"));
  }

  static void cb_sphere(Fl_Widget * m , void*) {
    cb_set_mode(m,at_sphere,at_sphere,2,gettext("Center,Point_on_sphere"));
  }

  Fl_Menu_Item Figure_menu[] = {
    {gettext("Fig"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("Save figure as text"), 0,  (Fl_Callback*)cb_Figure_Save, 0, 0, 0, 0, 14, 56},
    {gettext("Save as alternate filename"), 0,  (Fl_Callback*)cb_Figure_Save_as, 0, 0, 0, 0, 14, 56},
    {gettext("Insert"), 0,  (Fl_Callback*)cb_Figure_Insert, 0, 0, 0, 0, 14, 56},
    {gettext("Export Print"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("EPS PNG and preview"), 0,  (Fl_Callback*)cb_Figure_Preview, 0, 0, 0, 0, 14, 56},
    {gettext("to printer"), 0,  (Fl_Callback*)cb_Figure_Print, 0, 0, 0, 0, 14, 56},
    {gettext("latex preview"), 0,  (Fl_Callback*)cb_Figure_LaTeX_Preview, 0, 0, 0, 0, 14, 56},
    {gettext("latex printer"), 0,  (Fl_Callback*)cb_Figure_LaTeX_Print, 0, 0, 0, 0, 14, 56},
    {0}, // end print
    {0}, // end file
    {gettext("Edit"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("Add parameter"), 0,  (Fl_Callback *) cb_Figure_Parameter, 0, 0, 0, 0, 14, 56},
    {gettext("Add numerical value"), 0,  (Fl_Callback *) cb_NumericalEdit, 0, 0, 0, 0, 14, 56},
    {gettext("Add new entry"), 0x8006e,  (Fl_Callback *) cb_New_Input, 0, 0, 0, 0, 14, 56},
    {gettext("Add an object trace"), 0,  (Fl_Callback *) cb_Graph_Traceobject, 0, 0, 0, 0, 14, 56},
    {gettext("Change selection attribut"), 0,  (Fl_Callback *) cb_Figure_Change_Attributs, 0, 0, 0, 0, 14, 56},
    {gettext("Paste"), 0,  (Fl_Callback *) cb_Paste, 0, 0, 0, 0, 14, 56},
    {gettext("Delete selected levels"), 0,  (Fl_Callback *) cb_Delete, 0, 0, 0, 0, 14, 56},
    {gettext("Portrait"), 0,  (Fl_Callback *) cb_Figure_Portrait, 0, 0, 0, 0, 14, 56},
    {gettext("Landscape"), 0,  (Fl_Callback *) cb_Figure_Landscape, 0, 0, 0, 0, 14, 56},
    {gettext("Undo"), 0x4007a,  (Fl_Callback *) History_cb_Undo, 0, 0, 0, 0, 14, 56},
    {gettext("Redo"), 0x40079,  (Fl_Callback *) History_cb_Redo, 0, 0, 0, 0, 14, 56},
    {gettext("Focus"), 0x8006d,  (Fl_Callback *) cb_Figure_Focus, 0, 0, 0, 0, 14, 56},
    {0}, // end Edit
    {gettext("Graph"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("Function"), 0,  (Fl_Callback *) cb_Figure_Function, 0, 0, 0, 0, 14, 56},
    {gettext("Tangent"), 0,  (Fl_Callback *) cb_Figure_Tangent, 0, 0, 0, 0, 14, 56},
    {gettext("Recurrent sequence (2-d)"), 0,  (Fl_Callback *) cb_Figure_Sequence, 0, 0, 0, 0, 14, 56},
    {gettext("Area under curve"), 0,  (Fl_Callback *) cb_Figure_Plotarea, 0, 0, 0, 0, 14, 56},
    {gettext("Parametric"), 0,  (Fl_Callback *) cb_Figure_Param, 0, 0, 0, 0, 14, 56},
    {gettext("Polar (2-d)"), 0,  (Fl_Callback *) cb_Figure_Polar, 0, 0, 0, 0, 14, 56},
    {gettext("Implicit"), 0,  (Fl_Callback *) cb_Figure_Implicit, 0, 0, 0, 0, 14, 56},
    {gettext("Slopefield Ode (2-d)"), 0,  (Fl_Callback *) cb_Figure_Plotfield, 0, 0, 0, 0, 14, 56},
    {0}, // end Math
    {0}, // end menu
  };

  Fl_Menu_Item Figure_menubut[] = {
    {gettext("Frame"), 0,  (Fl_Callback *) cb_Frame, 0, 0, 0, 0, 14, 56},
    {gettext("Pointer"), 0,  (Fl_Callback *) cb_Pointer, 0, 0, 0, 0, 14, 56},
    {gettext("point"), 0,  (Fl_Callback *) cb_Point, 0, 0, 0, 0, 14, 56},
    {gettext("Lines"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("segment"), 0,  (Fl_Callback *) cb_Segment, 0, 0, 0, 0, 14, 56},
    {gettext("vector"), 0,  (Fl_Callback *) cb_Vector, 0, 0, 0, 0, 14, 56},
    {gettext("half_line"), 0,  (Fl_Callback *) cb_Demi_Droite, 0, 0, 0, 0, 14, 56},
    {gettext("line"), 0,  (Fl_Callback *) cb_Droite, 0, 0, 0, 0, 14, 56},
    {gettext("parallel"), 0,  (Fl_Callback *) cb_Parallele, 0, 0, 0, 0, 14, 56},
    {gettext("perpendicular"), 0,  (Fl_Callback *) cb_Perpendiculaire, 0, 0, 0, 0, 14, 56},
    {gettext("perpen_bisector"), 0,  (Fl_Callback *) cb_Mediatrice, 0, 0, 0, 0, 14, 56},
    {gettext("median_line"), 0,  (Fl_Callback *) cb_Mediane, 0, 0, 0, 0, 14, 56},
    {gettext("bisector"), 0,  (Fl_Callback *) cb_Bissectrice, 0, 0, 0, 0, 14, 56},
    {0}, // end Lines
    {gettext("Polygons"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("triangle"), 0,  (Fl_Callback *) cb_Triangle, 0, 0, 0, 0, 14, 56},
    {gettext("equilateral_triangle"), 0,  (Fl_Callback *) cb_Triangle_Equilateral, 0, 0, 0, 0, 14, 56},
    {gettext("square"), 0,  (Fl_Callback *) cb_Carre, 0, 0, 0, 0, 14, 56},
    {gettext("quadrilateral"), 0,  (Fl_Callback *) cb_Quadrilatere, 0, 0, 0, 0, 14, 56},
    {gettext("polygon"), 0,  (Fl_Callback *) cb_Polygone, 0, 0, 0, 0, 14, 56},
    {0}, // end Triangle
    {gettext("Circles"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("circle"), 0,  (Fl_Callback *) cb_Circle, 0, 0, 0, 0, 14, 56},
    {gettext("incircle"), 0,  (Fl_Callback *) cb_inscrit, 0, 0, 0, 0, 14, 56},
    {gettext("excircle"), 0,  (Fl_Callback *) cb_exinscrit, 0, 0, 0, 0, 14, 56},
    {gettext("circumcircle"), 0,  (Fl_Callback *) cb_circonscrit, 0, 0, 0, 0, 14, 56},
    {0}, // end Circles
    {gettext("Curves"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("ellipse"), 0,  (Fl_Callback *) cb_ellipse, 0, 0, 0, 0, 14, 56},
    {gettext("hyperbola"), 0,  (Fl_Callback *) cb_hyperbole, 0, 0, 0, 0, 14, 56},
    {gettext("parabola"), 0,  (Fl_Callback *) cb_parabole, 0, 0, 0, 0, 14, 56},
    {gettext("plotode"), 0,  (Fl_Callback *) cb_plotode, 0, 0, 0, 0, 14, 56},
    {0}, // end Conics
    {gettext("Surfaces (3d)"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("plane"), 0,  (Fl_Callback *) cb_plan, 0, 0, 0, 0, 14, 56},
    {gettext("sphere"), 0,  (Fl_Callback *) cb_sphere, 0, 0, 0, 0, 14, 56},
    {0}, // end Surfaces
    {gettext("Measures"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("distanceat"), 0,  (Fl_Callback *) cb_DistanceAB, 0, 0, 0, 0, 14, 56},
    {gettext("angleat"), 0,  (Fl_Callback *) cb_AngleABC, 0, 0, 0, 0, 14, 56},
    {gettext("areaat"), 0,  (Fl_Callback *) cb_Area, 0, 0, 0, 0, 14, 56},
    {gettext("perimeterat"), 0,  (Fl_Callback *) cb_Perimeter, 0, 0, 0, 0, 14, 56},
    {gettext("slopeat"), 0,  (Fl_Callback *) cb_Slope, 0, 0, 0, 0, 14, 56},
    {gettext("distanceatraw"), 0,  (Fl_Callback *) cb_DistanceABraw, 0, 0, 0, 0, 14, 56},
    {gettext("angleatraw"), 0,  (Fl_Callback *) cb_AngleABCraw, 0, 0, 0, 0, 14, 56},
    {gettext("areaatraw"), 0,  (Fl_Callback *) cb_Arearaw, 0, 0, 0, 0, 14, 56},
    {gettext("perimeteratraw"), 0,  (Fl_Callback *) cb_Perimeterraw, 0, 0, 0, 0, 14, 56},
    {gettext("slopeatraw"), 0,  (Fl_Callback *) cb_Sloperaw, 0, 0, 0, 0, 14, 56},
    {0}, // end measures
    {gettext("Transformations"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("reflection"), 0,  (Fl_Callback *) cb_Symetrie, 0, 0, 0, 0, 14, 56},
    {gettext("rotation"), 0,  (Fl_Callback *) cb_Rotation, 0, 0, 0, 0, 14, 56},
    {gettext("translation"), 0,  (Fl_Callback *) cb_Translation, 0, 0, 0, 0, 14, 56},
    {gettext("projection"), 0,  (Fl_Callback *) cb_Projection, 0, 0, 0, 0, 14, 56},
    {gettext("homothety"), 0,  (Fl_Callback *) cb_Homothetie, 0, 0, 0, 0, 14, 56},
    {gettext("similarity"), 0,  (Fl_Callback *) cb_Similitude, 0, 0, 0, 0, 14, 56},
    {gettext("inversion"), 0,  (Fl_Callback *) cb_Inversion, 0, 0, 0, 0, 14, 56},
    {gettext("reciprocation"), 0,  (Fl_Callback *) cb_Reciprocation, 0, 0, 0, 0, 14, 56},
    {0}, // end transformations
    {gettext("Intersections"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("inter_unique (1 point)"), 0,  (Fl_Callback *) cb_Inter_Unique, 0, 0, 0, 0, 14, 56},
    {gettext("inter (close to a point)"), 0,  (Fl_Callback *) cb_Inter_3, 0, 0, 0, 0, 14, 56},
    {gettext("inter (list of points)"), 0,  (Fl_Callback *) cb_Inter, 0, 0, 0, 0, 14, 56},
    {0}, // end intersections
    {gettext("tangent"), 0,  (Fl_Callback *) cb_Tangent, 0, 0, 0, 0, 14, 56},
    // {gettext("Exact"), 0,  (Fl_Callback *) cb_Exact, 0, 0, 0, 0, 14, 56},
    // {gettext("Approx"), 0,  (Fl_Callback *) cb_Approx, 0, 0, 0, 0, 14, 56},
    {0}, // end Menu
  };
  
  // Create a tile with 1st element = a History_Pack, 2nd element geometry
  // Pressing enter in a History_Pack Multiline_input should 
  // eval it + eval below + update parameters and geometry
  // by reading Gen_Output values below current level
  Figure::Figure(int X,int Y,int W,int H,int L,bool dim3):Fl_Tile(X,Y,W,H),namestr(0){
    BorderBox * borderbox = new BorderBox(X,Y,w()-labelsize(),h()-labelsize());
    resizable(borderbox);
    Fl_Tile::end();
#ifdef IPAQ
    disposition=1;
    if (H<340)
      H=340;
#else
    disposition=0;
#endif
    box(FL_FLAT_BOX);
    if (L>H/2)
      L=H/2;
    labelsize(L);
    int l=L+6;
    Fl_Group::current(this);
    if (disposition==1)
      s = new xcas::HScroll (X,Y+l+H/2,W,H/2-l);
    else
      s = new xcas::HScroll (X,Y+l,W/3,H-l);
    s->type(Fl_Scroll::VERTICAL_ALWAYS);
    s->box(FL_FLAT_BOX);
    History_Pack * hp = new History_Pack (s->x(),s->y(),max(s->w()-2*L,20),max(s->h()-L,20));
    // hp->new_question=new_question_multiline_input;
    if (hp){
      hp->eval_below=true;
      hp->eval_below_once=false;
    }
    hp->labelsize(L);
    hp->pretty_output=false;
    hp->eval_below=true;
    hp->eval=xcas::Xcas_eval;
    hp->_insert=xcas::Xcas_pack_insert;
    hp->_select=xcas::Xcas_pack_select; 
    hp->end();
    hp->add_entry(-1);
    s->end();
    if (dim3){
      if (disposition==1)
	geo = new Geo3d(X,Y+l,W,H/2,hp);
      else
	geo = new Geo3d(X+W/3,Y+l,2*W/3,H-l,hp);
    }
    else {
      if (disposition==1)
	geo = new Geo2d(X,Y+l,W,H/2,hp);
      else
	geo = new Geo2d(X+W/3,Y+l,2*W/3,H-l,hp);
    }
    geo->labelsize(L);
    geo->autoscale();
    geo->orthonormalize();
    hp->focus(0,true); //geo->handle(FL_FOCUS);
    barre=new Fl_Group(X,Y,W,l);
    Fl_Menu_Bar * menubar = new Fl_Menu_Bar(X,Y,Max(W/4,68),l);
    int n= Figure_menu->size();
    Fl_Menu_Item * menuitem = new Fl_Menu_Item[n];
    for (int i=0;i<n;++i)
      *(menuitem+i)=*(Figure_menu+i);
    menubar->menu (menuitem);
    change_menu_fontsize(menuitem,2,L); // 2=#submenus
    int x=X;
    x += menubar->w();
    mode = new Fl_Output(x,Y,Max((7*W)/60,35),l);
    x += mode->w();
    mode->value(gettext("Frame"));
    Fl_Menu_Button * menubut = new Fl_Menu_Button(x,Y,(4*W)/60,l);
    menubut->tooltip(gettext("Set mode for mouse constructions"));
#ifndef IPAQ
    menubut->label("Mode");
#endif
    n = Figure_menubut->size();
    Fl_Menu_Item * menuitembut = new Fl_Menu_Item[n];
    for (int i=0;i<n;++i)
      *(menuitembut+i)=*(Figure_menubut+i);
    menubut->menu (menuitembut);
    change_menu_fontsize(menuitembut,2,L); // 2=#submenus
    x += menubut->w();
    couleur = new Fl_Button(x,Y,(3*l)/2,l);
    x += couleur->w();
    couleur->color(0);
    couleur->tooltip(gettext("Color used for next mouse drawings"));
    couleur->callback((Fl_Callback *) cb_Figure_Color);
    lt = new Line_Type(x,Y,l,l);
    lt->show_pnt(true);
    lt->show_text(true);
    lt->show_poly(true);
    lt->tooltip(gettext("Line style used for next mouse drawings"));
    lt->callback((Fl_Callback *) cb_Figure_Line_Type);
    x += l;
    Fl_Check_Button * checkb = new Fl_Check_Button(x,Y,3*l,l,"Step");
    checkb->value(false);
    checkb->tooltip(gettext("Check for step evaluation"));
    checkb->callback((Fl_Callback *) cb_Figure_Autoeval);
    x += checkb->w();
    checkdisp = new Fl_Check_Button(x,Y,6*l,l,"Landscape");
    checkdisp->value(false);
    checkdisp->tooltip(gettext("Check for landscape"));
    checkdisp->callback((Fl_Callback *) cb_Figure_Disposition);
    x += checkdisp->w();
    Fl_Check_Button * checkex = new Fl_Check_Button(x,Y,l,l,"~");
    checkex->value(true);
    checkex->tooltip(gettext("Check if mouse clicks in approx mode"));
    checkex->callback((Fl_Callback *) cb_Figure_Approx);
    x += checkex->w();
    name = new Fl_Button(x,Y,W+X-x,l,"<Save figure as text>");
    name->callback((Fl_Callback *) cb_Figure_Save);
    name->tooltip(gettext("Save current figure commands independently of the session"));
    name->labelsize(L);
    name->hide();
    barre->end();
    parent_redraw(this);
  }

  int BorderBox::handle(int event){
    return 0;
  }

  void Figure::resize(int X,int Y,int W,int H,double dhp,double dgeo,double dmp){
    Fl_Widget::resize(X,Y,W,H);
    double dall=dhp+dgeo+dmp;
    dhp /= dall; dgeo /= dall; dmp /= dall;
    int l=labelsize()+6;
    if (W>10*l && dmp*W<4*l){
      dmp=4.0/W*l;
      dhp=dhp/(dhp+dgeo);
      dgeo=1.0-dhp-dmp;
    }
    barre->resize(X,Y,W,l);
    if (disposition==1){
      checkdisp->value(true);
      double dgeomp=dgeo+dmp;
      geo->resize(X,Y+l,max(W-5*l,int(W*dgeo/dgeomp+.5)),int(H*dgeomp+.5));
      geo->mouse_param_group->resize(X+geo->w(),Y+l,W-geo->w(),geo->h());
      s->resize(X,geo->y()+geo->h(),W,H-geo->h()-l);
    }
    else {
      checkdisp->value(false);
      int sw=s->w(),gw=geo->w(),mw=geo->mouse_param_group->w();
      int dw= W-(sw+gw+mw);
      sw += dw;
      if (sw<l || dhp!=0.25 || dgeo!=0.5 || dmp!=0.25 ){
	sw=int(W*dhp+.5);
	gw=int(W*dgeo+.5);
      }
      s->resize(X,Y+l,sw,H-l);
      geo->resize(X+s->w(),Y+l,gw,H-l);
      geo->mouse_param_group->resize(X+s->w()+geo->w(),Y+l,W-gw-sw,H-l);
    }
    //geo->hp->Fl_Group::resize(s->x(),s->y(),max(s->w()-l,20),max(s->h()-l,20));
    init_sizes();
    Fl_Group::resize(x(),y(),w(),h());
    redraw();
  }

  vecteur Graph2d3d::animate(int nframes){
    int n=param_group->children();
    vecteur res;
    if (!n || !hp)
      return res;
    bool back=nframes<0;
    if (back)
      nframes=-nframes;
    if (nframes<2)
      nframes=2;
    Gen_Value_Slider * last=0;
    for (int i=0;i<nframes;++i){
      for (int j=n-1;j>=0;--j){
	if (Gen_Value_Slider * gvs = dynamic_cast<Gen_Value_Slider *>(param_group->child(j))){
	  last=gvs;
	  double v=gvs->minimum()+(i*(gvs->maximum()-gvs->minimum()))/(nframes-1);
	  gvs->value(v);
	  gvs->adjust(false);
	}
      }
      if (!last)
	return res;
      last->adjust(true);
      // wait for the end of evaluation
      for (;;){
	Fl_Widget *o = Fl::readqueue();
	if (Xcas_Cancel && o==Xcas_Cancel){
	  o->do_callback();
	  hp->doing_eval=false;
	  return res;
	}
	Xcas_idle_function(0);
	if (!hp->doing_eval && !is_context_busy(hp->contextptr))
	  break;
      }
      Fl::flush();
      vecteur resi;
      const_iterateur it=plot_instructions.begin(),itend=plot_instructions.end();
      for (;it!=itend;++it){
	gen tmp = *it;
	if (tmp.is_symb_of_sommet(at_parameter))
	  continue;
	if (tmp.is_symb_of_sommet(at_pnt) && tmp._SYMBptr->feuille.type==_VECT){
	  vecteur attributs=*tmp._SYMBptr->feuille._VECTptr;
	  tmp=evalf(attributs[0],eval_level(hp->contextptr),hp->contextptr);
	  attributs=vecteur(attributs.begin()+1,attributs.end());
	  tmp=pnt_attrib(tmp,attributs,hp->contextptr);
	}
	else
	  tmp=evalf(tmp,eval_level(hp->contextptr),hp->contextptr);
	resi.push_back(tmp);
      }
      res.push_back(resi);
    }
    if (back){
      for (int i=nframes-1;i>=0;i--)
	res.push_back(res[i]);
    }
    return res;
  }

  std::string Turtle::latex_save_figure(){
    static int nsession=0;
    nsession++;
    double xunit=giac::horiz_latex/w();
    double save_vert_latex=giac::vert_latex;
    giac::vert_latex=xunit*h();
    string thename="tortue"+print_INT_(nsession)+".tex";
    char * filename = file_chooser(gettext("Export to LaTeX"),"*.tex",thename.c_str());

    if (filename)
      graph2tex(filename,turtlevect2vecteur(*turtleptr),0,w(),0,h(),xunit,xunit,true,get_context(this));
    giac::vert_latex=save_vert_latex;
    return thename;
  }
  

  void Turtle::draw(){
#if 0
    context * contextptr=get_context(this);
    if (is_context_busy(contextptr))
      return;
#endif
    pthread_mutex_lock(&turtle_mutex);
    try {
      int clip_x,clip_y,clip_w,clip_h;
      fl_clip_box(x(),y(),w(),h(),clip_x,clip_y,clip_w,clip_h);
      fl_push_clip(clip_x,clip_y,clip_w,clip_h);
      indraw();
      fl_pop_clip();
    } catch (...){ }
    pthread_mutex_unlock(&turtle_mutex);
  }
  
  void Turtle::indraw(){
    int deltax=x(),deltay=y();
    int horizontal_pixels=w()-2*giac::COORD_SIZE;
    // Check for fast redraw
    // Then redraw the background
    fl_color(FL_WHITE);
    if (!redraw_cap_only)
      fl_rectf(deltax, deltay, w(), h());
    if (turtleptr && !turtleptr->empty()){
      if (turtlezoom>8)
	turtlezoom=8;
      if (turtlezoom<0.125)
	turtlezoom=0.125;
      // check that position is not out of screen
      logo_turtle t=turtleptr->back();
      double x=turtlezoom*(t.x-turtlex);
      if (x<0)
	turtlex += int(x/turtlezoom);
      if (x>=w()-10)
	turtlex += int((x-w()+10)/turtlezoom);
      double y=turtlezoom*(t.y-turtley);
      if (y<0)
	turtley += int(y/turtlezoom);
      if (y>h()-10)
	turtley += int((y-h()+10)/turtlezoom);
    }
    if (maillage & 0x3){
      fl_color(FL_BLACK);
      double xdecal=std::floor(turtlex/10.0)*10;
      double ydecal=std::floor(turtley/10.0)*10;
      if ( (maillage & 0x3)==1){
	for (double i=xdecal;i<w()+xdecal;i+=10){
	  for (double j=ydecal;j<h()+ydecal;j+=10){
	    fl_point(deltax+int((i-turtlex)*turtlezoom+.5),deltay+h()-int((j-turtley)*turtlezoom+.5));
	  }
	}
      }
      else {
	double dj=std::sqrt(3.0)*10,i0=xdecal;
	for (double j=ydecal;j<h()+ydecal;j+=dj){
	  int J=deltay+int(h()-(j-turtley)*turtlezoom);
	  for (double i=i0;i<w()+xdecal;i+=10){
	    fl_point(deltax+int((i-turtlex)*turtlezoom+.5),J);
	  }
	  i0 += dj;
	  while (i0>=10)
	    i0 -= 10;
	}
      }
    }
    // Show turtle position/cap
    if (turtleptr && !turtleptr->empty() && !(maillage & 0x4)){
      logo_turtle turtle=turtleptr->back();
      fl_color(FL_YELLOW);
      fl_rectf(deltax+horizontal_pixels,deltay,w()-horizontal_pixels,2*COORD_SIZE);
      fl_color(FL_BLACK);
      fl_rect(deltax, deltay, w(), h());
      fl_font(FL_HELVETICA,labelsize());
      fl_draw(("x "+print_INT_(int(turtle.x+.5))).c_str(),deltax+horizontal_pixels,deltay+(2*COORD_SIZE)/3-2);
      fl_draw(("y "+print_INT_(int(turtle.y+.5))).c_str(),deltax+horizontal_pixels,deltay+(4*COORD_SIZE)/3-3);
      fl_draw(("t "+print_INT_(int(turtle.theta+.5))).c_str(),deltax+horizontal_pixels,deltay+2*COORD_SIZE-4);
    }
    if (redraw_cap_only){      
      redraw_cap_only=false;
      return;
    }
    // draw turtle Logo
    if (turtleptr){
      int l=turtleptr->size();
      if (l>0){
	logo_turtle prec =(*turtleptr)[0];
	for (int k=1;k<l;++k){
	  logo_turtle current =(*turtleptr)[k];
	  if (!current.s.empty()){ // Write a string
	    fl_font(FL_HELVETICA,current.radius);
	    xcas_color(current.color);
	    fl_draw(current.s.c_str(),int(deltax+turtlezoom*(current.x-turtlex)),int(deltay+h()-turtlezoom*(current.y-turtley)));
	  }
	  else {
	    if (current.radius>0){
	      int r=current.radius & 0x1ff; // bit 0-8
	      double theta1,theta2;
	      if (current.direct){
		theta1=prec.theta+double((current.radius >> 9) & 0x1ff); // bit 9-17
		theta2=prec.theta+double((current.radius >> 18) & 0x1ff); // bit 18-26
	      }
	      else {
		theta1=prec.theta-double((current.radius >> 9) & 0x1ff); // bit 9-17
		theta2=prec.theta-double((current.radius >> 18) & 0x1ff); // bit 18-26
	      }
	      bool rempli=(current.radius >> 27) & 0x1;
	      double angle;
	      int x,y,R;
	      R=int(2*turtlezoom*r+.5);
	      angle = M_PI/180*(theta2-90);
	      if (current.direct){
		x=int(turtlezoom*(current.x-turtlex-r*std::cos(angle) - r)+.5);
		y=int(turtlezoom*(current.y-turtley-r*std::sin(angle) + r)+.5);
	      }
	      else {
		x=int(turtlezoom*(current.x-turtlex+r*std::cos(angle) -r)+.5);
		y=int(turtlezoom*(current.y-turtley+r*std::sin(angle) +r)+.5);
	      }
	      xcas_color(current.color);
	      if (current.direct){
		if (rempli)
		  fl_pie(deltax+x,deltay+h()-y,R,R,theta1-90,theta2-90);
		else
		  fl_arc(deltax+x,deltay+h()-y,R,R,theta1-90,theta2-90);
	      }
	      else {
		if (rempli)
		  fl_pie(deltax+x,deltay+h()-y,R,R,90+theta2,90+theta1);
		else
		  fl_arc(deltax+x,deltay+h()-y,R,R,90+theta2,90+theta1);
	      }
	    } // end radius>0
	    else {
	      if (prec.mark){
		xcas_color(prec.color);
		fl_line(deltax+int(turtlezoom*(prec.x-turtlex)+.5),deltay+int(h()+turtlezoom*(turtley-prec.y)+.5),deltax+int(turtlezoom*(current.x-turtlex)+.5),deltay+int(h()+turtlezoom*(turtley-current.y)+.5));
	      }
	    }
	    if (current.radius<-1 && k+current.radius>=0){
	      // poly-line from (*turtleptr)[k+current.radius] to (*turtleptr)[k]
	      fl_begin_complex_polygon();
	      for (int i=0;i>=current.radius;--i){
		logo_turtle & t=(*turtleptr)[k+i];
		fl_vertex(deltax+turtlezoom*(t.x-turtlex),deltay+h()+turtlezoom*(turtley-t.y));
	      }
	      fl_vertex(deltax+turtlezoom*(current.x-turtlex),deltay+h()+turtlezoom*(turtley-current.y));
	      fl_end_complex_polygon();
	    }
	  } // end else (non-string turtle record)
	  prec=current;
	} // end for (all turtle records)
	logo_turtle & t = (*turtleptr)[l-1];
	int x=int(turtlezoom*(t.x-turtlex)+.5);
	int y=int(turtlezoom*(t.y-turtley)+.5);
	double cost=std::cos(t.theta*deg2rad_d);
	double sint=std::sin(t.theta*deg2rad_d);
	int Dx=int(turtlezoom*t.turtle_length*cost/2+.5);
	int Dy=int(turtlezoom*t.turtle_length*sint/2+.5);
	xcas_color(t.color);
	if (t.visible){
	  fl_line(deltax+x+Dy,deltay+h()-(y-Dx),deltax+x-Dy,deltay+h()-(y+Dx));
	  if (!t.mark)
	    xcas_color(t.color+1);
	  fl_line(deltax+x+Dy,deltay+h()-(y-Dx),deltax+x+3*Dx,deltay+h()-(y+3*Dy));
	  fl_line(deltax+x-Dy,deltay+h()-(y+Dx),deltax+x+3*Dx,deltay+h()-(y+3*Dy));
	}
      }
      return;
    } // End logo mode
  }

  int Turtle::handle(int event){
#if 0
    context * contextptr=get_context(this);
    if (is_context_busy(contextptr))
      return 0;
#endif
    // cerr << event << endl;
    if ( (event==FL_ENTER) || (event==FL_LEAVE) ){
      if (event==FL_LEAVE)
	redraw_cap_only=true;
      redraw();
      return 1;
    }
    if (event==FL_FOCUS){
      Fl::focus(this);
      return 1;
    }
    if (event==FL_PUSH){
      push_x=Fl::event_x()-x();
      push_y=Fl::event_y()-y();
      Fl::focus(this);
      return 1;      
    }
    int current_x=Fl::event_x()-x(),current_y=Fl::event_y()-y();
    // int horizontal_pixels=w();
    if (event== FL_MOUSEWHEEL ){
      if (Fl::e_dy<0)
	turtlezoom *= 0.707 ;
      else
	turtlezoom *= 1.414 ;
      redraw();
      return 1;
    }
    if (event==FL_RELEASE || event==FL_DRAG){
      if (current_y!=push_y){ 
	turtley += int((current_y-push_y));
	redraw();
      }
      if (current_x!=push_x){ 
	turtlex += int((push_x-current_x));
	redraw();
      }
      push_y=current_y;
      push_x=current_x;
      return 1;
    }
    if (event==FL_KEYBOARD){
      switch (Fl::event_key()){
      case FL_Left:
	turtlex += 10;
	redraw();
	return 1;
      case FL_Up:
	turtley -= 10;
	redraw();
	return 1;
      case FL_Right: 
	turtlex -= 10;
	redraw();
	return 1;
      case FL_Down: 
	turtley += 10;
	redraw();
	return 1;
      case '+':
	turtlezoom *= 1.414 ;
	redraw();
	return 1;
      case '-':
	turtlezoom *= 0.707 ;
	redraw();
	return 1;	
      }
    }
    return 0;
  }

  Turtle::~Turtle(){
    context * contextptr=get_context(this);
    if (turtle(contextptr).widget==this)
      turtle(contextptr).widget=0;
  }

  void cb_Logo_button(No_Focus_Button * b,void *){
    if (!Fl::focus())
      return;
    context * contextptr=get_context(b);
    int l=language(contextptr);
    string text;
    if (b->label()==gettext("fw"))
      text=localize("avance ",l);
    if (b->label()==gettext("bw"))
      text=localize("recule ",l);
    if (b->label()==gettext("tr"))
      text=localize("tourne_droite ",l);
    if (b->label()==gettext("tl"))
      text=localize("tourne_gauche ",l);
    if (b->label()==gettext("ss"))
      text=localize("pas_de_cote ",l);
    if (b->label()==gettext("ju"))
      text=localize("saute ",l);
    if (b->label()==gettext("pe")){
      context * contextptr=get_context(b);
      text=localize("crayon ",l);
      int col = turtle(contextptr).color;
      col=fl_show_colormap((Fl_Color) col);
      string s=col<1000?print_INT_(col):"";
      text += s;
      // b->color(col);
    }
    if (b->label()==gettext("ci"))
      text=localize("rond ",l);
    if (b->label()==gettext("di"))
      text=localize("disque ",l);
    if (b->label()==gettext("fr"))
      text=localize("rectangle_plein ",l);
    if (b->label()==gettext("ft"))
      text=localize("triangle_plein ",l);
    if (b->label()==gettext("cl"))
      text=localize("efface ",l);
    if (b->label()==gettext("ec"))
      text=localize("ecris ",l);
    if (b->label()==gettext("sg"))
      text=localize("signe ",l);
    Fl::focus(Xcas_input_focus);
    if (!text.empty())
      help_output(text.substr(0,text.size()-1),language(get_context(Fl::focus())));
    Fl::e_text= (char *) text.c_str();
    Fl::e_length=text.size();
    fl_handle(Fl::focus());
  }

  Fl_Widget * Logo_eval(Fl_Widget * w) {
    // Check if w is inside a pack inside a scroll inside a Logo group
    const context * contextptr = get_context(w);
    if (Fl_Input * i=dynamic_cast<Fl_Input *>(w)){
      Fl_Group * g=parent_skip_scroll(w->parent()); // should be a pack
      if (g)
	g=parent_skip_scroll(g);
      if (Logo * l= dynamic_cast<Logo *>(g)){
	string s(i->value());
	if (dynamic_cast<Multiline_Input_tab *>(i)){
	  if (!s.empty() && s[s.size()-1]!=';')
	    s += ";\n";
	  else
	    s += '\n';
	  gen g(s,contextptr);
	  if (!giac::first_error_line(contextptr)){
	    l->ed->editor->insert_position(l->ed->editor->buffer()->length());
	    l->ed->editor->insert(s.c_str());
	  }
	  else {
	    string logs(gettext("Not registered\n"));
	    logs += gettext("Parse error line ")+giac::print_INT_(giac::first_error_line(contextptr))+ gettext(" at ") +giac::error_token_name(contextptr);
	    fl_alert("%s",logs.c_str());
	  }
	}
	else
	  s = "// "+s+'\n';
	l->t->redraw();
      }
    }
    if (Xcas_Text_Editor * i=dynamic_cast<Xcas_Text_Editor *>(w)){
      Fl_Group * g=parent_skip_scroll(w->parent()); // should be a pack
      if (g)
	g=parent_skip_scroll(g);
      if (Logo * l= dynamic_cast<Logo *>(g)){
	string s(i->value());
	if (!s.empty() && s[s.size()-1]!=';')
	  s += ";\n";
	else
	  s += '\n';
	gen g(s,contextptr);
	if (!giac::first_error_line(contextptr)){
	  l->ed->editor->insert_position(l->ed->editor->buffer()->length());
	  l->ed->editor->insert(s.c_str());
	}
	else {
	  string logs(gettext("Not registered\n"));
	  logs += gettext("Parse error line ")+giac::print_INT_(giac::first_error_line(contextptr))+ gettext(" at ") +giac::error_token_name(contextptr);
	  fl_alert("%s",logs.c_str());
	}
	l->t->redraw();
      }
    }
    return Xcas_eval(w);
  }
    
  Turtle * in_find_turtle(Fl_Widget * wid){
    if (Turtle * g=dynamic_cast<Turtle *>(wid))
      return g;
    if (Fl_Group * gr =dynamic_cast<Fl_Group *>(wid)){
      // search in children
      int n=gr->children();
      for (int i=0;i<n;++i){
	if (Turtle * res = in_find_turtle(gr->child(i)))
	  return res;
      }
    }
    return 0;
  }

  Turtle * find_turtle(Fl_Widget * wid){
    if (!wid) return 0;
    if (Turtle * res=in_find_turtle(wid))
      return res;
    return find_turtle(wid->parent());
  }

  static void cb_Logo_Preview(Fl_Menu_* m , void*) {
    Turtle * gr = find_turtle(m);
    if (gr)
      widget_ps_print(gr,"tortue",true);
  }

  static void cb_Logo_Print(Fl_Menu_* m , void*) {
    Turtle * gr = find_turtle(m);
    if (gr)
      widget_print(gr);
  }

  const char * Turtle::latex(const char * filename_) const {
    const char * filename=0;
    if ( (filename=latexfilename(filename_)) ){
      double xunit=giac::horiz_latex/w();
      double yunit=giac::vert_latex/h();
      double unit=xunit<yunit?yunit:xunit;
      graph2tex(filename,turtlevect2vecteur(*turtleptr),0,w(),0,h(),unit,unit,true,get_context(this));    
    }
    return filename;
  }


  static void cb_Logo_LaTeX_Preview(Fl_Menu_* m , void*) {
    Turtle * f=find_turtle(m);
    if (f){
      string name=f->latex_save_figure();
      xdvi(name);
    }
  }

  static void cb_Logo_LaTeX_Print(Fl_Menu_* m , void*) {
    Turtle * f=find_turtle(m);
    if (f){
      string name=f->latex_save_figure();
      dvips(name);
    }
  }

  static void cb_Logo_None(Fl_Menu_* m , void*) {
    Turtle * f=find_turtle(m);
    if (f) { f->maillage=f->maillage & 0x4; f->redraw(); }
  }

  static void cb_Logo_Square(Fl_Menu_* m , void*) {
    Turtle * f=find_turtle(m);
    if (f) { f->maillage=1 | (f->maillage & 0x4); f->redraw(); }
  }

  static void cb_Logo_Triangle(Fl_Menu_* m , void*) {
    Turtle * f=find_turtle(m);
    if (f) { f->maillage = 2 | (f->maillage & 0x4); f->redraw(); }
  }

  static void cb_Logo_ShowPosition(Fl_Menu_* m , void*) {
    Turtle * f=find_turtle(m);
    if (f) { f->maillage=f->maillage & 0x3; f->redraw(); }
  }

  static void cb_Logo_HidePosition(Fl_Menu_* m , void*) {
    Turtle * f=find_turtle(m);
    if (f) { f->maillage=f->maillage | 0x4; f->redraw(); }
  }

  static void cb_Logo_Color(Fl_Menu_* m , void*) {
    const giac::context * contextptr=get_context(m);
    int i=fl_show_colormap(Fl_Color(turtle(contextptr).color));
    string s="crayon "+print_INT_(i);
    if (Fl_Input * i=dynamic_cast<Fl_Input *>(Fl::focus()))
      i->insert(s.c_str());
    else
      fl_message("%s",s.c_str());
  }

  static void cb_Logo_Zoomin(Fl_Menu_* m , void*) {
    Turtle * gr = find_turtle(m);
    if (gr){
      gr->turtlezoom*=1.414;
      gr->redraw();
    }
  }

  static void cb_Logo_Zoomout(Fl_Menu_* m , void*) {
    Turtle * gr = find_turtle(m);
    if (gr){
      gr->turtlezoom*=0.707;
      gr->redraw();
    }
  }

  static void cb_Logo_Right(Fl_Menu_* m , void*) {
    Turtle * gr = find_turtle(m);
    if (gr){
      gr->turtlex -= 10 ;
      gr->redraw();
    }
  }

  static void cb_Logo_Left(Fl_Menu_* m , void*) {
    Turtle * gr = find_turtle(m);
    if (gr){
      gr->turtlex += 10 ;
      gr->redraw();
    }
  }

  static void cb_Logo_Up(Fl_Menu_* m , void*) {
    Turtle * gr = find_turtle(m);
    if (gr){
      gr->turtley -= 10 ;
      gr->redraw();
    }
  }

  static void cb_Logo_Down(Fl_Menu_* m , void*) {
    Turtle * gr = find_turtle(m);
    if (gr){
      gr->turtley += 10 ;
      gr->redraw();
    }
  }

  Fl_Menu_Item Logo_menu[] = {
    {gettext("M"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("Mesh"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("None"), 0,  (Fl_Callback*)cb_Logo_None, 0, 0, 0, 0, 14, 56},
    {gettext("Square"), 0,  (Fl_Callback*)cb_Logo_Square, 0, 0, 0, 0, 14, 56},
    {gettext("Triangle"), 0,  (Fl_Callback*)cb_Logo_Triangle, 0, 0, 0, 0, 14, 56},
    {0},
    {gettext("View"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("Zoom in"), 0,  (Fl_Callback*)cb_Logo_Zoomin, 0, 0, 0, 0, 14, 56},
    {gettext("Zoom out"), 0,  (Fl_Callback*)cb_Logo_Zoomout, 0, 0, 0, 0, 14, 56},
    {gettext("Right"), 0,  (Fl_Callback*)cb_Logo_Right, 0, 0, 0, 0, 14, 56},
    {gettext("Left"), 0,  (Fl_Callback*)cb_Logo_Left, 0, 0, 0, 0, 14, 56},
    {gettext("Up"), 0,  (Fl_Callback*)cb_Logo_Up, 0, 0, 0, 0, 14, 56},
    {gettext("Down"), 0,  (Fl_Callback*)cb_Logo_Down, 0, 0, 0, 0, 14, 56},
    {gettext("ShowPosition"), 0,  (Fl_Callback*)cb_Logo_ShowPosition, 0, 0, 0, 0, 14, 56},
    {gettext("HidePosition"), 0,  (Fl_Callback*)cb_Logo_HidePosition, 0, 0, 0, 0, 14, 56},
    {0},
    {gettext("Export/Print"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("EPS/PNG and preview"), 0,  (Fl_Callback*)cb_Logo_Preview, 0, 0, 0, 0, 14, 56},
    {gettext("Print"), 0,  (Fl_Callback*)cb_Logo_Print, 0, 0, 0, 0, 14, 56},
    {gettext("Preview (with Latex)"), 0,  (Fl_Callback*)cb_Logo_LaTeX_Preview, 0, 0, 0, 0, 14, 56},
    {gettext("Print (with Latex)"), 0,  (Fl_Callback*)cb_Logo_LaTeX_Print, 0, 0, 0, 0, 14, 56},
    {0},
    {gettext("Color"), 0,  (Fl_Callback*)cb_Logo_Color, 0, 0, 0, 0, 14, 56},
    {0},
    {0}
  };

  Logo::Logo(int X,int Y,int W,int H,int L):Fl_Tile(X,Y,W,H){
    Fl_Tile::end();
    scroll_position = -1;
    box(FL_FLAT_BOX);
    if (L>H/2)
      L=H/2;
    Fl_Group::current(this);
    xcas::HScroll * s = new xcas::HScroll (X,Y,W/4,H);
    s->box(FL_FLAT_BOX);
    hp = new History_Pack (X,Y,max(W/4-20,W/5),max(H-L-20,20));
    // hp->new_question=new_question_multiline_input;
    hp->labelsize(L);
    hp->pretty_output=false;
    hp->eval_below=false;
    hp->eval=xcas::Logo_eval;
    hp->_insert=xcas::Xcas_pack_insert;
    hp->_select=xcas::Xcas_pack_select; 
    hp->end();
    s->end();
    t =  new Turtle(X+W/4,Y,W/2,H-L);
    t->labelsize(L);
    t->turtleptr=&giac::turtle_stack(context0); // will be overwritten by new_logo in History.cc
    t->turtlezoom=2;
    int bw=(W/2-5*L/2)/14; 
    button_group = new Fl_Group(X+W/4,Y+H-L,bw*14,L);
    int bx=button_group->x(),by=button_group->y();
    No_Focus_Button * avance = new No_Focus_Button(bx,by,bw,L);
    avance->label(gettext("fw"));
    avance->tooltip(gettext("Turtle n steps forward"));
    avance->callback((Fl_Callback *) cb_Logo_button);
    bx += bw;
    No_Focus_Button * recule = new No_Focus_Button(bx,by,bw,L);
    recule->label(gettext("bw"));
    recule->tooltip(gettext("Turtle n steps backward"));
    recule->callback((Fl_Callback *) cb_Logo_button);
    bx += bw;
    No_Focus_Button * td = new No_Focus_Button(bx,by,bw,L);
    td->label(gettext("tr"));
    td->tooltip(gettext("Turtle turns right n degrees"));
    td->callback((Fl_Callback *) cb_Logo_button);
    bx += bw;
    No_Focus_Button * tg = new No_Focus_Button(bx,by,bw,L);
    tg->label(gettext("tl"));
    tg->tooltip(gettext("Turtle turns left n degrees"));
    tg->callback((Fl_Callback *) cb_Logo_button);
    bx += bw;
    No_Focus_Button * pc = new No_Focus_Button(bx,by,bw,L);
    pc->label(gettext("ss"));
    pc->tooltip(gettext("Turtle steps to the left from n steps"));
    pc->callback((Fl_Callback *) cb_Logo_button);
    bx += bw;
    No_Focus_Button * sa = new No_Focus_Button(bx,by,bw,L);
    sa->label(gettext("ju"));
    sa->tooltip(gettext("Turtle jumps n steps"));
    sa->callback((Fl_Callback *) cb_Logo_button);
    bx += bw;
    No_Focus_Button * cr = new No_Focus_Button(bx,by,bw,L);
    cr->label(gettext("pe"));
    cr->tooltip(gettext("Change pen color"));
    cr->callback((Fl_Callback *) cb_Logo_button);
    bx += bw;
    No_Focus_Button * ro = new No_Focus_Button(bx,by,bw,L);
    ro->label(gettext("ci"));
    ro->tooltip(gettext("Circle arc"));
    ro->callback((Fl_Callback *) cb_Logo_button);
    bx += bw;
    No_Focus_Button * di = new No_Focus_Button(bx,by,bw,L);
    di->label(gettext("di"));
    di->tooltip(gettext("Filled circle arc"));
    di->callback((Fl_Callback *) cb_Logo_button);
    bx += bw;
    No_Focus_Button * rp = new No_Focus_Button(bx,by,bw,L);
    rp->label(gettext("fr"));
    rp->tooltip(gettext("Filled rectangle"));
    rp->callback((Fl_Callback *) cb_Logo_button);
    bx += bw;
    No_Focus_Button * tp = new No_Focus_Button(bx,by,bw,L);
    tp->label(gettext("ft"));
    tp->tooltip(gettext("Filled triangle"));
    tp->callback((Fl_Callback *) cb_Logo_button);
    bx += bw;
    No_Focus_Button * ec = new No_Focus_Button(bx,by,bw,L);
    ec->label(gettext("ec"));
    ec->tooltip(gettext("Write to the right of the turtle"));
    ec->callback((Fl_Callback *) cb_Logo_button);
    bx += bw;
    No_Focus_Button * sg = new No_Focus_Button(bx,by,bw,L);
    sg->label(gettext("sg"));
    sg->tooltip(gettext("Sign picture"));
    sg->callback((Fl_Callback *) cb_Logo_button);
    bx += bw;
    No_Focus_Button * ef = new No_Focus_Button(bx,by,bw,L);
    ef->label(gettext("cl"));
    ef->tooltip(gettext("Clear all"));
    ef->callback((Fl_Callback *) cb_Logo_button);
    button_group->end();
    menubar = new Fl_Menu_Bar(button_group->x()+button_group->w(),button_group->y(),W/2-button_group->w(),L);
    int ls= Logo_menu->size();
    Fl_Menu_Item * menuitem = new Fl_Menu_Item[Logo_menu->size()];
    for (int i=0;i<ls;++i)
      *(menuitem+i)=*(Logo_menu+i);
    menubar->menu (menuitem);    
    menubar->tooltip(gettext("Turtle menu"));
    ed = new Editeur(X+W/4+W/2,Y,W/4,H);
    // ed->callback(History_Pack_cb_eval,0);
    ed->extension="tor";
    ed->editor->buffer()->insert(0,"efface ;\n");
    ed->editor->insert_position(9);
    hp->add_entry(-1);
    hp->resize();
    parent_redraw(this);
  }

  int Logo::handle(int event){
    if (event==FL_MOUSEWHEEL){
      if (!Fl::event_inside(this))
	return 0;
      if (Fl::focus()==t){
	int res=t->handle(event);
	if (res)
	  return res;
      }
      if (!Fl::event_inside(hp->parent()))
	return 0;      
      return hp->parent()->handle(event);
    }
    return Fl_Tile::handle(event);
  }

  void Logo::draw(){
    Fl_Tile::draw();
    if (scroll_position>0 && hp){
      if (Fl_Scroll * scroll = dynamic_cast<Fl_Scroll *>(hp->parent())){
#ifdef _HAVE_FL_UTF8_HDR_
	scroll->scroll_to(0,scroll_position);
#else
	scroll->position(0,scroll_position);
#endif
	scroll_position=-1;
	Fl_Tile::draw();
      }
    }
  }
 
  void Logo::resize(int X,int Y,int W,int H){
    Fl_Tile::resize(X,Y,W,H);
    int L=labelsize();
    if (L>H/2)
      L=H/2;
    int hph=hp->h();
    Fl_Group * g =  hp->parent();
    if (Fl_Scroll * scroll = dynamic_cast<Fl_Scroll *>(g))
      scroll_position = scroll->yposition();
    g->resize(X,Y,W/4,H);
    hp->Fl_Group::resize(X,Y,max(W/4-20,W/5),hph);
    hp->resize();
    t->resize(X+W/4,Y,W/2,H-L);
    int bw=(W/2-5*L/2)/14*14; 
    button_group->resize(X+W/4,Y+H-L,bw,L);
    menubar->resize(X+W/4+bw,Y+H-L,W/2-bw,L);
    ed->resize(X+W/4+W/2,Y,W/4,H);
  }

  ostream & fltk_fl_widget_archive_function(ostream & os,void * ptr){
    Fl_Widget * w=(Fl_Widget *) ptr;
    const context * contextptr=get_context(w);
    Graph2d *i=dynamic_cast<Graph2d *>(w);
    if (!i)
      return archive(os,string2gen("Done",false),contextptr);
    os << _POINTER_ << " " << _FL_WIDGET_POINTER << endl;
    archive(os,i->plot_instructions,contextptr);
    archive(os,makevecteur(i->x(),i->y(),i->w(),i->h(),i->window_xmin,i->window_xmax,i->window_ymin,i->window_ymax),contextptr);
    return os;
  }

  gen fltk_fl_widget_unarchive_function(istream & os){
    // FIXME GIAC_CONTEXT
    gen gplot=unarchive(os,context0),gparam=unarchive(os,context0);
    if (gplot.type!=_VECT || gparam.type!=_VECT || gparam._VECTptr->size()<8)
      return 0;
    vecteur v=*gparam._VECTptr;
    Fl_Group::current(0);
    Graph2d * ptr= new Graph2d(v[0].val,v[1].val,v[2].val,v[3].val,"");
    ptr->plot_instructions=vecteur(*gplot._VECTptr);
    ptr->window_xmin=v[4]._DOUBLE_val;
    ptr->window_xmax=v[5]._DOUBLE_val;
    ptr->window_ymin=v[6]._DOUBLE_val;
    ptr->window_ymax=v[7]._DOUBLE_val;
    return gen(ptr,_FL_WIDGET_POINTER);
  }

  std::string fltk_fl_widget_texprint_function(void * ptr){
    static int counter=0;
    Fl_Widget * w=(Fl_Widget *) ptr;
    Graph2d * i=dynamic_cast<Graph2d *>(w);
    ++counter;
    double horiz_unit=horiz_latex/(i->window_xmax-i->window_xmin);
    double vert_unit=vert_latex/(i->window_ymax-i->window_ymin);
    double unit=horiz_unit;
    if (horiz_unit>vert_unit)
      unit=vert_unit;
    string tmpfilename("#tmp"+print_INT_(counter));
    FILE * out =fopen(tmpfilename.c_str(),"w");
    if (!out)
      return "Write error";
    if (!graph2tex(out,i->plot_instructions,i->window_xmin,i->window_xmax,i->window_ymin,i->window_ymax,unit,tmpfilename.c_str(),false,get_context(i)))
      return "Graph2tex Error";
    fclose(out);
    FILE * in=fopen(tmpfilename.c_str(),"r");
    if (!in)
      return "Read Error";
    string res;
    while (!feof(in))
      res += fgetc(in);
    return res;
  }

  extern Multiline_Input_tab * Xcas_multiline_input; 

  // adjust history level to the slider value, and eval history pack
  // if eval_hp is true
  void Gen_Value_Slider::adjust(bool eval_hp){
    Fl_Group * g = parent(); // param_group or tile
    if (!g) return;
    g=parent_skip_scroll(g); // mouse_param_group or history_pack
    if (!g) return;
    History_Pack * hp=0;
    int position=pos;
    if (pos==-2){
      hp=dynamic_cast<History_Pack *>(g);
      if (hp){
	for (int i=0;i<hp->children();i++){
	  if (hp->child(i)==parent()){
	    position=i;
	    break;
	  }
	}
      }
    }
    else {
      g=parent_skip_scroll(g); // parent of Graph2d screen
      if (!g) return;
      // Now find an history_pack inside g
      int n=g->children();
      for (int i=0;i<n;++i){
	Fl_Widget * wid = g->child(i);
	if (Fl_Scroll * s=dynamic_cast<Fl_Scroll *>(wid)) 
	  if (s->children())
	    wid=s->child(0);
	if ( (hp=dynamic_cast<History_Pack *>(wid)) )
	  break;
      } // end for i
    }
    if (hp){
      // check that level is gvs->g := ...
      unsigned m=hp->children();
      if (position>=0 && position<int(m)){
	bool do_cb=false;
	giac::gen gt(hp->value(position),hp->contextptr);
	if (gt.is_symb_of_sommet(at_sto) && gt._SYMBptr->feuille.type==_VECT && !gt._SYMBptr->feuille._VECTptr->empty() ){
	  // Check for element
	  gen & f = gt._SYMBptr->feuille._VECTptr->front();
	  if (f.is_symb_of_sommet(at_element)){
	    gen ff = f._SYMBptr->feuille;
	    if (ff.type==_VECT && !ff._VECTptr->empty())
	      ff = ff._VECTptr->front();
	    // Change gt value with parameter
	    ff = symbolic(at_element,gen(makevecteur(ff,value(),Fl_Valuator::step()),_SEQ__VECT));
	    gt=symbolic(at_sto,makevecteur(ff,gt._SYMBptr->feuille._VECTptr->back()));
	    do_cb=true;
	  }
	} // end element
	if (gt.is_symb_of_sommet(at_assume)){
	  gen & f = gt._SYMBptr->feuille;
	  if (f.is_symb_of_sommet(at_equal)||f.is_symb_of_sommet(at_same)){
	    gen & ff = f._SYMBptr->feuille;
	    if (ff.type==_VECT && !ff._VECTptr->empty()){
	      vecteur v=*ff._VECTptr;
	      if (v.back().type==_VECT && v.back()._VECTptr->size()>=3){
		vecteur & vb=*v.back()._VECTptr;
		gen step=(vb[2]-vb[1])/100;
		if (vb.size()>3)
		  step=vb[3];
		v.back()=makevecteur(value(),vb[1],vb[2],step);
	      }
	      else
		v.back()=makevecteur(value(),minimum(),maximum(),Fl_Valuator::step());
	      gt = symbolic(at_equal,v);
	      gt = symbolic(at_assume,gt);
	      do_cb = true;
	    }
	  }
	  if (f.is_symb_of_sommet(at_sto)){
	    gen & ff = f._SYMBptr->feuille;
	    if (ff.type==_VECT && !ff._VECTptr->empty()){
	      vecteur v=*ff._VECTptr;
	      if (v.front().type==_VECT && v.front()._VECTptr->size()>=3){
		vecteur & vf=*v.front()._VECTptr;
		gen step=(vf[2]-vf[1])/100.;
		if (vf.size()>3)
		  step=vf[3];
		v.front()=makevecteur(value(),vf[1],vf[2],step);
	      }
	      else
		v.front() = makevecteur(value(),minimum(),maximum(),Fl_Valuator::step());
	      gt = symbolic(at_sto,v);
	      gt = symbolic(at_assume,gt);
	      do_cb = true;
	    }
	  }
	} // end assume
	if (do_cb){
	  hp->update_pos=position;
	  if (pos==-2)
	    hp->eval_below=true;
	  gen curseur=protecteval(gt,1,hp->contextptr);
	  Graph2d3d * graph=find_graph2d3d(hp);
	  if (curseur.is_symb_of_sommet(at_parameter) && graph){
	    vecteur & v=graph->plot_instructions;
	    if (position>=0 && v.size()>position){
	      gen & vp=v[position];
	      if (vp.is_symb_of_sommet(at_parameter) && vp._SYMBptr->feuille[0]==curseur._SYMBptr->feuille[0])
		vp=curseur;
	    }
	  }
	  // check how to update (parameter in a figure or in history)
	  Fl_Group * gr = hp->widget_group(position);
	  if (gr && gr->children()>=3){
	    if (dynamic_cast<Fl_Output*>(gr->child(2))){
	      hp->update_pos=position;
	      hp->set_gen_value(position,gt,true);
	      return;
	    }
	  }
	  hp->set_gen_value(position,gt,false);
	  if (eval_hp && hp->children()>position+1){
	    hp->update_pos=position+1;
	    gt=hp->parse(position+1);
	    if (!is_undef(gt))
	      hp->set_gen_value(position+1,gt,true);
	  }
	} // end do_cb
      } // end if gvs->pos<children()
    } // end if history_pack hp
  }

  Figure * get_figure(Fl_Widget * widget){
    for (;widget;){
      if (Figure * f =dynamic_cast<Figure *>(widget))
	return f;
      widget = widget->parent();
    }
    return 0;
  }

  int Gen_Value_Slider::handle(int event){
    string tmp;
    Figure * f=get_figure(this);
    int position=pos;
    History_Pack * hp=f?f->geo->hp:get_history_pack(this,position);
    double tmin=minimum(),tmax=maximum(),tcur=value(),tstep=Fl_Valuator::step();
    // tstep=100*tstep/(tmax-tmin);
    if ((event==FL_PUSH || event==FL_DRAG || event==FL_RELEASE) &&Fl::event_button()== FL_RIGHT_MOUSE){
      if (event==FL_RELEASE && hp->children()>position){
	Fl_Widget * wid=hp->child(position);
	while (Fl_Group * s=dynamic_cast<Fl_Group *>(wid)) {
	  if (dynamic_cast<Xcas_Text_Editor *>(wid))
	    break;
	  if (s->children())
	    wid=s->child(0);
	}
	if (Multiline_Input_tab * m=dynamic_cast<Multiline_Input_tab *>(wid)){
	  gen g=m->g();
	  if (figure_param_dialog(hp,true,tmin,tmax,tcur,tstep,paramname,g.is_symb_of_sommet(at_assume),tmp) ){
	    minimum(tmin);
	    maximum(tmax);
	    value(tcur);
	    // tstep=tstep*(tmax-tmin)/100.;
	    step(tstep,10*tstep);
	    redraw();
	    label(paramname.c_str());
	    hp->set_value(position,tmp,true);
	    // if (!f)
	      hp->eval_below=true;
	  }
	}
	if (Xcas_Text_Editor * m=dynamic_cast<Xcas_Text_Editor *>(wid)){
	  gen g=m->g();
	  if (figure_param_dialog(hp,true,tmin,tmax,tcur,tstep,paramname,g.is_symb_of_sommet(at_assume),tmp) ){
	    minimum(tmin);
	    maximum(tmax);
	    value(tcur);
	    // tstep=tstep*(tmax-tmin)/100.;
	    step(tstep,10*tstep);
	    redraw();
	    label(paramname.c_str());
	    hp->set_value(position,tmp,true);
	    // if (!f)
	      hp->eval_below=true;
	  }
	}
      }
      return 1;
    }
    return Fl_Counter::handle(event);
  }

  Gen_Value_Slider::Gen_Value_Slider(int x,int y,int w,int h,int _pos,double m,double M,double mystep,const std::string & pname):Fl_Counter(x,y,w,h),pos(_pos) { 
    // type(FL_HOR_NICE_SLIDER);  
    // type(FL_HOR_FILL_SLIDER);
    minimum(m); maximum(M); 
    if (mystep>(M-m)/2 || mystep<0)
      mystep=(M-m)/2;
    step(mystep,10*mystep);
    // slider_size(0.05);
    callback(gen_value_slider_cb);
    paramname=pname;
    label(paramname.c_str());
    align(FL_ALIGN_LEFT);
  }

  void gen_value_slider_cb(Fl_Widget * widget,void *){
    if (Gen_Value_Slider * gvs=dynamic_cast<Gen_Value_Slider *>(widget))
      gvs->adjust(true);
  }

  gen Xcas_xyztrange(const gen & g,GIAC_CONTEXT){
    gen res=_xyztrange(g,contextptr);
    // Search if focus is inside a Graph2d group, if so adapt current cfg
    Fl_Widget * w = Fl::focus();
    Graph2d3d * gr=0;
    Figure * fig = 0;
    for (;w;){
      if ( (gr=dynamic_cast<Graph2d3d *>(w)) )
	break;
      if ( (fig=dynamic_cast<Figure *>(w)) ){
	gr = fig->geo;
	break;
      }
      w = w->parent();
    }
    if (gr){
      gr->window_xmin=global_window_xmin;
      gr->window_xmax=global_window_xmax;
      gr->window_ymin=global_window_ymin;
      gr->window_ymax=global_window_ymax;
      // gr->window_zmin=global_window_zmin;
      // gr->window_zmax=global_window_zmax;
      gr->show_axes=giac::show_axes(contextptr);
    }
    return res;
  }

  void Line_Type::draw(){
    xcas_color(color());
    fl_rectf(x(),y(),w(),h());
    int color=(line_type_ & 0xffff);
    xcas_color(color);
    fl_rect(x(),y(),w(),h());
    int width           =(line_type_ & 0x00070000) >> 16; // 3 bits
    int epaisseur_point =(line_type_ & 0x00380000) >> 19; // 3 bits
    epaisseur_point += 2;
    int type_line       =(line_type_ & 0x01c00000) >> 22; // 3 bits
    if (type_line>4)
      type_line=(type_line-4)<<8;
    int type_point      =(line_type_ & 0x0e000000) >> 25; // 3 bits
    int labelpos        =(line_type_ & 0x30000000) >> 28; // 2 bits
    bool fill_polygon   =(line_type_ & 0x40000000) >> 30;
    bool hidden_name    =(line_type_ & 0x80000000) >> 30;    
    if (show_line_){
      fl_line_style(type_line,width+1,0); 
      xcas_color(color); 
      fl_line(x(),y()+h()/2,x()+w(),y()+h()/2);
      fl_line_style(0); // back to default line style
    }
    if (show_pnt_){
      xcas_color(color);
      fltk_point(x(),y(),w()/2,h()/2-2,epaisseur_point,type_point);
    }
    if (show_text_ && !hidden_name){
      xcas_color(color);
      fl_font(FL_HELVETICA,14);
      int dx=2,dy=0;
      if (labelpos==1 || labelpos==2)
	dx=-14;
      if (labelpos==2 || labelpos==3)
	dy=+14;
      fl_draw("A",x()+w()/2+dx,y()+h()/2-2+dy);
    }
    if (show_poly_){
      xcas_color(color);
      if (fill_polygon)
	fl_pie(x()+w()/2,y()+h()/2,w()/2,h()/2,0,360);
      else
	fl_arc(x()+w()/2,y()+h()/2,w()/2,h()/2,0,360);
    }
  }

  Line_Type::Line_Type(int x,int y,int w,int h,int lt):Fl_Button(x,y,w,h),line_type_(lt),show_pnt_(false),show_line_(true),show_text_(false),show_poly_(false) { 
    box(FL_UP_FRAME); 
    color(FL_WHITE);
  }

  void Line_Type::line_type(int l){ 
    line_type_=l; 
    damage(FL_DAMAGE_ALL);
  }


#ifndef NO_NAMESPACE_XCAS
} // namespace giac
#endif // ndef NO_NAMESPACE_XCAS

#endif // HAVE_LIBFLTK
