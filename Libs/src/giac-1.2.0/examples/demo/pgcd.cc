// -*- compile-command: "g++ -g pgcd.cc -lgiac -lgmp" -*-
#include <giac/giac.h>

using namespace std;
using namespace giac;

int main(){
  context ct;
  cout << "Enter two polynomials ";
  gen a,b;
  cin >> a >> b;
  cout << "PGCD is " << _gcd(makesequence(a,b),&ct) << endl;
  return 0;
}
