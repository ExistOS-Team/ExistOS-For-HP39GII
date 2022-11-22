// -*- mode:C++ ; compile-command: "g++ -I. -I.. -I../include -g -c vecteur.cc -fno-strict-aliasing -DGIAC_GENERIC_CONSTANTS -DHAVE_CONFIG_H -DIN_GIAC" -*-
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
#include "iostream"
#if defined HAVE_SSTREAM || defined FXCG
#include <sstream>
#else
#include <strstream>
#endif
#include "gen.h"
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
#include "plot.h"
#include "misc.h"
#include "ti89.h"
#include "csturm.h"
#include "sparse.h"
#include "modfactor.h"
#include "quater.h"
#include "giacintl.h"
#ifdef HAVE_LIBGSL
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_eigen.h>
#include <gsl/gsl_poly.h>
#endif

// Apple has the Accelerate framework for lapack if you did not install Atlas/lapack
// (link with -framewrok Accelerate)
// it is not used by default because the Accelerate version is slower 
// than the current Atlas, at least on OSX.6, and is also slower than giac built-in

#if !defined(APPLE_SMART) && !defined(DONT_USE_LIBLAPLACK) 
#if defined __APPLE__ && !defined(HAVE_LIBLAPACK) && !defined(USE_GMP_REPLACEMENTS)
#define HAVE_LIBLAPACK
#endif
// for pocketcas compat.
#if defined(HAVE_LIBCLAPACK) && !defined(HAVE_LIBLAPACK)
#define HAVE_LIBLAPACK
#endif
#endif // APPLE_SMART

// Note that Atlas is slower than built-in for real matrices diago for n < about 1000
// and complex matrices diago for n<300
// the global variable CALL_LAPACK is set to 1111 by default
// can be modified from icas/xcas using lapack_limit() or shell variable GIAC_LAPACK
// #undef HAVE_LIBLAPACK

#ifdef HAVE_LIBLAPACK
#include <f2c.h>
#include <clapack.h>
#undef abs
#undef min
#endif

#if defined __i386__ && !defined PIC && !defined __APPLE__ && !defined _I386_
//#define _I386_
// commented because it will fail with -O2 optimizations under gcc >= 4.3 
// on Ubuntu 11.04 in Mac VirtualBox
#endif

#ifdef USTL
namespace ustl {
  inline bool operator > (const giac::index_t & a,const giac::index_t & b){ 
    if (a.size()!=b.size()) 
      return a.size()>b.size();
    return !giac::all_inf_equal(a,b);
  }
  inline bool operator < (const giac::index_t & a,const giac::index_t & b){ 
    if (a.size()!=b.size()) 
      return a.size()<b.size();
    return !giac::all_sup_equal(a,b);
  }
}
#define GIAC_HAS_STO_38
#undef PSEUDO_MOD
#endif


#ifndef NO_NAMESPACE_GIAC
namespace giac {
#endif // ndef NO_NAMESPACE_GIAC
  unsigned nbits(const gen & g){
    if (g.type==_INT_)
      return sizeinbase2(g.val>0?g.val:-g.val);
    else 
      return mpz_sizeinbase(*g._ZINTptr,2);
  }

#if defined(GIAC_HAS_STO_38) && defined(VISUALC)
  static const int rand_max=2147483647;
#else
  static const int rand_max=RAND_MAX;
#endif

#ifdef _I386_
  // a->a+b*c mod m
  inline void mod(int & a,int b,int c,int m){
    if (c){
      asm volatile("testl %%ebx,%%ebx\n\t" /* sign bit=1 if negative */
		   "jns .Lok%=\n\t"
		   "addl %%edi,%%ebx\n" /* a+=m*/
		   ".Lok%=:\t"
		   "imull %%ecx; \n\t" /* b*c in edx:eax */
		   "addl %%ebx,%%eax; \n\t" /* b*c+a */
		   "adcl $0x0,%%edx; \n\t" /* b*c+a carry */
		   "idivl %%edi; \n\t"
		   :"=d"(a)
		   :"a"(b),"b"(a),"c"(c),"D"(m)
		   );
    }
  }

  // a->a+b*c mod m
  inline int smod(int a,int b,int c,int m){
    if (c){
      if (a<0) a+=m;
      asm volatile("imull %%ecx; \n\t" /* b*c in edx:eax */
		   "addl %%ebx,%%eax; \n\t" /* b*c+a */
		   "adcl $0x0,%%edx; \n\t" /* b*c+a carry */
		   "idivl %%edi; \n\t"
		   :"=d"(a)
		   :"a"(b),"b"(a),"c"(c),"D"(m)
		   );
    }
    return a;
  }
#else
  // a->a+b*c mod m
  inline void mod(int & a,int b,int c,int m){
    a = (a + longlong(b)*c)%m;
  }

  // a->a+b*c mod m
  inline int smod(int a,int b,int c,int m){
    return (a + longlong(b)*c)%m;
  }

#endif

  vecteur makevecteur(const gen & a,const gen & b){
    vecteur v(2);
    v[0]=a;
    v[1]=b;
    return v;
  }

  vecteur makevecteur(int a,int b){
    vecteur v(2);
    v[0]=a;
    v[1]=b;
    return v;
  }

  vecteur makevecteur(const gen & a,const gen & b,const gen & c){
    vecteur v(3);
    v[0]=a;
    v[1]=b;
    v[2]=c;
    return v;
  }

  vecteur makevecteur(const gen & a){
    return vecteur(1,a);
  }

  vecteur makevecteur(const gen & a,const gen & b,const gen & c,const gen & d){
    vecteur v(4);
    v[0]=a;
    v[1]=b;
    v[2]=c;
    v[3]=d;
    return v;
  }

  vecteur makevecteur(const gen & a,const gen & b,const gen & c,const gen & d,const gen & e){
    vecteur v(5);
    v[0]=a;
    v[1]=b;
    v[2]=c;
    v[3]=d;
    v[4]=e;
    return v;
  }

  vecteur makevecteur(const gen & a,const gen & b,const gen & c,const gen & d,const gen & e,const gen & f){
    vecteur v(6);
    v[0]=a;
    v[1]=b;
    v[2]=c;
    v[3]=d;
    v[4]=e;
    v[5]=f;
    return v;
  }

  vecteur makevecteur(const gen & a,const gen & b,const gen & c,const gen & d,const gen & e,const gen & f,const gen & g){
    vecteur v(7);
    v[0]=a;
    v[1]=b;
    v[2]=c;
    v[3]=d;
    v[4]=e;
    v[5]=f;
    v[6]=g;
    return v;
  }

  vecteur makevecteur(const gen & a,const gen & b,const gen & c,const gen & d,const gen & e,const gen & f,const gen & g,const gen & h){
    vecteur v(8);
    v[0]=a;
    v[1]=b;
    v[2]=c;
    v[3]=d;
    v[4]=e;
    v[5]=f;
    v[6]=g;
    v[7]=h;
    return v;
  }

  vecteur makevecteur(const gen & a,const gen & b,const gen & c,const gen & d,const gen & e,const gen & f,const gen & g,const gen & h,const gen & i){
    vecteur v(9);
    v[0]=a;
    v[1]=b;
    v[2]=c;
    v[3]=d;
    v[4]=e;
    v[5]=f;
    v[6]=g;
    v[7]=h;
    v[8]=i;
    return v;
  }

  vecteur makevecteur(const gen & a,const gen & b,const gen & c,const gen & d,const gen & e,const gen & f,const gen & g,const gen & h,const gen & i,const gen &j){
    vecteur v(10);
    v[0]=a;
    v[1]=b;
    v[2]=c;
    v[3]=d;
    v[4]=e;
    v[5]=f;
    v[6]=g;
    v[7]=h;
    v[8]=i;
    v[9]=j;
    return v;
  }

  vecteur makevecteur(const gen & a,const gen & b,const gen & c,const gen & d,const gen & e,const gen & f,const gen & g,const gen & h,const gen & i,const gen &j,const gen & k){
    vecteur v(11);
    v[0]=a;
    v[1]=b;
    v[2]=c;
    v[3]=d;
    v[4]=e;
    v[5]=f;
    v[6]=g;
    v[7]=h;
    v[8]=i;
    v[9]=j;
    v[10]=k;
    return v;
  }

  vecteur makevecteur(const gen & a,const gen & b,const gen & c,const gen & d,const gen & e,const gen & f,const gen & g,const gen & h,const gen & i,const gen &j,const gen & k,const gen & l){
    vecteur v(12);
    v[0]=a;
    v[1]=b;
    v[2]=c;
    v[3]=d;
    v[4]=e;
    v[5]=f;
    v[6]=g;
    v[7]=h;
    v[8]=i;
    v[9]=j;
    v[10]=k;
    v[11]=l;
    return v;
  }

  vecteur makevecteur(const gen & a,const gen & b,const gen & c,const gen & d,const gen & e,const gen & f,const gen & g,const gen & h,const gen & i,const gen &j,const gen & k,const gen & l,const gen & m){
    vecteur v(13);
    v[0]=a;
    v[1]=b;
    v[2]=c;
    v[3]=d;
    v[4]=e;
    v[5]=f;
    v[6]=g;
    v[7]=h;
    v[8]=i;
    v[9]=j;
    v[10]=k;
    v[11]=l;
    v[12]=m;
    return v;
  }

  vecteur makevecteur(const gen & a,const gen & b,const gen & c,const gen & d,const gen & e,const gen & f,const gen & g,const gen & h,const gen & i,const gen &j,const gen & k,const gen & l,const gen & m,const gen& n){
    vecteur v(14);
    v[0]=a;
    v[1]=b;
    v[2]=c;
    v[3]=d;
    v[4]=e;
    v[5]=f;
    v[6]=g;
    v[7]=h;
    v[8]=i;
    v[9]=j;
    v[10]=k;
    v[11]=l;
    v[12]=m;
    v[13]=n;
    return v;
  }

  gen makesequence(const gen & a){
    return gen(vecteur(1,a),_SEQ__VECT);
  }

  gen makesequence(int a){
    return gen(vecteur(1,a),_SEQ__VECT);
  }

  gen makesequence(const gen & a,const gen & b){
    vecteur v(2);
    v[0]=a;
    v[1]=b;
    return gen(v,_SEQ__VECT);
  }

  gen makesequence(int a,int b){
    vecteur v(2);
    v[0]=a;
    v[1]=b;
    return gen(v,_SEQ__VECT);
  }

  gen makesequence(const gen & a,const gen & b,const gen & c){
    vecteur v(3);
    v[0]=a;
    v[1]=b;
    v[2]=c;
    return gen(v,_SEQ__VECT);
  }

  gen makesequence(const gen & a,const gen & b,const gen & c,const gen & d){
    vecteur v(4);
    v[0]=a;
    v[1]=b;
    v[2]=c;
    v[3]=d;
    return gen(v,_SEQ__VECT);
  }

  gen makesequence(const gen & a,const gen & b,const gen & c,const gen & d,const gen & e){
    vecteur v(5);
    v[0]=a;
    v[1]=b;
    v[2]=c;
    v[3]=d;
    v[4]=e;
    return gen(v,_SEQ__VECT);
  }

  gen makesequence(const gen & a,const gen & b,const gen & c,const gen & d,const gen & e,const gen & f){
    vecteur v(6);
    v[0]=a;
    v[1]=b;
    v[2]=c;
    v[3]=d;
    v[4]=e;
    v[5]=f;
    return gen(v,_SEQ__VECT);
  }

  gen makesequence(const gen & a,const gen & b,const gen & c,const gen & d,const gen & e,const gen & f,const gen & g){
    vecteur v(7);
    v[0]=a;
    v[1]=b;
    v[2]=c;
    v[3]=d;
    v[4]=e;
    v[5]=f;
    v[6]=g;
    return gen(v,_SEQ__VECT);
  }

  gen makesequence(const gen & a,const gen & b,const gen & c,const gen & d,const gen & e,const gen & f,const gen & g,const gen & h){
    vecteur v(8);
    v[0]=a;
    v[1]=b;
    v[2]=c;
    v[3]=d;
    v[4]=e;
    v[5]=f;
    v[6]=g;
    v[7]=h;
    return gen(v,_SEQ__VECT);
  }

  gen makesequence(const gen & a,const gen & b,const gen & c,const gen & d,const gen & e,const gen & f,const gen & g,const gen & h,const gen & i){
    vecteur v(9);
    v[0]=a;
    v[1]=b;
    v[2]=c;
    v[3]=d;
    v[4]=e;
    v[5]=f;
    v[6]=g;
    v[7]=h;
    v[8]=i;
    return gen(v,_SEQ__VECT);
  }

  ref_vecteur * makenewvecteur(const gen & a){
    return new_ref_vecteur(vecteur(1,a));
  }

  ref_vecteur * makenewvecteur(const gen & a,const gen & b){
    ref_vecteur *vptr=new_ref_vecteur(0);
    vptr->v.reserve(2);
    vptr->v.push_back(a);
    vptr->v.push_back(b);
    return vptr;
  }

  ref_vecteur * makenewvecteur(const gen & a,const gen & b,const gen & c){
    ref_vecteur * vptr=new_ref_vecteur(0);
    vptr->v.reserve(3);
    vptr->v.push_back(a);
    vptr->v.push_back(b);
    vptr->v.push_back(c);
    return vptr;
  }

  ref_vecteur * makenewvecteur(const gen & a,const gen & b,const gen & c,const gen & d){
    ref_vecteur * vptr=new_ref_vecteur(0);
    vptr->v.reserve(4);
    vptr->v.push_back(a);
    vptr->v.push_back(b);
    vptr->v.push_back(c);
    vptr->v.push_back(d);
    return vptr;
  }

  ref_vecteur * makenewvecteur(const gen & a,const gen & b,const gen & c,const gen & d,const gen & e){
    ref_vecteur * vptr=new_ref_vecteur(0);
    vptr->v.reserve(5);
    vptr->v.push_back(a);
    vptr->v.push_back(b);
    vptr->v.push_back(c);
    vptr->v.push_back(d);
    vptr->v.push_back(e);
    return vptr;
  }

  ref_vecteur * makenewvecteur(const gen & a,const gen & b,const gen & c,const gen & d,const gen & e,const gen & f){
    ref_vecteur * vptr=new_ref_vecteur(0);
    vptr->v.reserve(6);
    vptr->v.push_back(a);
    vptr->v.push_back(b);
    vptr->v.push_back(c);
    vptr->v.push_back(d);
    vptr->v.push_back(e);
    vptr->v.push_back(f);
    return vptr;
  }

  ref_vecteur * makenewvecteur(const gen & a,const gen & b,const gen & c,const gen & d,const gen & e,const gen & f,const gen & g){
    ref_vecteur * vptr=new_ref_vecteur(0);
    vptr->v.reserve(7);
    vptr->v.push_back(a);
    vptr->v.push_back(b);
    vptr->v.push_back(c);
    vptr->v.push_back(d);
    vptr->v.push_back(e);
    vptr->v.push_back(f);
    vptr->v.push_back(g);
    return vptr;
  }

  ref_vecteur * makenewvecteur(const gen & a,const gen & b,const gen & c,const gen & d,const gen & e,const gen & f,const gen & g,const gen & h){
    ref_vecteur * vptr=new_ref_vecteur(0);
    vptr->v.reserve(8);
    vptr->v.push_back(a);
    vptr->v.push_back(b);
    vptr->v.push_back(c);
    vptr->v.push_back(d);
    vptr->v.push_back(e);
    vptr->v.push_back(f);
    vptr->v.push_back(g);
    vptr->v.push_back(h);
    return vptr;
  }

  ref_vecteur * makenewvecteur(const gen & a,const gen & b,const gen & c,const gen & d,const gen & e,const gen & f,const gen & g,const gen & h,const gen & i){
    ref_vecteur * vptr=new_ref_vecteur(0);
    vptr->v.reserve(9);
    vptr->v.push_back(a);
    vptr->v.push_back(b);
    vptr->v.push_back(c);
    vptr->v.push_back(d);
    vptr->v.push_back(e);
    vptr->v.push_back(f);
    vptr->v.push_back(g);
    vptr->v.push_back(h);
    vptr->v.push_back(i);
    return vptr;
  }

  // make a matrix with free rows 
  // (i.e. it is possible to modify the answer in place)
  matrice makefreematrice(const matrice & m){
    matrice res(m);
    int s=int(m.size());
    for (int i=0;i<s;++i){
      if (m[i].type==_VECT){
	res[i]=makefreematrice(*m[i]._VECTptr);
      }
    }
    return res;
  }

  vecteur mergevecteur(const vecteur & a,const vecteur & b){
    if (is_undef(a)) return a;
    if (is_undef(b)) return b;
    int as=int(a.size());
    int bs=int(b.size());
    vecteur v;
    v.reserve(as+bs);
    vecteur::const_iterator it=a.begin(),itend=a.end();
    for (;it!=itend;++it)
      v.push_back(*it);
    it=b.begin();itend=b.end();
    for (;it!=itend;++it)
      v.push_back(*it);
    return v;
  }

  vecteur mergeset(const vecteur & a,const vecteur & b){
    if (is_undef(a)) return a;
    if (is_undef(b)) return b;
    if (a.empty())
      return b;
    vecteur v(a);
    vecteur::const_iterator it=b.begin(),itend=b.end();
    if ( (itend-it)>std::log(double(a.size()))){
      v.reserve(a.size()+(itend-it));
      for (;it!=itend;++it)
	v.push_back(*it);
      islesscomplexthanf_sort(v.begin(),v.end());
      vecteur res(1,v.front());
      res.reserve(v.size());
      it=v.begin()+1,itend=v.end();
      for (;it!=itend;++it){
	if (*it!=res.back())
	  res.push_back(*it);
      }
      return res;
    }
    for (;it!=itend;++it){
      if (!equalposcomp(v,*it))
	v.push_back(*it);
    }
    return v;
  }

  gen makesuite(const gen & a){
    if ( (a.type==_VECT) && (a.subtype==_SEQ__VECT) )
      return a;
    else 
      return gen(vecteur(1,a),_SEQ__VECT);
  }
  
  gen makesuite_inplace(const gen & a,const gen & b){
    if (a.type!=_VECT || a.subtype!=_VECT || (b.type==_VECT && b.subtype==_SEQ__VECT))
      return makesuite(a,b);
    a._VECTptr->push_back(b);
    return a;
  }

  gen makesuite(const gen & a,const gen & b){
    if ( (a.type==_VECT) && (a.subtype==_SEQ__VECT) ){
      if ( (b.type==_VECT) && (b.subtype==_SEQ__VECT) )
	return gen(mergevecteur(*a._VECTptr,*b._VECTptr),_SEQ__VECT);
      else {
	vecteur va=*a._VECTptr;
	va.push_back(b);
	return gen(va,_SEQ__VECT);
      }
    }
    else {
      if ( (b.type==_VECT) && (b.subtype==_SEQ__VECT) ){
	vecteur vb=*b._VECTptr;
	vb.insert(vb.begin(),a);
	return gen(vb,_SEQ__VECT);
      }
      else
	return gen(makevecteur(a,b),_SEQ__VECT);
    }
  }

  // gluing is done line1 of a with line1 of b and so on
  // look at mergevecteur too
  matrice mergematrice(const matrice & a,const matrice & b){
    if (a.empty())
      return b;
    if (b.empty())
      return a;
    const_iterateur ita=a.begin(),itaend=a.end();
    const_iterateur itb=b.begin(),itbend=b.end();
    matrice res;
    res.reserve(itaend-ita);
    if (itaend-ita!=itbend-itb){
      if (debug_infolevel<1)
	return vecteur(1,vecteur(1,gendimerr(gettext("mergematrice"))));
      if (debug_infolevel<1){
	res.dbgprint();
	std_matrix<gen> M;
	matrice2std_matrix_gen(res,M);
	M.dbgprint();
      }
      return vecteur(1,vecteur(1,gendimerr(gettext("mergematrice"))));
    }
    for (;ita!=itaend;++ita,++itb){
      if (ita->type!=_VECT || itb->type!=_VECT)
	return vecteur(1,vecteur(1,gensizeerr(gettext("mergematrice"))));
      res.push_back(mergevecteur(*ita->_VECTptr,*itb->_VECTptr));
    }
    return res;
  }
  
  static complex_double horner(const vector< complex_double > & v, const complex_double & c){
    vector< complex_double > :: const_iterator it=v.begin(),itend=v.end();
    complex_double res(0);
    for (;it!=itend;++it){
      res *= c;
      res += *it;
    }
    // COUT << v << "(" << c << ")" << "=" << res << endl;
    return res;
  }

  // find a root of a polynomial with float coeffs
  gen a_root(const vecteur & v,const complex_double & c0,double eps){
    if (v.empty())
      return gentypeerr(gettext("a_root"));
    vector< complex_double > v_d,dv_d;
    const_iterateur it=v.begin(),itend=v.end();
    int deg=int(itend-it)-1;
    if (deg==0)
      return gensizeerr(gettext("a_root"));
    if (deg==1)
      return -rdiv(v.back(),v.front(),context0);
    if (deg==2){ // use 2nd order equation formula
      return (-v[1]+sqrt(v[1]*v[1]-4*v[0]*v[2],context0))/(2*v[0]); // ok
    }
    v_d.reserve(deg+1);
    dv_d.reserve(deg);
    for (int d=deg;it!=itend;++it,--d){
      gen temp=it->evalf_double(1,context0); // ok
      if (temp.type==_DOUBLE_)
	v_d.push_back(temp._DOUBLE_val);
      else {
	if (temp.type!=_CPLX)
	  return undef;
	v_d.push_back(complex_double(temp._CPLXptr->_DOUBLE_val,(temp._CPLXptr+1)->_DOUBLE_val));
      }
    }
    // Preconditionning, x->x*lambda
    // a_n x^n + .. + a_0 = a_n*lambda^n x^n + a_[n-1]*lambda^(n-1)*x^(n-1) + 
    // = a_n*lambda^n * ( x^n + a_[n-1]/a_n/lambda * x^(n-1) +
    //                    +  a_[n-2]/a_n/lambda^2 * x^(n-1) + ...)
    // take the largest ratio (a_[n-d]/a_n)^(1/d) for lambda
    double ratio=0.0,tmpratio;
    for (int d=1;d<=deg;++d){
      tmpratio=std::pow(complex_abs(v_d[d]/v_d[0]),1.0/d);
      if (tmpratio>ratio)
	ratio=tmpratio;
    }
    double logratio=std::log(ratio);
    if (debug_infolevel)
      CERR << "balance ratio " << ratio << endl;
    bool real0=v_d[0].imag()==0;
    // Recompute coefficients
    for (int d=1;d<=deg;++d){
      bool real=real0 && v_d[d].imag()==0;
      v_d[d]=std::exp(std::log(v_d[d]/v_d[0])-d*logratio);
      if (real)
	v_d[d]=v_d[d].real();
    }
    v_d[0]=1;
    for (int d=0;d<deg;++d)
      dv_d.push_back(v_d[d]*(double)(deg-d)) ;
#ifndef __APPLE__
    if (debug_infolevel>2)
      COUT << "Aroot init " << c0 << " after renormalization: " << v_d << endl << "Diff " << dv_d << endl;
#endif
    // newton method with prefactor
    complex_double c(c0),newc,fc,newfc,fprimec,rapport;    
    double prefact=1.0;
    int maxloop=SOLVER_MAX_ITERATE;
    for (double j=1;j<1024;j=2*j,maxloop=(maxloop*3)/2){ // max 10 loop
      double prefactmult=0.5;
      fc=horner(v_d,c);
      for (int i=maxloop; i;--i){
	fprimec=horner(dv_d,c);
	if (fprimec==complex_double(0,0))
	  break;
	rapport=fc/fprimec;
	if (complex_abs(rapport)>1/eps) // denominator not invertible -> start elsewhere
	  break;
	newc=c-prefact*rapport;
	if (newc==c){
	  if (complex_abs(fc)<eps)
	    return gen(real(newc)*ratio,imag(newc)*ratio);
	  break;
	}
	newfc=horner(v_d,newc);
#ifndef __APPLE__
	if (debug_infolevel>2)
	  CERR << "proot (j=" << j << "i=" << i << "), z'=" << newc << " f(z')=" << newfc << " f(z)=" << fc << " " << prefact << endl;
#endif
	if (complex_abs(rapport)<eps)
	  return gen(real(newc)*ratio,imag(newc)*ratio);
	if (complex_abs(newfc)>complex_abs(fc)){
	  prefact=prefact*prefactmult;
	  // prefactmult = std::max(0.1,prefactmult*prefactmult);
	}
	else { 
	  prefactmult=0.5;
	  c=newc;
	  fc=newfc;
	  if (prefact>0.9)
	    prefact=1;
	  else
	    prefact=prefact*1.1;
	}
      }
      // c=complex_double(rand()*j/RAND_MAX,rand()*j/RAND_MAX);
      c=complex_double(std_rand()*1.0/RAND_MAX,std_rand()*1.0/RAND_MAX);
    }
    CERR << "proot error "+gen(v).print() << endl;
    return c;
  }

  matrice companion(const vecteur & w){
    vecteur v(w);
    if (!is_one(v.front()))
      v=divvecteur(v,v.front());
    int s=int(v.size())-1;
    if (s<=0)
      return vecteur(1,gendimerr());
    matrice m;
    m.reserve(s);
    for (int i=0;i<s;++i){
      vecteur w(s);
      w[s-1]=-v[s-i];
      if (i>0)
	w[i-1]=plus_one;
      m.push_back(w);
    }
    return m;
  }

  bool eigenval2(std_matrix<gen> & H,int n2,gen & l1, gen & l2,GIAC_CONTEXT){
    gen a=H[n2-2][n2-2],b=H[n2-2][n2-1],c=H[n2-1][n2-2],d=H[n2-1][n2-1];
    gen delta=a*a-2*a*d+d*d+4*b*c;
    bool save=complex_mode(contextptr);
    //cout << "delta " << delta << endl;
    complex_mode(true,contextptr);
    delta=sqrt(delta,contextptr);
    complex_mode(save,contextptr);
    l1=(a+d+delta)/2;
    l2=(a+d-delta)/2;
    return is_zero(im(l1,contextptr)) && is_zero(im(l2,contextptr));
  }


  void std_matrix_gen2matrice_destroy(std_matrix<gen> & M,matrice & m){
    int n=int(M.size());
    m.clear();
    m.reserve(n);
    for (int i=0;i<n;++i){
      m.push_back(new ref_vecteur(0));
      m.back()._VECTptr->swap(M[i]);
    }
  }

  static bool proot_real1(const vecteur & v,double eps,int rprec,vecteur & res,GIAC_CONTEXT){
    if (v.size()<2)
      return false;
    matrice m(companion(v)),md;
    int dim=int(m.size());
    matrice I(midn(dim));
    std_matrix<gen> H,P;
    matrix_double H1,P1;
    matrice2std_matrix_gen(m,H);
    matrice2std_matrix_gen(I,P);
    bool complex_schur=false;
    for (unsigned i=0;!complex_schur && i<H.size();++i){
      for (unsigned j=0;j<H[i].size();++j){
	if (H[i][j].type==_CPLX)
	  complex_schur=true;
      }
    }
    if (!francis_schur(H,0,dim,P,2*SOLVER_MAX_ITERATE,dim*eps,false,complex_schur,false,false,contextptr))
      hessenberg_schur(H,P,2*SOLVER_MAX_ITERATE,dim*eps,contextptr);
    if (1){ // FIXME check that H is ok
      eps=dim*dim*eps;
      // read eigenvalues on diagonal of H, using subdiagonal for complex pairs
      for (int i=0;i<dim;++i){
	if (i<dim-1 && is_greater(eps,abs(H[i+1][i],contextptr),contextptr)){
	  // subdiagonal element is 0 -> diagonal element is an eigenvalue
	  res.push_back(H[i][i]);
	  continue;
	}
	if (i==dim-1 && is_greater(eps,abs(H[i][i-1],contextptr),contextptr)){
	  // subdiagonal element is 0 -> diagonal element is an eigenvalue
	  res.push_back(H[i][i]);
	  if (debug_infolevel>2)
	    CERR << "Francis algorithm Success " << res << endl;
	  return true;
	}
	// non-0, next one must be 0
	if (i<dim-2 && !is_greater(eps,abs(H[i+2][i+1],contextptr),contextptr))
	  return false;
	if (i==dim-1)
	  return false;
	gen l1,l2;
	eigenval2(H,i+2,l1,l2,contextptr);
	res.push_back(l1);
	res.push_back(l2);
	++i;
      }
      if (debug_infolevel>2)
	CERR << "Francis algorithm Success " << res << endl;
      return true;
    }
    return false;
  }

  static bool in_proot(const vecteur & w,double & eps,int & rprec,vecteur & res,bool isolaterealroot,GIAC_CONTEXT){
    if (eps<1e-300)
      eps=1e-11;
    return proot_real1(w,eps,rprec,res,contextptr);
  }

  bool is_exact(const vecteur & v){
    const_iterateur it=v.begin(),itend=v.end();
    for (;it!=itend;++it){
      if (!is_exact(*it))
	return false;
    }
    return true;
  }

  bool is_exact(const gen & g){
    switch (g.type){
    case _DOUBLE_:
      return false;
    case _CPLX:
      return is_exact(*g._CPLXptr) && is_exact(*(g._CPLXptr+1));
    case _VECT:
      return is_exact(*g._VECTptr);
    default:
      return true;
    }
  }

  static vecteur proot(const vecteur & v,double & eps,int & rprec,bool ck_exact){
    int vsize=int(v.size());
    int deg=vsize-1;
    if (vsize<2)
      return vecteur(0);
    if (vsize==2)
      return vecteur(1,rprec<=50?evalf(-v[1]/v[0],1,context0):accurate_evalf(-v[1]/v[0],rprec)); // ok
    if (vsize==3 && !is_exactly_zero(v.back())){
      gen b2=accurate_evalf(-v[1]/2,rprec);
      gen delta=accurate_evalf(b2*b2-v[0]*v[2],rprec); // ok
      gen r1,r2;
      if (is_positive(b2,context0)){
	r1=b2+sqrt(delta,context0);
	r2=r1/v[0];
	r1=v[2]/r1;
      }
      else {
	r2=b2-sqrt(delta,context0);
	r1=r2/v[0];
	r2=v[2]/r2;
      }
      return makevecteur(r1,r2);
    }
    // check for 0
    if (v.back()==0){
      vecteur res=proot(vecteur(v.begin(),v.end()-1),eps,rprec,ck_exact);
      res.push_back(0);
      return res;
    }
    if (vsize%2 && v[1]==0){
      // check for composition with a power of X
      int gcddeg=0;
      for (int vi=2;vi<vsize;++vi){
	if (v[vi]!=0)
	  gcddeg=gcd(gcddeg,vi);
	if (gcddeg==1)
	  break;
      }
      if (gcddeg>1){
	vecteur vd;
	for (int i=0;i<vsize;i+=gcddeg){
	  vd.push_back(v[i]);
	}
	vecteur resd=proot(vd,eps,rprec,ck_exact),res;
	vecteur expj;
	for (int j=0;j<gcddeg;++j){
	  gen tmp=exp(j*cst_two_pi*cst_i/gcddeg,context0);
	  if (rprec<=50)
	    expj.push_back(evalf_double(tmp,1,context0));
	  else
	    expj.push_back(accurate_evalf(tmp,rprec));
	}
	for (int i=0;i<int(resd.size());++i){
	  gen r=pow(resd[i],inv(gcddeg,context0),context0);
	  for (int j=0;j<gcddeg;++j){
	    gen tmp=r*expj[j];
	    res.push_back(tmp);
	  }
	}
	return res;
      }
    }
    // now check if the input is exact if there are multiple roots
    if (ck_exact && is_exact(v)){
#if 0
      vecteur res;
      if (int(v.size())<PROOT_FACTOR_MAXDEG){
	gen g=symb_horner(v,vx_var());
	vecteur vv=factors(g,vx_var(),context0);
	for (unsigned i=0;i<vv.size()-1;i+=2){
	  gen vi=vv[i];
	  vi=_e2r(makevecteur(vi,vx_var()),context0);
	  if (vi.type==_VECT && vv[i+1].type==_INT_){
#if 1 // ndef HAVE_LIBPARI
	    gen norme=linfnorm(vi,context0);
	    if (norme.type==_ZINT){
	      rprec=giacmax(rprec,mpz_sizeinbase(*norme._ZINTptr,2));
	      eps=std::pow(2.0,-rprec);
	      if (eps==0) eps=1e-300;
	    }
#endif
	    int mult=vv[i+1].val;
	    vecteur current=proot(*vi._VECTptr,eps,rprec,false);
	    for (unsigned j=0;j<current.size();++j){
	      for (int k=0;k<mult;++k){
		res.push_back(current[j]);
	      }
	    }
	  }
	}
	return res;
      }
      polynome V;
      poly12polynome(v,1,V);
      factorization f=sqff(V);
      if (f.size()==1 && f.front().mult==1)
	return proot(accurate_evalf(v,rprec),eps,rprec,false);
      factorization::const_iterator it=f.begin(),itend=f.end();
      for (;it!=itend;++it){
	polynome pcur=it->fact;
	int n=it->mult;
	vecteur vcur;
	polynome2poly1(pcur,1,vcur);
	vecteur vf=accurate_evalf(vcur,rprec);
	vecteur current=proot(vf,eps,rprec,false);
	for (unsigned j=0;j<current.size();++j){
	  for (int k=0;k<n;++k){
	    res.push_back(current[j]);
	  }
	}
      }
      return res;
#else // without multiplicities
      modpoly p=derivative(v),res;
      res=gcd(v,p,0);
      res=operator_div(v,res,0);
      gen tmp=evalf(res,1,context0);
      if (tmp.type!=_VECT || is_undef(tmp))
	return res;
      return proot(*tmp._VECTptr,eps,rprec);
#endif
    }
    else {
      if (!is_numericv(v,1))
	return vecteur(0);
    }
    context ct;
#ifdef HAVE_LIBPTHREAD
    pthread_mutex_lock(&context_list_mutex);
#endif
    context_list().pop_back();
#ifdef HAVE_LIBPTHREAD
    pthread_mutex_unlock(&context_list_mutex);
#endif
    context * contextptr=&ct;
    epsilon(contextptr)=eps;
    bool add_conjugate=is_zero(im(v,contextptr),contextptr); // ok
    vecteur res,crystalball;
    bool cache=proot_cached(v,eps,crystalball);
    // CERR << v << " " << crystalball << endl;
    if (cache)
      return crystalball;
    cache=true;
    // call pari if degree is large
    if (
	0 && v.size()>=64 && 
	pari_polroots(accurate_evalf(v,rprec),crystalball,giacmax(rprec,53),contextptr) && !is_undef(crystalball)){
      proot_cache(v,eps,crystalball);
      return crystalball;
    }
    int nbits=45;
    rprec = 37;
    vecteur v_accurate(*evalf_double(v,1,contextptr)._VECTptr);
    if (crystalball.empty()){
      in_proot(v,eps,rprec,crystalball,true,contextptr);
      // CERR << crystalball << endl;
      proot_cache(v,eps,crystalball);
    }
    return crystalball;
  }

  vecteur proot(const vecteur & v,double & eps,int & rprec){
    return proot(v,eps,rprec,true);
  }

  vecteur proot(const vecteur & v,double eps){
    int rprec=45;
    return proot(v,eps,rprec);
  }

  vecteur real_proot(const vecteur & v,double eps,GIAC_CONTEXT){
#if 0
    gen r(complexroot(makesequence(v,eps),false,contextptr));
    if (r.type!=_VECT) return vecteur(1,undef);
    const vecteur &w = *r._VECTptr;
    if (is_undef(w)) return w;
    int nbits=int(1-3.2*std::log(eps));
    vecteur res;
    const_iterateur it=w.begin(),itend=w.end();
    for (;it!=itend;++it){
      if (it->type==_VECT && it->_VECTptr->size()==2){
	gen tmp=it->_VECTptr->front();
	if (tmp.type==_VECT){
	  tmp=(tmp._VECTptr->front()+tmp._VECTptr->back())/2;
	  if (eps<1e-14)
	    tmp=accurate_evalf(tmp,nbits);
	  else
	    tmp=evalf_double(tmp,1,contextptr);
	}
	res.push_back(tmp);
      }
    }
    return res;
#else
    vecteur w(proot(v,eps));
    if (is_undef(w)) return w;
    vecteur res;
    const_iterateur it=w.begin(),itend=w.end();
    for (;it!=itend;++it){
      if (is_real(*it,contextptr))
	res.push_back(*it);
    }
    return res;
#endif
  }

  // eps is defined using the norm of v
  vecteur proot(const vecteur & v){
    double eps=1e-12; 
    // this should take care of precision inside v!
    return proot(v,eps);
  }

  gen _proot(const gen & v,GIAC_CONTEXT){
    if ( v.type==_STRNG && v.subtype==-1) return  v;
    if (v.type!=_VECT)
      return _proot(makesequence(v,ggb_var(v)),contextptr);
    if (v._VECTptr->empty())
      return v;
    vecteur w=*v._VECTptr;
    int digits=decimal_digits(contextptr);
    double eps=epsilon(contextptr);
    if (v.subtype==_SEQ__VECT && w.back().type==_INT_){
      digits=giacmax(w.back().val,14);
      eps=std::pow(0.1,double(digits));
      w.pop_back();
    }
    if (w.size()==1)
      w.push_back(ggb_var(w[0]));
    if (w.size()==2 && w[1].type==_IDNT){
      gen tmp=_e2r(gen(w,_SEQ__VECT),contextptr);
      if (is_undef(tmp)) return tmp;
      if (tmp.type==_FRAC)
	tmp=tmp._FRACptr->num;
      if (tmp.type!=_VECT)
	return vecteur(0);
      w=*tmp._VECTptr;
    }
    for (unsigned i=0;i<w.size();++i){
      gen tmp=evalf(w[i],1,contextptr);
      if (tmp.type>_REAL && tmp.type!=_FLOAT_ && tmp.type!=_CPLX)
	return gensizeerr(contextptr);
    }
    int rprec(int(digits*3.3));
    return _sorta(proot(w,eps,rprec),contextptr);
  }
  gen symb_proot(const gen & e) {
    return symbolic(at_proot,e);
  }
  static const char _proot_s []="proot";
  static define_unary_function_eval (__proot,&_proot,_proot_s);
  define_unary_function_ptr5( at_proot ,alias_at_proot,&__proot,0,true);

  vecteur pcoeff(const vecteur & v){
    vecteur w(1,plus_one),new_w,somme;
    gen a,b;
    const_iterateur it=v.begin(),itend=v.end();
    for (;it!=itend;++it){
      if (it->type==_CPLX && it+1!=itend && is_zero(*it-conj(*(it+1),context0))){
	a=re(*it,context0);
	b=im(*it,context0);
	b=a*a+b*b;
	a=-2*a;
	w=w*makevecteur(1,a,b);
	++it;
	continue;
      }
      new_w=w;
      new_w.push_back(zero); // new_w=w*x
      mulmodpoly(w,-(*it),w); // w = -w*root
      addmodpoly(new_w,w,somme);
      w=somme;
    }
    return w;
  }
  gen _pcoeff(const gen & v,GIAC_CONTEXT){
    if ( v.type==_STRNG && v.subtype==-1) return  v;
    if (v.type!=_VECT)
      return symb_pcoeff(v);
    return gen(pcoeff(*v._VECTptr),_POLY1__VECT);
  }
  gen symb_pcoeff(const gen & e) {
    return symbolic(at_pcoeff,e);
  }
  static const char _pcoeff_s []="pcoeff";
  static define_unary_function_eval (__pcoeff,&_pcoeff,_pcoeff_s);
  define_unary_function_ptr5( at_pcoeff ,alias_at_pcoeff,&__pcoeff,0,true);

  gen _peval(const gen & e,GIAC_CONTEXT){
    if ( e.type==_STRNG && e.subtype==-1) return  e;
    if (e.type!=_VECT)
      return gentypeerr(contextptr);
    vecteur & args=*e._VECTptr;
    if ( (args.size()==2) && (args.front().type==_VECT) )
      return horner(*(args.front()._VECTptr),args.back());
    if ( (args.size()!=3) || (args[1].type!=_VECT) || (args[2].type!=_VECT) )
      return gentypeerr(contextptr);
    gen pol(args.front());
    vecteur vars(*args[1]._VECTptr);
    vecteur vals(*args[2]._VECTptr);
    if (vars.size()!=vals.size())
      return gendimerr(contextptr);
    for (int i=0;i<signed(vars.size());++i){
      if (vars[i].type!=_IDNT)
	return gensizeerr(contextptr);
    }
    // convert to internal form: 
    // now put vars at the beginning of the list of variables
    vecteur lv(vars);
    lvar(e,lv);
    vecteur lv1(lv.begin()+vars.size(),lv.end());
    pol=sym2r(pol,lv,contextptr);
    gen polnum,polden;
    fxnd(pol,polnum,polden);
    for (int i=0;i<signed(vals.size());++i){
      if (debug_infolevel)
	CERR << "// Peval conversion of var " << i << " " << CLOCK() << endl;
      vals[i]=e2r(vals[i],lv1,contextptr);
    }
    if (debug_infolevel)
      CERR << "// Peval conversion to internal form completed " << CLOCK() << endl;
    if (polnum.type==_POLY)
      polnum=peval(*polnum._POLYptr,vals,0);
    if (polden.type==_POLY)
      polden=peval(*polden._POLYptr,vals,0);
    pol=rdiv(polnum,polden,contextptr);
    return r2sym(pol,lv1,contextptr);
  }
  gen symb_peval(const gen & arg1,const gen & arg2) {
    return symbolic(at_peval,makesequence(arg1,arg2));
  }
  static const char _peval_s []="peval";
  static define_unary_function_eval (__peval,&_peval,_peval_s);
  define_unary_function_ptr5( at_peval ,alias_at_peval,&__peval,0,true);
  
  int vrows(const vecteur & a){
    return int(a.size());
  }

  // addvecteur is different from addmodpoly if a and b have != sizes
  // because it always start adding at the beginning of a and b
  void addvecteur(const vecteur & a,const vecteur & b,vecteur & res){
    if (&b==&res && &b!=&a){
      addvecteur(b,a,res);
      return ;
    }
    vecteur::const_iterator itb=b.begin(), itbend=b.end();
    if (&a==&res){ // in-place addition
      vecteur::iterator ita=res.begin(), itaend=res.end();
      for (;(ita!=itaend)&&(itb!=itbend);++ita,++itb){
	*ita=*ita+*itb;
      }
      for (;itb!=itbend;++itb)
	res.push_back(*itb);
      return;
    }
    vecteur::const_iterator ita=a.begin(), itaend=a.end();
    res.clear();
    res.reserve(giacmax(int(itbend-itb),int(itaend-ita)));
    for (;(ita!=itaend)&&(itb!=itbend);++ita,++itb){
      res.push_back(*ita+*itb);
    }
    for (;ita!=itaend;++ita)
      res.push_back(*ita);
    for (;itb!=itbend;++itb)
      res.push_back(*itb);
  }

  // subvecteur is different from submodpoly if a and b have != sizes
  // because it always start substr. at the beginning of a and b
  void subvecteur(const vecteur & a,const vecteur & b,vecteur & res){
    if (&b==&res){
      vecteur::const_iterator ita=a.begin(), itaend=a.end();
      vecteur::iterator itb=res.begin(), itbend=res.end();
      for (;(ita!=itaend)&&(itb!=itbend);++ita,++itb){
	*itb=*ita-*itb;
      }
      for (;ita!=itaend;++ita)
	res.push_back(*ita);
      return;
    }
    vecteur::const_iterator itb=b.begin(), itbend=b.end();
    if (&a==&res){ // in-place substract
      vecteur::iterator ita=res.begin(), itaend=res.end();
      for (;(ita!=itaend)&&(itb!=itbend);++ita,++itb){
	operator_minus_eq(*ita,*itb,context0);
      }
      for (;itb!=itbend;++itb)
	res.push_back(-*itb);
      return;
    }
    vecteur::const_iterator ita=a.begin(), itaend=a.end();
    res.clear();
    res.reserve(giacmax(int(itbend-itb),int(itaend-ita)));
    for (;(ita!=itaend)&&(itb!=itbend);++ita,++itb){
      res.push_back(*ita-*itb);
    }
    for (;ita!=itaend;++ita)
      res.push_back(*ita);
    for (;itb!=itbend;++itb)
      res.push_back(-*itb);
  }

  vecteur addvecteur(const vecteur & a,const vecteur & b){
    vecteur res;
    addvecteur(a,b,res);
    return res;
  }

  vecteur subvecteur(const vecteur & a,const vecteur & b){
    vecteur res;
    subvecteur(a,b,res);
    return res;
  }

  vecteur negvecteur(const vecteur & v){
    vecteur w;
    negmodpoly(v,w);
    return w;
  }

  gen dotvecteur(const vecteur & a,const vecteur & b){
    vecteur::const_iterator ita=a.begin(), itaend=a.end();
    vecteur::const_iterator itb=b.begin(), itbend=b.end();
    if (ita==itaend || itb==itbend) return 0;
    gen res,tmp;
    //if (ita->type==_VECT && itb->type==_VECT && dotvecteur_interp(a,b,res)) return res;
    //if (0 && itaend-ita>10 && itbend-itb>10 && ita->type==_POLY && itb->type==_POLY && dotvecteur_interp(a,b,res)) return res;
    for (;(ita!=itaend)&&(itb!=itbend);++ita,++itb){
      type_operator_times((*ita),(*itb),tmp);
      res += tmp;
    }
    return res;
  }

  gen dotvecteur(const gen & g1,const gen & g2){
    gen a(g1);
    gen b(g2);
    if (a.type!=_VECT || b.type!=_VECT)
      return gensizeerr(gettext("dotvector"));
    if (a.subtype==_VECTOR__VECT)
      return dotvecteur(vector2vecteur(*a._VECTptr),b);
    if (b.subtype==_VECTOR__VECT)
      return dotvecteur(a,vector2vecteur(*b._VECTptr));
    return dotvecteur(*a._VECTptr,*b._VECTptr);
  }

  void multvecteur(const gen & a,const vecteur & b,vecteur & res){
    if (b.empty()){
      res.clear();
      return;
    }
    if (b.front().type==_VECT && ckmatrix(b)){
      vecteur temp;
      if (&b==&res){
	iterateur it=res.begin(),itend=res.end();
	for (;it!=itend;++it){
	  if (it->type==_VECT)
	    multvecteur(a,*it->_VECTptr,*it->_VECTptr);
	  else
	    *it = a*(*it);
	}
	return;
      }
      const_iterateur it=b.begin(),itend=b.end();
      res.clear();
      res.reserve(itend-it);
      for (;it!=itend;++it){
	if (it->type==_VECT){
	  multvecteur(a,*it->_VECTptr,temp);
	  res.push_back(temp);
	}
	else
	  res.push_back(a*(*it));
      }
      return;
    }
    if (is_zero(a,context0)){
      if (&b==&res){
	iterateur it=res.begin(),itend=res.end();
	for (;it!=itend;++it)
	  *it=(*it)*zero;
      }
      else {
	const_iterateur it=b.begin(),itend=b.end();
	res.clear();
	res.reserve(itend-it);
	for (;it!=itend;++it)
	  res.push_back((*it)*zero);
      }
    }
    else {
      mulmodpoly(b,a,0,res);
    }
  }

  vecteur multvecteur(const gen & a,const vecteur & b){
    vecteur res;
    multvecteur(a,b,res);
    return res;
  }

  void divvecteur(const vecteur & b,const gen & a,vecteur & res){
    if (b.empty()){
      res.clear();
      return;
    }
    if (&b==&res){
      if (is_one(a))
	return;
      iterateur it=res.begin(),itend=res.end();
      mpz_t tmpz;
      mpz_init(tmpz);
      for (;it!=itend;++it){
	if (it->type==_VECT){
	  vecteur temp;
	  divvecteur(*it->_VECTptr,a,*it->_VECTptr);
	}
	else {
#ifndef USE_GMP_REPLACEMENTS
	  if (it->type==_ZINT && a.type==_ZINT && it->ref_count()==1){
	    my_mpz_gcd(tmpz,*it->_ZINTptr,*a._ZINTptr);
	    if (mpz_cmp_ui(tmpz,1)==0)
	      *it=fraction(*it,a);
	    else {
	      mpz_divexact(*it->_ZINTptr,*it->_ZINTptr,tmpz);
	      ref_mpz_t * den=new ref_mpz_t;
	      mpz_divexact(den->z,*a._ZINTptr,tmpz);
	      *it = fraction(*it,den);
	    }
	  }
	  else
#endif
	    *it=rdiv(*it,a,context0);
	}
      }
      mpz_clear(tmpz);
      return;
    }
    if (b.front().type==_VECT && ckmatrix(b)){
      const_iterateur it=b.begin(),itend=b.end();
      res.clear();
      res.reserve(itend-it);
      for (;it!=itend;++it){
	if (it->type==_VECT){
	  vecteur temp;
	  divvecteur(*it->_VECTptr,a,temp);
	  res.push_back(temp);
	}
	else
	  res.push_back(rdiv(*it,a,context0));
      }
      return;
    }
    divmodpoly(b,a,res);
  }

  vecteur divvecteur(const vecteur & b,const gen & a){
    vecteur res;
    divvecteur(b,a,res);
    return res;
  }

  void multmatvecteur(const matrice & a,const vecteur & b,vecteur & res){
    vecteur::const_iterator ita=a.begin(), itaend=a.end();
    res.clear();
    res.reserve(itaend-ita);
    for (;ita!=itaend;++ita)
      res.push_back(dotvecteur(*(ita->_VECTptr),b));
  }

  vecteur multmatvecteur(const matrice & a,const vecteur & b){
    vecteur res;
    multmatvecteur(a,b,res);
    return res;
  }

  void multvecteurmat(const vecteur & a,const matrice & b,vecteur & res){
    matrice btran;
    mtran(b,btran);
    multmatvecteur(btran,a,res);
  }

  vecteur multvecteurmat(const vecteur & a,const matrice & b){
    vecteur res;
    multvecteurmat(a,b,res);
    return res;
  }

  gen ckmultmatvecteur(const vecteur & a,const vecteur & b,GIAC_CONTEXT){
    if (ckmatrix(a)){
      if (ckmatrix(b)){
	matrice res;
	if (!mmultck(a,b,res))
	  return gendimerr("");
	gen tmp=_simplifier(res,contextptr);
	// code added for e.g. matpow([[0,1],[0,0]],n)
	if (contains(tmp,undef))
	  return res;
	return tmp;
      }
      // matrice * vecteur
      vecteur res;
      if (a.front()._VECTptr->size()!=b.size())
	return gendimerr(gettext("dotvecteur"));
      multmatvecteur(a,b,res);
      return _simplifier(res,contextptr);
    }
    if (ckmatrix(b)){
      vecteur res;
      multvecteurmat(a,b,res);
      return _simplifier(res,contextptr);
    }
    if (xcas_mode(contextptr)==3 || calc_mode(contextptr)==1)
      return apply(a,b,prod);
    return dotvecteur(a,b);
  }

  // *********************
  // ***   Matrices    ***
  // *********************

  bool ckmatrix(const matrice & a,bool allow_embedded_vect){
    vecteur::const_iterator it=a.begin(),itend=a.end();
    if (itend==it)
      return false;
    int s=-1;
    int cur_s;
    for (;it!=itend;++it){
      if (it->type!=_VECT)
	return false;
      cur_s=int(it->_VECTptr->size());
      if (!cur_s)
	return false;
      if (s<0)
	s = cur_s;
      else {
	if (s!=cur_s)
	  return false;
	if (s && (it->_VECTptr->front().type==_VECT && it->_VECTptr->front().subtype!=_POLY1__VECT) && !allow_embedded_vect)
	  return false;
      }
    }
    return true;
  }

  bool ckmatrix(const matrice & a){
    return ckmatrix(a,false);
  }

  bool ckmatrix(const gen & a,bool allow_embedded_vect){
    if (a.type!=_VECT)
      return false;
    return ckmatrix(*a._VECTptr,allow_embedded_vect);
  }

  bool ckmatrix(const gen & a){
    return ckmatrix(a,false);
  }

  bool is_squarematrix(const matrice & a){
    if (!ckmatrix(a))
      return false;
    return a.size()==a.front()._VECTptr->size();
  }

  bool is_squarematrix(const gen & a){
    if (!ckmatrix(a))
      return false;
    return a._VECTptr->size()==a._VECTptr->front()._VECTptr->size();
  }

  bool is_fully_numeric(const vecteur & v, int withfracint){
    const_iterateur it=v.begin(),itend=v.end();
    for (;it!=itend;++it){
      if (!is_fully_numeric(*it, withfracint))
	return false;
    }
    return true;
  }

  bool is_fully_numeric(const gen & a, int withfracint){
    switch (a.type){
    case _DOUBLE_: 
      return true;
    case _REAL:
      return true;
    case _CPLX:
      return is_fully_numeric(*a._CPLXptr, withfracint) && is_fully_numeric(*(a._CPLXptr+1), withfracint);
    case _VECT:
      return is_fully_numeric(*a._VECTptr, withfracint);
    case _IDNT:
      return strcmp(a._IDNTptr->id_name,"pi")==0;
    case _INT_:
    case _ZINT:
      return withfracint & num_mask_withint;
    case _FRAC:
      return (withfracint & num_mask_withfrac) && is_fully_numeric(a._FRACptr->num,withfracint) && is_fully_numeric(a._FRACptr->den,withfracint);
    default:
      return false;
    }
  }

  int mrows(const matrice & a){
    return int(a.size());
  }

  int mcols(const matrice & a){
    return int(a.begin()->_VECTptr->size());
  }

  void mdims(const matrice &m,int & r,int & c){
    r=int(m.size());
    c=0;
    if (r){
      const gen & g=m.front();
      if (g.type==_VECT)
	c=int(g._VECTptr->size());
    }
  }

  void mtran(const matrice & a,matrice & res,int ncolres){
    if (!ckmatrix(a,true)){
      res=vecteur(1,vecteur(ncolres,gensizeerr("Unable to tranpose")));
      return;
    }
    vecteur::const_iterator it=a.begin(),itend=a.end();
    int n=int(itend-it); // nrows of a = ncols of res if ncolres was 0
    res.clear();
    if (!n)
      return;
    if (!ncolres)
      ncolres=n;
    int c=int(it->_VECTptr->size()); // ncols of a = rows of res
    res.reserve(c);
    // find begin of each row
#if 1 // def VISUALC
    vecteur::const_iterator * itr=new vecteur::const_iterator[ncolres];
#else
    vecteur::const_iterator itr[ncolres];
#endif
    vecteur::const_iterator * itrend= itr+ncolres;
    vecteur::const_iterator * itrcur;
    int i;
    for (i=0;(i<n) && (it!=itend);++it,++i)
      itr[i]=it->_VECTptr->begin();
    for (;(i<ncolres) ;++i)
#if 1 // def VISUALC
      * (int *) &itr[i]=0;
#else
      itr[i]=(vecteur::const_iterator) NULL;
#endif
    // make current row of res with currents elements of itr[]
    for (int j=0;j<c;++j){
      gen cr=new_ref_vecteur(0);
      vecteur & cur_row=*cr._VECTptr;
      cur_row.clear();
      cur_row.reserve(ncolres);
      for (itrcur=itr;itrcur!=itrend;++itrcur){
	if
#if 1 // def VISUALC
	  (* (int *)itrcur!=0)
#else
	  (*itrcur!=(vecteur::const_iterator)NULL)
#endif
	    {
	      cur_row.push_back(**itrcur);
	      ++(*itrcur);
	    }
	else
	  cur_row.push_back(0);
      }
      res.push_back(cr);
    }
#if 1 // def VISUALC
    delete [] itr;
#endif
  }


  matrice mtran(const matrice & a){
    matrice res;
    mtran(a,res);
    return res;
  }

  gen _tran(const gen & a,GIAC_CONTEXT){
    if ( a.type==_STRNG && a.subtype==-1) return  a;
    vecteur v;
    if (!ckmatrix(a)){
      if (a.type==_VECT && !a._VECTptr->empty())
	v=vecteur(1,a);
      else
	return symb_tran(a);
    }
    else
      v=*a._VECTptr;
    matrice res;
    mtran(v,res);
    return gen(res,_MATRIX__VECT);
  }
  static const char _tran_s []="tran";
  static define_unary_function_eval (__tran,&_tran,_tran_s);
  define_unary_function_ptr5( at_tran ,alias_at_tran,&__tran,0,true);
  
  void smod_inplace(matrice & res,const gen & pi_p){
#ifndef USE_GMP_REPLACEMENTS
    if (pi_p.type==_ZINT && ckmatrix(res)){
      mpz_t tmpz;
      mpz_init(tmpz);
      for (unsigned i=0;i<res.size();++i){
	iterateur it=res[i]._VECTptr->begin(),itend=res[i]._VECTptr->end();
	for (;it!=itend;++it){
	  if (it->type!=_ZINT) // already smod-ed!
	    continue;
	  if (it->ref_count()!=1)
	    *it=smod(*it,pi_p);
	  if (mpz_cmp_ui(*it->_ZINTptr,0)>0){
	    mpz_sub(tmpz,*it->_ZINTptr,*pi_p._ZINTptr);
	    mpz_neg(tmpz,tmpz);
	    if (mpz_cmp(*it->_ZINTptr,tmpz)>0){
	      mpz_neg(tmpz,tmpz);
	      mpz_swap(tmpz,*it->_ZINTptr);
	    }
	  }
	  else {
	    mpz_add(tmpz,*it->_ZINTptr,*pi_p._ZINTptr);
	    mpz_neg(tmpz,tmpz);
	    if (mpz_cmp(*it->_ZINTptr,tmpz)<0){
	      mpz_neg(tmpz,tmpz);
	      mpz_swap(tmpz,*it->_ZINTptr);
	    }
	  }
	}
      }
      mpz_clear(tmpz);
    }
    else
#endif // USE_GMP_REPLACEMENTS
      res=smod(res,pi_p);
  }

  void uncoerce(gen & g,unsigned prealloc) ;
  void uncoerce(vecteur & v,unsigned prealloc){
    iterateur it=v.begin(),itend=v.end();
    for (;it!=itend;++it)
      uncoerce(*it,prealloc);
  }

  void uncoerce(gen & g,unsigned prealloc) {
    if (g.type==_INT_){
      int tmp =g.val;
#ifdef SMARTPTR64
      * ((ulonglong * ) &g) = ulonglong(new ref_mpz_t(prealloc)) << 16;
#else
      g.__ZINTptr = new ref_mpz_t(prealloc);
#endif
      g.type=_ZINT;
      mpz_set_si(*g._ZINTptr,tmp); 
    }
    else {
      if (g.type==_VECT)
	uncoerce(*g._VECTptr,prealloc);
    }
  }
  
  bool fracvect(const vecteur & v){
    for (unsigned i=0;i<v.size();++i){
      if (!is_cinteger(v[i]) && v[i].type!=_FRAC)
	return false;
    }
    return true;
  }

  double matrix_density(const matrice & a){
    int z=0,c=0;
    const_iterateur it=a.begin(),itend=a.end();
    for (;it!=itend;++it){
      if (it->type!=_VECT){
	if (is_zero(*it)) ++z;
	++c;
	continue;
      }
      const_iterateur jt=it->_VECTptr->begin(),jtend=it->_VECTptr->end();
      for (;jt!=jtend;++jt){
	if (is_zero(*jt)) ++z;
	++c;
      }
    }
    return (c-z)/double(c);
  }

  void mmult(const matrice & a_,const matrice & b,matrice & res){
    matrice btran;
    if (debug_infolevel>2)
      CERR << CLOCK() << " mmult begin" << endl;
    mtran(b,btran);
    mmult_atranb(a_,btran,res);
  }
  void mmult_atranb(const matrice & a_,const matrice & btran,matrice & res){
    // now make the (dotvecteur) product of row i of a with rows of btran to get
    // row i of res
    if (debug_infolevel>2)
      CERR << CLOCK() << " find lcm deno" << endl;
    matrice a(a_);
    vecteur adeno(a.size(),1),bdeno(btran.size(),1);
#if 0
    for (unsigned i=0;i<a.size();++i){
      a[i]=*a[i]._VECTptr;
      if (fracvect(*a[i]._VECTptr))
	lcmdeno(*a[i]._VECTptr,adeno[i],context0);
    }
    for (unsigned i=0;i<btran.size();++i){
      if (fracvect(*btran[i]._VECTptr))
	lcmdeno(*btran[i]._VECTptr,bdeno[i],context0);
    }
#endif
    if (debug_infolevel>2)
      CERR << CLOCK() << " lcm deno done" << endl;
      {
	vecteur::const_iterator ita=a.begin(),itaend=a.end();
	vecteur::const_iterator itbbeg=btran.begin(),itb,itbend=btran.end();
	int resrows=mrows(a);
	int rescols=mrows(btran);
	res.clear();
	res.reserve(resrows);
	double a_density=matrix_density(a),b_density=matrix_density(btran);
	if (a_density*b_density>0.1){
	  /* code for dense matrices */
	   vecteur cur_row;
	   for (;ita!=itaend;++ita){
	     cur_row.clear();
	     cur_row.reserve(rescols);
	     for (itb=itbbeg;itb!=itbend;++itb)
	       cur_row.push_back(dotvecteur(*(ita->_VECTptr),*(itb->_VECTptr)));
	     res.push_back(cur_row);
	   }
	} // end dense matrices
	else {
	  int s=int(btran.size());
	  gen tmp;
	  const_iterateur it,itend;
	  vector<const_iterateur> itbb(s);
	  iterateur itc;
	  for (;ita!=itaend;++ita){
	    vecteur c(rescols,zero);
	    it=ita->_VECTptr->begin();
	    itend=ita->_VECTptr->end();
	    itb=itbbeg;
	    for (int i=0;i<s;++i,++itb)
	      itbb[i]=itb->_VECTptr->begin();
	    for (;it!=itend;++it){
	      const gen & acur=*it;
	      if (is_zero(acur,context0)){
		int p=1;
		++it;
		for (; (it!=itend) && is_zero(*it,context0);++it,++p){
		}
		if (it==itend)
		  break;
		else
		  --it;
		for (int i=0;i<s;++i)
		  itbb[i]+=p;
	      }
	      else {
		itc=c.begin();
		gen tmp;
		for (int i=0;i<s;++itc,++(itbb[i]),++i){
		  type_operator_times(acur, *(itbb[i]),tmp);
		  *itc += tmp;
		}
	      }
	    }
	    res.push_back(c);
	  }
	} // end sparse matrices
      } // end #endif else
    for (unsigned i=0;i<adeno.size();++i){
      vecteur & v=*res[i]._VECTptr;
      for (unsigned j=0;j<bdeno.size();++j){
	v[j] = v[j]/(adeno[i]*bdeno[j]);
      }
    }
    // if (!res1.empty() && res1!=res) CERR << "err" << endl;
  }

  matrice mmult(const matrice & a,const matrice & b){
    matrice res;
    mmult(a,b,res);
    return res;
  }

  bool mmultck(const matrice & a, const matrice & b,matrice & res){
    if (mcols(a)!=mrows(b))
      return false; 
    mmult(a,b,res);
    return true;
  }

  matrice mmultck(const matrice & a, const matrice & b){
    matrice res;
    if (!mmultck(a,b,res))
      return vecteur(1,vecteur(1,gendimerr(gettext("mmultck"))));
    return res;
  }

  gen mtrace(const matrice & a){
    gen res(0);
    vecteur::const_iterator it=a.begin(),itend=a.end();
    for (int i=0;it!=itend;++it,++i)
      res = res + (*it)[i];
    return res;
  }
  
  gen ckmtrace(const gen & a,GIAC_CONTEXT){
    if (!is_squarematrix(a))
      return symbolic(at_trace,a); // gendimerr(contextptr); required to keep trace for geometry
    return mtrace(*a._VECTptr);
  }
  static const char _trace_s []="trace";
  static define_unary_function_eval (__trace,&ckmtrace,_trace_s);
  define_unary_function_ptr5( at_trace ,alias_at_trace,&__trace,0,true);

  gen common_deno(const vecteur & v){
    const_iterateur it=v.begin(),itend=v.end();
    gen lcm_deno(1);
    for (;it!=itend;++it){
      if (it->type==_FRAC)
	lcm_deno=rdiv(lcm_deno,gcd(lcm_deno,it->_FRACptr->den),context0)*(it->_FRACptr->den);
    }
    return lcm_deno;
  }

  static gen common_num(const vecteur & v){
    const_iterateur it=v.begin(),itend=v.end();
    gen gcd_num(0);
    for (;it!=itend;++it){
      if (it->type!=_FRAC)
	gcd_num=gcd(gcd_num,*it);
    }
    return gcd_num;
  }

  static inline gen trim(const gen & a,const gen & b,double eps){
    if (eps && a.type==_DOUBLE_ && b.type==_DOUBLE_ &&
	fabs(a._DOUBLE_val)<eps*fabs(b._DOUBLE_val)) 
      return 0;
    else
      return a;
  }

  gen exact_div(const gen & a,const gen & b){
    if (a.type==_POLY && b.type==_POLY){
      polynome *quoptr=new polynome, rem;
      if (!divrem1(*a._POLYptr,*b._POLYptr,*quoptr,rem,2)) 
	CERR << "bad quo("+a.print()+","+b.print()+")" << endl;
      gen res= *quoptr;
      // if (!is_zero(a-b*res))
      //	CERR << "Bad division" << endl;
      return res;
#if 0
      polynome quo;
      if (!a._POLYptr->Texactquotient(*b._POLYptr,quo))
	CERR << "bad quo("+a.print()+","+b.print()+")" << endl;
      return quo;
#endif
    }
    return rdiv(a,b,context0);
  }

  // v=(c1*v1+c2*v2)/c
  // Set cstart to 0, or to c+1 for lu decomposition
  void linear_combination(const gen & c1,const vecteur & v1,const gen & c2,const vecteur & v2,const gen & c,const gen & cinv,vecteur & v,double eps,int cstart){
    if (!is_one(cinv)){
      if (cinv.type==_FRAC)
	linear_combination(c1*cinv._FRACptr->num,v1,c2*cinv._FRACptr->num,v2,cinv._FRACptr->den,1,v,eps,cstart);
      else
	linear_combination(c1*cinv,v1,c2*cinv,v2,1,1,v,eps,cstart);
      return;
    }
    const_iterateur it1=v1.begin()+cstart,it1end=v1.end(),it2=v2.begin()+cstart;
    iterateur jt1=v.begin()+cstart;
#ifdef DEBUG_SUPPORT
    if (it1end-it1!=v2.end()-it2)
      setdimerr();
#endif
    if (it2==jt1){
      linear_combination(c2,v2,c1,v1,c,1,v,eps,cstart);
      return;
    }
    if (it1==jt1){
      if (is_one(c)){
	for (;jt1!=it1end;++jt1,++it2){
	  *jt1=trim(c1*(*jt1)+c2*(*it2),c1,eps);
	}
      }
      else {
	int t=0;
	if (c1.type==c2.type){
	  t=c1.type;
	  if (t==_EXT && *(c1._EXTptr+1)!=*(c2._EXTptr+1))
	    t=0;
	}
	for (;jt1!=it1end;++jt1,++it2){
#ifndef USE_GMP_REPLACEMENTS
	  if (t==_ZINT && jt1->type==_ZINT && c.type==_ZINT && it2->type==_ZINT && jt1->ref_count()==1){
	    mpz_mul(*jt1->_ZINTptr,*jt1->_ZINTptr,*c1._ZINTptr);
	    mpz_addmul(*jt1->_ZINTptr,*it2->_ZINTptr,*c2._ZINTptr);
	    mpz_divexact(*jt1->_ZINTptr,*jt1->_ZINTptr,*c._ZINTptr);
	    if (mpz_sizeinbase(*jt1->_ZINTptr,2)<31)
	      *jt1=int(mpz_get_si(*jt1->_ZINTptr));
	    continue;
	  }
#endif
	  if (t==_EXT && jt1->type==_EXT && it2->type==_EXT && *(jt1->_EXTptr+1)==*(c1._EXTptr+1) && *(it2->_EXTptr+1)==*(c1._EXTptr+1)){
	    gen tmp=change_subtype(*c1._EXTptr,_POLY1__VECT)*(*jt1->_EXTptr)+change_subtype(*c2._EXTptr,_POLY1__VECT)*(*it2->_EXTptr);
	    tmp=ext_reduce(tmp,*(c1._EXTptr+1));
	    *jt1=exact_div(tmp,c);
	    continue;
	  }
	  *jt1=trim(exact_div(c1*(*jt1)+c2*(*it2),c),c1,eps);
	}
      }
      return;
    }
    v.clear();
    v.reserve(it1end-it1);
    if (is_one(c)){
      for (;it1!=it1end;++it1,++it2)
	v.push_back(trim(c1*(*it1)+c2*(*it2),c1,eps));
    }
    else {
      for (;it1!=it1end;++it1,++it2)
	v.push_back(trim(exact_div(c1*(*it1)+c2*(*it2),c),c1,eps));
    }
  }

  // v1=v1+c2*v2 smod modulo
  void modlinear_combination(vecteur & v1,const gen & c2,const vecteur & v2,const gen & modulo,int cstart,int cend){
    if (!is_exactly_zero(c2)){
      iterateur it1=v1.begin()+cstart,it1end=v1.end();
      if (cend && cend>=cstart && cend<it1end-v1.begin())
	it1end=v1.begin()+cend;
      const_iterateur it2=v2.begin()+cstart;
      for (;it1!=it1end;++it1,++it2)
	*it1=smod((*it1)+c2*(*it2),modulo);
    }
  }


  int dotvecteur(const vecteur & a,const vecteur & b,int modulo){
    vecteur::const_iterator ita=a.begin(), itaend=a.end();
    vecteur::const_iterator itb=b.begin();
    int res=0;
    for (;ita!=itaend;++ita,++itb){
#ifdef _I386_
      mod(res,ita->val,itb->val,modulo);
#else
      res = (res + longlong(ita->val)*itb->val) % modulo; 
#endif
    }
    return res;
  }

  void multmatvecteur(const matrice & a,const vecteur & b,vecteur & res,int modulo){
    vecteur::const_iterator ita=a.begin(), itaend=a.end();
    res.clear();
    res.reserve(itaend-ita);
    for (;ita!=itaend;++ita)
      res.push_back(dotvecteur(*(ita->_VECTptr),b,modulo));
  }


  void matrice2std_matrix_gen(const matrice & m,std_matrix<gen> & M){
    int n=int(m.size());
    M.clear();
    M.reserve(n);
    for (int i=0;i<n;++i)
      M.push_back(*m[i]._VECTptr);
  }

  void std_matrix_gen2matrice(const std_matrix<gen> & M,matrice & m){
    int n=int(M.size());
    m.clear();
    m.reserve(n);
    for (int i=0;i<n;++i)
      m.push_back(M[i]);
  }

  bool vecteur2index(const vecteur & v,index_t & i){
    i.clear();
    const_iterateur it=v.begin(),itend=v.end();
    for (;it!=itend;++it){
      if (it->type!=_INT_)
	return false;
      i.push_back(it->val);
    }
    return true;
  }

  static void print_debug_info(const gen & pivot){
    if ( (pivot.type==_POLY) && !pivot._POLYptr->coord.empty())
      CERR << "poly(" << sum_degree(pivot._POLYptr->coord.front().index) << "," << pivot._POLYptr->coord.size() << ") ";
    else
      CERR << pivot << " ";
  }

  bool is_mod_vecteur(const vecteur & m,vector<int> & v,int & p){
    v.clear();
    v.reserve(m.size());
    const_iterateur it=m.begin(),itend=m.end();
    for (;it!=itend;++it){
      if (it->type==_MOD){
	if (!p)
	  p=(it->_MODptr+1)->val;
	if (*(it->_MODptr+1)!=p)
	  return false;
	v.push_back(it->_MODptr->val);
	continue;
      }
      if (it->is_symb_of_sommet(at_normalmod)){
	const gen & f=it->_SYMBptr->feuille;
	if (f.type!=_VECT || f._VECTptr->size()!=2 || f._VECTptr->front().type!=_INT_ || f._VECTptr->back().type!=_INT_)
	  return false;
	if (!p)
	  p=f._VECTptr->back().val;
	if (f._VECTptr->back().val!=p)
	  return false;
	v.push_back(f._VECTptr->front().val);
	continue;
      }
      if (it->type!=_INT_) return false;
      v.push_back(it->val);
    }
    return true;
  }

  bool is_mod_matrice(const matrice & m,vector< vector<int> > & M,int & p){
    const_iterateur it=m.begin(),itend=m.end();
    M.clear();
    M.reserve(m.size());
    for (;it!=itend;++it){
      M.push_back(vector<int>(0));
      if (it->type!=_VECT || !is_mod_vecteur(*it->_VECTptr,M.back(),p)) 
	return false;
    }
    return true;
  }
  bool is_integer_vecteur(const vecteur & m,bool intonly){
    const_iterateur it=m.begin(),itend=m.end();
    for (;it!=itend;++it){
      if (it->type==_INT_) continue;
      if (intonly) return false;
      if (it->type==_ZINT) continue;
      if (it->type==_CPLX && is_integer(*it->_CPLXptr) && is_exactly_zero(*(it->_CPLXptr+1))) continue;
      return false;
      // if (!is_integer(*it)) return false;
    }
    return true;
  }

  bool is_integer_matrice(const matrice & m,bool intonly){
    const_iterateur it=m.begin(),itend=m.end();
    for (;it!=itend;++it)
      if (it->type!=_VECT || !is_integer_vecteur(*it->_VECTptr,intonly)) return false;
    return true;
  }

  bool is_fraction_vecteur(const vecteur & m){
    const_iterateur it=m.begin(),itend=m.end();
    for (;it!=itend;++it)
      if (it->type!=_FRAC && !is_integer(*it)) return false;
    return true;
  }

  bool is_fraction_matrice(const matrice & m){
    const_iterateur it=m.begin(),itend=m.end();
    for (;it!=itend;++it)
      if (it->type!=_VECT || !is_fraction_vecteur(*it->_VECTptr)) return false;
    return true;
  }

  gen modproduct(const vecteur & v, const gen & modulo){
    const_iterateur it=v.begin(),itend=v.end();
    gen res(1);
    for (;it!=itend;++it){
      res = smod(res * (*it),modulo);
    }
    return res;
  }

  static gen untrunc1(const gen & g){
    if (g.type==_FRAC)
      return fraction(untrunc1(g._FRACptr->num),untrunc1(g._FRACptr->den));
    return g.type==_POLY?g._POLYptr->untrunc1():g;
  }

  vecteur fracmod(const vecteur & v,const gen & modulo){
    const_iterateur it=v.begin(),itend=v.end();
    vecteur res;
    res.reserve(itend-it);
    for (;it!=itend;++it){
      if (it->type==_VECT)
	res.push_back(fracmod(*it->_VECTptr,modulo));
      else
	res.push_back(fracmod(*it,modulo));
    }
    return res;
  }

#ifndef CLOCKS_PER_SEC
#define CLOCKS_PER_SEC 1e6
#endif
  

  // find lvar after doing halftan/tsimplify
  void alg_lvar_halftan_tsimplify(vecteur & res,vecteur & lv,GIAC_CONTEXT){
    lv=alg_lvar(res);
    if (!lv.empty() && lv.front().type==_VECT && lv.front()._VECTptr->size()>1){
      vecteur lw=*halftan(lv.front(),contextptr)._VECTptr;
      if (lvar(lw).size()<lv.front()._VECTptr->size()){
	res=*subst(gen(res),lv.front(),lw,false,contextptr)._VECTptr;
	lv=alg_lvar(res);
      }
      if (!lv.empty() && lv.front().type==_VECT && lv.front()._VECTptr->size()>1){
	lw=*tsimplify(lv.front(),contextptr)._VECTptr;
	if (lvar(lw).size()<lv.front()._VECTptr->size()){
	  res=*subst(gen(res),lv.front(),lw,false,contextptr)._VECTptr;
	  lv=alg_lvar(res);
	}
      }
    }
  }

  int rref_reduce(std_matrix<gen> &M,vecteur & pivots,vector<int> & permutation,gen & det,gen &detnum,int algorithm,int l,int lmax,int c,int cmax,int dont_swap_below,int rref_or_det_or_lu,int fullreduction,double eps,bool step_rref,const vecteur &lv,bool convert_internal,GIAC_CONTEXT){
    int linit=l;
    gen bareiss (1),invbareiss(1),pivot,temp;
    int pivotline,pivotcol;
    matrice res;
    int status=2;
    for (;(l<lmax) && (c<cmax);){
#ifdef TIMEOUT
      control_c();
#endif
      if (ctrl_c || interrupted)
	return 0;
      if ( (!fullreduction) && (l==lmax-1) )
	break;
      if (debug_infolevel>2)
	CERR <<  "// mrref line " << l << ":" << CLOCK() <<endl;
      pivot=M[l][c];
      if (debug_infolevel>2){
	CERR << "// ";
	print_debug_info(pivot);
      }
      pivotline=l;
      pivotcol=c;
      if (l<dont_swap_below){ // scan current line for the best pivot available
	for (int ctemp=c+1;ctemp<cmax;++ctemp){
	  temp=M[l][ctemp];
	  if (debug_infolevel>2)
	    print_debug_info(temp);
	  if (!is_zero(temp,contextptr) && temp.islesscomplexthan(pivot)){
	    pivot=temp;
	    pivotcol=ctemp;
	  }
	}	
      }
      else {      // scan M current column for the best pivot available
	if (rref_or_det_or_lu == 3){ // LU without line permutation
	  if (is_zero(pivot,contextptr)){
	    det = 0;
	    vecteur P;
	    vector_int2vecteur(permutation,P);
	    pivots.push_back(P);
	    return 1;
	  }
	}
	else {
	  for (int ltemp=l+1;ltemp<lmax;++ltemp){
	    temp=M[ltemp][c];
	    if (debug_infolevel>2)
	      print_debug_info(temp);
	    if (!is_zero(temp,contextptr) && (is_zero(pivot) || temp.islesscomplexthan(pivot))){
	      pivot=temp;
	      pivotline=ltemp;
	    }
	  }
	}
      }
      if (debug_infolevel>2)
	CERR << endl;
      //COUT << M << endl << pivot << endl;
      if (!is_zero(pivot,contextptr)){
	// exchange lines if needed
	if (l!=pivotline){
	  swap(M[l],M[pivotline]);
	  swap(permutation[l],permutation[pivotline]);
	  // temp = M[l];
	  // M[l] = M[pivotline];
	  // M[pivotline] = temp;
	  detnum = -detnum;
	}
	// make the reduction
	if (fullreduction){ // should be done after for efficiency
	  for (int ltemp=linit;ltemp<lmax;++ltemp){
	    if (debug_infolevel>=2)
	      CERR << "// " << l << "," << ltemp << " "<< endl;
	    if (ltemp!=l){
	      if (algorithm!=RREF_GAUSS_JORDAN) // M[ltemp] = rdiv( pivot * M[ltemp] - M[ltemp][pivotcol]* M[l], bareiss);
		linear_combination(pivot,M[ltemp],-M[ltemp][pivotcol],M[l],bareiss,invbareiss,M[ltemp],eps,0);
	      else // M[ltemp]=M[ltemp]-rdiv(M[ltemp][pivotcol],pivot)*M[l];
		linear_combination(plus_one,M[ltemp],-rdiv(M[ltemp][pivotcol],pivot,contextptr),M[l],plus_one,plus_one,M[ltemp],eps,0);
	    }
	  }
	}
	else { // subdiagonal reduction
	  for (int ltemp=l+1;ltemp<lmax;++ltemp){
	    if (debug_infolevel>=2)
	      CERR << "// " << l << "," << ltemp << " "<< endl;
	    if (algorithm!=RREF_GAUSS_JORDAN)
	      linear_combination(pivot,M[ltemp],-M[ltemp][pivotcol],M[l],bareiss,invbareiss,M[ltemp],eps,(c+1)*(rref_or_det_or_lu>0));
	    else {
	      gen coeff=M[ltemp][pivotcol]/pivot;
	      linear_combination(plus_one,M[ltemp],-coeff,M[l],plus_one,plus_one,M[ltemp],eps,(c+1)*(rref_or_det_or_lu>0));
	      if (rref_or_det_or_lu==2 || rref_or_det_or_lu == 3){
		M[ltemp][pivotcol]=0;
		M[ltemp][l]=coeff; // pivotcol replaced by l
	      }
	    }
	  }
	  if (rref_or_det_or_lu==1 && algorithm!=RREF_GAUSS_JORDAN) {
	    if (debug_infolevel>2)
	      CERR << "//mrref clear line " << l << endl;
	    // clear pivot line to save memory
	    M[l].clear();
	  }
	} // end else
	// COUT << M << endl;
	// increment column number if swap was allowed
	if (l>=dont_swap_below)
	  ++c;
	// increment line number since reduction has been done
	++l;	  
	// multiply det
	// set new bareiss for next reduction round
	if (algorithm!=RREF_GAUSS_JORDAN){
	  bareiss=pivot;
	  if (bareiss.type==_EXT)
	    invbareiss=inv(bareiss,contextptr);
	  else
	    invbareiss=1;
	}
	// save pivot for annulation test purposes
	if (rref_or_det_or_lu!=1){
	  if (convert_internal)
	    pivots.push_back(r2sym(pivot,lv,contextptr));
	  else
	    pivots.push_back(pivot);
	  if (debug_infolevel>2)
	    CERR << pivots.back() << endl;
	}
      }
      else { // if pivot is 0 increment either the line or the col
	status=3;
	if (rref_or_det_or_lu==1){
	  det=0;
	  return 1;
	}
	if (l>=dont_swap_below)
	  c++;
	else
	  l++;
      }
    } // end for reduction loop
    return status;
  }

  bool remove_identity(matrice & res,GIAC_CONTEXT){
    int s=int(res.size());
    // "shrink" res
    for (int i=0;i<s;++i){
      vecteur v = *res[i]._VECTptr;
      if (is_zero(v[i],context0))
	return false;
      gen tmp=new ref_vecteur(v.begin()+s,v.end());
      divvecteur(*tmp._VECTptr,v[i],*tmp._VECTptr);
      res[i] = normal(tmp,contextptr);
    }
    return true;
  }

  // row reduction from line l and column c to line lmax and column cmax
  // lmax and cmax are not included
  // line are numbered starting from 0
  // if fullreduction is false, reduction occurs under the diagonal only
  // if dont_swap_below !=0, for line numers < dont_swap_below
  // the pivot is searched in the line instead of the column
  // hence no line swap occur
  // convert_internal=false if we do not want conversion to rational fractions
  // algorithm=0 Gauss-Jordan, 1 guess, 2 Bareiss, 3 modular, 4 p-adic, 5 interp
  // rref_or_det_or_lu = 0 for rref, 1 for det, 2 for lu, 
  // 3 for lu without pemutation
  int mrref(const matrice & a, matrice & res, vecteur & pivots, gen & det,int l, int lmax, int c,int cmax,
	    int fullreduction_,int dont_swap_below,bool convert_internal,int algorithm_,int rref_or_det_or_lu,
	    GIAC_CONTEXT){
    if (!ckmatrix(a))
      return 0;
    double eps=epsilon(contextptr);
    unsigned as=unsigned(a.size()),a0s=unsigned(a.front()._VECTptr->size());
    bool step_rref=false;
    int algorithm=algorithm_;
    bool rm_idn_after=absint(fullreduction_)>=256;
    if (rm_idn_after)
      fullreduction_/=256;
    int fullreduction=fullreduction_;
    if (fullreduction<0)
      fullreduction=-fullreduction_;
    if (algorithm==RREF_GUESS && step_infolevel(contextptr) && as<5 && a0s<7){
      algorithm=RREF_GAUSS_JORDAN;
      step_rref=true;
    }
    int modular=(algorithm==RREF_MODULAR || algorithm==RREF_PADIC);
    // NOTE for integer matrices
    // p-adic is in n^3*log(nA)^2 where ||a||<=A
    // multi-modular is in n^3*(n+log(nA))*log(nA)
    // Bareiss is in n^3*M(n*log(nA)) where M is multiplication time
    // => for small A and large n p-adic,
    // but for large A and small n, Bareiss is faster
    if (fullreduction_>=0 && algorithm==RREF_GUESS && rref_or_det_or_lu==0 && as>10 && as==a0s-1 && int(as)==lmax && int(a0s)==cmax)
      modular=2;
    if (algorithm==RREF_GUESS && rref_or_det_or_lu<0){
      modular=1;
      rref_or_det_or_lu=-rref_or_det_or_lu;
    }
    if (rref_or_det_or_lu==2 || rref_or_det_or_lu == 3){ // LU decomposition
      algorithm=RREF_GAUSS_JORDAN;
      dont_swap_below=0;
      convert_internal=false;
      fullreduction=0;
    }
    vector<int> permutation(lmax);
    for (int i=0;i<lmax;++i)
      permutation[i]=i;
    gen tmp=a.front();
    if (lidnt(a).empty() && tmp.type==_VECT && !tmp._VECTptr->empty()){
      tmp=tmp._VECTptr->front();
      if (tmp.type==_MOD){
	gen modulo=*(tmp._MODptr+1);
	vecteur unmoda=*unmod(a)._VECTptr;
	if (!modrref(unmoda,res,pivots,det,l,lmax,c,cmax,
		     fullreduction,dont_swap_below,modulo,true/*ckprime*/,rref_or_det_or_lu)){
	  if (!mrref(unmoda,res,pivots,det,l,lmax,c,cmax,
		     fullreduction,dont_swap_below,convert_internal,algorithm,rref_or_det_or_lu,contextptr))
	    return 0;
	}
	res=*makemod(res,modulo)._VECTptr;
	// keep the permutation without makemod
	if (!pivots.empty()){
	  gen last=pivots.back();
	  pivots.pop_back();
	  pivots=*makemod(pivots,modulo)._VECTptr;
	  pivots.push_back(last);
	}
	det=makemod(det,modulo);
	return 1;
      }
    }
    int linit=l;//,previous_l=l;
    vecteur lv;
    bool num_mat=has_num_coeff(a);
    if (num_mat){
      if (is_fully_numeric(a))
	res=a;
      else {
	res=*evalf_VECT(a,0,1,contextptr)._VECTptr;
	num_mat=is_fully_numeric(res);
      }
      if (algorithm==RREF_GUESS)
	algorithm=RREF_LAGRANGE;
    }
    else
      res=a;
    //if (debug_infolevel) CERR << CLOCK()*1e-6 << " convert internal" << endl;
    if (convert_internal){
      // convert a to internal form
      alg_lvar_halftan_tsimplify(res,lv,contextptr);
      res = *(e2r(res,lv,contextptr)._VECTptr);
      if (lv.size()==1 && lv.front().type==_VECT && lv.front()._VECTptr->empty()){
	// cleanup res
	int i=0;
	for (;i<res.size();++i){
	  if (res[i].type==_VECT && res[i].ref_count()==1){
	    vecteur & resi=*res[i]._VECTptr;
	    int j=0;
	    for (;j<resi.size();++j){
	      gen resij=resi[j];
	      if (resij.type<_POLY)
		continue;
	      if (resij.type==_FRAC && resij._FRACptr->den.type<=_ZINT)
		resij=resij._FRACptr->num;
	      if (resij.type==_POLY && resij._POLYptr->dim==0 && resij._POLYptr->coord.size()==1){
		gen tmp=resij._POLYptr->coord.front().value;
		if (tmp.type<_POLY)
		  continue;
		if (tmp.type==_EXT && tmp._EXTptr->type==_VECT && (tmp._EXTptr+1)->type==_VECT && is_integer_vecteur(*tmp._EXTptr->_VECTptr) && is_integer_vecteur(*(tmp._EXTptr+1)->_VECTptr))
		  continue;
	      }
	      break;
	    }
	    if (j!=resi.size())
	      break;
	  }
	  else break;
	}
	if (i==res.size()){
	  for (i=0;i<res.size();++i){
	    if (res[i].type==_VECT && res[i].ref_count()==1){
	      vecteur & resi=*res[i]._VECTptr;
	      for (int j=0;j<resi.size();++j){
		gen & resij=resi[j];
		if (resij.type==_FRAC && resij._FRACptr->den.type<=_ZINT){
		  gen tmp=resij._FRACptr->num;
		  if (tmp.type==_POLY && tmp._POLYptr->dim==0 && tmp._POLYptr->coord.size()==1)
		    resij=fraction(tmp._POLYptr->coord.front().value,resij._FRACptr->den);
		}
		if (resij.type==_POLY && resij._POLYptr->dim==0 && resij._POLYptr->coord.size()==1)
		  resij=resij._POLYptr->coord.front().value;
	      }
	    }
	  }
	}
      }
    }
    //if (debug_infolevel) CERR << CLOCK()*1e-6 << " end convert internal" << endl;
    int lvs=int(lv.size());
    // COUT << res << endl;
    gen lcm_deno,gcd_num;
    gen detnum = plus_one;
    gen detden = plus_one;
    if (algorithm!=RREF_GAUSS_JORDAN){
      // remove common denominator of each line (fraction-free elim)
      iterateur it=res.begin(),itend=res.end();
      for (;it!=itend;++it){
	if (num_mat){ // divide each line by max coeff in abs value
	  lcm_deno=linfnorm(*it,contextptr);
	  detnum=lcm_deno*detnum;
	  multvecteur(inv(lcm_deno,contextptr),*it->_VECTptr,*it->_VECTptr);
	}
	else { // non num mat
	  lcm_deno=common_deno(*it->_VECTptr);
	  if (!is_one(lcm_deno)){
	    iterateur jt=it->_VECTptr->begin(),jtend=it->_VECTptr->end();
	    for (;jt!=jtend;++jt){
	      if (jt->type==_FRAC){
		gen nm(jt->_FRACptr->num);
		gen dn(jt->_FRACptr->den);
		// *jt -> lcmdeno* (nm/dn) = nm * tmp/dn
		gen tmp(lcm_deno);
		simplify(tmp,dn);
		if (dn.type<=_CPLX){
		  *jt=nm*tmp/dn;
		  continue;
		}
		if (dn.type==_POLY){
		  *jt=nm*tmp/dn._POLYptr->coord.front().value;
		  continue;
		}
		return 0; // settypeerr();
	      }
	      else
		*jt=(*jt) * lcm_deno;
	    }
	    detden = detden * lcm_deno;
	  }
	  gcd_num=common_num(*it->_VECTptr);
	  if (!is_zero(gcd_num,contextptr))
	    *it=rdiv(*it,gcd_num,contextptr);
	  detnum=detnum*gcd_num;
	} // end else (non num mat)
      } // end for (;it!=itend;)
      // check if res is integer or polynomial
      if (lvs==1 && lv.front().type==_VECT && lv.front()._VECTptr->empty() && (rref_or_det_or_lu==1 || modular ) && is_integer_matrice(res) && as<=a0s){
	matrice res1;
	if (!mrref(res,res1,pivots,det,l,lmax,c,cmax,fullreduction,dont_swap_below,false,algorithm,rref_or_det_or_lu,contextptr))
	  return 0;
	res=res1;
	det=detnum*det/detden;
	if (convert_internal)
	  det=r2sym(det,lv,contextptr);
	if (rref_or_det_or_lu==2 || rref_or_det_or_lu == 3){
	  vecteur P;
	  vector_int2vecteur(permutation,P);
	  pivots.push_back(P);
	}
	return 1;
      }
      bool fullreductionafter=rref_or_det_or_lu==0 && dont_swap_below==0 && c==0 && linit==0 && cmax>=lmax && step_infolevel(contextptr)==0 && fullreduction && algorithm!=RREF_GAUSS_JORDAN; // insure all pivots are normalized to be = to the determinant
      if ( ( (rref_or_det_or_lu==1 && as==a0s ) || fullreductionafter) 
	   && as>4 
	   && algorithm==RREF_GUESS && ( (convert_internal && lvs==1 && lv.front().type==_VECT) || res.front()._VECTptr->front().type==_POLY) ){
	// guess if Bareiss or Lagrange interpolation is faster
	// Bareiss depends on the total degree, Lagrange on partial degrees
	// gather line/columns statistics
	int polydim=res.front()._VECTptr->front().type==_POLY?res.front()._VECTptr->front()._POLYptr->dim:int(lv.front()._VECTptr->size());
	index_t col_totaldeg(as);
	vector< index_t > col_partialdeg(as,index_t(polydim));
	int maxtotaldeg=0,summaxtotaldeg=0;
	if (polydim){
	  index_t summaxdeg(polydim);
	  for (unsigned int i=0;i<as;++i){
	    index_t maxdeg(polydim);
	    for (unsigned int j=0;j<as;++j){
	      const gen & tmp = (*res[i]._VECTptr)[j];
	      if (tmp.type==_POLY){
		const index_t & degij=tmp._POLYptr->degree();
		maxdeg=index_lcm(degij,maxdeg);
		col_partialdeg[j]=index_lcm(degij,col_partialdeg[j]);
		int totaldeg=tmp._POLYptr->total_degree();
		if (maxtotaldeg<totaldeg)
		  maxtotaldeg=totaldeg;
		if (col_totaldeg[j]<totaldeg)
		  col_totaldeg[j]=totaldeg;
	      }
	    }
	    summaxtotaldeg += maxtotaldeg;
	    summaxdeg=summaxdeg+maxdeg;
	  }
	  maxtotaldeg=std::min(summaxtotaldeg,int(total_degree(col_totaldeg)));
	  index_t col_sumpartialdeg(polydim);
	  for (unsigned int j=0;j<as;++j)
	    col_sumpartialdeg = col_sumpartialdeg+col_partialdeg[j];
	  for (int i=0;i<polydim;++i){
	    summaxdeg[i]=std::min(summaxdeg[i],col_sumpartialdeg[i]);
	  }
	  if (debug_infolevel>2)
	    CERR << "Total degree " << maxtotaldeg << ", partial degrees " << summaxdeg << endl;
	  // Now modify algorithm to RREF_LAGRANGE if it's faster
	  double lagrange_time=std::pow(double(as),2)*(as*10+160); 
	  // coeffs of as*.+. are guess
	  for (int j=0;j<polydim;j++){
	    lagrange_time *= (summaxdeg[j]+1);
	  }
	  double bareiss_time=0;
	  // time is almost proportionnal to sum( comb(maxtotaldeg*j/as+polydim,polydim)^2, j=1..as-1)
	  for (unsigned int j=1;j<as;++j){
	    int tmpdeg=int(double(maxtotaldeg*j)/as+.5);
	    double tmp = evalf_double(comb(tmpdeg+polydim,polydim),1,contextptr)._DOUBLE_val;
	    tmp = tmp*tmp*std::log(tmp)*(as-j);
	    bareiss_time += 4*tmp; // 1 for *, 1 for /
	  }
	  bareiss_time *= as; // take account of the size of the coefficients
	  if (debug_infolevel>2)
	    CERR << "lagrange " << lagrange_time << " bareiss " << bareiss_time << endl;
	  if (lagrange_time<bareiss_time){
	    algorithm=RREF_LAGRANGE;
	  }
	} // end if (polydim)
      }
      if ( algorithm==RREF_LAGRANGE && ( (rref_or_det_or_lu==1 && as==a0s) || fullreductionafter ) ){
	vecteur lva=lvar(a);
	if ( (!convert_internal && lva.empty()) || (lvs==1 && lv.front()==lva) ){
	  // find degrees wrt main variable
	  int polydim=0;
	  int totaldeg=0;
	  gen coeffp; int tt=0;
	  vector<int> maxdegj(a0s);
	  for (unsigned int i=0;i<as;++i){
	    int maxdegi=0;
	    for (unsigned int j=0;j<a0s;++j){
	      gen & tmp = (*res[i]._VECTptr)[j];
	      if (tmp.type==_POLY){
		if (tt!=_USER)
		  tt=coefftype(*tmp._POLYptr,coeffp);
		polydim=tmp._POLYptr->dim;
		const int & curdeg=tmp._POLYptr->lexsorted_degree();
		if (curdeg>maxdegi)
		  maxdegi=tmp._POLYptr->lexsorted_degree();
		if (curdeg>maxdegj[j])
		  maxdegj[j]=curdeg;
		tmp=polynome2poly1(tmp,1);
	      }
	    }
	    totaldeg+=maxdegi;
	  }
	  if (polydim){
	    if (debug_infolevel)
	      CERR << CLOCK()*1e-6 << " det: begin interp" << endl;
	    totaldeg=std::min(totaldeg,total_degree(maxdegj));
	    if (!interpolable(totaldeg+1,coeffp,true,contextptr))
	      return 0;
	    int shift=coeffp.type?0:totaldeg/2;
	    proba_epsilon(contextptr) /= totaldeg;
	    vecteur X(totaldeg+1),Y(totaldeg+1),Z(totaldeg+1);
	    int x=0;
	    for (;x<=totaldeg;++x){
	      gen realx=interpolate_xi(x-shift,coeffp);
	      X[x]=realx;
	      vecteur resx;
	      resx.reserve(totaldeg+1);
	      if (debug_infolevel)
		CERR << CLOCK()*1e-6 << " det: begin horner" << endl;
	      for (unsigned int i=0;i<as;++i){
		vecteur resxi;
		resxi.reserve(a0s); // was (totaldeg+1);
		for (unsigned int j=0;j<a0s;++j){
		  const gen & tmp = (*res[i]._VECTptr)[j];
		  resxi.push_back(horner(tmp,realx));
		}
		resx.push_back(resxi);
	      }
	      if (debug_infolevel)
		CERR << CLOCK()*1e-6 << " det: end horner" << endl;
	      matrice res1;
	      if (!mrref(resx,res1,pivots,det,l,lmax,c,cmax,-fullreduction,dont_swap_below,false,algorithm_,rref_or_det_or_lu,contextptr))
		return 0;
	      Y[x]=det;
	      if (fullreduction){
		if (is_zero(det) )
		  break;
		// check diagonal coefficients of res1, they must be == det
		for (int i=0;i<res1.size();++i){
		  vecteur & res1i=*res1[i]._VECTptr;
		  if (res1i[i]==det)
		    continue;
		  if (res1i[i]==-det)
		    res1[i]=-res1[i];
		  else
		    res1[i]=(det/res1i[i])*res1[i];
		}
		// extract right submatrix
		res1=mtran(res1);
		Z[x]=vecteur(res1.begin()+lmax,res1.end());
	      } // if (fullreduction)
	    } // end for x
	    if (x==totaldeg+1){
	      proba_epsilon(contextptr) *= totaldeg;
	      if (debug_infolevel)
		CERR << CLOCK()*1e-6 << " det: divided diff" << endl;
	      // Lagrange interpolation
	      vecteur L=divided_differences(X,Y);
	      if (debug_infolevel)
		CERR << CLOCK()*1e-6 << " det: end divided diff" << endl;
	      det=untrunc1(L[totaldeg]);
	      monomial<gen> mtmp(1,1,polydim);
	      gen xpoly=polynome(mtmp);
	      for (int i=totaldeg-1;i>=0;--i){
		det = det*(xpoly-untrunc1(X[i]))+untrunc1(L[i]);
	      }
	      det=det*detnum/detden;
	      if (convert_internal)
		det=r2sym(det,lva,contextptr);
	      if (debug_infolevel)
		CERR << CLOCK()*1e-6 << " det: end interp" << endl;
	      if (fullreduction){
		vecteur R,RR;
		interpolate(X,Z,R,0);
		polymat2matpoly(R,RR);
		// apply poly12polynome in elements of R
		for (int i=0;i<RR.size();++i){
		  if (RR[i].type!=_VECT)
		    continue;
		  vecteur & w=*RR[i]._VECTptr;
		  for (int j=0;j<w.size();++j){
		    if (w[j].type==_VECT){
		      w[j]=poly12polynome(*w[j]._VECTptr,1,polydim);
		    }
		    if (convert_internal)
		      w[j]=r2sym(w[j],lva,contextptr);
		  }
		}
		vecteur R0=midn(lmax);
		for (int i=0;i<R0.size();++i){
		  (*R0[i]._VECTptr)[i]=det;
		}
		R=mergevecteur(R0,RR);
		res=mtran(R);
	      }
	      return 1;
	    } // if interpolation ok (x==totaldeg+1)
	    else { // back convert from poly1 to polynome
	      for (unsigned int i=0;i<as;++i){
		for (unsigned int j=0;j<a0s;++j){
		  gen & tmp = (*res[i]._VECTptr)[j];
		  if (tmp.type==_VECT){
		    tmp=poly12polynome(*tmp._VECTptr,1,polydim);
		  }
		}
	      }
	    }
	  } // end if polydim
	}
      }
    }

    std_matrix<gen> M;
    matrice2std_matrix_gen(res,M);
    // vecteur vtemp;
    pivots.clear();
    pivots.reserve(cmax-c);
    bool fullreductionafter=rref_or_det_or_lu==0 && dont_swap_below==0 && cmax-c>=lmax-linit && step_infolevel(contextptr)==0 && fullreduction && algorithm!=RREF_GAUSS_JORDAN;
    gen detnumsave=detnum;
    int status=rref_reduce(M,pivots,permutation,det,detnum,algorithm,l,lmax,c,cmax,dont_swap_below,rref_or_det_or_lu,(fullreductionafter?0:fullreduction),eps,step_rref,lv,convert_internal,contextptr);
    if (status!=2 && status!=3)
      return status;
    if (fullreductionafter){ 
      det=M[lmax-1][c-linit+lmax-1];
      if (status==3 || is_exactly_zero(det)){
      //if (status==3 || is_zero(det,contextptr) ){ 
	// not Cramer like, re-reduce, 
	pivots.clear();
	matrice2std_matrix_gen(res,M); det=detnum=detnumsave;// this should be commented but some outputs are more complicated
	rref_reduce(M,pivots,permutation,det,detnum,algorithm,l,lmax,c,cmax,dont_swap_below,rref_or_det_or_lu,fullreduction,eps,step_rref,lv,convert_internal,contextptr);
      }
      else {
	// back row reduction to echelon form for Cramer like system
	vecteur & Mlast=M[lmax-1];
	int shift=c-linit;
	gen d=Mlast[shift+lmax-1];
	for (l=lmax-2;l>=linit;--l){
	  vecteur Mlcopy(M[l]);
	  vecteur & Ml=M[l];
	  // Ll <- a_{n-1,n-1}*Ll-(a_{l,n-1}*L_{n-1}-a_{l,n-2}*L_{n-2}...)
	  // multvecteur(d,Ml,Ml); // should be done from shift+lmax
	  for (int j=shift+lmax;j<cmax;++j){
	    gen & Mlj=Ml[j];
	    Mlj = d*Mlj;
	  }
	  for (int j=lmax-1;j>=l+1;--j){
	    linear_combination(plus_one,Ml,-Mlcopy[shift+j],M[j],plus_one,1,Ml,eps,shift+lmax);
	  }
	  for (int j=shift+l+1;j<shift+lmax;++j)
	    Ml[j]=0;
	  Ml[shift+l]=d;
	  for (int j=shift+lmax;j<cmax;++j){
	    Ml[j]=exact_div(Ml[j],Mlcopy[shift+l]);
	  }
	}
      }
    } // end if fullreductionafter
    if (debug_infolevel>2)
      CERR << "// mrref reduction end:" << CLOCK() << endl;
    if (algorithm!=RREF_GAUSS_JORDAN){
      int last=giacmin(lmax,cmax);
      det=M[last-1][last-1];
      if ( (debug_infolevel>2) && (det.type==_POLY) )
	CERR << "// polynomial size " << det._POLYptr->coord.size() << endl;
      if (rref_or_det_or_lu==1) // erase last line of the matrix
	M[lmax-1].clear();
      det=rdiv(det*detnum,detden,contextptr);
      if (convert_internal)
	det=r2sym(det,lv,contextptr);
      // CERR << det << endl;
    }
    else {
      // adjust determinant by multiplication by all diagonal coeffs
      for (int i=linit;i<lmax && i<cmax;++i)
	detnum = detnum * M[i][i];
      det = rdiv(detnum,detden,contextptr);
      if (convert_internal)
	det = r2sym(det,lv,contextptr);
    }
    std_matrix_gen2matrice_destroy(M,res);
    int ok=1;
    if (convert_internal){
      if (rm_idn_after){
	if (!remove_identity(res,contextptr))
	  return 0;
	res = *(r2sym (res,lv,contextptr)._VECTptr);
	res =*normal(res,contextptr)._VECTptr;
	ok=2;
      }
      else
	res = *(r2sym (res,lv,contextptr)._VECTptr);
    }
    if (rref_or_det_or_lu==2 || rref_or_det_or_lu == 3){
      vecteur P;
      vector_int2vecteur(permutation,P);
      pivots.push_back(P);
    }
    if (debug_infolevel>2)
      CERR << "// mrref end:" << CLOCK() << " " << M << endl;
    return ok;
  }

  // convert a to vector< vector<int> > with modular reduction (if modulo!=0)
  void vect_vecteur_2_vect_vector_int(const std_matrix<gen> & M,int modulo,vector< vector<int> > & N){
    int Msize=int(M.size());
    N.clear();
    N.reserve(Msize);
    for (int k=0;k<Msize;k++){
      const vecteur & v = M[k];
      const_iterateur it=v.begin(),itend=v.end();
      vector<int> vi(itend-it);
      vector<int>::iterator jt=vi.begin();
      for (;it!=itend;++jt,++it){
	if (!modulo)
	  *jt=it->val;
	else
	  *jt=smod(*it,modulo).val;
      }
      N.push_back(vi);
    }
  }

  void vect_vector_int_2_vect_vecteur(const vector< vector<int> > & N,std_matrix<gen> & M){
    // Back convert N to M
    int Msize=int(N.size());
    M = std_matrix<gen>(Msize);
    for (int k=0;k<Msize;k++){
      const vector<int> & v = N[k];
      vector<int>::const_iterator it=v.begin(),itend=v.end();
      vecteur vi(itend-it);
      iterateur jt=vi.begin();
      for (;it!=itend;++jt,++it){
	*jt=*it;
      }
      M[k]=vi;
    }
  }

  //transforme un vecteur en vector<int>  
  void vecteur2vector_int(const vecteur & v,int m,vector<int> & res){
    vecteur::const_iterator it=v.begin(),itend=v.end();
    res.clear();
    if (m==0) {
      res.resize(itend-it);
      int * jt=&res.front();
      for (;it!=itend;++it,++jt){
	int t=it->type;
	if (t==0)
	  *jt=it->val; 
	else {
	  if (t==_MOD)
	    *jt=it->_MODptr->val;
	  else
	    *jt=it->to_int(); 
	}
      }
      return;
    }
    res.reserve(itend-it);
    if (m<0)
      m=-m;
    for (;it!=itend;++it){
      if (it->type==_MOD)
	res.push_back(it->_MODptr->val);
      else {
	int r=it->type==_ZINT?modulo(*it->_ZINTptr,m):(it->val % m);
	r += (unsigned(r)>>31)*m; // make positive
	r -= (unsigned((m>>1)-r)>>31)*m;
	res.push_back(r);// res.push_back(smod((*it),m).val); 
      }
    }
  } 

  bool vecteur2vectvector_int(const vecteur & v,int modulo,vector< vector<int> > & res){
    vecteur::const_iterator it=v.begin(),itend=v.end();
    res.resize(itend-it);
    for (int i=0;it!=itend;++i,++it){
      if (it->type!=_VECT)
	return false;
      vecteur2vector_int(*it->_VECTptr,modulo,res[i]);
    }
    return true;
  }

  void vector_int2vecteur(const vector<int> & v,vecteur & res){
    //transforme un vector<int> en vecteur 
    vector<int>::const_iterator it=v.begin(),itend=v.end();
    res.resize(itend-it);
    for (iterateur jt=res.begin();it!=itend;++jt,++it)
      *jt=*it;
  } 

  void vectvector_int2vecteur(const vector< vector<int> > & v,vecteur & res){
    //transforme un vector< vector<int> > en vecteur  
    int s=int(v.size());
    res.resize(s);
    for (int i=0;i<s;++i){
      if (res[i].type!=_VECT)
	res[i]=new ref_vecteur;
      vector_int2vecteur(v[i],*res[i]._VECTptr);
    }
  }


  // if dont_swap_below !=0, for line numers < dont_swap_below
  // the pivot is searched in the line instead of the column
  // hence no line swap occur
  // rref_or_det_or_lu = 0 for rref, 1 for det, 2 for lu, 
  // 3 for lu without permutation
  // fullreduction=0 or 1, use 2 if the right part of a is idn
  bool modrref(const matrice & a, matrice & res, vecteur & pivots, gen & det,int l, int lmax, int c,int cmax,int fullreduction,int dont_swap_below,const gen & modulo,bool ckprime,int rref_or_det_or_lu){
    if (ckprime && !is_probab_prime_p(modulo)){
      CERR << "Non prime modulo. Reduction mail fail" << endl;
    }
    // bool use_cstart=!c;
    // bool inverting=fullreduction==2;
    det = 1;
    int linit=l;//,previous_l=l;
    vecteur lv;
    // Large mod reduction (coeff do not fit in an int)
    res=a;
    // COUT << res << endl;
    std_matrix<gen> M;
    matrice2std_matrix_gen(res,M);
    gen pivot,temp;
    // vecteur vtemp;
    int pivotline,pivotcol;
    pivots.clear();
    pivots.reserve(cmax-c);
    for (;(l<lmax) && (c<cmax);){
      if ( (!fullreduction) && (l==lmax-1) ){
	det = smod(det*M[l][c],modulo);
	break;
      }
      pivot=M[l][c];
      pivotline=l;
      pivotcol=c;
      if (is_exactly_zero(pivot)){ // scan current line
	if (rref_or_det_or_lu==3){
	  det=0;
	  return true;
	}
	if (l<dont_swap_below){ 
	  for (int ctemp=c+1;ctemp<cmax;++ctemp){
	    temp=M[l][ctemp];
	    if (!is_exactly_zero(temp)){
	      pivot=temp;
	      pivotcol=ctemp;
	      break;
	    }
	  }
	}
	else {      // scan M current column for the best pivot available
	  for (int ltemp=l+1;ltemp<lmax;++ltemp){
	    temp=M[ltemp][c];
	    if (debug_infolevel>2)
	      print_debug_info(temp);
	    if (!is_exactly_zero(temp)){
	      pivot=temp;
	      pivotline=ltemp;
	      break;
	    }
	  }
	}
      } // end if is_zero(pivot), true pivot found on line or column
      if (!is_exactly_zero(pivot)){
	if (l!=pivotline){
	  swap(M[l],M[pivotline]);
	  det = -det;
	}
	det = smod(det*pivot,modulo);
	// save pivot for annulation test purposes
	if (rref_or_det_or_lu!=1)
	  pivots.push_back(pivot);
	// invert pivot 
	temp=invmod(pivot,modulo);
	if (fullreduction || rref_or_det_or_lu<2){
	  iterateur it=M[l].begin(),itend=M[l].end();
	  for (;it!=itend;++it)
	    *it=smod(temp * *it,modulo);
	}
	// make the reduction
	if (fullreduction){
	  for (int ltemp=linit;ltemp<lmax;++ltemp){
	    if (ltemp!=l)
	      modlinear_combination(M[ltemp],-M[ltemp][pivotcol],M[l],modulo,0);
	  }
	}
	else {
	  for (int ltemp=l+1;ltemp<lmax;++ltemp){
	    if (rref_or_det_or_lu>=2)
	      M[ltemp][pivotcol]=smod(M[ltemp][pivotcol]*temp,modulo);
	    modlinear_combination(M[ltemp],-M[ltemp][pivotcol],M[l],modulo,(c+1)*(rref_or_det_or_lu>0));
	  }
	} // end else
	// increment column number if swap was allowed
	if (l>=dont_swap_below)
	  ++c;
	// increment line number since reduction has been done
	++l;	  
      } // end if (!is_zero(pivot)
      else { // if pivot is 0 increment either the line or the col
	det = 0;
	if (rref_or_det_or_lu==1)
	  return true;
	if (l>=dont_swap_below)
	  c++;
	else
	  l++;
      }
    } // end for reduction loop
    std_matrix_gen2matrice_destroy(M,res);
    return true;
  }

  bool mrref(const matrice & a, matrice & res, vecteur & pivots, gen & det,GIAC_CONTEXT){
    return mrref(a,res,pivots,det,0,int(a.size()),0,int(a.front()._VECTptr->size()),
		 /* fullreduction */ 1,0,true,1,0,
	  contextptr)!=0;
  }

  bool modrref(const matrice & a, matrice & res, vecteur & pivots, gen & det,const gen& modulo){
    return modrref(a,res,pivots,det,0,int(a.size()),0,int(a.front()._VECTptr->size()),
		   true /* full reduction */,0 /* dont_swap_below*/,modulo,true /*ckprime*/,0 /* rref */);
  }

  // add identity matrix, modifies arref in place
  void add_identity(matrice & arref){
    int s=int(arref.size());
    vecteur v;
    gen un(1),zero(0);
    if (!arref.empty() && has_num_coeff(arref)){
      gen tmp=arref.front()._VECTptr->front();
      if (is_zero(tmp))
	tmp= tmp+1;
      un=tmp/tmp;
      zero=tmp-tmp;
    }
    for (int i=0;i<s;++i){
      gen tmp=new ref_vecteur(2*s,zero);
      iterateur it=tmp._VECTptr->begin(),jt=arref[i]._VECTptr->begin(),jtend=jt+s;
      for (;jt!=jtend;++it,++jt)
	*it=*jt;
      it+=i;
      *it=un;
      arref[i] = tmp;
    }
  }


  bool remove_identity(matrice & res){
    return remove_identity(res,context0);
  }

  // res += pn*x
  void add_multvecteur(vecteur & res,const gen & pn,const vecteur & x){
    iterateur it=res.begin(),itend=res.end();
    const_iterateur jt=x.begin();
    for (;it!=itend;++jt,++it){
#ifdef USE_GMP_REPLACEMENTS
      *it += pn*(*jt);
#else
      if (it->type==_ZINT && it->ref_count()==1 && pn.type==_ZINT){
	if (jt->type==_INT_){
	  if (jt->val>0)
	    mpz_addmul_ui(*it->_ZINTptr,*pn._ZINTptr,jt->val);
	  else
	    mpz_submul_ui(*it->_ZINTptr,*pn._ZINTptr,-jt->val);
	}
	else
	  mpz_addmul(*it->_ZINTptr,*pn._ZINTptr,*jt->_ZINTptr);	  
      }
      else
	*it += pn*(*jt);
#endif
    }
  }

  bool iszero(const vector<int> & p){
    vector<int>::const_iterator it=p.begin(),itend=p.end();
    for (;it!=itend;++it){
      if (*it)
	return false;
    }
    return true;    
  }
  
  static gen init_modulo(int n,double logbound){
#if 1 // def _I386_
    double pinit= double(longlong(1) << 60);
    pinit /=n ;
    pinit = std::sqrt(pinit);
    pinit -= 3*logbound; // keep enough primes satisfying p^2*n<2^63
    return nextprime(int(pinit)); 
#else
    return 36007;
#endif
  }

  // solve A*x=b where a and b have integer coeffs using a p-adic algorithm 
  // (ignoring extra columns of A)
  // lcmdeno of the answer may be used to give an estimate of the 
  // least divisor element of A if b is random
  // returns 0 if no invertible found, -1 if det==0, 1 otherwise
  int padic_linsolve(const matrice & A,const vecteur & b,vecteur & res,gen & p,gen & det_mod_p,gen & h2,unsigned reconstruct,int maxtry){
    return 0;
  }

  matrice mrref(const matrice & a,GIAC_CONTEXT){
    if (a.empty())
      return vecteur(vecteur(1,gendimerr(contextptr)));
    gen det;
    vecteur pivots;
    matrice res;
    if (!mrref(a,res,pivots,det,0,int(a.size()),0,int(a.front()._VECTptr->size()),
	  /* fullreduction */1,0,true,1,0,
	       contextptr))
      return vecteur(1,vecteur(1,gendimerr(contextptr)));
    return res;
  }

  bool read_reduction_options(const gen & a_orig,matrice & a,bool & convert_internal,int & algorithm,bool & minor_det,bool & keep_pivot,int & last_col){
    convert_internal=true;
    algorithm=RREF_GUESS;
    minor_det=false;
    keep_pivot=false;
    last_col=-1;
    if (ckmatrix(a_orig)){
      a=*a_orig._VECTptr;
    }
    else { // rref with options
      if (a_orig.type!=_VECT)
	return false;
      vecteur v=*a_orig._VECTptr;
      int s=int(v.size());
      if (s<=3 && v[0].is_symb_of_sommet(at_pnt)){
	for (int i=0;i<s;++i){
	  if (v[i].subtype==_VECTOR__VECT && v[i]._VECTptr->size()==2)
	    v[i]=v[i]._VECTptr->back()-v[i]._VECTptr->front();
	  if (v[i].type!=_VECT){
	    gen a,b;
	    reim(v[i],a,b,context0);
	    v[i]=makevecteur(a,b);
	  }
	}
	if (ckmatrix(v))
	  return read_reduction_options(v,a,convert_internal,algorithm,minor_det,keep_pivot,last_col);
      }
      if (!s || !ckmatrix(v[0]))
	return false;
      a=*v[0]._VECTptr;
      for (int i=1;i<s;++i){
	if (v[i]==at_lagrange)
	  algorithm=RREF_LAGRANGE;
	if (v[i]==at_irem)
	  algorithm=RREF_MODULAR;
	if (v[i]==at_linsolve)
	  algorithm=RREF_PADIC;
	if (v[i].type==_INT_){
	  if (v[i].subtype==_INT_SOLVER){
	    switch (v[i].val){
	    case _RATIONAL_DET:
	      convert_internal=false;
	      algorithm=RREF_GAUSS_JORDAN;
	      break;
	    case _BAREISS:
	      algorithm=RREF_BAREISS;
	      break;
	    case _KEEP_PIVOT:
	      keep_pivot=true;
	      break;
	    case _MINOR_DET:
	      minor_det=true;
	    }
	  }
	  else
	    last_col=v[i].val;
	}
      }
    }
    return true;
  }
  gen _rref(const gen & a_orig,GIAC_CONTEXT) {
    if ( a_orig.type==_STRNG && a_orig.subtype==-1) return  a_orig;
    matrice a;
    bool convert_internal,minor_det,keep_pivot;
    int algorithm,last_col;
    if (!read_reduction_options(a_orig,a,convert_internal,algorithm,minor_det,keep_pivot,last_col))
      return gensizeerr(contextptr);
    if (minor_det)
      return gensizeerr(gettext("minor_det option applies only to det"));
    gen det;
    vecteur pivots;
    matrice res;
    int ncols=int(a.front()._VECTptr->size());
    if (last_col>=0)
      ncols=giacmin(ncols,last_col);
    if (!mrref(a,res,pivots,det,0,int(a.size()),0,ncols,
	  /* fullreduction */1,0,convert_internal,algorithm,0,
	       contextptr))
      return gendimerr(contextptr);
    if (!keep_pivot){
      mdividebypivot(res,ncols,contextptr);
    }
    if (res.front().type==_VECT && res.front()._VECTptr->front().type==_MOD)
      return res;
    return ratnormal(res,contextptr);
  }
  static const char _rref_s []="rref";
  static define_unary_function_eval (__rref,&_rref,_rref_s);
  define_unary_function_ptr5( at_rref ,alias_at_rref,&__rref,0,true);

  // returns 0 if all elements are 0
  static gen first_non_zero(const vecteur & v,int lastcol,GIAC_CONTEXT){
    vecteur::const_iterator it=v.begin(),itend=v.end();
    if (itend-it>lastcol)
      itend=it+lastcol;
    if (has_num_coeff(v)){
      gen vmax=0,tmp; 
      // in approx mode, we want to make a relative comparison with 0
      // find the largest value of the row in absolute value
      for (;it!=itend;++it){
	if (has_evalf(*it,tmp,1,contextptr) && is_greater((tmp=abs(tmp,contextptr)),vmax,contextptr))
	  vmax=tmp;
      }
      it=v.begin();
      vmax=inv(vmax,contextptr);
      for (;it!=itend;++it){
	if (!is_zero(*it*vmax,contextptr))
	  //if (!is_exactly_zero(*it))
	  return *it;
      }
    }
    else {
      for (;it!=itend;++it){
	if (!is_zero(*it,contextptr))
	  return *it;
      }
    }
    return 0;
  }

  void mdividebypivot(matrice & a,int lastcol,GIAC_CONTEXT){
    if (lastcol==-1)
      lastcol=int(a.front()._VECTptr->size());
    if (lastcol==-2)
      lastcol=int(a.front()._VECTptr->size())-1;
    if (lastcol<0)
      lastcol=0;
    vecteur::const_iterator ita=a.begin(),itaend=a.end();
    gen pivot;
    for (;ita!=itaend;++ita){
      pivot=first_non_zero(*(ita->_VECTptr),lastcol,contextptr);
      if (!is_exactly_zero(pivot))
	divvecteur(*(ita->_VECTptr),pivot,*(ita->_VECTptr));
    }
  }

  void mdividebypivot(matrice & a,int lastcol){
    mdividebypivot(a,lastcol,context0);
  }

  void midn(int n,matrice & res){
    if (n<=0 || longlong(n)*n>LIST_SIZE_LIMIT){
      res= vecteur(1,vecteur(1,gendimerr(gettext("idn"))));
      return ;
    }
    res.clear();
    res.reserve(n);
    vecteur v;
    for (int i=0;i<n;++i){
      res.push_back(new ref_vecteur(n));
      (*res[i]._VECTptr)[i]=1;
    }
  }

  matrice midn(int n){
    matrice res;
    midn(n,res);
    return res;
  }

  gen _idn(const gen & e,GIAC_CONTEXT) {
    if ( e.type==_STRNG && e.subtype==-1) return  e;
    matrice res;
    if (e.type==_INT_)
      midn(e.val,res);
    else {
      if (e.type==_DOUBLE_)
	midn(int(e._DOUBLE_val),res);
      else {
	if ((e.type==_VECT) && is_squarematrix(*e._VECTptr))
	  midn(int(e._VECTptr->size()),res);
	else
	  return gensizeerr(contextptr);
      }
    }
    return gen(res,_MATRIX__VECT);
  }
  static const char _idn_s []="idn";
  static define_unary_function_eval (__idn,&_idn,_idn_s);
  define_unary_function_ptr5( at_idn ,alias_at_idn,&__idn,0,true);

  // find index i of x in v that is i such that v[i] <= x < v[i+1]
  // where v[-1]=-inf, and v[v.size()]=+inf
  int dichotomy(const vector<giac_double> & v,double x){
    int s=int(v.size());
    if (x<v[0])
      return -1;
    if (x>=v[s-1])
      return s-1;
    int a=0, b=s-1; // v[a] <= x < v[b]
    while (b-a>1){
      int c=(a+b)/2;
      if (x>=v[c])
	a=c;
      else
	b=c;
    }
    return a;
  }

  void vranm(int n,const gen & F,vecteur & res,GIAC_CONTEXT){
    gen f(F);
    // if (F.type==_USER) f=symbolic(at_rand,F);
    n=giacmax(1,n);
    if (n>LIST_SIZE_LIMIT){
      res=vecteur(1,gendimerr(contextptr)); // setstabilityerr();
      return;
    }
    res.reserve(n);
    if (is_zero(f,contextptr)){
      for (int i=0;i<n;++i){
	res.push_back((int) (2*randrange*giac_rand(contextptr)/(rand_max2+1.0)-randrange));
      }
      return;
    }
    if (is_integer(f)){
      if (f.type==_INT_){
	int t=f.val;
	if (t<0){
	  for (int i=0;i<n;++i)
	    res.push_back((int) (2*t*(giac_rand(contextptr)/(rand_max2+1.0))-t)	);
	  return;
	}
#if 0
	int add=(xcas_mode(contextptr)==3 || abs_calc_mode(contextptr)==38);
	for (int i=0;i<n;++i)
	  res.push_back(add+(int) t*(giac_rand(contextptr)/(rand_max2+1.0)));
	return;
#endif
      }
      for (int i=0;i<n;++i)
	res.push_back(_rand(f,contextptr));
      return;
    }
    if (f.type==_MOD){
      gen fm=*(f._MODptr+1);
      vranm(n,fm,res,contextptr);
      for (int i=0;i<n;++i)
	res[i]=makemod(res[i],fm);
      return;
    }
    if (f.type==_VECT){
      const vecteur & v = *f._VECTptr;
      int s=int(v.size());
      for (int i=0;i<n;++i){
	double d=giac_rand(contextptr)*double(s)/(rand_max2+1.0);
	res.push_back(v[int(d)]);
      }
      return;
    }
    if (f.is_symb_of_sommet(at_interval) && f._SYMBptr->feuille.type==_VECT){
      gen x=evalf(f._SYMBptr->feuille._VECTptr->front(),1,contextptr),y=evalf(f._SYMBptr->feuille._VECTptr->back(),1,contextptr);
      if (x.type==_DOUBLE_ && y.type==_DOUBLE_){
	double xd=x._DOUBLE_val,yd=y._DOUBLE_val;
	double scale=(yd-xd)/(rand_max2+1.0);
	for (int i=0;i<n;++i){
	  double xr= giac_rand(contextptr)*scale+xd;
	  res.push_back(xr);
	}
	return;
      }
      for (int i=0;i<n;++i)
	res.push_back(rand_interval(*f._SYMBptr->feuille._VECTptr,false,contextptr));
      return;
    }
    if (f==at_uniform || f==at_uniformd){
      for (int i=0;i<n;++i)
	res.push_back(giac_rand(contextptr)/(rand_max2+1.0));
      return;
    }
    if (f.is_symb_of_sommet(at_uniform) ||f.is_symb_of_sommet(at_uniformd) ){
      f=evalf_double(f._SYMBptr->feuille,1,contextptr);
      if (f.type!=_VECT || f._VECTptr->size()!=2 || f._VECTptr->front().type!=_DOUBLE_ || f._VECTptr->back().type!=_DOUBLE_){
	res=vecteur(1,gensizeerr(contextptr));
	return ;
      }
      double a=f._VECTptr->front()._DOUBLE_val,b=f._VECTptr->back()._DOUBLE_val,c=b-a;
      for (int i=0;i<n;++i)
	res.push_back(a+c*giac_rand(contextptr)/(rand_max2+1.0));
      return;      
    }
    if (f.is_symb_of_sommet(at_poisson) ||f.is_symb_of_sommet(at_POISSON) ){
      f=evalf_double(f._SYMBptr->feuille,1,contextptr);
      if (f.type!=_DOUBLE_ || f._DOUBLE_val<=0){
	res=vecteur(1,gensizeerr(contextptr));
	return ;
      }
      double lambda=f._DOUBLE_val;
      int Nv=int(2*lambda+53); // insure that poisson_cdf(lambda,Nv)==1 up to double precision
      if (Nv*n>5*lambda+n*std::ceil(std::log(double(Nv))/std::log(2.0))){
	vector<giac_double> tableau(Nv+1);
	long_double cumul=0;
	long_double current; 
	for (int k=0;k<Nv;++k){
	  // recompute current from time to time
	  if (k>>5==0)
	    current=std::exp(-lambda+k*std::log(lambda)-lngamma(k+1));
	  cumul += current;
	  tableau[k+1] = cumul;
	  current *= lambda/(k+1);
	}
	for (int i=0;i<n;++i){
	  res.push_back(dichotomy(tableau,double(giac_rand(contextptr))/rand_max2));
	}
	return;
      }
      for (int i=0;i<n;++i)
	res.push_back(randpoisson(lambda,contextptr));
      return;     
    }
    if (f.is_symb_of_sommet(at_exp) || f.is_symb_of_sommet(at_randexp) || f.is_symb_of_sommet(at_exponential) || f.is_symb_of_sommet(at_exponentiald)){
      f=evalf_double(f._SYMBptr->feuille,1,contextptr);
      if (f.type!=_DOUBLE_ || f._DOUBLE_val<=0){
	res=vecteur(1,gensizeerr(contextptr));
	return;
      }
      double lambda=f._DOUBLE_val;
      for (int i=0;i<n;++i)
	res.push_back(gen(-std::log(1-giac_rand(contextptr)/(rand_max2+1.0))/lambda));
      return;     
    }
    if (f.is_symb_of_sommet(at_geometric) || f.is_symb_of_sommet(at_randgeometric)){
      f=evalf_double(f._SYMBptr->feuille,1,contextptr);
      if (f.type!=_DOUBLE_ || f._DOUBLE_val<=0){
	res=vecteur(1,gensizeerr(contextptr));
	return;
      }
      double lambda=std::log(1-f._DOUBLE_val);
      for (int i=0;i<n;++i)
	res.push_back(int(std::ceil(std::log(1-giac_rand(contextptr)/(rand_max2+1.0))/lambda)));
      return;     
    }
    if (f==at_normald || f==at_NORMALD || f==at_normal || f==at_randNorm || f==at_randnormald)
      f=symbolic(at_normald,makesequence(0,1));
    if ( (f.is_symb_of_sommet(at_normald) || f.is_symb_of_sommet(at_NORMALD) || f.is_symb_of_sommet(at_normal) || f.is_symb_of_sommet(at_randNorm) || f.is_symb_of_sommet(at_randnormald)) && f._SYMBptr->feuille.type==_VECT && f._SYMBptr->feuille._VECTptr->size()==2 ){
      gen M=evalf_double(f._SYMBptr->feuille._VECTptr->front(),1,contextptr);
      f=evalf_double(f._SYMBptr->feuille._VECTptr->back(),1,contextptr);
      if (is_squarematrix(f)){
	int dim=int(f._VECTptr->size());
	vecteur w(dim);
	for (int i=0;i<n;++i){
	  for (int j=0;j<dim;++j){
	    double u=giac_rand(contextptr)/(rand_max2+1.0);
	    double d=giac_rand(contextptr)/(rand_max2+1.0);
	    w[j]=std::sqrt(-2*std::log(u))*std::cos(2*M_PI*d);
	  }
	  res.push_back(M+multmatvecteur(*f._VECTptr,w));
	}
	return;     
      }
      if (M.type!=_DOUBLE_ || f.type!=_DOUBLE_ || f._DOUBLE_val<=0 ){
	res=vecteur(1,gensizeerr(contextptr));
	return;
      }
      double m=M._DOUBLE_val,sigma=f._DOUBLE_val;
      for (int i=0;i<n;++i){
	double u=giac_rand(contextptr)/(rand_max2+1.0);
	double d=giac_rand(contextptr)/(rand_max2+1.0);
	res.push_back(m+sigma*std::sqrt(-2*std::log(u))*std::cos(2*M_PI*d));
      }
      return;     
    }
    if ( (f.is_symb_of_sommet(at_fisher) || f.is_symb_of_sommet(at_fisherd) || f.is_symb_of_sommet(at_randfisherd)
	  || f.is_symb_of_sommet(at_snedecor) 
	  || f.is_symb_of_sommet(at_randfisher)) && f._SYMBptr->feuille.type==_VECT && f._SYMBptr->feuille._VECTptr->size()==2 ){
      gen g1(f._SYMBptr->feuille._VECTptr->front()),g2(f._SYMBptr->feuille._VECTptr->back());
      if ( is_integral(g1) && g1.type==_INT_ && g1.val>0 && g1.val<=1000 && is_integral(g2) && g2.type==_INT_ && g2.val>0 && g2.val<=1000){
	int k1=g1.val,k2=g2.val;
	for (int i=0;i<n;++i)
	  res.push_back(randchisquare(k1,contextptr)/k1/(randchisquare(k2,contextptr)/k2));
	return ;
      }
    }
    if ( (f.is_symb_of_sommet(at_chisquare) || f.is_symb_of_sommet(at_randchisquare) ||
	  f.is_symb_of_sommet(at_chisquared) || f.is_symb_of_sommet(at_randchisquared) ) && f._SYMBptr->feuille.type==_INT_ && f._SYMBptr->feuille.val>0 && f._SYMBptr->feuille.val<=1000){
      int k=f._SYMBptr->feuille.val;
      for (int i=0;i<n;++i)
	res.push_back(randchisquare(k,contextptr));
      return;     
    }
    if (f==at_cauchy || f==at_cauchyd){
      for (int i=0;i<n;++i)
	res.push_back(std::tan(M_PI*giac_rand(contextptr)/(rand_max2+1.0)-.5));
      return;
    }
      if ( (f.is_symb_of_sommet(at_cauchy) || f.is_symb_of_sommet(at_cauchyd) ) && f._SYMBptr->feuille.type==_VECT && f._SYMBptr->feuille._VECTptr->size()==2 ){
      gen g1(f._SYMBptr->feuille._VECTptr->front()),g2(f._SYMBptr->feuille._VECTptr->back());
      g1=evalf_double(g1,1,contextptr);
      g2=evalf_double(g2,1,contextptr);
      if (g1.type!=_DOUBLE_ || g2.type!=_DOUBLE_){
	res=vecteur(1,gensizeerr(contextptr));
	return;
      }
      //double d1=g1._DOUBLE_val,d2=g2._DOUBLE_val;
      for (int i=0;i<n;++i)
	res.push_back(std::tan(M_PI*giac_rand(contextptr)/(rand_max2+1.0)-.5)*g2+g1);
      return;
    }
    if ( f.is_symb_of_sommet(at_student) || f.is_symb_of_sommet(at_randstudent) ||
	 f.is_symb_of_sommet(at_studentd) || f.is_symb_of_sommet(at_randstudentd)){
      if (f._SYMBptr->feuille.type==_INT_ && f._SYMBptr->feuille.val>0 && f._SYMBptr->feuille.val<=1000){
	int k=f._SYMBptr->feuille.val;
	for (int i=0;i<n;++i)
	  res.push_back(randstudent(k,contextptr));
	return; 
      }
      res= vecteur(1,gensizeerr(contextptr));
      return;
    }
    if (f.is_symb_of_sommet(at_multinomial) && f._SYMBptr->feuille.type==_VECT){
      gen P=f._SYMBptr->feuille;
      vecteur val;
      if (P._VECTptr->size()==2 && P._VECTptr->front().type==_VECT){
	if (P._VECTptr->back().type!=_VECT || P._VECTptr->front()._VECTptr->size()!=P._VECTptr->back()._VECTptr->size()){
	  res=vecteur(1,gensizeerr(contextptr));
	  return;
	}
	val=*P._VECTptr->back()._VECTptr;
	P=P._VECTptr->front();
      }
      if (!is_zero(1-_sum(P,contextptr))){
	res=vecteur(1,gensizeerr(contextptr));
	return;
      }
      const vecteur & v=*P._VECTptr;
      // cdf of probabilities
      unsigned vs=unsigned(v.size());
      vector<giac_double> tableau(vs+1);
      vector<int> eff(vs);
      if (!val.empty())
	res.reserve(n);
      gen g=evalf_double(v[0],1,contextptr);
      if (g.type!=_DOUBLE_){
	res=vecteur(1,gensizeerr(contextptr));
	return;
      }
      tableau[1]=g._DOUBLE_val*rand_max2;
      for (unsigned i=1;i<vs;++i){
	g=evalf_double(v[i],1,contextptr);
	if (g.type!=_DOUBLE_){
	  res=vecteur(1,gensizeerr(contextptr));
	  return;
	}
	tableau[i+1]=g._DOUBLE_val*rand_max2+tableau[i];
      }
      // generate n random values, count them if val=0 
      for (unsigned i=0;int(i)<n;++i){
	int j=dichotomy(tableau,giac_rand(contextptr));
	if (j>=int(vs))
	  j=vs;
	if (val.empty())
	  ++eff[j];
	else 
	  res.push_back(val[j]);
      }
      if (val.empty())
	vector_int2vecteur(eff,res);
      return;
    }
    if ( (f.is_symb_of_sommet(at_binomial) || f.is_symb_of_sommet(at_BINOMIAL))
	 && f._SYMBptr->feuille.type==_VECT && f._SYMBptr->feuille._VECTptr->size()==2){
      gen N=f._SYMBptr->feuille._VECTptr->front();
      f=evalf_double(f._SYMBptr->feuille._VECTptr->back(),1,contextptr);
      if (!is_integral(N) || N.type!=_INT_ || N.val<=0 || f.type!=_DOUBLE_ || f._DOUBLE_val<=0 || f._DOUBLE_val>=1){
	res= vecteur(1,gensizeerr(contextptr));
	return;
      }
      double p=f._DOUBLE_val;
      int Nv=N.val;
      if (Nv==1){
	int seuil=int(rand_max2*p);
	for (int i=0;i<n;++i){
	  res.push_back(giac_rand(contextptr)<=seuil);
	}
	return;
      }
      // computation time is proportionnal to Nv*n with the sum of n randoms value 0/1
      // other idea compute once binomial_cdf(Nv,k,p) for k in [0..Nv]
      // then find position of random value in the list: this costs Nv+n*ceil(log2(Nv)) operations
      if (double(Nv)*n>5*Nv+n*std::ceil(std::log(double(Nv))/std::log(2.0))){
	vector<giac_double> tableau(Nv+1);
	long_double cumul=0;
	long_double current; // =std::pow(1-p,Nv);
	for (int k=0;k<Nv;++k){
	  // recompute current from time to time
	  if (k%32==0)
	    current=std::exp(lngamma(Nv+1)-lngamma(k+1)-lngamma(Nv-k+1)+k*std::log(p)+(Nv-k)*std::log(1-p));
	  cumul += current;
	  tableau[k+1] = cumul;
	  current *= p*(Nv-k)/(k+1)/(1-p); 
	}
	for (int i=0;i<n;++i){
	  res.push_back(dichotomy(tableau,double(giac_rand(contextptr))/rand_max2));
	}
	return;
      }
      if (Nv>1000){
	for (int i=0;i<n;++i)
	  res.push_back(binomial_icdf(Nv,p,double(giac_rand(contextptr))/rand_max2,contextptr));
      }
      else {
	p *= rand_max2;
	for (int i=0;i<n;++i){
	  int ok=0;	  
	  for (int j=0;j<Nv;++j){
	    if (giac_rand(contextptr)<=p)
	      ok++;
	  }
	  res.push_back(ok);
	}
      }
      return;     
    }
    if (f.is_symb_of_sommet(at_program)){
      for (int i=0;i<n;++i)
	res.push_back(f(vecteur(0),contextptr));
      return;
    }
    if (f.is_symb_of_sommet(at_rootof)){
      gen ff=f._SYMBptr->feuille;
      if (ff.type==_VECT && !ff._VECTptr->empty()){
	ff=ff._VECTptr->back();
	if (ff.type==_VECT && !ff._VECTptr->empty()){
	  int d=int(ff._VECTptr->size())-1;
	  for (int i=0;i<n;++i){
	    gen g=vranm(d,0,contextptr);
	    res.push_back(symb_rootof(g,ff,contextptr));
	  }
	  return;
	}
      }
    }
    for (int i=0;i<n;++i)
      res.push_back(eval(f,eval_level(contextptr),contextptr));
  }

  vecteur vranm(int n,const gen & F,GIAC_CONTEXT){
    vecteur res;
    vranm(n,F,res,contextptr);
    return res;
  }

  matrice mranm(int n,int m,const gen & f,GIAC_CONTEXT){
    n=giacmax(1,n);
    m=giacmax(1,m);
    if (longlong(n)*m>LIST_SIZE_LIMIT/4)
      return vecteur(1,gendimerr(contextptr)); // setstabilityerr();
    matrice res;
    res.reserve(n);
    for (int i=0;i<n;++i){
      res.push_back(vecteur(0));
      vranm(m,f,*res[i]._VECTptr,contextptr);
    }
    return res;
  }

  gen _ranm(const gen & e,GIAC_CONTEXT){
    if ( e.type==_STRNG && e.subtype==-1) return  e;
    int n=0,m=0;
    switch (e.type){
    case _INT_:
      return vranm(e.val,zero,contextptr);
    case _DOUBLE_:
      return vranm(int(e._DOUBLE_val),zero,contextptr);
    case _VECT:
      if (e._VECTptr->size()==1)
	return _ranm(e._VECTptr->front(),contextptr);
      if (e._VECTptr->size()>=2){
	if (e._VECTptr->front().type==_INT_)
	  n=e._VECTptr->front().val;
	else {
	  if (e._VECTptr->front().type==_DOUBLE_)
	    n=int(e._VECTptr->front()._DOUBLE_val);
	  else
	    return gensizeerr(contextptr);
	}
	if ((*e._VECTptr)[1].type==_INT_)
	  m=(*e._VECTptr)[1].val;
	else {
	  if ((*e._VECTptr)[1].type==_DOUBLE_)
	    m=int((*e._VECTptr)[1]._DOUBLE_val);
	  else
	    return _randvector(e,contextptr); // try vector instead of gensizeerr(contextptr);
	}
	if (e._VECTptr->size()==3)
	  return gen(mranm(n,m,e._VECTptr->back(),contextptr),_MATRIX__VECT);
	if (e._VECTptr->size()==4){
	  gen loi=(*e._VECTptr)[2];
	  if (loi.type==_INT_ && e._VECTptr->back().type==_INT_){
	    // random integer vector in interval
	    int a=loi.val,b=e._VECTptr->back().val;
	    matrice M(n);
	    for (int j=0;j<n;++j){
	      gen res=vecteur(m);
	      for (int k=0;k<m;++k){
		(*res._VECTptr)[k]=(a+int((b-a+1)*(giac_rand(contextptr)/(rand_max2+1.0))));
	      }
	      M[j]=res;
	    }
	    return gen(M,_MATRIX__VECT);
	  }
	  if (loi.type==_FUNC){
	    if (loi==at_multinomial)
	      loi=symbolic(at_multinomial,e._VECTptr->back());
	    else
	      loi=loi(e._VECTptr->back(),contextptr);
	  }
	  else
	    loi=symb_of(loi,e._VECTptr->back());
	  return gen(mranm(n,m,loi,contextptr),_MATRIX__VECT);
	}
	if (e._VECTptr->size()>4){
	  gen loi=(*e._VECTptr)[2];
	  if (loi.type==_FUNC){
	    if (loi==at_multinomial)
	      loi=symbolic(at_multinomial,gen(vecteur(e._VECTptr->begin()+3,e._VECTptr->end()),_SEQ__VECT));
	    else
	      loi=loi(gen(vecteur(e._VECTptr->begin()+3,e._VECTptr->end()),_SEQ__VECT),contextptr);
	  }
	  else
	    loi=symb_of(loi,gen(vecteur(e._VECTptr->begin()+3,e._VECTptr->end()),_SEQ__VECT));
	  return gen(mranm(n,m,loi,contextptr),_MATRIX__VECT);
	}
	return gen(mranm(n,m,0,contextptr),_MATRIX__VECT);
      }
    default:
      return gensizeerr(contextptr);
    }
    return undef;
  }
  static const char _ranm_s []="ranm";
  static define_unary_function_eval (__ranm,&_ranm,_ranm_s);
  define_unary_function_ptr5( at_ranm ,alias_at_ranm,&__ranm,0,true);

  gen _randvector(const gen & e,GIAC_CONTEXT){
    if ( e.type==_STRNG && e.subtype==-1) return  e;
    int n=0;
    switch (e.type){
    case _INT_:
      return vranm(e.val,zero,contextptr);
    case _DOUBLE_:
      return vranm(int(e._DOUBLE_val),zero,contextptr);
    case _VECT:
      if (e._VECTptr->size()==1)
	return _randvector(e._VECTptr->front(),contextptr);
      if (e._VECTptr->size()>=2){
	if (e._VECTptr->front().type==_INT_)
	  n=e._VECTptr->front().val;
	else {
	  if (e._VECTptr->front().type==_DOUBLE_)
	    n=int(e._VECTptr->front()._DOUBLE_val);
	  else
	    return gensizeerr(contextptr);
	}
	gen loi=(*e._VECTptr)[1];
	gen res(vecteur(0));
	if (e._VECTptr->size()==3){
	  if (loi.type==_INT_ && e._VECTptr->back().type==_INT_){
	    // random integer vector in interval
	    int a=loi.val,b=e._VECTptr->back().val;
	    res._VECTptr->reserve(n);
	    for (int j=0;j<n;++j){
	      res._VECTptr->push_back(a+int((b-a+1)*(giac_rand(contextptr)/(rand_max2+1.0))));
	    }
	    return res;
	  } 
	  if (loi.type==_FUNC){
	    if (loi==at_multinomial)
	      loi=symbolic(at_multinomial,e._VECTptr->back());
	    else
	      loi=loi(e._VECTptr->back(),contextptr);
	  }
	  else
	    loi=symb_of(loi,e._VECTptr->back());
	}
	if (e._VECTptr->size()>3){
	  if (loi.type==_FUNC){
	    if (loi==at_multinomial)
	      loi=symbolic(at_multinomial,gen(vecteur(e._VECTptr->begin()+2,e._VECTptr->end()),_SEQ__VECT));
	    else 
	      loi=loi(gen(vecteur(e._VECTptr->begin()+2,e._VECTptr->end()),_SEQ__VECT),contextptr);
	  }
	  else
	    loi=symb_of(loi,gen(vecteur(e._VECTptr->begin()+2,e._VECTptr->end()),_SEQ__VECT));
	}
	vranm(n,loi,*res._VECTptr,contextptr);
	return res;
      }
    default:
      return gensizeerr(contextptr);
    }
    return undef;
  }
  static const char _randvector_s []="randvector";
  static define_unary_function_eval (__randvector,&_randvector,_randvector_s);
  define_unary_function_ptr5( at_randvector ,alias_at_randvector,&__randvector,0,true);

  static const char _ranv_s []="ranv";
  static define_unary_function_eval (__ranv,&_randvector,_ranv_s);
  define_unary_function_ptr5( at_ranv ,alias_at_ranv,&__ranv,0,true);


  bool minv(const matrice & a,matrice & res,bool convert_internal,int algorithm,GIAC_CONTEXT){
    if (debug_infolevel)
      CERR << CLOCK() << " matrix inv begin" << endl;
    matrice arref = a;
    add_identity(arref);
    if (debug_infolevel)
      CERR << CLOCK() << " identity added" << endl;
    int s=int(a.size());
    gen det;
    vecteur pivots;
    int ok=mrref(arref,res,pivots,det,0,s,0,2*s,
		 /* fullreduction */2*256,0,convert_internal,algorithm,0,
		 contextptr);
    if (!ok)
      return false;
    if (debug_infolevel)
      CERR << CLOCK() << " remove identity" << endl;
    if (ok!=2 && !remove_identity(res,contextptr))
      return false;
    if (debug_infolevel)
      CERR << CLOCK() << " end matrix inv" << endl;
    return true;
  }

  matrice minv(const matrice & a,GIAC_CONTEXT){
    matrice res;
    if (!minv(a,res,/*convert_internal */true,/* algorithm */ 1,contextptr))
      return vecteur(1,vecteur(1,gensizeerr(gettext("Not invertible"))));
    return res;
  }


  gen mdet(const matrice & a,GIAC_CONTEXT){
    if (!is_squarematrix(a))
      return gendimerr(contextptr);
    vecteur pivots;
    matrice res;
    gen determinant;
    int s=int(a.size());
    if (!mrref(a,res,pivots,determinant,0,s,0,s,
	  /* fullreduction */0,0,true,1/* guess algorithm */,1/* determinant */,
	       contextptr))
      return gendimerr(contextptr);
    return determinant;
  }

  gen _det(const gen & a_orig,GIAC_CONTEXT){
    if ( a_orig.type==_STRNG && a_orig.subtype==-1) return  a_orig;
    matrice a;
    bool convert_internal,minor_det,keep_pivot;
    int algorithm,last_col;
    if (!read_reduction_options(a_orig,a,convert_internal,algorithm,minor_det,keep_pivot,last_col))
      return gensizeerr(contextptr);
    if (keep_pivot)
      return gensizeerr(gettext("Option keep_pivot not applicable"));
    if (!is_squarematrix(a))
      *logptr(contextptr) << gettext("Warning: non-square matrix!") << endl;
    vecteur pivots;
    matrice res;
    gen determinant;
    int s=int(a.size());
    if (!mrref(a,res,pivots,determinant,0,s,0,s,
	  /* fullreduction */0,0,convert_internal,algorithm,1/* det */,
	       contextptr))
      return gendimerr(contextptr);
    return determinant;
  }
  static const char _det_s []="det";
  static define_unary_function_eval (__det,&_det,_det_s);
  define_unary_function_ptr5( at_det ,alias_at_det,&__det,0,true);

  // Find minimal poly by trying with 3 random vectors
  bool probabilistic_pmin(const matrice & m,vecteur & w,bool check,GIAC_CONTEXT){
    int n=int(m.size());
    modpoly p;
    for (int i=0;i<3;++i){
      vecteur v(vranm(n,0,0));
      // /* Old algorithm
      matrice temp(1,v);
      for (int j=0;j<n;++j){
	v=multmatvecteur(m,v);
	temp.push_back(v);
      }
      temp=mtran(temp);
      temp=mker(temp,contextptr);
      if (temp.empty() || is_undef(temp))
	return false; // setsizeerr();
      w=-*temp.front()._VECTptr;
      vreverse(w.begin(),w.end());
      w=trim(w,0);
      // */
      /*
      // New algorithm using A^(2n-1)v and Pade
      vecteur temp(1,v[0]);
      for (int j=1;j<2*n;++j){
	v=multmatvecteur(m,v);
	temp.push_back(v[0]);
      }
      w=reverse_rsolve(temp,false);
      // End new algorithù
      */
      if (signed(w.size())!=n+1 && !p.empty())
	w=lcm(w,p,0);
      p=w;
      if (signed(w.size())==n+1){
	w=w/w.front();
	return true;
      }
    }
    if (!check)
      return false;
    gen res=horner(w,m);
    return is_zero(res,contextptr);
  }

  // v1=v1+c2*v2 smod modulo
  void modlinear_combination(vector<int> & v1,int c2,
			     const vector<int> & v2,int modulo,int cstart,int cend,bool pseudo){
    if (c2){
      vector<int>::iterator it1=v1.begin()+cstart,it1end=v1.end(),it1_;
      if (cend && cend>=cstart && cend<it1end-v1.begin())
	it1end=v1.begin()+cend;
      vector<int>::const_iterator it2=v2.begin()+cstart;
      for (;it1!=it1end;++it1,++it2){
	*it1=( (*it1) + longlong(c2)*(*it2)) % modulo ; // replace smod
      }
    }
  }

  // Reduction to Hessenberg form, see e.g. Cohen algorithm 2.2.9
  // (with C array indices)
  // integer modulo case
  void mhessenberg(vector< vector<int> > & H,vector< vector<int> > & P,int modulo,bool compute_P){
    int t,u,n=int(H.size());
    vecteur vtemp;
    for (int m=0;m<n-2;++m){
      if (debug_infolevel>=5)
	CERR << "// hessenberg reduction line " << m << endl;
      // check for a non zero coeff in the column m below ligne m+1
      int i=m+1;
      for (;i<n;++i){
	t=H[i][m];
	if (t)
	  break;
      }
      if (i==n) //not found
	continue;
      t=invmod(t,modulo);
      // permutation of lines m+1 and i and columns m+1 and i
      if (i>m+1){
	H[i].swap(H[m+1]);
	if (compute_P)
	  P[i].swap(P[m+1]);
	for (int j=0;j<n;++j){
	  swapint(H[j][i],H[j][m+1]);
	  // tmp=H[j][i]; H[j][i]=H[j][m+1]; H[j][m+1]=tmp;
	}
      }
      // now coeff at line m+1 column m is H[m+1][m]=t!=0
      // creation of zeros in column m+1, lines i=m+2 and below
      vector<int> & Hmp1=H[m+1];
      for (i=m+2;i<n;++i){
	// line operation
	vector<int> & Hi=H[i];
	u=((longlong) t*Hi[m])%modulo;
	if (!u){ 
	  //CERR << "zero " << m << " " << i << endl;
	  continue;
	}
	if (debug_infolevel>3)
	  CERR << "// i=" << i << " " << u <<endl;
	modlinear_combination(Hi,-u,Hmp1,modulo,m,0,false); // H[i]=H[i]-u*H[m+1]; COULD START at m
	// column operation
	for (int j=0;j<n;++j){
	  vector<int> & Hj=H[j];
#ifdef _I386_
	  mod(Hj[m+1],u,Hj[i],modulo);
#else
	  int * ptr=&Hj[m+1];
	  *ptr=(*ptr+longlong(u)*Hj[i])%modulo;
#endif
	}
	if (compute_P)
	  modlinear_combination(P[i],-u,P[m+1],modulo,0,0,false); // P[i]=P[i]-u*P[m+1];
      }
    }
  }

  // Hessenberg reduction, P is not orthogonal
  // P^(-1)*H*P = original
  void hessenberg(std_matrix<gen> & H,std_matrix<gen> & P,GIAC_CONTEXT){
    int n=int(H.size());
    gen t,tabs,u,tmp;
    vecteur vtemp;
    for (int m=0;m<n-2;++m){
      if (debug_infolevel>=5)
	CERR << "// hessenberg reduction line " << m << endl;
      // check for a non zero coeff in the column m below ligne m+1
      int i=m+1;
      gen pivot=0;
      int pivotline=0;
      for (;i<n;++i){
	t=H[i][m];
	tabs=abs(t,contextptr);
	if (is_strictly_greater(tabs,pivot,contextptr)){
	  pivotline=i;
	  pivot=tabs;
	}
      }
      if (is_zero(pivot)) //not found
	continue;
      i=pivotline;
      t=H[i][m];
      // permutation of lines m+1 and i and columns m+1 and i
      /*
      if (i>m+1){
	for (int j=0;j<n;++j){
	  tmp=H[i][j];
	  H[i][j]=H[m+1][j];
	  H[m+1][j]=tmp;
	}
	for (int j=0;j<n;++j){
	  tmp=H[j][i];
	  H[j][i]=H[j][m+1];
	  H[j][m+1]=tmp;
	}
      }
      */
      if (i>m+1){
	swap(H[i],H[m+1]);
	swap(P[i],P[m+1]);
	for (int j=0;j<n;++j){
	  vecteur & Hj=H[j];
	  swapgen(Hj[i],Hj[m+1]);
	}
      }
      // now coeff at line m+1 column m is H[m+1][m]=t!=0
      // creation of zeros in column m+1, lines i=m+2 and below
      for (i=m+2;i<n;++i){
	// line operation
	u=rdiv(H[i][m],t,contextptr);
	if (debug_infolevel>2)
	  CERR << "// i=" << i << " " << u <<endl;
	linear_combination(plus_one,H[i],-u,H[m+1],plus_one,1,vtemp,1e-12,0); // H[i]=H[i]-u*H[m+1];
	swap(H[i],vtemp);
	linear_combination(plus_one,P[i],-u,P[m+1],plus_one,1,vtemp,1e-12,0); // H[i]=H[i]-u*H[m+1];
	swap(P[i],vtemp);
	// column operation
	for (int j=0;j<n;++j){
	  vecteur & Hj=H[j];
	  tmp=Hj[m+1]+u*Hj[i];
	  Hj[m+1]=tmp;
	}
      }
    }
  }

  double hypot(double x,double y){
    return std::sqrt(x*x+y*y);
  }

  // IMPROVE: don't do operations with 0
  void qr_rq(std_matrix<gen> & H,std_matrix<gen> & P,const gen & shift,int n,int & nitershift0,GIAC_CONTEXT){
    gen t,tn,tc,tabs,uabs,t2,u,un,uc,tmp1,tmp2,norme;
    int n_orig=int(H.size());
    vecteur v1,v2,TN(n_orig),UN(n_orig);
    if (is_zero(shift)){
      nitershift0++;
    }
    else{
      for (int i=0;i<n_orig;++i){
	H[i][i] -= shift;
      }
    }
    // H -> H-shift*identity
    for (int m=0;m<n-1;++m){
      // reduce coeff line m+1, col m
      t=H[m][m];
      // if (is_zero(t)) *logptr(contextptr) << "qr iteration: 0 on diagonal");
      int i=m+1;
      u=H[i][m];
      // now coeff at line m+1 column m is H[m+1][m]=t
      // creation of zeros in column m+1, lines i=m+2 and below
      // normalization of t and u such that t is real positive
      tabs=abs(t,contextptr);
      uabs=abs(u,contextptr);
      if (is_strictly_greater(uabs/tabs,1,contextptr))
	t2=uabs/u;
      else
	t2=tabs/t;
      t=t*t2; 
      u=u*t2;
      // compute unitary matrix coefficients
      tc=conj(t,contextptr);
      uc=conj(u,contextptr);
      norme=sqrt(re(u*uc+t*tc,contextptr),contextptr);
      un=u/norme; tn=t/norme; uc=conj(un,contextptr);	tc=conj(tn,contextptr); 
      // line operation
      // H[m]=uc*H[i]+tc*H[m] and H[i]=tn*H[i]-un*H[m];
      linear_combination(uc,H[i],tc,H[m],plus_one,1,v1,1e-12,0); 
      linear_combination(tn,H[i],-un,H[m],plus_one,1,v2,1e-12,0); 
      swap(H[m],v1);
      swap(H[i],v2);
      linear_combination(uc,P[i],tc,P[m],plus_one,1,v1,1e-12,0); 
      linear_combination(tn,P[i],-un,P[m],plus_one,1,v2,1e-12,0); 
      swap(P[m],v1);
      swap(P[i],v2);
      TN[m]=tn;
      UN[m]=un;
    } // end for m
    for (int m=0;m<n-1;++m){
      tn=TN[m];
      un=UN[m];
      tc=conj(tn,contextptr);
      uc=conj(un,contextptr);
      // column operation
      // int nmax=n_orig>m+3?m+3:n_orig;
      for (int j=0;j<n_orig;++j){
	vecteur & Hj=H[j];
	gen & Hjm=Hj[m];
	gen & Hjm1=Hj[m+1];
	tmp1=tn*Hjm+un*Hjm1;
	tmp2=-uc*Hjm+tc*Hjm1;
	Hjm=tmp1;
	Hjm1=tmp2;
      }
    }
    if (!is_zero(shift)){
      for (int i=0;i<n_orig;++i){
	H[i][i] += shift;
      }
    }
  }

  void re(std_matrix<gen> & H,int n,GIAC_CONTEXT){
    for (int i=0;i<n;i++){
      for (int j=0;j<n;j++){
	H[i][j]=re(H[i][j],contextptr);
      }
    }
  }

  void hessenberg_ortho(std_matrix<gen> & H,std_matrix<gen> & P,GIAC_CONTEXT){
    hessenberg_ortho(H,P,-1,-1,true,0,0.0,contextptr);
  }

  // v=(c1*v1+c2*v2), begin at cstart
  void linear_combination(const gen & c1,const vecteur & v1,const gen & c2,const vecteur & v2,vecteur & v,int cstart,double eps){
    eps=0;
    if (cstart<0)
      cstart=0;
    const_iterateur it1=v1.begin()+cstart,it1end=v1.end(),it2=v2.begin()+cstart;
    iterateur jt1=v.begin()+cstart;
#ifdef DEBUG_SUPPORT
    if (it1end-it1!=v2.end()-it2)
      setdimerr();
#endif
    if (it2==jt1){
      linear_combination(c2,v2,c1,v1,v,cstart,eps);
      return;
    }
    if (it1==jt1){
      for (;jt1!=it1end;++jt1,++it2){
	*jt1=trim(c1*(*jt1)+c2*(*it2),c1,eps);
      }
      return;
    }
    if (int(v.size())==it1end-it1){
      jt1=v.begin();
      for (int i=0;i<cstart;++i,++jt1)
	*jt1=0;
      for (;it1!=it1end;++it1,++it2,++jt1)
	*jt1=trim(c1*(*it1)+c2*(*it2),c1,eps);
      return;
    }
    v.clear();
    v.reserve(it1end-it1);
    for (int i=0;i<cstart;++i)
      v.push_back(0);
    for (;it1!=it1end;++it1,++it2)
      v.push_back(trim(c1*(*it1)+c2*(*it2),c1,eps));
  }

  matrice & H0(){
    static matrice * ptr=0;
    if (!ptr)
      ptr=new matrice;
    return *ptr;
  }

  void dbg_schur(const std_matrix<gen> & H,const std_matrix<gen> & P){
    matrice Hg,Pg;
    std_matrix_gen2matrice(H,Hg);
    std_matrix_gen2matrice(P,Pg);
    matrice res=mmult(mtran(Pg),Hg);
    res=mmult(res,Pg);
    gen t=subvecteur(res,H0());
    gen t1=_max(_abs(t,context0),context0);
    if (t1._DOUBLE_val>1e-5)
      CERR << "Error" << endl;
  }


  // Hessenberg reduction, P is orthogonal and should be initialized to identity
  // trn(P)*H*P=original
  // already_zero is either <=0 or an integer such that H[i][j]==0 if i>j+already_zero
  // (already_zero==1 if H is hessenberg, ==3 for Francis algorithm)
  void hessenberg_ortho(std_matrix<gen> & H,std_matrix<gen> & P,int firstrow,int n,bool compute_P,int already_zero,double eps,GIAC_CONTEXT){
    double eps_save(epsilon(contextptr));
    epsilon(eps,contextptr);
    int nH=int(H.size());
    if (n<0 || n>nH) 
      n=nH;
    if (firstrow<0 || firstrow>n)
      firstrow=0;
    gen t,tn,tc,tabs,u,un,uc,tmp1,tmp2,norme;
    vecteur v1(nH),v2(nH),TN(n,1),UN(n);
    for (int m=firstrow;m<n-2;++m){
      if (debug_infolevel>=5)
	CERR << "// hessenberg reduction line " << m << endl;
      // check for a non zero coeff in the column m below ligne m+1
      int i=m+1;
      gen pivot=0;
      int pivotline=0;
      int nend=n;
      if (already_zero && i+already_zero<n)
	nend=i+already_zero;
      for (;i<nend;++i){
	t=H[i][m];
	tabs=abs(t,contextptr);
	if (is_strictly_greater(tabs,pivot,contextptr)){
	  pivotline=i;
	  pivot=tabs;
	}
      }
      if (is_zero(pivot,contextptr)) //not found
	continue;
      i=pivotline;
      // exchange line and columns
      if (i>m+1){
	swap(H[i],H[m+1]);
	if (compute_P)
	  swap(P[i],P[m+1]);
	for (int j=0;j<n;++j){
	  vecteur & Hj=H[j];
	  swapgen(Hj[i],Hj[m+1]);
	}
      }
      // now coeff at line m+1 column m is H[m+1][m]=t!=0
      // creation of zeros in column m+1, lines i=m+2 and below
      // if (firstrow==100) dbg_schur(H,P);
      int nprime=n;
      for (i=m+2;i<nend;++i){
	// line operation
	t=H[m+1][m];
	u=H[i][m];
	// CERR << t << " " << u << endl;
	uc=conj(u,contextptr);
	tc=conj(t,contextptr);
	norme=sqrt(u*uc+t*tc,contextptr);
	un=u/norme; tn=t/norme; uc=conj(un,contextptr);	tc=conj(tn,contextptr); 
	if (is_zero(un,contextptr)){
	  UN[i]=0;
	  continue;
	}
	if (debug_infolevel>=3)
	  CERR << "// i=" << i << " " << u <<endl;
	// H[m+1]=tc*H[m+1]+uc*H[i] and H[i]=tn*H[i]-un*H[m+1];
	linear_combination(uc,H[i],tc,H[m+1],v1,0,0.0); 
	linear_combination(tn,H[i],-un,H[m+1],v2,0,0.0); 
	swap(H[m+1],v1);
	swap(H[i],v2);
	if (compute_P){
	  linear_combination(uc,P[i],tc,P[m+1],v1,0,0.0); 
	  linear_combination(tn,P[i],-un,P[m+1],v2,0,0.0); 
	  swap(P[m+1],v1);
	  swap(P[i],v2);
	}
	TN[i]=tn;
	UN[i]=un;
      }
      for (i=m+2;i<nprime;++i){ 
	un=UN[i];
	if (is_zero(un,contextptr))
	  continue;
	tn=TN[i];
	tc=conj(tn,contextptr);
	uc=conj(un,contextptr);
	// column operation
	for (int j=0;j<nH;++j){
	  vecteur & Hj=H[j];
	  tmp1=tn*Hj[m+1]+un*Hj[i];
	  tmp2=-uc*Hj[m+1]+tc*Hj[i];
	  Hj[m+1]=tmp1;
	  Hj[i]=tmp2;
	}
      }
      // if (firstrow==100) dbg_schur(H,P);
    }
    // make 0 below subdiagonal (i<nH all matrix, i<n only relevant lines/column)
    for (int i=2;i<n;i++){
      iterateur it=H[i].begin(),itend=it+i-1; // or min(i-1,n);
      for (;it!=itend;++it){
	if (debug_infolevel>2 && abs(*it,contextptr)>1e-10)
	  CERR << "Precision " << i << " " << *it << endl;
	*it=0;
      }
    }
    epsilon(eps_save,contextptr);
  }

  void tri_linear_combination(const gen & c1,const vecteur & x1,const gen & c2,const vecteur & x2,const gen & c3,const vecteur & x3,vecteur & y){
    const_iterateur it1=x1.begin(),it2=x2.begin(),it3=x3.begin(),it3end=x3.end();
    iterateur jt=y.begin();
    for (;it3!=it3end;++jt,++it1,++it2,++it3){
      *jt=c1*(*it1)+c2*(*it2)+c3*(*it3);
    }
  }

  void francis_schur_iterate(std_matrix<gen> & H,double eps,const gen & l1,int n_orig,int n1,int n2,std_matrix<gen> & P,bool compute_P,GIAC_CONTEXT){
    // compute (H-l1) on n1-th basis vector
    gen x=H[n1][n1]-l1,y=H[n1+1][n1];
    // make x real
    gen xr,xi,yr,yi;
    reim(x,xr,xi,contextptr);
    reim(y,yr,yi,contextptr);
    x = sqrt(xr*xr+xi*xi,contextptr);
    if (x==0) return;
    // gen xy = gen(xr/x,-xi/x); y=y*xy;
    y = gen((yr*xr+yi*xi)/x,(yi*xr-yr*xi)/x); 
    reim(y,yr,yi,contextptr);
    gen xy=sqrt(x*x+yr*yr+yi*yi,contextptr);
    // normalize eigenvector
    x = x/xy; y = y/xy;	
    // compute reflection matrix such that Q*[1,0]=[x,y]
    // hence column 1 is [x,y] and column2 is [conj(y),-x]
    // apply Q on H and P: line operations on H and P
    gen c11=x, c12=conj(y,contextptr),
      c21=y, c22=-x,tmp1,tmp2;
    vecteur v1(n_orig),v2(n_orig);
    linear_combination(c11,H[n1],c12,H[n1+1],v1,0,0.0);
    linear_combination(c21,H[n1],c22,H[n1+1],v2,0,0.0);
    swap(H[n1],v1);
    swap(H[n1+1],v2);
    if (compute_P){
      linear_combination(c11,P[n1],c12,P[n1+1],v1,0,0.0);
      linear_combination(c21,P[n1],c22,P[n1+1],v2,0,0.0);
      swap(P[n1],v1);
      swap(P[n1+1],v2);
    }
    // now columns operations on H (not on P)
    for (int j=0;j<n_orig;++j){
      vecteur & Hj=H[j];
      gen & Hjm1=Hj[n1];
      gen & Hjm2=Hj[n1+1];
      tmp1=Hjm1*c11+Hjm2*c21;
      tmp2=Hjm1*c12+Hjm2*c22;
      Hjm1=tmp1;
      Hjm2=tmp2;
    }
  }

  void francis_schur_iterate_real(std_matrix<gen> & H,int n_orig,int n1,int n2,std_matrix<gen> & P,bool compute_P,GIAC_CONTEXT){
    vecteur v1(n_orig),v2(n_orig),v3(n_orig);
    gen tmp1,tmp2,tmp3;
    gen s,p; // s=l1+l2, p=l1*l2
    s=H[n2-2][n2-2]+H[n2-1][n2-1];
    p=H[n2-2][n2-2]*H[n2-1][n2-1]-H[n2-1][n2-2]*H[n2-2][n2-1];
    // compute (H-l2)(H-l1)=(H-s)*H+p on n1-th basis vector (if n1==0, on [1,0,...,0])
    gen ha=H[n1][n1],hb=H[n1][n1+1],
      hd=H[n1+1][n1],he=H[n1+1][n1+1],
      hh=H[n1+2][n1+1];
    gen x=hb*hd+ha*(ha-s)+p,y=hd*(he-s+ha),z=hd*hh;
    // normalize, substract [1,0,0] and normalize again
    gen xyz=sqrt(x*conj(x,contextptr)+y*conj(y,contextptr)+z*conj(z,contextptr),contextptr);
    // if x/xyz is near 1, improve precision:
    // x/xyz-1 = ((x/xyz)^2-1)/(x/xyz+1)=-((y/xyz)^2+(z/xyz)^2)/(x/xyz+1)
    x=x/xyz; y=y/xyz; z=z/xyz;
    if (fabs(evalf_double(re(x,contextptr)-1,1,contextptr)._DOUBLE_val)<0.5)
      x=-(y*y+z*z)/(x+1);
    else
      x-=1;
    xyz=sqrt(x*conj(x,contextptr)+y*conj(y,contextptr)+z*conj(z,contextptr),contextptr);
    x=x/xyz; y=y/xyz; z=z/xyz;
    // compute reflection matrix let n=[[x],[y],[z]] trn(n)=conj([[x,y,z]])
    // Q=idn(3)-2*n*trn(n);
    // i.e. [[ 1-2x*conj(x), -2x*conj(y),   -2x*conj(z)  ],
    //       [ -2*y*conj(x), 1-2*y*conj(y), -2*y*conj(z) ],
    //       [ -2*z*conj(x), -2*z*conj(y),  1-2*z*conj(z)]]
    // apply Q on H and P: line operations on H and P
    gen c11=1-2*x*conj(x,contextptr),c12=-2*x*conj(y,contextptr),c13=-2*x*conj(z,contextptr);
    gen c21=-2*y*conj(x,contextptr),c22=1-2*y*conj(y,contextptr),c23=-2*y*conj(z,contextptr);
    gen c31=-2*z*conj(x,contextptr),c32=-2*z*conj(y,contextptr),c33=1-2*z*conj(z,contextptr);
    // CERR << "[[" << c11 <<"," << c12 << "," << c13 << "],[" <<  c21 <<"," << c22 << "," << c23 << "],[" << c31 <<"," << c32 << "," << c33 << "]]" << endl;
    tri_linear_combination(c11,H[n1],c12,H[n1+1],c13,H[n1+2],v1);
    tri_linear_combination(c21,H[n1],c22,H[n1+1],c23,H[n1+2],v2);
    tri_linear_combination(c31,H[n1],c32,H[n1+1],c33,H[n1+2],v3);
    swap(H[n1],v1);
    swap(H[n1+1],v2);
    swap(H[n1+2],v3);
#ifdef HAVE_LIBMPFR
    mpfr_t tmpf1,tmpf2; mpfr_init(tmpf1); mpfr_init(tmpf2);
#endif
    // now columns operations on H (not on P)
    for (int j=0;j<n_orig;++j){
      vecteur & Hj=H[j];
      gen & Hjm1=Hj[n1];
      gen & Hjm2=Hj[n1+1];
      gen & Hjm3=Hj[n1+2];
#ifdef HAVE_LIBMPFR
      tmp1=tri_linear_combination(Hjm1,c11,Hjm2,c21,Hjm3,c31,tmpf1,tmpf2);
      tmp2=tri_linear_combination(Hjm1,c12,Hjm2,c22,Hjm3,c32,tmpf1,tmpf2);
      tmp3=tri_linear_combination(Hjm1,c13,Hjm2,c23,Hjm3,c33,tmpf1,tmpf2);
#else
      tmp1=Hjm1*c11+Hjm2*c21+Hjm3*c31;
      tmp2=Hjm1*c12+Hjm2*c22+Hjm3*c32;
      tmp3=Hjm1*c13+Hjm2*c23+Hjm3*c33;
#endif
      Hjm1=tmp1;
      Hjm2=tmp2;
      Hjm3=tmp3;
    }
#ifdef HAVE_LIBMPFR
    mpfr_clear(tmpf1); mpfr_clear(tmpf2);
#endif
    // CERR << H << endl;
    if (compute_P){
      tri_linear_combination(c11,P[n1],c12,P[n1+1],c13,P[n1+2],v1);
      tri_linear_combination(c21,P[n1],c22,P[n1+1],c23,P[n1+2],v2);
      tri_linear_combination(c31,P[n1],c32,P[n1+1],c33,P[n1+2],v3);
      swap(P[n1],v1);
      swap(P[n1+1],v2);
      swap(P[n1+2],v3);
    }
  }
  
  // Francis algorithm on submatrix rows and columns n1..n2-1
  // Invariant: trn(P)*H*P=orig matrix
  bool francis_schur(std_matrix<gen> & H,int n1,int n2,std_matrix<gen> & P,int maxiter,double eps,bool is_hessenberg,bool complex_schur,bool compute_P,bool no_lapack,GIAC_CONTEXT){
    // complex_schur=false;
    vecteur eigenv;
    int n_orig=int(H.size());//,nitershift0=0;
    if (!is_hessenberg){
      // std_matrix_gen2matrice(H,H0());
      hessenberg_ortho(H,P,0,n_orig,compute_P,0,0.0,contextptr); // insure Hessenberg form (on the whole matrix)
    }
    if (n2-n1<=1)
      return true; // nothing to do
    if (n2-n1==2){ // 2x2 submatrix, we know how to diagonalize
      gen l1,l2;
      //cout << H << " " << P << " " << maxiter << " " << eps << endl; return true;
      if (eigenval2(H,n2,l1,l2,contextptr) || complex_schur){
	// choose l1 or l2 depending on H[n1][n1]-l1, H[n1][n1+1]
	if (is_greater(abs(H[n1][n1]-l1,contextptr),abs(H[n1][n1]-l2,contextptr),contextptr))
	  francis_schur_iterate(H,eps,l1,n_orig,n1,n2,P,compute_P,contextptr);
	else
	  francis_schur_iterate(H,eps,l2,n_orig,n1,n2,P,compute_P,contextptr);
      }
      return true;
    }
    for (int niter=0;n2-n1>2 && niter<maxiter;niter++){
      // make 0 below subdiagonal
      for (int i=2;i<n_orig;i++){
	vecteur & Hi=H[i];
	for (int j=0;j<i-1;j++){
	  Hi[j]=0;
	}
      }
      if (debug_infolevel>=2)
	CERR << CLOCK() << " qr iteration number " << niter << " " << endl;
      if (debug_infolevel>=5)
	H.dbgprint();
      // check if one subdiagonal element is sufficiently small, if so 
      // we can increase n1 or decrease n2 or split
      for (int i=n1;i<n2-1;++i){
	gen ratio=abs(H[i+1][i]/H[i][i],contextptr);
	ratio=evalf_double(ratio,1,contextptr);
	if (ratio.type==_DOUBLE_ && fabs(ratio._DOUBLE_val)<eps){
	  if (debug_infolevel>2)
	    CERR << "Francis split " << n1 << " " << i+1 << " " << n2 << endl;
	  // submatrices n1..i and i+1..n2-1
	  if (!francis_schur(H,n1,i+1,P,maxiter,eps,true,complex_schur,compute_P,true,contextptr))
	    return false;
	  return francis_schur(H,i+1,n2,P,maxiter,eps,true,complex_schur,compute_P,true,contextptr);
	}
      }
      // now H is proper hessenberg (indices n1 to n2-1)
      // find eigenvalues l1 and l2 of last 2x2 matrix, they will be taken as shfits
      // FIXME for complex matrices, direct reflection with eigenvector of l1 or l2
      if (complex_schur){
	gen l1,l2;
	l1=H[n2-1][n2-1];
	if (n2-n1>=2){
	  // take the closest eigenvalue of the last 2*2 block 
	  eigenval2(H,n2,l1,l2,contextptr);
	  if (is_greater(abs(l1-H[n2-1][n2-1],contextptr),abs(l2-H[n2-1][n2-1],contextptr),contextptr))
	    l1=l2;
	}
	//  FIXME? if H[n1][n1]-l1 is almost zero and H[n1][n1+1] also -> precision problem
	francis_schur_iterate(H,eps,l1,n_orig,n1,n2,P,compute_P,contextptr);
      }
      else
	francis_schur_iterate_real(H,n_orig,n1,n2,P,compute_P,contextptr);
      if (n1==100)
	dbg_schur(H,P);
      // CERR << H << endl;
      // chase the bulge: Hessenberg reduction on 2 subdiagonals
      hessenberg_ortho(H,P,n1,n2,compute_P,3,0.0,contextptr); // <- improve
    } // end for loop on niter
    return false;
  }

  // trn(P)*H*P=orig matrix
  void hessenberg_schur(std_matrix<gen> & H,std_matrix<gen> & P,int maxiter,double eps,GIAC_CONTEXT){
    int n_orig=int(H.size()),n=n_orig,nitershift0=0;
    bool real=true,is_double=false; 
    for (int i=0;real && i<n;i++){
      vecteur &Hi=H[i];
      for (int j=0;j<n;j++){
	gen Hij=Hi[j];
	if (!is_zero(im(Hij,contextptr))){
	  real=false;
	}
      }
    }
    if (francis_schur(H,0,n_orig,P,maxiter,std::sqrt(double(n_orig))*eps,false,!real,true,true,contextptr)){
      return ;
    }
    hessenberg_ortho(H,P,contextptr); // insure
    // make 0 below subdiagonal
    for (int i=2;i<n_orig;i++){
      for (int j=0;j<i-1;j++){
	H[i][j]=0;
      }
    }
    gen shift=0,ratio,oldratio=0,maxi,absmaxi,tmp,abstmp;
    vecteur SHIFT;
    for (int niter=0;n>1 && niter<maxiter;niter++){
      if (debug_infolevel>=2)
	CERR << CLOCK() << " qr iteration number " << niter << endl;
      shift=0;
      gen test=abs(H[n-1][n-2],contextptr);
      ratio=test/abs(H[n-1][n-1],contextptr);
      bool Small=is_strictly_greater(0.01,ratio,contextptr);
      if (Small)
	shift=H[n-1][n-1];
      else {
	if (n==2 || is_strictly_greater(0.01,(ratio=abs(H[n-2][n-3]/H[n-2][n-2],contextptr)),contextptr)){
	  // define shift according to the smallest eigenvalues 
	  // of the last 2x2 submatrix bloc
	  gen a=H[n-2][n-2],b=H[n-2][n-1],c=H[n-1][n-2],d=H[n-1][n-1];
	  gen delta=a*a-2*a*d+d*d+4*b*c;
	  if (real && n==2 && is_strictly_positive(-delta,0))
	    break;
	  delta=sqrt(delta,contextptr);
	  gen l1=(a+d+delta)/2,l2=(a+d-delta)/2;
	  if (is_strictly_greater(abs(l1,contextptr),abs(l2,contextptr),contextptr))
	    shift=l2;
	  else
	    shift=l1;
	}
	else {
	  if (niter>=maxiter/4)
	    shift=H[n-1][n-1]/2;
	}
      }
      oldratio=ratio;
      qr_rq(H,P,shift,n,nitershift0,contextptr);
      if (real && !is_zero(im(shift,contextptr)))
	SHIFT.push_back(shift);
      test=abs(H[n-1][n-2],contextptr);
      ratio=test/abs(H[n-1][n-1],contextptr);
      if (is_strictly_greater(gen(eps)/oldratio,1,contextptr) && is_strictly_greater(gen(eps)/ratio,1,contextptr)){
	// eigenvalue has been found
	niter=0;
	oldratio=0;
	n--;
	if (real && !SHIFT.empty()){
	  // int ni=SHIFT.size();
	  for(int i=0;i<(int)SHIFT.size();++i)
	    qr_rq(H,P,conj(SHIFT[i],contextptr),n,nitershift0,contextptr);
	  for (int i=0;i<n-1;i++){
	    vecteur & Pi=P[i];
	    maxi=Pi.front();
	    absmaxi=abs(maxi,contextptr);
	    for (int j=1;j<n-1;j++){
	      tmp=Pi[j];
	      abstmp=abs(tmp,contextptr);
	      if (abstmp>absmaxi){
		absmaxi=abstmp;
		maxi=tmp;
	      }
	    }
	    tmp=absmaxi/maxi;
	    multvecteur(tmp,Pi,Pi);
	    multvecteur(tmp,H[i],H[i]);
	    tmp=maxi/absmaxi;
	    for (int j=0;j<n_orig;j++){
	      gen & Hji= H[j][i];
	      Hji = tmp*Hji;
	    }
	  }
	  re(H,n-1,contextptr); re(P,n-1,contextptr);
	  n--;
	  SHIFT.clear();
	} // end if (real)
      } // end eigenvalue detected
    } // end loop on n for 0 subdiagonal elements
  }

  // Reduction to Hessenberg form, see e.g. Cohen algorithm 2.2.9
  // (with C array indices)
  // general case
  // if modulo==-1 Schur reduction up to precision eps and maxiterations maxiter
  // if modulo<-1, using orthogonal/unitary matrices
  bool mhessenberg(const matrice & M,matrice & h,matrice & p,int modulo,int maxiter,double eps,GIAC_CONTEXT){
    int n=int(M.size());
    if (!n || n!=mcols(M))
      return false; // setdimerr();
    bool modularize=!modulo && M[0][0].type==_MOD && (M[0][0]._MODptr+1)->type==_INT_;
    if (modularize)
      modulo=(M[0][0]._MODptr+1)->val;
    if (modulo>0){
      vector< vector<int> > H;
      if (!vecteur2vectvector_int(M,modulo,H))
	return false;
      vector< vector<int> > P;
      if (!vecteur2vectvector_int(midn(n),modulo,P))
	return false;
      mhessenberg(H,P,modulo,true);
      vectvector_int2vecteur(H,h);
      vectvector_int2vecteur(P,p);
      if (modularize){
	h=*makemod(h,modulo)._VECTptr;
	p=*makemod(p,modulo)._VECTptr;
      }
      return true;
    }
    std_matrix<gen> H,P(n,vecteur(n));
    for (int i=0;i<n;++i)
      P[i][i]=1;
    if (modulo<0){
#ifdef HAVE_LIBMPFR
      matrice2std_matrix_gen(*evalf(gen(M),1,contextptr)._VECTptr,H);
#else
      matrice2std_matrix_gen(*evalf_double(gen(M),1,contextptr)._VECTptr,H);
#endif
    }
    else
      matrice2std_matrix_gen(M,H);
    if (modulo==-1)
      hessenberg_schur(H,P,maxiter,eps,contextptr);
    else {
      if (modulo<0)
	hessenberg_ortho(H,P,contextptr);
      else
	hessenberg(H,P,contextptr);
    }
    // store result
    std_matrix_gen2matrice_destroy(H,h);
    std_matrix_gen2matrice_destroy(P,p);
    return true;
  }
  gen _hessenberg(const gen & g0,GIAC_CONTEXT){
    if ( g0.type==_STRNG && g0.subtype==-1) return  g0;
    gen g(g0);
    int modulo=0;
    double eps=epsilon(contextptr);
    int maxiter=500;
    if (g.type==_VECT && g._VECTptr->size()>=2 && g.subtype==_SEQ__VECT){
      vecteur & v = *g._VECTptr;
      gen v1=v[1];
      if (v1.type==_INT_)
	modulo=v1.val;
      else {
	v1=evalf_double(v1,1,contextptr);
	if (v1.type==_DOUBLE_){
	  modulo=-1;
	  eps=v1._DOUBLE_val;
	  if (v.size()>2 && v[2].type==_INT_)
	    maxiter=v[2].val;
	}
      }
      g=v.front();
    }
    if (!is_squarematrix(g))
      return symbolic(at_hessenberg,g);
    matrice m(*g._VECTptr),h,p;
    if (!mhessenberg(m,h,p,modulo,maxiter,eps,contextptr))
      return gensizeerr(contextptr);
    if (modulo<0)
      return makesequence(_trn(p,contextptr),h); // p,h such that p*h*p^-1=orig
    else
      return makesequence(inv(p,contextptr),h); // p,h such that p*h*p^-1=orig
  }
  static const char _hessenberg_s []="hessenberg";
  static define_unary_function_eval (__hessenberg,&_hessenberg,_hessenberg_s);
  define_unary_function_ptr5( at_hessenberg ,alias_at_hessenberg,&__hessenberg,0,true);

  int trace(const vector< vector<int> > & N,int modulo){
    longlong res=0;
    int n=int(N.size());
    for (int i=0;i<n;++i){
      res += N[i][i];
    }
    return res%modulo;
  }

  // Danilevsky algorithm
  // kind of row reduction to companion matrix
  // returns charpoly or minpoly 
  void mod_pcar(std_matrix<gen> & N,vecteur & res,bool compute_pmin){
    int n=int(N.size());
    if (n==1){
      res.resize(2);
      res[0]=1;
      res[1]=-N[0][0];
    }
    vecteur v(n),w(n);
    for (int k=0;k<n-1;++k){
      // search "pivot" on line k
      for (int j=k+1;j<n;++j){
	if (!is_zero(N[k][j])){
	  // swap columns and lines k+1 and j
	  if (j>k+1){
	    for (int i=k;i<n;++i)
	      swapgen(N[i][k+1],N[i][j]);
	    N[k+1].swap(N[j]);
	  }
	  break;
	}
      }
      v=N[k];
      gen akk1=v[k+1];
      if (is_zero(akk1)){
	// degenerate case, split N in two parts
	vecteur part1(k+2),part2;
	part1[0]=1;
	for (int i=0;i<=k;++i){
	  part1[i+1]=-N[k][k-i];
	}
	std_matrix<gen> N2(n-1-k);
	for (int i=k+1;i<n;++i)
	  N2[i-1-k]=vecteur(N[i].begin()+k+1,N[i].end());
	mod_pcar(N2,part2,compute_pmin);
	if (compute_pmin && part1==part2){
	  res.swap(part1);
	  return;
	}
	if (compute_pmin)
	  res=lcm(part1,part2,0);
	else
	  res=operator_times(part1,part2,0);
	return;
      }
      // multiply right by identity with line k+1 replaced by 
      // -N[k]/a_{k,k+1} except on diagonal 1/a_{k,k+1} 
      // this will replace line k by 0...010...0 (1 at column k+1)
      gen invakk1=inv(akk1,context0);
      for (int i=0;i<n;++i)
	w[i]=(-invakk1*v[i]);
      w[k+1]=invakk1;
      // column operations
      for (int l=k;l<n;++l){
	vecteur & Nl=N[l];
	gen Nlk1=Nl[k+1];
	for (int j=0;j<=k;++j){
	  gen & Nlj=Nl[j];
	  Nlj=Nlj+w[j]*Nlk1;
	}
	Nl[k+1]=invakk1*Nlk1;
	for (int j=k+2;j<n;++j){
	  gen & Nlj=Nl[j];
	  Nlj=Nlj+w[j]*Nlk1;
	}
      }
      // multiply left by identity with line k+1 replaced by original N[k]
      // line operations L_{k+1}=sum a_{k,i} L_i
      for (int j=0;j<n;++j){
	gen coeff(0);
	if (j>=1 && j<=k+1)
	  coeff=v[j-1];
	for (int i=k+1;i<n;++i){
	  coeff += v[i]*N[i][j];
	}
	N[k+1][j]=coeff;
      }
    }
    // get charpoly
    res.resize(n+1);
    res[0]=1;
    for (int i=0;i<n;++i)
      res[1+i]=-N[n-1][n-1-i];
  }

  bool mod_pcar(const matrice & A,vector< vector<int> > & N,int modulo,bool & krylov,vector<int> & res,GIAC_CONTEXT,bool compute_pmin){
    return false;
  }


  vecteur mpcar_int(const matrice & A,bool krylov,GIAC_CONTEXT,bool compute_pmin){
    int n=int(A.size());
    gen B=evalf_double(linfnorm(A,contextptr),0,contextptr);
    double Bd=B._DOUBLE_val;
    if (!Bd){
      modpoly charpol(n+1);
      charpol[0]=1;
      return charpol;
    }
    // max value of any coeff in the charpoly
    // max eigenval is <= sqrt(n)||A|| hence bound is in n (log(B)+log(n)/2)
    // we must add combinatorial (n k)<=2^n
    // or optimize comb(n,k)*(sqrt(n)||A||)^n
    // gives k<=n/(1+1/sqrt(n)B)
    double logbound=n/(1+1.0/std::sqrt(double(n))/Bd)*(std::log10(double(n))/2+std::log10(Bd))+n*std::log10(2.0),testvalue;
    double proba=proba_epsilon(contextptr),currentprob=1;
    gen currentp(init_modulo(n,logbound));
    gen pip(currentp);
    double pipd=std::log10(pip.val/2+1.0);
    vector<int> modpcar;
    vector< vector<int> > N;
    if (!mod_pcar(A,N,currentp.val,krylov,modpcar,contextptr,compute_pmin))
      return vecteur(1,gensizeerr(contextptr));
    modpoly charpol;
    vector_int2vecteur(modpcar,charpol);
    int initial_clock=CLOCK();
    int dbglevel=debug_infolevel;
    for (;pipd < (testvalue=logbound*charpol.size()/(n+1.0));){
      if (currentprob < proba &&  pipd<testvalue/1.33 && CLOCK()-initial_clock>min_proba_time*CLOCKS_PER_SEC)
	break;
      if (n>10 && dbglevel<2 && CLOCK()-initial_clock>60*CLOCKS_PER_SEC)
	dbglevel=2;
      if (dbglevel>1)
	CERR << CLOCK() << " " << 100*pipd/testvalue << " % done" << (currentprob<proba?", stable.":", unstable.")<< endl;
      currentp=nextprime(currentp.val+2);
      if (!mod_pcar(A,N,currentp.val,krylov,modpcar,contextptr,compute_pmin))
	return vecteur(1,gensizeerr(contextptr));
      if (modpcar.size()<charpol.size())
	continue;
      if (modpcar.size()>charpol.size()){
	vector_int2vecteur(modpcar,charpol);
	pip=currentp;
	continue;
      }
      bool stable;
      int tmp;
      if (pip.type==_ZINT && (tmp=ichinrem_inplace(charpol,modpcar,pip,currentp.val)) ){
	stable=tmp==2;
      } else {
	modpoly newcharpol,currentcharpol;
	vector_int2vecteur(modpcar,currentcharpol);
	newcharpol=ichinrem(charpol,currentcharpol,pip,currentp);
	stable=newcharpol==charpol;
	charpol.swap(newcharpol);
      }
      if (stable)
	currentprob=currentprob/currentp.val;
      else 
	currentprob=1.0;
      pip=pip*currentp;
      pipd += std::log10(double(currentp.val));
    }
    if (pipd<testvalue)
      *logptr(contextptr) << gettext("Probabilistic answer. Run proba_epsilon:=0 for a certified result. Error <") << proba << endl;
    return charpol;
  } // end if (is_integer_matrix)


  // Fadeev algorithm to compute the char poly of a matrix
  // B is a vector of matrices
  // the returned value is the vector of coeff of the char poly
  // see modpoly.h for polynomial operations on vecteur
  dense_POLY1 mpcar(const matrice & a,vecteur & Bv,bool compute_Bv,bool convert_internal,GIAC_CONTEXT){
    int n=int(a.size());
    gen modulo,fieldpmin;
    matrice A,Bi,Ai,I,lv;
    if (convert_internal){
      // convert a to internal form
      lv=alg_lvar(a);
      A = *(e2r(a,lv,contextptr)._VECTptr);
    }
    else
      A=a;
    midn(n,I);
    Bi=I; // B0=Id
    Bv.push_back(Bi); 
    vecteur P;
    gen pk;
    P.push_back(1); // p0= 1
    for (int i=1;i<=n;++i){
      // for polynomial coefficients interpolate?
      mmult(A,Bi,Ai); // Ai = A*Bi
      pk = rdiv(-mtrace(Ai),i,contextptr); 
      P.push_back(convert_internal?r2e(pk,lv,contextptr):pk);
      addvecteur( Ai,multvecteur(pk,I),Bi); // Bi = Ai+pk*I
      // COUT << i << ":" << Bi << endl;
      if (i!=n)
	Bv.push_back(convert_internal?r2e(Bi,lv,contextptr):Bi);
    }
    return P;
  }

  dense_POLY1 mpcar(const matrice & a,vecteur & Bv,bool compute_Bv,GIAC_CONTEXT){
    return mpcar(a,Bv,compute_Bv,false,contextptr);
  }

  gen _lagrange(const gen & g,GIAC_CONTEXT);

  static gen pcar_interp(const matrice & a,gen & g,GIAC_CONTEXT){
    vecteur res;
    if (poly_pcar_interp(a,res,false,contextptr)){
      if (g.type==_VECT)
	return res;
      return symb_horner(res,g);
    }
    int m=int(a.size());
    vecteur v1,v2,I(midn(m));
    int shift=-m/2;
    for (int j=0;j<=m;++j){
      v1.push_back(j-shift);
      v2.push_back(mdet(addvecteur(a,multvecteur(shift-j,I)),contextptr));
    }
    return _lagrange(makevecteur(v1,v2,g),contextptr);
  }

  gen _pcar(const gen & a,GIAC_CONTEXT){
    if ( a.type==_STRNG && a.subtype==-1) return  a;
    vecteur Bv;
    matrice M;
    gen b(undef);
    if (!is_squarematrix(a)){
      if (a.type!=_VECT)
	return symb_pcar(a);
      vecteur v=*a._VECTptr;
      int s=int(v.size());
      if (s<2 || !is_squarematrix(v.front()))
	return gensizeerr(contextptr);
      matrice &m=*v.front()._VECTptr;
      if (v.back().type==_INT_ && v.back().val==_FADEEV){
	vecteur res=mpcar(m,Bv,false,true,contextptr);
	return s==2?res:symb_horner(res,v[1]);
      }
      if (v.back()==at_pmin && probabilistic_pmin(m,Bv,false,contextptr))
	return s==2?Bv:symb_horner(Bv,v[1]); 
      if (v.back()==at_lagrange || v.back()==at_interp)
	return pcar_interp(m,s==2?vx_var():v[1],contextptr);
      b=v[1];
      M=m;
    }
    else
      M=*a._VECTptr;
    // int n=M.size();
    // search for the best algorithm
    gen p=M[0][0];
#if 0
    if (p.type==_USER){
      std_matrix<gen> m; vecteur w;
      matrice2std_matrix_gen(M,m);
      mod_pcar(m,w,true);
      if (is_undef(b))
	return gen(w,_POLY1__VECT);
      return symb_horner(w,b);	
    }
#endif
    if (p.type==_MOD && (p._MODptr+1)->type==_INT_){
      gen mg=unmod(M);
      if (mg.type==_VECT){
	matrice M1=*mg._VECTptr;
	vector< vector<int> > N;
	int modulo=(p._MODptr+1)->val;
	bool krylov=true;
	vector<int> res;
	if (mod_pcar(M1,N,modulo,krylov,res,contextptr,false)){
	  vecteur w;
	  vector_int2vecteur(res,w);
	  w=*makemod(w,modulo)._VECTptr;
	  if (is_undef(b))
	    return gen(w,_POLY1__VECT);
	  return symb_horner(w,b);	
	}
      }
    }
    if (is_fraction_matrice(M)){
      gen res=pcar_interp(M,is_undef(b)?vx_var():b,contextptr);
      return is_undef(b)?_e2r(res,contextptr):res;
    }
    vecteur res;
    if (poly_pcar_interp(M,res,false,contextptr))
      return res;
    res=mpcar(M,Bv,false,true,contextptr);
    if (is_undef(b))
      return res;
    return symb_horner(res,b);
  }
  static const char _pcar_s []="pcar";
  static define_unary_function_eval (__pcar,&_pcar,_pcar_s);
  define_unary_function_ptr5( at_pcar ,alias_at_pcar,&__pcar,0,true);


  static vecteur polymat2mat(const vecteur & v){
    if (v.empty()) 
      return v;
    if (v.front().type!=_VECT)
      return vecteur(1,gensizeerr(gettext("polymat2mat")));
    int l,c,s=int(v.size());
    vecteur w(v);
    for (int i=0;i<s;++i)
      w[i]=mtran(*v[i]._VECTptr);
    mdims(*v.front()._VECTptr,l,c);
    vecteur mat;
    mat.reserve(l*s);
    for (int k=0;k<s;++k){
      gen & g=w[k];
      for (int i=0;i<l;++i){
	mat.push_back(g[i]);
      }
    }
    return mat;
  }

  // dot product of a[0..a.size()-1] and b[pos..pos+a.size()-1]
  gen generalized_dotvecteur(const vecteur & a,const vecteur & b,int pos){
    vecteur::const_iterator ita=a.begin(), itaend=a.end();
    vecteur::const_iterator itb=b.begin()+pos;
    gen res;
    for (;(ita!=itaend);++ita,++itb){
      res = res + (*ita)*(*itb);
    }
    return res;
  }
  
  vecteur generalized_multmatvecteur(const matrice & a,const vecteur & b){
    vecteur::const_iterator ita=a.begin(), itaend=a.end();
    int s=int(b.size());
    int n=int(itaend-ita); // number of vectors stored in b=s/n
    vecteur res;
    res.reserve(s);
    for (int i=0;i<s;i+=n){
      for (ita=a.begin();ita!=itaend;++ita){
	res.push_back(generalized_dotvecteur(*(ita->_VECTptr),b,i));
      }
    }
    return res;
  }

  // [almost] rational jordan block
  matrice rat_jordan_block(const vecteur & v,int n,bool pseudo){
    if (n<1)
      return vecteur(1,gendimerr(gettext("rat_jordan_block")));
    int s=int(v.size())-1;
    // Size of the matrix is s*n
    vecteur ligne(s*n,zero);
    std_matrix<gen> M(s*n,ligne);
    for (int i=0;i<n;++i){
      // Fill the block-diagonal part with companion block
      for (int j=0;j<s;++j){
	M[i*s+j][i*s+s-1]=-v[s-j];
	if (j>0)
	  M[i*s+j][i*s+j-1]=plus_one;
      }
      // Fill the upper diagonal with idn or a single 1
      if (i!=n-1){
	if (pseudo)
	  M[i*s][i*s+s+s-1]=1;
	else {
	  for (int j=0;j<s;++j){
	    M[i*s+j][i*s+s+j]=1;
	  }
	}
      }
    }
    matrice res;
    std_matrix_gen2matrice_destroy(M,res);
    return res;
  }

  matrice pseudo_rat_to_rat(const vecteur & v,int n){
    if (n<1)
      return vecteur(1,gendimerr(gettext("pseudo_rat_ro_rat")));
    matrice A(rat_jordan_block(v,n,true));
    if (is_undef(A)) return A;
    // lines of A are initial v
    vecteur q(v);
    int d=int(q.size())-1; // degree of the polynomial
    matrice res(midn(n*d));
    vreverse(q.begin(),q.end());
    for (int j=1;j<n;++j){
      // compute Q(A) v_{j,0}
      vecteur QAvj0(n*d);
      for (int l=1;l<=d;++l){
	int mmax=giacmin(l,j);
	for (int m=1;m<=mmax;++m){
	  QAvj0=addvecteur(QAvj0,multvecteur(q[l]*comb((unsigned long) l,(unsigned long)m),*res[(j-m)*d+(l-m)]._VECTptr));
	}
      }
      // shift
      vecteur vj0=mergevecteur(vecteur(d),vecteur(QAvj0.begin(),QAvj0.begin()+(n-1)*d));
      // replace in res
      res[j*d]=vj0;
      // compute images by A, ..., A^[d-1]
      for (int l=1;l<d;++l){
	vj0=multmatvecteur(A,vj0);
	vecteur tmp(vj0);
	int mmax=giacmin(l,j);
	for (int m=1;m<=mmax;++m)
	  tmp=subvecteur(tmp,multvecteur(comb((unsigned long) l,(unsigned long) m),*res[(j-m)*d+(l-m)]._VECTptr));
	res[j*d+l]=tmp;
      }
    }
    return res;
  }

  // input trn(p)*d*p=original matrix, d upper triangular
  // output p*d*inv(p)=original matrix, d diagonal
  bool schur_eigenvectors(matrice &p,matrice & d,double eps,GIAC_CONTEXT){
    bool ans=true;
    int dim=int(p.size());
    matrice m(midn(dim)); 
    // columns of m are the vector of the basis of the Schur decomposition
    // in terms of the eigenvector
    for (int k=1;k<dim;++k){
      // compute column k of m
      for (int j=0;j<k;++j){
	gen tmp=0;
	for (int i=0;i<k;++i){
	  tmp += d[i][k]*m[j][i];
	}
	if (!is_zero(tmp)) 
	  tmp = tmp/(d[j][j]-d[k][k]);
	(*m[j]._VECTptr)[k]=tmp;
      }
    }
    m=minv(m,contextptr);
    if (is_undef(m)) 
      return false;
    p=mmult(*_trn(p,contextptr)._VECTptr,m);
    // set d to its diagonal
    for (int i=0;i<dim;++i){
      vecteur & di= *d[i]._VECTptr;
      for (int j=0;j<dim;++j){
	if (j==i) continue;
#ifndef GIAC_HAS_STO_38
	if (ans && j==i-1 && is_greater(abs(di[j]/di[j+1],contextptr),eps,contextptr)){
	  *logptr(contextptr) << gettext("Low accuracy for Schur row ") << j << " " << d[i] << endl;
	  ans=false;
	}
#endif
	di[j]=0;
      }
    }
    return ans;
  }



  // if jordan is false, errors for non diagonalizable matrices
  // if jordan is true, d is a matrix, not a vector
  bool egv(const matrice & m0,matrice & p,vecteur & d, GIAC_CONTEXT,bool jordan,bool rational_jordan_form,bool eigenvalues_only){
    matrice m=m0;
    if (m.size()==1){
      p=vecteur(1,vecteur(1,1));
      if (jordan)
	d=m;
      else
	d=*m.front()._VECTptr;
      return true;
    }
    if (has_num_coeff(m)){
      gen g=evalf(m,1,contextptr);
      if (g.type==_VECT)
	m=*g._VECTptr;
    }
    bool numeric_matrix=is_fully_numeric(m);
    bool sym=(m==mtran(*conj(m,contextptr)._VECTptr));
    double eps=epsilon(contextptr);
    if (eps<1e-15) eps=1e-15;
    // check for symmetric numeric matrix
    if (numeric_matrix){
      std_matrix<gen> H,P;
      matrice2std_matrix_gen(m,H);
      int dim(int(H.size()));
      matrice pid(midn(dim));
      matrice2std_matrix_gen(pid,P);
#if 1
      hessenberg_schur(H,P,SOLVER_MAX_ITERATE,dim*eps,contextptr);
      gen sumabs,shift;
      for (int i=0;i<dim;++i)
	sumabs += abs(H[i][i],contextptr);
      for (int i=dim-2;i>=0;--i){
	if (is_strictly_greater(abs(H[i+1][i],contextptr),10*dim*dim*eps*sumabs,contextptr)){
	  // shift H by cst_i
	  shift = sumabs*cst_i;
	  for (int j=0;j<dim;++j){
	    H[j][j] += shift;
	  }
	  hessenberg_schur(H,P,SOLVER_MAX_ITERATE,dim*eps,contextptr);
	  break;
	}
      }
      std_matrix_gen2matrice_destroy(P,p);
      std_matrix_gen2matrice_destroy(H,d);
      bool ans=schur_eigenvectors(p,d,eps,contextptr);
      if (!is_zero(shift)) 
	d=d-shift*midn(dim);
      return ans;
#else
      bool ans=francis_schur(H,0,dim,P,SOLVER_MAX_ITERATE,dim*eps,false,true,true,true,contextptr);
      std_matrix_gen2matrice_destroy(P,p);
      std_matrix_gen2matrice_destroy(H,d);
      return ans && schur_eigenvectors(p,d,eps,contextptr);
#endif
    } // end if (numeric_matrix)
    int taille=int(m.size());
    vecteur lv;
    alg_lvar_halftan_tsimplify(m,lv,contextptr);
    numeric_matrix=has_num_coeff(m) && is_fully_numeric(evalf(m,1,contextptr));
    matrice mr=*(e2r(numeric_matrix?exact(m,contextptr):m,lv,contextptr)._VECTptr); // convert to internal form
    // vecteur lv;
    // matrice mr = m;
    matrice m_adj;
    vecteur p_car;
    p_car=mpcar(mr,m_adj,true,contextptr);
    p_car=common_deno(p_car)*p_car; // remove denominators
    // extension handling
    gen modulo,fieldpmin;
    if (has_mod_coeff(p_car,modulo)){
      modpoly pc=*unmod(p_car)._VECTptr;
      vector< facteur<modpoly> > vpc; vector<modpoly> qmat;
      environment env;
      env.modulo=modulo; env.moduloon=true; env.pn=modulo;
      if (ddf(pc,qmat,&env,vpc)){
	int extdeg=1;
	for (int j=0;j<int(vpc.size());++j){
	  extdeg=lcm(extdeg,vpc[j].mult).val;
	}
#ifndef NO_RTTI
	if (extdeg>1){
	  *logptr(contextptr) << "Creating splitting field extension GF(" << modulo << "," << extdeg << ")" << endl;
	  gen tmp=_galois_field(makesequence(modulo,extdeg),contextptr);
	  tmp=tmp[plus_two];
	  tmp=eval(tmp[2],1,contextptr); // field generator
	  p_car=tmp*p_car;
	}
#endif
      }
      else
	*logptr(contextptr) << "Warning! Automatic extension not implemented. You can try to diagonalize the matrix * a non trivial element of GF(" << modulo << ",lcm of degrees of factor(" << symb_horner(p_car,vx_var()) << "))" <<  endl;
    }
#ifndef NO_RTTI
    if (has_gf_coeff(p_car,modulo,fieldpmin)){
      factorization f;
      gen res=gf_list()[pow(modulo,gfsize(fieldpmin),contextptr)].g;
      if (galois_field * ptr=dynamic_cast<galois_field *>(res._USERptr)){
	polynome P(1);
	poly12polynome(p_car,1,P,1);
	res=ptr->polyfactor(P,f);
	int extdeg=1;
	for (int i=0;i<int(f.size());++i){
	  extdeg=lcm(extdeg,f[i].fact.lexsorted_degree()).val;
	}
	if (extdeg>1){
	  extdeg *= gfsize(fieldpmin);
	  *logptr(contextptr) << "Creating splitting field extension GF(" << modulo << "," << extdeg << ")" << endl;
	  gen tmp=_galois_field(makesequence(modulo,extdeg),contextptr);
	  tmp=tmp[plus_two];
	  tmp=eval(tmp[2],1,contextptr); // field generator
	  p_car=tmp*p_car;
	}
      }
    }
#endif
    // factorizes p_car
    factorization f;
    polynome ppcar(poly1_2_polynome(p_car,1));
    polynome p_content(ppcar.dim);
    gen extra_div=1;
    if (!factor(ppcar,p_content,f,false,rational_jordan_form?false:withsqrt(contextptr),
		//false,
		complex_mode(contextptr),
		1,extra_div))
      return false;
    // insure that extra extensions created in factor are reduced inside m_adj
    //clean_ext_reduce(m_adj);
    factorization::const_iterator f_it=f.begin(),f_itend=f.end();
    int total_char_found=0;
    for (;f_it!=f_itend;++f_it){
      // find roots of it->fact
      // works currently only for 1st order factors
      // vecteur v=solve(f_it->fact);
      vecteur v;
      const polynome & itfact=f_it->fact;
      vecteur w=polynome2poly1(itfact,1);
      int s=int(w.size());
      if (s<2)
	continue;
      if (s==2)
	v.push_back(rdiv(-w.back(),w.front(),contextptr));
      if (is_undef(v))
	return false;
      gen x;
      vecteur cur_m_adj(m_adj),cur_lv(lv),new_m_adj,char_m;
      if (s>=3 && rational_jordan_form){
	int mult=f_it->mult;
	int qdeg=s-1;
	int n=mult*qdeg; // number of vectors to find
	// Divide cur_m_adj by w f_it->mult times
	// Collect the remainders matrices in C
	vecteur C,quo,rem;
	int char_line=0,char_found=0,cycle_size=mult; 
	for (int i=0;i<mult;++i){
	  DivRem(cur_m_adj,w,0,quo,rem);
	  // rem is a polynomial made of matrices
	  // we convert it to a matrix (explode the polys)
	  if (rem.empty()){
	    --cycle_size;
	  }
	  else {
	    C=mergematrice(C,polymat2mat(rem));
	    if (is_undef(C)) return false;
	  }
	  cur_m_adj=quo;
	}
	// char_line is the line where the reduction begins
	vecteur Ccopy(C),pivots;
	gen det;
	for (;char_found<n;){
	  // Reduce
	  if (!mrref(Ccopy,C,pivots,det,0,int(Ccopy.size()),0,taille,
		/* fullreduction */1,char_line,true,1,0,
		     contextptr))
	    return false;
	  // Extract a non-0 line at char_line
	  vecteur line=*C[char_line]._VECTptr;
	  if (is_zero(vecteur(line.begin(),line.begin()+taille),contextptr)){
	    // Keep lines 0 to char_line-1, remove last taille columns
	    Ccopy=mtran(vecteur(C.begin(),C.begin()+char_line));
	    if (signed(Ccopy.size())<taille)
	      return false; // setdimerr();
	    vecteur debut(Ccopy.begin(),Ccopy.end()-taille);
	    debut=mtran(debut);
	    // Cut first taille columns of the remainder of the matrix
	    Ccopy=mtran(vecteur(C.begin()+char_line,C.end()));
	    if (signed(Ccopy.size())<taille)
	      return false; // setdimerr();
	    vecteur fin(Ccopy.begin()+taille,Ccopy.end());
	    fin=mtran(fin);
	    Ccopy=mergevecteur(debut,fin);
	    --cycle_size;
	    continue;
	  }
	  Ccopy=vecteur(C.begin(),C.begin()+char_line);
	  // make a bloc with line and A, A^2, ..., A^[qdeg-1]*line
	  // and put them into Ccopy and in ptmp
	  vecteur ptmp;
	  for (int i=0;i<qdeg;++i){
	    Ccopy.push_back(line);
	    ptmp.push_back(line);
	    line=generalized_multmatvecteur(mr,line);
	  }
	  // finish Ccopy by copying the remaining lines of C
	  const_iterateur ittmp=C.begin()+char_line+1,ittmpend=C.end();
	  for (;ittmp!=ittmpend;++ittmp)
	    Ccopy.push_back(*ittmp);
	  // update d (with a ratjord bloc) 
	  int taille_bloc=qdeg*cycle_size;
	  matrice tmp=mtran(rat_jordan_block(w,cycle_size,false));
	  tmp=mergematrice(vecteur(qdeg*cycle_size,vecteur(total_char_found)),tmp);
	  tmp=mergematrice(tmp,vecteur(qdeg*cycle_size,vecteur(taille-total_char_found-taille_bloc)));
	  if (is_undef(tmp)) return false;
	  d=mergevecteur(d,tmp);
	  // update p with ptmp 
	  matrice padd;
	  for (int j=0;j<cycle_size;++j){
	    for (int i=0;i<qdeg;++i){
	      vecteur & ptmpi=*ptmp[i]._VECTptr;
	      padd.push_back(vecteur(ptmpi.begin()+taille*j,ptmpi.begin()+taille*(j+1)));
	    }  
	  }
	  matrice AA(pseudo_rat_to_rat(w,cycle_size));
	  if (is_undef(AA)) return false;
	  padd=mmult(AA,padd);
	  p=mergevecteur(p,padd);
	  char_found += taille_bloc;
	  total_char_found += taille_bloc;
	  char_line += cycle_size;
	}
	continue;
      } // end if s>=3 and rational_jordan_form
      if (s>=3){ // recompute cur_m_adj using new extensions
	cur_m_adj=*r2sym(m_adj,lv,contextptr)._VECTptr;
	identificateur tmpx(" x");
	vecteur ww(w.size());
	for (unsigned i=0;i<w.size();++i)
	  ww[i]=r2e(w[i],lv,contextptr);
	gen wwx=horner(ww,tmpx);
	v=solve(wwx,tmpx,complex_mode(contextptr),contextptr); 
	v=*apply(v,recursive_normal,contextptr)._VECTptr;
	if (v.size()!=w.size()-1){
	  gen m0num=evalf(m0,1,contextptr);
	  if (m0num.type==_VECT 
	      && is_numericm(*m0num._VECTptr)
	      // && lidnt(m0num).empty()
	      ){
	    *logptr(contextptr) << gettext("Unable to find exact eigenvalues. Trying approx") << endl;
	    return egv(*m0num._VECTptr,p,d,contextptr,jordan,false,eigenvalues_only);
	  }
	}
	// compute new lv and update v and m_adj accordingly
	cur_lv=alg_lvar(v);
	alg_lvar(cur_m_adj,cur_lv);
	cur_m_adj=*(e2r(cur_m_adj,cur_lv,contextptr)._VECTptr);
	v=*(e2r(v,cur_lv,contextptr)._VECTptr);
      }
      const_iterateur it=v.begin(),itend=v.end();
      gen cur_m;
      for (;it!=itend;++it){
	vecteur cur_m_adjx(cur_m_adj);
	char_m.clear();
	int n=f_it->mult;
	x=r2sym(*it,cur_lv,contextptr);
	if (eigenvalues_only && !jordan){
	  d=mergevecteur(d,vecteur(n,x));
	  total_char_found +=n;
	  continue;
	}
	// compute Taylor expansion of m_adj at roots of it->fact
	// at order n-1
	for (;;){
	  --n;
	  if (n){
	    cur_m=horner(cur_m_adjx,*it,0,new_m_adj);
	    if (char_m.empty())
	      char_m=mtran(*cur_m._VECTptr);
	    else
	      char_m=mergematrice(char_m,mtran(*cur_m._VECTptr));
	    if (is_undef(char_m) || (!jordan && !is_zero(cur_m,contextptr)) ){
#ifndef NO_STDEXCEPT
	      throw(std::runtime_error("Not diagonalizable at eigenvalue "+x.print()));
#endif
	      return false;
	    }
	    cur_m_adjx=new_m_adj;
	  }
	  else {
	    cur_m=horner(cur_m_adjx,*it);
	    char_m=mergematrice(char_m,mtran(*cur_m._VECTptr));
	    if (is_undef(char_m)) return false;
	    break;
	  }
	}
	n=f_it->mult;
	if (n==1){ 
	  char_m=mtran(*cur_m._VECTptr);
	  iterateur ct=char_m.begin(),ctend=char_m.end();
	  for (;ct!=ctend;++ct){
	    if (!is_zero(*ct,contextptr))
	      break;
	  }
	  if (ct==ctend)
	    return false; // setsizeerr(gettext("egv/jordan bug"));
	  // FIXME take 1st non-0 col as eigenvector
	  *ct=*ct/lgcd(*ct->_VECTptr);
	  gen eigenvector=r2sym(*ct,cur_lv,contextptr);
	  if (is_fully_numeric(eigenvector) || numeric_matrix)
	    eigenvector=_normalize(eigenvector,contextptr);
	  p.push_back(eigenvector);
	  if (jordan){
	    vecteur vegv(taille,zero);
	    if (total_char_found>taille)
	      return false; // setsizeerr(gettext("Bug in egv/jordan"));
	    vegv[total_char_found]=x;
	    d.push_back(vegv);
	  }
	  else
	    d.push_back(x);
	  ++total_char_found;
	  continue;
	}
	if (jordan){
	  // back to external form
	  char_m=*r2sym(char_m,cur_lv,contextptr)._VECTptr;
	  int egv_found=0;
	  int char_found=0;
	  vecteur char_m_copy(char_m),pivots;
	  gen det;
	  for (;char_found<n;){ 
	    if (!mrref(char_m_copy,char_m,pivots,det,0,taille,0,taille,
		  /* fullreduction */1,egv_found,true,1,0,
		       contextptr))
	      return false;
	    if (sym )
	      char_m=gramschmidt(char_m,false,contextptr);
	    char_m_copy.clear();
	    // extract non-0 lines starting from line number egv_found
	    vecteur vegv;
	    int j=0;
	    for (;j<egv_found;++j)
	      char_m_copy.push_back(vecteur(char_m[j]._VECTptr->begin(),char_m[j]._VECTptr->end()-taille));
	    for (;j<taille;++j){
	      vegv=vecteur( char_m[j]._VECTptr->begin(),char_m[j]._VECTptr->begin()+taille);
	      if (is_zero(vegv,contextptr) || (numeric_matrix && evalf(abs(vegv,contextptr),1,contextptr)._DOUBLE_val<10*taille*epsilon(contextptr)) ) 
		break;
	      // cycle found! 
	      // update char_m_copy with all the cycle except first vector
	      char_m_copy.push_back(vecteur(char_m[j]._VECTptr->begin(),char_m[j]._VECTptr->end()-taille));
	      // Store cycle
	      const_iterateur c_it=char_m[j]._VECTptr->begin(),c_itend=char_m[j]._VECTptr->end();
	      for (;c_it!=c_itend;c_it+=taille){
		p.push_back(vecteur(c_it,c_it+taille)); // char vector
		// update d
		vegv=vecteur(taille,zero);
		if (total_char_found>=taille)
		  return false; // setsizeerr(gettext("Bug in egv/jordan"));
		if (c_it==char_m[j]._VECTptr->begin()){
		  vegv[total_char_found]=x;
		  ++egv_found;
		}
		else {
		  vegv[total_char_found-1]=1;
		  vegv[total_char_found]=x;
		}
		++char_found;
		++total_char_found;
		d.push_back(vegv);
	      }
	    }
	    for (;j<taille;++j){
	      char_m_copy.push_back(vecteur(char_m[j]._VECTptr->begin()+taille,char_m[j]._VECTptr->end()));
	    }
	  }
	} // end if (jordan)
	else {
	  d=mergevecteur(d,vecteur(n,x));
	  // back to external form
	  cur_m=r2sym(cur_m,cur_lv,contextptr);
	  // column reduction
	  matrice m_egv=mrref(mtran(*cur_m._VECTptr),contextptr);
	  if (sym){
	    // orthonormalize basis
	    m_egv=gramschmidt(matrice(m_egv.begin(),m_egv.begin()+f_it->mult),false,contextptr);
	  }
	  // non zero rows of cur_m are eigenvectors
	  const_iterateur m_it=m_egv.begin(),m_itend=m_egv.end();
	  for (; m_it!=m_itend;++m_it){
	    if (!is_zero(*m_it,contextptr))
	      p.push_back(*m_it);
	  }
	}
      }
    } // end for factorization
    if (!p.empty()){
      if (!eigenvalues_only)
	p=mtran(p);
      if (jordan)
	d=mtran(d);
    }
    return true;
  }
  matrice megv(const matrice & e,GIAC_CONTEXT){
    matrice m;
    vecteur d;
    bool b=complex_mode(contextptr);
    complex_mode(true,contextptr);
    if (!egv(e,m,d,contextptr,false,false,false))
      *logptr(contextptr) << gettext("Low accuracy or not diagonalizable at some eigenvalue. Try jordan if the matrix is exact.") << endl;
    complex_mode(b,contextptr);
    return m;
  }

  gen symb_egv(const gen & a){
    return symbolic(at_egv,a);
  }
  gen _egv(const gen & a,GIAC_CONTEXT){
    if ( a.type==_STRNG && a.subtype==-1) return  a;
    if (!is_squarematrix(a)){
      if (a.type==_VECT)
	return gendimerr(contextptr);
      return symb_egv(a);
    }
    return megv(*a._VECTptr,contextptr);
  }
  static const char _egv_s []="egv";
  static define_unary_function_eval (__egv,&_egv,_egv_s);
  define_unary_function_ptr5( at_egv ,alias_at_egv,&__egv,0,true);


  vecteur megvl(const matrice & e,GIAC_CONTEXT){
    matrice m;
    vecteur d;
    bool b=complex_mode(contextptr);
    complex_mode(true,contextptr);
    if (!egv(e,m,d,contextptr,true,false,true))
      *logptr(contextptr) << gettext("Low accuracy") << endl;
    complex_mode(b,contextptr);
    return d;
  }
  gen symb_egvl(const gen & a){
    return symbolic(at_egvl,a);
  }
  gen _egvl(const gen & a,GIAC_CONTEXT){
    if ( a.type==_STRNG && a.subtype==-1) return  a;
    if (!is_squarematrix(a))
      return gendimerr(contextptr);
    return megvl(*a._VECTptr,contextptr);
  }
  static const char _egvl_s []="egvl";
  static define_unary_function_eval (__egvl,&_egvl,_egvl_s);
  define_unary_function_ptr5( at_egvl ,alias_at_egvl,&__egvl,0,true);

  vecteur mjordan(const matrice & e,bool rational_jordan,GIAC_CONTEXT){
    matrice m;
    vecteur d;
    if (!egv(e,m,d,contextptr,true,rational_jordan,false))
      *logptr(contextptr) << gettext("Low accuracy") << endl;
    return makevecteur(m,d);
  }
  gen symb_jordan(const gen & a){
    return symbolic(at_jordan,a);
  }
  gen jordan(const gen & a,bool rational_jordan,GIAC_CONTEXT){
    if (a.type==_VECT && a.subtype==_SEQ__VECT && a._VECTptr->size()==2 && is_squarematrix(a._VECTptr->front()) ){
      vecteur v(mjordan(*a._VECTptr->front()._VECTptr,rational_jordan,contextptr));
      if (is_undef(v))
	return v;
      gen tmpsto=sto(v[0],a._VECTptr->back(),contextptr);
      if (is_undef(tmpsto)) return tmpsto;
      return v[1];
    }
    if (!is_squarematrix(a))
      return symb_jordan(a);
    vecteur v(mjordan(*a._VECTptr,rational_jordan,contextptr));
    if (is_undef(v))
      return v;
    if (xcas_mode(contextptr)==1)
      return v[1];
    else
      return gen(v,_SEQ__VECT);
  }

  gen _jordan(const gen & a,GIAC_CONTEXT){
    if ( a.type==_STRNG && a.subtype==-1) return  a;
    bool mode=complex_mode(contextptr);
    complex_mode(true,contextptr);
    gen res=jordan(a,false,contextptr);
    complex_mode(mode,contextptr);
    return res;
  }
  static const char _jordan_s []="jordan";
  static define_unary_function_eval (__jordan,&_jordan,_jordan_s);
  define_unary_function_ptr5( at_jordan ,alias_at_jordan,&__jordan,0,true);

  gen _rat_jordan(const gen & a,GIAC_CONTEXT){
    if ( a.type==_STRNG && a.subtype==-1) return  a;
    return jordan(a,true,contextptr);
  }
  static const char _rat_jordan_s []="rat_jordan";
  static define_unary_function_eval (__rat_jordan,&_rat_jordan,_rat_jordan_s);
  define_unary_function_ptr5( at_rat_jordan ,alias_at_rat_jordan,&__rat_jordan,0,true);

  matrice diagonal_apply(const gen & g,const gen & x,const matrice & m,GIAC_CONTEXT){
    if (!is_squarematrix(m))
      return vecteur(1,gensizeerr(contextptr));
    int n=int(m.size());
    matrice res;
    for (int i=0;i<n;++i){
      vecteur v=*m[i]._VECTptr;
      gen tmp=subst(g,x,v[i],false,contextptr);
      if (is_undef(tmp))
	tmp=subst(g,x,v[i],true,contextptr);
      v[i]=tmp;
      res.push_back(v);
    }
    return res;
  }

  matrice analytic_apply(const gen &ux,const gen & x,const matrice & m,GIAC_CONTEXT){
    if (!is_squarematrix(m))
      return vecteur(1,gensizeerr(contextptr));
    int n=int(m.size());
    matrice p,d,N,v(n),D;
    bool cplx=complex_mode(contextptr),sqrtb=withsqrt(contextptr);
    complex_mode(true,contextptr);
    withsqrt(true,contextptr);
    if (!egv(m,p,d,contextptr,true,false,false))
      return vecteur(1,gensizeerr(contextptr));
    complex_mode(cplx,contextptr);
    withsqrt(sqrtb,contextptr);
    if (int(p.size())!=n)
      return vecteur(1,gensizeerr(gettext("Unable to find all eigenvalues")));
    // search for distance of 1st non-zero non-diagonal element
    int dist=0;
    for (int i=0;i<n;++i){
      for (int j=0;j<n;++j){
	const gen & g=d[i][j];
	if (!is_zero(g,contextptr) && i!=j)
	  dist=giacmax(dist,n-absint(i-j));
	if (i==j)
	  v[j]=g;
	else
	  v[j]=zero;
      }
      D.push_back(v);
    }
    identificateur y(" y");
    if (!dist) {// u(d) should be replaced with applying u to elements of d
      d=diagonal_apply(ux,x,d,contextptr); 
      if (is_undef(d)) return d;
      return mmult(mmult(p,d),minv(p,contextptr));
    }
    N=subvecteur(d,D);
    vecteur pol;
    if (!taylor(ux,x,y,dist,pol,contextptr)) 
      return vecteur(1,gensizeerr(ux.print()+gettext(" is not analytic")));
    if (is_undef(pol.back()))
      pol.pop_back();
    vreverse(pol.begin(),pol.end());
    // subst y with D (i.e. diagonal element by diagonal element)
    int pols=int(pol.size());
    for (int i=0;i<pols;++i){
      if (is_undef( (pol[i]=diagonal_apply(pol[i],y,D,contextptr)) ))
	return gen2vecteur(pol[i]);
    }
    gen res=horner(pol,N);
    if (res.type!=_VECT)
      return vecteur(1,gensizeerr(contextptr));
    d=mmult(p,*res._VECTptr);
    d=mmult(d,minv(p,contextptr));
    return d;
  }

  matrice analytic_apply(const unary_function_ptr *u,const matrice & m,GIAC_CONTEXT){
    identificateur x(" x");
    gen ux=(*u)(x,contextptr);
    return analytic_apply(ux,x,m,contextptr);
  }

  // return a vector which elements are the basis of the ker of a
  bool mker(const matrice & a,vecteur & v,int algorithm,GIAC_CONTEXT){
    v.clear();
    gen det;
    vecteur pivots;
    matrice res;
    if (!mrref(a,res,pivots,det,0,int(a.size()),0,int(a.front()._VECTptr->size()),
	  /* fullreduction */1,0,true,algorithm,0,
	       contextptr))
      return false;
    mdividebypivot(res,-1,contextptr);
    // put zero lines in res at their proper place, so that
    // non zero pivot are on the diagonal
    int s=int(res.size()),c=int(res.front()._VECTptr->size());
    matrice newres;
    newres.reserve(s);
    matrice::const_iterator it=res.begin(),itend=res.end();
    int i;
    for (i=0;(i<c) && (it!=itend);++i){
      if (it->_VECTptr->empty() || is_zero(((*(it->_VECTptr))[i]),contextptr)){
	newres.push_back(vecteur(c,zero));
      }
      else {
	newres.push_back(*it);
	++it;
      }
    }
    for (;i<c;++i)
      newres.push_back(vecteur(c,zero));
    // now tranpose newres & resize, keep the ith line if it's ith coeff is 0
    // replace 0 by -1 to get an element of the basis
    matrice restran;
    mtran(newres,restran,int(res.front()._VECTptr->size()));
    it=restran.begin();
    itend=restran.end();
    bool modular=!pivots.empty() && pivots.front().type==_MOD;
    for (int i=0;it!=itend;++it,++i){
      if (is_zero((*(it->_VECTptr))[i],contextptr)){
	(*(it->_VECTptr))[i]=modular?makemod(-1,*(pivots.front()._MODptr+1)):-1;
	v.push_back(*it);
      }
    }
    return true;
  }

  bool mker(const matrice & a,vecteur & v,GIAC_CONTEXT){
    return mker(a,v,1,contextptr);
  }

  vecteur mker(const matrice & a,GIAC_CONTEXT){
    vecteur v;
    if (!mker(a,v,contextptr))
      return vecteur(1,gendimerr(contextptr));
    return v;
  }
  gen _ker(const gen & a,GIAC_CONTEXT){
    if ( a.type==_STRNG && a.subtype==-1) return  a;
    if (!ckmatrix(a))
      return symb_ker(a);
    vecteur v;
    if (!mker(*a._VECTptr,v,contextptr))
      return vecteur(1,gendimerr(contextptr));
    return v;    
  }
  static const char _ker_s []="ker";
  static define_unary_function_eval (__ker,&_ker,_ker_s);
  define_unary_function_ptr5( at_ker ,alias_at_ker,&__ker,0,true);

  bool mimage(const matrice & a, vecteur & v,GIAC_CONTEXT){
    matrice atran;
    mtran(a,atran);
    v.clear();
    gen det;
    vecteur pivots;
    matrice res;
    if (!mrref(atran,res,pivots,det,0,int(atran.size()),0,int(atran.front()._VECTptr->size()),
	  /* fullreduction */1,0,true,1,0,
	       contextptr))
      return false;
    matrice::const_iterator it=res.begin(),itend=res.end();
    for (int i=0;it!=itend;++it,++i){
      if (!is_zero(*(it),contextptr))
	v.push_back(*it);
    }
    return true;
  }

  vecteur mimage(const matrice & a,GIAC_CONTEXT){
    vecteur v;
    if (!mimage(a,v,contextptr))
      return vecteur(1,gendimerr(contextptr));
    return v;
  }

  gen _image(const gen & a,GIAC_CONTEXT){
    if ( a.type==_STRNG && a.subtype==-1) return  a;
    if (!ckmatrix(a))
      return symb_image(a);
    vecteur v;
    if (!mimage(*a._VECTptr,v,contextptr))
      return gensizeerr(contextptr);
    return v;    
  }
  static const char _image_s []="image";
  static define_unary_function_eval (__image,&_image,_image_s);
  define_unary_function_ptr5( at_image ,alias_at_image,&__image,0,true);

  vecteur cross(const vecteur & v_orig,const vecteur & w_orig,GIAC_CONTEXT){
    vecteur v(v_orig),w(w_orig);
    int s1=int(v.size()),s2=int(w.size());
    bool vmat=ckmatrix(v),wmat=ckmatrix(w);
    if (vmat){
      if (s1!=1)
	v=mtran(v);
      v=*v.front()._VECTptr;
      s1=int(v.size());
    }
    if (wmat){
      if (s2!=1)
	w=mtran(w);
      w=*w.front()._VECTptr;
      s2=int(w.size());
    }
    if (s1==2){
      v.push_back(0);
      ++s1;
    }
    if (s2==2){
      w.push_back(0);
      ++s2;
    }
    if (s1!=3 || s2!=3)
      return vecteur(1,gendimerr(gettext("cross")));
    vecteur res;
    res.push_back(operator_times(v[1],w[2],contextptr)-operator_times(v[2],w[1],contextptr));
    res.push_back(operator_times(v[2],w[0],contextptr)-operator_times(v[0],w[2],contextptr));
    res.push_back(operator_times(v[0],w[1],contextptr)-operator_times(v[1],w[0],contextptr));
    if (vmat && wmat)
      return mtran(vecteur(1,res));
    return res;
  }
  /*
  vecteur cross(const vecteur & v_orig,const vecteur & w_orig){
    return cross(v_orig,w_orig,context0);
  }
  */
  gen symb_cross(const gen & arg1,const gen & arg2){
    return symbolic(at_cross,makesequence(arg1,arg2));
  }
  gen symb_cross(const gen & args){
    return symbolic(at_cross,args);
  }
  gen complex2vecteur(const gen & g,GIAC_CONTEXT){
    if (g.type!=_VECT){
      gen x,y;
      reim(g,x,y,contextptr);
      return makevecteur(x,y);
    }
    return g;
  }
    
  gen cross(const gen & a,const gen & b,GIAC_CONTEXT){
    gen g1(a);
    if (a.type==_VECT && a.subtype==_GGB__VECT)
      g1=a;
    gen g2(b);
    if (b.type==_VECT && b.subtype==_GGB__VECT)
      g2=b;
    if (g1.type!=_VECT || g2.type!=_VECT){
      g1=complex2vecteur(g1,contextptr);      
      g2=complex2vecteur(g2,contextptr);
      if (g1._VECTptr->size()==2 && g2._VECTptr->size()==2)
	return g1._VECTptr->front()*g2._VECTptr->back()-g1._VECTptr->back()*g2._VECTptr->front();
      if (g1._VECTptr->size()==2)
	g1=makevecteur(g1._VECTptr->front(),g1._VECTptr->back(),0);
      if (g2._VECTptr->size()==2)
	g2=makevecteur(g2._VECTptr->front(),g2._VECTptr->back(),0);
    }
    if (is_undef(g1) || g1.type!=_VECT || is_undef(g2) || g2.type!=_VECT)
      return gensizeerr(gettext("cross"));
    if (g1.subtype==_VECTOR__VECT && g2.subtype==_VECTOR__VECT)
      return _vector(cross(vector2vecteur(*g1._VECTptr),g2,contextptr),contextptr);
    if (g1.subtype==_VECTOR__VECT)
      return cross(vector2vecteur(*g1._VECTptr),g2,contextptr);
    if (g2.subtype==_VECTOR__VECT)
      return cross(g1,vector2vecteur(*g2._VECTptr),contextptr);
    if (g1._VECTptr->size()==2 && g2._VECTptr->size()==2 && calc_mode(contextptr)==1)
      return g1._VECTptr->front()*g2._VECTptr->back()-g1._VECTptr->back()*g2._VECTptr->front();
    return cross(*g1._VECTptr,*g2._VECTptr,contextptr);
  }
  /*
  gen cross(const gen & a,const gen & b){
    return cross(a,b,context0);
  }
  */
  gen _cross(const gen &args,GIAC_CONTEXT){
    if (args.type==_STRNG && args.subtype==-1) return args;
    if (args.type!=_VECT)
      return symb_cross(args);
    if (args._VECTptr->size()!=2)
      return gendimerr(contextptr);
    gen res=cross(args._VECTptr->front(),args._VECTptr->back(),contextptr);
    if (res.type==_VECT)
      res.subtype=args._VECTptr->front().subtype;
    return res;
  }
  static const char _cross_s []="cross";
  static define_unary_function_eval4 (__cross,&_cross,_cross_s,0,texprintsommetasoperator);
  define_unary_function_ptr5( at_cross ,alias_at_cross,&__cross,0,true);

  static string printassize(const gen & feuille,const char * sommetstr,GIAC_CONTEXT){
    string res(sommetstr);
    if (xcas_mode(contextptr)>0)
      res="nops";
    return res+"("+feuille.print(contextptr)+")";
  }
  gen symb_size(const gen & args){
    return symbolic(at_size,args);
  }
  gen _size(const gen &args,GIAC_CONTEXT){
    if (args.type==_STRNG && args.subtype==-1) return args;
    if (args.type==_STRNG)
      return (int) args._STRNGptr->size();
    if (args.type==_SYMB){
      if (args._SYMBptr->feuille.type==_VECT)
	return (int) args._SYMBptr->feuille._VECTptr->size();
      else
	return 1;
    }
    if (args.type==_POLY)
      return int(args._POLYptr->coord.size());
    if (args.type!=_VECT)
      return 1;
    int s=(int) args._VECTptr->size();
    if (args.subtype==_SEQ__VECT){
      if (s==2){
	if (args._VECTptr->back()==-1)
	  return tailles(args._VECTptr->front());
	return int(taille(args._VECTptr->front(),0));
      }
      if (s==0)
	return tailles(*_VARS(-2,contextptr)._VECTptr);
    }
    return s;
  }
  static const char _size_s []="size";
  static define_unary_function_eval2 (__size,&_size,_size_s,&printassize);
  define_unary_function_ptr5( at_size ,alias_at_size,&__size,0,true);


  bool mlu(const matrice & a0,vecteur & P,matrice & L,matrice & U,GIAC_CONTEXT){
    matrice a(a0);
    bool modular=false;
    if (!ckmatrix(a)){ // activate non-square matrix (instead of is_squarematrix)
      if (a.front().type==_VECT && !a.front()._VECTptr->empty() && (a.back()==at_irem || a.back()==at_ichinrem)){
	modular=true;
	a=*a.front()._VECTptr;
      }
      if (!ckmatrix(a)) return false; // setsizeerr(gettext("Expecting a square matrix"));
    }
    gen det;
    vecteur pivots;
    matrice res;
    int s=int(a.size()),C=int(a.front()._VECTptr->size());
    if (!mrref(a,res,pivots,det,0,s,0,C,
	  /* fullreduction */0,0,false,(modular?3:0) /* algorithm */,2 /* lu */,
	       contextptr))
      return false;
    if (pivots.empty())
      return false;
    gen tmp=pivots.back();
    if (tmp.type!=_VECT)
      return false; // setsizeerr();
    P=*tmp._VECTptr;
    // Make L and U from res
    L.reserve(s); U.reserve(s);
    for (int i=0;i<s;++i){
      vecteur & v=*res[i]._VECTptr;
      L.push_back(new ref_vecteur(s));
      vecteur & wl=*L.back()._VECTptr;
      for (int j=0;j<i && j<C;++j){ // L part
	wl[j]=v[j];
      }
      wl[i]=1;
      U.push_back(new ref_vecteur(C));
      vecteur & wu=*U.back()._VECTptr;
      for (int j=i;j<C;++j){ // U part
	wu[j]=v[j];
      }
    }
    return true;
  }

  // in: l= r rows, c cols
  // out: l= r,r and u=r rows, c cols
  void splitlu(matrice & l,matrice & u){
    u=l;
    int r,c;
    mdims(l,r,c);
    for (int i=0;i<r;++i){
      vecteur li=*l[i]._VECTptr;
      li.resize(r);
      vecteur & ui=*u[i]._VECTptr;
      for (int j=0;j<i;++j){
	ui[j]=0;
      }
      li[i]=1;
      for (int j=i+1;j<r;++j){
	li[j]=0;
      }
      l[i]=li;
    }
  }

  gen lu(const gen &args,GIAC_CONTEXT){
    matrice L,U,P;
    if (args.type!=_VECT)
      return gentypeerr(contextptr);
    // Giac LU decomposition
    if (!mlu(*args._VECTptr,P,L,U,contextptr))
      return gendimerr(contextptr);
    if (array_start(contextptr)){ //xcas_mode(contextptr) || abs_calc_mode(contextptr)==38){
      int s=int(P.size());
      for (int i=0;i<s;++i){
	P[i]=P[i]+1;
      }
    }
    return makesequence(P,L,U);
  }
  static const char _lu_s []="lu";
  static define_unary_function_eval (__lu,&lu,_lu_s);
  define_unary_function_ptr5( at_lu ,alias_at_lu,&__lu,0,true);

  gen qr(const gen &args_orig,GIAC_CONTEXT){
    gen args;
    int method=0; // use -1 to check built-in qr
#if !defined(HAVE_LIBLAPACK) || !defined (HAVE_LIBGSL)
    method=-3;
#endif
    if ( (args_orig.type==_VECT) && (args_orig._VECTptr->size()==2) && (args_orig._VECTptr->back().type==_INT_)){
      args=args_orig._VECTptr->front();
      method=args_orig._VECTptr->back().val;
    }
    else
      args=args_orig;
    if (!ckmatrix(args))
      return symbolic(at_qr,args);
    int rows = mrows(*args._VECTptr), cols = mcols(*args._VECTptr);
    if (rows < cols)
      method=-3;
    // if (!is_zero(im(args,contextptr),contextptr)) return gensizeerr(gettext("Complex entry!"));
    bool cplx=false;
    if (method<0 || !is_fully_numeric(evalf_double(args,1,contextptr)) || (cplx=!is_zero(im(args,contextptr),contextptr)) ){
      matrice r;
      if (is_fully_numeric(args)){ 
	// qr decomposition using rotations, numerically stable
	// but not suited to exact computations
	matrice h=*args._VECTptr,p(midn(int(h.size())));
	std_matrix<gen> H,P;
	matrice2std_matrix_gen(h,H);
	matrice2std_matrix_gen(p,P);
	qr_ortho(H,P,true,contextptr);
	std_matrix_gen2matrice_destroy(H,h);
	std_matrix_gen2matrice_destroy(P,p);
	if (method<=-3)
	  return makevecteur(_trn(p,contextptr),h);
	else
	  return makevecteur(_trn(p,contextptr),h,midn(int(h.size())));
      }
      // qr decomposition using GramSchmidt (not numerically stable)
      matrice res(gramschmidt(*_trn(args,contextptr)._VECTptr,r,cplx || method==-1 || method==-3,contextptr));
      if (method<=-3)
	return makesequence(_trn(res,contextptr),r);
      else
	return makesequence(_trn(res,contextptr),r,midn(int(r.size())));
    }
    return symbolic(at_qr,args);
  }
  static const char _qr_s []="qr";
  static define_unary_function_eval (__qr,&qr,_qr_s);
  define_unary_function_ptr5( at_qr ,alias_at_qr,&__qr,0,true);

  matrice thrownulllines(const matrice & res){
    int i=int(res.size())-1;
    for (;i>=0;--i){
      if (!is_zero(res[i],context0))
	break;
    }
    return vecteur(res.begin(),res.begin()+i+1);
  }
  gen _basis(const gen &args,GIAC_CONTEXT){
    if (args.type==_STRNG && args.subtype==-1) return args;
    if (!ckmatrix(args))
      return symbolic(at_basis,args);
    matrice res=mrref(*args._VECTptr,contextptr);
    return gen(thrownulllines(res),_SET__VECT);
  }
  static const char _basis_s []="basis";
  static define_unary_function_eval (__basis,&_basis,_basis_s);
  define_unary_function_ptr5( at_basis ,alias_at_basis,&__basis,0,true);

  void sylvester(const vecteur & v1,const vecteur & v2,matrice & res){
    int m=int(v1.size())-1;
    int n=int(v2.size())-1;
    if (m<0 || n<0){
      res.clear(); return;
    }
    res.resize(m+n);
    for (int i=0;i<n;++i){
      res[i]=new ref_vecteur(m+n);
      vecteur & w=*res[i]._VECTptr;
      for (int j=0;j<=m;++j)
	w[i+j]=v1[j];
    }
    for (int i=0;i<m;++i){
      res[n+i]=new ref_vecteur(m+n);
      vecteur & w=*res[n+i]._VECTptr;
      for (int j=0;j<=n;++j)
	w[i+j]=v2[j];
    }
  }

  // Sylvester matrix, in lines line0=v1 0...0, line1=0 v1 0...0, etc.
  matrice sylvester(const vecteur & v1,const vecteur & v2){
    matrice res;
    sylvester(v1,v2,res);
    return res;
  }

  gen _sylvester(const gen &args,GIAC_CONTEXT){
    if (args.type==_STRNG && args.subtype==-1) return args;
    if (args.type!=_VECT || args._VECTptr->size()<2)
      return gensizeerr(contextptr);
    vecteur & v = *args._VECTptr;
    gen x(vx_var());
    if (v.size()>2)
      x=v[2];
    gen p1(_e2r(makesequence(v[0],x),contextptr));
    gen p2(_e2r(makesequence(v[1],x),contextptr));
    if (p1.type==_FRAC)
      p1=inv(p1._FRACptr->den,contextptr)*p1._FRACptr->num;
    if (p2.type==_FRAC)
      p2=inv(p2._FRACptr->den,contextptr)*p2._FRACptr->num;
    if (p1.type!=_VECT || p2.type!=_VECT)
      return gensizeerr(contextptr);
    vecteur & v1 =*p1._VECTptr;
    vecteur & v2 =*p2._VECTptr;
    return sylvester(v1,v2);
  }
  static const char _sylvester_s []="sylvester";
  static define_unary_function_eval (__sylvester,&_sylvester,_sylvester_s);
  define_unary_function_ptr5( at_sylvester ,alias_at_sylvester,&__sylvester,0,true);

  gen _ibasis(const gen &args,GIAC_CONTEXT){
    if (args.type==_STRNG && args.subtype==-1) return args;
    if ( (args.type!=_VECT) || (args._VECTptr->size()!=2) )
      return symbolic(at_basis,args);
    gen g=args._VECTptr->front(),h=args._VECTptr->back();
    if (!ckmatrix(g) || !ckmatrix(h))
      return gensizeerr(contextptr);
    vecteur & v1=*g._VECTptr;
    vecteur & v2=*h._VECTptr;
    if (v1.empty() || v2.empty())
      return vecteur(0);
    vecteur v=mker(mtran(mergevecteur(v1,v2)),contextptr);
    if (is_undef(v)) return v;
    // if v is not empty compute each corresponding vector of the basis
    int s=int(v1.size());
    int l=int(v1.front()._VECTptr->size());
    matrice res;
    const_iterateur it=v.begin(),itend=v.end();
    for (;it!=itend;++it){
      vecteur tmp(l);
      vecteur & i=*it->_VECTptr;
      for (int j=0;j<s;++j)
	tmp=addvecteur(tmp,multvecteur(i[j],*v1[j]._VECTptr));
      res.push_back(tmp);
    }
    return gen(thrownulllines(mrref(res,contextptr)),_SET__VECT);
  }
  static const char _ibasis_s []="ibasis";
  static define_unary_function_eval (__ibasis,&_ibasis,_ibasis_s);
  define_unary_function_ptr5( at_ibasis ,alias_at_ibasis,&__ibasis,0,true);

  void sort_eigenvals(matrice & p,matrice & d,bool ascend,GIAC_CONTEXT){
    matrice pt; mtran(p,pt);
    vecteur D; D.reserve(d.size());
    for (int i=0;i<int(d.size());++i){
      gen tmp=makevecteur(d[i][i],pt[i]);
      D.push_back(tmp);
    }
    gen_sort_f_context(D.begin(),D.end(),complex_sort,contextptr);
    if (!ascend)
      vreverse(D.begin(),D.end());
    for (int i=0;i<int(D.size());++i){
      gen tmp=D[i];
      (*d[i]._VECTptr)[i]=tmp[0];
      pt[i]=tmp[1];
    }
    mtran(pt,p);
  }

  gen _svd(const gen &args_orig,GIAC_CONTEXT){
    if (args_orig.type==_STRNG && args_orig.subtype==-1) return args_orig;
    gen args;
    int method=0; // use -1 to check built-in svd, -2 for svl (singular values only) 
    if ( (args_orig.type==_VECT) && (args_orig._VECTptr->size()==2) && (args_orig._VECTptr->back().type==_INT_)){
      args=args_orig._VECTptr->front();
      method=args_orig._VECTptr->back().val;
    }
    else
      args=args_orig;
    if (!ckmatrix(args))
      return symbolic(at_svd,args);
    // if (!is_zero(im(args,contextptr),contextptr)) return gensizeerr(gettext("Complex entry!"));
    if (!has_num_coeff(args))
      *logptr(contextptr) << gettext("Warning: svd is implemented for numeric matrices") << endl;
    gen argsf=args;
    bool real=is_zero(im(argsf,contextptr));
    // non numeric code/also for complex
    if (!ckmatrix(argsf))
      return gensizeerr(contextptr);
    if (!lidnt(argsf).empty())
      *logptr(contextptr) << "Warning: SVD for symbolic matrix may fail!" << endl;
    matrice M=*argsf._VECTptr;
    bool transposed=M.size()<M.front()._VECTptr->size();
    if (transposed){
      gen tM=_trn(M,contextptr);
      if (!ckmatrix(tM))
	return gensizeerr(contextptr);
      M=*tM._VECTptr;
    }
    matrice tMM,p,d,Mp,invs,u;     vecteur svl;
    gen tMg=_trn(M,contextptr); // mtrn(*args._VECTptr,tm);
    if (!ckmatrix(tMg))
      return gensizeerr(contextptr);
    const matrice & tM=*tMg._VECTptr;
    if (M==tM){
      if (!egv(M,p,d,contextptr,true,false,false))
	return gensizeerr(contextptr);
      mtran(p,u);
      for (unsigned i=0;i<d.size();++i){
	vecteur vi=*d[i]._VECTptr;
	gen & di=vi[i];
	di=re(di,contextptr);
	if (is_strictly_positive(-di,contextptr))
	  u[i]=-u[i];
	svl.push_back(abs(di,contextptr));
      }
      if (method==-2)
	return svl;
      return makesequence(mtran(u),svl,p);
    }
    mmult(tM,M,tMM);
    if (!egv(tMM,p,d,contextptr,true,false,false))
      return gensizeerr(contextptr);
    // put 0 egvl at the beginning
    sort_eigenvals(p,d,true,contextptr);
    // should reorder eigenvalue (decreasing order)
    int s=int(d.size());
    gen svdmax2=d[s-1][s-1];
    gen eps=epsilon(contextptr);
    int smallsvl=0;
#if 1
    gen smalleps=(s*s)*eps*svdmax2;
#else
    for (int i=0;i<s-1;++i){
      if (is_greater(sqrt(eps,contextptr)*svdmax2,d[i][i],contextptr))
	++smallsvl;
      else
	break;
    }
    gen smalleps=s*pow(eps,inv(smallsvl?smallsvl:1,contextptr),contextptr)*svdmax2;
#endif
    for (int i=0;i<s;++i){
      vecteur vi=*d[i]._VECTptr;
      gen & di=vi[i];
      di=re(di,contextptr);
      // replace this value by 0 if it is small
      if (is_greater(smalleps,di,contextptr)) {
	di=0.0; smallsvl++;
      }
      di=sqrt(di,contextptr);
      svl.push_back(di);
      d[i]=vi;
    }
     if (smallsvl)
       *logptr(contextptr) << "Warning, ill-conditionned matrix, " << smallsvl << " small singular values were replaced by 0. Result is probably wrong." << endl;    
    if (method==-2){
      if (transposed){
	int add0=int(M.size()-M.front()._VECTptr->size());
	for (int i=0;i<add0;++i)
	  svl.push_back(0);
      }
      return svl;
    }
    mmult(M,p,Mp);
#if 0
    invs=d;
    for (int i=0;i<s;++i){
      invs[i]=*d[i]._VECTptr;
      gen & tmp=(*invs[i]._VECTptr)[i];
      tmp=inv(tmp,contextptr);
    }
    mmult(Mp,invs,u); 
    int complete=u.size()-u.front()._VECTptr->size();
    if (complete>0){
      // complete u to a unitary matrix by adding columns
      matrice tu;
      unsigned n=u.size();
      // take random vectors from canonical basis
      while (1){
	tu=*_trn(u,contextptr)._VECTptr;
	vector<int> v(n);
	for (unsigned i=0;i<n;++i)
	  v[i]=i;
	for (int i=0;i<complete;++i){
	  int j=int((double(std_rand())*v.size())/RAND_MAX);
	  vecteur tmp(n);
	  tmp[v[j]]=1;
	  tu.push_back(tmp);
	  v.erase(v.begin()+j);
	}
	gen uqr=qr(makesequence(_trn(tu,contextptr),-1),contextptr);
	if (uqr.type==_VECT && uqr._VECTptr->size()>=2 && is_squarematrix(uqr._VECTptr->front()) &&is_squarematrix((*uqr._VECTptr)[1]) ){
	  u=*uqr._VECTptr->front()._VECTptr;
	  tu=*_trn(u,contextptr)._VECTptr;
	  vecteur r=*(*uqr._VECTptr)[1]._VECTptr;
	  for (unsigned i=0;i<n;++i){
	    tu[i]=divvecteur(*tu[i]._VECTptr,r[i][i]);
	  }
	  u=*_trn(tu,contextptr)._VECTptr;
	  break;
	}
      }
    }
#else
    // M=u*s*trn(q), u and q unitary => tM*M=q*s^2*trn(q)
    // here tM*M=p*d^2*trn(p) so q=p is known, and u*s=M*q
    // since s is diagonal, u is obtained by dividing columns j of Mp by s[j]
    mtran(Mp,u);
    for (int i=0;i<s;++i){
      gen tmp=(*d[i]._VECTptr)[i];
      if (is_zero(tmp,contextptr)){ //is_greater(1e-8,tmp/(s*svdmax),contextptr)){
	tmp=l2norm(*u[i]._VECTptr,contextptr);
      }
      if (!is_zero(tmp,contextptr)) tmp=inv(tmp,contextptr);
      u[i]=tmp*u[i];
    }
    vreverse(u.begin(),u.end()); // put 0 SVD at the end
    mtran(u,Mp);
    // qr call required if 0 is a singular value
    gen tmp=qr(makesequence(Mp,-1),contextptr);
    if (tmp.type!=_VECT || tmp._VECTptr->size()!=3 || !ckmatrix(tmp._VECTptr->front()) || !ckmatrix(tmp[1]))
      return gensizeerr(contextptr);
    u=*tmp[0]._VECTptr;
    mtran(u,Mp);
    u=*tmp[1]._VECTptr;
    for (unsigned i=0;i<unsigned(u.size()) && int(i)<s;++i){
      if (is_strictly_positive(-u[i][i],contextptr))
	Mp[i]=-Mp[i];
    }
    if (s<Mp.size()) Mp.erase(Mp.begin()+s,Mp.end());
    vreverse(Mp.begin(),Mp.begin()+s);
    mtran(Mp,u);
#endif
    if (transposed)
      return makesequence(p,svl,u); 
    return makesequence(u,svl,p); 
  }
  static const char _svd_s []="svd";
  static define_unary_function_eval (__svd,&_svd,_svd_s);
  define_unary_function_ptr5( at_svd ,alias_at_svd,&__svd,0,true);

  gen _cholesky(const gen &_args,GIAC_CONTEXT){
    if (_args.type==_STRNG && _args.subtype==-1) return _args;
    if (!is_squarematrix(_args))
      return gensizeerr(contextptr);
    gen args;
    if (_args==_trn(_args,contextptr))
      args=_args;
    else
      args=(_args+_trn(_args,contextptr))/2;
    matrice &A=*args._VECTptr;
    int n=int(A.size()),j,k,l;
    std_matrix<gen> C(n,vecteur(n));
    for (j=0;j<n;j++) {
      gen s;
      for (l=j;l<n;l++) {
	s=0;
	for (k=0;k<j;k++) {
	  if (is_zero(C[k][k],contextptr)) 
	    return gensizeerr(gettext("Not invertible matrice"));
	  //if (is_strictly_positive(-C[k][k])) setsizeerr(gettext("Not a positive define matrice"));
	  s=s+C[l][k]*conj(C[j][k],contextptr)/C[k][k];
	}
	C[l][j]=ratnormal(A[l][j]-s,contextptr);
      }
    }
    for (k=0;k<n;k++) {
      gen c=normal(inv(sqrt(C[k][k],contextptr),contextptr),contextptr);
      for (j=k;j<n;j++) {
	C[j][k]=C[j][k]*c;
      }
    }
    matrice Cmat;
    std_matrix_gen2matrice_destroy(C,Cmat);
    return Cmat;
  }
  static const char _cholesky_s []="cholesky";
  static define_unary_function_eval (__cholesky,&_cholesky,_cholesky_s);
  define_unary_function_ptr5( at_cholesky ,alias_at_cholesky,&__cholesky,0,true);

  gen l2norm(const vecteur & v,GIAC_CONTEXT){
    const_iterateur it=v.begin(),itend=v.end();
    gen res,r,i;
    for (;it!=itend;++it){
      reim(*it,r,i,contextptr);
      res += r*r+i*i;
    }
    return sqrt(res,contextptr);
  }

  matrice gramschmidt(const matrice & m,matrice & r,bool normalize,GIAC_CONTEXT){
    r.clear();
    vecteur v(m);
    int s=int(v.size());
    if (!s)
      return v;
    vecteur sc(1,dotvecteur(*conj(v[0],contextptr)._VECTptr,*v[0]._VECTptr));
    if (is_zero(sc.back()))
      return v;
    vecteur rcol0(s);
    rcol0[0]=1;
    r.push_back(rcol0);
    for (int i=1;i<s;++i){
      gen cl,coeff;
      vecteur rcol(s);
      rcol[i]=1;
      for (int j=0;j<i;++j){
	coeff=rdiv(dotvecteur(*conj(v[j],contextptr)._VECTptr,*v[i]._VECTptr),sc[j],contextptr);
	cl=cl+coeff*v[j];
	rcol[j]=coeff;
      }
      v[i]=v[i]-cl;
      sc.push_back(dotvecteur(*conj(v[i],contextptr)._VECTptr,*v[i]._VECTptr));
      r.push_back(rcol);
      if (is_zero(sc.back(),contextptr))
	break;
    }
    r=mtran(*conj(r,contextptr)._VECTptr); // transconjugate
    if (normalize){
      gen coeff;
      for (int i=0;i<s;++i){
	if (is_zero(sc[i],contextptr))
	  break;
	coeff=sc[i]=sqrt(sc[i],contextptr);
	v[i]=rdiv(v[i],coeff,contextptr);
      }
      for (int i=0;i<s;++i){
	if (is_zero(sc[i],contextptr))
	  break;
	r[i]=sc[i]*r[i];
      }
    }
    return v;
  }

  matrice gramschmidt(const matrice & m,bool normalize,GIAC_CONTEXT){
    matrice r;
    return gramschmidt(m,r,normalize,contextptr);
  }

  // lll decomposition of M, returns S such that S=A*M=L*O

  matrice matpow(const matrice & m,const gen & n,GIAC_CONTEXT){
    identificateur x("x");
    gen ux=symb_pow(x,n);
    return analytic_apply(ux,x,m,contextptr);
  }

      // FIXME: pow should not always call egv stuff
  gen _matpow(const gen & a,GIAC_CONTEXT){
    if ( a.type==_STRNG && a.subtype==-1) return  a;
    if (a.type==_VECT && a._VECTptr->size()==2 && ckmatrix(a._VECTptr->front()))
      return matpow(*a._VECTptr->front()._VECTptr,a._VECTptr->back(),contextptr);
    return gensizeerr(contextptr);
  }
  static const char _matpow_s []="matpow";
  static define_unary_function_eval (__matpow,&_matpow,_matpow_s);
  define_unary_function_ptr5( at_matpow ,alias_at_matpow,&__matpow,0,true);



  // QR reduction, P is orthogonal and should be initialized to identity
  // trn(P)*H=original, Givens method
  void qr_ortho(std_matrix<gen> & H,std_matrix<gen> & P,bool computeP,GIAC_CONTEXT){
    matrix_double H1;
    int n=int(H.size()),lastcol=std::min(n-1,int(H.front().size()));
    gen t,tn,tc,tabs,u,un,uc,tmp1,tmp2,norme;
    vecteur v1,v2;
    for (int m=0;m<lastcol;++m){
      if (debug_infolevel>=5)
	CERR << "// Givens reduction line " << m << endl;
      // check for a non zero coeff in the column m below ligne m
      int i=m;
      gen pivot=0;
      int pivotline=0;
      for (;i<n;++i){
	t=H[i][m];
	tabs=abs(t,contextptr);
	if (is_strictly_greater(tabs,pivot,contextptr)){
	  pivotline=i;
	  pivot=tabs;
	}
      }
      if (is_zero(pivot)) //not found
	continue;
      i=pivotline;
      // exchange lines 
      if (i>m){
	swap(H[i],H[m]);
	swap(P[i],P[m]);
      }
      // now coeff at line m column m is H[m][m]=t!=0
      // creation of zeros in lines i=m+1 and below
      for (i=m+1;i<n;++i){
	// line operation
	t=H[m][m];
	if (is_zero(t)){
	  swap(H[i],H[m]);
	  swap(P[i],P[m]);
	  t=H[m][m];
	}
	u=H[i][m];
	if (is_zero(u))
	  continue;
	uc=conj(u,contextptr);
	tc=conj(t,contextptr);
	norme=sqrt(u*uc+t*tc,contextptr);
	un=u/norme; tn=t/norme; uc=conj(un,contextptr);	tc=conj(tn,contextptr); 
	if (debug_infolevel>=3)
	  CERR << "// i=" << i << " " << u <<endl;
	// H[m]=un*H[i]+tn*H[m] and H[i]=tn*H[i]-un*H[m];
	linear_combination(uc,H[i],tc,H[m],plus_one,1,v1,1e-12,0); 
	linear_combination(tn,H[i],-un,H[m],plus_one,1,v2,1e-12,0); 
	swap(H[m],v1);
	swap(H[i],v2);
	linear_combination(uc,P[i],tc,P[m],plus_one,1,v1,1e-12,0); 
	linear_combination(tn,P[i],-un,P[m],plus_one,1,v2,1e-12,0); 
	swap(P[m],v1);
	swap(P[i],v2);
      }
    }
  }


  ostream & operator << (ostream & os,const vector<giac_double> & m){
    int s=int(m.size());
    os << "[";
    for (int i=0;i<s;++i){
      os << m[i]; 
      if (i+1!=s)
	os << ",";
    }
    return os << "]";
  }

  ostream & operator << (ostream & os,const matrix_double & m){
    int s=int(m.size());
    os << "[";
    for (int i=0;i<s;++i){
      os << m[i] ;
      if (i+1!=s)
	os << ",";
      os << endl;
    }
    return os << "]";
  }

  void matrix_double::dbgprint() const { COUT << *this << std::endl; }

  ostream & operator << (ostream & os,const vector< complex_double > & m){
    int s=int(m.size());
    for (int i=0;i<s;++i)
      os << m[i].real() << "," << m[i].imag() << " ";
    return os;
  }

  ostream & operator << (ostream & os,const matrix_complex_double & m){
    int s=int(m.size());
    for (int i=0;i<s;++i)
      os << m[i] << endl;
    return os;
  }


  // lll decomposition of M, returns S such that S=A*M=L*O
  // L is lower and O is orthogonal
  matrice lll(const matrice & M,matrice & L,matrice & O,matrice &A,GIAC_CONTEXT){
    if (!ckmatrix(M))
      return vecteur(1,gensizeerr(contextptr));
    matrice res(M);
    int n=int(res.size());
    if (!n)
      return res;
    int c=int(res[0]._VECTptr->size());
    if (c<n)
      return vecteur(1,gendimerr(contextptr));
    A=midn(c);
    A=vecteur(A.begin(),A.begin()+n);
    int k=0;
    for (;k<n;){
      if (!k){ // push first vector
	vecteur tmp(c);
	tmp[0]=1;
	L.push_back(tmp);
	O.push_back(res.front());
	++k;
	continue;
      }
      // Find new vector in L,O
      vecteur tmp(c);
      gen Otmp(res[k]);
      for (int j=0;j<k;++j){
	// tmp[j]=dotvecteur(res[j],res[k])/dotvecteur(res[j],res[j]);
	tmp[j]=dotvecteur(conj(O[j],contextptr),Otmp)/dotvecteur(conj(O[j],contextptr),O[j]);
	Otmp=subvecteur(*Otmp._VECTptr,multvecteur(tmp[j],*O[j]._VECTptr));
      }
      tmp[k]=1;
      L.push_back(tmp);
      O.push_back(Otmp);
      // Compare norm of O[k] and O[k-1]
      for (int j=k-1;j>=0;--j){
	gen alpha=dotvecteur(conj(O[j],contextptr),res[k])/dotvecteur(conj(O[j],contextptr),O[j]);
	alpha=_round(alpha,contextptr);
	res[k]=subvecteur(*res[k]._VECTptr,multvecteur(alpha,*res[j]._VECTptr));
	A[k]=subvecteur(*A[k]._VECTptr,multvecteur(alpha,*A[j]._VECTptr));
	L[k]=subvecteur(*L[k]._VECTptr,multvecteur(alpha,*L[j]._VECTptr));
      }
      gen lastalpha=dotvecteur(conj(O[k-1],contextptr),res[k])/dotvecteur(conj(O[k-1],contextptr),O[k-1]);
      if (ck_is_greater(dotvecteur(conj(O[k],contextptr),O[k]),(gen(3)/4-lastalpha*lastalpha)*dotvecteur(conj(O[k-1],contextptr),O[k-1]),contextptr)){
	// Ok, continue the reduction
	++k;
      }
      else {
	swapgen(res[k],res[k-1]);
	swapgen(A[k],A[k-1]);
	--k;
	L.pop_back();
	L.pop_back();
	O.pop_back();
	O.pop_back();
      }
    }
    return res;
  }
  matrice lll(const matrice & m,GIAC_CONTEXT){
    matrice L,O,A;
    return lll(m,L,O,A,contextptr);
  }
  gen _lll(const gen & g,GIAC_CONTEXT){
    if ( g.type==_STRNG && g.subtype==-1) return  g;
    if (g.type!=_VECT)
      return gensizeerr(contextptr);
    matrice L,O,A;
    matrice S=lll(*g._VECTptr,L,O,A,contextptr);
    return makesequence(S,A,L,O);
  }
  static const char _lll_s []="lll";
  static define_unary_function_eval (__lll,&_lll,_lll_s);
  define_unary_function_ptr5( at_lll ,alias_at_lll,&__lll,0,true);
 
  void matrix_complex_double::dbgprint() const { COUT << *this << std::endl; }

  bool convert(const vecteur & v,vector<giac_double> & v1,bool crunch){
    int n=int(v.size());
    v1.clear();
    v1.reserve(n);
    for (int i=0;i<n;++i){
      if (v[i].type==_INT_){
	v1.push_back(v[i].val);
	continue;
      }
#if 0
      if (v[i].type==_FLOAT_){
	v1.push_back(get_double(v[i]._FLOAT_val));
	continue;
      }
#endif
      if (v[i].type==_ZINT){
	v1.push_back(mpz_get_d(*v[i]._ZINTptr));
	continue;
      }
#ifdef HAVE_LIBMPFR
      if (crunch && v[i].type==_REAL){
	v1.push_back(mpfr_get_d(v[i]._REALptr->inf,GMP_RNDN));
	continue;
      }
      if (crunch && v[i].type==_FRAC){
	gen g=accurate_evalf(v[i],60);
	if (g.type!=_REAL)
	  return false;
	v1.push_back(mpfr_get_d(g._REALptr->inf,GMP_RNDN));
	continue;
      }
#else
      if (crunch && v[i].type==_FRAC){
	gen g=evalf_double(v[i],1,context0);
	if (g.type!=_DOUBLE_)
	  return false;
	v1.push_back(g._DOUBLE_val);
	continue;
      }
#endif
      if (v[i].type!=_DOUBLE_)
	return false;
      v1.push_back(v[i]._DOUBLE_val);
    }
    return true;
  }

  bool convert(const vecteur & v,vector< complex_double > & v1,bool crunch){
    int n=int(v.size());
    v1.clear();
    v1.reserve(n);
    for (int i=0;i<n;++i){
      if (v[i].type==_INT_){
	v1.push_back(double(v[i].val));
	continue;
      }
#if 0
      if (v[i].type==_FLOAT_){
	v1.push_back(get_double(v[i]._FLOAT_val));
	continue;
      }
#endif
      if (v[i].type==_CPLX){
	gen r=evalf_double(*v[i]._CPLXptr,1,context0);
	gen im=evalf_double(*(v[i]._CPLXptr+1),1,context0);
	if (r.type!=_DOUBLE_ || im.type!=_DOUBLE_)
	  return false;
	v1.push_back(complex_double(r._DOUBLE_val,im._DOUBLE_val));
	continue;
      }
      if (v[i].type!=_DOUBLE_)
	return false;
      v1.push_back(v[i]._DOUBLE_val);
    }
    return true;
  }

  double complex_abs(const complex_double & c){
#if defined EMCC || defined FXCG
    double r=c.real(),i=c.imag();
    r=std::sqrt(r*r+i*i);
    return r;
#else
    return std::abs(c);
#endif
  }

  double complex_long_abs(const complex_long_double & c){
    return complex_abs(c);
  }
  
#ifndef NO_NAMESPACE_GIAC
} // namespace giac
#endif // ndef NO_NAMESPACE_GIAC
