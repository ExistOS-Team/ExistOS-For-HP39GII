#ifndef CPU_H
#define CPU_H
#include "rom.h"
void cpu_init(void);
int cpu_cycle(void);
unsigned int cpu_get_cycles(void);
void cpu_interrupt_begin(void);
void cpu_interrupt(unsigned short);
void cpu_unhalt(void);
int cpu_halted(void);
void cpu_print_debug();
#endif
