/* -*- mode:C++ ; compile-command: "g++-3.4 -I. -I.. -I../include -g -c ezgcd.cc -DHAVE_CONFIG_H -DIN_GIAC" -*- */

#include "giacPCH.h"
/*  Multivariate GCD for large data not covered by the heuristic GCD algo
 *  Copyright (C) 2000,7 B. Parisse, Institut Fourier, 38402 St Martin d'Heres
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
#include "threaded.h"
#include "ezgcd.h"
#include "sym2poly.h"
#include "gausspol.h"
#include "modpoly.h"
#include "monomial.h"
#include "derive.h"
#include "subst.h"
#include "solve.h"
#include "giacintl.h"

#ifndef NO_NAMESPACE_GIAC
namespace giac {
#endif // ndef NO_NAMESPACE_GIAC

  static bool convert_from_truncate(const vector< T_unsigned<int,hashgcd_U> > & p,hashgcd_U var,polynome & P){
    P.dim=1;
    P.coord.clear();
    vector< T_unsigned<int,hashgcd_U> >::const_iterator it=p.begin(),itend=p.end();
    P.coord.reserve(itend-it);
    index_t i(1);
    for (;it!=itend;++it){
      i.front()=it->u/var;
      P.coord.push_back(monomial<gen>(gen(it->g),i));
    }
    return true;
  }

  // return true if a good eval point has been found
  bool find_good_eval(const polynome & F,const polynome & G,polynome & Fb,polynome & Gb,vecteur & b,bool debuglog,const gen & mod){
    int Fdeg=int(F.lexsorted_degree()),Gdeg=int(G.lexsorted_degree()),nvars=int(b.size());
    gen Fg,Gg;
    int essai=0;
    int dim=F.dim;
    if ( //false &&
	mod.type==_INT_ && mod.val){
      int modulo=mod.val;
      std::vector<hashgcd_U> vars(dim);
      vector< T_unsigned<int,hashgcd_U> > f,g,fb,gb;
      index_t d(dim);
      if (convert(F,G,d,vars,f,g,modulo)){
	vector<int> bi(dim-1);
	vecteur2vector_int(b,modulo,bi);
	for (;;++essai){
	  if (modulo && essai>modulo)
	    return false;
	  peval_x2_xn<int,hashgcd_U>(f,bi,vars,fb,modulo);
	  if (&F==&G)
	    gb=fb;
	  else
	    peval_x2_xn(g,bi,vars,gb,modulo);	  
	  if (!fb.empty() && !gb.empty() && int(fb.front().u/vars.front())==Fdeg && int(gb.front().u/vars.front())==Gdeg){
	    // convert back fb and gb and return true
	    convert_from_truncate(fb,vars.front(),Fb);
	    convert_from_truncate(gb,vars.front(),Gb);
	    return true;
	  }
	  for (int i=0;i<dim-1;++i)
	    bi[i]=std_rand() % modulo;
	}
      }
    }
    for (;;++essai){
      if (!is_zero(mod) && essai>mod.val)
	return false;
      if (debuglog)
	CERR << "Find_good_eval " << CLOCK() << " " << b << endl;
      Fb=peval_1(F,b,mod);
      if (debuglog)
	CERR << "Fb= " << CLOCK() << " " << gen(Fb) << endl;
      if (&F==&G)
	Gb=Fb;
      else {
	Gb=peval_1(G,b,mod);
      }
      if (debuglog)
	CERR << "Gb= " << CLOCK() << " " << gen(Gb) << endl;
      if ( (Fb.lexsorted_degree()==Fdeg) && (Gb.lexsorted_degree()==Gdeg) ){
	if (debuglog)
	  CERR << "FOUND good eval" << CLOCK() << " " << b << endl;
	return true;
      }
      b=vranm(nvars,0,0); // find another random point
    }
  }
  static void add_dim(monomial<gen> & m,int d){
    index_t i(m.index.iref());
    for (int j=0;j<d;++j)
      i.push_back(0);
    m.index=i;
  }
  void change_dim(polynome & p,int dim){
    vector< monomial<gen> >::iterator it=p.coord.begin(),itend=p.coord.end();
    if (p.dim>=dim){
      p.dim=dim;
      for (;it!=itend;++it){
	it->index=index_t(it->index.begin(),it->index.begin()+dim);
      }
      return;
    }
    int delta_dim=dim-p.dim;
    p.dim=dim;
    for (;it!=itend;++it)
      add_dim(*it,delta_dim);
  }

  polynome peval_1(const polynome & p,const vecteur &v,const gen & mod){
#if defined(NO_STDEXCEPT) && !defined(RTOS_THREADX) && !defined(VISUALC)
    assert(p.dim==signed(v.size()+1));
#else
    if (p.dim!=signed(v.size()+1))
      setsizeerr(gettext("peval_1"));
#endif
    polynome res(1);
    index_t i(1);
    std::vector< monomial<gen> >::const_iterator it=p.coord.begin();
    std::vector< monomial<gen> >::const_iterator itend=p.coord.end();
    for (;it!=itend;){
      i[0]=it->index.front();
      polynome pactuel(Tnextcoeff<gen>(it,itend));
      gen g(peval(pactuel,v,mod));
      if ( (g.type==_POLY) && (g._POLYptr->dim==0) )
	g=g._POLYptr->coord.empty()?0:g._POLYptr->coord.front().value;
      if (!is_zero(g))
	res.coord.push_back(monomial<gen>(g,i));
    }
    return res;
  }

  bool ezgcd(const polynome & F_orig,const polynome & G_orig,polynome & GCD,bool is_sqff,bool is_primitive,int max_gcddeg,double maxop){
    return false;
  }

  bool try_hensel_lift_factor(const polynome & pcur,const polynome & F0,const factorization & v0,int mult,factorization & f){
    return false;
  }

  bool try_sparse_factor(const polynome & pcur,const factorization & v,int mult,factorization & f){
    return false;
  }

  bool try_hensel_egcd(const polynome & p,const polynome & q,polynome &u,polynome &v,polynome & d){
    return false;
  }

  bool try_sparse_factor_bi(polynome & pcur,int mult,factorization & f){
    return false;
  }
  
#ifndef NO_NAMESPACE_GIAC
} // namespace giac
#endif // ndef NO_NAMESPACE_GIAC
