// -*- mode:C++ ; compile-command: "g++-3.4 -I.. -g -c quater.cc -DHAVE_CONFIG_H -DIN_GIAC" -*-
#include "giacPCH.h"
/*
 *  Copyright (C) 2001,2014 B. Parisse, Institut Fourier, 38402 St Martin d'Heres
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

#include "quater.h"
#include "unary.h"
#include "sym2poly.h"
#include "usual.h"
#include "intg.h"
#include "subst.h"
#include "derive.h"
#include "lin.h"
#include "vecteur.h"
#include "gausspol.h"
#include "plot.h"
#include "prog.h"
#include "modpoly.h"
#include "series.h"
#include "tex.h"
#include "ifactor.h"
#include "risch.h"
#include "solve.h"
#include "modfactor.h"
#include "giacintl.h"
using namespace std;

#ifndef NO_NAMESPACE_GIAC
namespace giac {
#endif // ndef NO_NAMESPACE_GIAC

#ifdef NO_RTTI
  extern const unary_function_ptr * const  at_quaternion=0; // user-level quaterni
#else
  static const char _quaternion_s []="quaternion";
  gen _quaternion(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    if (args.type!=_VECT)
      return quaternion(args);
    vecteur v(*args._VECTptr);
    if (v.size()==1)
      return quaternion(v.front());
    if (v.size()!=4)
      return gensizeerr(gettext("Quaternion has 1 or 4 arguments"));
    return quaternion(v[0],v[1],v[2],v[3]);
  }
  static define_unary_function_eval (__quaternion,&_quaternion,_quaternion_s);
  define_unary_function_ptr5( at_quaternion ,alias_at_quaternion,&__quaternion,0,true); // auto-register
  
  string quaternion::print(GIAC_CONTEXT) const {
    return string(_quaternion_s)+"("+r.print()+","+i.print()+","+j.print()+","+k.print()+")";
    this->dbgprint();
  }

  quaternion::quaternion(const gen & g){
    if (g.type==_USER){
      const quaternion  * q =dynamic_cast<const quaternion *>(g._USERptr);
      if (q)
	*this=*q;
    }
    else {
      r=g;
      i=zero;
      j=zero;
      k=zero;
    }
  };

  static const char _galois_field_s []="GF";

  void lrdm(modpoly & p,int n); // form intg.cc

  // Is the polynomial v irreducible and primitive modulo p?
  // If it is only irreducible, returns 2 and sets vmin to a primitive poly mod p if primitive is true
  int is_irreducible_primitive(const modpoly & v,const gen & p,modpoly & vmin,int primitive,GIAC_CONTEXT){
    vmin=v;
    int m=int(v.size())-1;
    if (m<2)
      return 0; // setsizeerr(gettext("irreducibility: degree too short")+gen(v).print());
    gen gpm=pow(p,m);
    int pm=gpm.type==_INT_?gpm.val:RAND_MAX; // max number of tries
    environment E;
    environment * env=&E;
    env->modulo=p;
    env->pn=env->modulo;
    env->moduloon=true;
    vecteur polyx(2),g;
    polyx[0]=1;
    vecteur test(2);
    test[0]=1;
    // Irreducible: v must be prime with x^(p^k)-x, for k<=m/2
    for (int k=1;k<=m/2;k++){
      test=powmod(test,p,v,env);
      gcdmodpoly(operator_minus(test,polyx,env),v,env,g);
      if (!is_one(g)){
	return 0;
      }
    }
    if (primitive!=1){
      if (primitive==0)
	*logptr(contextptr) << gettext("Warning, minimal polynomial is only irreducible, not necessarily primitive") << '\n';
      return 3;
    }
    // Primi: must not divide x^[(p^m-1)/d]-1 for any prime divisor d of p^m-1
    gen pm_minus_1=gpm-1;
    vecteur vp(pfacprem(pm_minus_1,true,context0));
    int ntest=int(vp.size());
    for (int i=0;i<ntest;i+=2){
      // Compute x^[(p^m-1)/d] mod v, mod p, is it 1?
      gen pm_d=(gpm-1)/vp[i];
      test=powmod(polyx,pm_d,v,env);
      if (is_one(test)){
	vecteur cyclic;
	// Find a cyclic element in GF, [1,0] does not work
	for (int k=p.val+1;k<pm;k++){
	  cyclic=vecteur(m);
#if 0
	  // decompose k in base p -> REPLACED by random init
	  int k1=k;
	  for (int j=0;j<m;++j){
	    cyclic[j]=k1%p.val;
	    k1/=p.val;
	  }
#else
	  for (int j=0;j<m;++j)
	    cyclic[j]=giac_rand(context0) % p.val;
#endif
	  cyclic=trim(cyclic,0);
	  // ?cyclic
	  for (int i=0;i<ntest;i+=2){
	    gen pm_over_d=(gpm-1)/vp[i];
	    test=powmod(cyclic,pm_over_d,v,env);
	    if (is_one(test))
	      break; // not cyclic
	  }
	  if (!is_one(test)) // cyclic! 
	    break;
	}
	// cyclic is cyclic, find it's minimal polynomial
	// Compute 1,cyclic, ..., cyclic^m and find kernel 
	matrice minmat(m+1);
	minmat[0]=vecteur(1,1);
	for (int i=1;i<=m;++i)
	  minmat[i]=operator_mod(operator_times(cyclic,*minmat[i-1]._VECTptr,env),v,env);
	for (int i=0;i<=m;++i)
	  lrdm(*minmat[i]._VECTptr,m-1);
	minmat=mtran(minmat);
	matrice minred,pivots; gen det;
	if (!modrref(minmat,minred,pivots,det,0,m,0,m+1,true,0,p,false,0))
	  return 0;
	// Extract kernel from last column
	vmin=vecteur(m+1,1);
	for (int i=1;i<=m;++i)
	  vmin[i]=-minred[m-i][m];
	// vecteur tmpv;
	// COUT << is_irreducible_primitive(vmin,p,tmpv) << '\n';
	return 2;
      }
      /* vecteur test(pm_d+1);
	 test[0]=1;
	 test[pm_d]=-1; 
      if (is_zero(operator_mod(test,v,env))){
	return false;
      }
      */
    }
    return 1;
  }

  vecteur find_irreducible_primitive(const gen & p_,int m,bool primitive,GIAC_CONTEXT){
    if (p_.type==_ZINT){
#ifdef HAVE_LIBPARI
      gen pari=pari_ffinit(p_,m);
      pari=unmod(pari);
      if (pari.type==_VECT)
	return *pari._VECTptr;
#else
      return vecteur(1,gensizeerr("Compile with PARI for characteristic>2^31"));
#endif
    }
    int p=p_.val;
    if (p==2 && m==8)
      return makevecteur(1,0,0,0,1,1,1,0,1); // GF(2,8)
    // First check M random polynomials
    int M=100*m;
    // start coefficients near the end if only irreducible minpoly required
    // this make division by minpoly faster
    int start=primitive?1:m-2,nextpow2=1;
    for (int k=0;k<M;++k){
      if (start>1 && k==nextpow2){
	--start;
	nextpow2 *=2;
      }
      vecteur test(m+1),test2;
      test[0]=1;
      // random
      for (int j=start;j<=m;++j){
	if (p==2)
	  test[j]=(giac_rand(contextptr)>>29)%2;
	else
	  test[j]=giac_rand(contextptr)%p;
      }
      // *logptr(contextptr) << test << '\n';
      if (is_irreducible_primitive(test,p,test2,primitive?1:0,contextptr))
	return test2;
    }
    *logptr(contextptr) << gettext("Warning, random search for irreducible polynomial did not work, starting exhaustive search") << '\n';
    // Now test all possible coeffs for test[k] until it's irreducible
    double pm=std::pow(double(p),double(m));
    for (int k=0;k<pm;k++){
      vecteur test(m+1),test2;
      test[0]=1;
      // decompose k in base p
      for (int j=1,k1=k;j<=m;++j){
	test[j]=k1%p;
	k1/=p;
      }
      if (is_irreducible_primitive(test,p,test2,primitive?1:0,contextptr))
	return test2;
    }
    return vecteur(1,gensizeerr(gettext("No irreducible primitive polynomial found")));
  }
  bool make_free_variable(gen & g,GIAC_CONTEXT,bool warn,gen k,gen K){
    if (g.type!=_IDNT)
      return false;
    string s(g.print(contextptr));
    while (g==k || g==K || eval(g,1,contextptr)!=g){
      if (warn)
	*logptr(contextptr) << g << gettext(" already assigned. Trying ");
      autoname_plus_plus(s);
      if (warn)
	*logptr(contextptr) << s << '\n';
      g=identificateur(s);
    }
    return true;
  }
  int gfsize(const gen & P){
    if (P.type==_INT_){
      int res=0;
      for (int tmp=P.val;tmp;tmp/=2){
	res++;
      }
      return res-1;
    }
    if (P.type!=_VECT)
      return 0;
    return P._VECTptr->size()-1;
  }

  gen _galois_field(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    vecteur v;
    if (is_integer(args)){ // must be a power of a prime
      if (_isprime(args,contextptr)!=0){
	return gensizeerr(gettext("GF is used for non-prime finite field. Use % or mod for prime fields, e.g. 1 % ")+args.print(contextptr)+'.');
      }
      v.push_back(args);
    }
    else {
      if (args.type!=_VECT){
	vecteur lv=lvar(args);
	if (!lv.empty()){
	  gen tmp=_e2r(makesequence(args,lv[0]),contextptr),modulo;
	  if (tmp.type==_VECT){
	    if (has_mod_coeff(tmp,modulo)){
	      return _galois_field(makesequence(modulo,args),contextptr);
	    }
	  }
	}
	return galois_field(args,true,contextptr);
      }
      v=*args._VECTptr;
    }
    int s=int(v.size());
    if (s<1 || !is_integer(v[0]))
      return gensizeerr(contextptr);
    if (_isprime(v[0],contextptr)==0){
      gen pm=abs(v[0],contextptr); // ok
      vecteur u(pfacprem(pm,true,contextptr));
      if (u.size()!=2)
	return gensizeerr(gettext("Not a power of a prime"));
      v[0]=u[0];
      v.insert(v.begin()+1,u[1]);
      ++s;
    }
    if (s==3 && v[1].type!=_INT_){
      v.push_back(undef);
      ++s;
    }
    bool primitive=true;
    if (s>=3 && is_integer(v[0]) && is_zero(v.back())){
      primitive=false;
      v.pop_back();
      --s;
    }
    if (s==2 && v[1].type!=_INT_){
      vecteur l=lvar(v[1]);
      if (l.size()!=1)
	return gensizeerr(contextptr);
      v.push_back(l.front());
      primitive=false;
      ++s;
    }
    if (s==2){
#ifdef GIAC_HAS_STO_38
      gen k(identificateur("v")),g(identificateur("g")),K(identificateur("k"));
#else
      gen k(k__IDNT_e),g(g__IDNT_e),K(identificateur("K"));
#endif
      make_free_variable(k,contextptr,false,0,0);
      make_free_variable(K,contextptr,false,k,0);
      make_free_variable(g,contextptr,true,k,K);
      v.push_back(makevecteur(k,K,g));
      *logptr(contextptr) << gettext("Setting ") << g << gettext(" as generator for Galois field ") << K << '\n' << gettext("(auxiliary polynomial variable for addition representation ") << k << ")" << '\n';
      ++s;
    }
    if (s==3){
      gen fieldvalue;
      if (v[2].type==_IDNT){
	gen k(k__IDNT_e),g(v[2]),K(identificateur("K"));
	if (k==g)
	  k=l__IDNT_e;
	make_free_variable(k,contextptr,false,0,0);
	make_free_variable(K,contextptr,false,k,0);
	// make_free_variable(g,contextptr,true,k,K);
	if (v[1].type==_SYMB){
	  gen P=unmod(_e2r(makesequence(v[1],v[2]),contextptr));
	  if (P.type==_VECT){
	    vecteur vmin; // not used
	    int res=is_irreducible_primitive(*P._VECTptr,v[0],vmin,0,contextptr);
	    if (res==0)
	      return gensizeerr("Polynomial "+v[1].print(contextptr)+" is not irreducible modulo "+v[0].print(contextptr));
	    fieldvalue=galois_field(v[0],smod(P,v[0]),v[2],undef);
	  }
	}
	v[2]=makevecteur(k,K,g);
	if (fieldvalue.type==_USER){
	  galois_field *gf=dynamic_cast<galois_field *>( fieldvalue._USERptr);
	  gf->x=v[2];
	}
      }
      if (fieldvalue.type!=_USER)
	fieldvalue=galois_field(gen(v,args.subtype),primitive,contextptr);
      if (v.back().type==_VECT && v.back()._VECTptr->size()==3 && fieldvalue.type==_USER){
	// assign field and generator
	gen K=(*v.back()._VECTptr)[1];
	gen g=(*v.back()._VECTptr)[2];
	gen k=(*v.back()._VECTptr)[0];
	galois_field *gf=dynamic_cast<galois_field *>( fieldvalue._USERptr);
	*logptr(contextptr) << gettext("Assigning variables ") << g << gettext(" and ") << K << '\n';
	*logptr(contextptr) << gettext("Now e.g. ") << g << gettext("^200+1 will build an element of ") << K << '\n';
	sto(fieldvalue,K,contextptr);
	gen gene=galois_field(gf->p,gf->P,gf->x,makevecteur(1,0));
	sto(gene,g,contextptr);
	gen_context_t genec={gene,(context *)contextptr};
	gf_list()[pow(gf->p,gfsize(gf->P),contextptr)]=genec;
	return fieldvalue;
      }
      return fieldvalue;
    }
    if (s!=4)
      return gensizeerr(gettext("galois_field has 1 or 4 arguments (charac p, irred poly P, var name x, value as a poly of x or as a vector)"));
    vecteur a,P,vmin;
    gen & x= v[2];
    gen xid(x);
    if (x.type==_VECT && !x._VECTptr->empty())
      xid=x._VECTptr->front();
    if (!is_undef(v[3]) && v[3].type!=_VECT)
      v[3]=_e2r(makesequence(v[3],xid),contextptr); // ok
    if (v[1].type!=_VECT)
      v[1]=_e2r(makesequence(v[1],xid),contextptr); // ok
    v[1]=unmod(v[1]);
    if (v[1].type!=_VECT)
      return gensizeerr();
    if (is_undef(v[3])){
      int res=is_irreducible_primitive(*v[1]._VECTptr,v[0],vmin,primitive?1:0,contextptr);
      if (!res)
	return gensizeerr(gettext("Not irreducible or not primitive polynomial")+args.print());
      if (res==2)
	*logptr(contextptr) << gettext("Warning ") << symb_horner(*v[1]._VECTptr,xid) << gettext(" is irreducible but not primitive. You could use ") << symb_horner(vmin,xid) << gettext(" instead ") << '\n';
    }
    return galois_field(v[0],smod(v[1],v[0]),v[2],v[3]);
  }
  static define_unary_function_eval (__galois_field,&_galois_field,_galois_field_s);
  define_unary_function_ptr5( at_galois_field ,alias_at_galois_field,&__galois_field,0,true); // auto-register

  gen char2_uncoerce(const gen & a){
    if (a.type==_VECT)
      return *a._VECTptr;
    if (a.type!=_INT_)
      return undef;
    int ai=a.val;
    vecteur res;
    for (;ai;ai/=2)
      res.push_back(ai%2);
    reverse(res.begin(),res.end());
    return res;
  }

  string galois_field::print(GIAC_CONTEXT) const {
    gen xid(x);
    gen A(a);
    if (a.type==_INT_){
      int ai=a.val;
      if (!ai)
	return "0";
      // if (ai==1) return "1";
      A=char2_uncoerce(a);
    }
    if (x.type==_VECT && x._VECTptr->size()>=2){
      xid=x._VECTptr->front();
      if (!is_undef(a)){
	if (x._VECTptr->size()==3 && A.type==_VECT){
	  if (A._VECTptr->size()==1)
	    return '('+makemod(A._VECTptr->front(),p).print(contextptr)+')';
	  gen tmp=symb_horner(*A._VECTptr,x._VECTptr->back());
	  if (tmp.is_symb_of_sommet(at_plus))
	    return '('+tmp.print(contextptr)+')';
	  else
	    return tmp.print(contextptr);
	}
	return x._VECTptr->back().print()+"("+r2e(A,xid,contextptr).print()+")";      
      }
    }
    return string(_galois_field_s)+"("+p.print()+","+r2e(unmod(P),xid,contextptr).print()+","+x.print()+","+r2e(A,xid,contextptr).print()+")";
    this->dbgprint(); // not reached, it's for the debugger
    return "";
  }

  gen galois_field::giac_constructor(GIAC_CONTEXT) const {
    return symbolic(at_galois_field,makesequence(p,P,x,a));
  }

  string galois_field::texprint(GIAC_CONTEXT) const {
    gen xid(x);
    gen A(a);
    if (a.type==_INT_){
      int ai=a.val;
      if (!ai)
	return "0";
      // if (ai==1) return "1";
      A=char2_uncoerce(a);
    }
    if (x.type==_VECT && x._VECTptr->size()>=2){
      xid=x._VECTptr->front();
      if (!is_undef(a)){
	if (x._VECTptr->size()==3 && A.type==_VECT){
	  if (A._VECTptr->size()==1)
	    return makemod(A._VECTptr->front(),p).print(contextptr);
	  gen tmp=symb_horner(*A._VECTptr,x._VECTptr->back());
	  if (tmp.is_symb_of_sommet(at_plus))
	    return '('+gen2tex(tmp,contextptr)+')';
	  else
	    return gen2tex(tmp,contextptr);
	}
	return gen2tex(x._VECTptr->back(),contextptr)+"("+gen2tex(r2e(A,xid,contextptr),contextptr)+")";      
      }
    }
    return string(_galois_field_s)+"("+gen2tex(p,contextptr)+","+gen2tex(r2e(P,xid,contextptr),contextptr)+","+gen2tex(x,contextptr)+","+gen2tex(r2e(A,xid,contextptr),contextptr)+")";
  }

  galois_field::galois_field(const gen & g,bool primitive,GIAC_CONTEXT){
    if (g.type==_USER){
      const galois_field  * q =dynamic_cast<const galois_field *>(g._USERptr);
      if (q)
	*this=*q;
      else {
	P=gensizeerr(gettext("Unable to convert user type to galois field"));
      }
    }
    else {
      if (g.type!=_VECT || g._VECTptr->size()<2 || !is_integer(g._VECTptr->front()) || (*g._VECTptr)[1].type!=_INT_)
	P=gensizeerr(gettext("Expecting characteristic p, integer m"));
      else {
	gen p0=g._VECTptr->front(); // max(absint(),2);
	if (is_greater(1,p0,contextptr))
	  P=gensizeerr(gettext("Bad characteristic: ")+p0.print(contextptr));
	else {
	  int m0=(*g._VECTptr)[1].val; // max(absint(),2);
	  if (m0<2)
	    P=gensizeerr(gettext("Exponent must be >=2: ")+print_INT_(m0));
	  else {
	    p=p0;
	    P=find_irreducible_primitive(p0,m0,primitive,contextptr);
	    P=smod(P,this->p);
	    if (g._VECTptr->size()>2)
	      x=(*g._VECTptr)[2];
	    else {
	      x=vx_var;
	    }
	    a=undef;
	  }
	}
      }
    }
  };
  
  void galois_field::reduce(){
    if (!is_undef(a)){
#if 1 // compact representation of GF(2,n) for n<=30
      if (p.type==_INT_ && p.val==2){
	if (P.type==_VECT && P._VECTptr->size()<=30)
	  P=horner(P,2);
	if (P.type==_INT_ && a.type==_VECT)
	  a=horner(a,2);
	return;
      }
#endif
      a = smod(a,p);
      if (a.type!=_VECT)
	a=gen(vecteur(1,a),_POLY1__VECT);
      else 
	trim(*a._VECTptr);
    }
  }

  galois_field::galois_field(const gen p_,const gen & P_,const gen & x_,const gen & a_,bool doreduce):p(p_),P(P_),x(x_),a(a_) {
    if (doreduce)
      reduce();
  }

  galois_field::galois_field(const galois_field & q,bool doreduce):p(q.p),P(q.P),x(q.x),a(q.a) { 
    if (doreduce)
      reduce();
  }

  // find common GF, returns 0 if impossible, 1 if same, 2 if extension created
  int common_gf(galois_field & a,galois_field & b){
    if (a.p!=b.p || is_undef(a.P) || is_undef(b.P))
      return 0;
    if (a.P==b.P)
      return 1;
    gfmap & l=gf_list();
    gen_context_t agc=l[pow(a.p,gfsize(a.P),context0)],bgc=l[pow(b.p,gfsize(b.P),context0)];
    if (agc.ptr!=bgc.ptr || !equalposcomp(context_list(),agc.ptr))
      return 0;
    gen ag=agc.g, bg=bgc.g;
    context * contextptr=agc.ptr;
    if (ag.type==_USER && bg.type==_USER){
      galois_field * agptr=dynamic_cast<galois_field *>(ag._USERptr);
      galois_field * bgptr=dynamic_cast<galois_field *>(bg._USERptr);
      if (!agptr || !bgptr) 
	return 0;
      galois_field * tmpptr=agptr;
      if (agptr->P!=bgptr->P){
	// uncoerce in char 2
	gen Pa(char2_uncoerce(agptr->P));
	gen Pb(char2_uncoerce(bgptr->P));
	// build a common field extension
	int as=gfsize(a.P),bs=gfsize(b.P);
	int cs=lcm(as,bs).val;
	gen tmp;
	if (cs==as)
	  tmp=ag;
	if (cs==bs)
	  tmp=bg;
	if (cs!=as && cs!=bs){
	  // GF(a.p,cs), then factor a.P and b.P over GF 
	  // select one root as new generators for a.GF and b.GF
	  // update gf_list, a and b
	  *logptr(contextptr) << "Creating common field extension GF(" << a.p << "," << cs << ")" << '\n';
	  tmp=_galois_field(makesequence(a.p,cs),contextptr);
	}
	if (tmp.type!=_USER || !(tmpptr=dynamic_cast<galois_field *>(tmp._USERptr)))
	  return 0;     
	*logptr(contextptr) << "Minimal polynomial of field generator " << symb_horner(*tmpptr->P._VECTptr,tmpptr->x[2]) << '\n';
	tmp=galois_field(tmpptr->p,tmpptr->P,tmpptr->x,makevecteur(1,0),false); // field generator
	if (Pa.type==_VECT && Pb.type==_VECT){
	  polynome A; factorization f;
	  if (cs>as){
	    poly12polynome(*Pa._VECTptr,1,A,1);
	    gen af=tmpptr->polyfactor(A,f);
	    A=f.front().fact;
	    if (A.lexsorted_degree()!=1)
	      return 0;
	    ag=-A.coord.back().value/A.coord.front().value;
	    *logptr(contextptr) << "GF(" << a.p << "," << gfsize(a.P) << ") generator maps to " << ag << '\n';
	    agc.g=ag;
	    l[pow(a.p,gfsize(a.P),context0)]=agc;
	    f.clear();
	  }
	  if (cs>bs){
	    poly12polynome(*Pb._VECTptr,1,A,1);
	    gen af=tmpptr->polyfactor(A,f);
	    A=f.front().fact;
	    if (A.lexsorted_degree()!=1)
	      return 0;
	    bg=-A.coord.back().value/A.coord.front().value;
	    *logptr(contextptr) << "GF(" << b.p << "," << gfsize(b.P) << ") generator maps to " << bg << '\n';
	    bgc.g=bg;
	    l[pow(b.p,gfsize(b.P),context0)]=bgc;
	  }
	}
      }
      ag=horner(char2_uncoerce(a.a),ag);
      if (ag.type!=_USER)
	ag=galois_field(tmpptr->p,tmpptr->P,tmpptr->x,vecteur(1,ag),false);
      bg=horner(char2_uncoerce(b.a),bg);
      if (bg.type!=_USER)
	bg=galois_field(tmpptr->p,tmpptr->P,tmpptr->x,vecteur(1,bg),false);
      if (ag.type==_USER && bg.type==_USER){
	agptr=dynamic_cast<galois_field *>(ag._USERptr);
	bgptr=dynamic_cast<galois_field *>(bg._USERptr);
	a=*agptr;
	b=*bgptr;
	return 2;
      }     
    }
    return 0;
  }

  void gf_add(const vecteur & a,const vecteur &b,int p,vecteur & c){
    int n=int(a.size()),m=int(b.size());
    if (n<m){
      gf_add(b,a,p,c);
      return;
    }
    c.clear(); c.reserve(n);
    const_iterateur it=a.begin(),itend=a.end(),jt=b.begin();
    if (n>m){
      for (;n>m;--n,++it)
	c.push_back(*it);
    }
    else {
      for (;it!=itend;++it,++jt){
	int j=it->val+jt->val;
	j += (unsigned(j)>>31)*p; // make positive
	j -= (unsigned((p>>1)-j)>>31)*p;
	if (j){
	  c.push_back(j);
	  ++it;++jt;
	  break;
	}
      }
    }
    for (;it!=itend;++it,++jt){
      int j=it->val+jt->val;
      j += (unsigned(j)>>31)*p; // make positive
      j -= (unsigned((p>>1)-j)>>31)*p;
      c.push_back(j);
    }
  }

  // remove % p in MOD elements of g
  gen cleanup(const gen & p,const gen & g){
    if (g.type==_VECT){
      vecteur v=*g._VECTptr;
      for (int i=0;i<v.size();++i)
	v[i]=cleanup(p,v[i]);
      return gen(v,g.subtype);
    }
    if (g.type==_MOD){
      if (p!=*(g._MODptr+1))
	return gensizeerr(gettext("Incompatible characteristics"));
      return *g._MODptr;
    }
    if (g.type==_SYMB)
      return symbolic(g._SYMBptr->sommet,cleanup(p,g._SYMBptr->feuille));
    return g;
  }
  inline gen cleanup(const galois_field & gf,const gen & g){
    return cleanup(gf.p,g);
  }

  gen galois_field::operator + (const gen & g) const { 
    bool char2=a.type==_INT_ && p.type==_INT_ && p.val==2;
    if (is_integer(g)){
      if (char2){
	if (g.type==_ZINT?modulo(*g._ZINTptr,2)%2:g%2==0)
	  return *this;
	return galois_field(p,P,x,a.val ^ 1);
      }
      return galois_field(p,P,x,a+g);
    }
    if (g.type==_MOD){
      if (*(g._MODptr+1)!=p)
	return gensizeerr(gettext("Incompatible characteristics"));
      return *this+*g._MODptr; // galois_field(p,P,x,a+*g._MODptr)
    }
    if (g.type!=_USER)
      return sym_add(*this,cleanup(*this,g),context0); // ok symbolic(at_plus,makesequence(g,*this));
    if (galois_field * gptr=dynamic_cast<galois_field *>(g._USERptr)){
      if (char2 && gptr->a.type==_INT_){
	if (P==gptr->P)
	  return galois_field(p,P,x,a.val ^ gptr->a.val);
	return galois_field(p,P,x,char2_uncoerce(a),false)+galois_field(gptr->p,gptr->P,gptr->x,char2_uncoerce(gptr->a),false);
      }
      // if (gptr->p!=p || gptr->P!=P || is_undef(P) || is_undef(gptr->P)) return gensizeerr();
      if (a.type==_VECT && gptr->a.type==_VECT){
	galois_field * gfptr=new galois_field(p,P,x,new ref_vecteur(*a._VECTptr),false);
	int tst=common_gf(*gfptr,*gptr);
	if (!tst)
	  return gensizeerr();
	if (tst==1 && p.type==_INT_){
	  ref_gen_user * resptr=new ref_gen_user(*gfptr);
	  gf_add(*a._VECTptr,*gptr->a._VECTptr,p.val,*gfptr->a._VECTptr);
	  delete gfptr;
	  return resptr;
	}
	gen & A=gfptr->a;
	gen & B=gptr->a;
	if (A.type==_INT_ && B.type==_INT_)
	  A.val ^= B.val;
	else {
	  A=char2_uncoerce(A);
	  B=char2_uncoerce(B);
	  environment env;
	  env.modulo=p;
	  env.pn=env.modulo;
	  env.moduloon=true;
	  addmodpoly(*A._VECTptr,*B._VECTptr,&env,*gfptr->a._VECTptr);
	}
	// CERR << gfptr->P << " " << gfptr->a << '\n';	
	gen res=*gfptr;
	delete gfptr;
	return res;
      }
      return galois_field(p,P,x,a+gptr->a);
    }
    else
      return gensizeerr();
  }

  gen galois_field::operator - (const gen & g) const { 
    if (p.type==_INT_ && p.val==2)
      return *this + g;
    if (is_integer(g)){
      gen tmp=a-g;
      if (is_exactly_zero(tmp))
	return tmp;
      return galois_field(p,P,x,tmp);
    }
    if (g.type==_MOD){
      if (*(g._MODptr+1)!=p)
	return gensizeerr(gettext("Incompatible characteristics"));
      return *this-*g._MODptr;//galois_field(p,P,x,a-*g._MODptr);
    }
    if (g.type!=_USER)
      return sym_add(*this,cleanup(*this,-g),context0); // ok symbolic(at_plus,makesequence(-g,*this));
    if (galois_field * gptr=dynamic_cast<galois_field *>(g._USERptr)){
      return *this+(-g);
      // inactive code
      if (gptr->p!=p || gptr->P!=P || is_undef(P) || is_undef(gptr->P)) return gensizeerr();
      if (a.type==_VECT && gptr->a.type==_VECT){
	vecteur res;
	environment env;
	env.modulo=p;
	env.pn=env.modulo;
	env.moduloon=true;
	submodpoly(*a._VECTptr,*gptr->a._VECTptr,&env,res);
	return galois_field(p,P,x,res,false);
      }
      return galois_field(p,P,x,a-gptr->a);
    }
    else
      return gensizeerr();
  }

  gen galois_field::operator - () const { 
    if (p.type==_INT_ && p.val==2)
      return *this;
    return galois_field(p,P,x,-a,true);
  }

  gen galois_field::operator / (const gen & g) const { 
    bool char2=p.type==_INT_ && p.val==2 && a.type==_INT_;
    if (is_integer(g)){
      if (char2){
	if (g.type==_ZINT?modulo(*g._ZINTptr,2)==0:g.val%2==0)
	  return undef;
	return *this;
      }
      gen tmp=invmod(g,p);
      return (*this)*tmp;
    }
    gen tmp=g.inverse(context0);
    return (*this)*tmp;
  }

  // number of bits of tmp
  inline int nbits(int a){
    int N=0,tmp=a>>16;
    if (tmp){
      N+=16;
      a=tmp;
    }
    tmp=a>>8;
    if (tmp){
      N+=8;
      a=tmp;
    }
    tmp=a>>4;
    if (tmp){
      N+=4;
      a=tmp;
    }
    const int tab[16]={0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4};
    N += tab[a];
    return N;
  }

  const unsigned char gf28_tab2add[]={0,1,2,4,8,16,32,64,128,29,58,116,232,205,135,19,38,76,152,45,90,180,117,234,201,143,3,6,12,24,48,96,192,157,39,78,156,37,74,148,53,106,212,181,119,238,193,159,35,70,140,5,10,20,40,80,160,93,186,105,210,185,111,222,161,95,190,97,194,153,47,94,188,101,202,137,15,30,60,120,240,253,231,211,187,107,214,177,127,254,225,223,163,91,182,113,226,217,175,67,134,17,34,68,136,13,26,52,104,208,189,103,206,129,31,62,124,248,237,199,147,59,118,236,197,151,51,102,204,133,23,46,92,184,109,218,169,79,158,33,66,132,21,42,84,168,77,154,41,82,164,85,170,73,146,57,114,228,213,183,115,230,209,191,99,198,145,63,126,252,229,215,179,123,246,241,255,227,219,171,75,150,49,98,196,149,55,110,220,165,87,174,65,130,25,50,100,200,141,7,14,28,56,112,224,221,167,83,166,81,162,89,178,121,242,249,239,195,155,43,86,172,69,138,9,18,36,72,144,61,122,244,245,247,243,251,235,203,139,11,22,44,88,176,125,250,233,207,131,27,54,108,216,173,71,142};
  const unsigned char gf28_tab2mult[]={0,1,2,26,3,51,27,199,4,224,52,239,28,105,200,76,5,101,225,15,53,142,240,130,29,194,106,249,201,9,77,114,6,139,102,48,226,37,16,34,54,148,143,219,241,19,131,70,30,182,195,126,107,40,250,186,202,155,10,121,78,229,115,167,7,192,140,99,103,222,49,254,227,153,38,180,17,146,35,137,55,209,149,207,144,151,220,190,242,211,20,93,132,57,71,65,31,67,183,164,196,73,127,111,108,59,41,85,251,134,187,62,203,95,156,160,11,22,122,44,79,213,230,173,116,244,168,88,8,113,193,248,141,129,100,14,104,75,223,238,50,198,255,25,228,166,154,120,39,185,181,125,18,69,147,218,36,33,138,47,56,64,210,92,150,189,208,206,145,136,152,179,221,253,191,98,243,87,212,172,21,43,94,159,133,61,58,84,72,110,66,163,32,46,68,217,184,124,165,119,197,24,74,237,128,13,112,247,109,162,60,83,42,158,86,171,252,97,135,178,188,205,63,91,204,90,96,177,157,170,161,82,12,246,23,236,123,118,45,216,80,175,214,234,231,232,174,233,117,215,245,235,169,81,89,176};
  // multiplication of polynomial A and B modulo M
  int gf_char2_mult(int A,int B,int M){
    if (A==0 || B==0)
      return 0;
    if (M==285){ // special handling for GF(2,8)
      // tab2add[k+1] == g^k in add representation (base 2)
      // x==g^(tab2mult[x in add repr (base 2)]-1)
      int res=gf28_tab2mult[A]+gf28_tab2mult[B]-2;
      if (res>=255) res-=255;
      res=gf28_tab2add[res+1];
      //return res;
    }
    if (A<B)
      return gf_char2_mult(B,A,M);
    int R=0,N=nbits(M);
    int L=1<<(N-1); // L=M with all bits cleared except first example L=2^4
    for (;B;){
      R ^= ((B%2)*A); // adjust result if bit of b is 1
      A <<= 1 ; // shift A then divide by P if necessary
      A ^= (((A&L)>>(N-1))*M);
      B/=2;
      // loop unrolled (4 exec per loop)
      R ^= ((B%2)*A); // adjust result if bit of b is 1
      A <<= 1 ; // shift A then divide by P if necessary
      A ^= (((A&L)>>(N-1))*M);
      B/=2;
      R ^= ((B%2)*A); // adjust result if bit of b is 1
      A <<= 1 ; // shift A then divide by P if necessary
      A ^= (((A&L)>>(N-1))*M);
      B/=2;
      R ^= ((B%2)*A); // adjust result if bit of b is 1
      A <<= 1 ; // shift A then divide by P if necessary
      A ^= (((A&L)>>(N-1))*M);
      B/=2;
    }
    return R;
  }

  // quotient and remainder of A and B representing polynomials on Z/2Z
  int gf_char2_quorem(int A,int B,int &Q){
    Q=0;
    int a,b=nbits(B);
    while ((a=nbits(A))>=b){
      A ^= (B<<(a-b));
      Q ^= (1<<(a-b));
    }
    return A;
  }

  // assumes nbits(A)+nbits(B)<=62
  longlong gf_char2_mult(longlong A,longlong B){
    if (A<B)
      return gf_char2_mult(B,A);
    longlong R=0;
    for (;B;){
      R ^= ((B%2)*A); // adjust result if bit of b is 1
      A <<= 1 ; // shift A
      B/=2;
      // loop unrolled (4 exec per loop)
      R ^= ((B%2)*A); 
      A <<= 1 ; 
      B/=2;
      R ^= ((B%2)*A); 
      A <<= 1 ; 
      B/=2;
      R ^= ((B%2)*A); 
      A <<= 1 ; 
      B/=2;
    }
    return R;
  }

  // invert A mod N, A and N polynomial over Z/2Z
  int gf_char2_inv(int A,int N){
    if (N==285){
      if (A<2) return A; // means undef for A==0, inv(1)==1
      // A==g^(gf28_tab2mult[A]-1), A^-1=g^(256-gf28_tab2mult[A])
      int res=gf28_tab2add[257-gf28_tab2mult[A]];
      return res;
    }
    int U0=0,U1=1,U2,Q,R0(N),R1(A),R2;
    // A*Uk+N*Vk=Rk (polynomial product in Z/2Z[X], Vk not computed)
    while (R1){
      R2=gf_char2_quorem(R0,R1,Q);
      U2=U0 ^ gf_char2_mult(U1,Q);
      R0=R1;
      R1=R2;
      U0=U1;
      U1=U2;
    }
    return U0;
  }

  // a[astart..aend[ + b[bstart..bend[ to res
  // M==0 integer addition, M==-1 integer subtraction, else gf_char2
  void gf_char2_addp(const vector<int> & a,int astart,int aend,const vector<int> &b,int bstart,int bend,vector<int> & res,int M){
    res.clear();
    res.reserve(giacmax(aend-astart,bend-bstart));
    if (M<=0){
      if (M<0){
	for (--aend,--bend;aend>=astart && bend>=bstart;--aend,--bend){
	  res.push_back(a[aend]-b[bend]);
	}
	for (;bend>=bstart ;--bend){
	  res.push_back(-b[bend]);
	}
      }
      else {
	for (--aend,--bend;aend>=astart && bend>=bstart;--aend,--bend){
	  res.push_back(a[aend]+b[bend]);
	}
	for (;bend>=bstart ;--bend){
	  res.push_back(b[bend]);
	}
      }
    }
    else {
      for (--aend,--bend;aend>=astart && bend>=bstart;--aend,--bend){
	res.push_back(a[aend]^b[bend]);
      }
      for (;bend>=bstart ;--bend){
	res.push_back(b[bend]);
      }
    }
    for (;aend>=astart ;--aend){
      res.push_back(a[aend]);
    }
    reverse(res.begin(),res.end());
  }

  void gf_char2_addp(const vector<int> & a,const vector<int> &b,vector<int> & res,int M){
    gf_char2_addp(a,0,a.size(),b,0,b.size(),res,M);
  }

  // a[astart..aend[ * b[bstart..bend[ to res
  // M==0 integer multiplication, else gf_char2
  void gf_char2_multp(const vector<int> & a,int astart,int aend,const vector<int> &b,int bstart,int bend,vector<int> & res,int M){
    int adeg=aend-astart-1,bdeg=bend-bstart-1;
    if (bdeg<adeg){
      gf_char2_multp(b,bstart,bend,a,astart,aend,res,M);
      return;
    }
    // now bdeg>=adeg
    // INT_KARAMUL_SIZE=1; //M=0; // uncomment for debug with integer mult
    if (adeg>INT_KARAMUL_SIZE && bdeg>INT_KARAMUL_SIZE){
      if (bdeg/2>adeg){       
	// split b in 2 parts?
	vector<int> res1,res2;
	gf_char2_multp(a,astart,aend,b,bstart,bstart+bdeg/2,res1,M);
	res2.reserve(bdeg+adeg+1);
	gf_char2_multp(a,astart,aend,b,bstart+bdeg/2,bend,res2,M);
	for (int i=0;i<bdeg/2;++i)
	  res2.push_back(0);
	gf_char2_addp(res1,res2,res,M);
	return;
      }
      // adeg/2<=bdeg/2<=adeg
      int B=bdeg/2,bmid=bend-B,amid=aend-B;
      // a=a1+a2*x^B, b=b1+b2*x^B, 
      // a*b=a1*b1+a2*b2*x^(2B)+x^B*(a1*b2+a2*b1)
      //    =a1*b1+a2*b2*x^(2*B)+x^B*((a1+a2)*(b1+b2)-a1*b1-a2*b2)
      vector<int> a12,b12,reslow,reshigh,res3,res4;
      gf_char2_multp(a,amid,aend,b,bmid,bend,reslow,M);
      gf_char2_multp(a,astart,amid,b,bstart,bmid,reshigh,M);
      gf_char2_addp(a,amid,aend,a,astart,amid,a12,M);
      gf_char2_addp(b,bmid,bend,b,bstart,bmid,b12,M);
      gf_char2_multpoly(a12,b12,res3,M);
      gf_char2_addp(res3,reslow,res4,M?M:-1);
      gf_char2_addp(res4,reshigh,res3,M?M:-1);
      for (int i=0;i<B;++i)
	res3.push_back(0);
      for (int i=0;i<2*B;++i)
	reshigh.push_back(0);
      gf_char2_addp(reslow,res3,res4,M);
      gf_char2_addp(res4,reshigh,res,M);
      return;
    }
    // plain multiplication
    res.clear();
    res.resize(adeg+bdeg+1);
    if (M==0){
      for (int i=astart;i<aend;++i){
	int ai=a[i];
	int pos=i-astart-bstart;
	for (int j=bstart;j<bend;++j){
	  res[pos+j] += ai*b[j];
	}
      }
      return;
    }
    for (int i=astart;i<aend;++i){
      int ai=a[i];
      int pos=i-astart-bstart;
      for (int j=bstart;j<bend;++j){
	// loop could be rolled, pointers used, etc.
	// this is a little bit suboptimal because we make 2 read and 1 write
	// using a register would be better 
	res[pos+j] ^= gf_char2_mult(ai,b[j],M);
      }
    }
  }

  // a polynomial with coeffs in GF(2,x) -> A polynomial (Kronecker substution)
  void char2_kronecker_expand(const std::vector<int> & a,int n,std::vector<int> & A){
    A.clear();
    A.reserve(a.size()*n);
    vector<int> tmp(n);
    for (int i=0;i<a.size();++i){
      int ai=a[i];
      tmp.clear();
      while (ai){
	tmp.push_back(ai % 2);
	ai /= 2;
      }
      int fill=n-tmp.size();
      for (int j=0;j<fill;++j)
	A.push_back(0);
      vector<int>::const_iterator it=tmp.end(),itbeg=tmp.begin();
      for (;it!=itbeg;--it)
	A.push_back(*(it-1));
    }
  }

  // converse of kronecker_expand,
  // polynomials in a are reduced modulo the polynomial represented by M
  void char2_kronecker_shrink(const std::vector<int> & A,int n,int M,std::vector<int> & a){
    // multiplying 2 polynomials of size a by size b
    // returns a poly of size a+b-1 instead of a+b, must fill an initial 0
    std::vector<int>::const_iterator it=A.begin()+n-1,itnext,itend=A.end();
    for (;it<itend;){
      // read n integers
      int tmp=0,Q;
      itnext=it+n;
      for (;it<itnext;++it){
	tmp = 2*tmp+(*it)%2;
      }
      a.push_back(gf_char2_quorem(tmp,M,Q));
    }
  }

  bool gf_char2_multpoly(const std::vector<int> & a,const std::vector<int> & b,std::vector<int> & res,int M){
    if (a.empty() || b.empty()){
      res.clear();
      return true;
    }
    // speedups: 1/ Karatsuba 
    // 2/ in order to compute a(y)*b(y) we may write y=x^n for n==2*nbits(M)
    // and each coeff of a and b as a polynomial in x of degre<n
    // multiply A*B in x with FFT over Z/p1, p1=2013265921
    // reduce coeffs as a polynomial in y w.r.t. the polynomial corresp. to M
    int adeg=a.size()-1,bdeg=b.size()-1;
    if (//1
	adeg>FFTMUL_SIZE && bdeg>FFTMUL_SIZE
	){
      //CERR << "fft" << adeg << "x" << bdeg << endl;
      int n=2*(nbits(M)-1);
      int p1=2013265921;
      if (longlong(adeg+bdeg+1)*n<(1<<27)){
	vector<int> A,B,RES,W,tmpa,tmpb;
	char2_kronecker_expand(a,n,A);
	char2_kronecker_expand(b,n,B);
	if (fft2mult(1,A,B,RES,p1,W,tmpa,tmpb,true /* reverseatend */,true/*dividebyn*/,false /* pos coeff */)){
	  char2_kronecker_shrink(RES,n,M,res);
	  return true;
	}
      }
    }
    gf_char2_multp(a,0,a.size(),b,0,b.size(),res,M);
    return true;
  } 

  int dotgf_char2(const vector<int> & v,const vector<int> & w,int M){
    vector<int>::const_iterator it=v.begin(),itend=v.end(),jt=w.begin(),jtend=w.end();
    int res=0;
    for (;it!=itend && jt!=jtend;++it,++jt)
      res ^= gf_char2_mult(*it,*jt,M);
    return res;
  }

  // convert v in char 2, returns minimal polynomial or 0 (unknown) or -1 (unable to convert)
  int gf_char2_vecteur2vectorint(const vecteur & v,vector<int> & V,gen & x){
    // quick check
    int i=0;
    for (;i<v.size();++i){
      if (v[i].type==_INT_ || v[i].type==_ZINT)
	continue;
      if (v[i].type==_MOD && (*(v[i]._MODptr+1)!=plus_two))
	return -1;
      if (v[i].type!=_USER)
	return -1;
      if (galois_field * gf=dynamic_cast<galois_field *>(v[i]._USERptr)){
	if (gf->p!=plus_two || gf->P.type!=_INT_ || gf->a.type!=_INT_)
	  return -1;
	x=gf->x;
	break;
      }
      else
	return -1;
    }
    V.resize(v.size());
    int a=0;
    for (i=0;i<v.size();++i){
      if (v[i].type==_INT_){
	V[i]=v[i].val % 2;
	continue;
      }
      if (v[i].type==_ZINT){
	V[i]=modulo(*v[i]._ZINTptr,2) % 2;
	continue;
      }
      if (v[i].type==_MOD){
	if (*(v[i]._MODptr+1)!=plus_two)
	  return -1;
	V[i]=v[i]._MODptr->val % 2;
	continue;
      }
      if (v[i].type!=_USER)
	return -1;
      if (galois_field * gf=dynamic_cast<galois_field *>(v[i]._USERptr)){
	if (a==0)
	  a=gf->P.val;
	else {
	  if (a!=gf->P.val)
	    return -1;
	}
	V[i]=gf->a.val;
      }
    }
    return a;
  }

  // convert m in char 2, returns minimal polynomial or 0 (unknown) or -1 (unable to convert)
  int gf_char2_matrice2vectorvectorint(const matrice & m,vector< vector<int> > & M,gen & x){
    M.resize(m.size());
    int a=0,b;
    for (int i=0;i<m.size();++i){
      if (m[i].type!=_VECT || (b=gf_char2_vecteur2vectorint(*m[i]._VECTptr,M[i],x))<0 )
	return -1;
      if (a==0)
	a=b;
      else {
	if (b>0 && a!=b)
	  return -1;
      }
    }
    return a;
  }

  void gf_char2_vectorint2vecteur(const std::vector<int> & source,vecteur & target,int M,const gen & x){
    target.resize(source.size());
    for (int i=0;i<source.size();++i){
      target[i]=galois_field(plus_two,M,x,source[i]);
    }
  }

  void gf_char2_vectorvectorint2mat(const std::vector< std::vector<int> > & source,matrice & target,int M,const gen & x){
    target.resize(source.size());
    for (int i=0;i<source.size();++i){
      vecteur T;
      gf_char2_vectorint2vecteur(source[i],T,M,x);
      target[i]=T;
    }    
  }

  bool gf_char2_mmult_atranb(const std::vector< std::vector<int> > & A,const std::vector< std::vector<int> > & tranB,std::vector< std::vector<int> > & C,int M){
    int r=A.size();
    if (r==0)
      return false;
    int c=tranB.size();
    if (c==0 || A[0].size()!=tranB[0].size())
      return false;
    C.resize(r);
    for (int i=0;i<r;++i){
      C[i].resize(c);
      for (int j=0;j<c;j++){
	C[i][j]=dotgf_char2(A[i],tranB[j],M);
      }
    }
    return true;
  }

  // v1=v1+c2*v2 
  void gf_char2_linear_combination(std::vector<int> & v1,int c2,const std::vector<int> & v2,int cstart,int cend,int M){
    if (c2){
      std::vector<int>::iterator it1=v1.begin()+cstart,it1end=v1.end();
      if (cend && cend>=cstart && cend<it1end-v1.begin())
	it1end=v1.begin()+cend;
      std::vector<int>::const_iterator it2=v2.begin()+cstart;
      for (;it1!=it1end;++it1,++it2)
	*it1 ^= gf_char2_mult(c2,*it2,M);
    }
  }

  bool gf_char2_rref(std::vector< std::vector<int> > & N,const gen & x,int M,vecteur & pivots,std::vector<int> & permutation,std::vector<int> & maxrankcols,gen & det,int l, int lmax, int c,int cmax,int fullreduction,int dont_swap_below,int rref_or_det_or_lu){
    bool use_cstart=!c;
    bool inverting=fullreduction==2;
    int linit=l;
    int idet=1;
    pivots.clear();
    pivots.reserve(cmax-c);
    permutation.clear();
    maxrankcols.clear();
    for (int i=0;i<lmax;++i)
      permutation.push_back(i);
    for (;(l<lmax) && (c<cmax);){
      int pivot=N[l][c];
      if (rref_or_det_or_lu==3 && !pivot){ // LU without permutation
	det=0;
	return true;
      }
      if ( rref_or_det_or_lu==1 && l==lmax-1 ){
	idet = (idet * pivot);
	break;
      }
      int pivotline=l;
      int pivotcol=c;
      bool noswap=false;
      if (!pivot){
	if (l<dont_swap_below){ 
	  for (int ctemp=c+1;ctemp<cmax;++ctemp){
	    int temp=N[l][ctemp];
	    if (temp){
	      pivot=temp;
	      pivotcol=ctemp;
	      break;
	    }
	  }
	}
	else {      // scan N current column for the first pivot available
	  for (int ltemp=l+1;ltemp<lmax;++ltemp){
	    int temp=N[ltemp][c];
	    if (temp){
	      pivot=temp;
	      pivotline=ltemp;
	      break;
	    }
	  }
	}
      }
      if (pivot){
	maxrankcols.push_back(c);
	if (l!=pivotline){
	  swap(N[l],N[pivotline]);
	  swap(permutation[l],permutation[pivotline]);
	  pivotline=l;
	  // idet = -idet; // we are in characteristic 2
	}
	// save pivot for annulation test purposes
	if (rref_or_det_or_lu!=1)
	  pivots.push_back(pivot);
	// invert pivot 
	int invpivot=gf_char2_inv(pivot,M);
	// multiply det
	idet = gf_char2_mult(idet,pivot,M) ;
	if (fullreduction || rref_or_det_or_lu<2){ // not LU decomp
	  std::vector<int>::iterator it=N[pivotline].begin(),itend=N[pivotline].end();
	  int invpiv=gf_char2_inv(pivot,M);
	  for (;it!=itend;++it){
	    *it = gf_char2_mult(*it,invpivot,M);
	  }
	}
	// if there are 0 at the end, ignore them in linear combination
	int effcmax=cmax-1;
	const std::vector<int> & Npiv=N[pivotline];
	for (;effcmax>=c;--effcmax){
	  if (Npiv[effcmax])
	    break;
	}
	++effcmax;
	if (fullreduction && inverting && noswap)
	  effcmax=giacmax(effcmax,c+1+lmax);
	// make the reduction
	if (fullreduction){
	  for (int ltemp=linit;ltemp<lmax;++ltemp){
	    if (ltemp==l)
	      continue;
	    gf_char2_linear_combination(N[ltemp],N[ltemp][pivotcol],N[l],(use_cstart?c:cmax),effcmax,M);
	  }
	}
	else {
	  for (int ltemp=l+1;ltemp<lmax;++ltemp){
	    if (rref_or_det_or_lu>=2) // LU decomp
	      N[ltemp][pivotcol] = gf_char2_mult(N[ltemp][pivotcol],invpivot,M);
	    gf_char2_linear_combination(N[ltemp],-N[ltemp][pivotcol],N[l],(rref_or_det_or_lu>0)?(c+1):(use_cstart?c:cmax),effcmax,M);
	  }
	} // end else
	// increment column number if swap was allowed
	if (l>=dont_swap_below)
	  ++c;
	// increment line number since reduction has been done
	++l;	  
      } // end if (!is_zero(pivot)
      else { // if pivot is 0 increment either the line or the col
	idet = 0;
	if (rref_or_det_or_lu==1){
	  det=0;
	  return true;
	}
	if (l>=dont_swap_below)
	  c++;
	else
	  l++;
      }
    } // end for reduction loop
    det=galois_field(plus_two,M,x,idet);
    for (int i=0;i<pivots.size();++i){
      pivots[i]=galois_field(plus_two,M,x,idet);
    }
    return true;
  }

  // GF multiplication (except char 2), representation with vector< vector<int> >
  // coeffs are assumed in [0..modulo[
  // a[astart..aend[ + b[bstart..bend[ to res
  void gf_addp(const vector< vector<int> > & a,int astart,int aend,const vector< vector<int> > &b,int bstart,int bend,vector< vector<int> > & res,int modulo){
    res.clear();
    res.resize(giacmax(aend-astart,bend-bstart));
    int pos=0;
    for (--aend,--bend;aend>=astart && bend>=bstart;++pos,--aend,--bend){
      vector<int> v(a[aend]);
      addmod(v,b[bend],modulo);
      res[pos].swap(v);
    }
    for (;bend>=bstart ;++pos,--bend){
      res[pos]=b[bend];
    }
    for (;aend>=astart ;++pos,--aend){
      res[pos]=a[aend];
    }
    reverse(res.begin(),res.end());
  }

  void gf_addp(const vector< vector<int> > & a,const vector< vector<int> > &b,vector< vector<int> > & res,int modulo){
    gf_addp(a,0,a.size(),b,0,b.size(),res,modulo);
  }

  // a[astart..aend[ - b[bstart..bend[ to res
  void gf_subp(const vector< vector<int> > & a,int astart,int aend,const vector< vector<int> > &b,int bstart,int bend,vector< vector<int> > & res,int modulo){
    res.clear();
    res.resize(giacmax(aend-astart,bend-bstart));
    int pos=0;
    for (--aend,--bend;aend>=astart && bend>=bstart;++pos,--aend,--bend){
      vector<int> v(a[aend]);
      submod(v,b[bend],modulo);
      res[pos].swap(v);
    }
    for (;bend>=bstart ;++pos,--bend){
      res[pos]=b[bend];
      vector<int>::iterator it=res[pos].begin(),itend=res[pos].end();
      for (;it<itend;++it){
	int s=*it;
	*it=s?modulo-s:s;
      }
    }
    for (;aend>=astart ;++pos,--aend){
      res[pos]=a[aend];
    }
    reverse(res.begin(),res.end());
  }

  void gf_subp(const vector< vector<int> > & a,const vector< vector<int> > &b,vector< vector<int> > & res,int modulo){
    gf_subp(a,0,a.size(),b,0,b.size(),res,modulo);
  }

  void kronecker_expand(const std::vector< std::vector<int> > & a,int n,std::vector<int> & A){
    A.clear();
    A.reserve(a.size()*n);
    vector<int> tmp(n);
    for (int i=0;i<a.size();++i){
      const vector<int> & tmp=a[i];
      int fill=n-tmp.size();
      for (int j=0;j<fill;++j)
	A.push_back(0);
      vector<int>::const_iterator it=tmp.begin(),itend=tmp.end();
      for (;it!=itend;++it)
	A.push_back(*it);
    }
  }

  void kronecker_shrink(const std::vector<int> & A,int n,std::vector< std::vector<int> > & a,const std::vector<int> & pmin,int modulo){
    // multiplying 2 polynomials of size a by size b
    // returns a poly of size a+b-1 instead of a+b, must fill an initial 0
    std::vector<int>::const_iterator it=A.begin()+n-1,itnext,itend=A.end();
    for (;it<itend;){
      // read n integers
      vector<int> tmp,q,r;
      itnext=it+n;
      for (;it<itnext;++it){
	if (*it)
	  break;
      }
      tmp.reserve(itnext-it);
      for (;it<itnext;++it){
	tmp.push_back(*it);
      }
      DivRem(tmp,pmin,modulo,q,r);
      a.push_back(r);
    }
  }

  // a[astart..aend[ * b[bstart..bend[ to res
  void gf_multp(const vector < vector<int> > & a,int astart,int aend,const vector< vector<int> > &b,int bstart,int bend,vector< vector<int> > & res,const vector<int> & pmin,int modulo){
    int adeg=aend-astart-1,bdeg=bend-bstart-1;
    if (bdeg<adeg){
      gf_multp(b,bstart,bend,a,astart,aend,res,pmin,modulo);
      return;
    }
    // now bdeg>=adeg
#if 0
    //INT_KARAMUL_SIZE=1; //M=0; // uncomment for debug with integer mult
    if (adeg>KARAMUL_SIZE && bdeg>KARAMUL_SIZE){
      if (bdeg/2>adeg){       
	// split b in 2 parts?
	vector< vector<int> > res1,res2;
	gf_multp(a,astart,aend,b,bstart,bstart+bdeg/2,res1,pmin,modulo);
	res2.reserve(bdeg+adeg+1);
	gf_multp(a,astart,aend,b,bstart+bdeg/2,bend,res2,pmin,modulo);
	for (int i=0;i<bdeg/2;++i)
	  res2.push_back(vector<int>(0));
	gf_addp(res1,res2,res,modulo);
	return;
      }
      // adeg/2<=bdeg/2<=adeg
      int B=bdeg/2,bmid=bend-B,amid=aend-B;
      // a=a1+a2*x^B, b=b1+b2*x^B, 
      // a*b=a1*b1+a2*b2*x^(2B)+x^B*(a1*b2+a2*b1)
      //    =a1*b1+a2*b2*x^(2*B)+x^B*((a1+a2)*(b1+b2)-a1*b1-a2*b2)
      vector< vector<int> > a12,b12,reslow,reshigh,res3,res4;
      gf_multp(a,amid,aend,b,bmid,bend,reslow,pmin,modulo);
      gf_multp(a,astart,amid,b,bstart,bmid,reshigh,pmin,modulo);
      gf_addp(a,amid,aend,a,astart,amid,a12,modulo);
      gf_addp(b,bmid,bend,b,bstart,bmid,b12,modulo);
      gf_multpoly(a12,b12,res3,pmin,modulo);
      gf_subp(res3,reslow,res4,modulo);
      gf_subp(res4,reshigh,res3,modulo);
      for (int i=0;i<B;++i)
	res3.push_back(vector<int>(0));
      for (int i=0;i<2*B;++i)
	reshigh.push_back(vector<int>(0));
      gf_addp(reslow,res3,res4,modulo);
      gf_addp(res4,reshigh,res,modulo);
      return;
    }
#endif
    // plain multiplication, 
    res.clear();
    int s=adeg+bdeg+1;
    res.resize(s);
    vector<int> tmp;
    for (int k=0;k<s;++k){
      vector<int> & r =res[k];
      for (int i=giacmax(0,k-bdeg);i<=k && i<=adeg;++i){
	tmp.clear();
	mulext(a[i+astart],b[k-i+bstart],pmin,modulo,tmp);
	addmod(r,tmp,modulo);
      }
      res[k]=r % modulo;
    }
  }

  bool gf_multpoly(const std::vector< std::vector<int> > & a,const std::vector< std::vector<int> > & b,std::vector< std::vector<int> > & res,const std::vector<int> & pmin,int modulo){
    if (a.empty() || b.empty()){
      res.clear();
      return true;
    }
    // speedups: 1/ Karatsuba 
    // 2/ in order to compute a(y)*b(y) we may write y=x^n for n==2*nbits(M)
    // and each coeff of a and b as a polynomial in x of degre<n
    // multiply A*B in x with FFT over Z/p1, p1=2013265921
    // reduce coeffs as a polynomial in y w.r.t. the polynomial corresp. to M
    int adeg=a.size()-1,bdeg=b.size()-1;
#if 1
    if (//1
	adeg>FFTMUL_SIZE/10 && bdeg>FFTMUL_SIZE/10
	){
      //CERR << "fft" << adeg << "x" << bdeg << endl;
      int n=2*(pmin.size()-1);
      int p1=2013265921;
      if (longlong(adeg+bdeg+1)*n<(1<<27)){
	vector<int> A,B,RES,W,tmpa,tmpb,resp1,resp2,resp3,Wp1,Wp2,Wp3,Wp4,tmp7,tmp8;
	kronecker_expand(a,n,A);
	makepositive(&A.front(),A.size(),modulo);
	kronecker_expand(b,n,B);
	makepositive(&B.front(),B.size(),modulo);
#if 1
	if ( (modulo-1.0)*(modulo-1.0)*giacmin(adeg,bdeg)<p1 && fft2mult(modulo-1,A,B,RES,p1,W,tmpa,tmpb,true /* reverseatend */,true/*dividebyn*/,false /* make coeff + */)){
	  kronecker_shrink(RES,n,res,pmin,modulo);
	  return true;
	}
#endif
	vecteur AV,BV,ABV;
	vector_int2vecteur(A,AV);
	vector_int2vecteur(B,BV);
	if (fftmultp1234(AV,BV,modulo-1,modulo-1,ABV,modulo,tmpa,tmpb,resp1,resp2,resp3,Wp1,Wp2,Wp3,Wp4,tmp7,tmp8,true)){
	  vecteur2vector_int(ABV,modulo,RES);
	  kronecker_shrink(RES,n,res,pmin,modulo);
	  return true;
	}
      }
    }
#endif
    gf_multp(a,0,a.size(),b,0,b.size(),res,pmin,modulo);
    return true;
  } 

  int gf_vecteur2vectorvectorint(const vecteur & v,std::vector< std::vector<int> > & V,gen & x,std::vector<int> & pmin){
    // quick check
    int i=0,carac=0; gen P;
    for (;i<v.size();++i){
      if (v[i].type==_INT_ || v[i].type==_ZINT)
	continue;
      if (v[i].type==_MOD){
	gen & m=*(v[i]._MODptr+1);
	if (m.type!=_INT_)
	  return 0;
	if (carac==0)
	  carac=m.val;
	if (m.val!=carac)
	  return 0;
      }
      if (v[i].type!=_USER)
	return 0;
      if (galois_field * gf=dynamic_cast<galois_field *>(v[i]._USERptr)){
	if (gf->p.type!=_INT_ || gf->P.type!=_VECT)
	  return 0;
	if (carac==0)
	  carac=gf->p.val;
	if (gf->p.val!=carac)
	  return 0;
	x=gf->x;
	P=gf->P;
	vecteur2vector_int(*P._VECTptr,carac,pmin);
 	break;
      }
      else
	return 0;
    }
    // convert
    V.resize(v.size());
    for (i=0;i<v.size();++i){
      if (v[i].type==_INT_){
	V[i]=vector<int>(1,v[i].val % 2);
	continue;
      }
      if (v[i].type==_ZINT){
	V[i]=vector<int>(1,modulo(*v[i]._ZINTptr,2) % 2);
	continue;
      }
      if (v[i].type==_MOD){
	if (*(v[i]._MODptr+1)!=carac)
	  return 0;
	V[i]=vector<int>(1,v[i]._MODptr->val % 2);
	continue;
      }
      if (v[i].type!=_USER)
	return 0;
      if (galois_field * gf=dynamic_cast<galois_field *>(v[i]._USERptr)){
	if (P!=gf->P || gf->a.type!=_VECT)
	  return 0;
	vecteur2vector_int(*gf->a._VECTptr,carac,V[i]);
      }
    }
    return carac;
  }

  // convert m, returns minimal polynomial or 0 (unknown) or -1 (unable to convert)
  int gf_matrice2vectorvectorvectorint(const matrice & m,std::vector< std::vector< std::vector<int> > > & M,gen & x,std::vector<int> & pmin){
    M.resize(m.size());
    vector<int> b;
    int carac=0,c;
    for (int i=0;i<m.size();++i){
      if (m[i].type!=_VECT || !(c=gf_vecteur2vectorvectorint(*m[i]._VECTptr,M[i],x,b)))
	return 0;
      if (carac==0)
	carac=c;
      if (carac!=c)
	return 0;
      if (pmin.empty())
	pmin=b;
      else {
	if (!b.empty() && !(pmin==b))
	  return 0;
      }
    }
    return carac;
  }

  void gf_vectorvectorint2vecteur(const std::vector< std::vector<int> > & source,vecteur & target,const gen & carac,const vecteur & pmin,const gen & x){
    //CERR << pmin << endl;
    target.resize(source.size());
    vecteur tmp; 
    for (int i=0;i<source.size();++i){
      tmp.clear();
      vector_int2vecteur(source[i],tmp);
      target[i]=galois_field(carac,pmin,x,tmp);
    }
  }

  void gf_vectorvectorint2vecteur(const std::vector< std::vector<int> > & source,vecteur & target,int carac,const std::vector<int> & pmin,const gen & x){
    vecteur Pmin;
    vector_int2vecteur(pmin,Pmin);
    //CERR << pmin << " " << Pmin << endl;
    gf_vectorvectorint2vecteur(source,target,carac,Pmin,x);
  }

  void gf_vectorvectorvectorint2mat(const std::vector< std::vector< std::vector<int> > > & source,matrice & target,int carac,const std::vector<int> & pmin,const gen & x){
    vecteur Pmin;
    vector_int2vecteur(pmin,Pmin);
    target.resize(source.size());
    for (int i=0;i<source.size();++i){
      vecteur T;
      gf_vectorvectorint2vecteur(source[i],T,carac,Pmin,x);
      target[i]=T;
    }    
  }

  gen galois_field::operator * (const gen & g) const { 
    bool char2=p.type==_INT_ && p.val==2 && a.type==_INT_;
    if (is_integer(g)){
      if (char2){
	if (g.type==_ZINT?modulo(*g._ZINTptr,2)==0:g%2==0) 
	  return 0;
	return *this;
      }
      gen tmp=smod(g,p);
      if (is_exactly_zero(tmp))
	return zero;
      return galois_field(p,P,x,g*a);
    }
    if (g.type==_MOD){
      if (*(g._MODptr+1)!=p)
	return gensizeerr(gettext("Incompatible characteristics"));
      return *this*(*g._MODptr);
    }
    if (g.type==_VECT){
      vecteur v=*g._VECTptr;
      int s=int(v.size());
      for (int i=0;i<s;++i)
	v[i]=*this*v[i];
      return gen(v,g.subtype);
    }
    if (g.type!=_USER)
      return sym_mult(*this,g,context0); // ok symbolic(at_prod,makesequence(g,*this));
    if (galois_field * gptr=dynamic_cast<galois_field *>(g._USERptr)){
      if (char2 && P.type==_INT_ && gptr->a.type==_INT_){
	if (P!=gptr->P)
	  return galois_field(p,P,x,char2_uncoerce(a),false)*galois_field(gptr->p,gptr->P,gptr->x,char2_uncoerce(gptr->a),false);
	return galois_field(p,P,x,gf_char2_mult(a.val,gptr->a.val,P.val));
      }
      // if (gptr->p!=p || gptr->P!=P || P.type!=_VECT || is_undef(P) || is_undef(gptr->P)) return gensizeerr();
      if (a.type==_VECT && gptr->a.type==_VECT){
	galois_field * gfptr=new galois_field(p,P,x,new ref_vecteur(*a._VECTptr),false);
	int tst=common_gf(*gfptr,*gptr);
	if (!tst)
	  return gensizeerr();
	if (tst==1 && p.type==_INT_){
	  ref_gen_user * resptr=new ref_gen_user(*gfptr);
	  int m=p.val;
	  vector<int> amod,bmod,ab,pmod;
	  vecteur2vector_int(*a._VECTptr,0,amod);
	  vecteur2vector_int(*gptr->a._VECTptr,0,bmod);
	  vecteur2vector_int(*P._VECTptr,0,pmod);
	  mulext(amod,bmod,pmod,m,ab);
	  int absize=int(ab.size());
	  int * i=ab.empty()?0:&ab.front(),*iend=i+absize;
	  for (;i<iend;++i){
	    int j=*i;
	    //j =smod(j,m);
	    j += (unsigned(j)>>31)*m; // make positive
	    j -= (unsigned((m>>1)-j)>>31)*m;
	    *i=j;
	  }
	  // if (absize==1) return ab.front()?zero:makemod(ab.front(),p); // does not work chk_fhan12
	  vector_int2vecteur(ab,*gfptr->a._VECTptr);
	  delete gfptr;
	  return resptr; // galois_field(p,P,x,resdbg,false);
	}
	gen & A_=gfptr->a;
	gen & B_=gptr->a;
	if (A_.type==_INT_ && B_.type==_INT_ && gfptr->P.type==_INT_){
	  A_=gf_char2_mult(A_.val,B_.val,gfptr->P.val);
	}
	else {
	  A_=char2_uncoerce(A_);
	  B_=char2_uncoerce(B_);
	  environment env;
	  env.modulo=p;
	  env.pn=env.modulo;
	  env.moduloon=true;
	  vecteur resdbg,quo;
	  mulmodpoly(*A_._VECTptr,*B_._VECTptr,&env,resdbg);
	  DivRem(resdbg,*gptr->P._VECTptr,&env,quo,*A_._VECTptr);
	}
	gen res=*gfptr;
	delete gfptr;
	return res;
      }
      return galois_field(p,P,x,a*gptr->a);
    }
    else
      return gensizeerr();
  }

  gen galois_field::inv () const {
    if (p.type==_INT_ && p.val==2 && a.type==_INT_ && P.type==_INT_){
      return galois_field(p,P,x,gf_char2_inv(a.val,P.val));
    }
    gen A = char2_uncoerce(a);
    if (A.type!=_VECT || (P.type!=_VECT && P.type!=_INT_) )
      return gensizeerr(gettext("galois field inv"));
    if (A._VECTptr->empty())
      return galois_field(p,P,x,undef);
    modpoly u,v,d;
    environment * env=new environment;
    env->modulo=p;
    env->pn=env->modulo;
    env->moduloon=true;
    egcd(*A._VECTptr,*char2_uncoerce(P)._VECTptr,env,u,v,d);
    delete env;
    // d should be [1]
    if (d!=vecteur(1,1))
      return gensizeerr(gettext("GF inv internal bug"));
    return galois_field(p,P,x,u);
  }

  bool galois_field::operator == (const gen & g) const {
    if (is_zero())
      return is_exactly_zero(g);
    if (g.type!=_USER)
      return a==vecteur(1,g);
    if (galois_field * gptr=dynamic_cast<galois_field *>(g._USERptr)){
      if (gptr->p!=p || gptr->P!=P)
	return false;
      return gptr->a==a;
    }
    return false;
  }

  bool galois_field::is_zero () const {
    return a==zero || (a.type==_VECT && ( a._VECTptr->empty() || (a._VECTptr->size()==1 && a._VECTptr->front()==0) ) );
  }

  bool galois_field::is_one () const {
    return a==plus_one || (a.type==_VECT && a._VECTptr->size()==1 && a._VECTptr->front()==1);
  }

  bool galois_field::is_minus_one () const {
    return a==plus_one || (a.type==_VECT && a._VECTptr->size()==1 && smod(a._VECTptr->front(),p)==-1); // this is really plus_one because we are in char 2
  }

  gen galois_field::operator () (const gen & g,GIAC_CONTEXT) const {
    if (is_undef(a)){
      gen res;
      if (g.type==_VECT){
	vecteur v(*g._VECTptr);
	for (unsigned i=0;i<v.size();++i){
	  v[i]=(*this)(v[i],contextptr);
	}
	return gen(v,g.subtype);
      }
      else {
	gen xid(x);
	if (x.type==_VECT && !x._VECTptr->empty())
	  xid=x._VECTptr->front();
	vecteur v(1,xid);
	lvar(g,v);
	if (v.size()>1){
	  if (g.type==_IDNT)
	    return g;
	  else {
	    if (g.type==_SYMB){
	      if (g._SYMBptr->sommet==at_plus)
		return _plus((*this)(g._SYMBptr->feuille,contextptr),contextptr);
	      if (g._SYMBptr->sommet==at_neg)
		return -(*this)(g._SYMBptr->feuille,contextptr);
	      if (g._SYMBptr->sommet==at_prod)
		return _prod((*this)(g._SYMBptr->feuille,contextptr),contextptr);
	      if (g._SYMBptr->sommet==at_inv){
		gen tmp=(*this)(g._SYMBptr->feuille,contextptr);
		return _inv(tmp,contextptr);
	      }
	      if (g._SYMBptr->sommet==at_pow && g._SYMBptr->feuille.type==_VECT && g._SYMBptr->feuille._VECTptr->size()==2)
		return pow((*this)(g._SYMBptr->feuille[0],contextptr),g._SYMBptr->feuille[1],contextptr);
	      return gensizeerr(x[1].print()+"("+g.print()+gettext(") invalid, works only for integers or polynomials depending on ")+xid.print());
	    }
	  }
	}
	else
	  res=_e2r(makesequence(g,xid),contextptr);
      }
      if (res.type==_VECT){
	environment env;
	env.modulo=p;
	env.pn=env.modulo;
	env.moduloon=true;
	res=smod(res,p);
	res=operator_mod(*res._VECTptr,*P._VECTptr,&env);
      }
      return galois_field(p,P,x,res);
    }
    return *this;
  }

  gen galois_field::operator [] (const gen & g) {
    if (g.type==_INT_){
      int i= g.val;
      if (array_start(context0)) --i;
      switch (i){
      case 0:
	return p;
      case 1:
	return P;
      case 2:
	return x;
      case 3:
	return a;
      }
    }
    return undef;
  }

  gen galois_field::operator >(const gen & g) const {
    if (g.type!=_USER)
      return undef;
    galois_field * gf=dynamic_cast<galois_field *>(g._USERptr);
    if (!gf)
      return undef;
    return is_strictly_positive(p-gf->p,context0); // ok
  }

  gen galois_field::operator <(const gen & g) const {
    if (g.type!=_USER)
      return undef;
    galois_field * gf=dynamic_cast<galois_field *>(g._USERptr);
    if (!gf)
      return undef;
    return is_strictly_positive(gf->p-p,context0); // ok
  }

  gen galois_field::operator <=(const gen & g) const {
    if (g.type!=_USER)
      return undef;
    galois_field * gf=dynamic_cast<galois_field *>(g._USERptr);
    if (!gf)
      return undef;
    return is_positive(gf->p-p,context0); // ok
  }

  gen galois_field::operator >=(const gen & g) const {
    if (g.type!=_USER)
      return undef;
    galois_field * gf=dynamic_cast<galois_field *>(g._USERptr);
    if (!gf)
      return undef;
    return is_positive(p-gf->p,0);
  }

  gen galois_field::sqrt(GIAC_CONTEXT) const {
    unsigned m=gfsize(P);
    // K* has cardinal p^m-1, a has a sqrt iff a^((p^m-1)/2)==1
    environment env;
    env.modulo=p;
    gen gpm=pow(p,int(m),contextptr);
    // if (gpm.type!=_INT_) return gensizeerr(gettext("Field too large"));
    // int pn=gpm.val;
    env.moduloon=true;
    gen A_=char2_uncoerce(a);
    modpoly A=*A_._VECTptr;
    gen P_=char2_uncoerce(P);
    if (p!=2){
      modpoly test(powmod(A,(gpm-1)/2,*P_._VECTptr,&env));
      if (test.size()!=1 || test.front()!=1) 
	return undef;
      // if p^m=3 [4], return A^((p^m+1)/4)
      if (smod(gpm,4)==-1){
	test=powmod(A,(gpm+1)/4,*P_._VECTptr,&env);
	if (is_positive(-test.front(),contextptr))
	  test=-test;
	return galois_field(p,P,x,test);
      }
    }
    env.moduloon=false;
    env.coeff=*this;
    env.pn=gpm;
    modpoly X(3);
    X[0]=1;
    X[2]=-*this;
    polynome px(unmodularize(X));
    factorization sqff_f(squarefree_fp(px,env.modulo.val,m)),f;
    if (p!=2){
      if (!sqff_ffield_factor(sqff_f,env.modulo.val,&env,f) || f.size()!=2)
	return undef;
      sqff_f.swap(f);
    }
    gen tmp=sqff_f.front().fact.coord.back().value;
    if (tmp.type==_USER){
      if (galois_field * gf=dynamic_cast<galois_field *>(tmp._USERptr)){
	if (gf->p!=plus_two && is_positive(-gf->a._VECTptr->front(),contextptr))
	  tmp=-tmp;
      }
    }
    return tmp;
  }

  polynome galois_field::poly_reduce(const polynome & q) const {
    polynome res(q.dim);
    vector< monomial<gen> >::const_iterator it=q.coord.begin(),itend=q.coord.end();
    for (;it!=itend;++it){
      gen g=it->value;
      if (is_integer(g)){
	gen tmp=smod(g,p);
	if (is_exactly_zero(tmp))
	  continue;
	g=galois_field(p,P,x,vecteur(1,g));
      }
      res.coord.push_back(monomial<gen>(g,it->index));
    }
    return res;
  }

  gen galois_field::polygcd(const polynome & p,const polynome & q,polynome & res) const {
    res=Tgcdpsr(poly_reduce(p),poly_reduce(q));
    if (!res.coord.empty())
      res=res/res.coord.front().value;
    return 0;
  }

  int is_irreducible(const vecteur & v,const gen &g){
    if (v.size()<2) return 0;
    if (v.size()==2) return 1;
    gen card;
    environment env;
    if (g.type==_MOD){
      card=*(g._MODptr+1);
    }
    else {
      if (g.type!=_USER)
	return -1;
      if (galois_field * gf=dynamic_cast<galois_field *>(g._USERptr)){
	env.modulo=gf->p;
	env.coeff=g;
	card=pow(gf->p,gfsize(gf->P),context0);
      }
    }
    env.modulo=card;
    env.moduloon=true;
    gen un(pow(g,0,context0));
    for (int i=1;i<=(v.size()-1)/2;i++){
      modpoly xpi(powmod(makevecteur(un,1),pow(card,i,context0)-1,v,&env)),G;
      if (xpi.empty())
	xpi.push_back(-un);
      else {
	xpi.back()-=un;
	if (xpi.size()==1 && is_zero(xpi.back()))
	  xpi.clear();
      }
      euclide_gcd(v,xpi,&env,G);
      if (G.size()>1)
	return 0;
    }
    return 1;
  }

  gen galois_field::makegen(int i) const {
    if (p.type==_ZINT)
      return galois_field(p,P,x,vecteur(1,i));
    if (P.type==_INT_ && P.val==2){
      vecteur res;
      for (;i;i/=2)
	res.push_back(i%2);
      reverse(res.begin(),res.end());
      return galois_field(p,P,x,res);
    }
    gen P_(char2_uncoerce(P));
    if (P_.type!=_VECT || p.type!=_INT_)
      return gendimerr();
    unsigned n=unsigned(P_._VECTptr->size())-1;
    //    i += pow(p,int(n)).val;
    vecteur res;
    for (unsigned j=0;j<n;++j){
      if (!i)
	break;
      res.push_back(gen(i%p.val));
      i=i/p.val;
    }
    reverse(res.begin(),res.end());
    return galois_field(p,P_,x,res);
  }

  gen galois_field::polyfactor (const polynome & p0,factorization & f) const {
    f.clear();
    polynome p(p0.dim);
    // p0 may contain null coefficients if we multiply a polynomial over Z with a galois_field
    // because of the non-0 characteristic
    vector<monomial<gen> >::const_iterator it=p0.coord.begin(),itend=p0.coord.end();
    for (;it!=itend;++it){
      if (it->value!=0)
	p.coord.push_back(*it);
    }
    if (p.coord.empty())
      return 0;
    gen lcoeff=p.coord.front().value;
    p=lcoeff.inverse(context0)*p;
    if (p.dim!=1){
#if 1
      CERR << gettext("Warning: multivariate GF factorization is experimental and may fail") << '\n';
#else
      return gendimerr(gettext("Multivariate GF factorization not yet implemented"));
#endif
    }
    gen Pmin=char2_uncoerce(P);
    if (Pmin.type!=_VECT || this->p.type!=_INT_)
      return gensizeerr(gettext("GF polyfactor"));
    environment env;
    env.moduloon=true; // false;
    env.coeff=*this;
    env.modulo=this->p.to_int();
    int exposant=int(Pmin._VECTptr->size())-1;
    env.pn=pow(this->p,exposant);
    factorization sqff_f(squarefree_fp(p,env.modulo.val,exposant));
    if (!sqff_ffield_factor(sqff_f,env.modulo.val,&env,f))
      return gensizeerr(gettext("GF polyfactor"));
    if (!is_exactly_one(lcoeff))
      f.push_back(facteur<polynome>(
				    polynome(
					     monomial<gen>(lcoeff,0,p.dim)
					     ),
				    1));
    return 0;
  }

  gen galois_field::rand (GIAC_CONTEXT) const {
    int c=p.type==_INT_?p.val:RAND_MAX;
    int m=gfsize(P);
    vecteur v(m);
    for (int i=0;i<m;++i){
      if (c==2)
	v[i]=(giac_rand(contextptr) >> 29)%2;
      else
	v[i]=giac_rand(contextptr) % c;
    }
    v=trim(v,0);
    return galois_field(p,P,x,v);
  }

  gfmap & gf_list(){
    static gfmap * ans= new gfmap;
    return *ans;
  }

  bool has_gf_coeff(const vecteur & v,gen & p, gen & pmin){
    const_iterateur it=v.begin(),itend=v.end();
    for (;it!=itend;++it){
      if (has_gf_coeff(*it,p,pmin))
	return true;
    }
    return false;
  }

  bool has_gf_coeff(const polynome & P,gen & p, gen & pmin){
    vector< monomial<gen> >::const_iterator it=P.coord.begin(),itend=P.coord.end();
    for (;it!=itend;++it){
      if (has_gf_coeff(it->value,p,pmin))
	return true;
    }
    return false;
  }

  bool has_gf_coeff(const gen & e,gen & p, gen & pmin){
    switch (e.type){
    case _USER:
      if (galois_field * ptr=dynamic_cast<galois_field *>(e._USERptr)){
	p = ptr->p;
	pmin=ptr->P;
	return true;
      }
      return false;
    case _SYMB:
      return has_gf_coeff(e._SYMBptr->feuille,p,pmin);
    case _VECT:
      return has_gf_coeff(*e._VECTptr,p,pmin);
    case _POLY:
      return has_gf_coeff(*e._POLYptr,p,pmin);
    default:
      return false;
    }
  }

  bool has_gf_coeff(const polynome & P){
    vector< monomial<gen> >::const_iterator it=P.coord.begin(),itend=P.coord.end();
    for (;it!=itend;++it){
      if (has_gf_coeff(it->value))
	return true;
    }
    return false;
  }

  bool has_gf_coeff(const vecteur & v){
    const_iterateur it=v.begin(),itend=v.end();
    for (;it!=itend;++it){
      if (has_gf_coeff(*it))
	return true;
    }
    return false;
  }
  bool has_gf_coeff(const gen & e){
    switch (e.type){
    case _USER:
      if (galois_field * ptr=dynamic_cast<galois_field *>(e._USERptr)){
	return true;
      }
      return false;
    case _SYMB:
      return has_gf_coeff(e._SYMBptr->feuille);
    case _VECT:
      return has_gf_coeff(*e._VECTptr);
    case _POLY:
      return has_gf_coeff(*e._POLYptr);
    default:
      return false;
    }
  }

#endif // NO_RTTI

#ifndef NO_NAMESPACE_GIAC
} // namespace giac
#endif // ndef NO_NAMESPACE_GIAC

