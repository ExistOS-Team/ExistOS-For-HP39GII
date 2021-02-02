#include "swi_system_call.h"
#include "exception.h"
#include "mmu.h"
#include "syscall.h"
#include "uart_debug.h"
#include <reent.h>
#include <stdio.h>

unsigned int src_c_swi_handler(unsigned int r0, unsigned int r1, unsigned int r2,
                               unsigned int r3, unsigned int r4, unsigned int r5, unsigned int swiImmed) {
    switch (swiImmed) {
    // 20 functions from newlib. wait and isatty are not included here.
    case SWI_CLOSE:
        return (unsigned int)_close_r(_impure_ptr, (int)r0);
    case SWI_EXECVE:
        return (unsigned int)_execve_r(_impure_ptr, (const char *)r0, (char *const *)r1, (char *const *)r2);
    case SWI_FCNTL:
        return (unsigned int)_fcntl_r(_impure_ptr, (int)r0, (int)r1, (int)r2);
    case SWI_FORK:
        return (unsigned int)_fork_r(_impure_ptr);
    case SWI_FSTAT:
        return (unsigned int)_fstat_r(_impure_ptr, (int)r0, (struct stat *)r1);
    case SWI_GETPID:
        return (unsigned int)_getpid_r(_impure_ptr);
    case SWI_GETTIMEOFDAY:
        return (unsigned int)_gettimeofday_r(_impure_ptr, (struct timeval *)r0, (void *)r1);
    case SWI_KILL:
        return (unsigned int)_kill_r(_impure_ptr, (int)r0, (int)r1);
    case SWI_LINK:
        return (unsigned int)_link_r(_impure_ptr, (const char *)r0, (const char *)r1);
    case SWI_LSEEK:
        return (unsigned int)_lseek_r(_impure_ptr, (int)r0, (_off_t)r1, (int)r2);
    case SWI_MKDIR:
        return (unsigned int)_mkdir_r(_impure_ptr, (const char *)r0, (int)r1);
    case SWI_OPEN:
        return (unsigned int)_open_r(_impure_ptr, (const char *)r0, (int)r1, (int)r2);
    case SWI_READ:
        return (unsigned int)_read_r(_impure_ptr, (int)r0, (void *)r1, (size_t)r2);
    case SWI_RENAME:
        return (unsigned int)_rename_r(_impure_ptr, (const char *)r0, (const char *)r1);
    case SWI_BRK:
        return (unsigned int)_sbrk_r(_impure_ptr, (ptrdiff_t)r0);
    case SWI_STAT:
        return (unsigned int)_stat_r(_impure_ptr, (const char *)r0, (struct stat *)r1);
    case SWI_TIMES:
        return (unsigned int)_times_r(_impure_ptr, (struct tms *)r0);
    case SWI_UNLINK:
        return (unsigned int)_unlink_r(_impure_ptr, (const char *)r0);
    case SWI_WRITE:
        return (unsigned int)_write_r(_impure_ptr, (int)r0, (const void *)r1, (size_t)r2);

    // Other system calls
    case SWI_FSYNC:
        return (unsigned int)fsync((int)r0);
    case SWI_GETCWD:
        return (unsigned int)getcwd((char *)r0, (size_t)r1);
    default:
        return -1;
    }
}