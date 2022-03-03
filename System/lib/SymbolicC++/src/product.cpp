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
#ifndef SYMBOLIC_CPLUSPLUS_PRODUCT_DEFINE
#define SYMBOLIC_CPLUSPLUS_PRODUCT_DEFINE
#define SYMBOLIC_CPLUSPLUS_PRODUCT

Product::Product() {}

Product::Product(const Product &s)
 : CloningSymbolicInterface(s), factors(s.factors) {}

Product::Product(const Symbolic &s1,const Symbolic &s2)
{
 if(s1.type() == typeid(Product))
  factors = CastPtr<const Product>(s1)->factors;
 else factors.push_back(s1);
 if(s2.type() == typeid(Product))
 {
  CastPtr<const Product> p(s2);
  factors.insert(factors.end(),p->factors.begin(),p->factors.end());
 }
 else factors.push_back(s2);
}

Product::~Product() {}

Product &Product::operator=(const Product &p)
{
 if(this != &p) factors = p.factors;
 return *this;
}

int Product::printsNegative() const
{
 return factors.size() > 1                        &&
        factors.front().type() == typeid(Numeric) &&
        CastPtr<const Numeric>(factors.front())->isNegative();
}

void Product::print(ostream &o) const
{
 if(factors.empty()) o << 1;
 if(factors.size() == 1) factors.begin()->print(o);
 else
  for(list<Symbolic>::const_iterator i=factors.begin();i!=factors.end();++i)
  {
   o << ((i==factors.begin()) ? "":"*");
   if(*i == -1) { o << "-"; ++i; }
   if(i->type() != typeid(Product)    &&
      i->type() != typeid(Power)      &&
      i->type() != typeid(Sin)        &&
      i->type() != typeid(Cos)        &&
      i->type() != typeid(Log)        &&
      i->type() != typeid(Derivative) &&
      i->type() != typeid(Symbol)     &&
      i->type() != typeid(Numeric)) 
    { o << "("; i->print(o); o << ")"; }
   else i->print(o);
  }
}

Symbolic Product::subst(const Symbolic &x,const Symbolic &y,int &n) const
{
#if 0
/*
 if(x.type() == type())
 {
  int i;
  CastPtr<const Product> p(x);
  list<Symbolic>::const_iterator pi, lastpi, j, k;
  vector<list<Symbolic>::const_iterator> v(p->factors.size());
  vector<list<Symbolic>::const_iterator>::const_iterator vi, vj;
  i = 0; v[0] = factors.begin(); pi = p->factors.begin();
  while(i >= 0)
  {
    cout << "-- " << i << " / " << p->factors.size() << endl;
   while(v[i] != factors.end() && pi != p->factors.end())
    if(pi->compare(*v[i]))
    {
     // found a match for the factor, try match the next factor
     lastpi = pi; ++pi;
     v[++i] = factors.begin(); 
    }
    else ++v[i];

    cout << "!! " << i << " / " << p->factors.size() << endl;
   // found a match for every factor
   if(pi == p->factors.end())
   {
    cout << "** " << i << endl;
    for(vi=v.begin(); vi!=v.end(); ++vi)
    {
     // don't use any factor more than once
     for(vj=v.begin(); vj!=vi; ++vj)
      if(*vj == *vi) break;
     if(vj != vi) break;
     // check that the factors can be rearranged in the right order
     for(++vj; vj!=v.end(); ++vj)
      for(j=factors.begin(); j!=*vi; ++j)
       if(j == *vj && !(*vi)->commute(**vj)) break;
     if(vj != v.end()) break;
    }
    if(vi == v.end())
    {
     // check substitution
     for(j=factors.begin(); j!=factors.end(); ++j)
      if(find(v.begin(), v.end(), j) != v.end())
      {
       for(k=factors.begin(); k!=j; ++k)
        if(find(v.begin(), v.end(), k) == v.end() && !j->commute(*k)) break;
       if(k == j)
        for(++k; k!=factors.end(); ++k)
         if(find(v.begin(), v.end(), k) == v.end() && !j->commute(*k)) break;
       if(k == factors.end()) break;
      }
      else
      {
       for(k=factors.begin(); k!=factors.end(); ++k)
        if(find(v.begin(), v.end(), k) != v.end() && !j->commute(*k)) break;
       if(k == j)
        for(++k; k!=factors.end(); ++k)
         if(find(v.begin(), v.end(), k) != v.end() && !j->commute(*k)) break;
       if(k == factors.end()) break;
      }
     if(j != factors.end())
     {
      Product res;
      for(k=factors.begin(); k!=factors.end(); ++k)
      {
       if(k == j) res.factors.push_back(y);
       if(find(v.begin(), v.end(), k) == v.end()) res.factors.push_back(*k);
       ++n;
       return res.subst(x,y,n);
      }
     }
    }
   }

   // return to the search for the previous factor
   while(i >= 0 && v[i] == factors.end())
   {
    --i;
    if(pi == p->factors.end()) pi = lastpi; else --pi;
   }
   if(i >= 0) ++v[i];
   cout << "** " << i << " / " << p->factors.size() << endl;
  }
  cout << "done" << endl;
 }
*/
#else
 if(x.type() == type())
 {
  CastPtr<const Product> p(x);
  // vector<T>::iterator has ordering comparisons
  // while list<T>::iterator does not
  list<Symbolic> u;
  vector<Symbolic> v;
  list<Symbolic>::const_iterator i;
  list<Symbolic>::const_iterator i1;
  vector<Symbolic>::iterator j, insert;
  list< vector<Symbolic>::iterator >::iterator k;
  // we store lists of locations (iterators) in v in the list l,
  // each list in l should describe a path of unique locations through v
  list<list< vector<Symbolic>::iterator > > l;
  list<list< vector<Symbolic>::iterator > >::iterator li;

  // expand positive integer powers to match each factor individually
  for(i=p->factors.begin();i!=p->factors.end();++i)
  {
   if(i->type() == typeid(Power))
   {
    CastPtr<const Power> p(*i);
    if(p->parameters.back().type() == typeid(Numeric) &&
       Number<void>(p->parameters.back()).numerictype() == typeid(int))
    {
     int n = CastPtr<const Number<int> >(p->parameters.back())->n;
     if(n>0)
     {
      if(p->parameters.front().type() == typeid(Product))
      {
       CastPtr<Product> pwp(p->parameters.front());
       for(int m=0;m<n;++m)
        u.insert(u.end(), pwp->factors.begin(), pwp->factors.end());
      }
      else
       for(int m=0;m<n;++m) u.push_back(p->parameters.front());
     }
     else u.push_back(*i);
    }
    else u.push_back(*i);
   }
   else u.push_back(*i);
  }

  // expand positive integer powers to match each factor individually
  for(i1=factors.begin();i1!=factors.end();++i1)
  {
   if(i1->type() == typeid(Power))
   {
    CastPtr<const Power> p(*i1);
    if(p->parameters.back().type() == typeid(Numeric) &&
       Number<void>(p->parameters.back()).numerictype() == typeid(int))
    {
     int n = CastPtr<const Number<int> >(p->parameters.back())->n;
     if(n>0)
     {
      if(p->parameters.front().type() == typeid(Product))
      {
       CastPtr<Product> pwp(p->parameters.front());
       for(int m=0;m<n;++m)
        v.insert(v.end(), pwp->factors.begin(), pwp->factors.end());
      }
      else
       for(int m=0;m<n;++m) v.push_back(p->parameters.front());
     }
     else v.push_back(*i1);
    }
    else v.push_back(*i1);
   }
   else v.push_back(*i1);
  }

  i = u.begin();
  
  // initialize each path in l to begin with the location
  // of the first factor of x (copied in p)
  // there could be none, one or many such paths
  for(j=v.begin();j!=v.end();++j)
   if(*j == *i) 
   {
    l.push_back(list< vector<Symbolic>::iterator >());
    l.back().push_back(j);
   }

  // build each path by considering all possible
  // locations of subsequent factors of x and creating
  // all possible paths
  for(++i;i!=u.end();++i)
  {
   list<list< vector<Symbolic>::iterator > > l2;
   for(j=v.begin();j!=v.end();++j)
    if(*j == *i) 
     for(li=l.begin();li!=l.end();++li)
      if(find(li->begin(),li->end(),j) == li->end())
      {
       l2.push_back(*li);
       l2.back().push_back(j);
      }
   l = l2;
  }

  // erase paths that are too short
  for(li=l.begin();li!=l.end();)
   if(li->size() != u.size()) li = l.erase(li);
   else ++li;

  // search for a path which may be substituted
  for(li=l.begin();li!=l.end();++li)
  {
   for(k=li->begin();k!=li->end();++k)
   {
    list< vector<Symbolic>::iterator >::iterator k1 = k;
    // when consecutive values are in the wrong order
    // check that they commute
    for(++(k1=k);k1!=li->end();++k1)
     if(*k1 < *k && !(*k)->commute(**k1)) break;
    if(k1!=li->end()) break;
   }
   // values cannot commute to the right order
   if(k != li->end()) continue;

   // try to find a position that all values
   // can commute to, and set that as the place for substitution
   for(insert = v.begin();insert!=v.end();++insert)
   {
    for(k=li->begin();k!=li->end();++k)
    {
     vector<Symbolic>::iterator beg, end;
     if(insert <= *k) { beg = insert; end = *k; }
     else { beg = *k + 1; end = insert; }
     for(j=beg;j!=end;++j)
      if(find(li->begin(),li->end(),j) == li->end() &&
         !j->commute(**k)) break;
     if(j != end) break;
    }
    if(k == li->end()) break;
   }
   if(insert != v.end()) break;
  }

  // found a match
  if(li != l.end() && insert != v.end())
  {
   Product resultl, resultr;

   // if the term did not play a role in the substitution just copy it
   for(j=v.begin();j!=insert;++j)
    if(find(li->begin(),li->end(),j) == li->end())
     if(j->commute(x)) resultr.factors.push_back(*j);
     else              resultl.factors.push_back(*j);

   // perform the substitution
   resultl.factors.push_back(y);
   ++n;

   for(;j!=v.end();++j)
    if(find(li->begin(),li->end(),j) == li->end())
     resultr.factors.push_back(*j);

   return resultl*resultr.subst(x,y,n);
  }
 }
#endif
 // product does not contain expression for substitution
 // try to substitute in each factor
 Product p;
 for(list<Symbolic>::const_iterator i=factors.begin();i!=factors.end();++i)
  p.factors.push_back(i->subst(x,y,n));
 return p;
}

Simplified Product::simplify() const
{
 list<Symbolic>::const_iterator i;
 list<Symbolic>::iterator j, k, k1;
 Product r;

 // 1-element product:  (a) -> a
 if(factors.size() == 1) return factors.front().simplify();

 for(i=factors.begin();i!=factors.end();++i)
 {
  // absorb product of product: a * (a * a) * a -> a * a * a * a
  Simplified s = i->simplify();
  if(s.type() == typeid(Product))
  {
   CastPtr<const Product> product(s);
   r.factors.insert(r.factors.end(),product->factors.begin(),
                    product->factors.end());
  }
  else r.factors.push_back(s);
 }

 // if any matrices appear in the product,
 // pull everything (commutative) into the matrix
 SymbolicMatrix m(1,1,1);
 for(j=r.factors.begin();j!=r.factors.end();)
 {
  // found a matrix
  if(j->type() == typeid(SymbolicMatrix))
  {
   int br = 0;
   list<Symbolic>::iterator k;
   m = *CastPtr<const SymbolicMatrix>(*j);
   // some terms preceding the matrix must be brought in from the left
   k = j;
   if(j!=r.factors.begin())
    for(k--;1;)
    {
     // this is the last element so end the loop
     if(k == r.factors.begin()) br = 1;
     // only multiply with elements that commute, i.e. "scalars"
     if(k->commute(m))
     {
      m = *k * m;
      k1 = k;
      if(k != r.factors.begin()) k--;
      r.factors.erase(k1);
     }
     else br = 1;
     if(br) break;
    }

   // some terms following the matrix must be brought in from the right
   k = j;
   for(++k;k!=r.factors.end();)
    // multiply matrices with matrices
    if(k->type() == typeid(SymbolicMatrix))
    {
     m = m * *CastPtr<const SymbolicMatrix>(*k);
     k = r.factors.erase(k);
    }
    // only multiply with elements that commute, i.e. "scalars"
    else if(k->commute(m))
    {
     m = m * *k;
     k = r.factors.erase(k);
    }
    else break;
   
   // set *j to the resulting matrix
   *j = m.simplify();
   // set j to the next element after the last non-commuting element
   j = k;
  }
  else ++j;
 }

 // group common terms
 for(j=r.factors.begin();j!=r.factors.end();++j)
 {
  // numbers will be grouped later
  if(j->type() == typeid(Numeric)) continue;

  Symbolic n = 1;
  Symbolic j1 = *j;

  // the exponent in products must be ignored in grouping comparisons
  if(j1.type() == typeid(Power))
  {
   CastPtr<const Power> j2 = j1;
   n = j2->parameters.back();
   j1 = j2->parameters.front();
  }

  for(++(k=j);k!=r.factors.end() && j->commute(*k);)
  {
   // numbers will be grouped later
   if(k->type() == typeid(Numeric)) { ++k; continue; }

   Symbolic k1 = *k;
   Symbolic power = 1;
   // the exponent in products must be ignored in grouping comparisons
   if(k1.type() == typeid(Power))
   {
    CastPtr<const Power> k2 = k1;
    power = k2->parameters.back();
    k1 = k2->parameters.front();
   }

   if(j1 == k1)
   {
    n = n + power;
    k = r.factors.erase(k);
   }
   else ++k;
  }
  if(n == 0)
   *j = 1;
  else
   if(n == 1)
    *j = j1;
   else
    *j = (j1 ^ n).simplify();
 }

 // move numbers to the front
 Number<void> n = Number<int>(1);
 for(j=r.factors.begin();j!=r.factors.end();)
 {
  if(j->type() == typeid(Numeric))
  {
   n = n * Number<void>(*j);
   if(n.isZero()) return n;
   j = r.factors.erase(j);
  }
  else ++j;
 }

 if(!n.isOne()) r.factors.push_front(n->simplify());
 if(r.factors.size()==0) return Number<int>(1);
 if(r.factors.size()==1) return r.factors.front();
 return r;
}

int Product::compare(const Symbolic &s) const
{
 int c = 0;
 if(type() != s.type()) return 0;

 return (subst(s,Symbolic(1),c) == 1 && s.subst(*this,Symbolic(1),c) == 1
         && c == 2);
}

Symbolic Product::df(const Symbolic &s) const
{
 list<Symbolic>::iterator i;
 Product p(*this);
 Sum r;

 for(i=p.factors.begin();i!=p.factors.end();++i)
 {
  Symbolic t = *i;
  *i = i->df(s);
  r.summands.push_back(p);
  *i = t;
 }
 return r;
}

Symbolic Product::integrate(const Symbolic &s) const
{
 int count = 0;
 list<Symbolic>::const_iterator i, i1;

 for(i=factors.begin();i!=factors.end();++i)
  if(i->df(s) != 0) { ++count; i1 = i; }

 if(count == 1)
 {
  Product p;
  for(i=factors.begin();i!=factors.end();++i)
   if(i == i1) p.factors.push_back(::integrate(*i,s));
   else        p.factors.push_back(*i);
  return p;
 }

 if(count == 0) return *this * s;
 return Integral(*this,s);
}

Symbolic Product::coeff(const Symbolic &s) const
{
 int c = 0;
 Symbolic result = subst(s,1,c);

 if(c != 1) return 0;

 if(s.type() == typeid(Product))
 {
  CastPtr<const Product> p(s);
  list<Symbolic>::const_iterator i;
  for(i=p->factors.begin();i!=p->factors.end();++i)
  {
   // numbers always substitute successfully
   if(i->type() != typeid(Numeric))
    result.subst(*i,1,c);
   if(c != 1) return 0;
  }
 }

 if(s.type() == typeid(Power))
 {
  CastPtr<const Power> p(s);
  // numbers always substitute successfully
  if(p->parameters.front().type() != typeid(Numeric))
   result.subst(p->parameters.front(),1,c);
  if(c != 1) return 0;
 }

 if(s * result == *this) return result;

 return 0;
}

Expanded Product::expand() const
{
 list<Symbolic>::const_iterator i, k;
 list<Symbolic>::iterator j;
 Product r;

#if 0
 return *this;
 int mustexpand = 0;
 for(i=factors.begin();i!=factors.end();++i)
 {
  r.factors.push_back(i->expand());
  if(r.factors.back().type() == typeid(Sum)) mustexpand = 1;
 }
 if(!mustexpand) return r;
 r.factors.clear();
 Sum s; s.summands.push_back(Product());
 list<Symbolic>::iterator lsi, lsj;
 for(j=r.factors.begin();j!=r.factors.end();++j)
  if(j->type() == typeid(Sum))
  {
   CastPtr<const Sum> sum(*j);
   for(lsi=s.summands.begin();lsi!=s.summands.end();++lsi)
    for(k=sum->summands.begin();k!=sum->summands.end();++k)
    {
     CastPtr<Product> p(*lsi);
     p->factors.push_back(*k);
    }
  }
  else
   for(lsi=s.summands.begin();lsi!=s.summands.end();++i)
   {
    CastPtr<Product> p(*lsi);
    p->factors.push_back(*k);
   }
 return s;

#else
 for(i=factors.begin();i!=factors.end();++i)
 {
  Expanded s = i->expand();
  if(s.type() == typeid(Sum))
  {
   // make a copy of s
   CastPtr<Sum> sum(*s);
   k = i; ++k;
   for(j=sum->summands.begin();j!=sum->summands.end();++j)
   {
    Product p = r;
    p.factors.push_back(*j);
    p.factors.insert(p.factors.end(),k,factors.end());
    *j = p;
   }
   return sum->expand();
  }
  else r.factors.push_back(s);
 }
 return r;
#endif
}

int Product::commute(const Symbolic &s) const
{
 // Optimize the case for numbers
 if(s.type() == typeid(Numeric)) return 1;

 list<Symbolic>::const_iterator i;
 for(i=factors.begin();i!=factors.end();++i)
  if(!i->commute(s)) return 0;
 return 1;
}

PatternMatches
Product::match(const Symbolic &s, const list<Symbolic> &p) const
{
 PatternMatches l;
 list<Symbolic>::const_iterator i, i1;
 list<list<int> >::iterator j;
 list<int>::iterator k, k1;

 if(factors.size() == 0) return l;
 if(factors.size() == 1) return factors.front().match(s, p);
 if(s.type() != type()) return l;

 CastPtr<Product> prod(s);
 list<list<int> > thismatch;
 Product rest(*this);
 rest.factors.pop_front();
 thismatch.push_back(list<int>()); // start with an empty product

 for(i=prod->factors.begin();i!=prod->factors.end();++i)
 {
  for(j=thismatch.begin();j!=thismatch.end();++j)
  {
   list<int> s(*j); s.push_back(0); j->push_back(1);
   thismatch.insert(j, s);
  }
 }

 for(j=thismatch.begin();j!=thismatch.end();++j)
 {
  Product p1, p2;
  for(k=j->begin(), i=prod->factors.begin();
      k!=j->end() && i!=prod->factors.end(); ++i, ++k)
   if(*k) p1.factors.push_back(*i);
   else
   {
    for(k1=k,i1=i; k1!=j->end() && i1!=prod->factors.end(); ++i1,++k1)
     if((*k ^ *k1) && !i->commute(*i1)) break;
    if(k1 == j->end()) p2.factors.push_back(*i);
    else break;
   }
  if(k == j->end() && p1.factors.size() != 0 && p2.factors.size() != 0)
  {
   PatternMatches l1 = factors.front().match(p1, p);
   PatternMatches l2 = rest.match(p2, p);
   pattern_match_AND(l1, l2);
   pattern_match_OR(l, l1);
  }
 }
 return l;
}

PatternMatches
Product::match_parts(const Symbolic &s, const list<Symbolic> &p) const
{
 PatternMatches l = s.match(*this, p);
 list<Symbolic>::iterator i, i1, i2;
 list<list<int> >::iterator j;
 list<int>::iterator k, k1, k2;

 list<list<int> > matchpart;
 matchpart.push_back(list<int>()); // start with an empty product

 Product pr(*this);

 for(i=pr.factors.begin();i!=pr.factors.end();++i)
  if(i->type() == typeid(Power))
  {
   CastPtr<const Power> pw(*i);
   if(pw->parameters.back().type() == typeid(Numeric) && 
      Number<void>(pw->parameters.back()).numerictype() == typeid(int))
   {
    int s, t = CastPtr<const Number<int> >(pw->parameters.back())->n;
    if(t>0) *i = pw->parameters.front();
    if(i->type() == typeid(Product)) 
    {
     CastPtr<Product> pwp(*i);
     for(s=t-1; t>0 && s>0; s--)
      pr.factors.insert(i, pwp->factors.begin(), pwp->factors.end());
    }
    else
    for(s=t-1; t>0 && s>0; s--) pr.factors.insert(i, *i);
   }
  }

 for(i=pr.factors.begin();i!=pr.factors.end();++i)
 {
  PatternMatches lp = i->match_parts(s, p);
  pattern_match_OR(l, lp);
  for(j=matchpart.begin();j!=matchpart.end();++j)
  {
   list<int> s(*j); s.push_back(0); j->push_back(1);
   matchpart.insert(j, s);
  }
 }

 for(j=matchpart.begin();j!=matchpart.end();++j)
 {
  Product p1, p2;

#if 1
  for(k=j->begin(), i=pr.factors.begin();
      k!=j->end() && i!=pr.factors.end(); ++i, ++k)
   if(*k) p1.factors.push_back(*i);
   else p2.factors.push_back(*i);

  for(k=j->begin(), i=pr.factors.begin();
      k!=j->end() && i!=pr.factors.end(); ++i, ++k)
  {
   for(k1=j->begin(), i1=pr.factors.begin(); k1!=k && i1!=i; ++i1, ++k1)
    if(*k == *k1)
    {
     for(k2=j->begin(), i2=pr.factors.begin(); k2!=k && i2!=i; ++i2, ++k2)
      if(*k1 != *k2 && !i1->commute(*i2)) break;
     if(k2 != k) break;
    }
   if(k1 == k) break;
   for(++(k1=k), ++(i1=i); k1!=j->end() && i1!=pr.factors.end(); ++i1, ++k1)
    if(*k != *k1)
    {
     for(k2=j->begin(), i2=pr.factors.begin(); k2!=k && i2!=i; ++i2, ++k2)
      if(*k1 != *k2 && !i1->commute(*i2)) break;
     if(k2 != k) break;
    }
   if(k1 == k) break;
  }
  if(k != j->end())
#else
  list<int>::iterator endofp2;
  for(k=endofp2=j->begin(); k!=j->end();)
   if(*(k++) == 0 && (k == j->end() || *k == 1)) endofp2 = k;

  for(k=j->begin(), i=pr.factors.begin();
      k!=j->end() && i!=pr.factors.end(); ++i, ++k)
   if(*k) p1.factors.push_back(*i);
   else
   {
    for(k1=k,i1=i; k1!=endofp2 && i1!=pr.factors.end(); ++i1,++k1)
     if((*k ^ *k1) && !i->commute(*i1)) break;
    if(k1 == endofp2) p2.factors.push_back(*i);
    else break;
   }

  if(k == j->end())
#endif
  {
   if(p1.factors.size() != 0) pattern_match_OR(l, s.match(p1, p));
   if(p2.factors.size() != 0) pattern_match_OR(l, s.match(p2, p));
  }
 }
 return l;
}

#endif
#endif

