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
#ifndef SYMBOLIC_CPLUSPLUS_FUNCTIONS_DEFINE
#define SYMBOLIC_CPLUSPLUS_FUNCTIONS_DEFINE
#define SYMBOLIC_CPLUSPLUS_FUNCTIONS

//////////////////////////////////////
// Implementation of Sin            //
//////////////////////////////////////

Sin::Sin(const Sin &s) : Symbol(s) {}

Sin::Sin(const Symbolic &s) : Symbol(Symbol("sin")[s]) {}

Simplified Sin::simplify() const
{
 const Symbolic &s = parameters.front().simplify();
 if(s == 0) return Number<int>(0);
 if(s.type() == typeid(Product))
 {
  CastPtr<const Product> p(s);
  if(p->factors.front() == -1) return -Sin(-s);
 }
 if(s.type() == typeid(Numeric) &&
    Number<void>(s).numerictype() == typeid(double))
  return Number<double>(sin(CastPtr<const Number<double> >(s)->n));
 return *this;
}

Symbolic Sin::df(const Symbolic &s) const
{ return cos(parameters.front()) * parameters.front().df(s); }

Symbolic Sin::integrate(const Symbolic &s) const
{
 const Symbolic &x = parameters.front();
 if(x == s) return -cos(x);
 if(df(s) == 0) return *this * s;
 return Integral(*this,s);
}

//////////////////////////////////////
// Implementation of Cos            //
//////////////////////////////////////

Cos::Cos(const Cos &s) : Symbol(s) {}

Cos::Cos(const Symbolic &s) : Symbol(Symbol("cos")[s]) {}

Simplified Cos::simplify() const
{
 const Symbolic &s = parameters.front().simplify();
 if(s == 0) return Number<int>(1);
 if(s.type() == typeid(Product))
 {
  CastPtr<const Product> p(s);
  if(p->factors.front() == -1) return Cos(-s);
 }
 if(s.type() == typeid(Numeric) &&
    Number<void>(s).numerictype() == typeid(double))
  return Number<double>(cos(CastPtr<const Number<double> >(s)->n));
 return *this;
}

Symbolic Cos::df(const Symbolic &s) const
{ return -sin(parameters.front()) * parameters.front().df(s); }

Symbolic Cos::integrate(const Symbolic &s) const
{
 const Symbolic &x = parameters.front();
 if(x == s) return sin(x);
 if(df(s) == 0) return *this * s;
 return Integral(*this,s);
}

//////////////////////////////////////
// Implementation of Sinh           //
//////////////////////////////////////

Sinh::Sinh(const Sinh &s) : Symbol(s) {}

Sinh::Sinh(const Symbolic &s) : Symbol(Symbol("sinh")[s]) {}

Simplified Sinh::simplify() const
{
 const Symbolic &s = parameters.front().simplify();
 if(s == 0) return Number<int>(0);
 if(s.type() == typeid(Product))
 {
  CastPtr<const Product> p(s);
  if(p->factors.front() == -1) return -Sinh(-s);
 }
 if(s.type() == typeid(Numeric) &&
    Number<void>(s).numerictype() == typeid(double))
  return Number<double>(sinh(CastPtr<const Number<double> >(s)->n));
 return *this;
}

Symbolic Sinh::df(const Symbolic &s) const
{ return cosh(parameters.front()) * parameters.front().df(s); }

Symbolic Sinh::integrate(const Symbolic &s) const
{
 const Symbolic &x = parameters.front();
 if(x == s) return cosh(x);
 if(df(s) == 0) return *this * s;
 return Integral(*this,s);
}

//////////////////////////////////////
// Implementation of Cosh           //
//////////////////////////////////////

Cosh::Cosh(const Cosh &s) : Symbol(s) {}

Cosh::Cosh(const Symbolic &s) : Symbol(Symbol("cosh")[s]) {}

Simplified Cosh::simplify() const
{
 const Symbolic &s = parameters.front().simplify();
 if(s == 0) return Number<int>(1);
 if(s.type() == typeid(Product))
 {
  CastPtr<const Product> p(s);
  if(p->factors.front() == -1) return Cosh(-s);
 }
 if(s.type() == typeid(Numeric) &&
    Number<void>(s).numerictype() == typeid(double))
  return Number<double>(cosh(CastPtr<const Number<double> >(s)->n));
 return *this;
}

Symbolic Cosh::df(const Symbolic &s) const
{ return sinh(parameters.front()) * parameters.front().df(s); }

Symbolic Cosh::integrate(const Symbolic &s) const
{
 const Symbolic &x = parameters.front();
 if(x == s) return sinh(x);
 if(df(s) == 0) return *this * s;
 return Integral(*this,s);
}

//////////////////////////////////////
// Implementation of Log            //
//////////////////////////////////////

Log::Log(const Log &s) : Symbol(s) {}

Log::Log(const Symbolic &s1,const Symbolic &s2)
: Symbol(Symbol("log")[s1,s2]) {}

void Log::print(ostream &o) const
{
 if(parameters.size() == 2 && parameters.front() == SymbolicConstant::e)
 {
  Log l = *this;
  l.name = "ln";
  l.parameters.pop_front();
  l.print(o);
 }
 else
  Symbol::print(o);
}

Simplified Log::simplify() const
{
 // log_a(b)
 const Symbolic &a = parameters.front().simplify();
 const Symbolic &b = parameters.back().simplify();
 if(b == 1) return Number<int>(0);
 if(b == a) return Number<int>(1);
 if(b.type() == typeid(Power))
 {
  CastPtr<const Power> p = b;
  if(p->parameters.front() == a)
   return p->parameters.back();
 }
 if(b.type() == typeid(Numeric)                     &&
    Number<void>(b).numerictype() == typeid(double) &&
    CastPtr<const Number<double> >(b)->n > 0.0)
  return Product(Number<double>(log(CastPtr<const Number<double> >(b)->n)),
                 Power(ln(a),-1)).simplify();
 return *this;
}

// d/ds log_a(b) = d/ds (ln(b) / ln(a))
//               = (1/b db/ds - log_a(b) / a da/ds) / ln(a)
//               = (a db/ds - b log_a(b) da/ds) / (a b ln(a))
Symbolic Log::df(const Symbolic &s) const
{
 const Symbolic &a = parameters.front();
 const Symbolic &b = parameters.back();
 return (a * b.df(s) - b * *this * a.df(s)) / (a * b * ln(a));
}

Symbolic Log::integrate(const Symbolic &s) const
{
 const Symbolic &x = parameters.back();
 const Symbolic &a = parameters.front();
 // int(log_a(x)) = (x ln(x) - x) / ln(a)
 if(x == s && a.df(s) == 0)
  return (x * *this - x) / ln(a);
 if(df(s) == 0) return *this * s;
 return Integral(*this,s);
}

//////////////////////////////////////
// Implementation of Power          //
//////////////////////////////////////

Power::Power(const Power &s) : Symbol(s) {}

Power::Power(const Symbolic &s,const Symbolic &p) : Symbol("pow")
{ parameters.push_back(s); parameters.push_back(p); }

void Power::print(ostream &o) const
{
  if(*this == SymbolicConstant::i)
  { o << SymbolicConstant::i_symbol; return; }

  int parens1 = parameters.front().type() == typeid(Symbol)
             || parameters.front().type() == typeid(Sin)
             || parameters.front().type() == typeid(Cos)
             || parameters.front().type() == typeid(Sinh)
             || parameters.front().type() == typeid(Cosh)
             || parameters.front().type() == typeid(Log)
             || parameters.front().type() == typeid(Derivative);
  int parens2 = parameters.back().type() == typeid(Symbol)
             || parameters.back().type() == typeid(Sin)
             || parameters.back().type() == typeid(Cos)
             || parameters.back().type() == typeid(Sinh)
             || parameters.back().type() == typeid(Cosh)
             || parameters.back().type() == typeid(Log)
             || parameters.back().type() == typeid(Derivative);
  parens1 = !parens1;
  parens2 = !parens2;
  if(parens1) o << "(";
  parameters.front().print(o);
  if(parens1) o << ")";
  o << "^";
  if(parens2) o << "(";
  parameters.back().print(o);
  if(parens2) o << ")";
}

Simplified Power::simplify() const
{
 list<Symbolic>::iterator i, j;
 const Symbolic &b = parameters.front().simplify();
 const Symbolic &n = parameters.back().simplify();
 if(n == 0) return Number<int>(1);
 if(n == 1) return b;
 if(b == 0) return Number<int>(0);
 if(b == 1) return Number<int>(1);
 if(b.type() == typeid(Power))
 {
  CastPtr<const Power> p = b;
  return (p->parameters.front() ^ (p->parameters.back() * n)).simplify();
 }
 if(b.type() == typeid(Numeric) &&
    n.type() == typeid(Numeric) &&
    Number<void>(n).numerictype() == typeid(int))
 {
  int i = CastPtr<const Number<int> >(n)->n;
  int inv = (i<0) ? 1 : 0;
  Number<void> r = Number<int>(1), x = b;
  i = (inv) ? -i : i;
  while(i != 0)
  {
   if(i & 1)  r = r * x;
   x = x * x;
   i >>= 1;
  }
  if(inv) return (Number<int>(1) / r);
  return r;
 }
 if(n.type() == typeid(Log))
 {
  CastPtr<const Log> l(n);
  if(l->parameters.front() == b)
   return l->parameters.back();
 }
 if(b.type() == typeid(Numeric) &&
    n.type() == typeid(Numeric) &&
    Number<void>(b).numerictype() == typeid(double))
 {
  double nd, bd = CastPtr<const Number<double> >(b)->n;
  if(Number<void>(n).numerictype() == typeid(int))
   nd = CastPtr<const Number<int> >(n)->n;
  else if(Number<void>(n).numerictype() == typeid(double))
   nd = CastPtr<const Number<double> >(n)->n;
  else if(Number<void>(n).numerictype() == typeid(Rational<Number<void> >))
   nd = double(CastPtr<const Number<Rational<Number<void> > > >(n)->n);
  else return Power(b,n);
  if(bd >= 0.0 || int(nd) == nd)
   return Number<double>(pow(bd,nd));
 }
 if(b.type() == typeid(Numeric) &&
    n.type() == typeid(Numeric) &&
    Number<void>(n).numerictype() == typeid(double))
 {
  double bd, nd = CastPtr<const Number<double> >(n)->n;
  if(Number<void>(b).numerictype() == typeid(int))
   bd = CastPtr<const Number<int> >(b)->n;
  else if(Number<void>(b).numerictype() == typeid(double))
   bd = CastPtr<const Number<double> >(b)->n;
  else if(Number<void>(b).numerictype() == typeid(Rational<Number<void> >))
   bd = double(CastPtr<const Number<Rational<Number<void> > > >(b)->n);
  else return Power(b,n);
  if(bd >= 0.0 || int(nd) == nd)
   return Number<double>(pow(bd,nd));
 }
 return Power(b,n);
}

Expanded Power::expand() const
{
 Symbolic b = parameters.front().expand();
 Symbolic n = parameters.back().expand();
 if(b.type() != typeid(Sum)     &&
    b.type() != typeid(Product) &&
    b.type() != typeid(Numeric))
  return Power(b,n);

 // a^(b+c) == a^b a^c  when b and c commute
 if(n.type() == typeid(Sum))
 {
  CastPtr<const Sum> s(*n);
  Product p;
  list<Symbolic>::const_iterator k, k1;
  for(k=s->summands.begin();k!=s->summands.end();++k)
   for(++(k1=k);k1!=s->summands.end();++k1)
    if(!k->commute(*k1)) return Power(b,n);
  for(k=s->summands.begin();k!=s->summands.end();++k)
   p.factors.push_back(Power(b,*k));
  return p;
 }

 // (a*b)^c == a^c b^c  when a and b commute
 if(b.type() == typeid(Product))
 {
  CastPtr<const Product> p(*b);
  Product r;
  list<Symbolic>::const_iterator k, k1;
  for(k=p->factors.begin();k!=p->factors.end();++k)
   for(++(k1=k);k1!=p->factors.end();++k1)
    if(!k->commute(*k1)) return Power(b,n);
  for(k=p->factors.begin();k!=p->factors.end();++k)
   r.factors.push_back(Power(*k,n));
  return r;
 }

 if(n.type() != typeid(Numeric) ||
    Number<void>(n).numerictype() != typeid(int))
  return Power(b,n);

 int ae = Symbolic::auto_expand;
 int i = CastPtr<const Number<int> >(n)->n;
 if(i<0) return *this;
 int sgn = (i>=0) ? 1 : -1;
 Symbolic r = 1, x = b;
 i *= sgn;

 Symbolic::auto_expand = 0;
 while(i != 0)
 {
  if(i & 1) r = Product(r,x).expand();
  i >>= 1;
  if(i != 0) x = Product(x,x).expand();
 }
 Symbolic::auto_expand = ae;

 return Power(r,Number<int>(sgn));
}

Symbolic Power::subst(const Symbolic &x,const Symbolic &y,int &n) const
{
 if(x.type() == typeid(Power))
 {
  CastPtr<const Power> p(x);
  if(parameters.front() == p->parameters.front()                  &&
     parameters.back().type() == typeid(Numeric)                  &&
     p->parameters.back().type() == typeid(Numeric)               &&
     Number<void>(parameters.back()).numerictype() == typeid(int) &&
     Number<void>(p->parameters.back()).numerictype() == typeid(int))
  {
   int s = CastPtr<const Number<int> >(parameters.back())->n;
   int t = CastPtr<const Number<int> >(p->parameters.back())->n;
   if((s > 0) && (t > 0) && (s >= t))
   {
    ++n;
    return Power(parameters.front(),s - t).subst(x,y,n) * y;
   }
  }
 }
 return Symbol::subst(x,y,n);
}

// d/ds (a^b) = d/ds exp(b ln(a))
//            = (ln(a) db/ds + b/a da/ds) a^b
Symbolic Power::df(const Symbolic &s) const
{
 const Symbolic &a = parameters.front();
 const Symbolic &b = parameters.back();
 return (ln(a)*b.df(s) + (b/a)*a.df(s)) * *this;
}

Symbolic Power::integrate(const Symbolic &s) const
{
 const Symbolic &a = parameters.front();
 const Symbolic &b = parameters.back();
 if(a == s && b.df(s) == 0)
 {
  if(b == -1) return ln(a);
  return (a^(b+1)) / (b+1);
 }
 if(a == SymbolicConstant::e && b == s)
  return *this;
 if(df(s) == 0) return *this * s;
 return Integral(*this,s);
}

PatternMatches
Power::match_parts(const Symbolic &s, const list<Symbolic> &p) const
{
 PatternMatches l;
 if(s.type() == typeid(Power))
 {
  CastPtr<const Power> pw(s);
  l = pw->parameters.front().match(parameters.front(), p);
  if(!l.empty()                                                   &&
     parameters.back().type() == typeid(Numeric)                  &&
     pw->parameters.back().type() == typeid(Numeric)              &&
     Number<void>(parameters.back()).numerictype() == typeid(int) &&
     Number<void>(pw->parameters.back()).numerictype() == typeid(int))
  {
   int s = CastPtr<const Number<int> >(parameters.back())->n;
   int t = CastPtr<const Number<int> >(pw->parameters.back())->n;
   if((s < 0) || (t < 0) || (s < t))
    pattern_match_FALSE(l);
  }
 }

 if(parameters.back().type() == typeid(Numeric) &&
    Number<void>(parameters.back()).numerictype() == typeid(int))
 {
  int n = abs(CastPtr<const Number<int> >(parameters.back())->n);
  Product pr;
  while(n-->0) pr.factors.insert(pr.factors.end(), parameters.back());
  pattern_match_OR(l, pr.match_parts(s, p));
 }

 pattern_match_OR(l, Symbol::match_parts(s, p));
 return l;
}

//////////////////////////////////////
// Implementation of Derivative     //
//////////////////////////////////////

Derivative::Derivative(const Derivative &d) : Symbol(d) { }

Derivative::Derivative(const Symbolic &s1,const Symbolic &s2)
: Symbol("df")
{
 if(s1.type() == typeid(Derivative))
 {
  parameters = CastPtr<const Derivative>(s1)->parameters;
  parameters.push_back(s2);
 }
 else
 {
  parameters.push_back(s1);
  parameters.push_back(s2);
 }
}

Symbolic Derivative::subst(const Symbolic &x,const Symbolic &y,int &n) const
{
 if(*this == x) return y;

 list<Symbolic>::const_iterator i;
 list<Symbolic>::iterator j;

 if(x.type() == type() &&
    parameters.front() == CastPtr<const Derivative>(x)->parameters.front())
 {
  // make a copy of this
  CastPtr<Derivative> l(*this);
  CastPtr<const Derivative> d(x);
  i = d->parameters.begin();
  for(++i;i!=d->parameters.end();++i)
  {
   j = l->parameters.begin();
   for(++j;j!=l->parameters.end();++j)
    if(*j == *i) break;
   if(j == l->parameters.end()) break;
   l->parameters.erase(j);
  }
  if(i == d->parameters.end())
  {
   ++n;
   Symbolic newdf = y;
   j = l->parameters.begin();
   for(++j;j!=l->parameters.end();++j)
    newdf = ::df(newdf,*j);
   return newdf;
  }
 }

 i = parameters.begin();
 Symbolic dy = i->subst(x,y,n);

 if(dy == *i)
 {
  Derivative d(*this);
  for(j=d.parameters.begin(),++j;j!=d.parameters.end();++j)
   *j = j->subst(x,y,n);
  return d;
 }

 for(++i;i!=parameters.end();++i)
  dy = dy.df(i->subst(x,y,n));

 return dy;
}

Symbolic Derivative::df(const Symbolic &s) const
{
 list<Symbolic>::const_iterator i;

 if(parameters.front().type() == typeid(Symbol))
 {
  Symbolic result;
  CastPtr<const Symbol> sym(parameters.front());
  for(i=sym->parameters.begin();i!=sym->parameters.end();++i)
   result = result + Derivative(*this,*i) * i->df(s);

  return result;
 }

 if(parameters.front().df(s) != 0)
 {
  Derivative d(*this);
  d.parameters.push_back(s);
  return d;
 }

 return 0;
}

int Derivative::compare(const Symbolic &s) const
{
 list<Symbolic>::const_iterator i;
 list<Symbolic>::iterator j;

 if(s.type() != type()) return 0;
 // make a copy of s
 CastPtr<Derivative> d(*s);
 if(d->parameters.size() != parameters.size()) return 0;
 if(d->parameters.front() != parameters.front()) return 0;
 for(i=parameters.begin(),++i;i!=parameters.end();++i)
 {
  for(j=d->parameters.begin(),++j;j!=d->parameters.end();++j)
   if(*i == *j) break;
  if(j == d->parameters.end()) return 0;
  d->parameters.erase(j);
 }
 return 1;
}

Symbolic Derivative::integrate(const Symbolic &s) const
{
 int n = 0, n1;
 list<Symbolic>::const_iterator i, i1 = parameters.end();
 list<Symbolic>::iterator j;

 for(i=parameters.begin();i!=parameters.end();++i,++n)
  if(*i == s) { i1 = i; n1 = n; }

 if(i1 != parameters.end())
 {
  // make a copy of *this
  CastPtr<Derivative> d(*this);
  for(j=d->parameters.begin();n1!=0;++j,--n1);
  d->parameters.erase(j);
  if(d->parameters.size() == 1) return d->parameters.front();
  return *d;
 }

 if(parameters.front().df(s) == 0) return *this * s;
 return Integral(*this,s);
}

//////////////////////////////////////
// Implementation of Integral       //
//////////////////////////////////////

Integral::Integral(const Integral &d) : Symbol(d) { }

Integral::Integral(const Symbolic &s1,const Symbolic &s2) : Symbol("int")
{
 if(s1.type() == typeid(Integral))
 {
  parameters = CastPtr<const Integral>(s1)->parameters;
  parameters.push_back(s2);
 }
 else
 {
  parameters.push_back(s1);
  parameters.push_back(s2);
 }
}

Symbolic Integral::subst(const Symbolic &x,const Symbolic &y,int &n) const
{
 if(*this == x) { ++n; return y; }

 list<Symbolic>::const_iterator i = parameters.begin();
 Symbolic dy = i->subst(x,y,n);
 for(++i;i!=parameters.end();++i)
  dy = ::integrate(dy, i->subst(x,y,n));
 return dy;
}

Symbolic Integral::df(const Symbolic &s) const
{
 int n = 0, n1;
 list<Symbolic>::const_iterator i, i1 = parameters.end();
 list<Symbolic>::iterator j;

 for(i=parameters.begin();i!=parameters.end();++i,++n)
  if(*i == s) { i1 = i; n1 = n; }

 if(i1 != parameters.end())
 {
  // make a copy of *this
  CastPtr<Integral> in(*this);
  for(j=in->parameters.begin();n1!=0;++j,--n1);
  in->parameters.erase(j);
  if(in->parameters.size() == 1) return in->parameters.front();
  return *in;
 }

 if(parameters.front().df(s) == 0) return 0;
 return Derivative(*this,s);
}

Symbolic Integral::integrate(const Symbolic &s) const
{
 if(parameters.front().df(s) != 0)
 {
  Integral i(*this);
  i.parameters.push_back(s);
  return i;
 }

 return *this * s;
}

//////////////////////////////////////
// Implementation of Rows           //
//////////////////////////////////////

Rows::Rows(const Rows &s) : Symbol(s) {}

Rows::Rows(const Symbolic &s) : Symbol(Symbol("rows")[s]) {}

Simplified Rows::simplify() const
{
 const Symbolic &s = parameters.front().simplify();
 if(s.type() != typeid(SymbolicMatrix)) return Rows(s);
 return Symbolic(CastPtr<const SymbolicMatrix>(s)->rows());
}

//////////////////////////////////////
// Implementation of Columns        //
//////////////////////////////////////

Columns::Columns(const Columns &s) : Symbol(s) {}

Columns::Columns(const Symbolic &s) : Symbol(Symbol("columns")[s]) {}

Simplified Columns::simplify() const
{
 const Symbolic &s = parameters.front().simplify();
 if(s.type() != typeid(SymbolicMatrix)) return Columns(s);
 return Symbolic(CastPtr<const SymbolicMatrix>(s)->cols());
}

//////////////////////////////////////
// Implementation of Row            //
//////////////////////////////////////

Row::Row(const Row &s) : Symbol(s), row(s.row) {}

Row::Row(const Symbolic &s,int r) : Symbol(Symbol("row")[s,r]), row(r) {}

Simplified Row::simplify() const
{
 const Symbolic &s = parameters.front().simplify();
 if(s.type() != typeid(SymbolicMatrix)) return Row(s, row);
 return SymbolicMatrix(CastPtr<const SymbolicMatrix>(s)->operator[](row));
}

//////////////////////////////////////
// Implementation of Column         //
//////////////////////////////////////

Column::Column(const Column &s) : Symbol(s), column(s.column) {}

Column::Column(const Symbolic &s,int c)
 : Symbol(Symbol("column")[s,c]), column(c) {}

Simplified Column::simplify() const
{
 const Symbolic &s = parameters.front().simplify();
 if(s.type() != typeid(SymbolicMatrix)) return Column(s, column);
 return SymbolicMatrix(CastPtr<const SymbolicMatrix>(s)->operator()(column));
}

//////////////////////////////////////
// Implementation of Transpose      //
//////////////////////////////////////

Transpose::Transpose(const Transpose &s) : Symbol(s) {}

Transpose::Transpose(const Symbolic &s) : Symbol(Symbol("transpose")[s]) {}

Simplified Transpose::simplify() const
{
 const Symbolic &s = parameters.front().simplify();
 if(s.type() != typeid(SymbolicMatrix)) return Transpose(s);
 return SymbolicMatrix(CastPtr<const SymbolicMatrix>(s)->transpose());
}

//////////////////////////////////////
// Implementation of Trace          //
//////////////////////////////////////

Trace::Trace(const Trace &s) : Symbol(s) {}

Trace::Trace(const Symbolic &s) : Symbol(Symbol("tr")[s]) {}

Simplified Trace::simplify() const
{
 const Symbolic &s = parameters.front().simplify();
 if(s.type() != typeid(SymbolicMatrix)) return Trace(s);
 return CastPtr<const SymbolicMatrix>(s)->trace();
}

//////////////////////////////////////
// Implementation of Determinant    //
//////////////////////////////////////

Determinant::Determinant(const Determinant &s) : Symbol(s) {}

Determinant::Determinant(const Symbolic &s) : Symbol(Symbol("det")[s]) {}

Simplified Determinant::simplify() const
{
 const Symbolic &s = parameters.front().simplify();
 if(s.type() != typeid(SymbolicMatrix)) return Determinant(s);
 return CastPtr<const SymbolicMatrix>(s)->determinant();
}

//////////////////////////////////////
// Implementation of Vec            //
//////////////////////////////////////

Vec::Vec(const Vec &s) : Symbol(s) {}

Vec::Vec(const Symbolic &s) : Symbol(Symbol("vec")[s]) {}

Simplified Vec::simplify() const
{
 const Symbolic &s = parameters.front().simplify();
 if(s.type() != typeid(SymbolicMatrix)) return Vec(s);
 return SymbolicMatrix(CastPtr<const SymbolicMatrix>(s)->vec());
}

//////////////////////////////////////
// Implementation of Kronecker      //
//////////////////////////////////////

Kronecker::Kronecker(const Kronecker &s) : Symbol(s) {}

Kronecker::Kronecker(const Symbolic &s1, const Symbolic &s2)
 : Symbol(Symbol("kron")[s1,s2]) {}

Simplified Kronecker::simplify() const
{
 const Symbolic &s1 = parameters.front().simplify();
 const Symbolic &s2 = parameters.back().simplify();
 if(s1.type() != typeid(SymbolicMatrix)) return Kronecker(s1,s2);
 if(s2.type() != typeid(SymbolicMatrix)) return Kronecker(s1,s2);
 return SymbolicMatrix(CastPtr<const SymbolicMatrix>(s1)
                       ->kron(*CastPtr<const SymbolicMatrix>(s2)));
}

//////////////////////////////////////
// Implementation of DirectSum      //
//////////////////////////////////////

DirectSum::DirectSum(const DirectSum &s) : Symbol(s) {}

DirectSum::DirectSum(const Symbolic &s1, const Symbolic &s2)
 : Symbol(Symbol("dsum")[s1,s2]) {}

Simplified DirectSum::simplify() const
{
 const Symbolic &s1 = parameters.front().simplify();
 const Symbolic &s2 = parameters.back().simplify();
 if(s1.type() != typeid(SymbolicMatrix)) return DirectSum(s1,s2);
 if(s2.type() != typeid(SymbolicMatrix)) return DirectSum(s1,s2);
 return SymbolicMatrix(CastPtr<const SymbolicMatrix>(s1)
                       ->dsum(*CastPtr<const SymbolicMatrix>(s2)));
}

//////////////////////////////////////
// Implementation of Hadamard       //
//////////////////////////////////////

Hadamard::Hadamard(const Hadamard &s) : Symbol(s) {}

Hadamard::Hadamard(const Symbolic &s1, const Symbolic &s2)
 : Symbol(Symbol("hadamard")[s1,s2]) {}

Simplified Hadamard::simplify() const
{
 const Symbolic &s1 = parameters.front().simplify();
 const Symbolic &s2 = parameters.back().simplify();
 if(s1.type() != typeid(SymbolicMatrix)) return Hadamard(s1,s2);
 if(s2.type() != typeid(SymbolicMatrix)) return Hadamard(s1,s2);
 return SymbolicMatrix(CastPtr<const SymbolicMatrix>(s1)
                        ->hadamard(*CastPtr<const SymbolicMatrix>(s2)));
}

//////////////////////////////////////
// Implementation of Gamma          //
//////////////////////////////////////

Gamma::Gamma(const Gamma &s) : Symbol(s) {}

Gamma::Gamma(const Symbolic &s) : Symbol(Symbol("gamma")[s]) {}

Simplified Gamma::simplify() const
{
 const Symbolic &s = parameters.front().simplify();
 if(s.type() != typeid(Numeric)) return Gamma(s);
 CastPtr<Numeric> n(s);
 if(n->numerictype() == typeid(int))
 {
  int i = int(s) - 1;
  if(i >= 0)
  {
   Symbolic f = 1;
   while(i > 0) f *= i--;
   return f;
  }
 }
 return Gamma(s);
}

#endif
#endif

