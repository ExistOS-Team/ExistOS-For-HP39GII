#ifndef MAIN_H
#define MAIN_H
int ck_getkey(int * keyptr);
#include "giacPCH.h"
//#include "kdisplay.h"
extern giac::context * contextptr; // main.cc
bool eqws(char * s,bool eval); // from main.cc, s must be at least 512 char
giac::gen eqw(const giac::gen & ge,bool editable);
bool save_script(const char * filename,const ustl::string & s);
int run_script(const char* filename) ;
void erase_script();
void save(const char * fname); // save session
int restore_session(const char * fname);
void displaygraph(const giac::gen & ge);
void displaylogo();
void edit_script(const char * fname);
int load_script(const char * filename,string & s);
int find_color(const char * s); 
ustl::string khicas_state();
void do_run(const char * s,giac::gen & g,giac::gen & ge);
bool textedit(char * s);
void run(const char * s,int do_logo_graph_eqw=7);
void check_do_graph(giac::gen & ge,int do_logo_graph_eqw=7) ;
bool stringtodouble(const string & s1,double & d);


#endif
