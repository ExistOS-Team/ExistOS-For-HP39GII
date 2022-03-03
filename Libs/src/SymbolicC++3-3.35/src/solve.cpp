/*
    SymbolicC++ : An object oriented computer algebra system written in C++

    Copyright (C) 2008 Yorick Hardy and Willi-Hans Steeb

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "symbolic/symbolicc++.h"


#ifdef  SYMBOLIC_DEFINE
#ifndef SYMBOLIC_CPLUSPLUS_SOLVE_DEFINE
#define SYMBOLIC_CPLUSPLUS_SOLVE_DEFINE

Equations solve(const Symbolic &e, const Symbolic &x)
{
 Equations soln;
 if(e == 0)
  soln = (soln, x == x);
 else if(df(e,x,2) == 0)
  soln = (soln, x == -e.coeff(x,0)/e.coeff(x,1));
 else if(df(e,x,3) == 0)
 {
  Symbolic a = e.coeff(x,2), b = e.coeff(x,1), c = e.coeff(x,0);
  Symbolic d = b*b-4*a*c;

  list<Equations> eq;
  list<Equations>::iterator i;
  UniqueSymbol u, v, w;
  eq = ((u^2) + u*v + w).match(d, (u,v,w));
  for(i=eq.begin(); i!=eq.end(); ++i)
   try {
    Symbolic up = rhs(*i, u), vp = rhs(*i, v), wp = rhs(*i, w);
    if((vp^2)/4 == wp)
    {
     soln = (soln, x == (-b+up+vp/2)/(2*a), x == (-b-up-vp/2)/(2*a));
     break;
    }
   } catch(const SymbolicError &se) {}

  if(i == eq.end())
   soln = (soln, x == (-b+sqrt(d))/(2*a), x == (-b-sqrt(d))/(2*a));
 }
 else if(df(e,x,4) == 0)
 {
  Symbolic l = e/e.coeff(x,3);
  Symbolic a = l.coeff(x,2), b = l.coeff(x,1), c = l.coeff(x,0);
  Symbolic Q = (3*b-a*a)/9, R = 9*a*b-27*c-2*a*a*a;
  Symbolic S = (R+sqrt((Q^3)+(R^2)))^(Symbolic(1)/3);
  Symbolic T = (R-sqrt((Q^3)+(R^2)))^(Symbolic(1)/3);
  soln = (soln, x == S+T+a/3,
                x == -(S+T)/2-a/3+SymbolicConstant::i*sqrt(Symbolic(3))*(S-T),
                x == -(S+T)/2-a/3-SymbolicConstant::i*sqrt(Symbolic(3))*(S-T));
 }
 else if(e.coeff(x,-1) != 0)
 {
  Equations::iterator i;
  soln = solve(x*e, x);
  for(i = soln.begin(); i != soln.end();)
   if(i->lhs == x && i->rhs == 0) i = soln.erase(i);
   else ++i;
 }
 return soln;
}

Equations solve(const Equation &e, const Symbolic &x)
{ return solve(e.lhs - e.rhs, x); }

list<Equations> solve(const Equations &e, const list<Symbolic> &l)
{
 int sc = 0, free = 1;
 list<Equations> soln;
 Equations::const_iterator i, j, k;
 list<Equations>::const_iterator u;
 list<Symbolic>::const_iterator li;

 if(e.empty())
 {
  Equations eq;
  for(li=l.begin(); li!=l.end(); ++li) eq = (eq, *li == *li);
  soln.push_back(eq);
 }
 if(l.empty()) return soln; // no variables to solve for
 for(i=e.begin(); i!=e.end(); ++i)
 {
  Symbolic eqi = i->lhs-i->rhs;
  if(df(eqi,l.front()) != 0)
  {
   Equations s = solve(eqi, l.front());
   free = 0;
   for(j=s.begin(); j!=s.end(); ++j)
   {
    Equations eq;
    for(k=e.begin(); k!=e.end(); ++k)
     if(k!=i) eq.push_back(k->lhs->subst(j->lhs,j->rhs, sc) ==
                           k->rhs->subst(j->lhs,j->rhs, sc));
    list<Equations> slns = solve(eq, list<Symbolic>(++l.begin(), l.end()));
    for(u=slns.begin(); u!=slns.end(); ++u)
     pattern_match_OR(soln, (j->lhs == j->rhs[*u], *u));
   }
  }
 }

 if(free)
 {
  if(l.size() == 1)
  {
   Equations s;
   s = (s, l.front() == l.front());
   soln.push_back(s);
  }
  else
  {
   list<Equations> slns = solve(e, list<Symbolic>(++l.begin(), l.end()));
   for(u=slns.begin(); u!=slns.end(); ++u)
    soln.push_back((l.front() == l.front(), *u));
  }
 }

 return soln;
}

#endif
#endif

