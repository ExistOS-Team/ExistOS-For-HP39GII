// -*- mode:C++ ; compile-command: "g++ -I.. -g -c cas2.cc" -*-
#include "first.h"
#include <string>
#include "gen.h"

using namespace std;
using namespace giac;

int main(){
  string s;
  cin >> s;
  cout << eval(gen(s)) << endl;
}
