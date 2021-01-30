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
        return _close_r(_impure_ptr, r0);
    case SWI_EXECVE:
        return _execve_r(_impure_ptr, r0, r1, r2);
    case SWI_FCNTL:
        return _fcntl_r(_impure_ptr, r0, r1, r2);
    case SWI_FORK:
        return _fork_r(_impure_ptr);
    case SWI_FSTAT:
        return _fstat_r(_impure_ptr, r0, r1);
    case SWI_GETPID:
        return _getpid_r(_impure_ptr);
    case SWI_GETTIMEOFDAY:
        return _gettimeofday_r(_impure_ptr, r0, r1);
    case SWI_KILL:
        return _kill_r(_impure_ptr, r0, r1);
    case SWI_LINK:
        return _link_r(_impure_ptr, r0, r1);
    case SWI_LSEEK:
        return _lseek_r(_impure_ptr, r0, r1, r2);
    case SWI_MKDIR:
        return _mkdir_r(_impure_ptr, r0, r1);
    case SWI_OPEN:
        return _open_r(_impure_ptr, r0, r1, r2);
    case SWI_READ:
        return _read_r(_impure_ptr, r0, r1, r2);
    case SWI_RENAME:
        return _rename_r(_impure_ptr, r0, r1);
    case SWI_BRK:
        return _sbrk_r(_impure_ptr, r0);
    case SWI_STAT:
        return _stat_r(_impure_ptr, r0, r1);
    case SWI_TIMES:
        return _times_r(_impure_ptr, r0);
    case SWI_UNLINK:
        return _unlink_r(_impure_ptr, r0);
    case SWI_WRITE:
        return _write_r(_impure_ptr, r0, r1, r2);

    // Other system calls
    case SWI_FSYNC:
        return fsync(r0);
    case SWI_GETCWD:
        return getcwd(r0, r1);
    default:
        return -1;
    }
}