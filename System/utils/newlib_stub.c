


#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/types.h>
#include <unistd.h>
#include <malloc.h>

#include "sys_llapi.h"

#include "FreeRTOS.h"
#include "SysConf.h"
#include "task.h"

#undef errno
extern int errno;

extern char __HEAP_START asm("__HEAP_START");

void __sync_synchronize() {
}

static void *heap = NULL;
extern uint32_t SwapMemorySize;
extern bool MemorySwapEnable;
extern uint32_t OnChipMemorySize;
static int cur_heap_loc = 0;
caddr_t _sbrk(uint32_t incr) {

    char *prev_heap;
    if (heap == NULL) {
        heap = &__HEAP_START;
    }
    prev_heap = heap;

    if (cur_heap_loc == 0) {
        if (((uint32_t)heap + incr) > RAM_BASE + OnChipMemorySize) {
            if (MemorySwapEnable) {
                heap = (void *)(RAM_BASE + OnChipMemorySize);
                cur_heap_loc = 1;
                prev_heap = heap;
                heap += incr;
                ll_put_str("EXT HEAP 2\n");
                goto success;
            } else {

                ll_put_str("EXT HEAP NOMEM !\n");
                errno = ENOMEM;
                goto fail;
            }
        } else {
            heap += incr;
        }
    } else if (cur_heap_loc == 1) {

        if (((uint32_t)heap + incr) > RAM_BASE + OnChipMemorySize + SwapMemorySize) {
            ll_put_str("SWAP HEAP NOMEM !\n");
            errno = ENOMEM;
            goto fail;
        } else {
            heap += incr;
        }
    }
success:
    return (caddr_t)prev_heap;
fail:

    return (caddr_t)-1;
}

size_t getOnChipHeapAllocated() {
    if ((uint32_t)heap > RAM_BASE + OnChipMemorySize) {
        return OnChipMemorySize;
    } else {
        return (uint32_t)heap - ((uint32_t)&__HEAP_START);
    }
}

size_t getSwapMemHeapAllocated() {
    if ((uint32_t)heap > RAM_BASE + OnChipMemorySize) {
        return (uint32_t)heap - RAM_BASE - OnChipMemorySize;

    } else {
        return 0;
    }
}

uint32_t getHeapAllocateSize() {
    struct mallinfo info = mallinfo();
    return info.uordblks;
}

/*
size_t xPortGetFreeHeapSize( void )
{
        return HEAP_END - (uint32_t)heap;
}

size_t xPortGetTotalHeapSize( void )
{
        return HEAP_END - (uint32_t)(&__HEAP_START);
}
*/

uint32_t getHeapAddr() {
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

int _isatty(int file) {
    return -1;
}

int _kill_r(struct _reent *pReent, int pid, int signal) {
    pReent->_errno = ENOTSUP;
    return -1;
}

_CLOCK_T_ _times_r(struct _reent *pReent, struct tms *tbuf) {
    pReent->_errno = ENOTSUP;
    // ll_putStr("GET TIME\n");
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
    // Abort called
    printf("abort\n");
    while (1)
        ;
}

void _exit(int i) {
    printf("_exit\n");
    while (1)
        ;
}

void *__wrap_malloc(size_t sz) {
    void *mem;
    vTaskSuspendAll();

    // ll_put_str("malloc\n");

    mem = _malloc_r(_impure_ptr, sz);
    (void)xTaskResumeAll();

    return mem;
}

void __wrap_free(void *m) {
    // ll_put_str("free\n");

    vTaskSuspendAll();

    _free_r(_impure_ptr, m);

    (void)xTaskResumeAll();
}