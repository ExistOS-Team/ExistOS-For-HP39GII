/*
 * Boot Prep common file
 *
 * Copyright 2008-2009 Freescale Semiconductor
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#include "mmu.h"
#include "regsuartdbg.h"
#include "utils.h"
#include <stdarg.h>
#include "uart_debug.h"
volatile unsigned int dump_regs[18];

void uartdbg_dump_regs() __attribute__((naked));
void uartdbg_dump_regs()
{
    asm volatile("push {R0-R12,LR}");
    asm volatile("ldr R12,=dump_regs");
    asm volatile("stmia r12,{r0-lr}");
    asm volatile("pop {R0-R12,LR}");
    asm volatile("bx lr");
}

void uartdbg_print_dumpregs(){
    uartdbg_printf("R0  =%X,  R1 =%X, R2  =%X, R3  =%X\n", dump_regs[0], dump_regs[1], dump_regs[2], dump_regs[3]);
    uartdbg_printf("R4  =%X,  R5 =%X, R6  =%X, R7  =%X\n", dump_regs[4], dump_regs[5], dump_regs[6], dump_regs[7]);
    uartdbg_printf("R8  =%X,  R9 =%X, R10 =%X, R11 =%X\n", dump_regs[8], dump_regs[9], dump_regs[10], dump_regs[11]);
    uartdbg_printf("R12 =%X, R13 =%X, R14 =%X, R15 =%X\n\n", dump_regs[12], dump_regs[13], dump_regs[14], dump_regs[15]);
}

void uartdbg_putc(char ch) {
    int loop = 0;
    while (HW_UARTDBGFR_RD() & BM_UARTDBGFR_TXFF) {
        loop++;
        if (loop > 10000)
            return;
    };

    if (!(HW_UARTDBGFR_RD() & BM_UARTDBGFR_TXFF))
        HW_UARTDBGDR_WR(ch);
}

void uartdbg_printhex(int data) {
    int i = 0;
    char c;
    for (i = sizeof(int) * 2 - 1; i >= 0; i--) {
        c = data >> (i * 4);
        c &= 0xf;
        if (c > 9)
            uartdbg_putc(c - 10 + 'A');
        else
            uartdbg_putc(c + '0');
    }
}

void uartdbg_printhex8(int data) {
    int i = 0;
    char c;
    for (i = sizeof(char) * 2 - 1; i >= 0; i--) {
        c = data >> (i * 4);
        c &= 0xf;
        if (c > 9)
            uartdbg_putc(c - 10 + 'A');
        else
            uartdbg_putc(c + '0');
    }
}

int Pos_Div(int x, int y) {
    int ans = 0;
    int i;
    for (i = 31; i >= 0; i--) {
        if ((x >> i) >= y) {
            ans += (1 << i);
            x -= (y << i);
        }
    }

    return ans;
}

void uartdbg_printint(int data) {

    int i = 0;
    char str[10] = {0};
    int j = 0;
    while (j < 10 && data) {
        str[j] = data % 10;
        data = Pos_Div(data, 10);
        j++;
    }

    for (i = j - 1; i >= 0; i--) {
        uartdbg_putc(str[i] + '0');
    }
}

void uartdbg_printf(char *fmt, ...) {
    va_list args;
    int one;
    va_start(args, fmt);
    while (*fmt) {

        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {

            case 'x':
            case 'X':
                uartdbg_printhex(va_arg(args, int));
                break;
            case 'd':
            case 'D':
                uartdbg_printint(va_arg(args, int));
                break;
            case '%':
                uartdbg_putc('%');
                break;
            default:
                break;
            }

        } else {
            uartdbg_putc(*fmt);
        }
        fmt++;
    }
    va_end(args);
}

void uartdbg_print_regs() {
    unsigned int regs[16];
    unsigned int cp[15];
    unsigned int mode;

    mode = get_mode();
    


    asm volatile("str r0,%0"
                 : "=m"(regs[0]));
    asm volatile("str r1,%0"
                 : "=m"(regs[1]));
    asm volatile("str r2,%0"
                 : "=m"(regs[2]));
    asm volatile("str r3,%0"
                 : "=m"(regs[3]));
    asm volatile("str r4,%0"
                 : "=m"(regs[4]));
    asm volatile("str r5,%0"
                 : "=m"(regs[5]));
    asm volatile("str r6,%0"
                 : "=m"(regs[6]));
    asm volatile("str r7,%0"
                 : "=m"(regs[7]));
    asm volatile("str r8,%0"
                 : "=m"(regs[8]));
    asm volatile("str r9,%0"
                 : "=m"(regs[9]));
    asm volatile("str r10,%0"
                 : "=m"(regs[10]));
    asm volatile("str r11,%0"
                 : "=m"(regs[11]));
    asm volatile("str r12,%0"
                 : "=m"(regs[12]));
    asm volatile("str r13,%0"
                 : "=m"(regs[13]));
    asm volatile("str r14,%0"
                 : "=m"(regs[14]));
    asm volatile("str r15,%0"
                 : "=m"(regs[15]));

	asm volatile ("mrc p15, 0, r0, c0, c0, 0");
	asm volatile ("str r0,%0" :"=m"(cp[0]));
	asm volatile ("mrc p15, 0, r0, c1, c0, 0");
	asm volatile ("str r0,%0" :"=m"(cp[1]));
	asm volatile ("mrc p15, 0, r0, c2, c0, 0");
	asm volatile ("str r0,%0" :"=m"(cp[2]));

    uartdbg_printf("PC: (R15) = %X\n", regs[15]);
    switch (mode) {
    case USER_MODE:
        uartdbg_printf("MODE= USER_MODE\n");
        break;
    case FIQ_MODE:
        uartdbg_printf("MODE= FIQ_MODE\n");
        break;
    case IRQ_MODE:
        uartdbg_printf("MODE= IRQ_MODE\n");
        break;
    case SVC_MODE:
        uartdbg_printf("MODE= SVC_MODE\n");
        break;
    case ABT_MODE:
        uartdbg_printf("MODE= ABT_MODE\n");
        break;
    case UND_MODE:
        uartdbg_printf("MODE= UND_MODE\n");
        break;
    case SYS_MODE:
        uartdbg_printf("MODE= SYS_MODE\n");
        break;
    }
    uartdbg_printf("CP0: = %X, CP1: = %X,  CP2= %X\n", cp[0], cp[1], cp[2]);
    uartdbg_printf("R0  =%X,  R1 =%X, R2  =%X, R3  =%X\n", regs[0], regs[1], regs[2], regs[3]);
    uartdbg_printf("R4  =%X,  R5 =%X, R6  =%X, R7  =%X\n", regs[4], regs[5], regs[6], regs[7]);
    uartdbg_printf("R8  =%X,  R9 =%X, R10 =%X, R11 =%X\n", regs[8], regs[9], regs[10], regs[11]);
    uartdbg_printf("R12 =%X, R13 =%X, R14 =%X, R15 =%X\n\n", regs[12], regs[13], regs[14], regs[15]);
}
