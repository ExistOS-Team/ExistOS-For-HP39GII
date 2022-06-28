#include <stdio.h>
#include "mem.h"
#include "rom.h"
#include "interrupt.h"
#include "timer.h"

#define set_HL(x) do {unsigned int macro = (x); c.L = macro&0xFF; c.H = macro>>8;} while(0)
#define set_BC(x) do {unsigned int macro = (x); c.C = macro&0xFF; c.B = macro>>8;} while(0)
#define set_DE(x) do {unsigned int macro = (x); c.E = macro&0xFF; c.D = macro>>8;} while(0)
#define set_AF(x) do {unsigned int macro = (x); c.F = macro&0xFF; c.A = macro>>8;} while(0)

#define get_AF() ((c.A<<8) | c.F)
#define get_BC() ((c.B<<8) | c.C)
#define get_DE() ((c.D<<8) | c.E)
#define get_HL() ((c.H<<8) | c.L)

/* Flags */
#define set_Z(x) c.F = ((c.F&0x7F) | ((x)<<7))
#define set_N(x) c.F = ((c.F&0xBF) | ((x)<<6))
#define set_H(x) c.F = ((c.F&0xDF) | ((x)<<5))
#define set_C(x) c.F = ((c.F&0xEF) | ((x)<<4))

#define flag_Z !!((c.F & 0x80))
#define flag_N !!((c.F & 0x40))
#define flag_H !!((c.F & 0x20))
#define flag_C !!((c.F & 0x10))

/* Opcodes */
#define INC(x) \
	x++; \
	set_Z(!x); \
	set_H((x & 0xF) == 0); \
	set_N(0); \
	c.cycles += 1;

#define DEC(x) \
	x--; \
	set_Z(!x); \
	set_N(1); \
	set_H((x & 0xF) == 0xF); \
	c.cycles += 1;

#define LDRR(x, y) \
	x = y; \
	c.cycles += 1;

#define LDRIMM8(x) \
	x = mem_get_byte(c.PC); \
	c.PC += 1; \
	c.cycles += 2;

#define ANDR(x) \
	c.A &= x; \
	set_Z(!c.A); \
	set_H(1); \
	set_N(0); \
	set_C(0); \
	c.cycles += 1;

#define XORR(x) \
	c.A ^= x; \
	set_Z(!c.A); \
	set_H(0); \
	set_N(0); \
	set_C(0); \
	c.cycles += 1;

#define ORR(x) \
	c.A |= x; \
	set_Z(!c.A); \
	set_H(0); \
	set_N(0); \
	set_C(0); \
	c.cycles += 1;

#define CPR(x) \
	set_C((c.A - x) < 0); \
	set_H(((c.A - x)&0xF) > (c.A&0xF)); \
	set_Z(c.A == x); \
	set_N(1); \
	c.cycles += 1;

#define SUBR(x) \
	set_C((c.A - x) < 0); \
	set_H(((c.A - x)&0xF) > (c.A&0xF)); \
	c.A -= x; \
	set_Z(!c.A); \
	set_N(1); \
	c.cycles += 1;

struct CPU {
	unsigned char H;
	unsigned char L;

	unsigned char D;
	unsigned char E;

	unsigned char B;
	unsigned char C;

	unsigned char A;
	unsigned char F;

	unsigned short SP;
	unsigned short PC;
	unsigned int cycles;
};

static struct CPU c;
static int is_debugged;
static int halted;
static int ei_flag;

void cpu_init(void)
{
	set_AF(0x01B0);
	set_BC(0x0013);
	set_DE(0x00D8);
	set_HL(0x014D);
	c.SP = 0xFFFE;
#ifdef DEBUG
	c.PC = 0x0100;
#else
	c.PC = 0;
#endif
	c.cycles = 0;
	is_debugged = 0;
}

static unsigned short get_word_ticked(unsigned short i)
{
	unsigned short word;

	word = mem_get_byte(i);
	c.cycles += 1;
	timer_cycle();
	word |= (mem_get_byte(i+1)<<8);
	c.cycles += 1;
	timer_cycle();

	return word;
}

static void write_word_ticked(unsigned short d, unsigned short i)
{
	mem_write_byte(d+1, i >> 8);
	c.cycles += 1;
	timer_cycle();
	mem_write_byte(d, i & 0xFF);
	c.cycles += 1;
	timer_cycle();
}

static void RLC(unsigned char reg)
{
	unsigned char t, old;

	switch(reg)
	{
		case 0:	/* B */
			old = !!(c.B&0x80);
			c.B = (c.B << 1) | old;
			set_C(old);
			set_Z(!c.B);
		break;
		case 1:	/* C */
			old = !!(c.C&0x80);
			set_C(old);
			c.C = c.C<<1 | old;
			set_Z(!c.C);
		break;
		case 2:	/* D */
			old = !!(c.D&0x80);
			set_C(old);
			c.D = c.D<<1 | old;
			set_Z(!c.D);
		break;
		case 3:	/* E */
			old = !!(c.E&0x80);
			set_C(old);
			c.E = c.E<<1 | old;
			set_Z(!c.E);
		break;
		case 4:	/* H */
			old = !!(c.H&0x80);
			set_C(old);
			c.H = c.H<<1 | old;
			set_Z(!c.H);
		break;
		case 5:	/* L */
			old = !!(c.L&0x80);
			set_C(old);
			c.L = c.L<<1 | old;
			set_Z(!c.L);
		break;
		case 6:	/* (HL) */
			c.cycles += 1;
			timer_cycle();
			t = mem_get_byte(get_HL());
			old = !!(t&0x80);
			set_C(old);
			t = t<<1 | old;
			c.cycles += 1;
			timer_cycle();
			mem_write_byte(get_HL(), t);
			set_Z(!t);
		break;
		case 7:	/* A */
			old = !!(c.A&0x80);
			c.A = (c.A<<1) | old;
			set_C(old);
			set_Z(!c.A);
		break;
	}

	set_N(0);
	set_H(0);
}

static void RRC(unsigned char reg)
{
	unsigned char t, old;

	switch(reg)
	{
		case 0:	/* B */
			old = c.B&1;
			set_C(old);
			c.B = c.B>>1 | old<<7;
			set_Z(!c.B);
		break;
		case 1:	/* C */
			old = c.C&1;
			set_C(old);
			c.C = c.C>>1 | old<<7;
			set_Z(!c.C);
		break;
		case 2:	/* D */
			old = c.D&1;
			set_C(old);
			c.D = c.D>>1 | old<<7;
			set_Z(!c.D);
		break;
		case 3:	/* E */
			old = c.E&1;
			set_C(old);
			c.E = c.E>>1 | old<<7;
			set_Z(!c.E);
		break;
		case 4:	/* H */
			old = c.H&1;
			set_C(old);
			c.H = c.H>>1 | old<<7;
			set_Z(!c.H);
		break;
		case 5:	/* L */
			old = c.L&1;
			set_C(old);
			c.L = c.L>>1 | old<<7;
			set_Z(!c.L);
		break;
		case 6:	/* (HL) */
			c.cycles += 1;
			timer_cycle();
			t = mem_get_byte(get_HL());
			old = t;
			set_C(old);
			t = t>>1 | old<<7;
			c.cycles += 1;
			timer_cycle();
			mem_write_byte(get_HL(), t);
			set_Z(!t);
		break;
		case 7:	/* A */
			old = c.A&1;
			set_C(old);
			c.A = c.A>>1 | old<<7;
			set_Z(!c.A);
		break;
	}

	set_N(0);
	set_H(0);
}

static void RL(unsigned char reg)
{
	unsigned char t, t2;

	switch(reg)
	{
		case 0: /* B */
			t2 = flag_C;
			set_C(!!(c.B&0x80));
			c.B = (c.B << 1) | !!(t2);
			set_Z(!c.B);
		break;
		case 1: /* C */
			t2 = flag_C;
			set_C(!!(c.C&0x80));
			c.C = (c.C << 1) | !!(t2);
			set_Z(!c.C);
		break;
		case 2: /* D */
			t2 = flag_C;
			set_C(!!(c.D&0x80));
			c.D = (c.D << 1) | !!(t2);
			set_Z(!c.D);
		break;
		case 3: /* E */
			t2 = flag_C;
			set_C(!!(c.E&0x80));
			c.E = (c.E << 1) | !!(t2);
			set_Z(!c.E);
		break;
		case 4: /* H */
			t2 = flag_C;
			set_C(!!(c.H&0x80));
			c.H = (c.H << 1) | !!(t2);
			set_Z(!c.H);
		break;
		case 5: /* L */
			t2 = flag_C;
			set_C(!!(c.L&0x80));
			c.L = (c.L << 1) | !!(t2);
			set_Z(!c.L);
		break;
		case 6: /* (HL) */
			c.cycles += 1;
			timer_cycle();
			t = mem_get_byte(get_HL());
			t2 = flag_C;
			set_C(!!(t&0x80));
			t = (t << 1) | !!(t2);
			c.cycles += 1;
			timer_cycle();
			mem_write_byte(get_HL(), t);
			set_Z(!t);
		break;
		case 7: /* A */
			t2 = flag_C;
			set_C(!!(c.A&0x80));
			c.A = (c.A << 1) | !!(t2);
			set_Z(!c.A);
		break;
	}

	set_N(0);
	set_H(0);
}

static void RR(unsigned char reg)
{
	unsigned char t, t2;

	switch(reg)
	{
		case 0:	/* B */
			t2 = flag_C;
			set_C(c.B&1);
			c.B = (c.B >> 1) | t2<<7;
			set_Z(!c.B);
		break;
		case 1: /* C */
			t2 = flag_C;
			set_C(c.C&1);
			c.C = (c.C >> 1) | t2<<7;
			set_Z(!c.C);
		break;
		case 2: /* D */
			t2 = flag_C;
			set_C(c.D&1);
			c.D = (c.D >> 1) | t2<<7;
			set_Z(!c.D);
		break;
		case 3: /* E */
			t2 = flag_C;
			set_C(c.E&1);
			c.E = (c.E >> 1) | t2<<7;
			set_Z(!c.E);
		break;
		case 4: /* H */
			t2 = flag_C;
			set_C(c.H&1);
			c.H = (c.H >> 1) | t2<<7;
			set_Z(!c.H);
		break;
		case 5: /* L */
			t2 = flag_C;
			set_C(c.L&1);
			c.L = (c.L >> 1) | t2<<7;
			set_Z(!c.L);
		break;
		case 6: /* (HL) */
			c.cycles += 1;
			timer_cycle();
			t = mem_get_byte(get_HL());
			t2 = flag_C;
			set_C(t&1);
			t = (t >> 1) | t2<<7;
			set_Z(!t);
			c.cycles += 1;
			timer_cycle();
			mem_write_byte(get_HL(), t);
		break;
		case 7: /* A */
			t2 = flag_C;
			set_C(c.A&1);
			c.A = (c.A >> 1) | (t2<<7);
			set_Z(!c.A);
		break;
	}
	set_N(0);
	set_H(0);
}

static void SLA(unsigned char reg)
{
	unsigned char t;

	switch(reg)
	{
		case 0: /* B */
			set_C(!!(c.B & 0x80));
			c.B = c.B << 1;
			set_Z(!c.B);
		break;
		case 1: /* C */
			set_C(!!(c.C & 0x80));
			c.C = c.C << 1;
			set_Z(!c.C);
		break;
		case 2: /* D */
			set_C(!!(c.D & 0x80));
			c.D = c.D << 1;
			set_Z(!c.D);
		break;
		case 3: /* E */
			set_C(!!(c.E & 0x80));
			c.E = c.E << 1;
			set_Z(!c.E);
		break;
		case 4: /* H */
			set_C(!!(c.H & 0x80));
			c.H = c.H << 1;
			set_Z(!c.H);
		break;
		case 5: /* L */
			set_C(!!(c.L & 0x80));
			c.L = c.L << 1;
			set_Z(!c.L);
		break;
		case 6: /* (HL) */
			c.cycles += 1;
			timer_cycle();
			t = mem_get_byte(get_HL());
			set_C(!!(t & 0x80));
			t = t << 1;
			c.cycles += 1;
			timer_cycle();
			mem_write_byte(get_HL(), t);
			set_Z(!t);
		break;
		case 7: /* A */
			set_C(!!(c.A & 0x80));
			c.A = c.A << 1;
			set_Z(!c.A);
		break;
	}

	set_H(0);
	set_N(0);
}

static void SRA(unsigned char reg)
{
	unsigned char old, t;

	switch(reg)
	{
		case 0: /* B */
			set_C(c.B&1);
			old = c.B&0x80;
			c.B = c.B >> 1 | old;
			set_Z(!c.B);
		break;
		case 1: /* C */
			set_C(c.C&1);
			old = c.C&0x80;
			c.C = c.C >> 1 | old;
			set_Z(!c.C);
		break;
		case 2: /* D */
			set_C(c.D&1);
			old = c.D&0x80;
			c.D = c.D >> 1 | old;
			set_Z(!c.D);
		break;
		case 3: /* E */
			set_C(c.E&1);
			old = c.E&0x80;
			c.E = c.E >> 1 | old;
			set_Z(!c.E);
		break;
		case 4: /* H */
			set_C(c.H&1);
			old = c.H&0x80;
			c.H = c.H >> 1 | old;
			set_Z(!c.H);
		break;
		case 5: /* L */
			set_C(c.L&1);
			old = c.L&0x80;
			c.L = c.L >> 1 | old;
			set_Z(!c.L);
		break;
		case 6: /* (HL) */
			c.cycles += 1;
			timer_cycle();
			t = mem_get_byte(get_HL());
			set_C(t&1);
			old = t&0x80;
			t = t >> 1 | old;
			c.cycles += 1;
			timer_cycle();
			mem_write_byte(get_HL(), t);
			set_Z(!t);
		break;
		case 7: /* A */
			set_C(c.A&1);
			old = c.A&0x80;
			c.A = c.A >> 1 | old;
			set_Z(!c.A);
		break;
	}

	set_H(0);
	set_N(0);
}

static void SRL(unsigned char reg)
{
	unsigned char t;

	switch(reg)
	{
		case 0: /* B */
			set_C(c.B & 1);
			c.B = c.B >> 1;
			set_Z(!c.B);
		break;
		case 1: /* C */
			set_C(c.C & 1);
			c.C = c.C >> 1;
			set_Z(!c.C);
		break;
		case 2: /* D */
			set_C(c.D & 1);
			c.D = c.D >> 1;
			set_Z(!c.D);
		break;
		case 3: /* E */
			set_C(c.E & 1);
			c.E = c.E >> 1;
			set_Z(!c.E);
		break;
		case 4: /* H */
			set_C(c.H & 1);
			c.H = c.H >> 1;
			set_Z(!c.H);
		break;
		case 5: /* L */
			set_C(c.L & 1);
			c.L = c.L >> 1;
			set_Z(!c.L);
		break;
		case 6: /* (HL) */
			c.cycles += 1;
			timer_cycle();
			t = mem_get_byte(get_HL());
			set_C(t & 1);
			t = t >> 1;
			c.cycles += 1;
			timer_cycle();
			mem_write_byte(get_HL(), t);
			set_Z(!t);
		break;
		case 7: /* A */
			set_C(c.A & 1);
			c.A = c.A >> 1;
			set_Z(!c.A);
		break;
	}

	set_H(0);
	set_N(0);
}

static void SWAP(unsigned char reg)
{
	unsigned char t;

	switch(reg)
	{
		case 0: /* B */
			c.B = ((c.B&0xF)<<4) | ((c.B&0xF0)>>4);
			c.F = (!c.B)<<7;
		break;
		case 1: /* C */
			c.C = ((c.C&0xF)<<4) | ((c.C&0xF0)>>4);
			c.F = (!c.C)<<7;
		break;
		case 2: /* D */
			c.D = ((c.D&0xF)<<4) | ((c.D&0xF0)>>4);
			c.F = (!c.D)<<7;
		break;
		case 3: /* E */
			c.E = ((c.E&0xF)<<4) | ((c.E&0xF0)>>4);
			c.F = (!c.E)<<7;
		break;
		case 4: /* H */
			c.H = ((c.H&0xF)<<4) | ((c.H&0xF0)>>4);
			c.F = (!c.H)<<7;
		break;
		case 5: /* L */
			c.L = ((c.L&0xF)<<4) | ((c.L&0xF0)>>4);
			c.F = (!c.L)<<7;
		break;
		case 6: /* (HL) */
			c.cycles += 1;
			timer_cycle();
			t = mem_get_byte(get_HL());
			t = ((t&0xF)<<4) | ((t&0xF0)>>4);
			c.cycles += 1;
			timer_cycle();
			mem_write_byte(get_HL(), t);
			c.F = (!t)<<7;
		break;
		case 7: /* A */
			c.A = ((c.A&0xF)<<4) | ((c.A&0xF0)>>4);
			c.F = (!c.A)<<7;
		break;
	}
}

static void BIT(unsigned char bit, unsigned char reg)
{
	unsigned char t, f = 0 /* Make GCC happy */;

	switch(reg)
	{
		case 0: /* B */
		    f = !(c.B & bit);
		break;
		case 1: /* C */
		    f = !(c.C & bit);
		break;
		case 2: /* D */
		    f = !(c.D & bit);
		break;
		case 3: /* E */
		    f = !(c.E & bit);
		break;
		case 4: /* H */
		    f = !(c.H & bit);
		break;
		case 5: /* L */
		    f = !(c.L & bit);
		break;
		case 6: /* (HL) */
			c.cycles += 1;
			timer_cycle();
			t = mem_get_byte(get_HL());
			f = !(t & bit);
		break;
		case 7: /* A */
		    f = !(c.A & bit);
		break;
	}

	set_Z(f);
	set_N(0);
	set_H(1);
}

static void RES(unsigned char bit, unsigned char reg)
{
	unsigned char t;

	switch(reg)
	{
		case 0: /* B */
			c.B &= ~bit;
		break;
		case 1: /* C */
			c.C &= ~bit;
		break;
		case 2: /* D */
			c.D &= ~bit;
		break;
		case 3: /* E */
			c.E &= ~bit;
		break;
		case 4: /* H */
			c.H &= ~bit;
		break;
		case 5: /* L */
			c.L &= ~bit;
		break;
		case 6: /* (HL) */
			c.cycles += 1;
			timer_cycle();
			t = mem_get_byte(get_HL());
			t &= ~bit;
			c.cycles += 1;
			timer_cycle();
			mem_write_byte(get_HL(), t);
		break;
		case 7: /* A */
			c.A &= ~bit;
		break;
	}
}

static void SET(unsigned char bit, unsigned char reg)
{
	unsigned char t;

	switch(reg)
	{
		case 0: /* B */
			c.B |= bit;
		break;
		case 1: /* C */
			c.C |= bit;
		break;
		case 2: /* D */
			c.D |= bit;
		break;
		case 3: /* E */
			c.E |= bit;
		break;
		case 4: /* H */
			c.H |= bit;
		break;
		case 5: /* L */
			c.L |= bit;
		break;
		case 6: /* (HL) */
			c.cycles += 1;
			timer_cycle();
			t = mem_get_byte(get_HL());
			t |= bit;
			c.cycles += 1;
			timer_cycle();
			mem_write_byte(get_HL(), t);
		break;
		case 7: /* A */
			c.A |= bit;
		break;
	}
}

/*
00000xxx = RLC xxx
00001xxx = RRC xxx
00010xxx = RL xxx
00011xxx = RR xxx
00100xxx = SLA xxx
00101xxx = SRA xxx
00110xxx = SWAP xxx
00111xxx = SRL xxx
01yyyxxx = BIT yyy, xxx
10yyyxxx = RES yyy, xxx
11yyyxxx = SET yyy, xxx
*/
static void decode_CB(unsigned char t)
{
	unsigned char reg, opcode, bit;
	void (*f[])(unsigned char) = {RLC, RRC, RL, RR, SLA, SRA, SWAP, SRL};
	void (*f2[])(unsigned char, unsigned char) = {BIT, RES, SET};

	c.cycles += 1;

	reg = t&7;
	opcode = t>>3;
	if(opcode < 8)
	{
		f[opcode](reg);
		c.cycles += 1;
		return;
	}

	bit = opcode&7;
	opcode >>= 3;
	f2[opcode-1](1<<bit, reg);
	c.cycles += 1;
}

int cpu_halted(void)
{
	return halted;
}

unsigned int cpu_get_cycles(void)
{
	return c.cycles;
}

static int halt_bug = 0;

void cpu_unhalt(void)
{
	halted = 0;
}

void cpu_halt(void)
{
	halted = 1;
}

unsigned int cpu_getpc(void)
{
	return c.PC;
}

void cpu_interrupt_begin(void)
{
	halted = 0;

	c.SP -= 2;
	mem_write_word(c.SP, c.PC);
	c.cycles += 2;
}

void cpu_interrupt(unsigned short n)
{
	c.PC = n;
	c.cycles += 2;
	interrupt_disable();
	c.cycles += 1;
}

void cpu_print_debug(void)
{
	printf("PC: %04X AF: %02X%02X, BC: %02X%02X, DE: %02X%02X, HL: %02X%02X, SP: %04X, cycles: %d\n",
		c.PC, c.A, c.F, c.B, c.C, c.D, c.E, c.H, c.L, c.SP, c.cycles);
}

int cpu_cycle(void)
{
	unsigned char b, t;
	unsigned short s;
	unsigned int i;

	/* If any interrupts are pending, do them now */
	if(!ei_flag)
		interrupt_flush();
	else
	{
		ei_flag = 0;
		interrupt_enable();
	}

	/* If the cpu is halted, do nothing instead */
	if(halted)
	{
		c.cycles += 1;
		return 1;
	}

#ifdef DEBUG
	if(c.PC == 0x100)
		is_debugged = 1;
	if(c.PC == 0xC0BD && is_debugged)
	{is_debugged = 0; exit(0);}
#endif

	if(is_debugged)
		cpu_print_debug();

	/* Otherwise, execute as normal */
	b = mem_get_byte(c.PC);

	if(halt_bug)
		halt_bug = 0;
	else
		c.PC++;

	switch(b)
	{
		case 0x00:	/* NOP */
			c.cycles += 1;
		break;
		case 0x01:	/* LD BC, imm16 */
			s = mem_get_word(c.PC);
			set_BC(s);
			c.PC += 2;
			c.cycles += 3;
		break;
		case 0x02:	/* LD (BC), A */
			c.cycles += 1;
			timer_cycle();
			mem_write_byte(get_BC(), c.A);
			c.cycles += 1;
		break;
		case 0x03:	/* INC BC */
			set_BC(get_BC()+1);
			c.cycles += 2;
		break;
		case 0x04:	/* INC B */
			INC(c.B);
		break;
		case 0x05:	/* DEC B */
			DEC(c.B);
		break;
		case 0x06:	/* LD B, imm8 */
			LDRIMM8(c.B);
		break;
		case 0x07:	/* RLCA */
			RLC(7);
			set_Z(0);
			c.cycles += 1;
		break;
		case 0x08:	/* LD (imm16), SP */
			c.cycles += 3;
			timer_cycle();
			mem_write_word(mem_get_word(c.PC), c.SP);
			c.PC += 2;
			c.cycles += 2;
		break;
		case 0x09:	/* ADD HL, BC */
			i = get_HL() + get_BC();
			set_N(0);
			set_C(i >= 0x10000);
			set_H((i&0xFFF) < (get_HL()&0xFFF));
			set_HL(i&0xFFFF);
			c.cycles += 2;
		break;
		case 0x0A:	/* LD A, (BC) */
			c.cycles += 1;
			timer_cycle();
			c.A = mem_get_byte(get_BC());
			c.cycles += 1;
		break;
		case 0x0B:	/* DEC BC */
			s = get_BC();
			s--;
			set_BC(s);
			c.cycles += 2;
		break;
		case 0x0C:	/* INC C */
			INC(c.C);
		break;
		case 0x0D:	/* DEC C */
			DEC(c.C);
		break;
		case 0x0E:	/* LD C, imm8 */
			LDRIMM8(c.C);
		break;
		case 0x0F:	/* RRCA */
			RRC(7);
			set_Z(0);
			c.cycles += 1;
		break;
		case 0x11:	/* LD DE, imm16 */
			s = mem_get_word(c.PC);
			set_DE(s);
			c.PC += 2;
			c.cycles += 3;
		break;
		case 0x12:	/* LD (DE), A */
			c.cycles += 1;
			timer_cycle();
			mem_write_byte(get_DE(), c.A);
			c.cycles += 1;
		break;
		case 0x13:	/* INC DE */
			s = get_DE();
			s++;
			set_DE(s);
			c.cycles += 2;
		break;
		case 0x14:	/* INC D */
			INC(c.D);
		break;
		case 0x15:	/* DEC D */
			DEC(c.D);
		break;
		case 0x16:	/* LD D, imm8 */
			LDRIMM8(c.D);
		break;
		case 0x17:	/* RLA */
			RL(7);
			set_Z(0);
			c.cycles += 1;
		break;
		case 0x18:	/* JR rel8 */
			c.PC += (signed char)mem_get_byte(c.PC) + 1;
			c.cycles += 3;
		break;
		case 0x19:	/* ADD HL, DE */
			i = get_HL() + get_DE();
			set_H((i&0xFFF) < (get_HL()&0xFFF));
			set_HL(i);
			set_N(0);
			set_C(i > 0xFFFF);
			c.cycles += 2;
		break;
		case 0x1A:	/* LD A, (DE) */
			c.cycles += 1;
			timer_cycle();
			c.A = mem_get_byte(get_DE());
			c.cycles += 1;
		break;
		case 0x1B:	/* DEC DE */
			s = get_DE();
			s--;
			set_DE(s);
			c.cycles += 2;
		break;
		case 0x1C:	/* INC E */
			INC(c.E);
		break;
		case 0x1D:	/* DEC E */
			DEC(c.E);
		break;
		case 0x1E:	/* LD E, imm8 */
			LDRIMM8(c.E);
		break;
		case 0x1F:	/* RR A */
			RR(7);
			set_Z(0);
			c.cycles += 1;
		break;
		case 0x20:	/* JR NZ, rel8 */
			if(flag_Z == 0)
			{
				c.PC += (signed char)mem_get_byte(c.PC) + 1;
				c.cycles += 3;
			} else {
				c.PC += 1;
				c.cycles += 2;
			}
		break;
		case 0x21:	/* LD HL, imm16 */
			s = mem_get_word(c.PC);
			set_HL(s);
			c.PC += 2;
			c.cycles += 3;
		break;
		case 0x22:	/* LDI (HL), A */
			c.cycles += 1;
			timer_cycle();
			i = get_HL();
			mem_write_byte(i, c.A);
			i++;
			set_HL(i);
			c.cycles += 1;
		break;
		case 0x23:	/* INC HL */
			s = get_HL();
			s++;
			set_HL(s);
			c.cycles += 2;
		break;
		case 0x24:	/* INC H */
			INC(c.H);
		break;
		case 0x25:	/* DEC H */
			DEC(c.H);
		break;
		case 0x26:	/* LD H, imm8 */
			LDRIMM8(c.H);
		break;
		case 0x27:	/* DAA */
			s = c.A;

			if(flag_N)
			{
				if(flag_H)
					s = (s - 0x06)&0xFF;
				if(flag_C)
					s -= 0x60;
			}
			else
			{
				if(flag_H || (s & 0xF) > 9)
					s += 0x06;
				if(flag_C || s > 0x9F)
					s += 0x60;
			}

			c.A = s;
			set_H(0);
			set_Z(!c.A);
			if(s >= 0x100)
				set_C(1);
			c.cycles += 1;
		break;
		case 0x28:	/* JR Z, rel8 */
			if(flag_Z == 1)
			{
				c.PC += (signed char)mem_get_byte(c.PC) + 1;
				c.cycles += 3;
			} else {
				c.PC += 1;
				c.cycles += 2;
			}
		break;
		case 0x29:	/* ADD HL, HL */
			i = get_HL()*2;
			set_H((i&0xFFF) < (get_HL()&0xFFF));
			set_C(i > 0xFFFF);
			set_HL(i);
			set_N(0);
			c.cycles += 2;
		break;
		case 0x2A:	/* LDI A, (HL) */
			c.cycles += 1;
			timer_cycle();
			s = get_HL();
			c.A = mem_get_byte(s);
			set_HL(s+1);
			c.cycles += 1;
		break;
		case 0x2B: 	/* DEC HL */
			set_HL(get_HL()-1);
			c.cycles += 2;
		break;
		case 0x2C:	/* INC L */
			INC(c.L);
		break;
		case 0x2D:	/* DEC L */
			DEC(c.L);
		break;
		case 0x2E:	/* LD L, imm8 */
			LDRIMM8(c.L);
		break;
		case 0x2F:	/* CPL */
			c.A = ~c.A;
			set_N(1);
			set_H(1);
			c.cycles += 1;
		break;
		case 0x30:	/* JR NC, rel8 */
			if(flag_C == 0)
			{
				c.PC += (signed char)mem_get_byte(c.PC) + 1;
				c.cycles += 3;
			} else {
				c.PC += 1;
				c.cycles += 2;
			}
		break;
		case 0x31:	/* LD SP, imm16 */
			c.SP = mem_get_word(c.PC);
			c.PC += 2;
			c.cycles += 3;
		break;
		case 0x32:	/* LDD (HL), A */
			c.cycles += 1;
			timer_cycle();
			i = get_HL();
			mem_write_byte(i, c.A);
			set_HL(i-1);
			c.cycles += 1;
		break;
		case 0x33:	/* INC SP */
			c.SP++;
			c.cycles += 2;
		break;
		case 0x34:	/* INC (HL) */
			c.cycles += 1;
			timer_cycle();
			t = mem_get_byte(get_HL());
			t++;
			c.cycles += 1;
			timer_cycle();
			mem_write_byte(get_HL(), t);
			set_Z(!t);
			set_N(0);
			set_H((t & 0xF) == 0);
			c.cycles += 1;
		break;
		case 0x35:	/* DEC (HL) */
			c.cycles += 1;
			timer_cycle();
			t = mem_get_byte(get_HL());
			t--;
			c.cycles += 1;
			timer_cycle();
			mem_write_byte(get_HL(), t);
			set_Z(!t);
			set_N(1);
			set_H((t & 0xF) == 0xF);
			c.cycles += 1;
		break;
		case 0x36:	/* LD (HL), imm8 */
			c.cycles += 2;
			timer_cycle();
			t = mem_get_byte(c.PC);
			mem_write_byte(get_HL(), t);
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0x37:	/* SCF */
			set_N(0);
			set_H(0);
			set_C(1);
			c.cycles += 1;
		break;
		case 0x38:  /* JR C, rel8 */
			if(flag_C)
			{
				c.PC += (signed char)mem_get_byte(c.PC) + 1;
				c.cycles += 3;
			} else {
				c.PC += 1;
				c.cycles += 2;
			}
		break;
		case 0x39:	/* ADD HL, SP */
			i = get_HL() + c.SP;
			set_H((i&0x7FF) < (get_HL()&0x7FF));
			set_C(i > 0xFFFF);
			set_N(0);
			set_HL(i);
			c.cycles += 2;
		break;
		case 0x3A:	/* LDD A, (HL) */
			c.cycles += 1;
			timer_cycle();
			c.A = mem_get_byte(get_HL());
			set_HL(get_HL()-1);
			c.cycles += 1;
		break;
		case 0x3B:	/* DEC SP */
			c.SP--;
			c.cycles += 2;
		break;
		case 0x3C:	/* INC A */
			INC(c.A);
		break;
		case 0x3D:	/* DEC A */
			DEC(c.A);
		break;
		case 0x3E:	/* LD A, imm8 */
			LDRIMM8(c.A);
		break;
		case 0x3F:	/* CCF */
			set_N(0);
			set_H(0);
			set_C(!flag_C);
			c.cycles += 1;
		break;
		case 0x40:	/* LD B, B */
			LDRR(c.B, c.B);
		break;
		case 0x41:	/* LD B, C */
			LDRR(c.B, c.C);
		break;
		case 0x42:	/* LD B, D */
			LDRR(c.B, c.D);
		break;
		case 0x43:	/* LD B, E */
			LDRR(c.B, c.E);
		break;
		case 0x44:	/* LD B, H */
			LDRR(c.B, c.H);
		break;
		case 0x45:	/* LD B, L */
			LDRR(c.B, c.L);
		break;
		case 0x46:	/* LD B, (HL) */
			c.cycles += 1;
			timer_cycle();
			c.B = mem_get_byte(get_HL());
			c.cycles += 1;
		break;
		case 0x47:	/* LD B, A */
			LDRR(c.B, c.A);
		break;
		case 0x48:	/* LD C, B */
			LDRR(c.C, c.B);
		break;
		case 0x49:	/* LD C, C */
			LDRR(c.C, c.C);
		break;
		case 0x4A:	/* LD C, D */
			LDRR(c.C, c.D);
		break;
		case 0x4B:	/* LD C, E */
			LDRR(c.C, c.E);
		break;
		case 0x4C:	/* LD C, H */
			LDRR(c.C, c.H);
		break;
		case 0x4D:	/* LD C, L */
			LDRR(c.C, c.L);
		break;
		case 0x4E:	/* LD C, (HL) */
			c.cycles += 1;
			timer_cycle();
			c.C = mem_get_byte(get_HL());
			c.cycles += 1;
		break;
		case 0x4F:	/* LD C, A */
			LDRR(c.C, c.A);
		break;
		case 0x50:	/* LD D, B */
			LDRR(c.D, c.B);
		break;
		case 0x51:	/* LD D, C */
			LDRR(c.D, c.C);
		break;
		case 0x52:	/* LD D, D */
			LDRR(c.D, c.D);
		break;
		case 0x53:	/* LD D, E */
			LDRR(c.D, c.E);
		break;
		case 0x54:	/* LD D, H */
			LDRR(c.D, c.H);
		break;
		case 0x55:	/* LD D, L */
			LDRR(c.D, c.L);
		break;
		case 0x56:	/* LD D, (HL) */
			c.cycles += 1;
			timer_cycle();
			c.D = mem_get_byte(get_HL());
			c.cycles += 1;
		break;
		case 0x57:	/* LD D, A */
			LDRR(c.D, c.A);
		break;
		case 0x58:	/* LD E, B */
			LDRR(c.E, c.B);
		break;
		case 0x59:	/* LD E, C */
			LDRR(c.E, c.C);
		break;
		case 0x5A:	/* LD E, D */
			LDRR(c.E, c.D);
		break;
		case 0x5B:	/* LD E, E */
			LDRR(c.E, c.E);
		break;
		case 0x5C:	/* LD E, H */
			LDRR(c.E, c.H);
		break;
		case 0x5D:	/* LD E, L */
			LDRR(c.E, c.L);
		break;
		case 0x5E:	/* LD E, (HL) */
			c.cycles += 1;
			timer_cycle();
			c.E = mem_get_byte(get_HL());
			c.cycles += 1;
		break;
		case 0x5F:	/* LD E, A */
			LDRR(c.E, c.A);
		break;
		case 0x60:	/* LD H, B */
			LDRR(c.H, c.B);
		break;
		case 0x61:	/* LD H, C */
			LDRR(c.H, c.C);
		break;
		case 0x62:	/* LD H, D */
			LDRR(c.H, c.D);
		break;
		case 0x63:	/* LD H, E */
			LDRR(c.H, c.E);
		break;
		case 0x64:	/* LD H, H */
			LDRR(c.H, c.H);
		break;
		case 0x65:	/* LD H, L */
			LDRR(c.H, c.L);
		break;
		case 0x66:	/* LD H, (HL) */
			c.cycles += 1;
			timer_cycle();
			c.H = mem_get_byte(get_HL());
			c.cycles += 1;
		break;
		case 0x67:	/* LD H, A */
			LDRR(c.H, c.A);
		break;
		case 0x68:	/* LD L, B */
			LDRR(c.L, c.B);
		break;
		case 0x69:	/* LD L, C */
			LDRR(c.L, c.C);
		break;
		case 0x6A:	/* LD L, D */
			LDRR(c.L, c.D);
		break;
		case 0x6B:	/* LD L, E */
			LDRR(c.L, c.E);
		break;
		case 0x6C:	/* LD L, H */
			LDRR(c.L, c.H);
		break;
		case 0x6D:	/* LD L, L */
			LDRR(c.L, c.L);
		break;
		case 0x6E:	/* LD L, (HL) */
			c.cycles += 1;
			timer_cycle();
			c.L = mem_get_byte(get_HL());
			c.cycles += 1;
		break;
		case 0x6F:	/* LD L, A */
			LDRR(c.L, c.A);
		break;
		case 0x70:	/* LD (HL), B */
			c.cycles += 1;
			timer_cycle();
			mem_write_byte(get_HL(), c.B);
			c.cycles += 1;
		break;
		case 0x71:	/* LD (HL), C */
			c.cycles += 1;
			timer_cycle();
			mem_write_byte(get_HL(), c.C);
			c.cycles += 1;
		break;
		case 0x72:	/* LD (HL), D */
			c.cycles += 1;
			timer_cycle();
			mem_write_byte(get_HL(), c.D);
			c.cycles += 1;
		break;
		case 0x73:	/* LD (HL), E */
			c.cycles += 1;
			timer_cycle();
			mem_write_byte(get_HL(), c.E);
			c.cycles += 1;
		break;
		case 0x74:	/* LD (HL), H */
			c.cycles += 1;
			timer_cycle();
			mem_write_byte(get_HL(), c.H);
			c.cycles += 1;
		break;
		case 0x75:	/* LD (HL), L */
			c.cycles += 1;
			timer_cycle();
			mem_write_byte(get_HL(), c.L);
			c.cycles += 1;
		break;
		case 0x76:	/* HALT */
			if(interrupt_get_enabled())
			{
				halted = 1;
				c.cycles += 1;
				break;
			}

			if(!interrupt_pending())
				halted = 1;
			else
				halt_bug = 1;

			c.cycles += 1;
		break;
		case 0x77:	/* LD (HL), A */
			c.cycles += 1;
			timer_cycle();
			mem_write_byte(get_HL(), c.A);
			c.cycles += 1;
		break;
		case 0x78:	/* LD A, B */
			LDRR(c.A, c.B);
		break;
		case 0x79:	/* LD A, C */
			LDRR(c.A, c.C);
		break;
		case 0x7A:	/* LD A, D */
			LDRR(c.A, c.D);
		break;
		case 0x7B:	/* LD A, E */
			LDRR(c.A, c.E);
		break;
		case 0x7C:	/* LD A, H */
			LDRR(c.A, c.H);
		break;
		case 0x7D:	/* LD A, L */
			LDRR(c.A, c.L);
		break;
		case 0x7E:	/* LD A, (HL) */
			c.cycles++;
			timer_cycle();
			c.A = mem_get_byte(get_HL());
			c.cycles++;
		break;
		case 0x7F:	/* LD A, A */
			LDRR(c.A, c.A);
		break;
		case 0x80:	/* ADD B */
			i = c.A + c.B;
			set_H((c.A&0xF)+(c.B&0xF) > 0xF);
			set_C(i > 0xFF);
			set_N(0);
			c.A = i;
			set_Z(!c.A);
			c.cycles += 1;
		break;
		case 0x81:	/* ADD C */
			i = c.A + c.C;
			set_H((c.A&0xF)+(c.C&0xF) > 0xF);
			set_C(i > 0xFF);
			set_N(0);
			c.A = i;
			set_Z(!c.A);
			c.cycles += 1;
		break;
		case 0x82:	/* ADD D */
			i = c.A + c.D;
			set_H((c.A&0xF)+(c.D&0xF) > 0xF);
			set_C(i > 0xFF);
			set_N(0);
			c.A = i;
			set_Z(!c.A);
			c.cycles += 1;
		break;
		case 0x83:	/* ADD E */
			i = c.A + c.E;
			set_H((c.A&0xF)+(c.E&0xF) > 0xF);
			set_C(i > 0xFF);
			set_N(0);
			c.A = i;
			set_Z(!c.A);
			c.cycles += 1;
		break;
		case 0x84:	/* ADD H */
			i = c.A + c.H;
			set_H((c.A&0xF)+(c.H&0xF) > 0xF);
			set_C(i > 0xFF);
			set_N(0);
			c.A = i;
			set_Z(!c.A);
			c.cycles += 1;
		break;
		case 0x85:	/* ADD L */
			i = c.A + c.L;
			set_H((c.A&0xF)+(c.L&0xF) > 0xF);
			set_C(i > 0xFF);
			set_N(0);
			c.A = i;
			set_Z(!c.A);
			c.cycles += 1;
		break;
		case 0x86:	/* ADD (HL) */
			c.cycles += 1;
			timer_cycle();
			i = c.A + mem_get_byte(get_HL());
			set_H((i&0xF) < (c.A&0xF));
			set_C(i > 0xFF);
			set_N(0);
			c.A = i;
			set_Z(!c.A);
			c.cycles += 1;
		break;
		case 0x87:	/* ADD A */
			i = c.A + c.A;
			set_H((c.A&0xF)+(c.A&0xF) > 0xF);
			set_C(i > 0xFF);
			set_N(0);
			c.A = i;
			set_Z(!c.A);
			c.cycles += 1;
		break;
		case 0x88:	/* ADC B */
			i = c.A + c.B + flag_C >= 0x100;
			set_N(0);
			set_H(((c.A&0xF) + (c.B&0xF) + flag_C) >= 0x10);
			c.A = c.A + c.B + flag_C;
			set_C(i);
			set_Z(!c.A);
			c.cycles += 1;
		break;
		case 0x89:	/* ADC C */
			i = c.A + c.C + flag_C >= 0x100;
			set_N(0);
			set_H(((c.A&0xF) + (c.C&0xF) + flag_C) >= 0x10);
			c.A = c.A + c.C + flag_C;
			set_C(i);
			set_Z(!c.A);
			c.cycles += 1;
		break;
		case 0x8A:	/* ADC D */
			i = c.A + c.D + flag_C >= 0x100;
			set_N(0);
			set_H(((c.A&0xF) + (c.D&0xF) + flag_C) >= 0x10);
			c.A = c.A + c.D + flag_C;
			set_C(i);
			set_Z(!c.A);
			c.cycles += 1;
		break;
		case 0x8B:	/* ADC E */
			i = c.A + c.E + flag_C >= 0x100;
			set_N(0);
			set_H(((c.A&0xF) + (c.E&0xF) + flag_C) >= 0x10);
			c.A = c.A + c.E + flag_C;
			set_C(i);
			set_Z(!c.A);
			c.cycles += 1;
		break;
		case 0x8C:	/* ADC H */
			i = c.A + c.H + flag_C >= 0x100;
			set_N(0);
			set_H(((c.A&0xF) + (c.H&0xF) + flag_C) >= 0x10);
			c.A = c.A + c.H + flag_C;
			set_C(i);
			set_Z(!c.A);
			c.cycles += 1;
		break;
		case 0x8D:	/* ADC L */
			i = c.A + c.L + flag_C >= 0x100;
			set_N(0);
			set_H(((c.A&0xF) + (c.L&0xF) + flag_C) >= 0x10);
			c.A = c.A + c.L + flag_C;
			set_C(i);
			set_Z(!c.A);
			c.cycles += 1;
		break;
		case 0x8E:	/* ADC (HL) */
			c.cycles += 1;
			timer_cycle();
			t = mem_get_byte(get_HL());
			i = c.A + t + flag_C >= 0x100;
			set_N(0);
			set_H(((c.A&0xF) + (t&0xF) + flag_C) >= 0x10);
			c.A = c.A + t + flag_C;
			set_C(i);
			set_Z(!c.A);
			c.cycles += 1;
		break;
		case 0x8F:	/* ADC A */
			i = c.A + c.A + flag_C >= 0x100;
			set_N(0);
			set_H(((c.A&0xF) + (c.A&0xF) + flag_C) >= 0x10);
			c.A = c.A + c.A + flag_C;
			set_C(i);
			set_Z(!c.A);
			c.cycles += 1;
		break;
		case 0x90:	/* SUB B */
			SUBR(c.B);
		break;
		case 0x91:	/* SUB C */
			SUBR(c.C);
		break;
		case 0x92:	/* SUB D */
			SUBR(c.D);
		break;
		case 0x93:	/* SUB E */
			SUBR(c.E);
		break;
		case 0x94:	/* SUB H */
			SUBR(c.H);
		break;
		case 0x95:	/* SUB L */
			SUBR(c.L);
		break;
		case 0x96:	/* SUB (HL) */
			c.cycles += 1;
			timer_cycle();
			t = mem_get_byte(get_HL());
			set_C((c.A - t) < 0);
			set_H(((c.A - t)&0xF) > (c.A&0xF));
			c.A -= t;
			set_Z(!c.A);
			set_N(1);
			c.cycles += 1;
		break;
		case 0x97:	/* SUB A */
			SUBR(c.A);
		break;
		case 0x98:	/* SBC B */
			t = flag_C + c.B;
			set_H(((c.A&0xF) - (c.B&0xF) - flag_C) < 0);
			set_C((c.A - c.B - flag_C) < 0);
			set_N(1);
			c.A -= t;
			set_Z(!c.A);
			c.cycles += 1;
		break;
		case 0x99:	/* SBC C */
			t = flag_C + c.C;
			set_H(((c.A&0xF) - (c.C&0xF) - flag_C) < 0);
			set_C((c.A - c.C - flag_C) < 0);
			set_N(1);
			c.A -= t;
			set_Z(!c.A);
			c.cycles += 1;
		break;
		case 0x9A:	/* SBC D */
			t = flag_C + c.D;
			set_H(((c.A&0xF) - (c.D&0xF) - flag_C) < 0);
			set_C((c.A - c.D - flag_C) < 0);
			set_N(1);
			c.A -= t;
			set_Z(!c.A);
			c.cycles += 1;
		break;
		case 0x9B:	/* SBC E */
			t = flag_C + c.E;
			set_H(((c.A&0xF) - (c.E&0xF) - flag_C) < 0);
			set_C((c.A - c.E - flag_C) < 0);
			set_N(1);
			c.A -= t;
			set_Z(!c.A);
			c.cycles += 1;
		break;
		case 0x9C:	/* SBC H */
			t = flag_C + c.H;
			set_H(((c.A&0xF) - (c.H&0xF) - flag_C) < 0);
			set_C((c.A - c.H - flag_C) < 0);
			set_N(1);
			c.A -= t;
			set_Z(!c.A);
			c.cycles += 1;
		break;
		case 0x9D:	/* SBC L */
			t = flag_C + c.L;
			set_H(((c.A&0xF) - (c.L&0xF) - flag_C) < 0);
			set_C((c.A - c.L - flag_C) < 0);
			set_N(1);
			c.A -= t;
			set_Z(!c.A);
			c.cycles += 1;
		break;
		case 0x9E:	/* SBC (HL) */
			c.cycles += 1;
			timer_cycle();
			t = mem_get_byte(get_HL());
			b = flag_C + t;
			set_H(((c.A&0xF) - (t&0xF) - flag_C) < 0);
			set_C((c.A - t - flag_C) < 0);
			set_N(1);
			c.A -= b;
			set_Z(!c.A);
			c.cycles += 1;
		break;
		case 0x9F:	/* SBC A */
			t = flag_C + c.A;
			set_H(((c.A&0xF) - (c.A&0xF) - flag_C) < 0);
			set_C((c.A - c.A - flag_C) < 0);
			set_N(1);
			c.A -= t;
			set_Z(!c.A);
			c.cycles += 1;
		break;
		case 0xA0:	/* AND B */
			ANDR(c.B);
		break;
		case 0xA1:	/* AND C */
			ANDR(c.C);
		break;
		case 0xA2:	/* AND D */
			ANDR(c.D);
		break;
		case 0xA3:	/* AND E */
			ANDR(c.E);
		break;
		case 0xA4:	/* AND H */
			ANDR(c.H);
		break;
		case 0xA5:	/* AND L */
			ANDR(c.L);
		break;
		case 0xA6:	/* AND (HL) */
			c.cycles += 1;
			timer_cycle();
			c.A &= mem_get_byte(get_HL());
			set_Z(!c.A);
			set_H(1);
			set_N(0);
			set_C(0);
			c.cycles += 1;
		break;
		case 0xA7:	/* AND A */
			ANDR(c.A);
		break;
		case 0xA8:	/* XOR B */
			XORR(c.B);
		break;
		case 0xA9:	/* XOR C */
			XORR(c.C);
		break;
		case 0xAA:	/* XOR D */
			XORR(c.D);
		break;
		case 0xAB:	/* XOR E */
			XORR(c.E);
		break;
		case 0xAC:	/* XOR H */
			XORR(c.H);
		break;
		case 0xAD:	/* XOR L */
			XORR(c.L);
		break;
		case 0xAE:	/* XOR (HL) */
			c.cycles += 1;
			timer_cycle();
			c.A ^= mem_get_byte(get_HL());
			c.F = (!c.A)<<7;
			c.cycles += 1;
		break;
		case 0xAF:	/* XOR A */
			XORR(c.A);
		break;
		case 0xB0:	/* OR B */
			ORR(c.B);
		break;
		case 0xB1:	/* OR C */
			ORR(c.C);
		break;
		case 0xB2:	/* OR D */
			ORR(c.D);
		break;
		case 0xB3:	/* OR E */
			ORR(c.E);
		break;
		case 0xB4:	/* OR H */
			ORR(c.H);
		break;
		case 0xB5:	/* OR L */
			ORR(c.L);
		break;
		case 0xB6:	/* OR (HL) */
			c.cycles += 1;
			timer_cycle();
			c.A |= mem_get_byte(get_HL());
			c.F = (!c.A)<<7;
			c.cycles += 1;
		break;
		case 0xB7:	/* OR A */
			ORR(c.A);
		break;
		case 0xB8:	/* CP B */
			CPR(c.B);
		break;
		case 0xB9:	/* CP C */
			CPR(c.C);
		break;
		case 0xBA:	/* CP D */
			CPR(c.D);
		break;
		case 0xBB:	/* CP E */
			CPR(c.E);
		break;
		case 0xBC:	/* CP H */
			CPR(c.H);
		break;
		case 0xBD:	/* CP L */
			CPR(c.L);
		break;
		case 0xBE:	/* CP (HL) */
			c.cycles++;
			timer_cycle();
			t = mem_get_byte(get_HL());
			set_Z(c.A == t);
			set_H(((c.A - t)&0xF) > (c.A&0xF));
			set_N(1);
			set_C((c.A - t) < 0);
			c.cycles += 1;
		break;
		case 0xBF:	/* CP A */
			CPR(c.A);
		break;
		case 0xC0:	/* RET NZ */
			if(flag_Z == 0)
			{
				c.cycles += 2;
				timer_cycle();
				c.PC = get_word_ticked(c.SP);
				c.SP += 2;
				c.cycles += 1;
			} else {
				c.cycles += 2;
			}
		break;
		case 0xC1:	/* POP BC */
			c.cycles += 1;
			timer_cycle();
			s = get_word_ticked(c.SP);
			set_BC(s);
			c.SP += 2;
		break;
		case 0xC2:	/* JP NZ, mem16 */
			if(flag_Z == 0)
			{
				c.cycles += 1;
				timer_cycle();
				c.PC = get_word_ticked(c.PC);
				c.cycles += 1;
			} else {
				c.PC += 2;
				c.cycles += 3;
			}
		break;
		case 0xC3:	/* JP imm16 */
			c.cycles += 1;
			timer_cycle();
			c.PC = get_word_ticked(c.PC);
			c.cycles += 1;
		break;
		case 0xC4:	/* CALL NZ, imm16 */
			c.cycles += 1;
			timer_cycle();
			if(flag_Z == 0)
			{
				i = get_word_ticked(c.PC);
				c.cycles += 1;
				timer_cycle();
				c.SP -= 2;
				write_word_ticked(c.SP, c.PC+2);
				c.PC = i;
			} else {
				c.PC += 2;
				c.cycles += 2;
			}
		break;
		case 0xC5:	/* PUSH BC */
			c.SP -= 2;
			c.cycles += 2;
			timer_cycle();
			write_word_ticked(c.SP, get_BC());
		break;
		case 0xC6:	/* ADD A, imm8 */
			t = mem_get_byte(c.PC);
			set_C((c.A + t) >= 0x100);
			set_H(((c.A + t)&0xF) < (c.A&0xF));
			c.A += t;
			set_N(0);
			set_Z(!c.A);
			c.PC += 1;
			c.cycles += 2;
		break;
		case 0xC7:	/* RST 00 */
			c.cycles += 2;
			timer_cycle();
			c.SP -= 2;
			write_word_ticked(c.SP, c.PC);
			c.PC = 0x00;
		break;
		case 0xC8:	/* RET Z */
			if(flag_Z == 1)
			{
				c.cycles += 2;
				timer_cycle();
				c.PC = get_word_ticked(c.SP);
				c.SP += 2;
				c.cycles += 1;
			} else {
				c.cycles += 2;
			}
		break;
		case 0xC9:	/* RET */
			c.cycles += 1;
			timer_cycle();
			c.PC = get_word_ticked(c.SP);
			c.SP += 2;
			c.cycles += 1;
		break;
		case 0xCA:	/* JP Z, mem16 */
			if(flag_Z == 1)
			{
				c.cycles += 1;
				timer_cycle();
				c.PC = get_word_ticked(c.PC);
				c.cycles += 1;
			} else {
				c.PC += 2;
				c.cycles += 3;
			}
		break;
		case 0xCB:	/* RLC/RRC/RL/RR/SLA/SRA/SWAP/SRL/BIT/RES/SET */
			decode_CB(mem_get_byte(c.PC));
			c.PC += 1;
		break;
		case 0xCC:	/* CALL Z, imm16 */
			c.cycles += 1;
			timer_cycle();
			if(flag_Z == 1)
			{
				i = get_word_ticked(c.PC);
				c.cycles += 1;
				timer_cycle();
				c.SP -= 2;
				write_word_ticked(c.SP, c.PC+2);
				c.PC = i;
			} else {
				c.PC += 2;
				c.cycles += 2;
			}
		break;
		case 0xCD:	/* CALL imm16 */
			c.cycles += 1;
			timer_cycle();

			i = get_word_ticked(c.PC);

			c.cycles += 1;
			timer_cycle();

			c.SP -= 2;
			write_word_ticked(c.SP, c.PC+2);
			c.PC = i;
		break;
		case 0xCE:	/* ADC a, imm8 */
			t = mem_get_byte(c.PC);
			i = c.A + t + flag_C >= 0x100;
			set_N(0);
			set_H(((c.A&0xF) + (t&0xF) + flag_C) >= 0x10);
			c.A = c.A + t + flag_C;
			set_C(i);
			set_Z(!c.A);
			c.PC += 1;
			c.cycles += 2;
		break;
		case 0xCF:	/* RST 08 */
			c.cycles += 2;
			timer_cycle();
			c.SP -= 2;
			write_word_ticked(c.SP, c.PC);
			c.PC = 0x08;
		break;
		case 0xD0:	/* RET NC */
			if(flag_C == 0)
			{
				c.cycles += 2;
				timer_cycle();
				c.PC = get_word_ticked(c.SP);
				c.SP += 2;
				c.cycles += 1;
			} else {
				c.cycles += 2;
			}
		break;
		case 0xD1:	/* POP DE */
			c.cycles += 1;
			timer_cycle();
			s = get_word_ticked(c.SP);
			set_DE(s);
			c.SP += 2;
		break;
		case 0xD2:	/* JP NC, mem16 */
			if(flag_C == 0)
			{
				c.cycles += 1;
				timer_cycle();
				c.PC = get_word_ticked(c.PC);
				c.cycles += 1;
			} else {
				c.PC += 2;
				c.cycles += 3;
			}
		break;
		case 0xD4:	/* CALL NC, mem16 */
			c.cycles += 1;
			timer_cycle();
			if(flag_C == 0)
			{
				i = get_word_ticked(c.PC);
				c.cycles += 1;
				timer_cycle();
				c.SP -= 2;
				write_word_ticked(c.SP, c.PC+2);
				c.PC = i;
			} else {
				c.PC += 2;
				c.cycles += 2;
			}
		break;
		case 0xD5:	/* PUSH DE */
			c.SP -= 2;
			c.cycles += 2;
			timer_cycle();
			write_word_ticked(c.SP, get_DE());
		break;
		case 0xD6:	/* SUB A, imm8 */
			t = mem_get_byte(c.PC);
			set_C((c.A - t) < 0);
			set_H(((c.A - t)&0xF) > (c.A&0xF));
			c.A -= t;
			set_N(1);
			set_Z(!c.A);
			c.PC += 1;
			c.cycles += 2;
		break;
		case 0xD7:	/* RST 10 */
			c.cycles += 2;
			timer_cycle();
			c.SP -= 2;
			write_word_ticked(c.SP, c.PC);
			c.PC = 0x10;
		break;
		case 0xD8:	/* RET C */
			if(flag_C == 1)
			{
				c.cycles += 2;
				timer_cycle();
				c.PC = get_word_ticked(c.SP);
				c.SP += 2;
				c.cycles += 1;
			} else {
				c.cycles += 2;
			}
		break;
		case 0xD9:	/* RETI */
			c.cycles += 1;
			timer_cycle();
			c.PC = get_word_ticked(c.SP);
			c.SP += 2;
			c.cycles += 1;
			interrupt_enable();
		break;
		case 0xDA:	/* JP C, mem16 */
			if(flag_C == 1)
			{
				c.cycles += 1;
				timer_cycle();
				c.PC = get_word_ticked(c.PC);
				c.cycles += 1;
			} else {
				c.PC += 2;
				c.cycles += 3;
			}
		break;
		case 0xDC:	/* CALL C, mem16 */
			c.cycles += 1;
			timer_cycle();
			if(flag_C == 1)
			{
				i = get_word_ticked(c.PC);
				c.cycles += 1;
				timer_cycle();
				c.SP -= 2;
				write_word_ticked(c.SP, c.PC+2);
				c.PC = i;
			} else {
				c.PC += 2;
				c.cycles += 2;
			}
		break;
		case 0xDE:	/* SBC A, imm8 */
			t = mem_get_byte(c.PC);
			b = flag_C;
			set_H(((t&0xF) + flag_C) > (c.A&0xF));
			set_C(t + flag_C > c.A);
			set_N(1);
			c.A -= (b + t);
			set_Z(!c.A);
			c.PC += 1;
			c.cycles += 2;
		break;
		case 0xDF:	/* RST 18 */
			c.cycles += 2;
			timer_cycle();
			c.SP -= 2;
			write_word_ticked(c.SP, c.PC);
			c.PC = 0x18;
		break;
		case 0xE0:	/* LD (FF00 + imm8), A */
			t = mem_get_byte(c.PC);
			c.cycles += 2;
			timer_cycle();
			mem_write_byte(0xFF00 + t, c.A);
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0xE1:	/* POP HL */
			c.cycles += 1;
			timer_cycle();
			i = get_word_ticked(c.SP);
			set_HL(i);
			c.SP += 2;
		break;
		case 0xE2:	/* LD (FF00 + C), A */
			c.cycles += 1;
			timer_cycle();
			s = 0xFF00 + c.C;
			mem_write_byte(s, c.A);
			c.cycles += 1;
		break;
		case 0xE5:	/* PUSH HL */
			c.SP -= 2;
			c.cycles += 2;
			timer_cycle();
			write_word_ticked(c.SP, get_HL());
		break;
		case 0xE6:	/* AND A, imm8 */
			t = mem_get_byte(c.PC);
			set_N(0);
			set_H(1);
			set_C(0);
			c.A = t & c.A;
			set_Z(!c.A);
			c.PC += 1;
			c.cycles += 2;
		break;
		case 0xE7:	/* RST 20 */
			c.cycles += 2;
			timer_cycle();
			c.SP -= 2;
			write_word_ticked(c.SP, c.PC);
			c.PC = 0x20;
		break;
		case 0xE8:	/* ADD SP, imm8 */
			c.cycles += 1;
			timer_cycle();
			i = mem_get_byte(c.PC);
			c.PC += 1;
			set_Z(0);
			set_N(0);
			set_C(((c.SP+i)&0xFF) < (c.SP&0xFF));
			set_H(((c.SP+i)&0xF) < (c.SP&0xF));
			c.SP += (signed char)i;
			c.cycles += 3;
		break;
		case 0xE9:	/* JP HL */
			c.PC = get_HL();
			c.cycles += 1;
		break;
		case 0xEA:	/* LD (mem16), a */
			c.cycles += 1;
			timer_cycle();
			s = get_word_ticked(c.PC);
			mem_write_byte(s, c.A);
			c.PC += 2;
			c.cycles += 1;
		break;
		case 0xEE:	/* XOR A, imm8 */
			c.A ^= mem_get_byte(c.PC);
			c.F = (!c.A)<<7;
			c.PC += 1;
			c.cycles += 2;
		break;
		case 0xEF:	/* RST 28 */
			c.cycles += 2;
			timer_cycle();
			c.SP -= 2;
			write_word_ticked(c.SP, c.PC);
			c.PC = 0x28;
		break;
		case 0xF0:	/* LD A, (FF00 + imm8) */
			t = mem_get_byte(c.PC);
			c.cycles += 2;
			timer_cycle();
			c.A = mem_get_byte(0xFF00 + t);
			c.PC += 1;
			c.cycles += 1;
		break;
		case 0xF1:	/* POP AF */
			c.cycles += 1;
			timer_cycle();
			s = get_word_ticked(c.SP);
			set_AF(s&0xFFF0);
			c.SP += 2;
		break;
		case 0xF2:	/* LD A, (FF00 + c) */
			c.cycles += 1;
			timer_cycle();
			c.A = mem_get_byte(0xFF00 + c.C);
			c.cycles += 1;
		break;
		case 0xF3:	/* DI */
			c.cycles += 1;
			interrupt_disable();
		break;
		case 0xF5:	/* PUSH AF */
			c.SP -= 2;
			mem_write_word(c.SP, get_AF());
			c.cycles += 4;
		break;
		case 0xF6:	/* OR A, imm8 */
			c.A |= mem_get_byte(c.PC);
			c.F = (!c.A)<<7;
			c.PC += 1;
			c.cycles += 2;
		break;
		case 0xF7:	/* RST 30 */
			c.cycles += 2;
			timer_cycle();
			c.SP -= 2;
			write_word_ticked(c.SP, c.PC);
			c.PC = 0x30;
		break;
		case 0xF8:	/* LD HL, SP + imm8 */
			c.cycles += 1;
			timer_cycle();
			i = mem_get_byte(c.PC);
			set_N(0);
			set_Z(0);
			set_C(((c.SP+i)&0xFF) < (c.SP&0xFF));
			set_H(((c.SP+i)&0xF) < (c.SP&0xF));
			set_HL(c.SP + (signed char)i);
			c.PC += 1;
			c.cycles += 2;
		break;
		case 0xF9:	/* LD SP, HL */
			c.SP = get_HL();
			c.cycles += 2;
		break;
		case 0xFA:	/* LD A, (mem16) */
			c.cycles += 1;
			timer_cycle();
			s = get_word_ticked(c.PC);
			c.A = mem_get_byte(s);
			c.PC += 2;
			c.cycles += 1;
		break;
		case 0xFB:	/* EI */
			if(!interrupt_get_enabled())
				ei_flag = 1;

			interrupt_enable();
			c.cycles += 1;
		break;
		case 0xFE:	/* CP a, imm8 */
			t = mem_get_byte(c.PC);
			set_Z(c.A == t);
			set_N(1);
			set_H(((c.A - t)&0xF) > (c.A&0xF));
			set_C(c.A < t);
			c.PC += 1;
			c.cycles += 2;
		break;
		case 0xFF:	/* RST 38 */
			c.cycles += 2;
			timer_cycle();
			c.SP -= 2;
			write_word_ticked(c.SP, c.PC);
			c.PC = 0x38;
		break;
		default:
			printf("Unhandled opcode %02X at %04X\n", b, c.PC);
			printf("cycles: %d\n", c.cycles);
			return 0;
		break;
	}

	return 1;
}
