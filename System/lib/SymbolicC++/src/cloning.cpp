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

#include "cloning.h"


///////////////////////////////
// Cloning Implementation    //
///////////////////////////////

Cloning::Cloning() : refcount(0), free_p(0) {}

Cloning::Cloning(const Cloning &c) : refcount(c.refcount), free_p(c.free_p) { }

Cloning::~Cloning() {}


void Cloning::reference(Cloning *c)
{ if(c != 0 && c->refcount != 0) c->refcount++; }

void Cloning::unreference(Cloning *c)
{
 if(c != 0 && c->refcount != 0 && c->free_p != 0)
 {
  if(c->refcount == 1) c->free_p(c);
  else c->refcount--;
 }
}

///////////////////////////////
// CloningPtr Implementation //
///////////////////////////////

CloningPtr::CloningPtr() : value(0) { }

CloningPtr::CloningPtr(const Cloning &p)
{ value = p.clone(); }

CloningPtr::CloningPtr(const CloningPtr &p) : value(p.value)
{ Cloning::reference(value); }

CloningPtr::~CloningPtr()
{ Cloning::unreference(value); }

CloningPtr &CloningPtr::operator=(const Cloning &p)
{
 if(value == &p) return *this;
 Cloning::unreference(value);
 value = p.clone();
 return *this;
}

CloningPtr &CloningPtr::operator=(const CloningPtr &p)
{
 if(this == &p) return *this;
 if(value == p.value) return *this;
 Cloning::unreference(value);
 Cloning::reference(value = p.value);
 return *this;
}

