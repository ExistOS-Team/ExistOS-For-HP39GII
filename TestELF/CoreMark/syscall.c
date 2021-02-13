#include <stddef.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/times.h>
#include "swi_system_call.h"


volatile unsigned int swi(register unsigned int regs) __attribute__((naked));
volatile unsigned int swi(register unsigned int regs)
{
    __asm__ volatile ("":::"memory");
    __asm volatile("swi #0" :: "r"(regs));
}


volatile unsigned int syscall(unsigned int r0, unsigned int r1, unsigned int r2,
	unsigned int r3, unsigned int r4, unsigned int r5, unsigned int r7)
{
    struct regs{
        unsigned int r0;
        unsigned int r1;
        unsigned int r2;
        unsigned int r3;
        unsigned int r4;
        unsigned int r5;
        unsigned int r7;
    } regs;
    __asm__ volatile ("":::"memory");
    regs.r0 = r0;
    regs.r1 = r1;
    regs.r2 = r2;
    regs.r3 = r3;
    regs.r4 = r4;
    regs.r5 = r5;
    regs.r7 = r7;
    register unsigned int par __asm__("r0");
    par = (unsigned int)&regs;
    __asm__ volatile ("":::"memory");
    swi(par);
    return regs.r0;
}

_off_t _lseek(int file, _off_t offset, int whence) {
	return syscall(file,offset,whence,0,0,0,SWI_LSEEK);
}

int _close(int fd) {
	return syscall(fd,0,0,0,0,0,SWI_CLOSE);
}
 
void *_sbrk (ptrdiff_t incr)
{
	return (void *)syscall(incr,0,0,0,0,0,SWI_BRK);
}

int _write(int r0, const void *r1, size_t r2){
	return syscall(r0,(unsigned int)r1,r2,0,0,0,SWI_WRITE);
}

_ssize_t _read(int fd, void *ptr, size_t len) {
    return syscall(fd,(unsigned int)ptr,len,0,0,0,SWI_READ);
}

int _kill (int pid, int sig){
	return syscall(pid,sig,0,0,0,0,SWI_KILL);
}

int _getpid(void){
	return syscall(0,0,0,0,0,0,SWI_GETPID);
}

int _fstat(int file, struct stat *st) {
	return syscall(file,(unsigned int)st,0,0,0,0,SWI_FSTAT);
}

clock_t _times (struct tms * t)
{
	return syscall((unsigned int)t,0,0,0,0,0,SWI_TIMES);
}

int _isatty(int file) {
    return 1;
}

void _exit(int return_code){
	syscall(return_code,0,0,0,0,0,SWI_EXIT);
	while(1);
}

int get_us_count(){
	return syscall(0,0,0,0,0,0,SWI_GET_US_COUNT);
}


void sleep(int ms){
	syscall(ms,0,0,0,0,0,SWI_NANOSLEEP);
}