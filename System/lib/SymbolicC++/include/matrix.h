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


// matrix.h
// Matrix class

#ifndef MATRIX_H
#define MATRIX_H

#include <iostream>
#include <cmath>
#include <cassert>
#include <string>
#include <utility>
#include "identity.h"
#include "vector.h"
using namespace std;

// definition of class Matrix
template <class T> class Matrix
{
   protected:
      // Data Fields
      int rowNum, colNum;
      Vector<Vector<T> > mat;

   public:
      // Constructors
      Matrix();
      Matrix(int,int);
      Matrix(int,int,const T&);
      Matrix(const Vector<T>&);
      Matrix(const Matrix<T>&);
      ~Matrix();

      // Member Functions
      Vector<T>& operator [] (int);
      const Vector<T>& operator [] (int) const;
      Vector<T>  operator () (int) const;

      Matrix<T> identity();
      Matrix<T> transpose() const;
      Matrix<T> inverse() const;
      T trace() const;
      T determinant() const;

      int rows() const; 
      int cols() const; 
      void resize(int,int);
      void resize(int,int,const T&);
      void fill(const T&);

      // Arithmetic Operators
      const Matrix<T>& operator = (const Matrix<T>&);
      const Matrix<T>& operator = (const T&);

      Matrix<T> operator + () const;
      Matrix<T> operator - () const;
      Matrix<T> operator += (const Matrix<T>&);
      Matrix<T> operator -= (const Matrix<T>&);
      Matrix<T> operator *= (const Matrix<T>&);
      Matrix<T> operator +  (const Matrix<T>&) const;
      Matrix<T> operator -  (const Matrix<T>&) const;
      Matrix<T> operator *  (const Matrix<T>&) const;
      Vector<T> operator *  (const Vector<T>&) const;

      Matrix<T> operator += (const T&);
      Matrix<T> operator -= (const T&);
      Matrix<T> operator *= (const T&);
      Matrix<T> operator /= (const T&);
      Matrix<T> operator +  (const T&) const;
      Matrix<T> operator -  (const T&) const;
      Matrix<T> operator *  (const T&) const;
      Matrix<T> operator /  (const T&) const;

      Vector<T> vec() const;
      Matrix<T> kron(const Matrix<T>&) const;
      Matrix<T> dsum(const Matrix<T>&) const;
      Matrix<T> hadamard(const Matrix<T>&) const;
      pair<Matrix<T>, Matrix<T> > LU() const;

      ostream &output(ostream&) const;
      istream &input(istream&);
};

template <class T> T tr(const Matrix<T> &m) { return m.trace(); }
template <class T> T det(const Matrix<T> &m) { return m.determinant(); }

// implementation of class Matrix
template <class T> Matrix<T>::Matrix() 
   : rowNum(0), colNum(0), mat() {}

template <class T> Matrix<T>::Matrix(int r,int c)
   : rowNum(r), colNum(c), mat(r)
{ for(int i=0;i<r;i++) mat[i].resize(c); }

template <class T> Matrix<T>::Matrix(int r,int c,const T &value)
   : rowNum(r), colNum(c), mat(r)
{ for(int i=0;i<r;i++) mat[i].resize(c,value); }

template <class T> Matrix<T>::Matrix(const Vector<T> &v)
   : rowNum(v.size()), colNum(1), mat(rowNum)
{ for(int i=0;i<rowNum;i++) mat[i].resize(1,v[i]); }

template <class T> Matrix<T>::Matrix(const Matrix<T> &m)
   : rowNum(m.rowNum), colNum(m.colNum), mat(m.mat)
{ }

template <class T> Matrix<T>::~Matrix() { }

template <class T> Vector<T> & Matrix<T>::operator [] (int index)
{
   assert(index>=0 && index<rowNum);
   return mat[index];
}

template <class T>
const Vector<T> & Matrix<T>::operator [] (int index) const
{
   assert(index>=0 && index<rowNum);
   return mat[index];
}


template <class T> Vector<T> Matrix<T>::operator () (int index) const
{
   assert(index>=0 && index<colNum);
   Vector<T> result(rowNum);
   for(int i=0;i<rowNum;i++) result[i] = mat[i][index];
   return result;
}

template <class T> Matrix<T> Matrix<T>::identity()
{
   for(int i=0;i<rowNum;i++)
      for(int j=0;j<colNum;j++)
         if(i==j) mat[i][j] = one(T());
         else     mat[i][j] = zero(T());
   return *this;
}

template <class T> Matrix<T> Matrix<T>::transpose() const
{
   Matrix<T> result(colNum,rowNum);
   for(int i=0;i<rowNum;i++)
      for(int j=0;j<colNum;j++) result[j][i] = mat[i][j];
   return result;
}

// Symbolical Inverse using Leverrier's Method
template <class T> Matrix<T> Matrix<T>::inverse() const
{
   assert(rowNum == colNum);
   Matrix<T> B(*this), D, I(rowNum, colNum);
   T c0(B.trace()), c1, j(one(T()));
   int i;
   I.identity();
   for(j++,i=2;i<rowNum;i++,j++)
   {
      B = *this*(B-c0*I); 
      c0 = B.trace()/j;
   }
   D = *this*(B-c0*I); 
   c1 = D.trace()/j;
   return (B-c0*I)/c1;
}

template <class T> T Matrix<T>::trace() const
{
   assert(rowNum == colNum);
   T result(zero(T()));
   for(int i=0;i<rowNum;i++) result += mat[i][i];
   return result;
}

// Symbolical determinant
template <class T> T Matrix<T>::determinant() const
{
   assert(rowNum==colNum);
   Matrix<T> B(*this), I(rowNum, colNum, zero(T()));
   T c(B.trace());
   int i;
   for(i=0;i<rowNum;i++) I[i][i] = one(T());
   // Note that determinant of int-type gives zero 
   // because of division by T(i)
   for(i=2;i<=rowNum;i++)
   {
      B = *this * (B-c*I); 
      c = B.trace()/T(i);
   }
   if(rowNum%2) return c;
   return -c;
}

template <class T> int Matrix<T>::rows() const
{ return rowNum; }

template <class T> int Matrix<T>::cols() const
{ return colNum; }

template <class T> void Matrix<T>::resize(int r,int c)
{ resize(r, c, zero(T())); }
  
template <class T> void Matrix<T>::resize(int r,int c,const T &value)
{
   mat.resize(r);
   for(int i=0;i<r;i++) mat[i].resize(c,value);
   rowNum = r; colNum = c;
}

template <class T> void Matrix<T>::fill(const T &value)
{
   for(int i=0;i<rowNum;i++)
      for(int j=0;j<colNum;j++) mat[i][j] = value;
}

template <class T> 
const Matrix<T> & Matrix<T>::operator = (const Matrix<T> &m)
{
   if(this == &m) return *this;
   rowNum = m.rowNum; colNum = m.colNum;
   mat = m.mat;
   return *this;
}

template <class T> 
const Matrix<T> & Matrix<T>::operator = (const T &value)
{
   for(int i=0;i<rowNum;i++) mat[i] = value;
   return *this;
}

template <class T> Matrix<T> Matrix<T>::operator + () const
{ return *this; }

template <class T> Matrix<T> Matrix<T>::operator - () const
{ return *this * T(-1); }

template <class T> Matrix<T> Matrix<T>::operator += (const Matrix<T> &m)
{ return *this = *this + m; }

template <class T> Matrix<T> Matrix<T>::operator -= (const Matrix<T> &m)
{ return *this = *this - m; }

template <class T> Matrix<T> Matrix<T>::operator *= (const Matrix<T> &m)
{ return *this = *this * m; }

template <class T> 
Matrix<T> Matrix<T>::operator + (const Matrix<T> &m) const
{
   assert(rowNum == m.rowNum && colNum == m.colNum);
   Matrix<T> result(*this);
   for(int i=0;i<rowNum;i++) result[i] += m[i];
   return result;
}

template <class T> 
Matrix<T> Matrix<T>::operator - (const Matrix<T> &m) const
{
   assert(rowNum == m.rowNum && colNum == m.colNum);
   Matrix<T> result(*this);
   for(int i=0;i<rowNum;i++) result[i] -= m[i];
   return result;
}

template <class T> 
Matrix<T> Matrix<T>::operator * (const Matrix<T> &m) const
{
   assert(colNum == m.rowNum);
   Matrix<T> result(rowNum, m.colNum, zero(T()));
   for(int i=0;i<rowNum;i++)
      for(int j=0;j<m.colNum;j++)
         for(int k=0;k<colNum;k++)
            result[i][j] += mat[i][k]*m[k][j];
   return result;
}

template <class T> 
Vector<T> Matrix<T>::operator * (const Vector<T> &v) const
{
   assert(colNum == v.size());
   Vector<T> result(rowNum);
   // dot product | is used
   for(int i=0;i<rowNum;i++) result[i] = (mat[i] | v);
   return result;
}

template <class T> Matrix<T> Matrix<T>::operator += (const T &c)
{
   assert(rowNum == colNum);
   for(int i=0;i<rowNum;i++) mat[i][i] += c;
   return *this;
}

template <class T> Matrix<T> Matrix<T>::operator -= (const T &c)
{
   assert(rowNum == colNum);
   for(int i=0;i<rowNum;i++) mat[i][i] -= c;
   return *this;
}

template <class T> Matrix<T> Matrix<T>::operator *= (const T &c)
{
   for(int i=0;i<rowNum;i++) mat[i] *= c;
   return *this;
}

template <class T> Matrix<T> Matrix<T>::operator /= (const T &c)
{
   for(int i=0;i<rowNum;i++) mat[i] /= c;
   return *this;
}

template <class T> 
Matrix<T> Matrix<T>::operator + (const T &value) const
{
   assert(rowNum == colNum);
   Matrix<T> result(*this);
   return result += value;
}

template <class T> 
Matrix<T> Matrix<T>::operator - (const T &value) const
{
   assert(rowNum == colNum);
   Matrix<T> result(*this);
   return result -= value;
}

template <class T> 
Matrix<T> Matrix<T>::operator * (const T &value) const
{
   Matrix<T> result(*this);
   return result *= value;
}

template <class T> 
Matrix<T> Matrix<T>::operator / (const T &value) const
{
   Matrix<T> result(*this);
   return result /= value;
}

template <class T> 
Matrix<T> operator + (const T &value,const Matrix<T> &m)
{ return m + value; }

template <class T> 
Matrix<T> operator - (const T &value,const Matrix<T> &m)
{ return -m + value; }

template <class T> 
Matrix<T> operator * (const T &value,const Matrix<T> &m)
{
 int i, j;
 Matrix<T> m1(m);
 for(i=0;i<m1.rows();i++)
  for(j=0;j<m1.cols();j++)
   m1[i][j] = value*m1[i][j];
 return m1;
}

template <class T> 
Matrix<T> operator / (const T &value,const Matrix<T> &m)
{
   Matrix<T> result(m.rows(),m.cols());
   for(int i=0;i<result.rows();i++) result[i] = value/m[i];
   return result;
}

// Vectorize operator
template <class T> Vector<T> Matrix<T>::vec() const
{
   int i=0, j, k, size = rowNum*colNum;
   Vector<T> result(size);
   for(j=0;j<colNum;j++)
      for(k=0;k<rowNum;k++) result[i++] = mat[k][j];
   return result;
}

template <class T> Vector<T> vec(const Matrix<T> &m)
{ return m.vec(); }

// Kronecker Product
template <class T> Matrix<T> Matrix<T>::kron(const Matrix<T> &m) const
{
   int size1 = rowNum*m.rowNum,
       size2 = colNum*m.colNum,
       i, j, k, p;
   Matrix<T> result(size1, size2);
   for(i=0;i<rowNum;i++)
      for(j=0;j<colNum;j++)
         for(k=0;k<m.rowNum;k++)
            for(p=0;p<m.colNum;p++)
               result[k+i*m.rowNum][p+j*m.colNum] 
                  = mat[i][j]*m.mat[k][p];
   return result;
}

template <class T> 
Matrix<T> kron(const Matrix<T> &s,const Matrix<T> &m)
{ return s.kron(m); }

// Direct Sum
template <class T> Matrix<T> Matrix<T>::dsum(const Matrix<T> &m) const
{
   int size1 = rowNum+m.rowNum,
       size2 = colNum+m.colNum;
   Matrix<T> result(size1, size2);
   for(int i=0;i<size1;i++)
      for(int j=0;j<size2;j++)
      {
         if(i < rowNum && j < colNum)
           result[i][j] = mat[i][j];
         else if(i >= rowNum && j >= colNum)
           result[i][j] = m.mat[i-rowNum][j-colNum];
         else
           result[i][j] = zero(T());
      }
   return result;
}

template <class T> 
Matrix<T> dsum(const Matrix<T> &s,const Matrix<T> &m)
{ return s.dsum(m); }

// Hadamard product
template <class T> Matrix<T> Matrix<T>::hadamard(const Matrix<T> &m) const
{
   assert(rowNum == m.rowNum && colNum == m.colNum);
   Matrix<T> result(rowNum, colNum, zero(T()));
   for(int i=0;i<rowNum;i++)
      for(int j=0;j<m.colNum;j++)
         result[i][j] = mat[i][j]*m[i][j];
   return result;
}

template <class T> 
Matrix<T> hadamard(const Matrix<T> &s,const Matrix<T> &m)
{ return s.hadamard(m); }

template <class T>
pair<Matrix<T>, Matrix<T> > Matrix<T>::LU() const
{
 assert(rowNum == colNum);
 Matrix<T> L(rowNum,colNum,zero(T()));
 Matrix<T> U(*this);
 for(int i=0;i<rowNum;i++)
 {
  assert(U[i][i] != zero(T()));
  L[i][i] = U[i][i];
  U[i] /= L[i][i];
  U[i][i] = one(T());
  for(int j=i+1;j<colNum;j++)
  {
   L[j][i] = U[j][i];
   U[j] -= L[j][i]*U[i];
   U[j][i] = zero(T());
  }
 }
 return make_pair(L, U);
}

template <class T>
pair<Matrix<T>, Matrix<T> > LU(const Matrix<T> &m)
{ return m.LU(); }

template <class T> 
int operator == (const Matrix<T> &m1,const Matrix<T> &m2)
{
   if(m1.rows() != m2.rows()) return 0;
   for(int i=0;i<m1.rows();i++)
      if(m1[i] != m2[i]) return 0;
   return 1;
}

template <class T> 
int operator != (const Matrix<T> &m1,const Matrix<T> &m2)
{ return !(m1==m2); }

template <class T> ostream & Matrix<T>::output(ostream &s) const
{
   int t = colNum-1, maxwidth=0, i, j, k, l;
   vector<string> m(rowNum*colNum);
   for(i=0,k=0;i<rowNum;i++)
   {
      for(j=0;j<colNum;j++,k++)
      {
       // strore the string representation for each
       // element so that we can compute the maximum
       // string length and then center each element
       // in its column
       ostringstream os;
       os << mat[i][j];
       m[k] = os.str();
       if(maxwidth < (int)m[k].length()) maxwidth = m[k].length();
      }
   }
   for(i=0,k=0;i<rowNum;i++)
   {
      s << "[";
      for(j=0;j<t;j++,k++)
      {
       // add spaces around the string to center it
       l = maxwidth-m[k].length();
       if(l%2) m[k] = " " + m[k];
       for(l=l/2;l>0;l--) m[k] = " " + m[k] + " ";
       // output the centered string
       s << m[k] << " ";
      }
      // add spaces around the string to center it
      l = maxwidth-m[k].length();
      if(l%2) m[k] = " " + m[k];
      for(l=l/2;l>0;l--) m[k] = " " + m[k] + " ";
      // output the centered string
      s << m[k++] << "]" << endl;
   }
   return s;
}

template <class T> ostream & operator << (ostream &s,const Matrix<T> &m)
{ return m.output(s); }

template <class T> istream & Matrix<T>::input(istream &s)
{
   int i, j, num1, num2;
   s.clear();                 // set stream state to good
   s >> num1;                 // read in row number
   if(! s.good()) return s;   // can't get an integer, just return
   s >> num2;                 // read in column number
   if(! s.good()) return s;   // can't get an integer, just return
   resize(num1,num2);         // resize to Matrix into right order
   for(i=0;i<num1;i++)
      for(j=0;j<num2;j++) 
      {
         s >> mat[i][j];
         if(! s.good())
         {
            s.clear(s.rdstate() | ios::badbit);
            return s;
         }
      }
   return s;
}

template <class T> istream & operator >> (istream &s,Matrix<T> &m)
{ return m.input(s); }
#endif
