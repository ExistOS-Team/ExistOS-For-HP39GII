// -*- mode:C++ ; compile-command: "g++ -I.. -fPIC -DPIC -g -c renee.cc -o renee.lo && ln -sf renee.lo renee.o && gcc -shared renee.lo -lc  -Wl,-soname -Wl,librenee.so.0 -o librenee.so.0.0.0 && ln -sf librenee.so.0.0.0 librenee.so.0 && ln -sf librenee.so.0.0.0 librenee.so" -*-
/*
 *  Copyright (C) 2002 Renee De Graeve, Inst. Fourier, 38402 St Martin d'Heres
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
using namespace std;
#include <stdexcept>
#include <cmath>
#include <cstdlib>
#ifndef IN_GIAC
#include <giac/giac.h>
#else
#include "giac.h"
#endif
//#include "renee.h"
//#include "papier.h"

#ifndef NO_NAMESPACE_GIAC
namespace giac {
#endif // ndef NO_NAMESPACE_GIAC

#include <stdexcept>
#include <cmath>
#include <cstdlib>
#ifndef IN_GIAC
#include <giac/giac.h>
#else
#include "giac.h"
#endif

  void read_option(const vecteur & v,double xmin,double xmax,double ymin,double ymax,vecteur & attributs, int & nstep,int & jstep,GIAC_CONTEXT);

  void papier_lignes(vecteur & res,double xmin,double xmax,double ymin,double ymax,double angle,double deltax,double deltay,double pente,const vecteur & attributs,GIAC_CONTEXT){
    res.push_back(pnt_attrib(gen(makevecteur(xmin+ymin*cst_i,xmin+ymax*cst_i),_GROUP__VECT),attributs,contextptr)); 
    res.push_back(pnt_attrib(gen(makevecteur(xmax+ymax*cst_i,xmin+ymax*cst_i),_GROUP__VECT),attributs,contextptr));
    res.push_back(pnt_attrib(gen(makevecteur(xmax+ymin*cst_i,xmax+ymax*cst_i),_GROUP__VECT),attributs,contextptr));
    res.push_back(pnt_attrib(gen(makevecteur(xmax+ymin*cst_i,xmin+ymin*cst_i),_GROUP__VECT),attributs,contextptr));
    //const double cst_pi;
    double pi=evalf_double(cst_pi,1,contextptr)._DOUBLE_val;
    if (angle==pi/2){
      for (double x=xmin;x<=xmax;x+=deltax){
	res.push_back(pnt_attrib(gen(makevecteur(x+ymin*cst_i,x+ymax*cst_i),_GROUP__VECT),attributs,contextptr));
      }
      //return res;
    }
    double Y=(ymax-ymin)/pente;
    if (angle<pi/2){
      double q=std::floor(Y/deltax+1e-12);
      for (double x=xmin-q*deltax;x<=xmin;x+=deltax){
	double Y1=pente*(xmax-x)+ymin;
	double Y2=pente*(xmin-x)+ymin;
	double X1=(ymax-ymin)/pente+x;
	if (Y1<ymax){
	  res.push_back(pnt_attrib(gen(makevecteur(xmin+Y2*cst_i,xmax+Y1*cst_i),_GROUP__VECT),attributs,contextptr));
	}
	if (X1<xmax){
	  res.push_back(pnt_attrib(gen(makevecteur(xmin+Y2*cst_i,X1+ymax*cst_i),_GROUP__VECT),attributs,contextptr));
	}
      }
      for (double x=xmin;x<=xmax;x+=deltax){
	double Y1=pente*(xmax-x)+ymin;
	double Y2=pente*(xmin-x)+ymin;
	double X1=(ymax-ymin)/pente+x;
	if (Y1<ymax){
	  res.push_back(pnt_attrib(gen(makevecteur(x+ymin*cst_i,xmax+Y1*cst_i),_GROUP__VECT),attributs,contextptr));
	}
	if (X1<xmax){
	  res.push_back(pnt_attrib(gen(makevecteur(x+ymin*cst_i,X1+ymax*cst_i),_GROUP__VECT),attributs,contextptr));
	}
      }
      //return res;
    }
    if (angle>pi/2){
      double x=xmin;
      while (x<=xmax){
	double Y1=pente*(xmin-x)+ymin;
	double X1=(ymax-ymin)/pente+x;
	if (Y1<ymax){
	  res.push_back(pnt_attrib(gen(makevecteur(x+ymin*cst_i,xmin+Y1*cst_i),_GROUP__VECT),attributs,contextptr));
	}
	if (X1>xmin){
	  res.push_back(pnt_attrib(gen(makevecteur(x+ymin*cst_i,X1+ymax*cst_i),_GROUP__VECT),attributs,contextptr));
	}
	x=x+deltax;
      }
      double q=std::ceil(Y/deltax-1e-12);
      while (x<=xmax-q*deltax){
	double Y1=pente*(xmin-x)+ymin;
	double Y2=pente*(xmax-x)+ymin;
	double X1=(ymax-ymin)/pente+x;
	if (Y1<ymax){
	  res.push_back(pnt_attrib(gen(makevecteur(xmax+Y2*cst_i,xmin+Y1*cst_i),_GROUP__VECT),attributs,contextptr));
	}
	if (X1>xmin){
	  res.push_back(pnt_attrib(gen(makevecteur(xmax+Y2*cst_i,X1+ymax*cst_i),_GROUP__VECT),attributs,contextptr));
	}
	x=x+deltax;
      }
      //return res;
    }
  }
  
  gen Papier_pointe_quadrillage(const gen & args,int quadrillage,GIAC_CONTEXT){
    double xmin=gnuplot_xmin,xmax=gnuplot_xmax,ymin=gnuplot_ymin,ymax=gnuplot_ymax;
    double deltax=(xmax-xmin)/20,deltay=(ymax-ymin)/20,angle=evalf_double(cst_pi/2,1,contextptr)._DOUBLE_val;
    vecteur attributs(1,default_color(contextptr));
    if (args.type==_VECT){
      vecteur & w=*args._VECTptr;
      int s=w.size();
      if (s>0){
	gen tmp=evalf_double(w[0],1,contextptr);
	if (tmp.type==_DOUBLE_)
	  deltax=fabs(tmp._DOUBLE_val);
      }
      if (s>1){
	gen tmp=evalf_double(w[1],1,contextptr);
	if (tmp.type==_DOUBLE_){
	  angle=tmp._DOUBLE_val;
	  double pi=M_PI;
	  angle=angle-std::floor(angle/pi)*pi;
	  std::cout<<angle << endl; 
	  if (fabs(angle)<epsilon(contextptr) || fabs(pi-angle)<epsilon(contextptr))
	    setsizeerr();
	}
      }
      if (s>2){
	gen tmp=evalf_double(w[2],1,contextptr);
	if (tmp.type==_DOUBLE_)
	  deltay=fabs(tmp._DOUBLE_val);
      }
      //int nstep=int((xmax-xmin)/deltax),kstep=int((ymax-ymin)/deltay);
      gen x,y;
      for (int i=0;i<s;++i){
	if (w[i].is_symb_of_sommet(at_equal)){
	  if (w[i][1]==x__IDNT_e)
	    readrange(w[i],gnuplot_xmin,gnuplot_xmax,x,xmin,xmax,contextptr);
	  if (w[i][1]==y__IDNT_e)
	    readrange(w[i],gnuplot_xmin,gnuplot_xmax,y,ymin,ymax,contextptr);
	}
      }
      int n1,n2;
      read_option(w,xmin,xmax,ymin,ymax,attributs,n1,n2,contextptr);
      // if (!nstep)nstep=20;deltax=(xmax-xmin)/nstep;
      //if (!kstep) kstep=20;deltay=(ymax-ymin)/kstep;
    }

    deltax=(xmax-xmin)/std::floor(fabs((xmax-xmin)/deltax));
    deltay=(ymax-ymin)/std::floor(fabs((ymax-ymin)/deltay));
    if (quadrillage==2){
      int color=attributs[0].val;
      color = (color & 0xffff )| (7<<25) | (1 << 19);
      attributs[0]=color;
    }
    
    vecteur res;

    double pente=std::tan(angle);
    
    if (quadrillage==0 || quadrillage==1){
      for (double y=ymin;y<=ymax;y+=deltay){
	res.push_back(pnt_attrib(gen(makevecteur(xmin+y*cst_i,xmax+y*cst_i),_GROUP__VECT),attributs,contextptr));
      }
      //papier_lignes(res,xmin,xmax,ymin,ymax,0,deltax,deltay,pente,attributs,contextptr);
      papier_lignes(res,xmin,xmax,ymin,ymax,angle,deltax,deltay,pente,attributs,contextptr);
    }
    if (quadrillage==1){
      double u1=deltay/pente;
      if (u1-deltax==0) {angle=M_PI/2;}
      if (u1-deltax>0) {angle=std::atan(deltay/(u1-deltax)); }
      if (u1-deltax<0) {angle=std::atan(deltay/(u1-deltax))+M_PI;}
      papier_lignes(res,xmin,xmax,ymin,ymax,angle,deltax,deltay,std::tan(angle),attributs,contextptr);
      
    } // end if quadrillage== 1    
    if (quadrillage==2) {
      res.push_back(pnt_attrib(gen(makevecteur(xmin+ymin*cst_i,xmin+ymax*cst_i),_GROUP__VECT),attributs,contextptr)); 
      res.push_back(pnt_attrib(gen(makevecteur(xmax+ymax*cst_i,xmin+ymax*cst_i),_GROUP__VECT),attributs,contextptr));
      res.push_back(pnt_attrib(gen(makevecteur(xmax+ymin*cst_i,xmax+ymax*cst_i),_GROUP__VECT),attributs,contextptr));
      res.push_back(pnt_attrib(gen(makevecteur(xmax+ymin*cst_i,xmin+ymin*cst_i),_GROUP__VECT),attributs,contextptr));
      for (double y=ymin;y<=ymax;y+=deltay){
	double X=(y-ymin)/pente;
	int q=std::floor(X/deltax+1e-12);
	for (double x=xmin-q*deltax+X;x<xmax;x+=deltax){
	  res.push_back(pnt_attrib(x+y*cst_i,attributs,contextptr)); 
	}
      }
    }
    if (quadrillage==3) {
      papier_lignes(res,xmin,xmax,ymin,ymax,angle,deltax,deltay,pente,attributs,contextptr);
    }
    return res; // gen(res,_SEQ__VECT);
  }
  gen _Papier_pointe(const gen & args,GIAC_CONTEXT){
    return Papier_pointe_quadrillage(args,2,contextptr);
  }
  // const char _Papier_pointe_s []="papierp";
  const string _Papier_pointe_s("papierp");
  unary_function_eval __Papier_pointe(&giac::_Papier_pointe,_Papier_pointe_s);
  unary_function_ptr at_Papier_pointe (&__Papier_pointe,0,true);
	
  gen _Papier_quadrille(const gen & args,GIAC_CONTEXT){
    return Papier_pointe_quadrillage(args,0,contextptr);
  }
  // const char _Papier_quadrille_s []="papierq";
  const string _Papier_quadrille_s ("papierq");
  unary_function_eval __Papier_quadrille(&giac::_Papier_quadrille,_Papier_quadrille_s);
  unary_function_ptr at_Papier_quadrille (&__Papier_quadrille,0,true);
	
  gen _Papier_triangule(const gen & args,GIAC_CONTEXT){
    return Papier_pointe_quadrillage(args,1,contextptr);
  }
  // const char _Papier_triangule_s []="papiert";
  const string _Papier_triangule_s ("papiert");
  unary_function_eval __Papier_triangule(&giac::_Papier_triangule,_Papier_triangule_s);
  unary_function_ptr at_Papier_triangule (&__Papier_triangule,0,true);
	
  gen _Papier_ligne(const gen & args,GIAC_CONTEXT){
    return Papier_pointe_quadrillage(args,3,contextptr);
  }
  // const char _Papier_ligne_s []="papierl";
  const string _Papier_ligne_s("papierl");
  unary_function_eval __Papier_ligne(&giac::_Papier_ligne,_Papier_ligne_s);
  unary_function_ptr at_Papier_ligne (&__Papier_ligne,0,true);
	
#ifndef NO_NAMESPACE_GIAC
} // namespace giac
#endif // ndef NO_NAMESPACE_GIAC
