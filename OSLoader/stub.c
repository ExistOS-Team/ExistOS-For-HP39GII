#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/types.h>
#include <unistd.h>

#include "FreeRTOS.h"
#include "task.h"

#include "SystemConfig.h"
#include "uart_up.h"

#include "tusb.h"

#undef errno
extern int errno;
// extern int  __HEAP_START;

//! non-reentrant sbrk uses is actually reentrant by using current context
// ... because the current _reent structure is pointed to by global _impure_ptr
/*
char * sbrk(unsigned int incr) {

        return _sbrk_r(_impure_ptr, incr);
}
//! _sbrk is a synonym for sbrk.

char * _sbrk(unsigned int incr) {

        return sbrk(incr);
};
*/

/*
unsigned int __HEAP_START = HEAP_MEMORY;
unsigned char *heap = NULL;

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
*/

extern unsigned int __HEAP_START;
static void *heap = NULL;

caddr_t _sbrk(int incr) {
    void *prev_heap;

    if (heap == NULL) {
        heap = &__HEAP_START;
    }
    prev_heap = heap;
    if (((uint32_t)heap + incr) > HEAP_END) {
        printf("heap:%p, incr:%d\n", heap, incr);
        return 0;
    }
    heap += incr;

    return (caddr_t)prev_heap;
}

size_t xPortGetFreeHeapSize(void) {
    return HEAP_END - (uint32_t)heap;
}

int _close_r(struct _reent *pReent, int fd) {
    pReent->_errno = ENOTSUP;
    return -1;
}

int _execve_r(struct _reent *pReent, const char *filename, char *const *argv, char *const *envp) {
    pReent->_errno = ENOTSUP;
    return -1;
}

int _fcntl_r(struct _reent *pReent, int fd, int cmd, int arg) {
    pReent->_errno = ENOTSUP;
    return -1;
}

int _fork_r(struct _reent *pReent) {
    pReent->_errno = ENOTSUP;
    return -1;
}

int _fstat_r(struct _reent *pReent, int file, struct stat *st) {
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

int _kill_r(struct _reent *pReent, int pid, int signal) {
    pReent->_errno = ENOTSUP;
    return -1;
}

int _link_r(struct _reent *pReent, const char *oldfile, const char *newfile) {
    pReent->_errno = ENOTSUP;
    return -1;
}

_off_t _lseek_r(struct _reent *pReent, int file, _off_t offset, int whence) {
    pReent->_errno = ENOTSUP;
    return -1;
}

int _mkdir_r(struct _reent *pReent, const char *pathname, int mode) {
    pReent->_errno = ENOTSUP;
    return -1;
}

int _open_r(struct _reent *pReent, const char *file, int flags, int mode) {
    pReent->_errno = ENOTSUP;
    return -1;
}

_ssize_t _read_r(struct _reent *pReent, int fd, void *ptr, size_t len) {
    pReent->_errno = ENOTSUP;
    return -1;
}

_ssize_t _rename_r(struct _reent *pReent, const char *oldname, const char *newname) {
    pReent->_errno = ENOTSUP;
    return -1;
}

int _stat_r(struct _reent *pReent, const char *__restrict __path, struct stat *__restrict __sbuf) {
    pReent->_errno = ENOTSUP;
    return -1;
}

_CLOCK_T_ _times_r(struct _reent *pReent, struct tms *tbuf) {
    pReent->_errno = ENOTSUP;
    return -1;
}

int _unlink_r(struct _reent *pReent, const char *filename) {
    pReent->_errno = ENOTSUP;
    return -1;
}

int _wait_r(struct _reent *pReent, int *wstat) {
    pReent->_errno = ENOTSUP;
    return -1;
}

char log_buf[SYS_LOG_BUFSIZE] = {0};
uint32_t log_i = 0, log_j = 0;

extern uint32_t g_CDC_TransTo;
_ssize_t _write_r(struct _reent *pReent, int fd, const void *buf, size_t nbytes) {

    int i;

    if (fd < 3) {
        pReent->_errno = 0;
        int k = 0;
        while (k < nbytes) {
            log_buf[log_i++] = ((char *)buf)[k++];
            if (log_i > SYS_LOG_BUFSIZE) {
                log_i = 0;
            }
        }
        
        for (i = 0; i < nbytes; i++) {
            uart_putc(((char *)buf)[i]);
        }
        return i;
    }
    return -1;
}

int _gettimeofday_r(struct _reent *pReent, struct timeval *__tp, void *__tzp) {
    pReent->_errno = ENOTSUP;
    return -1;
}

/*
 * write "len" of char from "ptr" to file id "fd"
 * Return number of char written.
 *
 * Only work for STDOUT, STDIN, and STDERR
 */

int fsync(int __fd) {

    return 0;
}

char *getcwd(char *buf, size_t size) {

    return NULL;
}

void abort(void) {
    // Abort called
    while (1)
        ;
}

void _exit(int i) {

    while (1)
        ;
}