// -*- mode:C++ ; compile-command: "g++ -I.. -g gausspol.o sym2poly.o index.o input_lexer.o input_parser.o gaussint.o identificateur.o unary.o usual.o modpoly.o modfactor.o derive.o intg.o series.o subst.o symbolic.o vecteur.o moyal.o ifactor.o alg_EXT.o partfrac.cc -lgmp" -*-
/*
 *  Copyright (C) 2000 B. Parisse, Institut Fourier, 38402 St Martin d'Heres
 *
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
 *  along with this program. If not, see <http://www.gnu.org/licenses/>
 */
/* for profiler use -pg compile option then 
   a.out 
   then 
   gprof > test.gp 
   and 
   emacs test.gp &
*/

#include "sym2poly.h"
#include <iostream>
#include <time.h>

using namespace giac;

int main(int ARGC, char *ARGV[]){
  vecteur v,l;
  readargs(ARGC,ARGV,l,v);
  identificateur x;
  if (ARGC>2){
    assert(v[1].type==_IDNT);
    x=*v[1]._IDNTptr;
  }
  else
    x=*(l.compptr->front()._IDNTptr);
  clock_t start, end;  
  start = clock();
  cout << partfrac(v.front(),x) << endl;
  end = clock();
  cout << "# Time for (pf)" << double(end-start)/CLOCKS_PER_SEC << endl;
}

