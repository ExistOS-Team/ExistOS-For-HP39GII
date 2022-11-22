// -*- mode:C++ ; compile-command: "g++ -I.. -I../include -g -c -Wall -DHAVE_CONFIG_H -DIN_GIAC csturm.cc" -*-
#include "giacPCH.h"

/*
 *  Copyright (C) 2000,14 B. Parisse, Institut Fourier, 38402 St Martin d'Heres
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
#include <cmath>
#include <stdexcept>
#include <map>
#include "gen.h"
#include "csturm.h"
#include "vecteur.h"
#include "modpoly.h"
#include "unary.h"
#include "symbolic.h"
#include "usual.h"
#include "sym2poly.h"
#include "solve.h"
#include "prog.h"
#include "subst.h"
#include "permu.h"
#include "series.h"
#include "alg_ext.h"
#include "ti89.h"
#include "plot.h"
#include "modfactor.h"
#include"giacintl.h"

#ifndef NO_NAMESPACE_GIAC
namespace giac {
#endif // ndef NO_NAMESPACE_GIAC
  // compute Sturm sequence of r0 and r1,
  // returns gcd (without content)
  // and compute list of quotients, coeffP, coeffR
  // such that coeffR*r_(k+2) = Q_k*r_(k+1) - coeffP_k*r_k
  gen csturm_seq(modpoly & r0,modpoly & r1,vecteur & listquo,vecteur & coeffP, vecteur & coeffR,GIAC_CONTEXT){
    listquo.clear();
    coeffP.clear();
    coeffR.clear();
    if (r0.empty())
      return r1;
    if (r1.empty())
      return r0;
    gen tmp;
    lcmdeno(r0,tmp,contextptr);
    if (ck_is_positive(-tmp,contextptr))
      r0=-r0;
    r0=r0/abs(lgcd(r0),contextptr);
    lcmdeno(r1,tmp,contextptr);
    if (ck_is_positive(-tmp,contextptr))
      r1=-r1;
    r1=r1/abs(lgcd(r0),contextptr);
    // set auxiliary constants g and h to 1
    gen g(1),h(1);
    modpoly a(r0),b(r1),quo,r;
    gen b0(1);
    for (int loop_counter=0;;++loop_counter){
      int m=int(a.size())-1;
      int n=int(b.size())-1;
      int ddeg=m-n; // should be 1 generically
      if (!n) { // if b is constant, gcd=1
	return 1;
      }
      b0=b.front();
      if (b.front().type==_VECT) {
	// ddeg should be even if b0 is a _POLY1
	if (ddeg%2==0)
	  *logptr(contextptr) << gettext("Singular parametric Sturm sequence ") << a << "/" << b << endl;
      }
      else
	b0=abs(b.front(),contextptr); 
      coeffP.push_back(pow(b0,ddeg+1));
      DivRem(coeffP.back()*a,b,0,quo,r);
      listquo.push_back(quo);
      coeffR.push_back(g*pow(h,ddeg));
      if (r.empty()){
	return b/abs(lgcd(b),contextptr);
      }
      // remainder is non 0, loop continue: a <- b
      a=b;
      // now divides r by g*h^(m-n) and change sign, result is the new b
      b= -r/coeffR.back();
      g=b0;
      h=pow(b0,ddeg)/pow(h,ddeg-1);
    } // end while loop
  }

  int csturm_square(const gen & p,const gen & a,const gen & b,gen& pgcd,GIAC_CONTEXT){
    return -1;
  }

  vecteur crationalroot(polynome & p,bool complexe){
    vectpoly v;
    int i=1;
    polynome qrem;
    environment * env= new environment;
    env->complexe=complexe || !is_zero(im(p,context0));
    vecteur w;
    if (!do_linearfind(p,env,qrem,v,w,i))
      w.clear();
    delete env;
    p=qrem;
    return w;
  }

  static gen round2util(const gen & num,const gen & den,int n){
    if (num.type==_CPLX){
      gen r=round2util(*num._CPLXptr,den,n);
      gen i=round2util(*(num._CPLXptr+1),den,n);
      return r+cst_i*i;
    }
    // num must be a _ZINT
    mpz_t tmp1,tmp2;
    mpz_init_set(tmp1,*num._ZINTptr);
    mpz_mul_2exp(tmp1,tmp1,n+1); // tmp1=2^(n+1)*num
    mpz_add(tmp1,tmp1,*den._ZINTptr); //      + den
    mpz_init_set(tmp2,*den._ZINTptr);
    mpz_mul_ui(tmp2,tmp2,2); // tmp2=2*den
    mpz_fdiv_q(tmp1,tmp1,tmp2);
    gen res=tmp1;
    mpz_clear(tmp1); mpz_clear(tmp2);
    return res;
  }

  void in_round2(gen & x,const gen & deuxn, int n){
    if (x.type==_INT_ || x.type==_ZINT)
      return ;
    if (x.type==_FRAC && x._FRACptr->den.type==_CPLX)
      x=fraction(x._FRACptr->num*conj(x._FRACptr->den,context0),x._FRACptr->den.squarenorm(context0));
    if (x.type==_FRAC && x._FRACptr->den.type==_ZINT && 
	(x._FRACptr->num.type==_ZINT || 
	 (x._FRACptr->num.type==_CPLX && x._FRACptr->num._CPLXptr->type==_ZINT && (x._FRACptr->num._CPLXptr+1)->type==_ZINT)) 
	){
      gen num=x._FRACptr->num,d=x._FRACptr->den;
      x=round2util(num,d,n);
      x=x/deuxn;
      return;
    }
    x=_floor(x*deuxn+plus_one_half,context0)/deuxn;
  }

  void round2(gen & x,int n){
    if (x.type==_INT_ || x.type==_ZINT)
      return ;
    gen deuxn;
    if (n<30)
      deuxn = (1<<n);
    else {
      mpz_t tmp;
      mpz_init_set_si(tmp,1);
      mpz_mul_2exp(tmp,tmp,n);
      deuxn=tmp;
      mpz_clear(tmp);
    }
    in_round2(x,deuxn,n);
  }

  void round2(gen & x,const gen & deuxn,GIAC_CONTEXT){
    if (x.type==_INT_ || x.type==_ZINT)
      return;
    if (x.type!=_FRAC)
      x=_floor(x*deuxn+plus_one_half,context0)/deuxn;
    else {
      gen n=x._FRACptr->num,d=x._FRACptr->den;
      if (d.type==_INT_){
	int di=d.val,ni=1;
	while (di>1){ di=di>>1; ni=ni<<1;}
	if (ni==d.val)
	  return;
      }
      n=2*n*deuxn+d;
      x=iquo(n,2*d)/deuxn;
    }
  }


  gen _realroot(const gen & g,GIAC_CONTEXT){
    gen tmp=_proot(g,contextptr);
    if (tmp.type==_VECT){
      vecteur res;
      const_iterateur it=tmp._VECTptr->begin(),itend=tmp._VECTptr->end();
      for (;it!=itend;++it){
	if (is_real(*it,contextptr))
	  res.push_back(*it);
      }
      tmp=res;
    }
    return tmp;
  }
#ifndef NO_NAMESPACE_GIAC
} // namespace giac
#endif // ndef NO_NAMESPACE_GIAC
