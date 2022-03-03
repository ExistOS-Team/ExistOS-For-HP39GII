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
#ifndef SYMBOLIC_CPLUSPLUS_SYMBOL_DEFINE
#define SYMBOLIC_CPLUSPLUS_SYMBOL_DEFINE
#define SYMBOLIC_CPLUSPLUS_SYMBOL

//////////////////////////////////////
// Implementation of Symbol         //
//////////////////////////////////////

Symbol::Symbol(const Symbol &s)
: CloningSymbolicInterface(s),
  name(s.name), parameters(s.parameters), commutes(s.commutes) {}

Symbol::Symbol(const string &s,int c) : name(s), commutes(c) {}

Symbol::Symbol(const char *s,int c)   : name(s), commutes(c) {}

Symbol::~Symbol() {}

void Symbol::print(ostream &o) const
{
 list<Symbolic>::const_iterator i;
 o << name;
 if(!parameters.empty())
 {
  o << ((type() == typeid(Symbol)) ? "[":"(");
  parameters.front().print(o);
  for(i=++parameters.begin();i!=parameters.end();++i)
  { o << ","; i->print(o); }
  o << ((type() == typeid(Symbol)) ? "]":")");
 }
}

Symbolic Symbol::subst(const Symbolic &x,const Symbolic &y,int &n) const
{
 if(*this == x)
 {
  ++n;
  return y;
 }
 list<Symbolic>::iterator i;
 // make a copy of *this
 CastPtr<Symbol> s(*this);
 for(i=s->parameters.begin();i!=s->parameters.end();++i)
  *i = i->subst(x,y,n);
 // reset the simplified and expanded flags
 // since substitution may have changed this
 s->simplified = s->expanded = 0;
 return *s;
}

Simplified Symbol::simplify() const
{
 list<Symbolic>::iterator i;
 // make a copy of *this
 CastPtr<Symbol> sym(*this);

 for(i=sym->parameters.begin();i!=sym->parameters.end();++i)
  *i = i->simplify();

 return *sym;
}

int Symbol::compare(const Symbolic &s) const
{
 list<Symbolic>::const_iterator i;
 list<Symbolic>::const_iterator j;

 if(s.type() != type()) return 0;
 CastPtr<const Symbol> sym = s;
 if(sym->name != name) return 0;
 if(sym->parameters.size() != parameters.size()) return 0;
 for(i=parameters.begin(),j=sym->parameters.begin();
     i!=parameters.end();
     ++i,++j)
  if(*i != *j) return 0;
 return 1;
}

Symbolic Symbol::df(const Symbolic &s) const
{
 list<Symbolic>::const_iterator i;

 if(*this == s) return 1;

 Symbolic result = 0;
 for(i=parameters.begin();i!=parameters.end();++i)
  result = result + Derivative(*this,*i) * i->df(s);

 return result;
}

Symbolic Symbol::integrate(const Symbolic &s) const
{
 list<Symbolic>::const_iterator i;

 if(*this == s) return (s^2)/2;

 for(i=parameters.begin();i!=parameters.end();++i)
  if(i->df(s) != 0) return Integral(*this,s);

 return *this * s;
}

Symbolic Symbol::coeff(const Symbolic &s) const
{
 if(*this == s) return 1;
 return 0;
}

Expanded Symbol::expand() const
{
 // make a copy of *this
 CastPtr<Symbol> r(*this);
 list<Symbolic>::iterator i;
 for(i=r->parameters.begin();i!=r->parameters.end();++i)
  *i = i->expand();
 return *r;
}

int Symbol::commute(const Symbolic &s) const
{
 list<Symbolic>::const_iterator i;

 if(*this == s) return 1;
 for(i=parameters.begin();i!=parameters.end();++i)
  if(!i->commute(s)) return 0;

 try {
  CastPtr<const Symbol> cp(s);
  for(i=cp->parameters.begin();i!=cp->parameters.end();++i)
   if(!commute(*i)) return 0;
  return cp->commutes || commutes;
 }
 catch(bad_cast) {}
 
 if(s.type() == typeid(SymbolicMatrix))
  return commutes;

 return s.commute(*this);
}

PatternMatches
Symbol::match(const Symbolic &s, const list<Symbolic> &p) const
{
 PatternMatches l;
 list<Symbolic>::const_iterator i, j;

 if((type() == typeid(Symbol) || type() == typeid(UniqueSymbol))
    && find(p.begin(), p.end(), *this) != p.end())
  pattern_match_OR(l, *this == s);

 if(*this == s)
 {
  pattern_match_TRUE(l);
  return l;
 }

 if(type() == s.type())
 {
  CastPtr<Symbol> sym(s);
  if(name == sym->name && commutes == sym->commutes &&
     parameters.size() == sym->parameters.size())
   for(i=parameters.begin(), j=sym->parameters.begin();
       i!=parameters.end(); ++i, ++j)
   {
    PatternMatches ls = i->match(*j,p);
    if(i == parameters.begin()) pattern_match_OR(l, ls);
    else pattern_match_AND(l, ls);
   }
 }
 return l;
}

PatternMatches
Symbol::match_parts(const Symbolic &s, const list<Symbolic> &p) const
{
 PatternMatches l = s.match(*this, p);
 list<Symbolic>::const_iterator i;

 for(i=parameters.begin();i!=parameters.end();++i)
 {
  PatternMatches ls = i->match_parts(s, p);
  pattern_match_OR(l, ls);
 }
 return l;
}

// this should not be used with the functions cos, exp etc. since
// the overrides on simplification and other methods will be lost
Symbol Symbol::operator[](const Symbolic &s) const
{
 if(type() != typeid(Symbol))
  cerr << "Warning: " << *this << " [" << s
       << "] discards any methods which have been overridden." << endl;
 Symbol r(*this);
 r.parameters.push_back(s);
 return r;
}

// this should not be used with the functions cos, exp etc.
// since the override on simplification and other methods
// will be lost
Symbol Symbol::operator[](const list<Symbolic> &l) const
{
 if(type() != typeid(Symbol))
  cerr << "Warning: " << *this << " [..."
       << "] discards any methods which have been overridden." << endl;
 Symbol r(*this);
 r.parameters.insert(r.parameters.end(),l.begin(),l.end());
 return r;
}

// this should not be used with the functions cos, exp etc.
// since the override on simplification and other methods
// will be lost
Symbol Symbol::commutative(int c) const
{
 if(type() != typeid(Symbol))
  cerr << "Warning: " << *this << " .commutative(" << c
       << ") discards any methods which have been overridden." << endl;
 Symbol r(*this);
 r.commutes = c;
 return r;
}

Symbol Symbol::operator~() const
{ return commutative(!commutes); }


//////////////////////////////////////
// Implementation of UniqueSymbol   //
//////////////////////////////////////

UniqueSymbol::UniqueSymbol() : Symbol("") { p = new int(1); }

UniqueSymbol::UniqueSymbol(const UniqueSymbol &u) : Symbol(u), p(u.p)
{ ++(*p); }

UniqueSymbol::UniqueSymbol(const Symbol &s) : Symbol(s) { p = new int(1); }

UniqueSymbol::~UniqueSymbol()
{ if(--(*p) == 0) delete p; }

void UniqueSymbol::print(ostream &o) const
{ o << "c@" << p; }

int UniqueSymbol::compare(const Symbolic &s) const
{ return (type() == s.type() && p == CastPtr<UniqueSymbol>(s)->p); }

#endif
#endif

