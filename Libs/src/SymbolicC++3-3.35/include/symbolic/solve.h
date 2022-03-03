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


// solve.h

#ifndef SYMBOLIC_CPLUSPLUS_SOLVE

#ifdef  SYMBOLIC_FORWARD
#ifndef SYMBOLIC_CPLUSPLUS_SOLVE_FORWARD
#define SYMBOLIC_CPLUSPLUS_SOLVE_FORWARD

#endif
#endif

#ifdef  SYMBOLIC_DECLARE
#define SYMBOLIC_CPLUSPLUS_SOLVE
#ifndef SYMBOLIC_CPLUSPLUS_SOLVE_DECLARE
#define SYMBOLIC_CPLUSPLUS_SOLVE_DECLARE

Equations solve(const Symbolic &, const Symbolic &);
Equations solve(const Equation &, const Symbolic &);
list<Equations> solve(const Equations &, const list<Symbolic> &);

#endif
#endif


#endif
