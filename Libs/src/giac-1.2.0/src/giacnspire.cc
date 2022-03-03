#include "os.h"
#include "giac.h"

using namespace giac;
using namespace std;


int main(){
  vx_var=identificateur("x");
  clrscr();
  giac::context c;
#if 0
  for (unsigned i=0;i<32;++i)
    COUT << endl;
  char *value=0,saved[1024]="factor(x^4-1)",ans[1024];
  giac::gen g,savedg;
  for (unsigned i=0;;++i){
    if (value) free(value);
    if (show_msg_user_input("Expression? empty leaves",print_INT_(i).c_str(),saved,&value)==-1)
      return 0;
    g=giac::gen(value,&c);
    g=eval(g,&c);
    clrscr();
    //if (i) COUT << i-1 << ">>" << saved << endl << savedg << endl;
    COUT << i << ">>" << value << endl << g.print(&c) << endl;
    strncpy(ans,g.print(&c).c_str(),1023);
    ans[1023]=0;
    show_msgbox(value,ans);
    strncpy(saved,value,1023);
    savedg=g;
    saved[1023]=0;
  }
#else
  // identificateur x("x");
  // COUT << _factor(pow(x,4,&c)-1,&c) << endl;
  COUT << "Enter expression to eval, empty to quit, ?command for help " << endl;
  for (unsigned i=0;;++i){
    //console_cin.foreground_color(nio::COLOR_RED);
    COUT << i << ">> ";
    //console_cin.foreground_color(nio::COLOR_BLACK);
    string s;
    CIN >> s;
    if (s.empty())
      break;
    //console_cin.foreground_color(nio::COLOR_GREEN);
    ctrl_c=interrupted=false;
    giac::gen g(s,&c);
    // COUT << "type " << g.type << endl;
    // if (g.type==_SYMB) COUT << g._SYMBptr->sommet << endl;
    // COUT << "before eval " << g << endl; wait_key_pressed();
    g=eval(g,&c);
    //console_cin.foreground_color(nio::COLOR_BLUE);
    COUT << g << endl;
  }
#endif
  return 0;
}
