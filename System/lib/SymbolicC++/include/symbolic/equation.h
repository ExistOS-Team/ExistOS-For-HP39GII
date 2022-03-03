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


// equation.h

#ifndef SYMBOLIC_CPLUSPLUS_EQUATION

#include <list>

using namespace std;

#ifdef  SYMBOLIC_FORWARD
#ifndef SYMBOLIC_CPLUSPLUS_EQUATION_FORWARD
#define SYMBOLIC_CPLUSPLUS_EQUATION_FORWARD

class Equation;

#endif
#endif

#ifdef  SYMBOLIC_DECLARE
#define SYMBOLIC_CPLUSPLUS_EQUATION
#ifndef SYMBOLIC_CPLUSPLUS_EQUATION_DECLARE
#define SYMBOLIC_CPLUSPLUS_EQUATION_DECLARE

class Equation: public CloningSymbolicInterface
{
 public: Symbolic lhs, rhs;
	 list<Symbolic> free;
         Equation(const Equation&);
         Equation(const Equation&, const Symbolic &);
         Equation(const Symbolic&,const Symbolic&);
         ~Equation();

         void print(ostream&) const;
         Symbolic subst(const Symbolic&,const Symbolic&,int &n) const;
         Simplified simplify() const;
         int compare(const Symbolic&) const;
         Symbolic df(const Symbolic&) const;
         Symbolic integrate(const Symbolic&) const;
         Symbolic coeff(const Symbolic&) const;
         Expanded expand() const;
         int commute(const Symbolic&) const;
         PatternMatches match(const Symbolic&, const list<Symbolic>&) const;
         PatternMatches match_parts(const Symbolic&,
                                    const list<Symbolic>&) const;

         operator bool() const;
         operator int() const;

         Cloning *clone() const { return Cloning::clone(*this); }
};

#endif
#endif


#endif
