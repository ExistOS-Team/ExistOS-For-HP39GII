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


// constants.h

#ifndef SYMBOLIC_CPLUSPLUS_CONSTANTS

#ifdef  SYMBOLIC_DECLARE
#define SYMBOLIC_CPLUSPLUS_CONSTANTS
#ifndef SYMBOLIC_CPLUSPLUS_CONSTANTS_DECLARE
#define SYMBOLIC_CPLUSPLUS_CONSTANTS_DECLARE

namespace SymbolicConstant
{
 static const Symbolic i = Symbolic(Power(Symbolic(-1),
                                          Power(Symbolic(2), Symbolic(-1))));
 static const Symbolic i_symbol("i");
 static const Symbolic e       ("e");
 static const Symbolic pi      ("pi");
}

#endif
#endif
#endif
