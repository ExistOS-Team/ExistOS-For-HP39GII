
#include <stdlib.h>
#include <iostream>

#include "Fatfs/ff.h"
#include "kcasporing_gl.h"


#define GIAC_HISTORY_SIZE 2
#define GIAC_HISTORY_MAX_TAILLE 32

// //#include "keyboard.h"
#include "iostream_sub.h"
#include "giac.h"
#include "gen.h"

#include "sys_llapi.h"

using namespace giac;
using namespace std;

extern giac::context * contextptr;
extern bool esc_flag;

volatile bool interrupted = false;
volatile bool ctrl_c = false;

//, esc_flag = false;

namespace giac{

unsigned int rtc_get_tick_ms()
{
    return ll_get_time_ms();
}

stdostream cout;

}
#if 0
void do_run(const char * s,gen & g,gen & ge){
  if (!contextptr)
    contextptr=new giac::context;
  int S=strlen(s);
  char buf[S+1];
  buf[S]=0;
  for (int i=0;i<S;++i){
    char c=s[i];
    if (c==0x1e || c==char(0x9c))
      buf[i]='\n';
    else {
      if (c==0x0d)
	buf[i]=' ';
      else
	buf[i]=c;
    }
  }
  g=gen(buf,contextptr);
  std::cout << (g.print(contextptr).c_str()) << endl; //return ;
  giac::freeze=false;
  execution_in_progress = 1;
  giac::set_abort();
  ge=eval(equaltosto(g,contextptr),1,contextptr);
  giac::clear_abort();
  execution_in_progress = 0;
  if (esc_flag || ctrl_c){
    while (confirm("Interrupted","F1/F6: ok",true)==-1)
      ; // insure ON has been removed from keyboard buffer
    ge=string2gen("Interrupted",false);
    // memory full?
    if (!kbd_interrupted){
      // clear turtle, display msg
      turtle_stack()=vector<logo_turtle>(1,logo_turtle());
      while (confirm((lang?"Memoire remplie!":"Memory full"),"Purge variable",true)==-1)
	;
    /*
      gen g=select_var(contextptr);
      if (g.type==_IDNT)
	_purge(g,contextptr);
      else 
	_restart(0,contextptr);
*/
    }
  }
  std::cout << "Done" << endl;
  //Console_Output("Done"); return ;
  esc_flag=0;
  ctrl_c=false;
  giac::kbd_interrupted=interrupted=false;
}  

void run(const char * s,int do_logo_graph_eqw){
  if (strlen(s)>=2 && (s[0]=='#' ||
		       (s[0]=='/' && (s[1]=='/' || s[1]=='*'))
		       ))
    return;
  gen g,ge;
  do_run(s,g,ge);
  if (giac::freeze){
    giac::freeze=false;
    //DefineStatusMessage((char*)lang?"Taper sur une touche":"Screen freezed. Press any key.", 1, 0, 0);
    //DisplayStatusArea();
    //int keyflag = GetSetupSetting( (unsigned int)0x14);
    unsigned int key;
    //ck_getkey(&key);
  }
  int t=giac::taille(g,GIAC_HISTORY_MAX_TAILLE);  
  int te=giac::taille(ge,GIAC_HISTORY_MAX_TAILLE);
  bool do_tex=false;
  if (t<GIAC_HISTORY_MAX_TAILLE && te<GIAC_HISTORY_MAX_TAILLE){
    giac::vecteur &vin=history_in(contextptr);
    giac::vecteur &vout=history_out(contextptr);
    if (vin.size()>GIAC_HISTORY_SIZE)
      vin.erase(vin.begin());
    vin.push_back(g);
    if (vout.size()>GIAC_HISTORY_SIZE)
      vout.erase(vout.begin());
    vout.push_back(ge);
  }
  //check_do_graph(ge,do_logo_graph_eqw);
  string s_;
  if (ge.type==giac::_STRNG)
    s_='"'+*ge._STRNGptr+'"';
  else {
    if (te>256)
      s_="Object too large";
    else {
      if (ge.is_symb_of_sommet(giac::at_pnt) || (ge.type==giac::_VECT && !ge._VECTptr->empty() && ge._VECTptr->back().is_symb_of_sommet(giac::at_pnt)))
	s_="Graphic object";
      else {
	//do_tex=ge.type==giac::_SYMB && has_op(ge,*giac::at_inv);
	// tex support has been disabled!
	s_=ge.print(contextptr);
	// translate to tex? set do_tex to true
      }
    }
  }
  if (s_.size()>512)
    s_=s_.substr(0,509)+"...";

  std::cout << s_ << endl;
  
  //char* edit_line = (char*)Console_GetEditLine();
  //Console_Output((const unsigned char*)s_.c_str());

  //return ge; 
}
#endif
 



void tstdo_run(const char *s, gen &g, gen &ge)
{
	if (!contextptr)
		contextptr = new giac::context;
	int S = strlen(s);
	char buf[S + 1];
	buf[S] = 0;
	for (int i = 0; i < S; ++i)
	{
		char c = s[i];
		if (c == 0x1e || c == char(0x9c))
			buf[i] = '\n';
		else
		{
			if (c == 0x0d)
				buf[i] = ' ';
			else
				buf[i] = c;
		}
	}
	g = gen(buf, contextptr);
	std::cout << (g.print(contextptr).c_str()) << endl; // return ;
	giac::freeze = false;
	execution_in_progress = 1;

	// giac::set_abort();
	ge = eval(equaltosto(g, contextptr), 1, contextptr);
	// giac::clear_abort();

	execution_in_progress = 0;

	if (esc_flag || ctrl_c)
	{
		while (confirm("Interrupted", "F1/F6: ok", true) == -1)
			; // insure ON has been removed from keyboard buffer
		ge = string2gen("Interrupted", false);
		// memory full?
		if (!kbd_interrupted)
		{
			// clear turtle, display msg
			turtle_stack() = vector<logo_turtle>(1, logo_turtle());
			while (confirm((lang ? "Memoire remplie!" : "Memory full"), "Purge variable", true) == -1)
				;
			/*
			  gen g=select_var(contextptr);
			  if (g.type==_IDNT)
			_purge(g,contextptr);
			  else
			_restart(0,contextptr);
		*/
		}
	}

	std::cout << "Done" << endl;
	// Console_Output("Done"); return ;

	esc_flag = 0;
	ctrl_c = false;
	giac::kbd_interrupted = interrupted = 0;
}

void tstrun(const char *s)
{
	if (strlen(s) >= 2 && (s[0] == '#' ||
						   (s[0] == '/' && (s[1] == '/' || s[1] == '*'))))
		return;
	gen g, ge;
	tstdo_run(s, g, ge);
	if (giac::freeze)
	{
		giac::freeze = false;
		// DefineStatusMessage((char*)lang?"Taper sur une touche":"Screen freezed. Press any key.", 1, 0, 0);
		// DisplayStatusArea();
		// int keyflag = GetSetupSetting( (unsigned int)0x14);
		unsigned int key;
		// ck_getkey(&key);
	}

	/*
	int t=giac::taille(g,GIAC_HISTORY_MAX_TAILLE);
	int te=giac::taille(ge,GIAC_HISTORY_MAX_TAILLE);
	bool do_tex=false;
	if (t<GIAC_HISTORY_MAX_TAILLE && te<GIAC_HISTORY_MAX_TAILLE){
	  giac::vecteur &vin=history_in(contextptr);
	  giac::vecteur &vout=history_out(contextptr);
	  if (vin.size()>GIAC_HISTORY_SIZE)
		vin.erase(vin.begin());
	  vin.push_back(g);
	  if (vout.size()>GIAC_HISTORY_SIZE)
		vout.erase(vout.begin());
	  vout.push_back(ge);
	}*/

	string s_;
	if (ge.type == giac::_STRNG)
		s_ = '"' + *ge._STRNGptr + '"';
	else
	{
		// if (te>256)
		//   s_="Object too large";
		// else {

		if (ge.is_symb_of_sommet(giac::at_pnt) || (ge.type == giac::_VECT && !ge._VECTptr->empty() && ge._VECTptr->back().is_symb_of_sommet(giac::at_pnt)))
			s_ = "Graphic object";
		else
		{
			// do_tex=ge.type==giac::_SYMB && has_op(ge,*giac::at_inv);
			//  tex support has been disabled!
			s_ = ge.print(contextptr);
			// translate to tex? set do_tex to true
		}

		//}
	}
	if (s_.size() > 512)
		s_ = s_.substr(0, 509) + "...";

	std::cout << s_ << endl;

	// char* edit_line = (char*)Console_GetEditLine();
	// Console_Output((const unsigned char*)s_.c_str());

	// return ge;
}
  


    int kcas_main(int isAppli, unsigned short OptionNum);
extern "C"{
extern bool khicasRunning;
#include "SystemFs.h"
#include "lfs.h"
#include "SysConf.h"
 
    void testcpp() 
    {

		
		khicasRunning = true;

        if(vGL_Initialize())
        {
			khicasRunning=false;
          return;
        } 
/*
        tstrun("int(x)");
        tstrun("int(x^2)");
        tstrun("int(sin(x))");
        tstrun("diff(x^3*ln(x))");=
        tstrun("ilaplace(b/(s-A)^N,s,t)");
        tstrun("ilaplace(b/(s-3)^2,s,t)");
*/
    #if FS_TYPE == FS_FATFS
		FRESULT fr;
		fr = f_mkdir("/xcas");
        if(fr != FR_OK)
		{
			if(fr != FR_EXIST){
				std::cout << "Failed to create dir /xcas, " << fr <<endl;
			}
		}
	#else
		lfs *fs = (lfs *)GetFsObj();
		int fr;
		fr = lfs_mkdir(fs, "/xcas");
		if(fr){
			if(fr != LFS_ERR_EXIST)
				std::cout << "Failed to create dir /xcas, " << fr <<endl;
		}
		
	#endif
		kcas_main(0,0);

        /*
        context ct;
        contextptr=&ct;
        _srand(vecteur(0),contextptr);


        if (!contextptr)
        contextptr=new giac::context;*/

        
        //run("int(sin(x))\r\n", 7);

        //giac::caseval("int(sin(x))");

    }

}



// using namespace giac;
// using namespace std;


// extern "C"{


// #include <stdio.h>
// #include "sys_llapi.h"

// int system_checkCtrlC();



// char *giac_eval(char *s)
// {
//     const char *ret;
//     ret = giac::caseval(s);
//     return (char *)ret;
// }

// void testcpp() 
// {
//     unsigned int stime;
//     vx_var=identificateur("x");
//     cout << "test C++ STARTSTART" << endl;
//     //cout << pgcd(142,422) << endl;
    
//     //debug_infolevel = 3;


//     stime = ll_get_time_us()/1000;
//     cout << "( int(sqrt(2x*4)) ) Start Time:" << stime << endl;
//     cout << giac::caseval("int(sqrt(2x*4))")  <<endl;
//     cout << "Time:" << ll_get_time_us()/1000 - stime << endl;

    
//     stime = ll_get_time_us()/1000;
//     cout << "( int(sin(x)^2) ) Start Time:" << stime << endl;
//     cout << giac::caseval("int(sin(x)^2)")  <<endl;
//     cout << "Time:" << ll_get_time_us()/1000 - stime << endl;



//     stime = ll_get_time_us()/1000;
//     cout << "( texpand((cos(3*x)+cos(7*x))/cos(5*x)) ) Start Time:" << stime << endl;
//     cout << giac::caseval("texpand((cos(3*x)+cos(7*x))/cos(5*x))")  <<endl;
//     cout << "Time:" << ll_get_time_us()/1000 - stime << endl;
// /*
//     stime = ll_get_time_us()/1000;
//     cout << "( simplify(texpand((cos(3*x)+cos(7*x))/cos(5*x))) ) Start Time:" << stime << endl;
//     cout << giac::caseval("simplify(texpand((cos(3*x)+cos(7*x))/cos(5*x)))")  <<endl;
//     cout << "Time:" << ll_get_time_us()/1000 - stime << endl;

// */
//     cout << "test C++ 123456789" << endl;
 


// }


// }


// int giac::giac_checkCtrlC()
// {
//     /*
//     char key, press;
//     if(getKey(&key, &press))
//     {
//         if((key == KEY_ON) && (press == KEY_PRESS)){
//             return 1;
//         }
//     }*/
//     return 0;
// }


