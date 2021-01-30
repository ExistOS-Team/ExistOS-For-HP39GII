#include <stdarg.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "console.h"
#include "memory_map.h"
#include "regsuartdbg.h"
#include "uart_debug.h"

#include "vfsman.h"
#include "FreeRTOS.h"
#include "task.h"

#undef errno
extern int errno;
//extern int  __HEAP_START;

void *_sbrk_r(struct _reent *pReent, int incr);
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

int link(char *old, char *new) {
    return -1;
}


int _close_r(struct _reent *pReent, int fd){
    int fr;
    vTaskSuspendAll();
    fr = vfs_fclose(fd);
    pReent->_errno = fr;
    xTaskResumeAll();
    return fr;
}

int _fstat_r(struct _reent *pReent, int file, struct stat *st) {

    int fr;
    vTaskSuspendAll();

    //st->st_mode = S_IFCHR;
    fr = vfs_fstat(file,st);
    pReent->_errno = fr;

    xTaskResumeAll();
    return fr;
}

int _stat(const char *path, struct stat *st){
    return vfs_stat(path,st);
}

int _isatty(int file) {
    return 1;
}

int fsync (int __fd){
    int fr;
    //vTaskSuspendAll();
    fr = vfs_fsync(__fd);
    //xTaskResumeAll();
    return fr;
}

_off_t _lseek_r(struct _reent *pReent, int file, _off_t offset, int whence) {
    int fr;
    vTaskSuspendAll();

    fr = vfs_lseek(file,offset,whence);
    pReent->_errno = fr;

    xTaskResumeAll();
    return fr;
}


_ssize_t _read_r(struct _reent *pReent,
    int fd, void *ptr, size_t len){
    int fr;
    vTaskSuspendAll();

    fr = vfs_read(fd,ptr,len);
    pReent->_errno = fr;
    xTaskResumeAll();
    return fr;
}

char *getcwd(char *buf, size_t size){
    
    return NULL;

}

int mkdir(const char *pathname, mode_t mode){

    int fr;
    vTaskSuspendAll();

    vfs_mkdir(pathname);

    xTaskResumeAll();
    return fr;
}

void _exit(int i) {

    while (1)
        ;
}

int _open_r(struct _reent *pReent,
    const char *file, int flags, int mode){

        int fr;

        vTaskSuspendAll();

        fr = vfs_open(file, flags, mode);
        pReent->_errno = fr;

        xTaskResumeAll();

        return fr;
    }
    
/*
void __sync_synchronize() {
}
*/

void abort(void) {
    //Abort called
    while (1)
        ;
}

_ssize_t _write_r(struct _reent *pReent, int fd, const void *buf, size_t nbytes)
{
    vTaskSuspendAll();
    int i = 0;
    char *ptr = (char *)buf;

    if(fd > 2){

        i = vfs_write(fd, buf, nbytes);
        pReent->_errno = i;

    }else{
        // STDOUT, STDIN, STDERR
        while (*ptr && (i < nbytes)) {
            uartdbg_putc(*ptr);
            i++;
            ptr++;
        }   
    }
    xTaskResumeAll();
    return i;

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