
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdarg.h>

#include "py/obj.h"
#include "py/compile.h"
#include "py/runtime.h"
#include "py/stackctrl.h"
#include "py/gc.h"
#include "py/repl.h"
#include "py/mperrno.h"
#include "py/stream.h"

#include "mpconfigport.h"

typedef struct mpy_cb
{
    mp_uint_t (*mp_hal_ticks_ms)(void);
    void (*mp_hal_delay_us)(uint32_t us);
    void (*mp_hal_set_interrupt_char)(int c);
    void (*mp_hal_stdout_tx_strn)(const char *str, mp_uint_t len);
    int (*mp_hal_stdin_rx_chr)(void);
    uintptr_t (*mp_hal_stdio_poll)(uintptr_t poll_flags);
}mpy_cb;

mpy_cb curcbs;

int DEBUG_printf(const char *fmt, ...)
{
    int cnt;
    va_list args;
    va_start(args, fmt);
    cnt = vprintf(fmt, args);
    va_end(args);
    return cnt;
}

mp_lexer_t *mp_lexer_new_from_file(const char *filename) {
    mp_raise_OSError(MP_ENOENT);
}

/*
mp_uint_t mp_hal_ticks_ms(void) {
    if(curcbs.mp_hal_ticks_ms)
    {
        return curcbs.mp_hal_ticks_ms();
    }
    return 0;
}

void mp_hal_delay_us(uint32_t us) 
{
    if(curcbs.mp_hal_delay_us)
    {
        curcbs.mp_hal_delay_us(us);
    }
}

void mp_hal_set_interrupt_char(int c) {
    if(curcbs.mp_hal_set_interrupt_char)
    {
        curcbs.mp_hal_set_interrupt_char(c);
    }
}

void mp_hal_stdout_tx_strn(const char *str, mp_uint_t len) 
{
    if(curcbs.mp_hal_stdout_tx_strn)
    {
        curcbs.mp_hal_stdout_tx_strn(str, len);
    }
}

int mp_hal_stdin_rx_chr(void)
{
    if(curcbs.mp_hal_stdin_rx_chr)
    {
        return curcbs.mp_hal_stdin_rx_chr();
    }
    return 0;
}

uintptr_t mp_hal_stdio_poll(uintptr_t poll_flags) {
    uintptr_t ret = 0;

    if(curcbs.mp_hal_stdio_poll)
    {
        return curcbs.mp_hal_stdio_poll(poll_flags);
    }

    if (0) {
        ret |= MP_STREAM_POLL_RD;
    }
    return ret;
}

*/



mp_import_stat_t mp_import_stat(const char *path) {
    return MP_IMPORT_STAT_NO_EXIST;
}

void nlr_jump_fail(void *val) {
    uint32_t *s = val;
    s -= 20;
    for(int i = 0; i < 40; i++)
    {
        printf("[%08x]:%08x\n", &s[i], s[i]);
    }
    printf("FATAL: uncaught NLR %p\n", val);
    exit(1);
}


static const char *demo_single_input =
    "print('hello world!', list(x + 1 for x in range(10)), end='eol\\n')";

static const char *demo_file_input =
    "import micropython\n"
    "\n"
    "print(dir(micropython))\n"
    "\n"
    "for i in range(10):\n"
    "    print('iter {:08}'.format(i))";

static void do_str(const char *src, mp_parse_input_kind_t input_kind) {
    nlr_buf_t nlr;
    memset(&nlr, 0, sizeof(nlr));
    if (nlr_push(&nlr) == 0) {
        // Compile, parse and execute the given string.
        mp_lexer_t *lex = mp_lexer_new_from_str_len(MP_QSTR__lt_stdin_gt_, src, strlen(src), 0);
        qstr source_name = lex->source_name;
        mp_parse_tree_t parse_tree = mp_parse(lex, input_kind);
        mp_obj_t module_fun = mp_compile(&parse_tree, source_name, true);
        mp_call_function_0(module_fun);
        nlr_pop();
    } else {
        // Uncaught exception: print it out.
        mp_obj_print_exception(&mp_plat_print, (mp_obj_t)nlr.ret_val);
    }
}


static mp_obj_t pystack[1024];

static char *stack_top;
void mpy_init(uint8_t *heap, uint32_t heap_size, mpy_cb *mpycb)
{
    int stack_dummy;
    stack_top = (char *)&stack_dummy;

/*
    if(!mpycb)
    {
        return;
    }
    curcbs.mp_hal_delay_us = mpycb->mp_hal_delay_us;
    curcbs.mp_hal_set_interrupt_char = mpycb->mp_hal_set_interrupt_char;
    curcbs.mp_hal_stdin_rx_chr = mpycb->mp_hal_stdin_rx_chr;
    curcbs.mp_hal_stdio_poll = mpycb->mp_hal_stdio_poll;
    curcbs.mp_hal_stdout_tx_strn = mpycb->mp_hal_stdout_tx_strn;
    curcbs.mp_hal_ticks_ms = mpycb->mp_hal_ticks_ms;
*/
    mp_stack_ctrl_init();
    mp_stack_set_limit(16384 *4);
    gc_init(heap, heap + heap_size);

    #if MICROPY_ENABLE_PYSTACK
    mp_pystack_init(pystack, &pystack[1024]);
    #endif

    mp_init();

    
    do_str(demo_single_input, MP_PARSE_SINGLE_INPUT);
    //do_str(demo_file_input, MP_PARSE_FILE_INPUT);


}


#if MICROPY_ENABLE_GC
/*
uint32_t
    __attribute__((naked))
    __get_sp() {
    asm volatile("mov r0,sp");
    asm volatile("bx lr");
}*/

void gc_collect(void) {
    // WARNING: This gc_collect implementation doesn't try to get root
    // pointers from CPU registers, and thus may function incorrectly.
    void *dummy;
    gc_collect_start();
    gc_collect_root(&dummy, ((mp_uint_t)stack_top - (mp_uint_t)&dummy) / sizeof(mp_uint_t));
    gc_collect_end();
    gc_dump_info();
}
#endif



mp_obj_t mp_builtin_open(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(mp_builtin_open_obj, 1, mp_builtin_open);
