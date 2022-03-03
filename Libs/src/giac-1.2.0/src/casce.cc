// -*- mode:C++ ; compile-command: "g++ -I.. -I../include -g -c casce.cc" -*-
#include "first.h"

/*
 *  Copyright (C) 2000 B. Parisse, Institut Fourier, 38402 St Martin d'Heres
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
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include "global.h"
#ifdef HAVE_LIBREADLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif // HAVE_LIBREADLINE
using namespace std;
#include <string>
#include <stdexcept>
#include <fstream>
#include <iomanip>
#include <time.h>
#include <sys/time.h>
//#include <unistd.h> // For reading arguments from file
#include <fcntl.h>
#include <cstdlib>
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


using namespace giac;
#ifdef HAVE_LIBREADLINE
static char *line_read = (char *)NULL;

/* Read a string, and return a pointer to it.  Returns NULL on EOF. */
char *
rl_gets (int count)
{
  /* If the buffer has already been allocated, return the memory
     to the free pool. */
  if (line_read)
    {
      free (line_read);
      line_read = (char *)NULL;
    }
  
  /* Get a line from the user. */
  string prompt(giac::print_INT_(count)+">> ");
  line_read = readline ((char *)prompt.c_str());
  
  /* If the line has any text in it, save it on the history. */
  if (line_read && *line_read)
    add_history (line_read);
  
  return (line_read);
}

#endif // HAVE_LIBREADLINE

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

void xdvi_view_2d_plot(){
  double horiz_unit=giac::horiz_latex/(giac::gnuplot_xmax-giac::gnuplot_xmin);
  double vert_unit=giac::vert_latex/(giac::gnuplot_ymax-giac::gnuplot_ymin);
  double unit=horiz_unit;
  if (horiz_unit>vert_unit)
    unit=vert_unit;
  int i=giac::history_out(0).size();
  giac::graph2tex("casgraph.tex",giac::vecteur(giac::history_out(0).begin()+giac::erase_pos(i),giac::history_out(0).begin()+i),giac::gnuplot_xmin,giac::gnuplot_xmax,giac::gnuplot_ymin,giac::gnuplot_ymax,unit);
  system("latex casgraph.tex >& casgraph.___ && xdvi casgraph &") ;
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
  // Calling sequence should be
  // ARGV 0=cas.exe 1=start_directory 2=language_number 3=session_name
  // 4=xcas_history_level number,
  // If ARGC==5, remove all levels after xcas_history_level, no output
  // If ARGC==6, ARGV[5]=string to parse and eval, will replace level
  // Use -1 as history_level for addition
  // Output in Out.txt of all levels starting from replaced level
  // If ARGC>6, all history levels after ARGV[4] will be destroyed
  // and replaced by evaluation of ARGV[5..ARGC-1]
  // Output in Out.txt of all levels starting from replaced level
  // Examples
  // remove the file xcas.ar
  // cas.exe / 1 xcas.ar -1 "a:=1+1;" (create level 0)
  // cas.exe / 1 xcas.ar -1 "a+1;" (create level 1)
  // cas.exe / 1 xcas.ar 0 "a:=1+2;" (replace level 0 by 1+2,
  // recompute level 1, output level 0 and 1)
  // cas.exe / 1 xcas.ar 1 (remove all levels after level 1, no output)
  // cas.exe / 1 xcas.ar 1 "b:=a+2" "c:=b/2" (2 new levels created,
  // output both levels)
  // If ARGC==2, ARGV[1] is the start directory for casce.exe,
  // If ARGC==1 or 2, the file Cas.txt is read as before, output
  // written to Out.txt, but unlike before the session is archived and
  // unarchived with the name xcas.ar.
  if (ARGC<2 || ARGC==3){
    signal(SIGINT,giac::ctrl_c_signal_handler);
    giac::child_id=1;
    string session_name="xcas.ar";
    ofstream logfile("log.txt");
    context contexte;
    contexte.globalptr->_logptr_=&logfile;
    context * contextptr =0;
    logfile << "Called by " << ARGC << " ";
    for (int i=0;i<ARGC;++i)
      logfile << ARGV[i] << " ";
    logfile << endl;
    int session_level=-1;
    gen replace_by;
    if (ARGC>=3){
      chdir(ARGV[1]);
      giac::language(atoi(ARGV[2]),0);
      if (ARGC>=4)
	session_name=ARGV[3];
      if (ARGC>=5)
	session_level=atoi(ARGV[4]);
      if (ARGC==5 || ARGC>6){
	// Unarchive session and remove all levels after session_level
	unarchive_session(session_name,-1,0,contextptr);
	int s=min(history_in(contextptr).size(),history_out(contextptr).size());
	if (session_level>=0 && session_level<s){
	  history_in(contextptr)=vecteur(history_in(contextptr).begin(),history_in(contextptr).begin()+session_level);
	  history_out(contextptr)=vecteur(history_out(contextptr).begin(),history_out(contextptr).begin()+session_level);
	}	  
	if (ARGC==5){
	  archive_session(true,session_name,contextptr);
	  return 0;
	}
      }
      if (ARGC==6){ 
	// Replace exactly one history level and recompute other levels
	replace_by=gen(ARGV[5]);
	unarchive_session(session_name,session_level,replace_by,contextptr);
	archive_session(true,session_name,contextptr);
	int s=history_out(contextptr).size();
	ofstream out("Out.txt");
	for (int i=max(session_level,0);i<s;++i){
	  out << history_out(contextptr)[i] << '¤' << ' ' << char(10) << char(13) ;;
	}
	return 0;
      }
      if (ARGC>6){
	// Session is unarchived, history levels have been removed
	// Evaluate starting from ARGV[5]
	ofstream out("Out.txt");
	for (int i=5;i<ARGC;++i){
	  gen tmp=string(ARGV[i]);
	  history_in(contextptr).push_back(tmp);
	  tmp=giac::protecteval(tmp,giac::DEFAULT_EVAL_LEVEL,contextptr);
	  history_out(contextptr).push_back(tmp);
	  out << tmp << '¤' << ' ' << char(10) << char(13) ;
	}
	archive_session(true,session_name,contextptr);
	return 0;
      }
    }
    else
      chdir(giac::remove_filename(ARGV[0]).c_str());
    // Unarchive previous session
    logfile << "Unarchive " << session_name << " " << session_level << " " << replace_by << endl;
    unarchive_session(session_name,session_level,replace_by,contextptr);
    logfile << "After unarchive " << history_in(contextptr) << " " << history_out(contextptr) << endl;
    FILE * stream = fopen("gnuplot.txt","w");
    fputc(' ',stream);
    fflush(stream);
    fclose(stream);
    ifstream in("Cas.txt");
    ofstream out("Out.txt");
    giac::vecteur args,l;
    giac::readargs_from_stream(in,args,l);
    logfile << "Read " << args << endl;
    clock_t start, end;  
    int s=args.size();
    for (int i=0;i<s;++i){
      out << "// " << args[i] << ' ' << char(10) << char(13) ;
      start = clock();
      giac::gen tmp=giac::_simplifier(giac::protecteval(args[i],giac::DEFAULT_EVAL_LEVEL,contextptr));
      end = clock();
      out << tmp << '¤' <<' ' << char(10) << char(13) ;
      history_in(contextptr).push_back(args[i]);
      history_out(contextptr).push_back(tmp);
      // out << "// Time " << double(end-start)/CLOCKS_PER_SEC << char(10) << char(13)  << char(10) << char(13) ;
    }
    archive_session(true,session_name,contextptr);
    return 0;
  }
  if (ARGC==2 && (string(ARGV[1])=="-v" || string(ARGV[1])=="--version" ) ){
    cout << VERSION << endl;
    return 0;
  }
  return 0;
}
