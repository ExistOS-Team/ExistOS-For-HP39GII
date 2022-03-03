// -*- mode:C++ ; compile-command: "g++-3.4 -I. -I.. -g -c Equation.cc -DHAVE_CONFIG_H -DIN_GIAC -Wall" -*-
/*
 *  Copyright (C) 2005,2014 B. Parisse, Institut Fourier, 38402 St Martin d'Heres
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
#ifndef IN_GIAC
#include <giac/first.h>
#else
#include "first.h"
#endif
#include <string>
#ifdef HAVE_LIBFLTK
#include "Equation.h"
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Window.H>
#include <FL/fl_ask.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Return_Button.H>
#ifndef IN_GIAC
#include <giac/giac.h>
#else
#include "giac.h"
#endif
#include "History.h"
#include "Xcas1.h"
#include "Input.h"
#include "Equation.h"
#include "Graph.h"
#include <iostream>
#include <fstream>
using namespace std;
using namespace giac;

#ifndef NO_NAMESPACE_XCAS
namespace xcas {
#endif // ndef NO_NAMESPACE_XCAS

  unsigned max_prettyprint_equation=5000;
  
#ifdef _HAVE_FL_UTF8_HDR_
  Fl_Font cst_greek_translate(string & s0){
    string s1(s0);
    s0="";
    int n=s1.size();
    for (int i=0;i<n;++i){
      if (!isalpha(s1[i])){
	s0 += s1[i];
	continue;
      }
      string s;
      for (;i<n;++i){
	if (!isalpha(s1[i])){
	  --i;
	  break;
	}
	s+=s1[i];
      }
      bool done=false;
      switch (s.size()){
      case 2:
	if (s=="mu"){
	  s0+="μ";
	  done=true;
	}
	if (s=="nu"){
	  s0+="ν";
	  done=true;
	}
	if (s=="pi"){
	  s0+="π";
	  done=true;
	}
	if (s=="xi"){
	  s0+="ξ";
	  done=true;
	}
	if (s=="Xi"){
	  s0+="Ξ";
	  done=true;
	}
	break;
      case 3:
	if (s=="chi"){
	  s0+="χ";
	  done=true;
	}
	if (s=="phi"){
	  s0+="φ";
	  done=true;
	}
	if (s=="Phi"){
	  s0+="Φ";
	  done=true;
	}
	if (s=="eta"){
	  s0+="η";
	  done=true;
	}
	if (s=="rho"){
	  s0+="ρ";
	  done=true;
	}
	if (s=="tau"){
	  s0+="τ";
	  done=true;
	}
	if (s=="psi"){
	  s0+="ψ";
	  done=true;
	}
	if (s=="Psi"){
	  s0+="Ψ";
	  done=true;
	}
	break;
      case 4:
	if (s=="beta"){
	  s0+="β";
	  done=true;
	}
	if (s=="Beta"){
	  s0+="β";
	  done=true;
	}
	if (s=="zeta"){
	  s0+="ζ";
	  done=true;
	}
	if (s=="Zeta"){
	  s0+="ζ";
	  done=true;
	}
	if (s=="alef"){
	  s0+="ℵ";
	  done=true;
	}
	break;
      case 5:
	if (s=="alpha"){
	  s0+="α";
	  done=true;
	}
	if (s=="delta"){
	  s0+="δ";
	  done=true;
	}
	if (s=="Delta"){
	  s0+="Δ";
	  done=true;
	}
	if (s=="gamma"){
	  s0+="γ";
	  done=true;
	}
	if (s=="Gamma"){
	  s0+="Γ";
	  done=true;
	}
	if (s=="kappa"){
	  s0+="κ";
	  done=true;
	}
	if (s=="theta"){
	  s0+="θ";
	  done=true;
	}
	if (s=="Theta"){
	  s0+="Θ";
	  done=true;
	}
	if (s=="sigma"){
	  s0+="σ";
	  done=true;
	}
	if (s=="Sigma"){
	  s0+="Σ";
	  done=true;
	}
	if (s=="Omega"){
	  s0+="Ω";
	  done=true;
	}
	if (s=="omega"){
	  s0+="ω";
	  done=true;
	}
	break;
      case 6:
	if (s=="lambda"){
	  s0+="λ";
	  done=true;
	}
	if (s=="Lambda"){
	  s0+="λ";
	  done=true;
	}
	break;
      case 7:
	if (s=="epsilon"){
	  s0+="ε";
	  done=true;
	}
	if (s=="product"){
	  s0="Π";
	  done=true;
	}
	break;
      case 8:
	if (s=="infinity"){
	  s0+="∞";
	  done=true;
	}
	break;
      case 11:
	if (s=="euler_gamma"){
	  s0+="ϒ";
	  done=true;
	}
	break;
      } // end switch
      if (!done)
	s0+=s;
    } // end for (int i=0;i<n;i++)
    return FL_HELVETICA;
  }
#else

  Fl_Font cst_greek_translate(string & s0){
    int n=s0.size(),j;
    for (j=n-1;j>=2;--j){
      if (isalpha(s0[j]))
	break;
    }
    string s=s0.substr(0,j+1),sadd;
    if (j<n-1)
      sadd=s0.substr(j+1,n-1-j);
    switch (s.size()){
    case 2:
    if (s=="mu"){
      s0="m"+sadd;
      return FL_SYMBOL;
    }
    if (s=="nu"){
      s0="n"+sadd;
      return FL_SYMBOL;
    }
    if (s=="pi"){
      s0="p"+sadd;
      return FL_SYMBOL;
    }
    if (s=="xi"){
      s0="x"+sadd;
      return FL_SYMBOL;
    }
    if (s=="Xi"){
      s0="X"+sadd;
      return FL_SYMBOL;
    }
    if (s=="im"){
      s0="Á"+sadd;
      return FL_SYMBOL;      
    }
    if (s=="re"){
      s0="Â"+sadd;
      return FL_SYMBOL;
    }
    break;
    case 3:
    if (s=="chi"){
      s0="c"+sadd;
      return FL_SYMBOL;
    }
    if (s=="phi"){
      s0="f"+sadd;
      return FL_SYMBOL;
    }
    if (s=="Phi"){
      s0="f"+sadd;
      return FL_SYMBOL;
    }
    if (s=="eta"){
      s0="h"+sadd;
      return FL_SYMBOL;
    }
    if (s=="rho"){
      s0="r"+sadd;
      return FL_SYMBOL;
    }
    if (s=="tau"){
      s0="t"+sadd;
      return FL_SYMBOL;
    }
    if (s=="psi"){
      s0="y"+sadd;
      return FL_SYMBOL;
    }
    if (s=="Psi"){
      s0="Y"+sadd;
      return FL_SYMBOL;
    }
    break;
    case 4:
    if (s=="beta"){
      s0="b"+sadd;
      return FL_SYMBOL;
    }
    if (s=="Beta"){
      s0="b"+sadd;
      return FL_SYMBOL;
    }
    if (s=="zeta"){
      s0="z"+sadd;
      return FL_SYMBOL;
    }
    if (s=="Zeta"){
      s0="z"+sadd;
      return FL_SYMBOL;
    }
    break;
    case 5:
    if (s=="alpha"){
      s0="a"+sadd;
      return FL_SYMBOL;
    }
    if (s=="delta"){
      s0="d"+sadd;
      return FL_SYMBOL;
    }
    if (s=="Delta"){
      s0="D"+sadd;
      return FL_SYMBOL;
    }
    if (s=="gamma"){
      s0="g"+sadd;
      return FL_SYMBOL;
    }
    if (s=="Gamma"){
      s0="G"+sadd;
      return FL_SYMBOL;
    }
    if (s=="kappa"){
      s0="k"+sadd;
      return FL_SYMBOL;
    }
    if (s=="theta"){
      s0="q"+sadd;
      return FL_SYMBOL;
    }
    if (s=="Theta"){
      s0="Q"+sadd;
      return FL_SYMBOL;
    }
    if (s=="sigma"){
      s0="s"+sadd;
      return FL_SYMBOL;
    }
    if (s=="Sigma"){
      s0="S"+sadd;
      return FL_SYMBOL;
    }
    if (s=="Omega"){
      s0="W"+sadd;
      return FL_SYMBOL;
    }
    if (s=="omega"){
      s0="w"+sadd;
      return FL_SYMBOL;
    }
    if (s=="aleph"){
      s0="À"+sadd;
      return FL_SYMBOL;
    }
    break;
    case 6:
    if (s=="lambda"){
      s0="l"+sadd;
      return FL_SYMBOL;
    }
    if (s=="Lambda"){
      s0="L"+sadd;
      return FL_SYMBOL;
    }
    if (s=="approx"){
      s0="»"+sadd;
      return FL_SYMBOL;      
    }
    break;
    case 7:
    if (s=="epsilon"){
      s0="e"+sadd;
      return FL_SYMBOL;
    }
    if (s=="product"){
      s0="P"+sadd;
      return FL_SYMBOL;
    }
    break;
    case 8:
    if (s=="infinity"){
      s0="¥"+sadd;
      return FL_SYMBOL;
    }
    break;
    case 11:
    if (s=="euler_gamma"){
      s0="g"+sadd;
      return FL_SYMBOL;
    }
    break;
    }
    return FL_HELVETICA;
  }
#endif

  int Equation_binary_search_pos(const eqwdata & e,int x,int y){
    int ss=e.g._STRNGptr->size();
    int debut=0,fin=ss,pos,l=0;
    fl_font(FL_HELVETICA,e.eqw_attributs.fontsize);
    for (;debut+1<fin;){
      pos=(debut+fin)/2;
      l=int(fl_width(e.g._STRNGptr->substr(0,pos).c_str()));
      if (e.x+l<x)
	debut=pos;
      else
	fin=pos;
    }
    int l2=int(fl_width(e.g._STRNGptr->substr(0,fin).c_str()));
    x=x-e.x;
    if (x-l>l2-x)
      return fin;
    else
      return debut;
  }

  bool Equation_multistring_selection(const_iterateur & it,const_iterateur & itend,bool search_active){
    for (;it!=itend;++it){ // find first selection
      if (it->type==_EQW){
	if (search_active){
	  if (it->_EQWptr->active)
	    break;
	}
	else {
	  if (it->_EQWptr->selected)
	    break;
	}
      }
    }
    if (it==itend)
      return false; // nothing selected
    const_iterateur it_end_sel=it+1;
    for (;it_end_sel!=itend;++it_end_sel){ // find last selection
      if (it_end_sel->type==_EQW){ 
	if (search_active){
	  if (!it_end_sel->_EQWptr->active)
	    break;
	}
	else {
	  if (!it_end_sel->_EQWptr->selected)
	    break;
	}
      }
    }
    itend=it_end_sel-1;
    return true;
  }

  eqwdata Equation_total_size(const gen & g){
    if (g.type==_EQW)
      return *g._EQWptr;
    if (g.type!=_VECT || g._VECTptr->empty())
      return eqwdata(0,0,0,0,attributs(0,0,0),undef);
    return Equation_total_size(g._VECTptr->back());
  }

  void Equation_translate(gen & g,int deltax,int deltay){
    if (g.type==_EQW){
      g._EQWptr->x += deltax;
      g._EQWptr->y += deltay;
      g._EQWptr->baseline += deltay;
      return ;
    }
    if (g.type!=_VECT)
      setsizeerr();
    vecteur & v=*g._VECTptr;
    iterateur it=v.begin(),itend=v.end();
    for (;it!=itend;++it)
      Equation_translate(*it,deltax,deltay);
  }

  gen Equation_change_attributs(const gen & g,const attributs & newa){
    if (g.type==_EQW){
      gen res(*g._EQWptr);
      res._EQWptr->eqw_attributs = newa;
      return res;
    }
    if (g.type!=_VECT)
      setsizeerr();
    vecteur v=*g._VECTptr;
    iterateur it=v.begin(),itend=v.end();
    for (;it!=itend;++it)
      *it=Equation_change_attributs(*it,newa);
    return gen(v,g.subtype);
  }

  vecteur Equation_subsizes(const gen & arg,const attributs & a,int windowhsize,GIAC_CONTEXT){
    vecteur v;
    if ( (arg.type==_VECT) && ( (arg.subtype==_SEQ__VECT) 
				// || (!ckmatrix(arg)) 
				) ){
      const_iterateur it=arg._VECTptr->begin(),itend=arg._VECTptr->end();
      for (;it!=itend;++it)
	v.push_back(Equation_compute_size(*it,a,windowhsize,contextptr));
    }
    else {
      v.push_back(Equation_compute_size(arg,a,windowhsize,contextptr));
    }
    return v;
  }

  // vertical merge with same baseline
  // for vertical merge of hp,yp at top (like ^) add fontsize to yp
  // at bottom (like lower bound of int) substract fontsize from yp
  void Equation_vertical_adjust(int hp,int yp,int & h,int & y){
    int yf=min(y,yp);
    h=max(y+h,yp+hp)-yf;
    y=yf;
  }

  // utility for gen embedded widget memory allocation
  void fltk_fl_widget_delete_function(void * ptr){
    Fl_Widget * w=(Fl_Widget *) ptr;
    /* not attached to any group
    if (w->parent()){
      Fl_Group * wg=dynamic_cast<Fl_Group *>(w->parent());
      if (wg)
	wg->remove(w);
    }
    */
    delete w;
  }

  gen Equation_compute_symb_size(const gen & g,const attributs & a,int windowhsize,GIAC_CONTEXT){
    if (g.type!=_SYMB)
      return Equation_compute_size(g,a,windowhsize,contextptr);
    unary_function_ptr & u=g._SYMBptr->sommet;
    gen arg=g._SYMBptr->feuille,rootof_value;
    if (u==at_rootof && arg.type==_VECT && arg._VECTptr->size()==2 && arg._VECTptr->front().type==_VECT && has_rootof_value(arg._VECTptr->back(),rootof_value,contextptr)){
      return Equation_compute_symb_size(horner_rootof(*arg._VECTptr->front()._VECTptr,rootof_value,contextptr),a,windowhsize,contextptr);
    }
    if (u==at_multistring){
      gen tmp=_multistring(arg,contextptr);
      tmp.subtype=1;
      return Equation_compute_size(tmp,a,windowhsize,contextptr);
    }
    if (u==at_makevector){
      vecteur v(1,arg);
      if (arg.type==_VECT)
	v=*arg._VECTptr;
      iterateur it=v.begin(),itend=v.end();
      for (;it!=itend;++it){
	if ( (it->type==_SYMB) && (it->_SYMBptr->sommet==at_makevector) )
	  *it=_makevector(it->_SYMBptr->feuille,contextptr);
      }
      return Equation_compute_size(v,a,windowhsize,contextptr);
    }
    if (u==at_makesuite){
      if (arg.type==_VECT)
	return Equation_compute_size(gen(*arg._VECTptr,_SEQ__VECT),a,windowhsize,contextptr);
      else
	return Equation_compute_size(arg,a,windowhsize,contextptr);
    }
    if (u==at_sqrt)
      return Equation_compute_size(symbolic(at_pow,gen(makevecteur(arg,plus_one_half),_SEQ__VECT)),a,windowhsize,contextptr);
    if (u==at_division){
      if (arg.type!=_VECT || arg._VECTptr->size()!=2)
	return Equation_compute_size(arg,a,windowhsize,contextptr);
      gen tmp;
#ifdef SMARTPTR64
      * ((longlong * ) &tmp) = longlong(new ref_fraction(Tfraction<gen>(arg._VECTptr->front(),arg._VECTptr->back()))) << 16;
#else
      tmp.__FRACptr = new ref_fraction(Tfraction<gen>(arg._VECTptr->front(),arg._VECTptr->back()));
#endif
      tmp.type=_FRAC;
      return Equation_compute_size(tmp,a,windowhsize,contextptr);
    }
    if (u==at_prod){
      gen n,d;
      if (rewrite_prod_inv(arg,n,d)){
	if (n.is_symb_of_sommet(at_neg))
	  return Equation_compute_size(symbolic(at_neg,Tfraction<gen>(-n,d)),a,windowhsize,contextptr);
	return Equation_compute_size(Tfraction<gen>(n,d),a,windowhsize,contextptr);
      }
    }
    if (u==at_inv){
      if ( (is_integer(arg) && is_positive(-arg,contextptr))
	   || (arg.is_symb_of_sommet(at_neg)))
	return Equation_compute_size(symbolic(at_neg,Tfraction<gen>(plus_one,-arg)),a,windowhsize,contextptr);
      return Equation_compute_size(Tfraction<gen>(plus_one,arg),a,windowhsize,contextptr);
    }
    if (u==at_expr && arg.type==_VECT && arg.subtype==_SEQ__VECT && arg._VECTptr->size()==2 && arg._VECTptr->back().type==_INT_){
      gen varg1=Equation_compute_size(arg._VECTptr->front(),a,windowhsize,contextptr);
      eqwdata vv(Equation_total_size(varg1));
      gen varg2=eqwdata(0,0,0,0,a,arg._VECTptr->back());
      vecteur v12(makevecteur(varg1,varg2));
      v12.push_back(eqwdata(vv.dx,vv.dy,0,vv.y,a,at_expr,0));
      return gen(v12,_SEQ__VECT);
    }
    int llp=int(fl_width("("));
    int lrp=int(fl_width(")"));
    int lc=int(fl_width(","));
    string us=u.ptr()->s;
    fl_font(cst_greek_translate(us),a.fontsize);
    int ls=int(fl_width(us.c_str()));
    fl_font(FL_HELVETICA,a.fontsize);
    if (isalpha(u.ptr()->s[0]))
      ls += 2;
    if (u==at_abs)
      ls = 2;
    // special cases first int, sigma, /, ^
    // and if printed as printsommetasoperator
    // otherwise print with usual functional notation
    int x=0;
    int h=a.fontsize;
    int y=0;
    if ((u==at_integrate) || (u==at_sum) ){ // Int
      int s=1;
      if (arg.type==_VECT)
	s=arg._VECTptr->size();
      else
	arg=vecteur(1,arg);
      // s==1 -> general case
      if ( (s==1) || (s==2) ){ // int f(x) dx and sum f(n) n
	vecteur v(Equation_subsizes(gen(*arg._VECTptr,_SEQ__VECT),a,windowhsize,contextptr));
	eqwdata vv(Equation_total_size(v[0]));
	if (s==1){
	  x=a.fontsize;
	  Equation_translate(v[0],x,0);
	  x += int(fl_width(" dx"));
	}
	if (s==2){
	  if (u==at_integrate){
	    x=a.fontsize;
	    Equation_translate(v[0],x,0);
	    x += vv.dx+int(fl_width(" d"));
	    Equation_vertical_adjust(vv.dy,vv.y,h,y);
	    vv=Equation_total_size(v[1]);
	    Equation_translate(v[1],x,0);
	    Equation_vertical_adjust(vv.dy,vv.y,h,y);
	  }
	  else {
	    Equation_vertical_adjust(vv.dy,vv.y,h,y);
	    eqwdata v1=Equation_total_size(v[1]);
	    x=max(a.fontsize,v1.dx)+a.fontsize/3; // var name size
	    Equation_translate(v[1],0,-v1.dy-v1.y);
	    Equation_vertical_adjust(v1.dy,-v1.dy,h,y);
	    Equation_translate(v[0],x,0);
	    x += vv.dx; // add function size
	  }
	}
	if (u==at_integrate){
	  x += vv.dx;
	  if (h==a.fontsize)
	    h+=2*a.fontsize/3;
	  if (y==0){
	    y=-2*a.fontsize/3;
	    h+=2*a.fontsize/3;
	  }
	}
	v.push_back(eqwdata(x,h,0,y,a,u,0));
	return gen(v,_SEQ__VECT);
      }
      if (s>=3){ // int _a^b f(x) dx
	vecteur & intarg=*arg._VECTptr;
	gen tmp_l,tmp_u,tmp_f,tmp_x;
	attributs aa(a);
	if (a.fontsize>=10)
	  aa.fontsize -= 2;
	tmp_f=Equation_compute_size(intarg[0],a,windowhsize,contextptr);
	tmp_x=Equation_compute_size(intarg[1],a,windowhsize,contextptr);
	tmp_l=Equation_compute_size(intarg[2],aa,windowhsize,contextptr);
	if (s==4)
	  tmp_u=Equation_compute_size(intarg[3],aa,windowhsize,contextptr);
	x=a.fontsize;
	eqwdata vv(Equation_total_size(tmp_l));
	Equation_translate(tmp_l,x-2,-vv.y-vv.dy);
	vv=Equation_total_size(tmp_l);
	Equation_vertical_adjust(vv.dy,vv.y,h,y);
	int lx = vv.dx;
	if (s==4){
	  vv=Equation_total_size(tmp_u);
	  Equation_translate(tmp_u,x,a.fontsize-3-vv.y);
	  vv=Equation_total_size(tmp_u);
	  Equation_vertical_adjust(vv.dy,vv.y,h,y);
	}
	x += max(lx,vv.dx);
	Equation_translate(tmp_f,x,0);
	vv=Equation_total_size(tmp_f);
	Equation_vertical_adjust(vv.dy,vv.y,h,y);
	if (u==at_integrate){
	  x += vv.dx+int(fl_width(" d"));
	  Equation_translate(tmp_x,x,0);
	  vv=Equation_total_size(tmp_x);
	  Equation_vertical_adjust(vv.dy,vv.y,h,y);
	  x += vv.dx;
	}
	else {
	  x += vv.dx;
	  Equation_vertical_adjust(vv.dy,vv.y,h,y);
	  vv=Equation_total_size(tmp_x);
	  x=max(x,vv.dx)+a.fontsize/3;
	  Equation_translate(tmp_x,0,-vv.dy-vv.y);
	  Equation_vertical_adjust(vv.dy,-vv.dy,h,y);
	}
	vecteur res(makevecteur(tmp_f,tmp_x,tmp_l));
	if (s==4)
	  res.push_back(tmp_u);
	res.push_back(eqwdata(x,h,0,y,a,u,0));
	return gen(res,_SEQ__VECT);
      }
    }
    if (u==at_limit && arg.type==_VECT){ // limit
      vecteur & limarg=*arg._VECTptr;
      int s=limarg.size();
      if (s>=3){
	gen tmp_l,tmp_f,tmp_x,tmp_dir;
	attributs aa(a);
	if (a.fontsize>=10)
	  aa.fontsize -= 2;
	tmp_f=Equation_compute_size(limarg[0],a,windowhsize,contextptr);
	tmp_x=Equation_compute_size(limarg[1],aa,windowhsize,contextptr);
	tmp_l=Equation_compute_size(limarg[2],aa,windowhsize,contextptr);
	if (s==4)
	  tmp_dir=Equation_compute_size(limarg[3],aa,windowhsize,contextptr);
	eqwdata vf(Equation_total_size(tmp_f));
	eqwdata vx(Equation_total_size(tmp_x));
	eqwdata vl(Equation_total_size(tmp_l));
	eqwdata vdir(Equation_total_size(tmp_dir));
	int sous=max(vx.dy,vl.dy);
	if (s==4)
	  Equation_translate(tmp_f,vx.dx+vl.dx+vdir.dx+a.fontsize+4,0);
	else
	  Equation_translate(tmp_f,vx.dx+vl.dx+a.fontsize+2,0);
	Equation_translate(tmp_x,0,-sous-vl.y);
	Equation_translate(tmp_l,vx.dx+a.fontsize+2,-sous-vl.y);
	if (s==4)
	  Equation_translate(tmp_dir,vx.dx+vl.dx+a.fontsize+4,-sous-vl.y);
	h=vf.dy;
	y=vf.y;
	vl=Equation_total_size(tmp_l);
	Equation_vertical_adjust(vl.dy,vl.y,h,y);
	vecteur res(makevecteur(tmp_f,tmp_x,tmp_l));
	if (s==4){
	  res.push_back(tmp_dir);
	  res.push_back(eqwdata(vf.dx+vx.dx+a.fontsize+4+vl.dx+vdir.dx,h,0,y,a,u,0));
	}
	else
	  res.push_back(eqwdata(vf.dx+vx.dx+a.fontsize+2+vl.dx,h,0,y,a,u,0));
	return gen(res,_SEQ__VECT);
      }
    }
    if ( (u==at_of || u==at_at) && arg.type==_VECT && arg._VECTptr->size()==2 ){
      // user function, function in 1st arg, arguments in 2nd arg
      gen varg1=Equation_compute_size(arg._VECTptr->front(),a,windowhsize,contextptr);
      eqwdata vv=Equation_total_size(varg1);
      Equation_vertical_adjust(vv.dy,vv.y,h,y);
      gen arg2=arg._VECTptr->back();
      if (u==at_at && xcas_mode(contextptr)!=0){
	if (arg2.type==_VECT)
	  arg2=gen(addvecteur(*arg2._VECTptr,vecteur(arg2._VECTptr->size(),plus_one)),_SEQ__VECT);
	else
	  arg2=arg2+plus_one; 
      }
      gen varg2=Equation_compute_size(arg2,a,windowhsize,contextptr);
      Equation_translate(varg2,vv.dx+llp,0);
      vv=Equation_total_size(varg2);
      Equation_vertical_adjust(vv.dy,vv.y,h,y);
      vecteur res(makevecteur(varg1,varg2));
      res.push_back(eqwdata(vv.dx+vv.x+lrp,h,0,y,a,u,0));
      return gen(res,_SEQ__VECT);
    }
    if (u==at_pow){ 
      // first arg not translated
      gen varg=Equation_compute_size(arg._VECTptr->front(),a,windowhsize,contextptr);
      eqwdata vv=Equation_total_size(varg);
      // 1/2 ->sqrt, otherwise as exponent
      if (arg._VECTptr->back()==plus_one_half){
	Equation_translate(varg,a.fontsize,0);
	vecteur res(1,varg);
	res.push_back(eqwdata(vv.dx+a.fontsize,vv.dy+4,vv.x,vv.y,a,at_sqrt,0));
	return gen(res,_SEQ__VECT);
      }
      if (vv.g.type==_FUNC || vv.g.is_symb_of_sommet(at_pow))
	x=llp;
      Equation_translate(varg,x,0);
      Equation_vertical_adjust(vv.dy,vv.y,h,y);
      vecteur res(1,varg);
      // 2nd arg translated 
      if (vv.g.type==_FUNC || vv.g.is_symb_of_sommet(at_pow))
	x+=vv.dx+lrp;
      else
	x+=vv.dx+1;
      int arg1dy=vv.dy,arg1y=vv.y;
      if (a.fontsize>=10){
	attributs aa(a);
	aa.fontsize -= 2;
	varg=Equation_compute_size(arg._VECTptr->back(),aa,windowhsize,contextptr);
      }
      else
	varg=Equation_compute_size(arg._VECTptr->back(),a,windowhsize,contextptr);
      vv=Equation_total_size(varg);
      Equation_translate(varg,x,arg1y+(3*arg1dy)/4-vv.y);
      res.push_back(varg);
      vv=Equation_total_size(varg);
      Equation_vertical_adjust(vv.dy,vv.y,h,y);
      x += vv.dx;
      res.push_back(eqwdata(x,h,0,y,a,u,0));
      return gen(res,_SEQ__VECT);
    }
    if (u==at_factorial){
      vecteur v;
      gen varg=Equation_compute_size(arg,a,windowhsize,contextptr);
      eqwdata vv=Equation_total_size(varg);
      bool paren=need_parenthesis(vv.g) || vv.g==at_prod || vv.g==at_division || vv.g==at_pow;
      if (paren)
	x+=llp;
      Equation_translate(varg,x,0);
      Equation_vertical_adjust(vv.dy,vv.y,h,y);
      v.push_back(varg);
      x += vv.dx;
      if (paren)
	x+=lrp;
      varg=eqwdata(x+4,h,0,y,a,u,0);
      v.push_back(varg);
      return gen(v,_SEQ__VECT);
    }
    if (u==at_sto){ // A:=B, *it -> B
      gen varg=Equation_compute_size(arg._VECTptr->back(),a,windowhsize,contextptr);
      eqwdata vv=Equation_total_size(varg);
      Equation_vertical_adjust(vv.dy,vv.y,h,y);
      Equation_translate(varg,x,0);
      vecteur v(2);
      v[1]=varg;
      x+=vv.dx;
      x+=ls+3;
      // first arg not translated
      varg=Equation_compute_size(arg._VECTptr->front(),a,windowhsize,contextptr);
      vv=Equation_total_size(varg);
      if (need_parenthesis(vv.g))
	x+=llp;
      Equation_translate(varg,x,0);
      Equation_vertical_adjust(vv.dy,vv.y,h,y);
      v[0]=varg;
      x += vv.dx;
      if (need_parenthesis(vv.g))
	x+=lrp;
      v.push_back(eqwdata(x,h,0,y,a,u,0));
      return gen(v,_SEQ__VECT);
    }
    if (u==at_program && arg._VECTptr->back().type!=_VECT && !arg._VECTptr->back().is_symb_of_sommet(at_local) ){
      gen varg=Equation_compute_size(arg._VECTptr->front(),a,windowhsize,contextptr);
      eqwdata vv=Equation_total_size(varg);
      Equation_vertical_adjust(vv.dy,vv.y,h,y);
      Equation_translate(varg,x,0);
      vecteur v(2);
      v[0]=varg;
      x+=vv.dx;
      x+=int(fl_width("->"))+3;
      varg=Equation_compute_size(arg._VECTptr->back(),a,windowhsize,contextptr);
      vv=Equation_total_size(varg);
      Equation_translate(varg,x,0);
      Equation_vertical_adjust(vv.dy,vv.y,h,y);
      v[1]=varg;
      x += vv.dx;
      v.push_back(eqwdata(x,h,0,y,a,u,0));
      return gen(v,_SEQ__VECT);      
    }
    bool binaryop= (u.ptr()->printsommet==&printsommetasoperator) || equalposcomp(binary_op_tab(),u);
    if ( u!=at_sto && u.ptr()->printsommet!=NULL && !binaryop ){
      gen tmp=string2gen(g.print(contextptr),false);
      return Equation_compute_size(symbolic(at_expr,gen(makevecteur(tmp,xcas_mode(contextptr)),_SEQ__VECT)),a,windowhsize,contextptr);
    }
     vecteur v;
     if (!binaryop || arg.type!=_VECT)
       v=Equation_subsizes(arg,a,windowhsize,contextptr);
     else
       v=Equation_subsizes(gen(*arg._VECTptr,_SEQ__VECT),a,windowhsize,contextptr);
    iterateur it=v.begin(),itend=v.end();
    if ( it==itend || (itend-it==1) ){ 
      gen gtmp;
      if (it==itend)
	gtmp=Equation_compute_size(gen(vecteur(0),_SEQ__VECT),a,windowhsize,contextptr);
      else
	gtmp=*it;
      // unary op, shift arg position horizontally
      eqwdata vv=Equation_total_size(gtmp);
      bool paren = u!=at_neg || (vv.g!=at_prod && need_parenthesis(vv.g)) ;
      x=ls+(paren?llp:0);
      gen tmp=gtmp; Equation_translate(tmp,x,0);
      x=x+vv.dx+(paren?lrp:0);
      Equation_vertical_adjust(vv.dy,vv.y,h,y);
      return gen(makevecteur(tmp,eqwdata(x,h,0,y,a,u,0)),_EQW__VECT);
    }
    if (binaryop){ // op (default with par)
      int currenth=h,largeur=0;
      iterateur itprec=v.begin();
      h=0;
      if (u==at_plus){ // op without parenthesis
	llp=0;
	lrp=0;
      }
      for (;;){
	eqwdata vv=Equation_total_size(*it);
	if (need_parenthesis(vv.g))
	  x+=llp;
	if (u==at_plus && it!=v.begin() &&
	    ( 
	     (it->type==_VECT && it->_VECTptr->back().type==_EQW && it->_VECTptr->back()._EQWptr->g==at_neg) 
	    || 
	     ( it->type==_EQW && (is_integer(it->_EQWptr->g) || it->_EQWptr->g.type==_DOUBLE_) && is_strictly_positive(-it->_EQWptr->g,contextptr) ) 
	     ) 
	    )
	  x -= ls;
	if (x>windowhsize-vv.dx && x>windowhsize/2 && (itend-it)*vv.dx>windowhsize/2){
	  largeur=max(x,largeur);
	  x=0;
	  if (need_parenthesis(vv.g))
	    x+=llp;
	  h+=currenth;
	  Equation_translate(*it,x,0);
	  for (iterateur kt=v.begin();kt!=itprec;++kt)
	    Equation_translate(*kt,0,currenth);
	  if (y){
	    for (iterateur kt=itprec;kt!=it;++kt)
	      Equation_translate(*kt,0,-y);
	  }
	  itprec=it;
	  currenth=vv.dy;
	  y=vv.y;
	}
	else {
	  Equation_translate(*it,x,0);
	  vv=Equation_total_size(*it);
	  Equation_vertical_adjust(vv.dy,vv.y,currenth,y);
	}
	x+=vv.dx;
	if (need_parenthesis(vv.g))
	  x+=lrp;
	++it;
	if (it==itend){
	  for (iterateur kt=v.begin();kt!=itprec;++kt)
	    Equation_translate(*kt,0,currenth+y);
	  h+=currenth;
	  v.push_back(eqwdata(max(x,largeur),h,0,y,a,u,0));
	  // cerr << v << endl;
	  return gen(v,_SEQ__VECT);
	}
	x += ls+3;
      } 
    }
    // normal printing
    x=ls+llp;
    for (;;){
      eqwdata vv=Equation_total_size(*it);
      Equation_translate(*it,x,0);
      Equation_vertical_adjust(vv.dy,vv.y,h,y);
      x+=vv.dx;
      ++it;
      if (it==itend){
	x+=lrp;
	v.push_back(eqwdata(x,h,0,y,a,u,0));
	return gen(v,_SEQ__VECT);
      }
      x+=lc;
    }
  }

  // windowhsize is used for g of type HIST_EQW (history) right justify answers
  // Returns either a eqwdata type object (terminal) or a vector 
  // (of subtype _EQW__VECT or _HIST__VECT)
  gen Equation_compute_size(const gen & g,const attributs & a,int windowhsize,GIAC_CONTEXT){
    fl_font(FL_HELVETICA,a.fontsize);
    /**************
     * FL_WIDGET  *
     **************/
    if (g.type==_POINTER_) {
      if (g.subtype==_FL_WIDGET_POINTER){
	Fl_Widget * wg=(Fl_Widget *) g._POINTER_val;
	return eqwdata(wg->w(),wg->h(),0,0,a,g);
      }
#ifndef __APPLE__
      if (g.subtype==_FL_IMAGE_POINTER){
	Fl_Image * ptr=(Fl_Image *) g._POINTER_val;
	return eqwdata(ptr->w(),ptr->h(),0,0,a,g);
      }
#endif
    }
    /***************
     *   STRINGS   *
     ***************/
    if (g.type==_STRNG){
      string s;
      s=*g._STRNGptr;
      string cs;
      int ss=s.size();
      /* if (!ss)
	 return eqwdata(10,6,0,0,a,g); */
      int hsize=0,vsize=0;
      bool newline=false;
      vecteur res;
      gen tmps;
      for (int pos=0;pos<ss;++pos){
	char ch=s[pos];
	if (ch=='\n'){
	  newline=true;
	  hsize=max(hsize,int(fl_width((' '+cs).c_str())));
	  tmps=string2gen(cs,false);
	  vsize+=a.fontsize;
	  res.push_back(eqwdata(hsize,a.fontsize,0,-vsize,a,tmps));
	  cs="";
	}
	else
	  cs += ch;
      }
      hsize=max(hsize,int(fl_width((' '+cs).c_str())));
      vsize+=a.fontsize;
      tmps=string2gen(cs,false);
      if (!newline){
	tmps.subtype=g.subtype;
	return eqwdata(hsize,a.fontsize,0,0,a,tmps);
      }
      gen tmp=eqwdata(hsize,a.fontsize,0,-vsize,a,tmps);
      res.push_back(tmp);
      res.push_back(eqwdata(hsize,vsize,0,-vsize,a,at_multistring));
      tmp= gen(res,_EQW__VECT);
      Equation_translate(tmp,0,vsize);
      return tmp;
    }
    /*****************
     *   FRACTIONS   *
     *****************/
    if (g.type==_FRAC){
      if (is_integer(g._FRACptr->num) && is_positive(-g._FRACptr->num,contextptr))
	return Equation_compute_size(symbolic(at_neg,fraction(-g._FRACptr->num,g._FRACptr->den)),a,windowhsize,contextptr);
      gen v1=Equation_compute_size(g._FRACptr->num,a,windowhsize,contextptr);
      eqwdata vv1=Equation_total_size(v1);
      gen v2=Equation_compute_size(g._FRACptr->den,a,windowhsize,contextptr);
      eqwdata vv2=Equation_total_size(v2);
      // Center the fraction
      int w1=vv1.dx,w2=vv2.dx;
      int w=max(w1,w2)+6;
      vecteur v(3);
      v[0]=v1; Equation_translate(v[0],(w-w1)/2,11-vv1.y);
      v[1]=v2; Equation_translate(v[1],(w-w2)/2,7-vv2.dy-vv2.y);
      v[2]=eqwdata(w,4+vv1.dy+vv2.dy,0,7-vv2.dy,a,at_division,0);
      return gen(v,_SEQ__VECT);
    }
    /***************
     *   VECTORS   *
     ***************/
    if ( (g.type==_VECT) && !g._VECTptr->empty() ){
      if (g.subtype==_SPREAD__VECT)
	return Equation_compute_size(string2gen("spreadsheet",false),a,windowhsize,contextptr);
      vecteur v;
      const_iterateur it=g._VECTptr->begin(),itend=g._VECTptr->end();
      int x=0,y=0,h=a.fontsize; 
      /***************
       *   HISTORY   *
       ***************/
      if (g.subtype==_HIST__VECT){ 
	int l=windowhsize;
	vecteur vplot;
	for (int i=0;it!=itend;++it,++i){
	  gen tmpg(*it);
	  if (!rpn_mode(contextptr) && it->type==_VECT && !it->_VECTptr->empty()){
	    if (it->_VECTptr->front().type==_STRNG)
	      tmpg=makevecteur(it->_VECTptr->front(),string2gen("",false));
	    gen tmpback=it->_VECTptr->back();
	    if (tmpback.type==_POINTER_ && tmpback.subtype==_FL_WIDGET_POINTER && fl_widget_updatepict_function)
	      tmpback = fl_widget_updatepict_function(tmpback);
	    if (tmpback.is_symb_of_sommet(at_pnt) || (tmpback.type==_VECT && !tmpback._VECTptr->empty() && tmpback._VECTptr->back().is_symb_of_sommet(at_pnt)))
	      vplot.push_back(tmpback);
	    if (tmpback.is_symb_of_sommet(at_erase))
	      vplot.clear();
	    gen itfront=it->_VECTptr->front();
	    if (itfront.is_symb_of_sommet(at_expr)){
	      itfront=itfront._SYMBptr->feuille;
	      int mode=xcas_mode(contextptr);
	      if (itfront.type==_VECT && !itfront._VECTptr->empty()){
		mode=itfront._VECTptr->back().val;
		itfront=itfront._VECTptr->front();
	      }
	      if (itfront.type==_STRNG){
		int save_maple_mode=xcas_mode(contextptr);
		xcas_mode(contextptr)=mode;
		try {
		  itfront=gen(*itfront._STRNGptr,contextptr);
		} catch (std::runtime_error & e){
		}
		xcas_mode(contextptr)=save_maple_mode;
	      }
	    }
	  }
	  tmpg.subtype=_SEQ__VECT;
	  vecteur tmp(Equation_subsizes(tmpg,a,windowhsize,contextptr));
	  iterateur jt=tmp.begin(); // this is the question
	  // compute the size of writing the history level i
	  eqwdata w(Equation_total_size(*jt));
	  if (rpn_mode(contextptr)) // ignore question
	    v.push_back(eqwdata(1,1,x,-y,w.eqw_attributs,string2gen("",false)));
	  else { 
	    y += w.dy + 2;
	    x = int(fl_width((print_INT_(i)+": ").c_str()));
	    if (l<w.dx+x)
	      l=w.dx+x;
	    Equation_translate(*jt,x,-y-w.y); v.push_back(*jt);
	  }
	  jt=tmp.end()-1; // this is the answer
	  *jt=Equation_change_attributs(*jt,attributs(a.fontsize,a.background,a.text_color+4));
	  w=Equation_total_size(*jt);
	  y += w.dy + 2; 
	  x = int(fl_width("    "));
	  l=max(l,w.dx+x);
	  int xshift=x;
	  if (w.dx+4<windowhsize-a.fontsize){
	    if (center_history)
	      xshift=(windowhsize-w.dx-4)/2;
	    else
	      xshift=windowhsize-w.dx-4-a.fontsize;
	  }
	  Equation_translate(*jt,xshift,-y-w.y); v.push_back(*jt);
	}
	v.push_back(eqwdata(l,y,0,-y,a,at_makevector,0));
	// cerr << v << endl;
	gen res=gen(v,_HIST__VECT); Equation_translate(res,0,y); return res;
      } // END HISTORY
      /***************
       *   MATRICE   *
       ***************/
      if (ckmatrix(g) && g.subtype!=_SEQ__VECT && g.subtype!=_SET__VECT && g._VECTptr->front().subtype!=_SEQ__VECT){
	gen mkvect(at_makevector);
	mkvect.subtype=_SEQ__VECT;
	gen mkmat(at_makevector);
	mkmat.subtype=_MATRIX__VECT;
	int nrows,ncols;
	mdims(*g._VECTptr,nrows,ncols);
	if (ncols){
	  vecteur all_sizes;
	  all_sizes.reserve(nrows);
	  vector<int> row_heights(nrows),row_bases(nrows),col_widths(ncols);
	  // vertical gluing
	  for (int i=0;it!=itend;++it,++i){
	    gen tmpg=*it;
	    tmpg.subtype=_SEQ__VECT;
	    vecteur tmp(Equation_subsizes(tmpg,a,max(windowhsize/ncols-a.fontsize,230),contextptr));
	    int h=a.fontsize,y=0;
	    const_iterateur jt=tmp.begin(),jtend=tmp.end();
	    for (int j=0;jt!=jtend;++jt,++j){
	      eqwdata w(Equation_total_size(*jt));
	      Equation_vertical_adjust(w.dy,w.y,h,y);
	      col_widths[j]=max(col_widths[j],w.dx);
	    }
	    if (i)
	      row_heights[i]=row_heights[i-1]+h+a.fontsize;
	    else
	      row_heights[i]=h;
	    row_bases[i]=y;
	    all_sizes.push_back(tmp);
	  }
	  // accumulate col widths
	  col_widths.front() +=(3*a.fontsize)/2;
	  vector<int>::iterator iit=col_widths.begin()+1,iitend=col_widths.end();
	  for (;iit!=iitend;++iit)
	    *iit += *(iit-1)+a.fontsize;
	  // translate each cell
	  it=all_sizes.begin();
	  itend=all_sizes.end();
	  int h,y,prev_h=0;
	  for (int i=0;it!=itend;++it,++i){
	    h=row_heights[i];
	    y=row_bases[i];
	    iterateur jt=it->_VECTptr->begin(),jtend=it->_VECTptr->end();
	    for (int j=0;jt!=jtend;++jt,++j){
	      eqwdata w(Equation_total_size(*jt));
	      if (j)
		Equation_translate(*jt,col_widths[j-1]-w.x,-h-y);
	      else
		Equation_translate(*jt,-w.x+a.fontsize/2,-h-y);
	    }
	    it->_VECTptr->push_back(eqwdata(col_widths.back(),h-prev_h,0,-h,a,mkvect,0));
	    prev_h=h;
	  }
	  all_sizes.push_back(eqwdata(col_widths.back(),row_heights.back(),0,-row_heights.back(),a,mkmat,-row_heights.back()/2));
	  gen all_sizesg=all_sizes; Equation_translate(all_sizesg,0,row_heights.back()/2); return all_sizesg;
	}
      } // end matrices
      /*************************
       *   SEQUENCES/VECTORS   *
       *************************/
      // horizontal gluing
      x += a.fontsize/2;
      int ncols=itend-it;
      //ncols=min(ncols,5);
      for (;it!=itend;++it){
	gen cur_size=Equation_compute_size(*it,a,
					   max(windowhsize/ncols-a.fontsize,
#ifdef IPAQ
					       200
#else
					       480
#endif
					       ),contextptr);
	eqwdata tmp=Equation_total_size(cur_size);
	Equation_translate(cur_size,x-tmp.x,0); v.push_back(cur_size);
	x=x+tmp.dx+a.fontsize;
	Equation_vertical_adjust(tmp.dy,tmp.y,h,y);
      }
      gen mkvect(at_makevector);
      if (g.subtype==_SEQ__VECT)
	mkvect=at_makesuite;
      else
	mkvect.subtype=g.subtype;
      v.push_back(eqwdata(x,h,0,y,a,mkvect,0));
      return gen(v,_EQW__VECT);
    } // end sequences
    if (g.type==_USER){
      if (dynamic_cast<giac::galois_field *>(g._USERptr)){ 
	gen g1(g.print(contextptr),contextptr); 
	return Equation_compute_size(g1,a,windowhsize,contextptr);
      }
    }
    if (g.type!=_SYMB){
      string s=g.print(contextptr);
      if (s.size()>2000)
	s=s.substr(0,2000)+"...";
      fl_font(cst_greek_translate(s),a.fontsize);
      int i=int(fl_width(s.c_str()));
      gen tmp=eqwdata(i,a.fontsize,0,0,a,g);
      return tmp;
    }
    /**********************
     *  SYMBOLIC HANDLING *
     **********************/
    return Equation_compute_symb_size(g,a,windowhsize,contextptr);
    // return Equation_compute_symb_size(aplatir_fois_plus(g),a,windowhsize,contextptr);
    // aplatir_fois_plus is a problem for Equation_replace_selection
    // because it will modify the structure of the data
  }

  string Equation_extract_string(const string & cs,int begin_sel,int end_sel){
    int css=cs.size();
    if (!css)
      return cs;
    int sel0=min(max(0,begin_sel),css-1),sel1=css;
    if (end_sel>=0)
      sel1=min(end_sel,css);
    if (sel0>sel1)
      giac::swapint(sel0,sel1);
    return cs.substr(sel0,sel1-sel0);
  }

  void Equation_draw(const eqwdata & e,int x,int y,int rightx,int lowery,Equation * eq,int begin_sel,int end_sel){
    if ( (e.dx+e.x<x) || (e.x>rightx) || (e.y>y) || e.y+e.dy<lowery)
      return; // nothing to draw, out of window
    const giac::context * contextptr = get_context(eq);
    gen gg=e.g;
    bool selected=e.selected && Fl::focus()==eq;
    int fontsize=e.eqw_attributs.fontsize;
    Fl_Color text_color=Fl_Color(e.eqw_attributs.text_color);
    Fl_Color background=Fl_Color(e.eqw_attributs.background);
    fl_font(FL_HELVETICA,fontsize);
    fl_color(selected?background:text_color);
    if (gg.type==_POINTER_) {
      // wg->resize(e.x-x,y-e.y-e.dy,e.dx,e.dy);
      // wg->draw(); // automatically done if it belongs to the group
      return;
    }
    if (gg.type==_STRNG){
      string s;
      if (e.active){
	// draw s and the cursor
	s=*gg._STRNGptr;
	int ss=s.size();
	int pos=max(min(eq->active_pos,ss),0);
	if (eq->need_active_parse)
	  s=s.substr(0,pos)+"|"+s.substr(pos,ss-pos);
	else
	  s='"'+s.substr(0,pos)+"|"+s.substr(pos,ss-pos);
      }
      else {
	if (gg.subtype)
	  s='"'+*gg._STRNGptr;
	else
	  s=' '+*gg._STRNGptr;
      }
      string cs;
      int ss=s.size();
      int vsize = fontsize -2,pos;
      for (pos=0;pos<ss;++pos){
	char ch=s[pos];
	if (ch=='\n'){
	  check_fl_draw(cs.c_str(),eq->x()+e.x-x,eq->y()+y-e.y+vsize-e.dy,eq->clip_x,eq->clip_y,eq->clip_w,eq->clip_h,0,0);
	  cs="";
	  vsize += fontsize;
	}
	else
	  cs += ch;
      }
      check_fl_draw(cs.c_str(),eq->x()+e.x-x,eq->y()+y-e.y+vsize-e.dy,eq->clip_x,eq->clip_y,eq->clip_w,eq->clip_h,0,0);
      // If selected take care of begin/end selection
      int css=cs.size(); // must be >=1 since selected hence not active
      if (selected && css){
	int sel0=min(begin_sel+1,css-1),sel1=css;
	if (end_sel>=0)
	  sel1=min(end_sel+1,css);
	if (sel0>sel1)
	  giac::swapint(sel0,sel1);
	int deltax=int(fl_width(cs.substr(0,sel0).c_str()));
	cs=cs.substr(sel0,sel1-sel0);
	int dx=int(fl_width(cs.c_str()));
	fl_color(text_color);
	check_fl_rectf(eq->x()+e.x-x+deltax,eq->y()+y-e.y-e.dy+1,dx,e.dy+3,eq->clip_x,eq->clip_y,eq->clip_w,eq->clip_h,0,0);
	fl_color(background);
	check_fl_draw(cs.c_str(),eq->x()+e.x-x+deltax,eq->y()+y-e.y+vsize-e.dy,eq->clip_x,eq->clip_y,eq->clip_w,eq->clip_h,0,0);
      }
      return;
    }
    if (selected){
      fl_color(text_color);
      check_fl_rectf(eq->x()+e.x-x,eq->y()+y-e.y-e.dy+1,e.dx,e.dy+3,eq->clip_x,eq->clip_y,eq->clip_w,eq->clip_h,0,0);
      fl_color(background);
    }
    string s=gg.print(contextptr);
    if (gg.type==_IDNT && !s.empty() && s[0]=='_')
      s=s.substr(1,s.size()-1);
    if (s.size()>2000)
      s=s.substr(0,2000)+"...";
    Fl_Font font=cst_greek_translate(s);
    if (gg.type==_IDNT && font==FL_HELVETICA 
#if !defined(WIN32) && !defined(__APPLE__)
	&& gg!=cst_pi
#endif
	)
      font=FL_TIMES_BOLD_ITALIC; // FL_HELVETICA_BOLD_ITALIC;
    fl_font(font,fontsize);
    // cerr << s.size() << endl;
    check_fl_draw(s.c_str(),eq->x()+e.x-x,eq->y()+y-e.y,eq->clip_x,eq->clip_y,eq->clip_w,eq->clip_h,0,0);
    return;
  }

  void Equation_draw(const gen & g,int x,int y,int rightx,int lowery,Equation * equat){
    const giac::context * contextptr = get_context(equat);
    int eqx=equat->x(),eqy=equat->y();
    if (g.type==_EQW){ // terminal
      eqwdata & e=*g._EQWptr;
      Equation_draw(e,x,y,rightx,lowery,equat,equat->begin_sel,equat->end_sel);
    }
    if (g.type!=_VECT)
      return;
    vecteur & v=*g._VECTptr;
    if (v.empty())
      return;
    gen tmp=v.back();
    if (tmp.type!=_EQW){
      cerr << "EQW error:" << v << endl;
      return;
    }
    eqwdata & w=*tmp._EQWptr;
    if ( (w.dx+w.x-x<0) || (w.x>rightx) || (w.y>y) || (w.y+w.dy<lowery) )
      return; // nothing to draw, out of window
    /*******************
     * draw the vector *
     *******************/
    // v is the vector, w the master operator eqwdata
    gen oper=w.g; 
    bool selected=w.selected && Fl::focus()==equat;
    int fontsize=w.eqw_attributs.fontsize;
    int background=w.eqw_attributs.background;
    int text_color=w.eqw_attributs.text_color;
    int x0=w.x;
    int y0=w.y; // lower coordinate of the master vector
    int y1=y0+w.dy; // upper coordinate of the master vector
    if (selected){
      fl_color(text_color);
      check_fl_rectf(eqx+w.x-x,eqy+y-w.y-w.dy+1,w.dx,w.dy+3,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
    }
    else {
      fl_color(background);
      // fl_rectf(w.x-x,y-w.y-w.dy,w.dx,w.dy-2);
    }
    // draw arguments of v
    const_iterateur it=v.begin(),itend=v.end()-1;
    if (oper==at_multistring){
      const_iterateur it_beg=it,it_end=itend;
      if (Equation_multistring_selection(it_beg,it_end,false)){
	// begin_sel and end_sel apply respect. to it_beg and it_end sel. lines
	for (;it!=it_beg;++it)
	  Equation_draw(*it->_EQWptr,x,y,rightx,lowery,equat,-1,-1);
	if (it_beg==it_end){
	  Equation_draw(*it->_EQWptr,x,y,rightx,lowery,equat,equat->begin_sel,equat->end_sel);
	  ++it;
	}
	else {
	  Equation_draw(*it->_EQWptr,x,y,rightx,lowery,equat,equat->begin_sel,-1);
	  ++it;
	  for (;it!=it_end;++it)
	    Equation_draw(*it->_EQWptr,x,y,rightx,lowery,equat,-1,-1);
	  Equation_draw(*it->_EQWptr,x,y,rightx,lowery,equat,-1,equat->end_sel);
	  ++it;
	}
	for (;it!=itend;++it)
	  Equation_draw(*it->_EQWptr,x,y,rightx,lowery,equat,-1,-1);
	return;
      }
    }
    if (oper==at_expr && v.size()==3){
      Equation_draw(*it,x,y,rightx,lowery,equat);
      return;
    }
    for (;it!=itend;++it)
      Equation_draw(*it,x,y,rightx,lowery,equat);
    if (oper==at_multistring)
      return;
    fl_font(FL_HELVETICA,fontsize);
    fl_color(selected?background:text_color);
    string s;
    if (g.subtype==_HIST__VECT){ // For history, we must write history levels
      it=v.begin();
      int nlevels=(itend-it)/2-1,wlevel;
      int skip=2; // int skip=2-rpn_mode(contextptr);
      for (int i=0;it!=itend;it+=skip,++i){
	eqwdata tmp=Equation_total_size(*it);
	fl_font(FL_HELVETICA,tmp.eqw_attributs.fontsize);
	fl_color(FL_BLUE);
	// cerr << tmp << endl;
	int yy;
	// uncommented, seemed previously to be problematic with strings
	if (tmp.hasbaseline)
	  yy=y-tmp.baseline;
	else
	  yy=y-tmp.y-(tmp.dy-tmp.eqw_attributs.fontsize)/2;
	if (yy<0 || yy>y-lowery)	  
	  continue;
	if (rpn_mode(contextptr))
	  wlevel=nlevels-i;
	else
	  wlevel=i;
	if (wlevel || !rpn_mode(contextptr))
	  check_fl_draw((print_INT_(wlevel)+": ").c_str(),eqx-x,eqy+yy,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
      }
      return; // nothing else to do
    }
    if (oper.type==_FUNC){
      // catch here special cases user function, vect/matr, ^, int, sqrt, etc.
      unary_function_ptr & u=*oper._FUNCptr;
      if (u==at_at){ // draw brackets around 2nd arg
	gen arg2=v[1]; // 2nd arg of at_of, i.e. what's inside the parenth.
	eqwdata varg2=Equation_total_size(arg2);
	x0=varg2.x;
	y0=varg2.y;
	y1=y0+varg2.dy;
	fontsize=varg2.eqw_attributs.fontsize;
	fl_font(FL_HELVETICA,fontsize);
	if (x0<rightx)
	  check_fl_draw("[",eqx+x0-x-int(fl_width("[")),eqy+y-varg2.baseline,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	x0 += varg2.dx ;
	if (x0<rightx)
	  check_fl_draw("]",eqx+x0-x,eqy+y-varg2.baseline,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	return;
      }
      if (u==at_of){ // do we need to draw some parenthesis?
	gen arg2=v[1]; // 2nd arg of at_of, i.e. what's inside the parenth.
	if (arg2.type!=_VECT || arg2._VECTptr->back().type !=_EQW || arg2._VECTptr->back()._EQWptr->g!=at_makesuite){ // Yes (if not _EQW it's a sequence with parent)
	  eqwdata varg2=Equation_total_size(arg2);
	  x0=varg2.x;
	  y0=varg2.y;
	  y1=y0+varg2.dy;
	  fontsize=varg2.eqw_attributs.fontsize;
	  int pfontsize=max(fontsize,(fontsize+(varg2.baseline-varg2.y))/2);
	  fl_font(FL_HELVETICA,pfontsize); // was fontsize
	  if (x0<rightx)
	    check_fl_draw("(",eqx+x0-x-int(fl_width("(")),eqy+y-varg2.baseline,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	  x0 += varg2.dx ;
	  if (x0<rightx)
	    check_fl_draw(")",eqx+x0-x,eqy+y-varg2.baseline,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	}
	return;
      }
      if (u==at_makesuite){
	bool paren=v.size()!=2; // Sequences with 1 arg don't show parenthesis
	int pfontsize=max(fontsize,(fontsize+(w.baseline-w.y))/2);
	fl_font(FL_HELVETICA,pfontsize);
	if (paren && x0<rightx)
	  check_fl_draw("(",eqx+x0-x-int(fl_width("("))/2,eqy+y-w.baseline,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	x0 += w.dx;
	if (paren && x0<rightx)
	  check_fl_draw(")",eqx+x0-x-int(fl_width("("))/2,eqy+y-w.baseline,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0); 
	// print commas between args
	it=v.begin(),itend=v.end()-2;
	for (;it!=itend;++it){
	  eqwdata varg2=Equation_total_size(*it);
	  fontsize=varg2.eqw_attributs.fontsize;
	  fl_font(FL_HELVETICA,fontsize);
	  if (varg2.x+varg2.dx<rightx)
	    check_fl_draw(",",eqx+varg2.x+varg2.dx-x+1,eqy+y-varg2.baseline,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	}
	return;
      }
      if (u==at_makevector){ // draw [] delimiters for vector/matrices
	if (oper.subtype!=_SEQ__VECT){
	  int decal=1;
	  switch (oper.subtype){
	  case _MATRIX__VECT: decal=2; break;
	  case _SET__VECT: decal=4; break;
	  case _POLY1__VECT: decal=6; break;
	  }
	  if (x0+1<rightx){
	    check_fl_line(eqx+x0-x+1,eqy+y-y0+1,eqx+x0-x+1,eqy+y-y1+1,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	    check_fl_line(eqx+x0-x+decal,eqy+y-y0+1,eqx+x0-x+decal,eqy+y-y1+1,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	    check_fl_line(eqx+x0-x+1,eqy+y-y0+1,eqx+x0-x+fontsize/4,eqy+y-y0+1,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	    check_fl_line(eqx+x0-x+1,eqy+y-y1+1,eqx+x0-x+fontsize/4,eqy+y-y1+1,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	  }
	  x0 += w.dx ;
	  if (x0-1<rightx){
	    check_fl_line(eqx+x0-x-1,eqy+y-y0+1,eqx+x0-x-1,eqy+y-y1+1,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	    check_fl_line(eqx+x0-x-decal,eqy+y-y0+1,eqx+x0-x-decal,eqy+y-y1+1,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	    check_fl_line(eqx+x0-x-1,eqy+y-y0+1,eqx+x0-x-fontsize/4,eqy+y-y0+1,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	    check_fl_line(eqx+x0-x-1,eqy+y-y1+1,eqx+x0-x-fontsize/4,eqy+y-y1+1,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	  }
	} // end if oper.subtype!=SEQ__VECT
	if (oper.subtype!=_MATRIX__VECT){
	  // print commas between args
	  it=v.begin(),itend=v.end()-2;
	  for (;it!=itend;++it){
	    eqwdata varg2=Equation_total_size(*it);
	    fontsize=varg2.eqw_attributs.fontsize;
	    fl_font(FL_HELVETICA,fontsize);
	    if (varg2.x+varg2.dx<rightx)
	      check_fl_draw(",",eqx+varg2.x+varg2.dx-x+1,eqy+y-varg2.baseline,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	  }
	}
	return;
      }
      int lpsize=int(fl_width("("));
      int rpsize=int(fl_width(")"));
      eqwdata tmp=Equation_total_size(v.front()); // tmp= 1st arg eqwdata
      if (u==at_sto)
	tmp=Equation_total_size(v[1]);
      x0=w.x-x;
      y0=y-w.baseline;
      if (u==at_pow){
	if (!need_parenthesis(tmp.g)&& tmp.g!=at_pow && tmp.g!=at_prod && tmp.g!=at_division)
	  return;
	if (tmp.g==at_pow){
	  fontsize=tmp.eqw_attributs.fontsize+2;
	  fl_font(FL_HELVETICA,fontsize);
	}
	if (tmp.x-lpsize<rightx)
	  check_fl_draw("(",eqx+tmp.x-x-lpsize,eqy+y-tmp.baseline,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	if (tmp.x+tmp.dx<rightx)
	  check_fl_draw(")",eqx+tmp.x+tmp.dx-x,eqy+y-tmp.baseline,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	return;
      }
      if (u==at_program){
	if (tmp.x+tmp.dx<rightx)
	  check_fl_draw("->",eqx+tmp.x+tmp.dx-x,eqy+y-tmp.baseline,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	return;
      }
      if (u==at_sum){
	if (x0<rightx){
	  check_fl_line(eqx+x0,eqy+y0,eqx+x0+(2*fontsize)/3,eqy+y0,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	  check_fl_line(eqx+x0,eqy+y0-fontsize,eqx+x0+(2*fontsize)/3,eqy+y0-fontsize,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	  check_fl_line(eqx+x0,eqy+y0,eqx+x0+fontsize/2,eqy+y0-fontsize/2,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	  check_fl_line(eqx+x0+fontsize/2,eqy+y0-fontsize/2,eqx+x0,eqy+y0-fontsize,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	  if (v.size()>2){ // draw the =
	    eqwdata ptmp=Equation_total_size(v[1]);
	    if (ptmp.x+ptmp.dx<rightx)
	      check_fl_draw("=",eqx+ptmp.x+ptmp.dx-x-2,eqy+y-ptmp.baseline,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	  }
	}
	return;
      }
      if (u==at_abs){
	y0 =1+y-w.y;
	int h=w.dy;
	if (x0<rightx){
	  check_fl_line(eqx+x0+2,eqy+y0-1,eqx+x0+2,eqy+y0-h+3,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	  check_fl_line(eqx+x0+1,eqy+y0-1,eqx+x0+1,eqy+y0-h+3,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	  check_fl_line(eqx+x0+w.dx-1,eqy+y0-1,eqx+x0+w.dx-1,eqy+y0-h+3,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	  check_fl_line(eqx+x0+w.dx,eqy+y0-1,eqx+x0+w.dx,eqy+y0-h+3,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	}
	return;
      }
      if (u==at_sqrt){
	y0 =1+y-w.y;
	int h=w.dy;
	if (x0<rightx){
	  check_fl_line(eqx+x0+2,eqy+y0-h/2,eqx+x0+fontsize/2,eqy+y0-1,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	  check_fl_line(eqx+x0+fontsize/2,eqy+y0-1,eqx+x0+fontsize,eqy+y0-h+3,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	  check_fl_line(eqx+x0+fontsize,eqy+y0-h+3,eqx+x0+w.dx-1,eqy+y0-h+3,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	  ++y0;
	  check_fl_line(eqx+x0+2,eqy+y0-h/2,eqx+x0+fontsize/2,eqy+y0-1,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	  check_fl_line(eqx+x0+fontsize/2,eqy+y0-1,eqx+x0+fontsize,eqy+y0-h+3,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	  check_fl_line(eqx+x0+fontsize,eqy+y0-h+3,eqx+x0+w.dx-1,eqy+y0-h+3,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	}
	return;
      }
      if (u==at_factorial){
	check_fl_draw("!",eqx+w.x+w.dx-4-x,eqy+y-w.baseline,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	if (!need_parenthesis(tmp.g)
	    && tmp.g!=at_pow && tmp.g!=at_prod && tmp.g!=at_division
	    )
	  return;
	if (tmp.x-lpsize<rightx)
	  check_fl_draw("(",eqx+tmp.x-x-lpsize,eqy+y-tmp.baseline,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	if (tmp.x+tmp.dx<rightx)
	  check_fl_draw(")",eqx+tmp.x+tmp.dx-x,eqy+y-tmp.baseline,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	return;
      }
      if (u==at_integrate){
	x0+=2;
	y0+=fontsize/2;
	if (x0<rightx){
	  fl_arc(eqx+x0,eqy+y0,fontsize/3,fontsize/3,180,360);
	  check_fl_line(eqx+x0+fontsize/3,eqy+y0,eqx+x0+fontsize/3,eqy+y0-2*fontsize+4,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	  fl_arc(eqx+x0+fontsize/3,eqy+y0-2*fontsize+3,fontsize/3,fontsize/3,0,180);
	}
	if (v.size()!=2){ // if arg has size > 1 draw the d
	  eqwdata ptmp=Equation_total_size(v[1]);
	  if (ptmp.x<rightx)
	    check_fl_draw(" d",eqx+ptmp.x-x-int(fl_width(" d")),eqy+y-ptmp.baseline,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	}
	else {
	  eqwdata ptmp=Equation_total_size(v[0]);
	  if (ptmp.x+ptmp.dx<rightx)
	    check_fl_draw(" dx",eqx+ptmp.x+ptmp.dx-x,eqy+y-ptmp.baseline,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	}
	return;
      }
      if (u==at_division){
	if (x0<rightx){
	  int yy=eqy+y0-6;
	  check_fl_line(eqx+x0+2,yy,eqx+x0+w.dx-2,yy,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	  ++yy;
	  check_fl_line(eqx+x0+2,yy,eqx+x0+w.dx-2,yy,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	}
	return;
      }
      if (u==at_limit && v.size()>=4){
	if (x0<rightx)
	  check_fl_draw("lim",eqx+w.x-x,eqy+y-w.baseline,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	gen arg2=v[1]; // 2nd arg of limit, i.e. the variable
	if (arg2.type==_EQW){ 
	  eqwdata & varg2=*arg2._EQWptr;
	  if (varg2.x+varg2.dx+2<rightx)
	    check_fl_draw("->",eqx+varg2.x+varg2.dx+2-x,eqy+y-varg2.y,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	}
	if (v.size()>=5){
	  arg2=v[2]; // 3rd arg of lim, the point, draw a comma after if dir.
	  if (arg2.type==_EQW){ 
	    eqwdata & varg2=*arg2._EQWptr;
	    if (varg2.x+varg2.dx<rightx)
	      check_fl_draw(",",eqx+varg2.x+varg2.dx-x,eqy+y-varg2.baseline,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	  }
	}
	return;
      }
      bool parenthesis=true;
      string opstring(",");
      if (u.ptr()->printsommet==&printsommetasoperator || equalposcomp(binary_op_tab(),u) )
	opstring=u.ptr()->s;
      else {
	if (u==at_sto)
	  opstring=":=";
	parenthesis=false;
      }
      // int yy=y0; // y0 is the lower coordinate of the whole eqwdata
      // int opsize=int(fl_width(opstring.c_str()))+3;
      it=v.begin();
      itend=v.end()-1;
      // Reminder: here tmp is the 1st arg eqwdata, w the whole eqwdata
      if ( (itend-it==1) && ( (u==at_neg) 
			      || (u==at_plus) // uncommented for +infinity
			      ) ){ 
	if (u==at_neg &&need_parenthesis(tmp.g) && tmp.g!=at_prod){
	  if (tmp.x-lpsize<rightx)
	    check_fl_draw("(",eqx+tmp.x-x-lpsize,eqy+y-tmp.baseline,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	  if (tmp.x+tmp.dx<rightx)
	    check_fl_draw(")",eqx+tmp.x-x+tmp.dx,eqy+y-tmp.baseline,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	}
	if (w.x<rightx){
#if 1
	  if (u==at_neg){
	    int xx=w.x-x;
	    int cx=eqx+xx+2,cy=eqy+y-w.baseline-fontsize/3,ch=giacmax(2,(fontsize+5)/6);
	    if (fontsize<16) --cx; else ++cy;
	    if (fontsize<12) --cx;
	    if (fontsize>23) ++cx;
	    if (fontsize>29) ++cx;
	    if (fontsize>37) ++cx;
	    fl_line(cx-ch,cy,cx+ch-1,cy);
	    --cy;
	    fl_line(cx-ch,cy,cx+ch-1,cy);
	    if (fontsize>13){
	      --cy;
	      fl_line(cx-ch,cy,cx+ch-1,cy);
	      if (fontsize>23){
		cy+=3;
		fl_line(cx-ch,cy,cx+ch-1,cy);
	      }
	    }
	  }
	  else
	    check_fl_draw(u.ptr()->s,eqx+w.x-x,eqy+y-w.baseline,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
#else
	  fl_font(FL_TIMES_BOLD,fontsize);
	  check_fl_draw(u.ptr()->s,eqx+w.x-x,eqy+y-w.baseline,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
#endif
	}
	return;
      }
      // write first open parenthesis
      if (u==at_plus)
	parenthesis=false;
      else {
	if (parenthesis && need_parenthesis(tmp.g)){
	  if (w.x<rightx){
	    int pfontsize=max(fontsize,(fontsize+(tmp.baseline-tmp.y))/2);
	    fl_font(FL_HELVETICA,pfontsize);
	    check_fl_draw("(",eqx+w.x-x,eqy+y-tmp.baseline,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	    fl_font(FL_HELVETICA,fontsize);
	  }
	}
      }
      for (;;){
	// write close parenthesis at end
	int xx=tmp.dx+tmp.x-x;
	if (parenthesis && need_parenthesis(tmp.g)){
	  if (xx<rightx){
	    int pfontsize=min(max(fontsize,(fontsize+(tmp.baseline-tmp.y))/2),fontsize*2);
	    int deltapary=(2*(pfontsize-fontsize))/3;
	    fl_font(FL_HELVETICA,pfontsize);
	    check_fl_draw(")",eqx+xx,eqy+y-tmp.baseline+deltapary,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	    fl_font(FL_HELVETICA,fontsize);
	  }
	  xx +=rpsize;
	}
	++it;
	if (it==itend){
	  if (u.ptr()->printsommet==&printsommetasoperator || u==at_sto || equalposcomp(binary_op_tab(),u))
	    return;
	  else
	    break;
	}
	// write operator
	if (u==at_prod){
#if 1
	  // check_fl_draw(".",eqx+xx+3,eqy+y-tmp.baseline-fontsize/3,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	  fl_font(FL_TIMES_ITALIC,fontsize);
	  check_fl_draw(opstring.c_str(),eqx+xx+1,eqy+y-tmp.baseline,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
#else
	  int cx=eqx+xx+5,cy=-3+eqy+y-tmp.baseline-fontsize/3,ch=giacmax(2,fontsize/6);
	  // fl_line(cx-ch+1,cy,cx+ch-1,cy); // horizontal (smaller to avoid confusion with -)
	  if (fontsize<16) --cx;
	  if (fontsize<12) --cx;
	  fl_line(cx,cy+ch,cx,cy-ch);
	  fl_line(cx-ch,cy+ch,cx+ch,cy-ch);
	  fl_line(cx-ch,cy-ch,cx+ch,cy+ch);
#endif
	}
	else {
	  gen tmpgen;
	  if (u==at_plus && ( 
			     (it->type==_VECT && it->_VECTptr->back().type==_EQW && it->_VECTptr->back()._EQWptr->g==at_neg) 
			     || 
			     ( it->type==_EQW && (is_integer(it->_EQWptr->g) || it->_EQWptr->g.type==_DOUBLE_) && is_strictly_positive(-it->_EQWptr->g,contextptr) ) 
			     )
	      )
	    ;
	  else {
	    if (xx+1<rightx)
	      // fl_draw(opstring.c_str(),xx+1,y-tmp.y-tmp.dy/2+fontsize/2);
	      check_fl_draw(opstring.c_str(),eqx+xx+1,eqy+y-tmp.baseline,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	  }
	}
	// write right parent, update tmp
	tmp=Equation_total_size(*it);
	if (parenthesis && (need_parenthesis(tmp.g)) ){
	  if (tmp.x-lpsize<rightx){
	    int pfontsize=min(max(fontsize,(fontsize+(tmp.baseline-tmp.y))/2),fontsize*2);
	    int deltapary=(2*(pfontsize-fontsize))/3;
	    fl_font(FL_HELVETICA,pfontsize);
	    check_fl_draw("(",eqx+tmp.x-pfontsize*lpsize/fontsize-x,eqy+y-tmp.baseline+deltapary,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	    fl_font(FL_HELVETICA,fontsize);
	  }
	}
      } // end for (;;)
      if (w.x<rightx){
	s = u.ptr()->s;
	fl_font(cst_greek_translate(s),fontsize);
	s += '(';
	check_fl_draw(s.c_str(),eqx+w.x-x,eqy+y-w.baseline,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
	fl_font(FL_HELVETICA,fontsize);
      }
      if (w.x+w.dx-rpsize<rightx)
	check_fl_draw(")",eqx+w.x+w.dx-x-rpsize,eqy+y-w.baseline,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
      return;
    }
    s=oper.print(contextptr);
    if (w.x<rightx){
      fl_font(cst_greek_translate(s),fontsize);
      check_fl_draw(s.c_str(),eqx+w.x-x,eqy+y-w.baseline,equat->clip_x,equat->clip_y,equat->clip_w,equat->clip_h,0,0);
      fl_font(FL_HELVETICA,fontsize);
    }
  }

  void Equation::draw(){
    fl_clip_box(x(),y(),w(),h(),clip_x,clip_y,clip_w,clip_h);
    // cout << clip_x << " " << clip_y << " " << clip_w << " " << clip_h << endl;
    fl_push_clip(clip_x,clip_y,clip_w,clip_h);
    fl_color(attr.background);
    fl_rectf(clip_x, clip_y, clip_w, clip_h);
    fl_color(attr.text_color);
    fl_rect(x(), y(), w(), h());
    Equation_draw(data,xleft,ytop,xleft+w()-vscroll->w(),ytop-h()-hscroll->h(),this);
    Fl_Group::draw_children();
    fl_pop_clip();
  }

  bool Equation_box_sizes(const gen & g,int & l,int & h,int & x,int & y,attributs & attr,bool & selected,bool search_active=false){
    if (g.type==_EQW){
      eqwdata & w=*g._EQWptr;
      x=w.x;
      y=w.y;
      l=w.dx;
      h=w.dy;
      if (search_active)
	selected=w.active;
      else
	selected=w.selected;
      attr=w.eqw_attributs;
      return true;
    }
    else {
      if (g.type!=_VECT || g._VECTptr->empty() ){
	l=0;
	h=0;
	x=0;
	y=0;
	attr=attributs(0,0,0);
	selected=false;
	return true;
      }
      gen & g1=g._VECTptr->back();
      Equation_box_sizes(g1,l,h,x,y,attr,selected,search_active);
      return false;
    }
  }

  // Set the scrollbars according to data, xleft, ytop
  void Equation::setscroll(){
    int w=this->w()-vscroll->w();
    int h=this->h()-hscroll->h();
    redraw();
    eqwdata e=Equation_total_size(data);
    if (e.dx>w){
      xleft=max(e.x,xleft);
      xleft=min(e.x+e.dx-w,xleft);
      hscroll->value(xleft-e.x,w,0,e.dx);
      hscroll->show();
    }
    else {
      xleft = e.x - (w-e.dx)/2;
      hscroll->value(0,1,0,1);
      hscroll->hide();
    }
    if (e.dy>h){
      ytop=max(e.y+h,ytop);
      ytop=min(e.y+e.dy,ytop);
      vscroll->value(e.dy+e.y-ytop,h,0,e.dy);
      vscroll->show();
    }
    else {
      ytop = e.y + (e.dy + h)/2;
      vscroll->value(0,1,0,1);
      vscroll->hide();
    }
  }

  void Equation_cb_scroll(Fl_Widget * w, void*) {
    Equation_Scrollbar * s=dynamic_cast<Equation_Scrollbar *>(w);
    if (!s)
      return;
    if (s->vertical){
      eqwdata e=Equation_total_size(s->eqwptr->data);
      s->eqwptr->ytop=e.dy+e.y-s->value();
    }
    else
      s->eqwptr->xleft=s->value();
    s->eqwptr->setscroll();
    s->eqwptr->redraw();
  }

  // select or deselect part of the current eqution
  // This is done *in place*
  void Equation_select(gen & g,bool select,bool active_search){
    if (g.type==_EQW){
      eqwdata & e=*g._EQWptr;
      if (active_search)
	e.active=select;
      else
	e.selected=select;
    }
    if (g.type!=_VECT)
      return;
    vecteur & v=*g._VECTptr;
    iterateur it=v.begin(),itend=v.end();
    for (;it!=itend;++it)
      Equation_select(*it,select,active_search);
  }

  void Equation::deselect(){
    begin_sel=-1;
    end_sel=-1;
    Equation_select(data,false);
    redraw();
  }

  void Equation::select(){
    begin_sel=-1;
    end_sel=-1;
    Equation_select(data,true);
    adjust_xy_sel();
    redraw();
  }

  void Equation::fl_select(){
    const giac::context * contextptr = get_context(this);
    gen g=get_selection();
    string s;
    if (g.type==_STRNG) s=*g._STRNGptr; else s=g.print(contextptr);
    Fl::selection(*this,s.c_str(),s.size());
    if (cb_select)
      cb_select(s.c_str());
  }

  // check if interval xmin..xmax and x1..x2 have a non empty intersection
  bool interval_crossing(int xmin,int xmax,int x1,int x2){
    if (xmax<x1 || xmin>x2)
      return false;
    return true;
  }

  // Select in g the subrectangle xmin,ymin,xmax,ymax
  void Equation_select_rectangle(gen & g,int x1,int y1,int x2,int y2,Equation * eq){
    int xmin=min(x1,x2),xmax=max(x1,x2),ymin=min(y1,y2),ymax=max(y1,y2);
    if (g.type==_EQW) {
      Equation_select(g,true);
      if (g._EQWptr->g.type==_STRNG){
	// compute begin/end_sel
	if (y1<y2){
	  giac::swapint(y1,y2);
	  giac::swapint(x1,x2);
	}
	eq->begin_sel=Equation_binary_search_pos(*g._EQWptr,x1,y1);
	eq->end_sel=Equation_binary_search_pos(*g._EQWptr,x2,y2);
      }
      return;
    }
    if (g.type!=_VECT)
      return;
    vecteur & v=*g._VECTptr;
    // Find the first element of v containing the whole rectangle
    iterateur it=v.begin(),itend=v.end()-1,last_sel_it;
    for (;it!=itend;++it){
      int x,y,l,h;
      attributs attr(0,0,0);
      bool selected;
      Equation_box_sizes(*it,l,h,x,y,attr,selected);
      if ( (x<=xmin) && (y<=ymin) && (xmax<=x+l) && (ymax<=y+h) ){
	Equation_select_rectangle(*it,x1,y1,x2,y2,eq);
	return;
      }
    }
    // None, then we select each element that crosses the rectangle
    bool selectall=true,find_first_sel=false,find_last_sel=false;
    if (itend->type==_EQW && itend->_EQWptr->g==at_multistring){
      if (y1<y2){
	giac::swapint(y1,y2);
	giac::swapint(x1,x2);
      }
      find_first_sel=true;
      selectall=false;
    }
    it=v.begin();
    for (;it!=itend;++it){
      int x,y,l,h;
      attributs attr(0,0,0);
      bool selected;
      Equation_box_sizes(*it,l,h,x,y,attr,selected);
      bool xcross=interval_crossing(xmin,xmax,x,x+l),ycross=interval_crossing(ymin,ymax,y,y+h);
      if ( xcross && ycross ){
	Equation_select(*it,true);
	last_sel_it=it;
	if (find_first_sel){
	  find_first_sel=false;
	  find_last_sel=true;
	  eq->begin_sel=Equation_binary_search_pos(*it->_EQWptr,x1,y1);
	}
      }
      else {
	if (ycross && g.subtype==_HIST__VECT){
	  Equation_select(*it,true);
	  last_sel_it=it;
	}
	else
	  selectall=false;
      }
    }
    if (find_last_sel)
      eq->end_sel=Equation_binary_search_pos(*last_sel_it->_EQWptr,x2,y2);
    if (selectall) 
      Equation_select(g,true);
  }

  void Equation::select_rectangle(int x,int y){
    // x,y are the end of the mouse selection
    Equation_select_rectangle(data,xsel,ysel,x,y,this);
  }

  // return true if g has some selection inside
  bool Equation_adjust_xy(const gen & g,int & xleft,int & ytop,int & xright,int & ybottom,bool active_search=false){
    int x,y,w,h;
    attributs f(0,0,0);
    bool selected;
    Equation_box_sizes(g,w,h,x,y,f,selected,active_search);
    if ( (g.type==_EQW__VECT) || selected ){ // terminal or selected
      xleft=x;
      ybottom=y;
      if (selected){ // g is selected
	ytop=y+h;
	xright=x+w;
	return true;
      }
      else { // no selection
	xright=x;
	ytop=y;
	return false;
      }
    }
    if (g.type!=_VECT)
      return false;
    // last not selected, recurse
    iterateur it=g._VECTptr->begin(),itend=g._VECTptr->end()-1;
    for (;it!=itend;++it){
      if (Equation_adjust_xy(*it,xleft,ytop,xright,ybottom,active_search))
	return true;
    }
    return false;
  }
 
  void Equation::adjust_xy(){
    // recalculate xsel, ysel to begin top of selection
    // and xcur, ycur to end bottom of selection
    Equation_adjust_xy(data,xsel,ysel,xcur,ycur);
  }

  void Equation::adjust_xy_sel(){ 
    // same + recalc xleft
    Equation_adjust_xy(data,xsel,ysel,xcur,ycur);
    if ((xcur>xleft+w()) || (xsel<xleft) )
      xleft=max(xsel-w()/2,0);
    if ( ycur < ytop-h() || ysel >ytop )
      ytop=ysel;
    // recalc scrollbars
    setscroll();
  }

  // Find i position of first selected item in it..itend
  bool Equation_find_multistring_pos(const_iterateur it,const_iterateur itend,int & i,int &nrows,bool active_search=false){
    int t;
    nrows=itend-it;
    for (i=0;it!=itend;++it,++i){ // find row
      if (it->type!=_EQW || it->_EQWptr->g.type!=_STRNG)
	return false;
      if (Equation_adjust_xy(*it,t,t,t,t,active_search))
	break;
    }
    if (it==itend)
      return false;
    return true;
  }

  // Assumes it points to the beginning of a string, selects position i
  void Equation_select_multistring_pos(iterateur it,int i){
    iterateur jt=(it+i);
    Equation_select(*jt,true);
  }

  // find selection position in it..itend 
  // Return true if at least 1 item is selected
  bool Equation_find_vector_pos(const_iterateur it,const_iterateur itend,int & i,int &nrows){
    int t;
    nrows=itend-it;
    for (i=0;it!=itend;++it,++i){ // find row in i
      if (Equation_adjust_xy(*it,t,t,t,t))
	break;
    }
    if (it==itend){
      --i;
      return false;
    }
    return true;
  }

  // Find i,j position of first selected item in it..itend
  bool Equation_find_matrix_pos(const_iterateur it,const_iterateur itend,int & i,int &j,int &nrows,int & ncols,bool active_search=false){
    // (incomplete) check for a matrix: the first element must be a vector
    if (it->type!=_VECT || it->_VECTptr->empty() )
      return false;
    gen & tmp=it->_VECTptr->back();
    if (tmp.type!=_EQW || tmp._EQWptr->g !=at_makevector)
      return false;
    // find selection position and move down
    int t;
    nrows=itend-it;
    int c=it->_VECTptr->size();
    const_iterateur it0=it;
    for (;it0!=itend;++it0){ // more complete matrix check
      if (it0->type!=_VECT || int(it0->_VECTptr->size())!=c)
	return false;
    }
    for (i=0;it!=itend;++it,++i){ // find row
      if (Equation_adjust_xy(*it,t,t,t,t,active_search))
	break;
    }
    if (it==itend || it->type!=_VECT )
      return false;
    // find column
    const_iterateur jt=it->_VECTptr->begin(),jtend=it->_VECTptr->end()-1;
    ncols=jtend-jt;
    for (j=0;jt!=jtend;++jt,++j){
      if (Equation_adjust_xy(*jt,t,t,t,t,active_search))
	break;
    }
    if (jt==jtend)
      return false;
    return true;
  }

  // Assumes it points to the beginning of the matrix, selects position i,j
  void Equation_select_matrix_pos(iterateur it,int i,int j){
    iterateur jt=(it+i)->_VECTptr->begin()+j;
    Equation_select(*jt,true);
  }

  // increase selection up (like HP49 eqw Up key)
  // x,y is the cursor position
  void Equation_select_up(gen & g,int x,int y,int mode){
    if (g.type==_EQW){ // terminal -> select
      g._EQWptr->selected=true;
      return;
    }
    if (g.type!=_VECT || g._VECTptr->empty())
      return;
    vecteur & v=*g._VECTptr;
    iterateur it=v.begin(),itend=v.end()-1;
    if (v.back().type!=_EQW)
      setsizeerr();
    eqwdata & w=*v.back()._EQWptr;
    int i,j,nrows,ncols;
    if ( mode && w.g==at_makevector && Equation_find_matrix_pos(it,itend,i,j,nrows,ncols) ){
      --i;
      if (i<0)
	i=nrows-1;
      Equation_select(g,false);
      Equation_select_matrix_pos(it,i,j);
      return;
    }
    if ( mode && w.g==at_multistring && Equation_find_multistring_pos(it,itend,i,nrows) ){
      --i;
      if (i<0)
	i=nrows-1;
      Equation_select(g,false);
      Equation_select_multistring_pos(it,i);
      return;
    }
    // find a box containing the cursor
    int x0,y0,w0,h0;
    attributs attr(0,0,0);
    bool selected;
    for (;it!=itend;++it){
      Equation_box_sizes(*it,w0,h0,x0,y0,attr,selected);
      if ( (x0<x) && (x0+w0>=x) && (y0<=y) && (y0+h0>=y) )
	break;
    }
    if (it==itend)
      return;
    // if the box is selected select g else recurse
    if (selected)
      Equation_select(g,true);
    else
      Equation_select_up(*it,x,y,mode);
  }

  void Equation::select_up(int mode){
    const giac::context * contextptr = get_context(this);
    begin_sel=-1;
    end_sel=-1;
    // mode is 1 for shifted key
    Equation_select_up(data,xcur,ycur,mode);
    adjust_xy_sel();
    if (cb_select)
      cb_select(get_selection().print(contextptr).c_str());
    redraw();
  }

  // decrease selection up (like HP49 eqw Down key)
  void Equation_select_down(gen & g,int x,int y,int mode, Equation * eq){
    const giac::context * contextptr = get_context(eq);
    if (g.type==_EQW){ // terminal -> deselect and activate
      eqwdata & e=*g._EQWptr;
      e.selected=false;
      if (eq->modifiable){
	e.active=true;
	eq->active_pos=0;
	if (e.g.type==_STRNG )
	  eq->need_active_parse=false;
	else {
	  eq->need_active_parse=true;
	  e.g=string2gen(e.g.print(contextptr),false);
	  e.g.subtype=1;
	}
      }
      return;
    }
    if (g.type!=_VECT || g._VECTptr->empty() )
      return;
    vecteur & v=*g._VECTptr;
    iterateur it=v.begin(),itend=v.end()-1;
    if (itend->type!=_EQW)
      setsizeerr();
    eqwdata & w=*itend->_EQWptr;
    if (w.selected){ // deselect operator (sommet of the symbolic)
      w.selected=false;
      if (itend-it==1)
	return;
    }
    int i,j,nrows,ncols;
    if ( mode && w.g==at_makevector && Equation_find_matrix_pos(it,itend,i,j,nrows,ncols) ){
      ++i;
      if (i>=nrows)
	i=0;
      Equation_select(g,false);
      Equation_select_matrix_pos(it,i,j);
      return;
    }
    if ( mode && w.g==at_multistring && Equation_find_multistring_pos(it,itend,i,nrows) ){
      ++i;
      if (i>=nrows)
	i=0;
      Equation_select(g,false);
      Equation_select_multistring_pos(it,i);
      return;
    }
    // not selected, find the 1st selected element if any
    int x0,y0,w0,h0;
    attributs attr(0,0,0);
    bool selected;
    for (;it!=itend;++it){
      Equation_box_sizes(*it,w0,h0,x0,y0,attr,selected);
      if (selected)
	break;
    }
    if (selected){ // deselect all elements except this one
      iterateur it1=v.begin();
      bool des=false;
      int tmp;
      for (;it1!=itend;++it1){
	if (it1!=it){ // if *it1 was selected deselect it
	  if (Equation_adjust_xy(*it1,tmp,tmp,tmp,tmp)){
	    des=true;
	    Equation_select(*it1,false);
	  }
	}
      }
      if (!des){ // nothing else was selected, recurse
	Equation_select_down(*it,x,y,mode,eq);
      }
      return;
    }
    it=v.begin();
    for (;it!=itend;++it){
      Equation_box_sizes(*it,w0,h0,x0,y0,attr,selected);
      if ( (x0<=x) && (x0+w0>=x) && (y0<=y) && (y0+h0>=y) )
	break;
    }
    if (it!=itend)
      Equation_select_down(*it,x,y,mode,eq);
  }

  void Equation::select_down(int mode){
    const giac::context * contextptr = get_context(this);
    begin_sel=-1;
    end_sel=-1;
    Equation_select_down(data,xcur,ycur,mode,this);
    adjust_xy_sel();
    if (cb_select)
      cb_select(get_selection().print(contextptr).c_str());
    redraw();
  }

  gen Equation_data2gen(const gen & g,GIAC_CONTEXT){
    if (g.type==_EQW){
      eqwdata & e=*g._EQWptr;
      if ( e.g.type==_STRNG && e.active && e.g.subtype )
	 return gen(*e.g._STRNGptr,contextptr);
      return e.g;
    }
    if (g.type!=_VECT)
      return g;
    vecteur & v=*g._VECTptr;
    const_iterateur it=v.begin(),itend=v.end()-1;
    vecteur res;
    if (g.subtype==_HIST__VECT){
      vecteur tmp(2);
      for (;it!=itend;++it){
	tmp[0]=Equation_data2gen(*it,contextptr);
	++it;
	if (it==itend)
	  break;
	tmp[1]=Equation_data2gen(*it,contextptr);
	res.push_back(tmp);
      }
      return gen(res,_HIST__VECT);
    }
    res.reserve(itend-it);
    for (;it!=itend;++it){
      res.push_back(Equation_data2gen(*it,contextptr));
    }
    gen f=Equation_data2gen(*it,contextptr);
    if (f==at_makesuite)
      return gen(res,_SEQ__VECT);
    if (f==at_makevector){
      if (f.subtype==_MATRIX__VECT && ckmatrix(res)){
	iterateur it=res.begin();itend=res.end();
	for (;it!=itend;++it)
	  it->subtype=0;
	return res;
      }
      return gen(res,f.subtype);
    }
    if (f==at_program && res.size()==2)
      res.insert(res.begin()+1,res.front()*zero);
    if (f==at_at && xcas_mode(contextptr)!=0){
      if (res.size()==2){
	if (res.back().type==_VECT){
	  vecteur & resv=*res.back()._VECTptr;
	  res.back()=gen(subvecteur(resv,vecteur(resv.size(),plus_one)),_SEQ__VECT);
	}
	else
	  res.back()=res.back()-plus_one;
      }
    }
    gen arg=gen(res,g.subtype);
    if (f.type==_FUNC) {
      if (res.size()==1) 
	return symbolic(*f._FUNCptr,res.front());
      else {
	if (f==at_multistring){
	  gen tmp=_multistring(res,contextptr);
	  return tmp;
	}
	return symbolic(*f._FUNCptr,arg);
      }
    }
    else
      return f(arg,contextptr);
  }

  gen Equation_selected2gen(Equation * eq,const gen & g,bool & selected,bool search_active=false){
    const giac::context * contextptr = get_context(eq);
    if (g.type==_EQW){
      eqwdata & e=*g._EQWptr;
      if (search_active)
	selected = e.active;
      else
	selected = e.selected;
      if (!selected)
	return Equation_nullstring();
      if (e.g.type!=_STRNG)
	return e.g;
      if (e.active && e.g.subtype )
	return gen(*e.g._STRNGptr,contextptr); // parse
      // begin_sel/end_sel
      string res=*e.g._STRNGptr;
      if (!e.active)
	res=Equation_extract_string(*e.g._STRNGptr,eq->begin_sel,eq->end_sel);
      return string2gen(res,false);
    }
    selected=false;
    if (g.type!=_VECT)
      return g;
    vecteur & v=*g._VECTptr;
    const_iterateur it=v.begin(),itend=v.end()-1;
    if (itend->type==_EQW && itend->_EQWptr->g==at_multistring){
      string res;
      if (!Equation_multistring_selection(it,itend,search_active))
	return string2gen(res,false);
      selected=true;
      if (it==itend) // selection inside one string
	return Equation_selected2gen(eq,*it,selected,search_active);
      res=Equation_extract_string(*it->_EQWptr->g._STRNGptr,eq->begin_sel,-1)+'\n';
      ++it;
      for (;it!=itend;++it)
	res=res+*it->_EQWptr->g._STRNGptr+'\n';
      res=res+Equation_extract_string(*itend->_EQWptr->g._STRNGptr,-1,eq->end_sel);
      return string2gen(res,false);
    }
    int w0,h0,x0,y0;
    attributs attr(0,0,0);
    Equation_box_sizes(*itend,w0,h0,x0,y0,attr,selected,search_active);
    if (selected)
      return Equation_data2gen(g,contextptr);
    vecteur res;
    res.reserve(itend-it);
    for (;it!=itend;++it){
      gen tmp=Equation_selected2gen(eq,*it,selected,search_active);
      if (selected)
	res.push_back(tmp);
    }
    selected=!res.empty();
    if (!selected)
      return zero;
    if (res.size()==1) 
      return res.front();
    gen f=Equation_data2gen(*it,contextptr);
    if (f==at_makesuite)
      return gen(res,_SEQ__VECT);
    if (f==at_makevector){
      if (f.subtype==_MATRIX__VECT && ckmatrix(res)){
	iterateur it=res.begin();itend=res.end();
	for (;it!=itend;++it)
	  it->subtype=0;
	return res;
      }
      return gen(res,f.subtype);
    }
    if (f==at_program && res.size()==2)
      res.insert(res.begin()+1,res.front()*zero);
    if (f==at_at && xcas_mode(contextptr)!=0){
      if (res.size()==2){
	if (res.back().type==_VECT){
	  vecteur & resv=*res.back()._VECTptr;
	  res.back()=gen(subvecteur(resv,vecteur(resv.size(),plus_one)),_SEQ__VECT);
	}
	else
	  res.back()=res.back()-plus_one;
      }
    }
    gen arg=gen(res,g.subtype);
    if (f.type==_FUNC)
      return symbolic(*f._FUNCptr,arg);
    else
      return f(arg,contextptr);
  }

  // return a pointer to the selected object
  // it will modify g if for example a subsum of a sum is selected
  gen * Equation_selected(gen & g,attributs & attr,int windowhsize,vecteur & position,bool active_search,GIAC_CONTEXT){
    // FIXME begin_sel/end_sel
    int x0,y0,w0,h0,tmp;
    bool selected;
    if (!Equation_adjust_xy(g,tmp,tmp,tmp,tmp,active_search))
      return 0;
    Equation_box_sizes(g,w0,h0,x0,y0,attr,selected,active_search);
    if (selected)
      return &g;
    if (g.type!=_VECT)
      return 0;
    vecteur & v=*g._VECTptr;
    iterateur it=v.begin(),itend=v.end()-1;
    for (;it!=itend;++it){ // search first selected item
      Equation_box_sizes(*it,w0,h0,x0,y0,attr,selected,active_search);
      if (selected)
	break;
    }
    if (it==itend){
      it=v.begin();
      for (;it!=itend;++it){
	if (Equation_adjust_xy(*it,tmp,tmp,tmp,tmp,active_search)){
	  position.push_back(int(it-v.begin()));
	  return Equation_selected(*it,attr,windowhsize,position,active_search,contextptr);
	}
      }
      return 0;
    }
    iterateur itb=it;
    position.push_back(int(it-v.begin()));
    // find the end of the selection
    ++it;
    for (;it!=itend;++it){
      Equation_box_sizes(*it,w0,h0,x0,y0,attr,selected,active_search);
      if (!selected)
	break;
    }
    --it;
    Equation_box_sizes(*it,w0,h0,x0,y0,attr,selected,active_search);
    if (it==itb)
      return &*it;
    // arrange v
    if (itb==v.begin() && it+1==itend){
      position.pop_back();
      return &g;
    }
    vecteur subop(itb,it+1);
    subop.push_back(v.back());
    gen temp=Equation_data2gen(subop,contextptr);
    eqwdata e=Equation_total_size(*it);
    *it=Equation_compute_size(temp,attr,windowhsize,contextptr);
    Equation_translate(*it,e.x,e.y);
    int pos=itb-v.begin();
    v.erase(itb,it);
    return &v[pos];
  }

  // make a free copy of g
  gen Equation_copy(const gen & g){
    if (g.type==_EQW)
      return *g._EQWptr;
    if (g.type!=_VECT)
      return g;
    vecteur & v = *g._VECTptr;
    const_iterateur it=v.begin(),itend=v.end();
    vecteur res;
    res.reserve(itend-it);
    for (;it!=itend;++it)
      res.push_back(Equation_copy(*it));
    return gen(res,g.subtype);
  }

  // move selection right (like HP49 eqw Right key)
  void Equation_select_rightleft(gen & g,Equation * eqptr,int windowhsize,bool right,int mode,GIAC_CONTEXT){
    if (g.type!=_VECT)  // we are on a terminal, nothing to do
      return;
    // find 1st object with something selected
    vecteur & v=*g._VECTptr;
    iterateur it=v.begin(),itend=v.end()-1;
    eqwdata & w=*itend->_EQWptr;
    int i,j,nrows,ncols,tmp;
    if ( (mode) && (w.g==at_makevector) && Equation_find_matrix_pos(it,itend,i,j,nrows,ncols) ){
      if (right)
	++j;
      else
	--j;
      if (j<0)
	j=ncols-1;
      if (j>=ncols)
	j=0;
      Equation_select(g,false);
      Equation_select_matrix_pos(it,i,j);
      return;
    }
    for (;it!=itend;++it){
      if (Equation_adjust_xy(*it,tmp,tmp,tmp,tmp))
	break;
    }
    if (it==itend)
      return;
    iterateur it0=it; // save first selected
    // find first non selected
    if (right || (mode==2) ){
      ++it;
      for (;it!=itend;++it){
	if (!Equation_adjust_xy(*it,tmp,tmp,tmp,tmp))
	  break;
      }
      if (mode==2 && it0==v.begin() && it==itend)
	return;
      --it;
    }
    // it -> last selected if moving right or exchanging selection right/left
    int x0,y0,w0,h0;
    attributs attr(0,0,0);
    bool selected;
    Equation_box_sizes(*it,w0,h0,x0,y0,attr,selected);
    if (!selected){
      Equation_select_rightleft(*it,eqptr,windowhsize,right,mode,contextptr);
      return;
    }
    iterateur it1=it;
    if (right){
      ++it;
      if (it==itend)
	it=v.begin();
    }
    else {
      it=it0;
      if (it==v.begin())
	it=itend;
      --it;
    }
    if (mode==2){ // exchange it0->it1 with it
      if (it1==it0){
	gen tmp=*it;
	*it=*it0;
	*it0=tmp;
      }
      else {
	vecteur tmpv(it0,it1+1);
	tmpv.push_back(*itend);
	gen sel=Equation_data2gen(tmpv,contextptr);
	sel=Equation_compute_size(sel,attr,windowhsize,contextptr);
	Equation_select(sel,true);
	gen tmp=*it;
	*it=sel;
	*it1=tmp;
	v.erase(it0,it1);
      }
      vecteur position;
      gen g_copy=Equation_copy(eqptr->data);
      gen * ptr=Equation_selected(eqptr->data,attr,windowhsize,position,false,contextptr);
      if (ptr){
	if (ptr->type==_VECT && !ptr->_VECTptr->empty() && ptr->_VECTptr->back().type==_EQW && ptr->_VECTptr->back()._EQWptr->g==at_multistring){
	  eqptr->data=g_copy;
	  return;
	}
	const gen & tmp=Equation_data2gen(eqptr->data,contextptr);
	eqptr->data=Equation_compute_size(tmp,attr,windowhsize,contextptr);
	const_iterateur it=position.begin(),itend=position.end();
	ptr=&eqptr->data;
	for (;it!=itend;++it){
	  ptr=&(*ptr->_VECTptr)[it->val];
	}
	Equation_select(*ptr,true);
      }
      return;
    }
    else {
      // if shift-arrow, extend selection, mode==0 do not deselect
      if (mode==1)
	Equation_select(g,false);
    }
    Equation_select(*it,true);
  }

  void Equation::select_right(int mode){
    begin_sel=-1;
    end_sel=-1;
    const giac::context * contextptr = get_context(this);
    Equation_select_rightleft(data,this,w(),true,mode,contextptr);
    adjust_xy_sel();
    if (cb_select)
      cb_select(get_selection().print(contextptr).c_str());
    redraw();
  }

  void Equation::select_left(int mode){
    begin_sel=-1;
    end_sel=-1;
    const giac::context * contextptr = get_context(this);
    Equation_select_rightleft(data,this,w(),false,mode,contextptr);
    adjust_xy_sel();
    if (cb_select)
      cb_select(get_selection().print(contextptr).c_str());
    redraw();
  }

  /* // modify g in place
  void Equation_eval_function(const gen & f,gen & g,attributs iattr,int windowhsize){
    
    attributs attr(0,0,0);
    vecteur position;
    gen g_copy=Equation_copy(g);
    gen * ptr=Equation_selected(g,attr,windowhsize,position,contextptr);
    if (!ptr) // should begin entry mode here
      return; 
    if (ptr->type==_VECT && !ptr->_VECTptr->empty() && ptr->_VECTptr->back().type==_EQW && ptr->_VECTptr->back()._EQWptr->g==at_multistring){
      g=g_copy;
      return;
    }
    gen tmp=Equation_data2gen(*ptr,contextptr);
    tmp=f(eval(tmp));
    * ptr = Equation_compute_size(tmp,attr,windowhsize,contextptr);
    tmp=Equation_data2gen(g,contextptr);
    g=Equation_compute_size(tmp,iattr,windowhsize,contextptr);
    const_iterateur it=position.begin(),itend=position.end();
    ptr=&g;
    for (;it!=itend;++it){
      ptr=&(*ptr->_VECTptr)[it->val];
    }
    Equation_select(*ptr,true);
  }
  */

  void Equation::save_data(){
    int us=undo_history.size();
    if (undo_history_pos>=int(us)){
      undo_history.push_back(get_data());
    }
    else {
      undo_history[undo_history_pos]=get_data();
      iterateur it=undo_history.begin()+undo_history_pos+1;
      undo_history.erase(it,undo_history.end());
    }
    us=undo_history.size();
    undo_history_pos=us;
    if (us>1){ // check that the last 2 levels are not identical
      if (undo_history[us-2]==undo_history[us-1]){
	undo_history.pop_back();
	--undo_history_pos;
      }
    }
  }

  void Equation::rcl_data(int dpos){
    int us=undo_history.size();
    if (!us)
      return;
    if (undo_history_pos>=us)
      undo_history_pos=us-1;
    undo_history_pos += dpos ;
    if (undo_history_pos>=us)
      undo_history_pos=us-1;
    if (undo_history_pos<0)
      undo_history_pos=0;
    set_data(undo_history[undo_history_pos]);
    select();
    // adjust_xy_sel();
    // redraw();
  }

  void Equation::eval_function(const gen & f){
    save_data();
    gen fx=symbolic(at_of,makevecteur(f,get_selection()));
    replace_selection_wo_save(fx);
    save_data();
    xcas::History_Pack * hp=get_history_pack(this);
    giac::context * cptr=hp?hp->contextptr:0;
    //bool ok=
    make_thread(fx,eval_level(cptr),Equation_eval_callback,this,cptr);
  }

  gen Equation::get_selection(){
    bool has_selection;
    return Equation_selected2gen(this,data,has_selection);    
  }

  gen Equation::get_data() const{
    const giac::context * contextptr = get_context(this);
    return Equation_data2gen(data,contextptr);
  }

  string Equation::value() const{
    const giac::context * contextptr = get_context(this);
    return Equation_data2gen(data,contextptr).print(contextptr);
  }

  void Equation::set_data(const gen & g){
    const giac::context * contextptr = get_context(this);
    data=Equation_compute_size(g,attr,w(),contextptr);
    eqwdata e = Equation_total_size(data);
    // FIXME? ytop = e.y;
    ytop=h();
    // FIXME? xleft = e.x;
    xleft=0;
    setscroll();
  }

  bool Equation_replace_selection(const gen & f,Equation * eqwptr,int windowhsize,bool active_search=false){
    const giac::context * contextptr = get_context(eqwptr);
    eqwptr->redraw();
    gen & g = eqwptr->data;
    attributs attr(0,0,0);
    vecteur position;
    gen save_g=Equation_copy(g);
    gen * ptr=Equation_selected(g,attr,windowhsize,position,active_search,contextptr);
    if (!ptr) {
      g=save_g;
      return false; 
    }
    if (g.type==_VECT && g.subtype==_HIST__VECT && !position.empty()){
      if ( (position.front().val%2) ^ rpn_mode(contextptr)){
	g=save_g;
	return false;
      }
    }
    bool stringrep= !active_search && ptr->type==_EQW && ptr->_EQWptr->g.type==_STRNG && eqwptr->begin_sel>=0 && eqwptr->end_sel>=0 ;
    bool multistringrep= ptr->type==_VECT && !ptr->_VECTptr->empty() && ptr->_VECTptr->back().type==_EQW && ptr->_VECTptr->back()._EQWptr->g==at_multistring;
    if (multistringrep){ // check if the whole multistring is replaced or not
      *ptr=Equation_nullstring(eqwptr->attr,0,contextptr);
      multistringrep= in_multistring(eqwptr->data)!=-1;
    }
    if (stringrep || multistringrep){
      g=save_g;
      eqwptr->remove_selection();
      vecteur position;
      gen * act=Equation_selected(eqwptr->data,eqwptr->attr,eqwptr->w(),position,1,contextptr);
      string s;
      if (f.type==_STRNG)
	s=*f._STRNGptr;
      else
	s=f.print(contextptr);
      eqwptr->handle_text(s,act);
      return true;
    }
    if (g.type==_VECT && g.subtype==_HIST__VECT && position.size()>=1 && position.size()<=2){
      int pos=position.front().val; 
      bool doit=pos>=0 && pos<int(g._VECTptr->size());
      if (doit){
	if (position.size()==2){
	  doit=false;
	  if (is_zero(position.back())){ 
	    gen gtmp=g[pos];
	    if (gtmp.type==_VECT && gtmp._VECTptr->back().type==_EQW && gtmp._VECTptr->back()._EQWptr->g==at_expr)
	      doit=true;
	  }
	}
      }
      if (doit){ 
	g = save_g;
	Equation_select(g,false,active_search);
	vecteur & v = *g._VECTptr;
	int s=v.size(),newdx;
	eqwdata olde=Equation_total_size(v[pos]);
	if (position.size()==2){
	  gen vposfront=Equation_data2gen(v[pos],contextptr);
	  gen vposfrontf=vposfront._SYMBptr->feuille;
	  if (vposfrontf.type==_VECT && !vposfrontf._VECTptr->empty()){
	    vecteur vposfrontv=*vposfrontf._VECTptr;
	    vposfrontv[0]=f;
	    vposfrontf=gen(vposfrontv,vposfrontf.subtype);
	  }
	  else
	    vposfrontf=f;
	  v[pos]=Equation_compute_size(symbolic(at_expr,vposfrontf),olde.eqw_attributs,windowhsize,contextptr);
	}
	else {
	  v[pos]=Equation_compute_size(f,olde.eqw_attributs,windowhsize,contextptr);
	}
	eqwdata newe=Equation_total_size(v[pos]);
	int deltady=newe.dy-olde.dy;
	if (pos%2){
	  int xshift=int(fl_width("    "));;
	  if (newe.dx+4<windowhsize-eqwptr->attr.fontsize){
	    if (center_history)
	      xshift=(windowhsize-newe.dx-4)/2;
	    else
	      xshift=windowhsize-newe.dx-4-eqwptr->attr.fontsize;
	  }
	  newdx=newe.dx+xshift;
	  Equation_translate(v[pos],xshift-newe.x,olde.y-newe.y-deltady);
	}
	else {
	  newdx=newe.dx+olde.x;
	  Equation_translate(v[pos],olde.x-newe.x,olde.y-newe.y-deltady);
	}
	if (position.size()==2)
	  Equation_select(v[pos]._VECTptr->front(),true,active_search);
	else
	  Equation_select(v[pos],true,active_search);
	// translate the rest of history
	for (++pos;pos<s;++pos){
	  Equation_translate(v[pos],0,-deltady);
	}
	--pos;
	if (v[pos].type==_EQW){ // should be true!
	  eqwdata & e = *v[pos]._EQWptr;
	  e.dx=max(e.dx,newdx);
	  eqwptr->xleft=e.dx;
	}
	eqwptr->setscroll();
	return true;
      }
    }
    eqwdata olde=Equation_total_size(*ptr);
    *ptr=Equation_compute_size(f,olde.eqw_attributs,windowhsize,contextptr);
    gen tmp=Equation_data2gen(g,contextptr);
    g=Equation_compute_size(tmp,eqwptr->attr,windowhsize,contextptr);
    const_iterateur it=position.begin(),itend=position.end();
    ptr=&g;
    for (;it!=itend;++it){
      ptr=&(*ptr->_VECTptr)[it->val];
    }
    Equation_select(*ptr,true,active_search);
    return true;
  }

  bool Equation::replace_selection(const gen & f,bool active_search){
    save_data();
    return replace_selection_wo_save(f,active_search);
  }

  void Equation::adjust_widget_size(){
    // recompute widget size
    eqwdata e=Equation_total_size(data);
    int maxh=window()?(window()->h())/3:1000;
    int scrollsize=3;
    if (e.dx>w()-2*labelsize())
      scrollsize=labelsize()+3;
    int newh=min(e.dy+scrollsize+1,maxh); // =max(min(vv.dy+20,400),60);
    increase_size(this,newh-h());
    int largeur=labelsize();
    vscroll->resize(x()+Fl_Group::w()-largeur,y(),largeur,Fl_Group::h()-labelsize());
    hscroll->resize(x(),y()+Fl_Group::h()-labelsize(),Fl_Group::w()-2*largeur,labelsize());
    menubar->resize(hscroll->x()+hscroll->w(),hscroll->y(),this->w()-hscroll->w(),hscroll->h());
  }

  bool Equation::replace_selection_wo_save(const gen & f,bool active_search){
    bool res=Equation_replace_selection(f,this,w(),active_search);
    if (res)
      save_data();
    // recompute widget size
    adjust_widget_size();
    if (!active_search)
      fl_select();
    adjust_xy_sel();
    redraw();
    return res;
  }

  // deselect and activate
  bool Equation_deselect_and_activate(gen & g){
    if (g.type==_EQW){
      if (g._EQWptr->selected){
	g._EQWptr->selected=false;
	g._EQWptr->active=true;
	return true;
      }
      else
	return false;
    }
    if (g.type!=_VECT)
      return false;
    vecteur & v = *g._VECTptr;
    iterateur it=v.begin(),itend=v.end()-1;
    if (Equation_deselect_and_activate(*itend))
      return true;
    for (;it!=itend;++it){
      if (Equation_deselect_and_activate(*it))
	return true;
    }
    return false;
  }

  void Equation::deselect_and_activate(){
    begin_sel=-1;
    end_sel=-1;
    active_pos=0;
    Equation_deselect_and_activate(data);
  }
  
  bool Equation_desactivate_and_select(gen & g,bool need_active_parse){
    if (g.type==_EQW){
      if (g._EQWptr->active){
	g._EQWptr->active=false;
	g._EQWptr->selected=true;
	if (need_active_parse && g._EQWptr->g.type==_STRNG )
	  g._EQWptr->g=gen(*g._EQWptr->g._STRNGptr,context0);
	return true;
      }
      else
	return false;
    }
    if (g.type!=_VECT)
      return false;
    vecteur & v = *g._VECTptr;
    iterateur it=v.begin(),itend=v.end()-1;
    if (Equation_desactivate_and_select(*itend,need_active_parse))
      return true;
    for (;it!=itend;++it){
      if (Equation_desactivate_and_select(*it,need_active_parse))
	return true;
    }
    return false;
  }

  void Equation::desactivate_and_select(){
    begin_sel=-1;
    end_sel=-1;
    Equation_desactivate_and_select(data,need_active_parse);
  }
  
  // if nothing selected, select active -> select something
  // Parse selection
  void Equation::insure_something_selected(bool selectall){
    bool has_selection=false;
    gen g=Equation_selected2gen(this,data,has_selection);
    if (!has_selection){
      if (selectall)
	select();
      else {
	// Check for something active, desactive and select
	g=Equation_selected2gen(this,data,has_selection,1);
	if (has_selection){
	  desactivate_and_select();
	}
	else {
	  select_up(0);
	  g=Equation_selected2gen(this,data,has_selection);
	}
      }
    }
  }

  bool Equation_parse_desactivate(gen & g,Equation * eq){
    const giac::context * contextptr = get_context(eq);
    if (g.type==_EQW){
      eqwdata & e=*g._EQWptr;
      if (!e.active)
	return false;
      e.active=false;
      e.selected=true;
      if (e.g.type==_STRNG && eq->need_active_parse){
	try {
	  gen g1=gen(*e.g._STRNGptr,contextptr);
	  e.g=g1;
	  eq->replace_selection(g1);
	}
	catch (std::runtime_error & er){
	  cerr << er.what() << endl;
	}
      }
      return true;
    }
    if (g.type!=_VECT)
      return false;
    vecteur & v =*g._VECTptr;
    iterateur it=v.begin(),itend=v.end();
    for (;it!=itend;++it){
      if (Equation_parse_desactivate(*it,eq))
	return true;
    }
    return false;
  }

  bool Equation::parse_desactivate(){
    begin_sel=-1;
    end_sel=-1;
    if (!Equation_parse_desactivate(data,this))
      return false;
    return true;
  }

  // return true if something selected inside or whole selected
  bool Equation_is_selected(const gen & g,bool inside=false,bool active_search=false){
    if (g.type==_EQW){
      if (active_search)
	return g._EQWptr->active;
      else
	return g._EQWptr->selected;
    }
    if (g.type!=_VECT || g._VECTptr->empty())
      return false;
    if (Equation_is_selected(g._VECTptr->back(),inside,active_search))
      return true;
    if (!inside)
      return false;
    const_iterateur it=g._VECTptr->begin(),itend=g._VECTptr->end();
    for (;it!=itend;++it){
      if (Equation_is_selected(*it,inside,active_search))
	return true;
    }
    return false;
  }

  bool Equation::is_selected(bool inside,bool active_search){
    return Equation_is_selected(data,inside,active_search);
  }

  gen Equation_nullstring(){
    gen tmps=string2gen("",false);
    tmps.subtype=1;
    return tmps;
  }

  gen Equation_nullhistlevel(){
    gen hh1,hh2(string2gen("",false));
    hh1=Equation_nullstring();
    gen hh=gen(vecteur(1,makevecteur(hh1,hh2)),_HIST__VECT);
    return hh;
  }

  bool is_semi_expr(const gen & g){
    if (!g.is_symb_of_sommet(at_expr))
      return false;
    gen tmp=g._SYMBptr->feuille;
    return tmp.type==_VECT && tmp.subtype==_SEQ__VECT && tmp._VECTptr->front().type==_STRNG && *tmp._VECTptr->front()._STRNGptr==";";
  }

  gen Equation_nullstring(const attributs & a,int windowhsize,GIAC_CONTEXT){
    gen g=Equation_compute_size(Equation_nullstring(),a,windowhsize,contextptr);
    Equation_select(g,true,1);
    return g;
  }

  void Equation_remove_selection(gen & g,int & pos,Equation * eq){
    const giac::context * contextptr = get_context(eq);
    if (g.type==_EQW){
      eqwdata & e=*g._EQWptr;
      if (e.g.type==_STRNG){
	eq->need_active_parse=false;
	string s = *e.g._STRNGptr;
	int ss=s.size();
	if (ss){
	  int begin_sel=max(0,min(eq->begin_sel,ss-1));
	  int end_sel=ss;
	  if (eq->end_sel>=0)
	    end_sel=min(eq->end_sel,ss);
	  if (end_sel<begin_sel)
	    giac::swapint(begin_sel,end_sel);
	  if (end_sel<ss)
	    s = s.substr(0,begin_sel)+s.substr(end_sel,ss-end_sel);
	  else
	    s = s.substr(0,begin_sel);
	  pos = begin_sel;
	}
	else
	  pos=0;
	e.g = string2gen(s,false);
      }
      else {
	eq->need_active_parse=true;
	e.g=string2gen(e.g.print(contextptr),false);
	e.g.subtype=1;
	pos=e.g._STRNGptr->size();
      }
      e.active=true;
      e.selected=false;
      return;
    }
    if (g.type!=_VECT)
      return;
    vecteur & v =*g._VECTptr;
    iterateur it=v.begin(),itend=v.end();
    if (Equation_is_selected(*(itend-1),false)){
      g=Equation_nullstring(v.back()._EQWptr->eqw_attributs,0,contextptr);
      pos=0;
      return;
    }
    for (;it!=itend;++it){
      if (Equation_is_selected(*it,false))
	break;
    }
    if (it!=itend){
      iterateur it0=it;
      int pos=it0-v.begin();
      for (;it!=itend;++it){
	if (!Equation_is_selected(*it,false))
	  break;
      }
      if (v.back().type == _EQW && v.back()._EQWptr->g==at_multistring && it0->type==_EQW && it0->_EQWptr->g.type==_STRNG && eq->begin_sel>=0 && eq->end_sel>=0){
	--it; // it0 first selected, it last selected
	int ss=it->_EQWptr->g._STRNGptr->size();
	int begin_sel=min(eq->begin_sel,ss),end_sel=min(eq->end_sel,ss);
	Equation_select(*it0,false);
	if (it==it0){
	  *it->_EQWptr->g._STRNGptr=it->_EQWptr->g._STRNGptr->substr(0,begin_sel)+it->_EQWptr->g._STRNGptr->substr(end_sel,ss-end_sel);
	  Equation_select(*it,true,true);
	  eq->active_pos=begin_sel;
	  eq->need_active_parse=false;
	  return ;
	}
	begin_sel=giacmin(eq->begin_sel,it0->_EQWptr->g._STRNGptr->size());
	*it0->_EQWptr->g._STRNGptr=it0->_EQWptr->g._STRNGptr->substr(0,begin_sel)+it->_EQWptr->g._STRNGptr->substr(end_sel,ss-end_sel);
	Equation_select(*it0,true,true);
	eq->active_pos=begin_sel;
	eq->need_active_parse=false;
	++it0;
	++it;
	iterateur it1=it0;
	int deltay=0;
	int n=it0-v.begin();
	for (;it1!=it;++it1){
	  eqwdata e=Equation_total_size(*it1);
	  deltay=deltay+e.dy;
	}
	v.erase(it0,it);
	int s=v.size();
	for (int j=n;j<s;++j){
	  Equation_translate(v[j],0,deltay);
	}
	return;
      }
      // Check whether it0 is a string, in that case erase in the string
      if (it0+1==it && it0->type==_EQW && it0->_EQWptr->g.type==_STRNG){
	int ss=it0->_EQWptr->g._STRNGptr->size();
	int begin_sel=max(min(eq->begin_sel,ss),0),end_sel=max(min(eq->end_sel,ss),0);
	Equation_select(*it0,false);
	*it0->_EQWptr->g._STRNGptr=it0->_EQWptr->g._STRNGptr->substr(0,begin_sel)+it0->_EQWptr->g._STRNGptr->substr(end_sel,ss-end_sel);      
	Equation_select(*it0,true,true);
	eq->active_pos=begin_sel;
	eq->need_active_parse=false;
	return ;
      }
      // delete from it0 to it
      // NOTE that for history, it must be of even size
      if (g.subtype==_HIST__VECT){
	if ( (it0-v.begin())%2)
	  --it0;
	if ( (it-v.begin())%2)
	  ++it;
      }
      v.erase(it0,it);
      if (v.size()==1){ // only the last item, i.e. operator
	g=Equation_nullstring(v.back()._EQWptr->eqw_attributs,0,contextptr);
	pos=0;
	return;
      }
      if (v.size()==2 
	  // && (v.back()._EQWptr->g==at_plus || v.back()._EQWptr->g==at_prod) 
	  ){
	g=v.front();
	Equation_select(g,true);
	return;
      }
      /* it=v.begin(),itend=v.end();
	 for (;it!=itend;++it)
	 Equation_select(*it,true); */
      if (pos>1 && pos<int(v.size()))
	Equation_select(v[pos-1],true);
      return;
    }
    // Remove 1st selection 
    it=v.begin();
    for (;it!=itend;++it){
      if (Equation_is_selected(*it,true)){
	Equation_remove_selection(*it,pos,eq);
	return;
      }
    }
  }

  void Equation::remove_selection(){
    save_data();
    Equation_remove_selection(data,active_pos,this);
    begin_sel=-1;
    end_sel=-1;
    redraw();
  }

  void Equation::replace_down_left_activate(const gen & g){
    if (replace_selection(g)){
      select_down(0);
      select_left(0); 
      deselect_and_activate();
      if (in_multistring(data)==-1)
	need_active_parse=true;
    }
  }

  void handle_backspace(Equation * eqwptr,int pos,vecteur * vptr){
    string s=*(*vptr)[pos]._EQWptr->g._STRNGptr;
    iterateur it=vptr->begin()+pos;
    vptr->erase(it);
    --pos;
    string s0=*(*vptr)[pos]._EQWptr->g._STRNGptr;
    eqwptr->active_pos=s0.size();
    s=s0+s;
    Equation_select((*vptr)[pos],true,true);
    eqwptr->replace_selection(string2gen(s,false),1);
  }

  // act = 0 if nothing active or -> the active part
  // return true if the new selected part must be copied in clipboard
  bool Equation::handle_key(unsigned char c,gen * act){
    const giac::context * contextptr = get_context(this);
    if (!c || c>=0x80)
      return false;
    if (!act || need_active_parse){
      if (c=='(' || c==')'){ // if selection replace vector by sequences
	if (!act){
	  insure_something_selected();
	  gen g=get_selection();
	  if (g.is_symb_of_sommet(at_makevector))
	    replace_selection(symbolic(at_makesuite,g._SYMBptr->feuille));
	  else {
	    if (g.type==_VECT && g.subtype!=_SEQ__VECT){
	      g.subtype=_SEQ__VECT;
	      replace_selection(g);
	    }
	    else
	      replace_selection(symbolic(at_of,makevecteur(f__IDNT_e,g)));
	  }
	  return true;
	}
      }
      if (c=='[' || c==']'){ // if selection replace seq by vector
	if (!act){
	  insure_something_selected();
	  gen g=get_selection();
	  if (g.is_symb_of_sommet(at_makesuite))
	    replace_selection(symbolic(at_makevector,g._SYMBptr->feuille));
	  else {
	    if (g.type==_VECT && g.subtype==_SEQ__VECT){
	      g.subtype=0;
	      replace_selection(g);
	    }
	    else {
	      g.subtype=_SEQ__VECT;
	      replace_selection(symbolic(at_at,makevecteur(m__IDNT_e,g)));
	    }
	  }
	  return true;
	}
      }
      if (!act && (c=='+' || c=='-' || c=='*' || c=='/' || c=='^' || c==',' || c=='<' || c=='>' || c=='=') ){
	insure_something_selected();
	gen g=get_selection();
	gen tmp;
	if (c==','){
	  if (g.type==_VECT && g.subtype==_SEQ__VECT){
	    vecteur v=*g._VECTptr;
	    v.push_back(zero);
	    replace_selection(gen(v,g.subtype));
	    select_down(0);
	    select_left(0);
	    return false;
	  }
	  gen oldg=g;
	  select_up(0);
	  g=get_selection();
	  if (g.type==_VECT && oldg!=g){
	    vecteur v=*g._VECTptr;
	    v.push_back(zero);
	    replace_selection(gen(v,g.subtype));
	    select_down(0);
	    select_left(0);
	    return false;
	  }
	  if (g.type==_SYMB && g._SYMBptr->feuille.type==_VECT){
	    bool inside=g.is_symb_of_sommet(at_of) || g.is_symb_of_sommet(at_at);
	    int sub=0;
	    vecteur v;
	    if (inside){
	      if (g._SYMBptr->feuille._VECTptr->back().type==_VECT){
		v=*g._SYMBptr->feuille._VECTptr->back()._VECTptr;
		sub=g._SYMBptr->feuille._VECTptr->back().subtype;
	      }
	      else {
		v=makevecteur(g._SYMBptr->feuille._VECTptr->back());
		sub=_SEQ__VECT;
	      }
	    }
	    else
	      v=*g._SYMBptr->feuille._VECTptr;
	    v.push_back(zero);
	    if (g._SYMBptr->sommet==at_integrate && v.size()==3)
	      v.push_back(plus_one);
	    gen tmp;
	    if (inside)
	      tmp=symbolic(g._SYMBptr->sommet,gen(makevecteur(g._SYMBptr->feuille._VECTptr->front(),gen(v,sub)),g._SYMBptr->feuille.subtype));
	    else
	      tmp=symbolic(g._SYMBptr->sommet,gen(v,g._SYMBptr->feuille.subtype));
	    replace_selection(tmp);
	    select_down(0);
	    if (inside){
	      select_right(0);
	      select_down(0);
	      select_left(0);
	    }
	    return false;
	  }
	  if (g.type==_SYMB){
	    vecteur v(gen2vecteur(g._SYMBptr->feuille));
	    if (v.size()==1 && (g._SYMBptr->sommet==at_integrate || g._SYMBptr->sommet==at_sum || g._SYMBptr->sommet==at_limit || g._SYMBptr->sommet==at_derive) )
	      v.push_back(vx_var);
	    v.push_back(zero);
	    if (g._SYMBptr->sommet==at_integrate || g._SYMBptr->sommet==at_sum)
	      v.push_back(plus_one);
	    replace_selection(symbolic(g._SYMBptr->sommet,gen(v,_SEQ__VECT)));
	    return false;
	  }
	  tmp=at_makesuite;
	} // end if c==','
	else
	  tmp=gen(string("'")+char(c)+string("'"),contextptr);
	if (tmp.type!=_FUNC)
	  setsizeerr();
	unary_function_ptr f=*tmp._FUNCptr;
	if ( g.is_symb_of_sommet(at_makesuite) && ( (c!='/' && c!='^') || (g._SYMBptr->feuille.type==_VECT && g._SYMBptr->feuille._VECTptr->size()==2) ) ){
	  replace_selection(symbolic(f,g._SYMBptr->feuille));
	  return true;
	}
	else {
	  tmp=gen(makevecteur(g,0),_SEQ__VECT);
	  // tmp=gen(makevecteur(g,Equation_nullstring()),_SEQ__VECT);
	  replace_down_left_activate(symbolic(f,tmp));
	  return false;
	}
      }
    } // end not active or active_parse
    if (act){
      // *act is active and is a terminal of type string
      string s=*act->_EQWptr->g._STRNGptr;
      int ss=s.size();
      active_pos=max(min(active_pos,ss),0);
      vecteur * vptr;
      int pos;
      switch (c){
      case 0x0b:
	if (active_pos>0)
	  --active_pos;
	else {
	  pos=in_multistring(data,vptr);
	  if (pos>0){ // back one line and active_pos maximal
	    Equation_select(*act,false,true);
	    --pos;
	    Equation_select((*vptr)[pos],true,true);
	    active_pos=RAND_MAX;
	  }
	}
	return 1;
      case 0x0c:
	if (active_pos<ss)
	  ++active_pos;
	else {
	  pos=in_multistring(data,vptr);
	  if (pos>=0 && pos<int(vptr->size()-1)){ // forward one line and active_pos maximal
	    Equation_select(*act,false,true);
	    ++pos;
	    Equation_select((*vptr)[pos],true,true);
	    active_pos=0;
	  }
	}
	return 1;
      case 0x7f:
	if (active_pos>0){
	  s=s.substr(0,active_pos-1)+s.substr(active_pos,ss-active_pos);
	  --active_pos;
	}
	else {
	  pos=in_multistring(data,vptr);
	  if (pos==-1)
	    return 1;
	  if (pos>0){
	    handle_backspace(this,pos,vptr);
	    return 1;
	  }
	  else {
	    if (ss==0){
	      act->_EQWptr->active=false;
	      act->_EQWptr->selected=true;
	      begin_sel=-1;
	      end_sel=-1;
	      remove_selection();
	      return false;
	    }
	  }
	}
	break;
      default:
	s = s.substr(0,active_pos)+char(c)+s.substr(active_pos,ss-active_pos);
	++active_pos;
      }
      replace_selection(string2gen(s,false),1);
      return false;
    }
    // nothing active
    bool has_selection=false;
    gen g=Equation_selected2gen(this,data,has_selection);
    if (has_selection){
      string tmp(1,c);
      gen tmps=string2gen(tmp,false);
      tmps.subtype=1;
      if (replace_selection(tmps)){
	// if nothing active, activate
	attributs a(0,0,0);
	vecteur position;
	gen * ptr =Equation_selected(data,a,w(),position,1,contextptr);
	if (!ptr){
	  deselect_and_activate();
	  active_pos=1;
	}
	if (in_multistring(data)==-1 && g.type!=_STRNG)
	  need_active_parse=true;
      }
      return false;
    }
    // nothing selected
    select_up(0);
    return true;
  }

  /* not used // return true if conversion needed and done
  bool string2vector(const string & s,vector<string> & v){
    int pos=s.find('\n'),newpos;
    int ss=s.size();
    if (pos<0 || pos >= ss)
      return false;
    v.clear();
    v.push_back(s.substr(0,pos));
    for (;pos<ss;){
      newpos=s.find('\n',pos);
      if (newpos<0 || newpos >= ss){
	v.push_back(s.substr(pos,ss-pos));
	return true;
      }
      v.push_back(s.substr(pos,newpos-pos));
      pos=newpos;
    }
  }
  */

  bool Equation::handle_text(const string & paste_s_orig,gen * act){
    const giac::context * contextptr = get_context(this);
    string paste_s(paste_s_orig);
    int ps=paste_s.size();
    if (ps>2 && paste_s[0]=='"' && paste_s[ps-1]=='"'){
      ps = ps-2; 
      paste_s=paste_s.substr(1,ps);
    }
    int nl=paste_s.find('\n');
    bool needmulti=(nl>=0 && nl<ps);
    if (act){ // insert s
      string s;
      if (act->type!=_EQW || act->_EQWptr->g.type!=_STRNG){
	cerr << "Error " << *act << endl;
	return false;
      }
      s=*act->_EQWptr->g._STRNGptr;
      int pos=giacmax(giacmin(active_pos,s.size()),0);
      active_pos=pos;
      int toend=s.size()-pos;
      s=s.substr(0,active_pos)+paste_s+s.substr(active_pos,toend);
      gen g(s,contextptr);
      if (giac::first_error_line(contextptr))
	g=string2gen(s,false);
      if (!needmulti){
	if (replace_selection(g,1))
	  active_pos = pos+paste_s.size();
	return false;
      }
      vecteur * vptr;
      int n=in_multistring(data,vptr);
      if (n==-1){
	if (need_active_parse)
	  g=symbolic(at_expr,g);
	desactivate_and_select();
	if (replace_selection(g)){
	  if (need_active_parse)
	    select_down(0);
	  select_down(0);
	  select_left(0); 
	  pos = get_selection()._STRNGptr->size()-toend;
	  deselect_and_activate();
	  active_pos = max(pos,0);
	  need_active_parse=false;
	}
	return false;
      }
      // insert in current multistring
      s="";
      const_iterateur it=vptr->begin(),itend=vptr->end()-1;
      for (int i=0;i<n;++i,++it){
	if (it->type==_EQW && it->_EQWptr->g.type==_STRNG)
	  s += *it->_EQWptr->g._STRNGptr + '\n';
      }
      string ss=*act->_EQWptr->g._STRNGptr;
      s += ss.substr(0,active_pos)+paste_s+ss.substr(active_pos,ss.size()-active_pos);
      for (++it;it!=itend;++it){
	if (it->type==_EQW && it->_EQWptr->g.type==_STRNG)
	  s += '\n'+*it->_EQWptr->g._STRNGptr;
      }
      g=gen(s,contextptr);
      if (giac::first_error_line(contextptr))
	g=string2gen(s,false);
      desactivate_and_select();
      // count newline in paste_s, add to n, activate this line, cursor
      // at end of this line - the value of toend
      for (int i=0;i<ps;++i){
	if (paste_s[i]=='\n')
	  ++n;
      }
      select_up(0);
      vecteur position;
      attributs tmpattr(0,0,0);
      gen * ptr = Equation_selected(data,tmpattr,w(),position,false,contextptr);
      eqwdata olde=Equation_total_size(*ptr);
      * ptr = Equation_compute_size(g,olde.eqw_attributs,w(),contextptr);
      Equation_select(data,false);
      Equation_select((*ptr->_VECTptr)[n],true);
      g=get_selection();
      if (g.type==_STRNG){
	pos = g._STRNGptr->size();
	deselect_and_activate();
	need_active_parse = false;
	if (replace_selection(g,1))
	  active_pos = max(pos- toend,0) ;
	need_active_parse = false;
      }
      return false;
    }
    // remove selection and replace
    gen g(paste_s,contextptr);
    if (giac::first_error_line(contextptr))
      g=string2gen(paste_s,false);
    if (needmulti)
      g=symbolic(at_expr,g);
    replace_selection(g);
    return true;
  }

  bool Equation::handle_text(const string & paste_s_orig){
    vecteur position;
    const giac::context * contextptr = get_context(this);
    gen * act=Equation_selected(data,attr,w(),position,1,contextptr);
    return handle_text(paste_s_orig,act);
  }

  // activate at x,y, return cursor position or -1, or -3 for fl_widget_pointer
  int Equation_set_active(int x,int y,gen & g,Equation * eq,bool inmultistring,gen * & activecellptr){
    const giac::context * contextptr = get_context(eq);
    activecellptr=0;
    if (g.type==_EQW){
      // x,y must be in the box
      eqwdata & e=*g._EQWptr;
      if (e.x>x || e.x+e.dx<x || e.y>y || e.y+e.dy<y)
	return -1; 
      activecellptr=&e.g;
      if (e.g.type==_POINTER_){
	return -3;
      }
      e.active=true;
      if (e.g.type==_STRNG)
	eq->need_active_parse=!inmultistring && e.g._STRNGptr->empty();
      else {
	eq->need_active_parse=true;
	e.g=string2gen(e.g.print(contextptr),false);
	e.g.subtype=1;
      }
      // binary search for active_pos
      return Equation_binary_search_pos(e,x,y);
    }
    if (g.type!=_VECT)
      return -1;
    vecteur & v =*g._VECTptr;
    if (v.empty())
      return -1;
    iterateur it=v.begin(),itend=v.end()-1;
    if (!inmultistring)
      inmultistring = itend->type==_EQW && itend->_EQWptr->g==at_multistring;
    int pos;
    for (;it!=itend;++it){
      pos=Equation_set_active(x,y,*it,eq,inmultistring,activecellptr);
      if (pos!=-1){
	return pos;
      }
    }
    if (it->type==_EQW){ // change operator
      eqwdata & e=*it->_EQWptr;
      if (e.x>x || e.x+e.dx<x || e.y>y || e.y+e.dy<y)
	return -1;
      if ((eq && !eq->output_equation) && e.g.type==_FUNC && !e.g._FUNCptr->ptr()->printsommet && e.g!=at_makesuite && e.g!=at_makelist && e.g!=at_makevector ){
	eqwdata f(e);
	f.g=string2gen(e.g._FUNCptr->ptr()->s,false);
	int pos=Equation_binary_search_pos(f,x,y);
	string ans,debut=f.g._STRNGptr->substr(0,pos);
	if (handle_tab(debut,*giac::vector_completions_ptr(),eq->h()/2,eq->w()/2,pos,ans)){
	  gen tmp(ans,contextptr);
	  if (tmp.type==_SYMB){
	    e.g=tmp._SYMBptr->sommet;
	    return -2;
	  }
	  tmp=gen("'"+ans+"'",contextptr);
	  if (tmp.type==_FUNC){
	    e.g=tmp;
	    return -2;
	  }
	}
      }
    }
    return -1;
  }
  gen * Equation::set_active(int x,int y){
    const giac::context * contextptr = get_context(this);
    deselect();
    gen * gptr;
    int pos=Equation_set_active(x,y,data,this,false,gptr);
    if (pos==-3)
      return gptr;
    if (pos==-2){
      data=Equation_compute_size(get_data(),attr,w(),contextptr);
      select();
      redraw();
      return 0;
    }
    if (pos==-1)
      Equation_select(data,false,true);
    else
      active_pos=pos;
    return 0;
  }

  // Return 0 if the mouse cursor is not on a _POINTER_ object,
  // returns a pointer to the gen of type _POINTER_ otherwise
  gen * Equation_is_pointer_active(Equation * eq,const gen & g,int x,int y){
    if (g.type==_EQW){
      // x,y must be in the box
      eqwdata & e=*g._EQWptr;
      if (e.x>x || e.x+e.dx<x || e.y>y || e.y+e.dy<y)
	return 0; 
      if (e.g.type==_POINTER_)
	return &e.g;
      return 0;
    }
    if (g.type!=_VECT)
      return 0;
    vecteur & v =*g._VECTptr;
    if (v.empty())
      return 0;
    iterateur it=v.begin(),itend=v.end()-1;
    gen * pos;
    for (;it!=itend;++it){
      pos=Equation_is_pointer_active(eq,*it,x,y);
      if (pos)
	return pos;
    }
    return 0;
  }

  // for multistring Up and Down should not select
  void Equation_multistring_activate(Equation * eq,int direction,int pos){
    gen save_data=Equation_copy(eq->data);
    if (direction)
      eq->select_down(1);
    else
      eq->select_up(1);
    if (eq->get_selection().type==_STRNG){
      eq->deselect_and_activate();
      eq->active_pos=max(pos,0);
    }
    else {
      eq->data=save_data;
      eq->adjust_xy_sel();
    }
  }

  // return -1 if not in a multistring, position otherwise
  int in_multistring(const gen & data,vecteur * & vptr){
    if (data.type!=_VECT)
      return -1;
    vecteur & v =*data._VECTptr;
    iterateur it=v.begin(),itend=v.end();
    if (itend==it)
      return -1;
    --itend;
    if (itend->type==_EQW && itend->_EQWptr->g==at_multistring){
      for (;it!=itend;++it){
	if (it->type==_EQW && it->_EQWptr->active){
	  vptr = &v;
	  return it-v.begin();
	}
      }
      return -1;
    }
    for (;it!=itend;++it){
      int i=in_multistring(*it,vptr);
      if (i!=-1)
	return i;
    }
    return -1;
  }

  // return -1 if not in a multistring, position otherwise
  int in_multistring(const gen & data){
    vecteur * vptr;
    return in_multistring(data,vptr);
  }

  void handle_newline(Equation * eqwptr){
    const giac::context * contextptr = get_context(eqwptr);
    vecteur * vptr;
    int n=in_multistring(eqwptr->data,vptr);
    if (n!=-1){
      // add a new line here
      string s( *(*vptr)[n]._EQWptr->g._STRNGptr);
      int ss=s.size(),pos=max(min(eqwptr->active_pos,ss),0);
      string s1=s.substr(0,pos);
      string s2=s.substr(pos,ss-pos);
      (*vptr)[n]=Equation_compute_size(string2gen(s1,false),eqwptr->attr,0,contextptr);
      iterateur it=vptr->begin()+(n+1);
      vptr->insert(it,Equation_compute_size(string2gen(s2,false),eqwptr->attr,0,contextptr));
      Equation_select((*vptr)[n],false,true); // desactivate line n
      Equation_select((*vptr)[n+1],true,true); // activate line n+1
      eqwptr->active_pos=0;
      eqwptr->replace_selection(string2gen(s2,false),1);
    }
    else {
      // not in a multistring, transform current in a multistring
      bool parse=eqwptr->need_active_parse;
      eqwptr->need_active_parse=false;
      eqwptr->desactivate_and_select();
      gen g=eqwptr->get_selection();
      if (g.type==_STRNG)       // FIXME: should add newline at active_pos
	g=string2gen(*g._STRNGptr+'\n',false);
      bool res;
      if (parse){
	g=symbolic(at_expr,g);
	res=eqwptr->replace_selection(g);
	eqwptr->select_down(0);
      }
      else {
	res=eqwptr->replace_selection(g);
      }
      if (res){
	eqwptr->select_down(0);
	eqwptr->select_right(0);
	eqwptr->deselect_and_activate();
	eqwptr->need_active_parse=false;
      }
    }
  }

  bool switch_expr(Equation * eqwptr){
    gen g=eqwptr->get_selection();
    if (g.type==_STRNG){
      eqwptr->replace_selection(symbolic(at_expr,g));
      return true;
    }
    if (g.is_symb_of_sommet(at_expr)){
      eqwptr->replace_selection(g._SYMBptr->feuille);
      return true;
    }
    return false;
  }

  int Equation::handle(int event){
    if (event==FL_MOUSEWHEEL){
      eqwdata e=Equation_total_size(data);
      if (Fl::event_inside(this) && vscroll && h()<e.dy){
	int n=Fl::e_dy;
	int v=vscroll->value();
	if (n<0 && v==0)
	  return 0;
	if (n>0 && v>e.dy-h()+1)
	  return 0;
	return vscroll->handle(event);
      }
      else
	return 0;
    }
    xcas::History_Pack * hp=get_history_pack(this);
    giac::context * cptr=hp?hp->contextptr:0;
    if (is_context_busy(cptr)){
      thread_param * ptr= thread_param_ptr(cptr);
      if (ptr && ptr->f_param==this) // handle is locked during eval
	return 1;
    }
    int i=Fl_Group::handle(event);
    if (i)
      return i;
    i=in_handle(event);
    if (i==-1)
      return 1;
    //    if (i)
    //  Fl::focus(this);
    return i;
  }

  int Equation::in_handle(int event){
    const giac::context * contextptr = get_context(this);
    if (event==FL_FOCUS || event==FL_PUSH){
      redraw();
      Xcas_input_focus=this;
      Fl::focus(this);
      if (event==FL_FOCUS)
	return 1;
    }
    if (event==FL_KEYBOARD)
      Xcas_input_focus=this;
    bool is_history=(data.type==_VECT && data.subtype==_HIST__VECT);
    bool complete_level_selected=false;
    eqwdata data_eqwdata=Equation_total_size(data);
    if (is_history){ // check if a complete level is selected
      const_iterateur it=data._VECTptr->begin(),itend=data._VECTptr->end();
      for (;it!=itend;++it){
	eqwdata e=Equation_total_size(*it);
	if (e.selected){
	  complete_level_selected=true;
	  break;
	}
      }
    }
    attributs attr;
    vecteur position;
    gen f,* act=Equation_selected(data,attr,w(),position,1,contextptr);
    char ch;
    if (act && act->type==_EQW && act->_EQWptr->g.type==_STRNG)
      ;
    else
      act=0;
    if (event==FL_RELEASE && Fl::event_button()== FL_MIDDLE_MOUSE ){
      int x,y;
      x=Fl::event_x()-this->x()+xleft;
      y=Fl::event_y()-this->y();
      deselect();
      gen * gptr=set_active(x,ytop-y);
      if (gptr && gptr->type==_POINTER_)
	// should call the handle method of the pointer
	return 1;
      Fl::paste(*this);
      return 1;
    }
    if (event==FL_PASTE) {
      handle_text(Fl::event_text(),act);
      return 1;
    }
    if (event==FL_KEYBOARD){
      /* if (Fl::event_state()== FL_CTRL || Fl::event_state()== FL_ALT)
	 return 0; */
      bool sel_changed=true;
      bool shifton= (Fl::event_state()== FL_SHIFT );
      int mode=Fl::event_state(FL_CTRL);
      if (mode)
	mode=2;
      else 
	mode=shifton?0:1;
      int save_active_pos=active_pos;
      switch (Fl::event_key()){
      case FL_Left:
	if (act)
	  handle_key(0x0b,act);
	else {
	  if (!parse_desactivate())
	    select_left(mode); // it was shift_on
	}
	break;
      case FL_Right:
	if (act)
	  handle_key(0x0c,act);
	else {
	  if (!parse_desactivate())
	    select_right(mode);
	}
	break;
      case FL_Up:
	if (shifton && complete_level_selected){
	  select_left(0);
	  select_left(0);
	}
	else {
	  if (!parse_desactivate())
	    select_up(!shifton);
	  else 
	    if (!shifton)
	      Equation_multistring_activate(this,0,save_active_pos);
	}
	break;
      case FL_Down:
	if (shifton && complete_level_selected){
	  select_right(0);
	  select_right(0);
	}
	else {
	  if (!parse_desactivate())
	    select_down(!shifton);
	  else 
	    if (!shifton)
	      Equation_multistring_activate(this,1,save_active_pos);
	}
	break;
      case FL_Escape: 
	if (cb_escape)
	  cb_escape(this);
	else
	  parse_desactivate();
	redraw();
	return 1;
      case FL_Shift_L: case FL_Shift_R: 
      case FL_Caps_Lock: 
      case FL_Meta_L: case FL_Meta_R: case FL_Menu: case FL_Num_Lock:
      case FL_Insert: 
	return 1;
      case FL_Control_R: case FL_Control_L: case FL_Alt_L: case FL_Alt_R: 
	return 0;
      case FL_BackSpace:
      case FL_Delete: 
	if (is_history && complete_level_selected && cb_backspace){
	  cb_backspace(this);
	  return 1;
	}
	if (modifiable){
	  if (act){
	    handle_key(0x7f,act);
	    return 1;
	  }
	  else {
	    f=get_selection();
	    if (f.type==_SYMB){
	      f=f._SYMBptr->feuille;
	      if (f.type==_VECT && !f.subtype && !ckmatrix(f))
		f.subtype=_SEQ__VECT;
	      replace_selection(f);
	    }
	    else
	      remove_selection();
	  }
	}
	break;
      case FL_Enter: case FL_KP_Enter:
	/*
	if (old_upper_window_state==upper_window_mtrw){
	  manage_upper_window(upper_window_mtrw,false);
	}
	else {	  
	  manage_upper_window(upper_window_hist,false);
	}
	input_value(eqw_data2gen(data).print(contextptr).c_str());
	*/
	if (shifton)
	  handle_newline(this);
	else {
	  if (cb_enter)
	    cb_enter(this,0);
	  else
	    parse_desactivate();
	}
	redraw();
	return 1;
      case FL_Home: 
	xleft=data_eqwdata.x;
	sel_changed=false;
	break;
      case FL_End: 
	xleft=data_eqwdata.x+data_eqwdata.dx-w();
	sel_changed=false;
	break;
      case FL_Page_Up:
	ytop -= h();
	sel_changed=false;
	break;
      case FL_Page_Down: 
	ytop += h();
	sel_changed=false;
	break;
      default:
	if (modifiable){
	  ch=Fl::event_text()?Fl::event_text()[0]:0;
	  int es=Fl::event_state();
	  if (es== FL_ALT || es==1572864){ 
	    return 0;
	    // ?? 
	    if (ch=='v'){
	      gen e;
	      select_user_var(100,200,e,0);
	      if (output_equation){
		if (!parse_desactivate())
		  insure_something_selected(true);
		eval_function(e);
	      }
	      return 1;
	    }
	  }
	  if (ch==1){ // Ctrl-A select all
	    select();
	    fl_select();
	    return 1;
	  }
	  if (ch==3) { // Ctrl-C
	    static string s;
	    s=get_selection().print(contextptr);
	    Fl::copy(s.c_str(),s.size(),1);
	    return 1;
	  }
	  if (ch==18){ // Ctrl-R
	    in_Xcas_input_1arg(this,"integrate");
	    return 1;
	  }
	  if (ch==19){ // Ctrl-S
	    if (!parse_desactivate())
	      insure_something_selected(true);
	    eval_function(at_simplify);
	    return 1;
	  }
	  if (ch==4){ // Ctrl-D
	    in_Xcas_input_1arg(this,"diff");
	    return 1;
	  }
	  if (ch==5){ // Ctrl-E
	    if (!parse_desactivate())
	      insure_something_selected(true);
	    eval_function(at_eval);
	    return 1;
	  }
	  if (ch==6){ // Ctrl-F
	    if (!parse_desactivate())
	      insure_something_selected(true);
	    eval_function(at_factor);
	    return 1;
	  }
	  if (ch==12){ // Ctrl-L
	    in_Xcas_input_1arg(this,"limit");
	    return 1;
	  }
	  if (ch==14){ // Ctrl-N
	    if (!parse_desactivate())
	      insure_something_selected(true);
	    eval_function(at_normal);
	    return 1;
	  }
	  if (ch==16){ // Ctrl-P
	    if (!parse_desactivate())
	      insure_something_selected(true);
	    eval_function(at_partfrac);
	    return 1;
	  }
	  if (ch>=17 && ch<=20) // Ctrl-Q T
	    return 0;
	  if (ch==22){ // Ctrl-V
	    Fl::paste(*this,1);
	    return 1;
	  }
	  if (ch==26){ // Ctrl-Z
	    rcl_data(-1);
	    adjust_widget_size();
	    return 1;
	  }
	  if (ch==25){ // Ctrl-Y
	    rcl_data(1);
	    adjust_widget_size();
	    return 1;
	  }
	  if (ch==9){ // tab, ctrl-I
	    string s,ans;
	    if (act){
	      s=*act->_EQWptr->g._STRNGptr;
	      int ss=s.size();
	      active_pos=max(min(active_pos,ss),0);
	      string fin=s.substr(active_pos,s.size()-active_pos);
	      s=s.substr(0,active_pos);
	      int remove=active_pos;
	      if (handle_tab(s,(*vector_completions_ptr()),h()/2,w()/2,remove,ans)){
		int l=s.size()-remove;
		if (l<0)
		  l=0;
		ans=s.substr(0,l)+ans;
		replace_selection(string2gen(ans+fin,false),1);
		active_pos=ans.size();
		return 1;
	      }
	    }
	    else {
	      insure_something_selected();
	      gen gs=get_selection();
	      s=gs.print(contextptr);
	      int pos=s.find('(');
	      if (pos>0 && pos<int(s.size()))
		s=s.substr(0,pos);
	      int remove;
	      if (gs.type==_SYMB && handle_tab(s,(*vector_completions_ptr()),h()/2,w()/2,remove,ans)){
		// FIXME remove remove chars
		gen gf=gen("'"+s+ans+"'",contextptr);
		if (gf.type==_FUNC)
		  replace_selection(symbolic(*gf._FUNCptr,gs._SYMBptr->feuille));
	      }
	    }
	    return 1;
	  }
	  if (ch=='"'){
	    if (act &&  in_multistring(data)==-1){
	      // change mode, ! in multistring
	      need_active_parse=!need_active_parse;
	      redraw();
	      return 1;
	    }
	    if (switch_expr(this)){
	      redraw();
	      return 1;
	    }
	    else {
	      gen g=get_selection();
	      replace_selection(string2gen(g.print(),false));
	      deselect_and_activate();
	      need_active_parse=true;
	      return 1;
	    }
	  }
	  sel_changed=handle_key(ch,act);
	} // end if (modifiable)
      }
      if (sel_changed){
	fl_select();
	setscroll();
      }
      return 1;
    } // end event==FL_KEYBOARD
    int x,y;
    x=Fl::event_x() - this->x() +xleft;
    y=Fl::event_y() - this->y();
    if (event==FL_PUSH ){
      if ( (vscroll->visible() && (Fl::event_x() - this->x()>w()-vscroll->w()) ) 
	   || (hscroll->visible() && (y>h()-hscroll->h())) ){
	return 0;
      }
      if (Fl::event_button()!= FL_MIDDLE_MOUSE)
	// desactivate active cell if any
	parse_desactivate();
      xsel=x;
      ysel=ytop-y;
      xcur=xsel;
      ycur=ysel;
      if (Fl::event_button()!= FL_MIDDLE_MOUSE)
	deselect();
      redraw();
      gen * gptr=Equation_is_pointer_active(this,data,x,ytop-y);
      if (gptr && gptr->type==_POINTER_ && gptr->subtype==_FL_WIDGET_POINTER){
	// call the handle method of the pointer
	Fl_Widget * flptr=(Fl_Widget *) gptr->_POINTER_val;
	flptr->handle(event);
	Fl::focus(this);
	return 1;
      }
      return 1;
    }
    if (event==FL_DRAG){
      if (Fl::event_button()!= FL_MIDDLE_MOUSE)
	deselect();
      redraw();
      if ( (absint(xsel-x)<=2) && (absint(ysel-(ytop-y))<2) ){
	redraw();
	return 1;
      }
      gen * gptr=Equation_is_pointer_active(this,data,x,ytop-y);
      if (gptr && gptr->type==_POINTER_ && gptr->subtype==_FL_WIDGET_POINTER){
	// call the handle method of the pointer
	Fl_Widget * flptr=(Fl_Widget *) gptr->_POINTER_val;
	flptr->handle(event);
	Fl::focus(this);
	return 1;
      }
      if (Fl::event_button()!= FL_MIDDLE_MOUSE)
	select_rectangle(x,ytop-y);
      return 1;
    }
    if (event==FL_RELEASE){
      Fl::focus(this);
      deselect();
      redraw();
      gen * gptr=Equation_is_pointer_active(this,data,x,ytop-y);
      if (gptr && gptr->type==_POINTER_ && gptr->subtype==_FL_WIDGET_POINTER){
	// call the handle method of the pointer
	Fl_Widget * flptr=(Fl_Widget *) gptr->_POINTER_val;
	flptr->handle(event);
	Fl::focus(this);
	return 1;
      }
      if ( (absint(xsel-x)<=2) && (absint(ysel-(ytop-y))<2) ){
	if (Fl::event_clicks()){ // double-click
	  // x=xsel+10;
	  // y=ytop-ysel+4;
	  desactivate_and_select();
	  //gen g=get_selection();
	  select_rectangle(x,ytop-y);
	}
	else
	  gen * gptr=set_active(x,ytop-y);
	return 1;
      }
      select_rectangle(x,ytop-y);
      if (Fl::event_button()!= FL_MIDDLE_MOUSE)
	fl_select();
      return 1;
    }
    return 0;
  }

  static void cb_Equation_Select(Fl_Menu_* m , void*) {
    if (m){
      Equation * eq = dynamic_cast<Equation *>(m->parent());
      const giac::context * contextptr = get_context(eq);
      if (eq){
	Fl::focus(eq);
	Xcas_input_focus=eq;
	string s =eq->get_data().print(contextptr);
	Fl::selection(*eq,s.c_str(),s.size());
	eq->select();
	eq->redraw();
      }
    }
  }

  void Equation_eval_callback(const giac::gen & evaled_g,void * param){
    Fl_Widget * wid=static_cast<Fl_Widget *>(param);
    if (!wid)
      return;
    Equation * eq = dynamic_cast<Equation *>(wid);
    if (!eq)
      return;
    Xcas_input_focus=eq;
    eq->replace_selection_wo_save(evaled_g);
  }

  void cb_Equation_Function(Fl_Menu_* m ,const unary_function_ptr * u) {
    if (m){
      Equation * eq = dynamic_cast<Equation *>(m->parent());
      if (eq){
	Fl::focus(eq);
	Xcas_input_focus=eq;
	eq->save_data();
	xcas::History_Pack * hp=get_history_pack(eq);
	giac::context * cptr=hp?hp->contextptr:0;
	make_thread(symbolic(u,eq->get_selection()),eval_level(cptr),Equation_eval_callback,eq,cptr);
      }
    }
  }

  static void cb_Equation_Evalf(Fl_Menu_* m , void*) {
    cb_Equation_Function(m,at_evalf);
  }

  static void cb_Equation_Eval(Fl_Menu_* m , void*) {
    cb_Equation_Function(m,at_eval);
  }

  static void cb_Equation_Exact(Fl_Menu_* m , void*) {
    cb_Equation_Function(m,at_exact);
  }

  static void cb_Equation_Simplify(Fl_Menu_* m , void*) {
    cb_Equation_Function(m,at_simplify);
  }


  static void cb_Equation_Editselection(Fl_Menu_* m , void*) {
    if (m){
      Equation * eq = dynamic_cast<Equation *>(m->parent());
      if (eq){
	Fl::focus(eq);
	Xcas_input_focus=eq;
	eq->save_data();
	xcas::History_Pack * hp=get_history_pack(eq);
	giac::context * cptr=hp?hp->contextptr:0;
	gen g=eq->get_selection();
	eq->replace_selection(string2gen(g.print(),false));
	eq->deselect_and_activate();
	eq->need_active_parse=true;
      }
    }
  }

  static void cb_Equation_Normal(Fl_Menu_* m , void*) {
    cb_Equation_Function(m,at_normal);
  }

  static void cb_Equation_Factor(Fl_Menu_* m , void*) {
    cb_Equation_Function(m,at_factor);
  }

  static void cb_Equation_Back(Fl_Menu_* m , void*) {
    if (m){
      Equation * eq = dynamic_cast<Equation *>(m->parent());
      if (eq){
	Xcas_input_focus=eq;
	eq->rcl_data(-1);
      }
    }
  }

  static void cb_Equation_Forward(Fl_Menu_* m , void*) {
    if (m){
      Equation * eq = dynamic_cast<Equation *>(m->parent());
      if (eq){
	Xcas_input_focus=eq;
	eq->rcl_data(1);
      }
    }
  }

  Fl_Menu_Item Equation_menu[] = {
    {gettext("M"), 0,  0, 0, 64, 0, 0, 14, 56},
    {gettext("Select all"), 0,  (Fl_Callback*)cb_Equation_Select, 0, 0, 0, 0, 14, 56},
    {gettext("Edit selection"), 0,  (Fl_Callback*)cb_Equation_Editselection, 0, 0, 0, 0, 14, 56},
    {gettext("simplify"), 0,  (Fl_Callback*)cb_Equation_Simplify, 0, 0, 0, 0, 14, 56},
    {gettext("normal"), 0,  (Fl_Callback*)cb_Equation_Normal, 0, 0, 0, 0, 14, 56},
    {gettext("factor"), 0,  (Fl_Callback*)cb_Equation_Factor, 0, 0, 0, 0, 14, 56},
    {gettext("approx"), 0,  (Fl_Callback*)cb_Equation_Evalf, 0, 0, 0, 0, 14, 56},
    {gettext("exact"), 0,  (Fl_Callback*)cb_Equation_Exact, 0, 0, 0, 0, 14, 56},
    {gettext("eval"), 0,  (Fl_Callback*)cb_Equation_Eval, 0, 0, 0, 0, 14, 56},
    {gettext("Back"), 0,  (Fl_Callback*)cb_Equation_Back, 0, 0, 0, 0, 14, 56},
    {gettext("Forward"), 0,  (Fl_Callback*)cb_Equation_Forward, 0, 0, 0, 0, 14, 56},
    {0},
    {0}
  };

  void Equation::add_scroll_menu(){
    int largeur=labelsize();
    vscroll=new Equation_Scrollbar(x()+Fl_Group::w()-largeur,y(),largeur,Fl_Group::h()-labelsize(),this,true);
    vscroll->labelsize(largeur);
    hscroll=new Equation_Scrollbar(x(),y()+Fl_Group::h()-labelsize(),Fl_Group::w()-2*largeur,labelsize(),this,false);
    hscroll->labelsize(largeur);
    Fl_Group::add(vscroll);
    Fl_Group::add(hscroll);
    menubar = new Fl_Menu_Bar(hscroll->x()+hscroll->w(),hscroll->y(),this->w()-hscroll->w(),hscroll->h());
    int s= Equation_menu->size();
    Fl_Menu_Item * menuitem = new Fl_Menu_Item[s];
    for (int i=0;i<s;++i)
      *(menuitem+i)=*(Equation_menu+i);
    menubar->menu (menuitem);
    menubar->tooltip(gettext("Equation menu"));    
    Fl_Group::add(menubar);
    setscroll();
  }

  Equation::Equation(int x, int y, int w, int h): Fl_Group(x, y, max(w,20), max(h,20)){
    const giac::context * contextptr = get_context(this);
    xleft=0;
    ytop=h;
    xcur=0;
    ycur=0;
    begin_sel=-1;
    end_sel=-1;
    cb_escape=0;
    cb_enter=0;
    cb_backspace=0;
    cb_select=0;
    undo_history.clear();
    undo_history_pos=0;
    attr=attributs(14,Xcas_equation_background_color,Xcas_equation_color);
    labelsize(14);
    modifiable=true;
    output_equation=true;
    data=Equation_nullstring(attr,0,contextptr);
    Fl_Group::end();
    add_scroll_menu();
  }

  Equation::Equation(int x, int y, int w, int h, const char* l,const gen & g): Fl_Group(x, y, max(w,20), max(h,20), l){
    const giac::context * contextptr = get_context(this);
    xleft=0;
    ytop=h;
    xcur=0;
    ycur=0;
    begin_sel=-1;
    end_sel=-1;
    cb_escape=0;
    cb_enter=0;
    cb_backspace=0;
    cb_select=0;
    undo_history.clear();
    undo_history_pos=0;
    attr=attributs(14,Xcas_equation_background_color,Xcas_equation_color);
    labelsize(14);
    modifiable=true;
    output_equation=true;
    if (taille(g,max_prettyprint_equation)<max_prettyprint_equation)
      data=Equation_compute_size(g,attr,w,contextptr);
    else
      data=Equation_compute_size(string2gen("Object_too_large",false),attr,w,contextptr);
    Fl_Group::end();
    add_scroll_menu();
  }

  Equation::Equation(int x, int y, int w, int h, const char* l,const gen & g,attributs myattr) : Fl_Group(x, y, max(w,20), max(h,20), l){ 
    const giac::context * contextptr = get_context(this);
    labelsize(min(max(myattr.fontsize,10),16));
    xleft=0;
    ytop=h;
    xcur=0;
    ycur=0;
    begin_sel=-1;
    end_sel=-1;
    cb_escape=0;
    cb_enter=0;
    cb_backspace=0;
    cb_select=0;
    undo_history.clear();
    undo_history_pos=0;
    attr=myattr;
    modifiable=true;
    output_equation=true;
    if (taille(g,max_prettyprint_equation)<max_prettyprint_equation)
      data=Equation_compute_size(g,attr,w,contextptr);
    else
      data=Equation_compute_size(string2gen("Object_too_large",false),attr,w,contextptr);
    Fl_Group::end();
    add_scroll_menu();
  }

  Equation::Equation(int x, int y, int w, int h, const char* l,const gen & g,attributs myattr,GIAC_CONTEXT) : Fl_Group(x, y, max(w,20), max(h,20), l){ 
    labelsize(min(max(myattr.fontsize,10),16));
    xleft=0;
    ytop=h;
    xcur=0;
    ycur=0;
    begin_sel=-1;
    end_sel=-1;
    cb_escape=0;
    cb_enter=0;
    cb_backspace=0;
    cb_select=0;
    undo_history.clear();
    undo_history_pos=0;
    attr=myattr;
    modifiable=true;
    output_equation=true;
    if (taille(g,max_prettyprint_equation)<max_prettyprint_equation)
      data=Equation_compute_size(g,attr,w,contextptr);
    else
      data=Equation_compute_size(string2gen("Object_too_large",false),attr,w,contextptr);
    Fl_Group::end();
    add_scroll_menu();
  }

  void Equation::resize(int x, int y, int w, int h){
    Fl_Widget::resize(x,y,w,h);
    int L=giac::giacmin(20,labelsize());
    vscroll->resize(x+Fl_Group::w()-L,y,L,Fl_Group::h()-L);
    hscroll->resize(x,y+Fl_Group::h()-L,Fl_Group::w()-2*L,L);
    menubar->resize(hscroll->x()+hscroll->w(),hscroll->y(),2*L,L);
    setscroll();
  }

  Equation::~Equation(){
    if (Xcas_input_focus==this)
      Xcas_input_focus=0;
    remove(hscroll);
    remove(vscroll);
    remove(menubar);
    delete vscroll;
    delete hscroll;
    delete [] menubar->menu();
    delete menubar;
  }

  // return a variable from the list of vars
  bool select_user_var(int dx,int dy,gen & ans,GIAC_CONTEXT){
    gen res=_VARS(1,contextptr);
    if (res.type!=_VECT)
      return false;
    vecteur v(*res._VECTptr);
    int s=v.size();
    Fl_Group::current(0);
    Fl_Window * w = new Fl_Window(dx,dy);
    Fl_Return_Button * button0 = new Fl_Return_Button(2,2,dx/2-4,16);
    button0->shortcut(0xff0d);
    button0->label(gettext("OK"));
    Fl_Button * button1 = new Fl_Button(dx/2+2,2,dx/2-4,16);
    button1->shortcut(0xff1b);
    button1->label(gettext("Cancel"));
    Fl_Browser * browser = new Fl_Browser(2,20,dx,dy-22);
    browser->type(2);
    browser->label("Variables");
    // browser->callback((Fl_Callback*)cb_eqw_browser);
    for (int k=0;k<s;++k)
      browser->add(v[k].print(contextptr).c_str());
    browser->value(1);
    int i=1,r=0;
    w->end();
    w->resizable(w);
    w->set_modal();
    if (s){
      browser->value(1);
      w->show();
      Fl_Widget * savefocus=Fl::focus();
      Fl::focus(button1);
      for (;;) {
	Fl_Widget *o = Fl::readqueue();
	if (!o) Fl::wait();
	else {
	  if (o == button0) {r = 0; break;}
	  if (o == button1) {r = 1; break;}
	  if (o == w) { r=0; break; }
	}
      }
      Fl::focus(savefocus);
      w->hide();
      i=browser->value();
    }
    delete browser;
    delete button1;
    delete button0;
    delete w;
    if (r==0 && i<=s && i>0){
      ans=v[i-1];
      return true;
    }
    else
      return false;    
  } 

    Equation_Scrollbar::Equation_Scrollbar(int x,int y,int w,int h,Equation * ptr,bool is_vertical):Fl_Scrollbar(x,y,w,h),eqwptr(ptr),vertical(is_vertical) { 
      Fl_Scrollbar * o= this;
      o->maximum(1000);
      o->step(10);
      o->slider_size(1);
      o->callback( (Fl_Callback*) Equation_cb_scroll );
      if (is_vertical)
	o->type(FL_VERTICAL);
      else
	o->type(FL_HORIZONTAL);
      o->labelsize(giac::giacmin(20,ptr->labelsize()));
    };

#ifndef NO_NAMESPACE_XCAS
} // namespace xcas
#endif // ndef NO_NAMESPACE_XCAS

#endif // HAVE_LIBFLTK
