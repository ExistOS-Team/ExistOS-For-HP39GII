
#include <stdlib.h>
#include <iostream>

#include "keyboard.h"
#include "giac.h"
#include "gen.h"


using namespace giac;
using namespace std;


extern "C"{


#include <stdio.h>
#include "sys_llapi.h"

int system_checkCtrlC();



char *giac_eval(char *s)
{
    const char *ret;
    ret = giac::caseval(s);
    return (char *)ret;
}

void testcpp() 
{
    unsigned int stime;
    vx_var=identificateur("x");
    cout << "test C++ STARTSTART" << endl;
    //cout << pgcd(142,422) << endl;
    
    //debug_infolevel = 3;


    stime = ll_gettime_us()/1000;
    cout << "( int(sqrt(2x*4)) ) Start Time:" << stime << endl;
    cout << giac::caseval("int(sqrt(2x*4))")  <<endl;
    cout << "Time:" << ll_gettime_us()/1000 - stime << endl;

    
    stime = ll_gettime_us()/1000;
    cout << "( int(sin(x)^2) ) Start Time:" << stime << endl;
    cout << giac::caseval("int(sin(x)^2)")  <<endl;
    cout << "Time:" << ll_gettime_us()/1000 - stime << endl;



    stime = ll_gettime_us()/1000;
    cout << "( texpand((cos(3*x)+cos(7*x))/cos(5*x)) ) Start Time:" << stime << endl;
    cout << giac::caseval("texpand((cos(3*x)+cos(7*x))/cos(5*x))")  <<endl;
    cout << "Time:" << ll_gettime_us()/1000 - stime << endl;
/*
    stime = ll_gettime_us()/1000;
    cout << "( simplify(texpand((cos(3*x)+cos(7*x))/cos(5*x))) ) Start Time:" << stime << endl;
    cout << giac::caseval("simplify(texpand((cos(3*x)+cos(7*x))/cos(5*x)))")  <<endl;
    cout << "Time:" << ll_gettime_us()/1000 - stime << endl;

*/
    cout << "test C++ 123456789" << endl;
 


}


}


int giac::giac_checkCtrlC()
{
    char key, press;
    if(getKey(&key, &press))
    {
        if((key == KEY_ON) && (press == KEY_PRESS)){
            return 1;
        }
    }

    return 0;
}
