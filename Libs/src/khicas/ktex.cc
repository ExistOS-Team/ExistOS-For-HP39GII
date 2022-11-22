// -*- mode:C++ ; compile-command: "g++-3.4 -I.. -g -c tex.cc" -*-
#include "giacPCH.h"

/*
 *  Copyright (C) 2002,2014 B. Parisse, Institut Fourier, 38402 St Martin d'Heres
 *  Figure printing adapted from eukleides (c) 2002, Christian Obrecht
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
//#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef __ANDROID__
using std::vector;
#endif

using namespace std;
#include "gen.h"
#include "gausspol.h"
#include "identificateur.h"
#include "symbolic.h"
#include "poly.h"
#include "usual.h"
#include "tex.h"
#include "prog.h"
#include "rpn.h"
#include "plot.h"
#include "giacintl.h"

#ifndef NO_NAMESPACE_GIAC
namespace giac {
#endif // ndef NO_NAMESPACE_GIAC

#ifndef NO_NAMESPACE_GIAC
} // namespace giac
#endif // ndef NO_NAMESPACE_GIAC
