#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

/*
volatile uint32_t isIRQAllowed()
{
    register uint32_t ret;
    asm volatile("mrs %0, cpsr_all" : "=r"(ret));
    ret >>= 7;
    return (~ret) & 1;
}

void debugPutch(char ch)
{
    volatile uint8_t *UARTDBG = (uint8_t *)0x80070000;
    volatile uint32_t *UARTDBG1 = (uint32_t *)0x80070018;
    while(*UARTDBG1 & (1 << 3)){
        ;
    }

    *UARTDBG = ch;
}


void dbg_printhex(int data) {
    int i = 0;
    char c;
    for (i = sizeof(int) * 2 - 1; i >= 0; i--) {
        c = data >> (i * 4);
        c &= 0xf;
        if (c > 9)
            debugPutch(c - 10 + 'A');
        else
            debugPutch(c + '0');
    }
}

void dbg_printhex8(int data) {
    int i = 0;
    char c;
    for (i = sizeof(char) * 2 - 1; i >= 0; i--) {
        c = data >> (i * 4);
        c &= 0xf;
        if (c > 9)
            debugPutch(c - 10 + 'A');
        else
            debugPutch(c + '0');
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

void dbg_printint(int data) {

    int i = 0;
    char str[10] = {0};
    int j = 0;
    while (j < 10 && data) {
        str[j] = data % 10;
        data = Pos_Div(data, 10);
        j++;
    }

    for (i = j - 1; i >= 0; i--) {
        debugPutch(str[i] + '0');
    }
}

void dbg_printf(char *fmt, ...) 
{
    va_list args;
    va_start(args, fmt);
    while (*fmt) {

        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {

            case 'x':
            case 'X':
                dbg_printhex(va_arg(args, int));
                break;
            case 'd':
            case 'D':
                dbg_printint(va_arg(args, int));
                break;
            case '%':
                debugPutch('%');
                break;
            default:
                break;
            }

        } else {
            debugPutch(*fmt);
        }
        fmt++;
    }
    va_end(args);
}

*/
