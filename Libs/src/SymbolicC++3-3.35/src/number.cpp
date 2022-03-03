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
#ifndef SYMBOLIC_CPLUSPLUS_NUMBER_DEFINE
#define SYMBOLIC_CPLUSPLUS_NUMBER_DEFINE
#define SYMBOLIC_CPLUSPLUS_NUMBER

Numeric::Numeric() : CloningSymbolicInterface() {}

Numeric::Numeric(const Numeric &n) : CloningSymbolicInterface(n) {}

// Template specialization for Rational<Number<void> >
template <> Rational<Number<void> >::operator double() const
{
 pair<Number<void>,Number<void> > pr = p.match(p,q);
 if(pr.first.numerictype() == typeid(int))
 {
  CastPtr<const Number<int> > i1 = pr.first;
  CastPtr<const Number<int> > i2 = pr.second;
  return double(Rational<int>(i1->n,i2->n));
 }
 if(pr.first.numerictype() == typeid(Verylong))
 {
  CastPtr<const Number<Verylong> > v1 = pr.first;
  CastPtr<const Number<Verylong> > v2 = pr.second;
  return double(Rational<Verylong>(v1->n,v2->n));
 }
 cerr << "convert to double : "
      << pr.first.numerictype().name() << endl;
 throw SymbolicError(SymbolicError::NotDouble);
 return 0.0;
}

////////////////////////////////////
// Implementation of Numeric      //
////////////////////////////////////

pair<Number<void>,Number<void> >
Numeric::match(const Numeric &n1,const Numeric &n2)
{
 const type_info &t1 = n1.numerictype();
 const type_info &t2 = n2.numerictype();
 const type_info &type_int = typeid(int);
 const type_info &type_double = typeid(double);
 const type_info &type_verylong = typeid(Verylong);
 const type_info &type_rational = typeid(Rational<Number<void> >);

 if(t1 == type_int)
 {
  CastPtr<const Number<int> > i1 = n1;

  if(t2 == type_int)
   return pair<Number<void>,Number<void> >(n1,n2);
  if(t2 == type_double)
   return pair<Number<void>,Number<void> >(Number<double>(i1->n),n2);
  if(t2 == type_verylong)
   return pair<Number<void>,Number<void> >(Number<Verylong>(i1->n),n2);
  if(t2 == type_rational)
   return pair<Number<void>,Number<void> >
          (Number<Rational<Number<void> > >
              (Rational<Number<void> >(Number<void>(*i1)))
          ,n2);
  cerr << "Numeric cannot use " << t2.name() << endl;
  throw SymbolicError(SymbolicError::UnsupportedNumeric);
 }
 if(t1 == type_double)
 {
  CastPtr<const Number<double> > d1 = n1;

  if(t2 == type_int)
  {
   CastPtr<const Number<int> > i2 = n2;
   return pair<Number<void>,Number<void> >(n1,Number<double>(i2->n));
  }
  if(t2 == type_double)
   return pair<Number<void>,Number<void> >(n1,n2);
  if(t2 == type_verylong)
  {
   CastPtr<const Number<Verylong> > v2 = n2;
   return pair<Number<void>,Number<void> >(n1,Number<double>(v2->n));
  }
  if(t2 == type_rational)
  {
   CastPtr<const Number<Rational<Number<void> > > > r2 = n2;
   return pair<Number<void>,Number<void> >
          (n1,Number<double>(double(r2->n)));
  }
  cerr << "Numeric cannot use " << t2.name() << endl;
  throw SymbolicError(SymbolicError::UnsupportedNumeric);
 }
 if(t1 == type_verylong)
 {
  CastPtr<const Number<Verylong> > v1 = n1;

  if(t2 == type_int)
  {
   CastPtr<const Number<int> > i2 = n2;
   return pair<Number<void>,Number<void> >(n1,Number<Verylong>(i2->n));
  }
  if(t2 == type_double)
   return pair<Number<void>,Number<void> >(Number<double>(v1->n),n2);
  if(t2 == type_verylong)
   return pair<Number<void>,Number<void> >(n1,n2);
  if(t2 == type_rational)
   return pair<Number<void>,Number<void> >
          (Number<Rational<Number<void> > >
              (Rational<Number<void> >(Number<void>(*v1))),n2);
  cerr << "Numeric cannot use " << t2.name() << endl;
  throw SymbolicError(SymbolicError::UnsupportedNumeric);
 }
 if(t1 == type_rational)
 {
  CastPtr<const Number<Rational<Number<void> > > > r1 = n1;

  if(t2 == type_int)
  {
   CastPtr<const Number<int> > i2 = n2;
   return pair<Number<void>,Number<void> >
          (n1,
          Number<Rational<Number<void> > >
              (Rational<Number<void> >(Number<void>(*i2))));
  }
  if(t2 == type_double)
   return pair<Number<void>,Number<void> >
          (Number<double>(double(r1->n)),n2);
  if(t2 == type_verylong)
  {
   CastPtr<const Number<Verylong> > v2 = n2;
   return pair<Number<void>,Number<void> >
          (n1,
          Number<Rational<Number<void> > >
              (Rational<Number<void> >(Number<void>(*v2))));
  }
  if(t2 == type_rational)
   return pair<Number<void>,Number<void> >(n1,n2);
  cerr << "Numeric cannot use " << t2.name() << endl;
  throw SymbolicError(SymbolicError::UnsupportedNumeric);
 }
 cerr << "Numeric cannot use " << t1.name() << endl;
 throw SymbolicError(SymbolicError::UnsupportedNumeric);
 return pair<Number<void>,Number<void> >(n1,n2);
}

Symbolic Numeric::subst(const Symbolic &x,const Symbolic &y,int &n) const
{
 if(*this == x) { ++n; return y; }
 return *this;
}

int Numeric::compare(const Symbolic &s) const
{
 if(s.type() != type()) return 0;
 pair<Number<void>,Number<void> >
  p = Number<void>::match(*this,*Number<void>(s));
 return p.first->cmp(*(p.second));
}

Symbolic Numeric::df(const Symbolic &s) const
{ return Number<int>(0); }

Symbolic Numeric::integrate(const Symbolic &s) const
{ return Symbolic(*this) * s; }

Symbolic Numeric::coeff(const Symbolic &s) const
{
 if(s.type() == typeid(Numeric)) return *this / Number<void>(s);
 return Number<int>(0);
}

Expanded Numeric::expand() const
{ return *this; }

int Numeric::commute(const Symbolic &s) const
{ return 1; }

PatternMatches
Numeric::match(const Symbolic &s, const list<Symbolic> &p) const
{
 PatternMatches l;
 if(*this == s) pattern_match_TRUE(l);
 else pattern_match_FALSE(l);
 return l;
}

PatternMatches
Numeric::match_parts(const Symbolic &s, const list<Symbolic> &p) const
{ return s.match(*this, p); }

////////////////////////////////////
// Implementation of Number       //
////////////////////////////////////

template <class T> Number<T>::Number() : n()
{ simplified = expanded = 1; }

template <class T> Number<T>::Number(const Number &n)
 : Numeric(n), n(n.n) {}

template <class T> Number<T>::Number(const T &t) : n(t)
{ simplified = 0; expanded = 1; }

template <class T> Number<T>::~Number() {}

template <class T> Number<T> &Number<T>::operator=(const Number &n)
{
 if(this != &n) n = n.n;
 return *this;
}

template <class T> Number<T> &Number<T>::operator=(const T &t)
{ n = t; return *this; }

template <class T> void Number<T>::print(ostream &o) const
{ o << n; }

template <class T> const type_info &Number<T>::type() const
{ return typeid(Numeric); }

template <class T> const type_info &Number<T>::numerictype() const
{ return typeid(T); }

template <class T>
Simplified Number<T>::simplify() const
{ return *this; }

template <>
Simplified Number<Verylong>::simplify() const
{
 if(n <= Verylong(numeric_limits<int>::max())
    && n > Verylong(numeric_limits<int>::min()))
  return Number<int>(n);
 return *this;
}

template <>
Simplified Number<Rational<Number<void> > >::simplify() const
{
 if(n.den().isOne()) return *(n.num());
 return *this;
}

template <class T> Number<void> Number<T>::add(const Numeric &x) const
{
 if(numerictype() != x.numerictype())
   throw SymbolicError(SymbolicError::IncompatibleNumeric);
 CastPtr<const Number<T> > p = x;
 return Number<T>(n + p->n);
}

template <class T> Number<void> Number<T>::mul(const Numeric &x) const
{
 if(numerictype() != x.numerictype())
   throw SymbolicError(SymbolicError::IncompatibleNumeric);
 CastPtr<const Number<T> > p = x;
 return Number<T>(n * p->n);
}

template <class T> Number<void> Number<T>::div(const Numeric &x) const
{
 if(numerictype() != x.numerictype())
   throw SymbolicError(SymbolicError::IncompatibleNumeric);
 CastPtr<const Number<T> > p = x;
 return Number<T>(n / p->n);
}

template <class T> Number<void> Number<T>::mod(const Numeric &x) const
{
 if(numerictype() != x.numerictype())
   throw SymbolicError(SymbolicError::IncompatibleNumeric);
 CastPtr<const Number<T> > p = x;
 return Number<T>(n - p->n * (n / p->n));
}

template <> Number<void> Number<int>::add(const Numeric &x) const
{
 if(numerictype() != x.numerictype())
   throw SymbolicError(SymbolicError::IncompatibleNumeric);
 CastPtr<const Number<int> > p = x;
 int sum = n + p->n;
 if((n < 0 && p->n < 0 && sum >= 0) ||
    (n > 0 && p->n > 0 && sum <= 0))
 return Number<Verylong>(Verylong(n) + Verylong(p->n));
 return Number<int>(n + p->n);
}

template <> Number<void> Number<int>::mul(const Numeric &x) const
{
 if(numerictype() != x.numerictype())
   throw SymbolicError(SymbolicError::IncompatibleNumeric);
 CastPtr<const Number<int> > p = x;
 int product = n * p->n;
 if(n != 0 && product / n != p->n)
  return Number<Verylong>(Verylong(n) * Verylong(p->n));
 return Number<int>(product);
}

template <> Number<void> Number<int>::div(const Numeric &x) const
{
 if(numerictype() != x.numerictype())
   throw SymbolicError(SymbolicError::IncompatibleNumeric);
 CastPtr<const Number<int> > p = x;
 if(n % p->n != 0)   
  return Number<Rational<Number<void> > >
           (Rational<Number<void> >(Number<void>(*this),Number<void>(x)));
 return Number<int>(n / p->n);
}

template <> Number<void> Number<int>::mod(const Numeric &x) const
{
 if(numerictype() != x.numerictype())
   throw SymbolicError(SymbolicError::IncompatibleNumeric);
 return Number<int>(n % CastPtr<const Number<int> >(x)->n);
}

template <> Number<void> Number<double>::mod(const Numeric &x) const
{
 if(numerictype() != x.numerictype())
   throw SymbolicError(SymbolicError::IncompatibleNumeric);
 return Number<double>(fmod(n,CastPtr<const Number<double> >(x)->n));
}

template <> Number<void> Number<Verylong>::div(const Numeric &x) const
{
 if(numerictype() != x.numerictype())
   throw SymbolicError(SymbolicError::IncompatibleNumeric);
 CastPtr<const Number<Verylong> > p = x;
 if(n % p->n != Verylong(0))   
  return Number<Rational<Number<void> > >
           (Rational<Number<void> >(Number<void>(*this),Number<void>(x)));
 return Number<Verylong>(n / p->n);
}

template <class T> int Number<T>::isZero() const
{ return (n == zero(T())); }

template <class T> int Number<T>::isOne() const
{ return (n == one(T())); }

template <class T> int Number<T>::isNegative() const
{ return (n < zero(T())); }

template <class T> int Number<T>::cmp(const Numeric &x) const
{
 if(numerictype() != x.numerictype())
   throw SymbolicError(SymbolicError::IncompatibleNumeric);
 return (numerictype() == x.numerictype()) &&
        (n == CastPtr<const Number<T> >(x)->n);
}

////////////////////////////////////
// Implementation of Number<void> //
////////////////////////////////////

Number<void>::Number() : CastPtr<Numeric>(Number<int>(0)) {}

Number<void>::Number(const Number &n) : CastPtr<Numeric>(n) {}

Number<void>::Number(const Numeric &n) : CastPtr<Numeric>(n) {}

Number<void>::Number(const Symbolic &n) : CastPtr<Numeric>(Number<int>(0))
{
 if(n.type() != typeid(Numeric))
   throw SymbolicError(SymbolicError::NotNumeric);
 CastPtr<Numeric>::operator=(n);
}

Number<void>::~Number() {}

const type_info &Number<void>::numerictype() const
{ return (*this)->numerictype(); }

int Number<void>::isZero() const
{ return (*this)->isZero(); }

int Number<void>::isOne() const
{ return (*this)->isOne(); }

int Number<void>::isNegative() const
{ return (*this)->isNegative(); }

pair<Number<void>,Number<void> >
Number<void>::match(const Numeric &n1,const Numeric &n2)
{ return Numeric::match(n1,n2); }

pair<Number<void>,Number<void> >
Number<void>::match(const Number<void> &n1,const Number<void> &n2)
{ return Numeric::match(*n1,*n2); }

Number<void> Number<void>::operator+(const Numeric &n) const
{
 pair<Number<void>,Number<void> > p = Number<void>::match(*this,n);
 return p.first->add(*(p.second));
}

Number<void> Number<void>::operator-(const Numeric &n) const
{
 pair<Number<void>,Number<void> > p = Number<void>::match(*this,n);
 return p.first->add(*(p.second));
}

Number<void> Number<void>::operator*(const Numeric &n) const
{
 pair<Number<void>,Number<void> > p = Number<void>::match(*this,n);
 return p.first->mul(*(p.second));
}

Number<void> Number<void>::operator/(const Numeric &n) const
{
 pair<Number<void>,Number<void> > p = Number<void>::match(*this,n);
 return p.first->div(*(p.second));
}

Number<void> Number<void>::operator%(const Numeric &n) const
{
 pair<Number<void>,Number<void> > p = Number<void>::match(*this,n);
 return p.first->mod(*(p.second));
}

Number<void> &Number<void>::operator+=(const Numeric &n)
{ return *this = *this + n; }

Number<void> &Number<void>::operator*=(const Numeric &n)
{ return *this = *this * n; }

Number<void> &Number<void>::operator/=(const Numeric &n)
{ return *this = *this / n; }

Number<void> &Number<void>::operator%=(const Numeric &n)
{ return *this = *this % n; }

int Number<void>::operator==(const Numeric &n) const
{
 pair<Number<void>,Number<void> > p = Number<void>::match(*(*this),n);
 return p.first->compare(*(p.second));
}

int Number<void>::operator<(const Numeric &n) const
{
 pair<Number<void>,Number<void> > p = Number<void>::match(*(*this),n);
 return (p.first - p.second).isNegative();
}

int Number<void>::operator>(const Numeric &n) const
{ return !(*this < n) && !(*this == n); }

int Number<void>::operator<=(const Numeric &n) const
{ return (*this < n) || (*this == n); }

int Number<void>::operator>=(const Numeric &n) const
{ return !(*this < n); }

Number<void> Number<void>::operator+(const Number<void> &n) const
{ return operator+(*n); }

Number<void> Number<void>::operator-(const Number<void> &n) const
{ return operator-(*n); }

Number<void> Number<void>::operator*(const Number<void> &n) const
{ return operator*(*n); }

Number<void> Number<void>::operator/(const Number<void> &n) const
{ return operator/(*n); }

Number<void> Number<void>::operator%(const Number<void> &n) const
{ return operator%(*n); }

Number<void> &Number<void>::operator+=(const Number<void> &n)
{ return *this = *this + n; }

Number<void> &Number<void>::operator*=(const Number<void> &n)
{ return *this = *this * n; }

Number<void> &Number<void>::operator/=(const Number<void> &n)
{ return *this = *this / n; }

Number<void> &Number<void>::operator%=(const Number<void> &n)
{ return *this = *this % n; }

int Number<void>::operator==(const Number<void> &n) const
{ return operator==(*n); }

int Number<void>::operator<(const Number<void> &n) const
{ return operator<(*n); }

int Number<void>::operator>(const Number<void> &n) const
{ return operator>(*n); }

int Number<void>::operator<=(const Number<void> &n) const
{ return operator<=(*n); }

int Number<void>::operator>=(const Number<void> &n) const
{ return operator>=(*n); }

Numeric &Number<void>::operator*() const
{ return CastPtr<Numeric>::operator*(); }

template <> Number<void> zero(Number<void>)
{ return Number<int>(0); }

template <> Number<void> one(Number<void>)
{ return Number<int>(1); }

Number<void> operator+(const Numeric &n1,const Number<void> &n2)
{ return Number<void>(n1) + n2; }

Number<void> operator-(const Numeric &n1,const Number<void> &n2)
{ return Number<void>(n1) - n2; }

Number<void> operator*(const Numeric &n1,const Number<void> &n2)
{ return Number<void>(n1) * n2; }

Number<void> operator/(const Numeric &n1,const Number<void> &n2)
{ return Number<void>(n1) / n2; }

Number<void> operator%(const Numeric &n1,const Number<void> &n2)
{ return Number<void>(n1) % n2; }

#endif
#endif

