#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>

#include "regsuartdbg.h"

#undef errno
extern int errno;
int _end asm("end");

caddr_t _sbrk ( int incr )
{
	static unsigned char *heap = NULL;
	unsigned char *prev_heap;

	if (heap == NULL)
		{
			heap = (unsigned char *)&_end;
		}
	prev_heap = heap;

	heap += incr;

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


void abort(void)
{
	//Abort called 
	while(1);
}

int fputc(int ch, FILE *f)
{
	while(1);
}

int _write(int file, char *ptr, int len)
{
	uint16_t todo;

	int loop = 0;
	for(todo = 0; todo < len; todo++)
		{
			while (HW_UARTDBGFR_RD()&BM_UARTDBGFR_TXFF)
				{
					loop++;
					if (loop > 10000)
						break;
				};

			if(!(HW_UARTDBGFR_RD() &BM_UARTDBGFR_TXFF))
				HW_UARTDBGDR_WR(*ptr++);
		}

	return *ptr;
}
