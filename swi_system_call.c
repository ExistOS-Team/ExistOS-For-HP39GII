#include "swi_system_call.h"
#include "exception.h"
#include "mmu.h"
#include "syscall.h"
#include "uart_debug.h"
#include <reent.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "ProcessesMan.h"

struct _reent *_impure_ptr;

unsigned int src_c_swi_handler(unsigned int r0, unsigned int r1, unsigned int r2,
                               unsigned int r3, unsigned int r4, unsigned int r5, unsigned int r7) {
    unsigned int ret_value;
	//printf("SYSCALL: r0:%x r1:%x r2:%x r3:%x r4:%x r5:%x r7:%x\n",r0,r1,r2,r3,r4,r5,r7);
    switch (r7) {
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
        ret_value = (unsigned int)_getpid_r(_impure_ptr);
        uartdbg_printf("GET_PID:%x\n",ret_value);
        return ret_value;
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
        ret_value = (unsigned int)process_sbrk(get_current_running_task_pid(), r0);
		//uartdbg_print_regs();
		printf("sbrk ins:%08x rt:%08x\n",r0,ret_value);
        return ret_value;
		//return (unsigned int)pvPortMalloc(r0);
		//return 0x81000;
        //return (unsigned int)_sbrk_r(_impure_ptr, (ptrdiff_t)r0);



    case SWI_STAT:
        return (unsigned int)_stat_r(_impure_ptr, (const char *)r0, (struct stat *)r1);
    case SWI_TIMES:
        return (unsigned int)_times_r(_impure_ptr, (struct tms *)r0);
    case SWI_UNLINK:
        return (unsigned int)_unlink_r(_impure_ptr, (const char *)r0);
    case SWI_WRITE:
		return (unsigned int)_write_r(_impure_ptr, (int)r0, (const void *)r1, (size_t)r2);
	case SWI_EXIT:
		printf("API: task exit.\n");
		vTaskDelete(NULL);

    // Other system calls
    case SWI_FSYNC:
        return (unsigned int)fsync((int)r0);
    case SWI_GETCWD:
        return (unsigned int)getcwd((char *)r0, (size_t)r1);

	case SWI_NANOSLEEP:
        printf("API: SLEEP:%d ms.\n",r0);
		vTaskDelay(r0);
		return 0;

    default:
        uartdbg_printf("UNKNOWN SYSCALL: r0:%x r1:%x r2:%x r3:%x r4:%x r5:%x r7:%x\n",r0,r1,r2,r3,r4,r5,r7);
        return -1;
    }
}