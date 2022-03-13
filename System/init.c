
#include <sys/types.h>

extern unsigned int _sbss;
extern unsigned int _ebss;

extern unsigned int data_load_start;
extern unsigned int data_size;
extern unsigned int data_start;

extern void (*__preinit_array_start []) (void) __attribute__((weak));
extern void (*__preinit_array_end []) (void) __attribute__((weak));
extern void (*__init_array_start []) (void) __attribute__((weak));
extern void (*__init_array_end []) (void) __attribute__((weak));

void main();

/*
void __libc_init_array (void)
{
  size_t count;
  size_t i;

  count = __preinit_array_end - __preinit_array_start;
  for (i = 0; i < count; i++)
    __preinit_array_start[i] ();



  count = __init_array_end - __init_array_start;
  for (i = 0; i < count; i++)
    __init_array_start[i] ();
}*/

void volatile ___init() __attribute__((section(".init"))) __attribute__((naked));
void volatile ___init()
{


    __asm volatile("mov r13,#0x02300000");
    __asm volatile("add r13,#0x000FA000");

    char *src = (char *)&data_load_start;
    char *dst = (char *)&data_start;
    unsigned int size = ((unsigned int)&data_size);
    




    for(unsigned int i = 0; i < size; i++){
        *dst++ = *src++;    //copy data
    }
    
    for(char *i = (char *)&_sbss; i < (char *)&_ebss; i++){
		  *i = 0;		//clear bss
	  }

    //__libc_init_array();



         

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
