#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/times.h>
#include <stdlib.h>

#include "sys_llapi.h"

#include "FreeRTOS.h"
#include "task.h"
#include "SysConf.h"

#undef errno
extern int errno;

#define HEAP_END    (KERNEL_HEAP_END)


//extern unsigned int __HEAP_START;
static void *heap = NULL;
extern char __HEAP_START asm("__HEAP_START");
//void __sync_synchronize()  __attribute__((naked));
void __sync_synchronize()
{
    //asm volatile("" : : : "memory");
}

caddr_t _sbrk ( uint32_t incr )
{
    
    
	char *prev_heap;
    if(heap == NULL){
        heap = &__HEAP_START;
    }
    prev_heap = heap;
    
    if(((uint32_t)heap + incr) > HEAP_END){
        //dbg_printf("MEMORY OVER FLOW\n");
        return (caddr_t) -1;
    }
    heap += incr;
    //dbg_printf("heap:%x, incr:%d\n", heap, incr);
    return (caddr_t)prev_heap;

}


size_t xPortGetFreeHeapSize( void )
{
	return HEAP_END - (uint32_t)heap;
}

size_t xPortGetTotalHeapSize( void )
{
	return HEAP_END - (uint32_t)(&__HEAP_START);
}

uint32_t getHeapAddr()
{
    return (uint32_t)heap;
}

int _close_r(struct _reent *pReent, int fd) {
    pReent->_errno = ENOTSUP;
    return -1;
}

int _execve_r(struct _reent *pReent, const char *filename, char *const *argv, char *const *envp) {
    pReent->_errno = ENOTSUP;
    return -1;
}


int _fork_r(struct _reent *pReent) {
    pReent->_errno = ENOTSUP;
    return -1;
}


int _getpid_r(struct _reent *pReent) {
    pReent->_errno = ENOTSUP;
    return -1;
}

int _isatty_r(struct _reent *pReent, int file) {
    pReent->_errno = ENOTSUP;
    return -1;
}

int _isatty(int file)
{
    return -1;
}

int _kill_r(struct _reent *pReent, int pid, int signal) {
    pReent->_errno = ENOTSUP;
    return -1;
}

_CLOCK_T_ _times_r(struct _reent *pReent, struct tms *tbuf) {
    pReent->_errno = ENOTSUP;
    //ll_putStr("GET TIME\n");
    return -1;
}

int _wait_r(struct _reent *pReent, int *wstat) {
    pReent->_errno = ENOTSUP;
    return -1;
}


int _gettimeofday_r(struct _reent *pReent, struct timeval *__tp, void *__tzp) {
    pReent->_errno = ENOTSUP;

    printf("GET _gettimeofday_r\n");
    return -1;
}


char *getcwd(char *buf, size_t size) {

    return NULL;
}

void abort(void) {
    //Abort called
    printf("abort\n");
    while (1)
        ;
}

void _exit(int i) {
    printf("_exit\n");
    while (1)
        ;

}


void *__wrap_malloc(size_t sz)
{
    void *mem;
	vTaskSuspendAll();

    //ll_put_str("malloc\n");

    mem = _malloc_r(_impure_ptr, sz);
    ( void ) xTaskResumeAll();

    return mem;
    
}

void __wrap_free(void *m)
{
    //ll_put_str("free\n");
    
	vTaskSuspendAll();

    _free_r(_impure_ptr, m);
    
    ( void ) xTaskResumeAll();
}