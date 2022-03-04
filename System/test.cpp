
#include <stdlib.h>
#include <iostream>

#include "giac.h"

using namespace std;
using namespace giac;

extern "C"{
#include <stdio.h>
#include "sys_llapi.h"

int system_checkCtrlC();

void testcpp() 
{
    unsigned int stime;
    cout << "test C++ STARTSTART" << endl;
    //cout << pgcd(142,422) << endl;


    stime = ll_gettime_us()/1000;
    cout << "( int(sqrt(2x*4)) ) Start Time:" << stime << endl;
    cout << caseval("int(sqrt(2x*4))")  <<endl;
    cout << "Time:" << ll_gettime_us()/1000 - stime << endl;

    
    stime = ll_gettime_us()/1000;
    cout << "( int(sin(x)^2) ) Start Time:" << stime << endl;
    cout << caseval("int(sin(x)^2)")  <<endl;
    cout << "Time:" << ll_gettime_us()/1000 - stime << endl;


    cout << "test C++ 123456789" << endl;
 


}

}


int giac::giac_checkCtrlC()
{
    return system_checkCtrlC();
}
