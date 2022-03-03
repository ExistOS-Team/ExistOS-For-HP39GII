// -*- compile-command: "g++ addu.cc -o addu" -*-
#include <string>
#include <iostream>
#include <fstream>
#include <unistd.h>

using namespace std;

void add1u(string & s){
  if (s.size()>12 && s.substr(0,11)=="mount -f -s")
    s=s.substr(0,6)+" -f -u "+s.substr(12,s.size()-12);
  if (s.size()>9 && s.substr(0,8)=="mount -s")
    s=s.substr(0,6)+" -u "+s.substr(9,s.size()-9);
}

void addu(const std::string & filename,const string & target){
  FILE * in=fopen(filename.c_str(),"r");
  string s1,s2;
  char ch;
  for (;!feof(in);){
    ch=fgetc(in);
    if (feof(in) or ch=='\n'){
      add1u(s1);
      s1+='\n';
      s2+=s1;
      s1="";
    }
    else
      s1+=ch;
  }
  fclose(in);
  ofstream of(target.c_str());
  // of << "del " << filename << endl;
  // of << "del " << target << endl;
  of << s2;
  of.close();
}

int main(int argc,char ** argv){
  if (argc<3)
    return 1;
  addu(argv[1],argv[2]);
}
