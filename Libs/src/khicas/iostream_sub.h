// -*- mode:C++ -*-
#ifndef _IOSTREAM_SUB_
#define _IOSTREAM_SUB_
//#include "mistream.h"
//#include "mostream.h"
#include <istream>
#include <ostream>
#include <string>
#include <vector>
#include <map>
#include <ctime>
/*
#include "ustring.h"
#include "uvector.h"
#include "umap.h"*/

void sprint_double(char *ch, double d);

extern "C" {
  void dConsolePut(const char *);
  void dConsolePutChar (char c);
  void dConsoleRedraw() ;
}

struct stdostream {
  void flush(){ dConsoleRedraw();}
};

struct stdistream {
};

inline stdostream & operator << (stdostream & os,int i){
  char buf[32];
  //sprint_int(buf,i);
  sprintf(buf, "%d", i);
  dConsolePut(buf);
  return os;
}

inline stdostream & operator << (stdostream & os,unsigned i){
  char buf[32];
  sprintf(buf,"%u",i);
  dConsolePut(buf);
  return os;
}

inline stdostream & operator << (stdostream & os,char ch){
  dConsolePutChar(ch);
  return os;
}

inline stdostream & operator << (stdostream & os,double d){
  char buf[32];
  sprint_double(buf,d);
  //sprintf(buf,"%lf",d);
  dConsolePut(buf);
  return os;
}

inline stdostream & operator << (stdostream & os,clock_t t){
  char buf[32];
  sprintf(buf,"%u",t);
  dConsolePut(buf);
  return os;
}

inline stdostream & operator << (stdostream & os,const char * s){
  dConsolePut(s);
  return os;
}


inline stdostream &  operator<<(stdostream& os, std::ostream& (*pf)(std::ostream&))
{
  if(pf == (std::basic_ostream<char>& (*)(std::basic_ostream<char>&)) &std::endl)
  {
    dConsolePut("\n");
  }
  return os;
}
// inline stdostream & operator << (auto s)
// {
  
// }

inline stdostream & operator << (stdostream & os,const std::string & s){
  dConsolePut(s.c_str());
  return os;  
}

template<class T>
  stdostream & operator << (stdostream & os,const std::vector<T> & v){
  size_t s=v.size();
  os << "[";
  for (size_t i=0;i<s;++i){
    os << v[i] ;
    if (i!=s-1)
      os << ",";
  }
  os << "]";
  return os;
}

template<class T,class U>
  stdostream & operator << (stdostream & os,const std::map<T,U> & v){
  os << "{";
  typename std::map<T,U>::const_iterator it=v.begin(),itend=v.end();
  for (;it!=itend;){
    os << it->first << ":" << it->second ;
    ++it;
    if (it!=itend)
      os << ",";
  }
  os << "}";
  return os;
}

extern stdostream cout;

#endif
