

extern unsigned int _sbss;
extern unsigned int _ebss;

void main();

void _init() __attribute__((section(".init"))) __attribute__((naked));
void _init()
{

    __asm volatile("mov r13,#0x4F0000");

    for(char *i = (char *)&_sbss; i < (char *)&_ebss; i++){
		*i = 0;		//clear bss
	}

    
         
    main();

    while(1);

}


