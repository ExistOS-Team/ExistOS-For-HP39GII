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


// matnorm.h
// Norms of Matrices

#ifndef MATNORM_H
#define MATNORM_H

#include <iostream>
#include <math.h>
#include "vector.h"
#include "vecnorm.h"
#include "matrix.h"
#include "identity.h"
using namespace std;

template <class T> T norm1(const Matrix<T> &m)
{
   T maxItem(0), temp;
   int i,j;
   for(i=0;i<m.rows();i++) maxItem += m[i][0];
   for(i=1;i<m.cols();i++)
   {
      temp = zero(T());
      for(j=0;j<m.rows();j++) temp += abs(m[j][i]);
      if(temp > maxItem) maxItem = temp;
   }
   return maxItem;
}

template <class T> T normI(const Matrix<T> &m)
{
   T maxItem(norm1(m[0]));
   for(int i=1;i<m.rows();i++)
      if(norm1(m[i]) > maxItem) maxItem = norm1(m[i]);
   return maxItem;
}

template <class T> T normH(const Matrix<T> &m)
{ return sqrt((m*(m.transpose())).trace()); }
#endif
