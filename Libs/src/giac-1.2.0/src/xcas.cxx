#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_LIBFLTK
#include "hist.cxx"
#else
#include <iostream>
using namespace std;
int main(){
  cerr << "No GUI support, try cas instead or recompile Giac with" << endl;
  cerr << "./configure --enable-fltk-support" << endl;
  cerr << "make clean ; make" << endl;
}
#endif
