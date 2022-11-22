// -*- mode:C++ ; compile-command: "g++-3.4 -I.. -g -c risch.cc -DHAVE_CONFIG_H -DIN_GIAC" -*-
#include "giacPCH.h"
/*
 *  Copyright (C) 2003,14 B. Parisse, Institut Fourier, 38402 St Martin d'Heres
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
#include "vector.h"
#include <cmath>
#include <cstdlib>
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
#include "misc.h"
#include "giacintl.h"

#ifndef NO_NAMESPACE_GIAC
namespace giac {
#endif // ndef NO_NAMESPACE_GIAC

  gen risch(const gen & e_orig,const identificateur & x,gen & remains_to_integrate,GIAC_CONTEXT){
    remains_to_integrate=e_orig;
    return 0;
  }

  // integer roots of a polynomial
  vecteur iroots(const polynome & p){
    int s=p.dim;
    vecteur zerozero(s-1);
    vecteur P0(polynome2poly1(p/lgcd(p),1));
    vecteur P(P0);
    // eval every coeff at (0,...,0)
    int d=int(P.size());
    for (int i=0;i<d;++i){
      if (P[i].type==_POLY)
	P[i]=peval(*P[i]._POLYptr,zerozero,0);
    }
    // now search the integer roots of this polynomial
    polynome p0(poly12polynome(P,1,1));
    polynome p1=p0.derivative();
    p1=gcd(p0,p1);
    p0=p0/p1; // p0 is now squarefree with the same roots as initial p0
    // check that all coeffs are integer, if not call normal factorizatio
    vector< monomial<gen> >::const_iterator it=p0.coord.begin(),itend=p0.coord.end();
    vecteur res;
    for (;it!=itend;++it){
#ifndef HAVE_LIBNTL // with LIBNTL, linearfind does nothing!
      if (!is_integer(it->value)){
#endif
	factorization vden;
	gen extra_div=1;
	factor(p0,p1,vden,false,false,false,1,extra_div);
	factorization::const_iterator f_it=vden.begin(),f_itend=vden.end();
	// bool ok=true;
	for (;f_it!=f_itend;++f_it){
	  int deg=f_it->fact.lexsorted_degree();
	  if (deg!=1)
	    continue;
	  // extract the root
	  vecteur vtmp=polynome2poly1(f_it->fact,1);
	  gen root=-vtmp.back()/vtmp.front();
	  if (root.type==_INT_)
	    res.push_back(root);
	}
	return res;
#ifndef HAVE_LIBNTL
      }
#endif
    }
    environment * env=new environment;
    polynome temp(1);
    vectpoly v;
    int ithprime=1;
    if (!linearfind(p0,env,temp,v,ithprime)) // FIXME??
      res.clear();// int bound=
    delete env;
    d=int(v.size());
    for (int i=0;i<d;++i){
      vecteur tmpv=polynome2poly1(v[i]);
      if (tmpv.size()!=2)
	continue;
      gen g=-tmpv[1]/tmpv[0];
      if (g.type!=_INT_)
	continue;
      gen tmp=horner(P0,g);
      if (is_zero(tmp))
	res.push_back(g);
    }
    return res;
  }
#ifndef NO_NAMESPACE_GIAC
} // namespace giac
#endif // ndef NO_NAMESPACE_GIAC

