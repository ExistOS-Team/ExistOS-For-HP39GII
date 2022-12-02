#pragma once
#include "py/obj.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct mpy_cb
{
    mp_uint_t (*mp_hal_ticks_ms)(void);
    void (*mp_hal_delay_us)(uint32_t us);
    void (*mp_hal_set_interrupt_char)(int c);
    void (*mp_hal_stdout_tx_strn)(const char *str, mp_uint_t len);
    int (*mp_hal_stdin_rx_chr)(void);
    uintptr_t (*mp_hal_stdio_poll)(uintptr_t poll_flags);
}mpy_cb;


void mpy_init(uint8_t *heap, uint32_t heap_size, mpy_cb *mpycb);

