// -*- compile-command: "g++ -g find_global_var.cc -o find_global_var" -*-
#include <fstream>
#include <string>

using namespace std;

int main(int ARGC,char ** ARGV){
  char buf[10000];
  for (int i=1;i<ARGC;++i){
    ifstream j(ARGV[i]);
    cout << "// File " << ARGV[i] << endl;
    while (!j.eof()){
      j.getline(buf,10000,'\n');
      int s=strlen(buf);
      if (s>4 && buf[0]==' ' && buf[1]==' ' && buf[2]!=' '){
	for (int k=s-1;k>0;--k){
	  if (buf[k]!=' '){
	    if (buf[k]==';'){
	      string s=buf;
	      if (s.substr(0,16)!="  unary_function" && s.substr(0,14)!="  const string")
	      cout << buf << endl;
	    }
	    break;
	  }
	}
      }
    }
  }
}
