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
#ifndef SYMBOLIC_CPLUSPLUS_EQUATION_DEFINE
#define SYMBOLIC_CPLUSPLUS_EQUATION_DEFINE
#define SYMBOLIC_CPLUSPLUS_EQUATION

Equation::Equation(const Equation &s)
: CloningSymbolicInterface(s), lhs(s.lhs), rhs(s.rhs), free(s.free) {}

Equation::Equation(const Equation &s, const Symbolic &newfree)
: CloningSymbolicInterface(s), lhs(s.lhs), rhs(s.rhs), free(s.free)
{ free.push_back(newfree); }

Equation::Equation(const Symbolic &s1,const Symbolic &s2)
: lhs(s1), rhs(s2) {}

Equation::~Equation() {}

void Equation::print(ostream &o) const
{ o << lhs << " == " << rhs; }

Symbolic Equation::subst(const Symbolic &x,const Symbolic &y,int &n) const
{ return Equation(lhs.subst(x,y,n),rhs.subst(x,y,n)); }

Simplified Equation::simplify() const
{ return Equation(lhs.simplify(),rhs.simplify()); }

int Equation::compare(const Symbolic &s) const
{
 if(s.type() != type()) return 0;

 CastPtr<const Equation> e = s;
 return (lhs.compare(e->lhs) && rhs.compare(e->rhs)) ||
        (lhs.compare(e->rhs) && rhs.compare(e->lhs));
}

Symbolic Equation::df(const Symbolic &s) const
{ return Equation(lhs.df(s),rhs.df(s)); }

Symbolic Equation::integrate(const Symbolic &s) const
{ return Equation(::integrate(lhs,s),::integrate(rhs,s)); }

Symbolic Equation::coeff(const Symbolic &s) const
{ return 0; }

Expanded Equation::expand() const
{ return Equation(lhs.expand(),rhs.expand()); }

int Equation::commute(const Symbolic &s) const
{ return 0; }

PatternMatches
Equation::match(const Symbolic &s, const list<Symbolic> &p) const
{
 PatternMatches l;

 if(s.type() == type())
 {
  PatternMatches llhs = lhs.match(s, p);
  PatternMatches lrhs = rhs.match(s, p);
  pattern_match_OR(l, llhs);
  pattern_match_AND(l, lrhs);
 }

 return l;
}

PatternMatches
Equation::match_parts(const Symbolic &s, const list<Symbolic> &p) const
{ return s.match(*this, p); }

Equation::operator bool() const
{ return lhs.compare(rhs); }

Equation::operator int() const
{ return lhs.compare(rhs); }

#endif
#endif

