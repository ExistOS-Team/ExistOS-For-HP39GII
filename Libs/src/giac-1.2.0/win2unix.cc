// -*- compile-command: "g++ win2unix.cc -o win2unix" -*-
#include <string>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string.h>

using namespace std;


// Convert absolute installer Windows path to a Cygwin path
// and write corresponding files runxcas.fr/es/en
int main(int argc,char ** argv){
  if (argc<2)
    return 1;
  string s(argv[1]);
  for (int i=2;i<argc;++i){
    s += ' ';
    s += argv[i];
  }
  // if (!s.empty() && s[s.size()-1]!='\\') s += '\\';
  const char * ptr=s.c_str();
  int l=strlen(ptr);
  if (l<2)
    return 2;
  /*
  string s_nospace;
  for (int i=0;i<l;++i){
    if (s[i]==' ')
      s_nospace+="\\ ";
    else
      s_nospace+=s[i];
  }
  */
  string unixpath("/cygdrive/");
  unixpath += *ptr;
  ++ptr;
  for (++ptr;*ptr;++ptr){
    switch (*ptr){
    case '\\':
      unixpath += '/';
      break;
    default:
      unixpath += *ptr;
    }
  }
  chdir(unixpath.c_str());
  ofstream bf("cxcasfr.bat");
  // Was bf << "set PATH=%PATH%;" << s << endl; 
  bf << "set PATH=" << s << ";%PATH%" << endl; 
  bf << "bash.exe '" << unixpath << "/runxcas.fr' %1" << endl;
  ofstream cbf("xcasfr.bat");
  // Was bf << "set PATH=%PATH%;" << s << endl; 
  cbf << "set PATH=" << s << ";%PATH%" << endl; 
  cbf << "bash.exe '" << unixpath << "/runxcas.fr' %1" << endl;
  ofstream of("runxcas.fr");
  of << "#! /bin/bash\nexport LANG=fr_FR.UTF-8\n";
  of << "export XCAS_ROOT='" << unixpath << "'\n";
  of << "# export XCAS_HOME='/cygdrive/p'\n";
  of << "# export XCAS_AUTOSAVEFOLDER='/cygdrive/p'\n";
  of << "export XCAS_LOCALE=\"$XCAS_ROOT/locale/\"\n";
  of << "export XCAS_HELP=\"$XCAS_ROOT/aide_cas\"\n\"$XCAS_ROOT/xcas.exe\" \"$1\"\n";
  of.close();
  ofstream ebf("cxcases.bat");
  ebf << "set PATH=" << s << ";%PATH%" << endl; 
  ebf << "bash.exe '" << unixpath << "/runxcas.es' %1" << endl;
  ofstream bf1("xcases.bat");
  bf1 << "set PATH=" << s << ";%PATH%" << endl; 
  bf1 << "bash.exe '" << unixpath << "/runxcas.es' %1" << endl;
  ofstream of1("runxcas.es");
  of1 << "#! /bin/bash\nexport LANG=es_ES.UTF-8\n";
  of1 << "export XCAS_ROOT='" << unixpath << "'\n";
  of1 << "# export XCAS_HOME='/cygdrive/p'\n";
  of1 << "# export XCAS_AUTOSAVEFOLDER='/cygdrive/p'\n";
  of1 << "export XCAS_LOCALE=\"$XCAS_ROOT/locale/\"\n";
  of1 << "export XCAS_HELP=\"$XCAS_ROOT/aide_cas\"\n\"$XCAS_ROOT/xcas.exe\" \"$1\"\n";
  of1.close();
  ofstream grf1("xcasgre.bat");
  grf1 << "set PATH=" << s << ";%PATH%" << endl; 
  grf1 << "bash.exe '" << unixpath << "/runxcas.gre' %1" << endl;
  grf1.close();
  ofstream grf2("runxcas.gre");
  grf2 << "#! /bin/bash\nexport LANG=el_GR.UTF-8\n";
  grf2 << "export XCAS_ROOT='" << unixpath << "'\n";
  grf2 << "# export XCAS_HOME='/cygdrive/p'\n";
  grf2 << "# export XCAS_AUTOSAVEFOLDER='/cygdrive/p'\n";
  grf2 << "export XCAS_LOCALE=\"$XCAS_ROOT/locale/\"\n";
  grf2 << "export XCAS_HELP=\"$XCAS_ROOT/aide_cas\"\n\"$XCAS_ROOT/xcas.exe\" \"$1\"\n";
  grf2.close();
  ofstream enbf("cxcasen.bat");
  enbf << "set PATH=" << s << ";%PATH%" << endl; 
  enbf << "bash.exe '" << unixpath << "/runxcas.en' %1" << endl;
  ofstream bf2("xcasen.bat");
  bf2 << "set PATH=" << s << ";%PATH%" << endl; 
  bf2 << "bash.exe '" << unixpath << "/runxcas.en' %1" << endl;
  ofstream of2("runxcas.en");
  of2 << "#! /bin/bash\nexport LANG=en\n";
  of2 << "export XCAS_ROOT='" << unixpath << "'\n";
  of2 << "# export XCAS_HOME='/cygdrive/p'\n";
  of2 << "# export XCAS_AUTOSAVEFOLDER='/cygdrive/p'\n";
  of2 << "export XCAS_LOCALE=\"$XCAS_ROOT/locale/\"\n";
  of2 << "export XCAS_HELP=\"$XCAS_ROOT/aide_cas\"\n\"$XCAS_ROOT/xcas.exe\" \"$1\"\n";
  of2.close();
  ofstream bf3("xcaszh.bat");
  bf3 << "set PATH=" << s << ";%PATH%" << endl; 
  bf3 << "bash.exe '" << unixpath << "/runxcas.zh' %1" << endl;
  ofstream of3("runxcas.zh");
  of3 << "#! /bin/bash\nexport LANG=zh\n";
  of3 << "export XCAS_ROOT='" << unixpath << "'\n";
  of3 << "# export XCAS_HOME='/cygdrive/p'\n";
  of3 << "# export XCAS_AUTOSAVEFOLDER='/cygdrive/p'\n";
  of3 << "export XCAS_LOCALE=\"$XCAS_ROOT/locale/\"\n";
  of3 << "export XCAS_HELP=\"$XCAS_ROOT/aide_cas\"\n\"$XCAS_ROOT/xcas.exe\" \"$1\"\n";
  of3.close();
  ofstream bf4("xcasde.bat");
  bf4 << "set PATH=" << s << ";%PATH%" << endl; 
  bf4 << "bash.exe '" << unixpath << "/runxcas.de' %1" << endl;
  ofstream of4("runxcas.de");
  of4 << "#! /bin/bash\nexport LANG=de\n";
  of4 << "export XCAS_ROOT='" << unixpath << "'\n";
  of4 << "# export XCAS_HOME='/cygdrive/p'\n";
  of4 << "# export XCAS_AUTOSAVEFOLDER='/cygdrive/p'\n";
  of4 << "export XCAS_LOCALE=\"$XCAS_ROOT/locale/\"\n";
  of4 << "export XCAS_HELP=\"$XCAS_ROOT/aide_cas\"\n\"$XCAS_ROOT/xcas.exe\" \"$1\"\n";
  of4.close();
  return 0;
}

  /*
  ofstream bf0("altair.bat");
  bf0 << "set PATH=" << s << ";%PATH%" << endl; 
  bf0 << "bash.exe '" << unixpath << "/runxcas.alt'" << endl;
  ofstream of0("runxcas.alt");
  of << "#! /bin/bash\nexport LANG=fr_FR:fr\n";
  of << "export XCAS_ROOT='" << unixpath << "'";
  of << "\nexport XCAS_LOCALE=\"$XCAS_ROOT/locale/\"\n";
  of << "export XCAS_HELP=\"$XCAS_ROOT/aide_cas\"\n\"$XCAS_ROOT/xcas.exe\"\n";
  of.close();
  */
