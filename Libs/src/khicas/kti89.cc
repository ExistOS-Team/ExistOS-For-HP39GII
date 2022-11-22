// -*- mode:C++ ; compile-command: "g++ -I.. -g -c ti89.cc -fno-strict-aliasing -DGIAC_GENERIC_CONSTANTS -DIN_GIAC -DHAVE_CONFIG_H" -*-
#include "giacPCH.h"
/*
 *  Copyright (C) 2000,2007 B. Parisse, Institut Fourier, 38402 St Martin d'Heres
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
#include <string.h>
// #include <numeric_limits>
#include "misc.h"
#include "usual.h"
#include "sym2poly.h"
#include "rpn.h"
#include "prog.h"
#include "derive.h"
#include "subst.h"
#include "intg.h"
#include "vecteur.h"
#include "ifactor.h"
#include "solve.h"
#include "modpoly.h"
#include "permu.h"
#include "sym2poly.h"
#include "plot.h"
#include "lin.h"
#include "modpoly.h"
#include "desolve.h"
#include "alg_ext.h"
#include "moyal.h"
#include "ti89.h"
#include "maple.h"
#include "input_parser.h"
#include "input_lexer.h"
#include "giacintl.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef FXCG
extern "C" {
  //#include <keyboard.h>
#include <libfx.h>
}
#endif

#ifndef NO_NAMESPACE_GIAC
namespace giac {
#endif // ndef NO_NAMESPACE_GIAC

  gen _seq(const gen & g,GIAC_CONTEXT){
    gen g1(g);
    if (g.type==_VECT && g.subtype==_SEQ__VECT && !g._VECTptr->empty()){
      vecteur v(*g._VECTptr);
      if (v.size()>=2){
	gen x(v[1]);
	if (is_equal(x) && x._SYMBptr->feuille.type==_VECT && !x._SYMBptr->feuille._VECTptr->empty())
	  x=x._SYMBptr->feuille._VECTptr->front();
	if (v.front().is_symb_of_sommet(at_quote))
	  v.front()=v.front()._SYMBptr->feuille;
	//gen tmp(quote_eval(makevecteur(v.front()),makevecteur(x),contextptr));
	//v.front()=tmp[0];
      }
      else
	v.front()=eval(v.front(),eval_level(contextptr),contextptr);
      if (v.size()==2 && is_integral((g1=eval(v[1],eval_level(contextptr),contextptr))) && g1.val>=0){
	vecteur res;
	int s=g1.val,l=eval_level(contextptr);
	res.reserve(s);
	gen v0=v[0];
	for (int i=0;i<s;++i)
	  res.push_back(eval(v0,l,contextptr));
	return res;
      }
      g1=gen(v,_SEQ__VECT);
    }
    return seqprod(g1,0,contextptr);
  }
  static const char _seq_s[]="seq";
  static define_unary_function_eval_quoted (__seq,&_seq,_seq_s);
  define_unary_function_ptr5( at_seq ,alias_at_seq,&__seq,_QUOTE_ARGUMENTS,true);

  gen _logb(const gen & g,GIAC_CONTEXT){
    if (g.type!=_VECT || g._VECTptr->size()!=2)
      return gensizeerr(contextptr);
    int n=0; gen e1(g._VECTptr->front()),b(g._VECTptr->back()),q;
    if (is_integer(e1) && is_integer(b) && is_strictly_greater(b,1,contextptr) && !is_zero(e1)){
      while (is_zero(irem(e1,b,q))){
	if (q.type==_ZINT)
	  e1=*q._ZINTptr;
	else
	  e1=q;
	++n;
      }
    }
    return rdiv(ln(e1,contextptr),ln(b,contextptr),contextptr)+n;
    // return ln(g._VECTptr->front(),contextptr)/ln(g._VECTptr->back(),contextptr);
  }
  static const char _logb_s[]="logb";
  static define_unary_function_eval (__logb,&_logb,_logb_s);
  define_unary_function_ptr5( at_logb ,alias_at_logb,&__logb,0,true);

  gen _isprime(const gen & args,GIAC_CONTEXT){
    gen g=_is_prime(args,contextptr);
    if (is_undef(g)) return g;
    if (g.type==_VECT)
      return g;
    if (g==0){
      g.subtype=_INT_BOOLEAN;
      return g;
    }
    g=plus_one;
    g.subtype=_INT_BOOLEAN;
    return g;
  }
  static const char _isprime_s[]="isprime";
  static define_unary_function_eval (__isprime,&_isprime,_isprime_s);
  define_unary_function_ptr5( at_isprime ,alias_at_isprime,&__isprime,0,true);

  gen _cSolve(const gen & g,GIAC_CONTEXT){
    bool old_complex_mode=complex_mode(contextptr);
    complex_mode(true,contextptr);
    gen res=_solve(g,contextptr);
    complex_mode(old_complex_mode,contextptr);
    return res;
  }
  static const char _csolve_s[]="csolve";
  static define_unary_function_eval_quoted (__csolve,&_cSolve,_csolve_s);
  define_unary_function_ptr5( at_csolve ,alias_at_csolve,&__csolve,_QUOTE_ARGUMENTS,true);

  gen _cFactor(const gen & g,GIAC_CONTEXT){
    bool old_complex_mode=complex_mode(contextptr);
    complex_mode(true,contextptr);
    gen res=_factor(g,contextptr);
    complex_mode(old_complex_mode,contextptr);
    return res;
  }
  static const char _cfactor_s[]="cfactor";
  static define_unary_function_eval (__cfactor,&_cFactor,_cfactor_s);
  define_unary_function_ptr5( at_cfactor ,alias_at_cfactor,&__cfactor,0,true);

  gen _cpartfrac(const gen & g,GIAC_CONTEXT){
    bool old_complex_mode=complex_mode(contextptr);
    complex_mode(true,contextptr);
    gen res=_partfrac(g,contextptr);
    complex_mode(old_complex_mode,contextptr);
    return res;
  }
  static const char _cpartfrac_s[]="cpartfrac";
  static define_unary_function_eval (__cpartfrac,&_cpartfrac,_cpartfrac_s);
  define_unary_function_ptr5( at_cpartfrac ,alias_at_cpartfrac,&__cpartfrac,0,true);

  static gen zeros(const gen &g,bool complexmode,GIAC_CONTEXT){
    vecteur v(solvepreprocess(g,complexmode,contextptr));
    int s=int(v.size());
    if (s>2)
      return gentoomanyargs("solve");
    return solve(remove_equal(v.front()),v.back(),complexmode,contextptr);
  }
  gen _zeros(const gen & g,GIAC_CONTEXT){
    if (g.type==_VECT && g._VECTptr->size()==2){
      gen a=eval(g._VECTptr->front(),1,contextptr);
      gen b=eval(g._VECTptr->back(),1,contextptr);
      if (is_integral(a) && is_integral(b))
	return _matrix(makesequence(a,b,0.0),contextptr);
    }
    return zeros(g,complex_mode(contextptr),contextptr);
  }
  static const char _zeros_s[]="zeros";
  static define_unary_function_eval_quoted (__zeros,&_zeros,_zeros_s);
  define_unary_function_ptr5( at_zeros ,alias_at_zeros,&__zeros,_QUOTE_ARGUMENTS,true);

  gen _cZeros(const gen & g,GIAC_CONTEXT){
    bool b=complex_mode(contextptr);
    complex_mode(true,contextptr);
    gen res=zeros(g,true,contextptr);
    complex_mode(b,contextptr);
    return res;
  }
  static const char _czeros_s[]="czeros";
  static define_unary_function_eval_quoted (__czeros,&_cZeros,_czeros_s);
  define_unary_function_ptr5( at_czeros ,alias_at_czeros,&__czeros,_QUOTE_ARGUMENTS,true);

  static gen getDenom(const gen & g,GIAC_CONTEXT){
    vecteur num,den;
    prod2frac(g,num,den);
    return vecteur2prod(den);
  }
  gen _getDenom(const gen & g,GIAC_CONTEXT){
    return apply(g,getDenom,contextptr);
  }
  static const char _getDenom_s[]="getDenom";
  static define_unary_function_eval (__getDenom,&_getDenom,_getDenom_s);
  define_unary_function_ptr5( at_getDenom ,alias_at_getDenom,&__getDenom,0,true);

  static gen denom(const gen & g,GIAC_CONTEXT){
    gen res=_fxnd(g,contextptr);
    if (res.type!=_VECT) return res;
    return res._VECTptr->back();
  }
  gen _denom(const gen & g,GIAC_CONTEXT){
    return apply(g,denom,contextptr);
  }
  static const char _denom_s[]="denom";
  static define_unary_function_eval (__denom,&_denom,_denom_s);
  define_unary_function_ptr5( at_denom ,alias_at_denom,&__denom,0,true);

  static gen getNum(const gen & g,GIAC_CONTEXT){
    vecteur num,den;
    prod2frac(g,num,den);
    return vecteur2prod(num);
  }
  gen _getNum(const gen & g,GIAC_CONTEXT){
    return apply(g,getNum,contextptr);
  }
  static const char _getNum_s[]="getNum";
  static define_unary_function_eval (__getNum,&_getNum,_getNum_s);
  define_unary_function_ptr5( at_getNum ,alias_at_getNum,&__getNum,0,true);

  static gen numer(const gen & g,GIAC_CONTEXT){
    gen res=_fxnd(g,contextptr);
    if (res.type!=_VECT) return res;
    return res._VECTptr->front();
  }
  gen _numer(const gen & g,GIAC_CONTEXT){
    return apply(g,numer,contextptr);
  }
  static const char _numer_s[]="numer";
  static define_unary_function_eval (__numer,&_numer,_numer_s);
  define_unary_function_ptr5( at_numer ,alias_at_numer,&__numer,0,true);


  gen _comDenom(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    if (g.type!=_VECT || g._VECTptr->size()<2 )
      return ratnormal(g,contextptr);
    vecteur & v(*g._VECTptr);
    return _reorder(makesequence(v.front(),vecteur(v.begin()+1,v.end())),contextptr);
  }
  static const char _comDenom_s[]="comDenom";
  static define_unary_function_eval (__comDenom,&_comDenom,_comDenom_s);
  define_unary_function_ptr5( at_comDenom ,alias_at_comDenom,&__comDenom,0,true);

  index_t rand_index(int dim,int tdeg,GIAC_CONTEXT){
    index_t res(dim);
    index_t w(tdeg+dim);
    for (unsigned int i=0;i<w.size();++i)
      w[i]=i;
    for (int i=0;i<dim;++i){
      int tmp=int((double(giac_rand(contextptr))*w.size())/rand_max2);
      res[i]=w[tmp];
      w.erase(w.begin()+tmp);
    }
    sort(res.begin(),res.end());
    for (int i=dim-1;i>0;--i){
      res[i] -= res[i-1]+1;
    }
    return res;
  }

  gen _randPoly(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    vecteur v(gen2vecteur(g));
    int vs=int(v.size()),deg=10;
    gen x=vx_var();
    gen f=0;
    if (vs>=3 && v[2].type==_INT_ && v[1].type==_INT_ && v[0].type==_VECT){
      // randpoly(variables,total_degree,nterms,[law])
      gen a=v[0],b=v[1];
      v[0]=b; v[1]=v[2]; v[2]=a; 
    }
    if (vs>=2 && v[1].type==_VECT && v[0].type==_VECT){
      // randpoly(variables,partial_degrees,[law])
      vecteur vars=*v[0]._VECTptr;
      vecteur degs=*v[1]._VECTptr;
      if (degs.size()!=vars.size() || vars.empty())
	return gendimerr(contextptr);
      gen var=vars[0];
      vars.erase(vars.begin());
      gen deg=degs[0];
      degs.erase(degs.begin());
      if (deg.type!=_INT_ || deg.val<0)
	return gendimerr(contextptr);
      if (!vars.empty()){
	gen res=0;
	for (int i=deg.val;i>=0;--i){
	  v[0]=vars;
	  v[1]=degs;
	  gen tmp=_randPoly(gen(v,_SEQ__VECT),contextptr);
	  res += tmp*pow(var,i,contextptr);
	}
	return res;
      }
      v[0]=var;
      v[1]=deg;
    }
    if (vs>=3 && v[0].type==_INT_ && v[1].type==_INT_ && v[2].type==_VECT){
      // randpoly(total_degree,nterms,variables,[law])
      if (vs>=4)
	f=v[3];
      if (vs==5){
	if (f.type==_FUNC)
	  f=symbolic(*f._FUNCptr,v.back());
	else
	  f=symb_of(f,v.back());
      }
      if (vs>5){
	if (f.type==_FUNC)
	  f=symbolic(*f._FUNCptr,gen(vecteur(v.begin()+4,v.end()),_SEQ__VECT));
	else
	  f=symb_of(f,gen(vecteur(v.begin()+4,v.end()),_SEQ__VECT));
      }
      int tdeg=v[0].val;
      int nterms=absint(v[1].val);
      int dim=int(v[2]._VECTptr->size());
      vecteur w=vranm(nterms,f,contextptr);
      polynome p(dim);
      for (int i=0;i<nterms;++i){
	index_t current=rand_index(dim,tdeg,contextptr);
	p.coord.push_back(monomial<gen>(w[i],current));
      }
      p.tsort();
      return _poly2symb(makesequence(p,v[2]),contextptr);
    }
    if (vs==1){
      if (v[0].type==_INT_)
	deg=v[0].val;
      else {
	x=v[0];
	deg=10;
      }
    }
    else {
      if (vs>=2){
	if (v[0].type==_INT_){
	  deg=v[0].val;
	  if (v[1].type==_IDNT)
	    x=v[1];
	  else
	    f=v[1];
	}
	else {
	  if (v[1].type==_INT_)
	    deg=v[1].val;
	  else
	    f=v[1];
	  x=v[0];
	}
      }
      if (vs>=3){
	f=v[2];
      }
      if (vs>=4 && v[2].type==_INT_ && v[3].type==_INT_){
	vecteur w(absint(deg)+1,1);
#if 0
	if (calc_mode(contextptr)==1 || abs_calc_mode(contextptr)==38){
	  if (v[2]==v[3])
	    return gensizeerr(contextptr);
	  for (;;){
	    w[0]=int(giac_rand(contextptr)/(rand_max2+1.0)*(v[3].val-v[2].val+1)+v[2].val);
	    if (w[0].val)
	      break;
	  }
	}
#endif
	for (unsigned i=1;i<=(unsigned)absint(deg);++i){
	  w[i]=int(giac_rand(contextptr)/(rand_max2+1.0)*(v[3].val-v[2].val+1)+v[2].val);
	}
#ifdef GIAC_HAS_STO_38
	return w;
#else
	return symb_horner(w,x);
#endif
      }
      if (vs==4){
	if (f.type==_FUNC)
	  f=symbolic(*f._FUNCptr,v.back());
	else
	  f=symb_of(f,v.back());
      }
      if (vs>4){
	if (f.type==_FUNC)
	  f=symbolic(*f._FUNCptr,gen(vecteur(v.begin()+3,v.end()),_SEQ__VECT));
	else
	  f=symb_of(f,gen(vecteur(v.begin()+3,v.end()),_SEQ__VECT));
      }
    }
    vecteur w;
#if 0
    if (calc_mode(contextptr)==1 || abs_calc_mode(contextptr)==38){
      for (int essai=0;essai<1000;++essai){
	w=vranm(absint(deg)+1,f,contextptr);
	if (!is_zero(w.front()))
	  break;
      }
    }
    else
#endif
      {
	w=vranm(absint(deg),f,contextptr);
	w.insert(w.begin(),1);
      }
#ifdef GIAC_HAS_STO_38
    return w;
#else
    return (f.type==_VECT && f._VECTptr->empty())?gen(w,_POLY1__VECT):symb_horner(w,x);
#endif
  }
  static const char _randpoly_s[]="randpoly";
  static define_unary_function_eval (__randpoly,&_randPoly,_randpoly_s);
  define_unary_function_ptr5( at_randpoly ,alias_at_randpoly,&__randpoly,0,true);

  gen _fMin(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    vecteur v(gen2vecteur(g));
    if (v.size()==1)
      v.push_back(vx_var());
    gen w(fminmax(v,4,contextptr));
    if (is_undef(w)) return w;
    return solvepostprocess(w,v[1],contextptr);
  }
  static const char _fMin_s[]="fMin";
  static define_unary_function_eval (__fMin,&_fMin,_fMin_s);
  define_unary_function_ptr5( at_fMin ,alias_at_fMin,&__fMin,0,true);

  gen _fMax(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    vecteur v(gen2vecteur(g));
    if (v.size()==1)
      v.push_back(vx_var());
    gen w(fminmax(v,5,contextptr));
    if (is_undef(w)) return w;
    return solvepostprocess(w,v[1],contextptr);
  }
  static const char _fMax_s[]="fMax";
  static define_unary_function_eval (__fMax,&_fMax,_fMax_s);
  define_unary_function_ptr5( at_fMax ,alias_at_fMax,&__fMax,0,true);

  gen _taylor(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    /* if (xcas_mode(contextptr)==0)
       return _series(g); */
    vecteur v(gen2vecteur(g));
    if (v.empty())
      return gentoofewargs("Taylor needs 3 args");
    if (v.back().type==_INT_ && v.back().subtype==_INT_MAPLECONVERSION && v.back().val==_POLY1__VECT){
      gen p=v.back();
      v.pop_back();
      gen res=_taylor(gen(v,_SEQ__VECT),contextptr);
      res=_convert(makesequence(res,p),contextptr);
      return res;
    }
    if (v.size()<2)
      v.push_back(vx_var());
    if (v.size()<3)
      v.push_back(5);
    else is_integral(v[2]);
    gen x0;
    if (v.size()==4)
      x0=v[3];
    if (is_equal(v[1]))
      return _series(makesequence(v[0],v[1],v[2]),contextptr);
    return _series(makesequence(v[0],symb_equal(v[1],x0),v[2]),contextptr);//_series(makesequence(v[0],symbolic(at_equal,makesequence(v[1],x0)),v[2]),contextptr);
  }
  static const char _taylor_s[]="taylor";
  static define_unary_function_eval (__taylor,&_taylor,_taylor_s);
  define_unary_function_ptr5( at_taylor ,alias_at_taylor,&__taylor,0,true);

  gen arclen(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    vecteur v(gen2vecteur(g));
    if (!v.empty() && v.front().type==_VECT){
      // more than one arc?
      if (!v.front()._VECTptr->empty() && v.front()._VECTptr->back().is_symb_of_sommet(at_pnt)){
	v.front()=v.front()._VECTptr->back();
	*logptr(contextptr) << gettext("Selecting last arc") << endl;
      }
    }
    if (v.size()==3)
      v.insert(v.begin()+1,ggb_var(v.front()));
    if (v.size()!=4 || v[1].type!=_IDNT)
      return gentoofewargs("arcLen");
    gen fprime=derive(v[0],v[1],contextptr);
    if (is_undef(fprime)) return fprime;
    if (fprime.type==_VECT)
      fprime=_l2norm(fprime,contextptr);
    else
      fprime=sqrt(normal(sq(fprime)+1,contextptr),contextptr);
    return _integrate(makesequence(fprime,v[1],v[2],v[3]),contextptr);
  }
  gen _arcLen(const gen & g,GIAC_CONTEXT){
    bool b=complex_variables(contextptr);
    complex_variables(false,contextptr);
    gen res=arclen(g,contextptr);
    complex_variables(b,contextptr);
    return res;
  }

  static const char _arclen_s[]="arclen";
  static define_unary_function_eval (__arclen,&_arcLen,_arclen_s);
  define_unary_function_ptr5( at_arclen ,alias_at_arclen,&__arclen,0,true);
  
  // find T,N,B,kappa,tau, Nsurkappa (center of osc circle-M) return dimension
  gen frenet(const gen &g,gen & M,gen & T, gen &N, gen &B, gen & kappa,gen &Nsurkappa,gen & tau,gen & t,bool compute_torsion,GIAC_CONTEXT){
    if (g.is_symb_of_sommet(at_pnt)){
      if (g._SYMBptr->feuille.type==_VECT && g._SYMBptr->feuille._VECTptr->size()>=2){ // element
	gen arg=(*g._SYMBptr->feuille._VECTptr)[1];
	if (arg.type==_VECT)
	  return frenet((*arg._VECTptr)[1],M,T,N,B,kappa,Nsurkappa,tau,t,compute_torsion,contextptr);
      }
      return frenet(remove_at_pnt(g),M,T,N,B,kappa,Nsurkappa,tau,t,compute_torsion,contextptr);
    }
    if (g.is_symb_of_sommet(at_curve)){
      gen f=g._SYMBptr->feuille;
      gen res=frenet(f[0],M,T,N,B,kappa,Nsurkappa,tau,t,compute_torsion,contextptr);
      return res;
    }
    if (g.type!=_VECT || g._VECTptr->size()<2)
      return gensizeerr(contextptr);
    vecteur v=*g._VECTptr;
    gen f=v[0],x=v[1],x0=undef;
    if (f.is_symb_of_sommet(at_pnt)){
      x0=v[1];
      if (x0.is_symb_of_sommet(at_pnt)){
	*logptr(contextptr) << "Assuming " << x0 << " is inside " << f << "\n";
	// x0=projection(f,x0,contextptr);
      }
      f=remove_at_pnt(f);
      if (!f.is_symb_of_sommet(at_curve))
	return gensizeerr(contextptr);
      f=f._SYMBptr->feuille[0];
      x=f[1];
      t=makesequence(x,f[2],f[3],f[4]);
      f=f[0];
    }
    else
      t=x;
    if (v.size()==3)
      x0=v[2];
    if (v.size()>3)
      t=gen(vecteur(v.begin()+1,v.begin()+4),_SEQ__VECT);
    if (f.type!=_VECT){
      gen r,i;
      reim(f,r,i,contextptr);
      f=makevecteur(r,i);
    }
    M=f;
    int dim=int(f._VECTptr->size());
    gen vitesse=derive(f,x,contextptr),v2=normal(l2norm2(vitesse),contextptr);
    gen sqrtv2=sqrt(v2,contextptr); // v=ds/dt
    T=vitesse/sqrtv2;
    gen accel=derive(vitesse,x,contextptr);
    if (dim==2 && T.type==_VECT && T._VECTptr->size()==2){
      kappa=normal(vitesse[0]*accel[1]-vitesse[1]*accel[0],contextptr)/pow(v2,2)*sqrtv2;
      Nsurkappa=normal(v2/(vitesse[0]*accel[1]-vitesse[1]*accel[0])*makevecteur(-vitesse[1],vitesse[0]),contextptr);
      N=makevecteur(-T._VECTptr->back(),T._VECTptr->front());
    }
    else {
      gen an=normal(accel-(scalar_product(accel,vitesse,contextptr)/v2)*vitesse,contextptr); // normal accel
      gen normean=_l2norm(an,contextptr);
      N=an/normean;
      kappa=normean/v2;
      Nsurkappa=normal(an*v2/l2norm2(an),contextptr);
    }
    if (dim!=3 || !compute_torsion)
      B=tau=undef;
    else {
      B=cross(T,N,contextptr);
      gen f3=derive(accel,x,contextptr);
      tau=_det(makesequence(vitesse,accel,f3),contextptr)/l2norm2(cross(vitesse,accel,contextptr));
    }
    if (!is_undef(x0)){
      M=subst(M,x,x0,false,contextptr);
      T=subst(T,x,x0,false,contextptr);
      N=subst(N,x,x0,false,contextptr);
      B=subst(B,x,x0,false,contextptr);
      tau=subst(tau,x,x0,false,contextptr);
      kappa=subst(kappa,x,x0,false,contextptr);
      Nsurkappa=subst(Nsurkappa,x,x0,false,contextptr);
    }
    return dim;
  }
  // curvature([x(t),y(t)],t)
  gen curvature(const gen & g,GIAC_CONTEXT){
    if (g.is_symb_of_sommet(at_pnt)){
      if (g._SYMBptr->feuille.type==_VECT && g._SYMBptr->feuille._VECTptr->size()>=2){ // element
	gen arg=(*g._SYMBptr->feuille._VECTptr)[1];
	if (arg.type==_VECT)
	  return curvature((*arg._VECTptr)[1],contextptr);
      }
      return curvature(remove_at_pnt(g),contextptr);
    }
    if (g.is_symb_of_sommet(at_curve)){
      gen f=g._SYMBptr->feuille;
      return curvature(f[0],contextptr);
    }    
    if (g.type!=_VECT || g._VECTptr->size()<2)
      return gensizeerr(contextptr);
    vecteur v=*g._VECTptr;
    gen f=v[0],x=v[1],x0(undef);
    if (f.is_symb_of_sommet(at_pnt)){
      x0=v[1];
      if (x0.is_symb_of_sommet(at_pnt)){
	*logptr(contextptr) << "Assuming " << x0 << " is inside " << f << "\n";
	// x0=projection(f,x0,contextptr);
      }
      f=remove_at_pnt(f);
      if (!f.is_symb_of_sommet(at_curve))
	return gensizeerr(contextptr);
      f=f._SYMBptr->feuille[0];
      x=f[1];
      f=f[0];
    }
    if (v.size()>=3)
      x0=v[2];
    if (f.type!=_VECT){
      gen r,i;
      reim(f,r,i,contextptr);
      if (is_zero(i))
	f=makevecteur(x,r);
      else
	f=makevecteur(r,i);
    }
    int dim=int(f._VECTptr->size());
    gen vitesse=derive(f,x,contextptr),v2=normal(l2norm2(vitesse),contextptr);
    gen sqrtv2=sqrt(v2,contextptr); // v=ds/dt
    gen accel=derive(vitesse,x,contextptr),res;
    if (dim==2)
      res= normal(vitesse[0]*accel[1]-vitesse[1]*accel[0],contextptr)/pow(v2,2)*sqrtv2;
    else {
      if (dim==3){
	gen f3=cross(vitesse,accel,contextptr);
	res= _l2norm(f3,contextptr)/pow(v2,2)*sqrtv2;
      }
      else {
	gen an=accel-(scalar_product(accel,vitesse,contextptr)/v2)*vitesse; // normal
	res=_l2norm(an,contextptr)/v2;
      }
    }
    if (!is_undef(x0))
      res=subst(res,x,x0,false,contextptr);
    return res;
  }
  gen _curvature(const gen & g,GIAC_CONTEXT){
    bool b=complex_variables(contextptr);
    complex_variables(false,contextptr);
    gen res=curvature(g,contextptr);
    complex_variables(b,contextptr);
    return res;
  }
  static const char _curvature_s[]="curvature";
  static define_unary_function_eval (__curvature,&_curvature,_curvature_s);
  define_unary_function_ptr5( at_curvature ,alias_at_curvature,&__curvature,0,true);

  gen _frenet(const gen & g,GIAC_CONTEXT){
    bool b=complex_variables(contextptr);
    complex_variables(false,contextptr);
    gen t,M,T,N,B,kappa,Nsurkappa,tau,dim=frenet(g,M,T,N,B,kappa,Nsurkappa,tau,t,true,contextptr);
    complex_variables(b,contextptr);
    if (dim.type!=_INT_)
      return dim;
    if (dim.val==2)
      return makesequence(kappa,M+Nsurkappa,T,N);
    else
      return makesequence(kappa,M+Nsurkappa,tau,T,N,B);
  }
  static const char _frenet_s[]="frenet";
  static define_unary_function_eval (__frenet,&_frenet,_frenet_s);
  define_unary_function_ptr5( at_frenet ,alias_at_frenet,&__frenet,0,true);

  gen _osculating_circle(const gen & args,GIAC_CONTEXT){
    vecteur attributs(1,COLOR_BLACK);
    vecteur v(seq2vecteur(args));
    int s=read_attributs(v,attributs,contextptr);
    if (s==0)
      return gendimerr(contextptr);
    gen g;
    if (s==1)
      g=v.front();
    else
      g=gen(vecteur(v.begin(),v.begin()+s),_SEQ__VECT);
    bool b=complex_variables(contextptr);
    complex_variables(false,contextptr);
    gen t,M,T,N,B,kappa,Nsurkappa,tau,dim=frenet(g,M,T,N,B,kappa,Nsurkappa,tau,t,false,contextptr);
    complex_variables(b,contextptr);
    if (dim.type!=_INT_)
      return dim;
    if (dim.val==2)
      return put_attributs(_cercle(makesequence(_point(M,contextptr),_point(M+2*Nsurkappa,contextptr)),contextptr),attributs,contextptr);
    return gendimerr(contextptr);
  }
  static const char _osculating_circle_s[]="osculating_circle";
  static define_unary_function_eval (__osculating_circle,&_osculating_circle,_osculating_circle_s);
  define_unary_function_ptr5( at_osculating_circle ,alias_at_osculating_circle,&__osculating_circle,0,true);

  gen _evolute(const gen & args,GIAC_CONTEXT){
    vecteur attributs(1,COLOR_BLACK);
    vecteur v(seq2vecteur(args));
    int s=read_attributs(v,attributs,contextptr);
    if (s==0)
      return gendimerr(contextptr);
    gen g;
    if (s==1)
      g=v.front();
    else
      g=gen(vecteur(v.begin(),v.begin()+s),_SEQ__VECT);
    bool b=complex_variables(contextptr);
    complex_variables(false,contextptr);
    gen t,M,T,N,B,kappa,Nsurkappa,tau,dim=frenet(g,M,T,N,B,kappa,Nsurkappa,tau,t,false,contextptr);
    complex_variables(b,contextptr);
    if (dim.type!=_INT_)
      return dim;
    if (dim.val==2){
      if (t.type==_VECT)
	t=gen(mergevecteur(vecteur(1,M+Nsurkappa),*t._VECTptr),_SEQ__VECT);
      else
	t=gen(makevecteur(M+Nsurkappa,t),_SEQ__VECT);
      return put_attributs(_plotparam(t,contextptr),attributs,contextptr);
    }
    return gendimerr(contextptr);
  }
  static const char _evolute_s[]="evolute";
  static define_unary_function_eval (__evolute,&_evolute,_evolute_s);
  define_unary_function_ptr5( at_evolute ,alias_at_evolute,&__evolute,0,true);

  gen _dim(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    if (!ckmatrix(g))
      return _size(g,contextptr);
    vecteur res(2);
    if (!g._VECTptr->empty()){
      res[0]=int(g._VECTptr->size());
      res[1]=int(g._VECTptr->front()._VECTptr->size());
    }
    return res;
  }
  static const char _dim_s[]="dim";
  static define_unary_function_eval (__dim,&_dim,_dim_s);
  define_unary_function_ptr5( at_dim ,alias_at_dim,&__dim,0,true);

  // FIXME: the print() should have an additionnal format argument
  // that would cover normal, tex, C, and formatted output
  static string format(const gen & g,const string & forme,GIAC_CONTEXT){
    if (g.type == _ZINT){
      string txt = g.print();
      if (!forme.empty()){
	char ch=forme[0];
	if (tolower(ch) == 'f')
	  return txt;
	if (forme.size()<2)
	  return txt;
	unsigned int digits = atol(forme.substr(1,forme.size()-1).c_str()) - 1;
	if (tolower(ch) == 'e')
	  digits ++;
	digits = digits < 2 ? 2 : digits;
	if (digits + 1 < txt.size()){
	  string tmp = txt.substr(0, 1) + "." + txt.substr(1, digits) + "e+" + print_INT_(int(txt.size()) - 1);
	  return tmp;
	}
      }  
      return txt;
    }
    else {
      gen tmp=evalf_double(g,eval_level(contextptr),contextptr);
      string saveforme=format_double(contextptr);
      format_double(contextptr)=forme;
      string s=tmp.print(contextptr);
      format_double(contextptr)=saveforme;
      return s;
    }
  }
  gen _format(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    vecteur v(gen2vecteur(g));
    size_t vs=v.size();
    if (vs>=2 && v[0].type==_STRNG){
      const string & fmt=*v[0]._STRNGptr;
      size_t fs=fmt.size(),count=1;
      string res;
      for (size_t i=0;i<fs;++i){
	if (i==fs-1 || fmt[i]!='{' ||  (i!=0 && fmt[i-1]=='\\')){
	  res += fmt[i];
	  continue;
	}
	if (fmt[i+1]=='}'){
	  ++i;
	  if (count<vs)
	    res += v[count].type==_STRNG? *v[count]._STRNGptr: v[count].print(contextptr);
	  else
	    return gendimerr(contextptr);
	  ++count;
	  continue;
	}
	int c=0;
	for (++i;i<fs && fmt[i]>='0' && fmt[i]<='9';++i){
	  c=c*10+int(fmt[i]-'0');	  
	}
	if (i==fs || fmt[i]!='}')
	  return gendimerr(contextptr);
	c++;
	res += v[c].type==_STRNG? *v[c]._STRNGptr: v[c].print(contextptr);
	++count;
      }
      return string2gen(res,false);
    }
    if (vs!=2 || v[1].type!=_STRNG)
      return gensizeerr(contextptr);
    return string2gen(format(v.front(),*v[1]._STRNGptr,contextptr),false);
  }
  static const char _format_s[]="format";
  static define_unary_function_eval (__format,&_format,_format_s);
  define_unary_function_ptr5( at_format ,alias_at_format,&__format,0,true);

  gen _left(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    if (g.type==_SYMB && g._SYMBptr->feuille.type==_VECT && !g._SYMBptr->feuille._VECTptr->empty())
      return g._SYMBptr->feuille._VECTptr->front();
#if defined HAVE_LIBMPFI && !defined NO_RTTI
    if (g.type==_REAL){
      if (real_interval * ptr=dynamic_cast<real_interval *>(g._REALptr)){
	mpfr_t tmp; mpfr_init2(tmp,mpfi_get_prec(ptr->infsup));
	mpfi_get_left(tmp,ptr->infsup);
	gen einf=real_object(tmp);
	mpfr_clear(tmp);
	return einf;
      }
    }
#endif
    vecteur v(1,g);
    if (v.size()<2 || !is_integral(v[1]) || v[1].type!=_INT_)
      return g;
    if (v[0].type==_STRNG)
      return string2gen(v[0]._STRNGptr->substr(0,v[1].val),false);
    if (v[0].type==_VECT){
      const_iterateur it=v[0]._VECTptr->begin(),itend=v[0]._VECTptr->end();
      int length=giacmax(0,giacmin(int(itend-it),v[1].val));
      return gen(vecteur(it,it+length),v[0].subtype);
    }
    return g;
  }
  static const char _left_s[]="left";
  static define_unary_function_eval (__left,&_left,_left_s);
  define_unary_function_ptr5( at_left ,alias_at_left,&__left,0,true);

  gen _right(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    if (g.type==_SYMB && g._SYMBptr->feuille.type==_VECT && !g._SYMBptr->feuille._VECTptr->empty())
      return g._SYMBptr->feuille._VECTptr->back();
#if defined HAVE_LIBMPFI && !defined NO_RTTI
    if (g.type==_REAL){
      if (real_interval * ptr=dynamic_cast<real_interval *>(g._REALptr)){
	mpfr_t tmp; mpfr_init2(tmp,mpfi_get_prec(ptr->infsup));
	mpfi_get_right(tmp,ptr->infsup);
	gen einf=real_object(tmp);
	mpfr_clear(tmp);
	return einf;
      }
    }
#endif
    vecteur v(1,g);
    if (v.size()<2 || !is_integral(v[1]) || v[1].type!=_INT_)
      return g;
    if (v[0].type==_STRNG){
      string & s=*v[0]._STRNGptr;
      int l=int(s.size());
      int m=giacmin(giacmax(v[1].val,0),l);
      return string2gen(s.substr(l-m,m),false);
    }
    if (v[0].type==_VECT){
      const_iterateur it=v[0]._VECTptr->begin(),itend=v[0]._VECTptr->end();
      int length=giacmax(0,giacmin(int(itend-it),v[1].val));
      return gen(vecteur(itend-length,itend),v[0].subtype);
    }
    return g;
  }
  static const char _right_s[]="right";
  static define_unary_function_eval (__right,&_right,_right_s);
  define_unary_function_ptr5( at_right ,alias_at_right,&__right,0,true);

  gen _mid(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    vecteur v(1,g);
    if (g.type==_VECT && g.subtype==_SEQ__VECT)
      v=*g._VECTptr;
    if (v.size()<2 || v[1].type!=_INT_)
      return g;
    int shift = array_start(contextptr); //xcas_mode(contextptr)!=0 || abs_calc_mode(contextptr)==38;
    int debut=v[1].val-shift;
    int nbre=RAND_MAX;
    if (v.size()>2 && v[2].type==_INT_)
      nbre=v[2].val;
    if (v[0].type==_STRNG){
      string & s=*v[0]._STRNGptr;
      if (debut>=signed(s.size()) || debut<0)
	return string2gen("",false);
      int m=giacmin(giacmax(nbre,0),int(s.size()));
      return string2gen(s.substr(debut,m),false);
    }
    if (v[0].type==_VECT){
      const_iterateur it=v[0]._VECTptr->begin(),itend=v[0]._VECTptr->end();
      if (debut>=itend-it || debut<0)
	return gen(vecteur(0),v[0].subtype);
      int length=giacmax(0,giacmin(int(itend-it)-debut,nbre));
      return gen(vecteur(it+debut,it+debut+length),v[0].subtype);
    }
    return g;
  }
  static const char _mid_s[]="mid";
  static define_unary_function_eval (__mid,&_mid,_mid_s);
  define_unary_function_ptr5( at_mid ,alias_at_mid,&__mid,0,true);

  gen _ord(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    if (g.type==_VECT)
      return apply(g,_ord,contextptr);
    if (g.type!=_STRNG || !g._STRNGptr->size())
      return gensizeerr(contextptr);
    return int((*g._STRNGptr)[0]);
  }
  static const char _ord_s[]="ord";
  static define_unary_function_eval (__ord,&_ord,_ord_s);
  define_unary_function_ptr5( at_ord ,alias_at_ord,&__ord,0,true);

  static gen shiftrotate(const gen & g,bool right){
    bool shift=right;
    vecteur v(1,g);
    if (g.type==_VECT && g.subtype==_SEQ__VECT)
      v=*g._VECTptr;
    int nbre=-1;
    /* if (shift)
       nbre=1; */
    if (v.size()>1 && v[1].type==_INT_)
      nbre=v[1].val;
    if (nbre<0){
      nbre=-nbre;
      right=!right;
    }
    gen & a=v[0];
    if (a.type==_INT_){
      if (right)
	return a.val >> nbre;
      else
	return a.val << nbre;
    }
    if (a.type==_VECT){
      const_iterateur it=a._VECTptr->begin(),itend=a._VECTptr->end();
      nbre=giacmin(nbre,int(itend-it));
      if (shift){
	if (right)
	  return gen(mergevecteur(vecteur(it+nbre,itend),vecteur(nbre,0)),a.subtype);
	return gen(mergevecteur(vecteur(nbre,0),vecteur(it,itend-nbre)),a.subtype);
      }
      if (right)
	return gen(mergevecteur(vecteur(itend-nbre,itend),vecteur(it,itend-nbre)),a.subtype);
      return gen(mergevecteur(vecteur(it+nbre,itend),vecteur(it,it+nbre)),a.subtype);
    }
    if (a.type==_STRNG){
      string & s=*a._STRNGptr;
      int l=int(s.size());
      nbre=giacmin(nbre,l);
      if (shift){
	if (right)
	  return string2gen(s.substr(nbre,l-nbre)+string(nbre,' '),false);
	return string2gen(string(l-nbre,' ')+s.substr(0,nbre),false);
      }
      if (right)
	return string2gen(s.substr(l-nbre,nbre)+s.substr(0,l-nbre),false);
      return string2gen(s.substr(nbre,l-nbre)+s.substr(0,nbre),false);
    }
    return a;
  }
  gen _rotate(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    return shiftrotate(g,false);
  }
  static const char _rotate_s[]="rotate";
  static define_unary_function_eval (__rotate,&_rotate,_rotate_s);
  define_unary_function_ptr5( at_rotate ,alias_at_rotate,&__rotate,0,true);

  gen _shift(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    return shiftrotate(g,true);
  }
  static const char _shift_s[]="shift";
  static define_unary_function_eval (__shift,&_shift,_shift_s);
  define_unary_function_ptr5( at_shift ,alias_at_shift,&__shift,0,true);

  gen _augment(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    return concat(g,false,contextptr);
  }
  static const char _augment_s[]="augment";
  static define_unary_function_eval (__augment,&_augment,_augment_s);
  define_unary_function_ptr5( at_augment ,alias_at_augment,&__augment,0,true);

  gen _semi_augment(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    return concat(g,true,contextptr);
  }
  static const char _semi_augment_s[]="semi_augment";
  static define_unary_function_eval (__semi_augment,&_semi_augment,_semi_augment_s);
  define_unary_function_ptr5( at_semi_augment ,alias_at_semi_augment,&__semi_augment,0,true);


  static gen _rightapply(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    if (g.type!=_VECT)
      return _right(g,contextptr);
    return apply(g,_rightapply,contextptr);
  }
  gen _list2mat(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    if (g.type!=_VECT)
      return gensizeerr(contextptr);
    vecteur v=*g._VECTptr;
    if (ckmatrix(g)){
      for (unsigned i=0;i<v.size();++i){
	v[i].subtype=0;
      }
      return gen(v,_MATRIX__VECT);
    }
    if (v.size()!=2)
      return gensizeerr(contextptr);
    gen taille=evalf_double(v[1],1,contextptr);
    if (g.subtype!=_SEQ__VECT || v.size()!=2 || v[0].type!=_VECT || taille.type!=_DOUBLE_)
      return gen(vecteur(1,v),_MATRIX__VECT);
    vecteur res;
    int nbre=giacmax(1,int(taille._DOUBLE_val));
    const_iterateur it=v[0]._VECTptr->begin(),itend=v[0]._VECTptr->end();
    for (;it!=itend;it+=nbre){
      if (itend-it<nbre){
	res.push_back(mergevecteur(vecteur(it,itend),vecteur(nbre-(itend-it))));
	break;
      }
      res.push_back(vecteur(it,it+nbre));
    }
    return gen(res,_MATRIX__VECT);
  }
  static const char _list2mat_s[]="list2mat";
  static define_unary_function_eval (__list2mat,&_list2mat,_list2mat_s);
  define_unary_function_ptr5( at_list2mat ,alias_at_list2mat,&__list2mat,0,true);

  gen _mat2list(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    if (g.type!=_VECT)
      return gensizeerr(contextptr);
    vecteur & v=*g._VECTptr;
    int subt=abs_calc_mode(contextptr)==38?_LIST__VECT:0;
    if (!ckmatrix(v))
      return gen(v,subt);
    vecteur res;
    const_iterateur it=v.begin(),itend=v.end();
    for (;it!=itend;++it){
      res=mergevecteur(res,*it->_VECTptr);
    }
    return gen(res,subt);
  }
  static const char _mat2list_s[]="mat2list";
  static define_unary_function_eval (__mat2list,&_mat2list,_mat2list_s);
  define_unary_function_ptr5( at_mat2list ,alias_at_mat2list,&__mat2list,0,true);


  gen _product(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    if (g.type==_VECT){
      if (g.subtype!=_SEQ__VECT)
	return prodsum(g.eval(eval_level(contextptr),contextptr),true);
      vecteur v=*g._VECTptr;
      maple_sum_product_unquote(v,contextptr);
      int s=int(v.size());
      if (!adjust_int_sum_arg(v,s))
	return gensizeerr(contextptr);
      if (v.size()==4 && (v[2].type!=_INT_ || v[3].type!=_INT_)){
	if (v[1].type!=_IDNT){
	  if (v[1].type!=_SYMB || lidnt(v[1]).empty())
	    return prodsum(g.eval(eval_level(contextptr),contextptr),true);
	  identificateur tmp("x");
	  v[0]=quotesubst(v[0],v[1],tmp,contextptr);
	  v[1]=tmp;
	  gen res=_product(gen(v,g.subtype),contextptr);
	  return quotesubst(res,tmp,v[1],contextptr);
	}
	v=quote_eval(v,makevecteur(v[1]),contextptr);
	gen n=v[1];
	vecteur lv(1,n);
	lvar(v[0],lv);
	if (is_zero(derive(vecteur(lv.begin()+1,lv.end()),n,contextptr),contextptr)){
	  v[0]=e2r(v[0],lv,contextptr);
	  gen p1,p2;
	  fxnd(v[0],p1,p2);
	  return simplify(product(gen2polynome(p1,int(lv.size())),lv,n,v[2],v[3],contextptr)/product(gen2polynome(p2,int(lv.size())),lv,n,v[2],v[3],contextptr),contextptr);
	}
      }
      if (v.size()==4 && v[2].type==_INT_ && v[3].type==_INT_ && v[2].val>v[3].val){
	if (v[3].val==v[2].val-1)
	  return 1;
#if defined RTOS_THREADX || defined BESTA_OS || defined USTL
	{ gen a=v[2]; v[2]=v[3]; v[3]=a; }
#else
	swap(v[2],v[3]);
#endif
	v[2]=v[2]+1;
	v[3]=v[3]-1;
	return inv(seqprod(gen(v,_SEQ__VECT),1,contextptr),contextptr);
      }
      return seqprod(gen(v,_SEQ__VECT),1,contextptr);
    }
    gen tmp=g.eval(eval_level(contextptr),contextptr);
    if (tmp.type==_VECT)
      return _product(tmp,contextptr);
    return seqprod(g,1,contextptr);
  }
  static const char _product_s[]="product";
  static define_unary_function_eval_quoted (__product,&_product,_product_s);
  define_unary_function_ptr5( at_product ,alias_at_product,&__product,_QUOTE_ARGUMENTS,true);


  bool complex_sort(const gen & a,const gen & b,GIAC_CONTEXT){
    if (a.type==_VECT && !a._VECTptr->empty() && b.type==_VECT && !b._VECTptr->empty())
      return complex_sort(a._VECTptr->front(),b._VECTptr->front(),contextptr);
    if (a==b)
      return false;
    if (a.type==_CPLX && b.type==_CPLX){
      if (*a._CPLXptr!=*b._CPLXptr)
	return is_strictly_greater(*b._CPLXptr,*a._CPLXptr,contextptr);
      return is_strictly_greater(*(b._CPLXptr+1),*(a._CPLXptr+1),contextptr);
    }
    if (a.type==_CPLX){
      if (*a._CPLXptr!=b)
	return is_strictly_greater(b,*a._CPLXptr,contextptr);
      return is_strictly_greater(0,*(a._CPLXptr+1),contextptr);
    }
    if (b.type==_CPLX){
      if (a!=*b._CPLXptr)
	return is_strictly_greater(*b._CPLXptr,a,contextptr);
      return is_strictly_greater(*(b._CPLXptr+1),0,contextptr);
    }
    gen g=inferieur_strict(a,b,contextptr); 
    if (g.type!=_INT_)
      return a.islesscomplexthan(b);
    return g.val==1;
  }
  gen sortad(const vecteur & v,bool ascend,GIAC_CONTEXT){
    if (v.empty()) return v;
    vecteur valeur=*eval(v,eval_level(contextptr),contextptr)._VECTptr;
    bool ismat=ckmatrix(valeur);
    if (!ismat)
      valeur=vecteur(1,valeur);
    valeur=mtran(valeur);
#if 1
    gen_sort_f_context(valeur.begin(),valeur.end(),complex_sort,contextptr);
    if (!ascend)
      vreverse(valeur.begin(),valeur.end());
#else
    if (ascend)
      gen_sort_f(valeur.begin(),valeur.end(),first_ascend_sort);
    else
      gen_sort_f(valeur.begin(),valeur.end(),first_descend_sort);
#endif
    valeur=mtran(valeur);
    if (!ismat)
      return valeur.front();
    return valeur;
  }
  gen _sorta(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    if (g.type==_VECT)
      return sortad(*g._VECTptr,true,contextptr);
    return gensizeerr(contextptr);
  }
  static const char _sorta_s[]="sorta";
  static define_unary_function_eval (__sorta,&_sorta,_sorta_s);
  define_unary_function_ptr5( at_sorta ,alias_at_sorta,&__sorta,0,true);

  gen _sortd(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    if (g.type==_VECT)
      return sortad(*g._VECTptr,false,contextptr);
    return gensizeerr(contextptr);
  }
  static const char _sortd_s[]="sortd";
  static define_unary_function_eval (__sortd,&_sortd,_sortd_s);
  define_unary_function_ptr5( at_sortd ,alias_at_sortd,&__sortd,0,true);

  static const char _approx_s[]="approx";
  static define_unary_function_eval (__approx,&_evalf,_approx_s);
  define_unary_function_ptr5( at_approx ,alias_at_approx,&__approx,0,true);

  static const char _ceiling_s[]="ceiling";
  static define_unary_function_eval (__ceiling,&_ceil,_ceiling_s);
  define_unary_function_ptr5( at_ceiling ,alias_at_ceiling,&__ceiling,0,true);

  static const char _imag_s[]="imag";
  static define_unary_function_eval (unary__imag,(const gen_op_context)im,_imag_s);
  define_unary_function_ptr5( at_imag ,alias_at_imag,&unary__imag,0,true);

  static const char _real_s[]="real";
  static define_unary_function_eval (unary__real,(const gen_op_context)re,_real_s);
  define_unary_function_ptr5( at_real ,alias_at_real,&unary__real,0,true);

  gen _int(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    if (g.type==_VECT && g.subtype==_SEQ__VECT && g._VECTptr->size()==2 && g._VECTptr->front().type==_STRNG && g._VECTptr->back().type==_INT_){
      gen b=g._VECTptr->back();
      if (b.val<2 || b.val>36)
	return gendimerr(contextptr);
      gen res=0;
      const string & s=*g._VECTptr->front()._STRNGptr;
      int ss=int(s.size());
      for (int i=0;i<ss;++i){
	char ch=s[i];
	res = res*b;
	if (ch>='0' && ch<='9'){
	  res += int(ch-'0');
	  continue;
	}
	if (ch>='A' && ch<='Z'){
	  res += int(ch-'A')+10;
	  continue;
	}
	if (ch>='a' && ch<='z'){
	  res += int(ch-'a')+10;
	  continue;
	}
      }
      return res;
    }
    if (xcas_mode(contextptr)==3 || (python_compat(contextptr) && g.type!=_VECT)){
      gen g_=eval(g,1,contextptr);
      if (g_.type==_STRNG)
	g_=gen(*g_._STRNGptr,contextptr);
      return _floor(evalf(g_,1,contextptr),contextptr);
    }
    else
      return _integrate(g,contextptr);
  }
  static const char _int_s[]="int";
  static define_unary_function_eval (__int,(const gen_op_context)_int,_int_s);
  define_unary_function_ptr5( at_int ,alias_at_int,&__int,_QUOTE_ARGUMENTS,true);


  vecteur gen2vecteur(const gen & g,int exclude){
    if (g.type!=_VECT || g.subtype==exclude)
      return vecteur(1,g);
    return *g._VECTptr;
  }

  gen L1norm(const gen & g,GIAC_CONTEXT){
    if (g.type!=_VECT)
      return abs(g,contextptr);
    vecteur & v=*g._VECTptr;
    gen res;
    const_iterateur it=v.begin(),itend=v.end();
    for (;it!=itend;++it)
      res=res+abs(*it,contextptr);
    return res;
  }

  gen _rowNorm(const gen & g,GIAC_CONTEXT) {
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    if (!ckmatrix(g))
      return gentypeerr(contextptr);
    const_iterateur it=g._VECTptr->begin(),itend=g._VECTptr->end();
    gen res;
    for (;it!=itend;++it){
      res=max(res,L1norm(*it,contextptr),contextptr);
    }
    return res;
  }
  static const char _rownorm_s[]="rownorm";
  static define_unary_function_eval (__rownorm,&_rowNorm,_rownorm_s);
  define_unary_function_ptr5( at_rownorm ,alias_at_rownorm,&__rownorm,0,true);

  gen _colNorm(const gen & g,GIAC_CONTEXT) {
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    if (!ckmatrix(g))
      return gentypeerr(contextptr);
    return _rowNorm(mtran(*g._VECTptr),contextptr);
  }
  static const char _colnorm_s[]="colnorm";
  static define_unary_function_eval (__colnorm,&_colNorm,_colnorm_s);
  define_unary_function_ptr5( at_colnorm ,alias_at_colnorm,&__colnorm,0,true);

  gen exact_double(double d,double eps){
    if (eps<1e-14)
      eps=1e-14;
    if (d<0)
      return -exact_double(-d,eps);
    if (d > (1<<30) )
      return _floor(d,context0);
    if (d==0)
      return 0;
    if (d<1)
      return inv(exact_double(1/d,eps),context0);
    vector<int> res;
    double eps1(1+eps);
    for (;!interrupted;){
#ifdef TIMEOUT
      control_c();
#endif
      if (ctrl_c || interrupted) { 
	interrupted = true; ctrl_c=false;
	return gensizeerr(gettext("Stopped by user interruption.")); 
      }
      res.push_back(int(d*eps1));
      d=d-int(d*eps1);
      if (d<=eps)
	break;
      d=1/d;
      if (d > (1<<30))
	break;
      eps=eps*d*d;
    }
    if (res.empty())
      return gensizeerr(gettext("Stopped by user interruption.")); 
    reverse(res.begin(),res.end());
    vector<int>::const_iterator it=res.begin(),itend=res.end();
    gen x(*it);
    for (++it;it!=itend;++it){
      x=*it+inv(x,context0);
    }
    return x;
  }
  gen exact(const gen & g,GIAC_CONTEXT){
    switch (g.type){
    case _DOUBLE_:
      return exact_double(g._DOUBLE_val,epsilon(contextptr));
      //case _REAL: return exact_double(evalf_double(g,1,contextptr)._DOUBLE_val,epsilon(contextptr));
#ifdef BCD
    case _FLOAT_:
      return exact_double(evalf_double(g,1,contextptr)._DOUBLE_val,1e-10);
#endif
    case _CPLX:
      return exact(re(g,contextptr),contextptr)+cst_i*exact(im(g,contextptr),contextptr);
    case _SYMB:
      return symbolic(g._SYMBptr->sommet,exact(g._SYMBptr->feuille,contextptr));
    case _VECT:
      return apply(g,exact,contextptr);
    default:
      return g;
    }
  }
  gen _exact(const gen & g,GIAC_CONTEXT){
#if 0 // def BCD
    return symb_quote(exact(g,contextptr));
#else
    return exact(g,contextptr);
#endif
  }
  static const char _exact_s[]="exact";
  static define_unary_function_eval (__exact,&_exact,_exact_s);
  define_unary_function_ptr5( at_exact ,alias_at_exact,&__exact,0,true);

  gen fPart(const gen & g,GIAC_CONTEXT){
    if (is_undef(g))
      return g;
    if (is_equal(g))
      return apply_to_equal(g,fPart,contextptr);
    if (g.type==_VECT)
      return apply(g,fPart,contextptr);
    // if (is_strictly_positive(-g,contextptr)) return -fPart(-g,contextptr);
    // return g-_floor(g,contextptr);
    return g-_INT(g,contextptr);
  } 
  static const char _frac_s[]="frac";
  static define_unary_function_eval (__frac,&fPart,_frac_s);
  define_unary_function_ptr5( at_frac ,alias_at_frac,&__frac,0,true);


  gen _getKey(const gen & g,GIAC_CONTEXT){
      // //!!!!!!!!!!!!!!!!!!!!!!!!
      //!!!!!!
      //std::cout << "chk get_key" << endl;
      
      if (g.type==_INT_)
        return IsKeyDown(g.val);
      int key;
      GetKey(&key);
      if (key==KEY_CTRL_EXIT) key=5;
      if (key==KEY_CTRL_EXE) key=4; // OK
      if (key==KEY_CTRL_LEFT) key=0;
      if (key==KEY_CTRL_RIGHT) key=3;
      if (key==KEY_CTRL_UP) key=1;
      if (key==KEY_CTRL_DOWN) key=2;    
      return key;
      
      //return false;
  }
  static const char _getKey_s[]="get_key";
  static define_unary_function_eval(__getKey,&_getKey,_getKey_s);
  define_unary_function_ptr5( at_getKey ,alias_at_getKey,&__getKey,0,true);




#ifndef NO_NAMESPACE_GIAC
} // namespace giac
#endif // ndef NO_NAMESPACE_GIAC
