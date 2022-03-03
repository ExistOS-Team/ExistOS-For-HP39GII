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


// vector.h
// Vector class

#ifndef MVECTOR_H
#define MVECTOR_H

#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>
#include "identity.h"
using namespace std;

// definition of class Vector
template <class T> class Vector: public vector<T>
{
   public:
      // Constructors
      Vector();
      Vector(int);
      Vector(int,const T&);
      Vector(const Vector<T>&);
      ~Vector();

      // Member Functions
      T& operator [] (int);
      const T& operator [] (int) const;
      void reset(int);
      void reset(int,const T&);

      // Arithmetic Operators
      const Vector<T>& operator = (const T&);
      Vector<T> operator + () const;
      Vector<T> operator - () const;

      Vector<T> operator += (const Vector<T>&);
      Vector<T> operator -= (const Vector<T>&);
      Vector<T> operator *= (const Vector<T>&);
      Vector<T> operator /= (const Vector<T>&);
      Vector<T> operator +  (const Vector<T>&) const;
      Vector<T> operator -  (const Vector<T>&) const;
      Vector<T> operator *  (const Vector<T>&) const;
      Vector<T> operator /  (const Vector<T>&) const;

      Vector<T> operator += (const T&);
      Vector<T> operator -= (const T&);
      Vector<T> operator *= (const T&);
      Vector<T> operator /= (const T&);
      Vector<T> operator +  (const T&) const;
      Vector<T> operator -  (const T&) const;
      Vector<T> operator *  (const T&) const;
      Vector<T> operator /  (const T&) const;

      T operator | (const Vector<T>&) const; // Dot product / Inner product
      Vector<T> operator % (const Vector<T>&) const; // Cross product

      ostream &output(ostream&) const;
      istream &input(istream&);
};

// implementation of class Vector
template <class T> Vector<T>::Vector() : vector<T>() { }

template <class T> Vector<T>::Vector(int n) : vector<T>(n) { }

template <class T> Vector<T>::Vector(int n,const T &value)
 : vector<T>(n,value) { }

template <class T> Vector<T>::Vector(const Vector<T> &v) : vector<T>(v) { }

template <class T> Vector<T>::~Vector() { }

template <class T> void Vector<T>::reset(int length)
{ reset(length, zero(T())); }

template <class T> T& Vector<T>::operator [] (int i)
{ return vector<T>::at(i); }

template <class T> const T& Vector<T>::operator [] (int i) const
{ return vector<T>::at(i); }

template <class T> void Vector<T>::reset(int length, const T &value)
{
   vector<T>::resize(length);
   for(int i=0;i<length;i++) vector<T>::at(i) = value;
}

template <class T> const Vector<T> & Vector<T>::operator = (const T &value)
{
   int length = vector<T>::size();
   for(int i=0;i<length;i++) vector<T>::at(i) = value;
   return *this;
}

template <class T> Vector<T> Vector<T>::operator + () const
{ return *this; }

template <class T> Vector<T> Vector<T>::operator - () const
{ return *this * T(-1); }

template <class T> Vector<T> Vector<T>::operator += (const Vector<T> &v)
{
   int length = vector<T>::size();
   assert(vector<T>::size()==v.size());
   for(int i=0;i<length;i++) vector<T>::at(i) += v[i];
   return *this;
}

template <class T> Vector<T> Vector<T>::operator -= (const Vector<T> &v)
{
   int length = vector<T>::size();
   assert(vector<T>::size()==v.size());
   for(int i=0;i<length;i++) vector<T>::at(i) -= v[i];
   return *this;
}

template <class T> Vector<T> Vector<T>::operator *= (const Vector<T> &v)
{
   int length = vector<T>::size();
   assert(vector<T>::size()==v.size());
   for(int i=0;i<length;i++) vector<T>::at(i) *= v[i];
   return *this;
}

template <class T> Vector<T> Vector<T>::operator /= (const Vector<T> &v)
{
   int length = vector<T>::size();
   assert(vector<T>::size()==v.size());
   for(int i=0;i<length;i++) vector<T>::at(i) /= v[i];
   return *this;
}

template <class T> 
Vector<T> Vector<T>::operator + (const Vector<T> &v) const
{
   Vector<T> result(*this);
   return result += v;
}

template <class T> 
Vector<T> Vector<T>::operator - (const Vector<T> &v) const
{
   Vector<T> result(*this);
   return result -= v;
}

template <class T> 
Vector<T> Vector<T>::operator * (const Vector<T> &v) const
{
   Vector<T> result(*this);
   return result *= v;
}

template <class T> 
Vector<T> Vector<T>::operator / (const Vector<T> &v) const
{
   Vector<T> result(*this);
   return result /= v;
}

template <class T> Vector<T> Vector<T>::operator += (const T &c)
{
   int length = vector<T>::size();
   for(int i=0;i<length;i++) vector<T>::at(i) += c;
   return *this;
}

template <class T> Vector<T> Vector<T>::operator -= (const T &c)
{
   int length = vector<T>::size();
   for(int i=0;i<length;i++) vector<T>::at(i) -= c;
   return *this;
}

template <class T> Vector<T> Vector<T>::operator *= (const T &c)
{
   int length = vector<T>::size();
   for(int i=0;i<length;i++) vector<T>::at(i) *= c;
   return *this;
}

template <class T> Vector<T> Vector<T>::operator /= (const T &c)
{
   int length = vector<T>::size();
   for(int i=0;i<length;i++) vector<T>::at(i) /= c;
   return *this;
}

template <class T> Vector<T> Vector<T>::operator + (const T &c) const
{
   Vector<T> result(*this);
   return result += c;
}

template <class T> Vector<T> Vector<T>::operator - (const T &c) const
{
   Vector<T> result(*this);
   return result -= c;
}

template <class T> Vector<T> Vector<T>::operator * (const T &c) const
{
   Vector<T> result(*this);
   return result *= c;
}

template <class T> Vector<T> Vector<T>::operator / (const T &c) const
{
   Vector<T> result(*this);
   return result /= c;
}

template <class T> Vector<T> operator + (const T &c, const Vector<T> &v)
{ return v+c; }

template <class T> Vector<T> operator - (const T &c, const Vector<T> &v)
{ return -v+c; }

template <class T> Vector<T> operator * (const T &c, const Vector<T> &v)
{ return v*c; }

template <class T> Vector<T> operator / (const T &c, const Vector<T> &v)
{
   int length = v.size();
   Vector<T> result(v.size());
   for(int i=0;i<length;i++) result[i] = c/v[i];
   return result;
}

// Dot Product / Inner Product
template <class T> T Vector<T>::operator | (const Vector<T> &v) const
{
   int length = vector<T>::size();
   assert(vector<T>::size() == v.size());
   T result(zero(T()));
   for(int i=0;i<length;i++) result = result + vector<T>::at(i)*v[i];
   return result;
}

// Cross Product
template <class T> 
Vector<T> Vector<T>::operator % (const Vector<T> &v) const
{
   assert(vector<T>::size() == 3 && v.size() == 3);
   Vector<T> result(3);
   result[0] = vector<T>::at(1)*v[2]-v[1]*vector<T>::at(2);
   result[1] = v[0]*vector<T>::at(2)-vector<T>::at(0)*v[2];
   result[2] = vector<T>::at(0)*v[1]-v[0]*vector<T>::at(1);
   return result;
}

template <class T> ostream& Vector<T>::output(ostream &s) const
{
   int lastnum = vector<T>::size();
   for(int i=0;i<lastnum;i++) s << "[" << vector<T>::at(i) << "]" << endl;
   return s;
}

template <class T> ostream& operator << (ostream &s,const Vector<T> &v)
{ return v.output(s); }

template <class T> istream& Vector<T>::input(istream &s)
{
   int i, num;
   s.clear();                 // set stream state to good
   s >> num;                  // read size of Vector
   if(! s.good()) return s;  // can't get an integer, just return
   vector<T>::resize(num);
   for(i=0;i<num;i++)
   {
      s >> vector<T>::at(i); // read in entries
      if(! s.good())
      {
         s.clear(s.rdstate() | ios::badbit);
         return s;
      }
   }
   return s;
}

template <class T> istream & operator >> (istream &s,Vector<T> &v)
{ return v.input(s); }
#endif
