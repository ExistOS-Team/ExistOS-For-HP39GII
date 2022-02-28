

extern unsigned int _sbss;
extern unsigned int _ebss;

extern unsigned int data_load_start;
extern unsigned int data_size;
extern unsigned int data_start;

void main();

void _init() __attribute__((section(".init"))) __attribute__((naked));
void _init()
{
    unsigned int *src = &data_load_start;
    unsigned int *dst = &data_start;
    unsigned int size = ((unsigned int)&data_size) / 4;
    
    __asm volatile("mov r13,#0x4F0000");

    for(char *i = (char *)&_sbss; i < (char *)&_ebss; i++){
		*i = 0;		//clear bss
	}

    for(unsigned int i = 0; i < size; i++){
        *dst++ = *src++;    //copy data
    }
    
         
    main();

    while(1);

}


