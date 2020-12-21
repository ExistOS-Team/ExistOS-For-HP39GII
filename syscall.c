#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>

#include "regsuartdbg.h"
#include "uart_debug.h"
#include "console.h"

#undef errno
extern int errno;
int _end asm("end");

//extern int  __HEAP_START;

unsigned int __HEAP_START = 0x38000;
unsigned char *heap = NULL;

//malloc和sprintf等函数要用到
caddr_t _sbrk ( int incr ){
  unsigned char *prev_heap;
  if (heap == NULL) {
    heap = (unsigned char *)(__HEAP_START);
  }
  prev_heap = heap;
  heap += incr;
  uartdbg_printf("heap %x\n",heap);
  return (caddr_t) prev_heap;
}
 

int link(char *old, char *new)
{
	return -1;
}

int _close(int file)
{
	return -1;
}

int _fstat(int file, struct stat *st)
{
	st->st_mode = S_IFCHR;
	return 0;
}

int _isatty(int file)
{
	return 1;
}

int _lseek(int file, int ptr, int dir)
{
	return 0;
}

int _read(int file, char *ptr, int len)
{
	return 0;
}


void _exit(int i){
	
	while(1);
}

void _open(){
	
	
}

void __sync_synchronize(){
	
	
}

void abort(void)
{
	//Abort called 
	while(1);
}

int fputc(int ch, FILE *f)
{
	while(1);
}


/*
 * write "len" of char from "ptr" to file id "fd"
 * Return number of char written.
 *
* Only work for STDOUT, STDIN, and STDERR
 */

char kmsgBuff[4096];
unsigned int kmsgBuff_ptr = 0;

int _write(int fd, char *ptr, int len)
{
	/*
	asm volatile ("ldr r0,%0" :: "m"(fd));
	asm volatile ("ldr r1,%0" :: "m"(ptr));
	asm volatile ("ldr r2,%0" :: "m"(len));
	asm volatile ("swi #1000");
	*/
	
    int i = 0;


    if (fd > 2)
    {
        return -1;
    }

    while (*ptr && (i < len))
    {
		kmsgBuff[kmsgBuff_ptr++] = *ptr;
		if(kmsgBuff_ptr > 4096){
			kmsgBuff_ptr = 0;
		}
		uartdbg_putc(*ptr);
		//console_puts(*ptr);
		
        i++;
        ptr++;
    }

    return i;
}

