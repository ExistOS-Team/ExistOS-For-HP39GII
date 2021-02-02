#include <math.h>
#include <stdarg.h>

#include "txt.h"

volatile unsigned int *timer = (unsigned int *) ((0x80000000 + 0x0001C000) + 0x000000C0);
volatile unsigned int *uart = (unsigned int *) ((0x80000000 + 0x00070000) + 0x00000000);


void uartdbg_putc(char ch) {
	*uart = ch;
	delay_us(500);
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


void delay_us(unsigned int us){
	unsigned int cur = *timer;
	while(*timer - cur < us);
	return;
}

void far1() __attribute__((section(".far1")));
void far1()
{
	for(int i='1';i<='4';i++){
		delay_us(200);
		*uart = i;
	}
}

void far2() __attribute__((section(".far2")));
void far2()
{
	for(int i='5';i<='9';i++){
		delay_us(200);
		*uart = i;
	}
}

void far3() __attribute__((section(".far3")));
void far3()
{
	for(int i='1';i<='4';i++){
		delay_us(200);
		*uart = i;
	}
}

void far4() __attribute__((section(".far4")));
void far4()
{
	for(int i='5';i<='9';i++){
		delay_us(200);
		*uart = i;
	}
}

void far5() __attribute__((section(".far5")));
void far5()
{
	for(int i='1';i<='4';i++){
		delay_us(200);
		*uart = i;
	}
}

void far6() __attribute__((section(".far6")));
void far6()
{
	for(int i='5';i<='9';i++){
		delay_us(200);
		*uart = i;
	}
}

void far7() __attribute__((section(".far7")));
void far7()
{
	for(int i='5';i<='9';i++){
		delay_us(200);
		*uart = i;
	}
}

void far8() __attribute__((section(".far8")));
void far8()
{
	for(int i='a';i<='z';i++){
		delay_us(200);
		*uart = i;
	}
}

volatile int factorial(int  n){
        unsigned int stacks;
		if (n==1){
            return 1;
        }else {
				far1();
	far2();
	far3();
	far4();
	far5();
	far6();
	far7();
	far8();
			(*((void (*)())(  (*((unsigned int *)0xC0072200))  )))();
			__asm volatile ("str sp,%0":"=o"(stacks));
			uartdbg_printf("%d\n",n);
			uartdbg_printf("sp:%x\n",stacks);
			
            return  factorial(n-1)+n;//1*2*3...
    }
}

int main() {
	
	//uartdbg_printf(longtext);
	int s = (int)longtext + sizeof(longtext);
	for(char *i = longtext; i < s; i++ ){
		uartdbg_putc(*i);
	}
	
	for(int i='a';i<='z';i++){
		delay_us(100);
		*uart = i;
	}
	far1();
	far2();
	far3();
	far4();
	far5();
	far6();
	far7();
	far8();
	int a = factorial(1000);
	uartdbg_printf("\nres:%d\n",a);
    return 0;
}


