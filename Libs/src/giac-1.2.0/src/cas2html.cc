// -*- mode:C++ ; compile-command: "g++  cas2html.cc -o cas2html -lgiac -lgmp" -*-
// g++ --static cas2html.cc -o cas2html -lgiac -lgsl -lgslcblas -lmpfr -lgmp -ldl
// Pour installer sur BSD brandelf -t Linux cas2html
#ifndef IN_GIAC
#include <giac/giac.h>
#include <giac/mathml.h>
#include <giac/tex.h>
#else
#include "giac.h"
#include "mathml.h"
#include "tex.h"
#endif

#include <string>
#include <fstream>

using namespace std;
using namespace giac;

void elimine_inf_sup(string &s){
  string t="";
  while (t!=s){
    t=s;
    int i=s.find("<"), j=s.find(">");
    if (i!=-1)
      s.replace(s.find("<"),1,"&lt;");
    if (j!=-1)
      s.replace(s.find(">"),1,"&gt;");
  }
}
 

int main(int ARGC, char *ARGV[])
{
  giac::child_id=1;
  bool b_mathml=0, b_text=0, b_latex=0;
  string s, t;
  gen *g, e;
  // recherche des paramètres passés au programme
  for (int i=1 ; i<ARGC ; i++) {  
    if (string(ARGV[i])=="-mathml" || string(ARGV[i])=="-m")
      b_mathml=1;
    else if (string(ARGV[i])=="-text" || string(ARGV[i])=="-t")
      b_text=1;
    else if (string(ARGV[i])=="-latex" || string(ARGV[i])=="-l")
      b_latex=1;
    else {
      s=string(ARGV[i]);
      g= new gen(s);
      e=eval(*g);
      if (b_mathml){
	t=gen2mathml(e);
	s=gen2mathml(*g);
      }
      else if (b_latex){
	t=gen2tex(e);
	s=gen2tex(*g);
      }
      else{
	t=e.print();
	s=g->print();
      }
      if (b_text){
	elimine_inf_sup(t);
	elimine_inf_sup(s);
      }
      cout<<s<<endl;
      cout<<t<<endl;
      delete g;
    }
  }// end for i
}  
