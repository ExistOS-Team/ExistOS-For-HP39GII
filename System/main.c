
#include <stdint.h>
#include <stdio.h>

#include "sys_llapi.h"

int coremain(void);

void main()
{

    printf("Core Mark Testing..\n");
    coremain();
    printf("Core Mark Test Finish\n");

    for(;;){
        ll_delay(1000);
    }

    while(1);


}