// -*- mode:C++ ; compile-command: "g++-3.4 -I. -I.. -g -c Input.cc -Wall" -*-
/*
 *  Copyright (C) 2005,2014 B. Parisse, Institut Fourier, 38402 St Martin d'Heres
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
#ifndef IN_GIAC
#include <giac/first.h>
#else
#include "first.h"
#endif
#include <string>
#ifdef HAVE_LIBFLTK
#include "Input.h"
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Window.H>
#include <FL/fl_ask.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Tooltip.H>
#include "History.h"
#include "Xcas1.h"
#include "Tableur.h"
#include "Graph3d.h"
#include "Help1.h"
#ifndef IN_GIAC
#include <giac/plot.h>
#include <giac/help.h>
#include <giac/global.h>
#else
#include "plot.h"
#include "help.h"
#include "global.h"
#endif
#include <iostream>
#include <fstream>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
using namespace std;

#ifndef NO_NAMESPACE_XCAS
namespace xcas {
#endif // ndef NO_NAMESPACE_XCAS

#define TAB_ARGS 6

#if defined __APPLE__ || defined WIN32
  bool use_external_browser = true ;
#else
  bool use_external_browser = getenv("BROWSER") ;
#endif
  std::vector<std::string> Multiline_Input_tab::history;
  int Multiline_Input_tab::count=0;

  Fl_Help_Dialog * Xcas_help_window = new Fl_Help_Dialog();
  Fl_Widget * Xcas_input_focus=0;

  void system_browser(const string & s){
    int i=system(s.c_str());
    if (i!=0){
      fl_alert("%s",("Switching to internal browser, error running browser command "+s).c_str());
      use_external_browser=false;
    }
  }

  bool read_aide(const string & progname,int language){
    string helpfile("aide_cas");
    int helpitems=0;
    (*giac::vector_aide_ptr())=giac::readhelp(helpfile.c_str(),helpitems,false);
    if (!helpitems){
      if (getenv("XCAS_HELP"))
	helpfile=getenv("XCAS_HELP");
      else
	helpfile=giac::giac_aide_dir()+"aide_cas";
      (*giac::vector_aide_ptr())=giac::readhelp(helpfile.c_str(),helpitems);
    }
    if (!helpitems){
      cerr << "// Unable to open help file "<< helpfile << endl;
      return false;
    }
    else {
      cerr << "// Using help file " << helpfile << endl;
      giac::xcasroot()=giac::xcasroot_dir((char *) progname.c_str());
      /* patch for gsview TEMP, but does not work
      if (!getenv("TEMP")){
	if (giac::is_file_available("/tmp"))
	  setenv("TEMP","/tmp",1);
	else
	  setenv("TEMP",giac::xcasroot().c_str(),1);
      }
      */
      cerr << "// root dir " << giac::xcasroot() << endl;
      giac::html_help_init((char *) progname.c_str(),language);
      giac::update_completions();
      return true;
    }
  }

  // return the last keyword of s
  std::string motclef(const std::string & s){
    int l=s.size();
    int i=l-1;
    for (;i>=0;i--){
      if (giac::isalphan(s[i]))
	break;
    }
    l=i+1;
    for (;i>=0;i--){
      if (!giac::isalphan(s[i]))
	return s.substr(i+1,l-1-i);
    }
    return s.substr(0,l);
  }

  string findtooltip(const giac::gen & g){
    string s,s1,s2;
    static const char * tooltip_tab[]={"Integer","Expression","Variable","Matrix","Function","String","Polynom","Vector","Point","List","List of point","List of reals","List of integers","Sequence of variables","Command",};
    static const char * tooltip_name[]={"Intg","Expr","Var","Mtrx","Fnc","Str","Poly","Vect","Pnt","Lst","LstPnt","LstReal","LstIntg","SeqVar","Cmd",0};
    if (g.is_symb_of_sommet(giac::at_ou)){
      giac::gen & f = g._SYMBptr->feuille;
      if (f.type==giac::_VECT){
	giac::const_iterateur it=f._VECTptr->begin(),itend=f._VECTptr->end();
	if (it!=itend){
	  for (s=findtooltip(*it),++it;it!=itend;++it)
	    s = s + gettext(" or ") + findtooltip(*it);
	}
      }
      return s;
    }
    if (g.type==giac::_VECT && g._VECTptr->size()==1)
      return findtooltip(g._VECTptr->front())+"(optional)";
    s = g.print(giac::context0);
    int l=s.size();
    int p=s.find('(');
    if (p>0 && p<l){
      s1 = s.substr(0,p); s2=s.substr(p,l);
    }
    else s1=s;
    const char ** tooltip_ptr=tooltip_name;
    for (;*tooltip_ptr;++tooltip_ptr){
      if (*tooltip_ptr==s1){
	return tooltip_tab[tooltip_ptr-tooltip_name]+s2;
      }
    }
    return s;
  }

  std::string split(const std::string & s,Fl_Widget * w){
    fl_font(w->labelfont(),w->labelsize());
    string res,ajout;
    int fin=s.size(),debut=0;
    int taille=w->w();
    for (;debut<fin;){
      int l=fin-debut;
      ajout=s.substr(debut,l);
      if (fl_width(ajout.c_str())<=taille)
	return res+ajout;
      for (--l;l>0;--l){
	if (s[debut+l]==' '){
	  ajout=s.substr(debut,l);
	  if (fl_width(ajout.c_str())<=taille){
	    res += ajout;
	    res += '\n';
	    break;
	  }
	}
      }
      if (l==0)
	return res+ajout;
      debut += l+1;
    }
    return res;
  }

  void update_examples(const string & s,Fl_Browser * examples,Fl_Browser * related,Fl_Browser * syns,Fl_Output * output,Fl_Input ** argtab,int language){
    help_output(s,language);
    if (output->label())
      delete output->label();
    char * ptr=new char [s.size()+1];
    strcpy(ptr,s.c_str());
    output->label(ptr);
    if (output->parent())
      output->parent()->redraw();
    if (examples && output ){
      giac::aide cur_aide=giac::helpon(s,(*giac::vector_aide_ptr()),language,(*giac::vector_aide_ptr()).size(),false);
      string result=cur_aide.cmd_name;
      if (!cur_aide.syntax.empty()){
	result=result+"("+cur_aide.syntax+")\n";
	giac::gen helpg;
	giac::vecteur helpv;
	if (!cur_aide.syntax.empty()){
	  helpg=giac::gen(cur_aide.syntax,giac::context0);
	  if (helpg.type==giac::_VECT)
	    helpv=*helpg._VECTptr;
	  else
	    helpv=giac::vecteur(1,helpg);
	}
	giac::const_iterateur jt=helpv.begin(),jtend=helpv.end();
	Fl_Input ** ptr=argtab, ** argtabend=argtab+TAB_ARGS;
	if (jtend-jt>TAB_ARGS)
	  jtend=jt+TAB_ARGS;
	for (;*ptr && jt!=jtend;++jt,++ptr){
	  giac::gen tmp=*jt;
	  string tmps=tmp.print(giac::context0);
	  if (tmp.type==giac::_VECT)
	    (*ptr)->labelcolor(FL_BLUE);
	  else 
	    (*ptr)->labelcolor(FL_BLACK);
	  if ((*ptr)->label())
	    free((void *) (*ptr)->label());
	  char * chartab = (char *) malloc(sizeof(char)*(tmps.size()+1));
	  strcpy(chartab,tmps.c_str());
	  (*ptr)->label(chartab);
	  (*ptr)->show();
	  (*ptr)->value("");
	  tmps = findtooltip(tmp);
	  if ((*ptr)->tooltip())
	    free((void *) (*ptr)->tooltip());
	  char * chartab2 = (char *) malloc(sizeof(char)*(tmps.size()+1));
	  strcpy(chartab2,tmps.c_str());
	  (*ptr)->tooltip(chartab2);
	}
	int L=output->labelsize()+2;
	int eh=L*((1+(ptr-argtab))/2);
	int outputyh = output->y()+output->h();
	examples->resize(examples->x(),outputyh+eh,examples->w(),examples->parent()->h()-outputyh-eh-2);
	for (;ptr!=argtabend;++ptr){
	  if (*ptr)
	    (*ptr)->hide();
	}
      }
      vector<giac::localized_string>::const_iterator it=cur_aide.blabla.begin(),itend=cur_aide.blabla.end();
      for (;it!=itend;++it){
	if (it->language==language){
	  result = split(it->chaine,output) +'\n'+result ;
	  break;
	}
      }
      output->value(result.c_str());
      examples->clear();
      examples->add(s.c_str());
      std::vector<std::string>::const_iterator jt=cur_aide.examples.begin(),jtend=cur_aide.examples.end();
      for (;jt!=jtend;++jt)
	examples->add(jt->c_str());
      related->clear();
      syns->clear();
      std::vector<giac::indexed_string>::const_iterator kt=cur_aide.related.begin(),ktend=cur_aide.related.end();
      for (;kt!=ktend;++kt){
	string tmp=giac::localize(kt->chaine,language);
	related->add(tmp.c_str());
      }
      std::vector<giac::localized_string>::const_iterator lt=cur_aide.synonymes.begin(),ltend=cur_aide.synonymes.end();
      for (;lt!=ltend;++lt){
	if (lt->chaine!=s)
	  syns->add(lt->chaine.c_str());
      }
    }
  }

  void handle_tab_cb_browser(Fl_Browser * b,void *){
    int k=b->value();
    if (k>=1){
      string s=b->text(k);
      // find examples browser
      Fl_Group * g = b->parent();
      Fl_Browser * examples=0, * related=0,*syns=0;
      Fl_Output * output=0;
      Fl_Input * input=0,*argtab[TAB_ARGS]={0,0,0,0,0,0};
      if (g){
	int n=g->children();
	for (int i=0;i<n-1;i++){
	  if (i>4 && (output=dynamic_cast<Fl_Output *>(g->child(i)))){
	    related=dynamic_cast<Fl_Browser *>(g->child(i-4));
	    syns=dynamic_cast<Fl_Browser *>(g->child(i-3));
	    examples=dynamic_cast<Fl_Browser *>(g->child(i+7));
	    input=dynamic_cast<Fl_Input *>(g->child(i-1));
	    for (int k=0;k<TAB_ARGS;k++){
	      argtab[k] = dynamic_cast<Fl_Input *>(g->child(i+1+k));
	    }
	    break;
	  }
	}
      }
      if (output && input && examples && related && syns){
	update_examples(s,examples,related,syns,output,argtab,giac::language(giac::context0));
	input->value(b->text(k));
	if (Fl::event_clicks()){ 
	  g->hide();
	}
      }
      else
	help_output(s,giac::language(giac::context0));
    }
  }

  void browser_html_help(Fl_Browser * b,Fl_Browser * examples,Fl_Browser * related,Fl_Browser * syns,Fl_Output * output,Fl_Input ** argtab,int language){
    int k=b->value();
    if (k>=1){
      if (Xcas_help_window){
	string s=b->text(k);
	update_examples(s,examples,related,syns,output,argtab,language);
	std::map<std::string,std::string>::const_iterator it=giac::lexer_localization_map().find(s),itend=giac::lexer_localization_map().end();
	if (it!=itend)
	  s=it->second;
	vector<string> v=giac::html_help(giac::html_mtt,s);
	if (!v.empty()){
	  if (use_external_browser)
	    giac::system_browser_command(v.front());
	  else {
	    Xcas_help_window->load(v.front().c_str());
	    if (!xcas::Xcas_help_window->visible())
	      xcas::Xcas_help_window->show();
	  }
	}
      }
      b->window()->show();
      Fl::focus(b);
    }
  }

  Fl_Window * handle_tab_w = 0;
  // Find a completion of s in v -> ans, return true if user OK
  // dx,dy=size of browser window
  int handle_tab(const string & s,const vector<string> & v,int dx,int dy,int & remove,string & ans,bool allow_immediate_out){
    static Fl_Hold_Browser * browser = 0;
    static Fl_Hold_Browser * related = 0;
    static Fl_Hold_Browser * syns = 0;
    static Fl_Button * button0 = 0 ;
    static Fl_Button * button1 =0;
    static Fl_Button * button2 =0;
    static Fl_Button * topic_help=0;
    static Fl_Input * input = 0;
    static Fl_Multiline_Output * output = 0;
    static Fl_Hold_Browser * examples = 0;
    static Fl_Input * argtab[TAB_ARGS]={0,0,0,0,0,0};
    int L=16;
    const giac::context * contextptr=giac::context0;
    if (xcas::Xcas_input_focus && xcas::Xcas_input_focus->window()){
      dx=4*xcas::Xcas_input_focus->window()->w()/5;
      dy=4*xcas::Xcas_input_focus->window()->h()/5;
      L=xcas::Xcas_input_focus->labelsize()+2;
      contextptr=get_context(xcas::Xcas_input_focus);
    }
    else {
      if (dx>500)
	dx=500;
    }
    if (dy<300)
      dy=300;
    if (dx<240)
      dx=240;
    // search non ascii char in s starting from the end
    int ss=s.size();
    string res;
    remove=0;
    for (int i=ss-1;i>=0;--i,++remove){
      const char & ch =s[i];
      if (giac::isalphan(ch) || ch=='&' || ch=='|' || ch=='=' || ch==':' || ch=='@' || ch=='<' || ch=='>' || ch=='+' || ch=='-' || ch=='/' || ch=='*' || ch=='$' || ch=='%')
	res=ch+res;
      else {
	if (!res.empty())
	  break;
      }
    }
    ss=res.size();
    if (!handle_tab_w){
      Fl_Group::current(0);
      handle_tab_w=new Fl_Window(50,100,dx,dy,gettext("Index"));
      button0 = new Fl_Button(2,2,dx/3-4,L+2);
      button0->shortcut(0xff0d);
      button0->label(gettext("OK"));
      button0->tooltip(gettext("Click to copy the commandname to the commandline"));
      button1 = new Fl_Button(dx/3+2,2,dx/3-4,L+2);
      button1->shortcut(0xff1b);
      button1->label(gettext("Cancel"));
      button2 = new Fl_Button(2*dx/3+2,2,dx/3-4,L+2);
      button2->label(gettext("Details"));
      button2->tooltip(gettext("Show full HTML help in browser"));
      browser = new Fl_Hold_Browser(2,2*L+4,dx/2-2,dy/2-(2*L+4));
      browser->format_char(0);
      browser->type(2);
      browser->label(gettext("Index"));
      browser->align(FL_ALIGN_TOP);
      browser->callback((Fl_Callback*)handle_tab_cb_browser);
      // order is important: related,syns, examples,input,output
      related = new Fl_Hold_Browser(dx/2+2,2*L+4,dx/2-2,dy/4-(L+2));
      related->label(gettext("Related"));
      related->format_char(0);
      related->align(FL_ALIGN_TOP);
      related->tooltip(gettext("Click for help on related command"));
      syns = new Fl_Hold_Browser(dx/2+2,related->y()+related->h()+L+2,dx/2-2,dy/4-(2*L+4));
      syns->format_char(0);
      syns->label(gettext("Synonyms"));
      syns->align(FL_ALIGN_TOP);
      topic_help = new  Fl_Button(0,browser->y()+browser->h(),L,L+4);
      topic_help->label("?");
      topic_help->tooltip(gettext("Search this word in HTML help"));
      input = new Fl_Input(L,browser->y()+browser->h(),dx-L,L+4);
      input->when(FL_WHEN_CHANGED |FL_WHEN_ENTER_KEY |FL_WHEN_NOT_CHANGED);
      // input->label("?");
      input->tooltip(gettext("Show commandnames starting from this text"));
      output = new Fl_Multiline_Output(2,input->y()+input->h(),dx-4,3*L+9);
      output->tooltip(gettext("Command short description and syntax"));
      // arguments
      int ypos=output->y()+output->h();
      for (int j=0;j<TAB_ARGS;j++){
	argtab[j]= new Fl_Input(dx/4+(j%2)*dx/2,ypos+1+(j/2)*L,dx/4,L-1);
	argtab[j]->when(FL_WHEN_ENTER_KEY |FL_WHEN_NOT_CHANGED);
      }
      ypos += 3*L;
      examples = new Fl_Hold_Browser(output->x(),ypos,output->w(),handle_tab_w->h()-output->y()-output->h()-2-3*L);
      examples->label("Examples");
      examples->type(2);
      examples->align(FL_ALIGN_LEFT);
      examples->tooltip(gettext("Left-click: copy example to commandline, right-click: fill in template with example values"));
      handle_tab_w->end();
      handle_tab_w->resizable(handle_tab_w);
      change_group_fontsize(handle_tab_w,L-2);
    }
    else {
      browser->clear();
      examples->clear();
      related->clear();
    }
    if (ss)
      input->value(res.c_str());
    else {
      res=input->value();
      ss=res.size();
      allow_immediate_out=false;
    }
    input->position(ss,ss);
    vector<string> vres;
    int vs=v.size(),i=0,r=-1,i_=0;
    for (int k=0;k<vs;++k){
      vres.push_back(v[k]);
      browser->add(v[k].c_str());
      if (!i && v[k].substr(0,ss)==res){
	i=k+1;
      }
      if (v[k][0]==res[0]){
	i_=k+1;
      }
    }
    if (!i){
      if (allow_immediate_out)
	return 0;
      else
	i=i_;
      if (!i)
	i=1;
    }
    handle_tab_w->set_modal();
    if (vs){
      browser->value(i);
      string bt=browser->text(i);
      update_examples(bt,examples,related,syns,output,argtab,giac::language(contextptr));
      handle_tab_w->show();
      handle_tab_w->hotspot(handle_tab_w);
      Fl::focus(input);
      for (;;) {
	if (!handle_tab_w->shown()){
	  r=0; break;
	}
	Fl_Widget *o = Fl::readqueue();
	if (!o) Fl::wait();
	else {
	  if (o == topic_help){ help_fltk(input->value()); }
	  if (o == button0) {r = 0; break;}
	  if (o == button1) {r = 1; break;}
	  if (o == button2)  browser_html_help(browser,examples,related,syns,output,argtab,giac::language(contextptr));
	  int j=0;
	  for (;j<TAB_ARGS;j++){
	    if (o==argtab[j]){ 
	      cerr << j << endl;
	      Fl::e_keysym=argtab[j]->value()[0];
	      break;
	    }
	  }
	  if (j!=TAB_ARGS){ r=0; break; }
	  if ( o == examples && examples->value() ) { 
	    string tmp=examples->text(examples->value());
	    if (Fl::event_button()!=3 || (!tmp.empty() && tmp[0]==' ')){
	      r=2;
	      break;
	    }
	    giac::gen tmpg(tmp,contextptr);
	    if (browser->value()>=1 && tmpg.type==giac::_SYMB && tmpg._SYMBptr->sommet==giac::gen(browser->text(browser->value()),contextptr))
	      tmpg=tmpg._SYMBptr->feuille;
	    giac::vecteur v;
	    if (tmpg.type==giac::_VECT && tmpg.subtype==giac::_SEQ__VECT)
	      v=*tmpg._VECTptr;
	    else
	      v=giac::vecteur(1,tmpg);
	    int vs=v.size();
	    for (int j=0;j<TAB_ARGS && j<vs;++j){
	      if (argtab && argtab[j])
		argtab[j]->value(v[j].print(contextptr).c_str());
	    }
	  }
	  if ( o == related && related->value() ) {
	    string s=related->text(related->value());
	    update_examples(s,examples,related,syns,output,argtab,giac::language(contextptr)); 
	    for (i=0;i<vs;++i){
	      if (v[i]==s){
		browser->value(i+1);
		break;
	      }
	    }
	  }
	  if ( o == syns && syns->value() ) {
	    string s=syns->text(syns->value());
	    update_examples(s,examples,related,syns,output,argtab,giac::language(contextptr)); 
	    for (i=0;i<vs;++i){
	      if (v[i]==s){
		browser->value(i+1);
		break;
	      }
	    }
	  }
	  if (o == handle_tab_w) { r=1; break; }
	  if (o == input){
	    if (Fl::event_key(FL_Enter) || Fl::event_key(FL_KP_Enter)){
	      if (Fl::event_state(FL_SHIFT |FL_CTRL | FL_ALT)){
		Fl::focus(examples);
		browser_html_help(browser,examples,related,syns,output,argtab,giac::language(contextptr));
	      }
	      else {
		r=0;
		break;
	      }
	    }
	    else {
	      const char * entree=input->value();
	      char nentree[256]; nentree[255]=0;
	      char nch[256]; nentree[255]=0;
	      for (int j=0;j<255;++j){
		nentree[j]=tolower(entree[j]);
		if (!entree[j]) break;
	      }
	      for (i=1;i<=vs;++i){
		const char * ch=browser->text(i);
		for (int j=0;j<255;++j){
		  nch[j]=tolower(ch[j]);
		  if (!ch[j]) break;
		}
		if (ch){
		  int comp=strcmp(nch,nentree);
		  if (!comp)
		    comp=strcmp(ch,entree);
		  if (comp>=0)
		    break;
		}
	      }
	      if (i<=vs){
		browser->value(i);
		update_examples(browser->text(i),examples,related,syns,output,argtab,giac::language(contextptr));
	      }
	    }
	  }
	}
      }
      /* does not work properly, since focus might change
	 if (foc && foc->window())
	foc->window()->show();
	else 
      */
	handle_tab_w->hide();
      // Xcas_help_window->hide();
      i=browser->value();
    }
    // delete browser;
    // delete button1;
    // delete button0;
    // delete w;
    int j=examples->value(); // ,k=related->value();
    if (r==2 && j<=examples->size() && j>0){
      ans=examples->text(j);
      return 2;
    }
    if (r==0 && i<=vs && i>0){
      ans=vres[i-1];
      string addans;
      Fl_Input ** ptr=argtab;
      if (ptr && *ptr && (*ptr)->visible()){
	addans = "(";
	for (int j=0;*ptr && j<TAB_ARGS;++j){
	  string tmp=(*ptr)->value();
	  if (tmp.empty())
	    continue;
	  addans += tmp;
	  ++ptr;
	  if (!(*ptr)->visible())
	    break;
	  addans += ",";
	}
	if (addans[addans.size()-1]==',')
	  addans=addans.substr(0,addans.size()-1);
	addans += ")";
      }
      if (addans.size()>2){
	ans += addans;
	return 2;
      }
      return 1;
    }
    else
      return 0;
  }


  void Multiline_Input_tab::insert_replace(const string & chaine,bool selected){
    size_t pos1=position();
    size_t pos2=mark();
    if (pos1>pos2){
      size_t tmp=pos1;
      pos1=pos2;
      pos2=tmp;
    }
    string input_s(value()),new_input;
    size_t l=input_s.size();
    new_input=input_s.substr(0,pos1)+chaine;
    if (pos2<l)
      new_input += input_s.substr(pos2,l-pos2);
    value(new_input.c_str());
    pos2=pos1+string(chaine).size();
    if (selected)
      position(pos1,pos2);
    else
      position(pos2,pos2);
    set_changed();
  }

  void Multiline_default_callback(Fl_Widget * w,void *){
    w->handle(FL_ENTER);
  }

  Multiline_Input_tab::Multiline_Input_tab(int x,int y,int w,int h,const char * l): 
    Fl_Multiline_Input(x, y, w, h, l),handling(false),completion_tab(giac::vector_completions_ptr()),tableur(0),_g(giac::undef) {
    if (parent()){
      labelfont(parent()->labelfont());
      labelsize(parent()->labelsize());
      textfont(parent()->labelfont());
      textsize(parent()->labelsize());
    }
    Fl_Widget::callback(Multiline_default_callback); parent_redraw(this); 
    when(FL_WHEN_ENTER_KEY|FL_WHEN_NOT_CHANGED);
    color(FL_WHITE);
    textfont(FL_TIMES);
  }
  
  Comment_Multiline_Input::Comment_Multiline_Input(int x,int y,int w,int h,const char * l):
    Fl_Multiline_Input(x,y,w,h,l) {
    if (parent()){
      labelfont(parent()->labelfont());
      labelsize(parent()->labelsize());
      textfont(parent()->labelfont());
      textsize(parent()->labelsize());
    }
    parent_redraw(this);
    color(FL_WHITE);
  }

  void Multiline_Input_tab::match(){
    static bool recursive_call=false;
    if (mark()!=position())
      return;
    int lastkey=Fl::event_key();
    if (lastkey!='(' && lastkey!='[' && lastkey!='{' && lastkey!='}' && lastkey!=']' && lastkey!=')' && lastkey!='0' && lastkey!='9' && lastkey!='\'' && lastkey != '=' && lastkey !=FL_Left && lastkey !=FL_Right)
      return;
    if (recursive_call)
      return;
    recursive_call=true;
    // check if cursor is on [, (, ), ]
    int pos=position(),p0=pos;
    const char * c=value();
    int pmax=string(c).size();
    bool closing=false,opening=false;
    if (pos<pmax)
      opening= c[pos]=='(' || c[pos]=='[' || c[pos]=='{';
    if (!opening && pos){
      closing = c[pos-1]==')' || c[pos-1]==']' || c[pos-1]=='}';
      if (closing)
	--p0;
    }
    if (opening || closing){
      Fl::flush();
      usleep(100000);      
      if (true || !Fl::get_key(lastkey)){
	Fl::check();
	int pos=position(),p0=pos;
	const char * c=value();
	int pmax=string(c).size();
	bool closing=false,opening=false;
	if (pos<pmax)
	  opening= c[pos]=='(' || c[pos]=='[' || c[pos]=='{';
	if (!opening && pos){
	  closing = c[pos-1]==')' || c[pos-1]==']' || c[pos-1]=='}';
	  if (closing)
	    --p0;
	}
	if (opening || closing){
	  bool ok=giac::matchpos(c,p0);
	  if (!ok){
	    if (closing)
	      p0=pos-1;
	    else
	      p0=pos+1;
	  }
	  else {
	    if (opening && pos<pmax)
	      p0=p0+1;
	  }
	  mark(p0);
	  unsigned s=selection_color();
	  if (ok){
	    if (opening)
	      selection_color(FL_GREEN);
	    else
	      selection_color(fl_color_cube(0,FL_NUM_GREEN-1,2)); 
	  }
	  else
	    selection_color(FL_RED);
	  damage(damage() | FL_DAMAGE_ALL);
	  redraw();
	  Fl::flush();
	  usleep(70000);
	  if (!Fl::ready()){
	    for (int i=0;i<giac::PARENTHESIS_NWAIT;++i){
	      usleep(50000);
	      if (Fl::ready())
		break;
	    }
	  }
	  selection_color(s);
	  position(pos,pos);
	  damage(damage() | FL_DAMAGE_ALL);
	  redraw();
	  Fl::flush();
	}
      }
    }
    recursive_call=false;
  }

  int Multiline_Input_tab::handle(int event){
    // if (handling)      return 0;
    handling=true;
    string oldval(value());
    int res=in_handle(event);
    History_Pack * g=get_history_pack(this);
    if (g && value()!=oldval)
      g->modified(false);
    handling=false;
    return res;
  }

  bool is_text_a_level(const char * ch){
    unsigned l=strlen(ch),i;
    char cmp[]="// fltk ";
    for (i=0;i<l && i<8;++i){
      if (ch[i]!=cmp[i])
	return false;
    }
    return true;
  }

  void increase_size(Fl_Widget * wid, int L){
    if (!wid) return;
    if (L+wid->h()<=wid->labelsize()+4)
      L=wid->labelsize()+5-wid->h();
    int pos;
    History_Pack * g = get_history_pack(wid,pos);
    if (g){
      Fl_Group * gr=wid->parent();
      // find parents to above history pack
      std::vector<Fl_Widget *> parents(1,wid);
      for (;gr && gr!=g;gr=gr->parent()){
	parents.push_back(gr);
      }
      Fl_Widget  * tmp=0,*tmp2;
      Fl_Group * tmpg=0;
      int i=parents.size()-1;
      for (;i>=0;--i){
	tmpg=gr;
	tmp=parents[i];
	// move children of gr below tmp
	int k=tmpg->children();
	for (int j=0;j<k;++j){
	  tmp2=tmpg->child(j);
	  if (tmp2->y()>tmp->y()){
	    tmp2->resize(tmp2->x(),tmp2->y()+L,tmp2->w(),tmp2->h());
	    tmp2->redraw();
	  }
	  else {
	    if (tmp2==tmp){
	      tmp->Fl_Widget::resize(tmp->x(),tmp->y(),tmp->w(),tmp->h()+L);
	      tmp->redraw();
	      gr=dynamic_cast<Fl_Group *>(tmp);
	    }
	  }
	} // end for j
      } // end for i
      // recompute pack
      g->resize();
      g->redraw();
    }
  }

  void Multiline_Input_tab::resize_nl(){
    if (tableur)
      return;
    const char * ch = value();
    unsigned i=0,l=strlen(ch),nl=1;
    for (;i<l;++i){
      if (ch[i]=='\n')
	++nl;
    }
    increase_size(this,6+nl*(labelsize()+2)-h());
  }

  bool Multiline_Input_tab::need_nl(){
    int i=position(),i0;
    const char * ch=value();
    for (i0=i-1;i0>=0;--i0){
      if (ch[i0]=='\n')
	break;
    }
    string s=string(ch).substr(i0+1,i-i0-1);
    fl_font(textfont(),textsize());
    int lw=int(1.2*fl_width(s.c_str()));
    return lw>w()+20;
  }

  int height(const char * ch,int labelsize){
    int n=strlen(ch);
    int h0=labelsize+4,res=n?h0+2:1;
    int maxh=300;
    for (int i=0;i<n-1;++i,++ch){
      if (*ch=='\n'){
	res += h0;
	if (res>maxh)
	  break;
      }
    }
    return res;
  }

  int Multiline_Input_tab::in_handle(int event){
    History_Pack * g=get_history_pack(this);
    if (g && event==FL_MOUSEWHEEL){
      if (!Fl::event_inside(this))
	return 0;
      if (Fl_Scroll * sc = dynamic_cast<Fl_Scroll *>(g->parent())){
	int scy=sc->yposition()+labelsize()*Fl::e_dy;
	if (scy<0)
	  scy=0;
#ifdef _HAVE_FL_UTF8_HDR_
	sc->scroll_to(sc->xposition(),scy);
#else
	sc->position(sc->xposition(),scy);
#endif
	return 1;
      }
    }
    if (event==FL_FOCUS || event==FL_PUSH || event==FL_KEYBOARD){
      Xcas_input_focus=this;
      autosave_disabled=false;
    }
    if (event==FL_FOCUS){
      // Fl::focus(this);
      // redraw();
      return Fl_Multiline_Input::handle(event);
    }
    if (event==FL_UNFOCUS){
      return Fl_Multiline_Input::handle(event);
    }
    if (event==FL_PUSH && tableur)
      tableur->editing=true;
    if (g && event==FL_PASTE){
      // check that it's not a // fltk ... pasting a full level
      const char * ch=Fl::event_text();
      if (ch){
	if (is_text_a_level(ch))
	  return g->handle(event);
      }
      if (Fl_Multiline_Input::handle(event)){
	if (!tableur){
	  // add space for ch
	  unsigned i=0,l=strlen(ch),nl=0;
	  for (;i<l;++i){
	    if (ch[i]=='\n')
	      ++nl;
	  }
	  increase_size(this,nl*(labelsize()+1));
	}
	return 1;
      }
    }
    if (event==FL_KEYBOARD) {
      static string toolt;
      string str;
      int key=Fl::event_key();
      if (g && key==FL_F+9){
	History_Fold * hf = get_history_fold(g);
	if (!hf) return 0;
	hf->eval();
	return 1;
      }
      if (Fl::event_text()){ 
	int i=Fl::event_text()[0];
	switch (i){
	case 22: case 25: 
	  // Ctrl-V or Ctrl-Y paste, no need to check for level text 
	  // because they paste using an FL_PASTE event
	case 3: case 4: case 5: case 21: case 23: case 24: case 26:
	  if (Fl_Multiline_Input::handle(event)){
	    resize_nl();
	    return 1;
	  }
	  break;
	case 1: 
	  position(0);
	  mark(size());
	  Fl::selection(*this,value(),strlen(value()));
	  return 1;
	case 2:
	  insert("[]");
	  position(position()-1,position()-1);
	  return 1;
	case 11:
	  insert("\n");
	  resize_nl();
	  return 1;
	case 12:
	  insert("{}");
	  position(position()-1,position()-1);
	  return 1;
	case ' ': case ',': case '+':
	  if (need_nl()){
	    Fl_Multiline_Input::handle(event);
	    insert("\n");
	    resize_nl();
	    return 1;
	  }
	  break;
#ifndef __APPLE__
	case '(':
	  // Fl::belowmouse(this);
	  str=motclef(string(value()).substr(0,position()));
	  if (!str.empty()){
	    toolt=writehelp(helpon(str,*giac::vector_aide_ptr(),giac::language(g?g->contextptr:0),giac::vector_aide_ptr()->size()),giac::language(g?g->contextptr:0));
	    tooltip(toolt.c_str());
	    int hh=height(toolt.c_str(),Fl_Tooltip::size());
	    Fl_Tooltip::enter_area(this,0,-hh,0,0,toolt.c_str());
	  }
	  break;
	case ')':
	  tooltip("");
	  break;
#endif
	}
      }
      if (key==FL_Enter || key==FL_KP_Enter){
	if (Fl::event_shift() || strlen(value())==0){
	  insert("\n");
	  if (!tableur)
	    increase_size(this,labelsize()+1);
	  return 1;
	}
	giac::gen val;
	// keep warnings
	giac::context * contextptr=g?g->contextptr:0;
	ostringstream warnings ;
#ifdef WITH_MYOSTREAM
	giac::my_ostream * old=giac::logptr(contextptr);
	giac::my_ostream newptr(&warnings);
	logptr(&newptr,contextptr);
#else
	my_ostream * old=giac::logptr(contextptr);
	logptr(&warnings,contextptr);
#endif
	giac::first_error_line(contextptr)=0;
	try {
	  val=warn_equal(giac::gen(value(),contextptr),contextptr);
	}
	catch (...){
	  ;
	}
	string warn0=warnings.str(),curline,debutline;
	giac::logptr(old,contextptr);
	static string warn;
	warn="";
	// Read warn0 lines and skip Parsing and Success in warn
	unsigned warns=warn0.size();
	char ch;
	for (unsigned i=0;i<warns;){
	  ch = warn0[i];
	  ++i;
	  curline += ch;
	  if (ch=='\n' || i==warns){
	    debutline=curline.substr(0,min(size_t(10),curline.size()));
	    if (debutline!=gettext("// Parsing") && debutline!=gettext("// Success"))
	      warn += curline;
	    *old << curline;
	    curline="";
	  }
	}
	// check if there is a parse error at end of input
	// if so then does like a event_shift
	if (giac::first_error_line(contextptr)){
	  string logs;
	  int col=giac::lexer_column_number(contextptr);
	  string token=giac::error_token_name(contextptr);
	  logs = gettext("Syntax error in compatibility mode: ")+giac::print_program_syntax(giac::xcas_mode(contextptr))+"\n";
	  logs += gettext("Parse error line ")+giac::print_INT_(giac::first_error_line(contextptr))+ gettext(" column ")+giac::print_INT_(col) + gettext(" at ") ;
	  if (!token.empty() && token[0]=='%')
	    logs += token.substr(1,token.size()-1);
	  else
	    logs += token;
	  const char * ch=value();
	  unsigned line=0,line_beg=0,line_end=0,taille=strlen(ch),i;
	  for (i=0;i<taille;i++){
	    if (ch[i]=='\n'){
	      ++line;
	      line_end=i;
	      if (line==unsigned(giac::first_error_line(contextptr)))
		break;
	      line_beg=i+1;
	    }
	  }
	  if (line_beg+col>taille){
	    int ans=fl_ask("%s",((logs+'\n')+gettext("To get a newline, use shift-Enter. Reedit?")).c_str());
	    if (ans==1){
	      position(taille,taille);
	      Fl::focus(this);
	      handle(FL_FOCUS);
	      // insert("\n");
	      // if (!tableur) increase_size(this,labelsize()+1);
	      return 1;
	    }
	  }
	  else {
	    // position(line_beg,line_end);
	    int ans=fl_ask("%s",(logs+"\nReedit?").c_str());
	    if (ans){
	      i=line_beg+col-1;
	      position(max(int(i-token.size()),0),i);
	      Fl::focus(this);
	      handle(FL_FOCUS);
	      return 1;
	    }
	  }
	} // if first_error_line
	else { 
	  // Correct parse, check for warnings
	  if (warn.empty()){
	    _g=val;
	    clear_changed();
	  }
	  else
	    fl_message("%s",warn.c_str());
	}
	position(size(), 0);
	history.push_back(value());
	count=history.size();
	int pos=-1;
	History_Pack * hp=get_history_pack(this,pos);
	if (hp)
	  hp->update_pos=pos;
	find_fold_autosave_function(true);
	do_callback();
	return 1;
      }
      if (key==FL_Escape){
	if (tableur && tableur->editing){
	  tableur->editing=false;
	  Fl::focus(tableur);
	}
	else {
	  Fl::selection(*this,value(),strlen(value()));
	  value("");
	}
	return 1;
      }
      if (key==FL_BackSpace){
	const char * ch = value();
	unsigned l=strlen(ch);
	unsigned p=position();
	unsigned m=mark();
	if (tableur && tableur->editing && !l){
	  tableur->editing=false;
	  Fl::focus(tableur);
	  return 1;
	}
	if (l && p && p==m && p<=l && ch[p-1]=='\n'){
	  if (!tableur)
	    increase_size(this,-labelsize()-1);
	}
      }
      if (tableur){
	tableur->editing=true;
	tableur->edit_row=tableur->row();
	tableur->edit_col=tableur->col();
      }
      if (g && !tableur && key==FL_Right && !Fl::event_state(FL_SHIFT | FL_CTRL | FL_ALT)){ 
	int i=position();
	if (i==size())
	  return 1;
      }
      if (g && !tableur && key==FL_Left && !Fl::event_state(FL_SHIFT | FL_CTRL | FL_ALT)){ 
	int i=position();
	if (!i)
	  return 1;
      }
      if (g && !tableur && (key==FL_Up || key==FL_Page_Up) && !Fl::event_state(FL_SHIFT | FL_CTRL | FL_ALT)){ 
	// if we are at the top line, go one level up
	int i=position();
	if (i)
	  i=line_start(i);
	if (!i || (key==FL_Page_Up) ){
	  redraw();
	  g->_sel_begin=-1;
	  int pos=g->set_sel_begin(this);
	  g->_sel_begin=-1;
	  --pos;
	  if (pos>=0)
	    g->focus(pos,true);
	  return 1;
	}
      }
      if (g && !tableur && (key==FL_Down || key==FL_Page_Down) && !Fl::event_state(FL_SHIFT | FL_CTRL | FL_ALT)){ 
	// if we are at the top line, go one level up
	int i=position();
	i=line_end(i);
	if (i>=size() || (key==FL_Page_Down) ){
	  redraw();
	  g->_sel_begin=-1;
	  int pos=g->set_sel_begin(this)+1;
	  g->_sel_begin=-1;
	  g->focus(pos,true);
	  return 1;
	}
      }
      if (key==FL_Up || key==FL_Down){
	if (!Fl::event_state(FL_SHIFT |FL_CTRL | FL_ALT) && Fl_Multiline_Input::handle(event)){
	  match();
	  redraw();
	  return 1;
	}
	if (!Fl::event_state(FL_CTRL | FL_ALT))
	  return 0;
	// not handled by the input, use history
	int s=history.size();
	Fl::focus(this);
	if (Fl::event_key()==FL_Up){
	  --count;
	  if (count<0)
	    count=0;
	}
	else {
	  ++count;
	  if (count>=s)
	    count=max(0,s-1);
	}
	if (count<s){
	  string v(value());
	  int pos1=position(),pos2=mark();
	  if (pos1>pos2)
	    giac::swapint(pos1,pos2);
	  value((v.substr(0,pos1)+history[count]+v.substr(pos2,v.size()-pos2)).c_str());
	  position(pos1,pos1+history[count].size());
	}
	set_changed();
	return 1;
      }
    }
    if (!completion_tab || event!=FL_KEYBOARD || (Fl::event_text() && Fl::event_text()[0]!=9 && Fl::event_key()!=FL_F+1)){
      const char * ch = value();
      unsigned l=strlen(ch);
      unsigned p=position();
      unsigned m=mark();
      bool test_backspace=(l && p && p==m && p<=l && ch[p-1]=='\n') || (p!=m);
      int res=Fl_Multiline_Input::handle(event);
      if (event==FL_KEYBOARD && res ){ 
	match();
	redraw();
	if ( test_backspace && Fl::event_key()==FL_BackSpace)
	  resize_nl();
      }
      return res;
    }
    string s(value()),ans;
    if (position()<int(s.size()))
      s=s.substr(0,position());
    int delta=s.size();
    s=motclef(s);
    delta -= s.size();
    int remove;
    if (int ii=handle_tab(s,*completion_tab,window()->w(),window()->h()/3,remove,ans)){
      window()->show();
      cut(-remove-delta);
      if (ii==1){
	insert((ans+"()").c_str());
	position(size()-1);
      }
      else {
	insert(ans.c_str());
	position(size());
      }
      if (parent())
	parent_redraw(parent());
    }
    Fl::focus(this);
    handle(FL_FOCUS);
    return 1;
  }

  /* void Multiline_Input_tab::draw(){
    int pos=position();
    int clip_x,clip_y,clip_w,clip_h;
    fl_clip_box(x(),y(),w(),h(),clip_x,clip_y,clip_w,clip_h);
    if (clip_w==0 || clip_h==0)
      return;
    // if (pos!=mark()){
      Fl_Multiline_Input::draw();
      return;
      // }
      } */

  // geo_print / geoprint
  std::string _pnt2string(const giac::gen & g,const giac::context * contextptr){
    unsigned ta=taille(g,100);
    if (ta>100)
      return "Done";
    if (g.is_symb_of_sommet(giac::at_pnt) && !is3d(g)){
      giac::gen & f=g._SYMBptr->feuille;
      if (f.type==giac::_VECT && !f._VECTptr->empty()){
	giac::gen f0=f._VECTptr->front();
	if (f0.is_symb_of_sommet(giac::at_legende)){
	  return g.print(contextptr);
	}
	if (f0.is_symb_of_sommet(giac::at_curve)){
	  giac::gen f1=f[0]._SYMBptr->feuille;
	  if (f1.type==giac::_VECT && !f1._VECTptr->empty() ){
	    giac::gen f1f=f1._VECTptr->front();
	    if (f1f.type==giac::_VECT && f1f._VECTptr->size()>=4){
	      giac::vecteur f1v=*f1f._VECTptr;
	      return "plotparam("+pnt2string(f1v[0],contextptr)+","+f1v[1].print(contextptr)+"="+f1v[2].print(contextptr)+".."+f1v[3].print(contextptr)+")";
	    }
	  }
	}
	if (f0.is_symb_of_sommet(giac::at_cercle) && f0._SYMBptr->feuille.type==giac::_VECT){
	  giac::gen centre,rayon;
	  if (!giac::centre_rayon(f0,centre,rayon,true,0))
	    return "cercle_error";
	  if (!complex_mode(contextptr) && (centre.type<giac::_IDNT || centre.type==giac::_FRAC) )
	    return "cercle(point("+giac::re(centre,contextptr).print(contextptr)+","+giac::im(centre,contextptr).print(contextptr)+"),"+rayon.print(contextptr)+")";
	  else
	    return "cercle(point("+centre.print(contextptr)+"),"+rayon.print(contextptr)+")";
	}
	if (f0.type==giac::_VECT &&f0.subtype!=giac::_POINT__VECT){
	  std::string s="polygone(";
	  giac::const_iterateur it=f0._VECTptr->begin(),itend=f0._VECTptr->end();
	  if ( itend-it==2){ 
	    switch(f0.subtype){
	    case giac::_LINE__VECT:
	      s="droite(";
	      break;
	    case giac::_HALFLINE__VECT:
	      s="demi_droite(";
	      break;
	    case giac::_GROUP__VECT:
	      s="segment(";
	      break;
	    }
	    if (f0.subtype==giac::_LINE__VECT && it->type!=giac::_VECT){ // 2-d line
	      s += _equation(g,contextptr).print(contextptr) + ")";
	      return s;
	    }
	  }
	  for (;it!=itend;){
	    s += "point(";
	    if (!complex_mode(contextptr) && (it->type<giac::_IDNT || it->type==giac::_FRAC) )
	      s += giac::re(*it,contextptr).print(contextptr)+","+giac::im(*it,contextptr).print(contextptr);
	    else
	      s+=it->print(contextptr);
	    s+=")";
	    ++it;
	    s += it==itend?")":",";
	  }
	  return s;
	}
	if ( (f0.type!=giac::_FRAC && f0.type>=giac::_IDNT) || is3d(g) || complex_mode(contextptr))
	  return "point("+f0.print(contextptr)+")";
	else
	  return "point("+giac::re(f0,contextptr).print(contextptr)+","+giac::im(f0,contextptr).print(contextptr)+")";
      }
    } 
    if (g.type==giac::_VECT && !g._VECTptr->empty() && g._VECTptr->back().is_symb_of_sommet(giac::at_pnt)){
      std::string s = "[";
      giac::const_iterateur it=g._VECTptr->begin(),itend=g._VECTptr->end();
      for (;it!=itend;){
	s += pnt2string(*it,contextptr);
	++it;
	s += it==itend?"]":",";
      }
      return s;
    }
    return g.print(contextptr);
  }

  std::string pnt2string(const giac::gen & g,const giac::context * contextptr){
    try {
      return _pnt2string(g,contextptr);
    }
    catch (...){
      return "conversion error";
    }
  }

  void Gen_Output::value(const giac::gen & _g){
    g=_g;
    Fl_Multiline_Output::value(pnt2string(g,get_context(this)).c_str()); 
  }

  void Gen_Output::value(const char * ch){
    giac::context * contextptr=get_context(this);
    g=giac::gen(ch,contextptr);
    Fl_Multiline_Output::value(ch);
  }

  giac::gen Gen_Output::value() const {
    return g;
  }
  
  void Multiline_Input_tab::set_g(const giac::gen & g) {
    const giac::context * contextptr = get_context(this);
    value(g.print(contextptr).c_str());
    redraw();
    clear_changed();
    _g=g;
  }

  giac::gen Multiline_Input_tab::g() {
    if (!changed())
      return _g;
    giac::context * contextptr=get_context(this);
    _g=giac::gen(value(),contextptr);
    clear_changed();
    return _g;
  }

  void Enlargable_Multiline_Output::value(const char * ch){
    Fl_Multiline_Output::value(ch);
    resize();
  }

  void Enlargable_Multiline_Output::resize(){
    // Count number of \n
    const char * ch=Fl_Multiline_Output::value();
    int n=strlen(ch);
    int h0=labelsize(),res=n?h0+4:1;
    int j=0,nc=0,nl=0;
    string temp="";
    fl_font(textfont(),labelsize());
    for (int i=0;i<n;++i,++ch){
      if (*ch=='\n'){
	res += h0+1;
	nc=max(nc,j);
	j=0;
	nl=max(nl,int(fl_width(temp.c_str())));
	temp ="";
      }
      else {
	j++;
	temp += *ch;
      }
    }
    nl+=6;
    if (parent()){
      int w=parent()->w();
      int h=parent()->h();
      if (res<h-6)
	res=h-6;
      if (nl<w-labelsize())
	nl=w-labelsize();
      parent()->redraw();
    }
    Fl_Multiline_Output::resize(x(),y(),nl,res);
  }

  int Comment_Multiline_Input::handle(int event){
    string oldval(value());
    int res=in_handle(event);
    History_Pack * g=get_history_pack(this);
    if (g && value()!=oldval)
      g->modified(false);
    return res;
  }

  int Comment_Multiline_Input::in_handle(int event){
    if (event==FL_FOCUS){
      Xcas_input_focus=this;
      autosave_disabled=false;
      // Fl::focus(this);
      // redraw();
      return Fl_Multiline_Input::handle(event);
    }
    if (event==FL_UNFOCUS){
      return Fl_Multiline_Input::handle(event);
    }
    if (event==FL_KEYBOARD){
      redraw();
      int key=Fl::event_key();
      if ( (key==FL_Enter || key==FL_KP_Enter) 
	   && Fl::event_shift()){
	insert("\n");
	increase_size(this,labelsize()+2);
	return 1;
      }
      if (key==FL_BackSpace){
	const char * ch = value();
	unsigned l=strlen(ch);
	unsigned p=position();
	unsigned m=mark();
	if (l && p && p==m && p<=l && ch[p-1]=='\n'){
	  increase_size(this,-labelsize()-2);
	}
      }
      History_Pack * hp = get_history_pack(this);
      int change_focus=0;
      if (hp && ( (key==FL_Up && !line_start(position())) || key==FL_Page_Up))
	change_focus=-1;
      if (hp && ( (key==FL_Down && line_end(position())==size()) || key==FL_Page_Down || key==FL_Enter))
	change_focus=1;
      if (key==FL_Enter){
	string s=value();
	s+=' ';
	// search in s for a word with ., check if it's an existing filename or URL
	int ss=s.size();
	for (int i=0;i<ss;++i){
	  if (s[i]==' ')
	    continue;
	  int wordbegin=i;
	  bool haspoint=false;
	  // begin of word
	  for (++i;i<ss;++i){
	    if (s[i]=='.' &&
		i<ss-1 && s[i+1]>='a' && s[i+1]<='z' &&
		i && (isalpha(s[i-1]) || (s[i-1]>='0' && s[i-1]<='9'))
		)
	      haspoint=true;
	    if (s[i]==' ')
	      break;
	  }
	  if (!haspoint)
	    continue;
	  string url;
	  if (i>wordbegin+7 && (s.substr(wordbegin,7)=="http://" || s.substr(wordbegin,7)=="file://")){
	    url=s.substr(wordbegin,i-wordbegin);
	  }
	  else {
	    if (s[wordbegin]=='@'){
	      url=s.substr(wordbegin+1,i-wordbegin-1);
	    }
	  }
	  if (url.empty())
	    continue;
	  if (giac::is_file_available(url.c_str())){
	    if (url[0]!='/')
	      url=*giac::_pwd(0,0)._STRNGptr+"/"+url;
	  }
	  else {
	    if (url.size()<7 || (url.substr(0,7)!="http://" && url.substr(0,7)!="file://"))
	      url="http://"+url;
	  }
	  giac::system_browser_command(url);
	  // break;
	}
      }
      if (change_focus && hp){
	hp->_sel_begin=-1;
	int pos=hp->set_sel_begin(this);
	if (pos+change_focus>=0)
	  hp->focus(pos+change_focus,true);
	return 1;
      }      
    }
    return Fl_Multiline_Input::handle(event);
  }

#ifndef NO_NAMESPACE_XCAS
} // namespace xcas
#endif // ndef NO_NAMESPACE_XCAS


#endif // HAVE_LIBFLTK
