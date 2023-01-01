// -*- mode:C++ ; compile-command: "g++-3.4 -I.. -I../include -g -c -Wall modpoly.cc  -DHAVE_CONFIG_H -DIN_GIAC" -*-
// N.B.: compiling with g++-3.4 -O2 -D_I386_ does not work
#include "giacPCH.h"
/*  Univariate dense polynomials including modular arithmetic
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
using namespace std;
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "sym2poly.h"
#include "modpoly.h"
#include "usual.h"
#include "prog.h"
#include "derive.h"
#include "ezgcd.h"
#include "cocoa.h" // for memory_usage
#include "quater.h"
#include "modfactor.h"
#include "giacintl.h"
#include <stdlib.h>
#include <cmath>
#include <stdexcept>
#include <string.h>
#ifdef HAVE_SYS_TIME_H
#include <time.h>
#else
#if !defined BESTA_OS && !defined EMCC && !defined EMCC2
#define clock_t int
#define CLOCK() 0
#endif
#endif

#define GIAC_PRECOND 1 // if multiplying by w mod p, pre-computes (w*2^32)/p
//#define GIAC_CACHEW 1 // FFT, cache w^(2^t*p) for t>0
#if defined GIAC_CACHEW && GIAC_PRECOND
#undef GIAC_PRECOND // incompatible
#endif
//#undef GIAC_PRECOND

// vector class version 1 by Agner Fog https://github.com/vectorclass
// this might be faster for CPU with AVX512DQ instruction set
// (fast multiplication of Vec4q)
#ifdef HAVE_VCL1_VECTORCLASS_H 
#include <vcl1/vectorclass.h>
#endif

#ifndef NO_NAMESPACE_GIAC
namespace giac {
#endif // ndef NO_NAMESPACE_GIAC

  const double prec(1.0/(1LL<<51)); // 52?

  double find_invp(int p){
    return (1.0/p)*(1.0-prec); // insure that invp is lower than 1/p
  }

  // Fourier primes that fit in 32 bit int
  const int p1=2013265921,p2=1811939329,p3=469762049,p4=2113929217;
  const double invp1=(1.0-prec)/p1,invp2=(1.0-prec)/p2,invp3=(1.0-prec)/p3,invp4=(1.0-prec)/p4;
  const longlong p1p2=longlong(p1)*p2,p1p2sur2=p1p2/2;

  gen _fft_mult_size(const gen & args,GIAC_CONTEXT){
    if (args.type==_VECT && args._VECTptr->empty())
      return FFTMUL_SIZE;
    if (args.type!=_INT_ || args.val<1)
      return gensizeerr(contextptr);
    return FFTMUL_SIZE=args.val;
  }
  static const char _fft_mult_size_s []="fft_mult_size";
  static define_unary_function_eval (__fft_mult_size,&_fft_mult_size,_fft_mult_size_s);
  define_unary_function_ptr5( at_fft_mult_size ,alias_at_fft_mult_size,&__fft_mult_size,0,true);

  static const char _fft_mult_s []="fft_mult";
  static define_unary_function_eval (__fft_mult,&_fft_mult_size,_fft_mult_s);
  define_unary_function_ptr5( at_fft_mult ,alias_at_fft_mult,&__fft_mult,0,true);

  gen _min_proba_time(const gen & args,GIAC_CONTEXT){
    if (args.type==_INT_ && args.val>=0)
      return min_proba_time=args.val;
    if (args.type==_DOUBLE_ && args._DOUBLE_val>=0)
      return min_proba_time=args._DOUBLE_val;
    if (args.type==_VECT && args._VECTptr->empty())
      return min_proba_time;
    return gensizeerr(contextptr);
  }
  static const char _min_proba_time_s []="min_proba_time";
  static define_unary_function_eval (__min_proba_time,&_min_proba_time,_min_proba_time_s);
  define_unary_function_ptr5( at_min_proba_time ,alias_at_min_proba_time,&__min_proba_time,0,true);

  // random modular number
  gen nrandom(environment * env){
    if (env->moduloon && is_zero(env->coeff)){
      double d=env->modulo.to_int();
      int j=(int) (d*std_rand()/(RAND_MAX+1.0));
      return smod(gen(j),env->modulo);
    }
    else {
      double d=env->pn.to_int();
      int j=(int) (d*std_rand()/(RAND_MAX+1.0));
      return env->coeff.makegen(j);
    }
  }

  gen invenv(const gen & g,environment * env){
    if (g.type==_USER)
      return g._USERptr->inv();
    return invmod(g,env->modulo);
  }

  /*
  void inpowmod(const gen & a,int n,const gen & m,gen & res){
    if (!n){
      res=gen(1);
      return ;
    }
    if (n==1){
      res=a;
      return ;
    }
    inpowmod(a,n/2,m,res);
    res=smod((res*res),m);
    if (n%2)
      res=smod((res*a),m);
  }

  gen powmod(const gen & a,int n,const gen & m){
    if (!n)
      return 1;
    if (n==1)
      return a;
    assert(n>1);
    gen res;
    inpowmod(a,n,m,res);
    return res;
  }
  */
  unsigned powmod(unsigned a,unsigned long n,unsigned m){
    if (!n)
      return 1;
    if (n==1)
      return a;
    if (n==2)
      return (a*ulonglong(a))%m;
    unsigned b=a%m,c=1;
    while (n>0){
      if (n%2)
	c=(c*ulonglong(b))%m;
      n /= 2;
      b=(b*ulonglong(b))%m;
    }
    return c;
  }


  modpoly derivative(const modpoly & p){
    if (p.empty())
      return p;
    modpoly new_coord;
    int d=int(p.size())-1;
    new_coord.reserve(d);
    modpoly::const_iterator it=p.begin(); // itend=p.end(),
    for (;d;++it,--d)
      new_coord.push_back((*it)*gen(d));
    return new_coord;
  }

  modpoly derivative(const modpoly & p,environment * env){
    if (p.empty())
      return p;
    modpoly new_coord;
    int d=int(p.size())-1;
    new_coord.reserve(d);
    modpoly::const_iterator it=p.begin(); // itend=p.end(),
    gen n0( 0);
    for (;d;++it,--d)
      if ( smod((*it)*gen(d),env->modulo)!=n0 )
	break;
    for (;d;++it,--d)
      new_coord.push_back( smod((*it)*gen(d),env->modulo) );
    return new_coord;
  }

  modpoly integrate(const modpoly & p,const gen & shift_coeff){
    if (p.empty())
      return p;
    modpoly new_coord;
    new_coord.reserve(p.size());
    modpoly::const_iterator itend=p.end(),it=p.begin();
    for (int d=0;it!=itend;++it,++d)
      new_coord.push_back(normal(rdiv((*it),gen(d)+shift_coeff,context0),context0));
    return new_coord;
  }
  

  static bool is_rational(double d,int & num,int & den,double eps){
    double dcopy(d);
    // continued fraction expansion
    vector<int> v;
    for (int n=1;n<11;++n){
      v.push_back(int(d));
      d=d-int(d);
      if (fabs(d)<eps*n)
	break;
      d=1/d;
    }
    // re_VECTose fraction
    num=0;
    den=1;
    reverse(v.begin(),v.end());
    for (vector<int>::const_iterator it=v.begin();it!=v.end();++it){
      num=num+den*(*it);
      swap(num,den);
    }
    swap(num,den);
    return fabs(dcopy-(num*1.0)/den)<eps;
  }


  // return n such that p=phi_n, p is assumed to be irreducible
  // return 0 if p is not cyclotomic
  int is_cyclotomic(const modpoly & p,double eps){
    modpoly q; gen e;
    modpoly::const_iterator itend=p.end(),it=p.begin();
    for (;it!=itend;++it){
      if (it->type==_POLY){
	if (it->_POLYptr->coord.empty())
	  e=zero;
	else {
	  if (Tis_constant<gen>(*it->_POLYptr))
	    e=it->_POLYptr->coord.front().value;
	  else
	    return 0;
	}
      }
      else
	e=*it;
      if (e.type!=_INT_)
	return 0;
      q.push_back(e);
    }
    // q has integer coeff, q(X) must be = X^n conj(q(1/conj(X)))
    // if it has all its root over the unit circle
    // since q has integer coeff, q=X^n*q(1/X) i.e. is symmetric
    modpoly qs(q);
    reverse(q.begin(),q.end());
    if (q!=qs)
      return 0;
    // find arg of a root and compare to 2*pi
    gen r=a_root(qs,0,eps);
    if (is_undef(r)) return 0;
    double arg_d=evalf_double(arg(r,context0),1,context0)._DOUBLE_val;
    if (arg_d<0)
      arg_d=-arg_d;
    double d=2*M_PI/ arg_d;
    // find rational approx of d
    int num,den;
    if (!is_rational(d,num,den,eps) || num>100)
      return 0;
    if (p==cyclotomic(num))
      return num;
    else
      return 0;
  }
  int is_cyclotomic(const modpoly & p,GIAC_CONTEXT){
    return is_cyclotomic(p,epsilon(contextptr)); 
  }
  // use 0 for Z, n!=0 for Z/nZ
  modpoly modularize(const polynome & p,const gen & n,environment * env){
    bool ismod;
    if (env && env->coeff.type!=_USER && !is_zero(n)){
      env->modulo=n;
      env->pn=env->modulo;
      ismod=true;
      env->moduloon=true;
    }
    else
      ismod=false;
    gen n0(0);
    vecteur v;
    if (p.dim!=1) 
      return vecteur(1,gensizeerr(gettext("modpoly.cc/modularize")));
    if (p.coord.empty())
      return v;
    int deg=p.lexsorted_degree();
    int curpow=deg;
    v.reserve(deg+1);
    vector< monomial<gen> >::const_iterator it=p.coord.begin();
    vector< monomial<gen> >::const_iterator itend=p.coord.end();
    for (;it!=itend;++it){
      int newpow=it->index.front();
      for (;curpow>newpow;--curpow)
	v.push_back(n0);
      if (ismod)
	v.push_back(smod(it->value,env->modulo));
      else
	v.push_back(it->value);
      --curpow;
    }
    for (;curpow>-1;--curpow)
      v.push_back(n0);      
    return v;
  }

  modpoly modularize(const dense_POLY1 & p,const gen & n,environment * env){
    env->modulo=n;
    env->pn=env->modulo;
    env->moduloon=true;
    if (p.empty())
      return p;
    modpoly v;
    gen n0( 0);
    dense_POLY1::const_iterator it=p.begin(),itend=p.end();
    for (;it!=itend;++it){
      if (smod(*it,n)!=n0)
	break;
    }
    for (;it!=itend;++it)
      v.push_back(smod(*it,n));
    return v;
  }

  polynome unmodularize(const modpoly & a){
    if (a.empty())
      return polynome(1);
    vector< monomial<gen> > v;
    index_t i;
    int deg=int(a.size())-1;
    i.push_back(deg);
    vecteur::const_iterator it=a.begin();
    vecteur::const_iterator itend=a.end();
    gen n0( 0);
    for (;it!=itend;++it,--i[0]){
      if (*it!=n0)
	v.push_back(monomial<gen>(*it,i));
    }
    return polynome(1,v);
  }

  // random polynomial of degree =i
  modpoly random(int i,environment * env){
    vecteur v;
    v.reserve(i+1);
    gen e;
    do
      e=nrandom(env);
    while
      (is_zero(e));
    v.push_back(e);
    for (int j=1;j<=i;j++)
      v.push_back(nrandom(env));
    return v;
  }

  bool is_one(const modpoly & p){
    if (p.size()!=1)
      return false;
    return (is_one(p.front()));
  }

  // 1
  modpoly one(){
    vecteur v;
    v.push_back(gen(1));
    return v;
  }

  // x=x^1
  modpoly xpower1(){
    vecteur v;
    v.push_back(gen( 1));
    v.push_back(gen( 0));
    return v;
  }

  bool normalize_env(environment * env){
    if ( (env->moduloon && is_zero(env->coeff)) || is_zero(env->pn)){
      env->pn=env->modulo;
      if (env->complexe)
	env->pn = env->pn * env->pn ;
    }
    return (env->pn.type==_INT_);
  }

  // x^modulo
  modpoly xpowerpn(environment * env){
    if (!normalize_env(env))
      return vecteur(1,gendimerr(gettext("Field too large")));
    int deg=env->pn.val;
    vecteur v(deg+1);
    v[0]=1;
    return v;
  }

  // x -> x^p (non modular)
  vecteur x_to_xp(const vecteur & v, int p){
    if (p<=0) 
      return vecteur(1,gensizeerr(gettext("modpoly.cc/x_to_xp")));
    if ( (p==1) || v.empty())
      return v;
    const_iterateur it=v.begin(),itend=v.end();
    vecteur res;
    res.reserve(1+(itend-it-1)*p);
    res.push_back(*it);
    ++it;
    for (;it!=itend;++it){
      for (int i=1;i<p;++i)
	res.push_back(zero);
      res.push_back(*it);
    }
    return res;
  }

  // multiply by x^n
  void shiftmodpoly(modpoly & a,int n){
    a.reserve(a.size()+n);
    for (int i=0;i<n;i++)
      a.push_back(0);
  }

  // high = high*x^n + low, size of low must be < n
  void mergemodpoly(modpoly & high,const modpoly & low,int n){
    int l=int(low.size());
    for (int i=0;i<n-l;i++)
      high.push_back(0);
    modpoly::const_iterator it=low.begin(), itend=low.end();
    for (;it!=itend;++it)
      high.push_back(*it);
  }

  gen cstcoeff(const modpoly & q){
    modpoly::const_iterator it=q.end();
    --it;
    return *it;
  }

  // !! Do not call with modpoly slices if new_coord and th/other overlapp
  void Addmodpoly(modpoly::const_iterator th_it,modpoly::const_iterator th_itend,modpoly::const_iterator other_it,modpoly::const_iterator other_itend,environment * env, modpoly & new_coord){
    int n=int(th_itend-th_it);
    int m=int(other_itend-other_it);
    if (m>n){ // swap th and other in order to have n>=m
      modpoly::const_iterator tmp=th_it;
      th_it=other_it;
      other_it=tmp;
      tmp=th_itend;
      th_itend=other_itend;
      other_itend=tmp;
      int saven=n;
      n=m;
      m=saven;
    }
    if (m && other_it==new_coord.begin()){
      modpoly temp(new_coord);
      Addmodpoly(th_it,th_itend,temp.begin(),temp.end(),env,new_coord);
      return;
    }
    if (n && (th_it==new_coord.begin()) ){
      modpoly::iterator th=new_coord.begin()+n-m;
      bool trim=(n==m);
      // in-place addition
      if (env && env->moduloon)
	for (;m;++th,++other_it,--m)
	  *th=smod((*th)+(*other_it), env->modulo);
      else
	for (;m;++th,++other_it,--m)
	  *th += (*other_it);
      if (trim){ 
	for (th=new_coord.begin();th!=th_itend;++th){
	  if (!is_zero(*th))
	    break;
	}
	new_coord.erase(new_coord.begin(),th);
      }
      return;
    }
    new_coord.clear();
    if ( (n<0) || (m<0) )
      return ;
    new_coord.reserve(n);
    if (n>m){ // no trimming needed
      for (;n>m;++th_it,--n)
	new_coord.push_back(*th_it);
    }
    else { // n==m, first remove all 0 terms of the sum
      if (env && env->moduloon)
	for (;n && is_zero(smod((*th_it)+(*other_it), env->modulo));++th_it,++other_it,--n)
	  ;
      else
	for (;n && is_zero(*th_it+*other_it);++th_it,++other_it,--n)
	  ;
    }
    // finish addition
    if (env && env->moduloon)
      for (;n;++th_it,++other_it,--n)
	new_coord.push_back(smod((*th_it)+(*other_it), env->modulo));
    else
      for (;n;++th_it,++other_it,--n)
	new_coord.push_back( *th_it+(*other_it) );
  }

  void addmodpoly(const modpoly & th, const modpoly & other, environment * env,modpoly & new_coord){
    // assert( (&th!=&new_coord) && (&other!=&new_coord) );
    modpoly::const_iterator th_it=th.begin(),th_itend=th.end();
    modpoly::const_iterator other_it=other.begin(),other_itend=other.end();
    Addmodpoly(th_it,th_itend,other_it,other_itend,env,new_coord);
  }

  void addmodpoly(const modpoly & th, const modpoly & other, modpoly & new_coord){
    // assert( (&th!=&new_coord) && (&other!=&new_coord) );
    modpoly::const_iterator th_it=th.begin(),th_itend=th.end();
    modpoly::const_iterator other_it=other.begin(),other_itend=other.end();
    environment * env=new environment;
    Addmodpoly(th_it,th_itend,other_it,other_itend,env,new_coord);
    delete env;
  }
  
  // modular polynomial arithmetic: gcd, egcd, simplify
  modpoly operator_plus (const modpoly & th,const modpoly & other,environment * env) {
#ifdef TIMEOUT
    control_c();
#endif
    if (ctrl_c || interrupted) { 
      interrupted = true; ctrl_c=false;
      return modpoly(1,gensizeerr(gettext("Stopped by user interruption."))); 
    }
    // Tensor addition
    if (th.empty())
      return other;
    if (other.empty())
      return th;
    modpoly new_coord;
    addmodpoly(th,other,env,new_coord);
    return new_coord;
  } 

  modpoly operator + (const modpoly & th,const modpoly & other) {
#ifdef TIMEOUT
    control_c();
#endif
    if (ctrl_c || interrupted) { 
      interrupted = true; ctrl_c=false;
      return modpoly(1,gensizeerr(gettext("Stopped by user interruption."))); 
    }
    // Tensor addition
    if (th.empty())
      return other;
    if (other.empty())
      return th;
    modpoly new_coord;
    addmodpoly(th,other,new_coord);
    return new_coord;
  } 


  void Submodpoly(modpoly::const_iterator th_it,modpoly::const_iterator th_itend,modpoly::const_iterator other_it,modpoly::const_iterator other_itend,environment * env,modpoly & new_coord){
    int n=int(th_itend-th_it);
    if (!n){
      new_coord=modpoly(other_it,other_itend);
      mulmodpoly(new_coord,-1,new_coord);
      return;
    }
    int m=int(other_itend-other_it);
    if (th_it==new_coord.begin()){
      if (n<m){
	modpoly temp(new_coord);
	Submodpoly(temp.begin(),temp.end(),other_it,other_itend,env,new_coord);
	return;
      }
      else {
	modpoly::iterator th=new_coord.begin()+n-m;
	bool trim=(n==m);
	// in-place -
	if (env && env->moduloon)
	  for (;m;++th,++other_it,--m)
	    *th=smod((*th)-(*other_it), env->modulo);
	else
	  for (;m;++th,++other_it,--m)
	    *th -= (*other_it);
	if (trim){ 
	  for (th=new_coord.begin();th!=th_itend;++th){
	    if (!is_zero(*th))
	      break;
	  }
	  new_coord.erase(new_coord.begin(),th);
	}
      }
      return;
    }
    if (m && (other_it==new_coord.begin()) ){
      bool inplace=(m>n);
      if (n==m){ // look if highest coeff vanishes
	if (env && env->moduloon)
	  inplace=!is_zero(smod((*th_it)-(*other_it), env->modulo));
	else
	  inplace=!is_zero((*th_it)-(*other_it));
      }
      if (inplace){ // in-place subtraction
	modpoly::iterator th=new_coord.begin();
	if (env && env->moduloon){
	  for (;m>n;++th,--m)
	    *th=smod(-(*th),env->modulo);
	  for (;m;++th_it,++th,--m)
	    *th=smod((*th_it)-(*th), env->modulo);
	}
	else {
	  for (;m>n;++th,--m)
	    *th=-(*th);
	  for (;m;++th_it,++th,--m)
	    *th=(*th_it)-(*th);
	}
	return;
      }
      else { // copy new_coord to a temporary and call again Addmodpoly
	modpoly temp(new_coord);
	Submodpoly(th_it,th_itend,temp.begin(),temp.end(),env,new_coord);
	return;
      }
    }
    if ( (n<0) || (m<0) )
      return ;
    new_coord.clear();
    new_coord.reserve(giacmax(n,m));
    bool trimming;
    if (m==n)
      trimming=true;
    else
      trimming=false;
    if (env && env->moduloon){
      for (;m>n;++other_it,--m)
	new_coord.push_back(smod(-*other_it,env->modulo));
    }
    else {
      for (;m>n;++other_it,--m)
	new_coord.push_back(-*other_it);
    }
    for (;n>m;++th_it,--n)
      new_coord.push_back(*th_it);
    if (env && env->moduloon)
      for (;n;++th_it,++other_it,--n){
	gen tmp=smod((*th_it)-(*other_it), env->modulo);
	if ( trimming){ 
	  if (!is_zero(tmp)){
	    trimming=false;
	    new_coord.push_back(tmp);
	  }
	}
	else 
	  new_coord.push_back(tmp);
      }
    else
      for (;n;++th_it,++other_it,--n){
	gen tmp=(*th_it)-(*other_it);
	if ( trimming){ 
	  if (!is_zero(tmp)){
	    trimming=false;
	    new_coord.push_back(tmp);
	  }	
	}
	else 
	  new_coord.push_back(tmp);
      }
  }

  void submodpoly(const modpoly & th, const modpoly & other, environment * env,modpoly & new_coord){
    // assert( (&th!=&new_coord) && (&other!=&new_coord) );
    modpoly::const_iterator th_it=th.begin(),th_itend=th.end();
    modpoly::const_iterator other_it=other.begin(),other_itend=other.end();
    Submodpoly(th_it,th_itend,other_it,other_itend,env,new_coord);
  }

  void submodpoly(const modpoly & th, const modpoly & other, modpoly & new_coord){
    // assert( (&th!=&new_coord) && (&other!=&new_coord) );
    modpoly::const_iterator th_it=th.begin(),th_itend=th.end();
    modpoly::const_iterator other_it=other.begin(),other_itend=other.end();
    environment * env=new environment;
    Submodpoly(th_it,th_itend,other_it,other_itend,env,new_coord);
    delete env;
  }

  modpoly operator_minus (const modpoly & th,const modpoly & other,environment * env) {  
#ifdef TIMEOUT
    control_c();
#endif
    if (ctrl_c || interrupted) { 
      interrupted = true; ctrl_c=false;
      return modpoly(1,gensizeerr(gettext("Stopped by user interruption."))); 
    }
    // Tensor sub
    if (th.empty())
      return -other;
    if (other.empty())
      return th;    
    modpoly new_coord;
    submodpoly(th,other,env,new_coord);
    return new_coord;
  }

  modpoly operator - (const modpoly & th,const modpoly & other) {  
#ifdef TIMEOUT
    control_c();
#endif
    if (ctrl_c || interrupted) { 
      interrupted = true; ctrl_c=false;
      return modpoly(1,gensizeerr(gettext("Stopped by user interruption."))); 
    }
    // Tensor sub
    if (th.empty())
      return -other;
    if (other.empty())
      return th;    
    modpoly new_coord;
    submodpoly(th,other,new_coord);
    return new_coord;
  } 

  void mulmodpoly(const modpoly & th, const gen & fact,environment * env, modpoly & new_coord){
    if (!env || !env->moduloon){
      mulmodpoly(th,fact,new_coord);
      return;
    }
    if (is_exactly_zero(fact)){
      new_coord.clear();
      return ;
    }
    if (&th==&new_coord){
      if (is_one(fact))
	return;
      modpoly::iterator it=new_coord.begin(),itend=new_coord.end();
      if (!env->complexe && (env->modulo.type==_INT_) && (fact.type==_INT_) && (env->modulo.val<smallint) && (fact.val<smallint)){
	for (;it!=itend;++it)
	  it->val=smod( (it->val)*fact.val,env->modulo.val ) ;
      }
      else {
	for (;it!=itend;++it)
	  *it=smod( (*it)*fact,env->modulo);
      }
    }
    else { // &th!=&new_coord
      if (is_one(fact)){
	new_coord=th;
	return;
      }
      new_coord.clear();
      new_coord.reserve(th.size());
      modpoly::const_iterator it=th.begin(),itend=th.end();
      if (!env->complexe && (env->modulo.type==_INT_) && (fact.type==_INT_) && (env->modulo.val<smallint) && (fact.val<smallint)){
	for (;it!=itend;++it)
	  new_coord.push_back(smod( (it->val)*fact.val,env->modulo.val) );
      }
      else {
	for (;it!=itend;++it)
	  new_coord.push_back(smod( (*it)*fact,env->modulo) );
      }
    }
  }

  void mulmodpoly(const modpoly & th, const gen & fact, modpoly & new_coord){
    if (is_exactly_zero(fact)){
      new_coord.clear();
      return ;
    }
    if (&th==&new_coord){
      if (is_one(fact))
	return;
      modpoly::iterator it=new_coord.begin(),itend=new_coord.end();
#ifndef USE_GMP_REPLACEMENTS
      if (fact.type==_INT_){
	for (;it!=itend;++it){
	  if (it->type==_ZINT && it->ref_count()==1)
	    mpz_mul_si(*it->_ZINTptr,*it->_ZINTptr,fact.val);
	  else
	    *it= (*it)*fact;
	}
	return;
      }
      if (fact.type==_ZINT){
	for (;it!=itend;++it){
	  if (it->type==_ZINT && it->ref_count()==1)
	    mpz_mul(*it->_ZINTptr,*it->_ZINTptr,*fact._ZINTptr);
	  else
	    *it= (*it)*fact;
	}
	return;
      }
#endif
      for (;it!=itend;++it)
	type_operator_times(*it,fact,*it); // *it= (*it)*fact;
    }
    else { // &th!=&new_coord
      new_coord.clear();
      new_coord.reserve(th.size());
      modpoly::const_iterator it=th.begin(),itend=th.end();
      for (;it!=itend;++it)
	new_coord.push_back((*it)*fact);
    }
  } 

  modpoly operator * (const modpoly & th, const gen & fact){
#ifdef TIMEOUT
    control_c();
#endif
    if (ctrl_c || interrupted) { 
      interrupted = true; ctrl_c=false;
      return modpoly(1,gensizeerr(gettext("Stopped by user interruption."))); 
    }
    // Tensor constant multiplication
    if (is_one(fact))
      return th;
    modpoly new_coord;
    mulmodpoly(th,fact,new_coord);
    return new_coord;
  }

  modpoly operator * (const gen & fact,const modpoly & th){
#ifdef TIMEOUT
    control_c();
#endif
    if (ctrl_c || interrupted) { 
      interrupted = true; ctrl_c=false;
      return modpoly(1,gensizeerr(gettext("Stopped by user interruption."))); 
    }
    if (is_one(fact))
      return th;
    modpoly new_coord;
    mulmodpoly(th,fact,new_coord);
    return new_coord;
  }

  modpoly operator * (const modpoly & a, const modpoly & b) {
    environment env;
    modpoly temp(operator_times(a,b,&env));
    return temp;
  }
  

  modpoly operator_times(const modpoly & th, const gen & fact,environment * env){
#ifdef TIMEOUT
    control_c();
#endif
    if (ctrl_c || interrupted) { 
      interrupted = true; ctrl_c=false;
      return modpoly(1,gensizeerr(gettext("Stopped by user interruption."))); 
    }
    // Tensor constant multiplication
    if (is_one(fact))
      return th;
    modpoly new_coord;
    mulmodpoly(th,fact,env,new_coord);
    return new_coord;
  }

  modpoly operator_times(const gen & fact,const modpoly & th,environment * env){
#ifdef TIMEOUT
    control_c();
#endif
    if (ctrl_c || interrupted) { 
      interrupted = true; ctrl_c=false;
      return modpoly(1,gensizeerr(gettext("Stopped by user interruption."))); 
    }
    if (is_one(fact))
      return th;
    modpoly new_coord;
    mulmodpoly(th,fact,env,new_coord);
    return new_coord;
  }

  // *res = *res + a*b, *res must not be elsewhere referenced
  inline void add_mul(mpz_t * res,mpz_t & prod,const gen &a,const gen &b){
    switch ( (a.type<< _DECALAGE) | b.type) {
    case _INT___INT_:
      mpz_set_si(prod,a.val);
#ifdef mpz_mul_si
      mpz_mul_si(prod,prod,b.val);
#else
      if (b.val<0){
	mpz_mul_ui(prod,prod,-b.val);
	mpz_neg(prod,prod);
      }
      else
	mpz_mul_ui(prod,prod,b.val);
#endif
      break;
    case _ZINT__ZINT:
      mpz_mul(prod,*a._ZINTptr,*b._ZINTptr);
      break;
    case _INT___ZINT:
#ifdef mpz_mul_si
      mpz_mul_si(prod,*b._ZINTptr,a.val);
#else
      if (a.val<0){
	mpz_mul_ui(prod,*b._ZINTptr,-a.val);
	mpz_neg(prod,prod);
      }
      else
	mpz_mul_ui(prod,*b._ZINTptr,a.val);
#endif
      break;
    case _ZINT__INT_:
#ifdef mpz_mul_si
      mpz_mul_si(prod,*a._ZINTptr,b.val);
#else
      if (b.val<0){
	mpz_mul_ui(prod,*a._ZINTptr,-b.val);
	mpz_neg(prod,prod);
      }
      else
	mpz_mul_ui(prod,*a._ZINTptr,b.val);
#endif
      break;
    }
    mpz_add(*res,*res,prod);
  }

  // *res = *res - a*b, *res must not be referenced elsewhere
  inline void sub_mul(mpz_t * res,mpz_t & prod,const gen &a,const gen &b){
    switch ( (a.type<< _DECALAGE) | b.type) {
    case _INT___INT_:
      mpz_set_si(prod,a.val);
#ifdef mpz_mul_si
      mpz_mul_si(prod,prod,b.val);
#else
      if (b.val<0){
	mpz_mul_ui(prod,prod,-b.val);
	mpz_neg(prod,prod);
      }
      else
	mpz_mul_ui(prod,prod,b.val);
#endif
      break;
    case _ZINT__ZINT:
      mpz_mul(prod,*a._ZINTptr,*b._ZINTptr);
      break;
    case _INT___ZINT:
#ifdef mpz_mul_si
      mpz_mul_si(prod,*b._ZINTptr,a.val);
#else
      if (a.val<0){
	mpz_mul_ui(prod,*b._ZINTptr,-a.val);
	mpz_neg(prod,prod);
      }
      else
	mpz_mul_ui(prod,*b._ZINTptr,a.val);
#endif
      break;
    case _ZINT__INT_:
#ifdef mpz_mul_si
      mpz_mul_si(prod,*a._ZINTptr,b.val);
#else
      if (b.val<0){
	mpz_mul_ui(prod,*a._ZINTptr,-b.val);
	mpz_neg(prod,prod);
      }
      else
	mpz_mul_ui(prod,*a._ZINTptr,b.val);
#endif
      break;
    }
    mpz_sub(*res,*res,prod);
  }

  // set madeg to RAND_MAX if no truncation in degree 
  static void Muldense_POLY1(const modpoly::const_iterator & ita0,const modpoly::const_iterator & ita_end,const modpoly::const_iterator & itb0,const modpoly::const_iterator & itb_end,environment * env,modpoly & new_coord,int taille,int maxdeg){
    if (ita0==ita_end || itb0==itb_end || maxdeg<0){
      new_coord.clear();
      return;
    }
    mpz_t prod;
    mpz_init(prod);
    int newdeg=(ita_end-ita0)+(itb_end-itb0)-2,skip=0;
    if (maxdeg>=0 && newdeg>maxdeg){
      skip=newdeg-maxdeg;
      newdeg=maxdeg;
    }
    new_coord.resize(newdeg+1);
    modpoly::const_iterator ita_begin=ita0-1,ita=ita0,itb=itb0;
    gen * target=&new_coord.front();
    if (taille<128) 
      taille=0; 
    else {
      taille=sizeinbase2(taille/128);
      taille=(128 << taille);
    }
    ref_mpz_t * res = new ref_mpz_t(taille?taille:128); 
    for ( ; ita!=ita_end; ++ita ){
      if (skip){
	--skip;
	continue;
      }
      modpoly::const_iterator ita_cur=ita,itb_cur=itb;
      for (;itb_cur!=itb_end && ita_cur!=ita_begin;--ita_cur,++itb_cur) {
	add_mul(&res->z,prod,*ita_cur,*itb_cur); // res = res + (*ita_cur) * (*itb_cur);
      }
      int oldtaille=mpz_sizeinbase(res->z,2);
      if (env && env->moduloon){
	*target=smod(gen(res),env->modulo);
	res = new ref_mpz_t(taille?taille:oldtaille+64); 
      }
      else {
	// *target=res; 
	if (ref_mpz_t2gen(res,*target))
	  res = new ref_mpz_t(taille?taille:oldtaille+64); 
	else
	  mpz_set_si(res->z,0);
      }
      ++target;
    }
    --ita;
    ++itb;
    for ( ; itb!=itb_end;++itb){
      if (skip){
	--skip;
	continue;
      }
      modpoly::const_iterator ita_cur=ita,itb_cur=itb;
      for (;itb_cur!=itb_end && ita_cur!=ita_begin;--ita_cur,++itb_cur) {
	add_mul(&res->z,prod,*ita_cur,*itb_cur); // res=res+((*ita_cur)) * ((*itb_cur));
      }
      int oldtaille=mpz_sizeinbase(res->z,2);
      if (env && env->moduloon){
	*target=smod(gen(res),env->modulo);
	res = new ref_mpz_t(taille?taille:oldtaille); 
      }
      else {
	// *target=res; 
	if (ref_mpz_t2gen(res,*target))
	  res = new ref_mpz_t(taille?taille:oldtaille); 
	else
	  mpz_set_si(res->z,0);
      }
      ++target;
    }
    delete res;
    mpz_clear(prod);
  }

  // new_coord += a*b, used in gen.cc
  void add_mulmodpoly(const modpoly::const_iterator & ita0,const modpoly::const_iterator & ita_end,const modpoly::const_iterator & itb0,const modpoly::const_iterator & itb_end,environment * env,modpoly & new_coord){
    if (ita0==ita_end || itb0==itb_end)
      return;
    bool same=ita0==itb0 && ita_end==itb_end;
    mpz_t prod;
    mpz_init(prod);
    int ncs=int(new_coord.size());
    int news=int((ita_end-ita0)+(itb_end-itb0)-1);
    if (ncs<news)
      new_coord=mergevecteur(vecteur(news-ncs,0),new_coord);
    modpoly::const_iterator ita_begin=ita0-1,ita=ita0,itb=itb0;
    gen * target=&new_coord.front();
    if (ncs>news)
      target += (ncs-news);
    for ( ; ita!=ita_end; ++ita,++target ){
      if (!env && target->type==_ZINT && target->ref_count()==1){
	mpz_t * resz=target->_ZINTptr;
	modpoly::const_iterator ita_cur=ita,itb_cur=itb;
	for (;itb_cur!=itb_end && ita_cur!=ita_begin;--ita_cur,++itb_cur) {
	  add_mul(resz,prod,*ita_cur,*itb_cur); // res = res + (*ita_cur) * (*itb_cur);
	}
      }
      else {
	ref_mpz_t * res=new ref_mpz_t; 
	mpz_t * resz=&res->z;
	if (target->type==_INT_)
	  mpz_set_si(*resz,target->val);
	else
	  mpz_set(*resz,*target->_ZINTptr);
	modpoly::const_iterator ita_cur=ita,itb_cur=itb;
	for (;itb_cur!=itb_end && ita_cur!=ita_begin;--ita_cur,++itb_cur) {
	  add_mul(resz,prod,*ita_cur,*itb_cur); // res = res + (*ita_cur) * (*itb_cur);
	}
	if (env && env->moduloon)
	  *target=smod(gen(res),env->modulo);
	else
	  *target=res;
      }
    }
    --ita;
    ++itb;
    for ( ; itb!=itb_end;++itb,++target){
      if (!env && target->type==_ZINT && target->ref_count()==1){
	mpz_t * resz=target->_ZINTptr;
	modpoly::const_iterator ita_cur=ita,itb_cur=itb;
	for (;itb_cur!=itb_end && ita_cur!=ita_begin;--ita_cur,++itb_cur) {
	  add_mul(resz,prod,*ita_cur,*itb_cur); // res = res + (*ita_cur) * (*itb_cur);
	}
      }
      else {
	ref_mpz_t * res=new ref_mpz_t; 
	mpz_t * resz=&res->z;
	if (target->type==_INT_)
	  mpz_set_si(*resz,target->val);
	else
	  mpz_set(*resz,*target->_ZINTptr);
	modpoly::const_iterator ita_cur=ita,itb_cur=itb;
	for (;itb_cur!=itb_end && ita_cur!=ita_begin;--ita_cur,++itb_cur) {
	  add_mul(resz,prod,*ita_cur,*itb_cur); // res = res + (*ita_cur) * (*itb_cur);
	}
	if (env && env->moduloon)
	  *target=smod(gen(res),env->modulo);
	else
	  *target=res;
      }
    }
    mpz_clear(prod);
  }

  // new_coord memory must be reserved, Mulmodpoly clears new_coord
  // set madeg to RAND_MAX if no truncation in degree 
  static void Mulmodpolymod(modpoly::const_iterator ita,modpoly::const_iterator ita_end,modpoly::const_iterator itb,modpoly::const_iterator itb_end,environment * env,modpoly & new_coord,bool intcoeff,int taille,int seuil_kara,int maxdeg){
    if (maxdeg<0)
      return;
    if (ita_end-ita-1>maxdeg)
      ita=ita_end-maxdeg-1;
    if (itb_end-itb-1>maxdeg)
      itb=itb_end-maxdeg-1;
    int a=int(ita_end-ita);
    int b=int(itb_end-itb);
    if (!b)
      return ;
    if ( ( a <= seuil_kara) || ( b <= seuil_kara) ){
      if (intcoeff)
	Muldense_POLY1(ita,ita_end,itb,itb_end,env,new_coord,taille,maxdeg);
      else
	mulmodpoly_naive(ita,ita_end,itb,itb_end,env,new_coord);
      return ;
    }
    if (a<b){
      Mulmodpolymod(itb,itb_end,ita,ita_end,env,new_coord,intcoeff,taille,seuil_kara,maxdeg);
      return;
    }
    int mid=(a+1)/2;
    modpoly::const_iterator ita_mid=ita_end-mid;
    if (mid>=b){ // cut A in a/b+1 parts
      int nslices=a/b; // number of submultiplications -1
      ita_mid=ita+b;
      int maxdeg_shift = ita_end-ita_mid;
      Mulmodpolymod(itb,itb_end,ita,ita_mid,env,new_coord,intcoeff,taille,seuil_kara,maxdeg-maxdeg_shift); // initialization
      modpoly low;
      low.reserve(b*b);
      for (int i=1;i<nslices;i++){
	ita=ita_mid;
	ita_mid=ita_mid+b;
	shiftmodpoly(new_coord,b);
	maxdeg_shift -= b;
	Mulmodpolymod(itb,itb_end,ita,ita_mid,env,low,intcoeff,taille,seuil_kara,maxdeg-maxdeg_shift);
	addmodpoly(new_coord,low,env,new_coord);
      }
      // last multiplication
      mid=a%b;
      if (mid){
	shiftmodpoly(new_coord,mid);
	Mulmodpolymod(itb,itb_end,ita_mid,ita_end,env,low,intcoeff,taille,seuil_kara,maxdeg);
	addmodpoly(new_coord,low,env,new_coord);	
      }
      return ;
    }
    // A and B have comparable sizes.
    bool same=ita==itb && ita_end==itb_end;
    // cut A and B in two parts
    // A=A_low+x^mid*A_high, B=B_low+x^mid*B_high
    // A*B = A_low*B_low + x^[2*mid]* A_high*B_high
    //     + x^mid* [ (A_low+A_high)*(B_low+B_high)-A_low*B_low-A_high*B_high ]
    modpoly lowlow, Aplus, Bplus, lowhigh;
    modpoly::const_iterator itb_mid=itb_end-mid;
    lowlow.reserve(3*mid);
    Mulmodpolymod(ita_mid,ita_end,itb_mid,itb_end,env,lowlow,intcoeff,taille,seuil_kara,RAND_MAX);
    // If 2*mid is about maxdeg, 
    // A*B=A_low*B_low+x^mid*(A_low*B_hig+A_hig*B_low)+x^(2*mid)*A_hig*B_hig
    if (mid>=maxdeg/2-4){
      Mulmodpolymod(ita,ita_mid,itb,itb_mid,env,new_coord,intcoeff,taille,seuil_kara,maxdeg-2*mid); 
      Mulmodpolymod(ita,ita_mid,itb_mid,itb_end,env,Aplus,intcoeff,taille,seuil_kara,maxdeg-mid);
      Mulmodpolymod(ita_mid,ita_end,itb,itb_mid,env,Bplus,intcoeff,taille,seuil_kara,maxdeg-mid);
      addmodpoly(Aplus,Bplus,env,Aplus);
      shiftmodpoly(new_coord,mid);
      addmodpoly(new_coord,Aplus,env,new_coord);
      shiftmodpoly(new_coord,mid);
      addmodpoly(new_coord,lowlow,env,new_coord);
      trim_inplace(new_coord);
      return;
    }
    // COUT << "lowlow" << lowlow << '\n';
    // new_coord.reserve(2*mid);
    Mulmodpolymod(ita,ita_mid,itb,itb_mid,env,new_coord,intcoeff,taille,seuil_kara,RAND_MAX);
#if 0
    if (same){ 
      // (a+bx)^2=a^2+2*a*b*x+b^2*x^2, slower because a*b is not a square
      // a^2+b^2*x^2+((a+b)^2-a^2-b^2)*x is faster
      mergemodpoly(new_coord,lowlow,2*mid);
      Mulmodpolymod(ita,ita_mid,ita_mid,ita_end,env,lowhigh,intcoeff,taille,seuil_kara,RAND_MAX);
      mulmodpoly(lowhigh,2,lowhigh);
      shiftmodpoly(lowhigh,mid);
      addmodpoly(new_coord,lowhigh,env,new_coord);
      return;
    }
#endif
    // COUT << "new_coord" << new_coord << '\n';
    lowhigh.reserve(3*mid);
    Addmodpoly(ita,ita_mid,ita_mid,ita_end,env,Aplus);
    modpoly::const_iterator itap=Aplus.begin(),itap_end=Aplus.end();
    if (same){
      Mulmodpolymod(itap,itap_end,itap,itap_end,env,lowhigh,intcoeff,taille,seuil_kara,RAND_MAX);
    }
    else {
      Addmodpoly(itb,itb_mid,itb_mid,itb_end,env,Bplus);
      modpoly::const_iterator itbp=Bplus.begin(),itbp_end=Bplus.end();
      Mulmodpolymod(itap,itap_end,itbp,itbp_end,env,lowhigh,intcoeff,taille,seuil_kara,RAND_MAX);
    }
    // COUT << "lowhigh" << lowhigh << '\n';
    submodpoly(lowhigh,new_coord,env,lowhigh);
    mergemodpoly(new_coord,lowlow,2*mid);
#if 0
    submodpoly(lowhigh,lowlow,env,lowhigh);
    shiftmodpoly(lowhigh,mid);
    addmodpoly(new_coord,lowhigh,env,new_coord);
#else
    submodpoly(lowhigh,lowlow,env,lowlow);
    // COUT << "lowh-hh-ll" << lowlow << '\n';
    shiftmodpoly(lowlow,mid);
    addmodpoly(new_coord,lowlow,env,new_coord);
#endif
    // modpoly verif;
    // Muldense_POLY1(ita,ita_end,itb,itb_end,env,verif);
    // COUT << "newcoord" << new_coord << "=?" << verif << '\n';
  }


  inline void Muldensemodpolysmall(const modpoly::const_iterator & ita0,const modpoly::const_iterator & ita_end,const modpoly::const_iterator & itb0,const modpoly::const_iterator & itb_end,environment * env,modpoly & new_coord){  
    new_coord.clear();
    if (ita0==ita_end || itb0==itb_end) return;
    modpoly::const_iterator ita_begin=ita0,ita=ita0,itb=itb0;
    for ( ; ita!=ita_end; ++ita ){
      modpoly::const_iterator ita_cur=ita,itb_cur=itb;
      int res=0;
      for (;itb_cur!=itb_end;--ita_cur,++itb_cur) {
	res += ita_cur->val * itb_cur->val ;
	if (ita_cur==ita_begin)
	  break;
      }
      if (env && env->moduloon)
	new_coord.push_back(smod(res,env->modulo.val));
      else
	new_coord.push_back(res);
    }
    --ita;
    ++itb;
    for ( ; itb!=itb_end;++itb){
      int res= 0;
      modpoly::const_iterator ita_cur=ita,itb_cur=itb;
      for (;;) {
	res += ita_cur->val * itb_cur->val ;
	if (ita_cur==ita_begin)
	  break;
	--ita_cur;
	++itb_cur;
	if (itb_cur==itb_end)
	  break;
      }
      if (env && env->moduloon)
	new_coord.push_back(smod(res,env->modulo.val));
      else
	new_coord.push_back(res);
    }
  }

  static void Mulmodpolysmall(modpoly::const_iterator & ita,modpoly::const_iterator & ita_end,modpoly::const_iterator & itb,modpoly::const_iterator & itb_end,environment * env,modpoly & new_coord){
    int a=int(ita_end-ita);
    int b=int(itb_end-itb);
    if (!b)
      return ;
    if ( ( a <= INT_KARAMUL_SIZE) || ( b <= INT_KARAMUL_SIZE) ){
      Muldensemodpolysmall(ita,ita_end,itb,itb_end,env,new_coord);
      return ;
    }
    if (a<b){
      Mulmodpolysmall(itb,itb_end,ita,ita_end,env,new_coord);
      return;
    }
    int mid=(a+1)/2;
    modpoly::const_iterator ita_mid=ita_end-mid;
    if (mid>=b){ // cut A in a/b+1 parts
      int nslices=a/b; // number of submultiplications -1
      ita_mid=ita+b;
      Mulmodpolysmall(itb,itb_end,ita,ita_mid,env,new_coord); // initialization
      modpoly low;
      low.reserve(2*b);
      for (int i=1;i<nslices;i++){
	ita=ita_mid;
	ita_mid=ita_mid+b;
	shiftmodpoly(new_coord,b);
	Mulmodpolysmall(itb,itb_end,ita,ita_mid,env,low);
	addmodpoly(new_coord,low,env,new_coord);
      }
      // last multiplication
      mid=a%b;
      if (mid){
	shiftmodpoly(new_coord,mid);
	Mulmodpolysmall(itb,itb_end,ita_mid,ita_end,env,low);
	addmodpoly(new_coord,low,env,new_coord);	
      }
      return ;
    }
    // cut A and B in two parts
    // A=A_low+x^mid*A_high, B=B_low+x^mid*B_high
    // A*B = A_low*B_low + x^[2*mid]* A_high*B_high
    //     + x^mid* [ (A_low+A_high)*(B_low+B_high)-A_low*B_low-A_high*B_high ]
    modpoly lowlow, Aplus, Bplus, lowhigh;
    modpoly::const_iterator itb_mid=itb_end-mid;
    lowlow.reserve(3*mid);
    Mulmodpolysmall(ita_mid,ita_end,itb_mid,itb_end,env,lowlow);
    // COUT << "lowlow" << lowlow << '\n';
    // new_coord.reserve(2*mid);
    Mulmodpolysmall(ita,ita_mid,itb,itb_mid,env,new_coord);
    // COUT << "new_coord" << new_coord << '\n';
    lowhigh.reserve(2*mid);
    Addmodpoly(ita,ita_mid,ita_mid,ita_end,env,Aplus);
    Addmodpoly(itb,itb_mid,itb_mid,itb_end,env,Bplus);
    modpoly::const_iterator itap=Aplus.begin(),itap_end=Aplus.end();
    modpoly::const_iterator itbp=Bplus.begin(),itbp_end=Bplus.end();
    Mulmodpolysmall(itap,itap_end,itbp,itbp_end,env,lowhigh);
    // COUT << "lowhigh" << lowhigh << '\n';
    submodpoly(lowhigh,new_coord,env,lowhigh);
    mergemodpoly(new_coord,lowlow,2*mid);
    submodpoly(lowhigh,lowlow,env,lowlow);
    // COUT << "lowh-hh-ll" << lowlow << '\n';
    shiftmodpoly(lowlow,mid);
    addmodpoly(new_coord,lowlow,env,new_coord);
  }

  // Warning: mulmodpoly assumes that coeff are integers
  void mulmodpoly(const modpoly & a, const modpoly & b, environment * env,modpoly & new_coord,int maxdeg){
    if (a.empty() || b.empty()){
      new_coord.clear();
      return;
    }
    int as=int(a.size())-1;
    int bs=int(b.size())-1;
    if (!as){
      mulmodpoly(b,a.front(),env,new_coord);
      return;
    }
    if (!bs){
      mulmodpoly(a,b.front(),env,new_coord);
      return;
    }
    int product_deg=as+bs;
    if (&a==&new_coord){
      vecteur tmp;
      mulmodpoly(a,b,env,tmp,maxdeg);
      swap(tmp,new_coord);
      return;
      // setsizeerr(gettext("modpoly.cc/mulmodpoly"));
    }
    new_coord.reserve(product_deg+1);
    modpoly::const_iterator ita=a.begin(),ita_end=a.end(),itb=b.begin(),itb_end=b.end(); // ,ita_begin=a.begin()
    if ( env && (env->moduloon) && is_zero(env->coeff) && !env->complexe && (env->modulo.type==_INT_) && (env->modulo.val < smallint) && (product_deg < 65536) )
      Mulmodpolysmall(ita,ita_end,itb,itb_end,env,new_coord);
    else {
      // test for fft should perhaps take care of the size of env->modulo
      if ( (1 ||
	    (!env || !env->moduloon || env->modulo.type==_INT_) 
	    )
	   && as>=FFTMUL_SIZE && bs>=FFTMUL_SIZE
	   ){
	// Check that all coeff are integers
	for (;ita!=ita_end;++ita){
	  if (!ita->is_integer())
	    break;
	}
	for (;itb!=itb_end;++itb){
	  if (!itb->is_integer())
	    break;
	}
	if (ita==ita_end && itb==itb_end){
	  //CERR << "// fftmult" << '\n';
	  if (fftmult(a,b,new_coord,(env && env->moduloon && is_zero(env->coeff) && env->modulo.type==_INT_)?env->modulo.val:0,RAND_MAX)){
#if 0
	    vecteur save=new_coord;
	    Muldense_POLY1(a.begin(),ita_end,b.begin(),itb_end,env,new_coord,0,maxdeg);
	    if (save!=new_coord)
	      CERR << " fft mult error poly1" << a << "*" << b << ";" << (env && env->moduloon && is_zero(env->coeff)?env->modulo:zero) << '\n';
#endif
	    if (env && env->moduloon && env->modulo.type!=_INT_)
	      smod(new_coord,env->modulo,new_coord);
	    return ;
	  }
	}
	ita=a.begin();
	itb=b.begin();
      }
      int taille=0;//sizeinbase2(a)+sizeinbase2(b);
      if ((as<=KARAMUL_SIZE) && (bs<=KARAMUL_SIZE))
	Muldense_POLY1(ita,ita_end,itb,itb_end,env,new_coord,taille,maxdeg);
      else
	Mulmodpolymod(ita,ita_end,itb,itb_end,env,new_coord,true,taille,KARAMUL_SIZE,maxdeg);
    }
  }
  

  modpoly operator_times(const modpoly & a, const modpoly & b,environment * env) {
    // Multiplication
    // COUT << a <<"*" << b << "[" << modulo << "]" << '\n';
    if (a.empty())
      return a;
    if (b.empty())
      return b;
    modpoly new_coord;
    operator_times(a,b,env,new_coord);
    // COUT << new_coord << '\n';
    return new_coord;
  }

  modpoly unmod(const modpoly & a,const gen & m){
    modpoly res(a);
    iterateur it=res.begin(),itend=res.end();
    for (;it!=itend;++it){
      if (is_integer(*it))
	continue;
      if (it->type!=_MOD || *(it->_MODptr+1)!=m)
	return modpoly(1,gensizeerr("Can not convert "+it->print(context0)+" mod "+m.print(context0)));
      *it=*it->_MODptr;
    }
    return res;
  }

  bool unext(const modpoly & a,const gen & pmin,modpoly & res){
    res=a;
    iterateur it=res.begin(),itend=res.end();
    for (;it!=itend;++it){
      gen g=*it;
      if (g.type==_FRAC)
	return false;
      if (g.type==_EXT){
	if (*(g._EXTptr+1)!=pmin)
	  return false;
	g=*g._EXTptr;
	if (g.type==_VECT)
	  g.subtype=_POLY1__VECT;
	*it=g;
      }
    }
    return true;
  }

  void ext(modpoly & res,const gen & pmin){
    iterateur it=res.begin(),itend=res.end();
    for (;it!=itend;++it){
      *it=ext_reduce(*it,pmin);
    }
  }

  void modularize(modpoly & a,const gen & m){
    iterateur it=a.begin(),itend=a.end();
    for (;it!=itend;++it){
      *it=makemod(*it,m);
    }
  }

  void mulmodpoly_naive(modpoly::const_iterator ita,modpoly::const_iterator ita_end,modpoly::const_iterator itb,modpoly::const_iterator itb_end,environment * env,modpoly & new_coord){
    new_coord.clear();
    if (ita==ita_end || itb==itb_end)
      return;
    modpoly::const_iterator ita_begin=ita;
    if (ita==itb && ita_end==itb_end){
      // square polynomial
      // CERR << "square size " << ita_end-ita << '\n';
      for ( ; ita!=ita_end; ++ita ){
	modpoly::const_iterator ita_cur=ita,itb_cur=itb;
	gen res;
	for (;itb_cur<ita_cur;--ita_cur,++itb_cur) {
	  type_operator_plus_times(*ita_cur,*itb_cur,res);	  
	}
	if (res.type==_VECT && res.ref_count()==1) mulmodpoly(*res._VECTptr,2,*res._VECTptr); else 
	  res = 2*res;
	if (itb_cur==ita_cur)
	  type_operator_plus_times(*ita_cur,*itb_cur,res);
	new_coord.push_back(res);	
      }
      --ita;
      ++itb;
      for ( ; itb!=itb_end;++itb){
	modpoly::const_iterator ita_cur=ita,itb_cur=itb;
	gen res;
	for (;itb_cur<ita_cur;--ita_cur,++itb_cur) {
	  type_operator_plus_times(*ita_cur,*itb_cur,res);	  
	}
	if (res.type==_VECT && res.ref_count()==1) mulmodpoly(*res._VECTptr,2,*res._VECTptr); else 
	  res = 2*res;
	if (itb_cur==ita_cur)
	  type_operator_plus_times(*ita_cur,*itb_cur,res);
	new_coord.push_back(res);	
      }
      return;
    }
    // CERR << "non square size " << ita_end-ita << '\n';
    for ( ; ita!=ita_end; ++ita ){
      modpoly::const_iterator ita_cur=ita,itb_cur=itb;
      gen res;
      for (;;) {
	type_operator_plus_times(*ita_cur,*itb_cur,res);
	//res += (*ita_cur)*(*itb_cur); // res = res + (*ita_cur) * (*itb_cur);
	if (ita_cur==ita_begin)
	  break;
	--ita_cur;
	++itb_cur;
	if (itb_cur==itb_end)
	  break;
      }
      new_coord.push_back(res);	
    }
    --ita;
    ++itb;
    for ( ; itb!=itb_end;++itb){
      modpoly::const_iterator ita_cur=ita,itb_cur=itb;
      gen res;
      for (;;) {
	type_operator_plus_times(*ita_cur,*itb_cur,res);
	//res += (*ita_cur)*(*itb_cur);
	if (ita_cur==ita_begin)
	  break;
	--ita_cur;
	++itb_cur;
	if (itb_cur==itb_end)
	  break;
      }
      new_coord.push_back(res);	
    }
  }

  void mulmodpoly_kara_naive(const modpoly & a, const modpoly & b,environment * env,modpoly & new_coord,int seuil_kara){
    modpoly::const_iterator ita=a.begin(),ita_end=a.end(),itb=b.begin(),itb_end=b.end();
    Mulmodpolymod(ita,ita_end,itb,itb_end,env,new_coord,false,0,seuil_kara,RAND_MAX); // sizeinbase2(a)+sizeinbase2(b));
  }

  void mulmodpoly(const vector<modpoly *> & v,environment * env,modpoly & res,int dbg){
    int s=v.size();
    if (s==0){
      res.clear();
      return;
    }
    if (s==1){
      if (&res!=v[0])
	res=*v[0];
      return;
    }
    if (s==2){
      mulmodpoly(*v[0],*v[1],env,res);
      return;
    }
    if (s==3){
      modpoly tmp;
      mulmodpoly(*v[0],*v[1],env,tmp);
      mulmodpoly(tmp,*v[2],env,res);
      return;
    }
    if (dbg) CERR << CLOCK()*1e-6 << " begin mulmodpoly final degree " << s << "\n";
    modpoly tmp1,tmp2;
    vector<modpoly *> v1(v.begin(),v.begin()+s/2);
    vector<modpoly *> v2(v.begin()+s/2,v.end());
    mulmodpoly(v1,env,tmp1,dbg);
    mulmodpoly(v2,env,tmp2,dbg);
    mulmodpoly(tmp1,tmp2,env,res);      
    if (dbg) CERR << CLOCK()*1e-6 << " end mulmodpoly final degree " << s << "\n";
  }

  // return true if v empty
  bool trim(modpoly & v){
    iterateur it=v.begin(),itend=v.end();
    for (;it!=itend;++it){
      if (*it!=0)
	break;
    }
    if (it!=v.begin())
      v.erase(v.begin(),it);
    return v.empty();
  }

  // exchange outer and inner variable in source
  void reorder(const modpoly & source,modpoly & target){
    int ts=0,ss=int(source.size());
    modpoly::const_iterator it=source.begin(),itend=source.end();
    for (;it!=itend;++it)
      ts=giacmax(ts,it->type==_VECT?int(it->_VECTptr->size()):1);
    target.resize(ts);
    for (int i=0;i<ts;++i)
      target[i]=gen(vecteur(ss),_POLY1__VECT);
    for (int j=0;j<ss;++j){
      gen g=source[j];
      if (g.type!=_VECT){
	(*target[0]._VECTptr)[j]=g;
	continue;
      }
      vecteur & v =*g._VECTptr;
      int vs=int(v.size());
      int shift=ts-vs;
      for (int i=0;i<vs;++i){
	(*target[i+shift]._VECTptr)[j]=v[i];
      }
    }
    for (int i=0;i<ts;++i){
      if (trim(*target[i]._VECTptr))
	target[i]=0;
    }
  }

  // recursive 2d to 1d, inner variable must be of degree<n
  bool to1d(const modpoly & p,modpoly & q,int n){
    int ps=int(p.size());
    q.reserve(ps*n);
    for (int i=0;i<ps;++i){
      gen pi=p[i];
      if (pi.type!=_VECT){
	for (int j=1;j<n;++j)
	  q.push_back(0);
	q.push_back(pi);
	continue;
      }
      vecteur & v = *pi._VECTptr;
      int vs=int(v.size());
      if (vs>n) return false;
      for (int j=vs;j<n;++j)
	q.push_back(0);
      for (int j=0;j<vs;++j)
	q.push_back(v[j]);
    }
    return true;
  }

  void from1d(const modpoly & p,modpoly &q,int n){
    int ps = int(p.size());
    q.clear();
    q.reserve((ps+n-1)/n);
    int r=ps%n;
    vecteur tmp;
    tmp.reserve(n);
    const_iterateur it=p.begin(),itend=p.end();
    for (;r>0;++it,--r){
      tmp.push_back(*it);
    }
    trim(tmp);
    if (!tmp.empty())
      q.push_back(tmp);
    for (;it!=itend;){
      tmp.clear();
      for (r=n;r>0;++it,--r){
	tmp.push_back(*it);
      }
      trim(tmp);
      q.push_back(tmp.empty()?0:(tmp.size()==1?tmp.front():tmp));
    }
  }

  // eval p[i] at x in q[i]
  void horner2(const modpoly & p,const gen & x,modpoly & q){
    int ps = int(p.size());
    q.resize(ps);
    for (int i=0;i<ps;++i){
      gen pi=p[i];
      if (pi.type!=_VECT)
	q[i]=pi;
      else
	q[i]=horner(*pi._VECTptr,x,context0);
    }
  }

  void mulmodpoly_interpolate(const modpoly & p,const modpoly & q,int n,modpoly & res){
    modpoly px,qx,pqx;
    vecteur X,Y;
    int rs=int(p.size()+q.size()-1);
    res.resize(rs);
    if (debug_infolevel) 
      CERR << CLOCK()*1e-6 << " mulmodpoly_interpolate horner " << '\n';
    for (int i=-n;i<=n;++i){
      X.push_back(i);
      if (debug_infolevel>1) 
	CERR << CLOCK()*1e-6 << " mulmodpoly_interpolate horner2 " << i << '\n';
      horner2(p,i,px);
      if (debug_infolevel>1) 
	CERR << CLOCK()*1e-6 << " mulmodpoly_interpolate mult " << '\n';
      if (&p==&q){
	mulmodpoly_kara_naive(px,px,0,pqx,20);
#if 0
	vecteur tmp; mulmodpoly(px,px,0,tmp); 	
	if (tmp!=pqx) {
	  ofstream of("bugfft");
	  of << "p:=" << gen(px,_POLY1__VECT) << ":;" << '\n';
	  of << "correct p2 " << gen(pqx,_POLY1__VECT) << ":;" << '\n';
	  of << "wront p2 " << gen(tmp,_POLY1__VECT) << ":;" << '\n';
	  tmp=pqx-tmp;
	  of << "difference" << tmp << '\n';
	}
#endif
      }
      else {
	horner2(q,i,qx);
	mulmodpoly_kara_naive(px,qx,0,pqx,20);
      }
      Y.push_back(pqx);
    }
    if (debug_infolevel) 
      CERR << CLOCK()*1e-6 << " mulmodpoly_interpolate reorder " << '\n';
    vecteur Yr;
    reorder(Y,Yr);
    if (debug_infolevel) 
      CERR << CLOCK()*1e-6 << " mulmodpoly_interpolate rebuild " << '\n';
    for (int i=0;i<rs;++i){
      vecteur y=gen2vecteur(Yr[i]);
      if (y.size()<2*n+1)
	y.insert(y.begin(),int(2*n+1-y.size()),0);
      interpolate_inplace(X,y,0);
      res[i]=y;
    }
    if (debug_infolevel) 
      CERR << CLOCK()*1e-6 << " mulmodpoly_interpolate end " << '\n';
  }

  void operator_times (const modpoly & a, const modpoly & b,environment * env,modpoly & new_coord,int maxdeg) {
#ifdef TIMEOUT
    control_c();
#endif
    if (ctrl_c || interrupted) { 
      interrupted = true; ctrl_c=false;
      new_coord=modpoly(1,gensizeerr(gettext("Stopped by user interruption."))); 
      return;
    }
    if (a.size()==1){
      mulmodpoly(b,a.front(),env,new_coord);
      return;
    }
    if (b.size()==1){
      mulmodpoly(a,b.front(),env,new_coord);
      return;
    }
    if (env && env->moduloon && is_zero(env->coeff)){
      mulmodpoly(a,b,env,new_coord,maxdeg);
      return ;
    }
    bool gf=has_gf_coeff(a) || has_gf_coeff(b);
#if 1
    if (gf){
      vector<int> A,B; int M=-1; gen x;
      int agf=gf_char2_vecteur2vectorint(a,A,x),bgf=gf_char2_vecteur2vectorint(b,B,x);
      if (agf>0){
	if (bgf==0 || agf==bgf)
	  M=agf;
      }
      else {
	if (agf==0 && bgf>0)
	  M=bgf;
      }
      if (M>0){
	vector<int> R;
	if (gf_char2_multpoly(A,B,R,M)){
	  gf_char2_vectorint2vecteur(R,new_coord,M,x);
	  return;
	}
      }
    }
    if (gf){
      vector< vector<int> > A, B,R;
      vector<int> apmin,bpmin; gen x;
      int ac=gf_vecteur2vectorvectorint(a,A,x,apmin);
      if (ac){
	int bc=gf_vecteur2vectorvectorint(b,B,x,bpmin);
	if (bc==ac && apmin==bpmin){
	  if (gf_multpoly(A,B,R,apmin,ac)){
	    gf_vectorvectorint2vecteur(R,new_coord,ac,apmin,x);
	    return;
	  }
	}
      }
    }
#endif
    modpoly::const_iterator ita=a.begin(),ita_end=a.end(),itb=b.begin(),itb_end=b.end();
#if 1
    if (ita->type==_DOUBLE_ || (ita->type==_CPLX && (ita->subtype==3 || ita->_CPLXptr->type==_DOUBLE_ || (ita->_CPLXptr+1)->type==_DOUBLE_) ) ) {
      std::vector< complex_double > af,bf;
      if (convert(a,af,true) && convert(b,bf,true)){
	bool real=is_real(a,context0) && is_real(b,context0);
	int as=int(a.size()),bs=int(b.size());
	int rs=as+bs-1;
	int logrs=sizeinbase2(rs);
	if (logrs>30) { new_coord=modpoly(1,gensizeerr("Degree too large")); return ;}
	int n=(1u<<logrs); double invn=1.0/n;
	reverse(af.begin(),af.end()); af.resize(n);
	reverse(bf.begin(),bf.end()); bf.resize(n);
	fft2(&af.front(),n,2*M_PI/n);
	fft2(&bf.front(),n,2*M_PI/n);
	for (int i=0;i<n;++i)
	  af[i] *= bf[i];
	fft2(&af.front(),n,-2*M_PI/n);
	af.resize(rs);
	reverse(af.begin(),af.end());
	new_coord.clear(); new_coord.reserve(rs);
	if (real){
	  for (int i=0;i<rs;++i)
	    new_coord.push_back(invn*af[i].real());
	}
	else {
	  for (int i=0;i<rs;++i)
	    new_coord.push_back(invn*af[i]);
	}
	return;
      }
    }
#endif
    // Check that all coeff of a b are integers
    for (;ita!=ita_end;++ita){
      if (ita->type==_EXT){
	gen pmin=*(ita->_EXTptr+1);
	modpoly aa,bb;
	if (&a==&b && unext(a,pmin,aa)){
#if 0
	  if (pmin.type==_VECT && to1d(aa,bb,2*pmin._VECTptr->size()-3)){
	    aa.clear();
	    mulmodpoly_kara_naive(bb,bb,env,aa,KARAMUL_SIZE);
	    //mulmodpoly(bb,bb,env,aa);
	    from1d(aa,new_coord,2*pmin._VECTptr->size()-3);
	    ext(new_coord,pmin);
	    return;
	  }
#endif
	  int n=-1;
	  if (pmin.type==_VECT)
	    n=int(pmin._VECTptr->size())-2;
	  if (n>0 && aa.size()>=512)
	    mulmodpoly_interpolate(aa,aa,n,new_coord);
	  else
	    mulmodpoly_kara_naive(aa,aa,env,new_coord,10);
	  ext(new_coord,pmin);
	  return;
	}
	if (unext(a,pmin,aa) && unext(b,pmin,bb)){
	  if (0 && (aa.size()>=20 || bb.size()>=20)){
	    modpoly A,B,C; // it's slower
	    reorder(aa,A);
	    reorder(bb,B);
	    mulmodpoly_kara_naive(A,B,env,C,8);
	    reorder(C,new_coord);
	  }
	  else
	    mulmodpoly_kara_naive(aa,bb,env,new_coord,10);
	  ext(new_coord,pmin);
	  return;
	}
      }
      if (ita->type==_MOD 
	  //&& (ita->_MODptr+1)->type==_INT_
	  ){
	environment e;
	e.modulo=*(ita->_MODptr+1);
	e.moduloon=true;
	mulmodpoly(unmod(a,e.modulo),unmod(b,e.modulo),&e,new_coord,maxdeg);
	modularize(new_coord,e.modulo);
	return;
      }
      if (!ita->is_integer())
	break;
    }
    for (;itb!=itb_end;++itb){
      if (itb->type==_MOD 
	  //&& (itb->_MODptr+1)->type==_INT_
	  ){
	environment e;
	e.modulo=*(itb->_MODptr+1);
	e.moduloon=true;
	mulmodpoly(unmod(a,e.modulo),unmod(b,e.modulo),&e,new_coord,maxdeg);
	modularize(new_coord,e.modulo);
	return;
      }
      if (!itb->is_integer())
	break;
    }
    if (ita==ita_end && itb==itb_end){ // integer coefficients
      mulmodpoly(a,b,env,new_coord,maxdeg);
      return;
    }
    mulmodpoly_kara_naive(a,b,env,new_coord,KARAMUL_SIZE);
  }

  // res=(*it) * ... (*(it_end-1))
  void mulmodpoly(vector<modpoly>::const_iterator it,vector<modpoly>::const_iterator it_end,environment * env,modpoly & new_coord){
    int n=int(it_end-it);
    if (n>3){
      vector<modpoly>::const_iterator it_mid=it+(it_end-it)/2;
      modpoly first,second;
      mulmodpoly(it,it_mid,env,first);
      mulmodpoly(it_mid,it_end,env,second);
      mulmodpoly(first,second,env,new_coord);
      return ;
    }
    switch (n){
    case 0:
      return;
    case 1:
      new_coord=*it;
      return;
    case 2:
      operator_times(*it,*(it+1),env,new_coord);
      return;
    case 3:
      operator_times(*it,*(it+1),env,new_coord);
      new_coord=operator_times(*(it+2),new_coord,env);
      return ;
    }
  }

  void mulmodpoly(vector<modpoly>::const_iterator * it,int debut,int fin,environment * env,modpoly & pi){
    // pi = *(it[debut]);
    // for (int j=debut+1;j<=fin;j++){
    //  modpoly tmp;
    //  mulmodpoly(pi,*it[j],env,tmp);
    //  pi=tmp;
    // }
    //return ;
    if (fin-debut>2){
      int milieu=(debut+fin)/2;
      modpoly first,second;
      mulmodpoly(it,debut,milieu,env,first);
      mulmodpoly(it,milieu+1,fin,env,second);
      mulmodpoly(first,second,env,pi);
      return ;
    }
    switch (fin-debut){
    case 0:
      pi=*(it[debut]);
      break;
    case 1:
      operator_times(*(it[debut]),*(it[debut+1]),env,pi);
      break;
    case 2:
      operator_times(*(it[debut]),*(it[debut+1]),env,pi);
      pi=operator_times(pi,(*it[debut+2]),env);
      break;
    }
  }

  void negmodpoly(const modpoly & th, modpoly & new_coord){
    if (&th==&new_coord){
      modpoly::iterator a = new_coord.begin();
      modpoly::const_iterator a_end = new_coord.end();
      for (;a!=a_end;++a){
#ifndef USE_GMP_REPLACEMENTS
	if (a->type==_ZINT && a->ref_count()==1)
	  mpz_neg(*a->_ZINTptr,*a->_ZINTptr);
	else
#endif
	  *a=-(*a);
      }
    }
    else {
      new_coord.reserve(th.size());
      modpoly::const_iterator a = th.begin();
      modpoly::const_iterator a_end = th.end();
      for (;a!=a_end;++a)
	new_coord.push_back(-(*a));
    }
  }

  modpoly operator - (const modpoly & th) {  
    // Negate
    modpoly new_coord;
    negmodpoly(th,new_coord);
    return new_coord;
  }

  // right redimension poly to degree n
  void rrdm(modpoly & p, int n){
    int s=int(p.size());
    if (s==n+1)
      return;
    for (;s>n+1;--s){ // remove trainling coeff
      p.pop_back();
    }
    for (;s<n+1;++s){ // add zeros coeff
      p.push_back(0);
    }
  }

  // reduce mod env->modulo and trim. (T=int or longlong)
  void trim_inplace(vector<longlong> & p,longlong modulo){
    if (p.empty())
      return ;
    vector<longlong>::iterator it=p.begin(),itend=p.end();
    while ( (it!=itend) && (*it % modulo==0) )
      ++it;
    vector<longlong>::iterator it1=it;
    for (;it1!=itend;++it1){
      *it1=smodll(*it1,modulo);
    }
    p.erase(p.begin(),it);
  }

  void fast_trim_inplace(vector<longlong> & p,longlong modulo){
    if (p.empty())
      return ;
    vector<longlong>::iterator it=p.begin(),itend=p.end();
    while ( (it!=itend) && (*it==0 || *it % modulo==0) )
      ++it;
    p.erase(p.begin(),it);
  }

  void trim_inplace(vector<int> & p,int modulo){
    if (p.empty())
      return ;
    vector<int>::iterator it=p.begin(),itend=p.end();
    while ( (it!=itend) && (*it==0 || *it % modulo==0) )
      ++it;
    vector<int>::iterator it1=it;
    for (;it1!=itend;++it1){
      *it1=smod(*it1,modulo);
    }
    p.erase(p.begin(),it);
  }

  void fast_trim_inplace(vector<int> & p,int modulo,int maxsize){
    if (p.empty())
      return ;
    vector<int>::iterator it=p.begin(),itend=p.end();
    if (maxsize>=0 && maxsize<itend-it)
      it = itend-maxsize;
    while ( (it!=itend) && (*it==0 || *it % modulo==0) )
      ++it;
    p.erase(p.begin(),it);
  }

  // reduce mod env->modulo and trim.
  void trim_inplace(modpoly & p,environment * env){
    if (p.empty())
      return ;
    modpoly::iterator it=p.begin(),itend=p.end();
    if (env && env->moduloon){
      if (env->modulo.type==_ZINT){
	mpz_t &mo=*env->modulo._ZINTptr;
	for (;it!=itend;++it){
	  if (it->type==_ZINT && it->ref_count()==1){
	    mpz_t & m=*it->_ZINTptr;
	    mpz_mod(m,m,mo); // not smod-ed
	    if (mpz_cmp_si(m,0)!=0)
	      break;
	  }
	  else {
	    if (!is_zero(smod(*it,env->modulo)))
	      break;
	  }
	}
      }
      else {
	while ( (it!=itend) && (is_zero(smod(*it,env->modulo))) )
	  ++it;
      }
    }
    else
      while ( (it!=itend) && (is_zero(*it)) )
	++it;
    if (env && env->moduloon){
      modpoly::iterator it1=it;
      if (env->modulo.type==_ZINT){
	mpz_t &mo=*env->modulo._ZINTptr;
	mpz_t mo2; mpz_init_set(mo2,mo); mpz_tdiv_q_2exp(mo2,mo2,1);
	for (;it1!=itend;++it1){
	  if (it1->type==_ZINT && it1->ref_count()==1){
	    mpz_t & m=*it1->_ZINTptr;
	    mpz_mod(m,m,mo); // not smod-ed
	    if (mpz_cmp(m,mo2)>0)
	      mpz_sub(m,m,mo);
	    if (mpz_sizeinbase(m,2)<32)
	      *it1=mpz_get_si(m);
	  }
	  else
	    *it1=smod(*it1,env->modulo);
	}
	mpz_clear(mo2);
      }
      else {
	for (;it1!=itend;++it1){
	  *it1=smod(*it1,env->modulo);
	}
      }
    }
    p.erase(p.begin(),it);
  }

  modpoly trim(const modpoly & p,environment * env){
    if (p.empty())
      return p;
    modpoly::const_iterator it=p.begin(),itend=p.end();
    if (env && env->moduloon)
      while ( (it!=itend) && (is_zero(smod(*it,env->modulo))) )
	++it;
    else
      while ( (it!=itend) && (is_zero(*it)) )
	++it;
    modpoly new_coord ;
    if (env && env->moduloon)
      for (;it!=itend;++it)
	new_coord.push_back(smod(*it,env->modulo));
    else
      for (;it!=itend;++it)
	new_coord.push_back(*it);
    return new_coord;
  }

  void trim_inplace(modpoly & p){
    modpoly::iterator it=p.begin(),itend=p.end();
    while ( (it!=itend) && (is_zero(*it)) )
      ++it;
    if (it!=p.begin())
      p.erase(p.begin(),it);
  }

  void divmodpoly(const modpoly & th, const gen & fact, modpoly & new_coord){
    if (is_one(fact)){
      if (&th!=&new_coord)
	new_coord=th;
      return ;
    }
    if (fact.type==_USER || fact.type==_EXT){
      gen invfact=inv(fact,context0);
      mulmodpoly(th,invfact,new_coord);
      return;
    }
    if (&th==&new_coord){
      modpoly::iterator it=new_coord.begin(),itend=new_coord.end();
      for (;it!=itend;++it)
	//  *it =iquo(*it,fact);
	*it=rdiv(*it,fact,context0);
    }
    else {
      modpoly::const_iterator it=th.begin(),itend=th.end();
      for (;it!=itend;++it)
	new_coord.push_back(rdiv(*it,fact,context0)); // was iquo
      // new_coord.push_back(iquo(*it,fact));
    }
  }

  void iquo(modpoly & th,const gen & fact){
    modpoly::iterator it=th.begin(),itend=th.end();
#if !defined USE_GMP_REPLACEMENTS && !defined BF2GMP_H
    if (fact.type==_INT_ && fact.val<0){
      iquo(th,-fact);
      negmodpoly(th,th);
      return;
    }
    if (fact.type==_INT_ ){
      for (;it!=itend;++it){
	if (it->type==_ZINT && it->ref_count()==1)
	  mpz_tdiv_q_ui(*it->_ZINTptr,*it->_ZINTptr,fact.val);
	else {
	  if (it->type==_POLY){
	    polynome copie(*it->_POLYptr);
	    copie /= fact;
	    *it=copie;
	  }
	  else
	    *it=iquo(*it,fact); 
	}
      }
      return;
    }
    if (fact.type==_ZINT){
      for (;it!=itend;++it){
	if (it->type==_ZINT && it->ref_count()==1)
	  mpz_tdiv_q(*it->_ZINTptr,*it->_ZINTptr,*fact._ZINTptr);
	else
	  *it=iquo(*it,fact); 
      }
      return;
    }
#endif
    for (;it!=itend;++it)
      *it=iquo(*it,fact); 
  }

  void divmodpoly(const modpoly & th, const gen & fact, environment * env,modpoly & new_coord){
    if (is_one(fact)){
      if (&th!=&new_coord)
	new_coord=th;
      return ;
    }
    if (!env || !env->moduloon || !is_zero(env->coeff))
      divmodpoly(th,fact,new_coord);
    else {
      gen factinv(invmod(fact,env->modulo));
      mulmodpoly(th,factinv,env,new_coord);
    }
  }

  modpoly operator / (const modpoly & th,const gen & fact ) {  
    if (is_one(fact))
      return th;
    modpoly new_coord;
    divmodpoly(th,fact,new_coord);
    return new_coord;
  }

  modpoly operator_div (const modpoly & th,const gen & fact,environment * env ) {  
    if (is_one(fact))
      return th;
    modpoly new_coord;
    divmodpoly(th,fact,env,new_coord);
    return new_coord;
  }

  // fast div rem http://www.csd.uwo.ca/~moreno/CS424/Lectures/FastDivisionAndGcd.html/node3.html
  // fast modular inverse: f*g=1 mod x^l
  bool invmod(const modpoly & f,int l,environment * env,modpoly & g){
    if (f.empty())
      return false;
    gen finv=f.back();
    if (f.back()!=1){
      finv=invenv(finv,env);
      if (finv.type==_FRAC)
	return false;
      modpoly F;
      mulmodpoly(f,finv,env,F);
      if (!invmod(F,l,env,g))
	return false;
      mulmodpoly(g,finv,env,g);
      return true;
    }
    g=modpoly(1,1);
    for (longlong i=2;;){
      modpoly h,tmp1,tmp2;
      operator_times(g,g,env,h);
      if (h.size()>i)
	h=modpoly(h.end()-i,h.end());
      // g=plus_two*g-f*h;
      mulmodpoly(g,plus_two,env,tmp1);
      int taille=giacmin(i,l);
      if (taille>f.size())
	taille=f.size();
      modpoly F(f.end()-taille,f.end());
      operator_times(F,h,env,tmp2);
#if 0 // debug
      int fft_mult_save=FFTMUL_SIZE;
      FFTMUL_SIZE=1<<30;
      modpoly tmp3;
      operator_times(F,h,env,tmp3);
      if (tmp3!=tmp2)
	CERR << "Divquo/invmod error" << tmp3-tmp2 << '\n';
      FFTMUL_SIZE=fft_mult_save;
#endif
      submodpoly(tmp1,tmp2,env,g);
      if (g.size()>i)
	g=modpoly(g.end()-i,g.end());
      if (g.size()>l)
	g=modpoly(g.end()-l,g.end());
      g=trim(g,env);
      if (i>l) break;
      i=2*i;
    }
    return true;
  }

  // euclidean quotient using modular inverse
  int DivQuo(const modpoly & a, const modpoly & b, environment * env,modpoly & q){
    q.clear();
    int n=a.size(),m=b.size();
    if (n<m)
      return 1;
    int s=n-m+1;
    if (s>=FFTMUL_SIZE && m>=FFTMUL_SIZE && env && env->modulo.type==_INT_){
      int p=env->modulo.val,l=sizeinbase2(n);
      // check if p is a Fourier prime for n
      int N=1<<l;
      if ( (((p-1)>>l)<<l)==p-1 && is_probab_prime_p(p)){
	int w=nthroot(p,l);
	if (w){
	  vector<int> A,B,Wp,tmp0;
	  vecteur2vector_int(a,p,A);
	  vecteur2vector_int(b,p,B);
	  to_fft(A,p,w,Wp,N,tmp0,1,false,false); A.swap(tmp0);
	  to_fft(B,p,w,Wp,N,tmp0,1,false,false); B.swap(tmp0);
	  fft_aoverb_p(A,B,tmp0,p);
	  fft_reverse(Wp,p); 
	  from_fft(tmp0,p,Wp,A,true,false);
	  fast_trim_inplace(A,p);
	  if (A.size()==s){
	    vector_int2vecteur(A,q);
	    return 2;
	  }
	}
      }
    }
    modpoly f(b),g;
    reverse(f.begin(),f.end());
    if (!invmod(f,n-m+1,env,g))
      return 0;
    f=a;
    reverse(f.begin(),f.end());
    operator_times(f,g,env,q);
    if (q.size()>s)
      q=modpoly(q.end()-s,q.end());
    reverse(q.begin(),q.end());
    trim(q,env);
    return 1;
  }

  // for p prime such that p-1 is divisible by 2^N, compute a 2^N-th root of 1
  // otherwise return 0
 unsigned nthroot(unsigned p,unsigned N){
    unsigned expo=(p-1)>>N;
    if ( (expo<<N)!=p-1)
      return 0;
    for (unsigned n=2;;++n){
      int w=powmod(n,expo,p); // w=n^((p-1)/2^N)
      ulonglong r=w;
      for (unsigned i=1;i<N;++i)
	r=(r*r)%p;
      if (r==p-1) // r=w^(2^(N-1))=n^((p-1)/2)
	return w;
    }
  }

  int find_w(vector<int> & Wp,unsigned shift,unsigned p){
    unsigned n=1<<shift,w=0;
#if defined GIAC_PRECOND || defined GIAC_CACHEW
    int ws=Wp.size();
#else
    int ws=2*Wp.size();
#endif
    if (ws/n){
      w=Wp[ws/n];
      int wp=powmod(w,n/2,p);
      if (wp!=p-1){
	w=0; Wp.clear();
      }
      //CERR << Wp << endl;
    }
    if (w==0 && p!=p1 && p!=p2 && p!=p3)
      w=nthroot(p,shift);
    return w;
  }

  bool chk_equal_mod(const gen & a,longlong p,int m){
    if (a.type==_FRAC){
      int n=a._FRACptr->num.type==_ZINT?modulo(*a._FRACptr->num._ZINTptr,m):a._FRACptr->num.val;
      int d=a._FRACptr->den.type==_ZINT?modulo(*a._FRACptr->den._ZINTptr,m):a._FRACptr->den.val;
      return (n-longlong(p)*d)%m==0;
    }
    if (a.type==_ZINT)
      return (modulo(*a._ZINTptr,m)-p)%m==0;
    if (a.type==_INT_)
      return (a.val-p)%m==0;
    CERR << "Unknown type in reconstruction " << a << '\n';
    return false;
  }

  bool chk_equal_mod(const vecteur & a,const vecteur & p,int m){
    if (a.size()!=p.size())
      return false;
    const_iterateur it=a.begin(),itend=a.end(),jt=p.begin();
    for (;it!=itend;++jt,++it){
      if (it->type==_INT_ && *it==*jt) continue;
      if (jt->type!=_INT_ || !chk_equal_mod(*it,jt->val,m))
	return false;
    }
    return true;
  }

  bool chk_equal_mod(const vecteur & a,const vector<int> & p,int m){
    if (a.size()!=p.size())
      return false;
    const_iterateur it=a.begin(),itend=a.end();
    vector<int>::const_iterator jt=p.begin();
    for (;it!=itend;++jt,++it){
      if (it->type==_INT_ && it->val==*jt) continue;
      if (!chk_equal_mod(*it,*jt,m))
	return false;
    }
    return true;
  }


  inline int precond_mulmod31(int b1,int q1,int p,int q1surp){
    b1 += (b1>>31) &p;
    int t=longlong(b1)*q1-((longlong(b1)*q1surp)>>31)*p;
    t += (t>>31)&p; // t positive (or at least t-p is valid)
    return t;
  }

  // v *= m mod p
  void precond_mulmod31(vector<int> & v,int m,int p,int msurp){
    vector<int>::iterator it=v.begin(),itend=v.end();
    for (;it!=itend;++it){
      *it=precond_mulmod31(*it,m,p,msurp);
    }
  }

  void precond_mulmod31(vector<int> & v,int m,int p){
    m += (m>>31) &p;
    int msurp=((1LL<<31)*m)/p+1;
    msurp += (msurp>>31) & p;
    precond_mulmod31(v,m,p,msurp);
  }

  // invp is 1.0/p*(1.0-prec) < evalf(1/p) with a sufficient bias insuring r>=0
  inline int amodp(longlong a,int p,double invp){
    longlong q=a*invp; // q<=a/p, maximal relative error: prec+3*2^-53<2^-50
    int r= a-q*p; // max absolute error |a|*2^-50<=2^13, hence 0<=r<=p+2^13
    if (0 && a>0 && r<0)
      CERR << "err amodp a=" << a << " p=" << p << " r=" << r << "\n";
#ifndef GIAC_PRECOND
    r += (r>>31)&p; // this is not required if a>=0
#endif
    return r;
    if (r>p && r-p>= (1<<10) )//(r-(a%p)) %p!=0) // ((a-r)%p!=0)
      CERR << "err amodp a=" << a << " p=" << p << " r=" << r << "\n";
    return r; 
    return a%p;
  }

  inline int amodpplus(longlong a,int p,double invp){
    longlong q=a*invp; // q<=a/p, maximal relative error: prec+3*2^-53<2^-50
    int r= a-q*p; // max absolute error |a|*2^-50<=2^13, hence 0<=r<=p+2^13
    r += (r>>31)&p; // this is not required if a>=0
    return r;
  }
  // using apos_modp fails for n:=8000;a:=randpoly(n,3,[]):; b:=randpoly(n+2,3,[]):; ntl_on(false);time(r:=resultant(a,b));
  inline int apos_modp(longlong a,int p,double invp){
    longlong q=a*invp; // q<=a/p, maximal relative error: prec+3*2^-53<2^-50
    int r= a-q*p; // max absolute error |a|*2^-50<=2^13, hence 0<=r<=p+2^13
    return r;
  }

  void precond_mulmod_double(vector<int> & v,int m,int p,double invp){
    vector<int>::iterator it=v.begin(),itend=v.end();
    for (;it!=itend;++it){
      *it=amodp((*it)*longlong(m),p,invp);
    }
  }

  void precond_mulmod_double(vector<int> & v,int m,int p){
    double invp=find_invp(p);
    precond_mulmod_double(v,m,p,invp);
  }

  // Beware, precond_mulmod_double does not work if p is very near from 1ULL<<31
  inline void precond_mulmod(vector<int> & v,int m,int p){
    if (m==1)
      return;
#if 1 //def GIAC_PRECOND
    precond_mulmod31(v,m,p);
#else
    precond_mulmod_double(v,m,p);
#endif
  }

  // ab=a*b mod m, assumes that m is a Fourier prime for qi and ri
  // returns true if fft was used
  bool operator_times(const std::vector<int> & a,const std::vector<int> & b,int m,std::vector<int> & ab){
    if (a.size()<FFTMUL_SIZE || b.size()<FFTMUL_SIZE){
      smallmult(a.begin(),a.end(),b.begin(),b.end(),ab,m);
      return false;
    }
    vector<int> A,B,Wp,tmp0; 
    int l=sizeinbase2(a.size()+b.size()-1);
    int n=1<<l;
    int w=nthroot(m,l); 
    if (w==0){
      vector<int> resp1;
#if 1
      vector<int> resp2,resp3,Wp1,Wp2,Wp3,tmp_p,tmp_q;
      fft2mult(m,a,b,resp1,p1,Wp1,tmp_p,tmp_q,false,false,true);
      fft2mult(m,a,b,resp2,p2,Wp2,tmp_p,tmp_q,false,false,true);
      fft2mult(m,a,b,resp3,p3,Wp3,tmp_p,tmp_q,false,false,true);
      ichinremp1p2p3(resp1,resp2,resp3,n,resp1,m);
      reverse(resp1.begin(),resp1.end());
      ab.swap(resp1);
      return true;
#endif
      smallmult(a.begin(),a.end(),b.begin(),b.end(),ab,m);
      if (!resp1.empty() && !(resp1==ab))
	CERR << "bug\n";
      return false;
    }
    //smallmult(qi.begin(),qi.end(),bi.begin(),bi.end(),tmp1,m); // debug
    to_fft(a,m,w,Wp,n,tmp0,1,false,false); A.swap(tmp0);
    if (&a==&b)
      fft_ab_p(A,A,tmp0,m);
    else {
      to_fft(b,m,w,Wp,n,tmp0,1,false,false); B.swap(tmp0);
      fft_ab_p(A,B,tmp0,m);
    }
    fft_reverse(Wp,m); 
    from_fft(tmp0,m,Wp,ab,true,false);
    fast_trim_inplace(ab,m);
    return true;
  }
  
  // fast modular inverse: f*g=1 mod x^l
  bool invmod(const std::vector<int> & f,int l,int p,std::vector<int> & g){
    if (f.empty())
      return false;
    int finv=f.back() % p;
    finv += (finv>>31) & p;
    if (finv!=1){
      finv=invmod(finv,p);
      vector<int> F(f);
      precond_mulmod(F,finv,p);
      if (!invmod(F,l,p,g))
	return false;
      precond_mulmod(g,finv,p);
      return true;
    }
    g=vector<int>(1,1);
    for (longlong i=2;;){
      vector<int> h,tmp1;
      operator_times(g,g,p,h);
      if (h.size()>i)
	h=vector<int>(h.end()-i,h.end());
      // g=plus_two*g-f*h;
      tmp1=g;
      precond_mulmod(tmp1,2,p); // tmp1=2*g
      int taille=giacmin(i,l);
      if (taille>f.size())
	taille=f.size();
      vector<int> F(f.end()-taille,f.end());
      operator_times(F,h,p,g); 
      submodneg(g,tmp1,p); // g=tmp1-F*h
      if (g.size()>i)
	g=vector<int>(g.end()-i,g.end());
      if (g.size()>l)
	g=vector<int>(g.end()-l,g.end());
      fast_trim_inplace(g,p);
      if (i>l) break;
      i=2*i;
    }
    return true;
  }

  // euclidean quotient using modular inverse
  int DivQuo(const std::vector<int> & a, const std::vector<int> & b, int p,std::vector<int> & q,bool ck_exactquo){
    q.clear();
    int n=a.size(),m=b.size();
    if (n<m)
      return 1;
    int s=n-m+1;
    if (ck_exactquo && s>=FFTMUL_SIZE && m>=FFTMUL_SIZE){
      int l=sizeinbase2(n);
      // check if p is a Fourier prime for n
      int N=1<<l;
      if ( (((p-1)>>l)<<l)==p-1 && is_probab_prime_p(p)){
	int w=nthroot(p,l);
	if (w){
	  vector<int> A,B,Wp,tmp0;
	  to_fft(a,p,w,Wp,N,tmp0,1,false,false); A.swap(tmp0);
	  to_fft(b,p,w,Wp,N,tmp0,1,false,false); B.swap(tmp0);
	  if (fft_aoverb_p(A,B,tmp0,p)){
	    fft_reverse(Wp,p); 
	    from_fft(tmp0,p,Wp,q,true,false);
	    fast_trim_inplace(q,p);
	    if (q.size()==s)
	      return 2;
	  }
	}
      }
    }
    vector<int> f(b),g;
    reverse(f.begin(),f.end());
    if (!invmod(f,n-m+1,p,g))
      return 0;
    f=a;
    reverse(f.begin(),f.end());
    operator_times(f,g,p,q);
    if (q.size()>s)
      q=vector<int>(q.end()-s,q.end());
    reverse(q.begin(),q.end());
    fast_trim_inplace(q,p);
    return 1;
  }

  // reconstruct quo and rem by chinese remaindering
  // quo, rem are already computed for env->modulo
  bool divremrec(const modpoly & A, const modpoly & B, modpoly & quo, modpoly & rem,environment * env){
    gen M=mignotte_bound(A); // works for exact quotient only!
    gen B0=B[0];
    gen pip=env->modulo;
    gen p=pip;
    vecteur a,b,q,r,quo_,rem_;
    vector<int> ai,bi,qi,ri,qbi,tmp0,tmp1,Wp;
    unsigned long l=sizeinbase2(A.size())-1;
    unsigned long n=1<<(l+1);
    while (is_greater(M,pip,context0)){
      bool fourier_prime=false;
      for (;;){
	p=p-1;
	if (p.type==_INT_ && sizeinbase2(p)>l+8 && n<=(1<<22)){
	  p=prevprimep1p2p3(p.val-1,0,n);
	  fourier_prime=true;
	}
	else
	  p=prevprime(p);
	if (smod(B0,p)!=0)
	  break;
      }
      env->modulo=p; 
      // check that b*q+r==a before doing division
      if (fourier_prime){
	int m=p.val;
#if 1
	vecteur2vector_int(A,m,ai);
	vecteur2vector_int(B,m,bi);
	DivRem(ai,bi,m,qi,ri,rem.empty());
	if (chk_equal_mod(quo,qi,m) && chk_equal_mod(rem,ri,m)){
	  operator_times(quo,B,0,rem_);
	  addmodpoly(rem_,rem,rem_);
	  // add_mulmodpoly(quo_.begin(),quo_.end(),B.begin(),B.end(),0,rem);
	  if (rem_==A)
	    return true;
	}
	ichinrem_inplace(quo,qi,pip,m); 
	ichinrem_inplace(rem,ri,pip,m);
	pip=pip*p;
	continue;
#endif
	vecteur2vector_int(quo,m,qi);
	vecteur2vector_int(B,m,bi);
	operator_times(qi,bi,m,qbi);
	// fft_reverse(Wp,m); 
	vecteur2vector_int(rem,m,ri);
	addmod(qbi,ri,m);
	vecteur2vector_int(A,m,ai);
#if 0 // debug
	tmp0.clear();
	tmp0=tmp1; submod(tmp0,qbi,m); // debug
	if (!tmp0.empty())
	  CERR << "err\n";
	submod(tmp1,ai,m);  // debug
#endif
	submod(qbi,ai,m);
	if (qbi.empty()){
	  operator_times(quo,B,0,rem_);
	  addmodpoly(rem_,rem,rem_);
	  // add_mulmodpoly(quo_.begin(),quo_.end(),B.begin(),B.end(),0,rem);
	  if (rem_==A)
	    return true;
	}
	else {
	  DivRem(ai,bi,m,qi,ri);
	  //DivRem(A,B,env,q,r,false); // debug
	  ichinrem_inplace(quo,qi,pip,m); 
	  ichinrem_inplace(rem,ri,pip,m);
	}
	pip=pip*p;
	continue;
      }
      smod(A,p,a); smod(B,p,b);
      smod(quo,p,q); smod(rem,p,r);
      operator_times(b,q,env,rem_);
      addmodpoly(rem_,r,env,rem_);
      if (a==rem_){
	quo_=quo;
	rem_=rem;
      }
      else {
	DivRem(a,b,env,q,r,false);
	quo_=ichinrem(quo,q,pip,p);
	rem_=ichinrem(rem,r,pip,p);
      }
      if (quo==quo_ && rem==rem_){
	operator_times(quo_,B,0,rem_);
	addmodpoly(rem_,rem,rem_);
	// add_mulmodpoly(quo_.begin(),quo_.end(),B.begin(),B.end(),0,rem);
	if (rem_==A)
	  return true;
      }
      quo.swap(quo_);
      rem.swap(rem_);
      pip=pip*p;
    }
    return false;
  }

  // modular division
  bool DivRemInt(const modpoly & A, const modpoly & B, modpoly & quo, modpoly & rem){
    gen B0=B[0];
    // first try for exact quotient modulo a prime
    gen p = p1 ;
    while (smod(B,p)==0){
      p=prevprime(p-1);
    }
    vecteur a(A),b(B);
    smod(a,p,a); smod(b,p,b);
    environment env; env.modulo=p; env.moduloon=true;
    DivRem(a,b,&env,quo,rem,false);
    if (rem.empty()){
      // it is highly probable that the division is exact, 
      // reconstruct quo by Chinese remaindering
      if (divremrec(A,B,quo,rem,&env))
	return true;
    }
    // reconstruct both quo and rem of the pseudo-division
    gen Bb=pow(B0,A.size()-B.size()+1,context0);
    vecteur Apseudo(A);
    multvecteur(Bb,Apseudo,Apseudo);
    multvecteur(Bb,a,a);
    smod(a,p,a);
    multvecteur(Bb,quo,quo);
    smod(quo,p,quo);
    multvecteur(Bb,rem,rem);
    smod(rem,p,rem);
    if (!divremrec(Apseudo,B,quo,rem,&env))
      return false;
    Bb=inv(Bb,context0);
    multvecteur(Bb,quo,quo);
    multvecteur(Bb,rem,rem);
    return true;
  }

  int coefftype(const modpoly & v,gen & coefft){
    int t=0;
    const_iterateur it=v.begin(),itend=v.end();
    for (;it!=itend;++it){
      const unsigned char tmp=it->type;
      if (tmp==_INT_ || tmp==_ZINT)
	continue;
      t=tmp;
      coefft=*it;
      if (t==_USER)
	return t;
      if (t==_MOD)
	return t;
      if (t==_EXT)
	return t;
    }
    return t;
  }

  bool DivRem(const modpoly & th, const modpoly & other, environment * env,modpoly & quo, modpoly & rem,bool allowrational){
    // COUT << "DivRem" << th << "," << other << '\n';
    if (other.empty()){
#ifndef NO_STDEXCEPT
      setsizeerr(gettext("modpoly.cc/DivRem"));
#endif
      return false;
    } 
    if (th.empty()){
      quo=th;
      rem=th;
      return true ;
    }
    int a=int(th.size())-1;
    int b=int(other.size())-1;
    if (other.size()==1){
      divmodpoly(th,other.front(),env,quo);
      rem.clear();
      return true ;
    }
    quo.clear();
    if (a<b){
      rem=th;
      return true;
    }
    gen acoeff,bcoeff;
    int atype=coefftype(th,acoeff),btype=coefftype(other,bcoeff);
    if (atype==_MOD || btype==_MOD){
      environment e;
      e.modulo=atype==_MOD?*(acoeff._MODptr+1):*(bcoeff._MODptr+1);
      e.moduloon=true;
      if (!DivRem(unmod(th,e.modulo),unmod(other,e.modulo),&e,quo,rem,false))
	return false;
      modularize(quo,e.modulo);
      modularize(rem,e.modulo);
      return true;
    }
#if 1
    int divquores=0;
    if (env && env->moduloon &&
	other.size()>FFTMUL_SIZE && th.size()-other.size()>FFTMUL_SIZE){
      if (debug_infolevel>2)
	CERR << CLOCK()*1e-6 << " DivRem mod start" << endl;
      int l=sizeinbase2(other.size()),p=env->modulo.val;
      if (env->modulo.type==_INT_ && p-1==((p-1)>>l)<<l){
	vector<int> a,b,q,r;
	vecteur2vector_int(th,p,a);
	vecteur2vector_int(other,p,b);
	divquores=DivQuo(a,b,p,q,true); // check for exact quotient
	vector_int2vecteur(q,quo);
	if (debug_infolevel>2)
	  CERR << CLOCK()*1e-6 << " DivQuo mod Fourier prime end" << endl;
	rem.clear();
	if (divquores==2) 
	  return true;
	if (divquores){
	  operator_times(b,q,p,r);
	  submodneg(r,a,p);
	  vector_int2vecteur(r,rem);
	  if (debug_infolevel>2)
	    CERR << CLOCK()*1e-6 << " DivRem mod Fourier prime end" << endl;
	  return true;
	}
      }
      if ((divquores=DivQuo(th,other,env,quo))){
	rem.clear();
	if (divquores==2)
	  return true;
	modpoly tmp;
	operator_times(other,quo,env,tmp);
	submodpoly(th,tmp,env,rem);
	return true;
      }
    }
#endif
    if ( (env==0 || env->moduloon==false) && atype==0 && btype==0 && other.size()>FFTMUL_SIZE && th.size()-other.size()>FFTMUL_SIZE && DivRemInt(th,other,quo,rem) )
      return true;
    quo.reserve(a-b+1);
    // A=BQ+R -> A=(B*invcoeff)*Q+(R*invcoeff), 
    // make division of A*coeff by B*coeff and multiply R by coeff at the end
    gen coeff=other.front(),invcoeff; bool invother=false;
    if (coeff.type==_USER){
      invother=true;
      invcoeff=inv(coeff,context0);
    }
    if (coeff.type==_EXT){
      gen coeff0=*coeff._EXTptr;
      if (coeff0.type==_VECT){
	for (int i=0;i<coeff0._VECTptr->size();++i){
	  if ((*coeff0._VECTptr)[i].type==_USER){
	    invcoeff=inv(coeff,context0);
	    invother=true;
	    break;
	  }
	}
      }
    }
    if (!invother && env && env->moduloon){
      invcoeff=invmod(coeff,env->modulo);
      invother=true;
    }
    // copy rem to an array
    modpoly::const_iterator remit=th.begin(); // ,remend=rem.end();
    gen * tmp=new gen[a+1]; // must use new/delete
    gen * tmpend=&tmp[a];
    gen * tmpptr=tmpend; // tmpend points to the highest degree coeff of A
    /*
    vecteur vtmp(a+1);
    iterateur tmp=vtmp.begin();
    iterateur tmpend=vtmp.end()-1;
    iterateur tmpptr=tmpend; // tmpend points to the highest degree coeff of A
    */
    for (;tmpptr!=tmp-1;--tmpptr,++remit)
      *tmpptr=*remit;
    modpoly::const_iterator B_beg=other.begin(),B_end=other.end();
    mpz_t prod;
    mpz_init(prod);
    gen n0( 0),q,mod2(env?2*env->modulo:0);
    for (;a>=b;--a){
      if (invother){
	if (env && env->moduloon){
	  if (tmpend->type==_ZINT && invcoeff.type==_ZINT && env->modulo.type==_ZINT){
	    mpz_mul(prod,*tmpend->_ZINTptr,*invcoeff._ZINTptr);
	    mpz_fdiv_r(prod,prod,*env->modulo._ZINTptr); // prod positive
	    if (mpz_cmp(prod,*mod2._ZINTptr)>0)
	      mpz_sub(prod,prod,*env->modulo._ZINTptr);
	    q=prod;
	  }
	  else
	    q=smod(*tmpend*invcoeff,env->modulo);
	}
	else
	  q=*tmpend*invcoeff;
      }
      else {
	q=rdiv(*tmpend,coeff,context0);
	if (!allowrational){
	  if (q.type==_FRAC){
	    delete [] tmp;
	    return false;
	  }
	}
      }
      quo.push_back(q);
      --tmpend;
      bool fast=(env && is_zero(env->coeff) && (env->complexe || !env->moduloon) )?false:(q.type==_INT_) || (q.type==_ZINT);
      if (!is_zero(q)) {
	// tmp <- tmp - q *B.shifted
	tmpptr=tmpend;
	modpoly::const_iterator itq=B_beg;
	++itq; // first elements cancel
	if (env && (env->moduloon && !env->complexe && is_zero(env->coeff)) && (env->modulo.type==_INT_) && (env->modulo.val<smallint)){
	  for (;itq!=B_end;--tmpptr,++itq){ // no mod here to save comput. time
	    tmpptr->val -= q.val*itq->val ;
	  }	  
	}
	else {
	  mpz_set_si(prod,0);
	  for (;itq!=B_end;--tmpptr,++itq){ // no mod here to save comput. time
	    if (fast && (tmpptr->type==_ZINT) && 
#ifndef SMARTPTR64
		(tmpptr->__ZINTptr->ref_count==1) && 
#else
		((ref_mpz_t *) (* (ulonglong *) tmpptr >> 16))->ref_count==1 &&
#endif
		( (itq->type==_ZINT) || (itq->type==_INT_) ) )
	      sub_mul(tmpptr->_ZINTptr,prod,q,*itq);
	    else
	      *tmpptr = (*tmpptr)-q*(*itq) ;
	  }
	}
      }
      /*
      if (env && !env->moduloon) {
	CERR << quo << '\n';
	CERR << quo*other << '\n';
	CERR << "[";
	for (int i=1;i<a;++i)
	  CERR << tmp[a-i] << "," ;
	CERR << tmp[0] << "]" << '\n';
	CERR << '\n';
      }
      */
    } // end for (;;)
    // trim rem and multiply by coeff, this will modularize rem as well
    rem.clear();
    // bool trimming=true;
    if (env && env->moduloon){
      for (;tmpend!=tmp-1;--tmpend){
	if (tmpend->type==_ZINT && env->modulo.type==_ZINT){
	  mpz_fdiv_r(prod,*tmpend->_ZINTptr,*env->modulo._ZINTptr); // prod positive
	  if (mpz_cmp_si(prod,0))
	    break;
	}
	else {
	  if (!is_zero(smod(*tmpend,env->modulo)))
	    break;
	}
      }
      for (;tmpend!=tmp-1;--tmpend){
	if (tmpend->type==_ZINT && env->modulo.type==_ZINT){
	  mpz_fdiv_r(prod,*tmpend->_ZINTptr,*env->modulo._ZINTptr); // prod positive
	  if (mpz_cmp(prod,*mod2._ZINTptr)>0)
	    mpz_sub(prod,prod,*env->modulo._ZINTptr);
	  rem.push_back(prod);
	}
	else
	  rem.push_back(smod(*tmpend,env->modulo));
      }      
    }
    else {
      for (;tmpend!=tmp-1;--tmpend){
	if (!is_zero(*tmpend))
	  break;
      }
      for (;tmpend!=tmp-1;--tmpend){
	rem.push_back(*tmpend);
      }      
    }
    mpz_clear(prod);
    // COUT << "DivRem" << th << "-" << other << "*" << quo << "=" << rem << " " << th-other*quo << '\n';
    delete [] tmp;
    return true;
  }

  bool DenseDivRem(const modpoly & th, const modpoly & other,modpoly & quo, modpoly & rem,bool fastdivcheck){
    int n=int(th.size()), m=int(other.size());
    gen t=th[n-1], o=other[m-1];
    if (fastdivcheck && n && m ){
      if (is_zero(o)){
	if (!is_zero(t))
	  return false;
      }
      else {
	if (!is_zero(t % o))
	  return false;
	// if ((n>1) && (m>1))
	//  COUT << ( th[n-2]-other[m-2]*(t/o) ) % o << '\n';
      }
    }
    environment env;
    if (fastdivcheck){
      env.moduloon=true;
      env.modulo=p4;
      bool res=DivRem(th,other,&env,quo,rem,false);
      if (!res || !rem.empty())
	return false;
    }
    env.moduloon=false;
    bool res=DivRem(th,other,&env,quo,rem,false);
    return res;
  }

  modpoly operator / (const modpoly & th,const modpoly & other) {  
    modpoly rem,quo;
    environment env;
    DivRem(th,other,&env,quo,rem);
    return quo;
  }

  modpoly operator % (const modpoly & th,const modpoly & other) {  
    modpoly rem,quo;
    environment env;
    DivRem(th,other,&env,quo,rem);
    return rem;
  }

  modpoly operator_div (const modpoly & th,const modpoly & other,environment * env) {  
    modpoly rem,quo;
    DivRem(th,other,env,quo,rem);
    return quo;
  }

  modpoly operator_mod (const modpoly & th,const modpoly & other,environment * env) {  
    modpoly rem,quo;
    DivRem(th,other,env,quo,rem);
    return rem;
  }

  // Pseudo division a*th = other*quo + rem
  void PseudoDivRem(const dense_POLY1 & th, const dense_POLY1 & other, dense_POLY1 & quo, dense_POLY1 & rem, gen & a){
    int ts=int(th.size());
    int os=int(other.size());
    if (ts<os){
      quo.clear();
      rem=th;
      a=1;
    }
    else {
      gen l(other[0]);
      a=pow(l,ts-os+1);
      DenseDivRem(th*a,other,quo,rem);
    }
  }

  /*
  dense_POLY1 AscPowDivRemModifiable(dense_POLY1 & num, dense_POLY1 & den,int order){
    // reverse and adjust den degree to order
    reverse(den.begin(),den.end());
    rrdm(den,order);
    // reverse and adjust num degree to 2*order
    reverse(num.begin(),num.end());
    rrdm(num,2*order);
    dense_POLY1 quo,rem;
    DenseDivRem(num,den,quo,rem);
    reverse(quo.begin(),quo.end());
    return trim(quo,env);
  }

  dense_POLY1 AscPowDivRem(const dense_POLY1 & num, const dense_POLY1 & den,int order){
    dense_POLY1 numcopy(num),dencopy(den);
    return AscPowDivRemModifiable(numcopy,dencopy,order);
  }
  */

  // a-b*q
  int precond_a_bq(int a,int b,int q,int p,int qsurp){
    a += (a>>31)&p;
    a -= precond_mulmod31(b,q,p,qsurp);
    a += (a>>31)&p;
    return a;
  }

  // a-q1*b1-q2*b2
  int precond_a_q1b1_q2b2(int a,int q1,int b1,int q2,int b2,int p,int q1surp,int q2surp){
    a += (a>>31)&p; // insure a is positive
    b1 += (b1>>31)&p; // insure b1 is positive
    int t=longlong(b1)*q1-((longlong(b1)*q1surp)>>31)*p;
    t += (t>>31)&p; // t positive (or at least t-p is valid)
    a -= t;
    a += (a>>31)&p; // insure a is positive
    b2 += (b2>>31)&p; // insure b2 is positive
    t = longlong(b2)*q2-((longlong(b2)*q2surp)>>31)*p;
    t += (t>>31)&p; // t positive
    a -= t;
    return a; 
  }

  // Euclidean division modulo m
  void DivRem(const vector<int> & th, const vector<int> & other,int m,vector<int> & quo, vector<int> & rem,bool ck_exactquo){
    if (other.empty()){
      rem=th;
      quo.clear();
      return;
    }
    if (th.empty()){
      quo=th;
      rem=th;
      return;
    }
    int a=int(th.size())-1;
    int b=int(other.size())-1;
    vector<int> quo_,rem_; // debug
    if (b>=FFTMUL_SIZE && a-b>=FFTMUL_SIZE){
      int divquores=DivQuo(th,other,m,quo,ck_exactquo);
      if (divquores){
	rem.clear();
	if (divquores==2)
	  return;
	operator_times(other,quo,m,rem);
	submodneg(rem,th,m);
	return ;
	quo_=quo; rem_=rem;
      }
    }
    int coeff=other.front(),invcoeff=invmod(coeff,m);
    if (!b){
      quo=th;
      mulmod(quo,invcoeff,m);
      rem.clear();
      return;
    }
    quo.clear();
    double invm=1.0/m;//find_invp(m); => chk_normalize failure
    if (a==b+1){
      rem.clear();
      // frequent case in euclidean algorithms
      // rem=th-other*q
      vector<int>::const_iterator at=th.begin()+2,bt=other.begin()+1,btend=other.end();
#if 1
      //vector<int> rem_,quo_;
      {
      longlong q0=amodp(longlong(th[0])*invcoeff,m,invm);
      q0 += (q0>>63)&m;
      longlong q1=amodp(longlong(amodp(th[1]-other[1]*q0,m,invm) )*invcoeff,m,invm);
      q1 += (q1>>63)&m;
      quo.push_back(int(q0));
      quo.push_back(int(q1));
      // first part of the loop, remainder is empty, push r only if non 0
      for (;;++at){
	longlong r=*at-q1*(*bt);
	++bt;
	if (bt==btend){
	  r=amodp(r,m,invm);
	  if (r && r!=m && r!=-m)
	    rem.push_back(int(r));
	  return;
	}
	r -= q0*(*bt);
	r =amodp(r,m,invm);
	if (r && r!=m && r!=-m){
	  rem.push_back(int(r));
	  break;
	}
      }
      // second part of the loop, remainder is not empty, push r always
      --btend; ++at;
#if 1
      btend-=3; 
      int b1,b2=*bt;
      for (;bt<btend;at+=4,bt+=4){
	b1=bt[1];
	rem.push_back( amodp(at[0]-q1*b2-q0*b1,m,invm) );
	b2=bt[2];
	rem.push_back( amodp(at[1]-q1*b1-q0*b2,m,invm) );
	b1=bt[3];
	rem.push_back( amodp(at[2]-q1*b2-q0*b1,m,invm) );
	b2=bt[4];
	rem.push_back( amodp(at[3]-q1*b1-q0*b2,m,invm) );
      }
      btend+=3;
#endif
      for (;bt!=btend;++at,++bt){
	rem.push_back( amodp(*at-q1*(*bt)-q0*bt[1],m,invm) );
      }
      rem.push_back(amodp(*at-q1*(*bt),m,invm));
      return;
      //rem_.swap(rem);quo_.swap(quo);at=th.begin()+2;bt=other.begin()+1;btend=other.end();
      }
#endif
#ifdef GIAC_PRECOND
      invcoeff += (invcoeff>>31)&m;
      int invcoeffinv=(1LL<<31)*invcoeff/m+1;
      int q0=precond_mulmod31(th[0],invcoeff,m,invcoeffinv);
      //if ((q0-longlong(th[0])*invcoeff)%m!=0)
      //CERR << "err\n";
      int q0inv=(1LL<<31)*q0/m+1;
      int q1=precond_a_bq(th[1],other[1],q0,m,q0inv);
      q1=precond_mulmod31(q1,invcoeff,m,invcoeffinv);
      //if ((q1-(( (th[1]-other[1]*q0)%m )*invcoeff))%m!=0)
      //CERR << "err\n";
      int q1inv=(1LL<<31)*q1/m+1;
      quo.push_back(int(q0));
      quo.push_back(int(q1));
      // first part of the loop, remainder is empty, push r only if non 0
      for (--btend;bt!=btend;++at,++bt){
	int r=precond_a_q1b1_q2b2(*at,q1,*bt,q0,bt[1],m,q1inv,q0inv);
	if (r!=0){
	  rem.push_back(r);
	  ++at;++bt;
	  break;
	}
      }
      for (;bt!=btend;++at,++bt){
	int r=precond_a_q1b1_q2b2(*at,q1,*bt,q0,bt[1],m,q1inv,q0inv);
	// int s=(*at-q1*(*bt)-q0*bt[1])%m;
	//if ((longlong(r)-s)%m!=0)
	//CERR << "err\n";
	rem.push_back(r);
      }
      rem.push_back(precond_a_bq(*at,*bt,q1,m,q1inv));
      return;
      //submod(rem_,rem,m); submod(quo_,quo,m); 
      //if (!rem_.empty() || !quo_.empty())
      //CERR << "err\n";
#else
      longlong q0=(longlong(th[0])*invcoeff)%m;
      longlong q1= (( (th[1]-other[1]*q0)%m )*invcoeff)%m;
      quo.push_back(int(q0));
      quo.push_back(int(q1));
      // first part of the loop, remainder is empty, push r only if non 0
      for (;;++at){
	longlong r=*at-q1*(*bt);
	++bt;
	if (bt==btend){
	  r %= m;
	  if (r)
	    rem.push_back(int(r));
	  return;
	}
	r -= q0*(*bt);
	r %= m;
	if (r){
	  rem.push_back(int(r));
	  break;
	}
      }
      // second part of the loop, remainder is not empty, push r always
      --btend;
      for (++at;bt!=btend;++at,++bt){
	rem.push_back( (*at-q1*(*bt)-q0*bt[1])%m );
      }
      rem.push_back((*at-q1*(*bt))%m);
#endif
      return;
    }
    rem=th;     // code for a-b>1
    if (a<b)
      return;
    quo.reserve(a-b+1);
    // copy rem to an array
    vector<int>::const_iterator remit=rem.begin();//,remend=rem.end();
    if ((a-b+1)*double(m)*m<9e15){
      ALLOCA(longlong, tmp, (a+1)*sizeof(longlong));//longlong * tmp=(longlong *)alloca((a+1)*sizeof(longlong));
      longlong * tmpend=&tmp[a];
      longlong * tmpptr=tmpend; // tmpend points to the highest degree coeff of A
      for (;tmpptr!=tmp-1;--tmpptr,++remit)
	*tmpptr=*remit;
      vector<int>::const_iterator B_beg=other.begin(),B_end=other.end();
      int q;//n0(0),
      for (;a>=b;--a){
	q= amodp(longlong(invcoeff)*(*tmpend),m,invm); 
	quo.push_back(q);
	--tmpend;
	// tmp <- tmp - q *B.shifted (if q!=0)
	if (q) {
	  tmpptr=tmpend;
	  vector<int>::const_iterator itq=B_beg;
	  ++itq; // first elements cancel
	  for (;itq!=B_end;--tmpptr,++itq){ 
	    *tmpptr = (*tmpptr -(longlong(q) * (*itq)));
	  }
	}
      }
      // trim rem and multiply by coeff, this will modularize rem as well
      rem.clear();
      // bool trimming=true;
      for (;tmpend!=tmp-1;--tmpend){
	if (*tmpend && *tmpend % m)
	  break;   
      }
      for (;tmpend!=tmp-1;--tmpend){
	rem.push_back( amodp(*tmpend,m,invm));
      } 
      return;
    }
#if defined VISUALC || defined BESTA_OS
    int * tmp=new int[a+1];
#else
    int tmp[a+1];
#endif
    int * tmpend=&tmp[a];
    int * tmpptr=tmpend; // tmpend points to the highest degree coeff of A
    for (;tmpptr!=tmp-1;--tmpptr,++remit)
      *tmpptr=*remit;
    vector<int>::const_iterator B_beg=other.begin(),B_end=other.end();
    int q;//n0(0),
    for (;a>=b;--a){
      //q = longlong(invcoeff)*(*tmpend) % m;
      q = amodp(longlong(invcoeff)*(*tmpend),m,invm);  //q += (q>>31)&m;
      quo.push_back(q);
      --tmpend;
      // tmp <- tmp - q *B.shifted (if q!=0)
      if (q) {
	tmpptr=tmpend;
	vector<int>::const_iterator itq=B_beg;
	++itq; // first elements cancel
	for (;itq!=B_end;--tmpptr,++itq){ 
	  *tmpptr = amodp(*tmpptr -(longlong(q) * (*itq)),m,invm);
	  //*tmpptr=(*tmpptr -(longlong(q) * (*itq)))%m;
	}
      }
    }
    // trim rem and multiply by coeff, this will modularize rem as well
    rem.clear();
    // bool trimming=true;
    for (;tmpend!=tmp-1;--tmpend){
      if (*tmpend && (*tmpend % m))
	break;   
    }
    for (;tmpend!=tmp-1;--tmpend){
      //int r=*tmpend %m;
      int r=amodp(*tmpend,m,invm); //r += (r>>31)&m;
      rem.push_back(r);
    } 
#if defined VISUALC || defined BESTA_OS
    delete [] tmp;
#endif
    return;
    // debug
    if (quo_.size()){
      submod(quo_,quo,m); submod(rem_,rem,m);
      if (quo_.size() || rem_.size())
	CERR << "err\n";
    }
  }

  // Conversion from vector<gen> to vector<int> modulo m
  void modpoly2smallmodpoly(const modpoly & p,vector<int> & v,int m){
    v.clear();
    const_iterateur it=p.begin(),itend=p.end();
    v.reserve(itend-it);
    int g;
    bool trim=true;
    for (;it!=itend;++it){
      if (it->type==_INT_)
	g=it->val % m;
      else 
	g=smod(*it,m).val;
      if (g)
	trim=false;
      if (!trim)
	v.push_back(g);
    }
  }


  // Conversion from vector<int> to vector<gen> using smod
  void smallmodpoly2modpoly(const vector<int> & v,modpoly & p,int m){
    vector<int>::const_iterator it=v.begin(),itend=v.end();
    p.clear();
    p.reserve(itend-it);
    for (;it!=itend;++it){
      p.push_back(smod(*it,m));
    }
  }

  // compute r mod b into r
  // r, b must be allocated arrays of int
  // compute quotient if quoend!=0
  // set exactquo to true if you know that b divides r and only want to compute the quotient
  // this will not compute low degree coeff of r during division and spare some time
  static void rem(int * & r,int *rend,int * b,int *bend,int m,int * & quo,int *quoend,bool exactquo=false){
    int * i,*j,*rstop,*qcur,k,q,q2,lcoeffinv=1;
    k=int(bend-b);
    if (!k){
      quo=quoend;
      return;
    }
    if (rend-r<k){
      quo=quoend;
      return;
    }
    quo=quoend-((rend-r)-(k-1));
    qcur=quo;
    // inv leading coeff of b 
    if (*b !=1)
      lcoeffinv=invmod(*b,m);
    if (k==1){
      if (quoend){
	i=quo;
	for (;r!=rend;++r,++i){
	  type_operator_times_reduce(*r,lcoeffinv,*i,m);
	  // *i=(*r*lcoeffinv)%m;
	}
      }
      else
	r=rend;
      return;
    }
    if (rend-r==bend-b+1){
      // frequent case: the degree decrease by 1
      // a(x) += b(x)*(q1*x+q2)
      // first compute q1 and q2
      q=-smod(*r*longlong(lcoeffinv),m);
      ++r;
      q2=-smod( ((*r+longlong(q)* *(b+1))%m)*longlong(lcoeffinv),m);
      if (quoend){
	*qcur=-q;
	++qcur;
	*qcur=-q2;
      }
      ++r;
      // now compute a
      j=r;
      i=b+1;
      if (i!=bend){
	if (m<46340){
	  for (;;){
	    *j += q2* (*i);
	    ++i;
	    if (i==bend){
	      *j %= m;
	      break;
	    }
	    *j += q* (*i);
	    *j %= m;
	    ++j;
	  }
	}
	else {
	  for (;;){
	    type_operator_plus_times_reduce_nock(q2,*i,*j,m);
	    ++i;
	    if (i==bend)
	      break;
	    type_operator_plus_times_reduce_nock(q,*i,*j,m);
	    ++j;
	  }
	}
      } // end if i!=bend
    }    
    else {
      ++b; 
      // while degree(r)>=degree(b) do r <- r - r[0]*lcoeffinv*b
      // rend is not used anymore, we make it point k ints before
      rstop = rend-(k-1) ; // if r==rend then deg(r)==deg(b)
      for (;rstop-r>0;){
	type_operator_times_reduce(*r,lcoeffinv,q,m);
	// q=((*r)*longlong(lcoeffinv))%m;
	if (quoend){
	  *qcur=q;
	  ++qcur;
	}
	++r;
	if (q){
	  q=-q;
	  j=r;
	  i=b;
	  for (;i!=bend;++j,++i){
	    type_operator_plus_times_reduce_nock(q,*i,*j,m);
	    // *j = (*j + q * *i)%m;
	  }
	}
	if (exactquo && rend-r<=2*(k-1))
	  --bend;
      }
    }
    // trim answer
    for (;r!=rend;++r){
      if (*r)
	break;
    }
  }

  /*
  void rem_tabint(int * & r,int *rend,int * b,int *bend,int m,int * & quo,int *quoend){
    int * i,*j,*rstop,*qcur,k,q,lcoeffinv=1;
    k=bend-b;
    if (!k){
      quo=quoend;
      return;
    }
    if (rend-r<k){
      quo=quoend;
      return;
    }
    quo=quoend-((rend-r)-(k-1));
    qcur=quo;
    // inv leading coeff of b 
    if (*b !=1)
      lcoeffinv=invmod(*b,m);
    if (k==1){
      if (quoend){
	i=quo;
	for (;r!=rend;++r,++i){
	  type_operator_times_reduce(*r,lcoeffinv,*i,m);
	  // *i=(*r*lcoeffinv)%m;
	}
      }
      else
	r=rend;
      return;
    }
    ++b; 
    // while degree(r)>=degree(b) do r <- r - r[0]*lcoeffinv*b
    // rend is not used anymore, we make it point k ints before
    rstop = rend-(k-1) ; // if r==rend then deg(r)==deg(b)
    for (;rstop-r>0;){
      type_operator_times_reduce(*r,lcoeffinv,q,m);
      // q=((*r)*longlong(lcoeffinv))%m;
      if (quoend){
	*qcur=q;
	++qcur;
      }
      ++r;
      if (q){
	q=-q;
	j=r;
	i=b;
	for (;i!=bend;++j,++i){
	  // type_operator_plus_times_reduce_nock(q,*i,*j,m);
	  *j = (*j + q * *i)%m;
	  // *j = (*j + longlong(q) * *i)%m;
	}
      }
    }
    // trim answer
    for (;r!=rend;++r){
      if (*r)
	break;
    }
  }
  */

  static void gcdconvert(const modpoly & p,int m,int * a){
    const_iterateur it=p.begin(),itend=p.end();
    for (;it!=itend;++it,++a){
      if (it->type==_INT_)
	*a=it->val % m;
      else 
	*a=smod(*it,m).val;
    }
  }

  static bool gcdconvert(const polynome & p,int m,int * a){
    vector< monomial<gen> >::const_iterator it=p.coord.begin(),itend=p.coord.end();
    int deg;
    for (;it!=itend;){
      if (it->value.type==_INT_)
	*a=it->value.val % m;
      else {
	if (it->value.type==_ZINT)
	  *a=smod(it->value,m).val;
	else
	  return false;
      }
      deg=it->index.front();
      ++it;
      if (it==itend){
	for (++a;deg>0;++a,--deg){
	  *a=0;
	}
	return true;
      }
      deg -= it->index.front();
      for (++a,--deg;deg>0;++a,--deg){
	*a=0;
      }
    }
    return true;
  }

  // Efficient small modular gcd of p and q using vector<int>
  void gcdsmallmodpoly(const modpoly &p,const modpoly & q,int m,modpoly & d){

    int as=int(p.size()),bs=int(q.size());
#if defined VISUALC || defined BESTA_OS
    int *asave=new int[as], *a=asave,*aend=a+as;
    int *bsave=new int[bs], *b=bsave,*bend=b+bs,*qcur=0;
#else
#ifndef NO_STDEXCEPT
    if (as>1000000 || bs>1000000)
      setdimerr();
#endif
    int asave[as], *a=asave,*aend=a+as;
    int bsave[bs], *b=bsave,*bend=b+bs,*qcur=0;
#endif
    gcdconvert(p,m,a);
    int * t;
    gcdconvert(q,m,b);
    for (;b!=bend;){
      rem(a,aend,b,bend,m,qcur,0);
      t=a; a=b; b=t;
      t=aend; aend=bend; bend=t;      
    }
    d.clear();
    d.reserve(aend-a);
    int ainv=1;
    if (a!=aend)
      ainv=invmod(*a,m);
    for (;a!=aend;++a){
      d.push_back(smod((*a)*longlong(ainv),m));
    }
#if defined VISUALC || defined BESTA_OS
    delete [] asave;
    delete [] bsave;
#endif
  }

  bool gcdsmallmodpoly(const polynome &p,const polynome & q,int m,polynome & d,polynome & dp,polynome & dq,bool compute_cof){
    if (p.dim!=1 || q.dim!=1)
      return false;
    bool promote = m>=46340;
    int as=p.lexsorted_degree()+1,bs=q.lexsorted_degree()+1;
    if (as>HGCD*4 || bs>HGCD*4) 
      return false;
#if defined VISUALC || defined BESTA_OS
    int *asave = new int[as], *a=asave,*aend=a+as,*qcur=0;
    int *Asave = new int[as], *A=Asave,*Aend=A+as;
    int *bsave = new int[bs], *b=bsave,*bend=b+bs;
    int *Bsave = new int[bs], *B=Bsave,*Bend=B+bs;
#else // this will allocate too much on stack for as+bs large
    int asave[as], *a=asave,*aend=a+as,*qcur=0;
    int Asave[as], *A=Asave,*Aend=A+as;
    int bsave[bs], *b=bsave,*bend=b+bs;
    int Bsave[bs], *B=Bsave,*Bend=B+bs;
#endif
    int * t;
    if (gcdconvert(p,m,a) && gcdconvert(q,m,b) ){
      memcpy(Asave,asave,as*sizeof(int));
      memcpy(Bsave,bsave,bs*sizeof(int));
      for (;b!=bend;){
	rem(a,aend,b,bend,m,qcur,0);
	t=a; a=b; b=t;
	t=aend; aend=bend; bend=t;      
      }
      d.coord.clear();
      int ainv=1;
      int * aa=a;
      if (a!=aend)
	ainv=invmod(*a,m);
      if (promote){
	for (int deg=int(aend-a)-1;a!=aend;++a,--deg){
	  if (*a){
	    *a=smod((*a)*longlong(ainv),m);
	    d.coord.push_back(monomial<gen>(*a,deg,1,1));
	  }
	}
      }
      else {
	for (int deg=int(aend-a)-1;a!=aend;++a,--deg){
	  if (*a){
	    *a=smod((*a)*ainv,m);
	    d.coord.push_back(monomial<gen>(*a,deg,1,1));
	  }
	}
      }
      if (aa!=aend && compute_cof){
	if (debug_infolevel>20)
	  CERR << "gcdsmallmodpoly, compute cofactors " << CLOCK() << '\n';
#if defined VISUALC || defined BESTA_OS
	int * qsave=new int[std::max(as,bs)], *qcur=qsave,*qend=qsave+std::max(as,bs);
#else
	int qsave[std::max(as,bs)], *qcur=qsave,*qend=qsave+std::max(as,bs);
#endif
	// int * qsave=new int[as], *qcur=qsave,*qend=qsave+as;
	rem(A,Aend,aa,aend,m,qcur,qend);
	dp.coord.clear();
	for (int deg=int(qend-qcur)-1;qcur!=qend;++qcur,--deg){
	  if (*qcur)
	    dp.coord.push_back(monomial<gen>(smod(*qcur,m),deg,1,1));
	}
	qcur=qsave;
	rem(B,Bend,aa,aend,m,qcur,qend);
	dq.coord.clear();
	for (int deg=int(qend-qcur)-1;qcur!=qend;++qcur,--deg){
	  if (*qcur)
	    dq.coord.push_back(monomial<gen>(smod(*qcur,m),deg,1,1));
	}
	if (debug_infolevel>20)
	  CERR << "gcdsmallmodpoly, end compute cofactors " << CLOCK() << '\n';
#if defined VISUALC || defined BESTA_OS
	delete [] qsave; 
#endif
      }
#if defined VISUALC || defined BESTA_OS
      delete [] asave; delete [] Asave; delete [] bsave; delete [] Bsave;
#endif
      return true;
    }
    else {
#if defined VISUALC || defined BESTA_OS
      delete [] asave; delete [] Asave; delete [] bsave; delete [] Bsave;
#endif
      return false;
    }
  }

  // invert a1 mod m
  double invmod(double a1,double A){
    double a(A),a2,u=0,u1=1,u2,q;
    for (;a1;){
      q=std::floor(a/a1);
      a2=a-q*a1;
      u2=u-q*u1;
      a=a1;
      a1=a2;
      u=u1;
      u1=u2;
    }
    if (a==-1){ a=1; u=-u; }
    if (a!=1) return 0;
    if (u<0) u+=A;
    return u;
  }

  bool convertdouble(const modpoly & p,double M,vector<double> & v){
    v.clear(); v.reserve(p.size());
    int m=int(M);
    const_iterateur it=p.begin(),itend=p.end();
    for (;it!=itend;++it){
      if (it->type==_INT_)
	v.push_back(it->val % m);
      else {
	if (it->type==_ZINT)
	  v.push_back(smod(*it,m).val);
	else
	  return false;
      }
    }
    return true;    
  }

  bool convertfromdouble(const vector<double> & A,modpoly & a,double M){
    a.clear(); a.reserve(A.size());
    int m( (int)M);
    vector<double>::const_iterator it=A.begin(),itend=A.end();
    for (;it!=itend;++it){
      double d=*it;
      if (d!=int(d))
	return false;
      if (d>M/2)
	a.push_back(int(d)-m);
      else
	a.push_back(int(d));
    }
    return true;
  }

  void multdoublepoly(double x,vector<double> & v,double m){
    if (x==1)
      return;
    vector<double>::iterator it=v.begin(),itend=v.end();
    for (;it!=itend;++it){
      double t=*it * x;
      double q=std::floor(t/m);
      *it = t-q*m;
    }
  }

  // A = BQ+R mod m with B leading coeff = 1
  void quoremdouble(const vector<double> & A,const vector<double> & B,vector<double> & Q,vector<double> & R,double m){
    Q.clear();
    R=A;
    int rs=int(R.size()),bs=int(B.size());
    if (rs<bs)
      return;
    if (rs==bs+1){ } // possible improvement
    vector<double>::iterator it=R.begin(),itend=it+(rs-bs+1);
    for (;it!=itend;){
      double q=*it;
      Q.push_back(q);
      *it=0;
      ++it;
      vector<double>::iterator kt=it;
      vector<double>::const_iterator jt=B.begin()+1,jtend=B.end();
      for (;jt!=jtend;++kt,++jt){
	double d= *kt- q*(*jt);
	*kt=d-std::floor(d/m)*m;
      }
      for (;it!=itend;++it){
	if (*it)
	  break;
      }
    }
    for (;it!=R.end();++it){
      if (*it)
	break;
    }
    R.erase(R.begin(),it);
  }

  bool gcddoublemodpoly(const modpoly &p,const modpoly & q,double m,modpoly &a){
    vector<double> A,B,Q,R;
    if (!convertdouble(p,m,A) || !convertdouble(q,m,B))
      return false;
    while (!B.empty()){
      multdoublepoly(invmod(B.front(),m),B,m);
      quoremdouble(A,B,Q,R,m);
      swap(A,B);
      swap(B,R);
    }
    if (!A.empty())
      multdoublepoly(invmod(A.front(),m),A,m);
    return convertfromdouble(A,a,m);
  }

  void reverse_resize(modpoly & a,int N,int reserve){
    reverse(a.begin(),a.end());
    // for (int i=a.size();i<N;++i) a.push_back(0);
    a.resize(N);
    for (int i=0;i<a.size();++i){
      if (a[i].type==_ZINT)
	a[i]=*a[i]._ZINTptr;
      else
	a[i].uncoerce(reserve);
    }
  }

  // a=source mod x^N-1 mod p
  void reverse_assign(const modpoly & source,vector<int> & a,int N,int p){
    a.clear(); a.resize(N);
    if (source.empty()) return;
    const gen * stop=&*source.begin(),*start=&*source.end()-1;
    int i=0;
    for (;i<N && start>=stop;i++,--start){
      if (start->type==_INT_)
	a[i]=start->val % p;
      else 
	a[i]=modulo(*start->_ZINTptr,p);
    }
    for (i=0;start>=stop;--start){
      if (start->type==_INT_)
	a[i]=(a[i]+longlong(start->val)) %p;
      else
	a[i]=(a[i]+longlong(modulo(*start->_ZINTptr,p))) % p;
      ++i;
      if (i==N)
	i=0;
    }
  }
  
  // make f coeffs in [0,p]
  void make_positive(vector<int> & f,int p){
    for (vector<int>::iterator it=f.begin();it!=f.end();++it){
      int i=*it;
      i += (i>>31)&p;
      i -= p;
      i += (i>>31)&p;
      *it=i;
    }
  }

  void reverse_assign(vector<int> & a,int N,int p){
    if (a.size()>N){
      vector<int>::iterator it=a.begin(),jt=it+N,jtend=a.end();
      for (;it<jt;++it)
	*it += (*it>>31)&p;
      for (it=a.begin();jt!=jtend;++it,++jt){
	int i=*it,j=*jt;
	j += (j>>31)&p;
	i += j-p;
	i += (i>>31)&p;
	*jt=i;
      }
      a.erase(a.begin(),it);
      reverse(a.begin(),a.end());
    }
    else {
      make_positive(a,p);
      reverse(a.begin(),a.end());
      a.resize(N);
    }
  }

  // a=source mod x^N-1 mod p
  void reverse_assign(const vector<int> & source,vector<int> & a,int N,int p){
    a.clear(); a.resize(N);
    if (source.empty()) return;
    const int * stop=&*source.begin(),*start=&*source.end()-1;
    int i=0;
    for (;i<N && start>=stop;i++,--start){
      int k=*start;
      k += (k>>31)&p; // add p if k is negative
      // if (k<0)
      //CERR << "err\n";
      a[i]=k;
    }
    for (i=0;start>=stop;--start){
      int k=*start;
      k -= (k>>31)*p; 
      k += (a[i]-p);
      k -= (k>>31)*p; 
      a[i]= k ; 
      // a[i]=(a[i]+longlong(*start)) %p;
      // if ( (a[i]-longlong(k))%p!=0)
      //if (k<0)
      //CERR << "err\n";
      ++i;
      if (i==N)
	i=0;
    }
  }
  
  // a=source mod x^N-1
  void reverse_assign(const modpoly & source,modpoly & a,int N,int reserve){
    if (&source==&a){
      a.reserve(N);
      reverse(a.begin(),a.end());
      for (int i=0;i<a.size();++i)
	a[i].uncoerce(reserve);
      for (int i=a.size();i<N;++i){
	gen g; g.uncoerce(reserve);
	a.push_back(g);
      }
      return;
    }
    a.resize(N);
    const gen * stop=&*source.begin(),*start=&*source.end()-1;
    int i=0;
    for (;i<N && start>=stop;i++,--start){
      if (a[i].type!=_ZINT){
	a[i]=0;
	a[i].uncoerce(reserve);
      }
      if (start->type==_INT_)
	mpz_set_si(*a[i]._ZINTptr,start->val);
      else 
	mpz_set(*a[i]._ZINTptr,*start->_ZINTptr);
    }
    for (;i<N ;i++){
      gen & g=a[i];
      if (g.type==_ZINT)
	mpz_set_si(*g._ZINTptr,0);
      else {
	g=0;
	g.uncoerce(reserve);
      }
    }
    for (i=0;start>=stop;--start){
      if (start->type==_INT_){
	if (start->val>=0)
	  mpz_add_ui(*a[i]._ZINTptr,*a[i]._ZINTptr,start->val);
	else
	  mpz_sub_ui(*a[i]._ZINTptr,*a[i]._ZINTptr,-start->val);
      }
      else 
	mpz_add(*a[i]._ZINTptr,*a[i]._ZINTptr,*start->_ZINTptr);
      ++i;
      if (i==N)
	i=0;
    }
  }

  void fft_ab_p1(vector<int> &a,const vector<int> &b){
    size_t s=a.size();
    for (size_t i=0;i<s;++i){
      a[i]=(longlong(a[i])*b[i])%p1;
    }
  }

  void fft_ab_p2(vector<int> &a,const vector<int> &b){
    size_t s=a.size();
    for (size_t i=0;i<s;++i){
      a[i]=(longlong(a[i])*b[i])%p2;
    }
  }

  void fft_ab_p3(vector<int> &a,const vector<int> &b){
    size_t s=a.size();
    for (size_t i=0;i<s;++i){
      a[i]=(longlong(a[i])*b[i])%p3;
    }
  }

  void fft_ab(fft_rep & a,const fft_rep & b){
    fft_ab_p1(a.modp1,b.modp1);
    fft_ab_p2(a.modp2,b.modp2);
    fft_ab_p3(a.modp3,b.modp3);
  }

  void a_minus_qsize2_b(const vector<int> & ua,const vector<int> & q,const vector<int> &ub,vector<int> & ur,int p){
    double invp=find_invp(p);
    longlong q1=-q[0],q0=-q[1];
    ur.clear(); ur.push_back((q1*ub.front())%p);
    const int * it=&ub[0],*itend=it-1+ub.size(),*itmid=it+ub.size()-ua.size(),*jt=&ua[0];
    if (ua.empty()){
      for (;it!=itend;++it){
	ur.push_back(amodp(q0*it[0]+q1*it[1],p,invp));
      }
      ur.push_back(amodp(q0*it[0],p,invp));
    }
    else {
#if 1
      itmid-=4;
      int i0=it[0],i1;
      for (;it<itmid;it+=4){
	i1=it[1];
	ur.push_back(amodp(q0*i0+q1*i1,p,invp));
	i0=it[2];
	ur.push_back(amodp(q0*i1+q1*i0,p,invp));
	i1=it[3];
	ur.push_back(amodp(q0*i0+q1*i1,p,invp));
	i0=it[4];	
	ur.push_back(amodp(q0*i1+q1*i0,p,invp));
      }
      itmid+=4;
#endif
      for (;it!=itmid;++it){
	ur.push_back(amodp(q0*it[0]+q1*it[1],p,invp));
      }
#if 1
      itend-=4;
      i0=it[0];
      for (;it<itend;it+=4,jt+=4){
	i1=it[1];
	ur.push_back(amodp(q0*i0+q1*i1+jt[0],p,invp));
	i0=it[2];
	ur.push_back(amodp(q0*i1+q1*i0+jt[1],p,invp));
	i1=it[3];
	ur.push_back(amodp(q0*i0+q1*i1+jt[2],p,invp));
	i0=it[4];	
	ur.push_back(amodp(q0*i1+q1*i0+jt[3],p,invp));
      }
      itend+=4;
#endif
      for (;it!=itend;++jt,++it){
	ur.push_back(amodp(q0*it[0]+q1*it[1]+*jt,p,invp)); 
      }
      ur.push_back(amodp(q0*it[0]+*jt,p,invp));
    }
    //make_positive(ur,p);
  }

  void a_minus_qsize2_b(const vector<int> & ua,const vector<int>& q,const vector<int> &ub,vector<int> & ur,int p,int & q0inv,int & q1inv){
    //if (ua.empty())
      return a_minus_qsize2_b(ua,q,ub,ur,p);
    int q1=q[0],q0=q[1];
    if (q0inv==0 || q1inv==0){
      q0 += (q0>>31)&p;
      q1 += (q1>>31)&p;
      q0inv=(1ULL<<31)*unsigned(q0)/unsigned(p)+1;
      q1inv=(1ULL<<31)*unsigned(q1)/unsigned(p)+1;
    }
    ur.clear(); 
    ur.push_back(precond_mulmod31(-ub.front(),q1,p,q1inv)); // (-q1*ub.front())%p);
    const int * it=&ub[0],*itend=it-1+ub.size(),*itmid=it+ub.size()-ua.size(),*jt=&ua[0];
    for (;it!=itmid;++it){
      ur.push_back(precond_a_q1b1_q2b2(0,q0,it[0],q1,it[1],p,q0inv,q1inv));
    }
    for (;it!=itend;++jt,++it){
      ur.push_back(precond_a_q1b1_q2b2(*jt,q0,it[0],q1,it[1],p,q0inv,q1inv)); 
    }
    ur.push_back(precond_a_bq(*jt,it[0],q0,p,q0inv));//(*jt-q0*it[0])%p);
  }

  bool hgcd_iter_int(const vector<int> & a0i,const vector<int> & b0i,int m,vector<int> & ua,vector<int> & ub,vector<int> & va,vector<int> &vb,int p,vector<int> & coeffv,vector<int> & degv,vector<int> &a,vector<int> & b,vector<int> & q,vector<int> & r,vector<int> & ur,vector<int> & vr){
    if (debug_infolevel>2)
      CERR << CLOCK()*1e-6 << " halfgcd iter m=" << m << " dega0/a1 " << a0i.size() << "," << b0i.size() << '\n';
    int as=a0i.size(),as2=nextpow2(as);
    a.reserve(as2); b.reserve(as2);
    a.resize(a0i.size());
    b.resize(b0i.size());
    copy(a0i.begin(),a0i.end(),a.begin());
    copy(b0i.begin(),b0i.end(),b.begin());
    r.reserve(as);
    // initializes ua to 1 and ub to 0, the coeff of u in ua*a+va*b=a
    ua.reserve(as2); ua.clear(); ua.push_back(1); ub.clear(); ub.reserve(as2); ur.clear(); ur.reserve(as2); 
    va.reserve(as2); va.clear(); vb.clear(); vb.reserve(as2); vb.push_back(1); vr.clear(); vr.reserve(as2);
    vector<int>::iterator it,itend;
    // DivRem: a = bq+r 
    // hence ur <- ua-q*ub, vr <- va-q*vb verify
    // ur*a+vr*b=r
    // a <- b, b <- r, ua <- ub and ub<- ur
    for (;;){
      int n=int(b.size())-1;
      if (n<m){ // degree(b) is small enough
	if (debug_infolevel>2)
	  CERR << CLOCK()*1e-6 << " halfgcd iter end" << a0i.size() << "," << b0i.size() << '\n';
	make_positive(ua,p);
	make_positive(ub,p);
	make_positive(va,p);
	make_positive(vb,p);
	return true;
      }
      if (!degv.empty()){
	degv.push_back(degv.back()+b.size()-a.size());
	coeffv.push_back(b[0]);
      }
      DivRem(a,b,p,q,r); // division works always
      swap(a,b); swap(b,r); // a=b; b=r;
      // ur=ua-q*ub, ua<-ub, ub<-ur
      if (q.size()==2){ // here ua.size()<ub.size()
	int q1inv=0,q2inv=0;
	if (ub.empty())
	  swap(ua,ub);
	else {
	  a_minus_qsize2_b(ua,q,ub,ur,p,q1inv,q2inv);
	  swap(ua,ub); swap(ub,ur);
	}
	a_minus_qsize2_b(va,q,vb,vr,p,q1inv,q2inv);
	swap(va,vb); swap(vb,vr);
	continue;
      }
      if (ub.empty())
	swap(ua,ub);
      else {
	mulsmall(q.begin(),q.end(),ub.begin(),ub.end(),p,ur);
	submodneg(ur,ua,p);
	swap(ua,ub); swap(ub,ur); // ua=ub; ub=ur;
      }
      if (vb.size()==1 && vb.front()==1) vr.swap(q); else mulsmall(q.begin(),q.end(),vb.begin(),vb.end(),p,vr);
      submodneg(vr,va,p);
      swap(va,vb); swap(vb,vr); // ua=ub; ub=ur;
    }
    return false; // never reached
  }

  void a_minus_qsize2_b(const vecteur & ua,const vecteur & q,const vecteur &ub,vecteur & ur,int p){
    longlong q1=-q[0].val,q0=-q[1].val;
    ur.push_back((q1*ub.front().val)%p);
    const gen * it=&ub[0],*itend=it-1+ub.size(),*itmid=it+ub.size()-ua.size(),*jt=&ua[0];
    if (ua.empty()){
      for (;it!=itend;++it){
	ur.push_back((q0*it[0].val+q1*it[1].val)%p);
      }
      ur.push_back((q0*it[0].val)%p);
    }
    else {
      for (;it!=itmid;++it){
	ur.push_back((q0*it[0].val+q1*it[1].val)%p);
      }
      for (;it!=itend;++jt,++it){
	ur.push_back((q0*it[0].val+q1*it[1].val+jt->val)%p); 
      }
      ur.push_back((q0*it[0].val+jt->val)%p);
    }
  }

  void a_bc(const vector<int> &a,const vector<int> &b,const vector<int> &c,int p,vector<int> & res,vector<int> & tmp1){
    // res=trim(a-b*c,env); return;
    size_t as=a.size(),bs=b.size();
    if (as<=bs){
      tmp1.clear(); tmp1.reserve(bs);
      if (c.size()==2){
	a_minus_qsize2_b(a,c,b,tmp1,p);
	tmp1.swap(res); 
	//make_positive(res,p);
	return;
      }
    }
    mulsmall(b.begin(),b.end(),c.begin(),c.end(),p,tmp1);
    submodneg(tmp1,a,p);
    tmp1.swap(res);
  }

  // a-b*c
  void a_bc(const modpoly &a,const modpoly &b,const modpoly &c,environment * env,modpoly & res,modpoly & tmp1){
    // res=trim(a-b*c,env); return;
    size_t as=a.size(),bs=b.size();
    if (as<=bs && env->moduloon && env->modulo.type==_INT_){
      tmp1.clear(); tmp1.reserve(bs);
      int p=env->modulo.val;
      if (c.size()==2){
	a_minus_qsize2_b(a,c,b,tmp1,p);
	tmp1.swap(res); return;
      }
    }
    environment zeroenv;
    tmp1.clear();
    if (!b.empty() && !c.empty())
      operator_times(b,c,&zeroenv,tmp1);
    submodpoly(a,tmp1,res);
    trim_inplace(res,env);
  }

  void smod2N(mpz_t & z,unsigned long expoN,mpz_t & tmpqz,bool do_smod=false){
    mpz_tdiv_q_2exp(tmpqz,z,expoN);
    if (mpz_cmp_si(tmpqz,0)){
      mpz_tdiv_r_2exp(z,z,expoN);
      mpz_sub(z,z,tmpqz);
      mpz_tdiv_q_2exp(tmpqz,z,expoN);
      if (mpz_cmp_si(tmpqz,0)){      
	mpz_tdiv_r_2exp(z,z,expoN);
	mpz_sub(z,z,tmpqz); 
      }
    }
    if (!do_smod)
      return;
    mpz_tdiv_q_2exp(tmpqz,z,expoN-1);
    if (mpz_cmp_si(tmpqz,0)){
      mpz_sub(z,z,tmpqz);
      mpz_mul_2exp(tmpqz,tmpqz,expoN);
      mpz_sub(z,z,tmpqz);
    }
  }

  void trim_deg(modpoly & a,int deg){
    if (a.size()>deg+1)
      a.erase(a.begin(),a.end()-deg-1);
  }

  void trim_deg(vector<int> & a,int deg){
    if (a.size()>deg+1)
      a.erase(a.begin(),a.end()-deg-1);
  }

  void trim_deg(vector<longlong> & a,int deg){
    if (a.size()>deg+1)
      a.erase(a.begin(),a.end()-deg-1);
  }

#ifdef INT128
  #define GIAC_LLPRECOND 1
  inline longlong precond_mulmodll(ulonglong A,ulonglong W,ulonglong Winvp,ulonglong p){
    longlong t = uint128_t(A)*W-((uint128_t(A)*Winvp)>>64)*p;
    t+=((t>>63)&p);
    return t;
    // debug
    if ((uint128_t(A)*W-t)%p!=0)
      CERR << "err\n";
    return t;
  }
  inline ulonglong preconditionner_ll(ulonglong ww,longlong p){
    return 1+((uint128_t(1)<<64)*ww)/ulonglong(p); // quotient ceiling
  }
  longlong smodll(int128_t res,longlong m){
    res %= m;
    if (res>m/2)
      res -= m;
    return res;
  }

  // this does not work for 63 bits primes because long_double
  // aka float128 seems to be FPU 80 bits integers with 64 bits of mantissa
  // insufficent precision
  inline longlong amodpll(int128_t a,longlong p,long_double invp){
    longlong q=long_double(a)*invp;
    q=a-int128_t(q)*p;
    q+=(q>>63)&p;q+=(q>>63)&p;
    q-=p;q+=(q>>63)&p;
    return q;
    // debug
    if (q!=q%p)
      CERR << "err amodpll\n";
    return q%p;
  }

#if !defined USE_GMP_REPLACEMENTS && !defined BF2GMP_H
  void vecteur2vector_ll(const vecteur & v,longlong m,vector<longlong> & res){
    vecteur::const_iterator it=v.begin(),itend=v.end();
    res.clear();
    res.reserve(itend-it);
    if (m<0)
      m=-m;
    for (;it!=itend;++it){
      gen g=*it;
      if (it->type==_MOD)
	g=*it->_MODptr;
      longlong r=it->type==_ZINT?mpz_fdiv_ui(*it->_ZINTptr,m):(it->val % m);
      r += (ulonglong(r)>>63)*m; // make positive
      // r -= (ulonglong((m>>1)-r)>>31)*m; // smod
      res.push_back(r);
    }
  } 
#endif

  // longlong fft
  // exemple of Fourier primes (with 2^53-roots of unity)
  // [4719772409484279809,4782822804267466753,4854880398305394689,5071053180419178497,5179139571476070401,5323254759551926273,5395312353589854209,5503398744646746113,5998794703657500673,6151917090988097537,6269010681299730433,6566248256706183169,6782421038819966977,6962565023914786817,7097673012735901697,7557040174727692289,7728176960567771137,7908320945662590977,8295630513616453633,8583860889768165377,8592868089022906369,8691947280825057281,9097271247288401921]
  const longlong p5=9097271247288401921LL;
  const long_double invp5=long_double(1)/p5;
  static inline longlong addmodll(longlong a, longlong b, longlong p) { 
    longlong t=(a-p)+b;
    t += (t>>63)&p;
    return t; 
  }

  static inline longlong submodll(longlong a, longlong b, longlong p) { 
    longlong t=a-b;
    t += (t>>63)&p;
    return t; 
  }

  static inline longlong mulmodll(longlong a, longlong b, longlong p) { 
    return (int128_t(a)*b) % p;
  }

  void mulmodll(vector<longlong> & v,longlong b,longlong p){
    vector<longlong>::iterator it=v.begin(),itend=v.end();
    int128_t B=b;
    for (;it!=itend;++it){
      *it=(*it*B)%p;
    }
  }

  static inline longlong mulmodll(longlong a, longlong b, longlong p,long_double invp) { 
    return amodpll(int128_t(a)*b,p,invp);
  }

  void mulmodll(vector<longlong> & v,longlong b,longlong p,long_double invp){
    vector<longlong>::iterator it=v.begin(),itend=v.end();
    for (;it!=itend;++it){
      *it=mulmodll(*it,b,p,invp);
      // *it=(*it*int128_t(B))%p;
    }
  }

  void precond_mulmodll(vector<longlong> & v,longlong b,longlong bsurp,longlong p){
    vector<longlong>::iterator it=v.begin(),itend=v.end();
    for (;it!=itend;++it){
      *it=precond_mulmodll(*it,b,bsurp,p);
    }
  }

  // Euclidean division modulo m
  void DivRem(const vector<longlong> & th, const vector<longlong> & other,longlong m,vector<longlong> & quo, vector<longlong> & rem){
    if (other.empty()){
      rem=th;
      quo.clear();
      return;
    }
    if (th.empty()){
      quo=th;
      rem=th;
      return;
    }
    longlong a=longlong(th.size())-1;
    longlong b=longlong(other.size())-1;
    longlong coeff=other.front(),invcoeff=invmodll(coeff,m);
    long_double invm=long_double(1)/m;
    if (!b){
      quo=th;
      mulmodll(quo,invcoeff,m);
      rem.clear();
      return;
    }
    quo.clear();
    if (a==b+1){
      rem.clear();
      // frequent case in euclidean algorithms
      int128_t q0=amodpll(int128_t(th[0])*invcoeff,m,invm);
      if (q0<0) q0+=m;
      int128_t q1=amodpll(int128_t(amodpll(th[1]-other[1]*q0,m,invm) )*invcoeff,m,invm);// (( (th[1]-other[1]*q0)%m )*invcoeff)%m;
      if (q1<0) q1+=m;
      quo.push_back(longlong(q0));
      quo.push_back(longlong(q1));
      // rem=th-other*q
      vector<longlong>::const_iterator at=th.begin()+2,bt=other.begin()+1,btend=other.end();
      // first part of the loop, remainder is empty, push r only if non 0
      for (;;++at){
	int128_t r=*at-q1*(*bt);
	++bt;
	if (bt==btend){
	  r =amodpll(r,m,invm);
	  if (r&& r!=m && r!=-m)
	    rem.push_back(longlong(r));
	  return;
	}
	r -= q0*(*bt);
	r = amodpll(r,m,invm);
	if (r&& r!=m && r!=-m){
	  rem.push_back(longlong(r));
	  break;
	}
      }
      // second part of the loop, remainder is not empty, push r always
      for (++at;;++at){
	int128_t r=*at-q1*(*bt);
	++bt;
	if (bt==btend){
	  rem.push_back(amodpll(r,m,invm));
	  return;
	}
	rem.push_back(amodpll(r-q0*(*bt),m,invm));//rem.push_back((r-q0*(*bt))%m);
      }
    }
    rem=th;
    if (a<b)
      return;
    quo.reserve(a-b+1);
    // A=BQ+R -> A*invcoeff=(B*invcoeff)*Q+(R*invcoeff), 
    // make division of A*invcoeff by B*invcoeff and multiply R by coeff at the end
    // copy rem to an array
    vector<longlong>::const_iterator remit=rem.begin();//,remend=rem.end();
#if defined VISUALC || defined BESTA_OS
    longlong * tmp=new longlong[a+1];
#else
    longlong tmp[a+1];
#endif
    longlong * tmpend=&tmp[a];
    longlong * tmpptr=tmpend; // tmpend points to the highest degree coeff of A
    for (;tmpptr!=tmp-1;--tmpptr,++remit)
      *tmpptr=*remit;
    vector<longlong>::const_iterator B_beg=other.begin(),B_end=other.end();
    longlong q;//n0(0),
    for (;a>=b;--a){
      q= amodpll(int128_t(invcoeff)*(*tmpend),m,invm);
      quo.push_back(q);
      --tmpend;
      // tmp <- tmp - q *B.shifted (if q!=0)
      if (q) {
	tmpptr=tmpend;
	vector<longlong>::const_iterator itq=B_beg;
	++itq; // first elements cancel
	for (;itq!=B_end;--tmpptr,++itq){ 
	  *tmpptr = amodpll(*tmpptr -(int128_t(q) * (*itq)),m,invm); // (*tmpptr -(int128_t(q) * (*itq)))%m;
	}
      }
    }
    // trim rem and multiply by coeff, this will modularize rem as well
    rem.clear();
    // bool trimming=true;
    for (;tmpend!=tmp-1;--tmpend){
      if (*tmpend && *tmpend % m)
	break;   
    }
    for (;tmpend!=tmp-1;--tmpend){
      rem.push_back(*tmpend);
    } 
#if defined VISUALC || defined BESTA_OS
    delete [] tmp;
#endif
  }

  void smallmultll(const vector<longlong> & a,const vector<longlong> & b,vector<longlong> & new_coord,longlong modulo){
    int128_t test=int128_t(modulo)*std::min(a.size(),b.size());
    bool large=test/(1ULL<<63) > (1ULL<<63)/modulo;
    new_coord.clear();
    if (a.empty() || b.empty()) return;
    vector<longlong>::const_iterator ita_begin=a.begin(),ita=a.begin(),ita_end=a.end(),itb=b.begin(),itb_end=b.end();
    for ( ; ita!=ita_end; ++ita ){
      vector<longlong>::const_iterator ita_cur=ita,itb_cur=itb;
      if (large){
	longlong res=0;
	for (;itb_cur!=itb_end;--ita_cur,++itb_cur) {
	  res = (res + *ita_cur * int128_t(*itb_cur))%modulo ;
	  if (ita_cur==ita_begin)
	    break;
	}
	new_coord.push_back(res % modulo);
      }
      else {
	int128_t res=0;
	for (;itb_cur!=itb_end;--ita_cur,++itb_cur) {
	  res += *ita_cur * int128_t(*itb_cur) ;
	  if (ita_cur==ita_begin)
	    break;
	}
	new_coord.push_back(res % modulo);
      }
    }
    --ita;
    ++itb;
    for ( ; itb!=itb_end;++itb){
      vector<longlong>::const_iterator ita_cur=ita,itb_cur=itb;
      if (large){
	longlong res=0;
	for (;;) {
	  res = (res + *ita_cur * int128_t(*itb_cur))%modulo ;
	  if (ita_cur==ita_begin)
	    break;
	  --ita_cur;
	  ++itb_cur;
	  if (itb_cur==itb_end)
	    break;
	}
	new_coord.push_back( res % modulo);
      }
      else {
	int128_t res= 0;
	for (;;) {
	  res += *ita_cur * int128_t(*itb_cur) ;
	  if (ita_cur==ita_begin)
	    break;
	  --ita_cur;
	  ++itb_cur;
	  if (itb_cur==itb_end)
	    break;
	}
	new_coord.push_back(res % modulo);
      }    
    }
  }

  void a_minus_qsize2_b(const vector<longlong> & ua,const vector<longlong> & q,const vector<longlong> &ub,vector<longlong> & ur,longlong p){
    ur.clear();
    int128_t q1=-q[0],q0=-q[1];
    long_double invp=long_double(1)/p;
    ur.push_back(amodpll(q1*ub.front(),p,invp));
    const longlong * it=&ub[0],*itend=it-1+ub.size(),*itmid=it+ub.size()-ua.size(),*jt=&ua[0];
    if (ua.empty()){
      for (;it!=itend;++it){
	ur.push_back(amodpll(q0*it[0]+q1*it[1],p,invp));
      }
      ur.push_back(amodpll(q0*it[0],p,invp));
    }
    else {
#if 1
      itmid-=4;
      longlong i0=it[0],i1;
      for (;it<itmid;it+=4){
	i1=it[1];
	ur.push_back(amodpll(q0*i0+q1*i1,p,invp));
	i0=it[2];
	ur.push_back(amodpll(q0*i1+q1*i0,p,invp));
	i1=it[3];
	ur.push_back(amodpll(q0*i0+q1*i1,p,invp));
	i0=it[4];	
	ur.push_back(amodpll(q0*i1+q1*i0,p,invp));
      }
      itmid+=4;
#endif
      for (;it!=itmid;++it){
	ur.push_back(amodpll(q0*it[0]+q1*it[1],p,invp));
      }
#if 1
      itend-=4;
      i0=it[0];
      for (;it<itend;it+=4,jt+=4){
	i1=it[1];
	ur.push_back(amodpll(q0*i0+q1*i1+jt[0],p,invp));
	i0=it[2];
	ur.push_back(amodpll(q0*i1+q1*i0+jt[1],p,invp));
	i1=it[3];
	ur.push_back(amodpll(q0*i0+q1*i1+jt[2],p,invp));
	i0=it[4];	
	ur.push_back(amodpll(q0*i1+q1*i0+jt[3],p,invp));
      }
      itend+=4;
#endif
      for (;it!=itend;++jt,++it){
	ur.push_back(amodpll(q0*it[0]+q1*it[1]+*jt,p,invp)); 
      }
      ur.push_back(amodpll(q0*it[0]+*jt,p,invp));
    }
  }

  // v <- w-v % m
  void submodnegll(vector<longlong> & v,const vector<longlong> & w,longlong m){
    vector<longlong>::iterator it=v.begin(),itend=v.end();
    vector<longlong>::const_iterator jt=w.begin(),jtend=w.end();
    longlong addv=longlong(jtend-jt)-longlong(itend-it);
    if (addv>0){
      v.insert(v.begin(),addv,0);
      it=v.begin();
      itend=v.end();
    }
    else {
      itend -= jtend-jt;
      for (;it!=itend;++it)
	*it = -*it;
      itend += jtend-jt;
    }
    for (;it!=itend;++jt,++it){
      longlong a=*it,b=*jt;
      a += (a>>63)&m;
      b += (b>>63)&m;
      *it = b-a;
    }
    for (it=v.begin();it!=itend;++it){
      if (*it)
	break;
    }
    if (it!=v.begin())
      v.erase(v.begin(),it);
  }

  bool hgcd_iter_ll(const vector<longlong> & a0i,const vector<longlong> & b0i,longlong m,vector<longlong> & ua,vector<longlong> & ub,vector<longlong> & va,vector<longlong> &vb,longlong p,vector<longlong> & coeffv,vector<longlong> & degv,vector<longlong> &a,vector<longlong> & b,vector<longlong> & q,vector<longlong> & r,vector<longlong> & ur,vector<longlong> & vr){
    if (debug_infolevel>2)
      CERR << CLOCK()*1e-6 << " halfgcd iter m=" << m << " dega0/a1 " << a0i.size() << "," << b0i.size() << '\n';
    longlong as=a0i.size();
    a.resize(a0i.size());
    b.resize(b0i.size());
    copy(a0i.begin(),a0i.end(),a.begin());
    copy(b0i.begin(),b0i.end(),b.begin());
    r.reserve(as);
    // initializes ua to 1 and ub to 0, the coeff of u in ua*a+va*b=a
    ua.reserve(as); ua.clear(); ua.push_back(1); ub.clear(); ub.reserve(as); ur.clear(); ur.reserve(as); 
    va.reserve(as); va.clear(); vb.clear(); vb.reserve(as); vb.push_back(1); vr.clear(); vr.reserve(as);
    vector<longlong>::iterator it,itend;
    // DivRem: a = bq+r 
    // hence ur <- ua-q*ub, vr <- va-q*vb verify
    // ur*a+vr*b=r
    // a <- b, b <- r, ua <- ub and ub<- ur
#if 1
    for (;;){
      int n=int(b.size())-1;
      if (n<m){ // degree(b) is small enough
	if (debug_infolevel>2)
	  CERR << CLOCK()*1e-6 << " halfgcd iter end" << a0i.size() << "," << b0i.size() << '\n';
	return true;
      }
      if (!degv.empty()){
	degv.push_back(degv.back()+b.size()-a.size());
	coeffv.push_back(b[0]);
      }
      DivRem(a,b,p,q,r); // division works always
      swap(a,b); swap(b,r); // a=b; b=r;
      // ur=ua-q*ub, ua<-ub, ub<-ur
      if (q.size()==2){ // here ua.size()<ub.size()
	if (ub.empty())
	  swap(ua,ub);
	else {
	  a_minus_qsize2_b(ua,q,ub,ur,p);
	  swap(ua,ub); swap(ub,ur);
	}
	a_minus_qsize2_b(va,q,vb,vr,p);
	swap(va,vb); swap(vb,vr);
	continue;
      }
      if (ub.empty())
	swap(ua,ub);
      else {
	smallmultll(q,ub,ur,p);
	submodnegll(ur,ua,p);
	swap(ua,ub); swap(ub,ur); // ua=ub; ub=ur;
      }
      if (vb.size()==1 && vb.front()==1) vr.swap(q); else smallmultll(q,vb,vr,p);
      submodnegll(vr,va,p);
      swap(va,vb); swap(vb,vr); // ua=ub; ub=ur;
    }
    return false; // never reached
#else
    for (;;){
      longlong n=longlong(b.size())-1;
      if (n<m){ // degree(b) is small enough
	if (debug_infolevel>2)
	  CERR << CLOCK()*1e-6 << " halfgcd iter compute v " << a0i.size() << "," << b0i.size() << '\n';
	// va=(a-ua*a0i)/b0i
	smallmultll(ua,a0i,ur,p);
	submodnegll(ur,a,p);
	DivRem(ur,b0i,p,va,r); // shoud be va
	// vb=(b-ub*a0i)/b0i
	smallmultll(ub,a0i,ur,p);
	submodnegll(ur,b,p);
	DivRem(ur,b0i,p,vb,r); // should be vb
	if (debug_infolevel>2)
	  CERR << CLOCK()*1e-6 << " halfgcd iter end" << a0i.size() << "," << b0i.size() << '\n';
	//CERR << a0 << " " << a1 << " " << A << " " << B << " " << C << " " << D << '\n';
	return true;
      }
      if (!degv.empty()){
	degv.push_back(degv.back()+b.size()-a.size());
	coeffv.push_back(b[0]);
      }
      DivRem(a,b,p,q,r); // division works always
      swap(a,b); swap(b,r); // a=b; b=r;
      // ur=ua-q*ub, ua<-ub, ub<-ur
      if (ub.empty()){
	swap(ua,ub);
	continue;
      }
      if (q.size()==2){ // here ua.size()<ub.size()
	ur.clear();
	a_minus_qsize2_b(ua,q,ub,ur,p);
	swap(ua,ub); swap(ub,ur);continue;
      }
      smallmultll(q,ub,ur,p);
      submodnegll(ur,ua,p);
      swap(ua,ub); swap(ub,ur); // ua=ub; ub=ur;
    }
    return false; // never reached
#endif
  }

  // reverse *a..*b and neg
  void fft_rev1(longlong * a,longlong *b,int128_t p){
    for (;b>a;++a,--b){
      longlong tmp=*a;
      *a=p-*b;
      *b=p-tmp;
    }
    if (a==b)
      *a=p-*a;
  }

  void fft_reverse(vector<longlong> & W,longlong p){
    if (W.size()<2)
      return;
    longlong * a=&W.front();
#ifdef GIAC_LLPRECOND
    longlong N=W.size()/2;
    fft_rev1(a+1,a+N-1,p);
    fft_rev1(a+N+1,a+2*N-1,1);
#else
    fft_rev1(a+1,a+W.size()-1,p);
#endif
  }

#ifdef GIAC_LLPRECOND // preconditionned
  void fft2wp(vector<longlong> & W,longlong n,longlong w,longlong p){
    W.resize(n); 
    w=w % p;
    if (w<0) w += p;
    longlong N=n/2;
    ulonglong ww=1;
    for (longlong i=0;i<N;++i){
      W[i]=ww;
      ulonglong u=preconditionner_ll(ww,p);
      W[N+i]=u;
      ww=precond_mulmodll(w,ww,u,p);
      //ww=(ww*int128_t(w))%p;
    }
  }
#else
  void fft2wp(vector<longlong> & W,longlong n,longlong w,longlong p){
    W.reserve(n/2); 
    long_double invp=long_double(1)/p;
    w=amodpll(w,p,invp);//w % p;
    if (w<0) w += p;
    longlong N=n/2,ww=1;
    for (longlong i=0;i<N;++i){
      W.push_back(ww);
      ww=mulmodll(ww,w,p,invp);//(ww*int128_t(w))%p;
      ww+=(ww>>63)&p;
    }
  }
#endif

  void fft2wp5(vector<longlong> & W,longlong n,longlong w){
    W.reserve(n/2); 
    w=w % p5;
    if (w<0) w += p5;
    longlong N=n/2,ww=1;
    for (longlong i=0;i<N;++i){
      W.push_back(ww);
      ww=(ww*int128_t(w))%p5;
    }
  }

#ifdef GIAC_LLPRECOND
  inline void fft_loop_p(longlong & A,longlong & An2,longlong * W,longlong n2,longlong p,long_double invp){
    longlong s=A;
    longlong t=precond_mulmodll(An2,*W,*(W+n2),p);
    //longlong t1=mulmodll(*W,An2,p,invp); t1+=(t1>>63)&p;if ((t-t1)%p!=0)
    //  CERR << "err\n";
    A = addmodll(s,t,p);
    An2 = submodll(s,t,p); 
  }
  inline void fft_loop_p(longlong & A,longlong & An2,longlong W,longlong Winv,longlong p){
    longlong s=A;
    longlong t=precond_mulmodll(An2,W,Winv,p);
    //longlong t1=mulmodll(*W,An2,p,invp); t1+=(t1>>63)&p;if ((t-t1)%p!=0)
    //  CERR << "err\n";
    A = addmodll(s,t,p);
    An2 = submodll(s,t,p); 
  }
#else
  inline void fft_loop_p(longlong & A,longlong & An2,longlong * W,longlong n2,longlong p,long_double invp){
    longlong s=A;
    longlong t = mulmodll(*W,An2,p,invp);
    A = addmodll(s,t,p);
    An2 = submodll(s,t,p); 
  }
#endif

#if !defined NUMWORKS // !defined VISUALC && !defined USE_GMP_REPLACEMENTS && defined GIAC_LLPRECOND // de-recurse
  static void fft2pnopermbefore( longlong *A, longlong n, longlong *W,longlong p,long_double invp,longlong step) {  
    if (n==0)
      CERR << "bug\n";
    if (n<=1 ) return;
    if (n==2){
      longlong f0=A[0],f1=A[1];
      A[0]=addmodll(f0,f1,p);
      A[1]=submodll(f0,f1,p);
      return;
    }
    longlong n2s=n/2*step;
    // start by groups of 4
    step=n2s/2;
    longlong w1=W[step],w1surp=W[3*step];
    longlong *Aeff=A;
    for (longlong pos=0;pos<n;pos+=4,Aeff+=4){
      longlong f0=Aeff[0],f1=Aeff[1],f2=Aeff[2],f3=Aeff[3],
	f01=precond_mulmodll(f1-f3+p,w1,w1surp,p),
	f02p=addmodll(f0,f2,p),f02m=submodll(f0,f2,p),f13=addmodll(f1,f3,p);
      Aeff[0]=addmodll(f02p,f13,p);
      Aeff[1]=addmodll(f02m,f01,p);
      Aeff[2]=submodll(f02p,f13,p);
      Aeff[3]=submodll(f02m,f01,p);
    }
    longlong Wstack_[MAX_INTSTACK/2];
    longlong *Wstack=0;
    if (n>MAX_INTSTACK/2)
      Wstack=(longlong *)malloc(n*sizeof(longlong));
    else
      Wstack=Wstack_;
    // now by 8, then by 16, etc.
    for (longlong taille=8;taille<=n;taille*=2){
      step /= 2;
      Aeff=A;
      if (taille==n && step==1){
	longlong *An2=Aeff+n/2,*Aend=An2,*Weff=W+n2s;
	for(; Aeff<Aend; ) {
	  fft_loop_p(Aeff[0],An2[0],W[0],Weff[0],p);
	  fft_loop_p(Aeff[1],An2[1],W[1],Weff[1],p);
	  fft_loop_p(Aeff[2],An2[2],W[2],Weff[2],p);
	  fft_loop_p(Aeff[3],An2[3],W[3],Weff[3],p);
	  Aeff+=4; An2+=4; W+=4; Weff+=4;
	}
	break;
      }
      longlong * end=Wstack+taille,*source=W,*source2=W+n2s;
      for (longlong * target=Wstack;target<end;target+=8){
	target[0]=*source; source+=step;
	target[1]=*source; source+=step;
	target[2]=*source; source+=step;
	target[3]=*source; source+=step;
	target[4]=*source2; source2+=step;
	target[5]=*source2; source2+=step;
	target[6]=*source2; source2+=step;
	target[7]=*source2; source2+=step;
      }
      for (longlong pos=0;pos<n;pos+=taille){
	longlong *An2=Aeff+taille/2,*Aend=An2,*Weff=Wstack;
	longlong s=*Aeff,t1=*An2;
	*Aeff=addmodll(s,t1,p);
	*An2=submodll(s,t1,p);
	fft_loop_p(Aeff[1],An2[1],Weff[1],Weff[5],p);
	fft_loop_p(Aeff[2],An2[2],Weff[2],Weff[6],p);
	fft_loop_p(Aeff[3],An2[3],Weff[3],Weff[7],p);
	Aeff+=4; An2+=4; Weff+=8;
	for (;Aeff<Aend;){
	  fft_loop_p(Aeff[0],An2[0],Weff[0],Weff[4],p);
	  fft_loop_p(Aeff[1],An2[1],Weff[1],Weff[5],p);
	  fft_loop_p(Aeff[2],An2[2],Weff[2],Weff[6],p);
	  fft_loop_p(Aeff[3],An2[3],Weff[3],Weff[7],p);
	  Aeff+=4; An2+=4; Weff+=8;
	}
	Aeff+=taille/2;
      }
    }
    if (n>MAX_INTSTACK/2)
      free(Wstack);
  }

#else // de-recurse
  static void fft2pnopermbefore( longlong *A, longlong n, longlong *W,longlong p,long_double invp,longlong step) {  
    if ( n==1 ) return;
    // if p is fixed, the code is about 2* faster
    if (n==4){
      longlong w1=W[step];
      longlong f0=A[0],f1=A[1],f2=A[2],f3=A[3],
#ifdef GIAC_LLPRECOND
	f01=precond_mulmodll(submodll(f1,f3,p),w1,W[3*step],p),
#else
	f01=mulmodll(submodll(f1,f3,p),w1,p,invp),
#endif
	f02p=addmodll(f0,f2,p),f02m=submodll(f0,f2,p),f13=addmodll(f1,f3,p);
      A[0]=addmodll(f02p,f13,p);
      A[1]=addmodll(f02m,f01,p);
      A[2]=submodll(f02p,f13,p);
      A[3]=submodll(f02m,f01,p);
      return;
    }
    if (n==2){
      longlong f0=A[0],f1=A[1];
      A[0]=addmodll(f0,f1,p);
      A[1]=submodll(f0,f1,p);
      return;
    }
    fft2pnopermbefore(A, n/2, W,p,invp,2*step); 
    fft2pnopermbefore(A+n/2, n/2, W,p,invp,2*step); 
    longlong * An2=A+n/2;
    longlong * Aend=A+n/2;
    longlong n2s = n/2*step; // n2%4==0
    for(; A<Aend; ) {
      fft_loop_p(*A,*An2,W,n2s,p,invp);
      ++A; ++An2; W +=step ;
      fft_loop_p(*A,*An2,W,n2s,p,invp);
      ++A; ++An2; W +=step ;
      fft_loop_p(*A,*An2,W,n2s,p,invp);
      ++A; ++An2; W += step;
      fft_loop_p(*A,*An2,W,n2s,p,invp);
      ++A; ++An2; W +=step;
    }
  }  
#endif

#ifdef GIAC_LLPRECOND
  inline void fft_loop_p_(longlong & Acur,longlong & An2cur,longlong * Wcur,longlong n2,longlong p,long_double invp){
    longlong Ai,An2i;
    Ai=Acur;
    An2i=An2cur;
    Acur = addmodll(Ai,An2i,p);
    An2cur=precond_mulmodll((Ai-An2i)+p,*Wcur,*(Wcur+n2),p);
  }
#else
  inline void fft_loop_p_(longlong & Acur,longlong & An2cur,longlong * Wcur,longlong n2,longlong p,long_double invp){
    longlong Ai,An2i;
    Ai=Acur;
    An2i=An2cur;
    Acur = addmodll(Ai,An2i,p);
    An2cur=amodpll((int128_t(Ai)+(p-An2i))* *Wcur,p,invp);
    An2cur += (An2cur>>63)&p;
    return;
    longlong chk=(((int128_t(Ai)+(p-An2i))* *Wcur) % p);
    if ( An2cur!=chk) //(An2cur-int128_t(chk))%p!=0)
      CERR<<"err\n";
    An2cur=chk;
  }
#endif

#if !defined NUMWORKS // !defined VISUALC && !defined USE_GMP_REPLACEMENTS && defined GIAC_LLPRECOND // de-recurse
  static void fft2pnopermafter( longlong *A, longlong n, longlong *W,longlong p,long_double invp,longlong step) {  
    if (n==0)
      CERR << "bug\n";
    if (n<=1 ) return;
    if (n==2){
      longlong f0=A[0],f1=A[1];
      A[0]=addmodll(f0,f1,p);
      A[1]=submodll(f0,f1,p);
      return;
    }
    longlong n2s=n/2*step;
    // group by decreasing size
    longlong Wstack_[MAX_INTSTACK/2];
    longlong *Wstack=0;
    if (n>MAX_INTSTACK/2)
      Wstack=(longlong *)malloc(n*sizeof(longlong));
    else
      Wstack=Wstack_;
    longlong * end=Wstack+n,*source=W,*source2=W+n2s;
    for (longlong * target=Wstack;target<end;target+=8){
      target[0]=*source; source+=step;
      target[1]=*source; source+=step;
      target[2]=*source; source+=step;
      target[3]=*source; source+=step;
      target[4]=*source2; source2+=step;
      target[5]=*source2; source2+=step;
      target[6]=*source2; source2+=step;
      target[7]=*source2; source2+=step;
    }
    //size_t T=n*sizeof(longlong);
    //longlong * Wstack=(longlong*)stack_or_heap_alloc(T);//longlong Wstack[taille];
    for (longlong taille=n;taille>=8;taille/=2){
      longlong * Aeff=A;
      for (longlong pos=0;pos<n;pos+=taille){
	longlong *An2=Aeff+taille/2,*Aend=An2,*Weff=Wstack;
	longlong s=*Aeff,t1=*An2;
	*Aeff=addmodll(s,t1,p);
	*An2=submodll(s,t1,p);
	fft_loop_p_(Aeff[1],An2[1],&Weff[1],4,p,invp);
	fft_loop_p_(Aeff[2],An2[2],&Weff[2],4,p,invp);
	fft_loop_p_(Aeff[3],An2[3],&Weff[3],4,p,invp);
	Aeff+=4; An2+=4; Weff+=8;
	for (;Aeff<Aend;){
	  fft_loop_p_(Aeff[0],An2[0],&Weff[0],4,p,invp);
	  fft_loop_p_(Aeff[1],An2[1],&Weff[1],4,p,invp);
	  fft_loop_p_(Aeff[2],An2[2],&Weff[2],4,p,invp);
	  fft_loop_p_(Aeff[3],An2[3],&Weff[3],4,p,invp);
	  Aeff+=4; An2+=4; Weff+=8;
	}
	Aeff+=taille/2;
      }
      if (taille==8)
	break;
      longlong * end=Wstack+taille,*source=Wstack;
      for (longlong * target=Wstack;source<end;source+=16,target+=8){
	target[0]=source[0];
	target[1]=source[2];
	target[4]=source[4];
	target[5]=source[6];
	target[2]=source[8];
	target[3]=source[10];
	target[6]=source[12];
	target[7]=source[14];
      }
    }
    if (n>MAX_INTSTACK/2)
      free(Wstack);
    // finish by groups of 4
    step=n2s/2;
    longlong w1=W[step],w1surp=W[3*step];
    longlong *Aeff=A;
    for (longlong pos=0;pos<n;pos+=4,Aeff+=4){
      longlong f0=Aeff[0],f1=Aeff[1],f2=Aeff[2],f3=Aeff[3],
	f01=precond_mulmodll(f1-f3+p,w1,w1surp,p),
	f02p=addmodll(f0,f2,p),f02m=submodll(f0,f2,p),f13=addmodll(f1,f3,p);
      Aeff[0]=addmodll(f02p,f13,p);
      Aeff[1]=addmodll(f02m,f01,p);
      Aeff[2]=submodll(f02p,f13,p);
      Aeff[3]=submodll(f02m,f01,p);
    }
  }

#else // de-recurse
  static void fft2pnopermafter( longlong *A, longlong n, longlong *W,longlong p,long_double invp,longlong step) {  
    if (n==1) return;
    if (n==4){
      longlong w1=W[step];
      longlong f0=A[0],f1=A[1],f2=A[2],f3=A[3],
#ifdef GIAC_LLPRECOND
	f01=precond_mulmodll(submodll(f1,f3,p),w1,W[3*step],p),
#else
	f01=mulmodll(submodll(f1,f3,p),w1,p,invp),
#endif
	f02p=addmodll(f0,f2,p),f02m=submodll(f0,f2,p),f13=addmodll(f1,f3,p);
      A[0]=addmodll(f02p,f13,p);
      A[1]=addmodll(f02m,f01,p);
      A[2]=submodll(f02p,f13,p);
      A[3]=submodll(f02m,f01,p);
      return;
    }
    if (n==2){
      longlong f0=A[0],f1=A[1];
      A[0]=addmodll(f0,f1,p);
      A[1]=submodll(f0,f1,p);
      return;
    }
    // Step 1 : arithmetic
    longlong *An2=A+n/2;
    longlong * Acur=A,*An2cur=An2,*Wcur=W;
    longlong n2=n/2*step;
    for (;Acur!=An2;){
      longlong Ai,An2i;
      fft_loop_p_(*Acur,*An2cur,Wcur,n2,p,invp);
      ++Acur;++An2cur; Wcur +=step;
      fft_loop_p_(*Acur,*An2cur,Wcur,n2,p,invp);
      ++Acur;++An2cur; Wcur += step;
      fft_loop_p_(*Acur,*An2cur,Wcur,n2,p,invp);
      ++Acur;++An2cur; Wcur += step;
      fft_loop_p_(*Acur,*An2cur,Wcur,n2,p,invp);
      ++Acur;++An2cur; Wcur += step;
    }
    // Step 2 : recursive calls
    fft2pnopermafter(A, n/2, W,p,invp,2*step);
    fft2pnopermafter(An2, n/2, W,p,invp,2*step);
  }  
#endif

  // a=source mod x^N-1 mod p
  void reverse_assign(const vector<longlong> & source,vector<longlong> & a,longlong N,longlong p){
    a.clear(); a.resize(N);
    if (source.empty()) return;
    const longlong * stop=&*source.begin(),*start=&*source.end()-1;
    longlong i=0;
    for (;i<N && start>=stop;i++,--start){
      longlong k=*start;
      k += (k>>63)&p; // add p if k is negative
      a[i]=k;
    }
    for (i=0;start>=stop;--start){
      longlong k=*start;
      k += (k>>63)&p; 
      k += (a[i]-p);
      k += (k>>63)&p; 
      a[i]= k ; 
      ++i;
      if (i==N)
	i=0;
    }
  }
  
  void makemodulop(longlong * a,longlong as,longlong modulo){
    longlong *aend=a+as;
    for (;a!=aend;++a){
      *a %= modulo;
      // if (*a<0) *a += modulo; // *a -= (unsigned(modulo-*a)>>31)*modulo;
    }
  }

  void makepositive(longlong * p,longlong n,longlong modulo){
    longlong * pend=p+n;
    for (;p!=pend;++p){
      longlong P=*p;
      P += (P>>63) & modulo;
      P += (P>>63) & modulo;
      *p=P;
    }
  }

  void to_fft(const std::vector<longlong> & a,longlong modulo,longlong w,std::vector<longlong> & Wp,longlong n,std::vector<longlong> & f,bool reverse,bool makeplus,bool makemod=true){
    long_double invp=long_double(1)/modulo;
#if defined GIAC_LLPRECOND
    longlong nw=n;
#else
    longlong nw=n/2;
#endif
    longlong s=giacmin(a.size(),n);
    longlong logrs=sizeinbase2(n-1);
    if (reverse){
      if (&f==&a){
	if (f.size()>n){
	  vector<longlong> tmp(n);
	  reverse_assign(a,tmp,n,modulo);
	  tmp.swap(f);
	}
	else {
	  vector<longlong>::iterator it=f.begin(),itend=f.end();
	  for (;it!=itend;++it)
	    *it += (*it>>63)&modulo;
	  std::reverse(f.begin(),f.end());
	  f.resize(n);
	}
      }
      else {
	f.resize(n);
	reverse_assign(a,f,n,modulo);
      }
    }
    else {
      if (&f!=&a) 
	f=a;
      f.resize(n);
    }
    if (makemod)
      makemodulop(&f.front(),s,modulo);
    if (makeplus) makepositive(&f.front(),s,modulo);
    if (Wp.size()<nw || Wp[0]==0){
      Wp.clear();
      fft2wp(Wp,n,w,modulo);
    }
    fft2pnopermafter(&f.front(),n,&Wp.front(),modulo,invp,Wp.size()/nw);
  }

  void from_fft(const std::vector<longlong> & f,longlong p,std::vector<longlong> & Wp,std::vector<longlong> & res,bool reverseatend,bool revw){
    long_double invp=long_double(1)/p;
    if (&res!=&f) res=f;
    longlong n=res.size();
#if defined GIAC_LLPRECOND
    int nw=n;
#else
    int nw=n/2;
#endif
    if (revw) fft_reverse(Wp,p);
    fft2pnopermbefore(&res.front(),n,&Wp.front(),p,invp,Wp.size()/nw);
    if (revw) fft_reverse(Wp,p);
    longlong i=invmodll(n,p);
    //mulmodll(res,i,p,invp);
    i += (i>>63)&p;
    precond_mulmodll(res,i,preconditionner_ll(i,p),p);
    if (reverseatend)
      reverse(res.begin(),res.end());
  }

  void fft_ab_cd_p(const vector<longlong> &a,const vector<longlong> &b,const vector<longlong> & c,const vector<longlong> &d,vector<longlong> & res,longlong p){
    long_double invp=long_double(1)/p;
    longlong s=a.size();
    res.resize(s);
    for (longlong i=0;i<s;++i){
      res[i]=amodpll(int128_t(a[i])*b[i]+int128_t(c[i])*d[i],p,invp);
    }
  }

  longlong powmodll(longlong a,ulonglong n,longlong m,long_double invm){
    if (!n)
      return 1;
    if (n==1)
      return a;
    if (n==2)
      return amodpll(a*int128_t(a),m,invm);
    longlong b=a%m,c=1;
    while (n>0){
      if (n%2)
	c=amodpll(c*int128_t(b),m,invm);
      n /= 2;
      b=amodpll(b*int128_t(b),m,invm);
    }
    return c;
  }

  longlong powmodll(longlong a,ulonglong n,longlong m){
    return powmodll(a,n,m,long_double(1)/m);
  }

  // for p prime such that p-1 is divisible by 2^N, compute a 2^N-th root of 1
  // otherwise return 0
  longlong nthroot(longlong p,longlong N){
    longlong expo=(p-1)>>N;
    if ( (expo<<N)!=p-1)
      return 0;
    long_double invp=long_double(1)/p;
    for (longlong n=2;;++n){
      longlong w=powmodll(n,expo,p,invp); // w=n^((p-1)/2^N)
      int128_t r=w;
      for (longlong i=1;i<N;++i)
	r=amodpll(r*r,p,invp);
      if (r==p-1) // r=w^(2^(N-1))=n^((p-1)/2)
	return w;
    }
  }

  longlong find_wll(vector<longlong> & Wp,longlong shift,longlong p){
    longlong n=1<<shift,w=0;
#if defined GIAC_LLPRECOND
    longlong ws=Wp.size();
#else
    longlong ws=2*Wp.size();
#endif
    if (ws/n){
      w=Wp[ws/n];
      longlong wp=powmodll(w,n/2,p);
      if (wp!=p-1){
	w=0; Wp.clear();
      }
      //CERR << Wp << endl;
    }
    if (w==0 && p!=p1 && p!=p2 && p!=p3)
      w=nthroot(p,shift);
    return w;
  }

  // [[RA,RB],[RC,RD]]*[a0,a1]->[a,b]
  bool matrix22lltimesvect(const vector<longlong> & RA,const vector<longlong> & RB,const vector<longlong> & RC,const vector<longlong> & RD,const vector<longlong> & a0,const vector<longlong> &a1,longlong maxadeg,longlong maxbdeg,vector<longlong> & a,vector<longlong> &b,longlong p,vector<longlong> & ra,vector<longlong> & rb,vector<longlong> & rc,vector<longlong> & rd,vector<longlong> &Wp){
    longlong dega0=a0.size()-1,m=(dega0+1)/2;
    longlong maxabdeg=giacmax(maxadeg,maxbdeg);
    longlong bbsize=giacmin(maxabdeg+1,a0.size());
    longlong ddsize=giacmin(maxabdeg+1,a1.size());
    longlong Nreal=giacmax(bbsize+RC.size(),ddsize+RD.size())-2;
    int N2=giacmin(maxabdeg,Nreal);
    unsigned long l=sizeinbase2(N2)-1; 
    longlong n=1<<(l+1);
    longlong w=find_wll(Wp,l+1,p);
    // vector<longlong> adbg,bdbg;
    if (!w)
      return false;
    to_fft(RA,p,w,Wp,n,b,true,false,false);ra.swap(b);
    to_fft(RB,p,w,Wp,n,b,true,false,false);rb.swap(b);
    to_fft(RC,p,w,Wp,n,b,true,false,false);rc.swap(b);
    to_fft(RD,p,w,Wp,n,b,true,false,false);rd.swap(b);
    to_fft(a0,p,w,Wp,n,a,true,false,false);
    to_fft(a1,p,w,Wp,n,b,true,false,false);
    fft_reverse(Wp,p); 
    fft_ab_cd_p(rc,a,rd,b,rc,p);
    from_fft(rc,p,Wp,rc,true,false);
    fft_ab_cd_p(ra,a,rb,b,ra,p);
    from_fft(ra,p,Wp,ra,true,false);
    a.swap(ra);
    b.swap(rc);
    trim_deg(a,maxabdeg);
    fast_trim_inplace(a,p);
    trim_deg(b,maxabdeg);
    fast_trim_inplace(b,p);
    return true;
  }

  bool matrix22ll(vector<longlong> & RA,vector<longlong> &RB,vector<longlong> & RC,vector<longlong> &RD,vector<longlong> &SA,vector<longlong> &SB,vector<longlong> &SC,vector<longlong> &SD,vector<longlong> &A,vector<longlong> &B,vector<longlong> &C,vector<longlong> &D,longlong p,vector<longlong> & tmp,vector<longlong> & Wp){
    // 2x2 matrix operations
    // [[SA,SB],[SC,SD]]*[[RC,RD],[RA,RB]] == [[RA*SB+RC*SA,RB*SB+RD*SA],[RA*SD+RC*SC,RB*SD+RD*SC]]
    int Nreal=giacmax(giacmax(RC.size(),RD.size()),giacmax(RA.size(),RB.size()))+giacmax(giacmax(SC.size(),SD.size()),giacmax(SA.size(),SB.size()))-2;
    unsigned long l=sizeinbase2(Nreal)-1; // l=gen(Nreal).bindigits()-1; // m=2^l <= Nreal < 2^{l+1}
    unsigned long n=1<<(l+1);
    longlong w=nthroot(p,l+1);
    // vector<longlong> adbg,bdbg;
    if (!w)
      return false;
    // makepositive set to false since reverse_assign should make RA positive
    to_fft(SC,p,w,Wp,n,SC,true,false,false); 
    to_fft(SD,p,w,Wp,n,SD,true,false,false); 
    to_fft(RA,p,w,Wp,n,RA,true,false,false); 
    to_fft(RB,p,w,Wp,n,RB,true,false,false); 
    to_fft(RC,p,w,Wp,n,RC,true,false,false); 
    to_fft(RD,p,w,Wp,n,RD,true,false,false); 
    to_fft(SA,p,w,Wp,n,SA,true,false,false); 
    to_fft(SB,p,w,Wp,n,SB,true,false,false); 
    fft_reverse(Wp,p); 
    fft_ab_cd_p(RA,SB,RC,SA,A,p);
    from_fft(A,p,Wp,A,true,false);
    fft_ab_cd_p(RB,SB,RD,SA,SA,p); SA.swap(B);
    from_fft(B,p,Wp,B,true,false);
    fft_ab_cd_p(RA,SD,RC,SC,RA,p); RA.swap(C);
    from_fft(C,p,Wp,C,true,false);
    fft_ab_cd_p(RB,SD,RD,SC,RB,p); RB.swap(D);
    from_fft(D,p,Wp,D,true,false);
    //       fft_reverse(Wp,p); 
    fast_trim_inplace(A,p);
    fast_trim_inplace(B,p);
    fast_trim_inplace(C,p);
    fast_trim_inplace(D,p);
    return true;
  }

  void a_bc(const vector<longlong> &a,const vector<longlong> &b,const vector<longlong> &c,longlong p,vector<longlong> & res,vector<longlong> & tmp1){
    // res=trim(a-b*c,env); return;
    size_t as=a.size(),bs=b.size();
    if (as<=bs){
      tmp1.clear(); tmp1.reserve(bs);
      if (c.size()==2){
	a_minus_qsize2_b(a,c,b,tmp1,p);
	tmp1.swap(res); return;
      }
    }
    smallmultll(b,c,tmp1,p);
    submodnegll(tmp1,a,p);
    tmp1.swap(res);
  }

  bool hgcdll(const vector<longlong> & a0,const vector<longlong> & a1,longlong modulo,vector<longlong> & Wp,vector<longlong> &A,vector<longlong> &B,vector<longlong> &C,vector<longlong> &D,vector<longlong> & coeffv,vector<longlong> & degv,vector<longlong> & q,vector<longlong> & f,vector<longlong> & tmp0,vector<longlong> & tmp1,vector<longlong> & tmp2,vector<longlong> & tmp3){ // a0 is A in Yap, a1 is B
    vector<longlong> & g0=tmp2,&g1=tmp3;
    longlong dega0=a0.size()-1,dega1=a1.size()-1;
    longlong m=(dega0+1)/2;
    if (dega1<m){
      D=A=vector<longlong>(1,1);
      B.clear(); C.clear();
      return true;
    }
    if (m<HGCD/2){ 
      hgcd_iter_ll(a0,a1,m,A,C,B,D,modulo,coeffv,degv,q,f,tmp0,tmp1,tmp2,tmp3);
      return true;
    }
    vector<longlong> b0(a0.begin(),a0.end()-m); // quo(a0,x^m), A0 in Yap
    vector<longlong> b1(a1.begin(),a1.end()-m); // quo(a1,x^m), B0 in Yap
    // 1st recursive call
    vector<longlong> RA,RB,RC,RD;
    if (debug_infolevel>2)
      CERR << CLOCK()*1e-6 << " hgcdll 1st recursive call " << dega0 << "," << dega1 << '\n';
    if (!hgcdll(b0,b1,modulo,Wp,RA,RB,RC,RD,coeffv,degv,tmp0,tmp1,A,B,C,D))
      return false;
    if (debug_infolevel>2)
      CERR << CLOCK()*1e-6 << " hgcdll compute A' B' " << dega0 << "," << dega1 << '\n';
    longlong maxadeg=dega0+1-giacmax(RA.size(),RB.size()),maxbdeg=dega0-m/2;
    matrix22lltimesvect(RA,RB,RC,RD,a0,a1,maxadeg,maxbdeg,b0,b1,modulo,tmp0,tmp1,tmp2,tmp3,Wp);
    longlong dege=b1.size()-1;
    if (dege<m){
      A.swap(RA); B.swap(RB); C.swap(RC); D.swap(RD);  
      return true;
      // A=RA; B=RB; C=RC; D=RD; return true;
    }
    if (dege>=b0.size()-1)
      COUT << "hgcdll error" << '\n';
    if (debug_infolevel>2)
      CERR << CLOCK()*1e-6 << " hgcdll euclid div " << dega0 << "," << dega1 << '\n';
    // 1 euclidean step
    if (!degv.empty()){
      coeffv.push_back(b1[0]);
      degv.push_back(degv.back()+b1.size()-b0.size());
    }
    DivRem(b0,b1,modulo,q,f); // q,f are Q,D in Yap 
    // [[0,1],[1,-q]]*[[RA,RB],[RC,RD]] == [[RC,RD],[-RC*q+RA,-RD*q+RB]]
    a_bc(RA,RC,q,modulo,RA,tmp1); // RA=trim(RA-RC*q,&env);
    a_bc(RB,RD,q,modulo,RB,tmp1); // RB=trim(RB-RD*q,&env);
    longlong l=b1.size()-1,k=2*m-l;
    if (f.size()-1<m){
      A.swap(RC); B.swap(RD); C.swap(RA); D.swap(RB); return true;
    }
    g0.resize(b1.size()-k);
    copy(b1.begin(),b1.end()-k,g0.begin()); // vector<int> g0(b1.begin(),b1.end()-k); // quo(b,x^k), C0 in Yap
    if (f.size()>k){
      g1.resize(f.size()-k);
      copy(f.begin(),f.end()-k,g1.begin()); // quo(f,x^k), D0 in Yap
    }
    vector<longlong> &SA=b0,&SB=b1,&SC=q,&SD=f;
    if (debug_infolevel>2)
      CERR << CLOCK()*1e-6 << " hgcdll 2nd recursive call " << dega0 << "," << dega1 << '\n';
    if (!hgcdll(g0,g1,modulo,Wp,SA,SB,SC,SD,coeffv,degv,tmp0,tmp1,A,B,C,D))
      return false;
    if (debug_infolevel>2)
      CERR << CLOCK()*1e-6 << " hgcdll end 2nd recursive call " << dega0 << "," << dega1 << '\n';
    matrix22ll(RA,RB,RC,RD,SA,SB,SC,SD,A,B,C,D,modulo,tmp0,Wp);
    if (debug_infolevel>2)
      CERR << CLOCK()*1e-6 << " hgcdll end " << dega0 << "," << dega1 << '\n';
    return true;
  }

  void mulsmall(vector<longlong> & Q,longlong c,longlong m){
    if (c==1) return;
    //long_double invm=long_double(1)/long_double(m);
    longlong * ptr=&Q.front(), * ptrend=ptr+Q.size();
    for (;ptr!=ptrend;++ptr){
      //*ptr =amodpll(int128_t(*ptr)*c,m,invm);
      *ptr = (int128_t(*ptr)*c)%m;
    }
  }

  // resultant of P and Q modulo m, modifies P and Q, 
  longlong resultantll(vector<longlong> & P,vector<longlong> & Q,vector<longlong> & tmp1,vector<longlong> & tmp2,longlong m){
    if (P.size()<Q.size()){
      int res=(P.size() % 2==1 || Q.size() % 2==1)?1:-1; // (-1)^deg(P)*deg(Q)
      return res*resultantll(Q,P,tmp1,tmp2,m);
    }
    long_double invm=long_double(1)/m;
    if (P.size()==Q.size()){
      longlong coeff=Q[0];
      longlong invcoeff=invmodll(coeff,m);
      mulsmall(Q,invcoeff,m);
      DivRem(P,Q,m,tmp1,tmp2);
      int128_t res=(P.size() % 2==1)?1:-1;
      res *= powmodll(Q[0],longlong(P.size()-tmp2.size()),m,invm);
      return smodll(res*resultantll(Q,tmp2,P,tmp1,m), m);
    }
    // now P.size()>Q.size()
    int HGCD2=HGCD;
    if (Q.size()>=HGCD2){
      vector<longlong> coeffv,degv,A,B,C,D,a,b,b0,b1,b2,b3,b4,b5,b6,b7,Wp;
      coeffv.reserve(Q.size()+1);
      degv.reserve(Q.size()+1);
      degv.push_back(P.size()-1);
      while (Q.size()>=HGCD2){
	int deg1=P.size(),deg2=(3*deg1)/4;
	double coeff=nextpow2(deg1/2)*2./deg1;
	double coeff2=nextpow2(deg2)/double(deg2);
	coeff=0.5*std::min(coeff,coeff2);
	if (Wp.empty() && m!=p1 && m!=p2 && m!=p3){
	  longlong l=sizeinbase2(int(3*2*coeff/4*deg1-1));
	  longlong w=find_wll(Wp,l,m);
	  fft2wp(Wp,(1<<l),w,m);
	}
	if (debug_infolevel>2)
	  CERR << CLOCK()*1e-6 << " deg " << P.size() << " coeff " << coeff << "\n";
	int seuil=1+int(std::ceil((1-coeff)*P.size())); 
	if (HGCD/4>=Q.size()-seuil){
	  coeffv.push_back(Q.front());
	  degv.push_back(degv.back()+Q.size()-P.size());
	  DivRem(P,Q,m,a,b);
	  P.swap(Q);
	  Q.swap(b);
	  continue;
	}
	// 1st recursive call
	b0.resize(P.size()-seuil); 
	copy(P.begin(),P.end()-seuil,b0.begin()); // quo(P,x^s), 
	b1.resize(Q.size()-seuil);
	copy(Q.begin(),Q.end()-seuil,b1.begin()); // quo(Q,x^s), 
	hgcdll(b0,b1,m,Wp,A,B,C,D,coeffv,degv,b2,b3,b4,b5,b6,b7);
	longlong maxadeg=P.size()-giacmax(A.size(),B.size());
	matrix22lltimesvect(A,B,C,D,P,Q,maxadeg,maxadeg,a,b,m,b4,b5,b6,b7,Wp);
	if (b.size()<HGCD){
	  a.swap(P); b.swap(Q); break;
	}
	coeffv.push_back(b.front());
	degv.push_back(degv.back()+b.size()-a.size());
	DivRem(a,b,m,P,Q);
	b.swap(P); 
      }
      degv.push_back(Q.size()-1);
      longlong res=resultantll(P,Q,tmp1,tmp2,m);
      // adjust
      for (longlong i=0;i<coeffv.size();++i){
	if (degv[i]%2==1 && degv[i+1]%2==1)
	  res=-res;
	res=amodpll(int128_t(res)*powmodll(coeffv[i],degv[i]-degv[i+2],m),m,invm);
      }
      return smodll(res,m);
    }
    int128_t res=1;
    while (Q.size()>1){
#if 0
      longlong coeff=Q[0];
      longlong invcoeff=invmodll(coeff,m);
      mulsmall(Q,invcoeff,m);
      DivRem(P,Q,m,tmp1,tmp2);
      res = (res*powmodll(coeff,ulonglong(P.size()-1),m)) %m;
#else
      DivRem(P,Q,m,tmp1,tmp2);
      res = amodpll(res*powmodll(Q[0],P.size()-tmp2.size(),m,invm),m,invm);
#endif
      if (P.size()%2==0 && Q.size()%2==0)
	res = -res;
      P.swap(Q);
      Q.swap(tmp2);
    }
    if (Q.empty())
      return 0;
    res = amodpll(res*powmodll(Q[0],ulonglong(P.size()-1),m,invm),m,invm);
    return smodll(res,m);
  }

  void int2longlong(const vector<int> & p,vector<longlong> & P,int modulo){
    longlong m=modulo?modulo:p5;
    size_t s=p.size();
    if (P.size()<s)
      P.resize(s);
    for (size_t i=0;i<s;++i){
      longlong x=p[s-1-i];
      P[i]=x<0?x+m:x;
    }
  }

  bool fft2p5(const vector<int> & p,const vector<int> & q,vector<longlong> & PQ,vector<longlong> & W,int modulo){
    if (debug_infolevel)
      CERR << CLOCK()*1e-6 << "fft2p5 begin" << '\n';
    int ps=int(p.size()),qs=int(q.size()),rs=ps+qs-1;
    int logrs=sizeinbase2(rs);
    if (logrs>54) return false;
    int n=(1u<<logrs);
    vector<longlong> P(n),Q(n);
    int2longlong(p,P,modulo); 
    int2longlong(q,Q,modulo);
    if (W.empty() || W[0]==0){
      //const longlong r=4917923076487504807LL;
      longlong w=4917923076487504807LL;
      for (int i=0;i<54-logrs;++i)
	w=(int128_t(w)*w) % p5;
      // longlong w=powmodll(r,(1ul<<(54-logrs)),p5);
      fft2wp(W,n,w,p5);
    }
    fft2pnopermafter(&P.front(),n,&W.front(),p5,invp5,1);
    fft2pnopermafter(&Q.front(),n,&W.front(),p5,invp5,1);
    for (int i=0;i<n;++i){
      P[i]=mulmodll(P[i],Q[i],p5);
    }
    fft_reverse(W,p5);
    fft2pnopermbefore(&P.front(),n,&W.front(),p5,invp5,1);
    fft_reverse(W,p5);
    // divide by n
    longlong ninv=p5+(1-p5)/n;
    for (int i=0;i<rs;++i){
      P[i]=(int128_t(ninv)*P[i]) % p5; //mulmodll(ninv,P[i],p5);
      if (modulo){
	P[i]=smodll(P[i],modulo);
      }
      else {
	if (P[i]>p5/2)
	  P[i] -= p5;
      }
    }
    reverse(P.begin(),P.end());
    PQ.reserve(rs);
    int i;
    for (i=0;i<P.size();++i){
      if (P[i]!=0)
	break;
    }
    for (;i<P.size();++i)
      PQ.push_back(P[i]);
    if (debug_infolevel)
      CERR << CLOCK()*1e-6 << "fft2p5 end" << '\n';
    return true;
  }

#endif // INT128

  // find pseudo remainder of x mod p, 2^nbits>=p>2^(nbits-1)
  // assumes invp=2^(2*nbits)/p+1 has been precomputed 
  // and abs(x)<2^(31+nbits)
  // |remainder| <= max(2^nbits,|x|*p/2^(2nbits)), <=2*p if |x|<=p^2
  inline longlong pseudo_mod(longlong x,int p,unsigned invp){
    return x - (((x>>31)*invp)>>31)*p;
  }

  void fft_ab_p(const vector<int> &a,const vector<int> &b,vector<int> & res,int p){
    int s=a.size();
    res.resize(s);
#if 1 
    double invp=find_invp(p);
    for (int i=0;i<s;++i){
      res[i]=amodp(longlong(a[i])*b[i],p,invp);
    }
#else
    for (int i=0;i<s;++i){
      res[i]=(longlong(a[i])*b[i])%p;
    }
#endif
  }

  bool fft_aoverb_p(const vector<int> &a,const vector<int> &b,vector<int> & res,int p){
    int s=a.size();
    res.resize(s);
    for (int i=0;i<s;++i){
      if (a[i]==0){
	res[i]=0;
	continue;
      }
      int bi=b[i];
      if (bi==0) return false;
      invmod(bi,p);
      bi += (bi>>31)&p;
      res[i]=(longlong(a[i])*bi)%p;
    }
    return true;
  }

  void fft_ab_cd_p(const vector<int> &a,const vector<int> &b,const vector<int> & c,const vector<int> &d,vector<int> & res,int p){
    int s=a.size();
    res.resize(s);
#if 1 //def __x86_64__
    double invp=find_invp(p);
    for (int i=0;i<s;++i){
      longlong l=(longlong(a[i])*b[i]+longlong(c[i])*d[i]);
      double q=l*invp;
      l -= longlong(q)*p;
      res[i]=l;
    }
#else
    for (int i=0;i<s;++i){
      res[i]=(longlong(a[i])*b[i]+longlong(c[i])*d[i])%p;
    }
#endif
  }

  void fft_ab_cd_p1(const vector<int> &a,const vector<int> &b,const vector<int> & c,const vector<int> &d,vector<int> & res){
    int s=a.size();
    res.resize(s);
    for (int i=0;i<s;++i){
      res[i]=(longlong(a[i])*b[i]+longlong(c[i])*d[i])%p1;
    }
  }

  void fft_ab_cd_p2(const vector<int> &a,const vector<int> &b,const vector<int> & c,const vector<int> &d,vector<int> & res){
    int s=a.size();
    res.resize(s);
    for (int i=0;i<s;++i){
      res[i]=(longlong(a[i])*b[i]+longlong(c[i])*d[i])%p2;
    }
  }

  void fft_ab_cd_p3(const vector<int> &a,const vector<int> &b,const vector<int> & c,const vector<int> &d,vector<int> & res){
    int s=a.size();
    res.resize(s);
    for (int i=0;i<s;++i){
      res[i]=(longlong(a[i])*b[i]+longlong(c[i])*d[i])%p3;
    }
  }

  void fft_ab_cd(const fft_rep & a,const fft_rep & b,const fft_rep & c,const fft_rep & d,fft_rep & res){
    res.modulo=a.modulo;
    fft_ab_cd_p1(a.modp1,b.modp1,c.modp1,d.modp1,res.modp1);
    fft_ab_cd_p2(a.modp2,b.modp2,c.modp2,d.modp2,res.modp2);
    fft_ab_cd_p3(a.modp3,b.modp3,c.modp3,d.modp3,res.modp3);
  }

  void multi_fft_ab_cd(const multi_fft_rep & a,const multi_fft_rep & b,const multi_fft_rep & c,const multi_fft_rep & d,multi_fft_rep & res){
    res.modulo=a.modulo;
    fft_ab_cd_p1(a.p1p2p3.modp1,b.p1p2p3.modp1,c.p1p2p3.modp1,d.p1p2p3.modp1,res.p1p2p3.modp1);
    fft_ab_cd_p2(a.p1p2p3.modp2,b.p1p2p3.modp2,c.p1p2p3.modp2,d.p1p2p3.modp2,res.p1p2p3.modp2);
    fft_ab_cd_p3(a.p1p2p3.modp3,b.p1p2p3.modp3,c.p1p2p3.modp3,d.p1p2p3.modp3,res.p1p2p3.modp3);
    res.v.resize(a.v.size());
    for (size_t i=0;i<a.v.size();++i){
      res.v[i].modulo=a.v[i].modulo;
      fft_ab_cd_p1(a.v[i].modp1,b.v[i].modp1,c.v[i].modp1,d.v[i].modp1,res.v[i].modp1);
      fft_ab_cd_p2(a.v[i].modp2,b.v[i].modp2,c.v[i].modp2,d.v[i].modp2,res.v[i].modp2);
      fft_ab_cd_p3(a.v[i].modp3,b.v[i].modp3,c.v[i].modp3,d.v[i].modp3,res.v[i].modp3);
    }
  }

  void fft_ab_cd(const modpoly & a,const modpoly &b,const modpoly &c,const modpoly &d,unsigned long expoN,modpoly & res,mpz_t & tmp,mpz_t &tmpqz){
    int n=a.size();
    for (int i=0;i<n;++i){
      mpz_mul(tmp,*a[i]._ZINTptr,*b[i]._ZINTptr);
      mpz_addmul(tmp,*c[i]._ZINTptr,*d[i]._ZINTptr);
      smod2N(tmp,expoN,tmpqz);
      mpz_set(*res[i]._ZINTptr,tmp);
    }
  }

  // computes a*b+c*d
  // set N>=0 to an upper bound of the degree if you know one
  void ab_cd(int N,const modpoly &a,const modpoly &b,const modpoly &c,const modpoly & d,environment * env,modpoly & res,modpoly & tmp1,modpoly & tmp2){
    modpoly resdbg;
    if (N>=0){
      if (a.size()>=FFTMUL_SIZE/4 && b.size()>=FFTMUL_SIZE/4 && c.size()>=FFTMUL_SIZE/4 && d.size()>=FFTMUL_SIZE/4 && env->moduloon){
	// N is the degree after reduction mod env->modulo
	// but not the degree of a*b+c*d
	// therefore we make computation mod x^n-1
	int Nreal=giacmax(a.size()+b.size(),c.size()+d.size())-2;
	gen pPQ(Nreal*(2*env->modulo*env->modulo)+1);
	unsigned long l=gen(giacmin(N,Nreal)).bindigits()-1; // m=2^l <= Nreal < 2^{l+1}
	unsigned long n=1<<(l+1);
	if (env->modulo.type==_INT_){
	  int p=env->modulo.val;
	  vector<int> aa; reverse_assign(a,aa,n,p);
	  vector<int> bb; reverse_assign(b,bb,n,p);
	  vector<int> cc; reverse_assign(c,cc,n,p);
	  vector<int> dd; reverse_assign(d,dd,n,p);
	  vector<int> Wp1,Wp2,Wp3;
	  fft_rep aaf;
	  to_fft(aa,p,Wp1,Wp2,Wp3,n,aaf,false,true);
	  fft_rep bbf;
	  to_fft(bb,p,Wp1,Wp2,Wp3,n,bbf,false,true);
	  fft_rep ccf;
	  to_fft(cc,p,Wp1,Wp2,Wp3,n,ccf,false,true);
	  fft_rep ddf;
	  to_fft(dd,p,Wp1,Wp2,Wp3,n,ddf,false,true);
	  // a*b + c*d FFT size
	  fft_rep resf;
	  fft_ab_cd(aaf,bbf,ccf,ddf,resf);
	  from_fft(resf,Wp1,Wp2,Wp3,dd,aa,bb,cc,true,true);
	  vector_int2vecteur(dd,res);
	  if (res.size()>N+1)
	    res=modpoly(res.end()-N-1,res.end());
	  trim_inplace(res,env);
	  return;
	}
	unsigned long bound=pPQ.bindigits()+1; // 2^bound=smod bound on coeff of p*q
	unsigned long r=(bound >> l)+1;
	if (0){ // not checked
	  vector<int> Wp1,Wp2,Wp3;
	  multi_fft_rep aaf; to_multi_fft(a,env->modulo,Wp1,Wp2,Wp3,n,aaf,true,true);
	  multi_fft_rep bbf; to_multi_fft(b,env->modulo,Wp1,Wp2,Wp3,n,bbf,true,true);
	  multi_fft_rep ccf; to_multi_fft(c,env->modulo,Wp1,Wp2,Wp3,n,ccf,true,true);
	  multi_fft_rep ddf; to_multi_fft(d,env->modulo,Wp1,Wp2,Wp3,n,ddf,true,true);
	  multi_fft_rep resf;
	  multi_fft_ab_cd(aaf,bbf,ccf,ddf,resf);
	  from_multi_fft(resf,Wp1,Wp2,Wp3,res,true);
	  trim_inplace(res,env);
	  return;
	}
	if (l>=2 && bound>=(1<<(l-2)) ){
	  mpz_t tmp,tmpqz; mpz_init(tmp); mpz_init(tmpqz);
	  gen tmp1,tmp2; tmp1.uncoerce(); tmp2.uncoerce();
	  unsigned long expoN=r << l; // r*2^l
	  modpoly aa; reverse_assign(a,aa,n,expoN+2);
	  modpoly work; reverse_resize(work,n,expoN+2);
	  fft2rl(&aa.front(),n,r,l,&work.front(),true,tmp1,tmp2,tmpqz);
	  modpoly bb; reverse_assign(b,bb,n,expoN+2);
	  fft2rl(&bb.front(),n,r,l,&work.front(),true,tmp1,tmp2,tmpqz);
	  modpoly cc; reverse_assign(c,cc,n,expoN+2);
	  fft2rl(&cc.front(),n,r,l,&work.front(),true,tmp1,tmp2,tmpqz);
	  modpoly dd; reverse_assign(d,dd,n,expoN+2);
	  fft2rl(&dd.front(),n,r,l,&work.front(),true,tmp1,tmp2,tmpqz);
	  // a*b+c*d FFT size
	  reverse_resize(res,n,expoN+2);
	  fft_ab_cd(aa,bb,cc,dd,expoN,res,tmp,tmpqz);
	  fft2rl(&res.front(),n,r,l,&work.front(),false,tmp1,tmp2,tmpqz);
	  // divide by n mod 2^expoN+1
	  fft2rldiv(res,expoN,expoN-l-1,tmp,tmpqz);
	  if (res.size()>N+1)
	    res=modpoly(res.end()-N-1,res.end());
	  trim_inplace(res,env);
	  mpz_clear(tmpqz); mpz_clear(tmp);
	  return;
	  resdbg=res;
	}
      }
      if (1 && a.size()>N+1){
	ab_cd(N,modpoly(a.end()-N-1,a.end()),b,c,d,env,res,tmp1,tmp2);
	return;
      }
      if (1 && b.size()>N+1){
	ab_cd(N,a,modpoly(b.end()-N-1,b.end()),c,d,env,res,tmp1,tmp2);
	return;
      }
      if (1 && c.size()>N+1){
	ab_cd(N,a,b,modpoly(c.end()-N-1,c.end()),d,env,res,tmp1,tmp2);
	return;
      }
      if (1 && d.size()>N+1){
	ab_cd(N,a,b,c,modpoly(d.end()-N-1,d.end()),env,res,tmp1,tmp2);
	return;
      }
    } // end if (N>=0)
    // res=trim(a*b+c*d,env); return;
    if (1
	// && env && env->moduloon && env->modulo.type==_INT_ && longlong(env->modulo.val)*env->modulo.val<(1LL<<31)
	){
      // smod at end, faster for small modulo (modulo^2<2^31)
      environment zeroenv;
      tmp1.clear();
      if (!a.empty() && !b.empty())
	operator_times(a,b,&zeroenv,tmp1,N>=0?N:RAND_MAX);
      if (N>=0 && tmp1.size()>N+1)
	tmp1=modpoly(tmp1.end()-N-1,tmp1.end());
#if 0 // debug
      tmp2.clear();
      if (!a.empty() && !b.empty())
	operator_times(a,b,&zeroenv,tmp2,RAND_MAX);
      if (N>=0 && tmp2.size()>N+1)
	tmp2=modpoly(tmp2.end()-N-1,tmp2.end());
      if (tmp1!=tmp2)
	COUT << "error" << tmp1-tmp2 << '\n';
#endif
      tmp2.clear();
      if (!c.empty() && !d.empty())
	operator_times(c,d,&zeroenv,tmp2,N>=0?N:RAND_MAX);
      if (N>=0 && tmp2.size()>N+1)
	tmp2=modpoly(tmp2.end()-N-1,tmp2.end());
#if 0
      addmodpoly(tmp1,tmp2,res);
#else
      if (tmp1.size()>=tmp2.size()){
	if (!tmp2.empty())
	  addmodpoly(tmp1,tmp2,tmp1);
	res.swap(tmp1);
      }
      else {
	if (!tmp1.empty())
	  addmodpoly(tmp2,tmp1,tmp2);
	res.swap(tmp2);
      }
#endif
      trim_inplace(res,env);
      if (!resdbg.empty() && res!=resdbg) 
	COUT << res-resdbg << '\n';
    }
    else {
      tmp1.clear();
      if (!a.empty() && !b.empty())
	operator_times(a,b,env,tmp1);
      tmp2.clear();
      if (!c.empty() && !d.empty())
	operator_times(c,d,env,tmp2);
      addmodpoly(tmp1,tmp2,env,res);
      trim_inplace(res,env);
    }
  }

#ifdef GIAC_PRECOND
  inline int precond_mulmodp(unsigned A,unsigned W,unsigned Winvp,int p){
#if 1
    longlong t = ulonglong(A)*W-((ulonglong(A)*Winvp)>>32)*p;
    t += ((t>>31)&p);
    return t;
    unsigned s=(ulonglong(A)*W)%p;
    if (t!=s)
      CERR << '\n';
    return s;
#else
    longlong t = ulonglong(A)*W-((ulonglong(A)*Winvp)>>32)*p;
    //return t- (t>>63)*p;
    int tt= t- (t>>63)*p;
    unsigned s=(ulonglong(A)*W)%p;
    if (tt!=s)
      CERR << '\n';
    return s;
#endif
  }
#else
  inline int precond_mulmodp(unsigned A,unsigned W,unsigned Winvp,int p){
    return (ulonglong(A)*W)%p;
  }
#endif

  inline int mulmodp(int a,int b,int p){
    return (longlong(a)*b) % p;    
  }

  inline int mulmodp(int a,int b,int p,double invp){
    int t=amodp(longlong(a)*b, p,invp);    
    //t=(longlong(a)*b) % p;
    //t += (t>>31)&p;
    return t;
  }

  inline int pos_mulmodp(int a,int b,int p,double invp){
    int t=apos_modp(longlong(a)*b, p,invp);    
    return t;
  }

  // reverse *a..*b and neg
  void fft_rev1(int * a,int *b,longlong p){
    for (;b>a;++a,--b){
      int tmp=*a;
      *a=p-*b;
      *b=p-tmp;
    }
    if (a==b)
      *a=p-*a;
  }

#ifdef GIAC_PRECOND // preconditionned
  void fft_reverse(vector<int> & W,int p){
    if (W.size()<2)
      return;
    int * a=&W.front();
    int N=W.size()/2;
    fft_rev1(a+1,a+N-1,p);
    fft_rev1(a+N+1,a+2*N-1,1);
  }

  void fft2wp(vector<int> & W,int n,int w,int p){
    W.resize(n); 
    w %= p;
    if (w<0) w += p;
    double invp=double(1ULL<<32)/p;
    int N=n/2;
    unsigned ww=1;
    for (int i=0;i<N;++i){
      W[i]=ww;
#if 0
      unsigned u=ww*invp; u++; 
      // might error by 1 if 2^32*w-n*p==+/-1, because relative prec is 2^-53
      // fix with long_double, relative precision of 2^-64 
#else
      unsigned u=1+((1ULL<<32)*ww)/unsigned(p); // quotient ceiling
#endif
      W[N+i]=u; // stored as an int but it's an unsigned 
      ww=precond_mulmodp(w,ww,u,p);
      // ww=(ww*longlong(w))%p;
      // if (www!=ww)
      //CERR << '\n';
    }
  }
  void fft2wp1(vector<int> & W,int n,int w){
    W.resize(n); 
    const int p = p1 ;
    w=w % p;
    if (w<0) w += p;
    int N=n/2;
    unsigned ww=1;
    for (int i=0;i<N;++i){
      W[i]=ww;
      W[N+i]=1+((1ULL<<32)*ww)/unsigned(p); // quotient ceiling
      ww=(ww*longlong(w))%p;
    }
  }
  void fft2wp2(vector<int> & W,int n,int w){
    W.resize(n); 
    const int p = p2 ;
    w=w % p;
    if (w<0) w += p;
    int N=n/2;
    unsigned ww=1;
    for (int i=0;i<N;++i){
      W[i]=ww;
      W[N+i]=1+((1LL<<32)*ww)/unsigned(p); // quotient ceiling
      ww=(ww*longlong(w))%p;
    }
  }
  void fft2wp3(vector<int> & W,int n,int w){
    W.resize(n); 
    const int p = p3 ;
    w=w % p;
    if (w<0) w += p;
    int N=n/2;
    unsigned ww=1;
    for (int i=0;i<N;++i){
      W[i]=ww;
      W[N+i]=1+((1ULL<<32)*ww)/unsigned(p); // quotient ceiling
      ww=(ww*longlong(w))%p;
    }
  }
#else
  void fft_reverse(vector<int> & W,int p){
    if (W.size()<2)
      return;
    int * a=&W.front();
#ifdef GIAC_CACHEW
    for (int N=(W.size()+1)/2;N>=2;a+=N,N/=2){
      fft_rev1(a+1,a+N-1,p);
    }
#else
    fft_rev1(a+1,a+W.size()-1,p);
#endif
  }

  void fft2wp_add(vector<int> & W,int N){
    int step=1;
    for (N/=2;N;N/=2){
      step *= 2;
      for (int i=0;i<N;++i){
	W.push_back(W[i*step]);
      }
    }
    W.push_back(1);
  }

  void fft2wp(vector<int> & W,int n,int w,int p){
#ifdef GIAC_CACHEW
    W.reserve(n); 
#else
    W.reserve(n/2); 
#endif
    double invp=find_invp(p);
    w=amodp(w,p,invp);
    if (w<0) w += p;
    int N=n/2,ww=1;
    for (int i=0;i<N;++i){
      W.push_back(ww);
      ww=mulmodp(ww,w,p,invp);//(ww*longlong(w))%p;
    }
#ifdef GIAC_CACHEW
    fft2wp_add(W,N);
#endif
  }
  void fft2wp1(vector<int> & W,int n,int w){
#ifdef GIAC_CACHEW
    W.reserve(n); 
#else
    W.reserve(n/2); 
#endif
    const int p = p1 ;
    w=w % p;
    if (w<0) w += p;
    int N=n/2,ww=1;
    for (int i=0;i<N;++i){
      W.push_back(ww);
      ww=(ww*longlong(w))%p;
    }
#ifdef GIAC_CACHEW
    fft2wp_add(W,N);
#endif
  }
  void fft2wp2(vector<int> & W,int n,int w){
#ifdef GIAC_CACHEW
    W.reserve(n); 
#else
    W.reserve(n/2); 
#endif
    const int p = p2 ;
    w=w % p;
    if (w<0) w += p;
    int N=n/2,ww=1;
    for (int i=0;i<N;++i){
      W.push_back(ww);
      ww=(ww*longlong(w))%p;
    }
#ifdef GIAC_CACHEW
    fft2wp_add(W,N);
#endif
  }
  void fft2wp3(vector<int> & W,int n,int w){
#ifdef GIAC_CACHEW
    W.reserve(n); 
#else
    W.reserve(n/2); 
#endif
    const int p = p3 ;
    w=w % p;
    if (w<0) w += p;
    int N=n/2,ww=1;
    for (int i=0;i<N;++i){
      W.push_back(ww);
      ww=(ww*longlong(w))%p;
    }
#ifdef GIAC_CACHEW
    fft2wp_add(W,N);
#endif
  }
#endif

  void fft_rev(vector<int> & W,int p){
    if (p==p1 || p==p2 || p==p3){
      fft_reverse(W,p);
      return;
    }
    if (W.size()<2)
      return;
    int * a=&W.front();
    for (int N=(W.size()+1)/2;N;a+=N,N/=2){
      fft_rev1(a+1,a+N-1,p);
    }
  }

  //#define DEBUG 1
  // [[RA,RB],[RC,RD]]*[a0,a1]->[a,b]
  void matrix22inttimesvect(const vector<int> & RA,const vector<int> & RB,const vector<int> & RC,const vector<int> & RD,const vector<int> & a0,const vector<int> &a1,int maxadeg,int maxbdeg,vector<int> & a,vector<int> &b,int p,vector<int> & ra,vector<int> & rb,vector<int> & rc,vector<int> & rd,vector<int> &Wp){
    int dega0=a0.size()-1,m=(dega0+1)/2;
    int maxabdeg=giacmax(maxadeg,maxbdeg);
    int bbsize=giacmin(maxabdeg+1,a0.size());
    int ddsize=giacmin(maxabdeg+1,a1.size());
    int Nreal=giacmax(bbsize+RC.size(),ddsize+RD.size())-2;
    int N2=giacmin(maxabdeg,Nreal); // add 1 if fft is done without reverse
    unsigned long l=sizeinbase2(N2)-1; 
    // l=gen(N2).bindigits()-1; // m=2^l <= Nreal < 2^{l+1}
    unsigned long n=1<<(l+1);
    if (debug_infolevel>2)
      CERR << CLOCK()*1e-6 << " mat22vectint begin n=" << n << " N2=" << N2 << " ra=" << ra.size() << '\n';
    int w=find_w(Wp,l+1,p);
#ifdef GIAC_CACHEW
    //Wp.clear();
#endif
    // vector<int> adbg,bdbg;
    if (w){
      if (N2<n){ 
	// if N2==n, this branch would require moving the last coeff
	// of a and b at the front of a and b, because it would not be 0
	// but it's easier to run FFT in normal order
	to_fft(RA,p,w,Wp,n,ra,2,false,false);
	to_fft(RB,p,w,Wp,n,rb,2,false,false);
	to_fft(RC,p,w,Wp,n,rc,2,false,false);
	to_fft(RD,p,w,Wp,n,rd,2,false,false);
	to_fft(a0,p,w,Wp,n,a,2,false,false);
	to_fft(a1,p,w,Wp,n,b,2,false,false);
	fft_reverse(Wp,p); 
	fft_ab_cd_p(rc,a,rd,b,rc,p);
	from_fft(rc,p,Wp,rc,false,false);
	fft_ab_cd_p(ra,a,rb,b,a,p);
	from_fft(a,p,Wp,a,false,false);
	b.swap(rc);
	//fft_reverse(Wp,p); 
	a.pop_back(); 
	b.pop_back();
	fast_trim_inplace(a,p,maxabdeg+1);
	fast_trim_inplace(b,p,maxabdeg+1);
      }
      else {
	// reverse_assign should have made ra,rb, etc. positive
	to_fft(RA,p,w,Wp,n,ra,1,false,false);
	to_fft(RB,p,w,Wp,n,rb,1,false,false);
	to_fft(RC,p,w,Wp,n,rc,1,false,false);
	to_fft(RD,p,w,Wp,n,rd,1,false,false);
	to_fft(a0,p,w,Wp,n,a,1,false,false);
	to_fft(a1,p,w,Wp,n,b,1,false,false);
	fft_reverse(Wp,p); 
	fft_ab_cd_p(rc,a,rd,b,rc,p);
	from_fft(rc,p,Wp,rc,true,false);
	fft_ab_cd_p(ra,a,rb,b,a,p);
	from_fft(a,p,Wp,a,true,false);
	b.swap(rc);
	//fft_reverse(Wp,p); 
	fast_trim_inplace(a,p,maxabdeg+1);
	fast_trim_inplace(b,p,maxabdeg+1);
      }
    } else {
      vector<int> a0_,a1_; 
      reverse_assign(RA,ra,n,p);
      reverse_assign(RB,rb,n,p);
      reverse_assign(RC,rc,n,p);
      reverse_assign(RD,rd,n,p);
      reverse_assign(a0,a0_,n,p);
      reverse_assign(a1,a1_,n,p);
      vector<int> Wp1,Wp2,Wp3;
      fft_rep raf; 
      to_fft(ra,p,Wp1,Wp2,Wp3,n,raf,false,true);
      fft_rep rbf; to_fft(rb,p,Wp1,Wp2,Wp3,n,rbf,false,true);
      fft_rep rcf; to_fft(rc,p,Wp1,Wp2,Wp3,n,rcf,false,true);
      fft_rep rdf; to_fft(rd,p,Wp1,Wp2,Wp3,n,rdf,false,true);
      fft_rep a0f; 
      to_fft(a0_,p,Wp1,Wp2,Wp3,n,a0f,false,true);
      fft_rep a1f; 
      to_fft(a1_,p,Wp1,Wp2,Wp3,n,a1f,false,true);
      fft_rep resf;
      fft_ab_cd(raf,a0f,rbf,a1f,resf);
      fft_reverse(Wp1,p1); fft_reverse(Wp2,p2); fft_reverse(Wp3,p3);
      from_fft(resf,Wp1,Wp2,Wp3,a,ra,rb,rc,true,false);
      fft_ab_cd(rcf,a0f,rdf,a1f,resf);
      from_fft(resf,Wp1,Wp2,Wp3,b,ra,rb,rc,true,false);
      //fft_reverse(Wp1,p1); fft_reverse(Wp2,p2); fft_reverse(Wp3,p3);
      fast_trim_inplace(a,p,maxabdeg+1);
      //trim_deg(b,maxabdeg);
      fast_trim_inplace(b,p,maxabdeg+1);
    }
    // if (w && a!=adbg && b!=bdbg) CERR << "err\n";
    //trim_deg(a,maxabdeg);
    if (debug_infolevel>2)
      CERR << CLOCK()*1e-6 << " mat22vectint end " << n << '\n';
  }

  // [[RA,RB],[RC,RD]]*[a0,a1]->[a,b]
  void matrix22timesvect(const modpoly & RA,const modpoly & RB,const modpoly & RC,const modpoly & RD,const modpoly & a0,const modpoly &a1,int maxadeg,int maxbdeg,modpoly & a,modpoly &b,environment & env,modpoly & tmp1,modpoly & tmp2){
    bool doit=true;
    int dega0=a0.size()-1,m=(dega0+1)/2;
    int maxabdeg=giacmax(maxadeg,maxbdeg);
    if (1&& env.moduloon && a0.size()>=FFTMUL_SIZE/4 && a1.size()>=FFTMUL_SIZE/4 && RA.size()>=FFTMUL_SIZE/4 && RB.size()>=FFTMUL_SIZE/4){
      int bbsize=giacmin(maxabdeg+1,a0.size());
      int ddsize=giacmin(maxabdeg+1,a1.size());
      int Nreal=giacmax(bbsize+RC.size(),ddsize+RD.size())-2;
      int N2=giacmin(maxabdeg,Nreal);
      gen pPQ(Nreal*(2*env.modulo*env.modulo)+1);
      unsigned long l=gen(N2).bindigits()-1; // m=2^l <= Nreal < 2^{l+1}
      unsigned long n=1<<(l+1);
      unsigned long bound=pPQ.bindigits()+1; // 2^bound=smod bound on coeff of p*q
      unsigned long r=(bound >> l)+1;
      if (env.modulo.type==_INT_){
	doit=false; int p=env.modulo.val;
	vector<int> ra; reverse_assign(RA,ra,n,p);
	vector<int> rb; reverse_assign(RB,rb,n,p);
	vector<int> rc; reverse_assign(RC,rc,n,p);
	vector<int> rd; reverse_assign(RD,rd,n,p);
	vector<int> a0_; 
	reverse_assign(a0,a0_,n,p);
	vector<int> a1_; 
	reverse_assign(a1,a1_,n,p);
	vector<int> Wp1,Wp2,Wp3;
	fft_rep raf; 
	to_fft(ra,p,Wp1,Wp2,Wp3,n,raf,false,true);
	fft_rep rbf; to_fft(rb,p,Wp1,Wp2,Wp3,n,rbf,false,true);
	fft_rep rcf; to_fft(rc,p,Wp1,Wp2,Wp3,n,rcf,false,true);
	fft_rep rdf; to_fft(rd,p,Wp1,Wp2,Wp3,n,rdf,false,true);
	fft_rep a0f; 
	to_fft(a0_,p,Wp1,Wp2,Wp3,n,a0f,false,true);
	fft_rep a1f; 
	to_fft(a1_,p,Wp1,Wp2,Wp3,n,a1f,false,true);
	fft_rep resf;
	fft_ab_cd(raf,a0f,rbf,a1f,resf);
	fft_reverse(Wp1,p1); fft_reverse(Wp2,p2); fft_reverse(Wp3,p3);
	from_fft(resf,Wp1,Wp2,Wp3,a0_,ra,rb,rc,true,false);
	fft_ab_cd(rcf,a0f,rdf,a1f,resf);
	from_fft(resf,Wp1,Wp2,Wp3,a1_,ra,rb,rc,true,false);
	fft_reverse(Wp1,p1); fft_reverse(Wp2,p2); fft_reverse(Wp3,p3);
	vector_int2vecteur(a0_,a);
	trim_deg(a,maxabdeg);
	trim_inplace(a,&env);
	vector_int2vecteur(a1_,b);
	trim_deg(b,maxabdeg);
	trim_inplace(b,&env);
	//CERR << n << " " << b << '\n';
      }
      if (doit && l>=2 && bound>=(1<<(l-2)) ){
	doit=false;
	mpz_t tmp,tmpqz; mpz_init(tmp); mpz_init(tmpqz);
	gen tmp1g,tmp2g; tmp1g.uncoerce(); tmp2g.uncoerce();
	unsigned long expoN=r << l; // r*2^l
	modpoly aa; reverse_assign(RA,aa,n,expoN+2); 
	modpoly work; reverse_resize(work,n,expoN+2);
	// RA*a0+RB*a1 FFT size
	fft2rl(&aa.front(),n,r,l,&work.front(),true,tmp1g,tmp2g,tmpqz);
	modpoly & bb=tmp1; 
	reverse_assign(a0,bb,n,expoN+2);// reverse_resize(bb,n,expoN+2);
	fft2rl(&bb.front(),n,r,l,&work.front(),true,tmp1g,tmp2g,tmpqz);
	modpoly cc; reverse_assign(RB,cc,n,expoN+2);
	fft2rl(&cc.front(),n,r,l,&work.front(),true,tmp1g,tmp2g,tmpqz);
	modpoly & dd=tmp2;
	reverse_assign(a1,dd,n,expoN+2); // reverse_resize(dd,n,expoN+2);
	fft2rl(&dd.front(),n,r,l,&work.front(),true,tmp1g,tmp2g,tmpqz);
	reverse_resize(a,n,expoN+2);
	fft_ab_cd(aa,bb,cc,dd,expoN,a,tmp,tmpqz);
	fft2rl(&a.front(),n,r,l,&work.front(),false,tmp1g,tmp2g,tmpqz);
	// divide by n mod 2^expoN+1
	fft2rldiv(a,expoN,expoN-l-1,tmp,tmpqz);
	trim_deg(a,maxabdeg);
	trim_inplace(a,&env);
	reverse_assign(RC,aa,n,expoN+2);
	fft2rl(&aa.front(),n,r,l,&work.front(),true,tmp1g,tmp2g,tmpqz);
	reverse_assign(RD,cc,n,expoN+2);
	fft2rl(&cc.front(),n,r,l,&work.front(),true,tmp1g,tmp2g,tmpqz);
	// RC*a0+RD*a1 FFT size
	reverse_resize(b,n,expoN+2);
	fft_ab_cd(aa,bb,cc,dd,expoN,b,tmp,tmpqz);
	fft2rl(&b.front(),n,r,l,&work.front(),false,tmp1g,tmp2g,tmpqz);
	// divide by n mod 2^expoN+1
	fft2rldiv(b,expoN,expoN-l-1,tmp,tmpqz);
	trim_deg(b,maxabdeg);
	trim_inplace(b,&env);
	mpz_clear(tmpqz); mpz_clear(tmp);
      }
      if (doit){
	doit=false; 
	vector<int> Wp1,Wp2,Wp3;
	multi_fft_rep raf; 
	to_multi_fft(RA,env.modulo,Wp1,Wp2,Wp3,n,raf,true,true);
	//from_multi_fft(raf,Wp1,Wp2,Wp3,a,true); trim_inplace(a,&env);
	multi_fft_rep rbf; to_multi_fft(RB,env.modulo,Wp1,Wp2,Wp3,n,rbf,true,true);
	multi_fft_rep rcf; to_multi_fft(RC,env.modulo,Wp1,Wp2,Wp3,n,rcf,true,true);
	multi_fft_rep rdf; to_multi_fft(RD,env.modulo,Wp1,Wp2,Wp3,n,rdf,true,true);
	multi_fft_rep a0f; to_multi_fft(a0,env.modulo,Wp1,Wp2,Wp3,n,a0f,true,true);
	multi_fft_rep a1f; to_multi_fft(a1,env.modulo,Wp1,Wp2,Wp3,n,a1f,true,true);
	multi_fft_rep resf;
	multi_fft_ab_cd(raf,a0f,rbf,a1f,resf);
	from_multi_fft(resf,Wp1,Wp2,Wp3,a,true);
	trim_deg(a,maxabdeg);
	trim_inplace(a,&env);
	multi_fft_ab_cd(rcf,a0f,rdf,a1f,resf);
	from_multi_fft(resf,Wp1,Wp2,Wp3,b,true);
	trim_deg(b,maxabdeg);
	trim_inplace(b,&env);
	//CERR << n << " " << b << '\n';
      }
    }
    if (doit){
      ab_cd(maxbdeg,RC,a0,RD,a1,&env,b,tmp1,tmp2); // b=trim(RC*a0+RD*a1,&env); // C=B' in Yap
      // ab_cd(dega0,RA,a0,RB,a1,&env,a,tmp1,tmp2); // a=trim(RA*a0+RB*a1,&env); // A' in Yap
      ab_cd(maxadeg,RA,a0,RB,a1,&env,a,tmp1,tmp2); // a=trim(RA*a0+RB*a1,&env); // A' in Yap
    }
  }

  void matrix22int(vector<int> & RA,vector<int> &RB,vector<int> & RC,vector<int> &RD,vector<int> &SA,vector<int> &SB,vector<int> &SC,vector<int> &SD,vector<int> &A,vector<int> &B,vector<int> &C,vector<int> &D,int p,vector<int> & tmp0,vector<int> & Wp){
    // 2x2 matrix operations
    // [[SA,SB],[SC,SD]]*[[RC,RD],[RA,RB]] == [[RA*SB+RC*SA,RB*SB+RD*SA],[RA*SD+RC*SC,RB*SD+RD*SC]]
    int Nreal=giacmax(giacmax(RC.size(),RD.size()),giacmax(RA.size(),RB.size()))+giacmax(giacmax(SC.size(),SD.size()),giacmax(SA.size(),SB.size()))-2;
    // increase Nreal by 1 if fft without reverse
    unsigned long l=sizeinbase2(Nreal)-1; // l=gen(Nreal).bindigits()-1; // m=2^l <= Nreal < 2^{l+1}
    unsigned long n=1<<(l+1);
    if (debug_infolevel>2)
      CERR << CLOCK()*1e-6 << " mat22int begin " << n << " " << Nreal << '\n';
    int w=find_w(Wp,l+1,p);
#ifdef GIAC_CACHEW
    //Wp.clear();
#endif
    // vector<int> adbg,bdbg;
    if (w){
#if 0 // if set to 1 increase Nreal by 1, if set to 0 decrease Nreal by 1
      to_fft(SC,p,w,Wp,n,SC,2,false,false); 
      to_fft(SD,p,w,Wp,n,SD,2,false,false); 
      to_fft(RA,p,w,Wp,n,RA,2,false,false); 
      to_fft(RB,p,w,Wp,n,RB,2,false,false); 
      to_fft(RC,p,w,Wp,n,RC,2,false,false); 
      to_fft(RD,p,w,Wp,n,RD,2,false,false); 
      to_fft(SA,p,w,Wp,n,SA,2,false,false); 
      to_fft(SB,p,w,Wp,n,SB,2,false,false); 
      fft_reverse(Wp,p); 
      fft_ab_cd_p(RA,SB,RC,SA,A,p);
      from_fft(A,p,Wp,A,false,false);
      A.pop_back();
      fft_ab_cd_p(RB,SB,RD,SA,SA,p); SA.swap(B);
      from_fft(B,p,Wp,B,false,false);
      B.pop_back();
      fft_ab_cd_p(RA,SD,RC,SC,RA,p); RA.swap(C);
      from_fft(C,p,Wp,C,false,false);
      C.pop_back();
      fft_ab_cd_p(RB,SD,RD,SC,RB,p); RB.swap(D);
      from_fft(D,p,Wp,D,false,false);
      D.pop_back();
#else
      // makepositive set to false since reverse_assign should make RA positive
      to_fft(SC,p,w,Wp,n,SC,1,false,false); 
      to_fft(SD,p,w,Wp,n,SD,1,false,false); 
      to_fft(RA,p,w,Wp,n,RA,1,false,false); 
      to_fft(RB,p,w,Wp,n,RB,1,false,false); 
      to_fft(RC,p,w,Wp,n,RC,1,false,false); 
      to_fft(RD,p,w,Wp,n,RD,1,false,false); 
      to_fft(SA,p,w,Wp,n,SA,1,false,false); 
      to_fft(SB,p,w,Wp,n,SB,1,false,false); 
      fft_reverse(Wp,p); 
      fft_ab_cd_p(RA,SB,RC,SA,A,p);
      from_fft(A,p,Wp,A,true,false);
      fft_ab_cd_p(RB,SB,RD,SA,SA,p); SA.swap(B);
      from_fft(B,p,Wp,B,true,false);
      fft_ab_cd_p(RA,SD,RC,SC,RA,p); RA.swap(C);
      from_fft(C,p,Wp,C,true,false);
      fft_ab_cd_p(RB,SD,RD,SC,RB,p); RB.swap(D);
      from_fft(D,p,Wp,D,true,false);
#endif
      fast_trim_inplace(A,p);
      fast_trim_inplace(B,p);
      fast_trim_inplace(C,p);
      fast_trim_inplace(D,p);
      //fft_reverse(Wp,p); 
    }
    else {
      reverse_assign(RA,tmp0,n,p); RA.swap(tmp0);
      reverse_assign(RB,tmp0,n,p); RB.swap(tmp0);
      reverse_assign(RC,tmp0,n,p); RC.swap(tmp0);
      reverse_assign(RD,tmp0,n,p); RD.swap(tmp0);
      reverse_assign(SA,tmp0,n,p); SA.swap(tmp0);
      reverse_assign(SB,tmp0,n,p); SB.swap(tmp0);
      reverse_assign(SC,tmp0,n,p); SC.swap(tmp0);
      reverse_assign(SD,tmp0,n,p); SD.swap(tmp0);
      vector<int> Wp1,Wp2,Wp3,tmp_,tmp__;
      fft_rep raf; to_fft(RA,p,Wp1,Wp2,Wp3,n,raf,false,true);
      fft_rep rbf; to_fft(RB,p,Wp1,Wp2,Wp3,n,rbf,false,true);
      fft_rep rcf; to_fft(RC,p,Wp1,Wp2,Wp3,n,rcf,false,true);
      fft_rep rdf; to_fft(RD,p,Wp1,Wp2,Wp3,n,rdf,false,true);
      fft_rep saf; to_fft(SA,p,Wp1,Wp2,Wp3,n,saf,false,true);
      fft_rep sbf; to_fft(SB,p,Wp1,Wp2,Wp3,n,sbf,false,true);
      fft_rep resf;
      fft_ab_cd(raf,sbf,rcf,saf,resf);
      fft_reverse(Wp1,p1); fft_reverse(Wp2,p2); fft_reverse(Wp3,p3);
      from_fft(resf,Wp1,Wp2,Wp3,A,tmp0,tmp_,tmp__,true,false);
      fft_ab_cd(rbf,sbf,rdf,saf,resf);
      from_fft(resf,Wp1,Wp2,Wp3,B,tmp0,tmp_,tmp__,true,false);
      fft_reverse(Wp1,p1); fft_reverse(Wp2,p2); fft_reverse(Wp3,p3);
      to_fft(SC,p,Wp1,Wp2,Wp3,n,saf,false,true);
      to_fft(SD,p,Wp1,Wp2,Wp3,n,sbf,false,true);
      fft_reverse(Wp1,p1); fft_reverse(Wp2,p2); fft_reverse(Wp3,p3);
      fft_ab_cd(raf,sbf,rcf,saf,resf);
      from_fft(resf,Wp1,Wp2,Wp3,C,tmp0,tmp_,tmp__,true,false);
      fft_ab_cd(rbf,sbf,rdf,saf,resf);
      from_fft(resf,Wp1,Wp2,Wp3,D,tmp0,tmp_,tmp__,true,false);
      //fft_reverse(Wp1,p1); fft_reverse(Wp2,p2); fft_reverse(Wp3,p3);
      fast_trim_inplace(A,p);
      fast_trim_inplace(B,p);
      fast_trim_inplace(C,p);
      fast_trim_inplace(D,p);
    }
    if (debug_infolevel>2)
      CERR << CLOCK()*1e-6 << " mat22int end " << n << '\n';
  }

  void matrix22(modpoly & RA,modpoly &RB,modpoly & RC,modpoly &RD,modpoly &SA,modpoly &SB,modpoly &SC,modpoly &SD,modpoly &A,modpoly &B,modpoly &C,modpoly &D,environment & env,modpoly & tmp1,modpoly & tmp2){
    // 2x2 matrix operations
    // [[SA,SB],[SC,SD]]*[[RC,RD],[RA,RB]] == [[RA*SB+RC*SA,RB*SB+RD*SA],[RA*SD+RC*SC,RB*SD+RD*SC]]
    bool doit=true;
    // modpoly Adbg,Bdbg,Cdbg,Ddbg;
    if (env.moduloon && RA.size()>=FFTMUL_SIZE/4 && SA.size()>=FFTMUL_SIZE/4 && RB.size()>=FFTMUL_SIZE/4 && SB.size()>=FFTMUL_SIZE/4){
      int Nreal=giacmax(giacmax(RC.size(),RD.size()),giacmax(RA.size(),RB.size()))+giacmax(giacmax(SC.size(),SD.size()),giacmax(SA.size(),SB.size()))-2;
      gen pPQ(Nreal*(2*env.modulo*env.modulo)+1);
      unsigned long l=gen(Nreal).bindigits()-1; // m=2^l <= Nreal < 2^{l+1}
      unsigned long bound=pPQ.bindigits()+1; // 2^bound=smod bound on coeff of p*q
      unsigned long r=(bound >> l)+1;
      unsigned long n=1<<(l+1);
      if (env.modulo.type==_INT_){
	doit=false; int p=env.modulo.val;
	vector<int> ra; reverse_assign(RA,ra,n,p);
	vector<int> rb; reverse_assign(RB,rb,n,p);
	vector<int> rc; reverse_assign(RC,rc,n,p);
	vector<int> rd; reverse_assign(RD,rd,n,p);
	vector<int> sa; reverse_assign(SA,sa,n,p);
	vector<int> sb; reverse_assign(SB,sb,n,p);
	vector<int> Wp1,Wp2,Wp3;
	fft_rep raf; to_fft(ra,p,Wp1,Wp2,Wp3,n,raf,false,true);
	fft_rep rbf; to_fft(rb,p,Wp1,Wp2,Wp3,n,rbf,false,true);
	fft_rep rcf; to_fft(rc,p,Wp1,Wp2,Wp3,n,rcf,false,true);
	fft_rep rdf; to_fft(rd,p,Wp1,Wp2,Wp3,n,rdf,false,true);
	fft_rep saf; to_fft(sa,p,Wp1,Wp2,Wp3,n,saf,false,true);
	fft_rep sbf; to_fft(sb,p,Wp1,Wp2,Wp3,n,sbf,false,true);
	vector<int> tmpres;
	fft_rep resf;
	fft_ab_cd(raf,sbf,rcf,saf,resf);
	fft_reverse(Wp1,p1); fft_reverse(Wp2,p2); fft_reverse(Wp3,p3);
	from_fft(resf,Wp1,Wp2,Wp3,tmpres,ra,rb,rc,true,false);
	vector_int2vecteur(tmpres,A); trim_inplace(A,&env);
	fft_ab_cd(rbf,sbf,rdf,saf,resf);
	from_fft(resf,Wp1,Wp2,Wp3,tmpres,ra,rb,rc,true,false);
	fft_reverse(Wp1,p1); fft_reverse(Wp2,p2); fft_reverse(Wp3,p3);
	vector_int2vecteur(tmpres,B); trim_inplace(B,&env);
	reverse_assign(SC,sa,n,p); to_fft(sa,p,Wp1,Wp2,Wp3,n,saf,false,true);
	reverse_assign(SD,sb,n,p); to_fft(sb,p,Wp1,Wp2,Wp3,n,sbf,false,true);
	fft_ab_cd(raf,sbf,rcf,saf,resf);
	fft_reverse(Wp1,p1); fft_reverse(Wp2,p2); fft_reverse(Wp3,p3);
	from_fft(resf,Wp1,Wp2,Wp3,tmpres,ra,rb,rc,true,false);
	vector_int2vecteur(tmpres,C); trim_inplace(C,&env);
	fft_ab_cd(rbf,sbf,rdf,saf,resf);
	from_fft(resf,Wp1,Wp2,Wp3,tmpres,ra,rb,rc,true,false);
	fft_reverse(Wp1,p1); fft_reverse(Wp2,p2); fft_reverse(Wp3,p3);
	vector_int2vecteur(tmpres,D); trim_inplace(D,&env);
      }
      if (doit && l>=2 && bound>=(1<<(l-2)) ){
	doit=false;
	mpz_t tmp,tmpqz; mpz_init(tmp); mpz_init(tmpqz);
	gen tmp1,tmp2; tmp1.uncoerce(); tmp2.uncoerce();
	unsigned long expoN=r << l; // r*2^l
	modpoly work; reverse_resize(work,n,expoN+2);
	modpoly &ra=RA; reverse_assign(RA,ra,n,expoN+2);
	fft2rl(&ra.front(),n,r,l,&work.front(),true,tmp1,tmp2,tmpqz);
	modpoly &rb=RB; reverse_assign(RB,rb,n,expoN+2);
	fft2rl(&rb.front(),n,r,l,&work.front(),true,tmp1,tmp2,tmpqz);
	modpoly &rc=RC; reverse_assign(RC,rc,n,expoN+2);
	fft2rl(&rc.front(),n,r,l,&work.front(),true,tmp1,tmp2,tmpqz);
	modpoly &rd=RD; reverse_assign(RD,rd,n,expoN+2);
	fft2rl(&rd.front(),n,r,l,&work.front(),true,tmp1,tmp2,tmpqz);
	modpoly &sa=SA; reverse_assign(SA,sa,n,expoN+2);
	fft2rl(&sa.front(),n,r,l,&work.front(),true,tmp1,tmp2,tmpqz);
	modpoly &sb=SB; reverse_assign(SB,sb,n,expoN+2);
	fft2rl(&sb.front(),n,r,l,&work.front(),true,tmp1,tmp2,tmpqz);
	// A=trim(RA*SB+RC*SA,&env);
	reverse_resize(A,n,expoN+2); 
	fft_ab_cd(ra,sb,rc,sa,expoN,A,tmp,tmpqz);
	fft2rl(&A.front(),n,r,l,&work.front(),false,tmp1,tmp2,tmpqz);
	fft2rldiv(A,expoN,expoN-l-1,tmp,tmpqz);
	trim_inplace(A,&env);
	// B=trim(RB*SB+RD*SA,&env);
	reverse_resize(B,n,expoN+2); 
	fft_ab_cd(rb,sb,rd,sa,expoN,B,tmp,tmpqz);
	fft2rl(&B.front(),n,r,l,&work.front(),false,tmp1,tmp2,tmpqz);
	fft2rldiv(B,expoN,expoN-l-1,tmp,tmpqz);
	trim_inplace(B,&env);
	// C=trim(RA*SD+RC*SC,&env);
	reverse_assign(SC,sa,n,expoN+2); 
	fft2rl(&sa.front(),n,r,l,&work.front(),true,tmp1,tmp2,tmpqz);
	reverse_assign(SD,sb,n,expoN+2); 
	fft2rl(&sb.front(),n,r,l,&work.front(),true,tmp1,tmp2,tmpqz);
	reverse_resize(C,n,expoN+2); 
	fft_ab_cd(ra,sb,rc,sa,expoN,C,tmp,tmpqz);
	fft2rl(&C.front(),n,r,l,&work.front(),false,tmp1,tmp2,tmpqz);
	fft2rldiv(C,expoN,expoN-l-1,tmp,tmpqz);
	trim_inplace(C,&env);
	// D=trim(RB*SD+RD*SC,&env);
	reverse_resize(D,n,expoN+2);
	fft_ab_cd(rb,sb,rd,sa,expoN,D,tmp,tmpqz);
	fft2rl(&D.front(),n,r,l,&work.front(),false,tmp1,tmp2,tmpqz);
	fft2rldiv(D,expoN,expoN-l-1,tmp,tmpqz);
	trim_inplace(D,&env);
	mpz_clear(tmpqz); mpz_clear(tmp);
	doit=false;
	//Adbg=A; Bdbg=B; Cdbg=C; Ddbg=D;
      }
      if (doit){
	vector<int> Wp1,Wp2,Wp3;
	multi_fft_rep raf; to_multi_fft(RA,env.modulo,Wp1,Wp2,Wp3,n,raf,true,true);
	//from_multi_fft(raf,Wp1,Wp2,Wp3,a,true); trim_inplace(a,&env);
	multi_fft_rep rbf; to_multi_fft(RB,env.modulo,Wp1,Wp2,Wp3,n,rbf,true,true);
	multi_fft_rep rcf; to_multi_fft(RC,env.modulo,Wp1,Wp2,Wp3,n,rcf,true,true);
	multi_fft_rep rdf; to_multi_fft(RD,env.modulo,Wp1,Wp2,Wp3,n,rdf,true,true);
	multi_fft_rep saf; to_multi_fft(SA,env.modulo,Wp1,Wp2,Wp3,n,saf,true,true);
	multi_fft_rep sbf; to_multi_fft(SB,env.modulo,Wp1,Wp2,Wp3,n,sbf,true,true);
	multi_fft_rep resf;
	multi_fft_ab_cd(raf,sbf,rcf,saf,resf);
	from_multi_fft(resf,Wp1,Wp2,Wp3,A,true);
	trim_inplace(A,&env);
	multi_fft_ab_cd(rbf,sbf,rdf,saf,resf);
	from_multi_fft(resf,Wp1,Wp2,Wp3,B,true);
	trim_inplace(B,&env);
	to_multi_fft(SC,env.modulo,Wp1,Wp2,Wp3,n,saf,true,true);
	to_multi_fft(SD,env.modulo,Wp1,Wp2,Wp3,n,sbf,true,true);
	multi_fft_ab_cd(raf,sbf,rcf,saf,resf);
	from_multi_fft(resf,Wp1,Wp2,Wp3,C,true);
	trim_inplace(C,&env);
	multi_fft_ab_cd(rbf,sbf,rdf,saf,resf);
	from_multi_fft(resf,Wp1,Wp2,Wp3,D,true);
	trim_inplace(D,&env);
	doit=false;
      }
    }
    if (doit){
      ab_cd(-1,RA,SB,RC,SA,&env,A,tmp1,tmp2); // A=trim(RA*SB+RC*SA,&env);
      ab_cd(-1,RB,SB,RD,SA,&env,B,tmp1,tmp2); // B=trim(RB*SB+RD*SA,&env);
      ab_cd(-1,RA,SD,RC,SC,&env,C,tmp1,tmp2); // C=trim(RA*SD+RC*SC,&env);
      ab_cd(-1,RB,SD,RD,SC,&env,D,tmp1,tmp2); // D=trim(RB*SD+RD*SC,&env);
      // if (!doit && (A!=Adbg || B!=Bdbg || C!=Cdbg || D!=Ddbg))	COUT << "error" << '\n'; 
    }
  }

#define HGCD_DIV 1

  // coeffv and degv are used by resultant (otherwise they are left empty)
  // coeffv is the list of leading coefficients of the remainder sequence
  // degv is the list of degrees of the remainder sequence
  // q,f, are temporary
  bool hgcdint(const vector<int> & a0,const vector<int> & a1,int modulo,vector<int> & Wp,vector<int> &A,vector<int> &B,vector<int> &C,vector<int> &D,vector<int> & coeffv,vector<int> & degv,vector<int> & q,vector<int> & f,vector<int> & tmp0,vector<int> & tmp1,vector<int> & tmp2,vector<int> & tmp3){ // a0 is A in Yap, a1 is B
    vector<int> & g0=tmp2,&g1=tmp3;
    int dega0=a0.size()-1,dega1=a1.size()-1;
    int m=(dega0+1)/2;
    if (dega1<m){
      D=A=vector<int>(1,1);
      B.clear(); C.clear();
      return true;
    }
    if (m<HGCD){ 
      hgcd_iter_int(a0,a1,m,A,C,B,D,modulo,coeffv,degv,q,f,tmp0,tmp1,tmp2,tmp3);
      return true;
    }
    //q.reserve(a0.size()); f.reserve(a0.size()); tmp0.reserve(a0.size()); tmp1.reserve(a0.size()); tmp2.reserve(a0.size()); tmp3.reserve(a0.size()); A.reserve(a0.size()); B.reserve(a0.size()); C.reserve(a0.size()); D.reserve(a0.size());
    vector<int> b0,b1; b0.reserve(nextpow2(a0.size()-m)); b1.reserve(nextpow2(a0.size()-m));
    b0.resize(a0.size()-m); copy(a0.begin(),a0.end()-m,b0.begin()); // quo(a0,x^m), A0 in Yap
    b1.resize(a1.size()-m); copy(a1.begin(),a1.end()-m,b1.begin()); // quo(a1,x^m), B0 in Yap
    // 1st recursive call
    vector<int> RA,RB,RC,RD;
    if (debug_infolevel>2)
      CERR << CLOCK()*1e-6 << " hgcdint 1st recursive call " << dega0 << "," << dega1 << '\n';
    if (!hgcdint(b0,b1,modulo,Wp,RA,RB,RC,RD,coeffv,degv,tmp0,tmp1,A,B,C,D))
      return false;
    if (debug_infolevel>2)
      CERR << CLOCK()*1e-6 << " hgcdint compute A' B' " << dega0 << "," << dega1 << '\n';
    int maxadeg=dega0+1-giacmax(RA.size(),RB.size()),maxbdeg=dega0-m/2;
    //if (modulo==2112626689 && RA.size()==62 && RA[0]==390746818) CERR << "debug";
    matrix22inttimesvect(RA,RB,RC,RD,a0,a1,maxadeg,maxbdeg,b0,b1,modulo,tmp0,tmp1,tmp2,tmp3,Wp);
    int dege=b1.size()-1;
    if (dege<m){
      A.swap(RA); B.swap(RB); C.swap(RC); D.swap(RD);  
      return true;
      // A=RA; B=RB; C=RC; D=RD; return true;
    }
    if (dege>=b0.size()-1){
      COUT << "hgcdint error modulo " << modulo << '\n';
      return false;
    }
    if (debug_infolevel>2)
      CERR << CLOCK()*1e-6 << " hgcdint euclid div " << dega0 << "," << dega1 << '\n';
    // 1 euclidean step
    if (!degv.empty()){
      coeffv.push_back(b1[0]);
      degv.push_back(degv.back()+b1.size()-b0.size());
    }
    DivRem(b0,b1,modulo,q,f); // q,f are Q,D in Yap 
    // [[0,1],[1,-q]]*[[RA,RB],[RC,RD]] == [[RC,RD],[-RC*q+RA,-RD*q+RB]]
    a_bc(RA,RC,q,modulo,RA,b0); // RA=trim(RA-RC*q,&env);
    a_bc(RB,RD,q,modulo,RB,b0); // RB=trim(RB-RD*q,&env);
    int l=b1.size()-1,k=2*m-l;
    if (f.size()-1<m){
      A.swap(RC); B.swap(RD); C.swap(RA); D.swap(RB); 
      return true;
    }
    g0.resize(b1.size()-k);
    copy(b1.begin(),b1.end()-k,g0.begin()); // vector<int> g0(b1.begin(),b1.end()-k); // quo(b,x^k), C0 in Yap
    if (f.size()>k){
      g1.resize(f.size()-k);
      copy(f.begin(),f.end()-k,g1.begin()); // quo(f,x^k), D0 in Yap
    }
    vector<int> & SA=b0, &SB=b1,&SC=q,&SD=f;
    if (debug_infolevel>2)
      CERR << CLOCK()*1e-6 << " hgcdint 2nd recursive call " << dega0 << "," << dega1 << '\n';
    if (!hgcdint(g0,g1,modulo,Wp,SA,SB,SC,SD,coeffv,degv,tmp0,tmp1,A,B,C,D))
      return false;
    if (debug_infolevel>2)
      CERR << CLOCK()*1e-6 << " hgcdint end 2nd recursive call " << dega0 << "," << dega1 << '\n';
    matrix22int(RA,RB,RC,RD,SA,SB,SC,SD,A,B,C,D,modulo,tmp0,Wp);
    if (debug_infolevel>2)
      CERR << CLOCK()*1e-6 << " hgcdint end " << dega0 << "," << dega1 << '\n';
    return true;
  }

  // a0.size() must be > a1.size()
  // Computes the coefficient of a transition matrix M=[[A,B],[C,D]]
  // such that M*[a0,a1]=[a_k,a_(k+1)] where 
  // degree(a_k) >= m > degree(a_(k+1)), m=degree(a_0)/2 top rounded
  // if degrees are small, use iterative extended Euclide and computes 
  // a=A*a0+B*a1 and b=C*a0+D*a1
  // otherwise a is empty (and b also)
  // https://pdfs.semanticscholar.org/a7e7/b01a3dd6ac0ec160b35e513c5efa38c2369e.pdf (Yap half gcd algorithm lecture p.59)
  bool hgcd(const modpoly & a0,const modpoly & a1,const gen & modulo,modpoly &A,modpoly &B,modpoly &C,modpoly &D,modpoly & a,modpoly & b,modpoly & tmp1,modpoly & tmp2){ // a0 is A in Yap, a1 is B
    a.clear(); b.clear();
    int dega0=a0.size()-1,dega1=a1.size()-1;
    int m=(dega0+1)/2;
    if (dega1<m){
      D=A=makevecteur(1);
      B.clear(); C.clear();
      a=a0; b=a1;
      return true;
    }
    if (modulo.type==_INT_
#ifndef INT128
	  && dega0*double(modulo.val)*modulo.val<(1ULL<<63)
#endif
	){
      int p=modulo.val;
      vector<int> Wp,a0i,a1i,Ai,Bi,Ci,Di,coeffv,degv,tmp1,tmp2,tmp3,tmp4,tmp5,tmp6;
      vecteur2vector_int(a0,p,a0i);
      vecteur2vector_int(a1,p,a1i);
      if (hgcdint(a0i,a1i,p,Wp,Ai,Bi,Ci,Di,coeffv,degv,tmp1,tmp2,tmp3,tmp4,tmp5,tmp6)){
	vector_int2vecteur(Ai,A);
	vector_int2vecteur(Bi,B);
	vector_int2vecteur(Ci,C);
	vector_int2vecteur(Di,D);
	return true;
      }
    }
    environment env;
    env.modulo=modulo;
    env.moduloon=true;
    if (
	(modulo.type==_INT_  && m<64) || // m<64 seems optimal
	m<HGCD
	){ 
      if (debug_infolevel>2)
	CERR << CLOCK()*1e-6 << " halfgcd iter m=" << m << " dega0/a1 " << dega0 << "," << dega1 << '\n';
      if (modulo.type==_INT_ && modulo.val
#ifndef INT128
	  && dega0*double(modulo.val)*modulo.val<(1ULL<<63)
#endif
	  ){
	vector<int> a0i,b0i,ua,ub,va,vb,coeffv,degv,tmp0,tmp1,tmp2,tmp3,tmp4,tmp5; 
	int p=modulo.val;
	vecteur2vector_int(a0,p,a0i);
	vecteur2vector_int(a1,p,b0i);
	hgcd_iter_int(a0i,b0i,m,ua,ub,va,vb,p,coeffv,degv,tmp0,tmp1,tmp2,tmp3,tmp4,tmp5);
	vector_int2vecteur(ua,A);
	vector_int2vecteur(ub,C);
	vector_int2vecteur(va,B); 
	vector_int2vecteur(vb,D); 
	return true;
      }
      if (egcd_mpz(a0,a1,m,modulo,C,D,b,&A,&B,&a)){
	//CERR << a0 << " " << a1 << " " << A << " " << B << " " << C << " " << D << '\n';
	if (debug_infolevel>2)
	  CERR << CLOCK()*1e-6 << " halfgcd mpz iter end" << dega0 << "," << dega1 << '\n';
	return true;
      }
      // limit on m depends on modulo (smaller limit is faster for large modulo)
      modpoly q,r,tmp;
      a=a0;
      b=a1; 
      // initializes ua to 1 and ub to 0, the coeff of u in ua*a+va*b=a
      modpoly ua(one()),ub,va,vb(one()),ur,vr;
      // DivRem: a = bq+r 
      // hence ur <- ua-q*ub, vr <- va-q*vb verify
      // ur*a+vr*b=r
      // a <- b, b <- r, ua <- ub and ub<- ur
      for (;;){
	int n=int(b.size())-1;
	if (n<m){ // degree(b) is small enough
#ifdef HGCD_DIV
	  va=operator_div(operator_minus(a,operator_times(ua,a0,&env),&env),a1,&env); // ua*a0+va*b0=a
	  vb=operator_div(operator_minus(b,operator_times(ub,a0,&env),&env),a1,&env); // ub*a0+vb*b0=b
#endif
	  //if (ua!=A)
	  A.swap(ua);
	  //if (va!=B)
	  B.swap(va);
	  //if (ub!=C)
	  C.swap(ub);
	  //if (vb!=D)
	  D.swap(vb);
	  if (debug_infolevel>2)
	    CERR << CLOCK()*1e-6 << " halfgcd iter end" << dega0 << "," << dega1 << '\n';
	  //CERR << a0 << " " << a1 << " " << A << " " << B << " " << C << " " << D << '\n';
	  return true;
	}
	DivRem(a,b,&env,q,r); // division works always
	operator_times(q,ub,&env,tmp); submodpoly(ua,tmp,&env,ur); // ur=ua-q*ub;
#ifndef HGCD_DIV
	operator_times(q,vb,&env,tmp); submodpoly(va,tmp,&env,vr); // vr=va-q*vb;
	swap(va,vb); swap(vb,vr); // va=vb; vb=vr;
#endif
	swap(a,b); swap(b,r); // a=b; b=r;
	swap(ua,ub); swap(ub,ur); // ua=ub; ub=ur;
      }
      return false; // never reached
    }
    // 1st recursive call
    modpoly b0(a0.begin(),a0.end()-m); // quo(a0,x^m), A0 in Yap
    modpoly b1(a1.begin(),a1.end()-m); // quo(a1,x^m), B0 in Yap
    modpoly RA,RB,RC,RD,q,f;
    if (debug_infolevel>2)
      CERR << CLOCK()*1e-6 << " halfgcd 1st recursive call " << dega0 << "," << dega1 << '\n';
    if (!hgcd(b0,b1,modulo,RA,RB,RC,RD,a,b,tmp1,tmp2))
      return false;
    if (debug_infolevel>2)
      CERR << CLOCK()*1e-6 << " halfgcd compute A' B' " << dega0 << "," << dega1 << '\n';
    int maxadeg=dega0+1-giacmax(RA.size(),RB.size()),maxbdeg=dega0-m/2;
    matrix22timesvect(RA,RB,RC,RD,a0,a1,maxadeg,maxbdeg,a,b,env,tmp1,tmp2);
    int dege=b.size()-1;
    if (dege<m){
      A.swap(RA); B.swap(RB); C.swap(RC); D.swap(RD);  
      a.clear(); b.clear();
      return true;
      // A=RA; B=RB; C=RC; D=RD; return true;
    }
    if (dege>=a.size()-1)
      COUT << "hgcd error" << '\n';
    if (debug_infolevel>2)
      CERR << CLOCK()*1e-6 << " halfgcd euclid div " << dega0 << "," << dega1 << '\n';
    // 1 euclidean step
    if (!DivRem(a,b,&env,q,f)) // q,f are Q,D in Yap 
      return false;
    // [[0,1],[1,-q]]*[[RA,RB],[RC,RD]] == [[RC,RD],[-RC*q+RA,-RD*q+RB]]
    a_bc(RA,RC,q,&env,RA,tmp1); // RA=trim(RA-RC*q,&env);
    a_bc(RB,RD,q,&env,RB,tmp1); // RB=trim(RB-RD*q,&env);
    int l=b.size()-1,k=2*m-l;
    if (f.size()-1<m){
      A.swap(RC); B.swap(RD); C.swap(RA); D.swap(RB); return true;
    }
    modpoly g0(b.begin(),b.end()-k); // quo(b,x^k), C0 in Yap
    modpoly g1;
    if (f.size()>k)
      g1=modpoly(f.begin(),f.end()-k); // quo(f,x^k), D0 in Yap
    modpoly SA,SB,SC,SD;
    if (debug_infolevel>2)
      CERR << CLOCK()*1e-6 << " halfgcd 2nd recursive call " << dega0 << "," << dega1 << '\n';
    if (!hgcd(g0,g1,modulo,SA,SB,SC,SD,b0,b1,tmp1,tmp2))
      return false;
    if (debug_infolevel>2)
      CERR << CLOCK()*1e-6 << " halfgcd end 2nd recursive call " << dega0 << "," << dega1 << '\n';
    matrix22(RA,RB,RC,RD,SA,SB,SC,SD,A,B,C,D,env,tmp1,tmp2);
    if (debug_infolevel>2)
      CERR << CLOCK()*1e-6 << " halfgcd end " << dega0 << "," << dega1 << '\n';
    a.clear(); b.clear();
    return true;
  }

  // assumes degree(q)>degree(rem)
  static bool halfgcdmodpoly(modpoly &q,modpoly & rem,environment * env,modpoly & a,modpoly & RA,modpoly &RB,modpoly &RC,modpoly &RD,modpoly &b0,modpoly & b1,modpoly & tmp1,modpoly & tmp2){
    if (rem.size()<HGCD)
      return gcdmodpoly(q,rem,env,a);
    // now gcd(q,rem) with q.size()>rem.size()
    if (hgcd(q,rem,env->modulo,RA,RB,RC,RD,b0,b1,tmp1,tmp2)){
      if (b0.empty() || b1.empty()){
	int maxadeg=q.size()-giacmax(RA.size(),RB.size()),maxbdeg=q.size()/2;
#if 1
	matrix22timesvect(RA,RB,RC,RD,q,rem,maxadeg,maxbdeg,b0,b1,*env,tmp1,tmp2);
#else
	//ab_cd(q.size()-1,RA,q,RB,rem,env,b0,quo,tmp); // b0=trim(RA*q+RB*rem,env);
	ab_cd(maxadeg,RA,q,RB,rem,env,b0,tmp1,tmp2); // b0=trim(RA*q+RB*rem,env);
	ab_cd(maxbdeg,RC,q,RD,rem,env,b1,tmp1,tmp2); // b1=trim(RC*q+RD*rem,env);
#endif
      }
      else
	;//CERR << b1.size() << '\n';
      if (b1.empty()){
	a=b0;
	mulmodpoly(a,invenv(a.front(),env),env,a);
	return true;
      }
      DivRem(b0,b1,env,tmp1,rem);
      if (rem.empty()){
	a=b1;
	mulmodpoly(a,invenv(a.front(),env),env,a);
	return true;
      }
      return halfgcdmodpoly(b1,rem,env,a,RA,RB,RC,RD,b0,q,tmp1,tmp2);
    }
    return false;
  }

  void reim(const modpoly &p,modpoly & pr,modpoly & pi){
    size_t s=p.size();
    pr.reserve(s); pi.reserve(s);
    gen R,I;
    for (size_t i=0;i<s;++i){
      reim(p[i],R,I,context0);
      pr.push_back(R);
      pi.push_back(I);
    }
  }

  void euclide_gcd(const modpoly &p,const modpoly & q,environment * env,modpoly &a){
    a=p;
    modpoly b(q);
    modpoly quo,rem;
    while (!b.empty()){
      gen s=b.front();
      mulmodpoly(b,invenv(s,env),env,b);
      DivRem(a,b,env,quo,rem);
      // COUT << "a:" << a << "b:" << b << "q:" << quo << "r:" << rem << '\n';
      swap(a,b); // newa=b,  
      swap(b,rem); // newb=rem
    }
    if (!a.empty())
      mulmodpoly(a,invenv(a.front(),env),env,a);
  }

  bool gcdmodpoly(const modpoly &p,const modpoly & q,environment * env,modpoly &a){
    if (!env){
#ifndef NO_STDEXCEPT
      setsizeerr();
#endif
      return false;
    }
#if 1 // DivRem should be fast, Yap p.54
    if (p.size()<q.size()) return gcdmodpoly(q,p,env,a);
    // run half_gcd(2) to debug
    if (env->moduloon && env->complexe && env->modulo.type==_INT_ && env->modulo.val % 4==1){
      // find a sqrt of -1 mod modulo
      // replace i by + or - this sqrt
      // find gcd_+ and gcd_-, if same degree 
      // check gcd=1/2*(gcd_+ + gcd_-)+i/2(gcd_+ - gcd_-)
      modpoly pr,pi,qr,qi,p1,p2,q1,q2,g1,g2;
      reim(p,pr,pi);
      reim(q,qr,qi);
      int i=modsqrtminus1(env->modulo.val);
      gen ig(i);
      p1=pr+ig*pi; p2=pr-ig*pi;
      q1=qr+ig*qi; q2=qr-ig*qi;
      env->complexe=false;
      if (gcdmodpoly(p1,q1,env,g1) && gcdmodpoly(p2,q2,env,g2) && g1.size()==g2.size()){
	env->complexe=true;
	a=(g1+g2)-cst_i*ig*(g1-g2);
	mulmodpoly(a,invmod(2,env->modulo),env,a);
	modpoly & q=g1; modpoly & r=g2;
	DivRem(p,a,env,q,r);
	if (r.empty()){
	  DivRem(q,a,env,q,r);
	  if (r.empty())
	    return true;
	}
      }
      env->complexe=true;
    }
    if (env->moduloon && !env->complexe && p.size()>=HGCD && q.size()>=HGCD){
      modpoly rem,quo;
      DivRem(p,q,env,quo,rem);
      if (rem.empty()){
	a=q;
	mulmodpoly(a,invenv(a.front(),env),env,a);
	return true;
      }
      modpoly Q(q),RA,RB,RC,RD,b0,b1,tmp1,tmp2;
      if (debug_infolevel)
	CERR << CLOCK()*1e-6 <<" halfgcd begin" << '\n';
      bool b=halfgcdmodpoly(Q,rem,env,a,RA,RB,RC,RD,b0,b1,tmp1,tmp2);
      if (debug_infolevel)
	CERR << CLOCK()*1e-6 <<" halfgcd end" << '\n';
      return b;
    }
#endif
#if !defined(EMCC) && !defined(EMCC2)
    if (env->moduloon && is_zero(env->coeff) && !env->complexe && env->modulo.type==_INT_ && env->modulo.val < (1 << 15) ){
      gcdsmallmodpoly(p,q,env->modulo.val,a);
      return true;
    }
#endif
#if 0
    if (env->moduloon && is_zero(env->coeff) && !env->complexe && env->modulo.type==_INT_ && env->modulo.val < (1 << 26) ){
      if (gcddoublemodpoly(p,q,env->modulo.val,a))
	return true;
    }
#endif
    euclide_gcd(p,q,env,a);
    return true;
  }

  // compute gcd of p and q mod m, result in d
  void gcdsmallmodpoly(const vector<int> &p,const vector<int> & q,int m,vector<int> & d){
    gcdsmallmodpoly(p,q,m,d,0,0);
    return;
#if 0
    int as=int(p.size()),bs=int(q.size());
    if (!as){ d=q; return ; }
    if (!bs){ d=p; return ; }
#if defined VISUALC || defined BESTA_OS
    int *asave=new int[as], *a=asave,*aend=a+as,*qcur=0;
    int *bsave=new int[bs], *b=bsave,*bend=b+bs;
#else
    int asave[as], *a=asave,*aend=a+as,*qcur=0;
    int bsave[bs], *b=bsave,*bend=b+bs;
#endif
    memcpy(a,&*p.begin(),as*sizeof(int));
    memcpy(b,&*q.begin(),bs*sizeof(int));
    int * t;
    for (;b!=bend;){
      rem(a,aend,b,bend,m,qcur,0);
      t=a; a=b; b=t;
      t=aend; aend=bend; bend=t;      
    }
    d.clear();
    d.reserve(aend-a);
    int ainv=1;
    if (a!=aend)
      ainv=invmod(*a,m);
    if (m>=46340){
      for (;a!=aend;++a){
	d.push_back(smod((*a)*longlong(ainv),m));
      }
    }
    else {
      for (;a!=aend;++a){
	d.push_back(smod((*a)*ainv,m));
      }
    }
#if defined VISUALC || defined BESTA_OS
    delete [] asave;
    delete [] bsave;
#endif
#endif
  }

  void gcdsmallmodpoly(const vector<int> &p,const vector<int> & q,int m,vector<int> & d,vector<int> * pcof,vector<int> * qcof){
    int as=int(p.size()),bs=int(q.size());
    if (!as){ 
      // p==0, pcof is undefined
      if (pcof)
	pcof->clear();
      d=q; 
      if (qcof){
	qcof->clear();
	qcof->push_back(1);
      }
      return ; 
    }
    if (!bs){ 
      // q==0
      if (qcof)
	qcof->clear();
      d=p; 
      if (pcof){
	pcof->clear();
	pcof->push_back(1);
      }
      return ; 
    }
    int ms=std::max(as,bs);
#if defined VISUALC || defined BESTA_OS
    int *asave=new int[ms], *a=asave,*aend=a+as,*qcur=0,*qend;
    int *bsave=new int[ms], *b=bsave,*bend=b+bs;
#else
    int asave[ms], *a=asave,*aend=a+as,*qcur=0,*qend;
    int bsave[ms], *b=bsave,*bend=b+bs;
#endif
    bool swapab=false;
    memcpy(a,&*p.begin(),as*sizeof(int));
    memcpy(b,&*q.begin(),bs*sizeof(int));
    int * t;
    for (;b!=bend;swapab=!swapab){
      if (*b % m==0){ // make sure leading coeff of b is not 0
	++b; continue;
      }
      rem(a,aend,b,bend,m,qcur,0);
      t=a; a=b; b=t;
      t=aend; aend=bend; bend=t;      
    }
    if (a==aend){ // should not happen!
#if defined VISUALC || defined BESTA_OS
      delete [] asave;
      delete [] bsave;
#endif
      return;
    }
    // normalize gcd
    int ainv=1;
    ainv=invmod(*a,m);
    if (ainv!=1){
      for (int * acur=a;acur!=aend;++acur)
	*acur = smod((*acur)*longlong(ainv),m);
    }
#if defined VISUALC || defined BESTA_OS
    int * cof=new int[ms];
#else
    int cof[ms];
#endif
    // find p cofactor
    if (pcof){
      qcur=cof;
      qend=cof+ms;
      b=swapab?asave:bsave;
      bend=b+as;
      memcpy(b,&*p.begin(),as*sizeof(int));
      rem(b,bend,a,aend,m,qcur,qend,true);
      pcof->clear();
      pcof->reserve(qend-qcur);
      for (;qcur!=qend;++qcur)
	pcof->push_back(*qcur);
    }
    if (qcof){
      qcur=cof;
      qend=cof+ms;
      b=swapab?asave:bsave;
      bend=b+bs;
      memcpy(b,&*q.begin(),bs*sizeof(int));
      rem(b,bend,a,aend,m,qcur,qend,true);
      qcof->clear();
      qcof->reserve(qend-qcur);
      for (;qcur!=qend;++qcur)
	qcof->push_back(*qcur);
    }
    d.clear();
    d.reserve(aend-a);
    for (;a!=aend;++a)
      d.push_back(*a);
    // CERR << d << " " << pcof << " " << p << '\n';
    // CERR << d << " " << qcof << " " << q << '\n';
#if defined VISUALC || defined BESTA_OS
    delete [] asave;
    delete [] bsave;
    delete [] cof;
#endif
  }

  static void dbgp(const modpoly & a){
    COUT << a << '\n';
  }

  static bool content_mod(const polynome & p,vecteur & gcd,environment * env){
    vector< monomial<gen> >::const_iterator it=p.coord.begin(),itend=p.coord.end();
    for (;it!=itend;++it){
      if (gcd.size()==1 || it->value.type!=_VECT){
	gcd=vecteur(1,1);
	break;
      }
      gcdmodpoly(gcd,*it->value._VECTptr,env,gcd);
      if (is_undef(gcd))
	return false;
    }
    return true;
  }

  gen hornermod(const vecteur & v,const gen & alpha,const gen & modulo){
    gen res;
    const_iterateur it=v.begin(),itend=v.end();
    for (;it!=itend;++it){
      res = smod(res*alpha+*it,modulo);
    }
    return res;
  }

  int hornermod(const vecteur & v,int alpha,int modulo){
    int res=0;
    const_iterateur it=v.begin(),itend=v.end();
    for (;it!=itend;++it){
      /*
	if (it->type!=_INT_){
	CERR << v << '\n';
	setsizeerr(gen(v).print(context0));
	}
      */
      res = (res*alpha+it->val)%modulo;
    }
    return smod(res,modulo);
  }

  // eval p at xn=alpha modulo
  static polynome pevaln(const polynome & p,const gen & alpha,const gen & modulo,index_t * & degptr,bool estreel){
    int a=0,m=0,dim=p.dim;
    bool nonmod = is_zero(modulo);
    bool smallmod = estreel && alpha.type==_INT_ && modulo.type==_INT_ && (m=modulo.val)<46340 && (a=alpha.val)<46340 ;
    vector< monomial<gen> >::const_iterator it=p.coord.begin(),itend=p.coord.end();
    polynome res(dim);
    res.coord.reserve(itend-it);
    gen tmp;
    for (;it!=itend;++it){
      if (it->value.type==_VECT)
	tmp=nonmod?horner(*it->value._VECTptr,alpha):(smallmod?hornermod(*it->value._VECTptr,a,m):hornermod(*it->value._VECTptr,alpha,modulo));
      else
	tmp=it->value; // smod(it->value,modulo);
      if (!is_zero(tmp))
	res.coord.push_back(monomial<gen>(tmp,it->index));
      else {
	if (degptr){
	  // if one of the indices of it->index is the same as *degptr
	  // the lcoeff with respect to this variable may vanish
	  for (int i=0;i<dim;++i){
	    if ((*degptr)[i]==(it->index)[i]){
	      degptr=0;
	      break;
	    }
	  }
	}
      }
    }
    return res;
  }

  static bool divmod(polynome & p,const vecteur & v,environment * env){
    if (v.size()==1){
      if (!is_one(v.front())){
	if (!env || !env->moduloon || !is_zero(env->coeff))
	  return false; // setsizeerr();
	p=invmod(v.front(),env->modulo)*p;
      }
    }
    else {
      vector< monomial<gen> >::iterator it=p.coord.begin(),itend=p.coord.end();
      vecteur q,r;
      for (;it!=itend;++it){
	if (it->value.type!=_VECT)
	  return false; // setsizeerr();
	DivRem(*it->value._VECTptr,v,env,q,r);
	it->value=gen(q,_POLY1__VECT);
      }
    }
    return true;
  }

  static bool pp_mod(polynome & p,vecteur & v,environment * env){
    content_mod(p,v,env);
    return divmod(p,v,env);
  }

  // extract xn dependency as a modpoly
  static void convert_xn(const polynome & p,polynome & res){
    int dim=p.dim;
    res.dim=dim-1;
    vector< monomial<gen> >::const_iterator it=p.coord.begin(),itend=p.coord.end();
    index_t old(dim,-1);
    vecteur cur;
    for (;it!=itend;++it){
      const index_t & curi=it->index.iref();
      old[dim-1]=curi[dim-1];
      if (curi==old){
	cur[curi[dim-1]]=it->value;
      }
      else {
	if (!cur.empty()){
	  reverse(cur.begin(),cur.end());
	  res.coord.push_back(monomial<gen>(gen(cur,_POLY1__VECT),index_t(old.begin(),old.end()-1)));
	}
	old=curi;
	cur=vecteur(curi[dim-1]+1);
	cur[curi[dim-1]]=it->value;
      }
    }
    if (!cur.empty()){
      reverse(cur.begin(),cur.end());
      res.coord.push_back(monomial<gen>(gen(cur,_POLY1__VECT),index_t(old.begin(),old.end()-1)));
    }
  }

  // put back xn dependency as a modpoly
  static void convert_back_xn(const polynome & p,polynome & res){
    res.coord.clear();
    int dim=p.dim,deg;
    res.dim=dim+1;
    vector< monomial<gen> >::const_iterator it=p.coord.begin(),itend=p.coord.end();
    for (;it!=itend;++it){
      index_t i(it->index.iref());
      i.push_back(0);
      if (it->value.type!=_VECT)
	res.coord.push_back(monomial<gen>(it->value,i));
      else {
	const_iterateur jt=it->value._VECTptr->begin(),jtend=it->value._VECTptr->end();
	deg=int(jtend-jt)-1;
	for (;jt!=jtend;++jt,--deg){
	  if (!is_zero(*jt)){
	    i[dim]=deg;
	    res.coord.push_back(monomial<gen>(*jt,i));
	  }
	}
      }
    }
  }

  /*
  void modgcd_bi(const polynome &pxn, const polynome & qxn, int modulo,int gcddeg, polynome & d,polynome & pcofactor,polynome & qcofactor){
    d=polynome(dim-1);
    // we are now interpolating G=gcd(p,q)*a poly/x1
    // such that the leading coeff of G is Delta
    int pdeg(pxn.lexsorted_degree()),qdeg(qxn.lexsorted_degree()); 
    int delta=min(pdeg,qdeg);
    int e=0; // number of evaluations
    int alpha=0,ps,qs;
    if (debug_infolevel>1)
      CERR << "gcdmod find alpha dim " << d.dim << " " << CLOCK() << '\n';
    vector<int> palpha,qalpha,pcofactalpha,qcofactalpha,g,g1;
    for (;;++alpha){
      if (alpha==modulo)
	setsizeerr(gettext("Modgcd: no suitable evaluation point"));
      if (debug_infolevel>1)
	CERR << "gcdmod eval " << alpha << " dim " << d.dim << " " << CLOCK() << '\n';
      palpha=pevaln(pxn,alpha,modulo);
      if (palpha.empty())
	continue;
      if ((ps=palpha.size())==1){ 
	if (pdeg) 
	  continue;
	// gcd may only depend on first var
	d=cont;
	return;
      }
      qalpha=pevaln(qxn,alpha,modulo);
      if (qalpha.empty()) 
	continue;
      if ((qs=qalpha.size())==1){ 
	if (qdeg) 
	  continue;
	d=cont;
	return;
      }
      if ( ps!=pdeg+1 || qs!=qdeg+1 )
	continue;
      // palpha and qalpha are p_prim and q_prim evaluated at xn=alpha
      if (debug_infolevel>1)
	CERR << "gcdmod gcd at " << alpha << " dim " << d.dim << " " << CLOCK() << '\n';
      gcdsmallmodpoly(palpha,qalpha,modulo,g,pcofactalpha,qcofactalpha);
      int gdeg(g.size()-1);
      int gcd_plus_delta_deg=gcddeg+Delta.size()-1;
      if (gdeg==delta){
	if (debug_infolevel>1)
	  CERR << "gcdmod interp dim " << d.dim << " " << CLOCK() << '\n';
	g1=g;
	mulmodpoly(g1,smod(hornermod(Delta,alpha,modulo)*invmod(g.front(),modulo),modulo),modulo);
	smallmodpoly2modpoly(g1-pevaln(d,alpha,modulo),modulo,g1);
	mulpoly(g1,smod(invmod(hornermod(interp,alpha,modulo),modulo)*gen(interp,_POLY1__VECT),modulo),g1);
	smod(d+g1,modulo,d);
	interp=operator_times(interp,makevecteur(1,-alpha),&env);
	++e;
	if (e>gcddeg
	    || is_zero(g1)
	    ){
	  if (debug_infolevel)
	    CERR << "gcdmod pp1mod dim " << d.dim << " " << CLOCK() << '\n';
	  polynome pD,QP(dim),QQ(dim),R(d);
	  vecteur vtmp;
	  pp_mod(R,vtmp,&env);
	  convert_back_xn(R,pD);
	  // This removes the polynomial in x1 that we multiplied by
	  // (it was necessary to know the lcoeff of the interpolated poly)
	  if (debug_infolevel)
	    CERR << "gcdmod check dim " << d.dim << " " << CLOCK() << '\n';
	  divremmod(p,pD,modulo,QP,R);
	  // Now, gcd divides pD for gcddeg+1 values of x1
	  // degree(pD)<=degree(gcd)
	  if (R.coord.empty()){
	    divremmod(q,pD,modulo,QQ,R);
	    // If pD divides both P and Q, then the degree wrt variables
	    // x1,...,xn-1 is the right one (because it is <= since pD 
	    // divides the gcd and >= since pD(xn=one of the try) was a gcd
	    // The degree in xn is the right one because of the condition
	    // on the lcoeff
	    // Note that the division test might be much longer than the
	    // interpolation itself (e.g. if the degree of the gcd is small)
	    // but it seems unavoidable, for example if 
	    // P=Y-X+X(X-1)(X-2)(X-3)
	    // Q=Y-X+X(X-1)(X-2)(X-4)
	    // then gcd(P,Q)=1, but if we take Y=0, Y=1 or Y=2
	    // we get gcddeg=1 (probably degree 1 for the gcd)
	    // interpolation at X=0 and X=1 will lead to Y-X as candidate gcd
	    // and even adding X=2 will not change it
	    // We might remove division if we compute the cofactors of P and Q
	    // if P=pD*cofactor is true for degree(P) values of x1
	    // and same for Q, and the degrees wrt xn of pD and cofactors
	    // have sum equal to degree of P or Q + lcoeff then pD is the gcd
	    if (R.coord.empty()){
	      pD=pD*cont;
	      d=smod(pD*invmod(pD.coord.front().value,modulo),modulo);
	      pcofactor=pcofactor*QP;
	      pcofactor=smod(p_orig.coord.front().value*invmod(pcofactor.coord.front().value,modulo)*pcofactor,modulo);
	      qcofactor=qcofactor*QQ;
	      qcofactor=smod(q_orig.coord.front().value*invmod(qcofactor.coord.front().value,modulo)*qcofactor,modulo);
	      if (debug_infolevel)
		CERR << "gcdmod found dim " << d.dim << " " << CLOCK() << '\n';
	      return;
	    }
	  }
	  if (debug_infolevel)
	    CERR << "Gcdmod bad guess " << '\n';
	  continue;
	}
	else
	  continue;
      }
      if (gdeg[0]>delta[0]) 
	continue;
      if (delta[0]>=gdeg[0]){ // restart with g
	gcdv=vecteur(1,g);
	alphav=vecteur(1,alpha);
	delta=gdeg;
	d=(g*smod(hornermod(Delta,alpha,modulo),modulo))*invmod(g.coord.front().value,modulo);
	e=1;
	interp=makevecteur(1,-alpha);
	continue;
      }
  }
  */

  static int degree_xn(const polynome & p){
    int res=1;
    vector< monomial<gen> >::const_iterator it=p.coord.begin(),itend=p.coord.end();
    for (;it!=itend;++it){
      if (it->value.type==_VECT)
	res=giacmax(res,int(it->value._VECTptr->size()));
    }
    return res-1;
  }

  inline gen lcoeff(const polynome & p){
    return p.coord.empty()?0:p.coord.front().value;
  }

  // Find non zeros coeffs of p
  static int find_nonzero(const modpoly & p,index_t & res){
    res.clear();
    const_iterateur it=p.begin(),itend=p.end();
    res.reserve(itend-it);
    if (it==itend)
      return 0;
    int nzeros=0;
    for (;it!=itend;++it){
      if (is_zero(*it)){
	res.push_back(0);
	++nzeros;
      }
      else 	
	res.push_back(1);
    }
    return nzeros;
  }

  static void make_modprimitive_xn(polynome & p,const gen & modulo,polynome & content){
    int dim=p.dim,pxns=1;
    vector< polynome > pxn(1,polynome(dim));
    polynome d(dim),pcof(dim),qcof(dim);
    // fill pxn (not sorted)
    vector< monomial<gen> >::const_iterator it=p.coord.begin(),itend=p.coord.end();
    for (;it!=itend;++it){
      if (it->value.type!=_VECT)
	pxn[0].coord.push_back(*it);
      else {
	vecteur & v=*it->value._VECTptr;
	int j=int(v.size())-1;
	if (j>=0){
	  for (;j>=pxns;++pxns)
	    pxn.push_back(polynome(dim));
	  const_iterateur jt=v.begin(); // ,jtend=v.end();
	  for (;j>=0;--j,++jt){
	    if (!is_zero(*jt))
	      pxn[j].coord.push_back(monomial<gen>(*jt,it->index));
	  }
	}
      }
    }
    content.dim=dim;
    content.coord.clear();
    // now for each polynomial in pxn, sort and find gcd with content
    for (int j=pxns-1;j>=0;--j){
      pxn[j].tsort();
      modgcd(content,pxn[j],modulo,d,pcof,qcof,false);
      content=d;
      if (Tis_constant<gen>(content)){
	content.coord.front().value=1;
	return;
      }
    }
    polynome q,r;
    divremmod(p,content,modulo,q,r);
    p=q;
  }

  bool convert(const polynome &p_orig, const polynome & q_orig,index_t & d,std::vector<hashgcd_U> & vars,std::vector< T_unsigned<gen,hashgcd_U> > & p,std::vector< T_unsigned<gen,hashgcd_U> > & q){
    int dim=p_orig.dim;
    index_t pdeg(p_orig.degree()),qdeg(q_orig.degree()),pqdeg(pdeg+qdeg);
    // convert p_orig and q_orig to vector< T_unsigned<gen,hashgcd_U> >
    // using pqdeg (instead of max(pdeg,qdeg) because of gcd(lcoeff(p),lcoeff(q)))
    // additional factor 2 since computing cofactors require more
    ulonglong ans=1;
    for (int i=0;i<dim;++i){
      d[i]=2*(pdeg[i]+qdeg[i]+1); 
      int j=1;
      // round to next power of 2
      for (;;j++){
	if (!(d[i] >>= 1))
	  break;
      }
      d[i] = 1 << j;
      ans = ans*unsigned(d[i]);
      if (ans/RAND_MAX>=1){
	return false;
      }
    }
    vars[dim-1]=1;
    for (int i=dim-2;i>=0;--i){
      vars[i]=d[i+1]*vars[i+1];
    }
    convert<gen,hashgcd_U>(p_orig,d,p);
    convert<gen,hashgcd_U>(q_orig,d,q);
    return true;
  }

  bool gcd_modular(const polynome &p_orig, const polynome & q_orig, polynome & pgcd,polynome & pcofactor,polynome & qcofactor,bool compute_cofactors){
    if (debug_infolevel>1)
      CERR << "gcd modular algo begin " << CLOCK() << '\n';
    int dim=p_orig.dim;
    vector< T_unsigned<gen,hashgcd_U> > p,q,g,pcof,qcof;
    index_t d(dim);
    std::vector<hashgcd_U> vars(dim);
    if (dim==1 || p_orig.coord.empty() || is_one(q_orig) || q_orig.coord.empty() || is_one(p_orig) || !convert(p_orig,q_orig,d,vars,p,q) || !gcd(p,q,g,pcof,qcof,vars,compute_cofactors,threads)){
      if (&pcofactor!=&p_orig) pcofactor=p_orig; 
      if (&qcofactor!=&q_orig) qcofactor=q_orig;
      return gcd_modular_algo(pcofactor,qcofactor,pgcd,compute_cofactors);
    }
    convert_from<gen,hashgcd_U>(g,d,pgcd);
    pgcd.dim=qcofactor.dim=pcofactor.dim=dim;
    if (compute_cofactors){
      convert_from<gen,hashgcd_U>(pcof,d,pcofactor);
      convert_from<gen,hashgcd_U>(qcof,d,qcofactor);
    }
    return true;
  }

  bool convert(const polynome &p_orig, const polynome & q_orig,index_t & d,std::vector<hashgcd_U> & vars,std::vector< T_unsigned<int,hashgcd_U> > & p,std::vector< T_unsigned<int,hashgcd_U> > & q,int modulo){
    int dim=p_orig.dim;
    index_t pdeg(p_orig.degree()),qdeg(q_orig.degree());
    // convert p_orig and q_orig to vector< T_unsigned<int,hashgcd_U> >
    // using pqdeg (instead of max(pdeg,qdeg) because of gcd(lcoeff(p),lcoeff(q)))
    // additional factor 2 since computing cofactors require more
    double ans=1;
    d.clear();
    d.reserve(dim);
    for (int i=0;i<dim;++i){
      d.push_back(2*(pdeg[i]+qdeg[i]+1)); 
      if (d[i]<0)
	return false;
      int j=1;
      // round to next power of 2
      for (;;j++){
	if (!(d[i] >>= 1))
	  break;
      }
      d[i] = 1 << j;
      ans = ans*unsigned(d[i]);
      if (ans/RAND_MAX>=(sizeof(hashgcd_U)==4?1:RAND_MAX)) // 1 if hashgcd_U is unsigned int
	return false;
    }
    vars=std::vector<hashgcd_U>(dim);
    vars[dim-1]=1;
    for (int i=dim-2;i>=0;--i){
      vars[i]=d[i+1]*vars[i+1];
    }
    if (!convert_int32(p_orig,d,p,modulo) || !convert_int32(q_orig,d,q,modulo) )
      return false;
    return true;
  }

  bool mod_gcd(const polynome &p_orig, const polynome & q_orig, const gen & modulo, polynome & pgcd,polynome & pcofactor,polynome & qcofactor,bool compute_cofactors){
    if (debug_infolevel)
      CERR << "modgcd begin " << CLOCK()*1e-6 << '\n';
    int dim=p_orig.dim;
    if ( dim==1 || p_orig.coord.empty() || is_one(q_orig) || q_orig.coord.empty() || is_one(p_orig) || modulo.type!=_INT_ ){
      return mod_gcd_c(p_orig,q_orig,modulo,pgcd,pcofactor,qcofactor,compute_cofactors);
    }
    if (debug_infolevel)
      CERR << "modgcd begin dim>=2 " << CLOCK()*1e-6 << '\n';
    std::vector<hashgcd_U> vars(dim);
    vector< T_unsigned<int,hashgcd_U> > p,q,g,pcof,qcof;
    index_t d(dim);
    if (!convert(p_orig,q_orig,d,vars,p,q,modulo.val) || !mod_gcd(p,q,modulo.val,g,pcof,qcof,vars,compute_cofactors,threads))
      return mod_gcd_c(p_orig,q_orig,modulo,pgcd,pcofactor,qcofactor,compute_cofactors);
    convert_from<int,hashgcd_U>(g,d,pgcd);
    pgcd.dim=qcofactor.dim=pcofactor.dim=dim;
    if (compute_cofactors){
      convert_from<int,hashgcd_U>(pcof,d,pcofactor);
      convert_from<int,hashgcd_U>(qcof,d,qcofactor);
    }
    return true;
  }

  bool modgcd(const polynome &p_orig, const polynome & q_orig, const gen & modulo, polynome & d,polynome & pcofactor,polynome & qcofactor,bool compute_cofactors){
    return mod_gcd(p_orig,q_orig,modulo,d,pcofactor,qcofactor,compute_cofactors);
  }

  bool mod_gcd_c(const polynome &p_orig, const polynome & q_orig, const gen & modulo, polynome & d,polynome & pcofactor,polynome & qcofactor,bool compute_cofactors){
    if (p_orig.coord.empty() || is_one(q_orig)){
      d=q_orig;
      if (compute_cofactors){
	pcofactor=p_orig;
	qcofactor=q_orig/d;
      }
      return true;
    }
    if (q_orig.coord.empty() || is_one(p_orig)){
      d=p_orig;
      if (compute_cofactors){
	qcofactor=q_orig;
	pcofactor=p_orig/d;
      }
      return true;
    }
    int dim=p_orig.dim;
    d.dim=dim;
    pcofactor.dim=dim;
    qcofactor.dim=dim;
    environment env;
    env.modulo=modulo;
    env.pn=modulo;
    env.moduloon=true;
    bool estreel;
    if (gcdmod_dim1(p_orig,q_orig,modulo,d,pcofactor,qcofactor,compute_cofactors,estreel))
      return true;
    env.complexe=!estreel;
    if (debug_infolevel)
      CERR << "xn_gcdmod content/x1..xn-1 dim " << dim << " " << CLOCK() << '\n';
    // Make p and q primitive with respect to x1,...,xn-1
    // i.e. the coeff of p and q wrt x1,...,xn-1 which are polynomials in xn
    // are relative prime 
    polynome pxn,qxn,ptmp,qtmp,pcofactorxn,qcofactorxn,dxn,cont;
    convert_xn(p_orig,pxn);
    convert_xn(q_orig,qxn);
    vecteur pcont1,qcont1,pqcont1;
    if (!pp_mod(pxn,pcont1,&env) ||
	!pp_mod(qxn,qcont1,&env))
      return false;
    gcdmodpoly(pcont1,qcont1,&env,pqcont1);
    if (is_undef(pqcont1))
      return false;
    // Make p and q primitive with respect to xn
    // p(x1,...,xn) q(x1,...,xn) viewed as p(xn) and q(xn) 
    // with coeff polynomial wrt x1..xn-1
    make_modprimitive_xn(pxn,modulo,pcofactorxn);
    make_modprimitive_xn(qxn,modulo,qcofactorxn);
    modgcd(pcofactorxn,qcofactorxn,modulo,dxn,ptmp,qtmp,false);
    mulpoly(dxn,pqcont1,dxn);
    convert_back_xn(dxn,cont);
    if (compute_cofactors){
      mulpoly(pcofactorxn,pcont1,pcofactorxn);
      mulpoly(qcofactorxn,qcont1,qcofactorxn);
      convert_back_xn(pcofactorxn,pcofactor);
      convert_back_xn(qcofactorxn,qcofactor);
    }
    // Find degree of gcd with respect to xn, more precisely gcddeg>=degree/xn
    // and compute data for the sparse modular algorithm
    index_t vzero; // coeff of vzero correspond to zero or non zero
    int nzero=1; // Number of zero coeffs
    vecteur alphav,gcdv; // Corresponding values of alpha and gcd at alpha
    int gcddeg=0;
    vecteur b(dim-1),bnext;
    int pxndeg=degree_xn(pxn),qxndeg=degree_xn(qxn);
    for (int essai=0;essai<2;){
      gen pb(peval(pxn,b,modulo));
      gen qb(peval(qxn,b,modulo));
      for (;;){
	bnext=vranm(dim-1,0,0); // find another random point
	if (bnext!=b){ b=bnext; break; }
      }
      if (pb.type==_POLY && !pb._POLYptr->coord.empty())
	pb=pb._POLYptr->coord.front().value;
      if (pb.type!=_VECT || int(pb._VECTptr->size())!=pxndeg+1)
	continue;
      if (qb.type==_POLY && !qb._POLYptr->coord.empty())
	qb=qb._POLYptr->coord.front().value;
      if (qb.type!=_VECT || int(qb._VECTptr->size())!=qxndeg+1)
	continue;
      modpoly db;
      gcdmodpoly(*pb._VECTptr,*qb._VECTptr,&env,db);
      if (is_undef(db))
	return false;
      int dbdeg=int(db.size())-1;
      if (!dbdeg){ 
	gcddeg=0; break; 
      }
      if (!essai){ // 1st gcd test
	gcddeg=dbdeg;
	nzero=find_nonzero(db,vzero);
	++essai;
	continue;
      }
      // 2nd try
      if (dbdeg<gcddeg){ // 1st try was unlucky, restart 1st try
	gcddeg=dbdeg;
	nzero=find_nonzero(db,vzero);
	continue;
      }
      if (dbdeg!=gcddeg) 
	continue;
      // Same gcd degree for 1st and 2nd try, keep this degree
      index_t tmp;
      nzero=find_nonzero(db,tmp);
      if (nzero){
	vzero = vzero | tmp;
	// Recompute nzero, it is the number of 0 coeff of vzero
	index_t::const_iterator it=vzero.begin(),itend=vzero.end();
	for (nzero=0;it!=itend;++it){
	  if (!*it) ++nzero;
	}
      }
      ++essai;
    }
    if (!gcddeg){
      d=cont;
      return true;
    }
    vecteur interp(1,1);
    // gcd of leading coefficients of p and q viewed as poly in X_1...X_n-1
    // with coeff in Z[X_n]
    if (debug_infolevel)
      CERR << "gcdmod lcoeffn dim " << dim-1 << " " << CLOCK() << '\n';
    gen lp(pxn.coord.front().value),lq(qxn.coord.front().value);
    vecteur Delta(1,1),lcoeffp(1,1),lcoeffq(1,1);
    if (lp.type==_VECT)
      lcoeffp=*lp._VECTptr;
    if (lq.type==_VECT)
      lcoeffq=*lq._VECTptr;
    if ((lp.type==_VECT) && (lq.type==_VECT) ){
      gcdmodpoly(lcoeffp,lcoeffq,&env,Delta);
      if (is_undef(Delta))
	return false;
    }
    // estimate time for full lift or division try
    // size=p_orig.size()+q_orig.size()
    // sumdeg=pxndeg+qxndeg
    // %age=gcddeg/min(pxndeg,qxndeg)
    // %age^dim*(1-%age)^dim*size^2 estimates the time for division try
    // gcddeg*size estimates the time for lifting to gcddeg
    // sumdeg*size estimates the time for full lifting
    // if sumdeg<(gcddeg+%age^dim*(1-%age)^dim*size) do full lifting
    int Deltadeg = int(Delta.size())-1,liftdeg=giacmax(pxndeg,qxndeg)+Deltadeg;
    int gcddeg_plus_delta=gcddeg+Deltadeg;
    int liftdeg0=giacmax(liftdeg-gcddeg,gcddeg_plus_delta);
    // once liftdeg0 is reached we can replace g/gp/gq computation
    // by a check that d*dp=dxn*lcoeff(d*dp)/Delta at alpha
    // and d*dq=dxn*lcoeff(d*dq)/lcoeff(qxn) at alpha
    int sumdeg = pxndeg+qxndeg;
    double percentage = double(gcddeg)/giacmin(pxndeg,qxndeg);
    int sumsize = int(p_orig.coord.size()+q_orig.coord.size());
    double gcdlift=gcddeg+std::pow(percentage,dim)*std::pow(1-percentage,dim)*sumsize;
    bool compute_cof = sumdeg<gcdlift/(1+dim);
    polynome p(dim),q(dim);
    if (!compute_cof){
      convert_back_xn(pxn,p);
      convert_back_xn(qxn,q);
    }
    if (debug_infolevel)
      CERR << "dim " << dim << ", full lift:" << sumdeg << " , gcdlift:" << gcdlift/(1+dim) << " compute cofactors=" << compute_cof << '\n';
    d=polynome(dim-1);
    polynome dp(dim-1),dq(dim-1),g1(dim-1);
    // we are now interpolating G=gcd(p,q)*a poly/xn
    // such that the leading coeff of G is Delta
    index_t pdeg(pxn.degree()),qdeg(qxn.degree()); 
    int spdeg=0,sqdeg=0;
    for (int i=0;i<dim-1;++i){
      spdeg += pdeg[i];
      sqdeg += qdeg[i];
    }
    index_t delta=index_min(pdeg,qdeg);
    int e=0; // number of evaluations
    int alpha=0;
    if (debug_infolevel>1)
      CERR << "gcdmod find alpha dim " << d.dim << " " << CLOCK() << '\n';
    for (;;++alpha){
      if (alpha==modulo){
	CERR << "Modgcd: no suitable evaluation point" << '\n';
	return false;
      }
      if (debug_infolevel>1)
	CERR << "gcdmod eval " << alpha << " dim " << d.dim << " " << CLOCK() << '\n';
      index_t * pdegptr=&pdeg;
      const polynome & palpha=pevaln(pxn,alpha,modulo,pdegptr,estreel);
      if (palpha.coord.empty())
	continue;
      if (Tis_constant<gen>(palpha)){ 
	if (spdeg) 
	  continue;
	// gcd may only depend on xn
	d=cont;
	return true;
      }
      if (!pdegptr) 
	continue;
      index_t * qdegptr=&qdeg;
      const polynome & qalpha=pevaln(qxn,alpha,modulo,qdegptr,estreel);
      if (qalpha.coord.empty()) 
	continue;
      if (Tis_constant<gen>(qalpha)){ 
	if (sqdeg) 
	  continue;
	d=cont;
	return true;
      }
      if (!qdegptr)
	continue;
      // palpha/qalpha should have the same degrees than pxn/qxn
      // but the test requires checking all monomials of palpha/qalpha
      // if (palpha.lexsorted_degree()!=pdeg[0] || qalpha.lexsorted_degree()!=qdeg[0] )
      // continue;
      // palpha and qalpha are p_prim and q_prim evaluated at xn=alpha
      if (debug_infolevel>1)
	CERR << "gcdmod gcd at " << alpha << " dim " << d.dim << " " << CLOCK() << '\n';
      polynome g(dim-1),gp(dim-1),gq(dim-1);
      index_t * tmpptr=0;
      if (compute_cof && e>liftdeg0 && e<=liftdeg){
	g=pevaln(d,alpha,modulo,tmpptr,estreel);
	gp=pevaln(dp,alpha,modulo,tmpptr,estreel);
	gq=pevaln(dq,alpha,modulo,tmpptr,estreel);
	// check that g*gp=palpha*lcoeff/lcoeff
	mulpoly(gp,smod(lcoeff(palpha)*invmod(lcoeff(g)*lcoeff(gp),modulo),modulo),gp);
	mulpoly(gq,smod(lcoeff(qalpha)*invmod(lcoeff(g)*lcoeff(gq),modulo),modulo),gq);
	gp=smod(g*gp-palpha,modulo);
	gq=smod(g*gq-qalpha,modulo);
	if (is_zero(gp) && is_zero(gq)){
	  ++e;
	  continue;
	}
      }
      if (!modgcd(palpha,qalpha,modulo,g,gp,gq,compute_cof))
	return false;
      index_t gdeg(g.degree());
      if (gdeg==delta){
	// Try spmod first
	if (!compute_cof && nzero){
	  // Add alpha,g 
	  alphav.push_back(alpha);
	  gcdv.push_back(g);
	  if (gcddeg-nzero==e){ 
	    // We have enough evaluations, let's try SPMOD
#if 1
	    // Build the matrix, each line has coeffs / vzero
	    vector< vector<int> > m,minverse;
	    m.reserve(e+1);
	    for (int j=0;j<=e;++j){
	      index_t::reverse_iterator it=vzero.rbegin(),itend=vzero.rend();
	      vector<int> line;
	      line.reserve(e+1); // overflow if modulo too large
	      for (int p=alphav[j].val,pp=1;it!=itend;++it,pp=smod(p*pp,modulo.val)){
		if (*it)
		  line.push_back(pp);
	      }
	      reverse(line.begin(),line.end());
	      m.push_back(line);
	    }
	    // assume gcd is the vector of non zero coeffs of the gcd in x^n
	    // we have the relation
	    // m*gcd=gcdv
	    // invert m (if invertible)
	    longlong det_mod_p;
	    if (smallmodinv(m,minverse,modulo.val,det_mod_p) && det_mod_p){
	      // hence gcd=minverse*gcdv, where the i-th component of gcd
	      // must be "multiplied" by xn^degree_corresponding_vzero[i]
	      vector< polynome > minversegcd(e+1,polynome(dim));
	      // find bound of required size
	      size_t taille=1;
	      for (int k=0;k<=e;++k){
		if (gcdv[k].type==_POLY)
		  taille += gcdv[k]._POLYptr->coord.size();
	      }
	      for (int j=0;j<=e;++j)
		minversegcd[j].coord.reserve(taille);
	      polynome tmpadd(dim),tmpmult(dim);
	      tmpadd.coord.reserve(taille);
	      size_t taille2=0;
	      for (int j=0;j<=e;++j){
		for (int k=0;k<=e;++k){
		  // smallmult(minverse[j][k],tmpmult,tmpmult,modulo);
		  int fact=minverse[j][k];
		  if (!fact)
		    continue;
		  if (gcdv[k].type==_POLY)
		    tmpmult=*gcdv[k]._POLYptr;
		  else
		    tmpmult=polynome(gcdv[k],dim-1);
		  vector< monomial<gen> >::iterator it=tmpmult.coord.begin(),itend=tmpmult.coord.end();
		  for (;it!=itend;++it){
		    it->value=smod(fact*it->value,modulo);
		  }
		  tmpadd.coord.swap(minversegcd[j].coord);
		  // smalladd(tmpadd,tmpmult,minversegcd[j],modulo);
		  tmpadd.TAdd(tmpmult,minversegcd[j]);
		}
		polynome res(minversegcd[j].dim);
		res.coord.reserve(minversegcd[j].coord.size());
		res.coord.swap(minversegcd[j].coord);
		smod(res,modulo,minversegcd[j]);
		taille2 += minversegcd[j].coord.size();
	      }
	      polynome trygcd(dim-1);
	      index_t::const_iterator it=vzero.begin(),itend=vzero.end();
	      int deg=int(itend-it)-1;
	      for (int pos=0;it!=itend;++it,--deg){
		if (!*it)
		  continue;
		polynome & tmp=minversegcd[pos];
		tmp.untruncn(deg);
		polynome tmpxn;
		convert_xn(tmp,tmpxn);
		trygcd=trygcd+tmpxn;
		++pos;
	      }
#else
	    // Build the matrix, each line has coeffs / vzero
	    if (debug_infolevel>1)
	      CERR << CLOCK()*1e-6 << " SPMOD start" << '\n';
	    matrice m; m.reserve(e+1);
	    for (int j=0;j<=e;++j){
	      index_t::reverse_iterator it=vzero.rbegin(),itend=vzero.rend();
	      vecteur line; line.reserve(e+2);
	      for (gen p=alphav[j],pp=plus_one;it!=itend;++it,pp=smod(p*pp,modulo)){
		if (*it)
		  line.push_back( pp);
	      }
	      reverse(line.begin(),line.end());
	      line.push_back(gcdv[j]);
	      m.push_back(line);
	    }
	    // Reduce linear system modulo modulo
	    gen det; vecteur pivots; matrice mred;
	    if (debug_infolevel>1)
	      CERR << CLOCK()*1e-6 << " SPMOD begin rref" << '\n';
	    if (!modrref(m,mred,pivots,det,0,int(m.size()),0,int(m.front()._VECTptr->size())-1,true,false,modulo,false,false))
	      return false;
	    if (debug_infolevel>1)
	      CERR << CLOCK()*1e-6 << " SPMOD end rref" << '\n';
	    if (!is_zero(det)){	      
	      // Last column is the solution, it should be polynomials
	      // that must be untrunced with index = to non-0 coeff of vzero
	      polynome trygcd(dim);
	      index_t::const_iterator it=vzero.begin(),itend=vzero.end();
	      int deg=int(itend-it)-1;
	      for (int pos=0;it!=itend;++it,--deg){
		if (!*it)
		  continue;
		gen tmp=mred[pos][e+1]; // e+1=#of points -> last col
		if (tmp.type==_POLY){
		  //*tmp._POLYptr=
		  tmp._POLYptr->untruncn(deg);
		  polynome tmpxn;
		  convert_xn(*tmp._POLYptr,tmpxn);
		  trygcd=trygcd+tmpxn;
		}
		else {
		  if (!is_zero(tmp)){
		    vecteur tmpxn(deg+1);
		    tmpxn.front()=tmp;
		    trygcd=trygcd+monomial<gen>(gen(tmpxn,_POLY1__VECT),dim-1);
		  }
		}
		++pos;
	      }
#endif
	      // Check if trygcd is the gcd!
	      vecteur tmpv;
	      if (!pp_mod(trygcd,tmpv,&env))
		return false;
	      polynome pD,QP(dim),QQ(dim),R(dim);
	      convert_back_xn(trygcd,pD);
	      if (debug_infolevel>1)
		CERR << CLOCK()*1e-6 << " SPMOD try gcd candidate" << '\n';
	      if (pD.coord.size()<=p.coord.size() && pD.coord.size()<=q.coord.size() && divremmod(p,pD,modulo,QP,R) && R.coord.empty()){
		if (divremmod(q,pD,modulo,QQ,R) && R.coord.empty()){
		  pD=pD*cont;
		  d=smod(pD*invmod(pD.coord.front().value,modulo),modulo);
		  if (compute_cofactors){
		    pcofactor=pcofactor*QP;
		    pcofactor=smod(p_orig.coord.front().value*invmod(pcofactor.coord.front().value,modulo)*pcofactor,modulo);
		    qcofactor=qcofactor*QQ;
		    qcofactor=smod(q_orig.coord.front().value*invmod(qcofactor.coord.front().value,modulo)*qcofactor,modulo);
		  }
		  return true;
		}
	      }
	    }
	    // SPMOD not successful :-(
	    nzero=0;
	  } // end if gcddeg-nzero==e
	} // end if (nzero)
	if (debug_infolevel>1)
	  CERR << "gcdmod interp dim " << d.dim << " " << CLOCK() << '\n';
	if (compute_cof){
	  // interpolate p cofactor
	  mulpoly(gp,smod(hornermod(lcoeffp,alpha,modulo)*invmod(gp.coord.front().value,modulo),modulo),g1);
	  smod(g1-pevaln(dp,alpha,modulo,tmpptr,estreel),modulo,g1);
	  if (!is_zero(g1)){
	    mulpoly(g1,smod(invmod(hornermod(interp,alpha,modulo),modulo)*gen(interp,_POLY1__VECT),modulo),g1);
	    smod(dp+g1,modulo,dp);
	  }
	  // interpolate q cofactor
	  mulpoly(gq,smod(hornermod(lcoeffq,alpha,modulo)*invmod(gq.coord.front().value,modulo),modulo),g1);
	  smod(g1-pevaln(dq,alpha,modulo,tmpptr,estreel),modulo,g1);
	  if (!is_zero(g1)){
	    mulpoly(g1,smod(invmod(hornermod(interp,alpha,modulo),modulo)*gen(interp,_POLY1__VECT),modulo),g1);
	    smod(dq+g1,modulo,dq);
	  }
	}
	// interp GCD
	mulpoly(g,smod(hornermod(Delta,alpha,modulo)*invmod(g.coord.front().value,modulo),modulo),g1);
	smod(g1-pevaln(d,alpha,modulo,tmpptr,estreel),modulo,g1);
	if (!is_zero(g1)){
	  mulpoly(g1,smod(invmod(hornermod(interp,alpha,modulo),modulo)*gen(interp,_POLY1__VECT),modulo),g1);
	  smod(d+g1,modulo,d);
	}
	interp=operator_times(interp,makevecteur(1,-alpha),&env);
	++e;
	vecteur vtmp;
	if (compute_cof){
	  if (e>liftdeg){ 
	    // divide d,dp,dq by their content in xn
	    if (!pp_mod(d,vtmp,&env) ||
		!pp_mod(dp,vtmp,&env) ||
		!pp_mod(dq,vtmp,&env))
	      return false;
	    polynome pD(dim),PP(dim),QQ(dim);
	    // check xn degrees of d+dp=degree(pxn), d+dq=degree(qxn)
	    int dxndeg=degree_xn(d),dpxndeg=degree_xn(dp),dqxndeg=degree_xn(dq);
	    if ( dxndeg+dpxndeg==degree_xn(pxn) &&
		 dxndeg+dqxndeg==degree_xn(qxn) ){
	      convert_back_xn(d,pD);
	      d=pD*cont;
	      if (compute_cofactors){
		convert_back_xn(dp,PP);
		convert_back_xn(dq,QQ);
		pcofactor=PP*pcofactor;
		qcofactor=QQ*qcofactor;
		pcofactor=smod(p_orig.coord.front().value*invmod(pcofactor.coord.front().value,modulo)*pcofactor,modulo);
		qcofactor=smod(q_orig.coord.front().value*invmod(qcofactor.coord.front().value,modulo)*qcofactor,modulo);
	      }
	      if (debug_infolevel)
		CERR << "gcdmod end dim " << dim << " " << CLOCK() << '\n';
	      return true;
	    }
	    d.coord.clear(); dp.coord.clear(); dq.coord.clear();
	    gcdv.clear(); alphav.clear(); 
	    interp.clear(); interp.push_back(1);
	    e=0;	    
	  }
	}
	else {
	  if (e>gcddeg || is_zero(g1)){
	    if (debug_infolevel)
	      CERR << "gcdmod pp1mod dim " << dim << " " << CLOCK() << '\n';
	    polynome pD,QP(dim),QQ(dim),R(d);
	    if (!pp_mod(R,vtmp,&env))
	      return false;
	    convert_back_xn(R,pD);
	    // This removes the polynomial in xn that we multiplied by
	    // (it was necessary to know the lcoeff of the interpolated poly)
	    if (debug_infolevel)
	      CERR << "gcdmod check dim " << dim << " " << CLOCK() << '\n';
	    // Now, gcd divides pD for gcddeg+1 values of x1
	    // degree(pD)<=degree(gcd)
	    if (divremmod(p,pD,modulo,QP,R) && R.coord.empty()){
	      // If pD divides both P and Q, then the degree wrt variables
	      // x1,...,xn-1 is the right one (because it is <= since pD 
	      // divides the gcd and >= since pD(xn=one of the try) was a gcd
	      // The degree in xn is the right one because of the condition
	      // on the lcoeff
	      // Note that the division test might be much longer than the
	      // interpolation itself (e.g. if the degree of the gcd is small)
	      // but it seems unavoidable, for example if 
	      // P=Y-X+X(X-1)(X-2)(X-3)
	      // Q=Y-X+X(X-1)(X-2)(X-4)
	      // then gcd(P,Q)=1, but if we take Y=0, Y=1 or Y=2
	      // we get gcddeg=1 (probably degree 1 for the gcd)
	      // interpolation at X=0 and X=1 will lead to Y-X as candidate gcd
	      // and even adding X=2 will not change it
	      // We might remove division if we compute the cofactors of P and Q
	      // if P=pD*cofactor is true for degree(P) values of x1
	      // and same for Q, and the degrees wrt xn of pD and cofactors
	      // have sum equal to degree of P or Q + lcoeff then pD is the gcd
	      if (divremmod(q,pD,modulo,QQ,R) &&R.coord.empty()){
		pD=pD*cont;
		d=smod(pD*invmod(pD.coord.front().value,modulo),modulo);
		if (compute_cofactors){
		  pcofactor=pcofactor*QP;
		  pcofactor=smod(p_orig.coord.front().value*invmod(pcofactor.coord.front().value,modulo)*pcofactor,modulo);
		  qcofactor=qcofactor*QQ;
		  qcofactor=smod(q_orig.coord.front().value*invmod(qcofactor.coord.front().value,modulo)*qcofactor,modulo);
		}
		if (debug_infolevel)
		  CERR << "gcdmod found dim " << d.dim << " " << CLOCK() << '\n';
		return true;
	      }
	    }
	    if (debug_infolevel)
	      CERR << "Gcdmod bad guess " << '\n';
	  } // end if (e>gcddeg)
	} // end else [if (compute_cof)]
	continue;
      } // end gdeg==delta
      // FIXME: the current implementation may break if we are unlucky
      // If the degrees of palpha and qalpha are the same than 
      // those of pxn and qxn, delta <- index_min(gdeg,delta)
      // restart with g only if gdeg[j]<=delta[j] for all indices
      // stay with d only if delta[j]<=gdeg[j]
      if (gdeg[0]>delta[0]) 
	continue;
      if (delta[0]>=gdeg[0]){ // restart with g
	gcdv=vecteur(1,g);
	alphav=vecteur(1,alpha);
	delta=gdeg;
	d=(g*smod(hornermod(Delta,alpha,modulo),modulo))*invmod(g.coord.front().value,modulo);
	if (compute_cof){
	  dp=(gp*smod(hornermod(lcoeffp,alpha,modulo),modulo))*invmod(gp.coord.front().value,modulo);
	  dq=(gq*smod(hornermod(lcoeffq,alpha,modulo),modulo))*invmod(gq.coord.front().value,modulo);
	}
	e=1;
	interp=makevecteur(1,-alpha);
	continue;
      }
    }
  }

  void unmodularize(const modpoly &p,const gen & modulo,modpoly &P){
    P.clear(); P.reserve(p.size());
    const_iterateur it=p.begin(),itend=p.end();
    for (;it!=itend;++it){
      const gen & g=*it;
      if (g.type==_MOD)
	P.push_back(*g._MODptr);
      else
	P.push_back(g);
    }
  }

  // Previous Fourier prime, find n-throot of unity
  int prevfourier(unsigned p,int fourier_for_n,int & w){
    int l=sizeinbase2(fourier_for_n);
    unsigned pdiv=p>>l;
    for (--pdiv;pdiv>=(1<<(30-l));--pdiv){
      p=(pdiv<<l)+1;
      int p15=p%15;
      if (p15==0 || p15==3 || p15==6 || p15==9 || p15==12 || p15==5 || p15==10)
	continue;
      // find nthroot of 1 and checks Miller-Rabin primality
      unsigned char charprimes[]={2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97,101,103,107,109,113,127,131,137,139,149,151,157,163,167,173,179,181,191,193,197,199,211,223,227,229,233,239,241,251};
      w=0;
      for (int i=0;i<sizeof(charprimes);++i){
	ulonglong r=powmod(unsigned(charprimes[i]),pdiv,p);
	unsigned w_=r,j;
	if (r==1)
	  continue;
	for (j=1;j<l;++j){
	  r=(r*r)%p;
	  if (r==1 || r==p-1)
	    break;
	}
	if (r!=p-1)
	  break; // p is not prime
	if (j==l-1)
	  w=w_;
	if (w && i>=20)
	  return p;
      }
    }
    return 0;
  }

  // a list of 2048 primes generated by 
  // p:=2^31-2^16
  // L:=[]; for j from 1 to 2^11 do p:=prevprime(p-1); L.append(p); od; L
  const unsigned primes31[]={2147418083,2147418079,2147418067,2147418043,2147418041,2147418037,2147418011,2147418001,2147417969,2147417941,2147417939,2147417929,2147417911,2147417903,2147417869,2147417819,2147417813,2147417773,2147417759,2147417717,2147417707,2147417693,2147417669,2147417663,2147417653,2147417593,2147417551,2147417539,2147417527,2147417521,2147417453,2147417443,2147417423,2147417383,2147417381,2147417351,2147417329,2147417303,2147417267,2147417179,2147417171,2147417141,2147417117,2147417113,2147417093,2147417053,2147417023,2147416991,2147416987,2147416979,2147416967,2147416963,2147416949,2147416927,2147416907,2147416883,2147416877,2147416867,2147416823,2147416813,2147416807,2147416783,2147416771,2147416769,2147416759,2147416721,2147416709,2147416703,2147416657,2147416637,2147416589,2147416573,2147416561,2147416559,2147416553,2147416519,2147416507,2147416483,2147416477,2147416441,2147416379,2147416357,2147416343,2147416339,2147416301,2147416267,2147416241,2147416223,2147416213,2147416207,2147416189,2147416181,2147416177,2147416163,2147416147,2147416133,2147416121,2147416111,2147416109,2147416079,2147416067,2147416057,2147416031,2147416003,2147415989,2147415943,2147415931,2147415889,2147415883,2147415859,2147415857,2147415839,2147415817,2147415811,2147415773,2147415749,2147415737,2147415719,2147415709,2147415703,2147415659,2147415629,2147415619,2147415617,2147415607,2147415563,2147415559,2147415553,2147415541,2147415533,2147415527,2147415499,2147415463,2147415451,2147415449,2147415443,2147415427,2147415373,2147415343,2147415337,2147415323,2147415311,2147415307,2147415203,2147415181,2147415119,2147415107,2147415059,2147415047,2147415041,2147415029,2147415019,2147415013,2147414999,2147414977,2147414953,2147414947,2147414839,2147414837,2147414807,2147414791,2147414767,2147414723,2147414699,2147414651,2147414641,2147414629,2147414617,2147414603,2147414567,2147414531,2147414519,2147414513,2147414509,2147414497,2147414461,2147414441,2147414411,2147414407,2147414377,2147414369,2147414363,2147414317,2147414233,2147414231,2147414207,2147414201,2147414189,2147414183,2147414117,2147414057,2147414053,2147413993,2147413981,2147413979,2147413949,2147413913,2147413903,2147413889,2147413867,2147413859,2147413813,2147413781,2147413757,2147413753,2147413739,2147413729,2147413711,2147413661,2147413657,2147413651,2147413603,2147413559,2147413553,2147413529,2147413519,2147413441,2147413381,2147413343,2147413309,2147413297,2147413291,2147413283,2147413277,2147413273,2147413223,2147413211,2147413201,2147413153,2147413139,2147413109,2147413061,2147413049,2147413001,2147412989,2147412979,2147412977,2147412973,2147412961,2147412947,2147412929,2147412899,2147412877,2147412833,2147412797,2147412791,2147412731,2147412721,2147412691,2147412689,2147412671,2147412667,2147412637,2147412629,2147412601,2147412599,2147412593,2147412581,2147412577,2147412539,2147412521,2147412493,2147412479,2147412467,2147412461,2147412451,2147412413,2147412403,2147412361,2147412359,2147412347,2147412301,2147412277,2147412263,2147412257,2147412247,2147412233,2147412229,2147412209,2147412149,2147412143,2147412133,2147412037,2147412017,2147411963,2147411953,2147411911,2147411887,2147411879,2147411857,2147411839,2147411821,2147411789,2147411743,2147411723,2147411713,2147411683,2147411657,2147411639,2147411621,2147411603,2147411579,2147411561,2147411557,2147411551,2147411549,2147411533,2147411527,2147411501,2147411477,2147411473,2147411471,2147411429,2147411359,2147411339,2147411333,2147411297,2147411263,2147411251,2147411221,2147411213,2147411209,2147411183,2147411087,2147411033,2147411011,2147411009,2147411003,2147410973,2147410963,2147410949,2147410891,2147410873,2147410849,2147410829,2147410823,2147410817,2147410813,2147410789,2147410781,2147410757,2147410753,2147410729,2147410717,2147410687,2147410679,2147410673,2147410649,2147410637,2147410621,2147410619,2147410597,2147410567,2147410543,2147410537,2147410523,2147410483,2147410481,2147410451,2147410379,2147410373,2147410351,2147410339,2147410333,2147410327,2147410313,2147410297,2147410273,2147410271,2147410253,2147410247,2147410217,2147410163,2147410159,2147410127,2147410051,2147410043,2147410037,2147410007,2147409989,2147409977,2147409967,2147409949,2147409907,2147409899,2147409871,2147409841,2147409799,2147409793,2147409787,2147409751,2147409721,2147409713,2147409707,2147409653,2147409647,2147409631,2147409629,2147409623,2147409619,2147409601,2147409577,2147409547,2147409541,2147409499,2147409493,2147409491,2147409457,2147409443,2147409409,2147409403,2147409389,2147409373,2147409361,2147409353,2147409343,2147409337,2147409323,2147409311,2147409301,2147409287,2147409263,2147409239,2147409217,2147409181,2147409167,2147409163,2147409157,2147409137,2147409113,2147409083,2147409067,2147409049,2147409041,2147409031,2147408981,2147408957,2147408911,2147408909,2147408881,2147408827,2147408779,2147408761,2147408749,2147408741,2147408729,2147408717,2147408713,2147408707,2147408699,2147408629,2147408621,2147408609,2147408591,2147408587,2147408563,2147408551,2147408531,2147408513,2147408507,2147408467,2147408441,2147408387,2147408339,2147408323,2147408321,2147408303,2147408299,2147408293,2147408279,2147408273,2147408267,2147408243,2147408233,2147408209,2147408201,2147408171,2147408143,2147408111,2147408093,2147408083,2147408027,2147408017,2147407991,2147407973,2147407939,2147407907,2147407891,2147407877,2147407861,2147407807,2147407799,2147407793,2147407741,2147407699,2147407697,2147407681,2147407667,2147407657,2147407621,2147407609,2147407543,2147407463,2147407439,2147407429,2147407403,2147407337,2147407333,2147407319,2147407279,2147407271,2147407261,2147407253,2147407193,2147407177,2147407153,2147407133,2147407127,2147407117,2147407069,2147407039,2147407033,2147407027,2147407013,2147407001,2147406997,2147406991,2147406979,2147406953,2147406931,2147406917,2147406869,2147406839,2147406827,2147406823,2147406817,2147406773,2147406769,2147406757,2147406739,2147406721,2147406643,2147406631,2147406623,2147406601,2147406589,2147406553,2147406517,2147406511,2147406497,2147406491,2147406463,2147406451,2147406409,2147406397,2147406379,2147406367,2147406341,2147406319,2147406281,2147406229,2147406223,2147406199,2147406193,2147406167,2147406161,2147406139,2147406101,2147406061,2147406059,2147406047,2147406031,2147405993,2147405987,2147405977,2147405947,2147405921,2147405917,2147405893,2147405801,2147405759,2147405747,2147405693,2147405657,2147405591,2147405573,2147405569,2147405563,2147405531,2147405527,2147405503,2147405471,2147405467,2147405461,2147405417,2147405389,2147405357,2147405353,2147405333,2147405329,2147405291,2147405279,2147405243,2147405237,2147405209,2147405203,2147405173,2147405131,2147405129,2147405119,2147405111,2147405107,2147405063,2147405017,2147405003,2147404991,2147404981,2147404957,2147404943,2147404933,2147404901,2147404891,2147404859,2147404841,2147404829,2147404771,2147404711,2147404639,2147404621,2147404619,2147404601,2147404583,2147404579,2147404577,2147404489,2147404447,2147404439,2147404429,2147404393,2147404381,2147404349,2147404331,2147404327,2147404319,2147404309,2147404247,2147404243,2147404229,2147404211,2147404183,2147404157,2147404153,2147404141,2147404121,2147404099,2147404067,2147404037,2147404031,2147404019,2147404001,2147403983,2147403953,2147403893,2147403889,2147403887,2147403859,2147403857,2147403781,2147403779,2147403737,2147403719,2147403701,2147403673,2147403593,2147403581,2147403563,2147403547,2147403529,2147403509,2147403497,2147403469,2147403457,2147403409,2147403407,2147403403,2147403383,2147403343,2147403341,2147403311,2147403283,2147403217,2147403179,2147403143,2147403121,2147403103,2147403061,2147403007,2147402993,2147402989,2147402977,2147402951,2147402941,2147402927,2147402911,2147402899,2147402869,2147402833,2147402827,2147402791,2147402783,2147402779,2147402771,2147402711,2147402689,2147402687,2147402683,2147402611,2147402603,2147402563,2147402507,2147402489,2147402479,2147402443,2147402437,2147402423,2147402419,2147402417,2147402407,2147402381,2147402377,2147402371,2147402357,2147402329,2147402297,2147402269,2147402267,2147402239,2147402221,2147402203,2147402177,2147402161,2147402119,2147402099,2147402027,2147402009,2147401973,2147401957,2147401951,2147401943,2147401933,2147401891,2147401807,2147401769,2147401759,2147401747,2147401741,2147401721,2147401709,2147401681,2147401667,2147401661,2147401591,2147401573,2147401567,2147401549,2147401513,2147401499,2147401457,2147401447,2147401441,2147401369,2147401343,2147401327,2147401313,2147401307,2147401303,2147401271,2147401253,2147401241,2147401213,2147401181,2147401171,2147401169,2147401163,2147401129,2147401103,2147401099,2147401021,2147400991,2147400977,2147400973,2147400949,2147400943,2147400889,2147400877,2147400847,2147400821,2147400803,2147400799,2147400769,2147400763,2147400743,2147400683,2147400643,2147400637,2147400623,2147400599,2147400583,2147400571,2147400511,2147400481,2147400469,2147400457,2147400449,2147400433,2147400397,2147400377,2147400361,2147400337,2147400331,2147400329,2147400301,2147400293,2147400239,2147400217,2147400139,2147400127,2147400113,2147400089,2147400071,2147400053,2147400011,2147400001,2147399983,2147399981,2147399959,2147399957,2147399927,2147399923,2147399909,2147399851,2147399843,2147399819,2147399809,2147399803,2147399789,2147399777,2147399767,2147399759,2147399753,2147399731,2147399719,2147399711,2147399701,2147399699,2147399689,2147399671,2147399663,2147399629,2147399603,2147399593,2147399587,2147399581,2147399561,2147399533,2147399521,2147399519,2147399509,2147399491,2147399461,2147399447,2147399431,2147399407,2147399383,2147399381,2147399363,2147399329,2147399263,2147399227,2147399203,2147399197,2147399167,2147399161,2147399153,2147399147,2147399141,2147399123,2147399113,2147399087,2147399069,2147399063,2147399053,2147399021,2147399017,2147398997,2147398969,2147398963,2147398949,2147398919,2147398849,2147398819,2147398783,2147398769,2147398681,2147398679,2147398667,2147398597,2147398577,2147398559,2147398553,2147398549,2147398531,2147398529,2147398507,2147398501,2147398387,2147398361,2147398321,2147398313,2147398283,2147398277,2147398261,2147398229,2147398219,2147398207,2147398157,2147398109,2147398091,2147398081,2147398079,2147398021,2147398009,2147397997,2147397953,2147397947,2147397943,2147397883,2147397881,2147397877,2147397859,2147397853,2147397827,2147397821,2147397817,2147397809,2147397787,2147397751,2147397743,2147397731,2147397677,2147397643,2147397589,2147397587,2147397569,2147397563,2147397557,2147397541,2147397479,2147397463,2147397443,2147397437,2147397433,2147397409,2147397383,2147397361,2147397359,2147397353,2147397289,2147397283,2147397281,2147397269,2147397257,2147397209,2147397199,2147397193,2147397137,2147397097,2147397071,2147397029,2147397019,2147397011,2147396989,2147396987,2147396963,2147396921,2147396903,2147396897,2147396893,2147396887,2147396869,2147396857,2147396827,2147396819,2147396807,2147396761,2147396759,2147396749,2147396711,2147396687,2147396659,2147396623,2147396621,2147396609,2147396579,2147396569,2147396561,2147396557,2147396533,2147396521,2147396513,2147396441,2147396413,2147396411,2147396401,2147396399,2147396353,2147396351,2147396341,2147396323,2147396309,2147396281,2147396267,2147396243,2147396227,2147396213,2147396203,2147396189,2147396179,2147396159,2147396129,2147396077,2147396063,2147396057,2147396051,2147396039,2147395969,2147395961,2147395949,2147395937,2147395927,2147395907,2147395891,2147395841,2147395829,2147395777,2147395771,2147395729,2147395721,2147395709,2147395703,2147395697,2147395669,2147395661,2147395651,2147395633,2147395631,2147395609,2147395589,2147395553,2147395499,2147395489,2147395487,2147395427,2147395423,2147395421,2147395417,2147395379,2147395373,2147395343,2147395331,2147395309,2147395303,2147395297,2147395291,2147395259,2147395241,2147395193,2147395147,2147395123,2147395043,2147395039,2147394979,2147394973,2147394961,2147394959,2147394947,2147394917,2147394913,2147394889,2147394869,2147394853,2147394811,2147394761,2147394749,2147394719,2147394631,2147394619,2147394617,2147394589,2147394577,2147394569,2147394553,2147394551,2147394547,2147394539,2147394533,2147394527,2147394479,2147394467,2147394461,2147394427,2147394373,2147394371,2147394341,2147394289,2147394283,2147394281,2147394259,2147394247,2147394239,2147394217,2147394199,2147394187,2147394173,2147394167,2147394131,2147394127,2147394097,2147394089,2147394043,2147394023,2147394017,2147393959,2147393929,2147393921,2147393909,2147393881,2147393867,2147393863,2147393861,2147393767,2147393761,2147393747,2147393701,2147393693,2147393687,2147393683,2147393681,2147393671,2147393659,2147393639,2147393621,2147393609,2147393561,2147393533,2147393489,2147393461,2147393453,2147393449,2147393447,2147393419,2147393401,2147393399,2147393393,2147393377,2147393371,2147393359,2147393351,2147393317,2147393299,2147393291,2147393273,2147393257,2147393249,2147393221,2147393201,2147393179,2147393147,2147393141,2147393099,2147393093,2147393051,2147393009,2147392993,2147392991,2147392981,2147392903,2147392883,2147392879,2147392873,2147392853,2147392843,2147392829,2147392811,2147392789,2147392721,2147392703,2147392697,2147392669,2147392591,2147392571,2147392561,2147392547,2147392543,2147392531,2147392487,2147392469,2147392409,2147392399,2147392391,2147392367,2147392337,2147392301,2147392283,2147392271,2147392253,2147392249,2147392243,2147392201,2147392189,2147392153,2147392151,2147392147,2147392139,2147392129,2147392069,2147392031,2147392021,2147391991,2147391973,2147391931,2147391889,2147391863,2147391859,2147391847,2147391833,2147391811,2147391787,2147391781,2147391773,2147391751,2147391721,2147391713,2147391709,2147391691,2147391683,2147391641,2147391637,2147391629,2147391583,2147391569,2147391563,2147391559,2147391551,2147391511,2147391487,2147391461,2147391409,2147391403,2147391383,2147391353,2147391349,2147391347,2147391331,2147391313,2147391299,2147391283,2147391271,2147391269,2147391247,2147391241,2147391209,2147391203,2147391187,2147391161,2147391121,2147391107,2147391011,2147390977,2147390939,2147390933,2147390923,2147390897,2147390873,2147390827,2147390789,2147390783,2147390779,2147390737,2147390731,2147390671,2147390657,2147390639,2147390629,2147390627,2147390617,2147390611,2147390599,2147390587,2147390563,2147390489,2147390447,2147390437,2147390419,2147390387,2147390363,2147390341,2147390327,2147390291,2147390281,2147390257,2147390249,2147390233,2147390213,2147390209,2147390183,2147390149,2147390117,2147390111,2147390107,2147389991,2147389967,2147389961,2147389957,2147389939,2147389931,2147389919,2147389903,2147389897,2147389891,2147389877,2147389843,2147389819,2147389781,2147389763,2147389753,2147389691,2147389679,2147389631,2147389619,2147389591,2147389579,2147389577,2147389523,2147389501,2147389483,2147389471,2147389441,2147389421,2147389417,2147389403,2147389399,2147389369,2147389351,2147389333,2147389331,2147389327,2147389267,2147389219,2147389213,2147389183,2147389159,2147389151,2147389147,2147389141,2147389121,2147389109,2147389103,2147389067,2147389033,2147389019,2147388979,2147388961,2147388953,2147388923,2147388871,2147388851,2147388829,2147388809,2147388791,2147388773,2147388769,2147388751,2147388731,2147388709,2147388703,2147388701,2147388679,2147388611,2147388577,2147388563,2147388559,2147388497,2147388487,2147388479,2147388437,2147388431,2147388427,2147388413,2147388409,2147388403,2147388389,2147388377,2147388329,2147388323,2147388319,2147388301,2147388253,2147388251,2147388241,2147388239,2147388233,2147388211,2147388209,2147388179,2147388119,2147388107,2147388101,2147388097,2147388083,2147388049,2147388043,2147388041,2147388007,2147387959,2147387947,2147387933,2147387903,2147387897,2147387857,2147387839,2147387821,2147387791,2147387743,2147387741,2147387729,2147387719,2147387713,2147387699,2147387677,2147387651,2147387647,2147387629,2147387609,2147387573,2147387531,2147387507,2147387503,2147387441,2147387413,2147387393,2147387383,2147387339,2147387323,2147387299,2147387279,2147387257,2147387227,2147387219,2147387213,2147387153,2147387131,2147387083,2147387063,2147387059,2147387029,2147387017,2147387009,2147387003,2147386987,2147386921,2147386883,2147386853,2147386757,2147386753,2147386733,2147386729,2147386727,2147386711,2147386699,2147386687,2147386667,2147386639,2147386627,2147386607,2147386601,2147386583,2147386561,2147386529,2147386471,2147386463,2147386459,2147386447,2147386387,2147386303,2147386277,2147386273,2147386271,2147386259,2147386217,2147386211,2147386207,2147386187,2147386169,2147386147,2147386123,2147386117,2147386091,2147386051,2147386049,2147385973,2147385967,2147385941,2147385937,2147385931,2147385907,2147385893,2147385887,2147385857,2147385847,2147385829,2147385827,2147385809,2147385781,2147385769,2147385767,2147385763,2147385733,2147385727,2147385697,2147385677,2147385671,2147385659,2147385637,2147385631,2147385607,2147385601,2147385599,2147385589,2147385587,2147385553,2147385479,2147385473,2147385439,2147385431,2147385389,2147385371,2147385367,2147385337,2147385329,2147385313,2147385293,2147385281,2147385223,2147385211,2147385199,2147385197,2147385157,2147385151,2147385113,2147385103,2147385091,2147385089,2147385077,2147385049,2147385001,2147384977,2147384917,2147384909,2147384893,2147384861,2147384843,2147384839,2147384821,2147384797,2147384753,2147384747,2147384737,2147384711,2147384689,2147384683,2147384669,2147384653,2147384633,2147384621,2147384609,2147384599,2147384593,2147384537,2147384527,2147384509,2147384501,2147384497,2147384413,2147384387,2147384333,2147384329,2147384287,2147384263,2147384251,2147384243,2147384227,2147384221,2147384203,2147384201,2147384191,2147384111,2147384089,2147384027,2147384023,2147384021,2147384017,2147384011,2147384003,2147383999,2147383993,2147383991,2147383957,2147383943,2147383939,2147383873,2147383871,2147383867,2147383831,2147383829,2147383807,2147383789,2147383787,2147383783,2147383741,2147383739,2147383729,2147383709,2147383697,2147383681,2147383649,2147383633,2147383627,2147383573,2147383531,2147383519,2147383507,2147383501,2147383489,2147383481,2147383463,2147383421,2147383379,2147383369,2147383349,2147383327,2147383307,2147383291,2147383283,2147383267,2147383223,2147383193,2147383177,2147383129,2147383123,2147383093,2147383079,2147383019,2147383009,2147383003,2147382997,2147382973,2147382901,2147382899,2147382883,2147382877,2147382871,2147382851,2147382847,2147382827,2147382791,2147382773,2147382767,2147382751,2147382707,2147382697,2147382683,2147382647,2147382637,2147382623,2147382607,2147382599,2147382583,2147382581,2147382551,2147382539,2147382509,2147382469,2147382449,2147382443,2147382439,2147382407,2147382403,2147382383,2147382379,2147382323,2147382301,2147382287,2147382277,2147382253,2147382239,2147382233,2147382221,2147382199,2147382197,2147382179,2147382173,2147382137,2147382043,2147381989,2147381953,2147381927,2147381909,2147381903,2147381897,2147381869,2147381857,2147381851,2147381791,2147381783,2147381701,2147381693,2147381689,2147381669,2147381633,2147381629,2147381627,2147381573,2147381567,2147381527,2147381413,2147381371,2147381363,2147381351,2147381347,2147381323,2147381317,2147381309,2147381279,2147381267,2147381263,2147381251,2147381237,2147381227,2147381221,2147381219,2147381207,2147381153,2147381147,2147381123,2147381113,2147381107,2147381059,2147381029,2147381017,2147380957,2147380919,2147380909,2147380903,2147380883,2147380867,2147380853,2147380849,2147380841,2147380831,2147380811,2147380801,2147380799,2147380787,2147380783,2147380733,2147380727,2147380693,2147380681,2147380673,2147380663,2147380649,2147380639,2147380583,2147380577,2147380553,2147380537,2147380523,2147380451,2147380421,2147380387,2147380379,2147380373,2147380369,2147380357,2147380331,2147380309,2147380273,2147380243,2147380219,2147380189,2147380159,2147380141,2147380129,2147380099,2147380051,2147380033,2147379979,2147379977,2147379937,2147379929,2147379919,2147379917,2147379869,2147379863,2147379859,2147379847,2147379823,2147379811,2147379809,2147379799,2147379763,2147379761,2147379739,2147379721,2147379673,2147379671,2147379631,2147379571,2147379557,2147379541,2147379539,2147379529,2147379503,2147379499,2147379491,2147379473,2147379431,2147379419,2147379359,2147379319,2147379301,2147379197,2147379133,2147379109,2147379097,2147379077,2147378999,2147378983,2147378971,2147378941,2147378903,2147378887,2147378873,2147378843,2147378839,2147378837,2147378833,2147378819,2147378777,2147378767,2147378759,2147378729,2147378713,2147378671,2147378557,2147378539,2147378533,2147378521,2147378501,2147378479,2147378447,2147378411,2147378393,2147378381,2147378377,2147378353,2147378297,2147378269,2147378267,2147378257,2147378251,2147378227,2147378201,2147378131,2147378099,2147378063,2147378011,2147377987,2147377961,2147377951,2147377943,2147377927,2147377879,2147377871,2147377789,2147377787,2147377753,2147377741,2147377699,2147377697,2147377691,2147377679,2147377667,2147377633,2147377607,2147377601,2147377591,2147377567,2147377541,2147377523,2147377489,2147377487,2147377481,2147377469,2147377459,2147377429,2147377381,2147377343,2147377277,2147377273,2147377247,2147377241,2147377231,2147377213,2147377189,2147377153,2147377019,2147377003,2147376989,2147376983,2147376953,2147376929,2147376919,2147376893,2147376887,2147376883,2147376823,2147376817,2147376719,2147376691,2147376631,2147376617,2147376557,2147376551,2147376547,2147376529,2147376509,2147376503,2147376487,2147376463,2147376457,2147376433,2147376419,2147376401,2147376391,2147376379,2147376377,2147376367,2147376323,2147376241,2147376193,2147376151,2147376149,2147376113,2147376089,2147376073,2147376061,2147376059,2147376053,2147376017,2147376001,2147375999,2147375987,2147375969,2147375939,2147375899,2147375869,2147375849,2147375807,2147375773,2147375761,2147375729,2147375693,2147375683,2147375677,2147375669,2147375641,2147375627,2147375597,2147375591,2147375569,2147375567,2147375561,2147375539,2147375521,2147375509,2147375471,2147375437,2147375429,2147375413,2147375411,2147375407,2147375389,2147375371,2147375297,2147375291,2147375257,2147375249,2147375207,2147375201,2147375173,2147375171,2147375161,2147375149,2147375141,2147375137,2147375119,2147375099,2147375089,2147375051,2147374997,2147374993,2147374987,2147374969,2147374951,2147374897,2147374847,2147374841,2147374819,2147374787,2147374741,2147374727,2147374711,2147374673,2147374639,2147374633,2147374597,2147374577,2147374531,2147374519,2147374477,2147374459,2147374337,2147374331,2147374301,2147374297,2147374283
  };
  const unsigned nprimes31=sizeof(primes31)/sizeof(unsigned);

  // Previous Fourier prime
  int prevprimep1p2p3(int p,int maxp,int fourier_for_n){
    if (p==p1+2 || p==p1+1)
      return p1;
    if (p==p1 || p==p1-1 || p==p1-2)
      return p2;
    if (p==p2 || p==p2-1 || p==p2-2)
      return p3;
    if (p==p3 || p==p3-1 || p==p3-2)
      p=fourier_for_n?(p1-2):maxp;
    if (fourier_for_n){
      int l=sizeinbase2(fourier_for_n);
      int pdiv=p>>l;
      for (--pdiv;pdiv>=(1<<(30-l));--pdiv){
	p=(pdiv<<l)+1;
	if (p!=p1 && p!=p2&& p!=p3 && is_probab_prime_p(p))
	  return p;
      }
    }
    p=prevprime(p-2).val;
    if (p==p1 || p==p2 || p==p3)
      p=prevprime(p-2).val;
    return p;
  }

  longlong prevprimell(longlong p,longlong fourier_for_n=0){
    if (fourier_for_n){
      int l=sizeinbase2(fourier_for_n);
      longlong pdiv=p>>l;
      for (--pdiv;pdiv>=(1<<(62-l));--pdiv){
	p=(pdiv<<l)+1;
	if (is_probab_prime_p(p))
	  return p;
      }
    }
    gen g=prevprime(p-2);
    if (g.type==_ZINT)
      return mpz_get_si(*g._ZINTptr);
    return g.val;
  }

  bool gcd_modular_algo(const modpoly &p,const modpoly &q,modpoly &d,modpoly * p_simp,modpoly * q_simp){
    gen tmp;
    int pt=coefftype(p,tmp),qt=coefftype(q,tmp);
    //if (pt!=0 || qt!=0) return false;
    if ( (pt!=_INT_ && pt!=_CPLX)
	 || (qt!=_INT_ && qt!=_CPLX) )
      return false;
    gen gcdfirstcoeff(gcd(p.front(),q.front(),context0));
    int gcddeg= giacmin(int(p.size()),int(q.size()))-1;
    environment env;
    env.modulo=p1+1; env.moduloon=true;
    env.complexe=!vect_is_real(p,context0) || !vect_is_real(q,context0);
    int maxdeg=giacmax(p.size(),q.size())-1;
    int maxp=std::sqrt(p1p2/4./maxdeg);
    gen productmodulo(1);
    modpoly currentgcd,Q,R;
    for (;;){
      env.modulo=prevprimep1p2p3(env.modulo.val,maxp); 
      while (is_zero(p.front() % env.modulo) || is_zero(q.front() % env.modulo)){
	env.modulo=prevprimep1p2p3(env.modulo.val,maxp);
	if (env.complexe){
	  while (smod(env.modulo,4)!=1)
	    env.modulo=prevprimep1p2p3(env.modulo.val,maxp);
	}
      }
      modpoly gcdmod;
      gcdmodpoly(p,q,&env,gcdmod);
      if (is_undef(gcdmod))
	return false;
      gen adjustcoeff=gcdfirstcoeff*invmod(gcdmod.front(),env.modulo);
      mulmodpoly(gcdmod,adjustcoeff,&env,gcdmod);
      int m=int(gcdmod.size())-1;
      if (!m){
	d=makevecteur(1);
	return true;
      }
      if (m>gcddeg) // this prime is bad, just ignore
	continue;
      // combine step
      if (m<gcddeg){ // previous prime was bad
	gcddeg=m;
	currentgcd.swap(gcdmod);
	productmodulo=env.modulo;
      }
      else {
	// m==gcddeg, start combine
	if (productmodulo==gen(1)){ // no need to combine primes
	  currentgcd.swap(gcdmod);
	  productmodulo=env.modulo;
	}
	else {
	  if (productmodulo.type==_INT_)
	    currentgcd=ichinrem(gcdmod,currentgcd,env.modulo,productmodulo);
	  else
	    ichinrem_inplace(currentgcd,gcdmod,productmodulo,env.modulo.val);
	  productmodulo=productmodulo*env.modulo;
	}
      }
      // check candidate gcd
      modpoly dmod(currentgcd);
      if (is_undef(dmod))
	return false;
      ppz(dmod);
      if ( DenseDivRem(p,dmod,Q,R,true) && R.empty()){
	if (p_simp)
	  p_simp->swap(Q);
	if (DenseDivRem(q,dmod,Q,R,true) && R.empty() ){
	  if (q_simp)
	    q_simp->swap(Q);
	  d.swap(dmod);
	  return true;
	}
      }
    }
    return false;
  }

  modpoly gcd(const modpoly & p,const modpoly &q,environment * env,bool call_ntl){
    if (p.empty()) return q;
    if (q.empty()) return p;
    if (!env){
      if (p.front().type==_MOD){ 
	// unmodularize, get env, call gcdmodpoly
	environment e;
	e.modulo=*(p.front()._MODptr+1);
	e.moduloon=true;
	modpoly P,Q,A;
	unmodularize(p,e.modulo,P);
	unmodularize(q,e.modulo,Q);
	if (call_ntl && ntlgcd(P,Q,e.modulo,A))
	  ;
	else
	  gcdmodpoly(P,Q,&e,A);
	modularize(A,e.modulo);
	return A;
      }
      else {
	if (q.front().type==_MOD)
	  return gcd(q,p,env);
      }
    }
    if (!env || !env->moduloon || !is_zero(env->coeff)){
      modpoly g;
      if (call_ntl && ntlgcd(p,q,0,g))
	return g;
      if (gcd_modular_algo(p,q,g,NULL,NULL))
	return g;
      polynome r,s;
      int dim=giacmax(inner_POLYdim(p),inner_POLYdim(q));
      poly12polynome(p,1,r,dim);
      poly12polynome(q,1,s,dim);
      return polynome2poly1(gcd(r,s),1);
    }
    modpoly a;
    gcdmodpoly(p,q,env,a);
    return a;
    // dbgp(a);
    // return a;
  }

  modpoly lcm(const modpoly & p,const modpoly &q,environment * env){
    modpoly g(gcd(p,q,env));
    return operator_times(operator_div(p,g,env),q,env);
  }

  bool algnorme(const polynome & p_y,const polynome & pmini,polynome & n){
    n=resultant(p_y,pmini).trunc1();
    return true;
    matrice S=sylvester(polynome2poly1(pmini,1),polynome2poly1(p_y,1));
    S=mtran(S);
    gen g=det_minor(S,vecteur(0),false,context0);
    if (g.type!=_POLY)
      return false;
    n=*g._POLYptr;
    return true;
  }

#ifdef USE_GMP_REPLACEMENTS
  bool egcd_mpz(const modpoly & a,const modpoly &b,int degstop,const gen & m,modpoly & u,modpoly &v,modpoly & d,modpoly * u_ptr,modpoly * v_ptr,modpoly * r_ptr){
    return false;
  }

#else
  // set B to free mpz copies of a
  bool assign_mpz(const modpoly & a,modpoly &A,int s=128){
    int n=a.size();
    A.reserve(n);
    for (int i=0;i<n;++i){
      gen ai(a[i]);
      if (ai.type==_INT_)
	ai.uncoerce(s);
      else {
	if (ai.type!=_ZINT)
	  return false;
	gen b; b.uncoerce(s);
	mpz_set(*b._ZINTptr,*ai._ZINTptr);
	swapgen(ai,b);
      }
      A.push_back(ai);
    }
    return true;
  }

  void uncoerce(modpoly & R,int s){
    for (int i=0;i<R.size();++i){
      if (R[i].type==_INT_)
	R[i].uncoerce(s);
    }
  }

  // A.size()==B.size()+1, A -= (q1*X+q0)*B mod m
  // A[i] -= q1*B[i]+q0*B[i-1] mod m
  void rem_mpz(modpoly & A,const gen & q1,const gen & q0,const gen & m,modpoly & B,mpz_t & z,int cancel=0){
    int a=A.size()-1,i=1;
    if (cancel) 
      i=cancel;
    else {
      mpz_set(z,*A.front()._ZINTptr);
      mpz_submul(z,*q1._ZINTptr,*B.front()._ZINTptr);
      mpz_tdiv_r(*A.front()._ZINTptr,z,*m._ZINTptr);
      if (mpz_cmp_si(*A.front()._ZINTptr,0)==0)
	cancel=1;
    }
    for (;i<a;++i){
      mpz_set(z,*A[i]._ZINTptr);
      mpz_submul(z,*q1._ZINTptr,*B[i]._ZINTptr);
      mpz_submul(z,*q0._ZINTptr,*B[i-1]._ZINTptr);
      mpz_tdiv_r(*A[i]._ZINTptr,z,*m._ZINTptr);
      if (cancel==i && mpz_cmp_si(*A[i]._ZINTptr,0)==0)
	++cancel;
    }
    mpz_set(z,*A.back()._ZINTptr);
    mpz_submul(z,*q0._ZINTptr,*B.back()._ZINTptr);
    mpz_tdiv_r(*A.back()._ZINTptr,z,*m._ZINTptr);
    if (cancel)
      A.erase(A.begin(),A.begin()+cancel);
  }

  bool egcd_mpz(const modpoly & a,const modpoly &b,int degstop,const gen & m,modpoly & u,modpoly &v,modpoly & d,modpoly * u_ptr,modpoly * v_ptr,modpoly * r_ptr){
    if (m.type!=_ZINT)
      return false;
    environment env;
    env.modulo=m;
    env.moduloon=true;
    int s=mpz_sizeinbase(*m._ZINTptr,2)+1;
    modpoly A,B,U0(1,1),U1,Q,R;
    U0[0].uncoerce(s);
    assign_mpz(a,A,s);
    assign_mpz(b,B,s);
    bool swapped=A.size()<B.size();
    if (swapped)
      A.swap(B);
    gen q1,q0; q1.uncoerce(s); q0.uncoerce(s);
    gen Z; Z.uncoerce(2*s);
    mpz_t & z=*Z._ZINTptr; 
    int bs,niter=0;
    for (;(bs=B.size())-1>=degstop;++niter){
      // gen B0=invmod(B.front(),m); B0.uncoerce(s);
      mpz_invert(*q0._ZINTptr,*B.front()._ZINTptr,*m._ZINTptr);
      mpz_mul(z,*A.front()._ZINTptr,*q0._ZINTptr); // B0 stored in q0
      mpz_tdiv_r(*q1._ZINTptr,z,*m._ZINTptr); // A.front()*B0 modulo m
      if (A.size()==bs){ // may happen only at first iteration
	// quotient is a constant, A0*B0, replace A by A-quotient*B
	int cancel=1;
	for (int i=1;i<bs;++i){
	  mpz_set(z,*A[i]._ZINTptr);
	  mpz_submul(z,*q1._ZINTptr,*B[i]._ZINTptr);
	  mpz_tdiv_r(*A[i]._ZINTptr,z,*m._ZINTptr);
	  if (i==cancel && mpz_cmp_si(*A[i]._ZINTptr,0)==0)
	    ++cancel;
	}
	// no change for U0 since U1 is 0 at 1st iteration
	A.erase(A.begin(),A.begin()+cancel);
	if (A.empty())
	  break;
      }
      else {
	if (A.size()==bs+1){ // generic iteration, compute quotient=q1*X+q0
	  mpz_set(z,*A[1]._ZINTptr);
	  mpz_submul(z,*q1._ZINTptr,*B[1]._ZINTptr);
	  mpz_mul(z,z,*q0._ZINTptr); // B0 stored in q0
	  mpz_tdiv_r(*q0._ZINTptr,z,*m._ZINTptr); // (A[1]-q1*B[1])*B0 modulo m
	  // A -= (q1*X+q0)*B (2 leading terms cancel, maybe more)
	  rem_mpz(A,q1,q0,m,B,z,2);
	  if (A.empty())
	    break;
	  // U0=U0-(q1*X+q0)*U1 (U1.size()>U0.size(), no leading term cancel)
	  if (!U1.empty()){
	    U0.insert(U0.begin(),2,0);
	    U0[0].uncoerce(s);
	    U0[1].uncoerce(s);
	    if (U0.size()==2 && U1.size()==1 && mpz_cmp_si(*U1.front()._ZINTptr,1)==0){
	      mpz_neg(*U0[0]._ZINTptr,*q1._ZINTptr);
	      mpz_neg(*U0[1]._ZINTptr,*q0._ZINTptr);
	    }
	    else {
	      rem_mpz(U0,q1,q0,m,U1,z,0);
	    }
	  }
	}
	else {
	  // call divrem
	  DivRem(A,B,&env,Q,R,false);
	  uncoerce(R,s);
	  A.swap(R);
	  if (A.empty()) 
	    break; // B is the gcd
	  // U0=U0-Q*U1
	  operator_times(Q,U1,&env,R); submodpoly(U0,R,&env,U0); // ur=ua-q*ub;
	  uncoerce(U0,s);
	}
      }
      // next iteration A is the remainder (non zero) and U0 the coeff, swap
      A.swap(B);
      U0.swap(U1);
    }
    if (niter==0){
      if (swapped){
	u=vecteur(1,1);
	v=vecteur(0);
	d=a;
	if (r_ptr)
	  *r_ptr=b;
      }
      else {
	u=vecteur(0);
	v=vecteur(1,1);
	d=b;
	if (r_ptr)
	  *r_ptr=a;
      }
      if (u_ptr){
	*u_ptr=v;
	*v_ptr=u;
      }
      return true;
    }
    // B is the gcd, U1 is the coeff of a unless swapped is true
    trim_inplace(B,&env);
    d.swap(B);
    trim_inplace(U1,&env);
    u.swap(U1);
    if (r_ptr){
      trim_inplace(A,&env);
      r_ptr->swap(A);
    }
    if (u_ptr){
      trim_inplace(U0,&env);
      u_ptr->swap(U0);
    }
#if 0
    q1=invmod(d.front(),m);
    mulmodpoly(u,q1,&env,u);
    mulmodpoly(d,q1,&env,d);
#endif
    modpoly tmp1,tmp2;
    operator_times(u,swapped?b:a,&env,tmp1);
    // next step is not required because degree(d)<degree(smallest of a and b)
    // since we made at least 1 iteration without breaking
    if (0 && (d.size()==swapped?a.size():b.size())){
      submodpoly(tmp1,d,&env,tmp2); tmp1.swap(tmp2); // tmp1=u*a-d
    }
    DivRem(tmp1,swapped?a:b,&env,v,R); // R would be 0 if step above was taken, DivQuo might be called if degree are large, but egcd_mpz should not be called...
    negmodpoly(v,v);
    if (swapped)
      u.swap(v);
    if (u_ptr && v_ptr){
      if (niter==1){
	// breaked at 2nd iteration, at 1st iteration we have u=0 and v=1
	u_ptr->clear();
	*v_ptr=vecteur(1,1);
      }
      else {
	operator_times(*u_ptr,swapped?b:a,&env,tmp1);
	DivRem(tmp1,swapped?a:b,&env,*v_ptr,R); 
	negmodpoly(*v_ptr,*v_ptr);
      }
      if (swapped)
	u_ptr->swap(*v_ptr);
    }
    return true;
  }
#endif // USE_GMP_REPLACEMENTS

  // returns [[A,B],[C,D]] and d such that [[A,B],[C,D]]*[a,b]=[d,0]
  bool half_egcd(const modpoly &a,const modpoly &b,const gen & modulo,modpoly &A,modpoly &B,modpoly &C,modpoly &D,modpoly & d){
    if (a.size()<b.size())
      return half_egcd(b,a,modulo,B,A,D,C,d);
    environment env;
    env.modulo=modulo;
    env.moduloon=true;
    modpoly q,r,tmp1;
    if (a.size()==b.size()){ // requires an additional euclidean division step
      DivRem(a,b,&env,q,r);
      if (!half_egcd(b,r,modulo,A,B,C,D,d))
	return false;
      // [[A,B],[C,D]]*[b,r]=[d,0], where r=a-b*q
      // A*b+B*(a-b*q)=d i.e. B*a+(A-q*B)*b=d
      // C*b+D*(a-b*q)=0 i.e. D*a+(C-q*D)*b=0
      a_bc(A,B,q,&env,A,tmp1);
      a_bc(C,D,q,&env,C,tmp1);
      // now [[B,A],[D,C]]*[a,b]=[d,0]
      A.swap(B); C.swap(D);
      return true;
    }
    modpoly RA,RB,RC,RD,tmp0,tmp2;
    if (!hgcd(a,b,modulo,RA,RB,RC,RD,d,tmp0,tmp1,tmp2))
      return false;
    int maxadeg=a.size()+1-giacmax(RA.size(),RB.size());
    matrix22timesvect(RA,RB,RC,RD,a,b,maxadeg,maxadeg,d,tmp0,env,tmp1,tmp2);
    if (tmp0.empty()){
      A.swap(RA); B.swap(RB); C.swap(RC); D.swap(RD);
      return true;
    }
    modpoly & P2=d; modpoly & P3=tmp0; modpoly & P4=r; // Yap notations 
    // [[RA,RB],[RC,RD]]*[a,b]=[P2,P3]
    DivRem(P2,P3,&env,q,P4); // P4=P2-q*P3=RA*a+RB*b-q*(RC*a+RD*b)
    // [[0,1],[1,-q]]*[[RA,RB],[RC,RD]] == [[RC,RD],[-RC*q+RA,-RD*q+RB]]
    a_bc(RA,RC,q,&env,RA,tmp1);
    a_bc(RB,RD,q,&env,RB,tmp1); // [[RC,RD],[RA,RB]]*[a,b]=[P3,P4]
    if (P4.empty()){
      A.swap(RC); B.swap(RD); C.swap(RA); D.swap(RB); d.swap(tmp0); return true;
    }
    modpoly SA,SB,SC,SD;
    if (!half_egcd(P3,P4,modulo,SA,SB,SC,SD,d))
      return false;
    matrix22(RA,RB,RC,RD,SA,SB,SC,SD,A,B,C,D,env,tmp1,tmp2);
    return true;
  }

  void neg(vector<int> & v){
    vector<int>::iterator it=v.begin(),itend=v.end();
    for (;it!=itend;++it)
      *it=-*it;
  }
  
  int adjust_resultant(int &res, const vector<int> & coeffv,const vector<int> & degv,int m){
    // adjust
    for (int i=0;i<coeffv.size();++i){
      if (degv[i]%2==1 && degv[i+1]%2==1)
	res=-res;
      res=(longlong(res)*powmod(coeffv[i],degv[i]-degv[i+2],m))%m;
    }
    return res;
  }

#define EGCD_INT 1
#ifdef EGCD_INT
  // for a and b co-prime mod p
  // returns [[A,B],[C,D]] and d such that [[A,B],[C,D]]*[a,b]=[d,0]
  bool in_egcd_int(const vector<int> &a,const vector<int> &b,int p,vector<int> &A,vector<int> &B,vector<int> &C,vector<int> &D,vector<int> & coeffv,vector<int> & degv,int & d){
    d=1;
    vector<int> RA,RB,RC,RD,tmp0,tmp1,tmp2,tmp3,tmp4,tmp5,Wp;
    if (b.size()<HGCD){
      hgcd_iter_int(a,b,0,A,C,B,D,p,coeffv,degv,tmp0,tmp1,tmp2,tmp3,tmp4,tmp5);
      if (tmp0.size()>1)
	d=0;
      else {
	d=invmod(tmp0[0],p);
	degv.push_back(0);
	mulmod(A,d,p);
	mulmod(B,d,p);
      }
      return true;
    }
    if (!hgcdint(a,b,p,Wp,RA,RB,RC,RD,coeffv,degv,tmp0,tmp1,tmp2,tmp3,tmp4,tmp5))
      return false;
    int maxadeg=a.size()+1-giacmax(RA.size(),RB.size());
    matrix22inttimesvect(RA,RB,RC,RD,a,b,maxadeg,maxadeg,tmp0,tmp1,p,tmp2,tmp3,tmp4,tmp5,Wp);
    if (tmp1.empty()){
      A.swap(RA); B.swap(RB); C.swap(RC); D.swap(RD);
      return true;
    }
    vector<int> & P2=tmp0,& P3=tmp1,&q=tmp2,&r=tmp3; // Yap notations 
    degv.push_back(degv.back()+P3.size()-P2.size());
    coeffv.push_back(P3[0]);
    // [[RA,RB],[RC,RD]]*[a,b]=[P2,P3]
    DivRem(P2,P3,p,q,r); // P4=P2-q*P3=RA*a+RB*b-q*(RC*a+RD*b)
    // [[0,1],[1,-q]]*[[RA,RB],[RC,RD]] == [[RC,RD],[-RC*q+RA,-RD*q+RB]]
    a_bc(RA,RC,q,p,RA,tmp4);
    a_bc(RB,RD,q,p,RB,tmp4); // [[RC,RD],[RA,RB]]*[a,b]=[P3,P4]
    if (r.empty()){
      A.swap(RC); B.swap(RD); C.swap(RA); D.swap(RB); 
      return true;
    }
    vector<int> SA,SB,SC,SD;
    if (!in_egcd_int(P3,r,p,SA,SB,SC,SD,coeffv,degv,d))
      return false;
    matrix22int(RA,RB,RC,RD,SA,SB,SC,SD,A,B,C,D,p,tmp0,Wp);
    return true;
  }

  bool egcd_int(const vector<int> &a,const vector<int> &b,int p,vector<int> &A,vector<int> &B,vector<int> &C,vector<int> &D,int & d){
    if (a.size()<b.size()){
      bool res=egcd_int(b,a,p,B,A,D,C,d);
      if (a.size()%2==0 && b.size()%2==0){
	d=-d;
	neg(A); neg(B);
      }
      return res;
    }
    vector<int> q,r;
    if (a.size()==b.size()){ // requires an additional euclidean division step
      DivRem(a,b,p,q,r);
      if (!egcd_int(b,r,p,A,B,C,D,d))
	return false;
      // [[A,B],[C,D]]*[b,r]=[d,0], where r=a-b*q
      // A*b+B*(a-b*q)=d i.e. B*a+(A-q*B)*b=d
      // C*b+D*(a-b*q)=0 i.e. D*a+(C-q*D)*b=0
      a_bc(A,B,q,p,A,r);
      a_bc(C,D,q,p,C,r);
      // now [[B,A],[D,C]]*[a,b]=[d,0]
      A.swap(B); C.swap(D);
      d=(d*longlong(powmod(b[0],a.size()-r.size(),p)))%p;
      if (a.size()%2==0){
	d=-d;
	neg(A); neg(B);
      }
      return true;
    }
    vector<int> coeffv,degv;
    coeffv.reserve(b.size()+1);
    degv.reserve(b.size()+2);
    degv.push_back(a.size());
    if (!in_egcd_int(a,b,p,A,B,C,D,coeffv,degv,d))
      return false;
    adjust_resultant(d,coeffv,degv,p);
    mulmod(A,d,p);
    mulmod(B,d,p);
    return true;
  }

#endif
  
  // modular extended Euclide algorithm with rational reconstruction
  // this would become faster only for very large degrees
  bool egcd_z(const modpoly &a, const modpoly & b, modpoly & u,modpoly & v,modpoly & d,bool deterministic){
    d=gcd(a,b,0);
    if (d.size()>1){
      modpoly D;
      bool bb=egcd_z(a/d,b/d,u,v,D,deterministic);
      if (!bb) return false;
      u=u*d;
      v=v*d;
      d=D*d;
      return true;
    }
    if (a.size()>=NTL_XGCD && b.size()>=NTL_XGCD && ntlxgcd(a,b,0,u,v,d))
      return true;
    if (a.size()<HGCD || b.size()<HGCD)
      return false;
    environment env;
    env.moduloon=true;
    env.modulo=p1+1; gen pip=1;
    int gcddeg=giacmin(a.size(),b.size())-1;
    int maxdeg=giacmax(a.size(),b.size())-1;
    int maxp=std::sqrt(p1p2/4./maxdeg),iter;
    modpoly urec,vrec,drec,ucur,vcur,dcur;
    gen borne=pow(norm(a,context0),(int)b.size()-1)*pow(norm(b,context0),(int)a.size()-1);
    borne=2*pow(b.size(),a.size()-1)*pow(a.size(),b.size()-1)*borne*borne;
#if 1 // compute resultant first
#ifdef EGCD_INT
    gen R;
    vector<int> ai,bi,A,B,C,D; int di;
#else
    gen R=mod_resultant(a,b,0.0); // deterministic?0.0:1e-80); // deterministic
#endif
    bool stable=false; 
    int mem_reserve=0;//(sizeinbase2(borne)+1)/2+64;
    for (iter=0;is_greater(borne,pip*pip,context0);++iter){
      env.modulo=prevprimep1p2p3(env.modulo.val,maxp,maxdeg);
      while (is_zero(a.front() % env.modulo) || is_zero(b.front() % env.modulo))
	env.modulo=prevprimep1p2p3(env.modulo.val,maxp,maxdeg);
      int p=env.modulo.val;
#ifdef EGCD_INT
      vecteur2vector_int(a,p,ai);
      vecteur2vector_int(b,p,bi);
      egcd_int(ai,bi,p,A,B,C,D,di);
      if (di==0)
	continue;
      if (urec.empty()){
	vector_int2vecteur(A,urec);
	vector_int2vecteur(B,vrec);
	R=di;
	pip=env.modulo;
	continue;
      }
      if (pip.type==_INT_){
	ichinrem_inplace(urec,A,pip,p,mem_reserve);
	ichinrem_inplace(vrec,B,pip,p,mem_reserve);
	R=ichinrem(R,di,pip,p);
      }
      else {
	bool b0,b1,b2;
	if (!(b0=chk_equal_mod(R,di,p)))
	  R=ichinrem(R,di,pip,p);
	if (!(b1=chk_equal_mod(urec,A,p)))
	  ichinrem_inplace(urec,A,pip,p);
	if (!(b2=chk_equal_mod(vrec,B,p)))
	  ichinrem_inplace(vrec,B,pip,p);
	if (b0 && b1 && b2 && !deterministic){
	  if (stable)
	    break;
	  stable=true; // make a last run for more confidence
	}
	else
	  stable=false;
      }
      pip=pip*env.modulo;
#else
      egcd(a,b,&env,ucur,vcur,dcur);
      int m=dcur.size();
      if (m>gcddeg)
	continue;
      int r=R.type==_ZINT?modulo(*R._ZINTptr,env.modulo.val):R.val;
      r=(longlong(r)*invmod(dcur[0].val,env.modulo.val))%env.modulo.val;
      mulmodpoly(ucur,gen(r),&env,ucur);
      mulmodpoly(vcur,gen(r),&env,vcur);
      if (m<gcddeg || pip==1){ // 1st run or previous primes were bad
	gcddeg=m;
	pip=env.modulo;
	urec.swap(ucur);
	vrec.swap(vcur);
      }
      else { 
	// chinese remainder
	if (pip.type==_INT_){
	  urec=ichinrem(urec,ucur,pip,env.modulo);
	  vrec=ichinrem(vrec,vcur,pip,env.modulo);
	}
	else {
	  bool b1,b2;
	  if (!(b1=chk_equal_mod(urec,ucur,env.modulo.val)))
	    ichinrem_inplace(urec,ucur,pip,env.modulo.val);
	  if (!(b2=chk_equal_mod(vrec,vcur,env.modulo.val)))
	    ichinrem_inplace(vrec,vcur,pip,env.modulo.val);
	  if (b1 && b2 && !deterministic){
	    if (stable)
	      break;
	    stable=true; // make a last run for more confidence
	  }
	  else
	    stable=false;
	}
	pip=pip*env.modulo;
      }
#endif
    }
    if (debug_infolevel)
      CERR << CLOCK()*1e-6 << " #primes used " << iter << "\n";
    u.swap(urec);
    v.swap(vrec);
    d=makevecteur(R);
    return true;
#else
    // computing u,v and d simultaneously could be 2* efficient
    // if dcur is the resultant of ucur and vcur
    // test would be borne>=pip and no fractional reconstruction
    for (iter=0;is_greater(borne,pip,context0);++iter){
      env.modulo=prevprimep1p2p3(env.modulo.val,maxp,maxdeg);
      while (is_zero(a.front() % env.modulo) || is_zero(b.front() % env.modulo))
	env.modulo=prevprimep1p2p3(env.modulo.val,maxp,maxdeg);
      egcd(a,b,&env,ucur,vcur,dcur);
      int m=dcur.size();
      if (m>gcddeg)
	continue;
      if (m<gcddeg || pip==1){ // 1st run or previous primes were bad
	gcddeg=m;
	pip=env.modulo;
	urec.swap(ucur);
	vrec.swap(vcur);
	drec.swap(dcur);
      }
      else { 
	// chinese remainder
	if (pip.type==_INT_){
	  urec=ichinrem(urec,ucur,pip,env.modulo);
	  vrec=ichinrem(vrec,vcur,pip,env.modulo);
	  drec=ichinrem(drec,dcur,pip,env.modulo);
	}
	else {
	  ichinrem_inplace(urec,ucur,pip,env.modulo.val);
	  ichinrem_inplace(vrec,vcur,pip,env.modulo.val);
	  ichinrem_inplace(drec,dcur,pip,env.modulo.val);
	}
	pip=pip*env.modulo;
      }
    }
    // rational reconstruction
    if (debug_infolevel)
      CERR << CLOCK()*1e-6 << " fracmod begin\n" ;
    gen den(drec);
    d=fracmod(drec,pip,&den);
    u=fracmod(urec,pip,&den);
    v=fracmod(vrec,pip,&den);
    mulmodpoly(d,den,d);
    mulmodpoly(u,den,u);
    mulmodpoly(v,den,v);
    if (debug_infolevel)
      CERR << CLOCK()*1e-6 << " fracmod end, #primes used " << iter << "\n";
    return true;
#endif
  }

  // p1*u+p2*v=d
  void egcd(const modpoly &p1, const modpoly & p2, environment * env,modpoly & u,modpoly & v,modpoly & d,bool deterministic){
#if 1
    if (!p1.empty() && !p2.empty() &&
	(!env || !env->moduloon)){
      bool p1mod=p1.front().type==_MOD,p2mod=p1.front().type==_MOD;
      if (p1mod || p2mod){
	environment e;
	e.modulo=*((p1mod?p1:p2).front()._MODptr+1);
	e.moduloon=true;
	egcd(unmod(p1,e.modulo),unmod(p2,e.modulo),&e,u,v,d);
	modularize(u,e.modulo);
	modularize(v,e.modulo);
	modularize(d,e.modulo);
	return;
      }
    }
    if (env && env->moduloon){
      modpoly C,D;
      if (p1.size()>=HGCD && p2.size()>=HGCD && half_egcd(p1,p2,env->modulo,u,v,C,D,d)){
	if (!is_one(d.front())){
	  gen d0=invmod(d.front(),env->modulo);
	  mulmodpoly(u,d0,env,u);
	  mulmodpoly(v,d0,env,v);
	  mulmodpoly(d,d0,env,d);
	}
	return;
      }
      if (egcd_mpz(p1,p2,1,env->modulo,u,v,d,0,0,0))
	return;
    }
#endif
    if ( (!env || !env->moduloon || !is_zero(env->coeff))){
      gen p1g,p2g;
      int p1t=coefftype(p1,p1g);
      int p2t=coefftype(p2,p2g);
      if (p1t==0 && p2t==0 && egcd_z(p1,p2,u,v,d,deterministic))
	return;
      int dim=giacmax(inner_POLYdim(p1),inner_POLYdim(p2));
      polynome pp1(dim),pp2(dim),pu(dim),pv(dim),pd(dim);
      gen den1(1),den2(1);
      poly12polynome(p1,1,pp1,dim);
      lcmdeno(pp1,den1);
      if (!is_one(pp1)) pp1=den1*pp1;
      poly12polynome(p2,1,pp2,dim);
      lcmdeno(pp2,den2);
      if (!is_one(pp2)) pp2=den2*pp2;
      if (p1t==0 && p2t==0 
	  && p1.size()>=GIAC_PADIC/2 && p2.size()>=GIAC_PADIC/2
	  ){
	polynome2poly1(gcd(pp1,pp2),1,d);
	if (d.size()==1){
	  // solve sylvester matrix * []=d
	  matrice S=sylvester(p1,p2);
	  S=mtran(S);
	  int add=int(p1.size()+p2.size()-d.size()-2);
	  v=mergevecteur(vecteur(add,0),d);
	  u=linsolve(S,v,context0);
	  gen D;
	  lcmdeno(u,D,context0);
	  d=multvecteur(D,d);
	  v=vecteur(u.begin()+p2.size()-1,u.end());
	  u=vecteur(u.begin(),u.begin()+p2.size()-1);
	  if (!is_one(den1))
	    u=den1*u;		
	  if (!is_one(den2))
	    v=den2*v;
	  return;
	}
      }
      if (p1t==_EXT && p2t==_EXT && p1g.type==_EXT && p2g.type==_EXT && *(p1g._EXTptr+1)==*(p2g._EXTptr+1) && (p1g._EXTptr+1)->type==_VECT){
	polynome2poly1(gcd(pp1,pp2),1,d);
	if (d.size()==1){
	  polynome P1,P2;
	  if (algext_convert(pp1,p1g,P1) && algext_convert(pp2,p1g,P2)){
	    polynome pmini(P1.dim),P1n(P1.dim-1),P2n(P1.dim-1);
	    algext_vmin2pmin(*(p1g._EXTptr+1)->_VECTptr,pmini);
	    if (algnorme(P1,pmini,P1n) && algnorme(P2,pmini,P2n) ){
	      // first solve norme(p1)*un+norme(p2)*vn=d
	      // then norme(p1)/p1*un*p1+norme(p2)/p2*vn*p2=d
	      // hence u=norme(p1)/p1*un and v=norme(p2)/p2*vn
	      int p1t=coefftype(P1n,p1g);
	      int p2t=coefftype(P2n,p2g);
	      polynome P12g=gcd(P1n,P2n);
	      if (p1t==0 && p2t==0 && P12g.lexsorted_degree()==0){
		//CERR << P1n % pp1 << '\n';
		//CERR << P2n % pp2 << '\n';
		P1=P1n/pp1;
		P2=P2n/pp2;
		// solve sylvester matrix * []=d
		matrice S=sylvester(polynome2poly1(P1n,1),polynome2poly1(P2n,1));
		S=mtran(S);
		v=vecteur(S.size());
		v[S.size()-1]=d[0];
		u=linsolve(S,v,context0);
		gen D;
		lcmdeno(u,D,context0);
		d=multvecteur(D,d);
		int p2s=P2n.lexsorted_degree();
		v=vecteur(u.begin()+p2s,u.end());
		v=operator_times(v,polynome2poly1(P2,1),0);
		v=operator_mod(v,p1,0);
		u=vecteur(u.begin(),u.begin()+p2s);
		u=operator_times(u,polynome2poly1(P1,1),0);
		u=operator_mod(u,p2,0);
		if (!is_one(den1))
		  u=den1*u;		
		if (!is_one(den2))
		  v=den2*v;
		//CERR << (operator_times(u,p1,0)+operator_times(v,p2,0))/D << '\n';
		return;
	      }
	    }
	  }
	}
      }
      if (0 && p1t==_EXT && p2t==0 && p1g.type==_EXT && (p1g._EXTptr+1)->type==_VECT){
	polynome2poly1(gcd(pp1,pp2),1,d);
	if (d.size()==1){
	  polynome P1;
	  if (algext_convert(pp1,p1g,P1)){
	    polynome pmini(P1.dim),P1n(P1.dim-1);
	    algext_vmin2pmin(*(p1g._EXTptr+1)->_VECTptr,pmini);
	    if (algnorme(P1,pmini,P1n)){
	      // first solve norme(p1)*un+p2*v=d
	      // then norme(p1)/p1*un*p1+v*p2=d
	      // hence u=norme(p1)/p1*un 
	      int p1t=coefftype(P1n,p1g);
	      if (p1t==0){
		P1=P1n/pp1;
		// solve sylvester matrix * []=d
		matrice S=sylvester(polynome2poly1(P1n,1),p2);
		S=mtran(S);
		v=vecteur(S.size());
		v[S.size()-1]=d[0];
		u=linsolve(S,v,context0);
		gen D;
		lcmdeno(u,D,context0);
		d=multvecteur(D,d);
		int p2s=int(p2.size()-1);
		v=vecteur(u.begin()+p2s,u.end());
		u=vecteur(u.begin(),u.begin()+p2s);
		u=operator_times(u,polynome2poly1(P1,1),0);
		if (!is_one(den1))
		  u=den1*u;		
		if (!is_one(den2))
		  v=den2*v;
		//CERR << (operator_times(u,p1,0)+operator_times(v,p2,0))/D << '\n';
		return;
	      }
	    }
	  }
	}
      }
      egcd(pp1,pp2,pu,pv,pd);
      polynome2poly1(pu,1,u);
      polynome2poly1(pv,1,v);
      polynome2poly1(pd,1,d);
      if (is_minus_one(d)){
	d=-d; u=-u; v=-v;
      }
      if (!is_one(den1))
	u=den1*u;		
      if (!is_one(den2))
	v=den2*v;
      return;
    } // end if modular env does not apply
    if (p2.empty()){
      u=one();
      v.clear();
      d=p1;
      return ;
    }
    if (p1.empty()){
      v=one();
      u.clear();
      d=p2;
      return ;
    }
    modpoly a,b,q,r,tmp;
    bool swapped=false;
    // change feb 2017, add p2.size()==1 check because I prefer u!=0 if p1 and p2 are csts (and this is required in polynomial Smith normal form)
    if (p1.size()<p2.size() || p1.size()==1)
      swapped=true;
    // initializes a and b to p1, p2
    if (swapped){
      a=p2;
      b=p1;
    }
    else {
      a=p1;
      b=p2;
    }
    // initializes ua to 1 and ub to 0, the coeff of u in ua*a+va*b=a
    modpoly ua(one()),ub,ur;
    // TDivRem: a = bq+r 
    // hence ur <- ua-q*ub verifies
    // ur*a+vr*b=r
    // a <- b, b <- r, ua <- ub and ub<- ur
    for (;;){
      int n=int(b.size());
      if (n==1){ // b is cst !=0 hence is the gcd, ub is valid
	break;
      }
      DivRem(a,b,env,q,r); // division works always
      // if r is 0 then b is the gcd and ub the coeff
      if (r.empty())
	break;
      operator_times(q,ub,env,tmp); submodpoly(ua,tmp,env,ur); // ur=ua-q*ub;
      swap(a,b); swap(b,r); // a=b; b=r;
      swap(ua,ub); swap(ub,ur); // ua=ub; ub=ur;
    }
    // ub is valid and b is the gcd, vb=(b-ub*p1)/p2 if not swapped
    gen s=invmod(b.front(),env->modulo);
    mulmodpoly(b,s,env,d); // d=b*s;
    if (swapped){
      mulmodpoly(ub,s,env,v);
      // COUT << ub << "*" << s << "=" << v << '\n';
      // COUT << "swapped" << d << "-" << v << "*" << p2 << "/" << p1 << '\n';
      u=operator_div(operator_minus(d,operator_times(v,p2,env),env),p1,env);
    }
    else {
      mulmodpoly(ub,s,env,u);
      // COUT << d << "-" << u << "*" << p1 << "/" << p2 << '\n';
      v=operator_div(operator_minus(d,operator_times(u,p1,env),env),p2,env);
    }
    // COUT << "Verif " << p1 << "*" << u << "+" << p2 << "*" << v << "=" << p1*u+p2*v << " " << d << '\n';
  }

  // Solve a=b*x modulo the polynomial n 
  // with degree(a)<l and degree(b)<=degree(n)-l 
  // Assume degree(x)<degree(n)
  bool egcd_pade(const modpoly & n,const modpoly & x,int l,modpoly & a,modpoly &b,environment * env,bool psron){
    l=absint(l);
    if (!env && !x.empty() && x[0].type==_MOD){
      gen p=*(x[0]._MODptr+1);
      modpoly X(unmod(x,p));
      environment e; e.modulo=p; e.moduloon=true;
      if (!egcd_pade(n,X,l,a,b,&e,false)) 
	return false;
      a=*makemod(a,p)._VECTptr;
      b=*makemod(b,p)._VECTptr;
      return true;
    }
    if (n.size()==x.size()+1 && n.size()==2*l+1){
      if (env){
	modpoly A,B,C,D,tmp1,tmp2;
	a.clear();
	if (hgcd(n,x,env->modulo,A,B,C,D,a,b,tmp1,tmp2)){
	  if (a.empty()){
	    mulmodpoly(n,C,env,tmp1);
	    mulmodpoly(x,D,env,tmp2);
	    addmodpoly(tmp1,tmp2,env,a);
	    b.swap(D);
	    return true;
	  }
	}
      }
      // multimodular algorithm for integers, currently not faster...
      if (is_integer_vecteur(n) && is_integer_vecteur(x)){
	vector<int> N,X,A,B,C,D,tmp0,tmp1,tmp2,tmp3,Wp,coeffv,degv,q,f;
	vecteur aa,bb; gen chka1,chka2,chkb1;
	gen p(1<<30),pip(1);
	int stable=0;
	for (;p.val>>29;){
	  p=prevprime(p-1);
	  if (debug_infolevel) 
	    CERR << CLOCK()*1e-6 << " begin egcd mod " << p << '\n';
	  vecteur2vector_int(n,p.val,N);
	  vecteur2vector_int(x,p.val,X);
	  if (debug_infolevel) 
	    CERR << CLOCK()*1e-6 << " hgcd\n";
	  if (!hgcdint(N,X,p.val,Wp,A,B,C,D,coeffv,degv,q,f,tmp0,tmp1,tmp2,tmp3))
	    break;
	  if (debug_infolevel) 
	    CERR << CLOCK()*1e-6 << " hgcd end mod\n";
	  operator_times(N,C,p.val,tmp0);
	  operator_times(X,D,p.val,tmp1);
	  addmod(tmp0,tmp1,p.val); // tmp0=D*X mod N
	  int D0=invmod(D[0],p.val);
	  mulmod(D,D0,p.val);
	  mulmod(tmp0,D0,p.val);
	  gen den(1); int pos=N.size()/3;
	  if (aa.empty()){
	    vector_int2vecteur(tmp0,aa);
	    vector_int2vecteur(D,bb);
	  }
	  else {
	    bool chk=chk_equal_mod(chka2,tmp0.back(),p.val) 
	      // && chk_equal_mod(chka1,tmp0[pos],p.val) 
	      && chk_equal_mod(chkb1,D.back(),p.val);
	    if (chk){
	      if (stable<3)
		++stable;
	      else {
		if (debug_infolevel) 
		  CERR << CLOCK()*1e-6 << " final fracmod\n";
		a=fracmod(aa,pip,&den);
		b=fracmod(bb,pip,&den);
		if (debug_infolevel) 
		  CERR << CLOCK()*1e-6 << " end fracmod\n";
		if (chk_equal_mod(a,tmp0,p.val) && chk_equal_mod(b,D,p.val))
		  return true;
	      }
	    }
	    else
	      stable=0;
	    if (debug_infolevel) 
	      CERR << CLOCK()*1e-6 << " ichinrem\n";
	    ichinrem_inplace(aa,tmp0,pip,p.val);
	    ichinrem_inplace(bb,D,pip,p.val);
	    if (debug_infolevel) 
	      CERR << CLOCK()*1e-6 << " ichinrem end\n";
	  }
	  pip = pip*p;
	  if (debug_infolevel) 
	    CERR << CLOCK()*1e-6 << " fracmod\n";
	  //chka1=fracmod(aa[pos],pip);
	  chka2=fracmod(aa.back(),pip);
	  chkb1=fracmod(bb.back(),pip);
	  if (debug_infolevel) 
	    CERR << CLOCK()*1e-6 << " fracmod end\n";
	}
      }
    }
    modpoly r1(n);
    modpoly r2(x);
    modpoly v1,v2(one()),q,r(x),v(1,1);
    gen g(1),h(1),r20,r2pow(1),hpow;
    for (;;){
      // During the loop, v1*x+not_computed*n=r1 and v2*x+not_computed*n=r2
      int deg2=int(r2.size())-1;
      if (deg2<l){ 
	break;
      }
      int deg1=int(r1.size())-1,ddeg=deg1-deg2;
      if (!env || !env->moduloon || !is_zero(env->coeff)){
	r20=r2.front();
	r2pow=pow(r2.front(),ddeg+1);
	DivRem(r2pow*r1,r2,env,q,r);
      }
      else
	DivRem(r1,r2,env,q,r);
      v=operator_minus(r2pow*v1,operator_times(q,v2,env),env);
      if (!psron){
	gen tmp=gcd(lgcd(r),lgcd(v),context0);
	r=operator_div(r,tmp,env);
	v=operator_div(v,tmp,env);
      }
      else {
	if (!env || !env->moduloon || !is_zero(env->coeff)){
	  hpow=pow(h,ddeg);
	  r=operator_div(r,hpow*g,env);
	  v=operator_div(v,hpow*g,env);
	  if (ddeg==1)
	    h=r20;
	  else
	    h=(pow(r20,ddeg)*h)/hpow;
	  g=r20;
	}
      }
      r1=r2;
      r2=r;
      v1=v2;
      v2=v;
    }
    a=r;
    b=v;
    // If a and b are not prime together, we may have a failure
    q=gcd(a,b,env);
    if (q.size()>1)
      return false;
    return true;
  }

  // assumes degree(f)=degree(g)==n
  matrice bezoutian(const modpoly & f,const modpoly &g,environment * env){
    vecteur u(f),v(g);
    reverse(u.begin(),u.end());
    reverse(v.begin(),v.end());
    while (u.size()<v.size())
      u.push_back(0);
    while (v.size()<u.size())
      v.push_back(0);
    int S=u.size()-1;
    matrice bez(S);
    for (int i=0;i<S;++i)
      bez[i]=vecteur(S); 
    // initialization
    for (int i=0;i<S;++i){
      vecteur & b=*bez[i]._VECTptr;
      for (int j=i;j<S;++j){
	gen r = u[i]*v[j+1]-v[i]*u[j+1];
	b[j]=r;
      }
    }
    // recursion
    for (int i=1;i<=S-2;++i){
      vecteur & b=*bez[i]._VECTptr;
      for (int j=i;j<=S-2;++j){
	gen r = b[j];
	r += bez[i-1][j+1];
	b[j] = r;
      }
    }
    // modulo
    if (env && env->moduloon){
      for (int i=0;i<S;++i){
	vecteur & b=*bez[i]._VECTptr;
	for (int j=i;j<S;++j){
	  b[j]=smod(b[j],env->modulo);
	}
      }
    }
    // symmetry
    for (int i=1;i<S;++i){
      vecteur & b=*bez[i]._VECTptr;
      for (int j=0;j<i;++j)
	b[j]=bez[j][i];
    }
    return bez;
  }

  gen _bezoutian(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if (args.type!=_VECT || args._VECTptr->size()!=2)
      return gensizeerr(contextptr);
    gen f=args._VECTptr->front(),g=args._VECTptr->back();
    if (f.type!=_VECT)
      f=_e2r(makesequence(f,vx_var),contextptr);
    if (g.type!=_VECT)
      g=_e2r(makesequence(g,vx_var),contextptr);
    if (f.type!=_VECT || g.type!=_VECT )
      return gendimerr(contextptr);
    return bezoutian(*f._VECTptr,*g._VECTptr,0);
  }
  static const char _bezoutian_s []="bezoutian";
  static define_unary_function_eval (__bezoutian,&_bezoutian,_bezoutian_s);
  define_unary_function_ptr5( at_bezoutian ,alias_at_bezoutian,&__bezoutian,0,true);

  // Given [v_0 ... v_(2n-1)] (begin of the recurrence sequence) 
  // return [b_n...b_0] such that b_n*v_{n+k}+...+b_0*v_k=0
  // Example [1,-1,3,3] -> [1,-3,-6]
  // -> the recurrence relation is v_{n+2}=3v_{n+1}+6v_n
  // Algo: B*V=A with deg(A)< n and deg(B)=n -> B*V_truncated=A mod x^(2n)
  // psron=true by default to use the PSR Euclidean algorithm
  vecteur reverse_rsolve(const vecteur & v_orig,bool psron){
    if (v_orig.size()%2)
      return vecteur(1,gensizeerr(gettext("Argument must be a vector of even size")+gen(v_orig).print(context0)));
    vecteur v(v_orig);
    reverse(v.begin(),v.end());
    int n=int(v.size()/2);
    vecteur x2n(2*n+1),A,B;
    x2n[0]=1;
    egcd_pade(x2n,v,n,A,B,0,psron);
    vecteur G=gcd(A,B,0);
    v=B/G;
    reverse(v.begin(),v.end());
    v=trim(v,0);
    return v;
  }

  //***************************************************************
  // Fonctions independent on the actual implementation of modpoly
  //***************************************************************

  // given a, find u such that 
  // a[0]*...a[n-1]*u[n]+a[0]*...*a[n-2]*a[n]*u[n-1]+...+a[1]*...*a[n-1]*u[0]=1
  bool egcd(const vector<modpoly> & a,environment * env,vector<modpoly> & u){
    int n=int(a.size());
    if (n==0) return false; // setsizeerr(gettext("modpoly.cc/egcd"));
    // first compute the sequence of products
    // pi[0]=a[n-1], pi[k]=pi[k-1]*a[n-k-1], ... pi[n-2]=pi[n-3]*a[1]
    u.clear();
    u.reserve(n);
    vector<modpoly> pi;
    pi.reserve(n);
    pi.push_back(a[n-1]);
    modpoly tmp;
    for (int k=1;k<=n-2;k++){
      operator_times(pi[k-1],a[n-k-1],env,tmp);
      pi.push_back(tmp);
    }
    // COUT << "a:" << a << '\n';
    // COUT << "pi:" << pi << '\n';
    modpoly c(1,plus_one),U(1),v(1),d(1),q,r;
    // compute u[0] using egcd(a[0],p[n-2])
    // since a[0]*()+p[n-2]*u[0]=c
    // then solve ()=v[0]
    for (int k=0;k<=n-2;k++){
      egcd(a[k],pi[n-k-2],env,v,U,d);
      if (d.size()==1 && !is_one(d.front())){
	divmodpoly(v,d.front(),v);
	divmodpoly(U,d.front(),U);
	d.front()=1;
      }
      if (!is_one(d)) return false; // setsizeerr(gettext("modpoly.cc/egcd"));
      // multiply by v and U by c, compute new c, push u[]
      operator_times(U,c,env,tmp); DivRem(tmp,a[k],env,q,r); // r= U*c % a[k]
      u.push_back(r);
      operator_times(v,c,env,tmp); DivRem(tmp,pi[n-k-2],env,q,c); // c=(v*c) % pi[n-k-2];
    }
    u.push_back(c);
    // COUT << "u:" << u << '\n';
    return true;
  }
  
  // same as above
  /*
  vector<modpoly> egcd(const vector<modpoly> & a,environment * env){
    vector<modpoly> u;
    egcd(a,env,u);
    return u;
  }
  */

  modpoly simplify(modpoly & a, modpoly & b,environment * env){
    modpoly g;
    gcdmodpoly(a,b,env,g);
    a=operator_div(a,g,env);
    b=operator_div(b,g,env);
    return g;
  }

  static void inpowmod(const modpoly & p,const gen & n,const modpoly & pmod,environment * env,modpoly & res){
    if (is_zero(n)){
      res=one();
      return ;
    }
    if (is_one(n)){
      res=p;
      return;
    }
#if 1
    modpoly p2k(p),tmp,tmpq;
    res=one();
    gen N(n),q,r;
    while (!is_zero(N)){
      r=irem(N,2,q);
      N=iquo(N,2); // not sure q can be used because of inplace operations
      if (is_one(r)){
	operator_times(res,p2k,env,tmp);
	if (env)
	  DivRem(tmp,pmod,env,tmpq,res);
	else
	  swap(res,tmp); // res=tmp
      }
      operator_times(p2k,p2k,env,tmp);
      if (env)
	DivRem(tmp,pmod,env,tmpq,p2k);
      else
	swap(p2k,tmp); // res=tmp      
    }
#else    
    inpowmod(p,iquo(n,2),pmod,env,res);
    modpoly tmp,q;
    operator_times(res,res,env,tmp); 
    if (env)
      DivRem(tmp,pmod,env,q,res);
    else
      res=tmp; // res=(res*res) % pmod ;
    if (!is_zero(smod(n,2))){
      operator_times(res,p,env,tmp); 
      if (env)
	DivRem(tmp,pmod,env,q,res); // res=(res*p)%pmod;
      else
	res=tmp;
    }
#endif
  }

  modpoly powmod(const modpoly & p,const gen & n,const modpoly & pmod,environment * env){
    if (!ck_is_positive(n,0)){
      return vecteur(1,gensizeerr(gettext("modpoly.cc/powmod")));
    }
    modpoly res;
    inpowmod( (env?operator_mod(p,pmod,env):p) ,n,pmod,env,res);
    return res;
  }

  void hornerfrac(const modpoly & p,const gen &num, const gen &den,gen & res,gen & d){
    d=1;
    if (p.empty())
      res=0;
    else {
      modpoly::const_iterator it=p.begin(),itend=p.end();
      res=*it;
      ++it;
      if (it==itend){
	return;
      }
      d=den;
      for (;;){
	res=res*num+(*it)*d;
	++it;
	if (it==itend)
	  break;
	d=d*den;   
      }
    }    
  }

  gen hornerint(const modpoly & p,const gen & num,const gen & den,bool simp){
    mpz_t resz,dz,numz,denz;
    if (num.type==_INT_)
      mpz_init_set_si(numz,num.val);
    else
      mpz_init_set(numz,*num._ZINTptr);
    if (den.type==_INT_)
      mpz_init_set_si(denz,den.val);
    else
      mpz_init_set(denz,*den._ZINTptr);
    mpz_init_set(dz,denz);
    mpz_init(resz);
    modpoly::const_iterator it=p.begin(),itend=p.end();
    if (it->type==_INT_)
      mpz_set_si(resz,it->val);
    else
      mpz_set(resz,*it->_ZINTptr);
    ++it;
    for (;;){
      // res=res*num+(*it)*d;
      mpz_mul(resz,resz,numz);
      if (it->type==_INT_){
	if (it->val>0)
	  mpz_addmul_ui(resz,dz,it->val);
	else
	  mpz_submul_ui(resz,dz,-it->val);
      }
      else
	mpz_addmul(resz,dz,*it->_ZINTptr);
      ++it;
      if (it==itend)
	break;
      mpz_mul(dz,dz,denz); // d=d*den;
    }
    gen res;
    if (simp)
      res=rdiv(gen(resz),gen(dz),context0);
    else
      res=fraction(gen(resz),gen(dz));
    mpz_clear(resz);
    mpz_clear(dz);
    mpz_clear(denz);
    mpz_clear(numz);
    return res;
  }

  void cint2mpz(const gen & num,mpz_t & numr,mpz_t & numi){
    if (num.type==_INT_){
      mpz_set_si(numr,num.val);
      mpz_set_si(numi,0);
    }
    else {
      if (num.type==_ZINT){
	mpz_set(numr,*num._ZINTptr);
	mpz_set_si(numi,0);
      }
      else {
	if (num._CPLXptr->type==_INT_)
	  mpz_set_si(numr,num._CPLXptr->val);
	else
	  mpz_set(numr,*num._CPLXptr->_ZINTptr);
	if ((num._CPLXptr+1)->type==_INT_)
	  mpz_set_si(numi,(num._CPLXptr+1)->val);
	else
	  mpz_set(numi,*(num._CPLXptr+1)->_ZINTptr);
      }
    }
  }

  gen hornercint(const modpoly & p,const gen & num,const gen & den,bool simp){
    mpz_t resr,resi,dz,numr,numi,denz,tmp1,tmp2,tmp3,tmp4;
    mpz_init(numr); mpz_init(numi);
    cint2mpz(num,numr,numi);
    if (den.type==_INT_)
      mpz_init_set_si(denz,den.val);
    else
      mpz_init_set(denz,*den._ZINTptr);
    mpz_init_set(dz,denz);
    mpz_init(resr);
    mpz_init(resi);
    mpz_init(tmp1);
    mpz_init(tmp2);
    mpz_init(tmp3);
    mpz_init(tmp4);
    modpoly::const_iterator it=p.begin(),itend=p.end();
    cint2mpz(*it,resr,resi);
    ++it;
    for (;;){
      // res=res*num+(*it)*d;
      mpz_mul(tmp1,resr,numr);
      mpz_mul(tmp2,resi,numi);
      mpz_mul(tmp3,resr,numi);
      mpz_mul(tmp4,resi,numr);
      mpz_sub(resr,tmp1,tmp2);
      mpz_add(resi,tmp3,tmp4);
      if (it->type==_INT_){
	if (it->val>0)
	  mpz_addmul_ui(resr,dz,it->val);
	else
	  mpz_submul_ui(resr,dz,-it->val);
      }
      else {
	if (it->type==_ZINT)
	  mpz_addmul(resr,dz,*it->_ZINTptr);
	else {
	  cint2mpz(*it,tmp1,tmp2);
	  mpz_mul(tmp1,tmp1,dz);
	  mpz_mul(tmp2,tmp2,dz);
	  mpz_add(resr,resr,tmp1);
	  mpz_add(resi,resi,tmp2);
	}
      }
      ++it;
      if (it==itend)
	break;
      mpz_mul(dz,dz,denz); // d=d*den;
    }
    gen res;
    if (simp)
      res=rdiv(gen(gen(resr),gen(resi)),gen(dz));
    else
      res=fraction(gen(gen(resr),gen(resi)),gen(dz));
    mpz_clear(tmp4);
    mpz_clear(tmp3);
    mpz_clear(tmp2);
    mpz_clear(tmp1);
    mpz_clear(resr);
    mpz_clear(resi);
    mpz_clear(dz);
    mpz_clear(denz);
    mpz_clear(numr);
    mpz_clear(numi);
    return res;
  }

  gen horner(const modpoly & p,const fraction & f,bool simp){
    if (p.empty())
      return 0;
    gen num=f.num,den=f.den,d=den;
    modpoly::const_iterator it=p.begin(),itend=p.end();
    if (itend-it>2 && is_integer(num) && is_integer(den)){
      for (;it!=itend;++it){
	if (!is_integer(*it))
	  break;
      }
      if (it==itend)
	return hornerint(p,num,den,simp);
    }
    if (itend-it>2 && is_cinteger(num) && is_integer(den)){
      for (;it!=itend;++it){
	if (!is_cinteger(*it))
	  break;
      }
      if (it==itend)
	return hornercint(p,num,den,simp);
    }
    it=p.begin();
    gen res(*it);
    ++it;
    if (it==itend)
      return res;
    for (;;){
      res=res*num+(*it)*d;
      ++it;
      if (it==itend)
	break;
      d=d*den;   
    }
    return rdiv(res,d,context0);
  }

  // n=d-1-e, d=degree(Sd), e=degree(Sd1), Se=(lc(Sd1)^n*Sd1)/lc(Sd)^n
  void ducos_e(const modpoly & Sd,const gen & sd,const modpoly & Sd1,modpoly &Se){
    int n=int(Sd.size()-Sd1.size()-1);
    if (!n){
      Se=Sd1;
      return;
    }
    if (n==1){
      Se=Sd1.front()*Sd1/sd;
      return;
    }
    // n>=2
    gen sd1(Sd1.front()),s((sd1*sd1)/sd);
    for (int j=2;j<n;++j){
      s=(s*sd1)/sd;
    }
    Se=(s*Sd1)/sd;
  }

  // compute S_{e-1}
  void ducos_e1(const modpoly & A,const modpoly & Sd1,const modpoly & Se,const gen & sd,modpoly & res){
    int d=int(A.size())-1,e=int(Sd1.size())-1,dim=1;
    if (debug_infolevel>3)
      CERR << CLOCK()*1e-6 << " ducos_e1 begin d=" << d << '\n';
    gen cd1(Sd1.front()),se(Se.front());
    vector< modpoly > Hv(e);
    Hv.reserve(d);
    if (Se.size()>1 && Se[1]!=0){
      Hv.push_back(modpoly(Se.begin()+1,Se.end()));
      negmodpoly(Hv.back(),Hv.back());
    }
    else {
      modpoly tmp(e+1);
      tmp[0]=se;
      Hv.push_back(tmp-Se); // in fact it's -Se without first element
    }
    for (int j=e+1;j<d;++j){
      modpoly XHj1(Hv.back());
      XHj1.push_back(0); // X*H_{j-1}
      gen piXHj1;
      if (int(XHj1.size())-1-e>=0){
	piXHj1=XHj1[XHj1.size()-1-e];
	XHj1=XHj1-(piXHj1*Sd1)/cd1;
      }
      Hv.push_back(XHj1);
    }
    modpoly D,tmpv; // sum_{j<d} pi_j(A)*H_j/lc(A)
    D.reserve(d);
    // split next loop in 2 parts, because Hv indexes lower than e are straightforward
    if (debug_infolevel>3)
      CERR << CLOCK()*1e-6 << " ducos_e1 D begin" << '\n';
    for (int j=e-1;j>=0;--j){
      D.push_back(A[A.size()-1-j]*se);
    }
    if (debug_infolevel>3)
      CERR << CLOCK()*1e-6 << " ducos_e1 D j=e " << e << "<" << d << '\n';
    for (int j=e;j<d;++j){
      D = D + A[A.size()-1-j]*Hv[j];
    }
    if (debug_infolevel>3)
      CERR << CLOCK()*1e-6 << " ducos_e1 D end, start division" << '\n';
    if (is_integer(A.front())) 
      iquo(D,A.front()); 
    else 
      D = D/A.front();
    if (debug_infolevel>3)
      CERR << CLOCK()*1e-6 << " ducos_e1 D ready" << '\n';
    modpoly & Hd1=Hv.back();
    Hd1.push_back(0); // X*Hd1
    int hd1=int(Hd1.size())-1-e;
    gen hd=hd1<0?0:Hd1[hd1];
#if 1
    addmodpoly(Hd1,D,tmpv); 
    mulmodpoly(tmpv,cd1,tmpv);
    mulmodpoly(Sd1,hd,D);
    submodpoly(tmpv,D,res);
#else
    addmodpoly(D,Hd1,D); 
    mulmodpoly(D,cd1,D);
    mulmodpoly(Sd1,hd,tmpv);
    submodpoly(D,tmpv,D);
    D.swap(res);
    //res=cd1*(Hd1+D)-(hd*Sd1);
#endif
    if (debug_infolevel>3)
      CERR << CLOCK()*1e-6 << " ducos_e1 D final division" << '\n';
    trim_inplace(res); // res=trim(res,0);
    if (is_integer(sd)) iquo(res,sd); else res=res/sd;
    if (!res.empty() && res.front()==0)
      CERR << "err" << '\n';
    if (debug_infolevel>3)
      CERR << CLOCK()*1e-6 << " ducos_e1 end" << '\n';
    if ( (d-e+1)%2)
      res=-res;
  }

  void mulsmall(vector<int> & Q,int c,int m){
    int * ptr=&Q.front(), * ptrend=ptr+Q.size();
    for (;ptr!=ptrend;++ptr){
      *ptr = (longlong(*ptr)*c)%m;
    }
  }

  int resultant_iter(const vector<int> & P0,const vector<int> & Q0,int m){
    if (debug_infolevel>1)
      CERR << CLOCK()*1e-6 << " resultant_iter begin " << Q0.size() << '\n';
    vector<int> P(P0),Q(Q0),tmp1,tmp2;
    longlong res=1;
    double invm=find_invp(m);
    while (Q.size()>1){
      DivRem(P,Q,m,tmp1,tmp2);
      int ddeg=P.size()-tmp2.size();
      if (ddeg==2)
	res = amodp(res*amodp(Q[0]*longlong(Q[0]),m,invm),m,invm);
      else
	res = amodp(res*powmod(Q[0],ddeg,m),m,invm);
      if (P.size()%2==0 && Q.size()%2==0)
	res = -res;
      P.swap(Q);
      Q.swap(tmp2);
    }
    if (Q.empty())
      return 0;
    res = amodp(res*powmod(Q[0],P.size()-1,m),m,invm);
    if (debug_infolevel>1)
      CERR << CLOCK()*1e-6 << " resultant_iter end " << Q0.size() << '\n';
    return smod(res,m);
  }

  // adapted from NTL 
  inline int deg(const vector<int> & v){ return v.size()-1;}
  inline bool IsZero(const vector<int> &v){return v.empty();}
  inline int LeadCoeff(const vector<int> &v){return v.front(); }
  void RightShift(vector<int> & target,const vector<int> & source,long n){
    if (source.size()<n){
      target.clear(); return;
    }
    target.resize(source.size()-n);
    copy(source.begin(),source.end()-n,target.begin());
  }

  void ResHalfGCD(const vector<int> &U,const vector<int> & V,long d_red,vector<int> & cvec,vector<int> & dvec,vector<int> & A,vector<int> &B,vector<int> &C,vector<int> & D,int p,vector<int> & a,vector<int> &b,vector<int> & tmp1,vector<int> & tmp2,vector<int> & tmp3,vector<int> & tmp4){
    if (V.size()<=U.size()-d_red){
      D=A=vector<int>(1,1);
      B.clear(); C.clear(); 
      tmp1.clear(); tmp2.clear(); tmp3.clear(); tmp4.clear();
      return;
    }

    long n = deg(U) - 2*d_red + 2;
    if (n < 0) n = 0;

    vector<int> U1, V1;

    RightShift(U1, U, n);
    RightShift(V1, V, n);

    if (d_red <= HGCD) { 
      hgcd_iter_int(U1,V1,U1.size()-d_red,A,C,B,D,p,cvec,dvec,a,b,tmp1,tmp2,tmp3,tmp4); // d_red?
      tmp1.clear(); tmp2.clear(); tmp3.clear(); tmp4.clear();
      return;
    }

    long d1 = (d_red + 1)/2;
    if (d1 < 1) d1 = 1;
    if (d1 >= d_red) d1 = d_red - 1;

    vector<int> A1,B1,C1,D1,Wp;

    ResHalfGCD(U1, V1, d1, cvec, dvec,A1,B1,C1,D1,p,a,b,tmp1,tmp2,tmp3,tmp4);
    int maxdeg=U1.size()-giacmax(A1.size(),B1.size());
    matrix22inttimesvect(A1,B1,C1,D1,U1,V1,maxdeg,maxdeg,a,b,p,tmp1,tmp2,tmp3,tmp4,Wp);
    a.swap(U1); b.swap(V1);
    tmp1.clear(); tmp2.clear(); tmp3.clear(); tmp4.clear();
      
    long d2 = deg(V1) - deg(U) + n + d_red;

    if (IsZero(V1) || d2 <= 0) {
      A.swap(A1); B.swap(B1); C.swap(C1); D.swap(D1);
      return;
    }

    cvec.push_back( LeadCoeff(V1));
    dvec.push_back( dvec.back()-deg(U1)+deg(V1));
    DivRem(U1,V1,p,tmp2, tmp1); 
    U1.swap(V1); V1.swap(tmp1);
    a_bc(A1,C1,tmp2,p,A1,tmp1);
    a_bc(B1,D1,tmp2,p,B1,tmp1);

    ResHalfGCD(U1, V1, d2, cvec, dvec,A,B,C,D,p,a,b,tmp1,tmp2,tmp3,tmp4);
    matrix22int(A1,B1,C1,D1,A,B,C,D,a,b,U1,V1,p,tmp1,Wp);
    A.swap(a); B.swap(b); C.swap(U1); D.swap(V1);

  }

  void ResHalfGCD(vector<int>& U, vector<int> & V, vector<int>& cvec, vector<int>& dvec,int p,vector<int>& A1,vector<int>& B1,vector<int>& C1,vector<int>& D1,vector<int>& a,vector<int>& b,vector<int>& tmp1,vector<int>& tmp2,vector<int>& tmp3,vector<int>& tmp4){
    long d_red = (deg(U)+1)/2;

    if (IsZero(V) || deg(V) <= deg(U) - d_red) {
      tmp1.clear(); tmp2.clear(); tmp3.clear(); tmp4.clear();
      return;
    }

    long du = deg(U);


    long d1 = (d_red + 1)/2;
    if (d1 < 1) d1 = 1;
    if (d1 >= d_red) d1 = d_red - 1;

    ResHalfGCD(U, V, d1, cvec, dvec,A1,B1,C1,D1,p,a,b,tmp1,tmp2,tmp3,tmp4);
    int maxdeg=U.size()-giacmax(A1.size(),B1.size());
    vector<int> Wp;
    matrix22inttimesvect(A1,B1,C1,D1,U,V,maxdeg,maxdeg,a,b,p,tmp1,tmp2,tmp3,tmp4,Wp);
    U.swap(a); V.swap(b);
    tmp1.clear(); tmp2.clear(); tmp3.clear(); tmp4.clear();

    long d2 = deg(V) - du + d_red;

    if (IsZero(V) || d2 <= 0) {
      return;
    }

    cvec.push_back( LeadCoeff(V));
    dvec.push_back( dvec.back()-deg(U)+deg(V));
    DivRem(U,V, p,tmp2, tmp1); 
    U.swap(V); V.swap(tmp1);

    ResHalfGCD(U, V, d2, cvec, dvec,A1,B1,C1,D1,p,a,b,tmp1,tmp2,tmp3,tmp4);
    maxdeg=U.size()-giacmax(A1.size(),B1.size());
    matrix22inttimesvect(A1,B1,C1,D1,U,V,maxdeg,maxdeg,a,b,p,tmp1,tmp2,tmp3,tmp4,Wp);
    U.swap(a); V.swap(b);
  }

  inline void PlainResultant(int & res,const vector<int> & u,const vector<int> &v,int p){
    res=resultant_iter(u,v,p);
  }

  inline void power(int & res,int a,int m,int p){
    res=powmod(a,m,p);
  }

  void resultant_int_like_ntl(int & res, const vector<int> & u, const vector<int> & v,int p){
    if (deg(u) <= HGCD || deg(v) <= HGCD) { 
      PlainResultant(res, u, v,p);
      return;
    }

    vector<int> u1(u), v1(v),tmp1,tmp2;;

    int t; res=1;

    if (deg(u1) == deg(v1)) {
      DivRem(u1,v1,p,tmp1,tmp2);
      u1.swap(v1);
      v1.swap(tmp2);

      if (IsZero(v1)) {
	res=0;
	return;
      }

      power(t, LeadCoeff(u1), deg(u1) - deg(v1),p);
      res=(longlong(res)*t) %p;
      if (deg(u1) & 1)
	res=-res;
    }
    else if (deg(u1) < deg(v1)) {
      u1.swap(v1);
      if (deg(u1) & deg(v1) & 1)
	res=-res;
    }

    // deg(u1) > deg(v1) && v1 != 0

    vector<int> cvec,dvec;

    cvec.reserve(deg(v1)+2);
    dvec.reserve(deg(v1)+2);

    cvec.push_back( LeadCoeff(u1));
    dvec.push_back( deg(u1));

    vector<int> A1,B1,C1,D1,a,b,tmp3,tmp4; // all temporary

    while (deg(u1) > HGCD && !IsZero(v1)) { 
      ResHalfGCD(u1, v1, cvec, dvec,p,A1,B1,C1,D1,a,b,tmp1,tmp2,tmp3,tmp4);

      if (!IsZero(v1)) {
	cvec.push_back( LeadCoeff(v1));
	dvec.push_back( deg(v1));
	DivRem(u1,v1,p,tmp1,tmp2);
	u1.swap(v1);
	v1.swap(tmp2);
      }
    }

    if (IsZero(v1) && deg(u1) > 0) {
      res=0;
      return;
    }

    long i, l;
    l = dvec.size();

    if (deg(u1) == 0) {
      // we went all the way...

      for (i = 0; i <= l-3; i++) {
	power(t, cvec[i+1], dvec[i]-dvec[i+2],p);
	res=(longlong(res)*t) % p;
	if (dvec[i] & dvec[i+1] & 1)
	  res=-res;
      }

      power(t, cvec[l-1], dvec[l-2],p);
      res=(longlong(res)*t) % p;
    }
    else {
      for (i = 0; i <= l-3; i++) {
	power(t, cvec[i+1], dvec[i]-dvec[i+2],p);
	res=(longlong(res)*t) % p;
	if (dvec[i] & dvec[i+1] & 1)
	  res=-res;
      }

      power(t, cvec[l-1], dvec[l-2]-deg(v1),p);
      res=(longlong(res)*t) % p;
      if (dvec[l-2] & dvec[l-1] & 1)
	res=-res;

      PlainResultant(t, u1, v1,p);
      res=(longlong(res)*t) % p;
    }
  }

  // resultant of P and Q modulo m, modifies P and Q, 
  int resultant_int(vector<int> & P,vector<int> & Q,vector<int> & tmp1,vector<int> & tmp2,int m,int w){
    if (P.size()<Q.size()){
      int res=(P.size() % 2==1 || Q.size() % 2==1)?1:-1; // (-1)^deg(P)*deg(Q)
      return res*resultant_int(Q,P,tmp1,tmp2,m,w);
    }
    if (P.size()==Q.size()){
      int coeff=Q[0];
      int invcoeff=invmod(coeff,m);
      mulsmall(Q,invcoeff,m);
      DivRem(P,Q,m,tmp1,tmp2);
      longlong res=(P.size() % 2==1)?1:-1;
      res = res*powmod(Q[0],P.size()-tmp2.size(),m);
      return smod(res*resultant_int(Q,tmp2,P,tmp1,m,w),m);
    }
    // now P.size()>Q.size()
    int HGCD2=3*HGCD;
    if (Q.size()>=HGCD2){
      if (debug_infolevel>2)
	CERR << "resultantint hgcd mod " << m << '\n';
#if 0 //ndef USE_GMP_REPLACEMENTS // activate NTL-like ResHalfGCD code
      int r;
      resultant_int_like_ntl(r,P,Q,m);
      r=smod(r,m);
      return r;
#endif
      // old code
      vector<int> coeffv,degv,A,B,C,D,a,b,b0,b1,b2,b3,b4,b5,b6,b7,Wp;
      coeffv.reserve(Q.size()+1);
      degv.reserve(Q.size()+1);
      degv.push_back(P.size()-1);
      while (Q.size()>=HGCD2){
#if 0 
	// b4..b7 data is used below
	hgcdint(P,Q,m,Wp,A,B,C,D,coeffv,degv,b2,b3,b4,b5,b6,b7);
#else
	int deg1=P.size(),deg2=(3*deg1)/4;
	double coeff=nextpow2(deg1/2)*2.0/deg1;
	double coeff2=nextpow2(deg2)/double(deg2);
	coeff=0.5*std::min(coeff,coeff2);
	if (Wp.empty() && m!=p1 && m!=p2 && m!=p3){
	  int l=sizeinbase2(int(3*2*coeff/4*deg1-1));
	  if (w){
	    longlong ww=w; // powmod(w,2**(l-1),m)
	    for (int j=1;j<l;++j)
	      ww=(ww*ww)%m;
	    if (ww==1)
	      w=0;
	    else {
	      for (int j=0;ww!=m-1 && j<27;++j){
		ww=(ww*ww)%m;
		w=(w*longlong(w))%m;
	      }
	      if (ww!=m-1)
		w=0;
	    }
	  }
	  if (w==0) 
	    w=find_w(Wp,l,m);
	  fft2wp(Wp,(1<<l),w,m);
	}
	if (debug_infolevel>1)
	  CERR << CLOCK()*1e-6 << " deg " << P.size() << " coeff " << coeff << "\n";
	int seuil=1+int(std::ceil((1-coeff)*P.size())); 
	if (HGCD/4>=Q.size()-seuil){
	  coeffv.push_back(Q.front());
	  degv.push_back(degv.back()+Q.size()-P.size());
	  DivRem(P,Q,m,a,b);
	  P.swap(Q);
	  Q.swap(b);
	  continue;
	}
	// 1st recursive call
	b0.resize(P.size()-seuil); 
	copy(P.begin(),P.end()-seuil,b0.begin()); // quo(P,x^s), 
	b1.resize(Q.size()-seuil);
	copy(Q.begin(),Q.end()-seuil,b1.begin()); // quo(Q,x^s), 
	hgcdint(b0,b1,m,Wp,A,B,C,D,coeffv,degv,b2,b3,b4,b5,b6,b7); // degree=deg(P)*coeff
#endif
	int maxadeg=P.size()-giacmax(A.size(),B.size());
	matrix22inttimesvect(A,B,C,D,P,Q,maxadeg,maxadeg,a,b,m,b4,b5,b6,b7,Wp);
	if (b.size()<HGCD){
	  a.swap(P); b.swap(Q); 
	  break;
	}
	if (1 && a.size()-b.size()==1){
	  a.swap(P); b.swap(Q);	  
	  continue;
	}
	coeffv.push_back(b.front());
	degv.push_back(degv.back()+b.size()-a.size());
	DivRem(a,b,m,P,Q);
	b.swap(P); 
      }
      degv.push_back(Q.size()-1);
      int res=resultant_int(P,Q,tmp1,tmp2,m,w);
      adjust_resultant(res,coeffv,degv,m);
      return smod(res,m);
    }
    return resultant_iter(P,Q,m);
  }
  int sizeinbase2(const gen & g){
    if (g.type==_INT_)
      return sizeinbase2(absint(g.val));
    if (g.type==_ZINT)
      return mpz_sizeinbase(*g._ZINTptr,2);
    if (g.type!=_VECT)
      return -1;
    return sizeinbase2(*g._VECTptr);
  }
  int sizeinbase2(const vecteur & v){
    int m=0;
    const_iterateur it=v.begin(),itend=v.end();
    for (;it!=itend;++it){
      int c=sizeinbase2(*it);
      if (c>m)
	m=c;
    }
    return m+(sizeinbase2(int(v.size()))+1)/2;
  }
  int wpcount=0;
  gen mod_resultant(const modpoly & P,const modpoly & Q,double eps){
    gen R;
    if (P.size()>=NTL_RESULTANT && Q.size()>=NTL_RESULTANT && ntlresultant(P,Q,0,R))
      return R;
    // gen h2=4*pow(l2norm2(P),Q.size()-1)*pow(l2norm2(Q),P.size()-1);
    int h=sizeinbase2(P)*(int(Q.size())-1)+sizeinbase2(Q)*(int(P.size())-1)+1;
    gen D=1; // p-adic acceleration
    if (0 && P.size()>GIAC_PADIC && Q.size()>GIAC_PADIC){
      matrice S=sylvester(P,Q);
      vecteur v=vranm(int(S.size()),0,context0);
      vecteur u=linsolve(S,v,context0);
      lcmdeno(u,D,context0);
      h -= sizeinbase2(D);
    }
    gen P0=P.front(),Q0=Q.front();
#if 0 && defined INT128 // && defined GIAC_LLPRECOND
    if (1){
      vector<longlong> p,q,tmp1,tmp2;
      // reconstruct resultant/D
      int maxdeg=giacmax(P.size(),Q.size())-1;
      // precond compatible => p5
      longlong m=1LL<<61; 
      m=prevprimell(m-1,maxdeg);
      gen pim=m,res;
      vecteur2vector_ll(P,m,p);
      vecteur2vector_ll(Q,m,q);
      res=resultantll(p,q,tmp1,tmp2,m);
#if 0
      ntlresultant(P,Q,m,R,false); 
      if ((R-res)%m!=0)
	CERR << "bug\n";
#endif
      mpz_t tmpz;
      mpz_init(tmpz);
      int proba=0;
      int probamax=RAND_MAX;
      if (eps>0)
	probamax=int(-std::log(eps)/30/std::log(2.0));
      while (h>sizeinbase2(pim) && proba<probamax){
	m=prevprimell(m-1,maxdeg);
	vecteur2vector_ll(P,m,p);
	vecteur2vector_ll(Q,m,q);
	longlong r;
	r=resultantll(p,q,tmp1,tmp2,m);
#if 0
	ntlresultant(P,Q,m,R,false);
	if ((R-r)%m!=0)
	  CERR << "bug\n";
#endif
#ifndef USE_GMP_REPLACEMENTS
	if (pim.type==_ZINT && res.type==_ZINT){
	  longlong amodm=mpz_fdiv_ui(*res._ZINTptr,m);
	  if (amodm!=r){
	    gen u,v,d; longlong U;
	    egcd(pim,m,u,v,d);
	    if (u.type==_ZINT)
	      U=mpz_fdiv_ui(*u._ZINTptr,m);
	    else
	      U=u.val;
	    if (d==-1){ U=-U; v=-v; d=1; }
	    mpz_mul_si(tmpz,*pim._ZINTptr,(U*(r-int128_t(amodm)))%m);
	    mpz_add(*res._ZINTptr,*res._ZINTptr,tmpz);
	    proba=0;
	  }
	  else ++proba;
	}
	else
#endif
	  res=ichinrem(res,gen(r),pim,gen(m));
	pim=gen(m)*pim;
      }
      mpz_clear(tmpz);
      return smod(res,pim)*D;
    }
#endif // INT128
    if (debug_infolevel>1)
      CERR << CLOCK()*1e-6 << " Wpcount begin " << wpcount << "\n";
    vector<int> p,q,tmp1,tmp2;
    // reconstruct resultant/D
    int maxdeg=giacmax(P.size(),Q.size())-1;
    int w=0;
    int m=maxdeg<HGCD?primes31[0]:prevfourier(2147483647,maxdeg,w);//p1,w=1227303670;
    gen pim=m,res;
    vecteur2vector_int(P,m,p);
    vecteur2vector_int(Q,m,q);
    res=resultant_int(p,q,tmp1,tmp2,m,w);
#if 0
    ntlresultant(P,Q,m,R,false); 
    if ((R-res)%m!=0)
      CERR << "bug\n";
#endif
    if (D!=1)
      res=int((res.val*longlong(invmod(smod(D,m).val,m)))%m);
    mpz_t tmpz;
    mpz_init2(tmpz,h);
    int proba=0;
    int probamax=RAND_MAX;
    if (eps>0)
      probamax=1+int(-std::log(eps)/30/std::log(2.0));
    int maxp=std::sqrt(p1p2/4./maxdeg);int niter=1;
    if (debug_infolevel>1)
      CERR << CLOCK()*1e-6 << " resultant modular algo max #primes " << h/30 << endl;
    for (;h>sizeinbase2(pim) && proba<probamax;++niter){
      if (debug_infolevel>1)
	CERR << CLOCK()*1e-6 << " prevfourier start\n";
      if (maxdeg<HGCD){
	w=0;
	if (niter<nprimes31)
	  m=primes31[niter];
	else
	  m=prevprime(m-1).val;
      }
      else
	m=prevfourier(m-1,maxdeg,w);//prevprimep1p2p3(m-1,maxp,maxdeg);
      if (debug_infolevel>1)
	CERR << CLOCK()*1e-6 << " prevfourier end\n";
      // CERR << CLOCK()*1e-6 << " " << m << "\n";
      if (m && (is_multiple(P0,m)||is_multiple(Q0,m)))
	continue;
#if defined INT128 && !defined USE_GMP_REPLACEMENTS && !defined BF2GMP_H
      if (m<(1<<30) 
	  && m!=p3
	  ){
	CERR << CLOCK()*1e-6 << " modular resultant, switching to long primes\n";
	longlong m=(1LL<<61); //m=2305843009116209153+16385;
	vector<longlong> p,q,tmp1,tmp2;
	mpz_t tmpz;
	mpz_init(tmpz);
	for (;h>sizeinbase2(pim) && proba<probamax;++niter){
	  m=prevprimell(m-1,maxdeg);
	  vecteur2vector_ll(P,m,p);
	  vecteur2vector_ll(Q,m,q);
	  longlong r;
	  r=resultantll(p,q,tmp1,tmp2,m);
#if 0
	  ntlresultant(P,Q,m,R,false);
	  if ((R-r)%m!=0){
	    CERR << "bug\n";
	    CERR << m << endl;
	    break;
	  }
#endif
#ifndef USE_GMP_REPLACEMENTS
	  if (pim.type==_ZINT && res.type==_ZINT){
	    longlong amodm=mpz_fdiv_ui(*res._ZINTptr,m);
	    if (amodm!=r){
	      gen u,v,d; longlong U;
	      egcd(pim,m,u,v,d);
	      if (u.type==_ZINT)
		U=mpz_fdiv_ui(*u._ZINTptr,m);
	      else
		U=u.val;
	      if (d==-1){ U=-U; v=-v; d=1; }
	      mpz_mul_si(tmpz,*pim._ZINTptr,(U*(r-int128_t(amodm)))%m);
	      mpz_add(*res._ZINTptr,*res._ZINTptr,tmpz);
	      proba=0;
	    }
	    else 
	      ++proba;
	    mpz_mul_si(*pim._ZINTptr,*pim._ZINTptr,m);
	    continue;
	  }
	  else
#endif
	    res=ichinrem(res,gen(r),pim,gen(m));
	  pim=gen(m)*pim;
	}
	mpz_clear(tmpz);
	break;
      }
#endif
      vecteur2vector_int(P,m,p);
      vecteur2vector_int(Q,m,q);
      int r;
      if (debug_infolevel>1)
	CERR << CLOCK()*1e-6 << " resultant begin niter " << niter << " proba " << proba << " p= " << m << "\n";
      r=resultant_int(p,q,tmp1,tmp2,m,w); 
      if (debug_infolevel>1)
	CERR << CLOCK()*1e-6 << " resultant end " << niter << endl;
#if 0
      ntlresultant(P,Q,m,R,false);
      if ((R-r)%m!=0)
	CERR << "bug\n";
#endif
      if (D!=1)
	r=(r*longlong(invmod(smod(D,m).val,m)))%m;
#ifndef USE_GMP_REPLACEMENTS
      if (pim.type==_ZINT && res.type==_ZINT){
	if (debug_infolevel>1)
	  CERR << CLOCK()*1e-6 << " ichinrem start\n";
	int amodm=modulo(*res._ZINTptr,m);
	if ((amodm-r)%m!=0){
	  int u,v,d;
	  d=iegcd(modulo(*pim._ZINTptr,m),m,u,v);
	  if (d==-1){ u=-u; v=-v; }
	  mpz_mul_si(tmpz,*pim._ZINTptr,(u*(r-longlong(amodm)))%m);
	  mpz_add(*res._ZINTptr,*res._ZINTptr,tmpz);
	  if (debug_infolevel>1)
	    CERR << CLOCK()*1e-6 << " ichinrem end\n";
	  proba=0;
	}
	else ++proba;
      }
      else
#endif
	res=ichinrem(res,r,pim,m);
      pim=m*pim;
#if !defined USE_GMP_REPLACEMENTS && !defined BF2GMP_H
      if (res.type==_ZINT && niter==1)
	mpz_realloc2(*res._ZINTptr,h);
#endif
    }
    if (debug_infolevel>1)
      CERR << CLOCK()*1e-6 << " Wpcount end " << wpcount << " nprimes " << niter << "\n";
    mpz_clear(tmpz);
    return smod(res,pim)*D;
  }

  // resultant of P and Q, modifies P and Q, 
  // suitable if coeffs are invertible without fraction
  gen gf_ext_resultant(const vecteur & P0,const vecteur & Q0){
    vecteur P(P0),Q(Q0),tmp1,tmp2;
    gen res=1;
    while (Q.size()>1){
      gen coeff=Q[0];
      gen invcoeff=inv(coeff,context0);
      mulmodpoly(Q,invcoeff,Q);
      DivRem(P,Q,0,tmp1,tmp2);
      res = res*pow(coeff,int(P.size())-1);
      if (P.size()%2==0 && Q.size()%2==0)
	res = -res;
      P.swap(Q);
      Q.swap(tmp2);
    }
    if (Q.empty())
      return 0;
    res = (res*pow(Q[0],int(P.size())-1));
    return res;
  }

  void subresultant(const modpoly & P,const modpoly & Q,gen & res){
    if (
	(
	 //0 && 
	 P.size()>MODRESULTANT && Q.size()>MODRESULTANT && is_integer_vecteur(P) && is_integer_vecteur(Q))
	){
      res=mod_resultant(P,Q,0.0); 
      // according to my tests ducos is faster (except for very small coefficients)
      return ;
    }
    int d=int(P.size())-1,e=int(Q.size())-1;
    if (d<e){
      subresultant(Q,P,res);
      // adjust sign
      if ((d*e)%2) res=-res;
      return;
    }
    if (e<=0){
      res=pow((e<0?0:Q[0]),d,context0);
      return;
    }
    for (int i=0;i<P.size();++i){
      gen g=P[i];
      if (g.type==_USER){
	res=gf_ext_resultant(P,Q);
	return;
      }
      if (g.type==_EXT){
	gen h=*g._EXTptr;
	if (h.type==_VECT){
	  for (int j=0;j<h._VECTptr->size();++j){
	    gen k=(*h._VECTptr)[j];
	    if (k.type==_USER){
	      res=gf_ext_resultant(P,Q);
	      return;
	    }
	  }
	}
      }
    }
    gen sd(pow(Q[0],d-e,context0)),tmp;
    vecteur A(Q),a,B,C,quo;
    PseudoDivRem(P,-Q,quo,B,tmp);
    for (unsigned step=0;;++step){
      d=int(A.size())-1,e=int(B.size())-1;
      if (B.empty()){
	res=0;
	return ;
      }
      int delta=d-e;
      if (delta>1){
	gen sd(A[0]);
	if (step==0)
	  sd=pow(sd,P.size()-Q.size(),context0);
	ducos_e(A,sd,B,C);
      }
      else
	C=B;
      if (e==0){
	// adjust sign: already done by doing pseudodivrem(-Q,...)
	//if ((P.lexsorted_degree()*Q.lexsorted_degree())%2) C=-C;
	res=C[0];
	return;
      }
      ducos_e1(A,B,C,sd,B);
      A.swap(C); // A=C;
      sd=A[0];
    }
  }  

  // P(x) -> P(-x)
  void Pminusx(vecteur & P){
    unsigned Ps=unsigned(P.size());
    for (unsigned i=0;i<Ps;++i){
      if ( (Ps-i-1) %2)
	P[i]=-P[i];
    }
  }

  // split P=Pp-Pn in two parts, Pp positive coeffs and Pn negative coeffs
  void splitP(const vecteur &P,vecteur &Pp,vecteur &Pn){
    unsigned Ps=unsigned(P.size());
    Pp.resize(Ps);
    Pn.resize(Ps);
    for (unsigned i=0;i<Ps;++i){
      if (is_positive(P[i],context0))
	Pp[i]=P[i];
      else
	Pn[i]=-P[i];
    }
  }

#ifdef HAVE_LIBMPFI
  gen horner_basic(const modpoly & p,const gen & x){
    modpoly::const_iterator it=p.begin(),itend=p.end();
    gen res(*it);
    ++it;
    for (;it!=itend;++it)
      res=res*x+(*it);
    return res;
  }
  
  gen horner_interval(const modpoly & p,const gen & x){
    gen l=_left(x,context0),r=_right(x,context0);
    if (l.type!=_REAL || r.type!=_REAL)
      return gensizeerr(context0);
    bool lpos=is_positive(l,context0),rpos=is_positive(r,context0);
    if (lpos && rpos){
      l=real_interval(*l._REALptr);
      r=real_interval(*r._REALptr);
      gen n1,n2,p1,p2;
      modpoly pp,pn;
      splitP(p,pp,pn);
      p1=horner_basic(pp,l);
      p2=horner_basic(pp,r);
      n1=horner_basic(pn,l);
      n2=horner_basic(pn,r);
      l=_left(p1,context0)-_right(n2,context0);
      r=_right(p2,context0)-_left(n1,context0);
      l=gen(makevecteur(l,r),_INTERVAL__VECT);
      l=eval(l,1,context0);
      return l;
    }
    if ((is_exactly_zero(l) || !lpos) && (is_exactly_zero(r) || !rpos)){
      modpoly pm(p); Pminusx(pm);
      return horner_interval(pm,-x);
    }
    l=gen(makevecteur(l,0),_INTERVAL__VECT);
    l=eval(l,1,context0);
    l=horner_interval(p,l);
    r=gen(makevecteur(0,r),_INTERVAL__VECT);
    r=eval(r,1,context0);    
    r=horner_interval(p,r);
    gen m=min(_left(l,context0),_left(r,context0),context0);
    gen M=max(_right(l,context0),_right(r,context0),context0);
    l=gen(makevecteur(m,M),_INTERVAL__VECT);
    l=eval(l,1,context0);
    return l;
  }
#endif

  // p([l,r]) with l and r exact
  vecteur horner_interval(const modpoly & p,const gen & l,const gen & r){
    bool lpos=is_positive(l,context0),rpos=is_positive(r,context0);
    if (lpos && rpos){
      gen n1,n2,p1,p2;
      modpoly pp,pn;
      splitP(p,pp,pn);
      p1=horner(pp,l,0,false);
      p2=horner(pp,r,0,false);
      n1=horner(pn,l,0,false);
      n2=horner(pn,r,0,false);
      return makevecteur(p1-n2,p2-n1);
    }
    if ((is_exactly_zero(l) || !lpos) && (is_exactly_zero(r) || !rpos)){
      modpoly pm(p); Pminusx(pm);
      return horner_interval(pm,-r,-l);
    }
    vecteur L=horner_interval(p,l,0);
    vecteur R=horner_interval(p,0,r);
    gen m=min(L[0],R[0],context0);
    gen M=max(L[1],R[1],context0);
    return makevecteur(m,M);
  }

  /* set res to p^m
     If   p(x) = sum_{i=0}^n p_i x^k
     Then p(x)^m = sum_{k=0}^{m*n} a(m,k) x^k
     a(m,0) = p_0^m, 
     a(m,k) = 1/(k p_0) sum_{i=1}^min(n,k) p_i *((m+1)*i-k) *a(m,k-i),
     does not work in non-0 characteristic
  */
  bool miller_pow(const modpoly & p_,unsigned m,modpoly & res){
    if (p_.empty()){
      res.clear();
      return true;
    }
    // quichk check for 0 char
    const_iterateur it=p_.begin(),itend=p_.end();
    for (;it!=itend;++it){
      gen g=*it;
      int t=g.type;
      while (t==_EXT || t==_POLY){
	if (t==_EXT){ 
	  if (g._EXTptr->type==_VECT && !g._EXTptr->_VECTptr->empty()){
	    g=g._EXTptr->_VECTptr->front();
	    t=g.type;
	  }
	  else return false;
	}	  
	if (t==_POLY){
	  if (g._POLYptr->coord.empty())
	    return false;
	  g=g._POLYptr->coord.front().value;
	  t=g.type;
	}
      }
      if (t==_VECT || t==_MOD || t==_USER)
	return false;
    }
    modpoly p(p_);
    int shift=0;
    for (;!p.empty() && is_zero(p.back());++shift)
      p.pop_back();
    reverse(p.begin(),p.end());
    unsigned n=int(p.size())-1;
    unsigned mn=n*m;
    res.resize(mn+1);
    gen p0=p[0],invp0;
    if (p0.type==_VECT)
      return false;
    if (p0.type==_EXT || p0.type==_USER)
      invp0=inv(p0,context0);
    res[0]=pow(p0,int(m),context0);
    for (unsigned k=1;k<=mn;++k){
      unsigned end=k<n?k:n;
      gen tmp;
      for (unsigned i=1;i<=end;++i){
	tmp += int((m+1)*i-k)*(p[i]*res[k-i]);
      }
      if (is_zero(invp0))
	res[k]=tmp/(int(k)*p0);
      else
	res[k]=tmp*(invp0/int(k));
    }
    reverse(res.begin(),res.end());
    if (shift)
      res=mergevecteur(res,vecteur(m*shift,0));
    return true;
  }

  gen horner(const modpoly & p,const gen & x,environment * env,bool simp){
    int s=int(p.size());
    if (s==0)
      return 0;
    if (s==1)
      return p.front();
    if (is_inf(x)){
      if (s%2)
	return plus_inf*p.front();
      return x*p.front();
    }
    if (s==2){
      if (env && env->moduloon)
	return smod(p.front()*x+p.back(),env->modulo);
      else
	return p.front()*x+p.back();
    }
    if ( (!env || !env->moduloon || !is_zero(env->coeff)) && x.type==_FRAC)
      return horner(p,*x._FRACptr,simp);
#if defined HAVE_LIBMPFI && !defined NO_RTTI
    if (x.type==_REAL){
      if (dynamic_cast<real_interval *>(x._REALptr))
	return horner_interval(p,x);
    }
#endif
    modpoly::const_iterator it=p.begin(),itend=p.end();
    if (x.type==_CPLX && x.subtype==3){
      complex<double> res(0),X(x._CPLXptr->_DOUBLE_val,(x._CPLXptr+1)->_DOUBLE_val);
      bool ok=true;
      for (;ok && it!=itend;++it){
	res *=X;
	switch (it->type){
	case _INT_:
	  res += it->val;
	  break;
	case _DOUBLE_:
	  res += it->_DOUBLE_val;
	  break;
	case _CPLX:
	  if (it->subtype==3){
	    res += complex<double>(it->_CPLXptr->_DOUBLE_val,(it->_CPLXptr+1)->_DOUBLE_val);
	    break;
	  }
	default:
	  ok=false;
	}
      }
      if (ok) return res;
    }
    it=p.begin();
    gen res(*it);
    ++it;
    if (env && env->moduloon){
      for (;it!=itend;++it)
	res=smod(res*x+(*it),env->modulo);
    }
    else {
      for (;it!=itend;++it)
	res=res*x+(*it);
    }
    return res;
  }

  gen horner(const modpoly & p,const gen & x){
    return horner(p,x,0);
  }
   
  gen horner(const gen & g,const gen & x){
    if (g.type!=_VECT)
      return g;
    return horner(*g._VECTptr,x);
  }
  complex<double> horner_newton(const vecteur & p,const std::complex<double> &x,GIAC_CONTEXT){
    complex<double> num,den;
    const_iterateur it=p.begin(),itend=p.end();
    double n=itend-it-1; gen tmp;
    for (;it!=itend;--n,++it){
      num *= x;
      if (n) den *= x;
      switch (it->type){
      case _INT_:
	num += it->val;
	den += n*it->val;
	break;
      case _DOUBLE_:
	num += it->_DOUBLE_val;
	den += n*it->_DOUBLE_val;
	break;
      case _CPLX:
	tmp=it->subtype==3?*it:evalf_double(*it,1,contextptr);
	if (tmp.type==_CPLX && tmp.subtype==3){
	  num += complex<double>(tmp._CPLXptr->_DOUBLE_val,(tmp._CPLXptr+1)->_DOUBLE_val);
	  den += n*complex<double>(tmp._CPLXptr->_DOUBLE_val,(tmp._CPLXptr+1)->_DOUBLE_val);
	  break;
	}
      default:
	return (num=0)/(den=0);
      }
    } // end for
    return x-num/den;
  }
  gen _horner(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if (args.type!=_VECT)
      return symbolic(at_horner,args);
    vecteur & v=*args._VECTptr;
    int s=int(v.size());
    if (s<2)
      return gensizeerr(contextptr);
    const gen &p=v.front();
    const gen & q=v[1];
    if (p.type==_VECT){
      if (q.type==_VECT && (p._VECTptr->size()==q._VECTptr->size() || p._VECTptr->size()==q._VECTptr->size()+1) && s==3){
	// Horner-like evaluation for divided difference
	// p=divided differences, q=list of abscissas, r=eval point
	const gen & x=v[2];
	const vecteur & P=*p._VECTptr;
	s=int(P.size())-1;
	const vecteur & Q=*q._VECTptr;
	gen r=P[s];
	for (int i=s-1;i>=0;--i){
	  r=r*(x-Q[i])+P[i];
	}
	return r;
      }
      if (s==3){
	const gen & v2=v[2];
	if (v2.type==_FUNC && *v2._FUNCptr==at_newton){
	  // Newton iteration for a polynomial
	  complex<double> x;
	  if (q.type==_DOUBLE_)
	    x=q._DOUBLE_val;
	  else {
	    if (q.type==_CPLX && q.subtype==3)
	      x=complex<double>(q._CPLXptr->_DOUBLE_val,(q._CPLXptr+1)->_DOUBLE_val);
	    else {
	      gen tmp=evalf_double(q,1,contextptr);
	      if (tmp.type!=_CPLX || tmp.subtype!=3)
		return gensizeerr(contextptr);
	      x=complex<double>(tmp._CPLXptr->_DOUBLE_val,(tmp._CPLXptr+1)->_DOUBLE_val);
	    }
	  }
	  return horner_newton(*p._VECTptr,x,contextptr);
	}
      } // end newton iteration
      return horner(*p._VECTptr,q);
    }
    gen x;
    if (s==2)
      x=vx_var;
    else 
      x=v.back();
    if (!is_zero(derive(q,x,contextptr))) 
      return gensizeerr(contextptr);
    vecteur lv(1,x);
    lvar(p,lv);
    lvar(q,lv);
    gen aa=e2r(p,lv,contextptr),aan,aad;
    fxnd(aa,aan,aad);
    if ( ( (aad.type==_POLY)&&(aad._POLYptr->lexsorted_degree()) )
	 )
      return gensizeerr(contextptr);
    if (aan.type!=_POLY)
      return p;
    vecteur lv1=vecteur(lv.begin()+1,lv.end());
    gen ba=e2r(q,lv1,contextptr);
    vecteur a(polynome2poly1(*aan._POLYptr,1));
    return r2e(horner(a,ba),lv1,contextptr)/r2e(aad,lv,contextptr);
  }
  static const char _horner_s []="horner";
  static define_unary_function_eval (__horner,&_horner,_horner_s);
  define_unary_function_ptr5( at_horner ,alias_at_horner,&__horner,0,true);

  gen symb_horner(const modpoly & p,const gen & x,int d){
  // better suited if x is symbolic
    if (p.empty())
      return 0;
    modpoly::const_iterator it=p.begin(),itend=p.end();
    gen res;
    int i=int(itend-it)-1;
    if (!i)
      return *it;
    for (;i>=0;++it,--i){
      if (i==d+1)
	res=res+(*it)*x;
      else {
	if (i==d)
	  res=res+(*it);
	else
	  res=res+(*it)*symbolic(at_pow,gen(makevecteur(x,i-d),_SEQ__VECT));
      }
    }
    return res;
  }

  gen symb_horner(const modpoly & p,const gen & x){
    if (x.type==_VECT && x._VECTptr->empty())
      return gen(p,_POLY1__VECT);
    return symb_horner(p,x,0);
  }

  // p=(X-x)q+p(x)
  gen horner(const modpoly & p,const gen & x,environment * env,modpoly & q){
    modpoly::const_iterator it=p.begin(),itend=p.end();
    if (p.empty()){
      q.clear();
      return 0;
    }
    q.resize(itend-it-1); 
    gen res(*it);
    ++it;
    if (it==itend)
      return res;
    q[0]=res; 
    if (env && env->moduloon){
      for (int pos=1;;++pos){
	res=smod(res*x+(*it),env->modulo);
	++it;
	if (it==itend)
	  break;
	q[pos]=res;
      }
    }
    else {
      if (x==1){
	for (int pos=1;;++pos){
	  res += *it ;
	  ++it;
	  if (it==itend)
	    break;
	  q[pos]=res;
	}
      }
      else {
	for (int pos=1;;++pos){
	  res=res*x+(*it);
	  ++it;
	  if (it==itend)
	    break;
	  q[pos]=res;
	}
      }
    }
    return res;
  }

  static modpoly taylordiff(const modpoly & p,const gen & x){
    int d=int(p.size());
    modpoly res(p),P(p);
    for (int i=1;i<=d;++i){
      res[d-i]=horner(P,x);
      P=derivative(P)/gen(i);
    }
    return res;
  }
  
  void modpoly2mpzpoly(const modpoly & p,mpz_t * & res){
    const_iterateur it=p.begin(),itend=p.end();
    res=new mpz_t[itend-it];
    for (int i=0;it!=itend;++i,++it){
      if (it->type==_INT_)
	mpz_init_set_si(res[i],it->val);
      else
	mpz_init_set(res[i],*it->_ZINTptr);
    }
  }

  void taylorshift1n(mpz_t * tab,int size){
    for (int i=1;i<size;++i){
      // tab[j]=tab[j-1]+tab[j] for j from 1 to size-i
      for (int j=1;j<=size-i;++j){
	mpz_add(tab[j],tab[j],tab[j-1]);
      }
    }
  }

  void mpzpoly2modpolynodel(mpz_t * p,modpoly & res){
    iterateur it=res.begin(),itend=res.end();
    for (int i=0;it!=itend;++i,++it){
      *it=*(p+i);
    }
  }

  int binary_content(const modpoly & v){
#ifdef USE_GMP_REPLACEMENTS
    return 0;
#else
    int res=-1;
    for (int i=0;res && i<v.size();++i){
      if (is_zero(v[i]))
	continue;
      if (v[i].type==_INT_)
	return 0;
      if (v[i].type!=_ZINT)
	return -1;
      int cur=mpz_scan1(*v[i]._ZINTptr,0);
      if (res==-1 || cur<res)
	res=cur;
    }
    return res;
#endif
  }

  void mul_2exp(modpoly & v,int b){
    if (b<=0) return;
    for (int i=0;i<v.size();++i){
      if (v[i].type==_INT_)
	v[i].uncoerce();
      mpz_mul_2exp(*v[i]._ZINTptr,*v[i]._ZINTptr,b);
    }
  }

  void div_2exp(modpoly & v,int b){
    if (b<=0) return;
    for (int i=0;i<v.size();++i){
      if (v[i].type==_INT_)
	v[i].uncoerce();
      mpz_tdiv_q_2exp(*v[i]._ZINTptr,*v[i]._ZINTptr,b); // change for HP mpz_fdiv_q_2exp
    }
  }

  void taylorshift1(mpz_t * tab,int size,matrice * Pascal){
    int etages=0,s=size;
    while (s>=20*FFTMUL_SIZE){
      ++etages;
      s=(s+1)/2;
    }
    // size<=2^etages*s
    if (//0 && 
	etages){
      // slice tab in at most 2^etages blocks of size s
      // tab=[tab_k,..,tab_0]
      // shift them [stab_k,...,stab_0]
      // now compute (x+1)^(s*k)*stab_k+...+stab_0 
      // compute (x+1)^s, then 
      // (x+1)^s*stab_1+stab_0, (x+1)^s*stab_3+stab_2, ...
      // compute (x+1)^(2s) square of (x+1)^s, then
      // (x+1)^(2s)*((x+1)^s*stab_3+stab_2) + (x+1)^s*stab_1+stab_0
      // etc.
      int n=(size+s-1)/s; // super-degree
      matrice m(n),nextm((n+1)/2); 
      mpz_t * cur=tab+size;
      if (debug_infolevel) 
	CERR << CLOCK()*1e-6 << " init begin\n";
      for (int i=0;i<n;++i){
	cur -= s;
	if (cur<tab){
	  taylorshift1n(tab,s-(tab-cur));
	  m[i]=vecteur(s-(tab-cur));
	  mpzpoly2modpolynodel(tab,*m[i]._VECTptr);
	}
	else {
	  taylorshift1n(cur,s);
	  m[i]=vecteur(s);
	  mpzpoly2modpolynodel(cur,*m[i]._VECTptr);
	}
      }
      for (int i=0;i<nextm.size();++i){
	nextm[i]=vecteur(0);
      }
      if (debug_infolevel) 
	CERR << CLOCK()*1e-6 << " pascal " << s << '\n';
      vecteur x1s,tmp;
      gen * Pptr=Pascal && (Pascal->size()>=etages) ? &Pascal->front() : 0;
      if (Pptr){
	if (Pptr->type==_VECT && Pptr->_VECTptr->size()==s+1){
	  x1s=*Pptr->_VECTptr;
	  ++Pptr;
	}
	else {
	  Pascal->clear();
	  Pptr=0;
	}
      }
      if (x1s.empty()) {
	x1s=pascal_nth_line(s);
	if (Pascal){
	  Pascal->reserve(etages);
	  Pascal->push_back(x1s);
	}
      }
      if (debug_infolevel) 
	CERR << CLOCK()*1e-6 << " pascal end " << s << '\n';
      for (;etages>0;--etages){
	for (int i=0;i<n;i+=2){
	  if (i<n-1){
	    int b=binary_content(*m[i+1]._VECTptr);
	    if (debug_infolevel) 
	      CERR << CLOCK()*1e-6 << " * binary " << b << '\n';
	    div_2exp(*m[i+1]._VECTptr,b);
	    if (0 && x1s.size()<8*FFTMUL_SIZE)
	      mulmodpoly_kara_naive(x1s,*m[i+1]._VECTptr,0,*nextm[i/2]._VECTptr,INT_KARAMUL_SIZE);	    
	    else
	      mulmodpoly(x1s,*m[i+1]._VECTptr,0,*nextm[i/2]._VECTptr);
	    mul_2exp(*nextm[i/2]._VECTptr,b);
	    if (debug_infolevel) 
	      CERR << CLOCK()*1e-6 << " * end\n";
	    addmodpoly(*nextm[i/2]._VECTptr,*m[i]._VECTptr,0,*nextm[i/2]._VECTptr);
	  }
	  else
	    m[i]._VECTptr->swap(*nextm[i/2]._VECTptr);
	}
	if (etages>1){
	  if (Pptr){
	    x1s=*Pptr->_VECTptr;
	    ++Pptr;
	  }
	  else {
	    mulmodpoly(x1s,x1s,0,tmp);	
	    x1s.swap(tmp);
	    if (Pascal)
	      Pascal->push_back(x1s);
	  }
	}
	if (debug_infolevel) 
	  CERR << CLOCK()*1e-6 << " pascal^2 end\n";
	n=(n+1)/2;
	m.swap(nextm);
      }
      // transfert nextm[0] in tab
      const vecteur & v =*m[0]._VECTptr;
      for (int i=0;i<size;++i){
	if (v[i].type==_INT_)
	  mpz_set_si(tab[i],v[i].val);
	else
	  mpz_set(tab[i],*v[i]._ZINTptr);
      }
    }
    else
      taylorshift1n(tab,size);
  }

  void mpzpoly2modpoly(mpz_t * p,modpoly & res){
    iterateur it=res.begin(),itend=res.end();
    for (int i=0;it!=itend;++i,++it){
      *it=*(p+i);
      mpz_clear(p[i]);
    }
    delete [] p;
  }

  bool isintpoly(const modpoly & p){
    const_iterateur it=p.begin(),itend=p.end();
    for (;it!=itend;++it){
      if (!is_integer(*it))
	return false;
    }
    return true;
  }

  // shift polynomial
  modpoly taylor(const modpoly & p,const gen & x,environment * env,matrice * P){
    if (p.empty())
      return p;
    if ( (!env || !env->moduloon || !is_zero(env->coeff)) && x.type==_FRAC) // use derivatives of p
      return taylordiff(p,x);
    modpoly res,a,b;
    a=p;
    if (x==1 && a.size()>5 && isintpoly(a)){
      mpz_t * tab;
      modpoly2mpzpoly(a,tab);
      taylorshift1(tab,int(a.size()),P);
      mpzpoly2modpoly(tab,a);
      return a;
    }
    int d=int(p.size());
    for (int i=0;i<d;++i){
      res.push_back(horner(a,x,env,b));
      a.swap(b); // a=b;
    }
    reverse(res.begin(),res.end());
    return res;
  }

  gen lgcd(const dense_POLY1 & p){
    if (p.empty())
      return 1;
    dense_POLY1::const_iterator it=p.begin(),itend=p.end();
    gen n(*it),n1(1);
    for (;it!=itend;++it){
      n=gcd(n,*it,context0);
      if (n==n1)
        return 1;
    }
    return n;
  }

  // gcd of coeff of p and g
  gen lgcd(const dense_POLY1 & p,const gen & g){
    if (p.empty())
      return g;
    dense_POLY1::const_iterator it=p.begin(),itend=p.end();
    gen n(g);
    for (;it!=itend;++it){
      n=gcd(n,*it,context0);
      if (is_one(n))
        return 1;
    }
    return n;
  }

  gen ppz(dense_POLY1 & p){
    gen n(lgcd(p));
    p=p/n;
    return n;
  }

  // does not seem threadable, no idea why...
  gen norm(const dense_POLY1 & p,GIAC_CONTEXT){
    gen res;
    dense_POLY1::const_iterator it=p.begin(), itend=p.end();
    for (;it!=itend;++it){
      gen tmp(abs(*it,contextptr));
      if (is_strictly_greater(tmp,res,contextptr)) // (res<tmp)
	res=tmp;
    }
    return res;
  }

  gen intnorm(const dense_POLY1 & p,GIAC_CONTEXT){
    gen res,mres;
    dense_POLY1::const_iterator it=p.begin(), itend=p.end();
    for (;it!=itend;++it){
      if (it->type==_INT_){
	if (res.val*longlong(res.val)<it->val*longlong(it->val)){
	  res.val=absint(it->val);
	  mres.val=-res.val;
	}
	continue;
      }
      if (it->type!=_ZINT)
	return norm(p,contextptr);
      mres=res=*it;
      if (is_positive(res,contextptr))
	mres=-res;
      else
	res=-mres;
      break;
    }
    for (;it!=itend;++it){
      if (it->type==_INT_)
	continue;
      if (it->type!=_ZINT)
	return norm(p,contextptr);
      if (
	  (res.type==_ZINT && mpz_cmp(*it->_ZINTptr,*res._ZINTptr)>0)
	  || (res.type==_INT_ && mpz_cmp_si(*it->_ZINTptr,res.val)>0)
	  ){
	res=*it;
	mres=-res;
	continue;
      }
      if (
	  (mres.type==_ZINT && mpz_cmp(*mres._ZINTptr,*it->_ZINTptr)>0)
	  || (mres.type==_INT_ && mpz_cmp_si(*it->_ZINTptr,mres.val)<0)
	  ){
	mres=*it;
	res=-mres;
      }
    }
    //if (res!=norm(p,contextptr)) CERR << "intnorm err" << '\n';
    return res;
  }

  // assuming pmod and qmod are prime together, find r such that
  // r = p mod pmod  and r = q mod qmod
  // hence r = p + A*pmod = q + B*qmod
  // or A*pmod -B*qmod = q - p
  // assuming u*pmod+v*pmod=d we get
  // A=u*(q-p)/d
  dense_POLY1 ichinrem(const dense_POLY1 &p,const dense_POLY1 & q,const gen & pmod,const gen & qmod){
    gen u,v,d,tmp,pqmod(pmod*qmod);
    egcd(pmod,qmod,u,v,d);
    // COUT << u << "*" << pmod << "+" << v << "*" << qmod << "=" << d << " " << u*pmod+v*qmod << '\n';
    dense_POLY1::const_iterator a = p.begin();
    dense_POLY1::const_iterator a_end = p.end();
    dense_POLY1::const_iterator b = q.begin();
    dense_POLY1::const_iterator b_end = q.end();
    int n=int(a_end-a), m=int(b_end-b);
    dense_POLY1 res;
    res.reserve(giacmax(n,m));
    for (;m>n;++b,--m)
      res.push_back(smod(iquo(u*(*b),d),pqmod));
    for (;n>m;++a,--n)
      res.push_back(smod(*a-iquo(u*(*a),d),pqmod));
    for (;a!=a_end;++a,++b){
      res.push_back(smod(*a+iquo(u*(*b-*a),d) *pmod,pqmod)) ;
      // COUT << a->value << " " << b->value << "->" << tmp << " " << pqmod << '\n';
    }
    return res;
  }

  // p and q assumed to have the same size, gcd(pmod,qmod)=1
  bool ichinrem_inplace(dense_POLY1 &p,const dense_POLY1 & q,const gen & pmod,int qmodval){
    if (debug_infolevel>2)
      CERR << CLOCK()*1e-6 << " ichinrem begin "<< p.size() << '\n';
    gen u,v,d,tmp,pqmod(qmodval*pmod),pqmod2=iquo(pqmod,2),minuspqmod2=-pqmod2;
    egcd(pmod,qmodval,u,v,d);
    if (u.type==_ZINT)
      u=modulo(*u._ZINTptr,qmodval);
    int U=u.val;
    if (d==-1){ u=-u; v=-v; d=1; }
    if (d!=1)
      return false;
    if (pmod.type!=_ZINT)
      return false;
    dense_POLY1::iterator a = p.begin(),a_end = p.end();
    dense_POLY1::const_iterator b = q.begin(),b_end = q.end();
    int n=int(a_end-a), m=int(b_end-b);
    if (n!=m)
      return false;
    mpz_t tmpz;
    mpz_init(tmpz);
    for (;a!=a_end;++a,++b){
      // smod(*a+((u*(*b-*a))%qmod)*pmod,pqmod)
#ifndef USE_GMP_REPLACEMENTS
      if (a->type==_ZINT){
#if 1
	int amodq=modulo(*a->_ZINTptr,qmodval);
	if (amodq==b->val)
	  continue;
	mpz_mul_si(tmpz,*pmod._ZINTptr,(U*(b->val-longlong(amodq)))%qmodval);
	mpz_add(tmpz,tmpz,*a->_ZINTptr);	  
#else
	mpz_set_si(tmpz,b->val);
	mpz_sub(tmpz,tmpz,*a->_ZINTptr);
	mpz_mul_si(tmpz,*pmod._ZINTptr,(longlong(U)*modulo(tmpz,qmodval))%qmodval);
	mpz_add(tmpz,tmpz,*a->_ZINTptr);
#endif
      }
      else {
	mpz_mul_si(tmpz,*pmod._ZINTptr,(U*(longlong(b->val)-a->val))%qmodval);
	if (a->val>=0)
	  mpz_add_ui(tmpz,tmpz,a->val);
	else
	  mpz_sub_ui(tmpz,tmpz,-a->val);
      }
      if (mpz_cmp(tmpz,*pqmod2._ZINTptr)>=0)
	mpz_sub(tmpz,tmpz,*pqmod._ZINTptr);
      else {
	if (mpz_cmp(tmpz,*minuspqmod2._ZINTptr)<=0)
	  mpz_add(tmpz,tmpz,*pqmod._ZINTptr);
      }
      if (a->type==_ZINT) mpz_set(*a->_ZINTptr,tmpz); else *a=tmpz;
#else
      *a=*a+u*(*b-*a) *pmod ; // improve to modulo(U*(*b-*a), qmodval) and type checking for overwrite
      *a = smod(*a,pqmod);
#endif
    }
    mpz_clear(tmpz);
    if (debug_infolevel>2)
      CERR << CLOCK()*1e-6 << " ichinrem end "<< p.size() << '\n';
    return true;
  }

  void ichinremp1p2(const vector<int> & resp1,const vector<int> & resp2,size_t rs,vecteur & pq,int nbits){
    pq.clear();
    const int p1modinv=-9;//invmod(p1,p2);
    for (size_t i=0;i<rs;++i){
      //int A=pq[i].val,B=curres[i].val;
      int A=resp1[i],B=resp2[i];
      // A mod p1, B mod p2 -> res mod p1*p2
      longlong res=A+((longlong(p1modinv)*(B-A))%p2)*p1;
      if (res>p1p2sur2) res-=p1p2;
      else if (res<-p1p2sur2) res+=p1p2;
      pq.push_back(gen(res,nbits)); // pq[i]=res;
    }
  }

  bool ichinrem(const vector<int> & p,const vector<int> & q,int pmod,int qmod,int zsize,bool dosmod,vecteur & res){
    vector<int>::const_iterator a = p.begin(),a_end = p.end();
    vector<int>::const_iterator b = q.begin(),b_end = q.end();
    int u,v;
    if ( (a_end-a!=b_end-b) || iegcd(pmod,qmod,u,v)!=1)
      return false;
    res.reserve(a_end-a);
    res.clear();
    longlong pqmod=longlong(pmod)*qmod;
    for (;a!=a_end;++a,++b){
      // smod(*a+((u*(*b-*a))%qmod)*pmod,pqmod)
      longlong r=*a+ ( (u*(longlong(*b)-*a)) %qmod) *pmod ; 
      r -= (r>>63)*pqmod;
      if (dosmod && r>pqmod/2)
	r-=pqmod;
      res.push_back(gen(r,zsize));
    }    
    return true;
  }

  // p and q assumed to have the same size, gcd(pmod,qmod)=1
  // returns 0 on error, 1 if p has changed, 2 if p is unchanged
  int ichinrem_inplace(dense_POLY1 &p,const vector<int> & q,const gen & pmod,int qmodval,int reserve_mem){
    if (debug_infolevel>2)
      CERR << CLOCK()*1e-6 << " ichinrem_inplace begin deg "<< p.size() << '\n';
    gen u,v,d,tmp,pqmod(qmodval*pmod),pqmod2=iquo(pqmod,2),minuspqmod2=-pqmod2;
    egcd(pmod,qmodval,u,v,d);
    if (u.type==_ZINT)
      u=modulo(*u._ZINTptr,qmodval);
    if (d==-1){ u=-u; v=-v; d=1; }
    int U=u.val;
    if (d!=1)
      return 0;
    gen pmod_(pmod); 
    pmod_.uncoerce();
    dense_POLY1::iterator a = p.begin(),a_end = p.end();
    vector<int>::const_iterator b = q.begin(),b_end = q.end();
    int n=int(a_end-a), m=int(b_end-b);
    if (n!=m)
      return 0;
    bool changed=false;
    mpz_t tmpz;
    if (reserve_mem)
      mpz_init2(tmpz,reserve_mem);
    else
      mpz_init(tmpz);
    for (;a!=a_end;++a,++b){
      // smod(*a+((u*(*b-*a))%qmod)*pmod,pqmod)
#ifndef USE_GMP_REPLACEMENTS
      if (a->type==_ZINT){
	int amodq=modulo(*a->_ZINTptr,qmodval);
	if (amodq==*b)
	  continue;
	int ab=(U*(*b-longlong(amodq)))%qmodval;
	if (ab==0)
	  continue;
	changed=true;
	mpz_mul_si(tmpz,*pmod_._ZINTptr,ab);
	mpz_add(tmpz,tmpz,*a->_ZINTptr);	  
      }
      else {
	int ab=(U*(longlong(*b)-a->val))%qmodval;
	if (ab==0)
	  continue;
	changed=true;
	mpz_mul_si(tmpz,*pmod_._ZINTptr,ab);
	if (a->val>=0)
	  mpz_add_ui(tmpz,tmpz,a->val);
	else
	  mpz_sub_ui(tmpz,tmpz,-a->val);
      }
      if (mpz_cmp(tmpz,*pqmod2._ZINTptr)>0)
	mpz_sub(tmpz,tmpz,*pqmod._ZINTptr);
      else {
	if (mpz_cmp(tmpz,*minuspqmod2._ZINTptr)<=0)
	  mpz_add(tmpz,tmpz,*pqmod._ZINTptr);
      }
      // && a->ref_count()==1 ?
      if (a->type==_ZINT) 
	mpz_set(*a->_ZINTptr,tmpz); 
      else 
	*a=tmpz;
#else
      *a=*a+u*(*b-*a) *pmod ; // improve to modulo(U*(*b-*a), qmodval) and type checking for overwrite
      *a = smod(*a,pqmod);
#endif
    }
    mpz_clear(tmpz);
    if (debug_infolevel>2)
      CERR << CLOCK()*1e-6 << " ichinrem_inplace end deg "<< p.size() << '\n';
    return changed?1:2;
  }

  // assuming pmod and qmod are prime together, find r such that
  // r = p mod pmod  and r = q mod qmod
  // hence r = p + A*pmod = q + B*qmod
  // or A*pmod -B*qmod = q - p
  // assuming u*pmod+v*qmod=d we get
  // A=u*(q-p)/d
  modpoly chinrem(const modpoly & p,const modpoly & q, const modpoly & pmod, const modpoly & qmod,environment * env){
    modpoly u,v,d,r;
    egcd(pmod,qmod,env,u,v,d);
    r=operator_plus(p,operator_times(operator_times(u,operator_div(operator_minus(q,p,env),d,env),env),pmod,env),env);
    if (r.size()>=pmod.size()+qmod.size()-1)
      r=operator_mod(r,operator_times(pmod,qmod,env),env);
    return r;
  }

  void divided_differences(const vecteur & x,vecteur & res,environment * env,bool divexact){
    int s=int(x.size());
    for (int k=1;k<s;++k){
      if (env && env->moduloon){
	for (int j=s-1;j>=k;--j){
	  res[j]=smod((res[j]-res[j-1])*invmod(x[j]-x[j-k],env->modulo),env->modulo);
	}
      }
      else {
	for (int j=s-1;j>=k;--j){
	  gen & g=res[j];
	  operator_minus_eq(g,res[j-1],context0);
	  gen dx(x[j]-x[j-k]);
#ifndef USE_GMP_REPLACEMENTS
	  if (divexact && g.type==_ZINT && g.ref_count()==1 && dx.type==_INT_){
	    mpz_t * z=g._ZINTptr;
	    if (dx.val>0)
	      mpz_divexact_ui(*z,*z,dx.val);
	    else {
	      mpz_divexact_ui(*z,*z,-dx.val);
	      mpz_neg(*z,*z);
	    }
	  }
	  else
#endif
	    g=g/dx;
	}
      }
    }
  }

  void divided_differences(const vecteur & x,const vecteur & y,vecteur & res,environment * env){
    res=y;
    divided_differences(x,res,env,false);
  }

  void interpolate(const vecteur & x,const vecteur & y,modpoly & res,environment * env){
    vecteur alpha;
    divided_differences(x,y,alpha,env);
    unsigned s=unsigned(x.size());
    res.clear();
    res.reserve(s);
    int j=s-1;
    res.push_back(alpha[j]);
    for (j--;j>=0;j--){
      res.push_back(alpha[j]);
      iterateur it=res.end()-2,itbeg=res.begin()-1;
      const gen & fact = x[j];
      for (;it!=itbeg;it-=2){
	gen & tmp = *it;
	++it;
	*it -= tmp*fact;
	if (env && env->moduloon)
	  *it=smod(*it,env->modulo);
      }
    }
  }

  void interpolate_inplace(const vecteur & x,modpoly & res,environment * env){
    divided_differences(x,res,env,true);
    unsigned s=unsigned(x.size());
    int j=s-1;
    reverse(res.begin(),res.end());
    for (j--;j>=0;j--){
      iterateur it=res.begin()+(s-2-j),itbeg=res.begin()-1;
      const gen & fact = x[j];
      for (;it!=itbeg;it-=2){
	gen & tmp = *it;
	++it;
	type_operator_minus_times(tmp,fact,*it); // *it -= tmp*fact;
	if (env && env->moduloon)
	  *it=smod(*it,env->modulo);
      }
    }
  }

  // Multiplication of multivariate polynomials using Lagrange interpolation
  void mulpoly_interpolate(const polynome & p,const polynome & q,polynome & res,environment * env){
    int s=p.dim;
    gen modulo;
    if (env &&env->moduloon)
      modulo=env->modulo;
    if (s<2){
      mulpoly(p,q,res,modulo);
      return;
    }
    bool estreel=poly_is_real(p) && poly_is_real(q);
    polynome pxn,qxn;
    convert_xn(p,pxn);
    convert_xn(q,qxn);
    int pd=p.degree(s-1);
    int qd=q.degree(s-1);
    int sd=pd+qd;
    vecteur x(sd+1);
    vecteur y(sd+1);
    modpoly v;
    index_t * degptr=0;
    for (int i=0;i<=sd;++i){
      x[i]=i;
      y[i]=new ref_polynome(s);
      mulpoly_interpolate(pevaln(pxn,i,modulo,degptr,estreel),pevaln(qxn,i,modulo,degptr,estreel),*y[i]._POLYptr,env);
    }
    interpolate(x,y,v,env);
    poly12polynome(v,s,res,s);
  }

  int vect_polynome2poly1(vecteur & A){
    int dim=0;
    for (size_t i=0;i<A.size();++i){
      if (A[i].type==_POLY){
	dim=A[i]._POLYptr->dim;
	A[i]=gen(polynome2poly1(*A[i]._POLYptr,1),_POLY1__VECT);
      }
    }
    return dim;
  }

  void vect_poly12polynome(vecteur & v,int dim){
    iterateur it=v.begin(),itend=v.end();
    for (;it!=itend;++it){
      if (it->type==_VECT)
	*it=poly12polynome(*it->_VECTptr,1,dim);
    }
  }

  void mat_poly12polynome(matrice & A,int dim){
    iterateur it=A.begin(),itend=A.end();
    for (;it!=itend;++it){
      if (it->type==_VECT)
	vect_poly12polynome(*it->_VECTptr,dim);
    }
  }

  void vect_horner(const vecteur & v,const gen & g,vecteur & res){
    res=v;
    iterateur it=res.begin(),itend=res.end();
    for (;it!=itend;++it)
      if (it->type==_VECT)
	*it=horner(*it->_VECTptr,g);
  }

  // compute dotvecteur of a and b by interpolation if it would be faster
  // 1-d interpolation cost : D*M+D^2
  // where D=max(size(a[i])+size(b[i])-1), M=min(size(a),size(b))
  // normal cost: sum_i(size(a[i])*size(b[i]))
  // if a and b are of length n and degree n, interp cost is O(n^2)
  // while normal cost is O(n^3)
  // Beware: this is not interesting in characteristic 0 because
  // we replace n-deg polynomials with integers of size n*ln(n)
  bool dotvecteur_interp(const vecteur & a,const vecteur &b,gen & res){
    if (a.empty() || b.empty()){
      res=0; return true;
    }
    if (a.front().type==_POLY || b.front().type==_POLY){
      vecteur A(a), B(b); int dim;
      if (!(dim=vect_polynome2poly1(A)) || dim!=vect_polynome2poly1(B))
	return false;
      if (dotvecteur_interp(A,B,res)){
	if (res.type==_VECT) res=poly12polynome(*res._VECTptr,1,dim);
	return true;
      }
      return false;
    }
    if (a.front().type==_VECT || b.front().type==_VECT){
      int D=0,M=giacmin(int(a.size()),int(b.size()));
      double interpcost=0.0,normalcost=0.0;
      for (int i=0;i<M;++i){
	int as=1,bs=1;
	if (a[i].type==_VECT) as=int(a[i]._VECTptr->size());
	if (b[i].type==_VECT) bs=int(b[i]._VECTptr->size());
	if (D<as+bs-1) D=as+bs-1;
	normalcost += as*bs;
      }
      if (normalcost<D*(M+D))
	return false;
      // now do the real work!
      int shift=-D/2;
      vecteur X(D),Y(D),A(M),B(M);
      for (int j=0;j<D;++j){
	X[j]=j-shift;
	for (int i=0;i<M;++i){
	  A[i]=horner(a[i],j-shift);
	  B[i]=horner(b[i],j-shift);
	}
	Y[j]=dotvecteur(A,B);
      }
      vecteur R;
      interpolate(X,Y,R,0);
      res=R;
      return true;
    }
    return false;
  }

  // R is a degree D-1 polynomial of MxN matrices, 
  // rebuild a matrix of polynomials
  void polymat2matpoly(const vecteur & R,vecteur & res){
    if (R.empty()) return;
    int M,N,D=int(R.size());
    mdims(*R[0]._VECTptr,M,N);
    // init res
    res.resize(M);
    for (int i=0;i<M;++i){
      res[i]=vecteur(N);
      vecteur & resi=*res[i]._VECTptr;
      for (int j=0;j<N;++j)
	resi[j]=vecteur(D);
    }
    // modify in place
    for (int d=0;d<D;++d){
      vecteur & md=*R[d]._VECTptr;
      for (int i=0;i<M;++i){
	vecteur & resi=*res[i]._VECTptr;
	vecteur &mdi=*md[i]._VECTptr;
	for (int j=0;j<N;++j){
	  vecteur & resij=*resi[j]._VECTptr;
	  resij[d]=mdi[j];
	}
      }
    }
    for (int i=0;i<M;++i){
      vecteur & resi=*res[i]._VECTptr;
      for (int j=0;j<N;++j){
	trim(*resi[j]._VECTptr);
      }
    }
  }

  // warning b is already transposed
  bool mmult_interp(const matrice & a,const matrice &b,matrice & res){
    if (a.front()[0].type==_POLY || b.front()[0].type==_POLY){
      matrice A(a), B(b);
      int S=giacmin(int(A.size()),int(B.size())),dim=0;
      for (int i=0;i<S;++i){
	if (A[i].type!=_VECT || B[i].type!=_VECT) return false;
	A[i]=*A[i]._VECTptr;
	B[i]=*B[i]._VECTptr;
	if (!(dim=vect_polynome2poly1(*A[i]._VECTptr)) || dim!=vect_polynome2poly1(*B[i]._VECTptr))
	  return false;
      }
      if (mmult_interp(A,B,res)){
	mat_poly12polynome(res,dim);
	return true;
      }
      return false;
    }
    if (a.front()[0].type==_VECT || b.front()[0].type==_VECT){
      // find required degree
      int D=0,M=giacmin(int(a.size()),int(b.size())),N=0;
      for (int i=0;i<M;++i){
	gen ai=a[i],bi=b[i];
	if (ai.type!=_VECT || bi.type!=_VECT)
	  return false;
	vecteur av=*ai._VECTptr,bv=*bi._VECTptr;
	N=giacmin(int(av.size()),int(bv.size()));
	for (int j=0;j<N;++j){
	  int as=1,bs=1;
	  if (av[j].type==_VECT) as=int(av[j]._VECTptr->size());
	  if (bv[j].type==_VECT) bs=int(bv[j]._VECTptr->size());
	  if (D<as+bs-1) D=as+bs-1;
	}
      }
      // do the real work!
      int shift=D/2;
      vecteur X(D),Y(D),A(M),B(M);
      for (int j=0;j<D;++j){
	X[j]=j-shift;
	for (int i=0;i<M;++i){
	  vecteur tmp;
	  vect_horner(*a[i]._VECTptr,j-shift,tmp);
	  A[i]=tmp;
	  vect_horner(*b[i]._VECTptr,j-shift,tmp);
	  B[i]=tmp;
	}
	vecteur tmp;
	mmult_atranb(A,B,tmp);
	Y[j]=tmp;
      }
      vecteur R;
      interpolate(X,Y,R,0);
      polymat2matpoly(R,res);
      return true;
    }
    return false;
  }

  bool do_pcar_interp(const matrice & a,vecteur & p,bool compute_pmin,GIAC_CONTEXT){
    if (a.front()[0].type==_POLY){
      matrice A(a);
      int S=int(A.size()),dim=0;
      for (int i=0;i<S;++i){
	if (A[i].type!=_VECT) return false;
	A[i]=*A[i]._VECTptr;
	if (!(dim=vect_polynome2poly1(*A[i]._VECTptr)))
	  return false;
      }
      if (!do_pcar_interp(A,p,compute_pmin,contextptr))
	return false;
      vect_poly12polynome(p,dim);
      return true;
    }
    if (a.front()[0].type==_VECT){
      // find required number of interpolations
      int D=0,M=int(a.size()),N=0;
      for (int i=0;i<M;++i){
	gen ai=a[i];
	if (ai.type!=_VECT)
	  return false;
	vecteur av=*ai._VECTptr;
	N=int(av.size());
	for (int j=0;j<N;++j){
	  int as=1;
	  if (av[j].type==_VECT) as=int(av[j]._VECTptr->size());
	  if (D<as-1) D=as-1;
	}
      }
      int Dorig=D;
      D = M*D+1;
      // do the real work!
      int shift=-D/2;
      vecteur X(D),Y(D),A(M);
      int resdegp1=M+1;
      for (int j=0;j<D;++j,++shift){
	for (int i=0;i<M;++i){
	  vecteur tmp;
	  vect_horner(*a[i]._VECTptr,shift,tmp);
	  A[i]=tmp;
	}
	gen tmp;
	if (compute_pmin)
	  tmp=_pmin(A,contextptr);
	else
	  tmp=_pcar(A,contextptr);
	if (tmp.type!=_VECT)
	  return false;
	int tmpd=int(tmp._VECTptr->size());
	if (!j) resdegp1=tmpd;
	if (tmpd==resdegp1){
	  X[j]=shift;
	  Y[j]=tmp;
	  if (j==resdegp1*Dorig){
	    D=j+1;
	    break;
	  }
	  continue;
	}
	if (tmpd<resdegp1) // bad reduction, pmin degree is too small
	  continue;
	// tmpd>resdegp1, previous pmin were bad reduction, restart
	j=0;
	X[j]=shift;
	Y[j]=tmp;
      }
      vecteur R;
      X.resize(D); Y.resize(D); // early termination
      // pmin(a)==0 because it's a matrix with polynomial coeffs 
      // in the parameter of degree < D and it is 0 for D values
      // of the parameter
      interpolate(X,Y,R,0);
      // R is a polynomial of pmins, we must rebuild a pmin of polynomials
      // init res
      vecteur & res=p;
      res.resize(resdegp1);
      for (int i=0;i<resdegp1;++i){
	res[i]=gen(vecteur(D),_POLY1__VECT);
      }
      // modify in place
      for (int d=0;d<D;++d){
	if (R[d].type!=_VECT)
	  continue;
	vecteur & md=*R[d]._VECTptr;
	int shift=resdegp1-int(md.size());
	for (int i=shift;i<resdegp1;++i){
	  vecteur & resi=*res[i]._VECTptr;
	  resi[d]=md[i-shift];
	}
      }
      for (int i=0;i<res.size();++i){
	vecteur & resi=*res[i]._VECTptr;
	trim(resi);
      }
      return true;
    }
    return false;
  }

  bool poly_pcar_interp(const matrice & a,vecteur & p,bool compute_pmin,GIAC_CONTEXT){
    if (a.empty()) return false;
    if (a[0][0].type==_POLY || a[0][0].type==_VECT){
      if (!do_pcar_interp(a,p,compute_pmin,contextptr))
	return false;
      return true;
    }
    vecteur lv=alg_lvar(a);
    if (lv.empty())
      return false;
    matrice A=*(e2r(a,lv,contextptr)._VECTptr);
    for (int i=0;i<A.size();++i){
      gen Ai=A[i];
      if (Ai.type!=_VECT) return false;
      const_iterateur it=Ai._VECTptr->begin(),itend=Ai._VECTptr->end();
      for (;it!=itend;++it){
	if (it->type==_FRAC && it->_FRACptr->den.type==_POLY)
	  return false;
      }
    }
    // extract common denominator
    vecteur Aflat; gen d;
    aplatir(A,Aflat);
    const_iterateur jt=Aflat.begin();
    lcmdeno(Aflat,d,contextptr);
    for (int i=0;i<A.size();++i){
      gen Ai=A[i];
      if (Ai.type!=_VECT) return false;
      iterateur it=Ai._VECTptr->begin(),itend=Ai._VECTptr->end();
      for (;it!=itend;++it,++jt){
	*it=*jt;
      }
    }
    if (!do_pcar_interp(A,p,compute_pmin,contextptr))
      return false;
    // eigenvalues of A are lambda/d, 
    // we must scale p by d, leading coeff does not change, then /d, etc.
    gen powd=1;
    for (int i=0;i<p.size();++i){
      p[i]=r2e(p[i]/powd,lv,contextptr);
      powd=powd*d;
    }
    return true;
  }

  // n <- n mod N where N=2^expoN+1
  // n=q*(N-1)+r => n=q*N+(r-q)
  void smod2N(longlong & n,unsigned long expoN,bool do_smod){
    if (n<0){
      n=-n;
      smod2N(n,expoN,do_smod);
      n=-n;
      return;
    }
    longlong q = n >> expoN;
    if (q){
      n -=  q << expoN;
      n -= q;
    }
    if (n>0){
      q = n >> expoN;
      if (q){
	n -=  q << expoN;
	n -= q;
      }
    }
    if (!do_smod)
      return;
    if (n<0){
      q = (-n) >> (expoN-1);
      n += q+(q << expoN);
    }
    else {
      q = n >> (expoN-1);
      n -= q + (q << expoN);
    }
  }

  // replace g in-place by g mod N where N=2^expoN+1
  // if do_smod==true, returns g in [-N/2-1,N/2+1]
  void smod2N(gen & g,unsigned long expoN,mpz_t tmpqz,bool do_smod=false){
    if (g.type!=_ZINT){
      if (expoN<31){
	longlong n=g.val;
	smod2N(n,expoN,do_smod);
	g.val = n;
      }
      return;
    }
    mpz_t & z=*g._ZINTptr;
    mpz_tdiv_q_2exp(tmpqz,z,expoN);
    mpz_tdiv_r_2exp(z,z,expoN);
    mpz_sub(z,z,tmpqz);
    mpz_tdiv_q_2exp(tmpqz,z,expoN);
    mpz_tdiv_r_2exp(z,z,expoN);
    mpz_sub(z,z,tmpqz); 
    if (!do_smod)
      return;
    mpz_tdiv_q_2exp(tmpqz,z,expoN-1);
    mpz_sub(z,z,tmpqz);
    mpz_mul_2exp(tmpqz,tmpqz,expoN);
    mpz_sub(z,z,tmpqz);
  }

  void shift2N(gen & tmp,unsigned long shift){
    if (tmp.type==_INT_ ){
      if (shift<31){
	if (tmp.val<0)
	  tmp = -(longlong(-tmp.val)<<shift);
	else
	  tmp = longlong(tmp.val) << shift;
      }
      else {
	tmp.uncoerce();
	mpz_mul_2exp(*tmp._ZINTptr,*tmp._ZINTptr,shift);
      }
    }
    else {
      if (tmp.ref_count()!=1)
	tmp=*tmp._ZINTptr; // make a copy
      mpz_mul_2exp(*tmp._ZINTptr,*tmp._ZINTptr,shift);
    }
  }

  // z1 <- z1*2^(expoN-shift) mod 2^expoN+1
  void shiftsmod2N(mpz_t & z1,int expoN,int shift,mpz_t & tmpqz,bool do_smod=false){
    mpz_tdiv_q_2exp(tmpqz,z1,expoN-shift);
    mpz_tdiv_r_2exp(z1,z1,expoN-shift);
    mpz_mul_2exp(z1,z1,shift);
    mpz_sub(z1,z1,tmpqz);
    mpz_tdiv_q_2exp(tmpqz,z1,expoN);
    if (mpz_cmp_si(tmpqz,0)){      
      mpz_tdiv_r_2exp(z1,z1,expoN);
      mpz_sub(z1,z1,tmpqz); 
    }
    if (!do_smod)
      return;
    mpz_tdiv_q_2exp(tmpqz,z1,expoN-1);
    if (mpz_cmp_si(tmpqz,0)){
      mpz_sub(z1,z1,tmpqz);
      mpz_mul_2exp(tmpqz,tmpqz,expoN);
      mpz_sub(z1,z1,tmpqz);
    }
  }
  
  // Fast Fourier Transform, f the poly sum_{j<n} f_j x^j,
  // first call omega=2^r, omega is a 2^(l+1)-root of unity
  // computation is done modulo N=2^{r*2^(l)}+1
  // recursive calls will replace r by r*2^k with k<=l
  // return [f(1),f(omega),...,f(omega^[n-1]) 
  // WARNING f is given in ascending power
  // this version assumes that all integers are free _ZINT
  void fft2rl(gen * f,long n,int r,int l,gen * t,bool direct,gen & tmp1, gen & tmp2,mpz_t & tmpqz){
    if (n==1) return;
    unsigned long expoN = r<<l;
    if (n==2){
      mpz_add(*tmp1._ZINTptr,*f[0]._ZINTptr,*f[1]._ZINTptr);
      smod2N(*tmp1._ZINTptr,expoN,tmpqz);
      mpz_sub(*tmp2._ZINTptr,*f[0]._ZINTptr,*f[1]._ZINTptr);
      smod2N(*tmp2._ZINTptr,expoN,tmpqz);
      mpz_set(*f[0]._ZINTptr,*tmp1._ZINTptr);
      mpz_set(*f[1]._ZINTptr,*tmp2._ZINTptr);
      return;
    }
    // gen F0,F1,F2,F3;
    if (n==4){
      mpz_t & z1=*tmp1._ZINTptr;
      mpz_t & z2=*tmp2._ZINTptr;
      mpz_add(z1,*f[0]._ZINTptr,*f[2]._ZINTptr); // z1=f0+f2
      mpz_add(z2,*f[1]._ZINTptr,*f[3]._ZINTptr); // z2=f1+f3
      mpz_add(*t[0]._ZINTptr,z1,z2); // t0=f0+f1+f2+f3
      mpz_sub(*t[2]._ZINTptr,z1,z2); // t2=f0-f1+f2-f3
      mpz_sub(z1,*f[1]._ZINTptr,*f[3]._ZINTptr); // z1=f1-f3
      shiftsmod2N(z1,expoN,expoN/2,z2); // z1=(f1-f3)*w
      mpz_sub(z2,*f[0]._ZINTptr,*f[2]._ZINTptr); // z2=f0-f2
      if (direct){
	mpz_add(*f[1]._ZINTptr,z2,z1); // f1=f0-f2+(f1-f3)*w
	mpz_sub(*f[3]._ZINTptr,z2,z1); // f3=f0-f2-(f1-f3)*w
      }
      else {
	mpz_add(*f[3]._ZINTptr,z2,z1); // f3=f0-f2+(f1-f3)*w
	mpz_sub(*f[1]._ZINTptr,z2,z1); // f1=f0-f2-(f1-f3)*w
      }
      // F0=*t[0]._ZINTptr; F1=*t[1]._ZINTptr; F2=*t[2]._ZINTptr; F3=*t[3]._ZINTptr;
      mpz_set(*f[0]._ZINTptr,*t[0]._ZINTptr);
      mpz_set(*f[2]._ZINTptr,*t[2]._ZINTptr);
      return; // not full reduced mod N
    }
    unsigned long m=1<<(l+1);
    long step=r*(direct?m/n:-long(m/n)); // step is a power of 2
    gen * r0=t,*r1=t+n/2; 
    gen * it=f,*itn=it+n/2,*itend=itn;
    unsigned long shift=direct?0:expoN;
    // first step with 0 shift
    mpz_add(*r0->_ZINTptr,*it->_ZINTptr,*itn->_ZINTptr);
    smod2N(*r0->_ZINTptr,expoN,tmpqz);
    mpz_sub(*r1->_ZINTptr,*it->_ZINTptr,*itn->_ZINTptr);
    smod2N(*r1->_ZINTptr,expoN,tmpqz);
    for (++itn,shift+=step,++it,++r0,++r1;it!=itend;++itn,shift+=step,++it,++r0,++r1){
      mpz_t & z0=*tmp1._ZINTptr;
      mpz_t & z1=*tmp2._ZINTptr;
      mpz_add(z0,*it->_ZINTptr,*itn->_ZINTptr);
      if (mpz_sizeinbase(z0,2)>=expoN)
	smod2N(z0,expoN,tmpqz);
      mpz_set(*r0->_ZINTptr,z0);
      if (direct)
	mpz_sub(z1,*it->_ZINTptr,*itn->_ZINTptr);
      else
	mpz_sub(z1,*itn->_ZINTptr,*it->_ZINTptr);
#if 1
      shiftsmod2N(z1,expoN,shift,tmpqz);
#else
      mpz_mul_2exp(z1,z1,shift);
      smod2N(z1,expoN,tmpqz);
#endif
      mpz_set(*r1->_ZINTptr,z1);
    }
    // Recursive calls
    gen * r0f=f, * r1f=f+n/2;
    fft2rl(t,n/2,r,l,r0f,direct,tmp1,tmp2,tmpqz);
    fft2rl(t+n/2,n/2,r,l,r1f,direct,tmp1,tmp2,tmpqz);
    // Return a mix of r0/r1
    it=t; itend=it+n/2; itn=t+n/2;
#ifdef USE_GMP_REPLACEMENTS
    for (;it!=itend;){
      mpz_set(tmpqz,*it->_ZINTptr);
      mpz_set(*it->_ZINTptr,*f->_ZINTptr);
      mpz_set(*f->_ZINTptr,tmpqz);
      ++it; ++f;
      mpz_set(tmpqz,*itn->_ZINTptr);
      mpz_set(*itn->_ZINTptr,*f->_ZINTptr);
      mpz_set(*f->_ZINTptr,tmpqz);
      ++itn; ++f;
    }
#else
    for (;it!=itend;){
      mpz_swap(*f->_ZINTptr,*it->_ZINTptr);
      ++it; ++f;
      mpz_swap(*f->_ZINTptr,*itn->_ZINTptr);
      ++itn; ++f;
    }
#endif
    // if (n==4 && (f[-4]!=F0 || f[-3]!=F1 || f[-2]!=F2 || f[-1]!=F3)) COUT << "err" << '\n';
  }

  // Fast Fourier Transform, f the poly sum_{j<n} f_j x^j,
  // first call omega=2^r, omega is a 2^(l+1)-root of unity
  // computation is done modulo N=2^{r*2^(l)}+1
  // recursive calls will replace r by r*2^k with k<=l
  // return [f(1),f(omega),...,f(omega^[n-1]) 
  // WARNING f is given in ascending power
  void fft2rl(const modpoly & f,int r,int l,modpoly & res,bool direct,mpz_t & tmpqz){
    unsigned long n=f.size();
    unsigned long expoN = r<<l;
    if (1
	//&& expoN>30
	){
      modpoly F(f);res.clear(); res.resize(n); // free copy of F
      for (size_t i=0;i<n;++i){
	if (F[i].type==_INT_)
	  F[i].uncoerce(expoN+1);
	else
	  F[i]=*F[i]._ZINTptr;
	res[i].uncoerce(expoN+1);
      }
      gen tmp1,tmp2; tmp1.uncoerce(); tmp2.uncoerce();
      fft2rl(&F.front(),n,r,l,&res.front(),direct,tmp1,tmp2,tmpqz);
      F.swap(res);
      return;
    }
    if (n==1) return;
    if (n==2){
      gen tmp=f[0]+f[1];
      smod2N(tmp,expoN,tmpqz);
      res[0]=tmp;
      tmp=f[0]-f[1];
      smod2N(tmp,expoN,tmpqz);
      res[1]=tmp;
      return;
    }
    unsigned long m=1<<(l+1);
    long step=r*(direct?m/n:-long(m/n)); // step is a power of 2
    modpoly r0,r1; r0.reserve(n/2); r1.reserve(n/2);
    const_iterateur it=f.begin(),itn=it+n/2,itend=itn;
    unsigned long shift=direct?0:expoN;
    gen tmp; 
    // first step with 0 shift
    tmp=*it+(*itn);
    smod2N(tmp,expoN,tmpqz);
    r0.push_back(tmp);
    tmp=(*it)-(*itn);
    //if (!direct) tmp=-tmp;
    smod2N(tmp,expoN,tmpqz);
    r1.push_back(tmp);
    for (++itn,shift+=step,++it;it!=itend;++itn,shift+=step,++it){
      tmp=(*it)+(*itn);
      smod2N(tmp,expoN,tmpqz);
      r0.push_back(tmp);
      if (direct)
	tmp = (*it)-(*itn);
      else
	tmp = (*itn)-(*it);
      shift2N(tmp,shift);
      smod2N(tmp,expoN,tmpqz);
      r1.push_back(tmp);
    }
    // Recursive calls
    modpoly r0f(n/2),r1f(n/2);
    fft2rl(r0,r,l,r0f,direct,tmpqz);
    fft2rl(r1,r,l,r1f,direct,tmpqz);
    // Return a mix of r0/r1
    res.clear();
    res.reserve(n);
    it=r0f.begin(); itend=it+n/2; itn=r1f.begin();
    for (;it!=itend;){
      res.push_back(*it);
      ++it;
      res.push_back(*itn);
      ++itn;
    }
  }

  // alpha[i] *= beta[i] mod 2^(expoN)+1
  void fft2rltimes(modpoly & alpha,const modpoly & beta,unsigned long expoN,mpz_t & tmp,mpz_t & tmpqz){
    int n=alpha.size();
    for (unsigned long i=0;i<n;++i){
      if (alpha[i].type==_ZINT && beta[i].type==_ZINT){
	mpz_mul(tmp,*alpha[i]._ZINTptr,*beta[i]._ZINTptr);
	smod2N(tmp,expoN,tmpqz);
	mpz_set(*alpha[i]._ZINTptr,tmp);
      }
      else {
	type_operator_times(alpha[i],beta[i],alpha[i]); // alpha[i]=alpha[i]*beta[i];
	smod2N(alpha[i],expoN,tmpqz);
      }
    }
  }

  // alpha[i] *= beta[i] mod 2^(expoN)+1
  void fft2rltimes(const modpoly & alpha,const modpoly & beta,modpoly & res,unsigned long expoN,mpz_t & tmp,mpz_t & tmpqz){
    int n=alpha.size();
    for (unsigned long i=0;i<n;++i){
      if (alpha[i].type==_ZINT && beta[i].type==_ZINT){
	mpz_mul(tmp,*alpha[i]._ZINTptr,*beta[i]._ZINTptr);
	smod2N(tmp,expoN,tmpqz);
	mpz_set(*res[i]._ZINTptr,tmp);
      }
      else 
	COUT << "fft2rltimes type error" << '\n';
    }
  }

  // pq *= -2^shift mod N=2^(expoN+1) where -2^shift is the inverse of n mod N
  void fft2rldiv(modpoly & pq,unsigned long expoN,unsigned long shift,mpz_t & tmp,mpz_t & tmpqz){
    int n=pq.size();
    for (unsigned long i=0;i<n;++i){
      // pq[i]=-pq[i];
      if (pq[i].type==_INT_)
	mpz_set_si(tmp,-pq[i].val);
      else
	mpz_neg(tmp,*pq[i]._ZINTptr);
#if 1
      shiftsmod2N(tmp,expoN,shift,tmpqz,true);
#else
      mpz_mul_2exp(tmp,tmp,shift);
      smod2N(tmp,expoN,tmpqz,true);
#endif
      if (mpz_sizeinbase(tmp,2)<31)
	pq[i]=mpz_get_si(tmp);
      else {
	if (pq[i].type==_ZINT)
	  mpz_set(*pq[i]._ZINTptr,tmp);
	else
	  pq[i]=tmp;
      }
    }
    reverse(pq.begin(),pq.end());
    trim_inplace(pq);
  }

  void fftprod2rl(const modpoly & p0,const modpoly &q0,int r,int l,modpoly & pq){
    if (debug_infolevel>3)
      CERR << CLOCK()*1e-6 << " fftmult 2^r as a 2^l-root r=" << r << " l=" << l << '\n' ; 
    mpz_t tmp,tmpqz; mpz_init(tmp); mpz_init(tmpqz);
    unsigned long expoN=r << l; // r*2^l
    unsigned long n=1<<(l+1);
    modpoly p(p0),q(q0);
    reverse(p.begin(),p.end());
    reverse(q.begin(),q.end());
    unsigned long ps=long(p.size()),qs=long(q.size());
    for (unsigned long i=ps;i<n;++i)
      p.push_back(0);
    for (unsigned long i=qs;i<n;++i)
      q.push_back(0);
    modpoly alpha(n),beta(n);
    fft2rl(p,r,l,alpha,true,tmpqz);
    fft2rl(q,r,l,beta,true,tmpqz);
    fft2rltimes(alpha,beta,expoN,tmp,tmpqz);
    fft2rl(alpha,r,l,pq,false,tmpqz);
    // divide by n mod N and coerce
    // 2^{r*2^l}}=-1 mod N therefore n=2^{l+1} inverse is -2^{r*2^l-(l+1)}
    unsigned long shift=expoN-l-1;
    fft2rldiv(pq,expoN,shift,tmp,tmpqz);
    mpz_clear(tmpqz); mpz_clear(tmp);
    if (debug_infolevel>3)
      CERR << CLOCK()*1e-6 << " fftmult end 2^r as a 2^l-root r=" << r << " l=" << l << '\n' ; 
  }


  // Fast Fourier Transform, f the poly sum_{j<n} f_j x^j, 
  // and w=[1,omega,...,omega^[m-1]] with m a multiple of n (m=step*n)
  // return [f(1),f(omega),...,f(omega^[n-1]) [it's indeed n, not m]
  // WARNING f is given in ascending power
  void fft(const modpoly & f,const modpoly & w ,modpoly & res,environment * env){
    if (env && env->moduloon && env->modulo.type==_INT_ && is_integer_vecteur(f,true) && is_integer_vecteur(w,true)){
      vector<int> F=vecteur_2_vector_int(f);
      vector<int> W=vecteur_2_vector_int(w);
      vector<int> RES(F.size());
      int m=env->modulo.val;
#ifndef FXCG
      if (debug_infolevel>2)
	CERR << CLOCK()*1e-6 << " begin fft int " << W.size() << " memory " << memory_usage()*1e-6 << "M" << '\n';
#endif
      fft(F,W,RES,m);
#ifndef FXCG
      if (debug_infolevel>2)
	CERR << CLOCK()*1e-6 << " end fft int " << W.size() << " memory " << memory_usage()*1e-6 << "M" << '\n';
#endif
      unsigned n=unsigned(RES.size());
      res.clear();
      res.reserve(n);
      for (unsigned i=0;i<n;++i){
	if (RES[i]<0)
	  res.push_back(RES[i]+m);
	else
	  res.push_back(RES[i]);
      }
      return;
    }
    unsigned long n=long(f.size()); // unsigned long does not parse with gcc
    if (n==1){
      res = f;
      return ;
    }
    unsigned long m=long(w.size());
    unsigned long step=m/n;
    unsigned k=0;
    if (n%2){
      for (k=3;k*k<=n;k++){
	if (!(n%k))
	  break;
      }
    }
    else
      k=2;
    if (k*k>n){ 
      // prime size, slow discrete Fourier transform
      res.clear();
      res.reserve(n);
      gen tmp;
      unsigned pos;
      for (unsigned i=0;i<n;++i){
	tmp = 0;
	pos = 0;
	for (unsigned j=0;j<n;++j){
	  tmp = tmp + f[j]*w[pos];
	  pos = (pos+i*step)%m;
	  if (env && env->moduloon)
	    tmp=smod(tmp,env->modulo);
	}
	res.push_back(tmp);
      }
      return;
    }
    if (k!=2){
      // assumes n is divisible by k, nk=n/k
      // P(X)=P_k(X)*[X^nk]^(k-1)+...+P_1(X) degree(P_k)<nk
      // P(w^(kj+l))= Q_l ( (w^k)^j )
      // with Q_l=P_1^(w^l)+w^(nk)*P_2^(w^l)+...
      unsigned long n2=n/k;
      vector<modpoly> Q(k),Qfft(k);
      for (unsigned j=0;j<k;++j)
	Q[j]=vecteur(n2,0);
      gen tmp;
      for (unsigned j=0;j<k;j++){
	// find Q[j]
	for (unsigned i=0;i<n2;i++){
	  tmp=0;
	  for (unsigned J=0;J<k;J++){
	    tmp += f[J*n2+i]*w[(J*j*n2*step)%m];
	  }
	  tmp=tmp*w[j*step*i];
	  if (env && env->moduloon)
	    tmp=smod(tmp,env->modulo);
	  Q[j][i]=tmp;
	}
	fft(Q[j],w,Qfft[j],env);
      }
      // build fft
      res.clear();
      res.reserve(n);
      for (unsigned i=0;i<n2;++i){
	for (unsigned j=0;j<k;++j)
	  res.push_back(Qfft[j][i]);
      }
      return;
    }
    // Compute r0=sum_[j<n/2] (f_j+f_(j+n/2))*x^j
    // and r1=sum_[j<n/2] (f_j-f_(j+n/2))*omega^[step*j]*x^j
    unsigned long n2=n/2;
    modpoly r0,r1;
    r0.reserve(n2); r1.reserve(n2);
    const_iterateur it=f.begin(),itn=it+n2,itend=itn,itk=w.begin();
    gen tmp;
    for (;it!=itend;++itn,itk+=step,++it){
      tmp=(*it)+(*itn);
      if (env && env->moduloon)
	tmp=smod(tmp,env->modulo);
      r0.push_back(tmp);
      tmp=((*it)-(*itn))*(*itk);
      if (env && env->moduloon)
	tmp=smod(tmp,env->modulo);
      r1.push_back(tmp);
    }
    // Recursive call
    modpoly r0f(n2),r1f(n2);
    fft(r0,w,r0f,env);
    fft(r1,w,r1f,env);
    // Return a mix of r0/r1
    res.clear();
    res.reserve(n);
    it=r0f.begin(); itend=it+n2; itn=r1f.begin();
    for (;it!=itend;){
      res.push_back(*it);
      ++it;
      res.push_back(*itn);
      ++itn;
    }
  }

  static void fft2( complex<double> *A, int n, complex<double> *W, complex<double> *T ) {  
    if ( n==1 ) return;
    // if p is fixed, the code is about 2* faster
    if (n==4){
      complex<double> w1=W[1];
      complex<double> f0=A[0],f1=A[1],f2=A[2],f3=A[3],f01=(f1-f3)*w1;
      A[0]=(f0+f1+f2+f3);
      A[1]=(f0-f2+f01);
      A[2]=(f0-f1+f2-f3);
      A[3]=(f0-f2-f01);
      return;
    }
    if (n==2){
      complex<double> f0=A[0],f1=A[1];
      A[0]=(f0+f1);
      A[1]=(f0-f1);
      return;
    }
    int i,n2;
    n2 = n/2;
    // Step 1 : arithmetic
    complex<double> * Tn2=T+n2,*An2=A+n2;
    for( i=0; i<n2; ++i ) {
      complex<double> Ai,An2i;
      Ai=A[i];
      An2i=An2[i];
      T[i] = Ai+An2i; // addmod(Ai,An2i,p);
      Tn2[i] = (Ai-An2i)*W[i]; // submod(Ai,An2i,p); mulmod(t,W[i],p); 
      i++;
      Ai=A[i];
      An2i=An2[i];
      T[i] = Ai+An2i; // addmod(Ai,An2i,p);
      Tn2[i] = (Ai-An2i)*W[i]; // submod(Ai,An2i,p); mulmod(t,W[i],p); 
    }
    // Step 2 : recursive calls
    fft2( T,    n2, W+n2, A    );
    fft2( Tn2, n2, W+n2, A+n2 );
    // Step 3 : permute
    for( i=0; i<n2; ++i ) {
      A[  2*i] = T[i];
      A[2*i+1] = Tn2[i]; 
      ++i;
      A[  2*i] = T[i];
      A[2*i+1] = Tn2[i]; 
    }
    return;
  }  

  void fft2( complex<double> * A, int n, double theta){
#ifndef FXCG
    if (debug_infolevel>2)
      CERR << CLOCK()*1e-6 << " begin fft2 C " << n << " memory " << memory_usage()*1e-6 << "M" << '\n';
#endif
    vector< complex<double> > W,T(n);
    W.reserve(n); 
    double thetak(theta);
    for (int N=n/2;N;N/=2,thetak*=2){
      complex<double> ww(1);
      complex<double> wk(std::cos(thetak),std::sin(thetak));
      for (int i=0;i<N;ww=ww*wk,++i){
	if (i%64==0)
	  ww=complex<double>(std::cos(i*thetak),std::sin(i*thetak));
	W.push_back(ww);
      }
    }
    fft2(A,n,&W.front(),&T.front());
#ifndef FXCG
    if (debug_infolevel>2)
      CERR << CLOCK()*1e-6 << " end fft C " << n << " memory " << memory_usage()*1e-6 << "M" << '\n';
#endif
  }

  void fft(std::complex<double> * f,int n,const std::complex<double> * w,int m,complex< double> * t){
    if (n==1)
      return ;
    int step=m/n;
    int k=0;
    if (n%2){
      for (k=3;k*k<=n;k++){
	if (!(n%k))
	  break;
      }
    }
    else
      k=2;
    if (k*k>n){ 
      // prime size, slow discrete Fourier transform
      complex<double> *fj,*fend_=f+n-3,*fend=f+n;
      complex<double> * res=t;
      for (int i=0;i<n;++i){
	complex<double> tmp (0,0);
	int pos=0,istep=i*step;
	for (fj=f;fj<fend_;fj+=3){
	  tmp +=  fj[0]*w[pos];
	  pos += istep-m; pos += (unsigned(pos)>>31)*m;// pos = (pos+istep)%m;
	  tmp +=  fj[1]*w[pos];
	  pos += istep-m; pos += (unsigned(pos)>>31)*m;// pos = (pos+istep)%m;
	  tmp +=  fj[2]*w[pos];
	  pos += istep-m; pos += (unsigned(pos)>>31)*m;// pos = (pos+istep)%m;
	}
	for (;fj<fend;++fj){
	  tmp +=  (*fj)*w[pos];
	  pos += istep-m; pos += (unsigned(pos)>>31)*m;// pos = (pos+istep)%m;
	}
	*res=tmp;
	++res;
      }
      for (fj=f,res=t;fj<fend;++fj,++res){
	*fj=*res;
      }
      return;
    }
    if (k!=2){
      // assumes n is divisible by k, nk=n/k
      // P(X)=P_k(X)*[X^nk]^(k-1)+...+P_1(X) degree(P_k)<nk
      // P(w^(kj+l))= Q_l ( (w^k)^j )
      // with Q_l=P_1^(w^l)+w^(nk)*P_2^(w^l)+...
      unsigned long n2=n/k;
      for (int j=0;j<k;j++){
	// find Q[j]
	complex<double> * Qj=t+n2*j;
	for (unsigned i=0;i<n2;i++){
	  complex<double> tmp(0,0);
	  int pos=0,jn2step=j*n2*step;
	  const complex<double> * fi=&f[i], *fiend=fi+k*n2;
	  for (;fi<fiend;fi+=n2){
	    tmp += (*fi)*w[pos];
	    pos += jn2step-m; pos += (unsigned(pos)>>31)*m;
	  }
	  Qj[i]=tmp*w[j*step*i];
	}
      }
      for (int j=0;j<k;++j){
	fft(t+n2*j,n2,w,m,f+n2*j);
      }
      // build fft
      for (unsigned i=0;i<n2;++i){
	for (int j=0;j<k;++j,++f)
	  *f=t[n2*j+i];
      }
      return;
    }
    // Compute r0=sum_[j<n/2] (f_j+f_(j+n/2))*x^j
    // and r1=sum_[j<n/2] (f_j-f_(j+n/2))*omega^[step*j]*x^j
    unsigned long n2=n/2;
    complex<double> * r0=t, *r1=t+n2;
    complex<double> * it=f,*itn=f+n2,*itend=itn;
    const complex<double> *itk=w;
    for (;it!=itend;++itn,itk+=step,++it,++r0,++r1){
      *r0=*it+*itn;
      *r1=(*it-*itn)*(*itk);
    }
    // Recursive call
    complex<double> * r0f=f,*r1f=f+n2;
    fft(t,n2,w,m,r0f);
    fft(t+n2,n2,w,m,r1f);
    // Return a mix of r0/r1
    it=t; itend=t+n2; itn=t+n2;
    for (;it!=itend;){
      *f=*it;
      ++it; ++f;
      *f=*itn;
      ++itn; ++f;
    }
  }

  // inplace fft with positive representant
  static inline int addmod(int a, int b, int p) { 
    // if (a<0 || b<0) CERR << '\n';
    int t=(a-p)+b;
#if defined(EMCC) || defined(EMCC2)
    if (t<0) return t+p; else return t;
#else
    t += (t>>31)&p;
    return t; 
#endif
  }
  static inline int submod(int a, int b, int p) { 
    // if (a<0 || b<0) CERR << '\n';
    int t=a-b;
#if defined(EMCC) || defined(EMCC2)
    if (t<0) return t+p; else return t;
#else
    t += (t>>31)&p;
    return t; 
#endif
  }

  static inline int mulmod(int a, int b, int p) { 
    return (longlong(a)*b) % p;
  }

  inline int mulmodp1(int a,int b){
    return amodp((longlong(a)*b),p1,invp1); // FIXME? 
    return (longlong(a)*b) % p1;    
  }

  inline int precond_mulmodp1(unsigned A,unsigned W,unsigned Winvp){
    longlong t = ulonglong(A)*W-((ulonglong(A)*Winvp)>>32)*p1;
    return t- (t>>63)*p1;
    int tt= t- (t>>63)*p1;
    unsigned s=(ulonglong(A)*W)%p1;
    if (tt!=s)
      CERR << '\n';
    return s;
  }

  inline int mulmodp2(int a,int b){
    return amodp((longlong(a)*b),p2,invp2); // FIXME?
    return (longlong(a)*b) % p2;    
  }

  inline int precond_mulmodp2(unsigned A,unsigned W,unsigned Winvp){
    longlong t = ulonglong(A)*W-((ulonglong(A)*Winvp)>>32)*p2;
    return t- (t>>63)*p2;
    int tt= t- (t>>63)*p2;
    unsigned s=(ulonglong(A)*W)%p2;
    if (tt!=s)
      CERR << '\n';
    return s;
  }

  inline int mulmodp3(int a,int b){
    return amodp((longlong(a)*b),p3,invp3);
    return (longlong(a)*b) % p3;    
  }

  inline int precond_mulmodp3(unsigned A,unsigned W,unsigned Winvp){
    longlong t = ulonglong(A)*W-((ulonglong(A)*Winvp)>>32)*p3;
    return t- (t>>63)*p3;
    int tt= t- (t>>63)*p3;
    unsigned s=(ulonglong(A)*W)%p3;
    if (tt!=s)
      CERR << '\n';
    return s;
  }

  inline int mulmodp4(int a,int b){
    return amodp((longlong(a)*b),p4,invp4);
    return (longlong(a)*b) % p4;    
  }

  // this should probably not be defined because gcc does division by csts
  // with multiplication itself 

#ifdef GIAC_PRECOND // preconditionned
  inline void fft_loop_p1(int & A,int & An2,int W,int Winv){
    int s = A;
    int t1;
    // t1=longlong(*An2)*(*W)-((longlong(*An2)*(*(W+n2)))>>31)*p1; t1 -= (t1>>31)*p1;
    t1=precond_mulmodp1(An2,W,Winv);
    A = addmod(s,t1,p1);
    An2 = submod(s,t1,p1); 
  }
#else // not preconditionned
  inline void fft_loop_p1(int & A,int & An2,int W){
    int s=A;
    int t = mulmodp1(W,An2);
    // if (t1!=t) CERR << t1 << " " << t << '\n';
    A = addmod(s,t,p1);
    An2 = submod(s,t,p1); 
  }
#endif

  inline void fft_loop_p1_(int * Acur,int *An2cur,int * Wcur,int n2){
    int Ai,An2i;
    Ai=*Acur;
    An2i=*An2cur;
    *Acur = addmod(Ai,An2i,p1);
#ifdef GIAC_PRECOND
    *An2cur=precond_mulmodp1(submod(Ai,An2i,p1),*Wcur,*(Wcur+n2));
#else
    *An2cur=amodp((longlong(Ai)+p1-An2i)* *Wcur,p1,invp1);
#endif
  }

  inline void fft_loop_p2(int * A,int *An2,int *W,int n2){
    int s = *A;
#ifdef GIAC_PRECOND // preconditionned
    int t=precond_mulmodp2(*An2,*W,*(W+n2));
#else // not preconditionned
    int t = mulmodp2(*W,*An2);
#endif
    *A = addmod(s,t,p2);
    *An2 = submod(s,t,p2); 
  }

  inline void fft_loop_p2_(int * Acur,int *An2cur,int * Wcur,int n2){
    int Ai,An2i;
    Ai=*Acur;
    An2i=*An2cur;
    *Acur = addmod(Ai,An2i,p2);
#ifdef GIAC_PRECOND
    *An2cur=precond_mulmodp2(submod(Ai,An2i,p2),*Wcur,*(Wcur+n2));
#else
    *An2cur=amodp((longlong(Ai)+p2-An2i)* *Wcur,p2,invp2);
#endif
  }

  inline void fft_loop_p3(int * A,int *An2,int *W,int n2){
    int s = *A;
#ifdef GIAC_PRECOND // preconditionned
    int t=precond_mulmodp3(*An2,*W,*(W+n2));
#else // not preconditionned
    int t = mulmodp3(*W,*An2);
#endif
    *A = addmod(s,t,p3);
    *An2 = submod(s,t,p3); 
  }

  inline void fft_loop_p3_(int * Acur,int *An2cur,int * Wcur,int n2){
    int Ai,An2i;
    Ai=*Acur;
    An2i=*An2cur;
    *Acur = addmod(Ai,An2i,p3);
#ifdef GIAC_PRECOND
    *An2cur=precond_mulmodp3(submod(Ai,An2i,p3),*Wcur,*(Wcur+n2));
#else
    *An2cur=amodp((longlong(Ai)+p3-An2i)* *Wcur,p3,invp3);
#endif
  }

  // Interesting primes (from A parallel implementation for polynomial multiplication modulo a prime, Law & Monagan, pasco 2015)
  // p:=2^25; for k from 64 downto 1 do if isprime(k*p+1) then print(k*p+1); fi od
  // p1 := 2013265921 ; r:=1227303670; root of unity order 2^27 (15*2^27+1)
  // p2 := 1811939329 ; r:=814458146; order 2^26 
  // p3 := 469762049 ; r:=2187; order 2^26
  // p4 := 2113929217 ; ( 63×2^25 +1)
  // p5 := 1711276033 ; ( 51×2^25 +1 )
  // For polynomial multiplication applications mod a prime p <2^32
  // with degree product<2^26
  // make multiplication in Z[x] before reducing modulo p
  // multiplication in Z[x] is computed by chinrem 
  // from multiplication in Z/p1, Z/p2, Z/p3 using fft
  // of size 2^k>degree(product), root of unity from a power of r
  // For multiplication in Z[x], do it mod sufficiently many primes<2^32
  // input A with positive int, output fft in A
  // W must contain 
  // [1,w,...,w^(n/2-1),1,w^2,w^4,...,w^(n/2-2),1,w^4,...,w^(n/2-4)...,1,w^(n/4),1]
  static void fft2p1( int *A, int n, int *W, int *T,int step=1) {  
    int i,n2,t;
    if ( n==1 ) return;
    // if p is fixed, the code is about 2* faster
    if (n==4){
      int w1=W[step];
#if 1
      int f0=A[0],f1=A[1],f2=A[2],f3=A[3],f01=mulmod(submod(f1,f3,p1),w1,p1),f02p=addmod(f0,f2,p1),f02m=submod(f0,f2,p1),f13=addmod(f1,f3,p1);
      A[0]=addmod(f02p,f13,p1);
      A[1]=addmod(f02m,f01,p1);
      A[2]=submod(f02p,f13,p1);
      A[3]=submod(f02m,f01,p1);
#else
      longlong f0=A[0],f1=A[1],f2=A[2],f3=A[3],f01=(f1-f3)*w1;
      A[0]=(f0+f1+f2+f3)%p1;
      A[1]=(f0-f2+f01)%p1;
      A[2]=(f0-f1+f2-f3)%p1;
      A[3]=(f0-f2-f01)%p1;
#endif
      return;
    }
    if (n==2){
      int f0=A[0],f1=A[1];
      A[0]=addmod(f0,f1,p1);
      A[1]=submod(f0,f1,p1);
      return;
    }
    n2 = n/2;
    // Step 1 : arithmetic
    int * Tn2=T+n2,*An2=A+n2;
    for( i=0; i<n2; ++i ) {
      int Ai,An2i;
      Ai=A[i];
      An2i=An2[i];
      T[i] = addmod(Ai,An2i,p1);
      t = submod(Ai,An2i,p1);
      Tn2[i] = mulmodp1(t,W[i*step]); 
      i++;
      Ai=A[i];
      An2i=An2[i];
      T[i] = addmod(Ai,An2i,p1);
      t = submod(Ai,An2i,p1);
      Tn2[i] = mulmodp1(t,W[i*step]); 
    }
    // Step 2 : recursive calls
    fft2p1(T, n2, W, A,2*step);
    fft2p1(Tn2, n2, W, A+n2,2*step);
    // Step 3 : permute
    for( i=0; i<n2; ++i ) {
      A[2*i] = T[i];
      A[2*i+1] = Tn2[i]; 
      ++i;
      A[2*i] = T[i];
      A[2*i+1] = Tn2[i]; 
    }
    return;
  }  

  static void fft2p1nopermbefore( int *A, int n, int *W,int step=1) {  
    if (n==0)
      CERR << "bug" << endl;
    if ( n==1 ) return;
    // if p is fixed, the code is about 2* faster
    if (n==4){
      int w1=W[step];
      int f0=A[0],f1=A[1],f2=A[2],f3=A[3],f01=mulmod(submod(f1,f3,p1),w1,p1),f02p=addmod(f0,f2,p1),f02m=submod(f0,f2,p1),f13=addmod(f1,f3,p1);
      A[0]=addmod(f02p,f13,p1);
      A[1]=addmod(f02m,f01,p1);
      A[2]=submod(f02p,f13,p1);
      A[3]=submod(f02m,f01,p1);
      return;
    }
    if (n==2){
      int f0=A[0],f1=A[1];
      A[0]=addmod(f0,f1,p1);
      A[1]=submod(f0,f1,p1);
      return;
    }
    fft2p1nopermbefore( A, n/2, W,2*step); // fft2p1nopermbefore(A,n2,W+n2);
    fft2p1nopermbefore( A+n/2, n/2, W,2*step); // fft2p1nopermbefore(An2,n2,W+n2);
    int * An2=A+n/2;
    int * Aend=A+n/2;
#if 0
    Aend=A+n;
    for (; An2<Aend;){
      *An2=mulmodp1(*An2,*W); ++An2;  W+=step;
      *An2=mulmodp1(*An2,*W); ++An2;  W+=step;
      *An2=mulmodp1(*An2,*W); ++An2;  W+=step;
      *An2=mulmodp1(*An2,*W); ++An2;  W+=step;
    }
    An2=A+n/2; Aend=An2;
    for (;A<Aend;){
      int s,t;
      s=A[0]; t=An2[0]; A[0]=addmod(s,t,p1); An2[0]=submod(s,t,p1);
      s=A[1]; t=An2[1]; A[1]=addmod(s,t,p1); An2[1]=submod(s,t,p1);
      s=A[2]; t=An2[2]; A[2]=addmod(s,t,p1); An2[2]=submod(s,t,p1);
      s=A[3]; t=An2[3]; A[3]=addmod(s,t,p1); An2[3]=submod(s,t,p1);
      A+=4; An2+=4;
    }
    return;
#endif
    int n2s = n/2*step; // n2%4==0
#ifdef GIAC_PRECOND
#if 1
    for(; A<Aend; ) {
      fft_loop_p1(*A,*An2,*W,*(W+n2s));
      ++A; ++An2; W +=step ;
      fft_loop_p1(*A,*An2,*W,*(W+n2s));
      ++A; ++An2; W +=step ;
      fft_loop_p1(*A,*An2,*W,*(W+n2s));
      ++A; ++An2; W += step;
      fft_loop_p1(*A,*An2,*W,*(W+n2s));
      ++A; ++An2; W +=step;
    }
#else
    for(int i=0; i<n/2; i +=4 ) {
      fft_loop_p1(A[i],An2[i],W[i*step],W[(i+n2s)*step]);
      fft_loop_p1(A[i+1],An2[i+1],W[(i+1)*step],W[(i+1+n2s)*step]);
      fft_loop_p1(A[i+2],An2[i+2],W[(i+2)*step],W[(i+2+n2s)*step]);
      fft_loop_p1(A[i+3],An2[i+3],W[(i+3)*step],W[(i+3+n2s)*step]);
    }
#endif
#else // GIAC_PRECOND
    for(; A<Aend; ) {
#if 1
      fft_loop_p1(*A,*An2,*W);
      ++A; ++An2; W +=step ;
      fft_loop_p1(*A,*An2,*W);
      ++A; ++An2; W +=step ;
      fft_loop_p1(*A,*An2,*W);
      ++A; ++An2; W += step;
      fft_loop_p1(*A,*An2,*W);
      ++A; ++An2; W +=step;
#else
      fft_loop_p1(A[0],An2[0],W[0]);
      fft_loop_p1(A[1],An2[1],W[step]);
      fft_loop_p1(A[2],An2[2],W[2*step]);
      fft_loop_p1(A[3],An2[3],W[3*step]);
      A+=4; An2+=4; W +=4*step ;
#endif
    }
#endif // GIAC_PRECOND
  }  

  static void fft2p1nopermafter( int *A, int n, int *W,int step=1) {  
    if ( n==1 ) return;
    // if p is fixed, the code is about 2* faster
    if (n==4){
      int w1=W[step];
      int f0=A[0],f1=A[1],f2=A[2],f3=A[3],f01=mulmod(submod(f1,f3,p1),w1,p1),f02p=addmod(f0,f2,p1),f02m=submod(f0,f2,p1),f13=addmod(f1,f3,p1);
      A[0]=addmod(f02p,f13,p1);
      A[1]=addmod(f02m,f01,p1);
      A[2]=submod(f02p,f13,p1);
      A[3]=submod(f02m,f01,p1);
      return;
    }
    if (n==2){
      int f0=A[0],f1=A[1];
      A[0]=addmod(f0,f1,p1);
      A[1]=submod(f0,f1,p1);
      return;
    }
    // Step 1 : arithmetic
    int *An2=A+n/2;
#if 1
    int * Acur=A,*An2cur=An2,*Wcur=W;
    int n2=n/2*step;
    for (;Acur!=An2;){
      int Ai,An2i;
      fft_loop_p1_(Acur,An2cur,Wcur,n2);
      ++Acur;++An2cur; Wcur +=step;
      fft_loop_p1_(Acur,An2cur,Wcur,n2);
      ++Acur;++An2cur; Wcur += step;
      fft_loop_p1_(Acur,An2cur,Wcur,n2);
      ++Acur;++An2cur; Wcur += step;
      fft_loop_p1_(Acur,An2cur,Wcur,n2);
      ++Acur;++An2cur; Wcur += step;
    }
#else
    for( i=0; i<n/2; ++i ) {
      int Ai,An2i;
      Ai=A[i];
      An2i=An2[i];
      A[i] = addmod(Ai,An2i,p1);
      An2[i]=((longlong(Ai)+p1-An2i)*W[i*step]) % p1; // t = submod(Ai,An2i,p); An2[i] = mulmod(t,W[i],p); 
      i++;
      Ai=A[i];
      An2i=An2[i];
      A[i] = addmod(Ai,An2i,p1);
      An2[i]=((longlong(Ai)+p1-An2i)*W[i*step]) % p1; // t = submod(Ai,An2i,p); An2[i] = mulmod(t,W[i],p); 
    }
#endif
    // Step 2 : recursive calls
    fft2p1nopermafter(A, n/2, W,2*step);
    fft2p1nopermafter(An2, n/2, W,2*step);
  }  

  inline void fft_loop_p_precond(int & A,int & An2,int W,int Winv,int p){
    int s = A;
    int t1;
    // longlong t = ulonglong(A)*W-((ulonglong(A)*Winvp)>>32)*p; return t+ ((t>>31)&p);
    t1=precond_mulmodp(An2,W,Winv,p);
    A = addmod(s,t1,p);
    An2 = submod(s,t1,p); 
  }

  inline void fft_loop_p(int & A,int & An2,int W,int p,double invp){
    int s=A;
    int t = mulmodp(An2,W,p,invp);
    //t += (t>>31)&p; // FIXME?
    // if (t1!=t) CERR << t1 << " " << t << '\n';
    A = addmod(s,t,p);
    An2 = submod(s,t,p); 
  }

#if !defined NUMWORKS // !defined VISUALC && !defined USE_GMP_REPLACEMENTS && defined GIAC_PRECOND // de-recurse
  static void fft2pnopermbefore( int *A, int n, int *W,int p,double invp,int step) {  
    if (n==0)
      CERR << "bug\n";
    if (n<=1 ) return;
    if (n==2){
      int f0=A[0],f1=A[1];
      A[0]=addmod(f0,f1,p);
      A[1]=submod(f0,f1,p);
      return;
    }
    int n2s=n/2*step;
    // start by groups of 4
    step=n2s/2;
    int w1=W[step],w1surp=W[3*step];
    int *Aeff=A;
    for (int pos=0;pos<n;pos+=4,Aeff+=4){
      int f0=Aeff[0],f1=Aeff[1],f2=Aeff[2],f3=Aeff[3],
	f01=precond_mulmodp(f1-f3+p,w1,w1surp,p),
	f02p=addmod(f0,f2,p),f02m=submod(f0,f2,p),f13=addmod(f1,f3,p);
      Aeff[0]=addmod(f02p,f13,p);
      Aeff[1]=addmod(f02m,f01,p);
      Aeff[2]=submod(f02p,f13,p);
      Aeff[3]=submod(f02m,f01,p);
    }
    // now by 8, then by 16, etc.
    int Wstack_[MAX_INTSTACK];
    int *Wstack=0;
    if (n>MAX_INTSTACK)
      Wstack=(int *)malloc(n*sizeof(int));
    else
      Wstack=Wstack_;
    //size_t T=n*sizeof(int);
    //int * Wstack=(int*)stack_or_heap_alloc(T);//int Wstack[taille];
    for (int taille=8;taille<=n;taille*=2){
      step /= 2;
      Aeff=A;
      if (taille==n && step==1){
	int *An2=Aeff+n/2,*Aend=An2,*Weff=W+n2s;
	for(; Aeff<Aend; ) {
	  fft_loop_p_precond(Aeff[0],An2[0],W[0],Weff[0],p);
	  fft_loop_p_precond(Aeff[1],An2[1],W[1],Weff[1],p);
	  fft_loop_p_precond(Aeff[2],An2[2],W[2],Weff[2],p);
	  fft_loop_p_precond(Aeff[3],An2[3],W[3],Weff[3],p);
	  Aeff+=4; An2+=4; W+=4; Weff+=4;
	}
	break;
      }
      int * end=Wstack+taille,*source=W,*source2=W+n2s;
      for (int * target=Wstack;target<end;target+=8){
	target[0]=*source; source+=step;
	target[1]=*source; source+=step;
	target[2]=*source; source+=step;
	target[3]=*source; source+=step;
	target[4]=*source2; source2+=step;
	target[5]=*source2; source2+=step;
	target[6]=*source2; source2+=step;
	target[7]=*source2; source2+=step;
      }
      for (int pos=0;pos<n;pos+=taille){
	int *An2=Aeff+taille/2,*Aend=An2,*Weff=Wstack;
	int s=*Aeff,t1=*An2;
	*Aeff=addmod(s,t1,p);
	*An2=submod(s,t1,p);
	fft_loop_p_precond(Aeff[1],An2[1],Weff[1],Weff[5],p);
	fft_loop_p_precond(Aeff[2],An2[2],Weff[2],Weff[6],p);
	fft_loop_p_precond(Aeff[3],An2[3],Weff[3],Weff[7],p);
	Aeff+=4; An2+=4; Weff+=8;
	for (;Aeff<Aend;){
	  fft_loop_p_precond(Aeff[0],An2[0],Weff[0],Weff[4],p);
	  fft_loop_p_precond(Aeff[1],An2[1],Weff[1],Weff[5],p);
	  fft_loop_p_precond(Aeff[2],An2[2],Weff[2],Weff[6],p);
	  fft_loop_p_precond(Aeff[3],An2[3],Weff[3],Weff[7],p);
	  Aeff+=4; An2+=4; Weff+=8;
	}
	Aeff+=taille/2;
      }
    }
    if (n>MAX_INTSTACK)
      free(Wstack);
  }

#else // de-recurse
  static void fft2pnopermbefore( int *A, int n, int *W,int p,double invp,int step) {  
    if (n==0)
      CERR << "bug" << endl;
    if ( n==1 ) return;
    // if p is fixed, the code is about 2* faster
    if (n==4){
#ifdef GIAC_CACHEW
      int w1=W[1];
#else
      int w1=W[step];
#endif
      //CERR << n << " " << w1 << endl;
      int f0=A[0],f1=A[1],f2=A[2],f3=A[3],
#ifdef GIAC_PRECOND
	f01=precond_mulmodp(f1-f3+p,w1,W[3*step],p),
#else
	f01=mulmodp(submod(f1,f3,p),w1,p,invp),
#endif
	f02p=addmod(f0,f2,p),f02m=submod(f0,f2,p),f13=addmod(f1,f3,p);
      A[0]=addmod(f02p,f13,p);
      A[1]=addmod(f02m,f01,p);
      A[2]=submod(f02p,f13,p);
      A[3]=submod(f02m,f01,p);
      return;
    }
    if (n==2){
      int f0=A[0],f1=A[1];
      A[0]=addmod(f0,f1,p);
      A[1]=submod(f0,f1,p);
      return;
    }
    int * An2=A+n/2;
    int * Aend=A+n/2;
#ifdef GIAC_CACHEW
    fft2pnopermbefore( A, n/2, W+n/2,p,invp,2*step); // fft2pnopermbefore(A,n2,W+n2);
    fft2pnopermbefore( A+n/2, n/2, W+n/2,p,invp,2*step); // fft2pnopermbefore(An2,n2,W+n2);
    for(; A<Aend; ) {
      fft_loop_p(*A,*An2,*W,p,invp);
      fft_loop_p(A[1],An2[1],W[1],p,invp);
      fft_loop_p(A[2],An2[2],W[2],p,invp);
      fft_loop_p(A[3],An2[3],W[3],p,invp);
      A+=4; An2+=4; W+=4;
    }
#else // GIAC_CACHEW
    fft2pnopermbefore( A, n/2, W,p,invp,2*step); // fft2pnopermbefore(A,n2,W+n2);
    fft2pnopermbefore( A+n/2, n/2, W,p,invp,2*step); // fft2pnopermbefore(An2,n2,W+n2);
    int n2s = n/2*step; // n2%4==0
#ifdef GIAC_PRECOND
    for(; A<Aend; ) {
      fft_loop_p_precond(*A,*An2,*W,*(W+n2s),p);
      ++A; ++An2; W +=step ;
      fft_loop_p_precond(*A,*An2,*W,*(W+n2s),p);
      ++A; ++An2; W +=step ;
      fft_loop_p_precond(*A,*An2,*W,*(W+n2s),p);
      ++A; ++An2; W += step;
      fft_loop_p_precond(*A,*An2,*W,*(W+n2s),p);
      ++A; ++An2; W +=step;
    }
#else // GIAC_PRECOND
    for(; A<Aend; ) {
#if 1
      fft_loop_p(*A,*An2,*W,p,invp);
      W += step;
      fft_loop_p(A[1],An2[1],*W,p,invp);
      W += step;
      fft_loop_p(A[2],An2[2],*W,p,invp);
      W += step;
      fft_loop_p(A[3],An2[3],*W,p,invp);
      W += step; 
      A+=4; An2+=4;
#else
      fft_loop_p(*A,*An2,*W,p,invp);
      ++A; ++An2; W +=step ;
      fft_loop_p(*A,*An2,*W,p,invp);
      ++A; ++An2; W +=step ;
      fft_loop_p(*A,*An2,*W,p,invp);
      ++A; ++An2; W += step;
      fft_loop_p(*A,*An2,*W,p,invp);
      ++A; ++An2; W +=step;
#endif
    }
#endif // GIAC_PRECOND
#endif // GIAC_CACHEW
  }  
#endif // derecurse

#ifdef GIAC_PRECOND
  inline void fft_loop_p_(int * Acur,int *An2cur,int * Wcur,int n2,int p,double invp){
    int Ai,An2i;
    Ai=*Acur;
    An2i=*An2cur;
    *Acur = addmod(Ai,An2i,p);
    *An2cur=precond_mulmodp(Ai-An2i+p,*Wcur,*(Wcur+n2),p);
    // *An2cur=precond_mulmodp(submod(Ai,An2i,p),*Wcur,*(Wcur+n2),p);
  }
  inline void fft_loop_p_precond_(int * Acur,int *An2cur,int Wcur,int Winvp,int p){
    int Ai,An2i;
    Ai=*Acur;
    An2i=*An2cur;
    *Acur = addmod(Ai,An2i,p);
    *An2cur=precond_mulmodp(Ai-An2i+p,Wcur,Winvp,p);
  }
#else
  inline void fft_loop_p_(int * Acur,int *An2cur,int * Wcur,int p,double invp){
    int Ai,An2i;
    Ai=*Acur;
    An2i=*An2cur;
    *Acur = addmod(Ai,An2i,p);
    *An2cur=amodp((longlong(Ai)+p-An2i)* *Wcur,p,invp);
    // Ai==amodp((longlong(Ai)+p-An2i)* *Wcur,p,invp);
    //Ai+=(Ai>>31)&p; 
    //*An2cur=Ai;
  }
#endif


#if !defined NUMWORKS && defined GIAC_PRECOND // !defined VISUALC && !defined USE_GMP_REPLACEMENTS // de-recurse
  static void fft2pnopermafter( int *A, int n, int *W,int p,double invp,int step) {  
    if (n==0)
      CERR << "bug\n";
    if (n<=1 ) return;
    if (n==2){
      int f0=A[0],f1=A[1];
      A[0]=addmod(f0,f1,p);
      A[1]=submod(f0,f1,p);
      return;
    }
    int n2s=n/2*step;
    // group by decreasing size
    int Wstack_[MAX_INTSTACK];
    int * Wstack=0;
    if (n>MAX_INTSTACK)
      Wstack=(int *)malloc(n*sizeof(int));
    else
      Wstack=Wstack_;
    int * end=Wstack+n,*source=W,*source2=W+n2s;
    for (int * target=Wstack;target<end;target+=8){
      target[0]=*source; source+=step;
      target[1]=*source; source+=step;
      target[2]=*source; source+=step;
      target[3]=*source; source+=step;
      target[4]=*source2; source2+=step;
      target[5]=*source2; source2+=step;
      target[6]=*source2; source2+=step;
      target[7]=*source2; source2+=step;
    }
    for (int taille=n;taille>=8;taille/=2){
      int * Aeff=A;
      for (int pos=0;pos<n;pos+=taille){
	int *An2=Aeff+taille/2,*Aend=An2,*Weff=Wstack;
	int s=*Aeff,t1=*An2;
	*Aeff=addmod(s,t1,p);
	*An2=submod(s,t1,p);
	fft_loop_p_precond_(&Aeff[1],&An2[1],Weff[1],Weff[5],p);
	fft_loop_p_precond_(&Aeff[2],&An2[2],Weff[2],Weff[6],p);
	fft_loop_p_precond_(&Aeff[3],&An2[3],Weff[3],Weff[7],p);
	Aeff+=4; An2+=4; Weff+=8;
	for (;Aeff<Aend;){
#if 0 // def HAVE_VCL1_VECTORCLASS_H 
	  Vec4ui A4,An4,B4;
	  Vec4uq C4;
	  A4.load(Aeff);
	  An4.load(An2);
	  B4 = (A4-p)+An4;
	  B4 += ( (Vec4i) B4>>31) & p; // make positive
	  B4.store(Aeff);
	  C4 = extend( A4+p-An4);
	  A4.load(Weff); // W4
	  B4.load(Weff+4); // W4inv
	  C4 = C4*extend(A4)-((C4*extend(B4))>>32)*p;
	  // C4 += ( (Vec4q) C4>>63) & p;
	  // An4=compress(C4);
	  An4=compress(*(Vec4q*) &C4);
	  An4 += ( (Vec4i) An4>>31)&p; 
	  An4.store(An2);
	  Aeff+=4; An2+=4; Weff+=8; continue;
#endif // VECTORCLASS_H
	  fft_loop_p_precond_(&Aeff[0],&An2[0],Weff[0],Weff[4],p);
	  fft_loop_p_precond_(&Aeff[1],&An2[1],Weff[1],Weff[5],p);
	  fft_loop_p_precond_(&Aeff[2],&An2[2],Weff[2],Weff[6],p);
	  fft_loop_p_precond_(&Aeff[3],&An2[3],Weff[3],Weff[7],p);
	  Aeff+=4; An2+=4; Weff+=8;
	}
	Aeff+=taille/2;
      }
      if (taille==8)
	break;
      int * end=Wstack+taille,*source=Wstack;
      for (int * target=Wstack;source<end;source+=16,target+=8){
	target[0]=source[0];
	target[1]=source[2];
	target[4]=source[4];
	target[5]=source[6];
	target[2]=source[8];
	target[3]=source[10];
	target[6]=source[12];
	target[7]=source[14];
      }
    }
    if (n>MAX_INTSTACK)
      free(Wstack);
    // finish by groups of 4
    step=n2s/2;
    int w1=W[step],w1surp=W[3*step];
    int *Aeff=A;
    for (int pos=0;pos<n;pos+=4,Aeff+=4){
      int f0=Aeff[0],f1=Aeff[1],f2=Aeff[2],f3=Aeff[3],
	f01=precond_mulmodp(f1-f3+p,w1,w1surp,p),
	f02p=addmod(f0,f2,p),f02m=submod(f0,f2,p),f13=addmod(f1,f3,p);
      Aeff[0]=addmod(f02p,f13,p);
      Aeff[1]=addmod(f02m,f01,p);
      Aeff[2]=submod(f02p,f13,p);
      Aeff[3]=submod(f02m,f01,p);
    }
  }

#else // de-recurse
  static void fft2pnopermafter( int *A, int n, int *W,int p,double invp,int step) {  
    if ( n==1 ) return;
    // if p is fixed, the code is about 2* faster
    if (n==4){
#ifdef GIAC_CACHEW
      int w1=W[1];
#else
      int w1=W[step];
#endif
      int f0=A[0],f1=A[1],f2=A[2],f3=A[3],
#ifdef GIAC_PRECOND
	f01=precond_mulmodp(f1-f3+p,w1,W[3*step],p),
#else
	f01=mulmodp(submod(f1,f3,p),w1,p,invp),
#endif
	f02p=addmod(f0,f2,p),f02m=submod(f0,f2,p),f13=addmod(f1,f3,p);
      A[0]=addmod(f02p,f13,p);
      A[1]=addmod(f02m,f01,p);
      A[2]=submod(f02p,f13,p);
      A[3]=submod(f02m,f01,p);
      return;
    }
    if (n==2){
      int f0=A[0],f1=A[1];
      A[0]=addmod(f0,f1,p);
      A[1]=submod(f0,f1,p);
      return;
    }
    // Step 1 : arithmetic
    int *An2=A+n/2;
    int * Acur=A,*An2cur=An2,*Wcur=W;
#ifdef GIAC_CACHEW
    for (;Acur!=An2;){
      fft_loop_p_(Acur,An2cur,Wcur,p,invp);
      fft_loop_p_(&Acur[1],&An2cur[1],Wcur+1,p,invp);
      fft_loop_p_(&Acur[2],&An2cur[2],Wcur+2,p,invp);
      fft_loop_p_(&Acur[3],&An2cur[3],Wcur+3,p,invp);
      Acur+=4; An2cur+=4; Wcur += 4;
    }
    fft2pnopermafter(A, n/2, W+n/2,p,invp,2*step);
    fft2pnopermafter(An2, n/2, W+n/2,p,invp,2*step);
    return;
#endif
#ifdef GIAC_PRECOND
    int n2=n/2*step;
    for (;Acur!=An2;){
      fft_loop_p_(Acur,An2cur,Wcur,n2,p,invp);
      ++Acur;++An2cur; Wcur +=step;
      fft_loop_p_(Acur,An2cur,Wcur,n2,p,invp);
      ++Acur;++An2cur; Wcur += step;
      fft_loop_p_(Acur,An2cur,Wcur,n2,p,invp);
      ++Acur;++An2cur; Wcur += step;
      fft_loop_p_(Acur,An2cur,Wcur,n2,p,invp);
      ++Acur;++An2cur; Wcur += step;
      // continue;
#if 0 // def HAVE_VCL1_VECTORCLASS_H // debug
      A4.load(Acur-4);
      An4.load(An2cur-4);
      if ( horizontal_count(An4==compress(C4))!=4 || horizontal_count(A4==compress(B4))!=4)
	CERR << "err\n";
#endif
    }
#else // GIAC_PRECOND
    for (;Acur!=An2;){
      fft_loop_p_(Acur,An2cur,Wcur,p,invp);
      Wcur +=step;
      fft_loop_p_(&Acur[1],&An2cur[1],Wcur,p,invp);
      Wcur += step;
      fft_loop_p_(&Acur[2],&An2cur[2],Wcur,p,invp);
      Wcur += step;
      fft_loop_p_(&Acur[3],&An2cur[3],Wcur,p,invp);
      Acur+=4; An2cur+=4; Wcur += step;
    }
#endif // GIAC_PRECOND
    // Step 2 : recursive calls
    fft2pnopermafter(A, n/2, W,p,invp,2*step);
    fft2pnopermafter(An2, n/2, W,p,invp,2*step);
  }  
#endif // de-recurse


#if 0
  static void fft4wp1(vector<int> & W,int n,int w){
    W.reserve(n); 
    const int p = 2013265921 ;
    w=w % p;
    if (w<0) w += p;
    longlong wk=w;
    for (int N=n/2;N;N/=4,wk=(wk*wk)%p,wk=(wk*wk)%p){
      int ww=1;
      for (int i=0;i<N;ww=(ww*wk)%p,++i){
	W.push_back(ww);
      }
    }
  }

  static void fft4wp2(vector<int> & W,int n,int w){
    W.reserve(n); 
    const int p = 1811939329 ;
    w=w % p;
    if (w<0) w += p;
    longlong wk=w;
    for (int N=n/2;N;N/=4,wk=(wk*wk)%p,wk=(wk*wk)%p){
      int ww=1;
      for (int i=0;i<N;ww=(ww*wk)%p,++i){
	W.push_back(ww);
      }
    }
  }

  static void fft4p1nopermafter( int *A, int n, int *W) {  
    if ( n==1 ) return;
    // if p is fixed, the code is about 2* faster
    const int p = 2013265921 ;
    if (n==4){
      int w1=W[1];
      int f0=A[0],f1=A[1],f2=A[2],f3=A[3],f01=mulmod(submod(f1,f3,p),w1,p),f02p=addmod(f0,f2,p),f02m=submod(f0,f2,p),f13=addmod(f1,f3,p);
      A[0]=addmod(f02p,f13,p);
      A[1]=addmod(f02m,f01,p);
      A[2]=submod(f02p,f13,p);
      A[3]=submod(f02m,f01,p);
      return;
    }
    if (n==2){
     int f0=A[0],f1=A[1];
      A[0]=addmod(f0,f1,p);
      A[1]=submod(f0,f1,p);
      return;
    }
    int i,n2,n3,n4;
    n4=n/4; n2=n/2; n3=n2+n4;
    // Step 1 : arithmetic
    int *An4=A+n4, *An2=A+n2, *A3n4=A+n3,*Wn4=W+n4;
    for( i=0; i<n4; ++i ) {
      int Ai,An2i,An4i,A3n4i;
      Ai=A[i];
      An4i=An4[i];
      An2i=An2[i];
      A3n4i=A3n4[i];
      int w=W[2*i];
      int s1 = addmod(Ai,An2i,p);
      int s2 = addmod(An4i,A3n4i,p);
      A[i]=addmod(s1,s2,p);
      An4[i]=((longlong(s1)+p-s2)*w)%p;// mulmod(submod(s1,s2,p),w,p);
      s1 = ((longlong(Ai)+p-An2i)*W[i])%p;// mulmod(submod(Ai,An2i,p),W[i],p);
      s2 = ((longlong(An4i)+p-A3n4i)*Wn4[i])%p;// mulmod(submod(An4i,A3n4i,p),W[i+n4],p);
      An2[i]=addmod(s1,s2,p);
      A3n4[i]=((longlong(s1)+p-s2)*w)%p; // mulmod(submod(t1,t2,p),w,p);
      ++i;
      Ai=A[i];
      An4i=An4[i];
      An2i=An2[i];
      A3n4i=A3n4[i];
      w=W[2*i];
      s1 = addmod(Ai,An2i,p);
      s2 = addmod(An4i,A3n4i,p);
      A[i]=addmod(s1,s2,p);
      An4[i]=((longlong(s1)+p-s2)*w)%p;// mulmod(submod(s1,s2,p),w,p);
      s1 = ((longlong(Ai)+p-An2i)*W[i])%p;// mulmod(submod(Ai,An2i,p),W[i],p);
      s2 = ((longlong(An4i)+p-A3n4i)*Wn4[i])%p;// mulmod(submod(An4i,A3n4i,p),W[i+n4],p);
      An2[i]=addmod(s1,s2,p);
      A3n4[i]=((longlong(s1)+p-s2)*w)%p; // mulmod(submod(t1,t2,p),w,p);
    }
    // Step 2 : recursive calls
    fft4p1nopermafter(A, n4, W+n2);
    fft4p1nopermafter(A+n4, n4, W+n2);
    fft4p1nopermafter(A+n2, n4, W+n2);
    fft4p1nopermafter(A+n3, n4, W+n2);
    if (n==8){
      swapint(A[1],A[2]);
      swapint(A[5],A[6]);
    }
  }  
  static void fft4p2nopermafter( int *A, int n, int *W) {  
    if ( n==1 ) return;
    // if p is fixed, the code is about 2* faster
    const int p = 1811939329 ;
    if (n==4){
      int w1=W[1];
      int f0=A[0],f1=A[1],f2=A[2],f3=A[3],f01=mulmod(submod(f1,f3,p),w1,p),f02p=addmod(f0,f2,p),f02m=submod(f0,f2,p),f13=addmod(f1,f3,p);
      A[0]=addmod(f02p,f13,p);
      A[1]=addmod(f02m,f01,p);
      A[2]=submod(f02p,f13,p);
      A[3]=submod(f02m,f01,p);
      return;
    }
    if (n==2){
      int f0=A[0],f1=A[1];
      A[0]=addmod(f0,f1,p);
      A[1]=submod(f0,f1,p);
      return;
    }
    int i,n2,n3,n4;
    n4=n/4; n2=n/2; n3=n2+n4;
    // Step 1 : arithmetic
    int *An4=A+n4, *An2=A+n2, *A3n4=A+n3,*Wn4=W+n4;
    for( i=0; i<n4; ++i ) {
      int Ai,An2i,An4i,A3n4i;
      Ai=A[i];
      An4i=An4[i];
      An2i=An2[i];
      A3n4i=A3n4[i];
      int w=W[2*i];
      int s1 = addmod(Ai,An2i,p);
      int s2 = addmod(An4i,A3n4i,p);
      A[i]=addmod(s1,s2,p);
      An4[i]=((longlong(s1)+p-s2)*w)%p;// mulmod(submod(s1,s2,p),w,p);
      s1 = ((longlong(Ai)+p-An2i)*W[i])%p;// mulmod(submod(Ai,An2i,p),W[i],p);
      s2 = ((longlong(An4i)+p-A3n4i)*Wn4[i])%p;// mulmod(submod(An4i,A3n4i,p),W[i+n4],p);
      An2[i]=addmod(s1,s2,p);
      A3n4[i]=((longlong(s1)+p-s2)*w)%p; // mulmod(submod(t1,t2,p),w,p);
      ++i;
      Ai=A[i];
      An4i=An4[i];
      An2i=An2[i];
      A3n4i=A3n4[i];
      w=W[2*i];
      s1 = addmod(Ai,An2i,p);
      s2 = addmod(An4i,A3n4i,p);
      A[i]=addmod(s1,s2,p);
      An4[i]=((longlong(s1)+p-s2)*w)%p;// mulmod(submod(s1,s2,p),w,p);
      s1 = ((longlong(Ai)+p-An2i)*W[i])%p;// mulmod(submod(Ai,An2i,p),W[i],p);
      s2 = ((longlong(An4i)+p-A3n4i)*Wn4[i])%p;// mulmod(submod(An4i,A3n4i,p),W[i+n4],p);
      An2[i]=addmod(s1,s2,p);
      A3n4[i]=((longlong(s1)+p-s2)*w)%p; // mulmod(submod(t1,t2,p),w,p);
    }
    // Step 2 : recursive calls
    fft4p2nopermafter(A, n4, W+n2);
    fft4p2nopermafter(A+n4, n4, W+n2);
    fft4p2nopermafter(A+n2, n4, W+n2);
    fft4p2nopermafter(A+n3, n4, W+n2);
    if (n==8){
      swapint(A[1],A[2]);
      swapint(A[5],A[6]);
    }
  }  
#endif

  static void fft2p2nopermbefore( int *A, int n, int *W,int step=1) {  
    if ( n==1 ) return;
    // if p is fixed, the code is about 2* faster
    if (n==4){
      int w1=W[step];
      int f0=A[0],f1=A[1],f2=A[2],f3=A[3],f01=mulmod(submod(f1,f3,p2),w1,p2),f02p=addmod(f0,f2,p2),f02m=submod(f0,f2,p2),f13=addmod(f1,f3,p2);
      A[0]=addmod(f02p,f13,p2);
      A[1]=addmod(f02m,f01,p2);
      A[2]=submod(f02p,f13,p2);
      A[3]=submod(f02m,f01,p2);
      return;
    }
    if (n==2){
      int f0=A[0],f1=A[1];
      A[0]=addmod(f0,f1,p2);
      A[1]=submod(f0,f1,p2);
      return;
    }
    fft2p2nopermbefore( A, n/2, W,2*step); // fft2p2nopermbefore(A,n2,W+n2);
    int * An2=A+n/2;
    fft2p2nopermbefore( An2, n/2, W,2*step); // fft2p2nopermbefore(An2,n2,W+n2);
#if 1
    int n2 = n/2*step; // n2%4==0
    int * Aend=An2;
    for(; A<Aend; ) {
      fft_loop_p2(A,An2,W,n2);
      ++A; ++An2; W +=step ;
      fft_loop_p2(A,An2,W,n2);
      ++A; ++An2; W +=step ;
      fft_loop_p2(A,An2,W,n2);
      ++A; ++An2; W += step;
      fft_loop_p2(A,An2,W,n2);
      ++A; ++An2; W +=step;
    }
#else
    for(int i=0; i<n/2; i +=4 ) {
      int s = A[i];
      int t = mulmodp2(W[i*step],An2[i]);
      A[i] = addmod(s,t,p2);
      An2[i] = submod(s,t,p2); 
      s = A[i+1];
      t = mulmodp2(W[(i+1)*step],An2[i+1]);
      A[i+1] = addmod(s,t,p2);
      An2[i+1] = submod(s,t,p2); 
      s = A[i+2];
      t = mulmodp2(W[(i+2)*step],An2[i+2]);
      A[i+2] = addmod(s,t,p2);
      An2[i+2] = submod(s,t,p2); 
      s = A[i+3];
      t = mulmodp2(W[(i+3)*step],An2[i+3]);
      A[i+3] = addmod(s,t,p2);
      An2[i+3] = submod(s,t,p2); 
    }
#endif
  }  

  static void fft2p2nopermafter( int *A, int n, int *W,int step=1) {  
    if ( n==1 ) return;
    // if p is fixed, the code is about 2* faster
    if (n==4){
      int w1=W[step];
      int f0=A[0],f1=A[1],f2=A[2],f3=A[3],f01=mulmod(submod(f1,f3,p2),w1,p2),f02p=addmod(f0,f2,p2),f02m=submod(f0,f2,p2),f13=addmod(f1,f3,p2);
      A[0]=addmod(f02p,f13,p2);
      A[1]=addmod(f02m,f01,p2);
      A[2]=submod(f02p,f13,p2);
      A[3]=submod(f02m,f01,p2);
      return;
    }
    if (n==2){
      int f0=A[0],f1=A[1];
      A[0]=addmod(f0,f1,p2);
      A[1]=submod(f0,f1,p2);
      return;
    }
    // Step 1 : arithmetic
    int *An2=A+n/2;
#if 1
    int * Acur=A,*An2cur=An2,*Wcur=W;
    int n2=n/2*step;
    for (;Acur!=An2;){
      int Ai,An2i;
      fft_loop_p2_(Acur,An2cur,Wcur,n2);
      ++Acur;++An2cur; Wcur +=step;
      fft_loop_p2_(Acur,An2cur,Wcur,n2);
      ++Acur;++An2cur; Wcur += step;
      fft_loop_p2_(Acur,An2cur,Wcur,n2);
      ++Acur;++An2cur; Wcur += step;
      fft_loop_p2_(Acur,An2cur,Wcur,n2);
      ++Acur;++An2cur; Wcur += step;
    }
#else
    for( i=0; i<n/2; ++i ) {
      int Ai,An2i;
      Ai=A[i];
      An2i=An2[i];
      A[i] = addmod(Ai,An2i,p2);
      An2[i]=((longlong(Ai)+p2-An2i)*W[i*step]) % p2; // t = submod(Ai,An2i,p); An2[i] = mulmod(t,W[i],p); 
      i++;
      Ai=A[i];
      An2i=An2[i];
      A[i] = addmod(Ai,An2i,p2);
      An2[i]=((longlong(Ai)+p2-An2i)*W[i*step]) % p2; // t = submod(Ai,An2i,p); An2[i] = mulmod(t,W[i],p); 
    }
#endif
    // Step 2 : recursive calls
    fft2p2nopermafter(A, n/2, W,2*step);
    fft2p2nopermafter(An2, n/2, W,2*step);
  }  

  static void fft2p2( int *A, int n, int *W, int *T,int step=1) {  
    int i,n2,t;
    if ( n==1 ) return;
    // if p is fixed, the code is about 2* faster
    if (n==4){
      int w1=W[step];
#if 1
      int f0=A[0],f1=A[1],f2=A[2],f3=A[3],f01=mulmod(submod(f1,f3,p2),w1,p2),f02p=addmod(f0,f2,p2),f02m=submod(f0,f2,p2),f13=addmod(f1,f3,p2);
      A[0]=addmod(f02p,f13,p2);
      A[1]=addmod(f02m,f01,p2);
      A[2]=submod(f02p,f13,p2);
      A[3]=submod(f02m,f01,p2);
#else
      longlong f0=A[0],f1=A[1],f2=A[2],f3=A[3],f01=(f1-f3)*w1;
      A[0]=(f0+f1+f2+f3)%p2;
      A[1]=(f0-f2+f01)%p2;
      A[2]=(f0-f1+f2-f3)%p2;
      A[3]=(f0-f2-f01)%p2;
#endif
      return;
    }
    if (n==2){
      int f0=A[0],f1=A[1];
      A[0]=addmod(f0,f1,p2);
      A[1]=submod(f0,f1,p2);
      return;
    }
    n2 = n/2;
    // Step 1 : arithmetic
    int * Tn2=T+n2,*An2=A+n2;
    for( i=0; i<n2; ++i ) {
      int Ai,An2i;
      Ai=A[i];
      An2i=An2[i];
      T[i] = addmod(Ai,An2i,p2);
      t = submod(Ai,An2i,p2);
      Tn2[i] = mulmodp2(t,W[i*step]); 
      i++;
      Ai=A[i];
      An2i=An2[i];
      T[i] = addmod(Ai,An2i,p2);
      t = submod(Ai,An2i,p2);
      Tn2[i] = mulmodp2(t,W[i*step]); 
    }
    // Step 2 : recursive calls
    fft2p2(T, n2, W, A,2*step);
    fft2p2(Tn2, n2, W, A+n2,2*step);
    // Step 3 : permute
    for( i=0; i<n2; ++i ) {
      A[2*i] = T[i];
      A[2*i+1] = Tn2[i]; 
      ++i;
      A[2*i] = T[i];
      A[2*i+1] = Tn2[i]; 
    }
    return;
  }  

  static void fft2p3nopermbefore( int *A, int n, int *W,int step=1) {  
    if ( n==1 ) return;
    // if p is fixed, the code is about 2* faster
    if (n==4){
      int w1=W[step];
      int f0=A[0],f1=A[1],f2=A[2],f3=A[3],f01=mulmod(submod(f1,f3,p3),w1,p3),f02p=addmod(f0,f2,p3),f02m=submod(f0,f2,p3),f13=addmod(f1,f3,p3);
      A[0]=addmod(f02p,f13,p3);
      A[1]=addmod(f02m,f01,p3);
      A[2]=submod(f02p,f13,p3);
      A[3]=submod(f02m,f01,p3);
      return;
    }
    if (n==2){
      int f0=A[0],f1=A[1];
      A[0]=addmod(f0,f1,p3);
      A[1]=submod(f0,f1,p3);
      return;
    }
    fft2p3nopermbefore( A, n/2, W,2*step); // fft2p3nopermbefore(A,n2,W+n2);
    int * An2=A+n/2;
    fft2p3nopermbefore( An2, n/2, W,2*step); // fft2p3nopermbefore(An2,n2,W+n2);
#if 1
    int n2 = n/2*step; // n2%4==0
    int * Aend=An2;
    for(; A<Aend; ) {
      fft_loop_p3(A,An2,W,n2);
      ++A; ++An2; W +=step ;
      fft_loop_p3(A,An2,W,n2);
      ++A; ++An2; W +=step ;
      fft_loop_p3(A,An2,W,n2);
      ++A; ++An2; W += step;
      fft_loop_p3(A,An2,W,n2);
      ++A; ++An2; W +=step;
    }
#else
    for(int i=0; i<n/2; i +=4 ) {
      int s = A[i];
      int t = mulmodp3(W[i*step],An2[i]);
      A[i] = addmod(s,t,p3);
      An2[i] = submod(s,t,p3); 
      s = A[i+1];
      t = mulmodp3(W[(i+1)*step],An2[i+1]);
      A[i+1] = addmod(s,t,p3);
      An2[i+1] = submod(s,t,p3); 
      s = A[i+2];
      t = mulmodp3(W[(i+2)*step],An2[i+2]);
      A[i+2] = addmod(s,t,p3);
      An2[i+2] = submod(s,t,p3); 
      s = A[i+3];
      t = mulmodp3(W[(i+3)*step],An2[i+3]);
      A[i+3] = addmod(s,t,p3);
      An2[i+3] = submod(s,t,p3); 
    }
#endif
  }  

  static void fft2p3nopermafter( int *A, int n, int *W,int step=1) {  
    if ( n==1 ) return;
    // if p is fixed, the code is about 2* faster
    if (n==4){
      int w1=W[step];
      int f0=A[0],f1=A[1],f2=A[2],f3=A[3],f01=mulmod(submod(f1,f3,p3),w1,p3),f02p=addmod(f0,f2,p3),f02m=submod(f0,f2,p3),f13=addmod(f1,f3,p3);
      A[0]=addmod(f02p,f13,p3);
      A[1]=addmod(f02m,f01,p3);
      A[2]=submod(f02p,f13,p3);
      A[3]=submod(f02m,f01,p3);
      return;
    }
    if (n==2){
      int f0=A[0],f1=A[1];
      A[0]=addmod(f0,f1,p3);
      A[1]=submod(f0,f1,p3);
      return;
    }
    // Step 1 : arithmetic
    int *An2=A+n/2;
#if 1
    int * Acur=A,*An2cur=An2,*Wcur=W;
    int n2=n/2*step;
    for (;Acur!=An2;){
      int Ai,An2i;
      fft_loop_p3_(Acur,An2cur,Wcur,n2);
      ++Acur;++An2cur; Wcur +=step;
      fft_loop_p3_(Acur,An2cur,Wcur,n2);
      ++Acur;++An2cur; Wcur += step;
      fft_loop_p3_(Acur,An2cur,Wcur,n2);
      ++Acur;++An2cur; Wcur += step;
      fft_loop_p3_(Acur,An2cur,Wcur,n2);
      ++Acur;++An2cur; Wcur += step;
    }
#else
    for( i=0; i<n/2; ++i ) {
      int Ai,An2i;
      Ai=A[i];
      An2i=An2[i];
      A[i] = addmod(Ai,An2i,p3);
      An2[i]=((longlong(Ai)+p3-An2i)*W[i*step]) % p3; // t = submod(Ai,An2i,p); An2[i] = mulmod(t,W[i],p); 
      i++;
      Ai=A[i];
      An2i=An2[i];
      A[i] = addmod(Ai,An2i,p3);
      An2[i]=((longlong(Ai)+p3-An2i)*W[i*step]) % p3; // t = submod(Ai,An2i,p); An2[i] = mulmod(t,W[i],p); 
    }
#endif
    // Step 2 : recursive calls
    fft2p3nopermafter(A, n/2, W,2*step);
    fft2p3nopermafter(An2, n/2, W,2*step);
  }  

  static void fft2p3( int *A, int n, int *W, int *T,int step=1) {  
    int i,n2,t;
    if ( n==1 ) return;
    // if p is fixed, the code is about 2* faster
    if (n==4){
      int w1=W[step];
#if 1
      int f0=A[0],f1=A[1],f2=A[2],f3=A[3],f01=mulmod(submod(f1,f3,p3),w1,p3),f02p=addmod(f0,f2,p3),f02m=submod(f0,f2,p3),f13=addmod(f1,f3,p3);
      A[0]=addmod(f02p,f13,p3);
      A[1]=addmod(f02m,f01,p3);
      A[2]=submod(f02p,f13,p3);
      A[3]=submod(f02m,f01,p3);
#else
      longlong f0=A[0],f1=A[1],f2=A[2],f3=A[3],f01=(f1-f3)*w1;
      A[0]=(f0+f1+f2+f3)%p3;
      A[1]=(f0-f2+f01)%p3;
      A[2]=(f0-f1+f2-f3)%p3;
      A[3]=(f0-f2-f01)%p3;
#endif
      return;
    }
    if (n==2){
      int f0=A[0],f1=A[1];
      A[0]=addmod(f0,f1,p3);
      A[1]=submod(f0,f1,p3);
      return;
    }
    n2 = n/2;
    // Step 1 : arithmetic
    int * Tn2=T+n2,*An2=A+n2;
    for( i=0; i<n2; ++i ) {
      int Ai,An2i;
      Ai=A[i];
      An2i=An2[i];
      T[i] = addmod(Ai,An2i,p3);
      t = submod(Ai,An2i,p3);
      Tn2[i] = mulmodp3(t,W[i*step]); 
      i++;
      Ai=A[i];
      An2i=An2[i];
      T[i] = addmod(Ai,An2i,p3);
      t = submod(Ai,An2i,p3);
      Tn2[i] = mulmodp3(t,W[i*step]); 
    }
    // Step 2 : recursive calls
    fft2p3(T, n2, W, A,2*step);
    fft2p3(Tn2, n2, W, A+n2,2*step);
    // Step 3 : permute
    for( i=0; i<n2; ++i ) {
      A[2*i] = T[i];
      A[2*i+1] = Tn2[i]; 
      ++i;
      A[2*i] = T[i];
      A[2*i+1] = Tn2[i]; 
    }
    return;
  }  


  static void fft2p4nopermbefore( int *A, int n, int *W) {  
    int n2;
    if ( n==1 ) return;
    // if p is fixed, the code is about 2* faster
    const int p = 2113929217; 
    if (n==4){
      int w1=W[1];
      int f0=A[0],f1=A[1],f2=A[2],f3=A[3],f01=mulmod(submod(f1,f3,p),w1,p),f02p=addmod(f0,f2,p),f02m=submod(f0,f2,p),f13=addmod(f1,f3,p);
      A[0]=addmod(f02p,f13,p);
      A[1]=addmod(f02m,f01,p);
      A[2]=submod(f02p,f13,p);
      A[3]=submod(f02m,f01,p);
      return;
    }
    if (n==2){
      int f0=A[0],f1=A[1];
      A[0]=addmod(f0,f1,p);
      A[1]=submod(f0,f1,p);
      return;
    }
    n2 = n/2;
    fft2p4nopermbefore( A,    n2, W+n2);
    int * An2=A+n2;
    fft2p4nopermbefore( An2, n2, W+n2);
#if 1
    int * Aend=An2;
    for(; A<Aend; ) {
      int s = *A;
      int t = mulmod(*W,*An2,p);
      *A = addmod(s,t,p);
      *An2 = submod(s,t,p); 
      ++A; ++An2; ++W;
      s = *A;
      t = mulmod(*W,*An2,p);
      *A = addmod(s,t,p);
      *An2 = submod(s,t,p); 
      ++A; ++An2; ++W;
      s = *A;
      t = mulmod(*W,*An2,p);
      *A = addmod(s,t,p);
      *An2 = submod(s,t,p); 
      ++A; ++An2; ++W;
      s = *A;
      t = mulmod(*W,*An2,p);
      *A = addmod(s,t,p);
      *An2 = submod(s,t,p); 
      ++A; ++An2; ++W;
    }
#else
    for( i=0; i<n2; i++ ) {
      int s = A[i];
      int t = mulmod(W[i],An2[i],p);
      A[i] = addmod(s,t,p);
      An2[i] = submod(s,t,p); 
      ++i;
      s = A[i];
      t = mulmod(W[i],An2[i],p);
      A[i] = addmod(s,t,p);
      An2[i] = submod(s,t,p); 
      ++i;
      s = A[i];
      t = mulmod(W[i],An2[i],p);
      A[i] = addmod(s,t,p);
      An2[i] = submod(s,t,p); 
      ++i;
      s = A[i];
      t = mulmod(W[i],An2[i],p);
      A[i] = addmod(s,t,p);
      An2[i] = submod(s,t,p); 
    }
#endif
  }  

  static void fft2p4nopermafter( int *A, int n, int *W) {  
    int n2;
    if ( n==1 ) return;
    // if p is fixed, the code is about 2* faster
    const int p = 2113929217 ;
    if (n==4){
      int w1=W[1];
      int f0=A[0],f1=A[1],f2=A[2],f3=A[3],f01=mulmod(submod(f1,f3,p),w1,p),f02p=addmod(f0,f2,p),f02m=submod(f0,f2,p),f13=addmod(f1,f3,p);
      A[0]=addmod(f02p,f13,p);
      A[1]=addmod(f02m,f01,p);
      A[2]=submod(f02p,f13,p);
      A[3]=submod(f02m,f01,p);
      return;
    }
    if (n==2){
      int f0=A[0],f1=A[1];
      A[0]=addmod(f0,f1,p);
      A[1]=submod(f0,f1,p);
      return;
    }
    n2 = n/2;
    // Step 1 : arithmetic
    int *An2=A+n2;
#if 1
    int * Acur=A,*An2cur=An2,*Wcur=W;
    for (;Acur!=An2;){
      int Ai,An2i;
      Ai=*Acur;
      An2i=*An2cur;
      *Acur = addmod(Ai,An2i,p);
      *An2cur=((longlong(Ai)+p-An2i)* *Wcur) % p;
      ++Acur;++An2cur;++Wcur;
      Ai=*Acur;
      An2i=*An2cur;
      *Acur = addmod(Ai,An2i,p);
      *An2cur=((longlong(Ai)+p-An2i)* *Wcur) % p;
      ++Acur;++An2cur;++Wcur;
    }
#else
    for( i=0; i<n2; ++i ) {
      int Ai,An2i;
      Ai=A[i];
      An2i=An2[i];
      A[i] = addmod(Ai,An2i,p);
      An2[i]=((longlong(Ai)+p-An2i)*W[i]) % p; // t = submod(Ai,An2i,p); An2[i] = mulmod(t,W[i],p);     
      i++;
      Ai=A[i];
      An2i=An2[i];
      A[i] = addmod(Ai,An2i,p);
      An2[i]=((longlong(Ai)+p-An2i)*W[i]) % p; // t = submod(Ai,An2i,p); An2[i] = mulmod(t,W[i],p);     
    }
#endif
    // Step 2 : recursive calls
    fft2p4nopermafter(A, n2, W+n2);
    fft2p4nopermafter(An2, n2, W+n2);
  }  

  static void fft2( int *A, int n, int *W, int p, int *T ,bool permute=true) {  
    int i,n2,t;
    if ( n==1 ) return;
    if (p==2013265921){
      fft2p1(A,n,W,T);
      return;
    }
    if (p==1811939329){
      fft2p2(A,n,W,T);
      return;
    }
    if (n==4){
      int w1=W[1];
#if 1
      int f0=A[0],f1=A[1],f2=A[2],f3=A[3],f01=mulmod(submod(f1,f3,p),w1,p),f02p=addmod(f0,f2,p),f02m=submod(f0,f2,p),f13=addmod(f1,f3,p);
      A[0]=addmod(f02p,f13,p);
      A[1]=addmod(f02m,f01,p);
      A[2]=submod(f02p,f13,p);
      A[3]=submod(f02m,f01,p);
#else
      longlong f0=A[0],f1=A[1],f2=A[2],f3=A[3],f01=(f1-f3)*w1;
      A[0]=(f0+f1+f2+f3)%p;
      A[1]=(f0-f2+f01)%p;
      A[2]=(f0-f1+f2-f3)%p;
      A[3]=(f0-f2-f01)%p;
#endif
      return;
    }
    if (n==2){
      int f0=A[0],f1=A[1];
      A[0]=addmod(f0,f1,p);
      A[1]=submod(f0,f1,p);
      return;
    }
    n2 = n/2;
    // Step 1 : arithmetic
    int * Tn2=T+n2,*An2=A+n2;
    for( i=0; i<n2; i++ ) {
      int Ai,An2i;
      Ai=A[i];
      An2i=An2[i];
      T[i] = addmod(Ai,An2i,p);
      t = submod(Ai,An2i,p);
      Tn2[i] = mulmod(t,W[i],p); 
    }
    // Step 2 : recursive calls
    fft2(T, n2, W+n2, p, A,permute);
    fft2(Tn2, n2, W+n2, p, A+n2,permute);
    if (!permute){ 
      for( i=0; i<n2; i++ ) {
	A[2*i] = T[2*i];
	A[2*i+1] = T[2*i+1]; 
      }
      return;
    }
    // Step 3 : permute
    for( i=0; i<n2; i++ ) {
      A[2*i] = T[i];
      A[2*i+1] = Tn2[i]; 
    }
    return;
  }  


  void fft2wp4(vector<int> & W,int n,int w){
    W.reserve(n); 
    const int p = p4;
    w=w % p;
    if (w<0) w += p;
    longlong wk=w;
    for (int N=n/2;N;N/=2,wk=(wk*wk)%p){
      int ww=1;
      for (int i=0;i<N;ww=(ww*wk)%p,++i){
	W.push_back(ww);
      }
    }
  }

  void fft2w(vector<int> & W,int n,int w,int p){
    W.reserve(n); 
    w=w % p;
    if (w<0) w += p;
    longlong wk=w;
    for (int N=n/2;N;N/=2,wk=(wk*wk)%p){
      int ww=1;
      for (int i=0;i<N;ww=(ww*wk)%p,++i){
	W.push_back(ww);
      }
    }
  }

  void fft2(int * A, int n, int w, int p,bool permute){
#ifndef FXCG
    if (debug_infolevel>2)
      CERR << CLOCK()*1e-6 << " begin fft2 int " << n << " memory " << memory_usage()*1e-6 << "M" << '\n';
#endif
    vector<int> W,T(n);
    fft2w(W,n,w,p);
    int * Aend=A+n;
    for (int * a=A;a<Aend;++a)
      if (*a<0) *a += p;
    fft2(A,n,&W.front(),p,&T.front(),permute);
    for (int * a=A;a<Aend;++a)
      if (*a<0) *a += p;
#ifndef FXCG
    if (debug_infolevel>2)
      CERR << CLOCK()*1e-6 << " end fft int " << n << " memory " << memory_usage()*1e-6 << "M" << '\n';
#endif
  }

  void makepositive(int * p,int n,int modulo){
    int * pend=p+n;
    for (;p!=pend;++p){
      int P=*p;
      if (P>=0) continue;
      P += modulo;
      P += (unsigned(P)>>31)*modulo;
      *p=P;
    }
  }

  // copy source to target in reverse order
  void reverse_copy(const vector<int> & source,vector<int> & target){
    const int * sb=&source.front(), * s=sb+source.size();
    int * t=&target.front(), * tend=t+target.size();
    for (;s>sb && t<tend;){
      --s;
      *t=*s;
      ++t;
    }
    for (;t<tend;++t)
      *t=0;
  }

  void makemodulop(int * a,int as,int modulo){
    int *aend=a+as;
    if (modulo==p3){
      for (;a!=aend;++a)
	*a %= p3;
      return;
    }
    if (modulo==p2){
      for (;a!=aend;++a)
	*a %= p2;
      return;
    }
    if (modulo==p1){
      for (;a!=aend;++a)
	*a %= p1;
      return;
    }
    for (;a!=aend;++a){
      *a %= modulo;
      // if (*a<0) *a += modulo; // *a -= (unsigned(modulo-*a)>>31)*modulo;
    }
  }

  // res=a*b mod p
  bool fft2mult(int ablinfnorm,const vector<int> & a,const vector<int> & b,vector<int> & res,int modulo,vector<int> & W,vector<int> & fftmult_p,vector<int> & fftmult_q,bool reverseatend,bool dividebyn,bool makeplus){
    int as=int(a.size()),bs=int(b.size()),rs=as+bs-1;
    int logrs=sizeinbase2(rs);
    if (logrs>(modulo==p1?27:25)) return false;
    int n=(1u<<logrs);
    W.reserve(n);
    res.resize(n);
#if 1
    //fftmult_p.clear();
    fftmult_p.resize(n);
    //fftmult_q.clear();
    fftmult_q.resize(n);
    reverse_copy(a,fftmult_p);
    reverse_copy(b,fftmult_q);
#else
    fftmult_p=a;fftmult_q=b;
    reverse(fftmult_p.begin(),fftmult_p.end());
    fftmult_p.resize(n);
    reverse(fftmult_q.begin(),fftmult_q.end());
    fftmult_q.resize(n);
#endif
    if (ablinfnorm>modulo){
      makemodulop(&fftmult_p.front(),as,modulo);
      makemodulop(&fftmult_q.front(),bs,modulo);
    }
    // r:=1227303670; w:=powmod(r,2^(27-logrs),p1); 
    // fft(p,w,p1);fft(q,w,p1); res=p.*q; ifft(res,w,p1);
    int r=1227303670;
    if (modulo==p1){
      if (debug_infolevel>3)
	CERR << CLOCK()*1e-6 << " make+ p1 begin" << '\n';
      if (makeplus){
	makepositive(&fftmult_p.front(),as,p1);
	makepositive(&fftmult_q.front(),bs,p1);
      }
      if (debug_infolevel>3)
	CERR << CLOCK()*1e-6 << " make+ p1 end" << '\n';
      int w=powmod(r,(1u<<(27-logrs)),p1);
#if 0
      W.clear();
      fft4wp1(W,n,w);
      fft4p1nopermafter(&fftmult_p.front(),n,&W.front());
      fft4p1nopermafter(&fftmult_q.front(),n,&W.front());
      for (int i=0;i<n;++i){
	fftmult_p[i]=mulmod(fftmult_p[i],fftmult_q[i],p1);
      }
      w=invmod(w,p1); if (w<0) w+=p1;
      W.clear();
      fft2wp1(W,n,w);
      fft2p1nopermbefore(&fftmult_p.front(),n,&W.front());
#else
      if (W.empty() || W[0]==0){
	W.clear();
	fft2wp1(W,n,w);
      }
      fft2p1nopermafter(&fftmult_p.front(),n,&W.front());
      fft2p1nopermafter(&fftmult_q.front(),n,&W.front());
      for (int i=0;i<n;++i){
	fftmult_p[i]=mulmodp1(fftmult_p[i],fftmult_q[i]);
      }
      // vector<int> WW(W); fft_reverse(WW,p1);
      fft_reverse(W,p1);
      //w=invmod(w,p1); if (w<0) w+=p1; W.clear(); fft2wp1(W,n,w);
      fft2p1nopermbefore(&fftmult_p.front(),n,&W.front());
      fft_reverse(W,p1);
#endif
      fftmult_p.resize(rs);
      if (dividebyn){
	int ninv=invmod(n,p1); if (ninv<0) ninv+=p1;
	for (int i=0;i<rs;++i){
	  fftmult_p[i]=mulmod(ninv,fftmult_p[i],p1);
	  if (fftmult_p[i]>p1/2)
	    fftmult_p[i]-=p1;
	}
      }
      if (reverseatend)
	reverse(fftmult_p.begin(),fftmult_p.end());
      res.swap(fftmult_p);
      return true;
    }
    if (modulo==p2){// p2 := 1811939329 ; r:=814458146; order 2^26 
      r=814458146;
      int w=powmod(r,(1u<<(26-logrs)),p2);
      if (makeplus){
	if (debug_infolevel>3)
	  CERR << CLOCK()*1e-6 << " make+ p2 begin" << '\n';
	makepositive(&fftmult_p.front(),as,p2);
	makepositive(&fftmult_q.front(),bs,p2);
	if (debug_infolevel>3)
	  CERR << CLOCK()*1e-6 << " make+ p2 end" << '\n';
      }
#if 0
      W.clear();
      fft4wp2(W,n,w);
      fft4p2nopermafter(&fftmult_p.front(),n,&W.front());
      fft4p2nopermafter(&fftmult_q.front(),n,&W.front());
      for (int i=0;i<n;++i){
	fftmult_p[i]=mulmodp2(fftmult_p[i],fftmult_q[i]);
      }
      w=invmod(w,p2); if (w<0) w+=p2;
      W.clear();
      fft2wp2(W,n,w);
      fft2p2nopermbefore(&fftmult_p.front(),n,&W.front());
#else
      if (W.empty() || W[0]==0){
	W.clear();
	fft2wp2(W,n,w);
      }
      fft2p2nopermafter(&fftmult_p.front(),n,&W.front());
      fft2p2nopermafter(&fftmult_q.front(),n,&W.front());
      for (int i=0;i<n;++i){
	fftmult_p[i]=mulmodp2(fftmult_p[i],fftmult_q[i]);
      }
      fft_reverse(W,p2);
      // w=invmod(w,p2); if (w<0) w+=p2; W.clear(); fft2wp2(W,n,w);
      fft2p2nopermbefore(&fftmult_p.front(),n,&W.front());
      fft_reverse(W,p2);
#endif
      fftmult_p.resize(rs);
      if (dividebyn){
	int ninv=invmod(n,p2); if (ninv<0) ninv+=p2;
	for (int i=0;i<rs;++i){
	  fftmult_p[i]=mulmod(ninv,fftmult_p[i],p2);
	  if (fftmult_p[i]>p2/2)
	    fftmult_p[i]-=p2;
	}
      }
      if (reverseatend)
	reverse(fftmult_p.begin(),fftmult_p.end());
      res.swap(fftmult_p);
      return true;
    }
    if (modulo==p3){// order 2^26
      r=2187;
      int w=powmod(r,(1u<<(26-logrs)),p3);
      if (makeplus){
	makepositive(&fftmult_p.front(),as,p3);
	makepositive(&fftmult_q.front(),bs,p3);
      }
      if (W.empty() || W[0]==0){
	W.clear();
	fft2wp3(W,n,w);
      }
      fft2p3nopermafter(&fftmult_p.front(),n,&W.front());
      fft2p3nopermafter(&fftmult_q.front(),n,&W.front());
      for (int i=0;i<n;++i){
	fftmult_p[i]=mulmodp3(fftmult_p[i],fftmult_q[i]);
      }
      fft_reverse(W,p3);
      // w=invmod(w,p3); if (w<0) w+=p3; W.clear(); fft2wp3(W,n,w);
      fft2p3nopermbefore(&fftmult_p.front(),n,&W.front());
      fft_reverse(W,p3);
      fftmult_p.resize(rs);
      if (dividebyn){
	int ninv=invmod(n,p3); if (ninv<0) ninv+=p3;
	for (int i=0;i<rs;++i){
	  fftmult_p[i]=mulmod(ninv,fftmult_p[i],p3);
	  if (fftmult_p[i]>p3/2)
	    fftmult_p[i]-=p3;
	}
      }
      if (reverseatend)
	reverse(fftmult_p.begin(),fftmult_p.end());
      res.swap(fftmult_p);
      return true;
    }
    if (modulo==p4){// order 2^25
      r=1971140334;
      int w=powmod(r,(1u<<(25-logrs)),p4);
      if (makeplus){
	makepositive(&fftmult_p.front(),as,p4);
	makepositive(&fftmult_q.front(),bs,p4);
      }
      if (W.empty() || W[0]==0){ 
	W.clear();
	fft2wp4(W,n,w);
      }
      fft2p4nopermafter(&fftmult_p.front(),n,&W.front());
      fft2p4nopermafter(&fftmult_q.front(),n,&W.front());
      for (int i=0;i<n;++i){
	fftmult_p[i]=mulmodp4(fftmult_p[i],fftmult_q[i]);
      }
      fft_reverse(W,p4);
      // w=invmod(w,p4); if (w<0) w+=p4; W.clear(); fft2wp4(W,n,w);
      fft2p4nopermbefore(&fftmult_p.front(),n,&W.front());
      fft_reverse(W,p4);
      fftmult_p.resize(rs);
      if (dividebyn){
	int ninv=invmod(n,p4); if (ninv<0) ninv+=p4;
	for (int i=0;i<rs;++i){
	  fftmult_p[i]=mulmod(ninv,fftmult_p[i],p4);
	  if (fftmult_p[i]>p4/2)
	    fftmult_p[i]-=p4;
	}
      }
      if (reverseatend)
	reverse(fftmult_p.begin(),fftmult_p.end());
      res.swap(fftmult_p);
      return true;
    }
    return false;
  }

  void fft(int * f,int n,const int * w,int m,int * t,int p){
    if (n==1)
      return ;
    int step=m/n;
    int k=0;
    if (n%2){
      for (k=3;k*k<=n;k++){
	if (!(n%k))
	  break;
      }
    }
    else
      k=2;
    if (k*k>n){ 
      // prime size, slow discrete Fourier transform
      int *fj,*fend_=f+n-3,*fend=f+n;
      int * res=t;
      for (int i=0;i<n;++i){
	int tmp (0);
	int pos=0,istep=i*step;
	for (fj=f;fj<fend_;fj+=3){
	  tmp =  (tmp + longlong(fj[0])*w[pos])%p;
	  pos += istep-m; pos += (unsigned(pos)>>31)*m;// pos = (pos+istep)%m;
	  tmp =  (tmp + longlong(fj[1])*w[pos])%p;
	  pos += istep-m; pos += (unsigned(pos)>>31)*m;// pos = (pos+istep)%m;
	  tmp =  (tmp + longlong(fj[2])*w[pos])%p;
	  pos += istep-m; pos += (unsigned(pos)>>31)*m;// pos = (pos+istep)%m;
	}
	for (;fj<fend;++fj){
	  tmp =  (tmp + longlong(fj[0])*w[pos])%p;
	  pos += istep-m; pos += (unsigned(pos)>>31)*m;// pos = (pos+istep)%m;
	}
	*res=tmp;
	++res;
      }
      for (fj=f,res=t;fj<fend;++fj,++res){
	*fj=*res;
      }
      return;
    }
    if (k!=2){
      // assumes n is divisible by k, nk=n/k
      // P(X)=P_k(X)*[X^nk]^(k-1)+...+P_1(X) degree(P_k)<nk
      // P(w^(kj+l))= Q_l ( (w^k)^j )
      // with Q_l=P_1^(w^l)+w^(nk)*P_2^(w^l)+...
      unsigned long n2=n/k;
      for (int j=0;j<k;j++){
	// find Q[j]
	int * Qj=t+n2*j;
	for (unsigned i=0;i<n2;i++){
	  longlong tmp(0);
	  int pos=0,jn2step=j*n2*step;
	  const int * fi=&f[i], *fiend=fi+k*n2;
	  for (;fi<fiend;fi+=n2){
	    tmp = (tmp+longlong(*fi)*w[pos]) % p;
	    pos += jn2step-m; pos += (unsigned(pos)>>31)*m;
	  }
	  Qj[i]=(tmp*w[j*step*i])%p;
	}
      }
      for (int j=0;j<k;++j){
	fft(t+n2*j,n2,w,m,f+n2*j,p);
      }
      // build fft
      for (unsigned i=0;i<n2;++i){
	for (int j=0;j<k;++j,++f)
	  *f=t[n2*j+i];
      }
      return;
    }
    // Compute r0=sum_[j<n/2] (f_j+f_(j+n/2))*x^j
    // and r1=sum_[j<n/2] (f_j-f_(j+n/2))*omega^[step*j]*x^j
    unsigned long n2=n/2;
    int * r0=t, *r1=t+n2;
    int * it=f,*itn=f+n2,*itend=itn;
    const int *itk=w;
    for (;it!=itend;++itn,itk+=step,++it,++r0,++r1){
      longlong a(*it),b(*itn);
      *r0=(a+b)%p;
      *r1=((a-b)*(*itk))%p;
    }
    // Recursive call
    int * r0f=f,*r1f=f+n2;
    fft(t,n2,w,m,r0f,p);
    fft(t+n2,n2,w,m,r1f,p);
    // Return a mix of r0/r1
    it=t; itend=t+n2; itn=t+n2;
    for (;it!=itend;){
      *f=*it;
      ++it; ++f;
      *f=*itn;
      ++itn; ++f;
    }
  }

  void fft(const vector<int> & f,const vector<int> & w ,vector<int> & res,int modulo){
#if 1
    res=f;
    vector<int> tmp(w.size());
    fft(&res.front(),int(res.size()),&w.front(),int(w.size()),&tmp.front(),modulo);
    return;
#endif
    // longlong M=longlong(modulo)*modulo;
    unsigned long n=long(f.size()); // unsigned long does not parse with gcc
    if (n==4){
      int w1=w[w.size()/4];
      longlong f0=f[0],f1=f[1],f2=f[2],f3=f[3],f01=(f1-f3)*w1;
      res.resize(4);
      res[0]=(f0+f1+f2+f3)%modulo;
      res[1]=(f0-f2+f01)%modulo;
      res[2]=(f0-f1+f2-f3)%modulo;
      res[3]=(f0-f2-f01)%modulo;
      return;
    }
    if (n==1){
      res = f;
      return ;
    }
    unsigned long m=long(w.size());
    unsigned long step=m/n;
    unsigned k=0;
    if (n%2){
      for (k=3;k*k<=n;k++){
	if (!(n%k))
	  break;
      }
    }
    else
      k=2;
    if (k*k>n){ 
      // prime size, slow discrete Fourier transform
      res.clear();
      res.reserve(n);
      longlong tmp;
      unsigned pos;
      for (unsigned i=0;i<n;++i){
	tmp = 0;
	pos = 0;
	for (unsigned j=0;j<n;++j){
	  tmp = (tmp + longlong(f[j])*w[pos])%modulo;
	  pos = (pos+i*step)%m;
	}
	res.push_back(int(tmp));
      }
      return;
    }
    if (k!=2){
      // assumes n is divisible by k, nk=n/k
      // P(X)=P_k(X)*[X^nk]^(k-1)+...+P_1(X) degree(P_k)<nk
      // P(w^(kj+l))= Q_l ( (w^k)^j )
      // with Q_l=P_1^(w^l)+w^(nk)*P_2^(w^l)+...
      unsigned long n2=n/k;
      vector< vector<int> > Q(k),Qfft(k);
      for (unsigned j=0;j<k;++j)
	Q[j]=vector<int>(n2,0);
      longlong tmp;
      for (unsigned j=0;j<k;j++){
	// find Q[j]
	for (unsigned i=0;i<n2;i++){
	  tmp=0;
	  for (unsigned J=0;J<k;J++){
	    tmp = (tmp+longlong(f[J*n2+i])*w[(J*j*n2*step)%m])%modulo;
	  }
	  tmp=(tmp*w[j*step*i])%modulo;
	  Q[j][i]=int(tmp);
	}
	fft(Q[j],w,Qfft[j],modulo);
      }
      // build fft
      res.clear();
      res.reserve(n);
      for (unsigned i=0;i<n2;++i){
	for (unsigned j=0;j<k;++j)
	  res.push_back(Qfft[j][i]);
      }
      return;
    }
    // Compute r0=sum_[j<n/2] (f_j+f_(j+n/2))*x^j
    // and r1=sum_[j<n/2] (f_j-f_(j+n/2))*omega^[step*j]*x^j
    unsigned long n2=n/2;
    vector<int> r0,r1;
    r0.reserve(n2); r1.reserve(n2);
    vector<int>::const_iterator it=f.begin(),itn=it+n2,itend=itn,itk=w.begin();
    for (;it!=itend;++itn,itk+=step,++it){
      longlong a(*it),b(*itn);
      r0.push_back((a+b)%modulo);
      r1.push_back(((a-b)*(*itk))%modulo);
    }
    // Recursive call
    vector<int> r0f(n2);
    fft(r0,w,r0f,modulo); // r0 is not used anymore, alias for r1f
    fft(r1,w,r0,modulo);
    // Return a mix of r0/r1
    res.clear();
    res.reserve(n);
    it=r0f.begin(); itend=it+n2; itn=r0.begin();
    for (;it!=itend;){
      res.push_back(*it);
      ++it;
      res.push_back(*itn);
      ++itn;
    }
  }


  // Convolution of p and q, omega a n-th root of unity, n=2^k
  // WARNING p0 and q0 are given in ascending power
  void fftconv(const modpoly & p,const modpoly & q,unsigned long k,unsigned long n,const gen & omega,modpoly & pq,environment * env){
    vecteur w;
    w.reserve(n);
    w.push_back(1);
    gen omegan(omega),tmp;
    for (unsigned long i=1;i<n;++i){
      w.push_back(omegan);
      omegan=omegan*omega;
      if (env && env->moduloon)
	omegan=smod(omegan,env->modulo);
    }
    modpoly alpha(n),beta(n),gamma(n);
    fft(p,w,alpha,env);
    fft(q,w,beta,env);
    for (unsigned long i=0;i<n;++i){
      tmp=alpha[i]*beta[i];
      if (env && env->moduloon)
	gamma[i]=smod(tmp,env->modulo);
      else
	gamma[i]=tmp;
    }
    vecteur winv(1,1);
    winv.reserve(n);
    for (unsigned long i=1;i<n;++i)
      winv.push_back(w[n-i]);
    fft(gamma,winv,pq,env);
    pq=pq/gen(int(n));
    /*
    modpoly check(n);
    fft(alpha,winv,check,env);
    check=check/gen(int(n));
   */
  }

  // Convolution of p and q, omega a n-th root of unity, n=2^k
  // p and q are given in descending power order
  void fftconv(const modpoly & p0,const modpoly & q0,unsigned long k,const gen & omega,modpoly & pq,environment * env){
    unsigned long n= 1u <<k;
    // Adjust sizes
    modpoly p(p0),q(q0);
    reverse(p.begin(),p.end());
    reverse(q.begin(),q.end());
    unsigned long ps=long(p.size()),qs=long(q.size());
    for (unsigned long i=ps;i<n;++i)
      p.push_back(0);
    for (unsigned long i=qs;i<n;++i)
      q.push_back(0);
    fftconv(p,q,k,n,omega,pq,env);
    reverse(pq.begin(),pq.end());
    pq=trim(pq,env);
  }

  void vectorlonglong2vecteur(const vector<longlong> & v,vecteur & w){
    size_t s=v.size();
    w.resize(s);
    for (size_t i=0;i<s;++i)
      w[i]=v[i];
  }

  void vecteur2vectorint(const vecteur & v,int p,vector<int> & res){
    vecteur::const_iterator it=v.begin(),itend=v.end();
    res.clear();
    res.reserve(itend-it);
    int tmp;
    if (p==0){
      for (;it!=itend;++it){
	tmp=it->val;
	tmp += (unsigned(tmp)>>31)*p; // make it positive now!
	res.push_back(tmp);
      }
    }
    else {
      for (;it!=itend;++it){
	if (it->type==_ZINT)
	  tmp=modulo(*it->_ZINTptr,p);
	else
	  tmp=it->val % p;
	tmp += (unsigned(tmp)>>31)*p; // make it positive now!
	res.push_back(tmp);
      }
    }
  } 

  struct thread_fftmult_t {
    const vecteur * p,*q;
    gen P,Q;
    vecteur * res;
    int prime;
    vector<int> * a,*b,*resp1,*resp2,*resp3,*Wp1,*Wp2,*Wp3,*Wp4,*tmp_p,*tmp_q;
  };


  void * do_thread_fftmult(void * ptr_){
    thread_fftmult_t * ptr=(thread_fftmult_t *) ptr_;
    modpoly curres;
    if (fftmultp1234(*ptr->p,*ptr->q,ptr->P,ptr->Q,curres,ptr->prime,*ptr->a,*ptr->b,*ptr->resp1,*ptr->resp2,*ptr->resp3,*ptr->Wp1,*ptr->Wp2,*ptr->Wp3,*ptr->Wp4,*ptr->tmp_p,*ptr->tmp_q,false))
      return ptr;
    return 0;
  }

  // valid values for nbits=24 or 16, zsize>=2
#if !defined USE_GMP_REPLACEMENTS && !defined BF2GMP_H
  static void zsplit(const vecteur & p, int zsize,int nbits,vector<int> & pz){
    size_t s=p.size();
    int * target=&pz[0];
    int nbytes=nbits/8;
    int mask=0xffffff;
    if (nbits==16)
      mask=0xffff;
    vector<unsigned> tmp(zsize+2);
    for (size_t i=0;i<s;++i,target+=zsize){
      gen z=p[i];
      if (z.type==_INT_){
	int Z=z.val;
	if (Z>0){
	  *target = Z & mask;
	  target[1] = Z >> nbits;
	}
	else {
	  Z=-Z;
	  *target = -(Z & mask);
	  target[1] = -(Z >> nbits);
	}
      }
      else {
	size_t countp=0;
	for (int j=0;j<zsize+2;++j)
	  tmp[j]=0;
	mpz_export(&tmp[0],&countp,-1,4,0,0,*z._ZINTptr);
	if (nbits==16){
	  for (int i=0;i<countp;++i){
	    target[2*i]=tmp[i] & 0xffff;
	    target[2*i+1]=tmp[i] >> 16;
	  }
	}
	else {
	  int * targetsave=target;
	  for (int i=0;i<countp;i+=3){
	    *target=tmp[i] & 0xffffff;
	    ++target;
	    *target=((tmp[i+1]&0xffff) << 8) | (tmp[i]>>24);
	    ++target;
	    *target=((tmp[i+2]&0xff)<< 16) | (tmp[i+1]>>16);
	    ++target;
	    *target=tmp[i+2] >> 8;
	    ++target;
	  }
	  target = targetsave; 
	}
	if (mpz_sgn(*z._ZINTptr)<0){
	  for (int i=0;i<zsize;++i)
	    target[i]=-target[i];
	}
      }
    }
  }
#endif

#if !defined USE_GMP_REPLACEMENTS && !defined BF2GMP_H
  // pz is not const because we modify it in place for sign/carries handling
  static void zbuild(vector<longlong> & pz,int zsize,int nbits,vecteur & p){
    size_t s=pz.size()/zsize;
    int base=1<<nbits;
    longlong base2=longlong(base)*base;
    int nbytes=nbits/8;
    longlong mask=0xffffffLL;
    int nbits2=2*nbits;
    if (nbits==16){
      mask=0xffff;
    }
    vector<int> tmp(zsize+5);
    vector<unsigned> tmp2(zsize+2);
    mpz_t z;
    mpz_init(z);
    longlong * source=&pz[0];
    for (size_t i=0;i<s;++i){
      // handle sign/carry from source[0..zsize-1] to tmp[0..zsize+2]
      longlong * end=source+zsize;
      longlong * begin=source;
      // find sign
      for (--end;end>=begin;--end){
	if (*end)
	  break;
      }
      if (end<begin){
	source += zsize;
	continue; // coeff in p is 0
      }
      // check previous for carry
      longlong U=*end;
      if (end>begin && U/(1<<nbits)==0){
	*end=0;
	--end;
	*end += U*(1<<nbits);
	U=*end;
      }
      int sign=ulonglong(U)>>63; // 1 for neg, 0 for positive
      ++end;
      if (sign){
	for (;begin<end;++begin){
	  *begin=-*begin;
	}
      }
      // now make all coeff positive
      longlong finalcarry=0; int finalpow2=0;      
      begin=source;
      for (;;){
	if (*begin>=0){
	  ++begin;
	  if (begin==end)
	    break;
	  continue;
	}
	longlong s=1+(ulonglong(-*begin)>>nbits); 
	*begin += s*base;
	++begin;
	if (begin==end){
	  if (end==source+zsize){
	    CERR << "unexpected carry" << '\n';
	    break;
	  }
	  finalcarry=sign?s:-s;
	  finalpow2=end-source;
	  break;
	}
	*begin -= s;
      }
      // make all coeff smaller than base
      for (int j=0;j<zsize+5;++j)
	tmp[j]=0;
      int * ptr=&tmp[0];
      begin=source;
      for (;;){
	*ptr=(*begin) & mask;
	++ptr; 
	if (begin+1==end) 
	  break;
	begin[1] += (ulonglong(*begin) >> nbits);
	++begin;
      }
      *ptr = (ulonglong(*begin) >> nbits) & mask; ++ptr;
      *ptr = (ulonglong(*begin) >> (2*nbits)) & mask; ++ptr;
      if (nbits==16)
	*ptr = (ulonglong(*begin) >> 48) & mask;
      source += zsize;
      // base 2^16/2^24 to 2^32
      for (int j=0;j<zsize+2;++j)
	tmp2[j]=0;
      if (nbits==16){
	int s =(zsize+2)/2;
	for (int i=0;i<s;++i){
	  tmp2[i]=tmp[2*i] | (unsigned(tmp[2*i+1])<<16);
	}
      }
      else {
	int j=0;
	for (int i=0;i<zsize+2;i+=4){
	  tmp2[j]=tmp[i] | ((unsigned(tmp[i+1])&0xff)<<24);
	  ++j;
	  tmp2[j]=(tmp[i+1]>>8) | ((unsigned(tmp[i+2])&0xffff)<<16);
	  ++j;
	  tmp2[j]=(tmp[i+2]>>16) | (unsigned(tmp[i+3])<<8);
	  ++j;
	}
      }
      mpz_import(z,zsize,-1,4,0,0,&tmp2[0]);
      if (sign)
	mpz_neg(z,z);
      if (mpz_sizeinbase(z,2)<31)
	p[i]=mpz_get_si(z);
      else
	p[i]=z;
      if (finalcarry){
	p[i] = p[i]+gen(finalcarry)*pow(plus_two,finalpow2*nbits,context0);
      }
    }
    mpz_clear(z);
  }
#endif

  // ichinrem reconstruct in resp1 from resp1/resp2
  void ichinremp1p2(const std::vector<int> & resp1,const std::vector<int> & resp2,int n,std::vector<int> & Res,int modulo){
    size_t rs=resp1.size();
    if (&resp1!=&Res)
      Res.resize(rs);
    if (debug_infolevel>2)
      CERR << CLOCK()*1e-6 << " begin ichinremp1p2 mod " << modulo << '\n';
    int p1modinv=-9;//invmod(p1,p2);
    int modulo2=modulo/2;
    int n1=invmod(n,p1); if (n1<0) n1+=p1;
    int n2=invmod(n,p2); if (n2<0) n2+=p2;
    for (int i=0;i<rs;++i){
      int A=resp1[i],B=resp2[i];
      A=mulmod(n1,A,p1);
      B=mulmod(n2,B,p2);
      // a mod p1, b mod p2 -> res mod p1*p2
      longlong res=A+((longlong(p1modinv)*(B-longlong(A)))%p2)*p1;
      //res += (ulonglong(res)>>63)*p1p2; res -= (ulonglong(p1p2/2-res)>>63)*modulo;
      if (res>p1p2sur2) res-=p1p2; else if (res<=-p1p2sur2) res+=p1p2;
      //while (res>p1p2sur2) res-=p1p2; while (res<-p1p2sur2) res+=p1p2;
      A=res % modulo;
      A += (unsigned(A)>>31)*modulo; // A now positive
      A -= (unsigned(modulo2-A)>>31)*modulo; 
      // if (A>modulo2) A-=modulo;
      Res[i]=A;
    }
    if (debug_infolevel>2)
      CERR << CLOCK()*1e-6 << " end ichinremp1p2 mod " << modulo << '\n';
  }

  // ichinrem reconstruct in resp1 from resp1/resp2/resp3
  void ichinremp1p2p3(const std::vector<int> & resp1,const std::vector<int> & resp2,const std::vector<int> & resp3,int n,std::vector<int> & res,int modulo){
    if (debug_infolevel>2)
      CERR << CLOCK()*1e-6 << " begin ichinremp1p2p3 " << modulo << '\n';
    size_t rs=resp1.size();
    if (&resp1!=&res)
      res.resize(rs);
    int n1=invmod(n,p1); if (n1<0) n1+=p1;
    int n2=invmod(n,p2); if (n2<0) n2+=p2;
    int n3=invmod(n,p3); if (n3<0) n3+=p3;
    int z1=invmod(p1,p2); if (z1<0) z1+=p2;
    int z2=invmod((longlong(p1)*p2) % p3,p3); if (z2<0) z2+=p3;
    int z3=(longlong(p1)*p2)%modulo;
    int modulo2=modulo/2;
    for (int i=0;i<rs;++i){
      int u1=resp1[i],u2=resp2[i],u3=resp3[i];
      //u1 += (unsigned(u1)>>31)*p1;
      //u2 += (unsigned(u2)>>31)*p2;
      //u3 += (unsigned(u3)>>31)*p3;
      u1=mulmod(n1,u1,p1);
      u2=mulmod(n2,u2,p2);
      //u3=mulmod(n3,u3,p3);
      int v1=u1;
      // 4 v2=(u2−v1)×z1 mod p2 
      int v2=((longlong(u2)+p2-v1)*z1)%p2;
      // 5 t=(n3×u3−v1−v2×p1) mod p3 
      int t=(longlong(u3)*n3-v1-longlong(v2)*p1)%p3;
      //t += (unsigned(t)>>31)*p3; // if (t<0) t+=p3;
      // 6 v3 =t×z2 mod p3 
      int v3=smod(longlong(t)*z2,p3);
      // 7 u=(v1+v2×p1+v3×z3) mod q
      longlong u=(v1+longlong(v2)*p1+longlong(v3)*z3);
      u %= modulo;
      if (u>modulo2) u-=modulo; else if (u<-modulo2) u+=modulo;
      res[i]=u;
    }
    if (debug_infolevel>2)
      CERR << CLOCK()*1e-6 << " end ichinremp1p2p3 " << modulo << '\n';
  }

  void to_multi_fft(const vecteur & A,const gen & modulo,std::vector<int> & Wp1,std::vector<int> & Wp2,std::vector<int> & Wp3,unsigned long n,multi_fft_rep & f,bool reverse,bool makeplus){
    f.modulo=modulo;
    vector<int> a,P;
    to_fft(A,0,Wp1,Wp2,Wp3,a,n,f.p1p2p3,reverse,makeplus);
    gen pip=p3*gen(longlong(p1)*p2);
    gen lim=4*gen(long(n))*gen(modulo)*gen(modulo); 
    // linfnorm(A*B+C*D)<=(degree(A*B)+degree(C*D))*modulo^2<pip/2
    // 4*n because A*B+C*D can not be "reduced" more than half the degree of
    // A*B and C*D
    double start=n>8000?p1-1:std::sqrt(double(p1p2)/n)/2.0;
    for (int p=start;is_greater(lim,pip,context0);p--){
      p=prevprime(p).val;
      if (p==p2 || p==p3)
	p=prevprime(p-1).val;
      P.push_back(p);
      pip=p*pip;
    }
    //CERR << P.size() << endl;
    f.v.resize(P.size());
    for (int i=0;i<P.size();++i){
      to_fft(A,P[i],Wp1,Wp2,Wp3,a,n,f.v[i],reverse,makeplus);
#if 0 // 1 for debug
      vector<int> tmp,tmp2,tmp3,tmp4; vecteur AA(smod(A,f.v[i].modulo));
      from_fft(f.v[i],Wp1,Wp2,Wp3,tmp,tmp2,tmp3,tmp4,true);
      addmod(tmp2,tmp3,2);
#endif
    }
  }

  // p -> f it's FFT representation, p is reversed before FFT is called
  // a is a temporary vector = p mod modulo after call
  // Wp1, Wp2, Wp3 is a vector of powers of the n-th root of unity mod p1,p2,p3
  // if size is not n or Wp[0]==0, Wp is computed
  // do not share Wp1/p2/p3 between different threads
  void to_fft(const vecteur & p,int modulo,std::vector<int> & Wp1,std::vector<int> & Wp2,std::vector<int> & Wp3,std::vector<int> & a,int n,fft_rep & f,bool reverse,bool makeplus){
    if (modulo==0){
      vecteur2vectorint(p,p1,a);
      to_fft(a,p1,Wp1,Wp2,Wp3,n,f,reverse,makeplus,false);
      vecteur2vectorint(p,p2,a);
      to_fft(a,p2,Wp1,Wp2,Wp3,n,f,reverse,makeplus,false);
      vecteur2vectorint(p,p3,a);
      to_fft(a,p3,Wp1,Wp2,Wp3,n,f,reverse,makeplus,false);
      f.modulo=0;
    }
    else {
      vecteur2vectorint(p,modulo,a);
      to_fft(a,modulo,Wp1,Wp2,Wp3,n,f,reverse,makeplus,true);
    }
  }

  bool dop3(int modulo,int n){
    //return true;
    return modulo==p3 || (modulo!=p1 && modulo!=p2 && modulo*double(modulo)>p1p2/(1.999999*n));
  }

  // Faster code would require truncated FFT, see David Harvey
  // A cache-friendly truncated FFT
  void to_fft(const std::vector<int> & a,int modulo,int w,std::vector<int> & Wp,int n,std::vector<int> & f,int reverse,bool makeplus,bool makemod){
#if defined GIAC_PRECOND || defined GIAC_CACHEW
    int nw=n;
#else
    int nw=n/2;
#endif
    double invp=find_invp(modulo);
    int s=giacmin(a.size(),n);
    int logrs=sizeinbase2(n-1);
    if (reverse==1){
#if 0
      if (&f!=&a)
	f=a;
      reverse_assign(f,n,modulo);
#else
      if (&f==&a){
	if (f.size()>n)
	  reverse_assign(f,n,modulo);
	else {
	  vector<int>::iterator it=f.begin(),itend=f.end();
	  for (;it!=itend;++it)
	    *it += (*it>>31)&modulo;
	  std::reverse(f.begin(),f.end());
	  f.resize(n);
	}
      }
      else 
	reverse_assign(a,f,n,modulo);
#endif
    }
    else {
      if (reverse==0){
	if (&f!=&a) 
	  f=a;
	f.resize(n);
      }
      else {
	if (0 && &f!=&a && a.size()<=n){
	  f.clear(); f.reserve(n);
	  f.resize(n-a.size());
	  vector<int>::const_iterator it=a.begin(),itend=a.end();
	  for (;it!=itend;++it)
	    f.push_back(*it + ((*it>>31)&modulo));
	}
	else {
	  if (&f!=&a)
	    f=a;
	  vector<int>::iterator it=f.begin(),itend=f.end();
	  for (;it!=itend;++it)
	    *it += (*it>>31)&modulo;
	  if (f.size()>n){ // reduce mod x^n-1
	    vector<int>::reverse_iterator it=f.rbegin(),itend=it+n,jt=itend,jtend=f.rend();
	    for (;jt!=jtend;++jt){ // change for HP: was jt<jtend
	      int i=*it;
	      i += *jt-modulo;
	      i += (i>>31) & modulo;
	      *it =i;
	      ++it;
	      if (it==itend)
		it=f.rbegin();
	    }
	    f.erase(f.begin(),f.end()-n);
	  }
	  else
	    f.insert(f.begin(),n-f.size(),0);
	}
      }
    }
    if (makemod)
      makemodulop(&f.front(),s,modulo);
    if (makeplus) 
      makepositive(&f.front(),s,modulo);
    if (Wp.size()<nw || Wp[0]==0){
      ++wpcount;
      Wp.clear();
      fft2wp(Wp,n,w,modulo);
    }
#ifdef GIAC_CACHEW
    int ws=Wp.size()/2,shift=0;
    for (;ws>=n;ws/=2)
      shift+=ws;
    //vector<int> dbgv(Wp.begin()+shift,Wp.end()); CERR << dbgv << "\n";
    fft2pnopermafter(&f.front(),n,&Wp.front()+shift,modulo,invp,1);
#else
    fft2pnopermafter(&f.front(),n,&Wp.front(),modulo,invp,Wp.size()/nw);
#endif
  }

  void to_fft(const std::vector<int> & a,int modulo,std::vector<int> & Wp1,std::vector<int> & Wp2,std::vector<int> & Wp3,int n,fft_rep & f,bool reverse,bool makeplus,bool makemod){
#if defined GIAC_PRECOND || defined GIAC_CACHEW
    int nw=n;
#else
    int nw=n/2;
#endif
    int s=giacmin(a.size(),n);
    f.modulo=modulo;
    int logrs=sizeinbase2(n-1);
    if (modulo!=p2 && modulo!=p3){
      if (reverse){
	f.modp1.resize(n);
	reverse_assign(a,f.modp1,n,p1);
      }
      else {
	f.modp1=a;
	f.modp1.resize(n);
      }
      if (makemod)
	makemodulop(&f.modp1.front(),s,p1);
      if (makeplus) makepositive(&f.modp1.front(),s,p1);
      const int r1=1227303670;
      if (Wp1.size()!=nw || Wp1[0]==0){
	int w=powmod(r1,(1u<<(27-logrs)),p1);
	Wp1.clear();
	fft2wp1(Wp1,n,w);
      }
      fft2p1nopermafter(&f.modp1.front(),n,&Wp1.front());
    }
    if (modulo!=p1 && modulo!=p3){
      if (reverse){
	f.modp2.resize(n);
	reverse_assign(a,f.modp2,n,p2);
      }
      else {
	f.modp2=a;
	f.modp2.resize(n);
      }
      if (makemod)
	makemodulop(&f.modp2.front(),s,p2);
      if (makeplus) makepositive(&f.modp2.front(),s,p2);
      const int r2=814458146;
      if (Wp2.size()!=nw || Wp2[0]==0){
	int w=powmod(r2,(1u<<(26-logrs)),p2);
	Wp2.clear();
	fft2wp2(Wp2,n,w);
      }
      fft2p2nopermafter(&f.modp2.front(),n,&Wp2.front());
    }
    if (dop3(modulo,n)){
      if (reverse){
	f.modp3.resize(n);
	reverse_assign(a,f.modp3,n,p3);
      }
      else {
	f.modp3=a;
	f.modp3.resize(n);
      }
      if (makemod)
	makemodulop(&f.modp3.front(),s,p3);
      if (makeplus) makepositive(&f.modp3.front(),s,p3);
      const int r3=2187;
      if (Wp3.size()!=nw || Wp3[0]==0){
	int w=powmod(r3,(1u<<(26-logrs)),p3);
	Wp3.clear();
	fft2wp3(Wp3,n,w);
      }
      fft2p3nopermafter(&f.modp3.front(),n,&Wp3.front());
    }
  }

  void multmodp1(const vector<int> & a,const vector<int> & b,vector<int> & c){
    c.resize(a.size());
    const int * aptr=&a.front(),*aend=aptr+a.size(),*bptr=&b.front();
    int *cptr=&c.front();
    for (;aptr!=aend;++aptr,++bptr,++cptr){
      *cptr=(longlong(*aptr)*(*bptr))% p1;
    }
  }

  void multmodp2(const vector<int> & a,const vector<int> & b,vector<int> & c){
    c.resize(a.size());
    const int * aptr=&a.front(),*aend=aptr+a.size(),*bptr=&b.front();
    int *cptr=&c.front();
    for (;aptr!=aend;++aptr,++bptr,++cptr){
      *cptr=(longlong(*aptr)*(*bptr))% p2;
    }
  }

  void multmodp3(const vector<int> & a,const vector<int> & b,vector<int> & c){
    c.resize(a.size());
    const int * aptr=&a.front(),*aend=aptr+a.size(),*bptr=&b.front();
    int *cptr=&c.front();
    for (;aptr!=aend;++aptr,++bptr,++cptr){
      *cptr=(longlong(*aptr)*(*bptr))% p3;
    }
  }

  bool dotmult(const fft_rep & fa,const fft_rep & fb,fft_rep & f){
    if (fa.modulo!=fb.modulo)
      return false;
    f.modulo=fa.modulo;
    multmodp1(fa.modp1,fb.modp1,f.modp1);
    multmodp2(fa.modp2,fb.modp2,f.modp2);
    multmodp3(fa.modp3,fb.modp3,f.modp3);
    return true;
  }

  // FFT representation f -> res
  // Wp1,p2,p3 should be computed with to_fft
  // do not share Wp1/p2/p3 between different threads
  // division by n=size of f.modp1/p2/p3 is done
  // result should normally be reversed at end
  // tmp1/p2/p3 are temporary vectors
  void from_fft(const fft_rep & f,std::vector<int> & Wp1,std::vector<int> & Wp2,std::vector<int> & Wp3,std::vector<int> & res,std::vector<int> & tmp1,std::vector<int> & tmp2,std::vector<int> & tmp3,bool reverseatend,bool revw){
    int p=f.modulo;
    int n=p==p2?f.modp2.size():(p==p3?f.modp3.size():f.modp1.size());
    if (p!=p2 && p!=p3){
      tmp1=f.modp1;
      if (revw) fft_reverse(Wp1,p1);
      fft2p1nopermbefore(&tmp1.front(),n,&Wp1.front());
      if (revw) fft_reverse(Wp1,p1);
      if (p==p1){
	tmp1.swap(res);
	precond_mulmod(res,invmod(n,p1),p1);
	if (reverseatend)
	  reverse(res.begin(),res.end());
	return;
      }
    }
    if (p!=p1 && p!=p3){
      tmp2=f.modp2;
      if (revw) fft_reverse(Wp2,p2);
      fft2p2nopermbefore(&tmp2.front(),n,&Wp2.front());
      if (revw) fft_reverse(Wp2,p2);
      if (p==p2){
	tmp2.swap(res);
	precond_mulmod(res,invmod(n,p2),p2);
	if (reverseatend)
	  reverse(res.begin(),res.end());
	return;
      }
    }
    if (dop3(f.modulo,n)){
      tmp3=f.modp3;
      if (revw) fft_reverse(Wp3,p3);
      fft2p3nopermbefore(&tmp3.front(),n,&Wp3.front());
      if (revw) fft_reverse(Wp3,p3); 
      if (p==p3){
	tmp3.swap(res);
	precond_mulmod(res,invmod(n,p3),p3);
	if (reverseatend)
	  reverse(res.begin(),res.end());
	return;
      }
      ichinremp1p2p3(tmp1,tmp2,tmp3,n,res,f.modulo);
    }
    else 
      ichinremp1p2(tmp1,tmp2,n,res,f.modulo);
    if (reverseatend)
      reverse(res.begin(),res.end());
  }

  void from_fft(const std::vector<int> & f,int p,std::vector<int> & Wp,std::vector<int> & res,bool reverseatend,bool revw){
    if (&res!=&f) res=f;
    int n=res.size();
#if defined GIAC_PRECOND || defined GIAC_CACHEW
    int nw=n;
#else
    int nw=n/2;
#endif
    double invp=find_invp(p);
    if (revw) fft_reverse(Wp,p);
#ifdef GIAC_CACHEW
    int ws=Wp.size()/2,shift=0;
    for (;ws>=n;ws/=2)
      shift+=ws;
    //vector<int> dbgv(Wp.begin()+shift,Wp.end()); CERR << dbgv << "\n";
    fft2pnopermbefore(&res.front(),n,&Wp.front()+shift,p,invp,1);
#else
    fft2pnopermbefore(&res.front(),n,&Wp.front(),p,invp,Wp.size()/nw);
#endif
    if (revw) fft_reverse(Wp,p);
    int m=invmod(n,p);
    precond_mulmod(res,m,p);
    if (reverseatend)
      reverse(res.begin(),res.end());
  }

  void from_multi_fft(const multi_fft_rep & f,std::vector<int> & Wp1,std::vector<int> & Wp2,std::vector<int> & Wp3,vecteur & res,bool reverseatend){
    fft_reverse(Wp1,p1); fft_reverse(Wp2,p2); fft_reverse(Wp3,p3);
    const gen & modulo=f.modulo;
    gen pip;
    vector<int> tmp,tmp1,tmp2,tmp3;
    tmp=f.p1p2p3.modp1;
    int n=tmp.size();
    fft2p1nopermbefore(&tmp.front(),n,&Wp1.front());
    int ninv=invmod(n,p1);
    precond_mulmod(tmp,ninv,p1);
    tmp1=f.p1p2p3.modp2;
    fft2p2nopermbefore(&tmp1.front(),n,&Wp2.front());
    ninv=invmod(n,p2);
    precond_mulmod(tmp,ninv,p2);
    int zsize=64+sizeinbase2(4*n*modulo*modulo);
    // vector_int2vecteur(tmp,res); vecteur cur; vector_int2vecteur(tmp1,cur); cur=ichinrem(res,cur,p1,p2);
    ichinremp1p2(tmp,tmp1,tmp.size(),res,zsize);
    pip=longlong(p1)*p2;
    tmp=f.p1p2p3.modp3;
    fft2p3nopermbefore(&tmp.front(),n,&Wp3.front());
    ninv=invmod(n,p3);
    precond_mulmod(tmp,ninv,p3);
    //vector_int2vecteur(tmp,cur); cur=ichinrem(res,cur,pip,p3);
    ichinrem_inplace(res,tmp,pip,p3);
    pip=p3*pip;
    for (int i=0;i<f.v.size();++i){
      int p=f.v[i].modulo;
      from_fft(f.v[i],Wp1,Wp2,Wp3,tmp,tmp1,tmp2,tmp3,false,false);
      // vector_int2vecteur(tmp,cur); cur=ichinrem(res,cur,pip,p);
      ichinrem_inplace(res,tmp,pip,p);
      pip=p*pip;
    }
    if (reverseatend)
      reverse(res.begin(),res.end());
    fft_reverse(Wp1,p1); fft_reverse(Wp2,p2); fft_reverse(Wp3,p3);
    //smod(res,pip,res);
  }

  // Product of polynomial with integer coeffs using FFT
  bool fftmultp1234(const modpoly & p,const modpoly & q,const gen &P,const gen &Q,modpoly & pq,int modulo, vector<int> & a,vector<int>&b,vector<int> &resp1,vector<int>&resp2,vector<int> & resp3, vector<int> & Wp1,vector<int> & Wp2,vector<int> & Wp3,vector<int> & Wp4,vector<int> &tmp_p,vector<int> &tmp_q,bool compute_pq){
    int ps=int(p.size()),qs=int(q.size()),mindeg=giacmin(ps-1,qs-1);
    int rs=ps+qs-1;
    int logrs=sizeinbase2(rs);
    if (rs==(1<<(logrs-1))){
      modpoly q_(q);
      q_.pop_back();
      if (!fftmultp1234(p,q_,P,Q,pq,modulo,a,b,resp1,resp2,resp3,Wp1,Wp2,Wp3,Wp4,tmp_p,tmp_q,compute_pq))
	return false;
      // pq*x
      pq.push_back(0);
      // add q.back()*p
      q_=p;
      mulmodpoly(q_,q.back(),q_);
      addmodpoly(pq,q_,0,pq);
      return true;
    }
    if (logrs>25) return false;
    int n=(1u<<logrs);
    gen PQ=P*Q;
    if (compute_pq){ pq.clear(); pq.reserve(rs); }
#if 0 // def HAVE_LIBGMP
    if (modulo){
      vector<int> a,b; 
      int shift=int(std::ceil(std::log(modulo*double(modulo)*(mindeg+1))/std::log(2.0)));
      if (shift<=64) shift=64;
      else shift=128;
      if (shift==64){
	if (debug_infolevel>2)
	  CERR << CLOCK()*1e-6 << " begin Kronecker gmp conversion " << rs << '\n';
	vecteur2vectorint(p,modulo,a);
	//makepositive(&a.front(),ps,modulo);
	vecteur2vectorint(q,modulo,b);
	//makepositive(&b.front(),qs,modulo);
	mpz_t tmp1,tmp2;
	mpz_init2(tmp1,shift*rs);
	mpz_init2(tmp2,shift*rs);
	vector<longlong> A(ps),B(qs),C(rs);
	for (int i=0;i<ps;++i)
	  A[i]=a[i];
	for (int i=0;i<qs;++i)
	  B[i]=b[i];
	mpz_import(tmp1,ps,1,sizeof(longlong),0,0,&A.front());
	mpz_import(tmp2,qs,1,sizeof(longlong),0,0,&B.front());
	//CERR << gen(tmp1) << '\n' << gen(tmp2) << '\n';
	if (debug_infolevel>2)
	  CERR << CLOCK()*1e-6 << " begin Kronecker gmp mult " << rs << '\n';
	mpz_mul(tmp1,tmp1,tmp2);
	if (debug_infolevel>2)
	  CERR << CLOCK()*1e-6 << " end Kronecker gmp mult " << rs << '\n';
	size_t countp;
	mpz_export(&C.front(),&countp,1,sizeof(longlong),0,0,tmp1);
	for (int i=0;i<rs;++i){
	  int tmp(C[i] % modulo);
	  if (tmp>modulo/2) tmp-=modulo;
	  pq.push_back(tmp);
	}
	mpz_clear(tmp1); mpz_clear(tmp2);
	if (debug_infolevel>2)
	  CERR << CLOCK()*1e-6 << " end Kronecker conversion " << rs << '\n';
	return true;
      }
    }
#endif
    // Check for large coefficients (degree not too large)
    // (around 1000 for coeff of size 2^degree(p))
    // Following ntl src/ZZX1.c SSMul
    unsigned long l=gen(ps+qs-1).bindigits()-1; // m=2^l <= deg(p*q)+1 < 2^{l+1}
    // long m2 = 1u << (l + 1); /* m2 = 2m = 2^{l+1} */
    gen pPQ=gen(giacmin(ps,qs))*P*Q+1;
    PQ=evalf_double(P*Q,1,context0);
    unsigned long bound=pPQ.bindigits()+1; // 2^bound=smod bound on coeff of p*q
    unsigned long r=(bound >> l)+1;
#if defined INT128 
    // probably useful for very large degree not supported by p1 and p2
    // otherwise the condition is almost the same as for p1*p2
    // p1*p2=3647915701995307009, that's 0.4*p5
    if (0 && modulo!=p1 && modulo!=p2 && modulo!=p3 && modulo!=p4){
      vector<longlong> W;
      vector<int> a,b;
      vector<longlong> ab;
      if (modulo && modulo*longlong(modulo)<p5/(2*mindeg)){
	vecteur2vectorint(p,modulo,a);
	vecteur2vectorint(q,modulo,b);
	fft2p5(a,b,ab,W,modulo); // smod modulo at end
	vectorlonglong2vecteur(ab,pq);
	return true;
      }
      if (modulo==0 && P.type==_INT_ && Q.type==_INT_ && is_greater(p5,2*pPQ,context0)){
	vecteur2vectorint(p,modulo,a);
	vecteur2vectorint(q,modulo,b);
	fft2p5(a,b,ab,W,0); // smod modulo at end
	vectorlonglong2vecteur(ab,pq);
	return true;
      }
    }
#endif
    if (bound>(1<<(l-1))){
      fftprod2rl(p,q,r,l,pq);
      if (modulo)
	pq=smod(pq,modulo);
      return true;
    }
#if 1 // def FFTp1p2p3p4
    if (PQ.type==_DOUBLE_ && (modulo || !my_isinf(PQ._DOUBLE_val))){
      double PQd=PQ._DOUBLE_val;
      if (modulo){
	double pq2=modulo*double(modulo);
	// if (pq2<PQd) // change made january 2019
	PQd=pq2; // since we convert p to a mod modulo and make p positive
      }
      double test=PQd*(mindeg+1);
      if (test<p2*double(p1)/2 || modulo==p1 || modulo==p2 || modulo==p3 || modulo==p4){
	int reduce=modulo?modulo:p1;
	vecteur2vectorint(p,reduce,a);
	vecteur2vectorint(q,reduce,b);
	if (debug_infolevel>2)
	  CERR << CLOCK()*1e-6 << ( (modulo==p2 || modulo==p3 || modulo==p4)?" begin fft2 p234 ":" begin fft2 p1 ") << rs << '\n';
	if (modulo==p3 || modulo==p4) {
	  if (modulo==p3)
	    fft2mult(reduce,a,b,resp1,p3,Wp3,tmp_p,tmp_q,false,true,false);
	  else
	    fft2mult(reduce,a,b,resp1,p4,Wp4,tmp_p,tmp_q,false,true,false);
	}
	else {
	  if (modulo==p2)
	    fft2mult(reduce,a,b,resp1,p2,Wp2,tmp_p,tmp_q,false,true,false);
	  else
	    fft2mult(reduce,a,b,resp1,p1,Wp1,tmp_p,tmp_q,false,true,false);
	}
	if (debug_infolevel>2)
	  CERR << CLOCK()*1e-6 << ( (modulo==p2 || modulo==p3 || modulo==p4)?" end fft2 p234 ":" end fft2 p1 ") << rs << '\n';
	if (test>=p1/2 && modulo!=p1 && modulo!=p2 && modulo!=p3 && modulo!=p4) {
	  if (debug_infolevel>2)
	    CERR << CLOCK()*1e-6 << " begin fft2 p2 " << rs << '\n';
	  if (!modulo){
	    vecteur2vectorint(p,p2,a);
	    vecteur2vectorint(q,p2,b);
	  }
	  reduce=modulo?modulo:p2;
	  fft2mult(reduce,a,b,resp2,p2,Wp2,tmp_p,tmp_q,false,true,false);
	  if (debug_infolevel>2)
	    CERR << CLOCK()*1e-6 << " end fft2 p2 " << rs << '\n';
	  int p1modinv=-9;//invmod(p1,p2);
	  int modulo2=modulo/2;
	  if (modulo){
	    for (int i=0;i<rs;++i){
	      int A=resp1[i],B=resp2[i];
	      // a mod p1, b mod p2 -> res mod p1*p2
	      longlong res=A+((longlong(p1modinv)*(B-longlong(A)))%p2)*p1;
	      //res += (ulonglong(res)>>63)*p1p2; res -= (ulonglong(p1p2/2-res)>>63)*modulo;
	      if (res>p1p2sur2) res-=p1p2; else if (res<=-p1p2sur2) res+=p1p2;
	      //while (res>p1p2sur2) res-=p1p2; while (res<-p1p2sur2) res+=p1p2;
	      A=res % modulo;
	      A += (unsigned(A)>>31)*modulo; // A now positive
	      A -= (unsigned(modulo2-A)>>31)*modulo; 
	      // if (A>modulo2) A-=modulo;
	      resp1[i]=A;
	    }
	  }
	  else {
	    for (int i=0;i<rs;++i){
	      int A=resp1[i],B=resp2[i];
	      // a mod p1, b mod p2 -> res mod p1*p2
	      longlong res=A+((longlong(p1modinv)*(B-longlong(A)))%p2)*p1;
	      //res += (ulonglong(res)>>63)*p1p2; res -= (ulonglong(p1p2/2-res)>>63)*modulo;
	      if (res>p1p2sur2) res-=p1p2; else if (res<-p1p2sur2) res+=p1p2;
	      pq.push_back(res);
	    }
	  }
	  if (debug_infolevel>2)
	    CERR << CLOCK()*1e-6 << " end fft2 chinrem " << rs << '\n';
	  if (!modulo){
	    reverse(pq.begin(),pq.end());
	    return true;
	  }
	}
	reverse(resp1.begin(),resp1.end());
	if (!modulo || compute_pq)
	  vector_int2vecteur(resp1,pq);
	return true;
      }
      if (0 && // uncomment to test code below after debugging, fails sometimes
	  modulo && logrs<=25 && test<p1*double(p2)*p4/2){
	vecteur2vectorint(p,modulo,a);
	vecteur2vectorint(q,modulo,b);
	if (debug_infolevel>2)
	  CERR << CLOCK()*1e-6 << " begin fftp1 " << rs << '\n';
	fft2mult(modulo,a,b,resp1,p1,Wp1,tmp_p,tmp_q,false,false,false);
	if (debug_infolevel>2)
	  CERR << CLOCK()*1e-6 << " begin fftp2 " << rs << '\n';
	fft2mult(modulo,a,b,resp2,p2,Wp2,tmp_p,tmp_q,false,false,false);
	if (debug_infolevel>2)
	  CERR << CLOCK()*1e-6 << " begin fftp4 " << rs << '\n';
	fft2mult(modulo,a,b,resp3,p4,Wp4,tmp_p,tmp_q,false,false,false);
	if (debug_infolevel>2)
	  CERR << CLOCK()*1e-6 << " begin ichinrem " << modulo << '\n';
	int n1=invmod(n,p1); if (n1<0) n1+=p1;
	int n2=invmod(n,p2); if (n2<0) n2+=p2;
	int n3=invmod(n,p4); if (n3<0) n3+=p4;
	int z1=invmod(p1,p2); if (z1<0) z1+=p2;
	int z2=invmod((longlong(p1)*p2) % p4,p4); if (z2<0) z2+=p4;
	int z3=(longlong(p1)*p2)%modulo;
	int modulo2=modulo/2;
	for (int i=0;i<rs;++i){
	  int u1=resp1[i],u2=resp2[i],u3=resp3[i];
	  //u1 += (unsigned(u1)>>31)*p1;
	  //u2 += (unsigned(u2)>>31)*p2;
	  //u3 += (unsigned(u3)>>31)*p4;
	  u1=mulmodp1(n1,u1);
	  u2=mulmodp2(n2,u2);
	  //u3=mulmod(n3,u3,p4);
	  int v1=u1;
	  // 4 v2=(u2−v1)×z1 mod p2 
	  int v2=((longlong(u2)+p2-v1)*z1)%p2;
	  // 5 t=(n3×u3−v1−v2×p1) mod p4 
	  int t=(longlong(u3)*n3-v1-longlong(v2)*p1)%p4;
	  t += (unsigned(t)>>31)*p4; // if (t<0) t+=p4;
	  // 6 v3 =t×z2 mod p4 
	  int v3=smod(longlong(t)*z2,p4);
	  // 7 u=(v1+v2×p1+v3×z3) mod q
	  int u=(v1+longlong(v2)*p1+longlong(v3)*z3) % modulo;
	  if (u>modulo2) u-=modulo; else if (u<-modulo2) u+=modulo;
	  resp1[i]=u;
	}
	if (debug_infolevel>2)
	  CERR << CLOCK()*1e-6 << " end ichinrem " << modulo << '\n';
	reverse(resp1.begin(),resp1.end());
	if (compute_pq)
	  vector_int2vecteur(resp1,pq);
	return true;
      }
      if (modulo && test<p1*double(p2)*p3/2){
#if 0 // activate for checking to_fft and from_fft (+uncomment in previous if)
	fft_rep fa,fb,f; vector<int> tmp1,tmp2,tmp3;
	to_fft(p,modulo,Wp1,Wp2,Wp3,a,n,fa,true,false);
	to_fft(q,modulo,Wp1,Wp2,Wp3,b,n,fb,true,false);
	dotmult(fa,fb,f);
	from_fft(f,Wp1,Wp2,Wp3,resp1,tmp1,tmp2,tmp3,true,true);
	if (compute_pq)
	  vector_int2vecteur(resp1,pq);
	trim_inplace(pq);
	return true;
#endif
	vecteur2vectorint(p,modulo,a);
	vecteur2vectorint(q,modulo,b);
	if (debug_infolevel>2)
	  CERR << CLOCK()*1e-6 << " begin fftp1 " << rs << '\n';
	fft2mult(modulo,a,b,resp1,p1,Wp1,tmp_p,tmp_q,false,false,false);
	if (debug_infolevel>2)
	  CERR << CLOCK()*1e-6 << " begin fftp2 " << rs << '\n';
	fft2mult(modulo,a,b,resp2,p2,Wp2,tmp_p,tmp_q,false,false,false);
	if (debug_infolevel>2)
	  CERR << CLOCK()*1e-6 << " begin fftp3 " << rs << '\n';
	fft2mult(modulo,a,b,resp3,p3,Wp3,tmp_p,tmp_q,false,false,false);
	ichinremp1p2p3(resp1,resp2,resp3,n,resp1,modulo);
	reverse(resp1.begin(),resp1.end());
	if (compute_pq)
	  vector_int2vecteur(resp1,pq);
	return true;
      }
    } // PQ.type==_DOUBLE_
    if (// 0 &&
	modulo==0){
      gen Bound=2*(mindeg+1)*P*Q;
      int nbits=256;
      int nthreads=threads_allowed?threads:1;
#if !defined USE_GMP_REPLACEMENTS && !defined BF2GMP_H
      if (Bound.type==_ZINT)
	nbits=(mpz_sizeinbase(*Bound._ZINTptr,2)/64+1)*64;
      int nbytes=3;
      // if we use 3 bytes coeff wrt the additional variable
      // the min degree of arguments is zmindeg=(mindeg+1)*(1+nbits/24)
      // the max coefficient in the product is 2^24*2^24*zmindeg
      // it must be smaller than p1*p2/2
      if ((mindeg+1)*(1+nbits/24)>=(p1p2sur2>>48))
	nbytes=2;
      //int pzbound = 1 << (8*nbytes);
      int zsize=1+nbits/(8*nbytes);
      // time required by int->poly fft about 2*zsize*fft(rs) where zsize=nbits/24 or nbits/16
      // time required by ichinrem fft: 4+3*(nbits/32-4)*fft(rs)+C/2*(nbits/32)^2
      // where C*(nbits/32) is about fft(rs) for rs=2^19 and nbits around 200
      // -> FFTMUL_INT_MAXBITS around 1000
      if ( //1 ||        
	  (//0 && 
	    nbits>nthreads*FFTMUL_INT_MAXBITS)){
	// add one more variable to convert long integer coefficients into that variable
	longlong RS=longlong(rs)*zsize;
	if (RS!=int(RS))
	  return false;
	logrs=sizeinbase2(RS);
	if (logrs>25)
	  return false;
	int RS2=1<<logrs;
	vector<int> pz(p.size()*zsize),qz(q.size()*zsize);
	// split p and q in pz and qz using mpz_export with basis B=2^24 (3 bytes)
	// requires B=2^16 if min(degree) too large
	// 8 bits unused (zero-ed), zsize int per coefficient
	// mpz_export(&target,&countp,0,nbytes,0,8*(4-nbytes),integer);
	// sign is ignored by mpz_export
	if (debug_infolevel>2)
	  CERR << CLOCK()*1e-6 << " begin fft2 bigint conversion " << zsize << '\n';
	zsplit(p,zsize,nbytes*8,pz);
	zsplit(q,zsize,nbytes*8,qz);
	if (debug_infolevel>2)
	  CERR << CLOCK()*1e-6 << " begin fft2 int " << rs << '\n';
	// fftmult call below should be threaded...
	// CERR << pz << '\n' << qz << '\n';
	// pz and qz must be positive!
	fft2mult(p1,pz,qz,resp1,p1,Wp1,tmp_p,tmp_q,false,true,true);
	fft2mult(p2,pz,qz,resp2,p2,Wp2,tmp_p,tmp_q,false,true,true);
	if (debug_infolevel>2)
	  CERR << CLOCK()*1e-6 << " end fft2 int, begin ichinrem " << rs << '\n';
	reverse(resp1.begin(),resp1.end());
	reverse(resp2.begin(),resp2.end());
	// resp1 and resp2 have size (p.size()+q.size())*rs-1
	// but coefficients above RS are 0
	vector<longlong> pqz(RS);
	int p1modinv=-9;//invmod(p1,p2);
	for (int i=0;i<RS;++i){
	  int A=resp1[i],B=resp2[i];
	  // A mod p1, B mod p2 -> res mod p1*p2
	  longlong res=A+((longlong(p1modinv)*(B-A))%p2)*p1;
	  if (res>p1p2sur2) res-=p1p2;
	  else if (res<-p1p2sur2) res+=p1p2;
	  pqz[i]=res;
	}
	//CERR << "pz:" << pz << '\n' <<"qz:" << qz << '\n' << "resp1:"<<resp1 << '\n' << "resp2"<<resp2 << '\n' ;
	//CERR << "pqz" << pqz << '\n';
	pq.resize(rs);
	if (debug_infolevel>2)
	  CERR << CLOCK()*1e-6 << " begin int back conversion " << zsize << '\n';
	zbuild(pqz,zsize,nbytes*8,pq);
	if (debug_infolevel>2)
	  CERR << CLOCK()*1e-6 << " end fft2 " << rs << '\n';
	// fill pq from pqz using mpz_import
	// carry handling 
	// sum(x_k*B^k): iquorem(x_k,b^2) add quo to x_{k+2},
	// then iquorem(rem,b) add quo to x_{k+1}
	// after carry handling coefficients must be of type int and ||<2^24
	// put them into a vector<int>(zsize) then
	// mpz_import(mpz_target,count,0,nbytes,0,8*(4-nbytes),&array);
	// where mpz_target is pq[]
	return true;	
      }
#endif
      if (1){
	// chinese remaindering
	if (debug_infolevel>2)
	  CERR << CLOCK()*1e-6 << " begin fft2 int, p1 " << rs << '\n';
	// first prime used is p1
	fftmultp1234(p,q,P,Q,pq,p1,a,b,resp1,resp2,resp3,Wp1,Wp2,Wp3,Wp4,tmp_p,tmp_q,false);
	if (debug_infolevel>2)
	  CERR << CLOCK()*1e-6 << " end fft2 int p1 " << rs << '\n';
	gen bound=p1;
	if (debug_infolevel>2)
	  CERR << CLOCK()*1e-6 << " begin fft2 int p2 " << rs << '\n';
	fftmultp1234(p,q,P,Q,pq,p2,a,b,resp2,resp1,resp3,Wp1,Wp2,Wp3,Wp4,tmp_p,tmp_q,false);
	if (debug_infolevel>2)
	  CERR << CLOCK()*1e-6 << " end fft2 int p2 " << rs << '\n';
	bound=p2*bound;
#if 1
	ichinremp1p2(resp1,resp2,rs,pq,nbits);
#else
	ichinrem_inplace(pq,curres,p1,p2); // pq=ichinrem(pq,curres,p1,p2);
#endif
	modpoly curres; // not used
	gen bound_=bound;
	// valid primes m must verify m*m<1.8e18/mindeg
	int prime=p3; // prevprime((1<<30)).val;//prime=prevprime(p1-1).val;;
	vector<int> primes;
	for (int nprimes=0;is_greater(Bound,bound,context0);++nprimes){
	  primes.push_back(prime);
	  bound=prime*bound;
	  // using a prime above p3 might overflow
	  // unless an additional reduction modulo p1/p2/p3 is done
	  // after reduction modulo modulo in the recursive call
	  // because e.g. submod might return a negative number
	  if (logrs<=25 && prime==p3 && nprimes==0) 
	    prime=p4;//int(std::sqrt(1.8e18/mindeg));
	  else {
	    if (prime==p4)
	      prime=p2;
	    prime=prevprime(prime-1).val;
	    if (prime==p1 || prime==p2 || prime==p3)
	      prime=prevprime(prime-1).val;
	  }
	}
	bound=bound_;
	int ps=int(primes.size());
#ifdef HAVE_LIBPTHREAD
	if (nthreads>1){
	  vector<pthread_t> tab(nthreads);
	  vector<thread_fftmult_t> multparam(nthreads);
	  vector<bool> busy(nthreads,false);
	  vector< vector<int> > av(nthreads,vector<int>(n)),bv(nthreads,vector<int>(n)),resp1v(nthreads,vector<int>(n)),resp2v(nthreads,vector<int>(n)),resp3v(nthreads,vector<int>(n)),Wp1v(nthreads,vector<int>(n)),Wp2v(nthreads,vector<int>(n)),Wp3v(nthreads,vector<int>(n)),Wp4v(nthreads,vector<int>(n)),tmp_pv(nthreads,vector<int>(n)),tmp_qv(nthreads,vector<int>(n));
	  for (int j=0;j<nthreads;++j){
	    thread_fftmult_t tmp={&p,&q,P,Q,&curres,0,&av[j],&bv[j],&resp1v[j],&resp2v[j],&resp3v[j],&Wp1v[j],&Wp2v[j],&Wp3v[j],&Wp4v[j],&tmp_pv[j],&tmp_qv[j]};
	    multparam[j]=tmp;
	  }
	  int i=0;
	  for (;i<ps;){
	    if (debug_infolevel>2)
	      CERR << CLOCK()*1e-6 << " Prime " << i << " of " << ps << '\n';
	    for (int j=0;j<nthreads;++j,++i){
	      if (i>=ps){
		multparam[j].prime=0;
		busy[j]=false;
		continue;
	      }
	      multparam[j].prime=primes[i];
	      bool res=true;
	      busy[j]=true;
	      if (j<nthreads-1) res=pthread_create(&tab[j],(pthread_attr_t *) NULL,do_thread_fftmult,(void *) &multparam[j]);
	      if (res){
		do_thread_fftmult((void *)&multparam[j]);
		busy[j]=false;
	      }
	    }
	    for (int j=0;j<nthreads;++j){
	      void * ptr=(void *)&nthreads; // non-zero initialisation
	      if (j<nthreads-1 && busy[j])
		pthread_join(tab[j],&ptr);
	    }
	    for (int j=0;j<nthreads;++j){
	      prime=multparam[j].prime;
	      if (prime){
		ichinrem_inplace(pq,resp1v[j],bound,prime); // pq=ichinrem(pq,curres,bound,prime);
		bound=prime*bound;
	      }
	    }
	  }
	  return true;
	} // end nthreads
#endif // PTHREAD
	for (int i=0;i<ps;++i){
	  prime=primes[i];
	  curres.clear();
	  if (debug_infolevel>2)
	    CERR << CLOCK()*1e-6 << " BEGIN FFT2 MOD " << prime << '\n';
	  fftmultp1234(p,q,P,Q,curres,prime,a,b,resp1,resp2,resp3,Wp1,Wp2,Wp3,Wp4,tmp_p,tmp_q,false);
	  if (debug_infolevel>2)
	    CERR << CLOCK()*1e-6 << " END FFT2 MOD " << prime << '\n';
	  ichinrem_inplace(pq,resp1,bound,prime); // pq=ichinrem(pq,curres,bound,prime);
	  bound=prime*bound;
	}
	return true;
      } // end chinese remaindering method
    } // end if (modulo==0)
#endif // FFTp1p2p3p4
#if 1
    fftprod2rl(p,q,r,l,pq);
#else
    unsigned long mr=r<<l; // 2^mr is also a smod bound on coeff op p*q
    // Now work modulo the integer N=2^{m*r}+1
    // let omega=2^r, omega is a 2m-root of unity
    // since (2^r)^m=-1 mod N
    // Generic code should not be used since optimizations apply here:
    // omega^k=2^(rk) for k<m, and =-2^(r*(k-m)) for k>=m
    // mod operation: if a<N^2, a=N*q+r=(2^(m*r)+1)*q+r=2^(m*r)*q+q+r
    // first do euclidean div by 2^(m*r) -> Q=q,R=q+r -> r=R-Q
    environment env;
    env.modulo=pow(plus_two,mr)+1;
    env.pn=env.modulo;
    env.moduloon=true;
    fftconv(p,q,l+1,pow(plus_two,r),pq,&env);
#endif
    return true;
  }

  bool fftmult(const modpoly & p,const modpoly & q,modpoly & pq,int modulo,int maxdeg){
    vector<int> a,b,resp1,resp2,resp3,Wp1,Wp2,Wp3,Wp4,tmp_p,tmp_q;
    if (debug_infolevel>2) CERR << CLOCK()*1e-6 << " intnorm begin" << '\n';
    gen P=intnorm(p,context0), Q=intnorm(q,context0); // coeff assumed to be integers -> no context
    if (debug_infolevel>2) CERR << CLOCK()*1e-6 << " intnorm end" << '\n';
    return fftmultp1234(p,q,P,Q,pq,modulo,a,b,resp1,resp2,resp3,Wp1,Wp2,Wp3,Wp4,tmp_p,tmp_q,true);
  }

  modpoly fftmult(const modpoly & p,const modpoly & q){
    modpoly pq;
    fftmult(p,q,pq,0);
    return pq;
  }

  gen fastnorm(const dense_POLY1 & pp,GIAC_CONTEXT){
    gen tmp(0),r,I;
    for (unsigned i=0;i<pp.size();++i){
      reim(pp[i],r,I,contextptr);
      tmp += abs(r,contextptr) + abs(I,contextptr);
    }
    return tmp;
  }
#if 1
  bool giac_gcd_modular_algo1(polynome &p,polynome &q,polynome &d){
    environment env,envtmp;
    dense_POLY1 pp(modularize(p,0,&env)),qq(modularize(q,0,&env));
    if (is_undef(pp) || is_undef(qq))
      return false;
    // COUT << "modular gcd 1 " << pp << " " << qq << '\n';
    gen gcdfirstcoeff(gcd(pp.front(),qq.front(),context0));
    int gcddeg= giacmin(int(pp.size()),int(qq.size()))-1;
    gen bound(pow(gen(2),gcddeg+1)* abs(gcdfirstcoeff,context0));
    if (is_zero(im(pp,context0)) && is_zero(im(qq,context0)))
      bound=bound * min(norm(pp,context0), norm(qq,context0),context0);
    else 
      bound = bound * min(fastnorm(pp,context0),fastnorm(qq,context0),context0);
    env.moduloon = true;
    // env.modulo=nextprime(max(gcdfirstcoeff+1,gen(30011),context0)); 
    env.modulo=p1+1;
    env.pn=env.modulo;
    if (poly_is_real(p) && poly_is_real(q))
      env.complexe=false;
    else
      env.complexe=true;
    // find most efficient max prime: prime^2<p1p2/(4*maxdeg)
    int maxdeg=giacmax(pp.size(),qq.size());
    int maxp=std::sqrt(p1p2/4./maxdeg);
    gen productmodulo(1);
    dense_POLY1 currentgcd(p.dim),p_simp(p.dim),q_simp(p.dim),rem(p.dim);
    // 30011 leaves 267 primes below the 2^15 bound 
    for (;;){
      env.modulo=prevprimep1p2p3(env.modulo.val,maxp,maxdeg); 
      while (is_zero(pp.front() % env.modulo) || is_zero(qq.front() % env.modulo)){
	env.modulo=prevprimep1p2p3(env.modulo.val,maxp,maxdeg); 
	if (env.complexe){
	  while (smod(env.modulo,4)!=1)
	    env.modulo=prevprimep1p2p3(env.modulo.val,maxp,maxdeg);
	}
      }
      modpoly gcdmod;
      gcdmodpoly(pp,qq,&env,gcdmod);
      if (is_undef(gcdmod))
	return false;
      // COUT << "Modulo:" << modulo << " " << gcdmod << '\n';
      gen adjustcoeff=gcdfirstcoeff*invmod(gcdmod.front(),env.modulo);
      mulmodpoly(gcdmod,adjustcoeff,&env,gcdmod);
      int m=int(gcdmod.size())-1;
      if (!m){
	d=polynome(gen(1),1);
	return true;
      }
      if (m>gcddeg) // this prime is bad, just ignore
	continue;
      // combine step
      if (m<gcddeg){ // previous prime was bad
	gcddeg=m;
	currentgcd=gcdmod;
	productmodulo=env.modulo;
      }
      else {
	// m==gcddeg, start combine
	if (productmodulo==gen(1)){ // no need to combine primes
	  currentgcd=gcdmod;
	  productmodulo=env.modulo;
	}
	else {
	  // COUT << "Old gcd:" << productmodulo << " " << currentgcd << '\n' ;
	  currentgcd=ichinrem(gcdmod,currentgcd,env.modulo,productmodulo);
	  // COUT << "Combined to " << currentgcd << '\n';
	  productmodulo=productmodulo*env.modulo;
	}
      }
      // check candidate gcd
      modpoly dmod(modularize(currentgcd,productmodulo,&envtmp));
      if (is_undef(dmod))
	return false;
      ppz(dmod);
      if ( DenseDivRem(pp,dmod,p_simp,rem,true) && rem.empty() ){
	if (DenseDivRem(qq,dmod,q_simp,rem,true)
	    && (rem.empty())){
	  p=unmodularize(p_simp);
	  q=unmodularize(q_simp);
	  d=unmodularize(dmod);
	  return true;
	}
      }
    }
    return false;
  }

#else // OLDGCD1

  bool giac_gcd_modular_algo1(polynome &p,polynome &q,polynome &d){
    environment env,envtmp;
    dense_POLY1 pp(modularize(p,0,&env)),qq(modularize(q,0,&env));
    if (is_undef(pp) || is_undef(qq))
      return false;
    // COUT << "modular gcd 1 " << pp << " " << qq << '\n';
    gen gcdfirstcoeff(gcd(pp.front(),qq.front(),context0));
    int gcddeg= giacmin(int(pp.size()),int(qq.size()))-1;
    gen bound(pow(gen(2),gcddeg+1)* abs(gcdfirstcoeff,context0));
    if (is_zero(im(pp,context0)) && is_zero(im(qq,context0)))
      bound=bound * min(norm(pp,context0), norm(qq,context0),context0);
    else 
      bound = bound * min(fastnorm(pp,context0),fastnorm(qq,context0),context0);
    env.moduloon = true;
    // env.modulo=nextprime(max(gcdfirstcoeff+1,gen(30011),context0)); 
    env.modulo=30009;
    env.pn=env.modulo;
    if (poly_is_real(p) && poly_is_real(q))
      env.complexe=false;
    else
      env.complexe=true;
    gen productmodulo(1);
    dense_POLY1 currentgcd(p.dim),p_simp(p.dim),q_simp(p.dim),rem(p.dim);
    // 30011 leaves 267 primes below the 2^15 bound 
    for (;;){
      env.modulo=nextprime(env.modulo+2); 
      while (is_zero(pp.front() % env.modulo) || is_zero(qq.front() % env.modulo)){
	env.modulo=nextprime(env.modulo+2); 
	if (env.complexe){
	  while (smod(env.modulo,4)==1)
	    env.modulo=nextprime(env.modulo+2);
	}
      }
      modpoly gcdmod;
      gcdmodpoly(pp,qq,&env,gcdmod);
      if (is_undef(gcdmod))
	return false;
      // COUT << "Modulo:" << modulo << " " << gcdmod << '\n';
      gen adjustcoeff=gcdfirstcoeff*invmod(gcdmod.front(),env.modulo);
      mulmodpoly(gcdmod,adjustcoeff,&env,gcdmod);
      int m=int(gcdmod.size())-1;
      if (!m){
	d=polynome(gen(1),1);
	return true;
      }
      if (m>gcddeg) // this prime is bad, just ignore
	continue;
      // combine step
      if (m<gcddeg){ // previous prime was bad
	gcddeg=m;
	currentgcd=gcdmod;
	productmodulo=env.modulo;
      }
      else {
	// m==gcddeg, start combine
	if (productmodulo==gen(1)){ // no need to combine primes
	  currentgcd=gcdmod;
	  productmodulo=env.modulo;
	}
	else {
	  // COUT << "Old gcd:" << productmodulo << " " << currentgcd << '\n' ;
	  currentgcd=ichinrem(gcdmod,currentgcd,env.modulo,productmodulo);
	  // COUT << "Combined to " << currentgcd << '\n';
	  productmodulo=productmodulo*env.modulo;
	}
      }
      // check candidate gcd
      modpoly dmod(modularize(currentgcd,productmodulo,&envtmp));
      if (is_undef(dmod))
	return false;
      ppz(dmod);
      if ( (DenseDivRem(pp,dmod,p_simp,rem,true)) && (rem.empty()) && (DenseDivRem(qq,dmod,q_simp,rem,true)) && (rem.empty()) ){
	p=unmodularize(p_simp);
	q=unmodularize(q_simp);
	d=unmodularize(dmod);
	return true;
      }
    }
    return false;
  }
#endif // OLDGCD1

#ifdef HAVE_LIBNTL
#ifdef HAVE_LIBPTHREAD
  pthread_mutex_t ntl_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

#if 0 // def __i386__
  bool ininttype2ZZ(const inttype & temp,const inttype & step,NTL::ZZ & z,const NTL::ZZ & zzstep){
    if (temp==0){
      long j=0;
      z=j;
      return true;
    }
    inttype q;
    inttype rem(irem(temp,step,q));
#ifndef NO_STDEXCEPT
    if (rem.type!=_INT_) setsizeerr(gettext("modpoly.cc/ininttype2ZZ"));
#endif
    long longtemp=rem.val;
    ininttype2ZZ(q,step,z,zzstep);
    NTL::ZZ zztemp;
    zztemp=longtemp;
    z=z*zzstep+zztemp;
    return true;
  }
#else
  bool ininttype2ZZ(const inttype & temp,const inttype & step,NTL::ZZ & z,const NTL::ZZ & zzstep){
    if (temp.type==_INT_){
      z=temp.val;
      return true;
    }
    if (temp.type!=_ZINT)
      return false;
    if (mpz_cmp_si(*temp._ZINTptr,0)<0){
      bool b=ininttype2ZZ(-temp,step,z,zzstep);
      z=-z;
      return b;
    }
    vector<long> ecriture;
    inttype g(*temp._ZINTptr),r(*temp._ZINTptr);
    if (step.type==_INT_){
      mpz_t & z=*g._ZINTptr;
#ifdef BF2GMP_H
      mpz_t tmp; mpz_init(tmp);
      for (;mpz_cmp_si(z,0)!=0;){
	ecriture.push_back(mpz_tdiv_qr_ui(tmp,*r._ZINTptr,z,step.val));
	mpz_set(z,tmp);
      }
      mpz_clear(tmp);
#else
      for (;mpz_cmp_si(z,0)!=0;){
	ecriture.push_back(mpz_tdiv_qr_ui(z,*r._ZINTptr,z,step.val));
      }
#endif
    }
    else {
      for (;g!=0;){
	inttype q;
	inttype rem(irem(g,step,q));
#ifndef NO_STDEXCEPT
	if (rem.type!=_INT_) setsizeerr(gettext("modpoly.cc/ininttype2ZZ"));
#endif
	long r=rem.val;
	ecriture.push_back(r);
	g=q;
      }
    }
    z=0;
    NTL::ZZ zztemp;
    for (int i=ecriture.size()-1;i>=0;--i){
      z *= zzstep;
      zztemp=ecriture[i];
      z += zztemp;
    }
    return true;
    // CERR << temp << " " << z <<'\n';
  }
#endif

  NTL::ZZ inttype2ZZ(const inttype & i){
    int s=1<<30;
    inttype step(s); // 2^16 
    inttype temp(i),q;
    NTL::ZZ zzstep;
    zzstep=s;
    NTL::ZZ z;
    ininttype2ZZ(temp,step,z,zzstep);
    // COUT << "cl_I2ZZ" << i << " -> " << z << '\n';
    return NTL::ZZ(z);
  }

  void inZZ2inttype(const NTL::ZZ & zztemp,int shift,inttype & temp){
    NTL::ZZ zzq(zztemp);
    vector<long> v;
    while (zzq!=0){
      // v.push_back(NTL::DivRem(zzq,zzq,1<<shift));
      v.push_back(NTL::trunc_long(zzq,shift));
      NTL::RightShift(zzq,zzq,shift);
    }
    reverse(v.begin(),v.end());
    temp=0;
    temp.uncoerce(NTL::NumBits(zzq));
#if 1
    for (size_t i=0;i<v.size();++i){
      mpz_t & z=*temp._ZINTptr;
      mpz_mul_2exp(z,z,shift);
      mpz_add_ui(z,z,v[i]);
    }
#else
    for (size_t i=0;i<v.size();++i){
      longlong llongtemp=v[i];
      temp=temp*step+inttype(llongtemp);
    }
#endif
  }


  inttype ZZ2inttype(const NTL::ZZ & z){
    if (z<0)
      return -ZZ2inttype(-z);
    inttype temp(0);
    NTL::ZZ zztemp(z);
    if (sizeof(int*)==4)
      inZZ2inttype(zztemp,30,temp);
    else
      inZZ2inttype(zztemp,62,temp);
    // COUT << "zz2cl_I " << z << " -> " << temp << '\n';
    return temp;
  }

  NTL::ZZX tab2ZZX(const inttype * tab,int degree){
    NTL::ZZX f;
    f.rep.SetMaxLength(degree+1);
    f.rep.SetLength(degree+1);
    for (int i=0;i<=degree;i++)
      SetCoeff(f,i,inttype2ZZ(tab[i]));
    return NTL::ZZX(f);
  }

  void ZZX2tab(const NTL::ZZX & f,int & degree,inttype * & tab){
    // COUT << f << '\n';
    degree=deg(f);
    tab = new inttype[degree+1] ;
    for (int i=degree;i>=0;i--){
      inttype c=ZZ2inttype(coeff(f,i));
      tab[i]=c;
    }
  }

  NTL::GF2X modpoly2GF2X(const modpoly & p){
    NTL::GF2X f;
    int degree=p.size()-1;
    for (int i=0;i<=degree;i++)
      SetCoeff(f,i,p[degree-i].val);
    if (debug_infolevel>1)
      CERR << f << '\n';
    return f;
  }

  modpoly GF2X2modpoly(const NTL::GF2X & f){
    // COUT << f << '\n';
    int degree=deg(f);
    modpoly tab (degree+1) ;
    for (int i=degree;i>=0;i--){
      tab[i]=int(unsigned(rep(coeff(f,i))));
    }
    reverse(tab.begin(),tab.end());
    return tab;
  }

  // Don't forget to set the modulus with ZZ_p::init(p) before calling this 
  NTL::ZZ_pX modpoly2ZZ_pX(const modpoly & p){
    NTL::ZZ_pX f;
    int degree=p.size()-1;
    for (int i=0;i<=degree;i++){
      NTL::ZZ_p tmp;
      conv(tmp,inttype2ZZ(p[degree-i]));
      SetCoeff(f,i,tmp);
    }
    if (debug_infolevel>10) CERR << f << '\n';
    return f;
  }

  modpoly ZZ_pX2modpoly(const NTL::ZZ_pX & f){
    // COUT << f << '\n';
    int degree=deg(f);
    modpoly tab (degree+1) ;
    for (int i=degree;i>=0;i--){
      tab[i]=ZZ2inttype(rep(coeff(f,i)));
    }
    reverse(tab.begin(),tab.end());
    return tab;
  }

  NTL::ZZX modpoly2ZZX(const modpoly & p){
    NTL::ZZX f;
    int degree=p.size()-1;
    for (int i=0;i<=degree;i++){
      NTL::ZZ tmp=inttype2ZZ(p[degree-i]);
      SetCoeff(f,i,tmp);
    }
    if (debug_infolevel>10) CERR << f << '\n';
    return f;
  }

  modpoly ZZX2modpoly(const NTL::ZZX & f){
    // COUT << f << '\n';
    int degree=deg(f);
    modpoly tab (degree+1) ;
    for (int i=degree;i>=0;i--){
      tab[i]=ZZ2inttype(coeff(f,i));
    }
    reverse(tab.begin(),tab.end());
    return tab;
  }

  bool ntlresultant(const modpoly &p,const modpoly &q,const gen & modulo,gen & res,bool ntl_on_check){
    if ( ntl_on_check && ntl_on(context0)==0)
      return false;
#ifdef HAVE_LIBPTHREAD
    int locked=pthread_mutex_trylock(&ntl_mutex);
#endif // HAVE_LIBPTHREAD
    if (locked)
      return false;
    bool ok=true;
    try {
      if (is_zero(modulo)){
	NTL::ZZX P(modpoly2ZZX(p));
	NTL::ZZX Q(modpoly2ZZX(q));
	if (debug_infolevel)
	  CERR << CLOCK()*1e-6 << " ntlresultant begin\n";
	NTL::ZZ R(resultant(P,Q));
	if (debug_infolevel)
	  CERR << CLOCK()*1e-6 << " ntlresultant end\n";
	res=ZZ2inttype(R);
      }
      else {
	NTL::ZZ_p::init(inttype2ZZ(modulo));
	NTL::ZZ_pX P(modpoly2ZZ_pX(p));
	NTL::ZZ_pX Q(modpoly2ZZ_pX(q));
	if (debug_infolevel)
	  CERR << CLOCK()*1e-6 << " ntlresultant mod begin\n";
	NTL::ZZ_p R(resultant(P,Q));
	if (debug_infolevel)
	  CERR << CLOCK()*1e-6 << " ntlresultant mod end\n";
	res=ZZ2inttype(NTL::rep(R));
      }
    } catch(std::runtime_error & e){
      ok=false;
    }
#ifdef HAVE_LIBPTHREAD
    pthread_mutex_unlock(&ntl_mutex);
#endif
    return ok;
  }

  bool ntlxgcd(const modpoly &a,const modpoly &b,const gen & modulo,modpoly & u,modpoly &v,modpoly & d,bool ntl_on_check){
    if (ntl_on_check && ntl_on(context0)==0)   
      return false;
#ifdef HAVE_LIBPTHREAD
    int locked=pthread_mutex_trylock(&ntl_mutex);
#endif // HAVE_LIBPTHREAD
    if (locked)
      return false;
    bool ok=true;
    try {
      if (is_zero(modulo)){
	NTL::ZZX A(modpoly2ZZX(a));
	NTL::ZZX B(modpoly2ZZX(b));
	NTL::ZZX U,V; NTL::ZZ R;
	if (debug_infolevel)
	  CERR << CLOCK()*1e-6 << " ntlxgcd begin\n";
	XGCD(R,U,V,A,B);
	if (debug_infolevel)
	  CERR << CLOCK()*1e-6 << " ntlxgcd end\n";
	u=ZZX2modpoly(U);
	v=ZZX2modpoly(V);
	d=makevecteur(ZZ2inttype(R));
	if (debug_infolevel)
	  CERR << CLOCK()*1e-6 << " ntlxgcd end convert\n";
	ok=R!=0;
      }
      else {
	NTL::ZZ_p::init(inttype2ZZ(modulo));
	NTL::ZZ_pX A(modpoly2ZZ_pX(a));
	NTL::ZZ_pX B(modpoly2ZZ_pX(b));
	NTL::ZZ_pX U,V,D;
	if (debug_infolevel)
	  CERR << CLOCK()*1e-6 << " ntlxgcd begin\n";
	XGCD(D,U,V,A,B);
	if (debug_infolevel)
	  CERR << CLOCK()*1e-6 << " ntlxgcd end\n";
	u=ZZ_pX2modpoly(U);
	v=ZZ_pX2modpoly(V);
	d=ZZ_pX2modpoly(D);
	if (debug_infolevel)
	  CERR << CLOCK()*1e-6 << " ntlxgcd end convert\n";
      }
    } catch(std::runtime_error & e){
      ok=false;
    }
#ifdef HAVE_LIBPTHREAD
    pthread_mutex_unlock(&ntl_mutex);
#endif
    return ok;
  }

  bool ntlgcd(const modpoly &a,const modpoly &b,const gen & modulo,modpoly & d,bool ntl_on_check){
    if (ntl_on_check && ntl_on(context0)==0)   
      return false;
#ifdef HAVE_LIBPTHREAD
    int locked=pthread_mutex_trylock(&ntl_mutex);
#endif // HAVE_LIBPTHREAD
    bool ok=true;
    if (locked)
      return false;
    try {
      if (is_zero(modulo)){
	NTL::ZZX A(modpoly2ZZX(a));
	NTL::ZZX B(modpoly2ZZX(b));
	if (debug_infolevel)
	  CERR << CLOCK()*1e-6 << " ntlgcd begin\n";
	NTL::ZZX D(GCD(A,B));
	if (debug_infolevel)
	  CERR << CLOCK()*1e-6 << " ntlgcd end\n";
	d=ZZX2modpoly(D);
      }
      else {
	NTL::ZZ_p::init(inttype2ZZ(modulo));
	NTL::ZZ_pX A(modpoly2ZZ_pX(a));
	NTL::ZZ_pX B(modpoly2ZZ_pX(b));
	if (debug_infolevel)
	  CERR << CLOCK()*1e-6 << " ntlgcd mod begin\n";
	NTL::ZZ_pX D(GCD(A,B));
	if (debug_infolevel)
	  CERR << CLOCK()*1e-6 << " ntlgcd end\n";
	d=ZZ_pX2modpoly(D);
      }
    } catch(std::runtime_error & e){
      ok=false;
    }
#ifdef HAVE_LIBPTHREAD
    pthread_mutex_unlock(&ntl_mutex);
#endif
    return ok;
  }

  // modular resultant using NTL
  bool polynome2tab(const polynome & p,int deg,inttype * tab){
    inttype n0(0);
    if (p.dim!=1) return false; // setsizeerr(gettext("modpoly.cc/polynome2tab"));
    if (p.coord.empty())
      return true;
    if ( deg!=p.lexsorted_degree()) return false; // setsizeerr(gettext("modpoly.cc/polynome2tab"));
    int curpow=deg;
    vector< monomial<gen> >::const_iterator it=p.coord.begin();
    vector< monomial<gen> >::const_iterator itend=p.coord.end();
    for (;it!=itend;++it){
      int newpow=it->index.front();
      for (;curpow>newpow;--curpow)
	tab[curpow]=n0;
      tab[curpow]=it->value;
      --curpow;
    }
    for (;curpow>-1;--curpow)
      tab[curpow]=n0;
    return true;
  }

  polynome tab2polynome(const inttype * tab,int deg){
    vector< monomial<gen> > v;
    index_t i;
    i.push_back(deg);
    const inttype * tabend=tab+deg+1;
    gen n0(0);
    for (;tab!=tabend;--i[0]){
      --tabend;
      if (gen(*tabend)!=n0)
	v.push_back(monomial<gen>(gen(*tabend),i));
    }
    return polynome(1,v);
  }

  int ntlgcd(inttype *p, int pdeg,inttype * q,int qdeg, inttype * & res, int & resdeg,int debug=0){
    NTL::ZZX f(tab2ZZX(p,pdeg));
    NTL::ZZX g(tab2ZZX(q,qdeg));
    NTL::ZZX d(GCD(f,g));
    ZZX2tab(d,resdeg,res);
    return resdeg;
  }

  bool gcd_modular_algo1(polynome &p,polynome &q,polynome &d,bool compute_cof){
    if (ntl_on(context0)==0 || !poly_is_real(p) || !poly_is_real(q))
      return giac_gcd_modular_algo1(p,q,d);
    int np=p.lexsorted_degree();
    int nq=q.lexsorted_degree();
    if (np<NTL_MODGCD || nq<NTL_MODGCD)
      return giac_gcd_modular_algo1(p,q,d);
#ifdef HAVE_LIBPTHREAD
    int locked=pthread_mutex_trylock(&ntl_mutex);
#endif // HAVE_LIBPTHREAD
    if (locked)
      return giac_gcd_modular_algo1(p,q,d);
    bool res=true;
    try {
      inttype * tabp = new inttype[np+1]; // dense rep of the polynomial
      if (!polynome2tab(p,np,tabp)){
	delete [] tabp;
	return false;
      }
      inttype * tabq = new inttype[nq+1]; // dense rep of the polynomial
      if (!polynome2tab(q,nq,tabq)){
	delete [] tabp;
	delete [] tabq;
	return false;
      }
      int nd;
      inttype * res;
      ntlgcd(tabp,np,tabq,nq,res,nd);
      d=tab2polynome(res,nd);
      // COUT << "PGCD=" << d << '\n';
      delete [] res;
      delete [] tabp;
      delete [] tabq;
      if (compute_cof){
	p = p/d;
	q = q/d;
      }
    } catch(std::runtime_error & e){
      res=false;
    }
#ifdef HAVE_LIBPTHREAD
    pthread_mutex_unlock(&ntl_mutex);
#endif
    return res;
  }

#else // HAVE_LIBNTL
  bool ntlresultant(const modpoly &p,const modpoly &q,const gen & modulo,gen & res,bool ntl_on_check){
    return false;
  }

  bool ntlgcd(const modpoly &a,const modpoly &b,const gen & modulo,modpoly & d,bool ntl_on_check){
    return false;
  }


  bool ntlxgcd(const modpoly &a,const modpoly &b,const gen & modulo,modpoly & reu,modpoly &v,modpoly & d,bool ntl_on_check){
    return false;
  }

  bool gcd_modular_algo1(polynome &p,polynome &q,polynome &d,bool compute_cof){
    return giac_gcd_modular_algo1(p,q,d);
  }
#endif // HAVE_LIBNTL


#ifndef NO_NAMESPACE_GIAC
} // namespace giac
#endif // ndef NO_NAMESPACE_GIAC
