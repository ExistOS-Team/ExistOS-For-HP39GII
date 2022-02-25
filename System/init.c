

extern unsigned int _sbss;
extern unsigned int _ebss;

void main();

void _init() __attribute__((section(".init"))) __attribute__((naked));
void _init()
{

    for(char *i = (char *)&_sbss; i < (char *)&_ebss; i++){
		*i = 0;		//clear bss
	}
         
    main();

    while(1);

}


