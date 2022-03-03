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

#include "vecnorm.h"

double norm1(const Vector<double> &v)
{
   double result(0);
   for(int i=0;i<int(v.size());i++) result = result + fabs(v[i]);
   return result;
}
double normI(const Vector<double> &v)
{
   double maxItem(fabs(v[0])), temp;
   for(int i=1;i<int(v.size());i++)
   {
      temp = fabs(v[i]);
      if(temp > maxItem) maxItem = temp;
   }
   return maxItem;
}
