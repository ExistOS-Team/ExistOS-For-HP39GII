// -*- mode:C++ ; compile-command: "g++ -I.. -fPIC -DPIC -g -c pgcd.cpp -o pgcd.lo && ln -sf pgcd.lo pgcd.o && gcc -shared pgcd.lo -lc  -Wl,-soname -Wl,libpgcd.so.0 -o libpgcd.so.0.0.0 && ln -sf libpgcd.so.0.0.0 libpgcd.so.0 && ln -sf libpgcd.so.0.0.0 libpgcd.so" -*-
using namespace std;
#include <stdexcept>
#include <cmath>
#include <cstdlib>
#include <giac/giac.h>
//#include "pgcd.h"

#ifndef NO_NAMESPACE_GIAC
namespace giac {
#endif // ndef NO_NAMESPACE_GIAC

  gen pgcd(gen a,gen b){
    gen q,r;
    for (;b!=0;){
      r=irem(a,b,q);
      a=b;
      b=r;
    }
    return a;
  }
  gen _pgcd(const gen & args){
    if ( (args.type!=_VECT) || (args._VECTptr->size()!=2))
      setsizeerr();
    vecteur &v=*args._VECTptr;
    return pgcd(v[0],v[1]);
  }
  const string _pgcd_s("pgcd");
  unary_function_unary __pgcd(&_pgcd,_pgcd_s);
  unary_function_ptr at_pgcd (&__pgcd,0,true);
  

#ifndef NO_NAMESPACE_GIAC
} // namespace giac
#endif // ndef NO_NAMESPACE_GIAC
