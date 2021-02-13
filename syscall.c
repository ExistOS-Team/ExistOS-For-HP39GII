#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/times.h>

#include "console.h"
#include "memory_map.h"
#include "regsuartdbg.h"
#include "uart_debug.h"

#include "FreeRTOS.h"
#include "task.h"
#include "vfsman.h"

#include "regsdigctl.h"

#undef errno
extern int errno;
//extern int  __HEAP_START;

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

int _close_r(struct _reent *pReent, int fd) {
    int fr;
    vTaskSuspendAll();
    fr = vfs_fclose(fd);
    pReent->_errno = fr;
    xTaskResumeAll();
    return fr;
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
    int fr;
    vTaskSuspendAll();
    if(file > 2){
        fr = vfs_fstat(file, st);
        pReent->_errno = fr;
    }else{
        switch (file)
        {
        case STDOUT_FILENO:
            st->st_size = 2048;
            st->st_mode = S_IFCHR;
            st->st_blksize = 2048;
            st->st_blocks = 1;
            pReent->_errno = 0;
            fr = 0;
            break;
        case STDIN_FILENO:
            break;

        default:
            break;
        }
    }
    xTaskResumeAll();
    return fr;
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
    int fr;
    vTaskSuspendAll();
    fr = vfs_lseek(file, offset, whence);
    pReent->_errno = fr;
    xTaskResumeAll();
    return fr;
}

int _mkdir_r(struct _reent *pReent, const char *pathname, int mode) {
    int fr;
    vTaskSuspendAll();
    fr = vfs_mkdir(pathname);
    pReent->_errno = fr;
    xTaskResumeAll();
    return fr;
}

int _open_r(struct _reent *pReent, const char *file, int flags, int mode) {
    int fr;
    vTaskSuspendAll();
    fr = vfs_open(file, flags, mode);
    pReent->_errno = fr;
    xTaskResumeAll();
    return fr;
}

_ssize_t _read_r(struct _reent *pReent, int fd, void *ptr, size_t len) {
    int fr;
    vTaskSuspendAll();
    fr = vfs_read(fd, ptr, len);
    pReent->_errno = fr;
    xTaskResumeAll();
    return fr;
}

_ssize_t _rename_r(struct _reent *pReent, const char *oldname, const char *newname) {
    int fr;
    vTaskSuspendAll();
    fr = vfs_rename(oldname, newname);
    pReent->_errno = fr;
    xTaskResumeAll();
    return fr;
}

void *_sbrk_r(struct _reent *pReent, int incr);

int _stat_r(struct _reent *pReent, const char *__restrict __path, struct stat *__restrict __sbuf )
{
    int fr;
    vTaskSuspendAll();
    fr = vfs_stat(__path, __sbuf);
    pReent->_errno = fr;
    xTaskResumeAll();
    return fr;
}

_CLOCK_T_ _times_r(struct _reent *pReent, struct tms *tbuf) {
    unsigned int fr;
    tbuf->tms_cstime = HW_DIGCTL_MICROSECONDS_RD();
    tbuf->tms_cutime = HW_DIGCTL_MICROSECONDS_RD();
    tbuf->tms_stime = HW_DIGCTL_MICROSECONDS_RD();
    tbuf->tms_utime = HW_DIGCTL_MICROSECONDS_RD();
    fr = HW_DIGCTL_MICROSECONDS_RD();
    pReent->_errno = 0;
    return fr;
}

int _unlink_r(struct _reent *pReent, const char *filename) {
    pReent->_errno = ENOTSUP;
    return -1;
}

int _wait_r(struct _reent *pReent, int *wstat) {
    pReent->_errno = ENOTSUP;
    return -1;
}

_ssize_t _write_r(struct _reent *pReent, int fd, const void *buf, size_t nbytes) {
    vTaskSuspendAll();
    int i = 0;
    char *ptr = (char *)buf;

    if (fd > 2) {

        i = vfs_write(fd, buf, nbytes);
        pReent->_errno = i;

    } else {
        // STDOUT, STDIN, STDERR
        while (*ptr && (i < nbytes)) {
            uartdbg_putc(*ptr);
            i++;
            ptr++;
        }
    }
    xTaskResumeAll();
    //uartdbg_print_regs();
    return i;
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
/*
char kmsgBuff[4096];
unsigned int kmsgBuff_ptr = 0;

int _write(int fd, char *ptr, int len) {
    
	//asm volatile ("ldr r0,%0" :: "m"(fd));
	//asm volatile ("ldr r1,%0" :: "m"(ptr));
	//asm volatile ("ldr r2,%0" :: "m"(len));
	//asm volatile ("swi #1000");
	

    int i = 0;

    if (fd > 2) {
        return -1;
    }

    while (*ptr && (i < len)) {
        kmsgBuff[kmsgBuff_ptr++] = *ptr;
        if (kmsgBuff_ptr > 4096) {
            kmsgBuff_ptr = 0;
        }
        uartdbg_putc(*ptr);
        //console_puts(*ptr);

        i++;
        ptr++;
    }

    return i;
}
*/

int fsync(int __fd) {
    int fr;
    vTaskSuspendAll();
    fr = vfs_fsync(__fd);
    xTaskResumeAll();
    return fr;
}

char *getcwd(char *buf, size_t size) {
    int fr;
    vTaskSuspendAll();
    fr = vfs_getcwd(buf, size);
    xTaskResumeAll();
    if (fr != 0) {
        return NULL;
    }
    return buf;
}

void abort(void) {
    //Abort called
    while (1)
        ;
}

void _exit(int i) {

    while (1)
        ;
}