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


// multinomial.h

#ifndef _MULTINOMIAL
#define _MULTINOMIAL

#include <cassert>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <utility>  // for pair
#include <vector>
#include "identity.h"
using namespace std;

// Assumption : T has typecasts defined and can act as a numeric data type

// Multinomial class

template <class T>
class Multinomial
{
 public:
   Multinomial(): type(number),n(zero(T())) {}
   Multinomial(T);
   Multinomial(string x);
   Multinomial(const Multinomial<T> &p):
   variable(p.variable),type(p.type),n(p.n),u(p.u),m(p.m) {}

   Multinomial<T>& operator=(const T&);
   Multinomial<T>& operator=(const Multinomial<T>&);

   Multinomial<T> operator+() const;
   Multinomial<T> operator-() const;

   Multinomial<T> operator+(const Multinomial<T>&) const;
   Multinomial<T> operator-(const Multinomial<T>&) const;
   Multinomial<T> operator*(const Multinomial<T>&) const;
   Multinomial<T> operator+(const T&) const;
   Multinomial<T> operator-(const T&) const;
   Multinomial<T> operator*(const T&) const;

   Multinomial<T> operator^(unsigned int) const;

   Multinomial<T>& operator+=(const Multinomial<T>&);
   Multinomial<T>& operator-=(const Multinomial<T>&);
   Multinomial<T>& operator*=(const Multinomial<T>&);
   Multinomial<T>& operator+=(const T&);
   Multinomial<T>& operator-=(const T&);
   Multinomial<T>& operator*=(const T&);

   int operator==(const Multinomial<T>&) const;
   int operator!=(const Multinomial<T>&) const;
   int operator==(const T&) const;
   int operator!=(const T&) const;

   Multinomial<T> Diff(const string &) const;
   ostream &output(ostream &) const;

 protected:
   void remove_zeros(void);
   pair<Multinomial<T>,Multinomial<T> >
    reconcile(const Multinomial<T>&,const Multinomial<T>&) const;
   vector<string> toarray(void) const;
   string variable;
   enum { number, univariate, multivariate } type;
   T n;                               //number
   list<pair<T,int> > u;              //univariate
   list<pair<Multinomial<T>,int> > m; //multivariate
};


// additional functions that are not members of the class
template <class T>
Multinomial<T> operator+(const T&,const Multinomial<T>&);
template <class T>
Multinomial<T> operator-(const T&,const Multinomial<T>&);
template <class T>
Multinomial<T> operator*(const T&,const Multinomial<T>&);
template <class T>
int operator==(T,const Multinomial<T>&);
template <class T>
int operator!=(T,const Multinomial<T>&);


// implementation

template <class T>
Multinomial<T>::Multinomial(T c) :
variable(""), type(number), n(c) {}

template <class T>
Multinomial<T>::Multinomial(string x): variable(x),type(univariate)
{u.push_back(pair<T,int>(one(T()),1));}

template <class T>
Multinomial<T>& Multinomial<T>::operator=(const T &c)
{return *this = Multinomial<T>(c);}

template <class T>
Multinomial<T>& Multinomial<T>::operator=(const Multinomial<T> &p)
{
 if(this == &p) return *this;
 variable = p.variable;
 type = p.type; u = p.u; m = p.m;
 return *this;
}

template <class T>
Multinomial<T> Multinomial<T>::operator+() const
{return *this;}

template <class T>
Multinomial<T> Multinomial<T>::operator-() const
{
 Multinomial<T> p2(*this);
 typename list<pair<T,int> >::iterator i = p2.u.begin();
 typename list<pair<T,int> >::iterator j = p2.m.begin();
 for(;i!= p2.u.end();i++) i->first = -(i->first);
 for(;j!= p2.v.end();j++) j->first = -(j->first);
 return p2;
}

template <class T>
list <pair<T,int> >
merge(const list <pair<T,int> > &l1,const list <pair<T,int> > &l2)
{
 list <pair<T,int> > l;
 typename list<pair<T,int> >::const_iterator i = l1.begin();
 typename list<pair<T,int> >::const_iterator j = l2.begin();
 while(i!=l1.end() && j!=l2.end())
 {
  if     (i->second<j->second) l.push_back(*(j++));
  else if(i->second>j->second) l.push_back(*(i++));
  else // i->second==j->second
  {
   l.push_back(make_pair(i->first + j->first,i->second));
   i++; j++;
  }
 }
 while(i!=l1.end()) l.push_back(*(i++));
 while(j!=l2.end()) l.push_back(*(j++));
 return l;
}

template <class T>
Multinomial<T> 
Multinomial<T>::operator+(const Multinomial<T> &p) const
{
 Multinomial<T> p2;
 pair<Multinomial<T>,Multinomial<T> > xy = reconcile(*this,p);
 
 switch(xy.first.type)
 {
  case number:       p2.type = number; p2.n = xy.first.n + xy.second.n;
                     break;
  case univariate:   p2.type = univariate;
                     p2.variable = xy.first.variable;
                     p2.u = merge(xy.first.u,xy.second.u);
                     break;
  case multivariate: p2.type = multivariate;
                     p2.variable = xy.first.variable;
                     p2.m = merge(xy.first.m,xy.second.m);
                     break;
 }
 p2.remove_zeros();
 return p2;
}

template <class T>
Multinomial<T> 
Multinomial<T>::operator-(const Multinomial<T> &p) const
{ return (*this) + (-p); }

template <class T>
list <pair<T,int> >
distribute(const list <pair<T,int> > &l1,const list <pair<T,int> > &l2)
{
 list <pair<T,int> > l;
 typename list<pair<T,int> >::const_iterator i;
 typename list<pair<T,int> >::const_iterator j;

 for(i=l1.begin();i!=l1.end();i++)
 {
  list <pair<T,int> > t;
  for(j=l2.begin();j!=l2.end();j++)
   t.push_back(make_pair(i->first * j->first,i->second + j->second));
  l = merge(l,t);
 }
 return l;
}

template <class T>
Multinomial<T> Multinomial<T>::operator*(const Multinomial<T> &p) const
{
 Multinomial<T> p2;
 typename list<pair<T,int> >::const_iterator i, j;
 pair<Multinomial<T>,Multinomial<T> > xy = reconcile(*this,p);

 switch(xy.first.type)
 {
  case number:       p2.type = number; p2.n = xy.first.n * xy.second.n;
                     break;
  case univariate:   p2.type = univariate;
                     p2.variable = xy.first.variable;
                     p2.u = distribute(xy.first.u,xy.second.u);
                     break;
  case multivariate: p2.type = multivariate;
                     p2.variable = xy.first.variable;
                     p2.m = distribute(xy.first.m,xy.second.m);
                     break;
 }
 p2.remove_zeros();
 return p2;
}

template <class T>
Multinomial<T> Multinomial<T>::operator^(unsigned int n) const
{
 Multinomial<T> result(one(T())), factor(*this);
 while(n>0)
 {
  if(n%2 == 1) result *= factor;
  factor *= factor;
  n /= 2;
 }
 return result;
}

template <class T>
Multinomial<T> Multinomial<T>::operator+(const T &c) const
{return (*this) + Multinomial<T>(c);}

template <class T>
Multinomial<T> Multinomial<T>::operator-(const T &c) const
{return (*this) - Multinomial<T>(c);}

template <class T>
Multinomial<T> Multinomial<T>::operator*(const T &c) const
{return (*this) * Multinomial<T>(c);}

template <class T>
Multinomial<T>& Multinomial<T>::operator+=(const Multinomial<T> &p)
{return *this = *this + p;}

template <class T>
Multinomial<T>& Multinomial<T>::operator-=(const Multinomial<T> &p)
{return *this = *this - p;}

template <class T>
Multinomial<T>& Multinomial<T>::operator*=(const Multinomial<T> &p)
{return *this = *this * p;}

template <class T>
Multinomial<T> &Multinomial<T>::operator+=(const T &c)
{*this += Multinomial<T>(c);}

template <class T>
Multinomial<T> &Multinomial<T>::operator-=(const T &c)
{*this -= Multinomial<T>(c);}

template <class T>
Multinomial<T> &Multinomial<T>::operator*=(const T &c)
{*this *= Multinomial<T>(c);}

template <class T>
T Diff(const T &t,const string &x)
{return zero(T());}

// partial template specialization for polynomials
template <class T>
Multinomial<T> Diff(const Multinomial<T>&p,const string &x)
{return p.Diff(x);}

template <class T>
Multinomial<T> Multinomial<T>::Diff(const string &x) const
{
 Multinomial<T> p2;
 typename list<pair<T,int> >::const_iterator i = u.begin();
 typename list<pair<Multinomial<T>,int> >::const_iterator j = m.begin();

 if(type==number) return Multinomial<T>(zero(T()));
 if(type==univariate)
 {
  if(variable!=x) return Multinomial<T>(zero(T()));
  p2.variable = variable;
  p2.type = univariate;
  for(;i!=u.end();i++)
    p2.u.push_back(make_pair(i->first * T(i->second),i->second-1));
 }
 if(type == multivariate)
 {
  p2.variable = variable;
  p2.type = multivariate;
  if(variable==x)
   for(;j!=m.end();j++)
     p2.m.push_back(make_pair(j->first * T(j->second),j->second-1));
  else
   for(;j!=m.end();j++)
     p2.m.push_back(make_pair(j->first.Diff(x),j->second));
 }
 p2.remove_zeros();
 return p2;
}

template <class T>
int Multinomial<T>::operator==(const Multinomial<T> &p) const
{
 pair<Multinomial<T>,Multinomial<T> > xy = reconcile(*this,p);

 xy.first.remove_zeros();
 xy.second.remove_zeros();
 switch(xy.first.type)
 {
  case number:       return xy.first.n == xy.second.n;
                     break;
  case univariate:   return xy.first.u == xy.second.u;
                     break;
  case multivariate: return xy.first.m == xy.second.m;
                     break;
 }
 return 0;
}

template <class T>
int Multinomial<T>::operator!=(const Multinomial<T> &p) const
{return !(*this == p);}

template <class T>
int Multinomial<T>::operator==(const T &c) const
{return *this == Multinomial<T>(c);}

template <class T>
int Multinomial<T>::operator!=(const T &c) const
{return !(*this == c);}

template <class T>
vector<string>
Multinomial<T>::toarray(void) const
{
 ostringstream o;
 vector<string> v;
 typename list<pair<T,int> >::const_iterator i = u.begin();
 typename list<pair<Multinomial<T>,int> >::const_iterator j = m.begin();

 switch(type)
 {
  case number:       o << n; v.push_back(o.str()); break;
  case univariate:   if(i==u.end()) v.push_back("0");
                     while(i!=u.end())
                     {
                      if(i->first!=one(T()) || i->second==0)
                       o << "(" << i->first << ")";
                      if(i->second>0) o << variable;
                      if(i->second>1) o << "^" << i->second;
                      v.push_back(o.str());
                      o.str(""); i++;
                     }
                     break;
  case multivariate: if(j==m.end()) v.push_back("0");
                     while(j!=m.end())
                     {
                      if(j->second>0) o << variable;
                      if(j->second>1) o << "^" << j->second;
                      if(j->first!=one(T()) || j->second==0)
                      {
                       vector<string> v1 = j->first.toarray();
                       for(int k=0;k<int(v1.size());k++)
                        v.push_back(v1[k] + o.str());
                      }
                      o.str(""); j++;
                     }
                     break;
 }
 return v;
}

template <class T>
ostream &Multinomial<T>::output(ostream &o) const
{
 int k;
 typename list<pair<T,int> >::const_iterator i = u.begin();

 switch(type)
 {
  case number:       o << n; break;
  case univariate:   if(i==u.end()) o << "0";
                     while(i!=u.end())
                     {
                      if(i->first!=one(T()) || i->second==0)
                       o << "(" << i->first << ")";
                      if(i->second>0) o << variable;
                      if(i->second>1) o << "^" << i->second;
                      if(!(++i==u.end())) o << " + ";
                     }
                     break;
  case multivariate: vector<string> v = toarray();
                     for(k=0;k<int(v.size())-1;k++)
                      o << v[k] << " + ";
                     if(k<int(v.size())) o << v[k];
                     else o << "0";
                     break;
 }
 return o;
}

template <class T>
ostream &operator<<(ostream &o,const Multinomial<T> &p)
{return p.output(o);}

template <class T>
void Multinomial<T>::remove_zeros(void)
{
 {
  typename list<pair<T,int> >::iterator i, j;
  for(i=j=u.begin();i!=u.end();)
  {
   j++;
   if(i->first==zero(T())) u.erase(i);
   i=j;
  }
 }
 {
  typename list<pair<Multinomial<T>,int> >::iterator i, j;
  for(i=j=m.begin();i!=m.end();)
  {
   j++;
   i->first.remove_zeros();
   if(i->first==zero(T())) m.erase(i);
   i=j;
  }
 }
}

template <class T>
pair<Multinomial<T>,Multinomial<T> >
Multinomial<T>::reconcile(const Multinomial<T> &x,
                          const Multinomial<T> &y) const
{
 if(x.type==number && y.type==number) return make_pair(x,y);
 if(x.type==number && y.type==univariate)
 {
  if(y.u.empty()) return make_pair(x,Multinomial<T>(zero(T())));
  Multinomial<T> t(y.variable);
  t.u.clear();
  t.u.push_back(make_pair(x.n,0));
  return make_pair(t,y);
 }
 if(x.type==number && y.type==multivariate)
 {
  if(y.m.empty()) return make_pair(x,Multinomial<T>(zero(T())));
  Multinomial<T> t(y.variable);
  t.u.clear();
  t.u.push_back(make_pair(x.n,0));
  return reconcile(t,y);
 }
 if(x.type==univariate && y.type==univariate)
 {
  if(x.variable==y.variable) return make_pair(x,y);
  if(x.variable< y.variable)
  {
   Multinomial<T> t1(y.variable);
   Multinomial<T> t2(y.variable);
   t1.type = t2.type = multivariate;
   t1.u.clear();
   t1.m.push_back(make_pair(x,0));
   typename list<pair<T,int> >::const_iterator i = y.u.begin();
   for(;i!=y.u.end();i++)
   {
    Multinomial<T> t3(x.variable);
    t3.u.clear();
    t3.u.push_back(make_pair(i->first,0));
    t2.m.push_back(make_pair(t3,i->second));
   }
   return make_pair(t1,t2);
  }
  else 
  {
   Multinomial<T> t1(x.variable);
   Multinomial<T> t2(x.variable);
   t1.type = t2.type = multivariate;
   t2.u.clear();
   t2.m.push_back(make_pair(y,0));
   typename list<pair<T,int> >::const_iterator i = x.u.begin();
   for(;i!=x.u.end();i++)
   {
    Multinomial<T> t3(y.variable);
    t3.u.clear();
    t3.u.push_back(make_pair(i->first,0));
    t1.m.push_back(make_pair(t3,i->second));
   }
   return make_pair(t1,t2);
  }
 }
 if(x.type==univariate && y.type==multivariate)
 {
  if(y.m.empty())
  {
   Multinomial<T> t(x.variable);
   t.u.clear();
   return make_pair(x,t);
  }
  if(x.variable==y.variable)
   return make_pair(reconcile(x,y.m.front().first).first,y);
  if(x.variable< y.variable)
  {
   Multinomial<T> t(y.variable);
   t.type = multivariate;
   t.u.clear();
   t.m.push_back(make_pair(x,0));
   return make_pair(t,y);
  }
  else
  {
   Multinomial<T> t(x.variable);
   t.type = multivariate;
   t.u.clear();
   typename list<pair<T,int> >::const_iterator i = x.u.begin();
   for(;i!=x.u.end();i++)
   {
    Multinomial<T> t3(y.m.front().first.variable);
    t3.u.clear();
    t3.u.push_back(make_pair(i->first,0));
    t.m.push_back(make_pair(t3,i->second));
   }
   return reconcile(t,y);
  }
 }
 if(x.type==multivariate && y.type==multivariate)
 {
  if(x.variable==y.variable) return make_pair(x,y);
  if(x.variable< y.variable)
  {
   Multinomial<T> t(y.variable);
   t.type = multivariate;
   t.u.clear();
   t.m.push_back(make_pair(x,0));
   return make_pair(t,y);
  }
  else 
  {
   Multinomial<T> t(x.variable);
   t.type = multivariate;
   t.u.clear();
   t.m.push_back(make_pair(y,0));
   return make_pair(x,t);
  }
 }

 pair<Multinomial<T>,Multinomial<T> > p;
 p = reconcile(y,x);
 return make_pair(p.second,p.first);
}

// additional functions that are not members of the class
template <class T>
Multinomial<T> operator+(const T &c,const Multinomial<T> &p)
{return Multinomial<T>(c) + p;}

template <class T>
Multinomial<T> operator-(const T &c,const Multinomial<T> &p)
{return Multinomial<T>(c) - p;}

template <class T>
Multinomial<T> operator*(const T &c,const Multinomial<T> &p)
{return Multinomial<T>(c) * p;}

template <class T>
int operator==(const T &c,const Multinomial<T> &p) {return p == c;}

template <class T>
int operator!=(const T &c,const Multinomial<T> &p) {return p != c;}

template <class T>
Multinomial<T> zero(Multinomial<T>) {return Multinomial<T>(zero(T()));}

template <class T>
Multinomial<T> one(Multinomial<T>) {return Multinomial<T>(one(T()));}

#endif
