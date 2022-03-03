// -*- compile-command: "cl /Zi /I. /EHsc giac_oo.cpp giac_oo.lib gmp.lib" -*-
#include <giac/gen.h>
#include <giac/prog.h>
/*
const char * parse_eval(const char * ch,int level,void * contextptr){
  static std::string s;
  giac::context * cptr = (giac::context *)contextptr;
  giac::logptr(&std::cout,0);
  giac::child_id=1;
  giac::gen g(ch,cptr);
  g=protecteval(g,level,cptr);
  s=g.print(cptr);
  return s.c_str();
}
*/

int main(){
  std::string s;
  for (;;){
    std::cout << "Expression: " ;
    std::cin >> s;
    giac::gen g(s,0);
    if (is_zero(g))
      return 0;
    g=giac::protecteval(g,1,0);
    std::cout << g.print(0) << std::endl;
  }
}
