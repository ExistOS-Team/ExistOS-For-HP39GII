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


#ifndef IDENTITY_H
#define IDENTITY_H

#include <iostream>
#include <cstdlib>
#include <complex>
using namespace std;

template <class T> T zero(T) { static const T z = T() - T(); return z; }

template <class T> T one(T)
{
 static T z(zero(T()));
 static const T o = ++z;
 return o;
}

template <> char zero(char);
template <> char one(char);
template <> short zero(short);
template <> short one(short);
template <> int zero(int);
template <> int one(int);
template <> long zero(long);
template <> long one(long);
template <> float zero(float);
template <> float one(float);
template <> double zero(double);
template <> double one(double);


template <class T>
complex<T> zero(complex<T>) { return complex<T>(zero(T())); }

template <class T>
complex<T> one(complex<T>) { return complex<T>(one(T())); }

#endif
