// -*- mode:C++ ; compile-command: "g++ -I.. -I../include -g xcasce.cc -o xcasce -lgiac -lgmp" -*-
#include "first.h"

/*
 *  Copyright (C) 2005 B. Parisse, Institut Fourier, 38402 St Martin d'Heres
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include "global.h"
using namespace std;
#include <string>
#include <stdexcept>
#include <fstream>
#include <iomanip>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
//#include <unistd.h> // For reading arguments from file
#include <fcntl.h>
#include <cstdlib>
#include <signal.h>
#include "gen.h"
#include "index.h"
#include "sym2poly.h"
#include "derive.h"
#include "intg.h"
#include "tex.h"
#include "lin.h"
#include "solve.h"
#include "modpoly.h"
#include "usual.h"
#include "sym2poly.h"
#include "moyal.h"
#include "ifactor.h"
#include "gauss.h"
#include "isom.h"
#include "plot.h"
#include "prog.h"
#include "rpn.h"
#include "pari.h"
#include "help.h"
#include "plot.h"
#include "input_lexer.h"
#include "global.h"

using namespace giac;

namespace giac {
  void child_signal_handler(int signum);
  // void child_plot_done(int signum);
  extern volatile bool signal_child;
}

void flush_stdout(){
  usleep(2000);
  fflush (stdout);
}

void flush_stderr(){
  usleep(2000);
  fflush (stderr);
}

bool has_graph_output(const giac::gen & g){
  return (g.type==giac::_SYMB) && (g._SYMBptr->sommet==giac::at_graph2tex || equalposcomp(giac::implicittex_plot_sommets,g._SYMBptr->sommet));
}

void check_browser_help(const giac::gen & g){
  if (g.is_symb_of_sommet(giac::at_findhelp)){
    giac::gen f=g._SYMBptr->feuille;
    string s;
    if (f.type==giac::_SYMB)
      f=f._SYMBptr->sommet;
    if (f.type==giac::_FUNC)
      s=f._FUNCptr->ptr->s;
    giac::html_vtt=giac::html_help(giac::html_mtt,s);
    if (!giac::html_vtt.empty())
      giac::system_browser_command(giac::html_vtt.front());
  }
}

int main(int ARGC, char *ARGV[]){    
  //cerr << giac::remove_filename(ARGV[0]) << endl;
#ifdef HAVE_LIBGSL
    gsl_set_error_handler_off();
#endif
  giac::secure_run=false;
  giac::gnuplot_ymin=-2.4;
  giac::gnuplot_ymax=2.1;
  signal(SIGINT,giac::ctrl_c_signal_handler);
  giac::child_id=1;
  ofstream logfile("log.txt");
  context contexte;
  contexte.globalptr->logptr=&logfile;
  context * contextptr =0;
  logfile << "Called by " << ARGC << " ";
  for (int i=0;i<ARGC;++i)
    logfile << ARGV[i] << " ";
  logfile << endl;
  if (ARGC>=3){
    chdir(ARGV[1]);
    giac::language=atoi(ARGV[2]);
    logfile << "Directory " << ARGV[1] << " language " << ARGV[2] << endl;
  }
  else
    chdir(giac::remove_filename(ARGV[0]).c_str());
  logfile.flush();
  FILE * stream = fopen("gnuplot.txt","w");
  fputc(' ',stream);
  fflush(stream);
  fclose(stream);
  sigset_t mask, oldmask;
  sigemptyset (&mask);
  sigaddset (&mask, SIGUSR1);
  signal(SIGUSR1,giac::child_signal_handler);
  // signal(SIGUSR2,giac::child_plot_done);
  gen args;
  // Enter endless loop, use SIGUSR1 for processing commands
  for (;;){
    /* Wait for a signal to arrive. */
    sigprocmask (SIG_BLOCK, &mask, &oldmask);
    signal_child=false;
    // #ifndef WIN32
    while (!signal_child)
      sigsuspend (&oldmask);
    sigprocmask (SIG_UNBLOCK, &mask, NULL);
    // #endif
    // read and evaluate input
    ifstream in("Cas.txt");
    ofstream out("Out.txt");
    pid_t caller;
    in >> caller;
    logfile << "Caller process id " << caller << endl; logfile.flush();
    giac::vecteur args,l;
    giac::readargs_from_stream(in,args,l);
    logfile << "Read " << args << endl; logfile.flush();
    clock_t start, end;  
    int s=args.size();
    for (int i=0;i<s;++i){
      out << "// " << args[i] << ' ' << char(10) << char(13) ;
      start = clock();
      giac::gen tmp=giac::_simplifier(giac::protecteval(args[i],giac::DEFAULT_EVAL_LEVEL,contextptr));
      end = clock();
      out << tmp << char(10) << char(13) ;
      logfile << "Eval arg " << i << " -> " << tmp << char(10) << char(13) ; logfile.flush();
      history_in(0).push_back(args[i]);
      history_out(0).push_back(tmp);
      // out << "// Time " << double(end-start)/CLOCKS_PER_SEC << char(10) << char(13)  << char(10) << char(13) ;
    }
    out.close();
    in.close();
    // send signal to caller
    kill(caller,SIGUSR1);
  }
  return 0;
}
