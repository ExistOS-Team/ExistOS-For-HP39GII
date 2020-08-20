/*
Created by rollingQP
2020.8.19

To record how to reboot(reset)

*/
#include <reboot.h>

void reboot_test(int option){
    if(option == 1 || option == 2)
    {
        HW_CLKCTRL_RESET = option;
    }else{
        return;
    }
}
