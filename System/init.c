
#include <sys/types.h>

extern unsigned int _sbss;
extern unsigned int _ebss;

extern unsigned int data_load_start;
extern unsigned int data_size;
extern unsigned int data_start;

void main();
void __libc_init_array(void);


void _init() __attribute__((section(".init"))) __attribute__((naked));
void _init()
{
    unsigned int *src = &data_load_start;
    unsigned int *dst = &data_start;
    unsigned int size = ((unsigned int)&data_size) / 4;
    
    __asm volatile("mov r13,#0x02300000");

    for(char *i = (char *)&_sbss; i < (char *)&_ebss; i++){
		*i = 0;		//clear bss
	}

    for(unsigned int i = 0; i < size; i++){
        *dst++ = *src++;    //copy data
    }
    
    //__libc_init_array();


    typedef void(*pfunc)();
    extern pfunc __ctors_start__[];
    extern pfunc __ctors_end__[];
    pfunc *p;

    for (p = __ctors_start__; p < __ctors_end__; p++)
        (*p)();
         
    main();



    while(1);

}

extern void (*__fini_array_start []) (void);
extern void (*__fini_array_end []) (void);

void __libc_fini_array (void)
{
  size_t count;
  size_t i;
  
  count = __fini_array_end - __fini_array_start;
  for (i = count; i > 0; i--)
    __fini_array_start[i-1] ();

  
}
