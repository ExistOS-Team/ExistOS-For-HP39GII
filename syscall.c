#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>

#include "regsuartdbg.h"
#include "console.h"

#undef errno
extern int errno;
int _end asm("end");

//extern int  __HEAP_START;


//malloc和sprintf等函数要用到
caddr_t _sbrk ( int incr ){
  static unsigned char *heap = NULL;
  unsigned char *prev_heap;
  if (heap == NULL) {
    heap = (unsigned char *)(0x70000);
  }
  prev_heap = heap;
  heap += incr;
  return (caddr_t) prev_heap;
}


/*
caddr_t _sbrk ( int incr )
{
	return 0;
	static unsigned char *heap = NULL;
	unsigned char *prev_heap;

	if (heap == NULL)
		{
			heap = (unsigned char *)&_end;
		}
	prev_heap = heap;

	heap += incr;


	return (caddr_t) prev_heap;
	
}*/

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

int _write(int fd, char *ptr, int len)
{
    int i = 0;

    /*
     * write "len" of char from "ptr" to file id "fd"
     * Return number of char written.
     *
    * Only work for STDOUT, STDIN, and STDERR
     */
    if (fd > 2)
    {
        return -1;
    }

    while (*ptr && (i < len))
    {

		console_puts(*ptr);
		
        i++;
        ptr++;
    }

    return i;
}

