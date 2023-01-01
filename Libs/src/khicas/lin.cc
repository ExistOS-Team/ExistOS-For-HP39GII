// -*- mode:C++ ; compile-command: "g++ -I.. -g -c lin.cc -DHAVE_CONFIG_H -DIN_GIAC" -*-
#include "giacPCH.h"
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
using namespace std;
#include <stdexcept>
#include <cmath>
#include <cstdlib>
#include "sym2poly.h"
#include "usual.h"
#include "lin.h"
#include "subst.h"
#include "modpoly.h"
#include "prog.h"
#include "giacintl.h"

#ifndef NO_NAMESPACE_GIAC
namespace giac {
#endif // ndef NO_NAMESPACE_GIAC

  // Should be rewritten with a map container for better efficiency!

  bool contains(const gen & e,const unary_function_ptr & mys){
    if (e.type!=_SYMB)
      return false;
    if (e._SYMBptr->sommet==mys)
      return true;
    if (e._SYMBptr->feuille.type!=_VECT)
      return contains(e._SYMBptr->feuille,mys);
    vecteur::const_iterator it=e._SYMBptr->feuille._VECTptr->begin(),itend=e._SYMBptr->feuille._VECTptr->end();
    for (;it!=itend;++it)
      if (contains(*it,mys))
	return true;
    return false;
  }

  void compress(vecteur & res,GIAC_CONTEXT){
    if (res.size()==2) return;
    vecteur v,w;
    const_iterateur it=res.begin(),itend=res.end();
    v.reserve(itend-it);
    w.reserve((itend-it)/2);
    int pos;
    for (;it!=itend;++it){
      pos=equalposcomp(w,*(it+1));
      if (pos){
	v[2*(pos-1)] = recursive_normal(v[2*(pos-1)] + *it,false,contextptr);
	++it;
      }
      else {
	v.push_back(*it);
	++it;
	w.push_back(*it);
	v.push_back(*it);
      }
    }
    swap(res,v);
  }

  // back conversion
  gen unlin(vecteur & v,GIAC_CONTEXT){
    vecteur w;
    gen coeff;
    vecteur::const_iterator it=v.begin(),itend=v.end();
    for (;it!=itend;++it){
      coeff = *it;
      ++it;
      if (!is_zero(coeff))
	w.push_back(coeff*exp(*it,contextptr));
    }
    if (w.empty())
      return 0;
    return _plus(w,contextptr);
  }

  void convolution(const gen & coeff, const gen & arg,const vecteur & w,vecteur & res,GIAC_CONTEXT){
    vecteur::const_iterator it=w.begin(),itend=w.end();
    for (;it!=itend;++it){
      res.push_back(coeff*(*it));
      ++it;
      res.push_back(recursive_normal(arg+(*it),false,contextptr));
    }
    compress(res,contextptr);
  }

  void convolution(const vecteur & v,const vecteur & w, vecteur & res,GIAC_CONTEXT){
    res.clear();
    res.reserve(res.size()+v.size()*w.size()/2);
    vecteur::const_iterator it=v.begin(),itend=v.end();
    gen coeff;
    for (;it!=itend;++it){
      coeff = *it;
      ++it;
      convolution(coeff,*it,w,res,contextptr);
    }
  }

  void convolutionpower(const vecteur & v,int k,vecteur & res,GIAC_CONTEXT){
    res.clear();
    // should be improved for efficiency!
    if (k==0){
      res.push_back(1);
      res.push_back(0);
      return;
    }
    if (k==1){
      res=v;
      return;
    }
    convolutionpower(v,k/2,res,contextptr);
    vecteur tmp=res;
    convolution(tmp,tmp,res,contextptr);
    if (k%2){
      tmp=res;
      convolution(tmp,v,res,contextptr);
    }
  }

  // coeff & argument of exponential
  void lin(const gen & e,vecteur & v,bool convert_sqrt,GIAC_CONTEXT){
    if (e.type!=_SYMB){
      v.push_back(e);
      v.push_back(0);
      return ; // e*exp(0)
    }
    // e is symbolic, look for exp, cosh, sinh, +, *, neg and inv, ^
    unary_function_ptr s=e._SYMBptr->sommet;
    if ((s==at_plus) && (e._SYMBptr->feuille.type==_VECT)){
      vecteur::const_iterator it=e._SYMBptr->feuille._VECTptr->begin(),itend=e._SYMBptr->feuille._VECTptr->end();
      for (;it!=itend;++it)
	lin(*it,v,convert_sqrt,contextptr);
      compress(v,contextptr);
      return;
    }
    if (s==at_neg){
      vecteur tmp;
      lin(e._SYMBptr->feuille,tmp,convert_sqrt,contextptr);
      const_iterateur it=tmp.begin(),itend=tmp.end();
      for (;it!=itend;++it){
	v.push_back(-*it);
	++it;
	v.push_back(*it);
      }
      return;
    }
    if (s==at_inv){
      vecteur w;
      lin(e._SYMBptr->feuille,w,convert_sqrt,contextptr);
      if (w.size()==2){
	v.push_back(inv(w[0],contextptr));
	v.push_back(-w[1]);
      }
      else {
	gen coeff(unlin(w,contextptr));
	v.push_back(inv(coeff,contextptr));
	v.push_back(0);
      }
      return ;
    }
    if (s==at_prod){
      if (e._SYMBptr->feuille.type!=_VECT){
	lin(e._SYMBptr->feuille,v,convert_sqrt,contextptr);
	return;
      }
      vecteur w;
      vecteur::const_iterator it=e._SYMBptr->feuille._VECTptr->begin(),itend=e._SYMBptr->feuille._VECTptr->end();
      lin(*it,w,convert_sqrt,contextptr);
      ++it;
      for (;it!=itend;++it){
	vecteur v0;
	lin(*it,v0,convert_sqrt,contextptr);
	vecteur res;
	convolution(w,v0,res,contextptr);
	w=res;
      }
      v=mergevecteur(v,w);
      return;
    }
    if (s==at_pow){
      vecteur::const_iterator it=e._SYMBptr->feuille._VECTptr->begin();
      vecteur w;
      lin(*it,w,convert_sqrt,contextptr);
      ++it;
      if (w.size()==2){
	if ( is_zero(w[1]) && (w[0].type==_INT_ && convert_sqrt) ){
	  w[1]=ln(w[0],contextptr);
	  w[0]=plus_one;
	}
	v.push_back(pow(w[0],*it,contextptr));
	v.push_back(w[1]*(*it));
	return ;
      }
      if ((it->type==_INT_) && (it->val>=0)){
	vecteur z(w),tmp;
	convolutionpower(z,it->val,tmp,contextptr);
	v=mergevecteur(v,tmp);
	compress(v,contextptr);
	return ;
      }
      gen coeff=unlin(w,contextptr);
      v.push_back(pow(coeff,*it,contextptr));
      v.push_back(0);
      return ;
    }
    gen f=_lin(convert_sqrt?e._SYMBptr->feuille:makesequence(e._SYMBptr->feuille,at_sqrt),contextptr);
    if (s==at_exp){
      v.push_back(1);
      v.push_back(f);
      return ; // 1*exp(arg)
    }
    if (s==at_cosh || s==at_sinh){
      v.push_back(plus_one_half);
      v.push_back(f);
      v.push_back(s==at_cosh?plus_one_half:minus_one_half);
      v.push_back(-f);
      return ; // 1/2*exp(arg)+-1/2*exp(-arg)
    }
    v.push_back(symbolic(s,f));
    v.push_back(0);
  }

  void lin(const gen & e,vecteur & v,GIAC_CONTEXT){
    lin(e,v,true,contextptr);
  }

  symbolic symb_lin(const gen & a){
    return symbolic(at_lin,a);
  }

  // "unary" version
  gen _lin(const gen & args_,GIAC_CONTEXT){
    gen args(args_);
    bool convert_sqrt=true;
    if (args.type==_VECT && args.subtype==_SEQ__VECT && args._VECTptr->back()==at_sqrt){
      args=args._VECTptr->front();
      convert_sqrt=false;
    }
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    gen var,res;
    if (is_algebraic_program(args,var,res))
      return symbolic(at_program,makesequence(var,0,_lin(res,contextptr)));
    if (is_equal(args))
      return apply_to_equal(args,_lin,contextptr);
    vecteur v;
    if (args.type!=_VECT){
      lin(args,v,convert_sqrt,contextptr);
      return unlin(v,contextptr);
    }
    return apply(args,_lin,contextptr);
  }
  static const char _lin_s []="lin";
  static define_unary_function_eval (__lin,&_lin,_lin_s);
  define_unary_function_ptr5( at_lin ,alias_at_lin,&__lin,0,true);

  static const char _lineariser_s []="lineariser";
  static define_unary_function_eval (__lineariser,&_lin,_lineariser_s);
  define_unary_function_ptr5( at_lineariser ,alias_at_lineariser,&__lineariser,0,true);

  // back conversion
  gen tunlin(vecteur & v,GIAC_CONTEXT){
    vecteur w;
    gen coeff;
    vecteur::const_iterator it=v.begin(),itend=v.end();
    for (;it!=itend;++it){
      coeff = *it;
      ++it;
      coeff=coeff*(*it);
      if (!is_zero(coeff))
	w.push_back(coeff);
    }
    if (w.empty())
      return 0;
    if (w.size()==1)
      return w.front();
    return symbolic(at_plus,gen(w,_SEQ__VECT));
  }

  static void tadd(vecteur & res,const gen & coeff,const gen & angle,GIAC_CONTEXT){
    gen newangle=angle,newcoeff=coeff;
    if ( (newangle.type==_SYMB) && (newangle._SYMBptr->sommet==at_neg)){
      newcoeff=-coeff;
      newangle=-newangle;
    }
    if ( (newangle.type==_SYMB) && ( (newangle._SYMBptr->sommet==at_sin) || (newangle._SYMBptr->sommet==at_cos) ) ){
      res.push_back(newcoeff);
      res.push_back(newangle);
    }
    else {
      newcoeff=newcoeff*newangle;
      if (!is_zero(newcoeff)){
	res.push_back(newcoeff);
	res.push_back(plus_one);
      }
    }
  }

  void tconvolution(const gen & coeff, const gen & arg,const vecteur & w,vecteur & res,GIAC_CONTEXT){
    gen newcoeff,tmp;
    if ((arg.type==_SYMB) && (arg._SYMBptr->sommet==at_cos)){
      vecteur::const_iterator it=w.begin(),itend=w.end();
      for (;it!=itend;++it){
	newcoeff=coeff*(*it);
	++it;
	bool iscos=it->type==_SYMB && it->_SYMBptr->sommet==at_cos;
	if ( (it->type==_SYMB) && (iscos || it->_SYMBptr->sommet==at_sin) ){
	  newcoeff=recursive_normal(rdiv(newcoeff,plus_two,contextptr),false,contextptr);
	  gen tmp1(normal(it->_SYMBptr->feuille+arg._SYMBptr->feuille,false,contextptr));
	  gen tmp2(normal(it->_SYMBptr->feuille-arg._SYMBptr->feuille,false,contextptr));
	  tadd(res,newcoeff,iscos?cos(tmp2,contextptr):sin(tmp1,contextptr),contextptr);
	  tadd(res,newcoeff,iscos?cos(tmp1,contextptr):sin(tmp2,contextptr),contextptr);
	  continue;
	}
	res.push_back(recursive_normal(newcoeff*(*it),false,contextptr));
	res.push_back(arg);
      }
      compress(res,contextptr);
      return;
    }
    if ((arg.type==_SYMB) && (arg._SYMBptr->sommet==at_sin)){
      vecteur::const_iterator it=w.begin(),itend=w.end();
      for (;it!=itend;++it){
	newcoeff=coeff*(*it);
	++it;
	bool iscos=it->type==_SYMB && it->_SYMBptr->sommet==at_cos;
	if ( (it->type==_SYMB) && (iscos || it->_SYMBptr->sommet==at_sin) ){
	  newcoeff=recursive_normal(rdiv(newcoeff,plus_two,contextptr),false,contextptr);
	  gen tmp1(normal(arg._SYMBptr->feuille+it->_SYMBptr->feuille,false,contextptr));
	  gen tmp2(normal(arg._SYMBptr->feuille-it->_SYMBptr->feuille,false,contextptr));
	  if (iscos){
	    tadd(res,newcoeff,sin(tmp1,contextptr),contextptr);
	    tadd(res,newcoeff,sin(tmp2,contextptr),contextptr);
	  }
	  else {
	    tadd(res,newcoeff,cos(tmp2,contextptr),contextptr);
	    tadd(res,-newcoeff,cos(tmp1,contextptr),contextptr);
	  }
	  continue;
	}
	res.push_back(recursive_normal(newcoeff*(*it),false,contextptr));
	res.push_back(arg);
      }
      compress(res,contextptr);
      return;
    }
    const_iterateur it=w.begin(),itend=w.end();
    newcoeff=coeff*arg;
    for (;it!=itend;++it){
      res.push_back(recursive_normal(*it*newcoeff,false,contextptr));
      ++it;
      res.push_back(*it);
    }
  }

  void tconvolution(const vecteur & v,const vecteur & w, vecteur & res,GIAC_CONTEXT){
    res.clear();
    res.reserve(res.size()+v.size()*w.size()/2);
    vecteur::const_iterator it=v.begin(),itend=v.end();
    gen coeff;
    for (;it!=itend;++it){
      coeff = *it;
      ++it;
      tconvolution(coeff,*it,w,res,contextptr);
    }
  }

  void tconvolutionpower(const vecteur & v,int k,vecteur & res,GIAC_CONTEXT){
    res.clear();
    // should be improved for efficiency!
    if (k==0){
      res.push_back(1);
      res.push_back(1);
      return;
    }
    if (k==1){
      res=v;
      return;
    }
    tconvolutionpower(v,k/2,res,contextptr);
    vecteur tmp=res;
    tconvolution(tmp,tmp,res,contextptr);
    if (k%2){
      tmp=res;
      tconvolution(tmp,v,res,contextptr);
    }
  }

  // coeff & argument of sin/cos
  void tlin(const gen & e,vecteur & v,GIAC_CONTEXT){
    if (e.type!=_SYMB){
      v.push_back(e);
      v.push_back(1);
      return ; // e*1
    }
    // e is symbolic, look for cos, sin, +, *, neg and inv, ^
    unary_function_ptr s=e._SYMBptr->sommet;
    if ( (s==at_cos) || (s==at_sin)){
      v.push_back(1);
      v.push_back(e);
      return ; 
    }
    if ((s==at_plus) && (e._SYMBptr->feuille.type==_VECT)){
      vecteur::const_iterator it=e._SYMBptr->feuille._VECTptr->begin(),itend=e._SYMBptr->feuille._VECTptr->end();
      for (;it!=itend;++it)
	tlin(*it,v,contextptr);
      compress(v,contextptr);
      return;
    }
    if (s==at_neg){
      vecteur w;
      tlin(e._SYMBptr->feuille,w,contextptr);
      const_iterateur it=w.begin(),itend=w.end();
      for (;it!=itend;++it){
	v.push_back(-*it);
	++it;
	v.push_back(*it);
      }
      return;
    }
    if (s==at_prod){
      if (e._SYMBptr->feuille.type!=_VECT){
	tlin(e._SYMBptr->feuille,v,contextptr);
	return;
      }
      vecteur::const_iterator it=e._SYMBptr->feuille._VECTptr->begin(),itend=e._SYMBptr->feuille._VECTptr->end();
      vecteur w;
      tlin(*it,w,contextptr);
      ++it;
      for (;it!=itend;++it){
	vecteur v0;
	tlin(*it,v0,contextptr);
	vecteur res;
	tconvolution(w,v0,res,contextptr);
	w=res;
      }
      v=mergevecteur(v,w);
      return;
    }
    if (s==at_pow){
      vecteur::const_iterator it=e._SYMBptr->feuille._VECTptr->begin();
      /*
      if ( (v.size()==2) && ((it+1)->type==_INT_) && ((it+1)->val>=0) ){
	tlin(*it,v);
	++it;
	return tpow(v,it->val);
      }
      */
      if (((it+1)->type==_INT_) && ((it+1)->val>=0)){
	vecteur w;
	tlin(*it,w,contextptr);
	vecteur z(w);
	++it;
	tconvolutionpower(z,it->val,w,contextptr);
	v=mergevecteur(v,w);
	return ;
      }
    }
    gen te=_tlin(e._SYMBptr->feuille,contextptr);
    if (s==at_pow && te.type==_VECT && te._VECTptr->size()==2 && te._VECTptr->back()==plus_one_half)
      v.push_back(sqrt(te._VECTptr->front(),contextptr));
    else
      v.push_back(s(te,contextptr));
    v.push_back(1);
  }

  symbolic symb_tlin(const gen & a){
    return symbolic(at_tlin,a);
  }

  // "unary" version
  gen _tlin(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    gen var,res;
    if (is_algebraic_program(args,var,res))
      return symbolic(at_program,makesequence(var,0,_tlin(res,contextptr)));
    if (is_equal(args))
      return apply_to_equal(args,_tlin,contextptr);
    vecteur v;
    if (args.type!=_VECT){
      tlin(args,v,contextptr);
      return tunlin(v,contextptr);
    }
    return apply(args,_tlin,contextptr);
  }
  static const char _tlin_s []="tlin";
  static define_unary_function_eval (__tlin,&_tlin,_tlin_s);
  define_unary_function_ptr5( at_tlin ,alias_at_tlin,&__tlin,0,true);

  static const char _lineariser_trigo_s []="lineariser_trigo";
  static define_unary_function_eval (__lineariser_trigo,&_tlin,_lineariser_trigo_s);
  define_unary_function_ptr5( at_lineariser_trigo ,alias_at_lineariser_trigo,&__lineariser_trigo,0,true);

  // Expand and texpand
  static void split(const gen & e, gen & coeff, gen & arg,GIAC_CONTEXT){
    if (e.type==_INT_){
      coeff=e;
      arg=plus_one;
    }
    if ( (e.type==_SYMB) && (e._SYMBptr->sommet==at_neg)){
      split(e._SYMBptr->feuille,coeff,arg,contextptr);
      coeff=-coeff;
      return;
    }
    if ( (e.type!=_SYMB) || (e._SYMBptr->sommet!=at_prod) ) {
      coeff=plus_one;
      arg=e;
      return;
    }
    coeff = plus_one;
    arg = plus_one;
    const_iterateur it=e._SYMBptr->feuille._VECTptr->begin(),itend=e._SYMBptr->feuille._VECTptr->end();
    for (;it!=itend;++it){
      if (it->type==_INT_)
	coeff = coeff * (*it);
      else {
	if ( (it->type==_SYMB)  && (it->_SYMBptr->sommet==at_neg)){
	  coeff = -coeff;
	  arg = arg * (it->_SYMBptr->feuille);
	}
	else
	  arg= arg * (*it);
      }
    }
    if ( (coeff.type==_INT_) && (coeff.val<0) ){
      coeff=-coeff;
      arg=-arg;
    }
  }

  /*
  gen sigma(const vecteur & v){
    // an "intelligent" version of return symbolic(at_plus,v);
    // split each element of v as an integer coeff * an arg
    vecteur vcoeff,varg,res;
    int pos;
    gen coeff,arg;
    const_iterateur it=v.begin(),itend=v.end();
    vcoeff.reserve(itend-it);
    varg.reserve(itend-it);
    for (;it!=itend;++it){
      split(*it,coeff,arg);
      pos=equalposcomp(varg,arg);
      if (!pos){
	vcoeff.push_back(coeff);
	varg.push_back(arg);
      }
      else 
	vcoeff[pos-1]=vcoeff[pos-1]+coeff;
    }
    it=vcoeff.begin(),itend=vcoeff.end();
    res.reserve(itend-it);
    const_iterateur vargit=varg.begin();
    for (;it!=itend;++it,++vargit)
      if (!is_zero(*it))
	res.push_back((*it)*(*vargit));
    if (res.empty())
      return zero;
    if (res.size()>1)
      return symbolic(at_plus,res);
    return res.front();
  }
  */

  gen prod_expand(const gen & a,const gen & b,GIAC_CONTEXT){
    bool a_is_plus= (a.type==_SYMB) && (a._SYMBptr->sommet==at_plus);
    bool b_is_plus= (b.type==_SYMB) && (b._SYMBptr->sommet==at_plus);
    if ( (!a_is_plus) && (!b_is_plus) )
      return a*b;
    if (!a_is_plus) // distribute wrt b
      return symbolic(at_plus,a*(*b._SYMBptr->feuille._VECTptr));
    if (!b_is_plus)
      return symbolic(at_plus,b*(*a._SYMBptr->feuille._VECTptr));
    // distribute wrt a AND b
    const_iterateur ita=a._SYMBptr->feuille._VECTptr->begin(),itaend=a._SYMBptr->feuille._VECTptr->end();
    const_iterateur itb=b._SYMBptr->feuille._VECTptr->begin(),itbend=b._SYMBptr->feuille._VECTptr->end();
    if ((itbend-itb)*(itaend-ita)>MAX_PROD_EXPAND_SIZE)
      return a*b;
    vecteur v;
    v.reserve((itbend-itb)*(itaend-ita));
    for (;ita!=itaend;++ita){
      for (itb=b._SYMBptr->feuille._VECTptr->begin();itb!=itbend;++itb)
	v.push_back( (*ita) * (*itb) );
    }
    return symbolic(at_plus,gen(v,_SEQ__VECT));
  }

  static gen prod_expand(const const_iterateur it,const const_iterateur itend,GIAC_CONTEXT){
    int s=int(itend-it);
    if (s==0)
      return plus_one;
    if (s==1)
      return *it;
    return _simplifier(prod_expand(prod_expand(it,it+s/2,contextptr),prod_expand(it+s/2,itend,contextptr),contextptr),contextptr);
  }
  static gen prod_expand(const gen & e_orig,GIAC_CONTEXT){
    int te=taille(e_orig,MAX_PROD_EXPAND_SIZE);
    if (te>MAX_PROD_EXPAND_SIZE)
      return symbolic(at_prod,e_orig);
    gen e=aplatir_fois_plus(expand(e_orig,contextptr));
    if (taille(e,MAX_PROD_EXPAND_SIZE)>MAX_PROD_EXPAND_SIZE)
      return symbolic(at_prod,e_orig);
    if (e.type!=_VECT)
      return e;
    // look for sommet=at_plus inside e
    return prod_expand(e._VECTptr->begin(),e._VECTptr->end(),contextptr);
  }

  static gen prod_expand_nosimp(const const_iterateur it,const const_iterateur itend,GIAC_CONTEXT){
    int s=int(itend-it);
    if (s==0)
      return plus_one;
    if (s==1)
      return *it;
    return prod_expand(prod_expand_nosimp(it,it+s/2,contextptr),prod_expand_nosimp(it+s/2,itend,contextptr),contextptr);
  }
  static gen prod_expand_nosimp(const gen & e_orig,GIAC_CONTEXT){
    gen e=aplatir_fois_plus(e_orig);
    if (e.type!=_VECT)
      return e;
    // look for sommet=at_plus inside e
    return prod_expand_nosimp(e._VECTptr->begin(),e._VECTptr->end(),contextptr);
  }

  static void pow_expand_add_res(vector<gen> & factn,int pos,int sumexpo,const gen & coeff,const vecteur & w,const gen & p, int k,int n,vecteur & res,GIAC_CONTEXT){
    if (sumexpo==k){
      // End recursion
      res.push_back(coeff*p);
      return;
    }
    if (pos==n-1){
      // End recursion
      res.push_back(coeff/factn[k-sumexpo]*p*expand(pow(w[pos],k-sumexpo),contextptr));
      return;
    }
    for (int i=k-sumexpo;i>=0;--i){
      pow_expand_add_res(factn,pos+1,sumexpo+i,coeff/factn[i],w,expand(p*pow(w[pos],i),contextptr),k,n,res,contextptr);
    }
  }

  static gen expand_pow_expand(const gen & e,GIAC_CONTEXT){
    if (e.type!=_VECT || e._VECTptr->size()!=2)
      return e;
    vecteur & v=*e._VECTptr;
    gen base=expand(v[0],contextptr);
    gen exponent=expand(v[1],contextptr);
    if (v[1].is_symb_of_sommet(at_plus) && v[1]._SYMBptr->feuille.type==_VECT){
      vecteur & w=*v[1]._SYMBptr->feuille._VECTptr;
      const_iterateur it=w.begin(),itend=w.end();
      vecteur prodarg;
      prodarg.reserve(itend-it);
      for (;it!=itend;++it){
	prodarg.push_back(pow(base,*it,contextptr));
      }
      return _prod(prodarg,contextptr);
    }
    if (v[1].type==_INT_ ){
      if (v[0].is_symb_of_sommet(at_prod)&& v[0]._SYMBptr->feuille.type==_VECT){
	vecteur w(*v[0]._SYMBptr->feuille._VECTptr);
	iterateur it=w.begin(),itend=w.end();
	for (;it!=itend;++it){
	  *it=pow(expand(*it,contextptr),v[1],contextptr);
	}
	return _prod(w,contextptr);
      }
    }
    if (v[0].is_symb_of_sommet(at_plus) && v[0]._SYMBptr->feuille.type==_VECT && v[1].type==_INT_ && v[1].val>=0){
      int k=v[1].val;
      if (!k)
	return plus_one;
      if (k==1)
	return base;
      vector<gen> factn(k+1);
      factn[0]=1;
      for (int i=1;i<=k;++i){
	factn[i]=i*factn[i-1];
      }
      // (x1+...+xn)^k -> sum_{j1+...+jn=k} k!/(j1!j2!...jn!) x^j1 *... *x^jk
      vecteur & w=*v[0]._SYMBptr->feuille._VECTptr;
      int n=int(w.size());
      if (!n)
	return gensizeerr(contextptr);
      if (std::pow(double(n),double(k))>MAX_PROD_EXPAND_SIZE)
	return recursive_ratnormal(symb_pow(v[0],v[1]),contextptr);
      vecteur res;
      gen p;
      for (int i=k;i>=0;--i){
	p=expand(pow(w[0],i),contextptr);
	pow_expand_add_res(factn,1,i,factn[k]/factn[i],w,p,k,n,res,contextptr);
      }
      return symbolic(at_plus,res);
    }
    if (v[0].is_symb_of_sommet(at_prod) && v[0]._SYMBptr->feuille.type==_VECT){
      const vecteur & vb=*v[0]._SYMBptr->feuille._VECTptr;
      gen r1(1),r2(1);
      for (int i=0;i<vb.size();++i){
	if (fastsign(vb[i],contextptr)==1)
	  r1=r1*pow(vb[i],exponent,contextptr);
	else
	  r2=r2*vb[i];
      }
      if (r1!=1)
	return r1*pow(r2,exponent,contextptr);
    }
    return symb_pow(base,exponent);
  }

  static gen expand_neg_expand(const gen & g_orig,GIAC_CONTEXT){
    gen g=expand(g_orig,contextptr);
    return -g;
  }

  vecteur tchebycheff(int n,bool first_kind){
    vecteur v0,v1,vtmp;
    if (first_kind) {
      v0.push_back(1);
      v1.push_back(1);
      v1.push_back(0);
    }
    else
      v1.push_back(1);
    if (!n)
      return v0;
    if (n==1)
      return v1;
    for (--n;n;--n){
      multvecteur(2,v1,vtmp);
      vtmp.push_back(0);
      vtmp=vtmp-v0;
      v0=v1;
      v1=vtmp;
    }
    return v1; 
  }

  gen exp_expand(const gen & e,GIAC_CONTEXT){
    if (e.type!=_SYMB)
      return exp(e,contextptr);
    if (e._SYMBptr->sommet==at_plus)
      return symbolic(at_prod,apply(e._SYMBptr->feuille,exp_expand,contextptr));
    gen coeff,arg;
    split(e,coeff,arg,contextptr);
    return pow(exp(arg,contextptr),coeff,contextptr);
  }

  static gen ln_expand0(const gen & e,GIAC_CONTEXT){
    if (e.type==_FRAC)
      return ln(e._FRACptr->num,contextptr)-ln(e._FRACptr->den,contextptr);
    if (e.type!=_SYMB)
      return ln(e,contextptr);
    if (e._SYMBptr->sommet==at_prod)
      return _plus(apply(e._SYMBptr->feuille,ln_expand0,contextptr),contextptr);//symbolic(at_plus,apply(e._SYMBptr->feuille,ln_expand0,contextptr));
    if (e._SYMBptr->sommet==at_inv)
      return -ln_expand0(e._SYMBptr->feuille,contextptr);
    if (e._SYMBptr->sommet==at_pow){
      gen & tmp=e._SYMBptr->feuille;
      if (tmp.type==_VECT && tmp._VECTptr->size()==2){
	gen base=tmp._VECTptr->front(),expo=tmp._VECTptr->back();
	if (!complex_mode(contextptr) && expo.type==_INT_ && expo.val%2==0)
	  base=abs(base,contextptr);
	return expo*ln_expand0(base,contextptr);
      }
    }
    return ln(e,contextptr);
  }

  gen ln_expand(const gen & e0,GIAC_CONTEXT){
    gen e(factor(e0,false,contextptr));
    return ln_expand0(e,contextptr);
  }

  gen symhorner(const vecteur & v, const gen & e){
    if (v.empty())
      return zero;
    if (is_zero(e))
      return v.back();
    gen res=zero;
    const_iterateur it=v.begin(),itend=v.end();
    int n=int(itend-it)-1;
    for (;it!=itend;++it,--n)
      res = res + (*it)*pow(e,n);
    return res;
  }

  static gen cos_expand(const gen & e,GIAC_CONTEXT);
  static gen sin_expand(const gen & e,GIAC_CONTEXT){
    if (e.type!=_SYMB)
      return sin(e,contextptr);
    if (lidnt(e)==vecteur(1,cst_pi)){
      gen sine=sin(e,contextptr);
      if (!contains(lidnt(sine),cst_pi))
	return sine;
    }
    if (e._SYMBptr->sommet==at_plus){
      vecteur v=*e._SYMBptr->feuille._VECTptr;
      gen last=v.back(),first;
      v.pop_back();
      if (v.size()==1)
	first=v.front();
      else
	first=symbolic(at_plus,v);
      return sin_expand(first,contextptr)*cos_expand(last,contextptr)+cos_expand(first,contextptr)*sin_expand(last,contextptr);
    }
    if (e._SYMBptr->sommet==at_neg)
      return -sin_expand(e._SYMBptr->feuille,contextptr);
    gen coeff,arg;
    split(e,coeff,arg,contextptr);
    if (!is_one(coeff) && coeff.type==_INT_ && coeff.val<max_texpand_expansion_order)
      return symhorner(tchebycheff(coeff.val,false),cos(arg,contextptr))*sin(arg,contextptr);
    else
      return sin(e,contextptr);
  }

  static gen cos_expand(const gen & e,GIAC_CONTEXT){
    if (e.type!=_SYMB)
      return cos(e,contextptr);
    if (lidnt(e)==vecteur(1,cst_pi)){
      gen cose=cos(e,contextptr);
      if (!contains(lidnt(cose),cst_pi))
	return cose;
    }
    if (e._SYMBptr->sommet==at_plus){
      vecteur v=*e._SYMBptr->feuille._VECTptr;
      gen last=v.back(),first;
      v.pop_back();
      if (v.size()==1)
	first=v.front();
      else
	first=symbolic(at_plus,v);
      return cos_expand(first,contextptr)*cos_expand(last,contextptr)-sin_expand(first,contextptr)*sin_expand(last,contextptr);
    }
    if (e._SYMBptr->sommet==at_neg)
      return cos_expand(e._SYMBptr->feuille,contextptr);
    gen coeff,arg;
    split(e,coeff,arg,contextptr);
    if (!is_one(coeff) && coeff.type==_INT_ && coeff.val<max_texpand_expansion_order)
      return symhorner(tchebycheff(coeff.val,true),cos(arg,contextptr));
    else
      return cos(e,contextptr);
  }

  static gen sin2tancos(const gen & g,GIAC_CONTEXT){
    return symb_cos(g)*symb_tan(g);
  }

  static gen even_pow_cos2tan(const gen & g,GIAC_CONTEXT){
    if ( (g.type!=_VECT) || (g._VECTptr->size()!=2) )
      return g;
    gen a(g._VECTptr->front()),b(g._VECTptr->back());
    if ( (b.type!=_INT_) || (b.val%2) || (a.type!=_SYMB) || (a._SYMBptr->sommet!=at_cos) )
      return symbolic(at_pow,g);
    int i=b.val/2;
    return pow(plus_one+pow(symb_tan(a._SYMBptr->feuille),plus_two,contextptr),-i,contextptr);
  }

  static gen tan_expand(const gen & e,GIAC_CONTEXT){
    if (e.type!=_SYMB)
      return tan(e,contextptr);
    if (lidnt(e)==vecteur(1,cst_pi)){
      gen tane=tan(e,contextptr);
      if (!contains(lidnt(tane),cst_pi))
	return tane;
    }
    if (e._SYMBptr->sommet==at_plus){
      vecteur v=*e._SYMBptr->feuille._VECTptr;
      gen last=v.back(),first;
      v.pop_back();
      if (v.size()==1)
	first=v.front();
      else
	first=symbolic(at_plus,v);
      gen ta=tan_expand(first,contextptr);
      gen tb=tan_expand(last,contextptr);
      if (is_inf(ta))
	return -inv(tb,contextptr);
      if (is_inf(tb))
	return -inv(ta,contextptr);
      return rdiv(ta+tb,1-ta*tb,contextptr);
    }
    if (e._SYMBptr->sommet==at_neg)
      return -tan_expand(e._SYMBptr->feuille,contextptr);
    gen coeff,arg;
    split(e,coeff,arg,contextptr);
    if (!is_one(coeff) && coeff.type==_INT_ && coeff.val<max_texpand_expansion_order){
      gen g=rdiv(symhorner(tchebycheff(coeff.val,false),cos(arg,contextptr))*sin(arg,contextptr),symhorner(tchebycheff(coeff.val,true),cos(arg,contextptr)),contextptr);
      vector<const unary_function_ptr *> v;
      vector< gen_op_context > w;
      v.push_back(at_sin);
      w.push_back(&sin2tancos);
      g=subst(g,v,w,false,contextptr);
      v[0]=at_pow;
      w[0]=(&even_pow_cos2tan);
      g=subst(recursive_normal(g,false,contextptr),v,w,false,contextptr);      
      return recursive_normal(g,false,contextptr);
    }
    else
      return tan(e,contextptr);
  }

  symbolic symb_texpand(const gen & a){
    return symbolic(at_texpand,a);
  }

  // "unary" version
  gen _texpand(const gen & args,GIAC_CONTEXT){
    if ( args.type==_STRNG && args.subtype==-1) return  args;
    gen var,res;
    if (is_algebraic_program(args,var,res))
      return symbolic(at_program,makesequence(var,0,_texpand(res,contextptr)));
    if (is_equal(args))
      return apply_to_equal(args,_texpand,contextptr);
    vector<const unary_function_ptr *> v;
    vector< gen_op_context > w;
    v.push_back(at_exp);
    w.push_back(&exp_expand);
    v.push_back(at_ln);
    w.push_back(&ln_expand);
    v.push_back(at_prod);
    w.push_back(&prod_expand);
    v.push_back(at_sin);
    w.push_back(&sin_expand);
    v.push_back(at_cos);
    w.push_back(&cos_expand);
    v.push_back(at_tan);
    w.push_back(&tan_expand);
    return subst(args,v,w,false,contextptr);
  }
  static const char _texpand_s []="texpand";
  static define_unary_function_eval (__texpand,&_texpand,_texpand_s);
  define_unary_function_ptr5( at_texpand ,alias_at_texpand,&__texpand,0,true);

  static const char _developper_transcendant_s []="developper_transcendant";
  static define_unary_function_eval (__developper_transcendant,&_texpand,_developper_transcendant_s);
  define_unary_function_ptr5( at_developper_transcendant ,alias_at_developper_transcendant,&__developper_transcendant,0,true);

  gen ineq2diff(const gen & g){
    if (g.type!=_SYMB) return g;
    if (g._SYMBptr->sommet==at_superieur_strict || g._SYMBptr->sommet==at_superieur_egal){
      vecteur & v=*g._SYMBptr->feuille._VECTptr;
      return v[0]-v[1];
    }
    if (g._SYMBptr->sommet==at_inferieur_strict || g._SYMBptr->sommet==at_inferieur_egal){
      vecteur & v=*g._SYMBptr->feuille._VECTptr;
      return v[1]-v[0];
    }
    return g;
  }

  gen factor_ineq(const gen & g_,GIAC_CONTEXT){
    if (!is_inequation(g_))
      return g_;
    gen g=ineq2diff(g_);
    bool sup=true;
    vecteur h=gen2vecteur(_factors(g,contextptr));
    // remove odd multiplicities and ignore even 
    vecteur v;
    for (int i=1;i<h.size();i+=2){
      gen hi=h[i-1]; // remove content if any
      if (h[i].val % 2){
	if (hi.type < _IDNT){
	  if (!is_positive(hi,contextptr))
	    sup=!sup;
	}
	else {
	  gen shi=_sign(hi,contextptr);
	  if (shi==1)
	    continue;
	  if (shi==-1)
	    sup=!sup;
	  v.push_back(hi);
	}
      }
    }
    vecteur res1(v.size()),res2(v.size());
    for (int i=0;i<v.size();++i){
      res1[i]=symbolic(at_superieur_egal,v[i],0);
      res2[i]=symbolic(at_inferieur_egal,v[i],0);
    }
    if (v.size()==0){
      // *logptr(contextptr) << (sup?gettext("Inequation is true almost everywhere"):gettext("Equation is false almost everywhere")) << "\n";
      return sup?1:0;
    }
    if (v.size()==1){
      return sup?res1[0]:res2[0];
    }
    if (v.size()==2){
      gen res;
      if (sup)
	res=symbolic(at_ou,makesequence(
					   symbolic(at_and,makesequence(res1[0],res1[1])),
					   symbolic(at_and,makesequence(res2[0],res2[1]))
					));
      else
	res=symbolic(at_ou,makesequence(
					 symbolic(at_and,makesequence(res1[0],res2[1])),
					 symbolic(at_and,makesequence(res2[0],res1[1]))
					 ));
      return res;
    }
    int n=v.size();
    int N=1<<n;
    vector<bool> vb(n);
    vecteur res;
    for (int i=0;i<N;++i){
      // convert i base 2
      vb.clear();
      int i_=i,count=0;
      while (i_){ 
	bool b=i_%2;
	vb.push_back(b);
	i_ /= 2;
	if (b) count++;
      }
      // check if the number of 1 matches superieur
      if (sup){
	if (count % 2)
	  continue;
      }
      else {
	if (count % 2==0)
	  continue;
      }
      // match found
      while (vb.size()<n) vb.push_back(false);
      // create and symbolic
      vecteur tmp;
      for (int j=0;j<n;++j){
	tmp.push_back(vb[j]?res2[j]:res1[j]);
      }
      res.push_back(symbolic(at_and,gen(tmp,_SEQ__VECT)));
    }
    return symbolic(at_ou,gen(res,_SEQ__VECT));
  }

  vecteur andor2list(const gen & g_,GIAC_CONTEXT){
    if (g_.type!=_SYMB)
      return vecteur(1,vecteur(1,g_));
    gen g(g_);
    if (is_inequation(g))
      g=factor_ineq(g,contextptr);
    if (g._SYMBptr->sommet==at_ou){
      vecteur args(gen2vecteur(g._SYMBptr->feuille));
      int n=int(args.size());
      vecteur res;
      for (int i=0;i<n;++i){
	vecteur v(andor2list(args[i],contextptr));
	int l=int(v.size());
	for (int j=0;j<l;++j)
	  res.push_back(v[j]);
      }
      return res;
    }
    if (g._SYMBptr->sommet==at_and){
      vecteur args(gen2vecteur(g._SYMBptr->feuille));
      int n=int(args.size());
      vecteur res;
      longlong N=1;
      for (int i=0;i<n;++i){
	vecteur v(andor2list(args[i],contextptr));
	N*=v.size(); // res.size() at end of iteration
	if (N>RAND_MAX)
	  return vecteur(1,vecteur(1,gendimerr(contextptr)));
	if (i==0){
	  res.swap(v); continue;
	}
	vecteur newres; newres.reserve(N);
	// "multiply" element by element between res and v
	for (size_t I=0;I<res.size();++I){
	  for (size_t J=0;J<v.size();++J){
	    newres.push_back(mergevecteur(*res[I]._VECTptr,*v[J]._VECTptr));
	  }
	}
	res.swap(newres);
      }
      return res;
    }
    return vecteur(1,vecteur(1,g));
  }

  gen list2and(const gen & g){
    if (g.type!=_VECT)
      return g;
    if (g._VECTptr->empty())
      return 1;
    if (g._VECTptr->size()==1)
      return g;
    gen G=g; G.subtype=_SEQ__VECT;
    return symbolic(at_and,G);
  }

  gen list2orand(const vecteur & v){
    if (v.empty())
      return 1;
    if (v.size()==1)
      return list2and(v.front());
    vecteur w(v);
    for (int i=0;i<w.size();++i){
      w[i]=list2and(w[i]);
    }
    return symbolic(at_ou,gen(w,_SEQ__VECT));
  }

  bool are_inequations(const gen & g){
    if (g.type!=_VECT)
      return is_inequation(g);
    const vecteur & v=*g._VECTptr;
    size_t N=v.size();
    for (size_t i=0;i<N;++i){
      if (!are_inequations(v[i]))
	return false;
    }
    return true;
  }

  // returns true if v is a list of linear inequations, write them in a matrix
  // w=[...,[a,b,c],...] where a*x+b*y+c>=0 (> is replaced by >=) 
  bool and2mat(const vecteur & v,const gen &x,const gen &y,matrice &w,GIAC_CONTEXT){
    w.clear();
    for (size_t i=0;i<v.size();++i){
      gen g=v[i];
      if (!is_inequation(g))
	return false;
      g=ineq2diff(g);
      gen a,tmp,b,c;
      if (!is_linear_wrt(g,x,a,tmp,contextptr))
	return false;
      if (!is_linear_wrt(tmp,y,b,c,contextptr))
	return false;
      if (evalf_double(a,1,contextptr).type!=_DOUBLE_ || evalf_double(b,1,contextptr).type!=_DOUBLE_ || evalf_double(c,1,contextptr).type!=_DOUBLE_)
	return false;
      w.push_back(makevecteur(a,b,c));
    }
    return true;
  }

  // compute list of intersections points [x,y]
  // update x/ymin..max,1st call set xmin/ymin to 1e307 and xmax/ymax to -1e307 
  vecteur lin_ineq_inter(const matrice & m,double & xmin,double &xmax,double & ymin,double & ymax,GIAC_CONTEXT){
    size_t N=m.size();
    vecteur res; res.reserve((N*(N+1))/2);
    for (size_t i=0;i<N;++i){
      vecteur v1(gen2vecteur(m[i]));
      if (v1.size()!=3) return vecteur(1,gensizeerr(contextptr));
      gen a(v1[0]),b(v1[1]),c1(v1[2]);
      for (size_t j=i+1;j<N;++j){
	vecteur v2(gen2vecteur(m[j]));
	if (v2.size()!=3) return vecteur(1,gensizeerr(contextptr));
	gen c(v2[0]),d(v2[1]),c2(v2[2]);
	gen D=ratnormal(a*d-b*c,contextptr);
	if (is_zero(D)) continue; // parallel
	// solve([a*x+b*y+c1,c*x+d*y+c2],[x,y])
	gen x=(b*c2-c1*d)/D,y=(c*c1-a*c2)/D;
	// check that x,y verifies equations
	vecteur mxy=multmatvecteur(m,makevecteur(x,y,1));
	size_t pos=0;
	for (;pos<mxy.size();++pos){
	  if (!is_positive(mxy[pos],contextptr))
	    break;
	}
	if (pos<mxy.size())
	  continue;
	gen add(makevecteur(x,y));
	if (!equalposcomp(res,add))
	  res.push_back(add);
	x=evalf_double(x,1,contextptr);
	double xd=x._DOUBLE_val;
	if (xd>xmax)
	  xmax=xd;
	if (xd<xmin)
	  xmin=xd;
	y=evalf_double(y,1,contextptr);
	double yd=y._DOUBLE_val;
	if (yd>ymax)
	  ymax=yd;
	if (yd<ymin)
	  ymin=yd;
      }
    }
    return res;
  }

  void fill_attributs(vecteur & attr){
    if (!attr.empty() && attr[0].type==_INT_)
      attr[0] = attr[0].val | _FILL_POLYGON;
  }

  static void add_autoscale(vecteur & res,double xmin,double xmax,double ymin,double ymax,bool ortho=true){
    if (ortho){
      double dy=ymax-ymin,dx=xmax-xmin,r=dy/dx;
      if (r>0.25 && r<4){
	res.insert(res.begin(),symb_equal(change_subtype(_GL_ORTHO,_INT_PLOT),1));
	if (r>1)
	  res.insert(res.begin(),symb_equal(change_subtype(_GL_X,_INT_PLOT),symb_interval(xmin,xmax)));
	else
	  res.insert(res.begin(),symb_equal(change_subtype(_GL_Y,_INT_PLOT),symb_interval(ymin,ymax)));
	return;
      }
    }
    res.insert(res.begin(),symb_equal(change_subtype(_GL_X,_INT_PLOT),symb_interval(xmin,xmax)));
    res.insert(res.begin(),symb_equal(change_subtype(_GL_Y,_INT_PLOT),symb_interval(ymin,ymax)));
  }

  static void adjust(double & xmin,double & xmax,double & ymin,double & ymax,vecteur & res,bool zout){
    if (xmin==1e307) xmin=gnuplot_xmin;
    if (ymin==1e307) ymin=gnuplot_ymin;
    if (xmax==-1e307) xmax=gnuplot_xmax;
    if (ymax==-1e307) ymax=gnuplot_ymax;
    double dx=gnuplot_xmax-gnuplot_xmin,dy=gnuplot_ymax-gnuplot_ymin;
    if (xmax>xmin)
      dx=xmax-xmin;
    if (dx<1e-300)
      dx=1e-300;
    if (ymax>ymin)
      dy=ymax-ymin;
    if (dy<1e-300)
      dy=1e-300;
    // axes zoomeout factor
    double z=0.25;
    if (zout){
      xmin -= z*dx; xmax += z*dx;
      ymin -= z*dy; ymax += z*dy;
    }
    add_autoscale(res,xmin,xmax,ymin,ymax);
    if (zout){
      // zoomout factor
      z=6;
      xmin -= z*dx; xmax += z*dx;
      ymin -= z*dy; ymax += z*dy;
    }
  }

  // v should be a union of intersections, as returned by andor2list
  bool lin_ineq_plot(const vecteur & vsymb,const gen & x,const gen &y,const vecteur & attr_,vecteur & res,GIAC_CONTEXT){
    vecteur attr(attr_);
    fill_attributs(attr);
    double xmin=1e307,ymin=1e307,xmax=-1e307,ymax=-1e307;
    vecteur v(vsymb.size());
    for (size_t i=0;i<v.size();++i){
      matrice vi;
      if (!and2mat(gen2vecteur(vsymb[i]),x,y,vi,contextptr))
	return false;
      v[i]=vi;
    }
    matrice inter(vsymb.size()); // one list of intersection points for each element of v
    for (size_t i=0;i<v.size();++i){
      inter[i]=lin_ineq_inter(gen2vecteur(v[i]),xmin,xmax,ymin,ymax,contextptr);
    }
    adjust(xmin,xmax,ymin,ymax,res,true);
    // add border equations, find again intersections (lazy version, should be optimized)
    for (size_t i=0;i<v.size();++i){
      vecteur w(gen2vecteur(v[i]));
      w.push_back(makevecteur(1,0,-xmin)); // x-xmin>=0
      w.push_back(makevecteur(-1,0,xmax)); // -x+xmax>=0
      w.push_back(makevecteur(0,1,-ymin)); // y-ymin>=0
      w.push_back(makevecteur(0,-1,ymax)); // -y+ymax>=0
      inter[i]=lin_ineq_inter(w,xmin,xmax,ymin,ymax,contextptr);
    }
    // compute convexhull for each v[i]
    for (size_t i=0;i<inter.size();++i){
      vecteur cur(gen2vecteur(inter[i]));
      for (size_t j=0;j<cur.size();++j){
	cur[j]=cur[j][0]+cst_i*cur[j][1];
      }
      gen convhull=_convexhull(gen(cur,_SEQ__VECT),contextptr);
      vecteur argv=gen2vecteur(convhull);
      argv.push_back(argv.front());
      convhull=pnt_attrib(gen(argv,_GROUP__VECT),attr,contextptr);
      res.push_back(convhull);
    }
    return true;
  }

  // returns true if v is a list of polynomial inequations, write them in
  // "normal form" eq>=0 -> eq   
  bool and2listdiff(const vecteur & v,const gen &x,const gen &y,matrice &w,GIAC_CONTEXT){
    w.clear();
    for (size_t i=0;i<v.size();++i){
      gen g=v[i];
      if (!is_inequation(g))
	return false;
      g=ineq2diff(g);
      if (is_zero(_is_polynomial(makesequence(g,x),contextptr)))
	return false;
      if (is_zero(_is_polynomial(makesequence(g,y),contextptr)))
	return false;
      w.push_back(g);
    }
    return true;
  }

  void adjust_xy(const gen &g,double &xmin,double &xmax,double &ymin,double &ymax,GIAC_CONTEXT){
    if (g.is_symb_of_sommet(at_pnt))
      return adjust_xy(remove_at_pnt(g),xmin,xmax,ymin,ymax,contextptr);
    if (g.type==_VECT){
      const vecteur & v=*g._VECTptr;
      for (int i=0;i<v.size();++i)
	adjust_xy(v[i],xmin,xmax,ymin,ymax,contextptr);
      return;
    }
    if (g.is_symb_of_sommet(at_curve)){
      vecteur p=gen2vecteur(g[2]); gen x,y;
      for (int j=0;j<p.size();++j){
	reim(p[j],x,y,contextptr);
	if (x.type==_DOUBLE_){
	  double xd=x._DOUBLE_val;
	  if (xd<xmin) xmin=xd;
	  if (xd>xmax) xmax=xd;
	}
	if (y.type==_DOUBLE_){
	  double yd=y._DOUBLE_val;
	  if (yd<ymin) ymin=yd;
	  if (yd>ymax) ymax=yd;
	}
      }
    }
  }

  bool is_ok(const vecteur &m,int i,const gen & vars,const gen &cur,GIAC_CONTEXT){
    gen c(cur),x,y;
    if (c.type!=_VECT){
      reim(c,x,y,contextptr);
      c=makevecteur(x,y);
    }
    // check that cur verifies inequations>=0 inside m
    for (size_t pos=0;pos<m.size();++pos){
      if (pos==i) 
	continue;
      gen val=subst(m[pos],vars,c,false,contextptr);
      val=evalf_double(val,1,contextptr);
      // 1e-7 test should be improved by checking values near c to
      // determine sensitivity
      if (val.type==_DOUBLE_ && val._DOUBLE_val<-5e-7) 
	return false;
    }
    return true;
  }

  // return true if p_ was inserted in a loop at begin/end
  // false otherwise
  bool insert(const gen & p_,vecteur & branches,GIAC_CONTEXT){
    gen p(p_);
    if (p.type==_VECT && p._VECTptr->size()==2)
      p=p._VECTptr->front()+cst_i*p._VECTptr->back();
    // find nearest polygonal line from p in branches
    // insert p in this line, cut and insert the two polygonal lines
    gen mind(plus_inf); int b=-1; int pos=-1;
    for (int i=0;i<branches.size();++i){
      vecteur br=gen2vecteur(branches[i]);
      for (int j=1;j<br.size();++j){
	gen br1=br[j-1],br2=br[j];
	gen t=projection(br1,br2,p,contextptr);
	if (is_positive(t,contextptr) && is_greater(1,t,contextptr)){
	  gen proj=br1+t*(br2-br1);
	  gen curd=distance2pp(proj,p,contextptr);
	  if (is_greater(mind,curd,contextptr)){
	    mind=curd;
	    b=i;
	    pos=j-1;
	  }
	}
      }
    }
    if (b>=0){
      vecteur br=gen2vecteur(branches[b]);
      if (pos>=0 && pos+1<br.size()){
	// if br is closed, insert intersection point at begin/end
	if (br[0]==br.back()  && branches[b].subtype!=-1 ){
	  vecteur addbr=vecteur(br.begin()+pos+1,br.end());
	  addbr.insert(addbr.begin(),p);
	  for (int i=1;i<pos;++i)
	    addbr.push_back(br[i]);
	  addbr.push_back(p);
	  branches[b]=gen(addbr,-1);
	  return true;
	}
	else {
	  vecteur addbr=vecteur(br.begin()+pos+1,br.end());
	  br=vecteur(br.begin(),br.begin()+pos+1);
	  br.push_back(p);
	  addbr.insert(addbr.begin(),p);
	  branches.push_back(addbr);
	  branches[b]=br;
	}
      }
    }
    return false;
  }

  // fit polygonal line to domain
  int cut(const gen & eq,const gen & vx,const gen & vy,vecteur & v,double xmin,double xmax,double ymin,double ymax,vector<double> & xymin,vector<double> & xymax, vector<double> & yxmin,vector<double> & yxmax,GIAC_CONTEXT){
    if (v.size()<2)
      return -1;
    gen g=v[0],x,y; double xd,yd,lastxd,lastyd;
    reim(g,x,y,contextptr);
    x=evalf_double(x,1,contextptr); y=evalf_double(y,1,contextptr);
    if (x.type!=_DOUBLE_ || y.type!=_DOUBLE_) return -1;
    lastxd=xd=x._DOUBLE_val; lastyd=yd=y._DOUBLE_val;
    if (xd>=xmin && xd<=xmax && yd>=ymin && yd<=ymax){
      if ((xd==xmin || xd==xmax) && (yd==ymin || yd==ymax)){
	*logptr(contextptr) << "Please change border, [" << xd << "," << yd << "] verifies " << eq << '\n';
	return -1;
      }
      if (xd==xmin)
	yxmin.push_back(yd); 
      if (xd==xmax)
	yxmax.push_back(yd); 
      if (yd==ymin)
	xymin.push_back(xd); 
      if (yd==ymax)
	xymax.push_back(xd); 
      return 0;
    }
    double eps=epsilon(contextptr);
    for (int k=1;k<v.size();++k){
      g=v[k];
      reim(g,x,y,contextptr);
      x=evalf_double(x,1,contextptr); y=evalf_double(y,1,contextptr);
      if (x.type!=_DOUBLE_ || y.type!=_DOUBLE_) return -1;
      xd=x._DOUBLE_val; yd=y._DOUBLE_val;
      if (xd>=xmin && xd<=xmax && yd>=ymin && yd<=ymax){
	v.erase(v.begin(),v.begin()+k);
	// insert edge point by solving eq==0
	bool is_xmin=lastxd<xmin,is_xmax=lastxd>xmax;
	if (is_xmin || is_xmax){
	  gen eqy=subst(eq,vx,is_xmin?xmin:xmax,false,contextptr);
	  vecteur xx(gen2vecteur(_sort(makevecteur(lastyd,yd),contextptr)));
	  gen sol=_fsolve(makesequence(eqy,vy,xx,_BISECTION_SOLVER,100*eps),contextptr);
	  if (!get_sol(sol,contextptr))
	    return -1;
	  double Y=sol._DOUBLE_val;
	  if (is_xmin)
	    yxmin.push_back(Y);
	  else
	    yxmax.push_back(Y);
	  v.insert(v.begin(),gen(is_xmin?xmin:xmax,Y));
	  return k;
	}
	bool is_ymin=lastyd<ymin,is_ymax=lastyd>ymax;
	if (is_ymin || is_ymax){
	  gen eqx=subst(eq,vy,is_ymin?ymin:ymax,false,contextptr);
	  vecteur yy(gen2vecteur(_sort(makevecteur(lastxd,xd),contextptr)));
	  gen sol=_fsolve(makesequence(eqx,vx,yy,_BISECTION_SOLVER,100*eps),contextptr);
	  if (!get_sol(sol,contextptr))
	    return -1;
	  double X=sol._DOUBLE_val;
	  if (is_ymin)
	    xymin.push_back(X);
	  else
	    xymax.push_back(X);
	  v.insert(v.begin(),gen(X,is_ymin?ymin:ymax));
	  return k;
	}
	return -1; // should never be reached
      }
      lastxd=xd; lastyd=yd;
    }
    return 0;
  }

  int poly_ineq_inter(vecteur & m,matrice & res,gen & curves,const gen &vx,const gen & vy,double & xmin,double &xmax,double & ymin,double & ymax,vecteur & cache,GIAC_CONTEXT){
    gen vars(makevecteur(vx,vy));
    size_t N=m.size();
    // find intersection points
    for (size_t i=0;i<N;++i){
      gen v1(m[i]);
      vecteur line;
      for (size_t j=0;j<i;++j)
	line.push_back(res[j][i]); // curve[i] inter cur[j] : exchange order
      line.push_back(vecteur(0)); // empty vecteur
      for (size_t j=i+1;j<N;++j){
	gen v2(m[j]);
	if (v1==v2){
	  line.push_back(vecteur(0));
	  continue;
	}
	// solve([v1,v2],[x,y])
	gen mij=_solve(makesequence(makevecteur(v1,v2),vars),contextptr);
	vecteur sol=gen2vecteur(mij),newsol;
	for (int k=0;k<sol.size();++k){
	  gen cur=sol[k];
	  if (cur.type!=_VECT || cur._VECTptr->size()!=2)
	    continue;
	  // check that x,y verifies equations
	  size_t pos=0;
	  for (;pos<N;++pos){
	    if (pos==i || pos==j) continue; // it's 0 there
	    gen val=subst(m[pos],vars,cur,false,contextptr);
	    if (!is_positive(val,contextptr))
	      break;
	  }
	  if (pos<N)
	    continue;
	  newsol.push_back(cur);
	  gen x=cur[0],y=cur[1];
	  // intersection must be transversal, otherwise algorithm does not work
	  gen dv1(derive(v1,vars,contextptr)),dv2(derive(v2,vars,contextptr));
	  dv1=subst(dv1,vars,cur,false,contextptr),dv2=subst(dv2,vars,cur,false,contextptr);
	  gen chk=_det(makesequence(dv1,dv2),contextptr);
	  if (is_zero(chk,contextptr))
	    return -1;
	  x=evalf_double(x,1,contextptr);
	  double xd=x._DOUBLE_val;
	  if (xd>xmax)
	    xmax=xd;
	  if (xd<xmin)
	    xmin=xd;
	  y=evalf_double(y,1,contextptr);
	  double yd=y._DOUBLE_val;
	  if (yd>ymax)
	    ymax=yd;
	  if (yd<ymin)
	    ymin=yd;
	} // end k loop
	line.push_back(newsol);
      } // end j loop
      res.push_back(line);
    } // end i loop
    // FIXME add code to detect multiplicities (intersection points should be for 2 curves not more)
    vecteur C(N),P(N);
    // check for known curves (line/conics), for other curves -> implicitplot
    for (int i=0;i<N;++i){
      gen g;
      if (equation2geo2d(m[i],vx,vy,g,-3.125,3.125,0.0625,undef,3,contextptr)){
	// adjust xmin/xmax/ymin/ymax 
	adjust_xy(g,xmin,xmax,ymin,ymax,contextptr);
	C[i]=g;
      }
    }
    // modify xmin..xmax ymin..ymax if 1 vertex satisfies the equations
    while (1){
      bool ok=true;
      for (int i=0;i<m.size();++i){
	gen a=subst(m[i],vars,makevecteur(xmin,ymin),false,contextptr);
	if (is_zero(a)){ ok=false; break; }
	a=subst(m[i],vars,makevecteur(xmin,ymax),false,contextptr);
	if (is_zero(a)){ ok=false; break; }
	a=subst(m[i],vars,makevecteur(xmax,ymax),false,contextptr);
	if (is_zero(a)){ ok=false; break; }
	a=subst(m[i],vars,makevecteur(xmax,ymin),false,contextptr);
	if (is_zero(a)){ ok=false; break; }
      }
      if (ok) break;
      double z=rand()*0.1/RAND_MAX;
      xmin -= z*std::abs(xmin);
      z=rand()*0.1/RAND_MAX;
      xmax += z*std::abs(xmax);
      z=rand()*0.1/RAND_MAX;
      ymin -= z*std::abs(ymin);
      z=rand()*0.1/RAND_MAX;
      ymax += z*std::abs(ymax);      
    }
    vector<double> yxmin,yxmax,xymin,xymax; // contains intersections of curves with the 4 edges of the domain
    yxmin.push_back(ymin); yxmin.push_back(ymax); // y values at x=xmin
    yxmax.push_back(ymin); yxmax.push_back(ymax);
    xymin.push_back(xmin); xymin.push_back(xmax); // x values at y=ymin
    xymax.push_back(xmin); xymax.push_back(xmax);
    for (size_t i=0;i<N;++i){
      gen Ci=C[i];
      if (is_zero(Ci)) {
	gen key=makevecteur(m[i],vx,vy,xmin,xmax,ymin,ymax);
	iterateur it=cache.begin(),itend=cache.end();
	for (;it!=itend;++it){
	  gen k=(*it)[0][0];
	  k=ratnormal(m[i]/k,contextptr);
	  if (k.type==_FRAC || is_integer(k))
	    break;
	}
	if (it!=itend)
	  Ci=(*it)[1];
	else {
	  int nstep=8*gnuplot_pixels_per_eval;
#ifndef KHICAS
	  if (nstep<8000) nstep=8000;
#endif
	  Ci=plotimplicit(m[i],vx,vy,xmin,xmax,ymin,ymax,nstep,0,epsilon(contextptr),vecteur(1,default_color(contextptr)),false,true,contextptr,3);
	  cache.push_back(makevecteur(key,Ci));
	}
	if (Ci.type==_VECT && Ci._VECTptr->empty()){
	  // always true or always false
	  gen tst=subst(m[i],vars,makevecteur(xmin,ymin),false,contextptr);
	  if (!is_positive(tst,contextptr)){
	    curves=0;
	    return 0; // always false
	  }
	}
	// extract polygonal lines
	vecteur v(gen2vecteur(Ci));
	for (int j=0;j<v.size();++j){
	  v[j]=remove_at_pnt(v[j]);
	  if (v[j].is_symb_of_sommet(at_curve)) // should be true
	    v[j]=v[j][2];
	  // update begin/end of polygonal lines to fit xmin/xmax/ymin/ymax
	  vecteur tmp(gen2vecteur(v[j]));
	  reverse(tmp.begin(),tmp.end());
	  if (cut(m[i],vx,vy,tmp,xmin,xmax,ymin,ymax,xymin,xymax,yxmin,yxmax,contextptr)==-1) return -1;
	  reverse(tmp.begin(),tmp.end());
	  if (cut(m[i],vx,vy,tmp,xmin,xmax,ymin,ymax,xymin,xymax,yxmin,yxmax,contextptr)==-1) return -1;
	  v[j]=tmp;
	}
	Ci=C[i]=v;
      } // end implicit
      else { // find branchs in xmin..xmax,ymin..ymax for hyperbolas and segments
	gen eq,eqx,eqy;
	if (Ci.type==_VECT && Ci._VECTptr->size()==2 && Ci.subtype==_LINE__VECT)
	  eq=Ci._VECTptr->front()+t__IDNT_e*(Ci._VECTptr->front()-Ci._VECTptr->back());
	else {
	  eq=_parameq(Ci,contextptr);
	}
	if (Ci.is_symb_of_sommet(at_cercle)){
	  Ci=_paramplot(makesequence(eq,t__IDNT_e,-M_PI,M_PI,M_PI/100),contextptr);
	  Ci=remove_at_pnt(Ci);
	  Ci=vecteur(1,Ci[2]);
	} else {
	  reim(eq,eqx,eqy,contextptr);
	  vecteur txmin=solve(eqx-xmin,t__IDNT_e,0,contextptr);
	  for (int j=0;j<txmin.size();++j){
	    gen y=evalf_double(subst(eqy,t__IDNT_e,txmin[j],false,contextptr),1,contextptr);
	    if (y.type==_DOUBLE_ && y._DOUBLE_val>ymin && y._DOUBLE_val<ymax)
	      yxmin.push_back(y._DOUBLE_val);
	    else {
	      txmin.erase(txmin.begin()+j); --j;
	    }
	  }
	  vecteur txmax=solve(eqx-xmax,t__IDNT_e,0,contextptr);
	  for (int j=0;j<txmax.size();++j){
	    gen y=evalf_double(subst(eqy,t__IDNT_e,txmax[j],false,contextptr),1,contextptr);
	    if (y.type==_DOUBLE_ && y._DOUBLE_val>ymin && y._DOUBLE_val<ymax)
	      yxmax.push_back(y._DOUBLE_val);
	    else {
	      txmax.erase(txmax.begin()+j); --j;
	    }
	  }
	  vecteur tymin=solve(eqy-ymin,t__IDNT_e,0,contextptr);
	  for (int j=0;j<tymin.size();++j){
	    gen x=evalf_double(subst(eqx,t__IDNT_e,tymin[j],false,contextptr),1,contextptr);
	    if (x.type==_DOUBLE_ && x._DOUBLE_val>xmin && x._DOUBLE_val<xmax)
	      xymin.push_back(x._DOUBLE_val);
	    else {
	      tymin.erase(tymin.begin()+j); --j;
	    }
	  }
	  vecteur tymax=solve(eqy-ymax,t__IDNT_e,0,contextptr);
	  for (int j=0;j<tymax.size();++j){
	    gen x=evalf_double(subst(eqx,t__IDNT_e,tymax[j],false,contextptr),1,contextptr);
	    if (x.type==_DOUBLE_ && x._DOUBLE_val>xmin && x._DOUBLE_val<xmax)
	      xymax.push_back(x._DOUBLE_val);
	    else {
	      tymax.erase(tymax.begin()+j); --j;
	    }
	  }
	  vecteur T(mergevecteur(mergevecteur(txmin,txmax),mergevecteur(tymin,tymax)));
	  T=gen2vecteur(_sort(T,contextptr));
	  vecteur branches;
	  gen Cir=remove_at_pnt(Ci);
	  if (T.empty() && Cir.is_symb_of_sommet(at_curve)){ // ellipse, keep the polygonal line
	    branches=vecteur(1,remove_at_pnt(Ci)[2]); // 1 branch
	  }
	  else {
	    for (int i=1;i < T.size();++i){
	      gen t0(T[i-1]),t1(T[i]),tmid((t0+t1)/2.0);
	      gen x=subst(eqx,t__IDNT_e,tmid,false,contextptr);
	      x=evalf_double(x,1,contextptr);
	      gen y=subst(eqy,t__IDNT_e,tmid,false,contextptr);
	      y=evalf_double(y,1,contextptr);
	      if (x.type==_DOUBLE_ && y.type==_DOUBLE_){
		double xd=x._DOUBLE_val,yd=y._DOUBLE_val;
		if (xd>xmin && xd<xmax && yd>ymin && yd<ymax){
		  // curve branch is inside domain
		  gen a,b;
		  if (is_linear_wrt(eq,t__IDNT_e,a,b,contextptr)){
		    a=subst(eq,t__IDNT_e,t0,false,contextptr);
		    b=subst(eq,t__IDNT_e,t1,false,contextptr);
		    branches.push_back(gen(makevecteur(a,b),_LINE__VECT));
		  }
		  else {
#if 1
		    // eval eq from t0 to t1 with more accuracy near boundaries
		    vecteur br;
		    gen dt=t1-t0;
		    for (int i=0;i<=20;++i){
		      br.push_back(subst(eq,t__IDNT_e,t0+i*dt/1000,false,contextptr));
		    }
		    for (int i=2;i<=48;++i){
		      br.push_back(subst(eq,t__IDNT_e,t0+i*dt/50,false,contextptr));
		    }
		    for (int i=20;i>=0;--i){
		      br.push_back(subst(eq,t__IDNT_e,t1-i*dt/1000,false,contextptr));
		    }
#else
		    gen br=_plotparam(makesequence(eq,t__IDNT_e,t0,t1,(t1-t0)/100),contextptr);
		    br=remove_at_pnt(br)[2];
#endif
		    branches.push_back(br);
		  }
		}
	      }
	    } // end loop
	  } // end hyperbola or line
	  Ci=branches;
	} // end not circle
      } // end if (implicitplot) else conic
      // cut Ci branchs at intersection points, 
      vecteur intp; // list of intersections points for curve i
      aplatir(gen2vecteur(res[i]),intp,false);
      vecteur branches=gen2vecteur(Ci);
      for (int I=0;I<intp.size();++I){
	// insert in the right polygonal line at the right position, cut
	insert(intp[I],branches,contextptr);
      }
      // keep only valid branches
      Ci=branches;
      branches.clear();
      for (int j=0;j<Ci._VECTptr->size();++j){
	vecteur br=gen2vecteur((*Ci._VECTptr)[j]);
	if (br.size()>=2 && is_ok(m,i,vars,br.front(),contextptr) && is_ok(m,i,vars,br.back(),contextptr)){
	  gen chk=br.size()==2 ?(br[0]+br[1])/2:br[br.size()/2];
	  if ( is_ok(m,i,vars,chk,contextptr))
	    branches.push_back(br);
	}
      }
      C[i]=branches;
    }
    vecteur branches;
    aplatir(C,branches,false);
    // sort and add edges of the domain
    vector<double> tmp;
    tmp.clear(); tmp.swap(xymin);
    for (int i=0;i<tmp.size();++i){
      if (is_ok(m,-1,vars,makevecteur(tmp[i],ymin),contextptr))
	xymin.push_back(tmp[i]);
    }
    sort(xymin.begin(),xymin.end());
    for (int i=1;i<xymin.size();++i){
      if (fabs(xymin[i-1]-xymin[i])<1e-8)
	continue;
      gen chk=makevecteur((xymin[i-1]+xymin[i])/2,ymin);
      if (is_ok(m,-1,vars,chk,contextptr))
	branches.push_back(makevecteur(gen(xymin[i-1],ymin),gen(xymin[i],ymin)));
    }
    tmp.clear(); tmp.swap(xymax);
    for (int i=0;i<tmp.size();++i){
      if (is_ok(m,-1,vars,makevecteur(tmp[i],ymax),contextptr))
	xymax.push_back(tmp[i]);
    }
    sort(xymax.begin(),xymax.end());
    for (int i=1;i<xymax.size();++i){
      if (fabs(xymax[i-1]-xymax[i])<1e-8)
	continue;
      if (is_ok(m,-1,vars,makevecteur((xymax[i-1]+xymax[i])/2,ymax),contextptr))
	branches.push_back(makevecteur(gen(xymax[i-1],ymax),gen(xymax[i],ymax)));
    }
    tmp.clear(); tmp.swap(yxmin);
    for (int i=0;i<tmp.size();++i){
      if (is_ok(m,-1,vars,makevecteur(xmin,tmp[i]),contextptr))
	yxmin.push_back(tmp[i]);
    }
    sort(yxmin.begin(),yxmin.end());
    for (int i=1;i<yxmin.size();++i){
      if (fabs(yxmin[i-1]-yxmin[i])<1e-8)
	continue;
      if (is_ok(m,-1,vars,makevecteur(xmin,(yxmin[i-1]+yxmin[i])/2),contextptr))
	branches.push_back(makevecteur(gen(xmin,yxmin[i-1]),gen(xmin,yxmin[i])));
    }
    tmp.clear(); tmp.swap(yxmax);
    for (int i=0;i<tmp.size();++i){
      if (is_ok(m,-1,vars,makevecteur(xmax,tmp[i]),contextptr))
	yxmax.push_back(tmp[i]);
    }
    sort(yxmax.begin(),yxmax.end());
    for (int i=1;i<yxmax.size();++i){
      if (fabs(yxmax[i-1]-yxmax[i])<1e-8)
	continue;
      if (is_ok(m,-1,vars,makevecteur(xmax,(yxmax[i-1]+yxmax[i])/2),contextptr))
	branches.push_back(makevecteur(gen(xmax,yxmax[i-1]),gen(xmax,yxmax[i])));
    }
    curves=branches;
    return 1;
  }

  // RAND_MAX invalid data, -RAND_MAX if p is on v, index otherwise
  // index is 0 if outside
  int is_inside(const gen & p,const vecteur &v,GIAC_CONTEXT){
    if (v.size()<4 || v.front()!=v.back())
      return RAND_MAX;
    gen theta=0.0;
    for (int i=1;i<v.size();++i){
      gen a=v[0],b=v[1],c=a-p;
      if (is_zero(c))
	return -RAND_MAX;
      theta += arg((b-p)/c,contextptr);
    }
    if (theta.type!=_DOUBLE_)
      return RAND_MAX;
    double t=theta._DOUBLE_val; // t should be about a multiple of 2*pi
    return int(t/2/M_PI+.25);
  }

  // v and w assumed not to cross
  // 1 if v contains w, -1 if w contains v, 0 otherwise
  int curve_compare(const vecteur &v,const vecteur & w,GIAC_CONTEXT){
    if (v.size()<4 || w.size()<4)
      return RAND_MAX;
    gen p=v[0],q=w[0];
    int i=is_inside(q,v,contextptr),j=is_inside(p,w,contextptr);
    if (absint(i)==RAND_MAX || absint(j)==RAND_MAX)
      return RAND_MAX;
    if (i!=0 && j==0) // w[0] is inside v and v[0] is outside w
      return 1;
    if (i==0 && j!=0)
      return -1;
    return 0;
  }

  gen nearest(const vecteur &v,const vecteur & w,int & pos1,int & pos2,GIAC_CONTEXT){
    gen mind(distance2pp(v[0],w[0],contextptr));
    pos1=pos2=0;
    for (int i=0;i<v.size();++i){
      for (int j=0;j<w.size();++j){
	gen curd(distance2pp(v[i],w[j],contextptr));
	if (is_greater(mind,curd,contextptr)){
	  pos1=i; pos2=j; mind=curd;
	}
      }
    }
    return mind;
  }

  vecteur connect_nearest(const vecteur& C1,const vecteur &C2,GIAC_CONTEXT){
    int pos1,pos2;
    nearest(C1,C2,pos1,pos2,contextptr);
    // make polygon
    vecteur connect(C1.begin(),C1.begin()+pos1+1);
    for (int k=pos2;k<C2.size();++k)
      connect.push_back(C2[k]);
    for (int k=1;k<=pos2;++k)
      connect.push_back(C2[k]);
    for (int k=pos1;k<C1.size();++k)
      connect.push_back(C1[k]);
    return connect;
  }

  bool curves2polygons(const vecteur & C,vecteur & res,GIAC_CONTEXT){
    if (C.empty()) return true;
    // find curves that are inside other curves
    vector< vector<bool> > liens(C.size(),vector<bool>(C.size(),false));
    for (int j=0;j<C.size();++j){
      for (int k=j+1;k<C.size();++k){
	int l=curve_compare(gen2vecteur(C[j]),gen2vecteur(C[k]),contextptr);
	if (l==RAND_MAX) return false;
	if (l==0) continue;
	if (l>0) liens[j][k]=true;
	if (l<0) liens[k][j]=true;
      }
    }
    // count number of curves inside each curve
    vector<int> nliens(C.size());
    for (int j=0;j<C.size();++j){
      int count=0;
      for (int k=0;k<C.size();++k){
	if (liens[j][k]) ++count;
      }
      nliens[j]=count;
    }
    vector<bool> done(C.size());
    for (;;){
      // find the curve not done that has the max number of curves inside
      int maxin=-1,J=-1;
      for (int j=0;j<C.size();++j){
	if (!done[j] && nliens[j]>maxin){
	  J=j;
	  maxin=nliens[j];
	}
      }
      if (maxin==-1) return true; // all curves were done
      done[J]=true;
      // curve[J] is an outside curve
      // find direct childs, curves that are inside J but not inside other curves
      vector<int> childrens;
      if (maxin){
	for (int j=0;j<C.size();++j){
	  if (j==J) continue;
	  int k=0;
	  for (;k<C.size();++k){
	    if (k!=J && liens[k][j])
	      break;
	  }
	  if (k==C.size()){
	    // j is a direct child
	    childrens.push_back(j);
	    done[j]=true;
	    // recurse on C[j]'s children
	    vecteur in;
	    for (int l=0;l<C.size();++l){
	      if (liens[j][l])
		in.push_back(C[l]);
	    }
	    curves2polygons(in,res,contextptr);
	  }
	}
      }
      // fill polygon between J and childrens
      if (childrens.empty()){
	res.push_back(C[J]);
	continue;
      }
      // connect childrens
      vecteur C2(gen2vecteur(C[childrens[0]]));
      for (int j=1;j<childrens.size();++j)
	C2=connect_nearest(C2,gen2vecteur(C[childrens[j]]),contextptr);
      // connect nearest point of C[J] with C[childrens[0]]
      vecteur C1(gen2vecteur(C[J]));
      res.push_back(connect_nearest(C1,C2,contextptr));
    }
    return true;
  }

  // vsymb should be a union of intersections, as returned by andor2list
  bool poly_ineq_plot(const vecteur & vsymb,const gen & x,const gen &y,double & xmin,double & xmax,double & ymin,double & ymax,const vecteur & attr_,vecteur & res,GIAC_CONTEXT){
    vecteur attr(attr_); fill_attributs(attr);
    vecteur v(vsymb.size());
    for (size_t i=0;i<v.size();++i){
      matrice vi;
      if (!and2listdiff(gen2vecteur(vsymb[i]),x,y,vi,contextptr))
	return false;
      v[i]=vi;
    }
    // v[i] is a list of inequations eq>=0 stored as eq
    matrice inter(vsymb.size()),curves(vsymb.size()); // one list of matrices of intersection points for each element of v (and one list of curves)
    vecteur cache; // cache for implicitplot
    for (size_t i=0;i<v.size();++i){
      vecteur vi=gen2vecteur(v[i]);
      vecteur ii;
      int I=poly_ineq_inter(vi,ii,curves[i],x,y,xmin,xmax,ymin,ymax,cache,contextptr);
      if (I<0)
	return false;
      if (I==0)
	continue; // empty set
      // curves[i] is a list of branches
      vecteur B=gen2vecteur(curves[i]),C;
      // now we must connect them, this is an O(N^2) process
      while (!B.empty()){
	vecteur branch(gen2vecteur(B.back()));
	if (branch.size()<2)
	  return false;
	B.pop_back();	
	gen src=branch[0],dst=branch.back();
	// find a branch starting or ending at dst
	for (int j=0;j<B.size();++j){
	  vecteur cur=gen2vecteur(B[j]);
	  if (cur.size()<2)
	    return false;
	  if (is_greater(1e-8,abs(cur.back()-dst,contextptr),contextptr)){
	    B.erase(B.begin()+j);
	    for (int k=cur.size()-2;k>=0;--k){
	      branch.push_back(cur[k]);
	    }
	    dst=cur[0];
	    if (is_greater(1e-8,abs(dst-src,contextptr),contextptr)){
	      branch.back()=branch.front();
	      break;
	    }
	    j=-1;
	    continue;
	  }
	  if (!is_greater(1e-8,abs(cur[0]-dst,contextptr),contextptr))
	    continue;
	  B.erase(B.begin()+j);
	  for (int k=1;k<cur.size();++k){
	    branch.push_back(cur[k]);
	  }
	  dst=cur.back();
	  if (is_greater(1e-8,abs(dst-src,contextptr),contextptr)){
	    branch.back()=branch.front();
	    break;
	  }
	  j=-1;
	}
	if (!is_greater(1e-8,abs(dst-src,contextptr),contextptr))
	  return false; // could not loop
	C.push_back(branch);
      }
      curves2polygons(C,res,contextptr);
    } // end i loop
    for (int j=0;j<res.size();++j){
      res[j]=pnt_attrib(change_subtype(res[j],_GROUP__VECT),attr,contextptr);
    }
    add_autoscale(res,xmin,xmax,ymin,ymax);
    return true;
  }

  gen expand(const gen & e,GIAC_CONTEXT){
    if (is_equal(e))
      return apply_to_equal(e,expand,contextptr);
    if (e.type==_SYMB && (e._SYMBptr->sommet==at_and || e._SYMBptr->sommet==at_ou)){
#if 0 // for testing plot
      return lin_ineq_plot(andor2list(e,contextptr),x__IDNT_e,y__IDNT_e,vecteur(1,56),contextptr); 
#else
      return list2orand(andor2list(e,contextptr));
#endif
    }
    gen var,res;
    if (e.type!=_VECT && is_algebraic_program(e,var,res))
      return symbolic(at_program,makesequence(var,0,expand(res,contextptr)));
    if (e.type==_VECT && e.subtype==_SEQ__VECT && e._VECTptr->size()==2){
      gen last=e._VECTptr->back();
      if (last.type==_STRNG || last.type==_FUNC){
	vector<const unary_function_ptr *> v;
	vector< gen_op_context > w;
	if (contains(last,gen(at_prod)) || (last.type==_STRNG && !strcmp(last._STRNGptr->c_str(),"*"))){ // expand * with no further simplification
	  v.push_back(at_prod);
	  w.push_back(prod_expand_nosimp);
	}
	if (contains(last,gen(at_ln))){
	  v.push_back(at_ln);
	  w.push_back(&ln_expand);
	}
	if (contains(last,gen(at_exp))){
	  v.push_back(at_exp);
	  w.push_back(&exp_expand);
	}
	if (contains(last,gen(at_sin))){ 
	  v.push_back(at_sin);
	  w.push_back(&sin_expand);
	}
	if (contains(last,gen(at_cos))){
	  v.push_back(at_cos);
	  w.push_back(&cos_expand);
	}
	if (contains(last,gen(at_tan))){
	  v.push_back(at_tan);
	  w.push_back(&tan_expand);
	}
	return subst(e._VECTptr->front(),v,w,false,contextptr);	
      }
    }
    vector<const unary_function_ptr *> v;
    vector< gen_op_context > w;
    v.push_back(at_prod);
    v.push_back(at_pow);
    v.push_back(at_neg);
    w.push_back(&prod_expand);
    w.push_back(&expand_pow_expand);
    w.push_back(&expand_neg_expand);
    return _simplifier(subst(e,v,w,false,contextptr),contextptr);
  }
  static const char _expand_s []="expand";
  symbolic symb_expand(const gen & args){
    return symbolic(at_expand,args);
  }
  static define_unary_function_eval (__expand,&expand,_expand_s);
  define_unary_function_ptr( at_expand ,alias_at_expand ,&__expand);

  gen expexpand(const gen & e,GIAC_CONTEXT){
    if (is_equal(e))
      return apply_to_equal(e,expexpand,contextptr);
    gen var,res;
    if (is_algebraic_program(e,var,res))
      return symbolic(at_program,makesequence(var,0,expexpand(res,contextptr)));
    vector<const unary_function_ptr *> v(1,at_exp);
    vector< gen_op_context > w(1,&exp_expand);
    return subst(e,v,w,false,contextptr);
  }
  static const char _expexpand_s []="expexpand";
  static define_unary_function_eval (__expexpand,&expexpand,_expexpand_s);
  define_unary_function_ptr5( at_expexpand ,alias_at_expexpand,&__expexpand,0,true);

  gen lnexpand(const gen & e,GIAC_CONTEXT){
    if (is_equal(e))
      return apply_to_equal(e,lnexpand,contextptr);
    gen var,res;
    if (is_algebraic_program(e,var,res))
      return symbolic(at_program,makesequence(var,0,lnexpand(res,contextptr)));
    vector<const unary_function_ptr *> v(1,at_ln);
    vector< gen_op_context > w(1,&ln_expand);
    return subst(e,v,w,false,contextptr);
  }
  static const char _lnexpand_s []="lnexpand";
  static define_unary_function_eval (__lnexpand,&lnexpand,_lnexpand_s);
  define_unary_function_ptr5( at_lnexpand ,alias_at_lnexpand,&__lnexpand,0,true);

  gen trigexpand(const gen & e,GIAC_CONTEXT){
    if (is_equal(e))
      return apply_to_equal(e,trigexpand,contextptr);
    gen var,res;
    if (is_algebraic_program(e,var,res))
      return symbolic(at_program,makesequence(var,0,trigexpand(res,contextptr)));
    vector<const unary_function_ptr *> v;
    vector< gen_op_context > w;
    v.push_back(at_sin);
    w.push_back(&sin_expand);
    v.push_back(at_cos);
    w.push_back(&cos_expand);
    v.push_back(at_tan);
    w.push_back(&tan_expand);
    v.push_back(at_prod);
    w.push_back(&prod_expand);    
    return subst(e,v,w,false,contextptr);
  }
  static const char _trigexpand_s []="trigexpand";
  static define_unary_function_eval (__trigexpand,&trigexpand,_trigexpand_s);
  define_unary_function_ptr5( at_trigexpand ,alias_at_trigexpand,&__trigexpand,0,true);

#ifndef NO_NAMESPACE_GIAC
} // namespace giac
#endif // ndef NO_NAMESPACE_GIAC
