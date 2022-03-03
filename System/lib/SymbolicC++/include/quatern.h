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


// quatern.h
// Template Class for Quaternions


#ifndef QUATERNION_H
#define QUATERNION_H

#include <iostream>
#include <cmath>       // for sqrt()
#include "identity.h"
using namespace std;

template <class T> class Quaternion
{
   private:
      // Data Fields      
      T r, i, j, k;

   public:
      // Constructors
      Quaternion();
      Quaternion(T,T,T,T);
      Quaternion(const Quaternion<T>&);
      ~Quaternion();

      // Operators
      const Quaternion<T> &operator = (const Quaternion<T>&);
      Quaternion<T> operator + (const Quaternion<T>&);
      Quaternion<T> operator - (const Quaternion<T>&);
      Quaternion<T> operator - () const;
      Quaternion<T> operator * (const Quaternion<T>&);
      Quaternion<T> operator * (T);
      Quaternion<T> operator / (const Quaternion<T>&);
      Quaternion<T> operator ~ () const;

      // Member Functions
      Quaternion<T> sqr();
      Quaternion<T> conjugate() const;
      Quaternion<T> inverse() const;
      double magnitude() const;

      // Streams
      ostream &print(ostream &) const;
      istream &input(istream &s);
};

template <class T> Quaternion<T>::Quaternion() 
   : r(zero(T())),i(zero(T())),j(zero(T())),k(zero(T())) {}

template <class T> Quaternion<T>::Quaternion(T r1,T i1,T j1,T k1)
   : r(r1),i(i1),j(j1),k(k1) {}

template <class T> Quaternion<T>::Quaternion(const Quaternion<T> &arg)
   : r(arg.r),i(arg.i),j(arg.j),k(arg.k) {}

template <class T> Quaternion<T>::~Quaternion() {}

template <class T>
const Quaternion<T> &Quaternion<T>::operator = (const Quaternion<T> &rvalue)
{
   r = rvalue.r; i = rvalue.i; j = rvalue.j; k = rvalue.k;
   return *this;
}

template <class T>
Quaternion<T> Quaternion<T>::operator + (const Quaternion<T> &arg)
{ return Quaternion<T>(r+arg.r,i+arg.i,j+arg.j,k+arg.k); }

template <class T>
Quaternion<T> Quaternion<T>::operator - (const Quaternion<T> &arg)
{ return Quaternion<T>(r-arg.r,i-arg.i,j-arg.j,k-arg.k); }

template <class T>
Quaternion<T> Quaternion<T>::operator - () const
{ return Quaternion<T>(-r,-i,-j,-k); }

template <class T>
Quaternion<T> Quaternion<T>::operator * (const Quaternion<T> &arg)
{
   return Quaternion<T>(r*arg.r - i*arg.i - j*arg.j - k*arg.k,
                        r*arg.i + i*arg.r + j*arg.k - k*arg.j,
                        r*arg.j + j*arg.r + k*arg.i - i*arg.k,
                        r*arg.k + k*arg.r + i*arg.j - j*arg.i);
}

template <class T>
Quaternion<T> Quaternion<T>::operator * (T arg)
{ return Quaternion<T>(r*arg,i*arg,j*arg,k*arg); }

template <class T>
Quaternion<T> Quaternion<T>::operator / (const Quaternion<T> &arg)
{ return *this * arg.inverse(); }
 
// Normalize Quaternion
template <class T>
Quaternion<T> Quaternion<T>::operator ~ () const
{
   Quaternion<T> result;
   double length = magnitude();
   result.r = r/length; result.i = i/length;
   result.j = j/length; result.k = k/length;
   return result;
}

template <class T> Quaternion<T> Quaternion<T>::sqr()
{
   Quaternion<T> result;
   T temp;
   T two = one(T()) + one(T());
   temp = two*r;
   result.r = r*r - i*i - j*j - k*k;
   result.i = temp*i; result.j = temp*j; result.k = temp*k;
   return result;
}

template <class T> Quaternion<T> Quaternion<T>::conjugate() const
{ return Quaternion<T>(r,-i,-j,-k); }

template <class T> Quaternion<T> Quaternion<T>::inverse() const
{
   Quaternion<T> temp1(conjugate());
   T             temp2 = r*r + i*i + j*j + k*k;
   return Quaternion<T>(temp1.r/temp2,temp1.i/temp2,
                        temp1.j/temp2,temp1.k/temp2);
}

template <class T> double Quaternion<T>::magnitude() const
{ return sqrt(r*r + i*i + j*j + k*k); }

template <class T> ostream &Quaternion<T>::print(ostream &s) const
{
   s << "(" << r << "," << i << ","
            << j << "," << k << ")";
   return s;
}

template <class T> istream &Quaternion<T>::input(istream &s)
{
   s >> r >> i >> j >> k;
   return s;
}

template <class T> Quaternion<T> operator * (T factor,Quaternion<T> &arg)
{ return arg * factor; }

template <class T>
ostream & operator << (ostream &s,const Quaternion<T> &arg)
{ return arg.print(s); }

template <class T>
istream & operator >> (istream &s,Quaternion<T> &arg)
{ return arg.input(s); }

#endif
