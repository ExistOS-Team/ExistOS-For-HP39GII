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


// symbol.h

#ifndef SYMBOLIC_CPLUSPLUS_SYMBOL

#include <list>
#include <string>
using namespace std;

#ifdef  SYMBOLIC_FORWARD
#ifndef SYMBOLIC_CPLUSPLUS_SYMBOL_FORWARD
#define SYMBOLIC_CPLUSPLUS_SYMBOL_FORWARD

class Symbol;
class UniqueSymbol;

#endif
#endif

#ifdef  SYMBOLIC_DECLARE
#define SYMBOLIC_CPLUSPLUS_SYMBOL
#ifndef SYMBOLIC_CPLUSPLUS_SYMBOL_DECLARE
#define SYMBOLIC_CPLUSPLUS_SYMBOL_DECLARE

class Symbol: public CloningSymbolicInterface
{
 public: string name;
         list<Symbolic> parameters;
         int commutes;
         Symbol(const Symbol&);
         Symbol(const string&,int = 1);
         Symbol(const char*,int = 1);
         ~Symbol();

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

         Symbol operator[](const Symbolic&) const;
	 Symbol operator[](const list<Symbolic> &l) const;
         Symbol commutative(int=0) const;
         Symbol operator~() const;

         Cloning *clone() const { return Cloning::clone(*this); }
};

class UniqueSymbol: public Symbol
{
 private: int *p;
 public:
         UniqueSymbol();
         UniqueSymbol(const UniqueSymbol&);
         UniqueSymbol(const Symbol&);
         ~UniqueSymbol();

         void print(ostream&) const;
         int compare(const Symbolic&) const;

         Cloning *clone() const { return Cloning::clone(*this); }
};

#endif
#endif


#endif
