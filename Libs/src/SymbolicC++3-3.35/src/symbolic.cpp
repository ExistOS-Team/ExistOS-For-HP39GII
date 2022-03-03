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
#ifndef SYMBOLIC_CPLUSPLUS_SYMBOLIC_DEFINE
#define SYMBOLIC_CPLUSPLUS_SYMBOLIC_DEFINE
#define SYMBOLIC_CPLUSPLUS_SYMBOLIC

///////////////////////////////////////////////////
// Implementation for SymbolicInterface          //
///////////////////////////////////////////////////

SymbolicInterface::SymbolicInterface()
{ simplified = expanded = 0; }

SymbolicInterface::SymbolicInterface(const SymbolicInterface &s)
{ simplified = s.simplified; expanded = s.expanded; }

SymbolicInterface::~SymbolicInterface() {}

const type_info &SymbolicInterface::type() const
{ return typeid(*this); }


///////////////////////////////////////////////////
// Implementation for CloningSymbolicInterface   //
///////////////////////////////////////////////////

CloningSymbolicInterface::CloningSymbolicInterface()
 : SymbolicInterface(), Cloning() {}

CloningSymbolicInterface::CloningSymbolicInterface(
                                       const CloningSymbolicInterface &s)
 : SymbolicInterface(s), Cloning(s) {}

///////////////////////////////////////////////////
// Implementation for SymbolicProxy              //
///////////////////////////////////////////////////

SymbolicProxy::SymbolicProxy(const CloningSymbolicInterface &s)
 : CastPtr<CloningSymbolicInterface>(s) {}

SymbolicProxy::SymbolicProxy(const SymbolicProxy &s)
 : CastPtr<CloningSymbolicInterface>(s) {}

SymbolicProxy::SymbolicProxy(const Number<void> &n)
 : CastPtr<CloningSymbolicInterface>(n) {}

SymbolicProxy::SymbolicProxy() {}

void SymbolicProxy::print(ostream &o) const
{ (*this)->print(o); }

const type_info &SymbolicProxy::type() const
{ return (*this)->type(); }

Symbolic SymbolicProxy::subst(const Symbolic &x,
                              const Symbolic &y,int &n) const
{ return (*this)->subst(x,y,n); }

Simplified SymbolicProxy::simplify() const
{
 if((*this)->simplified) return *this;
 return (*this)->simplify();
}

int SymbolicProxy::compare(const Symbolic &s) const
{ return (*this)->compare(s); }

Symbolic SymbolicProxy::df(const Symbolic &s) const
{ return (*this)->df(s); }

Symbolic SymbolicProxy::integrate(const Symbolic &s) const
{ return (*this)->integrate(s); }

Symbolic SymbolicProxy::coeff(const Symbolic &s) const
{ return (*this)->coeff(s); }

Expanded SymbolicProxy::expand() const
{
 if((*this)->expanded) return *this;
 return (*this)->expand();
}

int SymbolicProxy::commute(const Symbolic &s) const
{ return (*this)->commute(s); }

PatternMatches
SymbolicProxy::match(const Symbolic &s, const list<Symbolic> &p) const
{ return (*this)->match(s,p); }

PatternMatches
SymbolicProxy::match_parts(const Symbolic &s, const list<Symbolic> &p) const
{ return (*this)->match_parts(s,p); }

SymbolicProxy &SymbolicProxy::operator=(const CloningSymbolicInterface &s)
{
 CastPtr<CloningSymbolicInterface>::operator=(s);
 return *this;
}

SymbolicProxy &SymbolicProxy::operator=(const SymbolicProxy &s)
{
 CastPtr<CloningSymbolicInterface>::operator=(s);
 return *this;
}

///////////////////////////////////////////////////
// Implementation for Simplified                 //
///////////////////////////////////////////////////

// these constructors should only be used by
// SymbolicInterface::simplify()

Simplified::Simplified(const CloningSymbolicInterface &s) : SymbolicProxy(s)
{ (*this)->simplified = 1; }

Simplified::Simplified(const SymbolicProxy &s) : SymbolicProxy(s)
{ (*this)->simplified = 1; }

Simplified::Simplified(const Number<void> &n) : SymbolicProxy(n)
{ (*this)->simplified = 1; }

///////////////////////////////////////////////////
// Implementation for Expanded                   //
///////////////////////////////////////////////////

// these constructors should only be used by
// SymbolicInterface::expand()

Expanded::Expanded(const CloningSymbolicInterface &s) : SymbolicProxy(s)
{ (*this)->expanded = 1; }

Expanded::Expanded(const SymbolicProxy &s) : SymbolicProxy(s)
{ (*this)->expanded = 1; }

Expanded::Expanded(const Number<void> &n) : SymbolicProxy(n)
{ (*this)->expanded = 1; }

///////////////////////////////////////////////////
// Implementation for Symbolic                   //
///////////////////////////////////////////////////

int Symbolic::auto_expand = 1;
int Symbolic::subst_count = 0;

Symbolic::Symbolic() : SymbolicProxy(Number<int>(0)) {}

Symbolic::Symbolic(const Symbolic &s) : SymbolicProxy(s)
{
 if(s.type() == typeid(SymbolicMatrix))
  // s is presumed const, so indexing via operator()
  // should access a copy
 {
  CastPtr<const SymbolicMatrix> csm(s);
  SymbolicProxy::operator=(*csm);
 }
}

Symbolic::Symbolic(const CloningSymbolicInterface &s)
{ *this = s; }

Symbolic::Symbolic(const SymbolicProxy &s) : SymbolicProxy(s)
{
 if(s.type() == typeid(SymbolicMatrix))
  // s is presumed const, so indexing via operator()
  // should access a copy
 {
  CastPtr<const SymbolicMatrix> csm(s);
  SymbolicProxy::operator=(*csm);
 }
}

Symbolic::Symbolic(const Number<void> &n) : SymbolicProxy(n) {}

Symbolic::Symbolic(const int &i)
 : SymbolicProxy(Number<int>(i).simplify()) {}

Symbolic::Symbolic(const double &d)
 : SymbolicProxy(Number<double>(d).simplify()) {}

Symbolic::Symbolic(const string &s)
 : SymbolicProxy(Symbol(s).simplify()) {}

Symbolic::Symbolic(const char *s)
 : SymbolicProxy(Symbol(s).simplify()) {}

Symbolic::Symbolic(const string &s,int n)
 : SymbolicProxy(SymbolicMatrix(s,n,1)) {}

Symbolic::Symbolic(const char *s,int n)
 : SymbolicProxy(SymbolicMatrix(s,n,1)) {}

Symbolic::Symbolic(const Symbolic &s,int n)
 : SymbolicProxy(SymbolicMatrix(s,n,1)) {}

Symbolic::Symbolic(const string &s,int n,int m)
 : SymbolicProxy(SymbolicMatrix(s,n,m)) {}

Symbolic::Symbolic(const char *s,int n,int m)
 : SymbolicProxy(SymbolicMatrix(s,n,m)) {}

Symbolic::Symbolic(const Symbolic &s,int n,int m)
 : SymbolicProxy(SymbolicMatrix(s,n,m)) {}

Symbolic::Symbolic(const list<Symbolic> &l)
{
 list<list<Symbolic> > ll;
 ll.push_back(l);
 (*this) = SymbolicMatrix(ll);
}

Symbolic::Symbolic(const list<list<Symbolic> > &l)
 : SymbolicProxy(SymbolicMatrix(l)) {}

Symbolic::~Symbolic() {}

SymbolicProxy &Symbolic::operator=(const CloningSymbolicInterface &s)
{
// cout << "*** " << &s << " " ; /*s.print(cout);*/ cout << endl;
#if 1
 if(auto_expand)
  SymbolicProxy::operator=(s.expand().simplify());
 else
#endif
  SymbolicProxy::operator=(s.simplify());
 return *this;
}

SymbolicProxy &Symbolic::operator=(const SymbolicProxy &s)
{ return SymbolicProxy::operator=(s); }

SymbolicProxy &Symbolic::operator=(const int &i)
{ return *this = Number<int>(i); }

SymbolicProxy &Symbolic::operator=(const double &d)
{ return *this = Number<double>(d); }

SymbolicProxy &Symbolic::operator=(const string &s)
{ return *this = Symbolic(s); }

SymbolicProxy &Symbolic::operator=(const char *s)
{ return *this = Symbolic(s); }

SymbolicProxy &Symbolic::operator=(const list<Symbolic> &l)
{ return *this = Symbolic(l); }

SymbolicProxy &Symbolic::operator=(const list<list<Symbolic> > &l)
{ return *this = Symbolic(l); }

Symbolic Symbolic::operator[](const Equation &p) const
{ return subst(p); }

Symbolic Symbolic::operator[](const Equations &l) const
{ return subst(l); }

Symbolic Symbolic::operator[](const Symbolic &p) const
{
 if(type() == typeid(Symbol))
  return CastPtr<const Symbol>(*this)->operator[](p);
 if(type() == typeid(SymbolicMatrix))
 {
  // make a copy of *this
  CastPtr<SymbolicMatrix> m(**this);
  int i, j;
  for(i=m->rows()-1;i>=0;i--)
   for(j=m->cols()-1;j>=0;j--)
    (*m)[i][j] = (*m)[i][j][p];
  return (*m);
 }
 return *this;
}

Symbolic Symbolic::operator[](const list<Symbolic> &l) const
{
 Symbolic result(*this);
 for(list<Symbolic>::const_iterator i=l.begin();i!=l.end();++i)
  result = result[*i];
 return result;
}

Symbolic &Symbolic::operator()(int i)
{
 if(type() != typeid(SymbolicMatrix))
 {
  if(i != 0)
  {
   cerr << "Attempted to cast " << *this
        << " in operator() to SymbolicMatrix failed." << endl;
   throw SymbolicError(SymbolicError::NotMatrix);
  }
  return *this;
 }
 CastPtr<SymbolicMatrix> m(*this);
 if(m->cols() == 1) return (*m)[i][0];
 if(m->rows() == 1) return (*m)[0][i];

 cerr << "Attempted to cast " << *this
      << " in operator() to SymbolicMatrix (vector) failed." << endl;
 throw SymbolicError(SymbolicError::NotVector);

 return *this;
}

Symbolic &Symbolic::operator()(int i,int j)
{
 if(type() != typeid(SymbolicMatrix))
 {
  if(i != 0 || j != 0)
  {
   cerr << "Attempted to cast " << *this
        << " in operator() to SymbolicMatrix failed." << endl;
   throw SymbolicError(SymbolicError::NotMatrix);
  }
  return *this;
 }

 CastPtr<SymbolicMatrix> m(*this);
 return (*m)[i][j];
}

const Symbolic &Symbolic::operator()(int i) const
{
 if(type() != typeid(SymbolicMatrix))
 {
  if(i != 0)
  {
   cerr << "Attempted to cast " << *this
        << " in operator() to SymbolicMatrix failed." << endl;
   throw SymbolicError(SymbolicError::NotMatrix);
  }
  return *this;
 }
 CastPtr<const SymbolicMatrix> m(*this);
 if(m->cols() == 1) return (*m)[i][0];
 if(m->rows() == 1) return (*m)[0][i];

 cerr << "Attempted to cast " << *this
      << " in operator() to SymbolicMatrix (vector) failed." << endl;
 throw SymbolicError(SymbolicError::NotVector);

 return *this;
}

const Symbolic &Symbolic::operator()(int i,int j) const
{
 if(type() != typeid(SymbolicMatrix))
 {
  if(i != 0 || j != 0)
  {
   cerr << "Attempted to cast " << *this
        << " in operator() to SymbolicMatrix failed." << endl;
   throw SymbolicError(SymbolicError::NotMatrix);
  }
  return *this;
 }

 CastPtr<const SymbolicMatrix> m(*this);
 return (*m)[i][j];
}

Symbolic Symbolic::subst(const Symbolic &x,const Symbolic &y,int &n) const
{ return SymbolicProxy::subst(x,y,n); }

Symbolic Symbolic::subst(const Symbolic &x,const int &j,int &n) const
{ return subst(x,Number<int>(j),n); }

Symbolic Symbolic::subst(const Symbolic &x,const double &d,int &n) const
{ return subst(x,Number<double>(d),n); }

Symbolic Symbolic::subst(const Equation &e,int &n) const
{
 int nsubs = 0;
 Symbolic lhs, rhs, r;
 list<Equations>::iterator i;
 // use the existing substitution for equations without binding variables
 if(e.free.empty()) return subst(e.lhs,e.rhs,n);

 PatternMatches eq = this->match_parts(e.lhs, e.free);
 for(i=eq.begin(),r=*this;i!=eq.end();++i)
 {
  lhs = e.lhs.subst(*i, nsubs);
  rhs = e.rhs.subst(*i, nsubs);
  // don't perform identity substitutions => subst_all loops infinitely
  if(lhs != rhs) r = r.subst(lhs,rhs,n);
 }
 return r;
}

Symbolic Symbolic::subst(const Equations &l,int &n) const
{
 Symbolic result(*this);
 for(Equations::const_iterator i=l.begin();i!=l.end();++i)
  result = result.subst(*i,n);
 return result;
}

Symbolic Symbolic::subst_all(const Symbolic &x,
                             const Symbolic &y,int &n) const
{
 int n1 = n;
 Symbolic r = subst(x,y,n);
 while(n != n1)
 {
  n1 = n;
  r = r.subst(x,y,n);
 }
 return r;
}

Symbolic Symbolic::subst_all(const Equation &e,int &n) const
{
 int n1 = n;
 Symbolic r = subst(e,n);
 while(n != n1)
 {
  n1 = n;
  r = r.subst(e,n);
 }
 return r;
}

Symbolic Symbolic::subst_all(const Equations &l,int &n) const
{
 int n1;
 Symbolic result(*this);
 do
 {
  n1 = n;
  for(Equations::const_iterator i=l.begin();i!=l.end();++i)
   result = result.subst(*i,n);
 } while(n != n1);

 return result;
}

Symbolic Symbolic::coeff(const Symbolic &s) const
{ return SymbolicProxy::coeff(s); }

Symbolic Symbolic::coeff(const Symbolic &s,int i) const
{
 if(i == 0) return subst(s,Number<int>(0));
 return SymbolicProxy::coeff(s^i);
}

Symbolic Symbolic::coeff(const int &i) const
{ return coeff(Number<int>(i)); }

Symbolic Symbolic::coeff(const double &d) const
{ return coeff(Number<double>(d)); }

Symbolic Symbolic::commutative(int c) const
{
 if(type() == typeid(Symbol))
  return CastPtr<const Symbol>(*this)->commutative(c);
 if(type() == typeid(SymbolicMatrix))
 {
  // make a copy *this
  CastPtr<SymbolicMatrix> m(**this);
  int i, j;
  for(i=m->rows()-1;i>=0;i--)
   for(j=m->cols()-1;j>=0;j--)
    (*m)[i][j] = ~ (*m)[i][j];
  return (*m);
 }
 return *this;
}

Symbolic Symbolic::operator~() const
{
 if(type() == typeid(Symbol))
  return CastPtr<const Symbol>(*this)->operator~();
 if(type() == typeid(SymbolicMatrix))
 {
  // make a copy of *this
  CastPtr<SymbolicMatrix> m(**this);
  int i, j;
  for(i=m->rows()-1;i>=0;i--)
   for(j=m->cols()-1;j>=0;j--)
    (*m)[i][j] = ~ (*m)[i][j];
  return (*m);
 }
 return *this;
}

Symbolic::operator int(void) const
{
 if(type() == typeid(Numeric) &&
    Number<void>(*this).numerictype() == typeid(int))
  return CastPtr<const Number<int> >(*this)->n;
 cerr << "Attempted to cast " << *this << " to int failed." << endl;
 throw SymbolicError(SymbolicError::NotInt);
 return 0;
}

Symbolic::operator double(void) const
{
 if(type() == typeid(Numeric) &&
    Number<void>(*this).numerictype() == typeid(double))
  return CastPtr<const Number<double> >(*this)->n;
 if(type() == typeid(Numeric) &&
    Number<void>(*this).numerictype() == typeid(int))
  return double(CastPtr<const Number<int> >(*this)->n);
 if(type() == typeid(Numeric) &&
    Number<void>(*this).numerictype() == typeid(Rational<Number<void> >))
 {
  CastPtr<const Number<Rational<Number<void> > > > n(*this);
  Symbolic num = n->n.num();
  Symbolic den = n->n.den();
  return double(num)/double(den);
 }
 cerr << "Attempted to cast " << *this << " to double failed." << endl;
 throw SymbolicError(SymbolicError::NotDouble);
 return 0.0;
}

Symbolic Symbolic::operator|(const Symbolic &s) const
{
 if(rows() != s.rows() || columns() != s.columns() ||
    (rows() != 1 && columns() != 1))
 {
  cerr << "Attempt to dot product " << *this
       << " and " << s << " failed." << endl;
  throw SymbolicError(SymbolicError::IncompatibleVector);
 }

 CastPtr<const SymbolicMatrix> m1(*this);
 CastPtr<const SymbolicMatrix> m2(s);

 if(m1->rows() == 1)
  return ((*m1)[0] | (*m2)[0]);
 else
  return ((*m1)(0) | (*m2)(0));
}

Symbolic Symbolic::operator%(const Symbolic &s) const
{
 if(rows() != s.rows() || columns() != s.columns() ||
    (rows() != 1 && columns() != 1))
 {
  cerr << "Attempt to cross product " << *this
       << " and " << s << " failed." << endl;
  throw SymbolicError(SymbolicError::IncompatibleVector);
 }

 CastPtr<const SymbolicMatrix> m1(*this);
 CastPtr<const SymbolicMatrix> m2(s);

 if(m1->rows() == 1)
  return SymbolicMatrix((*m1)[0] % (*m2)[0]);
 else
  return SymbolicMatrix((*m1)(0) % (*m2)(0));
}

int Symbolic::rows() const
{
 if(type() != typeid(SymbolicMatrix)) return 1;
 return CastPtr<const SymbolicMatrix>(*this)->rows();
}

int Symbolic::columns() const
{
 if(type() != typeid(SymbolicMatrix)) return 1;
 return CastPtr<const SymbolicMatrix>(*this)->cols();
}

Symbolic Symbolic::row(int i) const
{
 if(type() != typeid(SymbolicMatrix))
 {
   if(i == 0) return *this;
   cerr << "Attempted to cast " << *this
        << " in row() to SymbolicMatrix failed." << endl;
   throw SymbolicError(SymbolicError::NotMatrix);
 }
 Matrix<Symbolic> m = (*CastPtr<const SymbolicMatrix>(*this))[i];
 return SymbolicMatrix(m.transpose());
}

Symbolic Symbolic::column(int i) const
{
 if(type() != typeid(SymbolicMatrix))
 {
   if(i == 0) return *this;
   cerr << "Attempted to cast " << *this
        << " in column() to SymbolicMatrix failed." << endl;
   throw SymbolicError(SymbolicError::NotMatrix);
 }
 return SymbolicMatrix((*CastPtr<const SymbolicMatrix>(*this))(i));
}

Symbolic Symbolic::identity() const
{
 if(type() != typeid(SymbolicMatrix)) return Symbolic(1);
 return SymbolicMatrix(CastPtr<SymbolicMatrix>(**this)->identity());
}

Symbolic Symbolic::transpose() const
{
 if(type() != typeid(SymbolicMatrix)) return *this;
 return SymbolicMatrix(CastPtr<const SymbolicMatrix>(*this)->transpose());
}

Symbolic Symbolic::trace() const
{
 if(type() != typeid(SymbolicMatrix)) return *this;
 return CastPtr<const SymbolicMatrix>(*this)->trace();
}

Symbolic Symbolic::determinant() const
{
 if(type() != typeid(SymbolicMatrix)) return *this;
 return CastPtr<const SymbolicMatrix>(*this)->determinant();
}

Symbolic Symbolic::vec() const
{
 if(type() != typeid(SymbolicMatrix))
 {
   cerr << "Attempted to cast " << *this
        << " in vec() to SymbolicMatrix failed." << endl;
   throw SymbolicError(SymbolicError::NotMatrix);
 }
 return SymbolicMatrix(CastPtr<const SymbolicMatrix>(*this)->vec());
}

Symbolic Symbolic::kron(const Symbolic &s) const
{
 if(type() != typeid(SymbolicMatrix) ||
    s.type() != typeid(SymbolicMatrix))
 {
   cerr << "Attempted to cast " << *this << " or " << s
        << " in kron() to SymbolicMatrix failed." << endl;
   throw SymbolicError(SymbolicError::NotMatrix);
 }
 CastPtr<const SymbolicMatrix> m1(*this), m2(s);
 return SymbolicMatrix(m1->kron(*m2));
}

Symbolic Symbolic::dsum(const Symbolic &s) const
{
 if(type() != typeid(SymbolicMatrix) ||
    s.type() != typeid(SymbolicMatrix))
 {
   cerr << "Attempted to cast " << *this << " or " << s
        << " in dsum() to SymbolicMatrix failed." << endl;
   throw SymbolicError(SymbolicError::NotMatrix);
 }
 CastPtr<const SymbolicMatrix> m1(*this), m2(s);
 return SymbolicMatrix(m1->dsum(*m2));
}

Symbolic Symbolic::hadamard(const Symbolic &s) const
{
 if(type() != typeid(SymbolicMatrix) ||
    s.type() != typeid(SymbolicMatrix))
 {
   cerr << "Attempted to cast " << *this << " or " << s
        << " in hadamard() to SymbolicMatrix failed." << endl;
   throw SymbolicError(SymbolicError::NotMatrix);
 }
 CastPtr<const SymbolicMatrix> m1(*this), m2(s);
 return SymbolicMatrix(m1->hadamard(*m2));
}

Symbolic Symbolic::inverse() const
{
 if(type() != typeid(SymbolicMatrix)) return 1/(*this);
 CastPtr<const SymbolicMatrix> m(*this);
 return SymbolicMatrix(m->inverse());
}

#endif
#endif

