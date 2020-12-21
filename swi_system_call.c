

#include "uart_debug.h"



void src_c_swi_handler(unsigned int *arg0, unsigned int *arg1, unsigned int *arg2, unsigned int swiImmed){
	/*
	if(swiImmed != 0)
		printf("SWI: %d, arg0:%d, arg1:%d, arg2:%d\n",swiImmed,arg0,arg1,arg2);
	
	*/
	switch(swiImmed){
		case 1000:
			*arg2 = 55;
			printf("arg0 :%d\n",(*arg0));
			
	}
	
}