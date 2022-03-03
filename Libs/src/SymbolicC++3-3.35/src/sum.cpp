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
#ifndef SYMBOLIC_CPLUSPLUS_SUM_DEFINE
#define SYMBOLIC_CPLUSPLUS_SUM_DEFINE
#define SYMBOLIC_CPLUSPLUS_SUM

Sum::Sum() {}

Sum::Sum(const Sum &s)
 : CloningSymbolicInterface(s), summands(s.summands) {}

Sum::Sum(const Symbolic &s1,const Symbolic &s2)
{
 if(s1.type() == typeid(Sum)) summands = CastPtr<const Sum>(s1)->summands;
 else summands.push_back(s1);
 if(s2.type() == typeid(Sum))
 {
  CastPtr<const Sum> p(s2);
  summands.insert(summands.end(),p->summands.begin(),p->summands.end());
 }
 else summands.push_back(s2);
}

Sum::~Sum() {}

Sum &Sum::operator=(const Sum &s)
{
 if(this != &s) summands = s.summands;
 return *this;
}

void Sum::print(ostream &o) const
{
 if(summands.empty()) o << 0;
 for(list<Symbolic>::const_iterator i=summands.begin();i!=summands.end();
     ++i)
 {
  if((i->type() != typeid(Numeric)
      || !CastPtr<const Numeric>(*i)->isNegative()) &&
     (i->type() != typeid(Product)
      || !CastPtr<const Product>(*i)->printsNegative()))
  o << ((i==summands.begin()) ? "":"+");
  i->print(o);
 }
}

Symbolic Sum::subst(const Symbolic &x,const Symbolic &y,int &n) const
{
 if(x.type() == type())
 {
  list<Symbolic>::const_iterator i;
  list<Symbolic>::iterator j;
  // make a copy of *this
  CastPtr<Sum> s1(*this);
  CastPtr<const Sum> s2(x);
  for(i=s2->summands.begin();i!=s2->summands.end();++i)
  {
   for(j=s1->summands.begin();j!=s1->summands.end() && *i != *j;++j);
   if(j == s1->summands.end()) break;
   s1->summands.erase(j);
  }
  if(i == s2->summands.end())
  {
   ++n;
   // reset the simplified and expanded flags
   // since substitution may have changed this
   s1->simplified = s1->expanded = 0;
   return s1->subst(x,y,n) + y;
  }
 }
  
 // sum does not contain expression for substitution
 // try to substitute in each summand
 Sum s;
 for(list<Symbolic>::const_iterator i=summands.begin();i!=summands.end();
     ++i)
  s.summands.push_back(i->subst(x,y,n));
 return s;
}

Simplified Sum::simplify() const
{
 list<Symbolic>::const_iterator i;
 list<Symbolic>::iterator j, k;
 Sum r;

 // 1-element sum:  (a) -> a
 if(summands.size() == 1) return summands.front().simplify();

 // absorb sum of sums: a + (a + a) + a -> a + a + a + a
 for(i=summands.begin();i!=summands.end();++i)
 {
  Simplified s = i->simplify();
  if(s.type() == typeid(Sum))
  {
   CastPtr<const Sum> sum(s);
   r.summands.insert(r.summands.end(),sum->summands.begin(),
                     sum->summands.end());
  }
  else r.summands.push_back(s);
 }

 // collect matrices
 int firstm = 1;
 SymbolicMatrix m(1,1);
 for(j=r.summands.begin();j!=r.summands.end();)
 {
  if(j->type() == typeid(SymbolicMatrix))
  {
   if(firstm)
   {
    m = *CastPtr<const SymbolicMatrix>(*j);
    firstm = 1 - firstm;
   }
   else 
    m = m + *CastPtr<const SymbolicMatrix>(*j);
   j = r.summands.erase(j);
  }
  else ++j;
 }
 if(!firstm) r.summands.push_back(m.simplify());

 // group common terms
 for(j=r.summands.begin();j!=r.summands.end();)
 {
  // numbers will be grouped later
  if(j->type() == typeid(Numeric)) { ++j; continue; }

  Number<void> n = Number<int>(1);
  Symbolic j1 = *j;

  // the leading coefficient of products must be ignored in grouping comparisons
  if(j1.type() == typeid(Product))
  {
   CastPtr<Product> j2(j1);
   if(!j2->factors.empty() && j2->factors.front().type() == typeid(Numeric))
   {
    n = Number<void>(j2->factors.front());
    j2->factors.pop_front();
    j1 = j2->simplify();
   }
  }

  for(++(k=j);k!=r.summands.end();)
  {
   // numbers will be grouped later
   if(k->type() == typeid(Numeric)) { ++k; continue; }

   Symbolic k1 = *k;
   Number<void> coeff = Number<int>(1);
   // the leading coefficient of products must be ignored
   // in grouping comparisons

   if(k1.type() == typeid(Product))
   {
    // make a copy of k1
    CastPtr<Product> k2(*k1);
    if(!k2->factors.empty() && k2->factors.front().type() == typeid(Numeric))
    {
     coeff = Number<void>(k2->factors.front());
     k2->factors.pop_front();
     k1 = *k2;
    }
   }

   if(j1 == k1)
   {
    n = n + coeff;
    k = r.summands.erase(k);
   }
   else ++k;
  }
  if(n.isZero()) j = r.summands.erase(j);
  else *(j++) = (Symbolic(n) * j1).simplify();
 }

 // move numbers to the back
 Number<void> n = Number<int>(0);
 for(j=r.summands.begin();j!=r.summands.end();)
 {
  if(j->type() == typeid(Numeric))
  {
   n = n + Number<void>(*j);
   j = r.summands.erase(j);
  }
  else ++j;
 }
 
 if(!n.isZero()) r.summands.push_back(n->simplify());
 if(r.summands.size()==0) return Number<int>(0);
 if(r.summands.size()==1) return r.summands.front();

 return r;
}

int Sum::compare(const Symbolic &s) const
{
 if(type() != s.type()) return 0;
 // make a copy of s
 CastPtr<Sum> p(*s);

 list<Symbolic>::const_iterator i;
 list<Symbolic>::iterator j;

 if(summands.size() != p->summands.size()) return 0;
 for(i=summands.begin();i!=summands.end();++i)
 {
  for(j=p->summands.begin();j!=p->summands.end() && *i != *j;++j);
  if(j == p->summands.end()) return 0;
  p->summands.erase(j);
 }
  
 return 1;
}

Symbolic Sum::df(const Symbolic &s) const
{
 list<Symbolic>::const_iterator i;
 Sum r;
 for(i=summands.begin();i!=summands.end();++i)
  r.summands.push_back(i->df(s));
 return r;
}

Symbolic Sum::integrate(const Symbolic &s) const
{
 list<Symbolic>::const_iterator i;
 Sum r;
 for(i=summands.begin();i!=summands.end();++i)
  r.summands.push_back(::integrate(*i,s));
 return r;
}

Symbolic Sum::coeff(const Symbolic &s) const
{
 list<Symbolic>::const_iterator i;
 Sum r;
 for(i=summands.begin();i!=summands.end();++i)
  r.summands.push_back(i->coeff(s));
 return r;
}

Expanded Sum::expand() const
{
 list<Symbolic>::const_iterator i;
 Sum r;
 for(i=summands.begin();i!=summands.end();++i)
  r.summands.push_back(i->expand());
 return r;
}

int Sum::commute(const Symbolic &s) const
{
 list<Symbolic>::const_iterator i;

 // Optimize the case for numbers
 if(s.type() == typeid(Numeric)) return 1;

 // Optimize the case for a single symbol
 if(s.type() == typeid(Symbol))
 {
  list<Symbolic>::const_iterator i;
  for(i=summands.begin();i!=summands.end();++i)
   if(!i->commute(s)) return 0;
  return 1;
 }

 // if every term in the sum commutes with s
 // then the sum commutes with s
 for(i=summands.begin();i!=summands.end();++i)
  if(!i->commute(s)) break;
 if(i == summands.end()) return 1;

 // calculate the commutator [A, B] = A*B - B*A
 Expanded p1 = Product(*this,s).expand();
 Expanded p2 = Product(-s,*this).expand();
 Symbolic sum = Sum(p1,p2);
 // [A, B] == 0 implies A*B = B*A
 return sum == 0;
}

PatternMatches Sum::match(const Symbolic &s, const list<Symbolic> &p) const
{
 PatternMatches l;
 list<Symbolic>::const_iterator i;
 list<list<int> >::iterator j;
 list<int>::iterator k;

 if(summands.size() == 0) return l;
 if(summands.size() == 1) return summands.front().match(s, p);
 if(s.type() != type()) return l;

 CastPtr<Sum> sum(s);
 list<list<int> > thismatch;
 Sum rest(*this);
 rest.summands.pop_front();
 thismatch.push_back(list<int>()); // start with an empty sum

 for(i=sum->summands.begin();i!=sum->summands.end();++i)
 {
  for(j=thismatch.begin();j!=thismatch.end();++j)
  {
   list<int> s(*j); s.push_back(0); j->push_back(1);
   thismatch.insert(j, s);
  }
 }

 for(j=thismatch.begin();j!=thismatch.end();++j)
 {
  Sum s1, s2;
  for(k=j->begin(), i=sum->summands.begin();
      k!=j->end() && i!=sum->summands.end(); ++i, ++k)
   if(*k) s1.summands.push_back(*i); else s2.summands.push_back(*i);
  if(s1.summands.size() != 0 && s2.summands.size() != 0)
  {
   PatternMatches l1 = summands.front().match(s1, p);
   PatternMatches l2 = rest.match(s2, p);
   pattern_match_AND(l1, l2);
   pattern_match_OR(l, l1);
  }
 }

 return l;
}

PatternMatches
Sum::match_parts(const Symbolic &s, const list<Symbolic> &p) const
{
 PatternMatches l = s.match(*this, p);
 list<Symbolic>::const_iterator i;
 list<Sum>::iterator j;

 list<Sum> matchpart;
 matchpart.push_back(Sum()); // start with an empty sum

 for(i=summands.begin();i!=summands.end();++i)
 {
  PatternMatches lp = i->match_parts(s, p);
  pattern_match_OR(l, lp);
  for(j=matchpart.begin();j!=matchpart.end();++j)
  {
   Sum s(*j); s.summands.push_back(*i);
   if(s.summands.size() != 0 && s.summands.size() != summands.size())
    matchpart.insert(j, s);
  }
 }

 // remove empty sum
 for(j=matchpart.begin();j!=matchpart.end();++j)
  if(j->summands.size() == 0) break;
 if(j != matchpart.end()); matchpart.erase(j);

 for(j=matchpart.begin();j!=matchpart.end();++j)
 {
  PatternMatches lp = s.match(*j, p);
  pattern_match_OR(l, lp);
 }

 return l;
}

#endif
#endif

