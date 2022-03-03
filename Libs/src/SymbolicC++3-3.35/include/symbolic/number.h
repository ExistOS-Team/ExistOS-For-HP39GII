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


// number.h

#ifndef SYMBOLIC_CPLUSPLUS_NUMBER

#include <cmath>
#include <iostream>
#include <limits>
#include <typeinfo>
#include <utility>
#include "rational.h"
#include "verylong.h"
using namespace std;

#ifdef  SYMBOLIC_FORWARD
#ifndef SYMBOLIC_CPLUSPLUS_NUMBER_FORWARD
#define SYMBOLIC_CPLUSPLUS_NUMBER_FORWARD

class Numeric;
template <class T> class Number;

#endif
#endif

#ifdef  SYMBOLIC_DECLARE
#define SYMBOLIC_CPLUSPLUS_NUMBER
#ifndef SYMBOLIC_CPLUSPLUS_NUMBER_DECLARE
#define SYMBOLIC_CPLUSPLUS_NUMBER_DECLARE

class Numeric: public CloningSymbolicInterface
{
 public: Numeric();
         Numeric(const Numeric&);
         virtual const type_info &numerictype() const = 0;
         virtual Number<void> add(const Numeric&) const = 0;
         virtual Number<void> mul(const Numeric&) const = 0;
         virtual Number<void> div(const Numeric&) const = 0;
         virtual Number<void> mod(const Numeric&) const = 0;
         virtual int isZero() const = 0;
         virtual int isOne() const = 0;
         virtual int isNegative() const = 0;
         virtual int cmp(const Numeric &) const = 0;
         static pair<Number<void>,Number<void> >
             match(const Numeric&,const Numeric&);
         Symbolic subst(const Symbolic&,const Symbolic&,int &n) const;
         int compare(const Symbolic&) const;
         Symbolic df(const Symbolic&) const;
         Symbolic integrate(const Symbolic&) const;
         Symbolic coeff(const Symbolic&) const;
         Expanded expand() const;
         int commute(const Symbolic&) const;
         PatternMatches match(const Symbolic&, const list<Symbolic>&) const;
         PatternMatches match_parts(const Symbolic&,
                                    const list<Symbolic>&) const;
};

template <class T>
class Number: public Numeric
{
 public: T n;
         Number();
         Number(const Number&);
         Number(const T&);
         ~Number();

         Number &operator=(const Number&);
         Number &operator=(const T&);

         void print(ostream&) const;
         const type_info &type() const;
         const type_info &numerictype() const;
         Simplified simplify() const;

         Number<void> add(const Numeric&) const; 
         Number<void> mul(const Numeric&) const; 
         Number<void> div(const Numeric&) const; 
         Number<void> mod(const Numeric&) const; 
         int isZero() const;
         int isOne() const;
         int isNegative() const;
         int cmp(const Numeric &) const;

         Cloning *clone() const { return Cloning::clone(*this); }
};

template<>
class Number<void>: public CastPtr<Numeric>
{
 public: Number();
         Number(const Number&);
         Number(const Numeric&);
         Number(const Symbolic&);
         ~Number();

         const type_info &numerictype() const;
         int isZero() const;
         int isOne() const;
         int isNegative() const;
         static pair<Number<void>,Number<void> >
             match(const Numeric&,const Numeric&);
         static pair<Number<void>,Number<void> >
             match(const Number<void>&,const Number<void>&);

         Number<void> operator+(const Numeric&) const;
         Number<void> operator-(const Numeric&) const;
         Number<void> operator*(const Numeric&) const;
         Number<void> operator/(const Numeric&) const;
         Number<void> operator%(const Numeric&) const;
         Number<void> &operator+=(const Numeric&);
         Number<void> &operator*=(const Numeric&);
         Number<void> &operator/=(const Numeric&);
         Number<void> &operator%=(const Numeric&);
         int operator==(const Numeric&) const;
         int operator<(const Numeric&) const;
         int operator>(const Numeric&) const;
         int operator<=(const Numeric&) const;
         int operator>=(const Numeric&) const;

         Number<void> operator+(const Number<void>&) const;
         Number<void> operator-(const Number<void>&) const;
         Number<void> operator*(const Number<void>&) const;
         Number<void> operator/(const Number<void>&) const;
         Number<void> operator%(const Number<void>&) const;
         Number<void> &operator+=(const Number<void>&);
         Number<void> &operator*=(const Number<void>&);
         Number<void> &operator/=(const Number<void>&);
         Number<void> &operator%=(const Number<void>&);
         int operator==(const Number<void>&) const;
         int operator<(const Number<void>&) const;
         int operator>(const Number<void>&) const;
         int operator<=(const Number<void>&) const;
         int operator>=(const Number<void>&) const;

         Numeric &operator*() const;
};

Number<void> operator+(const Numeric&,const Number<void>&);
Number<void> operator-(const Numeric&,const Number<void>&);
Number<void> operator*(const Numeric&,const Number<void>&);
Number<void> operator/(const Numeric&,const Number<void>&);
Number<void> operator%(const Numeric&,const Number<void>&);

// Template specialization for Rational<Number<void> > and identities
template <> Rational<Number<void> >::operator double() const;
template <> Number<void> zero(Number<void>);
template <> Number<void> one(Number<void>);
#endif
#endif


#endif
