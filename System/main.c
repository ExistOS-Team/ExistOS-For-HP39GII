
#include <stdint.h>
#include <stdio.h>


#include "sys_llapi.h"

char s[] = "Check for working CXX compiler: D:/Program Files (x86)/GNU Arm Embedded Toolchain/10 2021.07/bin/arm-none-eabi-g++.exe - skipped\n";


void main()
{
    for(;;){
        ll_delay(1000);
        ll_putStr(s);
    }

    while(1);


}