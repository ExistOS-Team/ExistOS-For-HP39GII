// -*- mode:C++ ; compile-command: "g++-3.4 -I. -I.. -I../include -I../../giac/include -g -c Graph3d.cc -DIN_GIAC -DHAVE_CONFIG_H" -*-
#include "Graph3d.h"
/*
 *  Copyright (C) 2006,2014 B. Parisse, Institut Fourier, 38402 St Martin d'Heres
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
#define __CARBONSOUND__
#ifdef HAVE_LIBFLTK_GL
#include <FL/fl_ask.H>
#include <FL/fl_ask.H>
#include <FL/Fl.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Check_Button.H>
#include <fstream>
#include "vector.h"
#include <algorithm>
#include <fcntl.h>
#include <cmath>
#include <time.h> // for nanosleep
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h> // auto-recovery function
#include "path.h"
#ifndef IN_GIAC
#include <giac/misc.h>
#else
#include "misc.h"
#endif
#include "Equation.h"
#include "Editeur.h"
#include "Xcas1.h"
#include "Print.h"
#include "gl2ps.h"

#ifdef __APPLE__
#include <AGL/agl.h>
#include <FL/x.H>
#include <FL/fl_draw.H>
#define __APPLE_QUARTZ__ 1
#include "Fl_Gl_Choice.H"
extern Fl_Gl_Choice * gl_choice;
// be sure to remove static declaration in gl_start.cxx in fltk src directory
#endif

#ifndef HAVE_PNG_H
#undef HAVE_LIBPNG
#endif
#ifdef HAVE_LIBPNG
#include <png.h>
#endif

using namespace std;
using namespace giac;

#ifndef NO_NAMESPACE_XCAS
namespace xcas {
#endif // ndef NO_NAMESPACE_XCAS

  std::map<std::string,Fl_Image *> texture_cache;

  int Graph3d::opengl2png(const std::string & filename){
#ifdef HAVE_LIBPNG
    if (!screenbuf)
      return -1;
    int i;
    // unsigned rowbytes = w()*4;
    unsigned char *rows[h()];
    for (i = 0; i < h(); i++) {
      rows[i] = &screenbuf[(h() - i - 1)*4*w()];
    }
    int res= write_png(filename.c_str(), rows, w(), h(), PNG_COLOR_TYPE_RGBA, 8);
    if (res!=-1){
      string command="pngtopnm "+filename+" | pnmtops > "+remove_extension(filename)+".ps &";
      cerr << command << endl;
      system(command.c_str()); 
      command="pngtopnm "+filename+" | pnmtojpeg > "+remove_extension(filename)+".jpg &";
      cerr << command << endl;
      system(command.c_str()); 
    }
#endif
    return -1;
  }


  double giac_max(double i,double j){
    return i>j?i:j;
  }

  quaternion_double rotation_2_quaternion_double(double x, double y, double z,double theta){
    double t=theta*M_PI/180;
    double qx,qy,qz,qw,s=std::sin(t/2),c=std::cos(t/2);
    qx=x*s;
    qy=y*s;
    qz=z*s;
    qw=c;
    double n=std::sqrt(qx*qx+qy*qy+qz*qz+qw*qw);
    return quaternion_double(qw/n,qx/n,qy/n,qz/n);
  }

  void Graph3d::resize(int X,int Y,int W,int H){
    int oldh=h(),oldw=w();
    Graph2d3d::resize(X,Y,W,H);
    if (screenbuf){
      if (oldh==H && oldw==W)
	return;
      delete screenbuf;
    }
    screenbuf = new unsigned char[H*W*4];
  }

  Graph3d::Graph3d(int x,int y,int width, int height,const char* title,History_Pack * hp_): 
    Graph2d3d(x,y,width, height, title,hp_),
    theta_z(-110),theta_x(-13),theta_y(-95),
    delta_theta(5),draw_mode(GL_QUADS),printing(0),glcontext(0),dragi(0),dragj(0),push_in_area(false),depth(0),below_depth_hidden(false),screenbuf(0) {
    // end();
    // mode=0;
    display_mode |= 0x80;
    display_mode |= 0x200;
    box(FL_FLAT_BOX);
    couleur=_POINT_WIDTH_5;
    q=euler_deg_to_quaternion_double(theta_z,theta_x,theta_y);
    legende_size=max(min(legende_size,width/4),width/6);
    resize(x,y,width-legende_size,height);
    add_mouse_param_group(x,y,width,height);
    // 8 light initialization
    for (int i=0;i<8;++i)
      reset_light(i);
  }

  // t angle in radians -> r,g,b
  void arc_en_ciel(double t,int & r,int & g,int &b){
    int k=int(t/2/M_PI*126);
    arc_en_ciel(k,r,g,b);
  }

  bool get_glvertex(const vecteur & v,double & d1,double & d2,double & d3,double realiscmplx,double zmin,GIAC_CONTEXT){
    if (v.size()==3){
      gen tmp;
      tmp=evalf_double(v[0],2,contextptr);
      if (tmp.type!=_DOUBLE_) return false;
      d1=tmp._DOUBLE_val;
      tmp=evalf_double(v[1],2,contextptr);
      if (tmp.type!=_DOUBLE_) return false;
      d2=tmp._DOUBLE_val;
      tmp=evalf_double(v[2],2,contextptr);
      if (realiscmplx){
	double arg=0;
	if (realiscmplx<0){
	  d3=evalf_double(im(tmp,contextptr),2,contextptr)._DOUBLE_val-zmin;
	  arg=-d3*realiscmplx;
	}
	else {
	  if (tmp.type==_DOUBLE_){
	    d3=tmp._DOUBLE_val;
	    if (d3<0){
	      arg=M_PI;
	      d3=-d3;
	    }
	  }
	  else {
	    if (tmp.type==_CPLX && tmp._CPLXptr->type==_DOUBLE_ && (tmp._CPLXptr+1)->type==_DOUBLE_){
	      double r=tmp._CPLXptr->_DOUBLE_val;
	      double i=(tmp._CPLXptr+1)->_DOUBLE_val;
	      arg=std::atan2(i,r);
	      d3=std::sqrt(r*r+i*i);
	    }
	    else 
	      return false;
	  }
	} // end realiscmplx>0
	// set color corresponding to argument
	int r,g,b;
	arc_en_ciel(arg,r,g,b);
	glColor3f(r/255.,g/255.,b/255.);
	// glColor4i(r,g,b,int(std::log(d3+1)));
      } // end if (realiscmplx)
      else {
	if (tmp.type!=_DOUBLE_) return false;
	d3=tmp._DOUBLE_val;
      }
      return true;
    }
    return false;
  }

  bool get_glvertex(const gen & g,double & d1,double & d2,double & d3,double realiscmplx,double zmin,GIAC_CONTEXT){
    if (g.type!=_VECT)
      return false;
    return get_glvertex(*g._VECTptr,d1,d2,d3,realiscmplx,zmin,contextptr);
  }

  bool glvertex(const vecteur & v,double realiscmplx,double zmin,GIAC_CONTEXT){
    double d1,d2,d3;
    if (get_glvertex(v,d1,d2,d3,realiscmplx,zmin,contextptr)){
      glVertex3d(d1,d2,d3);
      return true;
    }
    return false;
  }

  void glraster(const vecteur & v){
    if (v.size()==3){
      double d1=evalf_double(v[0],2,context0)._DOUBLE_val;
      double d2=evalf_double(v[1],2,context0)._DOUBLE_val;
      double d3=evalf_double(v[2],2,context0)._DOUBLE_val;
      glRasterPos3d(d1,d2,d3);
    }
  }

  void glraster(const gen & g){
    if (g.type==_VECT)
      glraster(*g._VECTptr);
  }

  // draw s at g with mode= 0 (upper right), 1, 2 or 3
  void Graph3d::legende_draw(const gen & g,const string & s,int mode){
    context * contextptr=hp?hp->contextptr:get_context(this);
    gen gf=evalf_double(g,1,contextptr);
    if (gf.type==_VECT && gf._VECTptr->size()==3){
      double Ax=gf[0]._DOUBLE_val;
      double Ay=gf[1]._DOUBLE_val;
      double Az=gf[2]._DOUBLE_val;
      double Ai,Aj,Ad;
      find_ij(Ax,Ay,Az,Ai,Aj,Ad);
      int di=3,dj=1;
      find_dxdy(s,mode,labelsize(),di,dj);
      find_xyz(Ai+di,Aj+dj,Ad,Ax,Ay,Az);
      glRasterPos3d(Ax,Ay,Az);
      // string s1(s);
      // cst_greek_translate(s1);
      // draw_string(s1);
      draw_string(s);
    }
  }

  void glnormal(const vecteur & v){
    if (v.size()==3){
      double d1=evalf_double(v[0],2,context0)._DOUBLE_val;
      double d2=evalf_double(v[1],2,context0)._DOUBLE_val;
      double d3=evalf_double(v[2],2,context0)._DOUBLE_val;
      glNormal3d(d1,d2,d3);
    }
  }

  void glnormal(double d1,double d2,double d3,double e1,double e2,double e3,double f1,double f2,double f3){
    double de1(e1-d1),de2(e2-d2),de3(e3-d3),df1(f1-d1),df2(f2-d2),df3(f3-d3);
    glNormal3d(de2*df3-de3*df2,de3*df1-de1*df3,de1*df2-de2*df1);
  }

  void gltranslate(const vecteur & v){
    if (v.size()==3){
      double d1=evalf_double(v[0],2,context0)._DOUBLE_val;
      double d2=evalf_double(v[1],2,context0)._DOUBLE_val;
      double d3=evalf_double(v[2],2,context0)._DOUBLE_val;
      glTranslated(d1,d2,d3);
    }
  }


  void iso3d(const double &i1,const double &i2,const double &i3,const double &j1,const double &j2,const double &j3,const double &k1,const double &k2,const double &k3,double & x,double & y,double & z){
    double X=x,Y=y,Z=z;
    x=i1*X+j1*Y+k1*Z;
    y=i2*X+j2*Y+k2*Z;
    z=i3*X+j3*Y+k3*Z;
  }


#define LI 64
#define LH 64
  GLubyte image[LI][LH][3];

  void makeImage(void) {
    int i,j,c;
    for( i = 0 ; i < LI ; i++ ) {
      for( j = 0 ; j < LH ; j++ ) {
	c = (((i&0x8)==0)^
	     ((j&0x8)==0))*255;
	image[i][j][0] =(GLubyte) c;
	image[i][j][1] =(GLubyte) c;
	image[i][j][2] =(GLubyte) c; } }
  }

  bool test_enable_texture(Fl_Image * texture){
    if (!texture)
      return false;
    int depth=-1;
    if (texture->count()==1)
      depth=texture->d();
    if (depth==3 || depth==4){
      // define texture
      // makeImage();
      char * ptr=(char *)texture->data()[0];
      /*
      int W=texture->w(),H=texture->h();
      for (int y=0;y<H;++y){
	for (int x=0;x<W;x++)
	  cerr << unsigned(*(ptr+(x+y*W)*depth)) << " " << unsigned(*(ptr+(x+y*W)*depth+1)) << " " << unsigned(*(ptr+(x+y*W)*depth+2)) << ", ";
	cerr << endl;
      }
      */
      // texture->w() and texture->h() must be a power of 2!!
      glTexImage2D(GL_TEXTURE_2D,0,depth,
		   // LI,LH,
		   texture->w(),texture->h(),
		   0,
		   // GL_RGB,
		   (depth==3?GL_RGB:GL_RGBA),
		   GL_UNSIGNED_BYTE,
		   // &image[0][0][0]);
		   ptr);
      // not periodically
      glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
      glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP); 
      // adjust to nearest 
      glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,
		      // GL_NEAREST);
		      GL_LINEAR);
      glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,
		      // GL_NEAREST);
		      GL_LINEAR);
      //
      glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,
		GL_MODULATE);
      //GL_REPLACE);
      glEnable(GL_TEXTURE_2D);
      return true;
    }
    return false;
  }

  // Sphere centered at center, radius radius, i,j,k orthonormal, ntheta/nphi
  // number of subdivisions
  // mode=GL_QUADS for example
  void glsphere(const vecteur & center,const gen & radius,const vecteur & i0,const vecteur & j0,const vecteur & k0,int ntheta,int nphi,int mode,Fl_Image * texture,GIAC_CONTEXT){
    test_enable_texture(texture);
    double c1=evalf_double(center[0],1,contextptr)._DOUBLE_val; // center
    double c2=evalf_double(center[1],1,contextptr)._DOUBLE_val;
    double c3=evalf_double(center[2],1,contextptr)._DOUBLE_val;
    double r=evalf_double(radius,1,contextptr)._DOUBLE_val;
    double i1=evalf_double(i0[0],1,contextptr)._DOUBLE_val;
    double i2=evalf_double(i0[1],1,contextptr)._DOUBLE_val;
    double i3=evalf_double(i0[2],1,contextptr)._DOUBLE_val;
    double j1=evalf_double(j0[0],1,contextptr)._DOUBLE_val;
    double j2=evalf_double(j0[1],1,contextptr)._DOUBLE_val;
    double j3=evalf_double(j0[2],1,contextptr)._DOUBLE_val;
    double k1=evalf_double(k0[0],1,contextptr)._DOUBLE_val;
    double k2=evalf_double(k0[1],1,contextptr)._DOUBLE_val;
    double k3=evalf_double(k0[2],1,contextptr)._DOUBLE_val;
    double dtheta=2*M_PI/ntheta; // longitude
    double dphi=M_PI/nphi; // latitude
    double jsurtheta=0,isurphi=0,djsurtheta=1.0/ntheta,disurphi=1.0/nphi;
    double parallele1[ntheta+1],parallele2[ntheta+1],parallele3[ntheta+1];
    double x,y,z,oldx,oldy,oldz,X,Z;
    // Set initial parallel to the North pole
    for (int j=0;j<=ntheta;++j){
      parallele1[j]=k1; parallele2[j]=k2; parallele3[j]=k3;
    }
    // parallele1/2/3 contains the coordinate of the previous parallel
    for (int i=1;i<=nphi;i++,isurphi+=disurphi){
      // longitude theta=0, latitude phi=i*dphi
      X=std::sin(i*dphi); Z=std::cos(i*dphi); 
      oldx=X; oldy=0; oldz=Z;
      iso3d(i1,i2,i3,j1,j2,j3,k1,k2,k3,oldx,oldy,oldz);
      double * par1j=parallele1,* par2j=parallele2,*par3j=parallele3;
      jsurtheta=0;
      if (mode==GL_QUADS){
	glBegin(GL_QUAD_STRIP);
	for (int j=0;j<=ntheta;++par1j,++par2j,++par3j,jsurtheta+=djsurtheta){
	  glNormal3d(*par1j,*par2j,*par3j);
	  if (texture)
	    glTexCoord2f(jsurtheta,isurphi);
	  glVertex3d(c1+r*(*par1j),c2+r*(*par2j),c3+r*(*par3j));
	  if (texture)
	    glTexCoord2f(jsurtheta,isurphi+disurphi);
	  glVertex3d(c1+r*oldx,c2+r*oldy,c3+r*oldz);
	  *par1j=oldx; *par2j=oldy; *par3j=oldz;
	  // theta=j*dtheta, phi=i*dphi
	  ++j;
	  x=X*std::cos(j*dtheta); y=X*std::sin(j*dtheta); z=Z;
	  iso3d(i1,i2,i3,j1,j2,j3,k1,k2,k3,x,y,z);
	  oldx=x; oldy=y; oldz=z;
	}
	glEnd();
      }
      else 
      {
	for (int j=0;j<ntheta;){
	  glBegin(mode);
	  glNormal3d(parallele1[j],parallele2[j],parallele3[j]);
	  if (texture)
	    glTexCoord2f(double(j)/(ntheta),double(i-1)/(nphi));
	  glVertex3d(c1+r*parallele1[j],c2+r*parallele2[j],c3+r*parallele3[j]);
	  if (texture)
	    glTexCoord2f(double(j)/(ntheta),double(i)/(nphi));
	  glVertex3d(c1+r*oldx,c2+r*oldy,c3+r*oldz);
	  parallele1[j]=oldx; parallele2[j]=oldy; parallele3[j]=oldz;
	  ++j;
	  // theta=j*dtheta, phi=i*dphi
	  x=X*std::cos(j*dtheta); y=X*std::sin(j*dtheta); z=Z;
	  iso3d(i1,i2,i3,j1,j2,j3,k1,k2,k3,x,y,z);
	  if (texture)
	    glTexCoord2f(double(j+1)/(ntheta),double(i-1)/(nphi));
	  glVertex3d(c1+r*x,c2+r*y,c3+r*z);
	  if (texture)
	    glTexCoord2f(double(j+1)/(ntheta),double(i)/(nphi));
	  glVertex3d(c1+r*parallele1[j],c2+r*parallele2[j],c3+r*parallele3[j]);
	  glEnd();
	  oldx=x; oldy=y; oldz=z;
	}
      }
      parallele1[ntheta]=oldx; parallele2[ntheta]=oldy; parallele3[ntheta]=oldz;
    }
    if (texture)
      glDisable(GL_TEXTURE_2D);
  }

  // if all z values of surfaceg are pure imaginary return a negative int
  // else return 1
  bool find_zscale(const gen & surface,double & zmin,double & zmax){
    if (surface.type!=_VECT)
      return true;
    if (surface.subtype!=_POLYEDRE__VECT && surface.subtype!=_GROUP__VECT && surface._VECTptr->size()==3){
      if (!is_zero(re(surface._VECTptr->back(),context0)))
	return false;
      gen s3=evalf_double(surface._VECTptr->back()/cst_i,2,context0);
      if (s3.type==_DOUBLE_){
	double s3d = s3._DOUBLE_val;
	if (s3d<zmin) zmin=s3d;
	if (s3d>zmax) zmax=s3d;
      }
      return true;
    }
    const_iterateur it=surface._VECTptr->begin(),itend=surface._VECTptr->end();
    for (;it!=itend;++it){
      if (!find_zscale(*it,zmin,zmax))
	return false;
    }
    return true;
  }

  void glsurface(const gen & surfaceg,int draw_mode,Fl_Image * texture,GIAC_CONTEXT){
    if (!ckmatrix(surfaceg,true))
      return;
    test_enable_texture(texture);
    double realiscmplx=has_i(surfaceg),zmin=1e300,zmax=-1e300;
    matrice & surface = *surfaceg._VECTptr;
    if (realiscmplx && !texture){
      if (find_zscale(surface,zmin,zmax))
	realiscmplx=2*M_PI/(zmin-zmax);
    }
    int n=surface.size();
    if (surfaceg.subtype==_POLYEDRE__VECT){
      // implicit surface drawing with the given triangulation
      if (draw_mode==GL_QUADS || draw_mode==GL_TRIANGLES){
	glBegin(GL_TRIANGLES);
	for (int i=0;i<n;++i){
	  vecteur & v=*surface[i]._VECTptr;
	  const_iterateur it=v.begin();
	  double d1,d2,d3,e1,e2,e3,f1,f2,f3;
	  if (get_glvertex(*it,d1,d2,d3,realiscmplx,zmin,contextptr) &&
	      get_glvertex(*(it+1),e1,e2,e3,realiscmplx,zmin,contextptr) &&
	      get_glvertex(*(it+2),f1,f2,f3,realiscmplx,zmin,contextptr)){
	    glnormal(d1,d2,d3,e1,e2,e3,f1,f2,f3);
	    glVertex3d(d1,d2,d3);
	    glVertex3d(e1,e2,e3);
	    glVertex3d(f1,f2,f3);
	  }
	}
	glEnd();
      }
      else {
	glBegin(draw_mode);
	for (int i=0;i<n;++i){
	  vecteur & v=*surface[i]._VECTptr;
	  const_iterateur it=v.begin();
	  double d1,d2,d3,e1,e2,e3,f1,f2,f3;
	  if (get_glvertex(*it,d1,d2,d3,realiscmplx,zmin,contextptr) &&
	      get_glvertex(*(it+1),e1,e2,e3,realiscmplx,zmin,contextptr) &&
	      get_glvertex(*(it+2),f1,f2,f3,realiscmplx,zmin,contextptr)){
	    glnormal(d1,d2,d3,e1,e2,e3,f1,f2,f3);
	    glVertex3d(d1,d2,d3);
	    glVertex3d(e1,e2,e3);
	    glVertex3d(f1,f2,f3);
	    glVertex3d(d1,d2,d3);	    
	  }
	}
	glEnd();
      }
      if (texture)
	glDisable(GL_TEXTURE_2D);
      return;
    }
    if (surface.front()._VECTptr->size()<2){
      if (texture)
	glDisable(GL_TEXTURE_2D);
      return;
    }
    gen a,b,c,d;
    double xt=0,yt,dxt=double(1)/n,dyt;
    for (int i=1;i<n;++i,xt+=dxt){
      const vecteur & vprec=*surface[i-1]._VECTptr;
      const vecteur & v=*surface[i]._VECTptr;
      const_iterateur itprec=vprec.begin(),it=v.begin(),itend=v.end();
      yt=0; dyt=double(1)/(itend-it);
      a=*itprec; b=*it;
      double a1,a2,a3,b1,b2,b3,c1,c2,c3,d1,d2,d3;
      /*
      if (draw_mode==GL_QUADS){
	get_glvertex(b,b1,b2,b3,realiscmplx,zmin,contextptr);
	glBegin(GL_QUAD_STRIP);
	for (;it!=itend;yt+=dyt){
	  a=*itprec;
	  get_glvertex(a,a1,a2,a3,realiscmplx,zmin,contextptr);
	  ++it; ++itprec;
	  if (it==itend){
	    if (texture)
	      glTexCoord2f(xt,yt);
	    glVertex3f(a1,a2,a3);
	    glVertex3f(b1,b2,b3);
	    break;
	  }
	  b=*it;
	  get_glvertex(b,c1,c2,c3,realiscmplx,zmin,contextptr);
	  if (texture)
	    glTexCoord2f(xt,yt);
	  glnormal(a1,a2,a3,b1,b2,b3,c1,c2,c3);
	  glVertex3f(a1,a2,a3);
	  glVertex3f(b1,b2,b3);
	  b1=c1; b2=c2; b3=c3;
	}
	glEnd();
      }
      else 
      */
      {
	get_glvertex(a,a1,a2,a3,realiscmplx,zmin,contextptr);
	get_glvertex(b,b1,b2,b3,realiscmplx,zmin,contextptr);
	++it; ++itprec;
	for (;it!=itend;++it,++itprec,yt+=dyt){
	  c = *itprec;
	  d = *it;
	  get_glvertex(c,c1,c2,c3,realiscmplx,zmin,contextptr);
	  get_glvertex(d,d1,d2,d3,realiscmplx,zmin,contextptr);
	  glBegin(draw_mode);
	  if (texture)
	    glTexCoord2f(xt,yt);
	  glnormal(a1,a2,a3,b1,b2,b3,c1,c2,c3);
	  glVertex3f(a1,a2,a3); // itprec
	  glVertex3f(b1,b2,b3); // it
	  glVertex3f(d1,d2,d3); // it
	  glVertex3f(c1,c2,c3); // itprec
	  glEnd();
	  if (draw_mode==GL_QUADS){
	    glnormal(a1,a2,a3,c1,c2,c3,b1,b2,b3);
	    glBegin(draw_mode);
	    glVertex3f(a1,a2,a3);
	    glVertex3f(c1,c2,c3);
	    glVertex3f(d1,d2,d3);
	    glVertex3f(b1,b2,b3);
	    glEnd();
	  }
	  a=c; a1=c1; a2=c2; a3=c3;
	  b=d; b1=d1; b2=d2; b3=d3;
	}
      }
    }
    if (texture)
      glDisable(GL_TEXTURE_2D);
  }

  // surface without grid evaluation, should not happen!
  void glsurface(const vecteur & point,const gen & uv,double umin,double umax,double vmin,double vmax,int nu,int nv,int draw_mode,GIAC_CONTEXT){
    double u=umin,v=vmin,deltau=(umax-umin)/nu,deltav=(vmax-vmin)/nv;
    vecteur prevline(nv+1); //gen prevline[nv+1];//,line[nv+1];
    vecteur curuv(2);
    gen old,current;
    curuv[0]=u;
    for (int j=0;j<=nv;j++,v += deltav){
      curuv[1] = v;
      prevline[j]=subst(point,uv,curuv,false,contextptr);
    }
    u += deltau;
    for (int i=1;i<=nu;i++,u+=deltau){
      v=vmin;
      curuv = makevecteur(u,v);
      v += deltav;
      old = subst(point,uv,curuv,false,contextptr);
      for (int j=0;j<nv;v +=deltav){
	glBegin(draw_mode);
	curuv[1] = v;
	current = subst(point,uv,curuv,false,contextptr);
	glvertex(*prevline[j]._VECTptr,0,0,contextptr);
	prevline[j]=old;
	glvertex(*old._VECTptr,0,0,contextptr);
	++j;
	glvertex(*current._VECTptr,0,0,contextptr);
	glvertex(*prevline[j]._VECTptr,0,0,contextptr);
	old=current;
	glEnd();
      }
      prevline[nv]=old;
      /* for (int j=0;j<=nv;++j)
	 cerr << prevline[j] << " " ;
	cerr << endl;
      */
    }
  }

  unsigned int line_stipple(unsigned int i){
    switch (i){
    case 1: case 4:
      return 0xf0f0;
    case 2: case 5:
      return 0xcccc;
    case 3: case 6:
      return 0xaaaa;
    default:
      return 0xffff;
    }
  }

  bool is_approx_zero(const gen & dP,double window_xmin,double window_xmax,double window_ymin,double window_ymax,double window_zmin,double window_zmax){
    bool closed=false;
    if (dP.type==_VECT && dP._VECTptr->size()==3){
      closed=true;
      double dPx=evalf_double(dP[0],2,context0)._DOUBLE_val;
      if (fabs(dPx)>(window_xmax-window_xmin)*1e-6)
	closed=false;
      double dPy=evalf_double(dP[1],2,context0)._DOUBLE_val;
      if (fabs(dPy)>(window_ymax-window_ymin)*1e-6)
	closed=false;
      double dPz=evalf_double(dP[2],2,context0)._DOUBLE_val;
      if (fabs(dPz)>(window_zmax-window_zmin)*1e-6)
	closed=false;
    }
    return closed;
  }

  // translate giac GL constant to open GL constant
  unsigned gl_translate(unsigned i){
    switch (i){
    case _GL_LIGHT0:
      return GL_LIGHT0;
    case _GL_LIGHT1:
      return GL_LIGHT1;
    case _GL_LIGHT2:
      return GL_LIGHT2;
    case _GL_LIGHT3:
      return GL_LIGHT3;
    case _GL_LIGHT4:
      return GL_LIGHT4;
    case _GL_LIGHT5:
      return GL_LIGHT5;
    case _GL_AMBIENT:
      return GL_AMBIENT;
    case _GL_SPECULAR:
      return GL_SPECULAR;
    case _GL_DIFFUSE:
      return GL_DIFFUSE;
    case _GL_POSITION:
      return GL_POSITION;
    case _GL_SPOT_DIRECTION:
      return GL_SPOT_DIRECTION;
    case _GL_SPOT_EXPONENT:
      return GL_SPOT_EXPONENT;
    case _GL_SPOT_CUTOFF:
      return GL_SPOT_CUTOFF;
    case _GL_CONSTANT_ATTENUATION:
      return GL_CONSTANT_ATTENUATION;
    case _GL_LINEAR_ATTENUATION:
      return GL_LINEAR_ATTENUATION;
    case _GL_QUADRATIC_ATTENUATION:
      return GL_QUADRATIC_ATTENUATION;
    case _GL_LIGHT_MODEL_AMBIENT:
      return GL_LIGHT_MODEL_AMBIENT;
    case _GL_LIGHT_MODEL_LOCAL_VIEWER:
      return GL_LIGHT_MODEL_LOCAL_VIEWER;
    case _GL_LIGHT_MODEL_TWO_SIDE:
      return GL_LIGHT_MODEL_TWO_SIDE;
#ifndef WIN32
    case _GL_LIGHT_MODEL_COLOR_CONTROL:
      return GL_LIGHT_MODEL_COLOR_CONTROL;
#endif
    case _GL_SMOOTH:
      return GL_SMOOTH;
    case _GL_FLAT:
      return GL_FLAT;
    case _GL_SHININESS:
      return GL_SHININESS;
    case _GL_FRONT:
      return GL_FRONT;
    case _GL_BACK:
      return GL_BACK;
    case _GL_FRONT_AND_BACK:
      return GL_FRONT_AND_BACK;
    case _GL_AMBIENT_AND_DIFFUSE:
      return GL_AMBIENT_AND_DIFFUSE;
    case _GL_EMISSION:
      return GL_EMISSION;
#ifndef WIN32
    case _GL_SEPARATE_SPECULAR_COLOR:
      return GL_SEPARATE_SPECULAR_COLOR;
    case _GL_SINGLE_COLOR:
      return GL_SINGLE_COLOR;
#endif
    case _GL_BLEND:
      return GL_BLEND;
    case _GL_SRC_ALPHA:
      return GL_SRC_ALPHA;
    case _GL_ONE_MINUS_SRC_ALPHA:
      return GL_ONE_MINUS_SRC_ALPHA;
    case _GL_COLOR_INDEXES:
      return GL_COLOR_INDEXES;
    }
    cerr << "No GL equivalent for " << i << endl;
    return i;
  }

  void tran4(double * colmat){
    giac::swapdouble(colmat[1],colmat[4]);
    giac::swapdouble(colmat[2],colmat[8]);
    giac::swapdouble(colmat[3],colmat[12]);
    giac::swapdouble(colmat[6],colmat[9]);
    giac::swapdouble(colmat[7],colmat[13]);
    giac::swapdouble(colmat[11],colmat[14]);    
  }

  void get_texture(const gen & attrv1,Fl_Image * & texture){
    // set texture
    if (attrv1.type==_STRNG){
      std::map<std::string,Fl_Image *>::const_iterator it,itend=texture_cache.end();
      it=texture_cache.find(*attrv1._STRNGptr);
      if (it!=itend){
	texture=it->second;
	// texture->uncache();
      }
      else {
	texture=Fl_Shared_Image::get(attrv1._STRNGptr->c_str());
	if (texture){
	  int W=texture->w(),H=texture->h();
	  // take a power of 2 near w/h
	  W=1 << min(int(std::log(double(W))/std::log(2.)+.5),8);
	  H=1 << min(int(std::log(double(H))/std::log(2.)+.5),8);
	  texture=texture->copy(W,H);
	  texture_cache[*attrv1._STRNGptr]=texture;
	}
      }
    }
  }

  int gen2int(const gen & g){
    if (g.type==_INT_)
      return g.val;
    if (g.type==_DOUBLE_)
      return int(g._DOUBLE_val);
    setsizeerr(gettext("Unable to convert to int")+g.print());
    return -1;
  }

  void Graph3d::indraw(const giac::gen & g){
    context * contextptr=hp?hp->contextptr:get_context(this);
    if (g.type==_VECT)
      indraw(*g._VECTptr);
    if (g.is_symb_of_sommet(at_animation)){
      indraw(get_animation_pnt(g,animation_instructions_pos));
      return;
    }
    if (!g.is_symb_of_sommet(at_pnt)){
      update_infos(g,contextptr);
      return;
    }
    gen & f=g._SYMBptr->feuille;
    if (f.type!=_VECT)
      return;
    vecteur & v = *f._VECTptr;
    gen v0=v[0];
    bool est_hyperplan=v0.is_symb_of_sommet(at_hyperplan);
    string legende;
    vecteur style(get_style(v,legende));
    int styles=style.size();
    // color
    bool hidden_name = false;
    int ensemble_attributs=style.front().val;
    if (style.front().type==_ZINT){
      ensemble_attributs = mpz_get_si(*style.front()._ZINTptr);
      hidden_name=true;
    }
    else
      hidden_name=ensemble_attributs<0;
    int couleur=ensemble_attributs & 0x0000ff;
    int width           =(ensemble_attributs & 0x00070000) >> 16; // 3 bits
    int epaisseur_point =((ensemble_attributs & 0x00380000) >> 19)+1; // 3 bits
    int type_line       =(ensemble_attributs & 0x01c00000) >> 22; // 3 bits
    int type_point      =(ensemble_attributs & 0x0e000000) >> 25; // 3 bits
    int labelpos        =(ensemble_attributs & 0x30000000) >> 28; // 2 bits
    bool fill_polygon   =(ensemble_attributs & 0x40000000) >> 30;
    hidden_name = hidden_name || legende.empty();
    Fl_Image * texture=0;
    glLineWidth(width+1);
    gl2psLineWidth(width);
    // FIXME line_stipple disabled because of printing
    if (!printing)
      glLineStipple(1,line_stipple(type_line));
    else
      glLineStipple(1,0xffff);
    glPointSize(epaisseur_point);
    gl2psPointSize(epaisseur_point);
    if (styles<=2){
      glEnable(GL_COLOR_MATERIAL);
      glColorMaterial(GL_FRONT,GL_AMBIENT_AND_DIFFUSE);
    }
    else 
      glDisable(GL_COLOR_MATERIAL);
    GLfloat tab[4]={0,0,0,1};
    glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,tab);
    glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,50);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,tab);
    GLfloat tab1[4]={0.2,0.2,0.2,1};
    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,tab1);
    GLfloat tab2[4]={0.8,0.8,0.8,1};
    glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,tab2);
    if (est_hyperplan){
      glPolygonMode(GL_FRONT_AND_BACK,fill_polygon?GL_FILL:GL_LINE);
    }
    else {
      glPolygonMode(GL_FRONT_AND_BACK,fill_polygon?GL_FILL:GL_LINE);
      // glPolygonMode(GL_BACK,GL_POINT);
      // glMaterialfv(GL_BACK,GL_EMISSION,tab);
      // glMaterialfv(GL_BACK,GL_SPECULAR,tab);
      // glMaterialfv(GL_BACK,GL_AMBIENT_AND_DIFFUSE,tab);
    }
    // glGetFloatv(GL_CURRENT_COLOR,tab);

    // set other style attributs : 
    /* material = [gl_front|gl_back|gl_front_and_back,gl_shininess,valeur] or
       material = [gl_front|gl_back|gl_front_and_back, GL_AMBIENT| GL_DIFFUSE |
       GL_SPECULAR | GL_EMISSION | GL_AMBIENT_AND_DIFFUSE | GL_COLOR_INDEXES,
      [r,g,b,a] ]
      last arg is [ambient,diffuse,specular] for GL_COLOR_INDEXES
      or
      material=[gl_texture,"image_filename",...]
    */
    for (int i=2;i<styles;++i){
      gen & attr=style[i];
      if (attr.type==_VECT){
	vecteur & attrv=*attr._VECTptr;
	if (attrv.size()>=2 && attrv.front().type==_INT_ && attrv.front().val==_GL_TEXTURE){
	  get_texture(attrv[1],texture);
	  continue;
	}
	if (attrv.size()==2 && attrv.front().type==_INT_ && attrv.front().val==_GL_MATERIAL){
	  gen attrm =evalf_double(attrv.back(),1,contextptr);
	  if (debug_infolevel)
	    cerr << "Setting material " << attrm << endl;
	  if (attrm.type==_VECT && attrm._VECTptr->size()<=3 ){
	    gen attrv0=attrv.back()._VECTptr->front();
	    if (attrv0.type==_INT_ && attrv0.val==_GL_TEXTURE){
	      gen attrv1=(*attrv.back()._VECTptr)[1];
	      get_texture(attrv1,texture);
	      continue;
	    }
	    vecteur & attrmv = *attrm._VECTptr;
	    if (attrmv.back().type==_VECT && attrmv.back()._VECTptr->size()==4){
	      vecteur & w=*attrmv.back()._VECTptr;
	      GLfloat tab[4]={w[0]._DOUBLE_val,w[1]._DOUBLE_val,w[2]._DOUBLE_val,w[3]._DOUBLE_val};
	      glMaterialfv(gl_translate(gen2int(attrmv[0])),gl_translate(gen2int(attrmv[1])),tab);
	      continue;
	    }
	    if (attrmv.back().type==_VECT && attrmv.back()._VECTptr->size()==3){
	      vecteur & w=*attrmv.back()._VECTptr;
	      GLfloat tab[3]={w[0]._DOUBLE_val,w[1]._DOUBLE_val,w[2]._DOUBLE_val};
	      glMaterialfv(gl_translate(gen2int(attrmv[0])),GL_COLOR_INDEXES,tab);
	      continue;
	    }
	    if (attrmv.back().type==_DOUBLE_ && attrmv[1]._DOUBLE_val==_GL_SHININESS){
	      // glMaterialf(gl_translate(gen2int(attrmv[0])),GL_SHININESS,float(attrmv[2]._DOUBLE_val));
	      glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,float(attrmv[2]._DOUBLE_val));
	      continue;
	    }
	  }
	}
      }
    }
    if (texture){
      fill_polygon=true;
      couleur=FL_WHITE;
      glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
    }
    bool hidden_line = fill_polygon && (width==7 || (display_mode & 0x8) || texture );
    xcas_color(couleur,true);
    if (debug_infolevel){
      cerr << "opengl displaying " << g << endl;
      GLint b;
      GLfloat posf[4],direcf[4],ambient[4],diffuse[4],specular[4],emission[4],shini[1];
      GLfloat expo,cutoff;
      double pos[4],direc[4];
      glGetIntegerv(GL_BLEND,&b);
      cerr << "blend " << b << endl;
      for (int i=0;i<8;++i){
	glGetIntegerv(GL_LIGHT0+i,&b);
	if (b){
	  glGetLightfv(GL_LIGHT0+i,GL_SPOT_EXPONENT,&expo);
	  glGetLightfv(GL_LIGHT0+i,GL_SPOT_CUTOFF,&cutoff);
	  glGetLightfv(GL_LIGHT0+i,GL_POSITION,posf);
	  glGetLightfv(GL_LIGHT0+i,GL_SPOT_DIRECTION,direcf);
	  direcf[4]=0;
	  mult4(model_inv,posf,pos);
	  tran4(model);
	  mult4(model,direcf,direc);
	  tran4(model);
	  glGetLightfv(GL_LIGHT0+i,GL_AMBIENT,ambient);
	  glGetLightfv(GL_LIGHT0+i,GL_DIFFUSE,diffuse);
	  glGetLightfv(GL_LIGHT0+i,GL_SPECULAR,specular);
	  cerr << "light " << i << ": " <<
	    " pos " << pos[0] << "," << pos[1] << "," << pos[2] << "," << pos[3] << 
	    " dir " << direc[0] << "," << direc[1] << "," << direc[2] << "," << direc[3] << 
	    " ambient " << ambient[0] << "," << ambient[1] << "," << ambient[2] << "," << ambient[3] << 
	    " diffuse " << diffuse[0] << "," << diffuse[1] << "," << diffuse[2] << "," << diffuse[3] << 
	    " specular " << specular[0] << "," << specular[1] << "," << specular[2] << "," << specular[3] << 
	    " exponent " << expo << " cutoff " << cutoff << endl;
	}
      }
      // material colors
      glGetMaterialfv(GL_FRONT,GL_AMBIENT,ambient);
      glGetMaterialfv(GL_FRONT,GL_DIFFUSE,diffuse);
      glGetMaterialfv(GL_FRONT,GL_SPECULAR,specular);
      glGetMaterialfv(GL_FRONT,GL_EMISSION,emission);
      glGetMaterialfv(GL_FRONT,GL_SHININESS,shini);
      cerr << "front " << ": " <<
	" ambient " << ambient[0] << "," << ambient[1] << "," << ambient[2] << "," << ambient[3] << 
	" diffuse " << diffuse[0] << "," << diffuse[1] << "," << diffuse[2] << "," << diffuse[3] << 
	" specular " << specular[0] << "," << specular[1] << "," << specular[2] << "," << specular[3] << 
	" emission " << emission[0] << "," << emission[1] << "," << emission[2] << "," << emission[3] << 
	" shininess " << shini[0] << endl;
      glGetMaterialfv(GL_BACK,GL_AMBIENT,ambient);
      glGetMaterialfv(GL_BACK,GL_DIFFUSE,diffuse);
      glGetMaterialfv(GL_BACK,GL_SPECULAR,specular);
      glGetMaterialfv(GL_BACK,GL_EMISSION,emission);
      glGetMaterialfv(GL_BACK,GL_SHININESS,shini);
      cerr << "back " << ": " <<
	" ambient " << ambient[0] << "," << ambient[1] << "," << ambient[2] << "," << ambient[3] << 
	" diffuse " << diffuse[0] << "," << diffuse[1] << "," << diffuse[2] << "," << diffuse[3] << 
	" specular " << specular[0] << "," << specular[1] << "," << specular[2] << "," << specular[3] << 
	" emission " << emission[0] << "," << emission[1] << "," << emission[2] << "," << emission[3] << 
	" shininess " << shini[0] << endl;	
    }
    if (est_hyperplan){
      vecteur P,n;
      if (!hyperplan_normal_point(v0,n,P))
	return;
      P=*evalf_double(P,1,contextptr)._VECTptr;
      vecteur Porig(P);
      double n1=evalf_double(n[0],1,contextptr)._DOUBLE_val;
      double n2=evalf_double(n[1],1,contextptr)._DOUBLE_val;
      double n3=evalf_double(n[2],1,contextptr)._DOUBLE_val;
      if (fill_polygon){
	vecteur v1,v2;
	if (!normal3d(n,v1,v2))
	  return;
	double v11=evalf_double(v1[0],1,contextptr)._DOUBLE_val;
	double v12=evalf_double(v1[1],1,contextptr)._DOUBLE_val;
	double v13=evalf_double(v1[2],1,contextptr)._DOUBLE_val;
	double v21=evalf_double(v2[0],1,contextptr)._DOUBLE_val;
	double v22=evalf_double(v2[1],1,contextptr)._DOUBLE_val;
	double v23=evalf_double(v2[2],1,contextptr)._DOUBLE_val;
	// moves P along v1 so that one coordinate is in clip
	if (std::abs(v11)>=std::abs(v12) && std::abs(v11)>=std::abs(v13)){
	  P=addvecteur(P,multvecteur((window_xmin-P[0])/v11,v1));
	}
	if (std::abs(v12)>std::abs(v11) && std::abs(v12)>=std::abs(v13)){
	  P=addvecteur(P,multvecteur((window_ymin-P[1])/v12,v1));
	}
	if (std::abs(v13)>std::abs(v11) && std::abs(v13)>std::abs(v12)){
	  P=addvecteur(P,multvecteur((window_zmin-P[2])/v13,v1));
	}
	// find a large constant so that the points are outside clipping
	double v31=v11+v21,v32=v12+v22,v33=v13+v23,v41=v11-v21,v42=v12-v22,v43=v13-v23;
	double nv3=std::sqrt(v31*v31+v32*v32+v33*v33);
	double nv4=std::sqrt(v41*v41+v42*v42+v43*v43);
	double lambda=1;
	if (std::abs(v31)>0.1*nv3)
	  lambda=giac_max(lambda,(window_xmax-window_xmin)/std::abs(v31));
	if (std::abs(v32)>0.1*nv3)
	  lambda=giac_max(lambda,(window_ymax-window_ymin)/std::abs(v32));
	if (std::abs(v33)>0.1*nv3)
	  lambda=giac_max(lambda,(window_zmax-window_zmin)/std::abs(v33));
	if (std::abs(v41)>0.1*nv4)
	  lambda=giac_max(lambda,(window_xmax-window_xmin)/std::abs(v41));
	if (std::abs(v42)>0.1*nv4)
	  lambda=giac_max(lambda,(window_ymax-window_ymin)/std::abs(v42));
	if (std::abs(v43)>0.1*nv4)
	  lambda=giac_max(lambda,(window_zmax-window_zmin)/std::abs(v43));
	lambda *= 2;
	if (display_mode & 0x8){
	  // if lighting enabled, we must draw small quads otherwise
	  // light will be incorrectly rendered (vertices too far)
	  // divide v1 and v2 by 10 (100 quads)
	  P=subvecteur(P,multvecteur(lambda,addvecteur(v1,v2))); // base point
	  unsigned hyperplan_light_rep=10;
	  v1=multvecteur(2*double(lambda)/hyperplan_light_rep,v1);
	  v2=multvecteur(2*double(lambda)/hyperplan_light_rep,v2);
	  for (unsigned j=1;j<hyperplan_light_rep;++j){
	    vecteur P1=addvecteur(P,multvecteur(int(j-1),v2));
	    for (unsigned i=1;i<hyperplan_light_rep;++i){
	      vecteur P2(addvecteur(P1,v1));
	      vecteur P3(addvecteur(P2,v2));
	      vecteur P4(addvecteur(P1,v2));
	      glBegin(GL_QUADS);
	      glNormal3d(n1,n2,n3);
	      glvertex(P1,0,0,contextptr);
	      glvertex(P2,0,0,contextptr);
	      glvertex(P3,0,0,contextptr);
	      glvertex(P4,0,0,contextptr);
	      glNormal3d(-n1,-n2,-n3);
	      glvertex(P1,0,0,contextptr);
	      glvertex(P4,0,0,contextptr);
	      glvertex(P3,0,0,contextptr);
	      glvertex(P2,0,0,contextptr);
	      glEnd();
	      // cerr << P1 << "," << P2 << "," << P3 << "," << P4 << endl;
	      P1=P2;
	    }
	  }
	} // end if (display_mode & 0x8)
	else {
	  // Plan representation: face P+lambda*(-v1-v2), P+lambda*(-v1+v2), ...
	  vecteur P1(subvecteur(P,multvecteur(lambda,addvecteur(v1,v2))));
	  vecteur P2(subvecteur(P,multvecteur(lambda,addvecteur(v1,-v2))));
	  vecteur P4(addvecteur(P,multvecteur(lambda,addvecteur(v1,-v2))));
	  vecteur P3(addvecteur(P,multvecteur(lambda,addvecteur(v1,v2))));
	  glBegin(GL_QUADS);
	  // normal not needed (light not enabled)
	  glNormal3d(n1,n2,n3);
	  glvertex(P1,0,0,contextptr);
	  glvertex(P2,0,0,contextptr);
	  glvertex(P3,0,0,contextptr);
	  glvertex(P4,0,0,contextptr);
	  glNormal3d(-n1,-n2,-n3);
	  glvertex(P1,0,0,contextptr);
	  glvertex(P4,0,0,contextptr);
	  glvertex(P3,0,0,contextptr);
	  glvertex(P2,0,0,contextptr);
	  glEnd();
	}
      } // end fill_polygon
      else { // use equation
	glNormal3d(n1,n2,n3);
	if (std::abs(n1)>=std::abs(n2) && std::abs(n1)>=std::abs(n3)){
	  // x=a*y+b*z+c
	  double a=-n2/n1, b=-n3/n1;
	  double c=evalf_double(P[0]-a*P[1]-b*P[2],1,contextptr)._DOUBLE_val;
	  double dy=(window_ymax-window_ymin)/10;
	  for (int j=0;j<=10;++j){
	    double y=window_ymin+j*dy;
	    glBegin(GL_LINES);
	    glVertex3d(a*y+b*window_zmin+c,y,window_zmin);
	    glVertex3d(a*y+b*window_zmax+c,y,window_zmax);
	    glEnd();
	  }
	  double dz=(window_zmax-window_zmin)/10;
	  for (int j=0;j<=10;++j){
	    double z=window_zmin+j*dz;
	    glBegin(GL_LINES);
	    glVertex3d(a*window_ymin+b*z+c,window_ymin,z);
	    glVertex3d(a*window_ymax+b*z+c,window_ymax,z);
	    glEnd();
	  }
	}
	if (std::abs(n2)>std::abs(n1) && std::abs(n2)>=std::abs(n3)){
	  // y=a*x+b*z+c
	  double a=-n1/n2, b=-n3/n2;
	  double c=evalf_double(P[1]-a*P[0]-b*P[2],1,contextptr)._DOUBLE_val;
	  double dx=(window_xmax-window_xmin)/10;
	  for (int j=0;j<=10;++j){
	    double x=window_xmin+j*dx;
	    glBegin(GL_LINES);
	    glVertex3d(x,a*x+b*window_zmin+c,window_zmin);
	    glVertex3d(x,a*x+b*window_zmax+c,window_zmax);
	    glEnd();
	  }
	  double dz=(window_zmax-window_zmin)/10;
	  for (int j=0;j<=10;++j){
	    double z=window_zmin+j*dz;
	    glBegin(GL_LINES);
	    glVertex3d(window_xmin,a*window_xmin+b*z+c,z);
	    glVertex3d(window_xmax,a*window_xmax+b*z+c,z);
	    glEnd();
	  }
	}
	if (std::abs(n3)>std::abs(n1) && std::abs(n3)>std::abs(n2)){
	  // z=a*x+b*y+c
	  double a=-n1/n3, b=-n2/n3;
	  double c=evalf_double(P[2]-a*P[0]-b*P[1],1,contextptr)._DOUBLE_val;
	  double dx=(window_xmax-window_xmin)/10;
	  for (int j=0;j<=10;++j){
	    double x=window_xmin+j*dx;
	    glBegin(GL_LINES);
	    glVertex3d(x,window_ymin,a*x+b*window_ymin+c);
	    glVertex3d(x,window_ymax,a*x+b*window_ymax+c);
	    glEnd();
	  }
	  double dy=(window_ymax-window_ymin)/10;
	  for (int j=0;j<=10;++j){
	    double y=window_ymin+j*dy;
	    glBegin(GL_LINES);
	    glVertex3d(window_xmin,y,a*window_xmin+b*y+c);
	    glVertex3d(window_xmax,y,a*window_xmax+b*y+c);
	    glEnd();
	  }
	}
      }
      if (!hidden_name && show_names) 
	legende_draw(Porig,legende,labelpos);
      return;
    }
    if (v0.is_symb_of_sommet(at_hypersphere)){
      gen & f=v0._SYMBptr->feuille;
      if (f.type==_VECT && f._VECTptr->size()>=2){
	vecteur & v=*f._VECTptr;
	// Check that center is a 3-d point
	if (v.front().type==_VECT && v.front()._VECTptr->size()==3){
	  // Radius
	  gen r=v[1];
	  if (r.type==_VECT && r._VECTptr->size()==3)
	    r=l2norm(*r._VECTptr,contextptr); 
	  // Direction of axis, parallels and meridiens
	  vecteur dir1(makevecteur(1,0,0)),dir2(makevecteur(0,1,0)),dir3(makevecteur(0,0,1));
	  if (v.size()>=3 && v[2].type==_VECT && v[2]._VECTptr->size()==3){
	    dir3=*v[2]._VECTptr;
	    dir3=divvecteur(dir3,sqrt(dotvecteur(dir3,dir3),contextptr));
	    if (v.size()>=4 && v[3].type==_VECT && v[3]._VECTptr->size()==3){
	      dir1=*v[3]._VECTptr;
	      dir1=divvecteur(dir1,sqrt(dotvecteur(dir1,dir1),contextptr));
	    }
	    else {
	      if (!is_zero(dir3[0]) || !is_zero(dir3[1]) ){
		dir1=makevecteur(-dir3[1],dir3[0],0);
		dir1=divvecteur(dir1,sqrt(dotvecteur(dir1,dir1),contextptr));
	      }
	    }
	    dir2=cross(dir3,dir1,contextptr);
	  }
	  // optional discretisation info for drawing
	  if (v.size()>=5 && v[4].type==_INT_){
	    nphi=max(absint(v[4].val),3);
	    ntheta=ntheta;
	  }
	  if (v.size()>=6 && v[5].type==_INT_){
	    ntheta=max(absint(v[5].val),3);
	  }
	  // Now make the sphere
	  if (fill_polygon){
	    glsphere(*v.front()._VECTptr,r,dir1,dir2,dir3,ntheta,nphi,GL_QUADS,texture,contextptr);
	  }
	  if (!hidden_line){
	    if (fill_polygon)
	      glColor3d(0,0,0);
	    glsphere(*v.front()._VECTptr,r,dir1,dir2,dir3,min(ntheta,36),min(nphi,36),GL_LINE_LOOP,texture,contextptr);
	    xcas_color(couleur,true);
	  }
	  if (!hidden_name && show_names) legende_draw(v.front(),legende,labelpos);
	}
      }
      return;
    }
    if (v0.is_symb_of_sommet(at_curve) && v0._SYMBptr->feuille.type==_VECT && !v0._SYMBptr->feuille._VECTptr->empty()){
      gen & f = v0._SYMBptr->feuille._VECTptr->front();
      // f = vect[ pnt,var,xmin,xmax ]
      if (f.type==_VECT && f._VECTptr->size()>=4){
	vecteur & vf = *f._VECTptr;
	if (vf.size()>4){
	  gen poly=vf[4];
	  if (ckmatrix(poly)){
	    const_iterateur it=poly._VECTptr->begin(),itend=poly._VECTptr->end();
	    bool closed=fill_polygon && !(display_mode & 0x8); // ?is_approx_zero(poly._VECTptr->back()-poly._VECTptr->front(),window_xmin,window_xmax,window_ymin,window_ymax,window_zmin,window_zmax):false;
	    if (it->_VECTptr->size()==3){
	      glBegin(closed?GL_POLYGON:GL_LINE_STRIP);
	      // if (closed) ++it;
	      for (;it!=itend;++it){
		if (!glvertex(*it->_VECTptr,0,0,contextptr)){
		  glEnd();
		  glBegin(closed?GL_POLYGON:GL_LINE_STRIP);
		}
	      }
	      glEnd();
	      if (!hidden_name && show_names && !poly._VECTptr->empty()) legende_draw(poly._VECTptr->front(),legende,labelpos);
	      return ;
	    }
	  }
	}
	gen point=vf[0];
	gen var=vf[1];
	gen mini=vf[2];
	gen maxi=vf[3];
	bool closed=false;
	if (fill_polygon && !(display_mode & 0x8) )
	  closed=is_approx_zero(subst(point,var,mini,false,contextptr)-subst(point,var,maxi,false,contextptr),window_xmin,window_xmax,window_ymin,window_ymax,window_zmin,window_zmax);
	int n=nphi*10;
	gen delta=(maxi-mini)/n;
	glBegin(closed?GL_POLYGON:GL_LINE_STRIP);
	for (int i=closed?1:0;i<=n;++i){
	  if (!glvertex(*subst(point,var,mini,false,contextptr)._VECTptr,0,0,contextptr)){
	    glEnd();
	    glBegin(closed?GL_POLYGON:GL_LINE_STRIP);
	  }
	  mini = mini + delta;
	}
	glEnd();
	if (!hidden_name && show_names) legende_draw(mini,legende,labelpos);
      }
      return;
    }
    if (v0.is_symb_of_sommet(at_hypersurface)){
      gen & f = v0._SYMBptr->feuille;
      if (f.type!=_VECT || f._VECTptr->size()<3)
	return;
      gen & tmp = f._VECTptr->front();
      if (tmp.type!=_VECT || tmp._VECTptr->size()<4)
	return;
      if (tmp._VECTptr->size()>4){
	if (!fill_polygon)
	  glsurface((*tmp._VECTptr)[4],GL_LINE_LOOP,texture,contextptr);
	else {
	  glsurface((*tmp._VECTptr)[4],GL_QUADS,texture,contextptr);
	  // glLineWidth(width+2);
	  if (!hidden_line){
	    glColor3d(0,0,0);
	    glsurface((*tmp._VECTptr)[4],GL_LINE_LOOP,texture,contextptr);
	    xcas_color(couleur,true);
	  }
	}
	// if (!hidden_name && show_names) legende_draw(legende.c_str());
	return;
      }
      gen point = tmp._VECTptr->front(); // [x(u,v),y(u,v),z(u,v)]
      gen vars = (*tmp._VECTptr)[1]; // [u,v]
      gen mini = (*tmp._VECTptr)[2]; // [umin,vmin]
      gen maxi = (*tmp._VECTptr)[3]; // [umax,vmax]
      if (!check3dpoint(point) || 
	  vars.type!=_VECT || vars._VECTptr->size()!=2 
	  || mini.type!=_VECT || mini._VECTptr->size()!=2 
	  || maxi.type!=_VECT || maxi._VECTptr->size()!=2 )
	return;
      double umin=evalf_double(mini._VECTptr->front(),1,contextptr)._DOUBLE_val;
      double vmin=evalf_double(mini._VECTptr->back(),1,contextptr)._DOUBLE_val;
      double umax=evalf_double(maxi._VECTptr->front(),1,contextptr)._DOUBLE_val;
      double vmax=evalf_double(maxi._VECTptr->back(),1,contextptr)._DOUBLE_val;
      if (fill_polygon){
	glsurface(*point._VECTptr,vars,umin,umax,vmin,vmax,ntheta,nphi,GL_QUADS,contextptr);
	if (!hidden_line){
	  glColor3d(0,0,0);
	  glsurface(*point._VECTptr,vars,umin,umax,vmin,vmax,ntheta,nphi,GL_LINE_LOOP,contextptr);
	  xcas_color(couleur,true);
	}
      }
      else
	glsurface(*point._VECTptr,vars,umin,umax,vmin,vmax,ntheta,nphi,GL_LINE_LOOP,contextptr);
      // if (!hidden_name && show_names) legende_draw(?,legende,labelpos);
      return;
    }
    if (v0.type==_VECT && v0.subtype==_POINT__VECT && v0._VECTptr->size()==3 ){
      gen A(evalf_double(v0,1,contextptr));
      if (A.type==_VECT && A._VECTptr->size()==3 && type_point!=4){
	double xA=A._VECTptr->front()._DOUBLE_val;
	double yA=(*A._VECTptr)[1]._DOUBLE_val;
	double zA=A._VECTptr->back()._DOUBLE_val;
	double iA,jA,depthA;
	find_ij(xA,yA,zA,iA,jA,depthA);
	glLineWidth(1+epaisseur_point/2);
	switch(type_point){ 
	case 0:
	  glBegin(GL_LINES);
	  find_xyz(iA-epaisseur_point,jA-epaisseur_point,depthA,xA,yA,zA);
	  glVertex3d(xA,yA,zA);
	  find_xyz(iA+epaisseur_point,jA+epaisseur_point,depthA,xA,yA,zA);
	  glVertex3d(xA,yA,zA);
	  find_xyz(iA-epaisseur_point,jA+epaisseur_point,depthA,xA,yA,zA);
	  glVertex3d(xA,yA,zA);
	  find_xyz(iA+epaisseur_point,jA-epaisseur_point,depthA,xA,yA,zA);
	  glVertex3d(xA,yA,zA);
	  glEnd();
	  break;
	case 1:
	  // 1 losange, 
	  glBegin(GL_LINE_LOOP);
	  find_xyz(iA-epaisseur_point,jA,depthA,xA,yA,zA);
	  glVertex3d(xA,yA,zA);
	  find_xyz(iA,jA+epaisseur_point,depthA,xA,yA,zA);
	  glVertex3d(xA,yA,zA);
	  find_xyz(iA+epaisseur_point,jA,depthA,xA,yA,zA);
	  glVertex3d(xA,yA,zA);
	  find_xyz(iA,jA-epaisseur_point,depthA,xA,yA,zA);
	  glVertex3d(xA,yA,zA);
	  glEnd();
	  break;
	case 2:	  // 2 croix verticale, 
	  glBegin(GL_LINES);
	  find_xyz(iA,jA-epaisseur_point,depthA,xA,yA,zA);
	  glVertex3d(xA,yA,zA);
	  find_xyz(iA,jA+epaisseur_point,depthA,xA,yA,zA);
	  glVertex3d(xA,yA,zA);
	  find_xyz(iA-epaisseur_point,jA,depthA,xA,yA,zA);
	  glVertex3d(xA,yA,zA);
	  find_xyz(iA+epaisseur_point,jA,depthA,xA,yA,zA);
	  glVertex3d(xA,yA,zA);
	  glEnd();
	  break;
	case 3: // 3 carre. 
	  glBegin(GL_LINE_LOOP);
	  find_xyz(iA-epaisseur_point,jA-epaisseur_point,depthA,xA,yA,zA);
	  glVertex3d(xA,yA,zA);
	  find_xyz(iA+epaisseur_point,jA-epaisseur_point,depthA,xA,yA,zA);
	  glVertex3d(xA,yA,zA);
	  find_xyz(iA+epaisseur_point,jA+epaisseur_point,depthA,xA,yA,zA);
	  glVertex3d(xA,yA,zA);
	  find_xyz(iA-epaisseur_point,jA+epaisseur_point,depthA,xA,yA,zA);
	  glVertex3d(xA,yA,zA);
	  glEnd();
	  break;
	case 5: // 5 triangle, 
	  glBegin(GL_LINE_LOOP);
	  find_xyz(iA+epaisseur_point,jA-epaisseur_point,depthA,xA,yA,zA);
	  glVertex3d(xA,yA,zA);
	  find_xyz(iA+epaisseur_point,jA+epaisseur_point,depthA,xA,yA,zA);
	  glVertex3d(xA,yA,zA);
	  find_xyz(iA-epaisseur_point,jA,depthA,xA,yA,zA);
	  glVertex3d(xA,yA,zA);
	  glEnd();
	  break;
	case 6:  // 6 etoile, 
	  glBegin(GL_LINES);
	  find_xyz(iA,jA-epaisseur_point,depthA,xA,yA,zA);
	  glVertex3d(xA,yA,zA);
	  find_xyz(iA,jA+epaisseur_point,depthA,xA,yA,zA);
	  glVertex3d(xA,yA,zA);
	  find_xyz(iA-epaisseur_point/2,jA+epaisseur_point,depthA,xA,yA,zA);
	  glVertex3d(xA,yA,zA);
	  find_xyz(iA+epaisseur_point/2,jA-epaisseur_point,depthA,xA,yA,zA);
	  glVertex3d(xA,yA,zA);
	  find_xyz(iA-epaisseur_point/2,jA-epaisseur_point,depthA,xA,yA,zA);
	  glVertex3d(xA,yA,zA);
	  find_xyz(iA+epaisseur_point/2,jA+epaisseur_point,depthA,xA,yA,zA);
	  glVertex3d(xA,yA,zA);
	  glEnd();
	  break;
	default: 	  // 7 point
	  glBegin(GL_POINTS);
	  glvertex(*v0._VECTptr,0,0,contextptr);
	  glEnd();
	}
	glLineWidth(width+1);
      }
      if (!hidden_name && show_names) 
	legende_draw(v0,legende,labelpos);
      return;
    }
    if (v0.type==_VECT && v0.subtype!=_POINT__VECT){
      vecteur & vv0=*v0._VECTptr;
      vecteur w,lastpnt;
      if (v0.subtype==_POLYEDRE__VECT){
	// each element of v is a face 
	const_iterateur it=vv0.begin(),itend=vv0.end();
	for (;it!=itend;++it){
	  if (it->type==_VECT){ 
	    w = *evalf_double(*it,1,contextptr)._VECTptr;
	    int s=w.size();
	    if (s<3)
	      continue;
	    double d1,d2,d3,e1,e2,e3,f1,f2,f3;	    
	    if (get_glvertex(w[0],d1,d2,d3,0,0,contextptr) &&
		get_glvertex(w[1],e1,e2,e3,0,0,contextptr) &&
		get_glvertex(w[2],f1,f2,f3,0,0,contextptr) ){
	      glnormal(d1,d2,d3,e1,e2,e3,f1,f2,f3);
	      if (fill_polygon){
		glBegin(GL_POLYGON);
		glVertex3d(d1,d2,d3);
		glVertex3d(e1,e2,e3);
		glVertex3d(f1,f2,f3);
		for (int j=3;j<s;++j){
		  if (w[j].type!=_VECT || w[j]._VECTptr->size()!=3)
		    glvertex(vecteur(3,0),0,0,contextptr);
		  else 
		    glvertex( lastpnt=*w[j]._VECTptr,0,0,contextptr );
		}
		glEnd();
		xcas_color(couleur,true);
		// FIXME?? back face
		/*
		glnormal(f1,f2,f3,e1,e2,e3,d1,d2,d3);
		glBegin(GL_POLYGON);
		for (int j=s-1;j>=3;--j){
		  if (w[j].type!=_VECT || w[j]._VECTptr->size()!=3)
		    glvertex(vecteur(3,0),0,0,contextptr);
		  else 
		    glvertex( *w[j]._VECTptr ,0,0,contextptr);
		}
		glVertex3d(f1,f2,f3);
		glVertex3d(e1,e2,e3);
		glVertex3d(d1,d2,d3);
		glEnd();
		*/
	      }
	    }
	  }
	}
	for (it=vv0.begin();it!=itend;++it){
	  if (it->type==_VECT){ 
	    w = *evalf_double(*it,1,contextptr)._VECTptr;
	    int s=w.size();
	    if (s<3)
	      continue;
	    double d1,d2,d3,e1,e2,e3,f1,f2,f3;	    
	    if (get_glvertex(w[0],d1,d2,d3,0,0,contextptr) &&
		get_glvertex(w[1],e1,e2,e3,0,0,contextptr) &&
		get_glvertex(w[2],f1,f2,f3,0,0,contextptr) ){
	      glnormal(d1,d2,d3,e1,e2,e3,f1,f2,f3);
	      if (!hidden_line){
		glColor3d(0,0,0);
		glBegin(GL_LINE_LOOP);
		glVertex3d(d1,d2,d3);
		glVertex3d(e1,e2,e3);
		glVertex3d(f1,f2,f3);
		for (int j=3;j<s;++j){
		  if (w[j].type!=_VECT || w[j]._VECTptr->size()!=3)
		    glvertex(vecteur(3,0),0,0,contextptr);
		  else 
		    glvertex( lastpnt=*w[j]._VECTptr,0,0,contextptr );
		}
		glEnd();
		xcas_color(couleur,true);
	      }
	    }
	  } // end it->type==_VECT
	} // end for (;it!=itend;)
	if (!hidden_name && show_names) legende_draw(lastpnt,legende,labelpos);
	return;
      } // end polyedre
      int s=vv0.size();
      if (s==2 && check3dpoint(vv0.front()) && check3dpoint(vv0.back()) ){
	// segment, half-line, vector or line
	vecteur A(*evalf_double(vv0.front(),1,contextptr)._VECTptr),B(*evalf_double(vv0.back(),1,contextptr)._VECTptr);
	vecteur dir(subvecteur(B,A));
	double lambda=1,nu;
	double d1=evalf_double(dir[0],1,contextptr)._DOUBLE_val;
	double d2=evalf_double(dir[1],1,contextptr)._DOUBLE_val;
	double d3=evalf_double(dir[2],1,contextptr)._DOUBLE_val;
	double nd=std::sqrt(d1*d1+d2*d2+d3*d3);
	if (std::abs(d1)>=std::abs(d2) && std::abs(d1)>=std::abs(d3)){
	  nu=(window_xmin-evalf_double(A[0],1,contextptr)._DOUBLE_val)/d1;
	}
	if (std::abs(d2)>std::abs(d1) && std::abs(d2)>=std::abs(d3)){
	  nu=(window_ymin-evalf_double(A[1],1,contextptr)._DOUBLE_val)/d2;
	}
	if (std::abs(d3)>std::abs(d1) && std::abs(d3)>std::abs(d2)){
	  nu=(window_zmin-evalf_double(A[2],1,contextptr)._DOUBLE_val)/d3;
	}
	if (std::abs(d1)>=0.1*nd)
	  lambda=giac_max(lambda,(window_xmax-window_xmin)/std::abs(d1));
	if (std::abs(d2)>=0.1*nd)
	  lambda=giac_max(lambda,(window_ymax-window_ymin)/std::abs(d2));
	if (std::abs(d3)>=0.1*nd)
	  lambda=giac_max(lambda,(window_ymax-window_ymin)/std::abs(d3));
	lambda *= 2;
	glBegin(GL_LINE_STRIP);
	if (v0.subtype==_LINE__VECT){
	  // move A in clip
	  A=addvecteur(A,multvecteur(nu,dir));
	  B=subvecteur(B,multvecteur(nu,dir));
	  glvertex(subvecteur(A,multvecteur(lambda,dir)),0,0,contextptr);
	  glvertex(addvecteur(A,multvecteur(lambda,dir)),0,0,contextptr);
	}
	else {
	  glvertex(A,0,0,contextptr);
	  if (v0.subtype==_HALFLINE__VECT)
	    glvertex(addvecteur(A,multvecteur(lambda,dir)),0,0,contextptr);
	  else
	    glvertex(B,0,0,contextptr);
	}
	glEnd();
	if (v0.subtype==_VECTOR__VECT){
	  double xB=evalf_double(B[0],1,contextptr)._DOUBLE_val;
	  double yB=evalf_double(B[1],1,contextptr)._DOUBLE_val;
	  double zB=evalf_double(B[2],1,contextptr)._DOUBLE_val;
	  double xA=evalf_double(A[0],1,contextptr)._DOUBLE_val;
	  double yA=evalf_double(A[1],1,contextptr)._DOUBLE_val;
	  double zA=evalf_double(A[2],1,contextptr)._DOUBLE_val;
	  /* 2-d code */
	  double iA,jA,depthA,iB,jB,depthB,di,dj,dij;
	  find_ij(xB,yB,zB,iB,jB,depthB);
	  find_ij(xA,yA,zA,iA,jA,depthA);
	  di=iA-iB; dj=jA-jB;
	  dij=std::sqrt(di*di+dj*dj);
	  if (dij){
	    dij /= min(5,int(dij/10))+width;
	    di/=dij;
	    dj/=dij;
	    double dip=-dj,djp=di;
	    di*=std::sqrt(3.0);
	    dj*=std::sqrt(3.0);
	    double iC=iB+di+dip,jC=jB+dj+djp;
	    double iD=iB+di-dip,jD=jB+dj-djp;
	    double xC,yC,zC,xD,yD,zD;
	    find_xyz(iC,jC,depthB,xC,yC,zC);
	    find_xyz(iD,jD,depthB,xD,yD,zD);
	    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	    glBegin(GL_POLYGON);
	    glVertex3d(xB,yB,zB);
	    glVertex3d(xC,yC,zC);
	    glVertex3d(xD,yD,zD);
	    glEnd();
	  }
	}
	if (!hidden_name && show_names && v0.subtype!=_GROUP__VECT) 
	  legende_draw(multvecteur(0.5,addvecteur(A,B)),legende,labelpos);
	return;
      }
      if (s>2){ // polygon
	bool closed=vv0.front()==vv0.back();
	double d1,d2,d3,e1,e2,e3,f1,f2,f3;	    
	if (get_glvertex(vv0[0],d1,d2,d3,0,0,contextptr) &&
	    get_glvertex(vv0[1],e1,e2,e3,0,0,contextptr) &&
	    get_glvertex(vv0[2],f1,f2,f3,0,0,contextptr) ){
	  if (test_enable_texture(texture) && s<=5 && closed){
	    glBegin(GL_QUADS);
	    glnormal(d1,d2,d3,e1,e2,e3,f1,f2,f3);
	    glTexCoord2f(0,0);
	    glVertex3d(d1,d2,d3);
	    glTexCoord2f(0,1);
	    glVertex3d(e1,e2,e3);
	    glTexCoord2f(1,1);
	    glVertex3d(f1,f2,f3);
	    if (s==5){
	      glTexCoord2f(1,0);
	      glvertex(*vv0[3]._VECTptr,0,0,contextptr);
	    }
	    else
	      glVertex3d(d1,d2,d3);
	    glEnd();
	    glDisable(GL_TEXTURE_2D);
	  }
	  else {
	    glBegin(closed?GL_POLYGON:GL_LINE_STRIP);
	    glnormal(d1,d2,d3,e1,e2,e3,f1,f2,f3);
	    const_iterateur it=vv0.begin(),itend=vv0.end();
	    if (closed)
	      ++it;
	    for (;it!=itend;++it){
	      if (check3dpoint(*it))
		glvertex(*it->_VECTptr,0,0,contextptr);
	    }
	    glEnd();
	  }
	  if (!hidden_name && show_names && !vv0.empty()) 
	    legende_draw(vv0.front(),legende,labelpos);
	}
	return;
      }
    }
  }

  void Graph3d::indraw(const vecteur & v){
    const_iterateur it=v.begin(),itend=v.end();
    for (;it!=itend;++it)
      indraw(*it);
  }

  void mult4(double * colmat,double * vect,double * res){
    res[0]=colmat[0]*vect[0]+colmat[4]*vect[1]+colmat[8]*vect[2]+colmat[12]*vect[3];
    res[1]=colmat[1]*vect[0]+colmat[5]*vect[1]+colmat[9]*vect[2]+colmat[13]*vect[3];
    res[2]=colmat[2]*vect[0]+colmat[6]*vect[1]+colmat[10]*vect[2]+colmat[14]*vect[3];
    res[3]=colmat[3]*vect[0]+colmat[7]*vect[1]+colmat[11]*vect[2]+colmat[15]*vect[3];
  }

  void mult4(double * colmat,float * vect,double * res){
    res[0]=colmat[0]*vect[0]+colmat[4]*vect[1]+colmat[8]*vect[2]+colmat[12]*vect[3];
    res[1]=colmat[1]*vect[0]+colmat[5]*vect[1]+colmat[9]*vect[2]+colmat[13]*vect[3];
    res[2]=colmat[2]*vect[0]+colmat[6]*vect[1]+colmat[10]*vect[2]+colmat[14]*vect[3];
    res[3]=colmat[3]*vect[0]+colmat[7]*vect[1]+colmat[11]*vect[2]+colmat[15]*vect[3];
  }

  void mult4(double * c,double k,double * res){
    for (int i=0;i<16;i++)
      res[i]=k*c[i];
  }
  
  double det4(double * c){
    return c[0]*c[5]*c[10]*c[15]-c[0]*c[5]*c[14]*c[11]-c[0]*c[9]*c[6]*c[15]+c[0]*c[9]*c[14]*c[7]+c[0]*c[13]*c[6]*c[11]-c[0]*c[13]*c[10]*c[7]-c[4]*c[1]*c[10]*c[15]+c[4]*c[1]*c[14]*c[11]+c[4]*c[9]*c[2]*c[15]-c[4]*c[9]*c[14]*c[3]-c[4]*c[13]*c[2]*c[11]+c[4]*c[13]*c[10]*c[3]+c[8]*c[1]*c[6]*c[15]-c[8]*c[1]*c[14]*c[7]-c[8]*c[5]*c[2]*c[15]+c[8]*c[5]*c[14]*c[3]+c[8]*c[13]*c[2]*c[7]-c[8]*c[13]*c[6]*c[3]-c[12]*c[1]*c[6]*c[11]+c[12]*c[1]*c[10]*c[7]+c[12]*c[5]*c[2]*c[11]-c[12]*c[5]*c[10]*c[3]-c[12]*c[9]*c[2]*c[7]+c[12]*c[9]*c[6]*c[3];
  }

  void inv4(double * c,double * res){
    res[0]=c[5]*c[10]*c[15]-c[5]*c[14]*c[11]-c[10]*c[7]*c[13]-c[15]*c[9]*c[6]+c[14]*c[9]*c[7]+c[11]*c[6]*c[13];
    res[1]=-c[1]*c[10]*c[15]+c[1]*c[14]*c[11]+c[10]*c[3]*c[13]+c[15]*c[9]*c[2]-c[14]*c[9]*c[3]-c[11]*c[2]*c[13];
    res[2]=c[1]*c[6]*c[15]-c[1]*c[14]*c[7]-c[6]*c[3]*c[13]-c[15]*c[5]*c[2]+c[14]*c[5]*c[3]+c[7]*c[2]*c[13];
    res[3]=-c[1]*c[6]*c[11]+c[1]*c[10]*c[7]+c[6]*c[3]*c[9]+c[11]*c[5]*c[2]-c[10]*c[5]*c[3]-c[7]*c[2]*c[9];
    res[4]=-c[4]*c[10]*c[15]+c[4]*c[14]*c[11]+c[10]*c[7]*c[12]+c[15]*c[8]*c[6]-c[14]*c[8]*c[7]-c[11]*c[6]*c[12];
    res[5]=c[0]*c[10]*c[15]-c[0]*c[14]*c[11]-c[10]*c[3]*c[12]-c[15]*c[8]*c[2]+c[14]*c[8]*c[3]+c[11]*c[2]*c[12];
    res[6]=-c[0]*c[6]*c[15]+c[0]*c[14]*c[7]+c[6]*c[3]*c[12]+c[15]*c[4]*c[2]-c[14]*c[4]*c[3]-c[7]*c[2]*c[12];
    res[7]=c[0]*c[6]*c[11]-c[0]*c[10]*c[7]-c[6]*c[3]*c[8]-c[11]*c[4]*c[2]+c[10]*c[4]*c[3]+c[7]*c[2]*c[8];
    res[8]=c[4]*c[9]*c[15]-c[4]*c[13]*c[11]-c[9]*c[7]*c[12]-c[15]*c[8]*c[5]+c[13]*c[8]*c[7]+c[11]*c[5]*c[12];
    res[9]=-c[0]*c[9]*c[15]+c[0]*c[13]*c[11]+c[9]*c[3]*c[12]+c[15]*c[8]*c[1]-c[13]*c[8]*c[3]-c[11]*c[1]*c[12];
    res[10]=c[0]*c[5]*c[15]-c[0]*c[13]*c[7]-c[5]*c[3]*c[12]-c[15]*c[4]*c[1]+c[13]*c[4]*c[3]+c[7]*c[1]*c[12];
    res[11]=-c[0]*c[5]*c[11]+c[0]*c[9]*c[7]+c[5]*c[3]*c[8]+c[11]*c[4]*c[1]-c[9]*c[4]*c[3]-c[7]*c[1]*c[8];
    res[12]=-c[4]*c[9]*c[14]+c[4]*c[13]*c[10]+c[9]*c[6]*c[12]+c[14]*c[8]*c[5]-c[13]*c[8]*c[6]-c[10]*c[5]*c[12];
    res[13]=c[0]*c[9]*c[14]-c[0]*c[13]*c[10]-c[9]*c[2]*c[12]-c[14]*c[8]*c[1]+c[13]*c[8]*c[2]+c[10]*c[1]*c[12];
    res[14]=-c[0]*c[5]*c[14]+c[0]*c[13]*c[6]+c[5]*c[2]*c[12]+c[14]*c[4]*c[1]-c[13]*c[4]*c[2]-c[6]*c[1]*c[12];
    res[15]=c[0]*c[5]*c[10]-c[0]*c[9]*c[6]-c[5]*c[2]*c[8]-c[10]*c[4]*c[1]+c[9]*c[4]*c[2]+c[6]*c[1]*c[8];
    double det=det4(c);
    mult4(res,1/det,res);
  }

  void dim32dim2(double * view,double * proj,double * model,double x0,double y0,double z0,double & i,double & j,double & dept){
    double vect[4]={x0,y0,z0,1},res1[4],res2[4];
    mult4(model,vect,res1);
    mult4(proj,res1,res2);
    i=res2[0]/res2[3]; // x and y are in [-1..1]
    j=res2[1]/res2[3];
    dept=res2[2]/res2[3];
    i=view[0]+(i+1)*view[2]/2;
    j=view[1]+(j+1)*view[3]/2;
    // x and y are the distance to the BOTTOM LEFT of the window
  }

  void dim22dim3(double * view,double * proj_inv,double * model_inv,double i,double j,double depth_,double & x,double & y,double & z){
    i=(i-view[0])*2/view[2]-1;
    j=(j-view[1])*2/view[3]-1;
    double res2[4]={i,j,depth_,1},res1[4],vect[4];
    mult4(proj_inv,res2,res1);
    mult4(model_inv,res1,vect);
    x=vect[0]/vect[3];
    y=vect[1]/vect[3];
    z=vect[2]/vect[3];
  }

  void Graph3d::find_ij(double x,double y,double z,double & i,double & j,double & depth_) {
    dim32dim2(view,proj,model,x,y,z,i,j,depth_);
#ifdef __APPLE__
    j=this->y()+h()-j;
    i=i+this->x();
#else
    j=window()->h()-j;
#endif
    // cout << i << " " << j <<  endl;
  }

  void Graph3d::find_xyz(double i,double j,double depth_,double & x,double & y,double & z) {
#ifdef __APPLE__
    j=this->y()+h()-j;
    i=i-this->x();
#else
    j=window()->h()-j;
#endif
    dim22dim3(view,proj_inv,model_inv,i,j,depth_,x,y,z);
  }

  double sqrt3over2=std::sqrt(double(3.0))/2;

  bool find_xmin_dx(double x0,double x1,double & xmin,double & dx){
    if (x0>=x1)
      return false;
    double x0x1=x1-x0;
    dx=std::pow(10,std::floor(std::log10(x0x1)));
    if (x0x1/dx>6)
      dx *= 2;    
    if (x0x1/dx<1.3)
      dx /= 5;
    if (x0x1/dx<3)
      dx /=2;
    if (!dx)
      return false;
    xmin=std::ceil(x0/dx)*dx;
    return true;
  }

  void Graph3d::draw_string(const string & s){
    if (printing){
      gl2psText(s.c_str(),"Helvetica",labelsize());
    }
    else {
#if !defined(__APPLE__) || !defined(INT128) // (does not work with textures on mac 64 bits)
      gl_font(FL_HELVETICA,labelsize());
      gl_draw(s.c_str());
#endif
    }
  }

  void normalize(double & a,double &b,double &c){
    double n=std::sqrt(a*a+b*b+c*c);
    a /= n;
    b /= n;
    c /= n;
  }

  void Graph3d::normal2plan(double & a,double &b,double &c){
    context * contextptr=hp?hp->contextptr:get_context(this);
    a /= std::pow(window_xmax-window_xmin,2);
    b /= std::pow(window_ymax-window_ymin,2);
    c /= std::pow(window_zmax-window_zmin,2);
    normalize(a,b,c);
    if (std::abs(a)<=1e-3){
      a=0;
      if (std::abs(b)<=1e-3){
	b=0;
	c=1;
      }
      else {
	gen gcb(float2rational(c/b,1e-3,contextptr));
	gen cbn,cbd;
	fxnd(gcb,cbn,cbd);
	if (cbn.type==_INT_ && cbd.type==_INT_ &&  std::abs(double(cbn.val)/cbd.val-c/a)<1e-2){
	  b=cbn.val;
	  c=cbd.val;
	}
      }
    }
    else {
      gen gba(float2rational(b/a,1e-3,contextptr));
      gen ban,bad,can,cad;
      fxnd(gba,ban,bad);
      if (ban.type==_INT_ && bad.type==_INT_  && std::abs(double(ban.val)/bad.val-b/a)<1e-2){
	gen gca(float2rational(c/a,1e-3,contextptr));
	fxnd(gca,can,cad);
	if (can.type==_INT_ && cad.type==_INT_ &&  std::abs(double(can.val)/cad.val-c/a)<1e-2){
	  int g=gcd(cad.val,bad.val);
	  // b/a=ban/bad=ban*(cad/g)/ppcm
	  int ai,bi,ci;
	  ai=(cad.val*bad.val/g);
	  bi=(ban.val*cad.val/g);
	  ci=(can.val*bad.val/g);
	  if (std::abs(ai)<=13 && std::abs(bi)<=13 && std::abs(ci)<13){
	    a=ai; b=bi; c=ci;
	  }
	}
      }
    }
  }

  void Graph3d::current_normal(double & a,double & b,double & c) {
    double res1[4]={0,0,1,1},vect[4];
    mult4(model_inv,res1,vect);
    a=vect[0]/vect[3]-(window_xmax+window_xmin)/2;
    b=vect[1]/vect[3]-(window_ymax+window_ymin)/2;
    c=vect[2]/vect[3]-(window_zmax+window_zmin)/2;
    if (std::abs(a)<1e-3*(window_xmax-window_xmin))
      a=0;
    if (std::abs(b)<1e-3*(window_ymax-window_ymin))
      b=0;
    if (std::abs(c)<1e-3*(window_zmax-window_zmin))
      c=0;
  }

  void round0(double & x,double xmin,double xmax){
    if (std::abs(x)<1e-3*(xmax-xmin))
      x=0;
  }

  void Graph3d::display(){
    glEnable(GL_NORMALIZE);
    glEnable(GL_LINE_STIPPLE);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0,1.0);
    glEnable(GL_CLIP_PLANE0);
    glEnable(GL_CLIP_PLANE1);
    glEnable(GL_CLIP_PLANE2);
    glEnable(GL_CLIP_PLANE3);
    glEnable(GL_CLIP_PLANE4);
    glEnable(GL_CLIP_PLANE5);
    // cout << glIsEnabled(GL_CLIP_PLANE0) << endl;
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glShadeModel((display_mode & 0x10)?GL_FLAT:GL_SMOOTH);
    bool lighting=display_mode & 0x8;
    if (lighting)
      glClearColor(0, 0, 0, 0);
    else
      glClearColor(1, 1, 1, 1);
    // clear the color and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);      
    // view transformations
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();   
    glLoadIdentity();
    bool notperspective=display_mode & 0x4;
    if (notperspective)
      // glFrustum(-sqrt3over2,sqrt3over2,-sqrt3over2,sqrt3over2,sqrt3over2,3*sqrt3over2);
      glOrtho(-sqrt3over2,sqrt3over2,-sqrt3over2,sqrt3over2,-sqrt3over2,sqrt3over2);
    else
      glFrustum(-0.5,0.5,-0.5,0.5,sqrt3over2,3*sqrt3over2);
    // put the visualisation cube inside above visualisation
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();   
    glLoadIdentity();
    double dx=(window_xmax-window_xmin),dy=(window_ymax-window_ymin),dz=(window_zmax-window_zmin);
    if (dx==0) { dx=1; ++window_xmax; }
    if (dy==0) { dy=1; ++window_ymax; }
    if (dz==0) { dz=1; ++window_zmax; }
    double x,y,z,theta;
    get_axis_angle_deg( (dragi || dragj)?(quaternion_double(dragi*180/h(),0,0)*rotation_2_quaternion_double(1,0,0,dragj*180/w())*q):q,x,y,z,theta);
    // cerr << theta << " " << x << "," << y << "," << z << endl;
    if (!notperspective)
      glTranslated(0,0,-2*sqrt3over2);
    glRotated(theta,x,y,z);
    glScaled(1/dx,1/dy,1/dz);
    glTranslated(-(window_xmin+window_xmax)/2,-(window_ymin+window_ymax)/2,-(window_zmin+window_zmax)/2);
    // glRotated(theta_y,0,0,1);
    double plan0[]={1,0,0,0.5};
    double plan1[]={-1,0,0,0.5};
    double plan2[]={0,1,0,0.5};
    double plan3[]={0,-1,0,0.5};
    double plan4[]={0,0,1,0.5};
    double plan5[]={0,0,-1,0.5};
    plan0[3]=-window_xmin+dx/256;
    plan1[3]=window_xmax+dx/256;
    plan2[3]=-window_ymin+dy/256;
    plan3[3]=window_ymax+dy/256;
    plan4[3]=-window_zmin+dz/256;
    plan5[3]=window_zmax+dz/256;
    glGetDoublev(GL_PROJECTION_MATRIX,proj); // projection matrix in columns
    /*
    for (int i=0;i<15;++i)
      proj[i]=0;
    proj[0]=1/sqrt3over2;
    proj[5]=proj[0];
    proj[10]=-proj[0];
    proj[15]=1;
    */
    inv4(proj,proj_inv);
    glGetDoublev(GL_MODELVIEW_MATRIX,model); // modelview matrix in columns
    inv4(model,model_inv);
    glGetDoublev(GL_VIEWPORT,view);
    if (debug_infolevel>=2){
      double check[16];
      mult4(model,&model_inv[0],&check[0]);
      mult4(model,&model_inv[4],&check[4]);
      mult4(model,&model_inv[8],&check[8]);
      mult4(model,&model_inv[12],&check[12]);
      for (int i=0;i<16;++i){
	cout << model[i] << ",";
	if (i%4==3) cout << endl;
      }
      cout << endl;
      for (int i=0;i<16;++i){
	cout << model_inv[i] << ",";
	if (i%4==3) cout << endl;
      }
      cout << endl;
      for (int i=0;i<16;++i){
	cout << check[i] << ",";
	if (i%4==3) cout << endl;
      }
      cout << endl;
    }
    // drax
    bool fbox=(display_mode & 0x100);
    bool triedre=(display_mode & 0x200);
    glLineStipple(1,0xffff);
    glLineWidth(1);
    gl2psLineWidth(1);
    gl_color(FL_RED);
    // glColor3f(1,0,0);
    if (show_axes || triedre){
      glLineWidth(3);
      gl2psLineWidth(3);
      glBegin(GL_LINES);
      glVertex3d(0,0,0);
      glVertex3d(1,0,0);
      glEnd();
    }
    if (show_axes){
      glLineWidth(1);
      gl2psLineWidth(1);
      glBegin(GL_LINES);
      glVertex3d(0,0,0);
      glVertex3d(window_xmax,0,0);
      glEnd();
    }
    if (show_axes && !printing){
      glBegin(GL_LINES);
      glVertex3d(window_xmin,window_ymin,window_zmin);
      glVertex3d(window_xmax,window_ymin,window_zmin);
      glVertex3d(window_xmin,window_ymax,window_zmin);
      glVertex3d(window_xmax,window_ymax,window_zmin);
      glVertex3d(window_xmin,window_ymin,window_zmax);
      glVertex3d(window_xmax,window_ymin,window_zmax);
      glVertex3d(window_xmin,window_ymax,window_zmax);
      glVertex3d(window_xmax,window_ymax,window_zmax);
      glEnd();
    }
    gl_color(FL_GREEN);
    // glColor3f(0,1,0);
    if (show_axes || triedre){
      glLineWidth(3);
      gl2psLineWidth(3);
      glBegin(GL_LINES);
      glVertex3d(0,0,0);
      glVertex3d(0,1,0);
      glEnd();
    }
    if (show_axes){
      glLineWidth(1);
      gl2psLineWidth(1);
      glBegin(GL_LINES);
      glVertex3d(0,0,0);
      glVertex3d(0,window_ymax,0);
      glEnd();
    }
    if (show_axes && !printing){
      glBegin(GL_LINES);
      glVertex3d(window_xmin,window_ymin,window_zmin);
      glVertex3d(window_xmin,window_ymax,window_zmin);
      glVertex3d(window_xmax,window_ymin,window_zmin);
      glVertex3d(window_xmax,window_ymax,window_zmin);
      glVertex3d(window_xmin,window_ymin,window_zmax);
      glVertex3d(window_xmin,window_ymax,window_zmax);
      glVertex3d(window_xmax,window_ymin,window_zmax);
      glVertex3d(window_xmax,window_ymax,window_zmax);
      glEnd();
    }
    gl_color(FL_BLUE);
    // glColor3f(0,0,1);
    if (show_axes || triedre){
      glLineWidth(3);
      gl2psLineWidth(3);
      glBegin(GL_LINES);
      glVertex3d(0,0,0);
      glVertex3d(0,0,1);
      glEnd();
    }
    if (show_axes){
      glLineWidth(1);
      gl2psLineWidth(1);
      glBegin(GL_LINES);
      glVertex3d(0,0,0);
      glVertex3d(0,0,window_zmax);
      glEnd();
    }
    if (show_axes && !printing){
      glBegin(GL_LINES);
      glVertex3d(window_xmin,window_ymin,window_zmin);
      glVertex3d(window_xmin,window_ymin,window_zmax);
      glVertex3d(window_xmax,window_ymax,window_zmin);
      glVertex3d(window_xmax,window_ymax,window_zmax);
      glVertex3d(window_xmin,window_ymax,window_zmin);
      glVertex3d(window_xmin,window_ymax,window_zmax);
      glVertex3d(window_xmax,window_ymin,window_zmin);
      glVertex3d(window_xmax,window_ymin,window_zmax);
      glEnd();
    }
    if(show_axes){ // maillage
      glColor3f(1,0,0);
      glRasterPos3d(1,0,0);
      draw_string(x_axis_name.empty()?"x":x_axis_name);
      glColor3f(0,1,0);
      glRasterPos3d(0,1,0);
      draw_string(y_axis_name.empty()?"y":y_axis_name);
      glColor3f(0,0,1);
      glRasterPos3d(0,0,1);
      draw_string(z_axis_name.empty()?"z":z_axis_name);
      if (fbox){
	double xmin,dx,x,ymin,dy,y,zmin,dz,z;
	find_xmin_dx(window_xmin,window_xmax,xmin,dx);
	find_xmin_dx(window_ymin,window_ymax,ymin,dy);
	find_xmin_dx(window_zmin,window_zmax,zmin,dz);
	glLineStipple(1,0x3333);
	glBegin(GL_LINES);
	gl_color(FL_CYAN);
	for (z=zmin;z<=window_zmax;z+=2*dz){
	  for (y=ymin;y<=window_ymax;y+=2*dy){
	    glVertex3d(window_xmin,y,z);
	    glVertex3d(window_xmax,y,z);
	  }
	}
	for (z=zmin;z<=window_zmax;z+=2*dz){
	  for (x=xmin;x<=window_xmax;x+=2*dx){
	    glVertex3d(x,window_ymin,z);
	    glVertex3d(x,window_ymax,z);
	  }
	}
	for (x=xmin;x<=window_xmax;x+=2*dx){
	  for (y=ymin;y<=window_ymax;y+=2*dy){
	    glVertex3d(x,y,window_zmin);
	    glVertex3d(x,y,window_zmax);
	  }
	}
	glEnd();
	gl_color((display_mode & 0x8)?FL_WHITE:FL_BLACK);
	glPointSize(3);
	for (x=xmin;x<=window_xmax;x+=dx){
	  round0(x,window_xmin,window_xmax);
	  glBegin(GL_POINTS);
	  glVertex3d(x,window_ymin,window_zmin);
	  glEnd();
	  glRasterPos3d(x,window_ymin,window_zmin);
	  string tmps=giac::print_DOUBLE_(x,2)+x_axis_unit;
	  draw_string(tmps);
	}
	for (y=ymin;y<=window_ymax;y+=dy){
	  round0(y,window_ymin,window_ymax);
	  glBegin(GL_POINTS);
	  glVertex3d(window_xmin,y,window_zmin);
	  glEnd();
	  glRasterPos3d(window_xmin,y,window_zmin);
	  string tmps=giac::print_DOUBLE_(y,2)+y_axis_unit;
	  draw_string(tmps);
	}
	for (z=zmin;z<=window_zmax;z+=dz){
	  round0(z,window_zmin,window_zmax);
	  glBegin(GL_POINTS);
	  glVertex3d(window_xmin,window_ymin,z);
	  glEnd();
	  glRasterPos3d(window_xmin,window_ymin,z);
	  string tmps=giac::print_DOUBLE_(z,2)+z_axis_unit;
	  draw_string(tmps);
	}
      }
    }
    glClipPlane(GL_CLIP_PLANE0,plan0);
    glClipPlane(GL_CLIP_PLANE1,plan1);
    glClipPlane(GL_CLIP_PLANE2,plan2);
    glClipPlane(GL_CLIP_PLANE3,plan3);
    glClipPlane(GL_CLIP_PLANE4,plan4);
    glClipPlane(GL_CLIP_PLANE5,plan5);
    // mouse plan
    double normal_a,normal_b,normal_c;
    current_normal(normal_a,normal_b,normal_c);
    normal2plan(normal_a,normal_b,normal_c);
    double plan_x0,plan_y0,plan_z0,plan_t0;
    find_xyz(0,0,depth,plan_x0,plan_y0,plan_z0);
    plan_t0=normal_a*plan_x0+normal_b*plan_y0+normal_c*plan_z0;
    if (std::abs(plan_t0)<std::abs(window_zmax-window_zmin)/1000)
      plan_t0=0;
    if (show_axes && !printing && parent() && dynamic_cast<Figure *>(parent())){
      gl_color((display_mode & 0x8)?FL_WHITE:FL_BLACK);
      glLineWidth(2);
      gl2psLineWidth(2);
      double xa,ya,za,xb,yb,zb;
      // show mouse plan intersections with the faces of the clip planes
      // example with the face z=Z:
      // a*x+b*y=plan_t0-normal_c*Z
      // get the 4 intersections, keep only the valid ones
      glLineStipple(1,0x3333);
      if (is_clipped(normal_a,window_xmin,window_xmax,normal_b,window_ymin,window_ymax,plan_t0-normal_c*window_zmin,xa,ya,xb,yb)){
	glBegin(GL_LINE_STRIP);
	glVertex3d(xa,ya,window_zmin+dz/256);
	glVertex3d(xb,yb,window_zmin+dz/256);
	glEnd();
      }
      if (is_clipped(normal_a,window_xmin,window_xmax,normal_b,window_ymin,window_ymax,plan_t0-normal_c*window_zmax,xa,ya,xb,yb)){
	glBegin(GL_LINE_STRIP);
	glVertex3d(xa,ya,window_zmax-dz/256);
	glVertex3d(xb,yb,window_zmax-dz/256);
	glEnd();
      }
      if (is_clipped(normal_a,window_xmin,window_xmax,normal_c,window_zmin,window_zmax,plan_t0-normal_b*window_ymin,xa,za,xb,zb)){
	glBegin(GL_LINE_STRIP);
	glVertex3d(xa,window_ymin+dy/256,za);
	glVertex3d(xb,window_ymin+dy/256,zb);
	glEnd();
      }
      if (is_clipped(normal_a,window_xmin,window_xmax,normal_c,window_zmin,window_zmax,plan_t0-normal_b*window_ymax,xa,za,xb,zb)){
	glBegin(GL_LINE_STRIP);
	glVertex3d(xa,window_ymax-dy/256,za);
	glVertex3d(xb,window_ymax-dy/256,zb);
	glEnd();
      }
      if (is_clipped(normal_b,window_ymin,window_ymax,normal_c,window_zmin,window_zmax,plan_t0-normal_a*window_xmin,ya,za,yb,zb)){
	glBegin(GL_LINE_STRIP);
	glVertex3d(window_xmin+dx/256,ya,za);
	glVertex3d(window_xmin+dx/256,yb,zb);
	glEnd();
      }
      if (is_clipped(normal_b,window_ymin,window_ymax,normal_c,window_zmin,window_zmax,plan_t0-normal_a*window_xmax,ya,za,yb,zb)){
	glBegin(GL_LINE_STRIP);
	glVertex3d(window_xmax-dx/256,ya,za);
	glVertex3d(window_xmax-dx/256,yb,zb);
	glEnd();
      }
      // same for window_zmax, etc.
      glLineWidth(1);
      gl2psLineWidth(1);
      glLineStipple(1,0xffff);
    }
    if (lighting){
      /*
      GLfloat mat_specular[] = { 1.0,1.0,1.0,1.0 };
      GLfloat mat_shininess[] = { 50.0 };
      glMaterialfv(GL_FRONT,GL_SPECULAR,mat_specular);
      glMaterialfv(GL_FRONT,GL_SHININESS,mat_shininess);
      */
      glEnable(GL_LIGHTING);
      glEnable(GL_CULL_FACE);
      glEnable(GL_COLOR_MATERIAL);
      static GLfloat l_pos[8][4],l_dir[8][3],ambient[8][4],diffuse[8][4],specular[8][4];
      for (int i=0;i<8;++i){
	if (!light_on[i]){
	  glDisable(GL_LIGHT0+i);
	  continue;
	}
	glEnable(GL_LIGHT0+i);
	l_pos[i][0]=light_x[i];
	l_pos[i][1]=light_y[i];
	l_pos[i][2]=light_z[i];
	l_pos[i][3]=light_w[i];
	glLightfv(GL_LIGHT0+i,GL_POSITION,l_pos[i]);
	l_dir[i][0]=light_spot_x[i];
	l_dir[i][1]=light_spot_y[i];
	l_dir[i][2]=light_spot_z[i];
	glLightfv(GL_LIGHT0+i,GL_SPOT_DIRECTION,l_dir[i]);
	glLightf(GL_LIGHT0+i,GL_SPOT_EXPONENT,light_spot_exponent[i]);
	glLightf(GL_LIGHT0+i,GL_SPOT_CUTOFF,light_spot_cutoff[i]);
	glLightf(GL_LIGHT0+i,GL_CONSTANT_ATTENUATION,light_0[i]);
	glLightf(GL_LIGHT0+i,GL_LINEAR_ATTENUATION,light_1[i]);
	glLightf(GL_LIGHT0+i,GL_QUADRATIC_ATTENUATION,light_2[i]);
	ambient[i][0]=light_ambient_r[i];
	ambient[i][1]=light_ambient_g[i];
	ambient[i][2]=light_ambient_b[i];
	ambient[i][3]=light_ambient_a[i];
	glLightfv(GL_LIGHT0+i,GL_AMBIENT,ambient[i]);
	diffuse[i][0]=light_diffuse_r[i];
	diffuse[i][1]=light_diffuse_g[i];
	diffuse[i][2]=light_diffuse_b[i];
	diffuse[i][3]=light_diffuse_a[i];
	glLightfv(GL_LIGHT0+i,GL_DIFFUSE,diffuse[i]);
	specular[i][0]=light_specular_r[i];
	specular[i][1]=light_specular_g[i];
	specular[i][2]=light_specular_b[i];
	specular[i][3]=light_specular_a[i];
	glLightfv(GL_LIGHT0+i,GL_SPECULAR,specular[i]);
	if (debug_infolevel>=2){
	  GLfloat posf[4],direcf[4],ambient[4],diffuse[4],specular[4];
	  GLfloat expo,cutoff;
	  double pos[4],direc[4];
	  glGetLightfv(GL_LIGHT0+i,GL_SPOT_EXPONENT,&expo);
	  glGetLightfv(GL_LIGHT0+i,GL_SPOT_CUTOFF,&cutoff);
	  glGetLightfv(GL_LIGHT0+i,GL_POSITION,posf);
	  glGetLightfv(GL_LIGHT0+i,GL_SPOT_DIRECTION,direcf);
	  direcf[4]=0;
	  mult4(model_inv,posf,pos);
	  tran4(model);
	  mult4(model,direcf,direc);
	  tran4(model);
	  glGetLightfv(GL_LIGHT0+i,GL_AMBIENT,ambient);
	  glGetLightfv(GL_LIGHT0+i,GL_DIFFUSE,diffuse);
	  glGetLightfv(GL_LIGHT0+i,GL_SPECULAR,specular);
	  cerr << "light " << i << ": " <<
	    " pos " << pos[0] << "," << pos[1] << "," << pos[2] << "," << pos[3] << 
	    " dir " << direc[0] << "," << direc[1] << "," << direc[2] << "," << direc[3] << 
	    " ambient " << ambient[0] << "," << ambient[1] << "," << ambient[2] << "," << ambient[3] << 
	    " diffuse " << diffuse[0] << "," << diffuse[1] << "," << diffuse[2] << "," << diffuse[3] << 
	    " specular " << specular[0] << "," << specular[1] << "," << specular[2] << "," << specular[3] << 
	    " exponent " << expo << " cutoff " << cutoff << endl;
	}
      }
    }
    if (display_mode & 0x20){
      glDisable(GL_DEPTH_TEST);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    }
    // now draw each object
    if ( (display_mode & 2) && !animation_instructions.empty())
      indraw(animation_instructions[animation_instructions_pos % animation_instructions.size()]);
    if ( display_mode & 0x40 )
      indraw(trace_instructions);
    if (display_mode & 1)
      indraw(plot_instructions);
    if (display_mode & 0x8){
      glDisable(GL_LIGHTING);
      for (int i=0;i<8;++i)
	glDisable(GL_LIGHT0+i);
    }
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    gen plot_tmp,title_tmp;
    History_Pack * hp =get_history_pack(this);
    context * contextptr=hp?hp->contextptr:get_context(this);
    find_title_plot(title_tmp,plot_tmp,contextptr);
    indraw(plot_tmp);
    if (mode==1 && pushed && push_in_area && in_area){
      // draw segment between push and current
      gl_color(FL_RED);
      double x,y,z;
      glBegin(GL_LINES);
      find_xyz(push_i,push_j,push_depth,x,y,z);
      glVertex3d(x,y,z);
      find_xyz(current_i,current_j,current_depth,x,y,z);
      glVertex3d(x,y,z);
      glEnd();
    }
    glDisable(GL_CLIP_PLANE0);
    glDisable(GL_CLIP_PLANE1);
    glDisable(GL_CLIP_PLANE2);
    glDisable(GL_CLIP_PLANE3);
    glDisable(GL_CLIP_PLANE4);
    glDisable(GL_CLIP_PLANE5);
    /*
    if( show_axes){
      gl_color((display_mode & 0x8)?FL_WHITE:FL_BLACK);
      glRasterPos3d(window_xmin,window_ymin,window_zmin);
      string tmps=giac::print_DOUBLE_(window_xmin,2)+","+giac::print_DOUBLE_(window_ymin,2)+","+giac::print_DOUBLE_(window_zmin,2);
      draw_string(tmps);
      glRasterPos3d(window_xmax,window_ymax,window_zmax);
      tmps=giac::print_DOUBLE_(window_xmax,2)+","+giac::print_DOUBLE_(window_ymax,2)+","+giac::print_DOUBLE_(window_zmax,2);
      draw_string(tmps);
    }
    */
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gl_color((display_mode & 0x8)?FL_WHITE:FL_BLACK);
    double td=fl_width(title.c_str());
    glRasterPos3d(-0.4*td/w(),-1,depth-0.001);
    string mytitle(title);
    if (!is_zero(title_tmp) && function_final.type==_FUNC)
      mytitle=gen(symbolic(*function_final._FUNCptr,title_tmp)).print(contextptr);
    if (!mytitle.empty())
      draw_string(mytitle);
    glRasterPos3d(-1,-1,depth-0.001);
    if (!args_help.empty() && args_tmp.size()<= args_help.size()){
      draw_string(gettext("Click ")+args_help[giacmax(1,args_tmp.size())-1]);
    }
    glRasterPos3d(-0.98,0.87,depth-0.001);
    if (show_axes && !printing){
      string tmps=gettext("mouse plan ")+giac::print_DOUBLE_(normal_a,3)+"x+"+giac::print_DOUBLE_(normal_b,3)+"y+"+giac::print_DOUBLE_(normal_c,3)+"z="+ giac::print_DOUBLE_(plan_t0,3);
      // cerr << tmps << endl;
      draw_string(tmps); // +" Z="+giac::print_DOUBLE_(-depth,3));
    }
    if (below_depth_hidden){
      /* current mouse position
	double i=Fl::event_x();
	double j=Fl::event_y();
	j=window()->h()-j;
	i=(i-view[0])*2/view[2]-1;
	j=(j-view[1])*2/view[3]-1;
      */
      glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
      xcas_color((display_mode & 0x8)?FL_WHITE:FL_BLACK,true);
      glBegin(GL_POLYGON);
      glVertex3d(-1,-1,depth);
      glVertex3d(-1,1,depth);
      glVertex3d(1,1,depth);
      glVertex3d(1,-1,depth);
      glEnd();
    }
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glFlush();
    glFinish();
    if (screenbuf && !printing){
      glReadBuffer(GL_FRONT);
      glReadPixels(this->x(), window()->h()-this->y()-this->h(), w(), h(), GL_RGBA, GL_UNSIGNED_BYTE, screenbuf);
    }
  }

  void Graph3d::print(){
    if (!printing)
      return;
    // EPS output
    glViewport(0,0, w(), h()); // lower left
    int buf=1024*1024;
    for (;;buf+=1024*1024){
      gl2psBeginPage(label(),"xcasnew",0,GL2PS_EPS,GL2PS_BSP_SORT,GL2PS_SIMPLE_LINE_OFFSET | GL2PS_DRAW_BACKGROUND | GL2PS_USE_CURRENT_VIEWPORT,GL_RGBA,0,0,0,0,0,buf,printing,"output");
      gl2psEnable(GL2PS_LINE_STIPPLE);
      display();
      if (gl2psEndPage()!=GL2PS_OVERFLOW){
	break;
      }
    }    
  }

  bool find_clip_box(Fl_Widget * wid,int & x,int & y,int & w,int & h){
    if (!wid || !wid->window())
      return false;
    fl_push_no_clip();
    vector<Fl_Widget *> p;
    for (;wid;wid=wid->parent())
      p.push_back(wid);
    for (int i=p.size()-2;i>=0;--i){
      fl_clip_box(p[i]->x(),p[i]->y(),p[i]->w(),p[i]->h(),x,y,w,h);
      fl_push_clip(x,y,w,h);
    }
    for (int i=p.size()-1;i>=0;--i)
      fl_pop_clip();
    return true;
  }

  // if printing is true, we call gl2ps to make an eps file
  void Graph3d::draw(){
    // cerr << "graph3d" << endl;
    int clip_x,clip_y,clip_w,clip_h;
#ifdef __APPLE__
    if (!find_clip_box(this,clip_x,clip_y,clip_w,clip_h))
#endif
      fl_clip_box(x(),y(),w(),h(),clip_x,clip_y,clip_w,clip_h);
    // cerr << clip_x << " " << clip_y<< " " << clip_w<< " " << clip_h << endl;
    if (printing){
      fprintf(printing,"%s","\nGS\n\nCR\nCS\n% avant eps -> BeginDocument\n/b4_Inc_state save def\n%save state for cleanup\n/dict_count countdictstack def\n/op_count count 1 sub def\n%count objects on op stack\nuserdict begin\n%make userdict current dict\n/showpage { } def\n%redefine showpage to be null\n0 setgray 0 setlinecap\n1 setlinewidth 0 setlinejoin\n10 setmiterlimit [] 0 setdash newpath\n/languagelevel where\n%if not equal to 1 then\n{pop languagelevel                %set strokeadjust and\n1 ne\n%overprint to their defaults\n{false setstrokeadjust false setoverprint\n} if\n} if\n");
      // Translate by previous widget h()
      fprintf(printing,"[1 0 0 -1 0 0] concat\n%d %d translate\n",x(),-h()-y());
      fprintf(printing,"%s","\n%%BeginDocument: out.eps\n");
      print();
      fprintf(printing,"%s","\n%%EndDocument\ncount op_count sub {pop} repeat\ncountdictstack dict_count sub {end} repeat  %clean up dict stack\nb4_Inc_state restore\n% apres eps\n\nGR\n");
      return;
    }
    Fl_Window * win = window();
    if (!win)
      return;
    if (!hp)
      hp=geo_find_history_pack(this);
    context * contextptr = hp?hp->contextptr:0;
    int locked=0;
#ifdef HAVE_LIBPTHREAD
    locked=pthread_mutex_trylock(&interactive_mutex);
#endif
    bool b,block;
    if (!locked){
      b=io_graph(contextptr);
      io_graph(contextptr)=false;
      block=block_signal;
      block_signal=true;
    }
#ifdef __APPLE__
    GLContext context;
    if (!glcontext){ // create context
      GLContext shared_ctx = 0;
      context = aglCreateContext( gl_choice->pixelformat, shared_ctx);
      if (!context){ 
	if (!locked){
	  block_signal=block;
	  io_graph(contextptr)=b;
#ifdef HAVE_LIBPTHREAD
	  pthread_mutex_unlock(&interactive_mutex);
#endif
	}
	return ;
      }
      glcontext = (void *) context;
    }
    else
      context = (GLContext) glcontext;
    aglSetCurrentContext(context);
    GLint rect[] = { clip_x, win->h()-clip_y-clip_h, clip_w, clip_h};
    aglSetInteger( (GLContext)context, AGL_BUFFER_RECT, rect );
    aglEnable( (GLContext)context, AGL_BUFFER_RECT );
    OpaqueWindowPtr * winid=(OpaqueWindowPtr*) win->window_ref();
    aglSetWindowRef( context, winid );
    glEnable(GL_DEPTH_TEST);
    // glLoadIdentity();
    // glEnable(GL_SCISSOR_TEST);
    // glScissor(clip_x, win->h()-clip_y-clip_h, clip_w, clip_h); // lower left
    // glViewport(clip_x, win->h()-clip_y-clip_h, clip_w, clip_h); // lower left
    glViewport(0,0,w(),h());
    // glDrawBuffer(GL_FRONT);
    gl_font(FL_HELVETICA,labelsize());
    // GLint viewport[4];
    // glGetIntergerv(GL_VIEWPORT,viewport);
    //fl_push_clip(clip_x,clip_y,clip_w,clip_h);
    display();
#else
    gl_start();
    glEnable(GL_SCISSOR_TEST);
    glScissor(clip_x, win->h()-clip_y-clip_h, clip_w, clip_h); // lower left
    // glViewport(clip_x, win->h()-clip_y-clip_h, clip_w, clip_h); // lower left
    glViewport(x(),win->h()-y()-h(), w(), h()); // lower left
    gl_font(FL_HELVETICA,labelsize());
    // GLint viewport[4];
    // glGetIntergerv(GL_VIEWPORT,viewport);
    //fl_push_clip(clip_x,clip_y,clip_w,clip_h);
    display();
    gl_finish();
#endif
    struct timezone tz;
    gettimeofday(&animation_last,&tz);
    if (!paused)
      ++animation_instructions_pos;
    //fl_pop_clip();
    if (!locked){
      block_signal=block;
      io_graph(contextptr)=b;
#ifdef HAVE_LIBPTHREAD
      pthread_mutex_unlock(&interactive_mutex);
#endif
    }
  }

  const char * Graph3d::latex(const char * filename_){
    const char * filename=0;
    if ( (filename=Graph2d3d::latex(filename_)) ){
      ofstream of(filename);
      if (of){
	string epsfile(remove_extension(filename)+".eps");
	FILE * f=fopen(epsfile.c_str(),"w");
	if (f){
	  of << "% Generated by xcas\n" << endl;
	  of << giac::tex_preamble << endl;
	  of << "\\includegraphics[bb=0 0 400 "<< h() <<"]{" << remove_path(epsfile) << "}" << endl << endl ;
	  of << giac::tex_end << endl;
	  of.close();
	  int gw=w();
	  resize(x(),y(),400,h());
	  printing=f;
	  print();
	  printing=0;
	  resize(x(),y(),gw,h());
	  fclose(f);	
	}
      }
      
      /*
      FILE * f=fopen(filename,"w");
      if (f){
	int gw=w();
	resize(x(),y(),400,h());
	fprintf(f,"%s","% Generated by xcas\n");
	fprintf(f,"%s",giac::tex_preamble.c_str()) ;
	fprintf(f,"\n\n{\n\\setlength{\\unitlength}{1pt}\n\\begin{picture}(%d,%d)(%d,%d)\n{\\special{\"\n",w(),h(),0,0);
	printing=f;
	print();
	printing=0;
	resize(x(),y(),gw,h());
	fprintf(f,"%s","}\n}\\end{picture}\n}\n\n");
	fprintf(f,"%s",giac::tex_end.c_str());
	fclose(f);	
      }
      */
    }
    return filename;
  }

  Graph3d::~Graph3d(){ 
#ifdef __APPLE__
    if (glcontext){
      aglSetCurrentContext( NULL );
      aglSetWindowRef((GLContext) glcontext, NULL );    
      aglDestroyContext((GLContext)glcontext);
    }
#endif
  }

  int Graph3d::in_handle(int event){
    int res=Graph2d3d::in_handle(event);
    if (event==FL_FOCUS){
      if (!paused)
	--animation_instructions_pos;
      redraw();
    }
    if (event==FL_FOCUS || event==FL_UNFOCUS)
      return 1;
    if (event==FL_KEYBOARD){
      theta_z -= int(theta_z/360)*360;
      theta_x -= int(theta_x/360)*360;
      theta_y -= int(theta_y/360)*360;
      switch (Fl::event_text()?Fl::event_text()[0]:0){
      case 'y':
	theta_z += delta_theta;
	q=q*euler_deg_to_quaternion_double(delta_theta,0,0);
	redraw();
	return 1;
      case 'z':
	theta_x += delta_theta;
	q=q*euler_deg_to_quaternion_double(0,delta_theta,0);
	redraw();
	return 1;
      case 'x':
	theta_y += delta_theta;
	q=q*euler_deg_to_quaternion_double(0,0,delta_theta);
	redraw();
	return 1;
      case 'Y':
	theta_z -= delta_theta;
	q=q*euler_deg_to_quaternion_double(-delta_theta,0,0);
	redraw();
	return 1;
      case 'Z':
	theta_x -= delta_theta;
	q=q*euler_deg_to_quaternion_double(0,-delta_theta,0);
	redraw();
	return 1;
      case 'X':
	theta_y -= delta_theta;
	q=q*euler_deg_to_quaternion_double(0,0,-delta_theta);
	redraw();
	return 1;
      case 'u': case 'f':
	depth += 0.01;
	mouse_position->redraw();
	redraw();
	return 1;
      case 'd': case 'n':
	depth -= 0.01;
	mouse_position->redraw();
	redraw();
	return 1;
      case 'h': case 'H': // hide below depth
	below_depth_hidden=true;
	redraw();
	return 1;
      case 's': case 'S': // show below depth
	below_depth_hidden=false;
	redraw();
	return 1;
      }
    }
    current_i=Fl::event_x();
    current_j=Fl::event_y();
    current_depth=depth;
    double x,y,z;
    find_xyz(current_i,current_j,current_depth,x,y,z);
    in_area=(x>=window_xmin) && (x<=window_xmax) &&
      (y>=window_ymin) && (y<=window_ymax) && 
      (z>=window_zmin) && (z<=window_zmax);
    if (event==FL_MOUSEWHEEL){
      if (!Fl::event_inside(this))
	return 0;
      if (show_axes){ // checking in_area seems to difficult, especially if mouse plan is not viewed
	depth = int(1000*(depth-Fl::e_dy *0.01)+.5)/1000.0;
	mouse_position->redraw();
	redraw();
	return 1;
      }
      else {
	if (Fl::e_dy<0)
	  zoom(0.8);
	else
	  zoom(1.25);
	return 1;
      }
    }
    if ( (event==FL_PUSH || push_in_area))
      ;
    else
      in_area=false;
    if (event==FL_PUSH){
      if (this!=Fl::focus()){
	Fl::focus(this);
	handle(FL_FOCUS);
      }
      push_i=current_i;
      push_j=current_j;
      push_depth=current_depth;
      push_in_area = in_area;
      pushed = true;
      return 1;
    }
    if (!(display_mode & 0x80) && push_in_area && (event==FL_DRAG || event==FL_RELEASE)){
      double x1,y1,z1,x2,y2,z2;
      find_xyz(current_i,current_j,current_depth,x1,y1,z1);
      find_xyz(push_i,push_j,push_depth,x2,y2,z2);
      double newx=x1-x2, newy=y1-y2, newz=z1-z2;
      round3(newx,window_xmin,window_xmax);
      round3(newy,window_ymin,window_ymax);      
      round3(newz,window_zmin,window_zmax);
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
	pushed=false;
      return 1;
    }
    if (event==FL_DRAG){
      if (push_in_area)
	return 0;
      dragi=current_i-push_i;
      dragj=current_j-push_j;
      redraw();
      return 1;
    }
    if (event==FL_RELEASE){
      pushed = false;
      if (push_in_area)
	return 0;
      dragi=current_i-push_i;
      dragj=current_j-push_j;
      if (paused && absint(dragi)<4 && absint(dragj)<4)
	++animation_instructions_pos;
      else
	q=quaternion_double(dragi*180/h(),0,0)*rotation_2_quaternion_double(1,0,0,dragj*180/w())*q;
      dragi=dragj=0;
      redraw();
      return 1;
    }
    return res;
  }

  Geo3d::Geo3d(int x,int y,int width, int height, History_Pack * _hp): Graph3d(x,y,width,height,0,_hp) { 
    hp=_hp?_hp:geo_find_history_pack(this);
    if (hp){
      hp->eval_below=true;
      hp->eval_below_once=false;
      show_mouse_on_object=true;
    }
  }

  int Geo3d::in_handle(int event){
    if (!event)
      return 1;
    if (!hp)
      hp=geo_find_history_pack(this);
    // cerr << event << " " << mode << endl;
    int res=Graph3d::in_handle(event);
    if (event==FL_UNFOCUS){
      return 1;
    }
    if (event==FL_FOCUS){
      return 1;
    }
    if (int gres=geo_handle(event))
      return gres;
    // event not handeld by geometry
    return res;
  }

  void Geo3d::draw(){
    Graph3d::draw();
  }

  // set evryone to x
  void Graph3d::orthonormalize(){ 
    window_ymax=window_xmax;
    window_ymin=window_xmin;
    window_zmax=window_xmax;
    window_zmin=window_xmin;
    redraw();
  }

  void Graph3d::geometry_round(double x,double y,double z,double eps,gen & tmp,GIAC_CONTEXT) {
    tmp= geometry_round_numeric(x,y,z,eps,approx);
    if (tmp.type==_VECT)
      tmp.subtype = _POINT__VECT;
    selected=nearest_point(plot_instructions,tmp,eps,contextptr);
    // if there is a point inside selected, stop there, 
    // otherwise find a point that is near the line
    int pos=findfirstpoint(selection2vecteur(selected));
    if (pos<0){
      // line passing through tmp      
      double a,b,c;
      double i,j,k;
      find_ij(x,y,z,i,j,k);
      k--;
      find_xyz(i,j,k,a,b,c);
      gen line(makevecteur(tmp,makevecteur(a,b,c)),_LINE__VECT);
      /*
	current_normal(a,b,c);
	vecteur v(makevecteur(a,b,c));
	gen line(makevecteur(tmp,tmp+v),_LINE__VECT);
      */
      vector<int> sel2=nearest_point(plot_instructions,line,eps,contextptr);
      pos=findfirstpoint(selection2vecteur(sel2));
      if (pos>=0){
	selected.insert(selected.begin(),sel2[pos]);
      }
      else {
	vector<int>::const_iterator it=sel2.begin(),itend=sel2.end();
	for (;it!=itend;++it)
	  selected.push_back(*it);
      }
      if (selected.empty()){
	// add hyperplans
	const_iterateur it=plot_instructions.begin(),itend=plot_instructions.end();
	for (int pos=0;it!=itend;++it,++pos){
	  gen tmp=remove_at_pnt(*it);
	  if (tmp.is_symb_of_sommet(at_hyperplan)){
	    vecteur v=interdroitehyperplan(line,tmp,contextptr);
	    if (!v.empty() && !is_undef(v.front())){
	      gen inters=evalf_double(remove_at_pnt(v.front()),1,contextptr);
	      if (inters.type==_VECT && inters._VECTptr->size()==3){
		vecteur & xyz=*inters._VECTptr;
		if (xyz[0].type==_DOUBLE_ && xyz[1].type==_DOUBLE_ && xyz[2].type==_DOUBLE_){
		  double x=xyz[0]._DOUBLE_val;
		  double y=xyz[1]._DOUBLE_val;
		  double z=xyz[2]._DOUBLE_val;
		  if (x>=window_xmin && x<=window_xmax &&
		      y>=window_ymin && y<=window_ymax &&
		      z>=window_zmin && z<=window_zmax)
		    selected.push_back(pos);
		}
	      }
	    }
	  }
	}
      }
    }
  }

#ifndef NO_NAMESPACE_XCAS
} // namespace giac
#endif // ndef NO_NAMESPACE_XCAS

#endif // HAVE_LIBFLTK_GL
