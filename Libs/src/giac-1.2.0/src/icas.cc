// -*- mode:C++ ; compile-command: "g++-3.4 -I.. -I../include -g -c icas.cc" -*-
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "first.h"

/*
 *  Copyright (C) 2000,2014 B. Parisse, Institut Fourier, 38402 St Martin d'Heres
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
#include <stdio.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
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
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
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
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif


// using namespace giac;
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

#ifdef HAVE_EQASCII
extern "C" {
typedef struct
  {
    int x;			/* width */
    int y;			/* heigth */
    int baseline;		/* default line for single characters
				   counting from down */
  }
Tdim;

struct Tgraph;
struct Tgraph		/* the order of fields is important--see Tarray */
  {
    struct Tgraph **down;	/* downnodes for sequential chilren */
    Tdim dim;			/* dimensions of this field */
    int children;		/* number of children */
    struct Tgraph *up;		/* upnode */
    char *txt;			/* the text. #1 points for a child */
  };

Tdim dim (char *txt, struct Tgraph *graph);
char **draw (struct Tgraph *graph);
char *preparse (char *txt);
void dealloc(struct Tgraph *graph); 
}
#endif // HAVE_EQASCII

#include "Xcas1.h"
#include "Cfg.h"

/* TeXmacs interface 
   See /usr/share/TeXmacs examples/plugins and progs for ex. of scheme commands
   Run texmacs -x '(make-session "giac" "default")' to start automatically
   giac at texmacs beginning
 */

#define TEXMACS_DATA_BEGIN   ((char) 2)
#define TEXMACS_DATA_END     ((char) 5)
#define TEXMACS_DATA_ESCAPE  ((char) 27)
#define TEXMACS_DATA_COMMAND ((char) 16)

#define EMACS_ESCAPE   ((char) 8)
#define EMACS_DATA_BEGIN   ((char) 6)
#define EMACS_DATA_END   ((char) 7)
#define EMACS_QUESTION ((char) 1)
#define EMACS_RESULT ((char) 2)
#define EMACS_ERROR ((char) 9)
#define EMACS_INLINE_HELP ((char) 8)
#define EMACS_PROMPT ((char) 13)
#define EMACS_ASK_COMPLETION ((char) 31)
#define EMACS_COMPLETION ((char) 32)
#define EMACS_END_COMPLETION ((char) 33)

static int texmacs_counter= 0;


#ifdef VISUALC
using namespace giac;
#define STDIN_FILENO 0
namespace xcas {
  void icas_eval(giac::gen & g,giac::gen & gg,int & reading_file,std::string &filename,giac::context * contextptr){
    gg=protecteval(g,10,contextptr);
  }

  bool fltk_view(const giac::gen & g,giac::gen & ge,const std::string & filename,std::string & figure_filename,int file_type,const giac::context *contextptr){
    return false;
  }
}

#else
void ctrl_c_signal_handler(int signum){
  giac::ctrl_c=true;
  cerr << "icas/giac process " << getpid() << ", Ctrl-C pressed, interruption requested" << endl;
}
#endif

void format_plugin () {
  // The configuration of a plugin can be completed at startup time.
  // This is for instance interesting if you add tab-completion a posteriori.
  cout << TEXMACS_DATA_BEGIN << "command:";
  cout << "(plugin-configure complete (:tab-completion #t))";
  cout << TEXMACS_DATA_END;
}

void flush_stdout(){
#ifdef NSPIRE_NEWLIB
  giac::usleep(2000);
#else
  usleep(2000);
#endif
  fflush (stdout);
}

void flush_stderr(){
#ifdef NSPIRE_NEWLIB
  giac::usleep(2000);
#else
  usleep(2000);
#endif
  fflush (stderr);
}

void texmacs_next_input () {
  putchar(TEXMACS_DATA_BEGIN);
  printf("prompt#");
  // printf("quest(%i) ",texmacs_counter);
  printf("> ");
  putchar(TEXMACS_DATA_END);
  flush_stdout();
}

void ifstream_output(istream & tmpif){
  flush_stdout();
  putchar(TEXMACS_DATA_BEGIN);
  printf("ps:");
  char c;
  for (int j=1;!tmpif.eof();++j){
    tmpif.get(c);
    putchar(c);
    if (!(j%1024))
      flush_stdout();
  }
  putchar('\n');
  putchar(TEXMACS_DATA_END);
  flush_stdout();
}



void texmacs_graph_output(const giac::gen & g,giac::gen & gg,std::string & figfilename,int file_type,const giac::context * contextptr){
  if (!xcas::fltk_view(g,gg,"casgraph.eps",figfilename,file_type,contextptr)){
    putchar(TEXMACS_DATA_BEGIN);
    printf("verbatim: Plot cancelled or unable to plot\n");
    putchar(TEXMACS_DATA_END);
    flush_stdout();
    return;
  }
  if (0 
      && figfilename.empty()
      ){
    putchar(TEXMACS_DATA_BEGIN);
    if (gg.is_symb_of_sommet(giac::at_program))
      printf("verbatim: %s\n",gg.print().c_str());
    else {
      if (gg.type==giac::_STRNG)
	printf("verbatim: %s\n",gg._STRNGptr->c_str());
      else 
	printf("latex:\\[ %s \\]",giac::gen2tex(gg,giac::context0).c_str());
    }
  }
  else {
    putchar(TEXMACS_DATA_END);
    // putchar(TEXMACS_DATA_END);
    flush_stdout();
    // putchar(TEXMACS_DATA_BEGIN);
    // printf("output#");
    putchar(TEXMACS_DATA_BEGIN);
    printf("verbatim:");
    ifstream tmpif("casgraph.eps");
    ifstream_output(tmpif);
    putchar(TEXMACS_DATA_END);
    // putchar(TEXMACS_DATA_END);
    flush_stdout();
    // ofstream log("log");
    // log << g << endl;
  }
}

void texmacs_output(const giac::gen & g,giac::gen & gg,bool reading_file,int no,const giac::context * contextptr){
  giac::history_in(contextptr).push_back(g);
  giac::history_out(contextptr).push_back(gg);
  if (reading_file){
    putchar(TEXMACS_DATA_BEGIN);
    printf("verbatim: %s\n",g.print().c_str());
    putchar(TEXMACS_DATA_END);
    // putchar(TEXMACS_DATA_END);
    flush_stdout();
  }
  int graph_output=graph_output_type(gg);
  if (graph_output){
    string filename="";
    texmacs_graph_output(g,gg,filename,0,contextptr);
    return;
  }
  if (reading_file && gg.is_symb_of_sommet(giac::at_program))
     return; 
  if (g.is_symb_of_sommet(giac::at_nodisp))
    return;
  // putchar(TEXMACS_DATA_BEGIN);
  // printf("output#");
  putchar(TEXMACS_DATA_BEGIN);
  if (gg.is_symb_of_sommet(giac::at_program))
    printf("verbatim: %s\n",gg.print().c_str());
  else {
    if (gg.type==giac::_STRNG)
      printf("verbatim: %s\n",gg._STRNGptr->c_str());
    else 
      printf("latex:\\[ %s \\]",giac::gen2tex(gg,giac::context0).c_str());
  }
  putchar(TEXMACS_DATA_END);
  // putchar(TEXMACS_DATA_END);
  flush_stdout();
}

void check_browser_help(const giac::gen & g){
  if (g.is_symb_of_sommet(giac::at_findhelp)){
    giac::gen f=g._SYMBptr->feuille;
    string s;
    if (f.type==giac::_SYMB)
      f=f._SYMBptr->sommet;
    if (f.type==giac::_FUNC)
      s=f._FUNCptr->ptr()->s;
#if !defined EMCC && !defined NSPIRE_NEWLIB
    giac::html_vtt=giac::html_help(giac::html_mtt,s);
#ifndef HAVE_NO_SYSTEM
    if (!giac::html_vtt.empty())
      giac::system_browser_command(giac::html_vtt.front());
#endif
#endif
  }
}


int main(int ARGC, char *ARGV[]){    
  //giac::step_infolevel=1;
  cerr << "// Maximum number of parallel threads " << giac::threads << endl;
  giac::context giac_context;
  giac::context * contextptr = 
    //  (giac::context *) giac::context0 ; 
    &giac_context;
#if !defined EMCC && !defined NSPIRE_NEWLIB
  giac::xcasroot()=giac::xcasroot_dir(ARGV[0]);
#endif
#ifndef VISUALC
  signal(SIGINT,ctrl_c_signal_handler);
#endif
  //cerr << giac::remove_filename(ARGV[0]) << endl;
#ifdef HAVE_LIBGSL
  gsl_set_error_handler_off();
#endif
#ifdef HAVE_LIBFLTK
#if !defined(__APPLE__) && !defined(WIN32)
  if (getenv("DISPLAY"))
#endif
    Fl::gl_visual(FL_RGB | FL_DEPTH | FL_ACCUM | FL_ALPHA);
#endif
  giac::secure_run=false;
#if !defined EMCC && !defined NSPIRE_NEWLIB
  if (ARGC==2 && !strcmp(ARGV[1],"--rebuild-help-cache")){
    for (int i=0;i<=4;++i)
      giac::html_help_init("aide_cas",i,true,true);
    return 0;
  }
#endif
  giac::gnuplot_ymin=-2.4;
  giac::gnuplot_ymax=2.1;
#ifdef GNUWINCE
  if (ARGC<2 || ARGC==3){
    giac::child_id=1;
    // chdir("My Documents");
    if (ARGC==3){
      chdir(ARGV[1]);
      giac::language(atoi(ARGV[2]),contextptr);
    }
    else
      chdir(giac::remove_filename(ARGV[0]).c_str());
    FILE * stream = fopen("gnuplot.txt","w");
    fputc(' ',stream);
    fflush(stream);
    fclose(stream);
    ifstream in("Cas.txt");
    ofstream out("Out.txt");
    giac::outptr=&out;
    giac::vecteur args;
    giac::readargs_from_stream(in,args);
    clock_t start, end;  
    int s=args.size();
    for (int i=0;i<s;++i){
      out << "// " << args[i] << ' ' << char(10) << char(13) ;
      start = clock();
      giac::gen tmp=giac::_simplifier(giac::protecteval(args[i],giac::DEFAULT_EVAL_LEVEL,contextptr),contextptr);
      end = clock();
      out << tmp.print(contextptr) << ' ' << char(10) << char(13) ;
      // out << "// Time " << double(end-start)/CLOCKS_PER_SEC << char(10) << char(13)  << char(10) << char(13) ;
    }
    return 0;
  }
#endif
  if (ARGC==2 && (string(ARGV[1])=="-v" || string(ARGV[1])=="--version" ) ){
    cout << VERSION << endl;
#ifndef GNUWINCE
    return 0;
#endif
  }
  //signal(SIGUSR1,data_signal_handler);
  //signal(SIGUSR2,plot_signal_handler);
  giac::child_id=1;
  bool intexmacs=((ARGC>=2) && std::string(ARGV[1])=="--texmacs");
  bool inemacs=((ARGC>=2) && std::string(ARGV[1])=="--emacs");
  bool insage=((ARGC>=2) && std::string(ARGV[1])=="--sage");
  if (inemacs){
    putchar(EMACS_DATA_BEGIN);
    putchar(EMACS_ERROR);
  }
  if (insage)
    ARGC=1;
  // Config
  bool show_time=true,show_tex=false;
  giac::read_env(contextptr,!inemacs && !intexmacs);
  int savedbg=giac::debug_infolevel;
  giac::protected_read_config(contextptr,false);
  if (savedbg)
    giac::debug_infolevel=savedbg;
  // Help and completion
  int helpitems;
#ifdef STATIC_BUILTIN_LEXER_FUNCTIONS
  if (giac::debug_infolevel==-2){
#endif
    giac::readhelp((*giac::vector_aide_ptr()),"aide_cas",helpitems,false);
    if (!helpitems){
      if (getenv("XCAS_HELP"))
	giac::readhelp((*giac::vector_aide_ptr()),getenv("XCAS_HELP"),helpitems,true);
      else
	giac::readhelp((*giac::vector_aide_ptr()),(giac::giac_aide_dir()+"aide_cas").c_str(),helpitems,true);
    }
#ifdef STATIC_BUILTIN_LEXER_FUNCTIONS
  }
#endif
  if (giac::debug_infolevel){
    if (!helpitems)
      cerr << "Unable to open help file aide_cas" << endl;
    else
      cerr << "Registered " << helpitems << " commands" << endl;
  }
  giac::set_language(giac::language(contextptr),contextptr);

  /* *******
   * EMACS *
   *********/
  // #define EMACS_DEBUG 1
#if !defined EMCC && !defined NSPIRE_NEWLIB
  if (inemacs){
    giac::html_help_init(ARGV[0],false);
    int out_handle;
#ifdef WITH_GNUPLOT
    giac::run_gnuplot(out_handle);
#endif
    putchar(EMACS_DATA_END);
    putchar(EMACS_DATA_BEGIN);
    putchar(EMACS_RESULT);
    printf("Giac CAS for mupacs, released under the GPL license 3.0\n");
    printf("See http://www.gnu.org for license details\n");
    printf("May contain BSD licensed software parts (lapack, atlas, tinymt)\n");
    printf("| (c) 2006, 2015 B. Parisse & al (giac), F.Maltey & al (mupacs) |\n");
    putchar(EMACS_DATA_END);
    bool prompt=true;
    for (int k=0;;++k) {
      if (prompt){
	putchar(EMACS_DATA_BEGIN);
	putchar(EMACS_PROMPT);
	printf(">> ");
	putchar(EMACS_DATA_END);
      }
      else
	prompt=true;
      string buffer;
      char car;//,nxt;
#ifdef EMACS_DEBUG
      ofstream logfile(("log"+giac::print_INT_(k)).c_str());
#endif
      for (;;){
	int i=getchar();
	car=i;
#ifdef EMACS_DEBUG
	if (i<32)
	  logfile << "Ctrl-" << i << endl;
	else
	  logfile << car << endl;
#endif
	if (i==EOF)
	  break;
	if (i==EMACS_DATA_END){
	  buffer = buffer + '\n';
	  break;
	}
	buffer=buffer+car;
      }
      // end read buffer
      int s=buffer.size();
      if (s<=2)
	continue;
      if (buffer[0]==10){
	--s;
	buffer=buffer.substr(1,s);
	if (s<=2)
	  continue;
      }
      if (buffer[0]==EMACS_DATA_BEGIN){
	char cmd=buffer[1];
	s -= 2;
	buffer=buffer.substr(2,s);
	if (s && buffer[s-1]=='\n'){
	  --s;
	  buffer=buffer.substr(0,s);
	}
#ifdef EMACS_DEBUG
	logfile << buffer << " " << s << " " << int(buffer[buffer.size()-1]) <<endl;
#endif
	if (cmd==EMACS_ASK_COMPLETION){
	  // reading possible completions from aide_cas
	  vector<string> vres;
	  for (unsigned k=0;k<(*giac::vector_completions_ptr()).size();++k){
	    if ((*giac::vector_completions_ptr())[k].substr(0,s)==buffer){
	      vres.push_back((*giac::vector_completions_ptr())[k]);
	    }
	  }
	  // add global names
	  giac::gen gv(giac::_VARS(giac::zero,contextptr));
	  if (gv.type==giac::_VECT){
	    giac::vecteur & vv =*gv._VECTptr;
	    for (unsigned k=0;k<vv.size();++k){
	      if (vv[k].print(contextptr).substr(0,s)==buffer){
		vres.push_back(vv[k].print(contextptr));
	      }
	    }
	  }
	  string common;
	  int l=s;
	  int vs=vres.size();
#ifdef EMACS_DEBUG
	  logfile << vs << " completions" << endl;
#endif
	  for (int k=0;k<vs;k++){
	    if (common.empty()){
	      common=vres[k];
	      l=common.size();
	    }
	    else {
	      int maxl=min(common.size(),vres[k].size());
#ifdef EMACS_DEBUG
	      logfile << maxl << " " << common << " " << vres[k] << endl;
#endif
	      for (l=s;l<maxl;++l){
		if (common[l]!=vres[k][l])
		  break;
	      }
	      common=common.substr(0,l);
	    }
	  }
	  putchar(EMACS_DATA_BEGIN);
	  putchar(EMACS_COMPLETION);
	  if (vs!=1){
	    if (vs==0){
	      putchar(EMACS_ESCAPE);
	      putchar(EMACS_DATA_END);
	    }
	    for (int k=0;k<vs;k++){
	      printf("%s",vres[k].c_str());
	      if (k!=vs-1)
		printf(", ");
	    }
	  }
	  putchar(EMACS_DATA_END);
	  putchar(EMACS_DATA_BEGIN);
	  putchar(EMACS_END_COMPLETION);
	  // common completion
#ifdef EMACS_DEBUG
	  logfile << common << " " << l << " " << s;
#endif
	  if (l>s)
	    printf("%s",common.substr(s,l-s).c_str());
	  putchar(EMACS_DATA_END);
	  prompt=false;
	}
	if (cmd==EMACS_QUESTION){
	  if (buffer=="quit")
	    break;
	  if (buffer=="?")
	    buffer="?giac";
#ifdef HAVE_SIGNAL_H_OLD
	  giac::messages_to_print="";
#endif
	  giac::gen g(buffer,contextptr),gg;
	  
#ifdef HAVE_SIGNAL_H_OLD
	  if (giac::messages_to_print.size()>1){
	    putchar(EMACS_DATA_BEGIN);
	    putchar(EMACS_ERROR);
	    printf("%s\n",giac::messages_to_print.c_str());
	    putchar(EMACS_DATA_END);
	  }
#endif
	  check_browser_help(g);
	  putchar(EMACS_DATA_BEGIN);
	  putchar(EMACS_RESULT);
	  // printf("%s\n",giac::messages_to_print.c_str());

	  // GEO SETUP?
	  if ( (g.type==giac::_SYMB) && (g._SYMBptr->sommet==giac::at_xyztrange) && (g._SYMBptr->feuille.type==giac::_VECT) && (g._SYMBptr->feuille._VECTptr->size()<12) ){
#ifdef HAVE_LIBFLTK
	    xcas::Xcas_load_graph_setup(contextptr);
	    Fl::run();
	    Fl::wait(0.001);
#endif
	    giac::history_in(contextptr).push_back(g);
	    giac::history_out(contextptr).push_back(gg);
	    printf("Done\n");
	    putchar(EMACS_DATA_END); // end answer
	    continue;
	  }

	  int reading_file=0;
	  std::string filename;
	  xcas::icas_eval(g,gg,reading_file,filename,contextptr);
	  if (reading_file>=1 || graph_output_type(gg))
	    printf(xcas::fltk_view(g,gg,"",filename,reading_file,contextptr)?"Done\n":"Plot cancelled or unable to plot\n");
	  else {
	    giac::history_in(contextptr).push_back(g);
	    giac::history_out(contextptr).push_back(gg);
	    printf("%s\n",gg.print(contextptr).c_str());
	  }
	  putchar(EMACS_DATA_END);
	}
      } // end normal command
    } // end while(1)
    return 0;
  } // if (inemacs)
  /* END EMACS */

  /* *********************************************************
     *                       BEGIN TEXMACS                  *
     ********************************************************* */
  if ( intexmacs){
    giac::html_help_init(ARGV[0],false);
    int out_handle;
#ifdef WITH_GNUPLOT
    giac::run_gnuplot(out_handle);
#endif
    putchar(TEXMACS_DATA_BEGIN);
    printf("verbatim:");
    format_plugin();
    printf("--------------------------------------------------------------------\n");
    printf("|     Giac CAS for TeXmacs, released under the GPL license (3.0)    |\n");
    printf("|     See http://www.gnu.org for license details                    |\n");
    printf("|  May contain BSD licensed software parts (lapack, atlas, tinymt)  |\n");
    printf("| (c) 2003,2015 B. Parisse & al (giac), J. van der Hoeven (TeXmacs) |\n");
    printf("--------------------------------------------------------------------\n");
    switch (giac::xcas_mode(contextptr)){
    case 0:
      printf("Xcas (C-like) syntax mode\n");
      break;
    case 1:
      printf("Maple syntax mode\n");
      break;
    case 2:
      printf("MuPAD syntax mode\n");
      break;
    case 3:
      printf("TI89/92 syntax mode\n");
    } 
    printf("Type ? for documentation or ?commandname for help on commandname\n");
    printf("Type tabulation key to complete a partial command\n");
    putchar(TEXMACS_DATA_END);
    texmacs_next_input();
    while (1) {      
      string buffer;
      char car;//,nxt;
      for (;;){
	int i=getchar();
	// cerr << i << endl;
	if (i==EOF)
	  break;
	car=i;
	if (car=='\n'){
#if !defined VISUALC && !defined __MINGW_H
	  giac::set_nonblock_flag(STDIN_FILENO,1); // set non blocking mode on stdin
#endif
	  usleep(5000);
	  // ssize_t s=read(STDIN_FILENO,&nxt,1);
	  i=getchar();
#if !defined VISUALC && !defined __MINGW_H
	  giac::set_nonblock_flag(STDIN_FILENO,0); // set blocking mode on stdin
#endif
	  // cerr << "read "  << s << endl;
	  if (i==EOF)
	    break;
	  buffer += car;
	  car = i;
	  buffer += car;
	}
	else
	  buffer +=car;
      }
      // end read buffer
      if (buffer[0]==TEXMACS_DATA_COMMAND){
	int bs=buffer.size();
	--bs;
	buffer=buffer.substr(1,bs);
	int pos=buffer.find(' ');
	if (pos>0 && pos<bs-1){
	  bs = bs-pos-1;
	  buffer=buffer.substr(pos+1,bs);
	  pos=buffer.find(' ');
	  string complete_string=buffer.substr(1,pos-2);
	  // search non ascii char starting from the end
	  int ss=complete_string.size(),cs=ss;
	  string partial_string;
	  for (int i=ss-1;i>=0;--i){
	    if (giac::isalphan(complete_string[i]))
	      ;
	    else
	      break;
	    partial_string=complete_string[i]+partial_string;
	  }
	  ss=partial_string.size();
	  // remove beginning part
	  buffer=buffer.substr(pos+1,bs-pos-2);
	  int n=atoi(buffer.c_str());
	  string cmd("(tuple ");
	  cmd += '"'+complete_string.substr(0,n)+'"' ;
	  n=n-(cs-ss);
	  if (n>=0 && n<=cs){
	    // reading possible completions from aide_cas
	    vector<string> vres;
	    string res=partial_string.substr(0,n);
	    for (unsigned k=0;k<(*giac::vector_completions_ptr()).size();++k){
	      if ((*giac::vector_completions_ptr())[k].substr(0,n)==res){
		vres.push_back((*giac::vector_completions_ptr())[k]);
	      }
	    }
	    int vs=vres.size();
	    for (int k=0;k<vs;k++){
	      string & tmp=vres[k];
	      cmd += " \""+tmp.substr(n,tmp.size()-n)+'"';
	    }
	    putchar(TEXMACS_DATA_BEGIN);
	    printf("scheme:%s)",cmd.c_str());
	    putchar(TEXMACS_DATA_END);
	    flush_stdout();
	  } 
	  else {
	    putchar(TEXMACS_DATA_BEGIN);
	    printf("scheme:(tuple \"%s\" \"\")",complete_string.c_str());
	    putchar(TEXMACS_DATA_END);
	    flush_stdout();
	  }
	}
	else {
	  // should not happen
	}
	buffer="";
	continue;
      }
      if ( buffer=="quit") 
	break; // end of session
      // Begin answer
      putchar( TEXMACS_DATA_BEGIN); 
      printf("verbatim:");
      // ONLINE HELP
      if (buffer=="?"){
#ifndef HAVE_NO_SYSTEM
	giac::system_browser_command("doc/index.html");
#endif
	buffer="?giac";
      }
#ifdef HAVE_SIGNAL_H_OLD
      giac::messages_to_print="";
#endif
      giac::gen g(buffer,contextptr),gg;
#ifdef HAVE_SIGNAL_H_OLD
      printf("%s\n",giac::messages_to_print.c_str());
#endif
      check_browser_help(g);
      // END ONLINE HELP
      // GEO SETUP
      if ( (g.type==giac::_SYMB) && (g._SYMBptr->sommet==giac::at_xyztrange) && (g._SYMBptr->feuille.type==giac::_VECT) && (g._SYMBptr->feuille._VECTptr->size()<12) ){
#ifdef HAVE_LIBFLTK
	xcas::Xcas_load_graph_setup(contextptr);
	Fl::run();
	Fl::wait(0.001);
#endif
	giac::history_in(contextptr).push_back(g);
	giac::history_out(contextptr).push_back(gg);
	printf("Done\n");
	putchar(TEXMACS_DATA_BEGIN); 
	printf("latex:");
	printf("x_-=%.4f,x_+=%.4f,y_-=%.4f,y_+=%.4f",giac::gnuplot_xmin,giac::gnuplot_xmax,giac::gnuplot_ymin,giac::gnuplot_ymax);
	putchar(TEXMACS_DATA_END); 
	texmacs_counter++;
	putchar(TEXMACS_DATA_END); // end answer
	texmacs_next_input();
	continue;
      }
      // END GEO SETUP
      int reading_file=0;
      std::string filename;
      xcas::icas_eval(g,gg,reading_file,filename,contextptr);
      bool done=false;
      if (reading_file){ 
	if (reading_file>=2 || (giac::ckmatrix(gg,true)&&gg.subtype==giac::_SPREAD__VECT) ){
	  texmacs_graph_output(g,gg,filename,reading_file,contextptr);
	  done=true;
	}
	if (!done && g.type==giac::_VECT && gg.type==giac::_VECT){
	  giac::vecteur & gv=*g._VECTptr;
	  giac::vecteur & ggv=*gg._VECTptr;
	  // check if ggv last element is geometric -> geo2d or geo3d
	  int ggs=ggv.size();
	  if (ggs && (graph_output_type(ggv[ggs-1])) ){
	    texmacs_graph_output(g,gg,filename,reading_file,contextptr);
	    done=true;
	  }
	  // output each component of the vector gg 1 by 1
	  int s=min(gv.size(),ggv.size());
	  for (int j=0;!done && j<s;++j){
	    texmacs_output(gv[j],ggv[j],reading_file,texmacs_counter,contextptr);
	    texmacs_counter++;
	  }
	  done = true;
	}
      }
      if (!done) {
#ifdef HAVE_LIBFLTK
	if (g.type==giac::_SYMB && gg.is_symb_of_sommet(giac::at_pnt) && gg.subtype>=0 && equalposcomp(giac::implicittex_plot_sommets,g._SYMBptr->sommet))
	  ;// set_view_point(0,0); // FIXME
#endif
	texmacs_output(g,gg,false,0,contextptr);
	texmacs_counter++;
      }
      putchar(TEXMACS_DATA_END); // end answer
      texmacs_next_input();
    }
#ifdef WITH_GNUPLOT
    if (giac::has_gnuplot)
      giac::kill_gnuplot();
#endif
    return 0;
  }
  /* *********************************************************
     *                       END OF TEXMACS                  *
     ********************************************************* */
#endif // EMCC not defined

  if (insage || getenv("GIAC_NO_TIME"))
    show_time=false;
  if (getenv("GIAC_TIME"))
    show_time=true;
  if (getenv("GIAC_TEX")){
    cerr << "// Setting tex log" << endl;
    show_tex=true;
  }
#ifdef HAVE_LIBREADLINE
  if (ARGC==1){
    struct tms start, end;  
    using_history();
    cout << "Welcome to giac readline interface" << endl;
    cout << "(c) 2001,2015 B. Parisse & others" << endl;
    cout << "Homepage http://www-fourier.ujf-grenoble.fr/~parisse/giac.html" << endl;
    cout << "Released under the GPL license 3.0 or above" << endl;
    cout << "See http://www.gnu.org for license details" << endl;
    cout << "May contain BSD licensed software parts (lapack, atlas, tinymt)" << endl;
    cout << "-------------------------------------------------" << endl;
    cout << "Press CTRL and D simultaneously to finish session" << endl;
    cout << "Type ?commandname for help" << endl;
    for (int count=0;;++count) {
      char * res=rl_gets(count);
      if (!res)
	break;
      string s(res);
      int bs=s.size();
      if (insage && bs && s[bs-1]==63){
	string complete_string(s.substr(0,bs-1));
	// search non ascii char starting from the end
	int bs=complete_string.size();
	string partial_string;
	for (int i=bs-1;i>=0;--i){
	  if (giac::isalphan(complete_string[i]))
	    ;
	  else {
	    complete_string=complete_string.substr(i+1,bs-i-1);
	    break;
	  }
	}
	bs=complete_string.size();
	// reading pobsible completions from aide_cas
	vector<string> vres;
	for (unsigned k=0;k<(*giac::vector_completions_ptr()).size();++k){
	  if ((*giac::vector_completions_ptr())[k].substr(0,bs)==complete_string){
	    vres.push_back((*giac::vector_completions_ptr())[k]);
	  }
	}
	int vs=vres.size();
	for (int k=0;k<vs;k++){
	  string & tmp=vres[k];
	  printf("%s\n",tmp.c_str());
	}
	printf("%s","----\n");
	continue;
      }
      s += '\n';
#ifdef HAVE_SIGNAL_H_OLD
      giac::messages_to_print="";
#endif
      giac::gen gq(s,contextptr),ge;
      if (giac::first_error_line(contextptr)){
	cout << parser_error(contextptr);
      }
      giac::ctrl_c=false; giac::interrupted=false;
      int reading_file=0;
      std::string filename;
#ifdef __APPLE__
      unsigned startc=clock();
#endif
      times(&start);
      xcas::icas_eval(gq,ge,reading_file,filename,contextptr);
#ifdef __APPLE_
      startc=clock()-startc;
#endif
      times(&end);
      giac::history_in(contextptr).push_back(gq);
      giac::history_out(contextptr).push_back(ge);
      // 2-d plot?
      int graph_output=graph_output_type(ge);
      if (reading_file>=2 || graph_output || (giac::ckmatrix(ge,true) &&ge.subtype==giac::_SPREAD__VECT) ){
	if (xcas::fltk_view(gq,ge,"",filename,reading_file,contextptr))
	  cout << "Done";
	else
	  cout << "Plot cancelled or unable to plot";
      }
      else {
	int taillemax=1000;
	if (getenv("GIAC_TAILLEMAX"))
	  taillemax=atoi(getenv("GIAC_TAILLEMAX"));
	string s=taille(ge,taillemax)>taillemax?"Done":ge.print(contextptr);
	cout << s;
      }
      cout << endl;
#ifdef HAVE_SIGNAL_H_OLD
      cerr << giac::messages_to_print << endl;
#endif
      if (show_time){
#ifdef __APPLE__
	cerr << "// dclock1 " << double(startc)/CLOCKS_PER_SEC << endl;
#endif
	cerr << "// Time " << giac::delta_tms(start,end) << endl;
      }
#ifdef HAVE_EQASCII
      struct Tgraph *graph=(Tgraph *)malloc(sizeof(struct Tgraph));
      char **screen;
      int i, j;
      graph->up = 0;
      graph->down = 0;
      graph->children = 0;
      string ges(ge.print(contextptr));
      dim (preparse ((char *)ges.c_str()), graph);
      screen = draw (graph);
      for (i = 0; i < graph->dim.y; i++)
	{
	  for (j = 0; j < graph->dim.x; j++)
	    printf ("%c", screen[i][j]);
	  printf ("\n");
	}
      dealloc(graph); 
#endif // HAVE_EQASCII
    }
#ifdef WITH_GNUPLOT
    if (giac::has_gnuplot)
      giac::kill_gnuplot();
#endif
    return 0;
  }
#endif // HAVE_LIBREADLINE
  int command=0;
  bool showcommand=false;
  if (getenv("GIAC_SHOWCOMMAND"))
    showcommand=true;
  string s(ARGV[0]);
  // keep only what remains after the last / of s
  string t;
  giac::gen u;
  for (int i=s.size()-1;i>=0;--i){
    if (s[i]!='/')
      t=s[i]+t;
    else
      break;
  }
  if (t=="cas2tex"){
    command=-1;
    cout << giac::tex_preamble ;
  }
  else {
    if ( t=="icas" || t=="maplec" || t=="mupadc" || t=="giac" ){
      command=0;
      if (t=="maplec")
	giac::xcas_mode(contextptr)=1;
      if (t=="mupadc")
	giac::xcas_mode(contextptr)=2;
    }
    else {
      command=1;
      t = "'"+t+"'";
      u=giac::gen(t,contextptr);
      if (u.type!=giac::_FUNC)
	command=0;
    }
  }
  ofstream texlog("session.tex",ios::app);
  giac::vecteur v;
  giac::gen e;
#if defined VISUALC || defined __MINGW_H
  int f1,f2;
  string st;
#else
  timeval tt; // get time info for label initialization
  gettimeofday(&tt,0);
  giac::gen et((int) tt.tv_sec);
  string st(et.print(contextptr));
  struct tms start, end,f1,f2;  
  times(&start);
  f2=start;
#endif
  //xcas_mode(contextptr)=1;
  //rpn_mode=true;
#ifdef HAVE_SIGNAL_H_OLD
  giac::messages_to_print="";
#endif
  if (ARGC==2)
    giac::parser_filename(ARGV[1],contextptr);
  giac::readargs(ARGC,ARGV,v,contextptr);
#ifdef HAVE_SIGNAL_H_OLD
  cerr << giac::messages_to_print << endl;
  bool resultat=(giac::messages_to_print=="\n");
#else
  bool resultat=false;
#endif
  giac::vecteur::const_iterator it=v.begin(),itend=v.end();
  for (int i=0;it!=itend;++it,++i){
#ifdef HAVE_SIGNAL_H_OLD
    giac::messages_to_print = "";
#endif
    f1=f2;
    giac::gen gq(*it);
    giac::history_in(contextptr).push_back(gq);
    int reading_file=0;
    std::string filename;
    unsigned startc;
    if (command==-1){
      cout << "\\begin{equation} \\label{eq:d_" << st << "_" << i << "}" << endl;
      cout << giac::gen2tex(gq,contextptr)  ;
#ifdef __APPLE__
      startc=clock();
#endif
      xcas::icas_eval(gq,e,reading_file,filename,contextptr);
#ifdef __APPLE__
      startc=clock()-startc;
#endif
#ifdef HAVE_SIGNAL_H_OLD
      if (!giac::messages_to_print.empty())
	cerr << giac::messages_to_print << endl;
#endif
      if ((gq.type==giac::_SYMB) && (gq!=e))
	cout << " = " << giac::gen2tex(e,contextptr) ;
      cout << " \\end{equation} " << endl;
    }
    else {
      if (command>0)
	gq=giac::symbolic(*u._FUNCptr,gq);
#ifdef __APPLE__
      startc=clock();
#endif
      xcas::icas_eval(gq,e,reading_file,filename,contextptr);
#ifdef __APPLE__
      startc=clock()-startc;
#endif
#ifdef HAVE_SIGNAL_H_OLD
      if (!giac::messages_to_print.empty())
	cerr << giac::messages_to_print << endl;
#endif
      if (showcommand)
	cout << "// " << *it << endl;
      cout << e.print(contextptr) ;
      if (it+1!=itend){
	if ( (it->type!=giac::_SYMB) || (it->_SYMBptr->sommet!=giac::at_comment)) 
	  cout << "," <<endl ;
      }
      else
	cout << endl;
#if !defined VISUALC && ! defined __MINGW_H
      times(&f2);
      if (show_time){
#ifdef __APPLE__
	cerr << "// dclock2 " << double(startc)/CLOCKS_PER_SEC << endl;
#endif
	cerr << "// Time " << giac::delta_tms(f1,f2) << endl;
      }
#endif
      if (show_tex) { // append to session.tex
	texlog << "\\begin{equation} \\label{eq:d_" << st << "_" << i << "}" << endl;
	texlog << giac::gen2tex(gq,contextptr);
	if ((gq.type==giac::_SYMB) && (gq!=e))
	  texlog << " = " << giac::gen2tex(e,contextptr) ;
	texlog << " \\end{equation} " << endl;
      }
    } // end if (command==-1) else
    giac::history_out(contextptr).push_back(e); 
  }
  // cerr << messages_to_print << endl;
  // ofstream ans((string("ans")+giac::cas_suffixe).c_str());
  // ans << e << endl;
#if !defined VISUALC && !defined __MINGW_H
  times(&end);
  if (command==-1){
    cout << giac::tex_end << endl;
    cout << "% Generated by cas2tex in " << giac::delta_tms(start,end) << endl;
  }
  else {
    if (show_time)
      cerr << "// Total time " << giac::delta_tms(start,end) << endl;
  }
#endif
#ifdef WITH_GNUPLOT
  giac::kill_gnuplot();
#endif
  return resultat;
}
